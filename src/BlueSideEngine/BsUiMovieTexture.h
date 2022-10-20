#pragma once

#include "3DDevice.h"
#include "BsGenerateTexture.h"
#include "BsMoviePlayer.h"

#ifdef _XBOX
#include <xtl.h>
#include <xboxmath.h>
#include <AtgUtil.h>
#include <xaudio.h>
#endif

class CBsUiMovieTexture : public CBsGenerateTexture
{
public:
	CBsUiMovieTexture(char *szMoviePath);
	~CBsUiMovieTexture();

	virtual void Create(int nTextureWidth, int nTextureHeight, float fStartX = 0.f, float fStartY = 0.f, float fWidth = 1.f, float fHeight = 1.f, bool bAlpha = false);

	void Release();

	void Render(C3DDevice *pDevice);
	void Stop()							{ g_BsMoviePlayer.Stop(); }
	void Pause()						{ g_BsMoviePlayer.Pause(); }
	void Resume()						{ g_BsMoviePlayer.Resume(); }

protected:
	bool m_bBinkInited;
	VOID * m_pMovieData;
	DWORD m_dwMemFlags;
};
