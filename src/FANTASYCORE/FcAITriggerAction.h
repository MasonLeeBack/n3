#pragma		once

#include "FcAITriggerBase.h"

class CFcAIActionDelay : public CFcAIActionBase
{
public:
	CFcAIActionDelay();
	virtual int Command();
};

class CFcAIActionSetAni : public CFcAIActionBase
{
public:
	CFcAIActionSetAni( BOOL bLoop = FALSE );
	virtual int Command();
protected:
	BOOL m_bLoop;
};

class CFcAIActionFollowTarget : public CFcAIActionBase
{
public:
	CFcAIActionFollowTarget();
	virtual int Command();
};

class CFcAIActionOppositionTarget: public CFcAIActionBase
{
public:
	CFcAIActionOppositionTarget( BOOL bTarget );
	virtual int Command();

protected:
	BOOL m_bTarget;
};

class CFcAIActionStop : public CFcAIActionBase
{
public:
	CFcAIActionStop();
	virtual int Command();
};

class CFcAIActionLookTarget : public CFcAIActionBase
{
public:
	CFcAIActionLookTarget();
	virtual int Command();
};


class CFcAIActionChangeGlobalVariable : public CFcAIActionBase
{
public:
	CFcAIActionChangeGlobalVariable( int nValueType );
	virtual int Command();

protected:
	int m_nValueType;
	int m_nValueIndex;
};

class CFcAIActionGlobalVariableCounter : public CFcAIActionBase
{
public:
	CFcAIActionGlobalVariableCounter( int nValueType );
	virtual int Command();

protected:
	int m_nValueType;
	int m_nValueIndex;
};


class CFcAIActionSetTargetGlobalValue : public CFcAIActionBase
{
public:
	CFcAIActionSetTargetGlobalValue();
	virtual int Command();
};

class CFcAIActionSetTargetSlot : public CFcAIActionBase
{
public:
	CFcAIActionSetTargetSlot();
	virtual int Command();
};

class CFcAIActionEmptySlot : public CFcAIActionBase
{
public:
	CFcAIActionEmptySlot();
	virtual int Command();
};

class CFcAIActionDebugString : public CFcAIActionBase
{
public:
	CFcAIActionDebugString();
	virtual int Command();
};

class CFcAIActionLookSlot : public CFcAIActionBase
{
public:
	CFcAIActionLookSlot();
	virtual int Command();
};

class CFcAIActionInfluenceTargeting : public CFcAIActionBase
{
public:
	CFcAIActionInfluenceTargeting( BOOL bEmptyAll );
	virtual int Command();
protected:
	BOOL m_bEmptyAll;
};

class CFcAIActionInfluenceTargetingSlot : public CFcAIActionBase
{
public:
	CFcAIActionInfluenceTargetingSlot();
	virtual int Command();
};

class CFcAIActionCmd : public CFcAIActionBase
{
public:
	CFcAIActionCmd( int nCmdType );
	virtual int Command();
protected:
	int m_nCmdType;
};

class CFcAIActionCustomMove : public CFcAIActionBase
{
public:
	CFcAIActionCustomMove();
	virtual int Command();
};

class CFcAIActionExit : public CFcAIActionBase
{
public:
	CFcAIActionExit();
	virtual int Command();
};

class CFcAIActionPushButton : public CFcAIActionBase
{
public:
	CFcAIActionPushButton();
	virtual int Command();
};

class CFcAIActionReleaseButton : public CFcAIActionBase
{
public:
	CFcAIActionReleaseButton();
	virtual int Command();
};

class CFcAIActionAddDefenseProb : public CFcAIActionBase
{
public:
	CFcAIActionAddDefenseProb();
	virtual int Command();
};

class CFcAIActionDestroyElement : public CFcAIActionBase
{
public:
	CFcAIActionDestroyElement();
	virtual int Command();
};

class CFcAIActionLinkObject : public CFcAIActionBase
{
public:
	CFcAIActionLinkObject();
	virtual int Command();
};

class CFcAIActionUnlinkObject : public CFcAIActionBase
{
public:
	CFcAIActionUnlinkObject();
	virtual int Command();
};

class CFcAIActionSetArcherHitProb : public CFcAIActionBase
{
public:
	CFcAIActionSetArcherHitProb();
	virtual int Command();
};

class CFcAIActionSetControlIndex : public CFcAIActionBase
{
public:
	CFcAIActionSetControlIndex();
	virtual int Command();
};

class CFcAIActionInfluenceDamageEmpty : public CFcAIActionBase
{
public:
	CFcAIActionInfluenceDamageEmpty();
	virtual int Command();

};

class CFcAIActionInfluenceHitCountEmpty : public CFcAIActionBase
{
public:
	CFcAIActionInfluenceHitCountEmpty();
	virtual int Command();
};
	
class CFcAIActionGlobalVariableIntRandom : public CFcAIActionBase
{
public:
	CFcAIActionGlobalVariableIntRandom( int nValueCount );
	virtual int Command();
protected:
	int m_nValueCount;
	int m_nValueIndex;
};

class CFcAIActionLookCustom : public CFcAIActionBase
{
public:
	CFcAIActionLookCustom();
	virtual int Command();
};

class CFcAIActionSelfCustomMove : public CFcAIActionBase
{
public:
	CFcAIActionSelfCustomMove();
	virtual int Command();
};

class CFcAIActionDelayOrder : public CFcAIActionBase 
{
public:
	CFcAIActionDelayOrder();
	virtual int Command();
};

class CFcAIActionPauseFunction : public CFcAIActionBase
{
public:
	CFcAIActionPauseFunction();
	virtual int Command();
protected:
	std::vector<int> m_nVecFunctionList;
	bool m_bFind;
};

class CFcAIActionUnpauseFunction : public CFcAIActionBase
{
public:
	CFcAIActionUnpauseFunction();
	virtual int Command();
protected:
	std::vector<int> m_nVecFunctionList;
	bool m_bFind;
};

class CFcAIActionWarp: public CFcAIActionBase 
{
public:
	CFcAIActionWarp();
	virtual int Command();
};

class CFcAIActionDiminutionVelocity : public CFcAIActionBase
{
public:
	CFcAIActionDiminutionVelocity();
	virtual int Command();
};

class CFcAIActionResetMoveVector : public CFcAIActionBase
{
public:
	CFcAIActionResetMoveVector();
	virtual int Command();
};