#pragma once

#include "FcAITriggerBase.h"

class CFcAIActionFollowProp : public CFcAIActionBase
{
public:
	CFcAIActionFollowProp();
	virtual int Command();
};

class CFcAIActionSetTargetSlotProp : public CFcAIActionBase
{
public:
	CFcAIActionSetTargetSlotProp();
	virtual int Command();
};
