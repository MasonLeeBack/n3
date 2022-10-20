#pragma once

#include "FcAIHardCodingFunction.h"
#include "FcGameObject.h"

class CFcAISearch;
class CFcAIHardCodingFunctionBattleMeleeType1 : public CFcAIHardCodingFunction
{
public:
	CFcAIHardCodingFunctionBattleMeleeType1();
	virtual ~CFcAIHardCodingFunctionBattleMeleeType1();

	virtual CFcAIHardCodingFunction *Clone() { return new CFcAIHardCodingFunctionBattleMeleeType1; };
	virtual void ExecuteFunction();
	virtual void Initialize();
	virtual void SetNullFuncParam( CFcParamVariable *pParam );

protected:
	CFcAISearch *m_pSearch;
	GameObjHandle m_hTarget;
	int m_nState;
};


class CFcAIHardCodingFunctionBattleMeleeTroopType1 : public CFcAIHardCodingFunction
{
public:
	CFcAIHardCodingFunctionBattleMeleeTroopType1();
	virtual ~CFcAIHardCodingFunctionBattleMeleeTroopType1();

	virtual CFcAIHardCodingFunction *Clone() { return new CFcAIHardCodingFunctionBattleMeleeTroopType1; };
	virtual void ExecuteFunction();
	virtual void Initialize();
	virtual void SetNullFuncParam( CFcParamVariable *pParam );

protected:
	CFcAISearch *m_pSearch;
	GameObjHandle m_hTarget;
	int m_nState;
};


class CFcAIHardCodingFunctionBattleIdle : public CFcAIHardCodingFunction
{
public:
	CFcAIHardCodingFunctionBattleIdle();
	virtual ~CFcAIHardCodingFunctionBattleIdle();

	virtual CFcAIHardCodingFunction *Clone() { return new CFcAIHardCodingFunctionBattleIdle; };
	virtual void ExecuteFunction();
	virtual void Initialize();
	virtual void SetNullFuncParam( CFcParamVariable *pParam );
};

class CFcAIHardCodingFunctionCorrelationLastHit : public CFcAIHardCodingFunction 
{
public:
	CFcAIHardCodingFunctionCorrelationLastHit();
	virtual ~CFcAIHardCodingFunctionCorrelationLastHit();

	virtual CFcAIHardCodingFunction *Clone() { return new CFcAIHardCodingFunctionCorrelationLastHit; };
	virtual void SetNullFuncParam( CFcParamVariable *pParam );
	virtual void Initialize();
	virtual void ExecuteFunction() {}
	virtual void ExecuteCallbackFunction( int nIndex = 0 );
	virtual AI_CALLBACK_TYPE GetCallbackType( int nIndex = 0 ) { return AI_CALLBACK_HIT; }
	
protected:
	CFcAISearch *m_pSearch;
	GameObjHandle m_hTarget;
};

class CFcAIHardCodingFunctionTroopCommand : public CFcAIHardCodingFunction
{
public:
	CFcAIHardCodingFunctionTroopCommand();
	virtual ~CFcAIHardCodingFunctionTroopCommand();

	virtual CFcAIHardCodingFunction *Clone() { return new CFcAIHardCodingFunctionTroopCommand; };
	virtual void SetNullFuncParam( CFcParamVariable *pParam );
	virtual void Initialize();
	virtual void ExecuteFunction();
private:
	bool DelayOrder(int nMaxDelay); // 딜레이중일때는 return true
};

class CFcAIHardCodingFunctionBattleHeroTargeting : public CFcAIHardCodingFunction
{
public:
	CFcAIHardCodingFunctionBattleHeroTargeting();
	virtual ~CFcAIHardCodingFunctionBattleHeroTargeting();

	virtual CFcAIHardCodingFunction *Clone() { return new CFcAIHardCodingFunctionBattleHeroTargeting; };
	virtual void SetNullFuncParam( CFcParamVariable *pParam );
	virtual void Initialize();
	virtual void ExecuteFunction();

protected:
	int m_nState;
	int m_nRandomMaxRange;
	CFcAISearch *m_pSearch;
	GameObjHandle m_hTarget;
};

class CFcAIHardCodingFunctionBattleMeleeAttack : public CFcAIHardCodingFunction
{
public:
	CFcAIHardCodingFunctionBattleMeleeAttack();
	virtual ~CFcAIHardCodingFunctionBattleMeleeAttack();

	virtual CFcAIHardCodingFunction *Clone() { return new CFcAIHardCodingFunctionBattleMeleeAttack; };
	virtual void SetNullFuncParam( CFcParamVariable *pParam );
	virtual void Initialize();
	virtual void ExecuteFunction();
};

class CFcAIHardCodingFunctionBattleMeleeAttackCombo : public CFcAIHardCodingFunction
{
public:
	CFcAIHardCodingFunctionBattleMeleeAttackCombo();
	virtual ~CFcAIHardCodingFunctionBattleMeleeAttackCombo();

	virtual CFcAIHardCodingFunction *Clone() { return new CFcAIHardCodingFunctionBattleMeleeAttackCombo; };
	virtual void SetNullFuncParam( CFcParamVariable *pParam );
	virtual void Initialize();
	virtual void ExecuteFunction();
};

class CFcAIHardCodingFunctionBattleMeleeAttackCheckLink : public CFcAIHardCodingFunction
{
public:
	CFcAIHardCodingFunctionBattleMeleeAttackCheckLink();
	virtual ~CFcAIHardCodingFunctionBattleMeleeAttackCheckLink();

	virtual CFcAIHardCodingFunction *Clone() { return new CFcAIHardCodingFunctionBattleMeleeAttackCheckLink; };
	virtual void SetNullFuncParam( CFcParamVariable *pParam );
	virtual void Initialize();
	virtual void ExecuteFunction();
};

class CFcAIHardCodingFunctionBattleMeleeAttackFirstBattle : public CFcAIHardCodingFunction
{
public:
	CFcAIHardCodingFunctionBattleMeleeAttackFirstBattle();
	virtual ~CFcAIHardCodingFunctionBattleMeleeAttackFirstBattle();

	virtual CFcAIHardCodingFunction *Clone() { return new CFcAIHardCodingFunctionBattleMeleeAttackFirstBattle; };
	virtual void SetNullFuncParam( CFcParamVariable *pParam );
	virtual void Initialize();
	virtual void ExecuteFunction();

protected:
	bool m_bFirstFlag;
};

class CFcAIHardCodingFunctionBattleRangeType1 : public CFcAIHardCodingFunction
{
public:
	CFcAIHardCodingFunctionBattleRangeType1();
	virtual ~CFcAIHardCodingFunctionBattleRangeType1();

	virtual CFcAIHardCodingFunction *Clone() { return new CFcAIHardCodingFunctionBattleRangeType1; };
	virtual void ExecuteFunction();
	virtual void Initialize();
	virtual void SetNullFuncParam( CFcParamVariable *pParam );

protected:
	CFcAISearch *m_pSearch;
	GameObjHandle m_hTarget;
	int m_nState;
};

class CFcAIHardCodingFunctionCounterAttack : public CFcAIHardCodingFunction
{
public:
	CFcAIHardCodingFunctionCounterAttack();
	virtual ~CFcAIHardCodingFunctionCounterAttack();

	virtual CFcAIHardCodingFunction *Clone() { return new CFcAIHardCodingFunctionCounterAttack; };
	virtual void ExecuteFunction() {}
	virtual void Initialize();
	virtual void SetNullFuncParam( CFcParamVariable *pParam ) {}
	virtual void ExecuteCallbackFunction( int nIndex = 0 );
	virtual AI_CALLBACK_TYPE GetCallbackType( int nIndex = 0 ) { return AI_CALLBACK_HIT; }

protected:
};


class CFcAIHardCodingFunctionRageHit : public CFcAIHardCodingFunction
{
public:
	CFcAIHardCodingFunctionRageHit();
	virtual ~CFcAIHardCodingFunctionRageHit();

	virtual void Initialize();
	virtual void ExecuteFunction();
	virtual void ExecuteCallbackFunction( int nIndex = 0 );
	virtual CFcAIHardCodingFunction *Clone() { return new CFcAIHardCodingFunctionRageHit; };
	virtual void SetNullFuncParam( CFcParamVariable *pParam ) {}

	virtual int GetCallbackTypeCount() { return 2; }
	virtual AI_CALLBACK_TYPE GetCallbackType( int nIndex = 0 ) { 
		switch(nIndex) { 
			case 0: return AI_CALLBACK_HIT;
			case 1:	return AI_CALLBACK_ATTACK;
		};
		return AI_CALLBACK_NONE;
	};

protected:
	/*
	bool m_bDiscrease;
	int m_nPrevDecreaseTick;
	*/
};

class CFcAIHardCodingFunctionRageTime : public CFcAIHardCodingFunction
{
public:
	CFcAIHardCodingFunctionRageTime();
	virtual ~CFcAIHardCodingFunctionRageTime();

	virtual void Initialize();
	virtual void ExecuteFunction();
	virtual void ExecuteCallbackFunction( int nIndex = 0 );
	virtual CFcAIHardCodingFunction *Clone() { return new CFcAIHardCodingFunctionRageTime; };
	virtual void SetNullFuncParam( CFcParamVariable *pParam ) {}

	virtual AI_CALLBACK_TYPE GetCallbackType( int nIndex = 0 ) { return AI_CALLBACK_ATTACK; }

protected:
//	bool m_bDiscrease;
//	int m_nPrevDecreaseTick;
	int m_nPrevIncreaseTick;
};

class CFcAIHardCodingFunctionRageTroopHP : public CFcAIHardCodingFunction
{
public:
	CFcAIHardCodingFunctionRageTroopHP();
	virtual ~CFcAIHardCodingFunctionRageTroopHP();

	virtual void Initialize();
	virtual void ExecuteFunction();
	virtual void ExecuteCallbackFunction( int nIndex = 0 );
	virtual CFcAIHardCodingFunction *Clone() { return new CFcAIHardCodingFunctionRageTroopHP; };
	virtual void SetNullFuncParam( CFcParamVariable *pParam ) {}

	virtual AI_CALLBACK_TYPE GetCallbackType( int nIndex = 0 ) { return AI_CALLBACK_ATTACK; }

protected:
	/*
	bool m_bDiscrease;
	int m_nPrevDecreaseTick;
	*/
	int m_nPrevTroopHPPercent;
};
