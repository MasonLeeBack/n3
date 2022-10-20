#pragma once

class CFcGameObject;
class CShortPathFinder {
public:
	CShortPathFinder( CFcGameObject *pParent );
	virtual ~CShortPathFinder();

	void SetLength( float fLength );
	float GetLength();

	void Process();
	void Render();

	bool IsNewPath();
	bool IsLockPath() { return m_bLockPath; }
	void LockPath( D3DXVECTOR3 &vDir, int nValue );

protected:
	CFcGameObject *m_pParent;
	float m_fLength;
	float m_fTargetLength;
	static bool s_bAttrTable[256];	// 나중에 줄이자.. 8개면 되는데 ㅡㅡ;

	bool m_bLockPath;
	int m_nLockPathTick;
	D3DXVECTOR3 m_vLockDir;

	D3DXVECTOR3 m_vSource;
	D3DXVECTOR3 m_vDirection;
	D3DXVECTOR3 m_vResult;

	int m_nCurDir;
	int m_nPrevDir;
	int m_nCount;
	BYTE m_BlockAttr;
	static int s_pnArray[4][7];
	bool m_bCheckNewPath;
	std::vector<int> m_nVecBlockList;

	bool FindNextDirBlock( D3DXVECTOR3 &vPos );
	bool FindLastCloseBlock( D3DXVECTOR3 &vPos, D3DXVECTOR3 &vDir );

	bool GetPath( const D3DXVECTOR3 &vPos, const D3DXVECTOR3 &vDir );

	int CalcSearchDir( D3DXVECTOR3 *pDir );
	bool FindLeftPoint( int &nDir, int &nPosX, int &nPosZ );
	bool FindRightPoint( int &nDir, int &nPosX, int &nPosZ );
	int FindNextWayPoint();

	bool IsNoFinder();
	int GetBlockIndex( D3DXVECTOR3 &vPos );
	void CheckAndAddBlock();
	inline bool IsMoveBlock( D3DXVECTOR3 &vPos );
	inline bool IsMoveBlock( int nX, int nY );
	inline bool IsMoveBlock( float fX, float fY );
};