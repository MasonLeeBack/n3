#pragma once
#include "FcAdjutantObject.h"

class CFcAdjutantDarkElf : public CFcAdjutantObject
{
public:
	CFcAdjutantDarkElf(CCrossVector *pCross);
	virtual ~CFcAdjutantDarkElf();

protected:

public:
	// ºÎ¸ð Virtual Function
	virtual void SignalCheck( ASSignalData *pSignal );
};