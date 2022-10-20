#include "stdafx.h"
#include "FcGuardianManager.h"
#include "FcTroopObject.h"
#include "FcHeroObject.h"
#include "SmartPtr.h"
#include "FcWorld.h"
#include "FcGlobal.h"
#include "FcTroopAIObject.h"
#include "FcTroopManager.h"
#include "FcCameraObject.h"





CFcGuardianManager::CFcGuardianManager( CFcTroopManager* pManager )
{
	m_CmdType = GUARDIAN_CMDTYPE_DEFENSE;
	m_pTroopManager = pManager;

	m_bCmdDefenseMove[0] = m_bCmdDefenseMove[1] = false;			
	m_nDefenseAddRate[0] = m_nDefenseAddRate[1] = 0;
	m_nAttackAddRate[0] = m_nAttackAddRate[1] = 0;
	m_bUpdateProperty[0] = m_bUpdateProperty[1] = false;
	m_bEnable[0] = m_bEnable[1] = true;

	m_nDefenseAdd = 0;
	m_nAttackAdd = 0;
	m_nAddMoveSpeedRate = 0;
	m_nAddDamageTrap = 0;
}

CFcGuardianManager::~CFcGuardianManager()
{

}


void CFcGuardianManager::Process()
{
	if( m_CmdType == GUARDIAN_CMDTYPE_DEFENSE )
	{
		DefenseProcess();
	}
	else
	{
		AttackProcess();
	}

	for( int i=0; i<2; i++ )
	{
		if( m_hTroop[i] == NULL )
			continue;

		if( !m_hTroop[i]->IsEnable() )
			continue;

		if( m_hTroop[i]->IsEliminated() )
			m_hTroop[i].Identity();

		if( m_hCmdAttackTarget[i] )
		{
			if( m_hCmdAttackTarget[i]->IsEliminated() )
				m_hCmdAttackTarget[i].Identity();
		}
	}

	if( (GetProcessTick() % FRAME_PER_SEC) == 15 )
	{
		CheckGuardianTarget();
		
	}
	CheckLockOnTarget();
}

void CFcGuardianManager::AddGuardian( TroopObjHandle hTroop )
{
	int i = 0;

	for( i=0; i<2; i++ )
	{
		if( m_hTroop[i] == hTroop )
			return;
	}

	int nIndex = -1;
	for( i=0; i<2; i++ )
	{
		if( m_hTroop[i] == NULL )
		{
			nIndex = i;
			break;
		}
	}
	if( nIndex != -1 )
	{
		m_hTroop[i] = hTroop;
		CFcTroopAIObject::SetEnableAI( hTroop, false );
		hTroop->SetGuardian( true );
		m_bUpdateProperty[i] = false;

		// 이 함수는 초기화 할때만 사용
		// 여기서 SetGuardianPropertyTable는 안한다. (PostInitialize에서 처리)
	}
}


bool CFcGuardianManager::AddGuardian( int nIndex )
{
	if( m_GuardianTarget == NULL )
		return false;

	BsAssert( nIndex < 2 && nIndex >= 0 );
	BsAssert( m_hTroop[nIndex] == NULL );

	for( int i=0; i<2; i++ )
	{
		if( m_hTroop[i] == m_GuardianTarget )
			return false;
	}
	m_hTroop[nIndex] = m_GuardianTarget;
	CFcTroopAIObject::SetEnableAI( m_GuardianTarget, false );
	m_GuardianTarget->SetGuardian( true );
	m_bUpdateProperty[nIndex] = false;

	SetGuardianPropertyTable();

	return true;
}

void CFcGuardianManager::AddGuardian()
{
	int i = 0;

	if( m_GuardianTarget == NULL )
		return;

	for( i=0; i<2; i++ )
	{
		if( m_hTroop[i] == m_GuardianTarget )
			return;
	}

	int nIndex = -1;
	for( i=0; i<2; i++ )
	{
		if( m_hTroop[i] == NULL )
		{
			nIndex = i;
			break;
		}
	}
	if( nIndex != -1 )
	{
		m_hTroop[i] = m_GuardianTarget;
		CFcTroopAIObject::SetEnableAI( m_GuardianTarget, false );
		m_GuardianTarget->SetGuardian( true );
		m_bUpdateProperty[i] = false;

		SetGuardianPropertyTable();
	}
}

bool CFcGuardianManager::RemoveGuardian( int nIndex )
{
	if( m_hTroop[nIndex] )
	{
		m_hTroop[nIndex]->CmdStop();
		CFcTroopAIObject::SetEnableAI( m_hTroop[nIndex], true );
	}
	else
		return false;

	int nRate = m_hTroop[nIndex]->GetDefenseAdd();
	m_hTroop[nIndex]->SetDefenseAdd( -nRate );

	nRate = m_hTroop[nIndex]->GetAttackAdd();
	m_hTroop[nIndex]->SetAttackAdd( -nRate );

	nRate = m_hTroop[nIndex]->GetTrapDamageAdd();
	m_hTroop[nIndex]->SetTrapDamageAdd( -nRate );

	nRate = m_hTroop[nIndex]->GetMaxHPAdd();
	m_hTroop[nIndex]->SetMaxHPAdd( -nRate );

	nRate = m_hTroop[nIndex]->GetMoveSpeedAdd();
	m_hTroop[nIndex]->SetMoveSpeedAdd( -nRate );

	m_hTroop[nIndex]->SetFullHP();
	m_hTroop[nIndex]->SetHP();

	m_hTroop[nIndex]->SetGuardian( false );
	
	m_hTroop[nIndex].Identity();

	return true;
}

int CFcGuardianManager::GetGuardianNum()
{
	int nNum = 0;
	for( int i=0; i<2; i++ )
	{
		if( m_hTroop[i] )
			++nNum;
	}
	return nNum;
}

bool CFcGuardianManager::IsGuardian( TroopObjHandle hTroop )
{
	if( !hTroop )
		return false;

	for( int i=0; i<2; i++ )
	{
		if( m_hTroop[i] == hTroop )
			return true;
	}
	return false;
}

void CFcGuardianManager::CmdAttack()
{
	m_CmdType = GUARDIAN_CMDTYPE_ATTACK;

	if( m_LockOnTarget )
	{
		for( int i=0; i<2; i++ )
		{
			if( m_hTroop[i] )
			{
				m_hCmdAttackTarget[i] = m_LockOnTarget;

				if( m_hTroop[i]->GetType() == TROOPTYPE_RANGE || 
					m_hTroop[i]->GetType() == TROOPTYPE_RANGEMELEE )
				{
					m_hTroop[i]->CmdAttack( m_LockOnTarget, true, true );
				}
				else
				{
					m_hTroop[i]->CmdAttack( m_LockOnTarget, true, true );
				}
			}
		}
	}
	else
	{
		HeroObjHandle hPlayer = g_FcWorld.GetHeroHandle();
		D3DXVECTOR2 Pos = hPlayer->GetPosV2();
		D3DXVECTOR2 Dir = hPlayer->GetTroop()->GetDir();
		D3DXVec2Normalize( &Dir, &Dir );
		Dir = Dir * 4000.f;
		Pos = Pos + Dir;
		for( int i=0; i<2; i++ )
		{
			if( m_hTroop[i] )
				m_hTroop[i]->CmdMove( Pos.x, Pos.y, true );
		}
	}
}

void CFcGuardianManager::CmdDefense()
{
	m_CmdType = GUARDIAN_CMDTYPE_DEFENSE;

	// Hero핸들 변수로 가지고 있는게 낫겠다.
	D3DXVECTOR2 Pos = g_FcWorld.GetHeroHandle()->GetPosV2();
	for( int i=0; i<2; i++ )
	{
		if( m_hTroop[i] )
		{
			m_bCmdDefenseMove[i] = true;
			m_hTroop[i]->CmdRetreat( Pos.x, Pos.y, true );
		}
	}
}

void CFcGuardianManager::SetGuardianEnable( int nIndex, bool bEnable )
{
	if( m_bEnable[nIndex] == true && bEnable == false )
	{
		if( m_hTroop[nIndex] )
		{
			m_hTroop[nIndex]->CmdStop();
		}
	}
	m_bEnable[nIndex] = bEnable;
}

void CFcGuardianManager::SetGuardianPropertyTable()
{
	int nUnitIndex = g_FcWorld.GetHeroHandle()->GetUnitSoxIndex();

	for( int i=0; i<2; i++ )
	{
		if( m_hTroop[i] == NULL )
			continue;

		if( m_bUpdateProperty[i] == true )
			continue;

		m_hTroop[i]->SetAttackAdd( m_nAttackAdd );
		m_hTroop[i]->SetMoveSpeedAdd( m_nAddMoveSpeedRate );
		m_hTroop[i]->SetTrapDamageAdd( m_nAddDamageTrap );

		int nDef = m_hTroop[i]->GetDefense();
		int nDefencePoint = CHeroLevelTableLoader::GetInstance().GetGuardianDefensePoint( nUnitIndex, g_FcWorld.GetHeroHandle()->GetLevel() );
		if( nDef != 0 )
		{
			int nRate = ( nDefencePoint * 100 / nDef ) - 100;
			m_hTroop[i]->SetDefenseAdd( nRate + m_nDefenseAdd );
		}

		int nCurMaxHP = m_hTroop[i]->GetMaxHP();
		int nMaxHP = CHeroLevelTableLoader::GetInstance().GetGuardianMaxHP( nUnitIndex, g_FcWorld.GetHeroHandle()->GetLevel() );
		nMaxHP *= m_hTroop[i]->GetAliveUnitCount();
		if( nCurMaxHP > 0 )
		{
			int nRate = ( nMaxHP * 100 / nCurMaxHP ) - 100;
			m_hTroop[i]->SetMaxHPAdd( nRate );
		}
		m_hTroop[i]->SetFullHP();
		m_hTroop[i]->SetHP();

		m_bUpdateProperty[i] = true;
	}
}

void CFcGuardianManager::EliminateTroop( TroopObjHandle hTroop )
{
	if( m_GuardianTarget == hTroop )
	m_GuardianTarget.Identity();

	if( m_LockOnTarget == hTroop )
	m_LockOnTarget.Identity();
}


void CFcGuardianManager::SetGuardianTroopDefenseAdd( int nRate )
{
	m_nDefenseAdd += nRate;
	for( int i=0; i<2; i++ )
	{
		if( m_hTroop[i] == NULL )
			continue;

		m_hTroop[i]->SetDefenseAdd( nRate );
	}
}


void CFcGuardianManager::SetGuardianTroopAttackAdd( int nRate )
{
	m_nAttackAdd += nRate;
	for( int i=0; i<2; i++ )
	{
		if( m_hTroop[i] == NULL )
			continue;

		m_hTroop[i]->SetAttackAdd( nRate );
	}
}

void CFcGuardianManager::SetGuardianTroopMaxHPAdd( int nRate )
{
	for( int i=0; i<2; i++ )
	{
		if( m_hTroop[i] == NULL )
			continue;

		m_hTroop[i]->SetMaxHPAdd( nRate );
		m_hTroop[i]->SetFullHP();
		m_hTroop[i]->SetHP();
	}
}

void CFcGuardianManager::SetGuardianTroopMoveSpeedAdd( int nRate )
{
	m_nAddMoveSpeedRate += nRate;
	for( int i=0; i<2; i++ )
	{
		if( m_hTroop[i] == NULL )
			continue;

		m_hTroop[i]->SetMoveSpeedAdd( nRate );
	}
}

void CFcGuardianManager::SetGuardianTroopTrapDamageAdd( int nRate )
{
	m_nAddDamageTrap += nRate;
	for( int i=0; i<2; i++ )
	{
		if( m_hTroop[i] == NULL )
			continue;

		m_hTroop[i]->SetTrapDamageAdd( nRate );
	}
}

void CFcGuardianManager::AddGuardianTroopHP( int nRate,bool bPercent )
{
	for( int i=0; i<2; i++ )
	{
		if( m_hTroop[i] == NULL )
			continue;

		m_hTroop[i]->AddHP( nRate , bPercent );
		//m_hTroop[i]->SetFullHP();
		//m_hTroop[i]->SetHP();
	}
}


void CFcGuardianManager::AttackProcess()
{
	if( GetProcessTick()%40 == 17 )
	{
		for( int i=0; i<2; i++ )
		{
			if( m_hTroop[i] == NULL )
				continue;

			if( m_hTroop[i]->IsEnable() == false )
				continue;

			if( m_bEnable[i] == false )
				continue;

			// 공격명령을 수행 중인지
			bool bCmdAttack = IsDoingCmdAttack( m_hTroop[i], m_hCmdAttackTarget[i] );
			if( bCmdAttack )
			{
				// 궁병 활 공격 처리
				if( m_hTroop[i]->GetType() == TROOPTYPE_RANGE ||
					m_hTroop[i]->GetType() == TROOPTYPE_RANGEMELEE )
				{
					if( m_hCmdAttackTarget[i] )
					{
						std::vector<TroopObjHandle> VecTargetObjects;
						D3DXVECTOR2 Pos = m_hTroop[i]->GetPosV2();
						float fAttackRange = m_hTroop[i]->GetAttackRadius();
						g_FcWorld.GetTroopListInRange( &Pos, fAttackRange+1000.f, VecTargetObjects );
						int nCnt = VecTargetObjects.size();
						for( int j=0; j<nCnt; j++ )
						{
							if( VecTargetObjects[j] == m_hCmdAttackTarget[i] )
							{
								m_hTroop[i]->CmdAttack( m_hCmdAttackTarget[i], true, true );
								break;
							}
						}
					}
				}
				continue;
			}

			if( m_hTroop[i]->IsEngaged() )
				continue;

			bool bAttack = ProcessAttackTarget( m_hTroop[i] );
			if( bAttack )
				continue;

			TroopObjHandle hPlayer= g_FcWorld.GetHeroHandle()->GetTroop();

			float fRadius = m_hTroop[i]->GetRadius( false );
			m_hTroop[i]->CmdFollow(	hPlayer, fRadius );
		}
	}
}

void CFcGuardianManager::DefenseProcess()
{
	if( GetProcessTick()%40 == 32 )
	{
		for( int i=0; i<2; i++ )
		{
			if( m_hTroop[i] == NULL )
				continue;

			if( m_hTroop[i]->IsEnable() == false )
				continue;

			if( m_bEnable[i] == false )
				continue;

			if( m_bCmdDefenseMove[i] )
			{
				D3DXVECTOR2 HeroPos = g_FcWorld.GetHeroHandle()->GetTroop()->GetPosV2();
				D3DXVECTOR2 Dir =  m_hTroop[i]->GetPosV2() - HeroPos;
				float fDist = D3DXVec2LengthSq( &Dir );
				if( fDist < 700.f * 700.f )
				{
					m_bCmdDefenseMove[i] = false;
				}
				else
				{
					m_hTroop[i]->CmdRetreat( HeroPos.x, HeroPos.y, true );
				}
			}
			else
			{
				if( m_hTroop[i]->IsEngaged() )
				{
					D3DXVECTOR2 TargetPos = g_FcWorld.GetHeroHandle()->GetPosV2();
					D3DXVECTOR2 Dir = m_hTroop[i]->GetPosV2();
					Dir = Dir - TargetPos;
					float fDist = D3DXVec2LengthSq( &Dir );
					// 너무 멀리 떨어져 있으면 안 싸우고 쫓아오게 한다.
					if( fDist > 2000.f * 2000.f )
					{
						TroopObjHandle hPlayer= g_FcWorld.GetHeroHandle()->GetTroop();
						m_bCmdDefenseMove[i] = true;

						float fRadius = m_hTroop[i]->GetRadius( false );
						m_hTroop[i]->CmdFollow(	hPlayer, fRadius );
						continue;
					}
				}
				else
				{
					bool bAttack = ProcessAttackTarget( m_hTroop[i] );
					if( bAttack )
						continue;

					TroopObjHandle hPlayer= g_FcWorld.GetHeroHandle()->GetTroop();

					float fRadius = m_hTroop[i]->GetRadius( false );
					m_hTroop[i]->CmdFollow(	hPlayer, fRadius );
				}
			}
		}
	}
}


void CFcGuardianManager::FollowProcess()
{
	
}

bool CFcGuardianManager::ProcessAttackTarget( TroopObjHandle hTroop )
{
	std::vector<TroopObjHandle> VecTargetObjects;
	D3DXVECTOR2 Pos = hTroop->GetPosV2();
	switch( hTroop->GetType() )
	{
	case TROOPTYPE_MELEE:
	case TROOPTYPE_SPEAR:
		g_FcWorld.GetTroopListInRange( &Pos, 3000.f, VecTargetObjects );
		break;
	case TROOPTYPE_RANGEMELEE:
		g_FcWorld.GetTroopListInRange( &Pos, 5000.f, VecTargetObjects );
		break;
	default:
		g_FcWorld.GetTroopListInRange( &Pos, 3000.f, VecTargetObjects );
		break;
	}

	TroopObjHandle hTarget;
	float fDist = 0.f;
	int nCnt = VecTargetObjects.size();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hCurTroop = VecTargetObjects[i];
		if( !hCurTroop->IsEnable() )
			continue;

		if( hCurTroop->IsEliminated() )
			continue;

		if( hCurTroop->GetTeam() == 0 )
			continue;

		if( hTarget == NULL )
		{
			hTarget = hCurTroop;
			D3DXVECTOR2 Dir = hCurTroop->GetPosV2() - Pos;
			fDist = D3DXVec2LengthSq( &Dir );
		}
		else
		{
			D3DXVECTOR2 Dir = hCurTroop->GetPosV2() - Pos;
			float fCurDist = D3DXVec2LengthSq( &Dir );
			if( fDist > fCurDist )
				hTarget = hCurTroop;
		}
	}
	if( hTarget )
	{
		hTroop->CmdAttack( hTarget );
		return true;
	}
	return false;
}




#define GUARDIAN_CHECK_RADIUS	2000.f
#define LOCKON_CHECK_RADIUS	4000.f

void CFcGuardianManager::CheckGuardianTarget()
{
	TroopObjHandle hPlayerTroop = g_FcWorld.GetHeroHandle()->GetTroop();

	// 호위부대가 꽉 차 있으면 찾을 필요 없다.
	if( GetGuardianNum() == 2 )
		return;

	std::vector<TroopObjHandle> vecTroops;
	m_pTroopManager->GetFriendlyListInRange( &(hPlayerTroop->GetPosV2()), 0, GUARDIAN_CHECK_RADIUS, vecTroops );

	TroopObjHandle hNearstTroop;
	float fNearstDist;

	int nCnt = vecTroops.size();
	for( int j=0; j<nCnt; j++ )
	{
		if( vecTroops[j]->IsPlayerTroop() )
			continue;

		if( !vecTroops[j]->IsEnableGaurdian() )
			continue;

		if( vecTroops[j]->GetLeader() )
		{
			if( vecTroops[j]->GetLeader()->GetClassID() >= CFcGameObject::Class_ID_Hero )
			{
				continue;
			}
		}

		bool bFlag = false;
		for(int nCount=0; nCount<2; nCount++){
			if(vecTroops[j] == GetGuardianTroop( nCount )){
				bFlag = true;
				break;
			}
		}
		if(bFlag == true)
			continue;

		if( hNearstTroop == NULL )
		{
			hNearstTroop = vecTroops[j];
			D3DXVECTOR2 Dir = hNearstTroop->GetPosV2() - hPlayerTroop->GetPosV2();
			fNearstDist = D3DXVec2Length( &Dir );
			continue;
		}

		D3DXVECTOR2 Dir = vecTroops[j]->GetPosV2() - hPlayerTroop->GetPosV2();
		float fDist = D3DXVec2Length( &Dir );
		if( fNearstDist > fDist )	 // [PREFIX:nomodify] 2006/2/16 junyash PS#5206 reports fNearstDist uninitialized, but fNearstDist initialize at first loop. see above 'if( hNearstTroop == NULL )' scope's 'fNearstDist = D3DXVec2Length( &Dir );'. hNearstTroop setup NULL its construcotr.
		{
			hNearstTroop = vecTroops[j];
			fNearstDist = fDist;
		}
	}
	m_GuardianTarget = hNearstTroop;
}

void CFcGuardianManager::CheckLockOnTarget()
{
	TroopObjHandle hPlayerTroop = g_FcWorld.GetHeroHandle()->GetTroop();
	if( GetGuardianNum() == 0 )
		return;

	std::vector<TroopObjHandle> vecTroops;

	GameObjHandle hHero = g_FcWorld.GetHeroHandle();
	D3DXVECTOR3* pHeroDir = hHero->GetMoveDir();
	D3DXVECTOR2 HeroDirV2( pHeroDir->x, pHeroDir->z );

	D3DXVec2Normalize( &HeroDirV2, &HeroDirV2 );
	HeroDirV2 = HeroDirV2 * 30.f;

	D3DXVECTOR2 TargetPos = hPlayerTroop->GetPosV2() + HeroDirV2; 
	m_pTroopManager->GetEnemyListInRange( &TargetPos, 0, LOCKON_CHECK_RADIUS, vecTroops );

	std::vector<TroopObjHandle> vecTroopsInAngle;

	int nCnt = vecTroops.size();
	for( int j=0; j<nCnt; j++ )
	{
		TroopObjHandle hCurTroop = vecTroops[j];
		if( hCurTroop->IsPlayerTroop() )
			continue;

		CameraObjHandle CamHandle = CFcBaseObject::GetCameraObjectHandle( 0 );
		CCrossVector* pCross = CamHandle->GetCrossVector();

		D3DXVECTOR2 CamDir( pCross->m_ZVector.x, pCross->m_ZVector.z );
		D3DXVECTOR2 CamPos( pCross->m_PosVector.x, pCross->m_PosVector.z );
		D3DXVec2Normalize( &CamDir, &CamDir );
		D3DXVECTOR2 TargetDir = hCurTroop->GetPosV2() - CamPos;
		D3DXVec2Normalize( &TargetDir, &TargetDir);

		float fAngle = D3DXVec2Dot( &CamDir, &TargetDir );

		if( fAngle > 0.98f )
			vecTroopsInAngle.push_back( hCurTroop );
	}

	TroopObjHandle hNearstTroop;
	float fNearstDist;
	nCnt = vecTroopsInAngle.size();
	for( int j=0; j<nCnt; j++ )
	{
		TroopObjHandle hCurTroop = vecTroopsInAngle[j];
		if( hCurTroop->IsPlayerTroop() )
			continue;

		if( hCurTroop->IsEngaged() )
		{
			bool bValid = true;
			int nCnt = hCurTroop->GetMeleeEngageTroopCount();
			for( int i=0; i<nCnt; i++ )
			{
				TroopObjHandle hTroop = hCurTroop->GetMeleeEngageTroop( i );
				if( hTroop->IsPlayerTroop() )
				{
					bValid = false;
					break;
				}
			}
			if( bValid == false )
				continue;
		}

		if( hNearstTroop == NULL )
		{
			hNearstTroop = hCurTroop;
			D3DXVECTOR2 Dir = hNearstTroop->GetPosV2() - hPlayerTroop->GetPosV2();
			fNearstDist = D3DXVec2Length( &Dir );
			continue;
		}

		D3DXVECTOR2 Dir = hCurTroop->GetPosV2() - hPlayerTroop->GetPosV2();
		float fDist = D3DXVec2Length( &Dir );
		if( fNearstDist > fDist )
		{
			hNearstTroop = hCurTroop;
			fNearstDist = fDist;
		}
	}
	if( hNearstTroop )
		m_LockOnTarget = hNearstTroop;
}


bool CFcGuardianManager::IsDoingCmdAttack( TroopObjHandle hTroop, TroopObjHandle hTarget)
{
	if( hTarget == NULL )
		return false;

	if( (hTroop->GetState() == TROOPSTATE_MOVE_ATTACK || 
		hTroop->GetState() == TROOPSTATE_MELEE_READY ||
		hTroop->GetState() == TROOPSTATE_RANGE_ATTACK) &&
		hTroop->GetTargetTroop() == hTarget )
	{
		return true;
	}

	return false;
}

bool CFcGuardianManager::IsDoingCmdDefense( TroopObjHandle hTroop )
{
	return false;
}
