#pragma once

struct SHADOWVERTEX
{ 
	D3DXVECTOR3 Position; 
	D3DXVECTOR3 Normal;
};

struct SHADOWVERTEX_BLEND
{ 
	D3DXVECTOR3 Position; 
	D3DXVECTOR3 Normal; 
	int	nBlendIndices;
};

struct SHADOWVERTEX_STREAM
{ 
	D3DXVECTOR3 Position; 
	DWORD	PackedNormal;
};

struct SHADOWVERTEX_BLEND_STREAM
{ 
	D3DXVECTOR3 Position; 
	DWORD	PackedNormal;
	int	nBlendIndices;
};

struct SHADOWMESH_EDGE
{
	WORD  V[2];             // Vert indices for the edge.
	DWORD Face0, Face1;     // Adjacent faces.
};
struct SHADOWMESH_FACE
{
	D3DXPLANE Plane;        // Plane of the face.
	WORD      V[3];         // Vert indices for the face.
};

class CBsShadowVolumeMgr
{
public:
	CBsShadowVolumeMgr();
	virtual ~CBsShadowVolumeMgr();

protected:
	int			m_nShadowVolumeVertexDeclIndices[3];
	int			m_nShadowVolumeRenderVertexDecl;
	int			m_nShadowVolumeMaterialIndex;

public:
	void Create();

	void SetShadowVolumeSetting( C3DDevice *pDevice );
	void RestoreShadowVolumeSetting( C3DDevice *pDevice );
	void RenderShadowVolume( C3DDevice *pDevice );

	CBsMaterial* GetShadowVolumeMaterial();
	void SetVertexDeclarationVolume(int nBoneLinkCount);

	static bool BuildShadowMesh( DWORD            dwNumFaces,    // Number of faces.
		const WORD*      pMeshIndices,  // Input triangle indices (3*dwNumFaces).
		const BYTE*      pMeshVertices, // Input vertices.
		DWORD            dwVertexSize,  // Size of each vertex in bytes.
		DWORD&           dwTempCount,   // Number of output vertices.
		D3DXVECTOR3*        pTempVerts,    // Output vertices (up to # of input verts).
		DWORD&           dwNumEdges,    // Number of output edges.
		SHADOWMESH_EDGE* pEdges,        // Output edges (up to 3*dwNumFaces).
		SHADOWMESH_FACE* pFaces );       // Output faces (dwNumFaces).

	static bool GenerateShadowMesh( D3DXVECTOR3 *pMeshVertices, WORD *pMeshIndices, int nVertexCount, int nPrimCount, LPDIRECT3DVERTEXBUFFER9 *ppVB, LPDIRECT3DINDEXBUFFER9 *ppIB, int *pPrimitiveCount );	
	static bool GenerateShadowMeshForBlend( SHADOWVERTEX_BLEND *pMeshVertices, WORD *pMeshIndices, int nVertexCount, int nPrimCount, LPDIRECT3DVERTEXBUFFER9 *ppVB, LPDIRECT3DINDEXBUFFER9 *ppIB, int *pPrimitiveCount );	
};
