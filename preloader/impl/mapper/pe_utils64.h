#pragma once
namespace pe_utils
{
	void* get_module_base(LPCSTR moduleName);
	void* get_module_list();
	void* get_loaded_module_base(const unsigned short* mod_name);
	void* find_export_by_ordinal(void* module, UINT16 ordinal);
	void* find_export(void* module, const unsigned char* routine_name);
	UINT32* find_export_entry(void* module, const CHAR8* routine_name);
	UINT32* find_export_entry_by_ordinal(void* module, UINT16 ordinal);

	static UINT64 ascii_to_int(CHAR8* ascii)
	{
		UINT64 return_int = 0;
		while (*ascii)
		{
			if (*ascii <= '0' || *ascii >= '9')
				return 0;
			return_int *= 10;
			return_int += *ascii - '0';
			ascii++;
		}
		return return_int;
	}

	CHAR16 wc_to_lower(CHAR16 c)
	{
		if (c >= 'A' && c <= 'Z')
			return c += ('a' - 'A');
		else return c;
	}

	__int64 u_wcsnicmp(const CHAR16* First, const CHAR16* Second, unsigned long long Length)
	{
		for (int i = 0; i < Length && First[i] && Second[i]; ++i) // Channeling my inner Python developer
			if (wc_to_lower(First[i]) != wc_to_lower(Second[i]))
				return First[i] - Second[i];

		return 0;
	}

	void* get_module_list() 
	{
		ULONG length = 0;
		imports::zw_query_system_information((SYSTEM_INFORMATION_CLASS)SystemModuleInformation, 0, 0, &length);
		length += (10 * 1024);

		void* module_list = imports::ex_allocate_pool((POOL_TYPE)(POOL_COLD_ALLOCATION | PagedPool), length);
		NTSTATUS status = imports::zw_query_system_information((SYSTEM_INFORMATION_CLASS)SystemModuleInformation, module_list, length, &length);

		if (status) {
			if (module_list) ExFreePool(module_list);
			return 0;
		}

		if (!module_list) {
			return 0;
		}

		return module_list;
	}

	UINT32* find_export_entry_by_ordinal(void* module, UINT16 ordinal) 
	{
		PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)module;
		if (dos->e_magic != 0x5A4D)
			return NULL;

		PIMAGE_NT_HEADERS64 nt = (PIMAGE_NT_HEADERS)((UINT8*)module + dos->e_lfanew);
		UINT32 exports_rva = nt->OptionalHeader.DataDirectory[0].VirtualAddress; // This corresponds to export directory
		if (!exports_rva)
			return NULL;

		PIMAGE_EXPORT_DIRECTORY export_dir = (PIMAGE_EXPORT_DIRECTORY)((UINT8*)module + exports_rva);
		UINT16 index = ordinal - (UINT16)export_dir->Base;

		UINT32* export_func_table = (UINT32*)((UINT8*)module + export_dir->AddressOfFunctions);
		if (export_func_table[index] < nt->OptionalHeader.DataDirectory[0].VirtualAddress ||
			export_func_table[index] > nt->OptionalHeader.DataDirectory[0].VirtualAddress + nt->OptionalHeader.DataDirectory[0].Size)
			return export_func_table + index;
		// Handle the case of a forwarder export entry
		else
		{
			CHAR16 buffer[260];
			CHAR8* forwarder_rva_string = (CHAR8*)module + export_func_table[index];
			UINT16 dll_name_length;
			for (dll_name_length = 0; dll_name_length < 259; ++dll_name_length)
				if (forwarder_rva_string[dll_name_length] == '.') break;
			for (int i = 0; i < dll_name_length; ++i)
				buffer[i] = (CHAR16)forwarder_rva_string[i];
			buffer[dll_name_length] = L'\0';
			if (forwarder_rva_string[dll_name_length + 1] == '#')
				return find_export_entry_by_ordinal(get_loaded_module_base(buffer), (UINT16)ascii_to_int(&forwarder_rva_string[dll_name_length + 2]));
			else
				return find_export_entry(get_loaded_module_base(buffer), forwarder_rva_string + dll_name_length + 1);
		}
	}

	UINT32* find_export_entry(void* module, const CHAR8* routine_name) 
	{
		PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)module;
		if (dos->e_magic != 0x5A4D)
			return NULL;

		PIMAGE_NT_HEADERS64 nt = (PIMAGE_NT_HEADERS)((UINT8*)module + dos->e_lfanew);
		UINT32 exports_rva = nt->OptionalHeader.DataDirectory[0].VirtualAddress; // This corresponds to export directory
		if (!exports_rva)
			return NULL;

		PIMAGE_EXPORT_DIRECTORY export_dir = (PIMAGE_EXPORT_DIRECTORY)((UINT8*)module + exports_rva);
		UINT32* name_table = (UINT32*)((UINT8*)module + export_dir->AddressOfNames);

		// Binary Search
		for (int lower = 0, upper = export_dir->NumberOfNames - 1; upper >= lower;)
		{
			int i = (upper + lower) / 2;
			const CHAR8* func_name = (CHAR8*)((UINT8*)module + name_table[i]);
			__int64 diff = strcmp((const char*)routine_name, (const char*)func_name);
			if (diff > 0)
				lower = i + 1;
			else if (diff < 0)
				upper = i - 1;
			else
			{
				UINT32* export_func_table = (UINT32*)((UINT8*)module + export_dir->AddressOfFunctions);
				UINT16* ordinal_table = (UINT16*)((UINT8*)module + export_dir->AddressOfNameOrdinals);

				UINT16 index = ordinal_table[i];
				if (export_func_table[index] < nt->OptionalHeader.DataDirectory[0].VirtualAddress ||
					export_func_table[index] > nt->OptionalHeader.DataDirectory[0].VirtualAddress + nt->OptionalHeader.DataDirectory[0].Size)
					return export_func_table + index;
				// Handle the case of a forwarder export entry
				else
				{
					CHAR16 buffer[260];
					CHAR8* forwarder_rva_string = (CHAR8*)module + export_func_table[index];
					UINT16 dll_name_length;
					for (dll_name_length = 0; dll_name_length < 259; ++dll_name_length)
						if (forwarder_rva_string[dll_name_length] == '.') break;
					for (int j = 0; j < dll_name_length; ++j)
						buffer[j] = (CHAR16)forwarder_rva_string[j];
					buffer[dll_name_length] = L'\0';
					if (forwarder_rva_string[dll_name_length + 1] == '#')
						return find_export_entry_by_ordinal(get_loaded_module_base(buffer), (UINT16)ascii_to_int(&forwarder_rva_string[dll_name_length + 2]));
					else
						return find_export_entry(get_loaded_module_base(buffer), forwarder_rva_string + dll_name_length + 1);
				}
			}
		}
		return NULL;
	}

	void* find_export(void* module, const unsigned char* routine_name)
	{
		UINT32* entry = find_export_entry(module, routine_name);
		if (!entry)
			return NULL;
		return (void*)((UINT8*)module + *entry);
	}

	KLDR_DATA_TABLE_ENTRY* get_module_from_list(LIST_ENTRY* head, const CHAR16* mod_name)
	{
		for (LIST_ENTRY* it = head->Flink; it && it != head; it = it->Flink)
		{
			KLDR_DATA_TABLE_ENTRY* entry = CONTAINING_RECORD(it, KLDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
			if (!u_wcsnicmp(entry->BaseDllName.Buffer, mod_name, entry->BaseDllName.Length))
			{
				return entry;
			}
		}
		return NULL;
	}

	void* get_loaded_module_base(const unsigned short* mod_name)
	{
		void* g_kernel_base = (void*)get_module_base(0);

		static LIST_ENTRY* PsLoadedModuleList;
		if (!PsLoadedModuleList)
			PsLoadedModuleList = (LIST_ENTRY*)find_export(g_kernel_base, (const unsigned char*)"PsLoadedModuleList");

		KLDR_DATA_TABLE_ENTRY* module = get_module_from_list(PsLoadedModuleList, mod_name);
		if (!module)
			return NULL;
		return module->DllBase;
	}

	void* find_export_by_ordinal(void* module, UINT16 ordinal)
	{
		UINT32* entry = find_export_entry_by_ordinal(module, ordinal);
		if (!entry)
			return NULL;
		return (void*)((UINT8*)module + *entry);
	}

	void* get_module_base(LPCSTR moduleName) 
	{
		void* moduleBase = NULL;
		ULONG info = 0;
		NTSTATUS status = imports::zw_query_system_information(SystemModuleInformation, 0, info, &info);

		if (!info) {
			return moduleBase;
		}

		PRTL_PROCESS_MODULES modules = (PRTL_PROCESS_MODULES)imports::ex_allocate_pool(NonPagedPool, info);
		status = imports::zw_query_system_information(SystemModuleInformation, modules, info, &info);
		if (!NT_SUCCESS(status) || !modules) {
			return moduleBase;
		}
		PRTL_PROCESS_MODULE_INFORMATION module = modules->Modules;
		if (modules->NumberOfModules > 0) {
			if (!moduleName) {
				moduleBase = modules->Modules[0].ImageBase;
			}
			else {
				for (auto i = 0; i < modules->NumberOfModules; i++) {
					if (!strcmp((CHAR*)module[i].FullPathName, moduleName)) {
						moduleBase = module[i].ImageBase;
					}
				}
			}
		}

		if (modules) {
			imports::ex_free_pool_with_tag(modules, 0);
		}

		return moduleBase;
	}
}