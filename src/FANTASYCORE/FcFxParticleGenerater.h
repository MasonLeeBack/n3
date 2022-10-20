#pragma once

#include "FcFXBase.h"
#include "CrossVector.h"
#include "CDefinition.h"
#include "SmartPtr.h"


class CFcGameObject;



/*-----------------------------------------------------------------------------------
-
-				CFcFxParticleGenerater			Choi Jae Young
-												2005. 11. 1
-												v 0.01
-							In Blue-Side
-
-
-			note : 
-				  
-
------------------------------------------------------------------------------------*/
class CFcFxParticleGenerater :  public CFcFXBase
{
public:

	void							Process();
	void							PreRender();
	void							Render(C3DDevice *pDevice);
	int								ProcessMessage(int nCode,DWORD dwParam1=0, DWORD dwParam2=0,DWORD dwParam3=0);



private:

	PRIVATE_GETSET_PROP(int, ParticleGroupIndex, m_nParticleGroupIndex);
	PRIVATE_GETSET_PROP(int, ParticleIndex, m_nParticleIndex);
	PRIVATE_GETSET_PROP(CFcGameObject*, GameObj, m_pGameObj);
	PRIVATE_GETSET_PROP(bool, BTop, m_bTop);
	PRIVATE_GETSET_PROP(bool, BMiddle, m_bMiddle);
	PRIVATE_GETSET_PROP(bool, Loop, m_bLoop);



public:
	CFcFxParticleGenerater();
	virtual ~CFcFxParticleGenerater();
};
