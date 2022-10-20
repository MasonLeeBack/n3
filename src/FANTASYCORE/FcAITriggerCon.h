#pragma once

#include "FcAITriggerBase.h"

#include "FcAIObject.h"
#include "FcAIElement.h"


class CFcParamVariable;
class CFcAIConTargetTeamCheck : public CFcAIConditionBase
{
public:
	CFcAIConTargetTeamCheck( BOOL bSame );
	virtual bool CheckCondition();

protected:
	BOOL m_bSame;
};

class CFcAIConTargetLiveCheck : public CFcAIConditionBase
{
public:
	CFcAIConTargetLiveCheck( BOOL bSame );
	virtual bool CheckCondition();

protected:
	BOOL m_bSame;
};

class CFcAIConSelfLiveCheck : public CFcAIConditionBase
{
public:
	CFcAIConSelfLiveCheck( BOOL bSame );
	virtual bool CheckCondition();

protected:
	BOOL m_bSame;
};


class CFcAIConUnitState : public CFcAIConditionBase
{
public:
	CFcAIConUnitState( BOOL bTarget, int nStateIndex );
	virtual bool CheckCondition();

protected:
	BOOL m_bTarget;
	int m_nStateIndex;
};

class CFcAIConTargetRange : public CFcAIConditionBase
{
public:
	CFcAIConTargetRange();
	virtual bool CheckCondition();
};

class CFcAIConEnemySearchCount : public CFcAIConditionBase
{
public:
	CFcAIConEnemySearchCount();
	virtual bool CheckCondition();

protected:
	static std::vector<GameObjHandle> m_VecList;
};

class CFcAIConGlobalVariable : public CFcAIConditionBase
{
public:
	CFcAIConGlobalVariable( int nValueType );
	virtual bool CheckCondition();

protected:
	int m_nValueIndex;
	int m_nValueType;
};

class CFcAIConCheckSlot : public CFcAIConditionBase
{
public:
	CFcAIConCheckSlot( BOOL bValid );
	virtual bool CheckCondition();
protected:
	BOOL m_bValid;
};

class CFcAIConCanAni : public CFcAIConditionBase
{
public:
	CFcAIConCanAni( BOOL bValid );
	virtual bool CheckCondition();

protected:
	BOOL m_bValid;
};

class CFcAIConAniType : public CFcAIConditionBase
{
public:
	CFcAIConAniType( BOOL bTarget );
	virtual bool CheckCondition();

protected:
	BOOL m_bTarget;
};

class CFcAIConDebugString : public CFcAIConditionBase
{
public:
	CFcAIConDebugString();
	virtual bool CheckCondition();
};

class CFcAIConFunction : public CFcAIConditionBase
{
public:
	CFcAIConFunction();
	virtual bool CheckCondition();
};

class CFcAIConInfluenceTargetingCount : public CFcAIConditionBase
{
public:
	CFcAIConInfluenceTargetingCount();
	virtual bool CheckCondition();
};

class CFcAIConJobIndex : public CFcAIConditionBase
{
public:
	CFcAIConJobIndex();
	virtual bool CheckCondition();
};

class CFcAIConClassID : public CFcAIConditionBase
{
public:
	CFcAIConClassID();
	virtual bool CheckCondition();
};

class CFcAIConLinkClassID : public CFcAIConditionBase
{
public:
	CFcAIConLinkClassID();
	virtual bool CheckCondition();
};

class CFcAIConWeapon : public CFcAIConditionBase
{
public:
	CFcAIConWeapon( BOOL bShow );
	virtual bool CheckCondition();
protected:
	BOOL m_bShow;
};

class CFcAIConIsValidLink : public CFcAIConditionBase
{
public:
	CFcAIConIsValidLink( BOOL bValid );
	virtual bool CheckCondition();
protected:
	BOOL m_bValid;
};

class CFcAIConTargetViewAngle : public CFcAIConditionBase
{
public:
	CFcAIConTargetViewAngle();
	virtual bool CheckCondition();
};


class CFcAIConTargetValid : public CFcAIConditionBase
{
public:
	CFcAIConTargetValid( BOOL bValid );
	virtual bool CheckCondition();
protected:
	BOOL m_bValid;
};

class CFcAIConIsProcessTrueOrb : public CFcAIConditionBase
{
public:
	CFcAIConIsProcessTrueOrb();
	virtual bool CheckCondition();

};

class CFcAIConEventSeqCount : public CFcAIConditionBase
{
public:
	CFcAIConEventSeqCount( BOOL bTarget );
	virtual bool CheckCondition();
protected:
	BOOL m_bTarget;
};

class CFcAIConControlIndex : public CFcAIConditionBase
{
public:
	CFcAIConControlIndex();
	virtual bool CheckCondition();
};

class CFcAIConInfluenceDamage : public CFcAIConditionBase
{
public:
	CFcAIConInfluenceDamage();
	virtual bool CheckCondition();
};

class CFcAIConInfluenceHitCount : public CFcAIConditionBase
{
public:
	CFcAIConInfluenceHitCount();
	virtual bool CheckCondition();
};

class CFcAIConCheckCatch : public CFcAIConditionBase
{
public:
	CFcAIConCheckCatch( BOOL bValid );
	virtual bool CheckCondition();
protected:
	BOOL m_bValid;
};