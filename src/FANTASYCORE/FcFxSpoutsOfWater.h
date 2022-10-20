#pragma once

#include "FcCommon.h"
#include "FcFxBase.h."
#include "CDefinition.h"


class CFcGameObject;
typedef CSmartPtr<CFcGameObject> GameObjHandle;

/*-----------------------------------------------------------------------------------
-																					-
-	SpoutOfWater: 적 몸에 생성될 이펙트 추가 및 지형에 충돌할 지에 대한 플래그 추가 -
-									vector로 관리함.								-
-										2005.11.24 v.6								-
-																					-
-------------------------------------------------------------------------------------*/
struct SpoutOfWater
{
	int				_nFxDropWater;								// 물방울 생성 이펙트
	int				_nFxGround;									// 바닦에 나타날 이펙트
	int				_nFxEnemy[35];								// 적과 물줄기가 충돌하면 나타날 이펙트
	int				_nCurveTrail;							// 물 줄기
	int				_nNextIndex;
	int				_nNextIndex2;
	int				_nTick1;									// 물방울 생성후 가는 tick
	BOOL			_bGround;									// 지면으로 쏘는 녀석인가
	BOOL			_bBig;										// 큰 녀석인가
	BOOL			_bDelete;									// 지워질 녀석인가.
	BOOL			_bDraw;
	float			_fRandom;
	D3DXVECTOR3		_vFirstPos;									// 최초 시작 위치
	D3DXVECTOR3		_vTarget;									// 타겟위치

	SpoutOfWater()
	{
		memset(this, -1, sizeof(SpoutOfWater) );
		_fRandom = -1.0f;
	}
};
//*********************************************************************************************
//*********************************************************************************************













/*-----------------------------------------------------------------------------------
-
-				CFcFxSpoutsOfWater				Choi Jae Young
-												2005. 11. 24
-												v 0.6
-							In Blue-Side
-
-
-			note : 물줄기.  수정내용 : CreateSpoutsOfWater() , SpoutSimulation()추가, trail => 메쉬로 수정
-				   
-
------------------------------------------------------------------------------------*/
class CFcFxSpoutsOfWater : public CFcFXBase
{
public:

	void							Process();
	void							PreRender();
	void							Render(C3DDevice *pDevice);
	int								ProcessMessage(int nCode,DWORD dwParam1=0, DWORD dwParam2=0,DWORD dwParam3=0);
	



private:

	void							CreateSpoutsOfWater( D3DXVECTOR3 p_vPos, D3DXVECTOR3 p_vDir, int p_nStart, bool p_bGround, bool p_bBig);
	void							PushObj( SpoutOfWater* p_pObj );
	void							SpoutSimulation(SpoutOfWater* p_pObj);								
	void							EnemyEffect(SpoutOfWater* p_pObj);
	void							DeleteAllObj();
	void							UpdateObj();
	void							Init();										
	void							Reset();
	bool							IsAddObj( D3DXVECTOR3 p_vPos );


private:

	int								m_nFxIndex1;														// 물방울 생성 
	int								m_nFxIndex2;														// 바닦생성 
	int								m_nFxIndex3;														// 바닦 큰거
	int								m_nLevel;
	int								m_nSkinindex;
	int								m_nEnemySkinindex[35];
	int								m_nFxId;
	GameObjHandle					m_hParent;
	std::list<SpoutOfWater*>		m_listSpoutsOfWater;


public:

	CFcFxSpoutsOfWater(void);
	~CFcFxSpoutsOfWater(void);
};
//*********************************************************************************************
//*********************************************************************************************
