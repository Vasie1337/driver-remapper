#include <ntifs.h>
#include <windef.h>
#include <cstdint>
#include <intrin.h>
#include <ntimage.h>
#include <ntstatus.h>
#include <ntstrsafe.h>

#include <kernel/defs.h>
#include <kernel/structures.hpp>
#include <kernel/xor.hpp>

#include <impl/imports.h>
#include <impl/bytes.h>

#include <impl/utils/scanner.h>
#include <impl/utils/modules.h>
#include <impl/utils/drivers.h>

#include <impl/mapper/mapper.h>

modules::section_data text_section = modules::section_data();

static NTSTATUS map_new_driver()
{
	printf("Mapping driver...\n");

	const auto dos_headers = reinterpret_cast<PIMAGE_DOS_HEADER>(raw_driver_bytes);
	const auto nt_headers = reinterpret_cast<PIMAGE_NT_HEADERS>(raw_driver_bytes + dos_headers->e_lfanew);

	auto driver_size = nt_headers->OptionalHeader.SizeOfImage;

	if (driver_size > text_section.size)
	{
		printf("\rDriver size too big.\n");
		return STATUS_UNSUCCESSFUL;
	}

	auto local_image_base = reinterpret_cast<std::uint64_t>(imports::ex_allocate_pool(NonPagedPool, driver_size));
	if (!local_image_base)
	{
		printf("\rFailed to allocate local driver base.\n");
		return STATUS_UNSUCCESSFUL;
	}

	crt::kmemcpy(reinterpret_cast<void*>(local_image_base), raw_driver_bytes, sizeof(raw_driver_bytes));
	
	const auto current_image_section = IMAGE_FIRST_SECTION(nt_headers);

	for (auto i = 0; i < nt_headers->FileHeader.NumberOfSections; ++i) 
	{
		if ((current_image_section[i].Characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA) > 0)
			continue;

		crt::kmemcpy(
			reinterpret_cast<void*>(local_image_base + current_image_section[i].VirtualAddress),
			raw_driver_bytes + current_image_section[i].PointerToRawData,
			current_image_section[i].SizeOfRawData
		);
	}

	mapper::resolve_relocs(local_image_base, nt_headers);
	printf("\rResolved relocations.\n");

	mapper::resolve_imports(local_image_base, nt_headers);
	printf("\rResolved imports.\n");

	mapper::unload_discardable_sections(local_image_base, nt_headers);
	printf("\rUnloaded discardable sections.\n");

	ctx::write_protected_address(
		reinterpret_cast<void*>(text_section.address), 
		reinterpret_cast<void*>(local_image_base),
		driver_size, 
		true
	);
	printf("\rPatched driver to target.\n");

	crt::kmemset(reinterpret_cast<void*>(local_image_base), 0xcc, driver_size);
	imports::ex_free_pool_with_tag(reinterpret_cast<void*>(local_image_base), 0);
	
	printf("\rCalling entry...\n");

	const auto entry_address = reinterpret_cast<void*>(text_section.address + nt_headers->OptionalHeader.AddressOfEntryPoint);
	NTSTATUS(*entry_point)(void* p1, void* p2) = (NTSTATUS(*)(void*, void*))(entry_address);
	
	return entry_point(0, 0);
}

NTSTATUS manual_mapped_entry(void* a1, void* a2)
{
	const auto ntoskrnl = modules::get_kernel_module(skCrypt("ntoskrnl.exe"));
	if (!ntoskrnl)
	{
		printf("Couldnt find ntoskrnl.\n");
		return STATUS_UNSUCCESSFUL;
	}
	printf("Ntoskrnl base: 0x%llx\n", ntoskrnl.address);

	const auto calldrv_address = scanner::find_pattern(ntoskrnl.address, skCrypt("\xE8\x00\x00\x00\x00\x44\x8B\xF0\x85\xC0\x78\x15"), skCrypt("x????xxxxxxx"));
	if (!calldrv_address)
	{
		printf("Invalid pattern.\n");
		return STATUS_UNSUCCESSFUL;
	}
	printf("PnpCallDriverEntry address: 0x%llx\n", calldrv_address);

	const auto original_bytes = reinterpret_cast<std::uint8_t*>(imports::ex_allocate_pool(NonPagedPool, 0x5));
	if (!original_bytes)
	{
		printf("Failed to allocate buffer.\n");
		return STATUS_UNSUCCESSFUL;
	}

	ctx::nop_address_range(calldrv_address, 0x5, original_bytes);
	printf("Patched IopLoadDriver.\n");

	if (!modules::load_vurn_driver(skCrypt(L"\\registry\\machine\\SYSTEM\\CurrentControlSet\\Services\\drv")))
	{
		printf("Couldnt load driver.\n");
		ctx::restore_address_range(calldrv_address, 0x5, original_bytes);
		imports::ex_free_pool_with_tag(original_bytes, 0);
		return STATUS_UNSUCCESSFUL;
	}
	printf("Loaded vurnable driver.\n");
	
	ctx::restore_address_range(calldrv_address, 0x5, original_bytes);
	imports::ex_free_pool_with_tag(original_bytes, 0);
	printf("Restored original bytes.\n");

	const auto vurn_driver = modules::get_kernel_module(skCrypt("rtwlanu.sys"));
	if (!vurn_driver)
	{
		printf("Couldnt find vurnable driver.\n");
		return STATUS_UNSUCCESSFUL;
	}
	printf("Vurnable driver base: 0x%llx\n", vurn_driver.address);

	text_section = modules::find_section(vurn_driver.address, skCrypt(".text"));
	if (!text_section) 
	{
		printf("Text section is invalid.\n");
		return STATUS_UNSUCCESSFUL;
	}

	ctx::zero_address_range(text_section.address, text_section.size);

	const auto status = map_new_driver();
	if (NT_SUCCESS(status))
	{
		printf("Success!\n");
	}
	return status;
}