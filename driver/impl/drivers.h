#pragma once
namespace drivers
{
    NTSTATUS __fastcall invalid_rq(PDEVICE_OBJECT DeviceObject, PIRP Irp)
    {
        UNREFERENCED_PARAMETER(DeviceObject);
        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    NTSTATUS __fastcall create_driver(
        _In_opt_ PUNICODE_STRING DriverName,
        _In_ PEXDRIVER_INITIALIZE InitializationFunction,
        _In_ PVOID Parameter)


    {
        WCHAR NameBuffer[100];
        USHORT NameLength;
        UNICODE_STRING LocalDriverName;
        NTSTATUS Status;
        OBJECT_ATTRIBUTES ObjectAttributes;
        ULONG ObjectSize;
        PDRIVER_OBJECT DriverObject;
        UNICODE_STRING ServiceKeyName;
        HANDLE hDriver;
        ULONG i, RetryCount = 0;
        ULONG Seed = 0x7dfd66a4e;

    try_again:
        /* First, create a unique name for the driver if we don't have one */
        if (!DriverName)
        {
            /* Create a random name and set up the string */
            NameLength = (USHORT)swprintf(NameBuffer,
                L"\\Driver\\%08u",
                RtlRandomEx(&Seed));
            LocalDriverName.Length = NameLength * sizeof(WCHAR);
            LocalDriverName.MaximumLength = LocalDriverName.Length + sizeof(UNICODE_NULL);
            LocalDriverName.Buffer = NameBuffer;
        }
        else
        {
            /* So we can avoid another code path, use a local var */
            LocalDriverName = *DriverName;
        }

        /* Initialize the Attributes */
        ObjectSize = sizeof(DRIVER_OBJECT) + sizeof(EXTENDED_DRIVER_EXTENSION);
        InitializeObjectAttributes(&ObjectAttributes,
            &LocalDriverName,
            OBJ_PERMANENT | OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            NULL,
            NULL);

        /* Create the Object */
        Status = ObCreateObject(KernelMode,
            IoDriverObjectType,
            &ObjectAttributes,
            KernelMode,
            NULL,
            ObjectSize,
            0,
            0,
            (PVOID*)&DriverObject);
        if (!NT_SUCCESS(Status)) return Status;

        /* Set up the Object */
        RtlZeroMemory(DriverObject, ObjectSize);
        DriverObject->Type = IO_TYPE_DRIVER;
        DriverObject->Size = sizeof(DRIVER_OBJECT);
        DriverObject->Flags = DRVO_BUILTIN_DRIVER;
        DriverObject->DriverExtension = (PDRIVER_EXTENSION)(DriverObject + 1);
        DriverObject->DriverExtension->DriverObject = DriverObject;
        DriverObject->DriverInit = (PDRIVER_INITIALIZE)InitializationFunction;
        /* Loop all Major Functions */
        for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
        {
            /* Invalidate each function */
            DriverObject->MajorFunction[i] = invalid_rq;
        }

        /* Set up the service key name buffer */
        ServiceKeyName.MaximumLength = LocalDriverName.Length + sizeof(UNICODE_NULL);
        ServiceKeyName.Buffer = (PWCH)ExAllocatePoolWithTag(PagedPool, LocalDriverName.MaximumLength, 'xCoI');
        if (!ServiceKeyName.Buffer)
        {
            /* Fail */
            ObMakeTemporaryObject(DriverObject);
            ObDereferenceObject(DriverObject);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        /* For builtin drivers, the ServiceKeyName is equal to DriverName */
        RtlCopyUnicodeString(&ServiceKeyName, &LocalDriverName);
        ServiceKeyName.Buffer[ServiceKeyName.Length / sizeof(WCHAR)] = UNICODE_NULL;
        DriverObject->DriverExtension->ServiceKeyName = ServiceKeyName;

        /* Make a copy of the driver name to store in the driver object */
        DriverObject->DriverName.MaximumLength = LocalDriverName.Length;
        DriverObject->DriverName.Buffer = (PWCH)ExAllocatePoolWithTag(PagedPool,
            DriverObject->DriverName.MaximumLength,
            'xCoI');
        if (!DriverObject->DriverName.Buffer)
        {
            /* Fail */
            ObMakeTemporaryObject(DriverObject);
            ObDereferenceObject(DriverObject);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlCopyUnicodeString(&DriverObject->DriverName, &LocalDriverName);

        /* Add the Object and get its handle */
        Status = ObInsertObject(DriverObject,
            NULL,
            FILE_READ_DATA,
            0,
            NULL,
            &hDriver);

        /* Eliminate small possibility when this function is called more than
           once in a row, and KeTickCount doesn't get enough time to change */
        if (!DriverName && (Status == STATUS_OBJECT_NAME_COLLISION) && (RetryCount < 100))
        {
            RetryCount++;
            goto try_again;
        }

        if (!NT_SUCCESS(Status)) return Status;

        /* Now reference it */
        Status = ObReferenceObjectByHandle(hDriver,
            0,
            IoDriverObjectType,
            KernelMode,
            (PVOID*)&DriverObject,
            NULL);

        /* Close the extra handle */
        ZwClose(hDriver);

        if (!NT_SUCCESS(Status))
        {
            /* Fail */
            ObMakeTemporaryObject(DriverObject);
            ObDereferenceObject(DriverObject);
            return Status;
        }

        /* Finally, call its init function */
        Status = InitializationFunction(DriverObject, NULL, Parameter);
        if (!NT_SUCCESS(Status))
        {
            /* If it didn't work, then kill the object */
            ObMakeTemporaryObject(DriverObject);
            ObDereferenceObject(DriverObject);
            return Status;
        }

        /* Windows does this fixup, keep it for compatibility */
        for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
        {
            /*
             * Make sure the driver didn't set any dispatch entry point to NULL!
             * Doing so is illegal; drivers shouldn't touch entry points they
             * do not implement.
             */

             /* Check if it did so anyway */
            if (!DriverObject->MajorFunction[i])
            {
                /* Fix it up */
                DriverObject->MajorFunction[i] = invalid_rq;
            }
        }

        /* Return the Status */
        return Status;
    }
}