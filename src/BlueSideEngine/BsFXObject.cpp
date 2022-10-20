#include "StdAfx.h"
#include "BsKernel.h"
#include "BsFXObject.h"
#include "BsFXElement.h"
#include "bstreamext.h"
#include "BsFXParticleElement.h"
#include "BsFXMeshElement.h"
#include "BsFXPLElement.h"
#include "Box3.h"
#include "BsFXLoadObject.h"


DWORD CBsFXObject::m_dwProcessTick = 0;
CBsFXObject::CBsFXObject()
: CBsObject()
{
	SetObjectType(BS_FX_OBJECT);
	m_State = CBsFXObject::STOP;
	m_dwTotalFrame = 0;
	m_dwCurFrame = 0;
	m_dwOldTick = 0;
	m_dwPauseFrame = -1;
	m_nLoopCount = 0;
	m_nTotalLoopCount = 0;

	SetShadowCastType(BS_SHADOW_NONE);

	m_bShow = true;
//	m_fDistanceFromCamera = 0.f;	// mruete: prefix bug 320: renamed to m_fDistanceFromCamera


	m_bEnableInstancing = false;

#ifndef _LTCG
	m_pFXTemplate = NULL;
#endif

}

CBsFXObject::~CBsFXObject()
{
	SAFE_DELETE_PVEC( m_pVecElement );
}

void CBsFXObject::Release()
{
	for( DWORD i=0; i<m_pVecElement.size(); i++ ) {
		m_pVecElement[i]->DeleteEngineObject();
	}
}

#ifndef _LTCG
CBsFXTemplate * CBsFXObject::GetFXTemplate()
{
	return m_pFXTemplate;
}
#endif

DWORD CBsFXObject::GetProcessTick()
{
	return m_dwProcessTick;
//	return g_BsKernel.GetTick();
}

// mruete: prefix bug 320: renamed to m_fDistanceFromCamera
bool CBsFXObject::InitRender( float fDistanceFromCamera )
{
	CBsObject::InitRender( fDistanceFromCamera );
//	m_fDistanceFromCamera = fDistanceFromCamera;

//	if( m_State == CBsFXObject::STOP ) return;
	Process();
//	if( m_bShow == FALSE ) return;
	for( DWORD i=0; i<m_pVecElement.size(); i++ ) {
		m_pVecElement[i]->SetParentMatrix( ( D3DXMATRIX * )GetObjectMatrixByProcess() );
		if( m_pVecElement[i]->GetType() == CBsFXElement::PARTICLE) {
			int count = m_nTotalLoopCount - m_nLoopCount;
			DWORD ParticlecurrFrame = m_dwCurFrame + m_dwTotalFrame*count;
			m_pVecElement[i]->Process( ParticlecurrFrame, fDistanceFromCamera,count );		
		}
		else
		{
			m_pVecElement[i]->Process( m_dwCurFrame, fDistanceFromCamera );	
		}
	}
	return false;
}

void CBsFXObject::Process()
{
	switch( m_State ) {
		case CBsFXObject::PLAY:
		case CBsFXObject::RESUME:
		case CBsFXObject::STOPLOOP:
			{
				m_State = CBsFXObject::PLAY;
				DWORD dwProcessTick = GetProcessTick();
				m_dwCurFrame = dwProcessTick - m_dwOldTick;
			}			
			break;
		case CBsFXObject::PAUSE:
			m_dwPauseFrame = m_dwCurFrame;
			break;
		case CBsFXObject::STOP:
			Show( false );
			break;
	}

	if( m_nLoopCount != 0 && m_dwCurFrame == 0 ) 
		m_dwCurFrame++;

	if( m_dwCurFrame >= m_dwTotalFrame ) {
		if( m_nLoopCount == 0 ) {
			m_State = CBsFXObject::STOP;
			m_dwCurFrame = m_dwTotalFrame;
		}
		else {
			m_nLoopCount--;
			m_dwOldTick = GetProcessTick();
			m_dwCurFrame -= m_dwTotalFrame;
			m_dwCurFrame++;
		}
	}
}

int CBsFXObject::ProcessMessage(DWORD dwCode, DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{
	for( DWORD i=0; i<m_pVecElement.size(); i++ ) {
		if( m_pVecElement[i]->GetObjectIndex() == -1 ) continue;
		switch( m_pVecElement[i]->GetType() ) {
			case CBsFXElement::PARTICLE:
				if( dwCode == BS_ENABLE_OBJECT_CULL ) {
					g_BsKernel.SetParticleCull( m_pVecElement[i]->GetObjectIndex(), ( dwParam1 == TRUE ) );
					g_BsKernel.UseParticleAlphaFog( m_pVecElement[i]->GetObjectIndex(), ( dwParam1 == TRUE ) );
				}
				break;
			case CBsFXElement::MESH:
				g_BsKernel.SendMessage( m_pVecElement[i]->GetObjectIndex(), dwCode, dwParam1, dwParam2, dwParam3 );
				break;
		}
	}
	return CBsObject::ProcessMessage( dwCode, dwParam1, dwParam2, dwParam3 );
}

void CBsFXObject::Initialize( CBsFXTemplate *pLoadObj)
{
	BsAssert( m_pVecElement.empty() );

#ifndef _LTCG
	m_pFXTemplate = pLoadObj;
#endif

	m_dwTotalFrame = pLoadObj->m_dwTotalFrame;

	for( DWORD i=0; i<pLoadObj->m_pVecElement.size(); i++ ) {
		CBsFXElement *pSor;
		switch( pLoadObj->m_pVecElement[i]->GetType() ) {
			case CBsFXElement::PARTICLE:	pSor = new CBsFXParticleElement( pLoadObj->m_pVecElement[i] );	break;
			case CBsFXElement::MESH:		pSor = new CBsFXMeshElement( pLoadObj->m_pVecElement[i] );		break;
			case CBsFXElement::POINT_LIGHT:	pSor = new CBsFXPLElement( pLoadObj->m_pVecElement[i] );		break;
			default:
				return;
		}
		m_pVecElement.push_back( pSor );
		pSor->Activate();
	}
	SetState( STOP );
}

DWORD CBsFXObject::GetElementCount()
{
	return (DWORD)m_pVecElement.size();
}

CBsFXElement *CBsFXObject::GetElementFromIndex( DWORD dwIndex )
{
	if( dwIndex < 0 || dwIndex >= GetElementCount() ) return NULL;
	return m_pVecElement[dwIndex];
}

void CBsFXObject::SetState( STATE State, int nParam1, int nParam2 )
{
	m_State = State;
	switch( State ) {
		case CBsFXObject::PLAY:
			SetCurFrame(-1);
			m_dwOldTick = GetProcessTick();

			if( nParam1 > 0 )
			{
				m_nLoopCount = nParam1 - 1;
				m_nTotalLoopCount = m_nLoopCount;
			}
			else
			{
				m_nLoopCount = nParam1;
				m_nTotalLoopCount = m_nLoopCount;
			}

//			if( m_nLoopCount == 0 ) m_nLoopCount = 1;
			break;
		case CBsFXObject::STOP:
			Show( false );
			for( DWORD i=0; i<m_pVecElement.size(); i++ ) {
				m_pVecElement[i]->SetParentMatrix( ( D3DXMATRIX *)GetObjectMatrixByProcess() );
//				m_pVecElement[i]->Process( m_dwCurFrame, m_fDistanceFromCamera );
				m_pVecElement[i]->Process( m_dwCurFrame, GetDistanceFromCamByProcess() );
			}
			break;
		case CBsFXObject::PAUSE:
			m_dwPauseFrame = m_dwCurFrame;
			break;
		case CBsFXObject::RESUME:
			m_dwOldTick = GetProcessTick() - m_dwPauseFrame;
			break;
		case CBsFXObject::STOPLOOP:
			m_nLoopCount = 0;
			m_nTotalLoopCount = 0;
			break;
	}
}

CBsFXObject::STATE CBsFXObject::GetCurState()
{
	return m_State;
}

void CBsFXObject::SetCurFrame( DWORD dwFrame )
{
	m_dwCurFrame = dwFrame;
	for( DWORD i=0; i<m_pVecElement.size(); i++ ) {
		m_pVecElement[i]->SetCurFrame( m_dwCurFrame );
	}
}

DWORD CBsFXObject::GetCurFrame()
{
	return m_dwCurFrame;
}

DWORD CBsFXObject::GetTotalFrame()
{
	return m_dwTotalFrame;
}

void CBsFXObject::Show( bool bIsShow )
{
	m_bShow = bIsShow;
	for( DWORD i=0; i<m_pVecElement.size(); i++ ) {
		if( m_bShow == true )
			m_pVecElement[i]->Show();
		else 
		{
			m_pVecElement[i]->Hide();
		}
	}
}

bool CBsFXObject::GetBox3( Box3& B )
{
	const D3DXMATRIX *pMatrix;

	pMatrix = GetObjectMatrixByProcess();
	B.C.x = pMatrix->_41;
	B.C.y = pMatrix->_42;
	B.C.z = pMatrix->_43;

	B.E[0] = 1000.f;
	B.E[1] = 1000.f;
	B.E[2] = 1000.f;

	B.A[0] = *((BSVECTOR *)&pMatrix->_11);
	B.A[1] = *((BSVECTOR *)&pMatrix->_21);
	B.A[2] = *((BSVECTOR *)&pMatrix->_31);
	return true;
}

std::vector<CBsFXElement *> *CBsFXObject::GetElementList()
{
	return &m_pVecElement;
}