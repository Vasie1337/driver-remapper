#include <ntifs.h>
#include <windef.h>
#include <cstdint>
#include <intrin.h>
#include <ntimage.h>
#include <ntstatus.h>
#include <ntstrsafe.h>

#define printf(text, ...) DbgPrintEx(DPFLTR_IHVBUS_ID, 0, ("[TestDrv]: " text), ##__VA_ARGS__)

NTSTATUS manual_mapped_entry(PVOID a1, PVOID a2)
{
	printf("Hello from testdriver\n");
	return 0x1338;
}