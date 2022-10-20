#include "stdafx.h"
#include "XActSound.h"
#include "FcCommon.h"
#include "FcGlobal.h"

#ifdef _XBOX
#include "FCSndFX.h"	// Generate with XACT
#include "FcUtil.h"
#include "BsUtil.h"
#include <x3daudio.h>
#include "PerfCheck.h"
#include "X3dAudioVolumeTable.h"
#include "BsFileIO.h"
#include "FcGameObject.h"

#include "X3DSoundCalculate.h"
#include ".\\data\\Sound\\FcSoundPlayDefinitions.h"

const FLOAT SPEEDOFSOUND		 = 340.29f;
static DWORD dwIDCounter		 = 0;


// Speaker channel masks
const DWORD SPEAKER_FRONT_LEFT      = 0x01;
const DWORD SPEAKER_FRONT_RIGHT     = 0x02;
const DWORD SPEAKER_FRONT_CENTER    = 0x04;
const DWORD SPEAKER_LOW_FREQUENCY   = 0x08;
const DWORD SPEAKER_BACK_LEFT       = 0x10;
const DWORD SPEAKER_BACK_RIGHT      = 0x20;



// Pan the voice according to X3DAudio calculation
FLOAT g_EmitterAzimuths[] = { 0.0f };


CXACTSound::CXACTSound()
{
	m_dwCalculateFlags = X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER |
		X3DAUDIO_CALCULATE_LPF_DIRECT | X3DAUDIO_CALCULATE_LPF_REVERB |
		X3DAUDIO_CALCULATE_REVERB;

	int i = 0;
	m_vListenerPosition = XMVectorSet(0.f, 0.f, 0.f, 0.f);
	m_vOldListenerPos   = XMVectorSet(0.f, 0.f, 0.f, 0.f);

	for( i = 0;i < MAX_CUE_NUM;i++){	
		m_CueList[i].bEnable = false;
	}


	for( i = 0;i < XACT_VC_NUM;i++){	
		m_VolCtgy[i].XActCtgy = XACTCATEGORY_INVALID;
		m_VolCtgy[i].fVolume  = 1.f;
		m_SaveVolCtgy[i] = 0.f;
	}



	m_nvfTick = 0;
	m_fFadeDelta = 0.f;
	m_nIndexForFading = XACT_VC_NONE;

	for(i = 0;i < (int)WB_LIST_LAST;i++) 
		m_pWaveBank[i] = NULL;

	for(i = 0;i < (int)SB_TYPE_LAST;i++)
		m_pSoundBank[i] = NULL;
	memset(&m_pCurve,0,sizeof(X3DAUDIO_DISTANCE_CURVE));

	memset(&m_DSPSettings,0,sizeof(X3DAUDIO_DSP_SETTINGS) );
	memset(&m_Listener,0,sizeof(X3DAUDIO_LISTENER) );
	memset(&m_Emitter,0,sizeof(X3DAUDIO_EMITTER) );
	memset(&m_Cone,0,sizeof(X3DAUDIO_CONE) );
	memset(&m_vListenerPosition,0,sizeof(XMVECTOR) );
	memset(&m_vListenerPosition,0,sizeof(XMVECTOR));
	memset(&m_vListenerCameraEye,0,sizeof(XMVECTOR));
	memset(&m_vOldListenerPos,0,sizeof(XMVECTOR));
	memset(&m_fListenerAngle,0,sizeof(FLOAT));
	memset(&m_CueDebugStr,0,128);	
	memset(&m_pCurvePoint[0],0,sizeof(XACT_VOLUME_CATEGORY) * VOLUME_CURVE_RESOLUTION);

	memset( &m_pCurve,0,sizeof(X3DAUDIO_DISTANCE_CURVE) );

	m_nTempCueIndex = -1;
	m_nTempSBIndex = -1;

	for(i = 0;i < (int)CHANNELCOUNT;i++)
		m_fFadeVolumeDest[i] = 1.f;

}


CXACTSound::~CXACTSound()
{
	Clear();    
}

void CXACTSound::FinishStage()
{
	int i = 0;
	for( i = 0; i < MAX_CUE_NUM;i++){
		StopSound( i );
	}
}

bool CXACTSound::Initialize( const char* pGSFileName )
{


	HRESULT hr = S_OK;

	DWORD dwFileSize  = 0;
	DWORD dwAllocFlags = 0;

	// Load the XACT global settings file
	VOID* pbGlobalSettings = NULL;
	if( FAILED( hr = CBsFileIO::LoadFilePhysicalMemory( pGSFileName, &pbGlobalSettings, &dwFileSize, &dwAllocFlags ) ) )
	{
		BsAssert( 0 && "Could not load XGS file" );
		return false;
	}
	// Initialize the XACT runtime parameters
	XACT_RUNTIME_PARAMETERS xrParams;
	XACT_FILEIO_CALLBACKS xactFIOC;

	xactFIOC.getOverlappedResultCallback = &CBsFileIO::BsGetOverlappedResult;
	xactFIOC.readFileCallback = &CBsFileIO::BsReadFile;

	xrParams.lookAheadTime = 250;
	xrParams.pGlobalSettingsBuffer = pbGlobalSettings;
	xrParams.globalSettingsBufferSize = dwFileSize;
	xrParams.globalSettingsFlags = XACT_FLAG_API_CREATE_MANAGEDATA;
	xrParams.globalSettingsAllocAttributes = dwAllocFlags;
	xrParams.fnNotificationCallback = &this->XACTNotificationCallback;
	xrParams.fileIOCallbacks = xactFIOC;

	// Create the XACT runtime engine
	hr = XACTInitialize( &xrParams );
	if( FAILED( hr ) )
	{
		BsAssert( 0 && "XACTInitialized failed with error %#X" );
		return false;
	}

	X3DAudioInitialize( SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|
		SPEAKER_LOW_FREQUENCY|
		SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT,
		SPEEDOFSOUND );


	// Initialize 3D audio parameters
	X3DAUDIO_VECTOR ZeroVector = { 0.0f, 0.0f, 0.0f };

	// Set up listener parameters
	m_Listener.OrientFront.x        = 0.0f;
	m_Listener.OrientFront.y        = 0.0f;
	m_Listener.OrientFront.z        = 1.0f;
	m_Listener.OrientTop.x          = 0.0f;
	m_Listener.OrientTop.y          = 1.0f;
	m_Listener.OrientTop.z          = 0.0f;
	m_Listener.Position.x           = 0.0f;
	m_Listener.Position.y           = 0.0f;
	m_Listener.Position.z           = 0.0f;
	m_Listener.Velocity             = ZeroVector;

	// Set up emitter parameters
	m_Emitter.OrientFront.x         = 0.0f;
	m_Emitter.OrientFront.y         = 0.0f;
	m_Emitter.OrientFront.z         = 1.0f;
	m_Emitter.OrientTop.x           = 0.0f;
	m_Emitter.OrientTop.y           = 1.0f;
	m_Emitter.OrientTop.z           = 0.0f;
	m_Emitter.Position              = ZeroVector;
	m_Emitter.Velocity              = ZeroVector;
	m_Emitter.pCone                 = &m_Cone;
	m_Emitter.pCone->InnerAngle     = 0.0f; // Setting the inner cone angles to X3DAUDIO_2PI and
	// outer cone other than 0 causes
	// the emitter to act like a point emitter using the
	// INNER cone settings only.
	m_Emitter.pCone->OuterAngle     = 0.0f; // Setting the outer cone angles to zero causes
	// the emitter to act like a point emitter using the
	// OUTER cone settings only.
	m_Emitter.pCone->InnerVolume    = 0.0f;
	m_Emitter.pCone->OuterVolume    = 1.0f;
	m_Emitter.pCone->InnerLPF       = 0.0f;
	m_Emitter.pCone->OuterLPF       = 1.0f;
	m_Emitter.pCone->InnerReverb    = 0.0f;
	m_Emitter.pCone->OuterReverb    = 1.0f;

	m_Emitter.ChannelCount          = 1;
	m_Emitter.ChannelRadius         = 0.0f;




	const int nResolution = sizeof(nX3DAudioVolumeTable) / sizeof(nX3DAudioVolumeTable[0]);
	for(int i = 0; i < nResolution; i ++)
	{
		m_pCurvePoint[i].Distance   = ((float) i / (nResolution-1));
		m_pCurvePoint[i].DSPSetting = (float)nX3DAudioVolumeTable[i] / 256.f;
	}
	m_pCurve.pPoints    = m_pCurvePoint;
	m_pCurve.PointCount = nResolution;
	m_Emitter.pVolumeCurve = &m_pCurve;

	m_Emitter.pLFECurve             = NULL;
	m_Emitter.pLPFDirectCurve       = NULL;
	m_Emitter.pLPFReverbCurve       = NULL;
	m_Emitter.pReverbCurve          = NULL;
	m_Emitter.CurveDistanceScaler   = X3DAUDIO_DISTANCE_EMITER;
	m_Emitter.DopplerScaler         = 1.f;
	m_Emitter.pChannelAzimuths      = g_EmitterAzimuths;

	m_DSPSettings.SrcChannelCount     = 1;
	m_DSPSettings.DstChannelCount     = CHANNELCOUNT;
	m_DSPSettings.pMatrixCoefficients = new FLOAT[ CHANNELCOUNT ];
	if( m_DSPSettings.pMatrixCoefficients == NULL )
		return false;

	SetCategory();
	return true;
}




void CXACTSound::SetCategory()
{
	char *pCategory[XACT_VC_NUM] = 
	{
		"Music",
			"MOVE",
			"ATTACK",
			"CROWD_AMBIENCE",
			"FOOT",
			"HIT",
			"VOICE",
			"AMBIENCE",
			"OBJECT",
			"SYSTEM",
			"BGM",
			"REALMOVIE",
			"ENEMY",
			"EVENT"

	};

	for(int i = 0;i < XACT_VC_NUM;i++)
	{
		m_VolCtgy[i].XActCtgy = XACTGetCategory( pCategory[i] );
		BsAssert( m_VolCtgy[i].XActCtgy != XACTCATEGORY_INVALID && "Categoty is not found" );		
		m_VolCtgy[i].fVolume = 1.f;
		XACTSetVolume( m_VolCtgy[i].XActCtgy, m_VolCtgy[i].fVolume );
	}
}


int CXACTSound::GetCueIndex(int nType,char *szCueName)
{
	if(nType == -1){	
		//SoundDebugString("SoundError 타입이 -1");
		return 0;
	}
	else if(nType >= SB_TYPE_LAST)
	{
		//SoundDebugString("SoundError 타입이 너무큼");
		return 0;
	}else if (nType < -1) { //aleksger: prefix bug 887: Possible buffer underflow.
		BsAssert("SoundError - nType < -1");
		return 0;
	}

	int nCueIndex = m_pSoundBank[nType]->GetCueIndex( szCueName );
	if( nCueIndex == XACTINDEX_INVALID ){	
		LogCueErr(nType,szCueName);
		nCueIndex = -1;
	}
	return nCueIndex;
}

void CXACTSound::SetVolumeCtgy( XACT_VOLUME_CATEGORY Category,float fVolume)
{
	m_VolCtgy[Category].fVolume = fVolume;
	XACTSetVolume( m_VolCtgy[Category].XActCtgy, fVolume );
}

float CXACTSound::GetVolumeCtgy(XACT_VOLUME_CATEGORY Category)
{
	return m_VolCtgy[Category].fVolume;
}

void CXACTSound::DeleteWaveBank(int nWaveBankIdx)
{
	if(m_pWaveBank[nWaveBankIdx])
	{
		m_pWaveBank[nWaveBankIdx]->Destroy();
		m_pWaveBank[nWaveBankIdx] = NULL;
	}
}


void CXACTSound::RegisterIMWavebank(int nWaveBankIdx, VOID* pbWaveBank, const DWORD dwFileSize )
{
	IXACTWaveBank* pIXACTWaveBank;

	if( FAILED( XACTCreateInMemoryWaveBank( pbWaveBank, dwFileSize, 0, 0, &pIXACTWaveBank ) ) )
	{		
		BsAssert( 0 && "XACTCreateInMemoryWaveBank failed with error" );
	}

	m_pWaveBank[nWaveBankIdx] = pIXACTWaveBank;
}


void CXACTSound::LoadWaveBank(int nWaveBankIdx, const char* pFileName, bool useStream )
{
	HRESULT hr = S_OK;
	DWORD dwFileSize  = 0;

	IXACTWaveBank* pIXACTWaveBank;
	if( useStream )
	{
		HANDLE hFile;
		// Open the in streaming wave bank
		hFile = CBsFileIO::BsCreateFile( pFileName, GENERIC_READ, 0, 0, OPEN_EXISTING,
			FILE_FLAG_OVERLAPPED |  FILE_FLAG_NO_BUFFERING, 0 );

		if( hFile == INVALID_HANDLE_VALUE ) { BsAssert2( 0 && "Loading file failed with error %#X\n", hr ); }

		XACT_WAVEBANK_STREAMING_PARAMETERS wbParams = { 0 };
		wbParams.file = hFile;
		wbParams.packetSize = 16;

		// Register the streaming wave bank with XACT
		if( FAILED( hr = XACTCreateStreamingWaveBank( &wbParams, &pIXACTWaveBank ) ) )
			BsAssert2( 0 && "XACTCreateStreamingWaveBank failed with error %#X\n", hr );

		// Wait until wave bank is prepared
		DWORD dwState;
		do{
			XACTDoWork();
			pIXACTWaveBank->GetState( &dwState );
		} while( !( dwState & XACT_WAVEBANKSTATE_PREPARED ) );

	}
	else{
		VOID* pbWaveBank = NULL;
		DWORD dwAllocFlags = 0;


		if( FAILED( hr = CBsFileIO::LoadFilePhysicalMemory( pFileName,
			&pbWaveBank,
			&dwFileSize,
			&dwAllocFlags,
			XALLOC_PHYSICAL_ALIGNMENT_DEFAULT,
			XALLOC_MEMPROTECT_READWRITE, true) ) ) {		
				BsAssert( 0 && "Could not load XWB file" );
			}
			// Register the wave bank with XACT

			if( FAILED( hr = XACTCreateInMemoryWaveBank( pbWaveBank, dwFileSize, XACT_FLAG_API_CREATE_MANAGEDATA, dwAllocFlags, &pIXACTWaveBank ) ) ) {					
				CBsFileIO::FreePhysicalMemory( pbWaveBank, dwAllocFlags );
				BsAssert( 0 && "XACTCreateInMemoryWaveBank failed with error" );
			}		
	}
	m_pWaveBank[nWaveBankIdx] = pIXACTWaveBank;

}


void CXACTSound::LoadSoundBank(int nSoundBankIdx, const char* pFileName )
{
	HRESULT hr = S_OK;
	DWORD dwFileSize  = 0;
	DWORD dwAllocFlags = 0;

	VOID* pbSoundBank = NULL;
	if( FAILED( hr = CBsFileIO::LoadFilePhysicalMemory( pFileName,	&pbSoundBank, &dwFileSize, &dwAllocFlags ) ) )
	{
		BsAssert( 0 && "Could not load XSB file" );
	}
	// Register the sound bank with XACT
	IXACTSoundBank* pIXACTSoundBank;
	if( FAILED( hr = XACTCreateSoundBank( pbSoundBank, dwFileSize, XACT_FLAG_API_CREATE_MANAGEDATA, dwAllocFlags, &pIXACTSoundBank ) ) )
	{
		BsAssert( 0 && "XACTCreateSoundBank failed with error" );
	}
	m_pSoundBank[nSoundBankIdx] =  pIXACTSoundBank;
}


void CXACTSound::ReserveDeleteWaveBank( int nWaveBankIdx )
{
	// 사용하지 않는 것들을 모두 지우게되는데 그렇게 되면 안될듯..
	int nCnt = m_vecWaveBankDelete.size();
	for( int i=0 ; i<nCnt; i++ )
	{
		if( nWaveBankIdx == m_vecWaveBankDelete[i] )
			return;
	}
	if(m_pWaveBank[nWaveBankIdx])
	{
		m_vecWaveBankDelete.push_back( nWaveBankIdx );
	}

}

void CXACTSound::GetChennelVol( XACTCHANNELVOLUME *pChannelVolume,CueInstance *pCue,XACTCHANNELVOLUMEENTRY *pEntry,int nEntryCount)
{
	memset(pEntry,0, sizeof(XACTCHANNELVOLUMEENTRY) * nEntryCount );
	for( int  s = 0;s < nEntryCount;s ++){			
		pEntry[s].EntryIndex = s;
	}
	pChannelVolume->EntryCount = nEntryCount;
	pChannelVolume->paEntries = pEntry;

	if(pCue){
		pCue->pCue->GetChannelVolume( pChannelVolume );
	}
}

void CXACTSound::ChangeFadeMode( int nIndex, const float &fDestVol, const int &nTick )
{
	if( nIndex == -1 || nIndex >= MAX_CUE_NUM ){	
		BsAssert( 0 && "BGM Fade index wrong");
		return;
	}
	if( m_CueList[nIndex].bEnable && m_CueList[nIndex].pCue )
	{
		XACTCHANNELVOLUMEENTRY ChannelVolumeEntries[CHANNELCOUNT];
		XACTCHANNELVOLUME ChannelVolume;
		GetChennelVol( &ChannelVolume,&m_CueList[nIndex],ChannelVolumeEntries,CHANNELCOUNT);

		float fCurVol = ChannelVolume.paEntries[0].Volume;

		m_CueList[nIndex].FadeState = FSPT_FADE_ONLY;
		m_CueList[nIndex].fFadeDelta = ((fDestVol - fCurVol) / (float)nTick );
		m_CueList[nIndex].nFadeCount = 	nTick;
	}
}

void CXACTSound::VolumeProcess()
{
	/*if( m_nvfTick )
	{		
	m_VolCtgy[m_nIndexForFading].fVolume += m_fFadeDelta;
	XACTSetVolume( m_VolCtgy[m_nIndexForFading].XActCtgy ,  m_VolCtgy[m_nIndexForFading].fVolume );
	m_nvfTick--;
	}*/
}


void CXACTSound::ProcessCues()
{
	int i;
	DWORD dwState;
	int nPlayCount;
	for(i = 0;i < MAX_CUE_NUM;i++)
	{
		if(m_CueList[i].bEnable == false){ continue; }

		if( m_CueList[i].b3D ) 
		{ 
			if( m_CueList[i].StereoPlayType == FSPT_MONO && m_CueList[i].FadeState == FSPT_FADE_NONE ){
				Calculate3D(&m_CueList[i]); 
			}
		}

		m_CueList[i].pCue->GetState(&dwState);
		if( dwState & XACT_CUESTATE_STOPPED )
		{
			if(m_CueList[i].nPlayCount != PLAY_COUNT_LOOP)
				--m_CueList[i].nPlayCount;

			nPlayCount = m_CueList[i].nPlayCount;
			if( (nPlayCount == PLAY_COUNT_LOOP) || (nPlayCount > 0) ) {

				CueInstance *pCueIns = &m_CueList[i];
				D3DXVECTOR3 *pVec = NULL;
				if(pCueIns->b3D){				
					D3DXVECTOR3 Pos( pCueIns->Pos.x,pCueIns->Pos.y,pCueIns->Pos.z);
					pVec = &Pos;
				}

				//Confuse confuse confuse
				int *pBackupHandle = pCueIns->pHandle;
				int nCueIndex = CreateSound( pCueIns->Handle, pCueIns->nSoundBank,pCueIns->nCueIndex,
					pVec, pCueIns->nCtgy, nPlayCount, pCueIns->pHandle,pCueIns->bSetToLisnPos,pCueIns->StereoPlayType );
				StopSound( i );
				if( pBackupHandle ){ *pBackupHandle = nCueIndex; }

			}
			else if(nPlayCount == 0 || nPlayCount < 0) {
				StopSound( i );
			}
		}
	}
}

void CXACTSound::Process()
{
	m_Timer.MarkFrame();
	//VolumeProcess();
	ProcessFadeInOut();
	ProcessCues();
	XACTDoWork();
}



void CXACTSound::SetEmitterPos(int nIndex,D3DXVECTOR3 *pPos)
{
	if(nIndex == -1) return;
#ifdef _DEBUG_POSITION_ERR
	if(!(GetProcessTick() % 120)){	
		DebugString("*****Update Emitter Pos : %f %f %f*******\n",pPos->x,pPos->y,pPos->z );
	}
#endif

	if(m_CueList[nIndex].bEnable){
		m_CueList[nIndex].OldPos = m_CueList[nIndex].Pos;
		m_CueList[nIndex].Pos = XMVectorSet(pPos->x,pPos->y,pPos->z,0.f);
	}
}


// 3D사웝?EEmiitter와 Listener와의 거리 
bool CXACTSound::IsInDistanceLimit(const D3DXVECTOR3 &EmitterPos,float fTemp)
{
	D3DXVECTOR3 ListenerPos(m_vListenerPosition.x,m_vListenerPosition.y,m_vListenerPosition.z);
	if(D3DXVec3Length( &(ListenerPos - EmitterPos)) < (X3DAUDIO_DISTANCE_TRIGGER*fTemp) )
		return true;
	return false;
}

void CXACTSound::SetListenerPos( D3DXVECTOR3 &Pos,D3DXVECTOR3 &Eye )
{
#ifdef _DEBUG_POSITION_ERR
	if(!(GetProcessTick() % 120)){	
		DebugString("Update Listerpos : %f %f %f\n",Pos.x,Pos.y,Pos.z );
	}
#endif
	m_vListenerCameraEye = XMVectorSet( Eye.x, Eye.y, Eye.z, 0.f );
	m_vListenerPosition  = XMVectorSet( Pos.x, Pos.y, Pos.z, 0.f );

}

void CXACTSound::SetListenerPos(D3DXVECTOR3 &Pos, bool bSavePos)
{
#ifdef _DEBUG_POSITION_ERR
	DebugString("Wrong update for listener!!\n");
#endif

	if( bSavePos )
	{
		m_vOldListenerPos = m_vListenerPosition;
		m_vListenerPosition = XMVectorSet( Pos.x, Pos.y, Pos.z, 0.f );
	}
	else
	{
		m_vListenerPosition = XMVectorSet( Pos.x, Pos.y, Pos.z, 0.f );
	}
}



int CXACTSound::CreateSound( HANDLE Handle, int nType, char *szCueName, D3DXVECTOR3 *pPos,int nCtgy,int nPlayCount,int *pHandle,bool bSetLisnPos,FC_STEREO_PLAY_TYPE Stereo/* = FSO_NONE*/ )
{
#ifdef _DEBUG_POSITION_ERR
	if(pPos){
		DebugString("%s Cueindex Pos : %f %f %f\n",szCueName,pPos->x,pPos->y,pPos->z);
	}
#endif

#ifdef 	_SOUND_CUE_NAME_OUTPUT
	DebugString("CueName %s\n\n",szCueName );
#endif

	int nCueIndex = GetCueIndex(nType,szCueName);
	return CreateSound( Handle, nType, nCueIndex, pPos,nCtgy, nPlayCount, pHandle ,bSetLisnPos,Stereo );

}

DWORD g_TestIndentify;
int CXACTSound::CreateSound(  HANDLE Handle, int nType, int nCueIndex, D3DXVECTOR3 *pPos,int nCtgy,int nPlayCount,int *pHandle,bool bSetLisnPos, FC_STEREO_PLAY_TYPE Stereo/* = FSO_NONE*/)
{
	if(pPos)
	{
		if(IsInDistanceLimit(*pPos) == false)
			return -1;
	}
	// Create a new instance to track this cue
	int EptIdx = GetEmptyCueIndex();
	if(EptIdx == -1){
		EptIdx = SortingByPriority();
	}


	HRESULT hr = S_OK;
	bool b3D = (pPos != NULL);
	IXACTCue* pCue;                         // Cue instance
	if( FAILED( hr = m_pSoundBank[nType]->Prepare( nCueIndex, 0, &pCue ) ) )
	{
		SoundDebugString( "3D SoundPlay Fail, SoundBankID : %d CueID : %d( %s )\n",nType,nCueIndex,m_CueDebugStr);
		memset( m_CueDebugStr,0,128 );
		return -1;
	}

	m_CueList[EptIdx].pCue          = pCue;
	m_CueList[EptIdx].bEnable		= true;	
	m_CueList[EptIdx].Handle		= Handle;
	m_CueList[EptIdx].pHandle		= pHandle;
	m_CueList[EptIdx].nPlayCount	= nPlayCount;
	m_CueList[EptIdx].b3D			= b3D;
	m_CueList[EptIdx].nCtgy			= nCtgy;
	m_CueList[EptIdx].bSetToLisnPos = bSetLisnPos;
	//m_CueList[EptIdx].nTestIdentify = g_TestIndentify++;

	if( b3D ){
		m_CueList[EptIdx].Pos    = XMVectorSet( pPos->x, pPos->y, pPos->z, 0.0f );
		m_CueList[EptIdx].OldPos = XMVectorSet( pPos->x, pPos->y, pPos->z, 0.0f );
	}
	else{
		m_CueList[EptIdx].Pos    = XMVectorSet( 0,0,0, 0.0f );
		m_CueList[EptIdx].OldPos = XMVectorSet( 0,0,0, 0.0f );
	}

	m_CueList[EptIdx].bFadeStop	   = false;
	m_CueList[EptIdx].nPlayCount   = nPlayCount;
	//m_CueList[EptIdx].bOutPutString  = true;

	m_CueList[EptIdx].nSoundBank = nType;
	m_CueList[EptIdx].nCueIndex  = nCueIndex;
	m_CueList[EptIdx].StereoPlayType = Stereo;

	XACTCHANNELMAPENTRY ChannelMapEntries[CHANNELCOUNT];

	m_fFadeVolumeDest[0] = 1.f;
	m_fFadeVolumeDest[1] = 1.f;
	m_fFadeVolumeDest[2] = 1.f;
	m_fFadeVolumeDest[3] = 1.f;
	m_fFadeVolumeDest[4] = 1.f;
	m_fFadeVolumeDest[5] = 1.f;

	if( Stereo == FSPT_STEREO_NORMAL )
	{
		SetChannelValue( ChannelMapEntries[0], 0, 0, 1.f );
		SetChannelValue( ChannelMapEntries[1], 1, 1, 1.f );
		SetChannelValue( ChannelMapEntries[2], 0, 3, 0.5f );
		SetChannelValue( ChannelMapEntries[3], 1, 3, 0.5f );
		SetChannelValue( ChannelMapEntries[4], 0, 4, 1.f );
		SetChannelValue( ChannelMapEntries[5], 1, 5, 1.f );

	}
	else if( Stereo == FSPT_STEREO_FADE_EFT )
	{
		SetChannelValue( ChannelMapEntries[0], 0, 0, 1.f );
		SetChannelValue( ChannelMapEntries[1], 1, 1, 1.f );
		SetChannelValue( ChannelMapEntries[2], 0, 3, 0.5f);
		SetChannelValue( ChannelMapEntries[3], 1, 3, 0.5f);
		SetChannelValue( ChannelMapEntries[4], 0, 4, 1.f );
		SetChannelValue( ChannelMapEntries[5], 1, 5, 1.f );

		m_fFadeVolumeDest[0] = 1.f;
		m_fFadeVolumeDest[1] = 1.f;
		m_fFadeVolumeDest[2] = 0.5f;
		m_fFadeVolumeDest[3] = 0.5f;
		m_fFadeVolumeDest[4] = 1.f;
		m_fFadeVolumeDest[5] = 1.f;
		//m_CueList[EptIdx].FadeState = FSPT_FADE_IN;
	}
	else
	{
		if(b3D)
		{
			for(int i = 0;i < CHANNELCOUNT;i++)	{
				SetChannelValue( ChannelMapEntries[i], 0, i, 0 );
			}

		}
	}

	if((Stereo != FSPT_MONO) || b3D)
	{
		// Set channel map to all silence - this will get updated from the X3DAudio calculations
		DWORD dwNumEntries = sizeof(ChannelMapEntries) / sizeof(ChannelMapEntries[ 0 ]);
		XACTCHANNELMAP ChannelMap = { (XACTCHANNEL)dwNumEntries, ChannelMapEntries };
		pCue->SetChannelMap( &ChannelMap );
	}

	if( FAILED( hr = pCue->Play() ) )
	{
		pCue->Destroy();
		m_CueList[EptIdx].Init();
		return -1;		
	}
	return EptIdx;

}

void CXACTSound::SetChannelValue(XACTCHANNELMAPENTRY &Channel,XACTCHANNEL InputChannel,XACTCHANNEL OutputChannel,XACTVOLUME Volume)
{
	Channel.InputChannel = InputChannel;
	Channel.OutputChannel = OutputChannel;
	Channel.Volume = Volume;
}

int CXACTSound::GetEmptyCueIndex()
{
	for(int i = 0; i < MAX_CUE_NUM;i++)
	{
		if(m_CueList[i].bEnable == false) {
			return i;
		}
	}
	return -1;
}



void CXACTSound::Clear()
{
	for( int i = 0; i < MAX_CUE_NUM;i++){    
		if(m_CueList[i].bEnable)
		{
			m_CueList[i].pCue->Destroy();
			m_CueList[i].Init();

		}
	}

	for( int i=0; i < WB_LIST_LAST; i++ ){	
		if(m_pWaveBank[i]){ m_pWaveBank[i]->Destroy(); }
		SAFE_DELETE(m_pWaveBank[i]);
	}

	for( int i=0; i < SB_TYPE_LAST; i++ ){	
		if(m_pSoundBank[i])
		{			
			if(m_pSoundBank[i]){ m_pSoundBank[i]->Destroy(); }
			SAFE_DELETE(m_pSoundBank[i]);
		}		
	}

}
void CXACTSound::SaveVolCtgy()
{
	for(int i = 0;i < XACT_VC_NUM;i++){	
		m_SaveVolCtgy[i] = m_VolCtgy[i].fVolume;
	}
}

void CXACTSound::RestoreVolCtgy()
{
	for(int i = 0;i < XACT_VC_NUM;i++){	
		XACTSetVolume( m_VolCtgy[i].XActCtgy, m_SaveVolCtgy[i] );		
		m_VolCtgy[i].fVolume = m_SaveVolCtgy[i];
	}
}

void CXACTSound::ProcessFadeInOut()
{
	for(int i = 0;i < MAX_CUE_NUM;i++)
	{
		if(m_CueList[i].bEnable == false){ continue; }

		CueInstance *pCueIns = &m_CueList[i];
		float fSignal = 1.f;

		if( pCueIns->FadeState != FSPT_FADE_NONE && pCueIns->FadeState != FSPT_FADE_ONLY )
		{
			XACTCHANNELVOLUMEENTRY ChannelVolumeEntries[CHANNELCOUNT];
			XACTCHANNELVOLUME ChannelVolume;
			GetChennelVol( &ChannelVolume, pCueIns ,ChannelVolumeEntries,CHANNELCOUNT);

			bool bFadeOutFinish = true;
			bool bFadeInFinish = true;
			for(int s = 0;s < CHANNELCOUNT;s++){					
				if(pCueIns->FadeState == FSPT_FADE_OUT_STOP){ fSignal = -1.f; }
				ChannelVolume.paEntries[s].Volume += ((m_fFadeVolumeDest[s] / pCueIns->fFadeDelta) * fSignal);

				if(ChannelVolume.paEntries[s].Volume >= m_fFadeVolumeDest[s]) {				
					ChannelVolume.paEntries[s].Volume = m_fFadeVolumeDest[s];
				}
				else{
					bFadeInFinish = false;
				}

				if( ChannelVolume.paEntries[s].Volume <= 0.f ){
					ChannelVolume.paEntries[s].Volume = 0.f;
				}
				else{
					bFadeOutFinish = false;
				}
			}

			if( bFadeOutFinish == true && pCueIns->FadeState == FSPT_FADE_OUT_STOP )
			{
				pCueIns->FadeState = FSPT_FADE_OUT_FINISH;
				pCueIns->pCue->Stop( XACT_FLAG_CUE_STOP_IMMEDIATE );
			}
			else if(bFadeOutFinish == true && pCueIns->FadeState == FSPT_FADE_IN ) {			
				pCueIns->FadeState = FSPT_FADE_IN_FINISH;
			}
			pCueIns->pCue->SetChannelVolume( &ChannelVolume );
		}
		else if(pCueIns->FadeState == FSPT_FADE_ONLY)
		{
			if(pCueIns->nFadeCount)
			{
				XACTCHANNELVOLUMEENTRY ChannelVolumeEntries[CHANNELCOUNT];
				XACTCHANNELVOLUME ChannelVolume;
				GetChennelVol( &ChannelVolume,pCueIns,ChannelVolumeEntries,CHANNELCOUNT);
				bool bFinish = false;

				for(int s = 0;s < CHANNELCOUNT;s++)
				{
					ChannelVolume.paEntries[s].Volume += (pCueIns->fFadeDelta * m_fFadeVolumeDest[s]);
					if(ChannelVolume.paEntries[s].Volume >= m_fFadeVolumeDest[s]) {					
						ChannelVolume.paEntries[s].Volume = m_fFadeVolumeDest[s];
					}
					if( ChannelVolume.paEntries[s].Volume <= 0.f ) {
						ChannelVolume.paEntries[s].Volume = 0.f;
					}
				}
				pCueIns->pCue->SetChannelVolume( &ChannelVolume );
				pCueIns->nFadeCount--;
			}
			else
			{
				pCueIns->FadeState = FSPT_FADE_NONE;
			}

		}	

	}
}


void CXACTSound::Calculate3D( CueInstance* pInstance )
{
	if(pInstance->bFadeStop)
	{		
		bool bFinish = true;
		XACTCHANNELVOLUMEENTRY ChannelVolumeEntries[6];
		memset(ChannelVolumeEntries,0, sizeof(XACTCHANNELVOLUMEENTRY) * CHANNELCOUNT );
		for( int i = 0;i < CHANNELCOUNT;i++) {		
			ChannelVolumeEntries[i].EntryIndex = i;
		}
		XACTCHANNELVOLUME ChannelVolume = { CHANNELCOUNT, ChannelVolumeEntries };
		pInstance->pCue->GetChannelVolume( &ChannelVolume );
		for(int i = 0;i < CHANNELCOUNT;i++)
		{
			ChannelVolumeEntries[i].EntryIndex = (XACTCHANNEL)i;
			ChannelVolume.paEntries[i].Volume -= 0.1f;
			if(ChannelVolume.paEntries[i].Volume <= 0.f) {
				ChannelVolume.paEntries[i].Volume = 0.f;
			}
			else{
				bFinish = false;
			}
		}
		pInstance->pCue->SetChannelVolume( &ChannelVolume );
		if(bFinish){
			//스탑만 시켜도 알아서 파괴함
			pInstance->pCue->Stop( XACT_FLAG_CUE_STOP_IMMEDIATE );
		}
	}
	else{

		FLOAT fElapsedTime = (FLOAT)m_Timer.GetElapsedTime();
		DWORD     dwPulse = DWORD( (cosf( (FLOAT)m_Timer.GetAppTime() * 3.0f ) + 1.f ) * 80.f );
		D3DCOLOR  cBlend  = dwPulse | ( dwPulse << 8 ) | ( dwPulse << 16 );

		m_Listener.OrientFront.x = m_vListenerCameraEye.x;
		m_Listener.OrientFront.y = m_vListenerCameraEye.y;
		m_Listener.OrientFront.z = m_vListenerCameraEye.z;


		if( fElapsedTime > 0.0f )
		{
			// Position the sound and m_Listener in 3D.
			XMVECTOR vListenerVelocity = ( m_vListenerPosition - m_vOldListenerPos ) / fElapsedTime;
			XMVECTOR vSoundVelocity    = ( pInstance->Pos - pInstance->OldPos ) / fElapsedTime;

			// Set up m_Listener parameters
			m_Listener.Position.x = m_vListenerPosition.x;
			m_Listener.Position.y = m_vListenerPosition.y;
			m_Listener.Position.z = m_vListenerPosition.z;
			m_Listener.Velocity.x = vListenerVelocity.x;
			m_Listener.Velocity.y = vListenerVelocity.y;
			m_Listener.Velocity.z = vListenerVelocity.z;

			// Set up m_Emitter parameters
			if(pInstance->bSetToLisnPos == false)
			{
				m_Emitter.Position.x  = pInstance->Pos.x;
				m_Emitter.Position.y  = pInstance->Pos.y;
				m_Emitter.Position.z  = pInstance->Pos.z;

				//				if(pInstance->pObject && CFcBaseObject::IsValid( pInstance->pObject ))
				if(pInstance->pObject)
				{
					m_Emitter.Position.x  = pInstance->pObject->GetPos().x;
					m_Emitter.Position.y  = pInstance->pObject->GetPos().y;
					m_Emitter.Position.z  = pInstance->pObject->GetPos().z;
				}

				m_Emitter.Velocity.x  = vSoundVelocity.x;
				m_Emitter.Velocity.y  = vSoundVelocity.y;
				m_Emitter.Velocity.z  = vSoundVelocity.z;
			}
			else{

				m_Listener.OrientFront.x = 0;
				m_Listener.OrientFront.y = 0;
				m_Listener.OrientFront.z = 1;

				m_Emitter.Position.x = 0;
				m_Emitter.Position.y = 0;
				m_Emitter.Position.z = 0;

				m_Emitter.Velocity.x  = 0;
				m_Emitter.Velocity.y  = 0;
				m_Emitter.Velocity.z  = 0;

				m_Listener.Position.x = 0;
				m_Listener.Position.y = 0;
				m_Listener.Position.z = 10;
			}
		}
		else{
			//			SoundDebugString( "3D SOUND CALCULATION FAIL \n" );
		}
#if 1
		XACTCHANNELVOLUMEENTRY ChannelVolumeEntries[6];
		XACTCHANNELVOLUME ChannelVolume = { 6, ChannelVolumeEntries };

		SoundDebugString(pInstance,"******************************************************************\n");
		//SoundDebugString(pInstance,"Pool Index %d SoundBank:%d,SoundCue:%d\n",pInstance->nTestIdentify,pInstance->nSoundBank,pInstance->nCueIndex);

		float fVolume[6];
		D3DXVECTOR3 Emit( m_Emitter.Position.x,m_Emitter.Position.y,m_Emitter.Position.z );
		D3DXVECTOR3 Listn( m_Listener.Position.x,m_Listener.Position.y,m_Listener.Position.z );

		float fLengthTemp = D3DXVec3Length(&(Emit - Listn));
		SoundDebugString(pInstance,"Emitter Pos : (%f, %f, %f), Listener Pos(%f, %f, %f) -> Length : %f\n",
			m_Emitter.Position.x,m_Emitter.Position.y,m_Emitter.Position.z,
			m_Listener.Position.x,m_Listener.Position.y,m_Listener.Position.z,fLengthTemp);

		SoundDebugString(pInstance,"Listener Direction(%f, %f, %f)\n",
			m_Listener.OrientFront.x,m_Listener.OrientFront.y,m_Listener.OrientFront.z );

		X3DCalculate( m_Emitter.Position, m_Listener.Position, m_Listener.OrientFront, fVolume, X3DAUDIO_DISTANCE_EMITER);

		SoundDebugString(pInstance,"Volume Out:");
		for( DWORD i = 0; i < ChannelVolume.EntryCount; i++ )
		{
			ChannelVolumeEntries[i].EntryIndex = (XACTCHANNEL)i;
			ChannelVolumeEntries[i].Volume     = fVolume[i];
			SoundDebugString( pInstance,"%f ",fVolume[i] );
		}
		SoundDebugString(pInstance,"\n");

		if( pInstance->pCue ){		
			pInstance->pCue->SetChannelVolume( &ChannelVolume );
		}

#else
		// Set source position, velocity and volume

		// Retrieve 3D Audio parameters
		DWORD dwCalculateFlags = X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER |
			X3DAUDIO_CALCULATE_LPF_DIRECT | X3DAUDIO_CALCULATE_LPF_REVERB |
			X3DAUDIO_CALCULATE_REVERB;
		// There are other flags that control additional X3DAudio
		// functionality that will be illustrated as they become available

		//X3DAudioCalculate( &m_Listener, &m_Emitter, dwCalculateFlags, &m_DSPSettings );

		// Pan the voice according to X3DAudio calculation
		XACTCHANNELVOLUMEENTRY ChannelVolumeEntries[6];
		XACTCHANNELVOLUME ChannelVolume = { 6, ChannelVolumeEntries };

		float fVolResult;
		for( DWORD i = 0; i < ChannelVolume.EntryCount; i++ )
		{
			ChannelVolumeEntries[i].EntryIndex = (XACTCHANNEL)i;
			fVolResult = m_DSPSettings.pMatrixCoefficients[ i ];
			fVolResult = fVolResult * 3.f;
			if(fVolResult > 1.f){ fVolResult = 1.f; }
			ChannelVolumeEntries[i].Volume     = fVolResult;
		}
		if( pInstance->pCue ){
			pInstance->pCue->SetChannelVolume( &ChannelVolume );
		}
#endif
	}
}

void CXACTSound::StopFadeSound(int nHandle)
{
	if(nHandle == -1){ return; }
	if(m_CueList[nHandle].bEnable)
		m_CueList[nHandle].bFadeStop = true;
}

void CXACTSound::StopSound(int nHandle,bool bFadeEft)
{
	if(nHandle == -1)
		return;

	m_nTempCueIndex = m_CueList[nHandle].nCueIndex;
	m_nTempSBIndex  = m_CueList[nHandle].nSoundBank;
	if( m_CueList[nHandle].bEnable )
	{
		if(bFadeEft )
		{	
			if( m_CueList[nHandle].FadeState != FSPT_FADE_OUT_STOP &&
				m_CueList[nHandle].FadeState != FSPT_FADE_OUT_FINISH )
				m_CueList[nHandle].FadeState = FSPT_FADE_OUT_STOP;
		}
		else
		{
			m_CueList[nHandle].pCue->Stop( XACT_FLAG_CUE_STOP_IMMEDIATE );
			m_CueList[nHandle].pCue->Destroy();
			if( m_CueList[nHandle].pHandle ) { *m_CueList[nHandle].pHandle = -1; }
			m_CueList[nHandle].Init();
		}
	}
}

float CXACTSound::GetDistanceLimit()
{
	return X3DAUDIO_DISTANCE_TRIGGER;
}

void CXACTSound::XACTNotificationCallback( const XACT_NOTIFICATION* pNotification )
{
	if( ( NULL != pNotification ) && ( NULL != pNotification->pvContext ) )
	{
		XAudioSetEvent( (HANDLE) pNotification->pvContext );
	}
}

void CXACTSound::Pause( int nIndex,BOOL bPause )
{
	if(nIndex < 0) { return; }		
	if( nIndex > MAX_CUE_NUM ){ return; }
	DWORD dwState;
	if( m_CueList[nIndex].bEnable )
	{
		m_CueList[nIndex].pCue->GetState( &dwState );
		if(bPause)
		{
			if( dwState & XACT_CUESTATE_PLAYING ){
				m_CueList[nIndex].pCue->Pause( bPause );
			}
		}
		else{
			if( dwState & XACT_CUESTATE_PAUSED ){
				m_CueList[nIndex].pCue->Pause( bPause );
			}
		}
	}

}

void CXACTSound::StopSoundAll()
{
	for(int i = 0;i < MAX_CUE_NUM;i++)
	{
		StopSound(i,false);
	}
}

void CXACTSound::SetPauseAll( BOOL bPause )
{
	for(int i = 0;i < MAX_CUE_NUM;i++)
	{
		Pause( i, bPause );
	}
}

BYTE g_CtgyPriority[SC_CTGY_LAST] = 
{
	SC_WALK,
		SC_RUN,
		SC_JUMP,
		SC_DASH,
		SC_DOWN,
		SC_DOWN_ROLL,
		SC_DEFENSE,
		SC_HIT,
		SC_ATTACK_BASIC,
		SC_ATTACK_FINISH,
		SC_ATTACK_REPEAT,
		SC_COMMON,
		SC_HORSE_WALK,
		SC_HORSE_RUN,
		SC_HORSE_JUMP,
		SC_HORSE_STOP
};

int CXACTSound::SortingByPriority()
{	
	D3DXVECTOR3 CurPos;
	D3DXVECTOR3 PrePos;
	D3DXVECTOR3 LsnPos;
	for(int c = 0;c < SC_CTGY_LAST;c++)
	{
		CueInstance *pMostFarFromLsn = NULL;
		int nRstInx = -1;
		for(int i = 0;i < MAX_CUE_NUM;i++)
		{
			if(m_CueList[i].bEnable == false)
				continue;

			if( m_CueList[i].nCtgy == g_CtgyPriority[c] )
			{
				if(pMostFarFromLsn == NULL){
					pMostFarFromLsn = &m_CueList[i];
					nRstInx = i;
				}
				else{
					CurPos = D3DXVECTOR3(m_CueList[i].Pos.x,m_CueList[i].Pos.y,m_CueList[i].Pos.z);
					PrePos = D3DXVECTOR3(pMostFarFromLsn->Pos.x,pMostFarFromLsn->Pos.y,pMostFarFromLsn->Pos.z);
					LsnPos = D3DXVECTOR3(m_vListenerPosition.x,m_vListenerPosition.y,m_vListenerPosition.z);

					//float fLength1 =  D3DXVec3Length( &(CurPos - LsnPos) );
					//float fLength2 =  D3DXVec3Length( &(PrePos  - LsnPos) );

					if(D3DXVec3Length( &(CurPos - LsnPos) ) >  D3DXVec3Length( &(PrePos  - LsnPos) ))
					{
						pMostFarFromLsn = &m_CueList[i];
						nRstInx = i;
					}
				}
			}
		}
		if(pMostFarFromLsn) //검색된 게 있으툈E
		{
			SoundDebugString("Ctgy : %d, Stopped Sound index %d",pMostFarFromLsn->nCtgy,nRstInx);
			StopSound( nRstInx );
			return nRstInx;
		}
	}
	return -1;
}

void CXACTSound::SoundDebugString(CueInstance* pInstance,const char* format, ... )
{
#ifdef _DEBUG_SOUND_CODE
	bool bOutput = false;
	if( pInstance && pInstance->bOutPutString ){	
		bOutput = true;
	}
	else if( pInstance == NULL ){	
		bOutput = true;
	}
	if(bOutput)
	{
		char buf[1024];
		va_list	arg;
		va_start( arg, format );
		_vsnprintf( buf, 1024, format, arg );
		va_end( arg );
		OutputDebugString( buf );
	}
#endif
}
void CXACTSound::SoundDebugString(const char* format, ... )
{
#ifdef _DEBUG_SOUND_CODE
	char buf[1024];
	va_list	arg;
	va_start( arg, format );
	_vsnprintf( buf, 1024, format, arg );
	va_end( arg );
	OutputDebugString( buf );
#endif
}

bool CXACTSound::IsOwner( HANDLE hHandle,int nIndex )
{
	if(nIndex == -1){	
		return false;
	}
	if( nIndex >= MAX_CUE_NUM){
		return false;
	}
	if( m_CueList[ nIndex ].Handle == hHandle ){	
		return true;
	}
	return false;

}

void CXACTSound::SetFadeInValue(int nIndex, float fDeltaTick)
{
	if( nIndex == -1 || nIndex >= MAX_CUE_NUM ){	
		BsAssert( 0 && "BGM Fade index wrong");
		return;
	}
	m_CueList[ nIndex ].fFadeDelta = fDeltaTick;


}
#endif //_XBOX
