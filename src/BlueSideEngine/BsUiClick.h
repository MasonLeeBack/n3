#pragma once

#include "3DDevice.h"

struct UiClickElement{
	int		nX, nY;
	int		nSizeX, nSizeY;
	int		nStartTick;
	int		nFrame;
	int		nSpeed;
	float	fAlpha;
};

struct CLICKVERTEX	// fx 에서 사용될 입력 포맷 입니다 (20 바이트)
{
	D3DXVECTOR3 vecPos;
	D3DXVECTOR2 vecUV;
	float		frame;
	float		fAlpha;
};

typedef std::vector<UiClickElement*>	uiClickElementLIST;


class CBsUiClickTexture
{
public:
	CBsUiClickTexture();
	~CBsUiClickTexture(){Release();}

	void Create();
		
	void Render(C3DDevice *pDevice);

	void AddWave(int x, int y, int size, int nSpeed);

	int  GetWaveCount();

	int  GetWaveFrame();
	int  GetLastWaveRemainFrame();

protected:
	void InitVertexShader();
	void SetupVertexShader();
	void CreateVertexPool();

	void Release();

	void UpdateVertex(CLICKVERTEX* puiv, UiClickElement* pElement);
	void UpdateIndex(int nCount, WORD *pIndex);

	void RenderObjectUp(C3DDevice *pDevice, UiClickElement* pElement);
	void RenderProcess();

protected:
	int				m_nMaterialIndex;

	D3DXHANDLE		m_hLayerTexture;
	D3DXHANDLE		m_hWaveTexture;
	
	int				m_nUIClickVertexDeclIndex;

	D3DVIEWPORT9				m_Viewport;
	LPDIRECT3DVERTEXBUFFER9		m_pVB;
	LPDIRECT3DINDEXBUFFER9		m_pIB;
	CLICKVERTEX*				m_puiv;
	WORD*						m_pIndex;

	uiClickElementLIST			m_pElementList;
	int							m_nWaveTexId;
	int							m_nBackBufferTexId;
	int							m_nRenderTick;
};