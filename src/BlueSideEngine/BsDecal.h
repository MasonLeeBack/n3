#pragma once

#define DECAL_ALLOC_VERTEX_SIZE 50
#define DECAL_ALLOC_INDEX_SIZE 100

struct DECAL_VERTEX
{
	D3DXVECTOR3 pos;
	D3DXVECTOR2 uv;
	D3DCOLOR	color;
};


class CBsDecalVertex
{
public:
	D3DXVECTOR3						*m_pVertices;
	unsigned short						*m_pIndices;

	int										m_nVertexCount;
	int										m_nIndexCount;

	Box3									m_Obb;

	bool									m_bShareIndex;

public:

	CBsDecalVertex()
	{
        m_pVertices = NULL;
		m_pIndices = NULL;
		m_nVertexCount = 0;
		m_nIndexCount = 0;
		m_bShareIndex = false;
	}
	~CBsDecalVertex()
	{
		Release();
	}
	void Release()
	{
		if( m_pVertices ) {
			delete [] m_pVertices;
			m_pVertices = NULL;
		}
		if( !m_bShareIndex && m_pIndices ) {
			delete [] m_pIndices;
			m_pIndices = NULL;
		}
	}
};

struct CBsDecalGroup
{
	int nTextureIndex;
	int nMode;
	int nVertexCount;
	int nAllocVertexCount;
	int nIndexCount;
	int nAllocIndexCount;
	DECAL_VERTEX *pVertices;
	unsigned short *pIndices;
};

#define POINTLIGHT_DECAL	-1

class CBsDecal
{
public:
	bool									m_bEnable;
	int										m_nTextureID;
	D3DCOLOR							m_dwColor;

	static std::map< INT64, CBsDecalVertex* > s_DecalPool[ DOUBLE_BUFFERING ];

protected:

	CBsDecalVertex					*m_pDecalVertex;

	int										m_nLifeTime;
	int										m_nCreateTime;	

	int										m_nMode;
public:
	D3DXVECTOR3						m_UVTransX;
	D3DXVECTOR3						m_UVTransY;

public:	
	D3DXVECTOR3 *GetVertexPtr() { return m_pDecalVertex->m_pVertices;}
	unsigned short*GetIndexPtr() { return m_pDecalVertex->m_pIndices;}
	int	GetVertexSize() { return m_pDecalVertex->m_nVertexCount;}
	int GetIndexSize() { return m_pDecalVertex->m_nIndexCount; }
	void Initialize(float fX, float fZ, int nTextureID, float fRadius, int nLifeTime, int nCreateTime, float fRotAngle, D3DCOLOR dwColor, int nMode);		
	//void Render(C3DDevice *pDevice);
	int	GetCreateTime() { return m_nCreateTime; }
	int GetTotalLife() { return m_nLifeTime; }	
	int	GetTextureID() { return m_nTextureID;}
	int GetBlendMode() { return m_nMode;}
	const Box3&	GetObb() { return m_pDecalVertex->m_Obb; }

};

class C3DDevice;
class CBsDecalManager 
{
public:
	CBsDecalManager();
	virtual ~CBsDecalManager();

protected:
	std::deque< CBsDecal >			m_DecalList;
	std::vector< CBsDecalGroup > m_DecalGroupList[ DOUBLE_BUFFERING ];
	
	D3DXHANDLE	m_hVP;
	D3DXHANDLE	m_hTexture;
	D3DXHANDLE	m_hDiffuse;
	D3DXHANDLE	m_hUVMat;

	int					m_nDecalVertexDeclIndex;
	int					m_nMaterialIndex;
	int					m_nProcessTick;
	int					m_nRenderTick;
	int					m_nPointLightDecalTextureIndex;

public:
	void ClearAll();
	void ClearDecalGroup();
	void ClearDecalPool();
	void AddDecal(float fX, float fZ, int nTexture, float fRadius, int nLifeTime, float fRotAngle, D3DCOLOR dwColor, int nMode );
	void Update();
	void Render(C3DDevice *pDevice);
};
