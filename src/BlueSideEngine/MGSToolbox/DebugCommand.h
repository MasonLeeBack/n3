// Copyright © 2005 Microsoft Corporation

#pragma once

#include "MGSToolboxConfig.h"
#ifdef USE_MGSTOOLBOX

namespace MGSToolbox {

//-----------------------------------------------------------------------------

class DebugCommand
{
public:

	struct Arg
	{
		Arg() : i(0), f(0), c(0) {}
		~Arg() { delete [] c; c = NULL; }

		int i;
		float f;
		char * c;
	};

	typedef void (*command_fn)( Arg * args, char * outBuffer, unsigned int outBufferLength );

	DebugCommand ( char const * name, char const * argFormat, char const * helpText, command_fn pfCommand );
	~DebugCommand ( void );

	char const * getName   ( void ) const { return m_name; }
	char const * getFormat ( void ) const { return m_argFormat; }
	char const * getHelp   ( void ) const { return m_help; }

	//----------

	static DebugCommand const * getHead ( void ) { return s_pHead; }
	static DebugCommand const * getTail ( void ) { return s_pTail; }

	DebugCommand const * getNext ( void ) const { return m_pNext; }
	DebugCommand const * getPrev ( void ) const { return m_pPrev; }

	//----------

	static void Run ( char * name, char * args, char * outBuffer, unsigned int outBufferLength );
	static void Run ( char * command, char * outBuffer, unsigned int outBufferLength );

	//----------
    
protected:

	void Link ( void );
	void Unlink ( void );

	//----------

	static DebugCommand * s_pHead;
	static DebugCommand * s_pTail;

	static DebugCommand * Find ( char * name );

	bool ParseArgs ( char * args, Arg * outArgs );
	void Run ( Arg * args, char * outBuffer, unsigned int outBufferLength );

	//----------

	DebugCommand * m_pPrev;
	DebugCommand * m_pNext;

	char const * m_name;
	char const * m_argFormat;
	int          m_argCount;
	char const * m_help;
	command_fn   m_command;
};

//-----------------------------------------------------------------------------

}; // namespace MGSToolbox {

#endif // USE_MGSTOOLBOX