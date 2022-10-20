#include "StdAfx.h"
#include "FcHeroSeqEvent.h"
#include "FcHeroObject.h"
#include "FcProp.h"
#include "ASData.h"
#include "Data/SignalType.h"
#include "FcWorld.h"
#include "FcPropManager.h"
#include "DebugUtil.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


/*
 *	히어로 말타기 시퀀서
 */
CFcHeroSeqRideHorseEvent::CFcHeroSeqRideHorseEvent( CFcEventSequencerMng *pMng )
: CFcEventSequencerElement( pMng )
{
	m_nFlag = 0;
}

CFcHeroSeqRideHorseEvent::~CFcHeroSeqRideHorseEvent()
{
	CFcHeroObject *pObject = (CFcHeroObject *)m_pMng->GetVoid();

	*pObject->GetMoveTargetPos() = D3DXVECTOR3( 0.f, 0.f, 0.f );
	if( pObject->GetParentLinkHandle() )
		*pObject->GetParentLinkHandle()->GetMoveTargetPos() = D3DXVECTOR3( 0.f, 0.f, 0.f );
}

void CFcHeroSeqRideHorseEvent::Run()
{
	CFcHeroObject *pObject = (CFcHeroObject *)m_pMng->GetVoid();
	switch( m_nFlag ) {
		case 0: // 위치 선정
			{
				D3DXVECTOR3 vCross, vVec;

				vVec = pObject->GetPos() - m_Handle->GetPos();
				D3DXVec3Normalize( &vVec, &vVec );

				D3DXVec3Cross( &vCross, &m_Handle->GetCrossVector()->m_ZVector, &vVec );
				if( vCross.y > 0 ) {
					m_vPos = m_Handle->GetPos() + ( m_Handle->GetCrossVector()->m_XVector * 150.f );
					m_vDir = -m_Handle->GetCrossVector()->m_XVector; m_vDir.y = 0.f;
					m_nAniTypeIndex = 1;
				}
				else {
					m_vPos = m_Handle->GetPos() + ( m_Handle->GetCrossVector()->m_XVector * -150.f );
					m_vDir = m_Handle->GetCrossVector()->m_XVector; m_vDir.y = 0.f;
					m_nAniTypeIndex = 2;
				}

				pObject->CmdMove( (int)m_vPos.x, (int)m_vPos.z, -1.f, ANI_TYPE_WALK, -1 );
				m_nFlag = 1;
			}
			break;
		case 1:	// 말옆으로 이동, 바라보기
			pObject->CmdMove( (int)m_vPos.x, (int)m_vPos.z, -1.f, ANI_TYPE_WALK, -1 );
			pObject->SetFlocking( false );
			if( ( pObject->GetCurAniType() & 0x00FF ) == ANI_TYPE_STAND ) {
				*pObject->GetMoveTargetPos() = D3DXVECTOR3( 0.f, 0.f, 0.f );
				D3DXVECTOR3 *pDir = pObject->GetMoveDir();
				*pDir = m_vDir;
				pDir->y = 0.f;
				m_nFlag = 2;
			}
			break;
		case 2:	// 타기
			if( m_vDir == pObject->GetCrossVector()->m_ZVector ) {
				if( pObject->RideOn( m_Handle ) ) {

					pObject->ChangeAnimation( ANI_TYPE_RIDE_HORSE, m_nAniTypeIndex, ANI_ATTR_HORSE );
					pObject->SetBlendFrame( 0 );
				}
				m_nFlag = -1;

			}
			break;
	}
}

bool CFcHeroSeqRideHorseEvent::IsDestroy()
{
	if( m_nFlag == -1 ) return true;
	return false;
}


/*
 *	트롤 프랍들기 시퀀서
 */


CFcHeroSeqPickupEvent::CFcHeroSeqPickupEvent( CFcEventSequencerMng *pMng )
: CFcEventSequencerElement( pMng )
{
	m_nFlag = 0;
	m_nWeaponIndex = -1;
}

CFcHeroSeqPickupEvent::~CFcHeroSeqPickupEvent()
{
	CFcHeroObject *pObject = (CFcHeroObject *)m_pMng->GetVoid();

	*pObject->GetMoveTargetPos() = D3DXVECTOR3( 0.f, 0.f, 0.f );
	if( pObject->GetParentLinkHandle() )
		*pObject->GetParentLinkHandle()->GetMoveTargetPos() = D3DXVECTOR3( 0.f, 0.f, 0.f );
}

void CFcHeroSeqPickupEvent::Run()
{
	CFcHeroObject *pObject = (CFcHeroObject *)m_pMng->GetVoid();
//	pObject->SetFlocking( true, 10.f );
	switch( m_nFlag ) {
		case 0:	// 위치선정
			{
				// 프랍의 크로스 먼저 구한당..
				CCrossVector Cross;
				D3DXMATRIX matObject;
				matObject = *(D3DXMATRIX *)g_BsKernel.SendMessage( m_pProp->GetEngineIndex(), BS_GET_OBJECT_MAT );
				Cross.m_PosVector = *((D3DXVECTOR3*)&matObject._41);
				Cross.m_XVector = *((D3DXVECTOR3*)&matObject._11);
				Cross.m_YVector = *((D3DXVECTOR3*)&matObject._21);
				Cross.m_ZVector = *((D3DXVECTOR3*)&matObject._31);

				D3DXVECTOR3 vVec;

				vVec = pObject->GetPos() - m_pProp->GetPos();
				D3DXVec3Normalize( &vVec, &vVec );

				m_vPos = m_pProp->GetPos();
				m_vDir = Cross.m_ZVector; m_vDir.y = 0.f;


				m_vDirStart = m_vPos - pObject->GetPos();
				m_vDirStart.y = 0.f;
				m_nLoopCount = 0;

				D3DXVec3Normalize( &m_vDirStart, &m_vDirStart );

				D3DXVECTOR3 *pDir = pObject->GetMoveDir();
				*pObject->GetMoveTargetPos() = D3DXVECTOR3( 0.f, 0.f, 0.f );
				*pDir = m_vDirStart;
				pDir->y = 0.f;

				m_nFlag = 1;

				/*
				if( D3DXVec3Length( &m_vDirStart ) > pObject->GetMoveSpeed() ) {

					D3DXVec3Normalize( &m_vDirStart, &m_vDirStart );

					D3DXVECTOR3 *pDir = pObject->GetMoveDir();
					*pDir = m_vDirStart;
					pDir->y = 0.f;

					m_nFlag = 1;
				}
				else {
					D3DXVECTOR3 *pDir = pObject->GetMoveDir();
					*pDir = m_vDir;
					pDir->y = 0.f;

					m_nFlag = 3;
				}
				*/
			}
			break;
		case 1: // 이동전 먼저 방향틀기~
			if( D3DXVec3Length( pObject->GetMoveDir() ) <= 0.f ) {
				pObject->CmdMove( (int)m_vPos.x, (int)m_vPos.z, -1.f, ANI_TYPE_WALK );
				m_nFlag = 2;
			}
			else {
				/*
				D3DXVECTOR3 *pDir = pObject->GetMoveDir();
				*pDir = m_vDirStart;
				pDir->y = 0.f;
				*/
			}
			break;
		case 2:	// 이동, 방향 바꾸기
			if( pObject->IsStand() ) {
				*pObject->GetMoveTargetPos() = D3DXVECTOR3( 0.f, 0.f, 0.f );
				D3DXVECTOR3 *pDir = pObject->GetMoveDir();
				*pDir = m_vDir;
				pDir->y = 0.f;
				m_nFlag = 3;
			}
			else {
				++m_nLoopCount;
				if( m_nLoopCount > 150 ) {
					m_nFlag = 0;
				}
//				pObject->CmdMove( (int)m_vPos.x, (int)m_vPos.z, -1.f, ANI_TYPE_WALK );
//				pObject->SetFlocking( false );
			}
			break;
		case 3:	// 에뉘~
			if( D3DXVec3Length( pObject->GetMoveDir() ) <= 0.f ) {
				int nAniIndex = 0;
				PROPTYPEDATA *pData = g_FcWorld.GetPropManager()->GetPropTypeData_( m_pProp->GetTypeDataIndex() );
				if( strcmp( pData->cName, "P_PL_ET13.SKIN" ) == NULL ) {
					nAniIndex = 0;
				}
				else if( strcmp( pData->cName, "P_PL_ET14.SKIN" ) == NULL ) {
					nAniIndex = 1;
				}
				else if( strcmp( pData->cName, "P_MP_TR09.SKIN" ) == NULL ) {
					nAniIndex = 2;
				}
				pObject->ChangeAnimation( ANI_TYPE_CUSTOM_0, nAniIndex );
				// Show Weapon 시그널의 위치를 찾는다.
				CAniSignal *pSignalList;
				ASSignalData *pSignal;

				pSignalList = pObject->GetASData()->GetSignalList( pObject->GetAniIndex() );
				int nCount = pSignalList->GetSignalCount();

				int nFrame = 0;
				for( int i=0; i<nCount; i++ ) {
					pSignal = pSignalList->GetSignal( i );
					if( pSignal->m_nID == SIGNAL_TYPE_SHOW_WEAPON ) {
						nFrame = pSignal->m_nFrame;
						break;
					}
				}

				// 이벤트를 하나 더 둔다.
				// 이 이벤트 자체는 중간에 켄슬되어두 되지만
				// 프랍 하이드 되는 이벤트는 남아있어야 하기 떄문!!
				CFcHeroSeqHidePropEvent *pElement = new CFcHeroSeqHidePropEvent( m_pMng );
				pElement->m_bResetEvent = false;
				pElement->m_nAniIndex = pObject->GetAniIndex();
				pElement->m_nHidePropFrame = nFrame;
				pElement->m_pProp = m_pProp;
				m_pMng->AddEvent( pElement );

				m_nFlag = -1;
			}
			break;
			/*
		case 4: // 프랍 없에기 위해서!!
			DebugString( "Frame : %d\n", (int)pObject->GetCurFrame() );
			if( pObject->GetCurFrame() >= m_nHidePropFrame && ( pObject->GetCurFrame() < m_nHidePropFrame + pObject->GetFrameAdd() ) ) {
				g_FcWorld.GetPropManager()->RemoveProp( m_pProp );
				m_nFlag = -1;
			}
			break;
			*/
	}
}

bool CFcHeroSeqPickupEvent::IsDestroy()
{
	if( m_nFlag == -1 ) return true;
	CFcHeroObject *pObject = (CFcHeroObject *)m_pMng->GetVoid();
	if( pObject->IsDie() ) return true;
	return false;
}


CFcHeroSeqHidePropEvent::CFcHeroSeqHidePropEvent( CFcEventSequencerMng *pMng )
: CFcEventSequencerElement( pMng )
{
	m_nFlag = 0;
}

CFcHeroSeqHidePropEvent::~CFcHeroSeqHidePropEvent()
{
}

void CFcHeroSeqHidePropEvent::Run()
{
	CFcHeroObject *pObject = (CFcHeroObject *)m_pMng->GetVoid();
	if( pObject->GetAniIndex() != m_nAniIndex ) { 
		m_nFlag = -1;
		return;
	}
	if( pObject->GetCurFrame() >= m_nHidePropFrame && ( pObject->GetCurFrame() < m_nHidePropFrame + pObject->GetFrameAdd() ) ) {
		g_FcWorld.GetPropManager()->RemoveProp( m_pProp );
		m_nFlag = -1;
	}
}

bool CFcHeroSeqHidePropEvent::IsDestroy()
{
	if( m_nFlag == -1 ) return true;
	return false;
}
