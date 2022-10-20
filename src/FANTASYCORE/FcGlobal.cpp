#include "stdafx.h"

#ifdef _XBOX
#include <xtl.h>
#include "AtgSignin.h"
#endif //_XBOX

#include "FcGlobal.h"
#include "ASData.h"
#include "bstreamext.h"
#include ".\\data\\SignalType.h"
#include "Parser.h"
#include "LocalLanguage.h"
#include "FcWorld.h"
#include "InputPad.h"
#include "FcSoundManager.h"

#include "FcInterfaceManager.h"
#include "FcRealtimeMovie.h"
#include "FcItem.h"
#include "TextTable.h"
#include "BsMoviePlayer.h"
#include "FcGetCaptionUTF16.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

CFCGAMEDATA g_FCGameData;

ASSignalData g_DefaultHitSignal;

int g_nProcessTick = 0;
int g_nWaitTickPauseMenu = 0;
bool g_bPause = false;
int g_nPauseRef = 0;
LARGE_INTEGER	g_liStartPauseTime;
LARGE_INTEGER	g_liStagePauseTime;

//GAME_MISSION_TYPE와 대칭되는 unit sox id
//1, 0, 4, 2, 3, 6, 5,
int g_MissionToSoxID[MAX_MISSION_TYPE] = {
	CHAR_ID_INPHYY,
	CHAR_ID_ASPHARR,
	CHAR_ID_MYIFEE,
	CHAR_ID_TYURRU,
	CHAR_ID_KLARRANN,
	CHAR_ID_DWINGVATT,
	CHAR_ID_VIGKVAGK,
};

//unit sox id와 대칭되는 GAME_MISSION_TYPE
//1, 0, 4, 2, 3, 6, 5,
int g_SoxToMissionID[CHAR_ID_MAX] = {
	MISSION_TYPE_ASPHARR,
	MISSION_TYPE_INPHYY,
	MISSION_TYPE_TYURRU,
	MISSION_TYPE_KLARRANN,
	MISSION_TYPE_MYIFEE,
	MISSION_TYPE_VIGKVAGK,
	MISSION_TYPE_DWINGVATT,
};

int g_LightTroopSoxID[_LIGHT_TROOP_COUNT] = {
	CHAR_ID_INPHYY,
	CHAR_ID_ASPHARR,
	CHAR_ID_MYIFEE,
	CHAR_ID_TYURRU,
	CHAR_ID_KLARRANN,
};

//hero(GAME_MISSION_TYPE)별로 clear해야만 선택이 가능한 mission list
int g_NewCharEnableCondition[MAX_MISSION_TYPE][MAX_MISSION_TYPE] = {
	{ -1, -1, -1, -1, -1, -1, -1 },																				//MISSION_TYPE_INPHYY, 인피
	{ MISSION_TYPE_INPHYY, -1, -1, -1, -1, -1, -1 },	// + (인피의)STAGE_ID_WYANDEEK clear					//MISSION_TYPE_ASPHARR, 아스파
	{ MISSION_TYPE_INPHYY, -1, -1, -1, -1, -1, -1 },	// + (인피의)STAGE_ID_PHOLYA clear						//MISSION_TYPE_MYIFEE, 미피
	{ MISSION_TYPE_INPHYY, MISSION_TYPE_ASPHARR, MISSION_TYPE_MYIFEE, MISSION_TYPE_DWINGVATT, -1, -1, -1 },		//MISSION_TYPE_TYURRU, 튜루르 
	{ MISSION_TYPE_INPHYY, MISSION_TYPE_ASPHARR, MISSION_TYPE_MYIFEE, MISSION_TYPE_DWINGVATT, -1, -1, -1 },		//MISSION_TYPE_KLARRANN, 카라랑
	{ MISSION_TYPE_INPHYY, MISSION_TYPE_ASPHARR, MISSION_TYPE_MYIFEE, -1, -1, -1, -1 },							//MISSION_TYPE_DWINGVATT, 딩그밧
	{ MISSION_TYPE_INPHYY, MISSION_TYPE_ASPHARR, MISSION_TYPE_MYIFEE, MISSION_TYPE_DWINGVATT, MISSION_TYPE_TYURRU, MISSION_TYPE_KLARRANN, -1 },	//MISSION_TYPE_VIGKVAGK, 비그바그
};

//Loading에서 사용하는 Level text value
int g_LoadingLV[MAX_MISSION_TYPE][5][2] = {
	{ {1, 1}, {1, 1}, {2, 2}, {4, 4}, {5, 6}, }, //MISSION_TYPE_INPHYY
	{ {1, 1}, {2, 3}, {4, 4}, {5, 6}, {7, 7}, }, //MISSION_TYPE_ASPHARR
	{ {1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 7}, }, //MISSION_TYPE_MYIFEE
	{ {1, 1}, {3, 2}, {2, 3}, {4, 4}, {2, 6}, }, //MISSION_TYPE_TYURRU
	{ {1, 1}, {2, 2}, {1, 3}, {2, 4}, {4, 3}, }, //MISSION_TYPE_KLARRANN
	{ {1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 4}, }, //MISSION_TYPE_DWINGVATT
	{ {1, 3}, {1, 2}, {2, 4}, {3, 4}, {5, 4}, }, //MISSION_TYPE_VIGKVAGK
};

//Level에 따른 Accessory Slot개수
int g_AccessorySlotCountByLevel[_MAX_LEVEL_COUNT] = {
	1, 2, 2, 3, 3, 4, 4, 5, 5,
};

int g_NewSkillforLevel[CHAR_ID_MAX][_MAX_LEVEL_COUNT] = {
//	  1  2  3  4  5  6  7  8  9
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1, }, //CHAR_ID_ASPHARR
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1, }, //CHAR_ID_INPHYY
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1, }, //CHAR_ID_TYURRU
	{ 1, 1, 1, 1, 1, 1, 0, 0, 0, }, //CHAR_ID_KLARRANN	- 7, 8, 9에 추가 스킬이 없다.
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1, }, //CHAR_ID_MYIFEE
	{ 1, 1, 1, 1, 1, 0, 0, 0, 0, }, //CHAR_ID_VIGKVAGK	- 6, 7, 8, 9에 추가 스킬이 없다.
	{ 1, 1, 1, 1, 1, 1, 1, 1, 0, }, //CHAR_ID_DWINGVATT - 9에 추가 스킬이 없다.
};

//MenuLibCharInfo_Local.xls에서 잘못 기입한 관계로 다음과 같이 정정한다.
int g_MissionToLibCharInfo[MAX_MISSION_TYPE] = {
	1, //MISSION_TYPE_INPHYY,				
	0, //MISSION_TYPE_ASPHARR,
	2, //MISSION_TYPE_MYIFEE,
	3, //MISSION_TYPE_TYURRU,
	4, //MISSION_TYPE_KLARRANN,
	5, //MISSION_TYPE_DWINGVATT,
	6, //MISSION_TYPE_VIGKVAGK,
};


#ifdef _TEST_KILL_COUNT_FX_FILENAME
int g_KillEffectUnit[_KILL_EFFECT_COUNT] = { 10, 20, 40, 60, 80, 100 };
#else
int g_KillEffectUnit[_KILL_EFFECT_COUNT] = { 500, 1000, 2000, 3000, 4000, 5000 };
#endif

#ifdef _TEST_COMBO_COUNT_FX_FILENAME
int g_ComboEffectUnit[_COMBO_EFFECT_COUNT] = { 10, 20, 40, 60, 80, 100 };
#else
int g_ComboEffectUnit[_COMBO_EFFECT_COUNT] = { 500, 1000, 2000, 3000, 4000, 5000 };
#endif

static CHAR SAVE_ROOT[] = "save";
static CHAR SAVE_GAMEFILE[] = "save:\\savegame.txt"; // dummy

// Default로 Title화면으로 시작합니다. 디버그메뉴시작은 Command Arguments에 "startdebugmenu"를 넣으세요!!
BOOL g_bIsStartTitleMenu = TRUE;

void ResetProcessTick()
{
	g_nProcessTick = 0;
}

int GetProcessTick()
{
//	return CFcWorld::GetInstance().GetProcessTick();
	return g_nProcessTick;
}

float GetProcessTime()
{
	return float(g_nProcessTick)/40.f;
}

bool IsPause()
{
	return g_bPause;
}


int GetGuardianType(int nUnitSoxId)
{
	switch(nUnitSoxId)
	{
	case C_LS_SL1_OH:
	case C_LS_SL2_OH:
	case C_LS_SL3_OH:
	case C_DS_OC4_OH:
	case C_DS_OC5_OH:
	case C_DS_OC7_OH:
	case C_DS_OC8_OH:
	case C_DS_OC1_OH:
	case C_DS_OC2_OH:
	case C_DS_EF2_OH:
	case C_DS_GB1_OH:
	case C_DS_EF1_OH:
	case C_DS_GB2_OH:
	case C_LS_WR1_OH:
	case C_LS_SL_EX1_OH:
	case C_LS_SL_EX2_OH:
	case C_LS_SL_EX3_OH:	return GT_ONE_HAND;

	case C_DS_OC4_TH:
	case C_DS_OC5_TH:
	case C_DS_OC7_TH:
	case C_DS_OC8_TH:
	case C_DS_OC1_TH:
	case C_DS_OC2_TH:
	case C_LS_SL1_TH:
	case C_LS_SL2_TH:
	case C_LS_SL3_TH:
	case C_LS_SL4_TH:
	case C_LS_WR1_TH:		return GT_TWO_HAND;

	case C_LS_SL1_SP:
	case C_LS_SL2_SP:
	case C_LS_SL3_SP:
	case C_DS_GB1_SP:
	case C_DS_GB2_SP:		return GT_SPEAR;

	case C_LS_SL4_AR:
	case C_DS_GB1_AR:
	case C_DS_GB2_AR:
	case C_LS_SL_EX4_AR:	return GT_RANGE;
	default:
		{	
			return GT_ONE_HAND;
		}
	}
	
	return -1;
}


void CFCGAMEDATA::Clear()
{
	// g_DefaultHitSignal.m_pParam의 Delete는 ~ASSignalData 에서 처리
}

#define CFG_VIBRATION					"Vibration"
#define CFG_BGMVOLUME					"BGM"
#define CFG_SOUNDVOLUME					"Sound"
#define CFG_VOICEVOLUME					"Voice"
#define CFG_CAPTION						"Caption"
#define CFG_SAVESET						"SaveSet"
#define CFG_LANGUAGE					"Language"



CFCGAMEDATA::CFCGAMEDATA()
{
	bPause = false;
	nPlayerCount = 1;
	nEnablePadID = -1;

#ifdef _USE_TGS_TYPE_MENU
	nPlayerType = MISSION_TYPE_INPHYY;
#else
	nPlayerType = MISSION_TYPE_NONE;
#endif
	GuardianInfo[0].clear();
	GuardianInfo[1].clear();

	nPlayerSetting = 1;
	lSaveRandomSeed = 0;
	State = GAME_STATE_NON;
	cMapFileName[0] = NULL;
	bOpenLogo = true;
	memset(nTrgGlobalVar,-1,sizeof(int) * MAX_TRG_VAR_NUM);

	PrevStageId = STAGE_ID_NONE;
	SelStageId = STAGE_ID_NONE;

	//stage내에서 다음 stage결정한다. 예외처리
	nSpecialSelStageState = SPECIAL_STAGE_MENU_WAIT;
	SpeaialSelStageId = STAGE_ID_NONE;
	SpecialSelStageIdforTrigger = STAGE_ID_NONE;

	nMissionFinish = MISSION_FINISH_TYPE_NON;
	cFinishRealtimeMovieFileName[0] = NULL;

	bShowDeviceUI = true;
	bOnDeviceUI = false;
	bChangeDevice = false;
	bCancelDeviceSelector = false;
	bRemovedStorageDevice = false;
	bCheckFreeSpace = true;
	nSaveDataSize = STORAGE_ENOUGHSIZE;

	nSaveGameCount = 0;
	bFixedSaveData = false;
	bChangeProfile = false;
	bLockAchievement = false;
	bFirstInitProfile = false;

	szProfileName[0] = NULL;

#ifdef _XBOX
	m_DeviceID = XCONTENTDEVICE_ANY;
	m_OldDeviceID = m_DeviceID;
#else
	m_DeviceID = XCONTENTDEVICE_ANY;
#endif //_XBOX
	bCameraToggle = false;

	pDashBoard = NULL;
}

CFCGAMEDATA::~CFCGAMEDATA()
{
	CBsFileIO::FreePhysicalMemory(pDashBoard, NULL);
}

/*
void CFCGAMEDATA::LoadConfig()
{
	char cFileName[256];
#ifdef _XBOX
	strcpy(cFileName, "d:\\config.txt");
#else
	strcpy(cFileName, "config.txt");
#endif

	Parser parser;
	TokenList toklist;

	parser.Create();
	parser.EnableComments( true );

	parser.ReserveKeyword(CFG_VIBRATION);	
	parser.ReserveKeyword(CFG_BGMVOLUME);
	parser.ReserveKeyword(CFG_SOUNDVOLUME);	
	parser.ReserveKeyword(CFG_VOICEVOLUME);		
	parser.ReserveKeyword(CFG_CAPTION);		
	parser.ReserveKeyword(CFG_SAVESET);			
	parser.ReserveKeyword(CFG_LANGUAGE);	

	bool result = parser.ProcessSource( "", cFileName, &toklist );
	if( result == false ){
		return;
	}

	result = parser.ProcessHeaders( &toklist );
	if( result == false ){
		return;
	}
	parser.ProcessMacros( &toklist );
	TokenList::iterator itr = toklist.begin();
	
	while( 1 )
	{	

		if( itr == toklist.end() ){
			break;
		}

		if( strcmp(itr->GetKeyword(), CFG_VIBRATION) == 0){
			itr++; ConfigInfo.bVibration = (itr->GetInteger()) ? true : false;
		}
		else if( strcmp(itr->GetKeyword(), CFG_BGMVOLUME) == 0){
			itr++; ConfigInfo.nBGMVolume = itr->GetInteger();
		}
		else if( strcmp(itr->GetKeyword(), CFG_SOUNDVOLUME) == 0){
			itr++; ConfigInfo.nSoundVolume = itr->GetInteger();
		}
		else if( strcmp(itr->GetKeyword(), CFG_VOICEVOLUME) == 0){
			itr++; ConfigInfo.nVoiceVolume = itr->GetInteger();
		}
		else if( strcmp(itr->GetKeyword(), CFG_CAPTION) == 0){
			itr++; ConfigInfo.bCaption = (itr->GetInteger()) ? true : false;
		}
		else if( strcmp(itr->GetKeyword(), CFG_SAVESET) == 0){
			itr++; ConfigInfo.bSaveSet = (itr->GetInteger()) ? true : false;
		}
		else if( strcmp(itr->GetKeyword(), CFG_LANGUAGE) == 0){
			itr++; g_LocalLanguage.SetLanguage((GAME_LANGUAGE)(itr->GetInteger()));
		}

		itr++;
	}
}

void CFCGAMEDATA::SaveConfig()
{
#ifndef _XBOX
	char cFileName[256];

	strcpy(cFileName, "config.txt");

	FILE *fp = fopen(cFileName,"wt");
	if(fp == NULL)
		return;

	fprintf(fp,"%s\t%d\n", CFG_VIBRATION,	(ConfigInfo.bVibration) ? 1 : 0 );
	fprintf(fp,"%s\t%d\n", CFG_BGMVOLUME,	ConfigInfo.nBGMVolume);
	fprintf(fp,"%s\t%d\n", CFG_SOUNDVOLUME,	ConfigInfo.nSoundVolume);
	fprintf(fp,"%s\t%d\n", CFG_VOICEVOLUME,	ConfigInfo.nVoiceVolume);
	fprintf(fp,"%s\t%d\n", CFG_CAPTION,		ConfigInfo.bCaption);
	fprintf(fp,"%s\t%d\n", CFG_SAVESET,		ConfigInfo.bSaveSet);
	fprintf(fp,"%s\t%d\n", CFG_LANGUAGE,	(int)(g_LocalLanguage.GetLanguage()));

	fclose(fp);

#endif //!_XBOX
}
*/

ASSignalData* CFCGAMEDATA::GetDefaultTrapHitSignal( int nDamage )
{
	if( g_DefaultHitSignal.m_pParam == NULL )
	{
		g_DefaultHitSignal.m_pParam = new int[15];
	}
	g_DefaultHitSignal.m_nID = SIGNAL_TYPE_HIT;
	g_DefaultHitSignal.m_NumParam = 15;
	g_DefaultHitSignal.m_pParam[0] = 0;
	g_DefaultHitSignal.m_pParam[1] = nDamage;		// 이게 -1이고 nAttackPower가 0이면 HP가 0이 된다.
	g_DefaultHitSignal.m_pParam[2] = ANI_TYPE_HIT;
	g_DefaultHitSignal.m_pParam[3] = 1;
	g_DefaultHitSignal.m_pParam[4] = 5;
	g_DefaultHitSignal.m_pParam[5] = -10;
	g_DefaultHitSignal.m_pParam[6] = -1;
	g_DefaultHitSignal.m_pParam[7] = 0;
	g_DefaultHitSignal.m_pParam[8] = 0;
	g_DefaultHitSignal.m_pParam[9] = 0;
	g_DefaultHitSignal.m_pParam[10] = 6;
	g_DefaultHitSignal.m_pParam[11] = 0;
	g_DefaultHitSignal.m_pParam[14] = 0;

	return &g_DefaultHitSignal;
}



ASSignalData* CFCGAMEDATA::GetDefaultPhysicsHitSignal( int nDamage )
{
	if( g_DefaultHitSignal.m_pParam == NULL )
	{
		g_DefaultHitSignal.m_pParam = new int[15];
	}
	g_DefaultHitSignal.m_nID = SIGNAL_TYPE_HIT;
	g_DefaultHitSignal.m_NumParam = 15;
	g_DefaultHitSignal.m_pParam[0] = 0;
	g_DefaultHitSignal.m_pParam[1] = nDamage;		// 이게 -1이고 nAttackPower가 0이면 HP가 0이 된다.
	g_DefaultHitSignal.m_pParam[2] = ANI_TYPE_HIT;
	g_DefaultHitSignal.m_pParam[3] = 1;
	g_DefaultHitSignal.m_pParam[4] = 5;
	g_DefaultHitSignal.m_pParam[5] = -10;
	g_DefaultHitSignal.m_pParam[6] = -1;
	g_DefaultHitSignal.m_pParam[7] = 0;
	g_DefaultHitSignal.m_pParam[8] = 0;
	g_DefaultHitSignal.m_pParam[9] = 0;
	g_DefaultHitSignal.m_pParam[10] = 6;
	g_DefaultHitSignal.m_pParam[11] = 0;
	g_DefaultHitSignal.m_pParam[14] = 0;

	return &g_DefaultHitSignal;
}

//====================================================================
// 
//====================================================================
void CFCGAMEDATA::SaveSet( int nIndex )
{
	char szFileName[256];
	sprintf( szFileName, "DebugSet%d.set", nIndex );

	BFileStream Stream( szFileName, BFileStream::create );
	if( !Stream.Valid() ) return;

	CSaveDebugFlag Flag = *((CSaveDebugFlag*)this);
	Stream.Write( &Flag, sizeof(CSaveDebugFlag), ENDIAN_FOUR_BYTE );

}

void CFCGAMEDATA::LoadSet( int nIndex )
{
	char szFileName[256];
	sprintf( szFileName, "DebugSet%d.set", nIndex );

	BFileStream Stream( szFileName, BFileStream::openRead );
	if( !Stream.Valid() ) return;

	int asdf = sizeof(CSaveDebugFlag);
//	CSaveDebugFlag Flag = (CSaveDebugFlag*)this;
	Stream.Read( this, sizeof(CSaveDebugFlag), ENDIAN_FOUR_BYTE );
}

void CFCGAMEDATA::GetSaveDirectory( char *pSaveDir, int nBufferSize )
{
#ifdef _XBOX
	strcpy_s( pSaveDir, nBufferSize, "d:\\Save\\" ); //aleksger - safe string
#else
	GetCurrentDirectory( nBufferSize, pSaveDir );
	strcat( pSaveDir, "Save\\" );
#endif
}


void CFCGAMEDATA::LoadDashBoardCaption()
{	
	g_BsKernel.chdir("text");

	char cFullFileName[256];
	sprintf(cFullFileName, "%s%s\\%s_%s.txt", 
		g_BsKernel.GetCurrentDirectory(),
		g_LocalLanguage.GetLanguageDir(),
		_DASHBOARD_UTF16_FILENAME, 
		g_LocalLanguage.GetLanguageStr());

	g_BsKernel.chdir("..");

	DWORD dwFileSize;
	if(FAILED(CBsFileIO::LoadFilePhysicalMemory(cFullFileName,
							&pDashBoard,
							&dwFileSize,
							NULL,
							XALLOC_PHYSICAL_ALIGNMENT_DEFAULT,
							XALLOC_MEMPROTECT_READWRITE,
							false )))
	{
		BsAssert(0 && "Failed to open _DASHBOARD_UTF16");
		return;
	}

	// Read Unicode Caption from index
#ifdef _XBOX
	CFcGetCaptionUTF16::SetBufferPointer( pDashBoard, dwFileSize );
#endif
}


//====================================================================
// save load game data
//====================================================================
void CFCGAMEDATA::SaveGameData(int nState, XCONTENT_DATA* pContentData)
{
#ifdef _XBOX

	DWORD dwIndex;

	if( g_FCGameData.nEnablePadID == -1 )
	{
        dwIndex = XUSER_INDEX_NONE;
		return; // 무시 해 버림.
	}
	else
		dwIndex = g_FCGameData.nEnablePadID;

	// if there is no profile file, check free space of selected device to create a new profile file
	if(nState == SFF_FIXED_GAME_DATA && g_FCGameData.bFixedSaveData == false)
	{		
		XDEVICE_DATA  data={0,};
		if(ERROR_SUCCESS != XContentGetDeviceData(g_FCGameData.m_DeviceID, &data))
		{
			// 중간에 디바이스 장치가 제거 되었거나 기타 오류 처리
			return;
		}		

		if( data.ulDeviceFreeBytes < FIXEDSAVEDATA_ENOUGHSIZE)		
			return;

	}


	// Create event for asynchronous writing
	HANDLE hEventComplete = CreateEvent( NULL, FALSE, FALSE, NULL );

	BsAssert(  hEventComplete != NULL  );

	XOVERLAPPED xov = {0};
	xov.hEvent = hEventComplete;

	// Mount the device associated with the display name for writing
	
	if( XContentCreate( dwIndex, SAVE_ROOT, pContentData,
		XCONTENTFLAG_CREATEALWAYS | XCONTENTFLAG_NOPROFILE_TRANSFER, NULL, NULL, &xov) != ERROR_IO_PENDING )
	{
		CloseHandle( hEventComplete );
		BsAssert(0);
		return;
	}

	// Wait on hEventComplete handle
	if( XGetOverlappedResult( &xov, NULL, TRUE ) == ERROR_SUCCESS )
	{
		BFileStream Stream( SAVE_GAMEFILE, BFileStream::create );

		SAV_FILE_HEADER Header;
		Header.nVersion = SAV_FILE_VERSION_LAST;
		strcpy( Header.szHeaderString, SAV_FILE_HEADER_STRING );
		if(nState == SFF_FIXED_GAME_DATA)
			Header.nBodySize = g_FCGameData.tempFixedSave.GetSaveDataSize();
		else
			Header.nBodySize = g_FCGameData.tempUserSave.GetSaveDataSize();
		Stream.Write( &Header, sizeof( SAV_FILE_HEADER ), 4 );

		
		switch(nState)
		{
		case SFF_FIXED_GAME_DATA:	tempFixedSave.Save(&Stream); break;
		case SFF_USER_GAME_DATA:	{
	
			tempUserSave.Save(&Stream); break;
									}
		default: BsAssert(0);
		}

		// storage의 free space 체크
		//int nDataSize = Stream.Length(); //g_FCGameData.tempFixedSave.GetSaveDataSize();
		////g_FCGameData.nSaveDataSize = nDataSize;
		//ULARGE_INTEGER iBytesRequested = {0};
		//iBytesRequested.QuadPart = XContentCalculateSize(nDataSize, 1);
		//XDEVICE_DATA  data={0,};
		//if(ERROR_SUCCESS != XContentGetDeviceData(g_FCGameData.m_DeviceID, &data) ||		
		//	data.ulDeviceFreeBytes <= iBytesRequested.QuadPart) 
		//	g_FCGameData.bShowDeviceUI = true;
	}

	XContentClose( SAVE_ROOT, &xov );

	// Wait for XCloseContent to complete
	XGetOverlappedResult( &xov, NULL, TRUE );

	CloseHandle( hEventComplete );

	//save 후에 content를 update해야 한다.
	UpdateGetContentData();
#endif // _XBOX
}

bool CFCGAMEDATA::LoadGameData(int nState, FixedSaveData* pFixedSave, UserSaveData* pUserSave, XCONTENT_DATA* pContentData)
{
	bool bResult = true;
#ifdef _XBOX
	DWORD dwIndex;

	if( g_FCGameData.nEnablePadID == -1 )
	{
		dwIndex = XUSER_INDEX_NONE;
		return false;
	}
	else
		dwIndex = g_FCGameData.nEnablePadID;


	// Create event for asynchronous reading
	HANDLE hEventComplete = CreateEvent( NULL, FALSE, FALSE, NULL );

	BsAssert(  hEventComplete != NULL  );

	XOVERLAPPED xov = {0};
	xov.hEvent = hEventComplete;
	
	// Mount the device for reading
	if( XContentCreate( dwIndex , SAVE_ROOT, pContentData,
		XCONTENTFLAG_OPENEXISTING | XCONTENTFLAG_NOPROFILE_TRANSFER, NULL, NULL, &xov ) != ERROR_IO_PENDING )
	{
		CloseHandle( hEventComplete );
		BsAssert(0);
		return FALSE;
	}


	if( XGetOverlappedResult( &xov, NULL, TRUE ) == ERROR_SUCCESS )
	{
		BFileStream Stream( SAVE_GAMEFILE, BFileStream::openRead );
		SAV_FILE_HEADER Header;

		if( !Stream.Valid() )
		{
			return false;
		}

		Stream.Read( &Header, sizeof( SAV_FILE_HEADER)-sizeof(int) , 4 );
		if(Header.nVersion > SAV_FILE_VERSION_16) 
			Stream.Read( &Header.nBodySize, sizeof(int), 4 );

		switch(nState)
		{
		case SFF_FIXED_GAME_DATA:
			{
				BsAssert(pFixedSave);
				pFixedSave->Load(Header.nVersion, &Stream); // [PREFIX:nomodify] 2006/2/16 junyash PS#5197 Header.nVersion is initialized in Stream.Read( &Header, sizeof( SAV_FILE_HEADER)-sizeof(int) , 4 );
				break;
			}
		case SFF_USER_GAME_DATA:
			{
				BsAssert(pUserSave);
				if(pUserSave->Load(Header.nVersion, &Stream) == false){ // [PREFIX:nomodify] 2006/2/16 junyash PS#5197 Header.nVersion is initialized in Stream.Read( &Header, sizeof( SAV_FILE_HEADER)-sizeof(int) , 4 );
					bResult = false; // 읽기에 실패했을때.(데이타가 손상되었거나 유저 정보가 일치하지 않음)
				}
				break;
			}
		default: BsAssert(0);
		}
		if(Header.nVersion > SAV_FILE_VERSION_16) {
			if(Header.nBodySize != Stream.Tell()-sizeof(SAV_FILE_HEADER))
				bResult = false;
		}
	}
	else
		bResult = false;

	XContentClose( SAVE_ROOT, &xov );

	// Wait for XCloseContent to complete
	XGetOverlappedResult( &xov, NULL, TRUE );

	CloseHandle( hEventComplete );
#endif // _XBOX

	return bResult;
}




//====================================================================
// get content info
//====================================================================

#ifdef _XBOX
struct ContentSlotCompare
{
  bool operator()(const XCONTENT_DATA &lhs, const XCONTENT_DATA &rhs) const  { 	  
	  return GetSlotPart(lhs.szFileName) > GetSlotPart(rhs.szFileName);
  }

  std::string GetSlotPart(const std::string &filename) const {
	  	int nLeft = filename.find_last_of('_') +1,
			nRight= filename.find(".sav");
		return filename.substr(nLeft, nRight-nLeft);
  };
};	
#endif //_XBOX

bool CFCGAMEDATA::UpdateGetContentData()
{
#ifdef _XBOX
	HANDLE hEnum;
	DWORD cbBuffer;

	DWORD dwRet;
	DWORD dwIndex= XUSER_INDEX_NONE;

	nSaveGameCount = 0;

	if(g_FCGameData.nEnablePadID!=-1 && ATG::SignIn::IsUserSignedIn(g_FCGameData.nEnablePadID))
		dwIndex = g_FCGameData.nEnablePadID;
	else
		return true;

	dwRet = XContentCreateEnumerator( dwIndex, // Access data associated with this user
		m_DeviceID,         // Pass in selected device
		XCONTENTTYPE_SAVEDGAME,
		0,                     // No special flags
		ARRAYSIZE( m_ContentData ),
		&cbBuffer,
		&hEnum );

	if( dwRet != ERROR_SUCCESS ){
		BsAssert(0);
		return false;
	}
	else{
		// Since XCONTENT_DATA is fixed size, the total buffer size should
		// be the same as the size of the m_ContentData array.
		BsAssert( cbBuffer == sizeof( m_ContentData ) );
	}

	// Enumerate display names
	XEnumerate( hEnum, m_ContentData, sizeof( m_ContentData ),	&(nSaveGameCount), NULL );

	CloseHandle( hEnum );
	bFixedSaveData = false;

	// sort
	std::set<XCONTENT_DATA, ContentSlotCompare> contents;
	for(DWORD i=0; i<nSaveGameCount; i++)
	{
		if(!bFixedSaveData && strcmp(m_ContentData[i].szFileName, _FIXED_SAVE_FILENAME) == 0)
		{
			bFixedSaveData = true;
			
		}

		contents.insert(m_ContentData[i]);	
	}

	std::set<XCONTENT_DATA, ContentSlotCompare>::iterator itor = contents.begin();
	for(int i=0;itor != contents.end(); ++itor,++i)
		m_ContentData[i] = *itor;


	return true;
	
#endif //_XBOX
	return true;
}

XCONTENT_DATA* CFCGAMEDATA::GetContentData(int nIndex)
{
#ifdef _XBOX
	return &(m_ContentData[nIndex]);
#endif //_XBOX
	return NULL;
}

int CFCGAMEDATA::GetUserSaveGameCount()
{
	int nUserSaveGame = nSaveGameCount;
	if(bFixedSaveData == true){
		nUserSaveGame = nSaveGameCount -1;
	}

	return (nUserSaveGame >= 0) ? nUserSaveGame : 0;
}


bool CFCGAMEDATA::IsUserSavedGame( int nIndex )
{
#ifdef _XBOX
	CHAR szFileName[ XCONTENT_MAX_FILENAME_LENGTH ];
	sprintf( szFileName, SAV_FILE_NAME, nIndex );

	for(DWORD i=0; i<nSaveGameCount; i++)
	{
		if(strcmp(m_ContentData[i].szFileName, szFileName) == 0){
			return true;
		}
	}
#endif //_XBOX

	return false;
}

void CFCGAMEDATA::ShowDeviceUI()
{
#ifdef _XBOX
	DWORD dwRet;

	ZeroMemory( &m_Overlapped, sizeof( XOVERLAPPED ) );
	m_DeviceID = XCONTENTDEVICE_ANY;

	ULARGE_INTEGER iBytesRequested = {0};
	if(bCheckFreeSpace)
		iBytesRequested.QuadPart = STORAGE_ENOUGHSIZE;// XContentCalculateSize(nSaveDataSize, 0); // 최소한 필요한 스토리지 공간

	DWORD dwIndex = XUSER_INDEX_NONE;

	if(g_FCGameData.nEnablePadID!=-1 && ATG::SignIn::IsUserSignedIn(g_FCGameData.nEnablePadID))
		dwIndex = g_FCGameData.nEnablePadID;
	else
		return;
	
	dwRet = XShowDeviceSelectorUI( dwIndex , // User to receive input from
		XCONTENTTYPE_SAVEDGAME,   // List only save game devices
		XCONTENTFLAG_FORCE_SHOW_UI,                    // Force display even if multiple devices are not available to select.
		iBytesRequested,         // Size of the device data struct
		&m_DeviceID,            // Return selected device information
		&m_Overlapped );


	assert( dwRet == ERROR_IO_PENDING );
	bOnDeviceUI = true;
	//bShowDeviceUI = true;
	
#endif //_XBOX	
}


//====================================================================
//fixed saved game data
//====================================================================
void CFCGAMEDATA::SaveFixedGameData()
{
#ifdef _XBOX
	XCONTENT_DATA contentData = {0};
	strcpy( contentData.szFileName, "FixedSave.sav" );
	CFcGetCaptionUTF16::GetCaption( _TEX_FIXED_DATA, contentData.szDisplayName );
	contentData.dwContentType = XCONTENTTYPE_SAVEDGAME;
	contentData.DeviceID = m_DeviceID;

	SaveGameData(SFF_FIXED_GAME_DATA, &contentData);
#endif // _XBOX
}

bool CFCGAMEDATA::LoadFixedGameData()
{
#ifdef _XBOX
	XCONTENT_DATA contentData = {0};
	strcpy( contentData.szFileName, "FixedSave.sav" );
	CFcGetCaptionUTF16::GetCaption( _TEX_FIXED_DATA, contentData.szDisplayName );
	contentData.dwContentType = XCONTENTTYPE_SAVEDGAME;
	contentData.DeviceID = m_DeviceID;

	return LoadGameData(SFF_FIXED_GAME_DATA, &tempFixedSave, NULL, &contentData);
#else
	return true;
#endif // _XBOX
}

//====================================================================
//user saved game data
//====================================================================
bool CFCGAMEDATA::SaveUserGameData( XCONTENT_DATA* pContentData )
{
#ifdef _XBOX

	g_FCGameData.tempUserSave.nHeroID = g_MissionToSoxID[g_FCGameData.nPlayerType];
	g_FCGameData.tempUserSave.nCurStageID = (int)(g_FCGameData.SelStageId);
	g_FCGameData.tempUserSave.nSpeaialSelStageID = (int)(g_FCGameData.SpeaialSelStageId);

	for(int i=0; i<MAX_TRG_VAR_NUM; i++){
		g_FCGameData.tempUserSave.nTrgGlobalVar[i] = g_FCGameData.nTrgGlobalVar[i];
	}

	g_FCGameData.tempUserSave.EquipList.clear();
	int nCount = g_FcItemManager.GetItemCount();
	for(int i=0; i<nCount; i++)
	{
		HeroEquip* pEquip = g_FcItemManager.GetHeroEquip(i);
		HeroEquip Item;
		Item = *pEquip;
		g_FCGameData.tempUserSave.EquipList.push_back(Item);
	}

	XCONTENT_DATA contentData = {0};
	int nSaveSlot=0;

	if(pContentData == NULL)
	{
		if(nSaveGameCount >= _MAX_LOADSAVE_DATA_COUNT){
			return false;
		}		
		// 마지막 슬롯 번호를 찾는다.
		int nLastSlot=0;
		for(DWORD i=0; i<nSaveGameCount; ++i) 
		{
			if(strcmp(m_ContentData[i].szFileName, _FIXED_SAVE_FILENAME) == 0)
				continue;
			int nSlot = GetSlotNum(m_ContentData[i].szFileName);			
			if(nSlot > nLastSlot) { 
				nLastSlot = nSlot;			
			}			
		}
		nSaveSlot = nLastSlot+1;
	}
	else{		
		for(DWORD i=0; i<nSaveGameCount; ++i)
			if(&m_ContentData[i] == pContentData)
				break;		

		nSaveSlot = GetSlotNum(m_ContentData[i].szFileName);				
		DeleteGameData(pContentData);
	}
	char cFileName[XCONTENT_MAX_DISPLAYNAME_LENGTH];
	WCHAR szDisplayName[XCONTENT_MAX_DISPLAYNAME_LENGTH];
	GetSaveFileName(cFileName, _countof(cFileName), szDisplayName);
	
	sprintf( contentData.szFileName, SAV_FILE_NAME, cFileName, nSaveSlot );
	lstrcpyW( contentData.szDisplayName, szDisplayName );
	contentData.dwContentType = XCONTENTTYPE_SAVEDGAME;
	contentData.DeviceID = m_DeviceID;

	SaveGameData(SFF_USER_GAME_DATA, &contentData);

#endif // _XBOX

	return true;
}

bool CFCGAMEDATA::LoadUserGameData( XCONTENT_DATA* pContentData )
{
#ifdef _XBOX

	if(LoadGameData(SFF_USER_GAME_DATA, NULL, &tempUserSave, pContentData) == false){
		return false;
	}

	g_FCGameData.nPlayerType = g_SoxToMissionID[g_FCGameData.tempUserSave.nHeroID];
	g_FCGameData.SelStageId = (GAME_STAGE_ID)(g_FCGameData.tempUserSave.nCurStageID);
	g_FCGameData.SpeaialSelStageId = (GAME_STAGE_ID)(g_FCGameData.tempUserSave.nSpeaialSelStageID);

	for(int i=0; i<MAX_TRG_VAR_NUM; i++){
		g_FCGameData.nTrgGlobalVar[i] = g_FCGameData.tempUserSave.nTrgGlobalVar[i];
	}

	g_FcItemManager.Finalize();
	DWORD dwCount = g_FCGameData.tempUserSave.EquipList.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		HeroEquip Item = g_FCGameData.tempUserSave.EquipList[i];
		g_FcItemManager.AddItem(Item);
	}

	return true;
	
#else
	return true;
#endif // _XBOX
}

bool CFCGAMEDATA::LoadUserGameInfo( UserSaveData* ptempUserSave, XCONTENT_DATA* pContentData )
{
#ifdef _XBOX
	return LoadGameData(SFF_USER_GAME_DATA, NULL, ptempUserSave, pContentData);
#else
	return true;
#endif // _XBOX
}

//InitStage
void CFCGAMEDATA::AutoSave()
{
	g_FCGameData.tempUserSave.nHeroID = g_MissionToSoxID[g_FCGameData.nPlayerType];
	g_FCGameData.tempUserSave.nCurStageID = (int)(g_FCGameData.SelStageId);
	g_FCGameData.tempUserSave.nSpeaialSelStageID = (int)(g_FCGameData.SpeaialSelStageId);

	for(int i=0; i<MAX_TRG_VAR_NUM; i++){
		g_FCGameData.tempUserSave.nTrgGlobalVar[i] = g_FCGameData.nTrgGlobalVar[i];
	}

	g_FCGameData.tempUserSave.EquipList.clear();
	int nCount = g_FcItemManager.GetItemCount();
	for(int i=0; i<nCount; i++)
	{
		HeroEquip* pEquip = g_FcItemManager.GetHeroEquip(i);
		HeroEquip Item;
		Item = *pEquip;
		g_FCGameData.tempUserSave.EquipList.push_back(Item);
	}

	AutoSaveBuf.Clear();
	tempUserSave.Save( &AutoSaveBuf );	
}

//mission restart
bool CFCGAMEDATA::RollBack()
{
	AutoSaveBuf.Seek(0, BStream::fromBegin);
	tempUserSave.Load(SAV_FILE_VERSION_LAST, &AutoSaveBuf);

	g_FCGameData.nPlayerType = g_SoxToMissionID[g_FCGameData.tempUserSave.nHeroID];
	g_FCGameData.SelStageId = (GAME_STAGE_ID)(g_FCGameData.tempUserSave.nCurStageID);
	g_FCGameData.SpeaialSelStageId = (GAME_STAGE_ID)(g_FCGameData.tempUserSave.nSpeaialSelStageID);

	for(int i=0; i<MAX_TRG_VAR_NUM; i++){
		g_FCGameData.nTrgGlobalVar[i] = g_FCGameData.tempUserSave.nTrgGlobalVar[i];
	}
		
	g_FcItemManager.Finalize();
	DWORD dwCount = g_FCGameData.tempUserSave.EquipList.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		HeroEquip Item = g_FCGameData.tempUserSave.EquipList[i];
		g_FcItemManager.AddItem(Item);
	}
	return true;
}

//=============================================================================
//
//=============================================================================


//=============================================================================
//_UserSaveData
//=============================================================================
void _UserSaveData::Save( BStream *pStream )
{
#ifdef _XBOX
	DWORD dwIndex;

	if( g_FCGameData.nEnablePadID == -1 )
	{
        dwIndex = XUSER_INDEX_NONE;
		return; // 무시 해 버림.
	}
	else
		dwIndex = g_FCGameData.nEnablePadID;


	CHAR szProfileName[128]={0,};
	XUserGetName(dwIndex, szProfileName, sizeof(szProfileName));
	pStream->Write( szProfileName, sizeof( szProfileName ), 4 );


	pStream->Write( &nHeroID, sizeof( int ), 4 );
	pStream->Write( &nLevel, sizeof( int ), 4 );
	pStream->Write( &nExp, sizeof( int ), 4 );
	pStream->Write( &nCurStageID, sizeof( int ), 4 );
	pStream->Write( &nSpeaialSelStageID, sizeof( int ), 4 );
	
	pStream->Write( GuardianInfo, sizeof( GuardianTroop ) * 2, 4 );

	//trigger value
	for(int i = 0; i < MAX_TRG_VAR_NUM; i++ ){
		pStream->Write( &( nTrgGlobalVar[ i ] ), sizeof( int ), 4 );
	}
	
	//clear stage
	int nSize = ( int )ClearStageList.size();
	pStream->Write( &nSize, sizeof( int ), 4 );
	for(int i = 0; i < nSize; i++ )
	{
		pStream->Write( &( ClearStageList[ i ] ), sizeof( int ), 4 );
	}

	//item
	nSize = ( int )EquipList.size();
	pStream->Write( &nSize, sizeof( int ), 4 );
	for(int i = 0; i < nSize; i++ )
	{
		pStream->Write( &( EquipList[ i ] ), sizeof( HeroEquip ), 4 );
	}
	// game opstions
	bool bTempDumy = true;
	pStream->Write( &bTempDumy, sizeof(bTempDumy), 4);
	pStream->Write( &g_FCGameData.ConfigInfo.nBGMVolume, sizeof(g_FCGameData.ConfigInfo.nBGMVolume), 4);				
	pStream->Write( &g_FCGameData.ConfigInfo.nSoundVolume, sizeof(g_FCGameData.ConfigInfo.nSoundVolume), 4);			
	pStream->Write( &g_FCGameData.ConfigInfo.nVoiceVolume, sizeof(g_FCGameData.ConfigInfo.nVoiceVolume), 4);			
	pStream->Write( &g_FCGameData.ConfigInfo.bCaption, sizeof(g_FCGameData.ConfigInfo.bCaption), 4);
	pStream->Write( &g_FCGameData.ConfigInfo.bCameraUD_Reverse, sizeof(g_FCGameData.ConfigInfo.bCameraUD_Reverse), 4);		
	pStream->Write( &g_FCGameData.ConfigInfo.bCameraLR_Reverse, sizeof(g_FCGameData.ConfigInfo.bCameraLR_Reverse), 4);		
	pStream->Write( &g_FCGameData.ConfigInfo.nContrast, sizeof(g_FCGameData.ConfigInfo.nContrast), 4);			

	pStream->Write( &g_FCGameData.bCameraToggle, sizeof( int ), 4 );

#endif //_XBOX
}


int _UserSaveData::GetSaveDataSize() 
{	
	BCountStream stream;
	Save(&stream);
	return stream.Length();
}



bool _UserSaveData::Load(int nVersion, BStream *pStream)
{
	switch(nVersion)
	{
	case SAV_FILE_VERSION_LAST:	return Load_Version19(pStream);	
	case SAV_FILE_VERSION_18:	return Load_Version18(pStream);	
	case SAV_FILE_VERSION_17:	return Load_Version17(pStream);	
	case SAV_FILE_VERSION_16:	return Load_Version16(pStream);
	case SAV_FILE_VERSION_15:	return Load_Version15(pStream);
	case SAV_FILE_VERSION_14:	return Load_Version14(pStream);
	case SAV_FILE_VERSION_13:	return Load_Version13(pStream); 
	case SAV_FILE_VERSION_12:	return Load_Version12(pStream); 
	case SAV_FILE_VERSION_11:	return Load_Version11(pStream); 
	case SAV_FILE_VERSION_10:	return Load_Version10(pStream); 
	default: BsAssert(0);
	}
	return false;
}
bool _UserSaveData::Load_Version15(BStream *pStream)
{
#ifdef _XBOX
	DWORD dwIndex;
	if( g_FCGameData.nEnablePadID == -1 )
		dwIndex = XUSER_INDEX_NONE;
	else
		dwIndex = g_FCGameData.nEnablePadID;

	// 다른 프로필의 세이브 데이타를 가져올 경우 achievement 쪽의 동작을 막는다.
	CHAR szProfileName[128]={0,},szSavedName[128]={0,};
	XUserGetName(dwIndex, szProfileName, sizeof(szProfileName));
	pStream->Read( szSavedName, sizeof( szSavedName ), 4 );
	if(strcmp(szSavedName, szProfileName)!=0)
		g_FCGameData.bLockAchievement = true;
	else
		g_FCGameData.bLockAchievement = false;
#endif // _XBOX
	pStream->Read( &nHeroID, sizeof( int ), 4 );
	pStream->Read( &nLevel, sizeof( int ), 4 );
	pStream->Read( &nExp, sizeof( int ), 4 );
	pStream->Read( &nCurStageID, sizeof( int ), 4 );
	pStream->Read( &nSpeaialSelStageID, sizeof( int ), 4 );

	pStream->Read( GuardianInfo, sizeof( GuardianTroop ) * 2, 4 );
	pStream->Read( nTrgGlobalVar, sizeof( int ) * MAX_TRG_VAR_NUM, 4 );

	//clear stage
	int nSize, nStage;
	pStream->Read( &nSize, sizeof( int ), 4 );
	ClearStageList.clear();
	for(int i = 0; i < nSize; i++ )
	{
		pStream->Read( &nStage, sizeof( int ), 4 );
		ClearStageList.push_back( nStage );		// [PREFIX:nomodify] 2006/2/16 junyash PS# nStage is initialized in pStream->Read( &nStage, sizeof( int ), 4 );
	}

	//item
	HeroEquip Item;
	pStream->Read( &nSize, sizeof( int ), 4 );
	EquipList.clear();
	for(int i = 0; i < nSize; i++ )
	{
		pStream->Read( &Item, sizeof( HeroEquip ), 4 );
		EquipList.push_back( Item );
	}

	// game options
	bool bTempDumy = true;
	pStream->Read( &bTempDumy, sizeof(bTempDumy), 4);
	pStream->Read( &g_FCGameData.ConfigInfo.nBGMVolume, sizeof(g_FCGameData.ConfigInfo.nBGMVolume), 4);				
	pStream->Read( &g_FCGameData.ConfigInfo.nSoundVolume, sizeof(g_FCGameData.ConfigInfo.nSoundVolume), 4);			
	pStream->Read( &g_FCGameData.ConfigInfo.nVoiceVolume, sizeof(g_FCGameData.ConfigInfo.nVoiceVolume), 4);			
	pStream->Read( &g_FCGameData.ConfigInfo.bCaption, sizeof(g_FCGameData.ConfigInfo.bCaption), 4);
	pStream->Read( &g_FCGameData.ConfigInfo.bCameraUD_Reverse, sizeof(g_FCGameData.ConfigInfo.bCameraUD_Reverse), 4);		
	pStream->Read( &g_FCGameData.ConfigInfo.bCameraLR_Reverse, sizeof(g_FCGameData.ConfigInfo.bCameraLR_Reverse), 4);		
	pStream->Read( &g_FCGameData.ConfigInfo.nContrast, sizeof(g_FCGameData.ConfigInfo.nContrast), 4);
	
	g_BsMoviePlayer.SetVolume( (int)((g_FCGameData.ConfigInfo.nBGMVolume * 0.1f) * (float)_MAX_BINK_VOLUME) );
	g_pSoundManager->SetVolume(FC_MENU_VOL_BGM, g_FCGameData.ConfigInfo.nBGMVolume * 0.1f);
	g_pSoundManager->SetVolume(FC_MENU_VOL_EFT_SOUND, g_FCGameData.ConfigInfo.nSoundVolume * 0.1f);
	g_pSoundManager->SetVolume(FC_MENU_VOL_VOICE, g_FCGameData.ConfigInfo.nVoiceVolume * 0.1f);

	g_BsKernel.SetGammaRamp(float(g_FCGameData.ConfigInfo.nContrast)/10.f);

	pStream->Read( &g_FCGameData.bCameraToggle, sizeof( int ), 4);

	return true;
}
bool _UserSaveData::Load_Version14(BStream *pStream)
{
#ifdef _XBOX
	DWORD dwIndex;
	if( g_FCGameData.nEnablePadID == -1 )
		dwIndex = XUSER_INDEX_NONE;
	else
		dwIndex = g_FCGameData.nEnablePadID;

	CHAR szProfileName[128]={0,},szSavedName[128]={0,};
	XUserGetName(dwIndex, szProfileName, sizeof(szProfileName));
	pStream->Read( szSavedName, sizeof( szSavedName ), 4 );
//	if(strcmp(szSavedName, szProfileName)!=0)
//		return false;
#endif // _XBOX
	pStream->Read( &nHeroID, sizeof( int ), 4 );
	pStream->Read( &nLevel, sizeof( int ), 4 );
	pStream->Read( &nExp, sizeof( int ), 4 );
	pStream->Read( &nCurStageID, sizeof( int ), 4 );
	pStream->Read( &nSpeaialSelStageID, sizeof( int ), 4 );

	pStream->Read( GuardianInfo, sizeof( GuardianTroop ) * 2, 4 );
	pStream->Read( nTrgGlobalVar, sizeof( int ) * MAX_TRG_VAR_NUM, 4 );

	//clear stage
	int nSize, nStage;
	pStream->Read( &nSize, sizeof( int ), 4 );
	ClearStageList.clear();
	for(int i = 0; i < nSize; i++ )
	{
		pStream->Read( &nStage, sizeof( int ), 4 );
		ClearStageList.push_back( nStage );	 // [PREFIX:nomodify] 2006/2/16 junyash PS#5198 nStage is initialized in pStream->Read( &nStage, sizeof( int ), 4 );
	}

	//item
	HeroEquip Item;
	pStream->Read( &nSize, sizeof( int ), 4 );
	EquipList.clear();
	for(int i = 0; i < nSize; i++ )
	{
		pStream->Read( &Item, sizeof( HeroEquip ), 4 );
		EquipList.push_back( Item );
	}

	// game options
	bool bTempDumy = true;
	pStream->Read( &bTempDumy, sizeof(bTempDumy), 4);
	pStream->Read( &g_FCGameData.ConfigInfo.nBGMVolume, sizeof(g_FCGameData.ConfigInfo.nBGMVolume), 4);				
	pStream->Read( &g_FCGameData.ConfigInfo.nSoundVolume, sizeof(g_FCGameData.ConfigInfo.nSoundVolume), 4);			
	pStream->Read( &g_FCGameData.ConfigInfo.nVoiceVolume, sizeof(g_FCGameData.ConfigInfo.nVoiceVolume), 4);			
	pStream->Read( &g_FCGameData.ConfigInfo.bCaption, sizeof(g_FCGameData.ConfigInfo.bCaption), 4);
	pStream->Read( &g_FCGameData.ConfigInfo.bCameraUD_Reverse, sizeof(g_FCGameData.ConfigInfo.bCameraUD_Reverse), 4);		
	pStream->Read( &g_FCGameData.ConfigInfo.bCameraLR_Reverse, sizeof(g_FCGameData.ConfigInfo.bCameraLR_Reverse), 4);		
	pStream->Read( &g_FCGameData.ConfigInfo.nContrast, sizeof(g_FCGameData.ConfigInfo.nContrast), 4);
	
	g_BsMoviePlayer.SetVolume( (int)((g_FCGameData.ConfigInfo.nBGMVolume * 0.1f) * (float)_MAX_BINK_VOLUME) );
	g_pSoundManager->SetVolume(FC_MENU_VOL_BGM, g_FCGameData.ConfigInfo.nBGMVolume * 0.1f);
	g_pSoundManager->SetVolume(FC_MENU_VOL_EFT_SOUND, g_FCGameData.ConfigInfo.nSoundVolume * 0.1f);
	g_pSoundManager->SetVolume(FC_MENU_VOL_VOICE, g_FCGameData.ConfigInfo.nVoiceVolume * 0.1f);

	g_BsKernel.SetGammaRamp(float(g_FCGameData.ConfigInfo.nContrast)/10.f);

	return true;
}


bool _UserSaveData::Load_Version13(BStream *pStream)
{
#ifdef _XBOX
	DWORD dwIndex;
	if( g_FCGameData.nEnablePadID == -1 )
		dwIndex = XUSER_INDEX_NONE;
	else
		dwIndex = g_FCGameData.nEnablePadID;

	CHAR szProfileName[128]={0,},szSavedName[128]={0,};
	XUserGetName(dwIndex, szProfileName, sizeof(szProfileName));
	pStream->Read( szSavedName, sizeof( szSavedName ), 4 );
	if(strcmp(szSavedName, szProfileName)!=0)
		return false;
#endif // _XBOX
	pStream->Read( &nHeroID, sizeof( int ), 4 );
	pStream->Read( &nLevel, sizeof( int ), 4 );
	pStream->Read( &nExp, sizeof( int ), 4 );
	pStream->Read( &nCurStageID, sizeof( int ), 4 );
	pStream->Read( &nSpeaialSelStageID, sizeof( int ), 4 );

	pStream->Read( GuardianInfo, sizeof( GuardianTroop ) * 2, 4 );
	pStream->Read( nTrgGlobalVar, sizeof( int ) * MAX_TRG_VAR_NUM, 4 );

	//clear stage
	int nSize, nStage;
	pStream->Read( &nSize, sizeof( int ), 4 );
	ClearStageList.clear();
	for(int i = 0; i < nSize; i++ )
	{
		pStream->Read( &nStage, sizeof( int ), 4 );
		ClearStageList.push_back( nStage );	 // [PREFIX:nomodify] 2006/2/16 junyash PS#5199 nStage is initialized in pStream->Read( &nStage, sizeof( int ), 4 );
	}

	//item
	HeroEquip Item;
	pStream->Read( &nSize, sizeof( int ), 4 );
	EquipList.clear();
	for(int i = 0; i < nSize; i++ )
	{
		pStream->Read( &Item, sizeof( HeroEquip ), 4 );
		EquipList.push_back( Item );
	}
	return true;
}

bool _UserSaveData::Load_Version12(BStream *pStream)
{
	pStream->Read( &nHeroID, sizeof( int ), 4 );
	pStream->Read( &nLevel, sizeof( int ), 4 );
	pStream->Read( &nExp, sizeof( int ), 4 );
	pStream->Read( &nCurStageID, sizeof( int ), 4 );
	pStream->Read( &nSpeaialSelStageID, sizeof( int ), 4 );

	pStream->Read( GuardianInfo, sizeof( GuardianTroop ) * 2, 4 );
	pStream->Read( nTrgGlobalVar, sizeof( int ) * MAX_TRG_VAR_NUM, 4 );

	//clear stage
	int nSize, nStage;
	pStream->Read( &nSize, sizeof( int ), 4 );
	ClearStageList.clear();
	for(int i = 0; i < nSize; i++ )	 // [PREFIX:nomodify] 2006/2/16 junyash PS#5200 nSize is initialized in pStream->Read( &nSize, sizeof( int ), 4 );
	{
		pStream->Read( &nStage, sizeof( int ), 4 );
		ClearStageList.push_back( nStage );
	}

	//item
	HeroEquip Item;
	pStream->Read( &nSize, sizeof( int ), 4 );
	EquipList.clear();
	for(int i = 0; i < nSize; i++ )
	{
		pStream->Read( &Item, sizeof( HeroEquip ), 4 );
		EquipList.push_back( Item );
	}
	return true;
}

bool _UserSaveData::Load_Version11(BStream *pStream)
{
	pStream->Read( &nHeroID, sizeof( int ), 4 );
	pStream->Read( &nLevel, sizeof( int ), 4 );
	pStream->Read( &nExp, sizeof( int ), 4 );
	pStream->Read( &nCurStageID, sizeof( int ), 4 );
	pStream->Read( &nSpeaialSelStageID, sizeof( int ), 4 );

	pStream->Read( GuardianInfo, sizeof( GuardianTroop ) * 2, 4 );
	pStream->Read( nTrgGlobalVar, sizeof( int ) * MAX_TRG_VAR_NUM, 4 );

	//clear stage
	int nSize, nStage;
	pStream->Read( &nSize, sizeof( int ), 4 );
	ClearStageList.clear();
	for(int i = 0; i < nSize; i++ )	 // [PREFIX:nomodify] 2006/2/16 junyash PS#5201 nSize is initialized in pStream->Read( &nSize, sizeof( int ), 4 );
	{
		pStream->Read( &nStage, sizeof( int ), 4 );
		ClearStageList.push_back( nStage );
	}

	//item
	HeroEquip Item;
	pStream->Read( &nSize, sizeof( int ), 4 );
	EquipList.clear();
	for(int i = 0; i < nSize; i++ )
	{
		pStream->Read( &(Item.nItemSoxID), sizeof( int ), 4 );
		pStream->Read( &(Item.bAccoutered), sizeof( BOOL ), 4 );
		pStream->Read( &(Item.bNew), sizeof( BOOL ), 4 );
		
		EquipList.push_back( Item );
	}
	return true;
}


bool _UserSaveData::Load_Version10(BStream *pStream)
{
	pStream->Read( &nHeroID, sizeof( int ), 4 );
	pStream->Read( &nLevel, sizeof( int ), 4 );
	pStream->Read( &nExp, sizeof( int ), 4 );
	pStream->Read( &nCurStageID, sizeof( int ), 4 );
	pStream->Read( &nSpeaialSelStageID, sizeof( int ), 4 );
	
	pStream->Read( GuardianInfo, sizeof( GuardianTroop ) * 2, 4 );

	//clear stage
	int nSize, nStage;
	pStream->Read( &nSize, sizeof( int ), 4 );
	ClearStageList.clear();
	for(int i = 0; i < nSize; i++ )
	{
		pStream->Read( &nStage, sizeof( int ), 4 );
		ClearStageList.push_back( nStage );	 // [PREFIX:nomodify] 2006/2/16 junyash PS#5202 nStage is initialized in pStream->Read( &nStage, sizeof( int ), 4 );
	}

	//item
	HeroEquip Item;
	pStream->Read( &nSize, sizeof( int ), 4 );
	EquipList.clear();
	for(int i = 0; i < nSize; i++ )
	{
		pStream->Read( &(Item.nItemSoxID), sizeof( int ), 4 );
		pStream->Read( &(Item.bAccoutered), sizeof( BOOL ), 4 );
		pStream->Read( &(Item.bNew), sizeof( BOOL ), 4 );

		EquipList.push_back( Item );
	}
	return true;
}


//=============================================================================
//_HeroRecordInfo
//=============================================================================
void _HeroRecordInfo::Save( BStream *pStream )
{
	pStream->Write( &nHeroID, sizeof( int ), 4 );
	pStream->Write( &bAllStageClear, sizeof( bool ) );

	int nSize = ( int )StageResultList.size();
	pStream->Write( &nSize, sizeof( int ), 4 );
	for(int i = 0; i < nSize; i++ )
	{
		pStream->Write( &( StageResultList[ i ] ), sizeof( StageResultInfo ), 4 );
	}
}

void _HeroRecordInfo::Load(int nVersion, BStream *pStream)
{
	switch(nVersion)
	{
	case SAV_FILE_VERSION_LAST:	Load_Version19(pStream); break;
	case SAV_FILE_VERSION_18:	Load_Version18(pStream); break;
	case SAV_FILE_VERSION_17:	Load_Version17(pStream); break;
	case SAV_FILE_VERSION_16:	Load_Version16(pStream); break;
	case SAV_FILE_VERSION_15:	Load_Version15(pStream); break;
	case SAV_FILE_VERSION_14:	Load_Version14(pStream); break;
	case SAV_FILE_VERSION_13:	Load_Version13(pStream); break;
	case SAV_FILE_VERSION_12:	Load_Version12(pStream); break;
	case SAV_FILE_VERSION_11:	Load_Version11(pStream); break;
	case SAV_FILE_VERSION_10:	Load_Version10(pStream); break;
	default: BsAssert(0);
	}
}

void _HeroRecordInfo::Load_Version16(BStream *pStream)
{
	pStream->Read( &nHeroID, sizeof( int ), 4 );
	pStream->Read( &bAllStageClear, sizeof( bool ) );

	int nSize;
	StageResultInfo Result;
	pStream->Read( &nSize, sizeof( int ), 4 );
	StageResultList.clear();
	for(int i = 0; i < nSize; i++ )
	{
		pStream->Read( &Result, sizeof( StageResultInfo ), 4 );
		StageResultList.push_back( Result );
	}
}

void _HeroRecordInfo::Load_Version13(BStream *pStream)
{
	pStream->Read( &nHeroID, sizeof( int ), 4 );
	pStream->Read( &bAllStageClear, sizeof( bool ) );

	int nSize;
	StageResultInfo Result;
	pStream->Read( &nSize, sizeof( int ), 4 );
	StageResultList.clear();
	for(int i = 0; i < nSize; i++ )	 // [PREFIX:nomodify] 2006/2/16 junyash PS#5203 nSize is initialized in pStream->Read( &nSize, sizeof( int ), 4 );
	{
		pStream->Read( &Result, sizeof( StageResultInfo )-sizeof(int), 4 ); // 16버전에서 nHeroLevel 이 추가되어 용량이 변경됨
		StageResultList.push_back( Result );
	}
}

StageResultInfo* _HeroRecordInfo::GetStageResultInfo(int nStageID)
{
	DWORD dwCount = StageResultList.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		StageResultInfo* pStageResultInfo = &(StageResultList[i]);
		if(pStageResultInfo->nStageID == nStageID){
			return pStageResultInfo;
		}
	}

	return NULL;
}


//=============================================================================
//_FixedSaveData
//=============================================================================

int _FixedSaveData::GetSaveDataSize() 
{	
	BCountStream stream;
	Save(&stream);
	return stream.Length();
}

void _FixedSaveData::Save( BStream *pStream )
{
#ifdef _XBOX
	CHAR szProfileName[128]={0,};
	XUserGetName(g_FCGameData.nEnablePadID, szProfileName, sizeof(szProfileName));
#else
	CHAR szProfileName[128] = { 'P', 'c', ' ', 'u', 's', 'e', 'r', 0 };
#endif
	pStream->Write( szProfileName, sizeof( szProfileName ), 4 );

	pStream->Write( &nTotalPoint, sizeof( int ), 4 );
	pStream->Write( &bOpenBonusStage, sizeof( bool ) );
	pStream->Write( &bGoodEndingforAspharr, sizeof( bool ) );
	
	int nSize = ( int )HeroRecordList.size();
	pStream->Write( &nSize, sizeof( int ), 4 );
	for(int i = 0; i < nSize; i++ )
	{
		HeroRecordList[ i ].Save( pStream );
	}

	nSize = ( int )LibraryItemList.size();
	pStream->Write( &nSize, sizeof( int ), 4 );
	for( int i = 0; i < nSize; i++ )
	{
		pStream->Write( &( LibraryItemList[ i ] ), sizeof( LibraryItem ), 4 );
	}
	
	bool bTempDumy = true;
	pStream->Write( &bTempDumy, sizeof(bTempDumy), 4);
	pStream->Write( &g_FCGameData.ConfigInfo.nBGMVolume, sizeof(g_FCGameData.ConfigInfo.nBGMVolume), 4);				
	pStream->Write( &g_FCGameData.ConfigInfo.nSoundVolume, sizeof(g_FCGameData.ConfigInfo.nSoundVolume), 4);			
	pStream->Write( &g_FCGameData.ConfigInfo.nVoiceVolume, sizeof(g_FCGameData.ConfigInfo.nVoiceVolume), 4);			
	pStream->Write( &g_FCGameData.ConfigInfo.bCaption, sizeof(g_FCGameData.ConfigInfo.bCaption), 4);
	pStream->Write( &g_FCGameData.ConfigInfo.bCameraUD_Reverse, sizeof(g_FCGameData.ConfigInfo.bCameraUD_Reverse), 4);		
	pStream->Write( &g_FCGameData.ConfigInfo.bCameraLR_Reverse, sizeof(g_FCGameData.ConfigInfo.bCameraLR_Reverse), 4);		
	pStream->Write( &g_FCGameData.ConfigInfo.nContrast, sizeof(g_FCGameData.ConfigInfo.nContrast), 4);				
}

void _FixedSaveData::Load(int nVersion, BStream *pStream)
{
	switch(nVersion)
	{
	case SAV_FILE_VERSION_LAST:	Load_Version19(nVersion, pStream); break;
	case SAV_FILE_VERSION_18:	Load_Version18(nVersion, pStream); break;
	case SAV_FILE_VERSION_17:	Load_Version17(nVersion, pStream); break;
	case SAV_FILE_VERSION_16:	Load_Version16(nVersion, pStream); break;
	case SAV_FILE_VERSION_15:	Load_Version15(nVersion, pStream); break;
	case SAV_FILE_VERSION_14:	Load_Version14(nVersion, pStream); break;
	case SAV_FILE_VERSION_13:	Load_Version12(nVersion, pStream); break;
	case SAV_FILE_VERSION_12:	Load_Version12(nVersion, pStream); break;
	case SAV_FILE_VERSION_11:	Load_Version11(nVersion, pStream); break;
	case SAV_FILE_VERSION_10:	Load_Version10(nVersion, pStream); break;
	default: BsAssert(0);
	}
}


void _FixedSaveData::Load_Version19(int nVersion, BStream *pStream)
{
	// 다른 프로필의 세이브 데이타를 가져올 경우 achievement 쪽의 동작을 막는다.
	CHAR szSavedName[128]={0,};
	pStream->Read( szSavedName, sizeof( szSavedName ), 4 );
	
	g_FCGameData.bLockAchievement = false;

	pStream->Read( &nTotalPoint, sizeof( int ), 4 );
	pStream->Read( &bOpenBonusStage, sizeof( bool ) );
	pStream->Read( &bGoodEndingforAspharr, sizeof( bool ) );

	int nSize;
	HeroRecordInfo Result;
	pStream->Read( &nSize, sizeof( int ), 4 );
	HeroRecordList.clear();
	for(int i = 0; i < nSize; i++ )	 // [PREFIX:nomodify] 2006/2/16 junyash PS#5204 nSize is initialized in pStream->Read( &nSize, sizeof( int ), 4 );
	{
		HeroRecordList.push_back( Result );
		HeroRecordList[ i ].Load( nVersion, pStream );
	}

	LibraryItem LItem;
	pStream->Read( &nSize, sizeof( int ), 4 );
	LibraryItemList.clear();
	for( int i = 0; i < nSize; i++ )
	{
		pStream->Read( &LItem, sizeof( LibraryItem ), 4 );
		LibraryItemList.push_back( LItem );
	}

	bool bTempDumy = true;
	pStream->Read( &bTempDumy, sizeof(bTempDumy), 4);
	pStream->Read( &g_FCGameData.ConfigInfo.nBGMVolume, sizeof(g_FCGameData.ConfigInfo.nBGMVolume), 4);				
	pStream->Read( &g_FCGameData.ConfigInfo.nSoundVolume, sizeof(g_FCGameData.ConfigInfo.nSoundVolume), 4);			
	pStream->Read( &g_FCGameData.ConfigInfo.nVoiceVolume, sizeof(g_FCGameData.ConfigInfo.nVoiceVolume), 4);			
	pStream->Read( &g_FCGameData.ConfigInfo.bCaption, sizeof(g_FCGameData.ConfigInfo.bCaption), 4);
	pStream->Read( &g_FCGameData.ConfigInfo.bCameraUD_Reverse, sizeof(g_FCGameData.ConfigInfo.bCameraUD_Reverse), 4);		
	pStream->Read( &g_FCGameData.ConfigInfo.bCameraLR_Reverse, sizeof(g_FCGameData.ConfigInfo.bCameraLR_Reverse), 4);		
	pStream->Read( &g_FCGameData.ConfigInfo.nContrast, sizeof(g_FCGameData.ConfigInfo.nContrast), 4);
	
	g_BsMoviePlayer.SetVolume( (int)((g_FCGameData.ConfigInfo.nBGMVolume * 0.1f) * (float)_MAX_BINK_VOLUME) );
	g_pSoundManager->SetVolume(FC_MENU_VOL_BGM, g_FCGameData.ConfigInfo.nBGMVolume * 0.1f);	
	g_pSoundManager->SetVolume(FC_MENU_VOL_EFT_SOUND, g_FCGameData.ConfigInfo.nSoundVolume * 0.1f);
	g_pSoundManager->SetVolume(FC_MENU_VOL_VOICE, g_FCGameData.ConfigInfo.nVoiceVolume * 0.1f);
	g_BsKernel.SetGammaRamp(float(g_FCGameData.ConfigInfo.nContrast)/10.f);
}

void _FixedSaveData::Load_Version18(int nVersion, BStream *pStream)
{
	pStream->Read( &nTotalPoint, sizeof( int ), 4 );
	pStream->Read( &bOpenBonusStage, sizeof( bool ) );
	pStream->Read( &bGoodEndingforAspharr, sizeof( bool ) );

	int nSize;
	HeroRecordInfo Result;
	pStream->Read( &nSize, sizeof( int ), 4 );
	HeroRecordList.clear();
	for(int i = 0; i < nSize; i++ )	 // [PREFIX:nomodify] 2006/2/16 junyash PS#5204 nSize is initialized in pStream->Read( &nSize, sizeof( int ), 4 );
	{
		HeroRecordList.push_back( Result );
		HeroRecordList[ i ].Load( nVersion, pStream );
	}

	LibraryItem LItem;
	pStream->Read( &nSize, sizeof( int ), 4 );
	LibraryItemList.clear();
	for( int i = 0; i < nSize; i++ )
	{
		pStream->Read( &LItem, sizeof( LibraryItem ), 4 );
		LibraryItemList.push_back( LItem );
	}

	bool bTempDumy = true;
	pStream->Read( &bTempDumy, sizeof(bTempDumy), 4);
	pStream->Read( &g_FCGameData.ConfigInfo.nBGMVolume, sizeof(g_FCGameData.ConfigInfo.nBGMVolume), 4);				
	pStream->Read( &g_FCGameData.ConfigInfo.nSoundVolume, sizeof(g_FCGameData.ConfigInfo.nSoundVolume), 4);			
	pStream->Read( &g_FCGameData.ConfigInfo.nVoiceVolume, sizeof(g_FCGameData.ConfigInfo.nVoiceVolume), 4);			
	pStream->Read( &g_FCGameData.ConfigInfo.bCaption, sizeof(g_FCGameData.ConfigInfo.bCaption), 4);
	pStream->Read( &g_FCGameData.ConfigInfo.bCameraUD_Reverse, sizeof(g_FCGameData.ConfigInfo.bCameraUD_Reverse), 4);		
	pStream->Read( &g_FCGameData.ConfigInfo.bCameraLR_Reverse, sizeof(g_FCGameData.ConfigInfo.bCameraLR_Reverse), 4);		
	pStream->Read( &g_FCGameData.ConfigInfo.nContrast, sizeof(g_FCGameData.ConfigInfo.nContrast), 4);
	
	g_BsMoviePlayer.SetVolume( (int)((g_FCGameData.ConfigInfo.nBGMVolume * 0.1f) * (float)_MAX_BINK_VOLUME) );
	g_pSoundManager->SetVolume(FC_MENU_VOL_BGM, g_FCGameData.ConfigInfo.nBGMVolume * 0.1f);	
	g_pSoundManager->SetVolume(FC_MENU_VOL_EFT_SOUND, g_FCGameData.ConfigInfo.nSoundVolume * 0.1f);
	g_pSoundManager->SetVolume(FC_MENU_VOL_VOICE, g_FCGameData.ConfigInfo.nVoiceVolume * 0.1f);
	g_BsKernel.SetGammaRamp(float(g_FCGameData.ConfigInfo.nContrast)/10.f);
}

void _FixedSaveData::Load_Version14(int nVersion, BStream *pStream)
{
	pStream->Read( &nTotalPoint, sizeof( int ), 4 );
	pStream->Read( &bOpenBonusStage, sizeof( bool ) );

	int nSize;
	HeroRecordInfo Result;
	pStream->Read( &nSize, sizeof( int ), 4 );
	HeroRecordList.clear();
	for(int i = 0; i < nSize; i++ )	 // [PREFIX:nomodify] 2006/2/16 junyash PS#5204 nSize is initialized in pStream->Read( &nSize, sizeof( int ), 4 );
	{
		HeroRecordList.push_back( Result );
		HeroRecordList[ i ].Load( nVersion, pStream );
	}

	LibraryItem LItem;
	pStream->Read( &nSize, sizeof( int ), 4 );
	LibraryItemList.clear();
	for( int i = 0; i < nSize; i++ )
	{
		pStream->Read( &LItem, sizeof( LibraryItem ), 4 );
		LibraryItemList.push_back( LItem );
	}
	
	bool bTempDumy = true;
	pStream->Read( &bTempDumy, sizeof(bTempDumy), 4);
	pStream->Read( &g_FCGameData.ConfigInfo.nBGMVolume, sizeof(g_FCGameData.ConfigInfo.nBGMVolume), 4);				
	pStream->Read( &g_FCGameData.ConfigInfo.nSoundVolume, sizeof(g_FCGameData.ConfigInfo.nSoundVolume), 4);			
	pStream->Read( &g_FCGameData.ConfigInfo.nVoiceVolume, sizeof(g_FCGameData.ConfigInfo.nVoiceVolume), 4);			
	pStream->Read( &g_FCGameData.ConfigInfo.bCaption, sizeof(g_FCGameData.ConfigInfo.bCaption), 4);
	pStream->Read( &g_FCGameData.ConfigInfo.bCameraUD_Reverse, sizeof(g_FCGameData.ConfigInfo.bCameraUD_Reverse), 4);		
	pStream->Read( &g_FCGameData.ConfigInfo.bCameraLR_Reverse, sizeof(g_FCGameData.ConfigInfo.bCameraLR_Reverse), 4);		
	pStream->Read( &g_FCGameData.ConfigInfo.nContrast, sizeof(g_FCGameData.ConfigInfo.nContrast), 4);
	
	g_BsMoviePlayer.SetVolume( (int)((g_FCGameData.ConfigInfo.nBGMVolume * 0.1f) * (float)_MAX_BINK_VOLUME) );
	g_pSoundManager->SetVolume(FC_MENU_VOL_BGM, g_FCGameData.ConfigInfo.nBGMVolume * 0.1f);	
	g_pSoundManager->SetVolume(FC_MENU_VOL_EFT_SOUND, g_FCGameData.ConfigInfo.nSoundVolume * 0.1f);
	g_pSoundManager->SetVolume(FC_MENU_VOL_VOICE, g_FCGameData.ConfigInfo.nVoiceVolume * 0.1f);
	g_BsKernel.SetGammaRamp(float(g_FCGameData.ConfigInfo.nContrast)/10.f);
}
void _FixedSaveData::Load_Version12(int nVersion, BStream *pStream)
{
	pStream->Read( &nTotalPoint, sizeof( int ), 4 );
	pStream->Read( &bOpenBonusStage, sizeof( bool ) );

	int nSize;
	HeroRecordInfo Result;
	pStream->Read( &nSize, sizeof( int ), 4 );
	HeroRecordList.clear();
	for(int i = 0; i < nSize; i++ )	 // [PREFIX:nomodify] 2006/2/16 junyash PS#5205 nSize is initialized in pStream->Read( &nSize, sizeof( int ), 4 );
	{
		HeroRecordList.push_back( Result );
		HeroRecordList[ i ].Load( nVersion, pStream );
	}

	LibraryItem LItem;
	pStream->Read( &nSize, sizeof( int ), 4 );
	LibraryItemList.clear();
	for( int i = 0; i < nSize; i++ )
	{
		pStream->Read( &LItem, sizeof( LibraryItem ), 4 );
		LibraryItemList.push_back( LItem );
	}

}


HeroRecordInfo* _FixedSaveData::GetHeroRecordInfo(int nHeroID)
{
	DWORD dwCount = HeroRecordList.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		HeroRecordInfo* pHeroRecordInfo = &(HeroRecordList[i]);
		if(pHeroRecordInfo->nHeroID == nHeroID){
			return pHeroRecordInfo;
		}
	}

	return NULL;
}


LibraryItem* _FixedSaveData::GetLibraryData(int nItemID)
{
	DWORD dwCount = LibraryItemList.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		LibraryItem* pLibraryItem = &(LibraryItemList[i]);
		if(pLibraryItem->nID == nItemID){
			return pLibraryItem;
		}
	}

	return NULL;
}


void CFCGAMEDATA::GetSaveFileName(char* szFileName, const size_t szFileName_Len, WCHAR* pCaption)
{
	//File Name ---------------------------------------------------
	SYSTEMTIME LocalSysTime;
	GetLocalTime( &LocalSysTime );

	int nYear = (int)(LocalSysTime.wYear);
	int nMon = (int)(LocalSysTime.wMonth);
	int nDay = (int)(LocalSysTime.wDay);
	int nH = (int)(LocalSysTime.wHour);
	int nM = (int)(LocalSysTime.wMinute);
	int nS = (int)(LocalSysTime.wSecond);

	int nHeroID = g_FCGameData.tempUserSave.nHeroID;
	int nLevel = g_FCGameData.tempUserSave.nLevel;
	int nStageID = g_FCGameData.tempUserSave.nCurStageID;

	//file name
	sprintf_s(szFileName, szFileName_Len, "%d_%d_%d_%d_%d_%d_%d_%d_%d",
		nYear, nMon, nDay,
		nH, nM, nS,
		nHeroID, nLevel, nStageID);

	
	
	//caption ---------------------------------------------------

	//time-------
	int nH12 = nH;
	nH12 += ( nH == 0 ) ? 12 : ( ( nH > 12 ) ? -12 : 0 );
	int nAMPMTextID = ( nH < 12 ) ? _TEX_AM : _TEX_PM;

	WCHAR cAMPM[64];
#ifdef _XBOX
	CFcGetCaptionUTF16::GetCaption( nAMPMTextID, cAMPM );
#endif
		    
	//Name-------
	int nHeroTextID = _TEX_Inphyy + g_SoxToMissionID[nHeroID];

	WCHAR cHeroName[ 128 ];
#ifdef _XBOX
	CFcGetCaptionUTF16::GetCaption( nHeroTextID, cHeroName );
#endif

	
	//stage name-------
	int nStageTextID = GetStageIDforName(nStageID, nHeroID);
	
	WCHAR cStageName[ 128 ];
#ifdef _XBOX
	CFcGetCaptionUTF16::GetCaption( nStageTextID, cStageName );
#endif
	
	//Full Caption-------
	switch(g_LocalLanguage.GetLanguage())
	{
	case LANGUAGE_KOR:
	case LANGUAGE_JPN:
	case LANGUAGE_CHN:
		{
			wsprintfW(pCaption, L"%d/%d/%d  %s %d:%02d  %s Lv. %d  %s",
				nYear, nMon, nDay,
				cAMPM, nH12, nM,
				cHeroName, nLevel+1,
				cStageName);
			break;
		}
	case LANGUAGE_ENG:
	case LANGUAGE_GER:
	case LANGUAGE_FRA:
	case LANGUAGE_SPA:
	case LANGUAGE_ITA:
	default:
		{
			wsprintfW(pCaption, L"%d/%d/%d  %s %d:%02d  %s Lv. %d  %s",
				nMon, nDay, nYear,
				cAMPM, nH12, nM,
				cHeroName, nLevel+1,
				cStageName);
			break;
		}
	}
}


//주의 : 정확히는 아이디와 text가 1:1 matching되어야 하지만
//기획에서 stage를 나누는 과정에서 서로 다르게 표시되는 문제때문에..
//다음과 같이 처리하였다.
int CFCGAMEDATA::GetStageIDforName(int nStageID, int nHeroSoxID)
{
	int nStageName = -1;
	switch(nStageID)
	{
	case STAGE_ID_VARRGANDD:			nStageName = _TEX_VARRGANDD; break;
	case STAGE_ID_OUT_VARRGANDD:		nStageName = _TEX_OUT_VARRGANDD; break;
	case STAGE_ID_VARRFARRINN:			nStageName = _TEX_VARRFARRINN; break;
	case STAGE_ID_OUT_VARRFARRINN:		nStageName = _TEX_OUT_VARRFARRINN; break;
	case STAGE_ID_VARRVAZZAR:
		{
			switch(nHeroSoxID)
			{
			case CHAR_ID_MYIFEE:
			case CHAR_ID_DWINGVATT:	nStageName = _TEX_VARRVAZZAR_2; break;
			case CHAR_ID_ASPHARR:
			default: nStageName = _TEX_VARRVAZZAR; 
			}

			break;
		}
	case STAGE_ID_OUT_VARRVAZZAR:		nStageName = _TEX_OUT_VARRVAZZAR; break;
	case STAGE_ID_WYANDEEK:				nStageName = _TEX_WYANDEEK; break;
	case STAGE_ID_WYANDEEK_VILLAGE:		nStageName = _TEX_WYANDEEK_VILLAGE; break;
	case STAGE_ID_YWA_UE_UAR:			nStageName = _TEX_YWA_UE_UAR; break;
	case STAGE_ID_EAURVARRIA:			nStageName = _TEX_EAURVARRIA; break;
	case STAGE_ID_FELPPE:				nStageName = _TEX_FELPPE; break;
	case STAGE_ID_PHOLYA:				nStageName = _TEX_PHOLYA; break;
	case STAGE_ID_BEFORE_PHOLYA_1:		nStageName = _TEX_BEFORE_PHOLYA_1; break;
	case STAGE_ID_BEFORE_PHOLYA_2:		nStageName = _TEX_BEFORE_PHOLYA_2; break;
	case STAGE_ID_ANOTHER_WORLD:		nStageName = _TEX_ANOTHER_WORLD; break;
	case STAGE_ID_VARRVAZZAR_2:
		{
			switch(nHeroSoxID)
			{
			case CHAR_ID_MYIFEE:
			case CHAR_ID_DWINGVATT:	nStageName = _TEX_VARRVAZZAR; break;
			case CHAR_ID_ASPHARR:
			default: nStageName = _TEX_VARRVAZZAR_2; 
			}

			break;
		}
	default:
		{
			nStageName = _TEX_VARRGANDD;
			BsAssert(0);
		}
	}

	return nStageName;
}

void CFCGAMEDATA::GetStageName(int nStageID, char* szName, const size_t szName_Len, int nHeroSoxID)
{	
	int nStageName = GetStageIDforName(nStageID, nHeroSoxID);
	g_TextTable->GetText(nStageName, szName, szName_Len);
}


void CFCGAMEDATA::GetSavedFileInfo(XCONTENT_DATA* pContentData, SavedDataInfo* pSavedDataInfo)
{
#ifdef _XBOX
	char ctempText0[256], ctempText1[256];
	strcpy(ctempText0, pContentData->szFileName);

	//save file name info
	//nYear, nMonth, nDay
	pSavedDataInfo->nYear	= GetDataInfo(ctempText0, ctempText1, _countof(ctempText1));
	pSavedDataInfo->nMonth	= GetDataInfo(ctempText1, ctempText0, _countof(ctempText0));
	pSavedDataInfo->nDay	= GetDataInfo(ctempText0, ctempText1, _countof(ctempText1));

	//nHour, nMin, nSec
	pSavedDataInfo->nHour	= GetDataInfo(ctempText1, ctempText0, _countof(ctempText0));
	pSavedDataInfo->nMin	= GetDataInfo(ctempText0, ctempText1, _countof(ctempText1));
	pSavedDataInfo->nSec	= GetDataInfo(ctempText1, ctempText0, _countof(ctempText0));

	//nHeroID, nLevel, nStageID
	pSavedDataInfo->nHeroID = GetDataInfo(ctempText0, ctempText1, _countof(ctempText1));
	pSavedDataInfo->nLevel	= GetDataInfo(ctempText1, ctempText0, _countof(ctempText0));
	pSavedDataInfo->nStageID = GetDataInfo(ctempText0, ctempText1, _countof(ctempText1));

	//nSlot
	pSavedDataInfo->nSlot	= GetDataInfo(ctempText1, ctempText0, _countof(ctempText0));
#endif //_XBOX
}


bool CFCGAMEDATA::DeleteGameData(XCONTENT_DATA* pContentData)
{
	bool bResult = false;

#ifdef _XBOX
	// Create event for asynchronous reading
	HANDLE hEventComplete = CreateEvent( NULL, FALSE, FALSE, NULL );

	BsAssert(  hEventComplete != NULL  );

	XOVERLAPPED xov = {0};
	xov.hEvent = hEventComplete;
	DWORD dwIndex;

	if( g_FCGameData.nEnablePadID == -1 )
		dwIndex = XUSER_INDEX_NONE;
	else
		dwIndex = g_FCGameData.nEnablePadID;


	if(XContentDelete(dwIndex,pContentData, &xov)!= ERROR_IO_PENDING )
	{
		CloseHandle( hEventComplete );
		BsAssert(0);
		return FALSE;
	}

	if( XGetOverlappedResult( &xov, NULL, TRUE ) == ERROR_SUCCESS )
	{
		bResult = true;
	}	
	else 
	{
		CloseHandle( hEventComplete );
		BsAssert(0);
		return FALSE;
	}
	// Wait for XCloseContent to complete
	XGetOverlappedResult( &xov, NULL, TRUE );

	CloseHandle( hEventComplete );
#endif // _XBOX

	return bResult;

}

int CFCGAMEDATA::GetDataInfo(char* pSrcText, char* pDscText, const size_t szDsc_Len)
{	
	char ctempName[256];
	strcpy(ctempName, pSrcText);
	char* pSrc = strchr(pSrcText, '_');
	if(!pSrc)
		pSrc = strchr(pSrcText, '.');
	pSrcText = pSrc;
	strcpy_s(pDscText, szDsc_Len, pSrcText+1);

	int nLen = strlen(ctempName);
	for(int i=0; i<nLen; i++)
	{
		if(ctempName[i] == '_' || ctempName[i] == '.')
		{
			ctempName[i] = '\0';
			return atoi(ctempName);
		}
	}

	BsAssert(0);
	return -1;
}

int CFCGAMEDATA::GetSlotNum(const std::string &filename) const {
	size_t nLeft = filename.find_last_of('_') +1,
		nRight= filename.find(".sav");
	nLeft = filename.find_first_not_of('0', nLeft);
	std::string strSlot = filename.substr(nLeft, nRight-nLeft);
	return atoi(strSlot.c_str());
  };

bool IsPlayRealMovie()
{
	if( g_pFcRealMovie && g_pFcRealMovie->IsPlay() )
	{
		return true;
	}

	return false;
}

bool IsPlayEvent()
{
	if( g_InterfaceManager.IsShowLetterBox() )
	{
		return true;
	}

	return false;
}