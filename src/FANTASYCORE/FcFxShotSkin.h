#pragma once

#include "FcFXBase.h"
#include "CrossVector.h"
#include "CDefinition.h"
#include "FcCommon.h"


enum SkinObjState
{
	BeforCreate = 0,
	Create,
	Destory,

};
struct sSkinObj
{
	int				_nObjIndex;
	float			_fVel;
	float			_fScale;
	SkinObjState	_state;
	D3DXVECTOR3		_vFirstPos;
	D3DXVECTOR3		_vPos;
	D3DXVECTOR3     _vVel;
	D3DXVECTOR3		_vTPos;
};

typedef CSmartPtr<CFcGameObject> GameObjHandle;
/*-----------------------------------------------------------------------------------
-
-				CFcFxShotSkin					Choi Jae Young
-												2006.  1. 4
-												v 0.01
-							In Blue-Side
-
-
-			note : 
-				  
-
------------------------------------------------------------------------------------*/
class CFcFxShotSkin :  public CFcFXBase
{
public:

	void							Process();
	void							PreRender();
	void							Render(C3DDevice *pDevice);
	int								ProcessMessage(int nCode,DWORD dwParam1=0, DWORD dwParam2=0,DWORD dwParam3=0);
	
	

private:
	
	void							Initialize();
	void							Finalize();
	void							InitDeviceData();
	void							ReleaseDeviceData();
	void							UpdateObj();
	void							CreateObj(D3DXVECTOR3 p_vPos, D3DXVECTOR3 p_vTPos, int p_n = 1 );
	void							SetChangeTarget( IN D3DXVECTOR3 p_vTarget );


private:
	
	PRIVATE_GETSET_PROP(float, Scale, m_fScale);
	PRIVATE_GETSET_PROP(int, ListGenerater, m_nListGenerater);
	PRIVATE_GETSET_PROP(int, Skin, m_nSkinIndex);
	PRIVATE_GETSET_PROP(int, FxTem, m_nFxTem);
	std::list<sSkinObj*>			m_listObj;
	GameObjHandle					m_hParent;
	D3DXVECTOR3						m_vTarget;
	bool							m_bFire;
	bool							m_bChangeTarget;
	int								m_nDelay;




public:
	CFcFxShotSkin();
	virtual ~CFcFxShotSkin();
};
//**************************************************************************************
//**************************************************************************************