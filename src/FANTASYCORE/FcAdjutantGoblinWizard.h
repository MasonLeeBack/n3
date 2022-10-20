#pragma once
#include "FcAdjutantObject.h"

class CFcAdjutantGoblinWizard : public CFcAdjutantObject
{
public:
	CFcAdjutantGoblinWizard(CCrossVector *pCross);
	virtual ~CFcAdjutantGoblinWizard();

protected:

public:
	// ºÎ¸ð Virtual Function
	virtual void SignalCheck( ASSignalData *pSignal );
};