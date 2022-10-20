#include "StdAfx.h"
#include "FcAIObject.h"
#include "CrossVector.h"
#include "Parser.h"
#include "BsKernel.h"
#include "BStreamext.h"

#include "FcWorld.h"
#include "FcGlobal.h"
#include "FcAIElement.h"
#include "FcAISearch.h"
#include "FcAISearchGameObject.h"
#include "FcAISearchProp.h"
#include "FcAIFunc.h"
#include "FcAISearchSlot.h"

#include "FcAITriggerBase.h"
#include "FcAITriggerFactory.h"
#include "FcParamVariable.h"
#include "FcAIGlobalVariableMng.h"
#include "FcHorseObject.h"
#include "FcAIGlobalParam.h"

#include "FcTroopObject.h"

#include "FcAIHardCodingFunction.h"

#include "DebugUtil.h"
#include "PerfCheck.h"

#include "FcGlobal.h"
#include "FcAniObject.h"

#include <io.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

CSmartPtrMng<CFcAIObject> CFcAIObject::s_ObjectMng(DEFAULT_AI_OBJECT_POOL_SIZE);

DWORD CFcAIObject::s_dwLocalTick = 0;
DWORD CFcAIObject::s_dwTickIntervalOffset = 0;
char CFcAIObject::s_szErrorMessage[512] = {0,};
std::vector<CFcAIObject *> CFcAIObject::s_pVecEnableList;
std::vector<CFcAIObject *> CFcAIObject::s_pVecDisableList;
std::vector<CFcAIObject *> CFcAIObject::s_pVecFunctionList;
std::vector<CFcAIObject *> CFcAIObject::s_pVecPostEnableList;
std::vector<CFcAIObject *> CFcAIObject::s_pVecPostDisableList;

CFcAIObject::CFcAIObject()
{
// 이거 어케 초기화하지?
	m_hUnitObject.Identity();
	m_pGlobalVariableMng = NULL;
	m_bIsFunction = false;
	m_pSearchSlot = NULL;
	m_dwTickInterval = 10;
	m_bReset = false;
	m_bEnable = true;
	m_nDestroyTick = -1;
	m_nForceProcess = -1;
	m_bPreLoad = false;
	m_nLoadLevel = -1;
	m_nLoadModeType = -1;
	m_bCallbackFunc = false;
	memset( m_bCallbackProcess, 0, sizeof(m_bCallbackProcess) );

#ifdef _XBOX
	memset( m_szFileName, 0, sizeof(m_szFileName) );
#endif //_XBOX
}

CFcAIObject::~CFcAIObject()
{
	if( m_pSearchSlot && m_hUnitObject ) {
		GameObjHandle hTarget;
		CFcAISearchSlot::SlotStruct *pStruct; 
		CFcCorrelationMng *pCorMng;
		for( DWORD i=0; i<m_pSearchSlot->GetSlotCount(); i++ ) {
			pStruct = m_pSearchSlot->GetSlotFromIndex(i);
			if( pStruct == NULL ) continue;
			if( pStruct->nType != 0 ) continue;

			hTarget = pStruct->Handle;
			if( !CFcBaseObject::IsValid( hTarget ) ) continue;

			pCorMng = hTarget->GetCorrelationMng();
			if( pCorMng == NULL ) continue;
			CFcCorrelationTargeting *pCorrelation = (CFcCorrelationTargeting *)pCorMng->GetCorrelation( CFcCorrelation::CT_TARGETING_COUNTER );
			if( pCorrelation == NULL ) continue;

			pCorrelation->InfluenceTargeting( &m_hUnitObject, false );
		}
	}
	// 여기서 한번 검사하는 이유!!
	// CFcAIFunc 을 원래는 CreateObject<> 로 생성( SmartPtrMng 를 통해서 )해야하지만
	// Process 에서 루프 카운트를 줄이기 위해서 new 로 생성했귀땜쉬
	// Handle 값을 가지구 있지 않타!!
	if( CFcAIObject::s_ObjectMng[m_Handle.GetIndex()] == m_Handle )
		CFcAIObject::s_ObjectMng.DeleteHandle( m_Handle.GetHandle() );

	if( m_bEnable == true ) {
		for( DWORD i=0; i<s_pVecEnableList.size(); i++ ) {
			if( s_pVecEnableList[i] == this ) {
				s_pVecEnableList.erase( s_pVecEnableList.begin() + i );
				break;
			}
		}
	}
	else {
		for( DWORD i=0; i<s_pVecDisableList.size(); i++ ) {
			if( s_pVecDisableList[i] == this ) {
				s_pVecDisableList.erase( s_pVecDisableList.begin() + i );
				break;
			}
		}
	}


	Reset();
}

void CFcAIObject::Reset()
{
	m_bReset = true;
	SAFE_DELETE( m_pSearchSlot );
	SAFE_DELETE_PVEC( m_pVecElement );
	for( int i=0; i<AI_CALLBACK_NUM; i++ ) {
		SAFE_DELETE_PVEC( m_pVecCallbackElement[i] );
	}
	SAFE_DELETE_PVEC( m_pVecFunc );
	SAFE_DELETE( m_pGlobalVariableMng );
	SAFE_DELETE_VEC( m_bVecFuncUse );
}

void CFcAIObject::SetHandle( AIObjHandle Handle )
{
	m_Handle = Handle;

}
#define ELEMENT_DESTROY_RETURN_VALUE 0x000000ff
#define ELEMENT_CANCEL_RETURN_VALUE 0x000000fa


//#include "FcHeroObject.h"
void CFcAIObject::Process()
{
	if( m_pSearchSlot )
		m_pSearchSlot->Process();

	// 콜백 AI Element Process

	if( m_bIsFunction == false ) {
		PROFILE_TIME_TEST( ProcessCallbackElement() );
		if( m_nDestroyTick != -1 ) --m_nDestroyTick;

		if( m_nForceProcess != -1 ) {
			--m_nForceProcess;
			if( m_nForceProcess == -1 ) m_dwTickInterval = 1;
		}
		if( ( m_dwCreateTick + s_dwLocalTick ) % m_dwTickInterval != 0 ) return;

		if( m_nDestroyTick == -1 ) ProcessLOD();
	}
	else {
		((CFcAIFunc*)this)->m_dwTickInterval = ((CFcAIFunc*)this)->GetParent()->m_dwTickInterval;
	}

	int nResult;
	for( DWORD i=0; i<m_pVecElement.size(); i++ ) {
		CheckParentType( m_pVecElement[i] );
		// 병렬
		if( !m_pVecElement[i]->IsActivate() ) continue;
		nResult = m_pVecElement[i]->Process();
		switch( (nResult&0x0000ffff) ) {
			case ELEMENT_DESTROY_RETURN_VALUE:
				{
					short int nIndex = (nResult&0xffff0000) >> 16;
					if( nIndex == -1 ) {
						m_pVecElement[i]->SetActivate( false );
					}
					else {
						if( nIndex >= (int)m_pVecElement.size() ) break;
						m_pVecElement[nIndex]->SetActivate( false );
					}
				}
				break;
			case ELEMENT_CANCEL_RETURN_VALUE:
				return;
		}
	}
}

CFcAIObject *CFcAIObject::IsExist( const char *szFileName, int nLevel, bool bEnemy )
{
	int nSize = CFcAIObject::s_ObjectMng.Size();
	for( int i=0; i<nSize; i++ ) {
		if( CFcAIObject::s_ObjectMng[i] ) {
			if( CFcAIObject::s_ObjectMng[i] == this ) continue;
			if( CFcAIObject::s_ObjectMng[i]->m_bReset == true ) continue;
			if( strcmp( CFcAIObject::s_ObjectMng[i]->GetFileName(), szFileName ) == 0 ) {
				if( nLevel != -1 && CFcAIObject::s_ObjectMng[i]->IsBranchLevel() && nLevel != CFcAIObject::s_ObjectMng[i]->m_nLoadLevel ) continue;
				if( CFcAIObject::s_ObjectMng[i]->IsBranchMode() && (int)bEnemy != CFcAIObject::s_ObjectMng[i]->m_nLoadModeType ) continue;
				return CFcAIObject::s_ObjectMng[i];
			}
		}
	}
	return NULL;
}

bool CFcAIObject::Initialize( const char *szFileName, GameObjHandle hUnitObject, bool bPreLoad )
{
	PROFILE_FUNC( "Initialize AI" );
	if( szFileName == NULL ) return false;

//	CProfileMemChecker::GetInstance().StartBlock( "CFcAIObject::Initialize" );
	// 파일 이름을 가지구 있게한다.
	m_hUnitObject = hUnitObject;
	m_hCurUnitObject = m_hUnitObject;
#ifdef _XBOX
	sprintf( m_szFileName, szFileName );
#else
	m_szFileName = szFileName;
#endif
	m_bPreLoad = bPreLoad;
	if( m_bIsFunction == false ) {
		m_pSearchSlot = new CFcAISearchSlot( this );

		if( !bPreLoad ) {
			m_dwCreateTick = s_dwTickIntervalOffset;
			s_dwTickIntervalOffset++;
			if( s_dwTickIntervalOffset > MAX_LOD_INTERVAL )
				s_dwTickIntervalOffset = 0;
		}
	}

	CFcAIObject *pExistAIObject = IsExist( szFileName, ( m_bIsFunction == true ) ? -1 : hUnitObject->GetLevel(), hUnitObject->GetTeam() != 0 );
	m_bReset = false;
	// 같은 파일이 있을경우
	// 복사하구 끝낸다.
	if( pExistAIObject ) {
		if( pExistAIObject->IsFunction() == true )
			*(CFcAIFunc*)this = *(CFcAIFunc*)pExistAIObject;
		else {
			if( !bPreLoad ) {
				*this = *pExistAIObject;
				s_pVecEnableList.push_back( this );
			}
			else {
//				CProfileMemChecker::GetInstance().CancleBlock();
				return false;
				/*
				s_pVecDisableList.push_back( this );
				m_bEnable = false;
				*/
			}
		}
//		CProfileMemChecker::GetInstance().EndBlock();
		return true;
	}

	// 아니면 읽는다.

	bool bResult = LoadScript( szFileName );
	if( bResult == false ) {
#ifndef _LTCG
		CFcAIObject::s_szErrorMessage[ strlen(CFcAIObject::s_szErrorMessage) ] = '\n';
		g_BsKernel.AddConsoleString( CFcAIObject::s_szErrorMessage );
		DebugString( CFcAIObject::s_szErrorMessage );
#endif //_LTCG
		if( m_bIsFunction == false )
			Reset();
	}
	else {
		if( !m_bIsFunction ) {
			if( !bPreLoad )
				s_pVecEnableList.push_back( this );
			else s_pVecDisableList.push_back( this );
		}
		else s_pVecFunctionList.push_back( this );
	}
//	CProfileMemChecker::GetInstance().EndBlock();
	return bResult;
}

bool CFcAIObject::Reinitialize()
{
	m_bIsFunction = false;
	m_dwTickInterval = 10;
	Reset();
#ifdef _XBOX
	return Initialize( m_szFileName, m_hUnitObject );
#else
	return Initialize( m_szFileName.c_str(), m_hUnitObject );
#endif //_XBOX
}

const char *CFcAIObject::GetFileName()
{
#ifdef _XBOX
	return m_szFileName;
#else
	return m_szFileName.c_str();
#endif
}

void CFcAIObject::InitParser( Parser *pParser )
{
	pParser->ReserveKeyword("AI_ELEMENT");
	pParser->ReserveKeyword("AI_ELEMENT_END");
	pParser->ReserveKeyword("AI_ELEMENT_PROCESS_TYPE");
	pParser->ReserveKeyword("AI_ELEMENT_PARENT");
	pParser->ReserveKeyword("AI_END");
	pParser->ReserveKeyword("AI_CON");
	pParser->ReserveKeyword("AI_CON_END");
	pParser->ReserveKeyword("AI_ACTION");
	pParser->ReserveKeyword("AI_ACTION_END");
	pParser->ReserveKeyword("AI_ACTION_ELSE");
	pParser->ReserveKeyword("AI_ACTION_ELSE_END");
	pParser->ReserveKeyword("AI_FUNC");
	pParser->ReserveKeyword("AI_FUNC_END");
	pParser->ReserveKeyword("AI_SEARCH");
	pParser->ReserveKeyword("AI_SEARCH_END");
	pParser->ReserveKeyword("AI_SEARCH_TICK");
	pParser->ReserveKeyword("AI_SEARCH_PROP");

	pParser->ReserveKeyword("AI_GLOBAL_VARIABLE");
	pParser->ReserveKeyword("AI_GLOBAL_VARIABLE_END");

	pParser->ReserveKeyword("TRUE");
	pParser->ReserveKeyword("FALSE");

	pParser->ReserveKeyword("include_ai");

	// Tool Skip
	pParser->ReserveKeyword("AI_DESCRIPTION");
	pParser->ReserveKeyword("AI_DESCRIPTION_END");
	pParser->ReserveKeyword("AI_PARAMETER_DESCRIPTION");
	pParser->ReserveKeyword("AI_PARAMETER_DESCRIPTION_END");

	// Level
	pParser->ReserveKeyword("iflevel");
	pParser->ReserveKeyword("endlevel");

	// Friend, Enemy
	pParser->ReserveKeyword("iffriend");
	pParser->ReserveKeyword("endfriend");
	pParser->ReserveKeyword("ifenemy");
	pParser->ReserveKeyword("endenemy");


	for( DWORD i=0; i<CFcAIGlobalParam::GetInstance().GetGlobalParamCount(); i++ ) {
		pParser->ReserveKeyword( CFcAIGlobalParam::GetInstance().GetGlobalParamString(i) );
	}
}


bool CFcAIObject::LoadScript( const char *szFileName )
{
	PROFILE_FUNC( "LoadScript" );
	char szBinFileName[MAX_PATH];
	char szScriptFileName[MAX_PATH];
	sprintf( szBinFileName, "%sAI\\Unit\\%sb", g_BsKernel.GetCurrentDirectory(), szFileName );
	sprintf( szScriptFileName, "%sAI\\Unit\\%s", g_BsKernel.GetCurrentDirectory(), szFileName );

	TokenBuffer tb;
//	int nWriteTime = -1;

//	nWriteTime = GetFileTime( g_BsKernel.GetFullName( szScriptFileName ) );

// 헤더가 바껴도 갱신되지 않는 문제가 있다!!

	char* pFullName = g_BsKernel.GetFullName( szBinFileName );
	g_BsResChecker.AddResList( pFullName );

	if( tb.Load( pFullName ) == false ) {
		if( m_hUnitObject ) {
			char szStr[256];
			sprintf( szStr, "AI 파일 못찾음!! 맵이름 : [%s], AI이름 : [%s]\n", 
				g_FcWorld.GetMapFileName(), szFileName );
			DebugString( szStr );
		}
//		BsAssert( 0 && "Can't open Ai File" );
		return false;
		/*
		Parser parser;
		TokenList toklist;

		parser.Create();
		parser.EnableComments( true );

		InitParser( &parser );

		char cDir[MAX_PATH];
		sprintf( cDir, "%sAI\\Unit\\",  g_BsKernel.GetCurrentDirectory() );

		if( parser.ProcessSource( cDir, szFileName, &toklist ) == false ) {
			return false;
		}

		parser.SetDirectory( g_BsKernel.GetCurrentDirectory() );		// \data
		if( parser.ProcessHeaders( &toklist ) == false ) {
			return false;
		}

		parser.SetDirectory( cDir );

		parser.ProcessMacros( &toklist );

		tb.Initialize( &toklist );
		tb.Save( g_BsKernel.GetFullName( szBinFileName ), nWriteTime );
		*/
	}

	CFcAIElement *pElement = NULL;
	CFcAISearch *pAISearch = NULL;
	int nLevel = m_hUnitObject->GetLevel();
	int nMode = ( m_hUnitObject->GetTeam() == 0 ) ? 0 : 1;
	int nDefineLevel = -1;
	int nDefineMode = -1;
	bool bBranchLevel = false;
	bool bBranchMode = false;


	while( 1 ) {
		if( tb.IsEnd() ) break;

		while( 1 )		// 파서가 헤더 분석을 잘못해서 #이 들어가는 경우가 있음 -> #define ConvertToString(x) #x
		{
			if( tb.IsKeyword() && strcmp( tb.GetKeyword(), "define" ) != 0 )
			{
				break;
			}
			++tb;
		}

		if( tb.IsOperator() ) {
			tb++;
			continue;
		}
		if( !tb.IsKeyword() )
		{
			strcpy( CFcAIObject::s_szErrorMessage, tb.GetDescriptiveString().c_str() );
			return false;
			BsAssert(0);
		}

		// 상속 받은 클래스를 위한..
		ProcessParser( tb );

		if( !tb.IsKeyword() )
		{
			strcpy( CFcAIObject::s_szErrorMessage, tb.GetDescriptiveString().c_str() );
			return false;
			BsAssert(0);
		}
		if( strcmp( tb.GetKeyword(), "iflevel" ) == 0 ) {
			tb++;
			nDefineLevel = tb.GetInteger();
			bBranchLevel = true;
			tb++;
		}
		else if( strcmp( tb.GetKeyword(), "endlevel" ) == 0 ) {
			nDefineLevel = -1;
			tb++;
		}
		else if( strcmp( tb.GetKeyword(), "iffriend" ) == 0 ) {
			nDefineMode = 0;
			bBranchMode = true;
			tb++;
		}
		else if( strcmp( tb.GetKeyword(), "endfriend" ) == 0 ) {
			nDefineMode = -1;
			tb++;
		}
		else if( strcmp( tb.GetKeyword(), "ifenemy" ) == 0 ) {
			nDefineMode = 1;
			bBranchMode = true;
			tb++;
		}
		else if( strcmp( tb.GetKeyword(), "endenemy" ) == 0 ) {
			nDefineMode = -1;
			tb++;
		}

		if( nDefineLevel != -1 && nDefineLevel != nLevel ) {
			tb++;
			continue;
		}
		if( nDefineMode != -1 && nDefineMode != nMode ) {
			tb++;
			continue;
		}

		if( strcmp( tb.GetKeyword(), "include_ai" ) == 0 ) {
			tb++;
			if( !tb.IsString() ) {
				strcpy( CFcAIObject::s_szErrorMessage, tb.GetDescriptiveString().c_str() );
				return false;
			}
			CFcAIFunc *pAIFunc = new CFcAIFunc;

#ifdef _XBOX
			strcpy( pAIFunc->m_szFileName, tb.GetString() );//aleksger - safe string
#else
			pAIFunc->m_szFileName = tb.GetString();
#endif //_XBOX
			// 하드코딩 펑션 체크
			int nHCFIndex = CFcAIHardCodingFunction::FindFunction( pAIFunc->GetFuncString(), nLevel );
			if( nHCFIndex == -1 ) {
				pAIFunc->Initialize( tb.GetString(), m_hUnitObject, m_bPreLoad );
				pAIFunc->SetParent( this );
			}
			else {
				pAIFunc->SetParent( this );
				pAIFunc->SetUnitObjectHandle( m_hUnitObject );
				pAIFunc->SetHardCodingFunctionIndex( nHCFIndex );

			}
			m_pVecFunc.push_back( pAIFunc );
			m_bVecFuncUse.push_back( FALSE );
			tb++;
		}
		else if( strcmp( tb.GetKeyword(), "AI_ELEMENT_PARENT" ) == 0 ) {
			tb++;
			if( tb.IsInteger() ) {
				pElement->SetParentType( (AI_ELEMENT_PARENT)tb.GetInteger() );
			}
			tb++;
		}
		else if( strcmp( tb.GetKeyword(), "AI_FUNC" ) == 0 ) {
			tb++;
			while(1) {
				if( tb.IsKeyword() ) {
					if( strcmp( tb.GetKeyword(), "AI_FUNC_END" ) == 0 ) {
						break;
					}
					else if( strcmp( tb.GetKeyword(), "iflevel" ) == 0 ) {
						tb++;
						nDefineLevel = tb.GetInteger();
						bBranchLevel = true;
						tb++;
						continue;
					}
					else if( strcmp( tb.GetKeyword(), "endlevel" ) == 0 ) {
						nDefineLevel = -1;
						tb++;
						continue;
					}
					else if( strcmp( tb.GetKeyword(), "iffriend" ) == 0 ) {
						nDefineMode = 0;
						bBranchMode = true;
						tb++;
						continue;
					}
					else if( strcmp( tb.GetKeyword(), "endfriend" ) == 0 ) {
						nDefineMode = -1;
						tb++;
						continue;
					}
					else if( strcmp( tb.GetKeyword(), "ifenemy" ) == 0 ) {
						nDefineMode = 1;
						bBranchMode = true;
						tb++;
						continue;
					}
					else if( strcmp( tb.GetKeyword(), "endenemy" ) == 0 ) {
						nDefineMode = -1;
						tb++;
						continue;
					}

				}
				if( tb.IsOperator() && strcmp( tb.GetOperator(), "#" ) == 0 ) {
					tb++;
					continue;
				}
				if( nDefineLevel != -1 && nDefineLevel != nLevel ) {
					tb++;
					continue;
				}
				if( nDefineMode != -1 && nDefineMode != nMode ) {
					tb++;
					continue;
				}

				if( !tb.IsVariable() ) {
					strcpy( CFcAIObject::s_szErrorMessage, tb.GetDescriptiveString().c_str() );
					return false;
				}
				int nIndex = GetFunctionIndex( tb.GetVariable() );
				if( nIndex == -1 ) {
					strcpy( CFcAIObject::s_szErrorMessage, tb.GetDescriptiveString().c_str() );
					return false;
					BsAssert(0);
				}
				CFcAIFuncParam *pFunc = new CFcAIFuncParam( m_pVecFunc[nIndex], nIndex );
				int nCount = m_pVecFunc[nIndex]->GetParamCount();

				tb++;
				for( int i=0; i<nCount; i++ ) {
					CFcParamVariable *pParam = NULL;
					if( m_pVecFunc[nIndex]->GetHardCodingFunctionIndex() == -1 ) {
						pParam = new CFcParamVariable;
						*pParam = *m_pVecFunc[nIndex]->GetParam(i);
						SetExtensionVariableFromPV( pParam, tb );
						tb++;
					}
					else {
						if( SetExtensionVariable( &pParam, tb ) == false ) {
							i--;
							tb++;
							continue;
						}
						else tb++;
					}

					pFunc->AddParam( pParam );
				}
				pFunc->Initialize();
				pElement->AddFunction( pFunc );
			}
		}
		else if( strcmp( tb.GetKeyword(), "AI_FUNC_END" ) == 0 ) {
			tb++;
		}
		else if( strcmp( tb.GetKeyword(), "AI_GLOBAL_VARIABLE" ) == 0 ) {
			tb++;

			m_pGlobalVariableMng = new CFcAIGlobalVariableMng( m_Handle );

			while(1) {
				CFcParamVariable *pVariable = NULL;
				if( tb.IsKeyword() ) {
					if( strcmp( tb.GetKeyword(), "AI_GLOBAL_VARIABLE_END" ) == 0 ) {
						break;
					}
					else if( strcmp( tb.GetKeyword(), "iflevel" ) == 0 ) {
						tb++;
						nDefineLevel = tb.GetInteger();
						bBranchLevel = true;						
						tb++;
						continue;
					}
					else if( strcmp( tb.GetKeyword(), "endlevel" ) == 0 ) {
						nDefineLevel = -1;
						tb++;
						continue;
					}
					else if( strcmp( tb.GetKeyword(), "iffriend" ) == 0 ) {
						nDefineMode = 0;
						bBranchMode = true;
						tb++;
						continue;
					}
					else if( strcmp( tb.GetKeyword(), "endfriend" ) == 0 ) {
						nDefineMode = -1;
						tb++;
						continue;
					}
					else if( strcmp( tb.GetKeyword(), "ifenemy" ) == 0 ) {
						nDefineMode = 1;
						bBranchMode = true;
						tb++;
						continue;
					}
					else if( strcmp( tb.GetKeyword(), "endenemy" ) == 0 ) {
						nDefineMode = -1;
						tb++;
						continue;
					}
				}
				if( nDefineLevel != -1 && nDefineLevel != nLevel ) {
					tb++;
					continue;
				}
				if( nDefineMode != -1 && nDefineMode != nMode ) {
					tb++;
					continue;
				}
				if( tb.IsOperator() && strcmp( tb.GetOperator(), "#" ) == 0 ) {
					tb++;
					continue;
				}

				if( !tb.IsString() ) {
					strcpy( CFcAIObject::s_szErrorMessage, tb.GetDescriptiveString().c_str() );
					return false;
				}

				const char *szDesc = tb.GetString();
				tb++;
				if( !tb.IsInteger() ) {
					strcpy( CFcAIObject::s_szErrorMessage, tb.GetDescriptiveString().c_str() );
					return false;
				}

				int nVariableType = tb.GetInteger();
				tb++;
				pVariable = new CFcParamVariable( (CFcParamVariable::VT)nVariableType );
				pVariable->SetDescription( szDesc );

				SetExtensionVariableFromPV( pVariable, tb );

				tb++;
				m_pGlobalVariableMng->AddVariable( pVariable );
			}
			tb++;
		}

		else if( strcmp( tb.GetKeyword(), "AI_END" ) == 0 )		// 끝
		{
			break;
		}
		else if( strcmp( tb.GetKeyword(), "AI_ELEMENT" ) == 0 )
		{
			pElement = new CFcAIElement( this );
			tb++;
		}
		else if( strcmp( tb.GetKeyword(), "AI_ELEMENT_END" ) == 0 )
		{
			switch( pElement->GetProcessType() ) {
				case AI_ELEMENT_PROCESS_CALLBACK:
					m_pVecCallbackElement[pElement->GetProcessTypeParam()].push_back( pElement );
					m_bCallbackFunc = true;
					break;
				default:
					m_pVecElement.push_back( pElement );
					break;
			}
			tb++;
		}
		else if( strcmp( tb.GetKeyword(), "AI_ELEMENT_PROCESS_TYPE" ) == 0 ) {
			tb++;
			if( !tb.IsInteger() ) {
				strcpy( CFcAIObject::s_szErrorMessage, tb.GetDescriptiveString().c_str() );
				return false;
			}
			int nProcessType = tb.GetInteger();

			tb++;
			if( !tb.IsInteger() ) {
				strcpy( CFcAIObject::s_szErrorMessage, tb.GetDescriptiveString().c_str() );
				return false;
			}

			int nParam = tb.GetInteger();
			pElement->SetProcessType( (AI_ELEMENT_PROCESS_TYPE)nProcessType, nParam );
			tb++;
		}
		else if( strcmp( tb.GetKeyword(), "AI_SEARCH" ) == 0 || strcmp( tb.GetKeyword(), "AI_SEARCH_PROP" ) == 0 ) {

			if( strcmp( tb.GetKeyword(), "AI_SEARCH" ) == 0 ) 
				pAISearch = new CFcAISearchGameObject( this );
			else if( strcmp( tb.GetKeyword(), "AI_SEARCH_PROP" ) == 0 )
				pAISearch = new CFcAISearchProp( this );
			else {
				int asdf = 0;
			}

			tb++;
			while(1) {
				if( tb.IsKeyword() ) {
					if( strcmp( tb.GetKeyword(), "AI_SEARCH_END" ) == 0 ) {
						pElement->SetSearch( pAISearch );
						break;
					}
					if( strcmp( tb.GetKeyword(), "AI_SEARCH_TICK" ) == 0 ) {
						tb++;
						if( !tb.IsInteger() ) {
							strcpy( CFcAIObject::s_szErrorMessage, tb.GetDescriptiveString().c_str() );
							return false;
						}

						pAISearch->SetProcessTick( tb.GetInteger() );
						tb++;
						continue;
					}
					else if( strcmp( tb.GetKeyword(), "iflevel" ) == 0 ) {
						tb++;
						nDefineLevel = tb.GetInteger();
						bBranchLevel = true;
						tb++;
						continue;
					}
					else if( strcmp( tb.GetKeyword(), "endlevel" ) == 0 ) {
						nDefineLevel = -1;
						tb++;
						continue;
					}
					else if( strcmp( tb.GetKeyword(), "iffriend" ) == 0 ) {
						nDefineMode = 0;
						bBranchMode = true;
						tb++;
						continue;
					}
					else if( strcmp( tb.GetKeyword(), "endfriend" ) == 0 ) {
						nDefineMode = -1;
						tb++;
						continue;
					}
					else if( strcmp( tb.GetKeyword(), "ifenemy" ) == 0 ) {
						nDefineMode = 1;
						bBranchMode = true;
						tb++;
						continue;
					}
					else if( strcmp( tb.GetKeyword(), "endenemy" ) == 0 ) {
						nDefineMode = -1;
						tb++;
						continue;
					}
				}
				else if( tb.IsInteger() ) {
					if( nDefineLevel != -1 && nDefineLevel != nLevel ) {
						tb++;
						continue;
					}
					if( nDefineMode != -1 && nDefineMode != nMode ) {
						tb++;
						continue;
					}

					int nID = tb.GetInteger();
					if( (nID & AI_SEARCH_TYPE_MASK) > 0 ) {
						pAISearch->SetType( (AI_SEARCH_TYPE)(nID & AI_SEARCH_TYPE_MASK) );
					}
					else if( (nID & AI_SEARCH_CON_MASK) > 0 ) {
						pAISearch->AddCondition( (AI_SEARCH_CON)(nID & AI_SEARCH_CON_MASK) );
					}
					else if( (nID & AI_SEARCH_FILTER_MASK) > 0 ) {
						pAISearch->AddFilter( (AI_SEARCH_FILTER)(nID & AI_SEARCH_FILTER_MASK) );
					}
					tb++;

					int nCount = GetAISearchParamNum( nID );
					for( int i=0; i<nCount; i++ ) {
						CFcParamVariable *pParam = NULL;

						if( SetExtensionVariable( &pParam, tb ) == false ) {
							strcpy( CFcAIObject::s_szErrorMessage, tb.GetDescriptiveString().c_str() );
							return false;
							BsAssert( 0 && "Unknown Variable Type" );
						}

						tb++;
//						if( tb.IsOperator() ) tb++;
						if( tb.IsOperator() && strcmp( tb.GetOperator(), "," ) == NULL ) tb++;

						pAISearch->AddParam( pParam );
					}
				}
				else tb++;
			}

			tb++;
		}
		else if( strcmp( tb.GetKeyword(), "AI_CON" ) == 0 )
		{
			tb++;

			while(1)
			{
				CFcAIConditionBase *pBase = NULL;

				if( tb.IsKeyword() ) {
					if( strcmp( tb.GetKeyword(), "AI_CON_END" ) == 0 )
						break;
					else if( strcmp( tb.GetKeyword(), "iflevel" ) == 0 ) {
						tb++;
						nDefineLevel = tb.GetInteger();
						bBranchLevel = true;
						tb++;
						continue;
					}
					else if( strcmp( tb.GetKeyword(), "endlevel" ) == 0 ) {
						nDefineLevel = -1;
						tb++;
						continue;
					}
					else if( strcmp( tb.GetKeyword(), "iffriend" ) == 0 ) {
						nDefineMode = 0;
						bBranchMode = true;
						tb++;
						continue;
					}
					else if( strcmp( tb.GetKeyword(), "endfriend" ) == 0 ) {
						nDefineMode = -1;
						tb++;
						continue;
					}
					else if( strcmp( tb.GetKeyword(), "ifenemy" ) == 0 ) {
						nDefineMode = 1;
						bBranchMode = true;
						tb++;
						continue;
					}
					else if( strcmp( tb.GetKeyword(), "endenemy" ) == 0 ) {
						nDefineMode = -1;
						tb++;
						continue;
					}
				}
				if( nDefineLevel != -1 && nDefineLevel != nLevel ) {
					tb++;
					continue;
				}
				if( nDefineMode != -1 && nDefineMode != nMode ) {
					tb++;
					continue;
				}
				if( tb.IsOperator() && strcmp( tb.GetOperator(), "#" ) == 0 ) {
					tb++;
					continue;
				}

				if( !tb.IsInteger() ) {
					strcpy( CFcAIObject::s_szErrorMessage, tb.GetDescriptiveString().c_str() );
					return false;
				}
				int nID = tb.GetInteger(); 

				CFcAISearch::SEARCHER_TYPE SearcherType = CFcAISearch::GAMEOBJECT;
				if( pAISearch ) SearcherType = pAISearch->GetSearcherType();

				pBase = CFcAIConditionBase::CreateObject( nID, SearcherType );
				if( pBase == NULL ) {
					strcpy( CFcAIObject::s_szErrorMessage, tb.GetDescriptiveString().c_str() );
					return false;
				}
				pBase->SetParent( pElement );

				tb++;

				int nCount = GetAIConParamNum( nID );
				for( int i=0; i<nCount; i++ ) {
					CFcParamVariable *pParam = NULL;

					SAFE_DELETE_VEC( m_pVecTempOperatorParam );
					SAFE_DELETE_VEC( m_nVecTempOperator );
					if( SetExtensionVariable( &pParam, tb ) == false ) {
						strcpy( CFcAIObject::s_szErrorMessage, tb.GetDescriptiveString().c_str() );
						return false;
						BsAssert( 0 && "Unknown Variable Type" );
					}

					tb++;
//					if( tb.IsOperator() ) tb++;
					if( tb.IsOperator() && strcmp( tb.GetOperator(), "," ) == NULL ) tb++;

					pBase->AddParam( pParam );
					// 오퍼레이터가 있을 경우다!!
					if( pParam == NULL ) {
						pBase->AddOperatorParam( m_pVecTempOperatorParam, m_nVecTempOperator );
					}
				}
				pElement->AddCondition( pBase );
			}

			tb++;
		}
		else if( strcmp( tb.GetKeyword(), "AI_ACTION" ) == 0 || 
			strcmp( tb.GetKeyword(), "AI_ACTION_ELSE" ) == 0 )
		{
			bool bIfElseFlag = true;
			if( strcmp( tb.GetKeyword(), "AI_ACTION_ELSE" ) == 0 ) bIfElseFlag = false;

			tb++;

			while(1)
			{
				CFcAIActionBase *pBase = NULL;

				if( tb.IsKeyword() )
				{
					if( strcmp( tb.GetKeyword(), "AI_ACTION_END" ) == 0 || 
						strcmp( tb.GetKeyword(), "AI_ACTION_ELSE_END" ) == 0 )
						break;
					else if( strcmp( tb.GetKeyword(), "iflevel" ) == 0 ) {
						tb++;
						nDefineLevel = tb.GetInteger();
						bBranchLevel = true;
						tb++;
						continue;
					}
					else if( strcmp( tb.GetKeyword(), "endlevel" ) == 0 ) {
						nDefineLevel = -1;
						tb++;
						continue;
					}
					else if( strcmp( tb.GetKeyword(), "iffriend" ) == 0 ) {
						nDefineMode = 0;
						bBranchMode = true;
						tb++;
						continue;
					}
					else if( strcmp( tb.GetKeyword(), "endfriend" ) == 0 ) {
						nDefineMode = -1;
						tb++;
						continue;
					}
					else if( strcmp( tb.GetKeyword(), "ifenemy" ) == 0 ) {
						nDefineMode = 1;
						bBranchMode = true;
						tb++;
						continue;
					}
					else if( strcmp( tb.GetKeyword(), "endenemy" ) == 0 ) {
						nDefineMode = -1;
						tb++;
						continue;
					}
				}
				if( nDefineLevel != -1 && nDefineLevel != nLevel ) {
					tb++;
					continue;
				}
				if( nDefineMode != -1 && nDefineMode != nMode ) {
					tb++;
					continue;
				}
				if( tb.IsOperator() && strcmp( tb.GetOperator(), "#" ) == 0 ) {
					tb++;
					continue;
				}

				if( !tb.IsInteger() ) {
					strcpy( CFcAIObject::s_szErrorMessage, tb.GetDescriptiveString().c_str() );
					return false;
				}
				int nID = tb.GetInteger();

				CFcAISearch::SEARCHER_TYPE SearcherType = CFcAISearch::GAMEOBJECT;
				if( pAISearch ) SearcherType = pAISearch->GetSearcherType();

				pBase = CFcAIActionBase::CreateObject( nID, SearcherType );
				if( pBase == NULL ) {
					strcpy( CFcAIObject::s_szErrorMessage, tb.GetDescriptiveString().c_str() );
					return false;
				}
				pBase->SetParent( pElement );

				tb++;

				int nCount = GetAIActionParamNum( nID );
				for( int i=0; i<nCount; i++ )
				{
					CFcParamVariable *pParam = NULL;

					SAFE_DELETE_VEC( m_pVecTempOperatorParam );
					SAFE_DELETE_VEC( m_nVecTempOperator );
					if( SetExtensionVariable( &pParam, tb ) == false ) {
						strcpy( CFcAIObject::s_szErrorMessage, tb.GetDescriptiveString().c_str() );
						return false;
						BsAssert( 0 && "Unknown Variable Type" );
					}

					tb++;
					if( tb.IsOperator() && strcmp( tb.GetOperator(), "," ) == NULL ) tb++;

					pBase->AddParam( pParam );
					// 오퍼레이터가 있을 경우다!!
					if( pParam == NULL ) {
						pBase->AddOperatorParam( m_pVecTempOperatorParam, m_nVecTempOperator );
					}
				}
				if( bIfElseFlag == true )
					pElement->AddAction( pBase );
				else pElement->AddActionElse( pBase );
			}

			tb++;
		}
		else if( strcmp( tb.GetKeyword(), "AI_DESCRIPTION" ) == 0 ||
			strcmp( tb.GetKeyword(), "AI_PARAMETER_DESCRIPTION" ) == 0 ) {
			while(1) {
				tb++;
				if( tb.IsKeyword() && ( strcmp( tb.GetKeyword(), "AI_DESCRIPTION_END" ) == 0 || strcmp( tb.GetKeyword(), "AI_PARAMETER_DESCRIPTION_END" ) == 0 ) ) break;
			}
			tb++;
		}
		else tb++;
	}

#ifndef _LTCG
	if( bBranchLevel == true ) {
		m_nLoadLevel = nLevel;
		_DebugString( "Load AI Info : Brunch [ %s ], ( Level : %d )", szFileName, nLevel );
	}
	else _DebugString( "Load AI Info : Not Brunch [ %s ]", szFileName );

	if( bBranchMode == true ) {
		m_nLoadModeType = nMode;
		_DebugString( "    Mode %s\n", (nMode == 0) ? "F" : "E" );
	}
	else _DebugString( "    Mode Shared\n" );
#endif //_LTCG

	return true;
}

bool CFcAIObject::SetExtensionVariable( CFcParamVariable **pParam, TokenBuffer &itr )
{
	if( itr.IsReal() )	{	
		*pParam = new CFcParamVariable( CFcParamVariable::FLOAT ); 
		(*pParam)->SetVariable( (float)itr.GetReal() );	
		return true;
	}
	else if( itr.IsInteger() ) {	
		*pParam = new CFcParamVariable( CFcParamVariable::INT ); 
		(*pParam)->SetVariable( (int)itr.GetInteger() );	
		return true;
	}
	else if( itr.IsBoolean() ) {
		*pParam = new CFcParamVariable( CFcParamVariable::BOOLEAN );
		(*pParam)->SetVariable( (bool)itr.GetBoolean() );
		return true;
	}
	else if( itr.IsKeyword() ) {
		if( strcmp( itr.GetKeyword(), "TRUE" ) == NULL || strcmp( itr.GetKeyword(), "FALSE" ) == NULL ) {
			*pParam = new CFcParamVariable( CFcParamVariable::BOOLEAN );
			if( strcmp( itr.GetKeyword(), "TRUE" ) == NULL )
				(*pParam)->SetVariable( true );
			else (*pParam)->SetVariable( false );
			return true;
		}
		else if( CFcAIGlobalParam::GetInstance().IsValidGlobalParam( itr.GetKeyword() ) == true ) {
			*pParam = new CFcParamVariable( CFcParamVariable::CUSTOM );
			(*pParam)->SetVariable( (char*)itr.GetKeyword() );
			return true;
		}
	}
	else if( itr.IsString() ) {	
		const char *szStr = itr.GetString();
		// !!!! 위험위험..
		// TriggerBase 와 꼭 가치 변경!!
		if( m_pGlobalVariableMng && m_pGlobalVariableMng->GetVariableIndex( szStr ) != -1 ) {
			*pParam = new CFcParamVariableEx( CFcParamVariable::STRING, m_pGlobalVariableMng ); 
			(*pParam)->SetVariable( (char*)itr.GetString() );	
		}
		else {
			*pParam = new CFcParamVariable( CFcParamVariable::STRING ); 
			(*pParam)->SetVariable( (char*)itr.GetString() );	
		}
		return true;
	}
	else if( itr.IsOperator() ) {
		if( strcmp( itr.GetOperator(), "(" ) == NULL ) {
			for( int i=0; ; i++ ) {
				itr++;
				CFcParamVariable *pTempValue = NULL;
				if( SetExtensionVariable( &pTempValue, itr ) == false ) {
					if( itr.IsOperator() ) {
						int nOperator = -1;
						if( strcmp( itr.GetOperator(), "+" ) == NULL ) nOperator = 0;
						else if( strcmp( itr.GetOperator(), "-" ) == NULL ) nOperator = 1;
						else if( strcmp( itr.GetOperator(), "*" ) == NULL ) nOperator = 2;
						else if( strcmp( itr.GetOperator(), "/" ) == NULL ) nOperator = 3;
						else if( strcmp( itr.GetOperator(), ")" ) == NULL ) break;
						if( nOperator != -1 ) {
							m_nVecTempOperator.push_back( nOperator );
						}
					}
				}
				else {
					m_pVecTempOperatorParam.push_back( pTempValue );
				}
			}
			// 널셋팅!!!
			(*pParam) = NULL;

			return true;
		}
	}
	return false;
}

void CFcAIObject::SetExtensionVariableFromPV( CFcParamVariable *pVariable, TokenBuffer &itr )
{
	while(1) {
		if( itr.IsOperator() == TRUE ) itr++;
		else {
			break;
		}
	}
	if( m_pGlobalVariableMng ) {
//		itr++;
		if( itr.IsString() ) {	// 함수 변수를 글로벌벨류로 사용했을경우에두 처리해줘야한다.
			int nIndex = m_pGlobalVariableMng->GetVariableIndex( itr.GetString() );
			if( nIndex != -1 ) {
				CFcParamVariable *pParam = m_pGlobalVariableMng->GetVariable( nIndex );
				*pVariable = *pParam;
				return;
			}
		}
//		itr--;
	}
	switch( pVariable->GetType() ) {
		case CFcParamVariable::INT:
//			itr++;
			pVariable->SetVariable( (int)itr.GetInteger() );
			break;
		case CFcParamVariable::FLOAT:
//			itr++;
			pVariable->SetVariable( (float)itr.GetReal() );
			break;
		case CFcParamVariable::CHAR:
//			itr++;
			pVariable->SetVariable( (char)itr.GetInteger() );
			break;
		case CFcParamVariable::STRING:
//			itr++;
			pVariable->SetVariable( (char*)itr.GetString() );
			break;
		case CFcParamVariable::VECTOR:
			{
				float fArray[3];
				int nCount = 0;
				while(1) {
					if( itr.IsOperator() ) {
						itr++;
						continue;
					}

					else if( itr.IsReal() ) {
						fArray[nCount] = itr.GetReal();
						nCount++;
					}
					else if( itr.IsInteger() ) {
						fArray[nCount] = (float)itr.GetInteger();
						nCount++;
					}
					if( nCount == 3 ) break;
					itr++;

				}

				D3DXVECTOR3 vVec = D3DXVECTOR3( fArray[0], fArray[1], fArray[2] );
				pVariable->SetVariable( vVec );
			}
			break;
		case CFcParamVariable::PTR:
//			itr++;
			pVariable->SetVariable( (void*)NULL );
			break;
		case CFcParamVariable::BOOLEAN:
			{
//				itr++;
				bool bFlag = false;
				if( strcmp( itr.GetKeyword(), "TRUE" ) == 0 ) bFlag = true;
				pVariable->SetVariable( (bool)bFlag );
			}
			break;
		case CFcParamVariable::RANDOM:
			{
				int nValue[2];
				int nCount = 0;
				while(1) {
					if( itr.IsOperator() ) {
						itr++;
						continue;
					}
					else if( itr.IsInteger() ) {
						nValue[nCount] = (int)itr.GetInteger();
						nCount++;
					}
					if( nCount == 2 ) break;
					itr++;

				}
				pVariable->SetVariable( nValue[0], nValue[1] );
			}
			break;
	}
}

int CFcAIObject::GetFunctionIndex( const char *szStr )
{
	for( DWORD i=0; i<m_pVecFunc.size(); i++ ) {
		if( strcmp( szStr, m_pVecFunc[i]->GetFuncString() ) == 0 ) {
			if( m_pVecFunc[i]->GetHardCodingFunctionIndex() != -1 ) {
				if( m_bVecFuncUse[i] == TRUE ) {
					CFcAIFunc *pAIFunc = new CFcAIFunc;
#ifdef _XBOX
					sprintf( pAIFunc->m_szFileName, m_pVecFunc[i]->GetFileName() );
#else
					pAIFunc->m_szFileName = m_pVecFunc[i]->GetFileName();
#endif //_XBOX
					int nLevel = CFcAIHardCodingFunction::GetFunction( m_pVecFunc[i]->GetHardCodingFunctionIndex() )->GetLevel();

					int nHCFIndex = CFcAIHardCodingFunction::FindFunction( pAIFunc->GetFuncString(), nLevel );
					if( nHCFIndex == -1 ) {
						BsAssert( 0 && "하드코드 펑션은 여러개 사용 시 복사필요!! 일루온건 이상함!" );
					}
					else {
						pAIFunc->SetParent( this );
						pAIFunc->SetUnitObjectHandle( m_hUnitObject );
						pAIFunc->SetHardCodingFunctionIndex( nHCFIndex );
					}
					m_pVecFunc.push_back( pAIFunc );
					m_bVecFuncUse.push_back( TRUE );
					return m_pVecFunc.size() - 1;
				}
			}
			m_bVecFuncUse[i] = TRUE;
			return (int)i;
		}
	}
	return -1;
}


// STATIC
void CFcAIObject::InitializeObjects()
{
	CFcAIObject::ReleaseObjects();

	CFcAIHardCodingFunction::CreateObject();
}

void CFcAIObject::ReinitializeObjects()
{
	CFcAIObject::s_dwLocalTick = 0;

	int nSize = CFcAIObject::s_ObjectMng.Size();
	for( int i=0; i<nSize; i++ ) {
		if( CFcAIObject::s_ObjectMng[i] ) {
			CFcAIObject::s_ObjectMng[i]->Reset();
		}
	}
	for( int i=0; i<nSize; i++ ) {
		if( CFcAIObject::s_ObjectMng[i] ) {
			CFcAIObject::s_ObjectMng[i]->Reinitialize();
		}
	}
}

// STATIC
void CFcAIObject::ReleaseObjects( bool bExitGame )
{
	int nSize = CFcAIObject::s_ObjectMng.Size();
	for( int i=0; i<nSize; i++ ) {
		if( CFcAIObject::s_ObjectMng[i] ) {
			delete CFcAIObject::s_ObjectMng[i];
		}
	}
	s_pVecEnableList.clear();
	s_pVecDisableList.clear();
	s_pVecFunctionList.clear();
	s_pVecPostEnableList.clear();
	s_pVecPostDisableList.clear();

	CFcAIHardCodingFunction::ReleaseObject( bExitGame );

	CFcAIObject::s_dwTickIntervalOffset = 0;
	CFcAISearch::s_nTickIntervalOffset = 0;
}

int CFcAIObject::GetPreloadIndex( AIObjHandle &Handle )
{
	int nSize = CFcAIObject::s_ObjectMng.Size();
	for( int i=0; i<nSize; i++ ) {
		if( CFcAIObject::s_ObjectMng[i] ) {
			if( CFcAIObject::s_ObjectMng[i]->m_bReset == true ) continue;
			if( CFcAIObject::s_ObjectMng[i]->m_bPreLoad == false ) continue;

			if( strcmp( CFcAIObject::s_ObjectMng[i]->GetFileName(), Handle->GetFileName() ) == 0 ) {
				if( Handle->m_nLoadLevel != -1 && CFcAIObject::s_ObjectMng[i]->IsBranchLevel() && Handle->m_nLoadLevel != CFcAIObject::s_ObjectMng[i]->m_nLoadLevel ) continue;
				if( Handle->m_nLoadModeType != -1 && CFcAIObject::s_ObjectMng[i]->IsBranchLevel() && Handle->m_nLoadLevel != CFcAIObject::s_ObjectMng[i]->m_nLoadLevel ) continue;
				return i;
			}
		}
	}
	return -1;
}

const char *CFcAIObject::GetPreloadName( int nIndex )
{
	return CFcAIObject::s_ObjectMng[nIndex]->GetFileName();
}

// STATIC
void CFcAIObject::ProcessObjects()
{
	/*
	if( g_FCGameData.bEnableUnitAI == false ) return;
	int nSize = CFcAIObject::s_ObjectMng.Size();
	s_dwLocalTick = GetProcessTick();

	GameObjHandle hUnitObject;
	for( int i=0; i<nSize; i++ )
	{
		if( CFcAIObject::s_ObjectMng[i] )
		{
			if( CFcAIObject::s_ObjectMng[i]->IsEnable() == false ) continue;
			hUnitObject = CFcAIObject::s_ObjectMng[i]->GetUnitObjectHandle();
			if( !hUnitObject || CFcAIObject::s_ObjectMng[i]->IsDestroyThis() == true )
			{
				AIObjHandle hIdentity;
				hUnitObject->SetAIHandle( hIdentity );
				delete CFcAIObject::s_ObjectMng[i];
				continue;
			}
			CFcAIObject::s_ObjectMng[i]->Process();
		}
	}
	*/
	if( g_FCGameData.bEnableUnitAI == false ) return;
	s_dwLocalTick = GetProcessTick();

	PostListProcess();

	GameObjHandle hUnitObject;
	int nSize = (int)s_pVecEnableList.size();
	for( int i=0; i<nSize; i++ )
	{
		hUnitObject = CFcAIObject::s_pVecEnableList[i]->GetUnitObjectHandle();
		if( !hUnitObject || CFcAIObject::s_pVecEnableList[i]->IsDestroyThis() == true )
		{
			AIObjHandle hIdentity;
			hUnitObject->SetAIHandle( hIdentity );
			delete CFcAIObject::s_pVecEnableList[i];
			i--;
			nSize--;
			continue;
		}
		CFcAIObject::s_pVecEnableList[i]->Process();
	}
//	CProfileMemChecker::GetInstance().OutputResult( "CFcAIObject::Initialize" );
//	PostListProcess();
}

void CFcAIObject::PostListProcess()
{
	int nSize = s_pVecPostEnableList.size();
	if( nSize ) {
		for( int j=0; j<nSize; j++ ) {
			for( DWORD i=0; i<s_pVecDisableList.size(); i++ ) {
				if( s_pVecDisableList[i] == s_pVecPostEnableList[j] ) {
					s_pVecDisableList.erase( s_pVecDisableList.begin() + i );
					break;
				}
			}
			s_pVecEnableList.push_back( s_pVecPostEnableList[j] );
		}
		s_pVecPostEnableList.clear();
	}

	nSize = s_pVecPostDisableList.size();
	if( nSize ) {
		for( int j=0; j<nSize; j++ ) {
			for( DWORD i=0; i<s_pVecEnableList.size(); i++ ) {
				if( s_pVecEnableList[i] == s_pVecPostDisableList[j] ) {
					s_pVecEnableList.erase( s_pVecEnableList.begin() + i );
					break;
				}
			}
			s_pVecDisableList.push_back( s_pVecPostDisableList[j] );
		}
		s_pVecPostDisableList.clear();
	}
}

void CFcAIObject::operator = ( const CFcAIObject &Obj )
{
	m_bIsFunction = Obj.m_bIsFunction;
	m_bCallbackFunc = Obj.m_bCallbackFunc;
	if( m_bIsFunction ) {
#ifdef _XBOX
		sprintf( m_szFileName, Obj.m_szFileName );
#else
		m_szFileName = Obj.m_szFileName;
#endif //_XBOX
	}
	// 복사 연산자 관련 나중 수정 사항들
	//
	// 내부적으로 따로 사용하는 맴버가 없는 클래스들 ( CFcAIConditionBase, CFcAIActionBase, CFcAITriggerParam )은
	// 솔직히 new로 크레딧 하여 오브젝트 마다 가지고 있을 필요가 없다.
	// 나중에 속도나 메모리 관련하여 많이 먹는다고 했을때는 ( 많이 먹진 않치만.. 갯수가 많이질 경우 어케될지 모름 )
	// 레퍼런스 카운터를 두구 관리하는 오브젝트로 바꿔주삼~

	if( Obj.m_pGlobalVariableMng ) {
		m_pGlobalVariableMng = new CFcAIGlobalVariableMng( this );
		*m_pGlobalVariableMng = *Obj.m_pGlobalVariableMng;
	}

	for( DWORD i=0; i<Obj.m_pVecFunc.size(); i++ ) {
		CFcAIFunc *pAIFunc = new CFcAIFunc;
		pAIFunc->SetUnitObjectHandle( m_hUnitObject );
		*pAIFunc = *Obj.m_pVecFunc[i];
		pAIFunc->SetParent( this );
		m_pVecFunc.push_back( pAIFunc );
	}

	for( DWORD i=0; i<Obj.m_pVecElement.size(); i++ ) {
		CFcAIElement *pElement = new CFcAIElement( this );
		*pElement = *Obj.m_pVecElement[i];
		m_pVecElement.push_back( pElement );
	}

	for( int j=0; j<AI_CALLBACK_NUM; j++ ) {
		for( DWORD i=0; i<Obj.m_pVecCallbackElement[j].size(); i++ ) {
			CFcAIElement *pElement = new CFcAIElement( this );
			*pElement = *Obj.m_pVecCallbackElement[j][i];
			m_pVecCallbackElement[j].push_back( pElement );
		}
	}
}

void CFcAIObject::SetUnitObjectHandle( GameObjHandle &Handle ) 
{ 
	m_hUnitObject = Handle; 
}

GameObjHandle CFcAIObject::GetUnitObjectHandle()
{ 
	return m_hCurUnitObject; 
}

CFcAIGlobalVariableMng *CFcAIObject::GetGlobalVariableMng()
{
	return m_pGlobalVariableMng;
}

CFcAISearchSlot *CFcAIObject::GetSearchSlot()
{
	return m_pSearchSlot;
}

CFcAIElement *CFcAIObject::GetElement( DWORD dwIndex )
{
	return m_pVecElement[dwIndex]; 
}

bool CFcAIObject::IsFunction()
{
	return m_bIsFunction;
}

CFcAIFunc *CFcAIObject::GetFunction( DWORD dwIndex )
{
	return m_pVecFunc[dwIndex];
}

DWORD CFcAIObject::GetTickInterval()
{
	return m_dwTickInterval;
}
void CFcAIObject::SetTickInterval( DWORD dwValue )
{
	m_dwTickInterval = dwValue;
}


void CFcAIObject::ProcessLOD()
{
	// Test
//	m_dwTickInterval = 1;
//	return;

	BaseObjHandle Handle = CFcBaseObject::GetCameraObjectHandle(0);
	D3DXVECTOR3 vPos = Handle->GetPos() - m_hUnitObject->GetDummyPos();
	vPos.y = 0.f;
	float fLength = D3DXVec3Length( &vPos );
	if( fLength > 20000 ) m_dwTickInterval = MAX_LOD_INTERVAL * 5;
	else if( fLength > 10000 ) m_dwTickInterval = MAX_LOD_INTERVAL * 2;
	else if( fLength > 5000 ) m_dwTickInterval = MAX_LOD_INTERVAL;
	else if( fLength > 2500 ) m_dwTickInterval = MAX_LOD_INTERVAL / 2;
//	else if( fLength > 1000 ) m_dwTickInterval = MAX_LOD_INTERVAL / 8;
	else m_dwTickInterval = MAX_LOD_INTERVAL / 4;

	m_hUnitObject->SetTickInterval( m_dwTickInterval / ( MAX_LOD_INTERVAL / 16 ) );

/*
	m_dwTickInterval = 1;
	char szStr[128];
	D3DXVECTOR3 vCam = Handle->GetPos();
	D3DXVECTOR3 vUnit = m_hUnitObject->GetDummyPos();
	sprintf( szStr, "Tick : %d    Cam : ( %.2f, %.2f, %.2f ), Unit : ( %.2f, %.2f, %.2f )\n", GetProcessTick(), vCam.x, vCam.y, vCam.z, vUnit.x, vUnit.y, vUnit.z );
	DebugString( szStr );
*/
}


void CFcAIObject::SetEnable( bool bEnable )
{
	if( m_bEnable == bEnable ) return;
	if( bEnable == true ) {
		/*
		for( DWORD i=0; i<s_pVecDisableList.size(); i++ ) {
			if( s_pVecDisableList[i] == this ) {
				s_pVecDisableList.erase( s_pVecDisableList.begin() + i );
				break;
			}
		}
		s_pVecEnableList.push_back( this );
		*/
		if( m_hUnitObject ) m_hUnitObject->SetTickInterval(1);
		s_pVecPostEnableList.push_back( this );
	}
	else {
		/*
		for( DWORD i=0; i<s_pVecEnableList.size(); i++ ) {
			if( s_pVecEnableList[i] == this ) {
				s_pVecEnableList.erase( s_pVecEnableList.begin() + i );
				break;
			}
		}
		s_pVecDisableList.push_back( this );
		*/
		if( m_hUnitObject ) m_hUnitObject->SetTickInterval(1);
		s_pVecPostDisableList.push_back( this );
	}

	m_bEnable = bEnable;
}

bool CFcAIObject::IsEnable()
{
	return m_bEnable;
}

void CFcAIObject::CheckParentType( CFcAIElement *pElement )
{
	if( pElement == NULL ) {
		m_hCurUnitObject = m_hUnitObject;
		return;
	}
	switch( pElement->GetParentType() ) {
		case PARENT_THIS:
			m_hCurUnitObject = m_hUnitObject;
			break;
		case PARENT_LINK:
			{
				if( m_hUnitObject->GetClassID() == CFcGameObject::Class_ID_Horse ) {
					CFcHorseObject *pObject = (CFcHorseObject *)m_hUnitObject.GetPointer();
					if( pObject->GetLinkObjCount() > 0 )
						m_hCurUnitObject = pObject->GetLinkObjHandle(0);
					else m_hCurUnitObject.Identity();
				}
				else m_hCurUnitObject = m_hUnitObject;
			}
			break;
	}
}

bool CFcAIObject::IsDestroyThis()
{
	
	if( m_hUnitObject->IsDie() == true /*&& m_hUnitObject->IsInScanner() == false*/ && m_nDestroyTick == -1 ) {
		SetForceProcessTick(1);
		m_nDestroyTick = 2;
	}
	if( m_nDestroyTick == 0 ) return true;
	return false;
/*	
	if( m_hUnitObject->IsDie() == true && m_nDestroyTick == -1 ) return true;
	return false;
*/
}


void CFcAIObject::SetForceProcessTick( int nValue )
{
	m_nForceProcess = nValue;
}

bool CFcAIObject::IsBranchLevel()
{
	if( m_nLoadLevel == -1 ) return false;
	return true;
}

bool CFcAIObject::IsBranchMode()
{
	if( m_nLoadModeType == -1 ) return false;
	return true;
}

void CFcAIObject::ProcessCallbackElement()
{
	// 항상 실행되어야 한다. 근데 일딴 쓰는데 없으니 빼노차.
//	m_bCallbackProcess[AI_CALLBACK_ALWAYS] = true;

	bool bFlag = false;
	for( int i=0; i<AI_CALLBACK_NUM; i++ ) {
		if( m_bCallbackProcess[i] == false ) continue;
		bFlag = true;
		for( DWORD j=0; j<m_pVecCallbackElement[i].size(); j++ ) {
			CheckParentType( m_pVecCallbackElement[i][j] );
			// 병렬
			if( !m_pVecCallbackElement[i][j]->IsActivate() ) continue;
			int nResult = m_pVecCallbackElement[i][j]->Process();
			switch( (nResult&0x0000ffff) ) {
				case ELEMENT_DESTROY_RETURN_VALUE:
					{
						short int nIndex = (nResult&0xffff0000) >> 16;
						if( nIndex == -1 ) {
							m_pVecCallbackElement[i][j]->SetActivate( false );
						}
						else {
							if( nIndex >= (int)m_pVecCallbackElement[i].size() ) break;
							m_pVecCallbackElement[i][nIndex]->SetActivate( false );
						}
					}
					break;
				case ELEMENT_CANCEL_RETURN_VALUE:
					return;
			}
		}
	}
	if( bFlag == true ) {
		for( DWORD j=0; j<m_pVecParamFunc.size(); j++ ) {
			m_pVecParamFunc[j]->ProcessCallbackFunc();
		}
	}

	memset( m_bCallbackProcess, 0, sizeof(m_bCallbackProcess) );
}

void CFcAIObject::OnCallback( AI_CALLBACK_TYPE Type )
{ 
	if( !m_bEnable ) return;
	m_bCallbackProcess[(int)Type] = true; 
}

void CFcAIObject::GetFuncParamList( const char *szStr, std::vector<int> &vecIndex )
{
	const char *pFindStr;
	for( DWORD i=0; i<m_pVecParamFunc.size(); i++ ) {
		pFindStr = strstr( (const char*)m_pVecParamFunc[i]->GetFunc()->GetFuncString(), szStr );
		if( pFindStr ) vecIndex.push_back(i);
	}
}

void CFcAIObject::PauseFunction( int nFunctionIndex, bool bPause )
{
	m_pVecParamFunc[nFunctionIndex]->SetPause( bPause );
}
