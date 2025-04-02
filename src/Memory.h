#pragma once
#include <iostream>
#include <Windows.h>

namespace Memory {
	LPVOID CreateHook(LPVOID lpSrc, LPVOID lpDst, UINT nMangledBytes, LPVOID& lpGateway);

	void DisableSteamOverlay(LPVOID lpAddr, UINT nSize);

	void Detour32(LPVOID lpSrc, LPVOID lpDst, UINT nMangledBytes);
}