#include "stdafx.h"
#include "FcTroopObject.h"
#include "FcTroopAIObject.h"
#include "FcTroopAITrigger.h"
#include "Parser.h"
#include "Token.h"
#include "FcGlobal.h"
#include "FcWorld.h"
#include "BSFileManager.h"
#include "./data/TroopAIDef.h"
#include "DebugUtil.h"
#include "FcTroopManager.h"
#include "FcHeroObject.h"
#include "BsFileManager.h"
#include "FcProp.h"

#include <io.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

CSmartPtrMng<CFcTroopAIObject> CFcTroopAIObject::s_ObjectMng(DEFAULT_TROOP_AI_OBJECT_POOL_SIZE);
std::vector<TROOPAIGROUPDATA*> CFcTroopAIObject::s_VecGroupData;
int CFcTroopAIObject::s_PathID[MAX_AI_PATH_ID];
AIGUIDE_DATA CFcTroopAIObject::s_AIGuideData[MAX_AI_GUIDE_DATA];

#define TROOP_AI_CHECK_TICK	60

enum PATH_TYPE
{
	PATH_TYPE_NON = -1,
	PATH_TYPE_RANDOM,
	PATH_TYPE_LOOP,
	PATH_TYPE_MOVE
};


CFcTroopAIObject::CFcTroopAIObject()
{
	m_pGroupData = NULL;
	m_nAIID = -1;
	m_bEnable = true;
	m_cCurLabel[0] = NULL;
	for( int i=0; i<MAX_AI_PATH_ID; i++ )
	{
		s_PathID[i] = -1;
	}
	m_PreHeroPos = D3DXVECTOR2( 0.f, 0.f );		// use follow
	m_nLastPath1 = m_nLastPath2 = m_nLastPathType = m_nLastPathDir = -1;
}


CFcTroopAIObject::~CFcTroopAIObject()
{
	s_ObjectMng.DeleteHandle( m_Handle.GetHandle() );
}


bool CFcTroopAIObject::InitLoadScript( const char *szFileName )
{
	char cBinFileName[MAX_PATH];
	char cScriptFileName[MAX_PATH];

	sprintf( cBinFileName, "%sAI\\Troop\\%sb", g_BsKernel.GetCurrentDirectory(), szFileName );
	sprintf( cScriptFileName, "%sAI\\Troop\\%s", g_BsKernel.GetCurrentDirectory(), szFileName );

DebugString( "Load TroopAI %s\n", szFileName );

	TokenBuffer tb;
//	int nWriteTime = -1;

//	nWriteTime = GetFileTime( cScriptFileName);

	if( tb.Load( cBinFileName ) == false )
	{
		DebugString( "Troop AI load fail. %s\n", cBinFileName );
		BsAssert( 0 && "Troop AI load fail." );
		return false;
	}
	TROOPAIGROUPDATA* pGroupData = NULL;

	while( 1 )
	{
		if( tb.IsEnd() )
			break;

		while( 1 )		// 파서가 헤더 분석을 잘못해서 #이 들어가는 경우가 있음 -> #define ConvertToString(x) #x
		{
			if( tb.IsKeyword() && strcmp( tb.GetKeyword(), "define" ) != 0 )
			{
				break;
			}
			++tb;
		}

		if( !tb.IsKeyword() )
		{
			BsAssert( 0 && "Syntax error" );
		}

		if( strcmp( tb.GetKeyword(), "AIDef" ) == 0 )		// 끝
		{
			pGroupData = new TROOPAIGROUPDATA;

			tb++;
			if( strcmp( tb.GetKeyword(), "Id" ) == 0 )
			{
				tb++;
				pGroupData->nID = tb.GetInteger();
			}
			else		{ BsAssert(0); }

			tb++;
			if( strcmp( tb.GetKeyword(), "Name" ) == 0 )
			{
				tb++;
				BsAssert( strlen( tb.GetString() ) < 32 );
				strcpy( pGroupData->cName, tb.GetString() );
			}
			else		{ BsAssert(0); }

			tb++;

			TROOPAITRIGGERDATA* Data;

			while( 1 )
			{
				if( tb.IsKeyword() && strcmp( tb.GetKeyword(), "AIDefEnd" ) == 0 )
				{
					break;
				}

				if( tb.IsInteger() && tb.GetInteger() <= MAX_TROOP_AI_ID )		// 마이너스 값은 IF, CMD밖에 없다
				{
					Data = new TROOPAITRIGGERDATA;

					int nTypeID = tb.GetInteger();
					if( nTypeID >= TAI_IF_START && nTypeID <= TAI_IF_LAST )		// IF
					{
						Data->Type = TRIGGER_TYPE_IF;
					}
					else if( nTypeID >= TAI_TYPE_START && nTypeID <= TAI_TYPE_LAST )		// CMD Type
					{
						Data->Type = TRIGGER_TYPE_CMD;
					}
					else
					{
						BsAssert( 0 );
					}

//					tb++;

					Data->nID = tb.GetInteger();

					tb++;

					bool bCheckFilter = false;
 					int nIndex = 0;
					while( !(tb.IsInteger() && tb.GetInteger() <= MAX_TROOP_AI_ID) && !tb.IsKeyword() )
					{
						// Label 체크
						if( tb.IsVariable() )
						{
							tb++;
							if( tb.IsOperator() && strcmp( tb.GetOperator(), ":" ) == 0 )
							{
								tb--;
								break;
							}
							tb--;
						}

						if( bCheckFilter )
						{
							if(	!tb.IsInteger() )
							{
#ifdef TOKEN_DEBUGGING_INFO
								DebugString( "Invalid AI Filter Line %d\n", tb.GetLineNumber() );
#else
								DebugString( "Invalid AI Filter\n" );
#endif
							}
							bCheckFilter = false;
						}
						if( tb.IsInteger() )			{ Data->Variable[nIndex].SetVariable( tb.GetInteger() ); }
						else if( tb.IsReal() )		{ Data->Variable[nIndex].SetVariable( tb.GetReal() ); }
						else if( tb.IsString() )		{ Data->Variable[nIndex].SetVariable( (char *)tb.GetString() ); }
						else if( tb.IsOperator() )
						{
							Data->Variable[nIndex].SetVariable( (char *)tb.GetOperator() );
							if( strcmp( "+", tb.GetOperator() ) == 0 || strcmp( "-", tb.GetOperator() ) == 0 )
							{
								bCheckFilter = true;
							}
						}
						else if( tb.IsVariable() )
						{
							Data->Variable[nIndex].SetVariable( (char *)tb.GetVariable() );
						}
						else if( tb.IsBoolean() )		{ Data->Variable[nIndex].SetVariable( tb.GetBoolean() ); }
						else							{ BsAssert(0); }
						nIndex++;
						tb++;
					}
				}
				else if( tb.IsVariable() )
				{
					Data = new TROOPAITRIGGERDATA;

					Data->Type = TRIGGER_TYPE_LABEL;
					Data->Variable[0].SetVariable( (char *)tb.GetVariable() );
				
					tb++;
					BsAssert( tb.IsOperator() );
					tb++;
				}
				else
				{
					BsAssert( 0 );
				}
				pGroupData->VecTrigger.push_back( Data );
			}
			s_VecGroupData.push_back( pGroupData );
			tb++;
		}
	}
	return true;
}




/*

bool CFcTroopAIObject::InitLoadScript( const char *szFileName )
{
	Parser parser;
	TokenList toklist;

	parser.Create();
	parser.EnableComments( true );

	parser.ReserveKeyword("AI_DEF");
	parser.ReserveKeyword("ID");
	parser.ReserveKeyword("NAME");
	parser.ReserveKeyword("AI_SET");
	parser.ReserveKeyword("AI_CON");
	parser.ReserveKeyword("AI_CMD");
	parser.ReserveKeyword("AI_DEF_END");

	bool result;

	char cDir[MAX_PATH];
	sprintf( cDir, "%sAI\\Troop\\",  g_BsKernel.GetCurrentDirectory() );

	result = parser.ProcessSource( cDir, szFileName, &toklist );
	if( result == false )
	{
		BsAssert( 0 );
		return false;
	}

	parser.SetDirectory( g_BsKernel.GetCurrentDirectory() );		// \data
	result = parser.ProcessHeaders( &toklist );
	if( result == false )
	{
		BsAssert( 0 );
		return false;
	}
	parser.SetDirectory( cDir );

	parser.ProcessMacros( &toklist );
	TokenList::iterator itr = toklist.begin();

	TROOPAIGROUPDATA* pGroupData = NULL;

	while( 1 )
	{
		if( itr == toklist.end() )
			break;

		if( !itr->IsKeyword() )
		{
			BsAssert( 0 && "Syntax error" );
		}

		if( strcmp( itr->GetKeyword(), "AI_DEF" ) == 0 )		// 끝
		{
			pGroupData = new TROOPAIGROUPDATA;

			itr++;
			if( strcmp( itr->GetKeyword(), "ID" ) == 0 )
			{
				itr++;
				pGroupData->nID = itr->GetInteger();
			}
			else		{ BsAssert(0); }

			itr++;
			if( strcmp( itr->GetKeyword(), "NAME" ) == 0 )
			{
				itr++;
				BsAssert( strlen( itr->GetString() ) < 32 );
				strcpy( pGroupData->cName, itr->GetString() );
			}
			else		{ BsAssert(0); }

			itr++;
			while( strcmp( itr->GetKeyword(), "AI_SET" ) == 0 )
			{
				itr++;

				TROOPAIELEMENTDATA Data;
				Data.nID = itr->GetInteger();

				itr++;

				int nIndex = 0;
				while( !itr->IsKeyword() )
				{
					if( itr->IsInteger() )		{ Data.Variable[nIndex].SetVariable( itr->GetInteger() ); }
					else if( itr->IsReal() )	{ Data.Variable[nIndex].SetVariable( itr->GetReal() ); }
					else if( itr->IsString() )	{ Data.Variable[nIndex].SetVariable( (char *)itr->GetString() ); }
					else						{ BsAssert(0); }
					nIndex++;
					itr++;
				}

				pGroupData->VecSet.push_back( Data );
			}

			while( strcmp( itr->GetKeyword(), "AI_DEF_END" ) != 0 )
			{
				TROOPAITRIGGERDATA Data;
				if( strcmp( itr->GetKeyword(), "AI_CON" ) == 0 )
				{
					itr++;
					Data.Con.nID = itr->GetInteger();
					itr++;
					int nIndex = 0;
					while( !itr->IsKeyword() )
					{
						if( itr->IsInteger() )		{ Data.Con.Variable[nIndex].SetVariable( itr->GetInteger() ); }
						else if( itr->IsReal() )	{ Data.Con.Variable[nIndex].SetVariable( itr->GetReal() ); }
						else if( itr->IsString() )	{ Data.Con.Variable[nIndex].SetVariable( (char *)itr->GetString() ); }
						else						{ BsAssert(0); }
						nIndex++;
						itr++;
					}
				}
				if( strcmp( itr->GetKeyword(), "AI_CMD" ) == 0 )
				{
					itr++;
					Data.Cmd.nID = itr->GetInteger();
					itr++;
					int nIndex = 0;
					while( !itr->IsKeyword() )
					{
						if( itr->IsInteger() )		{ Data.Cmd.Variable[nIndex].SetVariable( itr->GetInteger() ); }
						else if( itr->IsReal() )	{ Data.Cmd.Variable[nIndex].SetVariable( itr->GetReal() ); }
						else if( itr->IsString() )	{ Data.Cmd.Variable[nIndex].SetVariable( (char *)itr->GetString() ); }
						else						{ BsAssert(0); }
						nIndex++;
						itr++;
					}
				}
				else
				{
					BsAssert(0);
				}
				pGroupData->VecTrigger.push_back( Data );
			}
			s_VecGroupData.push_back( pGroupData );
			itr++;
		}
	}
	return true;
}
*/

// Static
void CFcTroopAIObject::ProcessObjects()
{
	int nSize = CFcTroopAIObject::s_ObjectMng.Size();
	for( int i=0; i<nSize; i++ )
	{
		if( CFcTroopAIObject::s_ObjectMng[i] )
		{
			BsAssert( CFcTroopAIObject::s_ObjectMng[i]->IsParentTroop() );
			CFcTroopAIObject::s_ObjectMng[i]->Process();
		}
	}
}



// Static
void CFcTroopAIObject::ReleaseObjects()
{
	int nSize = CFcTroopAIObject::s_ObjectMng.Size();
	for( int i=0; i<nSize; i++ )
	{
		if( CFcTroopAIObject::s_ObjectMng[i] )
		{
			delete CFcTroopAIObject::s_ObjectMng[ i ];
		}
	}

	nSize = s_VecGroupData.size();
	for( int i=0; i<nSize; i++ )
	{
		delete s_VecGroupData[i];
	}
	s_VecGroupData.clear();
}

// Static
void CFcTroopAIObject::SetAI( TroopObjHandle hTroop, int nAIID )
{
	int nSize = CFcTroopAIObject::s_ObjectMng.Size();
	for( int i=0; i<nSize; i++ )
	{
		CFcTroopAIObject* pAIObj = CFcTroopAIObject::s_ObjectMng[i];
		if( pAIObj == NULL )
			continue;

		if( pAIObj->GetParentTroop() != hTroop )
			continue;

		pAIObj->Initialize( nAIID, hTroop );
		break;
	}
}


void CFcTroopAIObject::SetAIPath( int nPathID, int nAIPathID )
{
	BsAssert( nAIPathID >= 0 && nAIPathID < MAX_AI_PATH_ID );
	s_PathID[nAIPathID] = nPathID;
}


void CFcTroopAIObject::SetEnableAI( TroopObjHandle hTroop, bool bEnable )
{
	int nSize = CFcTroopAIObject::s_ObjectMng.Size();
	for( int i=0; i<nSize; i++ )
	{
		if( CFcTroopAIObject::s_ObjectMng[i] == NULL )
			continue;

		if( CFcTroopAIObject::s_ObjectMng[i]->GetParentTroop() != hTroop )
			continue;

		CFcTroopAIObject::s_ObjectMng[i]->SetEnable( bEnable );
	}
}

bool CFcTroopAIObject::IsEnableAI( TroopObjHandle hTroop )
{
	int nSize = CFcTroopAIObject::s_ObjectMng.Size();
	for( int i=0; i<nSize; i++ )
	{
		if( CFcTroopAIObject::s_ObjectMng[i] == NULL )
			continue;

		if( CFcTroopAIObject::s_ObjectMng[i]->GetParentTroop() != hTroop )
			continue;

		return CFcTroopAIObject::s_ObjectMng[i]->IsEnable();
	}
	return false;
}

int CFcTroopAIObject::GetAI( TroopObjHandle hTroop )
{
	int nSize = CFcTroopAIObject::s_ObjectMng.Size();
	for( int i=0; i<nSize; i++ )
	{
		CFcTroopAIObject* pAIObj = CFcTroopAIObject::s_ObjectMng[i];
		if( pAIObj == NULL )
			continue;

		if( pAIObj->GetParentTroop() == hTroop )
			return pAIObj->GetAIID();
	}
	return -1;
}

void CFcTroopAIObject::SetAIGuide( int nIndex, CFcProp* pProp, AREA_INFO* pAreaInfo )
{
	BsAssert( nIndex >= 0 && nIndex < MAX_AI_GUIDE_DATA );
	s_AIGuideData[nIndex].pProp = pProp;
	s_AIGuideData[nIndex].pArea = pAreaInfo;
}


bool CFcTroopAIObject::Initialize(int nAIID, TroopObjHandle hTroopObject )
{
	// InitLoadScript()가 Initialize보다 먼저 실행되야 한다.
	m_nAIID = nAIID;
	m_hTroop = hTroopObject;		// nAIID가 -1이라도 넣어줘야 한다
	m_pGroupData = NULL;
	if( m_nAIID == -1 )
	{
		return true;
	}

/*
	if( m_nAIID == g_FcWorld.GetTroopmanager()->GetGuardianAttackAI() )
	{
		m_hGuardianTarget.Identity();
	}
*/

	int nCnt = s_VecGroupData.size();
	BsAssert( nCnt > 0 && "AI로딩 안됐음 InitLoadScript()" );
	BsAssert( m_pGroupData == NULL );

	for( int i=0; i<nCnt; i++ )
	{
		if(s_VecGroupData[i]->nID == nAIID )
		{
			m_pGroupData = s_VecGroupData[i];
			break;
		}
	}
	if( m_pGroupData == NULL )
	{
		DebugString( "Cannot found AI ID %d\n", nAIID );
		BsAssert( 0 && "Cannot found TROOP AI ID" );
	}

	if( m_hTroop->IsEnable() )
		m_bEnable = true;

	// AI_SET 여기서 세팅
	return true;
}

void CFcTroopAIObject::Process()
{
	if( m_bEnable == false )
		return;

	if( m_pGroupData == NULL )
		return;

	if( GetProcessTick() % TROOP_AI_CHECK_TICK != 0 )
		return;

	bool bEnableCmd = false;
	bool bJumpLabel = false;
	char cCurLabel[64] = "";
	int nCnt = m_pGroupData->VecTrigger.size();
	for( int i=0; i<nCnt; i++ )
	{
		TROOPAITRIGGERDATA* Data = m_pGroupData->VecTrigger[i];

		if( bJumpLabel == true )
		{
			if( Data->Type == TRIGGER_TYPE_LABEL )
			{
				if( strcmp( Data->Variable[0].GetVariableString(), cCurLabel ) == 0 )
				{
					bJumpLabel = false;
					continue;
				}
			}
			else		// 다 무시
			{
				continue;
			}
		}

		if( Data->Type == TRIGGER_TYPE_IF )
		{
			if( CheckIf( Data ) == true )
			{
				bJumpLabel = true;
				strcpy( cCurLabel, m_cCurLabel );
				continue;
			}
		}
		else if( Data->Type == TRIGGER_TYPE_CMD )
		{
			if( CheckCmd( Data ) == true )
			{
				bEnableCmd = true;
				break;
			}
		}
	}

	if( bEnableCmd == false )
	{
		m_hTroop->CmdStop();		// 아무 명령도 안하면 멈춰버린다.
	}
	BsAssert( bJumpLabel == false && "Cannot found Label" );
	if( bJumpLabel == true )
	{
		DebugString( "Cannot found Label. TroopID : %d, Label : %s\n", m_nAIID, cCurLabel );
	}
}




void CFcTroopAIObject::operator = ( const CFcTroopAIObject &Obj )
{
	// 복사 연산자 관련 나중 수정 사항들
	//
	// 내부적으로 따로 사용하는 맴버가 없는 클래스들 ( CFcAIConditionBase, CFcAIActionBase, CFcAITriggerParam )은
	// 솔직히 new로 크레딧 하여 오브젝트 마다 가지고 있을 필요가 없다.
	// 나중에 속도나 메모리 관련하여 많이 먹는다고 했을때는 ( 많이 먹진 않치만.. 갯수가 많이질 경우 어케될지 모름 )
	// 레퍼런스 카운터를 두구 관리하는(COM) 오브젝트로 바꿔주삼~
/*
	for( DWORD i=0; i<Obj.m_pVecElement.size(); i++ ) {
		CFcAIElement *pElement = new CFcAIElement( m_Handle );
		*pElement = *Obj.m_pVecElement[i];
		m_pVecElement.push_back( pElement );
	}
*/
}


bool CFcTroopAIObject::CheckCmd( TROOPAITRIGGERDATA* pData )
{
	switch( pData->nID )
	{
	case Engage:
		return CmdEngage( pData );
	case EngageMercifully:
		return CmdEngageMercifully( pData );
	case EngageSmart:
		return CmdEngageSmart( pData );
	case EngageStanding:
		return CmdEngageStanding( pData );
	case EngageStandingMercifully:
		return CmdEngageStandingMercifully( pData );
	case Disengage:
		return CmdDisengage( pData );
	case Defense:
		return CmdDefense( pData );
	case DefenseProp:
		return CmdDefenseProp( pData );
	case UseAbility:
		return CmdUseAbility( pData );
	case MoveOnPath:
		return CmdMoveOnPath( pData );
	case MoveOnPathLoop:
		return CmdMoveOnPathLoop( pData );
	case MoveOnPathRandom:
		return CmdMoveOnPathRandom( pData );
	case MoveOnPathWithAvoidEnemy:
		return CmdMoveOnPathWithAvoidEnemy( pData );
	case MoveOnFullPath:
		return CmdMoveOnFullPath( pData );
	case AttackEnemyNearSomebody:
		return CmdAttackEnemyNearSomebody( pData );
	case AttackWall:
		return CmdAttackWall( pData );
	case SetTargetID:
		return CmdSetTargetID( pData);
	case Stop:
		return true;
	case Follow:
		return CmdFollow( pData );
	case EngageGuardianToTarget:
		return CmdEngageGuardianToTarget( pData );
	case EngageArea:
		return CmdEngageArea( pData );
	case DisengageOnAttacked:
		return CmdDisengageOnAttacked( pData );
	case EngageMercifullyArea:
		return CmdEngageMercifullyArea( pData );
	default:
		BsAssert(0);
	}
	return false;
}


bool CFcTroopAIObject::CheckIf( TROOPAITRIGGERDATA* pData )
{
	switch( pData->nID )
	{
	case IfHP:
		return _IfHP( pData );
	case IfMeleeEngaged:
		return _IfMeleeEngaged( pData );
	case IfEngaged:
		return _IfEngaged( pData );
	case IfNumFriendlyTroop:
		return _IfNumFriedlyTroop( pData );
	case IfNumEnemyTroop:
		return _IfNumEnemyTroop( pData );
	case IfThereIsTroop:
		return _IfThereIsTroop( pData );
	case IfIAm:
		return _IfIAm( pData );
	default:
		BsAssert( 0 );
	}
	return false;
}

bool CFcTroopAIObject::CmdEngage( TROOPAITRIGGERDATA* pData )
{
	FILTER_DATA FilterData[16];
	FILTER_DATA* pTemp = FilterData;
	int nFilterCount = SetFilterData( &(pData->Variable[0]), &pTemp);
	int nIndex = nFilterCount * 2;

	int nRange = pData->Variable[nIndex].GetVariableInt();

	D3DXVECTOR2 vPos = m_hTroop->GetPosV2();

	if( FilterData[0].nType >= AIGuide0 && FilterData[0].nType <= AIGuide15 )
	{
		return MoveByAIGuide( &vPos, nRange, FilterData, nFilterCount );
	}
	else
	{
		g_FcWorld.GetTroopListInRange( &vPos, static_cast<float>( nRange ), m_VecTargetObjects );
		TroopObjHandle hTarget = CheckTargetTroop( &vPos, nRange, FilterData, nFilterCount, m_VecTargetObjects );
		if( hTarget )
		{
			// ON_PROP 강제로 무시
			if( hTarget->GetType() == TROOPTYPE_ON_PROP )
				return false;

			m_hTroop->CmdAttack( hTarget );
			return true;
		}
	}
	return false;
}

bool CFcTroopAIObject::CmdEngageMercifully( TROOPAITRIGGERDATA* pData )
{
	FILTER_DATA FilterData[16];
	FILTER_DATA* pTemp = FilterData;
	int nFilterCount = SetFilterData( &(pData->Variable[0]), &pTemp);
	int nIndex = nFilterCount * 2;

	int nMaxEngage = pData->Variable[nIndex].GetVariableInt();
	int nRange = pData->Variable[nIndex + 1].GetVariableInt();

	D3DXVECTOR2 vPos = m_hTroop->GetPosV2();
	g_FcWorld.GetEnemyTroopListInRange( &vPos, m_hTroop->GetTeam(), static_cast<float>( nRange ), m_VecTargetObjects );
	int nCnt = m_VecTargetObjects.size();
	if( nCnt <= 0 )
		return false;

	TroopObjHandle hTarget = CheckTargetTroop( &vPos, nRange, FilterData, nFilterCount, m_VecTargetObjects, nMaxEngage );
	if( hTarget == NULL )
	{
		// 목표부대 없음
		return false;
	}

	// ON_PROP 강제로 무시
	if( hTarget->GetType() == TROOPTYPE_ON_PROP )
		return false;

	std::vector<TroopObjHandle> vecVoidObjects;
	nCnt = m_VecTargetObjects.size();
	for( int i = 0; i < nCnt; i ++ )
	{
		TroopObjHandle hTroop = m_VecTargetObjects[i];
		//if( m_hTroop->GetTeam() != hTroop->GetTeam() ) 위에서 해서 체크할 필요 없음

		if( hTroop->GetType() == TROOPTYPE_ON_PROP || hTroop->GetType() == TROOPTYPE_FLYING )
			continue;

		if( hTarget != hTroop )
		{
			D3DXVECTOR2 Dir = hTarget->GetPosV2() - hTroop->GetPosV2();
			if( D3DXVec2LengthSq( &Dir ) > 1000.f * 1000.f )
			{
				vecVoidObjects.push_back( hTroop );
			}
		}
	}
	m_hTroop->CmdEngageMercifully( hTarget, vecVoidObjects );

	return true;
}

bool CFcTroopAIObject:: CmdEngageSmart( TROOPAITRIGGERDATA* pData )
{
	FILTER_DATA TargetFilterData[16];
	FILTER_DATA* pTemp = TargetFilterData;
	int nTargetFilterCount = SetFilterData( &(pData->Variable[0]), &pTemp);
	int nIndex = nTargetFilterCount * 2;

	int nCheckRange = pData->Variable[nIndex].GetVariableInt();
	nIndex ++;

	FILTER_DATA VoidFilterData[16];
	pTemp = VoidFilterData;
	int nVoidFilterCount = SetFilterData( &(pData->Variable[nIndex]), &pTemp);
	nIndex += ( nVoidFilterCount * 2 );

	int nVoidRange = pData->Variable[nIndex].GetVariableInt();

	D3DXVECTOR2 vPos = m_hTroop->GetPosV2();

	g_FcWorld.GetEnemyTroopListInRange( &vPos, m_hTroop->GetTeam(), static_cast<float>( nCheckRange ), m_VecTargetObjects );

	std::vector<TroopObjHandle> vecVoidObjects;
	int nCnt = m_VecTargetObjects.size();
	for( int i = 0; i < nCnt; i ++ )
	{
		TroopObjHandle hTroop = m_VecTargetObjects[i];
		vecVoidObjects.push_back( hTroop );
	}

	TroopObjHandle hTarget = CheckTargetTroop( &vPos, nCheckRange, TargetFilterData, nTargetFilterCount, m_VecTargetObjects );
	if( hTarget == NULL )
	{
		// 목표부대 없음
		return false;
	}

	CheckTargetTroop( &vPos, nCheckRange, VoidFilterData, nVoidFilterCount, vecVoidObjects );
	// TODO : 피할 거리 처리해야 한다
	m_hTroop->CmdEngageMercifully( hTarget, vecVoidObjects );
	return true;
}

bool CFcTroopAIObject::CmdEngageStanding( TROOPAITRIGGERDATA* pData )
{
	FILTER_DATA FilterData[16];
	FILTER_DATA* pTemp = FilterData;
	int nFilterCount = SetFilterData( &(pData->Variable[0]), &pTemp);
	int nIndex = nFilterCount * 2;

	int nRange = pData->Variable[nIndex].GetVariableInt();

	D3DXVECTOR2 vPos = m_hTroop->GetPosV2();

	g_FcWorld.GetTroopListInRange( &vPos, static_cast<float>( nRange ), m_VecTargetObjects );

	TroopObjHandle hTarget = CheckTargetTroop( &vPos, nRange, FilterData, nFilterCount, m_VecTargetObjects );
	if( hTarget )
	{
		m_hTroop->CmdHold( hTarget );
		return true;
	}
	return false;
}

bool CFcTroopAIObject::CmdEngageStandingMercifully( TROOPAITRIGGERDATA* pData )
{
	FILTER_DATA FilterData[16];
	FILTER_DATA* pTemp = FilterData;
	int nFilterCount = SetFilterData( &(pData->Variable[0]), &pTemp);
	int nIndex = nFilterCount * 2;

	int nMaxEngage = pData->Variable[nIndex].GetVariableInt();

	int nRange = pData->Variable[nIndex + 1].GetVariableInt();

	D3DXVECTOR2 vPos = m_hTroop->GetPosV2();

	g_FcWorld.GetEnemyTroopListInRange( &vPos, m_hTroop->GetTeam(), static_cast<float>( nRange ), m_VecTargetObjects );
	int nCnt = m_VecTargetObjects.size();
	if( nCnt <= 0 )
		return false;

	TroopObjHandle hTarget = CheckTargetTroop( &vPos, nRange, FilterData, nFilterCount, m_VecTargetObjects, nMaxEngage );
	if( hTarget == NULL )
	{
		// 목표부대 없음
		return false;
	}

	std::vector<TroopObjHandle> vecVoidObjects;

	nCnt = m_VecTargetObjects.size();
	for( int i = 0; i < nCnt; i ++ )
	{
		TroopObjHandle hTroop = m_VecTargetObjects[i];
		{
			if( hTarget != hTroop )
			{
				D3DXVECTOR2 Dir = hTarget->GetPosV2() - hTroop->GetPosV2();
				if( D3DXVec2LengthSq( &Dir ) > 1000.f * 1000.f )
				{
					vecVoidObjects.push_back( hTroop );
				}
			}
		}
	}
	m_hTroop->CmdEngageMercifully( hTarget, vecVoidObjects );

	return true;
}

bool CFcTroopAIObject:: CmdDisengage( TROOPAITRIGGERDATA* pData )
{
	FILTER_DATA FilterData[16];
	FILTER_DATA* pTemp = FilterData;
	int nFilterCount = SetFilterData( &(pData->Variable[0]), &pTemp);
	int nIndex = nFilterCount*2;

	int nRange = pData->Variable[nIndex].GetVariableInt();

	D3DXVECTOR2 vPos = m_hTroop->GetPosV2();

	g_FcWorld.GetTroopListInRange( &vPos, static_cast<float>( nRange ), m_VecTargetObjects );

	std::vector<TroopObjHandle>::iterator it = m_VecTargetObjects.begin();

	TroopObjHandle hTarget = CheckTargetTroop( &vPos, nRange, FilterData, nFilterCount, m_VecTargetObjects );
	if( hTarget == NULL )
		return false;

//	BsAssert( hTarget );	// mruete: prefix bug 616: added assert

	int nCnt = m_VecTargetObjects.size();
	if( nCnt > 0 )
	{
		D3DXVECTOR2 Dir( 0.f, 0.f );
		for( int i = 0; i < nCnt; i ++ )
		{
			Dir = Dir + ( m_hTroop->GetPosV2() - hTarget->GetPosV2() );
		}
		D3DXVec2Normalize( &Dir, &Dir );
		D3DXVec2Scale( &Dir, &Dir, static_cast<float>( nRange ) );
		m_hTroop->CmdRetreat( m_hTroop->GetPos().x + Dir.x, m_hTroop->GetPos().z + Dir.y, true );
		return true;
	}
	return false;
}

bool CFcTroopAIObject:: CmdDefense( TROOPAITRIGGERDATA* pData )
{
//	[지킬대상filter] [거리min] [거리max] [공격대상filter] [시야거리]

	FILTER_DATA DefenseFilterData[16];
	FILTER_DATA* pTemp = DefenseFilterData;
	int nDefenseFilterCount = SetFilterData( &(pData->Variable[0]), &pTemp );
	int nIndex = nDefenseFilterCount * 2;

	int nMinRange = pData->Variable[nIndex].GetVariableInt();
	nIndex ++;

	int nMaxRange = pData->Variable[nIndex].GetVariableInt();
	nIndex ++;

	FILTER_DATA AttackFilterData[16];
	pTemp = AttackFilterData;
	int nAttackFilterCount = SetFilterData( &(pData->Variable[nIndex]), &pTemp);

	nIndex += ( nAttackFilterCount * 2 );
	int nSightRange = pData->Variable[nIndex].GetVariableInt();

	D3DXVECTOR2 vPos = m_hTroop->GetPosV2();

	std::vector<TroopObjHandle> vecObjects;

	TroopObjHandle hTroop;
	int nCnt = g_FcWorld.GetTroopCount();
	for( int i = 0; i < nCnt; i ++ )
	{
		vecObjects.push_back( g_FcWorld.GetTroopObject( i ) );
	}

	TroopObjHandle hDefenseObject = CheckTargetTroop( &vPos, 0, DefenseFilterData, nDefenseFilterCount, vecObjects );
	// 전 맵을 다 체크하네... 느리면 어떻하나

	if( hDefenseObject == NULL)
		// 지킬 부대 없음 --> 자기자신
		hDefenseObject = m_hTroop;

	D3DXVECTOR2 vDefensePos = hDefenseObject->GetPosV2();
	D3DXVECTOR2 Dir;

	D3DXVec2Subtract( &Dir, &vPos, &vDefensePos );
	int nDist = static_cast<int>( D3DXVec2Length( &Dir ) );

	if( nDist >= nMaxRange )		// 너무 멀리 있다면 nMinRange까지 돌아온다.
	{
		D3DXVec2Normalize( &Dir, &Dir );
		D3DXVec2Scale( &Dir, &Dir, static_cast<float>( nMinRange ) );

		m_hTroop->CmdMove( Dir.x + vDefensePos.x, Dir.y + vDefensePos.y, 1.f );
	}
	else
	{
		g_FcWorld.GetTroopListInRange( &vPos, static_cast<float>( nSightRange ), m_VecTargetObjects );

		TroopObjHandle hTarget = CheckTargetTroop( &vPos, nSightRange, AttackFilterData, nAttackFilterCount, m_VecTargetObjects );
		if( hTarget )
		{
			D3DXVECTOR2 vAttackPos = hTarget->GetPosV2();
			D3DXVec2Subtract( &Dir, &vAttackPos, &vDefensePos );

			int nDistA2D = static_cast<int>( D3DXVec2Length( &Dir ) );

			if( nDistA2D < nMaxRange )
			{
				m_hTroop->CmdAttack( hTarget );
				return true;
			}
		}
	}
	return false;
}

bool CFcTroopAIObject:: CmdDefenseProp( TROOPAITRIGGERDATA* pData )
{
	BsAssert( 0 );
	return false;
}

bool CFcTroopAIObject:: CmdUseAbility( TROOPAITRIGGERDATA* pData )
{
	BsAssert( 0 );
	return false;
}

// [walk/run]	[forward/reverse]	[Path번호1]	[Path번호2]
bool CFcTroopAIObject:: CmdMoveOnPath( TROOPAITRIGGERDATA* pData )
{
	int nWalkType = pData->Variable[0].GetVariableInt();
	int nMoveDirType = pData->Variable[1].GetVariableInt();
	int nPath1 = pData->Variable[2].GetVariableInt();
	int nPath2 = pData->Variable[3].GetVariableInt();

	if( IsOnPathState() )
	{
		if( m_nLastPath1 == nPath1 && 
			m_nLastPath2 == nPath2 &&
			m_nLastPathDir == nMoveDirType && 
			m_nLastPathType == PATH_TYPE_MOVE )
		{
			return true;
		}
	}

	BsAssert( nMoveDirType ==Forward || nMoveDirType == Backward );

	D3DXVECTOR3 Pos = m_hTroop->GetPos();
	int nPathIndex = -1;
	PATH_PACK* pPath = GetBestPathNode( Pos, nPath1, nPath2, nMoveDirType, &nPathIndex );
	if( pPath )
	{
		if( !CanICmdMove( m_hTroop, pPath, nMoveDirType, false) )
		{
			return false;
		}
		m_hTroop->CmdMovePath( pPath, nPathIndex, nMoveDirType == Forward, false );

		m_nLastPath1 = nPath1;
		m_nLastPath2 = nPath2;
		m_nLastPathType = PATH_TYPE_MOVE;
		m_nLastPathDir = nMoveDirType;

		return true;
	}

	DebugString( "Cannot found path %d, %d\n", nPath1, nPath2 );
	for( int i=0; i<MAX_AI_PATH_ID; i++ )
	{
		if( s_PathID[i] != -1 )
		{
			DebugString( "Registed Path %d, %d\n", i, s_PathID[i] );
		}
	}
	return false;
}

bool CFcTroopAIObject:: CmdMoveOnPathLoop( TROOPAITRIGGERDATA* pData )
{
	// Walk타입 체크해줘야 한다.
	int nWalkType = pData->Variable[0].GetVariableInt();
	int nMoveDirType = pData->Variable[1].GetVariableInt();
	int nPath1 = pData->Variable[2].GetVariableInt();
	int nPath2 = pData->Variable[3].GetVariableInt();

	if( IsOnPathState() )
	{
		if( m_nLastPath1 == nPath1 && 
			m_nLastPath2 == nPath2 &&
			m_nLastPathDir == nMoveDirType && 
			m_nLastPathType == PATH_TYPE_LOOP )
		{
			return true;
		}
	}

	BsAssert( nMoveDirType == Forward || nMoveDirType == Backward );

	D3DXVECTOR3 Pos = m_hTroop->GetPos();
	int nPathIndex = -1;
	PATH_PACK* pPath = GetBestPathNode( Pos, nPath1, nPath2, nMoveDirType, &nPathIndex );
	if( pPath )
	{
		if( !CanICmdMove( m_hTroop, pPath, nMoveDirType, true) )
		{
			return false;
		}
		m_hTroop->CmdMovePath( pPath, nPathIndex, nMoveDirType == Forward, true );

		m_nLastPath1 = nPath1;
		m_nLastPath2 = nPath2;
		m_nLastPathType = PATH_TYPE_LOOP;
		m_nLastPathDir = nMoveDirType;

		return true;
	}
	return false;
}

bool CFcTroopAIObject:: CmdMoveOnPathRandom( TROOPAITRIGGERDATA* pData )
{
	int nWalkType = pData->Variable[0].GetVariableInt();
	int nMoveDirType = pData->Variable[1].GetVariableInt();
	int nPath1 = pData->Variable[2].GetVariableInt();
	int nPath2 = pData->Variable[3].GetVariableInt();


	if( IsOnPathState() )
	{
		if( m_nLastPath1 == nPath1 && 
			m_nLastPath2 == nPath2 &&
			m_nLastPathDir == nMoveDirType && 
			m_nLastPathType == PATH_TYPE_RANDOM )
		{
			return true;
		}
	}


	BsAssert( nMoveDirType == Forward || nMoveDirType == Backward );

	D3DXVECTOR3 Pos = m_hTroop->GetPos();
	int nNearPathIndex = -1;

	int nCurIx = m_hTroop->GetCurrentPathIndex();

	if( nPath1 <= nCurIx && nCurIx <= nPath2 )
	{
		nPath1 = nCurIx;
		nPath2 = nCurIx;
	}
/*
	else
	{
		nPath1 = Random( ( nPath2 - nPath1 ) + 1 );
		nPath2 = nPath1;
	}
*/

	PATH_PACK* pPath = GetRandomPath( Pos, nPath1, nPath2, &nNearPathIndex, 0 );
//	PATH_PACK* pPath = GetBestPathNode( Pos, nPath1, nPath2, nMoveDirType, &nPathIndex );
	if( pPath )
	{
		if( !CanICmdMove( m_hTroop, pPath, nMoveDirType, false ) )
		{
			return false;
		}
		m_hTroop->CmdMovePath( pPath, nNearPathIndex, nMoveDirType == Forward, false );

		m_nLastPath1 = nPath1;
		m_nLastPath2 = nPath2;
		m_nLastPathType = PATH_TYPE_RANDOM;
		m_nLastPathDir = nMoveDirType;

		return true;
	}
	return false;
}

bool CFcTroopAIObject:: CmdMoveOnPathWithAvoidEnemy( TROOPAITRIGGERDATA* pData )
{
	BsAssert( 0 );
/*
	int nMoveDirType = pData->Variable[0].GetVariableInt();
	char* pPathStr1 = pData->Variable[1].GetVariableString();
	char* pPathStr2 = pData->Variable[2].GetVariableString();

	FILTER_DATA TargetFilterData[16];
	FILTER_DATA* pTemp = TargetFilterData;
	int nTargetFilterCount = SetFilterData( &(pData->Variable[3]), &pTemp);

	int index = 3 + nTargetFilterCount * 2;
	int nDist = pData->Variable[index].GetVariableInt();


	D3DXVECTOR3 Pos = m_hTroop->GetPos();
	int nNearPathIndex = -1;
	PATH_PACK* pPath = g_FcWorld.GetRandomPath( Pos, pPathStr1, pPathStr2, &nNearPathIndex );
	if( pPath )
	{
		m_hTroop->CmdMovePathAvoidEnemy( pPath, nNearPathIndex, nMoveDirType == Forward, false );
		return true;
	}
*/
	return false;
}

bool CFcTroopAIObject:: CmdMoveOnFullPath( TROOPAITRIGGERDATA* pData )
{
	// Walk타입 체크해줘야 한다.
	int nWalkType = pData->Variable[0].GetVariableInt();
	int nMoveDirType = pData->Variable[1].GetVariableInt();
	int nPath1 = pData->Variable[2].GetVariableInt();
	int nPath2 = pData->Variable[3].GetVariableInt();

	BsAssert( nMoveDirType == Forward || nMoveDirType == Backward );

	PATH_PACK* pPath = m_hTroop->GetFullPath();
	if( pPath == NULL )
	{
		D3DXVECTOR3 Pos = m_hTroop->GetPos();
		pPath = GetNearFullPath( Pos, nPath1, nPath2, nMoveDirType );
		BsAssert( pPath && "Cannot found FULL PATH" );
		
		int nPathIndex;
		if( nMoveDirType == Forward )
		{
			nPathIndex = 0;
		}
		else
		{
			nPathIndex = pPath->pathData.vecPosPool.size() - 1;
			BsAssert( nPathIndex >= 0 );
		}

		m_hTroop->SetFullPath( pPath, nPathIndex );
	}
	if( !CanICmdMove( m_hTroop, pPath, nMoveDirType, false) )
	{
		return false;
	}
	m_hTroop->CmdMoveFullPath( pPath, 0, nMoveDirType == Forward );
	return true;
}

bool CFcTroopAIObject:: CmdAttackEnemyNearSomebody( TROOPAITRIGGERDATA* pData )
{
	return false;
}

bool CFcTroopAIObject:: CmdAttackWall( TROOPAITRIGGERDATA* pData )
{
	return false;
}

bool CFcTroopAIObject:: CmdSetTargetID( TROOPAITRIGGERDATA* pData )
{
	char* pName = pData->Variable[0].GetVariableString();
	m_hTroopByIDFilter = g_FcWorld.GetTroopObject( pName );
	BsAssert( m_hTroopByIDFilter && "Invalid AI Filter ByID" );
	return false;
}

// follow 종류에 따른 위치 (Front, Guard, Tail)
static	float	_followerPos[3][TROOP_FOLLOWER_MAX * 2] = 
{
	{
		-0.75f, 4.0f,
		0.75f,  4.0f,
		-1.5f, 4.0f,
		1.5f,  4.0f,
		-0.75f, 2.0f,
		0.75f,  2.0f,
		-1.5f, 3.0f,
		1.5f,  3.0f,
		-1.5f, 2.0f,
		1.5f,  2.0f
	},
	{
		-1.0f, 1.0f,
		1.0f, 1.0f,
		-1.0f, 2.0f,
		1.0f, 2.0f,
		-1.0f,0.0f,
		1.0f,0.0f,
		-2.0f, -1.0f,
		2.0f, -1.0f,
		-2.0f,0.0f,
		2.0f,0.0f
	},
	{
		-1.0f,-1.0f,
		1.0f, -1.0f,
		-1.0f,-2.0f,
		1.0f, -2.0f,
		-1.0f,-3.0f,
		1.0f,-3.0f,
		-2.0f,-2.0f,
		2.0f,-2.0f,
		-2.0f,-3.0f,
		2.0f,-3.0f
	}
};

bool CFcTroopAIObject::CmdFollow( TROOPAITRIGGERDATA* pData )
{
	// Follow	[대상filter]	[시야거리]

	FILTER_DATA FilterData[16];
	FILTER_DATA* pTemp = FilterData;
	int nFilterCount = SetFilterData( &(pData->Variable[0]), &pTemp);

	int nIndex = nFilterCount*2;
	int nType = pData->Variable[nIndex].GetVariableInt();
	int nRange = pData->Variable[nIndex + 1].GetVariableInt();
	int nDist = pData->Variable[nIndex + 2].GetVariableInt();

	BsAssert (nType >= Front && nType <= Tail);
	nType -= Front;

	if (nDist < 100)
		nDist = 100;

	D3DXVECTOR2 vPos = m_hTroop->GetPosV2();
	g_FcWorld.GetTroopListInRange( &vPos, static_cast<float>( nRange ), m_VecTargetObjects );
	TroopObjHandle hTarget = CheckTargetTroop( &vPos, nRange, FilterData, nFilterCount, m_VecTargetObjects );

	if( hTarget )
	{
		m_hTroop->CmdFollow( hTarget, (float)nDist );
		return true;
	}
	return false;
}


bool CFcTroopAIObject::CmdEngageGuardianToTarget( TROOPAITRIGGERDATA* pData )
{
	bool bValid = false;
	for( int i=0; i<2; i++ )
	{
		TroopObjHandle hTroop = g_FcWorld.GetTroopmanager()->GetGuardianTroop( i );
		if( hTroop == NULL )
			continue;

		if( hTroop == m_hTroop )
		{
			bValid = true;
			break;
		}
	}
	BsAssert( bValid && "호위병도 아닌데 EngageGuardianToTarget을 불렀다." );

	if( m_hGuardianTarget == NULL )
	{
		m_hGuardianTarget = g_FcWorld.GetTroopmanager()->GetLockOnTarget();
		if( m_hGuardianTarget )
		{
			TROOPTYPE myTT = m_hTroop->GetType();
			TROOPTYPE tagTT = m_hGuardianTarget->GetType();

			if( myTT != TROOPTYPE_RANGEMELEE )
			{
				if( tagTT == TROOPTYPE_FLYING )
				{
					return false;
				}
			}
			m_hTroop->CmdAttack( m_hGuardianTarget, true );
			return true;
		}
	}

	return false;
}


bool CFcTroopAIObject::CmdEngageArea( TROOPAITRIGGERDATA* pData )
{
	FILTER_DATA FilterData[16];
	FILTER_DATA* pTemp = FilterData;
	int nFilterCount = SetFilterData( &(pData->Variable[0]), &pTemp);
	int nIndex = nFilterCount * 2;

	int nRange = pData->Variable[nIndex++].GetVariableInt();

	char* pAreaName = pData->Variable[nIndex].GetVariableString();
	AREA_INFO* pAreaInfo = g_FcWorld.GetAreaInfo( pAreaName );	

	D3DXVECTOR2 vPos = m_hTroop->GetPosV2();

	if( FilterData[0].nType >= AIGuide0 && FilterData[0].nType <= AIGuide15 )
	{
		return MoveByAIGuide( &vPos, nRange, FilterData, nFilterCount );
	}
	else
	{
		g_FcWorld.GetTroopmanager()->GetListInRangeAndArea( &vPos, static_cast<float>( nRange ), pAreaInfo->fSX, pAreaInfo->fSZ, pAreaInfo->fEX, pAreaInfo->fEZ, m_VecTargetObjects );
		
		TroopObjHandle hTarget = CheckTargetTroop( &vPos, nRange, FilterData, nFilterCount, m_VecTargetObjects );
		if( hTarget )
		{
			m_hTroop->CmdAttack( hTarget );
			return true;
		}
	}
	return false;
}



bool CFcTroopAIObject::CmdDisengageOnAttacked( TROOPAITRIGGERDATA* pData )
{
	int nRange = pData->Variable[0].GetVariableInt();

	if( m_hTroop->IsAttacked() )
	{
		m_hTroop->GetAttackedTroops( m_VecTargetObjects );

		int nCnt = m_VecTargetObjects.size();
		if( nCnt > 0 )
		{
			D3DXVECTOR2 Dir( 0.f, 0.f );
			for( int i = 0; i < nCnt; i ++ )
			{
				Dir = Dir + ( m_hTroop->GetPosV2() - m_VecTargetObjects[i]->GetPosV2() );
			}
			D3DXVec2Normalize( &Dir, &Dir );
			D3DXVec2Scale( &Dir, &Dir, static_cast<float>( nRange ) );
			m_hTroop->CmdRetreat( m_hTroop->GetPos().x + Dir.x, m_hTroop->GetPos().z + Dir.y, true );
			return true;
		}
	}
	return false;
}


bool CFcTroopAIObject::CmdEngageMercifullyArea( TROOPAITRIGGERDATA* pData )
{ 
	FILTER_DATA FilterData[16];
	FILTER_DATA* pTemp = FilterData;
	int nFilterCount = SetFilterData( &(pData->Variable[0]), &pTemp);
	int nIndex = nFilterCount * 2;

	int nMaxEngage = pData->Variable[nIndex].GetVariableInt();
	int nRange = pData->Variable[nIndex + 1].GetVariableInt();

	char* pAreaName = pData->Variable[nIndex + 2].GetVariableString();
	AREA_INFO* pAreaInfo = g_FcWorld.GetAreaInfo( pAreaName );	

	D3DXVECTOR2 vPos = m_hTroop->GetPosV2();
	g_FcWorld.GetTroopmanager()->GetListInRangeAndArea( &vPos, static_cast<float>( nRange ), pAreaInfo->fSX, pAreaInfo->fSZ, pAreaInfo->fEX, pAreaInfo->fEZ, m_VecTargetObjects );
	
	int nCnt = m_VecTargetObjects.size();
	if( nCnt <= 0 )
		return false;

	TroopObjHandle hTarget = CheckTargetTroop( &vPos, nRange, FilterData, nFilterCount, m_VecTargetObjects, nMaxEngage );
	if( hTarget == NULL )
	{
		// 목표부대 없음
		return false;
	}

	std::vector<TroopObjHandle> vecVoidObjects;
	nCnt = m_VecTargetObjects.size();
	for( int i = 0; i < nCnt; i ++ )
	{
		TroopObjHandle hTroop = m_VecTargetObjects[i];
		if( m_hTroop->GetTeam() != hTroop->GetTeam() )
		{
			if( hTarget != hTroop )
			{
				D3DXVECTOR2 Dir = hTarget->GetPosV2() - hTroop->GetPosV2();
				if( D3DXVec2LengthSq( &Dir ) > 1000.f * 1000.f )
				{
					vecVoidObjects.push_back( hTroop );
				}
			}
		}
	}
	m_hTroop->CmdEngageMercifully( hTarget, vecVoidObjects );

	return true;
}

bool CFcTroopAIObject::MoveByAIGuide( D3DXVECTOR2* pPos, int nRange, FILTER_DATA* pFilter, int nNumFilter )
{
	AREA_INFO* pArea = CheckTargetArea( pPos, nRange, pFilter, nNumFilter );
	if( pArea )
	{
		float fX = pArea->fSX + (pArea->fEX - pArea->fSX)/2.f;
		float fZ = pArea->fSZ + (pArea->fEZ - pArea->fSZ)/2.f;
		m_hTroop->CmdMove( fX, fZ, true );
		return true;
	}

	CFcProp* pProp = CheckTargetProp( pPos, nRange, pFilter, nNumFilter );
	if( pProp )
	{
		D3DXVECTOR2 Pos = pProp->GetPosV2();
		m_hTroop->CmdMove( Pos.x, Pos.y, true );
		return true;
	}

	return false;
}



/*
bool CFcTroopAIObject::CmdDisengage( TROOPAITRIGGERDATA* pData )
{
	int nRange = pData->Variable[0].GetVariableInt();
	int nFilter = pData->Variable[1].GetVariableInt();

	if( CheckTargetTroop( nRange, nFilter ) == true )
	{
		D3DXVECTOR3 Dir;
		D3DXVec3Subtract( &Dir, &(m_hTroop->GetPos()), &(m_hTarget->GetPos()) );

		D3DXVec3Normalize( &Dir, &Dir );

		float fRadius = m_hTroop->GetRadius();
		D3DXVec3Scale( &Dir, &Dir, fRadius );

		m_hTroop->CmdRetreat( m_hTarget->GetPos().x + Dir.x, m_hTarget->GetPos().z + Dir.z );

		return true;
	}
	return false;
}


bool CFcTroopAIObject::CmdModeEngage( TROOPAITRIGGERDATA* pData )
{
	int nRange = pData->Variable[0].GetVariableInt();
	int nFilter = pData->Variable[1].GetVariableInt();

	if( CheckTargetTroop( nRange, nFilter ) == true )
	{
		m_hTroop->CmdAttack( m_hTarget );
		return true;
	}
	return false;
}


bool CFcTroopAIObject::CmdModeStopAndDefense( TROOPAITRIGGERDATA* pData )
{
	int nRange = pData->Variable[0].GetVariableInt();
	int nFilter = pData->Variable[1].GetVariableInt();

	if( CheckTargetTroop( nRange, nFilter ) == true )
	{
		m_hTroop->CmdStop();
		return true;
	}
	return false;
}

bool CFcTroopAIObject::CmdResonableEngage( TROOPAITRIGGERDATA* pData )
{
	int nRange = pData->Variable[0].GetVariableInt();
	int nMaxEngageTroops = pData->Variable[1].GetVariableInt();
	int nTargetFilter = pData->Variable[2].GetVariableInt();
	int nTroopFilter = pData->Variable[3].GetVariableInt();

	if( CheckTargetTroop( nRange, nTargetFilter, nMaxEngageTroops, nTroopFilter ) == true )
	{
		m_hTroop->CmdResonableEngage( m_hTarget, m_VecTargetCounterattractionObjects );
		return true;
	}
	return false;
}
*/


bool CFcTroopAIObject::_IfHP( TROOPAITRIGGERDATA* pData )
{
	char* pOper = pData->Variable[0].GetVariableString();
	int nHPPer = pData->Variable[1].GetVariableInt();

	int nCurHPPer = m_hTroop->GetHP() * 100 / m_hTroop->GetMaxHP();

	if( CompareValue( nCurHPPer, pOper, nHPPer ) )
	{
		strcpy( m_cCurLabel, pData->Variable[2].GetVariableString() );
		return true;
	}
	return false;
}


bool CFcTroopAIObject::_IfMeleeEngaged( TROOPAITRIGGERDATA* pData )
{
	bool bEngaged = pData->Variable[0].GetVariableInt() != 0;

	bool bRet = m_hTroop->IsMeleeEngaged();
	if( bRet == bEngaged )
	{
		strcpy( m_cCurLabel, pData->Variable[1].GetVariableString() );
		return true;
	}
	return false;
}

bool CFcTroopAIObject::_IfEngaged( TROOPAITRIGGERDATA* pData )
{
	bool bEngaged = pData->Variable[0].GetVariableInt() != 0;

	bool bRet = m_hTroop->IsEngaged();
	if( bRet == bEngaged )
	{
		strcpy( m_cCurLabel, pData->Variable[1].GetVariableString() );
		return true;
	}
	return false;
}

bool CFcTroopAIObject::_IfNumFriedlyTroop( TROOPAITRIGGERDATA* pData )
{
	char* pOper = pData->Variable[0].GetVariableString();
	float fNumTroops = (float)pData->Variable[1].GetVariableInt();
	int nRange = pData->Variable[2].GetVariableInt();

	FILTER_DATA FilterData;
	FilterData.nOper = FILTER_DATA_ADD;
	FilterData.nType = Friend;

	D3DXVECTOR2 vPos = m_hTroop->GetPosV2();

	g_FcWorld.GetTroopListInRange( &vPos, static_cast<float>( nRange ), m_VecTargetObjects );
	if ( CheckTargetTroop( &vPos, nRange, &FilterData, 1, m_VecTargetObjects ) )
	{
		strcpy( m_cCurLabel, pData->Variable[3].GetVariableString() );
		return true;
	}
	return false;
	// }
}

bool CFcTroopAIObject::_IfNumEnemyTroop( TROOPAITRIGGERDATA* pData )
{
	char* pOper = pData->Variable[0].GetVariableString();
	float fNumTroops = (float)pData->Variable[1].GetVariableInt();
	int nRange = pData->Variable[2].GetVariableInt();

	FILTER_DATA FilterData;
	FilterData.nOper = FILTER_DATA_ADD;
	FilterData.nType = Enemy;

	D3DXVECTOR2 vPos = m_hTroop->GetPosV2();

	g_FcWorld.GetTroopListInRange( &vPos, static_cast<float>( nRange ), m_VecTargetObjects );
	if ( CheckTargetTroop( &vPos, nRange, &FilterData, 1, m_VecTargetObjects ) )
	{
		strcpy( m_cCurLabel, pData->Variable[3].GetVariableString() );
		return true;
	}
	return false;
	// }
}

bool CFcTroopAIObject::_IfThereIsTroop( TROOPAITRIGGERDATA* pData )
{
	FILTER_DATA FilterData[16];
	FILTER_DATA* pTemp = FilterData;
	int nFilterCount = SetFilterData( &(pData->Variable[0]), &pTemp);

	int nIndex = nFilterCount*2;
	int nRange = pData->Variable[nIndex].GetVariableInt();

	D3DXVECTOR2 vPos = m_hTroop->GetPosV2();

	g_FcWorld.GetTroopListInRange( &vPos, static_cast<float>( nRange ), m_VecTargetObjects );
	if ( CheckTargetTroop( &vPos, nRange, FilterData, nFilterCount, m_VecTargetObjects ) )
	{
		strcpy( m_cCurLabel, pData->Variable[nIndex+1].GetVariableString() );
		return true;
	}

	return false;
}

bool CFcTroopAIObject::_IfIAm( TROOPAITRIGGERDATA* pData )
{
	bool bRValue = false;
	int nTroopType = pData->Variable[0].GetVariableInt();
	TROOPTYPE myTT = m_hTroop->GetType();
	
	switch (nTroopType)
	{
	case PlayerType:
		bRValue = ( myTT == TROOPTYPE_PLAYER_1 || myTT == TROOPTYPE_PLAYER_2 );
		break;

	case GroundType:
		bRValue = ( myTT != TROOPTYPE_FLYING && myTT != TROOPTYPE_PLAYER_1 && myTT != TROOPTYPE_PLAYER_2 );
		break;

	case GuardianType:
		{
			TroopObjHandle hHandle = g_FcWorld.GetTroopmanager()->GetGuardianTroop( 0 );
			if( hHandle == m_hTroop )
			{
				bRValue = true;
				break;
			}
			hHandle = g_FcWorld.GetTroopmanager()->GetGuardianTroop( 1 );
			if( hHandle == m_hTroop )
			{
				bRValue = true;
				break;
			}
		}
		break;

	case MeleeType:
	case SwordType:
		bRValue = ( myTT == TROOPTYPE_MELEE );
		break;

	case SpearType:
		bRValue = ( myTT == TROOPTYPE_SPEAR );
		break;

	case RangeType:
	case ArcherType:
		bRValue = ( myTT == TROOPTYPE_RANGEMELEE );
		break;

	case BallistaType:
		bRValue = ( myTT == TROOPTYPE_SIEGE );
		break;

	case BossType:
		{
			DebugString("This filter is not supported yet. : Boss\n");
			BsAssert(0);
		}
		break;

	case FlyingType:
	case DragonType:
		bRValue = ( myTT == TROOPTYPE_FLYING );
		break;
	}

	if (bRValue == true)
		strcpy( m_cCurLabel, pData->Variable[1].GetVariableString() );

	return bRValue;
}

/*
bool CFcTroopAIObject::ConIsTarget( TROOPAIELEMENTDATA* pData )
{
	int nRange = pData->Variable[0].GetVariableInt();
	int nFilter = pData->Variable[1].GetVariableInt();

	if( CheckTargetTroop( nRange, nFilter ) == true )
		return true;
	else
		return false;

	return false;
}


bool CFcTroopAIObject::ConTargetHP( TROOPAIELEMENTDATA* pData )
{
	if( m_hTarget == NULL )
	{
		return false;
	}
	int nHP = pData->Variable[0].GetVariableInt();
	AI_OP Op = (AI_OP)pData->Variable[1].GetVariableInt();

	int nTargetHP = m_hTarget->GetHP();

	switch( Op ) {
	case AI_OP_EQUAL:					return ( nTargetHP == nHP );
	case AI_OP_NOT_EQUAL:				return ( nTargetHP != nHP );
	case AI_OP_GREATER:					return ( nTargetHP > nHP );
	case AI_OP_GREATER_THAN_OR_EQUAL:	return ( nTargetHP >= nHP );
	case AI_OP_LESS_THAN:				return ( nTargetHP < nHP );
	case AI_OP_LESS_THAN_OR_EQUAL:		return ( nTargetHP <= nHP );
	default:
		BsAssert(0);
		break;
	}
	return false;
}

bool CFcTroopAIObject::ConIsIdle( TROOPAIELEMENTDATA* pData )
{
	if( m_hTarget == NULL )
	{
		return false;
	}

	if( m_hTarget->GetState() == TROOPSTATE_NON )
	{
		return true;
	}
	return false;
}

bool CFcTroopAIObject::ConRatio( TROOPAIELEMENTDATA* pData )
{
	int nRatio = pData->Variable[0].GetVariableInt();

	if( Random( 10000 ) < nRatio )
	{
		return true;
	}
	return false;
}
*/



TroopObjHandle CFcTroopAIObject::CheckTargetTroop( D3DXVECTOR2* pPos, int nRange, FILTER_DATA* pFilter, int nNumFilter, std::vector<TroopObjHandle> &vecTargetObjects, int nMaxEngage )
{
//DebugString( "CTT %s\n", m_hTroop->GetName() );
	
	for( int i = 0; i < nNumFilter; i ++ )
	{
		if( pFilter[i].nType < Superlative )
		{
			std::vector<TroopObjHandle>::iterator it = vecTargetObjects.begin();
			while( it != vecTargetObjects.end() )
			{
				TroopObjHandle hTargetTroop = *it;

				if( hTargetTroop == m_hTroop )
				{
					it = vecTargetObjects.erase( it );
					continue;
				}
				// GetTroopListInRange 에서 이미 걸러서 온다
				//if( hTargetTroop->IsEliminated() )
				//{
				//	it = vecTargetObjects.erase( it );
				//	continue;
				//}
				//if( !hTargetTroop->IsEnable() )
				//{
				//	it = vecTargetObjects.erase( it );
				//	continue;
				//}

				// 밑에서 하고 있네
                //int nCnt = (*it)->GetMeleeEngageTroopCount();
				//if( nCnt >= nMaxEngage )
				//{
				//	it = vecTargetObjects.erase( it );
				//	continue;
				//}

				bool bRet = false;

//DebugString( "CTT2 %d\n", pFilter[i].nType );

				switch( pFilter[i].nType )
				{
				case Friend:
					bRet = ( m_hTroop->GetTeam() == hTargetTroop->GetTeam() );
					break;

				case Mine:
					bRet = ( m_hTroop->GetTeam() == hTargetTroop->GetTeam()
								&& m_hTroop->GetForce() == hTargetTroop->GetForce() );
					break;

				case Partner:
					bRet = ( m_hTroop->GetTeam() == hTargetTroop->GetTeam()
								&& m_hTroop->GetForce() != hTargetTroop->GetForce() );
					break;

				case Enemy:
					bRet = ( m_hTroop->GetTeam() != hTargetTroop->GetTeam() );
					break;

				case Player:
					bRet = ( hTargetTroop->GetType() == TROOPTYPE_PLAYER_1 || hTargetTroop->GetType() == TROOPTYPE_PLAYER_2 );
					break;

				case Player1:
					bRet = ( hTargetTroop->GetType() == TROOPTYPE_PLAYER_1 );
					break;

				case Player2:
					bRet = ( hTargetTroop->GetType() == TROOPTYPE_PLAYER_2 );
					break;

				case MyPlayer:
					bRet = ( m_hTroop->GetTeam() == hTargetTroop->GetTeam()
								&& m_hTroop->GetForce() == hTargetTroop->GetForce()
								&& ( hTargetTroop->GetType() == TROOPTYPE_PLAYER_1 || hTargetTroop->GetType() == TROOPTYPE_PLAYER_2 ) );
					break;
				
				case OtherPlayer:
					bRet = ( m_hTroop->GetTeam() == hTargetTroop->GetTeam()
								&& m_hTroop->GetForce() != hTargetTroop->GetForce()
								&& ( hTargetTroop->GetType() == TROOPTYPE_PLAYER_1 || hTargetTroop->GetType() == TROOPTYPE_PLAYER_2 ) );
					break;

				case MyLinker:
					bRet = ( m_hTroop->GetLinkedTroop() == hTargetTroop );
					break;

				case Ground:
					bRet = ( hTargetTroop->GetType() != TROOPTYPE_FLYING
								&& hTargetTroop->GetType() != TROOPTYPE_PLAYER_1
								&& hTargetTroop->GetType() != TROOPTYPE_PLAYER_2 );
					break;

				case Guardian:
					{
						TroopObjHandle hHandle = g_FcWorld.GetTroopmanager()->GetGuardianTroop( 0 );
						if( hHandle == hTargetTroop )
						{
							bRet = true;
							break;
						}
						hHandle = g_FcWorld.GetTroopmanager()->GetGuardianTroop( 1 );
						if( hHandle == hTargetTroop )
						{
							bRet = true;
							break;
						}
					}
					break;

				case Guardian1:
					{
						TroopObjHandle hHandle = g_FcWorld.GetTroopmanager()->GetGuardianTroop( 0 );
						if( hHandle == hTargetTroop )
						{
							bRet = true;
							break;
						}
					}
					break;

				case Guardian2:
					{
						TroopObjHandle hHandle = g_FcWorld.GetTroopmanager()->GetGuardianTroop( 1 );
						if( hHandle == hTargetTroop )
						{
							bRet = true;
							break;
						}
					}
					break;

				case Melee:
					bRet = ( hTargetTroop->GetType() == TROOPTYPE_MELEE || hTargetTroop->GetType() == TROOPTYPE_SPEAR );
					break;

				case Sword:
					bRet = ( hTargetTroop->GetType() == TROOPTYPE_MELEE );
					break;

				case Spear:
					bRet = ( hTargetTroop->GetType() == TROOPTYPE_SPEAR );
					break;

				case Range:
				case Archer:
					bRet = ( hTargetTroop->GetType() == TROOPTYPE_RANGEMELEE );
					break;

				case Siege:
				case Ballista:
					bRet = ( hTargetTroop->GetType() == TROOPTYPE_SIEGE );
					break;

				case Boss:
					{
						DebugString("This filter is not supported yet. : Boss\n");
						BsAssert(0);
					}
					break;

				case Flying:
				case Dragon:
					bRet = ( hTargetTroop->GetType() == TROOPTYPE_FLYING );
					break;


				case People:
					bRet = ( hTargetTroop->GetType() == TROOPTYPE_NOBATTLE );
					break;

				case ByID:
					bRet = ( m_hTroopByIDFilter == hTargetTroop );
					break;

				case AIGuide0:
				case AIGuide1:
				case AIGuide2:
				case AIGuide3:
				case AIGuide4:
				case AIGuide5:
				case AIGuide6:
				case AIGuide7:
				case AIGuide8:
				case AIGuide9:
				case AIGuide10:
				case AIGuide11:
				case AIGuide12:
				case AIGuide13:
				case AIGuide14:
				case AIGuide15:
/*
					{
						DebugString("This filter is not supported yet. : AIGuide\n");
						BsAssert(0);
					}
*/
					bRet = false;
					break;

				case HPUnder25:
					{
						int nPer = hTargetTroop->GetHP() * 100 / hTargetTroop->GetMaxHP();
						bRet = ( nPer < 25 );
					}
					break;

				case HPUnder50:
					{
						int nPer = hTargetTroop->GetHP() * 100 / hTargetTroop->GetMaxHP();
						bRet = ( nPer < 50 );
					}
					break;

				case HPUnder75:
					{
						int nPer = hTargetTroop->GetHP() * 100 / hTargetTroop->GetMaxHP();
						bRet = ( nPer < 75 );
					}
					break;

				case MyWorst:
					{
						bRet = m_hTroop->IsAttackedBy( hTargetTroop );
					}
					break;

				case MyPlayersWorst:
					{
						TroopObjHandle hHandle = g_FcWorld.GetHeroHandle()->GetTroop();
						if( m_hTroop->GetTeam() != hHandle->GetTeam() )
							break;
						if( m_hTroop->GetForce() != hHandle->GetForce() )
							break;

						bRet = hHandle->IsAttackedBy( hTargetTroop );
					}
					break;

				case OtherPlayersWorst:
					{
						TroopObjHandle hHandle = g_FcWorld.GetHeroHandle()->GetTroop();
						if( m_hTroop->GetTeam() != hHandle->GetTeam() )
							break;
						if( m_hTroop->GetForce() == hHandle->GetForce() )
							break;

						bRet = hHandle->IsAttackedBy( hTargetTroop );
					}

					break;

				case SomebodysWorst:
					bRet = m_hTroopByIDFilter->IsAttackedBy( hTargetTroop );
					break;

				case SwordsWorst:
					bRet = m_hTroopByIDFilter->IsMeleeAttackedBy( hTargetTroop );
					bRet = bRet && ( hTargetTroop->GetType() == TROOPTYPE_MELEE ); 
					break;

				case SpearsWorst:
					bRet = m_hTroopByIDFilter->IsMeleeAttackedBy( hTargetTroop );
					bRet = bRet && ( hTargetTroop->GetType() == TROOPTYPE_SPEAR );
					break;

				case MeleesWorst:
					bRet = m_hTroopByIDFilter->IsMeleeAttackedBy( hTargetTroop );
					bRet = bRet && ( hTargetTroop->GetType() == TROOPTYPE_MELEE || 
									hTargetTroop->GetType() == TROOPTYPE_SPEAR  ||
									hTargetTroop->GetType() == TROOPTYPE_RANGEMELEE );
					break;

				case ArchersWorst:
					bRet = m_hTroopByIDFilter->IsRangeAttackedBy( hTargetTroop );
					bRet = bRet && ( hTargetTroop->GetType() == TROOPTYPE_RANGEMELEE || 
									hTargetTroop->GetType() == TROOPTYPE_RANGE );
					break;

				case BallistasWorst:
					bRet = m_hTroopByIDFilter->IsRangeAttackedBy( hTargetTroop );
					bRet = bRet && ( hTargetTroop->GetType() == TROOPTYPE_SIEGE );
					break;

				case RangesWorst:
					bRet = m_hTroopByIDFilter->IsRangeAttackedBy( hTargetTroop );
					bRet = bRet && ( hTargetTroop->GetType() == TROOPTYPE_RANGEMELEE || 
									hTargetTroop->GetType() == TROOPTYPE_RANGE );
					break;

				case RideHorse:
					DebugString("This filter is not supported yet. : RideHorse\n");
					BsAssert(0);
					break;

				case Wall:
					bRet = ( hTargetTroop->GetType() == TROOPTYPE_WALL_DUMMY );
					break;
				}

				if( pFilter[i].nOper == FILTER_DATA_SUBTRACT )
					bRet = !bRet;

				if( bRet == false )
					it = vecTargetObjects.erase( it );
				else
					it++;
			}
		}
		else
		{
			switch( pFilter[i].nType )
			{
			case NearestToPlayer1:
				{
					TroopObjHandle hTarget;
                    TroopObjHandle hHandle = g_FcWorld.GetHeroHandle()->GetTroop();
					float fMinDist = 9999999.f; //static_cast<float>( nRange );
					int nCnt = vecTargetObjects.size();

					for(int c = 0; c < nCnt; c ++)
					{
	                    int nC = vecTargetObjects[ c ]->GetMeleeEngageTroopCount();
						if( nC >= nMaxEngage )
							continue;

						D3DXVECTOR2 Dir = hHandle->GetPosV2() - vecTargetObjects[ c ]->GetPosV2();
						float fDist = D3DXVec2Length( &Dir );
						if( fDist < fMinDist )
						{
							fMinDist = fDist;
							hTarget = vecTargetObjects[ c ];
						}
					}
					return hTarget;
				}
				break;

			case NearestToPlayer2:
				{
					BsAssert( 0 );
					TroopObjHandle hTarget;
					return hTarget;
				}
				break;

			case NearestToMyPlayer:
				{
					TroopObjHandle hTarget;
					TroopObjHandle hHandle = g_FcWorld.GetHeroHandle()->GetTroop();
					if( m_hTroop->GetTeam() != hHandle->GetTeam() )
						return hTarget;

					float fMinDist = 9999999.f; //static_cast<float>( nRange );
					int nCnt = vecTargetObjects.size();

					for(int c = 0; c < nCnt; c ++)
					{
	                    int nC = vecTargetObjects[ c ]->GetMeleeEngageTroopCount();
						if( nC >= nMaxEngage )
							continue;

						D3DXVECTOR2 Dir = hHandle->GetPosV2() - vecTargetObjects[ c ]->GetPosV2();
						float fDist = D3DXVec2Length( &Dir );
						if( fDist < fMinDist )
						{
							fMinDist = fDist;
							hTarget = vecTargetObjects[ c ];
						}
					}
					return hTarget;
				}
				break;

			case NearestToOtherPlayer:
				{
					TroopObjHandle hTarget;
					TroopObjHandle hHandle = g_FcWorld.GetHeroHandle()->GetTroop();
					if( m_hTroop->GetTeam() != hHandle->GetTeam() )
						return hTarget;

					float fMinDist = 9999999.f; //static_cast<float>( nRange );
					int nCnt = vecTargetObjects.size();

					for(int c = 0; c < nCnt; c ++)
					{
	                    int nC = vecTargetObjects[ c ]->GetMeleeEngageTroopCount();
						if( nC >= nMaxEngage )
							continue;

						D3DXVECTOR2 Dir = hHandle->GetPosV2() - vecTargetObjects[ c ]->GetPosV2();
						float fDist = D3DXVec2Length( &Dir );
						if( fDist < fMinDist )
						{
							fMinDist = fDist;
							hTarget = vecTargetObjects[ c ];
						}
					}
					return hTarget;
				}
				break;

			case Nearest:
				{
					TroopObjHandle hTarget;
					float fMinDist = 9999999.f; //static_cast<float>( nRange );
					int nCnt = vecTargetObjects.size();

					for(int c = 0; c < nCnt; c ++)
					{
	                    int nC = vecTargetObjects[ c ]->GetMeleeEngageTroopCount();
						if( nC >= nMaxEngage )
							continue;

						D3DXVECTOR2 Dir = m_hTroop->GetPosV2() - vecTargetObjects[ c ]->GetPosV2();
						float fDist = D3DXVec2Length( &Dir );
						if( fDist < fMinDist )
						{
							fMinDist = fDist;
							hTarget = vecTargetObjects[ c ];
						}
					}
					return hTarget;
				}
				break;

			case Weakest:
				{
					TroopObjHandle hTarget;
					int nMinHP = 999999;
					int nCnt = vecTargetObjects.size();

					for(int c = 0; c < nCnt; c ++)
					{
	                    int nC = vecTargetObjects[ c ]->GetMeleeEngageTroopCount();
						if( nC >= nMaxEngage )
							continue;

						int nHP = vecTargetObjects[ c ]->GetHP();
						if( nHP < nMinHP )
						{
							nMinHP = nHP;
							hTarget = vecTargetObjects[ c ];
						}
					}
					return hTarget;
				}

			case Strongest:
				{
					TroopObjHandle hTarget;
					int nMaxHP = 0;
					int nCnt = vecTargetObjects.size();

					for(int c = 0; c < nCnt; c ++)
					{
	                    int nC = vecTargetObjects[ c ]->GetMeleeEngageTroopCount();
						if( nC >= nMaxEngage )
							continue;

						int nHP = vecTargetObjects[ c ]->GetHP();
						if( nHP > nMaxHP )
						{
							nMaxHP = nHP;
							hTarget = vecTargetObjects[ c ];
						}
					}
					return hTarget;
				}

				break;

			case FarthestToPlayer1:
				{
					TroopObjHandle hTarget;
                    TroopObjHandle hHandle = g_FcWorld.GetHeroHandle()->GetTroop();
					float fMaxDist = 0.f;
					int nCnt = vecTargetObjects.size();

					for(int c = 0; c < nCnt; c ++)
					{
	                    int nC = vecTargetObjects[ c ]->GetMeleeEngageTroopCount();
						if( nC >= nMaxEngage )
							continue;

						D3DXVECTOR2 Dir = hHandle->GetPosV2() - vecTargetObjects[ c ]->GetPosV2();
						float fDist = D3DXVec2Length( &Dir );
						if( fDist > fMaxDist )
						{
							fMaxDist = fDist;
							hTarget = vecTargetObjects[ c ];
						}
					}
					return hTarget;
				}
				break;

			case FarthestToPlayer2:
				{
					BsAssert( 0 );
					TroopObjHandle hTarget;
					return hTarget;
				}
				break;

			case FarthestToMyPlayer:
				{
					TroopObjHandle hTarget;
					TroopObjHandle hHandle = g_FcWorld.GetHeroHandle()->GetTroop();
					if( m_hTroop->GetTeam() != hHandle->GetTeam() )
						return hTarget;

					float fMaxDist = 0.f;
					int nCnt = vecTargetObjects.size();

					for(int c = 0; c < nCnt; c ++)
					{
	                    int nC = vecTargetObjects[ c ]->GetMeleeEngageTroopCount();
						if( nC >= nMaxEngage )
							continue;

						D3DXVECTOR2 Dir = hHandle->GetPosV2() - vecTargetObjects[ c ]->GetPosV2();
						float fDist = D3DXVec2Length( &Dir );
						if( fDist > fMaxDist )
						{
							fMaxDist = fDist;
							hTarget = vecTargetObjects[ c ];
						}
					}
					return hTarget;
				}
				break;

			case FarthestToOtherPlayer:
				{
					TroopObjHandle hTarget;
					TroopObjHandle hHandle = g_FcWorld.GetHeroHandle()->GetTroop();
					if( m_hTroop->GetTeam() != hHandle->GetTeam() )
						return hTarget;

					float fMaxDist = 0.f;
					int nCnt = vecTargetObjects.size();

					for(int c = 0; c < nCnt; c ++)
					{
	                    int nC = vecTargetObjects[ c ]->GetMeleeEngageTroopCount();
						if( nC >= nMaxEngage )
							continue;

						D3DXVECTOR2 Dir = hHandle->GetPosV2() - vecTargetObjects[ c ]->GetPosV2();
						float fDist = D3DXVec2Length( &Dir );
						if( fDist > fMaxDist )
						{
							fMaxDist = fDist;
							hTarget = vecTargetObjects[ c ];
						}
					}
					return hTarget;
				}
				break;

			case Farthest:
				{
					TroopObjHandle hTarget;
					float fMaxDist = 0.f;
					int nCnt = vecTargetObjects.size();

					for(int c = 0; c < nCnt; c ++)
					{
	                    int nC = vecTargetObjects[ c ]->GetMeleeEngageTroopCount();
						if( nC >= nMaxEngage )
							continue;

						D3DXVECTOR2 Dir = m_hTroop->GetPosV2() - vecTargetObjects[ c ]->GetPosV2();
						float fDist = D3DXVec2Length( &Dir );
						if( fDist > fMaxDist )
						{
							fMaxDist = fDist;
							hTarget = vecTargetObjects[ c ];
						}
					}
					return hTarget;
				}
				break;

			default:
				{
					DebugString("Invalid Filter : (%d)\n", pFilter[i].nType);
					BsAssert(0);
				}
				break;
			}
		}
	}

	TroopObjHandle hTarget;
	float fMinDist = 9999999.f; //static_cast<float>( nRange );
	int nCnt = vecTargetObjects.size();

	for(int i = 0; i < nCnt; i ++)
	{
	    int nC = vecTargetObjects[ i ]->GetMeleeEngageTroopCount();
		if( nC >= nMaxEngage )
			continue;

		D3DXVECTOR2 Dir = m_hTroop->GetPosV2() - vecTargetObjects[ i ]->GetPosV2();
		float fDist = D3DXVec2Length( &Dir );
		if( fDist < fMinDist )
		{
			fMinDist = fDist;
			hTarget = vecTargetObjects[ i ];
		}
	}

	return hTarget;
}


CFcProp* CFcTroopAIObject::CheckTargetProp( D3DXVECTOR2* pPos, int nRange, FILTER_DATA* pFilter, int nNumFilter )
{
	for( int i = 0; i < nNumFilter; i ++ )
	{
		bool bRet = false;
		if( pFilter[i].nType >= AIGuide0 && pFilter[i].nType <= AIGuide15 )
		{
			int nIndex = pFilter[i].nType - AIGuide0;
			if( s_AIGuideData[nIndex].pProp )
				return s_AIGuideData[nIndex].pProp;
			else if( s_AIGuideData[nIndex].pArea == NULL )
			{
				DebugString( "AIGuide%d is not setting\n", nIndex );
			}
		}
	}
	return NULL;
}

AREA_INFO* CFcTroopAIObject::CheckTargetArea( D3DXVECTOR2* pPos, int nRange, FILTER_DATA* pFilter, int nNumFilter )
{
	AREA_INFO* pAreaInfo = NULL;

	float fMinDistSQR = static_cast<float>( nRange );
	fMinDistSQR *= fMinDistSQR;

	for( int i = 0; i < nNumFilter; i ++ )
	{
		if( pFilter[i].nType >= AIGuide0 && pFilter[i].nType <= AIGuide15 )
		{
			int nIndex = pFilter[i].nType - AIGuide0;
			if( s_AIGuideData[nIndex].pArea )
			{
				float fX = s_AIGuideData[nIndex].pArea->fSX + ( s_AIGuideData[nIndex].pArea->fEX - s_AIGuideData[nIndex].pArea->fSX ) / 2.f;
				float fZ = s_AIGuideData[nIndex].pArea->fSZ + ( s_AIGuideData[nIndex].pArea->fEZ - s_AIGuideData[nIndex].pArea->fSZ ) / 2.f;

				float fDistSQR = ( pPos->x - fX ) * ( pPos->x - fX ) + ( pPos->y - fZ ) * ( pPos->y - fZ );

				if( fDistSQR < fMinDistSQR )
				{
					fMinDistSQR = fDistSQR;
					pAreaInfo = s_AIGuideData[nIndex].pArea;
				}
			}
			else if( s_AIGuideData[nIndex].pProp == NULL )
			{
				DebugString( "AIGuide%d is not setting\n", nIndex );
			}
		}
	}

	return pAreaInfo;
}

int CFcTroopAIObject::GetNumEngage( TroopObjHandle hTroop, int nFilter )
{
/*
	int nCnt = hTroop->GetMeleeEngageTroopCount();

	int nRetCount = 0;
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTarget = hTroop->GetMeleeEngageTroop( i );
		switch( nFilter )
		{
		case AI_FILTER_FRIEND:
			if( hTarget->GetTeam() == 0 )		// 우리편 그룹 0으로 하드코딩
			{
				++nRetCount;
			}
			break;
		default:
			BsAssert(0);
			break;
		}
	}
	return nRetCount;
*/
	return 0;
}


bool CFcTroopAIObject::CompareValue( int nValue1, char* pOperStr, int nValue2 )
{
	switch( pOperStr[0] )
	{
	case '>':
		if( pOperStr[1] == '=' )
		{
			return( nValue1 >= nValue2 );
		}
		else
		{
			BsAssert( pOperStr[1] == NULL );
			return( nValue1 > nValue2 );
		}
	case '=':
		if( pOperStr[1] == '=' )
		{
			return( nValue1 == nValue2 );
		}
		else
		{
			BsAssert( 0 && "TroopAI Operator Err" );
			break;
		}
	case '<':
		if( pOperStr[1] == '=' )
		{
			return ( nValue1 <= nValue2 );
		}
		else
		{
			BsAssert( pOperStr[1] == NULL );
			return( nValue1 < nValue2 );
		}
	case '!':
		BsAssert( pOperStr[1] == '=' );
		return( nValue1 != nValue2 );
	default:
		BsAssert( 0 );
		break;
	}
 	return false;
}


bool CFcTroopAIObject::CheckFilter( TroopObjHandle hTroop, TroopObjHandle hTargetTroop, FILTER_DATA* pFilter, int nNumFilter )
{
	bool bRet = false;
	TroopObjHandle Handle;

	for( int i=0; i<nNumFilter; i++ )
	{
		switch( pFilter[i].nType )
		{
		case Friend:
			bRet = ( hTroop->GetTeam() == hTargetTroop->GetTeam() );
			break;

		case Mine:
			bRet = ( hTroop->GetTeam() == hTargetTroop->GetTeam()
						&& hTroop->GetForce() == hTargetTroop->GetForce() );
			break;

		case Partner:
			bRet = ( hTroop->GetTeam() == hTargetTroop->GetTeam()
						&& hTroop->GetForce() != hTargetTroop->GetForce() );
			break;

		case Enemy:
			bRet = ( hTroop->GetTeam() != hTargetTroop->GetTeam() );
			break;

		case Player:
			bRet = ( hTargetTroop->GetType() == TROOPTYPE_PLAYER_1 || hTargetTroop->GetType() == TROOPTYPE_PLAYER_2 );
			break;

		case Player1:
			bRet = ( hTargetTroop->GetType() == TROOPTYPE_PLAYER_1 );
			break;

		case Player2:
			bRet = ( hTargetTroop->GetType() == TROOPTYPE_PLAYER_2 );
			break;

		case MyPlayer:
			bRet = ( hTroop->GetTeam() == hTargetTroop->GetTeam()
						&& hTroop->GetForce() == hTargetTroop->GetForce()
						&& ( hTargetTroop->GetType() == TROOPTYPE_PLAYER_1 || hTargetTroop->GetType() == TROOPTYPE_PLAYER_2 ) );
			break;
		
		case OtherPlayer:
			bRet = ( hTroop->GetTeam() == hTargetTroop->GetTeam()
						&& hTroop->GetForce() != hTargetTroop->GetForce()
						&& ( hTargetTroop->GetType() == TROOPTYPE_PLAYER_1 || hTargetTroop->GetType() == TROOPTYPE_PLAYER_2 ) );
			break;

		case MyLinker:
			bRet = ( m_hTroop->GetLinkedTroop() == hTargetTroop );
			break;

		case Ground:
			bRet = ( hTargetTroop->GetType() != TROOPTYPE_FLYING
						&& hTargetTroop->GetType() != TROOPTYPE_PLAYER_1
						&& hTargetTroop->GetType() != TROOPTYPE_PLAYER_2 );
			break;

		case Guardian:
			Handle = g_FcWorld.GetTroopmanager()->GetGuardianTroop( 0 );
			if( Handle == hTargetTroop )
			{
				bRet = true;
				break;
			}
			Handle = g_FcWorld.GetTroopmanager()->GetGuardianTroop( 1 );
			if( Handle == hTargetTroop )
			{
				bRet = true;
				break;
			}
			break;

		case Guardian1:
			Handle = g_FcWorld.GetTroopmanager()->GetGuardianTroop( 0 );
			if( Handle == hTargetTroop )
			{
				bRet = true;
				break;
			}
			break;

		case Guardian2:
			Handle = g_FcWorld.GetTroopmanager()->GetGuardianTroop( 1 );
			if( Handle == hTargetTroop )
			{
				bRet = true;
				break;
			}
			break;

		case Melee:
			bRet = ( hTargetTroop->GetType() == TROOPTYPE_MELEE || hTargetTroop->GetType() == TROOPTYPE_SPEAR );
			break;

		case Sword:
			bRet = ( hTargetTroop->GetType() == TROOPTYPE_MELEE );
			break;

		case Spear:
			bRet = ( hTargetTroop->GetType() == TROOPTYPE_SPEAR );
			break;

		case Range:
		case Archer:
			bRet = ( hTargetTroop->GetType() == TROOPTYPE_RANGEMELEE );
			break;

		case Siege:
		case Ballista:
			bRet = ( hTargetTroop->GetType() == TROOPTYPE_SIEGE );
			break;

		case Flying:
		case Dragon:
			bRet = ( hTargetTroop->GetType() == TROOPTYPE_FLYING );
			break;

		case People:
			bRet = ( hTargetTroop->GetType() == TROOPTYPE_NOBATTLE );
			break;

		case ByID:
			{
				bRet = ( m_hTroopByIDFilter == hTargetTroop );
				break;
			}
		case AIGuide0:
		case AIGuide1:
		case AIGuide2:
		case AIGuide3:
		case AIGuide4:
		case AIGuide5:
		case AIGuide6:
		case AIGuide7:
		case AIGuide8:
		case AIGuide9:
		case AIGuide10:
		case AIGuide11:
		case AIGuide12:
		case AIGuide13:
		case AIGuide14:
		case AIGuide15:
			BsAssert(0);
			break;

		case HPUnder75:
			{
				int nPer = hTargetTroop->GetHP() * 100 / hTargetTroop->GetMaxHP();
				bRet = ( nPer <= 75 );
			}
			break;

		case HPUnder50:
			{
				int nPer = hTargetTroop->GetHP() * 100 / hTargetTroop->GetMaxHP();
				bRet = ( nPer <= 50 );
			}
			break;

		case HPUnder25:
			{
				int nPer = hTargetTroop->GetHP() * 100 / hTargetTroop->GetMaxHP();
				bRet = ( nPer <= 25 );
			}
			break;

		default:
			BsAssert(0);
			break;
		}

		if( pFilter[i].nOper == FILTER_DATA_ADD )
		{
			if( bRet == false )
				return false;
		}
		else if( pFilter[i].nOper == FILTER_DATA_SUBTRACT )
		{
			if( bRet == true )
				return false;
		}
		else
		{
			BsAssert(0);
		}
	}

	return true;
}

int CFcTroopAIObject::SetFilterData( CFcParamVariable* Variable, FILTER_DATA** ppFilter )
{
	int nFilterCount = 0;
	int nIndex = 0;

	while( 1 )
	{
		if( Variable[nIndex].GetType() != CFcParamVariable::STRING )
			break;

		char* str = Variable[nIndex].GetVariableString();

		if( str[0] == '+' ) 
		{
			(*ppFilter)[nFilterCount].nOper = FILTER_DATA_ADD;
			nIndex++;
			int nType = Variable[nIndex].GetVariableInt();
			(*ppFilter)[nFilterCount].nType = nType;
			nFilterCount++;
		}
		else if( str[0] == '-' )
		{
			(*ppFilter)[nFilterCount].nOper = FILTER_DATA_SUBTRACT;
			nIndex++;
			int nType = Variable[nIndex].GetVariableInt();
			(*ppFilter)[nFilterCount].nType = nType;
			nFilterCount++;
		}
		else
		{
			break;
		}

		++nIndex;
	}
	return nFilterCount;
}

// 노드 0번이 가장 가까운 녀석 체크
PATH_PACK* CFcTroopAIObject::GetNearFullPath( D3DXVECTOR3& Pos, int nPath1, int nPath2, int nDirType )
{
	// TODO: 최적화 필요! yooty
	float fMinPath = 0.f;
	PATH_PACK* pNearestPath = NULL;
	int nNearestPathIndex = -1;
	int nCnt = g_FcWorld.GetPathCount();
	int j = 0;
	for( int i=0; i<nCnt; i++ )
	{
		PATH_PACK* pPath = g_FcWorld.GetPathInfo( i );
		bool bValid = false;
		for( j=nPath1; j<=nPath2; j++ )
		{
			if( s_PathID[j] == pPath->pathInfo.nID )
			{
				bValid = true;
				break;
			}
		}
		if( bValid == false )
		{
			continue;
		}
		D3DXVECTOR3* PathPos;
		if( nDirType == Forward )
		{
			PathPos = &(pPath->pathData.vecPosPool[0]);
		}
		else
		{
			int nIndex = pPath->pathData.vecPosPool.size() - 1;
			BsAssert( nIndex >= 0 );
			PathPos = &(pPath->pathData.vecPosPool[nIndex]);
		}
		D3DXVECTOR3 Dir;
		D3DXVec3Subtract( &Dir, PathPos, &Pos );
		float fDist = D3DXVec3Length( &Dir );

		if( pNearestPath == NULL )
		{
			pNearestPath = pPath;
			nNearestPathIndex = j;
			fMinPath = fDist;
		}
		else
		{
			if( fDist < fMinPath )
			{
				pNearestPath = pPath;
				nNearestPathIndex = j;
				fMinPath = fDist;
			}
		}
	}
	BsAssert( pNearestPath );
	return pNearestPath;
}


PATH_PACK* CFcTroopAIObject::GetBestPathNode( D3DXVECTOR3& Pos, int nPath1, int nPath2, int nMoveDirType, int* nPathIndex )
{
	// TODO: 최적화 필요! yooty
	float fMinPath = 0.f;
	PATH_PACK* pNearestPath = NULL;
	int nNearestPathIndex = -1;
	int nCnt = g_FcWorld.GetPathCount();
	for( int i=0; i<nCnt; i++ )
	{
		PATH_PACK* pPath = g_FcWorld.GetPathInfo( i );
		bool bValid = false;
		for( int j=nPath1; j<=nPath2; j++ )
		{
			if( s_PathID[j] == pPath->pathInfo.nID )
			{
				bValid = true;
				break;
			}
		}
		if( bValid == false )
			continue;

		int nPosCnt = pPath->pathData.vecPosPool.size();
		for(int j=0; j<nPosCnt; j++ )
		{
			D3DXVECTOR3* PathPos = &(pPath->pathData.vecPosPool[j]);
			D3DXVECTOR3 Dir;
			D3DXVec3Subtract( &Dir, PathPos, &Pos );
			float fDist = D3DXVec3Length( &Dir );

			if( pNearestPath == NULL )
			{
				pNearestPath = pPath;
				nNearestPathIndex = j;
				fMinPath = fDist;
			}
			else
			{
				if( fDist < fMinPath )
				{
					pNearestPath = pPath;
					nNearestPathIndex = j;
					fMinPath = fDist;
				}
			}
		}
	}

	if( pNearestPath == NULL )
	{
		DebugString( "PathErr!! %d, %d, %d\n", nPath1, nPath2, nMoveDirType );
		return NULL;
	}

    int nPosCnt = pNearestPath->pathData.vecPosPool.size();
	int nNextIndex = nNearestPathIndex;

//	for( int i=0; i<nPosCnt; i++ )
	for( int i=0; i<2; i++ )		//  Path 앞에 2node만 검색한다.
	{
		if( nMoveDirType == Forward )		{ nNextIndex++; }
		else if( nMoveDirType == Backward ) { nNextIndex--;	}
		else								{ BsAssert(0); }
		if( nNextIndex < nPosCnt && nNextIndex >= 0 )
		{
			D3DXVECTOR3* pPathPos = &(pNearestPath->pathData.vecPosPool[nNearestPathIndex]);
			D3DXVECTOR3* pNextPathPos = &(pNearestPath->pathData.vecPosPool[nNextIndex]);
			D3DXVECTOR2 PathDir = D3DXVECTOR2( pNextPathPos->x - pPathPos->x, pNextPathPos->z - pPathPos->z );
			D3DXVECTOR2 MoveDir1 = D3DXVECTOR2( pPathPos->x - Pos.x, pPathPos->z - Pos.z );
			// 뒤로 가는 경우
			if( D3DXVec2Dot( &PathDir, &MoveDir1 ) < 0.f )
				nNearestPathIndex = nNextIndex;
		}
	}
	*nPathIndex = nNearestPathIndex;
	return pNearestPath;
}

PATH_PACK* CFcTroopAIObject::GetRandomPath( D3DXVECTOR3& Pos, int nPath1, int nPath2, int* nPathIndex, int nMaxUsePath )
{
	float fMinPath = 0.f;
	PATH_PACK* pNearestPath = NULL;
	int nNearestPathIndex = -1;

	int nPathCnt = 0;
	if( nPath2 < nPath1 )
	{
		int nTemp = nPath1;
		nPath1 = nPath2;
		nPath2 = nTemp;
	}

	// 중간에 빠진거 걸러내느라 복잡하다.
	for( int i=nPath1; i<=nPath2; i++ )
	{
		if( CFcTroopAIObject::s_PathID[i] >= 0 )
			++nPathCnt;
	}
	int nPathFouncCnt = Random( nPathCnt );
	int nPathID = -1;

	int nTempPathCnt = 0;
	for( int i=nPath1; i<=nPath2; i++ )
	{
		if( CFcTroopAIObject::s_PathID[i] >= 0 )
		{
			if( nTempPathCnt == nPathFouncCnt )
			{
				nPathID = i;
				break;
			}
			++nTempPathCnt;
		}
	}

	BsAssert( nPathID != -1 );
	DebugString( "RandomPath error! Troop:%s, Path1:%d, Path2:%d\n", m_hTroop->GetName(), nPath1, nPath2 );

	int nCnt = g_FcWorld.GetPathCount();
	for( int i=0; i<nCnt; i++ )
	{
		PATH_PACK* pPath = g_FcWorld.GetPathInfo( i );
		if( CFcTroopAIObject::s_PathID[nPathID] == pPath->pathInfo.nID )
		{
			pNearestPath = pPath;
//			nNearestPathIndex = nPathID;
			break;
		}
	}

	if( pNearestPath == NULL )
	{
		*nPathIndex = -1;
		return NULL;
	}

	int nPosCnt = pNearestPath->pathData.vecPosPool.size();
	for( int j=0; j<nPosCnt; j++ )
	{
		D3DXVECTOR3* PathPos = &(pNearestPath->pathData.vecPosPool[j]);
		D3DXVECTOR3 Dir;
		D3DXVec3Subtract( &Dir, PathPos, &Pos );
		float fDist = D3DXVec3Length( &Dir );

		if( nNearestPathIndex == -1 )
		{
			nNearestPathIndex = j;
			fMinPath = fDist;
		}
		else
		{
			if( fDist < fMinPath )
			{
				nNearestPathIndex = j;
				fMinPath = fDist;
			}
		}
	}
	BsAssert( pNearestPath );
	*nPathIndex = nNearestPathIndex;
	return pNearestPath;
}

bool CFcTroopAIObject::CanICmdMove( TroopObjHandle hTroop, PATH_PACK* pPath, int nMoveDirType, bool bLoop)
{
	BsAssert( pPath );
	if( bLoop )
	{
		return true;
	}
	
	D3DXVECTOR3 LastPos;
	if( nMoveDirType == Forward )
	{
		int nCnt = pPath->pathData.vecPosPool.size();
		BsAssert( nCnt > 0 );
		LastPos = pPath->pathData.vecPosPool[nCnt - 1];
	}
	else if( nMoveDirType == Backward )
	{
		int nCnt = pPath->pathData.vecPosPool.size();
		BsAssert( nCnt > 0 );
		LastPos = pPath->pathData.vecPosPool[0];
	}
	else
	{
		BsAssert( 0 );
	}
		
	D3DXVECTOR2 Dir = hTroop->GetPosV2() - D3DXVECTOR2(LastPos.x, LastPos.z);
	if( D3DXVec2Length( &Dir ) < 200.f )
	{
		return false;
	}
	return true;
}


bool CFcTroopAIObject::IsMovableAttr( float fX, float fY, int nRadius )
{
	int nX = (int)(fX / 100.f) - nRadius;
	int nY = (int)(fY / 100.f) - nRadius;

	int nCnt = nRadius * 2 + 1;
	for( int i=0; i<nCnt; i++ )
	{
		for( int j=0; j<nCnt; j++ )
		{
			if( g_FcWorld.GetAttr( nX + i, nY + j ) & 3 )
				return false;
		}
	}
	return true;
}


bool CFcTroopAIObject::IsOnPathState()
{
	TROOPSTATE State = m_hTroop->GetState();
	if( State == TROOPSTATE_MOVE_PATH ||
		State == TROOPSTATE_MOVE_PATH_LOOP ||
		State == TROOPSTATE_MOVE_FULL_PATH )
	{
		return true;
	}
	return false;
}