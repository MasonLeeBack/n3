#pragma once

#include "CrossVector.h"
#include "FcFxTidalWave.h"
#include "FcAbilityBase.h"
#include "CDefinition.h"

class CFcGameObject;
typedef CSmartPtr<CFcGameObject> GameObjHandle;




/*-----------------------------------------------------------------------------------
-
-				CDamageObjBase					Choi Jae Young
-												2005. 8. 24
-												v 0.01
-							In Blue-Side
-
-
-			note : base Ŭ����. ���� ���� ��ġ�� �������� �޴����� Ȯ���ϴ� ��� Ŭ����.
-				   
-
------------------------------------------------------------------------------------*/
class CDamageObjBase
{
	PROTECTED_GETSET_PROP(bool,Damage, m_bDamage);
	PROTECTED_GETSET_PROP(D3DXVECTOR3*,WaterPoint, m_WaterPoint);

public:
	CDamageObjBase():m_bDamage(0){};
	virtual ~CDamageObjBase() {};
	
};
//************************************************************************************
//************************************************************************************










/*-----------------------------------------------------------------------------------
-
-				CDamageObj						Choi Jae Young
-												2005. 8. 24
-												v 0.01
-							In Blue-Side
-
-
-			note : �������� ���� ������Ʈ. ������ GameObjHandle �ν� �����Ѵ�. 
-				   �������� �޾Ҵ����� m_bDamage �������� ����.
-
------------------------------------------------------------------------------------*/
class CDamageObj : public CDamageObjBase
{
public:
	void						Process(GameObjHandle p_hParent, bool p_bNotFullOrbAttack = false);
	PROTECTED_GETSET_PROP(float,Power, m_fPower);
	PROTECTED_GETSET_PROP(GameObjHandle,ObjHandle, m_ObjHandle);
public:

	virtual ~CDamageObj();
};
//************************************************************************************
//************************************************************************************










/*-----------------------------------------------------------------------------------
-
-				CDamageProp						Choi Jae Young
-												2005. 8. 24
-												v 0.01
-							In Blue-Side
-
-
-			note : �� orb�� �� ������ �� ������ ���� ������ ó��. �̵��� ���������� �Ʒ����� �Ѵ�.
-				   
-
------------------------------------------------------------------------------------*/
class CDamageProp : public CDamageObjBase
{
public:

	void						Process(GameObjHandle p_hParent, bool p_bNotFullOrbAttack = false);

	PROTECTED_GETSET_PROP(CFcProp*,PropHandle, m_ObjHandle);

};
//************************************************************************************
//************************************************************************************










/*-----------------------------------------------------------------------------------
-
-				CDamageObjManager				Choi Jae Young
-												2005. 8. 24
-												v 0.01
-							In Blue-Side
-
-
-			note : �� orb�� �� ������ �� ������ ���� ������ ó��. �̵��� ���������� �Ʒ����� �Ѵ�.
-				   
-
------------------------------------------------------------------------------------*/
class CDamageObjManager
{
public:

	void 						Process(bool p_bNotFullOrbAttack = false);
	void 						AddObj(std::vector<GameObjHandle> &ObjList,D3DXVECTOR3 *Pos,float *fPower);
	void 						AddProp(std::vector<CFcProp *> &ObjList,D3DXVECTOR3 *Pos);
	void 						Clear();

protected:

	std::vector<CDamageObj*>	m_ObjList;
	std::vector<CDamageProp*>	m_PropList;
	PROTECTED_GETSET_PROP(GameObjHandle,GameHandle, m_hParent);

public:

	CDamageObjManager(){};
};
//************************************************************************************
//************************************************************************************











/*-----------------------------------------------------------------------------------
-
-				CFcAbilityWater					Choi Jae Young
-												2005. 8. 23
-												v 0.01
-							In Blue-Side
-
-
-			note : �� orb�� �� ������ �� ������ ���� ������ ó��. �̵��� ���������� �Ʒ����� �Ѵ�.
-				   
-
------------------------------------------------------------------------------------*/
class CFcAbilityWater : public CFcAbilityBase
{
public:

	virtual void					Process();												//  �� orb ������ ��ġ�� �ӵ��ν� ������ �޴����� ���� �߰��Ѵ�.
	virtual void					Initialize(DWORD dwParam1 = 0, DWORD dwParam2 = 0);		//  �ʱ�ȭ. �� orb ����
	virtual void					Finalize();												//  �� orb �ε����� -1�� ����. �ǹ̾���

	virtual void					Initialize(bool p_b);	
	D3DXVECTOR3* GetPosition() { return NULL; };

protected:

	void							DamageProcess(D3DXVECTOR3 *Pos,float *fPower);			// ���� �������� ���� ������ �߰��ϴ� �Լ�.





protected:

	int								m_nTidalWaveFxID;										// �� orb �ε���
	bool							m_bNotFullOrb;											// false : full orb attack, true : not full orb attack.
	CDamageObjManager				m_DamageObjManager;										// ������ �޴���. 


public:

	CFcAbilityWater( GameObjHandle hParent );
	virtual ~CFcAbilityWater();
};
//************************************************************************************
//************************************************************************************
