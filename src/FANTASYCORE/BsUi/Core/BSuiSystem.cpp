#include "stdafx.h"

#include "BsUiSystem.h"

#include "BsUiLayer.h"
#include "BsUiButton.h"
#include "BsUiListBox.h"
#include "BsUiListCtrl.h"
#include "BsUiSlider.h"
#include "BSuiText.h"
#include "BSuiImageCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

BsUiWindow* BsUiSystem::m_pGrabbed = NULL;			// 마우스를 잡는다.
BsUiWindow* BsUiSystem::m_pFocus = NULL;				// 포커싱된 윈도우 객체

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

BsUiSystem::BsUiSystem()
{
	m_pSkin = NULL;
	m_pView = NULL;
	m_pLoader = NULL;

	m_nMenuTick = 0;
}

bool BsUiSystem::Create(BsUiSkin* pSkin, BsUiView* pView, BsUiLoader* pLoader, int nScreenWidth, int nScreenHeight)
{
	BsAssert(pSkin);
	BsAssert(pView);
	BsAssert(pLoader);

	m_pSkin = pSkin;
	m_pView = pView;
	m_pLoader = pLoader;
	m_nScreenWidth = nScreenWidth;
	m_nScreenHeight = nScreenHeight;

	return true;
}

bool BsUiSystem::Create(BsUiSkin* pSkin, BsUiView* pView, BsUiLoader* pLoader, BsUiStore* pStore, int nScreenWidth, int nScreenHeight)
{
	BsAssert(pStore);

	m_pStore = pStore;

	return Create(pSkin, pView, pLoader, nScreenWidth, nScreenHeight);
}

void BsUiSystem::Release()
{
	RemoveAllWindow();

	if(m_pSkin != NULL)
		delete m_pSkin;

	if(m_pView != NULL)
		delete m_pView;

	if(m_pLoader != NULL)
		delete m_pLoader;

	if(m_pStore != NULL)
		delete m_pStore;
}

BsUiHANDLE BsUiSystem::OpenLayer(BsUiCLASS hClass, int x, int y)
{
	BsUiLayer* pLayer =  g_BsUiSystem.GetLoader()->OpenLayer(hClass, x, y);

	if (pLayer == NULL)
		return NULL;

	AddWindow(pLayer);
	return pLayer->GetHWnd();
}

BsUiHANDLE BsUiSystem::OpenLayer(BsUiLayer* pLayer, BsUiCLASS hClass, int x, int y)
{
	g_BsUiSystem.GetLoader()->OpenLayer(pLayer, hClass, x, y);

	if (pLayer == NULL)
		return NULL;

	AddWindow(pLayer);
	return pLayer->GetHWnd();
}

void BsUiSystem::CloseLayer(BsUiHANDLE hWnd, bool bSavePos)
{
	BsUiWindow* pWindow = BsUi::BsUiGetWindow(hWnd);
	if(pWindow == NULL)
	{
		BsAssert(pWindow);
		return;
	}

	RemoveWindow(pWindow);

	if (bSavePos)
	{
		// save position.
	}

	// Remove any messages that reference pWindow
	BsUiMSGTOKENDEQUEITOR itr = m_msgQue.begin();
	while( itr != m_msgQue.end())
	{
		xwMessageToken* pMsg = (*itr);

		if(pMsg->hWnd == pWindow->GetHWnd())
		{
			itr = m_msgQue.erase(itr);
		}
		else
			++itr;
	}

	delete pWindow;
	pWindow = NULL;
}

bool BsUiSystem::LoadDef(const char* pFilename)
{
	BsAssert(pFilename);
	return m_pLoader->Create(pFilename);
}

void BsUiSystem::Update()
{
	while (!m_msgQue.empty())
	{
		xwMessageToken* pMsg = m_msgQue.front();
		BsAssert(pMsg);

		m_msgQue.pop_front();

		DispatchMessage(pMsg);
		delete pMsg;
	}

	if(m_pFocus == NULL)
	{
		BsUiWINDOWLISTITOR itr;
		BsUiWINDOWLISTITOR itr_begin = m_windows.begin();
		for(itr = itr_begin; itr!=m_windows.end();itr++)
		{
			BsUiWindow* pWindow = (*itr);
			if(pWindow->IsEnableWindow() == false){
				continue;
			}

			SetFocusWindow(pWindow);
			break;
		}
	}

	m_nMenuTick++;
}

void BsUiSystem::Draw()
{
	BsUiWINDOWLIST::reverse_iterator itr = m_windows.rbegin();

	while (itr != m_windows.rend())
	{
		BsUiWindow* pWnd = (*itr);
		BsAssert(pWnd);

		pWnd->Draw();
		++itr;
	}
}

DWORD BsUiSystem::DispatchMessage(xwMessageToken* pMsg)
{
	BsAssert(pMsg);

	// msg에 시간을 지정한다.
	pMsg->time = GetMenuTick();
	
	// skin에 우선적으로 물어보고
	if (m_pSkin->HookMessage(pMsg))
		return 0L;

	// 실제 윈도우로 간다.
	BsUiWindow* pWindow = BsUi::BsUiGetWindow(pMsg->hWnd);

	if (pWindow)
		return pWindow->ProcMessage(pMsg);
	else
	{
		switch (pMsg->message)
		{
		case XWMSG_LBUTTONDOWN:
		case XWMSG_RBUTTONDOWN:
		case XWMSG_LBUTTONDBLCLK:
		case XWMSG_RBUTTONDBLCLK:
			SetFocusWindow(NULL);
			break;
		case XWMSG_LBUTTONUP:
			{
				// 이것 별로 보기 좋지 않다. BsUiWindow에도 동일한 루틴이 있다.
				if (m_pSkin)
				{
					if (m_pSkin->IsDraggingObject())
					{
						// Drop에 따른 정보를 추가한 뒤에 Drop시킨다.
						{
							BsUiDragDropInfo dd;

							m_pSkin->GetDragDropInfo(&dd);

							memcpy(&dd.msgButtonUp, pMsg, sizeof(xwMessageToken));
							dd.hTargetClass = 0;
							dd.hTargetHwnd = 0;

							m_pSkin->SetDragDropInfo(&dd);
						}

						m_pSkin->DropObject(NULL, pMsg);
					}

					m_pSkin->ClearDragDropInfo();
				}
			}
			break;
		}
	}

	return 0L;
}

// 임시로 들어오는 모든 명령을 dispatch 시킨다.
DWORD BsUiSystem::TranslateMessage(DWORD message, WPARAM wParam, LPARAM lParam, POINT* pPt)
{
	switch (message)
	{
	case XWMSG_KEYDOWN:
		{
			BsUiHANDLE hWnd = (GetFocusWindow()) ? GetFocusWindow()->GetHWnd() : 0;
			PostMessage(hWnd, message, wParam, lParam);
			return 0L;
			/*
			xwMessageToken msg;

			msg.hWnd = (GetFocusWindow()) ? GetFocusWindow()->GetHWnd() : 0;
			msg.message = message;
			msg.wParam = (DWORD)wParam;
			msg.lParam = (DWORD)lParam;

			m_msgQue.push_back(pNewMsg);
			return DispatchMessage(&msg);
			*/
		}
		break;
	default:
		{
			BsUiWindow* pWnd = GetWindow(pPt);

			xwMessageToken msg;

			msg.hWnd = (pWnd) ? pWnd->GetHWnd() : 0;
			msg.message = message;
			msg.wParam = (DWORD)wParam;
			msg.lParam = (DWORD)lParam;
			msg.pt = *pPt;

			return DispatchMessage(&msg);
		}
		break;
	}

	return 0L;
}

DWORD BsUiSystem::SendMessage(BsUiHANDLE hWnd, DWORD message, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	xwMessageToken msg;

	msg.hWnd = hWnd;
	msg.message = message;
	msg.wParam = (DWORD)wParam;
	msg.lParam = (DWORD)lParam;

	return DispatchMessage(&msg);
}

void BsUiSystem::PostMessage(BsUiHANDLE hWnd, DWORD message, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	xwMessageToken* pNewMsg = new xwMessageToken;
	BsAssert(pNewMsg);

	pNewMsg->hWnd = hWnd;
	pNewMsg->message = message;
	pNewMsg->wParam = (DWORD)wParam;
	pNewMsg->lParam = (DWORD)lParam;
	
	m_msgQue.push_back(pNewMsg);
}

DWORD BsUiSystem::NotifyMessage(BsUiHANDLE hNotifyWnd, BsUiHANDLE hWnd, DWORD message, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	xwMessageToken msg;

	msg.hWnd = hWnd;
	msg.message = message;
	msg.wParam = (DWORD)wParam;
	msg.lParam = (DWORD)lParam;
	
	// skin에 우선적으로 물어보고
	if (m_pSkin->HookMessage(&msg))
		return 0L;

	// 통지받을 윈도우로 간다.
	BsUiWindow* pNotifyWindow = BsUi::BsUiGetWindow(hNotifyWnd);

	if (pNotifyWindow)
	{
		return pNotifyWindow->ProcMessage(&msg);
	}

	return 0L;
}

bool BsUiSystem::AddWindow(BsUiWindow* pWindow)
{
	BsAssert(pWindow);
	BsAssert(pWindow->IsWindow());

	m_windows.push_front(pWindow);		// 맨 위로 올린다.

	pWindow->PostMessage(XWMSG_OPENWINDOW);
	return true;
}

void BsUiSystem::RemoveWindow(BsUiWindow* pWindow)
{
	BsAssert(pWindow);
	BsAssert(pWindow->IsWindow());

	pWindow->SendMessage(XWMSG_CLOSEWINDOW);

	BsUiWINDOWLISTITOR itr = find(m_windows.begin(), m_windows.end(), pWindow);
	BsAssert(itr != m_windows.end());

	m_windows.erase(itr);
}

bool BsUiSystem::AddChildWindow(BsUiWindow* pParentWnd, BsUiWindow* pWindow)
{
	pWindow->PostMessage(XWMSG_OPENWINDOW);

	return true;
}

bool BsUiSystem::RemoveChildWindow(BsUiWindow* pParentWnd, BsUiWindow* pWindow)
{
	pWindow->SendMessage(XWMSG_CLOSEWINDOW);

	return true;
}

void BsUiSystem::RemoveAllWindow(void)
{
	while (!m_msgQue.empty())
	{
		xwMessageToken* pMsg = m_msgQue.front();
		BsAssert(pMsg);

		m_msgQue.pop_front();
		delete pMsg;
	}
	SetFocusWindow(NULL);
	SetGrabWindow(NULL);

	BsUiWINDOWLISTITOR itr;
	BsUiWINDOWLISTITOR itr_begin = m_windows.begin();
	for(itr = itr_begin; itr!=m_windows.end();itr++)
	{
		delete (*itr);
	}
	m_windows.clear();
}

bool BsUiSystem::PushModal(BsUiWindow* pWindow)
{
	BsUiWINDOWDEQUEITOR itr = m_modalStack.begin();

	while (itr != m_modalStack.end())
	{
		if ((*itr) == pWindow)
			return false;
	}

	m_modalStack.push_back(pWindow);
	return true;
}

bool BsUiSystem::PopModal(BsUiWindow* pWindow, int nResult /*= 0*/, void* pRetValue /*= NULL*/)
{
	if (m_modalStack.empty())
		return false;
	if (m_modalStack.back() != pWindow)
		return false;

	m_modalStack.pop_back();
	return true;
}

void BsUiSystem::MakeTop(BsUiWindow* pWindow)
{
	BsUiWINDOWLISTITOR itr = find(m_windows.begin(), m_windows.end(), pWindow);

	if (itr == m_windows.end())
		return;

	DWORD nAttr = pWindow->GetWindowAttr();

	if (nAttr & XWATTR_ALWAYSBOTTOM)
		return;
	if (m_windows.size() == 1)
		return;

	// 일단 빼서
	itr = m_windows.erase(itr);

	// 맨 앞에서부터 조건이 맞을 때까지 넣어둔다.
	itr = m_windows.begin();

	do {
		BsUiWindow* pComp = (*itr);
		BsAssert(pComp);

		DWORD nAttr2 = pComp->GetWindowAttr();

		if ((nAttr2 & XWATTR_ALWAYSTOP) && (nAttr & XWATTR_ALWAYSTOP) == false)
			++itr;
		else
		{
			m_windows.insert(itr, 1, pWindow);
			return;
		}
	} while (itr != m_windows.end());
}

BsUiWindow* BsUiSystem::GetWindow(POINT* pPt)
{
	if (m_pGrabbed)
		return m_pGrabbed;

	BsUiWINDOWLISTITOR itr = m_windows.begin();
	while (itr != m_windows.end())
	{
		BsUiWindow* pWindow = (*itr);
		BsAssert(pWindow);

		BsUiWindow* pOn = pWindow->GetWindowOnPos(pPt);
		if (pOn)
			return pOn;

		++itr;
	}

	return NULL;
}

BsUiWindow*	BsUiSystem::GetWindow(BsUiCLASS hClass)
{
	BsUiWINDOWLISTITOR itr = m_windows.begin();
	while (itr != m_windows.end())
	{
		BsUiWindow* pWindow = (*itr);
		BsAssert(pWindow);

		BsUiWindow* pOn = pWindow->GetWindow(hClass);
		if (pOn)
			return pOn;

		++itr;
	}

	return NULL;
}

void BsUiSystem::SetFocusWindow(BsUiWindow* pWindow)
{
	if (m_pFocus == pWindow)
		return;

#ifndef _BSUI_TOOL
	if(pWindow != NULL && pWindow->IsEnableWindow() == false)
		return;
#endif

	if (m_pFocus)
	{
		m_pFocus->SendMessage(XWMSG_KILLFOCUS);
        //m_pFocus->SendMessage(m_pFocus->GetHWnd(), XWMSG_KILLFOCUS);
	}

	if(pWindow == NULL)
	{
		m_pFocus = NULL;
		return;
	}

#ifdef _BSUI_TOOL
	m_pFocus = pWindow;
#else
	if(pWindow->GetType() != BsUiTYPE_LAYER)
	{
		m_pFocus = pWindow;
		pWindow->GetParentWindow()->SetChildWndFocus(pWindow);
	}
	else
	{
		BsUiWindow* pChild = pWindow->GetChildWndFocus();
		if(pChild != NULL){
			m_pFocus = pChild;
		}
		else
		{
			BsUiWindow* pChild = pWindow->GetEnableChildWindow();
			if(pChild != NULL){
				m_pFocus = pChild;
			}
			else{
				m_pFocus = pWindow;
			}
		}
	}

#endif
	if (m_pFocus)
	{
        m_pFocus->SendMessage(XWMSG_SETFOCUS);
        m_pFocus->SendMessage(XWMSG_MAKETOP);
	}
}

void BsUiSystem::SetFocusWindowClass(BsUiCLASS hClass)
{
	BsUiWindow* pWindow = GetWindow(hClass);
	SetFocusWindow(pWindow);
}

BsUiWindow* BsUiSystem::GetMsgReceiver()
{
	BsUiWindow* pWindow = BsUiSystem::GetGrabWindow();

	if (pWindow)
		return pWindow;

	pWindow = BsUiSystem::GetFocusWindow();

	if (pWindow)
		return pWindow;

	return NULL;
}

/*
bool BsUiSystem::SendEvent(int nCode, int nParam1, int nParam2, int nParam3)
{
	BsAssert(m_pMainWnd);

	BsUiWindow* pWindow = m_pMainWnd->GetMsgReceiver();

	if (pWindow == m_pMainWnd)			// 메인 = 게임화면으로 온 메시지는 게임에 위임한다.
		return false;

	// 전처리
	static DWORD tickcount = 0;
	static int tickgap = 800;

	switch (nCode)
	{
	case XBOXMSG_EVENT_ANALOG1:
		{
			KEY_EVENT_PARAM* pKeyParam = (KEY_EVENT_PARAM* ) nParam1;

			int& x = pKeyParam->nPosX;
			int& y = pKeyParam->nPosY;

//			debug("%d, %d\n", x, y);

			if (abs(x) == 32767 || abs(y) == 32767)
			{
				if (timeGetTime() - tickcount > tickgap)
				{
					if (x >= 32767)		pWindow->SendMessage(pWindow->GetHWnd(), XWMSG_MOVERIGHT);
					if (x <= -32767)	pWindow->SendMessage(pWindow->GetHWnd(), XWMSG_MOVELEFT);
					if (y >= 32767)		pWindow->SendMessage(pWindow->GetHWnd(), XWMSG_MOVEUP);
					if (y <= -32767)	pWindow->SendMessage(pWindow->GetHWnd(), XWMSG_MOVEDOWN);

					tickcount = timeGetTime();
					tickgap = 400;
				}
			}
			else
			{
				tickcount = 0;
				tickgap = 800;
			}
		}
		break;
	case XBOXMSG_EVENT_ANALOG1_RELEASE:
		{
			tickcount = 0;
			tickgap = 800;
		}
		break;
	case XBOXMSG_EVENT_ANALOG2:
		{
		}
		break;
	case XBOXMSG_EVENT_ANALOG2_RELEASE:
		{
		}
		break;
	case XBOXMSG_EVENT_A:
		{
			pWindow->SendMessage(pWindow->GetHWnd(), XWMSG_ENTERKEY);
		}
		break;
	case XBOXMSG_EVENT_A_RELEASE:
		break;
	case XBOXMSG_EVENT_B:
		{
			pWindow->SendMessage(pWindow->GetHWnd(), XWMSG_CANCELKEY);
		}
		break;
	case XBOXMSG_EVENT_B_RELEASE:
		break;
	case XBOXMSG_EVENT_Y:
		break;
	case XBOXMSG_EVENT_Y_RELEASE:
		break;
	case XBOXMSG_EVENT_UP:
		break;
	case XBOXMSG_EVENT_UP_RELEASE:
		break;
	case XBOXMSG_EVENT_DOWN:
		break;
	case XBOXMSG_EVENT_DOWN_RELEASE:
		break;
	case XBOXMSG_EVENT_LEFT:
		break;
	case XBOXMSG_EVENT_LEFT_RELEASE:
		break;
	case XBOXMSG_EVENT_RIGHT:
		break;
	case XBOXMSG_EVENT_RIGHT_RELEASE:
		break;
	case XBOXMSG_EVENT_LTRIGGER:
		break;
	case XBOXMSG_EVENT_LTRIGGER_RELEASE:
		break;
	case XBOXMSG_EVENT_RTRIGGER:
		break;
	case XBOXMSG_EVENT_RTRIGGER_RELEASE:
		break;
	}

	return true;
}
*/

void BsUiSystem::SetSelectedWindows(RECT SelectedBox)
{
	m_SelectedWindows.clear();
	RECT rect;

	BsUiWINDOWLISTITOR itr = m_windows.begin();
	while (itr != m_windows.end())
	{
		BsUiWindow* pWindow = (*itr);
		BsAssert(pWindow);

		pWindow->GetWindowRect(&rect);
		if(IsWindowInSelectedBox(SelectedBox, rect))
		{
			m_SelectedWindows.push_back(pWindow);

			BsUiWINDOWLIST	childList = pWindow->GetChildWindowList();
			BsUiWINDOWLISTITOR itr_child = childList.begin();
			while(itr_child != childList.end())
			{
				BsUiWindow* pChild = (*itr_child);
				m_SelectedWindows.push_back(pChild);

				++itr_child;
			}
		}
		++itr;
	}

	if(m_SelectedWindows.size() == 0)
	{
		itr = m_windows.begin();
		while (itr != m_windows.end())
		{
			BsUiWindow* pWindow = (*itr);
			BsAssert(pWindow);

			BsUiWINDOWLIST	childList = pWindow->GetChildWindowList();
			BsUiWINDOWLISTITOR itr_child = childList.begin();
			while(itr_child != childList.end())
			{
				BsUiWindow* pChild = (*itr_child);
				pChild->GetWindowRect(&rect);
				if(IsWindowInSelectedBox(SelectedBox, rect))
					m_SelectedWindows.push_back(pChild);

				++itr_child;
			}
			if(m_SelectedWindows.size() >0)
			{
				break;
			}

			++itr;
		}
	}

	if(m_SelectedWindows.size() > 0)
	{
		BsUiWindow* pWindow = (BsUiWindow*)(*m_SelectedWindows.begin());
		SetFocusWindow(pWindow);
	}
}

bool BsUiSystem::SetSelectedWindow(BsUiWindow* pWindow)
{
	if(pWindow == NULL)
	{
        BsAssert(pWindow);
		return false;
	}

	m_SelectedWindows.push_back(pWindow);
	SetFocusWindow(pWindow);

	return true;
}

bool BsUiSystem::RemoveSelectedWindow(BsUiWindow* pWindow)
{
	if(pWindow == NULL)
	{
		BsAssert(pWindow);
		return false;
	}

	BsUiWINDOWLISTITOR itr = find(m_SelectedWindows.begin(), m_SelectedWindows.end(), pWindow);
	BsAssert(itr != m_SelectedWindows.end());

	m_SelectedWindows.erase(itr);

	return true;
}

bool BsUiSystem::EditSelectedWindow(BsUiWindow* pWindow)
{
	if(pWindow == NULL)
	{
		BsAssert(pWindow);
		return false;
	}

	bool bLayerMode = false;
	int nCount = 0;
	BsUiWINDOWLISTITOR itr = m_SelectedWindows.begin();
	while(itr != m_SelectedWindows.end())
	{
		BsUiWindow* pSelWindow = (*itr);
		if(pSelWindow->GetParentWindow() == NULL)
		{
			bLayerMode = true;
			nCount++;
		}
		++itr;
	}

	if(bLayerMode == true && pWindow->GetParentWindow() != NULL)
	{
		if(nCount > 1)
			return false;
		else
		{
			BsUiWINDOWLISTITOR itr_find = find(m_SelectedWindows.begin(), m_SelectedWindows.end(), pWindow->GetParentWindow());
			if(itr_find == m_SelectedWindows.end())
				return false;
		}
	}
		
	
	BsUiWINDOWLISTITOR itr_Next;
	itr = m_SelectedWindows.begin();
	while(itr != m_SelectedWindows.end())
	{
		BsUiWindow* pSelWindow = (*itr);
		if(pWindow == pSelWindow)
		{
			itr_Next = itr;
			itr_Next++;
			break;
		}
		++itr;
	}
		
	if(itr == m_SelectedWindows.end())
	{	
		if(m_pFocus == NULL || m_SelectedWindows.size() == 0 || pWindow->GetParentWindow() == NULL)
		{
			SetFocusWindow(pWindow);
		}

		m_SelectedWindows.push_back(pWindow);

		BsUiWINDOWLIST childList = pWindow->GetChildWindowList();
		BsUiWINDOWLISTITOR itr = childList.begin();
		while(itr != childList.end())
		{
			BsUiWindow* pChild = (*itr);
			BsUiWINDOWLISTITOR itr_find = find(m_SelectedWindows.begin(), m_SelectedWindows.end(), pChild);
			if(itr_find == m_SelectedWindows.end())
			{
				m_SelectedWindows.push_back(pChild);
			}

			++itr;
		}
	}
	else
	{
		bool bChangeFocus = false;
		if(m_pFocus == pWindow)
		{	
			if(itr_Next == m_SelectedWindows.end())
			{
				itr_Next = m_SelectedWindows.begin();
			}
			
			BsUiWindow* pNextWindow = (BsUiWindow*)(*itr_Next);
			SetFocusWindow(pNextWindow);
		}

		m_SelectedWindows.erase(itr);

		BsUiWINDOWLIST childList = pWindow->GetChildWindowList();
		BsUiWINDOWLISTITOR itr = childList.begin();
		while(itr != childList.end())
		{
			BsUiWindow* pChild = (*itr);
			BsUiWINDOWLISTITOR itr_find = find(m_SelectedWindows.begin(), m_SelectedWindows.end(), pChild);
			if(itr_find != m_SelectedWindows.end())
			{
				m_SelectedWindows.erase(itr_find);
			}

			++itr;
		}

		if(m_SelectedWindows.size() == 0)
		{
			SetFocusWindow(NULL);
		}
		else
		{
			BsUiWINDOWLISTITOR itr_find = find(m_SelectedWindows.begin(), m_SelectedWindows.end(), m_pFocus);
			if(itr_find == m_SelectedWindows.end())
			{
				SetFocusWindow(*m_SelectedWindows.begin());
			}
		}
	}

	return true;
}

bool BsUiSystem::IsSelectedWindow(BsUiWindow* pWindow)
{
	BsUiWINDOWLISTITOR itr = find(m_SelectedWindows.begin(), m_SelectedWindows.end(), pWindow);
	if(itr != m_windows.end())
		return true;

	return false;
}

bool BsUiSystem::IsWindowInSelectedBox(RECT SelectedBox, RECT rect)
{
	if(rect.left >= SelectedBox.left && rect.left <= SelectedBox.right &&
		rect.top >= SelectedBox.top && rect.top <= SelectedBox.bottom)
	{
		return true;
	}
	else if(rect.right >= SelectedBox.left && rect.right <= SelectedBox.right &&
		rect.top >= SelectedBox.top && rect.top <= SelectedBox.bottom)
	{
		return true;
	}
	else if(rect.left >= SelectedBox.left && rect.left <= SelectedBox.right &&
		rect.bottom >= SelectedBox.top && rect.bottom <= SelectedBox.bottom)
	{
		return true;
	}
	else if(rect.right >= SelectedBox.left && rect.right <= SelectedBox.right &&
		rect.bottom >= SelectedBox.top && rect.bottom <= SelectedBox.bottom)
	{
		return true;
	}

	return false;
}

BsUiHANDLE BsUiSystem::AddWindow(BsUiTYPE uiType, BsUiCLASS hClass, POINT pos, BsUiWindow* pParent)
{
	BsUiWindow* pWindow = NULL;

	// class type info 생성해서 m_classes에 추가
	switch(uiType)
	{
	case BsUiTYPE_LAYER:
		{
			pWindow = (BsUiWindow*)(new BsUiLayer(hClass, pos, pParent));
			BsAssert(pWindow);
			
			g_BsUiSystem.AddWindow(pWindow);
			break;
		}
	case BsUiTYPE_BUTTON:
		{
			pWindow = (BsUiWindow*)(new BsUiButton(hClass, pos, pParent));
			break;
		}
	case BsUiTYPE_LISTBOX:
		{
			pWindow = (BsUiWindow*)(new BsUiListBox(hClass, pos, pParent));
			break;
		}
	case BsUiTYPE_LISTCTRL:
		{
			pWindow = (BsUiWindow*)(new BsUiListCtrl(hClass, pos, pParent));
			break;
		}
	case BsUiTYPE_TEXT:
		{
			pWindow = (BsUiWindow*)(new BsUiText(hClass, pos, pParent));
			break;
		}
	case BsUiTYPE_IMAGECTRL:
		{
			pWindow = (BsUiWindow*)(new BsUiImageCtrl(hClass, pos, pParent));
			break;
		}
	case BsUiTYPE_SLIDER:
		{
			pWindow = (BsUiWindow*)(new BsUiSlider(hClass, pos, pParent));
			break;
		}
	default: BsAssert(0);
	}

	BsAssert(pWindow);

	if(pWindow->GetType() != BsUiTYPE_LAYER)
	{	
		g_BsUiSystem.AddChildWindow(pParent, pWindow);
	}

	return pWindow->GetHWnd();
}

bool BsUiSystem::RemoveControl(BsUiHANDLE hWnd)
{
	BsUiWindow* pWindow = BsUi::BsUiGetWindow(hWnd);
	if(pWindow == NULL)
		return false;

	if(pWindow->GetType() == BsUiTYPE_LAYER)
	{
		g_BsUiSystem.CloseLayer(hWnd, false);
	}
	else
	{
		BsUiWindow* pParentWindow = pWindow->GetParentWindow();
		BsAssert(pParentWindow);

		pParentWindow->RemoveChildWindow(pWindow);
		pWindow->SendMessage(XWMSG_CLOSEWINDOW);

		delete pWindow;
		pWindow = NULL;
	}

	return true;
}


bool BsUiSystem::IsCanMoveSelectedWindows()
{
	bool bParentWindow = false;

	BsUiWINDOWLIST* pSelectWindows = g_BsUiSystem.GetSelectedWindows();
	BsUiWINDOWLISTITOR itr = pSelectWindows->begin();
	while (itr != pSelectWindows->end())
	{
		BsUiWindow* pWindow = (*itr);
		BsAssert(pWindow);

		if(pWindow->GetParentWindow() == NULL)
		{
			bParentWindow = true;
			break;
		}

		++itr;
	}

	itr = pSelectWindows->begin();
	while (itr != pSelectWindows->end())
	{
		BsUiWindow* pWindow = (*itr);
		BsAssert(pWindow);

		if(bParentWindow == true && pWindow->GetParentWindow() != NULL)
		{
			++itr;
			continue;
		}
		
		if(pWindow->IsMovableWindow() == false)
			return false;

		++itr;
	}

	return true;
}

void BsUiSystem::MoveSelectedWindows(POINT AddPos)
{
	bool bParentWindow = false;

	if(AddPos.x == 0 && AddPos.y == 0)
		return;
	
	list <BsUiWindow*>::iterator itr = m_SelectedWindows.begin();
	while (itr != m_SelectedWindows.end())
	{
		BsUiWindow* pWindow = (*itr);
		BsAssert(pWindow);

		if(pWindow->GetParentWindow() == NULL)
		{
			bParentWindow = true;
			break;
		}

		++itr;
	}
	
	itr = m_SelectedWindows.begin();
	while (itr != m_SelectedWindows.end())
	{
		BsUiWindow* pWindow = (*itr);
		BsAssert(pWindow);

		if((m_pGrabbed == pWindow) ||
			(bParentWindow == true && pWindow->GetParentWindow() != NULL))
		{
			++itr;
			continue;
		}

		POINT pos = pWindow->GetWindowPos();
		POINT size = pWindow->GetWindowSize();
		pWindow->MoveWindow(pos.x+AddPos.x, pos.y+AddPos.y, size.x, size.y);

		++itr;
	}
}

void BsUiSystem::ResizeSelectedWindows(ChangeWindowSize WindowSizeFlag, POINT AddSize)
{
	if(WindowSizeFlag == _NOT_CHANGE_SIZE)
		return;

	BsUiWINDOWLISTITOR itr = m_SelectedWindows.begin();
	while (itr != m_SelectedWindows.end())
	{
		BsUiWindow* pWindow = (*itr);
		BsAssert(pWindow);

		if(m_pGrabbed == pWindow)
		{
			++itr;
			continue;
		}

		POINT pos = pWindow->GetWindowPos();
		POINT size = pWindow->GetWindowSize();

		switch(WindowSizeFlag)
		{
		case _CHANGE_SIZE_LEFT:
		case _CHANGE_SIZE_LEFTTOP:
		case _CHANGE_SIZE_LEFTBOTTOM: pos.x -= AddSize.x; break;

		case _CHANGE_SIZE_RIGHT:
		case _CHANGE_SIZE_TOP:
		case _CHANGE_SIZE_BOTTOM:	
		case _CHANGE_SIZE_RIGHTTOP:
		case _CHANGE_SIZE_RIGHTBOTTOM: pos.x += size.x + AddSize.x; break;
		}

		switch(WindowSizeFlag)
		{
		case _CHANGE_SIZE_TOP:
		case _CHANGE_SIZE_LEFTTOP:
		case _CHANGE_SIZE_RIGHTTOP:		pos.y -= AddSize.y; break;

		case _CHANGE_SIZE_LEFT:
		case _CHANGE_SIZE_RIGHT:
		case _CHANGE_SIZE_BOTTOM:	
		case _CHANGE_SIZE_LEFTBOTTOM: 
		case _CHANGE_SIZE_RIGHTBOTTOM:	pos.y += size.y + AddSize.y; break;
		}
			

		pWindow->ResizeWindowSize(WindowSizeFlag, pos);

		++itr;
	}
}


bool BsUiSystem::MakeTabOrder(BsUiWindow* pWindow, BsUiTabOrder nOrder)
{
	BsUiWindow* pParent = pWindow->GetParentWindow();
	if(pParent == NULL)
		return false;

	BsUiWINDOWLISTITOR itr = find(m_windows.begin(), m_windows.end(), pParent);

	if (itr == m_windows.end())
		return false;

	BsUiWindow* pWnd = (*itr);
	return pWnd->MakeTabOrder(pWindow, nOrder);
}