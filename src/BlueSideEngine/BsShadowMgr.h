#pragma once

class CBsHFWorld;

class CBsShadowMgr
{
public:
	CBsShadowMgr();
	virtual ~CBsShadowMgr();

	void Create();
	void Reload();

	void Update(CBsCamera* pCamera);
	HRESULT RenderShadowMap(C3DDevice* pDevice);
protected:
	D3DVIEWPORT9 m_ShadowViewport;
	int			m_nShadowTexXSize;
	int			m_nShadowTexYSize;

	D3DXVECTOR3	m_vecLightDir;

	int			m_nVertexDeclIndices[BS_MAX_BONELINK_TECHNIQUE];
	int			m_nInstancingVertexDeclIndex;
	int			m_nShadowMaterialIndex;

	bool		m_bIsShadowPass;

	std::vector<CBsObject*>		m_ShadowCasterObjects;
	Box3		m_LightBox;

	// Transforms
	D3DXMATRIX m_LightViewProj;
	D3DXMATRIX m_LightViewProjForSampling;
	D3DXMATRIX m_LightViewProjNoView;
	D3DXMATRIX m_LightViewProjNoViewForSampling;

	LPDIRECT3DSURFACE9 m_pSMColorSurface, m_pSMZSurface;
	LPDIRECT3DTEXTURE9 m_pSMColorTexture, m_pSMZTexture;

	void ComputeLightMatrix();
	bool IsInLightBox(CBsObject* pObject);
    
	void SetShadowPass(bool bIsShadowPass)	{	m_bIsShadowPass = bIsShadowPass;	}
public:
	static int s_ShadowBufferWidth;
	static int s_ShadowBufferHeight;
	LPDIRECT3DTEXTURE9 GetShadowMap()	{	return m_pSMColorTexture;	}
	CBsMaterial* GetMaterial();
	
	void SetVertexDeclaration(int nBoneLinkCount);
	void SetVertexDeclarationInstancing();

	D3DXVECTOR4 GetLightDirection() { return D3DXVECTOR4(m_vecLightDir.x, m_vecLightDir.y, m_vecLightDir.z, 0.f); }
	
	bool				IsShadowPass()		{	return m_bIsShadowPass;	}	

	const D3DXMATRIX*	GetLightViewProject()		{	return &m_LightViewProj;	}
	const D3DXMATRIX*	GetLightViewProjectForSampling()	{	return &m_LightViewProjForSampling;	}
	const D3DXMATRIX*	GetLightViewProjectNoView()	{	return &m_LightViewProjNoView;	}
	const D3DXMATRIX*	GetLightViewProjectNoViewForSampling()	{	return &m_LightViewProjNoViewForSampling;	}
	///////////////////////////////////////////////////// for test
	int	GetShadowMapXSize()		{	return m_nShadowTexXSize;	}
	int	GetShadowMapYSize()		{	return m_nShadowTexYSize;	}
};