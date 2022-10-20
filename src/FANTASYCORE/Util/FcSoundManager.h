#pragma		once
#include "FcSndFX.h"
#include "XActSound.h"
#include ".\\data\\Sound\\FcSoundPlayDefinitions.h"
#include ".\\data\\Sound\\FcWaveSoundBankDef.h"
#include "CWavebankStream.h"
#include "FcSOXLoader.h"
#include "SmartPtr.h"

#ifndef _LTCG
#define	_OUTPUT_ERR_CUE_NAME //If this definition is enabled, output error cue list
#define _SOUND_CUE_NAME_OUTPUT
#endif


class CFcTroopObject;
typedef CSmartPtr<CFcTroopObject>	TroopObjHandle;


#define ADD_ORB_SOUND_INTERVAL	10

#define _SOUND_ENABLE
#define _SOUND_DEFAULT_FADE_TIME	80.f
#define _FC_MAX_BGM_HANDLE			3
#define NUM_MAX_WAVE_BANK			30
#define MAX_CUE						1100
#define MAX_CUE_KAKEGOE				1300

struct SOUND_PLAY_INFO;

class CFcGameObject;

struct CROWD_SOUND_DATA
{
	CROWD_SOUND_DATA()
	{
		Pos = D3DXVECTOR3( 0.f, 0.f, 0.f );
		nType = -1;
		for( int i=0; i<2; i++ ) {
			for( int j=0; j<5; j++ ) nSoundHandle[i][j] = -1;
		}
		nFirstUnitNum = 0;
		nTroopNum = 0;
		bCheck = false;
	}

	D3DXVECTOR3 Pos;
	int nType;
	int nSoundHandle[2][5];		// 2종류, 동시에 낼 수 있는 소리 최대 5개
	int nFirstUnitNum;
	int nTroopNum;
	bool bCheck;
};


const int SOUND_BGM_DUMMY_ADDRESS = -1;


struct FC_REGISTER_LIST_INFO{
    char szFileName[64];
	bool bUseStream;
	FC_REGISTER_LIST_INFO()
	{
		szFileName[0] = 0;
		bUseStream = false;
	};
};

struct SOUND_PLAY_TABLE{
	std::vector<SOUND_PLAY_INFO *>SoundList;
};

struct SOUND_SKIP_INFO
{
	int nLastFrame;
	int nTickInterval;
	SOUND_SKIP_INFO()
	{
		Init();
	}
	void Init()
	{
		nLastFrame = 0;
		nTickInterval = 0;
	}
};

enum FC_MENU_VOL
{
	FC_MENU_VOL_EFT_SOUND,
	FC_MENU_VOL_BGM,
	FC_MENU_VOL_VOICE	
};

struct FC_STEREO_PLAY_INFO
{
	int nLoop;
	float fFadeTick;
	int nCueIndex;
	FC_STEREO_PLAY_INFO()
	{
		nCueIndex = -1;
		nLoop = 1;
		fFadeTick = 0;
	}
};



class CXMPBGMPlayer;
class CFcWaveStream;

#define MAX_MISSION_WAVEBANK		32

struct MISSION_WAVEBANK_INFO
{
	char szMissionName[64];
	char *ppWBName[MAX_MISSION_WAVEBANK];
	MISSION_WAVEBANK_INFO() {	
		szMissionName[0] = NULL;
		for(int i = 0;i < MAX_MISSION_WAVEBANK;i++) {		
			ppWBName[i] = NULL;
		}
	}
};

#if defined(_XBOX) && defined(_SOUND_ENABLE)

class FcSoundManager : public CXACTSound
{
public:	
	FcSoundManager( const char* pDir );
	~FcSoundManager();
	void Destroy();

	bool Initialize(const char *szXGSName,const char *szBankListName);
	void PreLoadXWB(const char *szPackedXWBName);
	void FinishStage();
	void StopSoundAll();

	const char* GetDataDir()					{ return m_soundDir; }
	void		SetDataDir( const char* dir )	{ strcpy( m_soundDir, dir ); }

	//void MuteSound( );
	void SetVolumeAll( float fVol );
	int  PlaySound( SOUND_BANK_TYPE Type, int nCueIndex,int *pHandle = NULL,int nPlayCount = 1);
	int  PlaySound( SOUND_BANK_TYPE Type, char* pCueName,int *pHandle = NULL,int nPlayCount = 1);
	void PlayKakegoe( int nCueIndex ,D3DXVECTOR3 *pPos);

	void MuteBGM(bool bMute);
	void Reset();
	void Pause();
	void Resume();
	void Process();

	int  Play3DSound( HANDLE hHandle, int nType, int nCueIndex, D3DXVECTOR3* Pos,int *pHandle = NULL,int nPlayCount = 1,bool bSetToLisn = false,FC_STEREO_PLAY_TYPE Stereo = FSPT_MONO);
	int  Play3DSound( HANDLE hHandle,SOUND_PLAY_INFO *pData, D3DXVECTOR3* Pos,int *pHandle = NULL,int nPlayCount = 1,bool bSkipIgnore = false,bool bSetToLisn = false,FC_STEREO_PLAY_TYPE Stereo = FSPT_MONO);
	int  Play3DSound( HANDLE hHandle,int nType, char *szCueName, D3DXVECTOR3* Pos,int *pHandle = NULL,int nPlayCount = 1,bool bSetToLisn = false,FC_STEREO_PLAY_TYPE Stereo = FSPT_MONO);

	void SetEmitterPos(int nIndex,D3DXVECTOR3 *pPos);
	void StartStopFade(int nIndex);
	void SetListenerPos(D3DXVECTOR3 &Pos, bool bSavePos);
	void SetListenerPos(D3DXVECTOR3 &Pos, D3DXVECTOR3 &Eye);
	

	//BGM Functions
	void PlayBGM(int nIndex,int bLoop = 1,float fFadeTick = _SOUND_DEFAULT_FADE_TIME);
	void PlayBGM(char *szBGMName);
	void StopBGM( bool bFadeOut = false );
	//Ambience Functions
	void PlayAmbience( int nCueID );
	void StopAmbience();
	bool IsAmbiencePlay(){ return m_nAmbienceHandle != -1; }

	void SetVolumeBGM(int nPercent);
	int GetBGMVolume();


	void BGMFade(int nTick,int nTargetPercent );

	void Stop2DSound(int nHandle);
	void Stop3DSound(int nHandle);
	void StopFade3DSound(int nHandle);

	float GetDistanceLimit();
	D3DXVECTOR3 GetListenerPos(int nPlayerIndex = 0);
	void LoadWaveBank(NNN_WAVEBANK_LIST BANK_DEF);
	void LoadWaveBank(char *szName);
	void DeleteWaveBank(NNN_WAVEBANK_LIST BANK_DEF);
	void DeleteWaveBank(char const *szName);
	void LoadMissionWaveBank(const char *szMissionName);
	void DeleteLoadedMissionWaveBank();
	void SetVolumeCtgy(XACT_VOLUME_CATEGORY Category,float fVolume);
	float GetVolumeCtgy(XACT_VOLUME_CATEGORY Category);
	void  SaveVolCtgy();
	void  RestoreVolCtgy();
	void InitSet();
	void ProcessCrowdSound();
	bool IsInDistanceLimit(const D3DXVECTOR3 &EmitterPos,float fTemp = 1.f);
	bool IsInDistanceLimit(int nCtgy, const D3DXVECTOR3 &EmitterPos);

	int  GetCueIndex(int nType,char *szCueName);
	void SetPauseAll(BOOL bPause);

	float GetVolume(FC_MENU_VOL Type);
	void SetVolume(FC_MENU_VOL Type,float fVol);
	bool IsMissionWaveBankSuccess(){return m_bMissionFound;}
	char *GetMissionName(){ return m_szMapName; }
	void LogCueErr( int nType,char *szCueName );
	void LoadCharWaveBank(char *szWBName);
	void DeleteCharWaveBank();
	void ResetUnitCount();
	void AddCrowdInfo( TroopObjHandle hTroop );
	void PauseBGM( BOOL bPause );
	int PlaySystemSound(SOUND_BANK_TYPE Type, char* pCueName,int nCtgy = SC_COMMON,D3DXVECTOR3 *pPos = NULL, HANDLE hHandle = NULL);
	void PlayPropSound(D3DXVECTOR3 *pPos, SOUND_PROP_MATERIAL Material, SOUND_PROP_STATE State ,bool bBridge = false);
	bool IsBGMPlay();
	bool IsOwner( HANDLE hHandle,int nIndex );
	//New
	void EnableAutoRestoreVolCtgy( bool bEnable );
	
	void PlayFxSound( int nFxID,D3DXVECTOR3 *pPos );
	void PlayNPCSound( NPC_SOUND_ID nSoundID,D3DXVECTOR3 *pPos );
	void StopSoundRM();
	void SetSeSoundPlayingOn(bool bPlaying){ m_bSoundSePlayingOn = bPlaying; }
	bool GetSeSoundPlayingOn(){ return m_bSoundSePlayingOn; }

protected:
    void RegisterBankList(const char *szXACtBankFileName );
	void ResisterMissionWaveList(const char *szBGMListName);
	void RegisterCueList(char *szFileName,char **ppCueListBuf, const int nMaxCues);
	bool IsFoundCueFromTable(SOUND_PLAY_INFO *pData1,SOUND_PLAY_INFO *pData2);
	bool IsWaveBankLoaded(NNN_WAVEBANK_LIST BANK_DEF);
	bool IsWaveBankLoaded(char *szWaveBankName);
	int  GetMissionIndex(const char *szMissionName);
	void DeletePrevMissionWaveBank();
	void SaveCurMissionWaveList(int nMissionIndex);
	void SetMissionName(char *szBuf,const char *szMissionName);	
	D3DXVECTOR3 GetCrowdPos( D3DXVECTOR3* pPos );
	bool IsHandleValid(int nIndex){ return (nIndex != -1); }
	int GetCurMissionIndex();
	bool CheckSkip( int nCtgy );
	void CreateDirForSound( char *szCheckPath, const char *szKeyWord, char *szMakingPath );
	

protected:
	char								m_soundDir[MAX_PATH];
	FC_REGISTER_LIST_INFO				*m_RegisterWaveBankList[WB_LIST_LAST];
	SOUND_PLAY_TABLE					m_SoundTable[SC_CTGY_LAST];
	char								*m_pCueName[MAX_CUE];
	char								*m_pKakegoCueName[MAX_CUE_KAKEGOE];

	std::vector<MISSION_WAVEBANK_INFO *>m_MissionWaveBankList;
	//MISSION_WAVEBANK_INFO				m_PrevMissionWave;
	char								m_szMapName[64];	
	SOUND_SKIP_INFO						m_SkipInfo[SC_CTGY_LAST];
	
	FC_STEREO_PLAY_INFO					m_SaveBGMInfo;
	int									m_nBGMHandle;
	int									m_nAmbienceHandle;

	bool								m_bBGMFadeOut;

	float								m_fBGMVolume;
	bool								m_bMissionFound;
	std::vector<char *>					m_vtCharWbList;
//	int									m_nUnitCount[CUT_LAST];

	std::vector<TroopObjHandle>			m_vecCrowdInfo;
	std::vector<CROWD_SOUND_DATA>		m_vecCrowdSoundData;

	float								m_DitanceTableByCtry[SC_CTGY_LAST];
	char								*m_pPreloadedXWB;
	DWORD								m_dwPreloadedXWBFlags;
//	D3DXVECTOR3							m_CrowdPos;
	int									m_nCurMissionIndex;

#ifdef _OUTPUT_ERR_CUE_NAME
	char								m_szCueErrPath[MAX_PATH];
#endif
	bool								m_bAutoRestoreVolumeCtgy;
	bool								m_bSoundSePlayingOn;
};

#else

class FcSoundManager
{
public:
	FcSoundManager( const char* pDir ){};
	~FcSoundManager(){};
	void Destroy(){};

	bool Initialize(const char *szXGSName,const char *szBankListName){ return true; };
	void PreLoadXWB(const char *szPackedXWBName){};
	void FinishStage(){};

	void SetVolumeAll(float fVol ){};
	int  PlaySound( SOUND_BANK_TYPE Type, int nCueIndex,int *pHandle = NULL,int nPlayCount = 1){return -1; }
	int  PlaySound( SOUND_BANK_TYPE Type, char* pCueName,int *pHandle = NULL,int nPlayCount = 1){ return -1; };	
	void PlayKakegoe( int nCueIndex ){};

	void MuteBGM(bool bMute){};
	void Reset(){};
	void Pause(){};
	void Resume(){};
	void Process(){};

	int  Play3DSound( HANDLE hHandle, int nType, int nCueIndex, D3DXVECTOR3* Pos,int *pHandle = NULL,int nPlayCount = 1,bool bSetToLisn = false){ return -1; };
	int  Play3DSound( HANDLE hHandle,SOUND_PLAY_INFO *pData, D3DXVECTOR3* Pos,int *pHandle = NULL,int nPlayCount = 1,bool bSkipIgnore = false,bool bSetToLisn = false){ return -1; };
	int  Play3DSound( HANDLE hHandle,int nType, char *szCueName, D3DXVECTOR3* Pos,int *pHandle = NULL,int nPlayCount = 1,bool bSetToLisn = false){ return -1; };

	int  Play3DSound( int nType, int nCueIndex, D3DXVECTOR3* Pos,int *pHandle = NULL,int nPlayCount = 1,bool bSetToLisn = false){ return -1; };
	int  Play3DSound( SOUND_PLAY_INFO *pData, D3DXVECTOR3* Pos,int *pHandle = NULL,int nPlayCount = 1,bool bSkipIgnore = false,bool bSetToLisn = false){ return -1; };
	int  Play3DSound( int nType, char *szCueName, D3DXVECTOR3* Pos,int *pHandle = NULL,int nPlayCount = 1,bool bSetToLisn = false){ return -1; };


	void SetEmitterPos(int nIndex,D3DXVECTOR3 *pPos){};
	void SetListenerPos(D3DXVECTOR3 &Pos, bool bSavePos){};
	void SetListenerPos(D3DXVECTOR3 &Pos, D3DXVECTOR3 &Eye){};


	//BGM Functions
	void PlayBGM(int nIndex,BOOL bLoop){};
	void PlayBGM(char *szBGMName){};
	void StopBGM( bool bFadeOut = true ){};
	//Ambience Functions
	void PlayAmbience( int nCueID ){};
	void StopAmbience(){};
	bool IsAmbiencePlay(){ return false; }

	void SetVolumeBGM(int nPercent){};
	int GetBGMVolume(){return -1;}


	void BGMFade(int nTick,int nTargetPercent ){};

	void Stop2DSound(int nHandle){};
	void Stop3DSound(int nHandle){};
	void StopFade3DSound(int nHandle){};

	float GetDistanceLimit(){return -1.f;}
	D3DXVECTOR3 GetListenerPos(int nPlayerIndex = 0){ return D3DXVECTOR3( 0.f, 0.f, 0.f ); };
	void LoadWaveBank(NNN_WAVEBANK_LIST BANK_DEF){};
	void LoadWaveBank(char *szName){};
	void DeleteWaveBank(NNN_WAVEBANK_LIST BANK_DEF){};
	void DeleteWaveBank(char const *szName){};
	void LoadMissionWaveBank(const char *szMissionName){};
	void DeleteLoadedMissionWaveBank(){};
	void SetVolumeCtgy(XACT_VOLUME_CATEGORY Category,float fVolume){};
	float GetVolumeCtgy(XACT_VOLUME_CATEGORY Category){return 0.f;}
	void SaveVolCtgy(){};
	void RestoreVolCtgy(){};
	void ProcessCrowdSound(){};
	bool IsInDistanceLimit(const D3DXVECTOR3 &EmitterPos,float fTemp = 1.f){ return false; }
	bool IsInDistanceLimit(int nCtgy, const D3DXVECTOR3 &EmitterPos){ return false; }
	int  GetCueIndex(int nType,char *szCueName){return -1;}
	void SetPauseAll(BOOL bPause){};
	void PlayKakegoe( int nCueIndex ,D3DXVECTOR3 *pPos) {}

	float GetVolume(FC_MENU_VOL Type) { return 0.f; };
	void SetVolume(FC_MENU_VOL Type,float fVol) {};
	bool IsMissionWaveBankSuccess(){return false;}

	void LogCueErr( int nType,char *szCueName ) {};
	void LoadCharWaveBank(char *szWBName) {};
	void DeleteCharWaveBank() {};
	void ResetUnitCount() {};
	void AddCrowdInfo( TroopObjHandle hTroop ) {};
	void PauseBGM( BOOL bPause ) {};
	int PlaySystemSound(SOUND_BANK_TYPE Type, char* pCueName,int nCtgy = SC_COMMON,D3DXVECTOR3 *pPos = NULL, HANDLE hHandle = NULL){ return -1;};
	bool IsBGMPlay(){ return false; }
	bool IsOwner( HANDLE hHandle,int nIndex ){ return false; }
	void EnableAutoRestoreVolCtgy( bool bEnable ) {}
	void PlayPropSound( D3DXVECTOR3 *pPos, SOUND_PROP_MATERIAL Material, SOUND_PROP_STATE State ){};
	void StopSoundRM(){};
	void StopSoundAll(){};

};

#endif //_XBOX

extern FcSoundManager* g_pSoundManager;

