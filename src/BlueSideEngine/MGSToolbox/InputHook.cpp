// Copyright 2005 Microsoft Corporation

#include "stdafx.h"
#include "InputHook.h"

#ifdef USE_MGSTOOLBOX

#include <xtl.h>

#include "Utils.h"
#include "DebugValue.h"

namespace MGSToolbox {

//-----------------------------------------------------------------------------

DebugValue g_inputHookCombo          ( "Input Hook", "Button combo", XINPUT_GAMEPAD_LEFT_SHOULDER | XINPUT_GAMEPAD_RIGHT_SHOULDER );
DebugValue g_inputHookController     ( "Input Hook", "Controller", 0, 0, 3, 1);
DebugValue g_inputHookRepeatDelay    ( "Input Hook", "Repeat Delay", 500, 0, 1000, 10 );
DebugValue g_inputHookRepeatInterval ( "Input Hook", "Repeat Interval", 100, 10, 1000, 10 );

bool g_bGameInputEnabled = true;
bool g_bToolboxInputEnabled = false;

//----------

struct InputState
{
	DWORD controllerStatus[4];
	XINPUT_STATE controllerState[4];
};

InputState savedGameInputState;
InputState savedToolboxInputState;

//----------

int g_gamepadClicks = 0;
int g_gamepadState = 0;

int g_keyClicks[32];
int g_keyClickCount = 0;

//-----------------------------------------------------------------------------

void UpdateHook ( DWORD buttons )
{
	enum InputHookState 
	{
		IHS_DISABLED_ACTIVE,   // hook disabled, controller active
		IHS_DISABLED_IDLE,     // hook disabled, controller idle
		IHS_DISABLING,
		IHS_ENABLING,
		IHS_ENABLED_IDLE,      // hook enabled, controller idle
		IHS_ENABLED_ACTIVE,    // hook enabled, controller active
	};

	static InputHookState state = IHS_DISABLED_IDLE;

	//----------

	switch(state)
	{
	case IHS_DISABLED_ACTIVE:

		if(buttons == 0)
		{
			state = IHS_DISABLED_IDLE;
		}

		break;


	case IHS_DISABLED_IDLE:

		if(buttons & ~g_inputHookCombo.asInt())
		{
			state = IHS_DISABLED_ACTIVE;
		}
		else if((int)buttons == g_inputHookCombo.asInt())
		{
			dprintf("MGSToolbox - Enabling input hook\n");

			state = IHS_ENABLING;

			for(int i = 0; i < 4; i++)
			{
				savedGameInputState.controllerStatus[i] = XInputGetState(i,&savedGameInputState.controllerState[i]);
			}

			savedGameInputState.controllerState[g_inputHookController.asInt()].Gamepad.wButtons &= ~g_inputHookCombo.asInt();
		}

		break;


	case IHS_DISABLING:

		if(buttons == 0)
		{
			state = IHS_DISABLED_IDLE;
		}

		break;

	
	case IHS_ENABLING:

		if(buttons == 0)
		{
			state = IHS_ENABLED_IDLE;
		}

		break;


	case IHS_ENABLED_IDLE:

		if(buttons & ~g_inputHookCombo.asInt())
		{
			state = IHS_ENABLED_ACTIVE;
		}
		else if((int)buttons == g_inputHookCombo.asInt())
		{
			dprintf("MGSToolbox - Disabling input hook\n");

			state = IHS_DISABLING;

			for(int i = 0; i < 4; i++)
			{
				savedToolboxInputState.controllerStatus[i] = XInputGetState(i,&savedToolboxInputState.controllerState[i]);
			}

			savedToolboxInputState.controllerState[g_inputHookController.asInt()].Gamepad.wButtons &= ~g_inputHookCombo.asInt();
		}

		break;


	case IHS_ENABLED_ACTIVE:

		if(buttons == 0)
		{
			state = IHS_ENABLED_IDLE;
		}

		break;


	default:
		break;
	}

	//----------

	g_bGameInputEnabled = (state == IHS_DISABLED_IDLE) || (state == IHS_DISABLED_ACTIVE);
	g_bToolboxInputEnabled = (state == IHS_ENABLED_IDLE) || (state == IHS_ENABLED_ACTIVE);
}

//-----------------------------------------------------------------------------

void UpdateToolboxInput ( void )
{
	static __int64 buttonPressFrameTick[16] = { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 };
	static int     buttonRepeatCount[16] = { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 };
	
	static XINPUT_STATE oldState = {0,{0,0,0,0,0,0,0}};
	static XINPUT_STATE newState = {0,{0,0,0,0,0,0,0}};

	__int64 frameTick;
	__int64 frequency;

	QueryPerformanceCounter( reinterpret_cast<LARGE_INTEGER*>(&frameTick) );
	QueryPerformanceFrequency( reinterpret_cast<LARGE_INTEGER*>(&frequency) );

	if(g_bToolboxInputEnabled)
	{
		XInputGetState(0,&newState);
	}
	else
	{
		newState = savedToolboxInputState.controllerState[0];
	}

	g_gamepadClicks = 0;

	for(int i = 0; i < 16; i++)
	{
		int button = (1 << i);

		if(newState.Gamepad.wButtons & button)
		{
			if(!(oldState.Gamepad.wButtons & button))
			{
				g_gamepadClicks |= button;

				buttonPressFrameTick[i] = frameTick;
				buttonRepeatCount[i] = 0;
			}
			else
			{
				__int64 frameDelta = frameTick - buttonPressFrameTick[i];
				__int64 frameTime = frameDelta * 1000 / frequency;

				if(frameTime > g_inputHookRepeatDelay.asInt())
				{
					__int64 remainingTime = frameTime - g_inputHookRepeatDelay.asInt();
					if(remainingTime / g_inputHookRepeatInterval.asInt() > buttonRepeatCount[i])
					{
						++buttonRepeatCount[i];
						g_gamepadClicks |= button;
					}
				}
			}
		}
		else
		{
			buttonPressFrameTick[i] = 0;
			buttonRepeatCount[i] = 0;
		}
	}

	oldState = newState;

	g_gamepadState = newState.Gamepad.wButtons;

	g_keyClickCount = 0;
}

//-----------------------------------------------------------------------------

void InputHook::Update ( void )
{
	XINPUT_STATE state;

	DWORD result = XInputGetState(g_inputHookController.asInt(),&state);

	if(result == ERROR_SUCCESS)
	{
		UpdateHook(state.Gamepad.wButtons);
	}

	UpdateToolboxInput();
}

//-----------------------------------------------------------------------------

bool InputHook::Enabled ( void )
{
	return g_bToolboxInputEnabled;
}

//-----------------------------------------------------------------------------

DWORD InputHook::GetState ( DWORD dwUserIndex, PXINPUT_STATE pState )
{
	if(g_bGameInputEnabled)
	{
		return XInputGetState(dwUserIndex,pState);
	}
	else
	{
		*pState = savedGameInputState.controllerState[dwUserIndex];
		return savedGameInputState.controllerStatus[dwUserIndex];
	}
}

//-----------------------------------------------------------------------------

void Keyboard::AddKey ( int vkey, int flags )
{
	if(flags & XINPUT_KEYSTROKE_KEYDOWN)
	{
		g_keyClicks[g_keyClickCount++] = vkey;
	}
}

//-----------------------------------------------------------------------------

bool Keyboard::GetClicked ( int vkey )
{
	for(int i = 0; i < g_keyClickCount; i++)
	{
		if(g_keyClicks[i] == vkey) return true;
	}

	return false;
}

//-----------------------------------------------------------------------------

}; // namespace MGSToolbox {

#endif