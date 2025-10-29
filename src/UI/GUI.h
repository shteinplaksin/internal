#ifndef GUI_H_
#define GUI_H_

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>

#undef min
#undef max

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_win32.h>
#include <imgui/backends/imgui_impl_opengl3.h>

namespace GUI
{
	// инициализация
	bool init(HWND wnd_handle);
	void shutdown();
	bool getIsInit();

	// отрисовка
	void draw();

	// видимость
	bool getDoDraw();
	void setDoDraw(bool new_value);
}

#endif

