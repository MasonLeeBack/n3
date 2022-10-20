#pragma		once

#include "BStreamExt.h"

void	ResetProcessTick();
int		GetProcessTick();
float	GetProcessTime();
bool	IsPause();

int GetGuardianType(int nUnitSoxId);

#define MAX_GUARDIAN_UNIT_NUM	50
#define MAX_TRG_VAR_NUM			16
#define MAX_GUARDIAN_SLOT_NUM	2
#define FRAME_PER_SEC			60

#define _SCREEN_WIDTH			1920
#define _SCREEN_HEIGHT			1080

#define _MAX_USER_SAVE_DATA_COUNT		50
#define _MAX_LOADSAVE_DATA_COUNT		_MAX_USER_SAVE_DATA_COUNT + 1

#define _TOTAL_POINT_ADD_RANK_S				5
#define _TOTAL_POINT_ADD_RANK_A				4
#define _TOTAL_POINT_ADD_RANK_B				3
#define _TOTAL_POINT_ADD_RANK_C				2
#define _TOTAL_POINT_ADD_RANK_D				1
#define _TOTAL_POINT_ADD_RANK_F				0

class ASSignalData;
enum MISSION_FINISH_TYPE;

#define _WAIT_TICK_PAUSEMENU	60

#define _LIMIT_LEVEL			9
#define _LIMIT_EXP				999999

//#define _START_DEBUG_MENU
//#define _USE_TGS_TYPE_MENU
//#define _MENU_ONLY_TEXT

//movie
#define _MGS_LOGO_MOVIE			"MGS_Logo.bik"
#define _MAIN_OPENING_MOVIE_A	"N3_Main_Openinig_A.bik"
#define _MAIN_OPENING_MOVIE_B	"N3_Main_Openinig_B.bik"
#define _MAIN_BACKGROUND_MOVIE	"N3_Menu_Orb.bik"

//char openinig movie
#define _OPENING_MOVIE_INPHYY		"Inphyy.bik"
#define _OPENING_MOVIE_ASPHARR		"Aspharr.bik"
#define _OPENING_MOVIE_MYIFEE		"Myifee.bik"
#define _OPENING_MOVIE_TYURRU		"Tyurru.bik"
#define _OPENING_MOVIE_DWINGVATT	"Dwingvatt.bik"
#define _OPENING_MOVIE_VIGKVAGK		"VigkVagk.bik"

//staff roll
#define _STAFF_ROLL_MOVIE			"N3_staff.bik"

#define _TGS_LOADING			"TGS_Loading.dds"
#define _TGS_ASPHARR_MISSION	"TGS(0908).bsmap"
#define _TGS_INPHYY_MISSION		"TGS(0908).bsmap"

#define _LOADING_MISSION_LOGO		"mn_loading_logo"
#define _LOADING_MISSION_TEXT		"mn_loading"
#define _LOADING_MISSION_INPHYY		"mn_loading_in"
#define _LOADING_MISSION_ASPHARR	"mn_loading_as"
#define _LOADING_MISSION_MYIFEE		"mn_loading_my"
#define _LOADING_MISSION_TYURRU		"mn_loading_ty"
#define _LOADING_MISSION_KLARRANN	"mn_loading_kl"
#define _LOADING_MISSION_DWINGVATT	"mn_loading_dw"
#define _LOADING_MISSION_VIGKVAGK	"mn_loading_vg"

#define _OPENING_MOVIE_A		"N3_Main_Openinig_A.bik"
#define _PHANTA_Q_LOGO			"Logo_Dev.dds"

#define _STAGE_TUTORIAL_FILENAME	"dm_mp.bsmap"
#define _MAX_BINK_VOLUME	32768

#define _MAX_LEVEL_COUNT			9 // 1 - 9
#define _DASHBOARD_UTF16_FILENAME	"DashBoardUTF16"

#define _KILL_EFFECT_COUNT		6
#define _COMBO_EFFECT_COUNT		6
//#define _TEST_KILL_COUNT_FX_FILENAME
//#define _TEST_COMBO_COUNT_FX_FILENAME

enum
{
	SPECIAL_STAGE_MENU_WAIT,
	SPECIAL_STAGE_MENU_ON,
	SPECIAL_STAGE_MENU_OFF,
};

enum GAME_STATE
{
	GAME_STATE_NON = -1,

	//title menu
	GAME_STATE_INIT_TITLE,
	GAME_STATE_LOOP_TITLE,

	//stage
	GAME_STATE_INIT_STAGE,
	GAME_STATE_LOOP_STAGE,
	
	//pause
	GAME_STATE_INIT_PAUSE,
	GAME_STATE_LOOP_PAUSE,

	//stage result Sign
	GAME_STATE_INIT_RESULT_SIGN,
	GAME_STATE_LOOP_RESULT_SIGN,

	//stage result
	GAME_STATE_INIT_RESULT,
	GAME_STATE_LOOP_RESULT,

	//예외처리
	GAME_STATE_INIT_NEXT_STAGE,
	GAME_STATE_LOOP_NEXT_STAGE,

	GAME_STATE_SYSTEM_MESSAGE,
};

enum GAME_MISSION_TYPE
{
	MISSION_TYPE_NONE		= -1,
	MISSION_TYPE_INPHYY,
	MISSION_TYPE_ASPHARR,
	MISSION_TYPE_MYIFEE,
	MISSION_TYPE_TYURRU,
	MISSION_TYPE_KLARRANN,
	MISSION_TYPE_DWINGVATT,
	MISSION_TYPE_VIGKVAGK,

	MAX_MISSION_TYPE,
};

#define _LIGHT_TROOP_COUNT			5

//***GAME_STAGE_ID의 문제점***
//다음과 같은 예외처리가 되어 있다.
//기획의 후반 변경으로 CHAR_ID_MYIFEE, CHAR_ID_DWINGVATT의 경우는 예외로 처리된다.
//						사용케릭터		Text			배경이미지						이름이미지
//STAGE_ID_VARRVAZZAR  -> 정상작동:		발바잘성		mn_worldmap_vz.dds				mn_AN_vz
//					   -> 미피, 딩그밧	아이스게이트	mn_worldmap_vz_entrance.dds		mn_AN_vz_entrance
//STAGE_ID_VARRVAZZAR_2-> 딩그밧		발바잘성		mn_worldmap_vz.dds				mn_AN_vz

//위의 경우가 적용된 경우는 다음과 같다.
//1. LibraryMission.xls->sox
//2. briefing_(MissionID).txt
//3. GetStageIDforName(..)
//4. CFcMenuMissionDB::UpdateStageLayer(..)

enum GAME_STAGE_ID
{
	STAGE_ID_NONE			= -1,

	STAGE_ID_VARRGANDD		= 0,	//바르간다
	STAGE_ID_OUT_VARRGANDD,			//바르간다 외곽
	STAGE_ID_VARRFARRINN,			//발파린
	STAGE_ID_OUT_VARRFARRINN,		//발파린 외곽
	STAGE_ID_VARRVAZZAR,            //발바잘 후반
	STAGE_ID_OUT_VARRVAZZAR,		//발바잘 외곽
	STAGE_ID_WYANDEEK,				//완디크
	STAGE_ID_WYANDEEK_VILLAGE,		//완디크 근방의 고블린 마을
	STAGE_ID_YWA_UE_UAR,			//야페왈 숲
	STAGE_ID_EAURVARRIA,			//아바리아
	STAGE_ID_FELPPE,				//헬페마을
	STAGE_ID_PHOLYA,				//포리아 평원
	STAGE_ID_BEFORE_PHOLYA_1,		//포리아 평원 가기전1
	STAGE_ID_BEFORE_PHOLYA_2,		//포리아 평원 가기전2
	STAGE_ID_ANOTHER_WORLD,			//마계
	STAGE_ID_VARRVAZZAR_2,

	//world map상에 나타나지 안는다.
	STAGE_ID_TUTORIAL,				//tutorial

	STAGE_ID_ENDING_MYIFEE,			//미피 엔딩맵
	STAGE_ID_ENDING_DWINGVATT,		//딩그바트 엔딩맵
	STAGE_ID_ENDING_VIGKVAGK,		//비그바그 엔딩맵
	STAGE_ID_ENDING_MA_INPHYY,		//인피 마계 엔딩맵

	MAX_STAGE,
};


enum CHAR_ID
{
	CHAR_ID_NONE		= -1,

	CHAR_ID_ASPHARR,
	CHAR_ID_INPHYY,
	CHAR_ID_TYURRU,
	CHAR_ID_KLARRANN,
	CHAR_ID_MYIFEE,
	CHAR_ID_VIGKVAGK,
	CHAR_ID_DWINGVATT,

	CHAR_ID_MAX,
};


enum CHAR_TYPE_ID
{
	C_NONE			= -1,
	C_LP_KM,
	C_LP_KF,
	C_LP_MF,
	C_LP_PM,
	C_LP_WM,
	C_DP_TR,
	C_DP_GB,
	C_LS_SL1_OH,
	C_LS_SL1_SP,
	C_LS_SL2_OH,
	C_LS_SL2_SP,
	C_LS_SL3_OH,
	C_LS_SL3_SP,
	C_LS_SL4_AR,
	C_DS_OC4_OH,
	C_DS_OC4_TH,
	C_DS_OC5_OH,
	C_DS_OC5_TH,
	C_DS_OC7_OH,
	C_DS_OC7_TH,
	C_DS_OC8_OH,
	C_DS_OC8_TH,
	C_DS_OC1_OH,
	C_DS_OC1_TH,
	C_DS_OC2_OH,
	C_DS_OC2_TH,
	C_DS_DG,
	C_DS_EF2_OH,
	C_DS_GB1_OH,
	C_DS_GB1_SP,
	C_DS_GB1_AR,
	C_DS_EF1_OH,
	C_DS_CAT,
	C_DS_SEI,
	C_DS_GBC,
	C_LN_FR,
	C_LN_KB1,
	C_LN_KB2,
	C_LS_SL1_TH,
	C_LS_SL2_TH,
	C_LS_SL3_TH,
	C_LS_SL4_TH,
	C_LN_FM1,
	C_LN_DK1,
	C_DN_GK2,
	C_LN_CL1,
	C_LN_BA1,
	C_LN_EF1,
	C_DN_GT11,
	C_DN_GT10,
	C_DN_GT5,
	C_DN_GT61,
	C_DN_GT71,
	C_DN_GT81,
	C_LN_CT0,
	C_LN_CT1,
	C_LN_MF1,
	C_LN_CT3,
	C_LN_CT4,
	C_DUMMY1,
	C_LN_CT6,
	C_LN_CT7,
	C_DS_GB2_OH,
	C_DS_GB2_SP,
	C_DS_GB2_AR,
	C_DS_9S1,
	C_DS_KR1,
	C_DS_KR2,
	C_DN_OC1,
	C_LS_WR1_OH,
	C_LS_WR1_TH,
	C_DS_LZ1,
	C_DS_LZ2,
	C_DS_LZ3,
	C_LS_EM1,
	C_DS_TR1,
	C_DN_GK1,
	C_DN_9K1,
	C_DN_DE1,
	C_DN_KT1,
	C_LN_ME1,
	C_LN_KN1,
	C_LN_KN2,
	C_LN_KN3,
	C_DS_GBC_OH,
	C_LS_SL_EX1_OH,
	C_LS_SL_EX2_OH,
	C_LS_SL_EX3_OH,
	C_LS_SL_EX4_AR,
	C_DS_GB3,
	C_DS_GB4,
	C_DS_OC3,
	C_DN_GT9,
	C_DS_OC9,

	MAX_CHAR_TYPE,
};

enum GUARDIAN_TYPE
{
	GT_NONE			= -1,

	GT_ONE_HAND,
	GT_TWO_HAND,
	GT_SPEAR,
	GT_RANGE,

	GT_MAX,
};

enum LIBRARY_SECTION
{
	LIB_CHAR,
	LIB_MOVIE,
	LIB_MISSION,
	LIB_ART,
	LIB_MUSIC,

	LIB_MAX_COUNT,
};

enum LIBRARY_ITEM_STATUS
{
	LIS_DEFAULT,
	LIS_NEW,
	LIS_OPEN,
};

//-----------------------------------------------------
// Hero equipment infomation
typedef struct _HeroEquip
{
	_HeroEquip()
	{
		nItemSoxID = -1;
		bAccoutered = false;
		bNew = false;
		bStopEffect = false;
	};

	int		nItemSoxID;		// Equipment id
	BOOL	bAccoutered;	// 가지고 있는 것들중에 착용상태.
	BOOL	bNew;			// Mission 상에서 추가 된 아이템.
	BOOL	bStopEffect;	// effect 발동 되었는지 check.
} HeroEquip;

// Hero equipment infomation
typedef struct _GuardianTroop
{
	_GuardianTroop()
	{
		clear();
	};

	void clear()
	{
		nID = -1;
		nWidth = 0;
		nHeight = 0;
		nSpace = 0;
	};

	int		nID;			// unit sox id
	int		nWidth;			// 부대 가로 size
	int		nHeight;		// 부대 세로 size
	int		nSpace;			// 부대 간격
} GuardianTroop;

//user saved game data
typedef struct _UserSaveData
{
public:
	_UserSaveData()
	{
		Clear();
	};

	void Clear()		// 타이틀로 돌아오면 Clear를 불러준다.
	{	
		nHeroID = -1;
		nLevel = 0;
		nExp = 0;
		nCurStageID = STAGE_ID_NONE;
		nSpeaialSelStageID = STAGE_ID_NONE;

		memset(nTrgGlobalVar,-1,sizeof(int) * MAX_TRG_VAR_NUM);

		GuardianInfo[0].clear();
		GuardianInfo[1].clear();

		ClearStageList.clear();
		EquipList.clear();
	};
	
	int GetSaveDataSize();	// 세이브되는 데이타 변경시 항상 검토해 주세요.
	void Save(BStream *pStream);
	bool Load(int nVersion, BStream *pStream);
	bool Load_Version19(BStream *pStream) { return Load_Version15(pStream); }
	bool Load_Version18(BStream *pStream) { return Load_Version15(pStream); }
	bool Load_Version17(BStream *pStream) { return Load_Version15(pStream); }
	bool Load_Version16(BStream *pStream) { return Load_Version15(pStream); }
	bool Load_Version15(BStream *pStream);
	bool Load_Version14(BStream *pStream);
	bool Load_Version13(BStream *pStream);
	bool Load_Version12(BStream *pStream);
	bool Load_Version11(BStream *pStream);
	bool Load_Version10(BStream *pStream);

	int							nHeroID;				//unit sox id
	int							nLevel;
	int							nExp;
	int							nCurStageID;			//current stage ID
	int							nSpeaialSelStageID;		//aspharr special 분기 stage

	int							nTrgGlobalVar[MAX_TRG_VAR_NUM];			//Trigger에서 사용하는 변수 List
	GuardianTroop				GuardianInfo[MAX_GUARDIAN_SLOT_NUM];

	std::vector<int>			ClearStageList;			//clear한 stage ID
	std::vector<HeroEquip>		EquipList;				//item list
	
	
} UserSaveData;


//-----------------------------------------------------------
//Stage Result infomation
enum PRIZE_ITEM {
	PRIZE_ITEM_RANKLESS = 0,
	PRIZE_ITEM_RANK_S,
	PRIZE_ITEM_RANK_A,
	PRIZE_ITEM_MAX,
};
struct StageResultInfo
{
	StageResultInfo()
	{
		Clear();
	};

	void Clear()
	{
		nStageID = -1;
		nClearTime = 356400;
		nKillCount = 0;
		nNormalKillCount = 0;
		nOrbSparkKillCount = 0;
		nGetOrbEXP = 0;
		nMaxCombo = 0;
		nGuardianLiveRate = 0;
		nMissionClearCount = 0;
		nMissionFaultCount = 0;
		nBonusPoint = 0;
		nHeroLevel = 0;
		nPlayGrade = 5;
		for(int i=0; i<PRIZE_ITEM_MAX; ++i)
			nSpecialItem[i] = -1;		
		nEnemyLevelUp = 0;
		nGuardianOrbs=0;
	};

	int nStageID;					//GAME_STAGE_ID
	int nClearTime;
	int nKillCount;
	int nNormalKillCount;
	int nOrbSparkKillCount;
	int nGetOrbEXP;
	int nMaxCombo;
	int nGuardianLiveRate;
	int nMissionClearCount;
	int nMissionFaultCount;
	int nBonusPoint;
	int nPlayGrade;
	int nHeroLevel;
	int nSpecialItem[PRIZE_ITEM_MAX];				//S Rank로 clear했을때 보상으로 얻어지는 Item
	int nEnemyLevelUp;				//A Rank이상으로 clear하면 다음번에 Value만큼 enemy level이 올라간다.
	int nGuardianOrbs;	// 가디언들의 오브수를 저장해 둔다.
};


typedef struct _HeroRecordInfo
{
	_HeroRecordInfo()
	{
		nHeroID = -1;
		bAllStageClear = false;
	};

	void	Save(BStream *pStream);
	void	Load(int nVersion, BStream *pStream);
	void	Load_Version19(BStream *pStream)			{ Load_Version16(pStream); }
	void	Load_Version18(BStream *pStream)			{ Load_Version16(pStream); }
	void	Load_Version17(BStream *pStream)			{ Load_Version16(pStream); }
	void	Load_Version16(BStream *pStream);
	void	Load_Version15(BStream *pStream)			{ Load_Version13(pStream); }
	void	Load_Version14(BStream *pStream)			{ Load_Version13(pStream); }
	void	Load_Version13(BStream *pStream);
	void	Load_Version12(BStream *pStream)			{ Load_Version13(pStream); }
	void	Load_Version11(BStream *pStream)			{ Load_Version13(pStream); }
	void	Load_Version10(BStream *pStream)			{ Load_Version13(pStream); }

	StageResultInfo*	GetStageResultInfo(int nStageID);

	int								nHeroID;				//sox id
	bool							bAllStageClear;

	std::vector<StageResultInfo>	StageResultList;
} HeroRecordInfo;


typedef struct _LibraryItem
{
	_LibraryItem()
	{
		nID = -1;
		nState = LIS_DEFAULT;
	};

	int		nID;
	int		nState;
} LibraryItem;


typedef struct _FixedSaveData
{
public:
	_FixedSaveData()
	{
		Clear();
	};

	void Clear()		// sign이 바뀌면 Clear해 준다.
	{
		nTotalPoint = 0;
		bOpenBonusStage = false;
		bGoodEndingforAspharr = false;

		HeroRecordList.clear();
		LibraryItemList.clear();
	};

	int GetSaveDataSize();	// 세이브되는 데이타 변경시 항상 검토해 주세요.

	void Save(BStream *pStream);
	void Load(int nVersion, BStream *pStream);
	void Load_Version19(int nVersion, BStream *pStream);
	void Load_Version18(int nVersion, BStream *pStream);
	void Load_Version17(int nVersion, BStream *pStream)			{ Load_Version14(nVersion, pStream); }
	void Load_Version16(int nVersion, BStream *pStream)			{ Load_Version14(nVersion, pStream); }
	void Load_Version15(int nVersion, BStream *pStream)			{ Load_Version14(nVersion, pStream); }
	void Load_Version14(int nVersion, BStream *pStream);
	void Load_Version12(int nVersion, BStream *pStream);
	void Load_Version11(int nVersion, BStream *pStream)			{ Load_Version12(nVersion, pStream); }
	void Load_Version10(int nVersion, BStream *pStream)			{ Load_Version12(nVersion, pStream); }

	HeroRecordInfo*		GetHeroRecordInfo(int nHeroID);
	LibraryItem*		GetLibraryData(int nItemID);
	
	//stage infomation list						
	int			nTotalPoint;								//Library 상에서 쓰이는 Bonus Point 누적치;
	bool		bOpenBonusStage;							//마계가 열렸는지 check;
	bool		bGoodEndingforAspharr;						//good ending이고 모든 캐릭을 clear해야 마계가 열린다.
	
	std::vector<HeroRecordInfo>		HeroRecordList;			//Hero별로 갱신된 최고 기록을 저장
	std::vector<LibraryItem>		LibraryItemList;
} FixedSaveData;

typedef struct _SavedDataInfo
{
	_SavedDataInfo()
	{
		nYear = nMonth = nDay = 0;
		nHour = nMin = nSec = 0;
		nHeroID = -1;
		nLevel = 0;
		nStageID = -1;
	};

	int	nYear, nMonth, nDay;
	int nHour, nMin, nSec;

	int nHeroID;
	int nLevel;
	int nStageID;

	int nSlot;
} SavedDataInfo;

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------

typedef struct _Config
{
	_Config()
	{
		bVibration	 = true;
		nBGMVolume	 = 10;
		nSoundVolume = 10;
		nVoiceVolume = 10;
		bCaption = true;
		bCameraUD_Reverse = false;
		bCameraLR_Reverse = false;
		nContrast = 5;
	};

	//option
	bool							bVibration;
	int								nBGMVolume;				//1 - 10 단계
	int								nSoundVolume;			//1 - 10 단계
	int								nVoiceVolume;			//1 - 10 단계
	bool							bCaption;
	bool							bCameraUD_Reverse;		//camera up down reverse
	bool							bCameraLR_Reverse;		//camera lefr right reverse
	int								nContrast;				//1 - 10 단계
	
} Config;

// 처음부터 끝까지 유지해야 하는 데이터들

// debuging code
class CSaveDebugFlag {
public:
	bool bShowDebugArrow;
	bool bShowNavigationMesh;
	bool bShowCollisionMesh;
	bool bEnableUnitAI;
	bool bShowTroopState;
	bool bShowUnitState;
	bool bGod;
	bool bSaveInput;
	bool bMenuShow;
	bool bDebugRenderBaseObject;
	bool bDebugRenderTroopObject;
	bool bShowFriendlyGauge;
	bool bShowEnemyGauge;
	bool bShowMinimapAllObj;
	int	 nShowTroopGaugeForFar;
	bool bFastMode;
	bool bPlayMovie;
	bool bShowVariable;
	bool bOneShotMode;
	int nSkipMode;
	int nCapture;
	bool bShowOrbs;
	bool bNoDamage;
	bool bDrawProp;
	bool bShowHeroPos;
	bool bCharAllOpen;
	char cReserved[4072]; // 4096 - XX
	bool bCrashAll;
	bool bShowSafeArea;
	bool bShowAttr;

	CSaveDebugFlag() {
		bShowDebugArrow = false;
		bShowNavigationMesh = false;
		bShowCollisionMesh = false;
		bEnableUnitAI = true;
		bShowTroopState = false;
		bShowUnitState = false;
#ifdef _LTCG
		bSaveInput = false;
#else
		bSaveInput = false;
#endif
		bMenuShow = true;
		bDebugRenderBaseObject = false;
		bDebugRenderTroopObject = false;
		bShowFriendlyGauge = false;
		bShowEnemyGauge = false;
		bShowMinimapAllObj = false;
		nShowTroopGaugeForFar = 5000;
		bFastMode = false;
		bPlayMovie = true;
		bShowVariable = false;
		bOneShotMode = false;
		nSkipMode = 0;
		nCapture = 0;
		bShowOrbs = false;
		bNoDamage = false;
		bDrawProp = true;
		bCharAllOpen = false;
		memset( cReserved, 0, sizeof(cReserved) );
		bCrashAll = false;
		bShowSafeArea = false;	
		bShowAttr = false;
	}

	~CSaveDebugFlag() {}
};

struct SAV_FILE_HEADER
{
	int nVersion;
	char szHeaderString[ 256 ];
	int nBodySize;	// 헤더 사이즈를 제외한 바디 사이즈
};

#pragma 

#define SAV_FILE_VERSION_LAST	19
#define SAV_FILE_VERSION_18		18
#define SAV_FILE_VERSION_17		17
#define SAV_FILE_VERSION_16		16
#define SAV_FILE_VERSION_15		15
#define SAV_FILE_VERSION_14		14
#define SAV_FILE_VERSION_13		13
#define SAV_FILE_VERSION_12		12
#define SAV_FILE_VERSION_11		11
#define SAV_FILE_VERSION_10		10
#define SAV_FILE_HEADER_STRING	"Game Save File Version 0.18"
#define SAV_FILE_NAME			"%s_%04d.sav"
#define SAV_FILE_NAME_W			L"%S"
#define _FIXED_SAVE_FILENAME	"FixedSave.sav"

const int FIXEDSAVEDATA_ENOUGHSIZE = 245760;// 유저 프로필 FIXED SAVE DATA 파일 240KB
const int STORAGE_ENOUGHSIZE	= 307200; // 저장시 필요한 최소 스토리지 공간 240kb+60kb
const int FIRSTCREATE_ENOUGHSIZE = 73728; // 최초 파일 생성시 요구되는 폴더 생성 용량 72kb

enum _SAVED_FILE_FORMAT
{
	SFF_FIXED_GAME_DATA,
	SFF_USER_GAME_DATA,
};

#ifndef _XBOX
struct XCONTENT_DATA {
};
#define XCONTENTDEVICE_ANY 0
#endif //_XBOX

class CFCGAMEDATA : public CSaveDebugFlag
{
public:
	CFCGAMEDATA();
	~CFCGAMEDATA();

	void GetSaveDirectory( char *pSaveDir, int nBufferSize );
	void LoadDashBoardCaption();

	// Fixed Save function
	void SaveFixedGameData();
	bool LoadFixedGameData();
	
	// user Save function
	bool SaveUserGameData( XCONTENT_DATA* pContentData );
	bool LoadUserGameData( XCONTENT_DATA* pContentData );
	bool LoadUserGameInfo( UserSaveData* ptempSaveData, XCONTENT_DATA* pContentData );
	void GetSavedFileInfo(XCONTENT_DATA* pContentData, SavedDataInfo* pSavedDataInfo);
	bool DeleteGameData( XCONTENT_DATA* pContentData );
	
	// save content info
	bool UpdateGetContentData();
	XCONTENT_DATA* GetContentData(int nIndex);
	void ShowDeviceUI();
	bool IsFixedSavedGame()					{ return bFixedSaveData; }
	int GetUserSaveGameCount();
	bool IsUserSavedGame( int nIndex );

	//
	void AutoSave();
	bool RollBack();	

	//void LoadConfig();
	//void SaveConfig();

	void SaveSet( int nIndex );
	void LoadSet( int nIndex );

	void Clear();
	ASSignalData* GetDefaultTrapHitSignal( int nDamage );
	ASSignalData* GetDefaultPhysicsHitSignal( int nDamage );
	void GetSaveFileName(char* szFileName, const size_t szFileName_Len, WCHAR* szCaption);
	int GetStageIDforName(int nStageID, int nHeroSoxID);
	void GetStageName(int nStageID, char* szName, const size_t szName_Len, int nHeroSoxID);

protected:
	void SaveGameData(int nState, XCONTENT_DATA* pContentData);
	bool LoadGameData(int nState, FixedSaveData* pFixedSave, UserSaveData* pUserSave, XCONTENT_DATA* pContentData);
	int  GetDataInfo(char* pSrcText, char* pDscText, const size_t szDsc_Len);
	int GetSlotNum(const std::string &filename) const;

public:
#ifdef _XBOX
	XOVERLAPPED     m_Overlapped;                  // Overlapped object for device selector UI
	XCONTENTDEVICEID m_DeviceID;                   // Device identifier returned by device selector UI
	XCONTENTDEVICEID m_OldDeviceID;                   
#else
	int m_DeviceID;
#endif //_XBOX
 
	Config				ConfigInfo;

	bool				bPause;

	int					nPlayerCount;				//count는 1...
	int					nEnablePadID;

	int					nPlayerType;				//GAME_MISSION_TYPE
	GuardianTroop		GuardianInfo[MAX_GUARDIAN_SLOT_NUM];

	GAME_STAGE_ID		PrevStageId;				//이전의 Stage id - 미션이 끝나면 update
	GAME_STAGE_ID		SelStageId;					//현재의 Stage id

	//stage내에서 다음 stage결정한다. 예외처리
	int					nSpecialSelStageState;		//trigger와 통신한다
	GAME_STAGE_ID		SpeaialSelStageId;			//World에서 분기에 영향을 준다.
	GAME_STAGE_ID		SpecialSelStageIdforTrigger;			//trigger와 통신한다. (Stage ID)

	int					nPlayerSetting;
	bool				bOpenLogo;

	bool				bCameraToggle;

	//temp data
	StageResultInfo		stageInfo;
	MISSION_FINISH_TYPE nMissionFinish;
//	bool				bFinishMovie;				//stage 종료시 결과 sign후 결과로 이동/sign후 movie후 결과 이동 : true / false
	char				cFinishRealtimeMovieFileName[32];

	GAME_STATE State;
	char cMapFileName[64];
	unsigned long lSaveRandomSeed;

	char				cLastFailMissionName[64];
	int					nNumMissionFail;		// 연속으로 똑같은 미션을 몇번 실패했는지

	int					nTrgGlobalVar[MAX_TRG_VAR_NUM];	//user save data; // title 로 가면 초기화.

	char cNextStageMapFileName[64];		// 아바리아 같은 경우 다음 미션이 나눠지는 경우가 있다.

	//save data
	FixedSaveData	tempFixedSave;
	UserSaveData	tempUserSave;
	
	bool			bShowDeviceUI; // false 면 device selector 를 띄움
	bool			bOnDeviceUI;
	bool			bChangeDevice;
	bool			bCancelDeviceSelector;
	bool			bRemovedStorageDevice;
	bool			bCheckFreeSpace;
	int				nSaveDataSize;
	DWORD			nSaveGameCount;
	bool			bFixedSaveData;
	bool			bChangeProfile;
	bool			bLockAchievement;
	bool			bFirstInitProfile;

	char			szProfileName[128];
	VOID*			pDashBoard;

	bool			bCheatRank;

	BBufStream		AutoSaveBuf;
#ifdef _XBOX
	XCONTENT_DATA   m_ContentData[_MAX_LOADSAVE_DATA_COUNT];  // Data containing display names
#endif //_XBOX

};


extern CFCGAMEDATA g_FCGameData;
extern int g_nProcessTick;
extern int g_nWaitTickPauseMenu;
extern int g_MissionToSoxID[MAX_MISSION_TYPE];	// GAME_MISSION_TYPE : sox id
extern int g_SoxToMissionID[CHAR_ID_MAX];		// sox id : GAME_MISSION_TYPE
extern int g_LightTroopSoxID[_LIGHT_TROOP_COUNT];
extern int g_NewCharEnableCondition[MAX_MISSION_TYPE][MAX_MISSION_TYPE];
extern int g_LoadingLV[MAX_MISSION_TYPE][5][2];
extern int g_AccessorySlotCountByLevel[_MAX_LEVEL_COUNT];
extern int g_NewSkillforLevel[CHAR_ID_MAX][_MAX_LEVEL_COUNT];
extern int g_MissionToLibCharInfo[MAX_MISSION_TYPE];

extern int g_ComboEffectUnit[_COMBO_EFFECT_COUNT];
extern int g_KillEffectUnit[_KILL_EFFECT_COUNT];


bool IsPlayRealMovie();
bool IsPlayEvent();
void LoadDashBoardCaption();


// Default로 Title화면으로 시작합니다. 디버그메뉴시작은 Command Arguments에 "startdebugmenu"를 넣으세요!!
extern BOOL g_bIsStartTitleMenu;
