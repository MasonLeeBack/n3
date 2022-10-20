// Copyright © 2005 Microsoft Corporation

#pragma once

#include "MGSToolboxConfig.h"
#ifdef USE_MGSTOOLBOX

namespace MGSToolbox
{
	bool translateKeystroke ( int vkey, int flags, char & outChar );

	void dprintf ( const char * c, ... );

	extern IDirect3DDevice9 * g_pDevice;

	//----------

	struct SimpleRect
	{
		float x1, y1, z1;
		DWORD color1;

		float x2, y2, z2;
		DWORD color2;

		float x3,y3,z3;
		DWORD color3;
	};

	SimpleRect * BeginRects ( int rectCount );
	void EndRects ( SimpleRect * cursor );

	void DrawRect ( SimpleRect *& cursor, int x1, int y1, int x2, int y2, DWORD color );
	void DrawRect ( int x1, int y1, int x2, int y2, DWORD color );
	void DrawRect2 ( int x, int y, int w, int h, DWORD color );

};

#endif // USE_MGSTOOLBOX