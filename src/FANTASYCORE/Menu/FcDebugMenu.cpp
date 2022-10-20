#include "StdAfx.h"
#include "BsKernel.h"

#include "FcDebugMenu.h"
#include "BsUiSystem.h"
#include "InputPad.h"

#include "FcBaseObject.h"
#include "FcCameraObject.h"

#include "FcWorld.h"
#include "FantasyCore.h"

#include "FcInterfaceManager.h"
#include "LocalLanguage.h"

#include ".\\data\\Menu\\BsUiControlID.h"
#include "FcItem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

#ifndef _LTCG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
CFcMenuDebug::CFcMenuDebug(_FC_MENU_TYPE nType)
: CFcMenuForm(nType)
{
	BsUiHANDLE hMenuMain = AddUiLayer(new BsUiMainDebug(this), SN_DEBUGMENU_1);
	((BsUiMainDebug*)BsUi::BsUiGetWindow(hMenuMain))->Initialize();
	
	BsUiHANDLE hMenuSub = AddUiLayer(new BsUiSubDebug(this), SN_DEBUGMENU_2);
	((BsUiSubDebug*)BsUi::BsUiGetWindow(hMenuSub))->Initialize();
	
	g_BsUiSystem.SetFocusWindow(BsUi::BsUiGetWindow(hMenuMain));
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#define _DEBUG_ITEM_LOAD_MAP		0
#define _DEBUG_ITEM_LOCAL_LANGUAGE	1
#define _DEBUG_ITEM_PLAYER_COUNT	2
#define _DEBUG_ITEM_FREE_CAM		3
#define _DEBUG_ITEM_CAM_EDIT		4
#define _DEBUG_ITEM_CAM_SAVE		5
#define _DEBUG_ITEM_TITLE			6
#define _DEBUG_ITEM_SAVE_SET		7
#define _DEBUG_ITEM_SAVE_PLAY		8
#define _DEBUG_ITEM_PAUSE_CALL		9
#define _DEBUG_ITEM_STATUS_CALL		10

#define _TEXT_LOCAL_LANGUAGE	"Local Language :"
#define _TEXT_PLAYER_SET		"Player Set :"
#define _TEXT_SAVE_SET			"Save Set :"


BsUiMainDebug::BsUiMainDebug(CFcMenuForm* pMenu)
: BsUiLayer()
{
	m_pMenu = pMenu;
}

void BsUiMainDebug::Initialize()
{
	UpdateListBox();
}

void BsUiMainDebug::UpdateListBox()
{
	BsUiListBox* pListBox = (BsUiListBox*)GetWindow(LB_LIST_1);
	BsAssert(pListBox);

	pListBox->ClearItem();

	pListBox->AddItem("Load Map", _DEBUG_ITEM_LOAD_MAP);

	char szTemp[256];
	sprintf(szTemp, "%s %s", _TEXT_LOCAL_LANGUAGE, g_LocalLanguage.GetLanguageStr(g_LocalLanguage.GetLanguage()));
	pListBox->AddItem(szTemp, _DEBUG_ITEM_LOCAL_LANGUAGE );

	sprintf(szTemp, "%s (%d)", _TEXT_PLAYER_SET, g_FCGameData.nPlayerCount);
	pListBox->AddItem(szTemp, _DEBUG_ITEM_PLAYER_COUNT);

	pListBox->AddItem("Free Cam", _DEBUG_ITEM_FREE_CAM );
	pListBox->AddItem("Cam Edit", _DEBUG_ITEM_CAM_EDIT);
	pListBox->AddItem("Cam Save", _DEBUG_ITEM_CAM_SAVE);
	pListBox->AddItem("Title", _DEBUG_ITEM_TITLE);

	if( g_FCGameData.bSaveInput ){
		sprintf(szTemp, "%s true", _TEXT_SAVE_SET);
	}
	else{
		sprintf(szTemp, "%s false", _TEXT_SAVE_SET);
	}

	pListBox->AddItem(szTemp, _DEBUG_ITEM_SAVE_SET);
	pListBox->AddItem("Save Play", _DEBUG_ITEM_SAVE_PLAY);

	//if(g_FCGameData.State == GAME_STATE_INIT_PAUSE)
	{
		pListBox->AddItem("Pause", _DEBUG_ITEM_PAUSE_CALL);
		pListBox->AddItem("Status", _DEBUG_ITEM_STATUS_CALL);
	}

	InitSize();
}

void BsUiMainDebug::InitSize()
{
	BsUiListBox* pListBox = (BsUiListBox*)GetWindow(LB_LIST_1);
	BsAssert(pListBox);

	char* pszAttr = g_FontAttrMgr.GetAttrText(pListBox->GetFontAttr(BsUiFS_SELECTED));
	g_pFont->DrawUIText(0, 0, 0, 0, pszAttr);

	int nWidth(pListBox->GetWindowSize().x);
	int nHeight=0, nLine=0, nTempWidth=0;//aleksger: prefix bug 774: Uninitialized variables.
	int nWordCount(0);
	for(int i=0; i<pListBox->GetItemCount(); i++)
	{
		char* pStr = pListBox->GetString(i);
		g_pFont->GetTextLengthInfo(nTempWidth, nHeight, nLine, nWordCount, 0, 0, -1, -1, pStr);
		if(nWidth < nTempWidth){
			nWidth = nTempWidth;
		}
	}

	BsUiLayer* pLayer = (BsUiLayer*)GetWindow(SN_DEBUGMENU_1);
	BsAssert(pLayer);
	
	pLayer->SetWindowWidth(nWidth+20);
	POINT posListBox = {pListBox->GetWindowPos().x + nWidth, 0};
	pListBox->ResizeWindowSize(_CHANGE_SIZE_RIGHT, posListBox);
}

DWORD BsUiMainDebug::ProcMessage(xwMessageToken* pMsgToken)
{
	BsAssert(pMsgToken);

	BsUiCLASS hClass = BsUi::BsUiGetClass(pMsgToken->hWnd);

	switch(pMsgToken->message)
	{
	case XWMSG_KEYDOWN:
		{
			switch(hClass)
			{
			case LB_LIST_1: OnKeyDownListBox(pMsgToken); break;
			}
		}
		break;
	}

	return BsUiWindow::ProcMessage(pMsgToken);
}

void BsUiMainDebug::OpenWorldLoadList()
{
	BsUiWindow* pWindow;
	pWindow = g_BsUiSystem.GetWindow(SN_DEBUGMENU_2);
	BsAssert(pWindow);
	pWindow->SetWindowAttr(XWATTR_SHOWWINDOW, true);
	g_BsUiSystem.SetFocusWindowClass(LB_LIST_2);
	( ( BsUiSubDebug * ) pWindow )->InitWorldLoadList();
}

void BsUiMainDebug::SetLocalLanguage()
{
	BsUiWindow* pWindow;
	pWindow = g_BsUiSystem.GetWindow(SN_DEBUGMENU_2);
	BsAssert(pWindow);
	pWindow->SetWindowAttr(XWATTR_SHOWWINDOW, true);
	g_BsUiSystem.SetFocusWindowClass(LB_LIST_2);
	( ( BsUiSubDebug * ) pWindow )->InitLocalLanguage();
}

void BsUiMainDebug::SetPlayerCount()
{
	if(g_FcWorld.GetInstance().IsLoad() == true)
	{
		return;
	}

	g_FCGameData.nPlayerCount += 1;
	if(g_FCGameData.nPlayerCount == 3)
	{
		g_FCGameData.nPlayerCount = 1;
	}

	char szTemp[256];
	sprintf(szTemp, "%s (%d)", _TEXT_PLAYER_SET, g_FCGameData.nPlayerCount);

	BsUiListBox* pListBox = (BsUiListBox*)GetWindow(LB_LIST_1);
	pListBox->SetItemText(_DEBUG_ITEM_PLAYER_COUNT, szTemp);
}

void BsUiMainDebug::FreeCamMode()
{
	CameraObjHandle CamHandle;

	CamHandle = CFcBaseObject::GetCameraObjectHandle( 0 );
	if( CamHandle )
	{
		if( CamHandle->IsFreeCamMode() )
		{
			CamHandle->SetFreeCamMode( false );
			CInputPad::GetInstance().Enable( true );
			//CInputPad::GetInstance().Break( false );
		}
		else
		{
			CamHandle->SetFreeCamMode( true );
			CInputPad::GetInstance().Enable( false );
			//CInputPad::GetInstance().Break( true );
		}
	}
	g_MenuHandle->PostMessage(fcMSG_DEBUG_MENU_OUT);
}

void BsUiMainDebug::CamEditMode()
{
	CameraObjHandle CamHandle;

	CamHandle = CFcBaseObject::GetCameraObjectHandle( 0 );

	if( CamHandle )
		CamHandle->SetEditMode(!CamHandle->IsEditMode());

	g_MenuHandle->PostMessage(fcMSG_DEBUG_MENU_OUT);
}

void BsUiMainDebug::CamSave()
{
	CameraObjHandle CamHandle;

	CamHandle = CFcBaseObject::GetCameraObjectHandle( 0 );

	if( CamHandle )
		CamHandle->Save();

	g_MenuHandle->PostMessage(fcMSG_DEBUG_MENU_OUT);
}

void BsUiMainDebug::StartTitle()
{
	SetWindowAttr( XWATTR_SHOWWINDOW, false );
	g_BsUiSystem.SetFocusWindow( NULL );
	
	//g_MenuHandle->PostMessage(fcMSG_DEBUG_MENU_OUT);
	g_MenuHandle->PostMessage(fcMSG_REMOVE_ALL_MENU);
	//g_MenuHandle->PostMessage(fcMSG_LOGO_START);
	g_MenuHandle->PostMessage(fcMSG_TITLE_START);
	//g_MenuHandle->PostMessage(fcMSG_SEL_CHAR_START);
	//g_MenuHandle->PostMessage(fcMSG_SELECT_STAGE_START);
	//g_MenuHandle->PostMessage(fcMSG_STAGE_RESULT_START);
	//g_MenuHandle->PostMessage(fcMSG_GUARDIAN_START);
}

void BsUiMainDebug::ToggleSaveSet()
{
	//save set flag = false;
	static bool bsavesettempFlag = false;

	bsavesettempFlag = !bsavesettempFlag;

	char szTemp[256];
	if(bsavesettempFlag)
	{
		sprintf(szTemp, "%strue", _TEXT_SAVE_SET);
		g_FCGameData.bSaveInput = true;
	}
	else
	{
        sprintf(szTemp, "%sfalse", _TEXT_SAVE_SET);
		g_FCGameData.bSaveInput = false;
		CInputPad::GetInstance().StopSave();
	}

	BsUiListBox* pListBox = (BsUiListBox*)GetWindow(LB_LIST_1);
	pListBox->SetItemText(_DEBUG_ITEM_SAVE_SET, szTemp);
}

void BsUiMainDebug::OpenSavePlayFile()
{
	BsUiWindow* pWindow;
	pWindow = g_BsUiSystem.GetWindow(SN_DEBUGMENU_2);
	BsAssert(pWindow);
	pWindow->SetWindowAttr(XWATTR_SHOWWINDOW, true);
	g_BsUiSystem.SetFocusWindowClass(LB_LIST_2);
	( ( BsUiSubDebug * ) pWindow )->InitPlayList();
}

void BsUiMainDebug::StartPauseMenu()
{	
	// pause setting을 무시한다.
	g_MenuHandle->PostMessage(fcMSG_DEBUG_MENU_OUT, 1);

	g_MenuHandle->SetGamePauseMenu(_FC_MENU_PAUSE);
	g_MenuHandle->PostMessage(fcMSG_PAUSE_START);
}

void BsUiMainDebug::StartStatusMenu()
{	
	g_MenuHandle->PostMessage(fcMSG_DEBUG_MENU_OUT, 1);

	g_MenuHandle->SetGamePauseMenu(_FC_MENU_STATUS);
	g_MenuHandle->PostMessage(fcMSG_STATUS_START);
}

void BsUiMainDebug::OnKeyDownListBox(xwMessageToken* pMsgToken)
{
	switch (pMsgToken->lParam)
	{
	case PAD_INPUT_A:
		{
			BsUiListBox *pListBox;

			pListBox = ( BsUiListBox * )GetWindow( LB_LIST_1 );
			BsAssert( pListBox );

			switch( pListBox->GetItemData(pListBox->GetCurSel()))
			{
			case _DEBUG_ITEM_LOAD_MAP:			OpenWorldLoadList(); break;
			case _DEBUG_ITEM_LOCAL_LANGUAGE:	SetLocalLanguage(); break;
			case _DEBUG_ITEM_PLAYER_COUNT:		SetPlayerCount(); break;
			case _DEBUG_ITEM_FREE_CAM:			FreeCamMode(); break;
			case _DEBUG_ITEM_CAM_EDIT:			CamEditMode(); break;
			case _DEBUG_ITEM_CAM_SAVE:			CamSave(); break;
			case _DEBUG_ITEM_TITLE:				StartTitle(); break;
			case _DEBUG_ITEM_SAVE_SET:			ToggleSaveSet(); break;
			case _DEBUG_ITEM_SAVE_PLAY:			OpenSavePlayFile(); break;
			case _DEBUG_ITEM_PAUSE_CALL:		StartPauseMenu(); break;
			case _DEBUG_ITEM_STATUS_CALL:		StartStatusMenu(); break;
			}
		
			//g_MenuHandle->PostMessage(fcMSG_CLICK_FX_EFFECT, (DWORD)pListBox);
		}
		break;
	case PAD_INPUT_B:
		g_MenuHandle->PostMessage(fcMSG_DEBUG_MENU_OUT);
		break;
	}
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
BsUiSubDebug::BsUiSubDebug(CFcMenuForm* pMenu)
: BsUiLayer()
{
	m_pMenu = pMenu;
}

void BsUiSubDebug::Initialize()
{
	SetWindowAttr(XWATTR_SHOWWINDOW, false);
	m_nOpType = -1;
}

struct SortFilter_String {
	inline bool operator() ( std::string &a, std::string &b ) {
		return ( _stricmp( a.c_str(), b.c_str() ) < 0 ) ? true : false;
	}
};

void BsUiSubDebug::InitWorldLoadList()
{
	BsUiListBox* pListBox = (BsUiListBox*)GetWindow(LB_LIST_2);
	BsAssert(pListBox);

	char szFind[_MAX_PATH];
	WIN32_FIND_DATA FindFile;
	HANDLE hFindHandle;
	std::vector< std::string > FileList;
	std::string szFindName;

	g_BsKernel.chdir( "map" );
	strcpy( szFind, g_BsKernel.GetCurrentDirectory() );
	strcat( szFind, "*.bsmap" );
	g_BsKernel.chdir( ".." );

	pListBox->ClearItem();
	hFindHandle = FindFirstFile( szFind, &FindFile );
	if( hFindHandle == INVALID_HANDLE_VALUE ){
		return;
	}

	do{	
		szFindName = FindFile.cFileName;
		FileList.push_back( szFindName );
//		pListBox->AddItem( FindFile.cFileName, 0);
	}while(FindNextFile(hFindHandle, &FindFile));
	FindClose(hFindHandle);

	int i, nSize;

	sort( FileList.begin(), FileList.end(), SortFilter_String() );
	nSize = ( int )FileList.size();
	for( i = 0; i < nSize; i++ )
	{
		pListBox->AddItem( ( char * )FileList[ i ].c_str(), 0);
	}

	pListBox->SetCurSel(0);

	m_nOpType = _DEBUG_ITEM_LOAD_MAP;

	InitSize();
}

void BsUiSubDebug::InitLocalLanguage()
{
	BsUiListBox* pListBox = (BsUiListBox*)GetWindow(LB_LIST_2);
	BsAssert(pListBox);

	pListBox->ClearItem();
	for(int i=0; i<MAX_LANGAUGES; i++)
	{
		char* pText = (char*)g_LocalLanguage.GetLanguageStr((GAME_LANGUAGE)i);
		pListBox->AddItem(pText , i);
	}

	pListBox->SetCurSel((int)g_LocalLanguage.GetLanguage());

	m_nOpType = _DEBUG_ITEM_LOCAL_LANGUAGE;

	InitSize();
}

void BsUiSubDebug::InitPlayList()
{
	BsUiListBox* pListBox = (BsUiListBox*)GetWindow(LB_LIST_2);
	BsAssert(pListBox);

	char szFind[_MAX_PATH];
	WIN32_FIND_DATA FindFile;
	HANDLE hFindHandle;

	g_BsKernel.chdir( "input" );
	strcpy( szFind, g_BsKernel.GetCurrentDirectory() );
	strcat( szFind, "*.inp" );
	g_BsKernel.chdir( ".." );

	pListBox->ClearItem();

	hFindHandle = FindFirstFile( szFind, &FindFile );
	if( hFindHandle == INVALID_HANDLE_VALUE ){
		return;
	}

	do{	
		pListBox->AddItem( FindFile.cFileName, 0);
	}while( FindNextFile( hFindHandle, &FindFile ) );
	FindClose(hFindHandle);

	pListBox->SetCurSel(0);

	m_nOpType = _DEBUG_ITEM_SAVE_PLAY;

	InitSize();
}

void BsUiSubDebug::InitSize()
{
	BsUiHANDLE hHandle = m_pMenu->GetUiLayer(SN_DEBUGMENU_1);
	BsUiLayer* pLayer1 = (BsUiLayer*)BsUi::BsUiGetWindow(hHandle);
	BsAssert(pLayer1);

	hHandle = m_pMenu->GetUiLayer(SN_DEBUGMENU_2);
	BsUiLayer* pLayer2 = (BsUiLayer*)BsUi::BsUiGetWindow(hHandle);
	BsAssert(pLayer2);

	RECT rect;
	pLayer1->GetWindowRect(&rect);
	pLayer2->SetWindowPosX(rect.right + 10);

	BsUiListBox* pListBox = (BsUiListBox*)GetWindow(LB_LIST_2);
	BsAssert(pListBox);

	int nWidth(pListBox->GetWindowSize().x), nHeight, nLine, nTempWidth;
	int nWordCount(0);
	for(int i=0; i<pListBox->GetItemCount(); i++)
	{
		char* pStr = pListBox->GetString(i);
		g_pFont->GetTextLengthInfo(nTempWidth, nHeight, nLine, nWordCount, 0, 0, -1, -1, pStr);
		if(nWidth < nTempWidth)
		{
			nWidth = nTempWidth;
		}
	}

	pLayer2->SetWindowWidth(nWidth+20);
	POINT posListBox = {pListBox->GetWindowPos().x + nWidth, 0};
	pListBox->ResizeWindowSize(_CHANGE_SIZE_RIGHT, posListBox);
}

DWORD BsUiSubDebug::ProcMessage(xwMessageToken* pMsgToken)
{
	BsAssert(pMsgToken);

	BsUiCLASS hClass = BsUi::BsUiGetClass(pMsgToken->hWnd);

	switch(pMsgToken->message)
	{
	case XWMSG_KEYDOWN:
		{
			switch(hClass)
			{
			case LB_LIST_2: OnKeyDownListBox(pMsgToken); break;
			}
		}
		break;
	}

	return BsUiWindow::ProcMessage(pMsgToken);
}

void BsUiSubDebug::OnKeyDownListBox(xwMessageToken* pMsgToken)
{
	switch (pMsgToken->lParam)
	{
	case PAD_INPUT_B:
		SetWindowAttr(XWATTR_SHOWWINDOW, false);
		g_BsUiSystem.SetFocusWindowClass(LB_LIST_1);
		break;
	case PAD_INPUT_A:
		{
			switch(m_nOpType)
			{
			case _DEBUG_ITEM_LOAD_MAP:			LoadWorldMap(); break;
			case _DEBUG_ITEM_LOCAL_LANGUAGE:	SetLocalLanguage(); break;
			case _DEBUG_ITEM_SAVE_PLAY:			LoadPlayfile(); break;
			}
		}
		break;
	}
}

void BsUiSubDebug::LoadWorldMap()
{
	BsUiListBox* pListBox = (BsUiListBox*)GetWindow(LB_LIST_2);
	BsAssert(pListBox);

	const char* pStr = pListBox->GetString( pListBox->GetCurSel() );

	g_FC.SetMainState( GAME_STATE_INIT_STAGE, (DWORD)pStr );

	g_MenuHandle->PostMessage(fcMSG_DEBUG_MENU_OUT);
}

void BsUiSubDebug::SetLocalLanguage()
{
	BsUiListBox* pListBox = (BsUiListBox*)GetWindow(LB_LIST_2);
	BsAssert(pListBox);

	g_FcItemManager.Finalize();

	DWORD dwLocal = pListBox->GetItemData( pListBox->GetCurSel() );
	g_LocalLanguage.SetLanguage((GAME_LANGUAGE)dwLocal);
		
	g_FC.ReLoadLocal();

	g_BsUiSystem.SetFocusWindowClass(LB_LIST_1);
	BsUiWindow* pWindow = g_BsUiSystem.GetWindow(SN_DEBUGMENU_1);
	BsAssert(pWindow != NULL);

	((BsUiMainDebug*)pWindow)->UpdateListBox();
	SetWindowAttr(XWATTR_SHOWWINDOW, false);
}


void BsUiSubDebug::LoadPlayfile()
{
	BsUiListBox* pListBox = (BsUiListBox*)GetWindow(LB_LIST_2);
	BsAssert(pListBox);

	const char* pStr = pListBox->GetString( pListBox->GetCurSel() );

	CInputPad::GetInstance().Play( pStr );

	g_MenuHandle->PostMessage(fcMSG_DEBUG_MENU_OUT);
}
#endif