#include "FcFXBase.h"


struct RazerVertex
{
	D3DXVECTOR3 vPoint;
	D3DXVECTOR3 vTangent;
	float		fGenTime, fV;
};

struct sLineCurveUpdate
{
	int _nCount;
	D3DXVECTOR3* _vPos;
	sLineCurveUpdate()
	{
		memset(this, 0, sizeof(sLineCurveUpdate) );
	}
};

class CFcFXLineCurve : public CFcFXBase
{
protected:
	int			m_nPoint;
	BOOL		m_bUpdated;
//	D3DXVECTOR3* m_pvPoint;
//	D3DXVECTOR3* m_pvTangent;

	float		m_fLineWidth;
	float		m_fEpsilon;
	float		m_fLineLength;
	

	int			m_nMaterialIndex;
	int			m_nTextureID;

	DWORD		m_dwStartTick;
	bool		m_bIsVolumeTex;
	bool		m_bIsOrb;
	bool		m_bOnceTexCoord;
	RazerVertex* m_pVertexList;
	int			 m_nVertexCount;
	int			 m_nMAXVertexCount;
//	int			 m_dwVBWriteState;

	int			 m_nSample;
	D3DXVECTOR3* m_pSampleT;
	D3DXVECTOR3* m_pSample;

	int			m_nVertexDeclIdx;
//	LPDIRECT3DVERTEXBUFFER9		m_pVB;
	//std::string					m_strTexture;
	char					m_strTexture[128];


	DWORD						m_dwAlphaBlendOP;
	DWORD						m_dwSRCBlend;
	DWORD						m_dwDSTBlend;
	
	float						m_fTexAniSpd;
	float						m_fAlpha;

//	D3DXHANDLE					m_hVP;
	D3DXHANDLE					m_hTexture;
	D3DXHANDLE					m_hCurTime;
	D3DXHANDLE					m_hLife;
	D3DXHANDLE					m_hLifeRsq;
	D3DXHANDLE					m_hLineWidth;
	D3DXHANDLE					m_hCamPos;

	D3DXHANDLE					m_hTexAniSpd;
	D3DXHANDLE					m_hAlpha;
	std::vector<sLineCurveUpdate*>	m_vecUpdateCurve[2];
	//  Factor

public :
	CFcFXLineCurve();
	virtual ~CFcFXLineCurve();
	void Process();
	void PreRender();
	void Render(C3DDevice *pDevice);

	void Initialize( int , float , float);
	void UpdatePoints(int nCount, D3DXVECTOR3* pVec );

	
	int		ProcessMessage(int nCode,DWORD dwParam1=0, DWORD dwParam2=0,DWORD dwParam3=0);

	void	InitDeviceData() ;
	void	ReleaseDeviceData();
};





class CFcFXLineTrail : public CFcFXLineCurve
{
	float	m_fLife;
	DWORD	m_dwEndTime;
	

	int		m_nQueueBeginIdx;
	int		m_nQueueEndIdx;

//	LPDIRECT3DVERTEXBUFFER9		m_pVB2;
//	BOOL	m_bVBSwitch;
	DWORD	m_dwLastUpdateTick;
public:
	CFcFXLineTrail();

	void Initialize( int nLife, float fWidth, float fEpsilon);

	void Process() {};
	void PreRender();
	void Render(C3DDevice *pDevice);
	int	 ProcessMessage(int nCode,DWORD dwParam1=0, DWORD dwParam2=0,DWORD dwParam3=0);

	void UpdatePoint( D3DXVECTOR3* vPos, D3DXVECTOR3* vTngt);


	void	InitDeviceData() ;
	void	ReleaseDeviceData();
};





struct FXBBoardVtx
{
	D3DXVECTOR3 vPos;
	D3DXVECTOR3 vTexcoord;
	FXBBoardVtx()
	{
		memset(this, 0, sizeof(FXBBoardVtx));
	}
};

class CFcFXBillboard : public CFcFXBase
{
	//LPDIRECT3DVERTEXBUFFER9		 m_pVB;
	FXBBoardVtx*				m_pBBoard;
	char					 m_strTexture[128];

	int				m_nVertexDeclIdx;
	int				m_nTextureID;
	int				m_nMaterialIndex;
	int				m_nVBWriteIdx;
	int				m_nMaxBillboard;
	int				m_nCurBoardCount;


	DWORD			m_dwAlphaBlendOP;
	DWORD			m_dwSRCBlend;
	DWORD			m_dwDSTBlend;
	DWORD			m_dwStartTick;

	float			m_fBillboardSize;
	float			m_fBillboardLife;

	int				m_nTmpBuffer;
	D3DXVECTOR3		m_vTmpBuffer[100];
	float			m_fAlpha;

//	D3DXHANDLE		m_hVP;
	D3DXHANDLE		m_hTexture;
	D3DXHANDLE		m_hCurTime;
	D3DXHANDLE		m_hLife;
	D3DXHANDLE		m_hLifeRsq;
	D3DXHANDLE		m_hRadius;
	D3DXHANDLE		m_hCamAxisX;
	D3DXHANDLE		m_hCamAxisY;
	D3DXHANDLE		m_hAlpha;

	std::vector<D3DXVECTOR3>	m_vecAddBillborad[2];

public :
	CFcFXBillboard();
	virtual ~CFcFXBillboard();

	void	Process();
	void	PreRender();
	void	Render(C3DDevice *pDevice);
	void	Initialize(int nMaxBillboard, float fSize, float fLife );
	void	AddBillboard(D3DXVECTOR3* vPos);
	int		ProcessMessage(int nCode,DWORD dwParam1=0, DWORD dwParam2=0,DWORD dwParam3=0);
	void	InitDeviceData();
	void	ReleaseDeviceData();
};