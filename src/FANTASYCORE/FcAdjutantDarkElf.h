#pragma once
#include "FcAdjutantObject.h"

class CFcAdjutantDarkElf : public CFcAdjutantObject
{
public:
	CFcAdjutantDarkElf(CCrossVector *pCross);
	virtual ~CFcAdjutantDarkElf();

protected:

public:
	// �θ� Virtual Function
	virtual void SignalCheck( ASSignalData *pSignal );
};