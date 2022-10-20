#include "StdAfx.h"
#include "FcCatapultObject.h"
#include "FcUnitObject.h"
#include "FcWorld.h"
#include "DistVec3Lin3.h"
#include "IntLin3Box3.h"
#include "FcVelocityCtrl.h"
#include "FcAIObject.h"
#include "FcCatapultSeqEvent.h"
#include "FcTroopObject.h"
#include "FcGlobal.h"
#include "FcWorld.h"
#include "./Data/SignalType.h"
#include "FcProjectile.h"
#include "DebugUtil.h"
#include "FcSoundManager.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


float CFcCatapultObject::s_fRotationAngle = 0.5;

CFcCatapultObject::LinkBoneInfo CFcCatapultObject::s_LinkBoneName[] = {
	{ "Soldier_Dummy01", 0, },
	{ "Soldier_Dummy02", 1, },
	{ "Soldier_Dummy03", 2, },
	{ "Soldier_Dummy04", 3, },
};
char *CFcCatapultObject::s_szWheelBoneName[] = {
	"BoxBone02", 
	"BoxBone04",
	"BoxBone03",
	"BoxBone01",
};


CFcCatapultObject::CFcCatapultObject( CCrossVector *pCross )
: CFcUnitObject( pCross )
{
	SetClassID( Class_ID_Catapult );

	m_pVelocityCtrl = new CFcVelocityCtrl( 0.7f, 10.f, 0.7f, 1.02f, 0.96f );
	m_pRotationCtrl[0] = new CFcVelocityCtrl( 0.1f, 0.5f, 0.1f, 1.02f, 0.96f );
	m_pRotationCtrl[1] = new CFcVelocityCtrl( 0.1f, 0.5f, 0.1f, 1.02f, 0.96f );
	m_bStopFlag = true;
	m_nRotationAniIndex = -1;
	m_fCurMaximumSpeed = 0.f;
	memset( m_fWheelAngle, 0, sizeof(m_fWheelAngle) );
	m_bWorkable = false;

	m_bDieFlag = false;
	/*
	for( int i=0; i<4; i++ ) {
		m_nDieParticleIndex[i] = -1;
	}
	*/
	m_nBreakObjectIndex = -1;
	m_nSoundMoveHandle = -1;
//	m_fRotateAngle = m_fPrevRotateAngle = 0.f;
}

CFcCatapultObject::~CFcCatapultObject()
{
	if( CFcWorld::GetInstance().GetObjectImp() ) {
		SetInScanner( false );
	}
	SAFE_DELETE( m_pRotationCtrl[0] );
	SAFE_DELETE( m_pRotationCtrl[1] );
	SAFE_DELETE( m_pVelocityCtrl );
	if( m_nBreakObjectIndex != -1) {
		g_BsKernel.DeleteObject( m_nBreakObjectIndex );
	}
}

void CFcCatapultObject::CreateParts(INITGAMEOBJECTDATA *pData)
{
#ifdef _XBOX
	int i;
	int nPartsGroupCount = m_pUnitInfoData->GetPartsGroupCount();
	for( i = 0; i < nPartsGroupCount; i++ )
	{
		ASPartsData *pPartsData = m_pUnitInfoData->GetPartsGroupInfo( i );
		if( pPartsData->m_nSimulation == PHYSICS_BREAKABLE ) {
			m_pBreakableInfo = pData->vecPhysicsInfo[0].pData;
			m_nBreakableSkinIndex = pPartsData->m_pSkinIndex[0];			
		}
	}	
#endif //_XBOX
	CFcGameObject::CreateParts( pData );
}

void CFcCatapultObject::Initialize( INITGAMEOBJECTDATA* pData, int nForce, int nTeam, TroopObjHandle hTroop, float fScaleMin, float fScaleMax )
{
	CFcGameObject::Initialize( pData, nForce, nTeam, hTroop, fScaleMin, fScaleMax );

	float fMaximumSpeed = (float)m_pUnitInfoData->GetMoveSpeed( m_pUnitInfoData->GetAniIndex( ANI_TYPE_RUN, 0 ) );
	m_pVelocityCtrl->SetMaximumSpeed( fMaximumSpeed );

	g_BsKernel.SendMessage( m_nEngineIndex, BS_USE_ANI_CACHE, 0 );
}

void CFcCatapultObject::Process()
{
	m_bWorkable = true;
	if( m_VecLinkObjHandle.size() == 0 ) m_bWorkable = false;
	else {
		for( DWORD i=0; i<m_VecLinkObjHandle.size(); i++ ) {
			if( m_VecLinkObjHandle[i]->IsHit() || m_VecLinkObjHandle[i]->GetCurAniType() == ANI_TYPE_CUSTOM_0 ) {
				m_bWorkable = false;
				break;
			}
		}
	}
	SetVelocity( &D3DXVECTOR3( 0.f, 0.f, 0.f ) );
	// Die Check
	// 임시
	/*
	if( m_bDieFlag == false && m_bDead == true ) {
		CCrossVector CrossVec;
		for( int i=0; i<4; i++ ) {
			CrossVec = m_Cross;
			switch(i) {
				case 0:	
					CrossVec.MoveRightLeft( -120.f );
					CrossVec.MoveFrontBack( 230.f );
					break;
				case 1:	
					CrossVec.MoveRightLeft( 120.f );
					CrossVec.MoveFrontBack( 230.f );
					break;
				case 2:	
					CrossVec.MoveRightLeft( -120.f );
					CrossVec.MoveFrontBack( -230.f );
					break;
				case 3:	
					CrossVec.MoveRightLeft( 120.f );
					CrossVec.MoveFrontBack( -230.f );
					break;
			}
			CrossVec.m_PosVector.y += 50.f;
			m_nDieParticleIndex[i] = g_BsKernel.CreateParticleObject( 82, true, false, CrossVec );
			m_nDieFrame = 1;
		}
		m_bDieFlag = true;
		Break();
	}
	else if( m_bDieFlag == true ) {
		if( m_nDieFrame > DIE_KEEP_FRAME )
		{
			for( int i=0; i<4; i++ ) {
				if( m_nDieParticleIndex[i] != -1 ) {
					g_BsKernel.StopParticlePlay( m_nDieParticleIndex[i] );
					m_nDieParticleIndex[i] = -1;
				}
			}
		}
	}
	*/
	if( m_bDead == true && m_bDieFlag == false ) {
		m_bDieFlag = true;
		m_nDieFrame = 1;
		for( DWORD i=0; i<m_VecLinkObjHandle.size(); i++ ) {
			m_VecLinkObjHandle[i]->RideOut();
			i--;
		}
		Break();
	}
	////////////////


	CFcGameObject::Process();

	ProcessMove();

	ProcessChildAnimation();
	ProcessWheelSpeed();

	D3DXMATRIX matTemp, matTemp2;
	GameObjHandle Handle;
	D3DXVECTOR3 vTemp, vResult;
	matTemp2 = *m_Cross;
	matTemp2._41 = matTemp2._42 = matTemp2._43 = 0.f;
	for( DWORD i=0; i<m_VecLinkObjHandle.size(); i++ ) {
		Handle = m_VecLinkObjHandle[i];
		if( !CFcBaseObject::IsValid( Handle ) ) continue;
		if( Handle->IsDie() ) continue;
		D3DXMATRIX *pMatrix = (D3DXMATRIX *)g_BsKernel.SendMessage( m_nEngineIndex, BS_GET_BONE_WORLD_MATRIX, (DWORD)s_LinkBoneName[ m_nVecLinkBoneIndex[i] ].szBoneName );
		if( pMatrix == NULL ) continue;

		D3DXMatrixMultiply( &matTemp, pMatrix, &matTemp2 );
		vTemp = D3DXVECTOR3( matTemp._41, 0.f, matTemp._43 );
		vTemp = m_Cross.m_PosVector + vTemp;

		if( Handle->IsForceMove() ) {
			vResult = vTemp - ( Handle->GetPos() + *Handle->GetMoveVector() );
		}
		else vResult = vTemp - Handle->GetPos();
		vResult.y = 0.f;
		*Handle->GetMoveVector() += vResult;
		Handle->SetForceMove( true );
		Handle->ResetMoveVector();
	}
}

void CFcCatapultObject::PostProcess()
{
	CFcUnitObject::PostProcess();
	/*
	if( m_bDead == true )
		ProcessFlocking();
	*/
}

bool CFcCatapultObject::Render()
{
	if( m_nBreakObjectIndex != -1 ) {
		g_BsKernel.UpdateObject( m_nBreakObjectIndex, m_Cross);

		if( m_nDieFrame > DIE_KEEP_FRAME ) {
			float fAlphaWeight;
			fAlphaWeight = 1.0f - ( ( m_nDieFrame - DIE_KEEP_FRAME ) / DIE_ALPHA_FRAME );

			if( m_nBreakObjectIndex != -1 )
				g_BsKernel.SendMessage( m_nBreakObjectIndex, BS_ENABLE_OBJECT_ALPHABLEND, 1 );

			if( fAlphaWeight < 0.5f ) g_BsKernel.SendMessage( m_nBreakObjectIndex, BS_SHADOW_CAST, 0 );

			g_BsKernel.SendMessage( m_nBreakObjectIndex, BS_SET_OBJECT_ALPHA, ( DWORD )&fAlphaWeight );
		}
		return true;
	}
	bool bRet;

	bRet = CFcUnitObject::Render();
	if( !bRet )
	{
		return false;
	}
	
	// Wheel
	g_BsKernel.SendMessage( m_nEngineIndex, BS_SET_CLEAR_BONE_ROTATION );
	for( int i=0; i<4; i++ ) {
		D3DXVECTOR3 vRotate = D3DXVECTOR3( m_fWheelAngle[i], 0.f, 0.f );
		g_BsKernel.SendMessage( m_nEngineIndex, BS_SET_BONE_ROTATION, (DWORD)s_szWheelBoneName[i], (DWORD)&vRotate );
	}

	switch( m_nCurAniType ) {
		case ANI_TYPE_MOVE:
			for( DWORD i=0; i<m_VecLinkObjHandle.size(); i++ ) {
				if( !m_VecLinkObjHandle[i]->IsMovable() ) continue;

				int nAniIndex = s_LinkBoneName[ m_nVecLinkBoneIndex[i] ].nWalkAniIndex;

				switch( m_nRotationAniIndex ) {
					case -1:
						break;
					case 0:
						if( nAniIndex == 0 || nAniIndex == 2 ) {
							D3DXVECTOR3 vRotate = D3DXVECTOR3( 0.f, 180.f, 0.f );
							g_BsKernel.SendMessage( m_nEngineIndex, BS_SET_BONE_ROTATION, (DWORD)s_LinkBoneName[ m_nVecLinkBoneIndex[i] ].szBoneName, (DWORD)&vRotate );
							CCrossVector *pCross = m_VecLinkObjHandle[i]->GetCrossVector();
						}
						break;
					case 1:
						if( nAniIndex == 1 || nAniIndex == 3 ) {
							D3DXVECTOR3 vRotate = D3DXVECTOR3( 0.f, 180.f, 0.f );
							g_BsKernel.SendMessage( m_nEngineIndex, BS_SET_BONE_ROTATION, (DWORD)s_LinkBoneName[ m_nVecLinkBoneIndex[i] ].szBoneName, (DWORD)&vRotate );
							CCrossVector *pCross = m_VecLinkObjHandle[i]->GetCrossVector();
						}
						break;
				}
			}
			break;
	}

	return true;
}

void CFcCatapultObject::ProcessChildAnimation()
{
	if( m_bDead ) return;
	DWORD i;
	switch( m_nCurAniType ) {
		case ANI_TYPE_STAND:
			for( i=0; i<m_VecLinkObjHandle.size(); i++ ) {
				if( !m_VecLinkObjHandle[i]->IsDie() && !m_VecLinkObjHandle[i]->IsHit() && m_VecLinkObjHandle[i]->GetCurAniType() != ANI_TYPE_CUSTOM_0 && m_VecLinkObjHandle[i]->GetCurAniType() != ANI_TYPE_CUSTOM_1 ) {
					m_VecLinkObjHandle[i]->ChangeAnimation( ANI_TYPE_STAND, s_LinkBoneName[ m_nVecLinkBoneIndex[i] ].nWalkAniIndex );
				}
			}
			break;
		case ANI_TYPE_RUN:
			for( i=0; i<m_VecLinkObjHandle.size(); i++ ) {
				if( m_VecLinkObjHandle[i]->IsMovable() && m_VecLinkObjHandle[i]->GetCurAniType() != ANI_TYPE_CUSTOM_0 && m_VecLinkObjHandle[i]->GetCurAniType() != ANI_TYPE_CUSTOM_1 )
					m_VecLinkObjHandle[i]->ChangeAnimation( ANI_TYPE_WALK, s_LinkBoneName[ m_nVecLinkBoneIndex[i] ].nWalkAniIndex );
			}
			break;
		case ANI_TYPE_MOVE:
			for( i=0; i<m_VecLinkObjHandle.size(); i++ ) {
				if( !m_VecLinkObjHandle[i]->IsMovable() || m_VecLinkObjHandle[i]->GetCurAniType() == ANI_TYPE_CUSTOM_0 || m_VecLinkObjHandle[i]->GetCurAniType() == ANI_TYPE_CUSTOM_1 ) continue;

				int nAniIndex = s_LinkBoneName[ m_nVecLinkBoneIndex[i] ].nWalkAniIndex;

				switch( m_nRotationAniIndex ) {
					case -1:
						m_VecLinkObjHandle[i]->ChangeAnimation( ANI_TYPE_WALK, nAniIndex );
						break;
					case 0:
						if( nAniIndex == 0 || nAniIndex == 2 ) {
							m_VecLinkObjHandle[i]->ChangeAnimation( ANI_TYPE_WALK, nAniIndex + 1 );
							m_nBlendFrame = 20;
						}
						else {
							m_VecLinkObjHandle[i]->ChangeAnimation( ANI_TYPE_WALK, nAniIndex );
						}
						break;
					case 1:
						if( nAniIndex == 1 || nAniIndex == 3 ) {
							m_VecLinkObjHandle[i]->ChangeAnimation( ANI_TYPE_WALK, nAniIndex -1 );
							m_nBlendFrame = 20;
						}
						else {
							m_VecLinkObjHandle[i]->ChangeAnimation( ANI_TYPE_WALK, nAniIndex );
						}
						break;
						break;
				}
			}
			break;
	}
}

void CFcCatapultObject::ProcessWheelSpeed()
{
	switch( m_nCurAniType ) {
		case ANI_TYPE_STAND:
		case ANI_TYPE_RUN:
			if( m_pVelocityCtrl->GetCurSpeed() <= m_pVelocityCtrl->GetMinSpeed() ) break;

			for( int i=0; i<4; i++ ) {
				m_fWheelAngle[i] += ( m_pVelocityCtrl->GetCurSpeed() * 0.7f );
			}
			break;
		case ANI_TYPE_MOVE:
			{
				float fTemp[2];
				fTemp[0] = m_pRotationCtrl[0]->GetCurSpeed();
				if( fTemp[0] <= m_pRotationCtrl[0]->GetMinSpeed() ) fTemp[0] = 0.f;
				fTemp[1] = m_pRotationCtrl[1]->GetCurSpeed();
				if( fTemp[1] <= m_pRotationCtrl[1]->GetMinSpeed() ) fTemp[1] = 0.f;
				fTemp[0] -= fTemp[1];

				m_fWheelAngle[0] -= ( fTemp[0] * 3.0f );
				m_fWheelAngle[1] -= ( fTemp[0] * 3.0f );

				m_fWheelAngle[2] += ( fTemp[0] * 3.0f );
				m_fWheelAngle[3] += ( fTemp[0] * 3.0f );
			}
			break;
	}
}

void CFcCatapultObject::ProcessMove()
{
	float fSpeed = 0.f; 
	float fCurSpeed = m_pVelocityCtrl->GetCurSpeed();
	

	D3DXVECTOR3 vPos = GetDummyPos() - m_MoveTargetPos;
	vPos.y = 0.f;

	float fLength = D3DXVec3Length( &vPos );

	float fDistance = 0.f;
	if( m_bApplyFlocking == false ) fDistance = 0.f;
	else {
		fDistance = m_fMaxFlockingSize;
	}
	
	if( fLength <= GetStopableDistance() + fDistance ) {
		m_fCurMaximumSpeed = 0.f;
		m_bStopFlag = true;
		if( m_nSoundMoveHandle != -1){		
#ifdef _XBOX
			g_pSoundManager->StartStopFade( m_nSoundMoveHandle );
#endif
			m_nSoundMoveHandle = -1;
		}
	}
	else {
		if( m_bStopFlag == false ) 
		{
			m_fCurMaximumSpeed = (float)m_pUnitInfoData->GetMoveSpeed( m_pUnitInfoData->GetAniIndex( ANI_TYPE_RUN, 0 ) );
			if( m_nRotationAniIndex != -1 )
				m_fCurMaximumSpeed = 0.f;

			if( m_nSoundMoveHandle == -1){			
				m_nSoundMoveHandle = g_pSoundManager->Play3DSound(this,SB_COMMON,"SLING_MOVE", &m_Cross.m_PosVector );
			}
			else{
				if( g_pSoundManager->IsOwner( this, m_nSoundMoveHandle ) ){				
					g_pSoundManager->SetEmitterPos( m_nSoundMoveHandle, &m_Cross.m_PosVector );
				}
			}
			
		}
		else {
			m_fCurMaximumSpeed = 0.f;
		}
	}
	if( fCurSpeed > m_fCurMaximumSpeed || m_bWorkable == false ) {
		fSpeed = m_pVelocityCtrl->GetSpeed( CFcVelocityCtrl::DECELERATION );
		if( fSpeed <= m_pVelocityCtrl->GetMinSpeed() ) {
			fSpeed = 0.f;
		}
	}
	else if( fCurSpeed < m_fCurMaximumSpeed && m_bWorkable == true ) {
		fSpeed = m_pVelocityCtrl->GetSpeed( CFcVelocityCtrl::ACCELERATION );
		if( fSpeed >= m_fCurMaximumSpeed ) {
			m_pVelocityCtrl->SetCurSpeed( m_fCurMaximumSpeed );
			fSpeed = m_fCurMaximumSpeed;
		}
	}
	else fSpeed = m_fCurMaximumSpeed;

	MoveZ( fSpeed );

	if( fSpeed == 0.f && m_nRotationAniIndex == -1 ) {
		if( m_pEventSeq->GetEventCount() == 0 ) {
			ChangeAnimation( ANI_TYPE_STAND, 0 );
			m_nBlendFrame = 0;
		}
	}
	else {
		if( m_bWorkable == true ) {
			if( m_nRotationAniIndex == -1 ) {
				ChangeAnimation( ANI_TYPE_RUN, 0 );
			}
			else {
				ChangeAnimation( ANI_TYPE_MOVE, m_nRotationAniIndex );
			}
		}
	}
}

void CFcCatapultObject::MoveZ(float fDist)
{
	m_MoveVector+=m_Cross.m_ZVector*fDist; 
}

void CFcCatapultObject::CmdMove( int nX, int nY, float fSpeed, int nAniType, int nAniIndex )
{
	if( m_bDead ) return;
	if( !IsMovable() )
	{
		return;
	}

	D3DXVECTOR3 vTemp = D3DXVECTOR3( (float)nX, 0.f, (float)nY );
	D3DXVECTOR3 Pos = m_Cross.GetPosition();

	m_MoveTargetPos = vTemp;
	m_MoveDir = vTemp - D3DXVECTOR3( Pos.x, 0.f, Pos.z );
	BsAssert( (m_MoveDir.x == m_MoveDir.x) && (m_MoveDir.y == m_MoveDir.y) && (m_MoveDir.z == m_MoveDir.z) );
	D3DXVec3Normalize(&m_MoveDir, &m_MoveDir);

	SetFlocking( false );
	m_bStopFlag = false;
}

void CFcCatapultObject::CmdStop( int nAniType, int nAniIndex )
{
	m_bStopFlag = true;
	if( D3DXVec3LengthSq( &m_MoveDir ) > 0.f )
		m_MoveDir = D3DXVECTOR3( 0.f, 0.f, 0.f );

}

void CFcCatapultObject::RotationProcess()
{
	float fLength;
	D3DXVECTOR3 CrossVec;

//	if( m_pVelocityCtrl->GetCurSpeed() > m_pVelocityCtrl->GetMinSpeed() ) return;
	//////////////////////////////////////////////////
	float fAngle = 0.f;
	if( m_bWorkable == false ) m_nRotationAniIndex = -1;
	switch( m_nRotationAniIndex ) {
		case -1:
			m_pRotationCtrl[0]->GetSpeed( CFcVelocityCtrl::DECELERATION );
			m_pRotationCtrl[1]->GetSpeed( CFcVelocityCtrl::DECELERATION );
			break;
		case 0: 
			m_pRotationCtrl[0]->GetSpeed( CFcVelocityCtrl::ACCELERATION );	
			m_pRotationCtrl[1]->GetSpeed( CFcVelocityCtrl::DECELERATION );
			break;
		case 1: 
			m_pRotationCtrl[0]->GetSpeed( CFcVelocityCtrl::DECELERATION );
			m_pRotationCtrl[1]->GetSpeed( CFcVelocityCtrl::ACCELERATION );	
			break;
	}
	float fTemp[2];
	fTemp[0] = m_pRotationCtrl[0]->GetCurSpeed();
	if( fTemp[0] <= m_pRotationCtrl[0]->GetMinSpeed() ) fTemp[0] = 0.f;
	fTemp[1] = m_pRotationCtrl[1]->GetCurSpeed();
	if( fTemp[1] <= m_pRotationCtrl[1]->GetMinSpeed() ) fTemp[1] = 0.f;
	fAngle = fTemp[0] - fTemp[1];

	if( fAngle != 0.f ) {
		D3DXVECTOR3 Rotate;
		float fTemp = fAngle * 3.1415926f / 180.f;
		Rotate.x=-sin(fTemp)*m_Cross.m_XVector.x+cos(fTemp)*m_Cross.m_ZVector.x;
		Rotate.y=-sin(fTemp)*m_Cross.m_XVector.y+cos(fTemp)*m_Cross.m_ZVector.y;
		Rotate.z=-sin(fTemp)*m_Cross.m_XVector.z+cos(fTemp)*m_Cross.m_ZVector.z;
		m_Cross.m_ZVector=Rotate;
		D3DXVec3Normalize(&m_Cross.m_ZVector, &m_Cross.m_ZVector);
		D3DXVec3Cross(&m_Cross.m_XVector, &m_Cross.m_YVector, &m_Cross.m_ZVector);
		D3DXVec3Normalize(&m_Cross.m_XVector, &m_Cross.m_XVector);
	}
	//////////////////////////////////////////////////


	fLength=D3DXVec3Length(&m_MoveDir);
	if(fLength<=0.0f){
		float fMaximumSpeed = (float)m_pUnitInfoData->GetMoveSpeed( m_pUnitInfoData->GetAniIndex( ANI_TYPE_RUN, 0 ) );
		m_nRotationAniIndex = -1;
		return;
	}

//	/*
	fLength = D3DXVec3Length( &m_MoveTargetPos );
	if( fLength > 0.f ) {
		D3DXVECTOR3 vTargetVec = m_MoveTargetPos - GetDummyPos();
		vTargetVec.y = 0.f;
		D3DXVec3Normalize( &vTargetVec, &vTargetVec );
		m_MoveDir = vTargetVec;
		BsAssert( (m_MoveDir.x == m_MoveDir.x) && (m_MoveDir.y == m_MoveDir.y) && (m_MoveDir.z == m_MoveDir.z) );
	}
//	*/

	float fDot=D3DXVec3Dot(&m_MoveDir, &(m_Cross.m_ZVector));
	fAngle = acos( fDot ) * 180.f / 3.1415926f;

	if( fAngle <= fabs( GetStopableAngle() ) ) {
		m_nRotationAniIndex = -1;
		if( fAngle < s_fRotationAngle ) {
			m_Cross.m_ZVector=m_MoveDir;
			m_MoveDir=D3DXVECTOR3(0.0f, 0.0f, 0.0f);
			m_Cross.UpdateVectors();

			m_nRotationAniIndex = -1;
		}
		return;
	}
	else {
		if( fAngle > s_fRotationAngle ) {
			D3DXVec3Cross(&CrossVec, &m_MoveDir, &(m_Cross.m_ZVector));
			if(CrossVec.y>0){
				m_nRotationAniIndex = 0;
			}
			else{
				m_nRotationAniIndex = 1;
			}
			fDot=D3DXVec3Dot(&m_MoveDir, &(m_Cross.m_ZVector));
			fAngle = acos( fDot ) * 180.f / 3.1415926f;
			if( fAngle < s_fRotationAngle ) {
				m_Cross.m_ZVector=m_MoveDir;
				m_MoveDir=D3DXVECTOR3(0.0f, 0.0f, 0.0f);
				m_Cross.UpdateVectors();

				m_nRotationAniIndex = -1;
			}
			return;
		}
	}

	m_nRotationAniIndex = -1;

	m_Cross.m_ZVector=m_MoveDir;
	m_Cross.UpdateVectors();
	m_MoveDir=D3DXVECTOR3(0.0f, 0.0f, 0.0f);
}

bool CFcCatapultObject::IsHittable( GameObjHandle Handle )
{
	if( m_bDead == true ) return false;
//	if( m_bDieFlag == true ) return false;
	return true;
}

void CFcCatapultObject::CmdAttack( GameObjHandle Handle )
{
	if( m_bWorkable == true && GetParentTroop()->GetState() == TROOPSTATE_RANGE_ATTACK && m_pEventSeq->GetEventCount() == 0 ) {
		CFcCatapultSeqAttackEvent *pElement = new CFcCatapultSeqAttackEvent( m_pEventSeq );

		pElement->m_vTargetPos = GetParentTroop()->GetTargetTroop()->GetPos();

		m_pEventSeq->AddEvent( pElement );
		// Projectile Targeting 땜에 가지구있는다.. 여기서 가지구 있어야 쏠때 어색하지 않타!!
		m_vTargetPos = pElement->m_vTargetPos;
	}
}

void CFcCatapultObject::ProcessFlocking()
{
//	if( m_bDead ) return;

	m_Collision.C = *(BSVECTOR*)&m_Cross.m_PosVector;
	m_Collision.A[0] = *(BSVECTOR*)&m_Cross.m_XVector;
	m_Collision.A[1] = *(BSVECTOR*)&m_Cross.m_YVector;
	m_Collision.A[2] = *(BSVECTOR*)&m_Cross.m_ZVector;
	m_Collision.E[0] = 250.f + 50.f;
	m_Collision.E[1] = 2000.f;
	m_Collision.E[2] = GetUnitRadius() + 50.f;

	m_VecFlocking.erase( m_VecFlocking.begin(), m_VecFlocking.end() );
	CFcWorld::GetInstance().GetObjectListInRange( (D3DXVECTOR3*)&GetPos(), 1000.f, m_VecFlocking );

	D3DXVECTOR3 vVec[5];
	float fTempLength, fLength;
	D3DXVECTOR3 vTempResult, vResult;
	D3DXVECTOR3 vCross, vTemp, vNor;
	int nCheckIndex = -1;

	m_Collision.compute_vertices();

	memcpy( &vVec[0].x, &m_Collision.V[0].x, sizeof(D3DXVECTOR3) );
	memcpy( &vVec[1].x, &m_Collision.V[1].x, sizeof(D3DXVECTOR3) );
	memcpy( &vVec[2].x, &m_Collision.V[5].x, sizeof(D3DXVECTOR3) );
	memcpy( &vVec[3].x, &m_Collision.V[4].x, sizeof(D3DXVECTOR3) );
	memcpy( &vVec[4].x, &m_Collision.V[0].x, sizeof(D3DXVECTOR3) );


	for( int i = 0; i <(int)m_VecFlocking.size(); i++ ) {
		if( !CFcGameObject::IsValid( m_VecFlocking[i] ) ) continue;
		if( m_VecFlocking[i] == m_Handle ) continue;
		if( !m_VecFlocking[i]->IsFlockable( m_Handle ) ) continue;
		if( std::find( m_VecLinkObjHandle.begin(), m_VecLinkObjHandle.end(), m_VecFlocking[i] ) != m_VecLinkObjHandle.end() ) continue;

		if( TestIntersection( m_VecFlocking[i]->GetPos(), m_Collision ) == false ) continue;

		bool bFirst = true;
		vTemp = m_VecFlocking[i]->GetPos();
		vTemp.y = 0.f;
		for( int j=0; j<4; j++ ) {
			vNor = vVec[j] - vVec[j+1];
			vNor.y = 0.f;
			D3DXVec3Normalize( &vNor, &vNor );
			D3DXVec3Cross( &vCross, &vNor, &D3DXVECTOR3( 0.f, 1.f, 0.f ) );
			D3DXVec3Normalize( &vCross, &vCross );

			vCross = vTemp + ( vCross * 1000.f );
			if( GetIntersectPoint( vVec[j], vVec[j+1], vTemp, vCross, vTempResult ) )  {
				vTempResult.y = 0.f;
				fTempLength = D3DXVec3Length( &( vTemp - vTempResult ) );
				if( bFirst == true || fLength > fTempLength ) {
					bFirst = false;
					fLength = fTempLength;
					vResult = vTempResult;
					nCheckIndex = j;
				}
			}
		}
		if( bFirst == false ) {
			if( fLength > 300.f ) continue;
			m_VecFlocking[i]->Move( &( vResult - vTemp ) );

			switch( m_VecFlocking[i]->GetClassID() ) {
				case Class_ID_Unit:
					{
						D3DXVECTOR3 vTempNor[2];
						vTempNor[0] = vVec[nCheckIndex] - *m_VecFlocking[i]->GetMoveTargetPos();
						vTempNor[1] = vVec[nCheckIndex+1] - *m_VecFlocking[i]->GetMoveTargetPos();
						if( D3DXVec3LengthSq( &vTempNor[0] ) < D3DXVec3LengthSq( &vTempNor[1] ) )
							vTemp = vVec[nCheckIndex] - m_VecFlocking[i]->GetPos();
						else vTemp = vVec[nCheckIndex+1] - m_VecFlocking[i]->GetPos();

						vTemp.y = 0.f;
						D3DXVec3Normalize( &vTemp, &vTemp );
						((CFcUnitObject*)m_VecFlocking[i].GetPointer())->LockShortPathFinder( vTemp, 80 );
					}
					break;
				default:
					break;
			}
//			m_VecFlocking[i]->SetFlocking( true, GetUnitRadius() + m_VecFlocking[i]->GetUnitRadius() );
		}
	}
}

float CFcCatapultObject::GetStopableDistance()
{
	float fDistance = 0.f;
	float fTemp = m_pVelocityCtrl->GetCurSpeed();
	while(1) {
		fTemp *= m_pVelocityCtrl->GetDeceleration();
		fDistance += fTemp;
		if( fTemp < m_pVelocityCtrl->GetMinSpeed() ) break;
	}
	return fDistance;
}

float CFcCatapultObject::GetStopableAngle()
{
	float fAngle = 0.f;
	float fTemp[2] = { m_pRotationCtrl[0]->GetCurSpeed(), m_pRotationCtrl[1]->GetCurSpeed() };

	while(1) {
		fTemp[0] *= m_pRotationCtrl[0]->GetDeceleration();
		if( fTemp[0] <= m_pRotationCtrl[0]->GetMinSpeed() ) fTemp[0] = 0.f;
		fTemp[1] *= m_pRotationCtrl[1]->GetDeceleration();
		if( fTemp[1] <= m_pRotationCtrl[1]->GetMinSpeed() ) fTemp[1] = 0.f;

		fAngle += fTemp[0] - fTemp[1];
		if( fTemp[0] == 0.f && fTemp[1] == 0.f ) break;
		if( fAngle == 0.f ) break;
	}
	return fAngle;
}

bool CFcCatapultObject::LinkMe( GameObjHandle Handle, int nPlayerIndex /*= -1*/ )
{
	for( DWORD i=0; i<m_VecLinkObjHandle.size(); i++ ) {
		if( m_VecLinkObjHandle[i] == Handle ) return false;
	}

	if( m_VecLinkObjHandle.size() > 3 ) return false;
	if( Handle->GetParentLinkHandle() ) return false;

	Handle->SetParentLinkHandle( m_Handle );
	Handle->SetAniPosToLocalPosFlag( BS_CALC_POSITION_X | BS_CALC_POSITION_Y | BS_CALC_POSITION_Z );
	char *szBoneName = s_LinkBoneName[ m_VecLinkObjHandle.size() ].szBoneName;
	g_BsKernel.SendMessage( m_nEngineIndex, BS_LINKOBJECT_NAME, ( DWORD )szBoneName, Handle->GetEngineIndex() );
	g_BsKernel.ShowObject(Handle->GetEngineIndex(), true);

	m_nVecLinkBoneIndex.push_back( (int)m_VecLinkObjHandle.size() );
	m_VecLinkObjHandle.push_back( Handle );
//	Handle->SetAniPosToLocalPosFlag( BS_CALC_POSITION_X | BS_CALC_POSITION_Y | BS_CALC_POSITION_Z );

	if( m_VecLinkObjHandle.size() > 0 ) {
		if( m_hUnitAIHandle )
			m_hUnitAIHandle->SetEnable( true );
	}

	return true;
}

bool CFcCatapultObject::UnLinkMe( GameObjHandle Handle )
{
	int nIndex = -1;
	for( DWORD i=0; i<m_VecLinkObjHandle.size(); i++ ) {
		if( m_VecLinkObjHandle[i] == Handle ) {
			nIndex = (int)i;
			break;
		}
	}
	if( nIndex == -1 ) return false;

	D3DXMATRIX *pRetMat;
	D3DXVECTOR3 vPos;
	pRetMat = (D3DXMATRIX *)g_BsKernel.SendMessage( m_nEngineIndex, BS_UNLINKOBJECT, Handle->GetEngineIndex() );

	if( pRetMat ) {
		memcpy( &vPos.x, &pRetMat->_41, sizeof( D3DXVECTOR3 ) );
		vPos -= Handle->GetPos();
		*Handle->GetMoveVector() += vPos;
	}

	Handle->SetAniPosToLocalPosFlag( BS_CALC_POSITION_Y );
	if( !Handle->IsDie() && Handle->GetHP() > 0 ) {
		Handle->ChangeAnimation( ANI_TYPE_CUSTOM_0, 0, ANI_ATTR_NONE );
	}

	GameObjHandle hIdentity;
	m_nVecLinkBoneIndex.erase( m_nVecLinkBoneIndex.begin() + nIndex );
	m_VecLinkObjHandle.erase( m_VecLinkObjHandle.begin() + nIndex );

	if( m_VecLinkObjHandle.size() == 0 ) {
		if( m_hUnitAIHandle )
			m_hUnitAIHandle->SetEnable( false );
	}

	CmdStop();
	return true;
}


void CFcCatapultObject::CheckHitLinkObject( GameObjHandle Handle, int &nAniAttr, int &nAniType, int &nAniTypeIndex )
{
	if( Handle->GetHP() <= 0 ) {
		Handle->RideOut();

		Handle->SetAniFrame( ANI_TYPE_STAND, 0, 0 );
		((CFcUnitJobChange*)Handle.GetPointer())->ChangeJob( true );
		Handle->ChangeAnimation( ANI_TYPE_DIE, 0, ANI_ATTR_NONE, true );
		return;
	}

	nAniAttr = 0;
	for( DWORD i=0; i<m_VecLinkObjHandle.size(); i++ ) {
		if( m_VecLinkObjHandle[i] == Handle ) {
			nAniTypeIndex = s_LinkBoneName[ m_nVecLinkBoneIndex[i] ].nWalkAniIndex;
			break;
		}
	}
	nAniType = ANI_TYPE_HIT;
}

void CFcCatapultObject::DebugRender()
{
#ifndef _XBOX
	m_Collision.compute_vertices();
	C3DDevice* pDevice = g_BsKernel.GetDevice();
	pDevice->SaveState();
	pDevice->BeginScene();

	D3DXMATRIX mat, matWorld;
	pDevice->GetTransform( D3DTS_WORLD, &matWorld );
	D3DXMatrixIdentity( &mat );
	pDevice->SetTransform( D3DTS_WORLD, &mat );

	pDevice->SetPixelShader(NULL);
	pDevice->SetVertexShader(NULL);
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, false);
	pDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
	for( DWORD i=0; i<8; i++ ) pDevice->SetTexture( i, NULL );
	pDevice->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );


	struct LINE_VERTEX
	{
		D3DXVECTOR3 v;
		DWORD       color;
	};
	static LINE_VERTEX pVertices[8];

	LINE_VERTEX vVec[8];
	vVec[0].v = m_Collision.V[0];
	vVec[1].v = m_Collision.V[1];
	vVec[0].color = 0xFFFFFFFF;
	vVec[1].color = 0xFF0000FF;

	vVec[2].v = m_Collision.V[1];
	vVec[3].v = m_Collision.V[5];
	vVec[2].color = 0xFFFFFFFF;
	vVec[3].color = 0xFF0000FF;

	vVec[4].v = m_Collision.V[5];
	vVec[5].v = m_Collision.V[4];
	vVec[4].color = 0xFFFFFFFF;
	vVec[5].color = 0xFF0000FF;

	vVec[6].v = m_Collision.V[4];
	vVec[7].v = m_Collision.V[0];
	vVec[6].color = 0xFFFFFFFF;
	vVec[7].color = 0xFF0000FF;
	for( int i=0; i<8; i++ ) {
		vVec[i].v.y = 10.f;
	}

	pDevice->DrawPrimitiveUP( D3DPT_LINELIST, 4, vVec, sizeof(LINE_VERTEX) );

	pDevice->SetTransform( D3DTS_WORLD, &matWorld );
	pDevice->EndScene();
	pDevice->RestoreState();
#endif //_XBOX
}


bool CFcCatapultObject::IsFlockable( GameObjHandle Handle )
{
	if( m_Handle == Handle ) return false;
	switch( Handle->GetClassID() ) {
		case Class_ID_Catapult: return true;
		case Class_ID_MoveTower: return true;	
		default:
			break;
	}
	return false;
}

int CFcCatapultObject::GetLinkObjAniIndex( int nIndex )
{
	return m_nVecLinkBoneIndex[nIndex];
}

void CFcCatapultObject::Break()
{
	m_nBreakObjectIndex = g_BsKernel.CreateClothObjectFromSkin( m_nBreakableSkinIndex, m_pBreakableInfo , m_Cross);
	D3DXVECTOR3 Force(1,3,1);
	g_BsKernel.SendMessage(m_nBreakObjectIndex, BS_PHYSICS_ADDFORCE, (DWORD)&Force);
	g_pSoundManager->Play3DSound(NULL,SB_COMMON,"OB_WOOD_FALL_LOW", &m_Cross.m_PosVector );
	//g_BsKernel.SendMessage( m_nBreakObjectIndex, BS_PHYSICS_LINK_CHARACTER, m_nEngineIndex);
	//g_BsKernel.SendMessage( m_nEngineIndex, BS_LINKOBJECT, 0, m_nBreakObjectIndex);
	//g_BsKernel.ShowObject( m_nEngineIndex, false);
}

void CFcCatapultObject::SignalCheck( ASSignalData *pSignal )
{
	switch( pSignal->m_nID ) {
		case SIGNAL_TYPE_SHOOT_PROJECTILE:
			if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
			{
				PROJECTILE_PARAM Param;
				ProjetileHandle Handle;
				ASPartsData *pPartsData;
				CCrossVector Cross;
				D3DXVECTOR3 Position;

				pPartsData = m_pUnitInfoData->GetPartsGroupInfo( m_WeaponList[ 0 ].nPartsIndex );
				BsAssert( pPartsData->m_pSkinIndex[0] >= 0 );
				Cross = m_Cross;
				Position.x = ( float )pSignal->m_pParam[ 1 ];
				Position.y = ( float )pSignal->m_pParam[ 2 ];
				Position.z = ( float )pSignal->m_pParam[ 3 ];
				D3DXVec3TransformNormal( &Position, &Position, m_Cross );
				Cross.m_PosVector += Position;

				// 성벽, 부대 분기
				TroopObjHandle hTroop = m_hTroop->GetTargetTroop();
				if( hTroop && hTroop->GetType() == TROOPTYPE_WALL_DUMMY ) {
					CFcWallDummyTroop *pDummyTroop = (CFcWallDummyTroop *)hTroop.GetPointer();
					CFcProp *pProp = pDummyTroop->GetParentProp();

					Box3 Box;
					pProp->GetBox3( Box );
					
					D3DXVECTOR3 vTarget, vWidth, vHeight, vAxis[2];
					memcpy( &vTarget, &Box.C, sizeof(D3DXVECTOR3) );
					memcpy( &vAxis[0], &Box.A[0], sizeof(D3DXVECTOR3) );
					memcpy( &vAxis[1], &Box.A[1], sizeof(D3DXVECTOR3) );

					// 가로 
					vWidth = ( vAxis[0] * ( -Box.E[0] * 0.4f ) ) + ( vAxis[0] * ( Box.E[0] * RandomNumberInRange( 0.f,0.8f ) ) );
					// 새로
					vHeight = ( vAxis[1] * ( -Box.E[1] * 0.3f ) ) + ( vAxis[1] * ( Box.E[1] * RandomNumberInRange( 0.f,0.2f ) ) );
					vTarget += vWidth + vHeight;

					Param.vTarget = vTarget;
					Param.fAngle = 40.f;
				}
				else {
					// Angle 대애~~충~ 구하자..
					D3DXVECTOR3 vTemp1, vTemp2;
					D3DXVECTOR3 vVec1, vVec2;
					D3DXVECTOR3 vTarget = m_vTargetPos;

					vTemp1 = Position;	vTemp1.y = 0.f;
					vTemp2 = vTarget;	vTemp2.y = 0.f;
					vVec1 = vTemp1 - vTemp2;

					vTemp1 = Position;	vTemp1.x = vTemp1.z = 0.f;
					vTemp2 = vTarget;	vTemp2.x = vTemp2.z = 0.f;
					vVec2 = vTemp1 - vTemp2;
					float fHeight = Position.y - vTarget.y;

					D3DXVECTOR3 vResult = vVec1 + vVec2;
					D3DXVec3Normalize( &vResult, &vResult );
					D3DXVec3Normalize( &vVec1, &vVec1 );
					float fDot = D3DXVec3Dot( &vResult, &vVec1 );
					if( fDot >= 1.f ) fDot = 1.f;
					if( fDot <= -1.f ) fDot = -1.f;
					float fAngle = acos(fDot) * 180.f / 3.1415926f;

					if( fHeight <= 0.f ) {
						fAngle += 20.f;
					}
					else {
						fAngle = -fAngle + 20.f;
					}

					Param.vTarget = m_vTargetPos;
					Param.fAngle = fAngle;

				}

				Param.hParent = m_Handle;
				Param.pSignal = pSignal;
    			Handle = CFcProjectile::CreateObject( CFcProjectile::STONE, &Cross );
    			Handle->Initialize( pPartsData->m_pSkinIndex[0], &Param );
			}
			return;
	}
	CFcUnitObject::SignalCheck( pSignal );
}

bool CFcCatapultObject::GetExactHitAni( int &nAniType, int &nAniIndex )
{
	return false;
}

void CFcCatapultObject::CalculateParticlePosition( CCrossVector &Cross )
{
	CFcGameObject::CalculateParticlePosition( Cross );
	Cross.MoveFrontBack( 250.f );
}
