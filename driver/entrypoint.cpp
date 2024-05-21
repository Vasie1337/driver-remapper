#include <ntifs.h>
#include <windef.h>
#include <cstdint>
#include <intrin.h>
#include <ntimage.h>
#include <ntstatus.h>

#include <kernel/defs.h>
#include <kernel/structures.hpp>
#include <kernel/xor.hpp>

#include <impl/imports.h>
#include <impl/crt.h>

#include <impl/physical/physical.h>
#include <impl/pml4/pml4.h>
#include <impl/communication/structs.h>
#include <impl/requests/requests.h>
#include <impl/ioctl/ioctl.h>

constexpr auto DEVICE_NAME = L"\\Device\\{67481902-14CD-4FCB-B17F-A7515AD33274}";
constexpr auto DOS_NAME = L"\\DosDevices\\{67481902-14CD-4FCB-B17F-A7515AD33274}";

static NTSTATUS driver_entry( PDRIVER_OBJECT driver_obj, PUNICODE_STRING registry_path )
{
	UNREFERENCED_PARAMETER( registry_path );

	UNICODE_STRING Device;
	UNICODE_STRING DosDevices;

	imports::rtl_init_unicode_string( &Device, skCrypt( DEVICE_NAME ) );
	imports::rtl_init_unicode_string( &DosDevices, skCrypt( DOS_NAME ) );

	PDEVICE_OBJECT DeviceObject = NULL;
	NTSTATUS m_status = imports::io_create_device( driver_obj,
		0,
		&Device,
		FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN,
		FALSE,
		&DeviceObject 
	);

	if (NT_SUCCESS( m_status ))
	{
		SetFlag( driver_obj->Flags, DO_BUFFERED_IO );

		driver_obj->MajorFunction[IRP_MJ_CREATE] = reinterpret_cast< PDRIVER_DISPATCH >( ioctl::io_close );
		driver_obj->MajorFunction[IRP_MJ_CLOSE] = reinterpret_cast< PDRIVER_DISPATCH >( ioctl::io_close );
		driver_obj->MajorFunction[IRP_MJ_DEVICE_CONTROL] = reinterpret_cast< PDRIVER_DISPATCH >( ioctl::io_dispatch );
		driver_obj->DriverUnload = nullptr;

		ClearFlag( DeviceObject->Flags, DO_DIRECT_IO );
		ClearFlag( DeviceObject->Flags, DO_DEVICE_INITIALIZING );

		m_status = imports::io_create_symbolic_link( &DosDevices, &Device );

		if (!NT_SUCCESS( m_status ))
			imports::io_delete_device( DeviceObject );
	}

	return m_status;
}

__int64 DriverEntry( PVOID a1, PVOID a2 )
{
	if (!NT_SUCCESS(imports::io_create_driver(NULL, reinterpret_cast<PDRIVER_INITIALIZE>(driver_entry))))
	{
		printf("failed to create driver.\n");
		return STATUS_UNSUCCESSFUL;
	}

	return STATUS_SUCCESS;
}
