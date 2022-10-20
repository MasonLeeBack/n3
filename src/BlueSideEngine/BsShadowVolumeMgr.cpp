#include "stdafx.h"
#include "BsStream.h"
#include "BsKernel.h"
#include "BsMaterial.h"
#include "BsShadowVolumeMgr.h"

CBsShadowVolumeMgr::CBsShadowVolumeMgr()
{	
	for(int i=0;i<3;++i) {
		m_nShadowVolumeVertexDeclIndices[i] = -1;
	}
	m_nShadowVolumeRenderVertexDecl = -1;
	m_nShadowVolumeMaterialIndex = -1;
}

CBsShadowVolumeMgr::~CBsShadowVolumeMgr()
{
	SAFE_RELEASE_MATERIAL(m_nShadowVolumeMaterialIndex);

	for(int i=0 ; i<3 ; ++i) {	
		SAFE_RELEASE_VD(m_nShadowVolumeVertexDeclIndices[i]);
	}
	SAFE_RELEASE_VD(m_nShadowVolumeRenderVertexDecl);
}

void CBsShadowVolumeMgr::Create()
{
	if(m_nShadowVolumeMaterialIndex == -1) {
		char fullName[_MAX_PATH];
		strcpy(fullName, g_BsKernel.GetShaderDirectory());
		strcat(fullName, "ShadowVolume.fx");
		m_nShadowVolumeMaterialIndex = g_BsKernel.LoadMaterial(fullName);
	}

	// shadow volume declaration

	D3DVERTEXELEMENT9 decl0[] =
	{
		{ 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
#ifdef _XBOX
		{ 0, 12, D3DDECLTYPE_HEND3N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
#else
		{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
#endif
		//{ 1, 0, declNormal, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		D3DDECL_END()
	};
	m_nShadowVolumeVertexDeclIndices[0] = g_BsKernel.LoadVertexDeclaration(decl0);

	D3DVERTEXELEMENT9 decl1[] =
	{
		{ 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
#ifdef _XBOX
		{ 0, 12, D3DDECLTYPE_HEND3N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0, 16, D3DDECLTYPE_SHORT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0 },
#else
		{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0, 24, D3DDECLTYPE_SHORT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0 },
#endif
		D3DDECL_END()
	};
	m_nShadowVolumeVertexDeclIndices[1] = g_BsKernel.LoadVertexDeclaration(decl1);
	m_nShadowVolumeVertexDeclIndices[2] = g_BsKernel.LoadVertexDeclaration(decl1);

	D3DVERTEXELEMENT9 declRender[] =
	{
		{ 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		D3DDECL_END()
	};
	m_nShadowVolumeRenderVertexDecl = g_BsKernel.LoadVertexDeclaration(declRender);
}

void CBsShadowVolumeMgr::SetShadowVolumeSetting( C3DDevice *pDevice )
{	

	// Disable z-buffer writes (note: z-testing still occurs)
	pDevice->SetRenderState( D3DRS_ZWRITEENABLE,  FALSE );
#ifdef _XBOX
	pDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_GREATER );
#else
	pDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESS );
#endif

	bool bRenderShadowVolume = false;

	// Set up stencil compare function, reference value, and masks.
	// Stencil test passes if ((ref & mask) cmpfn (stencil & mask)) is true.
	// Make sure that no pixels get drawn to the frame buffer

	if( !bRenderShadowVolume ) {
		pDevice->SetRenderState( D3DRS_COLORWRITEENABLE, 0 );
		pDevice->SetRenderState( D3DRS_STENCILENABLE,    TRUE);
		// With 2-sided stencil, we can avoid rendering twice:		
		pDevice->SetRenderState( D3DRS_TWOSIDEDSTENCILMODE, TRUE);
	}
	else {
		pDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	}

	
//#ifdef _XBOX

	// With 2-sided stencil, we can avoid rendering twice:
	pDevice->SetRenderState( D3DRS_STENCILMASK,      0xffffffff );
	pDevice->SetRenderState( D3DRS_STENCILWRITEMASK, 0xffffffff );
	pDevice->SetRenderState( D3DRS_STENCILREF,       0x00000001 );

#ifdef _XBOX
	pDevice->SetRenderState( D3DRS_CCW_STENCILMASK,  0xffffffff );
	pDevice->SetRenderState( D3DRS_CCW_STENCILWRITEMASK, 0xffffffff );
	pDevice->SetRenderState( D3DRS_CCW_STENCILREF,   0x00000001 );
#endif

	pDevice->SetRenderState( D3DRS_STENCILFUNC,      D3DCMP_ALWAYS );
	pDevice->SetRenderState( D3DRS_CCW_STENCILFUNC,  D3DCMP_ALWAYS );

	bool bDepthPassOrFail = false;
	bool bInverseExtrude = false;

	if( bDepthPassOrFail ) {
		pDevice->SetRenderState( D3DRS_STENCILPASS,      bInverseExtrude ? D3DSTENCILOP_DECR : D3DSTENCILOP_INCR);
		pDevice->SetRenderState( D3DRS_STENCILZFAIL,      D3DSTENCILOP_KEEP);
		pDevice->SetRenderState( D3DRS_STENCILFAIL,      D3DSTENCILOP_KEEP );

		pDevice->SetRenderState( D3DRS_CCW_STENCILPASS,  bInverseExtrude ? D3DSTENCILOP_INCR : D3DSTENCILOP_DECR);
		pDevice->SetRenderState( D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_KEEP);
		pDevice->SetRenderState( D3DRS_CCW_STENCILFAIL,  D3DSTENCILOP_KEEP );
	}
	else {
		pDevice->SetRenderState( D3DRS_STENCILPASS,      D3DSTENCILOP_KEEP);
		pDevice->SetRenderState( D3DRS_STENCILZFAIL,     bInverseExtrude ? D3DSTENCILOP_INCR : D3DSTENCILOP_DECR );
		pDevice->SetRenderState( D3DRS_STENCILFAIL,      D3DSTENCILOP_KEEP );

		pDevice->SetRenderState( D3DRS_CCW_STENCILPASS,  D3DSTENCILOP_KEEP);
		pDevice->SetRenderState( D3DRS_CCW_STENCILZFAIL, bInverseExtrude ? D3DSTENCILOP_DECR : D3DSTENCILOP_INCR);
		pDevice->SetRenderState( D3DRS_CCW_STENCILFAIL,  D3DSTENCILOP_KEEP );	
	}

//#endif	
	//pDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME);

	pDevice->SetRenderState( D3DRS_CULLMODE,  D3DCULL_NONE);
	static float fSlopeScaled = -0.99f;
	static float fDepthBias = 0.f;
	
	pDevice->SetRenderState( D3DRS_SLOPESCALEDEPTHBIAS , *((DWORD*)&fSlopeScaled));
	//pDevice->SetRenderState( D3DRS_DEPTHBIAS, *((DWORD*)&fDepthBias));
}

void CBsShadowVolumeMgr::RestoreShadowVolumeSetting( C3DDevice *pDevice )
{
	pDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID);

	pDevice->SetRenderState( D3DRS_STENCILENABLE,    FALSE );
	pDevice->SetRenderState( D3DRS_TWOSIDEDSTENCILMODE, FALSE );
	pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
	pDevice->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA);

	pDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
#ifdef _XBOX
	pDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_GREATEREQUAL);
#else
	pDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL);	
#endif

	pDevice->SetRenderState( D3DRS_SLOPESCALEDEPTHBIAS , 0);
	pDevice->SetRenderState( D3DRS_DEPTHBIAS, 0);
}

void CBsShadowVolumeMgr::RenderShadowVolume( C3DDevice *pDevice )
{	
	static bool bUseShadowVolume = true;

	if( !bUseShadowVolume ) return;

	CBsCamera* pActiveCamera = g_BsKernel.GetActiveCamera();
	std::vector<int>& RenderObjectList = pActiveCamera->GetShadowObjectList();

	int nDrawCount = 0;
	
	for( unsigned int i=0; i<RenderObjectList.size(); ++i) {
#ifndef _LTCG
		CBsKernel::s_nTrackingIndex = i;
#endif
		CBsObject* pObject = g_BsKernel.GetEngineObjectPtr(RenderObjectList[i]);
		BsAssert(pObject && "Invalid Object Pointer");
		
		if( pObject == NULL ||
			pObject->GetShadowCastType() != BS_SHADOW_VOLUME ||
			pObject->GetDistanceFromCam() > SHADOW_LIMIT_FROMVIEW ) {
			continue;
		}
		pObject->PreRenderShadowVolume(pDevice);
		pObject->RenderShadowVolume(pDevice);
		pObject->PostRender(pDevice);
		nDrawCount++;
	}

	if( nDrawCount == 0 ) {
		return;
	}
	/*
	*/

	D3DXVECTOR3 v[4] =
	{
		D3DXVECTOR3(-1,1,0),
		D3DXVECTOR3( 1,1,0),
		D3DXVECTOR3(-1,-1,0),
		D3DXVECTOR3(1,-1,0),
	};

	CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr( m_nShadowVolumeMaterialIndex );
	
	pMaterial->BeginMaterial(3, 0);
	pMaterial->BeginPass(0);	
	g_BsKernel.SetVertexDeclaration( m_nShadowVolumeRenderVertexDecl );

	pDevice->SetRenderState( D3DRS_STENCILENABLE, TRUE );
	pDevice->SetRenderState( D3DRS_STENCILREF,    0x00000000 );
	pDevice->SetRenderState( D3DRS_STENCILFUNC,   D3DCMP_NOTEQUAL);
	pDevice->SetRenderState( D3DRS_STENCILZFAIL,   D3DSTENCILOP_KEEP );
	pDevice->SetRenderState( D3DRS_STENCILFAIL,   D3DSTENCILOP_KEEP );
	pDevice->SetRenderState( D3DRS_STENCILPASS,   D3DSTENCILOP_KEEP );

	// Set renderstates (disable z-buffering and turn on alphablending)	
	pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);	
	pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
	pDevice->SetRenderState(D3DRS_ALPHAREF, 0x01);

	pDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
	pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	pDevice->SetRenderState( D3DRS_ZWRITEENABLE,         FALSE );
	pDevice->SetRenderState( D3DRS_ZENABLE,         FALSE );

	pMaterial->CommitChanges();
	pDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, v, sizeof(D3DXVECTOR3) );	
	pMaterial->EndPass() ;
	pMaterial->EndMaterial();

	// restore
	pDevice->SetRenderState( D3DRS_ZENABLE,         TRUE );
	pDevice->SetRenderState( D3DRS_STENCILENABLE,   FALSE );

	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
	pDevice->SetRenderState(D3DRS_ALPHAREF, pDevice->GetAlphaRefValue());	
}

CBsMaterial* CBsShadowVolumeMgr::GetShadowVolumeMaterial()
{
	if(m_nShadowVolumeMaterialIndex == -1) {
		return NULL;
	}
	else {
		return g_BsKernel.GetMaterialPtr( m_nShadowVolumeMaterialIndex );
	}
}

void CBsShadowVolumeMgr::SetVertexDeclarationVolume(int nBoneLinkCount)
{
	nBoneLinkCount = BsMin(2, nBoneLinkCount);
	g_BsKernel.SetVertexDeclaration( m_nShadowVolumeVertexDeclIndices[nBoneLinkCount] );
}

struct CEdgeMapping
{
	int m_anOldEdge[2];  // vertex index of the original edge
	int m_aanNewEdge[2][2]; // vertex indexes of the new edge
	// First subscript = index of the new edge
	// Second subscript = index of the vertex for the edge

public:
	CEdgeMapping()
	{
		memset( m_anOldEdge, -1, sizeof(m_anOldEdge));
		memset( m_aanNewEdge, -1, sizeof(m_aanNewEdge));
	}
};


//--------------------------------------------------------------------------------------
// Takes an array of CEdgeMapping objects, then returns an index for the edge in the
// table if such entry exists, or returns an index at which a new entry for the edge
// can be written.
// nV1 and nV2 are the vertex indexes for the old edge.
// nCount is the number of elements in the array.
// The function returns -1 if an available entry cannot be found.  In reality,
// this should never happens as we should have allocated enough memory.
int FindEdgeInMappingTable( int nV1, int nV2, CEdgeMapping *pMapping, int nCount )
{
	for( int i = 0; i < nCount; ++i )
	{
		// If both vertex indexes of the old edge in mapping entry are -1, then
		// we have searched every valid entry without finding a match.  Return
		// this index as a newly created entry.
		if( ( pMapping[i].m_anOldEdge[0] == -1 && pMapping[i].m_anOldEdge[1] == -1 ) ||

			// Or if we find a match, return the index.
			( pMapping[i].m_anOldEdge[1] == nV1 && pMapping[i].m_anOldEdge[0] == nV2 ) )
		{
			return i;
		}
	}

	return -1;  // We should never reach this line
}

struct VERTEX_INFO
{
	D3DXVECTOR3 Pos;
	int nBlendIndices;
	int nIndex;
	int nNewIndex;
	VERTEX_INFO() {
		nBlendIndices = 0;
	}
	bool operator == ( const VERTEX_INFO &rhs ) {
		if( D3DXVec3LengthSq(&(Pos-rhs.Pos)) < 0.01f && nBlendIndices == rhs.nBlendIndices ) return true;
		else return false;		
	}
};

struct SortVertex 
{
	bool operator () ( const VERTEX_INFO& lhs, const VERTEX_INFO& rhs )
	{
		return memcmp( &lhs, &rhs, sizeof(D3DXVECTOR3)+sizeof(int) ) < 0;
	}
};

void GeneratePointReps( int nVertices, SHADOWVERTEX_BLEND *pVBData, DWORD *pdwPtRep)
{
	int i;
	std::vector< VERTEX_INFO > vertexList;

	for(i = 0; i < nVertices; i++) 
	{
		VERTEX_INFO va;
		va.Pos = pVBData[ i ].Position;
		va.nBlendIndices = pVBData[i].nBlendIndices;
		va.nIndex = i;
		va.nNewIndex = i;
		vertexList.push_back( va );
	}
	std::stable_sort( vertexList.begin(), vertexList.end(), SortVertex() );

	int nVertexIndex = vertexList[0].nIndex;
	for( i = 1; i < nVertices; i++)
	{
		if( !(vertexList[i-1] == vertexList[i]) ) {
			nVertexIndex = vertexList[i].nIndex;
		}
		vertexList[i].nNewIndex = nVertexIndex;
	}

	for(i = 0; i < nVertices; i++) {
		pdwPtRep[ vertexList[i].nIndex ] = vertexList[i].nNewIndex;
	}
}

void GeneratePointReps( int nVertices, D3DXVECTOR3 *pVBData, DWORD *pdwPtRep)
{	
	int i;
	std::vector< VERTEX_INFO > vertexList;

	for(i = 0; i < nVertices; i++) 
	{
		VERTEX_INFO va;
		va.Pos = pVBData[ i ];
		va.nIndex = i;
		va.nNewIndex = i;
		vertexList.push_back( va );
	}
	std::stable_sort( vertexList.begin(), vertexList.end(), SortVertex() );

	int nVertexIndex = vertexList[0].nIndex;
	for( i = 1; i < nVertices; i++)
	{
		if( !(vertexList[i-1] == vertexList[i]) ) {
			nVertexIndex = vertexList[i].nIndex;
		}
		vertexList[i].nNewIndex = nVertexIndex;
	}

	for(i = 0; i < nVertices; i++) {
		pdwPtRep[ vertexList[i].nIndex ] = vertexList[i].nNewIndex;
	}
}

bool CBsShadowVolumeMgr::GenerateShadowMesh( D3DXVECTOR3 *pMeshVertices, WORD *pMeshIndices, int nVertexCount, int nPrimCount, LPDIRECT3DVERTEXBUFFER9 *ppVB, LPDIRECT3DINDEXBUFFER9 *ppIB, int *pPrimitiveCount )
{
	int i;
	DWORD *pdwPtRep = new DWORD[ nVertexCount ];
	GeneratePointReps(  nVertexCount, pMeshVertices, pdwPtRep );

	// Maximum number of unique edges = Number of faces * 3
	DWORD dwNumEdges = nPrimCount * 3;
	CEdgeMapping *pMapping = new CEdgeMapping[ dwNumEdges ];

	int nNumMaps = 0;  // Number of entries that exist in pMapping

	// Create a new mesh
	SHADOWVERTEX *pNewVBData = new SHADOWVERTEX[ nPrimCount * 3 ];
	WORD *pdwNewIBData = new WORD[ (nPrimCount + dwNumEdges * 2) * 3 ];

	// nNextIndex is the array index in IB that the next vertex index value
	// will be store at.
	int nNextIndex = 0;

	//memset( pNewVBData, 0, (nPrimCount * 3) * sizeof(SHADOWVERTEX) );
	//memset( pdwNewIBData, 0, sizeof(WORD) * (nPrimCount + dwNumEdges * 2) * 3 );

	// pNextOutVertex is the location to write the next
	// vertex to.
	SHADOWVERTEX *pNextOutVertex = pNewVBData;

	// Iterate through the faces.  For each face, output new
	// vertices and face in the new mesh, and write its edges
	// to the mapping table.

	for( i = 0; i < nPrimCount; i++ )
	{
		// Copy the vertex data for all 3 vertices
		memcpy( pNextOutVertex, pMeshVertices + pMeshIndices[i * 3], sizeof(D3DXVECTOR3) );
		memcpy( pNextOutVertex + 1, pMeshVertices + pMeshIndices[i * 3 + 1], sizeof(D3DXVECTOR3));
		memcpy( pNextOutVertex + 2, pMeshVertices + pMeshIndices[i * 3 + 2], sizeof(D3DXVECTOR3));

		// Write out the face
		pdwNewIBData[nNextIndex++] = i * 3;
		pdwNewIBData[nNextIndex++] = i * 3 + 1;
		pdwNewIBData[nNextIndex++] = i * 3 + 2;

		// Compute the face normal and assign it to
		// the normals of the vertices.
		D3DXVECTOR3 v1, v2;  // v1 and v2 are the edge vectors of the face
		D3DXVECTOR3 vNormal;
		v1 = *(D3DXVECTOR3*)(pNextOutVertex + 1) - *(D3DXVECTOR3*)pNextOutVertex;
		v2 = *(D3DXVECTOR3*)(pNextOutVertex + 2) - *(D3DXVECTOR3*)(pNextOutVertex + 1);
		D3DXVec3Cross( &vNormal, &v1, &v2 );
		D3DXVec3Normalize( &vNormal, &vNormal );

		pNextOutVertex->Normal = vNormal;
		(pNextOutVertex + 1)->Normal = vNormal;
		(pNextOutVertex + 2)->Normal = vNormal;

		pNextOutVertex += 3;

		// Add the face's edges to the edge mapping table

		// Edge 1
		int nIndex;
		DWORD nVertIndex[3] = { pdwPtRep[ pMeshIndices[i * 3] ],
			pdwPtRep[ pMeshIndices[i * 3 + 1] ],
			pdwPtRep[ pMeshIndices[i * 3 + 2] ] };
		nIndex = FindEdgeInMappingTable( nVertIndex[0], nVertIndex[1], pMapping, dwNumEdges );

		// If error, we are not able to proceed, so abort.
		if( -1 == nIndex )
		{
// [PREFIX:beginmodify] 2006/2/15 junyash PS#5175 reports CBsShadowVolumeMgr::GenerateShadowMesh() leaks memory.
			delete [] pdwPtRep;
			delete [] pNewVBData;
			delete [] pdwNewIBData;
			delete [] pMapping;
// [PREFIX:endmodify] junyash
			return false;
		}

		if( pMapping[nIndex].m_anOldEdge[0] == -1 && pMapping[nIndex].m_anOldEdge[1] == -1 )
		{
			// No entry for this edge yet.  Initialize one.
			pMapping[nIndex].m_anOldEdge[0] = nVertIndex[0];
			pMapping[nIndex].m_anOldEdge[1] = nVertIndex[1];
			pMapping[nIndex].m_aanNewEdge[0][0] = i * 3;
			pMapping[nIndex].m_aanNewEdge[0][1] = i * 3 + 1;

			++nNumMaps;
		} else
		{
			// An entry is found for this edge.  Create
			// a quad and output it.
			// [PREFIX:beginmodify] 2006/2/25 realgaia PS#5803 reports CBsShadowVolumeMgr::GenerateShadowMesh() bounds violation (overflow) 'pMapping[nNumMaps-1]'
			BsAssert( nNumMaps > 0 && (DWORD)nNumMaps <= dwNumEdges );
			// [PREFIX:endmodify] 2006/2/25 realgaia PS#5803 reports CBsShadowVolumeMgr::GenerateShadowMesh() bounds violation (overflow) 'pMapping[nNumMaps-1]'


			pMapping[nIndex].m_aanNewEdge[1][0] = i * 3;      // For clarity
			pMapping[nIndex].m_aanNewEdge[1][1] = i * 3 + 1;

			// First triangle
			pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[0][1];
			pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[0][0];
			pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[1][0];

			// Second triangle
			pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[1][1];
			pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[1][0];
			pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[0][0];

			// pMapping[nIndex] is no longer needed. Copy the last map entry
			// over and decrement the map count.

			pMapping[nIndex] = pMapping[nNumMaps-1];
			memset( &pMapping[nNumMaps-1], 0xFF, sizeof( pMapping[nNumMaps-1] ));
			--nNumMaps;
		}

		// Edge 2
		nIndex = FindEdgeInMappingTable( nVertIndex[1], nVertIndex[2], pMapping, dwNumEdges );

		// If error, we are not able to proceed, so abort.
		if( -1 == nIndex )
		{
// [PREFIX:beginmodify] 2006/2/15 junyash PS#5175 reports CBsShadowVolumeMgr::GenerateShadowMesh() leaks memory.
			delete [] pdwPtRep;
			delete [] pNewVBData;
			delete [] pdwNewIBData;
			delete [] pMapping;
// [PREFIX:endmodify] junyash
			return false;		
		}

		if( pMapping[nIndex].m_anOldEdge[0] == -1 && pMapping[nIndex].m_anOldEdge[1] == -1 )
		{
			pMapping[nIndex].m_anOldEdge[0] = nVertIndex[1];
			pMapping[nIndex].m_anOldEdge[1] = nVertIndex[2];
			pMapping[nIndex].m_aanNewEdge[0][0] = i * 3 + 1;
			pMapping[nIndex].m_aanNewEdge[0][1] = i * 3 + 2;

			++nNumMaps;
		} 
		else
		{
			// An entry is found for this edge.  Create
			// a quad and output it.

// [PREFIX:beginmodify] 2006/2/25 realgaia PS#5803 reports CBsShadowVolumeMgr::GenerateShadowMesh() bounds violation (overflow) 'pMapping[nNumMaps-1]'
			BsAssert( nNumMaps > 0 && (DWORD)nNumMaps <= dwNumEdges );
// [PREFIX:endmodify] 2006/2/25 realgaia PS#5803 reports CBsShadowVolumeMgr::GenerateShadowMesh() bounds violation (overflow) 'pMapping[nNumMaps-1]'

			pMapping[nIndex].m_aanNewEdge[1][0] = i * 3 + 1;
			pMapping[nIndex].m_aanNewEdge[1][1] = i * 3 + 2;

			// First triangle
			pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[0][1];
			pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[0][0];
			pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[1][0];

			// Second triangle
			pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[1][1];
			pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[1][0];
			pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[0][0];

			// pMapping[nIndex] is no longer needed. Copy the last map entry
			// over and decrement the map count.

			pMapping[nIndex] = pMapping[nNumMaps-1];
			memset( &pMapping[nNumMaps-1], 0xFF, sizeof( pMapping[nNumMaps-1] ) );
			--nNumMaps;
		}

		// Edge 3
		nIndex = FindEdgeInMappingTable( nVertIndex[2], nVertIndex[0], pMapping, dwNumEdges );

		// If error, we are not able to proceed, so abort.
		if( -1 == nIndex )
		{
// [PREFIX:beginmodify] 2006/2/15 junyash PS#5175 reports CBsShadowVolumeMgr::GenerateShadowMesh() leaks memory.
			delete [] pdwPtRep;
			delete [] pNewVBData;
			delete [] pdwNewIBData;
			delete [] pMapping;
// [PREFIX:endmodify] junyash
			return false;
		}

		if( pMapping[nIndex].m_anOldEdge[0] == -1 && pMapping[nIndex].m_anOldEdge[1] == -1 )
		{
			pMapping[nIndex].m_anOldEdge[0] = nVertIndex[2];
			pMapping[nIndex].m_anOldEdge[1] = nVertIndex[0];
			pMapping[nIndex].m_aanNewEdge[0][0] = i * 3 + 2;
			pMapping[nIndex].m_aanNewEdge[0][1] = i * 3;

			++nNumMaps;
		} 
		else
		{
			// An entry is found for this edge.  Create
			// a quad and output it.
			// [PREFIX:beginmodify] 2006/2/25 realgaia PS#5803 reports CBsShadowVolumeMgr::GenerateShadowMesh() bounds violation (overflow) 'pMapping[nNumMaps-1]'
			BsAssert( nNumMaps > 0 && (DWORD)nNumMaps <= dwNumEdges );
			// [PREFIX:endmodify] 2006/2/25 realgaia PS#5803 reports CBsShadowVolumeMgr::GenerateShadowMesh() bounds violation (overflow) 'pMapping[nNumMaps-1]'


			pMapping[nIndex].m_aanNewEdge[1][0] = i * 3 + 2;
			pMapping[nIndex].m_aanNewEdge[1][1] = i * 3;

			// First triangle
			pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[0][1];
			pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[0][0];
			pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[1][0];

			// Second triangle
			pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[1][1];
			pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[1][0];
			pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[0][0];

			// pMapping[nIndex] is no longer needed. Copy the last map entry
			// over and decrement the map count.

			pMapping[nIndex] = pMapping[nNumMaps-1];
			memset( &pMapping[nNumMaps-1], 0xFF, sizeof( pMapping[nNumMaps-1] ));
			--nNumMaps;
		}
	}

	delete [] pdwPtRep;

	// Now the entries in the edge mapping table represent
	// non-shared edges.  What they mean is that the original
	// mesh has openings (holes), so we attempt to patch them.
	// First we need to recreate our mesh with a larger vertex
	// and index buffers so the patching geometry could fit.

	//DXUTTRACE( L"Faces to patch: %d\n", nNumMaps );

	// Create a mesh with large enough vertex and
	// index buffers.

	SHADOWVERTEX *pPatchVBData = new SHADOWVERTEX[ ( nPrimCount + nNumMaps ) * 3 ];
	WORD *pdwPatchIBData = new WORD[ (nNextIndex / 3 + nNumMaps * 7) * 3 ];


	//memset( pPatchVBData, 0, sizeof(SHADOWVERTEX) * ( nPrimCount + nNumMaps ) * 3 );
	//memset( pdwPatchIBData, 0, sizeof(WORD) * ( nNextIndex + 3 * nNumMaps * 7 ) );

	// Copy the data from one mesh to the other

	memcpy( pPatchVBData, pNewVBData, sizeof(SHADOWVERTEX) * nPrimCount * 3 );
	memcpy( pdwPatchIBData, pdwNewIBData, sizeof(WORD) * nNextIndex );

	// Replace pNewMesh with the updated one.  Then the code
	// can continue working with the pNewMesh pointer.

	delete [] pNewVBData;
	delete [] pdwNewIBData;

	pNewVBData = pPatchVBData;
	pdwNewIBData = pdwPatchIBData;

	// Now, we iterate through the edge mapping table and
	// for each shared edge, we generate a quad.
	// For each non-shared edge, we patch the opening
	// with new faces.

	// nNextVertex is the index of the next vertex.
	int nNextVertex = nPrimCount * 3;

	// [PREFIX:beginmodify] 2006/2/25 realgaia PS#5804 reports CBsShadowVolumeMgr::GenerateShadowMesh() bounds violation (overflow) 'pMapping[i]'
	BsAssert( nNumMaps >= 0 && (DWORD)nNumMaps <= dwNumEdges );
	// [PREFIX:endmodify] 2006/2/25 realgaia PS#5804 reports CBsShadowVolumeMgr::GenerateShadowMesh() bounds violation (overflow) 'pMapping[i]'


	for( i = 0; i < nNumMaps; ++i )
	{
		if( pMapping[i].m_anOldEdge[0] != -1 &&
			pMapping[i].m_anOldEdge[1] != -1 )
		{
			// If the 2nd new edge indexes is -1,
			// this edge is a non-shared one.
			// We patch the opening by creating new
			// faces.
			if( pMapping[i].m_aanNewEdge[1][0] == -1 ||  // must have only one new edge
				pMapping[i].m_aanNewEdge[1][1] == -1 )
			{
				// Find another non-shared edge that
				// shares a vertex with the current edge.
				for( int i2 = i + 1; i2 < nNumMaps; ++i2 )
				{
					if( pMapping[i2].m_anOldEdge[0] != -1 &&       // must have a valid old edge
						pMapping[i2].m_anOldEdge[1] != -1 &&
						( pMapping[i2].m_aanNewEdge[1][0] == -1 || // must have only one new edge
						pMapping[i2].m_aanNewEdge[1][1] == -1 ) )
					{
						int nVertShared = 0;
						if( pMapping[i2].m_anOldEdge[0] == pMapping[i].m_anOldEdge[1] )
							++nVertShared;
						if( pMapping[i2].m_anOldEdge[1] == pMapping[i].m_anOldEdge[0] )
							++nVertShared;

						if( 2 == nVertShared )
						{
							// These are the last two edges of this particular
							// opening. Mark this edge as shared so that a degenerate
							// quad can be created for it.

							pMapping[i2].m_aanNewEdge[1][0] = pMapping[i].m_aanNewEdge[0][0];
							pMapping[i2].m_aanNewEdge[1][1] = pMapping[i].m_aanNewEdge[0][1];
							break;
						}
						else if( 1 == nVertShared )
						{
							// nBefore and nAfter tell us which edge comes before the other.
							int nBefore, nAfter;
							if( pMapping[i2].m_anOldEdge[0] == pMapping[i].m_anOldEdge[1] )
							{
								nBefore = i;
								nAfter = i2;
							} 
							else
							{
								nBefore = i2;
								nAfter = i;
							}

							// Found such an edge. Now create a face along with two
							// degenerate quads from these two edges.

							pNewVBData[nNextVertex] = pNewVBData[pMapping[nAfter].m_aanNewEdge[0][1]];
							pNewVBData[nNextVertex+1] = pNewVBData[pMapping[nBefore].m_aanNewEdge[0][1]];
							pNewVBData[nNextVertex+2] = pNewVBData[pMapping[nBefore].m_aanNewEdge[0][0]];
							// Recompute the normal
							D3DXVECTOR3 v1 = pNewVBData[nNextVertex+1].Position - pNewVBData[nNextVertex].Position;
							D3DXVECTOR3 v2 = pNewVBData[nNextVertex+2].Position - pNewVBData[nNextVertex+1].Position;
							D3DXVec3Normalize( &v1, &v1 );
							D3DXVec3Normalize( &v2, &v2 );
							D3DXVec3Cross( &pNewVBData[nNextVertex].Normal, &v1, &v2 );
							pNewVBData[nNextVertex+1].Normal = pNewVBData[nNextVertex+2].Normal = pNewVBData[nNextVertex].Normal;

							pdwNewIBData[nNextIndex] = nNextVertex;
							pdwNewIBData[nNextIndex+1] = nNextVertex + 1;
							pdwNewIBData[nNextIndex+2] = nNextVertex + 2;

							// 1st quad

							pdwNewIBData[nNextIndex+3] = pMapping[nBefore].m_aanNewEdge[0][1];
							pdwNewIBData[nNextIndex+4] = pMapping[nBefore].m_aanNewEdge[0][0];
							pdwNewIBData[nNextIndex+5] = nNextVertex + 1;

							pdwNewIBData[nNextIndex+6] = nNextVertex + 2;
							pdwNewIBData[nNextIndex+7] = nNextVertex + 1;
							pdwNewIBData[nNextIndex+8] = pMapping[nBefore].m_aanNewEdge[0][0];

							// 2nd quad

							pdwNewIBData[nNextIndex+9] = pMapping[nAfter].m_aanNewEdge[0][1];
							pdwNewIBData[nNextIndex+10] = pMapping[nAfter].m_aanNewEdge[0][0];
							pdwNewIBData[nNextIndex+11] = nNextVertex;

							pdwNewIBData[nNextIndex+12] = nNextVertex + 1;
							pdwNewIBData[nNextIndex+13] = nNextVertex;
							pdwNewIBData[nNextIndex+14] = pMapping[nAfter].m_aanNewEdge[0][0];

							// Modify mapping entry i2 to reflect the third edge
							// of the newly added face.

							if( pMapping[i2].m_anOldEdge[0] == pMapping[i].m_anOldEdge[1] )
							{
								pMapping[i2].m_anOldEdge[0] = pMapping[i].m_anOldEdge[0];
							} else
							{
								pMapping[i2].m_anOldEdge[1] = pMapping[i].m_anOldEdge[1];
							}
							pMapping[i2].m_aanNewEdge[0][0] = nNextVertex + 2;
							pMapping[i2].m_aanNewEdge[0][1] = nNextVertex;

							// Update next vertex/index positions

							nNextVertex += 3;
							nNextIndex += 15;

							break;
						}
					}
				}
			} 
			else
			{
				// This is a shared edge.  Create the degenerate quad.

				// First triangle
				pdwNewIBData[nNextIndex++] = pMapping[i].m_aanNewEdge[0][1];
				pdwNewIBData[nNextIndex++] = pMapping[i].m_aanNewEdge[0][0];
				pdwNewIBData[nNextIndex++] = pMapping[i].m_aanNewEdge[1][0];

				// Second triangle
				pdwNewIBData[nNextIndex++] = pMapping[i].m_aanNewEdge[1][1];
				pdwNewIBData[nNextIndex++] = pMapping[i].m_aanNewEdge[1][0];
				pdwNewIBData[nNextIndex++] = pMapping[i].m_aanNewEdge[0][0];
			}
		}
	}

	delete[] pMapping;

	// At this time, the output mesh may have an index buffer
	// bigger than what is actually needed, so we create yet
	// another mesh with the exact IB size that we need and
	// output it.  This mesh also uses 16-bit index if
	// 32-bit is not necessary.

	int numFaces = nNextIndex / 3;
	int numVertices = ( nPrimCount + nNumMaps ) * 3;

	bool bNeed32Bit = numVertices > 65535;
	if( bNeed32Bit ) {
		delete [] pNewVBData;
		delete [] pdwNewIBData;
		return false;
	}

	LPDIRECT3DVERTEXBUFFER9 pVB;

#ifdef _XBOX
	g_BsKernel.CreateVertexBuffer( numVertices * sizeof(SHADOWVERTEX_STREAM), D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &pVB);
	SHADOWVERTEX_STREAM* pDstVertices;
	pVB->Lock( 0L, 0L, (VOID**)&pDstVertices, 0L );	
	D3DXVECTOR3 Normal;
	for( i = 0; i < numVertices; i++) {
		pDstVertices[i].Position = pNewVBData[i].Position;
		Normal = pNewVBData[i].Normal;
		pDstVertices[i].PackedNormal = MakePacked_11_11_10(int(Normal.x*1023.f), int(Normal.y*1023.f), int(Normal.z*511.f));
	}	
	pVB->Unlock();
#else
	g_BsKernel.CreateVertexBuffer( numVertices * sizeof(SHADOWVERTEX), D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &pVB);
	SHADOWVERTEX* pDstVertices;
	pVB->Lock( 0L, 0L, (VOID**)&pDstVertices, 0L );	
	memcpy(pDstVertices, pNewVBData, numVertices * sizeof(SHADOWVERTEX));
	pVB->Unlock();
#endif

	LPDIRECT3DINDEXBUFFER9 pIB;
	g_BsKernel.CreateIndexBuffer( numFaces*sizeof(WORD)*3, 0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &pIB );
	WORD* pFaceIndices;
	pIB->Lock( 0, 0, (VOID**)&pFaceIndices, 0 );	
	memcpy( pFaceIndices, pdwNewIBData, numFaces*sizeof(WORD)*3 );	
	pIB->Unlock();

	delete [] pNewVBData;
	delete [] pdwNewIBData;

	*ppVB = pVB;
	*ppIB = pIB;
	*pPrimitiveCount = numFaces;

	return true;  
}

//--------------------------------------------------------------------------------------
// Takes a mesh and generate a new mesh from it that contains the degenerate invisible
// quads for shadow volume extrusion.

bool CBsShadowVolumeMgr::GenerateShadowMeshForBlend( SHADOWVERTEX_BLEND *pMeshVertices, WORD *pMeshIndices, int nVertexCount, int nPrimCount, LPDIRECT3DVERTEXBUFFER9 *ppVB, LPDIRECT3DINDEXBUFFER9 *ppIB, int *pPrimitiveCount )
{
	int i;

	DWORD *pdwPtRep = new DWORD[ nVertexCount ];
	GeneratePointReps(  nVertexCount, pMeshVertices, pdwPtRep );

	// Maximum number of unique edges = Number of faces * 3
	DWORD dwNumEdges = nPrimCount * 3;
	CEdgeMapping *pMapping = new CEdgeMapping[ dwNumEdges ];

	int nNumMaps = 0;  // Number of entries that exist in pMapping

	// Create a new mesh
	SHADOWVERTEX_BLEND *pNewVBData = new SHADOWVERTEX_BLEND[ nPrimCount * 3 ];
	WORD *pdwNewIBData = new WORD[ (nPrimCount + dwNumEdges * 2) * 3 ];

	// nNextIndex is the array index in IB that the next vertex index value
	// will be store at.
	int nNextIndex = 0;

	//memset( pNewVBData, 0, (nPrimCount * 3) * sizeof(SHADOWVERTEX_BLEND) );
	//memset( pdwNewIBData, 0, sizeof(WORD) * (nPrimCount + dwNumEdges * 2) * 3 );

	// pNextOutVertex is the location to write the next
	// vertex to.
	SHADOWVERTEX_BLEND *pNextOutVertex = pNewVBData;

	// Iterate through the faces.  For each face, output new
	// vertices and face in the new mesh, and write its edges
	// to the mapping table.

	for( i = 0; i < nPrimCount; i++ )
	{
		// Copy the vertex data for all 3 vertices
		memcpy( pNextOutVertex, pMeshVertices + pMeshIndices[i * 3], sizeof(SHADOWVERTEX_BLEND) );
		memcpy( pNextOutVertex + 1, pMeshVertices + pMeshIndices[i * 3 + 1], sizeof(SHADOWVERTEX_BLEND));
		memcpy( pNextOutVertex + 2, pMeshVertices + pMeshIndices[i * 3 + 2], sizeof(SHADOWVERTEX_BLEND));

		// Write out the face
		pdwNewIBData[nNextIndex++] = i * 3;
		pdwNewIBData[nNextIndex++] = i * 3 + 1;
		pdwNewIBData[nNextIndex++] = i * 3 + 2;

		// Compute the face normal and assign it to
		// the normals of the vertices.
		D3DXVECTOR3 v1, v2;  // v1 and v2 are the edge vectors of the face
		D3DXVECTOR3 vNormal;
		v1 = *(D3DXVECTOR3*)(pNextOutVertex + 1) - *(D3DXVECTOR3*)pNextOutVertex;
		v2 = *(D3DXVECTOR3*)(pNextOutVertex + 2) - *(D3DXVECTOR3*)(pNextOutVertex + 1);
		D3DXVec3Cross( &vNormal, &v1, &v2 );
		D3DXVec3Normalize( &vNormal, &vNormal );

		pNextOutVertex->Normal = vNormal;
		(pNextOutVertex + 1)->Normal = vNormal;
		(pNextOutVertex + 2)->Normal = vNormal;

		pNextOutVertex += 3;

		// Add the face's edges to the edge mapping table

		// Edge 1
		int nIndex;
		DWORD nVertIndex[3] = { pdwPtRep[ pMeshIndices[i * 3] ],
			pdwPtRep[ pMeshIndices[i * 3 + 1] ],
			pdwPtRep[ pMeshIndices[i * 3 + 2] ] };
		nIndex = FindEdgeInMappingTable( nVertIndex[0], nVertIndex[1], pMapping, dwNumEdges );

		// If error, we are not able to proceed, so abort.
		if( -1 == nIndex )
		{
// [PREFIX:beginmodify] 2006/2/15 junyash PS#5176 reports CBsShadowVolumeMgr::GenerateShadowMeshForBlend() leaks memory.
			delete [] pdwPtRep;
			delete [] pNewVBData;
			delete [] pdwNewIBData;
			delete [] pMapping;
// [PREFIX:endmodify] junyash
			return false;
		}

		if( pMapping[nIndex].m_anOldEdge[0] == -1 && pMapping[nIndex].m_anOldEdge[1] == -1 )
		{
			// No entry for this edge yet.  Initialize one.
			pMapping[nIndex].m_anOldEdge[0] = nVertIndex[0];
			pMapping[nIndex].m_anOldEdge[1] = nVertIndex[1];
			pMapping[nIndex].m_aanNewEdge[0][0] = i * 3;
			pMapping[nIndex].m_aanNewEdge[0][1] = i * 3 + 1;

			++nNumMaps;
		} else
		{
			// An entry is found for this edge.  Create
			// a quad and output it.
			// [PREFIX:beginmodify] 2006/2/25 realgaia PS#5803 reports CBsShadowVolumeMgr::GenerateShadowMesh() bounds violation (overflow) 'pMapping[nNumMaps-1]'
			BsAssert( nNumMaps > 0 && (DWORD)nNumMaps <= dwNumEdges );
			// [PREFIX:endmodify] 2006/2/25 realgaia PS#5803 reports CBsShadowVolumeMgr::GenerateShadowMesh() bounds violation (overflow) 'pMapping[nNumMaps-1]'


			pMapping[nIndex].m_aanNewEdge[1][0] = i * 3;      // For clarity
			pMapping[nIndex].m_aanNewEdge[1][1] = i * 3 + 1;

			// First triangle
			pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[0][1];
			pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[0][0];
			pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[1][0];

			// Second triangle
			pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[1][1];
			pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[1][0];
			pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[0][0];

			// pMapping[nIndex] is no longer needed. Copy the last map entry
			// over and decrement the map count.

			pMapping[nIndex] = pMapping[nNumMaps-1];
			memset( &pMapping[nNumMaps-1], 0xFF, sizeof( pMapping[nNumMaps-1] ));
			--nNumMaps;
		}

		// Edge 2
		nIndex = FindEdgeInMappingTable( nVertIndex[1], nVertIndex[2], pMapping, dwNumEdges );

		// If error, we are not able to proceed, so abort.
		if( -1 == nIndex )
		{
// [PREFIX:beginmodify] 2006/2/15 junyash PS#5176 reports CBsShadowVolumeMgr::GenerateShadowMeshForBlend() leaks memory.
			delete [] pdwPtRep;
			delete [] pNewVBData;
			delete [] pdwNewIBData;
			delete [] pMapping;
// [PREFIX:endmodify] junyash
            return false;		
		}

		if( pMapping[nIndex].m_anOldEdge[0] == -1 && pMapping[nIndex].m_anOldEdge[1] == -1 )
		{
			pMapping[nIndex].m_anOldEdge[0] = nVertIndex[1];
			pMapping[nIndex].m_anOldEdge[1] = nVertIndex[2];
			pMapping[nIndex].m_aanNewEdge[0][0] = i * 3 + 1;
			pMapping[nIndex].m_aanNewEdge[0][1] = i * 3 + 2;

			++nNumMaps;
		} 
		else
		{
			// An entry is found for this edge.  Create
			// a quad and output it.
			// [PREFIX:beginmodify] 2006/2/25 realgaia PS#5803 reports CBsShadowVolumeMgr::GenerateShadowMesh() bounds violation (overflow) 'pMapping[nNumMaps-1]'
			BsAssert( nNumMaps > 0 && (DWORD)nNumMaps <= dwNumEdges );
			// [PREFIX:endmodify] 2006/2/25 realgaia PS#5803 reports CBsShadowVolumeMgr::GenerateShadowMesh() bounds violation (overflow) 'pMapping[nNumMaps-1]'


			pMapping[nIndex].m_aanNewEdge[1][0] = i * 3 + 1;
			pMapping[nIndex].m_aanNewEdge[1][1] = i * 3 + 2;

			// First triangle
			pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[0][1];
			pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[0][0];
			pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[1][0];

			// Second triangle
			pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[1][1];
			pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[1][0];
			pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[0][0];

			// pMapping[nIndex] is no longer needed. Copy the last map entry
			// over and decrement the map count.

			pMapping[nIndex] = pMapping[nNumMaps-1];
			memset( &pMapping[nNumMaps-1], 0xFF, sizeof( pMapping[nNumMaps-1] ) );
			--nNumMaps;
		}

		// Edge 3
		nIndex = FindEdgeInMappingTable( nVertIndex[2], nVertIndex[0], pMapping, dwNumEdges );

		// If error, we are not able to proceed, so abort.
		if( -1 == nIndex )
		{
// [PREFIX:beginmodify] 2006/2/15 junyash PS#5176 reports CBsShadowVolumeMgr::GenerateShadowMeshForBlend() leaks memory.
			delete [] pdwPtRep;
			delete [] pNewVBData;
			delete [] pdwNewIBData;
			delete [] pMapping;
// [PREFIX:endmodify] junyash
			return false;
		}

		if( pMapping[nIndex].m_anOldEdge[0] == -1 && pMapping[nIndex].m_anOldEdge[1] == -1 )
		{
			pMapping[nIndex].m_anOldEdge[0] = nVertIndex[2];
			pMapping[nIndex].m_anOldEdge[1] = nVertIndex[0];
			pMapping[nIndex].m_aanNewEdge[0][0] = i * 3 + 2;
			pMapping[nIndex].m_aanNewEdge[0][1] = i * 3;

			++nNumMaps;
		} 
		else
		{
			// An entry is found for this edge.  Create
			// a quad and output it.
			// [PREFIX:beginmodify] 2006/2/25 realgaia PS#5803 reports CBsShadowVolumeMgr::GenerateShadowMesh() bounds violation (overflow) 'pMapping[nNumMaps-1]'
			BsAssert( nNumMaps > 0 && (DWORD)nNumMaps <= dwNumEdges );
			// [PREFIX:endmodify] 2006/2/25 realgaia PS#5803 reports CBsShadowVolumeMgr::GenerateShadowMesh() bounds violation (overflow) 'pMapping[nNumMaps-1]'


			pMapping[nIndex].m_aanNewEdge[1][0] = i * 3 + 2;
			pMapping[nIndex].m_aanNewEdge[1][1] = i * 3;

			// First triangle
			pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[0][1];
			pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[0][0];
			pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[1][0];

			// Second triangle
			pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[1][1];
			pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[1][0];
			pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[0][0];

			// pMapping[nIndex] is no longer needed. Copy the last map entry
			// over and decrement the map count.

			pMapping[nIndex] = pMapping[nNumMaps-1];
			memset( &pMapping[nNumMaps-1], 0xFF, sizeof( pMapping[nNumMaps-1] ));
			--nNumMaps;
		}
	}

	delete [] pdwPtRep;

	// Now the entries in the edge mapping table represent
	// non-shared edges.  What they mean is that the original
	// mesh has openings (holes), so we attempt to patch them.
	// First we need to recreate our mesh with a larger vertex
	// and index buffers so the patching geometry could fit.

	//DXUTTRACE( L"Faces to patch: %d\n", nNumMaps );

	// Create a mesh with large enough vertex and
	// index buffers.

	SHADOWVERTEX_BLEND *pPatchVBData = new SHADOWVERTEX_BLEND[ ( nPrimCount + nNumMaps ) * 3 ];
	WORD *pdwPatchIBData = new WORD[ (nNextIndex / 3 + nNumMaps * 7) * 3 ];
	

	//memset( pPatchVBData, 0, sizeof(SHADOWVERTEX_BLEND) * ( nPrimCount + nNumMaps ) * 3 );
	//memset( pdwPatchIBData, 0, sizeof(WORD) * ( nNextIndex + 3 * nNumMaps * 7 ) );

	// Copy the data from one mesh to the other

	memcpy( pPatchVBData, pNewVBData, sizeof(SHADOWVERTEX_BLEND) * nPrimCount * 3 );
	memcpy( pdwPatchIBData, pdwNewIBData, sizeof(WORD) * nNextIndex );

	// Replace pNewMesh with the updated one.  Then the code
	// can continue working with the pNewMesh pointer.

	delete [] pNewVBData;
	delete [] pdwNewIBData;
	
	pNewVBData = pPatchVBData;
	pdwNewIBData = pdwPatchIBData;

	// Now, we iterate through the edge mapping table and
	// for each shared edge, we generate a quad.
	// For each non-shared edge, we patch the opening
	// with new faces.

	// nNextVertex is the index of the next vertex.
	int nNextVertex = nPrimCount * 3;

	// [PREFIX:beginmodify] 2006/2/25 realgaia PS#5804 reports CBsShadowVolumeMgr::GenerateShadowMesh() bounds violation (overflow) 'pMapping[i]'
	BsAssert( nNumMaps >= 0 && (DWORD)nNumMaps <= dwNumEdges );
	// [PREFIX:endmodify] 2006/2/25 realgaia PS#5804 reports CBsShadowVolumeMgr::GenerateShadowMesh() bounds violation (overflow) 'pMapping[i]'

	for( i = 0; i < nNumMaps; ++i )
	{
		if( pMapping[i].m_anOldEdge[0] != -1 &&
			pMapping[i].m_anOldEdge[1] != -1 )
		{
			// If the 2nd new edge indexes is -1,
			// this edge is a non-shared one.
			// We patch the opening by creating new
			// faces.
			if( pMapping[i].m_aanNewEdge[1][0] == -1 ||  // must have only one new edge
				pMapping[i].m_aanNewEdge[1][1] == -1 )
			{
				// Find another non-shared edge that
				// shares a vertex with the current edge.
				for( int i2 = i + 1; i2 < nNumMaps; ++i2 )
				{
					if( pMapping[i2].m_anOldEdge[0] != -1 &&       // must have a valid old edge
						pMapping[i2].m_anOldEdge[1] != -1 &&
						( pMapping[i2].m_aanNewEdge[1][0] == -1 || // must have only one new edge
						pMapping[i2].m_aanNewEdge[1][1] == -1 ) )
					{
						int nVertShared = 0;
						if( pMapping[i2].m_anOldEdge[0] == pMapping[i].m_anOldEdge[1] )
							++nVertShared;
						if( pMapping[i2].m_anOldEdge[1] == pMapping[i].m_anOldEdge[0] )
							++nVertShared;

						if( 2 == nVertShared )
						{
							// These are the last two edges of this particular
							// opening. Mark this edge as shared so that a degenerate
							// quad can be created for it.

							pMapping[i2].m_aanNewEdge[1][0] = pMapping[i].m_aanNewEdge[0][0];
							pMapping[i2].m_aanNewEdge[1][1] = pMapping[i].m_aanNewEdge[0][1];
							break;
						}
						else if( 1 == nVertShared )
							{
								// nBefore and nAfter tell us which edge comes before the other.
								int nBefore, nAfter;
								if( pMapping[i2].m_anOldEdge[0] == pMapping[i].m_anOldEdge[1] )
								{
									nBefore = i;
									nAfter = i2;
								} 
								else
								{
									nBefore = i2;
									nAfter = i;
								}

								// Found such an edge. Now create a face along with two
								// degenerate quads from these two edges.

								pNewVBData[nNextVertex] = pNewVBData[pMapping[nAfter].m_aanNewEdge[0][1]];
								pNewVBData[nNextVertex+1] = pNewVBData[pMapping[nBefore].m_aanNewEdge[0][1]];
								pNewVBData[nNextVertex+2] = pNewVBData[pMapping[nBefore].m_aanNewEdge[0][0]];
								// Recompute the normal
								D3DXVECTOR3 v1 = pNewVBData[nNextVertex+1].Position - pNewVBData[nNextVertex].Position;
								D3DXVECTOR3 v2 = pNewVBData[nNextVertex+2].Position - pNewVBData[nNextVertex+1].Position;
								D3DXVec3Normalize( &v1, &v1 );
								D3DXVec3Normalize( &v2, &v2 );
								D3DXVec3Cross( &pNewVBData[nNextVertex].Normal, &v1, &v2 );
								pNewVBData[nNextVertex+1].Normal = pNewVBData[nNextVertex+2].Normal = pNewVBData[nNextVertex].Normal;

								pdwNewIBData[nNextIndex] = nNextVertex;
								pdwNewIBData[nNextIndex+1] = nNextVertex + 1;
								pdwNewIBData[nNextIndex+2] = nNextVertex + 2;

								// 1st quad

								pdwNewIBData[nNextIndex+3] = pMapping[nBefore].m_aanNewEdge[0][1];
								pdwNewIBData[nNextIndex+4] = pMapping[nBefore].m_aanNewEdge[0][0];
								pdwNewIBData[nNextIndex+5] = nNextVertex + 1;

								pdwNewIBData[nNextIndex+6] = nNextVertex + 2;
								pdwNewIBData[nNextIndex+7] = nNextVertex + 1;
								pdwNewIBData[nNextIndex+8] = pMapping[nBefore].m_aanNewEdge[0][0];

								// 2nd quad

								pdwNewIBData[nNextIndex+9] = pMapping[nAfter].m_aanNewEdge[0][1];
								pdwNewIBData[nNextIndex+10] = pMapping[nAfter].m_aanNewEdge[0][0];
								pdwNewIBData[nNextIndex+11] = nNextVertex;

								pdwNewIBData[nNextIndex+12] = nNextVertex + 1;
								pdwNewIBData[nNextIndex+13] = nNextVertex;
								pdwNewIBData[nNextIndex+14] = pMapping[nAfter].m_aanNewEdge[0][0];

								// Modify mapping entry i2 to reflect the third edge
								// of the newly added face.

								if( pMapping[i2].m_anOldEdge[0] == pMapping[i].m_anOldEdge[1] )
								{
									pMapping[i2].m_anOldEdge[0] = pMapping[i].m_anOldEdge[0];
								} else
								{
									pMapping[i2].m_anOldEdge[1] = pMapping[i].m_anOldEdge[1];
								}
								pMapping[i2].m_aanNewEdge[0][0] = nNextVertex + 2;
								pMapping[i2].m_aanNewEdge[0][1] = nNextVertex;

								// Update next vertex/index positions

								nNextVertex += 3;
								nNextIndex += 15;

								break;
							}
					}
				}
			} 
			else
			{
				// This is a shared edge.  Create the degenerate quad.

				// First triangle
				pdwNewIBData[nNextIndex++] = pMapping[i].m_aanNewEdge[0][1];
				pdwNewIBData[nNextIndex++] = pMapping[i].m_aanNewEdge[0][0];
				pdwNewIBData[nNextIndex++] = pMapping[i].m_aanNewEdge[1][0];

				// Second triangle
				pdwNewIBData[nNextIndex++] = pMapping[i].m_aanNewEdge[1][1];
				pdwNewIBData[nNextIndex++] = pMapping[i].m_aanNewEdge[1][0];
				pdwNewIBData[nNextIndex++] = pMapping[i].m_aanNewEdge[0][0];
			}
		}
	}

	delete [] pMapping;

	// At this time, the output mesh may have an index buffer
	// bigger than what is actually needed, so we create yet
	// another mesh with the exact IB size that we need and
	// output it.  This mesh also uses 16-bit index if
	// 32-bit is not necessary.

	int numFaces = nNextIndex / 3;
	int numVertices = ( nPrimCount + nNumMaps ) * 3;

	bool bNeed32Bit = numVertices > 65535;
	if( bNeed32Bit ) {
		delete [] pNewVBData;
		delete [] pdwNewIBData;
		return false;
	}

	LPDIRECT3DVERTEXBUFFER9 pVB;

#ifdef _XBOX
	g_BsKernel.CreateVertexBuffer( numVertices * sizeof(SHADOWVERTEX_BLEND_STREAM), D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &pVB);
	SHADOWVERTEX_BLEND_STREAM* pDstVertices;
	pVB->Lock( 0L, 0L, (VOID**)&pDstVertices, 0L );	
	D3DXVECTOR3 Normal;
	for( i = 0; i < numVertices; i++) {
		pDstVertices[i].Position = pNewVBData[i].Position;
		Normal = pNewVBData[i].Normal;
		pDstVertices[i].PackedNormal = MakePacked_11_11_10(int(Normal.x*1023.f), int(Normal.y*1023.f), int(Normal.z*511.f));
		pDstVertices[i].nBlendIndices = pNewVBData[i].nBlendIndices;
	}
	pVB->Unlock();
#else
	g_BsKernel.CreateVertexBuffer( numVertices * sizeof(SHADOWVERTEX_BLEND), D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &pVB);
	SHADOWVERTEX_BLEND* pDstVertices;
	pVB->Lock( 0L, 0L, (VOID**)&pDstVertices, 0L );	
	memcpy(pDstVertices, pNewVBData, numVertices * sizeof(SHADOWVERTEX_BLEND));
	pVB->Unlock();
#endif

	LPDIRECT3DINDEXBUFFER9 pIB;
	g_BsKernel.CreateIndexBuffer( numFaces*sizeof(WORD)*3, 0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &pIB );
	WORD* pFaceIndices;
	pIB->Lock( 0, 0, (VOID**)&pFaceIndices, 0 );	
	memcpy( pFaceIndices, pdwNewIBData, numFaces*sizeof(WORD)*3 );	
	pIB->Unlock();

	delete [] pNewVBData;
	delete [] pdwNewIBData;

	*ppVB = pVB;
	*ppIB = pIB;
	*pPrimitiveCount = numFaces;

	return true;
}
