#ifndef __FC_TRIGGER_CONDTION_H__
#define __FC_TRIGGER_CONDTION_H__

#include "BSTriggerCondition.h"

class CFcConTimeElapsed : public CBSConditionActionBase
{
public:
	void Initialize();
	bool IsTrue();
protected:
	__int64      m_Frequency;
	__int64      m_BeginTime;
	__int64		 m_EndTime;
	int			 m_nCount;
};

class CFcTroopArrive : public CBSConditionActionBase
{
public:
	bool IsTrue();
};


//--------------------------------------------------------------------
class CFcConInputStickA : public CBSConditionActionBase
{
public:
	void Initialize();
	bool IsTrue();
};


class CFcConInputStickLeft : public CBSConditionActionBase
{
public:
	void Initialize();
	bool IsTrue();
};

class CFcConInputStickRight : public CBSConditionActionBase
{
public:
	void Initialize();
	bool IsTrue();
};

class CFcInputStickNeutral : public CBSConditionActionBase
{
public:
	void Initialize();
	bool IsTrue();
};

class CFcConCompareOrb : public CBSConditionActionBase
{
public:
	void Initialize();
	bool IsTrue();
};


class CFcConTimeElapsedFromMark : public CBSConditionActionBase
{
public:
	void Initialize();
	bool IsTrue();
};

class CFcConAttacked : public CBSConditionActionBase  //트룹이 공격당하고 있을때 (Melee / Range 구분없이)
{
public:
	void Initialize();
	bool IsTrue();
};

class CFcConAttackedTroop : public CBSConditionActionBase
{
public:
	void Initialize();
	bool IsTrue();
};

class CFcConCompareTroopDist : public CBSConditionActionBase
{
public:
	void Initialize();
	bool IsTrue();
};

class CFcConCompareTroopHP : public CBSConditionActionBase
{
public:
	void Initialize();
	bool IsTrue();
};

class CFcConTroopInArea : public CBSConditionActionBase
{
public:
	void Initialize();
	bool IsTrue();
};

class CFcConCompareLeaderHP : public CBSConditionActionBase
{
public:
	void Initialize();
	bool IsTrue();
};

class CFcConTroopMeleeAttacked : public CBSConditionActionBase
{
public:
	void Initialize();
	bool IsTrue();
};

class CFcConTroopRangeAttacked : public CBSConditionActionBase
{
public:
	void Initialize();
	bool IsTrue();
};

class CFcConUnBlockableAttacked : public CBSConditionActionBase
{
public:
	void Initialize();
	bool IsTrue();
};

class CFcConTroopNoEngaged : public CBSConditionActionBase
{
public:
	void Initialize();
	bool IsTrue();
};

class CFcConTroopNotInArea : public CBSConditionActionBase
{
public:
	void Initialize();
	bool IsTrue();
};

class CFcConVar : public CBSConditionActionBase
{
public:
	void Initialize();
	bool IsTrue();
};

class CFcConPropWasDestroyed : public CBSConditionActionBase
{
public:
	bool IsTrue();
};


class CFcConTroopEngaged : public CBSConditionActionBase
{
public:
	bool IsTrue();
};


class CFcConForceCompareHP : public CBSConditionActionBase
{
public:
	bool IsTrue();
};

class CFcConForceEliminated : public CBSConditionActionBase
{
public:
	bool IsTrue();
};

class CFcConForceInArea : public CBSConditionActionBase
{
public:
	bool IsTrue();
};

class CFcConForceNotInArea : public CBSConditionActionBase
{
public:
	bool IsTrue();
};

class CFcConForceAllInArea : public CBSConditionActionBase
{
public:
	bool IsTrue();
};

class CFcConPlayerGuardianNum : public CBSConditionActionBase
{
public:
	bool IsTrue();
};


class CFcConTroopType : public CBSConditionActionBase
{
public:
	bool IsTrue();
};

class CFcConGuardianTroopType : public CBSConditionActionBase
{
public:
	bool IsTrue();
};


class CFcConTroopLevel : public CBSConditionActionBase
{
public:
	bool IsTrue();
};


class CFcConOrbGaugeFull : public CBSConditionActionBase
{
public:
	bool IsTrue();
};


class CFcConTrueOrbGaugeFull : public CBSConditionActionBase
{
public:
	bool IsTrue();
};

class CFcConTotalKillNum : public CBSConditionActionBase 
{
public:
	bool IsTrue();
};

class CFcConForceKillNum : public CBSConditionActionBase 
{
public:
	bool IsTrue();
};

class CFcConGuardianInArea : public CBSConditionActionBase 
{
public:
	bool IsTrue();
};

class CFcConGuardianNotInArea : public CFcConGuardianInArea 
{
public:
	bool IsTrue();
};

class CFcConGuardianCompareDist : public CBSConditionActionBase 
{
public:
	bool IsTrue();
};

class CFcConGuardianExist : public CBSConditionActionBase 
{
public:
	bool IsTrue();
};

class CFcConGuardianTroopType2  : public CBSConditionActionBase 
{
public:
	bool IsTrue();
};

class CFcConSelectMissionMenuValue : public CBSConditionActionBase 
{
public:
	bool IsTrue();
};

class CFcConForceCompareUnitNumber : public CBSConditionActionBase 
{
public:
	bool IsTrue();
};

class CFcConTroopCompareUnitNumber : public CBSConditionActionBase 
{
public:
	bool IsTrue();
};

class CFcConRealmovieFinish : public CBSConditionActionBase
{
public:
	bool IsTrue();
};


class CFcConCompareTroopNForceDist : public CBSConditionActionBase
{
public:
	bool IsTrue();
};

class CFcConCompareForceNForceDist : public CBSConditionActionBase
{
public:
	bool IsTrue();
};


class CFcConVarCompare : public CBSConditionActionBase
{
public:
	void Initialize();
	bool IsTrue();
};


class CFcConTroopAttackedForce : public CBSConditionActionBase
{
public:
	bool IsTrue();
};

class CFcConTroopAttackedStr : public CBSConditionActionBase
{
public:
	bool IsTrue();
};

class CFcConItemBoxBroken : public CBSConditionActionBase
{
public:
	bool IsTrue();
};


class CFcConPropHP : public CBSConditionActionBase
{
public:
	bool IsTrue();
};

class CFcConIsDemoSkip : public CBSConditionActionBase
{
public:
	bool IsTrue();
};
class CFcConTroopCompareHPExceptLeader : public CBSConditionActionBase
{
public:
	bool IsTrue();
};





#endif