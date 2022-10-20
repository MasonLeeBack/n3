#include "stdafx.h"

#include "BsUiListCtrl.h"

#include "BsUiSystem.h"

#include "BsUiMenuInput.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


BsUiListCtrl::BsUiListCtrl() : BsUiWindow()
{
	m_nCurSel = 0;
	m_nTopIndex = 0;
	m_bDrawSelBox = true;
}

BsUiListCtrl::BsUiListCtrl(BsUiCLASS hClass, POINT pos, BsUiWindow* pParent) : BsUiWindow()
{
	BsAssert(hClass);
	
	m_nLbType = BsUiLBTYPE_NORMAL;
	m_nCols = 1;
	m_nLines = 5;

	m_nCurSel = 0;
	m_nTopIndex = 0;

	m_nItemWidth = 100 / m_nCols;
	m_nItemHeight = 100 / m_nLines;

	m_bDrawSelBox = true;

	// layer 정보를 읽어서 윈도우를 설정한다.
	BsUiWindow::Create(BsUiTYPE_LISTCTRL,
		hClass, pos.x, pos.y,
		100, 100,
		XWATTR_SHOWWINDOW | XWATTR_MOVABLE,
		pParent);
}

BsUiListCtrl::~BsUiListCtrl()
{
	Release();
}

bool BsUiListCtrl::Create(BsUiCLASS hClass, int x, int y, BsUiWindow* pParent)
{
	BsAssert(hClass);
	BsAssert(pParent);

	BsUiListCtrlItem* pListCtrlItem = static_cast <BsUiListCtrlItem* > (g_BsUiSystem.GetLoader()->GetClassInstance(hClass));
	BsAssert(pListCtrlItem);
	BsAssert(pListCtrlItem->nLine > 0);
	
	m_nLbType = pListCtrlItem->nLbType;
	m_nCols = pListCtrlItem->nCols;
	m_nLines = pListCtrlItem->nLine;
	m_nItemWidth = pListCtrlItem->size.cx / pListCtrlItem->nCols;
	m_nItemHeight = pListCtrlItem->size.cy / pListCtrlItem->nLine;
	m_bDrawSelBox = pListCtrlItem->nDrawSelBox ? true : false;

	//text
	for (int i=0; i<(int)pListCtrlItem->values.size(); ++i)
	{	
		char szStr[256];
		memset(szStr, 0, 256);
		//g_TextTable->GetText(pListCtrlItem->values[i].nTextId, 256, szStr); 
		this->AddString(szStr);
		this->SetItemData(i, pListCtrlItem->values[i].dwData);
	}

	// layer 정보를 읽어서 윈도우를 설정한다.
	return BsUiWindow::Create(pListCtrlItem->nType, 
		hClass, x, y, 
		pListCtrlItem->size.cx, pListCtrlItem->size.cy,
		pListCtrlItem->nAttr, pParent);
}

void BsUiListCtrl::Release()
{
	for (int i=0; i<(int ) m_Items.size(); ++i)
	{
		delete[] m_Items[i]->szText;
		delete m_Items[i];
	}

	m_Items.clear();
}

DWORD BsUiListCtrl::ProcMessage(xwMessageToken* pMsgToken)
{
	BsAssert(pMsgToken);

	switch (pMsgToken->message)
	{
	case XWMSG_LBUTTONDOWN:
		{
			int index = HitTest(&pMsgToken->pt);
			SetCurSel(index);
		}
		break;
	case XWMSG_LBUTTONUP:
		{
		}
		break;
	case XWMSG_KEYDOWN:
		{
			switch (m_nLbType)
			{
			case BsUiLBTYPE_NORMAL:
			case BsUiLBTYPE_FIXSELPOS:
				{
					switch (pMsgToken->lParam)
					{
					case MENU_INPUT_UP:
					case MENU_INPUT_LEFT:
						{
							int index = GetCurSel() - 1;
							if (index >= 0)
								SetCurSel(index);
						}
						break;
					case MENU_INPUT_DOWN:
					case MENU_INPUT_RIGHT:
						{
							int index = GetCurSel() + 1;
							if (index < GetItemCount())
								SetCurSel(index);
						}
						break;
					}
					
					g_BsUiSystem.NotifyMessage(m_hParentWnd, this->GetHWnd(), XWMSG_KEYDOWN, 0, pMsgToken->lParam);
				}
				break;
			case BsUiLBTYPE_ICON:
				{
					switch (pMsgToken->lParam)
					{
					case MENU_INPUT_UP:
						{
							int index = GetCurSel() - m_nCols;
							if (index >= 0)
								SetCurSel(index);
							else
								SetCurSel(0);
						}
						break;
					case MENU_INPUT_DOWN:
						{
							int index = GetCurSel() + m_nCols;
							if (index < GetItemCount())
								SetCurSel(index);
							else
								SetCurSel(GetItemCount() - 1);
						}
						break;
					case MENU_INPUT_LEFT:
						{
							int index = GetCurSel() - 1;
							if (index >= 0)
								SetCurSel(index);
						}
						break;
					case MENU_INPUT_RIGHT:
						{
							int index = GetCurSel() + 1;
							if (index < GetItemCount())
								SetCurSel(index);
						}
						break;
					}
							
					g_BsUiSystem.NotifyMessage(m_hParentWnd, this->GetHWnd(), XWMSG_KEYDOWN, 0, pMsgToken->lParam);
				}
				break;
			default:
				BsAssert(false);
				break;
			}
		}
		break;

	default:
		break;
	}
	
	return BsUiWindow::ProcMessage(pMsgToken);
}

int BsUiListCtrl::SetCurSel(int nIndex)
{
	m_nCurSel = nIndex;

	if (m_nCurSel >= 0 && m_nCurSel < GetItemCount())
		UpdateTopIndex();

	return m_nCurSel;
}

void BsUiListCtrl::SetItemData(int nIndex, DWORD dwData)
{
	m_Items[nIndex]->dwData = dwData;
}

DWORD BsUiListCtrl::GetItemData(int nIndex)
{
	return m_Items[nIndex]->dwData;
}


int BsUiListCtrl::SetTopIndex( int nIndex )
{
	BsAssert(nIndex >= 0 && nIndex <= (int ) m_Items.size());
	m_nTopIndex = nIndex;
	return nIndex;
}

int BsUiListCtrl::HitTest(POINT* pPt)
{
	for (int i=0; i<GetItemCount(); ++i)
	{
		RECT rect;
		GetItemRect(i, rect);

		if (rect.left <= pPt->x && pPt->x <= rect.right &&
			rect.top <= pPt->y && pPt->y <= rect.bottom)
			return i;
	}

	return -1;
}

// 스크롤이 추가되면 스크롤에 따른 처리도 추가할 것
int BsUiListCtrl::GetCountPerPage()
{
	switch (m_nLbType)
	{
	case BsUiLBTYPE_NORMAL:
		return m_nLines;
	case BsUiLBTYPE_FIXSELPOS:
		return m_nLines;
	case BsUiLBTYPE_ICON:
		return m_nLines * m_nCols;
	default:
		BsAssert(false);
		break;
	}

	return -1;
}

void BsUiListCtrl::GetItemRect(int nIndex, RECT& rect)
{
	POINT pt = GetWindowPos();

	switch (m_nLbType)
	{
	case BsUiLBTYPE_NORMAL:
		{
			nIndex -= m_nTopIndex;
			pt.y += nIndex * m_nItemHeight;
		}
		break;
	case BsUiLBTYPE_FIXSELPOS:
		{
			nIndex -= m_nCurSel;
			pt.y = pt.y + (GetWindowSize().y - m_nItemHeight) / 2;
			pt.y += nIndex * m_nItemHeight;
		}
		break;
	case BsUiLBTYPE_ICON:
		{
			nIndex -= m_nTopIndex;
			pt.x += m_nItemWidth * (nIndex % m_nCols);
			pt.y += m_nItemHeight * (nIndex / m_nCols);
		}
		break;
	default:
		BsAssert(false);
		break;
	}

	rect.left = pt.x;
	rect.top = pt.y;
	rect.right = rect.left + m_nItemWidth;
	rect.bottom = pt.y + m_nItemHeight;
}

void BsUiListCtrl::UpdateTopIndex()
{
	switch (m_nLbType)
	{
	case BsUiLBTYPE_NORMAL:
		{
			if (m_nCurSel - m_nTopIndex >= m_nLines)
				m_nTopIndex = m_nCurSel - m_nLines + 1;
			if (m_nCurSel < m_nTopIndex)
				m_nTopIndex = m_nCurSel;
		}
		break;
	case BsUiLBTYPE_FIXSELPOS:
		{
			m_nTopIndex = m_nCurSel - m_nLines / 2;

			if (m_nTopIndex < 0)
				m_nTopIndex = 0;
		}
		break;
	case BsUiLBTYPE_ICON:
		{
			if (m_nCurSel < m_nTopIndex)
				m_nTopIndex = (m_nCurSel / m_nCols) * m_nCols;

			while (m_nCurSel >= (m_nTopIndex + m_nCols * m_nLines))
				m_nTopIndex += m_nCols;
		}
		break;
	default:
		BsAssert(false);
		break;
	}
}


void BsUiListCtrl::SetColumns(int nCols)
{ 
	m_nCols = nCols;
	m_nItemWidth = GetWindowSize().x / nCols;
}

void BsUiListCtrl::SetLines(int nLines)
{ 
	m_nLines = nLines;
	m_nItemHeight = GetWindowSize().y / nLines;
}


void BsUiListCtrl::ClearItem()
{
	Release();
}

int BsUiListCtrl::AddString(const char* pStr)
{
	int nSize = (int)m_Items.size();

	BsUiItemInfo* pInfo = new BsUiItemInfo;
	BsAssert(pInfo);

	const size_t pStr_len = strlen(pStr)+1; //aleksger - safe string
	pInfo->szText = new char[pStr_len];
	strcpy_s(pInfo->szText, pStr_len, pStr);

	pInfo->nTextID = -1;
	pInfo->dwData = -1;

	m_Items.push_back(pInfo);
	return nSize;
}

int BsUiListCtrl::DeleteString(int nIndex)
{
	BsAssert((int)m_Items.size() > nIndex);

	BsUiItemInfo* pInfo = m_Items[nIndex];
	BsAssert(pInfo);

	delete pInfo;

	m_Items.erase(m_Items.begin() + nIndex);
	return nIndex;
}

int BsUiListCtrl::FindString(int nStartAfter, const char* pStr)
{
	while (nStartAfter < (int )  m_Items.size())
	{
		if (m_Items[nStartAfter]->szText == pStr)
			return nStartAfter;

		++nStartAfter;
	}

	return -1;
}

int BsUiListCtrl::SelectString(int nStartAfter, const char* pStr)
{
	int nIndex = FindString(nStartAfter, pStr);

	if (nIndex != -1)
		SetCurSel(nIndex);

	return nIndex;
}



bool BsUiListCtrl::AddItem(char* pStr, int nItem, int nSubItem)
{
	BsAssert(nItem != -1);
	BsAssert(nSubItem != -1);

	BsUiListItem* plItem = new BsUiListItem;
	BsAssert(plItem);

	if(nSubItem != 0)
	{
		bool bItem = false;
		bool bSubItem = false;
		for(int i=0; i<(int)m_lItems.size(); i++)
		{
			if(m_lItems[i]->nItem == nItem)
			{
				bItem = true;
				for(int j=0; j<(int)m_lItems.size(); j++)
				{
					if(m_lItems[j]->nSubItem == nSubItem)
					{
						bSubItem = true;
						break;
					}
				}
			}
		}

		if(bItem == false)
		{
			BsAssert(0);
			return false;
		}

		if(bSubItem == true)
		{
			BsAssert(0);
			return false;
		}
	}

	plItem->nItem = nItem;
	plItem->nSubItem = nSubItem;

	plItem->Item.szText = pStr;
	plItem->Item.nTextID = -1;
	plItem->Item.dwData = -1;

	m_lItems.push_back(plItem);

	return true;
}


bool BsUiListCtrl::DeleteItem(int nItem)
{
	for(int i=0; i<(int)m_lItems.size(); i++)
	{
		if(m_lItems[i]->nItem == nItem)
		{
			delete m_lItems[i];

			m_lItems.erase(m_lItems.begin() + i);

			i=0;
		}
	}

	return true;
}

bool BsUiListCtrl::ChangeItem(int nItem, int nSubItem, char* pStr)
{	
	for(int i=0; i<(int)m_lItems.size(); i++)
	{
		if(m_lItems[i]->nItem == nItem && m_lItems[i]->nSubItem == nSubItem)
		{
			m_lItems[i]->Item.szText = pStr;
			return true;
		}
	}

	return false;
}

void BsUiListCtrl::SetItemData(int nItem, int nSubItem, DWORD dwData)
{
	for(int i=0; i<(int)m_lItems.size(); i++)
	{
		if(m_lItems[i]->nItem == nItem && m_lItems[i]->nSubItem == nSubItem)
		{
			m_lItems[i]->Item.dwData = dwData;
			return;
		}
	}
}

BsUiAttrId	BsUiListCtrl::GetCurFontAttr(int nIndex)
{
	if(IsEnableWindow() == false)
	{
		return m_nFontAttr[BsUiFS_DISABLE];
	}
	else if(m_nCurSel == nIndex && BsUiSystem::GetFocusWindow() == this)
	{
		return m_nFontAttr[BsUiFS_SELECTED];
	}

	return m_nFontAttr[BsUiFS_DEFAULTED];
}

void BsUiListCtrl::ResizeWindowSize(ChangeWindowSize nFlag, POINT pos)
{
	BsUiWindow::ResizeWindowSize(nFlag, pos);

	POINT size = BsUiWindow::GetWindowSize();
	m_nItemWidth = size.x / m_nCols;
	m_nItemHeight = size.y / m_nLines;

}