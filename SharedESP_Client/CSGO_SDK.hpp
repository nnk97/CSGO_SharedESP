#pragma once

#include "Main.hpp"

#include "dt_recv2.h"
#include "Vector.h"

namespace CSGO
{
	typedef float matrix3x4[3][4];

	template<typename OrgFunc>
	OrgFunc __forceinline CallVirtual(PVOID pClass, DWORD i) { return (OrgFunc)((*(PDWORD*)pClass)[i]); }

	struct NetvarOffsets
	{
	public:
		DWORD m_flSimulationTime;
		DWORD m_vecOrigin;
		DWORD m_iTeamNumber;
		DWORD m_iFlags;
		DWORD m_iLifeState;
		DWORD m_vecViewOffset;
	};
	extern std::unique_ptr<NetvarOffsets> g_pOffsets;

	struct player_info_t
	{
	private:
		int _pad[4];
	public:
		char m_Nickname[128];
	private:
		char _pad1[0x256];
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

	enum FontFlags_t
	{
		FONTFLAG_NONE,
		FONTFLAG_ITALIC = 0x001,
		FONTFLAG_UNDERLINE = 0x002,
		FONTFLAG_STRIKEOUT = 0x004,
		FONTFLAG_SYMBOL = 0x008,
		FONTFLAG_ANTIALIAS = 0x010,
		FONTFLAG_GAUSSIANBLUR = 0x020,
		FONTFLAG_ROTARY = 0x040,
		FONTFLAG_DROPSHADOW = 0x080,
		FONTFLAG_ADDITIVE = 0x100,
		FONTFLAG_OUTLINE = 0x200,
		FONTFLAG_CUSTOM = 0x400,		// custom generated font - never fall back to asian compatibility mode
		FONTFLAG_BITMAP = 0x800,		// compiled bitmap font - no fallbacks
	};

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
	extern DWORD g_hMainFont;

	class CEntity
	{
	public:
		bool IsAlive()
		{
			return (*(int*)((uintptr_t)this + g_pOffsets->m_iLifeState) == 0);
		}

		bool IsCrouching()
		{
			return (*(int*)((uintptr_t)this + g_pOffsets->m_iFlags) & (1 << 1)); // FL_DUCKING
		}

		bool IsDormant()
		{
			PVOID pNetworkable = (PVOID)((DWORD)this + 0x8);
			if (!pNetworkable)
				return false;
			typedef bool(__thiscall* OrgFunc)(PVOID);
			return CallVirtual<OrgFunc>(pNetworkable, 9)(pNetworkable);
		}

		int Index()
		{
			PVOID pNetworkable = (PVOID)((DWORD)this + 0x8);
			if (!pNetworkable)
				return false;
			typedef int(__thiscall* OrgFunc)(PVOID);
			return CallVirtual<OrgFunc>(pNetworkable, 10)(pNetworkable);
		}

		Vector GetOrigin()
		{
			return *(Vector*)((uintptr_t)this + g_pOffsets->m_vecOrigin);
		}

		Vector GetViewOffset()
		{
			return *(Vector*)((uintptr_t)this + g_pOffsets->m_vecViewOffset);
		}

		int GetTeamID()
		{
			return *(int*)((uintptr_t)this + g_pOffsets->m_iTeamNumber);
		}

		float GetSimulationTime()
		{
			return *(float*)((uintptr_t)this + g_pOffsets->m_flSimulationTime);
		}
	};

	class CEntityList
	{
	public:	
		CEntity* GetEntity(int iEntIndex)
		{
			typedef CEntity*(__thiscall* OrgFunc)(PVOID, int);
			return CallVirtual<OrgFunc>(this, 3)(this, iEntIndex);
		}
	};
	extern CEntityList* g_pEntityList;

	class Color
	{
	public:
		Color()
		{
			SetColor(0, 0, 0);
		}

		Color(int _r, int _g, int _b)
		{
			SetColor(_r, _g, _b);
		}

		void SetColor(int _r, int _g, int _b)
		{
			SetColor(_r, _g, _b, 255);
		}

		void SetColor(int _r, int _g, int _b, int _a)
		{
			_color[0] = _r;
			_color[1] = _g;
			_color[2] = _b;
			_color[3] = _a;
		}

		inline int r() const { return _color[0]; }
		inline int g() const { return _color[1]; }
		inline int b() const { return _color[2]; }
		inline int a() const { return _color[3]; }

		static Color PrintDefault() { return Color(255, 122, 122); }

	private:
		unsigned char _color[4];
	};

	class CVarInterface
	{
	public:
		template<typename... Args>
		void const ConsoleColorPrintf(const Color& clr, const char *pFormat, Args... args)
		{
			typedef void(__cdecl* OrgFunc)(void*, const Color&, const char *, ...);
			return CallVirtual<OrgFunc>(this, 25)(this, clr, pFormat, args...);
		}

		template<typename... Args>
		void __inline DbgPrint(const char *pFormat, Args... args)
		{
			ConsoleColorPrintf(Color::PrintDefault(), pFormat, args...);
		}
	};
	extern CVarInterface* g_pCVar;

	class CEngine
	{
	public:
		void GetScreenSize(int& width, int& height)
		{
			typedef int(__thiscall* OrgFunc)(PVOID, int&, int&);
			CallVirtual<OrgFunc>(this, 5)(this, width, height);
		}

		int GetLocalPlayer()
		{
			typedef int(__thiscall* OrgFunc)(PVOID);
			return CallVirtual<OrgFunc>(this, 12)(this);
		}

		bool IsInGame()
		{
			typedef bool(__thiscall* OrgFunc)(PVOID);
			return CallVirtual<OrgFunc>(this, 26)(this);
		}

		bool IsConnected()
		{
			typedef bool(__thiscall* OrgFunc)(PVOID);
			return CallVirtual<OrgFunc>(this, 27)(this);
		}

		const matrix3x4& GetW2SMatrix()
		{
			typedef const matrix3x4&(__thiscall* OrgFunc)(PVOID);
			return CallVirtual<OrgFunc>(this, 37)(this);
		}

		bool GetPlayerInfo(int index, player_info_t* pInfo)
		{
			typedef bool(__thiscall* OrgFunc)(PVOID, int, player_info_t*);
			return CallVirtual<OrgFunc>(this, 8)(this, index, pInfo);
		}
	};
	extern CEngine* g_pEngine;

	class CServerIPFinder 
	{
	public:
		CServerIPFinder()
		{
			DWORD dwServerIPPtr = dwFindPattern(GetModuleHandleA("client.dll"), (PBYTE)"\x6A\x00\x68\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x8B\x74\x24\x1C", "xxx????x????xxxx");
			if (!dwServerIPPtr)
				MessageBoxA(NULL, "Failed to signature scan server IP ptr, manual update required!", "Error!", 0);
			dwServerIPPtr = *(DWORD*)(dwServerIPPtr + 3);
			m_ptr = (const char*)(dwServerIPPtr + 4);
		}
		
		const char* GetPtr() { return m_ptr; }
		bool IsLocalHost() { return (stricmp(m_ptr,"loopback:0") == 0); }

	private:
		const char* m_ptr = nullptr;

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

	};
	extern std::unique_ptr<CServerIPFinder> g_pServerIP;

	class CPanel
	{
	public:
		const char* GetPanelName(unsigned int iPanel)
		{
			typedef const char* (__thiscall* OrgFunc)(PVOID, unsigned int);
			return CallVirtual<OrgFunc>(this, 36)(this, iPanel);
		}
	};
	extern CPanel* g_pPanel;

	void InitializeSDK();

	void PlaceHooks();
	void RemoveHooks();
}
