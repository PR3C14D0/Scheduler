#pragma once
#include <iostream>
#include <Windows.h>

namespace Memory {
	LPVOID CreateHook(LPVOID lpSrc, LPVOID lpDst, UINT nMangledBytes, LPVOID& lpGateway);
}