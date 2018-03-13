#pragma once

#include <ntddk.h>

#define SystemModuleInformation 11

typedef NTSTATUS(__fastcall *MiProcessLoaderEntry)(PVOID pDriverSection, int bLoad);

typedef NTSTATUS(*NTQUERYSYSTEMINFORMATION)(
	IN ULONG SystemInformationClass,
	OUT PVOID   SystemInformation,
	IN ULONG_PTR    SystemInformationLength,
	OUT PULONG_PTR  ReturnLength OPTIONAL);

typedef struct _SYSTEM_MODULE_INFORMATION{
	HANDLE Section;
	PVOID MappedBase;
	PVOID Base;
	ULONG Size;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT PathLength;
	CHAR ImageName[256];
} SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegString);
VOID Unload(PDRIVER_OBJECT DriverObject);
ULONG_PTR Get_MiProcessLoaderEntry();
VOID _fastcall HideDriver(PVOID x);
ULONG JudgeLoadDriver();