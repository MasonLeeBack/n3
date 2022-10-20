#include "stdafx.h"
#include "BsKernel.h"
#include "BsHFWorld.h"
#include "BsMaterial.h"
#include "BsDecal.h"
#include "BsUtil.h"
#include	"IntBox3Frustum.h"

static CBsDecalManager *s_pDecalManager = NULL; 

std::map< INT64, CBsDecalVertex* > CBsDecal::s_DecalPool[ DOUBLE_BUFFERING ];

#define CLEAR_TICK_INTERVAL 2048

void CBsDecal::Initialize( float fX, float fZ, int nTextureID, float fRadius, int nLifeTime, int nCreateTime, float fRotAngle, D3DCOLOR dwColor, int nMode)
{
	m_bEnable = true;
	m_nTextureID = nTextureID;	
	m_nLifeTime = nLifeTime;	
	m_nCreateTime = nCreateTime;	
	m_nMode = nMode;
	m_dwColor = dwColor;

	int nCellX = (int) ((fX - fRadius) / DISTANCE_PER_CELL);
	int nCellZ = (int) ((fZ - fRadius) / DISTANCE_PER_CELL);

	int nXSize = ((int)((fX + fRadius) / DISTANCE_PER_CELL)) - nCellX +1 ;
	int nZSize = ((int)((fZ + fRadius) / DISTANCE_PER_CELL)) - nCellZ +1 ;

	// calculate uv mat
	// (C - R ) -> 0
	// (C + R ) -> 1
	// f(t) = (t / 2R)  + ((R-C) / 2R );

	if( fRotAngle == 0.f) {
		m_UVTransX.x = 1.f / (fRadius*2);
		m_UVTransX.y = 0.f;
		m_UVTransX.z = (fRadius - fX) / (2*fRadius);

		m_UVTransY.x = 0.f;
		m_UVTransY.y = 1.f / (fRadius*2);
		m_UVTransY.z = (fRadius - fZ) / (2*fRadius);
	}
	else {
			
		D3DXMATRIX matTrans, matInvTrans, matRot;
		D3DXMatrixTranslation( &matInvTrans, -0.5f, -0.5f, 0.0f);
		D3DXMatrixTranslation( &matTrans, 0.5f,0.5f, 0.0f);

		D3DXMatrixRotationZ(&matRot, D3DX_PI * 2 * fRotAngle / 360.f);

		D3DXMatrixMultiply(&matRot, &matInvTrans, &matRot);
		D3DXMatrixMultiply(&matRot, &matRot , &matTrans);

		D3DXMATRIX matScale, matBias;

		D3DXMatrixScaling(&matScale, 1.f / (fRadius*2), 1.f / (fRadius*2), 0.f);
		D3DXMatrixTranslation(&matBias, (fRadius - fX) / (2*fRadius),  (fRadius - fZ) / (2*fRadius), 0.f);

		D3DXMATRIX matUV;
		D3DXMatrixMultiply(&matUV, &matScale, &matBias);
		D3DXMatrixMultiply(&matUV, &matUV, &matRot);

		m_UVTransX.x = matUV._11;
		m_UVTransX.y = matUV._21;
		m_UVTransX.z = matUV._41;

		m_UVTransY.x = matUV._12;
		m_UVTransY.y = matUV._22;
		m_UVTransY.z = matUV._42;
	}
	
	LARGE_INTEGER qwKey;
	qwKey.LowPart = MAKELONG( nCellX, nCellZ);
	qwKey.HighPart = MAKELONG( nXSize, nZSize);
			
	int nBufIndex = CBsObject::GetProcessBufferIndex();
	std::map< INT64, CBsDecalVertex* >::iterator it = s_DecalPool[ nBufIndex ].find( qwKey.QuadPart );

	if( it != s_DecalPool[ nBufIndex ].end() ) {
		m_pDecalVertex = it->second;
	}
	else {	
		m_pDecalVertex = new CBsDecalVertex;
		m_pDecalVertex->m_nVertexCount = (nXSize+1) * (nZSize+1);
		m_pDecalVertex->m_nIndexCount = nXSize * nZSize * 6;

		D3DXVECTOR3 Vmax = D3DXVECTOR3( -FLT_MAX, -FLT_MAX, -FLT_MAX);
		D3DXVECTOR3 Vmin = D3DXVECTOR3( FLT_MAX, FLT_MAX, FLT_MAX);

		int i, j;

		CBsHFWorld *pHFWorld = (CBsHFWorld*)g_BsKernel.GetWorld();
		short *pHeightBuffer = pHFWorld->GetHeightBufferPtr();
		int nWorldWidth = pHFWorld->GetWorldXSize()+1;

			
		D3DXVECTOR3 *pVertices = new D3DXVECTOR3[ m_pDecalVertex->m_nVertexCount ];
		unsigned short *pIndices = new unsigned short[ m_pDecalVertex->m_nIndexCount ];

		for( j = 0; j <= nZSize ; j++) {
			for( i = 0; i <= nXSize; i++) {
				float fVertX = (i + nCellX) * DISTANCE_PER_CELL;
				float fVertZ = (j + nCellZ) * DISTANCE_PER_CELL;
				float fVertY = ( float ) pHeightBuffer[   (i + nCellX) + (j + nCellZ) * nWorldWidth ] + 2.f;
				D3DXVECTOR3 Pos(fVertX, fVertY, fVertZ);
				pVertices[ i + j * (nXSize+1) ] = Pos;
				D3DXVec3Maximize(&Vmax, &Vmax, &Pos);
				D3DXVec3Minimize(&Vmin, &Vmin, &Pos);
			}
		}

		m_pDecalVertex->m_Obb.C = BSVECTOR( (Vmax.x + Vmin.x ) * 0.5f, (Vmax.y + Vmin.y ) * 0.5f, (Vmax.z + Vmin.z ) * 0.5f);

		BSVECTOR	tmpVec = BSVECTOR( ( Vmax.x - Vmin.x ) * 0.5f, ( Vmax.y - Vmin.y ) * 0.5f, ( Vmax.z - Vmin.z ) * 0.5f);
		m_pDecalVertex->m_Obb.E[0] = tmpVec.x;
		m_pDecalVertex->m_Obb.E[1] = tmpVec.y;
		m_pDecalVertex->m_Obb.E[2] = tmpVec.z;
		m_pDecalVertex->m_Obb.A[0] = BSVECTOR(1.f, 0.f, 0.f);
		m_pDecalVertex->m_Obb.A[1] = BSVECTOR(0.f, 1.f, 0.f);
		m_pDecalVertex->m_Obb.A[2] = BSVECTOR(0.f, 0.f, 1.f);
		m_pDecalVertex->m_Obb.compute_vertices();

		int nWidth = nXSize + 1;
		for( j = 0; j < nZSize; j++ )
		{
			for( i = 0; i < nXSize; i++ )
			{
// [PREFIX:beginmodify] 2006/2/16 junyash PS#5166 reports Local declaration of 'nBufIndex' hides declaration of the same name in an outer scope
				#if 0
				int nBufIndex = (i + j * nXSize) * 6;
				pIndices[ nBufIndex + 0 ] = j * nWidth + i;
				pIndices[ nBufIndex + 1 ] = ( j + 1 ) * nWidth + i;
				pIndices[ nBufIndex + 2 ] = ( j + 1 ) * nWidth + i + 1;

				pIndices[ nBufIndex + 3 ] = j * nWidth + i;
				pIndices[ nBufIndex + 4 ] = ( j + 1 ) * nWidth + i + 1;
				pIndices[ nBufIndex + 5 ] =  j * nWidth + i + 1;
				#else
				int nIndicesBufIndex = (i + j * nXSize) * 6;
				pIndices[ nIndicesBufIndex + 0 ] = j * nWidth + i;
				pIndices[ nIndicesBufIndex + 1 ] = ( j + 1 ) * nWidth + i;
				pIndices[ nIndicesBufIndex + 2 ] = ( j + 1 ) * nWidth + i + 1;

				pIndices[ nIndicesBufIndex + 3 ] = j * nWidth + i;
				pIndices[ nIndicesBufIndex + 4 ] = ( j + 1 ) * nWidth + i + 1;
				pIndices[ nIndicesBufIndex + 5 ] =  j * nWidth + i + 1;
				#endif
// [PREFIX:endmodify] junyash
			}
		}

		m_pDecalVertex->m_pVertices = pVertices;
		m_pDecalVertex->m_pIndices = pIndices;

		s_DecalPool[ nBufIndex ].insert( std::map< INT64, CBsDecalVertex* >::value_type(qwKey.QuadPart, m_pDecalVertex) );
	}
}

CBsDecalManager::CBsDecalManager()
{

	char fullName[_MAX_PATH];
	strcpy(fullName, g_BsKernel.GetShaderDirectory());
	strcat(fullName, "Decal.fx");
	m_nMaterialIndex = g_BsKernel.LoadMaterial(fullName, FALSE);
	assert( m_nMaterialIndex != -1  && "Decal Shader Compile Error!!" );

	CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialIndex);

#ifdef _XBOX
	m_hTexture = pMaterial->GetParameterByName("TextureSampler");
	m_hVP = pMaterial->GetParameterByName("ViewProjectionCustom");
#else
	m_hTexture = pMaterial->GetParameterByName("diffuseTexture");
	m_hVP = pMaterial->GetParameterByName("viewProjectionCustom");
#endif

	D3DVERTEXELEMENT9 decl[] =
	{
		{ 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },	
		{ 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },	
		{ 0, 20, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },	
		D3DDECL_END()
	};

	m_nDecalVertexDeclIndex = g_BsKernel.LoadVertexDeclaration(decl);

	m_nProcessTick = 0;
	m_nRenderTick = 0;
	s_pDecalManager = this;	

	g_BsKernel.chdir("data");
	g_BsKernel.chdir("fx");
	m_nPointLightDecalTextureIndex = g_BsKernel.LoadTexture("glow256.dds", TEXTURE_NORMAL);
	g_BsKernel.chdir("..");
	g_BsKernel.chdir("..");

}

CBsDecalManager::~CBsDecalManager() 
{ 	
	SAFE_RELEASE_MATERIAL(m_nMaterialIndex);
	SAFE_RELEASE_VD(m_nDecalVertexDeclIndex);
}

void CBsDecalManager::ClearAll()
{	
	for( int nBuf = 0; nBuf < 2; nBuf++ ) {

		int i, nSize;
		std::vector< CBsDecalGroup > &decalGroupList = m_DecalGroupList[ nBuf ];

		nSize = decalGroupList.size();
		for( i = 0; i < nSize; i++) {
			::free( decalGroupList[i].pVertices );
			::free( decalGroupList[i].pIndices );
		}
		decalGroupList.clear();

		std::map< INT64, CBsDecalVertex* >::iterator it = CBsDecal::s_DecalPool[ nBuf ].begin();
		while( it != CBsDecal::s_DecalPool[ nBuf ].end() ) {
			delete it->second;
			++it;
		}
		CBsDecal::s_DecalPool[ nBuf ].clear();
	}

	m_DecalList.clear();
}

void CBsDecalManager::ClearDecalGroup()
{
	int i, nSize;

	int nBufIndex = CBsObject::GetRenderBufferIndex();

	std::vector< CBsDecalGroup > &decalGroupList = m_DecalGroupList[ nBufIndex ];

	nSize = decalGroupList.size();
	for( i = 0; i < nSize; i++) {
		::free( decalGroupList[i].pVertices );
		::free( decalGroupList[i].pIndices );
	}
	decalGroupList.clear();
}

void CBsDecalManager::ClearDecalPool()
{
	int nBufIndex = CBsObject::GetProcessBufferIndex();

	std::map< INT64, CBsDecalVertex* >::iterator it = CBsDecal::s_DecalPool[ nBufIndex ].begin();
	while( it != CBsDecal::s_DecalPool[ nBufIndex ].end() ) {
		delete it->second;
		++it;
	}
	CBsDecal::s_DecalPool[ nBufIndex ].clear();
}

void CBsDecalManager::AddDecal(float fX, float fZ, int nTexture, float fRadius, int nLifeTime, float fRotAngle, D3DCOLOR dwColor, int nMode )
{
	CBsDecal decal;
	decal.Initialize(fX, fZ, nTexture, fRadius, nLifeTime, m_nProcessTick, fRotAngle, dwColor, nMode);
	m_DecalList.push_back( decal );
}

void CBsDecalManager::Update()
{
	++m_nProcessTick;
	if( g_BsKernel.GetCameraCount() == 0) {
        return;
	}
	int nCameraIndex = g_BsKernel.GetCameraHandle(0);
	const Frustum &ActiveFrustum = ((CBsCamera*)g_BsKernel.GetEngineObjectPtr(nCameraIndex))->GetFrustum();

	std::deque< CBsDecal >::iterator it = m_DecalList.begin();

	std::vector< CBsDecalGroup > &decalGroupList = m_DecalGroupList[ CBsObject::GetProcessBufferIndex() ];

	for( ; it != m_DecalList.end() ; ) {

		int nCreateTime = it->GetCreateTime();
		float fLifeRatio = 0.f;

		if( it->GetTotalLife() != 0) {
			fLifeRatio = (float)(m_nProcessTick - nCreateTime) / it->GetTotalLife();
		}

		if( fLifeRatio > 1.f ) {			
			it = m_DecalList.erase(it);
		}
		else {	

			Box3 B = it->GetObb();
			if ( TestIntersection(B, ActiveFrustum ) ) {	// Render 하기로 결정

				int nTextureID = it->GetTextureID();
				int nMode = it->GetBlendMode();

				int nSize;
				nSize = decalGroupList.size();

				if( it->GetIndexSize() > 30000 ) {
                    //_DEBUGBREAK;
					it = m_DecalList.erase( it );
					continue;
				}

				int nUseIndex = -1;
				for( int j = 0; j < nSize; j++) {
					int nNeedBufSize = ( it->GetIndexSize() + decalGroupList[j].nIndexCount );
					if( nMode == decalGroupList[ j ].nMode && nTextureID == decalGroupList[ j ].nTextureIndex && nNeedBufSize < 30000) {
						nUseIndex = j;
						break;
					}
				}

				if( nUseIndex == -1) {
					CBsDecalGroup decalGroup;
					decalGroup.nMode = nMode;
					decalGroup.nTextureIndex = nTextureID;
					decalGroup.nAllocVertexCount = DECAL_ALLOC_VERTEX_SIZE;
					decalGroup.pVertices = (DECAL_VERTEX*) ::malloc( sizeof(DECAL_VERTEX) * DECAL_ALLOC_VERTEX_SIZE );
					decalGroup.nAllocIndexCount = DECAL_ALLOC_INDEX_SIZE;
					decalGroup.pIndices = (unsigned short*) ::malloc( sizeof(unsigned short) * DECAL_ALLOC_INDEX_SIZE );					
					decalGroup.nVertexCount = 0;
					decalGroup.nIndexCount = 0;
					decalGroupList.push_back( decalGroup );
					nUseIndex = decalGroupList.size() - 1;
				}
				CBsDecalGroup &decal = decalGroupList[nUseIndex];

				float fAlpha = 1.f;
				if( it->GetTotalLife() != 0) {							
					float IncTime = 0.1f;
					float DecTime = 0.85f;

					if( fLifeRatio > DecTime) {
						fAlpha = 1.f - (fLifeRatio-DecTime) / (1.f-DecTime);				
					}
					else if ( fLifeRatio < IncTime) {
						fAlpha = (fLifeRatio / IncTime);				
					}
				}

				int nNeededVertexSize = decal.nVertexCount + it->GetVertexSize();
				int nNeededIndexSize = decal.nIndexCount + it->GetIndexSize();


				if( nNeededVertexSize > decal.nAllocVertexCount ) {
					decal.nAllocVertexCount = BsMax( decal.nAllocVertexCount * 2 , nNeededVertexSize );						
					decal.pVertices = (DECAL_VERTEX*)::realloc( decal.pVertices, decal.nAllocVertexCount *sizeof(DECAL_VERTEX));
				}

				if( nNeededIndexSize > decal.nAllocIndexCount ) {
					decal.nAllocIndexCount = BsMax( decal.nAllocIndexCount * 2, nNeededIndexSize );
					decal.pIndices = (unsigned short*)::realloc( decal.pIndices, decal.nAllocIndexCount *sizeof(unsigned short));					
				}

				BYTE alpha = (BYTE)(fAlpha * 255);

				int nVertexSize = it->GetVertexSize();
				int nStartVertexOffset = decal.nVertexCount;
				D3DCOLOR dwColor = it->m_dwColor;
				DECAL_VERTEX *pVertex = &decal.pVertices[decal.nVertexCount];
				D3DXVECTOR3 *pSrcVertex = it->GetVertexPtr();
				for( int j = 0; j < nVertexSize; j++) {										
					pVertex->color = (dwColor & 0x00ffffff ) | (alpha<<24);
					pVertex->pos = *pSrcVertex;
					pVertex->uv.x = pSrcVertex->x * it->m_UVTransX.x + pSrcVertex->z * it->m_UVTransX.y + it->m_UVTransX.z;
					pVertex->uv.y = pSrcVertex->x * it->m_UVTransY.x + pSrcVertex->z * it->m_UVTransY.y + it->m_UVTransY.z;
					pVertex++;
					pSrcVertex++;
				}
				decal.nVertexCount += nVertexSize;

				int nIndexSize = it->GetIndexSize();
				unsigned short *pSrcIndices = it->GetIndexPtr();
				unsigned short *pIndices = &decal.pIndices[decal.nIndexCount];
				for( int j = 0; j < nIndexSize; j++) {	
					*pIndices = *pSrcIndices + nStartVertexOffset;
					pIndices++;
					pSrcIndices++;
				}
				decal.nIndexCount += nIndexSize;
			}
			if( fLifeRatio == 0.f) {
				it = m_DecalList.erase( it );
			}
			else {
				++it;
			}
		}
	}

	if( m_nProcessTick % CLEAR_TICK_INTERVAL < 2 ) {	// double buffer 라서 2번 지워준다
		ClearDecalPool();
	}
}

void CBsDecalManager::Render(C3DDevice *pDevice) 
{
	++m_nRenderTick;
	std::vector< CBsDecalGroup > &decalGroupList = m_DecalGroupList[ CBsObject::GetRenderBufferIndex() ];

	if( decalGroupList.empty() ) return;
	
	CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr( m_nMaterialIndex );
	pMaterial->BeginMaterial(0, 0);
	pMaterial->BeginPass(0);

	g_BsKernel.SetVertexDeclaration(m_nDecalVertexDeclIndex);

	const D3DXMATRIX* matViewProj;
	matViewProj = g_BsKernel.GetParamViewProjectionMatrix();
	pMaterial->SetMatrix( m_hVP, matViewProj);

	int nBeforeTextureID = -1;
	static float fSlopeScaled = -0.2f;

	pDevice->SetRenderState( D3DRS_SLOPESCALEDEPTHBIAS , *((DWORD*)&fSlopeScaled));
	pDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
	pDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
	pDevice->SetSamplerState( 0, D3DSAMP_BORDERCOLOR, D3DCOLOR_ARGB(0,0,0,0));


	pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE);
	pDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
	pDevice->SetRenderState( D3DRS_ALPHAREF, 0x01);
	pDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE);

	//////////////////////////////////////////////////////////////////////////

	int i, nSize;

	nSize = decalGroupList.size();

	pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE);
	for( i = 0; i < nSize; i++) {
		CBsDecalGroup &decalGroup = decalGroupList[i];

		if( decalGroup.nVertexCount == 0 ||
			decalGroup.nIndexCount == 0 ) continue;

		if( decalGroup.nTextureIndex == POINTLIGHT_DECAL ) {
			pMaterial->SetTexture( m_hTexture, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr( m_nPointLightDecalTextureIndex ) );
		}
		else {
			pMaterial->SetTexture( m_hTexture, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr( decalGroup.nTextureIndex ) );
		}
		
		if( decalGroup.nMode == 1) { 
			pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE );
		}
		pMaterial->CommitChanges();
#ifdef _XBOX
		
		DECAL_VERTEX *pVertices;
		unsigned short*pIndices;

		pDevice->BeginIndexedVertices( D3DPT_TRIANGLELIST, 0, decalGroup.nVertexCount, decalGroup.nIndexCount, D3DFMT_INDEX16, sizeof(DECAL_VERTEX), (VOID **)&pIndices,  (VOID **)&pVertices);
		memcpy(pVertices, decalGroup.pVertices, sizeof(DECAL_VERTEX) * decalGroup.nVertexCount );
		memcpy(pIndices, decalGroup.pIndices, sizeof(unsigned short) * decalGroup.nIndexCount);
		pDevice->EndIndexedVertices();		
#else
		pDevice->SetFVF( D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_DIFFUSE );
		//pDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, decalGroup.nVertexCount - 2, decalGroup.pVertices, sizeof(DECAL_VERTEX));
		// not yet support
#endif
		decalGroup.nVertexCount = 0;
		decalGroup.nIndexCount = 0;
		if( decalGroup.nMode == 1) { 
			pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
		}
	}

	if( m_nRenderTick % CLEAR_TICK_INTERVAL < 2 ) {
        ClearDecalGroup();                        
	}


	pMaterial->EndPass() ;
	pMaterial->EndMaterial();

	pDevice->SetRenderState( D3DRS_SLOPESCALEDEPTHBIAS , 0);

	pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	pDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );
	pDevice->SetRenderState( D3DRS_ALPHAREF, pDevice->GetAlphaRefValue() );

	pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW);

	pDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
	pDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );	

	pDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE);
}
