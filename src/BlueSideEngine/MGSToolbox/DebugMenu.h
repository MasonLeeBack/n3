// Copyright © 2005 Microsoft Corporation

#pragma once

#include "MGSToolboxConfig.h"
#ifdef USE_MGSTOOLBOX

namespace MGSToolbox {

//-----------------------------------------------------------------------------

class DebugMenu
{
public:

	static void LoadINI ( void );
	static void SaveINI ( void );

	static void LoadINI ( char const * fileName );
	static void SaveINI ( char const * fileName );

	static void Install ( void );
	static void Remove  ( void );

	static void Update  ( void );
	static void Render  ( void );

	static bool s_bInstalled;
};

//-----------------------------------------------------------------------------

}; // namespace MGSToolbox {

#endif // USE_MGSTOOLBOX