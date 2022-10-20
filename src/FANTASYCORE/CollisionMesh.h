#pragma		once

class NavCollisionCell
{
public:
	NavCollisionCell();
	~NavCollisionCell();

protected:
	D3DXVECTOR3 m_pVertex[3];
	NavCollisionCell *m_pSideCell[3];
};

struct NavTriangle {
	D3DXVECTOR3 vPos[3];
};

struct NavLine {
	D3DXVECTOR3 vPos[2];

	NavLine( D3DXVECTOR3 v1, D3DXVECTOR3 v2 ) {
		vPos[0] = v1;
		vPos[1] = v2;
	};
	NavLine() {
	}
};

struct NavCollisionGroup
{
	std::vector<NavTriangle> VecTri;
	std::vector<NavLine> VecLine;
	std::vector<NavLine> VecSortLine;
	std::vector<D3DXVECTOR3> VecPoint;

	DWORD dwColor;
};



class NavCollisionMesh
{
public:
	NavCollisionMesh();
	~NavCollisionMesh();

	// Initialize 包访 沏记
	void Initialize( int nGroupCount );
	void GenerateExternalLine();

	void AddTri( int nGroup, D3DXVECTOR3* pV1, D3DXVECTOR3* pV2, D3DXVECTOR3* pV3 );
	void Render();

	// 寇何 Call 沏记
	DWORD GetGroupCount();
	NavCollisionGroup *GetGroup( DWORD dwIndex );

	bool FindPath( D3DXVECTOR3 &vPos, D3DXVECTOR3 &vTarget, D3DXVECTOR3 &vResult );

	
protected:
	std::vector<NavCollisionGroup *> m_pVecGroup;

	void ProcessSinglePoint( NavCollisionGroup *pGroup );
	bool SortLine( NavCollisionGroup *pGroup );
	void GeneratePointList( NavCollisionGroup *pGroup );
	bool CheckExistLine( D3DXVECTOR3 &v1, D3DXVECTOR3 &v2, D3DXVECTOR3 *pTar );

//	void Perp( D3DXVECTOR2 &vSor, D3DXVECTOR2 &vResult );
//	bool Intersect( D3DXVECTOR3 &vSor1, D3DXVECTOR3 &vSor2, D3DXVECTOR3 &vTar1, D3DXVECTOR3 &vTar2 );
	bool CheckOpenPoint( DWORD dwGroup, D3DXVECTOR3 &vPos, D3DXVECTOR3 &vTar );
};
