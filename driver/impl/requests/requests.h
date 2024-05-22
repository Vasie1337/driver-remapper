#pragma once
namespace request
{
	NTSTATUS resolve_peb(invoke_data* request)
	{
		peb_invoke data = { 0 };
		std::uintptr_t out = 0;

		if (!crt::safe_copy(&data, request->data, sizeof(peb_invoke))) {
			return STATUS_UNSUCCESSFUL;
		}

		if (!data.pid) {
			return STATUS_UNSUCCESSFUL;
		}

		PEPROCESS process = 0;
		if (PsLookupProcessByProcessId((HANDLE)data.pid, &process) != STATUS_SUCCESS) {
			return STATUS_UNSUCCESSFUL;
		}

		PPEB pPeb = PsGetProcessPeb(process);

		ObfDereferenceObject(process);

		reinterpret_cast<peb_invoke*> (request->data)->handle = (ULONGLONG)pPeb;

		return STATUS_SUCCESS;
	}

	NTSTATUS get_module_base(invoke_data* request) {
		base_invoke data = { 0 };
		std::uintptr_t out = 0;

		if (!crt::safe_copy(&data, request->data, sizeof(base_invoke))) {
			return STATUS_UNSUCCESSFUL;
		}

		printf("pid: %i\n", data.pid);
		printf("looking for: %s\n", data.name);

		if (!data.pid) {
			return STATUS_UNSUCCESSFUL;
		}

		PEPROCESS process = 0;
		if (PsLookupProcessByProcessId((HANDLE)data.pid, &process) != STATUS_SUCCESS) {
			return STATUS_UNSUCCESSFUL;
		}

		if (!data.name) {
			uintptr_t base = (uintptr_t)PsGetProcessSectionBaseAddress(process);

			reinterpret_cast<base_invoke*> (request->data)->handle = base;

			return STATUS_SUCCESS;
		}

		ANSI_STRING ansi_name;
		RtlInitAnsiString(&ansi_name, data.name);

		UNICODE_STRING compare_name;
		RtlAnsiStringToUnicodeString(&compare_name, &ansi_name, TRUE);

		PPEB pPeb = PsGetProcessPeb(process);

		if (pPeb) {
			PPEB_LDR_DATA pLdr = (PPEB_LDR_DATA)pPeb->Ldr;

			if (pLdr) {
				for (PLIST_ENTRY listEntry = (PLIST_ENTRY)pLdr->ModuleListLoadOrder.Flink;
					listEntry != &pLdr->ModuleListLoadOrder;
					listEntry = (PLIST_ENTRY)listEntry->Flink) {

					PLDR_DATA_TABLE_ENTRY pEntry = CONTAINING_RECORD(listEntry, LDR_DATA_TABLE_ENTRY, InLoadOrderModuleList);
					printf("modules: %wZ\n", pEntry->BaseDllName);

					if (RtlCompareUnicodeString(&pEntry->BaseDllName, &compare_name, TRUE) == 0) {
						out = (uint64_t)pEntry->DllBase;
						break;
					}
				}
			}
		}

		RtlFreeUnicodeString(&compare_name);
		ObfDereferenceObject(process);

		reinterpret_cast<base_invoke*> (request->data)->handle = out;

		return STATUS_SUCCESS;
	}

	NTSTATUS resolve_dtb(invoke_data* request, PVARS vars)
	{
		dtb_invoke data = { 0 };

		if (!crt::safe_copy(&data, request->data, sizeof(dtb_invoke)))
		{
			return STATUS_UNSUCCESSFUL;
		}

		PEPROCESS process = 0;
		if (PsLookupProcessByProcessId((HANDLE)data.pid, &process) != STATUS_SUCCESS)
		{
			printf("invalid process.\n");
			return STATUS_UNSUCCESSFUL;
		}

		vars->m_stored_dtb = pml4::dirbase_from_base_address((void*)PsGetProcessSectionBaseAddress(process), vars);

		printf("cr3: %llx\n", vars->m_stored_dtb);

		ObfDereferenceObject(process);

		reinterpret_cast<dtb_invoke*> (request->data)->operation = true;

		return STATUS_SUCCESS;
	}

	NTSTATUS read_physical_memory(invoke_data* request, PVARS vars)
	{
		read_invoke data = { 0 };

		if (!crt::safe_copy(&data, request->data, sizeof(read_invoke)))
		{
			return STATUS_UNSUCCESSFUL;
		}

		if (!data.address || !data.pid || !data.buffer || !data.size || data.address >= 0x7FFFFFFFFFFF || !vars->m_stored_dtb)
			return STATUS_UNSUCCESSFUL;

		auto physical_address = physical::translate_linear(
			vars->m_stored_dtb,
			data.address);

		if (!physical_address)
		{
			return STATUS_UNSUCCESSFUL;
		}

		auto final_size = physical::find_min(
			PAGE_SIZE - (physical_address & 0xFFF),
			data.size);

		size_t bytes = 0;
		if (!NT_SUCCESS(physical::read_physical(
			physical_address,
			reinterpret_cast<void*>((reinterpret_cast<ULONG64>(data.buffer))),
			final_size,
			&bytes)))
		{
			printf("failed to do read\n");
			return STATUS_UNSUCCESSFUL;
		}

		return STATUS_SUCCESS;
	}

	NTSTATUS write_physical_memory(invoke_data* request, PVARS vars) {
		write_invoke data = { 0 };

		if (!crt::safe_copy(&data, request->data, sizeof(write_invoke))) {
			return STATUS_UNSUCCESSFUL;
		}

		if (!data.address || !data.pid || !data.buffer || !data.size || data.address >= 0x7FFFFFFFFFFF || !vars->m_stored_dtb) {
			return STATUS_UNSUCCESSFUL;
		}

		auto physical_address = physical::translate_linear(
			vars->m_stored_dtb,
			data.address);

		if (!physical_address) {
			return STATUS_UNSUCCESSFUL;
		}

		auto final_size = physical::find_min(
			PAGE_SIZE - (physical_address & 0xFFF),
			data.size);

		size_t bytes = 0;
		if (!NT_SUCCESS(physical::write_physical(
			physical_address,
			reinterpret_cast<void*>((reinterpret_cast<ULONG64>(data.buffer))),
			final_size,
			&bytes)))
		{
			return STATUS_UNSUCCESSFUL;
		}

		return STATUS_SUCCESS;
	}
}