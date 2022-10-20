#include "StdAfx.h"
#include "FcAdjutantObject.h"

#include "FcFXCommon.h"
#include "FcFxManager.h"

#include "FcWorld.h"
#include "./Data/SignalType.h"
#include "FcAIObject.h"
#include "FcAISearchSlot.h"
#include "FcAdjutantKingFrog.h"
#include "FcHeroObject.h"

int CFcAdjutantKingFrog::s_nRotationAngle = 18;

CFcAdjutantKingFrog::CFcAdjutantKingFrog(CCrossVector *pCross)
: CFcAdjutantObject( pCross )
{
	SetAdjutantClassID( Class_ID_Adjutant_KingFrog );
}

CFcAdjutantKingFrog::~CFcAdjutantKingFrog()
{
}

void CFcAdjutantKingFrog::SignalCheck( ASSignalData *pSignal )
{
	switch( pSignal->m_nID ) {
		case SIGNAL_TYPE_CUSTOM_MESSAGE:
			if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) ) {
				switch( pSignal->m_pParam[0] ) {
					case 0:
						{
							m_Cross.RotateYaw( (int)( pSignal->m_pParam[1] * 2.84f ) );
							m_MoveTargetPos = D3DXVECTOR3( 0.f, 0.f, 0.f );
							m_MoveDir = D3DXVECTOR3( 0.f, 0.f, 0.f );

							ChangeAnimation( ANI_TYPE_STAND, 0, ANI_ATTR_BATTLE );
							m_fFrame = 0.f;
							g_BsKernel.SetCurrentAni( m_nEngineIndex, m_nAniIndex, (float)m_fFrame );
							m_nBlendFrame = 0;
						}
						break;
				}

			}
			return;
		default:
			break;
	}
	CFcAdjutantObject::SignalCheck( pSignal );
}

void CFcAdjutantKingFrog::RotationProcess()
{
	float fLength;
	D3DXVECTOR3 CrossVec;

	if( m_bDead ) return;
	fLength = D3DXVec3LengthSq(&m_MoveDir);
	if( fLength <= 0.0f ) {
		/*
		if( m_nCurAniType == ANI_TYPE_CUSTOM_1 ) {
			int nFindAni = FindExactAni( m_pUnitInfoData->GetAniIndex( ANI_TYPE_STAND, 0 ) );
			ChangeAnimationByIndex( nFindAni );
		}
		*/
		return;
	}

	fLength = D3DXVec3LengthSq( &m_MoveTargetPos );
	if( fLength > 0.f ) {
		D3DXVECTOR3 vTargetVec = m_MoveTargetPos - GetPos();
		vTargetVec.y = 0.f;
		D3DXVec3Normalize( &vTargetVec, &vTargetVec );
		m_MoveDir = vTargetVec;
	}

	float fDot=D3DXVec3Dot(&m_MoveDir, &(m_Cross.m_ZVector));
	float fAngle = acos( fDot ) * 180.f / 3.1415926f;

	float fMinAngle = s_nRotationAngle / 2.84f;
	if( fAngle > fMinAngle ){
		D3DXVec3Cross(&CrossVec, &m_MoveDir, &(m_Cross.m_ZVector));
		if(CrossVec.y>0) {
			m_Cross.RotateYaw( s_nRotationAngle );
		}
		else {
			m_Cross.RotateYaw( -s_nRotationAngle );
		}
		/*
		if(CrossVec.y>0) {
			if( m_nCurAniType == ANI_TYPE_STAND && m_nChangeMoveAniIndex == -1 ) {
				ChangeAnimation( ANI_TYPE_CUSTOM_1, 1 );
			}
			if( m_nCurAniType != ANI_TYPE_CUSTOM_1 )
				m_Cross.RotateYaw( s_nRotationAngle );
		}
		else {
			if( m_nCurAniType == ANI_TYPE_STAND && m_nChangeMoveAniIndex == -1 ) {
				ChangeAnimation( ANI_TYPE_CUSTOM_1, 0 );
			}
			if( m_nCurAniType != ANI_TYPE_CUSTOM_1 )
				m_Cross.RotateYaw( -s_nRotationAngle );
		}
		*/

		float fPrevAngle = fAngle;
		fDot=D3DXVec3Dot(&m_MoveDir, &(m_Cross.m_ZVector));
		fAngle = acos( fDot ) * 180.f / 3.1415926f;
		if( fAngle <= fMinAngle ) {
			m_Cross.m_ZVector=m_MoveDir;
			m_Cross.UpdateVectors();
			m_MoveDir=D3DXVECTOR3(0.0f, 0.0f, 0.0f);

			/*
			if( m_nCurAniType == ANI_TYPE_CUSTOM_1 ) {
				int nFindAni = FindExactAni( m_pUnitInfoData->GetAniIndex( ANI_TYPE_STAND, 0 ) );
				ChangeAnimationByIndex( nFindAni );
			}
			*/
		}	
		return;
	}

	m_Cross.m_ZVector=m_MoveDir;
	m_Cross.UpdateVectors();
	m_MoveDir=D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	/*
	if( m_nCurAniType == ANI_TYPE_CUSTOM_1 ) {
		int nFindAni = FindExactAni( m_pUnitInfoData->GetAniIndex( ANI_TYPE_STAND, 0 ) );
		ChangeAnimationByIndex( nFindAni );
	}
	*/
}


bool CFcAdjutantKingFrog::IsFlockable( GameObjHandle Handle )
{
	if( Handle == m_Handle ) return false;
	if( Handle->GetParentLinkHandle() == m_Handle ) return false;

	switch( Handle->GetClassID() ) {
		case Class_ID_Hero:
			switch( ((HeroObjHandle)Handle)->GetHeroClassID() ) {
				case Class_ID_Hero_VigkVagk:	return true;
			}
			return false;
		case Class_ID_Catapult: return true;
		case Class_ID_Adjutant:	return false;
		default:
			break;
	}
	return false;
}

void CFcAdjutantKingFrog::ProcessFlocking()
{
	if( IsJump() ) return;
	CFcAdjutantObject::ProcessFlocking();
}