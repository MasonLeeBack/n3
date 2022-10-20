#pragma once

#include "FcGameObject.h"
#include "FcUnitObject.h"

#define MAX_SPINE_ROTATION_ANGLE 30.f	// �㸮 ȸ�� �ִ� ��
#define SPINE_ROTATION_ANGLE 3.f		// �㸮 ȸ�� �Ұ�

#define MAX_SPINE_PITCHING_ANGLE 80.f	// ���Ʒ� �ִ� ��
#define SPINE_PITCHING_ANGLE 1.f		// ���Ʒ� �ӵ�
#define PITCHING_MIN_UP_ANGLE 20.f		// 0�� �̻�, Value �� �����϶� Value ������ ����
#define PITCHING_MIN_DOWN_ANGLE	15.f		// 0�� ����, Value �� �̻��϶� Value ������ ����


class CFcArcherUnitObject : public CFcUnitObject
{
public:
	CFcArcherUnitObject(CCrossVector *pCross);
	virtual ~CFcArcherUnitObject();

protected:
	bool m_bApplyTracking;
	float m_fCurRotationAngle;

	bool m_bApplyPitching;
	float m_fCurPitchingAngle;

	int m_nDieCount;
	int m_nHitProb;
	int m_nHitProbAdd;

	void ProcessTargetTracking();
	void ProcessTargetPitching();
public:
	// �θ� Virtual Function
	/*
	virtual void Initialize( INITGAMEOBJECTDATA* pData, int nTeam, int nGroup, TroopObjHandle hTroop, float fScaleMin=1.f, float fScaleMax=1.f );
	virtual void Initialize(int nUnitIndex, int nTeam, int nGroup, TroopObjHandle hTroop, float fScaleMin=1.f, float fScaleMax=1.f);
	*/

	virtual bool Render();
	virtual void Process();
	virtual void PostProcess();
	virtual bool IsDefensible();


	virtual void RotationProcess();
	virtual void SignalCheck( ASSignalData *pSignal );

	void SetHitProbAdd( int nPer );
	void SetHitProb( int nProb );
};
