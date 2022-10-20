#pragma once

#define DEFAULT_SAVE_MATRIX_SIZE	1000

class CBsSaveMatrix
{
public:
	CBsSaveMatrix(void);
	virtual ~CBsSaveMatrix(void);

protected:
	std::vector< D3DXMATRIX * > m_ReallocList;
	std::vector< int > m_ReallocSizeList;
	D3DXMATRIX *m_pSaveMatrix;
	int m_nCurAllocSize;
	int m_nSaveMatrixSize;

public:
	void ResetAlloc();
	void ResetToDefault();
	D3DXMATRIX *AllocSaveMatrixBuffer( int nSize );

};
