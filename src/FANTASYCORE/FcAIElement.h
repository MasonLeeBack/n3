#pragma once

#include "Data/AIDef.h"
#include "FcAIObject.h"
#include "FcAISearch.h"
#include "FcAIFunc.h"
#include "FcAITriggerBase.h"
//class CFcAIConditionBase;
//class CFcAIActionBase;
//class CFcAISearch;
//class CFcAIFuncParam;

class CFcAIElement {
public:
	CFcAIElement();
//	CFcAIElement( AIObjHandle hParent );
	CFcAIElement( CFcAIObject *pParent );
	~CFcAIElement();

protected:
	CFcAIObject *m_pParent;
//	AIObjHandle m_hParent;
	static GameObjHandle s_DummyHandle;
	union {
		void *m_pTarget;
//		GameObjHandle *m_phTargetHandle;
	};
	GameObjHandle m_hTargetHandle;
	bool m_bActivate;

	AI_ELEMENT_PROCESS_TYPE m_ProcessType;
	int m_nProcessParam;

	AI_ELEMENT_PARENT m_ParentType;

	int m_nDelayTick[2];

	CFcAISearch *m_pAISearch;
	std::vector<CFcAIConditionBase *> m_pVecCondition;
	std::vector<CFcAIActionBase *> m_pVecAction;
	std::vector<CFcAIActionBase *> m_pVecActionElse;
	std::vector<CFcAIFuncParam *> m_pVecFunc;

	inline int ProcessElement()
	{
		int nResult;
		DWORD dwSize = m_pVecCondition.size();
		for( DWORD i=0; i<dwSize; i++ ) {
			if( m_pVecCondition[i]->CheckCondition() == false ) {
				for( DWORD j=0; j<m_pVecActionElse.size(); j++ ) {
					nResult = m_pVecActionElse[j]->Command();
					if( nResult != TRUE ) return nResult;
				}
				return FALSE;
			}
		}

		dwSize = m_pVecAction.size();
		for( DWORD i=0; i<dwSize; i++ ) {
			nResult = m_pVecAction[i]->Command();
			if( nResult != TRUE && nResult != FALSE ) return nResult;
		}

		dwSize = m_pVecFunc.size();
		for( DWORD i=0; i<dwSize; i++ ) {
			m_pVecFunc[i]->ProcessFunc();
		}
		return TRUE;
	};


public:
//	void SetParent( AIObjHandle hParent );
//	AIObjHandle GetParent();
	void SetParent( CFcAIObject *pParent );
	CFcAIObject *GetParent();

	void SetParentType( AI_ELEMENT_PARENT Type );
	AI_ELEMENT_PARENT GetParentType();

	void SetProcessType( AI_ELEMENT_PROCESS_TYPE Type, int nParam );
	void SetSearch( CFcAISearch *pSearch );
	CFcAISearch *GetSearch();

	void AddCondition( CFcAIConditionBase *pBase );
	void AddAction( CFcAIActionBase *pBase );
	void AddActionElse( CFcAIActionBase *pBase );
	void AddFunction( CFcAIFuncParam *pFunc );
	void SetDelayTick( int nTick );

	AI_ELEMENT_PROCESS_TYPE GetProcessType() { return m_ProcessType; }
	int GetProcessTypeParam() { return m_nProcessParam; }

	inline int Process() {
		if( (int)CFcAIObject::s_dwLocalTick - m_nDelayTick[1] < m_nDelayTick[0] ) {
			return FALSE;
		}

		if( m_pAISearch ) {
			if( m_pAISearch->Process( CFcAIObject::s_dwLocalTick ) == true ) {
				int nProcessCount = m_pAISearch->GetProcessCount();
				if( nProcessCount == 0 ) {
					m_hTargetHandle.Identity();
					m_pTarget = NULL;
//					m_phTargetHandle = &s_DummyHandle;
					return ProcessElement();
				}
				else {
					for( int j=0; j<m_pAISearch->GetProcessCount(); j++ ) {
						switch( m_pAISearch->GetSearcherType() ) {
							case CFcAISearch::GAMEOBJECT:
								m_hTargetHandle = *(GameObjHandle*)m_pAISearch->GetSearchProcess( j );
								break;
							case CFcAISearch::PROP:
								m_pTarget = m_pAISearch->GetSearchProcess( j );
								break;
						}
//						m_pTarget = m_pAISearch->GetSearchProcess( j );
//						if( (*m_phTargetHandle) == m_pParent->GetUnitObjectHandle() ) continue;
						if( m_hTargetHandle == m_pParent->GetUnitObjectHandle() ) continue;

						int nResult = ProcessElement();
						if( nResult != FALSE ) return nResult;
//						else if( nResult == -1 ) return -1;
					}
				}
			}
			else return ProcessElement();
		}
		else {
			return ProcessElement();
		}
		return FALSE;
	};

	inline GameObjHandle GetUnitHandle() { return m_pParent->GetUnitObjectHandle(); }
	inline GameObjHandle GetTargetHandle() { return m_hTargetHandle; }
	inline void *GetTargetPointer() { return m_pTarget; }

	void operator = ( CFcAIElement &Obj );

	void SetNullFuncParam( CFcParamVariable *pParam );
	void SetActivate( bool bFlag ) { m_bActivate = bFlag; }
	bool IsActivate() { return m_bActivate; }

};
