#include <iostream>
#include <Windows.h>
#include <libloaderapi.h>
#include "il2cpp.h"
#include "pragma.h"
#include "Memory.h"

void Main();

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

    HMODULE hDxgi = GetModuleHandle("dxgi.dll");
    std::cout << hDxgi << std::endl;

    LPVOID gateway = 0;
    Memory::CreateHook(GameAssembly, nullptr, 0, gateway);
}