// Copyright 2005 Microsoft Corporation

#include "stdafx.h"
#include "TextInput.h"

#ifdef USE_MGSTOOLBOX

#include <xtl.h>

#include "TextMatch.h"
#include "Utils.h"

namespace MGSToolbox {

//-----------------------------------------------------------------------------

TextInput::TextInput ( int bufferSize )
: m_data(new char[bufferSize + 2]),
  m_buffer(&m_data[1]),
  m_bufferSize(bufferSize),
  m_cursor(0),
  m_bInsert(true)
{
	memset(m_data,0,bufferSize+2);
}

TextInput::~TextInput ( void )
{
	delete [] m_data;
	m_data = NULL;
	m_buffer = NULL;
	m_bufferSize = 0;

	m_cursor = -1;
}

//-----------------------------------------------------------------------------

void TextInput::AddKey ( int vkey, int flags )
{
	int keydown = flags & XINPUT_KEYSTROKE_KEYDOWN;
	int keyrepeat = flags & XINPUT_KEYSTROKE_REPEAT;

	if(!(keydown || keyrepeat)) return;

	//----------

	switch(vkey)
	{
	case VK_BACK:
		if(m_cursor > 0)
		{
			memmove(&prev(),&current(),m_bufferSize - m_cursor);
			m_cursor--;
		}
		break;

	case VK_LEFT:
		if(flags & XINPUT_KEYSTROKE_CTRL)
		{
			if(prev() == ' ')
			{
				m_cursor -= rmatchAll(&prev(),' ');
			}

			m_cursor -= rmatchNot(&prev(),' ');
		}
		else
		{
			if(m_cursor > 0) m_cursor--;
		}
		break;

	case VK_RIGHT:
		if(flags & XINPUT_KEYSTROKE_CTRL)
		{
			if(current() != ' ')
			{
				m_cursor += matchNot(&current(),' ');
			}

			m_cursor += matchAll(&current(),' ');
		}
		else
		{
			if((m_cursor < m_bufferSize) && current()) m_cursor++;
		}
		break;

	case VK_HOME:
		m_cursor = 0;
		break;

	case VK_END:
		m_cursor = strlen(m_buffer);
		break;

	case VK_INSERT:
		m_bInsert = !m_bInsert;
	
	default:
		{
			char c;

			if(translateKeystroke(vkey,flags,c) && (m_cursor < m_bufferSize))
			{
				if(m_bInsert)
				{
					memmove(&next(),&current(),m_bufferSize - m_cursor - 1);
				}

				m_buffer[m_cursor++] = c;
			}
		}
		break;
	}
}

//-----------------------------------------------------------------------------

char * TextInput::GetContents ( void )
{
	return m_buffer;
}

void TextInput::SetContents ( char * buffer )
{
	strncpy(m_buffer,buffer,m_bufferSize);

	m_cursor = 0;
}

int TextInput::GetCursor ( void )
{
	return m_cursor;
}

void TextInput::Clear ( void )
{
	memset(m_buffer,0,m_bufferSize);

	m_buffer[0] = 0;
	m_cursor = 0;
}

//-----------------------------------------------------------------------------

}; // namespace MGSToolbox {

#endif