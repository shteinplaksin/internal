#include <thread>
#include <chrono>
#include <Windows.h>
#include <xostr/xorstr.hpp>
#include <LazyImporter/lazy_importer.hpp>
#include "Hook.h"
#include "Stealth.h"

void __stdcall MainThread(HINSTANCE instance) 
{
	Stealth::InitStealth(instance);

	if (Hook::init())
		goto shutdown;

	while (!LI_FN(GetAsyncKeyState)(VK_END))
		std::this_thread::sleep_for(std::chrono::milliseconds(25));

shutdown:
	Hook::shutdown();
	LI_FN(FreeLibrary)(instance);
}

bool __stdcall DllMain(HINSTANCE instance, DWORD reason, LPVOID p_reserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		LI_FN(DisableThreadLibraryCalls)(instance);
		if (static std::thread main_thread([instance] { MainThread(instance); }); main_thread.joinable())
			main_thread.detach();
	}
	return true;
}
