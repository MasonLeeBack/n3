#pragma once

#include "FcCommon.h"
#include "FcFxBase.h."
#include "CDefinition.h"


class CFcGameObject;
typedef CSmartPtr<CFcGameObject> GameObjHandle;

/*-----------------------------------------------------------------------------------
-																					-
-	SpoutOfWater: �� ���� ������ ����Ʈ �߰� �� ������ �浹�� ���� ���� �÷��� �߰� -
-									vector�� ������.								-
-										2005.11.24 v.6								-
-																					-
-------------------------------------------------------------------------------------*/
struct SpoutOfWater
{
	int				_nFxDropWater;								// ����� ���� ����Ʈ
	int				_nFxGround;									// �ٴۿ� ��Ÿ�� ����Ʈ
	int				_nFxEnemy[35];								// ���� ���ٱⰡ �浹�ϸ� ��Ÿ�� ����Ʈ
	int				_nCurveTrail;							// �� �ٱ�
	int				_nNextIndex;
	int				_nNextIndex2;
	int				_nTick1;									// ����� ������ ���� tick
	BOOL			_bGround;									// �������� ��� �༮�ΰ�
	BOOL			_bBig;										// ū �༮�ΰ�
	BOOL			_bDelete;									// ������ �༮�ΰ�.
	BOOL			_bDraw;
	float			_fRandom;
	D3DXVECTOR3		_vFirstPos;									// ���� ���� ��ġ
	D3DXVECTOR3		_vTarget;									// Ÿ����ġ

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
-			note : ���ٱ�.  �������� : CreateSpoutsOfWater() , SpoutSimulation()�߰�, trail => �޽��� ����
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

	int								m_nFxIndex1;														// ����� ���� 
	int								m_nFxIndex2;														// �ٴۻ��� 
	int								m_nFxIndex3;														// �ٴ� ū��
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
