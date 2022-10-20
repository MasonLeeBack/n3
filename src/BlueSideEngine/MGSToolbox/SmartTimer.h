// Copyright © 2005 Microsoft Corporation

#pragma once

#include "MGSToolboxConfig.h"
#ifdef USE_MGSTOOLBOX

#include <xtl.h>

#define SMART_TIMER(A,B) static MGSToolbox::SmartTimerStats s_timer##A(#A,B); MGSToolbox::SmartTimer timer##A(s_timer##A);

namespace MGSToolbox {

//-----------------------------------------------------------------------------

class SmartTimerStats
{
public:

	SmartTimerStats( char const * const name, unsigned int color );
	~SmartTimerStats ( void );

	void Add ( __int64 time )
	{
		m_count++;
		m_timeAccum += time;
	}

	//----------

	static void ClearAll ( void );
	static void DumpStats ( void );

	//----------

protected:

	void Clear ( void );

	char const * const m_name;
	unsigned int m_color;

	int m_count;
	__int64 m_timeAccum;

	SmartTimerStats * m_pPrev;
	SmartTimerStats * m_pNext;

	//----------

	static SmartTimerStats * s_pHead;
	static SmartTimerStats * s_pTail;
};

//-----------------------------------------------------------------------------

class SmartTimer
{
public:

	SmartTimer ( SmartTimerStats & stats ) 
	: m_stats(stats)
	{
		QueryPerformanceCounter(&m_timeBegin);
	}

	~SmartTimer ( void )
	{
		LARGE_INTEGER timeEnd;

		QueryPerformanceCounter(&timeEnd);

		m_stats.Add(timeEnd.QuadPart - m_timeBegin.QuadPart);
	}

protected:

	LARGE_INTEGER m_timeBegin;

	SmartTimerStats & m_stats;
};

//-----------------------------------------------------------------------------

}; // namespace MGSToolbox {

#endif // USE_MGSTOOLBOX