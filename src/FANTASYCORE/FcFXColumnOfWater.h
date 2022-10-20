#pragma once
#include "FcCommon.h"
#include "FcFXBase.h"
#include "CDefinition.h"

class C3DDevice;





/*-----------------------------------------------------------------------------------
-																					-
-		ColumnWaterObj	:	물기둥 obj 구조체. CFcFXColumnOfWater클래스에서			-
-									vector로 관리함.								-
-										2005.9.26 v.3								-
-																					-
-------------------------------------------------------------------------------------*/
struct ColumnWaterObj
{
	int				_index;
	int				_nTick;
	bool			_bSmall;
	D3DXVECTOR3		_vPos;
	ColumnWaterObj()
	{
		memset(this, 0, sizeof(ColumnWaterObj));
	}
};
//*********************************************************************************************
//*********************************************************************************************












class CFcGameObject;
typedef CSmartPtr<CFcGameObject> GameObjHandle;

/*-----------------------------------------------------------------------------------
-
-				CFcFXColumnOfWater				Choi Jae Young
-												2005. 9. 23
-												v 0.01
-							In Blue-Side
-
-
-			note : 튜르르 물기둥 공격의 렌더링 정보.
-				  
-
------------------------------------------------------------------------------------*/
class CFcFXColumnOfWater : public CFcFXBase
{
public:

	void							Process();
	void							PreRender();
	void							Render(C3DDevice *pDevice);
	int								ProcessMessage(int nCode,DWORD dwParam1=0, DWORD dwParam2=0,DWORD dwParam3=0);

	PROTECTED_GETSET_PROP(int, Life, m_nLife);
	PROTECTED_GETSET_PROP(int, ParentLevel, m_nLevel);



    
private:
						
	void							CreateNormalWaterObj( D3DXVECTOR3 p_vPos, int p_nFxId, bool p_bSmall, int p_nDelayTick );
	void							PushNormalWaterObj( ColumnWaterObj *p_pObj );
	void							DeleteAllObj();
	void							Update();
	void							Initialize( int p_nLifeTick );										// 현재 생각중인 옵션 : 전체 라이프 틱.
	void							Finalize();
	void							TickCount( int &p_nTick );




private:

	std::list<ColumnWaterObj*>		m_listFxObj;
	GameObjHandle					m_hParent;




public:

	CFcFXColumnOfWater(void);
	~CFcFXColumnOfWater(void);
};
//*********************************************************************************************
//*********************************************************************************************