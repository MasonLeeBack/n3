#include "StdAfx.h"
#include "FcAISearch.h"
#include "FcWorld.h"
#include "FcParamVariable.h"
#include "FcAIFunc.h"
#include "FcAIGlobalVariableMng.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

int CFcAISearch::s_nTickIntervalOffset = 0;
CFcAISearch::CFcAISearch( CFcAIObject *pParent )
{
	m_SearcherType = UNKNOWN;
	m_pParent = pParent;
	m_Type = AI_SEARCH_NONE;
	m_nProcessTick = 1;

	m_nStandTick = s_nTickIntervalOffset;
	s_nTickIntervalOffset+=1;
	if( s_nTickIntervalOffset > DEFAULT_FRAME_RATE ) s_nTickIntervalOffset = 0;

	m_nPrevSearchTick = 0;
}

CFcAISearch::~CFcAISearch()
{
	SAFE_DELETE_PVEC( m_pVecParam );
}

CFcAISearch::SEARCHER_TYPE CFcAISearch::GetSearcherType()
{
	return m_SearcherType;
}

void CFcAISearch::SetNullFuncParam( CFcParamVariable *pParam )
{
	for( DWORD i=0; i<m_pVecParam.size(); i++ ) {
		if( pParam == m_pVecParam[i] ) {
			m_pVecParam.erase( m_pVecParam.begin() + i );
			i--;
		}
	}
}

void CFcAISearch::SetType( AI_SEARCH_TYPE Type )
{
	m_Type = Type;
}

void CFcAISearch::AddFilter( AI_SEARCH_FILTER Filter )
{
	SearchFilterStruct Struct;
	Struct.Filter = Filter;
	Struct.dwParamOffset = m_pVecParam.size();
	m_VecFilter.push_back( Struct );
}

void CFcAISearch::AddCondition( AI_SEARCH_CON Con )
{
	SearchConStruct Struct;
	Struct.Con = Con;
	Struct.dwParamOffset = m_pVecParam.size();
	m_VecCon.push_back( Struct );
}

bool CFcAISearch::CheckCondition()
{
	return true;
}

bool CFcAISearch::Process( int nTick )
{
	return true;
}

void CFcAISearch::ProcessFilter()
{
}

int CFcAISearch::GetProcessCount()
{
	return 0;
}

void *CFcAISearch::GetSearchProcess( int nIndex )
{
	return NULL;
}

void CFcAISearch::SetProcessTick( int nTick )
{
	m_nProcessTick = nTick;
}

void CFcAISearch::AddParam( CFcParamVariable *pParam )
{
	m_pVecParam.push_back( pParam );
}

void CFcAISearch::AddParamAlloc( CFcParamVariable &Param )
{
	CFcParamVariable *pParam = new CFcParamVariable;
	*pParam = Param;
	m_pVecParam.push_back( pParam );
}


void CFcAISearch::operator = ( CFcAISearch &Obj )
{
	m_Type = Obj.m_Type;
	m_VecFilter = Obj.m_VecFilter;
	m_VecCon = Obj.m_VecCon;
	m_nProcessTick = Obj.m_nProcessTick;

	for( DWORD i=0; i<Obj.m_pVecParam.size(); i++ ) {
		if( m_pParent->IsFunction() == true ) {
			const char *szDesc = Obj.m_pVecParam[i]->GetDescription();
			if( szDesc ) {
				CFcParamVariable *pFuncParam = ((CFcAIFunc*)m_pParent)->GetParamFromDescription( szDesc );
				if( pFuncParam ) {
					m_pVecParam.push_back( pFuncParam );
					continue;
				}
			}
		}

		CFcParamVariable *pParam;
		if( Obj.m_pVecParam[i]->GetType() == CFcParamVariable::STRING ) {
			pParam = new CFcParamVariableEx( Obj.m_pVecParam[i]->GetType(), m_pParent->GetGlobalVariableMng() );
			*pParam = *Obj.m_pVecParam[i];
		}
		else {
			pParam = new CFcParamVariable( Obj.m_pVecParam[i]->GetType() );
			*pParam = *Obj.m_pVecParam[i];
		}
		m_pVecParam.push_back( pParam );
	}
}