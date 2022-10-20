#include "stdafx.h"
#include "BSTriggerManager.h"
#include "bstreamext.h"
#include "BSTriggerDefaultData.h"
#include "BSTriggerCodeDefine.h"
#include "BSTriggerCondition.h"
#include "BSTriggerAction.h"
#include "BSTriggerVariable.h"
#include "BSTriggerEvent.h"
#include "BSTrigger.h"
#include "Parser.h"
#include "Token.h"
#include "BsUtil.h"


#ifdef _USAGE_TOOL_
	#include "BSMisc.h"
	#include "BSCommonMsg.h"	// area,troop 관련 통지 메시지.
#endif


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

#define FC_LINE_END_CHAR		10

const int FC_TRIGGER_FILE_VER = 1200;

BSEventCodeInfo g_EventCodeTable[BTEC_LAST] =
{
	{ BTEC_HERO_DIE	 , "Activated when hero dies." },
	{ BTEC_MISSION_END, "Activated when mission ends." }
};

void GetConditionCodeNameByIndex(int iIndex,char *pBuf, const size_t pBuf_len)
{
	 //aleksger - safe string replaced strcpy with strcpy_s
	switch(iIndex)
	{
	case CON_INPUTSTICK_A:					strcpy_s(pBuf, pBuf_len,ConvertToString(CON_INPUTSTICK_A));					return;
	case CON_INPUTSTICK_LEFT:				strcpy_s(pBuf, pBuf_len,ConvertToString(CON_INPUTSTICK_LEFT));				return;
	case CON_INPUTSTICK_RIGHT:				strcpy_s(pBuf, pBuf_len,ConvertToString(CON_INPUTSTICK_RIGHT));				return;
	case CON_INPUTSTICK_NEUTRAL:			strcpy_s(pBuf, pBuf_len,ConvertToString(CON_INPUTSTICK_NEUTRAL));			return;
	case CON_COMPARE_ORB:					strcpy_s(pBuf, pBuf_len,ConvertToString(CON_COMPARE_ORB));					return;
	case CON_TIME_ELAPSED:					strcpy_s(pBuf, pBuf_len,ConvertToString(CON_TIME_ELAPSED));					return;
	case CON_TIME_ELAPSED_FROM_MARK:		strcpy_s(pBuf, pBuf_len,ConvertToString(CON_TIME_ELAPSED_FROM_MARK));		return;
	case CON_TROOP_ATTACKED:				strcpy_s(pBuf, pBuf_len,ConvertToString(CON_TROOP_ATTACKED));				return;
	case CON_TROOP_ATTACKED_TROOP:			strcpy_s(pBuf, pBuf_len,ConvertToString(CON_TROOP_ATTACKED_TROOP));			return;
	case CON_COMPARE_TROOP_DIST:			strcpy_s(pBuf, pBuf_len,ConvertToString(CON_COMPARE_TROOP_DIST));			return;
	case CON_COMPARE_TROOP_HP:				strcpy_s(pBuf, pBuf_len,ConvertToString(CON_COMPARE_TROOP_HP));				return;
	case CON_TROOP_IN_AREA:					strcpy_s(pBuf, pBuf_len,ConvertToString(CON_TROOP_IN_AREA));				return;
	case CON_COMPARE_LEADER_HP:				strcpy_s(pBuf, pBuf_len,ConvertToString(CON_COMPARE_LEADER_HP));			return;
	case CON_TROOP_MELEE_ATTACKED:			strcpy_s(pBuf, pBuf_len,ConvertToString(CON_TROOP_MELEE_ATTACKED));			return;
	case CON_TROOP_RANGE_ATTACKED:			strcpy_s(pBuf, pBuf_len,ConvertToString(CON_TROOP_RANGE_ATTACKED));			return;
	case CON_TROOP_UNBLOCKABLE_ATTACKED:	strcpy_s(pBuf, pBuf_len,ConvertToString(CON_TROOP_UNBLOCKABLE_ATTACKED));	return;
	case CON_TROOP_NO_ENGAGED:				strcpy_s(pBuf, pBuf_len,ConvertToString(CON_TROOP_NO_ENGAGED));				return;
	case CON_TROOP_NOT_IN_AREA:				strcpy_s(pBuf, pBuf_len,ConvertToString(CON_TROOP_NOT_IN_AREA));			return;
	case CON_VAR:							strcpy_s(pBuf, pBuf_len,ConvertToString(CON_VAR));							return;
	case CON_PROP_WAS_DESTROYED:			strcpy_s(pBuf, pBuf_len,ConvertToString(CON_PROP_WAS_DESTROYED));			return;
	case CON_TROOP_ENGAGED:					strcpy_s(pBuf, pBuf_len,ConvertToString(CON_TROOP_ENGAGED));				return;



	case CON_FORCE_COMPARE_HP:				strcpy_s(pBuf, pBuf_len,ConvertToString(CON_FORCE_COMPARE_HP));				return;
    case CON_FORCE_ELIMINATED:				strcpy_s(pBuf, pBuf_len,ConvertToString(CON_FORCE_ELIMINATED));				return;
	case CON_FORCE_IN_AREA:					strcpy_s(pBuf, pBuf_len,ConvertToString(CON_FORCE_IN_AREA));				return;
	case CON_FORCE_NOT_IN_AREA:				strcpy_s(pBuf, pBuf_len,ConvertToString(CON_FORCE_NOT_IN_AREA));			return;
	case CON_FORCE_ALL_IN_AREA:				strcpy_s(pBuf, pBuf_len,ConvertToString(CON_FORCE_ALL_IN_AREA));			return;
	case CON_PLAYER_GUARDIAN_NUM:			strcpy_s(pBuf, pBuf_len,ConvertToString(CON_PLAYER_GUARDIAN_NUM));			return;
	case CON_TROOP_TYPE:					strcpy_s(pBuf, pBuf_len,ConvertToString(CON_TROOP_TYPE));					return;
	case CON_GUARDIAN_TROOP_TYPE:			strcpy_s(pBuf, pBuf_len,ConvertToString(CON_GUARDIAN_TROOP_TYPE));			return;
	case CON_TROOP_LEVEL:					strcpy_s(pBuf, pBuf_len,ConvertToString(CON_TROOP_LEVEL));					return;
	case CON_ORB_GAUGE_FULL:				strcpy_s(pBuf, pBuf_len,ConvertToString(CON_ORB_GAUGE_FULL));				return;
	case CON_TRUE_ORB_GAUGE_FULL:			strcpy_s(pBuf, pBuf_len,ConvertToString(CON_TRUE_ORB_GAUGE_FULL));			return;
	case CON_TOTAL_KILL_NUM:				strcpy_s(pBuf, pBuf_len,ConvertToString(CON_TOTAL_KILL_NUM));				return;
	case CON_FORCE_KILL_NUM:				strcpy_s(pBuf, pBuf_len,ConvertToString(CON_FORCE_KILL_NUM));				return;
	case CON_GUARDIAN_IN_AREA:				strcpy_s(pBuf, pBuf_len,ConvertToString(CON_GUARDIAN_IN_AREA));				return;
	case CON_GUARDIAN_NOT_IN_AREA:			strcpy_s(pBuf, pBuf_len,ConvertToString(CON_GUARDIAN_NOT_IN_AREA));			return;
	case CON_GUARDIAN_COMPARE_DIST:			strcpy_s(pBuf, pBuf_len,ConvertToString(CON_GUARDIAN_COMPARE_DIST));		return;
	case CON_GUARDIAN_EXIST:				strcpy_s(pBuf, pBuf_len,ConvertToString(CON_GUARDIAN_EXIST));				return;
	case CON_GUARDIAN_TROOP_TYPE2:			strcpy_s(pBuf, pBuf_len,ConvertToString(CON_GUARDIAN_TROOP_TYPE2));			return;
	case CON_SELECT_MISION_MENU_VALUE:		strcpy_s(pBuf, pBuf_len,ConvertToString(CON_SELECT_MISION_MENU_VALUE));		return;

	case CON_FORCE_COMPARE_UNIT_NUMBER:		strcpy_s(pBuf, pBuf_len,ConvertToString(CON_FORCE_COMPARE_UNIT_NUMBER));	return;
	case CON_TROOP_COMPARE_UNIT_NUMBER:		strcpy_s(pBuf, pBuf_len,ConvertToString(CON_TROOP_COMPARE_UNIT_NUMBER));	return;
	case CON_REALMOVIE_FINISH:				strcpy_s(pBuf, pBuf_len,ConvertToString(CON_REALMOVIE_FINISH));				return;

	case CON_COMPARE_TROOP_N_FORCE_DIST:	strcpy_s(pBuf, pBuf_len,ConvertToString(CON_COMPARE_TROOP_N_FORCE_DIST));	return;
	case CON_COMPARE_FORCE_N_FORCE_DIST:	strcpy_s(pBuf, pBuf_len,ConvertToString(CON_COMPARE_FORCE_N_FORCE_DIST));	return;

	case CON_VAR_COMPARE:					strcpy_s(pBuf, pBuf_len,ConvertToString(CON_VAR_COMPARE));					return;

	case CON_TROOP_ATTACKED_FORCE:			strcpy_s(pBuf, pBuf_len,ConvertToString(CON_TROOP_ATTACKED_FORCE));			return;
	case CON_TROOP_ATTACKED_STR:			strcpy_s(pBuf, pBuf_len,ConvertToString(CON_TROOP_ATTACKED_STR));			return;
	case CON_ITEMBOX_BROKEN:				strcpy_s(pBuf, pBuf_len,ConvertToString(CON_ITEMBOX_BROKEN));				return;
	case CON_PROP_HP:						strcpy_s(pBuf, pBuf_len,ConvertToString(CON_PROP_HP));						return;
	case CON_IS_DEMO_SKIP:					strcpy_s(pBuf, pBuf_len,ConvertToString(CON_IS_DEMO_SKIP));						return;
	case CON_TROOP_COMPARE_HP_EXCEPT_LEADER:	strcpy_s(pBuf, pBuf_len,ConvertToString(CON_TROOP_COMPARE_HP_EXCEPT_LEADER));						return;

	default: strcpy_s(pBuf, pBuf_len,"CONDITION_NONAME");return;
		
	}
	//BSTriggerAssert(FALSE,"not exist in condition list");
}

void GetActionCodeNameByIndex(int iIndex,char *pBuf, const size_t pBuf_len)
{
	 //aleksger - safe string replaced strcpy with strcpy_s
	switch(iIndex)
	{
	case ACT_AI_DISABLE:					strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_AI_DISABLE));return;
	case ACT_AI_ENABLE:						strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_AI_ENABLE));return;
	case ACT_AISET:							strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_AISET));return;
	case ACT_AISETPATH:						strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_AISETPATH));return;
	case ACT_BGM_FADE:						strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_BGM_FADE));return;
	case ACT_BGM_MUTE:						strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_BGM_MUTE));return;
	case ACT_BGM_PLAY:						strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_BGM_PLAY));return;
	case ACT_BGM_STOP:						strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_BGM_STOP));return;
	case ACT_BGM_VOLUME:					strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_BGM_VOLUME));return;
	case ACT_CAM_RESET:						strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_CAM_RESET));return;
	case ACT_CAM_SET:						strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_CAM_SET));return;
	case ACT_CAM_TARGETAREA:				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_CAM_TARGETAREA));return;
	case ACT_CAM_TARGETTROOP:				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_CAM_TARGETTROOP));return;
	case ACT_DELAY:							strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_DELAY));return;
	case ACT_DEMO_PLAY:						strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_DEMO_PLAY));return;
	case ACT_EFFECT_FADEIN:					strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_EFFECT_FADEIN));return;
	case ACT_EFFECT_FADEINCOLOR:			strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_EFFECT_FADEINCOLOR));return;
	case ACT_EFFECT_FADEOUT:				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_EFFECT_FADEOUT));return;
	case ACT_EFFECT_FADEOUT_COLOR:			strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_EFFECT_FADEOUT_COLOR));return;
	case ACT_EFFECT_FOG:					strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_EFFECT_FOG));return;
	case ACT_EFFECTFX_RANDOM_SET:			strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_EFFECTFX_RANDOM_SET));return;
	case ACT_EFFECTFX_SET:					strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_EFFECTFX_SET));return;
	case ACT_EFFECT_GLOW_OFF:				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_EFFECT_GLOW_OFF));return;
	case ACT_EFFECT_GLOW_ON:				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_EFFECT_GLOW_ON));return;
	case ACT_EFFECT_LENSFLARE_OFF:			strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_EFFECT_LENSFLARE_OFF));return;
	case ACT_EFFECT_LENSFLARE_ON:			strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_EFFECT_LENSFLARE_ON));return;
	case ACT_EFFECT_LIGHT:					strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_EFFECT_LIGHT));return;
	case ACT_EFFECT_MOTIONBLUR_OFF:			strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_EFFECT_MOTIONBLUR_OFF));return;
	case ACT_EFFECT_MOTIONBLUR_ON:			strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_EFFECT_MOTIONBLUR_ON));return;
	case ACT_EFFECT_RAIN_OFF:				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_EFFECT_RAIN_OFF));return;
	case ACT_EFFECT_RAIN_ON:				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_EFFECT_RAIN_ON));return;
	case ACT_EFFECT_SET_FIRE:				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_EFFECT_SET_FIRE));return;
	case ACT_EFFECT_SNOW_OFF:				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_EFFECT_SNOW_OFF));return;
	case ACT_EFFECT_SNOW_ON:				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_EFFECT_SNOW_ON));return;
	case ACT_GATE_CLOSE:					strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_GATE_CLOSE));return;
	case ACT_GATE_OPEN:						strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_GATE_OPEN));return;
	case ACT_HOOK_INPUT_OFF:				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_HOOK_INPUT_OFF));return;
	case ACT_HOOK_INPUT_ON:					strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_HOOK_INPUT_ON));return;
	case ACT_INTERFACE_OFF:					strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_INTERFACE_OFF));return;
	case ACT_INTERFACE_ON:					strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_INTERFACE_ON));return;
	case ACT_LETTER_BOX_OFF:				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_LETTER_BOX_OFF));return;
	case ACT_LETTER_BOX_ON:					strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_LETTER_BOX_ON));return;
	case ACT_MAP_OFF:						strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_MAP_OFF));return;
	case ACT_MAP_ON:						strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_MAP_ON));return;
	case ACT_MAP_SCALE:						strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_MAP_SCALE));return;
	case ACT_MESSAGE_REPORT:				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_MESSAGE_REPORT));return;
	case ACT_MESSAGE_SAY:					strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_MESSAGE_SAY));return;
	case ACT_MESSAGE_SHOW:					strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_MESSAGE_SHOW));return;
	case ACT_MESSAGE_SHOWXY:				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_MESSAGE_SHOWXY));return;
	case ACT_MISSION_COMPLETED:				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_MISSION_COMPLETED));return;
	case ACT_MISSION_FAILED:				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_MISSION_FAILED));return;
	case ACT_ORB_ADD:						strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_ORB_ADD));return;
	case ACT_POINT_AREA_OFF:				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_POINT_AREA_OFF));return;
	case ACT_POINT_AREA_ON:					strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_POINT_AREA_ON));return;
	case ACT_POINT_FRIEND_IN_AREA:			strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_POINT_FRIEND_IN_AREA));return;
	case ACT_POINT_HOSTILE_IN_AREA:			strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_POINT_HOSTILE_IN_AREA));return;
	case ACT_POINT_TROOP_OFF:				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_POINT_TROOP_OFF));return;
	case ACT_POINT_TROOP_ON:				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_POINT_TROOP_ON));return;
	case ACT_SOUND_SET:						strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SOUND_SET));return;
	case ACT_SOUND_UNSET:					strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SOUND_UNSET));return;
	case ACT_TIME_MARK:						strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TIME_MARK));return;
	case ACT_TRIGGER_ACTIVATE:				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TRIGGER_ACTIVATE));return;
	case ACT_TRIGGER_DEACTIVATE:			strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TRIGGER_DEACTIVATE));return;
	case ACT_TRIGGER_LOOP:					strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TRIGGER_LOOP));return;
	case ACT_TRIGGER_RESET:					strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TRIGGER_RESET));return;
	case ACT_TRIGGER_RESETALL:				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TRIGGER_RESETALL));return;
	case ACT_TRIGGER_RESETTHIS:				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TRIGGER_RESETTHIS));return;
	case ACT_TROOP_ANIMATION:				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_ANIMATION));return;
	case ACT_TROOP_ANNIHILATE:				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_ANNIHILATE));return;
	case ACT_TROOP_ATTACK:					strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_ATTACK));return;
	case ACT_TROOP_ATTACK_LEADER:			strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_ATTACK_LEADER));return;
	case ACT_TROOP_DISABLE:					strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_DISABLE));return;
	case ACT_TROOP_DISABLE_ALL:				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_DISABLE_ALL));return;
	case ACT_TROOP_DISABLE_INAREA:			strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_DISABLE_INAREA));return;
	case ACT_TROOP_ENABLE:					strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_ENABLE));return;
	case ACT_TROOP_ENABLE_IN_AREA:			strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_ENABLE_IN_AREA));return;
	case ACT_TROOP_ENEMY_DISABLE_IN_AREA:	strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_ENEMY_DISABLE_IN_AREA));return;
	case ACT_TROOP_ENEMY_ENABLEINAREA:		strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_ENEMY_ENABLEINAREA));return;
	case ACT_TROOP_FOLLOW:					strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_FOLLOW));return;
	case ACT_TROOP_FRIEND_DISABLE_IN_AREA:	strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_FRIEND_DISABLE_IN_AREA));return;
	case ACT_TROOP_FRIEND_ENABLE_IN_AREA:	strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_FRIEND_ENABLE_IN_AREA));return;
	case ACT_TROOP_HP_FILL:					strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_HP_FILL));return;
	case ACT_TROOP_HP_MAX:					strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_HP_MAX));return;
	case ACT_TROOP_INVULNERABLE:			strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_INVULNERABLE));return;
	case ACT_TROOP_LEADER_INVULNERABLE:		strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_LEADER_INVULNERABLE));return;
	case ACT_TROOP_LEADER_VULNERABLE:		strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_LEADER_VULNERABLE));return;
	case ACT_TROOP_PLACE:					strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_PLACE));return;
	case ACT_TROOP_RANGE_ATTACK_AREA:		strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_RANGE_ATTACK_AREA));return;
	case ACT_TROOP_RANGE_ATTACK_WALL:		strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_RANGE_ATTACK_WALL));return;
	case ACT_TROOP_RE_NEW:					strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_RE_NEW));return;
	case ACT_TROOP_RE_NEW_OUTOFSIGHT:		strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_RE_NEW_OUTOFSIGHT));return;
	case ACT_TROOP_RUN:						strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_RUN));return;
	case ACT_TROOP_STOP:					strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_STOP));return;
	case ACT_TROOP_VULNERABLE:				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_VULNERABLE));return;
	case ACT_TROOP_WALK:					strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_WALK));return;
	case ACT_VAR_INCREASE:					strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_VAR_INCREASE));return;
	case ACT_VAR_SET:						strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_VAR_SET));return;
	case ACT_WALL_COLLAPSE:					strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_WALL_COLLAPSE));return;
	case ACT_WALL_SET_HP:					strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_WALL_SET_HP));return;
	case ACT_PROP_PUSH:						strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_PROP_PUSH));return;
	case ACT_DESTROY_BRIDGE:				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_DESTROY_BRIDGE));return;
	case ACT_SET_GUARDIAN_TROOP:			strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SET_GUARDIAN_TROOP));return;
	case ACT_SET_ENEMY_ATTR:				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SET_ENEMY_ATTR));return;
	case ACT_RESET_ENEMY_ATTR:				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_RESET_ENEMY_ATTR));return;
	case ACT_ENABLE_DYNAMIC_PROP:			strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_ENABLE_DYNAMIC_PROP));return;
	case ACT_DISABLE_DYNAMIC_PROP:			strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_DISABLE_DYNAMIC_PROP));return;
	case ACT_PLAY_REALTIME_MOVIE:			strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_PLAY_REALTIME_MOVIE));return;	
	case ACT_TROOP_HP_GAUGE_SHOW:			strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_HP_GAUGE_SHOW));return;	
	case ACT_TROOPTYPE_AI_DISABLE_IN_AREA:	strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOPTYPE_AI_DISABLE_IN_AREA));return;
	case ACT_TROOPTYPE_AI_ENABLE_IN_AREA:   strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOPTYPE_AI_ENABLE_IN_AREA));return;
	case ACT_TROOPTYPE_AISET_IN_AREA:       strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOPTYPE_AISET_IN_AREA ));return;
	case ACT_TROOPTYPE_ANIMATION_IN_AREA:   strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOPTYPE_ANIMATION_IN_AREA ));return;
	case ACT_TROOPTYPE_KILL_ALL_IN_AREA:    strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOPTYPE_KILL_ALL_IN_AREA ));return;
	case ACT_TROOPTYPE_DISABLE_IN_AREA:     strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOPTYPE_DISABLE_IN_AREA ));return;
	case ACT_TROOPTYPE_HP_FILL_IN_AREA:     strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOPTYPE_HP_FILL_IN_AREA ));return;
	case ACT_TROOPTYPE_HP_MAX_IN_AREA:      strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOPTYPE_HP_MAX_IN_AREA ));return;
	case ACT_TROOPTYPE_WALK:				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOPTYPE_WALK ));return;
	case ACT_TROOPTYPE_RUN:					strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOPTYPE_RUN ));return;
	case ACT_TROOPTYPE_STOP_IN_AREA:		strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOPTYPE_STOP_IN_AREA ));return;
	case ACT_TROOP_RANGE_ATTACK_TO_AREA:    strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_RANGE_ATTACK_TO_AREA ));return;
	case ACT_TROOPTYPE_SET_DIRECTION:		strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOPTYPE_SET_DIRECTION ));return;
	case ACT_TROOPTYPE_SHOW_IN_MINIMAP:		strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOPTYPE_SHOW_IN_MINIMAP ));return;
	case ACT_TROOPTYPE_HIDE_IN_MINIMAP:   strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOPTYPE_HIDE_IN_MINIMAP ));return;
	case ACT_TROOP_SHOW_IN_MINIMAP:			strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_SHOW_IN_MINIMAP ));return;
	case ACT_TROOP_HIDE_IN_MINIMAP:			strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_HIDE_IN_MINIMAP ));return;
	case ACT_UNIT_SHOW_IN_MINIMAP_ON_PROP:  strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_UNIT_SHOW_IN_MINIMAP_ON_PROP ));return;
	case ACT_UNIT_HIDE_IN_MINIMAP_ON_PROP:  strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_UNIT_HIDE_IN_MINIMAP_ON_PROP ));return;
	case ACT_PLAYER_ADD_ORB:				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_PLAYER_ADD_ORB));return;
	case ACT_FORCE_SHOW_HP_GAUGE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_FORCE_SHOW_HP_GAUGE));return;
	case ACT_FORCE_SHOW_HP_GAUGE_IN_AREA: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_FORCE_SHOW_HP_GAUGE_IN_AREA));return;
	case ACT_TROOPS_AI_ENABLE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOPS_AI_ENABLE));return;
	case ACT_TROOPS_AI_DISABLE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOPS_AI_DISABLE));return;
	case ACT_TROOPS_SET_AI: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOPS_SET_AI));return;
	case ACT_TROOPS_PLAY_ACTION: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOPS_PLAY_ACTION));return;
	case ACT_TROOPS_ANNIHILATE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOPS_ANNIHILATE));return;
	case ACT_TROOPS_DISABLE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOPS_DISABLE));return;
	case ACT_TROOPS_HP_FILL: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOPS_HP_FILL));return;
	case ACT_TROOPS_HP_MAX: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOPS_HP_MAX));return;
	case ACT_TROOPS_INVULNERABLE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOPS_INVULNERABLE));return;
	case ACT_TROOPS_VULNERABLE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOPS_VULNERABLE));return;
	case ACT_TROOPS_WALK: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOPS_WALK));return;
	case ACT_TROOPS_RUN: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOPS_RUN));return;
	case ACT_TROOPS_STOP: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOPS_STOP));return;
	case ACT_TROOPS_RANGE_ATTACK_AREA: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOPS_RANGE_ATTACK_AREA));return;
	case ACT_TROOPS_SET_DIR: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOPS_SET_DIR));return;
	case ACT_TROOPS_SHOW_MINIMAP: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOPS_SHOW_MINIMAP));return;
	case ACT_TROOPS_SHOW_HP_GAUGE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOPS_SHOW_HP_GAUGE));return;
	case ACT_TROOP_CHANGE_GROUP: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_CHANGE_GROUP));return;
	case ACT_DYNAMIC_FOG_ENABLE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_DYNAMIC_FOG_ENABLE));return;
	case ACT_DYNAMIC_FOG_CROSS: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_DYNAMIC_FOG_CROSS));return;
	case ACT_SET_DOF: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SET_DOF));return;
	case ACT_DISABLE_DOF: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_DISABLE_DOF));return;
	case ACT_DESTROY_CRUMBLE_STONE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_DESTROY_CRUMBLE_STONE));return;
	case ACT_DISABLE_DYNAMIC_FOG: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_DISABLE_DYNAMIC_FOG));return;
	case ACT_SET_PROP_AI_GUIDE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SET_PROP_AI_GUIDE));return;
	case ACT_SET_AREA_AI_GUIDE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SET_AREA_AI_GUIDE));return;
	case ACT_SET_GUARDIAN_AI: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SET_GUARDIAN_AI));return;
	case ACT_MOVE_MOVETOWER: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_MOVE_MOVETOWER));return;
	case ACT_ADD_OBJECTIVE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_ADD_OBJECTIVE));return;
	case ACT_REMOVE_OBJECTIVE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_REMOVE_OBJECTIVE));return;
	case ACT_CLEAR_OBJECTIVE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_CLEAR_OBJECTIVE));return;
	case ACT_DISPLAY_VAR: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_DISPLAY_VAR));return;
	case ACT_HIDE_VAR: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_HIDE_VAR));return;
	case ACT_DISPLAY_VAR_GAUGE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_DISPLAY_VAR_GAUGE));return;
	case ACT_HIDE_VAR_GAUGE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_HIDE_VAR_GAUGE));return;
	case ACT_RELEASE_GUARDIAN: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_RELEASE_GUARDIAN));return; 
	case ACT_SET_VAR_LIVE_TROOP_UNIT_COUNT: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SET_VAR_LIVE_TROOP_UNIT_COUNT));return;
	case ACT_SET_VAR_DEAD_TROOP_UNIT_COUNT: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SET_VAR_DEAD_TROOP_UNIT_COUNT));return;
	case ACT_SET_VAR_LIVE_FORCE_UNIT_COUNT: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SET_VAR_LIVE_FORCE_UNIT_COUNT));return;
	case ACT_SET_VAR_DEAD_FORCE_UNIT_COUNT: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SET_VAR_DEAD_FORCE_UNIT_COUNT));return;
	case ACT_PLAY_AMBI_SOUND_ON: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_PLAY_AMBI_SOUND_ON));return;
	case ACT_PLAY_AMBI_SOUND_OFF: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_PLAY_AMBI_SOUND_OFF));return;
	case ACT_SET_NEXT_STAGE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SET_NEXT_STAGE));return;
	case ACT_LINK_ELF_N_DRAGON: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_LINK_ELF_N_DRAGON));return;
	case ACT_SHOW_BOSS_GAUGE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SHOW_BOSS_GAUGE));return;
	case ACT_HIDE_BOSS_GAUGE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_HIDE_BOSS_GAUGE));return;
	case ACT_ADD_TROOP_LIVE_RATE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_ADD_TROOP_LIVE_RATE));return;
	case ACT_SET_DYNAMIC_PROP_GROUP_IN_AREA: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SET_DYNAMIC_PROP_GROUP_IN_AREA));return;
	case ACT_DROP_ITEM_AT_TROOP_POS: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_DROP_ITEM_AT_TROOP_POS));return;
	case ACT_DROP_ITEM_AT_LEADER_POS: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_DROP_ITEM_AT_LEADER_POS));return;
	case ACT_DROP_ITEM_IN_AREA: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_DROP_ITEM_IN_AREA));return;
	case ACT_SET_REALTIME_MOVIE_AFTER_MISSION_END: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SET_REALTIME_MOVIE_AFTER_MISSION_END));return;
	case ACT_SET_STOP_TRIGGER: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SET_STOP_TRIGGER));return;
		
	case ACT_TROOPS_ENABLE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOPS_ENABLE));return;
	case ACT_CHANGE_TEAM: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_CHANGE_TEAM));return;
	case ACT_GUARDIAN_WALK: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_GUARDIAN_WALK));return;
	case ACT_GUARDIAN_RUN: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_GUARDIAN_RUN));return;
	case ACT_GUARDIAN_STOP: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_GUARDIAN_STOP));return;
	case ACT_GUARDIAN_SET_DIR: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_GUARDIAN_SET_DIR));return;
	case ACT_GUARDIAN_RETREAT: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_GUARDIAN_RETREAT));return;
	case ACT_GUARDIAN_AI_SET: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_GUARDIAN_AI_SET));return;
	case ACT_TROOP_PICKOUT_ENABLE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_PICKOUT_ENABLE));return;


	case ACT_GUARDIAN_INC_POWER: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_GUARDIAN_INC_POWER));return;
	case ACT_GUARDIAN_INC_RANGE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_GUARDIAN_INC_RANGE));return;
	case ACT_TROOP_INC_POWER: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_INC_POWER));return;
	case ACT_TROOP_INC_RANGE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_INC_RANGE));return;
	case ACT_ENABLE_BLIZZARD_EFFECT: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_ENABLE_BLIZZARD_EFFECT));return;
	case ACT_DISABLE_BLIZZARD_EFFECT: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_DISABLE_BLIZZARD_EFFECT));return;
	case ACT_SHOW_MISSION_SELECT_MENU: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SHOW_MISSION_SELECT_MENU));return;
	case ACT_SET_PLAYER_LEVEL_EXP: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SET_PLAYER_LEVEL_EXP));return;
	case ACT_START_REALTIME_MOVIE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_START_REALTIME_MOVIE));return;
	case ACT_CHANGE_LIGHT: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_CHANGE_LIGHT));return;
	case ACT_CHANGE_FOG: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_CHANGE_FOG));return;
	case ACT_CHANGE_SCENE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_CHANGE_SCENE));return;
	case ACT_VAR_CLEARTIME: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_VAR_CLEARTIME));return;
	case ACT_VAR_KILLS: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_VAR_KILLS));return;
	case ACT_VAR_NORMALKILLS: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_VAR_NORMALKILLS));return;
	case ACT_VAR_ORBKILLS: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_VAR_ORBKILLS));return;
	case ACT_VAR_PHYSICSKILLS: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_VAR_PHYSICSKILLS));return;
	case ACT_VAR_HIGHESTCOMBO: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_VAR_HIGHESTCOMBO));return;
	case ACT_VAR_SUM: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_VAR_SUM));return;
	case ACT_VAR_SUB: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_VAR_SUB));return;
	case ACT_VAR_MUL: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_VAR_MUL));return;
	case ACT_VAR_DIV: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_VAR_DIV));return;
	case ACT_VAR_MOD: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_VAR_MOD));return;
	case ACT_TROOPS_SET_ATTACKABLE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOPS_SET_ATTACKABLE));return;
	case ACT_TRUE_ORB_ADD: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TRUE_ORB_ADD));return;
	case ACT_TROOP_SET_UNTOUCHABLE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_SET_UNTOUCHABLE));return;
	case ACT_VAT_CLEARTIME_SCORE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_VAT_CLEARTIME_SCORE));return;
	case ACT_VAR_KILLS_SCORE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_VAR_KILLS_SCORE));return;
	case ACT_VAR_ORBKILLS_SCORE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_VAR_ORBKILLS_SCORE));return;
	case ACT_VAR_HIGHESTCOMBO_SCORE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_VAR_HIGHESTCOMBO_SCORE));return;
	case ACT_VAR_SURVIVALRATE_SCORE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_VAR_SURVIVALRATE_SCORE));return;
	case ACT_VAR_ITEM_SCORE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_VAR_ITEM_SCORE));return;
	case ACT_SET_ADJUTANT_TYPE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SET_ADJUTANT_TYPE));return;
	case ACT_ADD_TROOP_EVENT_AREA: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_ADD_TROOP_EVENT_AREA));return;
	case ACT_SET_TROOP_EVENT_AREA: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SET_TROOP_EVENT_AREA));return;
	case ACT_FORCE_INVULNERABLE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_FORCE_INVULNERABLE));return;
	case ACT_FORCE_VULNERABLE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_FORCE_VULNERABLE));return;
	case ACT_SET_RESULTRANK: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SET_RESULTRANK));return;
	case ACT_SET_RANKPRIZE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SET_RANKPRIZE));return;
	case ACT_SET_RESULTBONUS: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SET_RESULTBONUS));return;
	case ACT_SET_RESULTRANKTABLE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SET_RESULTRANKTABLE));return;
	case ACT_CATAPULT_ATTACK_WALL: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_CATAPULT_ATTACK_WALL));return;
	case ACT_SHOW_WALL_HP_GUAGE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SHOW_WALL_HP_GUAGE));return;
	case ACT_SET_TRUEORBENABLE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SET_TRUEORBENABLE));return;
	case ACT_SHOW_TROOP_GAUGE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SHOW_TROOP_GAUGE));return;
	case ACT_HIDE_TROOP_GAUGE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_HIDE_TROOP_GAUGE));return;

	case ACT_VAR_TOTALUNITS: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_VAR_TOTALUNITS));return;
	case ACT_VAR_ORBSPARKON: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_VAR_ORBSPARKON));return;
	case ACT_PROP_REGEN: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_PROP_REGEN));return;
	case ACT_TROOP_INC_POWER_SOX: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_INC_POWER_SOX));return;
	case ACT_TROOP_INC_RANGE_SOX: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_INC_RANGE_SOX));return;
	case ACT_TROOP_INC_POWER_SOX_ID: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_INC_POWER_SOX_ID));return;
	case ACT_TROOP_INC_RANGE_SOX_ID: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_INC_RANGE_SOX_ID));return;
	case ACT_VAR_TOTALUNITS_TEAM: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_VAR_TOTALUNITS_TEAM));return;
	case ACT_VAR_TROOPHP: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_VAR_TROOPHP));return;
	case ACT_DARK_DYNAMIC_FOG_ENABLE: strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_DARK_DYNAMIC_FOG_ENABLE));return;
	case ACt_SET_TROOP_FX: strcpy_s(pBuf, pBuf_len,ConvertToString(ACt_SET_TROOP_FX));return;

	case ACT_CHANGE_SKYBOX:				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_CHANGE_SKYBOX));return;
	case ACT_SET_RANKPRIZE_S:			strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SET_RANKPRIZE_S));return;
	case ACT_SET_RANKPRIZE_A:			strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SET_RANKPRIZE_A));return;

	case ACT_GUARDIAN_PLACE:			strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_GUARDIAN_PLACE));return;

	case ACT_SET_ITEMINDEX:				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SET_ITEMINDEX));return;
	case ACT_VAR_GURADIAN_KILLCOUNT:	strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_VAR_GURADIAN_KILLCOUNT));return;
	case ACT_VAR_GURADIAN_ORB:			strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_VAR_GURADIAN_ORB));return;
	case ACT_HIDE_WALL_HP_GUAGE:		strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_HIDE_WALL_HP_GUAGE));return;

	case ACT_DARK_DYNAMIC_FOG_ENABLE_EX:	strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_DARK_DYNAMIC_FOG_ENABLE_EX));return;
	case ACT_DARK_DYNAMIC_FOG_DISABLE_EX:	strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_DARK_DYNAMIC_FOG_DISABLE_EX));return;

	case ACT_SET_ITEMBOX_ENABLE:		strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SET_ITEMBOX_ENABLE));return;
	case ACT_SET_ITEMBOX_DISABLE:		strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SET_ITEMBOX_DISABLE));return;

	case ACT_EFFECT_RAIN_ENABLE:		strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_EFFECT_RAIN_ENABLE));return;
	case ACT_EFFECT_RAIN_DISABLE:		strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_EFFECT_RAIN_DISABLE));return;
	case ACT_SKIP_TEXT:					strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SKIP_TEXT));return;
	case ACT_SKIP_REALMOVIE:			strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SKIP_REALMOVIE));return;
	case ACT_DELAY_ON_REALMOVIE:		strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_DELAY_ON_REALMOVIE));return;
	case ACT_SET_WEAPON_FROM_TROOP:		strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SET_WEAPON_FROM_TROOP));return;

	case ACT_SET_TROOP_AI_DISABLE_IN_AREA_FOR_RM:	strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SET_TROOP_AI_DISABLE_IN_AREA_FOR_RM));return;

	case ACT_VAR_SCORE_TROOP_HP:		strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_VAR_SCORE_TROOP_HP));return;
	case ACT_VAR_SCORE_TROOP_TEAM_HP:	strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_VAR_SCORE_TROOP_TEAM_HP));return;
	case ACT_VAR_SCORE_TROOP_NUM:		strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_VAR_SCORE_TROOP_NUM));return;
	case ACT_VAR_SCORE_RANGE:			strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_VAR_SCORE_RANGE));return;
	case ACT_VAR_SCORE_BROKEN_PROP:		strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_VAR_SCORE_BROKEN_PROP));return;

	case ACT_SET_PROP_CRUSH:			strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SET_PROP_CRUSH));return;

	case ACT_SET_CORPSE_DELAY:			strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SET_CORPSE_DELAY));return;

	case ACT_PLAY_SOUND_EFFECT:						strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_PLAY_SOUND_EFFECT));return;
	case ACT_ADD_MAXHP:								strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_ADD_MAXHP));return;
	case ACT_SHOW_TROOP_HP_GAUGE_EXCEPT_LEADER:		strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SHOW_TROOP_HP_GAUGE_EXCEPT_LEADER));return;
	case ACT_SET_DEFENSE_REF:						strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SET_DEFENSE_REF));return;
	case ACT_SHOW_CIRCLE_AREA_IN_MINIMAP:			strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SHOW_CIRCLE_AREA_IN_MINIMAP));return;
	case ACT_HIDE_CIRCLE_AREA_IN_MINIMAP:			strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_HIDE_CIRCLE_AREA_IN_MINIMAP));return;
	case ACT_TROOP_ANIMATION_WITH_DELAY:			strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_ANIMATION_WITH_DELAY));return;
	case ACT_TROOP_DAMAGED_ONLY_PLAYER:				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_DAMAGED_ONLY_PLAYER));return;

	case ACT_ITEM_PROBBT_ADD:					strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_ITEM_PROBBT_ADD));return;
	case ACT_VAR_SURVIVALRATE_SCORE_VAR_ARG:	strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_VAR_SURVIVALRATE_SCORE_VAR_ARG));return;
	case ACT_VAR_KILLS_SCORE_VAR_ARG:			strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_VAR_KILLS_SCORE_VAR_ARG));return;

	case ACT_ENABLE_EVENT_TROOP_AI:				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_ENABLE_EVENT_TROOP_AI));return;
	case ACT_DISABLE_EVENT_TROOP_AI:			strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_DISABLE_EVENT_TROOP_AI));return;

	case ACT_SET_UNIT_AI_ENABLE:				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SET_UNIT_AI_ENABLE));return;
	case ACT_SET_UNIT_AI_DISABLE:				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_SET_UNIT_AI_DISABLE));return;

	case ACT_TROOP_LAST_DISABLE:					strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOP_LAST_DISABLE));return;
	case ACT_TROOPS_LAST_DISABLE:					strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_TROOPS_LAST_DISABLE));return;

	case ACT_VAR_MISSION_LEVEL:						strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_VAR_MISSION_LEVEL));return;

	case ACT_VIG_MOTIONBLUR_ENABLE:					strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_VIG_MOTIONBLUR_ENABLE));return;
	case ACT_VIG_MOTIONBLUR_DISABLE: 				strcpy_s(pBuf, pBuf_len,ConvertToString(ACT_VIG_MOTIONBLUR_DISABLE));return;
	
	default: strcpy_s(pBuf, pBuf_len,"ACTION_NONAME");return;
	}
	//BSTriggerAssert(FALSE,"code is not exist in action list");

}

int GetConditionCodeIndexByName(const char *szCodeName)
{
	char szCodeNameTmp[MAX_BS_CODE_NAME];
	for(int i = 0;i < CON_LAST;i++)
	{
		GetConditionCodeNameByIndex(i,szCodeNameTmp, _countof(szCodeNameTmp));  //aleksger - safe string
		if(strcmp(szCodeName,szCodeNameTmp) == 0){
			return i;
		}
	}
	BSTriggerAssert(FALSE,"couldn't find parameter");
	return -1;
}


int GetActionCodeIndexByName(const char *szCodeName)
{
	char szCodeNameTmp[MAX_BS_CODE_NAME];
	for(int i = 0;i < ACT_LAST;i++)
	{
		GetActionCodeNameByIndex(i,szCodeNameTmp, _countof(szCodeNameTmp));  //aleksger - safe string
		if(strcmp(szCodeName,szCodeNameTmp) == 0){
			return i;
		}
	}

	BSTriggerAssert(FALSE,"Couldn't find action code name in list");
	return -1;
}

//----------------------------------------------------------------------------------------------------
//Util
bool GetLine(FILE *fp,char *Buffer,int Size)
{
	memset(Buffer,0,sizeof(char)*Size);
	int nCount = 0;
	while(1)
	{
		char ch = fgetc(fp);
		if(ch == ';'){return true;}
		if(ch == EOF){return false;}

		if(ch != '\n')
		{
			Buffer[nCount] = ch;
			nCount++;		
			if(nCount > Size-1)	{
				BSTriggerAssert(FALSE,"line is too long");
			}
		}
	}
	return true;
}

void GetElementByCharacter(char *szOut, const size_t szOut_len, char *szOrg,int ElementNum,const int OutstrMax,char Character)
{
	char *szTmp = new char[1024];
	memset(szTmp,0,sizeof(char)*1024);

	int SignalCount = 0;
	int StrCount = 0;
	for(int i = 0;i < (int)strlen(szOrg);i++)
	{
		szTmp[StrCount] = szOrg[i];
		StrCount++;
		if(szOrg[i] == Character || i == strlen(szOrg)-1)
		{
			if(i == strlen(szOrg)-1)
			{
				strcpy_s(szOut, szOut_len, szTmp); //aleksger - safe string
				break;
			}
			else if(SignalCount == ElementNum)
			{
				szTmp[StrCount-1] = 0;
				strcpy_s(szOut, szOut_len, szTmp); //aleksger - safe string
				break;
			}
			else
			{
				StrCount = 0;
				memset(szTmp,0,sizeof(char)*OutstrMax);
			}
			SignalCount++;
		}	

	}
	delete [] szTmp; //aleksger: prefix bug 815: Variable is a dynamically allocated array.
}

void MakeCompleteSentence(char *szBuf,char *szOrg,int Sequence,char *szParam)
{
	int nBufCount = 0;
	int nSignalCount = 0;
	for(int i = 0;i < (int)strlen(szOrg);i++)
	{
		szBuf[nBufCount] = szOrg[i];
		if(szBuf[nBufCount] == '[')
		{
			if(nSignalCount == Sequence)
			{
				nBufCount++;
				if(szParam)
				{
					for(int si = 0;si < (int)strlen(szParam);si++)
					{
						szBuf[nBufCount] = szParam[si];
						nBufCount++;
					}
				}
				while(szOrg[i] != ']')
				{
					i++;
				}
				szBuf[nBufCount] = szOrg[i];
			}
			nSignalCount++;
		}
		nBufCount++;
	}
}


//----------------------------------------------------------------------------------------------------
CBSTriggerManager::CBSTriggerManager()
{
#ifdef _USAGE_TOOL_
	ReadParamInfoList();
	ReadParamList();
	CreateEmptyEvents();
#endif

	//비교문 선택시
	/*strcpy(m_cstOptStr[0],"equal to ( = )");
	strcpy(m_cstOptStr[1],"greater than ( > )");
	strcpy(m_cstOptStr[2],"less than ( < )");
	strcpy(m_cstOptStr[3],"greater than or equal to ( >= )");
	strcpy(m_cstOptStr[4],"less than or equal to ( <= )");*/

	//데이터
	/*strcpy(m_cstDataTypeStr[0],"int");
	strcpy(m_cstDataTypeStr[1],"bool");
	strcpy(m_cstDataTypeStr[2],"random");
	strcpy(m_cstDataTypeStr[3],"string");*/


	m_pBackupCondition = new CBSConditionForTool;
	m_pBackupAction    = new CBSActionForTool;

}

void CBSTriggerManager::DeleteEvents()
{
	for(int i = 0;i < (int)m_EventList.size();i++){
		delete m_EventList[i];
	}
	m_EventList.clear();
}

void CBSTriggerManager::CreateEmptyEvents()
{	
	int iEventNum = sizeof(g_EventCodeTable) / sizeof(BSEventCodeInfo);
	for(int i = 0;i < iEventNum;i++)
	{
		CBSEventForTool *pEvent = new CBSEventForTool;
		pEvent->SetHandleIndex(g_EventCodeTable[i].iHandle);
		m_EventList.push_back(pEvent);
	}
}


void CBSTriggerManager::Clear()
{
	int i = 0;
	for(i = 0;i < (int)m_TriggerList.size();i++){
		delete m_TriggerList[i];}
	m_TriggerList.clear();

	DeleteEvents();

	for(i = 0;i < (int)m_ValList.size();i++){
		delete m_ValList[i];}
	m_ValList.clear();
	DeleteEvents();

}

CBSTriggerManager::~CBSTriggerManager()
{
	Destroy();
}

void CBSTriggerManager::Destroy()
{
	int i = 0;
	for(i = 0;i < (int)m_TriggerList.size();i++){
		delete m_TriggerList[i];}
	m_TriggerList.clear();

	DeleteEvents();

	for(i = 0;i < (int)m_ValList.size();i++){
		delete m_ValList[i];}
	m_ValList.clear();

	for(i = 0;i < (int)m_SelectableConditionList.size();i++){
		delete m_SelectableConditionList[i];}
	m_SelectableConditionList.clear();

	for(i = 0;i < (int)m_SelectableActionList.size();i++){
		delete m_SelectableActionList[i];}
	m_SelectableActionList.clear();

	for(i = 0;i < (int)m_ParamInfoList.size();i++){
		delete m_ParamInfoList[i];}
	m_ParamInfoList.clear();

	if(m_pBackupCondition)
	{
		delete m_pBackupCondition;
		m_pBackupCondition = NULL;
	}
	if(m_pBackupAction)
	{
		delete m_pBackupAction;
		m_pBackupAction = NULL;
	}
	
}

void CBSTriggerManager::SetParamProperty(char *szParamName,CBSTriggerParam *pParam)
{
	for(int i = 0;i < (int)m_ParamInfoList.size();i++)
	{
		if(strcmp(m_ParamInfoList[i]->GetName(),szParamName) == 0)
		{
			pParam->SetDataType(m_ParamInfoList[i]->GetDataType());
			pParam->SetCtrlType(m_ParamInfoList[i]->GetCtrlType());
			pParam->SetCodeIndex(i);
			return;
		}
	}
	char szErr[256];
	sprintf(szErr,"(%s) This Parameter didn't register.",szParamName);
	BSTriggerAssert(FALSE,szErr);
}

char *CBSTriggerManager::GetParamCodeName(int iCodeIndex)
{
	return m_ParamInfoList[iCodeIndex]->GetName();
}

void CBSTriggerManager::SetSeparateParameter(char *szSentence,CBSConditionActionForToolBase *pDest) //파라메터 식별하콅E
{
	bool bInput = false;
	char szTmp[256];
	memset(szTmp,0,sizeof(char)*256);
	int iStrTmpCount = 0;

	for(int i = 0;i < (int)strlen(szSentence);i++)
	{
		char ch = szSentence[i];
		if(ch == '[')
		{
			if(i+1 < (int)strlen(szSentence))
			{
				if(szSentence[i+1] > 0)
				{
					bInput = true;
				}
			}			
		}
		else if(ch == ']')
		{
			
			CBSTriggerParam *pData = new CBSTriggerParam;
			SetParamProperty(szTmp,pData);
			if(pData->GetCodeIndex() == -1)
			{
				delete pData;
				continue;
			}

			pDest->AddParameterData(pData);
			delete pData;
			pData = NULL;
			bInput = false;
			iStrTmpCount = 0;
			memset(szTmp,0,sizeof(char)*256);
		}
		else if(bInput)
		{
			szTmp[iStrTmpCount] = szSentence[i];
			iStrTmpCount++;
		}
	}
}

#ifdef _USAGE_TOOL_

void CBSTriggerManager::ReadParamInfoList()
{
	char szPath[MAX_PATH];
	::GetCurrentDirectory(MAX_PATH,szPath);
	// AfxMessageBox(szPath);
	strcat(szPath,"\\data\\BSTriggerParameter.txt");

	Parser parser;
	TokenList toklist;

	parser.Create();
	parser.EnableComments( true );

	bool result;
	result = parser.ProcessSource( "", szPath, &toklist );
	if( result == false ){
		return;
	}

	result = parser.ProcessHeaders( &toklist );
	if( result == false ){
		BsAssert( 0 );
		return;
	}
	parser.ProcessMacros( &toklist );
	TokenList::iterator itr = toklist.begin();

	CBSTriggerParamEx *pParam = NULL;
	while( 1 )
	{
		if( itr == toklist.end() )
		{
			break;
		}
		if(strcmp(itr->GetVariable(),"PARAMETER_NAME") == 0)
		{
			itr++;
			pParam = new CBSTriggerParamEx;
			pParam->SetName(itr->GetVariable());
			itr++;
		}
		if(strcmp(itr->GetVariable(),"PARAMETER_CAPTION") == 0)
		{
			itr++;
			if(pParam){pParam->SetCaption(itr->GetString());}
			itr++;
		}
		if(strcmp(itr->GetVariable(),"PARAMETER_DAYA_TYPE") == 0)
		{
			itr++;
			if(strcmp(itr->GetVariable(),"BS_TG_TYPE_INT") == 0)	{
				if(pParam){pParam->SetDataType(BS_TG_TYPE_INT);}
			}
			else if(strcmp(itr->GetVariable(),"BS_TG_TYPE_FLOAT") == 0)	{
				if(pParam){pParam->SetDataType(BS_TG_TYPE_FLOAT);}
			}
			else if(strcmp(itr->GetVariable(),"BS_TG_TYPE_STRING") == 0)
			{
				if(pParam){pParam->SetDataType(BS_TG_TYPE_STRING);}
			}
			itr++;
		}
		if(strcmp(itr->GetVariable(),"PARAMETER_CTRL_TYPE") == 0)
		{
			itr++;
			if(strcmp(itr->GetVariable(),"BS_TG_CTRL_EDIT") == 0){
				if(pParam){pParam->SetCtrlType(BS_TG_CTRL_EDIT);}
			}
			else if(strcmp(itr->GetVariable(),"BS_TG_CTRL_COMBOBOX") == 0)
			{
				if(pParam){pParam->SetCtrlType(BS_TG_CTRL_COMBOBOX);}
			}
			itr++;
			m_ParamInfoList.push_back(pParam);
			pParam = NULL;
		}
	}
}

void CBSTriggerManager::ReadParamList()
{
	char szLocale[128];
	GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_IOPTIONALCALENDAR,szLocale,128);
	int iLocale = atoi(szLocale);
	char szPath[MAX_PATH];
	::GetCurrentDirectory(MAX_PATH,szPath);
	// AfxMessageBox(szPath);

	if(iLocale == 5) //한국
	{
		strcat(szPath,"\\data\\BSConditionActionList_KOR.txt");
	}
	else if(iLocale == 3) //일본
	{
		strcat(szPath,"\\data\\BSConditionActionList_JPN.txt");              
	}
	else{
		BSTriggerAssert(FALSE,"Programe cannot support language current OS.");
		return;
	}

	Parser parser;
	TokenList toklist;

	parser.Create();
	parser.EnableComments( true );

	bool result;
	result = parser.ProcessSource( "", szPath, &toklist );
	if( result == false ){
		return;
	}

	result = parser.ProcessHeaders( &toklist );
	if( result == false ){
		BSTriggerAssert(FALSE,"Parameter file not found");
		return;
	}
	parser.ProcessMacros( &toklist );
	TokenList::iterator itr = toklist.begin();
	
	bool bIsCondition = false;
	bool bIsAction = false;
	CBSConditionForTool *pCondition = NULL;
	CBSActionForTool	*pAction = NULL;
	int iCount					 = 0;
	while( 1 )
	{
		if( itr == toklist.end() )
		{
			break;
		}
		if(strcmp(itr->GetVariable(),"START_CONDITIONS") == 0){
			itr++;
			bIsCondition = true;
			bIsAction = false;
		}
		else if(strcmp(itr->GetVariable(),"START_ACTIONS") == 0)
		{
			itr++;
			bIsCondition = false;
			bIsAction = true;
		}
		if(itr->IsVariable() && bIsCondition)
		{			
			if(strcmp(itr->GetVariable(),"CAPTION") == 0)
			{
				itr++;
				m_SelectableConditionList[m_SelectableConditionList.size()-1]->SetCaption(itr->GetString());
				itr++;
			}
			else
			{
				pCondition = new CBSConditionForTool;
				//pCondition->SetCodeIndex(GetConditionCodeIndexByName(itr->GetVariable()));
				pCondition->SetCodeIndex(m_SelectableConditionList.size());
				itr++;

				pCondition->SetSentence(itr->GetString());		
				SetSeparateParameter(pCondition->GetSentence(),pCondition);
				pCondition->SetCaption(itr->GetString());
				itr++;
				pCondition->SetMyNumber((int)m_SelectableConditionList.size());
				m_SelectableConditionList.push_back(pCondition);
			}
		}
		else if(itr->IsVariable() && bIsAction)
		{
			if(strcmp(itr->GetVariable(),"CAPTION") == 0)
			{
				itr++;
				m_SelectableActionList[m_SelectableActionList.size()-1]->SetCaption(itr->GetString());
				itr++;
			}
			else
			{
				pAction = new CBSActionForTool;
				//pAction->SetCodeIndex(GetActionCodeIndexByName(itr->GetVariable()));
				pAction->SetCodeIndex((int)m_SelectableActionList.size());//외부에서 추가 가능하도록 굳이 아이디로 안한다.
				itr++;

				pAction->SetSentence(itr->GetString());
				SetSeparateParameter(pAction->GetSentence(),pAction);

				pAction->SetCaption(itr->GetString());
				itr++;
				pAction->SetMyNumber((int)m_SelectableActionList.size());
				m_SelectableActionList.push_back(pAction);
			}
		}
	}
}


bool CBSTriggerManager::FixDataName(char *pOldName,char *pNewName)
{
	bool bResult;
	unsigned int i = 0;
	for(i = 0;i < m_TriggerList.size();i++){
		bResult = m_TriggerList[i]->FixDataName(pOldName,pNewName);
	}
	for(i = 0;i < m_EventList.size();i++){
		bResult |= m_EventList[i]->FixDataName(pOldName,pNewName);
	}
	return bResult;
}

// Document -> TriggerManager 통지 메시지 처리
DWORD	CBSTriggerManager::DocCommand(UINT nMsg,DWORD dwParam1/*=0*/,DWORD dwParam2/*=0*/)
{
	DWORD dwRet = 0L;
	switch( nMsg )
	{
	case NOTIFY_AREANAME_CHANGED :
	case NOTIFY_TROOPNAME_CHANGED :
	case NOTIFY_PROPNAME_CHANGED :
	case NOTIFY_PATHNAME_CHANGED :
		// BsAssert 쓰지 말고 에러 발생시 dwRet = 0 값을 리턴.
		// Property에서 에러 처리 할 것임.
		char *pszOldName = (char *)dwParam1;
		char *pszNewName = (char *)dwParam2;

		if(nMsg == NOTIFY_TROOPNAME_CHANGED || nMsg == NOTIFY_AREANAME_CHANGED)
		{
			if(strlen(pszOldName) == 0){
				return 1;
			}
		}
		FixDataName(pszOldName, pszNewName);
		dwRet = 1;
		break;
	}

	return dwRet;
}

#endif


char *CBSTriggerManager::GetParamCaption(int iCodeIndex)
{
	return m_ParamInfoList[iCodeIndex]->GetCaption();
}




void CBSTriggerManager::AddEmptyTrigger(char *szTriggerName)
{
	CBSTriggerForTool *pTrigData = new CBSTriggerForTool;
	pTrigData->SetName(szTriggerName);
	m_TriggerList.push_back(pTrigData);
}

void CBSTriggerManager::DeleteTrigger(int iTriggerIndex)
{
	for(int i = 0;i < GetTriggerCount();i++)
	{
		std::vector<CBSTriggerParam		*> ParamList;
		int nCount = GetTrigger(i)->GetConditionCount();
		int s = 0;
		for( s = 0;s < nCount;s++ ){		
			GetTrigger(i)->GetCondition(s)->GetParamList( ParamList );
		}
		nCount = GetTrigger(i)->GetActionCount();
		for( s = 0;s < nCount;s++ ){		
			GetTrigger(i)->GetAction(s)->GetParamList( ParamList );
		}
		nCount = ParamList.size();
		for( s = 0;s < nCount;s++ )
		{
			if( _stricmp( GetParamCaption( ParamList[s]->GetCodeIndex() ),"TriggerID" ) == 0 ){
				if( ParamList[s]->GetInteger() > iTriggerIndex){				
					ParamList[s]->SetData( ParamList[s]->GetInteger() - 1 );
				}
				else if( ParamList[s]->GetInteger() == iTriggerIndex && iTriggerIndex != i){				
					ParamList[s]->SetData( -1 );

					char szErrLog[128];
					sprintf(szErrLog,"You deleted %d. so, we'll change %d trigger param set -1",iTriggerIndex,i-1);
					BSTriggerAssert(false,szErrLog );
				}
			}
		}
	}

	delete m_TriggerList[iTriggerIndex];
	m_TriggerList.erase(m_TriggerList.begin()+iTriggerIndex);
}



void CBSTriggerManager::AddEmptyAction(CBSTriggerEventBase *pBase,int SelectableIndex)
{
	if(SelectableIndex < (int)m_SelectableActionList.size())
	{
		pBase->AddAction(m_SelectableActionList[SelectableIndex]);
	}
}

void CBSTriggerManager::AddEmptyVariable()
{
	CBSTriggerParam Param;
	CBSVariable *pValTmp = new CBSVariable;	
	SetParamProperty(TPC_VARIABLE_NAME,&Param);
	pValTmp->AddParameter(&Param);
	SetParamProperty(TPC_VARIABLE_TYPE,&Param);
	pValTmp->AddParameter(&Param);
	SetParamProperty(TPC_VARIABLE_VAL,&Param);
	pValTmp->AddParameter(&Param);
	SetParamProperty(TPC_VARIABLE_DESC,&Param);
	Param.SetData("Desc : ");
	pValTmp->AddParameter(&Param);
	m_ValList.push_back(pValTmp);
}

bool CBSTriggerManager::IsInValCodeIndex(int iCodeIndex)
{
	if(iCodeIndex == 7) //VariableID 하드코딩
	{
		return true;
	}
	return false;
}

void CBSTriggerManager::InsertVar(int iIndex,CBSVariable *pVar,int nIndexOrg)
{
	CBSVariable *pTmpVar = new CBSVariable;
	memcpyVar(pTmpVar,pVar);
	m_ValList.insert( m_ValList.begin() + iIndex, pTmpVar );

	std::vector<CBSTriggerParam *>vtParamList;
	for(int i = 0;i < (int)m_TriggerList.size();i++)
	{
		GetParamList( i,vtParamList );
		for( int s = 0;s < (int)vtParamList.size();s++ )
		{
			if( IsInValCodeIndex( vtParamList[s]->GetCodeIndex() ) )
			{
				if( vtParamList[s]->GetInteger() == nIndexOrg ){				
					vtParamList[s]->SetData( iIndex  );
				}
				else if(vtParamList[s]->GetInteger() >= iIndex){
					vtParamList[s]->SetData( vtParamList[s]->GetInteger() + 1 );
				}
			}
		}
	}
}


void CBSTriggerManager::DeleteVariable(int iValIndex)
{	
	for(int i = 0;i < (int)m_TriggerList.size();i++)
	{
		std::vector<CBSTriggerParam *>vtParamList;
		GetParamList( i,vtParamList );
		for( int s = 0;s < (int)vtParamList.size();s++ )
		{
			if( IsInValCodeIndex( vtParamList[s]->GetCodeIndex() ) )
			{
				if( vtParamList[s]->GetInteger() == iValIndex )
				{
#ifdef _USAGE_TOOL_
					char szErrMsg[256];
					sprintf( szErrMsg,"Trigger %d %s // variable index set -1",i,GetTrigger(i)->GetName() );
					AfxMessageBox( szErrMsg );
#endif					
					vtParamList[s]->SetData(-1);	
				}
				else if( vtParamList[s]->GetInteger() > iValIndex ){
					vtParamList[s]->SetData( vtParamList[s]->GetInteger()-1 );
				}
			}			
		}
	}
	delete m_ValList[iValIndex];	
	m_ValList.erase( m_ValList.begin() + iValIndex );

}
bool CBSTriggerManager::Save(BFileStream *fp)
{
	int iVer = FC_TRIGGER_FILE_VER;
	fp->Write(&iVer,sizeof(int),ENDIAN_FOUR_BYTE);

	int iCount = (int)m_TriggerList.size();
	fp->Write(&iCount,sizeof(int),ENDIAN_FOUR_BYTE);

	int i = 0;
	for(i = 0;i < (int)m_TriggerList.size();i++)	{
		m_TriggerList[i]->SaveTrigger(fp,iVer);
	}

	iCount = (int)m_EventList.size();
	fp->Write(&iCount,sizeof(int),ENDIAN_FOUR_BYTE);
	for(i = 0;i < (int)m_EventList.size();i++)	{
		m_EventList[i]->SaveEvent(fp,iVer);
	}

	iCount = (int)m_ValList.size();
	fp->Write(&iCount,sizeof(int),ENDIAN_FOUR_BYTE);
	for(i = 0;i < (int)m_ValList.size();i++)	{
		m_ValList[i]->SaveVariable(fp,iVer);
	}
	return true;
}

bool CBSTriggerManager::Save(char *szFileName)
{
	BFileStream fp(szFileName,BFileStream::create);

	if(fp.Valid()){
		Save(&fp);
	}
	else{
		return false;
	}
	return true;
}

bool CBSTriggerManager::SaveToText(char *szFileName)
{
	FILE *fp = fopen(szFileName,"wt");
	if(fp == NULL)
		return false;


	fprintf(fp,"FANTASYCORE_TRIGGER_DATA\n");
	fprintf(fp,"FILE_VERSION %d\n\n",FC_TRIGGER_FILE_VER);

	fprintf(fp,"%s\t%d\n",BSKW_TRIGGER_COUNT,m_TriggerList.size());

	int i = 0;
	for(i = 0;i < (int)m_TriggerList.size();i++)	{
		m_TriggerList[i]->SaveTriggerToText(fp);
	}

	fprintf(fp,"\n%s\t%d\n",BSKW_EVENT_COUNT,m_EventList.size());

	for(i = 0;i < (int)m_EventList.size();i++)	{
		m_EventList[i]->SaveEventToText(fp);
	}

	fprintf(fp,"\n%s\t%d\n",BSKW_VARIABLE_COUNT,m_ValList.size());
	for(i = 0;i < (int)m_ValList.size();i++)	{
		m_ValList[i]->SaveVariableToText(fp);
	}
	fclose(fp);
	return true;
}

bool CBSTriggerManager::LoadToText(char *szFileName)
{
	Clear();
	Parser parser;
	TokenList toklist;

	parser.Create();
	parser.EnableComments( true );

	parser.ReserveKeyword(BSKW_TRIGGER_COUNT);
	parser.ReserveKeyword(BSKW_EVENT_COUNT);
	parser.ReserveKeyword(BSKW_VARIABLE_COUNT);
	parser.ReserveKeyword(BSKW_CONDITION_COUNT);
	parser.ReserveKeyword(BSKW_ACTION_COUNT);
	parser.ReserveKeyword(BSKW_TRIGGER_NAME);
	parser.ReserveKeyword(BSKW_EVENT_NAME);
	parser.ReserveKeyword(BSKW_CONDITION_COUNT);
	parser.ReserveKeyword(BSKW_ACTION_COUNT);
	parser.ReserveKeyword(BSKW_CONDITION_INFOMATION);
	parser.ReserveKeyword(BSKW_ACTION_INFOMATION);
	parser.ReserveKeyword(BSKW_CODE_INDEX);
	parser.ReserveKeyword(BSKW_PARAMETER_COUNT);
	parser.ReserveKeyword(BSKW_TOOL_NUMBER);
	parser.ReserveKeyword(BSKW_TOOL_ORINDEX);
	parser.ReserveKeyword(BSKW_DATA_TYPE);
	parser.ReserveKeyword(BSKW_CTRL_TYPE);
	parser.ReserveKeyword(BSKW_CODE_INDEX);
	parser.ReserveKeyword(BSKW_DATA);
	parser.ReserveKeyword(BSKW_EVENT_HANDLE);


	bool result;
	result = parser.ProcessSource( "", szFileName, &toklist );
	if( result == false )
	{
		return false;
	}

	result = parser.ProcessHeaders( &toklist );
	if( result == false )
	{
		BSTriggerAssert(FALSE,"Couldn't find text trigger");
		return false;
	}
	parser.ProcessMacros( &toklist );
	TokenList::iterator itr = toklist.begin();

	while( 1 )
	{
		if( itr == toklist.end() )
		{
			break;
		}
		itr++;
		itr++;
		int nVer = itr->GetInteger();
		itr++; //일단 버젼 체크는 안하고있음

		if( strcmp( itr->GetKeyword(), BSKW_TRIGGER_COUNT ) == 0 )
		{
			itr++;
			int iTriggerCount = itr->GetInteger();
			itr++;
			int i = 0;
			for(i = 0;i < iTriggerCount;i++)
			{
				CBSTriggerForTool *pTrigTmp = new CBSTriggerForTool;
				pTrigTmp->LoadTriggerToText(itr);

#ifdef _USAGE_TOOL_
				int si = 0;
				for(si = 0;si < pTrigTmp->GetConditionCount();si++){
					CopySentence(pTrigTmp->GetCondition(si));
				}
				for(si = 0;si < pTrigTmp->GetActionCount();si++){
					CopySentence(pTrigTmp->GetAction(si));
				}
#endif
				m_TriggerList.push_back(pTrigTmp);
			}
		}

		//이벤트는 Clear시에 자동생성 해 주므로 굳이 따로 생성안해도 됨
		if( strcmp( itr->GetKeyword(), BSKW_EVENT_COUNT ) == 0 )
		{
			itr++;
			int iEventCount = itr->GetInteger();
			itr++;
			for(int i = 0;i < iEventCount;i++)
			{
				m_EventList[i]->LoadEventToText(itr);
#ifdef _USAGE_TOOL_
				int si = 0;
				for(si = 0;si < m_EventList[i]->GetConditionCount();si++){
					CopySentence(m_EventList[i]->GetCondition(si));
				}
				for(si = 0;si < m_EventList[i]->GetActionCount();si++){
					CopySentence(m_EventList[i]->GetAction(si));
				}
#endif
			}
		}		

		if( strcmp( itr->GetKeyword(), BSKW_VARIABLE_COUNT ) == 0 )
		{
			itr++;
			int iValCount = itr->GetInteger();
			itr++;
			for(int i = 0;i < iValCount;i++)
			{
				CBSVariable *pVal = new CBSVariable;
				pVal->LoadVariableToText(itr);
				m_ValList.push_back(pVal);
			}
		}
	}


	char szFileNameTmp[MAX_PATH];
	strcpy(szFileNameTmp,szFileName);
	// aleksger - replacing the logic to find the last dot so that it can never go past beginning of the string.
	char * szFileNameDot = strrchr(szFileNameTmp,'.');
	if (szFileNameDot) 
	{
		*szFileNameDot = NULL;
		strcat(szFileNameTmp, ".bstrg");
		Save(szFileNameTmp);
	}
	else BSTriggerAssert(FALSE,"Could not find the end of filename to save to.");
	/*int iLength = strlen(szFileNameTmp);
	char ch = -1;
	while(ch != '.'){
		iLength--;
		ch = szFileNameTmp[iLength];
	}
	strcpy(&szFileNameTmp[iLength],".bstrg");
	Save(szFileNameTmp);*/

	return true;
}

bool CBSTriggerManager::Load(char *szFileName)
{
	g_BsResChecker.AddResList( szFileName );

	BFileStream fp(szFileName);
	
	if( !fp.Valid() )
	{
		DebugString( "%s load fail\n", szFileName );
		return false;
	}
	
	DWORD dwLength = (DWORD)fp.Length();
	BYTE *lpBuffer = new BYTE[dwLength];
	fp.Read( lpBuffer , dwLength );

	BMemoryStream bsMem( lpBuffer , dwLength );
	Load(&bsMem);
	delete [] lpBuffer;
	return true;
}

bool CBSTriggerManager::Load(BStream *fp)
{
	Clear();
	CreateEmptyEvents();

	int iCount = 0;
	int i = 0;
	int iVer;
	fp->Read(&iVer,sizeof(int), ENDIAN_FOUR_BYTE);
	/*if(FC_TRIGGER_FILE_VER != iVer)
	{
		BSTriggerAssert(FALSE,"File version is not match.");
		return false;
	}*/


	int si = 0;
	fp->Read(&iCount,sizeof(int), ENDIAN_FOUR_BYTE);	
	for(i = 0;i < iCount;i++)	{
		CBSTriggerForTool *pTrigTmp = new CBSTriggerForTool;
		pTrigTmp->LoadTrigger(fp,iVer);
#ifdef _USAGE_TOOL_
		for(si = 0;si < pTrigTmp->GetConditionCount();si++){
			CopySentence(pTrigTmp->GetCondition(si));
		}
		for(si = 0;si < pTrigTmp->GetActionCount();si++){
			CopySentence(pTrigTmp->GetAction(si));
		}
#endif
		m_TriggerList.push_back(pTrigTmp);
	}

	fp->Read(&iCount,sizeof(int), ENDIAN_FOUR_BYTE);	
	for(i = 0;i < iCount;i++)	{
		if(i >= (int)m_EventList.size())
		{
			BSTriggerAssert(FALSE,"It is over index in eventlist");
		}

		m_EventList[i]->LoadEvent(fp,iVer);
#ifdef _USAGE_TOOL_
		for(si = 0;si < m_EventList[i]->GetConditionCount();si++){
			CopySentence(m_EventList[i]->GetCondition(si));
		}
		for(si = 0;si < m_EventList[i]->GetActionCount();si++){
			CopySentence(m_EventList[i]->GetAction(si));
		}
#endif
	}

	fp->Read(&iCount,sizeof(int), ENDIAN_FOUR_BYTE);	
	for(i = 0;i < iCount;i++)
	{
		CBSVariable *pValTmp = new CBSVariable;
		pValTmp->LoadVariable(fp,iVer);
		m_ValList.push_back(pValTmp);
	}

	return true;
}

void CBSTriggerManager::ReplaceVariable(int SrcIndex,int DestIndex)
{
	CBSVariable *pValTmp = m_ValList[DestIndex];
	m_ValList[DestIndex] = m_ValList[SrcIndex];
	m_ValList[SrcIndex] = pValTmp;
}


void CBSTriggerManager::AddEmptyCondition(CBSTriggerEventBase *pBase,int SelectableIndex)
{
	pBase->AddCondition(m_SelectableConditionList[SelectableIndex]);
}


void CBSTriggerManager::CopySentence(CBSConditionForTool *pCondition)
{
	for(int i = 0;i < (int)m_SelectableConditionList.size();i++)
	{
		if(m_SelectableConditionList[i]->GetCodeIndex() == pCondition->GetCodeIndex())	{
			pCondition->SetSentence(m_SelectableConditionList[i]->GetSentence());
			return;
		}
	}
	BSTriggerAssert(FALSE,"Not found condition index");
}

void CBSTriggerManager::CopySentence(CBSActionForTool *pAction)
{
	for(int i = 0;i < (int)m_SelectableActionList.size();i++)
	{
		if(m_SelectableActionList[i]->GetCodeIndex() == pAction->GetCodeIndex())	{
			pAction->SetSentence(m_SelectableActionList[i]->GetSentence());
			return;
		}
	}
	BSTriggerAssert(FALSE,"Not found action index");
}


int TrgStrlen(char *szStr)
{
	if(szStr){return strlen(szStr);}
	else{return 0;}
}


void BSTriggerAssert(BOOL bIsTrue,char *szMsg)
{
	if(bIsTrue == FALSE)
	{
#ifdef _USAGE_TOOL_
	AfxMessageBox(szMsg);
#else
		char szMsgTmp[256];
		strcpy(szMsgTmp,szMsg);
		DebugString( "%s\n", szMsgTmp );
		BsAssert(0 );
#endif

	}
}


void CBSTriggerManager::ReplaceNewCon(CBSTriggerEventBase *pTrigger,int iSelectableIndex,int iConIndex)
{
	memcpyCondition(pTrigger->GetCondition(iConIndex),m_SelectableConditionList[iSelectableIndex]);
}

void CBSTriggerManager::ReplaceNewAct(CBSTriggerEventBase *pTrigger,int iSelectableIndex,int iActIndex)
{
	memcpyAction(pTrigger->GetAction(iActIndex),m_SelectableActionList[iSelectableIndex]);
}

void CBSTriggerManager::InsertCondition(BST_TRGMODE mode,int iTriggerEventIndex, int Insert,CBSConditionForTool *pCondition)
{
	if(mode == BST_TRIGGER)
	{
		m_TriggerList[iTriggerEventIndex]->InsertCondition(Insert,pCondition);		
	}
	else if(mode == BST_EVENT)
	{
		m_EventList[iTriggerEventIndex]->InsertCondition(Insert,pCondition);
	}
}

void CBSTriggerManager::InsertAction(BST_TRGMODE mode,int iTriggerEventIndex, int Insert,CBSActionForTool *pAction)
{
	if(mode == BST_TRIGGER)
	{
		m_TriggerList[iTriggerEventIndex]->InsertAction(Insert,pAction);

	}
	else if(mode == BST_EVENT)
	{
		m_EventList[iTriggerEventIndex]->InsertAction(Insert,pAction);
	}
}


void CBSTriggerManager::BackupCondition(CBSConditionForTool *pCondition)
{
	memcpyCondition(m_pBackupCondition,pCondition);
}

void CBSTriggerManager::BackupAction(CBSActionForTool *pAction)
{
	memcpyAction(m_pBackupAction,pAction);
}

void CBSTriggerManager::RestoreCondition(CBSTriggerEventBase *pBase,int iConditionIndex)
{
	memcpyCondition(pBase->GetCondition(iConditionIndex),m_pBackupCondition);
}


void CBSTriggerManager::RestoreAction(CBSTriggerEventBase *pBase, int iActionIndex)
{
	memcpyAction(pBase->GetAction(iActionIndex),m_pBackupAction);
}



void CBSTriggerManager::InsertTrigger(int iIndex,CBSTriggerForTool *pTrigger)
{
	CBSTriggerForTool *pTmpTrg = new CBSTriggerForTool;
	memcpyTrigger(pTmpTrg,pTrigger);

	m_TriggerList.insert(m_TriggerList.begin() + iIndex,pTmpTrg);
}

void CBSTriggerManager::NotifyTrgListPaste(int nIndex)
{
	for(int i = 0;i < GetTriggerCount();i++)
	{
		std::vector<CBSTriggerParam		*> ParamList;
		int nCount = GetTrigger(i)->GetConditionCount();
		int s = 0;
		for( s = 0;s < nCount;s++ )
		{
			GetTrigger(i)->GetCondition(s)->GetParamList( ParamList );
		}
		nCount = GetTrigger(i)->GetActionCount();
		for( s = 0;s < nCount;s++ )
		{
			GetTrigger(i)->GetAction(s)->GetParamList( ParamList );
		}
		nCount = ParamList.size();
		for( s = 0;s < nCount;s++ )
		{
			if( _stricmp( GetParamCaption( ParamList[s]->GetCodeIndex() ),"TriggerID" ) == 0 ){
				if( ParamList[s]->GetInteger() >= nIndex)
				{
					ParamList[s]->SetData( ParamList[s]->GetInteger() + 1 );
				}
			}
		}
	}
}

void CBSTriggerManager::NotifyTrgListDelete(int nIndex)
{
	for(int i = 0;i < GetTriggerCount();i++)
	{
		std::vector<CBSTriggerParam		*> ParamList;
		int nCount = GetTrigger(i)->GetConditionCount();
		int s = 0;
		for( s = 0;s < nCount;s++ )
		{
			GetTrigger(i)->GetCondition(s)->GetParamList( ParamList );
		}
		nCount = GetTrigger(i)->GetActionCount();
		for( s = 0;s < nCount;s++ )
		{
			GetTrigger(i)->GetAction(s)->GetParamList( ParamList );
		}
		nCount = ParamList.size();
		for( s = 0;s < nCount;s++ )
		{
			if( _stricmp( GetParamCaption( ParamList[s]->GetCodeIndex() ),"TriggerID" ) == 0 ){
				if( ParamList[s]->GetInteger() == nIndex){
					ParamList[s]->SetData(-1);
					BSTriggerAssert( false, "TriggerID Parameter set -1(this index has been existing in condition or action)");
				}
				if( ParamList[s]->GetInteger() > nIndex ){				
					ParamList[s]->SetData( ParamList[s]->GetInteger() - 1 );
				}
			}
		}

	}
}


void CBSTriggerManager::GetParamList(int nTrgIdx, std::vector<CBSTriggerParam *> &vtTrgParam )
{
	BSTriggerAssert( nTrgIdx != -1 , "Trigger index -1");
	BSTriggerAssert( nTrgIdx < GetTriggerCount() , "Trigger index is over max");

	int i=0,is=0;
	CBSTriggerForTool *pTrg = GetTrigger( nTrgIdx );
	for( is = 0;is < pTrg->GetConditionCount();is++)
	{
		pTrg->GetCondition(is)->GetParamList( vtTrgParam );
	}
	for( is = 0;is < pTrg->GetActionCount();is++)
	{
		pTrg->GetAction(is)->GetParamList( vtTrgParam );
	}
}







