// Copyright 2005 Microsoft Corporation

#include "stdafx.h"
#include "TextMatch.h"

#ifdef USE_MGSTOOLBOX

#include <xtl.h>

namespace MGSToolbox {

//-----------------------------------------------------------------------------

int matchOne ( char * cursor, char c )
{
	return (*cursor == c) ? 1 : 0;
}

int matchAll ( char * cursor, char c )
{
	int count = 0;

	while(*cursor && (*cursor++ == c)) count++;

	return count;
}

int matchNot ( char * cursor, char c )
{
	int count = 0;

	while(*cursor && (*cursor++ != c)) count++;

	return count;
}

//-----------------------------------------------------------------------------

int matchSpace ( char * cursor ) 
{
	int count = 0;

	while(*cursor && isspace(*cursor++)) count++;

	return count;
}

int matchAlnum ( char * cursor )
{
	int count = 0;

	while(*cursor && isalnum(*cursor++)) count++;
	
	return count;
}

//-----------------------------------------------------------------------------

int matchAny ( char * cursor, char * string )
{
	int count = 0;

	while(*cursor)
	{
		bool found = false;

		for(char * s = string; *s; s++)
		{
			if(*cursor == *s)
			{
				found = true;
				break;
			}
		}

		if(!found) break;

		cursor++;
	}

	return count;
}

int matchExact ( char * cursor, char * string )
{
	int count = 0;

	while(*string)
	{
		if(*cursor != *string) return 0;

		count++;
		cursor++;
		string++;
	}

	return count;
}

int matchDelimited ( char * cursor, char start, char end )
{
	int a = matchOne(cursor,start); cursor += a;
    int b = matchNot(cursor,end);   cursor += b;
	int c = matchOne(cursor,end);   cursor += c;

	if(a && c)
	{
		return a + b + c;
	}
	else
	{
		return 0;
	}
}

//-----------------------------------------------------------------------------

int matchInt ( char * cursor )
{
	int count = 0;

	while(*cursor && isdigit(*cursor++)) count++;

	return count;
}

//----------

int matchFloat ( char * cursor )
{
	int a = matchInt(cursor);     cursor += a;
	int b = matchOne(cursor,'.'); cursor += b;
	int c = matchInt(cursor);     cursor += c;

	if(a && b && c) return a+b+c;

	return a;
}

//----------
// match a single- or double-quoted string

int matchString ( char * cursor )
{
	if(*cursor == '\"')
	{
		return matchDelimited(cursor,'\"','\"');
	}
	else if(*cursor == '\'')
	{
		return matchDelimited(cursor,'\'','\'');
	}
	else
	{
		return 0;
	}
}

//-----------------------------------------------------------------------------

int rmatchOne ( char * cursor, char c )
{
	return (*cursor == c) ? 1 : 0;
}

int rmatchAll ( char * cursor, char c )
{
	int count = 0;

	while(*cursor && (*cursor-- == c)) count++;

	return count;
}

int rmatchNot ( char * cursor, char c )
{
	int count = 0;

	while(*cursor && (*cursor-- != c)) count++;

	return count;
}

//----------

int rmatchSpace ( char * cursor )
{
	int count = 0;

	while(*cursor && isspace(*cursor--)) count++;

	return count;
}

int rmatchAlnum ( char * cursor )
{
	int count = 0;

	while(*cursor && isalnum(*cursor--)) count++;
	
	return count;
}

//-----------------------------------------------------------------------------

}; // namespace MGSToolbox {

#endif