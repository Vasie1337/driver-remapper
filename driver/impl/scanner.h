#pragma once

#include <impl/crt.h>

namespace scanner
{
    std::uint64_t find_pattern(std::uint64_t base, size_t range, const char* pattern, const char* mask)
    {
        const auto check_mask = [](const char* base, const char* pattern, const char* mask) -> bool
        {
            for (; *mask; ++base, ++pattern, ++mask)
            {
                if (*mask == 'x' && *base != *pattern)
                {
                    return false;
                }
            }

            return true;
        };

        range = range - crt::strlen(mask);

        for (size_t i = 0; i < range; ++i)
        {
            if (check_mask((const char*)base + i, pattern, mask))
            {
                return base + i;
            }
        }

        return NULL;
    }

    std::uint64_t find_pattern(std::uint64_t base, const char* pattern, const char* mask)
    {
        const PIMAGE_NT_HEADERS headers = (PIMAGE_NT_HEADERS)(base + ((PIMAGE_DOS_HEADER)base)->e_lfanew);
        const PIMAGE_SECTION_HEADER sections = IMAGE_FIRST_SECTION(headers);

        for (size_t i = 0; i < headers->FileHeader.NumberOfSections; i++)
        {
            const PIMAGE_SECTION_HEADER section = &sections[i];

            if (section->Characteristics & IMAGE_SCN_MEM_EXECUTE)
            {
                const auto match = find_pattern(base + section->VirtualAddress, section->Misc.VirtualSize, pattern, mask);

                if (match)
                {
                    return match;
                }
            }
        }

        return 0;
    }

    std::uint64_t find_pattern(std::uint64_t module_base, const char* pattern)
    {
        auto pattern_ = pattern;
        std::uint64_t first_match = 0;

        if (!module_base)
        {
            return 0;
        }

        const auto nt = reinterpret_cast<IMAGE_NT_HEADERS*>(module_base + reinterpret_cast<IMAGE_DOS_HEADER*>(module_base)->e_lfanew);

        for (std::uint64_t current = module_base; current < module_base + nt->OptionalHeader.SizeOfImage; current++)
        {
            if (!*pattern_)
            {
                return first_match;
            }

            if (*(std::uint8_t*)pattern_ == '\?' || *(std::uint8_t*)current == get_byte(pattern_))
            {
                if (!first_match)
                    first_match = current;

                if (!pattern_[2])
                    return first_match;

                if (*(WORD*)pattern_ == '\?\?' || *(std::uint8_t*)pattern_ != '\?')
                    pattern_ += 3;

                else
                    pattern_ += 2;
            }
            else
            {
                pattern_ = pattern;
                first_match = 0;
            }
        }

        return 0;
    }

    unsigned __int64 search_byte_sequence(unsigned __int64 base, unsigned __int64 size, unsigned char byte_sequence[])
    {
        for (int i = 0; i < size - sizeof(byte_sequence); ++i)
        {
            if (RtlCompareMemory((void*)(base + i), byte_sequence, sizeof(byte_sequence)) == sizeof(byte_sequence))
            {
                return base + i;
            }
        }
        return 0;
    }
}