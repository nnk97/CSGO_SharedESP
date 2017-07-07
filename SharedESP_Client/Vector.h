#pragma once

#ifdef D3DXVECTOR3
	#define Vector D3DXVECTOR3
#else

	class Vector
	{
	public:
		float x, y, z;
	};

#endif
