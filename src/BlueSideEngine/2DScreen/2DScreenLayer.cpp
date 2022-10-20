#include "stdafx.h"
#include "2DScreenLayer.h"

//$ global
IDirect3DTexture9	*g_pOriginalBackBuffer = NULL;
IDirect3DSurface9	*g_pOriginalBackBufferSurface = NULL;
static	COLORTEXTUREVERTEX	g_BackBufferVertex[6];

static	LPDIRECT3DTEXTURE9	g_pBackBuffer = NULL; //$
static	LPDIRECT3DSURFACE9	g_pBackBufferSurface = NULL;

// static members
bool C2DScreenLayer::m_bBackBufferIsVirgin = true;
int C2DScreenLayer::m_nLayerTable = 0;
//int	C2DScreenLayer::m_nTextureInfo = 0;

int	C2DScreenLayer::m_nBackBufferCapture = 1;
int C2DScreenLayer::m_nScreenWidth=0;
int C2DScreenLayer::m_nScreenHeight=0;

CBsKernel*						C2DScreenLayer::m_pKernel = NULL;
//C2DScreenLayer::_TextureInfo**	C2DScreenLayer::m_ppTextureInfo = NULL;
C2DScreenLayer**				C2DScreenLayer::m_ppLayerTable = NULL;

void	C2DScreenLayer :: NewLayer(int nId)
{
	C2DScreenLayer	**ppTemp;

	if (nId < m_nLayerTable)
	{
		assert(m_ppLayerTable[nId] == NULL && "Layer가 이미 존재 합니다.");   //$
		m_nId = nId;
	}
	else
	{

		ppTemp = (C2DScreenLayer**) malloc((nId + 1) * sizeof(C2DScreenLayer*));
		memset(ppTemp,0,(nId + 1) * sizeof(C2DScreenLayer*));
		memcpy(ppTemp,m_ppLayerTable,m_nLayerTable * sizeof(C2DScreenLayer*));

		if (m_ppLayerTable != NULL)
			free (m_ppLayerTable);

		m_ppLayerTable = ppTemp;

		m_nLayerTable = nId + 1;
		m_nId = nId;
	}

	m_ppLayerTable[m_nId] = this;

	memset(m_nParam,0,sizeof(int) * SCREEN_LAYER_PARAM_MAX);
	memset(m_fParam,0,sizeof(float) * SCREEN_LAYER_PARAM_MAX);
	memset(m_vParam,0,sizeof(D3DXVECTOR3) * SCREEN_LAYER_PARAM_MAX);

	m_pTexture = NULL;
	m_nTexId = -1;
	m_pSurface = NULL;

	m_ColorR = 255;
	m_ColorG = 255;
	m_ColorB = 255;
	m_ColorA = 255;

	m_bForceHide = false;
	m_bShow = true;
	m_nShowCount =	-1;
	m_nShowStep =	-1;
	m_bNextShow = true;
	m_nShowAlpha = 255;

	m_nBlendMode = BLEND_MODE_NORMAL;
	m_nSavedBlendMode = -1;
	m_bZEnabled = false;
}

void	C2DScreenLayer :: CreateBackBufferSurface()
{
#ifdef _XBOX
	if (g_pBackBuffer == NULL)
	{
		m_pKernel->GetDevice()->GetD3DDevice()->CreateTexture(BACKBUFFER_TEXTURE_SIZE, BACKBUFFER_TEXTURE_SIZE, 1, D3DUSAGE_RENDERTARGET, 
			D3DFMT_R5G6B5, D3DPOOL_DEFAULT, &g_pBackBuffer);

		if (g_pBackBuffer != NULL)
			g_pBackBuffer->GetSurfaceLevel(0, &g_pBackBufferSurface);

		g_pOriginalBackBuffer = new IDirect3DTexture9; //$	

		//g_pOriginalBackBuffer생성 후 바로 g_pOriginalBackBuffer->BlockUntilNotBusy()가 불려서 다운되는 경우가 있다
		// 그래서 g_pOriginalBackBuffer를 초기화하기 위해 XGSetTextreHeader()를 여기서 불러준다. yooty
		IDirect3DSurface8* backBufferSurface=(IDirect3DSurface8*)m_pKernel->GetDevice()->GetBackBuffer();
		assert(backBufferSurface);
		XGSetTextureHeader(m_nScreenWidth, m_nScreenHeight, 1, 0, D3DFMT_LIN_A8R8G8B8, 0, g_pOriginalBackBuffer,
			backBufferSurface->Data, m_nScreenWidth * 4);

		g_BackBufferVertex[0].Vertex=D3DXVECTOR3(1.0f, 1.0f, 0.0f);
		g_BackBufferVertex[1].Vertex=D3DXVECTOR3(0, 0, 0.0f);
		g_BackBufferVertex[2].Vertex=D3DXVECTOR3(0, 1.0f, 0.0f);
		g_BackBufferVertex[3].Vertex=D3DXVECTOR3(1.0f, 0, 0.0f);
		g_BackBufferVertex[4].Vertex=D3DXVECTOR3(0, 0, 0.0f);
		g_BackBufferVertex[5].Vertex=D3DXVECTOR3(1.0f, 1.0f, 0.0f);

		g_BackBufferVertex[0].dwColor= D3DCOLOR_RGBA(255,255,255,255);
		g_BackBufferVertex[1].dwColor= D3DCOLOR_RGBA(255,255,255,255);
		g_BackBufferVertex[2].dwColor= D3DCOLOR_RGBA(255,255,255,255);
		g_BackBufferVertex[3].dwColor= D3DCOLOR_RGBA(255,255,255,255);
		g_BackBufferVertex[4].dwColor= D3DCOLOR_RGBA(255,255,255,255);
		g_BackBufferVertex[5].dwColor= D3DCOLOR_RGBA(255,255,255,255);

		g_BackBufferVertex[0].fTextureU = (float)(m_nScreenWidth-1);
		g_BackBufferVertex[0].fTextureV = (float)(m_nScreenHeight-1);
		g_BackBufferVertex[1].fTextureU = 0.0f;
		g_BackBufferVertex[1].fTextureV = 0.0f;
		g_BackBufferVertex[2].fTextureU = 0.0f;
		g_BackBufferVertex[2].fTextureV = (float)(m_nScreenHeight-1);
		g_BackBufferVertex[3].fTextureU = (float)(m_nScreenWidth-1);
		g_BackBufferVertex[3].fTextureV = 0.0f;
		g_BackBufferVertex[4].fTextureU = 0.0f;
		g_BackBufferVertex[4].fTextureV = 0.0f;
		g_BackBufferVertex[5].fTextureU = (float)(m_nScreenWidth-1);
		g_BackBufferVertex[5].fTextureV = (float)(m_nScreenHeight-1);
	}
#endif
}

void	C2DScreenLayer :: PreprocessScreenLayers()
{
#ifdef _XBOX
	assert( g_pBackBuffer != NULL );
#endif

	int	nScan;
	for(nScan = 0;nScan < m_nLayerTable;nScan++)
	{
		if (m_ppLayerTable[nScan] != NULL)
			m_ppLayerTable[nScan]->Preprocess();
	}
}


void	C2DScreenLayer :: ProcessScreenLayers()
{
	int	nScan;

	for(nScan = 0;nScan < m_nLayerTable;nScan++)
	{
		if (m_ppLayerTable[nScan] != NULL)
			m_ppLayerTable[nScan]->Process();
	}

}

void	C2DScreenLayer :: PostprocessScreenLayers()
{
	int	nScan;

	for(nScan = 0;nScan < m_nLayerTable;nScan++)
	{
		if (m_ppLayerTable[nScan] != NULL)
			m_ppLayerTable[nScan]->Postprocess();
	}
}

void	C2DScreenLayer :: DrawScreenLayersBeforeFX()
{
	if (m_ppLayerTable[0] != NULL)
	{
		m_ppLayerTable[0]->SetLayerTexture(-1);	// texture reset
		m_ppLayerTable[0]->RTT();
		m_ppLayerTable[0]->SetLayerTexture(-1);
		m_ppLayerTable[0]->Draw();
	}
}

void	C2DScreenLayer :: DrawScreenLayersBeforeCapture()
{
	int	nScan;

	for(nScan = 1;nScan < m_nLayerTable;nScan++)
	{
		if (nScan == m_nBackBufferCapture)
		{
			C2DScreenLayer::GetBackBufferToTexture();
			break;
		}

		if (m_ppLayerTable[nScan] != NULL)
		{
			m_ppLayerTable[nScan]->SetLayerTexture(-1);	// texture reset
			m_ppLayerTable[nScan]->RTT();
			m_ppLayerTable[nScan]->SetLayerTexture(-1);
			m_ppLayerTable[nScan]->Draw();
		}
	}
}

void	C2DScreenLayer :: DrawScreenLayersAfterCapture()
{
	int	nScan;

	for(nScan = m_nBackBufferCapture;nScan < m_nLayerTable;nScan++)
	{
		if (m_ppLayerTable[nScan] != NULL)
		{
			m_ppLayerTable[nScan]->SetLayerTexture(-1);	// texture reset
			m_ppLayerTable[nScan]->RTT();
			m_ppLayerTable[nScan]->SetLayerTexture(-1);
			m_ppLayerTable[nScan]->Draw();
		}
	}
}

C2DScreenLayer :: C2DScreenLayer(CBsKernel *pKernel, char *pTextureFileName, bool bAlpha, int nId)
{
	NewLayer(nId);
	m_pKernel = pKernel;

	m_nType = SL_TYPE_NORMAL;

	if (pTextureFileName != NULL)
	{
		if (pTextureFileName[0] != '\x0') {
			m_nTexId = LoadLayerTexture(pTextureFileName, bAlpha);
			if(m_nTexId<0) {
				//$ CriticalException();
			}
		}
	}
	

	m_nWidth = 0;
	m_nHeight = 0;
	GetScreenSize();
}

C2DScreenLayer :: C2DScreenLayer(CBsKernel *pKernel, char *pTextureFileName, int nId)
{
	NewLayer(nId);
	m_pKernel = pKernel;

	m_nType = SL_TYPE_NORMAL;

	if (pTextureFileName != NULL)
	{
		if (pTextureFileName[0] != '\x0') {
			m_nTexId = LoadLayerTexture(pTextureFileName);
			if(m_nTexId<0) {
				//$ CriticalException();
			}
		}
	}
	

	m_nWidth = 0;
	m_nHeight = 0;
	GetScreenSize();
}

C2DScreenLayer :: C2DScreenLayer(CBsKernel *pKernel, int nId )
{
	NewLayer(nId);
	m_pKernel = pKernel;

	m_nType = SL_TYPE_BACK_BUFFER;

	m_nTexId = -1;
	m_pSurface = NULL;

	m_nWidth = BACKBUFFER_TEXTURE_SIZE;
	m_nHeight = BACKBUFFER_TEXTURE_SIZE;
	GetScreenSize();
}

C2DScreenLayer :: C2DScreenLayer(CBsKernel *pKernel, int nWidth, int nHeight, int nId )
{
	NewLayer(nId);
	m_pKernel = pKernel;

	m_nType = SL_TYPE_RTT;

	// nWidth, nHeight가 2의 자승이 아니면 2의 자승으로 맞춰서 텍스쳐 로딩 yooty
	float n=(float)_logb((double)nWidth);	// 이 함수는 소수점 이하가 안나오는군

	int texWidth, texHeight;
	if(pow(2.f, n)<nWidth) {
		texWidth=(int)pow(2.f, (float)(n+1));
	}
	else {
		texWidth=nWidth;
	}
	n=(float)_logb((double)nHeight);
	if(pow(2.f, n)<nHeight) {
		texHeight=(int)pow(2.f, (float)(n+1));
	}
	else {
		texHeight=nHeight;
	}
	m_pKernel->GetDevice()->GetD3DDevice()->CreateTexture(texWidth, texHeight, 1, D3DUSAGE_RENDERTARGET, 
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pTexture, NULL);

	if (m_pTexture != NULL)
		m_pTexture->GetSurfaceLevel(0, &m_pSurface);

	m_nWidth = nWidth;
	m_nHeight = nHeight;
	GetScreenSize();
	
}

C2DScreenLayer :: ~C2DScreenLayer(void)
{
	int	nScan;
	int	nRefCnt;

	if (m_nType == SL_TYPE_NORMAL)
	{
		if (m_nTexId != -1)
			UnloadLayerTexture(m_nTexId);

		assert(m_pSurface == NULL); //$
	}
	else if (m_nType == SL_TYPE_RTT)
	{
		if (m_nTexId != -1)
			UnloadLayerTexture(m_nTexId);

		if (m_pTexture != NULL)
		{
			nRefCnt = m_pTexture->Release();
			//$ DebugString("Texture release: Ref count: %d\n", nRefCnt);
		}

		if (m_pSurface != NULL)
		{
			nRefCnt = m_pSurface->Release();
			//$ DebugString("Surface release: Ref count: %d\n", nRefCnt);
		}
	}

	m_ppLayerTable[m_nId] = NULL;

	for(nScan = 0;nScan < m_nLayerTable;nScan++)
		if (m_ppLayerTable[nScan] != NULL)
			break;

	if (nScan >= m_nLayerTable)
	{
		free (m_ppLayerTable);
		m_nLayerTable = 0;
		m_ppLayerTable= NULL;

//////////////////////////////////////////////////////////////////

#ifdef _XBOX

		if (g_pBackBuffer != NULL)
		{
			g_pBackBuffer->BlockUntilNotBusy();
			g_pBackBuffer->Release();

			g_pBackBufferSurface->BlockUntilNotBusy();
			g_pBackBufferSurface->Release();
		}

		g_pBackBuffer = NULL;
		g_pBackBufferSurface = NULL;

		g_pOriginalBackBuffer->BlockUntilNotBusy();

		delete g_pOriginalBackBuffer;
		g_pOriginalBackBuffer = NULL;

#endif
		m_bBackBufferIsVirgin = true;

//////////////////////////////////////////////////////////////////

	}
}

void C2DScreenLayer::GetScreenSize()
{
	m_nScreenWidth=m_pKernel->GetDevice()->GetBackBufferWidth();
	m_nScreenHeight=m_pKernel->GetDevice()->GetBackBufferHeight();
}

void	C2DScreenLayer :: Show(bool bOpt,int nCount)
{
	if (bOpt != m_bShow) {
		if (nCount== 0) {
			m_bShow = bOpt;
			if(m_bShow==true)	{ m_nShowAlpha=255; }
			else				{ m_nShowAlpha=0; }
		}
		else {
			m_bShow = true;
		}
		m_bNextShow = bOpt;
		m_nShowCount = 0;
		m_nShowStep = nCount;
	}
}

bool	C2DScreenLayer :: IsShowing(void)
{
	return(m_bShow);
}

void	C2DScreenLayer :: SetBackBufferCapture(int nLayerId)
{
	m_nBackBufferCapture = nLayerId;
}

void	C2DScreenLayer :: SaveBlendMode(void)
{
	m_nSavedBlendMode = m_nBlendMode;
}

void	C2DScreenLayer :: RestoreBlendMode(void)
{
	SetBlendMode(m_nSavedBlendMode);
}


void	C2DScreenLayer :: SetBlendMode(int nBlendMode)
{
	if (m_nBlendMode != nBlendMode)
	{
		switch (nBlendMode)
		{
		case BLEND_MODE_NORMAL:
			m_pKernel->GetDevice()->SetRenderState( D3DRS_BLENDOP,			D3DBLENDOP_ADD);
			m_pKernel->GetDevice()->SetRenderState( D3DRS_SRCBLEND,			D3DBLEND_SRCALPHA);
			m_pKernel->GetDevice()->SetRenderState( D3DRS_DESTBLEND,		D3DBLEND_INVSRCALPHA);
			break;

		case BLEND_MODE_ADD:
			m_pKernel->GetDevice()->SetRenderState( D3DRS_BLENDOP,			D3DBLENDOP_ADD);
			m_pKernel->GetDevice()->SetRenderState( D3DRS_SRCBLEND,			D3DBLEND_SRCALPHA);
			m_pKernel->GetDevice()->SetRenderState( D3DRS_DESTBLEND,		D3DBLEND_ONE);
			break;

		case BLEND_MODE_REVSUBTRACT:
			m_pKernel->GetDevice()->SetRenderState( D3DRS_BLENDOP,			D3DBLENDOP_REVSUBTRACT);
			m_pKernel->GetDevice()->SetRenderState( D3DRS_SRCBLEND,			D3DBLEND_SRCALPHA);
			m_pKernel->GetDevice()->SetRenderState( D3DRS_DESTBLEND,		D3DBLEND_ONE);
			break;
		}

		m_nBlendMode = nBlendMode;
	}
}

void	C2DScreenLayer :: RTT(void)
{
	/* //$
	D3DXMATRIX		matView;

	if (m_nType == SL_TYPE_RTT && m_bShow == true)
	{

		m_pKernel->GetDevice()->SetRenderTarget(m_pSurface, NULL);
		m_pKernel->GetDevice()->BeginScene();
		m_pKernel->GetDevice()->SaveState();

		m_pKernel->GetDevice()->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
		m_pKernel->GetDevice()->SetRenderState( D3DRS_ZENABLE,  FALSE );
		m_pKernel->GetDevice()->SetRenderState( D3DRS_ZWRITEENABLE,  FALSE );
		m_pKernel->GetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE,  TRUE );
		m_pKernel->GetDevice()->SetRenderState( D3DRS_ALPHATESTENABLE,  TRUE );
		m_pKernel->GetDevice()->SetRenderState( D3DRS_ALPHAFUNC,		D3DCMP_GREATER);
		m_pKernel->GetDevice()->SetRenderState( D3DRS_ALPHAREF,			0);
		m_pKernel->GetDevice()->SetRenderState( D3DRS_BLENDOP,			D3DBLENDOP_ADD);
		m_pKernel->GetDevice()->SetRenderState( D3DRS_SRCBLEND,			D3DBLEND_SRCALPHA);
		m_pKernel->GetDevice()->SetRenderState( D3DRS_DESTBLEND,		D3DBLEND_INVSRCALPHA);

		
		m_pKernel->GetDevice()->SetRenderState(D3DRS_LIGHTING,FALSE);
		m_pKernel->GetDevice()->SetRenderState(D3DRS_FOGENABLE,FALSE);

		m_nBlendMode = -1;
		SetBlendMode(BLEND_MODE_NORMAL);


		m_pKernel->GetDevice()->SetTextureStageState(0, D3DTSS_ADDRESSU ,D3DTADDRESS_CLAMP);
		m_pKernel->GetDevice()->SetTextureStageState(0, D3DTSS_ADDRESSV ,D3DTADDRESS_CLAMP);
		m_pKernel->GetDevice()->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
		m_pKernel->GetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		m_pKernel->GetDevice()->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
		m_pKernel->GetDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
		m_pKernel->GetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		m_pKernel->GetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

		
		m_pKernel->GetDevice()->SetTexture(1, NULL);
		m_pKernel->GetDevice()->SetTexture(2, NULL);
		m_pKernel->GetDevice()->SetTexture(3, NULL);

		m_pKernel->GetDevice()->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		m_pKernel->GetDevice()->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		m_pKernel->GetDevice()->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
		m_pKernel->GetDevice()->SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		m_pKernel->GetDevice()->SetTextureStageState(3, D3DTSS_COLOROP, D3DTOP_DISABLE);
		m_pKernel->GetDevice()->SetTextureStageState(3, D3DTSS_ALPHAOP, D3DTOP_DISABLE);


		D3DXMatrixIdentity(&matView);

		m_pKernel->SetTransform(STR_TRANSFORM_VIEW, &matView);
		m_pKernel->SetTransform(STR_TRANSFORM_WORLD, &matView);

		D3DXMatrixOrthoOffCenterLH(&matView, 0, (float)(m_nWidth - 1), (float)(m_nHeight - 1), 0, -1.0f, 1.0f);

		m_pKernel->SetTransform(STR_TRANSFORM_PROJECTION, &matView);
		m_pKernel->SetVertexShader(D3DFVF_COLORTEXTUREVERTEX);

		UserRTT();

		m_pKernel->GetDevice()->RestoreState();
		m_pKernel->GetDevice()->SetRenderTarget(m_pKernel->GetDevice()->GetBackBuffer(), m_pKernel->GetDevice()->GetDepthBuffer());
		m_pKernel->GetDevice()->EndScene();

	}
	*/
}

void	C2DScreenLayer :: GetBackBufferToTexture(void)
{
#ifdef _XBOX
	D3DXMATRIX				Mat;

	if (g_pBackBuffer != NULL)
	{
////////////////////////////////////////////////////////////////////////////////
		//$ m_pKernel->GetDevice()->SaveState();

		m_pKernel->GetDevice()->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
		m_pKernel->GetDevice()->SetRenderState( D3DRS_ZENABLE,  FALSE );
		m_pKernel->GetDevice()->SetRenderState( D3DRS_ZWRITEENABLE,  FALSE );
		m_pKernel->GetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE,  FALSE );
		m_pKernel->GetDevice()->SetRenderState( D3DRS_ALPHATESTENABLE,  FALSE );
		m_pKernel->GetDevice()->SetRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1 );
		m_pKernel->GetDevice()->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
		m_pKernel->GetDevice()->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		
		m_pKernel->GetDevice()->SetRenderState(D3DRS_LIGHTING,FALSE);
		m_pKernel->GetDevice()->SetRenderState(D3DRS_FOGENABLE,FALSE);

		m_pKernel->GetDevice()->SetTextureStageState(0, D3DTSS_ADDRESSU ,D3DTADDRESS_CLAMP);
		m_pKernel->GetDevice()->SetTextureStageState(0, D3DTSS_ADDRESSV ,D3DTADDRESS_CLAMP);

		D3DXMatrixIdentity(&Mat);

		m_pKernel->SetTransform(STR_TRANSFORM_VIEW, &Mat);
		m_pKernel->SetTransform(STR_TRANSFORM_WORLD, &Mat);

		D3DXMatrixOrthoOffCenterLH(&Mat, 0, 1.0f, 1.0f, 0, -1.0f, 1.0f);

		m_pKernel->SetTransform(STR_TRANSFORM_PROJECTION, &Mat);

		m_pKernel->SetVertexShader(D3DFVF_COLORTEXTUREVERTEX);

////////////////////////////////////////////////////////////////////////////////

		if (g_pOriginalBackBufferSurface == NULL)
		{
//			m_pKernel->GetDevice()->GetD3DDevice()->GetBackBuffer(0,D3DBACKBUFFER_TYPE_MONO,&g_pOriginalBackBufferSurface);
			g_pOriginalBackBufferSurface=(IDirect3DSurface8*)m_pKernel->GetDevice()->GetBackBuffer();
		}
		XGSetTextureHeader(m_nScreenWidth, m_nScreenHeight, 1, 0, D3DFMT_LIN_A8R8G8B8, 0, g_pOriginalBackBuffer,
			g_pOriginalBackBufferSurface->Data, m_nScreenWidth * 4);

////////////////////////////////////////////////////////////////////////////////
	
		m_pKernel->GetDevice()->SetRenderTarget(g_pBackBufferSurface, NULL);
		m_pKernel->GetDevice()->BeginScene();
		m_pKernel->SetTexture(0,g_pOriginalBackBuffer);
		//$ m_pKernel->DrawCustomMesh(2, g_BackBufferVertex, sizeof(COLORTEXTUREVERTEX));
		m_pKernel->GetDevice()->EndScene();


////////////////////////////////////////////////////////////////////////////////
		m_pKernel->GetDevice()->RestoreState();

		m_pKernel->GetDevice()->SetRenderTarget(m_pKernel->GetDevice()->GetBackBuffer(), m_pKernel->GetDevice()->GetDepthBuffer());

		m_bBackBufferIsVirgin = false;

	}
#endif
}

void	C2DScreenLayer :: Draw(void)
{
	/* //$
	D3DXMATRIX		matView;

	if (m_bShow == true	&& m_bForceHide == false)
	{
		m_pKernel->GetDevice()->BeginScene();

		m_pKernel->GetDevice()->SaveState();

		m_pKernel->SetVertexShader(D3DFVF_COLORTEXTUREVERTEX);

		m_pKernel->GetDevice()->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
		m_pKernel->GetDevice()->SetRenderState( D3DRS_ZENABLE,  FALSE );
		m_pKernel->GetDevice()->SetRenderState( D3DRS_ZWRITEENABLE,  FALSE );
		m_pKernel->GetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE,  TRUE );
		m_pKernel->GetDevice()->SetRenderState( D3DRS_ALPHATESTENABLE,  TRUE );
		
		m_pKernel->GetDevice()->SetRenderState(D3DRS_LIGHTING,FALSE);
		m_pKernel->GetDevice()->SetRenderState(D3DRS_FOGENABLE,FALSE);

		m_pKernel->GetDevice()->SetRenderState( D3DRS_BLENDOP,			D3DBLENDOP_ADD);
		m_pKernel->GetDevice()->SetRenderState( D3DRS_SRCBLEND,			D3DBLEND_SRCALPHA);
		m_pKernel->GetDevice()->SetRenderState( D3DRS_DESTBLEND,		D3DBLEND_INVSRCALPHA);
		m_pKernel->GetDevice()->SetRenderState( D3DRS_ALPHAFUNC,		D3DCMP_GREATER);
		m_pKernel->GetDevice()->SetRenderState( D3DRS_ALPHAREF,			0);

		m_pKernel->GetDevice()->SetTextureStageState(0, D3DTSS_ADDRESSU ,D3DTADDRESS_CLAMP);
		m_pKernel->GetDevice()->SetTextureStageState(0, D3DTSS_ADDRESSV ,D3DTADDRESS_CLAMP);
		m_pKernel->GetDevice()->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
		m_pKernel->GetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		m_pKernel->GetDevice()->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
		m_pKernel->GetDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
		m_pKernel->GetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		m_pKernel->GetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );


		m_pKernel->GetDevice()->SetTexture(1, NULL);
		m_pKernel->GetDevice()->SetTexture(2, NULL);
		m_pKernel->GetDevice()->SetTexture(3, NULL);

		m_pKernel->GetDevice()->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		m_pKernel->GetDevice()->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		m_pKernel->GetDevice()->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
		m_pKernel->GetDevice()->SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		m_pKernel->GetDevice()->SetTextureStageState(3, D3DTSS_COLOROP, D3DTOP_DISABLE);
		m_pKernel->GetDevice()->SetTextureStageState(3, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

		D3DXMatrixIdentity(&matView);

		m_pKernel->SetTransform(STR_TRANSFORM_VIEW, &matView);
		m_pKernel->SetTransform(STR_TRANSFORM_WORLD, &matView);

		D3DXMatrixOrthoOffCenterLH(&matView, 0, (float)(m_nScreenWidth-1), (float)(m_nScreenHeight-1), 0, -1.0f, 1.0f);
		m_pKernel->GetTransform(STR_TRANSFORM_PROJECTION, &m_matSavedProj);
		m_pKernel->SetTransform(STR_TRANSFORM_PROJECTION, &matView);

		UserDraw();

		m_pKernel->GetDevice()->RestoreState();

		m_pKernel->GetDevice()->EndScene();

		if (m_nShowCount >= 0 && m_nShowStep > 0)
		{
			m_nShowCount++;
			m_bShow = true;

			m_nShowAlpha = (m_nShowCount * 256) / m_nShowStep;

			if (m_bNextShow == false)
				m_nShowAlpha = 256 - m_nShowAlpha;
		}

		if (m_nShowCount >= m_nShowStep)
		{
			m_bShow = m_bNextShow;
			m_nShowCount = -1;
			m_nShowStep = -1;
		}

	}
	*/
}

void	C2DScreenLayer :: SetVParam(int nId, D3DXVECTOR3	*pParam)
{
	assert(nId >= 0 && nId < SCREEN_LAYER_PARAM_MAX);

	m_vParam[nId] = *pParam;
}


void	C2DScreenLayer :: SetFParam(int nId, float fValue)
{
	assert(nId >= 0 && nId < SCREEN_LAYER_PARAM_MAX);

	m_fParam[nId] = fValue;
}


void	C2DScreenLayer :: SetNParam(int nId, int iValue)
{
	assert(nId >= 0 && nId < SCREEN_LAYER_PARAM_MAX);

	m_nParam[nId] = iValue;
}

D3DXVECTOR3*	C2DScreenLayer :: GetVParam(int nId)
{
	return &(m_vParam[nId]);
}

float			C2DScreenLayer :: GetFParam(int nId)
{
	return	m_fParam[nId];
}

int				C2DScreenLayer :: GetNParam(int nId)
{
	return	m_nParam[nId];
}


void	C2DScreenLayer :: Preprocess(void)
{
}

void	C2DScreenLayer :: Postprocess(void)
{
}

void	C2DScreenLayer :: Process(void)
{
}

void	C2DScreenLayer :: UserRTT(void)
{
}
	
void	C2DScreenLayer :: UserDraw(void)
{
}

void	C2DScreenLayer :: FillUpScreen(DWORD dwColor, float fU1, float fV1, float fU2, float fV2)
{
	COLORTEXTUREVERTEX	Vertex[6];

	Vertex[0].Vertex=D3DXVECTOR3((float)(m_nScreenWidth-1), (float)(m_nScreenHeight-1), 0.0f);
	Vertex[1].Vertex=D3DXVECTOR3(0, 0, 0.0f);
	Vertex[2].Vertex=D3DXVECTOR3(0, (float)(m_nScreenHeight-1), 0.0f);
	Vertex[3].Vertex=D3DXVECTOR3((float)(m_nScreenWidth-1), 0, 0.0f);
	Vertex[4].Vertex=D3DXVECTOR3(0, 0, 0.0f);
	Vertex[5].Vertex=D3DXVECTOR3((float)(m_nScreenWidth-1), (float)(m_nScreenHeight-1), 0.0f);

	Vertex[0].dwColor= dwColor;
	Vertex[1].dwColor= dwColor;
	Vertex[2].dwColor= dwColor;
	Vertex[3].dwColor= dwColor;
	Vertex[4].dwColor= dwColor;
	Vertex[5].dwColor= dwColor;

	Vertex[0].fTextureU = fU2;
	Vertex[0].fTextureV = fV1;
	Vertex[1].fTextureU = fU1;
	Vertex[1].fTextureV = fV2;
	Vertex[2].fTextureU = fU1;
	Vertex[2].fTextureV = fV1;
	Vertex[3].fTextureU = fU2;
	Vertex[3].fTextureV = fV2;
	Vertex[4].fTextureU = fU1;
	Vertex[4].fTextureV = fV2;
	Vertex[5].fTextureU = fU2;
	Vertex[5].fTextureV = fV1;

	//$ m_pKernel->DrawCustomMesh(2, Vertex, sizeof(COLORTEXTUREVERTEX));
}



void	C2DScreenLayer :: FillUpScreen(float fX, float fY, float fZ, DWORD dwColor, float fU1, float fV1, float fU2, float fV2)
{
	COLORTEXTUREVERTEX	Vertex[6];

	Vertex[0].Vertex=D3DXVECTOR3(m_nScreenWidth-1+fX, m_nScreenHeight-1+fY, fZ);
	Vertex[1].Vertex=D3DXVECTOR3(fX, fY, fZ);
	Vertex[2].Vertex=D3DXVECTOR3(fX, m_nScreenHeight-1+fY, fZ);
	Vertex[3].Vertex=D3DXVECTOR3(m_nScreenWidth+fX-1, fY, fZ);
	Vertex[4].Vertex=D3DXVECTOR3(fX, fY, fZ);
	Vertex[5].Vertex=D3DXVECTOR3(m_nScreenWidth-1+fX, m_nScreenHeight-1+fY, fZ);

	Vertex[0].dwColor= dwColor;
	Vertex[1].dwColor= dwColor;
	Vertex[2].dwColor= dwColor;
	Vertex[3].dwColor= dwColor;
	Vertex[4].dwColor= dwColor;
	Vertex[5].dwColor= dwColor;

	Vertex[0].fTextureU = fU2;
	Vertex[0].fTextureV = fV1;
	Vertex[1].fTextureU = fU1;
	Vertex[1].fTextureV = fV2;
	Vertex[2].fTextureU = fU1;
	Vertex[2].fTextureV = fV1;
	Vertex[3].fTextureU = fU2;
	Vertex[3].fTextureV = fV2;
	Vertex[4].fTextureU = fU1;
	Vertex[4].fTextureV = fV2;
	Vertex[5].fTextureU = fU2;
	Vertex[5].fTextureV = fV1;

	//$ m_pKernel->DrawCustomMesh(2, Vertex, sizeof(COLORTEXTUREVERTEX));
}


void	C2DScreenLayer :: DrawUVMesh(D3DXVECTOR3 center, DWORD dwColor, float	fScaleX, float fScaleY, int nU1, int nV1, int nU2, int nV2, int mapSizeX, int mapSizeY)
{
	float				u1,v1,u2,v2;

	u1 = ((float)(nU1)) / ((float)(mapSizeX));
	v1 = ((float)(nV2)) / ((float)(mapSizeY));
	u2 = ((float)(nU2)) / ((float)(mapSizeX));
	v2 = ((float)(nV1)) / ((float)(mapSizeY));

	DrawUVMesh(center, dwColor, (int)((float)(nU2 - nU1 + 1) * fScaleX), (int)((float)(nV2 - nV1 + 1) * fScaleY), u1,v1,u2,v2);

}


void	C2DScreenLayer :: DrawUVMesh(D3DXVECTOR3 center, DWORD dwColor, float fRot, float	fScaleX, float fScaleY, int nU1, int nV1, int nU2, int nV2, int mapSizeX, int mapSizeY)
{
	float				u1,v1,u2,v2;

	u1 = ((float)(nU1)) / ((float)(mapSizeX));
	v1 = ((float)(nV2)) / ((float)(mapSizeY));
	u2 = ((float)(nU2)) / ((float)(mapSizeX));
	v2 = ((float)(nV1)) / ((float)(mapSizeY));

	DrawUVMesh(center, dwColor, (int)((float)(nU2 - nU1 + 1) * fScaleX), (int)((float)(nV2 - nV1 + 1) * fScaleY), fRot, u1,v1,u2,v2);
}


void	C2DScreenLayer :: DrawUVMesh(D3DXVECTOR3 center, DWORD dwColor, int width, int height, int nU1, int nV1, int nU2, int nV2, int mapSizeX, int mapSizeY)
{

	float				u1,v1,u2,v2;

	u1 = ((float)(nU1)) / ((float)(mapSizeX));
	v1 = ((float)(nV2)) / ((float)(mapSizeY));
	u2 = ((float)(nU2)) / ((float)(mapSizeX));
	v2 = ((float)(nV1)) / ((float)(mapSizeY));

	DrawUVMesh(center, dwColor, width, height, u1,v1, u2, v2);
}

void	C2DScreenLayer :: DrawUVMesh(D3DXVECTOR3 center, DWORD dwColor, int width, int height, float fRot, float fU1, float fV1, float fU2, float fV2)
{
	int					nScan;
	D3DVECTOR			pos[4];
	D3DVECTOR			Crd[4];
	COLORTEXTUREVERTEX	Vertex[6];
	D3DVECTOR			lt,rb;

	float				fWidth, fHeight;

	fWidth = (float)width;
	
	if (fWidth < 0)
		fWidth *= -1;

	fHeight = (float)height;

	if (fHeight < 0)
		fHeight *= -1;

	lt.x = - fWidth / 2.0f;
	rb.x = fWidth / 2.0f;

	lt.y = - fHeight / 2.0f;
	rb.y = fHeight / 2.0f;

	Crd[0].x = rb.x;
	Crd[0].y = lt.y;

	Crd[1].x = lt.x;
	Crd[1].y = lt.y;

	Crd[2].x = lt.x;
	Crd[2].y = rb.y;

	Crd[3].x = rb.x;
	Crd[3].y = rb.y;


	for(nScan = 0;nScan < 4;nScan++)
	{
		pos[nScan].x = (float)cos(fRot) * Crd[nScan].x + (float)sin(fRot) * Crd[nScan].y + center.x;
		pos[nScan].y = -(float)sin(fRot) * Crd[nScan].x + (float)cos(fRot) * Crd[nScan].y + center.y;
		pos[nScan].z = center.z;
	}

	Vertex[0].Vertex=pos[3];
	Vertex[1].Vertex=pos[1];
	Vertex[2].Vertex=pos[2];
	Vertex[3].Vertex=pos[0];
	Vertex[4].Vertex=pos[1];
	Vertex[5].Vertex=pos[3];

	Vertex[0].dwColor= dwColor;
	Vertex[1].dwColor= dwColor;
	Vertex[2].dwColor= dwColor;
	Vertex[3].dwColor= dwColor;
	Vertex[4].dwColor= dwColor;
	Vertex[5].dwColor= dwColor;

	Vertex[0].fTextureU = fU2;
	Vertex[0].fTextureV = fV1;
	Vertex[1].fTextureU = fU1;
	Vertex[1].fTextureV = fV2;
	Vertex[2].fTextureU = fU1;
	Vertex[2].fTextureV = fV1;
	Vertex[3].fTextureU = fU2;
	Vertex[3].fTextureV = fV2;
	Vertex[4].fTextureU = fU1;
	Vertex[4].fTextureV = fV2;
	Vertex[5].fTextureU = fU2;
	Vertex[5].fTextureV = fV1;

	//$ m_pKernel->DrawCustomMesh(2, Vertex, sizeof(COLORTEXTUREVERTEX));
}

void	C2DScreenLayer :: DrawUVMesh(D3DXVECTOR3 center, DWORD dwColor, int width, int height, float fU1, float fV1, float fU2, float fV2)
{
	D3DVECTOR			lt,rb;
	COLORTEXTUREVERTEX	Vertex[6];

	lt.x = center.x - width / 2.0f;
	rb.x = center.x + width / 2.0f - 1;

	lt.y = center.y - height / 2.0f;
	rb.y = center.y + height / 2.0f - 1;

	Vertex[0].Vertex=D3DXVECTOR3(rb.x, rb.y, center.z);
	Vertex[1].Vertex=D3DXVECTOR3(lt.x, lt.y, center.z);
	Vertex[2].Vertex=D3DXVECTOR3(lt.x, rb.y, center.z);
	Vertex[3].Vertex=D3DXVECTOR3(rb.x, lt.y, center.z);
	Vertex[4].Vertex=D3DXVECTOR3(lt.x, lt.y, center.z);
	Vertex[5].Vertex=D3DXVECTOR3(rb.x, rb.y, center.z);

	Vertex[0].dwColor= dwColor;
	Vertex[1].dwColor= dwColor;
	Vertex[2].dwColor= dwColor;
	Vertex[3].dwColor= dwColor;
	Vertex[4].dwColor= dwColor;
	Vertex[5].dwColor= dwColor;

	Vertex[0].fTextureU = fU2;
	Vertex[0].fTextureV = fV1;
	Vertex[1].fTextureU = fU1;
	Vertex[1].fTextureV = fV2;
	Vertex[2].fTextureU = fU1;
	Vertex[2].fTextureV = fV1;
	Vertex[3].fTextureU = fU2;
	Vertex[3].fTextureV = fV2;
	Vertex[4].fTextureU = fU1;
	Vertex[4].fTextureV = fV2;
	Vertex[5].fTextureU = fU2;
	Vertex[5].fTextureV = fV1;

	//$ m_pKernel->DrawCustomMesh(2, Vertex, sizeof(COLORTEXTUREVERTEX));
}

void	C2DScreenLayer :: DrawUVMesh(D3DXVECTOR3* pLT, D3DXVECTOR3 *pRB, DWORD dwColorLT, DWORD dwColorRT, DWORD dwColorLB, DWORD dwColorRB, float fU1, float fV1, float fU2, float fV2)
{
	COLORTEXTUREVERTEX	Vertex[6];

	Vertex[0].Vertex=D3DXVECTOR3(pRB->x, pRB->y, pLT->z);
	Vertex[1].Vertex=D3DXVECTOR3(pLT->x, pLT->y, pLT->z);
	Vertex[2].Vertex=D3DXVECTOR3(pLT->x, pRB->y, pLT->z);
	Vertex[3].Vertex=D3DXVECTOR3(pRB->x, pLT->y, pLT->z);
	Vertex[4].Vertex=D3DXVECTOR3(pLT->x, pLT->y, pLT->z);
	Vertex[5].Vertex=D3DXVECTOR3(pRB->x, pRB->y, pLT->z);

	Vertex[0].dwColor= dwColorRB;
	Vertex[1].dwColor= dwColorLT;
	Vertex[2].dwColor= dwColorLB;
	Vertex[3].dwColor= dwColorRT;
	Vertex[4].dwColor= dwColorLT;
	Vertex[5].dwColor= dwColorRB;

	Vertex[0].fTextureU = fU2;
	Vertex[0].fTextureV = fV1;
	Vertex[1].fTextureU = fU1;
	Vertex[1].fTextureV = fV2;
	Vertex[2].fTextureU = fU1;
	Vertex[2].fTextureV = fV1;
	Vertex[3].fTextureU = fU2;
	Vertex[3].fTextureV = fV2;
	Vertex[4].fTextureU = fU1;
	Vertex[4].fTextureV = fV2;
	Vertex[5].fTextureU = fU2;
	Vertex[5].fTextureV = fV1;

	//$ m_pKernel->DrawCustomMesh(2, Vertex, sizeof(COLORTEXTUREVERTEX));
}

//begin do-g
void	C2DScreenLayer :: FillTriangle(DWORD dwColor, float x1, float y1, float x2, float y2, float x3, float y3)
{
	COLORVERTEX	Vertex[3];

	//$ m_pKernel->GetDevice()->SaveState();
	//$ m_pKernel->GetDevice()->SetVertexShader(D3DFVF_COLORVERTEX);
	m_pKernel->GetDevice()->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
	m_pKernel->GetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
	m_pKernel->GetDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
	m_pKernel->GetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	
	SetLayerTexture(-1);

	Vertex[0].Vertex=D3DXVECTOR3(x1,y1,0.0f);
	Vertex[1].Vertex=D3DXVECTOR3(x2,y2,0.0f);
	Vertex[2].Vertex=D3DXVECTOR3(x3,y3,0.0f);
	
	Vertex[0].dwColor= dwColor;
	Vertex[1].dwColor= dwColor;
	Vertex[2].dwColor= dwColor;
	
	//$ m_pKernel->DrawCustomMesh(1, Vertex, sizeof(COLORVERTEX));

	//$ m_pKernel->GetDevice()->RestoreState();
}
void	C2DScreenLayer :: FillTriangle(DWORD dwColor1, DWORD dwColor2, DWORD dwColor3, float x1, float y1, float x2, float y2, float x3, float y3)
{
	COLORVERTEX	Vertex[3];

	//$ m_pKernel->GetDevice()->SaveState();
	//$ m_pKernel->GetDevice()->SetVertexShader(D3DFVF_COLORVERTEX);
	m_pKernel->GetDevice()->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
	m_pKernel->GetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
	m_pKernel->GetDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
	m_pKernel->GetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	
	SetLayerTexture(-1);

	Vertex[0].Vertex=D3DXVECTOR3(x1,y1,0.0f);
	Vertex[1].Vertex=D3DXVECTOR3(x2,y2,0.0f);
	Vertex[2].Vertex=D3DXVECTOR3(x3,y3,0.0f);
	
	Vertex[0].dwColor= dwColor1;
	Vertex[1].dwColor= dwColor2;
	Vertex[2].dwColor= dwColor3;
	
	//$ m_pKernel->DrawCustomMesh(1, Vertex, sizeof(COLORVERTEX));

	//$ m_pKernel->GetDevice()->RestoreState();
}

//end

void	C2DScreenLayer :: FillRect(DWORD dwColor, float L, float T, float R, float B)
{
	FillRect2(dwColor, dwColor, L, T, R, B);
}

void	C2DScreenLayer :: FillRect2(DWORD dwColorT, DWORD dwColorB, float L, float T, float R, float B)
{
	COLORVERTEX	Vertex[6];

	C3DDevice* pDevice=m_pKernel->GetDevice();
	//$ DWORD dwSavedVS;
	//$ pDevice->GetD3DDevice()->GetVertexShader(&dwSavedVS);
	//$ pDevice->SetVertexShader(D3DFVF_COLORVERTEX);
	pDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
	pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	
	SetLayerTexture(-1);

	Vertex[0].Vertex=D3DXVECTOR3(L,T,0.0f);
	Vertex[1].Vertex=D3DXVECTOR3(L,B,0.0f);
	Vertex[2].Vertex=D3DXVECTOR3(R,T,0.0f);
	Vertex[3].Vertex=D3DXVECTOR3(R,T,0.0f);
	Vertex[4].Vertex=D3DXVECTOR3(L,B,0.0f);
	Vertex[5].Vertex=D3DXVECTOR3(R,B,0.0f);

	Vertex[0].dwColor= dwColorT;
	Vertex[1].dwColor= dwColorB;
	Vertex[2].dwColor= dwColorT;
	Vertex[3].dwColor= dwColorT;
	Vertex[4].dwColor= dwColorB;
	Vertex[5].dwColor= dwColorB;

	//$ m_pKernel->DrawCustomMesh(2, Vertex, sizeof(COLORVERTEX));
	//$ pDevice->SetVertexShader(dwSavedVS);
	pDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
}


void	C2DScreenLayer :: FillRect3(DWORD dwColorL, DWORD dwColorR, float L, float T, float R, float B)
{
	COLORVERTEX	Vertex[6];

	C3DDevice* pDevice=m_pKernel->GetDevice();
	//$ DWORD dwSavedVS;
	//$ pDevice->GetD3DDevice()->GetVertexShader(&dwSavedVS);
	//$ pDevice->SetVertexShader(D3DFVF_COLORVERTEX);
	pDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
	pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );

	SetLayerTexture(-1);

	Vertex[0].Vertex=D3DXVECTOR3(L,T,0.0f);
	Vertex[1].Vertex=D3DXVECTOR3(L,B,0.0f);
	Vertex[2].Vertex=D3DXVECTOR3(R,T,0.0f);
	Vertex[3].Vertex=D3DXVECTOR3(R,T,0.0f);
	Vertex[4].Vertex=D3DXVECTOR3(L,B,0.0f);
	Vertex[5].Vertex=D3DXVECTOR3(R,B,0.0f);

	Vertex[0].dwColor= dwColorL;
	Vertex[1].dwColor= dwColorL;
	Vertex[2].dwColor= dwColorR;
	Vertex[3].dwColor= dwColorR;
	Vertex[4].dwColor= dwColorL;
	Vertex[5].dwColor= dwColorR;

	//$ m_pKernel->DrawCustomMesh(2, Vertex, sizeof(COLORVERTEX));
	//$ pDevice->SetVertexShader(dwSavedVS);
	pDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
}


void	C2DScreenLayer :: SetLayerTexture(int nTexId)
{
	//$ m_pKernel->SetTexture(nTexId, 0, MAP_DIFFUSE);
}


int	C2DScreenLayer :: LoadLayerTexture(char *pTextureFileName)
{
	int	nTexId = -1;

	/*
	//$

	if (pTextureFileName[0] != '\x0')
	{
		nTexId = m_pKernel->LoadTexture(pTextureFileName, MAP_DIFFUSE);
		if(nTexId<0) {
			assert(0&&"LoadLayerTexture");
			CriticalException();
		}
	}
	*/
	return nTexId;
}


int	C2DScreenLayer :: LoadLayerTexture(char *pTextureFileName, bool bAlpha)
{
	int	nTexId = -1;

	//$
	/*
	if (pTextureFileName[0] != '\x0')
	{
		nTexId = m_pKernel->LoadTexture(pTextureFileName, MAP_DIFFUSE, 1, bAlpha);
		if(nTexId<0) {
			assert(0&&"LoadLayerTexture");
			CriticalException();
		}
	}
	*/

	return nTexId;
}


void	C2DScreenLayer :: UnloadLayerTexture(int nTexId)
{
	if(nTexId != -1)
		m_pKernel->ReleaseTexture(nTexId);
}

void	C2DScreenLayer :: GetLayerTextureSize(int nTexId, int *pWidth, int *pHeight)
{
	/*
	SIZE size;
	//nParam1=Texture Index, Get Texture Interface.
	//$ m_pKernel->SendMessage(0x10000000, SM_GET_TEXTURESIZE, nTexId, (int)&size);
	*pWidth = size.cx;
	*pHeight = size.cy;
	*/
}


void	C2DScreenLayer :: SetBackBufferAsTexture(int nStage)
{
#ifdef _XBOX
	m_pKernel->GetDevice()->GetD3DDevice()->SetTexture( nStage, g_pBackBuffer);
#endif
}


void	C2DScreenLayer :: CheckLayerUndeleted()
{
	int		nScan;
	int		nCount;
	char	Buf[2048];
	char	Buf2[256];
	
	nCount = 0;
	strcpy(Buf,"ID list of undeleted layers: ");

	for(nScan = 0;nScan < m_nLayerTable;nScan++)
	{
		if (m_ppLayerTable[nScan] != NULL)
		{
			sprintf(Buf2,"%d ",nScan);
			strcat(Buf,Buf2);
			nCount++;
		}
	}

	assert(nCount == 0 && Buf);
}

void	C2DScreenLayer :: EnableZ(float fNear, float fFar)
{
	//$
	/*
	D3DXMATRIX	matView;
	
	if (m_bZEnabled == false)
	{
		m_bZEnabled = true;

		m_pKernel->GetDevice()->SetRenderState( D3DRS_ZENABLE,  TRUE );
		m_pKernel->GetDevice()->SetRenderState( D3DRS_ZWRITEENABLE,  TRUE );
		
		D3DXMatrixIdentity(&matView);

		D3DXMatrixOrthoOffCenterLH(&matView, 0, (float)(m_nScreenWidth-1), (float)(m_nScreenHeight-1), 0, fNear, fFar);
		m_pKernel->SetTransform(STR_TRANSFORM_PROJECTION, &matView);	
	}
	*/
}


void	C2DScreenLayer :: DisableZ(void)
{
	//$
	/*
	D3DXMATRIX	matView;

	if (m_bZEnabled == true)
	{
		m_bZEnabled = false;

		m_pKernel->GetDevice()->SetRenderState( D3DRS_ZENABLE,  FALSE );
		m_pKernel->GetDevice()->SetRenderState( D3DRS_ZWRITEENABLE,  FALSE );
		
		D3DXMatrixIdentity(&matView);

		D3DXMatrixOrthoOffCenterLH(&matView, 0, (float)(m_nScreenWidth-1), (float)(m_nScreenHeight-1), 0, -1.0f, 1.0f);
		m_pKernel->SetTransform(STR_TRANSFORM_PROJECTION, &matView);
	}
	*/
}

C2DScreenLayer*	C2DScreenLayer :: GetLayer(int nId)
{
	if (nId < 0 || nId >= m_nLayerTable)
		return NULL;
	else
		return m_ppLayerTable[nId];
}