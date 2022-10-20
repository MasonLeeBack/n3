#include "StdAfx.h"
#include "FcAITriggerFactory.h"
#include "FcAITriggerBase.h"
#include "data/AIDef.h"
#include "FcAITriggerCon.h"
#include "FcAITriggerConTroop.h"
#include "FcAITriggerAction.h"
#include "FcAITriggerActionTroop.h"
#include "FcAITriggerConProp.h"
#include "FcAITriggerActionProp.h"
#include "FcParamVariable.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


// Trigger Param Count Define
AITriggerDefine g_AIConditionDefine[] = {
	{ AI_CON_IS_TARGET_MY_TEAM,					0, NULL },
	{ AI_CON_IS_TARGET_ENEMY_TEAM,				0, NULL },
	{ AI_CON_IS_TARGET_LIVE,					0, NULL },
	{ AI_CON_IS_TARGET_DIE,						0, NULL },
	{ AI_CON_IS_SELF_LIVE,						0, NULL },
	{ AI_CON_IS_SELF_DIE,						0, NULL },
	{ AI_CON_TARGET_HP,							2, NULL },
	{ AI_CON_SELF_HP,							2, NULL },
	{ AI_CON_TARGET_RANGE,						2, NULL },
	{ AI_CON_ENEMY_SEARCH_COUNT,				3, NULL },
	{ AI_CON_GLOBAL_VARIABLE_INT,				3, NULL },
	{ AI_CON_GLOBAL_VARIABLE_FLOAT,				3, NULL },
	{ AI_CON_GLOBAL_VARIABLE_STRING,			3, NULL },
	{ AI_CON_GLOBAL_VARIABLE_VECTOR,			5, NULL },
	{ AI_CON_GLOBAL_VARIABLE_VECTOR_LENGTH,		3, NULL },
	{ AI_CON_GLOBAL_VARIABLE_BOOLEAN,			3, NULL },
	{ AI_CON_GLOBAL_VARIABLE_RANDOM,			3, NULL },
	{ AI_CON_GLOBAL_VARIABLE_PTR,				3, NULL },
	{ AI_CON_CAN_ANI,							3, NULL },
	{ AI_CON_CANNOT_ANI,						3, NULL },
	{ AI_CON_SELF_ANI,							4, NULL },
	{ AI_CON_TARGET_ANI,						4, NULL },
	{ AI_CON_IS_VALID_SLOT,						1, NULL },
	{ AI_CON_IS_INVALID_SLOT,					1, NULL },
	{ AI_CON_FUNCTION,							3, NULL },
	{ AI_CON_INFLUENCE_TARGETING_COUNT,			2, NULL },
	{ AI_CON_INFLUENCE_DAMAGE,					2, NULL },
	{ AI_CON_INFLUENCE_HIT_COUNT,				2, NULL },
	{ AI_CON_JOB_INDEX,							2, NULL },
	{ AI_CON_TARGET_CLASS_ID,					2, NULL },
	{ AI_CON_TARGET_LINK_CLASS_ID,				2, NULL },
	{ AI_CON_IS_SHOW_WEAPON,					1, NULL },
	{ AI_CON_IS_HIDE_WEAPON,					1, NULL },
	{ AI_CON_TARGET_IS_VALID_LINK,				0, NULL },
	{ AI_CON_TARGET_IS_INVALID_LINK,			0, NULL },
	{ AI_CON_TARGET_VIEW_ANGLE,					2, NULL },
	{ AI_CON_TARGET_IS_VALID,					0, NULL },
	{ AI_CON_TARGET_IS_INVALID,					0, NULL },
	{ AI_CON_IS_PROCESS_TRUE_ORB,				1, NULL },
	{ AI_CON_SELF_EVENT_SEQ_COUNT,				2, NULL },
	{ AI_CON_AICIDX,							2, NULL },
	{ AI_CON_IS_VALID_CATCH,					0, NULL },
	{ AI_CON_IS_INVALID_CATCH,					0, NULL },

	{ AI_CON_TROOP_STATE,						2, NULL },
	{ AI_CON_TROOP_STATE_TABLE,					2, NULL },
	{ AI_CON_TROOP_SUB_STATE,					2, NULL },
	{ AI_CON_TROOP_OFFSET_RANGE,				2, NULL },
	{ AI_CON_TROOP_TARGET_OFFSET_RANGE,			2, NULL },
	{ AI_CON_TROOP_TARGET_TROOP_RANGE,			2, NULL },
	{ AI_CON_TROOP_IS_IN_RANGE,					1, NULL },
	{ AI_CON_TROOP_TARGET_IS_IN_RANGE,			1, NULL },
	{ AI_CON_TROOP_TARGET_IS_IN_TROOP,			1, NULL },
	{ AI_CON_TROOP_TARGET_IS_UNTOUCHABLE,		1, NULL },
	{ AI_CON_TROOP_READY_RANGE_ORDER,			3, NULL },
	{ AI_CON_TROOP_TARGET_IS_PLAYER,			1, NULL },

	{ AI_CON_DEBUG_STRING,						1, NULL },
	{ -1 },
};

AITriggerDefine g_AIActionDefine[] = {
	{ AI_ACTION_DELAY,							1, NULL },
	{ AI_ACTION_SET_ANI,						4, NULL },
	{ AI_ACTION_SET_ANI_LOOP,					5, NULL },
	{ AI_ACTION_FOLLOW_TARGET,					5, NULL },
	{ AI_ACTION_OPPOSITION_TARGET,				1, NULL },
	{ AI_ACTION_OPPOSITION_TARGET_TO_ME,		1, NULL },
	{ AI_ACTION_STOP,							3, NULL },
	{ AI_ACTION_LOOK_TARGET,					0, NULL },
	{ AI_ACTION_LOOK_CUSTOM,					1, NULL },
	{ AI_ACTION_CHANGE_GLOBAL_VARIABLE_INT,		2, NULL },
	{ AI_ACTION_CHANGE_GLOBAL_VARIABLE_FLOAT,	2, NULL },
	{ AI_ACTION_CHANGE_GLOBAL_VARIABLE_STRING,	2, NULL },
	{ AI_ACTION_CHANGE_GLOBAL_VARIABLE_VECTOR,	4, NULL },
	{ AI_ACTION_CHANGE_GLOBAL_VARIABLE_BOOLEAN,	2, NULL },
	{ AI_ACTION_CHANGE_GLOBAL_VARIABLE_RANDOM,	3, NULL },
	{ AI_ACTION_CHANGE_GLOBAL_VARIABLE_PTR,		1, NULL },
	{ AI_ACTION_SET_TARGET_GLOBAL_VARIABLE,		1, NULL },
	{ AI_ACTION_SET_TARGET_SLOT,				1, NULL },
	{ AI_ACTION_EMPTY_SLOT,						1, NULL },
	{ AI_ACTION_LOOK_SLOT,						2, NULL },
	{ AI_ACTION_INFLUENCE_TARGETING,			1, NULL },
	{ AI_ACTION_INFLUENCE_TARGETING_SLOT,		2, NULL },
	{ AI_ACTION_INFLUENCE_TARGETING_EMPTY,		0, NULL },
	{ AI_ACTION_INFLUENCE_DAMAGE_EMPTY,			0, NULL },
	{ AI_ACTION_INFLUENCE_HIT_COUNT_EMPTY,		0, NULL },

	{ AI_ACTION_CMD_ATTACK,						0, NULL },
	{ AI_ACTION_CUSTOM_MOVE,					6, NULL },
	{ AI_ACTION_SELF_CUSTOM_MOVE,				6, NULL },

	{ AI_ACTION_EXIT,							0, NULL },
	{ AI_ACTION_PUSH_BUTTON,					1, NULL },
	{ AI_ACTION_RELEASE_BUTTON,					1, NULL },

	{ AI_ACTION_ADD_DEFENSE_PROB,				1, NULL },
	{ AI_ACTION_DESTROY_ELEMENT,				1, NULL },
	{ AI_ACTION_LINK_OBJECT,					0, NULL },
	{ AI_ACTION_UNLINK_OBJECT,					0, NULL },
	{ AI_ACTION_SET_ARCHER_HIT_PROB,			1, NULL },
	{ AI_ACTION_GLOBAL_VARIABLE_INT_COUNTER,	2, NULL },
	{ AI_ACTION_GLOBAL_VARIABLE_INT_RANDOM2,	4, NULL },
	{ AI_ACTION_GLOBAL_VARIABLE_INT_RANDOM3,	6, NULL },
	{ AI_ACTION_GLOBAL_VARIABLE_INT_RANDOM4,	8, NULL },
	{ AI_ACTION_SET_AICIDX,						1, NULL },
	{ AI_ACTION_PAUSE_FUNCTION,					1, NULL },
	{ AI_ACTION_UNPAUSE_FUNCTION,				1, NULL },
	{ AI_ACTION_RESET_MOVE_VECTOR,				0, NULL },

	{ AI_ACTION_FOLLOW_TROOP,					4, NULL },

	{ AI_ACTION_DEBUG_STRING,					1, NULL },
	{ AI_ACTION_DELAY_ORDER,					1, NULL },
	{ AI_ACTION_WARP,							1, NULL },
	{ AI_ACTION_DIMINUTION_VELOCITY,			1, NULL },
	{ -1 },
};

AITriggerDefine g_AISearchDefine[] = {
	{ AI_SEARCH_ALL,						1, NULL },
	{ AI_SEARCH_GET,						1, NULL },
	{ AI_SEARCH_GLOBAL_VARIABLE,			1, NULL },
	{ AI_SEARCH_SLOT,						1, NULL },
	{ AI_SEARCH_ALL_TROOP_OFFSET,			1, NULL },
	{ AI_SEARCH_ALL_TARGET_TROOP_OFFSET,	1, NULL },
	{ AI_SEARCH_CORRELATION,				1, NULL },
	{ AI_SEARCH_ALL_TROOP,					2, NULL },
	{ AI_SEARCH_LINK_PARENT_TROOP,			0, NULL },
	{ -1 },
};

AITriggerDefine g_AISearchFilterDefine[] = {
	{ AI_SEARCH_FILTER_NONE,			0, NULL },
	{ AI_SEARCH_FILTER_RANDOM,			0, NULL },
	{ AI_SEARCH_FILTER_HERO,			0, NULL },
	{ AI_SEARCH_FILTER_NEAREST,			0, NULL },
	{ AI_SEARCH_FILTER_FARTHEST,		0, NULL },
	{ AI_SEARCH_FILTER_TARGETING_COUNT,	2, NULL },
	{ AI_SEARCH_FILTER_LAST_HIT,		0, NULL },
	{ AI_SEARCH_FILTER_DIR,				2, NULL },
	{ AI_SEARCH_FILTER_TROOP_OFFSET_NEAREST, 0, NULL },
	{ AI_SEARCH_FILTER_TROOP_OFFSET_FARTHEST, 0, NULL },
	{ AI_SEARCH_FILTER_PREFERENCE_CLASS_ID,	3, NULL },
	{ -1 },
};

AITriggerDefine g_AISearchConDefine[] = {
	{ AI_SEARCH_CON_NONE,				0, NULL },
	{ AI_SEARCH_CON_IS_VALID_SLOT,		1, NULL },
	{ AI_SEARCH_CON_IS_INVALID_SLOT,	1, NULL },
	{ AI_SEARCH_CON_GLOBAL_VARIABLE_INT,3, NULL },
	{ -1 },
};

int GetAIConParamNum( int nID )
{
	int nCount = 0;
	while(1) {
		if( g_AIConditionDefine[nCount].nTypeIndex == -1 ) break;
		if( g_AIConditionDefine[nCount].nTypeIndex == nID ) 
			return g_AIConditionDefine[nCount].nParamCount;
		++nCount;
	}
	BsAssert(0);
	return 0;
}

int GetAIActionParamNum( int nID )
{
	int nCount = 0;
	while(1) {
		if( g_AIActionDefine[nCount].nTypeIndex == -1 ) break;
		if( g_AIActionDefine[nCount].nTypeIndex == nID ) 
			return g_AIActionDefine[nCount].nParamCount;
		++nCount;
	}
	BsAssert(0);
	return 0;
}

int GetAISearchParamNum( int nID )
{
	int nCount = 0;
	while(1) {
		if( g_AISearchDefine[nCount].nTypeIndex == -1 ) break;
		if( g_AISearchDefine[nCount].nTypeIndex == nID ) 
			return g_AISearchDefine[nCount].nParamCount;
		++nCount;
	}

	nCount = 0;
	while(1) {
		if( g_AISearchFilterDefine[nCount].nTypeIndex == -1 ) break;
		if( g_AISearchFilterDefine[nCount].nTypeIndex == nID ) 
			return g_AISearchFilterDefine[nCount].nParamCount;
		++nCount;
	}
	nCount = 0;
	while(1) {
		if( g_AISearchConDefine[nCount].nTypeIndex == -1 ) break;
		if( g_AISearchConDefine[nCount].nTypeIndex == nID ) 
			return g_AISearchConDefine[nCount].nParamCount;
		++nCount;
	}

	BsAssert(0);
	return 0;
}

CFcAIConditionBase *CFcAIConditionBase::CreateObject( int nID, CFcAISearch::SEARCHER_TYPE SearcherType )
{
	CFcAIConditionBase* pBase = NULL;
	switch( nID )
	{
	case AI_CON_IS_TARGET_MY_TEAM:
		pBase = new CFcAIConTargetTeamCheck( TRUE );
		break;
	case AI_CON_IS_TARGET_ENEMY_TEAM:
		pBase = new CFcAIConTargetTeamCheck( FALSE );
		break;
	case AI_CON_IS_TARGET_LIVE:
		pBase = new CFcAIConTargetLiveCheck( TRUE );
		break;
	case AI_CON_IS_TARGET_DIE:
		pBase = new CFcAIConTargetLiveCheck( FALSE );
		break;
	case AI_CON_IS_SELF_LIVE:
		pBase = new CFcAIConSelfLiveCheck( TRUE );
		break;
	case AI_CON_IS_SELF_DIE:
		pBase = new CFcAIConSelfLiveCheck( FALSE );
		break;
	case AI_CON_TARGET_HP:
		pBase = new CFcAIConUnitState( TRUE, 0 );
		break;
	case AI_CON_SELF_HP:
		pBase = new CFcAIConUnitState( FALSE, 0 );
		break;
	case AI_CON_TARGET_RANGE:
		switch( SearcherType ) {
			case CFcAISearch::GAMEOBJECT:
				pBase = new CFcAIConTargetRange();
				break;
			case CFcAISearch::PROP:
				pBase = new CFcAIConTargetPropRange();
				break;
		}
		break;
	case AI_CON_ENEMY_SEARCH_COUNT:
		pBase = new CFcAIConEnemySearchCount();
		break;
	case AI_CON_GLOBAL_VARIABLE_INT:
		pBase = new CFcAIConGlobalVariable( CFcParamVariable::INT );
		break;
	case AI_CON_GLOBAL_VARIABLE_FLOAT:
		pBase = new CFcAIConGlobalVariable( CFcParamVariable::FLOAT );
		break;
	case AI_CON_GLOBAL_VARIABLE_STRING:
		pBase = new CFcAIConGlobalVariable( CFcParamVariable::STRING );
		break;
	case AI_CON_GLOBAL_VARIABLE_VECTOR:
		pBase = new CFcAIConGlobalVariable( CFcParamVariable::VECTOR );
		break;
	case AI_CON_GLOBAL_VARIABLE_VECTOR_LENGTH:
		pBase = new CFcAIConGlobalVariable( CFcParamVariable::VECTOR );
		break;
	case AI_CON_GLOBAL_VARIABLE_BOOLEAN:
		pBase = new CFcAIConGlobalVariable( CFcParamVariable::BOOLEAN );
		break;
	case AI_CON_GLOBAL_VARIABLE_RANDOM:
		pBase = new CFcAIConGlobalVariable( CFcParamVariable::RANDOM );
		break;
	case AI_CON_GLOBAL_VARIABLE_PTR:
		pBase = new CFcAIConGlobalVariable( CFcParamVariable::PTR );
		break;
	case AI_CON_CAN_ANI:
		pBase = new CFcAIConCanAni( TRUE );
		break;
	case AI_CON_CANNOT_ANI:
		pBase = new CFcAIConCanAni( FALSE );
		break;
	case AI_CON_SELF_ANI:
		pBase = new CFcAIConAniType( FALSE );
		break;
	case AI_CON_TARGET_ANI:
		pBase = new CFcAIConAniType( TRUE );
		break;
	case AI_CON_IS_VALID_SLOT:
		pBase = new CFcAIConCheckSlot( TRUE );
		break;
	case AI_CON_IS_INVALID_SLOT:
		pBase = new CFcAIConCheckSlot( FALSE );
		break;
	case AI_CON_TROOP_STATE:
		pBase = new CFcAIConTroopState();
		break;
	case AI_CON_TROOP_STATE_TABLE:
		pBase = new CFcAICoTroopStateTable();
		break;
	case AI_CON_TROOP_SUB_STATE:
		pBase = new CFcAIConTroopSubState();
		break;
	case AI_CON_TROOP_OFFSET_RANGE:
		pBase = new CFcAIConTroopOffsetRange( FALSE );
		break;
	case AI_CON_TROOP_TARGET_OFFSET_RANGE:
		pBase = new CFcAIConTroopOffsetRange( TRUE );
		break;
	case AI_CON_TROOP_TARGET_TROOP_RANGE:
		pBase = new CFcAIConTroopTargetTroopRange();
		break;
	case AI_CON_DEBUG_STRING:
		pBase = new CFcAIConDebugString();
		break;
	case AI_CON_FUNCTION:
		pBase = new CFcAIConFunction();
		break;
	case AI_CON_INFLUENCE_TARGETING_COUNT:
		pBase = new CFcAIConInfluenceTargetingCount();
		break;
	case AI_CON_JOB_INDEX:
		pBase = new CFcAIConJobIndex();
		break;
	case AI_CON_TROOP_IS_IN_RANGE:
		pBase = new CFcAIConTroopInRange( FALSE );
		break;
	case AI_CON_TROOP_TARGET_IS_IN_RANGE:
		pBase = new CFcAIConTroopInRange( TRUE );
		break;
	case AI_CON_TROOP_TARGET_IS_IN_TROOP:
		pBase = new CFcAIConTroopTargetInTroop();
		break;
	case AI_CON_TARGET_CLASS_ID:
		switch( SearcherType ) {
			case CFcAISearch::GAMEOBJECT:
				pBase = new CFcAIConClassID();
				break;
			case CFcAISearch::PROP:
				pBase = new CFcAIConTargetPropType();
				break;
		}
		break;
	case AI_CON_TARGET_LINK_CLASS_ID:
		pBase = new CFcAIConLinkClassID();
		break;
	case AI_CON_IS_SHOW_WEAPON:
		pBase = new CFcAIConWeapon( TRUE );
		break;
	case AI_CON_IS_HIDE_WEAPON:
		pBase = new CFcAIConWeapon( FALSE );
		break;
	case AI_CON_TARGET_IS_VALID_LINK:
		pBase = new CFcAIConIsValidLink( TRUE );
		break;
	case AI_CON_TARGET_IS_INVALID_LINK:
		pBase = new CFcAIConIsValidLink( FALSE );
		break;
	case AI_CON_TARGET_VIEW_ANGLE:
		pBase = new CFcAIConTargetViewAngle();
		break;
	case AI_CON_TARGET_IS_VALID:
		switch( SearcherType ) {
			case CFcAISearch::GAMEOBJECT:
				pBase = new CFcAIConTargetValid( TRUE );
				break;
			case CFcAISearch::PROP:
				pBase = new CFcAIConTargetPropValid( TRUE );
				break;
		}
		break;
	case AI_CON_TARGET_IS_INVALID:
		switch( SearcherType ) {
			case CFcAISearch::GAMEOBJECT:
				pBase = new CFcAIConTargetValid( FALSE );
				break;
			case CFcAISearch::PROP:
				pBase = new CFcAIConTargetPropValid( FALSE );
				break;
		}
		break;
	case AI_CON_IS_PROCESS_TRUE_ORB:
		pBase = new CFcAIConIsProcessTrueOrb();
		break;
	case AI_CON_SELF_EVENT_SEQ_COUNT:
		pBase = new CFcAIConEventSeqCount( FALSE );
		break;
	case AI_CON_AICIDX:
		pBase = new CFcAIConControlIndex();
		break;
	case AI_CON_INFLUENCE_DAMAGE:
		pBase = new CFcAIConInfluenceDamage();
		break;
	case AI_CON_INFLUENCE_HIT_COUNT:
		pBase = new CFcAIConInfluenceHitCount();
		break;
	case AI_CON_TROOP_TARGET_IS_UNTOUCHABLE:
		pBase = new CFcAIConTroopIsUntouchable();
		break;
	case AI_CON_IS_VALID_CATCH:
		pBase = new CFcAIConCheckCatch( TRUE );
		break;
	case AI_CON_IS_INVALID_CATCH:
		pBase = new CFcAIConCheckCatch( FALSE );
		break;
	case AI_CON_TROOP_READY_RANGE_ORDER:
		pBase = new CFcAIConTroopReadyRangeOrder();
		break;
	case AI_CON_TROOP_TARGET_IS_PLAYER:
		pBase = new CFcAIConTroopTargetIsPlayer();
		break;

	default:
		BsAssert(0);
		return NULL;
	}
	pBase->m_nID = nID;

	return pBase;
}

CFcAIActionBase *CFcAIActionBase::CreateObject( int nID, CFcAISearch::SEARCHER_TYPE SearcherType )
{
	CFcAIActionBase* pBase = NULL;
	switch( nID )
	{
	case AI_ACTION_DELAY:
		pBase = new CFcAIActionDelay();
		break;
	case AI_ACTION_SET_ANI:
		pBase = new CFcAIActionSetAni();
		break;
	case AI_ACTION_SET_ANI_LOOP:
		pBase = new CFcAIActionSetAni( TRUE );
		break;
	case AI_ACTION_FOLLOW_TARGET:
		switch( SearcherType ) {
			case CFcAISearch::GAMEOBJECT:
				pBase = new CFcAIActionFollowTarget();
				break;
			case CFcAISearch::PROP:
				pBase = new CFcAIActionFollowProp();
				break;
		}
		break;
	case AI_ACTION_OPPOSITION_TARGET:
		pBase = new CFcAIActionOppositionTarget( TRUE );
		break;
	case AI_ACTION_OPPOSITION_TARGET_TO_ME:
		pBase = new CFcAIActionOppositionTarget( FALSE );
		break;

	case AI_ACTION_STOP:
		pBase = new CFcAIActionStop();
		break;
	case AI_ACTION_LOOK_TARGET:
		pBase = new CFcAIActionLookTarget();
		break;
	case AI_ACTION_LOOK_CUSTOM:
		pBase = new CFcAIActionLookCustom();
		break;
	case AI_ACTION_CHANGE_GLOBAL_VARIABLE_INT:
		pBase = new CFcAIActionChangeGlobalVariable( CFcParamVariable::INT );
		break;
	case AI_ACTION_CHANGE_GLOBAL_VARIABLE_FLOAT:
		pBase = new CFcAIActionChangeGlobalVariable( CFcParamVariable::FLOAT );
		break;
	case AI_ACTION_CHANGE_GLOBAL_VARIABLE_STRING:
		pBase = new CFcAIActionChangeGlobalVariable( CFcParamVariable::STRING );
		break;
	case AI_ACTION_CHANGE_GLOBAL_VARIABLE_VECTOR:
		pBase = new CFcAIActionChangeGlobalVariable( CFcParamVariable::VECTOR );
		break;
	case AI_ACTION_CHANGE_GLOBAL_VARIABLE_BOOLEAN:
		pBase = new CFcAIActionChangeGlobalVariable( CFcParamVariable::BOOLEAN );
		break;
	case AI_ACTION_CHANGE_GLOBAL_VARIABLE_RANDOM:
		pBase = new CFcAIActionChangeGlobalVariable( CFcParamVariable::RANDOM );
		break;
	case AI_ACTION_CHANGE_GLOBAL_VARIABLE_PTR:
		pBase = new CFcAIActionChangeGlobalVariable( CFcParamVariable::PTR );
		break;
	case AI_ACTION_FOLLOW_TROOP:
		pBase = new CFcAIActionTroopFollow();
		break;
	case AI_ACTION_SET_TARGET_GLOBAL_VARIABLE:
		pBase = new CFcAIActionSetTargetGlobalValue();
		break;
	case AI_ACTION_SET_TARGET_SLOT:
		switch( SearcherType ) {
			case CFcAISearch::GAMEOBJECT:
				pBase = new CFcAIActionSetTargetSlot();
				break;
			case CFcAISearch::PROP:
				pBase = new CFcAIActionSetTargetSlotProp();
				break;
		}
		break;
	case AI_ACTION_EMPTY_SLOT:
		pBase = new CFcAIActionEmptySlot();
		break;
	case AI_ACTION_LOOK_SLOT:
		pBase = new CFcAIActionLookSlot();
		break;
	case AI_ACTION_DEBUG_STRING:
		pBase = new CFcAIActionDebugString();
		break;
	case AI_ACTION_INFLUENCE_TARGETING:
		pBase = new CFcAIActionInfluenceTargeting( FALSE );
		break;
	case AI_ACTION_INFLUENCE_TARGETING_SLOT:
		pBase = new CFcAIActionInfluenceTargetingSlot();
		break;
	case AI_ACTION_INFLUENCE_TARGETING_EMPTY:
		pBase = new CFcAIActionInfluenceTargeting( TRUE );
		break;
	case AI_ACTION_CMD_ATTACK:
		pBase = new CFcAIActionCmd( 0 );
		break;
	case AI_ACTION_CUSTOM_MOVE:
		pBase = new CFcAIActionCustomMove();
		break;
	case AI_ACTION_PUSH_BUTTON:
		pBase = new CFcAIActionPushButton();
		break;
	case AI_ACTION_RELEASE_BUTTON:
		pBase = new CFcAIActionReleaseButton();
		break;
	case AI_ACTION_EXIT:
		pBase = new CFcAIActionExit();
		break;
	case AI_ACTION_ADD_DEFENSE_PROB:
		pBase = new CFcAIActionAddDefenseProb();
		break;
	case AI_ACTION_DESTROY_ELEMENT:
		pBase = new CFcAIActionDestroyElement();
		break;
	case AI_ACTION_LINK_OBJECT:
		pBase = new CFcAIActionLinkObject();
		break;
	case AI_ACTION_UNLINK_OBJECT:
		pBase = new CFcAIActionUnlinkObject();
		break;
	case AI_ACTION_SET_ARCHER_HIT_PROB:
		pBase = new CFcAIActionSetArcherHitProb();
		break;
	case AI_ACTION_GLOBAL_VARIABLE_INT_COUNTER:
		pBase = new CFcAIActionGlobalVariableCounter( CFcParamVariable::INT );
		break;
	case AI_ACTION_SET_AICIDX:
		pBase = new CFcAIActionSetControlIndex();
		break;
	case AI_ACTION_INFLUENCE_DAMAGE_EMPTY:
		pBase = new CFcAIActionInfluenceDamageEmpty();
		break;
	case AI_ACTION_INFLUENCE_HIT_COUNT_EMPTY:
		pBase = new CFcAIActionInfluenceHitCountEmpty();
		break;
	case AI_ACTION_GLOBAL_VARIABLE_INT_RANDOM2:
		pBase = new CFcAIActionGlobalVariableIntRandom( 2 );
		break;
	case AI_ACTION_GLOBAL_VARIABLE_INT_RANDOM3:
		pBase = new CFcAIActionGlobalVariableIntRandom( 3 );
		break;
	case AI_ACTION_GLOBAL_VARIABLE_INT_RANDOM4:
		pBase = new CFcAIActionGlobalVariableIntRandom( 4 );
		break;
	case AI_ACTION_SELF_CUSTOM_MOVE:
		pBase = new CFcAIActionSelfCustomMove();
		break;
	case AI_ACTION_DELAY_ORDER:
		pBase = new CFcAIActionDelayOrder();
		break;
	case AI_ACTION_PAUSE_FUNCTION:
		pBase = new CFcAIActionPauseFunction();
		break;
	case AI_ACTION_UNPAUSE_FUNCTION:
		pBase = new CFcAIActionUnpauseFunction();
		break;
	case AI_ACTION_WARP:
		pBase = new CFcAIActionWarp();
		break;
	case AI_ACTION_DIMINUTION_VELOCITY:
		pBase = new CFcAIActionDiminutionVelocity();
		break;
	case AI_ACTION_RESET_MOVE_VECTOR:
		pBase = new CFcAIActionResetMoveVector();
		break;
	default:
		return NULL;
	}
	pBase->m_nID = nID;

	return pBase;
}
//////////////////////////////////////////////