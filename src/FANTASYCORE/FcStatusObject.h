#pragma once

#include "SmartPtr.h"
#include "CrossVector.h"

class CFcGameObject;
typedef CSmartPtr<CFcGameObject> GameObjHandle;
class CFcStatusObject {
public:
	enum STATUS_TYPE {
		STATUS_CHAOS,
		STATUS_DOWN_SPEED,
		STATUS_RECOVERY_HP,
		STATUS_IGNITION,
	};

	CFcStatusObject();
	virtual ~CFcStatusObject();

	static CFcStatusObject *BeginStatus( GameObjHandle Handle, STATUS_TYPE Type, int nParam1, int nParam2, int nParam3 );

protected:
	GameObjHandle m_Handle;
	STATUS_TYPE m_Type;
	bool m_bFinish;
	int m_nParam[3];
	int m_nPrevTeam;
	
	virtual void BeginStatus() {}
	virtual void FinishStatus() {}

public:
	virtual void Process() {}
	virtual void PostProcess() {}
	bool IsFinish() { 
		if( m_bFinish ) FinishStatus();
		return m_bFinish; 
	}

};

class CFcStatusChaos : public CFcStatusObject {
public:
	CFcStatusChaos();
	virtual ~CFcStatusChaos();

	virtual void Process();
protected:
	int m_nParticleObjectIndex;
	CCrossVector m_CrossVector;
	int m_nTimer;

	virtual void BeginStatus();
	virtual void FinishStatus();
};

class CFcStatusRecoveryHP : public CFcStatusObject {
public:
	CFcStatusRecoveryHP() {}

protected:
	virtual void BeginStatus();
};

class CFcStatusDownMoveSpeed : public CFcStatusObject {
public:
	CFcStatusDownMoveSpeed();
	virtual ~CFcStatusDownMoveSpeed();
	virtual void Process();
	virtual void PostProcess();

protected:
	int m_nFxObjectIndex;
	CCrossVector m_CrossVector;
	int m_nTimer;
	float m_fPrevMoveSpeed;

	virtual void BeginStatus();
	virtual void FinishStatus();
};

class CFcStatusIgnition : public CFcStatusObject {
public:
	CFcStatusIgnition();
	virtual ~CFcStatusIgnition();
	virtual void Process();

protected:
	int m_nFxObjectIndex;
	CCrossVector m_CrossVector;
	int m_nTimer;


	virtual void BeginStatus();
	virtual void FinishStatus();
};