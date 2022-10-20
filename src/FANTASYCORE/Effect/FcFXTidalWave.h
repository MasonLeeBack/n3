#ifndef __FC_FX_TIDALWAVE_H__
#define __FC_FX_TIDALWAVE_H__

#include "FcCommon.h"
#include "FcFxBase.h."
#include "CDefinition.h"

class C3DDevice;
class CFcProp;
//class GameObjHandle;
typedef CSmartPtr<CFcGameObject> GameObjHandle;
/*-----------------------------------------------------------------------------------
-
-		구조체 선언 : _VERTEX, _GROUP, CUSTOMVERTEX		Choi Jae Young
-														2005. 8. 25
-														v 0.01
-							In Blue-Side
-
-
-			note : 물 orb의 렌더링 클래스 내부에 사용하는 구조체.
-				   
-
------------------------------------------------------------------------------------*/
#ifdef _XBOX
struct _VERTEX							// rendering 정보와 attribute 정보를 분리한다.  => 렌더링 정보에 agp 메모리 대역폭을 줄일 수 있다.
{
	BSVECTOR			Pos;							
	BSVECTOR			Vel;					
	BSVECTOR			Acc;					
	bool				bNoEff;					
};
struct _GROUP							// 물 orb를 하나의 묶음으로 처리하기 위한 구조체
{
	BSVECTOR			Pos;					// 위치
	int					iV;						// iCol * iRow  (정점 갯수)
	int					iCol,iRow;				
	int					iX1, iY1, iX2, iY2;		//
	float				fRad;					//
	BSVECTOR*			vPos;					//
};
struct CUSTOMVERTEX						// rendering 정보만을 가지고 있는 구조체.
{
	BSVECTOR			Pos;					
	BSVECTOR			Normal;					
	D3DXVECTOR2			TexCoord;
};
#else
struct _VERTEX							// rendering 정보와 attribute 정보를 분리한다.  => 렌더링 정보에 agp 메모리 대역폭을 줄일 수 있다.
{
	BSVECTOR			Pos;							
	BSVECTOR			Vel;					
	BSVECTOR			Acc;					
	bool				bNoEff;					
};
struct _GROUP							// 물 orb를 하나의 묶음으로 처리하기 위한 구조체
{
	BSVECTOR			Pos;					// 위치
	int					iV;						// iCol * iRow  (정점 갯수)
	int					iCol,iRow;				
	int					iX1, iY1, iX2, iY2;		//
	float				fRad;					//
	BSVECTOR*			vPos;					//
};
struct CUSTOMVERTEX						// rendering 정보만을 가지고 있는 구조체.
{
	BSVECTOR			Pos;					
	BSVECTOR			Normal;					
	D3DXVECTOR2			TexCoord;
};
#endif


#define PI					3.141592f
#define	SPHERE_TABLE_SIZE	12
//*********************************************************************************************
//*********************************************************************************************





/*-----------------------------------------------------------------------------------
-																					-
-		WaterDrop	:	튜르르 공격중 물 뿌리는 공격에 사용.						-
-										2005.10.5 v0.01								-
-																					-
-------------------------------------------------------------------------------------*/
struct WaterDrop
{
	int _nIndex;
	int _nPosIndex;
	WaterDrop( int p_nIndex, int p_nPosIndex ) { _nIndex = p_nIndex, _nPosIndex = p_nPosIndex; }
};
//*********************************************************************************************
//*********************************************************************************************


struct sParticle
{
	float _fSpeed;
	float _fcosf;
	D3DXVECTOR3 _vPos;
};

struct sOrbAttackParticle
{
	D3DXVECTOR3 _vPos;
	D3DXVECTOR3 _vDir;
};

/*-----------------------------------------------------------------------------------
-
-				CFcFXTidalWave					Choi Jae Young
-												2005. 8. 25
-												v 0.01
-							In Blue-Side
-
-
-			note : 물 orb의 렌더링 정보 클래스.
-				   
-
------------------------------------------------------------------------------------*/
class CFcFXTidalWave : public CFcFXBase
{

public:

	void							Render(C3DDevice *pDevice);
	int								ProcessMessage(int nCode,DWORD dwParam1=0, DWORD dwParam2=0,DWORD dwParam3=0);
	inline int						GetVertexCount()							{ return m_iVertex; }
	inline int						GetEngineIndex()							{ return m_iEngineIndex; }
	inline D3DXVECTOR3				GetVertexPos( int iIndex )					{ D3DXVECTOR3 vTmp(m_pVList[iIndex].Pos.x,m_pVList[iIndex].Pos.y,m_pVList[iIndex].Pos.z); return vTmp;	}
	inline D3DXVECTOR3				GetVertexVel( int iIndex )					{ D3DXVECTOR3 vTmp(m_pVList[iIndex].Vel.x,m_pVList[iIndex].Vel.y,m_pVList[iIndex].Vel.z); return vTmp;  }

	PRIVATE_GETSET_PROP(bool, WaterDrop, m_bWaterDrop);





private:

	void							Process();
	void							Init(float fLifeTime);
	void							Ungroup();
	void							SetGroupPos( BSVECTOR *pPos, float fR);	
	void							SetSpread(BSVECTOR *pOrg, float fSpeed)		{ m_bSpread = true; m_vSpreadOrigin = (*pOrg); m_fSpreadSpeed = fSpeed;}
	void							Reset();
	void							Explode(BSVECTOR *pOrigin, float fSpeedMax);
	void							CreateVertices();
	void							CreateIndices();
	void							CustomRender(C3DDevice *pDev);
	void							ProcessSpring( CUSTOMVERTEX *pV);
	void							ProcessSafeRegion(CUSTOMVERTEX *pV);
	void							ProcessNormal(CUSTOMVERTEX *pV, WORD *pI);
	void							ComputeSpring(BSVECTOR *pOut, float fSpr, float fDist0, BSVECTOR *pObj, BSVECTOR *pOrg);
	void							ComputeBounce(BSVECTOR *pVelOut, BSVECTOR *pPos, float fHeight, BSVECTOR *pVel);
	void    						DamageProcess( CUSTOMVERTEX *pV,float *fPower);
	int								_Group(int iX1, int iY1, int iX2, int iY2);
	void							Clear();





private:

	_VERTEX*						m_pVList;
	_GROUP							m_G;
	CUSTOMVERTEX*					m_pV2; 
	WORD*							m_pI2; 

	std::vector<sParticle>			m_vecParticlePos[2];
	std::vector<sOrbAttackParticle>	m_vecOrbParticle[2];

	BSVECTOR						m_vPos;
	BSVECTOR						m_vSpreadOrigin;
	BSVECTOR						m_vGravity;
	BSVECTOR						m_vSafeCenter;
	D3DXVECTOR4						m_vCount;
	bool							m_bInitialized;
	bool							m_bSpread;
	float							m_fInitDist;
	float							m_fSpreadSpeed;
	float							m_fLifeTime;
	float							m_fTick;
	float							m_fSafeRad;

	int								m_nMaterialIndex;
	int								m_nSwizzleTex;
	int								m_nSplashPtc;
	int								m_nSplashPtc2;
	int								m_iEngineIndex;
	int								m_iCol, m_iRow;
	int								m_iVertex;
	int								m_iTriangle;
	int								m_nVertexDeclIdx;
	int								m_nFxGenerater;
	int								m_nWaterFxID;

	D3DXHANDLE						m_hWorldViewProj;
	D3DXHANDLE						m_hWorld;
	D3DXHANDLE						m_hworldInverseTranspose;
	D3DXHANDLE						m_hviewInverse;
	D3DXHANDLE						m_hlightDir;
	D3DXHANDLE						m_hlightDiff;
	D3DXHANDLE						m_hlightAmb;
	D3DXHANDLE						m_hcounter;
	D3DXHANDLE						m_hmatSpec;
	D3DXHANDLE						m_hBackBuffer;
	D3DXHANDLE						m_hSwizzle;
	D3DXHANDLE						m_hmatAmb;
	D3DXHANDLE						m_hmatDiff;
	D3DXHANDLE						m_hLightDirectionViewSpace;
	GameObjHandle					m_hParent;



public:

	CFcFXTidalWave();
	~CFcFXTidalWave();


};
//*********************************************************************************************
//*********************************************************************************************







#endif