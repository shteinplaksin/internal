#include <thread>
#include <chrono>
#include <Windows.h>
#include <xostr/xorstr.hpp>
#include <LazyImporter/lazy_importer.hpp>
#include "Hook.h"
#include "Stealth.h"

DWORD WINAPI MainThread(LPVOID param)
{
    HINSTANCE instance = static_cast<HINSTANCE>(param);

    __try
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        Stealth::InitStealth(instance);

        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        if (Hook::init())
            goto shutdown;

        while (!LI_FN(GetAsyncKeyState)(VK_END))
            std::this_thread::sleep_for(std::chrono::milliseconds(25));

    shutdown:
        Hook::shutdown();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        LI_FN(FreeLibraryAndExitThread)(instance, 0);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        LI_FN(FreeLibraryAndExitThread)(instance, 0);
    }

    return 0;
}

bool __stdcall DllMain(HINSTANCE instance, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        LI_FN(DisableThreadLibraryCalls)(instance);

        __try
        {
            HANDLE thread_handle = LI_FN(CreateThread)(nullptr, 0, MainThread, instance, 0, nullptr);
            if (thread_handle)
                LI_FN(CloseHandle)(thread_handle);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            return false;
        }
    }

    return true;
}
