#include "StdAfx.h"
#include "FcFlyUnitObject.h"
#include "FcGameObject.h"
#include "ASData.h"
#include "FcVelocityCtrl.h"
#include "FcUtil.h"
#include "FcSOXLoader.h"
#include "./Data/SignalType.h"
#include "FcWorld.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

#define CUSTOM_SIGNAL_END_GLIDE	1
#define CUSTOM_SIGNAL_END_FLY	2
#define CUSTOM_SIGNAL_CHANGE_MAXIMUM_SPEED 3


int CFcFlyUnitObject::s_nRotateAngle = 5;
int CFcFlyUnitObject::s_nRollingAngle = 5;
int CFcFlyUnitObject::s_nPitchAngle = 3;

CFcFlyUnitObject::CFcFlyUnitObject( CCrossVector *pCross )
: CFcGameObject( pCross )
{
	SetClassID( Class_ID_Fly );
	m_fMinHeight = 3000.f;
	m_fAttackHeight = 500.f;

	m_nMaxRollingAngle = 100;

	// 감속,가속
	m_pVelocityCtrl = new CFcVelocityCtrl( 3.0f, 50.f, 1.f, 1.05f, 0.95f );

	// 상승..하강
	m_pHeightCtrl = new CFcVelocityCtrl( 1.2f, 41.f, 1.f, 1.04f, 0.96f );
	m_fHeight = m_fMinHeight;


	m_nPrevRollAngle = 0;
	m_nRollAngle = 0;
	m_fHeight = m_fMinHeight + g_BsKernel.GetLandHeight( m_Cross.m_PosVector.x, m_Cross.m_PosVector.z );
	m_fFlyGravity = 0.f;

	m_fPrevHeight = 0.f;

	m_bRotate = false;
	m_bAttackFlag = false;
	m_bFlying = false;
	m_bDirectDie = false;
	m_bFallDamage = false;

	m_nPitchAngle = 0;
}

CFcFlyUnitObject::~CFcFlyUnitObject()
{
	SAFE_DELETE( m_pHeightCtrl );
	SAFE_DELETE( m_pVelocityCtrl );
}

void CFcFlyUnitObject::Process()
{
	CFcGameObject::Process();

	/*
	switch( m_nRagdollCount ) {
		case 0:
			if( GetHP() <= 0 ) {
				// Lagdoll Enable
				RunRagdoll();
				SetRagDollMode( RAGDOLL_MODE_FREE );
				SetDie();
				m_nRagdollCount = 1;
			}
			break;
		case 5:
			m_pHeightCtrl->SetMaximumSpeed( 0.f );
			m_pHeightCtrl->SetMinimumSpeed( 0.f );
			m_pHeightCtrl->SetStartSpeed( 0.f );
			m_fCurHeight = m_fMinHeight = 0.f;
//			g_BsKernel.SendMessage( m_nEngineIndex, BS_PHYSICS_SIMULATION, TRUE );
			++m_nRagdollCount;
			break;
		default:
			++m_nRagdollCount;
			break;
		case 400:
//			g_BsKernel.SendMessage( m_nEngineIndex, BS_PHYSICS_SIMULATION, FALSE );
			break;
	}
	*/

	if( !m_bDead ) {
		ProcessAttack();
		ProcessMove();
	}
}


void CFcFlyUnitObject::PostProcess()
{

	m_fGravity = 0.f;
	CFcGameObject::PostProcess();

	if( m_bDead == true ) {
		m_vVelocity = D3DXVECTOR3( 0.f, 0.f, 0.f );
		if( m_nCurAniType != ANI_TYPE_DIE && m_nCurAniType != ANI_TYPE_CUSTOM_1 ) {
			if( m_bDirectDie == false )
				ChangeAnimation( ANI_TYPE_CUSTOM_1, 0 );
			else ChangeAnimation( ANI_TYPE_DIE, 1 );

			m_fCurHeight = m_fMinHeight = 1.f;
			m_pHeightCtrl->SetCurSpeed( 0.f );

		}
		else {
			if( m_nCurAniType != ANI_TYPE_DIE )
				SetAniPosToLocalPosFlag( 0 );
			else SetAniPosToLocalPosFlag( BS_CALC_POSITION_Y );
			if( m_nCurAniType == ANI_TYPE_CUSTOM_1 && m_nCurAniTypeIndex == 1 && m_bFallDamage == false ) {
				D3DXVECTOR3 vNormal;
				float fHeight = g_BsKernel.GetLandHeight( GetPos().x, GetPos().z, &vNormal );
				if( m_Cross.m_PosVector.y <= fHeight + m_fCurHeight ) {
					ChangeAnimation( ANI_TYPE_DIE, 0 );
					m_fCurHeight = m_fMinHeight = 0.f;
					m_Cross.m_PosVector.y = fHeight;

					_FC_RANGE_DAMAGE rd;
					rd.Attacker = m_Handle;
					rd.pPosition = &m_Cross.m_PosVector;
					rd.fRadius = 350.f;
					rd.nPower = 100;
					rd.nPushY = 20; 
					rd.nPushZ = 25;
					g_FcWorld.GiveRangeDamage( rd );

					m_Cross.m_YVector = vNormal;
					m_Cross.UpdateVectorsY();

					m_bFallDamage = true;
				}
			}
			else if( m_nCurAniType == ANI_TYPE_DIE && m_nCurAniTypeIndex == 1 && m_bFallDamage == false ) {
				D3DXVECTOR3 vNormal;
				float fHeight = g_BsKernel.GetLandHeight( GetPos().x, GetPos().z, &vNormal );
				if( m_Cross.m_PosVector.y <= fHeight + m_fCurHeight ) {
					m_fCurHeight = m_fMinHeight = 0.f;
					m_Cross.m_PosVector.y = fHeight;

					_FC_RANGE_DAMAGE rd;
					rd.Attacker = m_Handle;
					rd.pPosition = &m_Cross.m_PosVector;
					rd.fRadius = 350.f;
					rd.nPower = 100;
					rd.nPushY = 20; 
					rd.nPushZ = 25;
					g_FcWorld.GiveRangeDamage( rd );

					m_Cross.m_YVector = vNormal;
					m_Cross.UpdateVectorsY();

					m_bFallDamage = true;
				}
			}
		}
	}

	if( m_fCurHeight > 0.f ) {
		// 높이 계산
		float fHeight = g_BsKernel.GetLandHeight( GetPos().x, GetPos().z );
		float fVel = 0.f;

		D3DXVECTOR3 vTarget = m_Cross.m_PosVector + ( m_Cross.m_ZVector * ( m_pVelocityCtrl->GetCurSpeed() * 20.f )  );

		float fMapWidth, fMapHeight;
		g_FcWorld.GetMapSize( fMapWidth, fMapHeight );
		if( vTarget.x <= 0.f ) vTarget.x = 0.f;
		if( vTarget.x >= fMapWidth ) vTarget.x = fMapWidth - 1.f;
		if( vTarget.z <= 0.f ) vTarget.z = 0.f;
		if( vTarget.z >= fMapHeight ) vTarget.z = fMapHeight - 1.f;

		float fHeight2 = CFcWorld::GetInstance().GetLandHeight( vTarget.x, vTarget.z );
		if( fHeight < fHeight2 ) {
			if( fHeight2 > m_Cross.m_PosVector.y ) {
				m_bFlying = false;
				m_pVelocityCtrl->GetSpeed( CFcVelocityCtrl::DECELERATION );
				m_pVelocityCtrl->GetSpeed( CFcVelocityCtrl::DECELERATION );
			}
			fHeight = fHeight2;
		}
		if( fHeight + m_fCurHeight > GetPos().y ) {
			fVel = m_pHeightCtrl->GetSpeed( CFcVelocityCtrl::ACCELERATION );
			m_fFlyGravity *= 0.94f;
		}
		else {
			fVel = m_pHeightCtrl->GetSpeed( CFcVelocityCtrl::DECELERATION );
			if( m_nCurAniType == ANI_TYPE_CUSTOM_1 ) m_fFlyGravity += -1.f;
			else m_fFlyGravity += -0.5f;
			if( m_fFlyGravity > 47.f )
				m_fFlyGravity = 47.f;
		}

		m_fHeight += fVel;
		m_fHeight += m_fFlyGravity;
		if( m_bDead == false && m_fHeight <= fHeight + 100 ) {
			m_fFlyGravity = 0.f;
		}


		if( m_fCurHeight == 0.f && m_fHeight < fHeight )
			m_fHeight = fHeight;

		m_fPrevHeight = m_Cross.m_PosVector.y;
		m_Cross.m_PosVector.y = m_fHeight;
	}
	SetTickInterval( 1 );
}

void CFcFlyUnitObject::CmdMove( int nX, int nY, float fSpeed, int nAniType, int nAniIndex )
{
	if( m_bDead )
		return;

	float fWidth, fHeight;
	g_FcWorld.GetMapSize( fWidth, fHeight );
	if( nX < 0.f ) nX = 1000;
	if( nX >= (int)fWidth ) nX = (int)fWidth - 1000;
	if( nY < 0.f ) nY = 1000;
	if( nY >= (int)fHeight ) nY = (int)fHeight - 1000;

	if( IsAttack() || IsJump() || IsHit() || IsDown() )
	{
		m_MoveTargetPos.x = (float)nX;
		m_MoveTargetPos.y = 0.f;
		m_MoveTargetPos.z = (float)nY;
		return;
	}

	D3DXVECTOR3 Pos = GetDummyPos();

	m_MoveDir.x = (float)nX - Pos.x;
	m_MoveDir.y = 0.f;
	m_MoveDir.z = (float)nY - Pos.z;
	D3DXVec3Normalize(&m_MoveDir, &m_MoveDir);

	m_MoveTargetPos.x = (float)nX;
	m_MoveTargetPos.y = 0.f;
	m_MoveTargetPos.z = (float)nY;

	if( fSpeed == -1.f ) {
		int nIndex = m_pUnitInfoData->GetAniIndex( nAniType & 0x00FF, nAniIndex, nAniType & 0xFF00 );
		if( nIndex != -1 ) m_pVelocityCtrl->SetMaximumSpeed( (float)m_pUnitInfoData->GetMoveSpeed( nIndex ) );
	}
	else m_pVelocityCtrl->SetMaximumSpeed( fSpeed );

	ChangeAnimation( ANI_TYPE_RUN, nAniIndex );
//	SetFlocking( false );
}

void CFcFlyUnitObject::CmdStop( int nAniType, int nAniIndex )
{
	if( IsMove() )
	{
		m_MoveTargetPos = m_Cross.GetPosition();
		ChangeAnimation( nAniType, nAniIndex );
	}
}

// 즉시 방향을 바꾼다
void CFcFlyUnitObject::CmdLookUp( GameObjHandle Handle )
{
	return;
}

void CFcFlyUnitObject::CmdVictory( float fX, float fY )
{
	//	D3DXVECTOR3 Dir;

	D3DXVECTOR3 Pos = m_Cross.GetPosition();

	m_MoveDir.x = fX - Pos.x;
	m_MoveDir.y = 0.f;
	m_MoveDir.z = fY - Pos.z;
	D3DXVec3Normalize(&m_MoveDir, &m_MoveDir);

	m_Cross.m_ZVector=m_MoveDir;
	m_Cross.UpdateVectors();

	if( IsStand() )
	{
		ChangeAnimation( ANI_TYPE_VICTORY, 0 );
	}
}

void CFcFlyUnitObject::RotationProcess()
{
	float fDot;
	D3DXVECTOR3 CrossVec;
	// Pitch Process
	float fPitch = m_fPrevHeight - m_Cross.m_PosVector.y;

	if( m_nCurAniType == ANI_TYPE_HIT ) return;
	if( ( m_nCurAniType == ANI_TYPE_RUN || ( m_nCurAniType == ANI_TYPE_DEFAULT && m_nCurAniTypeIndex == 3 && m_bAttackFlag == false ) ) && m_fPrevHeight != 0.f ) {
		if( fabs( fPitch ) > 10 ) {
			if( fPitch > 0.f ) {
				m_nPitchAngle += s_nPitchAngle;
				if( m_nPitchAngle > 130 ) m_nPitchAngle = 130;
			}
			else if( fPitch < 0.f ) {
				m_nPitchAngle -= (s_nPitchAngle*2);
				if( m_nPitchAngle < -130 ) m_nPitchAngle = -130;
			}
		}
		else {
			if( m_nPitchAngle > 0 ) {
				m_nPitchAngle -= (s_nPitchAngle*2);
				if( m_nPitchAngle < 0 ) m_nPitchAngle = 0;
			}
			else if( m_nPitchAngle < 0 ) {
				m_nPitchAngle += s_nPitchAngle;
				if( m_nPitchAngle > 0 ) m_nPitchAngle = 0;
			}
		}
	}
	else {
		if( m_nPitchAngle > 0 ) {
			m_nPitchAngle -= (s_nPitchAngle*2);
			if( m_nPitchAngle < 0 ) m_nPitchAngle = 0;
		}
		else if( m_nPitchAngle < 0 ) {
			m_nPitchAngle += s_nPitchAngle;
			if( m_nPitchAngle > 0 ) m_nPitchAngle = 0;
		}
	}
	////////////////

	D3DXVECTOR3 vTargetVec = m_MoveTargetPos - GetDummyPos();
	vTargetVec.y = 0.f;
	D3DXVec3Normalize( &vTargetVec, &vTargetVec );

	fDot = D3DXVec3Dot(&vTargetVec, &(m_Cross.m_ZVector));
	float fAngle = acos( fDot ) * 180.f / 3.1415926f;

	if( D3DXVec3Length(&m_MoveDir) <= 0.0f /*&& fAngle <= 5.f && m_pVelocityCtrl->GetCurSpeed() == 0.f*/ ) {
		m_Cross.RotateRoll( m_nPrevRollAngle );

		if( m_nRollAngle != 0 ) {
			if( m_nRollAngle > 0 ) {
				m_nRollAngle -= s_nRollingAngle;
				if( m_nRollAngle < 0 ) m_nRollAngle = 0;
			}
			else if( m_nRollAngle < 0 ) {
				m_nRollAngle += s_nRollingAngle;
				if( m_nRollAngle > 0 ) m_nRollAngle = 0;
			}
			if( m_nRollAngle == 0 ) m_bRotate = false;
			else m_bRotate = true;
		}
		m_Cross.RotateRoll( m_nRollAngle );
		m_nPrevRollAngle = -m_nRollAngle;
		return;
	}

	m_MoveDir = vTargetVec;
	if( fAngle > 5.f ) {
		D3DXVec3Cross(&CrossVec, &m_MoveDir, &(m_Cross.m_ZVector));

		m_Cross.RotateRoll( m_nPrevRollAngle );
		if( CrossVec.y >= 0.f ) {
			m_Cross.RotateYaw(s_nRotateAngle);
			m_nRollAngle -= s_nRollingAngle;
			if( m_nRollAngle < -m_nMaxRollingAngle ) m_nRollAngle = -m_nMaxRollingAngle;
		}
		else{
			m_Cross.RotateYaw(-s_nRotateAngle);
			m_nRollAngle += s_nRollingAngle;
			if( m_nRollAngle > m_nMaxRollingAngle ) m_nRollAngle = m_nMaxRollingAngle;
		}
		m_Cross.RotateRoll( m_nRollAngle );
		m_nPrevRollAngle = -m_nRollAngle;
		m_bRotate = true;

		return;
	}
 
	m_Cross.m_ZVector=m_MoveDir;
	m_Cross.UpdateVectors();
	m_MoveDir=D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	m_Cross.RotateRoll( m_nPrevRollAngle );

	if( m_nRollAngle != 0 ) {
		if( m_nRollAngle > 0 ) {
			m_nRollAngle -= s_nRollingAngle;
			if( m_nRollAngle < 0 ) m_nRollAngle = 0;
		}
		else if( m_nRollAngle < 0 ) {
			m_nRollAngle += s_nRollingAngle;
			if( m_nRollAngle > 0 ) m_nRollAngle = 0;
		}
		if( m_nRollAngle == 0 ) m_bRotate = false;
		else m_bRotate = true;
	}
	m_Cross.RotateRoll( m_nRollAngle );
	m_nPrevRollAngle = -m_nRollAngle;
}


void CFcFlyUnitObject::MoveZ(float fDist)
{
	m_MoveVector+=m_Cross.m_ZVector*fDist; 
}

void CFcFlyUnitObject::ProcessMove()
{

	if( m_nCurAniType == ANI_TYPE_STAND || m_nCurAniType == ANI_TYPE_HIT ) m_bFlying = false;
	else {
		m_bFlying = true;
	}

	float fSpeed;
	if( m_bFlying == true ) {
		fSpeed = m_pVelocityCtrl->GetSpeed( CFcVelocityCtrl::ACCELERATION );
		MoveZ( fSpeed );

		D3DXVECTOR3 Pos = GetDummyPos() - m_MoveTargetPos;
		Pos.y = 0.f;
		float fLength = D3DXVec3Length( &Pos );

		// 졸 느릴듯..나중에 함수루 구하장~
		if( fLength <= GetStopableDistance() ) 
			CmdStop( ANI_TYPE_STAND );
	}
	else {
		fSpeed = m_pVelocityCtrl->GetSpeed( CFcVelocityCtrl::DECELERATION );
		MoveZ( fSpeed );
	}

	// 영역 밖으로 넘어가는거 막자~
	float fWidth, fHeight;
	D3DXVECTOR3 vPostPos = m_Cross.m_PosVector + m_MoveVector;
	g_FcWorld.GetMapSize( fWidth, fHeight );
	/*
	if( vPostPos.x < 500.f || vPostPos.x >= ( fWidth - 500.f ) || vPostPos.z < 500.f || vPostPos.z >= ( fHeight - 500.f ) ) {
		if( m_bFlying ) {
			CmdStop( ANI_TYPE_STAND | GetCurAniAttr() );
		}
	}
	*/
	if( m_Cross.m_PosVector.x < 0.f ) m_Cross.m_PosVector.x = 0.f;
	if( m_Cross.m_PosVector.x >= fWidth ) m_Cross.m_PosVector.x = fWidth - 1.f;
	if( m_Cross.m_PosVector.z < 0.f ) m_Cross.m_PosVector.z = 0.f;
	if( m_Cross.m_PosVector.z >= fHeight ) m_Cross.m_PosVector.z = fHeight - 1.f;
}

float CFcFlyUnitObject::GetStopableDistance()
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

void CFcFlyUnitObject::ProcessAttack()
{
	int nAniType = m_pUnitInfoData->GetAniType( m_nAniIndex );
	int nAniIndex = m_pUnitInfoData->GetAniTypeIndex( m_nAniIndex );

	bool bFlag = false;

	m_bDirectDie = false;
	switch( nAniType ) {
		case ANI_TYPE_CUSTOM_0:
			if( m_fFrame >= 35.f ) {
				float fTemp = g_BsKernel.GetLandHeight( m_MoveTargetPos.x, m_MoveTargetPos.z );
				m_fCurHeight = fTemp + m_fAttackHeight;
				bFlag = true;
			}
			break;
		case ANI_TYPE_DEFAULT:
			switch( nAniIndex ) {
				case 1:		// 강하
					{
						float fTemp = g_BsKernel.GetLandHeight( m_MoveTargetPos.x, m_MoveTargetPos.z );
						m_fCurHeight = m_fAttackHeight;
						if( m_DummyPos.y - g_BsKernel.GetLandHeight( m_DummyPos.x, m_DummyPos.z ) <= m_fCurHeight ) {
							ChangeAnimation( ANI_TYPE_DEFAULT, 2, ANI_ATTR_NONE );
							if( m_fFlyGravity < -20.f ) m_fFlyGravity = -20.f;
						}
						bFlag = true;
					}
					break;
				case 2:		// 글라이딩 준비
					{
						float fTemp = g_BsKernel.GetLandHeight( m_MoveTargetPos.x, m_MoveTargetPos.z );
						m_fCurHeight = m_fAttackHeight;
						m_bAttackFlag = true;
						bFlag = true;
					}
					break;
				case 3:		// 글라이딩
					if( m_bAttackFlag == true ) {
						/*
						D3DXVECTOR2 vVec = D3DXVECTOR2( m_MoveTargetPos.x, m_MoveTargetPos.z ) - GetDummyPosV2();
						float fLength = D3DXVec2LengthSq( &vVec );
						D3DXVec2Normalize( &vVec, &vVec );
						float fDot = D3DXVec2Dot( &vVec, &D3DXVECTOR2( m_Cross.m_ZVector.x, m_Cross.m_ZVector.z ) );
						if( fDot <= 0.f ) {
							ChangeAnimation( ANI_TYPE_RUN, 0 );
						}
						else if( fLength <= 1000.f * 1000.f ) {
							ChangeAnimation( ANI_TYPE_ATTACK, 0, ANI_ATTR_BATTLE );
							m_bAttackFlag = false;
						}
						*/

						m_fCurHeight = m_fAttackHeight;
						bFlag = true;
						if( m_DummyPos.y - g_BsKernel.GetLandHeight( m_DummyPos.x, m_DummyPos.z ) <= m_fCurHeight ) {
							ChangeAnimation( ANI_TYPE_ATTACK, -1, ANI_ATTR_BATTLE );
							m_bAttackFlag = false;
						}
						m_bDirectDie = true;
					}
					break;
			}
			break;
		case ANI_TYPE_ATTACK:
			{
				m_fCurHeight = m_fAttackHeight;
				bFlag = true;
				m_bDirectDie = true;
			}
			break;
	}
	if( bFlag == false ) {
		m_fCurHeight = m_fMinHeight;
	}
}
void CFcFlyUnitObject::ProcessFlocking()
{
}

bool CFcFlyUnitObject::IsFlockable( GameObjHandle Handle )
{
	return false;
}

void CFcFlyUnitObject::CheckMoveLock( TroopObjHandle hTroop )
{
	// 할 필요 없다.
	return;
}

void CFcFlyUnitObject::SignalCheck( ASSignalData *pSignal )
{
	switch( pSignal->m_nID )
	{
		case SIGNAL_TYPE_CUSTOM_MESSAGE:
			if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
			{
				switch( pSignal->m_pParam[0] ) {
					case CUSTOM_SIGNAL_END_GLIDE:
						if( m_nPitchAngle > 80 ) {
							ChangeAnimation( ANI_TYPE_DEFAULT, 3, ANI_ATTR_NONE, true );
						}
						else if( m_bRotate == false ) {
							ChangeAnimation( ANI_TYPE_RUN, 0, ANI_ATTR_NONE, false );
						} 
						break;
					case CUSTOM_SIGNAL_END_FLY:
						if( m_nPitchAngle > 80 ) {
							ChangeAnimation( ANI_TYPE_DEFAULT, 3, ANI_ATTR_NONE, false );
						}
						else if( m_bRotate == true ) {
							if( Random(2) == 0 )
								ChangeAnimation( ANI_TYPE_DEFAULT, 3, ANI_ATTR_NONE, false );
						}
						break;
					case CUSTOM_SIGNAL_CHANGE_MAXIMUM_SPEED:
						m_pVelocityCtrl->SetMaximumSpeed( (float)pSignal->m_pParam[1] );
						m_bFlying = ( pSignal->m_pParam[2] ) ? true : false;
						break;
				}
			}
			break;
		default:
			CFcGameObject::SignalCheck( pSignal );
	}
}

bool CFcFlyUnitObject::Render()
{
	bool bRet;

	m_Cross.RotatePitch( m_nPitchAngle );

	bRet = CFcGameObject::Render();

	m_Cross.RotatePitch( -m_nPitchAngle );

	return bRet;
}

bool CFcFlyUnitObject::GetExactHitAni( int &nAniType, int &nAniIndex )
{
	switch( nAniType ) {
		case ANI_TYPE_DOWN_HIT:
		case ANI_TYPE_HIT:
		case ANI_TYPE_DOWN:
			nAniType = ANI_TYPE_HIT;
			m_Cross.RotateRoll( m_nPrevRollAngle );

			if( D3DXVec3LengthSq( &m_MoveDir ) > 0.f )
				m_Cross.m_ZVector=m_MoveDir;
			else m_Cross.m_ZVector = D3DXVECTOR3( 0.f, 0.f, 1.f );
			m_Cross.UpdateVectors();
			m_MoveDir=D3DXVECTOR3(0.0f, 0.0f, 0.0f);
//			}

			m_nPrevRollAngle = 0;
			m_nRollAngle = 0;
			m_nPitchAngle = 0;
			break;
	}
	return true;
}

bool CFcFlyUnitObject::CheckHit()
{
	if( m_fHP <= 0.f || m_bDead == true ) return false;
	return CFcGameObject::CheckHit();
}

bool CFcFlyUnitObject::IsMove()
{
	if( m_pVelocityCtrl->GetCurSpeed() > 0.f ) return true;
	return CFcGameObject::IsMove();
}

bool CFcFlyUnitObject::IsHittable( GameObjHandle Handle )
{
	if( m_fHP <= 0.f || m_bDead == true ) return false;
	return CFcGameObject::IsHittable( Handle );
}

void CFcFlyUnitObject::CmdPlace( float nX, float nY )
{
	CFcGameObject::CmdPlace( nX, nY );
	m_fHeight = m_fMinHeight + g_BsKernel.GetLandHeight( (float)nX, (float)nY );
}