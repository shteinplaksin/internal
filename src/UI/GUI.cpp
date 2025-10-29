#include "GUI.h"
#include "Theme.h"
#include "Tabs.h"
#include <Windows.h>
#include <xostr/xorstr.hpp>
#include <LazyImporter/lazy_importer.hpp>
#include <cstring>
#include <imgui/imgui.cpp>
#include <imgui/imgui_draw.cpp>
#include <imgui/imgui_tables.cpp>
#include <imgui/imgui_widgets.cpp>
#include <imgui/backends/imgui_impl_win32.cpp>
#include <imgui/backends/imgui_impl_opengl3.cpp>

static bool is_init = false;
static bool do_draw = true;
extern HWND g_hwnd;

bool GUI::init(HWND wnd_handle)
{
	if (is_init) return false;

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImFont* font = io.Fonts->AddFontFromFileTTF(xorstr_("C:\\Windows\\Fonts\\consola.ttf"), 15.0f);
	io.FontDefault = font ? font : io.Fonts->AddFontDefault();

	ImGui::StyleColorsDark();
	applyDarkOrangeTheme();
	ImGui_ImplWin32_Init(wnd_handle);
	ImGui_ImplOpenGL3_Init();
	is_init = true;
	return false;
}

void GUI::shutdown()
{
	if (!is_init) return;
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	is_init = false;
}

void GUI::draw()
{
	if (!g_hwnd || !LI_FN(IsWindow)(g_hwnd)) return;
	
	static bool insert_was_pressed = false;
	bool insert_is_pressed = (LI_FN(GetAsyncKeyState)(VK_INSERT) & 0x8000) != 0;
	if (insert_is_pressed && !insert_was_pressed) do_draw = !do_draw;
	insert_was_pressed = insert_is_pressed;
	
	ImGui_ImplOpenGL3_NewFrame();
	ImGuiIO& io = ImGui::GetIO();
	
	static LARGE_INTEGER frequency = {0}, last_time = {0};
	if (frequency.QuadPart == 0)
	{
		LI_FN(QueryPerformanceFrequency)(&frequency);
		LI_FN(QueryPerformanceCounter)(&last_time);
	}
	
	LARGE_INTEGER current_time;
	LI_FN(QueryPerformanceCounter)(&current_time);
	io.DeltaTime = (float)(current_time.QuadPart - last_time.QuadPart) / (float)frequency.QuadPart;
	last_time = current_time;
	if (io.DeltaTime <= 0.0f || io.DeltaTime > 1.0f) io.DeltaTime = 1.0f / 60.0f;
	
	RECT rect;
	if (LI_FN(GetClientRect)(g_hwnd, &rect))
	{
		int width = rect.right - rect.left;
		int height = rect.bottom - rect.top;
		if (width > 0 && height > 0)
			io.DisplaySize = ImVec2((float)width, (float)height);
	}
	
	if (do_draw)
	{
		POINT cursor_pos;
		if (LI_FN(GetCursorPos)(&cursor_pos))
		{
			io.MousePos = LI_FN(ScreenToClient)(g_hwnd, &cursor_pos) 
				? ImVec2((float)cursor_pos.x, (float)cursor_pos.y) 
				: ImVec2(-FLT_MAX, -FLT_MAX);
		}
		
		int vk_buttons[3] = {VK_LBUTTON, VK_RBUTTON, VK_MBUTTON};
		for (int i = 0; i < 3; i++)
			io.MouseDown[i] = (LI_FN(GetAsyncKeyState)(vk_buttons[i]) & 0x8000) != 0;
		
		io.KeyCtrl = (LI_FN(GetAsyncKeyState)(VK_CONTROL) & 0x8000) != 0;
		io.KeyShift = (LI_FN(GetAsyncKeyState)(VK_SHIFT) & 0x8000) != 0;
		io.KeyAlt = (LI_FN(GetAsyncKeyState)(VK_MENU) & 0x8000) != 0;
		io.KeySuper = false;
	}
	else
	{
		io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
		memset(io.MouseDown, 0, sizeof(io.MouseDown));
		io.KeyCtrl = io.KeyShift = io.KeyAlt = io.KeySuper = false;
	}
	
	io.MouseWheel = io.MouseWheelH = 0.0f;
	
	ImGui::NewFrame();
	if (do_draw) renderMainInterface();
	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool GUI::getIsInit() { return is_init; }
bool GUI::getDoDraw() { return do_draw; }
void GUI::setDoDraw(bool new_value) { do_draw = new_value; }
