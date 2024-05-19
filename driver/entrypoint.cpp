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


#define CASE_SENSITIVE
typedef unsigned char CHAR8;
typedef unsigned short CHAR16;

typedef struct _IMAGE_IMPORT_DESCRIPTOR2 {
	UINT32   LookupTableRVA;             // RVA to original unbound IAT (PIMAGE_THUNK_DATA)
	UINT32   TimeDateStamp;                  // 0 if not bound,
	// -1 if bound, and real date\time stamp
	//     in IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT (new BIND)
	// O.W. date/time stamp of DLL bound to (Old BIND)

	UINT32   ForwarderChain;                 // -1 if no forwarders
	UINT32   Name;
	UINT32   ImportAddressTable;                     // RVA to IAT (if bound this IAT has actual addresses)
} IMAGE_IMPORT_DESCRIPTOR2;

typedef IMAGE_IMPORT_DESCRIPTOR2* PIMAGE_IMPORT_DESCRIPTOR2;

typedef struct _IMAGE_IMPORT_DESCRIPTOR_OWN {
	UINT32   LookupTableRVA;             // RVA to original unbound IAT (PIMAGE_THUNK_DATA)
	UINT32   TimeDateStamp;                  // 0 if not bound,
	// -1 if bound, and real date\time stamp
	//     in IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT (new BIND)
	// O.W. date/time stamp of DLL bound to (Old BIND)

	UINT32   ForwarderChain;                 // -1 if no forwarders
	UINT32   Name;
	UINT32   ImportAddressTable;                     // RVA to IAT (if bound this IAT has actual addresses)
} IMAGE_IMPORT_DESCRIPTOR_OWN;

typedef IMAGE_IMPORT_DESCRIPTOR_OWN* PIMAGE_IMPORT_DESCRIPTOR_OWN;
typedef struct _RELOC_NAME_TABLE_ENTRY
{
	UINT16 Hint;
	CHAR8 Name[];
} RELOC_NAME_TABLE_ENTRY, PRELOC_NAME_TABLE_ENTRY;

typedef struct _RELOC_BLOCK_HDR
{
	UINT32 PageRVA;
	UINT32 BlockSize;
} RELOC_BLOCK_HDR, * PRELOC_BLOCK_HDR;

typedef struct _RELOC_ENTRY
{
	UINT16 Offset : 12;
	UINT16 Type : 4;
} RELOC_ENTRY, * PRELOC_ENTRY;

NTSTATUS map_new_driver()
{
	PVOID base = (PVOID)text_section.address;
	ULONG size = text_section.size;


	//Get the right pe header information. and validate signatures
	IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)raw_driver_bytes;
	if (dos->e_magic != 'ZM') return STATUS_UNSUCCESSFUL;
	IMAGE_NT_HEADERS64* nt = (IMAGE_NT_HEADERS64*)(raw_driver_bytes + dos->e_lfanew);
	if (nt->Signature != (UINT32)'EP') return STATUS_UNSUCCESSFUL;

	//Allocate memory for our local driver to be prepared.
	PMDL mdl = MmAllocatePagesForMdl({ 0 }, { ~0ul }, { 0 }, nt->OptionalHeader.SizeOfImage);
	BYTE* allocation = (BYTE*)MmMapLockedPages(mdl, KernelMode);

	memcpy(allocation, raw_driver_bytes, nt->FileHeader.SizeOfOptionalHeader);


	PPTE pte = pte::resolve_pte((ULONGLONG)base);
	if (!pte) return STATUS_UNSUCCESSFUL;
	pte->nx = true;
	pte->rw = true;

	// Copy sections one at a time
	PIMAGE_SECTION_HEADER sec_hdr = (PIMAGE_SECTION_HEADER)((BYTE*)(&nt->FileHeader) + sizeof(IMAGE_FILE_HEADER) + nt->FileHeader.SizeOfOptionalHeader);
	for (int i = 0; i < nt->FileHeader.NumberOfSections; i++, sec_hdr++) {
		memcpy(allocation + sec_hdr->VirtualAddress, raw_driver_bytes + sec_hdr->PointerToRawData, sec_hdr->SizeOfRawData);

		PPTE pte = pte::resolve_pte((ULONGLONG)base + sec_hdr->VirtualAddress);
		if (!pte) return STATUS_UNSUCCESSFUL;

		//Sections could be larger than 0x1000 bytes, u will have to size of section/0x1000. to get all pages and change protection so.
		if (!strcmp((const char*)sec_hdr->Name, ".text")) {
			//for(int i = 0; i < round_up(sec_hdr->SizeOfRawData / 0x1000); i++){ and do this for all pages.
				//pte = GetPte((ULONGLONG)base + sec_hdr->VirtualAddress + 0x1000*i);
				//pte->nx = false;
				//pte->rw = false;
			//}
			pte->nx = false;
			pte->rw = false;
		}
		else if (!strcmp((const char*)sec_hdr->Name, ".data")) {

			pte->nx = true;
			pte->rw = true;
		}
		else {
			pte->nx = false;
			pte->rw = false;
		}

	}

	//CODE FROM XIGMAPPER
	// Imports
	PIMAGE_DATA_DIRECTORY import_dir = &nt->OptionalHeader.DataDirectory[1];
	for (PIMAGE_IMPORT_DESCRIPTOR2 desc = (PIMAGE_IMPORT_DESCRIPTOR2)(allocation + import_dir->VirtualAddress); desc->LookupTableRVA; ++desc)
	{
		// Get unicode name from ascii name
		CHAR16 buffer[260];
		CHAR8* mod_name = (CHAR8*)(allocation + desc->Name);
		for (int i = 0; i < 259 && mod_name[i]; ++i)
			buffer[i] = (CHAR16)mod_name[i], buffer[i + 1] = L'\0';
		PVOID module_base = util::GetLoadedModuleBase(buffer);
		for (UINT64* lookup_entry = (UINT64*)(allocation + desc->LookupTableRVA), *iat_entry = (UINT64*)(allocation + desc->ImportAddressTable); *lookup_entry; ++lookup_entry, ++iat_entry)
		{
			if (*lookup_entry & (1ull << 63))
				*(PVOID*)iat_entry = util::FindExportByOrdinal(module_base, *lookup_entry & 0xFFFF);
			else
				*(PVOID*)iat_entry = util::FindExport(module_base, ((RELOC_NAME_TABLE_ENTRY*)(allocation + (*lookup_entry & 0x7FFFFFFF)))->Name);
		}
	}

	// Relocations
	INT64 load_delta = (INT64)(allocation - nt->OptionalHeader.ImageBase);
	PIMAGE_DATA_DIRECTORY reloc = &nt->OptionalHeader.DataDirectory[5];
	for (PRELOC_BLOCK_HDR i = (PRELOC_BLOCK_HDR)(allocation + reloc->VirtualAddress); i < (PRELOC_BLOCK_HDR)(allocation + reloc->VirtualAddress + reloc->Size); *(BYTE**)&i += i->BlockSize)
		for (PRELOC_ENTRY entry = (PRELOC_ENTRY)i + 4; (BYTE*)entry < (BYTE*)i + i->BlockSize; ++entry)
			if (entry->Type == 0xA)
				*(UINT64*)(allocation + i->PageRVA + entry->Offset) += load_delta;

	// Unload discardable sections
	sec_hdr = (PIMAGE_SECTION_HEADER)((BYTE*)(&nt->FileHeader) + sizeof(IMAGE_FILE_HEADER) + nt->FileHeader.SizeOfOptionalHeader);
	for (int i = 0; i < nt->FileHeader.NumberOfSections; i++, sec_hdr++)
		if (sec_hdr->Characteristics & 0x02000000)
			memset(allocation + sec_hdr->VirtualAddress, 0x00, sec_hdr->SizeOfRawData);

	if (!nt->OptionalHeader.AddressOfEntryPoint)
		return STATUS_UNSUCCESSFUL;

	//END CODE FROM XIGMAPPER


	//Zero the memory in the driver so we dont have any information left in it.
	util::ZeroMemory(base, size);


	//Patch in our driver :D
	util::WriteToProtectedMem((void*)(QWORD)base, (BYTE*)allocation, nt->OptionalHeader.SizeOfImage);


	//Now we can free the pool since we have already patch in the driver. Also zero it out so it can be traced.
	RtlZeroMemory(allocation, nt->OptionalHeader.SizeOfImage); // First zero it out so it cant be found :D
	MmUnmapLockedPages(allocation, mdl);
	MmFreePagesFromMdl(mdl);
	ExFreePool(mdl);

	////Call our main point from target driver base + offset to DriverEntry
	unsigned long long(*DriverEntry)(PDRIVER_OBJECT obj, PUNICODE_STRING str) =
		(unsigned long long(*)(PDRIVER_OBJECT, PUNICODE_STRING))(((BYTE*)base + nt->OptionalHeader.AddressOfEntryPoint));


	DriverEntry((PDRIVER_OBJECT)0, (PUNICODE_STRING)0);

	return STATUS_SUCCESS;
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