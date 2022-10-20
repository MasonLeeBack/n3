#include "stdafx.h"
#include "BsHFWorld.h"
#include "BsKernel.h"
#include "BsShadowMgr.h"
#include "Box3.h"
#include "Frustum.h"
#include "IntBox3Frustum.h"
#include "BsMaterial.h"

CBsHFWorld::CBsHFWorld(int nXSizePerZone/*=40*/, int nZSizePerZone/*=40*/)
{
	int i;

	m_nZoneCheckOffSet=16;

	m_nWorldXSize=0;
	m_nWorldZSize=0;
	m_pHeight=NULL;

	for(i=0;i<MAX_VISIBLE_ZONE_COUNT;i++){
		m_pVB[i]=NULL;
		m_nZoneIndex[i]=-1;
		m_bUse[i]=false;
	}
	m_pIB=NULL;
	m_nVertexCountPerZone=0;
	m_nFaceCountPerZone=0;

	m_nXSizePerZone = nXSizePerZone;
	m_nZSizePerZone = nZSizePerZone;

	m_pMaxHeight=NULL;
	m_pMinHeight=NULL;

	m_nHFMaterialIndex = -1;

	m_nTerrainVertexDeclIndex = -1;

	for(i=0;i<6;i++){
		m_nTextureIndex[i]=-1;
	}
}

CBsHFWorld::~CBsHFWorld()
{
	Clear();
}

void CBsHFWorld::Clear()
{
	int i;

	for(i=0;i<MAX_VISIBLE_ZONE_COUNT;i++){
		if(m_pVB[i]){
			m_pVB[i]->Release();
			m_pVB[i]=NULL;
		}
	}

	if(m_pMaxHeight){
		delete [] m_pMaxHeight;
		m_pMaxHeight=NULL;
	}
	if(m_pMinHeight){
		delete [] m_pMinHeight;
		m_pMinHeight=NULL;
	}

	if(m_pIB){
		m_pIB->Release();
		m_pIB=NULL;
	}

	SAFE_RELEASE_MATERIAL(m_nHFMaterialIndex);
	SAFE_RELEASE_VD(m_nTerrainVertexDeclIndex);

	for(i=0;i<6;i++){
		SAFE_RELEASE_TEXTURE(m_nTextureIndex[i]);
	}
}

void CBsHFWorld::Create(int nSizeX, int nSizeZ, short int *pHeight, const char **ppFileList, const char* pszLightMapFileName, DWORD *pMulColor, DWORD *pAddColor)
{
	int i, j, k;
	int nZone;
	m_nWorldXSize = nSizeX;
	m_nWorldZSize = nSizeZ;
	m_pHeight = pHeight;

	m_pdwMul = pMulColor;
	m_pdwAdd = pAddColor;

	m_nZoneHoriCount=m_nWorldXSize/m_nXSizePerZone;
	if(m_nWorldXSize%m_nXSizePerZone){
		m_nZoneHoriCount++;
	}
	m_nZoneVertCount=m_nWorldZSize/m_nZSizePerZone;
	if(m_nWorldZSize%m_nZSizePerZone){
		m_nZoneVertCount++;
	}

	m_pMaxHeight=new float[m_nZoneVertCount*m_nZoneHoriCount];
	m_pMinHeight=new float[m_nZoneVertCount*m_nZoneHoriCount];

	for(i=0;i<m_nZoneVertCount*m_nZoneHoriCount;i++){
		m_pMaxHeight[i]=-FLT_MAX;
		m_pMinHeight[i]=FLT_MAX;
	}
	for(i=0;i<(m_nWorldXSize+1)*(m_nWorldZSize);i++){
		nZone=i/(m_nWorldXSize+1)/m_nZSizePerZone*m_nZoneHoriCount+i%(m_nWorldXSize+1)/m_nXSizePerZone;
		if(nZone>=m_nZoneVertCount*m_nZoneHoriCount){
			continue;
		}
		if(m_pHeight[i]>m_pMaxHeight[nZone]){
			m_pMaxHeight[nZone]=m_pHeight[i];
		}
		if(m_pHeight[i]<m_pMinHeight[nZone]){
			m_pMinHeight[nZone]=m_pHeight[i];
		}
	}

	m_nVertexCountPerZone=(m_nXSizePerZone+1) * (m_nZSizePerZone+1);
	m_nFaceCountPerZone=m_nXSizePerZone*m_nZSizePerZone*2;


	// Allocation Vertex Buffer!!
	HFVERTEX *pVertexBuf;
	int nBufIndex;
	int nXVertexCountPerZone = m_nXSizePerZone+1;
	int nZVertexCountPerZone = m_nZSizePerZone+1;
	for(i=0;i<MAX_VISIBLE_ZONE_COUNT;i++){
		g_BsKernel.CreateVertexBuffer(m_nVertexCountPerZone * sizeof(HFVERTEX), D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &(m_pVB[i]));
		m_pVB[i]->Lock(0, 0, (void **)&pVertexBuf, 0);
		for(j=0;j<nZVertexCountPerZone;j++){
			for(k=0;k<nXVertexCountPerZone;k++){
				nBufIndex=(j*nXVertexCountPerZone+k);
				pVertexBuf[nBufIndex].vecPos=D3DXVECTOR3(k*DISTANCE_PER_CELL, 0.f, j*DISTANCE_PER_CELL);
				pVertexBuf[nBufIndex].dwMulColor = 0xffffffff;
				pVertexBuf[nBufIndex].dwAddColor = 0xffffffff;
			}
		}
		m_pVB[i]->Unlock();
	}

	// Make Triangle List Index Buffer!!
	WORD *pFaceBuf;
	g_BsKernel.CreateIndexBuffer(m_nFaceCountPerZone*sizeof(WORD)*3, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pIB);
	m_pIB->Lock(0, 0, (void **)&pFaceBuf, 0);
	int nIndex = 0;
	for( i=0; i < m_nZSizePerZone; ++i ) {
		for(int j=0; j < m_nXSizePerZone; ++j) {
			nIndex = i*nXVertexCountPerZone + j;
			*pFaceBuf = nIndex;
			pFaceBuf++;
			*pFaceBuf=nIndex+1+nXVertexCountPerZone;
			pFaceBuf++;
			*pFaceBuf=nIndex+1;
			pFaceBuf++;

			*pFaceBuf=nIndex;
			pFaceBuf++;
			*pFaceBuf=nIndex+nXVertexCountPerZone;
			pFaceBuf++;
			*pFaceBuf=nIndex+1+nXVertexCountPerZone;
			pFaceBuf++;
		}
	}
	m_pIB->Unlock();

	{	// Make Triangle Strip Index Buffer !!!!
/*		unsigned short yVerts = nXVertexCountPerZone;
		unsigned short xVerts = nZVertexCountPerZone;
		int total_strips = 
			yVerts-1;
		int total_indexes_per_strip = 
			xVerts<<1;

		// the total number of indices is equal
		// to the number of strips times the
		// indices used per strip plus one
		// degenerate triangle between each strip
		int total_indexes = 
			(total_indexes_per_strip * total_strips) 
			+ (total_strips<<1) - 2;

		m_nIndexCountPerZone = total_indexes;

		WORD *pFaceBuf;
		g_BsKernel.CreateIndexBuffer(total_indexes*sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pIB);
		m_pIB->Lock(0, 0, (void **)&pFaceBuf, 0);

		unsigned short* index = pFaceBuf;
		unsigned short start_vert = 0;

		for (int j=0;j<total_strips;++j)
		{
			int k=0;
			unsigned short vert=start_vert;

			// create a strip for this row
			for (k=0;k<xVerts;++k)
			{
				*(index++) = vert;
				*(index++) = vert + xVerts;
				vert += 1;
			}
			start_vert += xVerts;

			if (j+1<total_strips)
			{
				// add a degenerate to attach to 
				// the next row
				*(index++) = (vert-1)+xVerts;
				*(index++) = start_vert;
			}
		}
		m_pIB->Unlock();
*/
	}

	// Material Setting!!
	char fullName[_MAX_PATH];
	strcpy(fullName, g_BsKernel.GetShaderDirectory());
#ifdef _XBOX		
	strcat(fullName, "Terrain.fx");
#else
	strcat(fullName, "Terrain_PC.fx");
#endif

	m_nHFMaterialIndex = g_BsKernel.LoadMaterial(fullName, FALSE);

	CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(m_nHFMaterialIndex);
#ifdef _XBOX
	m_hLightDir = pMaterial->GetParameterByName("Direction");
	m_hLightDiffuse = pMaterial->GetParameterByName( "LightDiffuse");
	m_hLightSpecular = pMaterial->GetParameterByName("LightSpecular");
	m_hLightAmbient = pMaterial->GetParameterByName("LightAmbient");
	m_hFogFactor = pMaterial->GetParameterByName("FOGFACTOR");
	m_hFogColor = pMaterial->GetParameterByName("FOGCOLOR");

	m_hWVP = pMaterial->GetParameterByName("WorldViewProjection");
	m_hWV = pMaterial->GetParameterByName("WorldView");
	m_hWVIT = pMaterial->GetParameterByName("WorldViewInverseTranspose");
	m_hShadowMatrix = pMaterial->GetParameterByName("SHADOW_ALL");

	m_hLayer0Texture = pMaterial->GetParameterByName("Layer0Sampler");
	m_hLayer1Texture = pMaterial->GetParameterByName("Layer1Sampler");
	m_hLayer2Texture = pMaterial->GetParameterByName("Layer2Sampler");
	m_hLayer3Texture = pMaterial->GetParameterByName("Layer3Sampler");
	m_hLayer4Texture = pMaterial->GetParameterByName("Layer4Sampler");
	m_hBlendTexture = pMaterial->GetParameterByName("BlendSampler");
	m_hShadowTexture = pMaterial->GetParameterByName("ShadowMapSampler");
#else
	m_hLightDir = pMaterial->GetParameterByName( "lightDir");
	m_hLightDiffuse = pMaterial->GetParameterByName("lightDiffuse");
	m_hLightSpecular = pMaterial->GetParameterByName("lightSpecular");
	m_hLightAmbient = pMaterial->GetParameterByName( "lightAmbient");
	m_hFogFactor = pMaterial->GetParameterByName("fogFactor");
	m_hFogColor = pMaterial->GetParameterByName("fogColor");

	m_hWVP = pMaterial->GetParameterByName("worldViewProj");
	m_hWV = pMaterial->GetParameterByName("WorldView");
	m_hWVIT = pMaterial->GetParameterByName("worldviewInverseTranspose");
	m_hShadowMatrix = pMaterial->GetParameterByName("shadowAll");

	m_hLayer0Texture = pMaterial->GetParameterByName("Layer0texture");
	m_hLayer1Texture = pMaterial->GetParameterByName("Layer1texture");
	m_hLayer2Texture = pMaterial->GetParameterByName("Layer2texture");
	m_hLayer3Texture = pMaterial->GetParameterByName("Layer3texture");
	m_hLayer4Texture = pMaterial->GetParameterByName("Layer4texture");
	m_hBlendTexture = pMaterial->GetParameterByName("BlendTexture");
	m_hShadowTexture = pMaterial->GetParameterByName("ShadowMap");
#endif

	D3DVERTEXELEMENT9 decl[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0, 24, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
		{ 0, 28, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 1 },
		{ 0, 32, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },

		D3DDECL_END()
	};
	m_nTerrainVertexDeclIndex = g_BsKernel.LoadVertexDeclaration(decl);

	for(i=0;i<6;i++){
		m_nTextureIndex[i] = g_BsKernel.LoadTexture(ppFileList[i]);
	}
}

void CBsHFWorld::Reload()
{
	int i, j, k;

	for(i=0;i<MAX_VISIBLE_ZONE_COUNT;i++){
		if(m_pVB[i]){
			m_pVB[i]->Release();
			m_pVB[i]=NULL;
		}
	}
	// Allocation Vertex Buffer!!
	HFVERTEX *pVertexBuf;
	int nBufIndex;
	int nXVertexCountPerZone = m_nXSizePerZone+1;
	int nZVertexCountPerZone = m_nZSizePerZone+1;
	for(i=0;i<MAX_VISIBLE_ZONE_COUNT;i++){
		g_BsKernel.CreateVertexBuffer(m_nVertexCountPerZone * sizeof(HFVERTEX), D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &(m_pVB[i]));
		m_pVB[i]->Lock(0, 0, (void **)&pVertexBuf, 0);
		for(j=0;j<nZVertexCountPerZone;j++){
			for(k=0;k<nXVertexCountPerZone;k++){
				nBufIndex=(j*nXVertexCountPerZone+k);
				pVertexBuf[nBufIndex].vecPos=D3DXVECTOR3(k*DISTANCE_PER_CELL, 0.f, j*DISTANCE_PER_CELL);
				pVertexBuf[nBufIndex].dwMulColor = 0xffffffff;
				pVertexBuf[nBufIndex].dwAddColor = 0xffffffff;
			}
		}
		m_pVB[i]->Unlock();
	}

	if(m_pIB){
		m_pIB->Release();
		m_pIB=NULL;
	}
	WORD *pFaceBuf;
	g_BsKernel.CreateIndexBuffer(m_nFaceCountPerZone*sizeof(WORD)*3, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pIB);
	m_pIB->Lock(0, 0, (void **)&pFaceBuf, 0);
	int nIndex = 0;
	for( i=0; i < m_nZSizePerZone; ++i ) {
		for(int j=0; j < m_nXSizePerZone; ++j) {
			nIndex = i*nXVertexCountPerZone + j;
			*pFaceBuf = nIndex;
			pFaceBuf++;
			*pFaceBuf=nIndex+1+nXVertexCountPerZone;
			pFaceBuf++;
			*pFaceBuf=nIndex+1;
			pFaceBuf++;

			*pFaceBuf=nIndex;
			pFaceBuf++;
			*pFaceBuf=nIndex+nXVertexCountPerZone;
			pFaceBuf++;
			*pFaceBuf=nIndex+1+nXVertexCountPerZone;
			pFaceBuf++;
		}
	}
	m_pIB->Unlock();

	for(i=0;i<MAX_VISIBLE_ZONE_COUNT;i++){
		m_nZoneIndex[i]=-1;
		m_bUse[i]=false;
	}

	SAFE_RELEASE_MATERIAL(m_nHFMaterialIndex);

	// Material Setting!!
	char fullName[_MAX_PATH];
	strcpy(fullName, g_BsKernel.GetShaderDirectory());
#ifdef _XBOX		
	strcat(fullName, "Terrain.fx");
#else
	strcat(fullName, "Terrain_PC.fx");
#endif

	m_nHFMaterialIndex = g_BsKernel.LoadMaterial(fullName, FALSE);

	CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(m_nHFMaterialIndex);
#ifdef _XBOX
	m_hLightDir = pMaterial->GetParameterByName("Direction");
	m_hLightDiffuse = pMaterial->GetParameterByName( "LightDiffuse");
	m_hLightSpecular = pMaterial->GetParameterByName("LightSpecular");
	m_hLightAmbient = pMaterial->GetParameterByName("LightAmbient");
	m_hFogFactor = pMaterial->GetParameterByName("FOGFACTOR");
	m_hFogColor = pMaterial->GetParameterByName("FOGCOLOR");

	m_hWVP = pMaterial->GetParameterByName("WorldViewProjection");
	m_hWV = pMaterial->GetParameterByName("WorldView");
	m_hWVIT = pMaterial->GetParameterByName("WorldViewInverseTranspose");
	m_hShadowMatrix = pMaterial->GetParameterByName("SHADOW_ALL");

	m_hLayer0Texture = pMaterial->GetParameterByName("Layer0Sampler");
	m_hLayer1Texture = pMaterial->GetParameterByName("Layer1Sampler");
	m_hLayer2Texture = pMaterial->GetParameterByName("Layer2Sampler");
	m_hLayer3Texture = pMaterial->GetParameterByName("Layer3Sampler");
	m_hLayer4Texture = pMaterial->GetParameterByName("Layer4Sampler");
	m_hBlendTexture = pMaterial->GetParameterByName("BlendSampler");
	m_hShadowTexture = pMaterial->GetParameterByName("ShadowMapSampler");
#else
	m_hLightDir = pMaterial->GetParameterByName( "lightDir");
	m_hLightDiffuse = pMaterial->GetParameterByName("lightDiffuse");
	m_hLightSpecular = pMaterial->GetParameterByName("lightSpecular");
	m_hLightAmbient = pMaterial->GetParameterByName( "lightAmbient");
	m_hFogFactor = pMaterial->GetParameterByName("fogFactor");
	m_hFogColor = pMaterial->GetParameterByName("fogColor");

	m_hWVP = pMaterial->GetParameterByName("worldViewProj");
	m_hWV = pMaterial->GetParameterByName("WorldView");
	m_hWVIT = pMaterial->GetParameterByName("worldviewInverseTranspose");
	m_hShadowMatrix = pMaterial->GetParameterByName("shadowAll");

	m_hLayer0Texture = pMaterial->GetParameterByName("Layer0texture");
	m_hLayer1Texture = pMaterial->GetParameterByName("Layer1texture");
	m_hLayer2Texture = pMaterial->GetParameterByName("Layer2texture");
	m_hLayer3Texture = pMaterial->GetParameterByName("Layer3texture");
	m_hLayer4Texture = pMaterial->GetParameterByName("Layer4texture");
	m_hBlendTexture = pMaterial->GetParameterByName("BlendTexture");
	m_hShadowTexture = pMaterial->GetParameterByName("ShadowMap");
#endif
	SAFE_RELEASE_VD(m_nTerrainVertexDeclIndex);

	D3DVERTEXELEMENT9 decl[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0, 24, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
		{ 0, 28, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 1 },
		{ 0, 32, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },

		D3DDECL_END()
	};
	m_nTerrainVertexDeclIndex = g_BsKernel.LoadVertexDeclaration(decl);
}

void CBsHFWorld::Process()
{
}

void CBsHFWorld::Render(C3DDevice *pDevice)
{
	// Shadow는 추후 작업!! by jeremy
	CheckVisibleZone();

	float fStartX, fStartZ;
	D3DXMATRIX matWorld;
	D3DXMATRIX matWVP;
	D3DXMATRIX matWV;

	CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(m_nHFMaterialIndex);
	pMaterial->BeginMaterial(0, 0);
	pMaterial->BeginPass(0);

	D3DXVECTOR4 vecViewLight;
	D3DXVec4Transform(&vecViewLight, CBsKernel::GetInstance().GetLightDirection(), g_BsKernel.GetParamViewMatrix());

	pMaterial->SetVector(m_hLightDir, &vecViewLight);
	pMaterial->SetVector(m_hLightDiffuse, CBsKernel::GetInstance().GetLightDiffuse());
	pMaterial->SetVector(m_hLightSpecular, CBsKernel::GetInstance().GetLightSpecular());
	pMaterial->SetVector(m_hLightAmbient, CBsKernel::GetInstance().GetLightAmbient());

	pMaterial->SetVector(m_hFogFactor, CBsKernel::GetInstance().GetParamFogFactor());
	pMaterial->SetVector(m_hFogColor, CBsKernel::GetInstance().GetParamFogColor());

	pMaterial->SetTexture(m_hLayer0Texture, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(m_nTextureIndex[0]));
	pMaterial->SetTexture(m_hLayer1Texture, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(m_nTextureIndex[1]));
	pMaterial->SetTexture(m_hLayer2Texture, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(m_nTextureIndex[2]));
	pMaterial->SetTexture(m_hLayer3Texture, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(m_nTextureIndex[3]));
	pMaterial->SetTexture(m_hLayer4Texture, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(m_nTextureIndex[4]));
	pMaterial->SetTexture(m_hBlendTexture, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(m_nTextureIndex[5]));

	CBsShadowMgr* pShadowMgr = g_BsKernel.GetShadowMgr();
	pMaterial->SetTexture(m_hShadowTexture, pShadowMgr->GetShadowMap());

	float fOffsetX = 0.5f + (0.5f / (float)(pShadowMgr->GetShadowMapXSize()));
	float fOffsetY = 0.5f + (0.5f / (float)(pShadowMgr->GetShadowMapYSize()));
	unsigned int range = 1;
	float fBias    = 0.0f;
	D3DXMATRIX texScaleBiasMat( 0.5f,     0.0f,     0.0f,         0.0f,
								0.0f,    -0.5f,     0.0f,         0.0f,
								0.0f,     0.0f,     (float)range, 0.0f,
								fOffsetX, fOffsetY, 0.0f,         1.0f );

	int nRenderZoneCount = 0;

	for(int i=0;i<MAX_VISIBLE_ZONE_COUNT;i++){
		if((m_bUse[i])&&(m_pVB[i])){
			fStartX=m_nZoneIndex[i]%m_nZoneHoriCount*m_nXSizePerZone*DISTANCE_PER_CELL;
			fStartZ=(m_nZoneIndex[i]/m_nZoneHoriCount*m_nZSizePerZone)*DISTANCE_PER_CELL;

			D3DXMatrixTranslation(&matWorld, fStartX, 0.0f, fStartZ);

			pMaterial->SetMatrix(m_hWVP, D3DXMatrixMultiply(&matWVP, &matWorld, g_BsKernel.GetParamViewProjectionMatrix()));
			pMaterial->SetMatrix(m_hWV, D3DXMatrixMultiply(&matWVP, &matWorld, g_BsKernel.GetParamViewMatrix()));

			D3DXMATRIX matWorldViewIT;
			D3DXMatrixMultiply(&matWorldViewIT, &matWorld, CBsKernel::GetInstance().GetParamViewMatrix());
			D3DXMatrixInverse(&matWorldViewIT, NULL, &matWorldViewIT);
			D3DXMatrixTranspose(&matWorldViewIT, &matWorldViewIT);

			pMaterial->SetMatrix(m_hWVIT, &matWorldViewIT);

			// shadow !!
			D3DXMATRIX textureMatrix;
			D3DXMatrixMultiply(&textureMatrix, pShadowMgr->GetLightViewProject(), &texScaleBiasMat);
			D3DXMatrixMultiply(&textureMatrix, &matWorld,&textureMatrix);
			pMaterial->SetMatrix(m_hShadowMatrix, &textureMatrix );

			pMaterial->CommitChanges();

			pDevice->SetStreamSource( 0 , m_pVB[i] , sizeof(HFVERTEX));
			pDevice->SetIndices( m_pIB ); // , 0 );
			g_BsKernel.SetVertexDeclaration(m_nTerrainVertexDeclIndex);

			pDevice->DrawIndexedMeshVB( D3DPT_TRIANGLELIST, m_nVertexCountPerZone ,m_nFaceCountPerZone, 0, 0 );		// For Triangle List

			m_nUseCount[i]++;
			nRenderZoneCount++;
		}
		else if(m_nUseCount[i]>0){
			m_nUseCount[i]--;
		}
	}	// end for()

	char szBuf[40];
	sprintf(szBuf, "Render Zone : %d", nRenderZoneCount);
	g_BsKernel.PrintString(100, 400, szBuf);

	pMaterial->EndPass();
	pMaterial->EndMaterial();
}

D3DXVECTOR3 CBsHFWorld::GetNormalVector(int nCellX, int nCellZ)
{
	int nHeightIndex;
	float fDX, fDZ;
	D3DXVECTOR3 Return;

	nHeightIndex=(m_nWorldXSize+1)*nCellZ+nCellX;
	if(nCellX==0){
		fDX=(m_pHeight[nHeightIndex]-m_pHeight[nHeightIndex+1])/DISTANCE_PER_CELL;
	}
	else if(nCellX==m_nWorldXSize){
		fDX=(m_pHeight[nHeightIndex-1]-m_pHeight[nHeightIndex])/DISTANCE_PER_CELL;
	}
	else{
		fDX=(m_pHeight[nHeightIndex-1]-m_pHeight[nHeightIndex+1])/(DISTANCE_PER_CELL*2);
	}
	if(nCellZ==0){
		fDZ=(m_pHeight[nHeightIndex]-m_pHeight[nHeightIndex+m_nWorldXSize+1])/DISTANCE_PER_CELL;
	}
	else if(nCellZ==m_nWorldZSize){
		fDZ=(m_pHeight[nHeightIndex-m_nWorldXSize-1]-m_pHeight[nHeightIndex])/DISTANCE_PER_CELL;
	}
	else{
		fDZ=(m_pHeight[nHeightIndex-m_nWorldXSize-1]-m_pHeight[nHeightIndex+m_nWorldXSize+1])/(DISTANCE_PER_CELL*2);
	}

	Return.x=fDX;
	Return.y=2.0f;
	Return.z=fDZ;
	D3DXVec3Normalize(&Return, &Return);

	return Return;
}

float CBsHFWorld::GetLandHeight(float fX, float fZ, D3DXVECTOR3* N/*=NULL*/)
{
	int nCellX, nCellZ;
	float fModX, fModZ;
	float fCalcHeight1, fCalcHeight2, fRet;
	float fHeight[4];

	nCellX=(int)(fX/DISTANCE_PER_CELL);
	nCellZ=(int)(fZ/DISTANCE_PER_CELL);
	fModX=fX/DISTANCE_PER_CELL-nCellX;
	fModZ=fZ/DISTANCE_PER_CELL-nCellZ;

	fHeight[0]=m_pHeight[nCellZ*(m_nWorldXSize+1)+nCellX];
	fHeight[1]=m_pHeight[nCellZ*(m_nWorldXSize+1)+nCellX+1];
	fHeight[2]=m_pHeight[(nCellZ+1)*(m_nWorldXSize+1)+nCellX];
	fHeight[3]=m_pHeight[(nCellZ+1)*(m_nWorldXSize+1)+nCellX+1];

	if(fModX>fModZ){
		fCalcHeight1=fHeight[0]*(1.f-fModX)+fHeight[3]*fModX;
		fCalcHeight2=fHeight[0]*(1.f-fModX)+fHeight[1]*fModX;
		fRet=fCalcHeight1*(fModZ/fModX)+fCalcHeight2*(1-fModZ/fModX);
	}
	else if(fModX<fModZ){
		fCalcHeight1=fHeight[0]*(1-fModZ)+fHeight[2]*fModZ;
		fCalcHeight2=fHeight[0]*(1-fModZ)+fHeight[3]*fModZ;
		fRet=fCalcHeight1*(1-fModX/fModZ)+fCalcHeight2*(fModX/fModZ);
	}
	else{
		fRet=fHeight[0]*(1-fModZ)+fHeight[3]*fModZ;
	}

	if( N ){
		*N = GetNormalVector(nCellX, nCellZ);
	}

	return fRet;
}

float CBsHFWorld::GetHeight(float fX, float fZ)
{
	int nCellX, nCellZ;
	float fModX, fModZ;
	float fCalcHeight1, fCalcHeight2, fRet;
	float fHeight[4];

	nCellX=(int)(fX/DISTANCE_PER_CELL);
	nCellZ=(int)(fZ/DISTANCE_PER_CELL);
	fModX=fX/DISTANCE_PER_CELL-nCellX;
	fModZ=fZ/DISTANCE_PER_CELL-nCellZ;

	fHeight[0]=m_pHeight[nCellZ*(m_nWorldXSize+1)+nCellX];
	fHeight[1]=m_pHeight[nCellZ*(m_nWorldXSize+1)+nCellX+1];
	fHeight[2]=m_pHeight[(nCellZ+1)*(m_nWorldXSize+1)+nCellX];
	fHeight[3]=m_pHeight[(nCellZ+1)*(m_nWorldXSize+1)+nCellX+1];

	if(fModX>fModZ){
		fCalcHeight1=fHeight[0]*(1.f-fModX)+fHeight[3]*fModX;
		fCalcHeight2=fHeight[0]*(1.f-fModX)+fHeight[1]*fModX;
		fRet=fCalcHeight1*(fModZ/fModX)+fCalcHeight2*(1-fModZ/fModX);
	}
	else if(fModX<fModZ){
		fCalcHeight1=fHeight[0]*(1-fModZ)+fHeight[2]*fModZ;
		fCalcHeight2=fHeight[0]*(1-fModZ)+fHeight[3]*fModZ;
		fRet=fCalcHeight1*(1-fModX/fModZ)+fCalcHeight2*(fModX/fModZ);
	}
	else{
		fRet=fHeight[0]*(1-fModZ)+fHeight[3]*fModZ;
	}

	return fRet;
}

void CBsHFWorld::GetSlope(float fX, float fZ, float& fVX, float& fVZ)
{
	int nCellX, nCellZ;
	float fModX, fModZ;
	float fHeight[4];

	nCellX=(int)(fX/DISTANCE_PER_CELL);
	nCellZ=m_nWorldZSize-(int)(fZ/DISTANCE_PER_CELL)-1;
	fModX=fX/DISTANCE_PER_CELL-nCellX;
	fModZ=fZ/DISTANCE_PER_CELL+nCellZ-m_nWorldZSize+1;

	fHeight[0]=m_pHeight[nCellZ*(m_nWorldXSize+1)+nCellX];
	fHeight[1]=m_pHeight[nCellZ*(m_nWorldXSize+1)+nCellX+1];
	fHeight[2]=m_pHeight[(nCellZ+1)*(m_nWorldXSize+1)+nCellX];
	fHeight[3]=m_pHeight[(nCellZ+1)*(m_nWorldXSize+1)+nCellX+1];

	fVX=((fHeight[1]-fHeight[0])*fModZ+(fHeight[3]-fHeight[2])*(1-fModZ)) / DISTANCE_PER_CELL;
	fVZ=((fHeight[0]-fHeight[2])*(1-fModX)+(fHeight[1]-fHeight[3])*fModX) / DISTANCE_PER_CELL;
}

void CBsHFWorld::CheckVisibleZone()
{
	int i, j;
	int nPosX, nPosY, nCurrentZoneIndex;

	const D3DXMATRIX* pCamera = g_BsKernel.GetParamInvViewMatrix();

	nPosX=(int)(pCamera->_41/DISTANCE_PER_CELL/m_nXSizePerZone);
	nPosY=(int)((pCamera->_43/DISTANCE_PER_CELL)/m_nZSizePerZone);
	nCurrentZoneIndex=nPosY*m_nZoneHoriCount+nPosX;

	for(i=0;i<MAX_VISIBLE_ZONE_COUNT;i++){
		if(m_bUse[i]){
			if(nCurrentZoneIndex!=m_nZoneIndex[i]){
				if(!IsVisibleTestFromFrustum(m_nZoneIndex[i])){
					// VertexBuffer가 있지만, No Visible이다.
					m_bUse[i]=false;
				}
			}
		}
	}
	for(i=nPosY-m_nZoneCheckOffSet;i<=nPosY+m_nZoneCheckOffSet;i++){
		if((i<0)||(i>=m_nZoneVertCount)){
			continue;
		}
		for(j=nPosX-m_nZoneCheckOffSet;j<=nPosX+m_nZoneCheckOffSet;j++){
			if((j<0)||(j>=m_nZoneHoriCount)){
				continue;
			}
			if((nCurrentZoneIndex==i*m_nZoneHoriCount+j)||(IsVisibleTestFromFrustum(i*m_nZoneHoriCount+j))){
				CreateVisibleZone(i*m_nZoneHoriCount+j);
			}
		}
	}
}

bool CBsHFWorld::IsVisibleTestFromFrustum(int nIndex)
{
	float fStartX, fStartZ, fEndX, fEndZ;
	//	bool bInside=false;

	fStartX=(nIndex%m_nZoneHoriCount*m_nXSizePerZone*DISTANCE_PER_CELL);
	fStartZ=((nIndex/m_nZoneHoriCount*m_nZSizePerZone)*DISTANCE_PER_CELL);
	fEndX=fStartX + m_nXSizePerZone*DISTANCE_PER_CELL;
	fEndZ=fStartZ + m_nZSizePerZone*DISTANCE_PER_CELL;

	Box3 BoxZone;

	BoxZone.C = BSVECTOR((fStartX+fEndX)*0.5f,(m_pMaxHeight[nIndex]+m_pMinHeight[nIndex])*0.5f,(fStartZ+fEndZ)*0.5f);
	BoxZone.A[0]=BSVECTOR(1.f,0.f,0.f);
	BoxZone.A[1]=BSVECTOR(0.f,1.f,0.f);
	BoxZone.A[2]=BSVECTOR(0.f,0.f,1.f);

	BoxZone.E[0]=fabsf((fEndX-fStartX)*0.5f);
	BoxZone.E[1]=fabsf((m_pMaxHeight[nIndex]-m_pMinHeight[nIndex])*0.5f);
	BoxZone.E[2]=fabsf((fEndZ-fStartZ)*0.5f);

	BoxZone.compute_vertices();

	return g_BsKernel.IsVisibleTestFromCameraForGiantByRender(&BoxZone);
}

void CBsHFWorld::CreateVisibleZone(int nZoneIndex)
{

	int i, j;
	int nStartCellX, nStartCellZ;
	float fTextureU, fTextureV;
	int nEmptyZone, nBufIndex, nHeightIndex;
	//	short int nTileNum, nDir;

	for(i=0;i<MAX_VISIBLE_ZONE_COUNT;i++){
		if(m_nZoneIndex[i]==nZoneIndex){
			if(!m_bUse[i]){
				m_bUse[i]=true;
			}
			return;
		}
	}
	nEmptyZone=SearchEmptyZone();
	if(nEmptyZone==-1){
		assert(0);
		return;
	}

	nStartCellX=nZoneIndex%m_nZoneHoriCount*m_nXSizePerZone;
	nStartCellZ=nZoneIndex/m_nZoneHoriCount*m_nZSizePerZone;

	HFVERTEX *pBuf;
	m_pVB[nEmptyZone]->Lock(0, 0, (void **)&pBuf, 0);
	for(i=0;i<m_nZSizePerZone+1;i++){
		if(i+nStartCellZ>m_nWorldZSize){
			break;
		}
		for(j=0;j<m_nXSizePerZone+1;j++){
			if(j+nStartCellX>m_nWorldXSize){
				break;
			}

			if(i+nStartCellZ<0) {	// m_nWorldYSize와 m_nRealWorldYSize가 틀리 경우 문제를 막기 위해 넣음 yooty
				continue;
			}

			nBufIndex=(i*(m_nXSizePerZone+1)+j);
			fTextureU = float(nStartCellX+j)/float(m_nWorldXSize);
			fTextureV = 1.f-float(nStartCellZ+i)/float(m_nWorldZSize);

			nHeightIndex=(m_nWorldXSize+1)*(i+nStartCellZ)+j+nStartCellX;

			pBuf[nBufIndex].vecPos.y=m_pHeight[nHeightIndex];
			pBuf[nBufIndex].dwMulColor = m_pdwMul[nHeightIndex];
			pBuf[nBufIndex].dwAddColor = m_pdwAdd[nHeightIndex];
			pBuf[nBufIndex].vecUV.x=fTextureU;
			pBuf[nBufIndex].vecUV.y=fTextureV;
			pBuf[nBufIndex].vecNor=GetNormalVector(j+nStartCellX, i+nStartCellZ);
		}
	}
	m_pVB[nEmptyZone]->Unlock();

	m_nZoneIndex[nEmptyZone]=nZoneIndex;
	m_bUse[nEmptyZone]=true;
	m_nUseCount[nEmptyZone]=0;
}

int CBsHFWorld::SearchEmptyZone()
{
	int i, nUseCount, nIndex;

	nUseCount=INT_MAX;
	nIndex=-1;
	for(i=0;i<MAX_VISIBLE_ZONE_COUNT;i++){
		if(m_nZoneIndex[i]==-1){
			return i;
		}
		if(!m_bUse[i]){
			if(nUseCount>m_nUseCount[i]){
				nUseCount=m_nUseCount[i];
				nIndex=i;
			}
		}
	}
	return nIndex;
}