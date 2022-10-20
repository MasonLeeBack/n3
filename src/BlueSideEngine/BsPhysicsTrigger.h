#pragma once

class IBsPhysicsActor;

class CBsPhysicsTriggerBase
{
protected:
	IBsPhysicsActor *m_pTriggerActor;	
		
public:
	CBsPhysicsTriggerBase();
	virtual ~CBsPhysicsTriggerBase();
	virtual void OnTrigger() = 0;
	void CreateTriggerBox( D3DXVECTOR3 Size, D3DXVECTOR3 Pivot, D3DXMATRIX matObj );		
	void ReleaseTriggerActor();
	void SetTriggerPose( D3DXMATRIX &matObj );		
};
