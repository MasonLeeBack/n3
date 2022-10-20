#include "FcFXBase.h"
#include "CrossVector.h"


class CCloudMeshScale : public TimeValueTable
{
public :
	CCloudMeshScale(float fStartValue);
};

class CFcFXWind : public CFcFXBase
{
	int m_nObjIdx[4];
//	D3DXVECTOR3 m_vPos;
	DWORD m_dwStartTick;
	float m_fFrame;
	CCrossVector m_Cross;
	D3DXMATRIX	 m_Matrix;

	CCloudMeshScale *m_ScaleTable;

	float	m_fScaleX, m_fScaleY;
	int		m_nAniType;
public:
	CFcFXWind();
	~CFcFXWind();

	void PreRender();
	void InitDeviceData();
	void ReleaseDeviceData();
	void Process();
	int ProcessMessage(int nCode,DWORD dwParam1=0, DWORD dwParam2=0,DWORD dwParam3=0);
	void Update();

	std::vector<int>				m_vecSkinList;
	std::vector<int>				m_vecAniList;
};

/*
class CFcWindTrail
{
	int m_nTrail;
	int m_nTrailStep;

	int*	m_nTrailFx;
	D3DXVECTOR3* m_pVertex;

public:
	CFcWindTrail();
	Virtual ~CFcWindTrail();

	void Init()
	{
		float fRadius = 100.f;
		
		m_nTrailFx = new int[m_nTrail];
		m_pVertex = new D3DXVECTOR3[m_nTrail*m_nTrailStep];
		
		D3DXVECTOR3 vTap;

		for(int ii = 0; ii <  m_nTrail ; ++ii)
		{
			
			m_pVertex[ii*m_nTrailStep] = D3DXVECTOR3(0.f, 0.f, -100.f);
			m_pVertex[ii*m_nTrailStep+m_nTrail-1] = D3DXVECTOR3( sinf(6.28f*ii/m_nTrail) , cosf(6.28f*ii/m_nTrail), 0.f);
			vTap = m_pVertex[ii*m_nTrailStep+m_nTrail-1] - m_pVertex[ii*m_nTrailStep];
			vTap /= m_nTrailStep;


			for(int ij = 1; ij < m_nTrailStep-1; ij++)
			{
			}
		}
	}


	void Process()
	{

	}


	void Render()
	{

	}


};*/