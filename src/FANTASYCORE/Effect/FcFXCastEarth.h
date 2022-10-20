#include "FcFXBase.h"
#include "CrossVector.h"
#include "CDefinition.h"

enum FLOATSTONE { STONE_READY, STONE_UP, STONE_FLOAT, STONE_HALE, STONE_RAIN, STONE_DOWN, STONE_END};


struct CUSTOMPOINTSPRITE
{
	D3DXVECTOR4		Pos;		// position 
	D3DXVECTOR3		normal;		// size, text depth, 
};
#define D3DFVF_CUSTOMPOINTSPRITE (D3DFVF_XYZW|D3DFVF_NORMAL)


/*-----------------------------------------------------------------------------------
-																					-
-		EarthCast	:	어스 옵스팤 시전 속성값 정의								-
-										2005.9.26 v0.02								-
-																					-
-------------------------------------------------------------------------------------*/
struct EarthCast
{
	int				_nInNum;						//
	float			_fInRadius;						//
	float			_fInUpSpd;						//
	float			_fUpTime;						//
	float			_fFloatTime;					//
	int				_nInHeightMax;					//
	int				_nInHeightMin;					//
	float			_fInSizeMax;					//
	float			_fInSizeMin;					//
	int				_nOutNum;						//
	float			_fOutRadius;					//
	float			_fOutUpSpd;						//
	int				_nOutHeightMax;					//
	int				_nOutHeightMin;					//
	float			_fOutSizeMax;					//
	float			_fOutSizeMin;					//

	EarthCast()
	{
		memset(this, 0, sizeof(EarthCast));
	}
};
//*********************************************************************************************
//*********************************************************************************************








/*-----------------------------------------------------------------------------------
-
-				CFloatStone2					Choi Jae Young
-												2005. 9. 26
-												v 0.02
-							In Blue-Side
-
-
-			note : 바닦에 뜨는 돌맹이들(멋진 표현이 없다. -_-;)
-				  
-
------------------------------------------------------------------------------------*/
class CFloatStone2 :  public CFcFXBase
{
public:

	void							Process();
	void							PreRender();
	void							Render(C3DDevice *pDevice);
	int								ProcessMessage(int nCode,DWORD dwParam1=0, DWORD dwParam2=0,DWORD dwParam3=0);

	PROTECTED_GETSET_PROP(D3DXVECTOR4,CenterPos, m_vCenterPos);
	PROTECTED_GETSET_PROP(D3DXVECTOR4,InhalePos, m_vInhalePos);
	PROTECTED_GETSET_PROP(FLOATSTONE,FloatState, m_FloatState);
	PROTECTED_GETSET_PROP(float,InRadius, m_fInRadius);
	PROTECTED_GETSET_PROP(float,OutRadius, m_fOutRadius);
	PROTECTED_GETSET_PROP(int,InNumOfStone, m_nInNumOfStoneWidth);
	PROTECTED_GETSET_PROP(int,OutNumOfStone, m_nOutNumOfStoneWidth);




private:

	void							CreateVB();
	void							Finalize();
	void							Initialize();
	void							InitDeviceData();
	void							ReleaseDeviceData();
	void							Reset();




private:

	int								m_nMaterialId;
	int								m_nTexture;
	float							m_fIntervalTime;
	float							m_fAccmulatePos[2];
	float							m_fLerpValue;

	EarthCast						m_sEarthCast;

	LPDIRECT3DVERTEXBUFFER9			m_pVB[2];
	IDirect3DVertexDeclaration9*	m_pVertexDecl;

	D3DXHANDLE						m_hWorldViewProj;
	D3DXHANDLE						m_hInterval;
	D3DXHANDLE						m_hCenterPos;
	D3DXHANDLE						m_hCameraPos;
	D3DXHANDLE						m_hInhalePos;
	D3DXHANDLE						m_hAccmulatePos;
	D3DXHANDLE						m_hTexture;
	D3DXHANDLE						m_hLerpValue;
	std::vector<int>				m_vecSkinList;




public:

	CFloatStone2();
	virtual ~CFloatStone2();
};
//*********************************************************************************************
//*********************************************************************************************

