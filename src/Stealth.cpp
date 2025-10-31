#include "Stealth.h"
#include <winternl.h>
#include <LazyImporter/lazy_importer.hpp>
#include <cstring>
#include <cctype>

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

typedef struct _PEB_LDR_DATA_NT {
	ULONG Length;
	UCHAR Initialized;
	PVOID SsHandle;
	LIST_ENTRY InLoadOrderModuleList;
	LIST_ENTRY InMemoryOrderModuleList;
	LIST_ENTRY InInitializationOrderModuleList;
} PEB_LDR_DATA_NT, *PPEB_LDR_DATA_NT;

namespace
{
	bool IsBlueStacksProcess()
	{
		__try
		{
			char module_path[MAX_PATH]{};
			DWORD length = LI_FN(GetModuleFileNameA)(nullptr, module_path, MAX_PATH);
			if (!length || length >= MAX_PATH)
				return false;

			for (DWORD i = 0; i < length; ++i)
				module_path[i] = static_cast<char>(::tolower(static_cast<unsigned char>(module_path[i])));

			return std::strstr(module_path, "bluestacks") != nullptr || std::strstr(module_path, "hd-player") != nullptr;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return false;
		}
	}
}

void Stealth::ErasePEHeader(HINSTANCE hModule)
{
	__try
	{
		if (!hModule)
			return;

		PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hModule;
		
		if (LI_FN(IsBadReadPtr)(pDosHeader, sizeof(IMAGE_DOS_HEADER)))
			return;
		
		if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
			return;
		
		if (pDosHeader->e_lfanew <= 0 || pDosHeader->e_lfanew > 0x1000)
			return;
		
		PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((LPBYTE)hModule + pDosHeader->e_lfanew);
		
		if (LI_FN(IsBadReadPtr)(pNTHeader, sizeof(IMAGE_NT_HEADERS)))
			return;
		
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
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return;
	}
}

bool Stealth::RemoveFromPEB(HINSTANCE hModule)
{
	__try
	{
		auto NtQueryInformationProcess_fn = LI_FN(NtQueryInformationProcess).cached();
	
		PROCESS_BASIC_INFORMATION pbi{};
		ULONG ulRetLen = 0;
	
		NTSTATUS status = NtQueryInformationProcess_fn(LI_FN(GetCurrentProcess)(), ProcessBasicInformation, &pbi, sizeof(pbi), &ulRetLen);
	
		if (status != 0 || !pbi.PebBaseAddress)
			return false;

		PEB* pPeb = pbi.PebBaseAddress;
		PPEB_LDR_DATA_NT pLdr = (PPEB_LDR_DATA_NT)pPeb->Ldr;
		if (!pLdr)
			return false;

		PLDR_MODULE pMod = (PLDR_MODULE)pLdr->InLoadOrderModuleList.Flink;
		if (!pMod)
			return false;

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
		} while (pMod && pMod != pStartMod);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return false;
	}

	return false;
}

bool Stealth::CheckDebugger()
{
	__try
	{
		if (LI_FN(IsDebuggerPresent)())
			return true;

		BOOL bRemoteDebugger = FALSE;
		LI_FN(CheckRemoteDebuggerPresent)(LI_FN(GetCurrentProcess)(), &bRemoteDebugger);
	
		return bRemoteDebugger != FALSE;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return false;
	}
}

void Stealth::InitStealth(HINSTANCE hModule)
{
	__try
	{
		if (CheckDebugger())
			return;

		ErasePEHeader(hModule);

		if (!IsBlueStacksProcess())
			RemoveFromPEB(hModule);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return;
	}
}
