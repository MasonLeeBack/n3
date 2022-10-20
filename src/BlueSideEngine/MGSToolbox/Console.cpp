// Copyright 2005 Microsoft Corporation

#include "stdafx.h"
#include "Console.h"

#ifdef USE_MGSTOOLBOX

#include <xtl.h>
#include <stdio.h>

#include "SimpleFont.h"

namespace MGSToolbox {

//-----------------------------------------------------------------------------

Console::Console ( int posX, int posY, int width, int height, int opacity )
: m_positionX(posX),
  m_positionY(posY),
  m_width(width),
  m_height(height),
  m_opacity(opacity)
{
	m_buffer = new char[width*height];

	Clear();
}

Console::~Console ( void )
{
	delete m_buffer;
	m_buffer = NULL;
}

//-----------------------------------------------------------------------------

void Console::Scroll ( int lines )
{
	int count = m_width * m_height;

	for(int i = 0; i < count; i++)
	{
		int newChar = i + (lines * m_width);

		m_buffer[i] = (newChar < count) ? m_buffer[newChar] : 0;
	}
}

//----------
// Scroll the console up by one line and print the input text on the bottom line

void Console::PrintRaw ( char * text, int count )
{
	Scroll(1);

	count = min(m_width,count);

	char * outLine = &m_buffer[m_width * (m_height - 1)];  

	strncpy(outLine,text,count);
}

//----------

void Console::Print ( char * text )
{
	char * cursor = text;

	while(*cursor)
	{
		int lineLen = 0;
		
		char * cursor2 = cursor;

		while(*cursor2 && (*cursor2 != '\n')) { cursor2++; lineLen++; }

		if(lineLen > m_width)
		{
            PrintRaw(cursor,m_width);
			cursor += m_width;
			continue;
		}
		else
		{
			PrintRaw(cursor,lineLen);

			if(cursor[lineLen] == 0) return;

			cursor += lineLen + 1;

			continue;
		}
	}
}

//----------

void Console::Printf ( char * text, ... )
{
	char buffer[512];

	va_list arglist;
	
	va_start(arglist, text);
	_vsnprintf(buffer, 512, text, arglist);
	va_end(arglist);

	Print(buffer);
}

//-----------------------------------------------------------------------------

void Console::Clear ( void )
{
	memset(m_buffer,0,m_width * m_height);
}

//-----------------------------------------------------------------------------

void Console::Render ( void )
{
	SimpleFont::PrintBlock(m_positionX,m_positionY,m_width,m_height,(m_opacity << 24) | 0xFFFFFF,m_buffer);
}

//-----------------------------------------------------------------------------

void Console::Reset ( void )
{
	delete m_buffer;

	m_buffer = new char[m_width*m_height];

	Clear();
}

//-----------------------------------------------------------------------------

}; // namespace MGSToolbox {

#endif // USE_MGSTOOLBOX