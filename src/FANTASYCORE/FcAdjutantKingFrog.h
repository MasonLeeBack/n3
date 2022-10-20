#pragma once
#include "FcAdjutantObject.h"

class CFcAdjutantKingFrog : public CFcAdjutantObject
{
public:
	CFcAdjutantKingFrog(CCrossVector *pCross);
	virtual ~CFcAdjutantKingFrog();

protected:
	static int s_nRotationAngle;

public:
	// �θ� Virtual Function
	virtual void SignalCheck( ASSignalData *pSignal );
	virtual void RotationProcess();

	virtual bool IsFlockable( GameObjHandle Handle );
	virtual void ProcessFlocking();
};