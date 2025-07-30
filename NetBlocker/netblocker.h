#pragma once
#include <windows.h>
#include <string>
#include <thread>
#include <filesystem>
#include <psapi.h>
#include <iostream>
#include "soundSystem.h"

#pragma comment(lib, "winmm.lib")

namespace fs = std::filesystem;

namespace netBlock {
    inline bool isBlocked = false;
    inline char processName[256] = "Example.exe";
    inline std::wstring attachedProcessPath = L"";
    inline std::wstring attachedRuleName = L"";

    inline std::string GetExecutableFolder() {
        char buffer[MAX_PATH];
        GetModuleFileNameA(NULL, buffer, MAX_PATH);
        return fs::path(buffer).parent_path().string();
    }

    // Run command silently (hidden window)
    inline void RunCommandSilent(const std::wstring& cmd) {
        STARTUPINFOW si{};
        PROCESS_INFORMATION pi{};
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

        std::wstring fullCmd = L"cmd.exe /C " + cmd;

        if (CreateProcessW(nullptr, fullCmd.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
            WaitForSingleObject(pi.hProcess, INFINITE);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
    }

    inline void BlockApp(const std::wstring& ruleName, const std::wstring& appPath) {
        std::wstring cmd = L"netsh advfirewall firewall add rule name=\"" + ruleName +
            L"\" dir=out program=\"" + appPath + L"\" action=block";
        RunCommandSilent(cmd);
        soundSystem::SoundManager::PlaySound(true); // Use new SoundManager
    }

    inline void UnblockApp(const std::wstring& ruleName) {
        std::wstring cmd = L"netsh advfirewall firewall delete rule name=\"" + ruleName + L"\"";
        RunCommandSilent(cmd);
        soundSystem::SoundManager::PlaySound(false); // Use new SoundManager
    }

    // Get full process path reliably
    inline std::wstring GetProcessFullPath(HANDLE hProcess) {
        if (!hProcess) return L"";

        wchar_t path[MAX_PATH] = { 0 };
        DWORD size = MAX_PATH;
        if (QueryFullProcessImageNameW(hProcess, 0, path, &size)) {
            return std::wstring(path);
        }
        return L"";
    }

    // Input loop to toggle block/unblock
    inline void getInput() {
        while (true) {
            if ((GetAsyncKeyState(varsUI::toggleKeybind) & 0x8000) && !(GetAsyncKeyState(VK_LCONTROL) & 0x8000)) {
                if (!attachedProcessPath.empty()) {
                    if (!isBlocked) {
                        BlockApp(attachedRuleName, attachedProcessPath);
                        isBlocked = true;
                    }
                    else {
                        UnblockApp(attachedRuleName);
                        isBlocked = false;
                    }
                    Sleep(300);
                }
            }
            Sleep(10);
        }
    }
}