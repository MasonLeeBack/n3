#include "stdafx.h"
#include "BsUIBase.h"
#include "BsKernel.h"
#include "FrameChecker.h"
#include "BsrealMovie.h"

#ifdef _XBOX
#include "./DX9_Util/AtgFont.h"
#endif

//------------------------------------------------------------------------------------------------

void CBsKernel::CreateUI() 
{
	if (!m_pUIManager)
		m_pUIManager=new CBsUIManager();

	if(!m_pClickFxEffect)
		m_pClickFxEffect = new CBsUiClickTexture();
}

void CBsKernel::CreateUIforTool() 
{
	if (!m_pUIManager)
		m_pUIManager=new CBsUIManager();
}


void CBsKernel::SetUIViewerIndex(int nViewIndex) 
{ 
	if(m_pUIManager)
	{
		m_pUIManager->SetViewportIndex(nViewIndex);
	}
}

void CBsKernel::ClearUIElemental()
{
	if(m_pUIManager)
	{
		for(int i=0; i<2; i++)
		{
			m_pUIManager->ClearElementAll(i);
			m_pUIManager->ClearElementBGMovie(i);
			m_pUIManager->ClearElementException(i);
		}
	}
}

//------------------------------------------------------------------------------------------------
bool CBsKernel::DrawUIMesh(BS_UI_MODE mode,
						   int nMeshIndex, int nX, int nY,
						   D3DXCOLOR color, int nTexId,
                           float param1, float param2,
						   float param3, float param4)
{
	if(m_pUIManager == NULL){
		return false;
	}
	
	bool bRet = m_pUIManager->CreateElementMesh(mode,
		nMeshIndex, nX, nY,
		color, nTexId,
		param1, param2,
		param3, param4);

	return bRet;
}

bool CBsKernel::DrawUIBox(BS_UI_MODE mode, 
						  int x1, int y1, int x2, int y2, 
						  float fz, D3DXCOLOR color,
						  float fRot,
						  int nTextureID,
						  int ux1, int uy1,
						  int ux2, int uy2,
						  int nDummyTexId,
						  bool bRTT)
{
	if(m_pUIManager == NULL)
		return false;

	bool bRet = m_pUIManager->CreateElement_Pos2(mode,
		x1, y1, x2, y2,
		fz, color,
		fRot,
		nTextureID,
		ux1, uy1, ux2, uy2,
		nDummyTexId,
		bRTT);

	return bRet;
}

bool CBsKernel::DrawUIBox_s(BS_UI_MODE mode, 
							int x, int y, int sx, int sy, 
							float fz, D3DXCOLOR color,
							float fRot,
							int nTextureID,
							int ux1, int uy1,
							int ux2, int uy2,
							int nDummyTexId,
							bool bRTT)
{ 
	bool bRet = DrawUIBox(mode,
		x, y, x+sx, y+sy,
		fz, color,
		fRot,
		nTextureID,
		ux1, uy1, ux2, uy2,
		nDummyTexId,
		bRTT);

	return bRet;
}


bool CBsKernel::DrawUIBox_Pos4(BS_UI_MODE mode,
							   int x1, int y1, int x2, int y2,
							   int x3, int y3, int x4, int y4,
							   float fz, D3DXCOLOR color,
							   int nTextureID,
							   int ux1, int uy1,
							   int ux2, int uy2,
							   int nDummyTexId,
							   bool bRTT)
{	
	if(m_pUIManager == NULL)
		return false;

	bool bRet = m_pUIManager->CreateElement_Pos4(mode,
		x1, y1, x2, y2,
		x3, y3, x4, y4, 
		fz, color,
		nTextureID,
		ux1, uy1, ux2, uy2,
		nDummyTexId,
		bRTT);

	return bRet;
}

bool CBsKernel::DrawUIBox_PosColor4(BS_UI_MODE mode, 
									int x1, int y1, D3DXCOLOR color1,
									int x2, int y2, D3DXCOLOR color2,
									int x3, int y3, D3DXCOLOR color3,
									int x4, int y4, D3DXCOLOR color4,
									float fz,
									int nTextureID,
									int ux1, int uy1,
									int ux2, int uy2,
									int nDummyTexId,
									bool bRTT)
{	
	if(m_pUIManager == NULL)
		return false;

	bool bRet = m_pUIManager->CreateElement_PosColor4(mode,
		x1, y1, color1,
		x2, y2, color2,
		x3, y3, color3,
		x4, y4, color4,
		fz,
		nTextureID,
		ux1, uy1, ux2, uy2,
		nDummyTexId,
		bRTT);

	return bRet;
}

//------------------------------------------------------------------------------------------------
// debug string
#ifdef _USE_CONSOLE_COMMAND_
bool CBsKernel::CreateUIDebug(int x, int y, char *str, D3DXCOLOR color)
{
	if(m_pUIManager == NULL)
		return false;

	bool bRet = m_pUIManager->CreateElementDebug(x, y, color, str);

	return bRet;
}

void CBsKernel::RenderUIDebug(int x, int y, char* str, DWORD dwColor)
{
	if( strlen( str ) == 0 )
		return;

#ifdef _XBOX
	m_pFont->DrawText(float(x), float(y), dwColor, str);	
#else
	RECT rect = {x, y, 0, 0};
	m_pFont->DrawText(NULL, str, -1, &rect, DT_LEFT | DT_TOP | DT_NOCLIP, dwColor); 
#endif
}
#endif //#ifndef _LTCG

char *CBsKernel::GetUIFPSString() 
{ 
	return (char *)m_pChecker->GetFPSString(); 
}

//------------------------------------------------------------------------------------------------

#ifdef _USAGE_TOOL_

int CBsKernel::g_nSkipSameTexture;

#endif

//------------------------------------------------------------------------------------------------

void CBsKernel::RealMovieSave(BStream *pStream)	
{
	if (m_pRealMovie) {
		CBsRealMovie *pt=(CBsRealMovie *)m_pRealMovie;
		pt->Save(pStream);
	} 
	else {
		CBsRealMovie *pt=new CBsRealMovie;
		pt->Save(pStream);
		delete pt;
	}
}

void CBsKernel::RealMovieLoad(BStream *pStream)	
{
	if (!m_pRealMovie) m_pRealMovie = new CBsRealMovie(); // 없으면 재생성 하고, 있을때는 메모리 자동 관리 됩니다

	CBsRealMovie *pt=(CBsRealMovie *)m_pRealMovie;
    pt->Load(pStream);
	// pt->AllocateResource();
}


// void CBsKernel::RealMoviePlay(float fTime, D3DXMATRIX *pMatrix, CCrossVector *pCameraCross, CCrossVector *pLightCross)
void CBsKernel::RealMovieProcess(float fTime,int nTick)
{
	if (m_pRealMovie) 
	{
		((CBsRealMovie *)m_pRealMovie)->Process( fTime, nTick );
		// ((CBsRealMovie *)m_pRealMovie)->Play(fTime,pMatrix,pCameraCross,pLightCross);	
	}
}

void CBsKernel::RealMovieProcess(DWORD dwTick)
{
	if (m_pRealMovie) 
	{
		//((CBsRealMovie *)m_pRealMovie)->Process( fTime );
	}
}

void CBsKernel::RealMovieClear()
{
	if (m_pRealMovie) {
		CBsRealMovie *pt=(CBsRealMovie *)m_pRealMovie;
		delete pt; // pt->Clear() 포함

		m_pRealMovie=NULL;
	}
}

void CBsKernel::RealMovieNew()
{
	RealMovieClear();

	m_pRealMovie = new CBsRealMovie(); 

	CBsRealMovie *pt=(CBsRealMovie *)m_pRealMovie;
	pt->AllocateResource();
}

//------------------------------------------------------------------------------------------------

void CBsKernel::GetUITextureSize(int nTextureId, int &nWeight, int &nHeight)
{
	SIZE size = GetTextureSize(nTextureId);
	
	nWeight = size.cx;
	nHeight = size.cy;
}
/*
bool CBsKernel::IsUIRTTRenderEnable()
{
	if(m_pUIManager == NULL || m_pRTTManager == NULL)
	{
		return false;
	}

	DWORD hHandle = m_pUIManager->GetRTTHandle();
	if(m_pRTTManager->GetRTTextrue(hHandle) == NULL)
	{
		return false;
	}

	return m_pRTTManager->GetRTTextrue(hHandle)->IsEnable();
}

void CBsKernel::SetUIRTTRenderEnable(bool bEnable)
{
	if(m_pUIManager == NULL || m_pRTTManager == NULL)
	{
		return;
	}

	DWORD hHandle = m_pUIManager->GetRTTHandle();
	if(m_pRTTManager->GetRTTextrue(hHandle) == NULL)
	{
		return;
	}

	m_pRTTManager->GetRTTextrue(hHandle)->SetEnable(bEnable);
}
*/