#pragma once
#include "FcAdjutantObject.h"
class CFcAdjutantTeaTea : public CFcAdjutantObject
{
public:
	CFcAdjutantTeaTea(CCrossVector *pCross);
	virtual ~CFcAdjutantTeaTea();

protected:

public:
	// �θ� Virtual Function
	virtual void SignalCheck( ASSignalData *pSignal );
};