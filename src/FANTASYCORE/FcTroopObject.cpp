#include "stdafx.h"
#include "FcCommon.h"
#include "FcUnitObject.h"
#include "FcTroopObject.h"
#include "FcHeroObject.h"
#include "FcUtil.h"
#include "FcGlobal.h"
#include "FcWorld.h"
#include "BsFileManager.h"
#include "FcProp.h"
#include "FcInterfaceManager.h"
#include "DebugUtil.h"
#include "FcCameraObject.h"
#include "FcTroopManager.h"
#include "FcItem.h"


#include "navigationpath.h"
#include "FcAIObject.h"
#include "FcSOXLoader.h"

#include "BsSinTable.h"
#include "PerfCheck.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

#define DELAY_TROOP_ELIMINATED_TICK			40

extern CTroopSOXLoader		g_TroopSOX;


enum UNIT_CROSS_STATE {
	UNIT_CROSS_STATE_READY,
	UNIT_CROSS_STATE_GO_FIRST_POS,
	UNIT_CROSS_STATE_GO_SECOND_POS,
	UNIT_CROSS_STATE_GO_TARGET_POS,
	UNIT_CROSS_STATE_DONE
};

enum {
	RET_MELEE_ADJUST_POS_DONE,
	RET_MELEE_ADJUST_POS_DOING,
	RET_MELEE_ADJUST_POS_FAIL
};




CSmartPtrMng<CFcTroopObject> CFcTroopObject::s_ObjectMng(DEFAULT_BASE_TROOP_OBJECT_POOL_SIZE);




// Unit AI에서 사용
int CFcTroopObject::s_nTroopStateTable[ TROOPSTATE_NUM ] =
{
	0,																	// TROOPSTATE_NON
	UNIT_MOVABLE,														// TROOPSTATE_MOVE
	UNIT_MOVABLE,														// TROOPSTATE_MOVE_MERCIFULLY
	UNIT_MOVABLE,														// TROOPSTATE_MOVE_PATH
	UNIT_MOVABLE,														// TROOPSTATE_MOVE_PATH_LOOP
	UNIT_MOVABLE,														// TROOPSTATE_MOVE_FULL_PATH
	UNIT_MOVABLE,														// TROOPSTATE_RETREAT
	UNIT_MOVABLE | UNIT_ATTACKABLE | UNIT_RANGE,						// TROOPSTATE_RANGE_ATTACK
	UNIT_MOVABLE | UNIT_ATTACKABLE,										// TROOPSTATE_MELEE_ATTACK
	0,																	// TROOPSTATE_ELIMINATED
//	UNIT_MOVABLE,														// TROOPSTATE_CROSS_BRIDGE
	UNIT_MOVABLE,														// TROOPSTATE_MOVE_ATTACK
	UNIT_MOVABLE,														// TROOPSTATE_MELEE_READY
	UNIT_ATTACKABLE | UNIT_RANGE,										// TROOPSTATE_HOLD
	UNIT_ATTACKABLE | UNIT_RANGE,										// TROOPSTATE_HOLD_MERCIFULLY
	UNIT_MOVABLE,														// TROOPSTATE_FOLLOW
};


CFcTroopObject::CFcTroopObject()
{
	m_bEnable = false;
	m_bFirstEnable = true;
	m_State.State = TROOPSTATE_NON;
	m_State.SubState = TROOPSUBSTATE_WALK;
	m_PreState.State = TROOPSTATE_NON;
	m_PreState.SubState = TROOPSUBSTATE_WALK;
	m_Type = TROOPTYPE_NON;
	m_nTroopID = -1;
	m_nTroopIndex = -1;
	m_fRadius = 0.f;
	m_fOriginalRadius = 0.f;
	m_fMeleeRadius = 0.f;
	m_Pos.x = 0.f;
	m_Pos.z = 0.f;
	m_Pos.y =  0.f;
	m_fMoveSpeed = 8.f;
	m_fMoveSpeedRate = 1.f;
	m_fWalkMoveSpeedRate = 1.f;		// m_fMoveSpeed에 대한 비율

	m_nTeam = -1;
	m_nForce = -1;

	m_bUseNavMesh = true;
	m_pNavMesh = NULL;
	m_pNavPath = NULL;
	m_pNavCell = NULL;

	m_bNavPathActive = false;

	m_prevPathFindDest.x = 0.f;
	m_prevPathFindDest.y = 0.f;

	m_bForceCmd = false;
	m_bEliminated = false;
	m_nEliminatedDelayTick = 0;

	m_hFailEngageTroop.Identity();
	m_nCheckFailEngageTroopTick = 0;

	m_pCurFullPath = NULL;
	m_nCurFullPathIndex = 0;
	m_nCrossMoveDelay = 0;

	// 다리나 성문 지나가는 중이면
	m_bCross = false;
	m_pCrossProp = NULL;
	m_nFollower = 0;
	memset(m_Follower, 0, sizeof(FOLLOWERINFO) * TROOP_FOLLOWER_MAX);
	m_Dir = D3DXVECTOR2(0.0f,1.0f);
	m_nUnitNum = 0;
	m_pUnits = NULL;
	m_nCurFormationType = 0;
	m_LastMovePos = D3DXVECTOR2( 0.f, 0.f );
	m_bVisibleInMinimap = true;
	m_nMaxHP = 0;
	m_nHP = 0;
	m_bRenderSkip = false;
	m_pInitUnitData = NULL;
	m_nItemIndex = 0xcdcdcd;
	m_nKilled = 0;
	m_nRemoved = 0;
	m_bBoost = false;
	m_bAttackable = true;
	m_bUntouchable = false;
	m_fAttackRadius = 0.f;
	m_bStopProcess = false;
	m_bFakeMode = false;
	m_nAdditionalAttackPower = 0;  // 100% 그대로~
	m_nAdditionalDefense = 0;

	m_bGuardian = false;

	m_nMaxHPAdd = 0;
	m_nDefenseAdd = 0;
	m_nAttackAdd = 0;
	m_nTrapDamageAdd = 0;
	m_nMoveSpeedAdd = 0;

	m_bCheckHPInProcess = true;

	m_nCheckUnitsNearbyPlayerTick = -1;

	m_bFakeDisable = false;

	m_nGoblinCount = 0;
	m_nOrcCount = 0;
	m_nElfCount = 0;
	m_nEtcUnitCount = 0;

	m_nLevel = 0;
	m_bForceDisable = false;
	m_nLiveUnitNumBackup = 0;
}

CFcTroopObject::~CFcTroopObject()
{
	CFcTroopObject::s_ObjectMng.DeleteHandle( m_Handle.GetHandle() );
	SAFE_DELETE( m_pNavPath );
	SAFE_DELETE(m_pInitUnitData);
	SAFE_DELETEA( m_pUnits );
}

bool CFcTroopObject::Initialize( int nX, int nY, int nRadius, 
								int nForce, int nTeam, 
								int nID, char* pName, int nType, 
								int nCreateCount )
{
	m_nTroopID = nID;
	BsAssert( strlen( pName )<32 );
	strcpy( m_cName, pName );
	m_Type = (TROOPTYPE)nType;
	m_Pos.x = (float)nX;
	m_Pos.z = (float)nY;
	m_Pos.y =  g_BsKernel.GetLandHeight( m_Pos.x, m_Pos.z );
	m_fRadius = (float)nRadius;
	m_nTeam = nTeam;
	m_nForce = nForce;
	m_InitPos = m_Pos;
	BsAssert(m_nTeam < MAX_TEAM_COUNT);	// 팀은 최대 8개까지

	m_pNavMesh = g_FcWorld.GetNavMesh();
	if ( m_pNavMesh )
	{
		if ( !m_pNavPath )
		{
			m_pNavPath = new NavigationPath;
		}
		InitNavCell();
	}

	m_hAttackedRangeTroop.Identity();
	m_nDelayRangeAttackedTick = 0;

	m_hAttackedMeleeTroop.Identity();
	m_nDelayMeleeAttackedTick = 0;

	return true;
}


bool CFcTroopObject::ReInit( int nX, int nY, int nDir, bool bNew )
{
	m_State.State = TROOPSTATE_NON;		// 강제 세팅. 부대전멸했을 경우 SetState로 하면 안됨
	m_bEliminated = false;
	m_nEliminatedDelayTick = 0;
	m_vecEngageTroops.clear();
	m_nKilled = 0;
	m_nRemoved = 0;
	m_bEnable = true;
	m_bStopProcess = false;

	if( m_pInitUnitData && bNew )
	{
		float fRot = 0.f;
		if( nDir < 0 )
			fRot = (float)m_pInitUnitData->nDirection * 3.141592f / 4.f;
		else
			fRot = (float)nDir * 3.141592f / 4.f;
		
		D3DXVECTOR2 Dir;
		Dir.x = sinf( fRot );
		Dir.y = cosf( fRot );
		SetDir( Dir );

		for( int i=0; i<m_nUnitNum; i++ )
		{
			if( m_pUnits[i] == NULL )
				continue;

			m_pUnits[i]->Delete();
			m_pUnits[i]->SetInScanner( false );
//			CFcWorld::GetInstance().RemoveUnitToScanner( m_pUnits[i] );
			m_pUnits[ i ].Identity();
		}
		g_FcWorld.GetTroopmanager()->CreateUnits( m_Handle, m_pInitUnitData );
	}

	for( int i=0; i<m_nUnitNum; i++ )
	{
		if( m_pUnits[i] == NULL )
			continue;

		m_pUnits[i]->Enable( true );
		if( !m_pUnits[i]->IsDie() )
			m_pUnits[i]->ChangeAnimation( ANI_TYPE_STAND, 0 );
	}
//	CmdPlace( (float)nX, (float)nY, nDir );
	return true;
}


void CFcTroopObject::PostInitialize()
{
	if( m_nUnitNum > 0 )
	{
		GameObjHandle hUnit;
		for( int i=0; i<m_nUnitNum; i++ )
		{
			hUnit = m_pUnits[i];
			if( hUnit == NULL )
			{
				continue;
			}
		}
		
		if( hUnit )		// 유닛 다 죽으면 hUnit은 NULL, 하지만 상관없음
			SetLevel( hUnit->GetLevel() );
	}
	SetFullHP();
	CheckUnitKind();

	GameObjHandle hUnit = GetFirstLiveUnit();
	BsAssert( hUnit );
	UnitDataInfo *pUnitInfo = CUnitSOXLoader::GetInstance().GetUnitData( hUnit->GetUnitSoxIndex() );
	m_nCrowdUnitType = pUnitInfo->nCrowdUnitType;
}

void CFcTroopObject::Process()
{
	if( CanIProcess() == false )
		return;

	switch( m_State.State )
	{
	case TROOPSTATE_NON:
		if( m_vecEngageTroops.size() > 0 )	// CmdStop 같은 것으로 EngageTroops이 있는 경우에 STATE_NON으로 들어오는 경우가 있다.
		{
			m_State.State = TROOPSTATE_MELEE_ATTACK;
			ResetWayPoint();
		}
		break;
		// Param1 : deltaX, Param2 : deltaY, Param3 : targetX, Param4 : targetY
	case TROOPSTATE_MOVE:
	case TROOPSTATE_MOVE_ATTACK:
		if( ProcessMove() == false )
			ResetState();
		break;

	case TROOPSTATE_MOVE_MERCIFULLY:
		if( ProcessMoveMercifully() == false )
			ResetState();
		break;

	case TROOPSTATE_MOVE_PATH:
	case TROOPSTATE_MOVE_PATH_LOOP:
		if( ProcessMovePath() == false )
			ResetState();
		break;

	case TROOPSTATE_MOVE_FULL_PATH:
		if( ProcessMoveFullPath() == false )
			ResetState();
		break;

	case TROOPSTATE_RETREAT:
		if( ProcessMove() == false )
			ResetState();
		break;

	case TROOPSTATE_RANGE_ATTACK:
		break;

	case TROOPSTATE_MELEE_READY:		// Unit쪽에서 필요한 state라서 추가
		int nRet;
		nRet = ProcessMeleeReady();
/*
		TroopObjHandle hTarget = m_vecEngageTroops[0];
		if( hTarget )
*/
		if( nRet == RET_MELEE_ADJUST_POS_DONE )
		{
			ResetWayPoint();
			m_State.State = TROOPSTATE_MELEE_ATTACK;
			m_State.bForceAttack = false;
		}
		else if( nRet == RET_MELEE_ADJUST_POS_FAIL )
		{
			ResetState();
		}
		else
		{
			BsAssert( nRet == RET_MELEE_ADJUST_POS_DOING );
		}
		break;

	case TROOPSTATE_MELEE_ATTACK:		// Melee attack에서 빠져 나가는 상태는 TroopManager에서 해준다.
		if(ProcessMeleeAttack() == false )
		{
			OnProcessMeleeFinish();
			ResetState();
		}
		break;

	case TROOPSTATE_HOLD:
		if( ProcessHold() == false )
			ResetState();
		break;

	case TROOPSTATE_HOLD_MERCIFULLY:
		if( ProcessHoldMercifully() == false )
			ResetState();
		break;

	case TROOPSTATE_FOLLOW:
		if( ProcessFollow() == false )
			ResetState();
		break;

	case TROOPSTATE_ELIMINATED:
		break;
	}

	if( m_nDelayRangeAttackedTick > 0 )
	{
		--m_nDelayRangeAttackedTick;
		if( m_nDelayRangeAttackedTick == 0 )
			m_hAttackedRangeTroop.Identity();
	}

	if( m_nDelayMeleeAttackedTick > 0 )
	{
		--m_nDelayMeleeAttackedTick;
		if( m_nDelayMeleeAttackedTick == 0 )
			m_hAttackedMeleeTroop.Identity();
	}

	ProcessFormation();

	// WayPoint에 다다렀으면 다음 WayPoint로 이동
	if( GetProcessTick() % 3 == 1 )
	{
		D3DXVECTOR2 NewPos = D3DXVECTOR2(m_Pos.x, m_Pos.z);
		for( int i=0; i<m_nUnitNum; i++ )
		{
			if( m_pUnits[i] == NULL )
				continue;

			if( m_pUnits[i]->IsDie() || m_pUnits[i]->IsEnable() == false )
				continue;

			if( m_pUnits[i]->GetWayPointNum() == 0 )
				continue;

			float fDist = m_pUnits[i]->GetDistSqWayPoint();
			if( fDist < 500.f * 500.f )
			{
				m_pUnits[i]->PopWayPoint();
			}
		}
	}


	if( m_bEliminated )
	{
		if( m_nEliminatedDelayTick >= DELAY_TROOP_ELIMINATED_TICK )
		{
			if( m_State.State != TROOPSTATE_ELIMINATED )
			{
				SetState( TROOPSTATE_ELIMINATED );
				if( m_nItemIndex != 0xcdcdcd )
				{
					if( m_nItemIndex < 0 )
					{
						// Item drop table index로 처리
					}
					else
					{
						D3DXVECTOR3 Pos = GetPos();
						g_FcItemManager.CreateItemToWorld( m_nItemIndex, Pos );
					}
				}
			}
		}
		else
		{
			++m_nEliminatedDelayTick;
		}
	}

/*
	// 임시 코드
	// Kill과 죽은 유닛 갯수가 안 맞는다.
	bool bAllDie = true;
	for( int i=0; i<m_nUnitNum; i++ )
	{
		if( m_pUnits[i] != NULL )
		{
			bAllDie = false;
			break;
		}
	}
	if( bAllDie == true )
	{
		SetEliminate();
		CFcWorld::GetInstance().GetTroopmanager()->EliminateTroop( m_Handle );
	}
*/


	if( m_hFollowTroop )
	{
		if( m_hFollowTroop->IsEliminated() )
			m_hFollowTroop.Identity();
	}

	int nCnt = m_vecFollowedTroop.size();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTroop = m_vecFollowedTroop[i];
		if( hTroop->IsEliminated() )
		{
			m_vecFollowedTroop.erase( m_vecFollowedTroop.begin() + i );
			--i;
			--nCnt;
		}
	}
	
	m_Pos.y = g_BsKernel.GetLandHeight( m_Pos.x, m_Pos.z );

	if( !m_bFakeMode )
	{
		for( int i=0; i<m_nUnitNum; i++ )
		{
			if( m_pUnits[i] )
				m_pUnits[i]->Process();
		}
	}

	DisplayDebugString();

	SetHP();
	return;
/*
// Siva Test
	if( g_BsKernel.GetTick() % 1000 == 0 ) {
		D3DXVECTOR2 vVec;
		vVec.x = RandomNumberInRange( -100.f, 100.f );
		vVec.y = RandomNumberInRange( -100.f, 100.f );
		D3DXVec2Normalize( &vVec, &vVec );
		vVec *= 1200.f;
		CmdMove( m_Pos.x + vVec.x, m_Pos.z + vVec.y, 1.f );
	}
*/
}

void CFcTroopObject::PostProcess()
{
	if( CanIProcess() == false )
		return;

	if( m_bFakeMode )
		return;

	for( int i = 0; i < m_nUnitNum; i++ )
	{
		if( m_pUnits[ i ] )
		{
			m_pUnits[ i ]->PostProcess();
		}
	}
}

void CFcTroopObject::AddLeader( GameObjHandle hObj )
{
//	BsAssert( m_hLeader == NULL );
	m_hLeader = hObj;
}

void CFcTroopObject::AddUnit( GameObjHandle hObj, int nIndex )
{
	BsAssert( nIndex >= 0 && m_nUnitNum > nIndex ); 
	m_pUnits[ nIndex ] = hObj;

	switch( hObj->GetClassID() ) {
		case CFcGameObject::Class_ID_Catapult:
			{
				GameObjHandle Handle;
				int nLinkUnitSOXIndex = 34;
				D3DXVECTOR2 vPosTable[4] = { D3DXVECTOR2( -180.f, 200.f ), D3DXVECTOR2( 180.f, 200.f ),
											D3DXVECTOR2( -180.f, -200.f ), D3DXVECTOR2( 180.f, -200.f ) };

				D3DXVECTOR2 vCenter = m_vecTroopsFormation[0][m_vecTroopsFormation[0].size()-1].Dir * m_vecTroopsFormation[0][m_vecTroopsFormation[0].size()-1].fDist;

				// AI 파일이름의 뒷부분을 따라가게 하기위해서..
				char szAITailName[64] = { 0, };
				if( hObj->GetAIHandle() ) {
					int nLength = strlen( CUnitSOXLoader::GetInstance().GetUnitData( hObj->GetUnitSoxIndex() )->cUnitName );
					sprintf( szAITailName, hObj->GetAIHandle()->GetFileName() + nLength );
				}
				
				for( DWORD i=0; i<4; i++ ) {
					Handle = CFcGameObject::CreateGameObject( CFcGameObject::Class_ID_Unit, hObj->GetCrossVector() );

					// 레벨 0으로 고정!!
					Handle->Initialize( g_FcWorld.GetInitGameObjectData( nLinkUnitSOXIndex, 0 ), hObj->GetForce(), hObj->GetTeam(), m_Handle );
					Handle->SetLevel( hObj->GetLevel() );

					Handle->RideOn( hObj );
					Handle->CFcBaseObject::Enable( false );
					((CFcUnitObject*)Handle.GetPointer())->SetRandomStand( false );

					m_vecExtraUnits.push_back( Handle );
					/*
					vPosTable[i] += vCenter;
					float fLength = D3DXVec2Length( &vPosTable[i] );
					D3DXVec2Normalize( &vPosTable[i], &vPosTable[i] );
					AddVecFormation( 0, vPosTable[i], fLength );
					AddVecFormation( 1, vPosTable[i], fLength );
					*/
					AddVecFormation( 0, D3DXVECTOR2( 0.f, 0.f ), 0.f );
					AddVecFormation( 1, D3DXVECTOR2( 0.f, 0.f ), 0.f );

					char szFileName[64];
					AIObjHandle AIHandle;
					AIHandle = CFcAIObject::CreateObject<CFcAIObject>();
					sprintf( szFileName, "%s%s", CUnitSOXLoader::GetInstance().GetUnitData( nLinkUnitSOXIndex )->cUnitName, szAITailName );

					if( AIHandle->Initialize( szFileName, Handle ) == false ) {
						delete AIHandle;
					}
					else {
						Handle->SetAIHandle( AIHandle );
					}
				}
			}
			break;
		case CFcGameObject::Class_ID_MoveTower:
			{
				/*
				GameObjHandle Handle;
				int nLinkUnitSOXIndex = 28;
				D3DXVECTOR2 vPos = D3DXVECTOR2( 0.f, 0.f );

				for( DWORD i=0; i<10; i++ ) {
					Handle = CFcGameObject::CreateGameObject( CFcGameObject::Class_ID_Unit, hObj->GetCrossVector() );
					Handle->Initialize( g_FcWorld.GetInitGameObjectData( nLinkUnitSOXIndex ), hObj->GetForce(), hObj->GetTeam(), m_Handle, 0.9f, 1.1f );
//					Handle->Initialize( nLinkUnitSOXIndex, hObj->GetForce(), hObj->GetTeam(), m_Handle, 0.9f, 1.1f );
					Handle->SetLevel( hObj->GetLevel() );

					Handle->RideOn( hObj );
					Handle->CFcBaseObject::Enable( false );

					m_vecObj.push_back( Handle );
					AddVecFormation( 0, vPos, 0.f );
					AddVecFormation( 1, vPos, 0.f );

					char szFileName[64];
					AIObjHandle AIHandle;
					AIHandle = CFcAIObject::CreateObject<CFcAIObject>();
					sprintf( szFileName, "%s_Default.ai", CUnitSOXLoader::GetInstance().GetUnitData( nLinkUnitSOXIndex )->cUnitName );

					if( AIHandle->Initialize( szFileName, Handle ) == false ) {
						delete AIHandle;
					}
					else {
						Handle->SetAIHandle( AIHandle );
					}

				}
				*/
			}
			break;
		default:
			break;
	}
	
}

void CFcTroopObject::CmdDie( GameObjHandle hObj )
{
	int nSoxIndex;

	bool bCheck = false;
	for( int i=0; i<m_nUnitNum; i++ ) {
		if( m_pUnits[i] == hObj ) {
			bCheck = true;
			break;
		}
	}
	if( bCheck == false ) return;

	++m_nKilled;
	if( m_nKilled >= m_nUnitNum )
	{
		SetEliminate();
		CFcWorld::GetInstance().GetTroopmanager()->EliminateTroop( m_Handle );
	}
	nSoxIndex = hObj->GetUnitSoxIndex();
	if( nSoxIndex == 28 || nSoxIndex == 29 || nSoxIndex == 30 ||
		nSoxIndex == 34 || nSoxIndex == 62 ||
		nSoxIndex == 63 || nSoxIndex == 64 ) //고블린
	{
		m_nGoblinCount--;
		if( m_nGoblinCount < 0 )
		{
			m_nGoblinCount = 0;
		}
	}
	else if( nSoxIndex > 13 || nSoxIndex < 26 ) { //오크
		m_nOrcCount--;
		if( m_nOrcCount < 0 )
		{
			m_nOrcCount = 0;
		}
	}
	else if( nSoxIndex == 47 || nSoxIndex == 31 ) { //엘프
		m_nElfCount--;
		if( m_nElfCount < 0 )
		{
			m_nElfCount = 0;
		}
	}
	else{
		m_nEtcUnitCount--;
		if( m_nEtcUnitCount < 0 )
		{
			m_nEtcUnitCount = 0;
		}
	}
}

void CFcTroopObject::RemoveUnit( GameObjHandle hObj )
{
	bool bCheck = false;
	for( int i=0; i<m_nUnitNum; i++ )
	{
		if( m_pUnits[i] == hObj )
		{
			m_pUnits[i].Identity();
			bCheck = true;
			break;
		}
	}
	if( m_hLeader == hObj )
	{
		m_hLeader.Identity();
		bCheck = true;
	}
	if( bCheck == false ) return;
	m_nRemoved++;

	if( m_bStopProcess == false )
	{
		if( m_nRemoved >= m_nUnitNum )
		{
			SetHP();
			m_bStopProcess = true;
		}
	}
}

GameObjHandle CFcTroopObject::GetFirstLiveUnit()
{
	int nLiveCnt = 0;
	for( int i=0; i<m_nUnitNum; i++ )
	{
		if( m_pUnits[i] == NULL )
			continue;

		if( !m_pUnits[i]->IsDie() && m_pUnits[i]->GetHP() > 0 )
			return m_pUnits[i];
	}
	GameObjHandle hDummy;
	return hDummy;
}


int CFcTroopObject::GetAliveUnitCount()
{
	if( m_bForceDisable == true )
		return m_nLiveUnitNumBackup;

	int nLiveCnt = 0;
	for( int i=0; i<m_nUnitNum; i++ )
	{
		if( m_pUnits[i] == NULL )
			continue;

		if( !m_pUnits[i]->IsDie() && m_pUnits[i]->GetHP() > 0 )
			++nLiveCnt;
	}
	return nLiveCnt;
}

int CFcTroopObject::GetDeadUnitCount()
{
	//int nDeadCnt = 0;
	//int nCnt = m_vecObj.size();
	//for( int i=0; i<nCnt; i++ )
	//{
	//	GameObjHandle hUnit = m_vecObj[i];
	//	if( hUnit->IsDie() )
	//	{
	//		++nDeadCnt;
	//	}
	//}
	//return nDeadCnt;

	return m_nKilled;
}


D3DXVECTOR2 CFcTroopObject::GetAverageAliveUnitPos()
{
	D3DXVECTOR2 Pos( 0.f, 0.f );
	int nLiveCnt = 0;
	for( int i=0; i<m_nUnitNum; i++ )
	{
		if( m_pUnits[i] == NULL )
			continue;

		if( !m_pUnits[i]->IsDie() && m_pUnits[i]->GetHP() > 0 )
		{
			++nLiveCnt;
			Pos += m_pUnits[i]->GetPosV2();
		}
	}
	Pos = Pos / (float)nLiveCnt;
	return Pos;
}


bool CFcTroopObject::GetUnitCenterPos( D3DXVECTOR2* pPos )
{
	int nCnt = 0;
	pPos->x = 0.f;
	pPos->y = 0.f;
	for( int i=0; i<m_nUnitNum; i++ )
	{
		if( m_pUnits[i] == NULL )
			continue;

		if( m_pUnits[i]->IsDie() )
			continue;
		
		if( m_pUnits[i]->IsEnable() == false )
			continue;

		*pPos = *pPos + m_pUnits[i]->GetPosV2();
		++nCnt;
	}
	if( nCnt == 0 )
		return false;

	*pPos = *pPos / (float)nCnt;
	return true;
}


void CFcTroopObject::AddHP( int nVal , bool bPercent)
{
	for( int i=0; i<m_nUnitNum; i++ )
	{
		if( m_pUnits[i] == NULL )
			continue;

		if( m_pUnits[i]->GetHP() <= 0 )
			continue;

		if(bPercent)
		{
			int nMaxHP = m_pUnits[i]->GetMaxHP();
			int nCurHP = m_pUnits[i]->GetHP();
			int nNewHP = nCurHP + ( nMaxHP * nVal / 100 );
			if( nNewHP < 0 )		{ nNewHP = 0; }
			if( nNewHP > nMaxHP )	{ nNewHP = nMaxHP; }
			m_pUnits[i]->SetHP( nNewHP );
		}
		else{
			m_pUnits[i]->AddHP( nVal );
		}
	}
}

void CFcTroopObject::SetMaxHPAdd( int nRate )
{
	SetMaxHPAdd( (float)nRate );
}

void CFcTroopObject::SetMaxHPAdd( float fRate )
{
	for( int i=0; i<m_nUnitNum; i++ )
	{
		if( m_pUnits[i] == NULL )
			continue;

		if( m_pUnits[i]->GetHP() <= 0 )
			continue;

		float fMaxHP = (float)m_pUnits[i]->GetMaxHP();
		float fCurHP = m_pUnits[i]->GetFloatHP();
		
		float fHPRate = 100.f;
		if( fMaxHP > 0.f )
			fHPRate = fCurHP * 100.f / fMaxHP;

		int nNewRate = (int)fRate + m_pUnits[i]->GetMaxHPAdd();
		m_pUnits[i]->SetMaxHPAdd( nNewRate );
		float fTempMaxHP = (float)m_pUnits[i]->GetMaxHP();

		if( fHPRate > 100.f ) fHPRate = 100.f;
		fCurHP = fTempMaxHP * fHPRate / 100.f;

		if( (int)fCurHP < 0 )
		{
			m_pUnits[i]->SetHP( 1 );
		}
		else
		{
			m_pUnits[i]->SetHP( (int)fCurHP );
		}
	}

	m_nMaxHPAdd += (int)fRate;
}

void CFcTroopObject::SetDefenseAdd( int nRate )
{
	for( int i=0; i<m_nUnitNum; i++ )
	{
		if( m_pUnits[i] == NULL )
			continue;
		
		int nAddRate = nRate + m_pUnits[i]->GetDefenseAdd();
		m_pUnits[i]->SetDefenseAdd( nAddRate );
	}

	m_nDefenseAdd += nRate;
}

int CFcTroopObject::GetDefense()
{
	for( int i=0; i<m_nUnitNum; i++ )
	{
		if( m_pUnits[i] )
		{
			return m_pUnits[i]->GetDefense();
		}
	}
	return -1;
}

void CFcTroopObject::SetAttackAdd( int nRate )
{
	for( int i=0; i<m_nUnitNum; i++ )
	{
		if( m_pUnits[i] == NULL )
			continue;

		int nAddRate = nRate + m_pUnits[i]->GetAttackPowerAdd();
		m_pUnits[i]->SetAttackPowerAdd( nAddRate );
	}
	m_nAttackAdd = nRate;
}

int CFcTroopObject::GetAttackAdd()
{
	return m_nAttackAdd;
}

struct NearbyUnits_Compare
{
	bool operator() ( GameObjHandle& A, GameObjHandle& B)
	{
		GameObjHandle hHero = g_FcWorld.GetHeroHandle();
		D3DXVECTOR3 Dir1 = hHero->GetPos() - A->GetPos();
		D3DXVECTOR3 Dir2 = hHero->GetPos() - B->GetPos();

		float fDist1 = D3DXVec3LengthSq( &Dir1 );
		float fDist2 = D3DXVec3LengthSq( &Dir2 );

		return fDist1 <= fDist2;
	}
};


bool CFcTroopObject::GetUnitsNearbyPlayer( int* pLastTick, std::vector<GameObjHandle> &VecUnits, bool bUpdate )
{
	*pLastTick  = -1;

	if( IsEnable() == false || IsEliminated() == true || GetState() != TROOPSTATE_MELEE_READY )
		return false;

	if( m_State.hTarget != g_FcWorld.GetHeroHandle()->GetTroop() )
		return false;

	bool bUpdateNew = false;
	if( bUpdate )
	{
		bUpdateNew = true;
	}
	else
	{
		if( m_nCheckUnitsNearbyPlayerTick == -1 )
			bUpdateNew = true;
		else
			bUpdateNew = false;
	}

	if( bUpdateNew )
	{
		m_vecUnitsNearbyPlayer.clear();
		for( int i=0; i<m_nUnitNum; i++ )
		{
			if( m_pUnits[i] == NULL )
				continue;

			if( m_pUnits[i]->IsEnable() == false )
				continue;

			if( m_pUnits[i]->IsDie() == true )
				continue;

			m_vecUnitsNearbyPlayer.push_back( m_pUnits[i] );
		}
		std::sort(m_vecUnitsNearbyPlayer.begin(),m_vecUnitsNearbyPlayer.end(), NearbyUnits_Compare() );
		*pLastTick = m_nCheckUnitsNearbyPlayerTick = GetProcessTick();
		VecUnits = m_vecUnitsNearbyPlayer;
	}
	else
	{
		*pLastTick = m_nCheckUnitsNearbyPlayerTick;
		VecUnits = m_vecUnitsNearbyPlayer;
	}

	return true;
}

float CFcTroopObject::GetMoveSpeed()
{
	return (m_fMoveSpeed * m_fMoveSpeedRate * ((float)m_nMoveSpeedAdd/100.f + 1.f));
}

int CFcTroopObject::GetBossHP()
{
	GameObjHandle BossHandle = GetLeader();
	if( CFcBaseObject::IsValid( BossHandle ) == NULL )
	{
		if( GetUnitCount() == 0)
			return 0;

		if(CFcBaseObject::IsValid( GetUnit( 0 ) ) == NULL)
			return 0;

		BossHandle = GetUnit( 0 );
	}
	if( BossHandle )
		return BossHandle->GetHP();

	return 0;
}

int CFcTroopObject::GetBossMaxHP()
{
	GameObjHandle BossHandle = GetLeader();
	if( CFcBaseObject::IsValid( BossHandle ) == NULL )
	{
		if( GetUnitCount() == 0)
			return 0;

		if(CFcBaseObject::IsValid( GetUnit( 0 ) ) == NULL)
			return 0;

		BossHandle = GetUnit( 0 );
	}
	if( BossHandle )
		return BossHandle->GetMaxHP();

	return 1;
}


bool CFcTroopObject::CmdStop()
{
	if( SetState( TROOPSTATE_NON ) == false )
		return false;

	return true;
}

bool CFcTroopObject::CmdMoveAttack( float fX, float fY )
{
	if( SetState( TROOPSTATE_MOVE_ATTACK, (DWORD)(&fX), (DWORD)(&fY) ) == false )
		return false;

	SetMoveSpeedRate( 1.f );

	m_State.fTargetX = fX;
	m_State.fTargetY = fY;
	m_State.SubState = TROOPSUBSTATE_RUN;
	PathFind( fX, fY, 0 );
	return true;
}

bool CFcTroopObject::CmdMove( float fX, float fY, bool bRun )
{
	if( SetState( TROOPSTATE_MOVE, (DWORD)(&fX), (DWORD)(&fY) ) == false )
		return false;

	// 이동 트리거 발생시 모든 유닛이 랜덤한 딜레이 후에 이동을 시작합니당
	if( m_bTriggerCmd ) {
        int nCnt = GetUnitCount();
		for( int i=0; i<nCnt; i++ )
		{
			GameObjHandle hUnit = GetUnit( i );

			if( hUnit == NULL )
				continue;

			CFcGameObject::GameObj_ClassID ClassID = hUnit->GetClassID();
			if( ClassID == CFcGameObject::Class_ID_Horse ||
				ClassID == CFcGameObject::Class_ID_Catapult ||
				ClassID == CFcGameObject::Class_ID_MoveTower ||
				ClassID == CFcGameObject::Class_ID_Fly )
				continue;

			if( hUnit->IsDie() )
				continue;

			hUnit->EnableMoveDelay(true);
		}
	}

	if( bRun )
	{
		SetMoveSpeedRate( 1.f );
	}
	else
	{
		SetMoveSpeedRate( m_fWalkMoveSpeedRate );
	}

	m_State.fTargetX = fX;
	m_State.fTargetY = fY;
	if( bRun )
		m_State.SubState = TROOPSUBSTATE_RUN;
	else
		m_State.SubState = TROOPSUBSTATE_WALK;

	PathFind( fX, fY, 1 );
	return true;
}

bool CFcTroopObject::CmdRetreat( float fX, float fY, bool bRun)
{
	if( SetState( TROOPSTATE_RETREAT, (DWORD)(&fX), (DWORD)(&fY) ) == false )
		return false;
	
	m_State.fTargetX = fX;
	m_State.fTargetY = fY;

	if( bRun )
		m_State.SubState = TROOPSUBSTATE_RUN;
	else
		m_State.SubState = TROOPSUBSTATE_WALK;

	PathFind( fX, fY, 2 );
	return true;
}

bool CFcTroopObject::CmdAttack( TroopObjHandle hTarget, bool bBoost, bool bForce )
{
	if( hTarget == NULL )
		return false;

	if( bForce == false && (m_State.State == TROOPSTATE_MELEE_READY || m_State.State == TROOPSTATE_MELEE_ATTACK) )
	{
		return false;
	}

	int nCnt = m_vecEngageTroops.size();
	for( int i=0; i<nCnt; i++ )
	{
		if( m_vecEngageTroops[i] == hTarget )
			return true;
	}

	D3DXVECTOR3 Pos = hTarget->GetPos();
	CmdMoveAttack( Pos.x, Pos.z );
	m_State.hTarget = hTarget;		// 호위병 사용
	m_State.bForceAttack = bForce;

	if( m_State.bForceAttack )
	{
		m_vecEngageTroops.clear();
	}
							
	m_bBoost = bBoost;
	if( m_bBoost )
	{
		SetMoveSpeedRate( 2.f );
		for( int i=0; i<m_nUnitNum; i++ )
		{
			if( m_pUnits[i] )
			{
				m_pUnits[i]->SetBoost( true );
				m_pUnits[i]->SetMoveSpeedRate( 2.f );
			}
		}
	}
	return true;
}

bool CFcTroopObject::CmdDefense( TroopObjHandle hTarget )
{
	if( hTarget == NULL )
		return false;

	CmdStop();
	return true;
}

bool CFcTroopObject::CmdEngageMercifully( TroopObjHandle hTarget, std::vector<TroopObjHandle>& vecVoidTroops )
{
	if( SetState( TROOPSTATE_MOVE_MERCIFULLY, (DWORD)&hTarget, (DWORD)(&vecVoidTroops) ) == false )
		return false;

	m_State.hTarget = hTarget;

	m_vecVoidTroops.clear();
	int nCnt = vecVoidTroops.size();
	for( int i=0; i<nCnt; i++ )
	{
		m_vecVoidTroops.push_back( vecVoidTroops[i] );
	}
	D3DXVECTOR2 Pos = GetMercifullyNextPos();
	PathFind( Pos.x, Pos.y, 3 );
	m_State.fTargetX = Pos.x;
	m_State.fTargetY = Pos.y;
	m_State.SubState = TROOPSUBSTATE_RUN;

	return true;
}


bool CFcTroopObject::CmdAnnihilate()
{
	for( int i=0; i<m_nUnitNum; i++ )
	{
		if( m_pUnits[i] == NULL )
			continue;

		if( m_pUnits[i]->IsDie() == false ) {
			int nHP = m_pUnits[i]->GetHP();
			m_pUnits[i]->AddHP( -nHP );
			m_pUnits[i]->ChangeAnimation( ANI_TYPE_DIE, -1 );
		}
	}
	return true;
}

bool CFcTroopObject::CmdEnable( bool bEnable, bool bNew, bool bExceptGuardian )
{
	if( m_bEnable == bEnable ) return true;
	if( m_bEnable == false && bEnable == true )
	{
		if( m_bFirstEnable == true )
		{
			bNew = true;	// 무조건 true
			m_bFirstEnable = false;
		}
		ReInit( (int)GetPos().x, (int)GetPos().z, -1, bNew );
		// 추가적인 공격력 상승 값이 세팅이 되어 있다면 그 %만큼 올려준다.
		if( m_nAdditionalAttackPower != 0) 
		{
			int nUnits = GetUnitCount();
			for(int i=0; i < nUnits; ++i) 
			{
				if( m_pUnits[i] == NULL )
					continue;
				m_pUnits[i]->SetAttackPowerAdd(m_nAdditionalAttackPower); // 내부적으로 퍼센티지 계산
			}
		}
		if( m_nAdditionalDefense != 0 )
		{
			int nUnits = GetUnitCount();
			for(int i=0; i < nUnits; ++i) 
			{
				if( m_pUnits[i] == NULL )
					continue;
				m_pUnits[i]->SetDefenseAdd(m_nAdditionalDefense); // 내부적으로 퍼센티지 계산
			}
		}
	}
	// 호위부대는 disable시킬 수 없다.
	else if( m_bEnable == true && bEnable == false )
	{
		if( bExceptGuardian && IsGuardian() )
			return true;
	}

	m_bEnable = bEnable;

	bool bPlayerTroop = false;
	if( GetType() == TROOPTYPE_PLAYER_1 || GetType() == TROOPTYPE_PLAYER_2 )
		bPlayerTroop = true;

	for( int i=0; i<m_nUnitNum; i++ )
	{
		if( m_pUnits[i] == NULL )
			continue;

		m_pUnits[i]->Enable( bEnable, true );

		if( bPlayerTroop )
		{
			if( m_pUnits[i]->GetAIHandle() && !m_pUnits[i]->GetParentLinkHandle() )
				m_pUnits[i]->GetAIHandle()->SetEnable( false );			// 주인공은 무조건 false로 셑
		}
		else
		{
			if( m_pUnits[i]->GetAIHandle() && !m_pUnits[i]->GetParentLinkHandle() )
				m_pUnits[i]->GetAIHandle()->SetEnable( bEnable );
		}
	}

	m_bTriggerCmd = false;
	m_bUseTriggerCmd = false;

	return true;
}


bool CFcTroopObject::CmdFollow( TroopObjHandle hTarget, float fDist, bool bBoost )
{
	if( m_bCross )
		return false;

	TROOPSTATE OldState = m_State.State;
	if( SetState( TROOPSTATE_FOLLOW ) )
	{
		m_State.hTarget = hTarget;
		m_hFollowTroop = hTarget;
		m_State.SubState = TROOPSUBSTATE_RUN;
		m_State.fDist = fDist;
		if( OldState != TROOPSTATE_FOLLOW )
			m_State.bFollowMove = false;

		hTarget->SetFollowedTroop( m_Handle );

		m_bBoost = bBoost;
		if( m_bBoost )
		{
//			SetMoveSpeedRate( 1.f );
			for( int i=0; i<m_nUnitNum; i++ )
			{
				if( m_pUnits[i] )
				{
					m_pUnits[i]->SetBoost( true );
//					m_pUnits[i]->SetMoveSpeedRate( 1.f );
				}
			}
		}
	}
	m_vecEngageTroops.clear();
	return true;
}


bool CFcTroopObject::CmdInvulnerable( bool bEnable )
{
	if( IsEnable() != true || IsEliminated() == true )
		return true;

	for( int i=0; i<m_nUnitNum; i++ )
	{
		if( m_pUnits[i] == NULL )
			continue;

		m_pUnits[i]->SetInvulnerable( bEnable );
	}

	// Hero 처리
	HeroObjHandle hHero = g_FcWorld.GetHeroHandle();
	if( hHero )
	{
		if( hHero->GetTroop() == m_Handle )
			hHero->SetInvulnerable( bEnable );
	}
	return true;
}


bool CFcTroopObject::CmdLeaderInvulnerable( bool bEnable )
{
	if( m_hLeader )
		m_hLeader->SetInvulnerable( bEnable );

	// Hero 처리
	HeroObjHandle hHero = g_FcWorld.GetHeroHandle();
	if( hHero )
	{
		if( hHero->GetTroop() == m_Handle )
			hHero->SetInvulnerable( bEnable );
	}

	return true;
}


bool CFcTroopObject::CmdPlace( float fX, float fY, int nDir )
{
	m_Pos.x = fX;
	m_Pos.y = g_BsKernel.GetLandHeight( fX, fY );
	m_Pos.z = fY;

	float fRot = (float)nDir * 3.141592f / 4.f;
	D3DXVECTOR2 Dir;
	Dir.x = sinf( fRot );
	Dir.y = cosf( fRot );
	SetDir( Dir );

	SetUnitOffset();

	AdjustPositionByAttr();

	for( int i=0; i<m_nUnitNum; i++ )
	{
		if( m_pUnits[i] == NULL )
			continue;

		D3DXVECTOR2 Offset = m_pUnits[i]->GetTroopOffset();
		m_pUnits[i]->CmdPlace( fX + Offset.x, fY + Offset.y );
		CCrossVector* pCross = m_pUnits[i]->GetCrossVector();

		D3DXVECTOR3 Target = pCross->m_PosVector;
		Target.x += Dir.x;
		Target.z += Dir.y;
		pCross->LookAt( &Target );
		/*
		if( hUnit->GetAIHandle() ) 
			hUnit->GetAIHandle()->SetEnable( true );
		*/
	}
	InitNavCell();
	SetState( TROOPSTATE_NON );
	ResetWayPoint();

	if( m_Handle == g_FcWorld.GetHeroHandle()->GetTroop() )
	{
		CameraObjHandle CamHandle = CFcBaseObject::GetCameraObjectHandle( 0 );
		CamHandle->MovePlayBack( 1 );
	}

	return true;
}


bool CFcTroopObject::CmdRenew( float fX, float fY, int nDir )
{
	if( fX == 0.f && fY == 0.f ) {
		fX = m_InitPos.x;
		fY = m_InitPos.z;
		nDir = -1;
	}

	ReInit( (int)fX, (int)fY, nDir, true );
	CmdPlace( fX, fY, nDir );
	return true;
}


bool CFcTroopObject::CmdRun( float fX, float fY )
{
	return CmdMove( fX, fY, true );
}


bool CFcTroopObject::CmdWalk( float fX, float fY )
{
	return CmdMove( fX, fY, false );
}

bool CFcTroopObject::CmdMovePath( PATH_PACK* pPath, int nCurIndex, bool bForward, bool bLoop )
{
	if( bLoop )
	{
		if( SetState( TROOPSTATE_MOVE_PATH_LOOP, (DWORD)pPath, (DWORD)nCurIndex, (DWORD)bForward, (DWORD)bLoop ) == false )
		{
			return false;
		}
	}
	else
	{
		if( SetState( TROOPSTATE_MOVE_PATH, (DWORD)pPath, (DWORD)nCurIndex, (DWORD)bForward, (DWORD)bLoop ) == false )
		{
			return false;
		}
	}
	SetMoveSpeedRate( 1.f );

	m_State.pPath = pPath;
	m_State.nCurPathIndex = nCurIndex;
	m_State.bForward = bForward;
	m_State.bLoop = bLoop;
	m_State.SubState = TROOPSUBSTATE_RUN;

	BsAssert( m_State.pPath );
	
	//int nPathNum = m_State.pPath->pathData.vecPosPool.size();
	//BsAssert( m_State.nCurPathIndex >= 0 && m_State.nCurPathIndex < nPathNum );
	// 아무래도 패스 인덱스를 웨이포인트 맥스치와 비교해서 뻑나는 것 같은데... 그래서 뺐음 (길호)

DebugString( "CMP1 %d, %d, %s\n", (int)m_State.pPath, m_State.nCurPathIndex, m_cName );
DebugString( "CMP2 %d\n", (int)m_State.pPath->pathData.vecPosPool.size() );

	float fX = m_State.pPath->pathData.vecPosPool[m_State.nCurPathIndex].x;
	float fY = m_State.pPath->pathData.vecPosPool[m_State.nCurPathIndex].z;

	PathFind( fX, fY, 4 );
	return true;
}

bool CFcTroopObject::CmdMoveFullPath( PATH_PACK* pPath, int nCurIndex, bool bForward )
{
	if( SetState( TROOPSTATE_MOVE_FULL_PATH, (DWORD)pPath, (DWORD)nCurIndex, (DWORD)bForward, (DWORD)false ) == false )
	{
		return false;
	}
	nCurIndex = m_nCurFullPathIndex;		// 다른 AI들어가다가 다시 FullPath들어오면 진행중이던 PathIndex부터 다시 시작

	SetMoveSpeedRate( 1.f );

	m_State.pPath = pPath;
	m_State.nCurPathIndex = nCurIndex;
	m_State.bForward = bForward;
	m_State.bLoop = false;
	m_State.SubState = TROOPSUBSTATE_RUN;

	BsAssert( m_State.pPath );
	int nPathNum = m_State.pPath->pathData.vecPosPool.size();
	BsAssert( m_State.nCurPathIndex >= 0 && m_State.nCurPathIndex < nPathNum );

	float fX = m_State.pPath->pathData.vecPosPool[m_State.nCurPathIndex].x;
	float fY = m_State.pPath->pathData.vecPosPool[m_State.nCurPathIndex].z;

	InitNavCell();
	PathFind( fX, fY, 5 );
	return true;
}


bool CFcTroopObject::CmdCrossBridge( CFcProp* pProp )
{
	
	return true;
}

bool CFcTroopObject::CmdHold( TroopObjHandle hTarget )
{
	if( !IsEnemyInAttackRange( hTarget ) )
		return false;

	if( SetState( TROOPSTATE_HOLD ) == false )
		return false;

	m_State.hTarget = hTarget;
	return true;
}

bool CFcTroopObject::CmdHoldMercifully( TroopObjHandle hTarget )
{
	if( !IsEnemyInAttackRange( hTarget ) )
		return false;

	if( SetState( TROOPSTATE_HOLD_MERCIFULLY ) == false )
		return false;

	m_State.hTarget = hTarget;
	return true;
}


void CFcTroopObject::CmdForceDisable()
{
	m_bEnable = false;
	m_nLiveUnitNumBackup = GetAliveUnitCount();
	m_bForceDisable = true;

	BsAssert( GetType() != TROOPTYPE_PLAYER_1 );

	for( int i=0; i<m_nUnitNum; i++ )
	{
		if( m_pUnits[i] == NULL )
			continue;

		if( m_pUnits[i]->GetAIHandle() && !m_pUnits[i]->GetParentLinkHandle() )
			m_pUnits[i]->GetAIHandle()->SetEnable( false );

		m_pUnits[i]->Enable( false );
		m_pUnits[i]->Delete();
		m_pUnits[i]->SetInScanner( false );
		m_pUnits[i].Identity();
	}

	
}


bool CFcTroopObject::IsEliminated()
{
	if( m_State.State == TROOPSTATE_ELIMINATED )
	{
		return true;
	}
	return false;
}


bool CFcTroopObject::CmdVictory( float fX, float fY )
{
	for( int i=0; i<m_nUnitNum; i++ )
	{
		if( m_pUnits[i] == NULL )
			continue;

		if( Random( 10 ) == 0 )
			m_pUnits[i]->CmdVictory( fX, fY );
	}
	return true;
}


bool CFcTroopObject::IsMeleeTarget( TroopObjHandle hTroop )
{
	int nCnt = m_vecEngageTroops.size();

	for( int i=0; i<nCnt; i++ )
	{
		// TODO: Range 공격하는 녀석이면 빼줘야 함
		if( m_vecEngageTroops[i] == hTroop )		
		{
			return true;
		}
	}
	return false;
}



bool CFcTroopObject::CanIMelee()
{
	if( !m_bAttackable || m_State.State == TROOPSTATE_RETREAT || m_State.State == TROOPSTATE_MOVE ||
		/* m_State.State == TROOPSTATE_MOVE_MERCIFULLY || */ m_State.State == TROOPSTATE_MOVE_PATH ||
		m_State.State == TROOPSTATE_MOVE_PATH_LOOP || m_State.State == TROOPSTATE_MOVE_FULL_PATH )
	{
		return false;
	}

	if( m_State.State == TROOPSTATE_FOLLOW )
	{
		D3DXVECTOR2 TargetPos = m_State.hTarget->GetPosV2();
		D3DXVECTOR2 Dir = GetPosV2();

		Dir = Dir - TargetPos;
		float fDist = D3DXVec2LengthSq( &Dir );
		if( fDist > 2000.f * 2000.f )		// 너무 멀리 떨어져 있으면 안 싸우게 한다.
			return false;
	}

	if( GetType() == TROOPTYPE_WALL_DUMMY )
		return false;

	if( GetType() == TROOPTYPE_SIEGE )
		if( m_pUnits[0] && !m_pUnits[0]->IsDie() ) return false;

	if( GetType() == TROOPTYPE_RANGE )
		return false;

	if( GetType() == TROOPTYPE_RANGEMELEE && IsGuardian() )
		return false;

	return true;
}




void CFcTroopObject::AddMeleeEngageTroop( TroopObjHandle hTroop )
{
	int nCnt = 0;
#ifdef _DEBUG
	nCnt = m_vecEngageTroops.size();
	for( int i=0; i<nCnt ; i++ )
	{
		if( m_vecEngageTroops[i] == hTroop )
		{
			BsAssert(0);
		}
	}
#endif // #ifedf _DEBUG

	if(m_State.State != TROOPSTATE_MELEE_READY)
	{
		if( GetType() == TROOPTYPE_SIEGE || GetType() == TROOPTYPE_NOBATTLE )	// NOBATTLE도 MELEE_ATTACK 상태로 들어간다.
		{
			SetState( TROOPSTATE_MELEE_ATTACK );
			ResetWayPoint();
		}
		else
		{
			SetState( TROOPSTATE_MELEE_READY );

			TroopObjHandle hMoveTarget;
			if( (int)m_vecEngageTroops.size() > 0 )
			{
				hMoveTarget = m_vecEngageTroops[0];
			}
			else
			{
				hMoveTarget = hTroop;
			}
			D3DXVECTOR2 TargetPos = hMoveTarget->GetPosV2();					
			PathFind( TargetPos.x, TargetPos.y, 0, true );

			m_State.hTarget = hMoveTarget;
			m_State.fTargetX = TargetPos.x;
			m_State.fTargetY = TargetPos.y;
		}
		m_State.SubState = TROOPSUBSTATE_RUN;
	}

	D3DXVECTOR2 Dist = GetPosV2() - hTroop->GetPosV2();

//	m_fMeleeRadius = D3DXVec2Length( &Dist ) + hTroop->GetRadius( false );
//	m_fMeleeRadius = GetRadius( false ) + hTroop->GetRadius( false );

/*
	float fRadius = GetRadius( false ) + hTroop->GetRadius( false );
	if( fRadius > m_fMeleeRadius )
		m_fMeleeRadius = fRadius;
*/
/*
	if( m_fMeleeRadius == 0.f )
	{
		m_fMeleeRadius = hTroop->GetRadius( false ) + m_fRadius;
	}
	else
	{
		m_fMeleeRadius += hTroop->GetRadius( false );
	}
*/
	float fMaxRadius = 0.f;
	m_vecEngageTroops.push_back( hTroop );
	nCnt = m_vecEngageTroops.size();
	for( int i=0; i<nCnt; i++ )
	{
		float fCurRadius = m_vecEngageTroops[i]->GetRadius( false );
		if( fCurRadius > fMaxRadius )
			fMaxRadius = fCurRadius;
	}
	m_fMeleeRadius = fMaxRadius + m_fRadius;

	if( m_fMeleeRadius < m_fRadius )
	{
		m_fMeleeRadius = m_fRadius;
	}
}


void CFcTroopObject::RemoveMeleeEngageTroop( TroopObjHandle hTroop )
{
#ifdef _DEBUG
	int nTroopCnt = 0;
	int nCnt = m_vecEngageTroops.size();
	for( int i=0; i<nCnt; i++ )
	{
		if( m_vecEngageTroops[i]->GetTroopID() == hTroop->GetTroopID() )
		{
			++nTroopCnt;
		}
	}
	BsAssert( nTroopCnt <= 1 );
#endif

	std::vector<TroopObjHandle>::iterator it = m_vecEngageTroops.begin();
	for( it = m_vecEngageTroops.begin(); it != m_vecEngageTroops.end(); it++ )
	{
		TroopObjHandle hVecTroop = *it;
		if( hTroop->GetTroopID() == hVecTroop->GetTroopID() )
		{
			m_vecEngageTroops.erase( it );
			break;
		}
	}

	if( (int)m_vecEngageTroops.size() == 0 )
	{
		m_fMeleeRadius = 0.f;
	}
	else
	{
		m_fMeleeRadius -= hTroop->GetRadius( false );

		if( m_fMeleeRadius < m_fRadius )
		{
			m_fMeleeRadius = m_fRadius;
		}
	}
//	BsAssert( 0 && "Troop Cannot found" );
}

void CFcTroopObject::SetDefaultState()
{
	m_State.State = TROOPSTATE_NON;		// 강제 세팅. 부대전멸했을 경우 SetState로 하면 안됨
	m_bEliminated = false;
	m_nEliminatedDelayTick = 0;
	m_vecEngageTroops.clear();
	m_nKilled = 0;
	m_nRemoved = 0;
	m_bEnable = true;
	m_bStopProcess = false;
}


void CFcTroopObject::DisplayDebugString()
{
	if( g_FCGameData.bShowTroopState )
	{
		char cStateStr[128] = "", *pStateBeginStr; //aleksger - change to make sure that sprintf receives proper arguments.
		char cSubStateStr[32] = "";
		switch( m_State.State )
		{
		case TROOPSTATE_NON:					pStateBeginStr = "Non";					break;
		case TROOPSTATE_MOVE:					pStateBeginStr = "Move";				break;

		case TROOPSTATE_MOVE_MERCIFULLY:		pStateBeginStr = "MoveMercifully";		break;
		case TROOPSTATE_MOVE_PATH:				pStateBeginStr = "MovePath";			break;
		case TROOPSTATE_MOVE_PATH_LOOP:			pStateBeginStr = "MovePathLoop";		break;
		case TROOPSTATE_MOVE_FULL_PATH:			pStateBeginStr = "MoveFullPath";		break;
		case TROOPSTATE_RETREAT:				pStateBeginStr = "Retreat";				break;
		case TROOPSTATE_RANGE_ATTACK:			pStateBeginStr = "RangeAttack";			break;
		case TROOPSTATE_MELEE_ATTACK:			pStateBeginStr = "MeleeAttack";			break;
		case TROOPSTATE_ELIMINATED:				pStateBeginStr = "Eliminated";			break;
		case TROOPSTATE_MOVE_ATTACK:			pStateBeginStr = "Move_Attack";			break;
		case TROOPSTATE_MELEE_READY:			pStateBeginStr = "MeleeReady";			break;
		case TROOPSTATE_HOLD:					pStateBeginStr = "Hold";				break;
		case TROOPSTATE_HOLD_MERCIFULLY:		pStateBeginStr = "HoldMercifully";		break;
		case TROOPSTATE_FOLLOW:					pStateBeginStr = "Follow";				break;

		default:								pStateBeginStr = "X";					break;
		}
		switch( m_State.SubState ) {
			case TROOPSUBSTATE_WALK : strcpy( cSubStateStr, "Walk" );	break;
			case TROOPSUBSTATE_RUN : strcpy( cSubStateStr, "Run" );	break;
			case TROOPSUBSTATE_BATTLERUN : strcpy( cSubStateStr, "BattleRun" );	break;
		}

/*
		if( IsEnable() == false )
		{
			pStateBeginStr = "Disable" ;
		}
*/

		if( !(IsEnable() == false || IsEliminated()) )
		{

			//aleksger - the behavior of sprintf is undefined if the same buffer is used for source and destionation.
			//			converted to using a separate pointer.
			sprintf( cStateStr, "%s, %s HP:%d, %s", pStateBeginStr, cSubStateStr, GetHP(), GetName() );

			/*
			strcat( cStateStr, "\n" );
			switch( m_State.State )
			{
			case TROOPSUBSTATE_CROSS_READY:					strcat( cStateStr, "Non" );					break;
			case TROOPSUBSTATE_CROSS_GO_START_POS:			strcat( cStateStr, "Move" );				break;
			case TROOPSUBSTATE_CROSS:						strcat( cStateStr, "MoveMercifully" );		break;
			default:										strcat( cStateStr, "X" );					break;
			}
			*/

			D3DXVECTOR3 Pos = GetPos();
			Pos.y += 100.f + (GetProcessTick()+GetTroopID()*10)%200;
			g_BsKernel.SendMessage( g_FcWorld.GetPlayerCamIndex(0), BS_REFRESH_CAMERA );
			g_InterfaceManager.DrawDebugText( 0, &Pos, cStateStr );
			/*
			if( IsEnable() ) {
			sprintf( cStateStr, "True" );
			}
			else sprintf( cStateStr, "False" );
			Pos.y -= 30.f;
			g_InterfaceManager.DrawDebugText( 0, &Pos, cStateStr );
			*/
		}
	}

	if( g_FCGameData.bShowUnitState )
	{
		for( int i=0 ; i<m_nUnitNum; i++ )
		{
			if( m_pUnits[i] == NULL )
				continue;

			// 여기서 스테이트 얻음
			char szStr[256];
			sprintf( szStr, "HP:%d,AT:%d,AI:%d,AA:%d,T:%d,I:%d", m_pUnits[i]->GetHP(), 
				m_pUnits[i]->GetCurAniType(), m_pUnits[i]->GetCurAniIndex(), m_pUnits[i]->GetCurAniAttr(), m_pUnits[i]->GetTroop()->GetTroopID(), i );

			D3DXVECTOR3 Pos = m_pUnits[i]->GetPos();
			Pos.y += 200.f;
			g_InterfaceManager.DrawDebugText( 0, &Pos, szStr );
		}

	}	
}

void CFcTroopObject::SetFollowedTroop( TroopObjHandle hTroop )
{
	if( hTroop->IsEliminated() )
		return;

	int nCnt = m_vecFollowedTroop.size();
	for( int i=0; i<nCnt; i++ )
	{
		if( m_vecFollowedTroop[i] == hTroop )
		{
			return;
		}
	}
	m_vecFollowedTroop.push_back( hTroop );
}



void CFcTroopObject::SetForce( int nForce )
{
	m_nForce = nForce;

	for( int i=0; i<m_nUnitNum; i++ )
	{
		if( m_pUnits[i] )
			m_pUnits[i]->SetForce( nForce);
	}
}

void CFcTroopObject::SetTeam( int nTeam )
{
	m_nTeam = nTeam;
	for( int i=0; i<m_nUnitNum; i++ )
	{
		if( m_pUnits[i] )
			m_pUnits[i]->SetTeam( nTeam );
	}
}


void CFcTroopObject::ChangeTeam( int nTeam )
{
	SetTeam(nTeam);
	std::vector<TroopObjHandle>::iterator iter =  m_vecEngageTroops.begin();
	for(; iter != m_vecEngageTroops.end();)
	{
		if((*iter)->GetTeam() == m_nTeam) {
			(*iter)->RemoveMeleeEngageTroop(m_Handle);
			(*iter)->ResetState();
			iter = m_vecEngageTroops.erase(iter);
		}
		else
			++iter;
	}
	ResetState();
}


int CFcTroopObject::GetMaxHP()
{
	return m_nMaxHP;
}


void CFcTroopObject::Update()
{
	if( CanIProcess() == false )
		return;

	if( m_bFakeMode )
		return;

	for( int i = 0; i < m_nUnitNum; i++ )
	{
		if( m_pUnits[ i ] )
			m_pUnits[ i ]->Render();		// 이름 Update로 바꿔야겠네..
	}

}


void CFcTroopObject::Render()
{
	if( CanIProcess() == false )
		return;

	if( m_bFakeMode )
		return;

//	CheckCulling();
	
//#endif
}


void CFcTroopObject::CheckEngageTroop()
{
	std::vector<TroopObjHandle>::iterator it = m_vecEngageTroops.begin();
	for( it = m_vecEngageTroops.begin(); it != m_vecEngageTroops.end();)
	{
		TroopObjHandle hTarget = (*it);
		if( hTarget->IsEliminated() || !hTarget->IsEnable() )
		{
#ifdef _DEBUG
			int nTroopCnt = 0;
			int nCnt = m_vecEngageTroops.size();
			for( int i=0; i<nCnt; i++ )
			{
				if( m_vecEngageTroops[i]->GetTroopID() == hTarget->GetTroopID() )
				{
					++nTroopCnt;
				}
			}
			BsAssert( nTroopCnt <= 1 );
#endif
			it = m_vecEngageTroops.erase( it );
			continue;
//			RemoveMeleeEngageTroop( hTarget );		// 루프안에서 사용금지
		}
		it++;
	}
}

//#define SCATTER_PER_TROOP 200
const int SCATTER_DIST_TABLEMAX = 6;
const float SCATTER_DIST_TABLE[] = { 0.f, 150.f, 300.f, 400.f, 500.f, 600.f };
void CFcTroopObject::ScatterTroop() 
{ 
	int nEngagedTroops = m_vecEngageTroops.size();
	//if(!nEngagedTroops)
	//	return;
	float fAddDist = (float)nEngagedTroops;
	
	if( SCATTER_DIST_TABLEMAX <= nEngagedTroops)
// [PREFIX:beginmodify] 2006/2/15 junyash PS#5219 reports Reading invalid data from 'SCATTER_DIST_TABLE':  the readable size is '24' bytes, but '28' bytes may be read.
		//fAddDist = SCATTER_DIST_TABLE[SCATTER_DIST_TABLEMAX];
		fAddDist = SCATTER_DIST_TABLE[SCATTER_DIST_TABLEMAX-1];
// [PREFIX:endmodify] junyash
	else 
		fAddDist = SCATTER_DIST_TABLE[nEngagedTroops];

	if( m_vecTroopsFormation[m_nCurFormationType].size() == 0 )
		return;
	
	int nIndex = 0;
	for( int i = 0; i <m_nUnitNum; i++ )
	{
		if( m_pUnits[i] == NULL )
			continue;

/*
		if( m_pUnits[i]->GetClassID() == CFcGameObject::Class_ID_Horse )
		{			
			if( m_pUnits[i]->GetLinkObjCount() > 0 )
			{
				m_pUnits[i]->SetTroopOffset( &(m_pUnits[i]->GetLinkObjHandle(0)->GetTroopOffset()) );
			}
			continue;
		}
*/

		BsAssert( nIndex < (int)m_vecTroopsFormation[m_nCurFormationType].size() );
		UNIT_OFFSET* pOffset = &(m_vecTroopsFormation[m_nCurFormationType][nIndex]);
		if( pOffset == NULL ) continue;

		D3DXVECTOR2 Dir;
		Dir.x = pOffset->Dir.x * m_Dir.y + pOffset->Dir.y * m_Dir.x;
		Dir.y = pOffset->Dir.x * -m_Dir.x + pOffset->Dir.y * m_Dir.y;

		D3DXVec2Normalize( &Dir, &Dir );
		Dir = Dir * ( pOffset->fDist + fAddDist );
		m_pUnits[i]->SetTroopOffset( &Dir );

		nIndex++;
	}
}

int CFcTroopObject::GetNumLiveSoldier()
{
	int nRetCnt = 0;
	for( int i=0; i<m_nUnitNum; i++ )
	{
		if( m_pUnits[i] == NULL )
			continue;

		if( CFcBaseObject::IsValid( m_pUnits[i] ) && !m_pUnits[i]->IsDie() )
			++nRetCnt;
	}
	return nRetCnt;
}

void CFcTroopObject::SetEliminate()
{
	m_bEliminated = true;
//	SetState( TROOPSTATE_ELIMINATED );
}


void CFcTroopObject::OnAttack( int nType, GameObjHandle hUnit )
{
	if( g_FcWorld.CanIPlayBGM() && m_Handle == g_FcWorld.GetHeroHandle()->GetTroop() )
	{
		g_FcWorld.PlayLastBGM();
	}
	if( nType == TROOP_ATTACK_TYPE_MELEE )
	{
		if( hUnit && m_hAttackedMeleeTroop == NULL )
			hUnit->GetTroop()->OnAttacked( TROOP_ATTACKED_TYPE_MELEE, m_Handle );
	}
	else if( nType == TROOP_ATTACK_TYPE_RANGE )
	{
		if( hUnit && m_hAttackedRangeTroop == NULL )
			hUnit->GetTroop()->OnAttacked( TROOP_ATTACKED_TYPE_RANGE, m_Handle );
	}
	else
	{
		BsAssert( 0 );
	}
}


void CFcTroopObject::OnAttacked( int nType, TroopObjHandle hTroop )
{
	if( nType == TROOP_ATTACKED_TYPE_MELEE )
	{
		m_hAttackedMeleeTroop = hTroop;
		m_nDelayMeleeAttackedTick = 300;
	}
	else if( nType == TROOP_ATTACKED_TYPE_RANGE )
	{
		m_hAttackedRangeTroop = hTroop;
		m_nDelayRangeAttackedTick = 300;
	}
}


bool CFcTroopObject::IsAttacked()
{
//	return ( IsMeleeEngaged() || m_hAttackedRangeTroop != NULL || m_hAttackedMeleeTroop != NULL );
	return ( m_hAttackedRangeTroop != NULL || m_hAttackedMeleeTroop != NULL );
}


bool CFcTroopObject::IsAttacked( TroopObjHandle hTroop )
{
	int nCnt = m_vecEngageTroops.size();
	
	for( int i=0; i<nCnt; i++ )
	{
		if( m_vecEngageTroops[i] == hTroop )
		{
			return true;
		}
	}

	if( m_hAttackedMeleeTroop == hTroop )
	{
		return true;
	}

	if( m_hAttackedRangeTroop == hTroop )
	{
		return true;
	}

	return false;
}

bool CFcTroopObject::IsAttackedMelee()
{
	//return IsMeleeEngaged() || ( m_hAttackedMeleeTroop != NULL );
	return ( m_hAttackedMeleeTroop != NULL );
}

bool CFcTroopObject::IsAttackedRange()
{
	return ( m_hAttackedRangeTroop != NULL );
}


bool CFcTroopObject::IsEngaged()
{
	return (IsMeleeEngaged() || IsRangeEngaged());
}

int CFcTroopObject::GetPlayerIndex()
{
	HeroObjHandle hHero = g_FcWorld.GetHeroHandle();
	if( hHero && hHero->GetTroop() == m_Handle )
		return 0;

	return -1;
}

bool CFcTroopObject::IsPlayerTroop()
{
	HeroObjHandle hHero = g_FcWorld.GetHeroHandle();
	if( hHero && hHero->GetTroop() == m_Handle )
		return true;

	return false;
}

bool CFcTroopObject::GetCollisionPropHeight( float fX, float fY, float* pHeight )
{
	// 사용 안함
	return false;
}


void CFcTroopObject::DebugPathRender()
{
#if defined(_XBOX) && defined(_DEBUG_NAV)
	NavWayPointID NextNavpoint = m_pNavPath->WaypointList().begin();
	if( NextNavpoint == NULL )
	{
		return;
	}

	const DWORD LINE_FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;
	struct LINE_VERTEX
	{
		D3DXVECTOR3 v;
		DWORD       color;
	};

	C3DDevice* pDevice = g_BsKernel.GetDevice();
	pDevice->SaveState();

	D3DXMATRIX mat;
	D3DXMatrixIdentity( &mat );
	pDevice->SetTransform( D3DTS_WORLD, &mat);

	pDevice->SetPixelShader(NULL);
	pDevice->SetVertexShader(NULL);
	pDevice->SetFVF(LINE_FVF);
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, false);
	pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	for( DWORD i=0; i<8; i++ ) pDevice->SetTexture( i, NULL );

	D3DXVECTOR3 pos = (*NextNavpoint).Position;
	D3DXVECTOR3 StartPos( pos.x, 100.f, pos.z ); 

	++NextNavpoint;

	LINE_VERTEX pVertices[4];
	while( NextNavpoint != m_pNavPath->WaypointList().end() )
	{
		D3DXVECTOR3 pos = (*NextNavpoint).Position;
		D3DXVECTOR3 EndPos(pos.x, 100.f, pos.z );

		pVertices[0].v = StartPos;
		pVertices[0].color = 0xff0000ff;

		pVertices[1].v = EndPos;
		pVertices[1].color = 0xff0000ff;

		pDevice->SetFVF(LINE_FVF);
		pDevice->DrawPrimitiveUP(D3DPT_LINELIST,1,pVertices,sizeof(LINE_VERTEX));

		StartPos = EndPos;
		++NextNavpoint;
	}

	pDevice->RestoreState();
#endif
}


int CFcTroopObject::GetAttackedTroops( std::vector<TroopObjHandle> &Objects )
{
	Objects.clear();

	if( IsMeleeEngaged() )
	{
		//std::copy( m_vecEngageTroops.begin(), m_vecEngageTroops.eng(), back_insert_interator<vector<TroopObjHandle>>(Objects) ); // XDK 불안해서 못 쓰겠다!
		int nCnt = m_vecEngageTroops.size();
		for( int i = 0; i < nCnt; i ++ )
		{
			Objects.push_back( m_vecEngageTroops[i] );
		}
	}

	if( m_hAttackedMeleeTroop != NULL )
		Objects.push_back( m_hAttackedMeleeTroop );

	if( m_hAttackedRangeTroop != NULL )
		Objects.push_back( m_hAttackedRangeTroop );

	return Objects.size();
}

bool CFcTroopObject::IsAttackedBy( TroopObjHandle hTroop )
{
	if( m_hAttackedMeleeTroop == hTroop )
		return true;

	if( m_hAttackedRangeTroop == hTroop )
		return true;

	if( IsMeleeEngaged() )
	{
		int nCnt = m_vecEngageTroops.size();
		for( int i = 0; i < nCnt; i ++ )
		{
			if( hTroop == GetMeleeEngageTroop( i ) )
				return true;
		}
	}

	return false;
}

bool CFcTroopObject::IsMeleeAttackedBy( TroopObjHandle hTroop )
{
	if( m_hAttackedMeleeTroop == hTroop )
		return true;

	if( IsMeleeEngaged() )
	{
		int nCnt = m_vecEngageTroops.size();
		for( int i = 0; i < nCnt; i ++ )
		{
			if( hTroop == GetMeleeEngageTroop( i ) )
				return true;
		}
	}

	return false;
}

bool CFcTroopObject::IsRangeAttackedBy( TroopObjHandle hTroop )
{
	if( m_hAttackedRangeTroop == hTroop )
		return true;

	return false;
}

TroopObjHandle CFcTroopObject::GetAttackedTroop()
{
	TroopObjHandle hTroop;
	if( IsMeleeEngaged() )
	{
		return GetMeleeEngageTroop( 0 );
	}
	if( m_hAttackedMeleeTroop )
	{
		return m_hAttackedMeleeTroop;
	}
	if( m_hAttackedRangeTroop )
	{
		return m_hAttackedRangeTroop;
	}
	return hTroop;
}

// Pos는 Normalize된 것이여야 한다.
void CFcTroopObject::AddVecFormation( int nType, D3DXVECTOR2& Dir, float fDist )
{
	UNIT_OFFSET Offset;
	Offset.Dir = Dir;
	Offset.fDist = fDist;
	m_vecTroopsFormation[nType].push_back( Offset );
}

void CFcTroopObject::GetUnitFormationCross( int nIndex , CCrossVector* pCross )
{
	UNIT_OFFSET* pData = &(m_vecTroopsFormation[0][nIndex]);

	D3DXVECTOR2 Dir;
	Dir.x = pData->Dir.x * m_Dir.y + pData->Dir.y * m_Dir.x;
	Dir.y = pData->Dir.x * -m_Dir.x + pData->Dir.y * m_Dir.y;

	D3DXVec2Normalize( &Dir, &Dir );

	D3DXVECTOR2 NewPos = Dir * pData->fDist;
	NewPos = NewPos + GetPosV2();
	float fY=g_BsKernel.GetLandHeight( NewPos.x, NewPos.y );
	pCross->Reset();

	pCross->m_PosVector = D3DXVECTOR3( NewPos.x, fY, NewPos.y );
	D3DXVECTOR3 TargetPos = pCross->m_PosVector + D3DXVECTOR3( m_Dir.x, 0.f, m_Dir.y );
	pCross->LookAt( &TargetPos );
}


void CFcTroopObject::SetUnitOffset()
{
	if( m_vecTroopsFormation[m_nCurFormationType].size() == 0 )
		return;

	int nIndex = 0;
	for( int i = 0; i <m_nUnitNum; i++ )
	{
		if( m_pUnits[i] == NULL )
			continue;

/* 말은 사용안함
		if( m_pUnits[i]->GetClassID() == CFcGameObject::Class_ID_Horse )
		{
			
			if( m_pUnits[i]->GetLinkObjCount() > 0 )
			{
				m_pUnits[i]->SetTroopOffset( &(m_pUnits[i]->GetLinkObjHandle(0)->GetTroopOffset()) );
			}
			continue;
		}
*/

		BsAssert( nIndex < (int)m_vecTroopsFormation[m_nCurFormationType].size() );
//		UNIT_OFFSET* pOffset = &(m_vecTroopsFormation[m_nCurFormationType][i]);
		UNIT_OFFSET* pOffset = &(m_vecTroopsFormation[m_nCurFormationType][nIndex]);
		if( pOffset == NULL ) continue;

		D3DXVECTOR2 Dir;
		Dir.x = pOffset->Dir.x * m_Dir.y + pOffset->Dir.y * m_Dir.x;
		Dir.y = pOffset->Dir.x * -m_Dir.x + pOffset->Dir.y * m_Dir.y;

//		D3DXVECTOR2 Dir = pOffset->Dir;
		D3DXVec2Normalize( &Dir, &Dir );
		Dir = Dir * pOffset->fDist;
		m_pUnits[i]->SetTroopOffset( &Dir );

		nIndex++;
	}
}

void CFcTroopObject::UpdateObjects()
{
	int nSize=CFcTroopObject::s_ObjectMng.Size();
	for( int i=0;i<nSize;i++ ){
		CFcTroopObject* pTroop = CFcTroopObject::s_ObjectMng[ i ];
		if( pTroop )
			pTroop->Update();
	}
}


void CFcTroopObject::RenderObjects()
{
	int nSize=CFcTroopObject::s_ObjectMng.Size();
	for( int i=0;i<nSize;i++ ){
		CFcTroopObject* pTroop = CFcTroopObject::s_ObjectMng[ i ];
		if( pTroop )
			pTroop->Render();
	}
}

void CFcTroopObject::ProcessObjects()
{
	int nSize = CFcTroopObject::s_ObjectMng.Size();
	for( int i = 0; i < nSize; i++ )
	{
		CFcTroopObject* pTroop = CFcTroopObject::s_ObjectMng[ i ];
		if( pTroop )
			pTroop->Process();
	}
}

void CFcTroopObject::PostProcessObjects()
{
	int nSize = CFcTroopObject::s_ObjectMng.Size();
	for( int i = 0; i < nSize; i++ )
	{
		CFcTroopObject* pTroop = CFcTroopObject::s_ObjectMng[ i ];
		if( pTroop )
			pTroop->PostProcess();
	}

	if( g_FCGameData.bDebugRenderTroopObject == true ) {
		for( int i = 0; i < nSize; i++ )
		{
			CFcTroopObject* pTroop = CFcTroopObject::s_ObjectMng[ i ];
			if( pTroop )
				pTroop->DebugRender();
		}
	}

}

// Update 로 옮김
void CFcTroopObject::DebugRenderObjects()
{	
	/*	
	int nSize = CFcTroopObject::s_ObjectMng.Size();
	for( int i = 0; i < nSize; i++ )
	{
		CFcTroopObject* pTroop = CFcTroopObject::s_ObjectMng[ i ];
		if( pTroop )
			pTroop->DebugRender();
	}
	*/	
}
	

void CFcTroopObject::ReleaseObjects()
{
	int nSize = CFcTroopObject::s_ObjectMng.Size();
	for( int i=0; i < nSize; i++ ) {
		CFcTroopObject* pTroop = CFcTroopObject::s_ObjectMng[ i ];
		if( pTroop )
			delete pTroop;
	}
}


bool CFcTroopObject::SetState( TROOPSTATE State, DWORD Param1, DWORD Param2, DWORD Param3, DWORD Param4  )
{
	if( State == TROOPSTATE_ELIMINATED )
	{
		m_State.State = State;
		return true;
	}

	// 지금 들어온 상태가 AI에서 세팅한 것이고 현재 Trigger에서 세팅된 상태라면 리턴 (Trigger Cmd의 우선순위가 높다)
	if( State != TROOPSTATE_ELIMINATED && m_bUseTriggerCmd && m_bTriggerCmd == false )
		return false;

	if( m_State.State == TROOPSTATE_ELIMINATED )
		return false;

	

	if( m_bForceCmd )
	{
		m_State.State = State;
		return true;
	}

	switch( m_State.State )
	{
/*
	// 싸우는 중일때는 도망말고는 커맨드를 받을 수 가 없다
	case TROOPSTATE_MELEE_ATTACK:
		if( State != TROOPSTATE_RETREAT && State != TROOPSTATE_ELIMINATED )
		{
			return false;
		}
		break;
*/
	case TROOPSTATE_RETREAT:
		// 도망갈때는 공격 안되게 처리해야 한다.
		break;

	// TROOPSTATE_CROSS_BRIDGE가 다른 state보다 우선순위가 높다 (Melee, Non 제외)
/*
	case TROOPSTATE_CROSS_BRIDGE:
		if( State != TROOPSTATE_CROSS_BRIDGE && State != TROOPSTATE_MELEE_ATTACK && State != TROOPSTATE_NON )
		{
			return false;
		}
		break;
*/
	}

	switch( State )
	{
	case TROOPSTATE_MOVE_PATH:
	case TROOPSTATE_MOVE_PATH_LOOP:
	case TROOPSTATE_MOVE_FULL_PATH:
		if( m_State.State == State ) 
		{
			PATH_PACK* pPath = (PATH_PACK*)Param1;
			int nCurIndex = (int)Param2;
			bool bForward = (Param3 != 0);
			bool bLoop = (Param4 != 0);

			if( m_State.bLoop )
			{
				if( m_State.pPath == pPath && m_State.bLoop == bLoop )
				{
					return false;
				}
			}
			else
			{
				if( m_State.pPath == pPath && m_State.bForward == bForward && m_State.bLoop == bLoop )
				{
					return false;
				}
			}
		}
		break;

	case TROOPSTATE_MOVE:
	case TROOPSTATE_RETREAT:
	case TROOPSTATE_MOVE_ATTACK:
		if( m_State.State == State )
		{
			float fX = *((float*)Param1);
			float fY = *((float*)Param2);
			if( m_State.fTargetX >= fX - 0.1 && m_State.fTargetX <= fX + 0.1 &&
				m_State.fTargetY >= fY - 0.1 && m_State.fTargetY <= fY + 0.1 )
			{
				return false;
			}
		}
		break;

	case TROOPSTATE_MOVE_MERCIFULLY:
		{
			TroopObjHandle hTarget = *((TroopObjHandle*)Param1);
			if( m_State.State == State )
			{
				
				std::vector<TroopObjHandle>* vecVoidTroops = (std::vector<TroopObjHandle>*)Param2;

				if( m_State.hTarget == hTarget )
				{
					int nCnt = m_vecVoidTroops.size();
					int nNewCnt = vecVoidTroops->size();
					if( nCnt == nNewCnt )
					{
						bool bValid = true;
						for( int i=0; i<nCnt; i++ )
						{
							if( m_vecVoidTroops[i] != (*vecVoidTroops)[i] )
							{
								bValid = false;
								break;
							}
						}
						return false;
					}
				}
			}
			else
			{
				if( m_hFailEngageTroop && ( m_hFailEngageTroop == hTarget ) )
				{
					if( m_hFailEngageTroop == hTarget )
					{
						if( GetProcessTick() - m_nCheckFailEngageTroopTick < 300 )
						{
							return false;
						}
						else
						{
							m_hFailEngageTroop.Identity();
							m_nCheckFailEngageTroopTick = 0;
						}
					}
				}
			}
		}
		break;
	}

	if( ( m_State.State == TROOPSTATE_FOLLOW && State != TROOPSTATE_FOLLOW ) ||
		( m_State.State == TROOPSTATE_MOVE_ATTACK && State != TROOPSTATE_MOVE_ATTACK ) )
	{
		if( m_bBoost )
		{
			m_bBoost = false;
			SetMoveSpeedRate( 1.f );
			for( int i=0; i<m_nUnitNum; i++ )
			{
				if( m_pUnits[i] == NULL )
					continue;

				m_pUnits[i]->SetBoost( false );
				m_pUnits[i]->SetMoveSpeedRate( 1.f );
			}
		}
	}
	else if( m_State.State == TROOPSTATE_MOVE_ATTACK && State != TROOPSTATE_MOVE_ATTACK )
	{
		if( m_bBoost )
		{
			m_bBoost = false;
			SetMoveSpeedRate( 1.f );
			for( int i=0; i<m_nUnitNum; i++ )
			{
				if( m_pUnits[i] == NULL )
					continue;

				m_pUnits[i]->SetBoost( false );
				m_pUnits[i]->SetMoveSpeedRate( 1.f );
			}
		}
	}

	// State == TROOPSTATE_NON 이면 m_bUseTriggerCmd를 세팅하면 안된다.(세팅하면 AI가 안 먹는다.)
	if( m_bTriggerCmd && State != TROOPSTATE_NON )
	{
		m_bUseTriggerCmd = true;
	}

	m_State.State = State;
	return true;
}


bool CFcTroopObject::IsStopMove( D3DXVECTOR2* pPos )
{
	// 임시.. 길찾기 들어가면 수정해야 함
	D3DXVECTOR2 Result;
	Result.x = m_Pos.x - pPos->x;
	Result.y = m_Pos.z - pPos->y;
	float fLength = D3DXVec2Length( &Result );

//	D3DXVec2Subtract( &Result, &Pos, &Dir );
//	float fLength = D3DXVec2Length( &Result );

	if( fLength <= GetMoveSpeed() )
	{
		return true;
	}
	return false;
}






bool CFcTroopObject::ProcessMove()
{
	if( ProcessMove( m_State.fTargetX, m_State.fTargetY ) == RET_TYPE_ON_GOING )
	{
		return true;
	}
	return false;
}

bool CFcTroopObject::CmdRangeAttack( TroopObjHandle hTarget, bool bBoost, bool bForce )
{
	if( hTarget == NULL )
		return false;

	if( m_State.State == TROOPSTATE_HOLD || m_State.State == TROOPSTATE_HOLD_MERCIFULLY )
	{
		m_State.hTarget = hTarget;
		return true;
	}

	if( IsMeleeEngaged() && bForce == false )
		return false;

	if( GetType() == TROOPTYPE_SIEGE )
	{
		if( m_nUnitNum > 0 && m_pUnits[0] && !m_pUnits[0]->IsDie() )
		{
			if( CFcTroopObject::IsEnemyInAttackRange( hTarget, m_fAttackRadius ) )
			{
				if( SetState( TROOPSTATE_RANGE_ATTACK ) == false )
					return false;

				m_State.hTarget = hTarget;
				m_State.bForceAttack = bForce;
				return true;
			}
			else
			{
				D3DXVECTOR3 Pos = hTarget->GetPos();
				CmdMoveAttack( Pos.x, Pos.z );
			}
		}
	}
	else
	{
		if( CFcTroopObject::IsEnemyInAttackRange( hTarget, m_fAttackRadius ) )
		{
			
			if( SetState( TROOPSTATE_RANGE_ATTACK ) == false )
				return false;

			m_State.hTarget = hTarget;
			m_State.bForceAttack = bForce;
			return true;
		}
		else
		{
			D3DXVECTOR3 Pos = hTarget->GetPos();
			CmdMoveAttack( Pos.x, Pos.z );
		}
	}

	D3DXVECTOR3 Pos = hTarget->GetPos();
	CmdMoveAttack( Pos.x, Pos.z );

	return true;
}



bool CFcTroopObject::ProcessMoveMercifully()
{
	if( m_State.hTarget->IsEliminated() )
	{
		return false;
	}

	if( IsMeleeEngaged() )
	{
		return true;
	}

	if( (GetProcessTick() % FRAME_PER_SEC) == ( abs(m_nTroopID) % 40 ) )
	{
		D3DXVECTOR2 Pos = GetMercifullyNextPos();
		PathFind( Pos.x, Pos.y, 6 );		// Target위치가 변하므로 계속 체크
		m_State.fTargetX = Pos.x;
		m_State.fTargetY = Pos.y;
	}
	return ProcessMove();
}

bool CFcTroopObject::ProcessMovePath()
{
	BsAssert( m_State.pPath );
	int nPathNum = m_State.pPath->pathData.vecPosPool.size();
	BsAssert( m_State.nCurPathIndex >= 0 && m_State.nCurPathIndex < nPathNum );

	float fX = m_State.pPath->pathData.vecPosPool[m_State.nCurPathIndex].x;
	float fY = m_State.pPath->pathData.vecPosPool[m_State.nCurPathIndex].z;

	RET_TYPE_PROCESS_MOVE Ret = ProcessMove( fX, fY );
	if( Ret == RET_TYPE_DONE )
	{
		if( m_State.bForward )
		{
			++m_State.nCurPathIndex;
			if( m_State.nCurPathIndex >= nPathNum )
			{
				if( m_State.bLoop )
				{
					--m_State.nCurPathIndex;
					m_State.bForward = !m_State.bForward;
				}
				else
				{
					return false;
				}
			}
		}
		else
		{
			--m_State.nCurPathIndex;
			if( m_State.nCurPathIndex < 0 )
			{
				if( m_State.bLoop )
				{
					++m_State.nCurPathIndex;
					m_State.bForward = !m_State.bForward;
				}
				else
				{
					return false;
				}
			}
		}

		float fX = m_State.pPath->pathData.vecPosPool[m_State.nCurPathIndex].x;
		float fY = m_State.pPath->pathData.vecPosPool[m_State.nCurPathIndex].z;
		PathFind( fX, fY, 7 );
	}
	else if( Ret == RET_TYPE_FAIL )
	{
		return false;
	}

	return true;
}

bool CFcTroopObject::ProcessMoveFullPath()
{
	bool bRet = ProcessMovePath();
	m_nCurFullPathIndex = m_State.nCurPathIndex;
	return bRet;		
}


int CFcTroopObject::ProcessMeleeReady()
{
	GameObjHandle hUnit = GetFirstLiveUnit();
	if( hUnit )
	{
		int nIndex = hUnit->GetUnitSoxIndex();
		if( nIndex == 75 || nIndex == 5 )		// 비그바그, 트롤 예외 처리
		{
			m_State.SubState = TROOPSUBSTATE_RUN;
		}
		else
		{
			m_State.SubState = TROOPSUBSTATE_BATTLERUN;
		}
	}
	else
	{
		m_State.SubState = TROOPSUBSTATE_BATTLERUN;
	}
	return AdjustPos();
}



bool CFcTroopObject::ProcessMeleeAttack()
{
	if( (int)m_vecEngageTroops.size() == 0 )
	{
		return false;
	}

/*
	TroopObjHandle hMoveTarget = m_vecEngageTroops[0];
	if( hMoveTarget && hMoveTarget->GetUnitCount() < 3 && GetType() == TROOPTYPE_NOBATTLE )
	{
		if( GetProcessTick() % 20 == 5 )
		{
			
			if( (int)m_vecEngageTroops.size() > 0 )
			{
				hMoveTarget = m_vecEngageTroops[0];
			}

			if( hMoveTarget )
			{
				D3DXVECTOR2 TargetPos = hMoveTarget->GetPosV2();					
				PathFind( TargetPos.x, TargetPos.y, 0, true );

				m_State.hTarget = hMoveTarget;
				m_State.fTargetX = TargetPos.x;
				m_State.fTargetY = TargetPos.y;
			}
		}

		if( GetProcessTick() % 2 == 1 )
			ProcessMove();
	}
*/
	return true;
}

bool CFcTroopObject::ProcessHold()
{
	return true;
}

bool CFcTroopObject::ProcessHoldMercifully()
{
	return true;
}

bool CFcTroopObject::ProcessFollow()
{
	if( m_State.bFollowMove )
	{
		D3DXVECTOR2 TargetPos = m_State.hTarget->GetPosV2();
		D3DXVECTOR2 Dir = GetPosV2() - TargetPos;
		float fDist = D3DXVec2LengthSq( &Dir );

		float fRate = 0.f;
		// 호위부대 예외 처리
		if( g_FcWorld.GetTroopmanager()->IsGuardian( m_Handle ) )
		{
			
			if( fDist > 3500.f * 3500.f )			{ fRate = 4.f; }
			else if( fDist > 2500.f * 2500.f )		{ fRate = 3.f; }
			else if( fDist > 1500.f * 1500.f )		{ fRate = 2.f; }
			else									{ fRate = 1.f; }
			SetMoveSpeedRate( fRate );
		}
		else
		{
			if( fDist > 5000.f * 5000.f )			{ fRate = 2.f; }
			else if( fDist > 3000.f * 3000.f )		{ fRate = 2.f; }
			else if( fDist > 2000.f * 2000.f )		{ fRate = 1.5f; }
			else									{ fRate = 1.f; }
			SetMoveSpeedRate( fRate );
		}

		int nAddRate = GetMoveSpeedAdd();
		fRate += fRate * (float)nAddRate / 100.f;

		for( int i=0; i<m_nUnitNum; i++ )
		{
			if( m_pUnits[i] == NULL )
				continue;

			m_pUnits[i]->SetMoveSpeedRate( fRate );
		}

		ProcessMove();
	}

	if( GetProcessTick() % 40 != 28 )
		return true;

	if( m_State.hTarget )
	{
		if( !m_State.hTarget->IsEnable() )
			return false;
		
		if( m_State.hTarget->IsEliminated() )
			return false;

		D3DXVECTOR2 Pos = GetPosV2();
		D3DXVECTOR2 TargetPos = m_State.hTarget->GetPosV2();
		D3DXVECTOR2 Dir = TargetPos - Pos;

		float fCurDist = D3DXVec2LengthSq( &Dir );
		if( m_State.fDist * m_State.fDist < fCurDist )
		{
			PathFind( TargetPos.x, TargetPos.y, 1 );
			m_State.bFollowMove = true;
			m_State.fTargetX = TargetPos.x;
			m_State.fTargetY = TargetPos.y;
		}
		if( ( m_State.fDist - 200.f ) * ( m_State.fDist - 200.f ) > fCurDist )
		{
			m_State.bFollowMove = false;
		}
	}
	else
	{
		return false;
	}
	return true;
}											                                                               


#define ADD_ACTIVE_PROP_CHECK_RADIUS		5000.f

bool CFcTroopObject::ProcessFormation()
{
	if( GetProcessTick() % 40 )
	{
		m_pCrossProp = NULL;

		std::vector<CFcProp*> vecNearActiveProps;
		D3DXVECTOR3 Pos = GetPos();
		g_FcWorld.GetActiveProps( Pos, ADD_ACTIVE_PROP_CHECK_RADIUS, vecNearActiveProps );

		int nCnt = vecNearActiveProps.size();
		for( int i=0; i<nCnt; i++ )
		{
			CFcProp* pProp = vecNearActiveProps[i];
			if( pProp->IsBridgeProp() || pProp->IsGateProp() )
			{
				if( DoICrossBridge( pProp ) == true )
				{
					float fNewDist = -1.f;
					if( m_pCrossProp )
					{
						D3DXVECTOR2 Dir = m_pCrossProp->GetPosV2() - GetPosV2();
						float fDist = D3DXVec2LengthSq( &Dir );

						Dir = pProp->GetPosV2() - GetPosV2();
						float fNewDist = D3DXVec2LengthSq( &Dir );

						if( fDist > fNewDist )
						{
							m_pCrossProp = pProp;
						}
					}
					else
					{
						m_pCrossProp = pProp;
					}
					break;
				}
			}
		}
	}
/*
	if( m_pCrossProp )
	{
		bool bPreCross = m_bCross;
		m_bCross = true;
		m_nCurFormationType = 1;

		// m_bCross가 세팅된다음 PathFind를 불러야 한다.
		if( bPreCross == false && m_bNavPathActive )
		{
			if( m_pCrossProp->IsBridgeProp() )
				PathFind( m_LastMovePos.x, m_LastMovePos.y, 8, true );
		}
	}
	else
	{
		bool bPreCross = m_bCross;
		m_bCross = false;

		// m_bCross가 세팅된다음 PathFind를 불러야 한다.
		if( bPreCross == true && m_bNavPathActive )
		{
			if( m_pCrossProp->IsBridgeProp() )
				PathFind( m_LastMovePos.x, m_LastMovePos.y, 9 );
		}

		m_pCrossProp = NULL;
		m_nCurFormationType = 0;
	}

	if( m_bCross )
	{
		D3DXVECTOR2 Dir = m_pCrossProp->GetPosV2() - GetPosV2();
		D3DXVec2Normalize( &Dir, &Dir );
		m_Dir = Dir;
	}
*/
	if( (::GetProcessTick() % 20) == 3 ) {
		//SetUnitOffset();			
		ScatterTroop(); // SetUnitOffset 의 기능을 포함하면서(포메이션) Engage 된 적 부대 수만큼 유닛들의 간격을 늘려준다.
		AdjustPositionByAttr();
	}

	if( m_bCross )
		AdjustPositionByAttr();


	return true;
}


RET_TYPE_PROCESS_MOVE CFcTroopObject::ProcessMove( float fX, float fY )
{
	D3DXVECTOR2 NavPos;

	bool bDoneMove = true;

	NavPos.x = fX;
	NavPos.y = fY;

	if( m_bUseNavMesh && m_pNavMesh )
	{
		if( m_bNavPathActive )
		{
			bDoneMove = m_NextNavpoint == m_pNavPath->WaypointList().end();
			// Move along the waypoint path
			if( m_NextNavpoint != m_pNavPath->WaypointList().end() )
			{
				D3DXVECTOR3 pos = (*m_NextNavpoint).Position;
				NavPos.x = pos.x;
				NavPos.y = pos.z;
				// we have reached the end of the path
			} else
			{
				m_bNavPathActive = false;
			}
		}
		// no nav mesh
	}

	D3DXVECTOR2 GoalDir;
	D3DXVec2Subtract( &GoalDir, &NavPos, &D3DXVECTOR2(m_Pos.x, m_Pos.z) );
	float fDist = D3DXVec2Length( &GoalDir );

	if( fDist < GetMoveSpeed() )
	{
		if (m_bNavPathActive) {
			m_PreNavpoint = m_NextNavpoint;

			// 다리 건널 때는 바로 앞에 웨이 포인트로 이동 ( 가운데로 가게하기 위해 )
  			if( m_bCross )
			{
				++m_NextNavpoint;
			}
			else
			{
				m_NextNavpoint = m_pNavPath->GetFurthestVisibleWayPoint(m_NextNavpoint);
			}


//			m_NextNavpoint = m_pNavPath->GetFurthestVisibleWayPoint(m_NextNavpoint);


			if (m_NextNavpoint == m_pNavPath->WaypointList().end())
				m_bNavPathActive = false;
		}
		if( bDoneMove )
		{
			return RET_TYPE_DONE;
		}
		else
		{
			return RET_TYPE_ON_GOING;
		}
	}

	D3DXVECTOR2 Pos( GoalDir );
	D3DXVec2Normalize( &Pos, &Pos);

	CaculateDir( &Pos );

//	m_Dir = Pos;

	D3DXVec2Scale( &Pos, &Pos, GetMoveSpeed() );
	D3DXVec2Add( &Pos, &Pos, &D3DXVECTOR2( m_Pos.x, m_Pos.z ) );

	if( m_bUseNavMesh && m_pNavMesh )
	{
		// compute the next desired location
		D3DXVECTOR3 NextPos( Pos.x, 0.f, Pos.y );
		NavigationCell* pNextCell;

		// test location on the NavigationMesh and resolve collisions
		D3DXVECTOR3 CurPos( m_Pos.x, 0.f, m_Pos.z );
		bool bHitWall = m_pNavMesh->ResolveMotionOnMesh(CurPos, m_pNavCell, NextPos, &pNextCell);

		if( bHitWall && !m_bNavPathActive )
		{
			return RET_TYPE_FAIL;
		}

		BYTE cAttr = g_FcWorld.GetAttrEx( NextPos.x, NextPos.z );
		if( ( cAttr & 0x08 ) && GetTeam() == 0 )		 // 이벤트 못가는 속성 8임 4아님
		{
			return RET_TYPE_DONE;
		}
		else
		{
			Pos.x = NextPos.x;
			Pos.y = NextPos.z;
		}

		m_pNavCell = pNextCell;
	}
	m_Pos.x = Pos.x;
	m_Pos.z = Pos.y;

	BsAssert( m_Pos.x >= 0.f );
	BsAssert( m_Pos.z >= 0.f );

	if( GetProcessTick() % 40 == 30 )
	{
		D3DXVECTOR2 NewPos = D3DXVECTOR2(m_Pos.x, m_Pos.z);
		for( int i=0; i<m_nUnitNum; i++ )
		{
			if( m_pUnits[i] == NULL )
				continue;

			if( m_pUnits[i]->IsDie() || m_pUnits[i]->IsEnable() == false )
				continue;

			D3DXVECTOR2 NewUnitPos = NewPos + m_pUnits[i]->GetTroopOffset();
			m_pUnits[i]->PushWayPoint( &NewUnitPos );
		}
	}
	return RET_TYPE_ON_GOING;
}





int CFcTroopObject::AdjustPos()
{
/*
	int nCnt = m_vecEngageTroops.size();
	if( nCnt == 0 )
	{
		return RET_MELEE_ADJUST_POS_FAIL;
	}
	TroopObjHandle hTarget = m_vecEngageTroops[0];
	D3DXVECTOR2 TargetPos = hTarget->GetPosV2();

	D3DXVECTOR2 Dir;
	D3DXVec2Subtract( &Dir, &TargetPos, &GetPosV2() );
	
	float fDistSq = D3DXVec2LengthSq( &Dir);
	D3DXVec2Normalize( &Dir, &Dir );
	D3DXVec2Scale( &Dir, &Dir, m_fMoveSpeed * m_fMoveSpeedRate * 2.f );
	if( fDistSq > 200*200 )
	{
		m_Pos.x += Dir.x;
		m_Pos.z += Dir.y;

		BsAssert( m_Pos.x >= 0.f );
		BsAssert( m_Pos.z >= 0.f );

		return RET_MELEE_ADJUST_POS_DOING;
	}
*/	

	bool bValid = false;
	int nCnt = m_vecEngageTroops.size();
	for( int i=0; i<nCnt; i++ )
	{
		if( m_State.hTarget == m_vecEngageTroops[i] )
		{
			bValid = true;
			break;
		}
	}

	D3DXVECTOR2 TargetPos;
	if( bValid == false )
	{
		TargetPos = D3DXVECTOR2( m_State.fTargetX, m_State.fTargetY );
	}
	else
	{
		TargetPos = m_State.hTarget->GetPosV2();
		m_State.fTargetX = TargetPos.x;
		m_State.fTargetY = TargetPos.y;
	}

	if( GetProcessTick() % 10 == 0 )
		PathFind( TargetPos.x, TargetPos.y, 0, true );

	if( ProcessMove() == false )
	{
		return RET_MELEE_ADJUST_POS_DONE;
	}

	D3DXVECTOR2 Dir = TargetPos - GetPosV2();
	float fDistSq = D3DXVec2LengthSq( &Dir);
	if( fDistSq > 200*200 )
		return RET_MELEE_ADJUST_POS_DOING;

	return RET_MELEE_ADJUST_POS_DONE;
}

void CFcTroopObject::InitNavCell()
{
	if( m_pNavMesh )
		m_pNavCell = m_pNavMesh->FindClosestCell( D3DXVECTOR3( m_Pos.x, 0.f, m_Pos.z ) );
}

void CFcTroopObject::PathFind( float& dstX, float& dstY, int nDebugNo, bool bForce )
{
//	GetBoundCoord( dstX, dstY );
	BsAssert( m_pNavMesh );

	const float MIN_PATH_FIND_DIST_SQ = 500.f * 500.f;

	if( bForce == false )
	{
		// 똑같은 곳에 또 가라고 하면 스킵
		D3DXVECTOR2 Dir;
		D3DXVec2Subtract( &Dir, &m_prevPathFindDest, &D3DXVECTOR2( dstX, dstY ) );
		if ( D3DXVec2LengthSq( &Dir ) < MIN_PATH_FIND_DIST_SQ )
		{
			return;
		}
	}

	D3DXVECTOR3 src( m_Pos.x, 0.f, m_Pos.z );
	src.y = g_BsKernel.GetLandHeight( m_Pos.x, m_Pos.z );
	D3DXVECTOR3 dst( dstX, 0.f, dstY );

//DebugString( "PF1: %f, %f\n", dstX, dstY );

	NavigationCell* dstCell; 
	dst = m_pNavMesh->SnapPointToMesh( &dstCell, dst );
//	dstX = dst.x;
//	dstY = dst.z;

	m_bNavPathActive = false;
	if ( dstCell && m_pNavMesh->BuildNavigationPath( *m_pNavPath, m_pNavCell, src, dstCell, dst ) ) {
		if ( m_pNavPath->WaypointList().size() > 2 ) {
			m_bNavPathActive = true;
			m_NextNavpoint = m_pNavPath->WaypointList().begin();

			m_PreNavpoint = m_NextNavpoint;

			if( m_bCross )
			{
				++m_NextNavpoint;
			}
			else
			{
				m_NextNavpoint = m_pNavPath->GetFurthestVisibleWayPoint(m_NextNavpoint);
			}

			// 유닛용 Move List 생성
			/*
			m_VecNavMoveQueue.clear();
			NavWayPointID NextWayPoint = m_pNavPath->WaypointList().begin();
			while( NextWayPoint != m_pNavPath->WaypointList().end() ) {
				if( m_bCross ) ++NextWayPoint;
				else NextWayPoint = m_pNavPath->GetFurthestVisibleWayPoint( NextWayPoint );

				m_VecNavMoveQueue.push_back( *( (D3DXVECTOR3*)&(*NextWayPoint).Position ) );
			}
			*/
		}
		dstX = dst.x;
		dstY = dst.z;

		m_prevPathFindDest.x = dstX;
		m_prevPathFindDest.y = dstY;
	}

//DebugString( "PF2: %f, %f, %d\n", dstX, dstY, (int)m_bNavPathActive );

	if( m_bNavPathActive )
	{
		m_LastMovePos = D3DXVECTOR2( dstX, dstY );
	}
}


bool CFcTroopObject::DoICrossBridge( CFcProp* pProp )
{
	if( pProp->IsGateProp() )
	{
		if( !((CFcGateProp*)pProp)->IsOpened() )
		{
			return false;
		}
		D3DXVECTOR2 Dir = GetPosV2() - pProp->GetPosV2();
		float fDistSq = D3DXVec2LengthSq( &Dir );
		if( fDistSq > 2000.f * 2000.f )
		{
			return false;
		}
	}
	else if( pProp->IsBridgeProp() )
	{
		D3DXVECTOR2 Dir = GetPosV2() - pProp->GetPosV2();
		float fDistSq = D3DXVec2LengthSq( &Dir );
		if( fDistSq > 5000.f * 5000.f )
		{
			return false;
		}
	}
	return true;

/*
	if( m_State.State == TROOPSTATE_MOVE || m_State.State == TROOPSTATE_MOVE_MERCIFULLY ||
		m_State.State == TROOPSTATE_MOVE_PATH || m_State.State == TROOPSTATE_MOVE_PATH_LOOP ||
		m_State.State == TROOPSTATE_MOVE_FULL_PATH || m_State.State == TROOPSTATE_MOVE_ATTACK )
	{
		if( IsNavInTargetPos( pProp ) == true )
		{
			return true;
		}
	}
	return false;
*/
}


void CFcTroopObject::GetCrossStartPos( float fRadius, CFcProp* pProp, D3DXVECTOR2* pPos, D3DXVECTOR2* pTargetPos, float fAddDist, float* pNewRadius )
{
	const D3DXMATRIX Mat = pProp->GetMat();
	D3DXVECTOR2 PropPos = pProp->GetPosV2();
	D3DXVECTOR3 BasicDir( 0.f, 0.f, -1.f );
	D3DXVec3TransformCoord( &BasicDir, &BasicDir, &Mat );
	D3DXVECTOR2 Dir;
	Dir.x = BasicDir.x - Mat._41;
	Dir.y = BasicDir.z - Mat._43;

	D3DXVECTOR2 TroopDir = GetPosV2() - PropPos;
	D3DXVec2Normalize( &TroopDir, &TroopDir );

	float fDist = D3DXVec2Dot( &TroopDir, &Dir );
	if( fDist < 0.f )	// 문 뒤로 들어가는 경우
	{
		Dir.x = -Dir.x;
		Dir.y = -Dir.y;
	}

	AABB* pBox = pProp->GetBoundingBox();

	float fLength = pBox->Vmax.z - pBox->Vmin.z;
	fLength = fLength / 2.f + fAddDist;

	D3DXVECTOR2 NewPos;
	D3DXVec2Scale( &NewPos, &Dir, fLength + fRadius );
	pPos->x = NewPos.x + PropPos.x;
	pPos->y = NewPos.y + PropPos.y;

	D3DXVec2Scale( &NewPos, &Dir, -fLength - fRadius );

	pTargetPos->x = NewPos.x + PropPos.x;
	pTargetPos->y = NewPos.y + PropPos.y;

	if( pNewRadius )
		*pNewRadius = m_fRadius;		// 일단 그냥
}

bool CFcTroopObject::IsNavInTargetPos( CFcProp* pProp )
{
	D3DXVECTOR2 Pos, TargetPos;
	GetCrossStartPos( 100.f, pProp, &Pos, &TargetPos, 0.f );

	if( m_pNavMesh && m_bNavPathActive )
	{
//		WayPointID TestWaypoint = m_NextNavpoint;
		WayPointID TestWaypoint = m_PreNavpoint;

		while( TestWaypoint != m_pNavPath->WaypointList().end() )
		{
			D3DXVECTOR2 TestPos( (*TestWaypoint).Position.x, (*TestWaypoint).Position.z );

			if( TestPos.x < TargetPos.x + 500.f  && TestPos.x > TargetPos.x - 500.f &&
				TestPos.y < TargetPos.y + 500.f  && TestPos.y > TargetPos.y - 500.f )
			{
				return true;
			}
			++TestWaypoint;
		}
	}
	return false;
}

/*
//#define ACTIVE_PROP_CHECK_RADIUS		8000.f		// 동상 발판땜시 더 늘린다.


void CFcTroopObject::SearchNearActiveProps()
{
	return;	// 너무 느려서 안쓴다.

	if( GetProcessTick() % FRAME_PER_SEC == 40 )
	{
		m_vecNearActiveProps.clear();
		g_FcWorld.GetActiveProps( m_Pos, GetRadius(true)+ADD_ACTIVE_PROP_CHECK_RADIUS, m_vecNearActiveProps );

		// Prepare "GetCollisionPropHeight()"
		{
			m_vecNearCollisionProps.clear();

			int nCnt = m_vecNearActiveProps.size();
			for( int i = 0; i < nCnt; i ++ )
			{
				CFcProp* pProp = m_vecNearActiveProps[i];
				if( !pProp->IsCollisionProp() && !pProp->IsClimbProp() )
					continue;

				m_vecNearCollisionProps.push_back( pProp );
			}
		}
	}
}
*/

/*
void CFcTroopObject::UpdatePreState()
{
	m_bForceCmd = true;

 	switch( m_PreState.State )
	{
	case TROOPSTATE_MOVE:
		CmdMove( m_PreState.fTargetX, m_PreState.fTargetY );
		break;

	case TROOPSTATE_MOVE_ATTACK:
		CmdMoveAttack( m_PreState.fTargetX, m_PreState.fTargetY );
		break;

	case TROOPSTATE_MOVE_MERCIFULLY:
		CmdEngageMercifully( m_PreState.hTarget, m_vecVoidTroops );
		break;

	case TROOPSTATE_MOVE_PATH:
	case TROOPSTATE_MOVE_PATH_LOOP:
		CmdMovePath( m_PreState.pPath, m_PreState.nCurPathIndex, m_PreState.bForward, m_PreState.bLoop );
		break;

	case TROOPSTATE_MOVE_FULL_PATH:	// 예외 처리 
		CmdMovePath( m_PreState.pPath, m_PreState.nCurPathIndex, m_PreState.bForward, m_PreState.bLoop );
		break;

	default:
		m_State = m_PreState;
		break;
	}

	m_bForceCmd = false;
}
*/

#define CROSS_START_ADD_POS		500.f

/*


void CFcTroopObject::CrossStateReady()
{
	D3DXVECTOR2 Pos;
	D3DXVECTOR2 TargetPos;
	m_fOriginalRadius = m_fRadius;
	GetCrossStartPos( m_fRadius, m_State.pProp, &Pos, &TargetPos, CROSS_START_ADD_POS, &m_fRadius );

	m_State.TargetPos = D3DXVECTOR3( TargetPos.x, 0.f, TargetPos.y );

	int nCnt = m_vecObj.size();
	for( int i=0; i<nCnt; i++ )
	{
		UnitObjHandle hUnit = m_vecObj[i];
		D3DXVECTOR3 UnitPos = hUnit->GetTroopOffset();
		D3DXVec3Scale( &UnitPos, &UnitPos, m_fRadius/m_fOriginalRadius );
		hUnit->SetTroopOffset( &UnitPos );
		hUnit->SetCrossState( UNIT_CROSS_STATE_READY );
	}
	m_Pos = D3DXVECTOR3( Pos.x, 0.f, Pos.y );

	BsAssert( m_Pos.x >= 0.f );
	BsAssert( m_Pos.y >= 0.f );

	m_State.Substate = TROOPSUBSTATE_CROSS;
}
*/

#define CROSS_FIRST_SECOND_ADD_DIST		0.f
#define CROSS_MOVE_DELAY	40



D3DXVECTOR2 CFcTroopObject::GetMercifullyNextPos()
{
	D3DXVECTOR2 vecTargetDir = m_State.hTarget->GetPosV2() - GetPosV2();
//DebugString( "NPP1:%f, %f\n", vecTargetDir.x, vecTargetDir.y );
	float fTargetDist = D3DXVec2Length( &vecTargetDir );

	if( fTargetDist == 0.f )
	{
		return D3DXVECTOR2( m_Pos.x, m_Pos.z );
	}

	D3DXVECTOR2 Dir( 0.f, 0.f );

	int nCnt = m_vecVoidTroops.size();
	for( int i = 0; i < nCnt; i ++ )
	{
		// 타겟부대와 회피부대가 너무 가까우면 척력무시
		D3DXVECTOR2 vecVoid2TargetDir = m_State.hTarget->GetPosV2() - m_vecVoidTroops[i]->GetPosV2();

		float fVoid2TargetDist = D3DXVec2LengthSq( &vecVoid2TargetDir );

		if( fVoid2TargetDist < 1000.f * 1000.f)
			continue;

		D3DXVECTOR2 vecVoidDir = m_vecVoidTroops[i]->GetPosV2() - GetPosV2();
		float fVoidDist = D3DXVec2Length( &vecVoidDir );
		if( fVoidDist > 3000.f )
			continue;

		float fProj = D3DXVec2Dot( &vecTargetDir, &vecVoidDir ) / fTargetDist;

		D3DXVECTOR2 vecDummy;
		D3DXVec2Normalize( &vecDummy, &vecTargetDir );
		vecDummy = vecDummy * fProj;

		D3DXVECTOR2 vecRepulsion = vecDummy - vecVoidDir;
		float fRepulsion = D3DXVec2Length( &vecRepulsion );

		D3DXVec2Normalize( &vecRepulsion, &vecRepulsion );
		if( fRepulsion > 3000.f )
			fRepulsion = 3000.f;
		fRepulsion = 3000.0f - fRepulsion;
		vecRepulsion = vecRepulsion * ( fRepulsion * 0.5f );

		float fRatio = ( 8000.f - fVoidDist ) / 8000.f;
		if( fRatio < 0 )
			fRatio = 0.0f;

		vecRepulsion = vecRepulsion * fRatio;

		Dir = Dir + vecRepulsion;
	}
	Dir = Dir + vecTargetDir;

	D3DXVec2Normalize( &Dir, &Dir );
	Dir = Dir * GetMoveSpeed() * 80.f;
	D3DXVECTOR2 Pos = Dir + D3DXVECTOR2( m_Pos.x, m_Pos.z );

	return Pos;
}


/*
D3DXVECTOR2 CFcTroopObject::GetMercifullyNextPos()
{
	D3DXVECTOR2 GoalDir = m_State.hTarget->GetPosV2() - GetPosV2();

	D3DXVECTOR2 TargetDir( GoalDir );
	D3DXVec2Normalize( &TargetDir, &TargetDir);

	D3DXVECTOR2 Dir( 0.f, 0.f );

	int nCnt = m_vecVoidTroops.size();
	for( int i=0; i<nCnt; i++ )
	{
		D3DXVECTOR2 TempDir = GetPosV2() - m_vecVoidTroops[i]->GetPosV2();
		D3DXVec2Normalize( &TempDir, &TempDir );

		// 타겟과 voidTroop 거리 체크해서 너무 가까우면 무시
		D3DXVECTOR2 VoidToTargetDir = m_State.hTarget->GetPosV2() - m_vecVoidTroops[i]->GetPosV2();
		float fDist = D3DXVec2LengthSq( &VoidToTargetDir );
		if( fDist < 1000.f * 1000.f )
		{
			continue;
		}

		fDist = D3DXVec2Length( &TempDir );
		if( fDist < 4000.f && fDist > 0.f )
			fDist = (4000.f - fDist) / 4000.f * 0.4f;
		else
			fDist = 0.f;

		TempDir = TempDir * fDist;
		Dir = Dir + TempDir;
	}
	Dir = Dir + TargetDir;

	D3DXVec2Normalize( &Dir, &Dir );
	Dir = Dir * GetMoveSpeed() * 80.f;
	D3DXVECTOR2 Pos = Dir + D3DXVECTOR2( m_Pos.x, m_Pos.z );

	return Pos;
}
*/


bool CFcTroopObject::IsEnemyInAttackRange( TroopObjHandle hTarget, float radius )
{
	D3DXVECTOR2 Dir = hTarget->GetPosV2() - GetPosV2();
	float fDist = D3DXVec2Length( &Dir );
	if( fDist < radius )
	{
		return true;
	}
	return false;
}


// 못가는 속성위에 있는 녀석들 위치를 중심쪽으로 가는 속성을 찾아서 거기로 위치를 옮겨준다.
void CFcTroopObject::AdjustPositionByAttr()
{
	if( m_Handle == g_FcWorld.GetHeroHandle()->GetTroop() )
		return;

	// TroopOnProp같은 경우 Formation이 없다.
	int nFormationCnt = m_vecTroopsFormation[m_nCurFormationType].size();
	if( nFormationCnt == 0 )
		return;

	int nBasicDepth = 1;
	if( m_pCrossProp )
	{
		nBasicDepth = 3;
	}	

	float fMapXSize, fMapYSize;
	g_FcWorld.GetMapSize( fMapXSize, fMapYSize );

	for( int i=0; i<m_nUnitNum; i++ )
	{
		if( m_pUnits[i] == NULL )
			continue;
/*
		if( hUnit->IsDie() )		// 죽은 녀석들은 이미 erase 됐다.
			continue;
*/
		D3DXVECTOR2 Pos = m_pUnits[i]->GetTroopOffset();
		Pos += GetPosV2();
		if( Pos.x < 0 || Pos.x >= fMapXSize && Pos.y < 0 || Pos.y >= fMapYSize )
			continue;

		BYTE cAttr = CFcWorld::GetInstance().GetAttr( Pos.x, Pos.y );
		BYTE cAttrEx = CFcWorld::GetInstance().GetAttrEx( Pos.x, Pos.y );
		if( IsMovableAttr( Pos.x, Pos.y, 1, GetTeam() == 0, fMapXSize, fMapYSize ) == false )
		{
			D3DXVECTOR2 Dir = GetPosV2() - Pos;

			float fDist = D3DXVec2Length( &Dir );
			D3DXVec2Normalize( &Dir, &Dir );

			int nDepth = nBasicDepth;
			float fStep=80.f;

int nDebugCount = 0;

			while(1)
			{
/*
DebugString( "DC:%d\n", nDebugCount );
++nDebugCount;
*/
				D3DXVECTOR2 NewDir = Dir * fStep;
				D3DXVECTOR2 UnitPos = Pos + NewDir;
				BYTE cAttr = g_FcWorld.GetAttr( UnitPos.x, UnitPos.y );
				BYTE cAttrEx = g_FcWorld.GetAttrEx( UnitPos.x, UnitPos.y );

				fStep += 80.f;
				if( fStep > fDist + m_fRadius )	// 갈 수 있는 속성 없음
					break;

				if( Pos.x < 0 || Pos.x >= fMapXSize && Pos.y < 0 || Pos.y >= fMapYSize )
					continue;

				if( IsMovableAttr( UnitPos.x, UnitPos.y, 1, GetTeam() == 0, fMapXSize, fMapYSize ) == true )
				{
					if( nDepth == 0 )
					{
						D3DXVECTOR2 Offset = UnitPos - GetPosV2();
						m_pUnits[i]->SetTroopOffset( &Offset );
						break;
					}
					else
						--nDepth;
				}
				
			}
		}
	}

	// 다리 속성에서 약간 떨어지게 처리
	if( m_pCrossProp )
	{
		for( int i=0; i<m_nUnitNum; i++ )
		{
			if( m_pUnits[i] == NULL )
				continue;

			D3DXVECTOR2 Pos = m_pUnits[i]->GetTroopOffset();
			Pos += GetPosV2();
			if( Pos.x < 0 || Pos.x >= fMapXSize && Pos.y < 0 || Pos.y >= fMapYSize )
				continue;

			D3DXVECTOR2 MoveDir;
			bool bExistBridgeAttr = GetNearBridgeAttr( Pos, &MoveDir );
			if( bExistBridgeAttr )
			{
				int nDepth = 5;	// 4.5칸 떨어짐
				float fStep=80.f;

				int nDebugCount = 0;

				while(1)
				{
					D3DXVECTOR2 NewDir = MoveDir * fStep;
					D3DXVECTOR2 UnitPos = Pos + NewDir;
					BYTE cAttr = g_FcWorld.GetAttr( UnitPos.x, UnitPos.y );
					BYTE cAttrEx = g_FcWorld.GetAttrEx( UnitPos.x, UnitPos.y );

					fStep += 80.f;
					if( fStep > 320 )	// 갈 수 있는 속성 없음
						break;

					if( Pos.x < 0 || Pos.x >= fMapXSize && Pos.y < 0 || Pos.y >= fMapYSize )
						continue;

					if( IsMovableAttr( UnitPos.x, UnitPos.y, 1, GetTeam() == 0, fMapXSize, fMapYSize ) == true )
					{
						if( nDepth == 0 )
						{
							D3DXVECTOR2 Offset = UnitPos - GetPosV2();
							m_pUnits[i]->SetTroopOffset( &Offset );
							break;
						}
						else
							--nDepth;
					}
				}
			}
		}
	}


 	bool bIn = true;
	for( int i=0; i<m_nUnitNum; i++ )
	{
		GameObjHandle hUnit = m_pUnits[i];
		if( hUnit == NULL )
			continue;

		if( hUnit->IsDie() )
			continue;

		D3DXVECTOR2 Dir = GetPosV2() + hUnit->GetTroopOffset() - hUnit->GetPosV2();
		float fDistSq = D3DXVec2LengthSq( &Dir );
		if( fDistSq > ( 3000.f * 3000.f ) )
		{
			bIn = false;	
			break;
		}
	}
}

// 플레이어는 여기 들어오면 안된다.
bool CFcTroopObject::IsMovableAttr( float fX, float fY, int nRadius, bool bFriend, float fMapXSize, float fMapYSize )
{
	int nX = (int)(fX / 100.f) - nRadius;
	int nY = (int)(fY / 100.f) - nRadius;

	int nMapXSize = (int)fMapXSize;
	int nMapYSize = (int)fMapYSize;

	int nCnt = nRadius * 2 + 1;
	for( int i=0; i<nCnt; i++ )
	{
		for( int j=0; j<nCnt; j++ )
		{
			if( (nX + i) < 0 || (nX + i) >= nMapXSize && (nY + j) < 0 || (nY + j) >= nMapYSize )
				continue;

			BYTE cAttr = g_FcWorld.GetAttr( nX + i, nY + j );
			BYTE cAttrEX = g_FcWorld.GetAttrEx( nX + i, nY + j );
			if( (cAttr & 0x3) ||					// 일반, 유닛 속성
				(bFriend && ( cAttrEX & 0x8 ) ) ||	// 아군 속성
				(cAttr & 0x10) ||					// 히어로만 갈 수 있는 속성
				(cAttr & 0x8) )						// 절벽 속성
			{
				return false;
			}
		}
	}
	return true;
}

bool CFcTroopObject::GetNearBridgeAttr( D3DXVECTOR2 Pos, D3DXVECTOR2* MoveDir )
{
	bool bRet = false;
	*MoveDir = D3DXVECTOR2( 0.f, 0.f );

	float fSX = -1.f;
	float fSY = -1.f;
	for( int i=0; i<3; i++ )
	{
		fSX = -1.f;
		for( int j=0; j<3; j++ )
		{
			BYTE cAttr = g_FcWorld.GetAttr( Pos.x + fSX * 100.f, Pos.y + fSY * 100.f );
			// 다리 속성이면
			if( cAttr & 0x4 )
			{
				*MoveDir = *MoveDir + D3DXVECTOR2( fSX, fSY );
				bRet = true;
			}
			fSX += 1.f;
		}
		fSY += 1.f;
	}
	if( bRet == true )
	{
		D3DXVec2Normalize( MoveDir, MoveDir );
		return true;
	}
	return false;
}

void CFcTroopObject::CaculateDir( D3DXVECTOR2* pNewDir )
{
	float fDot = D3DXVec2Dot( &m_Dir, pNewDir );

	if( fDot > 0.707106781f )
	{
		m_Dir = *pNewDir;
	}
	else if( fDot < -0.707106781f )
	{
		m_Dir = -(*pNewDir);
	}
	else
	{
		D3DXVECTOR2 Dir = D3DXVECTOR2( -m_Dir.y, m_Dir.x );
		float fDot = D3DXVec2Dot( &Dir, pNewDir );
		if( fDot > 0.707106781f + 0.01f )
		{
			m_Dir.x = pNewDir->y;
			m_Dir.y = -pNewDir->x;
		}
		else
		{
			m_Dir.x = -pNewDir->y;
			m_Dir.y = pNewDir->x;
		}
	}
}

void CFcTroopObject::OnProcessMeleeFinish()
{
	// 발석차 부대의 제약 사항 : 리더가 없다. 발석차가 1대여야 한다.
	if( GetType() == TROOPTYPE_SIEGE )
	{
		BsAssert( m_hLeader == NULL );
		BsAssert( (int)m_nUnitNum>0 );

		GameObjHandle hUnit;
		for( int i=0; i<m_nUnitNum; i++ )
		{
			if( m_pUnits[i] != NULL )
			{
				hUnit = m_pUnits[i];
				break;
			}
		}
		if( hUnit )
			m_Pos = hUnit->GetPos();
	}
}


//// folded ///////////////////////////////////////////////////////
//
// 부대들이 follow할 때 마다, follwer리스트의 쫓는 부대의 rotten값을 0으로 만들고 다른 follower들의 rotten값을 
// 1 증가 시킨다. 이렇게 하면 follow 하다가 더 이상 follow하지 않는 부대의 rotten값은 계속 증가하게 된다.
// 동시에, rotten값이 follower 리스트 크기보다 큰 follower는 삭제한다.
// 즉시즉시 follower 리스트가 갱신되지 않지만, 맥시멈 TROOP_FOLLOWER_MAX tick 내에 완전히 갱신된다.
// 
///////////////////////////////////////////////////////////////////

int	CFcTroopObject::GetFollowerCount(int nTroopId)
{
	int nScan;
	int nRValue = -1;

	for(nScan = 0;nScan < m_nFollower;nScan++)
	{
		if (m_Follower[nScan].nFollower == nTroopId)
		{
			m_Follower[nScan].nRotten = 0;
			nRValue = nScan;
		}
		else
			m_Follower[nScan].nRotten++;
	}

	if (nRValue == -1)
	{
		BsAssert(nScan < TROOP_FOLLOWER_MAX  );// && "너무 많은 수의 부대가 하나의 부대를 따라가고 있습니다.");

		m_Follower[nScan].nFollower = nTroopId;
		m_Follower[nScan].nRotten = 0;
		m_nFollower++;
		nRValue = nScan;
	}

	int	nCopy1, nCopy2;

	nCopy1 = 0;
	nCopy2 = m_nFollower;

	for(nScan = 0;nScan < nCopy2;nScan++)
	{
		if (m_Follower[nScan].nRotten > nCopy2)
		{
			m_nFollower--;
			continue;
		}

		if (nCopy1 != nScan)
			m_Follower[nCopy1] = m_Follower[nScan];

		nCopy1++;
	}

	return nRValue;
}

void CFcTroopObject::DebugRender()
{
	if( !IsEliminated() && IsEnable() ) {
		static const int nIndex1 = g_BsKernel.LoadTexture( "DebugRes\\TroopTerritory.dds" );	// for debug
		g_BsKernel.AddDecal( m_Pos.x, m_Pos.z, nIndex1, GetRadius( false ), 0, 0 );

		static const int nIndex2 = g_BsKernel.LoadTexture( "DebugRes\\UnitPos.dds" );
		for( int i=0; i<m_nUnitNum; i++ )
		{
			if( !CFcBaseObject::IsValid( m_pUnits[i] ) ) continue;
			D3DXVECTOR2 vVec;
			vVec.x = m_pUnits[i]->GetTroopOffset().x + m_Pos.x;
			vVec.y = m_pUnits[i]->GetTroopOffset().y + m_Pos.z;

			if( m_pUnits[i] == NULL )
				continue;

			g_BsKernel.AddDecal( vVec.x, vVec.y, nIndex2, m_pUnits[i]->GetUnitRadius(), 0, 0 );
		}
	}
	/*
#ifndef _XBOX
	C3DDevice* pDevice = g_BsKernel.GetDevice();
	pDevice->SaveState();
	pDevice->BeginScene();

	D3DXMATRIX mat, matWorld;
	pDevice->GetTransform( D3DTS_WORLD, &matWorld );
	D3DXMatrixIdentity( &mat );
	pDevice->SetTransform( D3DTS_WORLD, &mat );

	pDevice->SetPixelShader(NULL);
	pDevice->SetVertexShader(NULL);z
	for( DWORD i=0; i<8; i++ ) pDevice->SetTexture( i, NULL );
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, false);
	pDevice->SetRenderState( D3DRS_LIGHTING, FALSE );


	struct LINE_VERTEX
	{
		D3DXVECTOR3 v;
		DWORD       color;
	};
	static LINE_VERTEX pVertices[1000];


	pDevice->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );

	std::vector<D3DXVECTOR3> *pVecList = GetMoveQueue();

	if( pVecList->size() >= 2 ) {
		for( DWORD i=0; i<pVecList->size()-1; i++ ) {
			pVertices[0].v = (*pVecList)[i]; pVertices[0].v.y += 200.f;
			pVertices[1].v = (*pVecList)[i+1]; pVertices[1].v.y += 200.f;
			pVertices[0].color = 0xFFFF0000;
			pVertices[1].color = 0xFFFF0000;
			pDevice->DrawPrimitiveUP( D3DPT_LINELIST, 1, pVertices, sizeof(LINE_VERTEX) );
		}
	}


	pDevice->SetTransform( D3DTS_WORLD, &matWorld );
	pDevice->EndScene();
	pDevice->RestoreState();
#endif //_XBOX
	*/
}

void CFcTroopObject::InitUnitArray( int nNum )
{
	if( m_pUnits )
	{
		BsAssert( m_nUnitNum >= nNum );
	}
	else
	{
		m_nUnitNum = nNum;
		m_pUnits = new GameObjHandle[m_nUnitNum];
	}
}

void CFcTroopObject::PostUnitArray()
{
	int nExtraCnt = m_vecExtraUnits.size();
	if( nExtraCnt > 0 )
	{
		GameObjHandle* pTemp = new GameObjHandle[m_nUnitNum + nExtraCnt];
		if( m_pUnits )
		{
			for( int i=0; i<m_nUnitNum; i++ )
			{
				pTemp[i] = m_pUnits[i];
			}
		}
//		memcpy( pTemp, m_pUnits, sizeof( GameObjHandle ) * m_nUnitNum );

		for( int i=0; i<nExtraCnt; i++ )
		{
			pTemp[m_nUnitNum + i] = m_vecExtraUnits[i];
		}
		if( m_pUnits )
			SAFE_DELETEA( m_pUnits );
		m_pUnits = pTemp;
		m_nUnitNum += (int)m_vecExtraUnits.size();
		m_vecExtraUnits.clear();
	}
}

///////////////////////////////////////////////////////////////////


/*
void CFcTroopObject::SetCrossUnitOffset()
{
	int nCnt = m_vecObj.size();

	

	for( int i=0; i<nCnt; i++ )
	{
		const D3DXMATRIX Mat = m_pCrossProp->GetMat();
		
		D3DXVECTOR3 Dir(Mat._31, Mat._32, Mat._33 );
		D3DXVECTOR3 CrossDir(Mat._11, Mat._12, Mat._13 );




		D3DXVECTOR2 Pos = GetPosV2();

		
	}
}
*/


void CFcTroopObject::SetDirByClock(int nClock,int nTotalTime)
{
	CCrossVector *pCross = NULL;
	for(int i = 0; i < m_nUnitNum; i++)
	{
		if( m_pUnits[i] == NULL )
			continue;

		pCross = m_pUnits[i]->GetCrossVector();
		pCross->RotateYaw( ( 1023 / nTotalTime ) * ( nClock) );
	}
}


void CFcTroopObject::SetHPPercent(int nPercent)
{
	float fPer = (float)nPercent / 100.f;
	for(int i = 0;i < m_nUnitNum;i++)
	{
		if( m_pUnits[i] == NULL )
			continue;

		if( m_pUnits[i]->IsDie() )
			continue;

		m_pUnits[i]->SetHP((int)(fPer * (float)m_pUnits[i]->GetMaxHP()));

		if( m_pUnits[i]->GetHP() <= 0 )
			m_pUnits[i]->ChangeAnimation( ANI_TYPE_DIE, -1 );
	}
}

void CFcTroopObject::AddHPPercent(int nPercent)
{
	float fPer = static_cast<float>(nPercent) / 100.f;
	for(int i = 0;i < m_nUnitNum;i++)
	{
		if( m_pUnits[i] == NULL )
			continue;

		if( m_pUnits[i]->IsDie() )
			continue;

		m_pUnits[i]->AddHP( static_cast<int>( fPer * static_cast<float>( m_pUnits[i]->GetMaxHP() ) ) );

		if( m_pUnits[i]->GetHP() <= 0 )
			m_pUnits[i]->ChangeAnimation( ANI_TYPE_DIE, -1 );
	}
}

void CFcTroopObject::SetDie()
{
	for(int i = 0;i < m_nUnitNum;i++)
	{
		if( m_pUnits[i] == NULL )
			continue;

		if( !m_pUnits[i]->IsDie() ) {
			m_pUnits[i]->ChangeAnimation(ANI_TYPE_DIE , 0);
			m_pUnits[i]->AddHP( -m_pUnits[i]->GetHP() );
		}
	}
}

void CFcTroopObject::SaveMaterialAmbient()
{
	for(int i = 0;i < m_nUnitNum;i++)
	{			
		if( m_pUnits[i] == NULL )
			continue;

		m_pUnits[i]->SaveMaterialAmbient();
	}
}

void CFcTroopObject::SetMaterialAmbient(D3DXVECTOR4 vColor)
{
	for(int i = 0;i < m_nUnitNum;i++)
	{
		if( m_pUnits[i] == NULL )
			continue;

		m_pUnits[i]->SetMaterialAmbient(vColor);
	}
}

void CFcTroopObject::RestoreMaterialAmbient()
{
	for(int i = 0;i < m_nUnitNum;i++)
	{
		if( m_pUnits[i] == NULL )
			continue;

		m_pUnits[i]->RestoreMaterialAmbient();
	}
}


void CFcTroopObject::ShowObjectBaseDecal(int nTexIndex, int nTick, int nAlphaMode)
{
	for( int i=0; i<m_nUnitNum; i++ )
	{
		if( m_pUnits[i] == NULL )
			continue;

		if( m_pUnits[i]->IsEnable() == false )
			continue;

		if( m_pUnits[i]->IsDie() )
			continue;

		D3DXVECTOR2 vVec = m_pUnits[i]->GetPosV2();
		if(nAlphaMode < 0 || nAlphaMode > 1){
			BsAssert(0);
		}
		g_BsKernel.AddDecal( vVec.x, vVec.y, nTexIndex, m_pUnits[i]->GetUnitRadius(), nTick, 0, D3DCOLOR_XRGB(255,255,255), nAlphaMode );
	}
}

bool CFcTroopObject::IsMoving()
{
	if( s_nTroopStateTable[ m_State.State ] & UNIT_MOVABLE )
	{
		return true;
	}
	return false;
}
	
void CFcTroopObject::SetInitUnitData( TROOP_ATTR* pData )
{
	BsAssert( m_pInitUnitData == NULL );
	m_pInitUnitData = new TROOP_ATTR;
	memcpy( m_pInitUnitData, pData, sizeof( TROOP_ATTR ) );
}


void CFcTroopObject::CheckUnitKind()
{
	int i, nSoxIndex;

	m_nGoblinCount = 0;
	m_nOrcCount = 0;
	m_nElfCount = 0;
	m_nEtcUnitCount = 0;
	for( i = 0; i < GetUnitCount(); i++ )
	{
		if( !m_pUnits[ i ] )
		{
			continue;
		}
		if( m_pUnits[ i ]->IsDie() )
		{
			continue;
		}
		nSoxIndex = m_pUnits[ i ]->GetUnitSoxIndex();

		if( nSoxIndex == 28 || nSoxIndex == 29 || nSoxIndex == 30 ||
			nSoxIndex == 34 || nSoxIndex == 62 ||
			nSoxIndex == 63 || nSoxIndex == 64 ) //고블린
		{
			m_nGoblinCount++;
		}
		else if( nSoxIndex > 13 || nSoxIndex < 26 ) { //오크
			m_nOrcCount++;
		}
		else if( nSoxIndex == 47 || nSoxIndex == 31 ) { //엘프
			m_nElfCount++;
		}
		else{
			m_nEtcUnitCount++;
		}
	}
}


void CFcTroopObject::CheckCulling()
{
/*
	if( m_bEnable == false )
	{
		m_bRenderSkip = true;
		return;
	}

	int nLoop;
	float fLength[2] = { -1, -1 };
	float fDot;
	D3DXVECTOR3 Distance[ 2 ];
	D3DXVECTOR3	Dir[ 2 ];
	CameraObjHandle CamHandle[2];
	float fRadius = GetRadius();

	for( nLoop = 0; nLoop < g_FCGameData.nPlayerCount; nLoop++ )
	{
		CamHandle[nLoop] = CFcBaseObject::GetCameraObjectHandle( nLoop );
		if( CamHandle[nLoop] )
		{
			Distance[ nLoop ] = m_Pos  - CamHandle[nLoop]->GetPos();
			fLength[nLoop] = D3DXVec3Length( Distance + nLoop );
			Dir[ nLoop ] = Distance[ nLoop ] / fLength[nLoop];
		}
	}

	m_bRenderSkip = true;
	for( nLoop = 0; nLoop < g_FCGameData.nPlayerCount; nLoop++ )
	{
		if( fLength[nLoop] < 50000.0f + fRadius )	//500미터 보다 가까이 있는 경우
		{
			if(CamHandle[nLoop].GetPointer() == NULL)
				continue;

			if( fLength[nLoop] <  fRadius * 2 ) {	// 아주 가까우면 무조건 보임
				m_bRenderSkip = false;
				break;
			}
			else {
				fDot = D3DXVec3Dot( Dir + nLoop, &( CamHandle[nLoop]->GetCrossVector()->m_ZVector ) );

				// 원뿔과 원뿔의 교차판정
				static const float fSqRatio = sqrtf((720.f/1280)*(720.f/1280)+1);
				float fHalfFovAngle = CamHandle[nLoop]->GetCamFov() * 0.5f * fSqRatio;
				float fHalfTroopAngle = acosf(fLength[nLoop] / sqrtf( (fRadius * fRadius) + (fLength[nLoop]*fLength[nLoop]) ));
				float fCosAngle = cosf(fHalfFovAngle + fHalfTroopAngle);

				if( fDot >=  fCosAngle ) {	// 보이는 경우
					m_bRenderSkip = false;
					break;
				}
			}
		}
	}

	m_bTriggerCmd = false;
	m_bUseTriggerCmd = false;
*/

	if( m_bEnable == false )
	{
		m_bRenderSkip = true;
		return;
	}
	float fLength = -1.0;
	float fDot;
	D3DXVECTOR3 Distance;
	float fRadius = GetRadius();

	CameraObjHandle CamHandle = CFcBaseObject::GetCameraObjectHandle( 0 );
	if( CamHandle )
	{
		Distance = m_Pos - CamHandle->GetPos();
		fLength = D3DXVec3Length( &Distance );
	}
	bool bValid = true;
	if( fLength < 50000.0f + fRadius )	
	{
		fDot = D3DXVec3Dot( &Distance, &( CamHandle->GetCrossVector()->m_ZVector ) );
		// 카메라 뒤에 있고.. 거리가 반지름 2배 이상 떨어져 있으면 스킵한다.
		if( ( fDot >= 0 ) || ( fLength < ( fRadius * 2.f ) ) )
		{
			bValid = false;
		}
	}

	if( bValid )
		m_bRenderSkip = true;
	else
		m_bRenderSkip = false;

	m_bTriggerCmd = false;
	m_bUseTriggerCmd = false;
}

void CFcTroopObject::SetFullHP()
{
	// Calculate Max HP
	m_nMaxHP = 0;
	for( int i=0; i<m_nUnitNum; i++ )
	{
		if( m_pUnits[i] == NULL )
			continue;

		m_nMaxHP += m_pUnits[i]->GetMaxHP();
	}
	m_nHP = m_nMaxHP;
}


void CFcTroopObject::SetHP()
{
	int nHP = 0;
	for( int i=0; i<m_nUnitNum; i++ )
	{
		if( m_pUnits[i] == NULL )
			continue;

		if( m_pUnits[i]->IsDie() )
			continue;

		nHP += m_pUnits[i]->GetHP();
	}
	m_nHP = nHP;
}


void CFcTroopObject::ResetWayPoint()
{
	for( int i=0; i<m_nUnitNum; i++ )
	{
		if( m_pUnits[i] == NULL )
			continue;
		
		m_pUnits[i]->ResetWayPoint();
	}
}

void CFcTroopObject::ResetState()
{
	m_State.State = TROOPSTATE_NON;
	if( m_bUseTriggerCmd )
		m_bUseTriggerCmd = false;
}


bool CFcTroopObject::CanIProcess()
{
	if( m_bEnable == false )
		return false;

	if( m_bStopProcess )
		return false;

	return true;
}


/////////////////////////////////////////////////////////////
// CFcRangeTroop class
CFcRangeTroop::CFcRangeTroop()
{
	
}


CFcRangeTroop::~CFcRangeTroop()
{

}

bool CFcRangeTroop::Initialize( int nX, int nY, int nRadius,
				int nForce, int nTeam, 
				int nID, char* pName, int nTroopType, 
				int nCreateCount )
{
	CFcTroopObject::Initialize( nX, nY, nRadius, nForce, nTeam, nID, pName, nTroopType, nCreateCount );
	return true;
}


bool CFcRangeTroop::CmdAttack( TroopObjHandle hTarget, bool bBoost, bool bForce )
{
	return CmdRangeAttack( hTarget, bBoost, bForce );
}

void CFcRangeTroop::Process()
{
	if( CanIProcess() == false )
		return;

	CFcTroopObject::Process();

	switch( m_State.State )
	{
	case TROOPSTATE_RANGE_ATTACK:
		if( ProcessRangeAttack() == false )
		{
			m_State.State = TROOPSTATE_NON;
		}
		break;
	}

//	CheckRangeAttack();
}

bool CFcRangeTroop::CmdEngageMercifully( TroopObjHandle hTarget, std::vector<TroopObjHandle>& vecVoidTroops )
{
	bool bRet = CFcTroopObject::CmdEngageMercifully( hTarget, vecVoidTroops );
	CheckRangeAttack();
	return bRet;
}


void CFcRangeTroop::CheckRangeAttack()
{
/*
	std::vector< TroopObjHandle > vecObjs;
	g_FcWorld.GetEnemyTroopListInRange( &GetPosV2(), m_nGroup, m_fAttackRadius, vecObjs );

	int nCnt = vecObjs.size();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTarget = vecObjs[i];
		if( hTarget->IsEliminated() )
		{
			continue;
		}
		if( !hTarget->IsEnable() )
		{
			continue;
		}
		CmdAttack( hTarget );
	}
*/

	if( IsMeleeEngaged() == true )
		return;

	if( m_State.hTarget == NULL )
		return;

	if( m_State.hTarget->IsEnable() == false )
		return;

	if( m_State.hTarget->IsEliminated() )
		return;

	if( !CFcTroopObject::IsEnemyInAttackRange( m_State.hTarget, m_fAttackRadius ) )
		return;

	CmdAttack( m_State.hTarget );
}


bool CFcRangeTroop::ProcessRangeAttack()
{
	BsAssert( m_State.hTarget );
	if( GetProcessTick() % 20 == ( abs(m_nTroopID) % 20 ) )
	{
		if( !CFcTroopObject::IsEnemyInAttackRange( m_State.hTarget, m_fAttackRadius ) )
		{
			return false;
		}
	}
	if( m_State.hTarget->IsEliminated() )
	{
		return false;
	}
	return true;
}


bool CFcRangeTroop::ProcessHold()
{
	BsAssert( m_State.hTarget );
	if( GetProcessTick() % 20 == ( abs(m_nTroopID) % 20 ) )
	{
		if( !CFcTroopObject::IsEnemyInAttackRange( m_State.hTarget, m_fAttackRadius ) )
		{
			m_State.hTarget.Identity();
			return false;
		}
	}
	if( m_State.hTarget->IsEliminated() )
	{
		m_State.hTarget.Identity();
		return false;
	}

	return true;
}

// 지금은 ProcessHold와 똑같음
bool CFcRangeTroop::ProcessHoldMercifully()
{
	BsAssert( m_State.hTarget );
	if( GetProcessTick() % 20 == ( abs(m_nTroopID) % 20 ) )
	{
		if( !CFcTroopObject::IsEnemyInAttackRange( m_State.hTarget, m_fAttackRadius ) )
		{
			m_State.hTarget.Identity();
			return false;
		}
	}
	if( m_State.hTarget->IsEliminated() )
	{
		m_State.hTarget.Identity();
		return false;
	}

	return true;
}



void CFcRangeTroop::PostInitialize()
{
	CFcTroopObject::PostInitialize();
	m_fAttackRadius = (float)g_TroopSOX.GetRange( TROOPTYPE_RANGEMELEE, GetLevel() );

}

bool CFcRangeTroop::IsEnemyInAttackRange( TroopObjHandle hTarget )
{
	return m_bAttackable && CFcTroopObject::IsEnemyInAttackRange( hTarget, m_fAttackRadius );
}






/////////////////////////////////////////////////////////////
// CFcOnPropTroop Class
CFcOnPropTroop::CFcOnPropTroop()
{
	m_pParentProp = NULL;
}


CFcOnPropTroop::~CFcOnPropTroop()
{

}

bool CFcOnPropTroop::Initialize( CFcProp* pProp )
{
	m_Type = TROOPTYPE_ON_PROP;
	m_Pos = pProp->GetPos();
//	m_ParentPropID = pProp->GetID();
	SetRadius( 4000.f );			// 부대 체크 범위 하드코딩
	m_pParentProp = pProp;

	// mruete: added temp var and assert
	const char * pPropName = pProp->GetName();
	BsAssert( NULL != pPropName );
	strcpy( m_cName, pPropName );

	m_nForce = pProp->GetParam( 1 );
	m_nTeam = pProp->GetParam( 2 ); 
	return true;
}

/*
bool CFcOnPropTroop::CmdAttack( TroopObjHandle hTarget )
{
	if( hTarget == NULL )
	{
		return false;
	}
	if( IsEnemyInAttackRange( hTarget, m_fAttackRadius ) )
	{
		if( SetState( TROOPSTATE_RANGE_ATTACK ) == false )
		{
			return false;
		}
		m_State.hTarget = hTarget;
	}
	else
	{
		D3DXVECTOR3 Pos = hTarget->GetPos();
		CmdMoveAttack( Pos.x, Pos.z );
	}

	return true;
}


void CFcOnPropTroop::Process()
{
	if( m_bEnable == false )
	{
		return;
	}

	CFcTroopObject::Process();

	switch( m_State.State )
	{
	case TROOPSTATE_RANGE_ATTACK:
		if( ProcessRangeAttack() == false )
		{
			m_State.State = TROOPSTATE_NON;
		}
		break;
	}

	CheckRangeAttack();
}
*/







/////////////////////////////////////////////////////////////
// CFcFlyTroop Class
CFcFlyTroop::CFcFlyTroop()
{
	m_bUseNavMesh = false;
	m_fAttackRadius = 0.f;
}

CFcFlyTroop::~CFcFlyTroop()
{
}

bool CFcFlyTroop::Initialize( int nX, int nY, int nRadius, int nForce, int nTeam, int nID, 
							 char* pName, int nTroopType, int nCreateCount )
{
	CFcTroopObject::Initialize( nX, nY, nRadius, nForce, nTeam, nID, pName, nTroopType, nCreateCount );
	return true;
}

bool CFcFlyTroop::CmdAttack( TroopObjHandle hTarget, bool bBoost, bool bForce )
{
	if( hTarget == NULL )
	{
		return false;
	}
	if( CFcTroopObject::IsEnemyInAttackRange( hTarget, m_fAttackRadius ) )
	{
		if( SetState( TROOPSTATE_RANGE_ATTACK ) == false )
		{
			return false;
		}
		m_State.hTarget = hTarget;
	}
	else
	{
		D3DXVECTOR3 Pos = hTarget->GetPos();
		CmdMoveAttack( Pos.x, Pos.z );
	}

	return true;
}


bool CFcFlyTroop::CmdEngageMercifully( TroopObjHandle hTarget, std::vector<TroopObjHandle>& vecVoidTroops )
{
	bool bRet = CFcTroopObject::CmdEngageMercifully( hTarget, vecVoidTroops );
	CheckRangeAttack();
	return bRet;
}


void CFcFlyTroop::CheckRangeAttack()
{

/*
//	if( IsMeleeEngaged() == true )
//	{
//		return;
//	}

	std::vector< TroopObjHandle > vecObjs;
	g_FcWorld.GetEnemyTroopListInRange( &GetPosV2(), m_nGroup, m_fAttackRadius, vecObjs );

	int nCnt = vecObjs.size();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTarget = vecObjs[i];
		if( hTarget->IsEliminated() )
		{
			continue;
		}
		if( !hTarget->IsEnable() )
		{
			continue;
		}
		CmdAttack( hTarget );
	}
*/

	if( m_State.hTarget == NULL )
		return;

	if( m_State.hTarget->IsEnable() == false )
		return;

	if( m_State.hTarget->IsEliminated() )
		return;

	if( !CFcTroopObject::IsEnemyInAttackRange( m_State.hTarget, m_fAttackRadius ) )
		return;

	CmdAttack( m_State.hTarget );
}

bool CFcFlyTroop::ProcessRangeAttack()
{
	BsAssert( m_State.hTarget );
	if( GetProcessTick() % 20 == ( abs(m_nTroopID) % 20 ) )
	{
		if( !CFcTroopObject::IsEnemyInAttackRange( m_State.hTarget, m_fAttackRadius ) )
		{
			return false;
		}
	}
	if( m_State.hTarget->IsEliminated() )
	{
		return false;
	}
	return true;
}


void CFcFlyTroop::Process()
{
	if( CanIProcess() == false )
		return;

	CFcTroopObject::Process();

	switch( m_State.State )
	{
	case TROOPSTATE_RANGE_ATTACK:
		if( ProcessRangeAttack() == false )
		{
			m_State.State = TROOPSTATE_NON;
		}
		break;
	}
//	CheckRangeAttack();
}


void CFcFlyTroop::PostInitialize()
{
	CFcTroopObject::PostInitialize();
	m_fAttackRadius = (float)g_TroopSOX.GetRange( TROOPTYPE_FLYING, GetLevel() );
}

bool CFcFlyTroop::IsEnemyInAttackRange( TroopObjHandle hTarget )
{
	return CFcTroopObject::IsEnemyInAttackRange( hTarget, m_fAttackRadius );
}



/*
/////////////////////////////////////////////////////////////
// CFcGuardianTroop class
CFcGuardianTroop::CFcGuardianTroop()
{
	m_CmdState = GUARDIAN_CMD_STATE_NON;
	m_pNearCrossProp = NULL;
}

CFcGuardianTroop::~CFcGuardianTroop()
{

}

void CFcGuardianTroop::SetHero( HeroObjHandle hHero )
{
	m_hHero = hHero;
	m_PreHeroPos = m_hHero->GetPosV2();
}




#define GUARDIAN_SEARCH_ENEMY_RADIUS		3000.f
#define GUARDIAN_SEARCH_ENEMY_DIST			3000.f

void CFcGuardianTroop::CmdGuardianAttack( D3DXVECTOR2* Dir )
{
	D3DXVECTOR2 TargetDir = *Dir * GUARDIAN_SEARCH_ENEMY_RADIUS;
	D3DXVECTOR2 Pos = TargetDir + GetPosV2();

	TroopObjHandle hTarget = g_FcWorld.GetNearMeleeEnemyInRange( &Pos, 0, GUARDIAN_SEARCH_ENEMY_RADIUS );
	// 호위병은 플레이어를 쫓아다니기 때문에 PathFind하기 전에 InitNavCell을 해줘야 한다.
	InitNavCell();
	m_CmdState = GUARDIAN_CMD_STATE_ATTACK;

	if( hTarget )
	{
		CmdAttack( hTarget );
	}
	else
	{
		CmdMoveAttack( Pos.x, Pos.y );
	}
}


#define GUARDIAN_SEARCH_DEFENSE_ENEMY_RADIUS	5000.f

void CFcGuardianTroop::CmdGuardianDefense( D3DXVECTOR2* Dir )
{
	D3DXVECTOR2 Pos = GetPosV2();
	TroopObjHandle hTarget = g_FcWorld.GetNearMeleeEnemyInRange( &Pos, 0, GUARDIAN_SEARCH_ENEMY_RADIUS );
	SetDefenseFormation( hTarget );
	m_CmdState = GUARDIAN_CMD_STATE_DEFENSE;
}

void CFcGuardianTroop::CmdGuardianDefenseHero()
{
	SetDefenseHeroFormation();
	m_CmdState = GUARDIAN_CMD_STATE_DEFENSE_HERO;
}

void CFcGuardianTroop::CmdGuardianSurroundEnemy( D3DXVECTOR2* Dir )
{

}

void CFcGuardianTroop::Process()
{
	CFcTroopObject::Process();

//	if( m_State.State == TROOPSTATE_CROSS_BRIDGE )
//	{
//		return;
//	}


	switch( m_CmdState )
	{
	case GUARDIAN_CMD_STATE_NON:
		ProcessGuardianNormalMode();
		break;

	case GUARDIAN_CMD_STATE_ATTACK:
		if( ProcessGuardianAttack() == false )
		{
			m_CmdState = GUARDIAN_CMD_STATE_NON;
		}
		break;

	case GUARDIAN_CMD_STATE_DEFENSE:
		if( ProcessGuardianDefense() == false )
		{
			m_CmdState = GUARDIAN_CMD_STATE_NON;
		}
		break;

	case GUARDIAN_CMD_STATE_DEFENSE_HERO:
		if( ProcessGuardianDefenseHero() == false )
		{
			m_CmdState = GUARDIAN_CMD_STATE_NON;
		}
		break;

	case GUARDIAN_CMD_STATE_SURROUND_ENEMY:
		if( ProcessGuardianSurroundEnemy() == false )
		{
			m_CmdState = GUARDIAN_CMD_STATE_NON;
		}
		break;
	}
}


// 히어로를 따라 다닌다
bool CFcGuardianTroop::ProcessGuardianNormalMode()
{
	if( m_pNearCrossProp == NULL )
	{
		m_Pos = m_hHero->GetPos();
	}
	else
	{
		m_Pos = D3DXVECTOR3( m_CrossStartPos.x, 0.f, m_CrossStartPos.y );
	}

	BsAssert( m_Pos.x >= 0.f );
	BsAssert( m_Pos.z >= 0.f );

	D3DXVECTOR2 Dir = m_hHero->GetPosV2() - m_PreHeroPos;

	if( D3DXVec2LengthSq( &Dir ) > 500.f * 500.f )
	{
		ResetTroopOffset();
		m_PreHeroPos = m_hHero->GetPosV2();
	}

	return true;
}


bool CFcGuardianTroop::ProcessGuardianAttack()
{
	if( !(m_State.State == TROOPSTATE_MOVE_ATTACK) &&
		(m_State.hTarget == NULL || m_State.hTarget->IsEliminated() ) )
	{
		return false;
	}

	if( m_State.State != TROOPSTATE_MELEE_ATTACK && 
		m_State.State != TROOPSTATE_RANGE_ATTACK &&
		m_State.State != TROOPSTATE_MOVE_ATTACK )
	{
		D3DXVECTOR2 Dir = GetPosV2() - m_hHero->GetPosV2();
		if( D3DXVec2LengthSq( &Dir ) > 5000.f * 5000.f )
		{
			return false;
		}
	}

	return true;
}

bool CFcGuardianTroop::ProcessGuardianDefense()
{
//	SetDefenseFormation( hTarget );
	return true;
}

bool CFcGuardianTroop::ProcessGuardianDefenseHero()
{
	return false;
}

bool CFcGuardianTroop::ProcessGuardianSurroundEnemy()
{
	return true;
}


void CFcGuardianTroop::SetDefenseFormation( TroopObjHandle hTarget )
{
	float PosTable[5] = { 0.f, -500.f, 500.f, -1000.f, 1000.f };
	D3DXVECTOR2 Dir;
	if( hTarget )
	{
		Dir = hTarget->GetPosV2() - GetPosV2();
	}
	else
	{
		Dir = D3DXVECTOR2(m_hHero->GetCrossVector()->m_ZVector.x, m_hHero->GetCrossVector()->m_ZVector.z );
	}
	D3DXVECTOR2 RowDir;
	RowDir.x = Dir.y * 1.f;
	RowDir.y = Dir.x * -1.f;

	int nCnt = m_vecObj.size();
	for( int i=0; i<nCnt; i++ )
	{
		int nColumn = i/5 + 1;
		int nRow = i%5;

		D3DXVECTOR2 TempPos1 = Dir * ( -500.f * (float)nColumn );
		D3DXVECTOR2 TempPos2 = RowDir * PosTable[nRow];
		TempPos1 = TempPos1 + TempPos2;
		m_vecObj[i]->SetTroopOffset( &TempPos1 );
	}
}


// 호위병이 플레이어를 4명, 8명, 16명씩 둘러싼다.

void CFcGuardianTroop::SetDefenseHeroFormation()
{
	int nUnitCnt = m_vecObj.size();

	int nLineNum = 0;
	int nCurCnt = nUnitCnt;
	int i = 1;
	int w = 0;
	while( 1 )
	{
		int nPosNum;
		bool bLast = false;
		if( nCurCnt < (1<<(nLineNum+2) ) )
		{
			bLast = true;
			nPosNum = nCurCnt;
		}
		else
		{
			nPosNum = (1<<(nLineNum+2) );
		}

		D3DXVECTOR2 HeroPos = m_hHero->GetPosV2();

		float fRadDelta = PI*2 / (float)nPosNum;

		D3DXVECTOR2 Dir( 1.f, 0.f );
		Dir = Dir * ((float)i * 500.f);

		for( int j=0; j<nPosNum; j++ )
		{
			D3DXVECTOR2 NewPos;

			float fRad = fRadDelta * (float)j;


			float fSin = sinf( fRad );
			float fCos = cosf( fRad );

			NewPos.x = Dir.x * fCos + Dir.y * fSin;
			NewPos.y = Dir.x * -fSin + Dir.y * fCos;

			// TODO: 맵사이즈 넘었을 때 처리 필요함

			m_vecObj[w]->SetTroopOffset( &NewPos );
			++w;
		}

		if( bLast )
			break;

		nCurCnt -= ( 1<<(nLineNum+2) );
		++nLineNum;
		++i;
	}

	m_Pos = m_hHero->GetPos();
}


void CFcGuardianTroop::ResetTroopOffset()
{
	const float PosTable[5] = { 0.f, -500.f, 500.f, -1000.f, 1000.f };

	D3DXVECTOR2 Dir;
	D3DXVECTOR2 RowDir;
	if( m_pNearCrossProp )
	{
		Dir = m_pNearCrossProp->GetPosV2() - m_CrossStartPos;
		RowDir.x = Dir.y * 1.f;
		RowDir.y = Dir.x * -1.f;
	}
	else
	{
		D3DXVECTOR3 DirV3 = m_hHero->GetCrossVector()->m_ZVector;
		Dir = D3DXVECTOR2( DirV3.x, DirV3.z );
		RowDir.x = Dir.y * 1.f;
		RowDir.y = Dir.x * -1.f;
	}

	D3DXVec2Normalize( &Dir, &Dir );
	D3DXVec2Normalize( &RowDir, &RowDir );

	int nCnt = m_vecObj.size();
	for( int i=0; i<nCnt; i++ )
	{
		int nColumn = i/5 + 1;
		int nRow = i%5;
		D3DXVECTOR2 TempPos1 = Dir * ( -500.f * (float)nColumn );
		D3DXVECTOR2 TempPos2 = RowDir * PosTable[nRow];
		TempPos1 = TempPos1 + TempPos2;
		m_vecObj[i]->SetTroopOffset( &TempPos1 );
	}
}


bool CFcGuardianTroop::DoICrossBridge( CFcProp* pProp )
{
	m_pNearCrossProp = NULL;

	if( pProp->IsGateProp() )
	{
		if( !((CFcGateProp*)pProp)->IsOpened() )
		{
			return false;
		}
		D3DXVECTOR2 Dir = GetPosV2() - pProp->GetPosV2();
		float fDistSq = D3DXVec2LengthSq( &Dir );
		if( fDistSq > 2000.f * 2000.f )
		{
			return false;
		}
	}
	else if( pProp->IsBridgeProp() )
	{
		D3DXVECTOR2 Dir = GetPosV2() - pProp->GetPosV2();
		float fDistSq = D3DXVec2LengthSq( &Dir );

		Dir = m_hHero->GetPosV2() - pProp->GetPosV2();
		float fHeroDistSq = D3DXVec2LengthSq( &Dir );

		if( fDistSq > 5000.f * 5000.f || fHeroDistSq > 5000.f * 5000.f )
		{
			return false;
		}
	}

	D3DXVECTOR2 TargetPos;
	GetCrossStartPos( m_fRadius, pProp, &m_CrossStartPos, &TargetPos, CROSS_START_ADD_POS, &m_fRadius );
	m_pNearCrossProp = pProp;


	if( IsHeroInTargetPos( pProp ) == true )
	{
		return true;
	}

//	if( IsNavInTargetPos( pProp ) == true )
//	{
//		return true;
//	}

	return false;
}


bool CFcGuardianTroop::IsHeroInTargetPos( CFcProp* pProp )
{
	D3DXVECTOR2 Pos, TargetPos;
	GetCrossStartPos( 100.f, pProp, &Pos, &TargetPos, 0.f );

	D3DXVECTOR2 HeroPos = m_hHero->GetPosV2();
	if( HeroPos.x < TargetPos.x + 500.f  && HeroPos.x > TargetPos.x - 500.f &&
		HeroPos.y < TargetPos.y + 500.f  && HeroPos.y > TargetPos.y - 500.f )
	{
		return true;
	}
	return false;
}
*/





/////////////////////////////////////////////////////////////
// CFcPlayerTroop class
CFcPlayerTroop::CFcPlayerTroop()
{
	m_bControl = true;	
}

CFcPlayerTroop::~CFcPlayerTroop()
{

}




void CFcPlayerTroop::Process()
{
	if( !m_hLeader ) return;

	if( CanIProcess() == false )		// 이거 여기 있어두 되나?
		return;

	if( m_bControl )
	{
		m_Pos = m_hLeader->GetPos();
		SetHP();
		if( m_hLeader )
			m_hLeader->Process();
	}
	else
	{
		if( GetProcessTick() % FRAME_PER_SEC == 0 )
		{
			DebugString( "Player Control 0\n" );
		}
		CFcTroopObject::Process();
	}

	m_Dir.x = m_hLeader->GetCrossVector()->m_ZVector.x;
	m_Dir.y = m_hLeader->GetCrossVector()->m_ZVector.z;

	CFcHeroObject* hHero = (CFcHeroObject*)(m_hLeader.m_pInstance);
	m_fMoveSpeed = hHero->GetMoveSpeed();

	BsAssert( m_Pos.x >= 0.f );
	BsAssert( m_Pos.z >= 0.f );


/*
	if( GetProcessTick() % FRAME_PER_SEC == 0  )
		CheckCollisionProps();
*/
}

	
void CFcPlayerTroop::SetControl( bool bEnable )
{
	m_bControl = bEnable;
	SetState( TROOPSTATE_NON );

	m_bTriggerCmd = false;
	m_bUseTriggerCmd = false;
	
	InitNavCell();
}








CFcMoveTower::CFcMoveTower()
{

}


CFcMoveTower::~CFcMoveTower()
{

}


void CFcMoveTower::Process()
{
	if( CanIProcess() == false )
		return;

	bool bMove = false;
	if( GetState() == TROOPSTATE_MOVE )
	{
		bMove = true;
	}
	CFcTroopObject::Process();
	if( bMove && GetState() != TROOPSTATE_MOVE )
		SetState( TROOPSTATE_MELEE_ATTACK );
}

bool CFcMoveTower::ProcessMeleeAttack()
{
	return true;
}





// CFcWallEnemyTroop class
// 특별한 처리가 필요한 카라랑 바르간다 미션에서 세랑 타고 성벽에 올라가는 적 고블린 부대
CFcWallEnemyTroop::CFcWallEnemyTroop()
{
	m_pEscapeTick = NULL;
}

CFcWallEnemyTroop::~CFcWallEnemyTroop()
{
	SAFE_DELETEA( m_pEscapeTick );
}

bool CFcWallEnemyTroop::Initialize( int nX, int nY, int nRadius, int nTeam, int nGroup, 
									int nID, char* pName, int nTroopType, int nCreateCount )
{
	CFcTroopObject::Initialize( nX, nY, nRadius, nTeam, nGroup, nID, pName, nTroopType, nCreateCount );
	m_Type = TROOPTYPE_WALLENEMY;
	for( int i=0; i<m_nUnitNum; i++ )
	{
		m_pUnits[i]->Enable( false );
	}

	if( m_nUnitNum > 0 )
		m_pEscapeTick = new int[m_nUnitNum];

	for( int i=0; i<m_nUnitNum; i++ )
	{
		m_pEscapeTick[i] = i * 120;		// 2초에 한 놈 씩 enble되게
	}

	return true;
}


void CFcWallEnemyTroop::Process()
{
	if( CanIProcess() == false )
		return;

	if( m_bEnable )
	{
		for( int i=0; i<m_nUnitNum; i++ )
		{
			if( m_pEscapeTick[i] > 0 )
			{
				--m_pEscapeTick[i];
				if( m_pEscapeTick[i] == 0 )
				{
					m_pUnits[i]->Enable( true );
				}
			}
		}
	}
}


bool CFcWallEnemyTroop::CmdConquestWall()
{
	m_bEnable = true;
	return true;
}

bool CFcWallEnemyTroop::IsAttackOnWall()
{
	return true;
}


// CFcCatapultTroop class
CFcCatapultTroop::CFcCatapultTroop()
{

}

CFcCatapultTroop::~CFcCatapultTroop()
{


}

bool CFcCatapultTroop::CmdAttack( TroopObjHandle hTarget, bool bBoost, bool bForce )
{
	return CmdRangeAttack( hTarget, bBoost );
}


void CFcCatapultTroop::PostInitialize()
{
	CFcTroopObject::PostInitialize();
	m_fAttackRadius = (float)g_TroopSOX.GetRange( TROOPTYPE_SIEGE, GetLevel() );
}

void CFcCatapultTroop::Process()
{
	/*
	if( m_pUnits[0] && m_pUnits[0]->IsDie() )
	{
		for( int i=1; i<m_nUnitNum; i++ )
		{
			if( m_pUnits[i] == NULL )
				continue;

			if( m_pUnits[i]->IsDie() )
				continue;

			m_pUnits[i]->SetHP( 0 );
		}
	}
	*/
	CFcTroopObject::Process();
}





CFcCitizenTroop::CFcCitizenTroop()
{

}

CFcCitizenTroop::~CFcCitizenTroop()
{

}


bool CFcCitizenTroop::CmdAttack( TroopObjHandle hTarget, bool bBoost, bool bForce )
{
	return false;
}






CFcWallDummyTroop::CFcWallDummyTroop()
{
	m_pParentProp = NULL;
}

CFcWallDummyTroop::~CFcWallDummyTroop()
{

}

bool CFcWallDummyTroop::CmdAttack( TroopObjHandle hTarget, bool bBoost, bool bForce )
{
	return false;
}


bool CFcWallDummyTroop::Initialize( CFcProp* pProp )
{
	m_Type = TROOPTYPE_WALL_DUMMY;
	m_Pos = pProp->GetPos();

	SetRadius( 100.f );			// 부대 체크 범위 하드코딩
	BsAssert( pProp->IsCrushProp() );
	m_pParentProp = (CFcBreakableProp*)pProp;
	m_bEnable = true;

	strcpy( m_cName, pProp->GetName() );

	m_nHP = pProp->GetParam( 0 ); 

	m_nForce = pProp->GetParam( 1 );
	m_nTeam = pProp->GetParam( 2 ); 

	m_nMaxHP = m_nHP;

	m_pParentProp->SetHP( m_nHP );
	m_pParentProp->SetMaxHP( m_nMaxHP );

	return true;
}


void CFcWallDummyTroop::AddHP( int nValue )
{
	if( m_nHP <= 0 ) return;
	m_nHP += nValue;
	m_pParentProp->AddHP( nValue );		// 게이지 표시를 위해 세팅
	if( m_nHP <= 0 ) {
		m_nHP = 0;
		SetState( TROOPSTATE_ELIMINATED );

		if( m_pParentProp )
			m_pParentProp->Break();
	}
}

int CFcWallDummyTroop::GetBossHP()
{
	return m_pParentProp->GetHP();
}

int CFcWallDummyTroop::GetBossMaxHP()
{
	return m_pParentProp->GetMaxHP();
}


void CFcWallDummyTroop::PostInitialize()
{
}

void CFcWallDummyTroop::Process()
{
	// HP 가 0 일 경우에 무너지게 해주3
	m_nHP = m_pParentProp->GetHP();
}