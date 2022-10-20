#pragma once

#define _PI						        (3.14f)
#define VOLUME_CURVE_RESOLUTION         (128)                      // 2 이상의 임의의 값으로, 값을 크게 하면 정도(精度)가 올라갑니다
#define MAX_CUE_NUM						180


// field range
#define X3DAUDIO_DISTANCE_EMITER		(50*100)
#define X3DAUDIO_DISTANCE_TRIGGER		(50*100)
const DWORD CHANNELCOUNT		 = 6;
#ifdef USE_DEBUG_TOOLS
//#define _DEBUG_SOUND_CODE
//#define _DEBUG_POSITION_ERR
#endif



enum XACT_VOLUME_CATEGORY
{
	XACT_VC_NONE = -1,
	XACT_VC_MUSIC,
	XACT_VC_MOVE,
	XACT_VC_ATTACK,
	XACT_VC_CROWD_AMBIENCE,
	XACT_VC_FOOT,
	XACT_VC_HIT,
	XACT_VC_VOICE,
	XACT_VC_AMBIENCE,
	XACT_VC_OBJECT,
	XACT_VC_SYSTEM,
	XACT_VC_BGM,
	XACT_VC_REALMOVE,
	XACT_VC_ENEMY,	
	XACT_VC_EVENT,
	XACT_VC_NUM
};

#ifdef _XBOX
#include <XAct.h>
#include <X3DAudio.h>
#include <list>
#include <vector>
#include "Atgutil.h"
#include ".\\data\\Sound\\FcWaveSoundBankDef.h"


class CFcGameObject;
//using namespace ATG;
// This keeps track of a playing cue instance (for display purposes)

const int PLAY_COUNT_LOOP	=	-100;


enum FC_STEREO_PLAY_TYPE
{
	FSPT_MONO = -1,
	FSPT_STEREO_NORMAL,
	FSPT_STEREO_FADE_EFT,
};

enum FC_STEREO_PLAY_STATE
{
	FSPT_FADE_NONE,
	FSPT_FADE_IN,
	FSPT_FADE_IN_FINISH,
	FSPT_FADE_OUT_STOP,
	FSPT_FADE_OUT_FINISH,
	FSPT_FADE_ONLY,


};

struct CueInstance
{
	IXACTCue*     pCue;
	bool		  bEnable;
	bool		b3D;	

	short		  nPlayCount;
	int			 *pHandle;

	//3D SoundData
	XMVECTOR	Pos;
	XMVECTOR	OldPos;

	short		  nSoundBank;
	short		  nCueIndex;
	short		  nCtgy;
	bool		bFadeStop;
	bool		bSetToLisnPos;
	bool			bOutPutString;

	FC_STEREO_PLAY_TYPE StereoPlayType;
	float		fFadeDelta;
	int			nFadeCount;

	//int			nTestIdentify;
	CFcGameObject *pObject;
	HANDLE		Handle;
	FC_STEREO_PLAY_STATE FadeState;
	void Init()
	{
		pCue = NULL;
		bEnable = false;
		pHandle = NULL;
		nPlayCount = 0;
		b3D = false;
		bFadeStop = false;
		nCtgy = -1;
		bSetToLisnPos = false;
		//nTestIdentify = -1;
		pObject = NULL;
		Handle = NULL;
		//bStereo = false;
		FadeState = FSPT_FADE_NONE;
		StereoPlayType = FSPT_MONO;
		fFadeDelta = 0;
		nFadeCount = 0;
	}

	CueInstance()
	{
		Init();
	}
};

struct VolumeInfo
{
    XACTCATEGORY XActCtgy;
    float		 fVolume;
};

class CXACTSound
{
public:
	CXACTSound();
	virtual ~CXACTSound();
	bool Initialize( const char* pGSFileName );
	void FinishStage();

	void LoadWaveBank(int nWaveBankIdx, const char* pFileName, bool useStream = false );
	void RegisterIMWavebank(int nWaveBankIdx, VOID* pbWaveBank, const DWORD dwFileSize );
	void LoadSoundBank(int nSoundBankIdx, const char* pFileName );

	void ReserveDeleteWaveBank( int nWaveBankIdx );

	void DeleteWaveBank(int nWaveBankIdx);
	virtual void Process();
	void ProcessFadeInOut();

	int CreateSound( HANDLE hHandle, int nType, int nCueIndex, D3DXVECTOR3 *pPos,int nCtgy,int nPlayCount = 1,int *pHandle = NULL,bool bSetLisnPos = false,FC_STEREO_PLAY_TYPE Stereo = FSPT_MONO);
	int CreateSound( HANDLE hHandle, int nType, char *szCueName, D3DXVECTOR3 *pPos,int nCtgy,int nPlayCount = 1,int *pHandle = NULL,bool bSetLisnPos = false,FC_STEREO_PLAY_TYPE Stereo = FSPT_MONO);

	
	//Set
	void SetEmitterPos(int nIndex,D3DXVECTOR3 *pPos);
	void SetVolumeCtgy(XACT_VOLUME_CATEGORY Category,float fVolume); //나중에 인터페이스 쪽에서 쓸 듯..
	void SetListenerPos(D3DXVECTOR3 &Pos, bool bSavePos);

	void SetListenerPos( D3DXVECTOR3 &Pos,D3DXVECTOR3 &Eye );

	//Get
	float GetVolumeCtgy(XACT_VOLUME_CATEGORY Category);
	float GetDistanceLimit();
	D3DXVECTOR3 GetListenerPos(){return D3DXVECTOR3( m_vListenerPosition.x,m_vListenerPosition.y,m_vListenerPosition.z); }

	//Stop
	void StopSound(int nHandle,bool bFadeEft = false);
	void StopFadeSound(int nHandle);
	void StopSoundAll();
	
	void  SaveVolCtgy();
	void  RestoreVolCtgy();
	int GetCueIndex(int nType,char *szCueName);
	void SetPauseAll(BOOL bPause);
	void Pause(int nIndex,BOOL bPause);

	virtual void LogCueErr( int nType,char *szCueName ){};
	bool IsOwner( HANDLE hHandle,int nIndex );
	

protected:
	void Clear();
	void Calculate3D( CueInstance* pInstance );
	void SetCategory();
	bool IsInDistanceLimit(const D3DXVECTOR3 &EmitterPos,float fTemp = 1.f);
	int GetEmptyCueIndex();
	void VolumeProcess();	
	void ProcessCues();
	int SortingByPriority();
	void SoundDebugString(CueInstance* pInstance,const char* format, ... );
	void SoundDebugString(const char* format, ... );
	void SetChannelValue(XACTCHANNELMAPENTRY &Channel,XACTCHANNEL ,XACTCHANNEL,XACTVOLUME);
	void SetChannelSetting( );
	void SetFadeInValue(int nIndex ,float fDeltaTick);
	void ChangeFadeMode( int nIndex, const float &fDestVol, const int &nTick );
	void GetChennelVol( XACTCHANNELVOLUME *pChannelVolume,CueInstance *pCue,XACTCHANNELVOLUMEENTRY *pEntry,int nEntryCount);
	
protected:

	// XACT stuff
	IXACTSoundBank						*m_pSoundBank[SB_TYPE_LAST];
	IXACTWaveBank						*m_pWaveBank[WB_LIST_LAST];	
    CueInstance							m_CueList[MAX_CUE_NUM];
	VolumeInfo							m_VolCtgy[XACT_VC_NUM];
	float								m_SaveVolCtgy[XACT_VC_NUM];
	
	DWORD								m_dwCalculateFlags;
	X3DAUDIO_DSP_SETTINGS				m_DSPSettings;
    X3DAUDIO_LISTENER					m_Listener;
    X3DAUDIO_EMITTER					m_Emitter;
    X3DAUDIO_CONE						m_Cone;

    // Sound source and listener positions
    XMVECTOR                m_vListenerPosition;    // Listener position vector //In general, Set camera vector
	XMVECTOR				m_vListenerCameraEye;
    XMVECTOR                m_vOldListenerPos;    // Listener position vector //In general, Set camera vector
    FLOAT                   m_fListenerAngle;       // Listener orientation angle in x-z

	int						m_nvfTick; //Volume Fade Tick
	float					m_fFadeDelta;
	char					m_CueDebugStr[128];
	XACT_VOLUME_CATEGORY	m_nIndexForFading;

	X3DAUDIO_DISTANCE_CURVE_POINT m_pCurvePoint[VOLUME_CURVE_RESOLUTION];
	X3DAUDIO_DISTANCE_CURVE		  m_pCurve;
	ATG::Timer					  m_Timer;
	float						  m_fFadeVolumeDest[CHANNELCOUNT];

	std::vector<int>		m_vecWaveBankDelete;	// 지울 웨이브뱅크들

	int						m_nTempCueIndex;
	int						m_nTempSBIndex;
	

private:
	static void XACTNotificationCallback( const XACT_NOTIFICATION* pNotification );
};

#endif //_XBOX