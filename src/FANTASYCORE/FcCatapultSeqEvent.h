#pragma once

#include "FcEventSequencer.h"
#include "FcGameObject.h"

class CFcCatapultSeqAttackEvent : public CFcEventSequencerElement 
{
public:
	CFcCatapultSeqAttackEvent( CFcEventSequencerMng *pMng );
	virtual ~CFcCatapultSeqAttackEvent();

public:
	int m_nFlag;
	int m_nAttackTime;
	D3DXVECTOR3 m_vTargetPos;

public:
	virtual void Run();
	virtual bool IsDestroy();
};