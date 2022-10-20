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
-			note : base 클래스. 현재 물의 위치와 데미지를 받는지를 확인하는 기반 클래스.
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
-			note : 데미지를 받을 오브젝트. 접근은 GameObjHandle 로써 접근한다. 
-				   데미지를 받았는지는 m_bDamage 변수값을 참조.
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
-			note : 물 orb의 각 생성및 각 정점에 대한 데미지 처리. 이동및 여러가지는 아래에서 한다.
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
-			note : 물 orb의 각 생성및 각 정점에 대한 데미지 처리. 이동및 여러가지는 아래에서 한다.
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
-			note : 물 orb의 각 생성및 각 정점에 대한 데미지 처리. 이동및 여러가지는 아래에서 한다.
-				   
-
------------------------------------------------------------------------------------*/
class CFcAbilityWater : public CFcAbilityBase
{
public:

	virtual void					Process();												//  물 orb 에대한 위치와 속도로써 데미지 메니져에 값을 추가한다.
	virtual void					Initialize(DWORD dwParam1 = 0, DWORD dwParam2 = 0);		//  초기화. 물 orb 생성
	virtual void					Finalize();												//  물 orb 인덱스를 -1로 세팅. 의미없음

	virtual void					Initialize(bool p_b);	
	D3DXVECTOR3* GetPosition() { return NULL; };

protected:

	void							DamageProcess(D3DXVECTOR3 *Pos,float *fPower);			// 실제 데미지를 입을 적들을 추가하는 함수.





protected:

	int								m_nTidalWaveFxID;										// 물 orb 인덱스
	bool							m_bNotFullOrb;											// false : full orb attack, true : not full orb attack.
	CDamageObjManager				m_DamageObjManager;										// 데미지 메니져. 


public:

	CFcAbilityWater( GameObjHandle hParent );
	virtual ~CFcAbilityWater();
};
//************************************************************************************
//************************************************************************************
