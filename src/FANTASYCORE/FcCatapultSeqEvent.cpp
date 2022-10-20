#include "StdAfx.h"
#include "FcCatapultSeqEvent.h"
#include "FcEventSequencer.h"
#include "FcCatapultObject.h"
#include "FcGlobal.h"
#include "FcTroopObject.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG



CFcCatapultSeqAttackEvent::CFcCatapultSeqAttackEvent( CFcEventSequencerMng *pMng )
: CFcEventSequencerElement( pMng )
{
	m_nFlag = 0;
	m_nAttackTime = 0;
}

CFcCatapultSeqAttackEvent::~CFcCatapultSeqAttackEvent()
{
}

void CFcCatapultSeqAttackEvent::Run()
{
	CFcCatapultObject *pObject = (CFcCatapultObject *)m_pMng->GetVoid();
	if( pObject->GetParentTroop()->GetState() != TROOPSTATE_RANGE_ATTACK ) {
		m_nFlag = -1;
		return;
	}

	switch( m_nFlag ) {
		case 0:
			{
				D3DXVECTOR3 vVec = m_vTargetPos - pObject->GetPos();
				D3DXVec3Normalize( &vVec, &vVec );
				*pObject->GetMoveDir() = vVec;
				*pObject->GetMoveTargetPos() = m_vTargetPos;
				m_nFlag = 1;
			}
			break;
		case 1:
			{
				if( D3DXVec3LengthSq( pObject->GetMoveDir() ) > 0.f ) break;
				pObject->ChangeAnimation( ANI_TYPE_CUSTOM_0, 0 );
				GameObjHandle Handle;
				for( DWORD i=0; i<pObject->GetLinkObjCount(); i++ ) {
					Handle = pObject->GetLinkObjHandle(i);
					if( !Handle->IsDie() && Handle->GetHP() > 0 && ( Handle->GetCurAniType() == ANI_TYPE_STAND || Handle->GetCurAniType() == ANI_TYPE_WALK ||Handle->GetCurAniType() == ANI_TYPE_CUSTOM_1  ) ) 
						Handle->ChangeAnimation( ANI_TYPE_CUSTOM_1, pObject->GetLinkObjAniIndex(i) );
				}
				m_nFlag = 2;
			}
			break;
		case 2:
			{
				if( pObject->GetCurAniType() == ANI_TYPE_CUSTOM_0 && pObject->GetCurAniIndex() == 1 ) {
					GameObjHandle Handle;
					for( DWORD i=0; i<pObject->GetLinkObjCount(); i++ ) {
						Handle = pObject->GetLinkObjHandle(i);
						if( !Handle->IsDie() && Handle->GetHP() > 0 && ( Handle->GetCurAniType() == ANI_TYPE_STAND || Handle->GetCurAniType() == ANI_TYPE_WALK ||Handle->GetCurAniType() == ANI_TYPE_CUSTOM_1  ) ) 
							Handle->ChangeAnimation( ANI_TYPE_STAND, pObject->GetLinkObjAniIndex(i) );
					}
					m_nFlag = 3;
				}
			}
			break;
		case 3:
			if( pObject->GetCurAniType() == ANI_TYPE_ATTACK ) {
				GameObjHandle Handle;
				for( DWORD i=0; i<pObject->GetLinkObjCount(); i++ ) {
					if( pObject->GetLinkObjAniIndex(i) < 2 ) continue;
					Handle = pObject->GetLinkObjHandle(i);
					if( !Handle->IsDie() && Handle->GetHP() > 0 && ( Handle->GetCurAniType() == ANI_TYPE_STAND || Handle->GetCurAniType() == ANI_TYPE_WALK ||Handle->GetCurAniType() == ANI_TYPE_CUSTOM_1  ) ) 
						Handle->ChangeAnimation( ANI_TYPE_ATTACK, pObject->GetLinkObjAniIndex(i) );
				}
				m_nAttackTime = GetProcessTick();
				m_nFlag = 4;
			}
			break;
		case 4:
			if( GetProcessTick() - m_nAttackTime > 300 ) m_nFlag = -1;
			break;
	}
}

bool CFcCatapultSeqAttackEvent::IsDestroy()
{
	if( m_nFlag == -1 ) return true;
	return false;
}
