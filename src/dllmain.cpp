#include <iostream>
#include <Windows.h>
#include "il2cpp.h"

void Main();

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, LPVOID lpvReserved) {
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH:
            FILE* f;
            freopen_s(&f, "CONOUT$", "w", stdout);
            AllocConsole();

            CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(Main), hInstance, NULL, nullptr);
            break;
    }
    return TRUE;
}

void Main() {
    std::cout << "Welcome to Scheduler!" << std::endl;
    std::cout << "By: PR3C14D0" << std::endl;
    std::cout << "Github: https://github.com/PR3C14D0" << std::endl;

}