#include "CSGO_SDK.hpp"

namespace CSGO
{
	std::unique_ptr<NetvarOffsets> g_pOffsets;

	CHLCLient* g_pClient;
	CSurface* g_pSurface;
	CEntityList* g_pEntityList;

	void InitializeSDK()
	{
		g_pOffsets = std::make_unique<NetvarOffsets>();

		// Grab all the interfaces out of the game

	}
}
