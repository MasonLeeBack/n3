#pragma once

#include "3DDevice.h"

class CBsGenerateTexture
{
public:
	CBsGenerateTexture();
	virtual ~CBsGenerateTexture();

	virtual void Create(int nTextureWidth, int nTextureHeight, float fStartX = 0.f, float fStartY = 0.f, float fWidth = 1.f, float fHeight = 1.f, bool bAlpha = false);
	void ReInitialize();
	void Clear();

	virtual void InitRenderRTT() {}
	virtual void Render(C3DDevice *pDevice)	{}
	virtual void Release() {}

	void	SetHandle(DWORD hHandle)	{ m_hHandle = hHandle; }
	DWORD	GetHandle()					{ return m_hHandle; }

	int		GetCameraIndex()			{ return m_nCameraIndex; }
	int		GetTextureID();

	bool	IsEnable()					{ return m_bEnable; }
	void	SetEnable(bool bEnable)		{ m_bEnable = bEnable; }

protected:
	DWORD							m_hHandle;
	int								m_nCameraIndex;
	bool							m_bEnable;
};

class CBsGenerateWorldLightMapTexture : public CBsGenerateTexture
{
public:
	CBsGenerateWorldLightMapTexture(int nBaseTextureIndex);
	virtual ~CBsGenerateWorldLightMapTexture();

	virtual void Create(int nTextureWidth, int nTextureHeight, float fStartX = 0.f, float fStartY = 0.f, float fWidth = 1.f, float fHeight = 1.f, bool bAlpha = false);
	void ReInitialize();
	void Clear();

	virtual void InitRenderRTT();
	virtual void Render(C3DDevice* pDevice);
protected:
	struct SCREEN_VERTEX {
		D3DXVECTOR2	vecPos;
		D3DXVECTOR2 vecUV;
	};
	SCREEN_VERTEX m_ScreenVertex[4];

	int			m_nVertexDeclIndex;

	int			m_nLightMap;
	int			m_nMaterialIndex;

	D3DXHANDLE	m_hInputBaseTexture;
};