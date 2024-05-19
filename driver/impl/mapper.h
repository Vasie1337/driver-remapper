#pragma once
namespace mapper
{
	PIMAGE_SECTION_HEADER translate_raw_section(PIMAGE_NT_HEADERS nt, DWORD rva)
	{
		auto section = IMAGE_FIRST_SECTION(nt);
		for (auto i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++section)
		{
			if (rva >= section->VirtualAddress && rva < section->VirtualAddress + section->Misc.VirtualSize)
			{
				return section;
			}
		}

		return NULL;
	}

	PVOID translate_raw(PBYTE base, PIMAGE_NT_HEADERS nt, DWORD rva)
	{
		auto section = translate_raw_section(nt, rva);
		if (!section)
		{
			return NULL;
		}

		return base + section->PointerToRawData + (rva - section->VirtualAddress);
	}

	void resolve_relocs(uintptr_t imageBase, uintptr_t newBase, uintptr_t delta)
	{
		const auto dosHeaders = reinterpret_cast<PIMAGE_DOS_HEADER>(imageBase);
		const auto ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS64>(imageBase + dosHeaders->e_lfanew);

		DWORD reloc_va = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;

		if (!reloc_va)
			return;

		auto current_base_relocation = reinterpret_cast<PIMAGE_BASE_RELOCATION>(newBase + reloc_va);
		const auto reloc_end = reinterpret_cast<uint64_t>(current_base_relocation) + ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;

		while (current_base_relocation->VirtualAddress && current_base_relocation->VirtualAddress < reloc_end && current_base_relocation->SizeOfBlock)
		{
			uint64_t current_reloc_address = newBase + current_base_relocation->VirtualAddress;
			uint16_t* current_reloc_item = reinterpret_cast<uint16_t*>(reinterpret_cast<uint64_t>(current_base_relocation) + sizeof(IMAGE_BASE_RELOCATION));
			uint32_t current_reloc_count = (current_base_relocation->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(uint16_t);

			for (auto i = 0u; i < current_reloc_count; ++i)
			{
				const uint16_t type = current_reloc_item[i] >> 12;
				const uint16_t offset = current_reloc_item[i] & 0xFFF;

				if (type == IMAGE_REL_BASED_DIR64)
					*reinterpret_cast<uint64_t*>(current_reloc_address + offset) += delta;
			}

			current_base_relocation = reinterpret_cast<PIMAGE_BASE_RELOCATION>(reinterpret_cast<uint64_t>(current_base_relocation) + current_base_relocation->SizeOfBlock);
		}
	}

	BOOLEAN resolve_imports(uintptr_t imageBase)
	{
		const auto dosHeaders = reinterpret_cast<PIMAGE_DOS_HEADER>(imageBase);
		const auto ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS64>(imageBase + dosHeaders->e_lfanew);

		auto rva = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
		if (!rva)
		{
			return TRUE;
		}

		auto importDescriptor = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(translate_raw(reinterpret_cast<PBYTE>(imageBase), ntHeaders, rva));
		if (!importDescriptor)
		{
			return TRUE;
		}

		for (; importDescriptor->FirstThunk; ++importDescriptor)
		{
			auto moduleName = reinterpret_cast<PCHAR>(translate_raw(reinterpret_cast<PBYTE>(imageBase), ntHeaders, importDescriptor->Name));
			if (!moduleName)
			{
				break;
			}


			auto modinfo = modules::get_kernel_module(moduleName);

			if (!modinfo)
			{
				return FALSE;
			}

			for (auto thunk = reinterpret_cast<PIMAGE_THUNK_DATA>(translate_raw(reinterpret_cast<PBYTE>(imageBase), ntHeaders, importDescriptor->FirstThunk)); thunk->u1.AddressOfData; ++thunk)
			{
				auto importByName = reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(translate_raw(reinterpret_cast<PBYTE>(imageBase), ntHeaders, static_cast<DWORD>(thunk->u1.AddressOfData)));

				uintptr_t funcPtr = reinterpret_cast<uintptr_t>(imports::rtl_find_exported_routine_by_name(reinterpret_cast<PVOID>(modinfo.address), importByName->Name));

				if (!funcPtr)
				{
					return FALSE;
				}

				thunk->u1.Function = funcPtr;
			}
		}

		return TRUE;
	}
}