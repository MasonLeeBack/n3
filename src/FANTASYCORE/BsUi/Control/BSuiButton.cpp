#include "stdafx.h"

#include "BsUiButton.h"

#include "BsUiSystem.h"

#include "BsUiMenuInput.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

BsUiButton::BsUiButton() : BsUiWindow()
{	
	m_nButtonMode = BsUiBUTTON_NORMAL;
	m_bButtonDown = false;
}

BsUiButton::BsUiButton(BsUiCLASS hClass, POINT pos, BsUiWindow* pParent) : BsUiWindow()
{
	BsAssert(hClass != 0);
	
	m_nButtonMode = BsUiBUTTON_NORMAL;
	m_bButtonDown = false;

	m_nGroupID = -1;

	BsUiWindow::Create(BsUiTYPE_BUTTON,
		hClass, pos.x, pos.y,
		100, 30,
		XWATTR_SHOWWINDOW | XWATTR_MOVABLE,
		pParent);
}

void BsUiButton::Release()
{	
}

bool BsUiButton::Create(BsUiCLASS hClass, int x, int y, BsUiWindow* pParent)
{
	BsAssert(hClass != 0);

	BsUiButtonItem* pButtonItem = static_cast <BsUiButtonItem* > (g_BsUiSystem.GetLoader()->GetClassInstance(hClass));
	BsAssert(pButtonItem);

	m_nGroupID = pButtonItem->nGroupID;
	m_bButtonDown = pButtonItem->nCheck ? true : false;
	
	return BsUiWindow::Create(pButtonItem->nType,
		hClass, x, y,
		pButtonItem->size.cx, pButtonItem->size.cy,
		pButtonItem->nAttr, pParent);
}


DWORD BsUiButton::ProcMessage(xwMessageToken* pMsgToken)
{
	BsAssert(pMsgToken);

	BsUiCLASS hClass = BsUi::BsUiGetClass(pMsgToken->hWnd);
	BsAssert(hClass);
	
	switch (pMsgToken->message)
	{
	case XWMSG_KEYDOWN:
		{
			BsAssert(this->GetHWnd() == pMsgToken->hWnd);
			g_BsUiSystem.NotifyMessage(m_hParentWnd, this->GetHWnd(), XWMSG_KEYDOWN, 0, pMsgToken->lParam);
		}
		break;
	case XWMSG_LBUTTONDOWN:
		{
			BsUiSystem::SetGrabWindow(this);
		}
		break;
	case XWMSG_LBUTTONUP:
		{
			if (BsUiSystem::GetGrabWindow() == this)
				Clicked();
		}
		break;
	}

	return BsUiWindow::ProcMessage(pMsgToken);
}

void BsUiButton::Clicked()
{
	BsUiSystem::SetGrabWindow(NULL);

	switch (m_nButtonMode)
	{
	case BsUiBUTTON_NORMAL:
		{
			//TRACE("Click\n");
			g_BsUiSystem.NotifyMessage(m_hParentWnd, this->GetHWnd(), XWMSG_BN_CLICKED);
		}
		break;
	case BsUiBUTTON_CHECKBUTTON:
		{
			// 이미 눌린 상태면 반응없음
			if (m_bButtonDown)
				break;

			g_BsUiSystem.SendMessage(m_hParentWnd, XWMSG_CHECKBUTTON, this->GetHWnd(), m_nGroupID);
			g_BsUiSystem.NotifyMessage(m_hParentWnd, this->GetHWnd(), XWMSG_BN_CLICKED);
		}
		break;
	}
}

