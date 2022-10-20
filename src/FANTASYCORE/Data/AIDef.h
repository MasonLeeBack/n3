#pragma		 once

enum AI_OP
{
	AI_OP_NOT_EQUAL = 0,
	AI_OP_EQUAL,
	AI_OP_GREATER,
	AI_OP_GREATER_THAN_OR_EQUAL,
	AI_OP_LESS_THAN,
	AI_OP_LESS_THAN_OR_EQUAL
};

enum AI_ELEMENT_PROCESS_TYPE
{
	AI_ELEMENT_PROCESS_TICK,
	AI_ELEMENT_PROCESS_LOD,
	AI_ELEMENT_PROCESS_CALLBACK,
};

enum AI_ELEMENT_PARENT
{
	PARENT_THIS,
	PARENT_LINK,
};

enum AI_SEARCH_TYPE
{
	AI_SEARCH_NONE = 0x00000000,						// �ƹ� �ǹ� ����
	AI_SEARCH_ALL = 0x00000001,							// [Range] ���� ���� ��� ������Ʈ �˻�
	AI_SEARCH_GET = 0x00000002,							// [ElementIndex] �ش� ��ũ��Ʈ�� ElementIndex ��° ������Ʈ�� Search ����� ���
	AI_SEARCH_GLOBAL_VARIABLE = 0x00000003,				// [GlobalValueString] AI_ACTION_SET_TARGET_GLOBAL_VARIABLE �� ���õ� ������Ʈ
	AI_SEARCH_SLOT = 0x00000004,						// [Index] AI_ACTION_SET_TARGET_SLOT �� ���õ� ������Ʈ
	AI_SEARCH_ALL_TROOP_OFFSET = 0x00000005,			// �δ볻�� �ڱ� ��ġ�� �������� �˻� -> RangeScan

	AI_SEARCH_ALL_TARGET_TROOP_OFFSET = 0x00000006,		// [Range]�δ뿡�� Ÿ���� �Ǿ��ִ� �δ� �߽ɱ��� Search, -1 �� ��� �δ� ��ü�� �Ѱ��ش�. -> TroopList
	AI_SEARCH_CORRELATION = 0x00000007,					// [Correlation Type]
	AI_SEARCH_ALL_TROOP = 0x00000008,				// [Range, Enemy/Friendly ( True,False ) ] �δ��߽����� �αٺδ� �˻� -1�� ��� AttackRadius �� �˻��Ѵ�.
	AI_SEARCH_LINK_PARENT_TROOP = 0x00000009,		// ���� ��ũ�� �δ�
	AI_SEARCH_TYPE_MASK = 0x000000FF,
};

enum AI_SEARCH_CON {
	AI_SEARCH_CON_NONE = 0x00000000,
	AI_SEARCH_CON_IS_VALID_SLOT = 0x00001000,		// [Index] ������ �������� Search �Ѵ�
	AI_SEARCH_CON_IS_INVALID_SLOT = 0x00002000,		// [Index] ������ ��������� Search �Ѵ�
	AI_SEARCH_CON_GLOBAL_VARIABLE_INT = 0x00003000,	// [GlobalValueString, Value, OPERATOERID]

	AI_SEARCH_CON_MASK = 0x0000FF00,
};

enum AI_SEARCH_FILTER
{
	AI_SEARCH_FILTER_NONE = 0x00000000,				
	AI_SEARCH_FILTER_RANDOM = 0x00010000,			// ���� ���� ��� ������Ʈ �˻� �� ������ 1���� ������Ʈ
	AI_SEARCH_FILTER_HERO = 0x00020000,				// �÷��̾�
	AI_SEARCH_FILTER_NEAREST = 0x00030000,			// ���� ������ ���� �� ������Ʈ
	AI_SEARCH_FILTER_FARTHEST = 0x00040000,			// ���� ���� ����� ������Ʈ
	AI_SEARCH_FILTER_TARGETING_COUNT = 0x00050000,	// [Count(int), OPERATOERID] ������Ʈ�� Ÿ���� ī��Ʈ�� ���͸�
	AI_SEARCH_FILTER_LAST_HIT = 0x00060000,			// ���������� �� ����
	AI_SEARCH_FILTER_MAXIMUM_DAMAGE = 0x00070000,
	AI_SEARCH_FILTER_DIR = 0x00080000,				// [Side(int) : <0,1,2,3:Left,Right,Front,Back>, Angle(int)] �� ������ ���� ���� ���ǰ͸�
	AI_SEARCH_FILTER_TROOP_OFFSET_NEAREST = 0x00090000,	// �δ���ġ �������� ��Ʈ
	AI_SEARCH_FILTER_TROOP_OFFSET_FARTHEST = 0x000A0000,	// �δ���ġ �������� ��Ʈ
	AI_SEARCH_FILTER_PREFERENCE_CLASS_ID = 0x000B0000,			// ClassID �� ����.. 3������ ��������

	AI_SEARCH_FILTER_MASK = 0x00FF0000,
};

enum AI_CALLBACK_TYPE {
	AI_CALLBACK_NONE = -1,
	AI_CALLBACK_ALWAYS,
	AI_CALLBACK_HIT,
	AI_CALLBACK_SIGNAL,
	AI_CALLBACK_DEFENSE,
	AI_CALLBACK_ATTACK,
	AI_CALLBACK_NUM,
};

enum AI_CON_TYPE
{
	AI_CON_IS_TARGET_MY_TEAM = 0,		// Target�� ���� ���� ���̶��
	AI_CON_IS_TARGET_ENEMY_TEAM,		// Target�� ���� �ٸ� ���̶��
	AI_CON_IS_TARGET_LIVE,				// Target�� ���������
	AI_CON_IS_TARGET_DIE,				// Target�� �׾�����
	AI_CON_IS_SELF_LIVE,				// �ڱⰡ ����ִٸ�
	AI_CON_IS_SELF_DIE,					// �ڱⰡ �׾��ٸ�
	AI_CON_TARGET_HP,					// [HP(int), OPERATOERID] 		Target�� HP�� OPERATERID �����̶��
	AI_CON_SELF_HP,						//  [HP(int), OPERATOERID] 		HP�� OPERATERID �����̶��
	AI_CON_TARGET_RANGE,				// [�Ÿ�(int), OPERATOERID]		Target�� �Ÿ��� OPERATERID �����̶��
	AI_CON_ENEMY_SEARCH_COUNT,			// [Value, OPERATOERID] Search ���� �˻��� �� ������Ʈ�� ����
	AI_CON_GLOBAL_VARIABLE_INT,			// [GlobalValueString, Value, OPERATOERID] ���� ���� üũ
	AI_CON_GLOBAL_VARIABLE_FLOAT,		//		""
	AI_CON_GLOBAL_VARIABLE_STRING,		//		""
	AI_CON_GLOBAL_VARIABLE_VECTOR,		//		""	�Ķ���ʹ� 3���� �ǰ���?
	AI_CON_GLOBAL_VARIABLE_VECTOR_LENGTH,//		""	��� �Ѱ�
	AI_CON_GLOBAL_VARIABLE_BOOLEAN,		//		""
	AI_CON_GLOBAL_VARIABLE_RANDOM,		//		"" �������� ���´�
	AI_CON_GLOBAL_VARIABLE_PTR,			//		""
	AI_CON_CAN_ANI,						// [AniAttr(int), AniType(int), AniIndex(int)]  �� �� �ֳ�?
	AI_CON_CANNOT_ANI,					// [AniAttr(int), AniType(int), AniIndex(int)]  �� �� ����?
	AI_CON_SELF_ANI,					// [AniAttr(int), AniType(int), AniIndex(int), OPERATOERID] �ڱ� �ڽ��� ���� ���� Ÿ��
	AI_CON_TARGET_ANI,					// [AniAttr(int), AniType(int), AniIndex(int), OPERATOERID] Ÿ���� ���� ���� Ÿ��
	AI_CON_IS_VALID_SLOT,				// [SlotIndex(int)] AI_ACTION_SET_TARGET_SLOT ���� ���õ� ������ ������Ʈ üũ
	AI_CON_IS_INVALID_SLOT,				// [SlotIndex(int)] AI_ACTION_SET_TARGET_SLOT ���� ���õ� ������ ������Ʈ üũ
	AI_CON_FUNCTION,					// [Function, Value, OPERATOERID]
	AI_CON_INFLUENCE_TARGETING_COUNT,	// [Value(int), OPERATOERID]	Target�� Ÿ���� ���� ���� ����
	AI_CON_INFLUENCE_DAMAGE,			// [Value(int), OPERATOERID]	Target�� ����� �� ����
	AI_CON_INFLUENCE_HIT_COUNT,			// [Value(int), OPERATOERID]	Target�� ��Ʈ ī��Ʈ ����
	AI_CON_JOB_INDEX,					// [Value(int), OPERATOERID]	������ 0, ������ 1
	AI_CON_TARGET_CLASS_ID,				// [Value(int), OPERATOERID]	Ŭ���� �Ƶ�~
	AI_CON_TARGET_LINK_CLASS_ID,		// [Value(int), OPERATOERID]	��ũ�� �θ� Ŭ���� �Ƶ�~
	AI_CON_IS_SHOW_WEAPON,				// [WeaponIndex]
	AI_CON_IS_HIDE_WEAPON,				// [WeaponIndex]
	AI_CON_TARGET_IS_VALID_LINK,		// ��ũ�� �θ� Ŭ������ �ִٸ�
	AI_CON_TARGET_IS_INVALID_LINK,		//	,.
	AI_CON_TARGET_VIEW_ANGLE,			// [Angle(int), OPERATOERID] �ڽ��� ZVector �� Ÿ���� ��ġ���� Angle��
	AI_CON_TARGET_IS_VALID,
	AI_CON_TARGET_IS_INVALID,
	AI_CON_IS_PROCESS_TRUE_ORB,			// [True/False]
	AI_CON_SELF_EVENT_SEQ_COUNT,
	AI_CON_AICIDX,						// [Value(int), OPERATOERID]
	AI_CON_IS_VALID_CATCH,				// Hero���� CatchHandle üũ
	AI_CON_IS_INVALID_CATCH,			// Hero���� CatchHandle üũ

	// ���� ����

	// �δ� ���� Condition
	AI_CON_TROOP_STATE,					// �δ� ����
	AI_CON_TROOP_STATE_TABLE,			// �δ� ���� ���̺� üũ
	AI_CON_TROOP_SUB_STATE,				// �δ� ���� ����
	AI_CON_TROOP_OFFSET_RANGE,			// [Value, OPERATOERID ]�δ뿡 ���õ� ��ġ�� ���� �Ÿ�
	AI_CON_TROOP_TARGET_OFFSET_RANGE,	// [Value, OPERATOERID ]�δ뿡 ���õ� ��ġ�� Ÿ�� �Ÿ�
	AI_CON_TROOP_TARGET_TROOP_RANGE,	// [Value, OPERATOERID ] ŸĹ�δ��� ��ġ�� ������ �Ÿ�
	AI_CON_TROOP_IS_IN_RANGE,			// [True/False] �ڽ��� �ڱ� �δ� ���� ���� �ִ°�
	AI_CON_TROOP_TARGET_IS_IN_RANGE,	// [True/False] Ÿ���� �ڱ� �δ� ���� ���� �ִ°�
	AI_CON_TROOP_TARGET_IS_IN_TROOP,	// [True/False] Ÿ���� Ÿ���� �δ� ����Ʈ�� �ִ� ���ΰ�...
	AI_CON_TROOP_TARGET_IS_UNTOUCHABLE,		// [True/False] �δ밡 ���� ���� ���� ��������.. Ʈ���� �÷��� üũ
	AI_CON_TROOP_READY_RANGE_ORDER,		// [Value, OPERATOERID, MaxUpdateTickInterval] �и������϶� Ÿ���� �÷��̾� �δ�� �ڽ��� �÷��̾���� �Ÿ��� ���°�� ����� ������ üũ
	AI_CON_TROOP_TARGET_IS_PLAYER,		// [True/False] �÷��̾� �δ��?
	AI_CON_DEBUG_STRING,
};


enum AI_ACTION_TYPE
{
	AI_ACTION_DELAY = 0,				// [Delay(int)]					Delay tick��ŭ ��ٸ���
	AI_ACTION_SET_ANI,					// [AniAttr(int), AniType(int), AniIndex(int), ovelap Flag ]	�� �ִϸ� AniType���� ����
	AI_ACTION_SET_ANI_LOOP,				// [AniAttr(int), AniType(int), AniIndex(int), ovelap Flag, Loop Count ]	�� �ִϸ� AniType���� ����
	AI_ACTION_FOLLOW_TARGET,			// [AniAttr(int), AniType(int), AniIndex(int), Value, speed(float)]		Value ��ŭ�� �Ÿ����� Ÿ�� ������Ʈ�� �i�ư���
	AI_ACTION_OPPOSITION_TARGET,		// [Value]						Value ��ŭ Ÿ���� �ݴ�������� ���� �̵���Ų��.
	AI_ACTION_OPPOSITION_TARGET_TO_ME,	// [Value]						Value ��ŭ Ÿ���� �ڽ��� �������� ���� �̵���Ų��.
	AI_ACTION_STOP,						// [AniAttr(int), AniType(int), AniIndex(int)]				�����
	AI_ACTION_LOOK_TARGET,				//								Ÿ���� �Ĵٺ���
	AI_ACTION_LOOK_CUSTOM,				// [Q(float)]					Ÿ���� �������� Angle �� ��ŭ ȸ�� �� ���� �ٶ󺻴�.
	AI_ACTION_CHANGE_GLOBAL_VARIABLE_INT,			// [GlobalValueString, Value]	���� ���� ����
	AI_ACTION_CHANGE_GLOBAL_VARIABLE_FLOAT,			//		""
	AI_ACTION_CHANGE_GLOBAL_VARIABLE_STRING,		//		""
	AI_ACTION_CHANGE_GLOBAL_VARIABLE_VECTOR,		//		"" �Ķ���ʹ� 3��
	AI_ACTION_CHANGE_GLOBAL_VARIABLE_BOOLEAN,		//		""
	AI_ACTION_CHANGE_GLOBAL_VARIABLE_RANDOM,		//		"" �Ķ���ʹ� 2�� �ּҰ�, �ִ밪�̴�.
	AI_ACTION_CHANGE_GLOBAL_VARIABLE_PTR,			//		"" �Ķ���ʹ� 1��.. ������ NULL ���õȴ�.
	AI_ACTION_SET_TARGET_GLOBAL_VARIABLE,	// [GlobalValueString]	���� Search �� Target �� �����͸� ��������(PTR)�� �����Ѵ�.
	AI_ACTION_SET_TARGET_SLOT,				// [Index(int)] ���� Search �� Target�� Slot�� �����Ѵ�.
	AI_ACTION_EMPTY_SLOT,					// [Index(int)] Slot�� ����.
	AI_ACTION_LOOK_SLOT,					// [index(int), TRUE,FALSE] ������ Slot�� ����/����
	AI_ACTION_INFLUENCE_TARGETING,			// [TRUE/FALSE]
	AI_ACTION_INFLUENCE_TARGETING_EMPTY,	//
	AI_ACTION_INFLUENCE_TARGETING_SLOT,		// [TRUE/FALSE]
	AI_ACTION_INFLUENCE_DAMAGE_EMPTY,		// 
	AI_ACTION_INFLUENCE_HIT_COUNT_EMPTY,	//

	AI_ACTION_CMD_ATTACK,				// Ÿ�� ����
	AI_ACTION_CUSTOM_MOVE,				// [AniAttr(int), AniType(int), AniIndex(int), speed(float), Angle(int), Length(int)]
										// �ڽŰ� Ÿ���� ������ �������� ���������� Angle ��ŭ ���� ������ Length��ŭ�� �Ÿ��� �̵�
	AI_ACTION_SELF_CUSTOM_MOVE,			// [AniAttr(int), AniType(int), AniIndex(int), speed(float), Angle(int), Length(int)] 
										// �ڱ� �ڽ���ġ���� ����������  Angle ��ŭ ���� ������ Length ��ŭ�� �Ÿ��� �̵��Ѵ�.
	AI_ACTION_EXIT,						// ���� Element üũ ����.. �������´�.

	AI_ACTION_PUSH_BUTTON,				// [Button Index] ��ư�� ������~ ����� ��Ʈ���� ����
	AI_ACTION_RELEASE_BUTTON,			// [Button Index] ��ư�� �Ŷ�~ ����� ��Ʈ���� ����

	AI_ACTION_ADD_DEFENSE_PROB,			// [Value] ���潺 Ȯ�� �ø� %��.. 
	AI_ACTION_DESTROY_ELEMENT,			// [Index] ������Ʈ ���� ����.. Index �� -1 �̸� �ڱ��ڽ�
	AI_ACTION_LINK_OBJECT,				// ��ũ~
	AI_ACTION_UNLINK_OBJECT,			// ��ũ~
	AI_ACTION_SET_ARCHER_HIT_PROB,		// [Value] ������ ��� ���߷� ����
	AI_ACTION_GLOBAL_VARIABLE_INT_COUNTER,	// [GlobalValueString, Value] �������� ����
	AI_ACTION_GLOBAL_VARIABLE_INT_RANDOM2,	// [GlobalValueString, Value1, Value2, Value1Prob]
	AI_ACTION_GLOBAL_VARIABLE_INT_RANDOM3,	// [GlobalValueString, Value1, Value2, Value3, Value1Prob, Value2Prob]
	AI_ACTION_GLOBAL_VARIABLE_INT_RANDOM4,	// [GlobalValueString, Value1, Value2, Value3, Value4, Value1Prob, Value2Prob, Value3Prob]
	AI_ACTION_SET_AICIDX,				// [Value]
	AI_ACTION_PAUSE_FUNCTION,			// [Function String]
	AI_ACTION_UNPAUSE_FUNCTION,			// [Function String]
	AI_ACTION_RESET_MOVE_VECTOR,		//

	// �δ� �׼�
	AI_ACTION_FOLLOW_TROOP,				// [AniAttr(int), AniType(int), AniIndex(int), speed(float)] �δ���ġ�� �̵�

	AI_ACTION_DEBUG_STRING,
	AI_ACTION_DELAY_ORDER,				 
	AI_ACTION_WARP,						 
	AI_ACTION_DIMINUTION_VELOCITY,		// [Value(float)] �и��°� ����
};

// �ϴ� CFcParamVariable �� �մ°� �����س��.. 
enum AI_VARIABLE_TYPE {
	VARIABLE_TYPE_INT = 0,
	VARIABLE_TYPE_FLOAT = 1,
	VARIABLE_TYPE_STRING = 3,
	VARIABLE_TYPE_VECTOR = 4,
	VARIABLE_TYPE_PTR = 5,
	VARIABLE_TYPE_BOOLEAN = 6,
	VARIABLE_TYPE_RANDOM = 7,
};

// GameObject ClassID 
enum GameObj_ClassID {
	Class_ID_Unknown = -1,
	Class_ID_Unit = 0,
	Class_ID_Fly = 1,
	Class_ID_Horse = 2,
	Class_ID_Archer = 3,
	Class_ID_Catapult = 4,
	Class_ID_MoveTower = 5,
	Class_ID_Adjutant = 6,

	Class_ID_Hero = 1000,
	Class_ID_Hero_Aspharr = 1001,
	Class_ID_Hero_Inphy = 1002,
	Class_ID_Hero_Klarrann = 1003,
	Class_ID_Hero_VigkVagk = 1004,
	Class_ID_Hero_Myifee = 1005,
	Class_ID_Hero_Dwingvatt = 1006,
	Class_ID_Hero_Tyurru = 1007,
};

// Key Define.. Input.h �� �ִ°� �״�� ����..
#define PAD_INPUT_A							0
#define PAD_INPUT_B							1
#define PAD_INPUT_X							2
#define PAD_INPUT_Y							3
#define PAD_INPUT_BLACK						4			// PAD_INPUT_R
#define PAD_INPUT_WHITE						5			// PAD_INPUT_L
#define PAD_INPUT_UP						6
#define PAD_INPUT_DOWN						7
#define PAD_INPUT_LEFT						8
#define PAD_INPUT_RIGHT						9
#define PAD_INPUT_START						10
#define PAD_INPUT_BACK						11
#define PAD_INPUT_LTHUMB					12
#define PAD_INPUT_RTHUMB					13
#define PAD_INPUT_LTRIGGER					14
#define PAD_INPUT_RTRIGGER					15
#define PAD_INPUT_LSTICK					16
#define PAD_INPUT_RSTICK					17

namespace AI_TROOP_STATE_DEFINE {
// Troop State Define FcTroopObject.h �� �ִ°� �״�� ����..
enum TROOPSTATE
{
	TROOPSTATE_NON=0,
	TROOPSTATE_MOVE,
	TROOPSTATE_MOVE_MERCIFULLY,		// m_vecTargetTroops, m_vecCounterattractionTroops
	TROOPSTATE_MOVE_PATH,
	TROOPSTATE_MOVE_PATH_LOOP,
	TROOPSTATE_MOVE_FULL_PATH,
	TROOPSTATE_RETREAT,
	TROOPSTATE_RANGE_ATTACK,
	TROOPSTATE_MELEE_ATTACK,
	TROOPSTATE_ELIMINATED,
	TROOPSTATE_MOVE_ATTACK,
	TROOPSTATE_MELEE_READY,			// �δ밡 melee�� ���� �������� �ִ� ����
	TROOPSTATE_HOLD,				// �� �����̰� ���� ���� ����
	TROOPSTATE_HOLD_MERCIFULLY,
	TROOPSTATE_FOLLOW,
	TROOPSTATE_NUM
};

enum TROOPSUBSTATE
{
	TROOPSUBSTATE_WALK,
	TROOPSUBSTATE_RUN,
	TROOPSUBSTATE_BATTLERUN,
	TROOPSUBSTATE_NUM
};
#define UNIT_MOVABLE			0x00000001
#define UNIT_ATTACKABLE			0x00000002
#define UNIT_JUMPABLE			0x00000004
#define UNIT_FLOCKABLE			0x00000008
#define UNIT_DEFENSIBLE			0x00000010
#define UNIT_RIDABLE			0x00000020
#define UNIT_RANGE				0x00000040

};

#ifndef _DEFINE_PROP_TYPE
#define _DEFINE_PROP_TYPE
enum PROP_TYPE
{
	PROP_TYPE_NORMAL,
	PROP_TYPE_TOWER,		// ����
	PROP_TYPE_BRIDGE,
	PROP_TYPE_GATE,
	PROP_TYPE_ENABLE_ATTACK_WALL,
	PROP_TYPE_TRAP,
	PROP_TYPE_BILLBOARD,
	PROP_TYPE_PICKUP,
	PROP_TYPE_CRUMBLE,		// �������鼭 ���� ���� �༮
	PROP_TYPE_FALLDOWN,
	PROP_TYPE_ANIMATION,
	PROP_TYPE_ITEM,
	PROP_TYPE_WALL
};
#endif //_DEFIND_PROP_TYPE

enum CORRELATION_TYPE {
		CT_NONE = 0x00,
		CT_TARGETING_COUNTER = 0x01,
		CT_HIT = 0x02,
	};

