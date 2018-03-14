#include "m.h"

ULONG_PTR Get_MiProcessLoaderEntry()
{
	UNICODE_STRING EtwWriteStringName;
	ULONG_PTR EtwWriteStringAddress;
	PHYSICAL_ADDRESS pAddress;
	ULONG_PTR EndAddress;
	UCHAR *p;
	ULONG i;

	RtlInitUnicodeString(&EtwWriteStringName, L"EtwWriteString");
	EtwWriteStringAddress = (ULONG_PTR)MmGetSystemRoutineAddress(&EtwWriteStringName);
	DbgBreakPoint();
	EndAddress = EtwWriteStringAddress - 0x6000;
	char temp_code[] = "\x48\x89\x5C\x24\x08\x48\x89\x6C\x24\x18"
		"\x48\x89\x74\x24\x20\x57\x41\x54\x41\x55"
		"\x41\x56\x41\x57";
	for (auto i = 0; i < 0x6000; i++)
	{
		if (memcmp((char*)EndAddress + i, temp_code, 24) == 0)
		{
			return EndAddress + i;
		}
	}
	KdPrint(("not found\n"));
	return 0;
}

//判断当前Driver是否加载成功
ULONG JudgeLoadDriver()
{
	NTQUERYSYSTEMINFORMATION m_NtQuerySystemInformation = NULL;
	UNICODE_STRING NtQuerySystemInformation_Name;
	PSYSTEM_MODULE_INFORMATION ModuleEntry;
	ULONG_PTR RetLength,BaseAddr,EndAddr;
	ULONG ModuleNumbers, Index;
	NTSTATUS Status;
	PVOID Buffer;

	RtlInitUnicodeString(&NtQuerySystemInformation_Name, L"NtQuerySystemInformation");
	m_NtQuerySystemInformation = (NTQUERYSYSTEMINFORMATION)MmGetSystemRoutineAddress(&NtQuerySystemInformation_Name);
	if (m_NtQuerySystemInformation == NULL)
	{
		KdPrint(("获取NtQuerySystemInformation函数失败！\n"));
		return 1;
	}

	RetLength = 0;
	Status = m_NtQuerySystemInformation(SystemModuleInformation, NULL, 0, &RetLength);
	if (Status < 0 && Status != STATUS_INFO_LENGTH_MISMATCH)
	{
		KdPrint(("NtQuerySystemInformation调用失败！错误码是：%x\n", Status));
		return 1;
	}

	Buffer = ExAllocatePoolWithTag(NonPagedPool, RetLength, 'ytz');
	if (Buffer == NULL)
	{
		KdPrint(("分配内存失败！\n"));
		return 1;
	}

	Status = m_NtQuerySystemInformation(SystemModuleInformation, Buffer, RetLength, &RetLength);
	if (Status < 0)
	{
		KdPrint(("NtQuerySystemInformation调用失败！错误码是：%x\n", Status));
		return 1;
	}

	ModuleNumbers = *(ULONG*)Buffer;
	ModuleEntry = (PSYSTEM_MODULE_INFORMATION)((ULONG_PTR)Buffer + 8);
	for (Index = 0; Index < ModuleNumbers; ++Index)
	{
		BaseAddr = (ULONG_PTR)ModuleEntry->Base;
		EndAddr = BaseAddr + ModuleEntry->Size;
		if (BaseAddr <= (ULONG_PTR)JudgeLoadDriver && (ULONG_PTR)JudgeLoadDriver <= EndAddr)
		{
			KdPrint(("模块名称是：%s\n", ModuleEntry->ImageName));
			return 2;
		}
		++ModuleEntry;
	}

	return 0;
}

VOID Unload(PDRIVER_OBJECT DriverObject)
{
	KdPrint(("Unload Success!\n"));
}

VOID _fastcall HideDriver(PVOID x)
{
	MiProcessLoaderEntry m_MiProcessLoaderEntry = NULL;
	PDRIVER_OBJECT DriverObject;
	LARGE_INTEGER SleepTime;
	ULONG RetValue;

	m_MiProcessLoaderEntry = (MiProcessLoaderEntry)Get_MiProcessLoaderEntry();
	if (m_MiProcessLoaderEntry == NULL)
	{
		PsTerminateSystemThread(STATUS_SUCCESS);
		return;
	}

	SleepTime.QuadPart = -100 * 1000 * 1000 * 10;				//找不到则休眠10毫秒

	//如果是1说明内部调用函数出错，现在只可能是0没找到，2找到了
	while (TRUE)
	{
		RetValue = JudgeLoadDriver();
		if (RetValue == 1)
		{
			PsTerminateSystemThread(STATUS_SUCCESS);
			return;
		}
		else if (RetValue == 2)
			break;
		else
			KeDelayExecutionThread(KernelMode, 0, &SleepTime);
	}

	DriverObject = (PDRIVER_OBJECT)x;

	m_MiProcessLoaderEntry(DriverObject->DriverSection, 0);
	DriverObject->DriverSection = NULL;
	DriverObject->DriverStart = NULL;
	DriverObject->DriverSize = NULL;
	DriverObject->DriverUnload = NULL;
	DriverObject->DriverInit = NULL;
	DriverObject->DeviceObject = NULL;

	JudgeLoadDriver();

	//PsTerminateSystemThread(STATUS_SUCCESS);
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegString)
{
	HANDLE hThread;

	PsCreateSystemThread(&hThread, 0, NULL, NULL, NULL, HideDriver, (PVOID)DriverObject);
	ZwClose(hThread);

	return STATUS_SUCCESS;
}