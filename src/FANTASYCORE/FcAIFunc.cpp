#include "StdAfx.h"
#include "FcAIFunc.h"
#include "Parser.h"
#include "FcParamVariable.h"
#include "FcAIElement.h"
#include "FcAIGlobalVariableMng.h"
#include "PerfCheck.h"
#include "FcAIHardCodingFunction.h"
#include "FcGlobal.h"
#include "PerfCheck.h"
#include "DebugUtil.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

CFcAIFunc::CFcAIFunc()
{
	m_pParent = NULL;
	m_bIsFunction = true;
	m_nHardCodingFunctionIndex = -1;

}

CFcAIFunc::~CFcAIFunc()
{
	for( DWORD i=0; i<m_pVecElement.size(); i++ ) {
		for( DWORD j=0; j<m_pVecParam.size(); j++ ) {
			m_pVecElement[i]->SetNullFuncParam( m_pVecParam[j] );
		}
	}
	for( int k=0; k<AI_CALLBACK_NUM; k++ ) {
		for( DWORD i=0; i<m_pVecCallbackElement[k].size(); i++ ) {
			for( DWORD j=0; j<m_pVecParam.size(); j++ ) {
				m_pVecCallbackElement[k][i]->SetNullFuncParam( m_pVecParam[j] );
			}
		}
	}

	SAFE_DELETE_PVEC( m_pVecParam );

	for( DWORD i=0; i<s_pVecFunctionList.size(); i++ ) {
		if( s_pVecFunctionList[i] == this ) {
			s_pVecFunctionList.erase( s_pVecFunctionList.begin() + i );
			i--;
		}
	}
	if( m_nHardCodingFunctionIndex != -1 ) {
		CFcAIHardCodingFunction::ReleaseFunction( m_nHardCodingFunctionIndex );
	}
}

void CFcAIFunc::SetParent( CFcAIObject *pParent )
{
	m_pParent = pParent;
}

CFcAIObject *CFcAIFunc::GetParent()
{
	return m_pParent;
}

void CFcAIFunc::SetHardCodingFunctionIndex( int nIndex )
{
	m_nHardCodingFunctionIndex = nIndex;
	CFcAIHardCodingFunction::GetFunction( nIndex )->SetParent( this );
}

int CFcAIFunc::GetHardCodingFunctionIndex()
{
	return m_nHardCodingFunctionIndex;
}

CFcAISearchSlot *CFcAIFunc::GetSearchSlot()
{
	return m_pParent->GetSearchSlot();
}

void CFcAIFunc::InitParser( Parser *pParser )
{
	CFcAIObject::InitParser( pParser );
	// func 파일에서 사용
	pParser->ReserveKeyword("AI_FUNCTION_PARAMETER");
	pParser->ReserveKeyword("AI_FUNCTION_PARAMETER_END");
}

void CFcAIFunc::ProcessParser( TokenBuffer &itr )
{
	CFcAIObject::ProcessParser( itr );
	if( strcmp( itr.GetKeyword(), "AI_FUNCTION_PARAMETER" ) == 0 ) {
		itr++;
		while(1) {
			CFcParamVariable *pParam = NULL;
			if( itr.IsKeyword() ) {
				if( strcmp( itr.GetKeyword(), "AI_FUNCTION_PARAMETER_END" ) == 0 ) {
					break;
				}
			}
			const char *szDesc = itr.GetVariable();
			itr++;
			int nVariableType = itr.GetInteger();
			itr++;

			pParam = new CFcParamVariable( (CFcParamVariable::VT)nVariableType );
			pParam->SetDescription( szDesc );
			m_pVecParam.push_back( pParam );
		};
	}
	else if( strcmp( itr.GetKeyword(), "AI_FUNCTION_PARAMETER_END" ) == 0 ) {
		itr++;
	}
}

bool CFcAIFunc::SetExtensionVariable( CFcParamVariable **pParam, TokenBuffer &itr )
{
	if( itr.IsVariable() ) {
		for( DWORD j=0; j<m_pVecParam.size(); j++ ) {
			if( strcmp( itr.GetVariable(), m_pVecParam[j]->GetDescription() ) == 0 ) {
				*pParam = m_pVecParam[j];
				return true;
//				break;
			}
		}
	}
	return CFcAIObject::SetExtensionVariable( pParam, itr );
}

const char *CFcAIFunc::GetFuncString()
{
	// 임시 땜빵 코드. 난 스트링 컨트롤 못해요~
	static char szTemp[64];
#ifdef _XBOX
	int nSize = strlen( m_szFileName );
#else
	int nSize = m_szFileName.length();
#endif
	int nCount = 0;
	char cPtr;
	for( int i=0; i<nSize; i++ ) {
		cPtr = m_szFileName[i];
		if( cPtr == '\\' || cPtr == '/' ) {
			nCount = 0;
			continue;
		}
		else if( cPtr == '.' ) {
			szTemp[nCount] = 0;
			break;
		}
		szTemp[nCount] = cPtr;
		nCount++;
	}
	return szTemp;
}

CFcAIObject *CFcAIFunc::IsExist( const char *szFileName, int nLevel, bool bEnemy )
{
	for( DWORD i=0; i<s_pVecFunctionList.size(); i++ ) {
		if( strcmp( s_pVecFunctionList[i]->GetFileName(), szFileName ) == NULL )
			return s_pVecFunctionList[i];
	}
	return NULL;
}

CFcParamVariable *CFcAIFunc::GetParamFromDescription( const char *szDescription )
{
	for( DWORD i=0; i<m_pVecParam.size(); i++ ) {
		if( strcmp( szDescription, m_pVecParam[i]->GetDescription() ) == 0 ) {
			return m_pVecParam[i];
		}
	}
	return NULL;
}

DWORD CFcAIFunc::GetParamCount()
{
	if( m_nHardCodingFunctionIndex == -1 )
		return m_pVecParam.size();
	else return CFcAIHardCodingFunction::GetFunction( m_nHardCodingFunctionIndex )->GetParamCount();
}

CFcParamVariable *CFcAIFunc::GetParam( DWORD dwIndex )
{
	if( dwIndex < 0 || dwIndex >= m_pVecParam.size() ) return NULL;
	return m_pVecParam[dwIndex];
}

void CFcAIFunc::ProcessCallbackElement()
{
	for( int i=0; i<AI_CALLBACK_NUM; i++ ) {
		m_bCallbackProcess[i] = m_pParent->m_bCallbackProcess[i];
	}

	CFcAIObject::ProcessCallbackElement();
}

void CFcAIFunc::operator = ( const CFcAIFunc &Obj )
{
	for( DWORD i=0; i<Obj.m_pVecParam.size(); i++ ) {
		CFcParamVariable *pParam = new CFcParamVariable( Obj.m_pVecParam[i]->GetType() );
		*pParam = *Obj.m_pVecParam[i];
		m_pVecParam.push_back( pParam );
	}                     

	// ㅡ,.ㅡ 하위 복사.ㅋㅋㅋㅋ 진짜 이상하다~~~
	*(CFcAIObject*)this = Obj;
	/////////////////////////////////////////////
	if( Obj.m_nHardCodingFunctionIndex != -1 ) {
		int nHCFIndex = CFcAIHardCodingFunction::FindFunction( GetFuncString() );
		SetHardCodingFunctionIndex( nHCFIndex );
	}
}

void CFcAIFunc::GetFuncParamList( const char *szStr, std::vector<int> &vecIndex )
{
	m_pParent->GetFuncParamList( szStr, vecIndex );
}

void CFcAIFunc::PauseFunction( int nFunctionIndex, bool bPause )
{
	m_pParent->PauseFunction( nFunctionIndex, bPause );
}

CFcAIFuncParam::CFcAIFuncParam( CFcAIFunc *pAIFunc, int nIndex )
{
	m_pAIFunc = pAIFunc;
	m_nIndex = nIndex;

	m_bPause = false;
}

CFcAIFuncParam::~CFcAIFuncParam()
{
	if( m_pAIFunc && m_pAIFunc->GetHardCodingFunctionIndex() != -1 ) {
		CFcAIHardCodingFunction *pHCF = CFcAIHardCodingFunction::GetFunction( m_pAIFunc->GetHardCodingFunctionIndex() );
		for( DWORD j=0; j<m_pVecParam.size(); j++ ) {
			pHCF->SetNullFuncParam( m_pVecParam[j] );
		}
	}

	m_pAIFunc = NULL;
	SAFE_DELETE_PVEC( m_pVecParam );
}

void CFcAIFuncParam::AddParam( CFcParamVariable *pParam )
{
	m_pVecParam.push_back( pParam );
}

void CFcAIFuncParam::ProcessFunc()
{
	if( m_bPause == true ) return;
	// Function Setting
	if( m_pAIFunc->GetHardCodingFunctionIndex() == -1 ) {
		CFcParamVariable *pSour, *pDest;
		for( DWORD i=0; i<m_pAIFunc->GetParamCount(); i++ ) {
			pSour = m_pVecParam[i];
			pDest = m_pAIFunc->GetParam(i);
			switch( pDest->GetType() ) {
				case CFcParamVariable::INT:		pDest->SetVariable( pSour->GetVariableInt() );		break;
				case CFcParamVariable::FLOAT:	pDest->SetVariable( pSour->GetVariableFloat() );	break;
				case CFcParamVariable::CHAR:	pDest->SetVariable( pSour->GetVariableChar() );		break;
				case CFcParamVariable::STRING:	pDest->SetVariable( pSour->GetVariableString() );	break;
				case CFcParamVariable::VECTOR:	pDest->SetVariable( pSour->GetVariableVector() );	break;
				case CFcParamVariable::PTR:		pDest->SetVariable( pSour->GetVariablePtr() );		break;
				case CFcParamVariable::BOOLEAN:	pDest->SetVariable( pSour->GetVariableBool() );		break;
				case CFcParamVariable::RANDOM:	pDest->SetVariable( pSour->GetVariableRandom()[0], pSour->GetVariableRandom()[1] );		break;
			}
		}
		m_pAIFunc->Process();
	}
	else {
		CFcAIHardCodingFunction *pHCF = CFcAIHardCodingFunction::GetFunction( m_pAIFunc->GetHardCodingFunctionIndex() );
		pHCF->SetParamPtr( &m_pVecParam );

		pHCF->Process();
		pHCF->ExecuteFunction();
	}
}

void CFcAIFuncParam::ProcessCallbackFunc()
{
	if( m_bPause == true ) return;
	if( m_pAIFunc->GetHardCodingFunctionIndex() == -1 ) {
		if( !m_pAIFunc->IsCallbackFunc() ) return;	// <-펑션 내에서 다른 펑션을 인클루드 해서 쓰는게 안된다.. Callback 느려지는 것땜에 어쩔수 없이 너음.. 펑션 내 인클루드가 필요하면 이것 풀구.. Callback 처리 다르게 하자..
		CFcParamVariable *pSour, *pDest;
		for( DWORD i=0; i<m_pAIFunc->GetParamCount(); i++ ) {
			pSour = m_pVecParam[i];
			pDest = m_pAIFunc->GetParam(i);
			switch( pDest->GetType() ) {
				case CFcParamVariable::INT:		pDest->SetVariable( pSour->GetVariableInt() );		break;
				case CFcParamVariable::FLOAT:	pDest->SetVariable( pSour->GetVariableFloat() );	break;
				case CFcParamVariable::CHAR:	pDest->SetVariable( pSour->GetVariableChar() );		break;
				case CFcParamVariable::STRING:	pDest->SetVariable( pSour->GetVariableString() );	break;
				case CFcParamVariable::VECTOR:	pDest->SetVariable( pSour->GetVariableVector() );	break;
				case CFcParamVariable::PTR:		pDest->SetVariable( pSour->GetVariablePtr() );		break;
				case CFcParamVariable::BOOLEAN:	pDest->SetVariable( pSour->GetVariableBool() );		break;
				case CFcParamVariable::RANDOM:	pDest->SetVariable( pSour->GetVariableRandom()[0], pSour->GetVariableRandom()[1] );		break;
			}
		}
		m_pAIFunc->ProcessCallbackElement();
	}
	else {
		CFcAIHardCodingFunction *pHCF = CFcAIHardCodingFunction::GetFunction( m_pAIFunc->GetHardCodingFunctionIndex() );
		if( pHCF->GetCallbackType() == AI_CALLBACK_NONE ) return;
		for( int i=0; i<pHCF->GetCallbackTypeCount(); i++ ) {
			if( m_pAIFunc->GetParent()->m_bCallbackProcess[pHCF->GetCallbackType(i)] == TRUE ) {
				pHCF->SetParamPtr( &m_pVecParam );
				pHCF->Process();
				pHCF->ExecuteCallbackFunction( pHCF->GetCallbackType(i) );
			}
		}
	}
}

void CFcAIFuncParam::operator = ( const CFcAIFuncParam &Obj )
{
	for( DWORD i=0; i<Obj.m_pVecParam.size(); i++ ) {
		CFcParamVariable *pParam = new CFcParamVariable( Obj.m_pVecParam[i]->GetType() );
		*pParam = *Obj.m_pVecParam[i];
		m_pVecParam.push_back( pParam );

	}
}

void CFcAIFuncParam::Initialize()
{
	int nHardCodeIndex = m_pAIFunc->GetHardCodingFunctionIndex();
	if( nHardCodeIndex != -1 ) {
		CFcAIHardCodingFunction *pHCF = CFcAIHardCodingFunction::GetFunction( nHardCodeIndex );
		pHCF->SetParamPtr( &m_pVecParam );
		pHCF->Initialize();
	}
}