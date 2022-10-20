#include "stdafx.h"

#include "toputil.h"

#include "BsUiWindow.h"
#include "BsUiSystem.h"

#include "BsUiLayer.h"
#include "BsUiButton.h"
#include "BsUiListBox.h"
#include "BsUiListCtrl.h"
#include "BsUiSlider.h"
#include "BsUiText.h"
#include "BsUiImageCtrl.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

BsUiWindow::BsUiWindow() : m_hClass(0), m_hWnd(0), m_pParentWnd(NULL), m_hParentWnd(0)
{
	m_ptMovingGap.x = 0;
	m_ptMovingGap.y = 0;

	m_nImageMode = _IMAGE_MODE_DEFAULT;

	for(int i=0; i<BsUiFS_COUNT; i++)
	{
		m_nFontAttr[i] = -1;
		m_nUVID[i] = -1;
		m_nBlockID[i] = -1;
	}
	m_color = D3DXCOLOR(1, 1, 1, 1);

	m_ptItemPos.x = 0;
	m_ptItemPos.y = 0;

	m_pLinkWnd = NULL;
	m_pChildWndFocus = NULL;

	m_bViewChildFocus = true;
}

bool BsUiWindow::Create(BsUiTYPE type, BsUiCLASS hClass, int x, int y, int cx, int cy, DWORD attr, BsUiWindow* pParent)
{
	m_type = type;
	m_hClass = hClass;
	m_pos.x = x;
	m_pos.y = y;
	m_size.x = cx;
	m_size.y = cy;
	m_attrDef = m_attrCur = attr;

	// 윈도우 핸들을 발급받는다.
	m_hParentWnd = (pParent) ? pParent->GetHWnd() : NULL;
	m_hWnd = BsUi::BsUiMakeHandle(this, hClass, m_hParentWnd);

	// 여기서 child 윈도우로 등록한다.
	if (pParent)
		pParent->AddChildWindow(this);

	m_bModalBox = false;
	m_bMoving = false;
	m_WindowSizeFlag = _NOT_CHANGE_SIZE;

	BsUiClassItem* pClassItem = static_cast <BsUiClassItem* > (g_BsUiSystem.GetLoader()->GetClassInstance(hClass));
	if(pClassItem)
	{
		//Image
		BsUiWindow::SetImageMode(pClassItem->nImageMode);
		for(int i=0; i<BsUiFS_COUNT; i++)
		{
			BsUiFocusState flag = (BsUiFocusState)i;
			BsUiWindow::SetImageUVID(flag, pClassItem->nUVID[flag]);
			BsUiWindow::SetImageBlock(flag, pClassItem->nBlockID[flag]);
		}

		//text
		BsUiWindow::SetItemTextID(pClassItem->nTextId);
		BsUiWindow::SetItemPos(pClassItem->ptTextPos);
		for(int i=0; i<BsUiFS_COUNT; i++)
		{	
			BsUiFocusState flag = (BsUiFocusState)i;
			BsUiWindow::SetFontAttr(flag, pClassItem->nFontAttr[flag]);
		}
	}

	return true;
}

void BsUiWindow::Release()
{
	if(m_Item.szText != NULL)
	{
		delete[] m_Item.szText;
		m_Item.szText = NULL;
	}

	if (BsUi::BsUiIsValid(m_hWnd))
		BsUi::BsUiDestroyHandle(m_hWnd);
	
	BsUiWINDOWLISTITOR itr;
	BsUiWINDOWLISTITOR itr_begin = m_child.begin();
	for(itr = itr_begin; itr!=m_child.end();itr++)
	{
		delete (*itr);
	}
	m_child.clear();
	
	itr_begin = m_tabOrder.begin();
	for(itr = itr_begin; itr!=m_tabOrder.end();itr++)
	{
		delete (*itr);
	}
	m_tabOrder.clear();
}

// Queue에 넣고 빠진다. 
void BsUiWindow::PostMessage(DWORD message, WPARAM wParam, LPARAM lParam)
{
	g_BsUiSystem.PostMessage(GetHWnd(), message, wParam, lParam);
}

// 즉각 실행한다.
DWORD BsUiWindow::SendMessage(DWORD message, WPARAM wParam, LPARAM lParam)
{
	return g_BsUiSystem.SendMessage(GetHWnd(), message, wParam, lParam);
}

// window를 갱신한다. 각종 state, message 등을 처리한다.
void BsUiWindow::Update()
{
	// 하위 윈도우들을 갱신한다.
	BsUiWINDOWLISTITOR itr = m_child.begin();

	while (itr != m_child.end())
	{
		BsUiWindow* pWindow = (*itr);
		BsAssert(pWindow);

		pWindow->Update();
		++itr;
	}

	// 닫을 창을 닫는다.
	itr = m_child.begin();
/*
	// 메시지로 창을 닫게끔 하자
	while (itr != m_child.end())
	{
		BsUiWindow* pWindow = (*itr);
		BsAssert(pWindow);

		if (pWindow->IsClose())
		{
			m_child.erase(itr++);

			// tab-order에 있다면 정보도 없앤다.
			if (pWindow->GetWindowAttr() & XWATTR_USETABORDER)
			{
				itr = find(m_tabOrder.begin(), m_tabOrder.end(), pWindow);

				if (itr != m_tabOrder.end())
					m_tabOrder.erase(itr);
			}

			pWindow->DestroyWindow();
			delete pWindow;

			// 포커스 지정
			if (GetFocus() == NULL)
			{
				BsUiWindow* pWnd = this->m_pParentWnd;

				if (pWnd == NULL)
					pWnd = g_BsUiSystem.GetMainWnd();
				
				pWnd->RunAroundTabOrder();
			}
		}
		else
		{
			++itr;
		}
	}
*/
}

void BsUiWindow::Draw()
{
	POINT pt;
	pt.x = 0; pt.y = 0;
	ClientToScreen(pt);

	if(IsShowWindow() == false)
		return;

	g_BsUiSystem.GetView()->Draw(this);

	// child 출력
	BsUiWINDOWLISTITOR itr = m_child.begin();

	while (itr != m_child.end())
	{
		BsUiWindow* pWindow = (*itr);
		BsAssert(pWindow);

		pWindow->Draw();
		++itr;
	}
}

// child window (주로 컨트롤)을 추가/삭제한다.
void BsUiWindow::AddChildWindow( BsUiWindow* pWindow )
{
	BsAssert(pWindow);

	if (!pWindow->IsWindow())
		throw BsUiException("the same child window is already registed.");

	m_child.push_back(pWindow);

	// 부모 윈도우로 지정한다.
	pWindow->SetParentWindow(this);

	// tab-order인 경우 관리 list에 추가한다.
	if (pWindow->GetWindowAttr() & XWATTR_USETABORDER)
		m_tabOrder.push_back(pWindow);
}

void BsUiWindow::RemoveChildWindow( BsUiWindow* pWindow )
{
	BsAssert(pWindow);

	if (!pWindow->IsWindow())
		throw BsUiException("the given child window does not regist before.");

	// child 리스트에서 제거한다.
	BsUiWINDOWLISTITOR itr = find(m_child.begin(), m_child.end(), pWindow);

	if (itr != m_child.end())
		m_child.erase(itr);

	
	// tab-order에 있다면 정보도 없앤다.
	if (pWindow->GetWindowAttr() & XWATTR_USETABORDER)
	{
		itr = find(m_tabOrder.begin(), m_tabOrder.end(), pWindow);

		if (itr != m_tabOrder.end())
			m_tabOrder.erase(itr);
	}
/*
	// 포커스는 따로 지정하게끔 한다.
	// 포커스 지정
	if (GetFocus() == NULL)
	{
		BsUiWindow* pWnd = this->m_pParentWnd;

		if (pWnd == NULL)
			pWnd = g_BsUiSystem.GetMainWnd();
		
		pWnd->RunAroundTabOrder();
	}
*/
}

BsUiWindow* BsUiWindow::GetDlgItem(BsUiCLASS hClass)
{
	BsUiWINDOWLISTITOR itr = m_child.begin();

	while (itr != m_child.end())
	{
		BsUiWindow* pWindow = (*itr);
		BsAssert(pWindow);

		if (pWindow->GetClass() == hClass)
			return pWindow;

		++itr;
	}

	return NULL;
}

void BsUiWindow::ClientToScreen(POINT& pt)
{
	pt.x += m_pos.x;
	pt.y += m_pos.y;
}

void BsUiWindow::ScreenToClient(POINT& pt)
{
	pt.x -= m_pos.x;
	pt.y -= m_pos.y;
}

void BsUiWindow::MoveWindow(int x, int y, int cx, int cy)
{
	if(this->IsDragAndDrop() == false)
	{
		BsUiWindow* pParent = this->GetParentWindow();
		if(pParent != NULL)
		{
			RECT ParentRect;
			pParent->GetWindowRect(&ParentRect);

			if(x < ParentRect.left)
				x = ParentRect.left;
			if(x+cx > ParentRect.right)
				x = ParentRect.right - cx;
			if(y < ParentRect.top)
				y = ParentRect.top;
			if(y+cy > ParentRect.bottom)
				y = ParentRect.bottom - cy;
		}
	}

	int dx = x - m_pos.x;
	int dy = y - m_pos.y;

	m_pos.x = x;
	m_pos.y = y;
	m_size.x = cx;
	m_size.y = cy;

	// child도 전부 갱신
	BsUiWINDOWLISTITOR itr = m_child.begin();

	while (itr != m_child.end())
	{
		BsUiWindow* pWnd = (*itr);
		BsAssert(pWnd);

		POINT pt = pWnd->GetWindowPos();
		POINT size = pWnd->GetWindowSize();

		pWnd->MoveWindow(pt.x + dx, pt.y + dy, size.x, size.y);

		++itr;
	}
}

// 자신 혹은 자신의 차일드 이후의 모든 윈도우 중에서 pPt을 포함하는 윈도우를 돌려준다.
BsUiWindow* BsUiWindow::GetWindowOnPos(POINT* pPt)
{
	int dx = pPt->x - m_pos.x;
	int dy = pPt->y - m_pos.y;

	if (dx >= 0 && dx < m_size.x && dy >= 0 && dy < m_size.y)
	{
		if(m_child.size() > 0)
		{
			BsUiWINDOWLISTITOR itr = m_child.end();

			do{
				--itr;

				BsUiWindow* pWnd = (*itr)->GetWindowOnPos(pPt);

				if (pWnd)
					return pWnd;

				
			}while (itr != m_child.begin());
		}
	}
	else
	{
		return NULL;
	}

	return this;
}

/*
BsUiWindow* BsUiWindow::RunAroundTabOrder()
{
	BsUiWindow* pFocus = BsUiSystem::GetFocusWindow();

	if (pFocus)
	{
		if (pFocus != this && GetWindow(pFocus->GetHWnd()))
		{
			if (!m_tabOrder.empty())
			{
				BsUiWINDOWLISTITOR itr = find(m_tabOrder.begin(), m_tabOrder.end(), pFocus);

				if (++itr != m_tabOrder.end())
					(*itr)->SetFocus();
				else
					m_tabOrder.front()->SetFocus();
			}

			return pFocus;
		}
	}

	// child 중에 앞에껄로
	if (!m_child.empty())
		BsUiSystem::SetFocusWindow(m_child.front());

	return BsUiSystem::GetFocusWindow();
}
*/
// modal처리 - 항상 메인 윈도우의 차일드로 붙는다.
void BsUiWindow::DoModal()
{
	if (!g_BsUiSystem.PushModal(this))
		throw BsUiException("the same modal window is already pushed.");

	// 스스로를 modal 윈도우로 인식
	m_bModalBox = true;

	// modal 윈도우는 무조건 포커스를 받는다.
	BsUiSystem::SetGrabWindow(NULL);
	g_BsUiSystem.PushModal(this);
	g_BsUiSystem.SetFocusWindow(this);
}

// modal로 시작한 윈도우는 반드시 이 함수로 종료해야 한다.
void BsUiWindow::EndOfModal(int nResult, void* pRetValue)
{
	if (!g_BsUiSystem.PopModal(this))
		throw BsUiException("the given modal window does not stack before.");

	// endof modal 처리를 수행케한다.
	g_BsUiSystem.PopModal(this, nResult, pRetValue);

	m_bModalBox = false;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

// 메시지를 처리한다.
DWORD BsUiWindow::ProcMessage(xwMessageToken* pMsgToken)
{
	BsAssert(pMsgToken);

	if (pMsgToken->hWnd != this->GetHWnd())
	{
		BsUiWindow* pMsgOwner = BsUi::BsUiGetWindow(pMsgToken->hWnd);

		if (!pMsgOwner)
			return -1;

		// 혹시나 child 윈도우에서 발송된 메시지라면
		if (find(m_child.begin(), m_child.end(), pMsgOwner) != m_child.end())
		{
			switch (pMsgToken->message)
			{
			case XWMSG_BN_CLICKED:
				break;
			case XWMSG_SD_CHANGED:
				break;
			}
		}

		return 0L;
	}
/*
#ifdef _DEBUG
	CString str;
	str.Format("msg(%d:%d:%d) - h(%d/%d)\n", pMsgToken->message, pMsgToken->wParam, pMsgToken->lParam, this->GetClass(), this->GetHWnd());

	DebugString(str);
#endif
*/
	switch (pMsgToken->message)
	{
	case XWMSG_LBUTTONDOWN:
		{
			BsUiSystem::SetFocusWindow(this);

			BsUiSkin* pSkin = g_BsUiSystem.GetSkin();

			if (pSkin)
			{
				pSkin->ClearDragDropInfo();

				int nTypeOfObject = 0;
				DWORD dwData = 0;

				if (pSkin->ReadyToDrag(this, pMsgToken, nTypeOfObject, dwData))
				{
					BsUiDragDropInfo dd;

					pSkin->GetDragDropInfo(&dd);

					memcpy(&dd.msgButtonDown, pMsgToken, sizeof(xwMessageToken));

					dd.hSourceClass = this->GetClass();
					dd.hSourceHwnd = this->GetHWnd();
					dd.nTypeObject = nTypeOfObject;
					dd.dwData = dwData;

					pSkin->SetDragDropInfo(&dd);
					break;
				}
			}

			BsAssert(g_BsUiSystem.GetSkin()); //aleksger: prefix bug 676: GetSkin() may return NULL.
			m_WindowSizeFlag = (ChangeWindowSize)g_BsUiSystem.GetSkin()->GetCanSizeState(this, pMsgToken->pt);
			if(m_WindowSizeFlag != _NOT_CHANGE_SIZE)
			{
				switch(m_WindowSizeFlag)
				{
				case _CHANGE_SIZE_LEFT:		m_ptMovingGap.x = pMsgToken->pt.x - this->GetWindowPos().x;	break;
				case _CHANGE_SIZE_RIGHT:	m_ptMovingGap.x = pMsgToken->pt.x - (this->GetWindowPos().x + this->GetWindowSize().x); break;
				case _CHANGE_SIZE_TOP:		m_ptMovingGap.y = pMsgToken->pt.y - this->GetWindowPos().y; break; 
				case _CHANGE_SIZE_BOTTOM:	m_ptMovingGap.y = pMsgToken->pt.y - (this->GetWindowPos().y + this->GetWindowSize().y); break;

				case _CHANGE_SIZE_LEFTTOP:
					{
						m_ptMovingGap.x = pMsgToken->pt.x - this->GetWindowPos().x;
						m_ptMovingGap.y = pMsgToken->pt.y - this->GetWindowPos().y;
						break; 
					}
				case _CHANGE_SIZE_LEFTBOTTOM:
					{
						m_ptMovingGap.x = pMsgToken->pt.x - this->GetWindowPos().x;
						m_ptMovingGap.y = pMsgToken->pt.y - (this->GetWindowPos().y + this->GetWindowSize().y);
						break;
					}
				case _CHANGE_SIZE_RIGHTTOP:
					{
						m_ptMovingGap.x = pMsgToken->pt.x - (this->GetWindowPos().x + this->GetWindowSize().x);
						m_ptMovingGap.y = pMsgToken->pt.y - this->GetWindowPos().y;
						break;
					}
				case _CHANGE_SIZE_RIGHTBOTTOM:
					{
						m_ptMovingGap.x = pMsgToken->pt.x - (this->GetWindowPos().x + this->GetWindowSize().x);
						m_ptMovingGap.y = pMsgToken->pt.y - (this->GetWindowPos().y + this->GetWindowSize().y);
						break;
					}
				}

				BsUiSystem::SetGrabWindow(this);
			}
			// window move를 시도한다.
			else if (IsMovableWindow() &&
				g_BsUiSystem.GetSkin()->IsCanMove(this, pMsgToken->pt))
			{
				m_ptMovingGap.x = pMsgToken->pt.x - this->GetWindowPos().x;
				m_ptMovingGap.y = pMsgToken->pt.y - this->GetWindowPos().y;
				m_bMoving = true;

				BsUiSystem::SetGrabWindow(this);

				//debug("윈도우이동: (%d)\n", this->GetClass());
			}
			else
			{	
				m_WindowSizeFlag = _NOT_CHANGE_SIZE;
				m_bMoving = false;
			}
		}
		break;
	case XWMSG_MOUSEMOVE:
		{
			if(m_WindowSizeFlag != _NOT_CHANGE_SIZE)
			{
				POINT pos;
				pos.x = pMsgToken->pt.x - m_ptMovingGap.x;
				pos.y = pMsgToken->pt.y - m_ptMovingGap.y;
				ResizeWindowSize(m_WindowSizeFlag, pos);//pMsgToken);
			}
			else if (m_bMoving)
			{
				MoveWindow(pMsgToken->pt.x - m_ptMovingGap.x,
					pMsgToken->pt.y - m_ptMovingGap.y,
					this->GetWindowSize().x,
					this->GetWindowSize().y);
			}
			else
			{
				BsUiSkin* pSkin = g_BsUiSystem.GetSkin();

				if (pSkin)
				{
					if (pSkin->DragObject(this, pMsgToken))
						pSkin->BeginDragDropInfo();
				}
			}
		}
		break;
	case XWMSG_LBUTTONUP:
		{	
			if(m_WindowSizeFlag == _NOT_CHANGE_SIZE &&
				m_bMoving == false &&
				BsUiSystem::GetGrabWindow() != NULL)
			{
				BsUiSystem::SetGrabWindow(NULL);
			}

			if(m_WindowSizeFlag != _NOT_CHANGE_SIZE || m_bMoving)
			{	
				if (BsUiSystem::GetGrabWindow() == this)
                    BsUiSystem::SetGrabWindow(NULL);
				
				m_WindowSizeFlag = _NOT_CHANGE_SIZE;
				m_bMoving = false;
			}
			else
			{
				BsUiSkin* pSkin = g_BsUiSystem.GetSkin();

				if (pSkin)
				{
					if (pSkin->IsDraggingObject())
					{
						// Drop에 따른 정보를 추가한 뒤에 Drop시킨다.
						{
							BsUiDragDropInfo dd;

							pSkin->GetDragDropInfo(&dd);

							memcpy(&dd.msgButtonUp, pMsgToken, sizeof(xwMessageToken));
							dd.hTargetClass = this->GetClass();
							dd.hTargetHwnd = this->GetHWnd();

							pSkin->SetDragDropInfo(&dd);
						}

						pSkin->DropObject(this, pMsgToken);
					}

					pSkin->ClearDragDropInfo();
				}
			}
		}
		break;
	case XWMSG_RBUTTONDOWN:
		{
			BsUiSystem::SetFocusWindow(this);
		}
		break;
	case XWMSG_KEYDOWN:
		break;

	case XWMSG_OPENWINDOW:
		{
			BsUiWINDOWLISTITOR itr = m_child.begin();

			while (itr != m_child.end())
			{
				BsUiWindow* pWindow = (*itr);
				BsAssert(pWindow);

				pWindow->SendMessage(pMsgToken->message, pMsgToken->wParam, pMsgToken->lParam);
				++itr;
			}
		}
		break;
	case XWMSG_CLOSEWINDOW:
		{
			BsUiWINDOWLISTITOR itr = m_child.begin();

			while (itr != m_child.end())
			{
				BsUiWindow* pWindow = (*itr);
				BsAssert(pWindow);

				pWindow->SendMessage(pMsgToken->message, pMsgToken->wParam, pMsgToken->lParam);
				m_child.erase(itr++);
				delete pWindow;
				pWindow = NULL;
			}

			if (BsUiSystem::GetFocusWindow() == this)
				BsUiSystem::SetFocusWindow(NULL);
			if (BsUiSystem::GetGrabWindow() == this)
				BsUiSystem::SetGrabWindow(NULL);
		}
		break;
	case XWMSG_MAKETOP:
		{
			if (GetParentWindow())
				GetParentWindow()->SendMessage(XWMSG_MAKETOP);
			else
				g_BsUiSystem.MakeTop(this);		// child가 아닌 경우에만 위로 올리게 된다.
		}
		break;
	case XWMSG_MAKECENTER:
		{
			this->SetWindowPos((g_BsUiSystem.GetScreenWidth() - m_size.x) / 2, (g_BsUiSystem.GetScreenHeight() - m_size.y) / 2);
		}
		break;
	case XWMSG_CHECKBUTTON: // check button을 클릭했을 때 group 내에서 유일하게 클릭되게끔 한다.
		{
			BsUiWINDOWLISTITOR itr = m_child.begin();

			while (itr != m_child.end())
			{
				BsUiWindow* pChild = (*itr);
				BsAssert(pChild);

				if (pChild->GetType() == BsUiTYPE_CHECKBUTTON)
				{
					BsUiButton* pButton = (BsUiButton* ) pChild;
					if (pButton->GetGroupID() == pMsgToken->lParam)
						pButton->SetCheck((pButton->GetHWnd() == pMsgToken->wParam));
				}

				++itr;
			}
		}
		break;
	}

	return 0L;
}

void BsUiWindow::ResizeWindowSize(ChangeWindowSize nFlag, POINT pos)//xwMessageToken* pMsgToken)
{
	RECT ParentRect;
	ParentRect.left = ParentRect.top = ParentRect.right = ParentRect.bottom = 0;
	BsUiWindow* pParent = this->GetParentWindow();
	if(pParent != NULL)
	{	
		pParent->GetWindowRect(&ParentRect);
	}
	

	RECT rect;
	this->GetWindowRect(&rect);

	POINT Addpos, Addsize;
	Addpos.x = Addpos.y = Addsize.x = Addsize.y = 0;

	switch(nFlag)//m_WindowSizeFlag)
	{
	case _CHANGE_SIZE_LEFT:
		{
			int nValue = rect.left - pos.x;
			if(this->GetWindowSize().x + nValue < 10)
			{
				nValue = 10 - this->GetWindowSize().x;
			}

			Addpos.x = -nValue;
			Addsize.x = nValue;

			if(pParent != NULL && rect.left + Addpos.x <= ParentRect.left)
			{	
				nValue = rect.left - ParentRect.left; //aleksger: removing extra declarations.
				Addpos.x = -nValue;
				Addsize.x = nValue;
			}
			break;
		}
	case _CHANGE_SIZE_RIGHT:
		{	
			int nValue = pos.x - rect.right;
			if(this->GetWindowSize().x + nValue < 10)
			{
				nValue = 10 - this->GetWindowSize().x;
			}
			Addsize.x = nValue;

			if(pParent != NULL && rect.right + Addsize.x > ParentRect.right)
			{
				Addsize.x = ParentRect.right - rect.right;
			}
			break;
		}
	case _CHANGE_SIZE_TOP:
		{	
			int nValue = rect.top - pos.y;
			if(this->GetWindowSize().y + nValue < 10)
			{
				nValue = 10 - this->GetWindowSize().y;
			}

			Addpos.y = -nValue;
			Addsize.y = nValue;

			if(pParent != NULL && rect.top + Addpos.y <= ParentRect.top)
			{	
				nValue = rect.top - ParentRect.top; //aleksger: removing extra declarations.
				Addpos.y = -nValue;
				Addsize.y = nValue;
			}

			break;
		}
	case _CHANGE_SIZE_BOTTOM:
		{	
			int nValue = pos.y - rect.bottom;
			if(this->GetWindowSize().y + nValue < 10)
			{
				nValue = 10 - this->GetWindowSize().y;
			}
			Addsize.y = nValue;

			if(pParent != NULL && rect.bottom + Addsize.y > ParentRect.bottom)
			{
				Addsize.y = ParentRect.bottom - rect.bottom;
			}
			break;
		}
	case _CHANGE_SIZE_LEFTTOP:
		{
			ResizeWindowSize(_CHANGE_SIZE_LEFT, pos);
			ResizeWindowSize(_CHANGE_SIZE_TOP, pos);
			break;
		}
	case _CHANGE_SIZE_LEFTBOTTOM:
		{
			ResizeWindowSize(_CHANGE_SIZE_LEFT, pos);
			ResizeWindowSize(_CHANGE_SIZE_BOTTOM, pos);
			break;
		}
	case _CHANGE_SIZE_RIGHTTOP:
		{
			ResizeWindowSize(_CHANGE_SIZE_RIGHT, pos);
			ResizeWindowSize(_CHANGE_SIZE_TOP, pos);
			break;
		}
	case _CHANGE_SIZE_RIGHTBOTTOM:
		{
			ResizeWindowSize(_CHANGE_SIZE_RIGHT, pos);
			ResizeWindowSize(_CHANGE_SIZE_BOTTOM, pos);
			break;
		}
	}
	
	m_pos.x += Addpos.x;
	m_pos.y += Addpos.y;
	m_size.x += Addsize.x;
	m_size.y += Addsize.y;	
}

BsUiWindow* BsUiWindow::GetChildWindow(BsUiCLASS hClass)
{
	BsUiWINDOWLISTITOR itr = m_child.begin();
	while (itr != m_child.end())
	{
		BsUiWindow* pWindow = (*itr);
		BsAssert(pWindow);
		
		if(hClass == NULL || pWindow->GetClass() == hClass)
		{
			return pWindow;
		}
		
		++itr;
	}

	return NULL;
}

BsUiWindow* BsUiWindow::GetEnableChildWindow(BsUiCLASS hClass)
{
	BsUiWINDOWLISTITOR itr = m_child.begin();
	while (itr != m_child.end())
	{
		BsUiWindow* pWindow = (*itr);
		BsAssert(pWindow);

		if(hClass == NULL || pWindow->GetClass() == hClass)
		{
			if(pWindow->IsEnableWindow())
				return pWindow;
		}

		++itr;
	}

	return NULL;
}

BsUiWindow* BsUiWindow::GetWindow(BsUiCLASS hClass)
{
	if(m_hClass == hClass)
	{
		return this;
	}
	else if(m_child.size() > 0)
	{
		return GetChildWindow(hClass);
	}

	return NULL;
}

void BsUiWindow::SetWindowAttr(DWORD attr, bool bValue)
{	
	bool bFlag = (m_attrCur & attr) ? true : false;
	if(bFlag != bValue)
	{
		if(bValue == false)
			m_attrCur -= attr;
		else
			m_attrCur |= attr;
	}
}

BsUiFocusState	BsUiWindow::GetCurFocusState()
{
	if(IsEnableWindow() == false)
	{
		return BsUiFS_DISABLE;
	}

	BsUiWindow* pFocus = BsUiSystem::GetFocusWindow();
	if(pFocus != NULL)
	{
		bool bFlag = false;
		if(pFocus == this)
		{
			bFlag = true;
		}
		else if(pFocus == m_pLinkWnd)
		{
			bFlag = true;
		}
		else
		{
			BsUiWINDOWLISTITOR itr = find(m_child.begin(), m_child.end(), pFocus);

			if(itr != m_child.end())
				bFlag = true;
		}

		if(bFlag == true)
		{
			return BsUiFS_SELECTED;
		}
	}

	return BsUiFS_DEFAULTED;
}

void BsUiWindow::SetItem(BsUiItemInfo item)
{
	m_Item.dwData = item.dwData;
	m_Item.nTextID = item.nTextID;
	SetItemText(item.szText);
}

void BsUiWindow::SetItemText(char* pStr)
{	
	if(m_Item.szText != NULL)
	{
// [beginmodify] 2006/2/3 junyash PS#4609 detect new[] and delete mis-match
		//delete m_Item.szText;
		delete[] m_Item.szText; // delate char[] is stable but delete[] to make sure.
// [endmodify] junyash
		m_Item.szText = NULL;
	}

	if(pStr != NULL)
	{
		const size_t pStr_len = strlen(pStr)+1;
		m_Item.szText = new char[pStr_len];
		strcpy_s(m_Item.szText, pStr_len, pStr); //aleksger - safe string
	}
}

void BsUiWindow::SetItemTextID(int nTextID)
{	
	if(nTextID == -1)
	{
		SetItemText(NULL);
		m_Item.nTextID = nTextID;
		return;
	}

	char szText[256];
	if(g_BsUiSystem.GetView()->GetText(nTextID, szText, _countof(szText)) == true)
	{	
		SetItemText(szText);
		m_Item.nTextID = nTextID;
	}
}

BsUiAttrId BsUiWindow::GetCurFontAttr()
{ 
	BsUiFocusState focusState = BsUiWindow::GetCurFocusState();
	if(m_bViewChildFocus == true)
	{
		BsUiWindow*	pParent = GetParentWindow();
		if(pParent != NULL && focusState == BsUiFS_DEFAULTED)
		{
			if(pParent->GetChildWndFocus() == this){
				focusState = BsUiFS_SELECTED;
			}
		}
	}

	return m_nFontAttr[focusState];
}

int BsUiWindow::GetCurImageUVID()
{ 
	BsUiFocusState focusState = BsUiWindow::GetCurFocusState();
	if(m_bViewChildFocus == true)
	{
		BsUiWindow*	pParent = GetParentWindow();
		if(pParent != NULL && focusState == BsUiFS_DEFAULTED)
		{
			if(pParent->GetChildWndFocus() == this){
				focusState = BsUiFS_SELECTED;
			}
		}
	}

	return m_nUVID[focusState];
}

int BsUiWindow::GetCurImageBlockID()
{ 
	BsUiFocusState focusState = BsUiWindow::GetCurFocusState();
	if(m_bViewChildFocus == true)
	{
		BsUiWindow*	pParent = GetParentWindow();
		if(pParent != NULL && focusState == BsUiFS_DEFAULTED)
		{
			if(pParent->GetChildWndFocus() == this){
				focusState = BsUiFS_SELECTED;
			}
		}
	}

	return m_nBlockID[focusState];
}


void BsUiWindow::MakeTopforChild(BsUiWindow* pWindow)
{	
	BsUiWINDOWLISTITOR itr = find(m_child.begin(), m_child.end(), pWindow);

	if (itr == m_child.end())
		return;

	DWORD nAttr = pWindow->GetWindowAttr();

	if (nAttr & XWATTR_ALWAYSBOTTOM)
		return;
	if (m_child.size() == 1)
		return;

	// 일단 빼서
	itr = m_child.erase(itr);

	// 맨 앞에서부터 조건이 맞을 때까지 넣어둔다.
	itr = m_child.begin();

	do {
		BsUiWindow* pComp = (*itr);
		BsAssert(pComp);

		DWORD nAttr2 = pComp->GetWindowAttr();

		if ((nAttr2 & XWATTR_ALWAYSTOP) && (nAttr & XWATTR_ALWAYSTOP) == false)
			++itr;
		else
		{
			m_child.insert(itr, 1, pWindow);
			return;
		}
	} while (itr != m_child.end());
}

//내부의 m_taborder과는 상관없다.
bool BsUiWindow::MakeTabOrder(BsUiWindow* pWindow, BsUiTabOrder nOrder)
{
	switch(nOrder)
	{
	case BsUiTO_TOP:	return MakeTabOrderTop(pWindow);
	case BsUiTO_UP:		return MakeTabOrderUp(pWindow);
	case BsUiTO_DOWN:	return MakeTabOrderDown(pWindow);
	case BsUiTO_BOTTOM:	return MakeTabOrderBottom(pWindow);
	}

	return false;
}

bool BsUiWindow::MakeTabOrderTop(BsUiWindow* pWindow)
{
	if (m_child.size() == 1)
		return false;

	DWORD nAttr = pWindow->GetWindowAttr();
	if (nAttr & XWATTR_ALWAYSBOTTOM)
		return false;

	BsUiWINDOWLISTITOR itr = find(m_child.begin(), m_child.end(), pWindow);

	if (itr == m_child.end())
		return false;
			
	// 일단 빼서
	itr = m_child.erase(itr);

	// 맨 앞에서부터 조건이 맞을 때까지 넣어둔다.
	itr = m_child.begin();
	
	do {
		BsUiWindow* pComp = (*itr);
		BsAssert(pComp);

		DWORD nAttr2 = pComp->GetWindowAttr();

		if ((nAttr2 & XWATTR_ALWAYSTOP) && (nAttr & XWATTR_ALWAYSTOP) == false)
			++itr;
		else
		{
			m_child.insert(itr, pWindow);
			return true;
		}
	} while (itr != m_child.end());

	return false;
}

bool BsUiWindow::MakeTabOrderUp(BsUiWindow* pWindow)
{
	if (m_child.size() == 1)
		return false;

	DWORD nAttr = pWindow->GetWindowAttr();
	if (nAttr & XWATTR_ALWAYSBOTTOM)
		return false;

	BsUiWINDOWLISTITOR itr = find(m_child.begin(), m_child.end(), pWindow);

	if (itr == m_child.end())
		return false;

	if (itr == m_child.begin())
		return false;

	//이전 것을 기억하고
	BsUiWindow* pItr = (*(--itr)); //aleksger: prefix bug 678: Multiple variable declaration

	// 일단 빼서
	itr = m_child.erase(++itr);

	//이전 것의 위치를 다시 확인하고
	itr = find(m_child.begin(), m_child.end(), pItr);
	if (itr == m_child.end())
	{
		BsAssert(0);
		return false;
	}

	do {
		BsUiWindow* pComp = (*itr);
		BsAssert(pComp);

		DWORD nAttr2 = pComp->GetWindowAttr();

		if ((nAttr2 & XWATTR_ALWAYSTOP) && (nAttr & XWATTR_ALWAYSTOP) == false)
			++itr;
		else
		{
			m_child.insert(itr, pWindow);
			return true;
		}
	} while (itr != m_child.end());

	return false;
}


bool BsUiWindow::MakeTabOrderBottom(BsUiWindow* pWindow)
{
	if (m_child.size() == 1)
		return false;

	DWORD nAttr = pWindow->GetWindowAttr();
	if (nAttr & XWATTR_ALWAYSTOP)
		return false;

	BsUiWINDOWLISTITOR itr = find(m_child.begin(), m_child.end(), pWindow);

	if (itr == m_child.end())
		return false;

	// 일단 빼서
	itr = m_child.erase(itr);

	// 맨 뒤에서부터 조건이 맞을 때까지 넣어둔다.
	itr = --m_child.end();

	do {
		BsUiWindow* pComp = (*itr);
		BsAssert(pComp);

		DWORD nAttr2 = pComp->GetWindowAttr();

		if ((nAttr2 & XWATTR_ALWAYSBOTTOM) && (nAttr & XWATTR_ALWAYSBOTTOM) == false)
			--itr;
		else if(itr == --m_child.end())
		{
			m_child.push_back(pWindow);
			return true;
		}
		else
		{
			m_child.insert(itr, pWindow);
			return true;
		}
	} while (itr != m_child.begin());

	return false;
}

bool BsUiWindow::MakeTabOrderDown(BsUiWindow* pWindow)
{
	if (m_child.size() == 1)
		return false;

	DWORD nAttr = pWindow->GetWindowAttr();
	if (nAttr & XWATTR_ALWAYSTOP)
		return false;

	BsUiWINDOWLISTITOR itr = find(m_child.begin(), m_child.end(), pWindow);

	if (itr == m_child.end() || itr == --m_child.end())
		return false;

	//이전 것을 기억하고
	BsUiWindow* pItr = (*(++itr)); //aleksger: prefix bug 679: Multiple variable declaration

	// 일단 빼서
	itr = m_child.erase(--itr);

	//이전 것의 위치를 다시 확인하고
	itr = find(m_child.begin(), m_child.end(), pItr);
	if (itr == m_child.end())
	{
		BsAssert(0);
		return false;
	}
	
	do {
		BsUiWindow* pComp = (*itr);
		BsAssert(pComp);

		DWORD nAttr2 = pComp->GetWindowAttr();

		if ((nAttr2 & XWATTR_ALWAYSBOTTOM) && (nAttr & XWATTR_ALWAYSBOTTOM) == false)
			--itr;
		else if(itr == --m_child.end())
		{
			m_child.push_back(pWindow);
			return true;
		}
		else
		{
			BsUiWindow* pwnd = (*++itr);
			m_child.insert(itr, pWindow);
			return true;
		}
	} while (itr != m_child.begin());

	return false;
}

BsUiHANDLE BsUiWindow::GetNextChildHandle(BsUiHANDLE hHandle, bool bEnableFocus)
{
	BsUiWindow* pWindow = BsUi::BsUiGetWindow(hHandle);
	BsUiWINDOWLISTITOR itr = find(m_child.begin(), m_child.end(), pWindow);

	if(m_child.empty())
		return NULL;
	while(1)
	{
		if(itr == --m_child.end()){
			itr = m_child.begin();
		}
		else{
			++itr;
		}
		
		pWindow = (*itr);
		if(pWindow->GetLinkWindow() != NULL){
			continue;
		}

		if(pWindow->GetHWnd() == hHandle){	
			return NULL;
		}		

		if(pWindow->IsEnableWindow() == bEnableFocus){
			return pWindow->GetHWnd();
		}
	}
}

BsUiHANDLE BsUiWindow::GetPrevChildHandle(BsUiHANDLE hHandle, bool bEnableFocus)
{
	BsUiWindow* pWindow = BsUi::BsUiGetWindow(hHandle);
	BsUiWINDOWLISTITOR itr = find(m_child.begin(), m_child.end(), pWindow);

	while(1)
	{
		if(itr == m_child.begin()){
			itr = --m_child.end();
		}
		else{
			--itr;
		}

		pWindow = (*itr);
		if(pWindow->GetLinkWindow() != NULL){
			continue;
		}

		if(pWindow->GetHWnd() == hHandle){	
			return NULL;
		}		

		if(pWindow->IsEnableWindow() == bEnableFocus){
			return pWindow->GetHWnd();
		}
	}
}

BsUiWindow*	BsUiWindow::SetFocusNextChileWindow(BsUiHANDLE hHandle, bool bEnableFocus)
{
	BsUiHANDLE hNextHandle = GetNextChildHandle(hHandle, bEnableFocus);
	if(hNextHandle != NULL){
		g_BsUiSystem.SetFocusWindow(BsUi::BsUiGetWindow(hNextHandle));
	}
	
	BsUiWindow* pNext = g_BsUiSystem.GetFocusWindow();

	return pNext;
}

BsUiWindow*	BsUiWindow::SetFocusPrevChileWindow(BsUiHANDLE hHandle, bool bEnableFocus)
{
	BsUiHANDLE hPrevHandle = GetPrevChildHandle(hHandle, bEnableFocus);
	if(hPrevHandle != NULL){
		g_BsUiSystem.SetFocusWindow(BsUi::BsUiGetWindow(hPrevHandle));
	}

	BsUiWindow* pPrev = g_BsUiSystem.GetFocusWindow();

	return pPrev;
}

BsUiWindow*	BsUiWindow::GetChildWndCurFocus()
{
	// 하위 윈도우들을 갱신한다.
	BsUiWINDOWLISTITOR itr = m_child.begin();

	while (itr != m_child.end())
	{
		BsUiWindow* pWindow = (*itr);
		BsAssert(pWindow);

		if(g_BsUiSystem.GetFocusWindow() == pWindow){
			return pWindow;
		}
		
		++itr;
	}

	return NULL;
}