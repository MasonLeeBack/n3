// Copyright 2005 Microsoft Corporation

#include "stdafx.h"
#include "DebugConsole.h"

#ifdef USE_MGSTOOLBOX

#include <xtl.h>
#include <stdio.h>
#include <assert.h>

#include "InputHook.h"
#include "SimpleFont.h"
#include "DebugCommand.h"
#include "DebugValue.h"
#include "TextInput.h"
#include "TextMatch.h"
#include "GPUStateSave.h"
#include "Utils.h"
#include "Console.h"

namespace MGSToolbox {

DebugValue g_debugConsoleOpacity ( "Debug Console", "Opacity",  128,1,255,1);

DebugValue g_debugConsoleHeight  ( "Debug Console", "Height",   50,2,50,1);
DebugValue g_debugConsoleWidth   ( "Debug Console", "Width",    80,20,80,1);

DebugValue g_debugConsoleX       ( "Debug Console", "Window X", 100,0,1280,1);
DebugValue g_debugConsoleY       ( "Debug Console", "Window Y", 100,0,720,1);

DebugValue g_debugConsoleEnable  ( "Debug Console", "Enable", false);

TextInput g_input(256);

bool DebugConsole::s_bInstalled = false;

Console * g_pConsole = NULL;

//-----------------------------------------------------------------------------

void DebugConsole::AddKey ( int vkey, int flags )
{
	// mask off tilde so we can use it to control the console

	if(vkey != VK_OEM_3)
	{
		g_input.AddKey(vkey,flags);
	}
}

//-----------------------------------------------------------------------------

void DebugConsole::Install ( void )
{
	assert(!s_bInstalled);

	s_bInstalled = true;

	g_pConsole = new Console ( g_debugConsoleX.asInt(),
	                           g_debugConsoleY.asInt(),
	                           g_debugConsoleWidth.asInt(),
	                           g_debugConsoleHeight.asInt(),
	                           g_debugConsoleOpacity.asInt() );
}

void DebugConsole::Remove ( void )
{
	assert(s_bInstalled);

	delete g_pConsole;
	g_pConsole = NULL;

	s_bInstalled = false;
}

//-----------------------------------------------------------------------------

void DebugConsole::Update ( void )
{
	if(!s_bInstalled) return;

	// If the user has hit the tilde key, toggle the console

	if(Keyboard::GetClicked(VK_OEM_3))
	{
		g_debugConsoleEnable.Increment();

		if(g_debugConsoleEnable.asBool())
		{
			g_input.Clear();
		}
	}

	// If the console isn't enabled, don't continue updating

	if(!g_debugConsoleEnable.asBool()) return;

	// If the console size has changed, clear the console

	if(g_debugConsoleWidth.isDirty() || g_debugConsoleHeight.isDirty())
	{
		g_pConsole->SetWidth(g_debugConsoleWidth.asInt());
		g_pConsole->SetHeight(g_debugConsoleHeight.asInt());

		g_debugConsoleWidth.setDirty(false);
		g_debugConsoleHeight.setDirty(false);
	}


	static char lastCommand[256] = "";

	// When the user presses up, copy the last command back to the input string

	if(Keyboard::GetClicked(VK_UP))
	{
		g_input.SetContents(lastCommand);
	}

	// When the user presses return, run the command and print the command + the command's result to the console

	if(Keyboard::GetClicked(VK_RETURN))
	{
		strncpy(lastCommand,g_input.GetContents(),256);

		char * commandOutput = new char[16384];

		commandOutput[0] = 0;

		g_pConsole->Printf(">%s",g_input.GetContents());

		DebugCommand::Run(g_input.GetContents(),commandOutput,16384);

		g_pConsole->Print(commandOutput);

		g_input.Clear();

		delete commandOutput;
	}
}

//----------

void DebugConsole::Render ( void )
{
	if(!s_bInstalled) return;
	if(!g_debugConsoleEnable.asBool()) return;

	int consoleX = g_debugConsoleX.asInt();
	int consoleY = g_debugConsoleY.asInt();
	int consoleWidth = g_debugConsoleWidth.asInt();
	int consoleHeight = g_debugConsoleHeight.asInt();

	//----------
	// draw console contents

	int opacity = g_debugConsoleOpacity.asInt();;

	g_pConsole->SetOpacity(opacity);
	g_pConsole->Render();

	//----------
	// draw input line

	{
		int x = consoleX;
		int y = consoleY + consoleHeight * SimpleFont::getHeight();

		int length = g_debugConsoleWidth.asInt();

		char buffer[80];

		memset(buffer,0,length);

		_snprintf(buffer,length,">%s",g_input.GetContents());

		SimpleFont::Print(x,y,(opacity << 24) | 0xFFFFFF,buffer,length);
	}

	//----------
	// draw cursor

	{
		RenderState_2D_PC state;

		int fw = SimpleFont::getWidth();
		int fh = SimpleFont::getHeight();

		int x1 = consoleX + fw * (g_input.GetCursor() + 1);
		int y1 = consoleY + fh * consoleHeight + fh/2;

		int x2 = x1 + fw;
		int y2 = y1 + fh/2;

		static int blink = 255;

		blink -= 16;

		if(blink < 0) blink = 255;

		int cursorOpacity = (blink * opacity) / 255;

		DrawRect(x1,y1,x2,y2, (cursorOpacity << 24) | 0xFFFFFF);
	}
}

//-----------------------------------------------------------------------------

}; // namespace MGSToolbox {

#endif // USE_MGSTOOLBOX