#pragma once

#include "FcGameObject.h"

class CFcRangeUnitObject : public CFcGameObject
{
public:
	CFcRangeUnitObject(CCrossVector *pCross);
	virtual ~CFcRangeUnitObject();

protected:

public:
	// ºÎ¸ð Virtual Function
	virtual void Process();
	virtual void PostProcess();
};
