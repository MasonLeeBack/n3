#ifndef _FCFXSIMPLETRAIL_H_
#define _FCFXSIMPLETRAIL_H_

#include "FcFXBase.h"

struct TRAIL_INFO
{
	D3DXVECTOR3 vDown;
	D3DXVECTOR3 vUp;
	float		fCreateTime;

	TRAIL_INFO()
	{
		ZeroMemory(this, sizeof(TRAIL_INFO) );
	}
};


struct TRAIL_VERTEX {
	D3DXVECTOR3 vPos;
//	D3DXVECTOR3 vColor;
	float fCreateTime, fTextureV;

	TRAIL_VERTEX()
	{
		ZeroMemory(this, sizeof(TRAIL_VERTEX) );
	}
};


const int c_TRAIL_MAX_POOL = 400;
const int c_TRAIL_MAX_INTERPOLATE = 40;
const int c_TRAIL_MAX_STEP			= 3;



// 시간은 초단위
class CFcFXSimpleTrail : public CFcFXBase
{

protected:
	int							m_nSampleTrailIndex;
	TRAIL_INFO					m_SampleTrail[10];

	TRAIL_INFO					m_NextSample;

	D3DXVECTOR4					m_TrailColor;
//	LPDIRECT3DVERTEXBUFFER9		m_pVB;
//	ID3DXEffect*				m_pEffect;
	int							m_nMaterialIndex;
	int							m_nPoolIdx; // VB Lock Pos;

	D3DXHANDLE					m_hVP;
	
	D3DXHANDLE					m_hNormalTexture;
	D3DXHANDLE					m_hEnvTexture;
	D3DXHANDLE					m_hDiffTexture;

	D3DXHANDLE					m_hCurTime;
	D3DXHANDLE					m_hLife;
	D3DXHANDLE					m_hLifeRsq;
	D3DXHANDLE					m_hColor;
	D3DXHANDLE					m_hAlpha;

	float						m_fLifeTime;
	float						m_fTrailEndTime;

	int							m_nTextureID;
	int							m_nOffsetTextureID;
	int							m_nRecentAccumCnt;
	int							m_nAdded;

	float						m_fCurTime;
	float						m_fStartTime;

	float						m_fLength;

	// TODO : 안쓰이는 Data인것 같은데, 확인해주시기 바랍니다. by jeremy
	std::string					m_strTexture;
	int*						m_pLoadedTexIDs;
	int							m_nLoadedTexCount;
	BOOL						m_bTextureChange;

	int							m_nDefTextureID;
	int							m_nDefOffsetTextureID;

	TRAIL_VERTEX				m_aBuffer[c_TRAIL_MAX_STEP*6*c_TRAIL_MAX_POOL]; 
	int							m_nBufferIdx;

	int							m_nVertexDeclIdx;
	float						m_fLastUpdateTime;

	DWORD						m_dwAlphaBlendOP;
	DWORD						m_dwSRCBlend;
	DWORD						m_dwDSTBlend;


	void Initialize();
	void SetColor(D3DXVECTOR3 &TrailColor) { m_TrailColor = D3DXVECTOR4(TrailColor.x, TrailColor.y, TrailColor.z, 1.0f); };
	void AddTrail(const D3DXVECTOR3* pPos);
	void StartTrail(const D3DXVECTOR3* pPos, float fLife);

	//void AddLerpTrace(int* pSample);
	void Add(int* pSample);
//	void AddCatmullromTrace(int* pSample);


public:
//	static int					_MAX_POOL;
//	static int					_MAX_INTERPOLATE;
//	static int					_MAX_STEP;

	CFcFXSimpleTrail();
	virtual ~CFcFXSimpleTrail();


	//Overide
	void Process();
	void PreRender();
	void Render(C3DDevice *pDevice);
	void RenderZero(C3DDevice *pDevice);
	int ProcessMessage(int nCode,DWORD dwParam1=0, DWORD dwParam2=0,DWORD dwParam3=0);
	void	InitDeviceData() ;
	void	ReleaseDeviceData();

	struct sAddTrail
	{
		D3DXVECTOR3 vPos[4];
		void Set( D3DXVECTOR3* v)
		{
			memcpy( vPos, v , sizeof(D3DXVECTOR3)*4);
		}
	};

	std::vector<sAddTrail>	m_vecAddTrailPos[2];
};


#endif