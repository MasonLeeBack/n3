#include "stdafx.h"

#include "FcMenuMessageBox.h"
#include "BSuiSystem.h"

#include "BsUiClassIDManager.h"
#include "BsUiFontAttrManager.h"

#include "FcInterfaceManager.h"
#include "FcMessageDef.h"

#include "TextTable.h"
#include "FcSoundManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


CFcMenuMessageBox::CFcMenuMessageBox(_FC_MENU_TYPE nType,
									 int nMsgAskType,
									 DWORD dwMainState,
									 CFcMenuForm* pCmdMenu,
									 BsUiHANDLE hCmdWnd,
									 DWORD dwFocusButton,
									 DWORD dwValue)
: CFcMenuForm(nType)
{
	m_pMessageBox = new CFcMenuMessageBoxLayer(this);
	AddUiLayer(m_pMessageBox, SN_MessageBox, -1, -1);
		
	m_nMsgAskType = nMsgAskType;
	m_dwMainState = dwMainState;
	m_pCmdMenu = pCmdMenu;
	m_hCmdWnd = hCmdWnd;
	m_dwFocusButton = dwFocusButton;

	m_pMessageBox->Initialize(nMsgAskType, (int)dwFocusButton, dwValue);
	g_BsUiSystem.SetFocusWindow(m_pMessageBox);
}

void CFcMenuMessageBox::RenderProcess()
{
	switch(GetStatus())
	{
	case _MS_OPEN:		SetStatus(_MS_OPENING); break;
	case _MS_OPENING:	SetStatus(_MS_NORMAL); break;
	case _MS_NORMAL:	break;
	case _MS_CLOSING:
		{
			int nTick = g_BsUiSystem.GetMenuTick() - m_nFormTick;
			if(nTick >= _TICK_END_MSG_CLOSING)
			{
				SetStatus(_MS_CLOSE);
				g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_END,
					(DWORD)this,
					(DWORD)m_dwMainState,
					(DWORD)m_pCmdMenu,
					(DWORD)m_nMsgAskType,
					(DWORD)m_hCmdWnd,
					m_pMessageBox->GetResponse());
			}
			break;
		}
	case _MS_CLOSE: break;
	}
}

//-----------------------------------------------------------------------------------------------------
CFcMenuMessageBoxLayer::CFcMenuMessageBoxLayer(CFcMenuForm* pMenu)
: BsUiLayer()
{
	m_pMenu = pMenu;
}

void CFcMenuMessageBoxLayer::Initialize(int nMsgAskType, int nFocusButton, DWORD dwValue)
{	
	char cText[256];
	switch(nMsgAskType)
	{
	case fcMT_LibOpenItem:
		{
			char cTempText[256];
			g_TextTable->GetText(g_FcMsgDef[nMsgAskType].nTextId, cTempText, _countof(cTempText));
#ifdef _XBOX
			sprintf_s(cText, cTempText, (int)dwValue);
#else
			sprintf(cText, cTempText);
#endif
			break;
		}
	default:
		{
			g_TextTable->GetText(g_FcMsgDef[nMsgAskType].nTextId, cText, _countof(cText));
		}
	}

	BsUiText* pText = (BsUiText*)GetWindow(TT_MsgText);
	pText->SetItemText(cText);
	
	SetButton(nMsgAskType, nFocusButton);
	m_dwResponse = ID_OK;

	BsUiWindow* pWindow = GetWindow(TT_MsgHelp);
	switch(m_nButtonCount)
	{
	case 0:
		{
			pWindow->SetWindowAttr(XWATTR_SHOWWINDOW, false);
			break;
		}
	case 1:
		{	
			pWindow->SetItemTextID(_TEX_HELP_MESSAGE_FOR_ONE_ITEM);
			break;
		}
	}
}


void CFcMenuMessageBoxLayer::SetButton(int nMsgAskType, int nFocusButton)
{
	BsUiButton* pButton1 = (BsUiButton*)GetWindow(BT_MsgBT1);
	BsUiButton* pButton2 = (BsUiButton*)GetWindow(BT_MsgBT2);
	BsUiButton* pButton3 = (BsUiButton*)GetWindow(BT_MsgBT3);

	BsUiItemInfo item;
	switch(g_FcMsgDef[nMsgAskType].nAskId)
	{
	case MSG_ABORTRETRYIGNORE:
		{	
			pButton1->SetItemData(ID_ABORT);
			pButton1->SetItemTextID(_TEX_ABORT);
			pButton2->SetItemData(ID_RETRY);
			pButton2->SetItemTextID(_TEX_RETRY);
			pButton3->SetItemData(ID_IGNORE);
			pButton3->SetItemTextID(_TEX_IGNORE);

			m_nButtonCount = 3;
			break;
		}
	case MSG_OK:
		{
			pButton1->SetItemData(ID_OK);
			pButton1->SetItemTextID(_TEX_OK);

			pButton2->SetWindowAttr(XWATTR_SHOWWINDOW, false);
			pButton2->SetWindowAttr(XWATTR_DISABLE, true);
			pButton3->SetWindowAttr(XWATTR_SHOWWINDOW, false);
			pButton3->SetWindowAttr(XWATTR_DISABLE, true);
			m_nButtonCount = 1;
			break;
		}
	case MSG_OKCANCEL:
		{	
			pButton1->SetItemData(ID_OK);
			pButton1->SetItemTextID(_TEX_OK);
			pButton2->SetItemData(ID_CANCEL);
			pButton2->SetItemTextID(_TEX_CANCEL);

			pButton3->SetWindowAttr(XWATTR_SHOWWINDOW, false);
			pButton3->SetWindowAttr(XWATTR_DISABLE, true);
			m_nButtonCount = 2;
			break;
		}
	case MSG_RETRYCANCEL:
		{
			pButton1->SetItemData(ID_RETRY);
			pButton1->SetItemTextID(_TEX_RETRY);
			pButton2->SetItemData(ID_CANCEL);
			pButton2->SetItemTextID(_TEX_CANCEL);

			pButton3->SetWindowAttr(XWATTR_SHOWWINDOW, false);
			pButton3->SetWindowAttr(XWATTR_DISABLE, true);
			m_nButtonCount = 2;
			break;
		}
	case MSG_YESNO:
		{	
			pButton1->SetItemData(ID_YES);
			pButton1->SetItemTextID(_TEX_YES);
			pButton2->SetItemData(ID_NO);
			pButton2->SetItemTextID(_TEX_NO);

			pButton3->SetWindowAttr(XWATTR_SHOWWINDOW, false);
			pButton3->SetWindowAttr(XWATTR_DISABLE, true);
			m_nButtonCount = 2;
			break;
		}
	case MSG_YESNOCANCEL:
		{	
			pButton1->SetItemData(ID_YES);
			pButton1->SetItemTextID(_TEX_YES);
			pButton2->SetItemData(ID_NO);
			pButton2->SetItemTextID(_TEX_NO);
			pButton2->SetItemData(ID_CANCEL);
			pButton2->SetItemTextID(_TEX_CANCEL);

			m_nButtonCount = 3;
			break;
		}
	default:
		pButton1->SetWindowAttr(XWATTR_SHOWWINDOW, false);
		pButton1->SetWindowAttr(XWATTR_DISABLE, true);
		pButton2->SetWindowAttr(XWATTR_SHOWWINDOW, false);
		pButton2->SetWindowAttr(XWATTR_DISABLE, true);
		pButton3->SetWindowAttr(XWATTR_SHOWWINDOW, false);
		pButton3->SetWindowAttr(XWATTR_DISABLE, true);
		m_nButtonCount = 0;

		break;
	}
	
	//nFocusButton에 따라 focus받는 것을 처리 하였는데...
	//일단은 선택 메세지에서 취소에 우선을 두는 처리로 바뀌었다.
	/*
	BsUiCLASS hClass;
	switch(nFocusButton)
	{
	case 0: hClass = BT_MsgBT1; break;
	case 1: hClass = BT_MsgBT2; break;
	case 2: hClass = BT_MsgBT3; break;
	default: hClass = BT_MsgBT1;
	}
	*/

	BsAssert(m_nButtonCount >= 0 && m_nButtonCount <= 3);
	BsUiCLASS hClass;
	switch(m_nButtonCount - 1)
	{
	case 0: hClass = BT_MsgBT1; break;
	case 1: hClass = BT_MsgBT2; break;
	case 2: hClass = BT_MsgBT3; break;
	default: hClass = BT_MsgBT1;
	}

	SetFocusWindowClass(hClass);
}


DWORD CFcMenuMessageBoxLayer::ProcMessage(xwMessageToken* pMsgToken)
{
	BsAssert(pMsgToken);

	BsUiCLASS hClass = BsUi::BsUiGetClass(pMsgToken->hWnd);

	switch(pMsgToken->message)
	{
	case XWMSG_KEYDOWN:
		{	
			if(m_nButtonCount > 0)
			{
				switch (pMsgToken->lParam)
				{
				case MENU_INPUT_B:
					{	
						break;
					}
				case MENU_INPUT_UP:
					{
						BsUiWindow* pPrev = SetFocusPrevChileWindow(pMsgToken->hWnd, true);
						if(pPrev != NULL && pPrev->GetHWnd() != pMsgToken->hWnd){
							g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CURSOR");
						}
						break;
					}	
				case MENU_INPUT_DOWN:
					{
						BsUiWindow* pNext = SetFocusNextChileWindow(pMsgToken->hWnd, true);
						if(pNext != NULL && pNext->GetHWnd() != pMsgToken->hWnd){
							g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CURSOR");
						}
						break;
					}
				case MENU_INPUT_A:
					{	
						switch(hClass)
						{
						case BT_MsgBT1:	m_dwResponse = ((BsUiButton*)GetWindow(BT_MsgBT1))->GetItemData(); break;
						case BT_MsgBT2:	m_dwResponse = ((BsUiButton*)GetWindow(BT_MsgBT2))->GetItemData(); break;
						case BT_MsgBT3:	m_dwResponse = ((BsUiButton*)GetWindow(BT_MsgBT3))->GetItemData(); break;
						//default: BsAssert(0);
						}

						switch(m_dwResponse)
						{
						case ID_OK:
						case ID_YES:
							{
								/*
								BsUiWindow* pFocus = g_BsUiSystem.GetFocusWindow();
								if(pFocus != NULL){	
									g_MenuHandle->PostMessage(fcMSG_CLICK_FX_EFFECT, (DWORD)pFocus);
								}
								*/

								g_pSoundManager->PlaySystemSound(SB_COMMON,"SYS_OK");
								break;
							}

						case ID_CANCEL:
						case ID_NO:
						case ID_ABORT:
						case ID_IGNORE:
						case ID_RETRY:
						case ID_ASYNC:
						case ID_TIMEOUT:	
							g_pSoundManager->PlaySystemSound(SB_COMMON,"SYS_CANCEL");
							break;
						}

						m_pMenu->SetStatus(_MS_CLOSING, false);
						break;
					}
				}
			}
			break;
		}
	case XWMSG_BN_CLICKED:
		{	
			;
		}
	}

	return BsUiWindow::ProcMessage(pMsgToken);
}