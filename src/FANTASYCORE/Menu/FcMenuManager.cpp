#include "stdafx.h"

#include "BsKernel.h"
#include "InputPad.h"

#include "FcWorld.h"
#include "FcHeroObject.h"
#include "FantasyCore.h"

#include "FcMenuForm.h"
#include "FcMenuManager.h"

#include "BsUiClassIDManager.h"
#include "BsUiFontAttrManager.h"

#include "FcMessageDef.h"
#include "FcGlobal.h"

#include "BsMoviePlayer.h"
#include "FcCameraObject.h"

#include "FcPhysicsLoader.h"
#include "FcLiveManager.h"


#ifdef _XBOX
#include <Xmp.h>
#endif //_XBOX

//menu
#ifndef _LTCG
#include "FcDebugMenu.h"
#endif
#include "FcMenuMessageBox.h"

//start
#include "FcMenuLogo.h"
#include "FcMenuLoadData.h"
#include "FcMenuMainOpening.h"
#include "FcMenuTitle.h"

//title
#include "FcMenuSelectChar.h"
#include "FcMenuLibrary.h"
#include "FcMenuOption.h"

//mission
#include "FcMenuSelectStage.h"
#include "FcMenuUsual.h"
#include "FcMenuSaveData.h"
#include "FcMenuGuardian.h"

//library
#include "FcMenuCharStock.h"
#include "FcMenuMissionStock.h"
#include "FcMenuArtWork.h"

//mission end
#include "FcMenuGameResult.h"
#include "FcMenuStageResult.h"

// pause
#include "FcMenuPause.h"
#include "FcMenuStatus.h"
#include "FcMenuAbility.h"
#include "FcMenuControl.h"
#include "FcMenuMissionObj.h"

#include "BsUiGDIView.h"
#include "BSuiMovieTexture.h"
#include "BsOffScreenMgr.h"
#include "FcInterfaceManager.h"

#include "FcRealtimeMovie.h"

#include "FcSoundManager.h"
#include "FcGlobal.h"

#ifdef _XBOX
#include "atgsignin.h"
#endif //_XBOX

#include "FcUtil.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

#define _WAVE_AREA_X			_SCREEN_WIDTH/2
#define _WAVE_AREA_Y			_SCREEN_HEIGHT/2

#define _WAVE_DEFAULT_COUNT		1
#define _WAVE_DEFAULT_SIZE		256
#define _WAVE_DEFAULT_SPEED		2

#define _RTT_SIZE				512
#define _GAME_STATE_ABILITY_X	512
#define _GAME_STATE_ABILITY_Y	560


#define  MISSION_CLEAR_MAP_MYIFEE		"CEM.bsmap"
#define  MISSION_CLEAR_MAP_DWINGVATT	"CED01.bsmap"
#define  MISSION_CLEAR_MAP_VIGKVAGK		"PFVED.bsmap"
#define  MISSION_CLEAR_MAP_MA_INPHYY	"AWED.bsmap"

//--- tohoshi
#if 1
// BINK volume
#define MOVIE_VOLUME_LOGO_START				(65536/1)
#define MOVIE_VOLUME_OPENING_START			(65536/1)		// Opening Movie
#define MOVIE_VOLUME_CREDITS_START			(65536/1)
#define MOVIE_VOLUME_MISSION_START			(65536/2)		// Character Movie
#define MOVIE_VOLUME_END_START				(65536/1)

// fcMSG_LOGO_START
bool SkipMovieCallback_fcMSG_LOGO_START(void* pParam)
{
	CBsMoviePlayer* pMoviePlayer = (CBsMoviePlayer*)pParam;
	pMoviePlayer->SetVolume( (int)((g_FCGameData.ConfigInfo.nBGMVolume * 0.1f) * (float)_MAX_BINK_VOLUME) );
	
	g_FC.ProcessLive();
	if(g_FCGameData.bChangeProfile)
	{
		return true;
	}

	CInputPad::GetInstance().Process();
	return( CInputPad::GetInstance().GetKeyPressTick(g_FCGameData.nEnablePadID, PAD_INPUT_START) ||
		CInputPad::GetInstance().GetKeyPressTick(g_FCGameData.nEnablePadID, PAD_INPUT_A) );
}
// fcMSG_OPENING_START
bool SkipMovieCallback_fcMSG_OPENING_START(void* pParam)
{
	CBsMoviePlayer* pMoviePlayer = (CBsMoviePlayer*)pParam;
	pMoviePlayer->SetVolume( (int)((g_FCGameData.ConfigInfo.nBGMVolume * 0.1f) * (float)_MAX_BINK_VOLUME) );
	
	g_FC.ProcessLive();
	if(g_FCGameData.bChangeProfile)
		return true;

	CInputPad::GetInstance().Process();
	return( CInputPad::GetInstance().GetKeyPressTick(g_FCGameData.nEnablePadID, PAD_INPUT_START) ||
		CInputPad::GetInstance().GetKeyPressTick(g_FCGameData.nEnablePadID, PAD_INPUT_A) );
}
// fcMSG_CREDITS_START
bool SkipMovieCallback_fcMSG_CREDITS_START(void* pParam)
{
	CBsMoviePlayer* pMoviePlayer = (CBsMoviePlayer*)pParam;
	pMoviePlayer->SetVolume( (int)((g_FCGameData.ConfigInfo.nBGMVolume * 0.1f) * (float)_MAX_BINK_VOLUME) );
	
	g_FC.ProcessLive();
	if(g_FCGameData.bChangeProfile)
		return true;

	CInputPad::GetInstance().Process();
	return( CInputPad::GetInstance().GetKeyPressTick(g_FCGameData.nEnablePadID, PAD_INPUT_START) ||
		CInputPad::GetInstance().GetKeyPressTick(g_FCGameData.nEnablePadID, PAD_INPUT_A) );
}
// SkipMovieCallback_fcMSG_FIRST_MISSION_START
bool SkipMovieCallback_fcMSG_FIRST_MISSION_START(void* pParam)
{
	CBsMoviePlayer* pMoviePlayer = (CBsMoviePlayer*)pParam;
	pMoviePlayer->SetVolume( (int)((g_FCGameData.ConfigInfo.nBGMVolume * 0.1f) * (float)_MAX_BINK_VOLUME) );
	
	g_FC.ProcessLive();
	if(g_FCGameData.bChangeProfile)
		return true;

	CInputPad::GetInstance().Process();
	return( CInputPad::GetInstance().GetKeyPressTick(g_FCGameData.nEnablePadID, PAD_INPUT_START) ||
		CInputPad::GetInstance().GetKeyPressTick(g_FCGameData.nEnablePadID, PAD_INPUT_A) );
}
// SkipMovieCallback_fcMSG_END_START
bool SkipMovieCallback_fcMSG_END_START(void* pParam)
{
	CBsMoviePlayer* pMoviePlayer = (CBsMoviePlayer*)pParam;
	pMoviePlayer->SetVolume( (int)((g_FCGameData.ConfigInfo.nBGMVolume * 0.1f) * (float)_MAX_BINK_VOLUME) );
	
	g_FC.ProcessLive();
	if(g_FCGameData.bChangeProfile)
		return true;

	CInputPad::GetInstance().Process();
	return( CInputPad::GetInstance().GetKeyPressTick(g_FCGameData.nEnablePadID, PAD_INPUT_START) ||
		CInputPad::GetInstance().GetKeyPressTick(g_FCGameData.nEnablePadID, PAD_INPUT_A) );
}

#else
bool SkipMovieCallback()
{
	CInputPad::GetInstance().Process();
	return( CInputPad::GetInstance().GetKeyPressTick(g_FCGameData.nEnablePadID, PAD_INPUT_START) ||
		CInputPad::GetInstance().GetKeyPressTick(g_FCGameData.nEnablePadID, PAD_INPUT_A) );
}
#endif
//--- tohoshi

std::vector<int> CFcMenuManager::m_PreLoadSkinList;
std::vector<int> CFcMenuManager::m_PreLoadAniList;
std::vector<int> CFcMenuManager::m_PreLoadTextureList;
std::vector<int> CFcMenuManager::m_PreLoadFXList;

CFcMenuManager::CFcMenuManager()
: CFcInterfaceObj()
{
	m_hMovieRTTHandle = NULL;
	m_nMovieTextureId = -1;

	m_h3DObjRTTHandle = NULL;
	m_pFcMenu3DObj = NULL;

	m_pMenuWorldMap = NULL;
	m_pMenuWorldMapInfo = NULL;

	m_nGamePauseMenuType = _FC_MENU_NONE;

	m_bSetToggleOpenMovie = true;

	m_AddWaveCount = 0;
	m_nMAXWaveCount = _WAVE_DEFAULT_COUNT;
	m_nWaveSize = _WAVE_DEFAULT_SIZE;
	m_nWaveSpeed = _WAVE_DEFAULT_SPEED;
	
	m_WaveState = _MENU_WAVE_END;
	m_pMenuWorldMapInfo = new FcMenuWorldMapInfo();

	m_bEnableKey = true;
	m_hPreCharLoadingThread = NULL;

	m_pLibManager = new CFcMenuLibraryManager();
}

CFcMenuManager::~CFcMenuManager()
{	
	Release();
}

void CFcMenuManager::CreateMovieRTT()
{
	if( m_hMovieRTTHandle ) {
		return;
	}
	char szFind[_MAX_PATH];
	g_BsKernel.chdir( "movies" );
	strcpy( szFind, g_BsKernel.GetCurrentDirectory() );
	strcat( szFind, _MAIN_BACKGROUND_MOVIE);
	g_BsKernel.chdir( ".." );

	CBsUiMovieTexture* pRTTexture = new CBsUiMovieTexture(szFind);
	BsAssert(g_BsKernel.GetRTTManager() != NULL);
	m_hMovieRTTHandle = g_BsKernel.GetRTTManager()->Create((CBsGenerateTexture*)pRTTexture, _RTT_SIZE, _RTT_SIZE);
	m_nMovieTextureId = g_BsKernel.GetRTTManager()->GetRTTextureID(m_hMovieRTTHandle);
}

void CFcMenuManager::Create3DObjRTT(int nTextureWidth, int nTextureHeight,
									float fStartX, float fStartY,
									float fWidth, float fHeight, bool bLight)
{
	CFcMenu3DObjManager* pRTTexture = new CFcMenu3DObjManager();
	BsAssert(g_BsKernel.GetRTTManager() != NULL);
	m_h3DObjRTTHandle = g_BsKernel.GetRTTManager()->Create((CBsGenerateTexture*)pRTTexture,
		nTextureWidth, nTextureHeight,
		fStartX, fStartY,
		fWidth, fHeight, true);

	m_pFcMenu3DObj = pRTTexture;
	if(bLight){
		m_pFcMenu3DObj->CreateLight();
	}
}

void CFcMenuManager::Release()
{
	while (!m_msgQue.empty())
	{
		FcMessageToken* pMsg = m_msgQue.front();
		BsAssert(pMsg);

		m_msgQue.pop_front();
		delete pMsg;
	}

	RemoveAllMenu();
	
	ReleaseMovieRTT();
	Release3DObjRTT();

	SAFE_DELETE(m_pMenuWorldMapInfo);
	SAFE_DELETE(m_pLibManager);
}

void CFcMenuManager::ReleaseMovieRTT()
{
 	if(m_hMovieRTTHandle == NULL)
		return;

	BsAssert(g_BsKernel.GetRTTManager() != NULL);
	g_BsKernel.GetRTTManager()->Release(m_hMovieRTTHandle);
	m_hMovieRTTHandle = NULL;
	m_nMovieTextureId = -1;
}

void CFcMenuManager::Release3DObjRTT()
{
	if(m_h3DObjRTTHandle != NULL)
	{
		BsAssert(g_BsKernel.GetRTTManager() != NULL);
		{
			// TODO : 여기 부분은 Process Thread만 여기로 진입하는것으로 알고 이렇게 처리합니다.
			//			Render Thread에서 여기로 오면 문제발생 소지가 있습니다.
//			g_BsKernel.GetRTTManager()->Release(m_h3DObjRTTHandle);
			KERNEL_COMMAND cmd;
			cmd.nCommand = KERNEL_COMMAND_DELETE_RTTOBJECT;
			cmd.nBuffer[ 0 ] = m_h3DObjRTTHandle;
			g_BsKernel.GetKernelCommand().push_back( cmd );
		}
		m_h3DObjRTTHandle = NULL;
		m_pFcMenu3DObj = NULL;
	}
}


void CFcMenuManager::Process()
{
	if(m_pMenuWorldMap != NULL){
		m_pMenuWorldMap->Process();
	}

	if(m_bEnableKey){
		OnInputKey();
	}

	FcMenuLISTITOR itr;
	FcMenuLISTITOR itr_begin = m_MenuList.begin();
	for(itr = itr_begin; itr!=m_MenuList.end();itr++)
	{
		(*itr)->Process();
	}

	g_BsUiSystem.Update();
	UpdateProcess();

	if(m_pFcMenu3DObj != NULL){
		m_pFcMenu3DObj->Process();
	}
}

void CFcMenuManager::Update()
{
	g_BsKernel.SetUIViewerIndex(_UI_FULL_VIEWER);

	FcMenuLISTITOR itr;
	FcMenuLISTITOR itr_begin = m_MenuList.begin();
	for(itr = itr_begin; itr!=m_MenuList.end();itr++)
	{
		(*itr)->RenderProcess();
	}

	UpdateSpecialEffect();
	UpdateGameInterface();

	g_BsUiSystem.Draw();
}

void CFcMenuManager::OnInputKey()
{
	if(g_BsKernel.GetClickFxEffect() != NULL)
	{
		if(m_WaveState != _MENU_WAVE_END){
			return;
		}
	}

	for(int Input = PAD_INPUT_A; Input <= PAD_INPUT_RSTICK; ++Input)
	{
		int nPressTick = CInputPad::GetInstance().GetKeyPressTick( g_FCGameData.nEnablePadID, Input );

		if(Input >= PAD_INPUT_UP && Input <= PAD_INPUT_RIGHT)
		{	
			if( ( nPressTick == 1) || ( nPressTick > 10 && nPressTick % 2 == 1 ) )
			{
				TranslateMessage(Input);
				return;
			}

			if(nPressTick >0){
				return;
			}
		}
		else if(Input == PAD_INPUT_LSTICK || Input == PAD_INPUT_RSTICK)
		{	
			if( ( nPressTick == 1) || ( nPressTick > 10 && nPressTick % 2 == 1 ) )
			{
				KEY_EVENT_PARAM* pParam = CInputPad::GetInstance().GetKeyParam(g_FCGameData.nEnablePadID, 0);
				if(pParam->nPosY > ANALOG_THRESH_HOLD)
					TranslateMessage(PAD_INPUT_UP);
				else if(pParam->nPosY < -ANALOG_THRESH_HOLD)
					TranslateMessage(PAD_INPUT_DOWN);
				else if(pParam->nPosX > ANALOG_THRESH_HOLD)
					TranslateMessage(PAD_INPUT_RIGHT);
				else if(pParam->nPosX < -ANALOG_THRESH_HOLD)
					TranslateMessage(PAD_INPUT_LEFT);
				return;
			};

			if(nPressTick >0){
				return;
			}
		}
		else
		{
			if( nPressTick == 1 )
			{
				TranslateMessage(Input);
				if(nPressTick >0){
					return;
				}
			}
		}
	}
}

void CFcMenuManager::TranslateMessage(int Input)
{
	int transInput = -1;
	switch(Input)
	{
	case PAD_INPUT_A:		transInput = MENU_INPUT_A; break;
	case PAD_INPUT_B:		transInput = MENU_INPUT_B; break;
	case PAD_INPUT_X:		transInput = MENU_INPUT_X; break;
	case PAD_INPUT_Y:		transInput = MENU_INPUT_Y; break;
	case PAD_INPUT_BLACK:	transInput = MENU_INPUT_BLACK; break;
	case PAD_INPUT_WHITE:	transInput = MENU_INPUT_WHITE; break;
	case PAD_INPUT_UP:		transInput = MENU_INPUT_UP; break;
	case PAD_INPUT_DOWN:	transInput = MENU_INPUT_DOWN; break;
	case PAD_INPUT_LEFT:	transInput = MENU_INPUT_LEFT; break;
	case PAD_INPUT_RIGHT:	transInput = MENU_INPUT_RIGHT; break;
	case PAD_INPUT_START:	transInput = MENU_INPUT_START; break;
	case PAD_INPUT_BACK:	transInput = MENU_INPUT_BACK; break;
	case PAD_INPUT_LTHUMB:	transInput = MENU_INPUT_LTHUMB; break;
	case PAD_INPUT_RTHUMB:	transInput = MENU_INPUT_RTHUMB; break;
	case PAD_INPUT_LTRIGGER:transInput = MENU_INPUT_LTRIGGER; break;
	case PAD_INPUT_RTRIGGER:transInput = MENU_INPUT_RTRIGGER; break;
	case PAD_INPUT_LSTICK:	transInput = MENU_INPUT_LSTICK; break;
	case PAD_INPUT_RSTICK:	transInput = MENU_INPUT_RSTICK; break;
	}

	g_BsUiSystem.TranslateMessage(XWMSG_KEYDOWN, 0, transInput, NULL);
}


void CFcMenuManager::PostMessage(FCMenu_Message message, DWORD param1, DWORD param2, DWORD param3, DWORD param4, DWORD param5, DWORD param6)
{
	FcMessageToken* pNewMsg = new FcMessageToken;
	BsAssert(pNewMsg);

	
	pNewMsg->message = message;
	pNewMsg->Param1 = (DWORD)param1;
	pNewMsg->Param2 = (DWORD)param2;
	pNewMsg->Param3 = (DWORD)param3;
	pNewMsg->Param4 = (DWORD)param4;
	pNewMsg->Param5 = (DWORD)param5;
	pNewMsg->Param6 = (DWORD)param6;

	if(pNewMsg->message == fcMSG_MESSAGEBOX_END)
	{
		m_msgQue.push_front(pNewMsg);
	}
	else
	{
		m_msgQue.push_back(pNewMsg);
	}
}

DWORD CFcMenuManager::PreCharLoadingThreadProc( LPVOID lpParameter )
{
	if( !m_PreLoadSkinList.empty() ) return 0;

	int nSkin;


	m_PreLoadTextureList.push_back( g_BsKernel.LoadTexture("Interface\\Local\\JPN\\HeadLine\\mn_headline_charselect_JPN.dds"));
	m_PreLoadTextureList.push_back( g_BsKernel.LoadTexture("Fx\\NonSelChar.dds"));
	m_PreLoadTextureList.push_back( g_BsKernel.LoadTexture("Fx\\NonSelChar_01.dds"));
	m_PreLoadTextureList.push_back( g_BsKernel.LoadTexture("Interface\\TGS_Loading.dds"));

	g_BsKernel.chdir("char");

	g_BsKernel.chdir("C_LP_KF");
	nSkin = g_BsKernel.LoadSkin(-1, "C_LP_KF1.SKIN");
	m_PreLoadSkinList.push_back(nSkin);
	g_BsKernel.DeleteObject( g_BsKernel.CreateStaticObjectFromSkin( nSkin ) );
	m_PreLoadAniList.push_back(g_BsKernel.LoadAni(-1, "C_LP_KF1_menu.ba"));
	g_BsKernel.chdir("..");

	g_BsKernel.chdir("C_LP_WM");
	nSkin = g_BsKernel.LoadSkin(-1, "C_LP_WM1.SKIN");
	m_PreLoadSkinList.push_back(nSkin);
	g_BsKernel.DeleteObject( g_BsKernel.CreateStaticObjectFromSkin( nSkin ));
	m_PreLoadAniList.push_back(g_BsKernel.LoadAni(-1, "C_LP_WM_menu.ba"));
	g_BsKernel.chdir("..");

	g_BsKernel.chdir("C_LP_MF");
	nSkin = g_BsKernel.LoadSkin(-1, "C_LP_MF1.SKIN");
	m_PreLoadSkinList.push_back(nSkin);
	g_BsKernel.DeleteObject( g_BsKernel.CreateStaticObjectFromSkin( nSkin ));
	m_PreLoadAniList.push_back(g_BsKernel.LoadAni(-1, "C_LP_MF_menu.ba"));
	g_BsKernel.chdir("..");

	g_BsKernel.chdir("C_LP_KM");
	nSkin = g_BsKernel.LoadSkin(-1, "C_LP_KM1.SKIN");
	m_PreLoadSkinList.push_back(nSkin);
	g_BsKernel.DeleteObject( g_BsKernel.CreateStaticObjectFromSkin( nSkin ));
	m_PreLoadAniList.push_back(g_BsKernel.LoadAni(-1, "C_LP_KM_menu.ba"));
	g_BsKernel.chdir("..");

	g_BsKernel.chdir("C_LP_PM");
	nSkin = g_BsKernel.LoadSkin(-1, "C_LP_PM1.SKIN");
	m_PreLoadSkinList.push_back(nSkin);
	g_BsKernel.DeleteObject( g_BsKernel.CreateStaticObjectFromSkin( nSkin ));
	m_PreLoadAniList.push_back(g_BsKernel.LoadAni(-1, "C_LP_PM_menu.ba"));
	g_BsKernel.chdir("..");

	g_BsKernel.chdir("C_DP_GB");
	nSkin = g_BsKernel.LoadSkin(-1, "C_DP_GB1.SKIN");
	m_PreLoadSkinList.push_back(nSkin);
	g_BsKernel.DeleteObject( g_BsKernel.CreateStaticObjectFromSkin( nSkin ));
	m_PreLoadAniList.push_back(g_BsKernel.LoadAni(-1, "C_DP_GB2_menu.ba"));
	g_BsKernel.chdir("..");

	g_BsKernel.chdir("C_DP_TR");
	nSkin = g_BsKernel.LoadSkin(-1, "C_DP_TR1.SKIN");
	m_PreLoadSkinList.push_back(nSkin);
	g_BsKernel.DeleteObject( g_BsKernel.CreateStaticObjectFromSkin( nSkin ));
	m_PreLoadAniList.push_back(g_BsKernel.LoadAni(-1, "C_DP_TR_menu.ba"));
	g_BsKernel.chdir("..");

	g_BsKernel.chdir("..");

	CAniInfoData	*pAniInfo;

	char *szAniList[] = {"AS\\C_LP_KF.info","AS\\C_LP_WM.info","AS\\C_LP_MF.info","AS\\C_LP_KM.info","AS\\C_LP_PM.info","AS\\C_DP_TR.info","AS\\C_DP_GB.info"};
	for( int k = 0; k < 7; k++) {
		pAniInfo = CAniInfoData::LoadAniInfoData(szAniList[k]);

		int nPartsGroupCount = pAniInfo->GetPartsGroupCount();
		for( int i = 0; i < nPartsGroupCount; i++ )
		{
			ASPartsData *pPartsData = pAniInfo->GetPartsGroupInfo( i );


			if( pPartsData->m_nSimulation == PHYSICS_CLOTH ) {

				int nSimulParts = pPartsData->GetPartsCount();

				for( int j = 0; j < nSimulParts; j++ ) { //aleksger: prefix bug 777: local variable hiding scope

					char szPhysicsFileName[255];
					strcpy( szPhysicsFileName, pPartsData->GetPartsSkinName( j ) );
					RemoveEXT( szPhysicsFileName );
					strcat( szPhysicsFileName, ".txt" );

					PHYSICS_DATA_CONTAINER *pContainer = CFcPhysicsLoader::LoadPhysicsData(szPhysicsFileName);		

					nSkin = g_BsKernel.LoadSkin( -1, pPartsData->GetPartsSkinName( j ) );
					m_PreLoadSkinList.push_back( nSkin );
					g_BsKernel.DeleteObject( g_BsKernel.CreateStaticObjectFromSkin( nSkin ));				
				}
			}
			else if( pPartsData->m_nSimulation == PHYSICS_COLLISION) {			
				CFcPhysicsLoader::LoadCollisionMesh( pPartsData->GetPartsSkinName( 0 ) );
			}
			else {
				nSkin = g_BsKernel.LoadSkin( -1, pPartsData->GetPartsSkinName( 0 ) );
				m_PreLoadSkinList.push_back( nSkin );
				g_BsKernel.DeleteObject( g_BsKernel.CreateStaticObjectFromSkin( nSkin ));
			}
		}
	}

	g_BsKernel.chdir("Fx");
	char szFullFileName[_MAX_PATH];
	sprintf(szFullFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), "NonSelChar.bfx" );
	m_PreLoadFXList.push_back( g_BsKernel.LoadFXTemplate(-1, szFullFileName) );
	g_BsKernel.chdir("..");

	CFcMenuManager  *pMenuManager = (CFcMenuManager  *)lpParameter;

	pMenuManager->LoadMenuScript( _FC_MENU_SEL_CHAR );

	return 1;
}

void CFcMenuManager::ReleasePreCharLoading()
{
	int i, nSize;
	
	nSize = m_PreLoadSkinList.size();
	for( i = 0; i < nSize; i++) {
        g_BsKernel.ReleaseSkin( m_PreLoadSkinList[i] );
	}
	m_PreLoadSkinList.clear();

	nSize = m_PreLoadAniList.size();
	for( i = 0; i < nSize; i++) {
		g_BsKernel.ReleaseAni( m_PreLoadAniList[i] );
	}
	m_PreLoadAniList.clear();

	nSize = m_PreLoadTextureList.size();
	for( i = 0; i < nSize; i++) {
		g_BsKernel.ReleaseTexture( m_PreLoadTextureList[i] );
	}
	m_PreLoadTextureList.clear();
	
	nSize = m_PreLoadFXList.size();
	for( i = 0; i < nSize; i++) {
        g_BsKernel.ReleaseFXTemplate( m_PreLoadFXList[i] );
	}
	m_PreLoadFXList.clear();
}

void CFcMenuManager::CreatePreCharLoadingThread()
{
}

void CFcMenuManager::WaitForPreCharLoading()
{
}
/*
#define _BACKGROUND_CHAR_LOADING
void CFcMenuManager::CreatePreCharLoadingThread()
{	
#ifdef _BACKGROUND_CHAR_LOADING
	if( m_hPreCharLoadingThread != 0 ) return;
	m_hPreCharLoadingThread = CreateThread( NULL, 65536, PreCharLoadingThreadProc, this, 0, 0 );
	XSetThreadProcessor(m_hPreCharLoadingThread, 0);
#else
	g_BsKernel.GetD3DDevice()->ReleaseThreadOwnership();
	{
		CTimeRender TimeRender( false, 20, false );
		PreCharLoadingThreadProc( this );
	}
	g_BsKernel.GetD3DDevice()->AcquireThreadOwnership();	
#endif	
}

void CFcMenuManager::WaitForPreCharLoading()
{		
#ifdef _BACKGROUND_CHAR_LOADING
	if( m_hPreCharLoadingThread == 0 ) return;
	
	g_BsKernel.GetD3DDevice()->ReleaseThreadOwnership();
	{	
		CTimeRender TimeRender( false, 20, false );
		WaitForSingleObject(m_hPreCharLoadingThread, INFINITE);	
	}
	g_BsKernel.GetD3DDevice()->AcquireThreadOwnership();

#endif
}
*/

void CFcMenuManager::UpdateProcess()
{
	m_bEnableKey = true;
	
	//Menu중에 상태가 normal, close가 아니면 Update를 하지 안는다.
	FcMenuLISTITOR itr;
	FcMenuLISTITOR itr_begin = m_MenuList.begin();
	for(itr = itr_begin; itr!=m_MenuList.end();itr++)
	{
		switch((*itr)->GetStatus())
		{
		case _MS_NORMAL:
		case _MS_CLOSE:	break;
		default:
			{
				m_bEnableKey = false;
				return;
			}
		}
	}

	CheckMenuWaveState();

	while (!m_msgQue.empty())
	{
		FcMessageToken* pMsg = m_msgQue.front();
		BsAssert(pMsg);

		if(DispatchMessage(pMsg) == 0L){
			m_msgQue.pop_front();
			delete pMsg;
		}
		else{
			break;
		}
	}
}

DWORD CFcMenuManager::DispatchMessage(FcMessageToken* pMsg)
{
	BsAssert(pMsg);
	switch(pMsg->message)
	{
	case fcMSG_GO_TO_TITLE:
	case fcMSG_GO_TO_SEL_STAGE:
	case fcMSG_MESSAGEBOX_END: break;
	default:
		{
			if(g_FC.GetMainState() == GAME_STATE_SYSTEM_MESSAGE){
				return (DWORD)pMsg;
			}

			if(m_WaveState != _MENU_WAVE_END)
			{
				m_bEnableKey = false;
				return (DWORD)pMsg;
			}

			if( UpdateSound(pMsg->message) == false )
			{
				m_bEnableKey = false;
				return (DWORD)pMsg;
			}
		}
	}

	m_bEnableKey = true;
	
	switch (pMsg->message)
	{
	//-------------------------------------------------
	//start
	case fcMSG_REMOVE_ALL_MENU:
		{
			RemoveAllMenu();
			break;
		}
	//-------------------------------------------------
	//click
	case fcMSG_CLICK_FX_EFFECT:
		{
			UpdateWave(pMsg);
			break;
		}
	//-------------------------------------------------
	// debug menu
	case fcMSG_DEBUG_MENU_START:
		{
			RemoveAllMenu();
			OpenMenu(_FC_MENU_DEBUG);
			break;
		}
	case fcMSG_DEBUG_MENU_OUT:
		{
			if(pMsg->Param1 == 0){
				g_FC.SetPause(false);
			}

			CloseMenu(_FC_MENU_DEBUG);
			break;
		}
	case fcMSG_DEBUG_MENU_TOGGLE:
		{
			if(IsSameMenu(_FC_MENU_DEBUG)){
				CloseMenu(_FC_MENU_DEBUG);
			}
			else{
				RemoveAllMenu();
				OpenMenu(_FC_MENU_DEBUG);
			}
			break;
		}

	//-------------------------------------------------
	// message box
	case fcMSG_MESSAGEBOX_START:
		{
			/*
			g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,			//message
				fcMT_LossOfController,									//param1
				NULL,													//param2
				hHandle,												//param3
				NULL,													//param4
				NULL,													//param5
				(DWORD)GAME_STATE_SYSTEM_MESSAGE);						//param6
			
			CFcMenuMessageBox* pMenu = new CFcMenuMessageBox(nType,		//type
				param1,													//nMsgType(FcMSG_ASK_TYPE)
				param2,													//Cmd Main State(GAME_STATE)
				(CFcMenuForm*)param3,									//pCmdMenu(pMenu)
				param4,													//hCmdWnd(hWnd)
				param5,													//dwFocusButton Index
				param6);												//dwValue
			*/
			
			OpenMenu(_FC_MENU_MSGBOX,
				pMsg->Param1,
				(DWORD)g_FC.GetMainState(),
				pMsg->Param2,
				pMsg->Param3,
				pMsg->Param4,
				pMsg->Param5);

			SetSystemMessage(pMsg);
			
			break;
		}
	case fcMSG_MESSAGEBOX_END:
		{	
			CFcMenuForm* pMsgMenu		= (CFcMenuForm*)pMsg->Param1;
			GAME_STATE nMainState		= (GAME_STATE)pMsg->Param2;
			CFcMenuForm* pCmdMenu		= (CFcMenuForm*)pMsg->Param3;
			FcMSG_ASK_TYPE nMsgAsk		= (FcMSG_ASK_TYPE)pMsg->Param4;
			BsUiHANDLE hWnd				= (BsUiHANDLE)pMsg->Param5;
			FcMSG_RESPONSE_TYPE nMsgRsp	= (FcMSG_RESPONSE_TYPE)pMsg->Param6;
			
			if(nMsgAsk == fcMT_CancelDeviceSelector && nMsgRsp == ID_NO) {                
				g_FCGameData.bShowDeviceUI = false;				
			}
			if(nMsgAsk == fcMT_RemoveStorage)
			{
				if(nMsgRsp == ID_YES)     {
					g_FCGameData.bShowDeviceUI = false;					
				}
				else
					g_FCGameData.bCancelDeviceSelector = true;
					//g_MenuHandle->CheckedSystemMessageBox(_SYS_MSG_CANCEL_DEVICESELECTOR, (DWORD)(false));

				if( g_pSoundManager )
					g_pSoundManager->SetPauseAll( false );					
			}

			g_FC.SetMainState(nMainState);

			if(pCmdMenu != NULL)
			{
				FcMenuLISTITOR itr;
				FcMenuLISTITOR itr_begin = m_MenuList.begin();
				for(itr = itr_begin; itr!=m_MenuList.end();itr++)
				{	
					if(pCmdMenu == (*itr))
					{
						pCmdMenu->ProcessMsg(nMsgAsk, hWnd, nMsgRsp);
						break;
					}
				}
			}
			else
			{
				BsUiWindow* pWindow = BsUi::BsUiGetWindow(hWnd);
				g_BsUiSystem.SetFocusWindow(pWindow);

				switch(g_FC.GetMainState())
				{
				case GAME_STATE_INIT_STAGE:
				case GAME_STATE_LOOP_STAGE:
				case GAME_STATE_INIT_RESULT_SIGN:
				case GAME_STATE_LOOP_RESULT_SIGN:
					{
						g_FC.SetPause(false);
						if(g_FcWorld.GetHeroHandle()){
							g_FcWorld.GetHeroHandle()->Resume();
						}
						break;
					}
				}
			}

			CloseMenu(_FC_MENU_MSGBOX, NULL, pMsgMenu);
			break;
		}

	//-------------------------------------------------
	//start
	case fcMSG_LOGO_START:
		{	
			RemoveAllMenu();

			g_pSoundManager->StopBGM();
			if(g_FCGameData.bPlayMovie)
			{	
				char szFind[_MAX_PATH];
				g_BsKernel.chdir( "movies" );
				strcpy( szFind, g_BsKernel.GetCurrentDirectory() );
				strcat( szFind, _MGS_LOGO_MOVIE);
				g_BsKernel.chdir( ".." );

				ReleaseMovieRTT();
				g_BsMoviePlayer.PlayMovie(szFind, false, &SkipMovieCallback_fcMSG_LOGO_START, true);
			}
			
			OpenMenu(_FC_MENU_LOGO);
			
			break;
		}
	case fcMSG_OPENING_START:
		{
			CloseMenu(_FC_MENU_LOGO);
			            
			g_pSoundManager->StopBGM();
			if(g_FCGameData.bPlayMovie)
			{
				m_bSetToggleOpenMovie = !m_bSetToggleOpenMovie;
				char szFind[_MAX_PATH];
				g_BsKernel.chdir( "movies" );
				strcpy( szFind, g_BsKernel.GetCurrentDirectory() );
				if(m_bSetToggleOpenMovie){
					strcat( szFind, _MAIN_OPENING_MOVIE_A);
				}
                else{
					strcat( szFind, _MAIN_OPENING_MOVIE_B);
				}
				g_BsKernel.chdir( ".." );

#ifdef _XBOX
				XMPOverrideBackgroundMusic();		// 유저 음악 플레이 못하게 함
				
				ReleaseMovieRTT();
				g_BsMoviePlayer.PlayMovie(szFind, false, &SkipMovieCallback_fcMSG_OPENING_START, true);

				XMPRestoreBackgroundMusic();		// 유저 음악 플레이 할 수 있음
#endif //_XBOX

				PostMessage(fcMSG_TITLE_START, _FC_MENU_OPENING);

				// g_FC.CreateLiveManager();
			}
			else
			{
				OpenMenu(_FC_MENU_OPENING);
			}
			break;
		}
	case fcMSG_TITLE_START:
		{
			g_FC.CreateLiveManager();

			_FC_MENU_TYPE nPreMenuType = (_FC_MENU_TYPE)(pMsg->Param1);
			if(pMsg->Param1 == 0)
			{
				CloseMenu(_FC_MENU_OPENING, &nPreMenuType);
				CloseMenu(_FC_MENU_SEL_CHAR, &nPreMenuType);
				CloseMenu(_FC_MENU_LOADDATA, &nPreMenuType);
				CloseMenu(_FC_MENU_LIBRARY, &nPreMenuType);
				CloseMenu(_FC_MENU_OPTION, &nPreMenuType);
                
				if(g_FCGameData.bChangeProfile == false && 
					g_FCGameData.SelStageId == STAGE_ID_TUTORIAL)
				{
					nPreMenuType = _FC_MENU_TUTORIAL;
					g_FCGameData.SelStageId = STAGE_ID_NONE;
				}
				else if(g_FCGameData.bChangeProfile == true &&
					nPreMenuType == 0)
				{
					nPreMenuType = _FC_MENU_TITLE;
					g_FCGameData.bChangeProfile = false;
				}
			}

			RemoveAllMenu();
			
			m_pMenuWorldMapInfo->Release();

			g_FCGameData.SelStageId = STAGE_ID_NONE;
			g_FCGameData.PrevStageId = STAGE_ID_NONE;
			g_FCGameData.SpeaialSelStageId = STAGE_ID_NONE;
			g_FCGameData.GuardianInfo[0].clear();
			g_FCGameData.GuardianInfo[1].clear();
			memset(g_FCGameData.nTrgGlobalVar,-1,sizeof(int) * MAX_TRG_VAR_NUM);

			OpenMenu(_FC_MENU_TITLE, nPreMenuType);

#ifdef _XBOX
			g_FC.SetLiveContext( LIVE_CONTEXT_MENU , LIVE_CONST_TITLE );	// RichPresence 에 알림. 캐릭터 선택중.
#endif
			
			CreatePreCharLoadingThread();

			break;
		}
	//-------------------------------------------------
	// title
	case fcMSG_SEL_CHAR_START:
		{
			GAME_MISSION_TYPE nGameType = MISSION_TYPE_NONE;
			if(CloseMenu(_FC_MENU_TITLE)){
				nGameType = MISSION_TYPE_NONE;
			}							
			WaitForPreCharLoading();
			OpenMenu(_FC_MENU_SEL_CHAR, nGameType);

#ifdef _XBOX
			g_FC.SetLiveContext( LIVE_CONTEXT_MENU , LIVE_CONST_CHAR );	// RichPresence 에 알림. 캐릭터 선택중.
#endif
			break;
		}
	case fcMSG_LOADDATA_START:
		{	
			WaitForPreCharLoading();
			CloseMenu(_FC_MENU_TITLE);
			OpenMenu(_FC_MENU_LOADDATA, _FC_MENU_TITLE);
			break;
		}
	case fcMSG_LIBRARY_START:
		{
			WaitForPreCharLoading();
			_FC_MENU_TYPE nPreMenuType = _FC_MENU_NONE;
			CloseMenu(_FC_MENU_TITLE, &nPreMenuType);
			CloseMenu(_FC_MENU_LIB_CHAR, &nPreMenuType);
			CloseMenu(_FC_MENU_LIB_MISSION, &nPreMenuType);
			CloseMenu(_FC_MENU_LIB_ART, &nPreMenuType);
			
			if(nPreMenuType == _FC_MENU_TITLE){
				m_pLibManager->Update();
			}
			OpenMenu(_FC_MENU_LIBRARY, nPreMenuType);

#ifdef _XBOX
			g_FC.SetLiveContext( LIVE_CONTEXT_MENU , LIVE_CONST_LIBRARY );	// RichPresence 에 알림..
#endif
			break;
		}
	case fcMSG_OPTION_START:
		{
			WaitForPreCharLoading();
			_FC_MENU_TYPE nPreMenuType = _FC_MENU_NONE;
			CloseMenu(_FC_MENU_TITLE, &nPreMenuType);
			CloseMenu(_FC_MENU_PAUSE, &nPreMenuType);

			OpenMenu(_FC_MENU_OPTION, nPreMenuType);

#ifdef _XBOX
			g_FC.SetLiveContext( LIVE_CONTEXT_MENU , LIVE_CONST_OPTION );	// RichPresence 에 알림..
#endif
			break;
		}
	case fcMSG_CREDITS_START:
		{
			WaitForPreCharLoading();
			char szFind[_MAX_PATH];
			g_BsKernel.chdir( "movies" );
			strcpy( szFind, g_BsKernel.GetCurrentDirectory() );
			strcat( szFind, _STAFF_ROLL_MOVIE);
			g_BsKernel.chdir( ".." );

#ifdef _XBOX
			XMPOverrideBackgroundMusic();		// 유저 음악 플레이 못하게 함

			ReleaseMovieRTT();
			g_BsMoviePlayer.PlayMovie(szFind, false, &SkipMovieCallback_fcMSG_CREDITS_START, true);

			XMPRestoreBackgroundMusic();		// 유저 음악 플레이 할 수 있음
#endif //_XBOX

			PostMessage(fcMSG_TITLE_START, _FC_MENU_CREDITS);
			break;
		}
	case fcMSG_TUTORIAL_START:
		{
			WaitForPreCharLoading();
			g_FCGameData.PrevStageId = STAGE_ID_NONE;
			g_FCGameData.SelStageId = STAGE_ID_TUTORIAL;
			strcpy(g_FCGameData.cMapFileName, _STAGE_TUTORIAL_FILENAME);
			g_MenuHandle->PostMessage(fcMSG_MISSION_START);
			break;
		}

	//-------------------------------------------------
	// mission
	case fcMSG_SELECT_STAGE_START:
		{
			g_FCGameData.GuardianInfo[0].clear();
			g_FCGameData.GuardianInfo[1].clear();

			CloseMenu(_FC_MENU_SEL_CHAR);
			if(CloseMenu(_FC_MENU_GUARDIAN)){
				g_FCGameData.SelStageId = g_FCGameData.PrevStageId;
			}
			if(CloseMenu(_FC_MENU_GAME_FAILED)){
				g_FCGameData.SelStageId = g_FCGameData.PrevStageId;
			}

			m_pMenuWorldMapInfo->Load();
			m_pMenuWorldMap = new FcMenuWorldMap();

			OpenMenu(_FC_MENU_SELECT_STAGE);
			break;
		}
	case fcMSG_SELECT_STAGE_TO_MISSION_START:
		{
			CloseMenu(_FC_MENU_SELECT_STAGE);

			if(m_pMenuWorldMap != NULL)
			{
				g_FCGameData.PrevStageId = g_FCGameData.SelStageId;
				g_FCGameData.SelStageId = (GAME_STAGE_ID)m_pMenuWorldMap->GetStageId();
				strcpy(g_FCGameData.cMapFileName, m_pMenuWorldMap->GetMapFileName());
			}

			SetGuardianforFirstMission();

			if(m_pMenuWorldMap != NULL)
			{
				delete m_pMenuWorldMap;
				m_pMenuWorldMap = NULL;
			}

			g_MenuHandle->PostMessage(fcMSG_MISSION_START);
			break;
		}
	case fcMSG_USUAL_START:
		{
			BsAssert(m_pMenuWorldMap != NULL);
			m_pMenuWorldMap->SetEnable(false);
			OpenMenu(_FC_MENU_USUAL, _FC_MENU_NONE);
			break;
		}
	case fcMSG_USUAL_END:
		{
			BsAssert(m_pMenuWorldMap != NULL);
			m_pMenuWorldMap->SetEnable(true);
			CloseMenu(_FC_MENU_USUAL);
			break;
		}
	case fcMSG_USUAL_TO_LOAD_START:
		{
			BsAssert(m_pMenuWorldMap != NULL);
			m_pMenuWorldMap->SetShowOn(false);

			CFcMenuForm* pMenu = GetMenu(_FC_MENU_SELECT_STAGE);
			BsAssert(pMenu != NULL);
			pMenu->SetShowOn(false);

			pMenu = GetMenu(_FC_MENU_USUAL);
			BsAssert(pMenu != NULL);
			pMenu->SetShowOn(false);

			OpenMenu(_FC_MENU_LOADDATA, _FC_MENU_USUAL);
			break;
		}
	case fcMSG_USUAL_TO_SAVEDATA_START:
		{
			BsAssert(m_pMenuWorldMap != NULL);
			m_pMenuWorldMap->SetShowOn(false);

			CFcMenuForm* pMenu = GetMenu(_FC_MENU_SELECT_STAGE);
			BsAssert(pMenu != NULL);
			pMenu->SetShowOn(false);

			pMenu = GetMenu(_FC_MENU_USUAL);
			BsAssert(pMenu != NULL);
			pMenu->SetShowOn(false);

			OpenMenu(_FC_MENU_SAVEDATA, _FC_MENU_USUAL);
			break;
		}
	case fcMSG_USUAL_TO_OPTION_START:
		{
			BsAssert(m_pMenuWorldMap != NULL);
			m_pMenuWorldMap->SetShowOn(false);

			CFcMenuForm* pMenu = GetMenu(_FC_MENU_SELECT_STAGE);
			BsAssert(pMenu != NULL);
			pMenu->SetShowOn(false);
            
			pMenu = GetMenu(_FC_MENU_USUAL);
			BsAssert(pMenu != NULL);
			pMenu->SetShowOn(false);

			OpenMenu(_FC_MENU_OPTION, _FC_MENU_USUAL);
			break;
		}
	case fcMSG_SUB_TO_USUAL:
		{
			_FC_MENU_TYPE nPreMenuType = _FC_MENU_NONE;
			CloseMenu(_FC_MENU_SAVEDATA, &nPreMenuType, NULL, false);
			CloseMenu(_FC_MENU_LOADDATA, &nPreMenuType, NULL, false);
			CloseMenu(_FC_MENU_OPTION, &nPreMenuType, NULL, false);

			BsAssert(m_pMenuWorldMap != NULL);
			m_pMenuWorldMap->SetShowOn(true);

			CFcMenuForm* pMenu = GetMenu(_FC_MENU_SELECT_STAGE);
			BsAssert(pMenu != NULL);
			pMenu->SetShowOn(true);

			pMenu = GetMenu(_FC_MENU_USUAL);
			BsAssert(pMenu != NULL);
			pMenu->SetShowOn(true);

			((CFcMenuUsual*)pMenu)->Reinit();
			pMenu->SetFocus(nPreMenuType);
			break;
		}
	case fcMSG_GUARDIAN_START:
		{
			CloseMenu(_FC_MENU_SELECT_STAGE);

			if(m_pMenuWorldMap != NULL)
			{
				g_FCGameData.PrevStageId = g_FCGameData.SelStageId;
				g_FCGameData.SelStageId = (GAME_STAGE_ID)m_pMenuWorldMap->GetStageId();
				strcpy(g_FCGameData.cMapFileName, m_pMenuWorldMap->GetMapFileName());

				delete m_pMenuWorldMap;
				m_pMenuWorldMap = NULL;
			}

			OpenMenu(_FC_MENU_GUARDIAN);
			break;
		}
	case fcMSG_FIRST_MISSION_START:
		{
			RemoveAllMenu();

			g_pSoundManager->StopBGM();		// 동영상에서는 Fade Out처리가 안된다.
//			g_pSoundManager->StopBGM( true );
//			if( g_pSoundManager->IsBGMPlay() )
//				return (DWORD)pMsg;

			if(g_FCGameData.bPlayMovie && g_FCGameData.nPlayerType != MISSION_TYPE_KLARRANN)
			{	
				char szFind[_MAX_PATH];
				g_BsKernel.chdir( "movies" );
				strcpy( szFind, g_BsKernel.GetCurrentDirectory() );
				g_BsKernel.chdir( ".." );

				switch(g_FCGameData.nPlayerType)
				{
				case MISSION_TYPE_INPHYY:	strcat( szFind, _OPENING_MOVIE_INPHYY); break;
				case MISSION_TYPE_ASPHARR:	strcat( szFind, _OPENING_MOVIE_ASPHARR); break;
				case MISSION_TYPE_MYIFEE:	strcat( szFind, _OPENING_MOVIE_MYIFEE); break;
				case MISSION_TYPE_TYURRU:	strcat( szFind, _OPENING_MOVIE_TYURRU); break;
				case MISSION_TYPE_DWINGVATT:strcat( szFind, _OPENING_MOVIE_DWINGVATT); break;
				case MISSION_TYPE_VIGKVAGK:	strcat( szFind, _OPENING_MOVIE_VIGKVAGK); break;
				default: BsAssert(0);
				}

#ifdef _XBOX
				XMPOverrideBackgroundMusic();		// 유저 음악 플레이 못하게 함

				ReleaseMovieRTT();
				g_BsMoviePlayer.PlayMovie(szFind, false, &SkipMovieCallback_fcMSG_FIRST_MISSION_START, true);

				XMPRestoreBackgroundMusic();		// 유저 음악 플레이 할 수 있음
#endif //_XBOX
			}

			SetGuardianforFirstMission();

			PostMessage(fcMSG_MISSION_START);

			break;
		}
	case fcMSG_MISSION_START:
		{
			g_pSoundManager->StopBGM();
			RemoveAllMenu();

			g_FC.SetMainState( GAME_STATE_INIT_STAGE, NULL);

			break;
		}	
	
	//-------------------------------------------------
	// library
	case fcMSG_LIB_CHAR_START:
		{	
			CloseMenu(_FC_MENU_LIBRARY);
			CloseMenu(_FC_MENU_LIB_CHAR_SUB);

			DWORD dwMissionType = pMsg->Param1;
			OpenMenu(_FC_MENU_LIB_CHAR, dwMissionType);
			break;
		}
	case fcMSG_LIB_CHAR_SUB_START:
		{
			CloseMenu(_FC_MENU_LIB_CHAR);
			OpenMenu(_FC_MENU_LIB_CHAR_SUB, pMsg->Param1);
			break;
		}
	case fcMSG_LIB_MISSION_START:
		{
			DWORD dwItemID = LIB_MAX_COUNT * LIBRARY_SECTION_GAP;

			CloseMenu(_FC_MENU_LIBRARY);
			if(CloseMenu(_FC_MENU_LIB_MISSION_SUB)){
				dwItemID = pMsg->Param1;
			}
			
			OpenMenu(_FC_MENU_LIB_MISSION, dwItemID);
			break;
		}
	case fcMSG_LIB_MISSION_SUB_START:
		{
			CloseMenu(_FC_MENU_LIB_MISSION);

			DWORD dwItemID = pMsg->Param1;

			OpenMenu(_FC_MENU_LIB_MISSION_SUB, (DWORD)dwItemID);
			break;
		}
	case fcMSG_LIB_ART_START:
		{	
			CloseMenu(_FC_MENU_LIBRARY);
			OpenMenu(_FC_MENU_LIB_ART);
			break;
		}
	
	//-------------------------------------------------
	// mission end
	//success
	case fcMSG_GAME_CLEAR_START:
		{
			CInputPad::GetInstance().Enable(false);
			CInputPad::GetInstance().ResetKeyBuffer();
			CFcWorld::GetInstance().GetHeroHandle( 0 )->Resume();
			CameraObjHandle CamHandle = CFcBaseObject::GetCameraObjectHandle(0);
			if(CamHandle){
				CamHandle->Enable(false);
			}

			CloseMenu(_FC_MENU_PAUSE);
			OpenMenu(_FC_MENU_GAME_CLEAR);
			break;
		}
	case fcMSG_SUCCESS_REAL_MOVIE_START:
		{
			RemoveAllMenu();
			break;
		}
	case fcMSG_STAGE_RESULT_START:
		{
			CloseMenu(_FC_MENU_GAME_CLEAR);
			OpenMenu(_FC_MENU_STAGE_RESULT);
			break;
		}
	case fcMSG_ENDING_CHAR_START:
		{
			g_FCGameData.GuardianInfo[0].clear();
			g_FCGameData.GuardianInfo[1].clear();

			CloseMenu(_FC_MENU_SAVEDATA);
			RemoveAllMenu();

			if(m_pMenuWorldMap != NULL)
			{
				delete m_pMenuWorldMap;
				m_pMenuWorldMap = NULL;
			}

			switch(g_FCGameData.nPlayerType)
			{
			case MISSION_TYPE_ASPHARR:
			case MISSION_TYPE_TYURRU:
			case MISSION_TYPE_KLARRANN:
				{
					g_MenuHandle->PostMessage(fcMSG_END_START);
					break;
				}
			case MISSION_TYPE_MYIFEE:
				{
					g_FCGameData.PrevStageId = g_FCGameData.SelStageId;
					g_FCGameData.SelStageId = STAGE_ID_ENDING_MYIFEE;
					strcpy(g_FCGameData.cMapFileName, MISSION_CLEAR_MAP_MYIFEE);
					g_MenuHandle->PostMessage(fcMSG_MISSION_START);
					break;
				}
			case MISSION_TYPE_DWINGVATT:
				{	
					g_FCGameData.PrevStageId = g_FCGameData.SelStageId;
					g_FCGameData.SelStageId = STAGE_ID_ENDING_DWINGVATT;
					strcpy(g_FCGameData.cMapFileName, MISSION_CLEAR_MAP_DWINGVATT);
					g_MenuHandle->PostMessage(fcMSG_MISSION_START);
					break;
				}
			case MISSION_TYPE_VIGKVAGK:
				{	
					g_FCGameData.PrevStageId = g_FCGameData.SelStageId;
					g_FCGameData.SelStageId = STAGE_ID_ENDING_VIGKVAGK;
					strcpy(g_FCGameData.cMapFileName, MISSION_CLEAR_MAP_VIGKVAGK);
					g_MenuHandle->PostMessage(fcMSG_MISSION_START);
					break;
				}
			case MISSION_TYPE_INPHYY:
				{
					if(g_FCGameData.SelStageId == STAGE_ID_ANOTHER_WORLD)
					{	
						g_FCGameData.PrevStageId = g_FCGameData.SelStageId;
						g_FCGameData.SelStageId = STAGE_ID_ENDING_MA_INPHYY;
						strcpy(g_FCGameData.cMapFileName, MISSION_CLEAR_MAP_MA_INPHYY);
						g_MenuHandle->PostMessage(fcMSG_MISSION_START);
					}
					else{
						g_MenuHandle->PostMessage(fcMSG_END_START);
					}
					break;
				}
			}
			
			break;
		}
	case fcMSG_END_START:
		{
			CloseMenu(_FC_MENU_SAVEDATA);
			RemoveAllMenu();
			
			char szFind[_MAX_PATH];
			g_BsKernel.chdir( "movies" );
			strcpy( szFind, g_BsKernel.GetCurrentDirectory() );
			strcat( szFind, _STAFF_ROLL_MOVIE);
			g_BsKernel.chdir( ".." );

#ifdef _XBOX
			XMPOverrideBackgroundMusic();		// 유저 음악 플레이 못하게 함

			ReleaseMovieRTT();
			g_BsMoviePlayer.PlayMovie(szFind, false, &SkipMovieCallback_fcMSG_END_START, true);

			XMPRestoreBackgroundMusic();		// 유저 음악 플레이 할 수 있음
#endif //_XBOX

			g_MenuHandle->PostMessage(fcMSG_GO_TO_TITLE);

			//OpenMenu(_FC_MENU_END);
			break;
		}
	case fcMSG_SAVEDATA_START:
		{
			RemoveAllMenu();
			OpenMenu(_FC_MENU_SAVEDATA, pMsg->Param1, pMsg->Param2);
			break;
		}
	//failed
	case fcMSG_GAME_OVER_START:
		{
			CInputPad::GetInstance().Enable(false);
			CameraObjHandle CamHandle = CFcBaseObject::GetCameraObjectHandle(0);
			if(CamHandle){
				CamHandle->Enable(false);
			}

			CloseMenu(_FC_MENU_PAUSE);
			OpenMenu(_FC_MENU_GAME_OVER);
			break;
		}
	case fcMSG_GAME_FAILED_START:
		{
			CloseMenu(_FC_MENU_GAME_OVER);
			OpenMenu(_FC_MENU_GAME_FAILED);
			break;
		}
	//-------------------------------------------------
	// pause
	case fcMSG_PAUSE_START:
		{
			if(g_MenuHandle->GetGamePauseMenu() == _FC_MENU_NONE)
			{
				g_FC.SetPause(true);
				g_FC.SaveLiveContext();
				SetGamePauseMenu(_FC_MENU_PAUSE);
			}

			_FC_MENU_TYPE nPreMenuType = _FC_MENU_NONE;
			CloseMenu(_FC_MENU_STATUS, &nPreMenuType);
			CloseMenu(_FC_MENU_ABILITY, &nPreMenuType);
			CloseMenu(_FC_MENU_CONTROL, &nPreMenuType);
			CloseMenu(_FC_MENU_MISSION_OBJ, &nPreMenuType);
			CloseMenu(_FC_MENU_OPTION, &nPreMenuType);

			OpenMenu(_FC_MENU_PAUSE, nPreMenuType);

#ifdef _XBOX
			g_FC.SetLiveContext(  LIVE_CONTEXT_MENU, LIVE_CONST_PAUSED );	// RichPresence 에 알림..
#endif
			break;
		}
	case fcMSG_PAUSE_FOR_XBOXGUIDE_START:
		{
			SetGamePauseMenu(_FC_MENU_PAUSE);
			OpenMenu(_FC_MENU_PAUSE, _FC_MENU_NONE);
			break;
		}
	case fcMSG_PAUSE_END:
		{
			BsAssert(g_MenuHandle->GetGamePauseMenu() == _FC_MENU_PAUSE);

			g_FC.SetPause(false);
			g_FC.RestoreLiveContext();
			SetGamePauseMenu(_FC_MENU_NONE);

			CloseMenu(_FC_MENU_PAUSE);
			SetGamePauseMenu(_FC_MENU_NONE);

			g_FcWorld.GetHeroHandle()->Resume();
			break;
		}
	case fcMSG_STATUS_START:
		{
			if(g_MenuHandle->GetGamePauseMenu() == _FC_MENU_NONE)
			{
				g_FC.SetPause(true);
				SetGamePauseMenu(_FC_MENU_STATUS);
			}

			CloseMenu(_FC_MENU_PAUSE);
			CloseMenu(_FC_MENU_ABILITY);

			//m_nGamePauseMenuType에 따라 end command가 달라진다.
			OpenMenu(_FC_MENU_STATUS);
			break;
		}
	case fcMSG_STATUS_END:
		{	
			if(g_MenuHandle->GetGamePauseMenu() == _FC_MENU_STATUS)
			{
				g_FC.SetPause(false);
				SetGamePauseMenu(_FC_MENU_NONE);

#ifdef _XBOX
				if(g_FC.GetLiveManager() != NULL && g_FC.GetLiveManager()->IsUIActive()){
					PostMessage(fcMSG_PAUSE_START);
				}
#endif
			}

			CloseMenu(_FC_MENU_STATUS);
			SetGamePauseMenu(_FC_MENU_NONE);
            
			g_FcWorld.GetHeroHandle()->Resume();
			break;
		}
	case fcMSG_ABILITY_START:
		{
			_FC_MENU_TYPE nPreMenuType = _FC_MENU_NONE;
			CloseMenu(_FC_MENU_PAUSE, &nPreMenuType);
			CloseMenu(_FC_MENU_STATUS, &nPreMenuType);

			OpenMenu(_FC_MENU_ABILITY, nPreMenuType);
			break;
		}
	case fcMSG_CONTROL_START:
		{	
			CloseMenu(_FC_MENU_PAUSE);
			OpenMenu(_FC_MENU_CONTROL);
			break;
		}
	case fcMSG_MISSION_OBJ_START:
		{	
			CloseMenu(_FC_MENU_PAUSE);
			OpenMenu(_FC_MENU_MISSION_OBJ);
			break;
		}
	//---------------------------------------------------
	// main state가 바뀌는 경우.
	case fcMSG_GO_TO_LOGO:
		{
			g_FC.SetPause(false);
			RemoveAllMenu();

			SetGamePauseMenu(_FC_MENU_NONE);

			//강제로 게임을 미션을 종료하고 title로 이동한다.
			g_FC.SetMainState( GAME_STATE_INIT_TITLE, 1, 1 );
			break;
		}
	case fcMSG_GO_TO_TITLE:
		{
			g_FC.SetPause(false);
			RemoveAllMenu();

			SetGamePauseMenu(_FC_MENU_NONE);

			//강제로 게임을 미션을 종료하고 title로 이동한다.
			g_FC.SetMainState( GAME_STATE_INIT_TITLE, 1 );
			if( g_pSoundManager ){ 
				g_pSoundManager->SaveVolCtgy();
				g_pSoundManager->SetVolumeAll( 0.f ); 				
				g_pSoundManager->EnableAutoRestoreVolCtgy( true );
			}

			break;
		}
	case fcMSG_GO_TO_SEL_STAGE:
		{
			g_FC.SetPause(false);
			RemoveAllMenu();

			SetGamePauseMenu(_FC_MENU_NONE);

			//강제로 게임을 미션을 종료하고 title로 이동한다.
			g_FC.SetMainState( GAME_STATE_INIT_TITLE );
			break;
		}
	case fcMSG_GO_TO_LOAD_SELECT_STAGE:
		{
			CloseMenu(_FC_MENU_LOADDATA);
			CloseMenu(_FC_MENU_USUAL);
			CloseMenu(_FC_MENU_SELECT_STAGE);

			if(m_pMenuWorldMap != NULL)
			{	
				delete m_pMenuWorldMap;
				m_pMenuWorldMap = NULL;
			}

			m_pMenuWorldMapInfo->Load();
			m_pMenuWorldMap = new FcMenuWorldMap();

			OpenMenu(_FC_MENU_SELECT_STAGE);
			break;
		}
		//---------------------------------------------------
		// 
	case fcMSG_SPECIAL_SELSTAGE_START:
		{
			g_FC.SetPause(true);
			OpenMenu(_FC_MENU_SPECIAL_SELSTAGE);
			break;
		}
	case fcMSG_SPECIAL_SELSTAGE_END:
		{
			g_FC.SetPause(false);
			CloseMenu(_FC_MENU_SPECIAL_SELSTAGE);
			g_FCGameData.nSpecialSelStageState = SPECIAL_STAGE_MENU_OFF;
			break;
		}
	}

	return 0L;
}

bool CFcMenuManager::OpenMenu(_FC_MENU_TYPE nType, DWORD param1, DWORD param2, DWORD param3, DWORD param4, DWORD param5, DWORD param6)
{
	switch(nType)
	{
	case _FC_MENU_MSGBOX: break;
	default:
		{
			if(IsSameMenu(nType) == true)
				return false;
		}
	}
	
	LoadMenuScript(nType);

#ifdef _XBOX
	if(m_hMovieRTTHandle == NULL)
	{
		switch(g_FCGameData.State)
		{
		case GAME_STATE_INIT_STAGE:
		case GAME_STATE_SYSTEM_MESSAGE: break;
		default:
			{
				switch(nType)
				{
				case _FC_MENU_NONE:
				case _FC_MENU_DEBUG:
				case _FC_MENU_MSGBOX:
				//misc
				case _FC_MENU_LOGO:
				case _FC_MENU_OPENING:
				//game pause
				case _FC_MENU_PAUSE:
				case _FC_MENU_STATUS:
				case _FC_MENU_ABILITY:
				case _FC_MENU_CONTROL:
				case _FC_MENU_MISSION_OBJ:
				case _FC_MENU_SPECIAL_SELSTAGE:
				//game end
				case _FC_MENU_GAME_CLEAR:
				case _FC_MENU_GAME_OVER:
				case _FC_MENU_STAGE_RESULT:
				case _FC_MENU_GAME_FAILED:
				//usual
				case _FC_MENU_USUAL: break;

				//..
				case _FC_MENU_LOADDATA:
					{
						if((_FC_MENU_TYPE)param1 == _FC_MENU_TITLE){
							CreateMovieRTT();
						}
						break;
					}
				case _FC_MENU_OPTION:
					{
						if((_FC_MENU_TYPE)param1 == _FC_MENU_TITLE){
							CreateMovieRTT();
						}
						break;
					}
				case _FC_MENU_SAVEDATA:
					{
						if((_FC_MENU_TYPE)param1 != _FC_MENU_USUAL){
							CreateMovieRTT();
						}
						break;
					}
				default:{
						CreateMovieRTT();
					}
				}
			}
		}
	}

#endif

	switch(nType)
	{
	case _FC_MENU_DEBUG:
		{
#ifndef _LTCG
			CFcMenuDebug* pMenu = new CFcMenuDebug(nType);
			m_MenuList.push_back(pMenu);
#endif
			break;
		}
	case _FC_MENU_MSGBOX:
		{
			CFcMenuMessageBox* pMenu = new CFcMenuMessageBox(nType,		//type
				param1,													//nMsgType
				param2,													//Cmd Main State
				(CFcMenuForm*)param3,									//pCmdMenu
				param4,													//hCmdWnd
				param5,													//dwFocusButton Index
				param6);												//dwValue

			m_MenuList.push_back(pMenu);
			break;
		}

	//-------------------------------------------------
	// start
	case _FC_MENU_LOGO:
		{
			CFcMenuLogo* pMenu = new CFcMenuLogo(nType);
			m_MenuList.push_back(pMenu);
			break;
		}
	case _FC_MENU_OPENING:
		{
			CFcMenuMainOpening* pMenu = new CFcMenuMainOpening(nType);
			m_MenuList.push_back(pMenu);
			break;
		}
	case _FC_MENU_TITLE:
		{	
			if( g_hPackedResTitle == NULL )
				g_hPackedResTitle = CBsFileIO::BsMountCompressedPackFile("d:\\data\\PackedResources\\Res_Title.pak");
			if( g_hPackedTexTitle == NULL )
				g_hPackedTexTitle = CBsFileIO::BsMountCompressedPackFile("d:\\data\\PackedResources\\Tex_Title.pak", true);

			g_BsResChecker.AddIgnoreFileList( "Cubemap.36t" );
			g_BsResChecker.AddIgnoreFileList( "Cubemap1.36t" );

			g_BsResChecker.SetGroup( RES_GROUP_TITLE );
			CFcMenuTitle* pMenu = new CFcMenuTitle(nType, (_FC_MENU_TYPE)param1);
			m_MenuList.push_back(pMenu);
			g_BsResChecker.PopGroup();

			g_BsResChecker.ClearIgnoreFileList();
			break;
		}

	//-------------------------------------------------
	//title
	case _FC_MENU_SEL_CHAR:
		{
			g_BsResChecker.AddIgnoreFileList( "Cubemap.36t" );
			g_BsResChecker.AddIgnoreFileList( "Cubemap1.36t" );

			g_BsResChecker.SetGroup( RES_GROUP_TITLE );
			Create3DObjRTT(1480, 840, 0.f, 0.f, 1.f, 1.f, true);
			CFcMenuSelChar* pMenu = new CFcMenuSelChar(nType);			
			pMenu->SetFocus(param1);
			m_MenuList.push_back(pMenu);
			g_BsResChecker.PopGroup();

			g_BsResChecker.ClearIgnoreFileList();
			break;
		}
	case _FC_MENU_LOADDATA:
		{
			CFcMenuLoadData* pMenu = new CFcMenuLoadData(nType, (_FC_MENU_TYPE)param1);
			m_MenuList.push_back(pMenu);
			break;
		}
	case _FC_MENU_LIBRARY:
		{
			CFcMenuLibrary* pMenu = new CFcMenuLibrary(nType, (_FC_MENU_TYPE)param1);
			m_MenuList.push_back(pMenu);
			break;
		}
	case _FC_MENU_OPTION:
		{	
			CFcMenuOption* pMenu = new CFcMenuOption(nType, (_FC_MENU_TYPE)param1);
			m_MenuList.push_back(pMenu);
			break;
		}

	//-------------------------------------------------
	// mission
	case _FC_MENU_SELECT_STAGE:
		{
			CFcMenuSelectStage* pMenu = new CFcMenuSelectStage(nType);
			m_MenuList.push_back(pMenu);
			break;
		}
	case _FC_MENU_USUAL:
		{
			CFcMenuUsual* pMenu = new CFcMenuUsual(nType);
			pMenu->SetFocus((_FC_MENU_TYPE)param1);
			m_MenuList.push_back(pMenu);
			break;
		}
	case _FC_MENU_SAVEDATA:
		{
			CFcMenuSaveData* pMenu = new CFcMenuSaveData(nType, (_FC_MENU_TYPE)param1, param2);
			m_MenuList.push_back(pMenu);
			break;
		}
	case _FC_MENU_GUARDIAN:
		{
			CFcMenuGuardian* pMenu = new CFcMenuGuardian(nType);
			m_MenuList.push_back(pMenu);
			break;
		}

	//-------------------------------------------------
	// library
	case _FC_MENU_LIB_CHAR:
		{
			CFcMenuCharStock* pMenu = new CFcMenuCharStock(nType, param1);
			m_MenuList.push_back(pMenu);
			break;
		}
	case _FC_MENU_LIB_CHAR_SUB:
		{
			CFcMenuCharStockSub* pMenu = new CFcMenuCharStockSub(nType, param1);
			m_MenuList.push_back(pMenu);
			break;
		}
	case _FC_MENU_LIB_MISSION:
		{
			//param1 : LibraryManager ItemID
			CFcMenuMissionDB* pMenu = new CFcMenuMissionDB(nType, param1);
			m_MenuList.push_back(pMenu);
			break;
		}
	case _FC_MENU_LIB_MISSION_SUB:
		{
			//param1 : LibraryManager ItemID
			CFcMenuMissionDBSub* pMenu = new CFcMenuMissionDBSub(nType, param1);
			m_MenuList.push_back(pMenu);
			break;
		}
	case _FC_MENU_LIB_ART:
		{	
			CFcMenuArtWork* pMenu = new CFcMenuArtWork(nType);
			m_MenuList.push_back(pMenu);
			break;
		}

	//-------------------------------------------------
	// mission end
	case _FC_MENU_GAME_CLEAR:
	case _FC_MENU_GAME_OVER:
		{
			CFcMenuGameResult* pMenu = new CFcMenuGameResult(nType);
			m_MenuList.push_back(pMenu);
			break;
		}
	case _FC_MENU_STAGE_RESULT:
		{
			CFcMenuStageResult* pMenu = new CFcMenuStageResult(nType);
			m_MenuList.push_back(pMenu);
			break;
		}
	case _FC_MENU_GAME_FAILED:
		{
			CFcMenuFailed* pMenu = new CFcMenuFailed(nType);
			m_MenuList.push_back(pMenu);
			break;
		}
	//-------------------------------------------------
	// pause
	case _FC_MENU_PAUSE:
		{
			CFcMenuPause* pMenu = new CFcMenuPause(nType);
			pMenu->SetFocus(param1);
			m_MenuList.push_back(pMenu);
			break;
		}
	case _FC_MENU_STATUS:
		{	
			Create3DObjRTT(_GAME_STATE_ABILITY_X, _GAME_STATE_ABILITY_Y, 0.f, 0.f, 1.f, 1.f, false);
			CFcMenuStatus* pMenu = new CFcMenuStatus(nType);
			m_MenuList.push_back(pMenu);
			break;
		}
	case _FC_MENU_ABILITY:
		{	
			Create3DObjRTT(_GAME_STATE_ABILITY_X, _GAME_STATE_ABILITY_Y, 0.f, 0.f, 1.f, 1.f, false);
			CFcMenuAbility* pMenu = new CFcMenuAbility(nType, (_FC_MENU_TYPE)param1);
			m_MenuList.push_back(pMenu);
			break;
		}
	case _FC_MENU_CONTROL:
		{	
			CFcMenuControl* pMenu = new CFcMenuControl(nType);
			m_MenuList.push_back(pMenu);
			break;
		}
	case _FC_MENU_MISSION_OBJ:
		{
			CFcMenuMissionObj* pMenu = new CFcMenuMissionObj(nType);
			m_MenuList.push_back(pMenu);
			break;
		}
	case _FC_MENU_SPECIAL_SELSTAGE:
		{
			CFcMenuSpecialSelStage* pMenu = new CFcMenuSpecialSelStage(nType);
			m_MenuList.push_back(pMenu);
			break;
		}

	default: BsAssert(0);
	}

	
	return true;
}


bool CFcMenuManager::CloseMenu(_FC_MENU_TYPE nType, _FC_MENU_TYPE* nPreType, CFcMenuForm* pMenu, bool bReleaseMovieRTT)
{	
	if(IsSameMenu(nType, pMenu) == false)
		return false;


	if(RemoveMenu(nType, pMenu) == false)
	{
		BsAssert(0);
		return false;
	}

	if(nPreType != NULL){
		(*nPreType) = nType;
	}

	switch(nType)
	{
	case _FC_MENU_SEL_CHAR:
	case _FC_MENU_STATUS:
	case _FC_MENU_ABILITY:
		{
			Release3DObjRTT();
			break;
		}
	}

#ifdef _XBOX
	if(m_hMovieRTTHandle != NULL)
	{
		switch(nType)
		{
		case _FC_MENU_MSGBOX:
		//usual
		case _FC_MENU_USUAL:
		//..
		case _FC_MENU_LOADDATA:
		case _FC_MENU_SAVEDATA:
		case _FC_MENU_OPTION:
			{
				if(bReleaseMovieRTT){
					//ReleaseMovieRTT();
				}
				break;
			}	
		default:
			{
				//ReleaseMovieRTT();
			}
		}
	}

#endif
	
	return true;
}

bool CFcMenuManager::RemoveMenu(_FC_MENU_TYPE nType, CFcMenuForm* pMenu)
{
	FcMenuLISTITOR itr;
	FcMenuLISTITOR itr_begin = m_MenuList.begin();
	for(itr = itr_begin; itr!=m_MenuList.end();itr++)
	{	
		if((*itr)->GetMenuType() == nType && (pMenu == NULL || pMenu == (*itr)))
		{
			delete (*itr);
			m_MenuList.erase(itr);
			return true;
		}
	}

	return false;
}


void CFcMenuManager::RemoveAllMenu()
{
	FcMenuLISTITOR itr;
	FcMenuLISTITOR itr_begin = m_MenuList.begin();
	for(itr = itr_begin; itr!=m_MenuList.end();itr++)
	{	
		delete (*itr);
	}

	m_MenuList.clear();

	Release3DObjRTT();
	//ReleaseMovieRTT();

	if(m_pMenuWorldMap != NULL)
	{
		delete m_pMenuWorldMap;
		m_pMenuWorldMap = NULL;
	}
}

bool CFcMenuManager::IsSameMenu(_FC_MENU_TYPE nType, CFcMenuForm* pMenu)
{
	FcMenuLISTITOR itr;
	FcMenuLISTITOR itr_begin = m_MenuList.begin();
	for(itr = itr_begin; itr!=m_MenuList.end();itr++)
	{
		if((*itr)->GetMenuType() == nType)
		{
			if(pMenu == NULL || pMenu == (*itr))
			{
				return true;
			}
		}
	}

	return false;
}

bool CFcMenuManager::IsSameMessageBox(int nMsgAsk, bool bExceptionClose)
{
	FcMenuLISTITOR itr;
	FcMenuLISTITOR itr_begin = m_MenuList.begin();
	for(itr = itr_begin; itr!=m_MenuList.end();itr++)
	{
		if((*itr)->GetMenuType() == _FC_MENU_MSGBOX)
		{
			CFcMenuForm* pMenu = (*itr);
			if(((CFcMenuMessageBox*)pMenu)->GetMsgAsk() == nMsgAsk)
			{
				if(bExceptionClose)
				{
					switch(pMenu->GetStatus())
					{
					case _MS_CLOSING:
					case _MS_CLOSE:		break;
					default:			return true;
					}
				}
				else{
					return true;
				}
			}
		}
	}

	return false;
}


CFcMenuForm* CFcMenuManager::GetMessageBoxMenu(int nMsgAsk)
{
	FcMenuLISTITOR itr;
	FcMenuLISTITOR itr_begin = m_MenuList.begin();
	for(itr = itr_begin; itr!=m_MenuList.end();itr++)
	{
		if((*itr)->GetMenuType() == _FC_MENU_MSGBOX)
		{
			CFcMenuForm* pMenu = (*itr);
			if(((CFcMenuMessageBox*)pMenu)->GetMsgAsk() == nMsgAsk)
			{
				return pMenu;
			}
		}
	}

	return NULL;
}


CFcMenuForm* CFcMenuManager::GetMenu(_FC_MENU_TYPE nType)
{
	FcMenuLISTITOR itr;
	FcMenuLISTITOR itr_begin = m_MenuList.begin();
	for(itr = itr_begin; itr!=m_MenuList.end();itr++)
	{
		if((*itr)->GetMenuType() == nType)
			return (*itr);
	}

	return NULL;
}


_FC_MENU_TYPE CFcMenuManager::GetCurMenuType()
{
	if((int)m_MenuList.size() == 0){
		return _FC_MENU_NONE;
	}

	FcMenuLISTITOR itr = m_MenuList.end();
	do{
		itr--;
		_FC_MENU_TYPE nType = (*itr)->GetMenuType();
		//if(nType != _FC_MENU_DEBUG && nType != _FC_MENU_MSGBOX) //aleksger: prefix bug 794: The expression was always true.
		{
			return nType;
		}
	}
	while(itr != m_MenuList.begin());

	return _FC_MENU_NONE;
}

DWORD CFcMenuManager::GetCurMenuHandle()
{
	if((int)m_MenuList.size() == 0){
		return NULL;
	}

	FcMenuLISTITOR itr = --m_MenuList.end();
	CFcMenuForm* pMenu = (*itr);
	return (DWORD)pMenu;
}

bool CFcMenuManager::IsSameLoadMenu(_FC_MENU_TYPE nType)
{	
	FcLoadMenuLISTITOR itr;
	FcLoadMenuLISTITOR itr_begin = m_LoadMenuList.begin();
	for(itr = itr_begin; itr!=m_LoadMenuList.end();itr++)
	{
		if((*itr) == nType)
		{			
			return true;
		}
	}

	m_LoadMenuList.push_back(nType);
	return false;
}


bool CFcMenuManager::LoadMenuScript(_FC_MENU_TYPE nType)
{
	if(IsSameLoadMenu(nType) == true){
		return false;
	}

	g_BsKernel.chdir( "Menu" );

	switch(nType)
	{
	case _FC_MENU_DEBUG:	g_BsUiSystem.LoadDef( g_BsKernel.GetFullName("UiDebugMenu.uit")); break;
	case _FC_MENU_MSGBOX:	g_BsUiSystem.LoadDef( g_BsKernel.GetFullName("UiMessageBox.uit")); break;

	//start
	case _FC_MENU_LOGO:
		{
			g_BsUiSystem.LoadDef( g_BsKernel.GetFullName("UiLogoPublisher.uit"));
			g_BsUiSystem.LoadDef( g_BsKernel.GetFullName("UiLogoDeveloper.uit"));
			g_BsUiSystem.LoadDef( g_BsKernel.GetFullName("UiCautionFiction.uit"));
			break;
		}
	
	case _FC_MENU_OPENING:	g_BsUiSystem.LoadDef( g_BsKernel.GetFullName("UiMainOpeningMovie.uit")); break;
	case _FC_MENU_TITLE:	g_BsUiSystem.LoadDef( g_BsKernel.GetFullName("UiTitle.uit")); break;

	//title
	case _FC_MENU_SEL_CHAR:		g_BsUiSystem.LoadDef( g_BsKernel.GetFullName("UiSelectChar.uit")); break;
	case _FC_MENU_LOADDATA:		g_BsUiSystem.LoadDef( g_BsKernel.GetFullName("UiLoadData.uit")); break;
	case _FC_MENU_LIBRARY:		g_BsUiSystem.LoadDef( g_BsKernel.GetFullName("UiLibrary.uit")); break;
	case _FC_MENU_OPTION:		g_BsUiSystem.LoadDef( g_BsKernel.GetFullName("UiOption.uit")); break;

	// mission
	case _FC_MENU_SELECT_STAGE:	g_BsUiSystem.LoadDef( g_BsKernel.GetFullName("UiSelectStage.uit")); break;
	case _FC_MENU_USUAL:		g_BsUiSystem.LoadDef( g_BsKernel.GetFullName("UiUsual.uit")); break;
	case _FC_MENU_SAVEDATA:		g_BsUiSystem.LoadDef( g_BsKernel.GetFullName("UiSaveData.uit")); break;
	case _FC_MENU_GUARDIAN:		g_BsUiSystem.LoadDef( g_BsKernel.GetFullName("UiGuardian.uit")); break;
	
	// library
	case _FC_MENU_LIB_CHAR:		g_BsUiSystem.LoadDef( g_BsKernel.GetFullName("UiCharStockDB.uit")); break;
	case _FC_MENU_LIB_CHAR_SUB:	g_BsUiSystem.LoadDef( g_BsKernel.GetFullName("UiCharStockDB_Sub.uit")); break;
	case _FC_MENU_LIB_MISSION:	g_BsUiSystem.LoadDef( g_BsKernel.GetFullName("UiMissionClearDB.uit")); break;
	case _FC_MENU_LIB_MISSION_SUB:	g_BsUiSystem.LoadDef( g_BsKernel.GetFullName("UiMissionClearDB_Sub.uit")); break;
	case _FC_MENU_LIB_ART:
		{
			g_BsUiSystem.LoadDef( g_BsKernel.GetFullName("UiArtWork.uit"));
			g_BsUiSystem.LoadDef( g_BsKernel.GetFullName("UiIllustviewer.uit"));
			break;
		}
	
	// mission end
	case _FC_MENU_GAME_CLEAR:	g_BsUiSystem.LoadDef( g_BsKernel.GetFullName("UiGameClear.uit")); break;
	case _FC_MENU_STAGE_RESULT:	g_BsUiSystem.LoadDef( g_BsKernel.GetFullName("UiStageResult.uit")); break;
	//case _FC_MENU_END:			g_BsUiSystem.LoadDef( g_BsKernel.GetFullName("UiGoExStage.uit")); break;
	case _FC_MENU_GAME_OVER:	g_BsUiSystem.LoadDef( g_BsKernel.GetFullName("UiGameOver.uit")); break;
	case _FC_MENU_GAME_FAILED:	g_BsUiSystem.LoadDef( g_BsKernel.GetFullName("UiFailed.uit")); break;
	case _FC_MENU_ADVERTISEMENT_TGS:  g_BsUiSystem.LoadDef( g_BsKernel.GetFullName("UiAdvertisement.uit")); break;

	// pause
	case _FC_MENU_PAUSE:		g_BsUiSystem.LoadDef( g_BsKernel.GetFullName("UiPause.uit")); break;
	case _FC_MENU_STATUS:		g_BsUiSystem.LoadDef( g_BsKernel.GetFullName("UiStatus.uit")); break;
	
	//etc
	case _FC_MENU_SPECIAL_SELSTAGE:	g_BsUiSystem.LoadDef( g_BsKernel.GetFullName("UiSpecialSelectStage.uit")); break;
	case _FC_MENU_ABILITY:		g_BsUiSystem.LoadDef( g_BsKernel.GetFullName("UiAbility.uit")); break;
	case _FC_MENU_CONTROL:		g_BsUiSystem.LoadDef( g_BsKernel.GetFullName("UiControl.uit")); break;
	case _FC_MENU_MISSION_OBJ:	g_BsUiSystem.LoadDef( g_BsKernel.GetFullName("UiMissionObj.uit")); break;

	default:
		{
			BsAssert(0);
			return false;
		}
	}

	g_BsKernel.chdir( ".." );

	return true;
}

void CFcMenuManager::UpdateGameInterface()
{
	switch(g_FCGameData.State)
	{
	case GAME_STATE_INIT_STAGE:
	case GAME_STATE_LOOP_STAGE:
	case GAME_STATE_INIT_PAUSE:
	case GAME_STATE_LOOP_PAUSE:
		{
			switch(GetCurMenuType())
			{
			case _FC_MENU_MSGBOX:	break;
			//puase
			case _FC_MENU_PAUSE:
			case _FC_MENU_STATUS:
			case _FC_MENU_ABILITY:
			case _FC_MENU_CONTROL:
			case _FC_MENU_MISSION_OBJ:
			case _FC_MENU_OPTION:
			//mission end
			case _FC_MENU_GAME_CLEAR:
			case _FC_MENU_GAME_OVER:
			case _FC_MENU_STAGE_RESULT:
				{
					g_InterfaceManager.ShowInterface(false);
					break;
				}
			default:
				{
					if( g_pFcRealMovie ){
                        g_InterfaceManager.ShowInterface( !g_pFcRealMovie->IsPlay() );
					}
					else{
						g_InterfaceManager.ShowInterface( true );
					}
					break;
				}
			}
			break;
		}
	case GAME_STATE_INIT_RESULT_SIGN:
	case GAME_STATE_LOOP_RESULT_SIGN:
	case GAME_STATE_INIT_RESULT:
	case GAME_STATE_LOOP_RESULT:
		{
			g_InterfaceManager.ShowInterface( false );
		}
		break;
	case GAME_STATE_SYSTEM_MESSAGE: break;
	default:
		{
			g_InterfaceManager.ShowInterface(true);
			break;
		}
	}
}

void CFcMenuManager::UpdateSpecialEffect()
{
	// back ground moive
	UpdateBGMovie();

	// 3d char
	if(m_pFcMenu3DObj != NULL){
		//m_pFcMenu3DObj->Update();
	}
	
	// worldmap
	if(m_pMenuWorldMap != NULL){
		m_pMenuWorldMap->Update();
	}
}

bool CFcMenuManager::UpdateBGMovie()
{
	if(m_hMovieRTTHandle == NULL){
		return false;
	}

	if(g_BsKernel.GetRTTManager()->GetRTTextrue(m_hMovieRTTHandle)->IsEnable() == false){
		return false;
	}

	if(m_nMovieTextureId != -1)
	{
		SIZE size = g_BsKernel.GetTextureSize(m_nMovieTextureId);
		g_BsKernel.DrawUIBox(_Ui_Mode_BGMovie, //_Ui_Mode_Movie,
			0, 0, _SCREEN_WIDTH, _SCREEN_HEIGHT,
			0.f, D3DXCOLOR(1,1,1,1),
			0.f,
			m_nMovieTextureId,
			0, 0, size.cx, size.cy,
			-1, true);
	}

	return true;
}

void CFcMenuManager::LoadGameMenuData()
{
	// mission end
	LoadMenuScript(_FC_MENU_GAME_CLEAR);
	LoadMenuScript(_FC_MENU_STAGE_RESULT);
	//LoadMenuScript(_FC_MENU_END);
	LoadMenuScript(_FC_MENU_GAME_OVER);
	LoadMenuScript(_FC_MENU_GAME_FAILED);

	// pause
	LoadMenuScript(_FC_MENU_PAUSE);
	LoadMenuScript(_FC_MENU_STATUS);
	LoadMenuScript(_FC_MENU_ABILITY);
	LoadMenuScript(_FC_MENU_CONTROL);
	LoadMenuScript(_FC_MENU_MISSION_OBJ);
	LoadMenuScript(_FC_MENU_OPTION);
}


void CFcMenuManager::CheckedSystemMessageBox(SYS_MESSAGE nMsg, DWORD param1, DWORD param2, DWORD param3, DWORD param4)
{
	if(nMsg == _SYS_MSG_REMOVE_PAD)
	{
		//g_MenuHandle->CheckedSystemMessageBox(_SYS_MSG_REMOVE_PAD, (DWORD)(!bRemove));
		// 타이틀 화면에서 사용하는 Xbox 360 컨트롤러가 선택되지 않은 상태에서는 경고메시지가 표시되지 않는다.
		if(param1)
		{
			switch(GetCurMenuType())
			{
			//start
			case _FC_MENU_LOGO:
			case _FC_MENU_OPENING:
			case _FC_MENU_CREDITS:			break;
			default:
				{
					SetSystemMessageLossOfController();
					break;
				}
			}
		}
	} else if( nMsg == _SYS_MSG_CANCEL_DEVICESELECTOR) 
	{
		switch(GetCurMenuType())
			{
			//start
			case _FC_MENU_LOGO:
			case _FC_MENU_OPENING:
			case _FC_MENU_CREDITS:
			
			//library
			case _FC_MENU_LIBRARY:
			case _FC_MENU_LIB_CHAR:
			case _FC_MENU_LIB_CHAR_SUB:
			case _FC_MENU_LIB_MISSION:
			case _FC_MENU_LIB_MISSION_SUB:
			case _FC_MENU_LIB_ART:			break;
			default:
				{
					SetSystemMessageCancelledDeviceSelector();
					break;
				}
			}
	}
	else if( nMsg == _SYS_MSG_REMOVE_STORAGE) 
	{
		switch(GetCurMenuType())
			{
			//start
			case _FC_MENU_LOGO:
			case _FC_MENU_OPENING:
			case _FC_MENU_CREDITS:
			
			//library
			case _FC_MENU_LIBRARY:
			case _FC_MENU_LIB_CHAR:
			case _FC_MENU_LIB_CHAR_SUB:
			case _FC_MENU_LIB_MISSION:
			case _FC_MENU_LIB_MISSION_SUB:
			case _FC_MENU_LIB_ART:			break;
			default:
				{
					SetSystemMessageRemoveStorage();
					break;
				}
			}
	}
}

void CFcMenuManager::SetSystemMessage(FcMessageToken* pMsg)
{
	/*
	CFcMenuMessageBox* pMenu = new CFcMenuMessageBox(nType,		//type
		param1,													//nMsgType(FcMSG_ASK_TYPE)
		param2,													//Cmd Main State(GAME_STATE)
		(CFcMenuForm*)param3,									//pCmdMenu(pMenu)
		param4,													//hCmdWnd(hWnd)
		param5,													//dwFocusButton Index
		param6);												//dwValue
	*/

	//dwValue
	if(pMsg->Param6 != (DWORD)GAME_STATE_SYSTEM_MESSAGE){
		return;
	}

	//nMsgType(FcMSG_ASK_TYPE)
	switch(pMsg->Param1)
	{
	case fcMT_LossOfController:
		{
			switch(g_FC.GetMainState())
			{
			case GAME_STATE_INIT_STAGE:
			case GAME_STATE_LOOP_STAGE:
			case GAME_STATE_INIT_RESULT_SIGN:
			case GAME_STATE_LOOP_RESULT_SIGN:
				{
#ifdef _XBOX
					if(g_FC.GetLiveManager()->IsUIActive() == false){
						g_FC.SetPause(true);
					}
#endif
					break;
				}
			}
			break;
		}
	case fcMT_CancelDeviceSelector:		break;
	case fcMT_RemoveStorage:			
		if( g_pSoundManager ){
			g_pSoundManager->SetPauseAll( true );
		}
		break;
	}
		
	g_FC.SetMainState(GAME_STATE_SYSTEM_MESSAGE);
}

void CFcMenuManager::SetSystemMessageLossOfController()
{
	if(GetCurMenuType() == _FC_MENU_TITLE)
	{
		CFcMenuTitle* pTitle = (CFcMenuTitle*)GetMenu(_FC_MENU_TITLE);
		if(pTitle->IsStartLayer() == true ){
			return;
		}
	}

	if(IsSameMessageBox(fcMT_LossOfController, true)){
		return;
	}
	
	BsUiHANDLE hHandle = NULL;
	g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,		//message
		fcMT_LossOfController,								//param1
		NULL,												//param2
		hHandle,											//param3
		NULL,												//param4
		NULL,												//param5
		(DWORD)GAME_STATE_SYSTEM_MESSAGE);					//param6
}

void CFcMenuManager::SetSystemMessageCancelledDeviceSelector()
{
	if(IsSameMessageBox(fcMT_CancelDeviceSelector, true)){
		return;
	}

	BsUiHANDLE hHandle = NULL;
	g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,		//message
		fcMT_CancelDeviceSelector,							//param1
		NULL,												//param2
		hHandle,											//param3
		NULL,												//param4
		NULL,												//param5
		(DWORD)GAME_STATE_SYSTEM_MESSAGE);					//param6
}

void CFcMenuManager::SetSystemMessageRemoveStorage()
{
	if(IsSameMessageBox(fcMT_RemoveStorage, true)){
		return;
	}

	BsUiHANDLE hHandle = NULL;
	g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,		//message
		fcMT_RemoveStorage,									//param1
		NULL,												//param2
		hHandle,											//param3
		NULL,												//param4
		NULL,												//param5
		(DWORD)GAME_STATE_SYSTEM_MESSAGE);					//param6
}

bool CFcMenuManager::CheckMenuWaveState()
{	
	if(g_BsKernel.GetClickFxEffect() == NULL){
		return false;
	}

	if(m_WaveState == _MENU_WAVE_END){
		return false;
	}
		
	int nCurFrame = g_BsKernel.GetClickFxEffect()->GetLastWaveRemainFrame();
	if(m_AddWaveCount < m_nMAXWaveCount && nCurFrame > 10)
	{
		SYSTEMTIME LocalSysTime;
		GetLocalTime( &LocalSysTime );
		int nM = (int)(LocalSysTime.wMinute);
		int nS = (int)(LocalSysTime.wSecond);
		int nMS = (int)(LocalSysTime.wMilliseconds);

		srand(nM * nS * nMS);

		POINT Addpos;
		//Addpos.x = RandomforMenu(_WAVE_AREA_X);
		//Addpos.y = RandomforMenu(_WAVE_AREA_Y);
		Addpos.x = rand() % _WAVE_AREA_X;
		Addpos.y = rand() % _WAVE_AREA_Y;
		g_BsKernel.GetClickFxEffect()->AddWave((_SCREEN_WIDTH/4) + Addpos.x,
			(_SCREEN_HEIGHT/4) + Addpos.y,
			m_nWaveSize, m_nWaveSpeed);

		m_AddWaveCount++;
	}

	m_WaveState = _MENU_WAVE_RUN;

	int nWave = g_BsKernel.GetClickFxEffect()->GetWaveCount();
	if(nWave == 0 && m_AddWaveCount == m_nMAXWaveCount)
	{
		m_WaveState = _MENU_WAVE_END;
		m_AddWaveCount = 0;
	}

	return true;
}

void CFcMenuManager::UpdateWave(FcMessageToken* pMsg)
{
	if(g_BsKernel.GetClickFxEffect() == NULL){
		return;
	}

	if(m_WaveState != _MENU_WAVE_END){
		return;
	}

	BsUiWindow* pFocus = (BsUiWindow*)(pMsg->Param1);
	m_nMAXWaveCount = pMsg->Param2 ? (int)(pMsg->Param2) : 1;
	m_nWaveSize = pMsg->Param3 ? (int)(pMsg->Param3) : _WAVE_DEFAULT_SIZE;
	m_nWaveSpeed = pMsg->Param4 ? (int)(pMsg->Param4) : _WAVE_DEFAULT_SPEED;

	RECT rect;
	if(pFocus != NULL)
	{
		if(pFocus->GetType() == BsUiTYPE_LISTBOX){
			BsUiListBox* pListBox = (BsUiListBox*)pFocus;
			pListBox->GetItemRect(pListBox->GetCurSel(), rect);
		}
		else{
			pFocus->GetWindowRect(&rect);
		}
	}
	else
	{
		rect.left = 0; rect.right = _SCREEN_WIDTH;
		rect.top = 0; rect.bottom = _SCREEN_HEIGHT;
	}

	m_WavePos.x = (rect.right - rect.left) / 2 + rect.left;
	m_WavePos.y = (rect.bottom - rect.top) / 2 + rect.top;

	g_BsKernel.GetClickFxEffect()->AddWave((int)m_WavePos.x, (int)m_WavePos.y, m_nWaveSize, m_nWaveSpeed);
	m_AddWaveCount++;

	m_WaveState = _MENU_WAVE_START;

	g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_START" );

	return;
}


//조건 :
// 1. 이미 load되어 있으면 무시한다.
// 2. 없는 sound를 delete하려고 하면 무시한다.
bool CFcMenuManager::UpdateSound(FCMenu_Message message)
{
	//DebugString( "*********************************************\n" );
	//DebugString( "Sound Menu Msg %d \n" ,message );
	//DebugString( "*********************************************\n" );
	switch(message)
	{	
	case fcMSG_LOGO_START:
		{
			break;
		}
	case fcMSG_TITLE_START:
		{
			//delete mission failed sound
			//load title
			if( IsSameMenu( _FC_MENU_OPTION ) || IsSameMenu( _FC_MENU_LIBRARY ) || IsSameMenu(_FC_MENU_LOADDATA) || IsSameMenu(_FC_MENU_SEL_CHAR) )
				break;


			g_pSoundManager->StopBGM();
			g_pSoundManager->DeleteWaveBank( WB_M_GAMEOVER );
			g_pSoundManager->LoadWaveBank( WB_M_SELECT );
			g_pSoundManager->PlayBGM( "M_Select" );
// !!!BGMFadeIn			
			break;
		}
	case fcMSG_SEL_CHAR_START:
		{
			//g_pSoundManager->DeleteWaveBank( WB_M_SELECT );
			//g_pSoundManager->LoadWaveBank( WB_M_SELECT );
			//delete title
			//load select char
			break;
		}
	case fcMSG_FIRST_MISSION_START: //첫번째 게임들어갈 때(셀렉트 메뉴에서 들어갈 때)
		{
//			g_pSoundManager->StopBGM();
			g_pSoundManager->StopBGM( true );

			if( g_pSoundManager->IsBGMPlay() )
				return false;

// !!!BGMFadeIn
			g_pSoundManager->DeleteWaveBank( WB_M_SELECT );
			//delete select title;

			break;
		}
	//case fcMSG_GO_TO_SEL_STAGE:					//stage clear시에 들어온다.
	case fcMSG_GO_TO_LOAD_SELECT_STAGE:			//Load시에 들어온다.
	case fcMSG_SELECT_STAGE_START:
		{
			//delete title
			//delete mission success
			//delete mission failed sound
			//load worldmap
			g_pSoundManager->StopBGM();
			g_pSoundManager->DeleteWaveBank( WB_M_SELECT );
			g_pSoundManager->DeleteWaveBank( WB_M_RESULT );
			g_pSoundManager->DeleteWaveBank( WB_M_GAMEOVER );
			g_pSoundManager->LoadWaveBank( WB_M_MAP );
			g_pSoundManager->PlayBGM( "M_Map" );
			break;
		}
	case fcMSG_MISSION_START: //게임 내 들어갈 때
		{
			//delete mission worldmap
			g_pSoundManager->StopBGM();
			g_pSoundManager->DeleteWaveBank( WB_M_GAMEOVER );
			g_pSoundManager->DeleteWaveBank( WB_M_MAP );

			// 게임이 레터 박스 상태에서 끝나면 로딩 화면에 레터 박스 상태로 나온다. 그래서 clear를 콜
			g_InterfaceManager.Clear();

			ReleasePreCharLoading();
			ReleaseMovieRTT();

			break;
		}
	case fcMSG_GAME_CLEAR_START:
		{
			g_pSoundManager->PlaySystemSound( SB_COMMON, "SYS_MISSION_CLEAR");
			break;
		}
	case fcMSG_STAGE_RESULT_START:
		{
			//load mission success sound
#ifdef _XBOX
			g_pSoundManager->SetSeSoundPlayingOn(true);
#endif
			g_pSoundManager->StopBGM();
			g_pSoundManager->LoadWaveBank( WB_M_RESULT );
			if( GetCurMenuType() == _FC_MENU_STAGE_RESULT)
			{	
				g_pSoundManager->PlayBGM( "M_Result" );
			}
			break;
		}
	case fcMSG_ENDING_CHAR_START:
	case fcMSG_END_START:
		{
			g_pSoundManager->FinishStage();
			g_pSoundManager->DeleteWaveBank( WB_M_RESULT );
			break;
		}		
	case fcMSG_GAME_OVER_START:
		{
#ifdef _XBOX
			g_pSoundManager->SetSeSoundPlayingOn(true);
#endif
			//load mission failed sound
			g_pSoundManager->StopBGM();
			g_pSoundManager->LoadWaveBank( WB_M_GAMEOVER );
			g_pSoundManager->PlayBGM( "M_Gameover" );
			break;
		}
	case fcMSG_GO_TO_TITLE: //다음 스테이지가 없어서 타이틀로 향할 때
		{
			//delete mission success sound
			g_pSoundManager->StopBGM();
			g_pSoundManager->DeleteWaveBank( WB_M_RESULT );
			break;
		}
	case fcMSG_SPECIAL_SELSTAGE_START:
		{
			g_pSoundManager->StopSoundAll();
			break;
		}
	}
	return true;
}


bool CFcMenuManager::IsSameMenuMessage(FCMenu_Message message)
{
	DWORD dwCount = m_msgQue.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		FcMessageToken* pMsg = m_msgQue[i];
		if(pMsg->message == message){
			return true;
		}
	}

	return false;
}


void CFcMenuManager::SetGuardianforFirstMission()
{
	g_FCGameData.GuardianInfo[0].clear();
	g_FCGameData.GuardianInfo[1].clear();
	
	switch(g_FCGameData.nPlayerType)
	{
	//guardian이 있다.
	case MISSION_TYPE_INPHYY:
		{
			if(g_FCGameData.SelStageId == STAGE_ID_ANOTHER_WORLD){
				return;
			}
			break;
		}
	case MISSION_TYPE_ASPHARR:
	case MISSION_TYPE_MYIFEE:	break;
	//guardian이 없다.
	case MISSION_TYPE_TYURRU:
	case MISSION_TYPE_KLARRANN:
	case MISSION_TYPE_DWINGVATT:
	case MISSION_TYPE_VIGKVAGK: return;
	default: BsAssert(0);
	}

	WorldMapPointLIST* pPointList = m_pMenuWorldMapInfo->GetPointList();
	BsAssert(pPointList);

	WorldMapPointInfo* pPointInfo = NULL;
	for(DWORD i=0; i<pPointList->size(); i++)
	{
		GAME_STAGE_ID StageId = (GAME_STAGE_ID)((*pPointList)[i].nId);
		if(g_FCGameData.SelStageId == StageId)
		{
			pPointInfo = &((*pPointList)[i]);
			break;
		}
	}
	BsAssert(pPointInfo);

	for(int i=0; i<2; i++)
	{
		int nSoxID = pPointInfo->nGuadian_L;
		if(i == 1){
			nSoxID = pPointInfo->nGuadian_R;
		}

		bool bValue = false;
		for(int j=0; j<GT_MAX; j++)
		{	
			if(pPointInfo->GuadianType[j].nID == nSoxID)
			{
				bValue = true;
				break;
			}
		}

		BsAssert(bValue);
	}

	int nGrdType_L = GetGuardianType(pPointInfo->nGuadian_L);
	if(nGrdType_L != -1){
		g_FCGameData.GuardianInfo[0] = pPointInfo->GuadianType[nGrdType_L];
	}

	int nGrdType_R = GetGuardianType(pPointInfo->nGuadian_R);
	if(nGrdType_R != -1){
		g_FCGameData.GuardianInfo[1] = pPointInfo->GuadianType[nGrdType_R];
	}
}