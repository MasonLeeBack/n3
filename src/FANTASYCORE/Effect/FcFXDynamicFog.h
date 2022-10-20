#pragma once
#include "FcFxBase.h"

#define DYNAMIC_FOG_SIZE 600.0f
#define MAX_FOG_BILLBOARD	1000

struct FOGVERTEX
{
	D3DXVECTOR3 vecWorldPos;
	D3DXVECTOR2 vecOffset;
	D3DXVECTOR2	vecTexCoord;
};

#define FOG_PARTICLE_START_UP	-1000.f
#define FOG_PARTICLE_ALPHA		0.14f

class CFcFXFog : public CFcFXBase
{
public:
	CFcFXFog();
	virtual ~CFcFXFog();

	void	Initialize(D3DXVECTOR2* pAreaSize, D3DXVECTOR2* pStartPos, char* pszTextureName);
	void	InitDeviceData();
	void	ReleaseDeviceData();

	void	Process();
	void	PeRender();
	void	Render(C3DDevice *pDevice);
	int		ProcessMessage(int nCode,DWORD dwParam1, DWORD dwParam2,DWORD dwParam3);

protected:
	LPDIRECT3DVERTEXBUFFER9	m_pVB;

	char		m_szTextureName[128];
	
	int			m_nVertexDeclIndex;
	int			m_nMaterialIndex;
	int			m_nTextureIndex;

	D3DXHANDLE		m_hTexture;
	D3DXHANDLE		m_hInverseView;
	D3DXHANDLE		m_hViewProjection;
	D3DXHANDLE		m_hParticleInfo;
	D3DXHANDLE		m_hFogColor;

	float			m_fStartTime;
	float			m_fElapsedTime;					// Rotate 관련해서 추가
	float			m_fFogSpd;						// 안개가 피어오르는 속도
	D3DXVECTOR2		m_ParticleInfo;					// x = y-axis offset, y = alpha

	D3DXVECTOR4	m_vecFogColor;
	D3DXVECTOR2	m_vecAreaSize;
	D3DXVECTOR2	m_vecStartPos;
};