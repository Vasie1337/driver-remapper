#pragma once
namespace mapper
{
	void resolve_relocs(std::uint64_t imageBase, PIMAGE_NT_HEADERS nt_headers)
	{
		INT64 load_delta = (INT64)(imageBase - nt_headers->OptionalHeader.ImageBase);
		PIMAGE_DATA_DIRECTORY reloc = &nt_headers->OptionalHeader.DataDirectory[5];

		for (PRELOC_BLOCK_HDR i = (PRELOC_BLOCK_HDR)(imageBase + reloc->VirtualAddress); 
			i < (PRELOC_BLOCK_HDR)(imageBase + reloc->VirtualAddress + reloc->Size); 
			*(std::uint8_t**)&i += i->BlockSize
			)
		{
			for (PRELOC_ENTRY entry = (PRELOC_ENTRY)i + 4; (std::uint8_t*)entry < (std::uint8_t*)i + i->BlockSize; ++entry)
			{
				if (entry->Type == 0xA)
					*(UINT64*)(imageBase + i->PageRVA + entry->Offset) += load_delta;
			}
		}
	}

	ULONG64 get_export(PBYTE base, PCHAR _export) {
		PIMAGE_DOS_HEADER dosHeaders = (PIMAGE_DOS_HEADER)base;
		if (dosHeaders->e_magic != IMAGE_DOS_SIGNATURE) {
			return 0;
		}

		PIMAGE_NT_HEADERS64 ntHeaders = (PIMAGE_NT_HEADERS64)(base + dosHeaders->e_lfanew);

		ULONG exportsRva = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
		if (!exportsRva) {
			return 0;
		}

		PIMAGE_EXPORT_DIRECTORY exports = (PIMAGE_EXPORT_DIRECTORY)(base + exportsRva);
		PULONG nameRva = (PULONG)(base + exports->AddressOfNames);

		for (ULONG i = 0; i < exports->NumberOfNames; ++i) {
			PCHAR func = (PCHAR)(base + nameRva[i]);
			if (strcmp(func, _export) == 0) {
				PULONG funcRva = (PULONG)(base + exports->AddressOfFunctions);
				PWORD ordinalRva = (PWORD)(base + exports->AddressOfNameOrdinals);

				return (ULONG64)base + funcRva[ordinalRva[i]];
			}
		}

		return 0;
	}

	void resolve_imports(std::uint64_t imageBase, PIMAGE_NT_HEADERS nt_headers)
	{
		ULONG importsRva = nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
		if (importsRva) {
			PIMAGE_IMPORT_DESCRIPTOR importDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)(imageBase + importsRva);

			for (; importDescriptor->FirstThunk; ++importDescriptor) {
				PCHAR moduleName = (PCHAR)(imageBase + importDescriptor->Name);
				auto module = modules::get_kernel_module(moduleName);
				
				PIMAGE_THUNK_DATA64 thunk = (PIMAGE_THUNK_DATA64)(imageBase + importDescriptor->FirstThunk);
				PIMAGE_THUNK_DATA64 thunkOriginal = (PIMAGE_THUNK_DATA64)(imageBase + importDescriptor->OriginalFirstThunk);

				for (; thunk->u1.AddressOfData; ++thunk, ++thunkOriginal) {
					PCHAR importName = ((PIMAGE_IMPORT_BY_NAME)(imageBase + thunkOriginal->u1.AddressOfData))->Name;
					ULONG64 import = get_export((PBYTE)module.address, importName);
					thunk->u1.Function = import;
				}
			}
		}
	}

	void unload_discardable_sections(std::uint64_t imageBase, PIMAGE_NT_HEADERS nt_headers)
	{
		const auto current_image_section = IMAGE_FIRST_SECTION(nt_headers);

		for (auto i = 0; i < nt_headers->FileHeader.NumberOfSections; ++i)
		{
			if (current_image_section[i].Characteristics & IMAGE_SCN_MEM_DISCARDABLE)
			{
				crt::kmemset(
					(void*)(imageBase + current_image_section[i].VirtualAddress), 
					0xCC, 
					current_image_section[i].SizeOfRawData
				);
			}
		}
	}
}