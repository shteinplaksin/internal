#ifndef STEALTH_H_
#define STEALTH_H_

#include <Windows.h>

namespace Stealth
{
	void ErasePEHeader(HINSTANCE hModule);
	bool RemoveFromPEB(HINSTANCE hModule);
	bool CheckDebugger();
	void InitStealth(HINSTANCE hModule);
}

#endif

