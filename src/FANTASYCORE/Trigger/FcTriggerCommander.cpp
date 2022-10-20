#include "stdafx.h"
#include "FcTriggerCommander.h"
#include "BSTriggerManager.h"
#include "BSTriggerCondition.h"
#include "BSTriggerAction.h"
#include "FcWorld.h"
#include "BSTriggerCodeDefine.h"
#include "FcTriggerCondition.h"
#include "FcTriggerAction.h"
#include "FcGlobal.h"
#include "BSTrigger.h"
#include "BSTriggerVariable.h"
#include "DebugUtil.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

CFcTriggerEventBase::CFcTriggerEventBase()
{
	Reset();
	//m_ActionState  = STATE_NOT_WORKING;
}

CFcTriggerEventBase::~CFcTriggerEventBase()
{
	SAFE_DELETE_PVEC( m_ConditionList );
	SAFE_DELETE_PVEC( m_ActionList );
}


CBSConditionActionBase *CFcTriggerEventBase::AllocateConditionByType(int iCodeIndex)
{
	switch(iCodeIndex)
	{
	case CON_INPUTSTICK_A:						return new CFcConInputStickA;
	case CON_INPUTSTICK_LEFT:					return new CFcConInputStickLeft;
	case CON_INPUTSTICK_RIGHT:					return new CFcConInputStickRight;
	case CON_INPUTSTICK_NEUTRAL:				return new CFcInputStickNeutral;
	case CON_COMPARE_ORB:						return new CFcConCompareOrb;
	case CON_TIME_ELAPSED:						return new CFcConTimeElapsed;
	case CON_TIME_ELAPSED_FROM_MARK:			return new CFcConTimeElapsedFromMark;
	case CON_TROOP_ATTACKED:					return new CFcConAttacked;
	case CON_TROOP_ATTACKED_TROOP:				return new CFcConAttackedTroop;
	case CON_COMPARE_TROOP_DIST:				return new CFcConCompareTroopDist;
	case CON_COMPARE_TROOP_HP:					return new CFcConCompareTroopHP;
	case CON_TROOP_IN_AREA:						return new CFcConTroopInArea;
	case CON_COMPARE_LEADER_HP:					return new CFcConCompareLeaderHP;
	case CON_TROOP_MELEE_ATTACKED:				return new CFcConTroopMeleeAttacked;
	case CON_TROOP_RANGE_ATTACKED:				return new CFcConTroopRangeAttacked;
	case CON_TROOP_UNBLOCKABLE_ATTACKED:		return new CFcConUnBlockableAttacked;
	case CON_TROOP_NO_ENGAGED:					return new CFcConTroopNoEngaged;
	case CON_TROOP_NOT_IN_AREA:					return new CFcConTroopNotInArea;
	case CON_VAR:								return new CFcConVar;
	case CON_PROP_WAS_DESTROYED:				return new CFcConPropWasDestroyed;
	case CON_TROOP_ENGAGED:						return new CFcConTroopEngaged;
	case CON_FORCE_COMPARE_HP:					return new CFcConForceCompareHP;
	case CON_FORCE_ELIMINATED:					return new CFcConForceEliminated;
	case CON_FORCE_IN_AREA:						return new CFcConForceInArea;
	case CON_FORCE_NOT_IN_AREA:					return new CFcConForceNotInArea;
	case CON_FORCE_ALL_IN_AREA:					return new CFcConForceAllInArea;
	case CON_PLAYER_GUARDIAN_NUM:				return new CFcConPlayerGuardianNum;
	case CON_TROOP_TYPE:						return new CFcConTroopType;
	case CON_GUARDIAN_TROOP_TYPE:				return new CFcConGuardianTroopType;
	case CON_TROOP_LEVEL:						return new CFcConTroopLevel;
	case CON_ORB_GAUGE_FULL:					return new CFcConOrbGaugeFull;
	case CON_TRUE_ORB_GAUGE_FULL:				return new CFcConTrueOrbGaugeFull;
	case CON_TOTAL_KILL_NUM:					return new CFcConTotalKillNum;
	case CON_FORCE_KILL_NUM:					return new CFcConForceKillNum;
	case CON_GUARDIAN_IN_AREA:					return new CFcConGuardianInArea;
	case CON_GUARDIAN_NOT_IN_AREA:				return new CFcConGuardianNotInArea;
	case CON_GUARDIAN_COMPARE_DIST:				return new CFcConGuardianCompareDist;
	case CON_GUARDIAN_EXIST:					return new CFcConGuardianExist;
	case CON_GUARDIAN_TROOP_TYPE2:				return new CFcConGuardianTroopType2;
	case CON_SELECT_MISION_MENU_VALUE:			return new CFcConSelectMissionMenuValue;

	case CON_FORCE_COMPARE_UNIT_NUMBER:			return new CFcConForceCompareUnitNumber;
	case CON_TROOP_COMPARE_UNIT_NUMBER:			return new CFcConTroopCompareUnitNumber;
	case CON_REALMOVIE_FINISH:					return new CFcConRealmovieFinish;

	case CON_COMPARE_TROOP_N_FORCE_DIST:		return new CFcConCompareTroopNForceDist;
	case CON_COMPARE_FORCE_N_FORCE_DIST:		return new CFcConCompareForceNForceDist;

	case CON_VAR_COMPARE:						return new CFcConVarCompare;

	case CON_TROOP_ATTACKED_FORCE:				return new CFcConTroopAttackedForce;
	case CON_TROOP_ATTACKED_STR:				return new CFcConTroopAttackedStr;
	case CON_ITEMBOX_BROKEN:					return new CFcConItemBoxBroken;

	case CON_PROP_HP:							return new CFcConPropHP;
	case CON_IS_DEMO_SKIP:						return new CFcConIsDemoSkip;
	case CON_TROOP_COMPARE_HP_EXCEPT_LEADER:	return new CFcConTroopCompareHPExceptLeader;
	}
	BsAssert(0 && "Not defined condition type");
	return NULL;
}

CBSConditionActionBase *CFcTriggerEventBase::AllocateActionByType(int iCodeIndex)
{
	switch(iCodeIndex)
	{
	case ACT_AI_DISABLE:					return new CFcActAIDisable;
	case ACT_AI_ENABLE:						return new CFcActAIEnable;
	case ACT_AISET:							return new CFcActAISet;
	case ACT_AISETPATH:						return new CFcActAISetPath;
	case ACT_BGM_FADE:						return new CFcActBGMFade;
	case ACT_BGM_MUTE:						return new CFcActBGMMute;
	case ACT_BGM_PLAY:						return new CFcActBGMPlay;
	case ACT_BGM_STOP:						return new CFcActBGMStop;
	case ACT_BGM_VOLUME:					return new CFcActBGMVolume;
	case ACT_CAM_RESET:						return new CFcActCamReset;
	case ACT_CAM_SET:						return new CFcActCamSet;
	case ACT_CAM_TARGETAREA:				return new CFcActCamTargetArea;
	case ACT_CAM_TARGETTROOP:				return new CFcActCamTargetTroop;
	case ACT_DELAY:							return new CFcActDelay;
	case ACT_DEMO_PLAY:						return new CFcActDemoDelay;
	case ACT_EFFECT_FADEIN:					return new CFcActEffectFadeIn;
	case ACT_EFFECT_FADEINCOLOR:			return new CFcActEffectFadeInColor;
	case ACT_EFFECT_FADEOUT:				return new CFcActEffectFadeOut;
	case ACT_EFFECT_FADEOUT_COLOR:			return new CFcActEffectFadeOutColor;
	case ACT_EFFECT_FOG:					return new CFcActEffectFog;
	case ACT_EFFECTFX_RANDOM_SET:			return new CFcActEffectFXRandomSet;
	case ACT_EFFECTFX_SET:					return new CFcActEffectFXSet;
	case ACT_EFFECT_GLOW_OFF:				return new CFcActEffectGlowOff;
	case ACT_EFFECT_GLOW_ON:				return new CFcActEffectGlowOn;
	case ACT_EFFECT_LENSFLARE_OFF:			return new CFcActEffectLensflareOff;
	case ACT_EFFECT_LENSFLARE_ON:			return new CFcActEffectLensflareOn;
	case ACT_EFFECT_LIGHT:					return new CFcActEffectLight;
	case ACT_EFFECT_MOTIONBLUR_OFF:			return new CFcActEffectMotionblurOff;
	case ACT_EFFECT_MOTIONBLUR_ON:			return new CFcActEffectMotionblurOn;
	case ACT_EFFECT_RAIN_OFF:				return new CFcActEffectRainOff;
	case ACT_EFFECT_RAIN_ON:				return new CFcActEffectRainOn;
	case ACT_EFFECT_SET_FIRE:				return new CFcActEffectSetFire;
	case ACT_EFFECT_SNOW_OFF:				return new CFcActEffectSnowOff;
	case ACT_EFFECT_SNOW_ON:				return new CFcActEffectSnowOn;
	case ACT_GATE_CLOSE:					return new CFcActGateClose;
	case ACT_GATE_OPEN:						return new CFcActGateOpen;
	case ACT_HOOK_INPUT_OFF:				return new CFcActHookInputOff;
	case ACT_HOOK_INPUT_ON:					return new CFcActHookInputOn;
	case ACT_INTERFACE_OFF:					return new CFcActInterfaceOff;
	case ACT_INTERFACE_ON:					return new CFcActInterfaceOn;
	case ACT_LETTER_BOX_OFF:				return new CFcActLetterBoxOff;
	case ACT_LETTER_BOX_ON:					return new CFcActLetterBoxOn;
	case ACT_MAP_OFF:						return new CFcActMapOff;
	case ACT_MAP_ON:						return new CFcActMapOn;
	case ACT_MAP_SCALE:						return new CFcActMapScale;
	case ACT_MESSAGE_REPORT:				return new CFcActMessageReport;
	case ACT_MESSAGE_SAY:					return new CFcActMessageSay;
	case ACT_MESSAGE_SHOW:					return new CFcActMessageShow;
	case ACT_MESSAGE_SHOWXY:				return new CFcActMessageShowXY;
	case ACT_MISSION_COMPLETED:				return new CFcActMissionCompleted;
	case ACT_MISSION_FAILED:				return new CFcActMissionFailed;
	case ACT_ORB_ADD:						return new CFcActOrbAdd;
	case ACT_POINT_AREA_OFF:				return new CFcActPointAreaOff;
	case ACT_POINT_AREA_ON:					return new CFcActPointAreaOn;
	case ACT_POINT_FRIEND_IN_AREA:			return new CFcActPointFriendInArea;
	case ACT_POINT_HOSTILE_IN_AREA:			return new CFcActPointHostileInArea;
	case ACT_POINT_TROOP_OFF:				return new CFcActPointTroopOFf;
	case ACT_POINT_TROOP_ON:				return new CFcActPointTroopOn;
	case ACT_SOUND_SET:						return new CFcActSoundSet;
	case ACT_SOUND_UNSET:					return new CFcActSoundUnset;
	case ACT_TIME_MARK:						return new CFcActTimeMark;
	case ACT_TRIGGER_ACTIVATE:				return new CFcActTriggerActivate;
	case ACT_TRIGGER_DEACTIVATE:			return new CFcActTriggerDeactivate;
	case ACT_TRIGGER_LOOP:					return new CFcActTriggerLoop;
	case ACT_TRIGGER_RESET:					return new CFcActTriggerReset;
	case ACT_TRIGGER_RESETALL:				return new CFcActTriggerResetAll;
	case ACT_TRIGGER_RESETTHIS:				return new CFcActTriggerResetThis;
	case ACT_TROOP_ANIMATION:				return new CFcActTroopAnimation;
	case ACT_TROOP_ANNIHILATE:				return new CFcActTroopAnnihilate;
	case ACT_TROOP_ATTACK:					return new CFcActTroopAttack;
	case ACT_TROOP_ATTACK_LEADER:			return new CFcActTroopAttackLeader;
	case ACT_TROOP_DISABLE:					return new CFcActTroopDisable;
	case ACT_TROOP_DISABLE_ALL:				return new CFcActTroopDisableAll;
	case ACT_TROOP_DISABLE_INAREA:			return new CFcActTroopDisableInArea;
	case ACT_TROOP_ENABLE:					return new CFcActTroopEnable;
	case ACT_TROOP_ENABLE_IN_AREA:			return new CFcActEnableInArea;
	case ACT_TROOP_ENEMY_DISABLE_IN_AREA:	return new CFcActEnemyDisableInArea;
	case ACT_TROOP_ENEMY_ENABLEINAREA:		return new CFcActEnemyEnableInArea;
	case ACT_TROOP_FOLLOW:					return new CFcActTroopFllow;
	case ACT_TROOP_FRIEND_DISABLE_IN_AREA:	return new CFcActTroopFriendDisableInArea;
	case ACT_TROOP_FRIEND_ENABLE_IN_AREA:	return new CFcActTroopFriendEnableInArea;
	case ACT_TROOP_HP_FILL:					return new CFcActTroopHPFill;
	case ACT_TROOP_HP_MAX:					return new CFcActTroopHPMAX;
	case ACT_TROOP_INVULNERABLE:			return new CFcActTroopInvulnerable;
	case ACT_TROOP_LEADER_INVULNERABLE:		return new CFcActTroopLeaderInvulnerable;
	case ACT_TROOP_LEADER_VULNERABLE:		return new CFcActTroopLeaderVulnerable;
	case ACT_TROOP_PLACE:					return new CFcActTroopPlace;
	case ACT_TROOP_RANGE_ATTACK_AREA:		return new CFcActTroopRangeAttackArea;
	case ACT_TROOP_RANGE_ATTACK_WALL:		return new CFcActTroopRangeAttackWall;
	case ACT_TROOP_RE_NEW:					return new CFcActTroopRenew;
	case ACT_TROOP_RE_NEW_OUTOFSIGHT:		return new CFcActTroopRenewOutOfSight;
	case ACT_TROOP_RUN:						return new CFcActTroopRun;
	case ACT_TROOP_STOP:					return new CFcActTroopStop;
	case ACT_TROOP_VULNERABLE:				return new CFcActTroopVulnerable;
	case ACT_TROOP_WALK:					return new CFcActTroopWalk;
	case ACT_VAR_INCREASE:					return new CFcActVarIncrease;
	case ACT_VAR_SET:						return new CFcActVarSet;
	case ACT_WALL_COLLAPSE:					return new CFcActWallCollapse;
	case ACT_WALL_SET_HP:					return new CFcActWallSetHP;
	case ACT_PROP_PUSH:						return new CFcPropPush;
	case ACT_DESTROY_BRIDGE:				return new CFcDestroyBridge;
	case ACT_SET_GUARDIAN_TROOP:			return new CFcSetGuardianTroop;
	case ACT_SET_ENEMY_ATTR:				return new CFcActSetEnemyAttr;
	case ACT_RESET_ENEMY_ATTR:				return new CFcActResetEnemyAttr;
	case ACT_ENABLE_DYNAMIC_PROP:			return new CFcActEnableDynamicProp;
	case ACT_DISABLE_DYNAMIC_PROP:			return new CFcActDisableDynamicProp;
	case ACT_PLAY_REALTIME_MOVIE:			return new CFcPlayRealtimeMovie;
	case ACT_TROOP_HP_GAUGE_SHOW:			return new CFcTroopHPGaugeShow;

	case ACT_TROOPTYPE_AI_DISABLE_IN_AREA:	return new CTroopTypeAIDisableInArea;
	case ACT_TROOPTYPE_AI_ENABLE_IN_AREA:	return new CTroopTypeAIEnableInArea;
	case ACT_TROOPTYPE_AISET_IN_AREA:		return new CTroopTypeAISetInArea;
	case ACT_TROOPTYPE_ANIMATION_IN_AREA:	return new CTroopTypeAnimationInArea;
	case ACT_TROOPTYPE_KILL_ALL_IN_AREA:	return new CTroopTypeKillAllInArea;
	case ACT_TROOPTYPE_DISABLE_IN_AREA:		return new CTroopTypeDIsableInArea;
	case ACT_TROOPTYPE_HP_FILL_IN_AREA:		return new CTroopTypeHPFillInArea;
	case ACT_TROOPTYPE_HP_MAX_IN_AREA:		return new CTroopTypeHPMaxInArea;
	case ACT_TROOPTYPE_WALK:				return new CTroopTypeWalk;
	case ACT_TROOPTYPE_RUN:					return new CTroopTypeRun;
	case ACT_TROOPTYPE_STOP_IN_AREA:		return new CTroopTypeStopInArea;
	case ACT_TROOP_RANGE_ATTACK_TO_AREA:	return new CTroopRangeAttackToArea;
	case ACT_TROOPTYPE_SET_DIRECTION:		return new CTroopTypeSetDirection;
	case ACT_TROOPTYPE_SHOW_IN_MINIMAP:		return new CTroopTypeShowInMinimap;
	case ACT_TROOPTYPE_HIDE_IN_MINIMAP:		return new CTroopTypeHideInMinimap;
	case ACT_TROOP_SHOW_IN_MINIMAP:			return new CTroopShowInMinimap;
	case ACT_TROOP_HIDE_IN_MINIMAP:			return new CTroopHideInMinimap;
	case ACT_UNIT_SHOW_IN_MINIMAP_ON_PROP:	return new CUnitOnPropShowInMinimap;
	case ACT_UNIT_HIDE_IN_MINIMAP_ON_PROP:	return new CUnitOnPropHideInMinimap;


	case ACT_PLAYER_ADD_ORB:				return new CFcActPlayerAddOrb;
	case ACT_FORCE_SHOW_HP_GAUGE:			return new CFcActTeamShowHPGauge;
	case ACT_FORCE_SHOW_HP_GAUGE_IN_AREA:	return new CFcActTeamShowHPGaugeInArea;

	case ACT_TROOPS_AI_ENABLE:				return new CFcActTroopsAIEnable;
	case ACT_TROOPS_AI_DISABLE:				return new CFcActTroopsAIDisable;
	case ACT_TROOPS_SET_AI:					return new CFcActTroopsSetAI;
	case ACT_TROOPS_PLAY_ACTION:			return new CFcActTroopsPlayAction;
	case ACT_TROOPS_ANNIHILATE:				return new CFcActTroopsAnnihilate;
	case ACT_TROOPS_DISABLE:				return new CFcActTroopsDisable;	
	case ACT_TROOPS_HP_FILL:				return new CFcActTroopsHPFill;
	case ACT_TROOPS_HP_MAX:					return new CFcActTroopsHPMax;
	case ACT_TROOPS_INVULNERABLE:			return new CFcActTroopsInvulnerable;
	case ACT_TROOPS_VULNERABLE:				return new CFcActTroopsVulnerable;
	case ACT_TROOPS_WALK:					return new CFcActTroopsWalk;
	case ACT_TROOPS_RUN:					return new CFcActTroopsRun;
	case ACT_TROOPS_STOP:					return new CFcActTroopsStop;
	case ACT_TROOPS_RANGE_ATTACK_AREA:		return new CFcActTroopsRangeAttackArea;
	case ACT_TROOPS_SET_DIR:				return new CFcActTroopsSetDir;
	case ACT_TROOPS_SHOW_MINIMAP:			return new CFcActTroopsShowMinimap;
	case ACT_TROOPS_SHOW_HP_GAUGE:			return new CFcActTroopsShowHPGauge;
	case ACT_TROOP_CHANGE_GROUP:			return new CFcActTroopsChangeGroup;
	case ACT_DYNAMIC_FOG_ENABLE:			return new CFcActDynamicFogEnable;
	case ACT_DYNAMIC_FOG_CROSS:				return new CFcActDynamicFogCross;
	case ACT_SET_DOF:						return new CFcActSetDOF;
	case ACT_DISABLE_DOF:					return new CFcActDisableDOF;
	case ACT_DESTROY_CRUMBLE_STONE:			return new CFcActDestroyCrumbleStone;
	case ACT_DISABLE_DYNAMIC_FOG:			return new CFcActDynamicFogDisable;
	case ACT_SET_PROP_AI_GUIDE:				return new CFcActSetPropAIGuide;
	case ACT_SET_AREA_AI_GUIDE:				return new CFcActSetAreaAIGuide;
	case ACT_SET_GUARDIAN_AI:				return new CFcActSetGuardianAI;
	case ACT_MOVE_MOVETOWER:				return new CFcActMoveMoveTower;
	case ACT_ADD_OBJECTIVE:					return new CFcActAddObjective;
	case ACT_REMOVE_OBJECTIVE:				return new CFcActRemoveObjective;
	case ACT_CLEAR_OBJECTIVE:				return new CFcActClearObjective;
	case ACT_DISPLAY_VAR:					return new CFcActDisplayVar;
	case ACT_HIDE_VAR:						return new CFcActHideVar;
	case ACT_DISPLAY_VAR_GAUGE:				return new CFcActDisplayVarGauge;
	case ACT_HIDE_VAR_GAUGE:				return new CFcActHideVarGauge;

	case ACT_RELEASE_GUARDIAN:				return new CFcActReleaseGuardian;
	case ACT_SET_VAR_LIVE_TROOP_UNIT_COUNT:	return new CFcActSetVarLiveTroopUnitCount;
	case ACT_SET_VAR_DEAD_TROOP_UNIT_COUNT:	return new CFcActSetVarDeadTroopUnitCount;
	case ACT_SET_VAR_LIVE_FORCE_UNIT_COUNT:	return new CFcActSetVarLiveForceUnitCount;
	case ACT_SET_VAR_DEAD_FORCE_UNIT_COUNT:	return new CFcActSetVarDeadForceUnitCount;
	case ACT_PLAY_AMBI_SOUND_ON:			return new CFcActPlayAmbiSoundOn;
	case ACT_PLAY_AMBI_SOUND_OFF:			return new CFcActPlayAmbiSoundOff;

	case ACT_SET_NEXT_STAGE:				return new CFcActSetNextStage;
	case ACT_LINK_ELF_N_DRAGON:				return new CFcActLinkElfNDragon;

	case ACT_SHOW_BOSS_GAUGE:				return new CFcActShowBossGauge;
	case ACT_HIDE_BOSS_GAUGE:				return new CFcActHideBossGauge;
	case ACT_ADD_TROOP_LIVE_RATE:			return new CFcActAddTroopLiveRate;
	
	case ACT_SET_DYNAMIC_PROP_GROUP_IN_AREA:	return new CFcActSetDynamicPropGroupInArea;
	case ACT_DROP_ITEM_AT_TROOP_POS:			return new CFcActDropItemAtTroopPos;
	case ACT_DROP_ITEM_AT_LEADER_POS:			return new CFcActDropItemAtLeaderPos;
	case ACT_DROP_ITEM_IN_AREA:					return new CFcActDropItemInArea;

	case ACT_SET_REALTIME_MOVIE_AFTER_MISSION_END:	return new CFcActSetRealtimeMovieAfterMissionEnd;
	case ACT_SET_STOP_TRIGGER:					return new CFcActStopTrigger;

	case ACT_TROOPS_ENABLE:					return new CFcActTroopsEnable;
	case ACT_CHANGE_TEAM:					return new CFcActChangeTeam;

	case ACT_GUARDIAN_WALK:					return new CFcActGuardianWalk;
	case ACT_GUARDIAN_RUN:					return new CFcActGuardianRun;
	case ACT_GUARDIAN_STOP:					return new CFcActGuardianStop;
	case ACT_GUARDIAN_SET_DIR:				return new CFcActGuardianSetDir;
	case ACT_GUARDIAN_RETREAT:				return new CFcActGuardianRetreat;
	case ACT_GUARDIAN_AI_SET:				return new CFcActGuardianAISet;

	case ACT_TROOP_PICKOUT_ENABLE:			return new CFcActTroopPickoutEnable;

	case ACT_GUARDIAN_INC_POWER:			return new CFcActGuardianIncPower;
	case ACT_GUARDIAN_INC_RANGE:			return new CFcActGuardianIncRange; 		

	case ACT_TROOP_INC_POWER:				return new CFcActTroopIncPower;	
	case ACT_TROOP_INC_RANGE:				return new CFcActTroopIncRange;

	case ACT_ENABLE_BLIZZARD_EFFECT:		return new CFcActEnableBlizzardEffect;
	case ACT_DISABLE_BLIZZARD_EFFECT:		return new CFcActDisableBlizzardEffect;
	case ACT_SHOW_MISSION_SELECT_MENU:		return new CFcActShowMissionSelectMenu;
	case ACT_SET_PLAYER_LEVEL_EXP:			return new CFcActSetPlayerLevelExp;

	case ACT_START_REALTIME_MOVIE:			return new CFcStartRealtimeMovie;

	case ACT_CHANGE_LIGHT:						return new CFcActChangeLight;
	case ACT_CHANGE_FOG:						return new CFcActChangeFog;
	case ACT_CHANGE_SCENE:						return new CFcActChangeScene;

	case ACT_VAR_CLEARTIME:					return new CFcActVarClearTime;
	case ACT_VAR_KILLS:						return new CFcActVarKills;
	case ACT_VAR_NORMALKILLS:				return new CFcActVarNormalKills;
	case ACT_VAR_ORBKILLS:					return new CFcActVarOrbKills;
	case ACT_VAR_PHYSICSKILLS:				return new CFcActVarPhysicsKills;
	case ACT_VAR_HIGHESTCOMBO:				return new CFcActVarHighestCombo;
	case ACT_VAR_SUM:						return new CFcActVarSum;
	case ACT_VAR_SUB:						return new CFcActVarSub;
	case ACT_VAR_MUL:						return new CFcActVarMul;
	case ACT_VAR_DIV:						return new CFcActVarDiv;
	case ACT_VAR_MOD:						return new CFcActVarMod;

	case ACT_TROOPS_SET_ATTACKABLE:			return new CFcActTroopsSetAttackable;
	case ACT_TRUE_ORB_ADD:					return new CFcActTrueOrbAdd;
	case ACT_TROOP_SET_UNTOUCHABLE:			return new CFcTroopSetUntouchable;

	case ACT_VAT_CLEARTIME_SCORE:			return new CFcActVarClearTimeScore;
	case ACT_VAR_KILLS_SCORE:				return new CFcActVarKillsScore;
	case ACT_VAR_ORBKILLS_SCORE:			return new CFcActVarOrbKillsScore;
	case ACT_VAR_HIGHESTCOMBO_SCORE:		return new CFcActVarHighestComboScore;
	case ACT_VAR_SURVIVALRATE_SCORE:		return new CFcActVarSurviveRateScore;
	case ACT_VAR_ITEM_SCORE:				return new CFcActVarItemScore;

	case ACT_SET_ADJUTANT_TYPE:				return new CFcSetAdjutantType;

	case ACT_ADD_TROOP_EVENT_AREA:			return new CFcActAddTroopEventArea;
	case ACT_SET_TROOP_EVENT_AREA:			return new CFcActSetTroopEventArea;
	case ACT_FORCE_INVULNERABLE:			return new CFcForceInvulnerable;
	case ACT_FORCE_VULNERABLE:				return new CFcForceVulnerable;

	case ACT_SET_RESULTRANK:				return new CFcActResultRank;
	case ACT_SET_RANKPRIZE:					return new CFcActRankPrize;
	case ACT_SET_RESULTBONUS:				return new CFcActResultBonus;
	case ACT_SET_RESULTRANKTABLE:			return new CFcActResultRankTable;
	case ACT_CATAPULT_ATTACK_WALL:			return new CFcActCatapultAttackWall;
	case ACT_SHOW_WALL_HP_GUAGE:			return new CFcActShowWallHPGuage;
	case ACT_SET_TRUEORBENABLE:				return new CFcActSetTrueOrbEnable;

	case ACT_SHOW_TROOP_GAUGE:				return new CFcActShowTroopGauge;
	case ACT_HIDE_TROOP_GAUGE:				return new CFcActHideTroopGauge;

	case ACT_VAR_TOTALUNITS:				return new CFcActVarTotalUnits;
	case ACT_VAR_ORBSPARKON:				return new CFcActVarOrbSparkOn;
	case ACT_PROP_REGEN:					return new CFcActPropRegen;

	case ACT_TROOP_INC_POWER_SOX:			return new CFcActTroopIncPowerSox;
	case ACT_TROOP_INC_RANGE_SOX:			return new CFcActTroopIncRangeSox;
	case ACT_TROOP_INC_POWER_SOX_ID:		return new CFcActTroopIncPowerSoxID;
	case ACT_TROOP_INC_RANGE_SOX_ID:		return new CFcActTroopIncRangeSoxID;

	case ACT_VAR_TOTALUNITS_TEAM:			return new CFcActVarTotalUnitsTeam;

	case ACT_VAR_TROOPHP:					return new CFcActVarTroopHP;

	case ACT_DARK_DYNAMIC_FOG_ENABLE:		return new CFcActDarkDynamicFogEnable;
	case ACt_SET_TROOP_FX:					return new CFcActSetTroopFX;
	case ACT_CHANGE_SKYBOX:					return new CFcActChangeSkyBox;
	case ACT_SET_RANKPRIZE_S:				return new CFcActSetRankPrizeS;
	case ACT_SET_RANKPRIZE_A:				return new CFcActSetRankPrizeA;
	case ACT_GUARDIAN_PLACE:				return new CFcActGuardianPlace;
	case ACT_SET_ITEMINDEX:					return new CFcActSetItemIndex;
	case ACT_VAR_GURADIAN_KILLCOUNT:		return new CFcActVarGuardianKillCount;
	case ACT_VAR_GURADIAN_ORB:				return new CFcActVarGuardianOrb;
	case ACT_HIDE_WALL_HP_GUAGE:			return new CFcActHideWallHPGuage;
	case ACT_DARK_DYNAMIC_FOG_ENABLE_EX:	return new CFcActDarkDynamicFogEnableEx;
	case ACT_DARK_DYNAMIC_FOG_DISABLE_EX:	return new CFcActDarkDynamicFogDisableEx;
	case ACT_SET_ITEMBOX_ENABLE:			return new CFcActSetItemBoxEnable;
	case ACT_SET_ITEMBOX_DISABLE:			return new CFcActSetItemBoxDisable;
	case ACT_EFFECT_RAIN_ENABLE:			return new CFcActEffectRainEnable;
	case ACT_EFFECT_RAIN_DISABLE:			return new CFcActEffectRainDisable;
	case ACT_SKIP_TEXT:						return new CFcActSkipText;
	case ACT_SKIP_REALMOVIE:				return new CFcActSkipRealMovie;
	case ACT_DELAY_ON_REALMOVIE:			return new CFcActDelayOnRealMovie;
	case ACT_SET_WEAPON_FROM_TROOP:			return new CFcActSetWeaponFromTroop;
	case ACT_SET_TROOP_AI_DISABLE_IN_AREA_FOR_RM:	return new CFcSetTroopAIDisableInAreaForRM;
	case ACT_VAR_SCORE_TROOP_HP:			return new CFcActVarScoreTroopHP;
	case ACT_VAR_SCORE_TROOP_TEAM_HP:		return new CFcActVarScoreTroopTeamHP;
	case ACT_VAR_SCORE_TROOP_NUM:			return new CFcActVarScoreTroopNum;
	case ACT_VAR_SCORE_RANGE:				return new CFcActVarScoreRange;
	case ACT_VAR_SCORE_BROKEN_PROP:			return new CFcActVarScoreBrokenProp;
	case ACT_SET_PROP_CRUSH:				return new CFcActSetPropCrush;
	case ACT_SET_CORPSE_DELAY:				return new CFcActSetCorpseDelay;
	case ACT_PLAY_SOUND_EFFECT:						return new CFcActPlaySoundEffect;
	case ACT_ADD_MAXHP:								return new CFcActAddMaxHP;
	case ACT_SHOW_TROOP_HP_GAUGE_EXCEPT_LEADER:		return new CFcActShowTroopHPGaugeExceptLeader;
	case ACT_SET_DEFENSE_REF:						return new CFcActSetDefenseRef;
	case ACT_SHOW_CIRCLE_AREA_IN_MINIMAP:			return new CFcActShowCircleAreaInMinimap;
	case ACT_HIDE_CIRCLE_AREA_IN_MINIMAP:			return new CFcActHideCircleAreaInMinimap;
	case ACT_TROOP_ANIMATION_WITH_DELAY:			return new CFcTroopAnimationWithDelay;
	case ACT_TROOP_DAMAGED_ONLY_PLAYER:			return new CFcActTroopDamagedOnlyPlayer;
	case ACT_ITEM_PROBBT_ADD:					return new CFcActItemProbbtAdd;

	case ACT_VAR_SURVIVALRATE_SCORE_VAR_ARG:	return new CFcActVarSurvivalRateScoreVarArg;
	case ACT_VAR_KILLS_SCORE_VAR_ARG:			return new CFcActVarKillsScoreVarArg;

	case ACT_ENABLE_EVENT_TROOP_AI:				return new CFcActEnableEventTroopAI;
	case ACT_DISABLE_EVENT_TROOP_AI:			return new CFcActDisableEventTroopAI;

	case ACT_POINT_TROOP_STR_ON:				return new CFcActPointTroopStrOn;
	case ACT_POINT_TROOP_STR_OFF:				return new CFcActPointTroopStrOff;

	case ACT_SET_UNIT_AI_ENABLE:				return new CFcActSetUnitAIEnable;
	case ACT_SET_UNIT_AI_DISABLE:				return new CFcActSetUnitAIDisable;

	case ACT_TROOP_LAST_DISABLE:				return new CFcActTroopLastDisable;
	case ACT_TROOPS_LAST_DISABLE:				return new CFcActTroopsLastDisable;

	case ACT_VAR_MISSION_LEVEL:					return new CFcActVarMissionLevel;

	case ACT_VIG_MOTIONBLUR_ENABLE:				return new CFcActVigMotionBlurEnable;
	case ACT_VIG_MOTIONBLUR_DISABLE:			return new CFcActVigMotionBlurDisable;


	case ACT_SOUND_SE_PLAYING_OFF:				return new CFcActSeSoundPlayingOff;
	case ACT_SOUND_SE_PLAYING_ON:				return new CFcActSeSoundPlayingOn;


	}
	return NULL;
}


void CFcTriggerEventBase::AllCopyFromToolData(CBSTriggerEventBase *pTrigger)
{
	int i;
	for(i = 0;i < pTrigger->GetConditionCount();i++)
	{
		CBSConditionForTool		*pSrcCon = pTrigger->GetCondition(i);
		CBSConditionActionBase	*pDestCon = AllocateConditionByType(pSrcCon->GetCodeIndex()); //타입을 보컖E클래스를 선택해서 할큱E
		for(int si=0;si < pSrcCon->GetParamCount();si++){
			pDestCon->AddParameterData(pSrcCon->GetParamData(si)); //모탛E파라메터 데이터를 카피해 넣는다.
		}
		if(pSrcCon->GetOperIndex() != -1){
			pDestCon->SetOperIndex(pSrcCon->GetOperIndex());
		}
		pDestCon->SetSkip( pSrcCon->IsSkip() );
		pDestCon->Initialize();
		m_ConditionList.push_back(pDestCon);
	}

	for(i = 0;i < pTrigger->GetActionCount();i++)
	{
		CBSActionForTool		*pSrcAct = pTrigger->GetAction(i);
		CBSConditionActionBase	*pDestAct = AllocateActionByType(pSrcAct->GetCodeIndex());
		for(int si=0;si < pSrcAct->GetParamCount();si++){
			pDestAct->AddParameterData(pSrcAct->GetParamData(si));			
		}		
		pDestAct->Initialize();
		pDestAct->SetSkip(pSrcAct->IsSkip());
		m_ActionList.push_back(pDestAct);
	}
}

void CFcTriggerEventBase::Reset()
{
	m_bConSucd = false;
	m_bEnable = true;
	m_bLoop   = false;		

	unsigned int i = 0;
	for(i = 0;i < m_ConditionList.size();i++)
	{
		m_ConditionList[i]->Initialize();
	}
	for(i = 0;i < m_ActionList.size();i++)
	{
		m_ActionList[i]->SetEnable(true);
		m_ActionList[i]->Initialize();
	}
	
}

//-----------------------------------------------------------------------------------------------------
CFcTriggerCommander::CFcTriggerCommander( int nCheckTick )
{
	m_nCheckTick = nCheckTick;
	m_nCurTriggerIndex = -1;
	ReSetKeyHookOn();
}

CFcTriggerCommander::~CFcTriggerCommander()
{
	SAFE_DELETE_PVEC( m_TriggerList );
	SAFE_DELETE_PVEC( m_EventList );
	SAFE_DELETE_PVEC( m_ValList );

}

void CFcTriggerCommander::Load(char *FileName)
{
	m_bStop = false;

	CBSTriggerManager *pTriggerManager = new CBSTriggerManager();
	pTriggerManager->Load(FileName);
//	pTriggerManager->LoadToText(FileName);

	int i = 0;
	for(i = 0;i < pTriggerManager->GetTriggerCount();i++)
	{
		CFcTriggerEventBase *pTriger = new CFcTriggerEventBase;
		pTriger->AllCopyFromToolData((CBSTriggerEventBase *)pTriggerManager->GetTrigger(i));		
		m_TriggerList.push_back(pTriger);
	}

	for(i = 0;i < pTriggerManager->GetEventCount();i++)
	{
		CFcTriggerEventBase *pEvent = new CFcTriggerEventBase;
		pEvent->AllCopyFromToolData((CBSTriggerEventBase *)pTriggerManager->GetEvent(i));		
		m_EventList.push_back(pEvent);
	}
	
	for(i = 0;i < pTriggerManager->GetVariableCount();i++)
	{
		CBSVariable *pData = new CBSVariable();
		memcpyVar(pData,pTriggerManager->GetVariable(i));
		m_ValList.push_back(pData);
	}
	
	delete pTriggerManager;
}


bool CFcTriggerCommander::ConditionCheck(CFcTriggerEventBase *pTrigger)
{
	if(pTrigger->IsConSucd()) //한번 모두 True체크된 컨디션은 해당 트리거가 비활성화 될 때까지 그냥 True //액션이 계속 실행되기 위해서 넣어놨음
		return true;
	
	for(int i = 0;i < pTrigger->GetConditionCount();i++)
	{
		if(pTrigger->GetCondition(i)->GetOperIndex() == -1)
		{
			if(pTrigger->GetCondition(i)->IsSkip() == false) {
				if(pTrigger->GetCondition(i)->IsTrue() == false)
					return false;
			}
		}
		else
		{
			bool bResult = false;
			bool bSkipAll = true;
			bool bNotEnd = true;
			int j = i;
			for( j = i; j < pTrigger->GetConditionCount() && bNotEnd; j++)
			{
				if(pTrigger->GetCondition(j)->GetOperIndex() == -1)
				{
					bNotEnd = false;
				}

				if(bResult == true)
					continue;

				if(pTrigger->GetCondition(j)->IsSkip() == (BOOL)true){
					continue;
				}
				else{
					bSkipAll = false;
					if(pTrigger->GetCondition(j)->IsTrue() == true)
						bResult = true;
				}
			}
			i = j;

			if(bSkipAll == true)
				bResult = true;

			if (bResult == false)
				return false;
		}
	}
	pTrigger->SetConSucd(true);
	return true;
}

void CFcTriggerCommander::HookOnProcess()
{
	if(CInputPad::GetInstance().IsHookOn())
	{
		for(int i = 0;i < PAD_INPUT_RSTICK+1;i++)
		{
			if(CInputPad::GetInstance().GetKeyPressTick(g_FCGameData.nEnablePadID, i))
			{
				m_nKeyTypeForHookOn[i] = i;
			}
		}
	}
}

void CFcTriggerCommander::Process()
{	
	if( m_bStop )
		return;

	int iOrIndex = -1;
	int j = 0;
	int i = 0;

	HookOnProcess();
	if( (GetProcessTick() % m_nCheckTick) != 0 ){
		return;
	}

	for(i = 0;i < (int)m_TriggerList.size();i++)
	{
		m_nCurTriggerIndex = i; //현재 사용되고 있는 트리거에다가 명령을 주는 액션이 있기 때문에 필요하다

		if(m_TriggerList[i]->IsEnable() == false){ //비활성화 시킨 상태면 스킵
			continue;
		}

		if(m_TriggerList[i]->IsLoop() != true) //루프가 체크되어있는 트리거라면 컨디션이 스킵되고 무조건 액션이 실행
		{
			if(ConditionCheck(m_TriggerList[i]) == false)
				continue;
		}

		for(j = 0;j < m_TriggerList[i]->GetActionCount();j++)
		{
			if(m_TriggerList[i]->GetAction(j)->IsSkip() == TRUE)
				continue;

			if(m_TriggerList[i]->GetAction(j)->IsEnable() == false){
				continue; //실행됐던 액션이면 다음 액션으로...
			}
			if(m_bStop) { break; }

			int nState = m_TriggerList[i]->GetAction(j)->DoCommand();
			if( nState == ACTION_STATE_WORKING ){ break; } //일하고 있는 놈이면 루프에서 빠져나온다.

			DebugString("Current TriggerLine %d / ActionLine:%d\n",i,j);
			switch(nState)
			{
			case ACTION_STATE_FINISH:				
				m_TriggerList[i]->GetAction(j)->SetEnable(false);
				if(j == m_TriggerList[i]->GetActionCount()-1) //마지막 액션이면서
				{
					if(m_TriggerList[i]->IsLoop() != true) {  //루프가 체크되지 않은 액션이면
						m_TriggerList[i]->SetEnable(false);
					}
				}
				break;
			case ACTION_STATE_RESET:
				m_TriggerList[i]->Reset();
				break;
			}
		}
		if(m_bStop) { break; }
	}
}
	
void CFcTriggerCommander::DoEvent(BS_EVENT_CODE EventType)
{
	bool bConditionTrue = true;
	int iEventIndex = EventType;

	int i;
	for(i = 0;i < m_EventList[iEventIndex]->GetConditionCount();i++)
	{
		if(m_EventList[iEventIndex]->GetCondition(i)->IsTrue())
		{
		}
		else
		{
			bConditionTrue = false;
			break;
		}
	}
	if(!bConditionTrue)
		return;

	for(i = 0;i < m_EventList[iEventIndex]->GetActionCount();i++)
	{
		m_EventList[iEventIndex]->GetAction(i)->DoCommand();
	}
}


int CFcTriggerCommander::GetVariableType(int iIndex)
{
	return m_ValList[iIndex]->GetParamData(1)->GetInteger();
}
char *CFcTriggerCommander::GetVariableValStr(int iIndex)
{
	return m_ValList[iIndex]->GetParamData(2)->GetString();
}

int  CFcTriggerCommander::GetVariableValInteger(int iIndex)
{
	return m_ValList[iIndex]->GetParamData(2)->GetInteger();
}

float CFcTriggerCommander::GetVariableValFloat(int iIndex)
{
	return m_ValList[iIndex]->GetParamData(2)->GetDecimal();
}

void CFcTriggerCommander::SetVariableVal(int iIndex, int nValue)
{
	m_ValList[iIndex]->GetParamData(2)->SetData( nValue );
}

void CFcTriggerCommander::SetVariableVal(int iIndex, float fValue)
{
	m_ValList[iIndex]->GetParamData(2)->SetData( fValue );
}

void CFcTriggerCommander::SetVariableVal(int iIndex, char* pStr)
{
	m_ValList[iIndex]->GetParamData(2)->SetData( pStr );
}


void CFcTriggerCommander::ResetTrigger(int iTriggerIndex)
{
	BsAssert(iTriggerIndex < (int)m_TriggerList.size());
	if( iTriggerIndex >= (int)m_TriggerList.size() )
	{
		DebugString( "CFcTriggerCommander::ResetTrigger Invalid Trigger Index  Ask to yooty" ); 
		return;
	}

	m_TriggerList[iTriggerIndex]->Reset();
}

void CFcTriggerCommander::EnableTrigger(int iIndex,bool bEnable)
{
	m_TriggerList[iIndex]->SetEnable(bEnable);
}

void CFcTriggerCommander::SetLoopCurTrg(bool bLoop)
{
	if(m_nCurTriggerIndex != -1)
	{
		m_TriggerList[m_nCurTriggerIndex]->SetEnable(true);
		m_TriggerList[m_nCurTriggerIndex]->SetLoop(bLoop);
		for(int i = 0;i < m_TriggerList[m_nCurTriggerIndex]->GetActionCount();i++)
		{
			m_TriggerList[m_nCurTriggerIndex]->GetAction(i)->Initialize();
			m_TriggerList[m_nCurTriggerIndex]->GetAction(i)->SetEnable(true);
			
		}
	}
}

void CFcTriggerCommander::ResetAll()
{
	for(unsigned int i = 0;i < m_TriggerList.size();i++)
	{
		m_TriggerList[i]->Reset();
	}
}
void CFcTriggerCommander::ReSetKeyHookOn()
{
	for(int i = 0;i < PAD_INPUT_RSTICK+1;i++)
	{
		m_nKeyTypeForHookOn[i] = -1;
	}
}

short CFcTriggerCommander::GetKeyHookOn(int nKey)
{
	return m_nKeyTypeForHookOn[nKey];
}

int CFcTriggerCommander::GetVariableCount()
{
	return (int)m_ValList.size();
}

CBSVariable *CFcTriggerCommander::GetVariable( int iIndex )
{
	return m_ValList[iIndex];
}
