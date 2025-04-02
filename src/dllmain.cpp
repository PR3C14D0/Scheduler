#include <iostream>
#include <Windows.h>
#include <libloaderapi.h>
#include <d3d11.h>
#include <dxgi.h>
#include <wrl.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui_impl_win32.h>

#include "il2cpp.h"
#include "pragma.h"
#include "Memory.h"
#include "Util.h"

using namespace Microsoft::WRL;

void Main();

typedef HRESULT(__stdcall* DXGIPresent_t)(IDXGISwapChain*, UINT, UINT);
HRESULT __stdcall hkPresent(IDXGISwapChain* This, UINT SyncInterval, UINT Flags);

DXGIPresent_t g_ogPresent = nullptr;
bool g_bImGuiInit = false;

ComPtr<ID3D11Device> g_dev;
ComPtr<ID3D11DeviceContext> g_con;
ComPtr<ID3D11Texture2D> g_backBuffer;
ComPtr<ID3D11RenderTargetView> g_rtv;

HWND g_hwnd = NULL;

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, LPVOID lpvReserved) {
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH:
#ifndef NDEBUG
            AllocConsole();
            FILE* f;
            freopen_s(&f, "CONOUT$", "w", stdout);
#endif
            CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(Main), hInstance, NULL, nullptr);
            break;
    }
    return TRUE;
}

void Main() {
    std::cout << "Welcome to Scheduler!" << std::endl;
    std::cout << "By: PR3C14D0" << std::endl;
    std::cout << "Github: https://github.com/PR3C14D0" << std::endl;

    HMODULE GameAssembly = GetModuleHandle("GameAssembly.dll");
   
    ComPtr<ID3D11Device> pDev;
    ComPtr<ID3D11DeviceContext> pCon;
    ComPtr<IDXGISwapChain> pSc;

    HWND hwnd = NULL;
    hwnd = FindWindow(nullptr, "Schedule I");
    g_hwnd = hwnd;

    if (!hwnd) {
        std::cout << "Failed to find the Window Handle" << std::endl;
        return;
    }

    DXGI_SWAP_CHAIN_DESC scDesc = { };
	scDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	scDesc.BufferCount = 1;
	scDesc.SampleDesc.Count = 1;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scDesc.Windowed = TRUE;
	scDesc.OutputWindow = hwnd;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

    ThrowIfFailed(D3D11CreateDeviceAndSwapChain(nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        NULL,
        nullptr,
        NULL,
        D3D11_SDK_VERSION, 
        &scDesc,
        pSc.GetAddressOf(),
        pDev.GetAddressOf(),
        nullptr,
        pCon.GetAddressOf()
    ));

    void** pVmt = *(reinterpret_cast<void***>(pSc.Get()));
    std::cout << "DXGI Virtual method table at: 0x" << std::hex << (DWORD_PTR)pVmt << std::endl;

    LPVOID lpPresent = pVmt[8];
    std::cout << "IDXGISwapChain::Present address: 0x" << std::hex << (DWORD_PTR)lpPresent << std::endl;
    g_ogPresent = reinterpret_cast<DXGIPresent_t>(lpPresent);

    //Memory::DisableSteamOverlay(lpPresent, 5);


    LPVOID gateway = nullptr;
    LPVOID lpPresentRelay = Memory::CreateHook(lpPresent, &hkPresent, 5, gateway);
    g_ogPresent = reinterpret_cast<DXGIPresent_t>(gateway);
    std::cout << "Gateway at: 0x" << std::hex << (DWORD_PTR)gateway << std::endl;

    Memory::Detour32(lpPresent, lpPresentRelay, 5);
}

HRESULT __stdcall hkPresent(IDXGISwapChain* This, UINT SyncInterval, UINT Flags) {
   
    if (!g_bImGuiInit) {
        
        This->GetDevice(IID_PPV_ARGS(g_dev.GetAddressOf()));
        g_dev->GetImmediateContext(g_con.GetAddressOf());

        This->GetBuffer(0, IID_PPV_ARGS(g_backBuffer.GetAddressOf()));
        g_dev->CreateRenderTargetView(g_backBuffer.Get(), nullptr, g_rtv.GetAddressOf());

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui_ImplDX11_Init(g_dev.Get(), g_con.Get());
        ImGui_ImplWin32_Init(g_hwnd);

        ImGuiIO& io = ImGui::GetIO(); (void)io;

        g_bImGuiInit = true;
    }

    g_con->OMSetRenderTargets(1, g_rtv.GetAddressOf(), nullptr);

    ImGui_ImplWin32_NewFrame();
    ImGui_ImplDX11_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowSize(ImVec2{ 300, 300 });
    ImGui::Begin("Scheduler");
    ImGui::End();

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return g_ogPresent(This, SyncInterval, Flags);
}