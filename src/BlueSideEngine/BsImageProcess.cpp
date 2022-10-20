#include "stdafx.h"
#include "BsKernel.h"
#include "BsImageProcess.h"
#include "BsMaterial.h"
#include "BsUIBase.h"
#include "BsShadowMgr.h"


const float c_fDofoutfar	= 1000.f;
const float c_fDofoutnear = 500.f;


//**********************************************************************
//	class CBsRenderTargetChain
//**********************************************************************
CBsRenderTargetChain::CBsRenderTargetChain()
{
	m_nNext = 0;
	for(int i=0; i<2; ++i) {
		m_nRTTexture[i] = -1;
#ifndef _XBOX 
		m_pRTSurface[i] = NULL;
#endif
	}
}


CBsRenderTargetChain::~CBsRenderTargetChain()
{
	Clear();
}

void CBsRenderTargetChain::Create(int nTextureWidth, int nTextureHeight, D3DFORMAT format)
{
	Clear();
	m_nRTTextureWidth = nTextureWidth;
	m_nRTTextureHeight = nTextureHeight;
	m_TextureFormat = format;
	for(int i=0; i<2; ++i) {
		m_nRTTexture[i] = g_BsKernel.CreateTexture(m_nRTTextureWidth, m_nRTTextureHeight, D3DUSAGE_RENDERTARGET, format);
#ifndef _XBOX
		LPDIRECT3DTEXTURE9 pTexture = (LPDIRECT3DTEXTURE9)(g_BsKernel.GetTexturePtr(m_nRTTexture[i]));
		pTexture->GetSurfaceLevel(0, &m_pRTSurface[i]);
#endif
	}
}

void CBsRenderTargetChain::Clear()
{
	for(int i=0; i<2; ++i) {
#ifndef _XBOX
		SAFE_RELEASE(m_pRTSurface[i]);
#endif
		SAFE_RELEASE_TEXTURE(m_nRTTexture[i]);
	}
}

void CBsRenderTargetChain::Reload()
{
	Clear();
	Create(m_nRTTextureWidth, m_nRTTextureHeight, m_TextureFormat);
}

//**********************************************************************
//	class CBsImageProcessFilter
//**********************************************************************

CBsImageProcessFilter::CBsImageProcessFilter()
{
	m_FilterType = FILTER_NONE;
	m_nMaterialIndex = -1;

	m_hInputTextureHandle = NULL;
	m_hDestTextureHandle = NULL;
	m_hTextureSizeHandle = NULL;
	m_hOriginIntensity = NULL;
	m_hBlurIntensity = NULL;
	m_hGlowIntensity = NULL;


	m_nVertexDeclIdx = -1;
}

CBsImageProcessFilter::~CBsImageProcessFilter()
{
	Clear();
}

void CBsImageProcessFilter::Create(int nTextureWidth, int nTextureHeight, eFILTER_TYPE filter )
{
	char* pszMaterialFileName=NULL;
	switch(filter) {
		case FILTER_DOWNFILTER4:
			pszMaterialFileName = "scene_downfilter4.fx";
			break;
		case FILTER_BRIGHTPASS:
			pszMaterialFileName = "scene_brightpass.fx";
			break;
		case FILTER_BLOOMH:
			pszMaterialFileName = "scene_bloomH.fx";
			break;
		case FILTER_BLOOMV:
			pszMaterialFileName = "scene_bloomV.fx";
			break;
		case FILTER_UPFILTER4:
			pszMaterialFileName = "scene_upfilter4.fx";
			break;
		case FILTER_COMBINE:
			pszMaterialFileName = "scene_combine.fx";
			break;
		case FILTER_DOF:
			pszMaterialFileName = "scene_DOF.fx";
			break;
		case FILTER_CUSTOMBLUR1:
			pszMaterialFileName = "scene_CustomBlur1.fx";
			break;
		case FILTER_CUSTOMBLUR2:
			pszMaterialFileName = "scene_CustomBlur2.fx";
			break;
		default:
			BsAssert( 0 && "Invalid Filter!!");
			break;
	}
	char fullName[_MAX_PATH];
	strcpy(fullName, g_BsKernel.GetShaderDirectory());
	strcat(fullName, pszMaterialFileName);
	m_nMaterialIndex = g_BsKernel.LoadMaterial(fullName, FALSE);

	CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialIndex);

	switch(filter) {
		case FILTER_DOWNFILTER4:
		case FILTER_BRIGHTPASS:
		case FILTER_BLOOMH:
		case FILTER_BLOOMV:
		case FILTER_UPFILTER4:
			{
#ifdef _XBOX
				m_hInputTextureHandle = pMaterial->GetParameterByName( "SceneSampler");
				m_hTextureSizeHandle = pMaterial->GetParameterByName( "VIEWPORTPIXELSIZE");
				m_HighlightThreshold = pMaterial->GetParameterByName( "HighlightThreshold");
#else
				m_hInputTextureHandle = pMaterial->GetParameterByName( "SceneMap");
				m_hTextureSizeHandle = pMaterial->GetParameterByName( "ScreenSize");
				m_HighlightThreshold = pMaterial->GetParameterByName( "HighlightThreshold");
#endif
			}
			break;
		case FILTER_COMBINE:
			{
#ifdef _XBOX
				m_hInputTextureHandle = pMaterial->GetParameterByName( "FinalSampler");
				m_hDestTextureHandle = pMaterial->GetParameterByName( "BackBufferSampler");
				m_hTextureSizeHandle = pMaterial->GetParameterByName( "VIEWPORTPIXELSIZE");

				m_hOriginIntensity = pMaterial->GetParameterByName( "SceneIntensity");
				m_hBlurIntensity = pMaterial->GetParameterByName( "GlowIntensity" );
				m_hGlowIntensity = pMaterial->GetParameterByName( "HighlightIntensity");
				m_hAddColor = pMaterial->GetParameterByName( "addColor");
#else
				m_hInputTextureHandle = pMaterial->GetParameterByName( "FinalTexture");
				m_hDestTextureHandle = pMaterial->GetParameterByName( "BackBufferTexture");
				m_hTextureSizeHandle = pMaterial->GetParameterByName( "ScreenSize");

				m_hOriginIntensity = pMaterial->GetParameterByName( "SceneIntensity");
				m_hBlurIntensity = pMaterial->GetParameterByName( "GlowIntensity" );
				m_hGlowIntensity = pMaterial->GetParameterByName( "HighlightIntensity");
				m_hAddColor = pMaterial->GetParameterByName( "addColor");
#endif
			}
			break;
		case FILTER_DOF:
			{
#ifdef _XBOX
				m_hInputTextureHandle = pMaterial->GetParameterByName( "NearSampler");
				m_hBlurIntensity = pMaterial->GetParameterByName( "fTap" );
				m_hTextureSizeHandle = pMaterial->GetParameterByName( "VIEWPORTPIXELSIZE");
#else
				m_hInputTextureHandle = pMaterial->GetParameterByName( "NearMap");
				m_hBlurIntensity = pMaterial->GetParameterByName( "fTap" );
				m_hTextureSizeHandle = pMaterial->GetParameterByName( "ScreenSize");
#endif
			}
			break;
		case FILTER_CUSTOMBLUR1:
			{
#ifdef _XBOX
				m_hInputTextureHandle = pMaterial->GetParameterByName("BackSampler");
				m_hOffsetTexture = pMaterial->GetParameterByName("TextureSampler");
				m_hTime = pMaterial->GetParameterByName("TIME");
#else
				m_hInputTextureHandle = pMaterial->GetParameterByName("backTexture");
				m_hOffsetTexture = pMaterial->GetParameterByName("diffuseTexture");
				m_hTime = pMaterial->GetParameterByName("fTime");
#endif
			}
			break;
		case FILTER_CUSTOMBLUR2:
			{
#ifdef _XBOX
				m_hInputTextureHandle = pMaterial->GetParameterByName("BackSampler");
				m_hOffsetTexture = pMaterial->GetParameterByName("TextureSampler");
				m_hTime = pMaterial->GetParameterByName("TIME");
#else
				m_hInputTextureHandle = pMaterial->GetParameterByName("backTexture");
				m_hOffsetTexture = pMaterial->GetParameterByName("diffuseTexture");
				m_hTime = pMaterial->GetParameterByName("fTime");
#endif

			}
			break;
		default:
			BsAssert( 0 && "Invalid Filter!!");
			break;
	}

	D3DVERTEXELEMENT9 decl[] =
	{
		{ 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};

	m_nVertexDeclIdx = g_BsKernel.LoadVertexDeclaration(decl);
}

void CBsImageProcessFilter::Clear()
{
	SAFE_RELEASE_MATERIAL(m_nMaterialIndex);
	SAFE_RELEASE_VD(m_nVertexDeclIdx);
}

//**********************************************************************
//	class CBsImageProcess
//**********************************************************************

CBsImageProcess::CBsImageProcess()
{
	m_nBackBufferTexture = -1;
	m_nBackBufferTextureBeforeAlphaBlendPass = -1;
	m_nBackBufferTextureExtraFilter = -1;

#ifdef _XBOX
	EnableImageProcess(TRUE);
#else
	EnableImageProcess(TRUE);
#endif

	// m_fSceneIntensity와 m_fBlurIntensity의 합은 1이 되는게 좋다.
	SetSceneIntensity(0.6f);
	SetBlurIntensity(0.4f);

	SetGlowIntensity(0.15f);
#ifdef _SHOW_DEBUG_TEXTURE_
	m_bShowDebugTexture = FALSE;
	m_nDefaultMaterial = -1;
#endif

	m_nEnableDOF = 0;

	m_fDOFFocusOutFar = c_fDofoutfar; // 고정수치 실기무비 이외엔 변경안함
	m_fDOFFocusOutNear = c_fDofoutfar;


	m_fDOFFocus = 1500.f;			//  게임 카메라에서 설정 (실기무비 제외)
	m_fDOFFocusRangeFar = 1000.f;
	m_fDOFFocusRangeNear = 1000.f;


	SetHighlightThreshold(0.9f);

	m_v4AddColor = D3DXVECTOR4(0,0,0,0);

	m_nTextureIdCustomBlur = -1;
	m_nTextureIdCustomBlur2 = -1;

}

CBsImageProcess::~CBsImageProcess()
{
	Clear();
}

void CBsImageProcess::Create()
{
	AvailMemoryDifference temp(0, "CBsImageProcess::Create()");
	C3DDevice *pDevice;
	pDevice = g_BsKernel.GetDevice();
	// Test by jeremy
	int nBackBufferWidth = g_BsKernel.GetDevice()->GetBackBufferWidth();
	int nBackBufferHeight = g_BsKernel.GetDevice()->GetBackBufferHeight();
#ifdef _XBOX    
	m_nBackBufferTexture = g_BsKernel.CreateTexture(nBackBufferWidth, nBackBufferHeight, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8);
	g_BsKernel.GetDevice()->GetD3DDevice()->CreateRenderTarget( nBackBufferWidth/4, nBackBufferHeight/4, D3DFMT_A8R8G8B8,
		D3DMULTISAMPLE_NONE, 0, FALSE, &m_pRTSurface1_4, NULL );
#else
	m_nBackBufferTexture = g_BsKernel.CreateTexture(nBackBufferWidth, nBackBufferHeight, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8);
	LPDIRECT3DTEXTURE9 pTexture = (LPDIRECT3DTEXTURE9)g_BsKernel.GetTexturePtr(m_nBackBufferTexture);
	pTexture->GetSurfaceLevel(0, &m_pBackBufferSurface);
#endif
	m_nBackBufferTextureBeforeAlphaBlendPass = g_BsKernel.CreateTexture(nBackBufferWidth, nBackBufferHeight, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8);

#ifndef _USAGE_TOOL_
	m_nBackBufferTextureExtraFilter = g_BsKernel.CreateTexture(nBackBufferWidth, nBackBufferHeight, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8);
#endif //ifndef _USAGE_TOOL_
	
	CBsImageProcessFilter* pFilter = new CBsImageProcessFilter;
	pFilter->Create(nBackBufferWidth/4, nBackBufferHeight/4, FILTER_DOWNFILTER4);
	m_FilterList.push_back(pFilter);

	pFilter = new CBsImageProcessFilter;
	pFilter->Create(nBackBufferWidth/4, nBackBufferHeight/4, FILTER_BLOOMH);
	m_FilterList.push_back(pFilter);

	pFilter = new CBsImageProcessFilter;
	pFilter->Create(nBackBufferWidth/4, nBackBufferHeight/4, FILTER_BLOOMV);
	m_FilterList.push_back(pFilter);

	pFilter = new CBsImageProcessFilter;
	pFilter->Create(nBackBufferWidth/4, nBackBufferHeight/4, FILTER_COMBINE);
	m_FilterList.push_back(pFilter);

	pFilter = new CBsImageProcessFilter;
	pFilter->Create(nBackBufferWidth/4, nBackBufferHeight/4, FILTER_DOF);
	m_FilterList.push_back(pFilter);

#ifndef _USAGE_TOOL_
	pFilter = new CBsImageProcessFilter;
	pFilter->Create(nBackBufferWidth/4, nBackBufferHeight/4, FILTER_CUSTOMBLUR1);
	m_FilterList.push_back(pFilter);

	pFilter = new CBsImageProcessFilter;
	pFilter->Create(nBackBufferWidth/4, nBackBufferHeight/4, FILTER_CUSTOMBLUR2);
	m_FilterList.push_back(pFilter);
#endif //ifndef _USAGE_TOOL_


	m_RTChain1_4.Create(nBackBufferWidth/4, nBackBufferHeight/4, D3DFMT_A8R8G8B8);

#ifndef _USAGE_TOOL_
	g_BsKernel.chdir("data");
	g_BsKernel.chdir("Map");
	m_nTextureIdCustomBlur = g_BsKernel.LoadTexture("waves.dds");
	m_nTextureIdCustomBlur2 = g_BsKernel.LoadTexture("tyWave.dds");
	g_BsKernel.chdir("..");
	g_BsKernel.chdir("..");
#endif //ifndef _USAGE_TOOL_

#ifdef _SHOW_DEBUG_TEXTURE_
	char fullName[_MAX_PATH];
	strcpy(fullName, g_BsKernel.GetShaderDirectory());
	strcat(fullName, "scene_default.fx");
	m_nDefaultMaterial = g_BsKernel.LoadMaterial(fullName, FALSE);
#endif
}

void CBsImageProcess::Reload()
{
	Clear();
	Create();
}

void CBsImageProcess::ScreenCaptureFinal(C3DDevice* pDevice)
{
#ifdef _XBOX
	pDevice->Resolve(D3DRESOLVE_RENDERTARGET0, NULL, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(m_nBackBufferTexture), NULL, 0, 0, NULL, 1.0f, 0, NULL );
#else
	if(!m_pBackBufferSurface)
		return;
	pDevice->StretchRect(pDevice->GetBackBuffer(), NULL, m_pBackBufferSurface, NULL, D3DTEXF_NONE );
#endif
}

void CBsImageProcess::ScreenCaptureBeforeAlphaBlendPass(C3DDevice* pDevice)
{
#ifdef _XBOX
	pDevice->Resolve(D3DRESOLVE_RENDERTARGET0, NULL, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(m_nBackBufferTextureBeforeAlphaBlendPass), NULL, 0, 0, NULL, 1.0f, 0, NULL );
#else
	LPDIRECT3DTEXTURE9 pTexture = (LPDIRECT3DTEXTURE9)g_BsKernel.GetTexturePtr(m_nBackBufferTextureBeforeAlphaBlendPass);

	if(pTexture == NULL)
		return;

	LPDIRECT3DSURFACE9 pSurface = NULL;
	pTexture->GetSurfaceLevel(0, &pSurface);
	pDevice->StretchRect(pDevice->GetBackBuffer(), NULL, pSurface, NULL, D3DTEXF_NONE);
	SAFE_RELEASE(pSurface);
#endif
}


void CBsImageProcess::ScreenCaptureExtraFilter(C3DDevice* pDevice)
{
#ifndef _USAGE_TOOL_
#ifdef _XBOX
	pDevice->Resolve(D3DRESOLVE_RENDERTARGET0, NULL, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(m_nBackBufferTextureExtraFilter), NULL, 0, 0, NULL, 1.0f, 0, NULL );
#else
	LPDIRECT3DTEXTURE9 pTexture = (LPDIRECT3DTEXTURE9)g_BsKernel.GetTexturePtr(m_nBackBufferTextureExtraFilter);
	LPDIRECT3DSURFACE9 pSurface = NULL;
	pTexture->GetSurfaceLevel(0, &pSurface);
	pDevice->StretchRect(pDevice->GetBackBuffer(), NULL, pSurface, NULL, D3DTEXF_NONE);
	SAFE_RELEASE(pSurface);
#endif
#endif
}

void CBsImageProcess::Clear()
{
#ifndef _XBOX
	SAFE_RELEASE(m_pBackBufferSurface);
#endif
	SAFE_RELEASE_TEXTURE(m_nBackBufferTexture);
	SAFE_RELEASE_TEXTURE(m_nBackBufferTextureBeforeAlphaBlendPass);
	SAFE_RELEASE_TEXTURE(m_nBackBufferTextureExtraFilter);
	SAFE_RELEASE_TEXTURE(m_nTextureIdCustomBlur);
	SAFE_RELEASE_TEXTURE(m_nTextureIdCustomBlur2);

	for(unsigned int i=0;i<m_FilterList.size();++i) {
		if(m_FilterList[i]) {
			delete m_FilterList[i];
			m_FilterList[i] = NULL;
		}
	}
	m_FilterList.clear();
}

void CBsImageProcess::Render(C3DDevice* pDevice)
{
	if(!IsEnableImageProcess()) {
		return;
	}

#ifndef _XBOX
	if(m_pBackBufferSurface == NULL)
		return;
#endif

	if( m_nBackBufferTexture == -1)
		return;

	LPDIRECT3DTEXTURE9 pBridgeTexture = NULL;
	int nBackBufferWidth = pDevice->GetBackBufferWidth();
	int nBackBufferHeight = pDevice->GetBackBufferHeight();

	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, false);
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, false);
	pDevice->SetRenderState(D3DRS_ZENABLE, false);
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);

	CBsUIElement::UIVERTEX uiv[4];
	uiv[0].vecPos = D3DXVECTOR3(-1.f, 1.f, 0.f);
	uiv[0].vecUV = D3DXVECTOR2(0.f, 0.f );
	uiv[1].vecPos = D3DXVECTOR3( 1.f, 1.f, 0.f);
	uiv[1].vecUV = D3DXVECTOR2(1.f, 0.f );
	uiv[2].vecPos = D3DXVECTOR3( 1.f, -1.f, 0.f);
	uiv[2].vecUV = D3DXVECTOR2(1.f, 1.f );
	uiv[3].vecPos = D3DXVECTOR3( -1.f, -1.f, 0.f);
	uiv[3].vecUV = D3DXVECTOR2(0.f, 1.f );

	static short s_QuadIndices[] =
	{
		0,1,3,
			1,2,3,
	};
	pDevice->BeginScene();


#ifdef _XBOX
	static BOOL bGPR = FALSE;
	if(bGPR)
		pDevice->GetD3DDevice()->SetShaderGPRAllocation(0, 32, GPU_GPRS - 32);

	static BOOL bHALFPIXELOFFSET =FALSE;
	if(bHALFPIXELOFFSET)
		pDevice->SetRenderState( D3DRS_HALFPIXELOFFSET,  TRUE ); 
#endif

	// Set Depth/Stencil NULL
	pDevice->SetDepthStencilSurface( NULL );

	{	// DownSampling & Bright Pass
		CBsImageProcessFilter* pFilter = m_FilterList[0];
		CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(pFilter->m_nMaterialIndex);
#ifdef _XBOX
		pDevice->SetRenderTarget(0, m_pRTSurface1_4);
#else
		pDevice->SetRenderTarget(0, m_RTChain1_4.GetNextRTSurface());
#endif
		pMaterial->SetTexture(pFilter->m_hInputTextureHandle, m_nBackBufferTexture);

		pMaterial->SetVector(pFilter->m_hTextureSizeHandle, &D3DXVECTOR4(float(nBackBufferWidth), float(nBackBufferHeight), 0.f, 0.f));
		pMaterial->SetFloat(pFilter->m_HighlightThreshold, m_fHighlightThreshold);

		pMaterial->BeginMaterial(0, 0);
		pMaterial->BeginPass(0);

		pMaterial->CommitChanges();

		g_BsKernel.SetVertexDeclaration(pFilter->m_nVertexDeclIdx);

		pDevice->DrawIndexedPrimitiveUP( D3DPT_TRIANGLELIST, 0, 4, 2,
			s_QuadIndices, D3DFMT_INDEX16, uiv, sizeof(CBsUIElement::UIVERTEX) );

		#ifdef _XBOX
		pDevice->Resolve(D3DRESOLVE_RENDERTARGET0, NULL, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(m_RTChain1_4.GetNextRTTextureIndex()), NULL, 0, 0, NULL, 1.0f, 0, NULL );
		#endif
		m_RTChain1_4.Flip();
		pBridgeTexture = (LPDIRECT3DTEXTURE9)CBsKernel::GetInstance().GetTexturePtr(m_RTChain1_4.GetPrevRTTextureIndex());

		pMaterial->EndPass();
		pMaterial->EndMaterial();
	}

	{	// Blur & Bloom Horizon		

		CBsImageProcessFilter* pFilter = m_FilterList[1];
		CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(pFilter->m_nMaterialIndex);
#ifdef _XBOX
		pDevice->SetRenderTarget(0, m_pRTSurface1_4);
#else
		pDevice->SetRenderTarget(0, m_RTChain1_4.GetNextRTSurface());
#endif

		pMaterial->SetTexture(pFilter->m_hInputTextureHandle, pBridgeTexture);
		pMaterial->SetVector(pFilter->m_hTextureSizeHandle, &D3DXVECTOR4(float(nBackBufferWidth)/4.f, float(nBackBufferHeight)/4.f, 0.f, 0.f));

		pMaterial->BeginMaterial(0, 0);
		pMaterial->BeginPass(0);

		pMaterial->CommitChanges();

		g_BsKernel.SetVertexDeclaration(pFilter->m_nVertexDeclIdx);

		pDevice->DrawIndexedPrimitiveUP( D3DPT_TRIANGLELIST, 0, 4, 2,
			s_QuadIndices, D3DFMT_INDEX16, uiv, sizeof(CBsUIElement::UIVERTEX) );

#ifdef _XBOX
		pDevice->Resolve(D3DRESOLVE_RENDERTARGET0, NULL, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(m_RTChain1_4.GetNextRTTextureIndex()), NULL, 0, 0, NULL, 1.0f, 0, NULL );
#endif
		m_RTChain1_4.Flip();
		pBridgeTexture = (LPDIRECT3DTEXTURE9)CBsKernel::GetInstance().GetTexturePtr(m_RTChain1_4.GetPrevRTTextureIndex());

		pMaterial->EndPass();
		pMaterial->EndMaterial();		
	}

	{	// Blur & Bloom Vertical
		
		CBsImageProcessFilter* pFilter = m_FilterList[2];
		CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(pFilter->m_nMaterialIndex);
#ifdef _XBOX
		pDevice->SetRenderTarget(0, m_pRTSurface1_4);
#else
		pDevice->SetRenderTarget(0, m_RTChain1_4.GetNextRTSurface());
#endif

		pMaterial->SetTexture(pFilter->m_hInputTextureHandle, pBridgeTexture);
		pMaterial->SetVector(pFilter->m_hTextureSizeHandle, &D3DXVECTOR4(float(nBackBufferWidth)/4.f, float(nBackBufferHeight)/4.f, 0.f, 0.f));

		pMaterial->BeginMaterial(0, 0);
		pMaterial->BeginPass(0);

		pMaterial->CommitChanges();

		g_BsKernel.SetVertexDeclaration(pFilter->m_nVertexDeclIdx);

		pDevice->DrawIndexedPrimitiveUP( D3DPT_TRIANGLELIST, 0, 4, 2,
			s_QuadIndices, D3DFMT_INDEX16, uiv, sizeof(CBsUIElement::UIVERTEX) );
#ifdef _XBOX
		pDevice->Resolve(D3DRESOLVE_RENDERTARGET0, NULL, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(m_RTChain1_4.GetNextRTTextureIndex()), NULL, 0, 0, NULL, 1.0f, 0, NULL );
#endif
		m_RTChain1_4.Flip();
		pBridgeTexture = (LPDIRECT3DTEXTURE9)CBsKernel::GetInstance().GetTexturePtr(m_RTChain1_4.GetPrevRTTextureIndex());

		pMaterial->EndPass();
		pMaterial->EndMaterial();

	}

	// restore render target, depth/stencil surface
	{
        pDevice->SetRenderTarget( 0, pDevice->GetBackBuffer() );
		pDevice->SetDepthStencilSurface( pDevice->GetDepthStencilSurface() );
	}

	{	// Final Combine
		CBsImageProcessFilter* pFilter = m_FilterList[3];
		CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(pFilter->m_nMaterialIndex);

		pMaterial->SetTexture(pFilter->m_hInputTextureHandle, pBridgeTexture);
		pMaterial->SetTexture(pFilter->m_hDestTextureHandle, m_nBackBufferTexture);

		pMaterial->SetVector(pFilter->m_hTextureSizeHandle, &D3DXVECTOR4(float(nBackBufferWidth), float(nBackBufferHeight), 0.f, 0.f));

		pMaterial->SetFloat(pFilter->m_hOriginIntensity, m_fSceneIntensity);
		pMaterial->SetFloat(pFilter->m_hBlurIntensity, m_fBlurIntensity);
		pMaterial->SetFloat(pFilter->m_hGlowIntensity, m_fGlowIntensity);
		pMaterial->SetVector(pFilter->m_hAddColor, &m_v4AddColor);
		pMaterial->BeginMaterial(0, 0);

		pMaterial->BeginPass(0);
		pMaterial->CommitChanges();

		g_BsKernel.SetVertexDeclaration(pFilter->m_nVertexDeclIdx);
		pDevice->DrawIndexedPrimitiveUP( D3DPT_TRIANGLELIST, 0, 4, 2,
			s_QuadIndices, D3DFMT_INDEX16, uiv, sizeof(CBsUIElement::UIVERTEX) );

		pMaterial->EndPass();
		pMaterial->EndMaterial();
	}
		
#ifdef _XBOX
	if(m_nEnableDOF && g_BsKernel.GetCameraCount() ) {	
		// DOF
		ScreenCaptureExtraFilter(pDevice);		

		CBsCamera *pCamera = (CBsCamera*)g_BsKernel.GetCamera( g_BsKernel.GetCameraHandle(0) );
		const D3DXMATRIX* mProj =  pCamera->GetProjectionMatrix();
		D3DXVECTOR4	vFocus(0.0f, 0.0f, BsMax(1.0f, m_fDOFFocus ), 1.f );
		D3DXVECTOR4	vProj;
		CBsImageProcessFilter* pFilter = m_FilterList[4];
		CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(pFilter->m_nMaterialIndex);

		pMaterial->SetTexture(pFilter->m_hInputTextureHandle, m_nBackBufferTextureExtraFilter);

		pMaterial->BeginMaterial(0, 0);
		pMaterial->BeginPass(0);
		pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
		pDevice->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
		pDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
		pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
		pDevice->SetRenderState( D3DRS_ZWRITEENABLE,  FALSE );
		pDevice->SetRenderState( D3DRS_ZFUNC,  D3DCMP_GREATEREQUAL);
		
		pMaterial->SetVector(pFilter->m_hTextureSizeHandle, &D3DXVECTOR4(1.f/float(nBackBufferWidth), 1.f/float(nBackBufferHeight), 0.f, 0.f));

		g_BsKernel.SetVertexDeclaration(pFilter->m_nVertexDeclIdx);

		pMaterial->SetFloat(pFilter->m_hBlurIntensity, 0.5f);
		pDevice->SetRenderState( D3DRS_ZENABLE,  D3DZB_FALSE );
		pMaterial->CommitChanges();
		pDevice->DrawIndexedPrimitiveUP( D3DPT_TRIANGLELIST, 0, 4, 2,
			s_QuadIndices, D3DFMT_INDEX16, uiv, sizeof(CBsUIElement::UIVERTEX) );
		pDevice->SetRenderState( D3DRS_ZENABLE,  D3DZB_TRUE );

		int nTap = 4;
		float fDist;
		static float fBlur =  0.7f;
		static float fBlurNear =  1.2f;
		for(int i =1 ; i <= nTap ; i++) {
			vFocus.z = m_fDOFFocus + m_fDOFFocusRangeFar + m_fDOFFocusOutFar*(i)/nTap;
			D3DXVec4Transform(&vProj, &vFocus, mProj);
			fDist = vProj.z/vProj.w;
			fDist = 1.f - fDist;

			uiv[0].vecPos.z = fDist;
			uiv[1].vecPos.z = fDist;
			uiv[2].vecPos.z = fDist;
			uiv[3].vecPos.z = fDist;
			
			pMaterial->SetFloat(pFilter->m_hBlurIntensity, (float)i*fBlur);
			pMaterial->CommitChanges();
			pDevice->DrawIndexedPrimitiveUP( D3DPT_TRIANGLELIST, 0, 4, 2,
				s_QuadIndices, D3DFMT_INDEX16, uiv, sizeof(CBsUIElement::UIVERTEX) );
		}

		nTap =2;
		pDevice->SetRenderState( D3DRS_ZFUNC,  D3DCMP_LESSEQUAL );
		for( int i = 1 ; i <= nTap ; i++) {
			vFocus.z = m_fDOFFocus - m_fDOFFocusRangeNear- m_fDOFFocusOutNear*(i)/nTap;
			D3DXVec4Transform(&vProj, &vFocus, mProj);
			fDist = vProj.z/vProj.w;
			fDist = 1.f - fDist;

			uiv[0].vecPos.z = fDist;
			uiv[1].vecPos.z = fDist;
			uiv[2].vecPos.z = fDist;
			uiv[3].vecPos.z = fDist;

			pMaterial->SetFloat(pFilter->m_hBlurIntensity, (float)(i)*fBlurNear+0.5f);
			pMaterial->CommitChanges();
			pDevice->DrawIndexedPrimitiveUP( D3DPT_TRIANGLELIST, 0, 4, 2,
				s_QuadIndices, D3DFMT_INDEX16, uiv, sizeof(CBsUIElement::UIVERTEX) );
		}
		pDevice->SetRenderState( D3DRS_ZFUNC,  D3DCMP_GREATEREQUAL );

		pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,  FALSE );
		pMaterial->EndPass();
		pMaterial->EndMaterial();
	}
#endif

#ifndef _USAGE_TOOL_
	if(m_eExtraFilter == FILTER_CUSTOMBLUR1)
	{
		ScreenCaptureExtraFilter(pDevice);
		CBsImageProcessFilter* pFilter = m_FilterList[5];
		CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(pFilter->m_nMaterialIndex);
		pDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE );
		pDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );

		pMaterial->SetTexture(pFilter->m_hInputTextureHandle, m_nBackBufferTextureExtraFilter);
		pMaterial->SetTexture(pFilter->m_hOffsetTexture, m_nTextureIdCustomBlur );

		pMaterial->SetFloat(pFilter->m_hTime, float(g_BsKernel.GetRenderTick()) /20.f  );

		pMaterial->BeginMaterial(0, 0);
		pMaterial->BeginPass(0);

		pMaterial->CommitChanges();
		g_BsKernel.SetVertexDeclaration(pFilter->m_nVertexDeclIdx);
		pDevice->DrawIndexedPrimitiveUP( D3DPT_TRIANGLELIST, 0, 4, 2,
			s_QuadIndices, D3DFMT_INDEX16, uiv, sizeof(CBsUIElement::UIVERTEX) );

		pMaterial->EndPass() ;
		pMaterial->EndMaterial();
	}
	if(m_eExtraFilter == FILTER_CUSTOMBLUR2) {
		ScreenCaptureExtraFilter(pDevice);
		CBsImageProcessFilter* pFilter = m_FilterList[6];
		CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(pFilter->m_nMaterialIndex);
		pDevice->SetRenderState( D3DRS_ZENABLE,  D3DZB_FALSE );
		pDevice->SetRenderState( D3DRS_ZWRITEENABLE,  FALSE );

		pMaterial->SetTexture(pFilter->m_hInputTextureHandle, m_nBackBufferTextureExtraFilter);

		pMaterial->SetTexture(pFilter->m_hOffsetTexture, m_nTextureIdCustomBlur2 );
		pMaterial->SetFloat(pFilter->m_hTime, float(g_BsKernel.GetRenderTick()) /25.f  );

		pMaterial->BeginMaterial(0, 0);
		pMaterial->BeginPass(0);

		pMaterial->CommitChanges();
		g_BsKernel.SetVertexDeclaration(pFilter->m_nVertexDeclIdx);

		pDevice->DrawIndexedPrimitiveUP( D3DPT_TRIANGLELIST, 0, 4, 2,
			s_QuadIndices, D3DFMT_INDEX16, uiv, sizeof(CBsUIElement::UIVERTEX) );

		pMaterial->EndPass() ;
		pMaterial->EndMaterial();
	}
#endif

#ifdef _XBOX
	pDevice->SetRenderState( D3DRS_HALFPIXELOFFSET,  FALSE ); 
	if(bGPR)
		pDevice->GetD3DDevice()->SetShaderGPRAllocation(0, 0, 0);
#endif

	/////////////////////////////////////
#ifdef _SHOW_DEBUG_TEXTURE_
	if(m_bShowDebugTexture) {
		LPDIRECT3DTEXTURE9 pShadowTexture = g_BsKernel.GetShadowMgr()->GetShadowMap();

		CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(m_nDefaultMaterial);
		pMaterial->SetTexture((D3DXHANDLE)"SceneMap", pShadowTexture);
		pMaterial->BeginMaterial(0, 0);
		pMaterial->BeginPass(0);
		pMaterial->CommitChanges();

		uiv[0].vecPos = D3DXVECTOR3(0.f, 1.f, 0.f);		// Top-Left
		uiv[0].vecUV = D3DXVECTOR2(0.f, 0.f );
		uiv[1].vecPos = D3DXVECTOR3( 1.f, 1.f, 0.f);		// Top-Right
		uiv[1].vecUV = D3DXVECTOR2(1.f, 0.f );
		uiv[2].vecPos = D3DXVECTOR3( 1.f, 0.f, 0.f);		// Bottom-Right
		uiv[2].vecUV = D3DXVECTOR2(1.f, 1.f );
		uiv[3].vecPos = D3DXVECTOR3( 0.f, 0.f, 0.f);		// Bottom-Left
		uiv[3].vecUV = D3DXVECTOR2(0.f, 1.f );

		pDevice->DrawIndexedPrimitiveUP( D3DPT_TRIANGLELIST, 0, 4, 2,
			s_QuadIndices, D3DFMT_INDEX16, uiv, sizeof(CBsUIElement::UIVERTEX) );

		pMaterial->EndPass();
		pMaterial->EndMaterial();
	}
#endif
	pDevice->EndScene();

	pDevice->SetRenderState(D3DRS_ZENABLE, true);
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, true);
}


// DOF func

void	CBsImageProcess::SetDOFFocusRangeFar(float fFocusRange)
{
	m_fDOFFocusRangeFar = fFocusRange;
}

void	CBsImageProcess::SetDOFFocusRangeNear(float fFocusRange)
{
	m_fDOFFocusRangeNear = fFocusRange;
}


void	CBsImageProcess::SetDOFFocusOutFarDef()
{
	m_fDOFFocusOutFar = c_fDofoutfar;
}

void	CBsImageProcess::SetDOFFocusOutNearDef()
{
	m_fDOFFocusOutNear = c_fDofoutnear;
}