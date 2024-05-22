#include <ntifs.h>
#include <windef.h>
#include <cstdint>
#include <intrin.h>
#include <ntimage.h>
#include <ntstatus.h>

#include <kernel/defs.h>
#include <kernel/structures.hpp>
#include <kernel/xor.hpp>

#include <impl/crt.h>

#include <impl/physical/physical.h>
#include <impl/pml4/pml4.h>
#include <impl/communication/structs.h>
#include <impl/requests/requests.h>
#include <impl/ioctl/ioctl.h>

#define DEVICE_NAME L"\\Device\\{37581902-15CD-4FCB-B17F-A7515AD33274}"
#define DOS_NAME L"\\DosDevices\\{37581902-15CD-4FCB-B17F-A7515AD33274}"

static NTSTATUS driver_entry( PDRIVER_OBJECT driver_obj, PUNICODE_STRING registry_path )
{
	UNREFERENCED_PARAMETER( registry_path );

	printf("Setting up IOCTL...\n");

	UNICODE_STRING Device;
	UNICODE_STRING DosDevices;

	RtlInitUnicodeString( &Device, skCrypt( DEVICE_NAME ) );
	RtlInitUnicodeString( &DosDevices, skCrypt( DOS_NAME ) );

	PDEVICE_OBJECT DeviceObject = { };
	NTSTATUS Status = IoCreateDevice(driver_obj, 0, &Device, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &DeviceObject);
	if (!NT_SUCCESS(Status)) {
		printf("Failed to create device.\n");
		printf("0x%llx.\n", Status);
		return Status;
	}

	if (DeviceObject && !DeviceObject->DeviceExtension) {
		DeviceObject->DeviceExtension = ExAllocatePool(NonPagedPool, sizeof(VARS));
	}
	else {
		return STATUS_ALREADY_INITIALIZED;
	}

	Status = IoCreateSymbolicLink(&DosDevices, &Device);
	if (!NT_SUCCESS(Status)) {
		printf("Failed to create link.\n");
		printf("0x%llx.\n", Status);
		IoDeleteDevice(DeviceObject);
		return Status;
	}

	SetFlag(driver_obj->Flags, DO_BUFFERED_IO);
	
	driver_obj->MajorFunction[IRP_MJ_CREATE] = ioctl::io_close;
	driver_obj->MajorFunction[IRP_MJ_CLOSE] = ioctl::io_close;
	driver_obj->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ioctl::io_dispatch;

	ClearFlag(DeviceObject->Flags, DO_DIRECT_IO);
	ClearFlag(DeviceObject->Flags, DO_DEVICE_INITIALIZING);
	
	return Status;
}

__int64 DriverEntry(void* a1, void* a2)
{
	printf("Dispatched new driver.\n");

	NTSTATUS Status = IoCreateDriver(NULL, driver_entry);

	if (!NT_SUCCESS(Status))
	{
		printf("Failed to create driver.\n");
		printf("0x%llx.\n", Status);
		return STATUS_UNSUCCESSFUL;
	}

	return STATUS_SUCCESS;
}
