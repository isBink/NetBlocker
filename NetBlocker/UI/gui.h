#pragma once
#include <d3d11.h>   // DirectX 11 header
#include <dxgi.h>    // DXGI header for SwapChain
#include <sstream>   // For std::stringstream
#include <string>    // For std::string
#include <vector>
#include <iostream>
#include <array>

namespace gui
{
    inline int posX = 0;
    inline int posY = 0;

    // winapi window vars
    inline HWND window = nullptr;
    inline WNDCLASSEX windowClass = { };

    // points for window movement
    inline POINTS position = { };

    // DirectX 11 state variables
    inline ID3D11Device* device = nullptr;                         // Device for rendering
    inline ID3D11DeviceContext* deviceContext = nullptr;           // Device context
    inline IDXGISwapChain* swapChain = nullptr;                    // Swap chain
    inline ID3D11RenderTargetView* mainRenderTargetView = nullptr; // Render target view
    inline DXGI_SWAP_CHAIN_DESC presentParameters = { };           // Present parameters for SwapChain

    // handle window creation & destruction
    void CreateHWindow(const char* windowName) noexcept;
    void DestroyHWindow() noexcept;
    // handle device creation & destruction
    bool CreateDevice() noexcept;
    void ResetDevice() noexcept;
    void DestroyDevice() noexcept;

    // Handle creating and destroying the render target
    void CreateRenderTarget();
    void CleanupRenderTarget();

    // handle ImGui creation & destruction
    void CreateImGui() noexcept;
    void DestroyImGui() noexcept;

    // Rendering cycle
    void BeginRender() noexcept;
    void EndRender() noexcept;
    void Render() noexcept;
}