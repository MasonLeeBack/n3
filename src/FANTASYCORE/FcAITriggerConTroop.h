#pragma once

#include "FcAITriggerBase.h"

#include "FcAIObject.h"
#include "FcAIElement.h"


class CFcParamVariable;


class CFcAIConTroopState : public CFcAIConditionBase {
public:
	CFcAIConTroopState();
	virtual bool CheckCondition();
};

class CFcAICoTroopStateTable : public CFcAIConditionBase {
public:
	CFcAICoTroopStateTable();
	virtual bool CheckCondition();
};

class CFcAIConTroopOffsetRange : public CFcAIConditionBase {
public:
	CFcAIConTroopOffsetRange( BOOL bTarget );
	virtual bool CheckCondition();

protected:
	BOOL m_bTarget;
};

class CFcAIConTroopInRange : public CFcAIConditionBase {
public:
	CFcAIConTroopInRange( BOOL bTarget );
	virtual bool CheckCondition();
protected:
	BOOL m_bTarget;
};

class CFcAIConTroopSubState : public CFcAIConditionBase {
public:
	CFcAIConTroopSubState();
	virtual bool CheckCondition();
};

class CFcAIConTroopTargetTroopRange : public CFcAIConditionBase {
public:
	CFcAIConTroopTargetTroopRange();
	virtual bool CheckCondition();
};

class CFcAIConTroopTargetInTroop : public CFcAIConditionBase {
public:
	CFcAIConTroopTargetInTroop();
	virtual bool CheckCondition();
};

class CFcAIConTroopIsUntouchable : public CFcAIConditionBase {
public:
	CFcAIConTroopIsUntouchable();
	virtual bool CheckCondition();
};

class CFcAIConTroopReadyRangeOrder : public CFcAIConditionBase {
public:
	CFcAIConTroopReadyRangeOrder();
	virtual bool CheckCondition();
};

class CFcAIConTroopTargetIsPlayer : public CFcAIConditionBase {
public:
	CFcAIConTroopTargetIsPlayer();
	virtual bool CheckCondition();
};