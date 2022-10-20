#include "stdafx.h"
#include "FcTriggerCondition.h"
#include "BSTriggerManager.h"
#include "FcGlobal.h"
#include "FcWorld.h"
#include "BSFileManager.h"
#include "FcTroopObject.h"
#include "FcTroopManager.h"
#include "FcUnitObject.h"
#include "InputPad.h"
#include "FcTriggerCommander.h"
#include "FcProp.h"
#include "FcHeroObject.h"
#include "DebugUtil.h"
#include "FcRealtimeMovie.h"
#include "BsTriggerVariable.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


void CFcConTimeElapsed::Initialize()
{
	m_BeginTime = -1;
	m_nCount = 0;
}


bool CFcConTimeElapsed::IsTrue()
{
	/*int iSeconds;
	int nOperType;

	if(m_BeginTime == -1)
	{
		QueryPerformanceFrequency((LARGE_INTEGER *)&m_Frequency);
		QueryPerformanceCounter((LARGE_INTEGER *)&m_BeginTime);
	}

	QueryPerformanceCounter((LARGE_INTEGER *)&m_EndTime);
	__int64 ElapseTime = m_EndTime - m_BeginTime;
	double During = double(ElapseTime / m_Frequency);
	if(During >= 1)
	{
		QueryPerformanceCounter((LARGE_INTEGER *)&m_BeginTime);
		m_nCount++;
	}*/

	int iSeconds = GetParamInt( 0 );
	int nOperType = GetParamInt( 1 );

	if(m_BeginTime == -1){	
		m_BeginTime = GetProcessTick();
	}
	if(GetProcessTick() - m_BeginTime >= FRAME_PER_SEC){	
		m_nCount++;
		m_BeginTime = GetProcessTick();
	}
	
	//if(!( % FRAME_PER_SEC)){	
	//	m_nCount++;
	//}

	return Compare( m_nCount, nOperType, iSeconds );
}
//-----------------------------------------------------------------------------------------------------


bool CFcTroopArrive::IsTrue()
{
	// 일단 부대 중점이 영역 안으로 들어오면 체크하게 한당..
	int nTroopIndex;
	GetParamInt( 0, nTroopIndex );
	char* pAreaName = GetParamStr( 1 );

	TroopObjHandle hTroop = CFcWorld::GetInstance().GetTroopObject( nTroopIndex );
	AREA_INFO *pArea = CFcWorld::GetInstance().GetAreaInfo( pAreaName );

	if( hTroop->GetPos().x < pArea->fSX ) return false;
	if( hTroop->GetPos().x > pArea->fEX ) return false;
	if( hTroop->GetPos().z < pArea->fSZ ) return false;
	if( hTroop->GetPos().z > pArea->fEZ ) return false;	

	return true;
}

//-----------------------------------------------------------------------------------------------------

void CFcConInputStickA::Initialize()
{
}

bool CFcConInputStickA::IsTrue()
{
	if(g_FcWorld.GetTriggerCommander()->GetKeyHookOn(PAD_INPUT_A) != -1)
	{
		g_FcWorld.GetTriggerCommander()->ReSetKeyHookOn();
		return true;
	}
	return false;
	
}
//-----------------------------------------------------------------------------------------------------

void CFcConInputStickLeft::Initialize()
{
}

bool CFcConInputStickLeft::IsTrue()
{
	return true;
}

//-----------------------------------------------------------------------------------------------------


void CFcConInputStickRight::Initialize()
{
}

bool CFcConInputStickRight::IsTrue()
{
	return true;
}
//-----------------------------------------------------------------------------------------------------


void CFcInputStickNeutral::Initialize()
{
}

bool CFcInputStickNeutral::IsTrue()
{
	return true;
}
//-----------------------------------------------------------------------------------------------------


void CFcConCompareOrb::Initialize()
{
}

bool CFcConCompareOrb::IsTrue()
{
	int nPlayerID = GetParamInt( 0 );
	int nOrbValue = GetParamInt( 1 );
	int nOperType = GetParamInt( 2 );

	BsAssert( nPlayerID == 0 );
	HeroObjHandle hHero = g_FcWorld.GetHeroHandle();
	int nCurOrb = hHero->GetOrbSpark();

	return Compare( nCurOrb, nOperType, nOrbValue );
}
//-----------------------------------------------------------------------------------------------------


void CFcConTimeElapsedFromMark::Initialize()
{
}

bool CFcConTimeElapsedFromMark::IsTrue()
{
	int nTimeMarkID = GetParamInt( 0 );
	int nSeconds = GetParamInt( 1 );
	int nOperType = GetParamInt( 2 );

	int nTimeMarkSeconds = g_FcWorld.GetTimeMark( nTimeMarkID );
	if( nTimeMarkSeconds == -1 )		// 타임마크가 세팅 안되어있을 땐 return false
	{
		return false;
	}

	nSeconds = ( GetProcessTick() / FRAME_PER_SEC ) - nSeconds;

	return Compare( nTimeMarkSeconds, nOperType, nSeconds );
}
//-----------------------------------------------------------------------------------------------------


void CFcConAttacked::Initialize()
{
}

bool CFcConAttacked::IsTrue()
{
	char* szTroopName = GetParamStr( 0 );
	TroopObjHandle hTroop = CFcWorld::GetInstance().GetTroopObject( szTroopName );
	if( hTroop->IsAttacked() )
	{
		DebugString(szTroopName);
		DebugString( " Attacked TRUE\n" );

		return true;
	}
/*
	if(hTroop->GetState() != TROOPSTATE_RANGE_DAMAGE){return false;}
	if(hTroop->GetState() != TROOPSTATE_MELEE_DAMAGE){return false;}
*/
	return false;
}
//-----------------------------------------------------------------------------------------------------

void CFcConAttackedTroop::Initialize()
{
}

bool CFcConAttackedTroop::IsTrue()
{
	char* szTroopName = GetParamStr( 0 );
	TroopObjHandle hTroop = CFcWorld::GetInstance().GetTroopObject( szTroopName );

	char* szTargetTroopName = GetParamStr( 1 );
	TroopObjHandle hTargetTroop = CFcWorld::GetInstance().GetTroopObject( szTargetTroopName );
	if( hTargetTroop->IsAttacked( hTroop ) )
	{
		return true;
	}
	return false;
}
//-----------------------------------------------------------------------------------------------------


void CFcConCompareTroopDist::Initialize()
{
}

bool CFcConCompareTroopDist::IsTrue()
{
	char* szTroopName = GetParamStr( 0 );
	TroopObjHandle hTroop = CFcWorld::GetInstance().GetTroopObject( szTroopName );

	char* szTargetTroopName = GetParamStr( 1 );
	TroopObjHandle hTargetTroop = CFcWorld::GetInstance().GetTroopObject( szTargetTroopName );

	int nDist = (int) GetParamFloat( 2 );
	int nOper = GetParamInt( 3 );

	D3DXVECTOR2 Dir = hTroop->GetPosV2() - hTargetTroop->GetPosV2();
	int nTroopDist = (int)D3DXVec2Length( &Dir );
	
	return Compare( nTroopDist, nOper, nDist );
}
//-----------------------------------------------------------------------------------------------------


void CFcConCompareTroopHP::Initialize()
{
}

bool CFcConCompareTroopHP::IsTrue()
{
	char* szTroopName = GetParamStr( 0 );
	TroopObjHandle hTroop = CFcWorld::GetInstance().GetTroopObject( szTroopName );
	if( hTroop->IsEnable() == false )
		return false;

	int nHPPer = GetParamInt( 1 );
	int nOper = GetParamInt( 2 );
	int nTroopHPPer = 0;
	int nMaxHP = 0;
	int nHP = 0;

	if( hTroop )
	{
		switch( hTroop->GetType() ) {
			case TROOPTYPE_SIEGE:
				{
					GameObjHandle hUnit = hTroop->GetUnit(0);
					if( hUnit ) {
						nMaxHP = hUnit->GetMaxHP();
						nHP = hUnit->GetHP();
					}
				}
				break;
			default:
				nMaxHP = hTroop->GetMaxHP();
				nHP = hTroop->GetHP();
				break;
		}
		if( nMaxHP )
			nTroopHPPer = nHP * 100 / nMaxHP;
		else
			BsAssert(0);

//		DebugString( "CTHP : %d, %d, %d\n", nHP, nMaxHP, nTroopHPPer );
	}
	else
		BsAssert(0);

	if( nTroopHPPer == 0 && nHP > 0 )
		nTroopHPPer = 1;

	return Compare( nTroopHPPer, nOper, nHPPer );
}

//-----------------------------------------------------------------------------------------------------


void CFcConTroopInArea::Initialize()
{
}

bool CFcConTroopInArea::IsTrue()
{
	char* szTroopName = GetParamStr( 0 );
	TroopObjHandle hTroop = CFcWorld::GetInstance().GetTroopObject( szTroopName );

//	if( hTroop->IsEnable() == false )
//		return false;

	if( hTroop->IsEliminated() == true )
		return false;

	char* pAreaName = GetParamStr( 1 );
	AREA_INFO* pInfo = g_FcWorld.GetAreaInfo( pAreaName );
	D3DXVECTOR3 Pos = hTroop->GetPos();
	return IsInRect( Pos.x, Pos.z, pInfo->fSX, pInfo->fSZ, pInfo->fEX, pInfo->fEZ );
}
//-----------------------------------------------------------------------------------------------------


void CFcConCompareLeaderHP::Initialize()
{
}

bool CFcConCompareLeaderHP::IsTrue()
{
	char* szTroopName = GetParamStr( 0 );
	TroopObjHandle hTroop = CFcWorld::GetInstance().GetTroopObject( szTroopName );

	int nHpPer = GetParamInt( 1 );
	int nOper = GetParamInt( 2 );
	int nLeaderHPPer = 0;
	int nHP = 0;
	int nMaxHP = 0;

	if( hTroop )
	{
		GameObjHandle hLeader = hTroop->GetLeader();

		if( hLeader )
		{
			if( hLeader->IsDie() )
				nLeaderHPPer = 0;
			else
			{
				nMaxHP = hLeader->GetMaxHP();
				nHP = hLeader->GetHP();

				if( nMaxHP )
					nLeaderHPPer = nHP * 100 / nMaxHP;
				else
					BsAssert(0);
			}
		}
		else
			nLeaderHPPer = 0;
	}
	else
		BsAssert(0);

	if( nLeaderHPPer == 0 && nHP > 0 )
		nLeaderHPPer = 1;

	return Compare( nLeaderHPPer, nOper, nHpPer );
}
//-----------------------------------------------------------------------------------------------------


void CFcConTroopMeleeAttacked::Initialize()
{
}

bool CFcConTroopMeleeAttacked::IsTrue()
{
	char* szTroopName = GetParamStr( 0 );
	TroopObjHandle hTroop = CFcWorld::GetInstance().GetTroopObject( szTroopName );

	return hTroop->IsAttackedMelee();
}

//-----------------------------------------------------------------------------------------------------
void CFcConTroopRangeAttacked::Initialize()
{
}

bool CFcConTroopRangeAttacked::IsTrue()
{
	char* szTroopName = GetParamStr( 0 );
	TroopObjHandle hTroop = CFcWorld::GetInstance().GetTroopObject( szTroopName );

	return hTroop->IsAttackedRange();
}
//-----------------------------------------------------------------------------------------------------
void CFcConUnBlockableAttacked::Initialize()
{
}

bool CFcConUnBlockableAttacked::IsTrue()
{
	return true;
}
//-----------------------------------------------------------------------------------------------------

void CFcConTroopNoEngaged::Initialize()
{
}

bool CFcConTroopNoEngaged::IsTrue()
{
	char* szTroopName = GetParamStr( 0 );
	TroopObjHandle hTroop = CFcWorld::GetInstance().GetTroopObject( szTroopName );
	return !hTroop->IsEngaged();
}
//-----------------------------------------------------------------------------------------------------

void CFcConTroopNotInArea::Initialize()
{
}

bool CFcConTroopNotInArea::IsTrue()
{
	char* szTroopName = GetParamStr( 0 );
	TroopObjHandle hTroop = CFcWorld::GetInstance().GetTroopObject( szTroopName );

//	if( hTroop->IsEnable() == false )
//		return true;

	if( hTroop->IsEliminated() == true )
		return true;

	char* pAreaName = GetParamStr( 1 );
	AREA_INFO* pInfo = g_FcWorld.GetAreaInfo( pAreaName );
 	D3DXVECTOR3 Pos = hTroop->GetPos();
	return !IsInRect( Pos.x, Pos.z, pInfo->fSX, pInfo->fSZ, pInfo->fEX, pInfo->fEZ );
}
//-----------------------------------------------------------------------------------------------------
void CFcConVar::Initialize()
{
}

bool CFcConVar::IsTrue()
{
	int nVarID = GetParamInt( 0 );
	int nValue = GetParamInt( 1 );
	int nOper = GetParamInt( 2 );
	
	if( nVarID < 0 )
	{
		DebugString( "Invalid parameter in ConVar\n" );
		BsAssert( 0 );
	}
	return Compare( g_FcWorld.GetTriggerVarInt( nVarID ) , nOper, nValue );
}

//-----------------------------------------------------------------------------------------------------
bool CFcConPropWasDestroyed::IsTrue()
{
	CFcProp *pProp = g_FcWorld.GetProp(GetParamStr( 0 ));
	if( !pProp ) return false;
	return pProp->IsBreak();
}


bool CFcConTroopEngaged::IsTrue()
{
	char* szTroopName = GetParamStr( 0 );
	TroopObjHandle hTroop = CFcWorld::GetInstance().GetTroopObject( szTroopName );	

	if( hTroop->IsEngaged() )
		return true;

	return false;
}


bool CFcConForceCompareHP::IsTrue()
{
	int nForce = GetParamInt( 0 );
	int nPercent = GetParamInt( 1 );
	int nOperType = GetParamInt( 2 );

	CFcTroopManager* pTroopManager = g_FcWorld.GetTroopmanager();
	int nCnt = pTroopManager->GetTroopCount();

	int nMaxHP = 0;
	int nHP = 0;
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hHandle = pTroopManager->GetTroop( i );
		if( !hHandle->IsEnable() )
			continue;

		if( hHandle->GetForce() != nForce )
			continue;

//		if( !( hHandle->IsEliminated() ) )
		nHP += hHandle->GetHP();

		nMaxHP += hHandle->GetMaxHP();
	}

	
	int nCurPercent;	
	if( nMaxHP > 0 )
		nCurPercent = ( nHP * 100 ) / nMaxHP;
	else
	{

//		nCurPercent = 0;
		return false;
	}
/*
DebugString( "CompareHP Bug Team %d, Per %d, Oper %d, HP %d, MaxHP %d, PerResult %d\n", 
	nForce, nPercent, nOperType, nHP, nMaxHP, nCurPercent );


for( int i=0; i<nCnt; i++ )
{
	TroopObjHandle hHandle = pTroopManager->GetTroop( i );
	if( !hHandle->IsEnable() )
		continue;

	if( hHandle->GetForce() != nForce )
		continue;

	DebugString( "CompareTroop %s, %d, %d\n", hHandle->GetName(), hHandle->GetHP(), hHandle->GetMaxHP() );
}
*/


	if( nCurPercent == 0 && nHP > 0 )
		nCurPercent = 1;

	return Compare( nCurPercent, nOperType, nPercent );
}

bool CFcConForceEliminated::IsTrue()
{
	int nForce = GetParamInt( 0 );
	CFcTroopManager* pTroopManager = g_FcWorld.GetTroopmanager();
	int nCnt = pTroopManager->GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hHandle = pTroopManager->GetTroop( i );

		if( hHandle->GetForce() != nForce )
			continue;

		if( !hHandle->IsEnable() )
			continue;

		if( !hHandle->IsEliminated() )
			return false;
	}
	return true;
}

bool CFcConForceInArea::IsTrue()
{
	char* pAreaName = GetParamStr( 0 );
	int nForce = GetParamInt( 1 );
	AREA_INFO *pArea = CFcWorld::GetInstance().GetAreaInfo( pAreaName );
	CFcTroopManager* pTroopManager = g_FcWorld.GetTroopmanager();
	int nCnt = pTroopManager->GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hHandle = pTroopManager->GetTroop( i );
		if( hHandle->GetForce() != nForce )
			continue;

//		if( !hHandle->IsEnable() )
//			continue;

		if( hHandle->IsEliminated() == true )
			continue;

		D3DXVECTOR3 Pos = hHandle->GetPos();
		if( Pos.x > pArea->fSX && Pos.x < pArea->fEX &&
			Pos.z > pArea->fSZ && Pos.z < pArea->fEZ )
			return true;
	}
	return false;
}

bool CFcConForceNotInArea::IsTrue()
{
	char* pAreaName = GetParamStr( 0 );
	int nForce = GetParamInt( 1 );
	AREA_INFO *pArea = CFcWorld::GetInstance().GetAreaInfo( pAreaName );
	CFcTroopManager* pTroopManager = g_FcWorld.GetTroopmanager();
	int nCnt = pTroopManager->GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hHandle = pTroopManager->GetTroop( i );

		if( hHandle->GetForce() != nForce )
			continue;

//		if( !hHandle->IsEnable() )
//			continue;

		if( hHandle->IsEliminated() )
			continue;

		D3DXVECTOR3 Pos = hHandle->GetPos();
		if( Pos.x > pArea->fSX && Pos.x < pArea->fEX &&
			Pos.z > pArea->fSZ && Pos.z < pArea->fEZ )
			return false;
	}
	return true;
}

bool CFcConForceAllInArea::IsTrue()
{
	char* pAreaName = GetParamStr( 0 );
	int nForce = GetParamInt( 1 );
	AREA_INFO *pArea = CFcWorld::GetInstance().GetAreaInfo( pAreaName );
	CFcTroopManager* pTroopManager = g_FcWorld.GetTroopmanager();
	int nCnt = pTroopManager->GetTroopCount();
	int nNum = 0;
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hHandle = pTroopManager->GetTroop( i );

		if( hHandle->GetForce() != nForce )
			continue;

//		if( !hHandle->IsEnable() )
//			continue;

		if( hHandle->IsEliminated() )
			continue;

		nNum ++;
		D3DXVECTOR3 Pos = hHandle->GetPos();
		if( !(Pos.x > pArea->fSX && Pos.x < pArea->fEX &&
			Pos.z > pArea->fSZ && Pos.z < pArea->fEZ) )
			return false;
	}

	if(nNum == 0)
		return false;

	return true;
}


bool CFcConPlayerGuardianNum::IsTrue()
{
	int nNum = GetParamInt( 0 );
	int nOper = GetParamInt( 1 );
	int nCurGuardianNum = g_FcWorld.GetTroopmanager()->GetGuardianNum();

	return Compare( nCurGuardianNum, nOper, nNum );
}


bool CFcConTroopType::IsTrue()
{
	char* pTroopName = GetParamStr( 0 );
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	int nTroopType = GetParamInt( 1 );
	if( hTroop )
	{
		if( hTroop->GetType() == (TROOPTYPE)nTroopType )
			return true;
	}
	return false;
}

bool CFcConGuardianTroopType::IsTrue()
{
	int nTroopType = GetParamInt( 0 );
	for( int i=0; i<2; i++ )
	{
		TroopObjHandle hTroop = g_FcWorld.GetTroopmanager()->GetGuardianTroop( i );
		if( hTroop )
		{
			if( hTroop->GetType() == (TROOPTYPE)nTroopType )
				return true;
		}
	}
	return false;
}

bool CFcConTroopLevel::IsTrue()
{
	char* pTroopName = GetParamStr( 0 );
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );	
	int nLevel = GetParamInt( 1 ) - 1;
	int nOperType = GetParamInt( 2 );

	return Compare( hTroop->GetLevel(), nOperType, nLevel );
}



bool CFcConOrbGaugeFull ::IsTrue()
{
	HeroObjHandle hHero = g_FcWorld.GetHeroHandle();
	int nCurOrb = hHero->GetOrbSpark();
	int nMaxOrb = hHero->CalcMaxOrbSpark();
	if( nCurOrb == nMaxOrb )
		return true;
	return false;
}

bool CFcConTrueOrbGaugeFull ::IsTrue()
{
	HeroObjHandle hHero = g_FcWorld.GetHeroHandle();
	int nCurOrb = hHero->GetTrueOrbSpark();
	int nMaxOrb = hHero->GetMaxTrueOrbSpark();
	if( nCurOrb == nMaxOrb )
		return true;
	return false;
}

bool CFcConTotalKillNum::IsTrue()
{
	int nNum = GetParamInt( 0 );
	int nOper = GetParamInt( 1 );
	
	HeroObjHandle Handle = g_FcWorld.GetHeroHandle();
	return Compare( Handle->GetKillCount() , nOper, nNum );
}

bool CFcConForceKillNum::IsTrue()
{
	int nForce = GetParamInt( 0 );
	int nNum = GetParamInt( 1 );
	int nOper = GetParamInt( 2 );
	int nTeamKill = 0;

	CFcTroopManager* pTroopManager = g_FcWorld.GetTroopmanager();
	int nCnt = pTroopManager->GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hHandle = pTroopManager->GetTroop( i );

		if( hHandle->GetForce() == nForce )
			nTeamKill += hHandle->GetDeadUnitCount();
	}

	//HeroObjHandle Handle = g_FcWorld.GetHeroHandle( 0 );
	return Compare( nTeamKill , nOper, nNum );
}

bool CFcConGuardianInArea::IsTrue()
{
	int nIndex = GetParamInt( 0 );
	char* pAreaName = GetParamStr( 1 );

	TroopObjHandle hTroop = g_FcWorld.GetTroopmanager()->GetGuardianTroop( nIndex );
	if(!hTroop)
		return false;	
	AREA_INFO* pInfo = g_FcWorld.GetAreaInfo( pAreaName );
	D3DXVECTOR3 Pos = hTroop->GetPos();
	return IsInRect( Pos.x, Pos.z, pInfo->fSX, pInfo->fSZ, pInfo->fEX, pInfo->fEZ );
}

bool CFcConGuardianNotInArea::IsTrue()
{	
	return !CFcConGuardianInArea::IsTrue();
}

bool CFcConGuardianCompareDist::IsTrue()
{
	int nIndex		= GetParamInt( 0 );
	char* pTroopName= GetParamStr( 1 );
	int	nRange		= GetParamInt( 2 );
	int nOper		= GetParamInt( 3 );
	
	TroopObjHandle hGuardian = g_FcWorld.GetTroopmanager()->GetGuardianTroop( nIndex );
	if(!hGuardian)
		return false;		
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );	
	if(!hTroop)
		return false;	
	D3DXVECTOR3 vTemp = hTroop->GetPos() - hGuardian->GetPos();
	float nDistance = D3DXVec3Length( &vTemp );
	
	return Compare( int(nDistance) , nOper, nRange);
}

bool CFcConGuardianExist::IsTrue()
{
	int nIndex = GetParamInt( 0 );
	TroopObjHandle hTroop = g_FcWorld.GetTroopmanager()->GetGuardianTroop( nIndex );
	if(!hTroop)
		return false;
	return true;
}

bool CFcConGuardianTroopType2::IsTrue()
{
	int nIndex = GetParamInt( 0 );
	int nTroopType = GetParamInt( 1 );
	
	TroopObjHandle hTroop = g_FcWorld.GetTroopmanager()->GetGuardianTroop( nIndex );
	if( hTroop )
		return hTroop->GetType() == (TROOPTYPE)nTroopType;

	return false;
}

bool CFcConSelectMissionMenuValue::IsTrue() 
{
	GAME_STAGE_ID nStageID = (GAME_STAGE_ID)GetParamInt(0);

	return (g_FCGameData.SpecialSelStageIdforTrigger == nStageID);
}


bool CFcConForceCompareUnitNumber::IsTrue()
{
	int nForce = GetParamInt( 0 );
	int nPer = GetParamInt( 1 );
	int nOper = GetParamInt( 2 );

	CFcTroopManager* pTroopManager = g_FcWorld.GetTroopmanager();
	int nCnt = pTroopManager->GetTroopCount();

	int nTotalUnitCnt = 0;
	int nLiveUnitCnt = 0;
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hHandle = pTroopManager->GetTroop( i );
		if( !hHandle->IsEnable() )
			continue;

		if( hHandle->GetForce() != nForce )
			continue;

		nLiveUnitCnt += hHandle->GetAliveUnitCount();
		nTotalUnitCnt += hHandle->GetUnitCount();
	}
	int nCurPer = (int)((float)nLiveUnitCnt / (float)nTotalUnitCnt * 100.f);
	return Compare( nCurPer, nOper, nPer );
}


bool CFcConTroopCompareUnitNumber::IsTrue()
{
	char* szTroopName = GetParamStr( 0 );
	int nPer = GetParamInt( 1 );
	int nOper = GetParamInt( 2 );

	TroopObjHandle hTroop = CFcWorld::GetInstance().GetTroopObject( szTroopName );

	int nUnitCnt = hTroop->GetUnitCount();
	int nLiveUnitCnt = hTroop->GetAliveUnitCount();

	int nCurPer = (int)((float)nLiveUnitCnt / (float)nUnitCnt * 100.f);
	return Compare( nCurPer, nOper, nPer );
}


bool CFcConRealmovieFinish::IsTrue()
{
	return !(g_pFcRealMovie->IsPlay());
}


bool CFcConCompareTroopNForceDist::IsTrue()
{
	char* szTroopName = GetParamStr( 0 );
	int nTeam = GetParamInt( 1 );
	int nForce = GetParamInt( 2 );
	float fDist = (float)GetParamInt( 3 );
	int nOper = GetParamInt( 4 );

	fDist = fDist / 100.f;
	fDist = fDist * fDist;

	TroopObjHandle hTargetTroop = CFcWorld::GetInstance().GetTroopObject( szTroopName );
	D3DXVECTOR2 TroopPos = hTargetTroop->GetPosV2();
	D3DXVECTOR2 ForcePos( 0.f, 0.f );
	int nForceCnt = 0;

	std::vector<TroopObjHandle> vecObjs;

	int nCnt = g_FcWorld.GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTroop = g_FcWorld.GetTroopObject( i );
		if( !hTroop->IsEnable() )
			continue;

		if( hTroop->IsEliminated() )
			continue;

		if( hTroop->GetTeam() != nTeam || hTroop->GetForce() != nForce )
			continue;

		vecObjs.push_back( hTroop );
		nForceCnt += 1;
	}
	if( nForceCnt == 0 )
		return false;

	nCnt = vecObjs.size();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTroop = vecObjs[i];
		D3DXVECTOR2 Pos1 = hTroop->GetPosV2();
		D3DXVECTOR2 Pos2 = hTargetTroop->GetPosV2();

		D3DXVECTOR2 Dir = Pos1 - Pos2;
		Dir = Dir / 100.f;
		float fCurDist = D3DXVec2LengthSq( &Dir );
		
		bool bRet = Compare( (int)fCurDist, nOper, (int)fDist );
		if( bRet == true )
			return true;
	}
	return false;
}

bool CFcConCompareForceNForceDist::IsTrue()
{
	int nTeam1 = GetParamInt( 0 );
	int nForce1 = GetParamInt( 1 );
	int nTeam2 = GetParamInt( 2 );
	int nForce2 = GetParamInt( 3 );
	float fDist = (float)GetParamInt( 4 );
	int nOper = GetParamInt( 5 );

	fDist = fDist / 100.f;
	fDist = fDist * fDist;

	std::vector<TroopObjHandle> vecForce1;

	int nForceCnt = 0;
	int nCnt = g_FcWorld.GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTroop = g_FcWorld.GetTroopObject( i );
		if( !hTroop->IsEnable() )
			continue;

		if( hTroop->IsEliminated() )
			continue;

		if( hTroop->GetTeam() != nTeam1 || hTroop->GetForce() != nForce1 )
			continue;

		vecForce1.push_back( hTroop );
		++nForceCnt;
	}

	if( nForceCnt == 0 )
		return false;

	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTroop = g_FcWorld.GetTroopObject( i );
		if( !hTroop->IsEnable() )
			continue;

		if( hTroop->IsEliminated() )
			continue;

		if( hTroop->GetTeam() != nTeam2 || hTroop->GetForce() != nForce2 )
			continue;

		int nForce1Cnt = vecForce1.size();
		for( int j=0; j<nForceCnt; j++ )
		{
			TroopObjHandle hForce1 = vecForce1[j];
			D3DXVECTOR2 Pos1 = hForce1->GetPosV2();
			D3DXVECTOR2 Pos2 = hTroop->GetPosV2();

			D3DXVECTOR2 Dir = Pos1 - Pos2;
			Dir = Dir / 100.f;
			float fCurDist = D3DXVec2LengthSq( &Dir );

			bool bRet = Compare( (int)fCurDist, nOper, (int)fDist );
			if( bRet == true )
				return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------------------------------
void CFcConVarCompare::Initialize()
{
}

bool CFcConVarCompare::IsTrue()
{
	int nVar1ID = GetParamInt( 0 );
	int nVar2ID = GetParamInt( 1 );
	int nOper = GetParamInt( 2 );
	
	if( nVar1ID < 0 || nVar2ID < 0)
	{
		DebugString( "Invalid parameter in ConVar\n" );
		BsAssert( 0 );
	}

	return Compare( g_FcWorld.GetTriggerVarInt( nVar1ID ), nOper, g_FcWorld.GetTriggerVarInt( nVar2ID ) );
}

//-----------------------------------------------------------------------------------------------------


bool CFcConTroopAttackedForce::IsTrue() {
	int nForce = GetParamInt(0);

	int nCnt = g_FcWorld.GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTroop = g_FcWorld.GetTroopObject( i );
		
		if( !hTroop || !hTroop->IsEnable() )
			continue;

		if( hTroop->GetForce() != nForce )
			continue;

		if( hTroop->IsAttacked( ) ) 
			return true;
	}	
	return false;
}
bool CFcConTroopAttackedStr::IsTrue() {
	char* szStr = GetParamStr( 0 );

	int nCnt = g_FcWorld.GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTroop = g_FcWorld.GetTroopObject( i );
		
		if( !hTroop || !hTroop->IsEnable() )
			continue;

		char* pTempStr = strstr( (char*)hTroop->GetName(), szStr ); 
		if( pTempStr == NULL )
			continue;

		if( hTroop->IsAttacked() ) 
			return true;
	}	
	return false;
}


bool CFcConItemBoxBroken::IsTrue() {

	char* pPropName = GetParamStr(0);
	CFcProp* pProp = g_FcWorld.GetProp( pPropName );
	if(!pProp || !pProp->IsItemProp())
		return false;
	CFcItemProp* pItem = (CFcItemProp*)pProp;    
	if(pItem->IsBreak())
		return true;

	return false;
}


bool CFcConPropHP::IsTrue()
{
	char* pPropName = GetParamStr( 0 );
	int nPer = GetParamInt( 1 );
	int nOper = GetParamInt( 2 );

	CFcProp* pProp = g_FcWorld.GetProp( pPropName );
	if( pProp->IsCrushProp() )
	{
		CFcBreakableProp* pBreakable = (CFcBreakableProp*) pProp;
		if( pBreakable->GetMaxHP() <= 0 )
		{
			char szErr[128];
			sprintf(szErr,"( Name : %s )Prop max HP is 0 // can't divide",pPropName);
			BsAssert( 0 && szErr );
		}		
		int nPropPer = pBreakable->GetHP() * 100 / pBreakable->GetMaxHP();
		return Compare( nPropPer, nOper, nPer);
	}
	else
	{
		BsAssert( 0 && "Is not breakable prop" );
		DebugString( "ConPropHP is not breakable prop %s\n", pPropName );
		return false;
	}
	return false;
}

bool CFcConIsDemoSkip::IsTrue()
{
	if( g_FcWorld.IsDemoSkip() )
	{
		g_FcWorld.SetDemoSkip( false );
		g_FcWorld.SetDemoType( -1 );	// reset
		return true;
	}
	return false;
}

bool CFcConTroopCompareHPExceptLeader::IsTrue()
{
	char* szTroopName = GetParamStr( 0 );
	TroopObjHandle hTroop = CFcWorld::GetInstance().GetTroopObject( szTroopName );
	int nPercent = GetParamInt( 1 );
	int nOperType = GetParamInt( 2 );

	int nHP = hTroop->GetHP();
	int nMaxHP = hTroop->GetMaxHP();
	GameObjHandle hLeader = hTroop->GetLeader();
	if( hLeader )
	{
		nHP -= hLeader->GetHP();
		nMaxHP -= hLeader->GetMaxHP();
	}

	int nCurPercent;	
	if( nMaxHP > 0 )
		nCurPercent = ( nHP * 100 ) / nMaxHP;
	else
	{
		return false;
	}
	if( nCurPercent == 0 && nHP > 0 )
		nCurPercent = 1;

	return Compare( nCurPercent, nOperType, nPercent );
}