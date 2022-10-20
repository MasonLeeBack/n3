#pragma once
#include "FcAdjutantObject.h"

class CFcAdjutantDwykfarrio : public CFcAdjutantObject
{
public:
	CFcAdjutantDwykfarrio(CCrossVector *pCross);
	virtual ~CFcAdjutantDwykfarrio();

protected:

public:
	// ºÎ¸ð Virtual Function
	virtual void SignalCheck( ASSignalData *pSignal );
};

