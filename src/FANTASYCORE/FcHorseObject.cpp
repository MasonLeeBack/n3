#include "StdAfx.h"
#include "BsKernel.h"
#include "InputPad.h"
#include "ASData.h"
#include "FcBaseObject.h"
#include "FcCameraObject.h"
#include "FCHorseObject.h"
#include "FcTroopObject.h"
#include "FcGlobal.h"
#include "FcWorld.h"
#include "FcVelocityCtrl.h"
#include "DebugUtil.h"
#include "FcHeroObject.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


int CFcHorseObject::s_nRotationAngle = 5;
int CFcHorseObject::s_nMaxRollAngle = 30;
int CFcHorseObject::s_nRollValue = 3;

CFcHorseObject::CFcHorseObject( CCrossVector *pCross )
	: CFcUnitObject( pCross )
{
	SetClassID( Class_ID_Horse );
	m_nLinkPlayerIndex = -1;	

	m_nPrevRollAngle = 0;
	m_nRollAngle = 0;
	m_bStopFlag = false;

	m_nRotationAniType = -1;
	m_fPrevRotationAngle = 0.f;

	// 감속,가속
	m_pVelocityCtrl = new CFcVelocityCtrl( 1.5f, 20.f, 1.f, 1.10f, 0.93f );
	m_fCurMaximumSpeed = 0.f;
}

CFcHorseObject::~CFcHorseObject(void)
{
	SAFE_DELETE( m_pVelocityCtrl );
}


void CFcHorseObject::Initialize( INITGAMEOBJECTDATA* pData, int nForce, int nTeam, TroopObjHandle hTroop, float fScaleMin, float fScaleMax )
{
	CFcGameObject::Initialize( pData, nForce, -1, hTroop, fScaleMin, fScaleMax );
	float fMaximumSpeed = (float)m_pUnitInfoData->GetMoveSpeed( m_pUnitInfoData->GetAniIndex( ANI_TYPE_RUN, 0 ) );
	m_pVelocityCtrl->SetMaximumSpeed( fMaximumSpeed );
}

void CFcHorseObject::Process()
{
	CFcGameObject::Process();

	if( m_nCurAniType == ANI_TYPE_RUN ) m_nRunFrame++;
	else m_nRunFrame = 0;

	ProcessChildAnimation();
	ProcessMove();

	if( !m_VecLinkObjHandle.empty() ) {
		CCrossVector *pCross = m_VecLinkObjHandle[0]->GetCrossVector();
		*pCross = m_Cross;
	}
}

bool CFcHorseObject::Render()
{
	return CFcUnitObject::Render();
}

void CFcHorseObject::PositionProcess()
{
	CFcGameObject::PositionProcess();
}

bool CFcHorseObject::LinkMe( GameObjHandle Handle, int nPlayerIndex /*= -1*/ )
{
	for( DWORD i=0; i<m_VecLinkObjHandle.size(); i++ ) {
		if( m_VecLinkObjHandle[i] == Handle ) return false;
	}

	g_BsKernel.SendMessage( m_nEngineIndex, BS_LINKOBJECT_NAME, ( DWORD )"Dummy_병사", Handle->GetEngineIndex() );
	m_VecLinkObjHandle.push_back( Handle );
	m_nLinkPlayerIndex = nPlayerIndex;
	Handle->SetAniPosToLocalPosFlag( BS_CALC_POSITION_X | BS_CALC_POSITION_Y | BS_CALC_POSITION_Z );
	if( m_nLinkPlayerIndex != -1 )
	{
		CInputPad::GetInstance().AttachObject( m_nLinkPlayerIndex, m_Handle );
		CameraObjHandle CamHandle;

		CInputPad::GetInstance().AttachObject( m_nLinkPlayerIndex, m_Handle );
		CamHandle = CFcBaseObject::GetCameraObjectHandle( m_nLinkPlayerIndex );
		CamHandle->AttachObject( m_Handle );
		m_bUsePathFind = false;
	}
	g_BsKernel.SetPreCalcAni( m_nEngineIndex, true);

	return true;
}

bool CFcHorseObject::UnLinkMe( GameObjHandle Handle )
{
	int nIndex = -1;
	for( DWORD i=0; i<m_VecLinkObjHandle.size(); i++ ) {
		if( m_VecLinkObjHandle[i] == Handle ) {
			nIndex = (int)i;
			break;
		}
	}
	if( nIndex == -1 ) return false;

	if( m_nLinkPlayerIndex != -1 )
	{
		CameraObjHandle CamHandle;

		CInputPad::GetInstance().AttachObject( m_nLinkPlayerIndex, Handle );
		CamHandle = CFcBaseObject::GetCameraObjectHandle( m_nLinkPlayerIndex );
		CamHandle->AttachObject( Handle );
		m_bUsePathFind = true;
	}
	g_BsKernel.SendMessage( m_nEngineIndex, BS_UNLINKOBJECT, Handle->GetEngineIndex() );
	Handle->SetAniPosToLocalPosFlag( BS_CALC_POSITION_Y );
	m_VecLinkObjHandle.erase( m_VecLinkObjHandle.begin() + nIndex );
//	m_LinkObjHandle.Identity();
	CmdStop();
	m_MoveTargetPos = D3DXVECTOR3( 0.f, 0.f, 0.f );
//	ChangeAnimation( ANI_TYPE_STAND, 0 );
//	SetRefreshAni (false);
	return true;
}

void CFcHorseObject::ProcessKeyEvent(int nKeyCode, KEY_EVENT_PARAM *pKeyParam)
{
	if( nKeyCode == PAD_INPUT_EVENT_RESTORE_KEY ) {
		if( !m_VecLinkObjHandle.empty() && m_VecLinkObjHandle[0]->GetClassID() == Class_ID_Hero ) {
			m_MoveTargetPos = D3DXVECTOR3( 0.f, 0.f, 0.f );
			m_fCurMaximumSpeed = 0.f;
		}
		return;
	}

	bool bRelease=false;

	if(nKeyCode&0x80000000){
		bRelease=true;
	}
	nKeyCode&=0x7fffffff;

	if(nKeyCode==PAD_INPUT_LSTICK){
		if( IsMovable() || m_nCurAniType == ANI_TYPE_CUSTOM_0 )
		{
			if(bRelease){
				m_fCurMaximumSpeed = 0.f;
			}
			else{
				D3DXVECTOR2 vVec = D3DXVECTOR2( (float)pKeyParam->nPosX, (float)pKeyParam->nPosY );
				float fLength = D3DXVec2Length( &vVec );
				int nAniType;
				if( fLength < 25000.f ) {
					nAniType = ANI_TYPE_WALK;
				}
				else {
					nAniType = ANI_TYPE_RUN;
				}

				CalculateMoveDir( pKeyParam, m_nLinkPlayerIndex );

				m_fCurMaximumSpeed = (float)m_pUnitInfoData->GetMoveSpeed( m_pUnitInfoData->GetAniIndex( nAniType, 0 ) );
			}
		}
		else if( IsJump() ) {
			if(bRelease){
				m_fCurMaximumSpeed = 0.f;
			}
		}
	}
	else if( !bRelease )
	{
		m_VecLinkObjHandle[0]->ProcessKeyEvent(nKeyCode, pKeyParam);
	}
}

void CFcHorseObject::ProcessRoll()
{
	if( m_nRollAngle != 0 ) {
		int nRollValue = s_nRollValue;
//		if( m_nCurAniType == ANI_TYPE_RUN ) nRollValue = s_nRollValue;
//		else nRollValue = s_nRollValue * 2;

		if( m_nRollAngle > 0 ) {
			m_nRollAngle -= nRollValue;
			if( m_nRollAngle < 0 ) m_nRollAngle = 0;
		}
		else if( m_nRollAngle < 0 ) {
			m_nRollAngle += nRollValue;
			if( m_nRollAngle > 0 ) m_nRollAngle = 0;
		}
	}
	m_Cross.RotateRoll( m_nRollAngle );
	m_nPrevRollAngle = -m_nRollAngle;
}

void CFcHorseObject::RotationProcess()
{
//	if( m_nCurAniType == ANI_TYPE_JUMP_UP ) return;

	float fLength;
	D3DXVECTOR3 CrossVec;

	m_Cross.RotateRoll( m_nPrevRollAngle );

	fLength=D3DXVec3Length(&m_MoveDir);
	if(fLength<=0.0f){
		m_nRotationAniType = -1;

		ProcessRoll();
		return;
	}

	fLength=D3DXVec3Length(&m_MoveVector);
	if( fLength <= 0.0f ) {
		m_MoveDir = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		m_nRotationAniType = -1;

		ProcessRoll();
		return;
	}

	fLength = D3DXVec3Length( &m_MoveTargetPos );
	if( fLength > 0.f ) {
		D3DXVECTOR3 vTargetVec = m_MoveTargetPos - GetDummyPos();
		vTargetVec.y = 0.f;
		D3DXVec3Normalize( &vTargetVec, &vTargetVec );
		m_MoveDir = vTargetVec;
	}

	float fDot=D3DXVec3Dot(&m_MoveDir, &(m_Cross.m_ZVector));
	float fAngle = acos( fDot ) * 180.f / 3.1415926f;

	float fMinAngle = s_nRotationAngle / 2.84f;
	if( fAngle > fMinAngle ){
		D3DXVec3Cross(&CrossVec, &m_MoveDir, &(m_Cross.m_ZVector));
		if(CrossVec.y>0){
			m_Cross.RotateYaw( s_nRotationAngle );

			if( m_nCurAniType == ANI_TYPE_RUN ) {
				m_nRollAngle -= s_nRollValue;
				if( m_nRollAngle < -s_nMaxRollAngle ) m_nRollAngle = -s_nMaxRollAngle;
			}
			if( fabs( fAngle - m_fPrevRotationAngle ) > fMinAngle * 4.f )
				m_nRotationAniType = 0;

		}
		else{
			m_Cross.RotateYaw( -s_nRotationAngle );

			if( m_nCurAniType == ANI_TYPE_RUN ) {
				m_nRollAngle += s_nRollValue;
				if( m_nRollAngle > s_nMaxRollAngle ) m_nRollAngle = s_nMaxRollAngle;
			}
			if( fabs( fAngle - m_fPrevRotationAngle ) > fMinAngle * 4.f )
				m_nRotationAniType = 1;

		}
		fDot=D3DXVec3Dot(&m_MoveDir, &(m_Cross.m_ZVector));
		fAngle = acos( fDot ) * 180.f / 3.1415926f;
		if( fAngle < fMinAngle ) {
			m_Cross.m_ZVector=m_MoveDir;
			m_Cross.UpdateVectors();
		}	
		m_fPrevRotationAngle = fAngle;

		if( m_nCurAniType == ANI_TYPE_RUN )
			m_Cross.RotateRoll( m_nRollAngle );
		else ProcessRoll();
		m_nPrevRollAngle = -m_nRollAngle;
		return;
	}

	m_Cross.m_ZVector=m_MoveDir;
	m_Cross.UpdateVectors();
	m_MoveDir=D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	m_nRotationAniType = -1;
	ProcessRoll();
}

void CFcHorseObject::MoveZ(float fDist)
{
	m_MoveVector+=m_Cross.m_ZVector*fDist; 
}

void CFcHorseObject::CmdAttack( GameObjHandle Handle )
{
	if( !Handle ) return;
	if( m_VecLinkObjHandle.empty() ) return;
	D3DXVECTOR3 vVec, vUp, vResult, vCross;
	float fLength;

	vUp = D3DXVECTOR3( 0.f, 1.f, 0.f );
	vVec = GetDummyPos() - Handle->GetDummyPos();
	vVec.y = 0.f;
	fLength = D3DXVec3Length( &vVec );
	D3DXVec3Normalize( &vVec, &vVec );

	D3DXVec3Cross( &vCross, &vVec, &vUp );

	vResult = Handle->GetDummyPos() + ( -vCross * 80.f );
	vVec = vResult - GetDummyPos();
	vVec.y = 0.f;
	D3DXVec3Normalize( &vVec, &vVec );

	vResult += ( vVec * ( fLength + 200.f ) );

	CmdMove( (int)vResult.x, (int)vResult.z );
}



bool CFcHorseObject::GetCollisionPropHeight( float fX, float fY, float* pHeight)
{
	TroopObjHandle hTroop;
	if( !m_VecLinkObjHandle.empty() )
	{
		hTroop = m_VecLinkObjHandle[0]->GetTroop();
	}
	else {
		hTroop = m_hTroop;
		BsAssert( m_hTroop );
	}

	return hTroop->GetCollisionPropHeight( fX, fY, pHeight );
}

void CFcHorseObject::ProcessChildAnimation()
{
	if( !m_VecLinkObjHandle.empty() ) {
		int nAniAttr = GetCurAniAttr();
		int nAniType = GetCurAniType() & 0x00FF;

		switch( nAniType ) {
			case ANI_TYPE_RUN:
			case ANI_TYPE_WALK:
				if( m_VecLinkObjHandle[0]->IsMovable() )
				{
					m_VecLinkObjHandle[0]->ChangeAnimation( nAniType, 0, ANI_ATTR_HORSE );
				}
				break;
			case ANI_TYPE_STAND:
				if( m_VecLinkObjHandle[0]->GetCurAniType() == ( ANI_ATTR_HORSE | ANI_TYPE_RUN ) ||
					m_VecLinkObjHandle[0]->GetCurAniType() == ( ANI_ATTR_HORSE | ANI_TYPE_WALK ) )
					m_VecLinkObjHandle[0]->ChangeAnimation( nAniType, 0, ANI_ATTR_HORSE );
				break;
			case ANI_TYPE_CUSTOM_0:
				break;
		}
	}
}

void CFcHorseObject::CheckMoveLock( TroopObjHandle hTroop )
{
	if( !m_VecLinkObjHandle.empty() )
		CFcGameObject::CheckMoveLock( m_VecLinkObjHandle[0]->GetTroop() );
}

float CFcHorseObject::GetStopableDistance()
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

void CFcHorseObject::ProcessMove()
{
	if( m_nCurAniType == ANI_TYPE_JUMP_UP ) return;

	float fCurSpeed = m_pVelocityCtrl->GetCurSpeed();

	float fSpeed = 0.f;
	int nAniType = -1;

	if( D3DXVec3LengthSq( &m_MoveTargetPos ) > 0.f ) {
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
		}
		else {
			if( m_nCurAniType == ANI_TYPE_STAND ) m_fCurMaximumSpeed = 0.f;
			else {
				if( m_bStopFlag == false )
					m_fCurMaximumSpeed = (float)m_pUnitInfoData->GetMoveSpeed( m_nAniIndex );
			}
		}
	}

	m_fFrameAdd = 1.f;
	if( fCurSpeed > m_fCurMaximumSpeed ) {
		fSpeed = m_pVelocityCtrl->GetSpeed( CFcVelocityCtrl::DECELERATION );
		if( fSpeed <= m_pVelocityCtrl->GetMinSpeed() ) {
			fSpeed = 0.f;
			nAniType = ANI_TYPE_STAND;
		}
		else if( fSpeed <= m_fCurMaximumSpeed ) {
			m_pVelocityCtrl->SetCurSpeed( m_fCurMaximumSpeed );
			fSpeed = m_fCurMaximumSpeed;
		}
//		if( m_fCurMaximumSpeed == 0.f ) {
			nAniType = ANI_TYPE_WALK;
			m_fFrameAdd = (float)(int)(fSpeed * 0.4f);
			if( m_fFrameAdd < 1.f ) m_fFrameAdd = 1.f;
//		}
	}
	else if( fCurSpeed < m_fCurMaximumSpeed ) {
		fSpeed = m_pVelocityCtrl->GetSpeed( CFcVelocityCtrl::ACCELERATION );
		if( fSpeed >= m_fCurMaximumSpeed ) {
			m_pVelocityCtrl->SetCurSpeed( m_fCurMaximumSpeed );
			fSpeed = m_fCurMaximumSpeed;
		}
	}
	else fSpeed = m_fCurMaximumSpeed;

	if( fSpeed == 0.f && m_fCurMaximumSpeed == 0.f ) {
		nAniType = ANI_TYPE_STAND;
	}
	MoveZ( fSpeed );

	if( nAniType == -1 ) {
		if( fSpeed <= m_pUnitInfoData->GetMoveSpeed( m_pUnitInfoData->GetAniIndex( ANI_TYPE_WALK, 0 ) ) ) {
			nAniType = ANI_TYPE_WALK;
			m_pVelocityCtrl->SetAcceleration( 1.09f );
		}
		else {
			nAniType = ANI_TYPE_RUN;
			m_pVelocityCtrl->SetAcceleration( 1.13f );
		}
	}
	if( m_nRotationAniType == -1 ) {
		ChangeAnimation( nAniType, m_nCurAniTypeIndex ); 
	}
	else {
		int nPrevAniType = m_nCurAniType;
		if( nAniType == ANI_TYPE_WALK || nAniType == ANI_TYPE_STAND ) {
			ChangeAnimation( ANI_TYPE_CUSTOM_0, m_nRotationAniType );
			if( nPrevAniType != m_nCurAniType ) m_nBlendFrame = 10;
		}
		else {
			ChangeAnimation( nAniType, -1 ); 
			ChangeAnimation( nAniType, -1 ); 
		}
	}
	if( !m_VecLinkObjHandle.empty() && m_VecLinkObjHandle[0]->IsMovable() )
		m_VecLinkObjHandle[0]->ChangeAnimation( nAniType, 0, ANI_ATTR_HORSE );
}

void CFcHorseObject::CmdMove( int nX, int nY, float fSpeed, int nAniType, int nAniIndex )
{
	CFcUnitObject::CmdMove( nX, nY, fSpeed, nAniType, nAniIndex );
	m_bStopFlag = false;
}

void CFcHorseObject::CmdStop( int nAniType, int nAniIndex )
{
	m_bStopFlag = true;
	m_fCurMaximumSpeed = 0.f;
}

bool CFcHorseObject::IsHittable( GameObjHandle Handle )
{
	return false;
}

void CFcHorseObject::CheckHitLinkObject( GameObjHandle Handle, int &nAniAttr, int &nAniType, int &nAniTypeIndex )
{
	nAniAttr = ANI_ATTR_HORSE;
	nAniTypeIndex = 0;

	if( nAniType == ANI_TYPE_DOWN ) {
		Handle->RideOut();
		nAniAttr = 0;
	}
	if( Handle->GetHP() <= 0 ) {
		Handle->RideOut();
		Handle->SetVelocity( &D3DXVECTOR3( 0.f, 20.f, -15.f ) );

		nAniAttr = 0;
		nAniType = ANI_TYPE_DOWN_HIT;
	}
}


void CFcHorseObject::ProcessFlocking()
{
	/*
	if( m_bDead ) return;

	m_nFlockingInterval++;
//	if( m_nFlockingInterval % ( FLOCKING_INTERVAL / 4 ) == 0 ) {
		float fScanRange = GetUnitRadius() * 3;
		m_VecFlocking.erase( m_VecFlocking.begin(), m_VecFlocking.end() );
		CFcWorld::GetInstance().GetObjectListInRange( (D3DXVECTOR3*)&GetPos(), fScanRange, m_VecFlocking );
//	}

	D3DXVECTOR3 Direction;
	fScanRange = GetUnitRadius();
	for( int i = 0; i <(int)m_VecFlocking.size(); i++ ) {
		if( !CFcGameObject::IsValid( m_VecFlocking[i] ) ) continue;
		if( !m_VecFlocking[i]->IsFlockable( m_Handle ) ) continue;

		Direction = m_VecFlocking[i]->GetPos() - GetPos();
		Direction.y = 0.f;
		float fSumRadius = fScanRange + m_VecFlocking[ i ]->GetUnitRadius();
		float fLength = D3DXVec3Length( &Direction );
		if( fLength > fSumRadius ) continue;

		float fSpeed = ( fSumRadius ) - fLength;

		D3DXVec3Normalize( &Direction, &Direction );
		m_VecFlocking[i]->Move( &( Direction * fSpeed ) );

		m_VecFlocking[i]->SetFlocking( true, fSumRadius );
	}
	*/
}


bool CFcHorseObject::IsFlockable( GameObjHandle Handle )
{
	switch( Handle->GetClassID() ) {
		case Class_ID_Horse: return true;
		case Class_ID_Catapult: return true;
		case Class_ID_MoveTower: return true;
		case Class_ID_Hero:
			switch( ((HeroObjHandle)Handle)->GetHeroClassID() ) {
				case Class_ID_Hero_VigkVagk:	return true;
				default:
					break;
			}
		default:
			break;
	}
	return false;
}
