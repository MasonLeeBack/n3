#pragma once

#include "FcEventSequencer.h"
#include "FcGameObject.h"

// ��Ÿ�� ������~
class CFcHeroSeqRideHorseEvent : public CFcEventSequencerElement 
{
public:
	CFcHeroSeqRideHorseEvent( CFcEventSequencerMng *pMng );
	virtual ~CFcHeroSeqRideHorseEvent();

public:
	int m_nFlag;
	int m_nAniTypeIndex;
	D3DXVECTOR3 m_vPos;
	D3DXVECTOR3 m_vDir;
	GameObjHandle m_Handle;

public:
	virtual void Run();
	virtual bool IsDestroy();
};

// Ʈ�� ������� ������~
class CFcProp;
class CFcHeroSeqPickupEvent : public CFcEventSequencerElement
{
public:
	CFcHeroSeqPickupEvent( CFcEventSequencerMng *pMng );
	virtual ~CFcHeroSeqPickupEvent();

public:
	int m_nFlag;
	CFcProp *m_pProp;
	int m_nLoopCount;
	int m_nWeaponIndex;
	D3DXVECTOR3 m_vPos;
	D3DXVECTOR3 m_vDir;
	D3DXVECTOR3 m_vDirStart;

public:
	virtual void Run();
	virtual bool IsDestroy();
};

// ���� ������ ������~
class CFcHeroSeqHidePropEvent : public CFcEventSequencerElement
{
public:
	CFcHeroSeqHidePropEvent( CFcEventSequencerMng *pMng );
	virtual ~CFcHeroSeqHidePropEvent();

public:
	int m_nFlag;
	int m_nAniIndex;
	int m_nHidePropFrame;
	CFcProp *m_pProp;

public:
	virtual void Run();
	virtual bool IsDestroy();
};
