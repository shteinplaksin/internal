#include "Tabs.h"
#include "Config.h"
#include "Theme.h"
#include <imgui/imgui.h>
#include <xostr/xorstr.hpp>
#include <array>
#include <cstring>
#include <algorithm>

namespace GUI { void setDoDraw(bool new_value); }

enum class TabId : int { Rage = 0, Visuals, Legit, Misc, Settings, Info, Count };

struct TabDescriptor { TabId id; const char* category; const char* label; };

std::array<TabDescriptor, static_cast<std::size_t>(TabId::Count)> kTabs{
    TabDescriptor{ TabId::Rage, xorstr_("Rage"), xorstr_("Ragebot") },
    TabDescriptor{ TabId::Visuals, xorstr_("Semi-Safe"), xorstr_("Visuals") },
    TabDescriptor{ TabId::Legit, xorstr_("Semi-Safe"), xorstr_("Legit") },
    TabDescriptor{ TabId::Misc, xorstr_("Utility"), xorstr_("Misc") },
    TabDescriptor{ TabId::Settings, xorstr_("Utility"), xorstr_("Settings") },
    TabDescriptor{ TabId::Info, xorstr_("Utility"), xorstr_("Info") }
};

void renderMenuBar()
{
    using namespace UIConstants;
    if (!ImGui::BeginMenuBar()) return;

    ImGui::SetCursorPosY(0.0f);
    ImGui::SetCursorPosX(315.0f);
    const char* menu_title = xorstr_("Internal");
    ImGui::TextUnformatted(menu_title);

    ImGui::SetCursorPosY(0.0f);
    ImGui::SetCursorPosX(605.0f);
    if (ImGui::Button(xorstr_("-"), ImVec2(20.0f, 20.0f))) ImGui::SetWindowSize(ImVec2(kDefaultMenuWidth, 20.0f));
    ImGui::SameLine();
    if (ImGui::Button(xorstr_("+"), ImVec2(20.0f, 20.0f))) ImGui::SetWindowSize(ImVec2(kDefaultMenuWidth, kDefaultMenuHeight));
    ImGui::SameLine();
    if (ImGui::Button(xorstr_("x"), ImVec2(20.0f, 20.0f))) GUI::setDoDraw(false);

    ImGui::EndMenuBar();
}

void renderStatusBar()
{
    using namespace UIConstants;
    const float y = ImGui::GetWindowHeight() - kStatusBarHeight;
    ImGui::SetCursorPos(ImVec2(10.0f, y));
    ImGui::Separator();
    ImGui::SetCursorPosX(10.0f);
    ImGui::TextUnformatted(xorstr_("Status:"));
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), xorstr_("Menu Active"));
}

void renderRageTab()
{
    ImGui::SeparatorText(xorstr_("Ragebot Core"));
    ImGui::SetCursorPosX(10.0f); ImGui::Checkbox(xorstr_("Enable Ragebot"), &g_rageSettings.enableRageBot);
    ImGui::SetCursorPosX(10.0f); ImGui::Checkbox(xorstr_("Enable Anti-Aim"), &g_rageSettings.enableAntiAim);
    ImGui::SetCursorPosX(10.0f); ImGui::Checkbox(xorstr_("Auto-shoot"), &g_rageSettings.autoshoot);

    ImGui::SetCursorPosX(40.0f);
    ImGui::PushItemWidth(120.0f);
    ImGui::SliderFloat(xorstr_("Hitchance##rage"), &g_rageSettings.hitchance, 0.0f, 100.0f, xorstr_("%.0f%%"));
    ImGui::SetCursorPosX(40.0f);
    ImGui::SliderFloat(xorstr_("Resolver Strength##rage"), &g_rageSettings.resolverStrength, 0.0f, 100.0f, xorstr_("%.0f%%"));
    ImGui::PopItemWidth();
}

void renderVisualsTab()
{
    ImGui::SeparatorText(xorstr_("ESP Settings"));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f));

    ImGui::SetCursorPosX(20.0f); ImGui::Checkbox(xorstr_("Box ESP"), &g_visualSettings.box);
    ImGui::SameLine(); ImGui::SetCursorPosX(140.0f);
    ImGui::ColorEdit4(xorstr_("##BoxColor"), g_visualSettings.boxColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreviewHalf);

    ImGui::SetCursorPosX(20.0f); ImGui::Checkbox(xorstr_("Skeleton ESP"), &g_visualSettings.skeleton);
    ImGui::SameLine(); ImGui::SetCursorPosX(140.0f);
    ImGui::ColorEdit4(xorstr_("##SkeletonColor"), g_visualSettings.skeletonColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreviewHalf);

    ImGui::SetCursorPosX(20.0f); ImGui::Checkbox(xorstr_("Glow"), &g_visualSettings.glow);
    ImGui::SameLine(); ImGui::SetCursorPosX(140.0f);
    ImGui::ColorEdit4(xorstr_("##GlowColor"), g_visualSettings.glowColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreviewHalf);

    ImGui::SetCursorPosX(20.0f); ImGui::Checkbox(xorstr_("Health Bar"), &g_visualSettings.healthBar);
    ImGui::SameLine(); ImGui::SetCursorPosX(140.0f);
    ImGui::ColorEdit4(xorstr_("##HealthColor"), g_visualSettings.healthColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreviewHalf);

    ImGui::SetCursorPosX(40.0f);
    ImGui::PushItemWidth(120.0f);
    ImGui::SliderFloat(xorstr_("Visual FOV"), &g_visualSettings.visualFov, 10.0f, 200.0f);
    ImGui::PopItemWidth();
    ImGui::PopStyleVar();

    g_anyEspEnabled = g_visualSettings.box || g_visualSettings.skeleton || g_visualSettings.glow || g_visualSettings.healthBar;
}

void renderLegitTab()
{
    ImGui::SeparatorText(xorstr_("Legitbot Settings"));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f));

    ImGui::SetCursorPosX(20.0f); ImGui::Checkbox(xorstr_("Enable Trigger"), &g_legitSettings.enableTrigger);
    ImGui::SetCursorPosX(20.0f); ImGui::Checkbox(xorstr_("Use Smooth"), &g_legitSettings.useSmooth);

    ImGui::SetCursorPosX(40.0f);
    ImGui::PushItemWidth(120.0f);
    ImGui::SliderFloat(xorstr_("Smooth##legit"), &g_legitSettings.smoothValue, 1.0f, 10.0f);
    ImGui::SetCursorPosX(40.0f);
    ImGui::SliderFloat(xorstr_("Legit FOV"), &g_legitSettings.fov, 1.0f, 10.0f);
    ImGui::PopItemWidth();

    ImGui::SeparatorText(xorstr_("Hitboxes"));
    ImGui::SetCursorPosX(20.0f); ImGui::Checkbox(xorstr_("Head"), &g_legitSettings.boneHead);
    ImGui::SameLine(120.0f); ImGui::Checkbox(xorstr_("Neck"), &g_legitSettings.boneNeck);
    ImGui::SetCursorPosX(20.0f); ImGui::Checkbox(xorstr_("Chest"), &g_legitSettings.boneChest);
    ImGui::SameLine(120.0f); ImGui::Checkbox(xorstr_("Stomach"), &g_legitSettings.boneStomach);

    ImGui::TextDisabled(xorstr_("(?)"));
    if (ImGui::IsItemHovered()) ImGui::SetTooltip(xorstr_("Выберите области для легит-аима и настройте точность."));
    ImGui::PopStyleVar();
}

void renderMiscTab()
{
    ImGui::SeparatorText(xorstr_("Utility"));
    ImGui::SetCursorPosX(10.0f); ImGui::Checkbox(xorstr_("Bunny Hop"), &g_miscSettings.bunnyHop);
    ImGui::SetCursorPosX(10.0f); ImGui::Checkbox(xorstr_("No Flash"), &g_miscSettings.noFlash);
    ImGui::SetCursorPosX(10.0f); ImGui::Checkbox(xorstr_("Radar Hack"), &g_miscSettings.radarHack);

    ImGui::SetCursorPosX(40.0f);
    ImGui::PushItemWidth(120.0f);
    ImGui::SliderFloat(xorstr_("Night Mode"), &g_miscSettings.nightMode, 0.0f, 1.0f);
    ImGui::PopItemWidth();
}

void renderSettingsTab()
{
    ImGui::SeparatorText(xorstr_("Configuration"));
    ImGui::SetCursorPosX(40.0f);
    ImGui::PushItemWidth(120.0f);
    const char* configurations[]{ xorstr_("Default"), xorstr_("Rage"), xorstr_("Legit"), xorstr_("Streaming") };
    ImGui::Combo(xorstr_("Config Preset"), &g_settingsData.selectedConfig, configurations, IM_ARRAYSIZE(configurations));
    ImGui::PopItemWidth();

    ImGui::SetCursorPosX(20.0f); ImGui::Checkbox(xorstr_("Auto Save"), &g_settingsData.autoSave);
    if (ImGui::Button(xorstr_("Save##settings"), ImVec2(140.0f, 35.0f))) {}
    ImGui::SameLine();
    if (ImGui::Button(xorstr_("Load##settings"), ImVec2(140.0f, 35.0f))) {}
}

void renderInfoTab()
{
    ImGui::SeparatorText(xorstr_("Information"));
    const char* info_line1 = xorstr_("AssaultCube Internal build - Dark Orange UI");
    const char* info_line2 = xorstr_("Status: Running inside AssaultCube (x86).");
    ImGui::SetCursorPosX(10.0f); ImGui::TextUnformatted(info_line1);
    ImGui::SetCursorPosX(10.0f); ImGui::TextUnformatted(info_line2);
    ImGui::Spacing();

    ImGui::SetCursorPosX(10.0f);
    if (ImGui::Button(xorstr_("Check for updates"), ImVec2(200.0f, 40.0f))) {}

    ImGui::SetCursorPosX(10.0f);
    if (ImGui::Button(xorstr_("Exit"), ImVec2(200.0f, 40.0f))) g_infoData.confirmExit = true;

    if (g_infoData.confirmExit)
    {
        ImGui::OpenPopup(xorstr_("Confirm Exit"));
        g_infoData.confirmExit = false;
    }

    if (ImGui::BeginPopupModal(xorstr_("Confirm Exit"), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextWrapped(xorstr_("Close the overlay and shut down?"));
        if (ImGui::Button(xorstr_("Yes"), ImVec2(80.0f, 0.0f))) { GUI::setDoDraw(false); ImGui::CloseCurrentPopup(); }
        ImGui::SameLine();
        if (ImGui::Button(xorstr_("No"), ImVec2(80.0f, 0.0f))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

static void renderTabContent()
{
    switch (static_cast<TabId>(g_activeTab))
    {
        case TabId::Rage: renderRageTab(); break;
        case TabId::Visuals: renderVisualsTab(); break;
        case TabId::Legit: renderLegitTab(); break;
        case TabId::Misc: renderMiscTab(); break;
        case TabId::Settings: renderSettingsTab(); break;
        case TabId::Info: renderInfoTab(); break;
        default: break;
    }
}

void renderSidebar(float contentHeight)
{
    using namespace UIConstants;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + kSidebarOffset);
    ImGui::BeginChild(xorstr_("LeftTabs"), ImVec2(kSidebarWidth, contentHeight), false);

    ImGui::SetCursorPosY(kSidebarLogoY);
    ImGui::SetCursorPosX(kSidebarLogoX);
    ImGui::SetWindowFontScale(1.1f);
    const char* logo_text = xorstr_("isvqq");
    ImGui::TextUnformatted(logo_text);
    ImGui::SetWindowFontScale(1.0f);

    const ImVec4 activeColor{1.0f, 0.5f, 0.0f, 0.4f};
    const ImVec4 activeHoverColor{1.0f, 0.6f, 0.0f, 0.6f};
    const ImVec4 activePressColor{1.0f, 0.4f, 0.0f, 0.6f};

    const char* currentCategory{nullptr};
    for (std::size_t i = 0; i < kTabs.size(); ++i)
    {
        const TabDescriptor& tab = kTabs[i];
        if (!currentCategory || std::strcmp(currentCategory, tab.category) != 0)
        {
            currentCategory = tab.category;
            ImGui::SeparatorText(currentCategory);
        }

        ImGui::SetCursorPosX(kSidebarOffset);
        const bool selected = (g_activeTab == static_cast<int>(tab.id));
        if (selected)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, activeColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, activeHoverColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, activePressColor);
        }

        if (ImGui::Button(tab.label, ImVec2(kSidebarButtonWidth, kSidebarButtonHeight)))
            g_activeTab = static_cast<int>(tab.id);

        if (selected) ImGui::PopStyleColor(3);
    }

    ImGui::EndChild();
}

void renderMainPanel(float contentHeight)
{
    using namespace UIConstants;
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + kMainPanelOffset);
    ImGui::BeginChild(xorstr_("MainPanel"), ImVec2(0.0f, contentHeight), false);
    renderTabContent();
    ImGui::EndChild();
}

void renderMainInterface()
{
    using namespace UIConstants;
    ImGuiIO& io = ImGui::GetIO();
    const ImVec2 screenSize = io.DisplaySize;
    const ImVec2 defaultSize(kDefaultMenuWidth, kDefaultMenuHeight);
    const ImVec2 minSize(kMinMenuWidth, kMinMenuHeight);
    const ImVec2 centerPos((screenSize.x - defaultSize.x) * 0.5f, (screenSize.y - defaultSize.y) * 0.5f);

    ImGui::SetNextWindowSize(defaultSize, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(minSize, ImVec2(FLT_MAX, FLT_MAX));
    ImGui::SetNextWindowPos(centerPos, ImGuiCond_FirstUseEver);

    constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

    const char* window_title = xorstr_("CS2");
    if (ImGui::Begin(window_title, nullptr, windowFlags))
    {
        const ImVec2 windowPos = ImGui::GetWindowPos();
        const ImVec2 windowSize = ImGui::GetWindowSize();
        ImGui::GetWindowDrawList()->AddLine(ImVec2(windowPos.x, windowPos.y), ImVec2(windowPos.x + windowSize.x, windowPos.y), IM_COL32(255, 128, 0, 255), 1.0f);

        renderMenuBar();
        const float contentHeight = std::max(0.0f, ImGui::GetContentRegionAvail().y - kStatusBarHeight);
        renderSidebar(contentHeight);
        renderMainPanel(contentHeight);
        renderStatusBar();
    }

    ImGui::End();
}
