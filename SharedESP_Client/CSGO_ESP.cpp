#include "CSGO_ESP.hpp"

namespace CSGO
{
	namespace ESP
	{
		int iLastWidth = 0, iLastHeight = 0;

		bool WorldToScreen(Vector &vOrigin, Vector &vScreen)
		{
			const matrix3x4& worldToScreen = g_pEngine->GetW2SMatrix();

			float w = worldToScreen[3][0] * vOrigin[0] + worldToScreen[3][1] * vOrigin[1] + worldToScreen[3][2] * vOrigin[2] + worldToScreen[3][3];
			vScreen.z = 0;
			if (w > 0.01)
			{
				float inverseWidth = 1 / w;
				vScreen.x = (iLastWidth / 2) + (0.5 * ((worldToScreen[0][0] * vOrigin[0] + worldToScreen[0][1] * vOrigin[1] + worldToScreen[0][2] * vOrigin[2] + worldToScreen[0][3]) * inverseWidth) * iLastWidth + 0.5);
				vScreen.y = (iLastHeight / 2) - (0.5 * ((worldToScreen[1][0] * vOrigin[0] + worldToScreen[1][1] * vOrigin[1] + worldToScreen[1][2] * vOrigin[2] + worldToScreen[1][3]) * inverseWidth) * iLastHeight + 0.5);
				return true;
			}
			return false;
		}

		void DrawPlayer(CEntity* pEntity, Vector vecLocalPos)
		{
			Vector vecHead, vecFeet, vecHeadScreen, vecFeetScreen;

			vecFeet = pEntity->GetOrigin();
			vecHead = Vector(vecFeet);
			vecHead.z += pEntity->IsCrouching() ? 54.f : 73.f;

			// Small offset to make boxes a bit biggers
			vecFeet.z -= 3.25f;
			vecHead.z += 3.25f;

			if (!WorldToScreen(vecFeet, vecFeetScreen) || !WorldToScreen(vecHead, vecHeadScreen))
				return;

			float flWidth = abs(vecFeetScreen.y - vecHeadScreen.y) / 3.35f;

			g_pSurface->DrawSetColor(0, 0, 0, 225);
			g_pSurface->DrawOutlinedRect(vecHeadScreen.x - flWidth - 1, vecHeadScreen.y - 1, vecFeetScreen.x + flWidth + 1, vecFeetScreen.y + 1);
			g_pSurface->DrawOutlinedRect(vecHeadScreen.x - flWidth + 1, vecHeadScreen.y + 1, vecFeetScreen.x + flWidth - 1, vecFeetScreen.y - 1);
			g_pSurface->DrawSetColor(255, 0, 0, 255);
			g_pSurface->DrawOutlinedRect(vecHeadScreen.x - flWidth, vecHeadScreen.y, vecFeetScreen.x + flWidth, vecFeetScreen.y);

			player_info_t pInfo;
			if (!g_pEngine->GetPlayerInfo(pEntity->Index(), &pInfo))
				return;

			wchar_t szString[128] = { '\0' };
			wsprintfW(szString, L"%S", pInfo.m_Nickname);

			int iWide, iTall;
			g_pSurface->GetTextSize(g_hMainFont, szString, iWide, iTall);

			g_pSurface->DrawSetTextFont(g_hMainFont);
			g_pSurface->DrawSetTextColor(255, 255, 255, 255);
			g_pSurface->DrawSetTextPos(vecHeadScreen.x - iWide / 2, vecHeadScreen.y - 15);
			g_pSurface->DrawPrintText(szString, wcslen(szString));

			Vector vecDelta = vecLocalPos - vecFeet;
			float flDistance = vecDelta.Length() * 0.01905f;
			
			char buffer[64];
			sprintf_s(buffer, "%.0f m", flDistance);
			wsprintfW(szString, L"%S", buffer);

			g_pSurface->GetTextSize(g_hMainFont, szString, iWide, iTall);

			g_pSurface->DrawSetTextPos(vecHeadScreen.x - iWide / 2, vecFeetScreen.y + 1);
			g_pSurface->DrawPrintText(szString, wcslen(szString));
		}

		void DrawPlayerFromCache(CEntity* pEntity, Vector vecLocalPos)
		{
			Vector vecHead, vecFeet, vecHeadScreen, vecFeetScreen;

			SyncData::CDataManager::PlayerData Data;
			SyncData::g_DataManager->GetLastRecord(pEntity->Index(), Data);

			float flLifetime = CSGO::g_pGlobals->curtime - Data.m_RecvTime;
			if (flLifetime > 1.f)
				return;

			int iAlpha = static_cast<int>((1.f - flLifetime) * 255.f);

			vecFeet = Data.m_Position;
			vecHead = Vector(vecFeet);
			vecHead.z += Data.m_Crouching ? 54.f : 73.f;

			// Small offset to make boxes a bit biggers
			vecFeet.z -= 3.25f;
			vecHead.z += 3.25f;

			if (!WorldToScreen(vecFeet, vecFeetScreen) || !WorldToScreen(vecHead, vecHeadScreen))
				return;

			float flWidth = abs(vecFeetScreen.y - vecHeadScreen.y) / 3.35f;

			g_pSurface->DrawSetColor(0, 0, 0, iAlpha);
			g_pSurface->DrawOutlinedRect(vecHeadScreen.x - flWidth - 1, vecHeadScreen.y - 1, vecFeetScreen.x + flWidth + 1, vecFeetScreen.y + 1);
			g_pSurface->DrawOutlinedRect(vecHeadScreen.x - flWidth + 1, vecHeadScreen.y + 1, vecFeetScreen.x + flWidth - 1, vecFeetScreen.y - 1);
			g_pSurface->DrawSetColor(255, 255, 0, iAlpha);
			g_pSurface->DrawOutlinedRect(vecHeadScreen.x - flWidth, vecHeadScreen.y, vecFeetScreen.x + flWidth, vecFeetScreen.y);

			player_info_t pInfo;
			if (!g_pEngine->GetPlayerInfo(pEntity->Index(), &pInfo))
				return;

			wchar_t szString[128] = { '\0' };
			wsprintfW(szString, L"%S", pInfo.m_Nickname);

			int iWide, iTall;
			g_pSurface->GetTextSize(g_hMainFont, szString, iWide, iTall);

			g_pSurface->DrawSetTextFont(g_hMainFont);
			g_pSurface->DrawSetTextColor(255, 255, 255, iAlpha);
			g_pSurface->DrawSetTextPos(vecHeadScreen.x - iWide / 2, vecHeadScreen.y - 15);
			g_pSurface->DrawPrintText(szString, wcslen(szString));

			Vector vecDelta = vecLocalPos - vecFeet;
			float flDistance = vecDelta.Length() * 0.01905f;

			char buffer[64];
			sprintf_s(buffer, "%.0f m", flDistance);
			wsprintfW(szString, L"%S", buffer);

			g_pSurface->GetTextSize(g_hMainFont, szString, iWide, iTall);

			g_pSurface->DrawSetTextPos(vecHeadScreen.x - iWide / 2, vecFeetScreen.y + 1);
			g_pSurface->DrawPrintText(szString, wcslen(szString));
		}

		void OnPaintTraverse()
		{
			CEntity* pLocal = g_pEntityList->GetEntity(g_pEngine->GetLocalPlayer());
			if (!pLocal)
				return;

			Vector vecLocalPosition = pLocal->GetOrigin() + pLocal->GetViewOffset();
			int iLocalTeam = pLocal->GetTeamID();

			// Loop through players and draw them out
			for (int i = 1; i < 64; i++)
			{
				CEntity* pEntity = g_pEntityList->GetEntity(i);
				if (!pEntity || pEntity == pLocal)
					continue;
				if (pEntity->GetTeamID() == iLocalTeam)
					continue;
				if (!pEntity->IsAlive())
					continue;

				pEntity->IsDormant() ? DrawPlayerFromCache(pEntity, vecLocalPosition) : DrawPlayer(pEntity, vecLocalPosition);
			}
		}

		PaintTraverseFn orgPaintTraverse = nullptr;
		void __fastcall hkPaintTraverse(void* ECX, void* EDX, unsigned int iPanel, bool bForceRepaint, bool bAllowForce)
		{
			// Call to original function
			orgPaintTraverse(ECX, iPanel, bForceRepaint, bAllowForce);

			const char* pszPanelName = g_pPanel->GetPanelName(iPanel);	
			if (pszPanelName[0] == 'M' && pszPanelName[12] == 'P') //MatSystemTopPanel
			{
				//Handle resolution changes
				int iWidth, iHeight;
				g_pEngine->GetScreenSize(iWidth, iHeight);
				if (iWidth != iLastWidth || iHeight != iLastHeight)
				{
					iLastWidth = iWidth;
					iLastHeight = iHeight;
					g_pSurface->SetFontGlyphSet(g_hMainFont, "Verdana", 12, 600, 0, 0, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
				}

				if (g_pEngine->IsInGame() && g_pEngine->IsConnected())
					OnPaintTraverse();
			}
		}
	}
}