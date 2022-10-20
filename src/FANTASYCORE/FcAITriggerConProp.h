#pragma once

#include "FcAITriggerBase.h"

class CFcAIConTargetPropRange : public CFcAIConditionBase
{
public:
	CFcAIConTargetPropRange();
	virtual bool CheckCondition();
};

class CFcAIConTargetPropValid : public CFcAIConditionBase
{
public:
	CFcAIConTargetPropValid( BOOL bValid );
	virtual bool CheckCondition();
protected:
	BOOL m_bValid;
};

class CFcAIConTargetPropType : public CFcAIConditionBase
{
public:
	CFcAIConTargetPropType();
	virtual bool CheckCondition();

};