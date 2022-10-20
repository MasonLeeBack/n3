#pragma once
#include "FcAdjutantObject.h"

#define MAX_SPINE_PITCHING_ANGLE 80.f	// 위아래 최대 각
#define SPINE_PITCHING_ANGLE 1.f		// 위아래 속도
#define PITCHING_MIN_UP_ANGLE 20.f		// 0도 이상, Value 값 이하일때 Value 각으로 설정
#define PITCHING_MIN_DOWN_ANGLE	6.f		// 0도 이하, Value 값 이상일때 Value 각으로 설정

class CFcAdjutantSyumerrt : public CFcAdjutantObject
{
public:
	CFcAdjutantSyumerrt(CCrossVector *pCross);
	virtual ~CFcAdjutantSyumerrt();
	

	virtual void Initialize( INITGAMEOBJECTDATA* pData, int nTeam, int nGroup, TroopObjHandle hTroop, float fScaleMin=1.f, float fScaleMax=1.f );

protected:
	int		m_nProjectileFXIdx;

	void CreateMeleeProjectile( ASSignalData *pSignal );
	void CreateRangeProjectile( ASSignalData *pSignal );
	
	/*
	void Create1kProjectile( ASSignalData *pSignal );
	void CreateMultiProjectile( ASSignalData *pSignal );
	void CreatePierceProjectile( ASSignalData *pSignal );
	*/

public:
	// 부모 Virtual Function
	virtual void SignalCheck( ASSignalData *pSignal );
};