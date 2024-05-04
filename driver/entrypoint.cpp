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

#include "requests/physical/directory_table_base.cpp"
#include "requests/physical/pml4_walk_entry.cpp"

#include "requests/get_module_base.cpp"
#include "requests/read_physical_memory.cpp"
#include "requests/write_physical_memory.cpp"

std::uintptr_t m_cave_base = 0;
std::uintptr_t m_new_entry = 0;

#define DEVICE_NAME L"\\Device\\{3DC31255-A121-4FEA-B9F4-DD7367D106FF}"
#define DOS_NAME L"\\DosDevices\\{3DC31255-A121-4FEA-B9F4-DD7367D106FF}"

#define TARGET_DRIVER_PATH L"\\??\\C:\\Users\\Vasie\\Desktop\\IndirectKmd.sys"

namespace ioctl
{
	NTSTATUS io_dispatch(PDEVICE_OBJECT device_object, PIRP irp)
	{
		UNREFERENCED_PARAMETER(device_object);

		invoke_data* fortnite_request = (invoke_data*)(irp->AssociatedIrp.SystemBuffer);

		//printf( "code: %llx\n", fortnite_request->code );

		switch (fortnite_request->code)
		{
		case invoke_resolve_dtb:
		{
			if (request::resolve_dtb(fortnite_request) != STATUS_SUCCESS)
			{
				irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
			}
			break;
		}

		case invoke_base:
		{
			if (request::get_module_base(fortnite_request) != STATUS_SUCCESS)
			{
				irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
			}
			break;
		}

		case invoke_read:
		{
			if (request::read_physical_memory(fortnite_request) != STATUS_SUCCESS)
			{
				irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
			}
			break;
		}

		case invoke_peb:
		{
			if (request::resolve_peb(fortnite_request) != STATUS_SUCCESS)
			{
				irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
			}
			break;
		}

		case invoke_write:
		{
			if (request::write_physical_memory(fortnite_request) != STATUS_SUCCESS)
			{
				irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
			}
			break;
		}
		}

		irp->IoStatus.Status = STATUS_SUCCESS;
		imports::iof_complete_request(irp, IO_NO_INCREMENT);

		return STATUS_SUCCESS;
	}

	NTSTATUS io_close(PDEVICE_OBJECT, PIRP Irp)
	{
		Irp->IoStatus.Status = STATUS_SUCCESS;
		Irp->IoStatus.Information = 0;

		imports::iof_complete_request(Irp, IO_NO_INCREMENT);
		return STATUS_SUCCESS;
	}

}

NTSTATUS signed_cave_entry(PDRIVER_OBJECT driver_obj, PUNICODE_STRING registry_path)
{
	UNREFERENCED_PARAMETER(registry_path);

	printf("successfully dispatched new driver.\n ");

	BYTE m_shellcode[] = {
			0x48, 0xB8,  // Load a 64-bit immediate value into RAX register
			0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, // Placeholder for our entry.
			0xFF, 0xE0	// Jump to the address specified in RAX register	
	};

	*(PVOID*)&m_shellcode[2] = reinterpret_cast<PVOID>(&ioctl::io_dispatch);

	if (!NT_SUCCESS(ctx::write_protected_address((PVOID)m_new_entry, m_shellcode, sizeof(m_shellcode), TRUE)))
	{
		printf("failed to write io dispatch.\n");
		return STATUS_UNSUCCESSFUL;
	}

	*(PVOID*)&m_shellcode[2] = reinterpret_cast<PVOID>(&ioctl::io_close);

	auto m_create_close = reinterpret_cast<PVOID>(m_cave_base + sizeof(m_shellcode));

	if (!NT_SUCCESS(ctx::write_protected_address(m_create_close, m_shellcode, sizeof(m_shellcode), TRUE)))
	{
		printf("failed to write io close.\n");
		return STATUS_UNSUCCESSFUL;
	}

	UNICODE_STRING Device;
	UNICODE_STRING DosDevices;

	imports::rtl_init_unicode_string(&Device, skCrypt(DEVICE_NAME));
	imports::rtl_init_unicode_string(&DosDevices, skCrypt(DOS_NAME));

	PDEVICE_OBJECT DeviceObject = NULL;
	NTSTATUS m_status = imports::io_create_device(driver_obj,
		0,
		&Device,
		FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN,
		FALSE,
		&DeviceObject);


	if (NT_SUCCESS(m_status))
	{
		SetFlag(driver_obj->Flags, DO_BUFFERED_IO);

		driver_obj->MajorFunction[IRP_MJ_CREATE] = reinterpret_cast<PDRIVER_DISPATCH>(m_create_close);
		driver_obj->MajorFunction[IRP_MJ_CLOSE] = reinterpret_cast<PDRIVER_DISPATCH>(m_create_close);
		driver_obj->MajorFunction[IRP_MJ_DEVICE_CONTROL] = reinterpret_cast<PDRIVER_DISPATCH>(m_new_entry);
		driver_obj->DriverUnload = nullptr;

		ClearFlag(DeviceObject->Flags, DO_DIRECT_IO);
		ClearFlag(DeviceObject->Flags, DO_DEVICE_INITIALIZING);

		m_status = imports::io_create_symbolic_link(&DosDevices, &Device);

		if (!NT_SUCCESS(m_status))
		{
			printf("failed to create symbolic link, status: %p\n", m_status);
			imports::io_delete_device(DeviceObject);
		}
	}
	else
	{
		printf("failed to create device, status: %p\n", m_status);
	}

	return m_status;
}

NTSTATUS hook()
{
	const auto system_service = modules::get_kernel_module(skCrypt("IndirectKmd.sys"));

	if (!system_service)
	{
		printf("invalid system service.\n");
		return STATUS_UNSUCCESSFUL;
	}

	BYTE section_name[] = { '.', 't', 'e', 'x', 't', '\0' };

	m_cave_base = modules::find_section(system_service, reinterpret_cast<char*>(section_name));
	if (!m_cave_base)
	{
		printf("failed to find .text section.\n");
		return STATUS_UNSUCCESSFUL;
	}

	crt::kmemset(&section_name, 0, sizeof(section_name));

	BYTE m_shellcode[] = {
			0x48, 0xB8,  // Load a 64-bit immediate value into RAX register
			0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, // Placeholder for our entry.
			0xFF, 0xE0	// Jump to the address specified in RAX register	
	};

	m_new_entry = m_cave_base;

	*(PVOID*)&m_shellcode[2] = reinterpret_cast<PVOID>(signed_cave_entry);
	ctx::write_protected_address((PVOID)m_new_entry, m_shellcode, sizeof(m_shellcode), TRUE);

	imports::io_create_driver(NULL, reinterpret_cast<PDRIVER_INITIALIZE>(m_new_entry));

	return STATUS_SUCCESS;
}

NTSTATUS bypass_integrity_check()
{
	PCWSTR file_path = TARGET_DRIVER_PATH;

	UNICODE_STRING unicode_file_path = { 0 };
	OBJECT_ATTRIBUTES object_attributes = { 0 };
	IO_STATUS_BLOCK io_status_block = { 0 };

	HANDLE FileHandle = 0;

	imports::rtl_init_unicode_string(&unicode_file_path, TARGET_DRIVER_PATH);

	InitializeObjectAttributes(&object_attributes,
		&unicode_file_path,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		NULL,
		NULL
	);

	return ZwCreateFile(
		&FileHandle,
		GENERIC_READ,
		&object_attributes,
		&io_status_block,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		0,
		FILE_OPEN,
		FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0
	);
}

__int64 DriverEntry(PVOID a1, PVOID a2)
{
	auto status = bypass_integrity_check();
	if (!NT_SUCCESS(status)) {
		printf("Failed to bypass the integrity check, status: %p\n", status);
	}

	return hook();
}