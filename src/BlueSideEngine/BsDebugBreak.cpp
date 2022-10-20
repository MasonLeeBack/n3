#include <stdafx.h>

#include "BsDebugBreak.h"

namespace Debug {

	enum {
		eMaxCallstackSize = 32
	};

#if defined(_XBOX) && !defined(_LTCG)

#include <xbdm.h>

void DumpCallstack()
{
	void * callstack[ eMaxCallstackSize ];

	HRESULT hr = DmCaptureStackBackTrace( eMaxCallstackSize, reinterpret_cast<void**>(&callstack) );
	if ( FAILED(hr) )
		return;

	int numCalls = 0;
	while ( numCalls < eMaxCallstackSize )
	{
		if ( callstack[numCalls] == 0 )
			break;

		DebugString( "\t%d: 0x%08x\n", numCalls, reinterpret_cast<unsigned int>( callstack[numCalls] ) );

		++numCalls;
	}
}

#else

void DumpCallstack()
{
}

#endif

char const * EventToString( DebugEvent event )
{
	switch( event )
	{
	case OnNothing:
		return "no action";
	case OnLoad:
		return "load";
	case OnAddRef:
		return "add ref";
	case OnRelease:
		return "release";
	case OnDelete:
		return "delete";
	case OnMemory:
		return "memory change action";
	case OnAll:
		return "all actions";
	default:
		return "<unknown action>";
	}
}

DebugInfo const * LookupDebugInfo( DebugInfo const * infoList, char const * name )
{
	if ( name == NULL )
		return NULL;

	int infoIndex = 0;

	while ( infoList[infoIndex].name[0] )
	{
		if ( _stricmp( infoList[infoIndex].name, name ) == 0 )
			return &infoList[infoIndex];

		++infoIndex;
	}

	return NULL;
}

void TriggerEvent( DebugInfo const * infoList, char const * name, int refCount, DebugEvent event )
{
	DebugInfo const * debugInfo = LookupDebugInfo( infoList, name );
	if ( debugInfo == NULL )
		return;

	if ( debugInfo->showEvent & event )
	{
		char const * actionString = (debugInfo->breakEvent & event)  ? "Break On" : "Info";
		char const * eventString  = EventToString( event );

		DebugString( "%s: %s - %s (%d)\n", actionString, name, eventString, refCount );
		DumpCallstack();
	}

	if ( debugInfo->breakEvent & event )
	{
		_DEBUGBREAK;
//		DebugBreak();
	}
}

};
