#include "CSGO_ESP.hpp"

namespace CSGO
{
	namespace ESP
	{
		PaintTraverseFn orgPaintTraverse = nullptr;

		void __fastcall hkPaintTraverse(void* ECX, void* EDX, unsigned int iPanel, bool bForceRepaint, bool bAllowForce)
		{


			// Call to original function
			orgPaintTraverse(ECX, iPanel, bForceRepaint, bAllowForce);
		}
	}
}