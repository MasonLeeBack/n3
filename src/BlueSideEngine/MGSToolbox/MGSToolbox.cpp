// Copyright 2005 Microsoft Corporation

#include "stdafx.h"
#include "MGSToolBox.h"

#ifdef USE_MGSTOOLBOX

#include <xtl.h>
#include <assert.h>

#include "Utils.h"
#include "Shaders.h"
#include "Profiler.h"
#include "SimpleFont.h"
#include "InputHook.h"
#include "DebugMenu.h"
#include "DebugValue.h"
#include "DebugCommand.h"
#include "DebugConsole.h"

#include "BsMem.h"

extern void BsKernelUpdate();

namespace MGSToolbox {

//-----------------------------------------------------------------------------

int g_frameCount = 0;

DebugValue g_debugMenuEnabled ( "Debug Menu", "Enable", false );

//-----------------------------------------------------------------------------

void Install ( IDirect3DDevice9 * pDevice )
{
	assert(pDevice);

	g_pDevice = pDevice;

	Shaders::Install();

	ProfileManager::Install();

	DebugMenu::Install();

	DebugConsole::Install();
}

//----------

void Remove ( void )
{
	DebugConsole::Remove();

	DebugMenu::Remove();

	ProfileManager::Remove();

	Shaders::Remove();

	g_pDevice = NULL;
}

//-----------------------------------------------------------------------------

void BeginFrame ( void )
{
	ProfileManager::BeginFrame();

	//----------

	InputHook::Update();

	if(InputHook::Enabled())
	{
		XINPUT_KEYSTROKE key;

		while(XInputGetKeystroke(0,XINPUT_FLAG_KEYBOARD,&key) == ERROR_SUCCESS)
		{
			// Send the keystrokes off to all the toolbox systems that want them

			Keyboard::AddKey(key.VirtualKey,key.Flags);
			DebugConsole::AddKey(key.VirtualKey,key.Flags);
		}
	}

	//----------

	DebugConsole::Update();
}

//----------

void EndFrame ( void )
{
	g_frameCount++;

	if(InputHook::Enabled())
	{
		SimpleFont::Printf(100,60,"Toolbox input ENABLED");
	}
	else
	{
		SimpleFont::Printf(100,60,"Toolbox input DISABLED"); 
	}

	//----------

	BsMem::Update();
	BsMem::Display(100,120);

	//----------

	BsKernelUpdate();

	//----------

	DebugConsole::Render();

	//----------
    
	if(g_gamepadClicks & XINPUT_GAMEPAD_START)
	{
		g_debugMenuEnabled.Increment();
	}

	if(g_debugMenuEnabled.asBool())
	{
		CPUProfiler prof("Debug Menu",0xFFFFFF00);

		DebugMenu::Update();
		DebugMenu::Render();
	}

	//----------

	ProfileManager::EndFrame();

	ProfileRenderer::DrawGraphs();
}

//----------

int GetFrameCount ( void )
{
	return g_frameCount;
}

//-----------------------------------------------------------------------------

};

#endif // USE_MGSTOOLBOX