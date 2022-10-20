#pragma once
#include "FcAdjutantObject.h"



class CFcAdjutantYesperratt : public CFcAdjutantObject
{
public:
	struct SOUND_FOR_ROOTS
	{
		int nHandle;
		int nPlayCount;
		SOUND_FOR_ROOTS(){	
			Init();
		}
		void Init()
		{
			nHandle = -1;
			nPlayCount = 3;
		}
	};

	CFcAdjutantYesperratt(CCrossVector *pCross);
	virtual ~CFcAdjutantYesperratt();

protected:
	void ProectileRoots( ASSignalData *pSignal );
	void ProjectileVolcano( ASSignalData *pSignal );
	void ProjectileNiddle( ASSignalData *pSignal );

//	CCrossVector m_VolcanoCross;
	D3DXVECTOR3 m_VolcanoPos;
	int m_nVolcanoRemainCount;
	int m_nVolcanoInterval;
	int m_nVolcanoRandomRange;
	int m_nVolcanoPrevTick;
	ASSignalData *m_pVolcanoSignal;
	SOUND_FOR_ROOTS m_RootsSound;


public:
	// ºÎ¸ð Virtual Function
	virtual void SignalCheck( ASSignalData *pSignal );
	virtual void Process();
};