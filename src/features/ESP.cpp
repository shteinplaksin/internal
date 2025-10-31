#include "ESP.h"
#include "Offsets.h"
#include "../UI/Config.h"
#include <Windows.h>
#include <SehCompat.h>
#include <LazyImporter/lazy_importer.hpp>
#include <xostr/xorstr.hpp>
#include <imgui/imgui.h>
#include <algorithm>
#include <cstdint>
#include <cstring>

namespace Features
{
    namespace Esp
    {
        namespace
        {
            struct Vec3
            {
                float x;
                float y;
                float z;
            };

            struct Matrix4x4
            {
                float data[16];
            };

            static std::uintptr_t moduleBase()
            {
                static std::uintptr_t cache{};
                if (!cache)
                {
                    const char* module_name = xorstr_("ac_client.exe");
                    HMODULE handle = LI_FN(GetModuleHandleA)(module_name);
                    cache = reinterpret_cast<std::uintptr_t>(handle);
                }
                return cache;
            }

            template <typename T>
            static T read(std::uintptr_t address)
            {
                return *reinterpret_cast<T*>(address);
            }

            static Matrix4x4 readViewMatrix(std::uintptr_t base)
            {
                Matrix4x4 matrix{};
                std::memcpy(matrix.data, reinterpret_cast<void*>(base + Offsets::ViewMatrix), sizeof(matrix.data));
                return matrix;
            }

            static Vec3 readPosition(std::uintptr_t entity)
            {
                return Vec3{
                    read<float>(entity + Offsets::Entity::PosX),
                    read<float>(entity + Offsets::Entity::PosY),
                    read<float>(entity + Offsets::Entity::PosZ)
                };
            }

            static bool worldToScreen(const Vec3& world, ImVec2& screen, const Matrix4x4& matrix, const ImVec2& display)
            {
                const float* m = matrix.data;
                float clip_x = world.x * m[0] + world.y * m[1] + world.z * m[2] + m[3];
                float clip_y = world.x * m[4] + world.y * m[5] + world.z * m[6] + m[7];
                float clip_w = world.x * m[12] + world.y * m[13] + world.z * m[14] + m[15];

                if (clip_w < 0.1f) return false;

                float inv_w = 1.0f / clip_w;
                float ndc_x = clip_x * inv_w;
                float ndc_y = clip_y * inv_w;

                screen.x = (display.x * 0.5f) + (ndc_x * display.x * 0.5f);
                screen.y = (display.y * 0.5f) - (ndc_y * display.y * 0.5f);

                return screen.x >= 0.0f && screen.x <= display.x && screen.y >= 0.0f && screen.y <= display.y;
            }

            static bool isAlive(std::uintptr_t entity)
            {
                int state = read<int>(entity + Offsets::Entity::State);
                int health = read<int>(entity + Offsets::Entity::Health);
                return state == 0 && health > 0;
            }

            static ImU32 packColor(const float color[4])
            {
                return ImGui::ColorConvertFloat4ToU32(ImVec4(color[0], color[1], color[2], color[3]));
            }

            static ImU32 friendlyColor(ImU32 base_color)
            {
                ImVec4 c = ImGui::ColorConvertU32ToFloat4(base_color);
                c.x *= 0.6f;
                c.y = std::min(1.0f, c.y + 0.2f);
                c.z = std::min(1.0f, c.z + 0.4f);
                return ImGui::ColorConvertFloat4ToU32(c);
            }

            static void drawGlow(ImDrawList* draw_list, const ImVec2& top_left, const ImVec2& bottom_right, ImU32 color)
            {
                for (int i = 1; i <= 3; ++i)
                {
                    float offset = static_cast<float>(i);
                    draw_list->AddRect(ImVec2(top_left.x - offset, top_left.y - offset), ImVec2(bottom_right.x + offset, bottom_right.y + offset), color, 0.0f, 0, 1.0f);
                }
            }

            static void drawBox(ImDrawList* draw_list, const ImVec2& top_left, const ImVec2& bottom_right, ImU32 color)
            {
                draw_list->AddRect(top_left, bottom_right, color, 0.0f, 0, 1.2f);
            }

            static void drawSkeleton(ImDrawList* draw_list, const ImVec2& head, const ImVec2& feet, ImU32 color)
            {
                float mid_x = (head.x + feet.x) * 0.5f;
                draw_list->AddLine(ImVec2(mid_x, head.y), ImVec2(mid_x, feet.y), color, 1.2f);
            }

            static void drawHealth(ImDrawList* draw_list, const ImVec2& top_left, const ImVec2& bottom_right, float ratio, ImU32 color)
            {
                ImVec2 bar_start{ top_left.x - 6.0f, bottom_right.y };
                ImVec2 bar_end{ top_left.x - 2.0f, top_left.y };
                draw_list->AddRectFilled(bar_start, bar_end, IM_COL32(10, 10, 10, 190));
                float height = (bar_end.y - bar_start.y) * ratio;
                draw_list->AddRectFilled(ImVec2(bar_start.x, bar_end.y - height), ImVec2(bar_end.x, bar_end.y), color);
            }

            static void drawName(ImDrawList* draw_list, const ImVec2& top_left, const ImVec2& bottom_right, const char* name)
            {
                ImVec2 text_size = ImGui::CalcTextSize(name);
                ImVec2 pos{ top_left.x + ((bottom_right.x - top_left.x) - text_size.x) * 0.5f, top_left.y - text_size.y - 4.0f };
                draw_list->AddText(ImVec2(pos.x + 1.0f, pos.y + 1.0f), IM_COL32(10, 10, 10, 220), name);
                draw_list->AddText(pos, IM_COL32(255, 255, 255, 230), name);
            }
        }

        void render()
        {
            __try
            {
                const bool enable_box = g_visualSettings.box;
                const bool enable_skeleton = g_visualSettings.skeleton;
                const bool enable_glow = g_visualSettings.glow;
                const bool enable_health = g_visualSettings.healthBar;
                if (!enable_box && !enable_skeleton && !enable_glow && !enable_health) return;

                if (!ImGui::GetCurrentContext()) return;
                ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
                if (!draw_list) return;

                const std::uintptr_t base = moduleBase();
                if (!base) return;

                std::uintptr_t local_player = read<std::uintptr_t>(base + Offsets::LocalPlayer);
                if (!local_player) return;

                std::uintptr_t* entity_list = read<std::uintptr_t*>(base + Offsets::EntityList);
                if (!entity_list) return;

                Matrix4x4 view_matrix = readViewMatrix(base);

                int max_players = std::clamp(read<int>(base + Offsets::MaxPlayers), 0, 128);
                if (max_players <= 0) return;

                ImGuiIO& io = ImGui::GetIO();
                ImVec2 display = io.DisplaySize;
                if (display.x <= 0.0f || display.y <= 0.0f) return;

                int local_team = read<int>(local_player + Offsets::Entity::Team);

                ImU32 box_color_enemy = packColor(g_visualSettings.boxColor);
                ImU32 skeleton_color_enemy = packColor(g_visualSettings.skeletonColor);
                ImU32 glow_color_enemy = packColor(g_visualSettings.glowColor);
                ImU32 health_color = packColor(g_visualSettings.healthColor);

                ImU32 box_color_friendly = friendlyColor(box_color_enemy);
                ImU32 skeleton_color_friendly = friendlyColor(skeleton_color_enemy);
                ImU32 glow_color_friendly = friendlyColor(glow_color_enemy);

                for (int i = 0; i < max_players; ++i)
                {
                    std::uintptr_t entity = entity_list[i];
                    if (!entity || entity == local_player) continue;
                    if (!isAlive(entity)) continue;

                    Vec3 position = readPosition(entity);
                    Vec3 head = position;
                    head.z += 72.0f;

                    ImVec2 screen_feet;
                    ImVec2 screen_head;
                    if (!worldToScreen(position, screen_feet, view_matrix, display)) continue;
                    if (!worldToScreen(head, screen_head, view_matrix, display)) continue;

                    float height = screen_feet.y - screen_head.y;
                    if (height <= 2.0f) continue;
                    float half_width = height * 0.3f;
                    ImVec2 top_left{ screen_feet.x - half_width, screen_head.y };
                    ImVec2 bottom_right{ screen_feet.x + half_width, screen_feet.y };

                    int team = read<int>(entity + Offsets::Entity::Team);
                    bool enemy = (team != local_team);

                    ImU32 box_color = enemy ? box_color_enemy : box_color_friendly;
                    ImU32 skeleton_color = enemy ? skeleton_color_enemy : skeleton_color_friendly;
                    ImU32 glow_color = enemy ? glow_color_enemy : glow_color_friendly;

                    if (enable_glow) drawGlow(draw_list, top_left, bottom_right, glow_color);
                    if (enable_box) drawBox(draw_list, top_left, bottom_right, box_color);
                    if (enable_skeleton) drawSkeleton(draw_list, screen_head, screen_feet, skeleton_color);

                    if (enable_health)
                    {
                        int hp = std::clamp(read<int>(entity + Offsets::Entity::Health), 0, 100);
                        float ratio = static_cast<float>(hp) * 0.01f;
                        drawHealth(draw_list, top_left, bottom_right, ratio, health_color);
                    }

                    char name[16]{};
                    std::memcpy(name, reinterpret_cast<void*>(entity + Offsets::Entity::Name), sizeof(name) - 1);
                    if (name[0] != '\0') drawName(draw_list, top_left, bottom_right, name);
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
            }
        }
    }
}
