// Copyright © 2005 Microsoft Corporation

// Simple text input manager - feed it keys from XInputGetKeystroke, get input
// strings back.

#pragma once

#include "MGSToolboxConfig.h"
#ifdef USE_MGSTOOLBOX

namespace MGSToolbox {

//-----------------------------------------------------------------------------

class TextInput
{
public:

	TextInput ( int bufferSize );
	virtual ~TextInput();

	void AddKey ( int vkey, int flags );

	char * GetContents ( void );
	void   SetContents ( char * buffer );

	void   Clear ( void );

	int    GetCursor ( void );

protected:

	char & current ( void ) { return m_buffer[m_cursor]; }
	char & prev ( void ) { return m_buffer[m_cursor-1]; }
	char & next ( void ) { return m_buffer[m_cursor+1]; }

	char * m_data;
	char * m_buffer;
	int    m_bufferSize;
	int    m_cursor;

	bool   m_bInsert;
};

//-----------------------------------------------------------------------------

}; // namespace MGSToolbox

#endif // USE_MGSTOOLBOX