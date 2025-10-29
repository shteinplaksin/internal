#include "Theme.h"
#include <imgui/imgui.h>

void applyDarkOrangeTheme()
{
	ImGuiStyle& style = ImGui::GetStyle();

	style.WindowPadding = ImVec2(8.0f, 8.0f);
	style.FramePadding = ImVec2(4.0f, 3.0f);
	style.CellPadding = ImVec2(4.0f, 2.0f);
	style.ItemSpacing = ImVec2(8.0f, 4.0f);
	style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
	style.ScrollbarSize = 14.0f;

	style.WindowRounding = 6.0f;
	style.ChildRounding = 6.0f;
	style.FrameRounding = 3.0f;
	style.PopupRounding = 6.0f;
	style.ScrollbarRounding = 9.0f;
	style.GrabRounding = 3.0f;
	style.TabRounding = 4.0f;

	style.WindowBorderSize = style.ChildBorderSize = style.PopupBorderSize = 1.0f;
	style.FrameBorderSize = style.TabBorderSize = 0.0f;

	ImVec4* colors = style.Colors;
	colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.09f, 0.09f, 0.94f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
	colors[ImGuiCol_Border] = ImVec4(0.27f, 0.27f, 0.27f, 0.50f);
	colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.20f, 0.34f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.27f, 0.27f, 0.27f, 0.54f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.32f, 0.32f, 0.32f, 0.54f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.27f, 0.27f, 0.27f, 0.54f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.32f, 0.32f, 0.32f, 0.54f);
	colors[ImGuiCol_Separator] = ImVec4(0.27f, 0.27f, 0.27f, 0.50f);
	colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
}
