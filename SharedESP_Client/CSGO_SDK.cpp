#include "CSGO_SDK.hpp"

namespace CSGO
{
	std::unique_ptr<NetvarOffsets> g_pOffsets = nullptr;

	CHLCLient* g_pClient = nullptr;
	CSurface* g_pSurface = nullptr;
	CEntityList* g_pEntityList = nullptr;
	CVarInterface* g_pCVar = nullptr;
	CEngine* g_pEngine = nullptr;
	PVOID g_pPanel = nullptr;

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

	const char* CEntity::GetNickname()
	{
		player_info_t pInfo;
		if (!g_pEngine->GetPlayerInfo(Index(), &pInfo))
			return nullptr;
		return pInfo.m_Nickname;
	}

	void InitializeSDK()
	{
		// Grab all the interfaces out of the game
		g_pCVar = GetInterface<CVarInterface>("vstdlib.dll", "VEngineCvar");
		g_pClient = GetInterface<CHLCLient>("client.dll", "VClient");
		g_pSurface = GetInterface<CSurface>("vguimatsurface.dll", "VGUI_Surface");
		g_pEntityList = GetInterface<CEntityList>("client.dll", "VClientEntityList");//
		g_pEngine = GetInterface<CEngine>("engine.dll", "VEngineClient");
		g_pPanel = GetInterface<void>("vgui2.dll", "VGUI_Panel");

		// Grab network variables
		std::unique_ptr<CNetVars> pNVManager = std::make_unique<CNetVars>();
		g_pOffsets = std::make_unique<NetvarOffsets>();
		g_pOffsets->m_iFlags = pNVManager->GetOffset("DT_BasePlayer", "m_fFlags");
		g_pOffsets->m_vecOrigin = pNVManager->GetOffset("DT_BaseEntity", "m_vecOrigin");
		g_pOffsets->m_iLifeState = pNVManager->GetOffset("DT_BasePlayer", "m_lifeState");
		g_pOffsets->m_flSimulationTime = pNVManager->GetOffset("DT_CSPlayer", "m_flSimulationTime");
		g_pOffsets->m_iTeamNumber = pNVManager->GetOffset("DT_BaseEntity", "m_iTeamNum");

		g_pCVar->DbgPrint("  >>>  CSGO_SharedESP SDK loaded!\n");
	}

	std::unique_ptr<CVMTHookManager> g_pPanelHook;

	void PlaceHooks()
	{
		g_pPanelHook = std::make_unique<CVMTHookManager>((PDWORD*)g_pPanel);
		g_pPanelHook->dwHookMethod(NULL/*painttraverse*/, 41);
	}

	void RemoveHooks()
	{
		g_pPanelHook->UnHook();
	}
}
