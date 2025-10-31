#include "Tabs.h"
#include "Config.h"
#include "Theme.h"
#include <imgui/imgui.h>
#include <array>
#include <cstring>
#include <algorithm>
#include <Windows.h>

#if defined(_MSC_VER)
#define GUI_TRY __try
#define GUI_EXCEPT(x) __except(x)
#else
#define GUI_TRY try
#define GUI_EXCEPT(x) catch(...)
#endif

namespace GUI { void setDoDraw(bool new_value); }

enum class TabId : int { Rage = 0, Visuals, Legit, Misc, Settings, Info, Count };

struct TabDescriptor { TabId id; const char* category; const char* label; };

std::array<TabDescriptor, static_cast<std::size_t>(TabId::Count)> kTabs{
    TabDescriptor{ TabId::Rage, "Rage", "Ragebot" },
    TabDescriptor{ TabId::Visuals, "Semi-Safe", "Visuals" },
    TabDescriptor{ TabId::Legit, "Semi-Safe", "Legit" },
    TabDescriptor{ TabId::Misc, "Utility", "Misc" },
    TabDescriptor{ TabId::Settings, "Utility", "Settings" },
    TabDescriptor{ TabId::Info, "Utility", "Info" }
};

void renderMenuBar()
{
    GUI_TRY
    {
        using namespace UIConstants;
        if (!ImGui::BeginMenuBar()) return;

        ImGui::SetCursorPosY(0.0f);
        ImGui::SetCursorPosX(315.0f);
        const char* menu_title = "Internal";
        ImGui::TextUnformatted(menu_title);

        ImGui::SetCursorPosY(0.0f);
        ImGui::SetCursorPosX(605.0f);
        if (ImGui::Button("-", ImVec2(20.0f, 20.0f))) ImGui::SetWindowSize(ImVec2(kDefaultMenuWidth, 20.0f));
        ImGui::SameLine();
        if (ImGui::Button("+", ImVec2(20.0f, 20.0f))) ImGui::SetWindowSize(ImVec2(kDefaultMenuWidth, kDefaultMenuHeight));
        ImGui::SameLine();
        if (ImGui::Button("x", ImVec2(20.0f, 20.0f))) GUI::setDoDraw(false);

        ImGui::EndMenuBar();
    }
    GUI_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
    }
}

void renderStatusBar()
{
    using namespace UIConstants;
    const float y = ImGui::GetWindowHeight() - kStatusBarHeight;
    ImGui::SetCursorPos(ImVec2(10.0f, y));
    ImGui::Separator();
    ImGui::SetCursorPosX(10.0f);
    ImGui::TextUnformatted("Status:");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Menu Active");
}

void renderRageTab()
{
    ImGui::SeparatorText("Ragebot Core");
    ImGui::SetCursorPosX(10.0f); ImGui::Checkbox("Enable Ragebot", &g_rageSettings.enableRageBot);
    ImGui::SetCursorPosX(10.0f); ImGui::Checkbox("Enable Anti-Aim", &g_rageSettings.enableAntiAim);
    ImGui::SetCursorPosX(10.0f); ImGui::Checkbox("Auto-shoot", &g_rageSettings.autoshoot);

    ImGui::SetCursorPosX(40.0f);
    ImGui::PushItemWidth(120.0f);
    ImGui::SliderFloat("Hitchance##rage", &g_rageSettings.hitchance, 0.0f, 100.0f, "%.0f%%");
    ImGui::SetCursorPosX(40.0f);
    ImGui::SliderFloat("Resolver Strength##rage", &g_rageSettings.resolverStrength, 0.0f, 100.0f, "%.0f%%");
    ImGui::PopItemWidth();
}

void renderVisualsTab()
{
    ImGui::SeparatorText("ESP Settings");
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f));

    ImGui::SetCursorPosX(20.0f); ImGui::Checkbox("Box ESP", &g_visualSettings.box);
    ImGui::SameLine(); ImGui::SetCursorPosX(140.0f);
    ImGui::ColorEdit4("##BoxColor", g_visualSettings.boxColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreviewHalf);

    ImGui::SetCursorPosX(20.0f); ImGui::Checkbox("Skeleton ESP", &g_visualSettings.skeleton);
    ImGui::SameLine(); ImGui::SetCursorPosX(140.0f);
    ImGui::ColorEdit4("##SkeletonColor", g_visualSettings.skeletonColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreviewHalf);

    ImGui::SetCursorPosX(20.0f); ImGui::Checkbox("Glow", &g_visualSettings.glow);
    ImGui::SameLine(); ImGui::SetCursorPosX(140.0f);
    ImGui::ColorEdit4("##GlowColor", g_visualSettings.glowColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreviewHalf);

    ImGui::SetCursorPosX(20.0f); ImGui::Checkbox("Health Bar", &g_visualSettings.healthBar);
    ImGui::SameLine(); ImGui::SetCursorPosX(140.0f);
    ImGui::ColorEdit4("##HealthColor", g_visualSettings.healthColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreviewHalf);

    ImGui::SetCursorPosX(40.0f);
    ImGui::PushItemWidth(120.0f);
    ImGui::SliderFloat("Visual FOV", &g_visualSettings.visualFov, 10.0f, 200.0f);
    ImGui::PopItemWidth();
    ImGui::PopStyleVar();

    g_anyEspEnabled = g_visualSettings.box || g_visualSettings.skeleton || g_visualSettings.glow || g_visualSettings.healthBar;
}

void renderLegitTab()
{
    ImGui::SeparatorText("Legitbot Settings");
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f));

    ImGui::SetCursorPosX(20.0f); ImGui::Checkbox("Enable Trigger", &g_legitSettings.enableTrigger);
    ImGui::SetCursorPosX(20.0f); ImGui::Checkbox("Use Smooth", &g_legitSettings.useSmooth);

    ImGui::SetCursorPosX(40.0f);
    ImGui::PushItemWidth(120.0f);
    ImGui::SliderFloat("Smooth##legit", &g_legitSettings.smoothValue, 1.0f, 10.0f);
    ImGui::SetCursorPosX(40.0f);
    ImGui::SliderFloat("Legit FOV", &g_legitSettings.fov, 1.0f, 10.0f);
    ImGui::PopItemWidth();

    ImGui::SeparatorText("Hitboxes");
    ImGui::SetCursorPosX(20.0f); ImGui::Checkbox("Head", &g_legitSettings.boneHead);
    ImGui::SameLine(120.0f); ImGui::Checkbox("Neck", &g_legitSettings.boneNeck);
    ImGui::SetCursorPosX(20.0f); ImGui::Checkbox("Chest", &g_legitSettings.boneChest);
    ImGui::SameLine(120.0f); ImGui::Checkbox("Stomach", &g_legitSettings.boneStomach);

    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Выберите области для легит-аима и настройте точность.");
    ImGui::PopStyleVar();
}

void renderMiscTab()
{
    ImGui::SeparatorText("Utility");
    ImGui::SetCursorPosX(10.0f); ImGui::Checkbox("Bunny Hop", &g_miscSettings.bunnyHop);
    ImGui::SetCursorPosX(10.0f); ImGui::Checkbox("No Flash", &g_miscSettings.noFlash);
    ImGui::SetCursorPosX(10.0f); ImGui::Checkbox("Radar Hack", &g_miscSettings.radarHack);

    ImGui::SetCursorPosX(40.0f);
    ImGui::PushItemWidth(120.0f);
    ImGui::SliderFloat("Night Mode", &g_miscSettings.nightMode, 0.0f, 1.0f);
    ImGui::PopItemWidth();
}

void renderSettingsTab()
{
    ImGui::SeparatorText("Configuration");
    ImGui::SetCursorPosX(40.0f);
    ImGui::PushItemWidth(120.0f);
    const char* configurations[]{ "Default", "Rage", "Legit", "Streaming" };
    ImGui::Combo("Config Preset", &g_settingsData.selectedConfig, configurations, IM_ARRAYSIZE(configurations));
    ImGui::PopItemWidth();

    ImGui::SetCursorPosX(20.0f); ImGui::Checkbox("Auto Save", &g_settingsData.autoSave);
    if (ImGui::Button("Save##settings", ImVec2(140.0f, 35.0f))) {}
    ImGui::SameLine();
    if (ImGui::Button("Load##settings", ImVec2(140.0f, 35.0f))) {}
}

void renderInfoTab()
{
    ImGui::SeparatorText("Information");
    const char* info_line1 = "AssaultCube Internal build - Dark Orange UI";
    const char* info_line2 = "Status: Running inside AssaultCube (x86).";
    ImGui::SetCursorPosX(10.0f); ImGui::TextUnformatted(info_line1);
    ImGui::SetCursorPosX(10.0f); ImGui::TextUnformatted(info_line2);
    ImGui::Spacing();

    ImGui::SetCursorPosX(10.0f);
    if (ImGui::Button("Check for updates", ImVec2(200.0f, 40.0f))) {}

    ImGui::SetCursorPosX(10.0f);
    if (ImGui::Button("Exit", ImVec2(200.0f, 40.0f))) g_infoData.confirmExit = true;

    if (g_infoData.confirmExit)
    {
        ImGui::OpenPopup("Confirm Exit");
        g_infoData.confirmExit = false;
    }

    if (ImGui::BeginPopupModal("Confirm Exit", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextWrapped("Close the overlay and shut down?");
        if (ImGui::Button("Yes", ImVec2(80.0f, 0.0f))) { GUI::setDoDraw(false); ImGui::CloseCurrentPopup(); }
        ImGui::SameLine();
        if (ImGui::Button("No", ImVec2(80.0f, 0.0f))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

static void renderTabContent()
{
    GUI_TRY
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
    GUI_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
    }
}

void renderSidebar(float contentHeight)
{
    bool child_begun = false;
    GUI_TRY
    {
        using namespace UIConstants;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + kSidebarOffset);
        ImGui::BeginChild("LeftTabs", ImVec2(kSidebarWidth, contentHeight), false);
        child_begun = true;

        ImGui::SetCursorPosY(kSidebarLogoY);
        ImGui::SetCursorPosX(kSidebarLogoX);
        ImGui::SetWindowFontScale(1.1f);
        const char* logo_text = "isvqq";
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
        child_begun = false;
    }
    GUI_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
        if (child_begun) ImGui::EndChild();
    }
}

void renderMainPanel(float contentHeight)
{
    bool child_begun = false;
    GUI_TRY
    {
        using namespace UIConstants;
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + kMainPanelOffset);
        ImGui::BeginChild("MainPanel", ImVec2(0.0f, contentHeight), false);
        child_begun = true;
        renderTabContent();
        ImGui::EndChild();
        child_begun = false;
    }
    GUI_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
        if (child_begun) ImGui::EndChild();
    }
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

    const char* window_title = "CS2";
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
