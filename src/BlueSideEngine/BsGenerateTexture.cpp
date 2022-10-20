#include "stdafx.h"
#include "BsGenerateTexture.h"
#include "BsKernel.h"
#include "BsMaterial.h"


CBsGenerateTexture::CBsGenerateTexture()
{
	m_hHandle = NULL;
	m_nCameraIndex = -1;

	m_bEnable = true;
}

CBsGenerateTexture::~CBsGenerateTexture()
{	
	Clear();
}

void CBsGenerateTexture::Create(int nTextureWidth, int nTextureHeight,
								float fStartX, float fStartY,
								float fWidth, float fHeight, bool bAlpha)
{
	Clear();

	D3DFORMAT nFormat = bAlpha ? D3DFMT_A8R8G8B8 : D3DFMT_X8R8G8B8;
	int nRTTextureID = g_BsKernel.CreateTexture(nTextureWidth,
		nTextureHeight,
		D3DUSAGE_RENDERTARGET,
		nFormat);

	m_nCameraIndex = g_BsKernel.CreateCameraObject(nRTTextureID, fStartX, fStartY, fWidth, fHeight, true);
}

void CBsGenerateTexture::Clear()
{
	if(m_nCameraIndex != -1){
		g_BsKernel.DeleteObject(m_nCameraIndex);
		m_nCameraIndex = -1;
	}
}

int CBsGenerateTexture::GetTextureID()
{
	CBsCamera* pCamera = (CBsCamera*)(g_BsKernel.GetEngineObjectPtr(m_nCameraIndex));
	return pCamera->GetRenderTargetTexture();
}

void CBsGenerateTexture::ReInitialize()
{	
	Clear();
}

CBsGenerateWorldLightMapTexture::CBsGenerateWorldLightMapTexture(int nBaseTexture) : CBsGenerateTexture()
{
	m_nLightMap = nBaseTexture;
	m_nMaterialIndex = -1;
	m_nVertexDeclIndex = -1;

	m_hInputBaseTexture = NULL;
}

CBsGenerateWorldLightMapTexture::~CBsGenerateWorldLightMapTexture()
{
	Clear();
}

void CBsGenerateWorldLightMapTexture::Create(int nTextureWidth, int nTextureHeight, float fStartX, float fStartY, float fWidth, float fHeight, bool bAlpha)
{
	CBsGenerateTexture::Create(nTextureWidth, nTextureHeight, fStartX, fStartY, fWidth, fHeight, bAlpha);

	D3DVERTEXELEMENT9 decl[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 8, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};
	m_nVertexDeclIndex = g_BsKernel.LoadVertexDeclaration(decl);

	char fullName[_MAX_PATH];
	strcpy(fullName, g_BsKernel.GetShaderDirectory());
	strcat(fullName, "scene_default.fx");

	BsAssert( m_nMaterialIndex < 0 && "Duplicated Index!!");
	m_nMaterialIndex = g_BsKernel.LoadMaterial(fullName, FALSE);
	CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialIndex);

#ifdef _XBOX
	m_hInputBaseTexture = pMaterial->GetParameterByName( "SceneSampler");
#else
	m_hInputBaseTexture = pMaterial->GetParameterByName( "SceneMap");
#endif
}

void CBsGenerateWorldLightMapTexture::ReInitialize()
{
	Clear();
}

void CBsGenerateWorldLightMapTexture::Clear()
{
	CBsGenerateTexture::Clear();

	SAFE_RELEASE_MATERIAL(m_nMaterialIndex);

	SAFE_RELEASE_VD(m_nVertexDeclIndex);
}

void CBsGenerateWorldLightMapTexture::InitRenderRTT()
{
}

void CBsGenerateWorldLightMapTexture::Render(C3DDevice* pDevice)
{
#ifdef _XBOX
	pDevice->ClearBuffer(0x00000000);
	CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialIndex);

	m_ScreenVertex[0].vecPos = D3DXVECTOR2(-1.f, 1.f);
	m_ScreenVertex[0].vecUV  = D3DXVECTOR2( 0.f, 0.f);
	m_ScreenVertex[1].vecPos = D3DXVECTOR2( 1.f, 1.f);
	m_ScreenVertex[1].vecUV  = D3DXVECTOR2( 1.f, 0.f);
	m_ScreenVertex[2].vecPos = D3DXVECTOR2( 1.f,-1.f);
	m_ScreenVertex[2].vecUV  = D3DXVECTOR2( 1.f, 1.f);
	m_ScreenVertex[3].vecPos = D3DXVECTOR2(-1.f,-1.f);
	m_ScreenVertex[3].vecUV  = D3DXVECTOR2( 0.f, 1.f);

	pMaterial->BeginMaterial(0, 0);
	pMaterial->BeginPass(0);

	g_BsKernel.SetVertexDeclaration(m_nVertexDeclIndex);

	pMaterial->SetTexture(m_hInputBaseTexture, m_nLightMap);
	pMaterial->CommitChanges();

	SCREEN_VERTEX* pVertices;

	// Base LightMap Render!!
	pDevice->BeginVertices(D3DPT_QUADLIST, 4, sizeof(SCREEN_VERTEX), (void**)&pVertices);
	memcpy(pVertices, m_ScreenVertex, sizeof(SCREEN_VERTEX)*4);
	pDevice->EndVertices();

	int nCamCount = g_BsKernel.GetCameraCount();

	float fRcpWorldXSize = g_BsKernel.GetRcpWorldXSize();
	float fRcpWorldZSize = g_BsKernel.GetRcpWorldZSize();

	for(int i=0;i<nCamCount;++i) {
		CBsCamera* pCamera = g_BsKernel.GetCamera(g_BsKernel.GetCameraHandle(i));
		std::vector<int>& ObjectList = pCamera->GetDrawObjectList();
		UINT uiObjectCount = ObjectList.size();
		CBsObject* pObject = NULL;
		pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		for(UINT j=0;j<uiObjectCount;++j) {
			pObject = g_BsKernel.GetEngineObjectPtr(ObjectList[j]);
			if(pObject->GetShadowCastType() != BS_SHADOW_IMPOSTER)
				continue;
			D3DXVECTOR2 vecPos = D3DXVECTOR2(pObject->GetObjectMatrix()->_41*fRcpWorldXSize, pObject->GetObjectMatrix()->_43*fRcpWorldZSize);
			vecPos.x = vecPos.x*2.f - 1.f;
			vecPos.y = vecPos.y*2.f - 1.f;
			float sizeX = 1000.f*fRcpWorldXSize;
			float sizeZ = 1000.f*fRcpWorldZSize;

			m_ScreenVertex[0].vecPos = D3DXVECTOR2(-sizeX, sizeZ) + vecPos;
			m_ScreenVertex[1].vecPos = D3DXVECTOR2( sizeX, sizeZ) + vecPos;
			m_ScreenVertex[2].vecPos = D3DXVECTOR2( sizeX,-sizeZ) + vecPos;
			m_ScreenVertex[3].vecPos = D3DXVECTOR2(-sizeX,-sizeZ) + vecPos;

			// Test!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			static int nTexture = 4;

			pMaterial->SetTexture(m_hInputBaseTexture, nTexture);
			pMaterial->CommitChanges();

			pDevice->BeginVertices(D3DPT_QUADLIST, 4, sizeof(SCREEN_VERTEX), (void**)&pVertices);
			memcpy(pVertices, m_ScreenVertex, sizeof(SCREEN_VERTEX)*4);
			pDevice->EndVertices();
		}
		pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	}

	pMaterial->EndPass();
	pMaterial->EndMaterial();
#endif
}