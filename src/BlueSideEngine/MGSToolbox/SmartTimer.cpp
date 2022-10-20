// Copyright 2005 Microsoft Corporation

#include "stdafx.h"
#include "SmartTimer.h"

#ifdef USE_MGSTOOLBOX

#include <xtl.h>

#include "Utils.h"

namespace MGSToolbox {

//-----------------------------------------------------------------------------

SmartTimerStats * SmartTimerStats::s_pHead = NULL;
SmartTimerStats * SmartTimerStats::s_pTail = NULL;

SmartTimerStats::SmartTimerStats( char const * const name, unsigned int color ) 
	: m_name(name), m_color(color), m_timeAccum(0), m_count(0)
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

SmartTimerStats::~SmartTimerStats ( void )
{
	if(m_pNext) m_pNext->m_pPrev = m_pPrev;
	if(m_pPrev) m_pPrev->m_pNext = m_pNext;

    if(s_pHead == this) s_pHead = m_pNext;
	if(s_pTail == this) s_pTail = m_pPrev;

	m_pNext = NULL;
	m_pPrev = NULL;
}

//----------

void SmartTimerStats::Clear ( void )
{
	m_timeAccum = 0;
	m_count = 0;
}

void SmartTimerStats::ClearAll ( void )
{
	SmartTimerStats * cursor = s_pHead;

	while(cursor)
	{
		cursor->Clear();

		cursor = cursor->m_pNext;
	}
}

void SmartTimerStats::DumpStats ( void )
{
	LARGE_INTEGER freq;

	QueryPerformanceFrequency(&freq);

	SmartTimerStats * cursor = s_pHead;

	while(cursor)
	{
		float time = (float)((double)cursor->m_timeAccum / (double)freq.QuadPart);

		time *= 1000.0f;

		dprintf("Timer %s, %d, %f\n",cursor->m_name,cursor->m_count,time);

		cursor = cursor->m_pNext;
	}
}

//-----------------------------------------------------------------------------

}; // namespace MGSToolbox {

#endif