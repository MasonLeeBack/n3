#pragma once

#include "FcFXBase.h"
#include "CrossVector.h"
#include "CDefinition.h"
#include "SmartPtr.h"


class CFcGameObject;



/*-----------------------------------------------------------------------------------
-
-				CFcFxGenerater					Choi Jae Young
-												2005. 11. 1
-												v 0.01
-							In Blue-Side
-
-
-			note : 
-				  
-
------------------------------------------------------------------------------------*/
class CFcFxGenerater :  public CFcFXBase
{
public:

	void							Process();
	void							PreRender();
	void							Render(C3DDevice *pDevice);
	int								ProcessMessage(int nCode,DWORD dwParam1=0, DWORD dwParam2=0,DWORD dwParam3=0);
	void							Update();
	

private:

	PRIVATE_GETSET_PROP(int, FxIndex, m_nFxIndex);
	PRIVATE_GETSET_PROP(int, FxObjIndex, m_nFxObjectIndex);
	PRIVATE_GETSET_PROP(CFcGameObject*, GameObj, m_pGameObj);
	PRIVATE_GETSET_PROP(bool, BTop, m_bTop);
	PRIVATE_GETSET_PROP(bool, BMiddle, m_bMiddle);
	PRIVATE_GETSET_PROP(bool, Loop, m_bLoop);
	



public:
	CFcFxGenerater();
	virtual ~CFcFxGenerater();
};
//**************************************************************************************
//**************************************************************************************








/*-----------------------------------------------------------------------------------
-
-				CFcFxThrowGenerater				Choi Jae Young
-												2005. 11. 2
-												v 0.01
-							In Blue-Side
-
-
-			note : 
-				  
-
------------------------------------------------------------------------------------*/
class CFcFxThrowGenerater :  public CFcFXBase
{
public:

	void							Process();
	void							PreRender();
	void							Render(C3DDevice *pDevice);
	int								ProcessMessage(int nCode,DWORD dwParam1=0, DWORD dwParam2=0,DWORD dwParam3=0);
	void							Update();



private:

	int								m_nState[3];
	int								m_nFxIndex[3];
	int								m_nFxObjIndex[3];
	int								m_nTrail;
	float							m_fSpd;
	bool							m_bTrail;
    D3DXVECTOR3						m_vPos[3];
	D3DXVECTOR3						m_vDir[3];




public:
	CFcFxThrowGenerater();
	virtual ~CFcFxThrowGenerater();
};
//**************************************************************************************
//**************************************************************************************








struct ListObject
{
	int				_nFxIndex;
	int				_nTick;
	D3DXVECTOR3		_vPos;
	D3DXVECTOR3		_vDir;
	ListObject()
	{
		memset( this, 0, sizeof(ListObject));
	}
};


/*-----------------------------------------------------------------------------------
-
-				CFcFxListGenerater				Choi Jae Young
-												2005. 11. 28
-												v 0.01
-							In Blue-Side
-
-
-			note : 
-				  
-
------------------------------------------------------------------------------------*/
class CFcFxListGenerater :  public CFcFXBase
{
public:

	void							Process();
	void							PreRender();
	void							Render(C3DDevice *pDevice);
	int								ProcessMessage(int nCode,DWORD dwParam1=0, DWORD dwParam2=0,DWORD dwParam3=0);
	void							Update();
	void							CreateFxObj( D3DXVECTOR3 p_vPos, D3DXVECTOR3 p_vDir );


private:

	PRIVATE_GETSET_PROP(int, LastTick, m_nLastTick);
	PRIVATE_GETSET_PROP(int, FxIndex, m_nFxIndex);
	PRIVATE_GETSET_PROP(int, LimitNum, m_nLimitNum);
	PRIVATE_GETSET_PROP(float, Scale, m_fScale);
	PRIVATE_GETSET_PROP(bool, Loop, m_bLoop);	//
	PRIVATE_GETSET_PROP(int, CreatePercent, m_nCreatePercent);
	std::list<ListObject*>			m_listObj;




public:
	CFcFxListGenerater();
	virtual ~CFcFxListGenerater();
};
//**************************************************************************************
//**************************************************************************************






/*-----------------------------------------------------------------------------------
-
-				CFcFxRandomGenerater			Choi Jae Young
-												2005. 12. 4
-												v 0.01
-							In Blue-Side
-
-
-			note : 
-				  
-
------------------------------------------------------------------------------------*/
class CFcFxRandomGenerater :  public CFcFXBase
{
public:

	void							Process();
	void							PreRender();
	void							Render(C3DDevice *pDevice);
	int								ProcessMessage(int nCode,DWORD dwParam1=0, DWORD dwParam2=0,DWORD dwParam3=0);



private:

	PRIVATE_GETSET_PROP(int, FxIndex, m_nFxIndex);
	PRIVATE_GETSET_PROP(int, FxObjIndex, m_nFxObjectIndex);
	PRIVATE_GETSET_PROP(int, Life, m_nLife);
	PRIVATE_GETSET_PROP(int, Percent, m_nPercent);
	PRIVATE_GETSET_PROP(float, RandomValue, m_fRandom);
	PRIVATE_GETSET_PROP(D3DXVECTOR3, Pos, m_vPos);
	PRIVATE_GETSET_PROP(D3DXVECTOR3, Dir, m_vDir);
	PRIVATE_GETSET_PROP(bool, Loop, m_bLoop);
	PRIVATE_GETSET_PROP(bool, Start, m_bStart);
	PRIVATE_GETSET_PROP(bool, RandomScale, m_bRandomScale);
	
	


public:
	CFcFxRandomGenerater();
	virtual ~CFcFxRandomGenerater();
};
//**************************************************************************************
//**************************************************************************************