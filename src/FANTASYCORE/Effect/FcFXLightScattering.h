#pragma once
#include "FcFxBase.h"

/*
	숲에서 빛새어나오는 효과 .. by realgaia
*/

struct LIGHT_SCATTER_VERTEX
{
	D3DXVECTOR3 pos;
	D3DXVECTOR2 uv;
};

struct LIGHT_RAY_INFO
{
	int nNum;
	int nTexture[20];
	BSVECTOR Scale[20];
    BSVECTOR Direction[20];
	BSVECTOR Position[20];
	AABB BoundingBox;
	bool bVisible;
};

class CFcFXLightScattering : public CFcFXBase
{
public:
	CFcFXLightScattering();
	virtual ~CFcFXLightScattering();
	
protected:

	int		m_nMaterialIndex;
	int		m_nVertexDeclIndex;
	int		m_nTextureIndex[4];
	
	LPDIRECT3DVERTEXBUFFER9	m_pVB;

	D3DXHANDLE	m_hWorld;
	D3DXHANDLE	m_hTexture;		
	bool	m_bPholyaEnable;

	std::vector< LIGHT_RAY_INFO > m_vecLightInfo;


	std::vector< BSVECTOR > m_PholyaLayList;

	bool	m_bInitialize;

protected:
	void	AddRayInfo( int dwParam1, int dwParam2, int dwParam3 );
		 
public:
	
	void	Initialize();	
	void	InitDeviceData();
	void	ReleaseDeviceData();	
	void	Process();
	void	PreRender();
	void	Render(C3DDevice *pDevice);
	int		ProcessMessage(int nCode,DWORD dwParam1, DWORD dwParam2,DWORD dwParam3);
};
