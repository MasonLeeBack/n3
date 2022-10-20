#pragma once
#include "FcUnitObject.h"

class CFcUnit9SObject : public CFcUnitObject
{
public:
	CFcUnit9SObject(CCrossVector *pCross);
	virtual ~CFcUnit9SObject();

public:
	// ºÎ¸ð Virtual Function
	virtual void SignalCheck( ASSignalData *pSignal );
};