#include "Memory.h"

LPVOID Memory::CreateHook(LPVOID lpSrc, LPVOID lpDst, UINT nMangledBytes, LPVOID& lpGateway) {
	/*
		Relay: ABS_JMP Size: 
			6 bytes instruction 
					+
			8 bytes address
			Total: 14 bytes

		Gateway: 
			Mangled bytes (X bytes)
					+
			JMP32 (5 bytes)

		The ABS JMP will jump to our hook function, and then, call the gateway.
	*/

	LPVOID lpLoc = (char*)lpSrc - 0x2000;
	LPVOID lpRelay = nullptr;

	do {
		lpRelay = VirtualAlloc(lpLoc, 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		lpLoc = (char*)lpLoc + 0x200;
	} while (lpRelay == nullptr);

	std::cout << "Hook at: 0x" << std::hex << (DWORD_PTR)lpRelay << std::endl;

	LPVOID lpAddr = lpRelay;

	LPVOID lpMangledBytes = VirtualAlloc(nullptr, nMangledBytes, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	memcpy(lpMangledBytes, lpSrc, nMangledBytes);
	VirtualProtect(lpMangledBytes, nMangledBytes, PAGE_READONLY, nullptr);
	
	std::cout << "Mangled bytes address: 0x" << std::hex << (DWORD_PTR)lpMangledBytes << std::endl;

	const char ABS_JMP[] = {
		0xFF, 0x25, 0x00, 0x00, 0x00, 0x00
	};

	UINT nJmpSize = sizeof(ABS_JMP);
	
	memcpy(lpAddr, ABS_JMP, nJmpSize);

	*(DWORD_PTR*)((char*)lpAddr + nJmpSize) = reinterpret_cast<DWORD_PTR>(lpDst);

	lpAddr = (char*)lpAddr + nJmpSize + sizeof(DWORD_PTR);

	/* 
		Rel32 JMP: 5 bytes (JMP + 32 Bit relative address
			Relative address: Destination - Source - JMP instruction (5)
	*/
	memcpy(lpAddr, lpMangledBytes, nMangledBytes);
	DWORD relAddr = (DWORD)lpSrc - (DWORD)lpAddr - 5;
	*((char*)lpAddr + nMangledBytes) = 0xE9;
	*(DWORD*)((char*)lpAddr + nMangledBytes + 1) = relAddr;

	lpGateway = lpAddr;

	return lpRelay;
}

void Memory::DisableSteamOverlay(LPVOID lpAddr, UINT nSize) {
	for (UINT i = 0; i < nSize; i++) {
		*((char*)lpAddr + i) = 0x90;
	}
}