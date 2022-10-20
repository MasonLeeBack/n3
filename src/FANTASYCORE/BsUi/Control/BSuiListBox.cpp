#include "stdafx.h"

#include "BsUiListBox.h"

#include "BsUiSystem.h"

#include "BsUiMenuInput.h"
#ifndef _BSUI_TOOL
#include "FcSoundManager.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

BsUiListBox::BsUiListBox() : BsUiWindow()
{
	m_nCurSel = 0;
	m_nTopIndex = 0;
	m_bDrawSelBox = true;

	for(int i=0; i<BsUiFS_COUNT; i++)
	{
		m_nSelBoxUVID[i] = -1;
		m_nSelBoxBlockID[i] = -1;
	}
}

BsUiListBox::BsUiListBox(BsUiCLASS hClass, POINT pos, BsUiWindow* pParent) : BsUiWindow()
{
	BsAssert(hClass);

	m_nLines = 5;

	m_nCurSel = 0;
	m_nTopIndex = 0;

	m_nItemWidth = 100;
	m_nItemHeight = 100 / m_nLines;

	m_bDrawSelBox = true;

	for(int i=0; i<BsUiFS_COUNT; i++)
	{
		m_nSelBoxUVID[i] = -1;
		m_nSelBoxBlockID[i] = -1;
	}

	// layer 정보를 읽어서 윈도우를 설정한다.
	BsUiWindow::Create(BsUiTYPE_LISTBOX,
		hClass, pos.x, pos.y,
		100, 100,
		XWATTR_SHOWWINDOW | XWATTR_MOVABLE,
		pParent);
}

BsUiListBox::~BsUiListBox()
{
	Release();
}

bool BsUiListBox::Create(BsUiCLASS hClass, int x, int y, BsUiWindow* pParent)
{
	BsAssert(hClass);
	BsAssert(pParent);

	BsUiListBoxItem* pListBoxItem = static_cast <BsUiListBoxItem* > (g_BsUiSystem.GetLoader()->GetClassInstance(hClass));
	BsAssert(pListBoxItem);
	BsAssert(pListBoxItem->nLine > 0);
	
	
	m_nLines = pListBoxItem->nLine;
	m_nItemWidth = pListBoxItem->size.cx;
	m_nItemHeight = pListBoxItem->size.cy / pListBoxItem->nLine;
	m_bDrawSelBox = pListBoxItem->nDrawSelBox ? true : false;

	for(int i=0; i<BsUiFS_COUNT; i++)
	{
		BsUiFocusState flag = (BsUiFocusState)i;
		SetSelBoxUVID(flag, pListBoxItem->nSelBoxUVID[flag]);
		SetSelBoxBlock(flag, pListBoxItem->nSelBoxBlockID[flag]);
	}
	
	//text
	BsUiVALUEITEMVECT* pItemList = &(pListBoxItem->values);
	for (DWORD i=0; i<pItemList->size(); ++i)
	{	
		BsUiValueItem Item = (*pItemList)[i];
		this->AddItem(Item.nID, Item.dwData);
	}

	// layer 정보를 읽어서 윈도우를 설정한다.
	return BsUiWindow::Create(pListBoxItem->nType, 
		hClass, x, y, 
		pListBoxItem->size.cx, pListBoxItem->size.cy,
		pListBoxItem->nAttr, pParent);
}

void BsUiListBox::Release()
{
	for (int i=0; i<(int ) m_Items.size(); ++i)
	{
		delete[] m_Items[i]->szText;
		delete m_Items[i];
	}

	m_Items.clear();
}

DWORD BsUiListBox::ProcMessage(xwMessageToken* pMsgToken)
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
			switch (pMsgToken->lParam)
			{
			case MENU_INPUT_UP:
				{
					int index = GetCurSel() - 1;
					if (index >= 0)
					{
						SetCurSel(index);
						//gamenote : List 내에서 이동할때..
#ifndef _BSUI_TOOL
						g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CURSOR");
#endif
					}
				}
				break;
			case MENU_INPUT_DOWN:
				{
					int index = GetCurSel() + 1;
					if (index < GetItemCount())
					{
						SetCurSel(index);
						//gamenote : List 내에서 이동할때..
#ifndef _BSUI_TOOL
						g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CURSOR");
#endif
					}
				}
				break;
			}

			g_BsUiSystem.NotifyMessage(m_hParentWnd, this->GetHWnd(), XWMSG_KEYDOWN, 0, pMsgToken->lParam);
		}
		break;

	default:
		break;
	}
	
	return BsUiWindow::ProcMessage(pMsgToken);
}

int BsUiListBox::SetCurSel(int nIndex)
{
	m_nCurSel = nIndex;

	if (m_nCurSel >= 0 && m_nCurSel < GetItemCount())
		UpdateTopIndex();

	return m_nCurSel;
}

int BsUiListBox::AddItem(char* szText, DWORD dwData)
{
	if(szText == NULL)
		return -1;

	int nSize = (int)m_Items.size();

	BsUiItemInfo* pInfo = new BsUiItemInfo;
	BsAssert(pInfo);

	const size_t szText_len = strlen(szText)+1; //aleksger - safe string
	pInfo->szText = new char[szText_len];
	strcpy_s(pInfo->szText, szText_len, szText);

	pInfo->nTextID = -1;
	pInfo->dwData = dwData;

	m_Items.push_back(pInfo);
	return nSize;
}

int BsUiListBox::AddItem(int nTextID, DWORD dwData)
{
	char szText[256];
	if(g_BsUiSystem.GetView()->GetText(nTextID, szText, _countof(szText)) == false) //aleksger - safe string
		return -1;

	int nSize = (int)m_Items.size();

	BsUiItemInfo* pInfo = new BsUiItemInfo;
	BsAssert(pInfo);
		
	const size_t szText_len = strlen(szText)+1; //aleksger - safe string
	pInfo->szText = new char[szText_len];
	strcpy_s(pInfo->szText, szText_len, szText);

	pInfo->nTextID = nTextID;
	pInfo->dwData = dwData;

	m_Items.push_back(pInfo);
	return nSize;
}

int BsUiListBox::RemoveItem(int nIndex)
{
	BsAssert((int)m_Items.size() > nIndex);

	BsUiItemInfo* pInfo = m_Items[nIndex];
	BsAssert(pInfo);

	delete[] pInfo->szText;
	delete pInfo;

	m_Items.erase(m_Items.begin() + nIndex);
	return nIndex;
}


void BsUiListBox::ClearItem()
{
	SetTopIndex(0);
	Release();
}

int BsUiListBox::FindItem(int nStartAfter, const char* pStr)
{
	while (nStartAfter < (int)m_Items.size())
	{
		if (m_Items[nStartAfter]->szText == pStr)
			return nStartAfter;

		++nStartAfter;
	}

	return -1;
}

int BsUiListBox::FindItem(int nStartAfter, int nTextID)
{
	while (nStartAfter < (int)m_Items.size())
	{
		if (m_Items[nStartAfter]->nTextID == nTextID)
			return nStartAfter;

		++nStartAfter;
	}

	return -1;
}

int BsUiListBox::FindItem(int nStartAfter, DWORD dwData)
{
	while (nStartAfter < (int)m_Items.size())
	{
		if (m_Items[nStartAfter]->dwData == dwData)
			return nStartAfter;

		++nStartAfter;
	}

	return -1;
}

int BsUiListBox::SelectItem(int nStartAfter, const char* pStr)
{
	int nIndex = FindItem(nStartAfter, pStr);

	if (nIndex != -1)
		SetCurSel(nIndex);

	return nIndex;
}

int BsUiListBox::SelectItem(int nStartAfter, int nTextID)
{
	int nIndex = FindItem(nStartAfter, nTextID);

	if (nIndex != -1)
		SetCurSel(nIndex);

	return nIndex;
}

int BsUiListBox::SelectItem(int nStartAfter, DWORD dwData)
{
	int nIndex = FindItem(nStartAfter, dwData);

	if (nIndex != -1)
		SetCurSel(nIndex);

	return nIndex;
}

bool BsUiListBox::SetItemText(int nIndex, const char* szText)
{
	if((int)m_Items.size() <= nIndex)
		return false;

	BsUiItemInfo* pItem = m_Items[nIndex];

	if(pItem->szText != NULL)
		delete[] pItem->szText;

	const size_t szText_len = strlen(szText)+1; //aleksger - safe string
	pItem->szText = new char[szText_len];
	strcpy_s(pItem->szText, szText_len, szText);
	
	return true;
}
char* BsUiListBox::GetItemText(int nIndex)
{
	if((int)m_Items.size() <= nIndex)
		return false;

	return m_Items[nIndex]->szText;
}

bool BsUiListBox::SetItemTextId(int nIndex, int nTextId)
{
	if((int)m_Items.size() <= nIndex)
		return false;

	char szText[256];
	if(g_BsUiSystem.GetView()->GetText(nTextId, szText, _countof(szText)) == false)//aleksger - safe string
		return false;

	BsUiItemInfo* pItem = m_Items[nIndex];
	if(pItem->szText != NULL)
		delete[] pItem->szText;

	const size_t szText_len = strlen(szText)+1; //aleksger - safe string
	pItem->szText = new char[szText_len];
	strcpy_s(pItem->szText, szText_len, szText);

	pItem->dwData = nTextId;
	return true;
}

int BsUiListBox::GetItemTextId(int nIndex)
{
	if((int)m_Items.size() <= nIndex)
		return false;

	return m_Items[nIndex]->nTextID;
}

bool BsUiListBox::SetItemData(int nIndex, DWORD dwData)
{
	if((int)m_Items.size() <= nIndex)
		return false;

	m_Items[nIndex]->dwData = dwData;
	return true;
}

DWORD BsUiListBox::GetItemData(int nIndex)
{
	if((int)m_Items.size() <= nIndex)
		return false;

	return m_Items[nIndex]->dwData;
}


int BsUiListBox::SetTopIndex( int nIndex )
{
	BsAssert(nIndex >= 0 && nIndex <= (int ) m_Items.size());
	m_nTopIndex = nIndex;
	return nIndex;
}

int BsUiListBox::HitTest(POINT* pPt)
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
int BsUiListBox::GetPageCount()
{
	if(m_nLines == 0){
		return 0;
	}

	return m_nCurSel / m_nLines;
}

int	BsUiListBox::GetLastPage()
{
	if(m_nLines == 0){
		return 0;
	}

	return m_Items.size() / m_nLines;
}

void BsUiListBox::GetItemRect(int nIndex, RECT& rect)
{
	POINT pt = GetWindowPos();

	nIndex -= m_nTopIndex;
	pt.y += nIndex * m_nItemHeight;
	
	rect.left = pt.x;
	rect.top = pt.y;
	rect.right = rect.left + m_nItemWidth;
	rect.bottom = pt.y + m_nItemHeight;
}

void BsUiListBox::UpdateTopIndex()
{
	if (m_nCurSel - m_nTopIndex >= m_nLines)
		m_nTopIndex = m_nCurSel - m_nLines + 1;
	if (m_nCurSel < m_nTopIndex)
		m_nTopIndex = m_nCurSel;
}

void BsUiListBox::SetLines(int nLines)
{ 
	m_nLines = nLines;
	m_nItemHeight = GetWindowSize().y / nLines;
}


void BsUiListBox::ResizeWindowSize(ChangeWindowSize nFlag, POINT pos)
{
	BsUiWindow::ResizeWindowSize(nFlag, pos);

	POINT size = BsUiWindow::GetWindowSize();
	m_nItemWidth = size.x;
	m_nItemHeight = size.y / m_nLines;
}

BsUiAttrId	BsUiListBox::GetCurFontAttr(int nIndex)
{
	BsUiFocusState focusState = BsUiWindow::GetCurFocusState();
	if(focusState == BsUiFS_SELECTED || focusState == BsUiFS_DEFAULTED)
	{
		if(m_nCurSel == nIndex){
			return m_nFontAttr[BsUiFS_SELECTED];
		}

		return m_nFontAttr[BsUiFS_DEFAULTED];
	}

	return m_nFontAttr[focusState];
}

int	BsUiListBox::GetCurSelBoxUVID()
{
	return m_nSelBoxUVID[BsUiWindow::GetCurFocusState()];
}

int BsUiListBox::GetCurSelBoxBlockID()
{
	return m_nSelBoxBlockID[BsUiWindow::GetCurFocusState()];
}