#pragma once
#include "FcFxBase.h"




struct DROP
{
private:
	WORD	wMoveTableIdxX, wMoveTableIdxY;
	POINT	ptMoveTableIdx;
	BOOL	bActive;
	float	fAccelX, fAccelY;
	int		nFormTime;

	static float DROP::_fMoveX[120];
	static float DROP::_fMoveY[100];
	
public:
	float	fx, fy;
	float	fSize;	// 반지름
	DROP()
	{
		ZeroMemory(this, sizeof(DROP) );
	}

	void Set(float fxIn, float fyIn, float fSizeIn, int nFormTimeIn);

	void Active()
	{
		bActive = TRUE;
	}

	void Inactive()
	{
		bActive = FALSE;
	}

	BOOL IsActive()
	{	
		return bActive;
	}

	int GetMoveTableIdx(BOOL bXaxis = FALSE)
	{
		if(bXaxis)
			return wMoveTableIdxX++;
		else
			return wMoveTableIdxY++;
	}


	void ResetMoveTableIdx(BOOL bXaxis = FALSE)
	{
		if(bXaxis)
			wMoveTableIdxX = 0;
		else
			wMoveTableIdxY = 0;
	}

	BOOL IsFormed() { return nFormTime ? TRUE : FALSE; }
	void ProcessFormed() { --nFormTime;	}
	void ProcessDown(float fdx, float fdy)
	{
		fx += fdx;
		fy += fdy;

		if(fy <= -1.f)
			Inactive();
	}

	void GetMoveDelta(float &fDxOut, float &fDyOut);
};

// 드랍추가
// 드랍 흐르기
// 버텍스 버퍼 구성하기
// 라인 그리기
struct DROPLINE_VERTEX 
{
	D3DXVECTOR2 vPos;
	D3DXVECTOR3	vTex;  // fRefract, fCreateTime;
//	float		fCreateTime;//, fTextureV;

	DROPLINE_VERTEX()
	{
		ZeroMemory(this, sizeof(DROPLINE_VERTEX) );
	}
};

struct DROP_VERTEX 
{
	D3DXVECTOR2 vPos;
	D3DXVECTOR2	vTex;
	
	DROP_VERTEX()
	{
		ZeroMemory(this, sizeof(DROP_VERTEX) );
	}
};




class CFcFXScreenWaterDrop : public CFcFXBase
{
	static int	_MAX_DROP;
	static int	_MAX_QUAD;

	std::vector<DROP>	m_Drops;
	float				m_fDropSize;
	float				m_fLifeTime;
	float				m_fCurTime;
	

	LPDIRECT3DVERTEXBUFFER9		m_pVBLine;
	int							m_nMaterialIndex;
	int							m_nPoolIdx; // VB Lock Pos;

	int							m_nTextureId;

	LPDIRECT3DVERTEXBUFFER9		m_pVBDrop;

	D3DXHANDLE					m_hConst;
	D3DXHANDLE					m_hTexture;
	D3DXHANDLE					m_hNormTexture;

	int							m_nGenOften;

	int							m_nLineVertexDeclIdx;
	int							m_nDropVertexDeclIdx;
public :
	CFcFXScreenWaterDrop();
	virtual ~CFcFXScreenWaterDrop();

	void	Initialize();

	//Overide
	void	Process();
	void	PreRender();
	void	Render(C3DDevice *pDevice);
	int		ProcessMessage(int nCode,DWORD dwParam1=0, DWORD dwParam2=0,DWORD dwParam3=0);
	void	InitDeviceData() ;
	void	ReleaseDeviceData();
	//Overide~

	void	GenDrop();
	int		GetInactiveDropIdx();
	BOOL	IsActiveAnyDrop();
	void	SetDropGenInfo( int nOften, float fMaxSize);
	void	InactiveAllDrop();
	int		GetActiveDropCount();

//	virtual void ProcessDrops();

};