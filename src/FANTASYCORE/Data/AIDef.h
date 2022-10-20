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
	AI_SEARCH_NONE = 0x00000000,						// 아무 의미 없다
	AI_SEARCH_ALL = 0x00000001,							// [Range] 영역 내의 모든 오브젝트 검색
	AI_SEARCH_GET = 0x00000002,							// [ElementIndex] 해당 스크립트의 ElementIndex 번째 엘리먼트의 Search 결과를 사용
	AI_SEARCH_GLOBAL_VARIABLE = 0x00000003,				// [GlobalValueString] AI_ACTION_SET_TARGET_GLOBAL_VARIABLE 로 셋팅된 오브젝트
	AI_SEARCH_SLOT = 0x00000004,						// [Index] AI_ACTION_SET_TARGET_SLOT 로 셋팅된 오브젝트
	AI_SEARCH_ALL_TROOP_OFFSET = 0x00000005,			// 부대내의 자기 위치를 기준으로 검색 -> RangeScan

	AI_SEARCH_ALL_TARGET_TROOP_OFFSET = 0x00000006,		// [Range]부대에서 타겟팅 되어있는 부대 중심기준 Search, -1 일 경우 부대 전체를 넘겨준다. -> TroopList
	AI_SEARCH_CORRELATION = 0x00000007,					// [Correlation Type]
	AI_SEARCH_ALL_TROOP = 0x00000008,				// [Range, Enemy/Friendly ( True,False ) ] 부대중심으로 인근부대 검색 -1일 경우 AttackRadius 로 검색한다.
	AI_SEARCH_LINK_PARENT_TROOP = 0x00000009,		// 내가 링크한 부대
	AI_SEARCH_TYPE_MASK = 0x000000FF,
};

enum AI_SEARCH_CON {
	AI_SEARCH_CON_NONE = 0x00000000,
	AI_SEARCH_CON_IS_VALID_SLOT = 0x00001000,		// [Index] 슬롯이 차있으면 Search 한다
	AI_SEARCH_CON_IS_INVALID_SLOT = 0x00002000,		// [Index] 슬롯이 비어있으면 Search 한다
	AI_SEARCH_CON_GLOBAL_VARIABLE_INT = 0x00003000,	// [GlobalValueString, Value, OPERATOERID]

	AI_SEARCH_CON_MASK = 0x0000FF00,
};

enum AI_SEARCH_FILTER
{
	AI_SEARCH_FILTER_NONE = 0x00000000,				
	AI_SEARCH_FILTER_RANDOM = 0x00010000,			// 영역 내의 모든 오브젝트 검색 후 랜덤한 1개의 오브젝트
	AI_SEARCH_FILTER_HERO = 0x00020000,				// 플레이어
	AI_SEARCH_FILTER_NEAREST = 0x00030000,			// 영역 내에서 가장 먼 오브젝트
	AI_SEARCH_FILTER_FARTHEST = 0x00040000,			// 영역 내의 히어로 오브젝트
	AI_SEARCH_FILTER_TARGETING_COUNT = 0x00050000,	// [Count(int), OPERATOERID] 오브젝트의 타갯팅 카운트로 필터링
	AI_SEARCH_FILTER_LAST_HIT = 0x00060000,			// 마지막으로 날 깐쉑히
	AI_SEARCH_FILTER_MAXIMUM_DAMAGE = 0x00070000,
	AI_SEARCH_FILTER_DIR = 0x00080000,				// [Side(int) : <0,1,2,3:Left,Right,Front,Back>, Angle(int)] 각 방향의 일정 각도 안의것만
	AI_SEARCH_FILTER_TROOP_OFFSET_NEAREST = 0x00090000,	// 부대위치 기준으로 소트
	AI_SEARCH_FILTER_TROOP_OFFSET_FARTHEST = 0x000A0000,	// 부대위치 기준으로 소트
	AI_SEARCH_FILTER_PREFERENCE_CLASS_ID = 0x000B0000,			// ClassID 로 소팅.. 3개까지 지정가능

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
	AI_CON_IS_TARGET_MY_TEAM = 0,		// Target이 나와 같은 팀이라면
	AI_CON_IS_TARGET_ENEMY_TEAM,		// Target이 나와 다른 팀이라면
	AI_CON_IS_TARGET_LIVE,				// Target이 살아있으면
	AI_CON_IS_TARGET_DIE,				// Target이 죽었으면
	AI_CON_IS_SELF_LIVE,				// 자기가 살아있다면
	AI_CON_IS_SELF_DIE,					// 자기가 죽었다면
	AI_CON_TARGET_HP,					// [HP(int), OPERATOERID] 		Target의 HP가 OPERATERID 조건이라면
	AI_CON_SELF_HP,						//  [HP(int), OPERATOERID] 		HP가 OPERATERID 조건이라면
	AI_CON_TARGET_RANGE,				// [거리(int), OPERATOERID]		Target과 거리가 OPERATERID 조건이라면
	AI_CON_ENEMY_SEARCH_COUNT,			// [Value, OPERATOERID] Search 에서 검색된 적 오브젝트의 갯수
	AI_CON_GLOBAL_VARIABLE_INT,			// [GlobalValueString, Value, OPERATOERID] 전역 변수 체크
	AI_CON_GLOBAL_VARIABLE_FLOAT,		//		""
	AI_CON_GLOBAL_VARIABLE_STRING,		//		""
	AI_CON_GLOBAL_VARIABLE_VECTOR,		//		""	파라메터는 3개가 되겠지?
	AI_CON_GLOBAL_VARIABLE_VECTOR_LENGTH,//		""	요건 한개
	AI_CON_GLOBAL_VARIABLE_BOOLEAN,		//		""
	AI_CON_GLOBAL_VARIABLE_RANDOM,		//		"" 랜덤값이 나온다
	AI_CON_GLOBAL_VARIABLE_PTR,			//		""
	AI_CON_CAN_ANI,						// [AniAttr(int), AniType(int), AniIndex(int)]  할 수 있냐?
	AI_CON_CANNOT_ANI,					// [AniAttr(int), AniType(int), AniIndex(int)]  할 수 없냐?
	AI_CON_SELF_ANI,					// [AniAttr(int), AniType(int), AniIndex(int), OPERATOERID] 자기 자신의 현제 에니 타입
	AI_CON_TARGET_ANI,					// [AniAttr(int), AniType(int), AniIndex(int), OPERATOERID] 타겟의 현제 에니 타입
	AI_CON_IS_VALID_SLOT,				// [SlotIndex(int)] AI_ACTION_SET_TARGET_SLOT 으로 셋팅된 슬롯의 오브젝트 체크
	AI_CON_IS_INVALID_SLOT,				// [SlotIndex(int)] AI_ACTION_SET_TARGET_SLOT 으로 셋팅된 슬롯의 오브젝트 체크
	AI_CON_FUNCTION,					// [Function, Value, OPERATOERID]
	AI_CON_INFLUENCE_TARGETING_COUNT,	// [Value(int), OPERATOERID]	Target의 타겟팅 갯수 저항 조건
	AI_CON_INFLUENCE_DAMAGE,			// [Value(int), OPERATOERID]	Target의 대미지 양 조건
	AI_CON_INFLUENCE_HIT_COUNT,			// [Value(int), OPERATOERID]	Target의 히트 카운트 조건
	AI_CON_JOB_INDEX,					// [Value(int), OPERATOERID]	메인잡 0, 서브잡 1
	AI_CON_TARGET_CLASS_ID,				// [Value(int), OPERATOERID]	클래스 아뒤~
	AI_CON_TARGET_LINK_CLASS_ID,		// [Value(int), OPERATOERID]	링크된 부모 클래스 아뒤~
	AI_CON_IS_SHOW_WEAPON,				// [WeaponIndex]
	AI_CON_IS_HIDE_WEAPON,				// [WeaponIndex]
	AI_CON_TARGET_IS_VALID_LINK,		// 링크된 부모 클래스가 있다면
	AI_CON_TARGET_IS_INVALID_LINK,		//	,.
	AI_CON_TARGET_VIEW_ANGLE,			// [Angle(int), OPERATOERID] 자신의 ZVector 와 타겟의 위치와의 Angle값
	AI_CON_TARGET_IS_VALID,
	AI_CON_TARGET_IS_INVALID,
	AI_CON_IS_PROCESS_TRUE_ORB,			// [True/False]
	AI_CON_SELF_EVENT_SEQ_COUNT,
	AI_CON_AICIDX,						// [Value(int), OPERATOERID]
	AI_CON_IS_VALID_CATCH,				// Hero류의 CatchHandle 체크
	AI_CON_IS_INVALID_CATCH,			// Hero류의 CatchHandle 체크

	// 프랍 관련

	// 부대 관련 Condition
	AI_CON_TROOP_STATE,					// 부대 상태
	AI_CON_TROOP_STATE_TABLE,			// 부대 상태 테이블 체크
	AI_CON_TROOP_SUB_STATE,				// 부대 서브 상태
	AI_CON_TROOP_OFFSET_RANGE,			// [Value, OPERATOERID ]부대에 셋팅된 위치와 나의 거리
	AI_CON_TROOP_TARGET_OFFSET_RANGE,	// [Value, OPERATOERID ]부대에 셋팅된 위치와 타갯 거리
	AI_CON_TROOP_TARGET_TROOP_RANGE,	// [Value, OPERATOERID ] 타캣부대의 위치와 나와의 거리
	AI_CON_TROOP_IS_IN_RANGE,			// [True/False] 자신이 자기 부대 영역 내에 있는가
	AI_CON_TROOP_TARGET_IS_IN_RANGE,	// [True/False] 타갯이 자기 부대 영역 내에 있는가
	AI_CON_TROOP_TARGET_IS_IN_TROOP,	// [True/False] 타갯이 타갯팅 부대 리스트에 있는 놈인가...
	AI_CON_TROOP_TARGET_IS_UNTOUCHABLE,		// [True/False] 부대가 공격 에니 가능 상태인지.. 트리거 플레그 체크
	AI_CON_TROOP_READY_RANGE_ORDER,		// [Value, OPERATOERID, MaxUpdateTickInterval] 밀리레뒤일때 타겟이 플레이어 부대고 자신이 플레이어와의 거리가 몇번째로 가까운 놈인지 체크
	AI_CON_TROOP_TARGET_IS_PLAYER,		// [True/False] 플레이어 부대냐?
	AI_CON_DEBUG_STRING,
};


enum AI_ACTION_TYPE
{
	AI_ACTION_DELAY = 0,				// [Delay(int)]					Delay tick만큼 기다린다
	AI_ACTION_SET_ANI,					// [AniAttr(int), AniType(int), AniIndex(int), ovelap Flag ]	내 애니를 AniType으로 세팅
	AI_ACTION_SET_ANI_LOOP,				// [AniAttr(int), AniType(int), AniIndex(int), ovelap Flag, Loop Count ]	내 애니를 AniType으로 세팅
	AI_ACTION_FOLLOW_TARGET,			// [AniAttr(int), AniType(int), AniIndex(int), Value, speed(float)]		Value 만큼의 거리까지 타겟 오브젝트를 쫒아간다
	AI_ACTION_OPPOSITION_TARGET,		// [Value]						Value 만큼 타겟의 반대방향으로 강제 이동시킨다.
	AI_ACTION_OPPOSITION_TARGET_TO_ME,	// [Value]						Value 만큼 타겟을 자신의 방향으로 강제 이동시킨다.
	AI_ACTION_STOP,						// [AniAttr(int), AniType(int), AniIndex(int)]				멈춘다
	AI_ACTION_LOOK_TARGET,				//								타겟을 쳐다본다
	AI_ACTION_LOOK_CUSTOM,				// [Q(float)]					타겟을 기준으로 Angle 값 만큼 회전 된 쪽을 바라본다.
	AI_ACTION_CHANGE_GLOBAL_VARIABLE_INT,			// [GlobalValueString, Value]	전역 변수 변경
	AI_ACTION_CHANGE_GLOBAL_VARIABLE_FLOAT,			//		""
	AI_ACTION_CHANGE_GLOBAL_VARIABLE_STRING,		//		""
	AI_ACTION_CHANGE_GLOBAL_VARIABLE_VECTOR,		//		"" 파라메터는 3개
	AI_ACTION_CHANGE_GLOBAL_VARIABLE_BOOLEAN,		//		""
	AI_ACTION_CHANGE_GLOBAL_VARIABLE_RANDOM,		//		"" 파라메터는 2개 최소값, 최대값이다.
	AI_ACTION_CHANGE_GLOBAL_VARIABLE_PTR,			//		"" 파라메터는 1개.. 무조건 NULL 셋팅된다.
	AI_ACTION_SET_TARGET_GLOBAL_VARIABLE,	// [GlobalValueString]	현제 Search 된 Target 의 포인터를 전역변수(PTR)에 셋팅한다.
	AI_ACTION_SET_TARGET_SLOT,				// [Index(int)] 현제 Search 된 Target을 Slot에 셋팅한다.
	AI_ACTION_EMPTY_SLOT,					// [Index(int)] Slot을 비운다.
	AI_ACTION_LOOK_SLOT,					// [index(int), TRUE,FALSE] 시점을 Slot에 고정/해재
	AI_ACTION_INFLUENCE_TARGETING,			// [TRUE/FALSE]
	AI_ACTION_INFLUENCE_TARGETING_EMPTY,	//
	AI_ACTION_INFLUENCE_TARGETING_SLOT,		// [TRUE/FALSE]
	AI_ACTION_INFLUENCE_DAMAGE_EMPTY,		// 
	AI_ACTION_INFLUENCE_HIT_COUNT_EMPTY,	//

	AI_ACTION_CMD_ATTACK,				// 타겟 공격
	AI_ACTION_CUSTOM_MOVE,				// [AniAttr(int), AniType(int), AniIndex(int), speed(float), Angle(int), Length(int)]
										// 자신과 타겟의 방향을 기준으로 오른쪽으로 Angle 만큼 돌린 방향의 Length만큼의 거리로 이동
	AI_ACTION_SELF_CUSTOM_MOVE,			// [AniAttr(int), AniType(int), AniIndex(int), speed(float), Angle(int), Length(int)] 
										// 자기 자신위치에서 오른쪽으로  Angle 만큼 돌린 방향의 Length 만큼의 거리로 이동한다.
	AI_ACTION_EXIT,						// 하위 Element 체크 무시.. 빠져나온다.

	AI_ACTION_PUSH_BUTTON,				// [Button Index] 버튼을 눌러라~ 히어로 컨트롤을 위해
	AI_ACTION_RELEASE_BUTTON,			// [Button Index] 버튼을 뗘라~ 히어로 컨트롤을 위해

	AI_ACTION_ADD_DEFENSE_PROB,			// [Value] 디펜스 확률 올림 %당.. 
	AI_ACTION_DESTROY_ELEMENT,			// [Index] 엘리먼트 영구 삭제.. Index 가 -1 이면 자기자신
	AI_ACTION_LINK_OBJECT,				// 링크~
	AI_ACTION_UNLINK_OBJECT,			// 언링크~
	AI_ACTION_SET_ARCHER_HIT_PROB,		// [Value] 아쳐일 경우 명중률 셋팅
	AI_ACTION_GLOBAL_VARIABLE_INT_COUNTER,	// [GlobalValueString, Value] 전역변수 증감
	AI_ACTION_GLOBAL_VARIABLE_INT_RANDOM2,	// [GlobalValueString, Value1, Value2, Value1Prob]
	AI_ACTION_GLOBAL_VARIABLE_INT_RANDOM3,	// [GlobalValueString, Value1, Value2, Value3, Value1Prob, Value2Prob]
	AI_ACTION_GLOBAL_VARIABLE_INT_RANDOM4,	// [GlobalValueString, Value1, Value2, Value3, Value4, Value1Prob, Value2Prob, Value3Prob]
	AI_ACTION_SET_AICIDX,				// [Value]
	AI_ACTION_PAUSE_FUNCTION,			// [Function String]
	AI_ACTION_UNPAUSE_FUNCTION,			// [Function String]
	AI_ACTION_RESET_MOVE_VECTOR,		//

	// 부대 액션
	AI_ACTION_FOLLOW_TROOP,				// [AniAttr(int), AniType(int), AniIndex(int), speed(float)] 부대위치로 이동

	AI_ACTION_DEBUG_STRING,
	AI_ACTION_DELAY_ORDER,				 
	AI_ACTION_WARP,						 
	AI_ACTION_DIMINUTION_VELOCITY,		// [Value(float)] 밀리는거 감쇄
};

// 일단 CFcParamVariable 에 잇는거 복사해논다.. 
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

// Key Define.. Input.h 에 있는거 그대루 복사..
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
// Troop State Define FcTroopObject.h 에 있는거 그대루 복사..
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
	TROOPSTATE_MELEE_READY,			// 부대가 melee를 위해 겹쳐지고 있는 상태
	TROOPSTATE_HOLD,				// 안 움직이고 적이 오면 공격
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
	PROP_TYPE_TOWER,		// 망루
	PROP_TYPE_BRIDGE,
	PROP_TYPE_GATE,
	PROP_TYPE_ENABLE_ATTACK_WALL,
	PROP_TYPE_TRAP,
	PROP_TYPE_BILLBOARD,
	PROP_TYPE_PICKUP,
	PROP_TYPE_CRUMBLE,		// 무너지면서 길을 막는 녀석
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

