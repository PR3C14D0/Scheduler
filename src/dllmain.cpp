#include <iostream>
#include <Windows.h>
#include <libloaderapi.h>
#include <d3d11.h>
#include <dxgi.h>
#include <wrl.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui_impl_win32.h>
#include <Psapi.h>

#include "il2cpp.h"
#include "pragma.h"
#include "Memory.h"
#include "Util.h"

using namespace Microsoft::WRL;

void Main();

typedef HRESULT(__stdcall* DXGIPresent_t)(IDXGISwapChain*, UINT, UINT);
typedef void(__fastcall* CashUpdate_t)(ScheduleOne_Money_MoneyManager_o* _this);
HRESULT __stdcall hkPresent(IDXGISwapChain* This, UINT SyncInterval, UINT Flags);
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void __fastcall hkCashUpdate(ScheduleOne_Money_MoneyManager_o* _this);

DXGIPresent_t g_ogPresent = nullptr;
CashUpdate_t g_ogCashUpdate = nullptr;
bool g_bImGuiInit = false;

bool g_bChangeCash = false;

ComPtr<ID3D11Device> g_dev;
ComPtr<ID3D11DeviceContext> g_con;
ComPtr<ID3D11Texture2D> g_backBuffer;
ComPtr<ID3D11RenderTargetView> g_rtv;

LONG_PTR g_OldWndProc;

HWND g_hwnd = NULL;

#define CHANGE_CASH_VALUE 0x9316B0;

float g_fCash = 0.0f;

void ClassicHook();
void SteamHook(HMODULE hModule);


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
    LPVOID lpGameAssembly = reinterpret_cast<LPVOID>(GameAssembly);

    HWND hwnd = NULL;
    hwnd = FindWindow(nullptr, "Schedule I");
    g_hwnd = hwnd;

    if (!hwnd) {
        std::cout << "Failed to find the Window Handle" << std::endl;
        return;
    }

    HMODULE GameOverlay = nullptr;
    GameOverlay = GetModuleHandle("GameOverlayRenderer64.dll");

    if (GameOverlay) {
        SteamHook(GameOverlay);
    }
    else {
        ClassicHook();
    }

    g_OldWndProc = SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);

    LPVOID lpCashUpdate = (char*)lpGameAssembly + CHANGE_CASH_VALUE;
    g_ogCashUpdate = reinterpret_cast<CashUpdate_t>(lpCashUpdate);
    std::cout << "Cash update value address: 0x" << std::hex << (DWORD_PTR)lpCashUpdate << std::endl;
    
    LPVOID lpCashGateway = nullptr;
    LPVOID lpCashRelay = Memory::CreateHook(lpCashUpdate, &hkCashUpdate, 7, lpCashGateway);
    
    g_ogCashUpdate = reinterpret_cast<CashUpdate_t>(lpCashGateway);
    Memory::Detour32(lpCashUpdate, lpCashRelay, 7);
}

void ClassicHook() {
    /* Dummy device method */
    ComPtr<ID3D11Device> pDev;
    ComPtr<ID3D11DeviceContext> pCon;
    ComPtr<IDXGISwapChain> pSc;

    DXGI_SWAP_CHAIN_DESC scDesc = { };
    scDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    scDesc.BufferCount = 1;
    scDesc.SampleDesc.Count = 1;
    scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    scDesc.Windowed = TRUE;
    scDesc.OutputWindow = g_hwnd;
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

    LPVOID gateway = nullptr;
    LPVOID lpPresentRelay = Memory::CreateHook(lpPresent, &hkPresent, 5, gateway);
    g_ogPresent = reinterpret_cast<DXGIPresent_t>(gateway);
    std::cout << "Gateway at: 0x" << std::hex << (DWORD_PTR)gateway << std::endl;
    Memory::Detour32(lpPresent, lpPresentRelay, 5);

}

void SteamHook(HMODULE hModule) {
    // 48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 41 8B E8 - HkPresent
    // 48 89 5C 24 ? 57 48 83 EC ? 33 C0 48 89 44 24 - CreateHook

    const char* hkPresentSig = "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 41 8B E8";
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
    if (ImGui::Button("Add 10k")) {
        HMODULE GameAssembly = GetModuleHandle("GameAssembly.dll");
        LPVOID lpGameAssembly = reinterpret_cast<LPVOID>(GameAssembly);
        DWORD_PTR lpCashAddr = Memory::FindDMAAddy((DWORD_PTR)((char*)lpGameAssembly + 0x037A4690), { 0x40, 0xB8, 0x10, 0xF0, 0x1C8, 0x38 });
        *(float*)lpCashAddr = *(float*)lpCashAddr + 10000.f;
    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return g_ogPresent(This, SyncInterval, Flags);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
        return 0;

    return CallWindowProc((WNDPROC)g_OldWndProc, hwnd, uMsg, wParam, lParam);
}

void __fastcall hkCashUpdate(ScheduleOne_Money_MoneyManager_o* _this) {

    return g_ogCashUpdate(_this);
}