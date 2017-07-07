#pragma once

#include "Main.hpp"

#include "dt_recv2.h"
#include "Vector.h"

namespace CSGO
{
	template<typename OrgFunc>
	OrgFunc __forceinline CallVirtual(PVOID pClass, DWORD i) { return (OrgFunc)((*(PDWORD*)pClass)[i]); }

	typedef void* (__cdecl* CreateInterface_t)(const char*, int*);
	typedef void* (*CreateInterfaceFn)(const char* pName, int* pReturnCode);

	struct NetvarOffsets
	{
	public:
		DWORD m_flSimulationTime;
		DWORD m_vecOrigin;
		DWORD m_iTeamNumber;
		DWORD m_iFlags;
	};
	extern std::unique_ptr<NetvarOffsets> g_pOffsets;

	struct player_info_t
	{
	private:
		int _pad[4];
	public:
		char m_Nickname[128];
	};

	class ClientClass
	{
	public:
		const char* GetName(void)
		{
			return *(char**)(this + 0x8);
		}
		RecvTable* GetTable()
		{
			return *(RecvTable**)(this + 0xC);
		}
		ClientClass* NextClass()
		{
			return *(ClientClass**)(this + 0x10);
		}
		int GetClassID(void)
		{
			return *(int*)(this + 0x14);
		}
	};

	class CHLCLient
	{
	public:
		ClientClass* GetClientClasses()
		{
			typedef ClientClass*(__thiscall* OrgFunc)(PVOID);
			return CallVirtual<OrgFunc>(this, 8)(this);
		}
	};
	extern CHLCLient* g_pClient;

	class CSurface
	{
	public:
		void DrawSetColor(int r, int g, int b, int a)
		{
			typedef void(__thiscall* OrgFunc)(PVOID, int, int, int, int);
			CallVirtual<OrgFunc>(this, 15)(this, r, g, b, a);
		}

		void DrawOutlinedRect(int x0, int y0, int x1, int y1)
		{
			typedef void(__thiscall* OrgFunc)(void*, int x0, int y0, int x1, int y1);
			CallVirtual<OrgFunc>(this, 18)(this, x0, y0, x1, y1);
		}

		void DrawSetTextFont(DWORD font)
		{
			typedef void(__thiscall* OrgFunc)(PVOID, DWORD);
			CallVirtual<OrgFunc>(this, 23)(this, font);
		}

		void DrawSetTextColor(int r, int g, int b, int a)
		{
			typedef void(__thiscall* OrgFunc)(PVOID, int, int, int, int);
			CallVirtual<OrgFunc>(this, 25)(this, r, g, b, a);
		}

		void DrawSetTextPos(int x, int y)
		{
			typedef void(__thiscall* OrgFunc)(PVOID, int, int);
			CallVirtual<OrgFunc>(this, 26)(this, x, y);
		}

		void DrawPrintText(const wchar_t *text, int textLen)
		{
			typedef void(__thiscall* OrgFunc)(PVOID, const wchar_t*, int, int);
			return CallVirtual<OrgFunc>(this, 28)(this, text, textLen, 0);
		}

		DWORD CreateHLFont()
		{
			typedef DWORD(__thiscall* OrgFunc)(PVOID);
			return CallVirtual<OrgFunc>(this, 71)(this);
		}

		void SetFontGlyphSet(DWORD& font, const char* windowsFontName, int tall, int weight, int blur, int scanlines, int flags)
		{
			typedef void(__thiscall* OrgFunc)(PVOID, DWORD, const char*, int, int, int, int, int, int, int);
			CallVirtual<OrgFunc>(this, 72)(this, font, windowsFontName, tall, weight, blur, scanlines, flags, 0, 0);
		}

		void GetTextSize(DWORD font, const wchar_t *text, int &wide, int &tall)
		{
			typedef void(__thiscall* OrgFunc)(void*, DWORD, const wchar_t*, int&, int&);
			CallVirtual<OrgFunc>(this, 79)(this, font, text, wide, tall);
		}
	};
	extern CSurface* g_pSurface;

	class CEntity
	{
	public:
		bool IsAlive()
		{
			return (*(int*)((uintptr_t)this + g_pOffsets->m_iFlags) == 0);
		}

		bool IsDormant()
		{
			PVOID pNetworkable = (PVOID)((DWORD)this + 0x8);
			if (!pNetworkable)
				return false;
			typedef bool(__thiscall* OrgFunc)(PVOID);
			return CallVirtual<OrgFunc>(pNetworkable, 9)(pNetworkable);
		}

		Vector GetOrigin()
		{
			return *(Vector*)((uintptr_t)this + g_pOffsets->m_vecOrigin);
		}

		const char* GetNickname()
		{
			return nullptr;
		}
	};

	class CEntityList
	{
	public:	
		CEntity* GetEntity(int i)
		{
			typedef CEntity*(__thiscall* OrgFunc)(PVOID, int);
			return CallVirtual<OrgFunc>(this, 3)(this, i);
		}
	};
	extern CEntityList* g_pEntityList;

	void InitializeSDK();
}
