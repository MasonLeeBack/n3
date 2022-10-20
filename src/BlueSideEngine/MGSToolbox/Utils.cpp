// Copyright 2005 Microsoft Corporation

#include "stdafx.h"
#include "Utils.h"

#ifdef USE_MGSTOOLBOX

#include <xtl.h>
#include <stdio.h>

namespace MGSToolbox {

//-----------------------------------------------------------------------------

IDirect3DDevice9 * g_pDevice = NULL;

//-----------------------------------------------------------------------------

void dprintf ( const char * c, ... )
{
	char buffer[512];

	va_list arglist;
	
	va_start(arglist, c);
	_vsnprintf_s(buffer, _countof(buffer), _TRUNCATE, c, arglist);
	va_end(arglist);

	OutputDebugString(buffer);
}

//-----------------------------------------------------------------------------

bool translateKeystroke ( int vkey, int flags, char & outChar )
{
	int shift = (flags & XINPUT_KEYSTROKE_SHIFT);

	const char numbers[10] = {'0','1','2','3','4','5','6','7','8','9'};
	const char symbols[10] = {')','!','@','#','$','%','^','&','*','('};

	if((vkey >= VK_0) && (vkey <= VK_9))
	{
		outChar = shift ? symbols[vkey-VK_0] : numbers[vkey-VK_0];
		return true;
	}

	if((vkey >= VK_NUMPAD0) && (vkey <= VK_NUMPAD9))
	{
		outChar = numbers[vkey-VK_NUMPAD0];
		return true;
	}

	if((vkey >= 'A') && (vkey <= 'Z'))
	{
		outChar = shift ? (char)vkey : (char)tolower(vkey);
		return true;
	}

	switch(vkey)
	{
	case VK_SPACE:      outChar = (char)vkey; break;

	case VK_OEM_1:      outChar = shift ? ':' : ';'; break;
	case VK_OEM_PLUS:   outChar = shift ? '+' : '='; break;
	case VK_OEM_COMMA:  outChar = shift ? '<' : ','; break;
	case VK_OEM_MINUS:  outChar = shift ? '_' : '-'; break;
	case VK_OEM_PERIOD: outChar = shift ? '>' : '.'; break;
	case VK_OEM_2:      outChar = shift ? '?' : '/'; break;
	case VK_OEM_3:      outChar = shift ? '~' : '`'; break;
	case VK_OEM_4:      outChar = shift ? '{' : '['; break;
	case VK_OEM_5:      outChar = shift ? '|' : '\\'; break;
	case VK_OEM_6:      outChar = shift ? '}' : ']'; break;
	case VK_OEM_7:      outChar = shift ? '\"' : '\''; break;
	case VK_MULTIPLY:   outChar = '*'; break;
	case VK_ADD:        outChar = '+'; break;
	case VK_SUBTRACT:   outChar = '-'; break;
	case VK_DECIMAL:    outChar = '.'; break;
	case VK_DIVIDE:     outChar = '/'; break;
	default:            outChar = 0; break;
	}

	return (outChar != 0);
}

//-----------------------------------------------------------------------------

void DrawRect ( SimpleRect *& cursor, int x1, int y1, int x2, int y2, DWORD color )
{
	cursor->x1 = float(x1) - 0.5f;
	cursor->y1 = float(y1) - 0.5f;  
	cursor->z1 = 1.0f;  
	cursor->color1 = color;

	cursor->x2 = float(x2) - 0.5f;
	cursor->y2 = float(y1) - 0.5f;
	cursor->z2 = 1.0f;  
	cursor->color2 = color;

	cursor->x3 = float(x2) - 0.5f;  
	cursor->y3 = float(y2) - 0.5f;
	cursor->z3 = 1.0f;  
	cursor->color3 = color;

	cursor++;
}

SimpleRect * BeginRects ( int rectCount )
{
	if(rectCount < 1) return NULL;

	SimpleRect * cursor = NULL;

	g_pDevice->BeginVertices( D3DPT_RECTLIST, rectCount * 3, sizeof(SimpleRect) / 3, (void**)&cursor);

	return cursor;
}

void EndRects ( SimpleRect * cursor )
{
	if(cursor)
	{
		g_pDevice->EndVertices();
	}
}

void DrawRect ( int x1, int y1, int x2, int y2, DWORD color )
{
	SimpleRect * cursor = BeginRects(1);

	DrawRect(cursor,x1,y1,x2,y2,color);

	EndRects(cursor);
}

void DrawRect2 ( int x, int y, int w, int h, DWORD color )
{
	SimpleRect * cursor = BeginRects(1);

	DrawRect(cursor,x,y,x+w,y+h,color);

	EndRects(cursor);
}

//-----------------------------------------------------------------------------

};

#endif // USE_MGSTOOLBOX