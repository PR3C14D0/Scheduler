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

    if (dwPid <= 0) {
        std::cout << "Process not found" << std::endl;
        return 1;
    }

    const char* dllName = "dxgi.dll";
    char dllPath[MAX_PATH];

    GetFullPathName(dllName, MAX_PATH, dllPath, nullptr);
    std::cout << "DLL name: " << dllName << std::endl;
    std::cout << "DLL path: " << dllPath << std::endl;

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
    LPVOID lpAddr = VirtualAllocEx(hProcess, nullptr, MAX_PATH, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    std::cout << "Allocated memory address: 0x" << std::hex << reinterpret_cast<DWORD_PTR>(lpAddr) << std::endl;
    WriteProcessMemory(hProcess, lpAddr, dllPath, MAX_PATH, nullptr);

    HANDLE hThread = CreateRemoteThreadEx(hProcess, nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(LoadLibraryA), lpAddr,  NULL, nullptr, nullptr);

    if (hThread)
        CloseHandle(hThread);

    if (hProcess)
        CloseHandle(hProcess);

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