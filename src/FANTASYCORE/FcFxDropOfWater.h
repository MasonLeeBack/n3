#pragma once
#include "FcFXBase.h"
#include "FcCommon.h"
#include "CDefinition.h"

class C3DDevice;

enum WATER_STATE
{
	NONE_DROP = -1,
	CREATE_DROP = 0,
	UP_DROP,
	FIRE_DROP,
};

class CFcGameObject;
typedef CSmartPtr<CFcGameObject> GameObjHandle;

/*-----------------------------------------------------------------------------------
-																					-
-		FxTyruruObject	:		물덩어리											-
-									vector로 관리함.								-
-										2005.11.25 v.1								-
-																					-
-------------------------------------------------------------------------------------*/
struct FxTyruruObject
{
	bool			_bSpout;
	int				_nFxObjIndex1;
	int				_nFxObjIndex2;
	int				_nState;
	int				_nTick;
	int				_nTick2;
	float			_fSize;
	float			_fHeight;
	float			_fDistFromCenter;
	D3DXVECTOR3		_vPos;
	D3DXVECTOR3		_vDir;
	FxTyruruObject()
	{
		memset( this, 0, sizeof(FxTyruruObject));
		_nFxObjIndex1 = -1;
		_nFxObjIndex2 = -1;
	}
};
//*********************************************************************************************
//*********************************************************************************************







/*-----------------------------------------------------------------------------------
-																					-
-		FxSpoutWater	:	물쏘는것												-
-									vector로 관리함.								-
-										2005.11.25 v.01								-
-																					-
-------------------------------------------------------------------------------------*/
struct FxSpoutWater
{
	int				_nFxIndex;
	int				_nStartTick;
	D3DXVECTOR3		_vPos;
	D3DXVECTOR3		_vVel;
	FxSpoutWater()
	{
		memset( this, 0, sizeof(FxSpoutWater));
	}
};
//*********************************************************************************************
//*********************************************************************************************







/*-----------------------------------------------------------------------------------
-																					-
-		FxWater	:	적 몸에 나타나는 물 오브젝트 배열								-
-									vector로 관리함.								-
-										2005.11.28 v.01								-
-																					-
-------------------------------------------------------------------------------------*/
struct FxWater
{
	int				_nFxIndex[35];
	int				_nCurrentId;
	D3DXVECTOR3		_vPos;
	D3DXVECTOR3		_vDir;
	FxWater()
	{
		memset( this, 0, sizeof(FxWater));
	}
};
//*********************************************************************************************
//*********************************************************************************************








/*-----------------------------------------------------------------------------------
-
-				CFcFxDropOfWater				Choi Jae Young
-												2005. 10. 19
-												v 0.01
-							In Blue-Side
-
-
-			note : 튜르르 .
-				  
-
------------------------------------------------------------------------------------*/
class CFcFxDropOfWater  : public CFcFXBase
{
public:

	void							Process();
	void							PreRender();
	void							Render(C3DDevice *pDevice);
	int								ProcessMessage(int nCode,DWORD dwParam1=0, DWORD dwParam2=0,DWORD dwParam3=0);
	


	
private:

	void							CreateObject( int p_nFirstTick, int p_nNum = 5 );	
	void							Initialize();
	void							Finalize();
	void							UpdateObj1();
	void							UpdateObj2();
	void							AddTyruruObj(FxTyruruObject* p_Obj);
	void							ClearTyruruObj();
	void							TickCount();





private:

	PRIVATE_GETSET_PROP(WATER_STATE, WState, m_eState);
	int								m_nTick;
	int								m_nFxTypeIndex1;						// 물덩어리 모이는 이펙트
	int								m_nFxTypeIndex2;						// 물쏘는 이펙트
	int								m_nParticle;							// 물쏠때 먼저 나타나는 파티클
	int								m_nSkinindex;							// 물덩어리 스킨
	int								m_nFxid;								// 클래스 핸들
	int								m_nEnemySkinindex[35];
	D3DXVECTOR3						m_vCenter;
	GameObjHandle					m_hParent;
	std::vector<FxTyruruObject*>	m_vecFxObject;
	std::list<FxSpoutWater*>		m_listSpoutObject;
	std::list<FxWater*>				m_listWater;



public:

	CFcFxDropOfWater(void);
	~CFcFxDropOfWater(void);
};
//*********************************************************************************************
//*********************************************************************************************