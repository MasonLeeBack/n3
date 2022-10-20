// Copyright © 2005 Microsoft Corporation

// Prefab shaders for doing toolbox rendering stuff

#pragma once

#include "MGSToolboxConfig.h"
#ifdef USE_MGSTOOLBOX

namespace MGSToolbox
{
	namespace Shaders
	{
		void Install ( void );
		void Remove  ( void );

		extern IDirect3DVertexShader9 * g_vsPassthruPC;
		extern IDirect3DVertexShader9 * g_vsPassthruPCT;

		extern IDirect3DPixelShader9 * g_psPassthruC;
		extern IDirect3DPixelShader9 * g_psMultiplyCT;

		extern const DWORD g_vsdPassthruPC[];
		extern const DWORD g_vsdPassthruPCT[];

		extern const DWORD g_psdPassthruC[];
		extern const DWORD g_psdMultiplyCT[];
	};
};

#endif // USE_MGSTOOLBOX