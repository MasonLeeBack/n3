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
#include "FcMenuDef.h"
#include ".\\data\\interface\\BsUiControlID.h"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
CFcMenuDebug::CFcMenuDebug(_FC_MENU_TYPE nType)
: CFcMenuForm(nType)
{
	m_hMenuMain = g_BsUiSystem.OpenLayer(new BsUiMainDebug(this), SN_DEBUGMENU_1, -1, -1);
	((BsUiMainDebug*)BsUi::BsUiGetWindow(m_hMenuMain))->Initailize();

	m_hMenuSub = g_BsUiSystem.OpenLayer(new BsUiSubDebug(), SN_DEBUGMENU_2, -1, -1);
	((BsUiSubDebug*)BsUi::BsUiGetWindow(m_hMenuSub))->Initailize();

	g_BsUiSystem.SetFocusWindow(BsUi::BsUiGetWindow(m_hMenuMain));
}

CFcMenuDebug::~CFcMenuDebug()
{
	if(m_hMenuMain != NULL)
		g_BsUiSystem.CloseLayer(m_hMenuMain, false);

	if(m_hMenuSub != NULL)
		g_BsUiSystem.CloseLayer(m_hMenuSub, false);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
BsUiMainDebug::BsUiMainDebug(CFcMenuForm* pMenu)
: BsUiLayer()
{
	m_pMenu = pMenu;
}

void BsUiMainDebug::Initailize()
{
	BsUiListBox* pListBox = (BsUiListBox*)GetWindow(LB_LIST_1);
	ASSERT(pListBox);

	pListBox->AddString( "Load Map" );
	pListBox->AddString( "Free Cam" );
	pListBox->AddString( "Cam Edit" );
	pListBox->AddString( "Cam Save" );
	pListBox->AddString( "Title" );
}

DWORD BsUiMainDebug::ProcMessage(xwMessageToken* pMsgToken)
{
	ASSERT(pMsgToken);

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
	pWindow = BsUiSystem::GetInstance().GetWindow(SN_DEBUGMENU_2);
	ASSERT(pWindow);
	pWindow->SetWindowAttr(XWATTR_SHOWWINDOW, true);
	BsUiSystem::GetInstance().SetFocusWindowClass(LB_LIST_2);
	( ( BsUiSubDebug * ) pWindow )->InitList();
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
		}
		else
		{
			CamHandle->SetFreeCamMode( true );
			CInputPad::GetInstance().Enable( false );
		}
	}
	g_MenuHandle->PostMessage(0, fcMSG_DEBUG_MENU_OUT);
}

void BsUiMainDebug::CamEditMode()
{
	CameraObjHandle CamHandle;

	CamHandle = CFcBaseObject::GetCameraObjectHandle( 0 );

	if( CamHandle )
		CamHandle->SetEditMode(!CamHandle->IsEditMode());

	g_MenuHandle->PostMessage(0, fcMSG_DEBUG_MENU_OUT);
}

void BsUiMainDebug::CamSave()
{
	CameraObjHandle CamHandle;

	CamHandle = CFcBaseObject::GetCameraObjectHandle( 0 );

	if( CamHandle )
		CamHandle->Save();

	g_MenuHandle->PostMessage(0, fcMSG_DEBUG_MENU_OUT);
}

void BsUiMainDebug::StartTitle()
{
	SetWindowAttr( XWATTR_SHOWWINDOW, false );
	BsUiSystem::GetInstance().SetFocusWindow( NULL );
	
	g_MenuHandle->PostMessage(0, fcMSG_DEBUG_MENU_OUT);
	g_MenuHandle->PostMessage(0, fcMSG_LOGO_START);
}



void BsUiMainDebug::OnKeyDownListBox(xwMessageToken* pMsgToken)
{
	switch (pMsgToken->lParam)
	{
	case PAD_INPUT_A:
		BsUiListBox *pListBox;

		pListBox = ( BsUiListBox * )GetWindow( LB_LIST_1 );
		ASSERT( pListBox );

		switch( pListBox->GetCurSel() )
		{
		case 0:
			OpenWorldLoadList();
			break;
		case 1:
			FreeCamMode();
			break;

		case 2:
			CamEditMode();
			break;

		case 3:
			CamSave();
			break;

		case 4:
			StartTitle();
			break;
		}
		break;
	case PAD_INPUT_B:
		g_MenuHandle->PostMessage(0, fcMSG_DEBUG_MENU_OUT);
		break;
	}
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void BsUiSubDebug::Initailize()
{
	SetWindowAttr(XWATTR_SHOWWINDOW, false);
}

void BsUiSubDebug::InitList()
{
	BsUiListBox* pListBox = (BsUiListBox*)GetWindow(LB_LIST_2);
	ASSERT(pListBox);

	char szFind[_MAX_PATH];
	WIN32_FIND_DATA FindFile;
	HANDLE hFindHandle;

	g_BsKernel.chdir( "map" );
	strcpy( szFind, g_BsKernel.GetCurrentDirectory() );
	strcat( szFind, "*.bsmap" );

	pListBox->ClearItem();
	hFindHandle = FindFirstFile( szFind, &FindFile );
	if( hFindHandle == INVALID_HANDLE_VALUE ){
		return;
	}
	do{
		pListBox->AddString( FindFile.cFileName );
	}while(FindNextFile(hFindHandle, &FindFile));
	FindClose(hFindHandle);

	g_BsKernel.chdir( ".." );
}

DWORD BsUiSubDebug::ProcMessage(xwMessageToken* pMsgToken)
{
	ASSERT(pMsgToken);

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
		BsUiSystem::GetInstance().SetFocusWindowClass(LB_LIST_1);
		break;
	case PAD_INPUT_A:
		BsUiListBox* pListBox = (BsUiListBox*)GetWindow(LB_LIST_2);
		ASSERT(pListBox);

		SetWindowAttr(XWATTR_SHOWWINDOW, false);
		BsUiSystem::GetInstance().SetFocusWindowClass(LB_LIST_1);
		const char* pStr = pListBox->GetString( pListBox->GetCurSel() );
		SetMainState( GAME_STATE_INIT_STAGE, (DWORD)pStr );
		break;
	}
}