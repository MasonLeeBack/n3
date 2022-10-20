#include "stdafx.h"
#include "FcSoundManager.h"
#include "BsKernel.h"
#include "DebugUtil.h"
#include "XMPBGMPlayer.h"
#include "FcUtil.h"
#include "Token.h"
#include "Parser.h"
#include "FcWaveStream.h"
#include "FcCommon.h"
#include "FcSOXLoader.h"
#include "BsFileIO.h"
#include "FcTroopObject.h"


#include "BsKernel.h"
#include "FcWorld.h"

#include "bstreamext.h"
#include "FcGlobal.h"
//#include ".\\data\\Sound\\NNN_SOUND.h"
#include "BsUtil.h"
#include "LocalLanguage.h"
#include "data/FXList.h"


#define _PRELOAD_XWB_FILE	"D:\\Data\\Sound\\SoundRes\\XWBPreload.pak"

FcSoundManager* g_pSoundManager = NULL;

CSoundPlayCrowdTable g_SoundPlayCrowdTable;

#if defined(_XBOX) && defined(_SOUND_ENABLE)
#define MAX_PROP_SOUND_TABLE	16
#define MAX_DISTANCE_PROP		8000.f

PropSoundInfo g_PropSoundInfo[MAX_PROP_SOUND_TABLE] = 
{
	{SPM_NONE,SPS_ATTACKED,"OB_WOOD_HIT"},
	{SPM_NONE,SPS_DESTROYED,"OB_WOOD_BREAK"},
	/*{SPM_NONE,SPS_CRASH_FLOOR_SMALL,"OB_WOOD_FALL_LOW"},
	{SPM_NONE,SPS_CRASH_FLOOR_BIG,"OB_WOOD_FALL_HIGH"},
	{SPM_NONE,SPS_CRASH_WATER_SMALL,"OB_WOOD_FALLL_WATER_S"},
	{SPM_NONE,SPS_CRASH_WATER_BIG,"OB_WOOD_FALLL_WATER_L"},*/

	{SPM_TREE_STRUCTURE,SPS_ATTACKED,"OB_WOOD_HIT"},
	{SPM_TREE_STRUCTURE,SPS_DESTROYED,"OB_WOOD_BREAK"},
	/*{SPM_TREE_STRUCTURE,SPS_CRASH_FLOOR_SMALL,"OB_WOOD_FALL_LOW"},
	{SPM_TREE_STRUCTURE,SPS_CRASH_FLOOR_BIG,"OB_WOOD_FALL_HIGH"},
	{SPM_TREE_STRUCTURE,SPS_CRASH_WATER_SMALL,"OB_WOOD_FALLL_WATER_S"},
	{SPM_TREE_STRUCTURE,SPS_CRASH_WATER_BIG,"OB_WOOD_FALLL_WATER_L"},*/

	{SPM_TREE_PIECE,SPS_ATTACKED,"OB_S_WOOD_HIT"},
	{SPM_TREE_PIECE,SPS_DESTROYED,"OB_S_WOOD_BREAK"},
	/*{SPM_TREE_PIECE,SPS_CRASH_FLOOR_SMALL,"OB_S_WOOD_FALL_LOW"},
	{SPM_TREE_PIECE,SPS_CRASH_FLOOR_BIG,"OB_S_WOOD_FALL_HIGH"},
	{SPM_TREE_PIECE,SPS_CRASH_WATER_SMALL,"OB_S_WOOD_FALLL_WATER_S"},
	{SPM_TREE_PIECE,SPS_CRASH_WATER_BIG,"OB_S_WOOD_FALLL_WATER_L"},*/


	{SPM_ROCK_PIECE,SPS_ATTACKED,"OB_BLOCK_HIT"},
	{SPM_ROCK_PIECE,SPS_DESTROYED,"OB_BLOCK_BREAK"},
	/*{SPM_ROCK_PIECE,SPS_CRASH_FLOOR_SMALL,"OB_BLOCK_FALL_LOW"},
	{SPM_ROCK_PIECE,SPS_CRASH_FLOOR_BIG,"OB_BLOCK_FALL_HIGH"},
	{SPM_ROCK_PIECE,SPS_CRASH_WATER_SMALL,"OB_BLOCK_FALLL_WATER_S"},
	{SPM_ROCK_PIECE,SPS_CRASH_WATER_BIG,"OB_BLOCK_FALLL_WATER_L"},*/

	{SPM_ROCK,SPS_ATTACKED,"OB_ROCK_HIT"},
	{SPM_ROCK,SPS_DESTROYED,"OB_ROCK_BREAK"},
	/*{SPM_ROCK,SPS_CRASH_FLOOR_SMALL,"OB_ROCK_FALL_LOW"},
	{SPM_ROCK,SPS_CRASH_FLOOR_BIG,"OB_ROCK_FALL_HIGH"},
	{SPM_ROCK,SPS_CRASH_WATER_SMALL,"OB_ROCK_FALLL_WATER_S"},
	{SPM_ROCK,SPS_CRASH_WATER_BIG,"OB_ROCK_FALLL_WATER_L"},*/

	{SPM_BRIDGE,SPS_ATTACKED,"OB_WOOD_HIT"},
	{SPM_BRIDGE,SPS_DESTROYED,"EV_WYANDEEK_02"},
	/*{SPM_BRIDGE,SPS_CRASH_FLOOR_SMALL,"OB_WOOD_FALL_LOW"},
	{SPM_BRIDGE,SPS_CRASH_FLOOR_BIG,"OB_WOOD_FALL_HIGH"},
	{SPM_BRIDGE,SPS_CRASH_WATER_SMALL,"OB_WOOD_FALLL_WATER_S"},
	{SPM_BRIDGE,SPS_CRASH_WATER_BIG,"OB_WOOD_FALLL_WATER_L"},*/

	{SPM_STONE_STATUE,SPS_ATTACKED,"OB_ROCK_HIT"},
	{SPM_STONE_STATUE,SPS_DESTROYED,"EV_VARRGANDD_02"},
	/*{SPM_STONE_STATUE,SPS_CRASH_FLOOR_SMALL,"OB_ROCK_FALL_LOW"},
	{SPM_STONE_STATUE,SPS_CRASH_FLOOR_BIG,"OB_ROCK_FALL_HIGH"},
	{SPM_STONE_STATUE,SPS_CRASH_WATER_SMALL,"OB_ROCK_FALLL_WATER_S"},
	{SPM_STONE_STATUE,SPS_CRASH_WATER_BIG,"OB_ROCK_FALLL_WATER_L"} ,  */

	{SPM_DESTROY_WALL, SPS_ATTACKED,"OB_ROCK_HIT"},
	{SPM_DESTROY_WALL, SPS_DESTROYED,"OB_ROCK_FALL_HIGH"},
	/*{SPM_DESTROY_WALL, SPS_CRASH_FLOOR_SMALL,"OB_ROCK_FALL_LOW"},
	{SPM_DESTROY_WALL, SPS_CRASH_FLOOR_BIG,"OB_ROCK_FALL_HIGH"},
	{SPM_DESTROY_WALL, SPS_CRASH_WATER_SMALL,"OB_ROCK_FALLL_WATER_S"},
	{SPM_DESTROY_WALL, SPS_CRASH_WATER_BIG,"OB_ROCK_FALLL_WATER_L"}*/
};

FcSoundManager::FcSoundManager( const char* pDir )
{

	strcpy( m_soundDir, pDir );
	int i = 0;

	memset(m_RegisterWaveBankList, 0 , sizeof(m_RegisterWaveBankList));
	memset(m_pCueName, 0 , sizeof(m_pCueName));
	memset(m_pKakegoCueName, 0 , sizeof(m_pKakegoCueName));


	m_pPreloadedXWB = NULL;
	m_dwPreloadedXWBFlags = 0;
	m_fBGMVolume = 1.f;
	m_bMissionFound = false;
	m_nAmbienceHandle = -1;
	m_bSoundSePlayingOn = true;


	memset(m_szMapName,0, 64);
	/*
	for(i = 0;i < CUT_LAST;i++){
	m_nUnitCount[i] = 0;
	}
	*/

#ifdef _OUTPUT_ERR_CUE_NAME
	memset(m_szCueErrPath,0, MAX_PATH);
#endif
	m_nBGMHandle = -1;
	m_nCurMissionIndex = -1;

	m_bBGMFadeOut = false;
	m_bAutoRestoreVolumeCtgy = false;
}


void FcSoundManager::StopSoundRM()
{
	for(int i = 0;i < MAX_CUE_NUM;i++)
	{
		if(m_nAmbienceHandle != i && m_nBGMHandle != i) 
		{		
			CXACTSound::StopSound(i,false);
		}
	}
}

void FcSoundManager::StopSoundAll()
{
	CXACTSound::StopSoundAll();
}

FcSoundManager::~FcSoundManager()
{
	Destroy();	
}

void FcSoundManager::InitSet()
{
	for(int i = 0;i < SC_CTGY_LAST;i++)
	{
		m_SkipInfo[i].Init();
	}
	m_SkipInfo[SC_HIT].nTickInterval  = 2;
	m_SkipInfo[SC_DEFENSE].nTickInterval = 2;
	m_SkipInfo[SC_RUN].nTickInterval  = 5;
	m_SkipInfo[SC_WALK].nTickInterval = 3;
	m_SkipInfo[SC_DOWN].nTickInterval = 1;
	m_SkipInfo[SC_DOWN_ROLL].nTickInterval = 1;
	m_SkipInfo[SC_ATTACK_BASIC].nTickInterval = 1;
	m_SkipInfo[SC_ADD_ORB].nTickInterval = ADD_ORB_SOUND_INTERVAL;

	m_DitanceTableByCtry[SC_COMMON]		 = 5000.f;
	m_DitanceTableByCtry[SC_WALK]		 = 2000.f;
	m_DitanceTableByCtry[SC_RUN]		 = 2500.f;
	m_DitanceTableByCtry[SC_JUMP]		 = 1500.f;
	m_DitanceTableByCtry[SC_DASH]		 = 1500.f;
	m_DitanceTableByCtry[SC_DOWN]		 = 2000.f;
	m_DitanceTableByCtry[SC_DOWN_ROLL]	 = 2000.f;
	m_DitanceTableByCtry[SC_HORSE_WALK]	 = 1500.f;
	m_DitanceTableByCtry[SC_HORSE_RUN]	 = 1500.f;
	m_DitanceTableByCtry[SC_HORSE_JUMP]  = 1500.f;
	m_DitanceTableByCtry[SC_HORSE_STOP]  = 1500.f;
	m_DitanceTableByCtry[SC_HIT]		 = 1500.f;
	m_DitanceTableByCtry[SC_DEFENSE]     = 1500.f;
	m_DitanceTableByCtry[SC_ATTACK_BASIC] = 1500.f;
	m_DitanceTableByCtry[SC_HIT]		  = 1500.f;
	m_DitanceTableByCtry[SC_ATTACK_REPEAT] = 1500.f;
	m_DitanceTableByCtry[SC_ATTACK_FINISH] = 1500.f;

	m_DitanceTableByCtry[SC_ADD_ORB] = 1500.f;
	m_DitanceTableByCtry[SC_RANDOM_30] = 1500.f;

	m_DitanceTableByCtry[SC_LEADER_ATTACK_BASIC] = 3000.f;
	m_DitanceTableByCtry[SC_LEADER_ATTACK_REPEAT] = 3000.f;
	m_DitanceTableByCtry[SC_LEADER_ATTACK_FINISH] = 3000.f;
}

void FcSoundManager::FinishStage()
{
	m_bSoundSePlayingOn = true;
	m_nAmbienceHandle = -1;
	m_nBGMHandle = -1;
	CXACTSound::FinishStage();
	DeleteCharWaveBank();	
	InitSet();
	memset(m_szMapName,0, 64);
	m_vecCrowdInfo.clear();

	if(m_nCurMissionIndex != -1)
	{
		for(int i = 0;i < MAX_MISSION_WAVEBANK;i++)
		{
			char *szWBName = m_MissionWaveBankList[m_nCurMissionIndex]->ppWBName[i];
			if( szWBName ){
				DeleteWaveBank( szWBName );
			}
		}
	}
	m_nCurMissionIndex =-1;
	m_bMissionFound = false;
}

void FcSoundManager::Destroy()
{
	CXACTSound::Clear();
	int i = 0;
	for(i = 0;i < WB_LIST_LAST;i++) {
		SAFE_DELETE( m_RegisterWaveBankList[i] );
	}
	for(i = 0;i < SC_CTGY_LAST;i++)
	{
		for(unsigned int s = 0;s < m_SoundTable[i].SoundList.size();s++){		
			delete m_SoundTable[i].SoundList[s];
		}
	}
	for(i = 0;i < MAX_CUE;i++){
		if(m_pCueName[i]){
			delete []m_pCueName[i];
		}
	}
	for(i = 0;i < MAX_CUE_KAKEGOE;i++)
	{
		if(m_pKakegoCueName[i])
			delete []m_pKakegoCueName[i];
	}

	for(i = 0;i < (int)m_MissionWaveBankList.size();i++)
	{
		for(int s = 0;s < MAX_MISSION_WAVEBANK;s++){		
			SAFE_DELETEA( m_MissionWaveBankList[i]->ppWBName[s] );
		}
		delete m_MissionWaveBankList[i];
	}

	for(i = 0;i < (int)m_vtCharWbList.size();i++) {
		SAFE_DELETEA( m_vtCharWbList[i] );
	}

	if(m_pPreloadedXWB)
	{
		CBsFileIO::FreePhysicalMemory(m_pPreloadedXWB, m_dwPreloadedXWBFlags);
		m_pPreloadedXWB = NULL;
		m_dwPreloadedXWBFlags = 0;
	}

}


void FcSoundManager::PreLoadXWB(const char *szPackedXWBName)
{
	BsAssert(m_pPreloadedXWB == 0);

	DWORD dwFileSize  = 0;

	if( FAILED( CBsFileIO::LoadFilePhysicalMemory( szPackedXWBName, (void**)&m_pPreloadedXWB, &dwFileSize, &m_dwPreloadedXWBFlags ) ) )
	{
		BsAssert( 0 && "Could not load packed XWBs" );
		return;
	}
	DWORD dwXWBCount = *((DWORD*)m_pPreloadedXWB);
	BsAssert( dwXWBCount && "no wavebanks" );

	PACKEDFILEENTRY *pPFEntry = (PACKEDFILEENTRY*)(m_pPreloadedXWB+sizeof(DWORD));

	// [PREFIX:beginmodify] 2006/2/16 junyash PS#5235 reports Local declaration of 'i' hides declaration of the same name in an outer scope.
	//for( DWORD i = 0; i < dwXWBCount; i++)
	for( DWORD dwi = 0; dwi < dwXWBCount; dwi++)
		// [PREFIX:endmodify] junyash
	{
		bool bFound = false;
		for(int i = 0;i < WB_LIST_LAST;i++)
		{
			if( !m_RegisterWaveBankList[i] ) continue;
			if(stricmp( m_RegisterWaveBankList[i]->szFileName, pPFEntry->szFileName ) == 0)
			{
				bFound = true;

				CXACTSound::RegisterIMWavebank(i, m_pPreloadedXWB + pPFEntry->dwOffset, pPFEntry->dwSize );
				break;
			}
		}
		if(!bFound)
		{
			DebugString("Load : [%s] Wavebank file was not found\n",pPFEntry->szFileName);
			BsAssert(0 && "Load : Wavebank file was not found");
		}
		pPFEntry++;

	}
}



bool FcSoundManager::Initialize(const char *szXGSName,const char *szBankListName)
{
	Destroy();

	char szFileName[MAX_PATH];
	sprintf( szFileName, "%s%s", m_soundDir, szXGSName );
	if(!CXACTSound::Initialize( szFileName ))	
		BsAssert( 0 && "XAct Init failed" );

	RegisterBankList( szBankListName ); //일단 리스트 만들어서
	ResisterMissionWaveList( "FcMissionWaveList.txt" );

	RegisterCueList("NNN_SoundCues.txt", m_pCueName, MAX_CUE );
	RegisterCueList("NNN_SoundCues_KAKEGOE.txt", m_pKakegoCueName, MAX_CUE_KAKEGOE );

	// 캐릭터 효과음과 리얼무비 등은 따로 등록
	//WaveBank 기본적인 것들 로딩

#ifdef _PACKED_RESOURCES
	PreLoadXWB(_PRELOAD_XWB_FILE);
#else
	//LoadWaveBank( WB_ANIMAL );
	LoadWaveBank( WB_COMMON_ATTACK_A );
	LoadWaveBank( WB_COMMON_ATTACK_B );
	LoadWaveBank( WB_COMMON_ATTACK_C );
	LoadWaveBank( WB_COMMON_ATTACK_D );
	LoadWaveBank( WB_COMMON_ATTACK_E );
	LoadWaveBank( WB_COMMON_ATTACK_F );
	LoadWaveBank( WB_COMMON_ATTACK_G );
	LoadWaveBank( WB_COMMON_ATTACK_H );
	LoadWaveBank( WB_COMMON_ATTACK_I );
	LoadWaveBank( WB_COMMON_ATTACK_J );
	LoadWaveBank( WB_COMMON_ATTACK_K );

	LoadWaveBank( WB_COMMON_DOWN_GRASS );
	LoadWaveBank( WB_COMMON_DOWN_SNOW );
	LoadWaveBank( WB_COMMON_DOWN_SOIL_ROCK );
	LoadWaveBank( WB_COMMON_DOWN_WATER );

	LoadWaveBank( WB_COMMON_ENEMY_99  );
	LoadWaveBank( WB_COMMON_ENEMY_9S );
	LoadWaveBank( WB_COMMON_ENEMY_DG );
	LoadWaveBank( WB_COMMON_ENEMY_DK );
	LoadWaveBank( WB_COMMON_ENEMY_GB );
	LoadWaveBank( WB_COMMON_ENEMY_KG );
	LoadWaveBank( WB_COMMON_ENEMY_LE );
	LoadWaveBank( WB_COMMON_ENEMY_PT );
	LoadWaveBank( WB_COMMON_ENEMY_PY );

	LoadWaveBank( WB_COMMON_ALLY_ALL );

	LoadWaveBank( WB_COMMON_FOOTSTEP_01_SOIL );
	LoadWaveBank( WB_COMMON_FOOTSTEP_02_ROCK );
	LoadWaveBank( WB_COMMON_FOOTSTEP_05_GRASS );
	LoadWaveBank( WB_COMMON_FOOTSTEP_07_SHOAL );
	LoadWaveBank( WB_COMMON_FOOTSTEP_08_WATER );
	LoadWaveBank( WB_COMMON_FOOTSTEP_10_SNOW );
	LoadWaveBank( WB_COMMON_FOOTSTEP_BIG );
	LoadWaveBank( WB_COMMON_FOOTSTEP_LARGE );
	LoadWaveBank( WB_COMMON_IMPACT_SOUND );

	LoadWaveBank( WB_COMMON_MOVE );
	LoadWaveBank( WB_COMMON_OB_ATTACK );
	LoadWaveBank( WB_COMMON_OB_CROWD );
	LoadWaveBank( WB_COMMON_OB_OTHER );
	LoadWaveBank( WB_COMMON_HIT_SOUND );
	LoadWaveBank( WB_COMMON_DAMMY );

	LoadWaveBank( WB_COMMON_REALMOVIE_ETC );
	LoadWaveBank( WB_COMMON_REALMOVIE_EV );
	LoadWaveBank( WB_COMMON_REALMOVIE_FW );
	LoadWaveBank( WB_COMMON_REALMOVIE_PF );
	LoadWaveBank( WB_COMMON_REALMOVIE_VF );
	LoadWaveBank( WB_COMMON_REALMOVIE_VG );
	LoadWaveBank( WB_COMMON_REALMOVIE_VR );
	LoadWaveBank( WB_COMMON_REALMOVIE_YU );
	LoadWaveBank( WB_COMMON_SYSTEM_SOUND );

	LoadWaveBank( WB_SYS_ASPHAR );
	LoadWaveBank( WB_SYS_COM );
	LoadWaveBank( WB_SYS_DWINGVATT );
	LoadWaveBank( WB_SYS_INPHYY );
	LoadWaveBank( WB_SYS_KLARRANN );
	LoadWaveBank( WB_SYS_TYURRUU );
	LoadWaveBank( WB_SYS_MYIFEE );

	LoadWaveBank( WB_KAKEGOE_PEOPLEVOICE );



#endif

	CSoundPlayTable Table;
	Table.Load();
	for(int i = 0;i < Table.GetSoundCount();i++)
	{
		SOUND_PLAY_INFO *pData = Table.GetSoundPlayTable(i);		
		SOUND_PLAY_INFO *pNewData = new SOUND_PLAY_INFO;
		memcpy( pNewData,pData,sizeof( SOUND_PLAY_INFO ));
		m_SoundTable[pData->nCtgy].SoundList.push_back( pNewData );
	}
	InitSet();
	g_SoundPlayCrowdTable.Load();



	return true;
}


void FcSoundManager::PlayKakegoe( int nCueIndex,D3DXVECTOR3 *pPos )
{
#ifdef _SOUND_CUE_NAME_OUTPUT
	DebugString("****************\nKakegoe name %s\n**********************\n",m_pKakegoCueName[nCueIndex] );
#endif
	Play3DSound( NULL, SB_ALL_KAKEGOE, m_pKakegoCueName[nCueIndex], pPos);
}

int FcSoundManager::GetMissionIndex(const char *szMissionName)
{
	for(int i = 0;i < (int)m_MissionWaveBankList.size();i++)
	{
		if( stricmp( m_MissionWaveBankList[i]->szMissionName, szMissionName ) == 0 ) {
			return i;
		}
	}
	//	DebugString("%s Mission Wavebanks couldn't load",szMissionName);
	return -1;
}

void FcSoundManager::SetMissionName(char *szBuf,const char *szMissionName)
{
	for(int i = 0;i < (int)strlen(szMissionName);i++)
	{
		if(szMissionName[i] != '.'){		
			szBuf[i] = szMissionName[i];
		}
		else
		{
			szBuf[i] = NULL;
			strupr(&szBuf[0]);
			break;
		}
	}
}


void FcSoundManager::LoadMissionWaveBank(const char *szMissionName)
{
	memset(m_szMapName,0, 64);
	SetMissionName(m_szMapName,szMissionName);	
	m_nCurMissionIndex = GetMissionIndex( m_szMapName ); //현재 로드하려는 미션의 웨이브 리스트 인덱스를 가져온다.

#ifdef _OUTPUT_ERR_CUE_NAME
	memset(	m_szCueErrPath,0, MAX_PATH );
#endif

	if(m_nCurMissionIndex != -1){
		m_bMissionFound = true;
	}
	else{
		m_bMissionFound = false;
		return;		
	}
	//DeletePrevMissionWaveBank( );


	for(int i = 0;i < MAX_MISSION_WAVEBANK;i++)
	{
		char *szWBName = m_MissionWaveBankList[m_nCurMissionIndex]->ppWBName[i];
		if( szWBName )
		{
			LoadWaveBank( szWBName );
		}
	}
	//SaveCurMissionWaveList( m_nCurMissionIndex );


#ifdef	_OUTPUT_ERR_CUE_NAME
	char szMakingPath[MAX_PATH];
	char szFindPath[MAX_PATH];
	sprintf( szMakingPath,"%sSoundCueError",g_BsKernel.GetCurrentDirectory());
	sprintf( szFindPath,"%s*.*",g_BsKernel.GetCurrentDirectory());

	CreateDirForSound( szFindPath,"SoundCueError",szMakingPath );

	sprintf( szMakingPath,"%sSoundCueError\\%s",g_BsKernel.GetCurrentDirectory(),g_LocalLanguage.GetLanguageStr());
	sprintf( szFindPath,"%sSoundCueError\\*.*",g_BsKernel.GetCurrentDirectory());

	CreateDirForSound( szFindPath,g_LocalLanguage.GetLanguageStr(),szMakingPath );

	sprintf(m_szCueErrPath,"%s\\CueErr_%s.txt",szMakingPath,m_szMapName);
	DeleteFile(m_szCueErrPath);
#endif
}


void FcSoundManager::CreateDirForSound( char *szCheckPath, const char *szKeyWord, char *szMakingPath )
{
#ifdef	_OUTPUT_ERR_CUE_NAME
	WIN32_FIND_DATA Result;
	HANDLE hHandle = FindFirstFile( szCheckPath,&Result );	

	bool bFound = false;
	if( INVALID_HANDLE_VALUE == hHandle ) {
		OutputDebugString( "FindFirstFile failed." );
	}
	else
	{
		do
		{
			if( stricmp(Result.cFileName,szKeyWord) == 0)
			{
				bFound = true;
			}
		} while( FindNextFile( hHandle, &Result ) );

		FindClose( hHandle );
	}

	if(!bFound)	{
		CreateDirectory( szMakingPath, NULL );
	}

#endif
}
// philt: The DeleteLoadedMissionWaveBank function is here so that I can properly clear out absolutely all memory
//        when leaving a stage.  It should only be called if ENABLE_FULL_STAGE_CLEANOUT is defined and should only
//        be used when looking for memory leaks.

void FcSoundManager::DeleteLoadedMissionWaveBank()
{
	/*
	for(int i = 0;i < MAX_MISSION_WAVEBANK;i++)
	{
	if ( m_PrevMissionWave.ppWBName[i] )
	{
	DeleteWaveBank( m_PrevMissionWave.ppWBName[i] );
	SAFE_DELETEA( m_PrevMissionWave.ppWBName[i] );
	}
	}

	memset(m_PrevMissionWave.szMissionName, 0, _countof(m_PrevMissionWave.szMissionName));
	memset(m_szMapName, 0, _countof(m_szMapName));

	m_bMissionFound = false;*/
}

void FcSoundManager::DeletePrevMissionWaveBank()
{
	/*
	if(m_nCurMissionIndex == -1)
	return;
	char *PrevWBName = NULL;
	char *szWBName = NULL;


	for(int i = 0;i < MAX_MISSION_WAVEBANK;i++)
	{
	for(int s = 0;s < MAX_MISSION_WAVEBANK;s++)
	{
	PrevWBName = m_PrevMissionWave.ppWBName[i];
	szWBName   = m_MissionWaveBankList[m_nCurMissionIndex]->ppWBName[s];

	if( PrevWBName != NULL && szWBName != NULL )
	{
	//DebugString( "Prev %s Cur %s\n",PrevWBName,szWBName );
	if( stricmp( PrevWBName, szWBName ) == 0 ){				
	SAFE_DELETEA( m_PrevMissionWave.ppWBName[i] );
	}
	}
	}
	if(m_PrevMissionWave.ppWBName[i]) {		
	DeleteWaveBank( m_PrevMissionWave.ppWBName[i] );
	SAFE_DELETEA( m_PrevMissionWave.ppWBName[i] );
	}
	}*/
}

void FcSoundManager::SaveCurMissionWaveList(int nMissionIndex)
{
	/*
	strcpy(m_PrevMissionWave.szMissionName,m_MissionWaveBankList[nMissionIndex]->szMissionName);
	for(int  i = 0;i < MAX_MISSION_WAVEBANK;i++)
	{
	if(m_PrevMissionWave.ppWBName[i]){		
	SAFE_DELETEA( m_PrevMissionWave.ppWBName[i] );
	}
	if(m_MissionWaveBankList[nMissionIndex]->ppWBName[i])
	{
	char *szCurWBName = m_MissionWaveBankList[nMissionIndex]->ppWBName[i];
	size_t szCurWBName_len = strlen(szCurWBName)+1; //aleksger - safe string
	m_PrevMissionWave.ppWBName[i] = new char[szCurWBName_len];
	strcpy_s( m_PrevMissionWave.ppWBName[i],szCurWBName_len, szCurWBName );
	}
	}*/

}
bool FcSoundManager::IsWaveBankLoaded(NNN_WAVEBANK_LIST BANK_DEF)
{
	if(CXACTSound::m_pWaveBank[BANK_DEF]) {	
		return true;
	}
	return false;
}

bool FcSoundManager::IsWaveBankLoaded(char *szWaveBankName)
{
	for(int i = 0;i < WB_LIST_LAST;i++)
	{
		if( !m_RegisterWaveBankList[i] ) continue;
		if(stricmp(m_RegisterWaveBankList[i]->szFileName,szWaveBankName) == 0)
		{
			if(FcSoundManager::m_pWaveBank[i]) {
				return true;
			}
		}
	}
	return false;
}

void FcSoundManager::RegisterCueList(char *szFileName, char **ppCueListBuf, const int nMaxCues)
{
	Parser parser;
	TokenList toklist;
	parser.Create();
	parser.EnableComments( true );

	bool result;
	result = parser.ProcessSource( m_soundDir, szFileName, &toklist );
	if( result == false ){
		BsAssert(0 && "cue list was not found");
	}
	result = parser.ProcessHeaders(&toklist);
	parser.ProcessMacros( &toklist );

	TokenList::iterator itr = toklist.begin();

	int nIndex = 0;
	const int szBuf_size = 64; //aleksger - safe string
	char szBuf[szBuf_size];
	int nSaveLine;
	while( nIndex < nMaxCues )
	{
		if( itr == toklist.end() ){
			break;
		}
		itr++;
		const int ppCueListBuf_nIndex_size = 32; //aleksger - safer strings
		ppCueListBuf[nIndex] = new char[ppCueListBuf_nIndex_size];

		nSaveLine = itr->GetLineNumber();
		strcpy_s( ppCueListBuf[nIndex], ppCueListBuf_nIndex_size, itr->GetVariable());itr++;

		if( itr == toklist.end() ){
			break;
		}
		if(itr->IsVariable()) //띄어쓰기 있는 이름때문에
		{			
			sprintf_s(szBuf,szBuf_size, "%s %s",ppCueListBuf[nIndex],itr->GetVariable());
			strcpy_s( ppCueListBuf[nIndex],ppCueListBuf_nIndex_size, szBuf );//aleksger - safer strings
			itr++;
		}
		if(itr->IsInteger() && nSaveLine == itr->GetLineNumber())
		{
			sprintf_s(szBuf,szBuf_size, "%s %d",ppCueListBuf[nIndex],itr->GetInteger());
			strcpy_s( ppCueListBuf[nIndex],ppCueListBuf_nIndex_size, szBuf );//aleksger - safer strings
			itr++;
		}
		nIndex++;

		BsAssert( nIndex < nMaxCues);
	}

}

bool FcSoundManager::IsFoundCueFromTable(SOUND_PLAY_INFO *pData1,SOUND_PLAY_INFO *pData2)
{
	if( pData1->nAmor != pData2->nAmor )return false;
	if( pData1->nFloor != pData2->nFloor )return false;
	if( pData1->nWeapon != pData2->nWeapon )return false;
	if( pData1->nEnemyWeapon != pData2->nEnemyWeapon )return false;
	if( pData1->nWeaponUseType != pData2->nWeaponUseType )return false;
	if( pData1->nWeight != pData2->nWeight )return false;
	if( pData1->nAttType != pData2->nAttType )return false;	

	return true;
}



int FcSoundManager::Play3DSound( HANDLE hHandle, int nType, int nCueIndex, D3DXVECTOR3* Pos,int *pHandle,int nPlayCount,bool bSetToLisn,FC_STEREO_PLAY_TYPE Stereo)
{
	if( !GetSeSoundPlayingOn() )
		return -1;
	return CXACTSound::CreateSound( hHandle,nType, nCueIndex, Pos,SC_COMMON ,nPlayCount,pHandle,bSetToLisn,Stereo);
}

int  FcSoundManager::Play3DSound( HANDLE hHandle,int nType, char *szCueName, D3DXVECTOR3* Pos,int *pHandle,int nPlayCount,bool bSetToLisn,FC_STEREO_PLAY_TYPE Stereo)
{	
	if( !GetSeSoundPlayingOn() )
		return -1;

	return CXACTSound::CreateSound( hHandle,nType, szCueName, Pos,SC_COMMON ,nPlayCount,pHandle,bSetToLisn,Stereo);
}

int  FcSoundManager::Play3DSound( HANDLE hHandle,SOUND_PLAY_INFO *pData, D3DXVECTOR3* Pos,int *pHandle,int nPlayCount,bool bSkipIgnore/* = false*/,bool bSetToLisn,FC_STEREO_PLAY_TYPE Stereo)
{
	if( !GetSeSoundPlayingOn() )
		return -1;

	int nCueIndex = -1;
	if( pData->nCtgy == SC_COMMON )
	{
		if( pData->nCueID != -1)
		{
			if(!m_pCueName[pData->nCueID]){
				DebugString( "%d Cue index is empty\n",pData->nCueID );
				BsAssert( 0 && "Sound Table reference error" );
				return -1;
			}
			return CXACTSound::CreateSound( hHandle,SB_COMMON, m_pCueName[pData->nCueID], Pos, SC_COMMON,nPlayCount,pHandle);
		}
		else
		{
			DebugString("-1 Sound Handle is found");
			return -1;
		}
	}
	else
	{
		if( pData->nCtgy >=  SC_CTGY_LAST){
			BsAssert( 0 && "Sound Table reference error" );
			return -1;
		}
		if(pData->nCtgy < 0 )
		{
			BsAssert( 0 && "Sound Ctgy -1" );
			return -1;
		}
		for(unsigned int i = 0;i < m_SoundTable[pData->nCtgy].SoundList.size();i++)
		{	
			SOUND_PLAY_INFO *ptb = m_SoundTable[pData->nCtgy].SoundList[i];
			if( IsFoundCueFromTable(ptb,pData) )
			{
				if( CheckSkip( pData->nCtgy ) && bSkipIgnore == false ) {				
					return -2;
				}

				if( ptb->nCueID < 0 || ptb->nCueID >= MAX_CUE || m_pCueName[ptb->nCueID] == NULL )
				{
					BsAssert( 0 && "Sound playing reference error" );
					DebugString("Table Info is not match(3D) Ctgy %d/ Amor %d/ Floor %d/ Weapon %d/ Enemyweapon %d/WeaponUseType %d/ AttType%d\n",
						pData->nCtgy, pData->nAmor, pData->nFloor, pData->nWeapon, pData->nEnemyWeapon,pData->nWeaponUseType,pData->nAttType );
					return -1;
				}
				return CXACTSound::CreateSound( hHandle,SB_COMMON, m_pCueName[ptb->nCueID], Pos,pData->nCtgy, nPlayCount,pHandle,bSetToLisn,Stereo);
			}
		}

		DebugString(
			"Table Info is not match(3D) Ctgy %d/ Amor %d/ Floor %d/ Weapon %d/ Enemyweapon %d/WeaponUseType %d/ AttType%d\n",
			pData->nCtgy, pData->nAmor, pData->nFloor, pData->nWeapon, pData->nEnemyWeapon,pData->nWeaponUseType,pData->nAttType );
	}	
	return -1;
}

void FcSoundManager::LoadWaveBank(char *szName)
{
	if( IsWaveBankLoaded(szName) == true) {
		DebugString("%s Wavebank loading skipped\n",szName);
		return;		
	}

	for(int i = 0;i < WB_LIST_LAST;i++)
	{
		if( !m_RegisterWaveBankList[i] ) continue;
		if(stricmp( m_RegisterWaveBankList[i]->szFileName, szName ) == 0)
		{
			char cStr[256];
			sprintf( cStr, "Load wave bank %s", szName );
			g_BsMemChecker.Start( cStr );

			LoadWaveBank((NNN_WAVEBANK_LIST)i);

			g_BsMemChecker.End();
			return;
		}
	}
	DebugString("Load : [%s] Wavebank file was not found\n",szName);
	BsAssert(0 && "Load : Wavebank file was not found");
}

void FcSoundManager::LoadWaveBank(NNN_WAVEBANK_LIST BANK_DEF)
{
	if( IsWaveBankLoaded(BANK_DEF) == true) {
		DebugString("%d Wavebank loading skipped\n",BANK_DEF);
		return;		
	}

	char szFullName[MAX_PATH];
	if( !m_RegisterWaveBankList[BANK_DEF] ) return;
	sprintf( szFullName, "%sSoundRes\\%s.xwb", m_soundDir,  m_RegisterWaveBankList[BANK_DEF]->szFileName );

	char cStr[256];
	sprintf( cStr, "Load wave bank %s", m_RegisterWaveBankList[BANK_DEF]->szFileName );
	g_BsMemChecker.Start( cStr );

	CXACTSound::LoadWaveBank((int)BANK_DEF,
		szFullName,
		m_RegisterWaveBankList[BANK_DEF]->bUseStream);

	g_BsMemChecker.End();
}

void FcSoundManager::DeleteWaveBank(char const *szName)
{
	for(int i = 0;i < WB_LIST_LAST;i++)
	{
		if( !m_RegisterWaveBankList[i] ) continue;
		if(stricmp( m_RegisterWaveBankList[i]->szFileName, szName ) == 0)
		{
			DeleteWaveBank( (NNN_WAVEBANK_LIST)i );
			//CXACTSound::ReserveDeleteWaveBank( (NNN_WAVEBANK_LIST)i );
			return;
		}
	}
	BsAssert(0 && "Delete : Wavebank file was not found");
}

void FcSoundManager::DeleteWaveBank(NNN_WAVEBANK_LIST BANK_DEF)
{
	CXACTSound::DeleteWaveBank( (int)BANK_DEF );
	//CXACTSound::ReserveDeleteWaveBank( (NNN_WAVEBANK_LIST)BANK_DEF );
}

void FcSoundManager::ResisterMissionWaveList(const char *szBGMListName)
{

	Parser parser;
	TokenList toklist;
	bool result;
	parser.Create();
	parser.EnableComments( true );

	result = parser.ProcessSource( m_soundDir, szBGMListName, &toklist );
	if( result == false ){
		BsAssert(0 && "wavebank list was not found");
	}

	result = parser.ProcessHeaders(&toklist);
	parser.ProcessMacros( &toklist );

	TokenList::iterator itr = toklist.begin();    
	if(result == false) {
		BsAssert(0 && "wavebank list header was not found");
	}

	while( 1 )
	{
		if( itr == toklist.end() ){
			break;
		}

		if(itr->IsVariable())
		{
			itr++;
			MISSION_WAVEBANK_INFO *pBGMInfo = new MISSION_WAVEBANK_INFO;
			strcpy( pBGMInfo->szMissionName,itr->GetString());
			strupr( pBGMInfo->szMissionName );
			itr++;

			for(int i = 0;i < MAX_MISSION_WAVEBANK;i++)
			{
				if(itr->IsString()) {
					size_t itr_len = strlen(itr->GetString())+1; // aleksger - safe string
					pBGMInfo->ppWBName[i] = new char[itr_len];
					strcpy_s(pBGMInfo->ppWBName[i],itr_len,itr->GetString());					
				}
				else if(itr->IsVariable()) {				
					if(strcmp(itr->GetVariable(),"END") == 0) {
						m_MissionWaveBankList.push_back( pBGMInfo );
						break;
					}						
				}
				itr++;
			}

			BsAssert( !itr->IsString() );
		}
		itr++;
	}
}

void FcSoundManager::RegisterBankList(const char *szXACtBankFileName )
{

	Parser parser;
	TokenList toklist;
	bool result;
	bool bWaveBank = true;
	int  nIndex = -1;
	char szFullName[MAX_PATH];
	bool bUseStream = false;
	//------------------------------------------------------------------------------------

	parser.Create();
	parser.EnableComments( true );

	result = parser.ProcessSource( m_soundDir, szXACtBankFileName, &toklist );
	if( result == false ){
		BsAssert(0 && "wavebank list was not found");
	}

	result = parser.ProcessHeaders(&toklist);
	parser.ProcessMacros( &toklist );


	TokenList::iterator itr = toklist.begin();    
	if(result == false) {
		BsAssert(0 && "wavebank list header was not found");
	}

	while( 1 )
	{
		if( itr == toklist.end() ){
			break;
		}
		if(itr->IsVariable())
		{
			if(strcmp(itr->GetVariable(),"SOUND_BANK") == 0) {
				bWaveBank = false;
				itr++;
			}
			else if(strcmp(itr->GetVariable(),"WAVE_BANK") == 0)
			{
				bWaveBank = true;
				itr++;
			}
		}


		char szFileName[64];
		nIndex = itr->GetInteger(); itr++;		
		strcpy( szFileName, itr->GetString() ); itr++;
		sprintf( szFullName, "%sSoundRes\\%s", m_soundDir,  szFileName );

		if(bWaveBank){			
			strcat( szFullName,".xwb" );
			bUseStream = false;
			if( itr->IsBoolean() ) { bUseStream = itr->GetBoolean(); itr++; }
			m_RegisterWaveBankList[nIndex] = new FC_REGISTER_LIST_INFO;
			m_RegisterWaveBankList[nIndex]->bUseStream = bUseStream;
			strcpy( m_RegisterWaveBankList[nIndex]->szFileName, szFileName);			
		}
		else {
			strcat( szFullName,".xsb" );
			CXACTSound::LoadSoundBank(nIndex , szFullName );
		}
	}
}


void FcSoundManager::EnableAutoRestoreVolCtgy( bool bEnable )
{
	m_bAutoRestoreVolumeCtgy = bEnable;
}

void FcSoundManager::SetVolumeAll( float fVol )
{
	SetVolume(FC_MENU_VOL_VOICE,fVol );
	SetVolume(FC_MENU_VOL_BGM,fVol );
	SetVolume(FC_MENU_VOL_EFT_SOUND,fVol);
}


void FcSoundManager::Reset()
{
}
///////////////////////////////////////////////////////////////////////////////////////
// 배경음악 volume 줄이고 나머지 다 중지
void FcSoundManager::Pause()
{

}

void FcSoundManager::StartStopFade(int nIndex)
{
	if(nIndex >= 0 && nIndex < MAX_CUE_NUM )
	{
		m_CueList[nIndex].FadeState = FSPT_FADE_OUT_STOP;
		m_CueList[nIndex].fFadeDelta = 60.f;
	}
}

void FcSoundManager::Process()
{
	ProcessCrowdSound();
	CXACTSound::Process();

	if( m_bAutoRestoreVolumeCtgy )
	{
		m_bAutoRestoreVolumeCtgy = false;
		RestoreVolCtgy();				
	}
	if( m_nBGMHandle != -1 ){ //플레이 중이고
		if( IsOwner( (HANDLE)&SOUND_BGM_DUMMY_ADDRESS, m_nBGMHandle ) == false) { //BGM 내부적으로 끝난 상황이면

			// fade out 처리
			if( m_bBGMFadeOut )
			{
				if( m_CueList[m_nBGMHandle].FadeState != FSPT_FADE_OUT_STOP ){
					m_nBGMHandle = -1;
					m_bBGMFadeOut = false;
				}
			}

			// 루프 처리
			if( m_nBGMHandle != -1 )
			{
				if( m_SaveBGMInfo.nLoop ){ //루프체크되어있으면
					D3DXVECTOR3 Pos = GetListenerPos();
					m_nBGMHandle = Play3DSound((HANDLE)&SOUND_BGM_DUMMY_ADDRESS,SB_MUSIC_BANK,m_SaveBGMInfo.nCueIndex,&Pos,NULL,1,false,FSPT_STEREO_FADE_EFT);
					SetFadeInValue( m_nBGMHandle, m_SaveBGMInfo.fFadeTick );
				}
				else{			
					m_nBGMHandle = -1;
				}
			}
		}
	}
}


int FcSoundManager::PlaySound( SOUND_BANK_TYPE Type, int cueID ,int *pHandle, int nPlayCount )
{
	if( !GetSeSoundPlayingOn() )
		return -1;
	return CXACTSound::CreateSound( NULL,Type, cueID,NULL,SC_COMMON,nPlayCount,pHandle);
}

int FcSoundManager::PlaySound( SOUND_BANK_TYPE Type, char* pCueName,int *pHandle, int nPlayCount)
{
	if( !GetSeSoundPlayingOn() )
		return -1;
	return CXACTSound::CreateSound( NULL,Type, pCueName,NULL,SC_COMMON,nPlayCount,pHandle);
}

void FcSoundManager::PlayAmbience( int nCueID )
{
	StopAmbience();
	D3DXVECTOR3 Pos = GetListenerPos();
	m_nAmbienceHandle = CXACTSound::CreateSound( NULL,SB_COMMON, m_pCueName[nCueID],&Pos,SC_COMMON,PLAY_COUNT_LOOP,&m_nAmbienceHandle,false,FSPT_STEREO_FADE_EFT);
}

void FcSoundManager::StopAmbience()
{
	if ( m_nAmbienceHandle != -1 )
	{
		CXACTSound::StopSound( m_nAmbienceHandle );
		m_nAmbienceHandle = -1;
	}
}

void FcSoundManager::PauseBGM( BOOL bPause )
{
	CXACTSound::Pause( m_nBGMHandle, bPause );
}




int FcSoundManager::GetCurMissionIndex()
{
	BsAssert( m_nCurMissionIndex != -1 && "Mission not found in sound manager" );
	return m_nCurMissionIndex;
}

void FcSoundManager::StopBGM( bool bFadeOut )
{
	if( strlen(m_szMapName) && m_bMissionFound == false )
		return;

	if( IsHandleValid( m_nBGMHandle ) ){

		CXACTSound::StopSound( m_nBGMHandle , bFadeOut);
		if( bFadeOut == false )
		{
			m_nBGMHandle = -1;
		}
		else
		{
			m_bBGMFadeOut = true;
		}
	}		
}

void FcSoundManager::PlayBGM(char *szBGMName)
{
	if( strlen(m_szMapName) && m_bMissionFound == false )
		return;

	StopBGM();
	m_SaveBGMInfo.nCueIndex = -1;
	m_SaveBGMInfo.nLoop		= 1;
	m_SaveBGMInfo.fFadeTick = _SOUND_DEFAULT_FADE_TIME;	
	D3DXVECTOR3 Pos = GetListenerPos();

	m_SaveBGMInfo.nCueIndex = GetCueIndex(SB_MUSIC_BANK , szBGMName);
	m_nBGMHandle = Play3DSound((HANDLE)&SOUND_BGM_DUMMY_ADDRESS,SB_MUSIC_BANK,szBGMName,&Pos,NULL,1,false,FSPT_STEREO_FADE_EFT);
	SetFadeInValue( m_nBGMHandle,_SOUND_DEFAULT_FADE_TIME );
}

void FcSoundManager::PlayBGM(int nIndex,int nLoop,float fFadeTick/* = _SOUND_DEFAULT_FADE_TIME*/)
{
	if( strlen(m_szMapName) && m_bMissionFound == false )
		return;

	StopBGM();

	char *pBGMName = m_MissionWaveBankList[GetCurMissionIndex()]->ppWBName[nIndex];	
	m_SaveBGMInfo.nLoop		= nLoop;
	m_SaveBGMInfo.fFadeTick = fFadeTick;

	if(pBGMName){
		m_SaveBGMInfo.nCueIndex = GetCueIndex( SB_MUSIC_BANK, pBGMName );
		D3DXVECTOR3 Pos = GetListenerPos();
		m_nBGMHandle = Play3DSound((HANDLE)&SOUND_BGM_DUMMY_ADDRESS,SB_MUSIC_BANK,pBGMName,&Pos,NULL,1,false,FSPT_STEREO_FADE_EFT);
		SetFadeInValue( m_nBGMHandle,fFadeTick );
	}
	else{
		DebugString("BGM Play Error : Index was excess max");
	}
}

bool FcSoundManager::IsBGMPlay()
{   
	return (m_nBGMHandle != -1);
}



void FcSoundManager::SetVolumeBGM(int nPercent)
{
	SetVolume( FC_MENU_VOL_BGM , ((float)nPercent / 100.f) );
}


void FcSoundManager::SetListenerPos( D3DXVECTOR3 &Pos, D3DXVECTOR3 &Eye )
{
	CXACTSound::SetListenerPos( Pos, Eye );
}

void FcSoundManager::SetListenerPos( D3DXVECTOR3 &Pos, bool bSavePos )
{
	CXACTSound::SetListenerPos( Pos, bSavePos );
}


void FcSoundManager::SetEmitterPos(int nIndex,D3DXVECTOR3 *Pos)
{
	CXACTSound::SetEmitterPos( nIndex, Pos );
}

void FcSoundManager::MuteBGM(bool bMute)
{
	if(bMute)
	{
		m_fBGMVolume = GetVolume( FC_MENU_VOL_BGM );
		SetVolume( FC_MENU_VOL_BGM,0.f );
	}
	else{
		SetVolume( FC_MENU_VOL_BGM, m_fBGMVolume );
	}



	//m_pStreamPlayer->Mute( bMute );
}

void FcSoundManager::BGMFade(int nTick,int nTargetPercent )
{
	//m_pStreamPlayer->Fade( nTick, nTargetPercent);
	if( m_nBGMHandle != -1 )
	{
		ChangeFadeMode(m_nBGMHandle , nTargetPercent / 100.f ,nTick );
	}
}

void FcSoundManager::Stop3DSound(int nHandle)
{
	CXACTSound::StopSound( nHandle );
}

void FcSoundManager::Stop2DSound(int nHandle)
{
	//CXACTSound::Stop2DSound( nHandle );
	CXACTSound::StopSound( nHandle );
}

void FcSoundManager::StopFade3DSound(int nHandle)
{
	//CXACTSound::StopFade3DSound( nHandle );
	CXACTSound::StopFadeSound( nHandle );
}

float FcSoundManager::GetDistanceLimit()
{
	return CXACTSound::GetDistanceLimit();
}

D3DXVECTOR3 FcSoundManager::GetListenerPos(int nPlayerIndex)
{
	return CXACTSound::GetListenerPos();
}


void  FcSoundManager::SetVolumeCtgy(XACT_VOLUME_CATEGORY Category,float fVolume)
{
	CXACTSound::SetVolumeCtgy(Category, fVolume);
}
float FcSoundManager::GetVolumeCtgy(XACT_VOLUME_CATEGORY Category)
{
	return CXACTSound::GetVolumeCtgy(Category);
}
void  FcSoundManager::SaveVolCtgy()
{
	//BsAssert( m_bAutoRestoreVolumeCtgy == false && "Sound volume auto restore" );
	if( m_bAutoRestoreVolumeCtgy )
	{
		DebugString( "[Restore vol ctgy] It's safe to do only once\n" );
	}
	CXACTSound::SaveVolCtgy();
}

void  FcSoundManager::RestoreVolCtgy()
{
	//BsAssert( m_bAutoRestoreVolumeCtgy == false && "Sound volume auto restore" );
	if( m_bAutoRestoreVolumeCtgy )
	{
		DebugString( "[Restore vol ctgy] It's safe to do only once\n" );
	}

	CXACTSound::RestoreVolCtgy();
}

void FcSoundManager::DeleteCharWaveBank()
{
	for( unsigned int i = 0;i < m_vtCharWbList.size();i++ ){
		if( m_vtCharWbList[i] )
		{
			DeleteWaveBank( m_vtCharWbList[i] );
			SAFE_DELETEA( m_vtCharWbList[i] );
		}
	}
	m_vtCharWbList.clear();
}

void FcSoundManager::LoadCharWaveBank( char *szWBName )
{
	if( stricmp( szWBName,"noname" ) == 0 ){ return; }

	for( unsigned int i = 0;i < m_vtCharWbList.size();i++ ){	
		if( stricmp( m_vtCharWbList[i],szWBName ) == 0 ){		//이미 해당미션에서 중복됐으면
			return;
		}
	}
	char *pBuf = new char[ strlen(szWBName) + 1 ];
	strcpy_s(pBuf,strlen(szWBName) + 1,szWBName);
	m_vtCharWbList.push_back( pBuf );
	LoadWaveBank( szWBName );

}

void FcSoundManager::ResetUnitCount()
{
	m_vecCrowdInfo.clear();
	/*
	m_CrowdPos = D3DXVECTOR3(0,0,0);
	for(int i = 0;i < CUT_LAST;i++){	
	m_nUnitCount[i] = 0;
	}
	*/
}

void FcSoundManager::AddCrowdInfo( TroopObjHandle hTroop )
{
	/*
	SOUND_CROWD_DATA Data;
	Data.nUnitNum = nUnitNum;
	Data.nType = nType;
	Data.m_Pos = * pPos;
	*/
	m_vecCrowdInfo.push_back( hTroop );
}


D3DXVECTOR3 FcSoundManager::GetCrowdPos( D3DXVECTOR3* pPos )
{	
	D3DXVECTOR3 CrowdPos;
	if( D3DXVec3Length( &(*pPos - GetListenerPos()) ) > (X3DAUDIO_DISTANCE_TRIGGER * 0.6f))
	{		
		D3DXVECTOR3 Dir = *pPos - GetListenerPos();
		D3DXVec3Normalize(&Dir,&Dir);
		CrowdPos = GetListenerPos() + (Dir * (X3DAUDIO_DISTANCE_TRIGGER * 0.6f));		
		return CrowdPos;		
	}	
	return *pPos;

}

void  FcSoundManager::ProcessCrowdSound()
{
	if( GetProcessTick() % 40 != 7 )
		return;

	std::vector<CROWD_SOUND_DATA>	vecTempCrowdGroup;

	// Make group
	int nCnt = m_vecCrowdInfo.size();
	int nGroupIndex = 0;
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTroop = m_vecCrowdInfo[i];
		if( hTroop->IsEliminated() || hTroop->IsEnable() == false )
			continue;

		bool bGroupExist = false;
		int nTempCnt = vecTempCrowdGroup.size();
		for( int j=0; j<nTempCnt; j++ )
		{
			CROWD_SOUND_DATA* pTempData = &(vecTempCrowdGroup[j]);
			if( pTempData->nType == hTroop->GetCrowdUnitType() )
			{
				D3DXVECTOR3 Dir = hTroop->GetPos() - pTempData->Pos;
				float fDist = D3DXVec3LengthSq( &Dir );

				if( fDist < 8000.f * 8000.f )	// 범위 안에 있다면 같은 그룹으로 간주
				{
					float fTroopNum = (float)pTempData->nTroopNum;
					++(pTempData->nTroopNum);
					D3DXVECTOR3 NewPos = pTempData->Pos * fTroopNum;
					NewPos = NewPos + hTroop->GetPos();
					pTempData->Pos = NewPos / (float)pTempData->nTroopNum;
					pTempData->nFirstUnitNum += hTroop->GetAliveUnitCount();
					bGroupExist = true;
					break;
				}
			}
		}
		// 아무곳에도 속하는 곳이 없다면 그룹 만들어 줌
		if( bGroupExist == false )
		{
			CROWD_SOUND_DATA Data;
			Data.nFirstUnitNum = hTroop->GetAliveUnitCount();
			Data.Pos = hTroop->GetPos();
			Data.nTroopNum = 1;
			Data.nType = hTroop->GetCrowdUnitType();
			vecTempCrowdGroup.push_back( Data );
		}
	}

	int nCrowdCnt = m_vecCrowdSoundData.size();
	for( int j=0; j<nCrowdCnt; j++ )
	{
		m_vecCrowdSoundData[j].bCheck = false;
	}

	// 그룹 나눈 것으로 현재 등록된 그룹 사운드 조절
	nCnt = vecTempCrowdGroup.size();
	for( int i=0; i<nCnt; i++ )
	{
		CROWD_SOUND_DATA* pTempData = &(vecTempCrowdGroup[i]);

		bool bExist = false;;
		int nCrowdCnt = m_vecCrowdSoundData.size();
		for( int j=0; j<nCrowdCnt; j++ )
		{
			CROWD_SOUND_DATA* pCurData = &(m_vecCrowdSoundData[j]);
			if( pCurData->nType != pTempData->nType )
				continue;

			D3DXVECTOR3 Dir = pCurData->Pos - pTempData->Pos;
			float fDist = D3DXVec3LengthSq( &Dir );
			if( fDist < 5000.f * 5000.f )
			{
				pCurData->Pos = pTempData->Pos;
				pCurData->nFirstUnitNum = pTempData->nFirstUnitNum;
				pCurData->nTroopNum = pTempData->nTroopNum;


				int nTableIndex = 0;
				int nCrowdTableCount = g_SoundPlayCrowdTable.GetTableCount();
				for( int k=0; k<nCrowdTableCount; k++ )
				{
					SOUND_CROWD_INFO *pCrowdTableData = g_SoundPlayCrowdTable.GetTableData(k);
					if( pCrowdTableData->UnitType != pCurData->nType )
						continue;

					if( pCurData->nFirstUnitNum * 3 < pCrowdTableData->nUnitCount )
					{
						if( pCurData->nSoundHandle[0][nTableIndex] != -1 )
						{
							StopFade3DSound( pCurData->nSoundHandle[0][nTableIndex] );
							pCurData->nSoundHandle[0][nTableIndex] = -1;
						}

						if( pCurData->nSoundHandle[1][nTableIndex] != -1 )
						{
							StopFade3DSound( pCurData->nSoundHandle[1][nTableIndex] );
							pCurData->nSoundHandle[1][nTableIndex] = -1;
						}
					}
					else
					{
						if( pCurData->nSoundHandle[0][nTableIndex] == -1 )
						{
							if(stricmp( pCrowdTableData->pPlayCue[0],"noname" ) != 0) //지정된 사운드가 있으면
							{
								D3DXVECTOR3 NewPos = GetCrowdPos( &(pCurData->Pos) );
								pCurData->nSoundHandle[0][nTableIndex] = Play3DSound( NULL, SB_AMBI_CROWD, pCrowdTableData->pPlayCue[0], &NewPos );
							}
						}
						else
						{
							D3DXVECTOR3 NewPos = GetCrowdPos( &(pCurData->Pos) );
							SetEmitterPos( pCurData->nSoundHandle[0][nTableIndex], &NewPos );

							//							SetEmitterPos( pCurData->nSoundHandle[0][nTableIndex], &(pCurData->Pos) );
						}
						if( pCurData->nSoundHandle[1][nTableIndex] == -1 )
						{
							if(stricmp( pCrowdTableData->pPlayCue[1],"noname" ) != 0) //지정된 사운드가 있으면
							{
								D3DXVECTOR3 NewPos = GetCrowdPos( &(pCurData->Pos) );
								pCurData->nSoundHandle[1][nTableIndex] = Play3DSound( NULL, SB_AMBI_CROWD, pCrowdTableData->pPlayCue[1], &NewPos );
							}
						}
						else
						{
							D3DXVECTOR3 NewPos = GetCrowdPos( &(pCurData->Pos) );
							SetEmitterPos( pCurData->nSoundHandle[1][nTableIndex], &NewPos );

							//							SetEmitterPos( pCurData->nSoundHandle[1][nTableIndex], &(pCurData->Pos) );
						}
					}

					++nTableIndex;
				}
				bExist = true;
				pCurData->bCheck = true;
				break;
			}
		}
		if( bExist == false )
		{
			pTempData->bCheck = true;
			m_vecCrowdSoundData.push_back( *pTempData );
		}
	}

	// 리스트에 없는 부대는 없애 버리기
	nCrowdCnt = m_vecCrowdSoundData.size();
	for( int j=0; j<nCrowdCnt; j++ )
	{
		CROWD_SOUND_DATA* pData = &(m_vecCrowdSoundData[j]);
		if( pData->bCheck )
			continue;

		for( int i=0; i<5; i++ )
		{
			if( pData->nSoundHandle[0][i] != -1 )
				StopFade3DSound( pData->nSoundHandle[0][i] );

			if( pData->nSoundHandle[1][i] != -1 )
				StopFade3DSound( pData->nSoundHandle[1][i] );
		}

		m_vecCrowdSoundData.erase( m_vecCrowdSoundData.begin() + j );
		--nCrowdCnt;
		--j;
	}


	/*
	int nCount = g_SoundPlayCrowdTable.GetTableCount();

	D3DXVECTOR3 CrowdPos;

	for(int i = 0; i < nCount; i++)
	{
	SOUND_CROWD_INFO *pData = g_SoundPlayCrowdTable.GetTableData(i);

	for(int s = 0;s < 2;s++) {
	if( pData->nSoundHandle[s] == -1) { //사운드가 없고
	if( pData->nUnitCount < m_nUnitCount[ pData->UnitType ] ) //유닛카운트가 넘어가면
	{
	if(stricmp( pData->pPlayCue[s],"noname" ) != 0) { //지정된 사운드가 있으면
	CrowdPos = GetCrowdPos();
	pData->nSoundHandle[s] = Play3DSound( NULL, SB_AMBI_CROWD,pData->pPlayCue[s], &CrowdPos );
	}
	}
	}
	else{ //사운드가 나오고 있고
	if( pData->nUnitCount > m_nUnitCount[ pData->UnitType ] ) {//유닛카운트가 모자라면				
	if(pData->nSoundHandle[s] != -1)
	{
	StopFade3DSound( pData->nSoundHandle[s] );
	pData->nSoundHandle[s] = -1;
	}
	}
	else
	{
	if( pData->nSoundHandle[s] != -1) {
	CrowdPos = GetCrowdPos();
	SetEmitterPos( pData->nSoundHandle[s], &CrowdPos );
	}
	}
	}
	}
	}
	*/
}


bool FcSoundManager::IsInDistanceLimit(int nCtgy, const D3DXVECTOR3 &EmitterPos)
{
	//m_DitanceTableByCtry[nCtgy]

	if( nCtgy == -1 )
	{
		BsAssert(nCtgy != -1 && "Category not defined");
		return false;
	}

	float fLength = D3DXVec3Length(&(GetListenerPos() - EmitterPos));
	if( fLength < m_DitanceTableByCtry[nCtgy] ){	
		return true;
	}
	return false;

}

bool FcSoundManager::IsInDistanceLimit(const D3DXVECTOR3 &EmitterPos,float fTemp)
{
	return CXACTSound::IsInDistanceLimit( EmitterPos ,fTemp);
}

int FcSoundManager::GetCueIndex(int nType,char *szCueName)
{
	return CXACTSound::GetCueIndex(nType,szCueName);
}


void FcSoundManager::SetPauseAll(BOOL bPause)
{
	CXACTSound::SetPauseAll(bPause);
}

float FcSoundManager::GetVolume(FC_MENU_VOL Type)
{
	switch(Type)
	{
	case FC_MENU_VOL_EFT_SOUND:
		return GetVolumeCtgy(XACT_VC_MOVE); //이거 하나만 해도 다 똑같기 때문에
		break;
	case FC_MENU_VOL_BGM:
		return GetVolumeCtgy(XACT_VC_BGM); //이거 하나만 해도 다 똑같기 때문에
		break;
	case FC_MENU_VOL_VOICE:
		return GetVolumeCtgy(XACT_VC_VOICE); //이거 하나만 해도 다 똑같기 때문에
		break;
	}
	return 0.f;
}

void FcSoundManager::SetVolume(FC_MENU_VOL Type,float fVol)
{
	switch(Type)
	{
	case FC_MENU_VOL_EFT_SOUND:		
		CXACTSound::SetVolumeCtgy( XACT_VC_MOVE,fVol );
		CXACTSound::SetVolumeCtgy( XACT_VC_ATTACK,fVol );
		CXACTSound::SetVolumeCtgy( XACT_VC_CROWD_AMBIENCE,fVol );
		CXACTSound::SetVolumeCtgy( XACT_VC_FOOT,fVol );
		CXACTSound::SetVolumeCtgy( XACT_VC_HIT,fVol );
		CXACTSound::SetVolumeCtgy( XACT_VC_AMBIENCE,fVol );
		CXACTSound::SetVolumeCtgy( XACT_VC_OBJECT,fVol );
		CXACTSound::SetVolumeCtgy( XACT_VC_SYSTEM,fVol );
		CXACTSound::SetVolumeCtgy( XACT_VC_REALMOVE,fVol );
		CXACTSound::SetVolumeCtgy( XACT_VC_ENEMY,fVol );
		CXACTSound::SetVolumeCtgy( XACT_VC_EVENT,fVol );	
		break;
	case FC_MENU_VOL_BGM:
		CXACTSound::SetVolumeCtgy( XACT_VC_MUSIC,fVol );
		break;
	case FC_MENU_VOL_VOICE:
		CXACTSound::SetVolumeCtgy(XACT_VC_VOICE,fVol);
		break;
	}

}

bool FcSoundManager::CheckSkip( int nCtgy )
{
	SOUND_SKIP_INFO *pInfo = &m_SkipInfo[nCtgy];
	if(( GetProcessTick() - pInfo->nLastFrame ) < pInfo->nTickInterval ){
		return true;
	}
	else{
		pInfo->nLastFrame = GetProcessTick();
		return false;
	}
}

int FcSoundManager::PlaySystemSound( SOUND_BANK_TYPE Type, char* pCueName ,int nCtgy ,D3DXVECTOR3 *pPos, HANDLE hHandle )
{
	if( CheckSkip( nCtgy ) )
		return -1;

	if(pPos){
		return CXACTSound::CreateSound( hHandle,Type,pCueName,pPos,SC_COMMON,1,NULL,true, FSPT_MONO );
		//return Play3DSound( hHandle,Type,pCueName,pPos,NULL,1,true,FSPT_MONO);
	}
	else{
		D3DXVECTOR3 Pos = GetListenerPos();
		return CXACTSound::CreateSound( hHandle,Type,pCueName,&Pos,SC_COMMON,1,NULL,true, FSPT_MONO );

		//return Play3DSound( hHandle,Type,pCueName,&Pos,NULL,1,true,FSPT_MONO );
	}
	return -1;
}

void FcSoundManager::LogCueErr( int nType,char *szCueName )
{
#ifdef	_OUTPUT_ERR_CUE_NAME
	if( strlen(m_szCueErrPath) )
	{
		char szOutPut[256];
		memset(szOutPut,0,256);

		FILE *fp = fopen(m_szCueErrPath,"a");
// [PREFIX:beginmodify] 2006/2/25 junyash PS#5825 dereferencing NULL pointer
		if (fp)
		{
			sprintf(szOutPut,"SoundBank %d Sound Cue [ %s ]\n",nType,szCueName);
			fprintf(fp,szOutPut);
			fclose(fp);
		}
// [PREFIX:endmodify] junyash
	}

#endif
}

bool FcSoundManager::IsOwner( HANDLE hHandle,int nIndex )
{
	return CXACTSound::IsOwner( hHandle, nIndex );
}

void FcSoundManager::PlayPropSound(D3DXVECTOR3 *pPos, SOUND_PROP_MATERIAL Material, SOUND_PROP_STATE State ,bool bBridge )
{
	if( bBridge ){	
		Material = SPM_BRIDGE;
		if(pPos) {		
			float fLength = D3DXVec3Length(&(*pPos - GetListenerPos()));
			if(fLength > MAX_DISTANCE_PROP ) {		
				return;
			}
		}
	}


	for(int i = 0;i < MAX_PROP_SOUND_TABLE;i++)
	{
		if( g_PropSoundInfo[i].nPropMaterial == Material && g_PropSoundInfo[i].nPropState == State )
		{
			switch( Material )
			{
			case SPM_STONE_STATUE:
			case SPM_DESTROY_WALL:
			case SPM_BRIDGE:
				if( State == SPS_DESTROYED) {					
					PlaySystemSound( SB_COMMON ,g_PropSoundInfo[i].szCueName );
				}
				else{
					Play3DSound( NULL, SB_COMMON ,g_PropSoundInfo[i].szCueName,pPos );
				}				
				break;
			default:
				Play3DSound( NULL, SB_COMMON ,g_PropSoundInfo[i].szCueName,pPos );
			}

			/*
			if(b3D)
			{
				PlaySystemSound( SB_COMMON ,g_PropSoundInfo[i].szCueName );
			}
			else{
				PlaySystemSound( SB_COMMON ,g_PropSoundInfo[i].szCueName );
			}*/
			/*switch( g_PropSoundInfo[i].nPropMaterial )
			{
			case SPM_STONE_STATUE:
			case SPM_DESTROY_WALL:
			case SPM_BRIDGE:
				if(State == SPS_DESTROYED) {
					PlaySystemSound( SB_COMMON ,g_PropSoundInfo[i].szCueName );
				}
				else{
					Play3DSound( NULL, SB_COMMON ,g_PropSoundInfo[i].szCueName,pPos );
				}
				break;
			default:
				Play3DSound( NULL, SB_COMMON ,g_PropSoundInfo[i].szCueName,pPos );
				break;
			}*/
			return;
		}
	}
}

void FcSoundManager::PlayFxSound( int nFxID,D3DXVECTOR3 *pPos )
{
	switch(nFxID)
	{
	case FX_99BALL01: //99군단병꺼 터지는 거
		PlayNPCSound( EM_9S_ATTACK_02 ,pPos );
		break;
	case FX_MIRA01BOOM: //미라바리스 불
		PlayNPCSound( EM_MM_ATTACK_01_B ,pPos );		
		break;
	case FX_MIRA02_2AIRBOOM: //미라바리스
		PlayNPCSound( EM_MM_ATTACK_03 ,pPos );
		break;
	case FX_GBMADO01:
		PlayNPCSound( EM_GP_MAGIC_02 ,pPos );
		break;
	case FX_ESPROOTS: //
		PlayNPCSound( EM_YR_ATTACK_01 ,pPos );
		break;

	case FX_ESPVOLCANO: //에스파라타 용암
		PlayNPCSound( EM_YR_ATTACK_02 ,pPos );
		break;
	case FX_99K07TENTACLE: //99일밤왕 뭔가??
		break;
	case FX_99K03SPHEARB: //99일밤왕 땅에서 솟는 원 폭발
		PlayNPCSound( EM_MM_ATTACK_02 ,pPos );		
		break;
	}
	DebugString( "FxSound - FxID %d\n\n", nFxID );
}

void FcSoundManager::PlayNPCSound( NPC_SOUND_ID nSoundID,D3DXVECTOR3 *pPos )
{
	//case FX_GBMADO01:  //고블린 마도승
	switch(nSoundID)
	{
	case EM_99_GUARD:	 Play3DSound(NULL,SB_COMMON,"EM_99_GUARD",pPos);
	case EM_99_ATTACK_01:	 Play3DSound(NULL,SB_COMMON,"EM_99_ATTACK_01",pPos);
	case EM_99_ATTACK_02:	 Play3DSound(NULL,SB_COMMON,"EM_99_ATTACK_02",pPos);
	case EM_99_ATTACK_03:	 Play3DSound(NULL,SB_COMMON,"EM_99_ATTACK_03",pPos);
	case EM_99_ATTACK_04:	 Play3DSound(NULL,SB_COMMON,"EM_99_ATTACK_04",pPos);
	case EM_99_ATTACK_05:	 Play3DSound(NULL,SB_COMMON,"EM_99_ATTACK_05",pPos);
	case EM_99_WARP_01:	 Play3DSound(NULL,SB_COMMON,"EM_99_WARP_01",pPos);
	case EM_99_WARP_02:	 Play3DSound(NULL,SB_COMMON,"EM_99_WARP_02",pPos);
	case EM_99_DASH_01:	 Play3DSound(NULL,SB_COMMON,"EM_99_DASH_01",pPos);
	case EM_99_DASH_02:	 Play3DSound(NULL,SB_COMMON,"EM_99_DASH_02",pPos);
	case EM_99_DASH_03:	 Play3DSound(NULL,SB_COMMON,"EM_99_DASH_03",pPos);
	case EM_99_FLY_01:	 Play3DSound(NULL,SB_COMMON,"EM_99_FLY_01",pPos);
	case EM_99_FLY_02:	 Play3DSound(NULL,SB_COMMON,"EM_99_FLY_02",pPos);
	case EM_99_FLY_03:	 Play3DSound(NULL,SB_COMMON,"EM_99_FLY_03",pPos);
	case EM_99_SUMMON_02:	 Play3DSound(NULL,SB_COMMON,"EM_99_SUMMON_02",pPos);
	case EM_99_HIT_01:	 Play3DSound(NULL,SB_COMMON,"EM_99_HIT_01",pPos);
	case EM_99_HIT_02:	 Play3DSound(NULL,SB_COMMON,"EM_99_HIT_02",pPos);
	case EM_99_HIT_03:	 Play3DSound(NULL,SB_COMMON,"EM_99_HIT_03",pPos);
	case EM_99_HIT_04:	 Play3DSound(NULL,SB_COMMON,"EM_99_HIT_04",pPos);
	case EM_99_STAND_01:	 Play3DSound(NULL,SB_COMMON,"EM_99_STAND_01",pPos);
	case EM_99_WALK_01:	 Play3DSound(NULL,SB_COMMON,"EM_99_WALK_01",pPos);
	case EM_DF_DASH_01:	 Play3DSound(NULL,SB_COMMON,"EM_DF_DASH_01",pPos);
	case EM_DF_DASH_02:	 Play3DSound(NULL,SB_COMMON,"EM_DF_DASH_02",pPos);
	case EM_DF_WALK_01:	 Play3DSound(NULL,SB_COMMON,"EM_DF_WALK_01",pPos);
	case EM_DF_RUN_01:	 Play3DSound(NULL,SB_COMMON,"EM_DF_RUN_01",pPos);
	case EM_DF_DEFENSE_01:	 Play3DSound(NULL,SB_COMMON,"EM_DF_DEFENSE_01",pPos);
	case EM_DF_DEFENSE_02:	 Play3DSound(NULL,SB_COMMON,"EM_DF_DEFENSE_02",pPos);
	case EM_DF_ATTACK_01:	 Play3DSound(NULL,SB_COMMON,"EM_DF_ATTACK_01",pPos);
	case EM_DF_ATTACK_02:	 Play3DSound(NULL,SB_COMMON,"EM_DF_ATTACK_02",pPos);
	case EM_DF_ATTACK_03:	 Play3DSound(NULL,SB_COMMON,"EM_DF_ATTACK_03",pPos);
	case EM_DF_ATTACK_04:	 Play3DSound(NULL,SB_COMMON,"EM_DF_ATTACK_04",pPos);
	case EM_DF_ATTACK_05:	 Play3DSound(NULL,SB_COMMON,"EM_DF_ATTACK_05",pPos);
	case EM_LE_GUARD_01:	 Play3DSound(NULL,SB_COMMON,"EM_LE_GUARD_01",pPos);
	case EM_LE_GUARD_02:	 Play3DSound(NULL,SB_COMMON,"EM_LE_GUARD_02",pPos);
	case EM_LE_WALK:		 Play3DSound(NULL,SB_COMMON,"EM_LE_WALK",pPos);
	case EM_LE_RUN:			Play3DSound(NULL,SB_COMMON,"EM_LE_RUN",pPos);
	case EM_LE_ATTACK_01:	 Play3DSound(NULL,SB_COMMON,"EM_LE_ATTACK_01",pPos);
	case EM_LE_ATTACK_02:	 Play3DSound(NULL,SB_COMMON,"EM_LE_ATTACK_02",pPos);
	case EM_LE_ATTACK_03:	 Play3DSound(NULL,SB_COMMON,"EM_LE_ATTACK_03",pPos);
	case EM_LE_ATTACK_04:	 Play3DSound(NULL,SB_COMMON,"EM_LE_ATTACK_04",pPos);
	case EM_LE_ATTACK_05:	 Play3DSound(NULL,SB_COMMON,"EM_LE_ATTACK_05",pPos);
	case EM_LE_ATTACK_06:	 Play3DSound(NULL,SB_COMMON,"EM_LE_ATTACK_06",pPos);
	case EM_LE_ATTACK_07:	 Play3DSound(NULL,SB_COMMON,"EM_LE_ATTACK_07",pPos);
	case EM_LE_ATTACK_08:	 Play3DSound(NULL,SB_COMMON,"EM_LE_ATTACK_08",pPos);
	case EM_YR_ATTACK_01:	 Play3DSound(NULL,SB_COMMON,"EM_YR_ATTACK_01",pPos);
	case EM_YR_ATTACK_02:	 Play3DSound(NULL,SB_COMMON,"EM_YR_ATTACK_02",pPos);
	case EM_YR_ATTACK_03:	 Play3DSound(NULL,SB_COMMON,"EM_YR_ATTACK_03",pPos);
	case EM_YR_ATTACK_04:	 Play3DSound(NULL,SB_COMMON,"EM_YR_ATTACK_04",pPos);
	case EM_YR_DASH_01:		 Play3DSound(NULL,SB_COMMON,"EM_YR_DASH_01",pPos);
	case EM_YR_DASH_02:		 Play3DSound(NULL,SB_COMMON,"EM_YR_DASH_02",pPos);
	case EM_YR_GUARD_01:	 Play3DSound(NULL,SB_COMMON,"EM_YR_GUARD_01",pPos);
	case EM_PY_ATTACK_01:	 Play3DSound(NULL,SB_COMMON,"EM_PY_ATTACK_01",pPos);
	case EM_PY_ATTACK_02:	 Play3DSound(NULL,SB_COMMON,"EM_PY_ATTACK_02",pPos);
	case EM_PY_ATTACK_03:	 Play3DSound(NULL,SB_COMMON,"EM_PY_ATTACK_03",pPos);
	case EM_PY_ATTACK_04:	 Play3DSound(NULL,SB_COMMON,"EM_PY_ATTACK_04",pPos);
	case EM_PY_ATTACK_05:	 Play3DSound(NULL,SB_COMMON,"EM_PY_ATTACK_05",pPos);
	case EM_PY_BACK_01:	     Play3DSound(NULL,SB_COMMON,"EM_PY_BACK_01",pPos);
	case EM_PY_DASH_01:		 Play3DSound(NULL,SB_COMMON,"EM_PY_DASH_01",pPos);
	case EM_PT_FOOT:		 Play3DSound(NULL,SB_COMMON,"EM_PT_FOOT",pPos);
	case EM_PT_ATTACK_01:	 Play3DSound(NULL,SB_COMMON,"EM_PT_ATTACK_01",pPos);
	case EM_PT_ATTACK_02:	 Play3DSound(NULL,SB_COMMON,"EM_PT_ATTACK_02",pPos);
	case EM_PT_ATTACK_03:	 Play3DSound(NULL,SB_COMMON,"EM_PT_ATTACK_03",pPos);
	case EM_PT_ATTACK_04:	 Play3DSound(NULL,SB_COMMON,"EM_PT_ATTACK_04",pPos);
	case EM_PT_ATTACK_05:	 Play3DSound(NULL,SB_COMMON,"EM_PT_ATTACK_05",pPos);
	case EM_GP_VOICE:		 Play3DSound(NULL,SB_COMMON,"EM_GP_VOICE",pPos);
	case EM_GP_DEAD:		 Play3DSound(NULL,SB_COMMON,"EM_GP_DEAD",pPos);
	case EM_GP_MAGIC_01:	 Play3DSound(NULL,SB_COMMON,"EM_GP_MAGIC_01",pPos);
	case EM_GP_MAGIC_02:	 Play3DSound(NULL,SB_COMMON,"EM_GP_MAGIC_02",pPos);
	case EM_GP_MAGIC_03:	 Play3DSound(NULL,SB_COMMON,"EM_GP_MAGIC_03",pPos);
	case EM_GP_MAGIC_04:	 Play3DSound(NULL,SB_COMMON,"EM_GP_MAGIC_04",pPos);
	case EM_GP_MAGIC_05:	 Play3DSound(NULL,SB_COMMON,"EM_GP_MAGIC_05",pPos);
	case EM_KG_ATTACK_01:	 Play3DSound(NULL,SB_COMMON,"EM_KG_ATTACK_01",pPos);
	case EM_KG_ATTACK_02:	 Play3DSound(NULL,SB_COMMON,"EM_KG_ATTACK_02",pPos);
	case EM_KG_ATTACK_03:	 Play3DSound(NULL,SB_COMMON,"EM_KG_ATTACK_03",pPos);
	case EM_KG_ATTACK_04:	 Play3DSound(NULL,SB_COMMON,"EM_KG_ATTACK_04",pPos);
	case EM_KG_ATTACK_05:	 Play3DSound(NULL,SB_COMMON,"EM_KG_ATTACK_05",pPos);
	case EM_KG_ATTACK_06:	 Play3DSound(NULL,SB_COMMON,"EM_KG_ATTACK_06",pPos);
	case EM_SM_WALK_01:		 Play3DSound(NULL,SB_COMMON,"EM_SM_WALK_01",pPos);
	case EM_SM_HIT_01:		 Play3DSound(NULL,SB_COMMON,"EM_SM_HIT_01",pPos);
	case EM_SM_HIT_02:		 Play3DSound(NULL,SB_COMMON,"EM_SM_HIT_02",pPos);
	case EM_SM_ATTACK_01:	 Play3DSound(NULL,SB_COMMON,"EM_SM_ATTACK_01",pPos);
	case EM_SM_ATTACK_02:	 Play3DSound(NULL,SB_COMMON,"EM_SM_ATTACK_02",pPos);
	case EM_SM_ATTACK_03:	 Play3DSound(NULL,SB_COMMON,"EM_SM_ATTACK_03",pPos);
	case EM_SM_ATTACK_04:	 Play3DSound(NULL,SB_COMMON,"EM_SM_ATTACK_04",pPos);
	case EM_SM_ATTACK_05:	 Play3DSound(NULL,SB_COMMON,"EM_SM_ATTACK_05",pPos);
	case EM_SM_ATTACK_06:	 Play3DSound(NULL,SB_COMMON,"EM_SM_ATTACK_06",pPos);
	case EM_SM_ATTACK_07:	 Play3DSound(NULL,SB_COMMON,"EM_SM_ATTACK_07",pPos);
	case EM_SM_DASH_01:		 Play3DSound(NULL,SB_COMMON,"EM_SM_DASH_01",pPos);
	case EM_MM_WALK_01:		 Play3DSound(NULL,SB_COMMON,"EM_MM_WALK_01",pPos);
	case EM_MM_HIT_01:		 Play3DSound(NULL,SB_COMMON,"EM_MM_HIT_01",pPos);
	case EM_MM_HIT_02:		 Play3DSound(NULL,SB_COMMON,"EM_MM_HIT_02",pPos);
	case EM_MM_ATTACK_01_A:	 Play3DSound(NULL,SB_COMMON,"EM_MM_ATTACK_01a",pPos);
	case EM_MM_ATTACK_01_B:	 Play3DSound(NULL,SB_COMMON,"EM_MM_ATTACK_01b",pPos);
	case EM_MM_ATTACK_02:	 Play3DSound(NULL,SB_COMMON,"EM_MM_ATTACK_02",pPos);
	case EM_MM_ATTACK_03:	 Play3DSound(NULL,SB_COMMON,"EM_MM_ATTACK_03",pPos);
	case EM_9S_WALK_01:		 Play3DSound(NULL,SB_COMMON,"EM_9S_WALK_01",pPos);
	case EM_9S_ATTACK_01:	 Play3DSound(NULL,SB_COMMON,"EM_9S_ATTACK_01",pPos);
	case EM_9S_ATTACK_02:	 Play3DSound(NULL,SB_COMMON,"EM_9S_ATTACK_02",pPos);
	case EM_TT_ATTACK_01:	 Play3DSound(NULL,SB_COMMON,"EM_TT_ATTACK_01",pPos);
	}

}



#endif //_XBOX
