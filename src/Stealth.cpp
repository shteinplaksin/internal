#include "Stealth.h"
#include <winternl.h>
#include <LazyImporter/lazy_importer.hpp>

typedef struct _LDR_MODULE {
	LIST_ENTRY InLoadOrderModuleList;
	LIST_ENTRY InMemoryOrderModuleList;
	LIST_ENTRY InInitializationOrderModuleList;
	PVOID BaseAddress;
	PVOID EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	ULONG Flags;
	SHORT LoadCount;
	SHORT TlsIndex;
	LIST_ENTRY HashTableEntry;
	ULONG TimeDateStamp;
} LDR_MODULE, *PLDR_MODULE;

typedef NTSTATUS (NTAPI *pNtQueryInformationProcess)(HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);

void Stealth::ErasePEHeader(HINSTANCE hModule)
{
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hModule;
	PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((LPBYTE)hModule + pDosHeader->e_lfanew);
	
	if (pNTHeader->Signature != IMAGE_NT_SIGNATURE)
		return;

	if (pNTHeader->FileHeader.SizeOfOptionalHeader)
	{
		DWORD dwOldProtect = 0;
		WORD wSizeOfOptionalHeader = pNTHeader->FileHeader.SizeOfOptionalHeader;
		
		if (LI_FN(VirtualProtect)(pDosHeader, wSizeOfOptionalHeader, PAGE_EXECUTE_READWRITE, &dwOldProtect))
		{
			LI_FN(RtlSecureZeroMemory)(pDosHeader, wSizeOfOptionalHeader);
			LI_FN(VirtualProtect)(pDosHeader, wSizeOfOptionalHeader, dwOldProtect, &dwOldProtect);
		}
	}
}

bool Stealth::RemoveFromPEB(HINSTANCE hModule)
{
	auto NtQueryInformationProcess = LI_FN(NtQueryInformationProcess).cached();
	
	PROCESS_BASIC_INFORMATION pbi;
	ULONG ulRetLen = 0;
	
	NTSTATUS status = NtQueryInformationProcess(LI_FN(GetCurrentProcess)(), ProcessBasicInformation, &pbi, sizeof(pbi), &ulRetLen);
	
	if (status != 0)
		return false;

	PEB* pPeb = pbi.PebBaseAddress;
	if (!pPeb)
		return false;

	PLDR_MODULE pMod = (PLDR_MODULE)pPeb->Ldr->InLoadOrderModuleList.Flink;
	PLDR_MODULE pStartMod = pMod;

	do
	{
		if (pMod->BaseAddress == hModule)
		{
			pMod->InLoadOrderModuleList.Blink->Flink = pMod->InLoadOrderModuleList.Flink;
			pMod->InLoadOrderModuleList.Flink->Blink = pMod->InLoadOrderModuleList.Blink;

			pMod->InInitializationOrderModuleList.Blink->Flink = pMod->InInitializationOrderModuleList.Flink;
			pMod->InInitializationOrderModuleList.Flink->Blink = pMod->InInitializationOrderModuleList.Blink;

			pMod->InMemoryOrderModuleList.Blink->Flink = pMod->InMemoryOrderModuleList.Flink;
			pMod->InMemoryOrderModuleList.Flink->Blink = pMod->InMemoryOrderModuleList.Blink;
			
			return true;
		}

		pMod = (PLDR_MODULE)pMod->InLoadOrderModuleList.Flink;
	} while (pMod != pStartMod);

	return false;
}

bool Stealth::CheckDebugger()
{
	if (LI_FN(IsDebuggerPresent)())
		return true;

	BOOL bRemoteDebugger = FALSE;
	LI_FN(CheckRemoteDebuggerPresent)(LI_FN(GetCurrentProcess)(), &bRemoteDebugger);
	
	return bRemoteDebugger != FALSE;
}

void Stealth::InitStealth(HINSTANCE hModule)
{
	if (CheckDebugger())
		return;

	ErasePEHeader(hModule);
	RemoveFromPEB(hModule);
}

