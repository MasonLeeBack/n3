#pragma once

#include "FcGameObject.h"

class CFcRangeUnitObject : public CFcGameObject
{
public:
	CFcRangeUnitObject(CCrossVector *pCross);
	virtual ~CFcRangeUnitObject();

protected:

public:
	// �θ� Virtual Function
	virtual void Process();
	virtual void PostProcess();
};
