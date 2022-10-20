#pragma once

#include "FcGameObject.h"
#include "FcUnitObject.h"

#define MAX_SPINE_ROTATION_ANGLE 30.f	// 허리 회전 최대 각
#define SPINE_ROTATION_ANGLE 3.f		// 허리 회전 소곧

#define MAX_SPINE_PITCHING_ANGLE 80.f	// 위아래 최대 각
#define SPINE_PITCHING_ANGLE 1.f		// 위아래 속도
#define PITCHING_MIN_UP_ANGLE 20.f		// 0도 이상, Value 값 이하일때 Value 각으로 설정
#define PITCHING_MIN_DOWN_ANGLE	15.f		// 0도 이하, Value 값 이상일때 Value 각으로 설정


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
	// 부모 Virtual Function
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
