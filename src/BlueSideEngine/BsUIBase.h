#pragma once

#include "3DDevice.h"
#include "Singleton.h"

//------------------------------------------------------------------------------------------------

static inline D3DXVECTOR4 GetVec4Color( D3DCOLORVALUE clr )
{ 
	D3DXVECTOR4 vec4Color;
	vec4Color = D3DXVECTOR4(clr.r, clr.g, clr.b, clr.a);
	return vec4Color;
}

static inline DWORD GetdwColor( D3DXCOLOR clr )
{ 
	int a = (int)(clr.a * 255);
	int r = (int)(clr.r * 255);
	int g = (int)(clr.g * 255);
	int b = (int)(clr.b * 255);

	DWORD dwColor = a<<24 | r<<16 | g<<8 | b;
	return dwColor;
}

class CBsUIElement 
{
public:
	struct UIVERTEX	// fx 에서 사용될 입력 포맷 입니다 (20 바이트)
	{
		D3DXVECTOR3 vecPos;
		D3DXVECTOR2 vecUV;
		DWORD		dwColor;
	};

	CBsUIElement(int mode,
		int x1, int y1, D3DXCOLOR color1,
		int x2, int y2, D3DXCOLOR color2,
		int x3, int y3, D3DXCOLOR color3,
		int x4, int y4, D3DXCOLOR color4,
		float fz, float fRot,
		int nTextureID,
		int ux1, int uy1, int ux2, int uy2,
		int m_nPosColorType, char* szText,
		int	nDummyTexId,
		bool bRTT,
		int nMeshIndex,
		int nViewId,
		int nExceptionType);

	~CBsUIElement() { Clear(); }
	
    void Clear();

public:
	int		m_nMode; // 드로잉 모드 

	int		m_nX1, m_nY1;
	int		m_nX2, m_nY2;
	int		m_nX3, m_nY3;
	int		m_nX4, m_nY4;

	D3DXCOLOR m_Color1;
	D3DXCOLOR m_Color2;
	D3DXCOLOR m_Color3;
	D3DXCOLOR m_Color4;

	float	m_fZ;				//deep

	int     m_nTexId;
	int		m_ux1, m_uy1, m_ux2, m_uy2;
	
	float	m_fRot;
	
	char	*m_pText;

	int		m_nPosColorType;
	int		m_nViewId;				// 그려질 Viewport 인덱스를 보관하고 있습니다
	int		m_nDummyTexId;			// 일단은 alpha map

	bool	m_bRTT;

	int		m_nMeshIndex;			// Mesh Index
	int		m_nExceptionType;
};

//------------------------------------------------------------------------------------------------
typedef std::vector<CBsUIElement*>	uiElementLIST;

class CBsUIManager : public CSingleton<CBsUIManager> // 커널에서 m_pUIManager 를 할당하여 관리합니다 
{
public:
	CBsUIManager();
   ~CBsUIManager();
	
	void ReInitialize();

	bool CreateElement(int mode, 
		int x1, int y1, D3DXCOLOR color1,
		int x2, int y2, D3DXCOLOR color2,
		int x3, int y3, D3DXCOLOR color3,
		int x4, int y4, D3DXCOLOR color4,
		float fz, float fRot = 0.f,
		int nTextureID = -1,
		int ux1 = 0, int uy1 = 0, int ux2 = 0, int uy2 = 0,
		int nPosColorType = 0, char *szText = NULL,
		int nDummyTexId = -1,
		bool bRTT = false,
		int nMeshIndex = -1);

	//---------------------------------------------------------
	bool CreateElementDebug(int x1, int y1, D3DXCOLOR color, char *szText);

	bool CreateElementMesh(BS_UI_MODE mode, int nMeshIndex, int nX, int nY, D3DXCOLOR color, int nTexId,
		float param1 = 0.f, float param2 = 0.f, float param3 = 0.f, float param4 = 0.f);

	bool CreateElement_Pos2(int mode,
		int x1, int y1, int x2, int y2,
		float fz, D3DXCOLOR color,
		float fRot = 0.f,
		int nTextureID = -1,
		int ux1 = 0.f, int uy1 = 0.f,
		int ux2 = 0.f, int uy2 = 0.f,
		int nDummyTexId = -1,
		bool bRTT = false);

	bool CreateElement_Pos4(int mode, 
		int x1, int y1, int x2, int y2,
		int x3, int y3, int x4, int y4,
		float fz, D3DXCOLOR color,
		int nTextureID = -1,
		int ux1 = 0.f, int uy1 = 0.f,
		int ux2 = 0.f, int uy2 = 0.f,
		int nDummyTexId = -1,
		bool bRTT = false);

	bool CreateElement_PosColor4(int mode, 
		int x1, int y1, D3DXCOLOR color1,
		int x2, int y2, D3DXCOLOR color2,
		int x3, int y3, D3DXCOLOR color3,
		int x4, int y4, D3DXCOLOR color4,
		float fz,
		int nTextureID = -1,
		int ux1 = 0.f, int uy1 = 0.f,
		int ux2 = 0.f, int uy2 = 0.f,
		int nDummyTexId = -1,
		bool bRTT = false);
	
	//---------------------------------------------------------
	void Render(C3DDevice *pDevice);
	void RenderBGMovie(C3DDevice *pDevice);
	void RenderException(C3DDevice *pDevice, int nExceptionType);
	void SetUIException(int nExceptionType)		{ m_nExceptionType = nExceptionType; }

	void ClearElementAll(int nIndex);
	void ClearElementBGMovie(int nIndex);
	void ClearElementException(int nIndex);

	void SetViewportIndex(int nIndex)		{ m_nViewerIndex = nIndex; }

	static int GetRenderBufferIndex() { return m_sCurrentRenderBuffer; }
	static int GetProcessBufferIndex() { return m_sCurrentProcessBuffer; }
#ifdef _XBOX
	static void FlipRenderBuffer() { m_sCurrentRenderBuffer = 1 - m_sCurrentRenderBuffer;}
	static void FlipProcessBuffer() { m_sCurrentProcessBuffer = 1 - m_sCurrentProcessBuffer;}
#else
	static void FlipRenderBuffer() {}
	static void FlipProcessBuffer() {}
#endif
	static void ResetDoubleBuffer()
	{
		m_sCurrentRenderBuffer = 0;
		m_sCurrentProcessBuffer = 0;
	}


protected:
	void InitVertexShader();
	void SetupShader0();
	void SetupShader1();
	void SetuoDecl();
	void Release();
	void CreateVertexPool();

	void Clear(); // 소거시 자동으로 호출 됩니다
	

	void SetViewport(int nViewIndex, float fX, float fY, float fWidth, float fHeight);
	D3DVIEWPORT9*	GetViewport(int nViewIndex);
	
	void RenderElement(C3DDevice *pDevice, int nViewIndex, uiElementLIST* pElementList);

	void RenderObjectUp(C3DDevice *pDevice, uiElementLIST* pElementList);
	
	void UpdateVertex(uiElementLIST* pElementList, CBsUIElement::UIVERTEX* puiv);
	void UpdateIndex(int nCount, WORD *pIndex);

	void DrawIndexedPrimitiveUP(C3DDevice *pDevice, int nCount);

	void RenderUIMesh(C3DDevice *pDevice, CBsUIElement* pElment);


protected:
	//shader
	int							m_nMaterialIndex0;
	D3DXHANDLE					m_hLayerTexture;
	D3DXHANDLE					m_hDummyTexture;
	D3DXHANDLE					m_hVolumnTexture;
	D3DXHANDLE					m_hInputTime;
	D3DXHANDLE					m_hInputTileX;
	D3DXHANDLE					m_hInputTileY;
	D3DXHANDLE					m_hInput_X;
	D3DXHANDLE					m_hInput_Y;
	D3DXHANDLE					m_hInputClickTime;


	int							m_nMaterialIndex1;
	D3DXHANDLE					m_hMeshTex;
	D3DXHANDLE					m_hMeshVolTex;
	D3DXHANDLE					m_hMesh_X;
	D3DXHANDLE					m_hMesh_Y;
	D3DXHANDLE					m_hMeshset0;
	D3DXHANDLE					m_hMeshset1;
	D3DXHANDLE					m_hMeshset2;
	D3DXHANDLE					m_hMeshset3;
	D3DXHANDLE					m_hMeshColor;

	int							m_nLayerTexWave;
	int							m_nLayerTexLight;
	
	int							m_nUIVertexDeclIndex;
	int							m_nUIMeshVertexDeclIndex;

	std::vector<CBsUIElement *> m_EListFullMode[2];
	std::vector<CBsUIElement *> m_EList1PMode[2];
	std::vector<CBsUIElement *> m_EList2PMode[2];
	std::vector<CBsUIElement *>	m_EListforBGMovie[2];		//특수 처리
	std::vector<CBsUIElement *>	m_EListforException[2];		//특수 처리

	// vectex buffer를 하나로 유지하기 위해 사용 됩니다
	CBsUIElement::UIVERTEX*		m_puiv;
	WORD*						m_pIndex;

	//전체, 1P, 2P
	int			 m_nViewerIndex;
	D3DVIEWPORT9 m_viewport;
	D3DVIEWPORT9 m_viewport1P;
	D3DVIEWPORT9 m_viewport2P;

	int				m_nExceptionType;

	static int					m_sCurrentRenderBuffer;
	static int					m_sCurrentProcessBuffer;	
};

