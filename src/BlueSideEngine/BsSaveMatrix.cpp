#include "stdafx.h"
#include "BsObject.h"
#include ".\bssavematrix.h"

CBsSaveMatrix::CBsSaveMatrix(void)
{
	m_nSaveMatrixSize = DEFAULT_SAVE_MATRIX_SIZE;
	m_pSaveMatrix = ( D3DXMATRIX * )malloc( sizeof( D3DXMATRIX ) * m_nSaveMatrixSize );
	m_nCurAllocSize = 0;
}

CBsSaveMatrix::~CBsSaveMatrix(void)
{
	if( m_pSaveMatrix )
	{
		free( m_pSaveMatrix );
		m_pSaveMatrix = NULL;
	}

	int nSize = ( int )m_ReallocList.size();
	for( int i = 0; i < nSize; i++ )
	{
		free( m_ReallocList[ i ] );
	}	
}

D3DXMATRIX *CBsSaveMatrix::AllocSaveMatrixBuffer( int nSize )
{
	if( m_nSaveMatrixSize - m_nCurAllocSize < nSize )
	{
		D3DXMATRIX *pRealloc;

		pRealloc = ( D3DXMATRIX * )malloc( sizeof( D3DXMATRIX ) * nSize );
		BsAssert( pRealloc );
		m_ReallocList.push_back( pRealloc );
		m_ReallocSizeList.push_back( nSize );
		return pRealloc;
	}

	D3DXMATRIX *pRet;

	pRet = m_pSaveMatrix + m_nCurAllocSize;
	m_nCurAllocSize += nSize;

	return pRet;
}

void CBsSaveMatrix::ResetAlloc()
{ 
	int nSize = ( int )m_ReallocList.size();
	if( nSize )
	{
		int nTotalReallocSize = 0;
		for( int i = 0; i < nSize; i++ )
		{
			free( m_ReallocList[ i ] );
			nTotalReallocSize += m_ReallocSizeList[ i ];
		}
		m_ReallocList.clear();
		m_ReallocSizeList.clear();
		m_pSaveMatrix = ( D3DXMATRIX *)realloc( m_pSaveMatrix, sizeof( D3DXMATRIX ) * ( m_nSaveMatrixSize + nTotalReallocSize ) );
		BsAssert(m_pSaveMatrix);
		m_nSaveMatrixSize += nTotalReallocSize;

		DebugString("Render Index(%d), size = %d : Save Matrix Buffer Reallocation!\n", CBsObject::GetRenderBufferIndex(), m_nSaveMatrixSize);
	}
	m_nCurAllocSize = 0; 
}

void CBsSaveMatrix::ResetToDefault()
{
	int nSize = ( int )m_ReallocList.size();
	for( int i = 0; i < nSize; i++ )
	{
		free( m_ReallocList[ i ] );
	}

	m_ReallocList.clear();
	m_ReallocSizeList.clear();

	m_pSaveMatrix = ( D3DXMATRIX *)realloc( m_pSaveMatrix, sizeof( D3DXMATRIX ) * DEFAULT_SAVE_MATRIX_SIZE );
	BsAssert(m_pSaveMatrix);
	m_nSaveMatrixSize = DEFAULT_SAVE_MATRIX_SIZE;

	m_nCurAllocSize = 0;
}
