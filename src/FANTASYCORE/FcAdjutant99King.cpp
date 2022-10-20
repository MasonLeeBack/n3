#include "StdAfx.h"
#include "FcAdjutantObject.h"

#include "FcFXCommon.h"
#include "FcFxManager.h"

#include "FcWorld.h"
#include "./Data/SignalType.h"
#include "FcAIObject.h"
#include "FcAISearchSlot.h"
#include "FcAdjutant99King.h"
#include "FcWorld.h"
#include "FcTroopObject.h"
#include "FcTroopManager.h"
#include "FcVelocityCtrl.h"
#include "Data/FXList.h"
#include "FcProjectileMagicFx.h"
#include "FcProjectileTimeBomb.h"
#include "FcGlobal.h"


CFcAdjutant99King::CFcAdjutant99King(CCrossVector *pCross)
: CFcAdjutantObject( pCross )
{
	SetAdjutantClassID( Class_ID_Adjutant_99King );
	m_bFloating = false;
	m_bAddHeight = false;
	m_bLinkFx = false;
	m_BlackHall.Identity();

	m_nLinkFxIndex[0] = m_nLinkFxIndex[1] = -1;

	m_pVelocityCtrl = new CFcVelocityCtrl( 3.0f, 20.f, 1.f, 1.09f, 0.92f );

	m_nTentacleRemainCount = 0;
	m_pTentacleSignal = NULL;
}

CFcAdjutant99King::~CFcAdjutant99King()
{
	delete m_pVelocityCtrl;

	for( int i=0; i<2; i++ ) {
		if( m_nLinkFxIndex[i] == -1 ) continue;

		g_BsKernel.SendMessage( m_nEngineIndex, BS_UNLINKOBJECT, m_nLinkFxIndex[i] );
		g_BsKernel.DeleteObject( m_nLinkFxIndex[i] );
	}
}

void CFcAdjutant99King::Process()
{
	ProcessMove();
	CFcAdjutantObject::Process();
	CheckBlackHall();
	CheckTentacle();
	// Fx Link
	if( m_bLinkFx == false ) {
		m_bLinkFx = true;

		m_nLinkFxIndex[0] = g_BsKernel.CreateFXObject( FX_99K_EYES );
		m_nLinkFxIndex[1] = g_BsKernel.CreateFXObject( FX_99K_AURA );

		for( int i=0; i<2; i++ ) {
			g_BsKernel.SetFXObjectState( m_nLinkFxIndex[i], CBsFXObject::PLAY, -1 );
			g_BsKernel.ShowObject( m_nLinkFxIndex[i], true );
		}

		g_BsKernel.SendMessage( m_nEngineIndex, BS_LINKOBJECT_NAME, (DWORD)"Bip01 Head", m_nLinkFxIndex[0] );
		g_BsKernel.SendMessage( m_nEngineIndex, BS_LINKOBJECT_NAME, (DWORD)"Bip01", m_nLinkFxIndex[1] );
	}


}

void CFcAdjutant99King::CheckTentacle()
{
	if( m_nTentacleRemainCount <= 0 ) return;
	if( GetProcessTick() - m_nTentaclePrevTick >= m_nTentacleInterval ) {
		m_nTentaclePrevTick += m_nTentacleInterval;

		CCrossVector Cross;
		PROJECTILE_PARAM Param;
		ProjetileHandle Handle;
		D3DXVECTOR3 vNor;

		Param.hParent = m_Handle;
		Param.pSignal = m_pTentacleSignal;
		Param.fSize = 150.f;


		Cross.m_PosVector = m_TentaclePos;
		Cross.m_PosVector.y = g_BsKernel.GetLandHeight( m_TentaclePos.x, m_TentaclePos.z, &vNor );
		Cross.m_YVector = vNor;
		Cross.UpdateVectorsY();

		Cross.RotateYaw( Random(1024) );
		Cross.MoveFrontBack( (float)Random( m_nTentacleRandomRange ) );

		Handle = CFcProjectile::CreateObject( CFcProjectile::TIMEBOMB, &Cross );
		((CFcProjectileTimeBomb*)Handle.GetPointer())->Initialize( FX_99K07TENTACLE, -1, 5, 1, 5, &Param );

		m_nTentacleRemainCount--;
	}
}

void CFcAdjutant99King::PostProcess()
{
	CFcAdjutantObject::PostProcess();
}

void CFcAdjutant99King::ProcessMove()
{
	if( !m_bFloating ) return;
	if( m_nCurAniType == ANI_TYPE_WALK || m_nCurAniType == ANI_TYPE_RUN || m_nChangeMoveAniIndex != -1 ) {
		D3DXVECTOR3 vVec = m_MoveTargetPos - GetDummyPos();
		vVec.y = 0.f;
		float fDistance = 0.f;
		fDistance = m_fMoveTargetLength;
		if( m_bApplyFlocking == true && m_fMaxFlockingSize > fDistance ) {
			fDistance = m_fMaxFlockingSize;
		}
		fDistance += GetStopableDistance();

		if( D3DXVec3Length( &vVec ) <= fDistance ) {
			m_fMoveSpeed = m_pVelocityCtrl->GetSpeed( CFcVelocityCtrl::DECELERATION );
		}
		else {
			m_fMoveSpeed = m_pVelocityCtrl->GetSpeed( CFcVelocityCtrl::ACCELERATION );
		}
	}
	else {
		m_fMoveSpeed = m_pVelocityCtrl->GetSpeed( CFcVelocityCtrl::DECELERATION );
	}
}

float CFcAdjutant99King::GetStopableDistance()
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


bool CFcAdjutant99King::Render()
{
	return CFcAdjutantObject::Render();
}

float CFcAdjutant99King::AddPos( float x, float y, float z )
{
	if( m_bFloating == true )
		return ( CFcAdjutantObject::AddPos( x, y, z ) + 75.f );
	return CFcAdjutantObject::AddPos( x, y, z );
}

void CFcAdjutant99King::SiganlCheckWarp( ASSignalData *pSignal )
{
	GameObjHandle hUnit;
	D3DXVECTOR3 vTarget, vVec;
	float fWarpRange = 500.f;

	if( GetAIHandle() ) {
		GameObjHandle *pSearchHandle = GetAIHandle()->GetSearchSlot()->GetHandle(0);
		if( pSearchHandle )
			hUnit = *pSearchHandle;
	}

	if( !CFcBaseObject::IsValid( hUnit ) ) {
		vTarget = m_Cross.m_ZVector * ( fWarpRange * 2.f );
		vVec = -m_Cross.m_ZVector;
		vVec.y = 0.f;
	}
	else {
		vTarget = hUnit->GetPos() - m_Cross.m_PosVector;
		vTarget.y = 0.f;
		float fLength = D3DXVec3Length( &vTarget );
		D3DXVec3Normalize( &vTarget, &vTarget );
		vVec = -vTarget;
		vTarget *= ( fWarpRange + fLength );
	}

	m_Cross.m_PosVector += vTarget;
	m_Cross.m_ZVector = vVec;
	m_Cross.UpdateVectors();
	g_FcWorld.PlaySimpleFx( FX_99K_WARP_E, &m_Cross );
}

void CFcAdjutant99King::SignalCheck( ASSignalData *pSignal )
{
	switch( pSignal->m_nID ) {
		case SIGNAL_TYPE_CUSTOM_MESSAGE:
			if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) ) {
				switch( pSignal->m_pParam[0] ) {
					case 0:	// 공중 부양 On/Off
						m_bFloating = ( pSignal->m_pParam[1] == 1 );
						break;
					case 1:	// 워프
						SiganlCheckWarp( pSignal );
						break;
				}
			}
			return;
		case SIGNAL_TYPE_SHOOT_PROJECTILE:
			if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) ) {
				switch( m_nCurAniTypeIndex ) {
					case 5:		// 빨아들이기
					case 13:
						ProjectileBlackHall( pSignal );
						break;
					case 16:	// 원기옥
					case 17:
						ProjectileEnergyBall( pSignal );
						break;
					case 8:		// 촉수
						ProjectileTentacle( pSignal );
						break;


				}
			}
			return;
	}
	CFcAdjutantObject::SignalCheck( pSignal );
}

void CFcAdjutant99King::ProjectileBlackHall( ASSignalData *pSignal )
{
	if( m_BlackHall ) return;
	CCrossVector Cross;
	PROJECTILE_PARAM Param;
	D3DXVECTOR3 Position;

	Param.hParent = m_Handle;
	Param.pSignal = pSignal;

	Cross = m_Cross;
	Position.x = ( float )pSignal->m_pParam[ 1 ];
	Position.y = ( float )pSignal->m_pParam[ 2 ];
	Position.z = ( float )pSignal->m_pParam[ 3 ];
	D3DXVec3TransformNormal( &Position, &Position, m_Cross );
	Cross.m_PosVector += Position;

	m_BlackHall = CFcProjectile::CreateObject( CFcProjectile::BLACKHALL, &Cross );
	m_BlackHall->Initialize( -1, &Param );
}

void CFcAdjutant99King::ProjectileEnergyBall( ASSignalData *pSignal )
{
	GameObjHandle hUnit;
	if( GetAIHandle() ) {
		GameObjHandle *pSearchHandle = GetAIHandle()->GetSearchSlot()->GetHandle(0);
		if( pSearchHandle )
			hUnit = *pSearchHandle;
		else return;
	}
	else return;
	if( !CFcBaseObject::IsValid( hUnit ) ) return;

	if( CFcTroopObject::s_nTroopStateTable[ m_hTroop->GetState() ] & UNIT_RANGE ) {
		GetAIHandle()->GetSearchSlot()->SetHandle( 0, NULL );
	}

	CCrossVector Cross;
	D3DXVECTOR3 Position;
	PROJECTILE_PARAM Param;
	ProjetileHandle Handle;

	Param.hParent = m_Handle;
	Param.pSignal = pSignal;
	Param.vTarget = hUnit->GetPos();
	Param.vTarget.y += 50.f;
	Param.fSize = 700.f;

	Cross = m_Cross;
	Position.x = ( float )pSignal->m_pParam[ 1 ];
	Position.y = ( float )pSignal->m_pParam[ 2 ];
	Position.z = ( float )pSignal->m_pParam[ 3 ];
	D3DXVec3TransformNormal( &Position, &Position, m_Cross );
	Cross.m_PosVector += Position;

	Handle = CFcProjectile::CreateObject( CFcProjectile::MAGIC_FX, &Cross );
	((CFcProjectileMagicFx*)Handle.GetPointer())->Initialize( FX_99K03SPHEAR02, FX_99K03SPHEARB, &Param );
	((CFcProjectileMagicFx*)Handle.GetPointer())->SetDirectHideThrowFx( false );
	((CFcProjectileMagicFx*)Handle.GetPointer())->SetHitType( ANI_TYPE_DOWN );

	CFcVelocityCtrl *pCtrl = ((CFcProjectileMagicFx*)Handle.GetPointer())->GetVelocityCtrl();
	pCtrl->SetStartSpeed( 40.f );
	pCtrl->SetMaximumSpeed( 40.f );
	pCtrl->SetMinimumSpeed( 40.f );
}

void CFcAdjutant99King::ProjectileTentacle( ASSignalData *pSignal )
{
	GameObjHandle hUnit;
	if( GetAIHandle() ) {
		GameObjHandle *pSearchHandle = GetAIHandle()->GetSearchSlot()->GetHandle(0);
		if( pSearchHandle )
			hUnit = *pSearchHandle;
		else return;
	}
	else return;
	if( !CFcBaseObject::IsValid( hUnit ) ) return;

	// 레벨에 따라 조절해 주삼
	m_TentaclePos = hUnit->GetPos();
	m_nTentacleRemainCount = 4;
	m_nTentacleInterval = 5;
	m_nTentacleRandomRange = 300;
	m_pTentacleSignal = pSignal;

	m_nTentaclePrevTick = GetProcessTick();
}

bool CFcAdjutant99King::GetExactHitAni( int &nAniType, int &nAniIndex )
{
	switch( nAniType ) {
		case ANI_TYPE_DIE:
		case ANI_TYPE_DOWN:
		case ANI_TYPE_DOWN_HIT:
			if( m_fHP <= 0.f ) {
				nAniType = ANI_TYPE_DIE;
				if( m_bFloating == true ) nAniIndex = 1;
				else nAniIndex = 0;
				return true;
			}
			return false;
		default:	return false;
	}
	return false;
}

void CFcAdjutant99King::GetExactHitVelocity( float &fVelocityZ, float &fVelocityY )
{
	fVelocityZ = 0.0f;
	fVelocityY = 0.0f;
}

int CFcAdjutant99King::FindExactAni( int nAniIndex )
{
	int nAniType = m_pUnitInfoData->GetAniType(nAniIndex);
	int nAniTypeIndex = m_pUnitInfoData->GetAniTypeIndex(nAniIndex);
	int nAniAttr = m_pUnitInfoData->GetAniAttr(nAniIndex);

	switch( nAniType ) {
		case ANI_TYPE_STAND:
			if( m_bFloating == true ) return m_pUnitInfoData->GetAniIndex( ANI_TYPE_STAND, 1 );
			else return m_pUnitInfoData->GetAniIndex( ANI_TYPE_STAND, 0 );
			break;
		case ANI_TYPE_WALK:
			if( m_bFloating == true ) return m_pUnitInfoData->GetAniIndex( ANI_TYPE_RUN, 0, ANI_ATTR_BATTLE );
			else return m_pUnitInfoData->GetAniIndex( ANI_TYPE_WALK, 0, ANI_ATTR_BATTLE );
			break;
		case ANI_TYPE_RUN:
			if( m_bFloating == true ) return m_pUnitInfoData->GetAniIndex( ANI_TYPE_RUN, 0, ANI_ATTR_BATTLE );
			else return m_pUnitInfoData->GetAniIndex( ANI_TYPE_WALK, 0, ANI_ATTR_BATTLE );
			break;
	}
	return nAniIndex;
}


void CFcAdjutant99King::CmdLookUp( GameObjHandle Handle )
{
	if( !CFcBaseObject::IsValid( Handle ) ) return;

	D3DXVECTOR3 TargetPos = Handle->GetPos();
	D3DXVECTOR3 Pos = m_Cross.GetPosition();

	m_MoveDir.x = TargetPos.x - Pos.x;
	m_MoveDir.y = 0.f;
	m_MoveDir.z = TargetPos.z - Pos.z;
	BsAssert( (m_MoveDir.x == m_MoveDir.x) && (m_MoveDir.y == m_MoveDir.y) && (m_MoveDir.z == m_MoveDir.z) );
	D3DXVec3Normalize(&m_MoveDir, &m_MoveDir);

	m_MoveTargetPos = TargetPos;
}

void CFcAdjutant99King::CmdStop( int nAniType, int nAniIndex )
{
	if( m_bDead ) return;
	if( IsMove() || m_nChangeMoveAniIndex != -1 )
	{
		ResetMoveVector();
		m_MoveTargetPos = m_Cross.GetPosition() + ( m_Cross.m_ZVector * GetStopableDistance() );
		m_nChangeMoveAniIndex = -1;

		BsAssert( m_MoveTargetPos.x == m_MoveTargetPos.x );

		if( IsMove() ) {
			int nFindAni = FindExactAni( m_pUnitInfoData->GetAniIndex( nAniType & 0x00FF, nAniIndex, nAniType & 0xFF00 ) );
			ChangeAnimation( m_pUnitInfoData->GetAniType( nFindAni ), m_pUnitInfoData->GetAniTypeIndex( nFindAni ), m_pUnitInfoData->GetAniAttr( nFindAni ) );
		}
	}
}


void CFcAdjutant99King::CheckBlackHall()
{
	if( !m_BlackHall ) return;
	if( m_nAIControlIndex == -1 || IsDie() ) {
		((CFcProjectileBlackHall*)m_BlackHall.GetPointer())->Finish();
		m_BlackHall.Identity();
	}
}

void CFcAdjutant99King::LookAtHitUnit( D3DXVECTOR3 *pDirection )
{
}

bool CFcAdjutant99King::CheckDefense()
{
	if( m_bDead ) return false;
	if( !m_HitParam.ObjectHandle ) return false;

	int nAniIndex = 0;
	if( m_bFloating == true ) nAniIndex = 0;
	else  nAniIndex = 4;
	// 진오브일 경우 걍 무시!!
	if( g_FcWorld.IsProcessTrueOrb( GetTeam() ) ) {
		ChangeAnimation( ANI_TYPE_DEFENSE, 3 + nAniIndex, ANI_ATTR_NONE, false );
		return true;
	}
	bool bResult = false;
	if( m_nCurAniType == ANI_TYPE_DEFENSE ) {
//		ChangeAnimation( ANI_TYPE_DEFENSE, 3 + nAniIndex, ANI_ATTR_NONE, true );
		bResult = true;
	}
	if( bResult == false && Random( 100 ) < GetDefenseProb() ) {
		ChangeAnimation( ANI_TYPE_DEFENSE, 3 + nAniIndex, ANI_ATTR_NONE, false );
		bResult = true;
	}
	if( bResult == true ) {
		D3DXVECTOR3 vDir = m_HitParam.ObjectHandle->GetPos() - GetPos();
		vDir.y = 0.f;
		D3DXVec3Normalize( &vDir, &vDir );
		m_MoveDir = vDir;

		float fHitVelocityZ = ( float )m_HitParam.fVelocityZ * 0.6f;
		float fHitVelocityY = ( float )m_HitParam.fVelocityY * 0.6f;
		D3DXVECTOR3 Direction = m_HitParam.ObjectHandle->GetCrossVector()->m_PosVector - m_Cross.m_PosVector;

		Direction.y = 0.0f;
		D3DXVec3Normalize( &Direction, &Direction );

		GetExactHitVelocity( fHitVelocityZ, fHitVelocityY );

		SetVelocityX( Direction.x * fHitVelocityZ );
		SetVelocityZ( Direction.z * fHitVelocityZ );
		SetVelocityY( fHitVelocityY );
		return true;
	}
	return bResult;
}

bool CFcAdjutant99King::IsFlockable( GameObjHandle Handle )
{
	if( Handle == m_Handle ) return false;
	if( Handle->GetParentLinkHandle() == m_Handle ) return false;

	switch( Handle->GetClassID() ) {
		case Class_ID_Catapult: return true;
		default:
			break;
	}
	return false;
}

bool CFcAdjutant99King::CalculateDamage(int nAttackPower)
{
	if( m_bFloating ) {
		// 부유상태일때는 HP 1 달게 해야하는데.. nAttackPower 를 줄이면 GameObject에서 
		// - Defense 한 수치가 0보다 작으면 Damage 를 5로 셋팅하기 때문에 nAttackPower 를 조절해줘야한다.
		nAttackPower = GetDefense() + 1;
	}
	return CFcAdjutantObject::CalculateDamage( nAttackPower );
}

void CFcAdjutant99King::ProcessFlocking()
{
	float fScanRange = GetUnitRadius() * 4;
	m_VecFlocking.erase( m_VecFlocking.begin(), m_VecFlocking.end() );
	CFcWorld::GetInstance().GetObjectListInRange( (D3DXVECTOR3*)&GetPos(), fScanRange, m_VecFlocking );

	D3DXVECTOR3 Direction;
	fScanRange = GetUnitRadius();
	for( int i = 0; i <(int)m_VecFlocking.size(); i++ ) {
		if( !CFcGameObject::IsValid( m_VecFlocking[i] ) ) continue;
		if( !m_VecFlocking[i]->IsFlockable( m_Handle ) ) continue;

		Direction = m_VecFlocking[i]->GetPos() - GetPos();
		Direction.y = 0.f;
		float fSumRadius = fScanRange + m_VecFlocking[ i ]->GetUnitRadius();
		float fLength = D3DXVec3LengthSq( &Direction );
		if( fLength > fSumRadius * fSumRadius ) continue;

		float fSpeed = fSumRadius - sqrtf( fLength );

		D3DXVec3Normalize( &Direction, &Direction );
		m_VecFlocking[i]->Move( &( Direction * fSpeed ) );
		m_VecFlocking[i]->SetFlocking( true, fSumRadius );
	}
}