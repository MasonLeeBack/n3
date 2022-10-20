#pragma once

#include "FcFxBase.h"

struct MOTIONBLUR_VERTEX 
{
	D3DXVECTOR2 vPos;
	D3DXVECTOR2	vTex;
	
	MOTIONBLUR_VERTEX()
	{
		ZeroMemory(this, sizeof(MOTIONBLUR_VERTEX) );
	}
};

// 화면 떨기
// 코드 정리
// 메세지 처리

class CFcFXScreenMotionBlur : public CFcFXBase
{
	LPDIRECT3DVERTEXBUFFER9		m_pVB;
	int							m_nMaterialIndex;


	float	m_fBlendFactor;
	float	m_fScaleFactor;

	int		m_nNoiseTextureId;
	int		m_nTableTextureId;
	D3DXHANDLE hBackTexture;
	D3DXHANDLE hNoiseTexture;
	D3DXHANDLE hTableTexture;
//	D3DXHANDLE hBlendFactor;
//	D3DXHANDLE hScaleFactor;
	D3DXHANDLE hColor;
	D3DXHANDLE hTime;
	D3DXHANDLE hConst;
	D3DXHANDLE hConst2;

	float m_fNoiseIntensity;
	float m_fLomoThick;
	float m_fNoisySmooth;
	float m_fSceneBlendRate;
	float m_fSpeed;

	int		m_nVertexDeclIdx;

	float	m_fRandom[2];
	int		m_nTechnique;

	D3DXVECTOR4	m_vColor;
public:

	CFcFXScreenMotionBlur();
	virtual ~CFcFXScreenMotionBlur();

	// Overide
	void InitDeviceData();
	void ReleaseDeviceData();
	void Process();
	void Render(C3DDevice *pDevice);
	int ProcessMessage(int nCode,DWORD dwParam1=0, DWORD dwParam2=0,DWORD dwParam3=0);


	void Initialize();
//	void SetBlurScale(float fScale);
};



class CFcFXXMB  : public CFcFXBase
{
	LPDIRECT3DVERTEXBUFFER9		m_pVB;
	int							m_nMaterialIndex;


	int			m_nRand[6];
	D3DXVECTOR3 m_vEmit;



	D3DXHANDLE hBackTexture;
	D3DXHANDLE hTime;

	int m_nVertexDeclIdx;

public:
	CFcFXXMB();
	virtual ~CFcFXXMB();


	void InitDeviceData();
	void ReleaseDeviceData();
	
	void Process();
	void Render(C3DDevice *pDevice);
	int ProcessMessage(int nCode,DWORD dwParam1=0, DWORD dwParam2=0,DWORD dwParam3=0);


	void Initialize();
};



class CFcFXInppyyBlur  : public CFcFXBase
{
/*	LPDIRECT3DVERTEXBUFFER9		m_pVB;
	IDirect3DVertexDeclaration9*m_pVertexDecl;
	int							m_nMaterialIndex;
	int							m_nTextureId;


//	int			m_nRand[6];
//	D3DXVECTOR3 m_vEmit;



	D3DXHANDLE hBackTexture;
	D3DXHANDLE hTexture;
	D3DXHANDLE hTime;*/

public:
	CFcFXInppyyBlur();
	virtual ~CFcFXInppyyBlur();

/*
	void InitDeviceData();
	void ReleaseDeviceData();
*/
	void Process();
	void Render(C3DDevice *pDevice);
	int ProcessMessage(int nCode,DWORD dwParam1=0, DWORD dwParam2=0,DWORD dwParam3=0);

	bool	m_bCustom2;


	void Initialize();
};





class CFcFXLomoFilter : public CFcFXBase
{
	LPDIRECT3DVERTEXBUFFER9		m_pVB;
	int							m_nMaterialIndex;

	int m_nTextureID;

	D3DXHANDLE			hTexture;
	D3DXHANDLE			hBlend;
	D3DXHANDLE			hConst;
	D3DXHANDLE			hRandom;

	int		m_nNoiseTextureId;
	D3DXHANDLE hTime;
	D3DXHANDLE hBackTexture;
	D3DXHANDLE hNoiseTexture;
//	D3DXHANDLE hTableTexture;


	int		m_nVertexDeclIdx;

	float m_fNoiseIntensity;
	float m_fLomoThick;
	float m_fNoisySmooth;
	float m_fSceneBlendRate;


	float m_fFactor;

public:

	CFcFXLomoFilter();
	virtual ~CFcFXLomoFilter() {};

	// Overide
	void InitDeviceData();
	void ReleaseDeviceData();
	void Process();
	void Render(C3DDevice *pDevice);
	int ProcessMessage(int nCode,DWORD dwParam1=0, DWORD dwParam2=0,DWORD dwParam3=0);

};


class CFcFX2DFSEffect : public CFcFXBase
{
	LPDIRECT3DVERTEXBUFFER9		m_pVB;
	int							m_nMaterialIndex;

	int		m_nTextureID;
	int		m_nVertexDeclIdx;
	DWORD	m_dwStartTick;
	

	float	m_fAlpha;
	int		m_nDevide;
	int		m_nInterval;

	D3DXHANDLE			hTexture;
	D3DXHANDLE			hFactor;

	int		m_nTechnique;

public:

	bool m_bEnvirEffect;
	bool GetEnvir2DEffect()				{ return m_bEnvirEffect;};
	void SetEnvir2DEffect( bool p_b )	{ m_bEnvirEffect = p_b; };

	CFcFX2DFSEffect();
	virtual ~CFcFX2DFSEffect() {};

	// Overide
	void InitDeviceData();
	void ReleaseDeviceData();
	void Process();
	void Render(C3DDevice *pDevice);
	int ProcessMessage(int nCode,DWORD dwParam1=0, DWORD dwParam2=0,DWORD dwParam3=0);

};