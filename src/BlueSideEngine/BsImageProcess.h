#pragma once

//#define _SHOW_DEBUG_TEXTURE_

enum	eFILTER_TYPE
{
	FILTER_NONE = 0,
	FILTER_DOWNFILTER4,
	FILTER_BRIGHTPASS,
	FILTER_BLOOMH,
	FILTER_BLOOMV,
	FILTER_UPFILTER4,
	FILTER_COMBINE,
	FILTER_DOF,
	FILTER_CUSTOMBLUR1,
	FILTER_CUSTOMBLUR2,
};

class CBsRenderTargetChain
{
public:
	CBsRenderTargetChain();
	virtual ~CBsRenderTargetChain();

	void Create(int nTextureWidth, int nTextureHeight, D3DFORMAT format);
	void Clear();

	void Reload();
protected:
	int					m_nNext;

	int					m_nRTTextureWidth;
	int					m_nRTTextureHeight;
	D3DFORMAT			m_TextureFormat;

	int					m_nRTTexture[2];
#ifndef _XBOX
	LPDIRECT3DSURFACE9	m_pRTSurface[2];
#endif
public:
	void Flip()			{	m_nNext = 1 - m_nNext;	}
	int					GetPrevRTTextureIndex()		{	return m_nRTTexture[1-m_nNext];	}
	int					GetNextRTTextureIndex()		{	return m_nRTTexture[m_nNext];	}
#ifndef _XBOX 
	LPDIRECT3DSURFACE9	GetPrevRTSurface()			{	return m_pRTSurface[1-m_nNext];	}
	LPDIRECT3DSURFACE9	GetNextRTSurface()			{	return m_pRTSurface[m_nNext];	}
#endif
};

class CBsImageProcessFilter
{
public:
	CBsImageProcessFilter();
	virtual ~CBsImageProcessFilter();

	void	Create(int nTextureWidht, int nTextureHeight, eFILTER_TYPE filter );
public:
	eFILTER_TYPE	m_FilterType;

	int				m_nMaterialIndex;

	D3DXHANDLE		m_hInputTextureHandle;
	D3DXHANDLE		m_hDestTextureHandle;
	D3DXHANDLE		m_hTextureSizeHandle;

	D3DXHANDLE		m_hOriginIntensity;
	D3DXHANDLE		m_hBlurIntensity;
	D3DXHANDLE		m_hGlowIntensity;

	D3DXHANDLE		m_HighlightThreshold;
	D3DXHANDLE		m_hAddColor;
	
	D3DXHANDLE		m_hTime;
	D3DXHANDLE		m_hOffsetTexture;

	int				m_nVertexDeclIdx;

protected:
	void	Clear();
};

class CBsImageProcess
{
public:
	CBsImageProcess();
	virtual ~CBsImageProcess();

	void		Create();
	void		Reload();

	void		ScreenCaptureFinal(C3DDevice* pDevice);
	void		ScreenCaptureBeforeAlphaBlendPass(C3DDevice* pDevice);
	void		ScreenCaptureExtraFilter(C3DDevice* pDevice);

	void		Render(C3DDevice* pDevice);

	void		EnableImageProcess(BOOL bEnable)	{	m_bEnableImageProcess = bEnable;	}
	BOOL		IsEnableImageProcess()				{	return m_bEnableImageProcess;		}

	void		SetSceneIntensity(float fSceneIntensity)	{	m_fSceneIntensity = fSceneIntensity;	}
	float		GetSceneIntensity()				{	return m_fSceneIntensity;	}

	void		SetBlurIntensity(float fBlur)	{	m_fBlurIntensity = fBlur;	}
	float		GetBlurIntensity()				{	return m_fBlurIntensity;	}

	void		SetGlowIntensity(float fGlow)	{	m_fGlowIntensity = fGlow;	}
	float		GetGlowIntensity()				{	return m_fGlowIntensity;	}


	void		SetHighlightThreshold(float fHight)	{	m_fHighlightThreshold = fHight;	}
	float		GetHighlightThreshold()				{	return m_fHighlightThreshold;	}


	// DOF func
	void		SetDOFFocus(float fFocus)	{ m_fDOFFocus = fFocus; }
	float		GetDOFFocus()				{ return m_fDOFFocus;	}
	void		SetDOFFocusRangeFar(float fFocusRange);
	float		GetDOFFocusRangeFar()			{ return m_fDOFFocusRangeFar;	}
	void		SetDOFFocusRangeNear(float fFocusRange);
	float		GetDOFFocusRangeNear()			{ return m_fDOFFocusRangeNear;	}


	void		SetDOFFocusOutFar(float fTap)	{ m_fDOFFocusOutFar = fTap ; }
	void		SetDOFFocusOutNear(float fTap)	{ m_fDOFFocusOutNear = fTap ; }
	float		GetDOFFocusOutFar()				{ return m_fDOFFocusOutFar; }
	float		GetDOFFocusOutNear()			{ return m_fDOFFocusOutNear; }


	void		SetDOFFocusOutFarDef();
	void		SetDOFFocusOutNearDef();

	BOOL		IsEnabledDOF()				{ return m_nEnableDOF;	}
	void		EnableDOF(int nOn)			{ m_nEnableDOF = nOn;	}
	// DOF func~


	// Add Color In Scene Combine Filter
	void		SetAddColor(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 0.0f ) { m_v4AddColor.x = r, m_v4AddColor.y = g, m_v4AddColor.z = b, m_v4AddColor.w = a; }
	D3DXVECTOR4	GetAddColor()				{ return m_v4AddColor;	}



	int			GetBackBufferTexture()						{	return m_nBackBufferTexture;	}
	int			GetBackBufferTextureBeforeAlphaBlendPass()	{	return m_nBackBufferTextureBeforeAlphaBlendPass;	}

	void		SetExtraFilter(eFILTER_TYPE eFilter)
	{
		m_eExtraFilter = eFilter;
	}

#ifdef _SHOW_DEBUG_TEXTURE_
	void		EnableShowDebugTexture(BOOL bShow)	{	m_bShowDebugTexture = bShow;	}
#endif
protected:
	void		Clear();


	BOOL		m_bEnableImageProcess;
	float		m_fSceneIntensity;
	float		m_fBlurIntensity;
	float		m_fGlowIntensity;
	float		m_fHighlightThreshold;

	// DOF Var
	int			m_nEnableDOF;
	float		m_fDOFFocus;
	float		m_fDOFFocusRangeFar;
	float		m_fDOFFocusRangeNear;
	float		m_fDOFFocusOutFar;
	float		m_fDOFFocusOutNear;
	// DOF Var~

	// Add Color In Scene Combine Filter
	D3DXVECTOR4	m_v4AddColor;


	int			m_nTextureIdCustomBlur;
	int			m_nTextureIdCustomBlur2;
	eFILTER_TYPE	m_eExtraFilter;

	CBsRenderTargetChain				m_RTChain1_4;
#ifdef _XBOX
	LPDIRECT3DSURFACE9					m_pRTSurface1_4;
//	int									m_nDepthTexture;
#endif

	std::vector<CBsImageProcessFilter*>	m_FilterList;
	int									m_nBackBufferTexture;
	int									m_nBackBufferTextureBeforeAlphaBlendPass;
	int									m_nBackBufferTextureExtraFilter;

#ifdef _SHOW_DEBUG_TEXTURE_
	BOOL	m_bShowDebugTexture;
	int		m_nDefaultMaterial;
#endif

#ifndef _XBOX
	LPDIRECT3DSURFACE9					m_pBackBufferSurface;
#endif
};