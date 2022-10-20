// Copyright 2005 Microsoft Corporation

#include "stdafx.h"
#include "DebugMenu.h"

#ifdef USE_MGSTOOLBOX

#include <xtl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "SimpleFont.h"
#include "InputHook.h"
#include "DebugValue.h"
#include "MGSToolBox.h"

namespace MGSToolbox {

//-----------------------------------------------------------------------------

DebugValue g_debugMenuX ( "Debug Menu", "Menu X", 400, 0, 1280, 1 );
DebugValue g_debugMenuY ( "Debug Menu", "Menu Y", 400, 0, 720, 1 );

DebugValue g_debugMenuSave ( "Debug Menu", "Save Settings", DebugMenu::SaveINI );
DebugValue g_debugMenuLoad ( "Debug Menu", "Load Settings", DebugMenu::LoadINI );

DebugValue g_debugMenuAutoSave ( "Debug Menu", "Autosave Settings", false );
DebugValue g_debugMenuAutoLoad ( "Debug Menu", "Autoload Settings", false );

DebugValue g_debugMenuINI ( "Debug Menu", "INI file", "d:\\toolbox.ini" );

DebugValue * g_pCurrentDebugValue = NULL;

bool g_bDebugValuesDirty = false;

time_t g_debugINIFileTime = 0;

bool DebugMenu::s_bInstalled = false;

//-----------------------------------------------------------------------------

void stripWhitespace ( char * buffer )
{
	char * end = buffer + strlen(buffer);

	char * cursor1 = buffer;
	while(*cursor1 && isspace(*cursor1)) cursor1++;

	if(cursor1 == end)
	{
		buffer[0] = 0;
		return;
	}

	char * cursor2 = end - 1;
	while(isspace(*cursor2) && (cursor2 != cursor1)) cursor2--;

	memmove(buffer,cursor1,cursor2-cursor1+1);
	buffer[cursor2-cursor1+1] = 0;
}

//----------

bool getNameValue ( char * buffer, char * name, char * value )
{
	if(strlen(buffer) < 3) return false;

	char * cursor = buffer;
	char * end = buffer + strlen(buffer);

	while(*cursor && (*cursor != '=')) cursor++;

	if(cursor == buffer) return false;
	if(cursor > end-2) return false;

	memcpy(name,buffer,cursor-buffer);
	name[cursor-buffer] = 0;

	memcpy(value,cursor+1,end-cursor-1);
	value[end-cursor-1] = 0;

	stripWhitespace(name);
	stripWhitespace(value);

	return true;
}

//-----------------------------------------------------------------------------

void DebugMenu::LoadINI ( char const * fileName )
{
	FILE * file = fopen(fileName,"r");

	if(file == NULL) return;

	//----------

	struct _stat stat;

	if(!_fstat(_fileno(file),&stat))
	{
		g_debugINIFileTime = stat.st_mtime;
	}

	//----------

	char buffer[256];
	char group[256] = "";

	while(fgets(buffer,256,file))
	{
		stripWhitespace(buffer);

		if(buffer[0] == '[')
		{
			memcpy(group,buffer+1,strlen(buffer) - 2);
			group[strlen(buffer) - 2] = 0;
		}
		else
		{
			char name[256] = "";
			char value[256] = "";

			if(getNameValue(buffer,name,value))
			{
				DebugValue * debugValue = DebugValue::Find(group,name);

				if(debugValue)
				{
					debugValue->fromString(value);
				}
			}
		}
	}

	fclose(file);
}

//----------

void DebugMenu::SaveINI ( char const * fileName )
{
	FILE * file = fopen(fileName,"wb");

	if(file == NULL) return;

	bool firstGroup = true;

	for(DebugValue * cursor = DebugValue::getHead(); cursor; cursor = cursor->getNext())
	{
		if((cursor->getPrev() == NULL) || (stricmp(cursor->getGroup(),cursor->getPrev()->getGroup())))
		{
			if(!firstGroup) fprintf(file,"\r\n");

			fprintf(file,"[%s]\r\n",cursor->getGroup());

			firstGroup = false;
		}

		char buffer[256];

		cursor->toString(buffer,256);

		fprintf(file,"%s = %s\r\n",cursor->getName(),buffer);
	}

	fclose(file);

	//----------

	{
		struct _stat stat;

		if(!_stat(fileName,&stat))
		{
			g_debugINIFileTime = stat.st_mtime;
		}
	}
}

//----------

void DebugMenu::LoadINI ( void )
{
	LoadINI ( g_debugMenuINI.asString() );
}

void DebugMenu::SaveINI ( void )
{
	SaveINI ( g_debugMenuINI.asString() );
}

//-----------------------------------------------------------------------------

void DebugMenu::Install ( void )
{
	DebugValue::Sort();

	// On first startup we load the INI file twice, so that if the first INI file
	// changes g_debugMenuINI we can load the updated one as well.

	DebugMenu::LoadINI();
	DebugMenu::LoadINI();

	s_bInstalled = true;
}

void DebugMenu::Remove ( void )
{
	s_bInstalled = false;
}

//-----------------------------------------------------------------------------

void DebugMenu::Update ( void )
{
	if(!s_bInstalled) return;

	if(g_debugMenuAutoLoad.asBool() && (GetFrameCount() % 10 == 0))
	{
		char const * ini = g_debugMenuINI.asString();

		struct _stat stat;

		if(!_stat(ini,&stat))
		{
			if(stat.st_mtime > g_debugINIFileTime)
			{
				LoadINI(ini);
			}
		}
	}

	//----------

	if(g_pCurrentDebugValue == NULL)
	{
		g_pCurrentDebugValue = DebugValue::getHead();
	}

	if(g_pCurrentDebugValue == NULL) return;

	DebugValue * & current = g_pCurrentDebugValue;


	//----------
	// Up and down on the dpad changes the selected option in the current group

	if(g_gamepadClicks & XINPUT_GAMEPAD_DPAD_UP)
	{
		if(current->getPrev() && !stricmp(current->getGroup(),current->getPrev()->getGroup()))
		{
			current = current->getPrev();
		}
		else
		{
			// User pressed up while at the top of a group, wrap around by going to the end of the current group

			while(current->getNext() && !stricmp(current->getGroup(),current->getNext()->getGroup()))
				current = current->getNext();
		}
	}

	if(g_gamepadClicks & XINPUT_GAMEPAD_DPAD_DOWN)
	{
		if(current->getNext() && !stricmp(current->getGroup(),current->getNext()->getGroup()))
		{
			current = current->getNext();
		}
		else
		{
			// User pressed down while at the bottom of a group, wrap around by going to the top of the current group

			while(current->getPrev() && !stricmp(current->getGroup(),current->getPrev()->getGroup()))
				current = current->getPrev();
		}
	}

	//----------
	// Right and left on the dpad increments/decrements the current option ( x10 if the A button is held down )

	if(g_gamepadClicks & XINPUT_GAMEPAD_DPAD_RIGHT)
	{
		int reps = (g_gamepadState & XINPUT_GAMEPAD_A) ? 10 : 1;

		for(int i = 0; i < reps; i++) current->Increment();

		g_bDebugValuesDirty = true;
	}

	if(g_gamepadClicks & XINPUT_GAMEPAD_DPAD_LEFT)
	{
		int reps = (g_gamepadState & XINPUT_GAMEPAD_A) ? 10 : 1;

		for(int i = 0; i < reps; i++) current->Decrement();

		g_bDebugValuesDirty = true;
	}

	//----------
	// Shoulder buttons flip through pages of debug options

	if(g_gamepadClicks & XINPUT_GAMEPAD_LEFT_SHOULDER)
	{
		// step up to the top of the current group

		while(current->getPrev() && !stricmp(current->getGroup(),current->getPrev()->getGroup()))
			current = current->getPrev();

		// step up to the previous group

		if(current->getPrev()) current = current->getPrev();

		// and then step to the top of that group

		while(current->getPrev() && !stricmp(current->getGroup(),current->getPrev()->getGroup()))
			current = current->getPrev();
	}

	if(g_gamepadClicks & XINPUT_GAMEPAD_RIGHT_SHOULDER)
	{
		// step to the end of the current group

		while(current->getNext() && !stricmp(current->getGroup(),current->getNext()->getGroup()))
			current = current->getNext();

		// and then step down one

		if(current->getNext()) current = current->getNext();
	}

	//----------

	else
	{
		if(g_bDebugValuesDirty && g_debugMenuAutoSave.asBool())
		{
			SaveINI();
			g_bDebugValuesDirty = false;
		}
	}
}

//-----------------------------------------------------------------------------

void DebugMenu::Render ( void )
{
	if(!s_bInstalled) return;

	// Render the group names 

	{
		int lineX = g_debugMenuX.asInt();
		int lineY = g_debugMenuY.asInt();

		DebugValue * cursor = DebugValue::getHead();

		while(cursor)
		{
			// skip all entries that don't start a group

			if(cursor->getPrev() && (stricmp(cursor->getGroup(),cursor->getPrev()->getGroup()) == 0)) 
			{
				cursor = cursor->getNext();
				continue;
			}

			// make the group of the selected item yellow

			DWORD color = (stricmp(cursor->getGroup(),g_pCurrentDebugValue->getGroup()) == 0) ? 0xFFFFFF00 : 0xFFA0A0A0;

			// right-justify the group name

			int groupLen = strlen(cursor->getGroup());

			int newLineX = g_debugMenuX.asInt() - strlen(cursor->getGroup()) * SimpleFont::getWidth();

			// print the group name

			SimpleFont::Printf(newLineX,lineY,color,"%s",cursor->getGroup());

			// go to the next line

			lineY += SimpleFont::getHeight();

			cursor = cursor->getNext();
		}

		DebugValue * groupCursor = DebugValue::getHead();
	}

	// Render the current group

	{
		int lineX = g_debugMenuX.asInt();
		int lineY = g_debugMenuY.asInt();

		DebugValue * cursor = g_pCurrentDebugValue;

		// Run the cursor back to the beginning of the group

		while(cursor->getPrev() && (stricmp(cursor->getGroup(),cursor->getPrev()->getGroup()) == 0)) 
		{
			cursor = cursor->getPrev();
		}

		// Render all items in the group

		while(cursor)
		{
			// get the value string

			char lineValue[256];

			cursor->toString(lineValue,256);

			// make the current debug value yellow

			DWORD color = (cursor == g_pCurrentDebugValue) ? 0xFFFFFF00 : 0xFFA0A0A0;

			// draw the value string (the '|' provides a visual divider between the group names and the values)

			SimpleFont::Printf(lineX,lineY,color," | %s : %s",cursor->getName(),lineValue);

			// Stop when we hit the end of the group

			if(cursor->getNext() && (stricmp(cursor->getGroup(),cursor->getNext()->getGroup()) != 0)) break;

			lineY += SimpleFont::getHeight();

			cursor = cursor->getNext();
		}
	}
}

//-----------------------------------------------------------------------------

}; // namespace MGSToobox {

#endif // USE_MGSTOOLBOX