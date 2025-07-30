#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include "imgui/imgui.h"

namespace KbSystem {

    struct BindState {
        void* valuePtr = nullptr;
        void* keybindPtr = nullptr;
    };

    static std::vector<BindState> g_activeBindStates;

    inline bool IsKeyDown(int vk) {
        return (GetAsyncKeyState(vk) & 0x8000) != 0;
    }

    inline const char* GetVirtualKeyCodeName(int vk) {
        static char name[128];
        if (vk == 0) return "None";

        UINT scanCode = MapVirtualKeyA(vk, MAPVK_VK_TO_VSC);
        switch (vk) {
        case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN:
        case VK_PRIOR: case VK_NEXT: case VK_END: case VK_HOME:
        case VK_INSERT: case VK_DELETE: case VK_DIVIDE: case VK_NUMLOCK:
            scanCode |= 0x100;
            break;
        }

        if (GetKeyNameTextA(scanCode << 16, name, sizeof(name)) > 0)
            return name;

        return "Unknown";
    }

    inline bool ButtonWithKeybind(const char* label, int* keybind, bool disableable = true) {
        ImGui::PushID(label);

        std::string buttonLabel = std::string("[") + GetVirtualKeyCodeName(*keybind) + "]";

        float buttonHeight = ImGui::GetFrameHeight();
        float textHeight = ImGui::GetTextLineHeight();

        float offsetY = (buttonHeight - textHeight) * 0.5f;

        ImVec2 originalPos = ImGui::GetCursorPos();

        ImGui::SetCursorPosY(originalPos.y + offsetY);
        ImGui::TextDisabled("%s", label);

        float textAndSpacingWidth = ImGui::CalcTextSize(label).x + ImGui::GetStyle().ItemInnerSpacing.x;

        ImGui::SetCursorPos(ImVec2(originalPos.x + textAndSpacingWidth, originalPos.y));

        bool clicked = ImGui::Button(buttonLabel.c_str());

        bool found = false;
        for (const auto& bs : g_activeBindStates) {
            if (bs.keybindPtr == (void*)keybind) {
                found = true;
                break;
            }
        }
        if (!found) {
            g_activeBindStates.push_back({ nullptr, (void*)keybind });
        }

        if (ImGui::IsItemHovered() && IsKeyDown(VK_CONTROL)) {
            std::string tooltip = "Press a key to set the bind.";
            if (disableable) {
                tooltip += "Press DELETE to clear.";
            }
            ImGui::SetTooltip(tooltip.c_str());
            if (disableable && IsKeyDown(VK_DELETE)) {
                *keybind = 0;
            }
            else {
                for (int key = 0x08; key <= 0xFE; key++) {
                    if ((GetAsyncKeyState(key) & 0x1) &&
                        key != VK_CONTROL && key != VK_LCONTROL && key != VK_RCONTROL &&
                        key != VK_SHIFT && key != VK_LSHIFT && key != VK_RSHIFT &&
                        key != VK_MENU && key != VK_LMENU && key != VK_RMENU) {
                        *keybind = key;
                        break;
                    }
                }
            }
        }

        ImGui::PopID();
        return clicked;
    }

}
