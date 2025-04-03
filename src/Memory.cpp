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
		lpRelay = VirtualAlloc(lpLoc, 1, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
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
	DWORD relAddr = (DWORD_PTR)lpSrc - (DWORD_PTR)lpAddr - 5;
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

void Memory::Detour32(LPVOID lpSrc, LPVOID lpDst, UINT nMangledBytes) {
	DWORD dwOldProt;
	VirtualProtect(lpSrc, nMangledBytes, PAGE_EXECUTE_READWRITE, &dwOldProt);
	for (UINT i = 0; i < nMangledBytes; i++) {
		*((char*)lpSrc + i) = 0x90;
	}

	DWORD relAddr = (DWORD_PTR)lpDst - (DWORD_PTR)lpSrc - 5;

	*(char*)lpSrc = 0xE9;
	*(DWORD*)((char*)lpSrc + 1) = relAddr;
	VirtualProtect(lpSrc, nMangledBytes, dwOldProt, nullptr);
	return;
}

uintptr_t Memory::FindDMAAddy(uintptr_t ptr, std::vector<unsigned int> offsets) {
	uintptr_t addr = ptr;
	for (unsigned int i = 0; i < offsets.size(); ++i)
	{
		addr = *(uintptr_t*)addr;
		addr += offsets[i];
	}
	return addr;
}

std::vector<int> Memory::PatternToByte(const char* pattern) {
	std::vector<int> bytes;
	const char* current = pattern;

	while (*current) {
		if (*current == ' ') {
			++current;
			continue;
		}

		if (*current == '?') {
			bytes.push_back(-1);  // Wildcard
			if (*(current + 1) == '?') ++current;
		}
		else {
			bytes.push_back(static_cast<int>(std::strtoul(current, nullptr, 16)));
		}
		current += 2;
	}

	return bytes;
}

LPVOID Memory::FindBySignature(HMODULE hModule, const char* pattern) {
	if (!hModule || !pattern) return nullptr;

	MODULEINFO modInfo;
	if (!GetModuleInformation(GetCurrentProcess(), hModule, &modInfo, sizeof(modInfo)))
		return nullptr;

	uintptr_t start = reinterpret_cast<uintptr_t>(hModule);
	uintptr_t end = start + modInfo.SizeOfImage;

	std::vector<int> patternBytes = PatternToByte(pattern);
	size_t patternSize = patternBytes.size();

	for (uintptr_t addr = start; addr < end - patternSize; addr++) {
		SIZE_T bytesRead;
		std::vector<BYTE> buffer(patternSize);

		if (ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<LPCVOID>(addr), buffer.data(), patternSize, &bytesRead) &&
			bytesRead == patternSize) {

			bool found = true;
			for (size_t i = 0; i < patternSize; i++) {
				if (patternBytes[i] != -1 && buffer[i] != static_cast<BYTE>(patternBytes[i])) {
					found = false;
					break;
				}
			}

			if (found) {
				return reinterpret_cast<LPVOID>(addr);
			}
		}
	}

	return nullptr;
}