#include <MinHook.h>
#include <gl/GL.h>
#include <xostr/xorstr.hpp>
#include <LazyImporter/lazy_importer.hpp>
#include <cstring>
#include "Hook.h"
#include "UI/GUI.h"

typedef BOOL(__stdcall* TWglSwapBuffers)(HDC hDc);

static bool is_init{};
static HWND wnd_handle{};
static void* p_swap_buffers{};
static TWglSwapBuffers origin_wglSwapBuffers{};

HWND g_hwnd = NULL;

static bool __stdcall wglSwapBuffers(HDC hDc);

bool Hook::init()
{
	if (is_init) return false;

	MH_Initialize();
	p_swap_buffers = (void*)LI_FN(GetProcAddress)(LI_FN(GetModuleHandleA)(xorstr_("opengl32.dll")), xorstr_("wglSwapBuffers"));
	if (!p_swap_buffers) return true;

	MH_CreateHook(p_swap_buffers, &wglSwapBuffers, (LPVOID*)&origin_wglSwapBuffers);
	MH_EnableHook(MH_ALL_HOOKS);
	is_init = true;
	return false;
}

void Hook::shutdown()
{
	if (!is_init) return;
	if (GUI::getIsInit()) GUI::shutdown();
	MH_DisableHook(MH_ALL_HOOKS);
	MH_RemoveHook(MH_ALL_HOOKS);
	is_init = false;
}

bool Hook::getIsInit()
{
	return is_init;
}

bool __stdcall wglSwapBuffers(HDC hDc)
{
	HWND current_wnd = LI_FN(WindowFromDC)(hDc);
	
	if (!wnd_handle && current_wnd && LI_FN(IsWindow)(current_wnd))
	{
		char current_class[128]{};
		LI_FN(GetClassNameA)(current_wnd, current_class, sizeof(current_class));
		
		if (std::strcmp(current_class, xorstr_("BlueStacksApp")) == 0)
		{
			wnd_handle = current_wnd;
			g_hwnd = wnd_handle;
			GUI::init(wnd_handle);
		}
	}
	
	if (current_wnd == wnd_handle && GUI::getIsInit())
		GUI::draw();
	
	return origin_wglSwapBuffers(hDc);
}
