#include "stdafx.h"
#include "FcAITriggerBase.h"
#include "data/AIDef.h"
#include "BsCommon.h"
#include "FcParamVariable.h"
#include "FcAIGlobalVariableMng.h"
#include "FcAIObject.h"
#include "FcAIElement.h"
#include "FcAIFunc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

CFcAIBase::CFcAIBase()
{
	m_nID = -1;
	m_pParent = NULL;
}

CFcAIBase::~CFcAIBase()
{
	SAFE_DELETE_PVEC( m_pVecParam );
	for( DWORD i=0; i<m_pVecOperator.size(); i++ ) {
		if( m_pVecOperator[i] ) {
			SAFE_DELETE_VEC( m_pVecOperator[i]->nVecOperator );
			SAFE_DELETE_PVEC( m_pVecOperator[i]->pVecParam );
			SAFE_DELETE( m_pVecOperator[i]->pResult );
		}
	}
	SAFE_DELETE_PVEC( m_pVecOperator );
}

void CFcAIBase::AddOperatorParam( std::vector<CFcParamVariable *> &pVecOperatorParam, std::vector<int> &nVecOperator )
{
	OperatorStruct *pStruct = new OperatorStruct;
	pStruct->pVecParam = pVecOperatorParam;
	pStruct->nVecOperator = nVecOperator;
	pStruct->pResult = NULL;

	pStruct->bPreCalc = true;
	for( DWORD i=0; i<pVecOperatorParam.size(); i++ ) {
		if( pVecOperatorParam[i]->GetType() == CFcParamVariable::STRING ) {
			if( pVecOperatorParam[i]->GetVariableString()[0] == '@' &&
				pVecOperatorParam[i]->GetVariableString()[1] == '@' ) {
					pStruct->bPreCalc = false;
					break;
				}
		}
		else if( pVecOperatorParam[i]->GetType() == CFcParamVariable::CUSTOM ) {
			if( CFcAIGlobalParam::GetInstance().IsValidGlobalParam( pVecOperatorParam[i]->GetVariableString() ) ) {
				int nIndex = CFcAIGlobalParam::GetInstance().GetIndex( pVecOperatorParam[i]->GetVariableString() );
				if( CFcAIGlobalParam::s_GlobalParamList[ nIndex ].bChange == true ) {
					pStruct->bPreCalc = false;
					break;
				}
			}
		}
	}

	m_pVecOperator.push_back( pStruct );
}

void CFcAIBase::CalcOperatorValue( DWORD dwIndex )
{
	/*
	if( m_pVecOperator[dwIndex]->pVecParam[0]->GetType() == CFcParamVariable::STRING ) {
		CFcAIGlobalVariableMng *pMng = m_pParent->GetParent()->GetGlobalVariableMng();
		int nIndex = pMng->GetVariableIndex( m_pVecOperator[dwIndex]->pVecParam[0]->GetVariableString() );
		if( nIndex != -1 ) {
			m_pVecOperator[dwIndex]->pVecParam[0]->Clear( pMng->GetVariable( nIndex )->GetType() );
			*m_pVecOperator[dwIndex]->pVecParam[0] = *pMng->GetVariable( nIndex );
		}
	}
	*/


	CFcParamVariable *pTemp = CFcAIGlobalParam::GetInstance().GetGlobalParamValue( m_pParent, m_pVecOperator[dwIndex]->pVecParam[0] );
	if( m_pVecOperator[dwIndex]->bPreCalc == false && m_pVecOperator[dwIndex]->pResult ) {
		SAFE_DELETE( m_pVecOperator[dwIndex]->pResult );
	}
	m_pVecOperator[dwIndex]->pResult = new CFcParamVariable( pTemp->GetType() );
	CFcParamVariable *pResult = m_pVecOperator[dwIndex]->pResult;
	*pResult= *pTemp;

	for( DWORD i=0; i<m_pVecOperator[dwIndex]->nVecOperator.size(); i++ ) {
		switch( m_pVecOperator[dwIndex]->nVecOperator[i] ) {
			case 0:	*pResult += *CFcAIGlobalParam::GetInstance().GetGlobalParamValue( m_pParent, m_pVecOperator[dwIndex]->pVecParam[i+1] );	break;
			case 1:	*pResult -= *CFcAIGlobalParam::GetInstance().GetGlobalParamValue( m_pParent, m_pVecOperator[dwIndex]->pVecParam[i+1] );	break;
			case 2:	*pResult *= *CFcAIGlobalParam::GetInstance().GetGlobalParamValue( m_pParent, m_pVecOperator[dwIndex]->pVecParam[i+1] );	break;
			case 3:	*pResult /= *CFcAIGlobalParam::GetInstance().GetGlobalParamValue( m_pParent, m_pVecOperator[dwIndex]->pVecParam[i+1] );	break;
		}
	}
}

void CFcAIBase::SetNullFuncParam( CFcParamVariable *pParam )
{
	for( DWORD i=0; i<m_pVecParam.size(); i++ ) {
		if( m_pVecParam[i] ) {
			if( m_pVecParam[i] == pParam ) {
				m_pVecParam.erase( m_pVecParam.begin() + i );
				i--;
			}
		}
	}
	for( DWORD i=0; i<m_pVecOperator.size(); i++ ) {
		if( m_pVecOperator[i] ) {
			for( DWORD j=0; j<m_pVecOperator[i]->pVecParam.size(); j++ ) {
				if( m_pVecOperator[i]->pVecParam[j] == pParam ) {
					m_pVecOperator[i]->pVecParam.erase( m_pVecOperator[i]->pVecParam.begin() + j );
					j--;
				}
			}
		}
	}

}

void CFcAIBase::operator = ( CFcAIBase &Obj )
{
	m_nID = Obj.m_nID;
	for( DWORD i=0; i<Obj.m_pVecParam.size(); i++ ) {
		if( Obj.m_pVecParam[i] ) {
			bool bFuncParam = false;
			if( m_pParent->GetParent()->IsFunction() == true ) {
				CFcAIObject *pParent = m_pParent->GetParent();
				const char *szDesc = Obj.m_pVecParam[i]->GetDescription();
				if( szDesc ) {
					CFcParamVariable *pFuncParam = ((CFcAIFunc*)pParent)->GetParamFromDescription( szDesc );
					if( pFuncParam ) {
						m_pVecParam.push_back( pFuncParam );
						bFuncParam = true;
					}
				}
			}
			if( bFuncParam == false ) {
				CFcParamVariable *pParam;
				if( Obj.m_pVecParam[i]->GetType() == CFcParamVariable::STRING ) {
					pParam = new CFcParamVariableEx( Obj.m_pVecParam[i]->GetType(), m_pParent->GetParent()->GetGlobalVariableMng() );
					*pParam = *Obj.m_pVecParam[i];
				}
				else {
					pParam = new CFcParamVariable( Obj.m_pVecParam[i]->GetType() );
					*pParam = *Obj.m_pVecParam[i];
				}
				m_pVecParam.push_back( pParam );
			}
		}
		else m_pVecParam.push_back( NULL );

		if( Obj.m_pVecOperator[i] ) {
			OperatorStruct *pStruct = NULL;
			pStruct = new OperatorStruct;
			pStruct->pResult = NULL;
			pStruct->nVecOperator = Obj.m_pVecOperator[i]->nVecOperator;
			pStruct->bPreCalc = Obj.m_pVecOperator[i]->bPreCalc;

			for( DWORD j=0; j<Obj.m_pVecOperator[i]->pVecParam.size(); j++ ) {
				bool bFuncParam = false;
				if( m_pParent->GetParent()->IsFunction() == true ) {
					CFcAIObject *pParent = m_pParent->GetParent();
					const char *szDesc = Obj.m_pVecOperator[i]->pVecParam[j]->GetDescription();
					if( szDesc ) {
						CFcParamVariable *pFuncParam = ((CFcAIFunc*)pParent)->GetParamFromDescription( szDesc );
						if( pFuncParam ) {
							pStruct->pVecParam.push_back( pFuncParam );
							bFuncParam = true;
						}
					}
				}

				if( bFuncParam == false ) {
					if( Obj.m_pVecOperator[i]->pVecParam[j]->GetType() == CFcParamVariable::STRING ) {
						CFcParamVariable *pParam = new CFcParamVariableEx( Obj.m_pVecOperator[i]->pVecParam[j]->GetType(), m_pParent->GetParent()->GetGlobalVariableMng() );
						*pParam = *Obj.m_pVecOperator[i]->pVecParam[j];
						if( Obj.m_pVecOperator[i]->pVecParam[j]->GetVariableString()[0] == '@' ) {
							pParam->SetDescription( NULL );
						}
						pStruct->pVecParam.push_back( pParam );
					}
					else {
						CFcParamVariable *pParam = new CFcParamVariable( Obj.m_pVecOperator[i]->pVecParam[j]->GetType() );
						*pParam = *Obj.m_pVecOperator[i]->pVecParam[j];
						pStruct->pVecParam.push_back( pParam );
					}
				}
			}
			m_pVecOperator.push_back( pStruct );
		}
		else m_pVecOperator.push_back( NULL );
	}
}

CFcAIConditionBase::CFcAIConditionBase()
{
}

CFcAIConditionBase::~CFcAIConditionBase()
{
}

bool CFcAIConditionBase::Compare( int nValue1, int nValue2, int nOp )
{
	switch( nOp ) {
	case AI_OP_EQUAL:
		if(nValue1==nValue2) { return true; }	break;
	case AI_OP_NOT_EQUAL:
		if(nValue1!=nValue2) { return true; } break;
	case AI_OP_GREATER:
		if(nValue1>nValue2) { return true; }	break;
	case AI_OP_GREATER_THAN_OR_EQUAL:
		if(nValue1>=nValue2) { return true; } break;
	case AI_OP_LESS_THAN:
		if(nValue1<nValue2) {	return true; } break;
	case AI_OP_LESS_THAN_OR_EQUAL:
		if(nValue1<=nValue2) { return true; } break;
	default:
		BsAssert(0);							break;
	}
	return false;
}

bool CFcAIConditionBase::Compare( float fValue1, float fValue2, int nOp )
{
	switch( nOp ) {
	case AI_OP_EQUAL:
		if(fValue1==fValue2) { return true; }	break;
	case AI_OP_NOT_EQUAL:
		if(fValue1!=fValue2) { return true; } break;
	case AI_OP_GREATER:
		if(fValue1>fValue2) { return true; }	break;
	case AI_OP_GREATER_THAN_OR_EQUAL:
		if(fValue1>=fValue2) { return true; } break;
	case AI_OP_LESS_THAN:
		if(fValue1<fValue2) {	return true; } break;
	case AI_OP_LESS_THAN_OR_EQUAL:
		if(fValue1<=fValue2) { return true; } break;
	default:
		BsAssert(0);							break;
	}
	return false;
}

CFcAIActionBase::CFcAIActionBase()
{
}

CFcAIActionBase::~CFcAIActionBase()
{
}