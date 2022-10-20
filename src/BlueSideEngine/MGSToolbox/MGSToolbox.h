// Copyright © 2005 Microsoft Corporation

#pragma once

#include "MGSToolboxConfig.h"
#ifdef USE_MGSTOOLBOX

namespace MGSToolbox
{
	void Install ( IDirect3DDevice9 * pDevice );
	void Remove  ( void );

	void BeginFrame ( void );
	void EndFrame   ( void );

	int  GetFrameCount ( void );
};

#else

namespace MGSToolbox
{
	inline void Install ( IDirect3DDevice9 * pDevice ) {};
	inline void Remove  ( void ) {};

	inline void BeginFrame ( void ) {};
	inline void EndFrame   ( void ) {};

	inline int  GetFrameCount ( void ) { return 0; };
};

#endif