#include "StdAfx.h"
#include "FcAIGlobalVariableMng.h"
#include "FcParamVariable.h"
#include "BsCommon.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

CFcAIGlobalVariableMng::CFcAIGlobalVariableMng( CFcAIObject *pParent )
{
	m_pParent = pParent;
}

CFcAIGlobalVariableMng::~CFcAIGlobalVariableMng()
{
	SAFE_DELETE_PVEC( m_pVecList );
}

void CFcAIGlobalVariableMng::AddVariable( CFcParamVariable *pVariable )
{
	m_pVecList.push_back( pVariable );
}

CFcParamVariable *CFcAIGlobalVariableMng::GetVariable( DWORD dwIndex )
{
	if( dwIndex < 0 || dwIndex >= m_pVecList.size() ) return NULL;
	return m_pVecList[dwIndex];
}

int CFcAIGlobalVariableMng::GetVariableIndex( const char *szDescription )
{
	for( DWORD i=0; i<m_pVecList.size(); i++ ) {
		if( strcmp( szDescription, m_pVecList[i]->GetDescription() ) == NULL ) return (int)i;
	}
	return -1;
}

void CFcAIGlobalVariableMng::operator = ( CFcAIGlobalVariableMng &Obj )
{
	for( DWORD i=0; i<Obj.m_pVecList.size(); i++ ) {
		CFcParamVariable *pVariable = new CFcParamVariable( Obj.m_pVecList[i]->GetType() );
		*pVariable = *Obj.m_pVecList[i];
		m_pVecList.push_back( pVariable );
	}
}


CFcParamVariableEx::CFcParamVariableEx( VT Type, CFcAIGlobalVariableMng *pMng )
: CFcParamVariable( Type )
{
	m_pGlobalMng = pMng;
	m_nGlobalValueIndex = -1;
}

CFcParamVariableEx::~CFcParamVariableEx()
{
}

int CFcParamVariableEx::GetVariableInt()
{
	if( m_nGlobalValueIndex == -2 ) {
		return CFcParamVariable::GetVariableInt();
	}
	else if( m_nGlobalValueIndex == -1 ) {
		if( m_Type == STRING ) {
			m_nGlobalValueIndex = m_pGlobalMng->GetVariableIndex( GetVariableString() );
			if( m_nGlobalValueIndex == -1 ) {
				m_nGlobalValueIndex = -2;
			}
			else {
				CFcParamVariable *pVariable = m_pGlobalMng->GetVariable( m_nGlobalValueIndex );
				BsAssert( NULL != pVariable );	// mruete: prefix bug 548: added assert
				switch( pVariable->GetType() ) {
					case CFcParamVariable::INT:
						return pVariable->GetVariableInt();
						break;
					case CFcParamVariable::RANDOM:
						return pVariable->GetGenRandom();
						break;
				}
			}
		}
		else {
			m_nGlobalValueIndex = -2;
		}
	}
	else {
		CFcParamVariable *pVariable = m_pGlobalMng->GetVariable( m_nGlobalValueIndex );
		switch( pVariable->GetType() ) {
			case CFcParamVariable::INT:
				return pVariable->GetVariableInt();
				break;
			case CFcParamVariable::RANDOM:
				return pVariable->GetGenRandom();
				break;
		}
	}
	return CFcParamVariable::GetVariableInt();
}