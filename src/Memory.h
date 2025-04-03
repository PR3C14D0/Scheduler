#pragma once
#include <iostream>
#include <Windows.h>
#include <vector>
#include <Psapi.h>

namespace Memory {
	LPVOID CreateHook(LPVOID lpSrc, LPVOID lpDst, UINT nMangledBytes, LPVOID& lpGateway);

	void DisableSteamOverlay(LPVOID lpAddr, UINT nSize);

	void Detour32(LPVOID lpSrc, LPVOID lpDst, UINT nMangledBytes);

	uintptr_t FindDMAAddy(uintptr_t ptr, std::vector<unsigned int> offsets);
	std::vector<int> PatternToByte(const char* pattern);
	LPVOID FindBySignature(HMODULE hModule, const char* pattern);
}