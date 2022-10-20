#include "StdAfx.h"
#include "FcState.h"
#include "FcSOXFile.h"
#include "BsCommon.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG



CFcState::CFcState( const char *szFileName )
{
	CFcSOXFile Sox( szFileName );

	CFcSOXFile::SOX_Field Field;
	for( int i=1; i<=Sox.GetItemCount(); i++ ) {
		StateBlock *pBlock = new StateBlock;

		// State String
		Sox.GetField( i, 0, Field );
		pBlock->szStr = Field.uData.pValue;
		m_Hash.insert( Field.uData.pValue, i-1 );

		// Value Type
		Sox.GetField( i, 1, Field );
		switch( Field.nFldTyp ) {
			case CFcSOXFile::FT_INT:	pBlock->BlockType = BT_INT;		break;
			case CFcSOXFile::FT_FLOAT:	pBlock->BlockType = BT_FLOAT;	break;
		}

		// Enum 일 경우에..
		Sox.GetField( i, 2, Field );
		if( Field.uData.nValue != -1 ) {
			pBlock->BlockType = BT_ENUM;
		}

		pBlock->pValue = NULL;
		AddStateBlock( pBlock );
	}
}

CFcState::~CFcState()
{
	SAFE_DELETE_PVEC( m_pVecBlockList );
}

void CFcState::AddStateBlock( StateBlock *pBlock )
{
	m_pVecBlockList.push_back( pBlock );
}

int CFcState::GetIndex( const char *szStr )
{
	int *pResult = m_Hash.search( szStr );
	return ( pResult == NULL ) ? -1 : *pResult;
}

CFcState::BLOCK_TYPE CFcState::GetStateType( DWORD dwIndex )
{
	return m_pVecBlockList[dwIndex]->BlockType;
}

CFcState::BLOCK_TYPE CFcState::GetStateType( const char *szStr )
{
	int nIndex = GetIndex( szStr );
	return ( nIndex == -1 ) ? BT_UNKNOWN : m_pVecBlockList[nIndex]->BlockType;
}

void *CFcState::GetStateValue( DWORD dwIndex )
{
	return &m_pVecBlockList[dwIndex]->pValue;
}

int CFcState::GetStateValueInt( DWORD dwIndex )
{
	return m_pVecBlockList[dwIndex]->nValue;
}

float CFcState::GetStateValueFloat( DWORD dwIndex )
{
	return m_pVecBlockList[dwIndex]->fValue;
}

void *CFcState::GetStateValuePtr( const char *szStr )
{
	int nIndex = GetIndex( szStr );
	return ( nIndex == -1 ) ? NULL : &m_pVecBlockList[nIndex]->pValue;
}

void *CFcState::GetStateValue( const char *szStr )
{
	int nIndex = GetIndex( szStr );
	return ( nIndex == -1 ) ? NULL : m_pVecBlockList[nIndex]->pValue;
}

int CFcState::GetStateValueInt( const char *szStr )
{
	int nIndex = GetIndex( szStr );
	return ( nIndex == -1 ) ? NULL : m_pVecBlockList[nIndex]->nValue;
}

float CFcState::GetStateValueFloat( const char *szStr )
{
	int nIndex = GetIndex( szStr );
	return ( nIndex == -1 ) ? NULL : m_pVecBlockList[nIndex]->fValue;
}

void CFcState::SetStateValue( DWORD dwIndex, void *pValue )
{
	m_pVecBlockList[dwIndex]->pValue = pValue;
}

void CFcState::SetStateValueInt( DWORD dwIndex, int nValue )
{
	m_pVecBlockList[dwIndex]->nValue = nValue;
}

void CFcState::SetStateValueFloat( DWORD dwIndex, float fValue )
{
	m_pVecBlockList[dwIndex]->fValue = fValue;
}

void CFcState::SetStateValue( const char *szStr, void *pValue )
{
	int nIndex = GetIndex( szStr );
	if( nIndex != -1 ) SetStateValue( nIndex, pValue );
}

void CFcState::SetStateValueInt( const char *szStr, int nValue )
{
	int nIndex = GetIndex( szStr );
	if( nIndex != -1 ) SetStateValueInt( nIndex, nValue );
}

void CFcState::SetStateValueFloat( const char *szStr, float fValue )
{
	int nIndex = GetIndex( szStr );
	if( nIndex != -1 ) SetStateValueFloat( nIndex, fValue );
}
