#include "Main.hpp"

void Initialize()
{
	// Hook CSGO drawing functions
	CSGO::InitializeSDK();
	CSGO::PlaceHooks();

	// Spawn update thread to send/recv data
	SyncData::g_DataManager = std::make_unique<SyncData::CDataManager>();
}

int __stdcall DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Initialize, 0, 0, 0);
	return TRUE;
}

