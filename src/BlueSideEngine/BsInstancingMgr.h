#pragma once

class C3DDevice;
class CBsSubMesh;
class CBsImplMaterial;

#define MAX_INSTANCING_TABLE_COUNT 200
#define MAX_INSTANCING_COUNT			5000
#define INSTANCING_MATERIAL_COUNT 7
#define MORPHING_MATERIAL_INDEX 4
#define SHADOW_MATERIAL_INDEX 5
#define INSTANCING_VERTEXBUFFER_COUNT 3

struct InstancingSet
{
	int nSkinIndex;
	CBsSubMesh *pSubMesh;
	CBsImplMaterial *pImplMaterial;

	int nLodLevel;
	int nMaterialIndex;	
	
	std::vector< D3DXMATRIX* > matObjectList;

	InstancingSet() : nSkinIndex(-1), pSubMesh(0), nLodLevel(-1), nMaterialIndex(-1), pImplMaterial(0) {;}
};

class DeviceDataSet
{
public:
	LPDIRECT3DVERTEXBUFFER9 m_pVB;
	int		m_nStartBufferIndex;

public:
	DeviceDataSet() : m_pVB ( NULL ), m_nStartBufferIndex( 0 ) { }
	~DeviceDataSet() {
		if( m_pVB ) {
			m_pVB->Release();
			m_pVB = NULL;
		}
	}
};

class CBsMaterial;
class CBsInstancingMgr
{
public:
	CBsInstancingMgr();
	virtual ~CBsInstancingMgr();

public:
	enum 
	{
		INSTANCING_NONE = -1,
		INSTANCING_NORMAL = 0,
		INSTANCING_SHADOW = 1,
		INSTANCING_REFLECT = 2,
	};

protected:

	InstancingSet	m_InstancingTable[ MAX_INSTANCING_TABLE_COUNT ];
	int					m_nTableCount;

	int		m_nInstanceCount;
	int		m_nBufferSize;
	
	bool	m_bEnable;

	int		m_nCurrentUsingBufferIndex;

	DeviceDataSet		m_DeviceDataSet[ INSTANCING_VERTEXBUFFER_COUNT ];

	CBsMaterial		*m_pMaterial[ INSTANCING_MATERIAL_COUNT ];
	CBsMaterial		*m_pOriginalMaterial[ INSTANCING_MATERIAL_COUNT ];
	int					m_nInstancingMaterial[ INSTANCING_MATERIAL_COUNT];
	int					m_nOriginMaterial[ INSTANCING_MATERIAL_COUNT ];
	D3DXHANDLE	m_hVertsPerInstance[ INSTANCING_MATERIAL_COUNT];
	D3DXMATRIX	*m_pInstancingMatrix;

public:
	void SetCurrentUsingBufferIndex( int nIndex ) { m_nCurrentUsingBufferIndex = nIndex; }
	void RenderInstancingObjectList( C3DDevice *pDevice );
	bool AddInstancingObject( int nSkinIndex, CBsSubMesh *pSubMesh, int nLodLevel, D3DXMATRIX *pMatrix, CBsImplMaterial *pImplMaterial , D3DXMATRIX *pBoneMatrix);
	bool IsEnable() {return m_bEnable;}
	void SetEnable( bool bEnable) { m_bEnable = bEnable; }
	void SetBufferSize( int nBufferSize ) { m_nBufferSize = nBufferSize;}
	int GetBufferSize() { return m_nBufferSize;}
	
};

