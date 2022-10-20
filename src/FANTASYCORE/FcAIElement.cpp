#include "StdAfx.h"
#include "FcAIElement.h"
#include "FcAITriggerBase.h"
#include "FcAISearch.h"
#include "FcAISearchGameObject.h"
#include "FcAISearchProp.h"
#include "PerfCheck.h"
#include "FcAIFunc.h"
#include "FcCameraObject.h"
#include "FcGameObject.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

GameObjHandle CFcAIElement::s_DummyHandle;

CFcAIElement::CFcAIElement()
{
	m_pAISearch = NULL;
	m_ProcessType = AI_ELEMENT_PROCESS_LOD;
	m_nProcessParam = 0;
	m_pParent = NULL;
	m_ParentType = PARENT_THIS;
	m_bActivate = true;
	m_pTarget = NULL;
//	m_phTargetHandle = &s_DummyHandle;

	memset( m_nDelayTick, 0, sizeof(m_nDelayTick) );
}

CFcAIElement::CFcAIElement( CFcAIObject *pParent )
{
	m_pAISearch = NULL;
	m_pParent = pParent;
	m_ProcessType = AI_ELEMENT_PROCESS_LOD;
	m_nProcessParam = 0;
	m_ParentType = PARENT_THIS;
	m_bActivate = true;
	m_pTarget = NULL;
//	m_phTargetHandle = &s_DummyHandle;

	memset( m_nDelayTick, 0, sizeof(m_nDelayTick) );
}

CFcAIElement::~CFcAIElement()
{
	SAFE_DELETE( m_pAISearch );
	SAFE_DELETE_PVEC( m_pVecCondition );
	SAFE_DELETE_PVEC( m_pVecAction );
	SAFE_DELETE_PVEC( m_pVecActionElse );
	SAFE_DELETE_PVEC( m_pVecFunc );
}

void CFcAIElement::SetProcessType( AI_ELEMENT_PROCESS_TYPE Type, int nParam )
{
	m_ProcessType = Type;
	m_nProcessParam = nParam;
}

void CFcAIElement::SetParentType( AI_ELEMENT_PARENT Type )
{
	m_ParentType = Type;
}

AI_ELEMENT_PARENT CFcAIElement::GetParentType()
{
	return m_ParentType;
}

/*
void CFcAIElement::SetParent( AIObjHandle hParent )
{
	m_hParent = hParent;
}

AIObjHandle CFcAIElement::GetParent()
{
	return m_hParent;
}
*/
void CFcAIElement::SetParent( CFcAIObject *pParent )
{
	m_pParent = pParent;
}

CFcAIObject *CFcAIElement::GetParent()
{
	return m_pParent;
}

void CFcAIElement::AddCondition( CFcAIConditionBase *pBase )
{
	m_pVecCondition.push_back( pBase );
}

void CFcAIElement::AddAction( CFcAIActionBase *pBase )
{
	m_pVecAction.push_back( pBase );
}

void CFcAIElement::AddActionElse( CFcAIActionBase *pBase )
{
	m_pVecActionElse.push_back( pBase );
}

void CFcAIElement::AddFunction( CFcAIFuncParam *pFunc )
{
	m_pVecFunc.push_back( pFunc );
	m_pParent->AddFuncParamList( pFunc );
}

/*
bool CFcAIElement::Process()
{
	if( (int)CFcAIObject::s_dwLocalTick - m_nDelayTick[1] < m_nDelayTick[0] ) {
		return false;
	}

	if( m_pAISearch ) {
		if( m_pAISearch->Process( CFcAIObject::s_dwLocalTick ) == true ) {
			int nProcessCount = m_pAISearch->GetProcessCount();
			if( nProcessCount == 0 ) {
				m_hTargetHandle.Identity();
			}
			else {
				for( int j=0; j<m_pAISearch->GetProcessCount(); j++ ) {
					m_pAISearch->SetSearchProcess( j, m_hTargetHandle );
					if( m_hTargetHandle == m_pParent->GetUnitObjectHandle() ) continue;

					if( ProcessElement() == true ) {
						return true;
					}
				}
			}
		}
		else return ProcessElement();
	}
	else {
		return ProcessElement();
	}
	return false;
}

bool CFcAIElement::ProcessElement()
{
	for( DWORD i=0; i<m_pVecCondition.size(); i++ ) {
		if( m_pVecCondition[i]->CheckCondition() == false ) {
			for( DWORD j=0; j<m_pVecActionElse.size(); j++ ) {
				m_pVecActionElse[j]->Command();
			}
			return false;
		}
	}

	for( DWORD i=0; i<m_pVecAction.size(); i++ ) {
		m_pVecAction[i]->Command();
	}

	for( DWORD i=0; i<m_pVecFunc.size(); i++ ) {
		m_pVecFunc[i]->ProcessFunc();
	}
	return true;
}
*/

void CFcAIElement::SetSearch( CFcAISearch *pSearch )
{
	m_pAISearch = pSearch;
}

CFcAISearch *CFcAIElement::GetSearch()
{
	return m_pAISearch;
}
/*
GameObjHandle CFcAIElement::GetUnitHandle()
{
	return m_hParent->GetUnitObjectHandle();
}

GameObjHandle CFcAIElement::GetTargetHandle()
{
	return m_hTargetHandle;
}
*/

void CFcAIElement::operator = ( CFcAIElement &Obj )
{
	m_ProcessType = Obj.m_ProcessType;
	m_nProcessParam = Obj.m_nProcessParam;
	m_ParentType = Obj.m_ParentType;

	CFcAISearch::SEARCHER_TYPE SearcherType = CFcAISearch::GAMEOBJECT;
	if( Obj.GetSearch() != NULL ) {
		SearcherType = Obj.GetSearch()->GetSearcherType();
		switch( SearcherType ) {
			case CFcAISearch::GAMEOBJECT:	
				m_pAISearch = new CFcAISearchGameObject( m_pParent );
				break;
			case CFcAISearch::PROP:			
				m_pAISearch = new CFcAISearchProp( m_pParent );
				break;
		}
		*m_pAISearch = *Obj.m_pAISearch;
	}

	for( DWORD i=0; i<Obj.m_pVecCondition.size(); i++ ) {
		CFcAIConditionBase *pBase = CFcAIConditionBase::CreateObject( Obj.m_pVecCondition[i]->GetID(), SearcherType );
		pBase->SetParent( this );	// 반드시 부모를 바꿔줘야 한다!!
		*pBase = *Obj.m_pVecCondition[i];
		m_pVecCondition.push_back( pBase );
	}

	for( DWORD i=0; i<Obj.m_pVecAction.size(); i++ ) {
		CFcAIActionBase *pBase = CFcAIActionBase::CreateObject( Obj.m_pVecAction[i]->GetID(), SearcherType );
// [PREFIX:beginmodify] 2006/2/25 junyash PS#5812 dereferencing NULL pointer
		BsAssert(pBase);
// [PREFIX:endmodify] junyash
		pBase->SetParent( this );	// 반드시 부모를 바꿔줘야 한다!!
		*pBase = *Obj.m_pVecAction[i];
		m_pVecAction.push_back( pBase );
	}

	for( DWORD i=0; i<Obj.m_pVecActionElse.size(); i++ ) {
		CFcAIActionBase *pBase = CFcAIActionBase::CreateObject( Obj.m_pVecActionElse[i]->GetID(), SearcherType );
		pBase->SetParent( this );	// 반드시 부모를 바꿔줘야 한다!!
		*pBase = *Obj.m_pVecActionElse[i];
		m_pVecActionElse.push_back( pBase );
	}

	for( DWORD i=0; i<Obj.m_pVecFunc.size(); i++ ) {
		CFcAIFuncParam *pFunc = new CFcAIFuncParam( m_pParent->GetFunction( Obj.m_pVecFunc[i]->GetIndex() ), Obj.m_pVecFunc[i]->GetIndex() );

		*pFunc = *Obj.m_pVecFunc[i];
		pFunc->Initialize();
		AddFunction( pFunc );
//		m_pVecFunc.push_back( pFunc );
	}
}

void CFcAIElement::SetDelayTick( int nTick )
{
	m_nDelayTick[0] = nTick;
	m_nDelayTick[1] = (int)CFcAIObject::s_dwLocalTick;
}

void CFcAIElement::SetNullFuncParam( CFcParamVariable *pParam )
{
	if( m_pAISearch )
		m_pAISearch->SetNullFuncParam( pParam );

	for( DWORD i=0; i<m_pVecCondition.size(); i++ )
		m_pVecCondition[i]->SetNullFuncParam( pParam );

	for( DWORD i=0; i<m_pVecAction.size(); i++ )
		m_pVecAction[i]->SetNullFuncParam( pParam );

	for( DWORD i=0; i<m_pVecActionElse.size(); i++ )
		m_pVecActionElse[i]->SetNullFuncParam( pParam );
}