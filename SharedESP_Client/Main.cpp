#include "Main.hpp"

void Initialize()
{
	// Let's try to connec to the server first


	// Hook CSGO drawing functions
	CSGO::InitializeSDK();
	CSGO::PlaceHooks();

	// Spawn update thread to send/recv data
	SyncData::g_DataManager = std::make_unique<SyncData::CDataManager>();
}

void CleanUp()
{
	CSGO::RemoveHooks();
}

int __stdcall DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Initialize, 0, 0, 0);
	else if (dwReason == DLL_PROCESS_DETACH)
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)CleanUp, 0, 0, 0);
	return TRUE;
}

