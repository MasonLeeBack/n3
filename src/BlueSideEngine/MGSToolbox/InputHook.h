// Copyright © 2005 Microsoft Corporation

#pragma once

#include "MGSToolboxConfig.h"
#if defined(USE_MGSTOOLBOX)

namespace MGSToolbox
{
	namespace InputHook
	{
		// hook

		void Update  ( void );
		bool Enabled ( void );

		// To connect the input hook to the client game, replace calls to XInputGetState with MGSToolbox::InputHook::GetState

		DWORD GetState ( DWORD dwUserIndex, PXINPUT_STATE pState );
	};

	//----------
	// Toolbox input

	namespace Keyboard
	{
		void AddKey     ( int vkey, int flags );
		bool GetClicked ( int vkey );
	};

	extern int g_gamepadClicks;
	extern int g_gamepadState;
};

#elif defined(_XBOX)

namespace MGSToolbox
{
	namespace InputHook
	{
        inline DWORD GetState ( DWORD dwUserIndex, PXINPUT_STATE pState ) { return XInputGetState( dwUserIndex, pState ); }
	}
};

#endif
