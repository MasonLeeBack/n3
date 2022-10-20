#pragma once

#include "Singleton.h"

class C3DDevice;
class CBsGenerateTexture;

class CBsOffScreenManager : public CSingleton<CBsOffScreenManager>
{
public:
	CBsOffScreenManager();
	~CBsOffScreenManager();

	DWORD	Create(CBsGenerateTexture* pBsGTex,
		int nWidth, int nHeight, 
		float fStartX = 0.f, float fStartY = 0.f,
		float fWidth = 1.f, float fHeight = 1.f,
		bool bAlpha = false);

	void	ReInitialize();
	void	Clear();
	bool	Release(DWORD hHandle);

	void	InitRenderRTTs();
	void	Render(C3DDevice *pDevice);

	int						GetRTTextureID(DWORD hHandle);
	SIZE					GetRTTSize(DWORD hHandle);

	bool	IsEnable()					{ return m_bEnable; }
	void	SetEnable(bool bEnable)		{ m_bEnable = bEnable; }

	CBsGenerateTexture*		GetRTTextrue(DWORD hHandle);

	static CBsCriticalSection	s_csOffScreenManager;

protected:
	bool					IsValid(DWORD hHandle);
	std::vector<CBsGenerateTexture*> m_pRttList;

	bool		m_bEnable;
};