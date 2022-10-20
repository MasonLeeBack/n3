// Copyright © 2005 Microsoft Corporation

#pragma once

#include "MGSToolboxConfig.h"
#ifdef USE_MGSTOOLBOX

namespace MGSToolbox {

//-----------------------------------------------------------------------------

class DebugConsole
{
public:

	static void AddKey ( int vkey, int flags );

	static void Install ( void );
	static void Remove  ( void );

	static void Update  ( void );
	static void Render  ( void );

	static bool s_bInstalled;
};

//-----------------------------------------------------------------------------

}; // namespace MGSToolbox {

#endif // USE_MGSTOOLBOX