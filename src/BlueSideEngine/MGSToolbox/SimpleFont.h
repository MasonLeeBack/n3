// Copyright © 2005 Microsoft Corporation

#pragma once

#include "MGSToolboxConfig.h"
#ifdef USE_MGSTOOLBOX

namespace MGSToolbox
{
	namespace SimpleFont
	{

		enum RenderFlags {
			eHandleTab     = 0x01,
			eHandleNewline = 0x02,
			eRenderSolid   = 0x04,
		};

		// Printf-style formatting

		void Printf(int x, int y, const char * c, ...);
		void Printf(int x, int y, DWORD color, const char * c, ...);

		// No formatting, draws unprintable characters as blanks. Used for rendering the console.

		void Print(int x, int y, const char * c, int count);
		void Print(int x, int y, DWORD color, const char * c, int count);

		void PrintBlock(int x, int y, int width, int height, DWORD color, const char * c );

		// Width / height of each glyph, in pixels

		int getWidth  ( void );
		int getHeight ( void );
	};
};

#endif