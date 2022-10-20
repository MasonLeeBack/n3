#pragma once
#include "FcAdjutantObject.h"

#define MAX_SPINE_PITCHING_ANGLE 80.f	// ���Ʒ� �ִ� ��
#define SPINE_PITCHING_ANGLE 1.f		// ���Ʒ� �ӵ�
#define PITCHING_MIN_UP_ANGLE 20.f		// 0�� �̻�, Value �� �����϶� Value ������ ����
#define PITCHING_MIN_DOWN_ANGLE	6.f		// 0�� ����, Value �� �̻��϶� Value ������ ����

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
	// �θ� Virtual Function
	virtual void SignalCheck( ASSignalData *pSignal );
};