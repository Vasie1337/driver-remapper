#pragma once
namespace ioctl
{
	NTSTATUS io_dispatch(PDEVICE_OBJECT device_object, PIRP irp)
	{
		UNREFERENCED_PARAMETER(device_object);

		PVARS vars = (PVARS)device_object->DeviceExtension;

		invoke_data* request = (invoke_data*)(irp->AssociatedIrp.SystemBuffer);

		switch (request->code)
		{
		case invoke_resolve_dtb:
		{
			if (request::resolve_dtb(request, vars) != STATUS_SUCCESS)
			{
				irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
			}
			break;
		}

		case invoke_base:
		{
			if (request::get_module_base(request) != STATUS_SUCCESS)
			{
				irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
			}
			break;
		}

		case invoke_read:
		{
			if (request::read_physical_memory(request, vars) != STATUS_SUCCESS)
			{
				irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
			}
			break;
		}

		case invoke_peb:
		{
			if (request::resolve_peb(request) != STATUS_SUCCESS)
			{
				irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
			}
			break;
		}

		case invoke_write:
		{
			if (request::write_physical_memory(request, vars) != STATUS_SUCCESS)
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