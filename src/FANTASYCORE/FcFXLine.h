#include "FcFXBase.h"


struct RazerVertex
{
	D3DXVECTOR3 vPoint;
	D3DXVECTOR3 vTangent;
	float		fGenTime, fV;
};


class CFcFXLineCurve : public CFcFXBase
{
protected:
	int m_nPoint;
	BOOL		m_bUpdated;
//	D3DXVECTOR3* m_pvPoint;
//	D3DXVECTOR3* m_pvTangent;

	float		m_fLineWidth;
	float		m_fEpsilon;
	float		m_fLineLength;

	int			m_nMaterialIndex;
	int			m_nTextureID;

	DWORD		m_dwStartTick;
//	DWORD		m_dwLastTick;
	


	RazerVertex* m_pVertexList;
	int			 m_nVertexCount;
	int			 m_nMAXVertexCount;
	int			 m_dwVBWriteState;


	int			 m_nSample;
	D3DXVECTOR3* m_pSampleT;
	D3DXVECTOR3* m_pSample;

	IDirect3DVertexDeclaration9*m_pVertexDecl;
	LPDIRECT3DVERTEXBUFFER9		m_pVB;
	std::string					m_strTexture;



	D3DXHANDLE					m_hVP;
	D3DXHANDLE					m_hTexture;
	D3DXHANDLE					m_hCurTime;
	D3DXHANDLE					m_hLife;
	D3DXHANDLE					m_hLifeRsq;
	D3DXHANDLE					m_hLineWidth;
	D3DXHANDLE					m_hCamPos;
	//  Factor

public :
	CFcFXLineCurve();
	virtual ~CFcFXLineCurve();
	void Process();
	void PreRender();
	void Render(C3DDevice *pDevice);

	void Initialize( int , float , float);
	void UpdatePoints(int nCount, D3DXMATRIX* pvMtx );

	
	int		ProcessMessage(int nCode,DWORD dwParam1=0, DWORD dwParam2=0,DWORD dwParam3=0);

	void	InitDeviceData() ;
	void	ReleaseDeviceData();
};



class CFcFXLinePalabola : public CFcFXLineCurve
{
	float		m_fLife;
	float		m_fReachTime;
	
public :
	CFcFXLinePalabola();
	void Initialize(D3DXVECTOR3* , D3DXVECTOR3* , D3DXVECTOR3* , int , float , float , float );
	int	 ProcessMessage(int nCode,DWORD dwParam1=0, DWORD dwParam2=0,DWORD dwParam3=0);

	void Process();
	void Render(C3DDevice *pDevice);
};