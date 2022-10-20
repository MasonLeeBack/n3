#include "FcFXBase.h"

class CFcFXEarthExplode :  public CFcFXBase
{ 
	int* m_pnObjEngineIndex;
	int	m_nMaterialIndex;
	int m_nTextureId;
//	int m_nTextureId2;
//	int m_nNoiseTextureId;
	DWORD* m_pdwKeyTable;

	int			m_nVertexDeclIdx;
	LPDIRECT3DINDEXBUFFER9  m_pIB;

	int			m_nCurKeyIdx;
	float		m_fInterpolate;
	int			m_nMaxKeyTable;
	int			m_nStartTick;
	D3DXVECTOR3 m_vPos;


	//POINT Sprite
	D3DXHANDLE		m_hWVP;
	D3DXHANDLE		m_hW;
	D3DXHANDLE		m_hP;
	D3DXHANDLE		m_hV;
	D3DXHANDLE		m_hTexture;
	D3DXHANDLE		m_hInterpolate;

	float			m_fScale;
	DWORD			m_dwCurFrame;

public :
	CFcFXEarthExplode();
	virtual ~CFcFXEarthExplode();

	void Initialize();
	void Process();
	void Render(C3DDevice *pDevice);
	void InitDeviceData();
	void ReleaseDeviceData();

	void FindKeyFrame();
	int ProcessMessage(int nCode,DWORD dwParam1=0, DWORD dwParam2=0,DWORD dwParam3=0);
	;
};


	

class CEarthLerper : public TimeValueTable
{
public :
	CEarthLerper(float fStartValue);
};



class CFcFXEarth : public CFcFXBase
{
	int m_nObjIdx[20];
	D3DXVECTOR3 m_vPos;
//	float m_fFrame;
	DWORD m_dwTick;
	D3DXMATRIX m_Matrix;


	CEarthLerper				*m_pEarthLerper;
public:
	CFcFXEarth();
	~CFcFXEarth();

	void PreRender();
	void Initialize();
	void InitDeviceData();
	void ReleaseDeviceData();
	void Process();
	int ProcessMessage(int nCode,DWORD dwParam1=0, DWORD dwParam2=0,DWORD dwParam3=0);
	void Update();
	std::vector<int>				m_vecSkinList;
};






















