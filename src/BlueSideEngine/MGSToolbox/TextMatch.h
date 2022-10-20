// Copyright © 2005 Microsoft Corporation

#pragma once

#include "MGSToolboxConfig.h"
#ifdef USE_MGSTOOLBOX

// A poor substitute for regexes.

namespace MGSToolbox
{
	int matchOne    ( char * cursor, char c );
	int matchAll    ( char * cursor, char c );
	int matchNot    ( char * cursor, char c );

	int matchSpace  ( char * cursor );
	int matchAlnum  ( char * cursor );

	int matchAny    ( char * cursor, char * string );
	int matchExact  ( char * cursor, char * text );
	
	int matchDelim  ( char * cursor, char start, char end );

	int matchInt    ( char * cursor );
	int matchFloat  ( char * cursor );
	int matchString ( char * cursor );

	//----------
	// reverse matches require a string with a null terminator at both ends in order to be safe

	int rmatchOne   ( char * cursor, char c );
	int rmatchAll   ( char * cursor, char c );
	int rmatchNot   ( char * cursor, char c );

	int rmatchSpace ( char * cursor );
	int rmatchAlnum ( char * cursor );
};

#endif