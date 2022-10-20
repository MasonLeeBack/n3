// Copyright © 2005 Microsoft Corporation

#pragma once

#include "MGSToolboxConfig.h"
#ifdef USE_MGSTOOLBOX

namespace MGSToolbox {

//-----------------------------------------------------------------------------

class Console
{
public:

	Console ( int positionX, int positionY, int width, int height, int opacity = 160 );
	virtual ~Console ( void );

	void Render ( void );

	void Clear  ( void );

	void Print  ( char * text );
	void Printf ( char * text, ... );

	int  GetOpacity ( void )  { return m_opacity; }
	void SetOpacity ( int o ) { m_opacity = o; }

	void SetWidth ( int width ) { m_width = width; Reset(); }
	void SetHeight ( int height ) { m_height = height; Reset(); }

protected:

	void Reset ( void );

	void Scroll ( int lines );
	void PrintRaw ( char * text, int count );

	int m_positionX;
	int m_positionY;

	int m_width;
	int m_height;

	int m_opacity;

	char * m_buffer;
};

//-----------------------------------------------------------------------------

}; // namespace MGSToolbox {

#endif