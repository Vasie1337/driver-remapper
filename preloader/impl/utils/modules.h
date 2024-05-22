namespace crt
{
    //int get_random_seed( )
    //{
    //    ULONG seed = ke_query_time_increment( );
    //    return rtl_random_ex( &seed );
    //}

    INT kmemcmp( const void* s1, const void* s2, size_t n )
    {
        const unsigned char* p1 = ( const unsigned char* ) s1;
        const unsigned char* end1 = p1 + n;
        const unsigned char* p2 = ( const unsigned char* ) s2;
        int                   d = 0;
        for ( ;;) {
            if ( d || p1 >= end1 ) break;
            d = ( int ) *p1++ - ( int ) *p2++;
            if ( d || p1 >= end1 ) break;
            d = ( int ) *p1++ - ( int ) *p2++;
            if ( d || p1 >= end1 ) break;
            d = ( int ) *p1++ - ( int ) *p2++;
            if ( d || p1 >= end1 ) break;
            d = ( int ) *p1++ - ( int ) *p2++;
        }
        return d;
    }

    //INT kMemcmp( const void* str1, const void* str2, size_t count )
    //{
    //    register const unsigned char* s1 = ( const unsigned char* ) str1;
    //    register const unsigned char* s2 = ( const unsigned char* ) str2;
    //    while ( count-- > 0 )
    //    {
    //        if ( *s1++ != *s2++ )
    //            return s1 [ -1 ] < s2 [ -1 ] ? -1 : 1;
    //    }
    //    return 0;
    //}


    void* kmemcpy( void* dest, const void* src, size_t len )
    {
        char* d = ( char* ) dest;
        const char* s = ( const char* ) src;
        while ( len-- )
            *d++ = *s++;
        return dest;
    }

    void* kmemset( void* dest, UINT8 c, size_t count )
    {
        size_t blockIdx;
        size_t blocks = count >> 3;
        size_t bytesLeft = count - (blocks << 3);
        UINT64 cUll =
            c
            | ((( UINT64 ) c) << 8)
            | ((( UINT64 ) c) << 16)
            | ((( UINT64 ) c) << 24)
            | ((( UINT64 ) c) << 32)
            | ((( UINT64 ) c) << 40)
            | ((( UINT64 ) c) << 48)
            | ((( UINT64 ) c) << 56);

        UINT64* destPtr8 = ( UINT64* ) dest;
        for ( blockIdx = 0; blockIdx < blocks; blockIdx++ ) destPtr8 [ blockIdx ] = cUll;

        if ( !bytesLeft ) return dest;

        blocks = bytesLeft >> 2;
        bytesLeft = bytesLeft - (blocks << 2);

        UINT32* destPtr4 = ( UINT32* ) &destPtr8 [ blockIdx ];
        for ( blockIdx = 0; blockIdx < blocks; blockIdx++ ) destPtr4 [ blockIdx ] = ( UINT32 ) cUll;

        if ( !bytesLeft ) return dest;

        blocks = bytesLeft >> 1;
        bytesLeft = bytesLeft - (blocks << 1);

        UINT16* destPtr2 = ( UINT16* ) &destPtr4 [ blockIdx ];
        for ( blockIdx = 0; blockIdx < blocks; blockIdx++ ) destPtr2 [ blockIdx ] = ( UINT16 ) cUll;

        if ( !bytesLeft ) return dest;

        UINT8* destPtr1 = ( UINT8* ) &destPtr2 [ blockIdx ];
        for ( blockIdx = 0; blockIdx < bytesLeft; blockIdx++ ) destPtr1 [ blockIdx ] = ( UINT8 ) cUll;

        return dest;
    }
}

namespace modules
{
    void* get_system_information(SYSTEM_INFORMATION_CLASS information_class)
    {
        unsigned long size = 32;
        char buffer[32]{};

        ZwQuerySystemInformation(information_class, buffer, size, &size);

        void* info = ExAllocatePool(NonPagedPool, size);

        if (!info)
            return nullptr;

        if (!NT_SUCCESS(ZwQuerySystemInformation(information_class, info, size, &size)))
        {
            ExFreePoolWithTag(info, 0);
            return nullptr;
        }

        return info;
    }

    struct section_data { 
        std::uint64_t address; 
        size_t size; 
        bool operator!() const {
            return address == 0 || size == 0;
        }
    };

    auto find_section( std::uint64_t ModuleBase, char* SectionName ) -> section_data
    {
        PIMAGE_NT_HEADERS NtHeaders = ( PIMAGE_NT_HEADERS ) ( ModuleBase + ( ( PIMAGE_DOS_HEADER ) ModuleBase )->e_lfanew );
        PIMAGE_SECTION_HEADER Sections = IMAGE_FIRST_SECTION( NtHeaders );

        for ( DWORD i = 0; i < NtHeaders->FileHeader.NumberOfSections; i++ )
        {
            PIMAGE_SECTION_HEADER Section = &Sections [ i ];
            if ( crt::kmemcmp( Section->Name, SectionName, 5 ) == 0 )
            {
                section_data data{ ModuleBase + Section->VirtualAddress, Section->SizeOfRawData };
                return data;
            }
        }

        return section_data();
    }

    struct kernel_module_data {
        std::uint64_t address;
        size_t size;
        bool operator!() const {
            return address == 0 || size == 0;
        }
    };

    kernel_module_data get_kernel_module(const char* name)
    {
        const auto to_lower = [](char* string) -> const char*
        {
            for (char* pointer = string; *pointer != '\0'; ++pointer)
            {
                *pointer = (char)(short)tolower(*pointer);
            }

            return string;
        };

        const PRTL_PROCESS_MODULES info = (PRTL_PROCESS_MODULES)get_system_information(SystemModuleInformation);

        if (!info)
            return kernel_module_data();

        for (size_t i = 0; i < info->NumberOfModules; ++i)
        {
            const auto& mod = info->Modules[i];

            //printf( "%s\n", mod.FullPathName + mod.OffsetToFileName );

            if (crt::strcmp(to_lower_c((char*)mod.FullPathName + mod.OffsetToFileName), name) == 0)
            {
                const auto address = (std::uint64_t)mod.ImageBase;
                const auto size = mod.ImageSize;
                ExFreePoolWithTag(info, 0);
                return { address, size };
            }
        }

        ExFreePoolWithTag(info, 0);
        return kernel_module_data();
    }

    bool safe_copy( void* dst, void* src, size_t size )
    {
        SIZE_T bytes = 0;

        if (MmCopyVirtualMemory( IoGetCurrentProcess(), src, IoGetCurrentProcess(), dst, size, KernelMode, &bytes ) == STATUS_SUCCESS && bytes == size)
        {
            return true;
        }

        return false;
    }

    bool load_vurn_driver(PCWSTR driver_name)
    {
        UNICODE_STRING ServiceName{};
        RtlInitUnicodeString(&ServiceName, driver_name);

        const auto Status = ZwLoadDriver(&ServiceName);
        if (Status == STATUS_SUCCESS || Status == STATUS_IMAGE_ALREADY_LOADED) {
            return true;
        }
        return false;
    }
}

namespace ctx
{
    auto write_protected_address( void* address, void* buffer, SIZE_T size, bool Restore ) -> BOOLEAN
    {
        NTSTATUS Status = { STATUS_SUCCESS };

        auto Mdl = IoAllocateMdl(address, size, FALSE, FALSE, NULL);

        MmProbeAndLockPages(Mdl, KernelMode, IoReadAccess);

        auto Mapping = MmMapLockedPagesSpecifyCache(Mdl, KernelMode, MmNonCached, NULL, FALSE, NormalPagePriority);

        Status = MmProtectMdlSystemAddress(Mdl, PAGE_READWRITE);

        if ( Status != STATUS_SUCCESS )
        {
            printf( "change page protection.\n" );
            MmUnmapLockedPages(Mapping, Mdl);
            MmUnlockPages(Mdl);
            IoFreeMdl(Mdl);
        }

        crt::kmemcpy( Mapping, buffer, size );

        if ( Restore )
        {
            Status = MmProtectMdlSystemAddress(Mdl, PAGE_READONLY);

            if ( Status != STATUS_SUCCESS )
            {
                printf( "restore page.\n" );

                MmUnmapLockedPages( Mapping, Mdl );
                MmUnlockPages( Mdl );
                IoFreeMdl( Mdl );
            }
        }

        MmUnmapLockedPages( Mapping, Mdl );
        MmUnlockPages( Mdl );
        IoFreeMdl( Mdl );

        return Status;
    }

    void nop_address_range(std::uint64_t address, size_t size, uint8_t* original_bytes)
    {
        crt::kmemcpy(original_bytes, (void*)address, size);

        uint8_t* Buffer = (uint8_t*)ExAllocatePool(NonPagedPool, size);
        if (Buffer)
        {
            crt::kmemset(Buffer, 0x90, size);
            ctx::write_protected_address((void*)address, Buffer, size, true);
            ExFreePoolWithTag(Buffer, 0);
        }
    }

    void zero_address_range(std::uint64_t address, size_t size)
    {
        uint8_t* Buffer = (uint8_t*)ExAllocatePool(NonPagedPool, size);
        if (Buffer)
        {
            crt::kmemset(Buffer, 0xCC, size);
            ctx::write_protected_address((void*)address, Buffer, size, true);
            ExFreePoolWithTag(Buffer, 0);
        }
    }

    void generate_random_bytes(std::uint8_t* buffer, size_t size) 
    {
        auto seed = KeQueryTimeIncrement();

        for (auto i = 0; i < size; ++i) 
        {
            auto random_number = RtlRandomEx(&seed);
            buffer[i] = static_cast<std::uint8_t>(random_number & 0xFF);
        }
    }

    void randomize_address_range(std::uint64_t address, size_t size)
    {
        uint8_t* Buffer = (uint8_t*)ExAllocatePool(NonPagedPool, size);
        if (Buffer)
        {
            ctx::generate_random_bytes(Buffer, size);
            ctx::write_protected_address((void*)address, Buffer, size, true);
            ExFreePoolWithTag(Buffer, 0);
        }
        else
        {
            printf("Failed to allocate pool.\n");
        }
    }

    void restore_address_range(std::uint64_t address, size_t size, uint8_t* original_bytes) 
    {
        ctx::write_protected_address((void*)address, original_bytes, size, true);
    }

    void print_bytes(std::uint64_t address, size_t size) 
    {
        for (std::uint64_t i = 0; i < size; ++i) 
        {
            const auto Byte = *(uint8_t*)(address + i);
            printf("%02x\n", Byte);
        }
    }

    size_t get_function_size(void* function)
    {
        PBYTE Byte = (PBYTE)function;
        size_t Size = 0;
        do
        {
            ++Size;
        } while (*(Byte++) != 0xC3);
        return Size;
    }
}

namespace pte
{
    unsigned __int64 find_pte_base()
    {
        unsigned char MiGetPteAddress_sequence[] =
        {
            0x48, 0xC1, 0xE9, 0x09, 0x48, 0xB8, 0xF8, 0xFF, 0xFF, 0xFF, 0x7F, 0x00, 0x00, 0x00, 0x48, 0x23, 0xC8, 0x48, 0xB8
        };

        const auto ntoskrnl = modules::get_kernel_module(skCrypt("ntoskrnl.exe"));
        if (!ntoskrnl)
        {
            printf("Couldnt find ntoskrnl.\n");
            return STATUS_UNSUCCESSFUL;
        }

        unsigned __int64 result = scanner::search_byte_sequence(unsigned __int64(ntoskrnl.address), unsigned __int64(ntoskrnl.size), MiGetPteAddress_sequence);

        return result ? *(unsigned __int64*)(result + sizeof(MiGetPteAddress_sequence)) : 0;
    }

    unsigned __int64 resolve_pte(ULONGLONG addr)
    {
        auto MiGetPteAddress = [](unsigned __int64 a1) -> unsigned __int64
        {
            unsigned __int64 pte_base = find_pte_base();

            return pte_base ? ((a1 >> 9) & 0x7FFFFFFFF8) + pte_base : 0;
        };

        return MiGetPteAddress(addr);
    }
}
