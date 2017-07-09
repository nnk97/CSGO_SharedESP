#include "CSGO_SDK.hpp"

namespace CSGO
{
	std::unique_ptr<NetvarOffsets> g_pOffsets = nullptr;
	std::unique_ptr<CServerIPFinder> g_pServerIP = nullptr;

	CHLCLient* g_pClient = nullptr;
	CSurface* g_pSurface = nullptr;
	CEntityList* g_pEntityList = nullptr;
	CVarInterface* g_pCVar = nullptr;
	CEngine* g_pEngine = nullptr;
	CPanel* g_pPanel = nullptr;
	GlobalVars_t* g_pGlobals = nullptr;
	
	DWORD g_hMainFont = NULL;

	typedef void* (__cdecl* CreateInterface_t)(const char*, int*);
	typedef void* (*CreateInterfaceFn)(const char* pName, int* pReturnCode);

	template<typename Interface>
	Interface* GetInterface(const char* pszModule, const char* pszInterface)
	{
		int i = 0;
		char Buffer[64] = { 0 };
		Interface* Outcome = nullptr;
		auto Factory = (CreateInterfaceFn)GetProcAddress(GetModuleHandleA(pszModule), "CreateInterface");

		do
		{
			sprintf_s(Buffer, "%s%03i", pszInterface, i++);
			Outcome = (Interface*)Factory(Buffer, NULL);
			if (i >= 100)
				MessageBoxA(NULL, "Failed to find one of the interfaces, manual update required!", "Error!", 0);
		} while (!Outcome);

		if (g_pCVar)
			g_pCVar->DbgPrint("\t%s => [ 0x%08X ]\n", pszInterface, Outcome);

		return Outcome;
	}

	bool bDataCompare(const BYTE *pData, const BYTE *bMask, const char *szMask)
	{
		for (; *szMask; ++szMask, ++pData, ++bMask)
			if (*szMask == 'x' && *pData != *bMask)
				return false;
		return (*szMask) == NULL;
	}

	DWORD dwFindPattern(HMODULE hModule, const BYTE* bMask, char* szMask)
	{
		MODULEINFO ModuleInfo;
		GetModuleInformation(GetCurrentProcess(), hModule, &ModuleInfo, sizeof(MODULEINFO));
		for (DWORD i = 0; i < ModuleInfo.SizeOfImage; i++)
			if (bDataCompare((BYTE*)((DWORD)hModule + i), bMask, szMask))
				return (DWORD)((DWORD)hModule + i);
		return NULL;
	}

	CServerIPFinder::CServerIPFinder()
	{
		DWORD dwServerIPPtr = dwFindPattern(GetModuleHandleA("client.dll"), (PBYTE)"\x6A\x00\x68\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x8B\x74\x24\x1C", "xxx????x????xxxx");
		if (!dwServerIPPtr)
			MessageBoxA(NULL, "Failed to signature scan server IP ptr, manual update required!", "Error!", 0);
		dwServerIPPtr = *(DWORD*)(dwServerIPPtr + 3);
		m_ptr = (const char*)(dwServerIPPtr + 4);
	}

	void InitializeSDK()
	{
		// Grab all the interfaces out of the game
		g_pCVar = GetInterface<CVarInterface>("vstdlib.dll", "VEngineCvar");
		g_pClient = GetInterface<CHLCLient>("client.dll", "VClient");
		g_pSurface = GetInterface<CSurface>("vguimatsurface.dll", "VGUI_Surface");
		g_pEntityList = GetInterface<CEntityList>("client.dll", "VClientEntityList");
		g_pEngine = GetInterface<CEngine>("engine.dll", "VEngineClient");
		g_pPanel = GetInterface<CPanel>("vgui2.dll", "VGUI_Panel");

		// C casting, lols
		g_pGlobals = **(GlobalVars_t***)(dwFindPattern(GetModuleHandleA("client.dll"), (PBYTE)"\xA1\x00\x00\x00\x00\x8B\x4D\xFC\x8B\x55\x08", "x????xxxxxx") + 1);

		// Grab network variables
		std::unique_ptr<CNetVars> pNVManager = std::make_unique<CNetVars>();
		g_pOffsets = std::make_unique<NetvarOffsets>();
		g_pOffsets->m_iFlags = pNVManager->GetOffset("DT_BasePlayer", "m_fFlags");
		g_pOffsets->m_vecOrigin = pNVManager->GetOffset("DT_BaseEntity", "m_vecOrigin");
		g_pOffsets->m_iLifeState = pNVManager->GetOffset("DT_BasePlayer", "m_lifeState");
		g_pOffsets->m_flSimulationTime = pNVManager->GetOffset("DT_CSPlayer", "m_flSimulationTime");
		g_pOffsets->m_iTeamNumber = pNVManager->GetOffset("DT_BaseEntity", "m_iTeamNum");
		g_pOffsets->m_vecViewOffset = pNVManager->GetOffset("DT_BasePlayer", "m_vecViewOffset[0]");

		g_pServerIP = std::make_unique<CServerIPFinder>();
		
		g_hMainFont = g_pSurface->CreateHLFont();

		g_pCVar->DbgPrint("  >>>  CSGO_SharedESP SDK loaded!\n");
	}
	
	// Hooking VTables
	std::unique_ptr<CVMTHookManager> g_pPanelHook;
	std::unique_ptr<CVMTHookManager> g_pClientHook;

	typedef void(__thiscall* ShutdownFn)(PVOID);
	ShutdownFn orgShutdown = nullptr;
	void __fastcall hkShutdown(void* ecx, void* edx)
	{
		SyncData::g_DataManager->m_bExit = true;
		if (SyncData::g_DataManager->m_Thread.joinable())
			SyncData::g_DataManager->m_Thread.join();

		orgShutdown(ecx);
	}

	typedef void(__thiscall* FrameStageNotifyFn)(PVOID, int);
	FrameStageNotifyFn orgFrameStageNotify = nullptr;
	void __fastcall hkFrameStageNotify(void* ecx, void* edx, int iStage)
	{
		orgFrameStageNotify(ecx, iStage);

		// On stage: FRAME_NET_UPDATE_POSTDATAUPDATE_END
		if (iStage == 3)
		{
			CEntity* pLocal = g_pEntityList->GetEntity(g_pEngine->GetLocalPlayer());
			if (!pLocal)
				return;

			int iLocalTeam = pLocal->GetTeamID();

			for (int i = 1; i < 64; i++)
			{
				CEntity* pEntity = g_pEntityList->GetEntity(i);
				if (!pEntity || pEntity->GetTeamID() == iLocalTeam || !pEntity->IsAlive())
				{
					SyncData::g_DataManager->MarkAsInvalidEntity(i);
					continue;
				}

				SyncData::CDataManager::PlayerData PD;
				SyncData::g_DataManager->GetLastRecord(i, PD);

				if (pEntity->IsDormant() || pEntity->GetSimulationTime() == 0.f || pEntity->GetSimulationTime() < PD.m_SimulationTime)
				{
					SyncData::g_DataManager->SetQueryStatus(i, true);
					SyncData::g_DataManager->SetSendingStatus(i, false);
				}
				else
				{
					SyncData::g_DataManager->SetQueryStatus(i, false);

					// Got new data for the server?
					if (pEntity->GetSimulationTime() > PD.m_SimulationTime)
						SyncData::g_DataManager->SetSendingStatus(i, true);

					SyncData::g_DataManager->SetLastRecord(i, pEntity->IsCrouching(), pEntity->GetSimulationTime(), pEntity->GetOrigin());
				}
			}
		}
	}

	void PlaceHooks()
	{
		g_pPanelHook = std::make_unique<CVMTHookManager>((PDWORD*)g_pPanel);
		ESP::orgPaintTraverse = (ESP::PaintTraverseFn)g_pPanelHook->dwHookMethod((DWORD)ESP::hkPaintTraverse, 41);

		g_pClientHook = std::make_unique<CVMTHookManager>((PDWORD*)g_pClient);
		orgShutdown = (ShutdownFn)g_pClientHook->dwHookMethod((DWORD)hkShutdown, 4);
		orgFrameStageNotify = (FrameStageNotifyFn)g_pClientHook->dwHookMethod((DWORD)hkFrameStageNotify, 36);
	}

	void RemoveHooks()
	{
		g_pPanelHook->UnHook();
		g_pClientHook->UnHook();
	}
}
