// Copyright 2005 Microsoft Corporation

#include "stdafx.h"
#include "DebugCommand.h"

#ifdef USE_MGSTOOLBOX

#include <xtl.h>
#include <assert.h>
#include <stdio.h>

#include "TextMatch.h"

namespace MGSToolbox {

//-----------------------------------------------------------------------------

void _command_echo ( DebugCommand::Arg * args, char * outBuffer, unsigned int outBufferLength )
{
	sprintf_s(outBuffer,outBufferLength,"%s\n",args[0].c);
}

DebugCommand command_echo("echo","%s","Print the input string to the console",_command_echo);

//----------

void _command_help ( DebugCommand::Arg * args, char * outBuffer, unsigned int outBufferLength )
{
	DebugCommand const * cursor = DebugCommand::getHead();

	outBuffer += sprintf_s(outBuffer,outBufferLength,"Command List:\n");
	outBuffer += sprintf_s(outBuffer,outBufferLength,"-------------\n");

	while(cursor)
	{
		if(strlen(cursor->getFormat()))
		{
			outBuffer += sprintf_s(outBuffer,outBufferLength,"%s %s : %s\n",cursor->getName(),cursor->getFormat(),cursor->getHelp());
		}
		else
		{
			outBuffer += sprintf_s(outBuffer,outBufferLength,"%s : %s\n",cursor->getName(),cursor->getHelp());
		}

		cursor = cursor->getNext();
	}

	outBuffer += sprintf_s(outBuffer,outBufferLength,"-------------\n");
}

DebugCommand command_help("help","","Print the list of commands available",_command_help);

//----------

DebugCommand * DebugCommand::s_pHead = NULL;
DebugCommand * DebugCommand::s_pTail = NULL;

//-----------------------------------------------------------------------------

DebugCommand::DebugCommand ( char const * name, char const * argFormat, char const * helpText, command_fn pfCommand )
: m_pPrev(NULL),
  m_pNext(NULL),
  m_name(name),
  m_argFormat(argFormat),
  m_argCount(0),
  m_help(helpText),
  m_command(pfCommand)
{
	m_argCount = 0;

	for(char const * cursor = m_argFormat; *cursor; cursor++)
	{
		if(*cursor == '%') m_argCount++;
	}

	assert(m_argCount <= 8);

	Link();
}

//----------

DebugCommand::~DebugCommand ( void )
{
	Unlink();
}

//-----------------------------------------------------------------------------

void DebugCommand::Link ( void )
{
	if(s_pTail)
	{
		m_pNext = NULL;
		m_pPrev = s_pTail;

		s_pTail->m_pNext = this;
		s_pTail = this;
	}
	else
	{
		s_pHead = this;
		s_pTail = this;

		m_pPrev = NULL;
		m_pNext = NULL;
	}
}

//----------

void DebugCommand::Unlink ( void )
{
	if(m_pNext) m_pNext->m_pPrev = m_pPrev;
	if(m_pPrev) m_pPrev->m_pNext = m_pNext;

	if(s_pHead == this) s_pHead = m_pNext;
	if(s_pTail == this) s_pTail = m_pPrev;

	m_pNext = NULL;
	m_pPrev = NULL;
}

//-----------------------------------------------------------------------------

bool DebugCommand::ParseArgs ( char * argString, Arg * outArgs )
{
	if(argString == NULL)
	{
		return ((m_argFormat == NULL) || (strlen(m_argFormat) == 0));
	}

	char * argCursor = argString;
    Arg * outCursor = outArgs;

	for(char const * formatCursor = m_argFormat; *formatCursor; formatCursor++)
	{
		// ignore whitespace in format spec

		if(*formatCursor == ' ') continue;

		// ignore whitespace between args

		argCursor += matchSpace(argCursor);

		// match non-format characters exactly

		if(*formatCursor != '%')
		{
			if(!matchOne(argCursor,*formatCursor)) return false;

			argCursor++;
			continue;
		}

		formatCursor++;

		switch(*formatCursor)
		{
		case 'd':
			{
				int a = matchInt(argCursor);

				if(a == 0) return false;

				outCursor->i = atoi(argCursor);
				argCursor += a;
				outCursor++;
				continue;
			}

		case 'f':
			{
				int a = matchFloat(argCursor);

				if(a == 0) return false;

				outCursor->f = (float)atof(argCursor);
				argCursor += a;
				outCursor++;
				continue;
			}

		case 's':
			{
				int a = matchString(argCursor);

				if(a != 0)
				{
					outCursor->c = new char[a];
					strncpy(outCursor->c,argCursor+1,a-2);
					outCursor->c[a-2] = 0;
				}
				else
				{
					a = matchNot(argCursor,formatCursor[1]);

					if(a == 0) return false;

					outCursor->c = new char[a+1];
					strncpy(outCursor->c,argCursor,a);
					outCursor->c[a] = 0;
				}

				argCursor += a;
				outCursor++;
				continue;
			}

		default:
			return false;
		}
	}

	argCursor += matchSpace(argCursor);

	return *argCursor == 0;
}

//-----------------------------------------------------------------------------

DebugCommand * DebugCommand::Find ( char * name )
{
	for(DebugCommand * cursor = s_pHead; cursor; cursor = cursor->m_pNext)
	{
		if(stricmp(name,cursor->m_name)) continue;

		return cursor;
	}

	return NULL;
}

//-----------------------------------------------------------------------------

void DebugCommand::Run ( Arg * args, char * outBuffer, unsigned int outBufferLength)
{
	if(m_command)
	{
		m_command(args,outBuffer,outBufferLength);
	}
	else
	{
		sprintf_s(outBuffer,outBufferLength,"Error - Debug command is missing its callback");
	}
}

//-----------------------------------------------------------------------------

void DebugCommand::Run ( char * name, char * args, char * outBuffer, unsigned int outBufferLength )
{
	Arg argblock[8];

	DebugCommand * command = Find(name);

	if(command)
	{
		if(command->ParseArgs(args,argblock))
		{
			command->Run(argblock,outBuffer,outBufferLength);
		}
		else
		{
			sprintf_s(outBuffer,outBufferLength,"Error - command arguments aren't compatible with command \'%s %s\'",command->m_name, command->m_argFormat);
		}
	}
	else
	{
		sprintf_s(outBuffer,outBufferLength,"Error - command %s not found",name);
	}
}

//-----------------------------------------------------------------------------

void DebugCommand::Run ( char * text, char * outBuffer, unsigned int outBufferLength )
{
	if(text == NULL) return;
	if(strlen(text) == 0) return;

	char * cursor = text;

	cursor += matchSpace(cursor);

	char * command = cursor;

	cursor += matchAlnum(cursor);

	if(*cursor == 0)
	{
		Run(command,NULL,outBuffer,outBufferLength);
	}
	else
	{
		*cursor++ = 0;

		Run(command,cursor,outBuffer,outBufferLength);
	}

	return;
}

//-----------------------------------------------------------------------------

}; // namespace MGSToolbox

#endif