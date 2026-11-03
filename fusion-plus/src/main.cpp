#include "main.h"


void Main::Init()
{
	Base::Init();
}

void Main::Kill()
{
	Base::Kill();
	// Keep the DLL and hook trampolines resident until the JVM exits: callbacks
	// already in flight can still return through them after hooks are disabled.
	ExitThread(0);
}

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		Main::HModule = hModule;
		DisableThreadLibraryCalls(hModule);

		HANDLE hThread = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(Main::Init), hModule, 0, nullptr);

		if (hThread) CloseHandle(hThread);
	}

	return TRUE;
}

