#include "StdAfx.h"
#include "BsKernel.h"
#include "CrossVector.h"

#include "FcHitMarkMgr.h"

CFcHitMarkMgr::CFcHitMarkMgr(void)
{
	m_nCurHitMarkCount = 0;
}

CFcHitMarkMgr::~CFcHitMarkMgr(void)
{
}

void CFcHitMarkMgr::AddHitMark( CCrossVector &Cross, int nFrame, int nHitMarkIndex )
{
	int i;
	float fLength;
	D3DXVECTOR3 Distance;
	
	if( m_nCurHitMarkCount >= MAX_HITMAKR_BUF_SIZE )
	{
		return;
	}

	for( i = 0; i < m_nCurHitMarkCount; i++ )
	{
		if( m_HitMarkList[ i ].nHitMarkIndex == nHitMarkIndex )
		{
			Distance = m_HitMarkList[ i ].Position - Cross.m_PosVector;
			fLength = D3DXVec3Length( &Distance );
			if( fLength < MIN_HITMARK_DISTANCE )
			{
				return;
			}
		}
	}
	m_HitMarkList[ m_nCurHitMarkCount ].Position = Cross.m_PosVector;
	m_HitMarkList[ m_nCurHitMarkCount ].nFrame = nFrame;
	m_HitMarkList[ m_nCurHitMarkCount ].nHitMarkIndex = nHitMarkIndex;
	m_nCurHitMarkCount++;
	g_BsKernel.CreateParticleObject( nHitMarkIndex, false, false, Cross );
}

void CFcHitMarkMgr::ProcessHitMarkMgr( int nProcessTick )
{
	int i;

	for( i = 0; i < m_nCurHitMarkCount; i++ )
	{
		if( m_HitMarkList[ i ].nFrame >= nProcessTick - MIN_HITMARK_FRAME_DIST )
		{
			memcpy( m_HitMarkList, m_HitMarkList + i, sizeof( HitMarkInfo ) * ( m_nCurHitMarkCount - i ) );
			m_nCurHitMarkCount -= i;
			return;
		}
	}
	if( m_nCurHitMarkCount > 0 )
	{
		if( m_HitMarkList[ m_nCurHitMarkCount - 1 ].nFrame < nProcessTick - MIN_HITMARK_FRAME_DIST )
		{	
			m_nCurHitMarkCount = 0;
		}
	}
}
