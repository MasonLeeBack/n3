#include "Stdafx.h"
#include "skip.h"

CSkipper::CSkipper( int nFrame )
{
	SetFrame(nFrame);
	m_bStart=false;
	m_bSkip=false;
}

CSkipper::~CSkipper()
{
}

void CSkipper::SetFrame( int nFrame ) 
{ 
	m_nFrameRate = nFrame;
	if( m_nFrameRate > MAX_FRAME_RATE )
	{
		m_nFrameRate = MAX_FRAME_RATE;
	}
	m_fDeltaTimePerFrame = 1.f/float(m_nFrameRate);

	QueryPerformanceFrequency( &m_liFrequency );
	m_liFramePerFreq.QuadPart = m_liFrequency.QuadPart / m_nFrameRate;
}

void CSkipper::Start()
{
	QueryPerformanceFrequency( &m_liFrequency );
	QueryPerformanceCounter( &m_liStartTime );
	m_liFramePerFreq.QuadPart = m_liFrequency.QuadPart / m_nFrameRate;
	m_bStart = true;
}

void CSkipper::Stop()
{
	m_bStart=false;
}

bool CSkipper::IsWait()
{
	if( (m_bSkip) || ( !m_bStart ) )
	{
		return false;
	}

	LARGE_INTEGER liCurTime;

	QueryPerformanceCounter( &liCurTime );
	if( liCurTime.QuadPart < m_liStartTime.QuadPart )
	{
		return true;
	}

	return false;
}

void CSkipper::Wait()
{
	LARGE_INTEGER liCurTime;

	if( (m_bSkip) || ( !m_bStart ) )
	{
		return;
	}
	QueryPerformanceCounter( &liCurTime );
	while( liCurTime.QuadPart < m_liStartTime.QuadPart )
	{
		QueryPerformanceCounter( &liCurTime );
	}
}

bool CSkipper::IsSkip()
{
	LARGE_INTEGER liCurTime;

	if( !m_bStart )
	{
		return false;
	}

	QueryPerformanceCounter( &liCurTime );

//	if( ( liCurTime.QuadPart > m_liStartTime.QuadPart ) )
	if( ( liCurTime.QuadPart > ( m_liStartTime.QuadPart + m_liFramePerFreq.QuadPart ) ) )
	{
		m_bSkip=true;
	}
	else
	{
		m_bSkip=false;
	}
	m_liPrevTime.QuadPart = m_liStartTime.QuadPart;
	m_liStartTime.QuadPart = liCurTime.QuadPart + m_liFramePerFreq.QuadPart;

	return m_bSkip;
}
