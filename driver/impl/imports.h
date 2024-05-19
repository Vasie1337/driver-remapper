#pragma once
#include <ntifs.h>

#define VentroAPI inline

EXTERN_C
{
NTSTATUS MmCopyVirtualMemory
(
	PEPROCESS SourceProcess,
	PVOID SourceAddress,
	PEPROCESS TargetProcess,
	PVOID TargetAddress,
	SIZE_T BufferSize,
	KPROCESSOR_MODE PreviousMode,
	PSIZE_T ReturnSize
);
NTSTATUS
ZwQuerySystemInformation
(
	SYSTEM_INFORMATION_CLASS SystemInformationClass,
	PVOID SystemInformation,
	ULONG SystemInformationLength,
	ULONG* ReturnLength
);
PPEB 
PsGetProcessPeb
(
	PEPROCESS Process
);
PVOID
PsGetProcessSectionBaseAddress
(
	PEPROCESS Process
);
NTSTATUS
IoCreateDriver
(
	PUNICODE_STRING DriverName,
	PDRIVER_INITIALIZE InitializationFunction
);

NTSTATUS 
ObCreateObject
(
	KPROCESSOR_MODE ProbeMode,
	POBJECT_TYPE 	Type,
	POBJECT_ATTRIBUTES ObjectAttributes,
	KPROCESSOR_MODE 	AccessMode,
	PVOID ParseContext,
	ULONG 	ObjectSize,
	ULONG PagedPoolCharge,
	ULONG NonPagedPoolCharge,
	PVOID * Object
);

PVOID
RtlFindExportedRoutineByName
(
	PVOID ImageBase,
	PCCH RoutineNam
);

__declspec(dllimport) POBJECT_TYPE IoDriverObjectType;
}

namespace imports
{
	//struct m_imported
	//{
	//	uintptr_t ex_allocate_pool;
	//	uintptr_t zw_query_system_information;
	//	uintptr_t ex_free_pool_with_tag;
	//	//uintptr_t ke_get_current_thread; //ntoskrnl.exe KeGetCurrentThread -> 
	//	uintptr_t rtl_init_ansi_string;
	//	uintptr_t rtl_ansi_string_to_unicode_string;
	//	uintptr_t mm_copy_virtual_memory;
	//	uintptr_t io_get_current_process;
	//	uintptr_t ps_lookup_process_by_process_id;
	//	uintptr_t ps_get_process_peb;
	//	uintptr_t ob_reference_object_safe;
	//	//uintptr_t zw_allocate_virtual_memory;
	//	uintptr_t rtl_compare_unicode_string;
	//	uintptr_t rtl_free_unicode_string;
	//	uintptr_t obf_dereference_object;
	//	uintptr_t mm_copy_memory;
	//	uintptr_t ps_get_process_section_base_address;
	//	//uintptr_t zw_query_virtual_memory;
	//	//uintptr_t zw_free_virtual_memory;
	//	uintptr_t io_create_driver;
	//	uintptr_t io_allocate_mdl;
	//	uintptr_t mm_probe_and_lock_pages;
	//	uintptr_t mm_map_locked_pages_specify_cache;
	//	uintptr_t mm_protect_mdl_system_address;
	//	uintptr_t mm_unmap_locked_pages;
	//	uintptr_t mm_unlock_pages;
	//	uintptr_t io_free_mdl;
	//	uintptr_t iof_complete_request;
	//	uintptr_t rtl_init_unicode_string;
	//	//uintptr_t ex_raise_hard_error;
	//	uintptr_t io_create_symbolic_link;
	//	uintptr_t io_delete_device;
	//	uintptr_t io_create_device;
	//	uintptr_t rtl_get_version;
	//	uintptr_t mm_map_io_space_ex;
	//	uintptr_t mm_unmap_io_space;
	//	uintptr_t mm_get_virtual_for_physical;
	//	uintptr_t mm_get_physical_memory_ranges;
	//	//uintptr_t rtl_random_ex;
	//	//uintptr_t ke_query_time_increment;
	//	//uintptr_t ke_stack_attach_process;
	//	//uintptr_t ke_unstack_detach_process;
	//};
	// 
	//m_imported imported;

	VentroAPI NTSTATUS rtl_get_version( PRTL_OSVERSIONINFOW lpVersionInformation )
	{
		return RtlGetVersion(lpVersionInformation);
	}

	VentroAPI PVOID mm_map_io_space_ex( PHYSICAL_ADDRESS PhysicalAddress, SIZE_T NumberOfBytes, ULONG Protect )
	{
		return MmMapIoSpaceEx(PhysicalAddress, NumberOfBytes, Protect);
	}

	VentroAPI PVOID rtl_find_exported_routine_by_name(PVOID ImageBase, PCCH RoutineNam)
	{
		return RtlFindExportedRoutineByName(ImageBase, RoutineNam);
	}

	VentroAPI NTSTATUS zw_load_driver( PUNICODE_STRING DriverServiceName )
	{
		return ZwLoadDriver(DriverServiceName);
	}

	VentroAPI VOID mm_unmap_io_space( PVOID BaseAddress, SIZE_T NumberOfBytes )
	{
		return MmUnmapIoSpace(BaseAddress, NumberOfBytes);
	}

	VentroAPI PPHYSICAL_MEMORY_RANGE mm_get_physical_memory_ranges( )
	{
		return MmGetPhysicalMemoryRanges( );
	}

	VentroAPI NTSTATUS zw_query_system_information( SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength )
	{
		return ZwQuerySystemInformation(SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength);
	}

	VentroAPI PVOID ex_allocate_pool( POOL_TYPE PoolType, SIZE_T NumberOfBytes )
	{
		return ExAllocatePool(PoolType, NumberOfBytes);
	}

	VentroAPI BOOLEAN ob_reference_object_safe( PVOID Object )
	{
		return ObReferenceObjectSafe(Object);
	}

	VentroAPI void ex_free_pool_with_tag( PVOID P, ULONG TAG )
	{
		return ExFreePoolWithTag(P, TAG);
	}

	//VentroAPI PKTHREAD ke_get_current_thread( )
	//{
	//	return reinterpret_cast< PKTHREAD( * )() >(imported.ke_get_current_thread)();
	//}

	VentroAPI VOID rtl_init_ansi_string( PANSI_STRING DestinationString, PCSZ SourceString )
	{
		return RtlInitAnsiString(DestinationString, SourceString);
	}

	VentroAPI NTSTATUS rtl_ansi_string_to_unicode_string( PUNICODE_STRING DestinationString, PCANSI_STRING SourceString, BOOLEAN AllocateDestinationString )
	{
		return RtlAnsiStringToUnicodeString(DestinationString, SourceString, AllocateDestinationString);
	}

	VentroAPI NTSTATUS mm_copy_virtual_memory( PEPROCESS SourceProcess, PVOID SourceAddress, PEPROCESS TargetProcess, PVOID TargetAddress, SIZE_T BufferSize, KPROCESSOR_MODE PreviousMode, PSIZE_T ReturnSize )
	{
		return MmCopyVirtualMemory(SourceProcess, SourceAddress, TargetProcess, TargetAddress, BufferSize, PreviousMode, ReturnSize);
	}

	VentroAPI PEPROCESS io_get_current_process( )
	{
		return IoGetCurrentProcess();
	}

	//VentroAPI NTSTATUS zw_allocate_virtual_memory( HANDLE ProcessHandle, PVOID* BaseAddress, ULONG_PTR ZeroBits, PSIZE_T RegionSize, ULONG AllocationType, ULONG Protect )
	//{
	//	return reinterpret_cast< NTSTATUS( * )(HANDLE, PVOID*, ULONG_PTR, PSIZE_T, ULONG, ULONG) > (imported.zw_allocate_virtual_memory)(ProcessHandle, BaseAddress, ZeroBits, RegionSize, AllocationType, Protect);
	//}

	VentroAPI NTSTATUS ps_lookup_process_by_process_id( HANDLE ProcessId, PEPROCESS* Process )
	{
		return PsLookupProcessByProcessId(ProcessId, Process);
	}

	VentroAPI PPEB ps_get_process_peb( PEPROCESS Process )
	{
		return PsGetProcessPeb(Process);
	}

	VentroAPI LONG rtl_compare_unicode_string( PCUNICODE_STRING String1, PCUNICODE_STRING String2, BOOLEAN CaseInSensitive )
	{
		return RtlCompareUnicodeString(String1, String2, CaseInSensitive);
	}

	VentroAPI VOID rtl_free_unicode_string( PUNICODE_STRING UnicodeString )
	{
		return RtlFreeUnicodeString(UnicodeString);
	}

	VentroAPI LONG_PTR obf_dereference_object( PVOID Object )
	{
		return ObfDereferenceObject(Object);
	}

	VentroAPI NTSTATUS mm_copy_memory( PVOID TargetAddress, MM_COPY_ADDRESS SourceAddress, SIZE_T NumberOfBytes, ULONG Flags, PSIZE_T NumberOfBytesTransferred )
	{
		return MmCopyMemory(TargetAddress, SourceAddress, NumberOfBytes, Flags, NumberOfBytesTransferred);
	}

	VentroAPI PVOID ps_get_process_section_base_address( PEPROCESS Process )
	{
		return PsGetProcessSectionBaseAddress(Process);
	}

	//VentroAPI NTSTATUS zw_query_virtual_memory( HANDLE ProcessHandle, PVOID BaseAddress, MEMORY_INFORMATION_CLASS MemoryInformationClass, PVOID MemoryInformation, SIZE_T MemoryInformationLength, PSIZE_T ReturnLength )
	//{
	//	return reinterpret_cast< NTSTATUS( * )(HANDLE, PVOID, MEMORY_INFORMATION_CLASS, PVOID, SIZE_T, PSIZE_T) >(imported.zw_query_virtual_memory)(ProcessHandle, BaseAddress, MemoryInformationClass, MemoryInformation, MemoryInformationLength, ReturnLength);
	//}

	//VentroAPI NTSTATUS zw_free_virtual_memory( HANDLE ProcessHandle, PVOID* BaseAddress, PSIZE_T RegionSize, ULONG FreeType )
	//{
	//	return reinterpret_cast< NTSTATUS( * )(HANDLE, PVOID*, PSIZE_T, ULONG) >(imported.zw_free_virtual_memory)(ProcessHandle, BaseAddress, RegionSize, FreeType);
	//}

	VentroAPI NTSTATUS io_create_driver( PUNICODE_STRING Driver, PDRIVER_INITIALIZE INIT )
	{
		return IoCreateDriver(Driver, INIT);
	}

	VentroAPI PMDL io_allocate_mdl( PVOID VirtualAddress, ULONG Length, BOOLEAN SecondaryBuffer, BOOLEAN ChargeQuota, PIRP Irp )
	{
		return IoAllocateMdl(VirtualAddress, Length, SecondaryBuffer, ChargeQuota, Irp);
	}

	VentroAPI VOID mm_probe_and_lock_pages( PMDL MemoryDescriptorList, KPROCESSOR_MODE AccessMode, LOCK_OPERATION Operation )
	{
		return MmProbeAndLockPages(MemoryDescriptorList, AccessMode, Operation);
	}

	VentroAPI PVOID mm_map_locked_pages_specify_cache( PMDL MemoryDescriptorList, KPROCESSOR_MODE AccessMode, MEMORY_CACHING_TYPE CacheType, PVOID RequestedAddress, ULONG BugCheckOnFailure, ULONG Priority )
	{
		return MmMapLockedPagesSpecifyCache(MemoryDescriptorList, AccessMode, CacheType, RequestedAddress, BugCheckOnFailure, Priority);
	}

	VentroAPI NTSTATUS mm_protect_mdl_system_address( PMDL MemoryDescriptorList, ULONG NewProtect )
	{
		return MmProtectMdlSystemAddress(MemoryDescriptorList, NewProtect);
	}

	VentroAPI VOID mm_unmap_locked_pages( PVOID BaseAddress, PMDL MemoryDescriptorList )
	{
		return MmUnmapLockedPages(BaseAddress, MemoryDescriptorList);
	}

	VentroAPI VOID mm_unlock_pages( PMDL MemoryDescriptorList )
	{
		return MmUnlockPages(MemoryDescriptorList);
	}

	VentroAPI VOID io_free_mdl( PMDL Mdl )
	{
		return IoFreeMdl(Mdl);
	}
	
	VentroAPI VOID iof_complete_request( PIRP Irp, CCHAR PriorityBoost )
	{
		return IofCompleteRequest(Irp, PriorityBoost);
	}

	VentroAPI VOID rtl_init_unicode_string( PUNICODE_STRING DestinationString, PCWSTR SourceString )
	{
		return RtlInitUnicodeString(DestinationString, SourceString);
	}	
	
	//VentroAPI NTSTATUS ex_raise_hard_error( NTSTATUS ErrorStatus, ULONG NumberOfParameters,
	//	ULONG UnicodeStringParameterMask, PULONG_PTR Parameters,
	//	ULONG ValidResponseOptions, PULONG Response )
	//{
	//	return reinterpret_cast< NTSTATUS( * )( NTSTATUS, ULONG, ULONG, PULONG_PTR, ULONG, PULONG ) >(imported.ex_raise_hard_error )( ErrorStatus, NumberOfParameters , UnicodeStringParameterMask, Parameters, ValidResponseOptions, Response);
	//}

	VentroAPI NTSTATUS io_create_symbolic_link( PUNICODE_STRING SymbolicLinkName, PUNICODE_STRING DeviceName )
	{
		return IoCreateSymbolicLink(SymbolicLinkName, DeviceName);
	}

	VentroAPI VOID io_delete_device( PDEVICE_OBJECT DeviceObject )
	{
		return IoDeleteDevice(DeviceObject);
	}

	VentroAPI NTSTATUS io_create_device( PDRIVER_OBJECT DriverObject, ULONG DeviceExtensionSize, PUNICODE_STRING DeviceName, DEVICE_TYPE DeviceType, ULONG DeviceCharacteristics, BOOLEAN Exclusive, PDEVICE_OBJECT* DeviceObject )
	{
		return IoCreateDevice(DriverObject, DeviceExtensionSize, DeviceName, DeviceType, DeviceCharacteristics, Exclusive, DeviceObject);
	}

	//VentroAPI ULONG ke_query_time_increment(  )
	//{
	//	return reinterpret_cast< ULONG( * )( ) >( imported.ke_query_time_increment )( );
	//}

	//VentroAPI ULONG rtl_random_ex( PULONG Seed )
	//{
	//	return reinterpret_cast< ULONG( * )( PULONG ) >( imported.rtl_random_ex )( Seed );
	//}
}