#include <iostream>
#include <Windows.h>
#include <libloaderapi.h>
#include <d3d11.h>
#include <dxgi.h>
#include <wrl.h>

#include "il2cpp.h"
#include "pragma.h"
#include "Memory.h"
#include "Util.h"

using namespace Microsoft::WRL;

void Main();

typedef HRESULT(__stdcall* DXGIPresent_t)(IDXGISwapChain*, UINT, UINT);
HRESULT __stdcall hkPresent(IDXGISwapChain* This, UINT SyncInterval, UINT Flags);

DXGIPresent_t g_ogPresent = nullptr;

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
    
    return g_ogPresent(This, SyncInterval, Flags);
}