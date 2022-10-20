#pragma once

#include "FcFXBase.h"
#include "CrossVector.h"
#include "CDefinition.h"
#include "SmartPtr.h"


class CFcGameObject;
typedef CSmartPtr<CFcGameObject> GameObjHandle;


/*-----------------------------------------------------------------------------------
-
-				CFcFxBurning					Choi Jae Young
-												2005. 11. 1
-												v 0.01
-							In Blue-Side
-
-
-			note : 
-				  
-
------------------------------------------------------------------------------------*/
class CFcFxBurning :  public CFcFXBase
{
public:

	void							Process();
	void							PreRender();
	void							Render(C3DDevice *pDevice);
	int								ProcessMessage(int nCode,DWORD dwParam1=0, DWORD dwParam2=0,DWORD dwParam3=0);
	void							Update();


private:

	PRIVATE_GETSET_PROP(D3DXCOLOR, OrigineColor, m_cOrigin);
	PRIVATE_GETSET_PROP(D3DXCOLOR, AddColor, m_cAddColor);
	PRIVATE_GETSET_PROP(int, FxTemplateIndex, m_nFxTemplateIndex);
	PRIVATE_GETSET_PROP(int, FxObjIndex, m_nFxObjectIndex);
	PRIVATE_GETSET_PROP(CFcGameObject*, GameObj, m_pGameObj);
	PRIVATE_GETSET_PROP(bool, Burn, m_bBurn);




public:
	CFcFxBurning();
	virtual ~CFcFxBurning();
};
