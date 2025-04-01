#include "Memory.h"

LPVOID Memory::CreateHook(LPVOID lpSrc, LPVOID lpDst, UINT nMangledBytes, LPVOID& lpGateway) {
	LPVOID lpLoc = (char*)lpSrc - 0x2000;
	LPVOID lpRelay = nullptr;

	do {
		lpRelay = VirtualAlloc(lpLoc, 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		lpLoc = (char*)lpLoc + 0x200;
	} while (lpRelay == nullptr);

	std::cout << "Hook at: 0x" << std::hex << (DWORD_PTR)lpRelay << std::endl;

	LPVOID lpAddr = lpRelay;

	LPVOID lpMangledBytes = VirtualAlloc(nullptr, nMangledBytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	memcpy(lpMangledBytes, lpSrc, nMangledBytes);
	VirtualProtect(lpMangledBytes, nMangledBytes, PAGE_READONLY, nullptr);
	
	std::cout << "Mangled bytes address: 0x" << std::hex << (DWORD_PTR)lpMangledBytes << std::endl;

	return 0x00;
}