#include <iostream>
#include <Windows.h>
#include <libloaderapi.h>
#include <tlhelp32.h>

DWORD GetPid(const char* processName);

int main() {
    std::cout << "Welcome to Scheduler Injector!" << std::endl;
    std::cout << "By: PR3C14D0" << std::endl;
    std::cout << "Github: https://github.com/PR3C14D0" << std::endl;

    const char* procName = "Schedule I.exe";

    DWORD dwPid = GetPid(procName);
    std::cout << "Process name: " << procName << std::endl;
    std::cout << "Process ID: " << dwPid << std::endl;

    const char* dllName = "dxgi.dll";
    char dllPath[MAX_PATH];

    return 0;
}

DWORD GetPid(const char* processName) {
    DWORD dwPid = 0;

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    PROCESSENTRY32 pe = { };
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnap, &pe)) {
        do {
            if (strcmp(pe.szExeFile, processName) == 0) {
                dwPid = pe.th32ProcessID;
            }
        } while (Process32Next(hSnap, &pe));
    }

    return dwPid;
}