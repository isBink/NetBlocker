#include "gui.h"
#include <tchar.h>
#include <psapi.h>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include <array>
#include <fstream>
#include <windows.h>
#include <tlhelp32.h>
#include <filesystem>
#include "vars.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_dx11.h"
#include "../imgui/imgui_impl_win32.h"
#include "../netblocker.h"
#include "../kbSystem.h"
#include "../soundSystem.h"
namespace fs = std::filesystem;

ImVec4 MultiplyColor(const ImVec4& color, float factor)
{
    return ImVec4(color.x * factor, color.y * factor, color.z * factor, color.w);
}

uint32_t HexStringToUint32(const std::string& hexStr) {
    uint32_t value = 0;
    std::stringstream ss;
    ss << std::hex << hexStr;
    ss >> value;
    return value;
}

std::string Uint32ToHexString(uint32_t value) {
    std::stringstream ss;
    ss << "0x" << std::hex << std::uppercase << value;
    return ss.str();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND window,
    UINT message,
    WPARAM wideParameter,
    LPARAM longParameter
);

LRESULT CALLBACK WindowProcess(
    HWND window,
    UINT message,
    WPARAM wideParameter,
    LPARAM longParameter)
{
    if (ImGui_ImplWin32_WndProcHandler(window, message, wideParameter, longParameter))
        return true;

    switch (message)
    {
    case WM_SIZE: {
        if (gui::device && wideParameter != SIZE_MINIMIZED)
        {
            gui::presentParameters.BufferDesc.Width = LOWORD(longParameter);
            gui::presentParameters.BufferDesc.Height = HIWORD(longParameter);
            gui::ResetDevice();
        }
        return 0;
    }

    case WM_SYSCOMMAND: {
        if ((wideParameter & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    }

    case WM_DESTROY: {
        PostQuitMessage(0);
    } return 0;

    case WM_LBUTTONDOWN: {
        // Removed window dragging behavior
    } return 0;

    case WM_MOUSEMOVE: {
        if (wideParameter == MK_LBUTTON)
        {
            // Disabled window dragging behavior
        }
    } return 0;

    }

    return DefWindowProc(window, message, wideParameter, longParameter);
}

void setStyle()
{
    auto accentColor = varsUI::accentColor;
    ImGuiStyle& style = ImGui::GetStyle();

    style.Colors[ImGuiCol_Text] = ImColor(255, 255, 255, 255);

    style.Colors[ImGuiCol_Button] = MultiplyColor(accentColor, 0.5f);
    style.Colors[ImGuiCol_ButtonHovered] = accentColor;
    style.Colors[ImGuiCol_ButtonActive] = MultiplyColor(accentColor, 1.1f);

    style.Colors[ImGuiCol_FrameBg] = MultiplyColor(accentColor, 0.2f);
    style.Colors[ImGuiCol_FrameBgHovered] = MultiplyColor(accentColor, 0.5f);
    style.Colors[ImGuiCol_FrameBgActive] = MultiplyColor(accentColor, 0.7f);

    style.Colors[ImGuiCol_Header] = MultiplyColor(accentColor, 0.3f);
    style.Colors[ImGuiCol_HeaderHovered] = MultiplyColor(accentColor, 0.6f);
    style.Colors[ImGuiCol_HeaderActive] = accentColor;

    style.Colors[ImGuiCol_Tab] = MultiplyColor(accentColor, 0.3f);
    style.Colors[ImGuiCol_TabHovered] = MultiplyColor(accentColor, 0.6f);
    style.Colors[ImGuiCol_TabActive] = accentColor;

    style.Colors[ImGuiCol_SliderGrab] = MultiplyColor(accentColor, 0.7f);
    style.Colors[ImGuiCol_SliderGrabActive] = accentColor;

    style.Colors[ImGuiCol_CheckMark] = accentColor;
    style.Colors[ImGuiCol_Separator] = MultiplyColor(accentColor, 0.4f);
    style.Colors[ImGuiCol_SeparatorHovered] = MultiplyColor(accentColor, 0.7f);
    style.Colors[ImGuiCol_SeparatorActive] = accentColor;

    style.Colors[ImGuiCol_Border] = MultiplyColor(accentColor, 0.4f);
    style.Colors[ImGuiCol_BorderShadow] = MultiplyColor(accentColor, 0.2f);

    style.Colors[ImGuiCol_TitleBg] = MultiplyColor(accentColor, 0.2f);
    style.Colors[ImGuiCol_TitleBgActive] = MultiplyColor(accentColor, 0.5f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = MultiplyColor(accentColor, 0.1f);

    style.Colors[ImGuiCol_WindowBg] = ImColor(20, 20, 20, 255);
}

void gui::CreateHWindow(const char* windowName) noexcept
{
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_CLASSDC;
    windowClass.lpfnWndProc = WindowProcess;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = GetModuleHandleA(0);
    windowClass.hIcon = 0;
    windowClass.hCursor = LoadCursor(0, IDC_ARROW);
    windowClass.hbrBackground = 0;
    windowClass.lpszMenuName = 0;
    windowClass.lpszClassName = "class001";
    windowClass.hIconSm = 0;

    RegisterClassEx(&windowClass);

    HWND taskbar = FindWindow("Shell_TrayWnd", nullptr);
    if (taskbar)
    {
        RECT taskbarRect;
        GetWindowRect(taskbar, &taskbarRect);
        varsUI::taskbarHeight = taskbarRect.bottom - taskbarRect.top;
    }

    window = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TOPMOST,
        "class001",
        windowName,
        WS_POPUP,
        0,
        0 - varsUI::taskbarHeight,
        varsUI::Width,
        varsUI::Height,
        0,
        0,
        windowClass.hInstance,
        0
    );

    SetLayeredWindowAttributes(window, RGB(0, 0, 0), 0, LWA_COLORKEY);

    ShowWindow(window, SW_SHOWDEFAULT);
    UpdateWindow(window);
}

void gui::DestroyHWindow() noexcept
{
    DestroyWindow(window);
    UnregisterClassA("class001", GetModuleHandle(nullptr));
}

bool gui::CreateDevice() noexcept
{
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Width = varsUI::Width;
    swapChainDesc.BufferDesc.Height = varsUI::Height;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = window;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL featureLevel;
    D3D_FEATURE_LEVEL featureLevelArray[1] = { D3D_FEATURE_LEVEL_11_0 };

    if (FAILED(D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        featureLevelArray,
        1,
        D3D11_SDK_VERSION,
        &swapChainDesc,
        &swapChain,
        &device,
        &featureLevel,
        &deviceContext)))
    {
        return false;
    }

    CreateRenderTarget();
    return true;
}

void gui::CreateRenderTarget()
{
    ID3D11Texture2D* backBuffer;
    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
    device->CreateRenderTargetView(backBuffer, NULL, &mainRenderTargetView);
    backBuffer->Release();
    deviceContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
}

void gui::ResetDevice() noexcept
{
    CleanupRenderTarget();

    swapChain->ResizeBuffers(0, varsUI::Width, varsUI::Height, DXGI_FORMAT_UNKNOWN, 0);
    CreateRenderTarget();
}

void gui::CleanupRenderTarget()
{
    if (mainRenderTargetView) {
        mainRenderTargetView->Release();
        mainRenderTargetView = nullptr;
    }
}

void gui::DestroyDevice() noexcept
{
    CleanupRenderTarget();
    if (swapChain) { swapChain->Release(); swapChain = nullptr; }
    if (deviceContext) { deviceContext->Release(); deviceContext = nullptr; }
    if (device) { device->Release(); device = nullptr; }
}

void gui::CreateImGui() noexcept
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ::ImGui::GetIO();
    io.IniFilename = NULL;

    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX11_Init(device, deviceContext);
}

void gui::DestroyImGui() noexcept
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void gui::BeginRender() noexcept
{
    MSG message;
    while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&message);
        DispatchMessage(&message);

        if (message.message == WM_QUIT)
        {
            varsUI::isRunning = !varsUI::isRunning;
            return;
        }
    }

    setStyle();

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void gui::EndRender() noexcept
{
    ImGui::EndFrame();

    const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    deviceContext->ClearRenderTargetView(mainRenderTargetView, clearColor);

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    swapChain->Present(1, 0);
}

DWORD GetProcessIdByName(const std::string& processName) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32 processEntry = { sizeof(PROCESSENTRY32) };

    if (Process32First(snapshot, &processEntry)) {
        do {
            if (processName == processEntry.szExeFile) {
                CloseHandle(snapshot);
                return processEntry.th32ProcessID;
            }
        } while (Process32Next(snapshot, &processEntry));
    }
    CloseHandle(snapshot);
    return 0;
}

using namespace std;
static DWORD processId = 0;
static HANDLE processHandle = nullptr;

void gui::Render() noexcept
{
    setStyle();

    // Initialize sound system if not already done
    static bool soundSystemInitialized = false;
    if (!soundSystemInitialized) {
        soundSystem::SoundManager::Initialize();
        soundSystemInitialized = true;
    }

    if (varsUI::isVisible) {
        ImGui::SetNextWindowPos({ 50, 50 + static_cast<float>(varsUI::taskbarHeight) }, ImGuiCond_Once);
        ImGui::SetNextWindowSize({ static_cast<float>(varsUI::imWIDTH), static_cast<float>(varsUI::imHEIGHT) });
        ImGui::Begin(varsUI::menuName.c_str(), &varsUI::isRunning, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);

        gui::posX = ImGui::GetWindowPos().x;
        gui::posY = ImGui::GetWindowPos().y;

        ImGui::InputText("Process Name", netBlock::processName, IM_ARRAYSIZE(netBlock::processName));
        if (ImGui::Button("Attach")) {
            processId = GetProcessIdByName(netBlock::processName);
            if (processId) {
                processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
                if (processHandle) {
                    netBlock::attachedProcessPath = netBlock::GetProcessFullPath(processHandle);
                    netBlock::attachedRuleName = L"Block_" + std::wstring(netBlock::processName, netBlock::processName + strlen(netBlock::processName));
                }
            }
        }

        auto& soundPacks = soundSystem::SoundManager::GetSoundPacks();
        std::vector<const char*> packNames;
        for (const auto& pack : soundPacks) {
            packNames.push_back(pack.name.c_str());
        }

        int currentPack = soundSystem::SoundManager::GetSelectedPackIndex();
        if (ImGui::Combo("Sound Pack", &currentPack, packNames.data(), static_cast<int>(packNames.size()))) {
            soundSystem::SoundManager::SetSelectedPackIndex(currentPack);
        }

        // Refresh button
        if (ImGui::Button("Refresh Sounds")) {
            soundSystem::SoundManager::ReloadSounds();
        }

        KbSystem::ButtonWithKeybind("Block/Unblock", &varsUI::toggleKeybind, false);

        ImGui::End();
    }

    if (true)
    {
        ImGui::SetNextWindowPos({ 0, static_cast<float>(varsUI::taskbarHeight) }, ImGuiCond_Once);
        ImGui::SetNextWindowSize({ static_cast<float>(GetSystemMetrics(SM_CXSCREEN)), static_cast<float>(GetSystemMetrics(SM_CYSCREEN)) - static_cast<float>(varsUI::taskbarHeight) });

        ImGui::Begin(
            "Overlay",
            nullptr,
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoInputs
        );

        if (!netBlock::attachedProcessPath.empty()) {
            ImVec2 windowSize = ImGui::GetWindowSize();

            std::string procText = "Attached to: " + std::string(netBlock::processName);
            ImVec2 textSize = ImGui::CalcTextSize(procText.c_str());
            ImGui::SetCursorPos(ImVec2((windowSize.x - textSize.x) * 0.5f, 35));
            ImGui::Text("%s", procText.c_str());

            const char* statusText;
            ImVec4 statusColor;

            std::string keyName = KbSystem::GetVirtualKeyCodeName(varsUI::toggleKeybind);
            std::string blockedText = "INTERNET BLOCKED - PRESS " + keyName + " TO UNBLOCK";
            std::string unblockedText = "INTERNET UNBLOCKED - PRESS " + keyName + " TO BLOCK";

            if (netBlock::isBlocked) {
                statusText = blockedText.c_str();
                statusColor = ImVec4(1.f, 0.f, 0.f, 1.f);
            }
            else {
                statusText = unblockedText.c_str();
                statusColor = ImVec4(0.f, 1.f, 0.f, 1.f);
            }

            ImVec2 statusSize = ImGui::CalcTextSize(statusText);
            ImGui::SetCursorPos(ImVec2((windowSize.x - statusSize.x) * 0.5f, 50));
            ImGui::TextColored(statusColor, "%s", statusText);
        }

        ImGui::End();
    }
}
