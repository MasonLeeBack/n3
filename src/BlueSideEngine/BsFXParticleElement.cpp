#include "StdAfx.h"
#include "BsFXElement.h"
#include "BsFXParticleElement.h"
#include "BsKernel.h"
#include "bstreamext.h"


CBsFXParticleElement::CBsFXParticleElement( CBsFXElement *pParent )
: CBsFXElement( pParent )
{
	m_Type = CBsFXElement::PARTICLE;
	if( pParent ) {
		m_bIterator = ((CBsFXParticleElement*)pParent)->m_bIterator;
		m_bLoop = ((CBsFXParticleElement*)pParent)->m_bLoop;
		m_bStopFlag = ((CBsFXParticleElement*)pParent)->m_bStopFlag;
	}
	else {
		m_bIterator = FALSE;
		m_bLoop = TRUE;
		m_bStopFlag = TRUE;
	}

	if ( m_nLoadIndex !=- 1 ) {
		g_BsKernel.AddParticleGroupRef( m_nLoadIndex );
	}
}

CBsFXParticleElement::~CBsFXParticleElement()
{
	Deactivate();
	SAFE_RELEASE_PARTICLE_GROUP(m_nLoadIndex);
}

void CBsFXParticleElement::Process( DWORD dwTime, float fDistanceFromCamera, int count /*= 0*/ )
{
	if( m_nObjectIndex == -1 ) return;

	if( m_bShow == FALSE ) {
		g_BsKernel.ShowParticle( m_nObjectIndex, false );
		return;
	}

	if( count ) {
		DWORD dwTime2 = dwTime - m_dwEndFrame*count;
		CBsFXElement::Process( dwTime2, fDistanceFromCamera );
	}
	else
	{
		CBsFXElement::Process( dwTime, fDistanceFromCamera );
	}
	
	if( m_bStopFlag == TRUE ) {
		if( dwTime < m_dwStartFrame || dwTime >= m_dwEndFrame ) {
			g_BsKernel.ShowParticle( m_nObjectIndex, false );
			return;
		}
	}
	else {
		if( dwTime < m_dwStartFrame ) {
			g_BsKernel.ShowParticle( m_nObjectIndex, false );
			return;
		}
	}
	g_BsKernel.SetParticleTick( m_nObjectIndex, dwTime - m_dwStartFrame );
	g_BsKernel.ShowParticle( m_nObjectIndex, true );

	// UpdateObject
	g_BsKernel.UpdateParticle( m_nObjectIndex, &m_matObject, true, &m_Color );
}

BOOL CBsFXParticleElement::Load( BStream *pStream )
{
	BsAssert( m_nLoadIndex == -1 );

	CBsFXElement::Load( pStream );

	pStream->Read( &m_bIterator, sizeof(BOOL), ENDIAN_FOUR_BYTE );
	pStream->Read( &m_bLoop, sizeof(BOOL), ENDIAN_FOUR_BYTE );
	pStream->Read( &m_bStopFlag, sizeof(BOOL), ENDIAN_FOUR_BYTE );

	// Load Particle
	m_nLoadIndex = g_BsKernel.LoadParticleGroup( -1, m_szFileName.c_str() );
	if( m_nLoadIndex == -1 ) return FALSE;

	return TRUE;
}

BOOL CBsFXParticleElement::Activate()
{
	BsAssert( m_nObjectIndex == -1 );
	m_nObjectIndex = g_BsKernel.CreateParticleObject( m_nLoadIndex, m_bLoop!=0, m_bIterator!=0, &m_matObject, 1.f, NULL, false );

	if( m_nObjectIndex > -1 ) return TRUE;

	return FALSE;
}

void CBsFXParticleElement::SetCurFrame( DWORD dwValue )
{
	if( m_nObjectIndex == -1 ) return;
	g_BsKernel.SetParticleTick( m_nObjectIndex, (int)dwValue );
}

DWORD CBsFXParticleElement::GetCurFrame()
{
	if( m_nObjectIndex == -1 ) return 0;
	return g_BsKernel.GetParticleTick( m_nObjectIndex );
}


void CBsFXParticleElement::Deactivate()
{
	if( m_nObjectIndex != -1 ) 
	{
		g_BsKernel.DeleteParticleObject( m_nObjectIndex );
		m_nObjectIndex = -1;
	}
}


