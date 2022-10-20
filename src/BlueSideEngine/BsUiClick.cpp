#include "stdafx.h"

#include "BsUiClick.h"

#include "BsKernel.h"
#include "BsMaterial.h"
#include "BsImageProcess.h"


#define _CLICK_SQUARE_COUNT		5
#define _VERTEX_BUFFER_SIZE		4 * _CLICK_SQUARE_COUNT
#define _INDEX_BUFFER_SIZE		2 * _CLICK_SQUARE_COUNT

#define _WAVE_VOLUME_FRAME		64			
#define _WAVE_START_FRAME		0
#define _WAVE_END_FRAME			6
#define _WAVE_VOLUME_SIZE_X		512//256
#define _WAVE_VOLUME_SIZE_Y		512//256
#define _START_ALPHA_DOWN		20
#define _PLAY_FRAME				_WAVE_VOLUME_FRAME - _WAVE_END_FRAME;
#define _START_ALPHA			0.5f

static inline float RandomNumberInRange( float Min, float Max )
{
	float fRange, fRandom;

	fRandom = rand() / ( float )RAND_MAX;
	fRange = Max - Min;
	fRandom *= fRange;
	fRandom += Min;
    
	return fRandom;
}

CBsUiClickTexture::CBsUiClickTexture()
{
	m_nMaterialIndex = -1;
	m_nUIClickVertexDeclIndex = -1;

	m_pVB = NULL;
	m_pIB = NULL;
	m_puiv = NULL;
	m_pIndex = NULL;

	m_nBackBufferTexId = -1;

	InitVertexShader();

	SetupVertexShader();

	CreateVertexPool();

	Create();
		
	//-------------------------------------------------------------------
	m_Viewport.X = 0;
	m_Viewport.Y = 0;
	m_Viewport.Width = g_BsKernel.GetDevice()->GetBackBufferWidth();
	m_Viewport.Height = g_BsKernel.GetDevice()->GetBackBufferHeight();
#ifdef INV_Z_TRANSFORM
	m_Viewport.MinZ = 1.f;
	m_Viewport.MaxZ = 0.f;
#else
	m_Viewport.MinZ = 0.f;
	m_Viewport.MaxZ = 1.f;
#endif

	m_nRenderTick = 0;
}

void CBsUiClickTexture::InitVertexShader()
{
	char fullName[_MAX_PATH];
	strcpy(fullName, g_BsKernel.GetShaderDirectory());
	strcat(fullName, "UiWave.fx");
	m_nMaterialIndex = g_BsKernel.LoadMaterial(fullName, FALSE);
}

void CBsUiClickTexture::SetupVertexShader()
{	
	CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialIndex);

#ifdef _XBOX
	m_hLayerTexture = pMaterial->GetParameterByName("LayerSampler");
	m_hWaveTexture = pMaterial->GetParameterByName("WaveSampler");
#else
	m_hLayerTexture = pMaterial->GetParameterByName("LayerTexture");
	m_hWaveTexture = pMaterial->GetParameterByName("WaveTexture");
#endif

	D3DVERTEXELEMENT9 decl[] =
	{
		{ 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		{ 0, 20, D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
		{ 0, 24, D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2 },

		D3DDECL_END()
	};
	m_nUIClickVertexDeclIndex = g_BsKernel.LoadVertexDeclaration(decl);
}

void CBsUiClickTexture::CreateVertexPool()
{
	if(m_puiv == NULL)
	{
		m_puiv = new CLICKVERTEX[_CLICK_SQUARE_COUNT*4];
	}

	if(m_pIndex == NULL)
	{	
		m_pIndex = new WORD[_CLICK_SQUARE_COUNT*6];
		UpdateIndex(_CLICK_SQUARE_COUNT, m_pIndex);
	}
}

void CBsUiClickTexture::Create()
{
	g_BsKernel.chdir("data");
	g_BsKernel.chdir("interface");
	m_nWaveTexId = g_BsKernel.LoadVolumeTexture("wave_128_u8v8_64.dds");//wave_64_u8v8_64.dds");//wave_256_u8v8_64.dds");//
	g_BsKernel.chdir("..");
	g_BsKernel.chdir("..");

	m_nBackBufferTexId = g_BsKernel.GetImageProcess()->GetBackBufferTexture();
	BsAssert(m_nBackBufferTexId != -1);
}


void CBsUiClickTexture::Release()
{
	SAFE_RELEASE_VD(m_nUIClickVertexDeclIndex);

	if(m_puiv) {
		delete[] m_puiv;
		m_puiv = NULL;
	}

	if(m_pIndex) {
		delete[] m_pIndex;
		m_pIndex = NULL;
	}

	if(m_pIndex) {
		delete[] m_pIndex;
		m_pIndex = NULL;
	}

	for(DWORD i=0; i<m_pElementList.size(); i++)
	{
		delete m_pElementList[i];
	}
	m_pElementList.clear();
}


void CBsUiClickTexture::AddWave(int x, int y, int size, int nSpeed)
{
	if(m_pElementList.size() >= _CLICK_SQUARE_COUNT)
	{
		delete m_pElementList[0];
		m_pElementList.erase(m_pElementList.begin());
	}

	UiClickElement* pElement = new UiClickElement();
	pElement->nX = x;
	pElement->nY = y;
	pElement->nSizeX = size;	//_WAVE_VOLUME_SIZE_X;
	pElement->nSizeY = size;	//_WAVE_VOLUME_SIZE_Y;
	pElement->nFrame = _WAVE_START_FRAME;
	pElement->nSpeed = nSpeed;
	pElement->fAlpha = 0.5f;


	m_pElementList.push_back(pElement);

	if(m_pElementList.size() == 1){
		m_nRenderTick = 0;
	}
}


void CBsUiClickTexture::Render(C3DDevice *pDevice)
{
	if(m_nWaveTexId == -1 || m_nBackBufferTexId == -1){
		return;
	}

	if(m_pElementList.size() == 0){
		return;
	}

	//D3DVIEWPORT9 tempViewport;
	//pDevice->GetViewport(&tempViewport);
	pDevice->SetViewport(&m_Viewport);
	
	//---------------------------------------------------------
	DWORD pm1,pm2,pm3,pm4,pm5,pm6;
	pDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &pm1);
	pDevice->GetRenderState(D3DRS_ALPHATESTENABLE,  &pm2);
	pDevice->GetRenderState(D3DRS_SRCBLEND,         &pm3);
	pDevice->GetRenderState(D3DRS_DESTBLEND,        &pm4);
	pDevice->GetRenderState(D3DRS_FILLMODE,         &pm5);
	pDevice->GetRenderState(D3DRS_ZENABLE,			&pm6);

	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE,  FALSE);
	pDevice->SetRenderState(D3DRS_SRCBLEND,         D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND,        D3DBLEND_INVSRCALPHA);
	pDevice->SetRenderState(D3DRS_FILLMODE,         D3DFILL_SOLID);
	pDevice->SetRenderState(D3DRS_ZENABLE,			D3DZB_TRUE);

	//---------------------------------------------------------	
	//pDevice->BeginScene();

	RenderProcess();

	for(DWORD i=0; i<m_pElementList.size(); i++)
	{
		g_BsKernel.GetImageProcess()->ScreenCaptureFinal(pDevice);

		UiClickElement* pElement = m_pElementList[i];
		RenderObjectUp(pDevice, pElement);
	}

	//pDevice->EndScene();

	//---------------------------------------------------------
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, pm1);
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE,  pm2);
	pDevice->SetRenderState(D3DRS_SRCBLEND,         pm3);
	pDevice->SetRenderState(D3DRS_DESTBLEND,        pm4);
	pDevice->SetRenderState(D3DRS_FILLMODE,         pm5);
	pDevice->SetRenderState(D3DRS_ZENABLE, pm6);

	//---------------------------------------------------------

	//pDevice->SetViewport(&tempViewport);

	m_nRenderTick++;
}

void CBsUiClickTexture::RenderProcess()
{
	float fAddAlpha = _START_ALPHA / _START_ALPHA_DOWN;
	for(DWORD i=0; i<m_pElementList.size(); i++)
	{
		UiClickElement* pElement = m_pElementList[i];
		if(m_nRenderTick % 2 == 0)
		{
			pElement->nFrame += 1 * pElement->nSpeed;
			int nFlag = _PLAY_FRAME - _START_ALPHA_DOWN;
			if(pElement->nFrame >= nFlag ){
				pElement->fAlpha -= fAddAlpha;
			}
		}

		if(pElement->fAlpha < 0.f){
			pElement->fAlpha = 0.f;
		}

		int nPlayFrame = _PLAY_FRAME;
		if( pElement->nFrame >= nPlayFrame )
		{
			delete m_pElementList[i];
			m_pElementList.erase( m_pElementList.begin() + i );
			i--;
			continue;
		}
	}
}

void CBsUiClickTexture::RenderObjectUp(C3DDevice *pDevice, UiClickElement* pElement)
{
	assert(m_puiv != NULL);

	UpdateVertex(m_puiv, pElement);

	CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialIndex);
	
	LPDIRECT3DBASETEXTURE9 pTexture0 = NULL;
	
	pTexture0 = (LPDIRECT3DBASETEXTURE9)g_BsKernel.GetTexturePtr(m_nBackBufferTexId);
	assert(pTexture0 != NULL);

	LPDIRECT3DBASETEXTURE9 pTexture1 = (LPDIRECT3DBASETEXTURE9)g_BsKernel.GetTexturePtr(m_nWaveTexId);
	assert(pTexture1 != NULL);
	
	bool bIsSave = false;
	if(bIsSave) {
		HRESULT hr = D3DXSaveTextureToFile("Wave_2.bmp", D3DXIFF_BMP, pTexture0, NULL);
		bIsSave = false;
	}

	pMaterial->SetTexture(m_hLayerTexture, pTexture0);
	pMaterial->SetTexture(m_hWaveTexture, pTexture1);

	int nTechIndex = 0;
	pMaterial->BeginMaterial(nTechIndex, 0);
	pMaterial->BeginPass(0);

	pMaterial->CommitChanges();

	g_BsKernel.SetVertexDeclaration(m_nUIClickVertexDeclIndex);
	
	pDevice->DrawIndexedPrimitiveUP( D3DPT_TRIANGLELIST, 0, 4, 2,
		m_pIndex, D3DFMT_INDEX16, m_puiv, sizeof(CLICKVERTEX) );

	pMaterial->EndPass();
	pMaterial->EndMaterial();
	
	if(bIsSave) {
		HRESULT hr = D3DXSaveTextureToFile("test_2.bmp", D3DXIFF_BMP, pTexture0, NULL);
	}
}

void CBsUiClickTexture::UpdateVertex(CLICKVERTEX* puiv, UiClickElement* pElement)
{
	int nBsx = g_BsKernel.GetDevice()->GetBackBufferWidth();
	int nBsy = g_BsKernel.GetDevice()->GetBackBufferHeight();

	float fposX =  (float)pElement->nX;
	float fposY =  (float)pElement->nY;
	float fHalfsizeX = (float)pElement->nSizeX;
	float fHalfsizeY = (float)pElement->nSizeY;

	float fx1= ((fposX - fHalfsizeX) / nBsx - 0.5f)*2;
	float fy1=-((fposY - fHalfsizeY) / nBsy - 0.5f)*2;
	float fx2= ((fposX + fHalfsizeX) / nBsx - 0.5f)*2;
	float fy2=-((fposY + fHalfsizeY) / nBsy - 0.5f)*2;

	// vectex pos
	puiv[0].vecPos	= D3DXVECTOR3(fx1, fy2, 0.f);
	puiv[1].vecPos	= D3DXVECTOR3(fx2, fy2, 0.f);
	puiv[2].vecPos	= D3DXVECTOR3(fx1, fy1, 0.f);
	puiv[3].vecPos	= D3DXVECTOR3(fx2, fy1, 0.f);

	// texture pos
	puiv[0].vecUV	= D3DXVECTOR2(0.f, 1.f);
	puiv[1].vecUV	= D3DXVECTOR2(1.f, 1.f);
	puiv[2].vecUV	= D3DXVECTOR2(0.f, 0.f);
	puiv[3].vecUV	= D3DXVECTOR2(1.f, 0.f);

	puiv[0].frame	= (float)pElement->nFrame/_WAVE_VOLUME_FRAME;
	puiv[1].frame	= (float)pElement->nFrame/_WAVE_VOLUME_FRAME;
	puiv[2].frame	= (float)pElement->nFrame/_WAVE_VOLUME_FRAME;
	puiv[3].frame	= (float)pElement->nFrame/_WAVE_VOLUME_FRAME;

	puiv[0].fAlpha	= (float)pElement->fAlpha;
	puiv[1].fAlpha	= (float)pElement->fAlpha;
	puiv[2].fAlpha	= (float)pElement->fAlpha;
	puiv[3].fAlpha	= (float)pElement->fAlpha;
}

void CBsUiClickTexture::UpdateIndex(int nCount, WORD *pIndex)
{
	for(int i=0;i<nCount;i++){
		pIndex[i*6]=i*4;
		pIndex[i*6+1]=i*4+2;
		pIndex[i*6+2]=i*4+1;

		pIndex[i*6+3]=i*4+1;
		pIndex[i*6+4]=i*4+2;
		pIndex[i*6+5]=i*4+3;
	}
}

int CBsUiClickTexture::GetWaveCount()
{ 
	int nCount = (int)m_pElementList.size();
	return nCount;
}

int CBsUiClickTexture::GetWaveFrame()
{ 
	return _WAVE_VOLUME_FRAME - _WAVE_END_FRAME - _WAVE_START_FRAME;
}

int CBsUiClickTexture::GetLastWaveRemainFrame()
{
	if(m_pElementList.size() == 0){
		return 0;
	}

	return m_pElementList[m_pElementList.size()-1]->nFrame - _WAVE_START_FRAME;
}