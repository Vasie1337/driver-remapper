#pragma once
#include <impl/mapper/pe_utils64.h>

namespace mapper
{
	void resolve_relocs(std::uint64_t imageBase, PIMAGE_NT_HEADERS nt_headers)
	{
		INT64 load_delta = (INT64)(imageBase - nt_headers->OptionalHeader.ImageBase);
		PIMAGE_DATA_DIRECTORY reloc = &nt_headers->OptionalHeader.DataDirectory[5];
		for (PRELOC_BLOCK_HDR i = (PRELOC_BLOCK_HDR)(imageBase + reloc->VirtualAddress); i < (PRELOC_BLOCK_HDR)(imageBase + reloc->VirtualAddress + reloc->Size); *(std::uint8_t**)&i += i->BlockSize)
			for (PRELOC_ENTRY entry = (PRELOC_ENTRY)i + 4; (std::uint8_t*)entry < (std::uint8_t*)i + i->BlockSize; ++entry)
				if (entry->Type == 0xA)
					*(UINT64*)(imageBase + i->PageRVA + entry->Offset) += load_delta;
	}

	void resolve_imports(std::uint64_t imageBase, PIMAGE_NT_HEADERS nt_headers)
	{
		PIMAGE_DATA_DIRECTORY import_dir = &nt_headers->OptionalHeader.DataDirectory[1];
		for (PIMAGE_IMPORT_DESCRIPTOR2 desc = (PIMAGE_IMPORT_DESCRIPTOR2)(imageBase + import_dir->VirtualAddress); desc->LookupTableRVA; ++desc)
		{
			CHAR16 buffer[260];
			CHAR8* mod_name = (CHAR8*)(imageBase + desc->Name);
			for (int i = 0; i < 259 && mod_name[i]; ++i)
				buffer[i] = (CHAR16)mod_name[i], buffer[i + 1] = L'\0';
			void* module_base = pe_utils::get_loaded_module_base(buffer);
			for (UINT64* lookup_entry = (UINT64*)(imageBase + desc->LookupTableRVA), *iat_entry = (UINT64*)(imageBase + desc->ImportAddressTable); *lookup_entry; ++lookup_entry, ++iat_entry)
			{
				if (*lookup_entry & (1ull << 63))
					*(void**)iat_entry = pe_utils::find_export_by_ordinal(module_base, *lookup_entry & 0xFFFF);
				else
					*(void**)iat_entry = pe_utils::find_export(module_base, ((RELOC_NAME_TABLE_ENTRY*)(imageBase + (*lookup_entry & 0x7FFFFFFF)))->Name);
			}
		}
	}

	void unload_discardable_sections(std::uint64_t imageBase, PIMAGE_NT_HEADERS nt_headers)
	{
		auto sec_hdr = (PIMAGE_SECTION_HEADER)((std::uint8_t*)(&nt_headers->FileHeader) + sizeof(IMAGE_FILE_HEADER) + nt_headers->FileHeader.SizeOfOptionalHeader);
		for (int i = 0; i < nt_headers->FileHeader.NumberOfSections; i++, sec_hdr++)
			if (sec_hdr->Characteristics & 0x02000000)
				crt::kmemset((void*)(imageBase + sec_hdr->VirtualAddress), 0x00, sec_hdr->SizeOfRawData);
	}
}