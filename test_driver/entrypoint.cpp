#include <ntifs.h>
#include <windef.h>
#include <cstdint>
#include <intrin.h>
#include <ntimage.h>
#include <ntstatus.h>
#include <ntstrsafe.h>

NTSTATUS manual_mapped_entry(PVOID a1, PVOID a2)
{
	return 0x1338;
}