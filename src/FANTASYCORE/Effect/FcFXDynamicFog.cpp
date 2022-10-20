#include "StdAfx.h"
#include ".\fcfxdynamicfog.h"
#include "BsMaterial.h"
#include "FcWorld.h"
#include "FcUtil.h"
#include "FcGlobal.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

/******************************************************************************************
								Class Name : CFcFXFog
******************************************************************************************/
#ifdef _XBOX
#define VERTEXCOUNT_PER_QUAD		4
#else
#define VERTEXCOUNT_PER_QUAD		6
#endif
float g_fTest = 5.0f;
CFcFXFog::CFcFXFog()
{
	m_pVB = NULL;

	m_nVertexDeclIndex = -1;
	m_nMaterialIndex = -1;
	m_nTextureIndex = -1;

	m_vecAreaSize = D3DXVECTOR2(0.f, 0.f);
	m_vecStartPos = D3DXVECTOR2(0.f, 0.f);
	m_fFogSpd = 10.0f;

}

CFcFXFog::~CFcFXFog()
{
	m_hTexture = NULL;
	m_hInverseView = NULL;
	m_hViewProjection = NULL;
	m_hParticleInfo = NULL;
	m_hFogColor = NULL;

	m_fStartTime = 0.f;
	m_fElapsedTime = 0.f;
	m_ParticleInfo = D3DXVECTOR2(0.f, 0.f);
}

void CFcFXFog::Initialize(D3DXVECTOR2* pAreaSize, D3DXVECTOR2* pStartPos, char* pszTextureName)
{
	m_vecAreaSize = *pAreaSize;
	m_vecStartPos = *pStartPos;
	strcpy(m_szTextureName, pszTextureName);

	SAFE_RELEASE_TEXTURE(m_nTextureIndex);
	g_BsKernel.chdir("Fx");
	m_nTextureIndex = g_BsKernel.GetInstance().LoadTexture(m_szTextureName);
	g_BsKernel.chdir("..");

	m_vecFogColor = D3DXVECTOR4( 0.6f, 0.7f, 0.8f, 0.f);
	m_fFogSpd = 10.0f;
	CFcFXBase::Initialize();
}

void CFcFXFog::InitDeviceData()
{
	char szFullName[_MAX_PATH];
	strcpy(szFullName, g_BsKernel.GetInstance().GetShaderDirectory());
	strcat(szFullName, "EnvironmentFog.fx");
	m_nMaterialIndex = g_BsKernel.GetInstance().LoadMaterial(szFullName, FALSE);
	int i = 0;
	CBsMaterial* pMaterial = g_BsKernel.GetInstance().GetMaterialPtr(m_nMaterialIndex);

	// by jeremy
	BsAssert( m_nTextureIndex != -1 );
//	g_BsKernel.chdir("Fx");
//	m_nTextureIndex = g_BsKernel.GetInstance().LoadTexture(m_szTextureName);
//	g_BsKernel.chdir("..");
#ifdef _XBOX
	m_hInverseView			= pMaterial->GetParameterByName("ViewInverse");
	m_hViewProjection		= pMaterial->GetParameterByName("ViewProjection");
	m_hTexture				= pMaterial->GetParameterByName("FogSampler");
	m_hParticleInfo			= pMaterial->GetParameterByName("PARTICLEINFO");
	m_hFogColor				= pMaterial->GetParameterByName("FOGCOLOR");
#else
	m_hInverseView			= pMaterial->GetParameterByName("viewInverse");
	m_hViewProjection		= pMaterial->GetParameterByName("viewProj");
	m_hTexture				= pMaterial->GetParameterByName("FogTexture");
	m_hParticleInfo			= pMaterial->GetParameterByName("ParticleInfo");
	m_hFogColor				= pMaterial->GetParameterByName("FogColor");
#endif

	D3DVERTEXELEMENT9 decl[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		{ 0, 20, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
		D3DDECL_END()
	};
	m_nVertexDeclIndex = g_BsKernel.LoadVertexDeclaration(decl);

	g_BsKernel.CreateVertexBuffer(MAX_FOG_BILLBOARD * VERTEXCOUNT_PER_QUAD * sizeof(FOGVERTEX),
		0, 0, D3DPOOL_MANAGED,	&m_pVB);
	D3DXVECTOR3* pPosPool = new D3DXVECTOR3[MAX_FOG_BILLBOARD];
	for (int i = 0; i < MAX_FOG_BILLBOARD; i++) {
		pPosPool[i].x = m_vecStartPos.x + m_vecAreaSize.x * RandomNumberInRange(0.f, 1.f);
		pPosPool[i].z = m_vecStartPos.y + m_vecAreaSize.y * RandomNumberInRange(0.f, 1.f);
		pPosPool[i].y = g_BsKernel.GetHeight(pPosPool[i].x, pPosPool[i].z) + RandomNumberInRange(-100.f, 500.f);
	}
	//-------------------------------------------------------------------------
	FOGVERTEX* pVerts;
	const float fHalfSize = DYNAMIC_FOG_SIZE*0.5f;
	m_pVB->Lock(0, 0, (void**)&pVerts, 0);
	for (i = 0; i < MAX_FOG_BILLBOARD; i++) {
		float fHalfSize = DYNAMIC_FOG_SIZE*0.5f + RandomNumberInRange(0.f, 600.f);
#ifdef _XBOX
		pVerts[i*VERTEXCOUNT_PER_QUAD].vecWorldPos = pPosPool[i];
		pVerts[i*VERTEXCOUNT_PER_QUAD].vecTexCoord = D3DXVECTOR2(0.f, 0.f);
		pVerts[i*VERTEXCOUNT_PER_QUAD].vecOffset = D3DXVECTOR2(-fHalfSize, fHalfSize);

		pVerts[i*VERTEXCOUNT_PER_QUAD+1].vecWorldPos = pPosPool[i];
		pVerts[i*VERTEXCOUNT_PER_QUAD+1].vecTexCoord = D3DXVECTOR2(1.f, 0.f);
		pVerts[i*VERTEXCOUNT_PER_QUAD+1].vecOffset = D3DXVECTOR2(fHalfSize, fHalfSize);

		pVerts[i*VERTEXCOUNT_PER_QUAD+2].vecWorldPos = pPosPool[i];
		pVerts[i*VERTEXCOUNT_PER_QUAD+2].vecTexCoord = D3DXVECTOR2(1.f, 1.f);
		pVerts[i*VERTEXCOUNT_PER_QUAD+2].vecOffset = D3DXVECTOR2(fHalfSize, -fHalfSize);

		pVerts[i*VERTEXCOUNT_PER_QUAD+3].vecWorldPos = pPosPool[i];
		pVerts[i*VERTEXCOUNT_PER_QUAD+3].vecTexCoord = D3DXVECTOR2(0.f, 1.f);
		pVerts[i*VERTEXCOUNT_PER_QUAD+3].vecOffset = D3DXVECTOR2(-fHalfSize, -fHalfSize);
#else
		pVerts[i*VERTEXCOUNT_PER_QUAD].vecWorldPos = pPosPool[i];
		pVerts[i*VERTEXCOUNT_PER_QUAD].vecTexCoord = D3DXVECTOR2(0.f, 0.f);
		pVerts[i*VERTEXCOUNT_PER_QUAD].vecOffset = D3DXVECTOR2(-fHalfSize, fHalfSize);

		pVerts[i*VERTEXCOUNT_PER_QUAD+1].vecWorldPos = pPosPool[i];
		pVerts[i*VERTEXCOUNT_PER_QUAD+1].vecTexCoord = D3DXVECTOR2(1.f, 0.f);
		pVerts[i*VERTEXCOUNT_PER_QUAD+1].vecOffset = D3DXVECTOR2(fHalfSize, fHalfSize);

		pVerts[i*VERTEXCOUNT_PER_QUAD+2].vecWorldPos = pPosPool[i];
		pVerts[i*VERTEXCOUNT_PER_QUAD+2].vecTexCoord = D3DXVECTOR2(0.f, 1.f);
		pVerts[i*VERTEXCOUNT_PER_QUAD+2].vecOffset = D3DXVECTOR2(-fHalfSize, -fHalfSize);

		pVerts[i*VERTEXCOUNT_PER_QUAD+3].vecWorldPos = pPosPool[i];
		pVerts[i*VERTEXCOUNT_PER_QUAD+3].vecTexCoord = D3DXVECTOR2(1.f, 0.f);
		pVerts[i*VERTEXCOUNT_PER_QUAD+3].vecOffset = D3DXVECTOR2(fHalfSize, fHalfSize);

		pVerts[i*VERTEXCOUNT_PER_QUAD+4].vecWorldPos = pPosPool[i];
		pVerts[i*VERTEXCOUNT_PER_QUAD+4].vecTexCoord = D3DXVECTOR2(1.f, 1.f);
		pVerts[i*VERTEXCOUNT_PER_QUAD+4].vecOffset = D3DXVECTOR2(fHalfSize, -fHalfSize);

		pVerts[i*VERTEXCOUNT_PER_QUAD+5].vecWorldPos = pPosPool[i];
		pVerts[i*VERTEXCOUNT_PER_QUAD+5].vecTexCoord = D3DXVECTOR2(0.f, 1.f);
		pVerts[i*VERTEXCOUNT_PER_QUAD+5].vecOffset = D3DXVECTOR2(-fHalfSize, -fHalfSize);
#endif
	}
	m_pVB->Unlock();

	SAFE_DELETEA(pPosPool);
}

void CFcFXFog::ReleaseDeviceData()
{
	SAFE_RELEASE_MATERIAL(m_nMaterialIndex);
	SAFE_RELEASE_VD(m_nVertexDeclIndex);
	SAFE_RELEASE_TEXTURE(m_nTextureIndex);

	SAFE_RELEASE(m_pVB);
}

void CFcFXFog::Process()
{
	const D3DXVECTOR2 start = D3DXVECTOR2(-1000.f, 0.f);
	const D3DXVECTOR2 end = D3DXVECTOR2(0.f, 0.14f);
	const float			fRcpFadeInInterval = 1.f/m_fFogSpd;			// 10초동안 Fade In

	if(m_state == PLAY) {
		// Fade In 조절
		m_fElapsedTime = GetProcessTime() - m_fStartTime;
		float fParameter = m_fElapsedTime * fRcpFadeInInterval;
		fParameter = BsMax( 0.f, fParameter);
		fParameter = BsMin( 1.f, fParameter);
		D3DXVec2Lerp(&m_ParticleInfo, &start, &end, fParameter);

		m_vecFogColor.w = m_ParticleInfo.y;
	}
}


void CFcFXFog::PeRender()
{
	CFcFXBase::PreRender();
}


void CFcFXFog::Render(C3DDevice *pDevice)
{
	if(m_state != PLAY)
		return;
	CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialIndex);

	pMaterial->BeginMaterial(0, 0);
	pMaterial->BeginPass(0);

	pMaterial->SetMatrix(m_hInverseView, g_BsKernel.GetParamInvViewMatrix());
	pMaterial->SetMatrix(m_hViewProjection, g_BsKernel.GetParamViewProjectionMatrix());
	static float fFogTime = 0.1f;
	float fRotateFactor = m_fElapsedTime*fFogTime;
	D3DXVECTOR4 vecParticleInfo = D3DXVECTOR4( g_BsKernel.GetRenderTime()*fFogTime, m_ParticleInfo.x, sinf(fRotateFactor), cosf(fRotateFactor));
	pMaterial->SetVector(m_hParticleInfo, &vecParticleInfo);
	pMaterial->SetVector(m_hFogColor, &m_vecFogColor);
	pMaterial->SetTexture(m_hTexture, m_nTextureIndex);

	g_BsKernel.SetVertexDeclaration(m_nVertexDeclIndex);

	pDevice->SetStreamSource(0, m_pVB, sizeof(FOGVERTEX));
	pMaterial->CommitChanges();

#ifdef _XBOX
	pDevice->DrawPrimitiveVB(D3DPT_QUADLIST, 0, MAX_FOG_BILLBOARD );
#else
	pDevice->DrawPrimitiveVB(D3DPT_TRIANGLELIST, 0, MAX_FOG_BILLBOARD * 2 );
#endif
	pMaterial->EndPass();
	pMaterial->EndMaterial();
}

int CFcFXFog::ProcessMessage(int nCode,DWORD dwParam1, DWORD dwParam2,DWORD dwParam3)
{
	switch(nCode) {
		case FX_INIT_OBJECT:
			Initialize((D3DXVECTOR2*)dwParam1, (D3DXVECTOR2*)dwParam2, (char*)dwParam3);
			return 1;
		case FX_PLAY_OBJECT:
			m_fStartTime = GetProcessTime();
			m_state = PLAY;
			if(dwParam1) m_fFogSpd = (float)dwParam1;
			return 1;
		case FX_STOP_OBJECT:
			m_state = READY;
			return 1;
		case FX_UPDATE_OBJECT:
			BsAssert( 0 && "Invalid FX Command!!" );
			return 0;
	}
	return  CFcFXBase::ProcessMessage( nCode, dwParam1, dwParam2, dwParam3);
}