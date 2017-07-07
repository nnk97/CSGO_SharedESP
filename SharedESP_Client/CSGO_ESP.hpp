#pragma once

#include "Main.hpp"

namespace CSGO
{
	namespace ESP
	{
		typedef void(__thiscall *PaintTraverseFn)(PVOID, unsigned int, bool, bool);
		extern PaintTraverseFn orgPaintTraverse;
		extern void __fastcall hkPaintTraverse(void* ECX, void* EDX, unsigned int iPanel, bool bForceRepaint, bool bAllowForce);
	}
}