#ifndef HOOK_H_
#define HOOK_H_

#include <Windows.h>

extern HWND g_hwnd;

namespace Hook
{
	bool init();
	void shutdown();
	bool getIsInit();
}

#endif
