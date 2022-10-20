#include "stdafx.h"
#include "FcTroopManager.h"
#include "FcWorld.h"
#include "FcAIObject.h"
#include "FcTroopObject.h"
#include "BSFileManager.h"
#include "FcUtil.h"
#include "FcTroopAIObject.h"
#include "FcSOXLoader.h"
#include "DebugUtil.h"
#include "FcProp.h"
#include "FcSoundManager.h"
//#include ".\\data\\Sound\\NNN_SOUND.h"
#include "FcGlobal.h"
#include "FcGameObject.h"
#include "PerfCheck.h"
#include "FcItem.h"
#include "FcSOXLoader.h"
#include "FcGuardianManager.h"
#include "FcCameraObject.h"
#ifdef	_XBOX
#include ".\\MS\\CClipTest.h"
#endif


// Include Unit
#include "FcHorseObject.h"
#include "FcHeroObject.h"




#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

extern CTroopSOXLoader		g_TroopSOX;


///////////////////////////////////////////////////////////////////////
// CFcTroopManager class //////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////



CFcTroopManager::CFcTroopManager()
{
//	m_nGuardianAttackAI = m_nGuardianDefenseAI = m_nGuardianNormalAI = -1;
	m_nCurEnableID = 0;
	m_pGuardianManager = NULL;
}

CFcTroopManager::~CFcTroopManager()
{
	CFcTroopObject::ReleaseObjects();
	SAFE_DELETE( m_pGuardianManager );
}


bool CFcTroopManager::Initialize()
{
	m_pGuardianManager = new CFcGuardianManager( this );
	return true;
}

void CFcTroopManager::PostProcess()
{
	CFcTroopObject::PostProcessObjects();
}

void CFcTroopManager::Process()
{
	if( GetProcessTick() % 40 == 38 )
	{
		int nTroopCnt = m_vecTroop.size();
		for( int i=0; i<nTroopCnt; i++ )
		{
			TroopObjHandle hTroop = m_vecTroop[i];
			if( hTroop->IsEliminated() || hTroop->IsEnable() == false || !hTroop->IsCheckHPInProcess() )
				continue;

			if( hTroop->GetHP() <= 0 )
			{
				hTroop->SetEliminate();
			}
		}
	}

	// Seperate Enable Troops
	int nGroupCnt = m_vecEnableTroops.size();
	for( int i=0; i<nGroupCnt; i++ )
	{
		ENALBE_TROOP_LIST_INFO* pInfo = &(m_vecEnableTroops[i]);
		if( ::GetProcessTick() % pInfo->nOneEnableTick != 0 )
			continue;
		else
		{
			if( pInfo->m_vecTroops.size() == 0 )
			{
				m_vecEnableTroops.erase( m_vecEnableTroops.begin() + i );
				--nGroupCnt;
				--i;
				continue;
			}
			else
			{
				pInfo->m_vecTroops[0]->CmdEnable( true );
				pInfo->m_vecTroops.erase( pInfo->m_vecTroops.begin() );
			}
		}
	}

	ProcessFake();
	PROFILE_TIME_TEST( CFcTroopObject::ProcessObjects() );

	int nUnitCount = 0;	

	g_pSoundManager->ResetUnitCount();
	int nCnt = m_vecTroop.size();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hCurTroop = m_vecTroop[i];

//		if( hCurTroop->IsEliminated() || hCurTroop->IsEnable() == false )
		if( hCurTroop->IsEnable() == false )
			continue;

		std::vector<bool>::iterator it = m_vecFakeState.begin() + i;
		if( CheckFakeTroop( hCurTroop ) == true )
		{
			*it = true;
			hCurTroop->SetRenderSkip( true );
		}
		else
		{
			*it = false;
			hCurTroop->SetRenderSkip( false );
		}

		// *******************
		// ProcessCrowdSound
		// *******************
		int nState = hCurTroop->GetState();
		if(nState == TROOPSTATE_MOVE_ATTACK || nState == TROOPSTATE_MELEE_ATTACK)
		{				
			float fDist = D3DXVec3Length(&(hCurTroop->GetPos() - g_pSoundManager->GetListenerPos(0)));			
			CameraObjHandle CamHandle = CFcBaseObject::GetCameraObjectHandle( 0 );			 
			if( fDist < CamHandle->GetFogFar() )
			{
				if( hCurTroop->IsEnable() && hCurTroop->IsEliminated() == false )
				{
					g_pSoundManager->AddCrowdInfo( hCurTroop );
					
/*
					for( int c = 0;c < hCurTroop->GetUnitCount();c++)
					{
						GameObjHandle hUnit = hCurTroop->GetUnit(c);
						if( hUnit == NULL )
							continue;

						if( hUnit->IsEnable() == false )
							continue;

						if( hUnit->IsDie() )
							continue;

						UnitDataInfo *pData = CUnitSOXLoader::GetInstance().GetUnitData( hUnit->GetUnitSoxIndex() );
						D3DXVECTOR3 UnitPos = hUnit->GetPos();
						g_pSoundManager->AddUnitCount((CROWD_UNIT_TYPE)pData->nCrowdUnitType,&UnitPos);
					}
*/
				}
			}
		}

		// Remove any targets that have been eliminated or disabled now!
		if( hCurTroop->GetMeleeEngageTroopCount() > 0)
			hCurTroop->CheckEngageTroop();
/*
		//
		// All friendly troops are inserted at the front of m_vecTroop, so
		// they have already tested melee against all of the enemies and all of
		// the enemies have tested against them so we don't need to do any more
		// tests as at TGS there are only two groups and enemies don't fight
		// each other....
		//
		if(hCurTroop->GetTeam() != 0)
			continue;
*/
		// **************
		// ProcessMelee()
		// **************
//		for( int j=i+1; j<nCnt; j++ )
		for( int j=0; j<nCnt; j++ )
		{
			TroopObjHandle hTargetTroop = m_vecTroop[j];

			if( hCurTroop == hTargetTroop )
				continue;

			if( hCurTroop->GetTeam() == hTargetTroop->GetTeam() || hTargetTroop->IsEliminated() || hTargetTroop->IsEnable() == false)
				continue;

			bool bCurCanMelee = hCurTroop->CanIMelee();
			bool bTarCanMelee = hTargetTroop->CanIMelee();

			// Can these two troops melee?
			if(IsValidMelee( hCurTroop, hTargetTroop ) && (bCurCanMelee||bTarCanMelee))
			{
				bool bInRange = IsMeleeRange( hCurTroop, hTargetTroop );

				// Compare current against target
/*
				if(bCurCanMelee)
				{
					bool bTargetInCurList = hCurTroop->IsMeleeTarget( hTargetTroop );
					if( !bTargetInCurList && bInRange )
					{
						hCurTroop->AddMeleeEngageTroop( hTargetTroop );
					}
					else if( bTargetInCurList && !bInRange )
					{
						hCurTroop->RemoveMeleeEngageTroop( hTargetTroop );
					}
				}
*/

				bool bTargetInCurList = hCurTroop->IsMeleeTarget( hTargetTroop );
				if( !bTargetInCurList && bInRange )
				{
					if(bCurCanMelee)
						hCurTroop->AddMeleeEngageTroop( hTargetTroop );
				}
				else if( bTargetInCurList && !bInRange )
				{
					hCurTroop->RemoveMeleeEngageTroop( hTargetTroop );
				}

/*
				// Compare target against current
				if(bTarCanMelee)
				{
					bool bCurInTargetList = hTargetTroop->IsMeleeTarget( hCurTroop );
					if( !bCurInTargetList && bInRange )
					{
						hTargetTroop->AddMeleeEngageTroop( hCurTroop );
					}
					else if( bCurInTargetList && !bInRange )
					{
						hTargetTroop->RemoveMeleeEngageTroop( hCurTroop );
					}
				}
*/
			}
//			hTargetTroop->ScatterTroop();
		}
//		hCurTroop->ScatterTroop();
	}

	m_pGuardianManager->Process();
}


#define UNIT_DISTANCE					400.f
#define UNIT_DISTANCE_RANDOM_RANGE		50.f
#define TROOP_ADDTIONAL_RADIUS			400.f


TroopObjHandle CFcTroopManager::AddTroop( CFcWorld* pWorld, TROOP_INFO* pTroopInfo )
{
	PROFILE_FUNC( "AddTroop" );
	int nX = (int)(pTroopInfo->m_areaInfo.fSX + ( pTroopInfo->m_areaInfo.fEX - pTroopInfo->m_areaInfo.fSX ) / 2.f);
	int nY = (int)(pTroopInfo->m_areaInfo.fSZ + ( pTroopInfo->m_areaInfo.fEZ - pTroopInfo->m_areaInfo.fSZ ) / 2.f);

	int nTeam = pTroopInfo->m_troopAttr.nTeam;
	int nGroup = pTroopInfo->m_troopAttr.nGroup;

	bool bLeader = false;
	if( pTroopInfo->m_troopAttr.nLeaderUnitType != -1 )
		bLeader = true;

	int nKindOfUnit = 5;
	int nKindOfHorse = 3;
	for(int j = 0; j < 5; j++)
	{
		if(pTroopInfo->m_troopAttr.nUnitType[ j ] == -1 )
		{
			nKindOfUnit = j;
			break;
		}
	}

/*   말 안탐
	for(int j = 0; j < 3; j++)
	{
		if(pTroopInfo->m_troopAttr.nUnitHorseType[ j ] == -1 )
		{
			nKindOfHorse = j;
			break;
		}
	}
*/
	if( !bLeader && nKindOfUnit == 0 && /*nKindOfHorse == 0 &&*/ pTroopInfo->m_troopAttr.nTroopType != TROOPTYPE_WALLENEMY ) {
		BsAssert( 0 && "Troop unit type isn't setting" );
		DebugString( "%s troop unit type is not setting!\n", pTroopInfo->m_troopAttr.szCaption );
		TroopObjHandle hIdentity;
		return hIdentity;
	}

	TROOPTYPE TroopType = TROOPTYPE_NON;
	if( (TROOPTYPE)pTroopInfo->m_troopAttr.nTroopType != TROOPTYPE_PLAYER_1 &&
		(TROOPTYPE)pTroopInfo->m_troopAttr.nTroopType != TROOPTYPE_PLAYER_2 &&
		(TROOPTYPE)pTroopInfo->m_troopAttr.nTroopType != TROOPTYPE_WALLENEMY )
	{
		if( bLeader && nKindOfUnit == 0 )
		{
			int nLeaderIndex = pTroopInfo->m_troopAttr.nLeaderUnitType;
			TroopType = (TROOPTYPE)CUnitSOXLoader::GetInstance().GetTroopType( nLeaderIndex );
		}
		else
		{
			for( int i=0; i<nKindOfUnit; i++ )
			{
				int nUnitIndex = pTroopInfo->m_troopAttr.nUnitType[i];
				TROOPTYPE CurTroopType = (TROOPTYPE)CUnitSOXLoader::GetInstance().GetTroopType( nUnitIndex );
				if( TroopType == TROOPTYPE_NON )
				{
					TroopType = CurTroopType;
				}
				else
				{
					if( TroopType != CurTroopType )
					{
						BsAssert( 0 && "UnitType missmatch in Troop" );
						DebugString( "UnitType missmatch in Troop" );
					}
				}
			}
		}
	}
	else
	{
		TroopType = (TROOPTYPE)pTroopInfo->m_troopAttr.nTroopType;
	}

	int nUnitCount = 0;
	int nRow = 1;
	int nCol = 1;
	if( nKindOfUnit > 0 /*|| nKindOfHorse > 0*/ ) {
		nRow = pTroopInfo->m_troopAttr.nUnitRow;
		nCol = pTroopInfo->m_troopAttr.nUnitCol;
		nUnitCount = nRow * nCol;
	}
	if( bLeader ) ++nUnitCount;
	TroopObjHandle hTroop = InitTroop( TroopType, pTroopInfo, nX, nY, -1, nUnitCount );
	TroopAIObjHandle TroopAIHandle;
	TroopAIHandle = CFcTroopAIObject::CreateObject<CFcTroopAIObject>();
	TroopAIHandle->Initialize( pTroopInfo->m_troopAttr.nUnitAI, hTroop );

	CCrossVector Cross;
	GameObjHandle Handle;

	float fUnitHP = 777.f;		// 디폴트 777
	if( pTroopInfo->m_troopAttr.nTroopHp > 0 )
		fUnitHP = (float)pTroopInfo->m_troopAttr.nTroopHp / (float)nUnitCount;

	int nLeaderPosType = pTroopInfo->m_troopAttr.nLeaderPos;
	// 포메이션 별로 유닛 위치 계산하고 저장, 방향은 나중에 따로 계산
	float fRange = InitVecFormation( hTroop, bLeader, nLeaderPosType, nRow, nCol, TROOP_FORMATION_TYPE_RECT, pTroopInfo->m_troopAttr.nDensity );

	hTroop->SetRadius( fRange );
//	NextTimeMark( "LoadWorld CreateObject_4\n" );
	float fRot = (float)pTroopInfo->m_troopAttr.nDirection * 3.141592f / 4.f;
	D3DXVECTOR2 Dir;
	Dir.x = sinf( fRot );
	Dir.y = cosf( fRot );
	hTroop->SetDir( Dir );

	hTroop->SetInitUnitData( &(pTroopInfo->m_troopAttr) );
	hTroop->InitUnitArray( nUnitCount );
	hTroop->CmdEnable( pTroopInfo->m_troopAttr.nEnable != 0 );
	return hTroop;
}

/*
GameObjHandle CFcTroopManager::CreateGameObject( int nUnitIndex, CCrossVector &Cross )
{
GameObjHandle Handle;
int nUnitType;

nUnitType = CUnitSOXLoader::GetInstance().GetUnitType( nUnitIndex );
switch( nUnitType ) {
case UNIT_TYPE_DEFAULT:
switch( nUnitIndex )
{
case 0:
Handle=CFcBaseObject::CreateObject< CAspharr >( &Cross );
break;
case 1:
Handle=CFcBaseObject::CreateObject< CInphy >( &Cross );
break;
case 4:
Handle=CFcBaseObject::CreateObject< CKlarrann >( &Cross );
break;
default:
Handle=CFcBaseObject::CreateObject< CFcUnitObject >( &Cross );
break;
}
break;
case UNIT_TYPE_FLY:
Handle=CFcBaseObject::CreateObject< CFcFlyUnitObject >( &Cross );
break;
case UNIT_TYPE_HORSE:
Handle=CFcBaseObject::CreateObject< CFcHorseObject >( &Cross );
break;
case UNIT_TYPE_ARCHER:
Handle =CFcBaseObject::CreateObject< CFcArcherUnitObject >( &Cross );
break;
case UNIT_TYPE_CATAPULT:
Handle=CFcBaseObject::CreateObject< CFcCatapultObject >( &Cross );
break;
case UNIT_TYPE_MOVETOWER:
Handle=CFcBaseObject::CreateObject< CFcUnitObject >( &Cross );
break;
}
return Handle;
}
*/

TroopObjHandle CFcTroopManager::InitTroop( int Type, TROOP_INFO* pTroopInfo, int nX, int nY, int nRange, int nUnitCount )
{
	TroopObjHandle hTroop;
	int nID = pTroopInfo->m_areaInfo.nUId;
	int nForce = pTroopInfo->m_troopAttr.nTeam;
	int nTeam = pTroopInfo->m_troopAttr.nGroup;
	char* pCaption = pTroopInfo->m_troopAttr.szCaption;
	int nLeaderLevel = pTroopInfo->m_troopAttr.nLeaderLevel;
	int nTroopLevel = pTroopInfo->m_troopAttr.nTroopLevel;
	bool bEnableGuardian = (pTroopInfo->m_troopAttr.nGuardPickout != 0 );

	// pTroopInfo->m_troopAttr.nTroopType 사용 안함
	switch( Type )
	{
		case TROOPTYPE_PLAYER_1:
		case TROOPTYPE_PLAYER_2:
			hTroop = CFcTroopObject::CreateObject< CFcPlayerTroop >();
			hTroop->CmdEnable( true );
			((CFcPlayerTroop*)hTroop.GetPointer())->Initialize( nX, nY, 500,				// 플레이어 부대 영역은 반지금 5미터
				nForce, nTeam, nID, pCaption, 
				pTroopInfo->m_troopAttr.nTroopType, 0 );
			break;
		case TROOPTYPE_RANGEMELEE:
		case TROOPTYPE_RANGE:
			{
				hTroop = CFcTroopObject::CreateObject< CFcRangeTroop >();
				// TroopType 하드코딩!! 맵툴에 추가해야 함
				CFcRangeTroop* pRange = (CFcRangeTroop*)hTroop.GetPointer();
				// 화살 공격 범위는 내부에서 TroopTable, Level 보고 결정
				pRange->Initialize( nX, nY, nRange, 
					nForce, nTeam, nID, pCaption,
					Type, nUnitCount );
			}
			break;
		case -1: //셋팅 안되있는에들두 있어서 일단 너놀께요..
			DebugString( "Troop %s type is not setting\n", pCaption );
		case TROOPTYPE_SPEAR:
		case TROOPTYPE_MELEE:
			hTroop = CFcTroopObject::CreateObject< CFcTroopObject >();
			hTroop->Initialize( nX, nY, nRange, nForce, nTeam, 
								nID, pCaption, TROOPTYPE_MELEE, nUnitCount );
			break;
		case TROOPTYPE_SIEGE:
			hTroop = CFcTroopObject::CreateObject< CFcCatapultTroop >();
			hTroop->Initialize( nX, nY, nRange, nForce, nTeam, 
								nID, pCaption, TROOPTYPE_SIEGE, nUnitCount );
			break;
		case TROOPTYPE_FLYING:
			{
				hTroop = CFcTroopObject::CreateObject< CFcFlyTroop >();
				CFcFlyTroop*  pFly = (CFcFlyTroop*)hTroop.GetPointer();
				pFly->Initialize( nX, nY, nRange, 
									nForce, nTeam, nID, pCaption,
									TROOPTYPE_FLYING, nUnitCount );
			}
			break;
		case TROOPTYPE_MOVETOWER:
			// Melee 타입 체크하는 루틴 필요
			hTroop = CFcTroopObject::CreateObject< CFcMoveTower >();
			// TroopType 하드코딩!! 맵툴에 추가해야 함
			hTroop->Initialize( nX, nY, nRange, nForce, nTeam, 
								nID, pCaption, TROOPTYPE_MOVETOWER, nUnitCount );
			break;
		case TROOPTYPE_WALLENEMY:
			hTroop = CFcTroopObject::CreateObject< CFcWallEnemyTroop >();
			hTroop->Initialize( nX, nY, nRange, nForce, nTeam, 
				nID, pCaption, TROOPTYPE_WALLENEMY, nUnitCount );
			break;

		case TROOPTYPE_NOBATTLE:
			hTroop = CFcTroopObject::CreateObject< CFcCitizenTroop >();
			hTroop->Initialize( nX, nY, nRange, nForce, nTeam, 
				nID, pCaption, TROOPTYPE_NOBATTLE, nUnitCount );
			break;
		default:
			BsAssert(0&&"Unknown Troop Type");
			return hTroop;
	}
/*
	// 나머지 셋팅할것들..
	if( pTroopInfo->m_troopAttr.nMoveSpeed > 0 )
		hTroop->SetMoveSpeed( (float)pTroopInfo->m_troopAttr.nMoveSpeed );
*/

	hTroop->SetEnableGuardian( bEnableGuardian );

	if(nTeam == 0) // 아군
		m_vecTroop.insert(m_vecTroop.begin(), hTroop);
	else
		m_vecTroop.push_back( hTroop );

	return hTroop;
}


#define CROSS_DENSITY	100


float CFcTroopManager::InitVecFormation( TroopObjHandle hTroop, bool bLeader, int nLeaderPosType, int nRow, int nCol, TROOP_FORMATION_TYPE FormationType, int nDensity)
{
	if( nDensity == -1 )
		nDensity = 500;
	else
		nDensity *= 100;

	int nUnitCount = nRow * nCol;
	if( bLeader )
	{
		if( nRow == 1 && nCol == 1 )		// 부대원이 리더 하나라면 중앙에 배치
		{
			D3DXVECTOR2 Dir;
			Dir.x = 0.f;
			Dir.y = 1.f;
			float fDist = 0.f;
			hTroop->AddVecFormation( 0, Dir, fDist );
		}
		else if( nLeaderPosType == -1 || nLeaderPosType == 0 )		// 전방
		{
			D3DXVECTOR2 Dir;
			Dir.x = 0.f;
			Dir.y = (float)( ( nCol * nDensity ) / 2 + nDensity );
			float fDist = D3DXVec2Length( &Dir );
			D3DXVec2Normalize( &Dir, &Dir );
			hTroop->AddVecFormation( 0, Dir, fDist );
		}
		else
		{
			BsAssert(0);
		}
	}

	BsAssert( nRow > 0 );
	BsAssert( nCol > 0 );

	float fRange = (float) sqrt( (float)(nRow * nDensity) * (float)(nRow * nDensity) + (float)(nCol * nDensity) * (float)(nCol * nDensity) ) / 2;
	fRange += TROOP_ADDTIONAL_RADIUS;


	float fRandomRange = 0.f;
	if( nUnitCount > 1 )
	{
		fRandomRange = UNIT_DISTANCE_RANDOM_RANGE;
	}

	if( hTroop->GetType() != TROOPTYPE_WALLENEMY )
	{
		// 부대 중심부터의 오프셋 계산
		float fSX = - (float)( ( nRow - 1 ) * nDensity ) / 2.f;
		float fSY = - (float)( ( nCol - 1 ) * nDensity ) / 2.f;
		for( int i=0; i<nUnitCount; i++ )
		{
			float fPosX = (float)(i % nRow);
			float fPosY = (float)(i / nRow);
			float fCurX = fSX + fPosX * (float)nDensity;
			float fCurY = fSY + fPosY * (float)nDensity;
			D3DXVECTOR2 Dir;
			Dir.x = RandomNumberInRange( fCurX - UNIT_DISTANCE_RANDOM_RANGE,  fCurX + UNIT_DISTANCE_RANDOM_RANGE );
			Dir.y = RandomNumberInRange( fCurY - UNIT_DISTANCE_RANDOM_RANGE,  fCurY + UNIT_DISTANCE_RANDOM_RANGE );
			float fDist = D3DXVec2Length( &Dir );
			D3DXVec2Normalize( &Dir, &Dir );
			hTroop->AddVecFormation( 0, Dir, fDist );
		}
		///////////////////////////////
		// 다리나 성문 지나가는 대형
		nDensity = CROSS_DENSITY;
		nRow = 3;
		nCol = nUnitCount/3;

		if( bLeader )
		{
			if( nLeaderPosType == -1 || nLeaderPosType == 0 )		// 전방
			{
				D3DXVECTOR2 Dir;
				Dir.x = 0;
				Dir.y = (float)( ( nCol * nDensity ) / 2 + nDensity );
				float fDist = D3DXVec2Length( &Dir );
				D3DXVec2Normalize( &Dir, &Dir );
				hTroop->AddVecFormation( 1, Dir, fDist );
			}
			else
			{
				BsAssert(0);
			}
		}

		fSX = - (float)( nRow * nDensity ) / 2.f;
		fSY = - (float)( nCol * nDensity ) / 2.f;
		for( int i = 0; i < nUnitCount; i++ )
		{
			float fPosX = (float)(i % nRow);
			float fPosY = (float)(i / nRow);
			float fCurX = fSX + fPosX * (float)nDensity;
			float fCurY = fSY + fPosY * (float)nDensity;

			D3DXVECTOR2 Dir;
			Dir.x = fCurX;
			Dir.y = fCurY;
			float fDist = D3DXVec2Length( &Dir );
			D3DXVec2Normalize( &Dir, &Dir );
			hTroop->AddVecFormation( 1, Dir, fDist );
		}
	}
	else
	{
		nDensity = CROSS_DENSITY;
		nRow = nUnitCount/2;
		nCol = 2;

		BsAssert(nRow); //aleksger: prefix bug 624: make sure nRow is not zero.

		float fSX = - (float)( nRow * nDensity ) / 2.f;
		float fSY = - (float)( nCol * nDensity ) / 2.f;
		for( int i = 0; i < nUnitCount; i++ )
		{
			float fPosX = (float)(i % nRow);
			float fPosY = (float)(i / nRow);
			float fCurX = fSX + fPosX * (float)nDensity;
			float fCurY = fSY + fPosY * (float)nDensity;

			D3DXVECTOR2 Dir;
			Dir.x = fCurX;
			Dir.y = fCurY;
			float fDist = D3DXVec2Length( &Dir );
			D3DXVec2Normalize( &Dir, &Dir );
			hTroop->AddVecFormation( 0, Dir, fDist );
		}
	}
	return fRange;
}

void CFcTroopManager::ProcessFake()
{
	int nCnt = m_vecTroop.size();
	for( int i=0; i<nCnt; i++ )
	{
		bool bFake = m_vecFakeState[i];
		TroopObjHandle hTroop = m_vecTroop[i];
		if( bFake == false )
		{
			hTroop->SetFakeMode( false );
			continue;
		}
	
		if( hTroop->IsEliminated() )
			continue;
		if( !hTroop->IsEnable() )
			continue;

		if( hTroop->IsMeleeEngaged() )
		{
			int nMeleeTroopCnt = hTroop->GetMeleeEngageTroopCount();
			for( int j=0; j<nMeleeTroopCnt; j++ )
			{
				TroopObjHandle hMeleeTroop = hTroop->GetMeleeEngageTroop( j );
				int nIndex = hMeleeTroop->GetIndex();
				if( m_vecFakeState[nIndex] == false )
				{
					bFake = false;
					break;
				}
			}
		}
		else if( hTroop->IsRangeEngaged() )
		{
			TroopObjHandle hRangeTroop = hTroop->GetRangeTarget();
			int nIndex = hRangeTroop->GetIndex();
			if( m_vecFakeState[nIndex] == false )
			{
				bFake = false;
			}
		}
		hTroop->SetFakeMode( bFake );
		CaculateFakeMode( hTroop );
	}
}



bool CFcTroopManager::RegisterGuardian( int nIndex )
{
	return m_pGuardianManager->AddGuardian( nIndex );
}

bool CFcTroopManager::RegisterCancelGuardian( int nIndex )
{
	return m_pGuardianManager->RemoveGuardian( nIndex );
}


int CFcTroopManager::GetGuardianNum()
{
	return m_pGuardianManager->GetGuardianNum();
}


bool CFcTroopManager::IsGuardian( TroopObjHandle hTroop )
{
	return m_pGuardianManager->IsGuardian( hTroop );
}

void CFcTroopManager::CreateUnits( TroopObjHandle hTroop, TROOP_ATTR* pInitInfo )
{
	GameObjHandle Handle;
	bool bLeader = false;
	float fMaxMoveSpeed = 0.f;
	float fMaxWalkMoveSpeed = 0.f;
	if( pInitInfo->nLeaderUnitType != -1 )
		bLeader = true;

	int nForce = pInitInfo->nTeam;
	int nTeam = pInitInfo->nGroup;

	int nKindOfUnit = 5;
//	int nKindOfHorse = 3;
	for(int j = 0; j < 5; j++)
	{
		if(pInitInfo->nUnitType[ j ] == -1 )
		{
			nKindOfUnit = j;
			break;
		}
	}
/* 말 안탐
	for(int j = 0; j < 3; j++)
	{
		if(pInitInfo->nUnitHorseType[ j ] == -1 )
		{
			nKindOfHorse = j;
			break;
		}
	}
*/
	int nUnitCount = 0;
	int nRow = 1;
	int nCol = 1;
//	if( nKindOfUnit > 0 || nKindOfHorse > 0 ) {
	if( nKindOfUnit > 0 ) {
		nRow = pInitInfo->nUnitRow;
		nCol = pInitInfo->nUnitCol;
		nUnitCount = nRow * nCol;
	}
	if( bLeader ) ++nUnitCount;

	hTroop->InitUnitArray( nUnitCount );
	
	for( int i=0; i<nUnitCount; i++ )
	{
		CCrossVector Cross;
		hTroop->GetUnitFormationCross( i, &Cross );

		int nUnitIndex = -1;
		if( bLeader || nKindOfUnit > 0 ) {
			int nRandomValue = Random( nKindOfUnit );

			int nLevel = 0;
			if( bLeader && i == 0 )
			{
				nUnitIndex = pInitInfo->nLeaderUnitType;
				nLevel = CLevelTableLoader::GetInstance().GetMatchingLevel( nUnitIndex, pInitInfo->nLeaderLevel );
			}
			else
			{
				nUnitIndex = pInitInfo->nUnitType[nRandomValue];
				nLevel = CLevelTableLoader::GetInstance().GetMatchingLevel( nUnitIndex, pInitInfo->nTroopLevel );
			}
DebugString( "CU! %d, %d, %d, %d\n", nUnitIndex, pInitInfo->nLeaderLevel, pInitInfo->nTroopLevel, nRandomValue );

			if( nUnitIndex != -1 ) {
				float fMinScale = (float)CUnitSOXLoader::GetInstance().GetMinScale( nUnitIndex );
				float fMaxScale = (float)CUnitSOXLoader::GetInstance().GetMinScale( nUnitIndex );

				Handle = CFcGameObject::CreateGameObject( (CFcGameObject::GameObj_ClassID)CUnitSOXLoader::GetInstance().GetUnitType( nUnitIndex ), &Cross, nUnitIndex );

				INITGAMEOBJECTDATA* pInitData = NULL;

				// GetInitGameObjectData내부에서 GetMatchingLevel를 불러준다.
				if( bLeader && i == 0 )
					pInitData = g_FcWorld.GetInitGameObjectData( nUnitIndex, pInitInfo->nLeaderLevel );
				else
					pInitData = g_FcWorld.GetInitGameObjectData( nUnitIndex, pInitInfo->nTroopLevel );

				Handle->Initialize( pInitData, nForce, nTeam, hTroop, fMinScale/100.f, fMaxScale/100.f );

				if( bLeader && i == 0 )
					hTroop->AddLeader( Handle );

				Handle->SetLevel( nLevel );	// 캐릭터 레벨을 정해주면 HP 자동으로 셋팅된다..

				// Create AI
				AIObjHandle AIHandle;
				AIHandle = CFcAIObject::CreateObject<CFcAIObject>();
				char szStr[64];
				if( bLeader && i == 0 )
					strcpy_s( szStr, _countof(szStr),pInitInfo->StrBlock.szLeaderUnitAI); //aleksger: prefix bug 625: Safe CRT strings
				else
					strcpy_s( szStr, _countof(szStr), pInitInfo->StrBlock.szUnitAI[nRandomValue]);//aleksger: prefix bug 625: Safe CRT strings

				if( strlen( szStr ) == 0 || atoi(szStr) == -1 || szStr[0] == -1 )
					sprintf( szStr, "%s_Default.ai", CUnitSOXLoader::GetInstance().GetUnitData( nUnitIndex )->cUnitName );//aleksger: prefix bug 625: Safe CRT strings

				if( AIHandle->Initialize( szStr, Handle ) == false )
					delete AIHandle;
				else
					Handle->SetAIHandle( AIHandle );

				hTroop->AddUnit( Handle, i );
				Handle->CFcBaseObject::Enable( false );

				// 임시 by Siva
				if( Handle->GetClassID() == CFcGameObject::Class_ID_Adjutant || 
					Handle->GetClassID() == CFcGameObject::Class_ID_Hero )
					hTroop->SetCheckHPInProcess( false );

				// 속도 체크 유닛 다 체크해서 느리다
				float fCurMaxMoveSpeed = 0.f;
				float fCurMaxWalkMoveSpeed = 0.f;
//				if( Handle->GetClassID() >= CFcGameObject::Class_ID_Hero && ((CFcHeroObject*)Handle.m_pInstance)->GetPlayerIndex() != -1 )
				if( Handle->GetClassID() >= CFcGameObject::Class_ID_Hero && pInitData->nUnitIndex <= 6 )
				{
					fCurMaxMoveSpeed = CHeroLevelTableLoader::GetInstance().GetMoveSpeed( nUnitIndex, Handle->GetLevel() );
					fCurMaxWalkMoveSpeed = fCurMaxMoveSpeed * 0.5f;
				}
				else
				{
					CAniInfoData* pUnitInfoData = CAniInfoData::GetData( pInitData->nAniInfoDataIndex );
					int nAniIndex = pUnitInfoData->GetAniIndex( ANI_TYPE_RUN, 0 );
					
					if( nAniIndex != -1 )
						fCurMaxMoveSpeed = (float)pUnitInfoData->GetMoveSpeed( nAniIndex );
					
					nAniIndex = pUnitInfoData->GetAniIndex( ANI_TYPE_WALK, 0 );
					if( nAniIndex != -1 )
						fCurMaxWalkMoveSpeed = (float)pUnitInfoData->GetMoveSpeed( nAniIndex );
				}

				if( fMaxMoveSpeed < fCurMaxMoveSpeed )
					fMaxMoveSpeed = fCurMaxMoveSpeed;

				if( fMaxWalkMoveSpeed < fCurMaxWalkMoveSpeed )
					fMaxWalkMoveSpeed = fCurMaxWalkMoveSpeed;
			}
		}
/* 말 안탐
		// 말태우기..
		if( bLeader || nKindOfHorse > 0 )
		{
			HorseObjHandle Horse;
			int nHorseIndex;
			if( bLeader && i == 0 ) nHorseIndex = pInitInfo->nLeaderHorseType;
			else nHorseIndex = pInitInfo->nUnitHorseType[ Random( nKindOfHorse ) ];

			if( nHorseIndex != -1 ) {
				Horse = CFcGameObject::CreateGameObject( CFcGameObject::Class_ID_Horse, &Cross ); //(CFcGameObject::GameObj_ClassID)CUnitSOXLoader::GetInstance().GetUnitType( nHorseIndex )
				Horse->Initialize( nHorseIndex, nTeam, nGroup, hTroop );	// 말탄 애들의 천 시뮬 때문에 말 크기는 1.0 고정으로 한다..
				g_FcWorld.AddUnitToScanner( Horse );
				hTroop->AddUnit( Horse );
				Horse->SetLevel( Handle->GetLevel() );

				// Create AI
				if( nUnitIndex != -1 ) {
					char szFileName[64];
					AIObjHandle AIHandle;
					AIHandle = CFcAIObject::CreateObject<CFcAIObject>();
					sprintf( szFileName, "%s_horse.ai", CUnitSOXLoader::GetInstance().GetUnitData( nUnitIndex )->cUnitName );
					if( AIHandle->Initialize( szFileName, Horse ) == false ) {
						delete AIHandle;
					}
					else Horse->SetAIHandle( AIHandle );
					Handle->RideOn( Horse );
				}
			}
		}
*/
	}

	hTroop->PostUnitArray();
	
	// 이동 속도 세팅 
	// Unit들 다 만들어주고 해야 함
	float fSOXMoveSpeed = g_TroopSOX.GetMoveSpeed( hTroop->GetType() );
	if( fSOXMoveSpeed > 0.f )
	{
		hTroop->SetMoveSpeed( fSOXMoveSpeed );
		hTroop->SetWalkSpeedRate( fSOXMoveSpeed * 0.5f );
	}
	else
	{
		hTroop->SetMoveSpeed( fMaxMoveSpeed );
		if( fMaxMoveSpeed > 0.f && fMaxWalkMoveSpeed > 0.f )
			hTroop->SetWalkSpeedRate( fMaxWalkMoveSpeed / fMaxMoveSpeed );
		else
			hTroop->SetWalkSpeedRate( 0.5f );
	}
	hTroop->PostInitialize();
}



float CFcTroopManager::GetGuardianLiveRatePercent()
{
	float fMaxHP = 0.f;
	float fHP = 0.f;
	for( int i=0; i<2; i++ )
	{
		if( m_FirstGuardian[i] )
		{
			TroopObjHandle hTroop = m_FirstGuardian[i];
			fMaxHP += hTroop->GetMaxHP();
			fHP += hTroop->GetHP();
		}
	}

	if( fMaxHP == 0.f )
		return 0.f;

	return ( fHP / fMaxHP * 100.f );
}

float CFcTroopManager::GetFriendlyTroopsLiveRatePercent()
{
	int nTotalFriendly = 0;
	int nSurvivor = 0;
	
	int nCnt = GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hHandle = GetTroop( i );
		if( hHandle->GetTeam()!=0)
			continue;
		nTotalFriendly += hHandle->GetUnitCount();
        nSurvivor += hHandle->GetAliveUnitCount();	
	}
	if(nSurvivor > nTotalFriendly)
		nSurvivor = nTotalFriendly;
	
	return ((float)nSurvivor/nTotalFriendly)*100;
}

#include "FcCameraObject.h"
#include "CrossVector.h"
TroopObjHandle CFcTroopManager::AddPlayerTroop( CFcWorld *pWorld, TROOP_INFO *pTroopInfo )
{
	BsAssert( m_hPlayerTroop == NULL );
	int nSX = (int)( pTroopInfo->m_areaInfo.fSX + ( pTroopInfo->m_areaInfo.fEX - pTroopInfo->m_areaInfo.fSX ) / 2.f );
	int nSY = (int)( pTroopInfo->m_areaInfo.fSZ + ( pTroopInfo->m_areaInfo.fEZ - pTroopInfo->m_areaInfo.fSZ ) / 2.f );

	TroopObjHandle hTroop = InitTroop( pTroopInfo->m_troopAttr.nTroopType, pTroopInfo, nSX, nSY, 500, 0 );
	HeroObjHandle hHero = CFcWorld::GetInstance().CreateHero( pTroopInfo->m_troopAttr.nLeaderUnitType, (float)nSX, (float)nSY );

	// GetInitGameObjectData 내부에서 GetMatchingLevel을 불러준다.
	INITGAMEOBJECTDATA* pInitData = g_FcWorld.GetInitGameObjectData( pTroopInfo->m_troopAttr.nLeaderUnitType, pTroopInfo->m_troopAttr.nLeaderLevel );
	hHero->Initialize( pInitData, pTroopInfo->m_troopAttr.nTeam, pTroopInfo->m_troopAttr.nGroup,
						hTroop, 1.f, 1.f);		// 주인공 스케일은 1.0으로 고정
	
	int nLevel = CLevelTableLoader::GetInstance().GetMatchingLevel( pTroopInfo->m_troopAttr.nLeaderUnitType, pTroopInfo->m_troopAttr.nLeaderLevel );
	if( g_FCGameData.nPlayerType != MISSION_TYPE_NONE )
	{
		hHero->SetLevel( g_FCGameData.tempUserSave.nLevel );
		hHero->SetExp( g_FCGameData.tempUserSave.nExp );
		hTroop->SetLevel( g_FCGameData.tempUserSave.nLevel );

		int nEnemyLevelUp = 0;
/*
		int nHeroID = g_MissionToSoxID[g_FCGameData.nPlayerType];
		HeroRecordInfo* pHeroRecordInfo = g_FCGameData.tempFixedSave.GetHeroRecordInfo(nHeroID);
		if(pHeroRecordInfo)
		{
			StageResultInfo* pStageResultInfo = pHeroRecordInfo->GetStageResultInfo(g_FCGameData.SelStageId);
			if(pStageResultInfo){
				nEnemyLevelUp = pStageResultInfo->nEnemyLevelUp;
			}
		}
*/
	}
	else
	{
		hHero->SetLevel( nLevel );
		hTroop->SetLevel( nLevel );
	}
/*
	int nHeroId = g_MissionToSoxID[g_FCGameData.nPlayerType];
	HeroRecordInfo* pRecordInfo = g_FCGameData.tempFixedSave.GetHeroRecordInfo(nHeroId);
*/
	
	if( ( g_FCGameData.nPlayerType != MISSION_TYPE_NONE && g_FCGameData.PrevStageId != STAGE_ID_NONE ) )
	{	
		for( int i=0; i<g_FcItemManager.GetItemCount(); i++ )
		{
			HeroEquip* pEquip = g_FcItemManager.GetHeroEquip(i);
			if( pEquip->bAccoutered )
			{
				ItemDataTable* pItemData = g_FcItemManager.GetItemDataSoxDirectly( pEquip->nItemSoxID );
				if( pItemData->nItemType == ITP_WEAPON )
				{
					HeroObjHandle Handle = g_FcWorld.GetHeroHandle();
/*
					char cSkinName[256];		
					sprintf(cSkinName, "Weapons\\%s.skin", pItemData->szSkinName );
					Handle->ChangeWeapon( cSkinName, 0 );
*/

					bool bRequireRelease = false; // Indicate that we did a load here.

					int nSkinIndex = g_FcItemManager.GetSkinIndex( pEquip->nItemSoxID );
					if( nSkinIndex < 0 )
					{
						char cSkinName[256];		
						sprintf(cSkinName, "Weapons\\%s.skin", pItemData->szSkinName );
						nSkinIndex = g_BsKernel.LoadSkin( -1, cSkinName );

						bRequireRelease = true;
					}

					Handle->ChangeWeapon( nSkinIndex, 0 );

					if ( bRequireRelease )
						g_BsKernel.ReleaseSkin( nSkinIndex );
				}
			}
		}
	}
	else
	{
		// 디폴트 무기 장착!
		int nWeaponSoxIndex = -1;
		int nAccrySoxIndex  = -1;

		switch(pWorld->GetHeroHandle()->GetHeroClassID())
		{
		case CFcGameObject::Class_ID_Hero_Aspharr:
			nWeaponSoxIndex = ITEM_CHURCH_KNIGHT_SPEAR;
			break;
		case CFcGameObject::Class_ID_Hero_Inphy:
			nWeaponSoxIndex = ITEM_CHURCH_KNIGHT_SWORD;
			break;
		case CFcGameObject::Class_ID_Hero_Klarrann:
			nWeaponSoxIndex = ITEM_OLD_ECON;
			break;

		case CFcGameObject::Class_ID_Hero_VigkVagk:			
			break;

		case CFcGameObject::Class_ID_Hero_Myifee:
			nWeaponSoxIndex = ITEM_DOUBLE_EDGE;
			break;
		case CFcGameObject::Class_ID_Hero_Dwingvatt:
			nWeaponSoxIndex = ITEM_PANG_DAGGER;
			break;
		case CFcGameObject::Class_ID_Hero_Tyurru:
			nWeaponSoxIndex = ITEM_MAGIC_SCHOOL_WAND;
			break;
		}

		if(nWeaponSoxIndex != -1){
			g_FcItemManager.AddItem( nWeaponSoxIndex, true, false ); 
/*
			char cSkinName[256];		
			sprintf(cSkinName, "Weapons\\%s.skin", g_FcItemManager.GetItemDataSoxDirectly(nWeaponSoxIndex)->szSkinName );
			
			Handle->ChangeWeapon( cSkinName, 0 );
*/

			int nSkinIndex = g_FcItemManager.GetSkinIndex( nWeaponSoxIndex );
			
			if ( nSkinIndex != -1 )
			{
				g_FcWorld.GetHeroHandle()->ChangeWeapon( nSkinIndex, 0 );
			}
			else
			{
				char cSkinName[256];
				sprintf(cSkinName, "Weapons\\%s.skin", g_FcItemManager.GetItemDataSoxDirectly(nWeaponSoxIndex)->szSkinName );
				nSkinIndex = g_BsKernel.LoadSkin( -1, cSkinName );

				g_FcWorld.GetHeroHandle()->ChangeWeapon( nSkinIndex, 0 );

				g_BsKernel.ReleaseSkin( nSkinIndex );
			}
		}
		if(nAccrySoxIndex != -1){ g_FcItemManager.AddItem( nAccrySoxIndex, true, false ); }
	}
	hTroop->InitUnitArray( 1 );

	hTroop->AddLeader( hHero );
	hTroop->AddUnit( hHero, 0 );

	m_hPlayerTroop = hTroop;

	CameraObjHandle CamHandle=CFcBaseObject::GetCameraObjectHandle(0);
	CamHandle->SetTargetTroop( hHero->GetTroop() );

	// 방향 설정
	float fRot = (float)pTroopInfo->m_troopAttr.nDirection * 3.141592f / 4.f;

	D3DXVECTOR2 Dir;
	Dir.x = sinf( fRot );
	Dir.y = cosf( fRot );

	hTroop->SetDir( Dir );

	// 방향 맞추기
	CCrossVector* pHeroCross = hHero->GetCrossVector();
	D3DXVECTOR3 TargetPos = pHeroCross->m_PosVector + D3DXVECTOR3( Dir.x, 0.f, Dir.y );
	pHeroCross->LookAt( &TargetPos );
	
	CamHandle->GetCrossVector()->RotateYawByWorld( (int)(( fRot * 4.f ) * 4) );

	// Hero AI 등록
	// 이벤트 발생시 사용하기 위해서임.. 기본으로 Disable시켜논다.
	char szFileName[64];
	AIObjHandle AIHandle;
	AIHandle = CFcAIObject::CreateObject<CFcAIObject>();
	sprintf( szFileName, "%s_Player.ai", CUnitSOXLoader::GetInstance().GetUnitData(pTroopInfo->m_troopAttr.nLeaderUnitType)->cUnitName );
	if( AIHandle->Initialize( szFileName, hHero ) == false ) {
		delete AIHandle;
	}
	else {
		hHero->SetAIHandle( AIHandle );
		AIHandle->SetEnable( false );
	}

/* 말 안탐
	int nHorseIndex = pTroopInfo->m_troopAttr.nLeaderHorseType;
	if( nHorseIndex != -1 ) {
		CCrossVector Cross;
		HorseObjHandle Horse = CFcGameObject::CreateGameObject( CFcGameObject::Class_ID_Horse, hHero->GetCrossVector() );

		Horse->Initialize( nHorseIndex, hHero->GetForce(), hHero->GetTeam(), hTroop, 1.0f, 1.0f );	// 말탄 애들의 천 시뮬 때문에 말 크기는 1.0 고정으로 한다..
		pWorld->AddUnitToScanner( Horse );
		hTroop->AddUnit( Horse );
		Horse->SetLevel( hHero->GetLevel() );

		// Create AI
		char szFileName[64];
		AIObjHandle AIHandle;
		AIHandle = CFcAIObject::CreateObject<CFcAIObject>();
		sprintf( szFileName, "%s_Player.ai", CUnitSOXLoader::GetInstance().GetUnitData( pTroopInfo->m_troopAttr.nLeaderUnitType )->cUnitName );
		if( AIHandle->Initialize( szFileName, Horse ) == false ) {
			delete AIHandle;
		}
		else {
			Horse->SetAIHandle( AIHandle );
			AIHandle->SetEnable( true );
		}

		hHero->RideOn( Horse );
		hHero->ChangeAnimation( ANI_TYPE_STAND, 0, ANI_ATTR_HORSE );
	}
*/
	// Player movespeed는 Process에서 갱신한다.
	hTroop->PostInitialize();
	return hTroop;
}


TroopObjHandle CFcTroopManager::AddTroopOnProp( CFcWorld* pWorld, CFcProp* pProp )
{
	TroopObjHandle hTroop = CFcTroopObject::CreateObject< CFcOnPropTroop >();
	CFcOnPropTroop* pOnPropTroop = (CFcOnPropTroop*)hTroop.GetPointer();
	pOnPropTroop->Initialize( pProp );

	//	hTroop->CmdEnable( pTroopInfo->m_troopAttr.nEnable != 0 );
	hTroop->SetMoveSpeed( 0.f );

	int nUnitIndex = pProp->GetParam(0);
	int nForce = pProp->GetParam(1);
	int nTeam = pProp->GetParam(2);
	int nMapLevel = pProp->GetParam(3);
	int nBookingCnt = pProp->GetBookingNum();
	float fMinScale = (float)CUnitSOXLoader::GetInstance().GetMinScale( nUnitIndex );
	float fMaxScale = (float)CUnitSOXLoader::GetInstance().GetMaxScale( nUnitIndex );

	if(nTeam == 0)
		m_vecTroop.insert(m_vecTroop.begin(), hTroop);
	else
		m_vecTroop.push_back( hTroop );

	BsAssert( nBookingCnt > 0 );

	hTroop->InitUnitArray( nBookingCnt );

	INITGAMEOBJECTDATA* pInitData = g_FcWorld.GetInitGameObjectData( nUnitIndex, nMapLevel );
	if( pInitData == NULL )
	{
		DebugString( "이 와치타워에 있는 유닛이 다른 부대에 유닛으로 존재해야 합니다.\n" );
		BsAssert( 0 );
	}

	int nLevel = CLevelTableLoader::GetInstance().GetMatchingLevel( nUnitIndex, nMapLevel );
	for( int i=0; i<nBookingCnt; i++ )
	{
		CCrossVector Cross;
		D3DXMATRIX matObject;
		matObject = *(D3DXMATRIX *)g_BsKernel.SendMessage( pProp->GetEngineIndex(), BS_GET_OBJECT_MAT );

		Cross.m_PosVector = *(pProp->GetBookingPos( i ));// + pProp->GetPos();
		D3DXVec3TransformCoord( &Cross.m_PosVector, &Cross.m_PosVector, &matObject );

		GameObjHandle Handle = CFcGameObject::CreateGameObject( (CFcGameObject::GameObj_ClassID)CUnitSOXLoader::GetInstance().GetUnitType( nUnitIndex ), &Cross, nUnitIndex );
		Handle->Initialize( pInitData, nForce, nTeam, hTroop, fMinScale/100.f, fMaxScale/100.f );

		Handle->SetLevel( nLevel );	// 캐릭터 레벨을 정해주면 HP 자동으로 셋팅된다..
		Handle->SetInScanner( true );
		hTroop->AddUnit( Handle, i );
		Handle->SetFreezePos( true );
		Handle->SetOnPropIndex( (int)pProp );

		// Initialize AI
		char szFileName[64];
		AIObjHandle AIHandle;
		AIHandle = CFcAIObject::CreateObject<CFcAIObject>();
		sprintf( szFileName, "%s_Prop.ai", CUnitSOXLoader::GetInstance().GetUnitData(nUnitIndex)->cUnitName );
		if( AIHandle->Initialize( szFileName, Handle ) == false ) {
			delete AIHandle;
		}
		else Handle->SetAIHandle( AIHandle );
	}

	// Unit들 다 만들어주고 해야 함
	hTroop->PostInitialize();
	hTroop->CmdEnable( true );

	return hTroop;
}


TroopObjHandle CFcTroopManager::AddTroopWallDummy( CFcWorld* pWorld, CFcProp* pProp )
{
	TroopObjHandle hTroop = CFcTroopObject::CreateObject< CFcWallDummyTroop >();
	CFcWallDummyTroop* pDummyTroop = (CFcWallDummyTroop*)hTroop.GetPointer();
	pDummyTroop->Initialize( pProp );
	hTroop->SetMoveSpeed( 0.f );
	int nTeam = pProp->GetParam( 2 );
	if(nTeam == 0)
		m_vecTroop.insert(m_vecTroop.begin(), hTroop);
	else
		m_vecTroop.push_back( hTroop );

	return hTroop;
}



const D3DXVECTOR3& CFcTroopManager::GetPos( int nIndex )
{
	int nCnt = m_vecTroop.size();
	BsAssert( nCnt > nIndex );
	return m_vecTroop[nIndex]->GetPos();
}




void CFcTroopManager::CmdMove( int nIndex, float fX, float fY )
{
	int nCnt = m_vecTroop.size();
	BsAssert( nCnt > nIndex );
	m_vecTroop[nIndex]->CmdMove( fX, fY, 1.f );
}



void CFcTroopManager::CmdGuardianAttack()
{
	m_pGuardianManager->CmdAttack();
}

void CFcTroopManager::CmdGuardianDefense()
{
	m_pGuardianManager->CmdDefense();
}


bool CFcTroopManager::CmdRegisterGuardian( int nIndex )
{
	TroopObjHandle hTroop = m_pGuardianManager->GetGuardianTroop( nIndex );
	if( hTroop )
	{
		return RegisterCancelGuardian( nIndex );
	}
	else
	{
		return RegisterGuardian( nIndex );
	}
	return false;
}


void CFcTroopManager::AddHero( HeroObjHandle Handle )
{
	if( m_hHero )		// changehero 치트키 사용했을 때
	{
		m_hHero.Identity();
		m_hHero = Handle;

		m_hPlayerTroop->AddLeader( m_hHero );
		m_hHero->SetTroop( m_hPlayerTroop );
	}
	else
	{
		m_hHero = Handle;
	}	
}

void CFcTroopManager::GetListInRange( D3DXVECTOR2 *pPosition, float fRadius, std::vector<TroopObjHandle> &Objects )
{
	Objects.clear();

	float fRadiusSq = fRadius * fRadius;
	int nCnt = m_vecTroop.size();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTroop = m_vecTroop[i];
		if( hTroop->IsEliminated() )
			continue;

		if( !hTroop->IsEnable() )
			continue;

		D3DXVECTOR2 Vec = *pPosition - hTroop->GetPosV2();
		float fDist = D3DXVec2LengthSq( &Vec );
		if( fDist < fRadiusSq )
			Objects.push_back( hTroop );
	}
}

void CFcTroopManager::GetEnemyListInRange( D3DXVECTOR2 *pPosition, int nTeam, float fRadius, std::vector<TroopObjHandle> &Objects )
{
	Objects.clear();

	float fRadiusSq = fRadius * fRadius;
	int nCnt = m_vecTroop.size();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTroop = m_vecTroop[i];
		if( hTroop->IsEliminated() )
			continue;

		if( !hTroop->IsEnable() )
			continue;

		if( hTroop->GetTeam() == nTeam)
			continue;

		D3DXVECTOR2 Vec = *pPosition - hTroop->GetPosV2();
		float fDist = D3DXVec2LengthSq( &Vec );
		if( fDist < fRadiusSq )
		{
			Objects.push_back( hTroop );
		}
	}
}


void CFcTroopManager::GetFriendlyListInRange( D3DXVECTOR2 *pPosition, int nTeam, float fRadius, std::vector<TroopObjHandle> &Objects )
{
	Objects.clear();

	float fRadiusSq = fRadius * fRadius;
	int nCnt = m_vecTroop.size();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTroop = m_vecTroop[i];
		if( hTroop->IsEliminated() )
			continue;

		if( !hTroop->IsEnable() )
			continue;

		if( hTroop->GetTeam() != nTeam )
			continue;

		D3DXVECTOR2 Vec = *pPosition - hTroop->GetPosV2();
		float fDist = D3DXVec2LengthSq( &Vec );
		if( fDist < fRadiusSq )
		{
			Objects.push_back( hTroop );
		}
	}
}


void CFcTroopManager::GetListInRangeAndArea( D3DXVECTOR2 *pPosition , float fRadius, float fSX, float fSY, float fEX, float fEY, std::vector<TroopObjHandle> &Objects )
{
	Objects.clear();

	float fRadiusSq = fRadius * fRadius;
	int nCnt = m_vecTroop.size();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTroop = m_vecTroop[i];
		if( hTroop->IsEliminated() )
			continue;

		if( !hTroop->IsEnable() )
			continue;

		D3DXVECTOR3 Pos = hTroop->GetPos();

		D3DXVECTOR2 Vec = *pPosition - hTroop->GetPosV2();
		float fDist = D3DXVec2LengthSq( &Vec );
		if(fDist < fRadiusSq && fSX < Pos.x && fEX > Pos.x && fSY < Pos.z && fEY > Pos.z )
		{
			Objects.push_back( hTroop );
		}
	}
}

TroopObjHandle CFcTroopManager::GetNearMeleeEnemyInRange( D3DXVECTOR2 *pPosition, int nTeam, float fRadius )
{
	std::vector<TroopObjHandle> vecObjects;
	GetEnemyListInRange( pPosition, nTeam, fRadius, vecObjects );
	float fMinDist = fRadius * fRadius + 10000.f;

	TroopObjHandle hTarget;
	int nCnt = vecObjects.size();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTroop = vecObjects[i];
		if( hTroop->GetType() == TROOPTYPE_FLYING )
			continue;

		if( !hTroop->IsEnable() )
			continue;

		if( hTroop->IsEliminated() )
			continue;

		D3DXVECTOR2 Dir = hTroop->GetPosV2() - *pPosition;
		float fDist = D3DXVec2LengthSq( &Dir );
		if( fDist < fMinDist )
		{
			hTarget = hTroop;
			fMinDist = fDist;
		}
	}
	return hTarget;
}


TroopObjHandle CFcTroopManager::GetTroopByName( char* pName )
{
	int nCnt = m_vecTroop.size();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hObj = m_vecTroop[i];
		if( strcmp(hObj->GetName(), pName) == 0 )
		{
			return hObj;
		}
	}

	BsAssert( 0 && "Object handle not found." );
	DebugString( "Fail GetTroopByName %s\n", pName );

	TroopObjHandle handle;
	return handle;
}


void CFcTroopManager::UpdateObjects()
{
	CFcTroopObject::UpdateObjects();
}

void CFcTroopManager::RenderObjects()
{
	CFcTroopObject::RenderObjects();
}

int CFcTroopManager::SetEnableTroopByName( char* pStr )
{
	ENALBE_TROOP_LIST_INFO Info;

	int nCnt = m_vecTroop.size();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTroop = m_vecTroop[i];
		D3DXVECTOR3 Pos = hTroop->GetPos();
		char* pTempStr = strstr( (char*)hTroop->GetName(), pStr ); 
		if( pTempStr )
			Info.m_vecTroops.push_back( hTroop );
	}

	if( Info.m_vecTroops.size() > 0 )
	{
		Info.nID = m_nCurEnableID;
		++m_nCurEnableID;
		m_vecEnableTroops.push_back( Info );
		return Info.nID;
	}
	return -1;
}



void CFcTroopManager::SetDisableTroopInArea( float fSX, float fSY, float fEX, float fEY )
{
	int nCnt = m_vecTroop.size();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTroop = m_vecTroop[i];
		D3DXVECTOR3 Pos = hTroop->GetPos();
		if( fSX < Pos.x && fEX > Pos.x && fSY < Pos.z && fEY > Pos.z )
			hTroop->CmdEnable( false );
	}
}


int CFcTroopManager::SetEnableTroopInArea( float fSX, float fSY, float fEX, float fEY )
{
	ENALBE_TROOP_LIST_INFO Info;
	
	int nCnt = m_vecTroop.size();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTroop = m_vecTroop[i];
		D3DXVECTOR3 Pos = hTroop->GetPos();
		if( fSX < Pos.x && fEX > Pos.x && fSY < Pos.z && fEY > Pos.z )
		{
			Info.m_vecTroops.push_back( hTroop );
		}
	}

	if( Info.m_vecTroops.size() > 0 )
	{
		Info.nID = m_nCurEnableID;
		++m_nCurEnableID;
		m_vecEnableTroops.push_back( Info );
		return Info.nID;
	}
	return -1;
}

void CFcTroopManager::SetDisableEnemyTroopInArea( float fSX, float fSY, float fEX, float fEY )
{
	int nCnt = m_vecTroop.size();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTroop = m_vecTroop[i];
		D3DXVECTOR3 Pos = hTroop->GetPos();

		if( hTroop->GetTeam() == 0 )		// 아군은 Team을 0으로 생각
		{
			continue;
		}
		if( fSX < Pos.x && fEX > Pos.x && fSY < Pos.z && fEY > Pos.z )
		{
			hTroop->CmdEnable( false );
		}
	}
}

int CFcTroopManager::SetEnableEnemyTroopInArea( float fSX, float fSY, float fEX, float fEY )
{
	ENALBE_TROOP_LIST_INFO Info;
	Info.nID = m_nCurEnableID;
	++m_nCurEnableID;

	int nCnt = m_vecTroop.size();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTroop = m_vecTroop[i];
		D3DXVECTOR3 Pos = hTroop->GetPos();

		if( hTroop->GetTeam() == 0 )		// 아군은 Team을 0으로 생각
		{
			continue;
		}
		if( fSX < Pos.x && fEX > Pos.x && fSY < Pos.z && fEY > Pos.z )
		{
			Info.m_vecTroops.push_back( hTroop );
		}
	}

	if( Info.m_vecTroops.size() > 0 )
	{
		m_vecEnableTroops.push_back( Info );
		return Info.nID;
	}
	return -1;
}

void CFcTroopManager::SetDisableFriendTroopInArea( float fSX, float fSY, float fEX, float fEY )
{
	int nCnt = m_vecTroop.size();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTroop = m_vecTroop[i];
		D3DXVECTOR3 Pos = hTroop->GetPos();

		if( hTroop->GetTeam() != 0 )		// 아군은 Team을 0으로 생각
		{
			continue;
		}
		if( fSX < Pos.x && fEX > Pos.x && fSY < Pos.z && fEY > Pos.z )
		{
			hTroop->CmdEnable( false );
		}
	}
}

int CFcTroopManager::SetEnableFriendTroopInArea( float fSX, float fSY, float fEX, float fEY )
{
	ENALBE_TROOP_LIST_INFO Info;
	Info.nID = m_nCurEnableID;
	++m_nCurEnableID;

	int nCnt = m_vecTroop.size();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTroop = m_vecTroop[i];
		D3DXVECTOR3 Pos = hTroop->GetPos();

		if( hTroop->GetTeam() != 0 )		// 아군은 Team을 0으로 생각
			continue;

		if( fSX < Pos.x && fEX > Pos.x && fSY < Pos.z && fEY > Pos.z )
			Info.m_vecTroops.push_back( hTroop );
	}

	if( Info.m_vecTroops.size() > 0 )
	{
		m_vecEnableTroops.push_back( Info );
		return Info.nID;
	}
	return -1;
}


void CFcTroopManager::SetGuardianTroop( TroopObjHandle hTroop )
{
	m_pGuardianManager->AddGuardian( hTroop );
}


TroopObjHandle CFcTroopManager::GetGuardianTroop( int nIndex )
{
	return m_pGuardianManager->GetGuardianTroop( nIndex );
}

void CFcTroopManager::SetGuardianEnable( int nIndex, bool bEnable )
{
	return m_pGuardianManager->SetGuardianEnable( nIndex, bEnable );
}



void CFcTroopManager::SetFirstGuardian()
{
	for( int i=0; i<2; i++ )
		m_FirstGuardian[i] = m_pGuardianManager->GetGuardianTroop( i );
}



void CFcTroopManager::DebugPathRender()
{
	int nCnt = m_vecTroop.size();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTroop = m_vecTroop[i];
		hTroop->DebugPathRender();
	}
}

TroopObjHandle CFcTroopManager::GetGuardianTarget()
{
	return m_pGuardianManager->GetGuardianTarget();
}

TroopObjHandle CFcTroopManager::GetLockOnTarget()
{
	return m_pGuardianManager->GetLockOnTarget();
}


int g_PlayerID = 0;

bool CFcTroopManager::IsValidMelee( TroopObjHandle hMe, TroopObjHandle hTarget )
{
	TROOPTYPE MyType = hMe->GetType();
	TROOPTYPE TargetType = hTarget->GetType();
	if( MyType == TROOPTYPE_FLYING || TargetType == TROOPTYPE_FLYING ||
		MyType == TROOPTYPE_ON_PROP || TargetType == TROOPTYPE_ON_PROP )
	{
		return false;
	}

	if( hMe->IsForceAttack() )
	{
		if( hMe->GetTargetTroop() == hTarget )
			return true;
		else
			return false;
	}

/*
	if( MyType == TROOPTYPE_NOBATTLE )
	{
		if( !(hTarget->GetState() == TROOPSTATE_MOVE_ATTACK && hTarget->GetTargetTroop() == hMe ) )
			return false;
	}

	// 타입 더 생기면 여기 추가
	if( TargetType == TROOPTYPE_NOBATTLE )
	{
		if( !(hMe->GetState() == TROOPSTATE_MOVE_ATTACK && hMe->GetTargetTroop() == hTarget) )
			return false;
	}
*/
	return true;
}

bool CFcTroopManager::IsMeleeRange( TroopObjHandle hMe, TroopObjHandle hTarget )
{
	D3DXVECTOR2 Vec = hMe->GetPosV2() - hTarget->GetPosV2();

	float fDist = hMe->GetRadius( false ) + hTarget->GetRadius( false );

	if( Vec.x > fDist )
	{
		return false;
	}
	if( Vec.y > fDist )
	{
		return false;
	}

	fDist = fDist * fDist;
	if( fDist > D3DXVec2LengthSq( &Vec ) )
	{
		return true;
	}
	return false;
}

HeroObjHandle* CFcTroopManager::GetHero( float x, float y, float range)
{
	D3DXVECTOR2 Dir;
	Dir.x = x;
	Dir.y = y;

	BsAssert( m_hHero );
	D3DXVECTOR2 HeroPos = m_hHero->GetPosV2();
	Dir = Dir - HeroPos;
	float fDist = D3DXVec2LengthSq( &Dir );
	if( fDist < range*range )
		return &m_hHero;

	return NULL;
}

void CFcTroopManager::SetGuardianAI( int nAttackAI, int nDefenseAI, int nNormalAI )
{
/*
	m_nGuardianAttackAI = nAttackAI;
	m_nGuardianDefenseAI = nDefenseAI;
	m_nGuardianNormalAI = nNormalAI;

	for( int j=0; j<2; j++ )
	{
		if( !m_FirstGuardian[j] )
			continue;

		for( int i=0; i<2; i++ )
		{
			if( m_GuardianInfo[i].hGuardian == m_FirstGuardian[j] )
			{
				int nAI = CFcTroopAIObject::GetAI( m_GuardianInfo[i].hGuardian );
				m_GuardianInfo[i].nPreAIID = nAI;
				CFcTroopAIObject::SetAI( m_GuardianInfo[i].hGuardian, m_nGuardianDefenseAI  );
			}
		}
	}
*/
}

Box3 g_DebugBox;
TroopObjHandle g_DebugTroop;
BSVECTOR g_DebugC;
float g_DebugRange;

bool CFcTroopManager::CheckFakeTroop( TroopObjHandle hTroop )
{
	if( m_hPlayerTroop == hTroop )
		return false;

	if( hTroop->GetType() == TROOPTYPE_ON_PROP )
		return false;

	if( hTroop->GetType() == TROOPTYPE_WALL_DUMMY )
		return false;

	if( hTroop->IsFakeDisable() == true )
		return false;

	AABB Box;
	Box3 NewBox;

	D3DXVECTOR3 Pos = hTroop->GetPos();
//	float fDist = hTroop->GetRadius();		// 부대원들 위치로 수정해야 한다.

	float fDist = 0.f;
	int nUnitCnt = hTroop->GetUnitCount();
	for( int i=0; i<nUnitCnt; i++ )
	{
		GameObjHandle hUnit = hTroop->GetUnit( i );
		if( hUnit == NULL )
			continue;

		D3DXVECTOR2 Dir = hUnit->GetPosV2();
		Dir = Dir - D3DXVECTOR2( Pos.x, Pos.z );
		float fCurDist = D3DXVec2LengthSq( &Dir );
		if( fDist < fCurDist )
		{
			fDist = fCurDist;
		}
	}
	fDist = sqrt( fDist );

	Box.Vmin = BSVECTOR( Pos.x - fDist, Pos.y - 2500.f, Pos.z - fDist );		// 다리중앙에 있을때 사라지는 경우가 있어서 범위를 늘립니다.
	Box.Vmax = BSVECTOR( Pos.x + fDist, Pos.y + 2500.f, Pos.z + fDist );

	NewBox.C = ( Box.Vmax + Box.Vmin ) * 0.5f;

	NewBox.A[0]=BSVECTOR(1.f,0.f,0.f);
	NewBox.A[1]=BSVECTOR(0.f,1.f,0.f);
	NewBox.A[2]=BSVECTOR(0.f,0.f,1.f);

	NewBox.E[0]=fabsf((Box.Vmax.x - Box.Vmin.x)*0.5f);
	NewBox.E[1]=fabsf((Box.Vmax.y - Box.Vmin.y)*0.5f);
	NewBox.E[2]=fabsf((Box.Vmax.z - Box.Vmin.z)*0.5f);

	NewBox.compute_vertices();

#ifdef	_XBOX
	int			iCameraHandle;
	CBsCamera*	pCBsCamera;
	float		fPers;
	float		fAspect;	
	float		fNear;
	float		fFar0, fFar1, fFar2;
	XMMATRIX	mV;
	XMVECTOR	vPlane[ CCLIP_TEST_PLANE_MAX ];
	XMVECTOR	vEye;

	iCameraHandle	= g_BsKernel.GetCameraHandle( 0 );
	pCBsCamera		= g_BsKernel.GetCamera( iCameraHandle );
	fPers			= pCBsCamera->GetFOVByProcess();
	fAspect			= pCBsCamera->GetAspect();
	fNear			= pCBsCamera->GetNearZ();
	fFar0			= pCBsCamera->GetFogEnd()*0.5f;
	fFar1			= pCBsCamera->GetFogEnd();
	fFar2			= pCBsCamera->GetFarZ();

	CSmartPtr<CFcCameraObject> hCamera = CFcBaseObject::GetCameraObjectHandle( 0 );
	CCrossVector* pCross = hCamera->GetCrossVector();
	mV				= _XMMATRIX( ( FLOAT* )	pCross->ConvertViewMatrix() );

	// multi-thread preparation version
	CClipTest::ComputeClipPlane( fPers,
								 fAspect,
								 fNear,
								 fFar0,
								 fFar1,
								 fFar2,
								 mV,
								 vPlane,
								 &vEye );
	CClipTest::SetClipPlane( vPlane );

	// clip test
	// This works good bat only wroks on XBOX.
	float	fRadiusXRadius	= NewBox.E[0]*NewBox.E[0] + NewBox.E[1]*NewBox.E[1] + NewBox.E[2]*NewBox.E[2];

g_DebugBox = NewBox;
g_DebugTroop = hTroop;
g_DebugC = NewBox.C;
g_DebugRange = fRadiusXRadius;

	if( CClipTest::TestFar1( NewBox.C, fRadiusXRadius ) )
	{
		return false;
	}
#else
	if( g_BsKernel.IsVisibleTestFromCamera( g_BsKernel.GetCameraHandle(0), &NewBox ) )
	{
		return false;
	}
#endif
	return true;
}


bool CFcTroopManager::IsFinishEnableTroops( int nID )
{
	int nCnt = m_vecEnableTroops.size();
	for( int i=0; i<nCnt; i++ )
	{
		if( m_vecEnableTroops[i].nID == nID )
		{
			return false;
		}
	}
	return true;
}

void CFcTroopManager::EliminateTroop( TroopObjHandle hTroop )
{
	m_pGuardianManager->EliminateTroop( hTroop );
}

void CFcTroopManager::PostInitialize()
{
	int nSize = m_vecTroop.size();
	for( int i=0; i<nSize; i++ )
		m_vecTroop[i]->SetIndex( i );

	BsAssert( m_vecFakeState.size() == 0 );

	m_vecFakeState.resize( nSize );
	nSize = m_vecFakeState.size();
	for( int i=0; i<nSize; i++ )
	{
		std::vector<bool>::iterator it = m_vecFakeState.begin() + i;
		*it = false;
	}
	m_pGuardianManager->SetGuardianPropertyTable();
}


void CFcTroopManager::SetGuardianTroopDefenseAdd( int nRate )
{
	m_pGuardianManager->SetGuardianTroopDefenseAdd( nRate );
}

void CFcTroopManager::SetGuardianTroopAttackAdd( int nRate )
{
	m_pGuardianManager->SetGuardianTroopAttackAdd( nRate );
}

void CFcTroopManager::SetGuardianTroopMoveSpeedAdd( int nRate )
{
	m_pGuardianManager->SetGuardianTroopMoveSpeedAdd( nRate );
}

void CFcTroopManager::SetGuardianTroopMaxHPAdd( int nRate )
{
	m_pGuardianManager->SetGuardianTroopMaxHPAdd( nRate );
}

void CFcTroopManager::SetGuardianTroopTrapDamageAdd( int nRate )
{
	m_pGuardianManager->SetGuardianTroopTrapDamageAdd( nRate );
}

void CFcTroopManager::AddGuardianTroopHP( int nRate,bool bPercent )
{
	m_pGuardianManager->AddGuardianTroopHP( nRate , bPercent);
}
