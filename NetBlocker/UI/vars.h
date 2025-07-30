#pragma once
#include "../imgui/imgui.h"

namespace varsUI {
    const int Width = GetSystemMetrics(SM_CXSCREEN);    // Full screen width
    const int Height = GetSystemMetrics(SM_CYSCREEN);   // Full screen height
    inline int DPI = 100;

    const int imWIDTH = (Width / 4 * (DPI / 100));
    const int imHEIGHT = (Height / 4 * (DPI / 100));

    inline bool isRunning = true;
    inline bool isVisible = true;
    inline int taskbarHeight = 0;
    inline int uiUpdateTime = 10;
    inline ImVec4 accentColor = ImColor(55, 55, 55, 255);
    inline float version = 1.1;
    inline std::string menuName = "NetBlocker";
    inline int toggleKeybind = VK_F1;
}