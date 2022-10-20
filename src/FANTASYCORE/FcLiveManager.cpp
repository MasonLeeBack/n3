#include "stdafx.h"

#ifdef _XBOX
#include "xonline.h"
#include "AtgSignIn.h"
#include "N3GameConfig.spa.h"
#include "FcLiveManager.h"
#include "FcAchievement.h"
#include "FcCommon.h"
#include "FcGlobal.h"
#include "FantasyCore.h"
#include "BsMoviePlayer.h"
#include "FcRealtimeMovie.h"
#include "FcInterfaceManager.h"
#include "FcSoundManager.h"
#include "InputPad.h"

#ifdef _DEBUG
#pragma comment( lib, "xonlined.lib" )
#else
#pragma comment( lib, "xonline.lib" )
#endif

#define _SLEEP_PERIOD 2000

HANDLE g_hTerminateThreadEvent = 0;
HANDLE g_hThreadQuittingEvent = 0;
static CBsCriticalSection s_csSigninLock;

DWORD WINAPI UpdateThreadProc( LPVOID lpParameter )
{
	CFcLiveManager *pManager = (CFcLiveManager*)lpParameter;

	while( 1 )
	{
		if( WaitForSingleObject( g_hTerminateThreadEvent, 0 ) == WAIT_OBJECT_0 )
		{
			SetEvent(g_hThreadQuittingEvent);
			break;
		}
		if( pManager->IsUIActive() == FALSE )
		{
			THREAD_AUTOLOCK(&s_csSigninLock);
			pManager->UpdatePresence();
		}
		Sleep(_SLEEP_PERIOD);
	}

	return 1;
}


CFcLiveManager::CFcLiveManager()
{
	ZeroMemory( m_UserData, sizeof( m_UserData ) );
	m_dwUserCount = 0;
	m_dwSignedInUserMask = 0;
	ZeroMemory( m_bFriendsUIActive, sizeof( m_bFriendsUIActive ) );
	m_bUIActive = FALSE;
	m_hNotification = 0;

	m_dwStartTick = 0;
	m_dwPresenceCount = 0;
	m_pAchievement = NULL;

	m_dwWorkerThreadID = 0;
	m_hWorkerThread = 0;
	m_pSettingResults = NULL;
	m_nYAxis_Inversion = 0;
	m_bController_Vibration = true;
	ZeroMemory( &m_Overlapped, sizeof(m_Overlapped) );
	m_dwOverlappedReadStatus = ERROR_SUCCESS;

	m_pfnProfileProcess = CFcLiveManager::EmptyProcess;
	
	memset(szCurrContext, 0, sizeof(szCurrContext));
	memset(szSaveContext, 0, sizeof(szSaveContext));
	nCurrCode = 0;
	nSaveCode = 0;
	
	MakePresenceMap();
}

CFcLiveManager::~CFcLiveManager()
{
	ReleaseProfileBuffer();
	ClearPresenceMap();

	if(m_hWorkerThread)
	{
		SetEvent(g_hTerminateThreadEvent);
		WaitForSingleObject( g_hThreadQuittingEvent, INFINITE);
		CloseHandle(m_hWorkerThread);
	}
	if(g_hTerminateThreadEvent)
		CloseHandle(g_hTerminateThreadEvent);
	if(g_hThreadQuittingEvent)
		CloseHandle(g_hThreadQuittingEvent);

	SAFE_DELETE( m_pAchievement );
}

// tfukui@MS: Allow to connect to PC/server
#ifdef _TNT_
#ifdef _TICKET_TRACKER_
VOID CFcLiveManager::InitializeXNet(VOID)
{
	int status;
#ifdef _XBOX
	// init the xnet libraries
	XNetStartupParams xnsp;
	memset(&xnsp, 0, sizeof(xnsp));
	xnsp.cfgSizeOfStruct = sizeof(XNetStartupParams);
	xnsp.cfgFlags = XNET_STARTUP_BYPASS_SECURITY;
	status = XNetStartup(&xnsp);

	XNADDR xnHostAddr;
	DWORD dwStatus;
	do {
		// do other work in this loop
		dwStatus = XNetGetTitleXnAddr( &xnHostAddr );
	} while( dwStatus == XNET_GET_XNADDR_PENDING );
	
#endif // _XBOX
	status = WSASYSNOTREADY;
	WORD wVersionRequested = MAKEWORD( 2, 2 );
	WSADATA wsaData;
	status = WSAStartup( wVersionRequested, &wsaData );
}
#endif // _TICKET_TRACKER_
#endif // _TNT_


HRESULT	CFcLiveManager::Initialize(VOID)
{
	ZeroMemory( m_UserData, sizeof( m_UserData ) );
	for( DWORD i = 0; i < NUM_USERS; ++i )
		m_UserData[i].dwLastPresenceEnumerate = GetTickCount();

	// Initialize Live
#ifdef _TNT_
#ifdef _TICKET_TRACKER_
	InitializeXNet();
#endif //_TICKET_TRACKER_
#endif //_TNT_

	if( FAILED( XOnlineStartup() ) )
		return E_FAIL;

	m_bUIActive = FALSE;

	ATG::SignIn::Initialize( NUM_MIN_USERS, NUM_MAX_USERS, FALSE , NUM_SIGNIN_PANES );
	// ATG::SignIn::ShowSignInUI();

	m_hNotification = XNotifyCreateListener( XNOTIFY_SYSTEM | XNOTIFY_LIVE );
	if( m_hNotification == NULL || m_hNotification == INVALID_HANDLE_VALUE )
		return E_FAIL;

	m_dwStartTick = GetTickCount();
	m_dwPresenceCount = 0;
	m_dwUserCount = 0;
	m_dwSignedInUserMask = 0;
	ZeroMemory( m_bFriendsUIActive, sizeof( m_bFriendsUIActive ) );

	for ( DWORD UserIndex = 0; UserIndex < XUSER_MAX_COUNT; UserIndex++ )
	{
		// check for signed in user
		if ( XUserGetXUID( UserIndex, &m_UserData[m_dwUserCount].xuid ) == ERROR_SUCCESS)
		{
			m_UserData[m_dwUserCount].dwUserIndex = UserIndex;
			m_UserData[m_dwUserCount].dwPresenceID = 0;
			m_UserData[m_dwUserCount].dwDescID = 0;

			m_bFriendsUIActive[m_dwUserCount] = TRUE;
			m_dwUserCount++;
			m_dwSignedInUserMask |= (1<<UserIndex);

			if( m_dwUserCount >= NUM_USERS )
				break;
		}
	}

	// 처음 자동 로그인 되는 유저가 있다면..
	if( m_dwUserCount )
	{
		if( ProfileInitialize() )
		{
			ReadProfileSettings();
		}
	}
	
	m_pAchievement = new CFcAchievement;
	m_pAchievement->Initialize();
	
	g_hTerminateThreadEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
	g_hThreadQuittingEvent= CreateEvent( NULL, FALSE, FALSE, NULL );
	m_hWorkerThread = CreateThread( NULL, 65536, UpdateThreadProc, this, 0, &m_dwWorkerThreadID );
	DebugString("Create Thread : UpdateThreadProc() Handle=%d(%d)\n", m_hWorkerThread, m_dwWorkerThreadID);
	if(m_hWorkerThread)
		XSetThreadProcessor( m_hWorkerThread, 3);

	if(m_hWorkerThread && g_hThreadQuittingEvent && g_hThreadQuittingEvent)
		return S_OK;
	else
		return E_FAIL;
}

VOID CFcLiveManager::CheckSignedInInfo(VOID)
{
	THREAD_AUTOLOCK(&s_csSigninLock);

	m_dwUserCount = 0;
	for( DWORD dwCnt = 0; dwCnt < XUSER_MAX_COUNT; ++dwCnt )
	{
		if( ATG::SignIn::IsUserSignedIn( dwCnt ) )
		{
			m_UserData[m_dwUserCount].dwUserIndex = dwCnt;
			m_UserData[m_dwUserCount].dwPresenceID = 0;
			m_UserData[m_dwUserCount].dwDescID = 0;

			XUserGetXUID( dwCnt, &m_UserData[m_dwUserCount].xuid );
			m_dwUserCount++;
			m_bFriendsUIActive[m_dwUserCount] = TRUE;
		}
	}

	// 
	m_dwSignedInUserMask = ATG::SignIn::GetSignedInUserMask();

	// 나중에 GAME_STAGE_ID 값 이용할 것.
	if( g_FCGameData.cMapFileName[0] != NULL )
	{
		if(strcmp(g_FCGameData.cMapFileName, _STAGE_TUTORIAL_FILENAME) == 0)
		{
			SetRichPresenceInfo( LIVE_CONTEXT_MENU, LIVE_CONST_TUTORIAL );
		}
		else
		{
			SetRichPresenceInfo( LIVE_CONTEXT_MAP, g_FCGameData.cMapFileName );
		}
	}
	else
	{
		SetRichPresenceInfo( LIVE_CONTEXT_MENU , LIVE_CONST_TITLE );
	}
}

VOID CFcLiveManager::Process(VOID)
{
	// Call m_pfnProfileProcess here so there is one frame between
	// UpdateAndRetrieveNotify() and test for IO completion...
	(*m_pfnProfileProcess)( this );

	UpdateAndRetrieveNotify();
	// BringUpFriendsUI();		// 동작시 친구 리스트 XUI를 띄운다.
	ProcessDeviceUI();

	if( IsUIActive() == FALSE )
	{
		// Note: UpdatePresence has been moved to a worker thread
		UpdateAchievement();
	}
}

VOID	CFcLiveManager::ProcessDeviceUI(VOID)
{
	if( ( !g_FCGameData.bShowDeviceUI ) 
		&& ( !g_FCGameData.bOnDeviceUI )
		&& ( ATG::SignIn::GetSignedInUser() != -1 )  
		&& ( !ATG::SignIn::IsSystemUIShowing() ) )
	{
		g_FCGameData.ShowDeviceUI();		
	}
}

extern bool s_bCheckSaveDataCount; // = false;

HRESULT	 CFcLiveManager::UpdateAndRetrieveNotify(VOID)
{
	// Retreive UI state
	ATG::SignIn::Update();

	DWORD dwNotificationId;
	ULONG ulParam;

	if( XNotifyGetNext( m_hNotification, 0, &dwNotificationId, &ulParam ) )
	{
		switch( dwNotificationId )
		{
		case XN_SYS_SIGNINCHANGED: 
			{
				bool bReset = false;
				DWORD dwOldUsers = m_dwUserCount;				
				DWORD dwOldSignedInUserMask = m_dwSignedInUserMask;

				CheckSignedInInfo();			

				if( m_dwUserCount )
				{
					if( ProfileInitialize() )
					{
						ReadProfileSettings();
					}
				}
// [beginmodify] 2006/2/13 junyash PS#4937 Vibration turn on, when sign-out
				else
				{
					m_nYAxis_Inversion = 0;
					m_bController_Vibration = true;
				}
// [endmodify] junyash

				if(g_FCGameData.nEnablePadID==-1) // 타이틀 화면 진입 안했음			
					break;				

				// 사인 인/아웃된 패드ID 가 현재 활성화된 유저의 패드인지 체크
				if(( (dwOldSignedInUserMask^m_dwSignedInUserMask)&(1<<g_FCGameData.nEnablePadID)) == 0)	
				{
					// 현재 활성화된 유저의 패드 번호에 아무런 변화가 없었다면 
					// (사인아웃후 곧바로 다른프로필로 사인인한 경우)
					// 프로필이름이 바뀌어 있는지 검사. 
					char szProfileName[128] = {0,};
					XUserGetName(g_FCGameData.nEnablePadID, szProfileName, sizeof(szProfileName));
					if(strcmp(szProfileName, g_FCGameData.szProfileName)==0) // 바뀌지 않았다면 통과
						break;
					else
						bReset = true;
				}

				if(bReset || dwOldUsers != m_dwUserCount) {	
					s_bCheckSaveDataCount = false;
					g_FCGameData.bChangeProfile = true;
					g_FCGameData.tempFixedSave.Clear();
					g_FCGameData.nSaveGameCount = 0;
					g_FCGameData.bShowDeviceUI = false;
					g_FCGameData.bRemovedStorageDevice = false;
					if(ATG::SignIn::IsUserSignedIn(g_FCGameData.nEnablePadID)==TRUE) // 프로필 네임 업데이트
						XUserGetName(g_FCGameData.nEnablePadID, g_FCGameData.szProfileName, sizeof(g_FCGameData.szProfileName));	

					g_MenuHandle->PostMessage(fcMSG_GO_TO_TITLE);
					g_FCGameData.bShowDeviceUI = true;
					g_FCGameData.nEnablePadID = -1;
					g_FCGameData.m_DeviceID = XCONTENTDEVICE_ANY;
					g_FCGameData.m_OldDeviceID = XCONTENTDEVICE_ANY;

					g_FCGameData.bFirstInitProfile = false;

					//Mocha..
					//시모다씨의 문제점...
					//sign out시에 m_bController_Vibration, m_nYAxis_Inversion값이 정확하지 안아서
					//밑의 경우처럼 분리해서 따로 처리하고 있다.
					//꼭 확인 부탁할 것
					if(bReset || dwOldUsers < m_dwUserCount)
					{
						g_FCGameData.ConfigInfo.bVibration = m_bController_Vibration;
						g_FCGameData.ConfigInfo.bCameraUD_Reverse = m_nYAxis_Inversion ? true : false;
					}
					else
					{
						g_FCGameData.ConfigInfo.bVibration = true;
						g_FCGameData.ConfigInfo.bCameraUD_Reverse = false;
					}

					g_FCGameData.ConfigInfo.nBGMVolume	 = 10;
					g_FCGameData.ConfigInfo.nSoundVolume = 10;
					g_FCGameData.ConfigInfo.nVoiceVolume = 10;
					g_FCGameData.ConfigInfo.nContrast = 5;

					CInputPad::GetInstance().SetVibrationOn(g_FCGameData.ConfigInfo.bVibration);
					g_BsMoviePlayer.SetVolume( (int)((g_FCGameData.ConfigInfo.nBGMVolume * 0.1f) * (float)_MAX_BINK_VOLUME) );
					g_pSoundManager->SetVolume(FC_MENU_VOL_BGM, g_FCGameData.ConfigInfo.nBGMVolume * 0.1f);
					g_pSoundManager->SetVolume(FC_MENU_VOL_EFT_SOUND, g_FCGameData.ConfigInfo.nSoundVolume * 0.1f);
					g_pSoundManager->SetVolume(FC_MENU_VOL_VOICE, g_FCGameData.ConfigInfo.nVoiceVolume * 0.1f);
					g_BsKernel.SetGammaRamp(float(g_FCGameData.ConfigInfo.nContrast)/10.f);                    
				}

				DebugString(" LiveManager::UpdateAndRetrieveNotify - XN_SYS_SIGNINCHANGED Msg\n" );

				
			}			
			break;

		case XN_SYS_UI:
			{
				// bool bPreUIActive = (m_bUIActive == TRUE);
				BOOL bPreUIActive = m_bUIActive;
				m_bUIActive = (BOOL)ulParam;
				
				if(ATG::SignIn::IsUserSignedIn(g_FCGameData.nEnablePadID)
					&& g_FCGameData.bShowDeviceUI == false
					&& m_bUIActive == false
					&& g_FCGameData.bChangeProfile == false) 
				{
					if(g_FCGameData.m_DeviceID == XCONTENTDEVICE_ANY){
						//g_MenuHandle->CheckedSystemMessageBox(_SYS_MSG_CANCEL_DEVICESELECTOR, (DWORD)(false));
						g_FCGameData.bCancelDeviceSelector=true;
					}


					g_FCGameData.bShowDeviceUI = true;
					g_FCGameData.bOnDeviceUI = false;
				}
				// else if((m_bUIActive==TRUE) != bPreUIActive)
				else if( m_bUIActive != bPreUIActive )
				{
					g_FC.SetPauseForXUI( m_bUIActive ? true : false );
				}

				if(g_FCGameData.bChangeProfile && m_bUIActive == false){
					g_FCGameData.bChangeProfile = false;
				}

// [beginmodify] 2006/2/13 junyash PS#2795,4786, use Gamer profile setting for Vibration
				if( m_dwUserCount )
				{
					if( ProfileInitialize() )
					{
						ReadProfileSettings();
					}
				}
// [beginmodify] 2006/2/13 junyash PS#4937 Vibration turn on, when sign-out
				else
				{
					m_nYAxis_Inversion = 0;
					m_bController_Vibration = true;
				}
// [endmodify] junyash
// [endmodify] junyash
			}
			break;
		case XN_SYS_STORAGEDEVICESCHANGED :
			{	
				// 사용중인 디바이스의 유무를 검사, 없으면 경고 메시지를 띄움				
				if(g_FCGameData.m_DeviceID == XCONTENTDEVICE_ANY || g_FCGameData.nEnablePadID == -1)
					break;

				DWORD dwRet = XContentGetDeviceState(g_FCGameData.m_DeviceID, NULL);
				if(dwRet == ERROR_DEVICE_NOT_CONNECTED) {
					g_FCGameData.m_DeviceID = XCONTENTDEVICE_ANY;
					// g_MenuHandle->CheckedSystemMessageBox(_SYS_MSG_REMOVE_STORAGE, (DWORD)(false));
					g_FCGameData.bRemovedStorageDevice=true;
					g_FCGameData.nSaveGameCount = 0;
				}               				

				DebugString(" LiveManager::UpdateAndRetrieveNotify - XN_SYS_STORAGEDEVICESCHANGED Msg\n" );
			}
			break;
		case XN_SYS_INPUTDEVICESCHANGED :
			{
				// 메뉴 구성 되면 처리할 부분.
				// ERROR_DEVICE_NOT_CONNECTED
				BOOL bRemove = FALSE;
				XINPUT_CAPABILITIES InpInfo;

				if(g_FCGameData.nEnablePadID != -1)
				{
					if( XInputGetCapabilities( g_FCGameData.nEnablePadID, XINPUT_FLAG_GAMEPAD, &InpInfo ) 
						== ERROR_DEVICE_NOT_CONNECTED ){
						bRemove = true;
					}
				}
				else
				{
					bRemove = TRUE;

					for ( DWORD UserIndex = 0; UserIndex < XUSER_MAX_COUNT; UserIndex++ )
					{
						if( XInputGetCapabilities( UserIndex, XINPUT_FLAG_GAMEPAD, &InpInfo ) 
							== ERROR_DEVICE_NOT_CONNECTED )
						{
							// 패드가 없다.
							DebugString(" LiveManager::XNotifyGetNext - Input[%d] - ERROR_DEVICE_NOT_CONNECTED\n", UserIndex );					
							// bRemove = ( bRemove || TRUE);
						}
						else
						{
							// 패드가 있다.
							DebugString(" LiveManager::XNotifyGetNext - Input[%d] - SUCCESS\n");
							bRemove = FALSE; //( bRemove || FALSE );
						}
					}
				}

				if(bRemove == TRUE){
					g_MenuHandle->CheckedSystemMessageBox(_SYS_MSG_REMOVE_PAD, (DWORD)bRemove);
				}

				return S_OK;
			}
			break;
		case XN_LIVE_LINK_STATE_CHANGED :
			{
				// BOOL
				// Set to TRUE if the link is connected; FALSE if disconnected.
				if( ((BOOL)ulParam) == TRUE )
				{
					return S_OK;
				}
				else
				{
					return S_OK;
				}
			}
			break;
		case XN_LIVE_CONNECTIONCHANGED :
			{
				// HRESULT hrLiveConnection
				// Current status of the Live connection.
				// Returns XONLINE_S_LOGON_CONNECTION_ESTABLISHED if the connection is valid. 
				// Otherwise, returns one of the following errors:

				//  XONLINE_E_LOGON_NO_NETWORK_CONNECTION 
				//	XONLINE_E_LOGON_CANNOT_ACCESS_SERVICE 
				//	XONLINE_E_LOGON_UPDATE_REQUIRED 
				//	XONLINE_E_LOGON_SERVERS_TOO_BUSY 
				//	XONLINE_E_LOGON_CONNECTION_LOST 
				//	XONLINE_E_LOGON_KICKED_BY_DUPLICATE_LOGON 
				//	XONLINE_E_LOGON_INVALID_USER
				return S_OK;
			}
			break;
		case XN_SYS_PROFILESETTINGCHANGED :
			{
// [beginmodify] 2006/2/13 junyash PS#4786, use Gamer profile setting for Vibration
				if( m_dwUserCount )
				{
					if( ProfileInitialize() )
					{
						ReadProfileSettings();
					}
				}
// [beginmodify] 2006/2/13 junyash PS#4937 Vibration turn on, when sign-out
				else
				{
					m_nYAxis_Inversion = 0;
					m_bController_Vibration = true;
				}
// [endmodify] junyash
// [endmodify] junyash
				return S_OK;
			}
			break;

		}
	}

	return S_OK;
}

VOID CFcLiveManager::BringUpFriendsUI(VOID)
{
	if( m_bUIActive == FALSE )
	{
		if( ATG::SignIn::AreUsersSignedIn() )
		{
			// Bring up the Friends UI for each one of the users one at a time
			for( DWORD dwCnt = 0; dwCnt < m_dwUserCount; ++dwCnt )
			{
				if( m_bFriendsUIActive[dwCnt] == FALSE )
				{
					DWORD dwRet;
					dwRet = XShowFriendsUI( m_UserData[dwCnt].dwUserIndex );
					assert( dwRet == ERROR_SUCCESS );

					m_bUIActive = TRUE;
					m_bFriendsUIActive[dwCnt] = TRUE;
					break;
				}
			}
		}
	}
}

VOID CFcLiveManager::ShowSigninUI(VOID)
{
	ATG::SignIn::ShowSignInUI();
}


VOID CFcLiveManager::ShowAchievementUI(VOID)
{
	m_pAchievement->ShowAchievementUI();
}

VOID CFcLiveManager::UpdatePresence()
{	// This function is currently called in a separate thread thread due to
	// the fact that XUserSetProperty stalls...
	for( DWORD dwCnt = 0; dwCnt < m_dwUserCount; ++dwCnt )
	{
		XUserSetContext( m_UserData[dwCnt].dwUserIndex, 
			CONTEXT_GAME_RICHPRESENCE_LIST, m_UserData[dwCnt].dwPresenceID );
		XUserSetContext( m_UserData[dwCnt].dwUserIndex, 
			CONTEXT_GAME_SECOND_MSG_LIST, m_UserData[dwCnt].dwDescID );

	}
}

VOID CFcLiveManager::EnumeratePresence( DWORD dwUserIndex )
{
	DWORD   cbBuffer;
	HANDLE  hFriendsEnum;

	m_UserData[dwUserIndex].dwLastPresenceEnumerate = GetTickCount();

	// Enumerate presence strings of friends
	DWORD dwRet;
	dwRet = XFriendsCreateEnumerator(
		dwUserIndex,                    // user of whom to enumerate friends
		0,                              // starting index
		MAX_FRIENDS,                    // max number of friends
		&cbBuffer,                      // size of buffer needed
		&hFriendsEnum );

	// Presence information not yet available
	if( dwRet != ERROR_SUCCESS )
		return;

	dwRet = XEnumerate( hFriendsEnum, m_Friends, sizeof( m_Friends ), &m_dwPresenceCount, NULL );
	if( dwRet != ERROR_SUCCESS )
		return;

	CloseHandle( hFriendsEnum );
}

VOID CFcLiveManager::UpdateAchievement()
{
	m_pAchievement->Update();
}

BOOL	CFcLiveManager::HasSignedInUser(VOID)
{
	return ( ATG::SignIn::GetSignedInUser() != -1 );
}
BOOL	CFcLiveManager::IsUserSignedIn(VOID)const
{

	return g_FCGameData.nEnablePadID !=-1 && ATG::SignIn::IsUserSignedIn(g_FCGameData.nEnablePadID);
}

void	CFcLiveManager::SetRichPresenceMainID(DWORD dwPCode/*=0*/ )
{
	for( DWORD dwCnt = 0; dwCnt < m_dwUserCount; ++dwCnt )
	{
		m_UserData[dwCnt].dwPresenceID = dwPCode;
	}
}

void	CFcLiveManager::SetRichPresenceDescID(DWORD dwDCode/*=0*/ )
{
	for( DWORD dwCnt = 0; dwCnt < m_dwUserCount; ++dwCnt )
	{
		m_UserData[dwCnt].dwDescID = dwDCode;
	}
}


void	CFcLiveManager::SetRichPresenceInfo( int nCode,const char* szMapName )
{
	mapStrPack::iterator it;

	strcpy(szCurrContext, szMapName);
	szCurrContext[strlen(szMapName)] = 0;
	nCurrCode = nCode;

	if( nCode == LIVE_CONTEXT_MAP )
	{
		char szTemp[32];
		strcpy( szTemp, szMapName );
		szTemp[0] = tolower( szTemp[0] );
		szTemp[3] = tolower( szTemp[3] );

		char* pPoint = strrchr( szTemp , '.' );

		if( pPoint != NULL )
		{
			*pPoint = NULL;

			it = m_mapPresence.find( szTemp );

			if( it != m_mapPresence.end() )
			{
				SetRichPresenceMainID( it->second.nMsg1 );
				SetRichPresenceDescID( it->second.nMsg2 );
			}
			else
			{
				SetRichPresenceMainID( CONTEXT_GAME_RICHPRESENCE_LIST_TITLESCENE );
				SetRichPresenceDescID( CONTEXT_GAME_SECOND_MSG_LIST_TITLEDESC );
#ifdef DEBUG
				BsAssert( 0 && "Not found RichPresence Info " );
#endif
			}
		}
		else
		{
			SetRichPresenceMainID( CONTEXT_GAME_RICHPRESENCE_LIST_TITLESCENE );
			SetRichPresenceDescID( CONTEXT_GAME_SECOND_MSG_LIST_TITLEDESC );
#ifdef DEBUG
			BsAssert( 0 && "Not found RichPresence Info " );
#endif
		}
	}
	else
	{
		it = m_mapPresence.find( szMapName );

		if( it != m_mapPresence.end() )
		{
			SetRichPresenceMainID( it->second.nMsg1 );
			SetRichPresenceDescID( it->second.nMsg2 );
		}
		else
		{
			SetRichPresenceMainID( CONTEXT_GAME_RICHPRESENCE_LIST_TITLESCENE );
			SetRichPresenceDescID( CONTEXT_GAME_SECOND_MSG_LIST_TITLEDESC );
#ifdef DEBUG
			BsAssert( 0 && "Not found RichPresence Info " );
#endif
		}
	}
}

void	CFcLiveManager::MakePresenceMap(void)
{
	ClearPresenceMap();

	std::pair< std::string , MsgContextPack > pairPresence;

	m_mapPresence[ "title" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_TITLESCENE,CONTEXT_GAME_SECOND_MSG_LIST_TITLEDESC);
	m_mapPresence[ "character" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_CHARACTERSELECT,CONTEXT_GAME_SECOND_MSG_LIST_CHARDESC);
	m_mapPresence[ "ending" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_ENDINGSCENE,CONTEXT_GAME_SECOND_MSG_LIST_ENDINGDESC);
	m_mapPresence[ "library" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_LIBRARYSCENE,CONTEXT_GAME_SECOND_MSG_LIST_LIBDESC);
	m_mapPresence[ "option" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_OPTIONSCENE,CONTEXT_GAME_SECOND_MSG_LIST_OPTIONDESC);
	m_mapPresence[ "tutorial" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_TUTORIALSCENE,CONTEXT_GAME_SECOND_MSG_LIST_TUTORIALDESC);
	m_mapPresence[ "save" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_SAVESCENE,CONTEXT_GAME_SECOND_MSG_LIST_SAVEDESC);
	m_mapPresence[ "loading" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_LOADSCENE,CONTEXT_GAME_SECOND_MSG_LIST_LOADDESC);
	m_mapPresence[ "paused" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_PAUSEDSCENE,CONTEXT_GAME_SECOND_MSG_LIST_PAUSEDESC);

	m_mapPresence[ "vf_in" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_PLAYVARRFARRINN,CONTEXT_GAME_SECOND_MSG_LIST_IN_VF);
	m_mapPresence[ "ev_in" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_PLAYEAURVARRIA,CONTEXT_GAME_SECOND_MSG_LIST_IN_EV);
	m_mapPresence[ "vf_in2" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_PLAYWYANDEEKGATE,CONTEXT_GAME_SECOND_MSG_LIST_IN_WY_WAY);
	m_mapPresence[ "wy_in" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_PLAYWYANDEEKBASE,CONTEXT_GAME_SECOND_MSG_LIST_IN_WY_BASE);
	m_mapPresence[ "ph_in" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_PLAYPHOLYA,CONTEXT_GAME_SECOND_MSG_LIST_IN_PH);
	m_mapPresence[ "yw_in" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_PLAYYWAUEUAR,CONTEXT_GAME_SECOND_MSG_LIST_IN_YW);
	m_mapPresence[ "ma_in" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_PLAYANOTHERWORLD,CONTEXT_GAME_SECOND_MSG_LIST_IN_MA);

	m_mapPresence[ "vf_as" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_PLAYVARRFARRINN,CONTEXT_GAME_SECOND_MSG_LIST_AS_VF);
	m_mapPresence[ "ev_as" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_PLAYEAURVARRIA,CONTEXT_GAME_SECOND_MSG_LIST_AS_EV);
	m_mapPresence[ "vz_as2" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_PLAYVARRVAZZARR,CONTEXT_GAME_SECOND_MSG_LIST_AS_VZ);
	m_mapPresence[ "wy_as" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_PLAYWYANDEEKBASE,CONTEXT_GAME_SECOND_MSG_LIST_AS_WY);
	m_mapPresence[ "ph_as" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_PLAYPHOLYA,CONTEXT_GAME_SECOND_MSG_LIST_AS_PH);
	m_mapPresence[ "yw_as" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_PLAYYWAUEUAR,CONTEXT_GAME_SECOND_MSG_LIST_AS_YW);

	m_mapPresence[ "he_my" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_PLAYFELPPE,CONTEXT_GAME_SECOND_MSG_LIST_MY_HE);
	m_mapPresence[ "vg_my" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_PLAYVARRGANDD,CONTEXT_GAME_SECOND_MSG_LIST_MY_VG);
	m_mapPresence[ "vz_my" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_PLAYICEGATE,CONTEXT_GAME_SECOND_MSG_LIST_MY_IG);
	m_mapPresence[ "vz_my0" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_PLAYEVERRMT,CONTEXT_GAME_SECOND_MSG_LIST_MY_EM);
	m_mapPresence[ "ph_my" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_PLAYPHOLYA,CONTEXT_GAME_SECOND_MSG_LIST_MY_PH);

	m_mapPresence[ "vg_ty" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_PLAYVARRGANDD,CONTEXT_GAME_SECOND_MSG_LIST_TY_VG);
	m_mapPresence[ "ev_ty" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_PLAYEAURVARRIA,CONTEXT_GAME_SECOND_MSG_LIST_TY_EV);
	m_mapPresence[ "wy_ty" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_PLAYWYANDEEKBASE,CONTEXT_GAME_SECOND_MSG_LIST_TY_WY);
	m_mapPresence[ "ph_ty" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_PLAYPHOLYA,CONTEXT_GAME_SECOND_MSG_LIST_TY_PH);

	m_mapPresence[ "vg_dw" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_PLAYVARRGANDD,CONTEXT_GAME_SECOND_MSG_LIST_DW_VG);
	m_mapPresence[ "vz_dw" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_PLAYICEGATE,CONTEXT_GAME_SECOND_MSG_LIST_DW_IG);
	m_mapPresence[ "vz_dw2" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_PLAYVARRVAZZARR,CONTEXT_GAME_SECOND_MSG_LIST_DW_VZ);
	m_mapPresence[ "ph_dw" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_PLAYPHOLYA,CONTEXT_GAME_SECOND_MSG_LIST_DW_PH);

	m_mapPresence[ "vg_kl" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_PLAYVARRGANDD,CONTEXT_GAME_SECOND_MSG_LIST_KL_VG);
	m_mapPresence[ "wy_kl" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_PLAYWYANDEEKBASE,CONTEXT_GAME_SECOND_MSG_LIST_KL_WY);
	m_mapPresence[ "ph_kl" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_PLAYPHOLYA,CONTEXT_GAME_SECOND_MSG_LIST_KL_PH);

	m_mapPresence[ "vg_vi" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_PLAYVARRGANDD,CONTEXT_GAME_SECOND_MSG_LIST_VI_VG);
	m_mapPresence[ "ph_vi" ] = 
		MsgContextPack(CONTEXT_GAME_RICHPRESENCE_LIST_PLAYPHOLYA,CONTEXT_GAME_SECOND_MSG_LIST_VI_PH);
}

void	CFcLiveManager::ClearPresenceMap(void)
{
	m_mapPresence.clear();
}

///////////////////////////////////////////////


BOOL	CFcLiveManager::ProfileInitialize()
{
	ZeroMemory( m_Settings, sizeof(XUSER_PROFILE_SETTING) * NUM_SETTINGS );
	ReleaseProfileBuffer();

	// return FALSE;

	m_dwSettingSizeMax = 0;
	DWORD dwErr;
	dwErr = XUserReadProfileSettings( 0,                      // A title in your family or 0 for the current title
		0,                      // Player index (not used)
		NUM_SETTINGS,           // Number of settings to read
		SettingIDs,             // List of settings to read
		&m_dwSettingSizeMax,    // Results size (0 to determine maximum)
		NULL,                   // Results (not used)
		NULL );                 // Overlapped (not used)

	BsAssert( dwErr == ERROR_INSUFFICIENT_BUFFER );
	BsAssert( m_dwSettingSizeMax > 0 );

	if( ( dwErr != ERROR_INSUFFICIENT_BUFFER ) || ( m_dwSettingSizeMax == 0) )
	{
		return FALSE;
	}

	// NOTE: The game is responsible for freeing this memory when it is no longer needed
	BYTE* pData = new BYTE [ m_dwSettingSizeMax ];
	m_pSettingResults = ( XUSER_READ_PROFILE_SETTING_RESULT* )pData;

	return TRUE;
}

void	CFcLiveManager::ReleaseProfileBuffer()
{
	if( m_pSettingResults )
	{
        delete [] (BYTE*)m_pSettingResults;
		m_pSettingResults = NULL;
	}
}

void	CFcLiveManager::ReadProfileSettings()
{
	// If there is still an outstanding IO then cancel it as we no longer
	// need the results and we need to re-use m_Overlapped
	if(m_pfnProfileProcess == CFcLiveManager::ProfileOverlappedIoPorcess)
		XCancelOverlapped(&m_Overlapped);

	ZeroMemory( &m_Overlapped, sizeof(m_Overlapped) );
	m_dwOverlappedReadStatus = ERROR_SUCCESS;
	DWORD dwUserIndex = ATG::SignIn::GetSignedInUser();

// [beginmodify] 2006/1/27 junyash PS#2795, TCR#067GP use Gamer profile setting for Vertical View Control
	// TODO: check why ATG::SignIn::GetSignedInUser(); return -1
	//DebugString(" LiveManager::ReadProfileSettings() ATG::SignIn::GetSignedInUser() %d\n", dwUserIndex );					
	if ( dwUserIndex > 3 ) dwUserIndex = 0;
// [endmodify] junyash

	m_dwOverlappedReadStatus = XUserReadProfileSettings( 0,                   // A title in your family or 0 for the current title
									dwUserIndex,  // Player index making the request
									NUM_SETTINGS,        // Number of settings to read
									SettingIDs,          // List of settings to read
									&m_dwSettingSizeMax, // Results size
									m_pSettingResults,   // Results go here
									&m_Overlapped );     // Overlapped struct

	BsAssert( m_dwOverlappedReadStatus == ERROR_SUCCESS || m_dwOverlappedReadStatus == ERROR_IO_PENDING );

	// Call immediately otherwise the m_Overlapped is invalid
	if ( m_dwOverlappedReadStatus == ERROR_SUCCESS )
	{
		ProfileOverlappedIoPorcess(this);
	}
	else
	{
		m_pfnProfileProcess = CFcLiveManager::ProfileOverlappedIoPorcess;
	}
}

VOID		CFcLiveManager::EmptyProcess(CFcLiveManager* pFcLM)
{
}

VOID		CFcLiveManager::ProfileOverlappedIoPorcess(CFcLiveManager* pFcLM)
{
	// If the read has completed, show the settings
	if( pFcLM->m_dwOverlappedReadStatus == ERROR_SUCCESS || XHasOverlappedIoCompleted( &pFcLM->m_Overlapped ) )
	{
		// Let the message display for a bit
		// Sleep( 1000 ); // remove in a real game

		// The sample doesn't check for read failures
		if( pFcLM->m_dwOverlappedReadStatus == ERROR_SUCCESS || XGetOverlappedExtendedError( &pFcLM->m_Overlapped ) == ERROR_SUCCESS )
		{
// [beginmodify] 2006/2/13 junyash PS#2795,4786 use User Profile
			#if 0
			for( DWORD i=0; i < NUM_SETTINGS; ++i )
			{
				XUSER_PROFILE_SETTING* pSrc = pFcLM->m_pSettingResults->pSettings + i;

				switch( XUserGetProfileSettingType( pSrc->dwSettingId ) )
				{
				case XUSER_DATA_TYPE_INT32:
					pFcLM->m_nYAxis_Inversion = (XSOURCE_NO_VALUE == pSrc->source ? 0 : pSrc->data.nData);
					break;
				}
			}
			#else
			#endif
			//DebugString(" LiveManager::ProfileOverlappedIoPorcess() XUserReadProfileSettings() SettingsLen %d\n", pFcLM->m_pSettingResults->dwSettingsLen );					
			for( DWORD i=0; i < NUM_SETTINGS; ++i )
			{
				XUSER_PROFILE_SETTING* pSrc = pFcLM->m_pSettingResults->pSettings + i;

				switch( pSrc->dwSettingId )
				{
				case XPROFILE_GAMER_YAXIS_INVERSION:
					pFcLM->m_nYAxis_Inversion = (XSOURCE_NO_VALUE == pSrc->source ? 0 : pSrc->data.nData);
					//DebugString(" XPROFILE_GAMER_YAXIS_INVERSION %d\n", pFcLM->m_nYAxis_Inversion );
					break;
				case XPROFILE_OPTION_CONTROLLER_VIBRATION:
					pFcLM->m_bController_Vibration = (XSOURCE_NO_VALUE == pSrc->source ? 0 : pSrc->data.nData) ? true : false;
					//DebugString(" XPROFILE_OPTION_CONTROLLER_VIBRATION %d\n", pFcLM->m_bController_Vibration );
					break;
				}
			}
// [endmodify] junyash
		}
		else
		{
			XCancelOverlapped(&pFcLM->m_Overlapped);

#ifdef _DEBUG
			DebugString( "OverlappedError code : %d\n", XGetOverlappedExtendedError( &pFcLM->m_Overlapped ) );
			BsAssert( 0 && "XGetOverlappedExtendedError( &pFcLM->m_Overlapped ) != ERROR_SUCCESS" );
#endif
		}

		pFcLM->m_pfnProfileProcess = CFcLiveManager::EmptyProcess;
	}
}

VOID CFcLiveManager::SaveContext()
{
	if(szCurrContext[0])
	{
		strcpy( szSaveContext, szCurrContext );
		szSaveContext[strlen(szCurrContext)] = 0;
	}
	nSaveCode = nCurrCode;
}

VOID CFcLiveManager::RestoreContext()
{
	if(szSaveContext[0])
		SetRichPresenceInfo(nSaveCode, szSaveContext);
}

#endif