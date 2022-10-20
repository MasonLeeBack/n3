#pragma once
#include "FcAdjutantObject.h"
class CFcAdjutantTeaTea : public CFcAdjutantObject
{
public:
	CFcAdjutantTeaTea(CCrossVector *pCross);
	virtual ~CFcAdjutantTeaTea();

protected:

public:
	// ºÎ¸ð Virtual Function
	virtual void SignalCheck( ASSignalData *pSignal );
};