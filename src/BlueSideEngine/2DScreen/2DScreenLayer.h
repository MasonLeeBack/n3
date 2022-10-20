#ifndef __2D_SCREEN_LAYER__
#define	__2D_SCREEN_LAYER__

#include "BsKernel.h"

//------------------------------------------------------------------------------------------------

extern IDirect3DTexture9 *g_pOriginalBackBuffer; //$

#define		BACKBUFFER_TEXTURE_SIZE		256

#define		SCREEN_LAYER_PARAM_MAX		4

#define		SL_TYPE_NORMAL				0
#define		SL_TYPE_BACK_BUFFER			1
#define		SL_TYPE_RTT					2

#define		BLEND_MODE_NORMAL			0
#define		BLEND_MODE_ADD				1
#define		BLEND_MODE_REVSUBTRACT		2

//------------------------------------------------------------------------------------------------

struct COLORVERTEX
{
	D3DXVECTOR3 Vertex;
	DWORD dwColor;
};
#define D3DFVF_COLORVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)

struct COLORTEXTUREVERTEX
{
	D3DXVECTOR3 Vertex;
	DWORD		dwColor;
	float		fTextureU;
	float		fTextureV;
};
#define	D3DFVF_COLORTEXTUREVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)

//------------------------------------------------------------------------------------------------

class	C2DScreenLayer
{
public:

	static	void	CreateBackBufferSurface();
	static	void	PreprocessScreenLayers();
	static	void	ProcessScreenLayers();
	static	void	PostprocessScreenLayers();
	static  void	DrawScreenLayersBeforeFX();
	static	void	DrawScreenLayersBeforeCapture();
	static	void	DrawScreenLayersAfterCapture();
	
	static	void	CheckLayerUndeleted();
	static	C2DScreenLayer*	GetLayer(int nId);
	
	C2DScreenLayer(CBsKernel *pKernel, char *pTextureFileName, bool bAlpha, int nId); // SL_TYPE_NORMAL with alpha
	C2DScreenLayer(CBsKernel *pKernel, char *pTextureFileName, int nId);	//	SL_TYPE_NORMAL
	C2DScreenLayer(CBsKernel *pKernel, int nId);							//	SL_TYPE_BACK_BUFFER
	C2DScreenLayer(CBsKernel *pKernel, int nWidth, int nHeight,int nId);	//	SL_TYPE_RTT

	~C2DScreenLayer();

	void GetScreenSize();

	void	FillRect(DWORD dwColor, float L, float T, float R, float B);

	void			SetVParam(int nId, D3DXVECTOR3	*pParam);
	void			SetFParam(int nId, float fValue);
	void			SetNParam(int nId, int iValue);

	D3DXVECTOR3		*GetVParam(int nId);
	float			GetFParam(int nId);
	int				GetNParam(int nId);


	void			ForceHide(bool bOpt) {m_bForceHide = bOpt;}	// 스크린 캡춰용으로 레이어 숨길 때만 사용. 어으으
	void			Show(bool bOpt, int nStep = 0);
	bool			IsShowing(void);

	void			SetBackBufferCapture(int nLayerId);	// 몇번 layer 앞에서 back buffer를 캡춰할 것인지 지정한다.
	static void		SetBackBufferAsTexture(int nStage = 0);

	void			EnableZ(float fNear, float fFar);
	void			DisableZ(void);

	unsigned char	m_ColorR;
	unsigned char	m_ColorG;
	unsigned char	m_ColorB;
	unsigned char	m_ColorA;

	int				GetScreenWidth(void)	{ return m_nScreenWidth; }
	int				GetScreenHeight(void)	{ return m_nScreenHeight; }

	void	FillUpScreen(DWORD dwColor, float fU1, float fV1, float fU2, float fV2);
	void	FillUpScreen(float fX, float fY, float fZ, DWORD dwColor, float fU1, float fV1, float fU2, float fV2);

protected:

	void	NewLayer(int nId);

	void	SetLayerTexture(int nTexId);
	
	int 	LoadLayerTexture(char *pFileName, bool bAlpha);
	int 	LoadLayerTexture(char *pFileName);

	
	void	UnloadLayerTexture(int nTexId);

	void	GetLayerTextureSize(int nTexId, int *pWidth, int *pHeight);

	

	void	DrawUVMesh(D3DXVECTOR3 center, DWORD dwColor, float	fScaleX, float fScaleY, int nU1, int nV1, int nU2, int nV2, int mapSizeX, int mapSizeY);
	void	DrawUVMesh(D3DXVECTOR3 center, DWORD dwColor, float fRot, float	fScaleX, float fScaleY, int nU1, int nV1, int nU2, int nV2, int mapSizeX, int mapSizeY);
	void	DrawUVMesh(D3DXVECTOR3 center, DWORD dwColor, int width, int height, int nU1, int nV1, int nU2, int nV2, int mapSizeX, int mapSizeY);

	void	DrawUVMesh(D3DXVECTOR3 center, DWORD dwColor, int width, int height, float fRot, float fU1, float fV1, float fU2, float fV2);
	void	DrawUVMesh(D3DXVECTOR3 center, DWORD dwColor, int width, int height, float fU1, float fV1, float fU2, float fV2);
	
	void	DrawUVMesh(D3DXVECTOR3* pLT, D3DXVECTOR3 *pRB, DWORD dwColorLT, DWORD dwColorRT, DWORD dwColorLB, DWORD dwColorRB, float fU1, float fV1, float fU2, float fV2);

	void	FillTriangle(DWORD dwColor1, DWORD dwColor2, DWORD dwColor3, float x1, float y1, float x2, float y2, float x3, float y3);
	void	FillTriangle(DWORD dwColor, float x1, float y1, float x2, float y2, float x3, float y3);//do-g
	void	FillRect2(DWORD dwColorT, DWORD dwColorB, float L, float T, float R, float B);
	void	FillRect3(DWORD dwColorL, DWORD dwColorR, float L, float T, float R, float B);

	static	void	GetBackBufferToTexture(void);

	void			Draw(void);
	void			RTT(void);

	virtual	void	Preprocess(void);
	virtual	void	Process(void);
	virtual	void	Postprocess(void);
	virtual	void	UserDraw(void);
	virtual	void	UserRTT(void);

	void			SetBlendMode(int nBlendMode);
	void			SaveBlendMode(void);
	void			RestoreBlendMode(void);

	int			m_nId;
	int			m_nType;
	int			m_nWidth, m_nHeight;
	bool		m_bForceHide;
	bool		m_bShow;
	int			m_nShowCount;
	int			m_nShowStep;
	bool		m_bNextShow;
	char		m_TextureFileName[MAX_PATH];

	int			m_nShowAlpha;
	bool		m_bZEnabled;
	
	int			m_nTexId;

	//RTT에서만 쓰인다... 임시..
	LPDIRECT3DTEXTURE9	m_pTexture;	//$
	LPDIRECT3DSURFACE9	m_pSurface;


	D3DXVECTOR3	m_vParam[SCREEN_LAYER_PARAM_MAX];
	float		m_fParam[SCREEN_LAYER_PARAM_MAX];
	int			m_nParam[SCREEN_LAYER_PARAM_MAX];


	typedef	struct
	{
		char				m_TextureFileName[MAX_PATH];
		int					m_nWidth,m_nHeight;
		LPDIRECT3DTEXTURE9	m_pTexture;	//$
		LPDIRECT3DSURFACE9	m_pSurface;
		int					m_nRefCount;
		bool				m_bAlpha;
	} _TextureInfo;

	static	int				m_nLayerTable;
	static	C2DScreenLayer	**m_ppLayerTable;

	//static	int				m_nTextureInfo;
	//static	_TextureInfo	**m_ppTextureInfo;
	
	static	CBsKernel			*m_pKernel;
	
	static	bool				m_bBackBufferIsVirgin;
	static	int					m_nBackBufferCapture;

	D3DXMATRIX				m_matSavedProj;

	int						m_nBlendMode;	// 0: normal, 1: add
	int						m_nSavedBlendMode;

	static int m_nScreenWidth;
	static int m_nScreenHeight;
};

//------------------------------------------------------------------------------------------------

#endif

