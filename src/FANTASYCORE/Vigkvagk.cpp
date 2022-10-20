#include "StdAfx.h"
#include "Vigkvagk.h"
#include "FcWorld.h"
#include "FcCameraObject.h"
#include "FcPropManager.h"
#include "FcProp.h"
#include "InputPad.h"
#include "FcHeroSeqEvent.h"
#include "Data/SignalType.h"
#include "FcProjectile.h"
#include "FcUtil.h"
#include "FcTroopObject.h"
#include "FcGlobal.h"
#include "./Data/FXList.h"
#include "FcSoundManager.h"
#include "FcAdjutantObject.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


int CVigkvagk::s_nRotationAngle = 30;
ASSignalData *CVigkvagk::s_pHitSignal = NULL;
int CVigkvagk::s_nRefCount = 0;

CVigkvagk::CVigkvagk( CCrossVector *pCross )
: CFcHeroObject( pCross )
{
	SetHeroClassID( Class_ID_Hero_VigkVagk );
	m_bBackAttack = false;
	m_bApplyDamage = false;
//	m_nRotationAngle = -1;
	m_nRotationAngle = s_nRotationAngle;

	if( s_pHitSignal == NULL ) {
		s_pHitSignal = new ASSignalData;
		s_pHitSignal->m_nID = SIGNAL_TYPE_HIT_TEMP;
		s_pHitSignal->m_NumParam = HIT2_SIGNAL_LENGTH;
		s_pHitSignal->m_pParam = new int[s_pHitSignal->m_NumParam];

		memset( s_pHitSignal->m_pParam, 0, sizeof(int) * s_pHitSignal->m_NumParam );

		s_pHitSignal->m_Hit2AniType = ANI_TYPE_DOWN;
		s_pHitSignal->m_Hit2AttackPoint = 0;
		s_pHitSignal->m_Hit2Length = 20;
		s_pHitSignal->m_Hit2AniIndex = 0;
		s_pHitSignal->m_Hit2PushY = 20;
		s_pHitSignal->m_Hit2PushZ = -10;
		s_pHitSignal->m_Hit2SoundAction = ATT_NONE;
		s_pHitSignal->m_Hit2WeaponMaterial = WT_VIGKVAGK;
	}
	++s_nRefCount;
}


CVigkvagk::~CVigkvagk()
{
	--s_nRefCount;

	BsAssert( s_nRefCount >= 0 );
	if( s_nRefCount == 0 ) {
		if( s_pHitSignal ) {
			delete s_pHitSignal;
			s_pHitSignal = NULL;
		}
	}
}

void CVigkvagk::Initialize( INITGAMEOBJECTDATA* pData, int nForce, int nTeam, TroopObjHandle hTroop, float fScaleMin, float fScaleMax )
{
	CFcHeroObject::Initialize( pData, nForce, nTeam, hTroop, fScaleMin, fScaleMax );

	for( DWORD i=0; i<m_WeaponList.size(); i++ ) {
		m_WeaponList[i].bShow = false;
		if( m_WeaponList[i].nObjectIndex == -1 ) continue;
		g_BsKernel.ShowObject( m_WeaponList[i].nObjectIndex, false );
	}
	m_nCurrentWeaponIndex = -1;

}


void CVigkvagk::ProcessFlocking()
{
	if( IsJump() ) return;
//	m_nFlockingInterval++;
	float fScanRange = GetUnitRadius() * 3;
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
		float fLength = D3DXVec3Length( &Direction );
		if( fLength > fSumRadius ) continue;

		if( m_VecFlocking[i]->IsDown() ) {
			if( m_VecFlocking[i]->IsHittable( m_Handle ) && IsMove() ) {
				HIT_PARAM HitParam;

				HitParam.ObjectHandle = m_Handle;
				HitParam.nAttackPower = 0;
				HitParam.pHitSignal = s_pHitSignal;
				HitParam.Position = m_Cross.m_PosVector;
				HitParam.nHitRemainFrame = s_pHitSignal->m_Hit2Length;
				HitParam.fVelocityY = (float)s_pHitSignal->m_Hit2PushY;
				HitParam.fVelocityZ = (float)s_pHitSignal->m_Hit2PushZ;

				m_VecFlocking[i]->CmdHit( &HitParam );
			}
		}
		else {
			float fSpeed = ( fSumRadius ) - fLength;

			D3DXVec3Normalize( &Direction, &Direction );
			m_VecFlocking[i]->Move( &( Direction * fSpeed ) );

			m_VecFlocking[i]->SetFlocking( true, fSumRadius );
		}
	}
}

void CVigkvagk::RotationProcess()
{
	float fLength;
	D3DXVECTOR3 CrossVec;

	if( m_bDead ) return;
	fLength = D3DXVec3LengthSq(&m_MoveDir);
	if( fLength <= 0.0f ) {
		if( m_nCurAniType == ANI_TYPE_CUSTOM_1 ) {
			int nFindAni = FindExactAni( m_pUnitInfoData->GetAniIndex( ANI_TYPE_STAND, 0 ) );
			ChangeAnimationByIndex( nFindAni );
		}
		return;
	}

	fLength = D3DXVec3LengthSq( &m_MoveTargetPos );
	if( fLength > 0.f ) {
		D3DXVECTOR3 vTargetVec = m_MoveTargetPos - GetPos();
		vTargetVec.y = 0.f;
		D3DXVec3Normalize( &vTargetVec, &vTargetVec );
		m_MoveDir = vTargetVec;
		BsAssert( (m_MoveDir.x == m_MoveDir.x) && (m_MoveDir.y == m_MoveDir.y) && (m_MoveDir.z == m_MoveDir.z) );
	}

	float fDot=D3DXVec3Dot(&m_MoveDir, &(m_Cross.m_ZVector));
	float fAngle = acos( fDot ) * 180.f / 3.1415926f;

	float fMinAngle = m_nRotationAngle / 2.84f;
	if( fAngle > fMinAngle ){
		D3DXVec3Cross(&CrossVec, &m_MoveDir, &(m_Cross.m_ZVector));
		if(CrossVec.y>0) {
			if( m_nCurAniType == ANI_TYPE_STAND || m_nCurAniType == ANI_TYPE_CUSTOM_1 ) {
//				ChangeAnimation( ANI_TYPE_CUSTOM_1, 0 );
			}

			m_Cross.RotateYaw( m_nRotationAngle );
		}
		else {
			if( m_nCurAniType == ANI_TYPE_STAND || m_nCurAniType == ANI_TYPE_CUSTOM_1 ) {
//				ChangeAnimation( ANI_TYPE_CUSTOM_1, 1 );
			}

			m_Cross.RotateYaw( -m_nRotationAngle );
		}

		float fPrevAngle = fAngle;
		fDot=D3DXVec3Dot(&m_MoveDir, &(m_Cross.m_ZVector));
		fAngle = acos( fDot ) * 180.f / 3.1415926f;
		if( fAngle <= fMinAngle ) {
			m_Cross.m_ZVector=m_MoveDir;
			m_Cross.UpdateVectors();
			m_MoveDir=D3DXVECTOR3(0.0f, 0.0f, 0.0f);

			if( m_nCurAniType == ANI_TYPE_CUSTOM_1 ) {
				int nFindAni = FindExactAni( m_pUnitInfoData->GetAniIndex( ANI_TYPE_STAND, 0 ) );
				ChangeAnimationByIndex( nFindAni );
			}
		}	
		return;
	}

	m_Cross.m_ZVector=m_MoveDir;
	m_Cross.UpdateVectors();
	m_MoveDir=D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	if( m_nCurAniType == ANI_TYPE_CUSTOM_1 ) {
		int nFindAni = FindExactAni( m_pUnitInfoData->GetAniIndex( ANI_TYPE_STAND, 0 ) );
		if( m_fHP == 0 ) {
			int asdf =0;
		}
		ChangeAnimationByIndex( nFindAni );
	}
}

void CVigkvagk::MoveZ(float fDist)
{
	m_MoveVector+=m_Cross.m_ZVector*fDist; 
}


void CVigkvagk::Process()
{
	CFcHeroObject::Process();

	if( !IsMove() && m_pEventSeq->GetEventCount() == 0 && m_nCurAniType != ANI_TYPE_CUSTOM_1 && m_nCurAniType != ANI_TYPE_ATTACK ) m_MoveDir = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	if( g_FcWorld.IsProcessTrueOrb( GetTeam() ) ) {
		if( m_fHP <= 0.f && m_nCurAniType != ANI_TYPE_DIE ) {
			m_bDead = true;
			ChangeAnimation( ANI_TYPE_DIE, -1 );
		}
		else {
			//AddHP( -1 );
			//if( ( GetProcessTick() % (FRAME_PER_SEC / 2)) == 0){		
				//g_pSoundManager->PlaySound(SB_COMMON,"HIT_G_SWING_S");
			//}
		}
	}
}

/*
void CVigkvagk::ProcessMove()
{
	switch( m_nCurAniType ) {
		case ANI_TYPE_WALK:
		case ANI_TYPE_RUN:
			{
				if( D3DXVec3LengthSq( &m_MoveTargetPos ) > 0.f ) {
					D3DXVECTOR3 vPos = GetDummyPos() - m_MoveTargetPos;
					vPos.y = 0.f;

					float fLength = D3DXVec3Length( &vPos );

					float fDistance = 0.f;
					if( m_bApplyFlocking == false ) fDistance = m_fMoveSpeed;
					else {
						fDistance = m_fMaxFlockingSize;
					}
					fDistance += 5.f;	// 아씨..이상하게 안스네..

					if( fLength <= fDistance ) {
						CmdStop( ANI_TYPE_STAND | GetCurAniAttr(), -1 );
						break;
					}
				}
				MoveZ( m_fMoveSpeed );
			}
			break;
	}
}
*/

void CVigkvagk::GetExactHitVelocity( float &fVelocityZ, float &fVelocityY )
{
	fVelocityZ = 0.0f;
	fVelocityY = 0.0f;
}

void CVigkvagk::ProcessKeyEvent(int nKeyCode, KEY_EVENT_PARAM *pKeyParam)
{
	if( nKeyCode == PAD_INPUT_EVENT_RESTORE_KEY ) {
		ResetKeyProcess();
		if( IsMove() ) CmdStop();
		return;
	}

	if( m_nPlayerIndex != -1 ) {
		CameraObjHandle CamHandle = CFcBaseObject::GetCameraObjectHandle( m_nPlayerIndex );
		if (CamHandle->IsEditMode() || CamHandle->IsFreeCamMode()) return;
	}

	bool bRelease=false;

	if(nKeyCode&0x80000000){
		bRelease=true;
	}

	if( !IsOrbSparkOn() )
	{
		if( !bRelease && ( nKeyCode & 0x7fffffff ) == PAD_INPUT_Y ) {
			if( ( m_nCurAniType == ANI_TYPE_STAND && m_nCurAniTypeIndex == 0 ) ||
				( m_nCurAniType == ANI_TYPE_WALK && m_nCurAniTypeIndex == 0 ) ||
				( m_nCurAniType == ANI_TYPE_RUN && m_nCurAniTypeIndex == 0 ) ||
				( m_nCurAniType == ANI_TYPE_CUSTOM_1 ) ) {
				if( PickupWeapon() == true ) return;
			}
		}
	}

	CFcHeroObject::ProcessKeyEvent( nKeyCode, pKeyParam );
}

bool CVigkvagk::PickupWeapon()
{
	if( m_pEventSeq->GetEventCount() != 0 ) return true;

	float fRange = 700.f;

	if( m_nPlayerIndex != -1 ) {
		std::vector< GameObjHandle > VecObjList;
		g_FcWorld.GetEnemyObjectListInRange( m_nTeam, &m_Cross.m_PosVector, 300.f, VecObjList );
		if( VecObjList.size() > 0 ) return false;
	}
	////////////////////////////////////////////////

	CFcPropManager *pPropMng = g_FcWorld.GetPropManager();
	if( pPropMng == NULL ) return false;
//	if( m_nCurAniType != ANI_TYPE_STAND ) return false;
//	if( m_nCurAniTypeIndex != 0 ) return false;

	std::vector< CFcProp * > pVecList;
	pPropMng->GetObjectListInRange( &m_Cross.m_PosVector, fRange, pVecList );
	if( pVecList.empty() ) return false;

	CFcProp *pTargetProp = NULL;
	float fNearestRange = fRange * fRange;
	float fTemp;
	for( DWORD i=0; i<pVecList.size(); i++ ) {
		if( !pVecList[i]->IsPickUpProp() || CheckLevel( pVecList[i] ) == false ) {
			pVecList.erase( pVecList.begin() + i );
			--i;
			continue;
		}
		fTemp = D3DXVec2LengthSq( &( GetPosV2() - pVecList[i]->GetPosV2() ) );
		if( fTemp < fNearestRange || !pTargetProp ) {
			fNearestRange = fTemp;
			pTargetProp = pVecList[i];
		}
	}

	if( !pTargetProp ) return false;
	if( !pTargetProp->IsPickUpProp() ) return false;

	CFcHeroSeqPickupEvent *pElement = new CFcHeroSeqPickupEvent( m_pEventSeq );
	pElement->m_nTime = -1;
	pElement->m_pProp = pTargetProp;
	m_pEventSeq->AddEvent( pElement );

	return true;
}

bool CVigkvagk::CheckLevel( CFcProp *pProp )
{
	if( m_nPlayerIndex == -1 ) return true;
	PROPTYPEDATA *pData = g_FcWorld.GetPropManager()->GetPropTypeData_( pProp->GetTypeDataIndex() );
	if( strcmp( pData->cName, "P_PL_ET13.SKIN" ) == NULL && m_nLevel < 1 ) return false;
	if( strcmp( pData->cName, "P_PL_ET14.SKIN" ) == NULL && m_nLevel < 2 ) return false;
	if( strcmp( pData->cName, "P_MP_TR09.SKIN" ) == NULL && m_nLevel < 3 ) return false;

	return true;
}

void CVigkvagk::SignalCheck( ASSignalData *pSignal )
{
 	switch( pSignal->m_nID ) {
		case SIGNAL_TYPE_SHOOT_PROJECTILE:
			if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
			{
				PROJECTILE_PARAM Param;
				ProjetileHandle Handle;
				CCrossVector Cross;
				D3DXVECTOR3 Position;
				ASPartsData *pPartsData;

				D3DXMATRIX matWeapon;
				int nWeaponIndex = m_WeaponList[ pSignal->m_pParam[0] ].nObjectIndex;
				if( m_WeaponList[ pSignal->m_pParam[0] ].bShow == false ) break;

				matWeapon = *( D3DXMATRIX * )g_BsKernel.SendMessage( nWeaponIndex, BS_GET_OBJECT_MAT );
				memcpy( &Cross.m_XVector, &matWeapon._11, sizeof( D3DXVECTOR3 ) );
				memcpy( &Cross.m_YVector, &matWeapon._21, sizeof( D3DXVECTOR3 ) );
				memcpy( &Cross.m_ZVector, &matWeapon._31, sizeof( D3DXVECTOR3 ) );
				memcpy( &Cross.m_PosVector, &matWeapon._41, sizeof( D3DXVECTOR3 ) );

				/*
				Cross = m_Cross;
				Position.x = ( float )pSignal->m_pParam[ 1 ];
				Position.y = ( float )pSignal->m_pParam[ 2 ];
				Position.z = ( float )pSignal->m_pParam[ 3 ];
				D3DXVec3TransformNormal( &Position, &Position, m_Cross );
				Cross.m_PosVector += Position;
				*/

				Param.pSignal = pSignal;
				Param.hParent = m_Handle;
				Param.vTarget = m_Cross.m_ZVector;
				D3DXVec3Normalize( &Param.vTarget, &Param.vTarget );
				switch( nWeaponIndex ) {
					case 0:
						Param.vTarget *= 30.f;
						break;
					case 1:
					case 2:
						Param.vTarget *= 45.f;
						break;
				}

				pPartsData = m_pUnitInfoData->GetPartsGroupInfo( m_WeaponList[ pSignal->m_pParam[ 0 ] ].nPartsIndex );
				Handle = CFcProjectile::CreateObject( CFcProjectile::PHYSICS_THROW, &Cross );
				Handle->Initialize( pPartsData->m_pSkinIndex[0], &Param );
			}
			return;
		case SIGNAL_TYPE_CUSTOM_MESSAGE:
			if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
			{
				switch( pSignal->m_pParam[0] ) {
					case 1:	// 회전
//						m_nRotationAngle = abs( pSignal->m_pParam[1] );
						break;
					case 2: // Damage Check
						m_bApplyDamage = ( pSignal->m_pParam[1] == 1 );
						break;
					case 3:	// 뷰벡터 변경
						m_Cross.RotateYaw( (int)( pSignal->m_pParam[1] * 2.84f ) );
						m_MoveTargetPos = D3DXVECTOR3( 0.f, 0.f, 0.f );
						m_MoveDir = D3DXVECTOR3( 0.f, 0.f, 0.f );
						NextAnimation();
						break;
				}
			}
			return;
	}
	CFcHeroObject::SignalCheck( pSignal );
}

void CVigkvagk::CmdMove( int nX, int nY, float fSpeed, int nAniType, int nAniIndex )
{
	if( m_nUnitSOXIndex == 5 ) {
		int nFindAni = FindExactAni( m_pUnitInfoData->GetAniIndex( nAniType, nAniIndex ) );
		CFcHeroObject::CmdMove( nX, nY, fSpeed, m_pUnitInfoData->GetAniType( nFindAni ), m_pUnitInfoData->GetAniTypeIndex( nFindAni ) );
	}
	else CFcUnitObject::CmdMove( nX, nY, fSpeed, nAniType, nAniIndex );
}

void CVigkvagk::CmdStop( int nAniType, int nAniIndex )
{
	if( m_nUnitSOXIndex == 5 ) {
		int nFindAni = FindExactAni( m_pUnitInfoData->GetAniIndex( nAniType, nAniIndex ) );
		CFcHeroObject::CmdStop( m_pUnitInfoData->GetAniType( nFindAni ), m_pUnitInfoData->GetAniTypeIndex( nFindAni ) ); 
	}
	else CFcUnitObject::CmdStop( nAniType, nAniIndex );
}

int CVigkvagk::FindExactAni( int nAniIndex )
{
	int nAniType = m_pUnitInfoData->GetAniType(nAniIndex);
	int nAniTypeIndex = m_pUnitInfoData->GetAniTypeIndex(nAniIndex);
	int nAniAttr = m_pUnitInfoData->GetAniAttr(nAniIndex);

	switch( nAniType ) {
		case ANI_TYPE_STAND:
			if( m_WeaponList[0].bShow ) return m_pUnitInfoData->GetAniIndex( ANI_TYPE_STAND, 4 );
			else if( m_WeaponList[1].bShow || m_WeaponList[2].bShow ) return m_pUnitInfoData->GetAniIndex( ANI_TYPE_STAND, 5 );
			else if( m_LeftCatchHandle ) return m_pUnitInfoData->GetAniIndex( ANI_TYPE_STAND, 8 );
			else if( m_CatchHandle ) return m_pUnitInfoData->GetAniIndex( ANI_TYPE_STAND, 6 );
			else return m_pUnitInfoData->GetAniIndex( ANI_TYPE_STAND, 0 );

			break;
		case ANI_TYPE_WALK:
			if( GetPlayerIndex() == -1 ) m_nRotationAngle = s_nRotationAngle;
			else m_nRotationAngle = 30;

			if( m_WeaponList[0].bShow ) return m_pUnitInfoData->GetAniIndex( ANI_TYPE_WALK, 1 );
			else if( m_WeaponList[1].bShow || m_WeaponList[2].bShow ) return m_pUnitInfoData->GetAniIndex( ANI_TYPE_WALK, 2 );
			else if( m_LeftCatchHandle ) return m_pUnitInfoData->GetAniIndex( ANI_TYPE_WALK, 4 );
			else if( m_CatchHandle ) return m_pUnitInfoData->GetAniIndex( ANI_TYPE_WALK, 3 );
			else return m_pUnitInfoData->GetAniIndex( ANI_TYPE_WALK, 0 );

			break;
		case ANI_TYPE_RUN:
			if( GetPlayerIndex() == -1 ) m_nRotationAngle = s_nRotationAngle;
			else m_nRotationAngle = 30;

			if( m_WeaponList[0].bShow ) return m_pUnitInfoData->GetAniIndex( ANI_TYPE_WALK, 1 );
			else if( m_WeaponList[1].bShow || m_WeaponList[2].bShow ) return m_pUnitInfoData->GetAniIndex( ANI_TYPE_RUN, 1 );
			else if( m_LeftCatchHandle ) return m_pUnitInfoData->GetAniIndex( ANI_TYPE_RUN, 3 );
			else if( m_CatchHandle ) return m_pUnitInfoData->GetAniIndex( ANI_TYPE_RUN, 2 );
			else return m_pUnitInfoData->GetAniIndex( ANI_TYPE_RUN, 0 );

			break;
		case ANI_TYPE_SPECIAL_ATTACK:
			if( nAniTypeIndex != 0 && nAniTypeIndex != 1 ) break;
			switch( nAniTypeIndex ) {
				case 0:	//발동
					if( m_WeaponList[0].bShow ) return m_pUnitInfoData->GetAniIndex( ANI_TYPE_SPECIAL_ATTACK, 5 );
					else if( m_WeaponList[1].bShow || m_WeaponList[2].bShow ) return m_pUnitInfoData->GetAniIndex( ANI_TYPE_SPECIAL_ATTACK, 4 );
					else return m_pUnitInfoData->GetAniIndex( ANI_TYPE_SPECIAL_ATTACK, 0 );
					break;
				case 1:	//끝
					if( m_WeaponList[0].bShow ) return m_pUnitInfoData->GetAniIndex( ANI_TYPE_SPECIAL_ATTACK, 6 );
					else if( m_WeaponList[1].bShow || m_WeaponList[2].bShow ) return m_pUnitInfoData->GetAniIndex( ANI_TYPE_SPECIAL_ATTACK, 7 );
					else return m_pUnitInfoData->GetAniIndex( ANI_TYPE_SPECIAL_ATTACK, 1 );
					break;
			}
			break;

	}

	return nAniIndex;
}

bool CVigkvagk::GetExactHitAni( int &nAniType, int &nAniIndex )
{
	/*
	if( nAniType == ANI_TYPE_HIT )
	{
		return false;
	}
	if( nAniType == ANI_TYPE_DOWN )
	{
		if( nAniIndex == 2 )
		{
			nAniIndex = 0;
		}
		else
		{
			nAniType = ANI_TYPE_HIT;
			if( m_WeaponList[0].bShow ) nAniIndex = 1;
			else nAniIndex = 0;
		}

		if( m_bBackAttack == true ) nAniType |= ANI_ATTR_BACK;

	}

	*/

	/*
	switch( nAniType ) {
		case ANI_TYPE_HIT:
			if( m_WeaponList[0].bShow ) nAniIndex = 1;
			break;
		case ANI_TYPE_DOWN:
			nAniIndex = Random(2);
			break;
	}
	*/

	if( nAniType == ANI_TYPE_DIE || m_bDead || m_fHP <= 0.f ) {
		nAniType = ANI_TYPE_DIE;
		if( m_bBackAttack == true ) nAniType |= ANI_ATTR_BACK;
		nAniIndex = -1;
		return true;
	}
	if( m_bApplyDamage == false ) return false;

	switch( nAniType ) {
		case ANI_TYPE_HIT:
			/*
			switch( nAniIndex ) {
				case 0:
				case 1:
					nAniIndex = 0;
					if( m_WeaponList[0].bShow ) nAniIndex = 1;
					break;
				case 2:
					nAniIndex = 4 + Random(2);
					break;
			}
			*/
			switch( Random(3) ) {
				case 0:
					nAniIndex = 0;
					if( m_WeaponList[0].bShow ) nAniIndex = 1;
					break;
				case 1:
				case 2:
					nAniIndex = 4 + Random(2);
					break;
			}
			break;
		case ANI_TYPE_DOWN:
			nAniIndex = Random(2);
			break;
	}
	if( m_bBackAttack == true ) nAniType |= ANI_ATTR_BACK;

	return true;
}


void CVigkvagk::LookAtHitUnit( D3DXVECTOR3 *pDirection )
{
	float fDot = D3DXVec3Dot( pDirection, &m_Cross.m_ZVector );
	if( fDot > 0 ) m_bBackAttack = false;
	else m_bBackAttack = true;
	return;
}

bool CVigkvagk::CheckDefense()
{
	if( m_bDead ) return false;
	if( GetPlayerIndex() == -1 ) {
		if( !m_HitParam.ObjectHandle ) return false;

		// 진오브일 경우 걍 무시!!
		if( g_FcWorld.IsProcessTrueOrb( GetTeam() ) ) return true;

		if( m_HitParam.ObjectHandle->IsGuardBreak( m_Handle ) || Random(100) < m_HitParam.nGuardBreak ) 
			return false;
//		bool bResult = false;
		if( m_nCurAniType == ANI_TYPE_DEFENSE ) return true;
		/*
			int nKindOf = ( m_nCurAniTypeIndex + 1 ) / 5;
			int nIndex = m_nCurAniTypeIndex % 5;
			ChangeAnimation( ANI_TYPE_DEFENSE, ( nKindOf * 5 ) + 3, ANI_ATTR_NONE, true );
			bResult = true;
		}
		return bResult;
		*/
		return false;
	}
	else {
		if( m_nCurAniType != ANI_TYPE_DEFENSE ) return false;

		int nKindOf = ( m_nCurAniTypeIndex + 1 ) / 5;
		int nIndex = m_nCurAniTypeIndex % 5;
		switch( nIndex ) {
			case 1:	// 루프
//				ChangeAnimation( ANI_TYPE_DEFENSE, ( nKindOf * 5 ) + 3, ANI_ATTR_NONE, false );
				return true;
			case 3:	// 타격
				return true;
		}
		return false;
	}
	return false;
//	return CFcHeroObject::CheckDefense();
}

bool CVigkvagk::CalculateDamage( int nAttackPower )
{
	if( g_FCGameData.bOneShotMode == true || m_bApplyDamage == true ) return CFcHeroObject::CalculateDamage( nAttackPower );


	/*
	if( m_HitParam.ObjectHandle->GetClassID() == Class_ID_Hero ) {
		return CFcHeroObject::CalculateDamage( nAttackPower );
	}
	*/

	int nDamage = 0;
	if( m_nCurAniType == ANI_TYPE_HIT || m_nCurAniType == ANI_TYPE_DOWN ) {
		if( m_HitParam.pHitSignal->m_HitAttackPoint >= 100 ) {
			nDamage = RandomNumberInRange(30,50);
		}
		else {
			nDamage = RandomNumberInRange(10,30);
		}
	}
	else if( m_nCurAniType != ANI_TYPE_DEFENSE ) {
		if( m_HitParam.pHitSignal->m_HitAttackPoint >= 100 ) {
			nDamage = RandomNumberInRange(10,20);
		}
		else {
			nDamage = RandomNumberInRange(5,10);
		}
		//g_pSoundManager->PlaySound(SB_COMMON,"HIT_G_SWING_S");
	}

	nDamage += GetDefense();
	return CFcHeroObject::CalculateDamage( nDamage );
}

void CVigkvagk::CalculateParticlePosition( CCrossVector &Cross )
{
	CFcGameObject::CalculateParticlePosition( Cross );
	Cross.MoveFrontBack( 50.f );
}

bool CVigkvagk::Catch( GameObjHandle Handle )
{
	if( m_nAniIndex == 106 )
	{
		return CatchLeftHand( Handle );
	}
	else
	{
		return CatchRightHand( Handle );
	}
}

bool CVigkvagk::CatchLeftHand( GameObjHandle Handle )
{
	if( m_LeftCatchHandle )
	{
		return false;
	}
	if( Handle->IsDie() ) return false;

	g_BsKernel.SetPreCalcAni( m_nEngineIndex, true);

	Handle->RideOut();

	m_LeftCatchHandle = Handle;
	m_LeftCatchHandle->RunRagdoll( 1 );
	g_BsKernel.SendMessage( m_LeftCatchHandle->GetEngineIndex(), BS_PHYSICS_SIMULATION, TRUE );

	m_LeftCatchHandle->ChangeAnimation( ANI_TYPE_DOWN, 0 );	// Ragdoll 상태일때 AI 못돌게 할려고..
	switch( m_nAniIndex )
	{
	case 106:	// 오른손 인간잡기
		ChangeAnimationByIndex( 116 );
		m_nCatchBoneIndex = g_BsKernel.SendMessage( m_LeftCatchHandle->GetEngineIndex(), BS_GET_BONE_INDEX, ( DWORD )"Bip01 R Foot" );
		break;
	}

	return true;
}

bool CVigkvagk::CatchRightHand( GameObjHandle Handle )
{
	if( m_CatchHandle )
	{
		return false;
	}
	if( Handle->IsDie() ) return false;

	g_BsKernel.SetPreCalcAni( m_nEngineIndex, true);

	Handle->RideOut();

	m_CatchHandle = Handle;
	m_CatchHandle->RunRagdoll( 1 );
	g_BsKernel.SendMessage( m_CatchHandle->GetEngineIndex(), BS_PHYSICS_SIMULATION, TRUE );

	m_CatchHandle->ChangeAnimation( ANI_TYPE_DOWN, 0 );	// Ragdoll 상태일때 AI 못돌게 할려고..
	switch( m_nAniIndex )
	{
	case 90:	// 오른손 인간잡기
		ChangeAnimationByIndex( 98 );
		m_nCatchBoneIndex = g_BsKernel.SendMessage( m_CatchHandle->GetEngineIndex(), BS_GET_BONE_INDEX, ( DWORD )"Bip01 R Foot" );
		break;
	}

	return true;
}

void CVigkvagk::UnlinkCatchObject( int nX, int nY, int nZ, bool bEvent /*= false*/, int nHandIndex /*= -1*/ )
{
	if( bEvent )
	{
		return;
	}
	if( ( nHandIndex == -1 ) || ( nHandIndex == 1 ) )
	{
		if( m_LeftCatchHandle )
		{
			ProcessUnlink( m_LeftCatchHandle, nX, nY, nZ );
		}
	}

	CFcHeroObject::UnlinkCatchObject( nX, nY, nZ, bEvent, nHandIndex );
}

void CVigkvagk::SendCatchMsg()
{
	int nBone;
	if( m_nAniIndex == 116 )	// 왼손잡기..
	{
		if( m_LeftCatchHandle )
		{
			m_LeftCatchHandle->SetCatchedHero( true );
			DebugString("CVigkvagk::Render() Left Catch : LinkObject(%d)\n", m_LeftCatchHandle->GetEngineIndex() );
			nBone = g_BsKernel.SendMessage( m_nEngineIndex, BS_GET_BONE_INDEX, ( DWORD )"Dummy_WP_L" );
			g_BsKernel.SendMessage( m_nEngineIndex, BS_LINKOBJECT, nBone, m_LeftCatchHandle->GetEngineIndex() );
			g_BsKernel.SendMessage( m_LeftCatchHandle->GetEngineIndex(), BS_SET_RAGDOLL_CATCH_BONE, m_nCatchBoneIndex );
		}
	}
	else
	{
		if( m_CatchHandle )
		{
			m_CatchHandle->SetCatchedHero( true );
			DebugString("CVigkvagk::Render() Right Catch : LinkObject(%d)\n", m_CatchHandle->GetEngineIndex() );
			nBone = g_BsKernel.SendMessage( m_nEngineIndex, BS_GET_BONE_INDEX, ( DWORD )"Dummy_WP_R" );
			g_BsKernel.SendMessage( m_nEngineIndex, BS_LINKOBJECT, nBone, m_CatchHandle->GetEngineIndex() );
			g_BsKernel.SendMessage( m_CatchHandle->GetEngineIndex(), BS_SET_RAGDOLL_CATCH_BONE, m_nCatchBoneIndex );
		}
	}
}

void CVigkvagk::SendUnlinkCatchMsg( int nHandIndex /*= -1*/ )
{
	if( ( m_CatchHandle ) && ( nHandIndex != 1 ) )
	{
		m_CatchHandle->SetCatchedHero( false );
		DebugString("CVigkvagk::Render() Right Uncatch : UnLinkObject(%d)\n", m_CatchHandle->GetEngineIndex());
		g_BsKernel.SendMessage( m_nEngineIndex, BS_UNLINKOBJECT, m_CatchHandle->GetEngineIndex(), ( DWORD )&m_UnlinkVelocity );
		m_CatchHandle->UnlinkRagdoll();
		m_CatchHandle.Identity();
	}

	if( ( m_LeftCatchHandle ) && ( nHandIndex != 0 ) )
	{
		m_LeftCatchHandle->SetCatchedHero( false );
		DebugString("CVigkvagk::Render() Left Uncatch : UnLinkObject(%d)\n", m_LeftCatchHandle->GetEngineIndex());
		g_BsKernel.SendMessage( m_nEngineIndex, BS_UNLINKOBJECT, m_LeftCatchHandle->GetEngineIndex(), ( DWORD )&m_UnlinkVelocity );
		m_LeftCatchHandle->UnlinkRagdoll();
		m_LeftCatchHandle.Identity();
	}
}

bool CVigkvagk::IsFlockable( GameObjHandle Handle )
{
	if( Handle == m_Handle ) return false;
	if( Handle->GetParentLinkHandle() == m_Handle ) return false;

	switch( Handle->GetClassID() ) {
		case Class_ID_Hero:
			if( IsJump() ) return false;
			switch( ((CFcHeroObject*)Handle.GetPointer())->GetHeroClassID() ) {
				case Class_ID_Hero_VigkVagk:	
					if( Handle->IsJump() ) return false;
					return true;
				default:	return false;
			}
			return false;
		case Class_ID_Catapult: return true;
		case Class_ID_Adjutant:
			switch( ((CFcAdjutantObject*)Handle.GetPointer())->GetAdjutantClassID() ) {
				case Class_ID_Adjutant_99King:	return true;
				default:	return false;
			}
			return false;
	}
	return false;
}

float CVigkvagk::GetFrameAdd()
{
	if( ( m_bOrbSpark ) && ( m_nCurAniType == ANI_TYPE_WALK ) )
	{
		return 1.5f;
	}
	return CFcHeroObject::GetFrameAdd();
}