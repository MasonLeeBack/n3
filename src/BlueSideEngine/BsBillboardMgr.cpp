#include "stdafx.h"
#include ".\bsbillboardmgr.h"
#include "BsMaterial.h"
#include "BsSkin.h"
#include "BsUtil.h"
#include "BsObject.h"

CBsBillboardMgr::CBsBillboardMgr()
{
	m_pZoneList = NULL;
	m_nXZoneCount = m_nZZoneCount = 0;
	m_nNumRenderBillboard = 0;
	m_pVB	    = NULL;
	m_nBillboardVertexDeclIndex = -1;
	m_nMaterialIndex = -1;
	m_nTextureIndex = -1;
	m_hTexture = m_hInvV = m_hVP = m_hCommon = m_hFogFactor = m_hFogColor = m_hLightAmbient = NULL;
	m_hRCPXSize = m_hRCPZSize = m_hLightTexture = m_hObjTexture = NULL;
	m_bRenderReady = false;
	m_pArraySprite = NULL;
	m_nRenderBillboard = 0;
	m_nSpriteNum = 0;
}

CBsBillboardMgr::~CBsBillboardMgr()
{
}

HRESULT CBsBillboardMgr::InitBillBoard(int iMapXSize, int iMapZSize, const char* pTexName, int nDataNum, SPRITE_DATA* pData)
{
	D3DVERTEXELEMENT9	BillboardVertexDecl[] = {
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },		// WPostion
		{ 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },		// Texcoord
		{ 0, 20, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },		// OffsetPos
		D3DDECL_END()
	};
	BsAssert( m_nBillboardVertexDeclIndex < 0 && "Duplicated Index!!");
	m_nBillboardVertexDeclIndex = g_BsKernel.LoadVertexDeclaration(BillboardVertexDecl);

	char fullName[_MAX_PATH];
	strcpy(fullName, g_BsKernel.GetShaderDirectory());
	strcat(fullName, "billboard.fx");
	m_nMaterialIndex = g_BsKernel.LoadMaterial(fullName, FALSE );
	if (m_nMaterialIndex == -1)
		return E_FAIL;

	CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialIndex);

#ifdef _XBOX
	m_hTexture = pMaterial->GetParameterByName("g_Sampler");
	m_hObjTexture = pMaterial->GetParameterByName("g_ObjSampler");
	m_hVP = pMaterial->GetParameterByName("VIEWPROJ");
	m_hInvV = pMaterial->GetParameterByName("INVERSEVIEW");
	m_hCommon = pMaterial->GetParameterByName("COMMON");
	m_hFogFactor = pMaterial->GetParameterByName("FOGFACTOR");
	m_hFogColor = pMaterial->GetParameterByName("FOGCOLOR");
	m_hLightAmbient = pMaterial->GetParameterByName("LIGHTAMBIENT");
	m_hLightTexture = pMaterial->GetParameterByName("lightmapSampler");
	m_hRCPXSize = pMaterial->GetParameterByName("RCPWORLDXSIZE");
	m_hRCPZSize = pMaterial->GetParameterByName("RCPWORLDZSIZE");
#else
	m_hTexture = pMaterial->GetParameterByName("Tex0");
	m_hObjTexture = pMaterial->GetParameterByName("ObjTex");
	m_hVP = pMaterial->GetParameterByName("matVP");
	m_hInvV = pMaterial->GetParameterByName("matInvV");
	m_hCommon = pMaterial->GetParameterByName("vCommon");
	m_hFogFactor = pMaterial->GetParameterByName("fogFactor");
	m_hFogColor = pMaterial->GetParameterByName("fogColor");
	m_hLightAmbient = pMaterial->GetParameterByName("lightAmbient");
	m_hLightTexture = pMaterial->GetParameterByName("lightmapTexture");
	m_hRCPXSize = pMaterial->GetParameterByName("fRcpWorldXSize");
	m_hRCPZSize = pMaterial->GetParameterByName("fRcpWorldZSize");
#endif

	UINT uiBufferSize = MAX_RENDER_BILLBOARD * sizeof(BILLBOARD_VERTEX) * NUM_VERTEX_PER_BILLBOARD;
	g_BsKernel.CreateVertexBuffer(uiBufferSize, D3DUSAGE_WRITEONLY, NULL, D3DPOOL_MANAGED, &m_pVB);
	if(!m_pVB) {
		BsAssert( 0 && "Failed CreateVertexBuffer!!");
		return E_FAIL;
	}

	CreateBillboardZone(iMapXSize, iMapZSize);

	if (nDataNum > 0) {
		m_nSpriteNum = nDataNum;
		m_pArraySprite = new SPRITE_DATA[m_nSpriteNum];
		
		for (int i = 0; i < m_nSpriteNum; i++) {
			m_pArraySprite[i] = pData[i];
		}

#ifdef _USAGE_TOOL_
		strcpy( fullName, g_BsKernel.GetCurrentDirectory() );
		strcat( fullName, "prop\\");
#else
		strcpy(fullName, "d:\\data\\prop\\");
#endif
		strcat(fullName, pTexName);

		SpriteTextureLoad(fullName);
	}
	return S_OK;
}

HRESULT CBsBillboardMgr::SpriteTextureLoad(LPCSTR szFileName)
{
	m_nTextureIndex = g_BsKernel.LoadTexture(szFileName);
	if (m_nTextureIndex == -1)
		return E_FAIL;
	return S_OK;
}

void CBsBillboardMgr::CreateBillboardZone(int nMapXSize, int nMapZSize)
{
	if (nMapXSize > ZONE_SIZE) {
		m_nXZoneCount = nMapXSize / ZONE_SIZE;
		if (nMapXSize % ZONE_SIZE)
			m_nXZoneCount += 1;
	}
	else {
		m_nXZoneCount = 1;
	}

	if (nMapZSize > ZONE_SIZE) {
		m_nZZoneCount = nMapZSize / ZONE_SIZE;
		if (nMapZSize % ZONE_SIZE)
			m_nZZoneCount += 1;
	}
	else {
		m_nZZoneCount = 1;
	}

	m_pZoneList = new ZONE_DATA[m_nXZoneCount * m_nZZoneCount];

	for(int i=0 ; i<m_nZZoneCount ; ++i ) {
		for(int j=0 ; j<m_nXZoneCount ; ++j ) {
			int nZoneIndex = i*m_nXZoneCount + j;

			float fZoneSizeXMin, fZoneSizeXMax, fZoneSizeZMin, fZoneSizeZMax;
			fZoneSizeXMin = float(j)*float(ZONE_SIZE);
			fZoneSizeXMax = fZoneSizeXMin + float(ZONE_SIZE);

			fZoneSizeZMin = float(i)*float(ZONE_SIZE);
			fZoneSizeZMax = fZoneSizeZMin + float(ZONE_SIZE);

			m_pZoneList[nZoneIndex].ZoneBoundingBox.C.x = 0.5f*(fZoneSizeXMax+fZoneSizeXMin);
			m_pZoneList[nZoneIndex].ZoneBoundingBox.C.y = 0.f;
			m_pZoneList[nZoneIndex].ZoneBoundingBox.C.z = 0.5f*(fZoneSizeZMax+fZoneSizeZMin);

			m_pZoneList[nZoneIndex].ZoneBoundingBox.A[0] = BSVECTOR( 1.f, 0.f, 0.f);
			m_pZoneList[nZoneIndex].ZoneBoundingBox.A[1] = BSVECTOR( 0.f, 1.f, 0.f);
			m_pZoneList[nZoneIndex].ZoneBoundingBox.A[2] = BSVECTOR( 0.f, 0.f, 1.f);

			m_pZoneList[nZoneIndex].ZoneBoundingBox.E[0] = 0.5f*float(ZONE_SIZE);
			m_pZoneList[nZoneIndex].ZoneBoundingBox.E[1] = 0.f;
			m_pZoneList[nZoneIndex].ZoneBoundingBox.E[2] = 0.5f*float(ZONE_SIZE);
		}
	}
}

void CBsBillboardMgr::PutPosToPool(D3DXVECTOR3 vWPos, D3DXVECTOR2 vOffset, int nBillboardIndex)
{
	BILLBOARD_DATA	Data;
	Data.vecWorldPos = vWPos;
	Data.vecOffset = vOffset;
	Data.nBufferIndex = nBillboardIndex;

	const float fRcpZoneSize = 1.f/float(ZONE_SIZE);

	int nXZoneIndex = int(vWPos.x*fRcpZoneSize);
	int nZZoneIndex = int(vWPos.z*fRcpZoneSize);
	int nCurrentZoneIndex = nZZoneIndex*m_nXZoneCount+nXZoneIndex;

	ZONE_DATA* pZone = &m_pZoneList[nCurrentZoneIndex];


	pZone->arrayBillboardData.push_back(Data);
	pZone->bZoneEnable = true;

	BOOL bIsBoundingBoxRefresh = FALSE;

	if(vWPos.y>pZone->fZoneMaxHeight) {
		pZone->fZoneMaxHeight = vWPos.y;
		bIsBoundingBoxRefresh = TRUE;
	}
	if(vWPos.y<pZone->fZoneMinHeight) {
		pZone->fZoneMinHeight = vWPos.y;
		bIsBoundingBoxRefresh = TRUE;
	}

	// BoundingBox Refresh
	if(bIsBoundingBoxRefresh) {
		float fMiddle = 0.5f*(pZone->fZoneMinHeight + pZone->fZoneMaxHeight);
		float fGap = pZone->fZoneMaxHeight - pZone->fZoneMinHeight;
		pZone->ZoneBoundingBox.C.y = fMiddle;
		pZone->ZoneBoundingBox.E[1] = 0.5f*fGap;
		pZone->ZoneBoundingBox.compute_vertices();
	}
}

D3DXVECTOR2	*CBsBillboardMgr::FindTexCoord(int nBufferIndex)
{
	if (nBufferIndex > 0) {
		for (int i = 0; i < m_nSpriteNum; i++) {
			if (m_pArraySprite[i].nIndex == nBufferIndex)
				return m_pArraySprite[i].vecTexCoord;
		}
	}
	return NULL;
}

void CBsBillboardMgr::OptiZone()
{
	int nCurrentZoneIndex = 0;
	for (int i = 0; i < m_nXZoneCount; i++) {
		for (int j = 0; j < m_nZZoneCount; j++) {
			if (m_pZoneList[nCurrentZoneIndex].bZoneEnable) {
				m_arrayZoneIndices.push_back(nCurrentZoneIndex);
				m_pZoneList[nCurrentZoneIndex].pRenderVertices = new BILLBOARD_VERTEX[m_pZoneList[nCurrentZoneIndex].arrayBillboardData.size() * NUM_VERTEX_PER_BILLBOARD];
				for (int k = 0; k < (int)m_pZoneList[nCurrentZoneIndex].arrayBillboardData.size(); k++) {
#ifdef _XBOX
					m_pZoneList[nCurrentZoneIndex].pRenderVertices[k*4].WPos = m_pZoneList[nCurrentZoneIndex].pRenderVertices[k*4+1].WPos = 
						m_pZoneList[nCurrentZoneIndex].pRenderVertices[k*4+2].WPos = m_pZoneList[nCurrentZoneIndex].pRenderVertices[k*4+3].WPos = 
						m_pZoneList[nCurrentZoneIndex].arrayBillboardData[k].vecWorldPos;

					int iIndex = m_pZoneList[nCurrentZoneIndex].arrayBillboardData[k].nBufferIndex;		// 지금은 Skin Index
					D3DXVECTOR2*	vTexTB = m_pArraySprite[iIndex].vecTexCoord;

					float fOffsetX = m_pZoneList[nCurrentZoneIndex].arrayBillboardData[k].vecOffset.x/2;
					float fOffsetY = m_pZoneList[nCurrentZoneIndex].arrayBillboardData[k].vecOffset.y;
					m_pZoneList[nCurrentZoneIndex].pRenderVertices[k*4].LPos = D3DXVECTOR2(-fOffsetX, fOffsetY);
					m_pZoneList[nCurrentZoneIndex].pRenderVertices[k*4].Tex = vTexTB[0];
					m_pZoneList[nCurrentZoneIndex].pRenderVertices[k*4+1].LPos = D3DXVECTOR2(fOffsetX, fOffsetY);
					m_pZoneList[nCurrentZoneIndex].pRenderVertices[k*4+1].Tex = D3DXVECTOR2(vTexTB[1].x, vTexTB[0].y);
					m_pZoneList[nCurrentZoneIndex].pRenderVertices[k*4+2].LPos = D3DXVECTOR2(fOffsetX, 0.0f);
					m_pZoneList[nCurrentZoneIndex].pRenderVertices[k*4+2].Tex = vTexTB[1];
					m_pZoneList[nCurrentZoneIndex].pRenderVertices[k*4+3].LPos = D3DXVECTOR2(-fOffsetX, 0.0f);
					m_pZoneList[nCurrentZoneIndex].pRenderVertices[k*4+3].Tex = D3DXVECTOR2(vTexTB[0].x, vTexTB[1].y);
#else
					m_pZoneList[nCurrentZoneIndex].pRenderVertices[k*6].WPos = m_pZoneList[nCurrentZoneIndex].pRenderVertices[k*6+1].WPos = 
						m_pZoneList[nCurrentZoneIndex].pRenderVertices[k*6+2].WPos = m_pZoneList[nCurrentZoneIndex].pRenderVertices[k*6+3].WPos =
						m_pZoneList[nCurrentZoneIndex].pRenderVertices[k*6+4].WPos = m_pZoneList[nCurrentZoneIndex].pRenderVertices[k*6+5].WPos = 
						m_pZoneList[nCurrentZoneIndex].arrayBillboardData[k].vecWorldPos;

					int iIndex = m_pZoneList[nCurrentZoneIndex].arrayBillboardData[k].nBufferIndex;
					D3DXVECTOR2*	vTexTB = m_pArraySprite[iIndex].vecTexCoord;

					float fOffsetX = m_pZoneList[nCurrentZoneIndex].arrayBillboardData[k].vecOffset.x/2;
					float fOffsetY = m_pZoneList[nCurrentZoneIndex].arrayBillboardData[k].vecOffset.y;
					m_pZoneList[nCurrentZoneIndex].pRenderVertices[k*6].LPos = D3DXVECTOR2(-fOffsetX, fOffsetY);
					m_pZoneList[nCurrentZoneIndex].pRenderVertices[k*6].Tex = vTexTB[0];
					m_pZoneList[nCurrentZoneIndex].pRenderVertices[k*6+1].LPos = D3DXVECTOR2(fOffsetX, fOffsetY);
					m_pZoneList[nCurrentZoneIndex].pRenderVertices[k*6+1].Tex = D3DXVECTOR2(vTexTB[1].x, vTexTB[0].y);
					m_pZoneList[nCurrentZoneIndex].pRenderVertices[k*6+2].LPos = D3DXVECTOR2(-fOffsetX, 0.0f);
					m_pZoneList[nCurrentZoneIndex].pRenderVertices[k*6+2].Tex = D3DXVECTOR2(vTexTB[0].x, vTexTB[1].y);
					m_pZoneList[nCurrentZoneIndex].pRenderVertices[k*6+3].LPos = D3DXVECTOR2(fOffsetX, fOffsetY);
					m_pZoneList[nCurrentZoneIndex].pRenderVertices[k*6+3].Tex = D3DXVECTOR2(vTexTB[1].x, vTexTB[0].y);
					m_pZoneList[nCurrentZoneIndex].pRenderVertices[k*6+4].LPos = D3DXVECTOR2(fOffsetX, 0.0f);
					m_pZoneList[nCurrentZoneIndex].pRenderVertices[k*6+4].Tex = vTexTB[1];
					m_pZoneList[nCurrentZoneIndex].pRenderVertices[k*6+5].LPos = D3DXVECTOR2(-fOffsetX, 0.0f);
					m_pZoneList[nCurrentZoneIndex].pRenderVertices[k*6+5].Tex = D3DXVECTOR2(vTexTB[0].x, vTexTB[1].y);
#endif
				}	// end for()
			}
			nCurrentZoneIndex += 1;
			m_bRenderReady = true;
		}
	}
}

void CBsBillboardMgr::RefreshZone(D3DXVECTOR3 vecPos)
{
	const float fRcpZoneSize = 1.f/float(ZONE_SIZE);
	int nXZoneIndex = int(vecPos.x*fRcpZoneSize);
	int nZZoneIndex = int(vecPos.z*fRcpZoneSize);

	RefreshZone(nXZoneIndex, nZZoneIndex);
}

void CBsBillboardMgr::RefreshZone(int nXZoneIndex, int nZZoneIndex)
{
	int nCurrentZoneIndex = nZZoneIndex*m_nXZoneCount+nXZoneIndex;

	ZONE_DATA* pZone = &m_pZoneList[nCurrentZoneIndex];

	SAFE_DELETEA(pZone->pRenderVertices);

	UINT uiValidZoneIndexCount = m_arrayZoneIndices.size();
	bool bFind = false;
	for(UINT uiCount=0;uiCount<uiValidZoneIndexCount;++uiCount) {
		if(m_arrayZoneIndices[uiCount]==nCurrentZoneIndex) {
			bFind = true;
		}
	}
	if(!bFind) {
		m_arrayZoneIndices.push_back(nCurrentZoneIndex);
	}

	pZone->pRenderVertices = new BILLBOARD_VERTEX[pZone->arrayBillboardData.size() * NUM_VERTEX_PER_BILLBOARD];
	for (int k = 0; k < (int)pZone->arrayBillboardData.size(); k++) {
#ifdef _XBOX
		pZone->pRenderVertices[k*4].WPos = pZone->pRenderVertices[k*4+1].WPos = 
			pZone->pRenderVertices[k*4+2].WPos = pZone->pRenderVertices[k*4+3].WPos = 
			pZone->arrayBillboardData[k].vecWorldPos;

		int iIndex = pZone->arrayBillboardData[k].nBufferIndex;		// 지금은 Skin Index
		D3DXVECTOR2*	vTexTB = m_pArraySprite[iIndex].vecTexCoord;

		float fOffsetX = pZone->arrayBillboardData[k].vecOffset.x/2;
		float fOffsetY = pZone->arrayBillboardData[k].vecOffset.y;
		pZone->pRenderVertices[k*4].LPos = D3DXVECTOR2(-fOffsetX, fOffsetY);
		pZone->pRenderVertices[k*4].Tex = vTexTB[0];
		pZone->pRenderVertices[k*4+1].LPos = D3DXVECTOR2(fOffsetX, fOffsetY);
		pZone->pRenderVertices[k*4+1].Tex = D3DXVECTOR2(vTexTB[1].x, vTexTB[0].y);
		pZone->pRenderVertices[k*4+2].LPos = D3DXVECTOR2(fOffsetX, 0.0f);
		pZone->pRenderVertices[k*4+2].Tex = vTexTB[1];
		pZone->pRenderVertices[k*4+3].LPos = D3DXVECTOR2(-fOffsetX, 0.0f);
		pZone->pRenderVertices[k*4+3].Tex = D3DXVECTOR2(vTexTB[0].x, vTexTB[1].y);
#else
		pZone->pRenderVertices[k*6].WPos = pZone->pRenderVertices[k*6+1].WPos = 
			pZone->pRenderVertices[k*6+2].WPos = pZone->pRenderVertices[k*6+3].WPos =
			pZone->pRenderVertices[k*6+4].WPos = pZone->pRenderVertices[k*6+5].WPos = 
			pZone->arrayBillboardData[k].vecWorldPos;

		int iIndex = pZone->arrayBillboardData[k].nBufferIndex;
		D3DXVECTOR2*	vTexTB = FindTexCoord(iIndex);

		float fOffsetX = pZone->arrayBillboardData[k].vecOffset.x/2;
		float fOffsetY = pZone->arrayBillboardData[k].vecOffset.y;
		pZone->pRenderVertices[k*6].LPos = D3DXVECTOR2(-fOffsetX, fOffsetY);
		pZone->pRenderVertices[k*6].Tex = vTexTB[0];
		pZone->pRenderVertices[k*6+1].LPos = D3DXVECTOR2(fOffsetX, fOffsetY);
		pZone->pRenderVertices[k*6+1].Tex = D3DXVECTOR2(vTexTB[1].x, vTexTB[0].y);
		pZone->pRenderVertices[k*6+2].LPos = D3DXVECTOR2(-fOffsetX, 0.0f);
		pZone->pRenderVertices[k*6+2].Tex = D3DXVECTOR2(vTexTB[0].x, vTexTB[1].y);
		pZone->pRenderVertices[k*6+3].LPos = D3DXVECTOR2(fOffsetX, fOffsetY);
		pZone->pRenderVertices[k*6+3].Tex = D3DXVECTOR2(vTexTB[1].x, vTexTB[0].y);
		pZone->pRenderVertices[k*6+4].LPos = D3DXVECTOR2(fOffsetX, 0.0f);
		pZone->pRenderVertices[k*6+4].Tex = vTexTB[1];
		pZone->pRenderVertices[k*6+5].LPos = D3DXVECTOR2(-fOffsetX, 0.0f);
		pZone->pRenderVertices[k*6+5].Tex = D3DXVECTOR2(vTexTB[0].x, vTexTB[1].y);
#endif
	}	// end for()
}


void CBsBillboardMgr::ClearZone(int nXZoneIndex, int nZZoneIndex)
{
	int nCurrentZoneIndex = nZZoneIndex*m_nXZoneCount+nXZoneIndex;
	BsAssert( m_pZoneList );

	ZONE_DATA* pZone = &m_pZoneList[nCurrentZoneIndex];
	pZone->arrayBillboardData.clear();
	pZone->bZoneEnable = false;
	SAFE_DELETEA(pZone->pRenderVertices);
}

void CBsBillboardMgr::PreRender(void)
{
	int nRenderZoneSize = (int)m_arrayZoneIndices.size();
	int i = 0, j = 0, h = 0;
	int size = 0;
	int CheckCount = 0;
	m_nNumRenderBillboard = 0;
	BSVECTOR vecCamPos;

	vecCamPos.x = g_BsKernel.GetParamInvViewMatrix()->_41;
	vecCamPos.y = g_BsKernel.GetParamInvViewMatrix()->_42;
	vecCamPos.z = g_BsKernel.GetParamInvViewMatrix()->_43;

	for (i = 0; i < nRenderZoneSize; i++) {
		int nZoneIndex = m_arrayZoneIndices[i];
		if(!m_pZoneList[nZoneIndex].bZoneEnable)
			continue;
		BSVECTOR vecDist = m_pZoneList[nZoneIndex].ZoneBoundingBox.C - vecCamPos;
		float fDistance = BsVec3Length(&vecDist);
		if(fDistance>5000.f)
			continue;

		if (IsVisibleTest(nZoneIndex)) {
			// CHECK_RANGE범위 안의 Zone들.
			BILLBOARD_VERTEX* pVerts;
			CheckCount += (int)m_pZoneList[nZoneIndex].arrayBillboardData.size();
			if (CheckCount < MAX_RENDER_BILLBOARD) {
				m_pVB->Lock(m_nNumRenderBillboard * sizeof(BILLBOARD_VERTEX) * NUM_VERTEX_PER_BILLBOARD,
					sizeof(BILLBOARD_VERTEX) * (int)m_pZoneList[nZoneIndex].arrayBillboardData.size() * NUM_VERTEX_PER_BILLBOARD, (void**)&pVerts, 0);
				memcpy(pVerts, m_pZoneList[nZoneIndex].pRenderVertices, sizeof(BILLBOARD_VERTEX) * NUM_VERTEX_PER_BILLBOARD * (int)m_pZoneList[nZoneIndex].arrayBillboardData.size());
				m_nNumRenderBillboard += (int)m_pZoneList[nZoneIndex].arrayBillboardData.size();
				m_pVB->Unlock();
			}
			else {
				int GapSize = MAX_RENDER_BILLBOARD - m_nNumRenderBillboard;
				m_pVB->Lock(m_nNumRenderBillboard * sizeof(BILLBOARD_VERTEX) * NUM_VERTEX_PER_BILLBOARD, sizeof(BILLBOARD_VERTEX) * GapSize * NUM_VERTEX_PER_BILLBOARD, (void**)&pVerts, 0);
				memcpy(pVerts, m_pZoneList[nZoneIndex].pRenderVertices, sizeof(BILLBOARD_VERTEX) * GapSize * NUM_VERTEX_PER_BILLBOARD);
				m_nNumRenderBillboard += GapSize;
				m_pVB->Unlock();
				break;
			}
		}
	}
}

bool CBsBillboardMgr::IsVisibleTest(int nZoneIndex)
{
	return g_BsKernel.IsVisibleTestFromCamera(&m_pZoneList[nZoneIndex].ZoneBoundingBox);
}

void CBsBillboardMgr::ReleaseDeviceData(void)
{
	SAFE_RELEASE(m_pVB);

	SAFE_RELEASE_TEXTURE(m_nTextureIndex);
	SAFE_RELEASE_MATERIAL(m_nMaterialIndex);
	SAFE_RELEASE_VD(m_nBillboardVertexDeclIndex);

	m_bRenderReady = false;
	m_arrayZoneIndices.clear();
	if (m_pZoneList){
		delete [] m_pZoneList; m_pZoneList = NULL;
	}
	if (m_pArraySprite) {
		delete [] m_pArraySprite; m_pArraySprite = NULL;
	}
}

void CBsBillboardMgr::Render(C3DDevice* pDevice)
{
	if (!m_bRenderReady)
		return;

	if( m_nMaterialIndex == -1 )
		return;

	PreRender();

	if (m_nNumRenderBillboard > 0) {
		D3DXVECTOR4 vCommon((float)g_BsKernel.GetRenderTick() / 30.0f, CHECK_RANGE - 2000.0f, 1.0f, 1.0f);			
		CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialIndex);

		g_BsKernel.SetVertexDeclaration(m_nBillboardVertexDeclIndex);

		pMaterial->BeginMaterial(0, 0);

		pMaterial->SetVector(m_hCommon, &vCommon);
		pMaterial->SetVector(m_hFogFactor, g_BsKernel.GetParamFogFactor());
		pMaterial->SetVector(m_hFogColor, g_BsKernel.GetParamFogColor());
		pMaterial->SetVector(m_hLightAmbient, g_BsKernel.GetLightAmbient());
		if (g_BsKernel.GetWorldLightMap() != -1) {
			pMaterial->SetTexture(m_hLightTexture, g_BsKernel.GetWorldLightMap());
		}
		else {
			pMaterial->SetTexture(m_hLightTexture, g_BsKernel.GetBlankTexture());
		}
		pMaterial->SetFloat(m_hRCPXSize, g_BsKernel.GetRcpWorldXSize());
		pMaterial->SetFloat(m_hRCPZSize, g_BsKernel.GetRcpWorldZSize());

		pMaterial->SetMatrix(m_hInvV, g_BsKernel.GetParamInvViewMatrix());
		pMaterial->SetMatrix(m_hVP, g_BsKernel.GetParamViewProjectionMatrix());

		pMaterial->BeginPass(0);
		pMaterial->SetTexture(m_hTexture, m_nTextureIndex);
		pDevice->SetStreamSource(0, m_pVB, 0, sizeof(BILLBOARD_VERTEX));;
		pMaterial->CommitChanges();

		pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
		pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
		pDevice->SetRenderState(D3DRS_ALPHAREF, 0x7f);
#ifdef _XBOX
		pDevice->DrawPrimitiveVB(D3DPT_QUADLIST, 0, m_nNumRenderBillboard);
#else
		pDevice->DrawPrimitiveVB(D3DPT_TRIANGLELIST, 0, m_nNumRenderBillboard * 2);
#endif
		pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

		m_nRenderBillboard = m_nNumRenderBillboard;

		pMaterial->EndPass();
		pMaterial->EndMaterial();
	}
}

CBsObjectBillboardMgr::CBsObjectBillboardMgr()
{
	m_hObjectTexture = NULL;
	m_nVertexDeclIndex = -1;
	m_nCurrentPool = 0;
	SetCurrentBillboardRange(5000.f);

	memset(m_arrayObjectBillboardPool, 0, sizeof(OBJECT_BILLBOARD_DATA*)*MAX_NUM_OBJECT);
}

CBsObjectBillboardMgr::~CBsObjectBillboardMgr()
{
}

int CBsObjectBillboardMgr::FindBillboard( int *pSkinIndex, int nNumVari )
{
	int i, j;

	for( i = 0; i < m_nCurrentPool; i++ )
	{
		for( j = 0; j < nNumVari; j++ )
		{
			if( m_arrayObjectBillboardPool[ i ]->pnSkinIndices[ j ] != pSkinIndex[ j ] )
			{
				break;
			}
		}
		if( j >= nNumVari )
		{
			return i;
		}
	}

	return -1;
}

int CBsObjectBillboardMgr::RegisterAniBillboardType(const char* szFolderName, int* pnSkinIndices, int nSkinCount,  int iNumAnim, int iBillboardSize)
{
	int nTextureIndex;
#ifdef _XBOX
	nTextureIndex = g_BsKernel.LoadArrayTexture(szFolderName);
	if (nTextureIndex == -1)
	{
		return -1;
	}
#else
	return -1; // PC버전은 아직 지원이 안됨... 
#endif
	AvailMemoryDifference temp(0, "Unit Billboard Register");

	OBJECT_BILLBOARD_DATA* pData = new OBJECT_BILLBOARD_DATA();

	pData->nSkinCount = nSkinCount;
	pData->nBillboardSize = iBillboardSize;
	pData->pnSkinIndices = new int[nSkinCount];
	pData->pMaterialAmb = new D3DXVECTOR4[nSkinCount];
	pData->pMaterialDiff = new D3DXVECTOR4[nSkinCount];
	for (int i = 0; i < nSkinCount; i++) {
		int iSkinInd = pnSkinIndices[i];
		pData->pMaterialAmb[i] =  g_BsKernel.Get_pSkin(iSkinInd)->GetSkinMaterialAVParam(PARAM_MATERIALAMBIENT);
		pData->pMaterialDiff[i] = g_BsKernel.Get_pSkin(iSkinInd)->GetSkinMaterialAVParam(PARAM_MATERIALDIFFUSE);
	}
	memset(pData->pnSkinIndices, 0, sizeof(int)*nSkinCount);
	for (int i = 0; i < nSkinCount; i++) 
		pData->pnSkinIndices[i] = pnSkinIndices[i];
	pData->nTextureIndex = nTextureIndex;
	
	UINT uiBufferSize = MAX_NUM_OBJECT_BILLBOARD * sizeof(OBJ_BILLBOARD_VERTEX) * NUM_VERTEX_PER_BILLBOARD;
	g_BsKernel.CreateVertexBuffer(uiBufferSize, D3DUSAGE_WRITEONLY, NULL, D3DPOOL_MANAGED, &pData->pVB);
	if(!pData->pVB) {
		BsAssert( 0 && "Failed CreateVertexBuffer!!");
		return -1;
	}

	pData->pVertices[0] = new OBJ_BILLBOARD_VERTEX[MAX_NUM_OBJECT_BILLBOARD * NUM_VERTEX_PER_BILLBOARD];
	pData->pVertices[1] = new OBJ_BILLBOARD_VERTEX[MAX_NUM_OBJECT_BILLBOARD * NUM_VERTEX_PER_BILLBOARD];
	pData->pAnimationOffset = new BillboardAnimOffset[iNumAnim];

	m_arrayObjectBillboardPool[m_nCurrentPool] = pData;

	return m_nCurrentPool++;
}

void CBsObjectBillboardMgr::SetBillboardAniOffset(int nBillboardIndex, int nAniIndex, float fAniOffset, int nAniType, int nAniAttr)
{
	m_arrayObjectBillboardPool[nBillboardIndex]->pAnimationOffset[nAniIndex].fOffset = fAniOffset;
	m_arrayObjectBillboardPool[nBillboardIndex]->pAnimationOffset[nAniIndex].nAnimType = nAniType;
	m_arrayObjectBillboardPool[nBillboardIndex]->pAnimationOffset[nAniIndex].nAniAttr = nAniAttr;
}

HRESULT CBsObjectBillboardMgr::InitBillBoard(int iMapXSize, int iMapZSize, const char* pTexName, int nDataNum, SPRITE_DATA* pData)
{
	CBsBillboardMgr::InitBillBoard(iMapXSize, iMapZSize, pTexName, nDataNum, pData);		// Grass

	D3DVERTEXELEMENT9 ObjBillboardVertexDecl[] = {
		{ 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },		// Position
		{ 0, 16, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },		// Texture Coordinate
		{ 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },		// Billboard Offset
		{ 0, 32, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },		// Color
		D3DDECL_END()
	};

	m_nVertexDeclIndex = g_BsKernel.LoadVertexDeclaration(ObjBillboardVertexDecl);

	CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialIndex);

#ifdef _XBOX
	m_hObjectTexture = pMaterial->GetParameterByName("g_ObjSampler");
	m_hLightColor = pMaterial->GetParameterByName("LIGHTAMBIENT");
#else
	m_hObjectTexture = pMaterial->GetParameterByName("ObjTex");
	m_hLightColor = pMaterial->GetParameterByName("lightAmbient");
#endif

	return S_OK;
}

void CBsObjectBillboardMgr::ReleaseDeviceData(void)
{
	SAFE_RELEASE_VD(m_nVertexDeclIndex);
	CBsBillboardMgr::ReleaseDeviceData();

	for(int i=0;i<m_nCurrentPool;++i) {
		if(m_arrayObjectBillboardPool[i]) {
			delete m_arrayObjectBillboardPool[i];
			m_arrayObjectBillboardPool[i] = NULL;
		}
	}
	memset(m_arrayObjectBillboardPool, 0, sizeof(OBJECT_BILLBOARD_DATA*)*MAX_NUM_OBJECT);

	m_nCurrentPool = 0;
	m_hObjectTexture = NULL;
	m_hLightColor = NULL;
}

void CBsObjectBillboardMgr::Clear(void)
{

}

void CBsObjectBillboardMgr::AddObjectBillboard(int nBillBoardIndex, D3DXVECTOR3 vWorldPos, int iSkinIndex, int nAniIndex, float fAlpha, float fAniFrameRatio)
{
	int nBufferIndex = CBsObject::GetProcessBufferIndex();
	int nCount = 0;
	nCount = m_arrayObjectBillboardPool[nBillBoardIndex]->nRenderBillboardCount[nBufferIndex];
#ifdef _XBOX
	float fOffset = m_arrayObjectBillboardPool[nBillBoardIndex]->pAnimationOffset[nAniIndex].fOffset;
	m_arrayObjectBillboardPool[nBillBoardIndex]->pVertices[nBufferIndex][nCount * 4].WPos = 
        m_arrayObjectBillboardPool[nBillBoardIndex]->pVertices[nBufferIndex][nCount * 4 + 1].WPos = 
		m_arrayObjectBillboardPool[nBillBoardIndex]->pVertices[nBufferIndex][nCount * 4 + 2].WPos = 
		m_arrayObjectBillboardPool[nBillBoardIndex]->pVertices[nBufferIndex][nCount * 4 + 3].WPos = 
			D3DXVECTOR4(vWorldPos.x, vWorldPos.y, vWorldPos.z, fOffset + fAniFrameRatio);
	float fSize = (float)m_arrayObjectBillboardPool[nBillBoardIndex]->nBillboardSize;

	m_arrayObjectBillboardPool[nBillBoardIndex]->pVertices[nBufferIndex][nCount * 4].Tex = D3DXVECTOR2(0.0f, 0.0f);
	m_arrayObjectBillboardPool[nBillBoardIndex]->pVertices[nBufferIndex][nCount * 4 + 1].Tex = D3DXVECTOR2(1.0f, 0.0f);
	m_arrayObjectBillboardPool[nBillBoardIndex]->pVertices[nBufferIndex][nCount * 4 + 2].Tex = D3DXVECTOR2(1.0f, 1.0f);
	m_arrayObjectBillboardPool[nBillBoardIndex]->pVertices[nBufferIndex][nCount * 4 + 3].Tex = D3DXVECTOR2(0.0f, 1.0f);
	m_arrayObjectBillboardPool[nBillBoardIndex]->pVertices[nBufferIndex][nCount * 4].LPos = D3DXVECTOR2(-fSize/2, fSize);
	m_arrayObjectBillboardPool[nBillBoardIndex]->pVertices[nBufferIndex][nCount * 4 + 1].LPos = D3DXVECTOR2(fSize/2, fSize);
	m_arrayObjectBillboardPool[nBillBoardIndex]->pVertices[nBufferIndex][nCount * 4 + 2].LPos = D3DXVECTOR2(fSize/2, 0.0f);
	m_arrayObjectBillboardPool[nBillBoardIndex]->pVertices[nBufferIndex][nCount * 4 + 3].LPos = D3DXVECTOR2(-fSize/2, 0.0f);
#else
	float fOffset = m_arrayObjectBillboardPool[nBillBoardIndex]->pAnimationOffset[nAniIndex].fOffset;
	m_arrayObjectBillboardPool[nBillBoardIndex]->pVertices[nBufferIndex][nCount * 4].WPos = 
		m_arrayObjectBillboardPool[nBillBoardIndex]->pVertices[nBufferIndex][nCount * 4+1].WPos = 
		m_arrayObjectBillboardPool[nBillBoardIndex]->pVertices[nBufferIndex][nCount * 4+2].WPos = 
		m_arrayObjectBillboardPool[nBillBoardIndex]->pVertices[nBufferIndex][nCount * 4+3].WPos = 
		m_arrayObjectBillboardPool[nBillBoardIndex]->pVertices[nBufferIndex][nCount * 4+4].WPos =
		m_arrayObjectBillboardPool[nBillBoardIndex]->pVertices[nBufferIndex][nCount * 4+5].WPos = 
			D3DXVECTOR4(vWorldPos.x, vWorldPos.y, vWorldPos.z, fOffset + fAniFrameRatio);

	m_arrayObjectBillboardPool[nBillBoardIndex]->pVertices[nBufferIndex][nCount * 4].Color = 
		m_arrayObjectBillboardPool[nBillBoardIndex]->pVertices[nBufferIndex][nCount * 4 + 1].Color = 
		m_arrayObjectBillboardPool[nBillBoardIndex]->pVertices[nBufferIndex][nCount * 4 + 2].Color = 
		m_arrayObjectBillboardPool[nBillBoardIndex]->pVertices[nBufferIndex][nCount * 4 + 3].Color = 
		m_arrayObjectBillboardPool[nBillBoardIndex]->pVertices[nBufferIndex][nCount * 4 + 4].Color = 
		m_arrayObjectBillboardPool[nBillBoardIndex]->pVertices[nBufferIndex][nCount * 4 + 5].Color = 
		D3DCOLOR_XRGB(255, 255, 255);

	float fSize = (float)m_arrayObjectBillboardPool[nBillBoardIndex]->nBillboardSize;

	m_arrayObjectBillboardPool[nBillBoardIndex]->pVertices[nBufferIndex][nCount * 4].Tex = D3DXVECTOR2(0.0f, 0.0f);
	m_arrayObjectBillboardPool[nBillBoardIndex]->pVertices[nBufferIndex][nCount * 4+1].Tex = D3DXVECTOR2(1.0f, 0.0f);
	m_arrayObjectBillboardPool[nBillBoardIndex]->pVertices[nBufferIndex][nCount * 4+2].Tex = D3DXVECTOR2(0.0f, 1.0f);
	m_arrayObjectBillboardPool[nBillBoardIndex]->pVertices[nBufferIndex][nCount * 4+3].Tex = D3DXVECTOR2(1.0f, 0.0f);
	m_arrayObjectBillboardPool[nBillBoardIndex]->pVertices[nBufferIndex][nCount * 4+4].Tex = D3DXVECTOR2(1.0f, 1.0f);
	m_arrayObjectBillboardPool[nBillBoardIndex]->pVertices[nBufferIndex][nCount * 4+5].Tex = D3DXVECTOR2(0.0f, 1.0f);
	m_arrayObjectBillboardPool[nBillBoardIndex]->pVertices[nBufferIndex][nCount * 4].LPos = D3DXVECTOR2(-fSize/2, fSize);
	m_arrayObjectBillboardPool[nBillBoardIndex]->pVertices[nBufferIndex][nCount * 4+1].LPos = D3DXVECTOR2(fSize/2, fSize);
	m_arrayObjectBillboardPool[nBillBoardIndex]->pVertices[nBufferIndex][nCount * 4+2].LPos = D3DXVECTOR2(-fSize/2, 0.0f);
	m_arrayObjectBillboardPool[nBillBoardIndex]->pVertices[nBufferIndex][nCount * 4+3].LPos = D3DXVECTOR2(fSize/2, fSize);
	m_arrayObjectBillboardPool[nBillBoardIndex]->pVertices[nBufferIndex][nCount * 4+4].LPos = D3DXVECTOR2(fSize/2, 0.0f);
	m_arrayObjectBillboardPool[nBillBoardIndex]->pVertices[nBufferIndex][nCount * 4+5].LPos = D3DXVECTOR2(-fSize/2, 0.0f);
#endif
	m_arrayObjectBillboardPool[nBillBoardIndex]->nRenderBillboardCount[nBufferIndex]++;
	return ;
}


int g_iDebugBufferInd;
OBJECT_BILLBOARD_DATA*	g_pDebugBillData;

void CBsObjectBillboardMgr::Render(C3DDevice* pDevice)
{
	CBsBillboardMgr::Render(pDevice);

	D3DXVECTOR4 vCommon;
	if( CBsBillboardMgr::m_nMaterialIndex == -1 )
		return;
	CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(CBsBillboardMgr::m_nMaterialIndex);
	vCommon = D3DXVECTOR4(0.0f, 0.958318f, 0.0f, 1.0f);
	g_BsKernel.SetVertexDeclaration(m_nVertexDeclIndex);
	pMaterial->BeginMaterial(1, 0);
	pMaterial->SetVector(CBsBillboardMgr::m_hCommon, &vCommon);
	pMaterial->SetVector(CBsBillboardMgr::m_hFogFactor, g_BsKernel.GetParamFogFactor());
	pMaterial->SetVector(CBsBillboardMgr::m_hFogColor, g_BsKernel.GetParamFogColor());
	if (g_BsKernel.GetWorldLightMap() != -1) {
		int iLightMapIndex = g_BsKernel.GetWorldLightMap();
		pMaterial->SetTexture(CBsBillboardMgr::m_hLightTexture, (LPDIRECT3DBASETEXTURE9)g_BsKernel.GetTexturePtr(iLightMapIndex));
	}
	else {
		int iLightMapIndex = g_BsKernel.GetBlankTexture();
		pMaterial->SetTexture(CBsBillboardMgr::m_hLightTexture, (LPDIRECT3DBASETEXTURE9)g_BsKernel.GetTexturePtr(iLightMapIndex));
	}
	pMaterial->SetFloat(CBsBillboardMgr::m_hRCPXSize, g_BsKernel.GetRcpWorldXSize());
	pMaterial->SetFloat(CBsBillboardMgr::m_hRCPZSize, g_BsKernel.GetRcpWorldZSize());
	pMaterial->SetMatrix(CBsBillboardMgr::m_hInvV, g_BsKernel.GetParamInvViewMatrix());
	pMaterial->SetMatrix(CBsBillboardMgr::m_hVP, g_BsKernel.GetParamViewProjectionMatrix());

	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	g_BsKernel.GetD3DDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
	pDevice->SetRenderState(D3DRS_ALPHAREF, 0x01);

#ifdef _XBOX
	pDevice->SetSamplerState(0, D3DSAMP_MAGFILTERZ, D3DTEXF_LINEAR);
	pDevice->SetSamplerState(0, D3DSAMP_MINFILTERZ, D3DTEXF_LINEAR);
#endif

	D3DXVECTOR4 L;
	L = -(*g_BsKernel.GetLightDirection());
	D3DXVECTOR4 N = -D3DXVECTOR4(g_BsKernel.GetParamInvViewMatrix()->_31, g_BsKernel.GetParamInvViewMatrix()->_32, 
		g_BsKernel.GetParamInvViewMatrix()->_33, 0.0f);
	float Diff = max(0.0f, D3DXVec4Dot(&N, &L));

	pMaterial->BeginPass(0);
	int iBufferInd = CBsObject::GetRenderBufferIndex();
	for (int i = 0; i < m_nCurrentPool; i++) {
		if (m_arrayObjectBillboardPool[i]->nRenderBillboardCount[iBufferInd]) {
			OBJ_BILLBOARD_VERTEX* pVerts;

			D3DXVECTOR4 vecAmbient;
			vecAmbient.x = m_arrayObjectBillboardPool[i]->pMaterialAmb[0].x * g_BsKernel.GetLightAmbient()->x;
			vecAmbient.y = m_arrayObjectBillboardPool[i]->pMaterialAmb[0].y * g_BsKernel.GetLightAmbient()->y;
			vecAmbient.z = m_arrayObjectBillboardPool[i]->pMaterialAmb[0].z * g_BsKernel.GetLightAmbient()->z;
			vecAmbient.w = 0.f;
			D3DXVECTOR4 vecDiffuse;
			vecDiffuse.x = m_arrayObjectBillboardPool[i]->pMaterialDiff[0].x * g_BsKernel.GetLightDiffuse()->x * Diff;
			vecDiffuse.y = m_arrayObjectBillboardPool[i]->pMaterialDiff[0].y * g_BsKernel.GetLightDiffuse()->y * Diff;
			vecDiffuse.z = m_arrayObjectBillboardPool[i]->pMaterialDiff[0].z * g_BsKernel.GetLightDiffuse()->z * Diff;
			vecDiffuse.w = 0.f;

			D3DXVECTOR4 vecRes = vecAmbient + vecDiffuse;
			vecRes.w = 1.f;

			m_arrayObjectBillboardPool[i]->pVB->Lock(0, 0, (void**)&pVerts, 0);

// memcpy하다가 다운되는 경우 있다!
g_iDebugBufferInd = iBufferInd;
g_pDebugBillData = m_arrayObjectBillboardPool[i];

			memcpy(pVerts, m_arrayObjectBillboardPool[i]->pVertices[iBufferInd], m_arrayObjectBillboardPool[i]->nRenderBillboardCount[iBufferInd] * sizeof(OBJ_BILLBOARD_VERTEX) * NUM_VERTEX_PER_BILLBOARD);
			m_arrayObjectBillboardPool[i]->pVB->Unlock();

#ifdef _XBOX
			pMaterial->SetTexture(m_hObjectTexture, (LPDIRECT3DARRAYTEXTURE9)g_BsKernel.GetTexturePtr(m_arrayObjectBillboardPool[i]->nTextureIndex));
#endif
			pMaterial->SetVector(m_hLightColor, &vecRes);
			pDevice->SetStreamSource(0, m_arrayObjectBillboardPool[i]->pVB, 0, sizeof(OBJ_BILLBOARD_VERTEX));
			pMaterial->CommitChanges();
#ifdef _XBOX
			pDevice->DrawPrimitiveVB(D3DPT_QUADLIST, 0, m_arrayObjectBillboardPool[i]->nRenderBillboardCount[iBufferInd]);
#else
			pDevice->DrawPrimitiveVB(D3DPT_TRIANGLELIST, 0, m_arrayObjectBillboardPool[i]->nRenderBillboardCount[iBufferInd] * 2);
#endif
			m_arrayObjectBillboardPool[i]->nRenderBillboardCount[iBufferInd] = 0;
		}
	}
	pMaterial->EndPass();

	pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	pDevice->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	pDevice->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	pDevice->SetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	pDevice->SetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_ALPHAREF, 0x7f);

	pMaterial->EndMaterial();
}