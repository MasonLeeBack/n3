#ifndef __FC_TRIGGER_ACTION_H__
#define __FC_TRIGGER_ACTION_H__
#include "BSTriggerAction.h"



class CFcActAIDisable : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};




class CFcActAIEnable : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActAISet : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};


class CFcActAISetPath : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActBGMFade : public CBSConditionActionBase
{
public:
	void Initialize(){}
	int DoCommand();
};



class CFcActBGMMute : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActBGMPlay : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActBGMStop : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActBGMVolume : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActCamReset : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActCamSet : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActCamTargetArea : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActCamTargetTroop : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActDelay : public CBSConditionActionBase
{
public:
	void Initialize();
	int DoCommand();
protected:
	__int64 m_BeginTime;
	__int64 m_nCount;
};



class CFcActDemoDelay : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActEffectFadeIn : public CBSConditionActionBase
{
public:
    void Initialize(){m_bStart = false;}
	int DoCommand();
protected:
    bool m_bStart;
};



class CFcActEffectFadeInColor : public CBSConditionActionBase
{
public:
	void Initialize(){m_bStart = false;}
	int DoCommand();
protected:
    bool m_bStart;
};



class CFcActEffectFadeOut : public CBSConditionActionBase
{
public:
	void Initialize(){m_bStart = false;}
	int DoCommand();
protected:
    bool m_bStart;
};



class CFcActEffectFadeOutColor : public CBSConditionActionBase
{
public:
	void Initialize(){m_bStart = false;}
	int DoCommand();
protected:
    bool m_bStart;
};



class CFcActEffectFog : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActEffectFXRandomSet : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActEffectFXSet : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActEffectGlowOff : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActEffectGlowOn : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActEffectLensflareOff : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActEffectLensflareOn : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActEffectLight : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActEffectMotionblurOff : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActEffectMotionblurOn : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActEffectRainOff : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActEffectRainOn : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActEffectSetFire : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActEffectSnowOff : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActEffectSnowOn : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActGateClose : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActGateOpen : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActHookInputOff : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActHookInputOn : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActInterfaceOff : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActInterfaceOn : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActLetterBoxOff : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActLetterBoxOn : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActMapOff : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActMapOn : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActMapScale : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActMessageReport : public CBSConditionActionBase
{
public:
	void Initialize();
	int DoCommand();
protected:
	bool m_bPlayed;
};



class CFcActMessageSay : public CBSConditionActionBase
{
public:
	void Initialize();
	int DoCommand();
protected:
	bool m_bPlayed;
};



class CFcActMessageShow : public CBSConditionActionBase
{
public:
	void Initialize();
	int DoCommand();
protected:
	bool m_bShow;
};



class CFcActMessageShowXY : public CBSConditionActionBase
{
public:
	void Initialize();
	int DoCommand();
protected:
	bool m_bShow;
};



class CFcActMissionCompleted : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActMissionFailed : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActOrbAdd : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActPointAreaOff : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActPointAreaOn : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActPointFriendInArea : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActPointHostileInArea : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActPointTroopOFf : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActPointTroopOn : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActSoundSet : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActSoundUnset : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActTimeMark : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActTriggerActivate : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActTriggerDeactivate : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActTriggerLoop : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActTriggerReset : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActTriggerResetAll : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActTriggerResetThis : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActTroopAnimation : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActTroopAnnihilate : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActTroopAttack : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActTroopAttackLeader : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActTroopDisable : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActTroopDisableAll : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActTroopDisableInArea : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActTroopEnable : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActEnableInArea : public CBSConditionActionBase
{
public:
	void Initialize() { m_nID = -1; m_bFirst = true; }
	int DoCommand();

protected:
	bool m_bFirst;
	int m_nID;
};



class CFcActEnemyDisableInArea : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActEnemyEnableInArea : public CBSConditionActionBase
{
public:
	void Initialize() { m_nID = -1; m_bFirst = true; }
	int DoCommand();

protected:
	bool m_bFirst;
	int m_nID;
};



class CFcActTroopFllow : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActTroopFriendDisableInArea : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();


};



class CFcActTroopFriendEnableInArea : public CBSConditionActionBase
{
public:
	void Initialize() { m_nID = -1; m_bFirst = true; }
	int DoCommand();

protected:
	bool m_bFirst;
	int m_nID;
};



class CFcActTroopHPFill : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActTroopHPMAX : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActTroopInvulnerable : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActTroopLeaderInvulnerable : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActTroopLeaderVulnerable : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActTroopPlace : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActTroopRangeAttackArea : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActTroopRangeAttackWall : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActTroopRenew : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActTroopRenewOutOfSight : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActTroopRun : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActTroopStop : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActTroopVulnerable : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActTroopWalk : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActVarIncrease : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActVarSet : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActWallCollapse : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};



class CFcActWallSetHP : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};


class CFcPropPush : public CBSConditionActionBase
{
public:
	void Initialize() {}
	int DoCommand();
};


class CFcDestroyBridge : public CBSConditionActionBase
{
public:
	void Initialize()	{}
	int DoCommand();
};


class CFcSetGuardianTroop : public CBSConditionActionBase
{
public:
	void Initialize()	{}
	int DoCommand();
};

class CFcActSetEnemyAttr : public CBSConditionActionBase
{
public:
	void Initialize()	{}
	int DoCommand();
};

class CFcActResetEnemyAttr : public CBSConditionActionBase
{
public:
	void Initialize()	{}
	int DoCommand();
};


class CFcActEnableDynamicProp : public CBSConditionActionBase
{
public:
	void Initialize()	{}
	int DoCommand();
};


class CFcActDisableDynamicProp : public CBSConditionActionBase
{
public:
	void Initialize()	{}
	int DoCommand();
};


class CFcPlayRealtimeMovie : public CBSConditionActionBase
{
public:
	void Initialize();//	{ m_bStart = false; }
	int DoCommand();

//	bool m_bStart;
};

class CFcTroopHPGaugeShow : public CBSConditionActionBase
{
public:
	void Initialize()	{}
	int DoCommand();
};




/*
class CFcActHeroMove : public CBSConditionActionBase
{
public:
	CFcActHeroMove();
	void Initialize();
	int DoCommand();
};


class CFcActTroopMove : public CBSConditionActionBase
{
public:
	CFcActTroopMove();
	void Initialize();
	int DoCommand();
};


class CFcSetTroopAI : public CBSConditionActionBase
{
public:
	CFcSetTroopAI(){};
	int DoCommand();
};

class CFcSetUnitAI : public CBSConditionActionBase
{
public:
	CFcSetUnitAI(){};
	int DoCommand();
};

class CFcSetBossAI : public CBSConditionActionBase
{
public:
	CFcSetBossAI(){};
	int DoCommand();
};
*/

class CTroopTypeAIDisableInArea : public CBSConditionActionBase
{
public:
	CTroopTypeAIDisableInArea(){};
	int DoCommand();
};


class CTroopTypeAIEnableInArea : public CBSConditionActionBase
{
public:
	CTroopTypeAIEnableInArea(){};
	int DoCommand();
};

class CTroopTypeAISetInArea : public CBSConditionActionBase
{
public:
	CTroopTypeAISetInArea(){};
	int DoCommand();
};

class CTroopTypeAnimationInArea : public CBSConditionActionBase
{
public:
	CTroopTypeAnimationInArea(){};
	int DoCommand();
};

class CTroopTypeKillAllInArea : public CBSConditionActionBase
{
public:
	CTroopTypeKillAllInArea(){};
	int DoCommand();
};

class CTroopTypeDIsableInArea : public CBSConditionActionBase
{
public:
	CTroopTypeDIsableInArea(){};
	int DoCommand();
};

class CTroopTypeHPFillInArea : public CBSConditionActionBase
{
public:
	CTroopTypeHPFillInArea(){};
	int DoCommand();
};

class CTroopTypeHPMaxInArea : public CBSConditionActionBase
{
public:
	CTroopTypeHPMaxInArea(){};
	int DoCommand();
};

class CTroopTypeWalk : public CBSConditionActionBase
{
public:
	CTroopTypeWalk(){};
	int DoCommand();
};

class CTroopTypeRun : public CBSConditionActionBase
{
public:
	CTroopTypeRun(){};
	int DoCommand();
};

class CTroopTypeStopInArea : public CBSConditionActionBase
{
public:
	CTroopTypeStopInArea(){};
	int DoCommand();
};

class CTroopRangeAttackToArea : public CBSConditionActionBase
{
public:
	CTroopRangeAttackToArea(){};
	int DoCommand();
};


class CTroopTypeSetDirection : public CBSConditionActionBase
{
public:
	CTroopTypeSetDirection(){};
	int DoCommand();
};

class CTroopTypeShowInMinimap : public CBSConditionActionBase
{
public:
	CTroopTypeShowInMinimap(){};
	int DoCommand();
};

class CTroopTypeHideInMinimap : public CBSConditionActionBase
{
public:
	CTroopTypeHideInMinimap(){};
	int DoCommand();
};

class CTroopShowInMinimap : public CBSConditionActionBase
{
public:
	CTroopShowInMinimap(){};
	int DoCommand();
};

class CTroopHideInMinimap : public CBSConditionActionBase
{
public:
	CTroopHideInMinimap(){};
	int DoCommand();
};

class CUnitOnPropShowInMinimap : public CBSConditionActionBase
{
public:
	CUnitOnPropShowInMinimap(){};
	int DoCommand();
};

class CUnitOnPropHideInMinimap : public CBSConditionActionBase
{
public:
	CUnitOnPropHideInMinimap(){};
	int DoCommand();
};



class CFcActPlayerAddOrb : public CBSConditionActionBase
{
public:
	CFcActPlayerAddOrb() {};
	int DoCommand();
};

class CFcActTeamShowHPGauge : public CBSConditionActionBase
{
public:
	CFcActTeamShowHPGauge() {};
	int DoCommand();
};

class CFcActTeamShowHPGaugeInArea : public CBSConditionActionBase
{
public:
	CFcActTeamShowHPGaugeInArea() {};
	int DoCommand();
};

class CFcActTroopsAIEnable : public CBSConditionActionBase
{
public:
	CFcActTroopsAIEnable() {};
	int DoCommand();
};

class CFcActTroopsAIDisable : public CBSConditionActionBase
{
public:
	CFcActTroopsAIDisable() {};
	int DoCommand();
};

class CFcActTroopsSetAI : public CBSConditionActionBase
{
public:
	CFcActTroopsSetAI() {};
	int DoCommand();
};

class CFcActTroopsPlayAction : public CBSConditionActionBase
{
public:
	CFcActTroopsPlayAction() {};
	int DoCommand();
};

class CFcActTroopsAnnihilate : public CBSConditionActionBase
{
public:
	CFcActTroopsAnnihilate() {};
	int DoCommand();
};

class CFcActTroopsDisable : public CBSConditionActionBase
{
public:
	CFcActTroopsDisable() {};
	int DoCommand();
};


class CFcActTroopsHPFill : public CBSConditionActionBase
{
public:
	CFcActTroopsHPFill() {};
	int DoCommand();
};

class CFcActTroopsHPMax : public CBSConditionActionBase
{
public:
	CFcActTroopsHPMax() {};
	int DoCommand();
};

class CFcActTroopsInvulnerable : public CBSConditionActionBase
{
public:
	CFcActTroopsInvulnerable() {};
	int DoCommand();
};

class CFcActTroopsVulnerable : public CBSConditionActionBase
{
public:
	CFcActTroopsVulnerable() {};
	int DoCommand();
};

class CFcActTroopsWalk : public CBSConditionActionBase
{
public:
	CFcActTroopsWalk() {};
	int DoCommand();
};

class CFcActTroopsRun : public CBSConditionActionBase
{
public:
	CFcActTroopsRun() {};
	int DoCommand();
};

class CFcActTroopsStop : public CBSConditionActionBase
{
public:
	CFcActTroopsStop() {};
	int DoCommand();
};

class CFcActTroopsRangeAttackArea : public CBSConditionActionBase
{
public:
	CFcActTroopsRangeAttackArea() {};
	int DoCommand();
};

class CFcActTroopsSetDir : public CBSConditionActionBase
{
public:
	CFcActTroopsSetDir() {};
	int DoCommand();
};

class CFcActTroopsShowMinimap : public CBSConditionActionBase
{
public:
	CFcActTroopsShowMinimap() {};
	int DoCommand();
};

class CFcActTroopsShowHPGauge : public CBSConditionActionBase
{
public:
	CFcActTroopsShowHPGauge() {};
	int DoCommand();
};

class CFcActTroopsChangeGroup : public CBSConditionActionBase
{
public:
	CFcActTroopsChangeGroup() {};
	int DoCommand();
};

class CFcActDynamicFogEnable : public CBSConditionActionBase
{
public:
	CFcActDynamicFogEnable() {};
	int DoCommand();

};

class CFcActDynamicFogCross : public CBSConditionActionBase
{
public:
	CFcActDynamicFogCross() {};
	int DoCommand();

};


class CFcActSetDOF : public CBSConditionActionBase
{
public:
	CFcActSetDOF() {};
	int DoCommand();
};

class CFcActDisableDOF : public CBSConditionActionBase
{
public:
	CFcActDisableDOF() {};
	int DoCommand();
};


class CFcActDestroyCrumbleStone : public CBSConditionActionBase
{
public:
	CFcActDestroyCrumbleStone() {};
	int DoCommand();
};

class CFcActDynamicFogDisable : public CBSConditionActionBase
{
public:
	CFcActDynamicFogDisable() {};
	int DoCommand();
};



class CFcActSetPropAIGuide : public CBSConditionActionBase
{
public:
	CFcActSetPropAIGuide() {};
	int DoCommand();
};

class CFcActSetAreaAIGuide : public CBSConditionActionBase
{
public:
	CFcActSetAreaAIGuide() {};
	int DoCommand();
};

class CFcActSetGuardianAI : public CBSConditionActionBase
{
public:
	CFcActSetGuardianAI() {};
	int DoCommand();
};

class CFcActMoveMoveTower : public CBSConditionActionBase
{
public:
	CFcActMoveMoveTower() {};
	int DoCommand();
};

class CFcActAddObjective : public CBSConditionActionBase
{
public:
	CFcActAddObjective() {};
	int DoCommand();
};

class CFcActRemoveObjective : public CBSConditionActionBase
{
public:
	CFcActRemoveObjective() {};
	int DoCommand();
};

class CFcActClearObjective : public CBSConditionActionBase
{
public:
	CFcActClearObjective() {};
	int DoCommand();
};




class CFcActDisplayVar : public CBSConditionActionBase
{
public:
	CFcActDisplayVar() {};
	int DoCommand();
};

class CFcActHideVar : public CBSConditionActionBase
{
public:
	CFcActHideVar() {};
	int DoCommand();
};


class CFcActDisplayVarGauge : public CBSConditionActionBase
{
public:
	CFcActDisplayVarGauge() {};
	int DoCommand();
};

class CFcActHideVarGauge : public CBSConditionActionBase
{
public:
	CFcActHideVarGauge() {};
	int DoCommand();
};

class CFcActReleaseGuardian : public CBSConditionActionBase
{
public:
	CFcActReleaseGuardian() {};
	int DoCommand();
};


class CFcActSetVarLiveTroopUnitCount : public CBSConditionActionBase
{
public:
	CFcActSetVarLiveTroopUnitCount() {};
	int DoCommand();
};


class CFcActSetVarDeadTroopUnitCount : public CBSConditionActionBase
{
public:
	CFcActSetVarDeadTroopUnitCount() {};
	int DoCommand();
};

class CFcActSetVarLiveForceUnitCount : public CBSConditionActionBase
{
public:
	CFcActSetVarLiveForceUnitCount() {};
	int DoCommand();
};

class CFcActSetVarDeadForceUnitCount : public CBSConditionActionBase
{
public:
	CFcActSetVarDeadForceUnitCount() {};
	int DoCommand();
};


class CFcActPlayAmbiSoundOn : public CBSConditionActionBase
{
public:
	CFcActPlayAmbiSoundOn() {};
	int DoCommand();
};

class CFcActPlayAmbiSoundOff : public CBSConditionActionBase
{
public:
	CFcActPlayAmbiSoundOff() {};
	int DoCommand();
};


class CFcActSetNextStage : public CBSConditionActionBase
{
public:
	CFcActSetNextStage() {};
	int DoCommand();
};

class CFcActLinkElfNDragon : public CBSConditionActionBase
{
public:
	CFcActLinkElfNDragon() {};
	int DoCommand();
};

class CFcActShowBossGauge : public CBSConditionActionBase
{
public:
	CFcActShowBossGauge() {};
	int DoCommand();
};


class CFcActHideBossGauge : public CBSConditionActionBase
{
public:
	CFcActHideBossGauge() {};
	int DoCommand();
};

class CFcActAddTroopLiveRate : public CBSConditionActionBase
{
public:
	CFcActAddTroopLiveRate() {};
	int DoCommand();
};

class CFcActSetDynamicPropGroupInArea : public CBSConditionActionBase
{
public:
	CFcActSetDynamicPropGroupInArea() {};
	int DoCommand();
};

class CFcActDropItemAtTroopPos : public CBSConditionActionBase
{
public:
	CFcActDropItemAtTroopPos() {};
	int DoCommand();
};

class CFcActDropItemAtLeaderPos : public CBSConditionActionBase
{
public:
	CFcActDropItemAtLeaderPos() {};
	int DoCommand();
};

class CFcActDropItemInArea : public CBSConditionActionBase
{
public:
	CFcActDropItemInArea() {};
	int DoCommand();
};


class CFcActSetRealtimeMovieAfterMissionEnd : public CBSConditionActionBase
{
public:
	CFcActSetRealtimeMovieAfterMissionEnd() {};
	int DoCommand();
};

class CFcActStopTrigger : public CBSConditionActionBase
{
public:
	CFcActStopTrigger() {};
	int DoCommand();
};

class CFcActTroopsEnable : public CBSConditionActionBase
{
public:
	void Initialize() { m_nID = -1; m_bFirst = true; }
	int DoCommand();

protected:
	bool m_bFirst;
	int m_nID;
};


class CFcActChangeTeam: public CBSConditionActionBase
{
public:
	CFcActChangeTeam() {};
	int DoCommand();
};

class CFcActGuardianWalk: public CBSConditionActionBase
{
public:
	CFcActGuardianWalk() {};
	int DoCommand();
};

class CFcActGuardianRun: public CBSConditionActionBase
{
public:
	CFcActGuardianRun() {};
	int DoCommand();
};

class CFcActGuardianStop: public CBSConditionActionBase
{
public:
	CFcActGuardianStop() {};
	int DoCommand();
};

class CFcActGuardianSetDir: public CBSConditionActionBase
{
public:
	CFcActGuardianSetDir() {};
	int DoCommand();
};

class CFcActGuardianRetreat: public CBSConditionActionBase
{
public:
	CFcActGuardianRetreat() {};
	int DoCommand();
};

class CFcActGuardianAISet : public CBSConditionActionBase
{
public:
	CFcActGuardianAISet() {};
	int DoCommand();
};

class CFcActTroopPickoutEnable : public CBSConditionActionBase
{
public:
	CFcActTroopPickoutEnable() {};
	int DoCommand();
};


class CFcActGuardianIncPower : public CBSConditionActionBase
{
public:
	CFcActGuardianIncPower() {};
	int DoCommand();
};
class CFcActGuardianIncRange : public CBSConditionActionBase
{
public:
	CFcActGuardianIncRange() {};
	int DoCommand();
};

class CFcActTroopIncPower : public CBSConditionActionBase
{
public:
	CFcActTroopIncPower() {};
	int DoCommand();
};	
class CFcActTroopIncRange : public CBSConditionActionBase
{
public:
	CFcActTroopIncRange() {};
	int DoCommand();
};

class CFcActEnableBlizzardEffect: public CBSConditionActionBase
{
public:
	CFcActEnableBlizzardEffect() {};
	int DoCommand();
};
class CFcActDisableBlizzardEffect : public CBSConditionActionBase
{
public:
	CFcActDisableBlizzardEffect() {};
	int DoCommand();
};

class CFcActShowMissionSelectMenu : public CBSConditionActionBase
{
public:
	CFcActShowMissionSelectMenu() {};
	int DoCommand();
};

class CFcActSetPlayerLevelExp : public CBSConditionActionBase
{
public:
	CFcActSetPlayerLevelExp() {};
	int DoCommand();
};


class CFcStartRealtimeMovie : public CBSConditionActionBase
{
public:
	CFcStartRealtimeMovie() {};
	int DoCommand();
};

class CFcActChangeLight: public CBSConditionActionBase
{
public:
	CFcActChangeLight() {};
	void Initialize(){m_bStart = false;}
	int DoCommand();

	bool m_bStart;
};

class CFcActChangeFog: public CBSConditionActionBase
{
public:
	CFcActChangeFog() {};
	void Initialize(){m_bStart = false;}
	int DoCommand();

	bool m_bStart;	
};

class CFcActChangeScene: public CBSConditionActionBase
{
public:
	CFcActChangeScene() {};
	void Initialize(){m_bStart = false;}
	int DoCommand();

	bool m_bStart;
	
};

class CFcActVarClearTime: public CBSConditionActionBase
{
public:
	CFcActVarClearTime() {};
	int DoCommand();
};
class CFcActVarKills: public CBSConditionActionBase
{
public:
	CFcActVarKills() {};
	int DoCommand();
};
class CFcActVarNormalKills: public CBSConditionActionBase
{
public:
	CFcActVarNormalKills() {};
	int DoCommand();
};
class CFcActVarOrbKills: public CBSConditionActionBase
{
public:
	CFcActVarOrbKills() {};
	int DoCommand();
};
class CFcActVarPhysicsKills: public CBSConditionActionBase
{
public:
	CFcActVarPhysicsKills() {};
	int DoCommand();
};
class CFcActVarHighestCombo: public CBSConditionActionBase
{
public:
	CFcActVarHighestCombo() {};
	int DoCommand();
};
class CFcActVarSum: public CBSConditionActionBase
{
public:
	CFcActVarSum() {};
	int DoCommand();
};
class CFcActVarSub: public CBSConditionActionBase
{
public:
	CFcActVarSub() {};
	int DoCommand();
};
class CFcActVarMul: public CBSConditionActionBase
{
public:
	CFcActVarMul() {};
	int DoCommand();
};
class CFcActVarDiv: public CBSConditionActionBase
{
public:
	CFcActVarDiv() {};	
	int DoCommand();
};
class CFcActVarMod: public CBSConditionActionBase
{
public:
	CFcActVarMod() {};
	int DoCommand();
};

class CFcActTroopsSetAttackable : public CBSConditionActionBase
{
public:
	CFcActTroopsSetAttackable() {};
	int DoCommand();
};

class CFcActTrueOrbAdd : public CBSConditionActionBase
{
public:
	CFcActTrueOrbAdd() {};
	int DoCommand();
};

class CFcTroopSetUntouchable : public CBSConditionActionBase
{
public:
	CFcTroopSetUntouchable() {};
	int DoCommand();
};

class CFcActVarClearTimeScore : public CBSConditionActionBase
{
public:
	CFcActVarClearTimeScore() {};
	int DoCommand();
};
class CFcActVarKillsScore : public CBSConditionActionBase
{
public:
	CFcActVarKillsScore() {};
	int DoCommand();
};
class CFcActVarOrbKillsScore : public CBSConditionActionBase
{
public:
	CFcActVarOrbKillsScore() {};
	int DoCommand();
};
class CFcActVarHighestComboScore : public CBSConditionActionBase
{
public:
	CFcActVarHighestComboScore() {};
	int DoCommand();
};
class CFcActVarSurviveRateScore: public CBSConditionActionBase
{
public:
	CFcActVarSurviveRateScore() {};
	int DoCommand();
};
class CFcActVarItemScore: public CBSConditionActionBase
{
public:
	CFcActVarItemScore() {};
	int DoCommand();
};

class CFcSetAdjutantType : public CBSConditionActionBase
{
public:
	CFcSetAdjutantType() {};
	int DoCommand();
};


class CFcActAddTroopEventArea : public CBSConditionActionBase
{
public:
	CFcActAddTroopEventArea() {};
	int DoCommand();
};

class CFcActSetTroopEventArea : public CBSConditionActionBase
{
public:
	CFcActSetTroopEventArea() {};
	int DoCommand();
};

class CFcForceInvulnerable : public CBSConditionActionBase
{
public:
	CFcForceInvulnerable() {};
	int DoCommand();
};

class CFcForceVulnerable : public CBSConditionActionBase
{
public:
	CFcForceVulnerable() {};
	int DoCommand();
};

class CFcActResultRank : public CBSConditionActionBase
{
public:
	CFcActResultRank() {};
	int DoCommand();
};
class CFcActRankPrize : public CBSConditionActionBase
{
public:
	CFcActRankPrize() {};
	int DoCommand();
};


class CFcActResultBonus : public CBSConditionActionBase
{
public:
	CFcActResultBonus() {};
	int DoCommand();
};

class CFcActResultRankTable : public CBSConditionActionBase
{
public:
	CFcActResultRankTable() {};
	int DoCommand();
};

class CFcActCatapultAttackWall : public CBSConditionActionBase
{
public:
	CFcActCatapultAttackWall() {};
	int DoCommand();
};

class CFcActShowWallHPGuage : public CBSConditionActionBase
{
public:
	CFcActShowWallHPGuage() {};
	int DoCommand();
};

class CFcActSetTrueOrbEnable : public CBSConditionActionBase 
{
public:
	CFcActSetTrueOrbEnable() {};
	int DoCommand();
};

class CFcActShowTroopGauge : public CBSConditionActionBase
{
public:
	CFcActShowTroopGauge() {};
	int DoCommand();
};

class CFcActHideTroopGauge : public CBSConditionActionBase
{
public:
	CFcActHideTroopGauge() {};
	int DoCommand();
};

class CFcActVarTotalUnits : public CBSConditionActionBase
{
public:
	CFcActVarTotalUnits() {};
	int DoCommand();
};


class CFcActVarOrbSparkOn : public CBSConditionActionBase
{
public:
	CFcActVarOrbSparkOn() {};
	int DoCommand();
};


class CFcActPropRegen : public CBSConditionActionBase
{
public:
	CFcActPropRegen() {};
	int DoCommand();
};

class CFcActTroopIncPowerSox : public CBSConditionActionBase
{
public:
	CFcActTroopIncPowerSox() {};
	int DoCommand();
};
class CFcActTroopIncRangeSox : public CBSConditionActionBase
{
public:
	CFcActTroopIncRangeSox() {};
	int DoCommand();
};
class CFcActTroopIncPowerSoxID : public CBSConditionActionBase
{
public:
	CFcActTroopIncPowerSoxID() {};
	int DoCommand();
};
class CFcActTroopIncRangeSoxID : public CBSConditionActionBase
{
public:
	CFcActTroopIncRangeSoxID() {};
	int DoCommand();
};

class CFcActVarTotalUnitsTeam : public CBSConditionActionBase
{
public:
	CFcActVarTotalUnitsTeam() {};
	int DoCommand();
};

class CFcActVarTroopHP: public CBSConditionActionBase
{
public:
	CFcActVarTroopHP() {};
	int DoCommand();
};

class CFcActDarkDynamicFogEnable : public CBSConditionActionBase
{
public:
	CFcActDarkDynamicFogEnable() {};
	int DoCommand();
};

class CFcActSetTroopFX : public CBSConditionActionBase
{
public:
	CFcActSetTroopFX() {};
	int DoCommand();
};

class CFcActChangeSkyBox : public CBSConditionActionBase 
{
public:
	CFcActChangeSkyBox() {};
	int DoCommand();
};

class CFcActSetRankPrizeS : public CBSConditionActionBase 
{
public:
	CFcActSetRankPrizeS() {};
	int DoCommand();
};

class CFcActSetRankPrizeA : public CBSConditionActionBase 
{
public:
	CFcActSetRankPrizeA() {};
	int DoCommand();
};

class CFcActGuardianPlace : public CBSConditionActionBase 
{
public:
	CFcActGuardianPlace() {};
	int DoCommand();
};

class CFcActSetItemIndex : public CBSConditionActionBase
{
public:
	CFcActSetItemIndex() {};
	int DoCommand();
};

class CFcActVarGuardianKillCount : public CBSConditionActionBase
{
public:
	CFcActVarGuardianKillCount() {};
	int DoCommand();
};
class CFcActVarGuardianOrb : public CBSConditionActionBase
{
public:
	CFcActVarGuardianOrb() {};
	int DoCommand();
};


class CFcActHideWallHPGuage : public CBSConditionActionBase
{
public:
	CFcActHideWallHPGuage		() {};
	int DoCommand();
};

class CFcActDarkDynamicFogEnableEx : public CBSConditionActionBase
{
public:
	CFcActDarkDynamicFogEnableEx		() {};
	int DoCommand();
};

class CFcActDarkDynamicFogDisableEx : public CBSConditionActionBase
{
public:
	CFcActDarkDynamicFogDisableEx		() {};
	int DoCommand();
};

class CFcActSetItemBoxEnable : public CBSConditionActionBase
{
public:
	CFcActSetItemBoxEnable		() {};
	int DoCommand();
};

class CFcActSetItemBoxDisable : public CBSConditionActionBase
{
public:
	CFcActSetItemBoxDisable 	() {};
	int DoCommand();
};

class CFcActEffectRainEnable : public CBSConditionActionBase
{
public:
	CFcActEffectRainEnable 	() {};
	int DoCommand();
};
class CFcActEffectRainDisable : public CBSConditionActionBase
{
public:
	CFcActEffectRainDisable 	() {};
	int DoCommand();
};

class CFcActSkipText : public CBSConditionActionBase
{
public:
	CFcActSkipText () {};
	int DoCommand();
};

class CFcActSkipRealMovie : public CBSConditionActionBase
{
public:
	CFcActSkipRealMovie() {};
	int DoCommand();
};

class CFcActDelayOnRealMovie : public CBSConditionActionBase
{
public:
	CFcActDelayOnRealMovie() {};
	int DoCommand();
};

class CFcActSetWeaponFromTroop : public CBSConditionActionBase
{
public:
	CFcActSetWeaponFromTroop() {};
	int DoCommand();
};

class CFcSetTroopAIDisableInAreaForRM : public CBSConditionActionBase
{
public:
	CFcSetTroopAIDisableInAreaForRM() {};
	int DoCommand();
};

class CFcActVarScoreTroopHP : public CBSConditionActionBase
{
public:
	CFcActVarScoreTroopHP() {};
	int DoCommand();
};
class CFcActVarScoreTroopTeamHP: public CBSConditionActionBase
{
public:
	CFcActVarScoreTroopTeamHP() {};
	int DoCommand();
};
class CFcActVarScoreTroopNum: public CBSConditionActionBase
{
public:
	CFcActVarScoreTroopNum() {};
	int DoCommand();
};
class CFcActVarScoreRange: public CBSConditionActionBase
{
public:
	CFcActVarScoreRange() {};
	int DoCommand();
};
class CFcActVarScoreBrokenProp: public CBSConditionActionBase
{
public:
	CFcActVarScoreBrokenProp() {};
	int DoCommand();
};
class CFcActSetPropCrush: public CBSConditionActionBase
{
public:
	CFcActSetPropCrush() {};
	int DoCommand();
};
class CFcActSetCorpseDelay: public CBSConditionActionBase
{
public:
	CFcActSetCorpseDelay() {};
	int DoCommand();
};
class CFcActPlaySoundEffect: public CBSConditionActionBase
{
public:
	CFcActPlaySoundEffect() {};
	int DoCommand();
};
class CFcActAddMaxHP: public CBSConditionActionBase
{
public:
	CFcActAddMaxHP() {};
	int DoCommand();
};

class CFcActShowTroopHPGaugeExceptLeader: public CBSConditionActionBase
{
public:
	CFcActShowTroopHPGaugeExceptLeader() {};
	int DoCommand();
};

class CFcActSetDefenseRef: public CBSConditionActionBase
{
public:
	CFcActSetDefenseRef() {};
	int DoCommand();
};
class CFcActShowCircleAreaInMinimap: public CBSConditionActionBase
{
public:
	CFcActShowCircleAreaInMinimap() {};
	int DoCommand();
};
class CFcActHideCircleAreaInMinimap: public CBSConditionActionBase
{
public:
	CFcActHideCircleAreaInMinimap() {};
	int DoCommand();
};

class CFcTroopAnimationWithDelay: public CBSConditionActionBase
{
public:
	CFcTroopAnimationWithDelay()
	{
		m_bStart = false;
		m_pCheckTick = NULL;
	};

	~CFcTroopAnimationWithDelay()
	{
		SAFE_DELETEA( m_pCheckTick );
	}

	int DoCommand();

protected:
	bool m_bStart;
	int*  m_pCheckTick;
};


class CFcActTroopDamagedOnlyPlayer: public CBSConditionActionBase
{
public:
	CFcActTroopDamagedOnlyPlayer() {};
	int DoCommand();
};

class CFcActItemProbbtAdd : public CBSConditionActionBase
{
public:
	CFcActItemProbbtAdd(){};
	int DoCommand();
};

class CFcActVarSurvivalRateScoreVarArg : public CBSConditionActionBase
{
public:
	CFcActVarSurvivalRateScoreVarArg(){};
	int DoCommand();
};
class CFcActVarKillsScoreVarArg: public CBSConditionActionBase
{
public:
	CFcActVarKillsScoreVarArg(){};
	int DoCommand();
};

class CFcActEnableEventTroopAI: public CBSConditionActionBase
{
public:
	CFcActEnableEventTroopAI(){};
	int DoCommand();
};

class CFcActDisableEventTroopAI: public CBSConditionActionBase
{
public:
	CFcActDisableEventTroopAI(){};
	int DoCommand();
};

class CFcActPointTroopStrOn: public CBSConditionActionBase
{
public:
	CFcActPointTroopStrOn(){};
	int DoCommand();
};

class CFcActPointTroopStrOff: public CBSConditionActionBase
{
public:
	CFcActPointTroopStrOff(){};
	int DoCommand();
};

class CFcActSetUnitAIEnable: public CBSConditionActionBase
{
public:
	CFcActSetUnitAIEnable(){};
	int DoCommand();
};

class CFcActSetUnitAIDisable: public CBSConditionActionBase
{
public:
	CFcActSetUnitAIDisable(){};
	int DoCommand();
};

class CFcActTroopLastDisable: public CBSConditionActionBase
{
public:
	CFcActTroopLastDisable(){};
	int DoCommand();
};

class CFcActTroopsLastDisable: public CBSConditionActionBase
{
public:
	CFcActTroopsLastDisable(){};
	int DoCommand();
};

class CFcActVarMissionLevel: public CBSConditionActionBase
{
public:
	CFcActVarMissionLevel(){};
	int DoCommand();
};
class CFcActVigMotionBlurEnable:public CBSConditionActionBase
{
public:
	CFcActVigMotionBlurEnable(){};
	int DoCommand();
};
class CFcActVigMotionBlurDisable:public CBSConditionActionBase
{
public:
	CFcActVigMotionBlurDisable(){};
	int DoCommand();
};

class CFcActSeSoundPlayingOff : public CBSConditionActionBase
{
public:
	CFcActSeSoundPlayingOff(){};
	int DoCommand();
};

class CFcActSeSoundPlayingOn : public CBSConditionActionBase
{
public:
	CFcActSeSoundPlayingOn(){};
	int DoCommand();
};

#endif