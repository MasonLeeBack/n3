#include "stdafx.h"
#include "BsUiMovieTexture.h"
#include "BsKernel.h"
#include "BsFileIO.h"
#include "binkxenon\bink.h"

CBsUiMovieTexture::CBsUiMovieTexture(char *szMoviePath)
: CBsGenerateTexture()
{
	DWORD dwFileSize;

	if(!FAILED(CBsFileIO::LoadFilePhysicalMemory(szMoviePath, &m_pMovieData, &dwFileSize, &m_dwMemFlags)))
	{
		
#ifdef _XBOX // no endian swap for PC
		//Bink needs the movie endian swapped - This should really be done off-line
		XGEndianSwapMemory(m_pMovieData, m_pMovieData, XGENDIAN_8IN32, sizeof(DWORD), dwFileSize / sizeof(DWORD));
#endif

		if(FAILED(g_BsMoviePlayer.InitBinkMovie(m_pMovieData, true)))
		{
			m_bBinkInited = false;
			CBsFileIO::FreePhysicalMemory(m_pMovieData, m_dwMemFlags);
			m_pMovieData = NULL;
		}
		else
			m_bBinkInited = true;
	}
}

CBsUiMovieTexture::~CBsUiMovieTexture()
{
	if(m_bBinkInited)
	{
		g_BsMoviePlayer.Stop();
		m_bBinkInited = false;
	}
	if(m_pMovieData)
	{
		CBsFileIO::FreePhysicalMemory(m_pMovieData, m_dwMemFlags);
		m_pMovieData = NULL;
	}

}

void CBsUiMovieTexture::Create(int nTextureWidth, int nTextureHeight, float fStartX, float fStartY, float fWidth, float fHeight, bool bAlpha)
{
	BsAssert(m_bBinkInited);
	
	BINKSUMMARY sum;
	
	// Ignore the input width and height! Make the texture the same size as the movie,
	// so the video won't get scaled!
	g_BsMoviePlayer.GetBinkSummary(&sum);
	CBsGenerateTexture::Create(sum.Width, sum.Height, fStartX, fStartY, fWidth, fHeight, bAlpha);

	RECT Rect;
	Rect.left = 0; Rect.right = sum.Width;
	Rect.top = 0; Rect.bottom = sum.Height;

}

void CBsUiMovieTexture::Release()
{
	CBsGenerateTexture::Clear();

	if(m_bBinkInited)
	{
		g_BsMoviePlayer.Stop();
		m_bBinkInited = false;
	}
	if(m_pMovieData)
	{
		CBsFileIO::FreePhysicalMemory(m_pMovieData, m_dwMemFlags);
		m_pMovieData = NULL;
	}
}

void CBsUiMovieTexture::Render(C3DDevice *pDevice)
{
	g_BsKernel.LockActiveCamera(GetCameraIndex());

	/////////////////////////////////////////////////////////////
	// Camera Lock하는 곳으로 옮겨야합니다. by jeremy
	pDevice->SetViewport(g_BsKernel.GetActiveCamera()->GetCameraViewport());
	/////////////////////////////////////////////////////////////

	pDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
	pDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE);
	pDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_MAX);

	g_BsKernel.GetDevice()->SaveState();
		
	g_BsMoviePlayer.RenderNextFrame();
	
	g_BsKernel.GetDevice()->RestoreState();

	pDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
	pDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ZERO);
	pDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);

	g_BsKernel.UnlockActiveCamera();
}
