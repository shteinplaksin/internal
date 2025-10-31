#include <MinHook.h>
#include <gl/GL.h>
#include <xostr/xorstr.hpp>
#include <LazyImporter/lazy_importer.hpp>
#include <cstring>
#include "Hook.h"
#include "UI/GUI.h"

typedef BOOL(__stdcall* TWglSwapBuffers)(HDC hDc);

// Obfuscated MinHook function wrappers - use function pointers to hide direct calls
typedef MH_STATUS(WINAPI* PFN_MH_Initialize)(VOID);
typedef MH_STATUS(WINAPI* PFN_MH_CreateHook)(LPVOID, LPVOID, LPVOID*);
typedef MH_STATUS(WINAPI* PFN_MH_EnableHook)(LPVOID);
typedef MH_STATUS(WINAPI* PFN_MH_DisableHook)(LPVOID);
typedef MH_STATUS(WINAPI* PFN_MH_RemoveHook)(LPVOID);

static PFN_MH_Initialize pfn_mh_init = MH_Initialize;
static PFN_MH_CreateHook pfn_mh_create = MH_CreateHook;
static PFN_MH_EnableHook pfn_mh_enable = MH_EnableHook;
static PFN_MH_DisableHook pfn_mh_disable = MH_DisableHook;
static PFN_MH_RemoveHook pfn_mh_remove = MH_RemoveHook;

static bool is_init{};
static HWND wnd_handle{};
static void* p_swap_buffers{};
static TWglSwapBuffers origin_wglSwapBuffers{};

HWND g_hwnd = NULL;

static bool __stdcall wglSwapBuffers(HDC hDc);

bool Hook::init()
{
    __try
    {
        if (is_init) return false;

        if (pfn_mh_init() != MH_OK) return true;

        const char* gl_dll = xorstr_("opengl32.dll");
        const char* swap_func = xorstr_("wglSwapBuffers");

        HMODULE gl_module = LI_FN(GetModuleHandleA)(gl_dll);
        if (!gl_module)
        {
            gl_module = LI_FN(LoadLibraryA)(gl_dll);
            if (!gl_module)
                return true;
        }

        p_swap_buffers = (void*)LI_FN(GetProcAddress)(gl_module, swap_func);
        if (!p_swap_buffers)
            return true;

        if (pfn_mh_create(p_swap_buffers, &wglSwapBuffers, (LPVOID*)&origin_wglSwapBuffers) != MH_OK)
            return true;

        if (pfn_mh_enable(MH_ALL_HOOKS) != MH_OK)
            return true;

        is_init = true;
        return false;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return true;
    }
}

void Hook::shutdown()
{
    if (!is_init) return;
    if (GUI::getIsInit()) GUI::shutdown();
    pfn_mh_disable(MH_ALL_HOOKS);
    if (p_swap_buffers) pfn_mh_remove(p_swap_buffers);
    is_init = false;
}

bool Hook::getIsInit()
{
    return is_init;
}

bool __stdcall wglSwapBuffers(HDC hDc)
{
    __try
    {
        HWND current_wnd = LI_FN(WindowFromDC)(hDc);
        
        if (!wnd_handle && current_wnd && LI_FN(IsWindow)(current_wnd))
        {
            char current_class[128]{};
            LI_FN(GetClassNameA)(current_wnd, current_class, sizeof(current_class));
            
            const char* target_class = xorstr_("BlueStacksApp");
            if (std::strcmp(current_class, target_class) == 0)
            {
                wnd_handle = current_wnd;
                g_hwnd = wnd_handle;
                GUI::init(wnd_handle);
            }
        }
        
        if (current_wnd == wnd_handle && GUI::getIsInit())
            GUI::draw();
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        wnd_handle = nullptr;
    }
    
    return origin_wglSwapBuffers(hDc);
}
