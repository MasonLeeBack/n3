#include "stdafx.h"

#include "BsUiSlider.h"

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

BsUiSlider::BsUiSlider() : BsUiWindow()
{
	m_sdType = BsUiSDTYPE_NORMAL;
	m_sdOrientation = BsUiSDO_HORIZONAL;
	m_ptBarSize.x = 10;
	m_ptBarSize.y = 30;

	m_nMinRange = 0;
	m_nMaxRange = 10;
	m_nTicCount = m_nMaxRange - m_nMinRange;

	for(int i=0; i<BsUiFS_COUNT; i++)
	{
		m_nBarUVID[i] = -1;
		m_nBar1UVID[i] = -1;
		m_nBar2UVID[i] = -1;
	}
	m_nLineUV = -1;
	m_nBaseUV = -1;
	m_nTicIndex = 0;
	m_bMovingTic = false;
	m_bBarPosBreak = false;
}

BsUiSlider::BsUiSlider(BsUiCLASS hClass, POINT pos, BsUiWindow* pParent) : BsUiWindow()
{
	BsAssert(hClass != 0);

	m_sdType = BsUiSDTYPE_NORMAL;
	m_sdOrientation = BsUiSDO_HORIZONAL;
	m_ptBarSize.x = 10;
	m_ptBarSize.y = 30;

	m_nMinRange = 0;
	m_nMaxRange = 10;
	m_nTicCount = m_nMaxRange - m_nMinRange;

	for(int i=0; i<BsUiFS_COUNT; i++)
	{
		m_nBarUVID[i] = -1;
		m_nBar1UVID[i] = -1;
		m_nBar2UVID[i] = -1;
	}
	m_nLineUV = -1;
	m_nBaseUV = -1;
	m_nTicIndex = 0;
	m_bMovingTic = false;
		
	BsUiWindow::Create(BsUiTYPE_SLIDER,
		hClass, pos.x, pos.y,
		200, 30,
		XWATTR_SHOWWINDOW | XWATTR_MOVABLE,
		pParent);
}


bool BsUiSlider::Create(BsUiCLASS hClass, int x, int y, BsUiWindow* pParent)
{
	BsAssert(hClass);
	BsAssert(pParent);

	BsUiSliderItem* pSliderItem = static_cast <BsUiSliderItem* > (g_BsUiSystem.GetLoader()->GetClassInstance(hClass));
	BsAssert(pSliderItem);

	m_sdType = pSliderItem->nSdType;
	m_sdOrientation = pSliderItem->nSdOrientation;
	m_ptBarSize = pSliderItem->ptBarSize;
	m_nTicIndex = 0;
	SetRange(pSliderItem->nMinRange, pSliderItem->nMaxRange);

	for(int i=0; i<BsUiFS_COUNT; i++)
	{
		m_nBarUVID[i] = pSliderItem->nBarUVID[i];
		m_nBar1UVID[i] = pSliderItem->nBar1UVID[i];
		m_nBar2UVID[i] = pSliderItem->nBar2UVID[i];
	}
	m_nLineUV = pSliderItem->nLineUV;
	m_nBaseUV = pSliderItem->nBaseUV;
	
	m_bMovingTic = false;
	m_nCurOffset = 0;

	// layer 정보를 읽어서 윈도우를 설정한다.
	return BsUiWindow::Create(pSliderItem->nType, 
		hClass, x, y, 
		pSliderItem->size.cx, pSliderItem->size.cy, 
		pSliderItem->nAttr, pParent);
}

void BsUiSlider::Release()
{
	for (int i=0; i<(int ) m_Items.size(); ++i)
	{
		delete[] m_Items[i]->szText;
		delete m_Items[i];
	}

	m_Items.clear();
}

DWORD BsUiSlider::ProcMessage(xwMessageToken* pMsgToken)
{
	BsAssert(pMsgToken);
	
	switch (pMsgToken->message)
	{
	case XWMSG_LBUTTONDOWN:
		{
			switch(m_sdType)
			{
			case BsUiSDTYPE_NORMAL:
			case BsUiSDTYPE_GAUGE:
				{
					if (IsHitTic(pMsgToken->pt.x, pMsgToken->pt.y))
					{
						POINT pt;
						GetTicPos(pt);

						m_ptMovingTicGap.x = pMsgToken->pt.x - pt.x;
						m_ptMovingTicGap.y = pMsgToken->pt.y - pt.y;
						m_bMovingTic = true;

						BsUiSystem::SetGrabWindow(this);
					}
					else
					{
						m_bMovingTic = false;
					}
					break;
				}
			case BsUiSDTYPE_NUMERAL:
			case BsUiSDTYPE_TEXT:
				{
					break;	
				}
			}
		}
		break;
	case XWMSG_LBUTTONUP:
		{
			switch(m_sdType)
			{
			case BsUiSDTYPE_NORMAL:
			case BsUiSDTYPE_GAUGE:
				{
					if (m_bMovingTic)
					{
						if (BsUiSystem::GetGrabWindow() == this)
							BsUiSystem::SetGrabWindow(NULL);
					}

					m_bMovingTic = false;

					if (GetWindowOnPos(&pMsgToken->pt) == this)
					{
						// tic 위치이동
						m_nTicIndex = GetTicIndex(pMsgToken->pt.x, pMsgToken->pt.y);

						// 알린다.
						xwMsg_NotifySlider info;
						
						info.nMinRange = m_nMinRange;
						info.nMaxRange = m_nMaxRange;
						info.nCurIndex = m_nTicIndex;

						g_BsUiSystem.NotifyMessage(m_hParentWnd, this->GetHWnd(), XWMSG_SD_CHANGED, (WPARAM)&info);
					}

					break;
				}
			case BsUiSDTYPE_NUMERAL:
			case BsUiSDTYPE_TEXT:		break;
			}
		}
		break;
	case XWMSG_MOUSEMOVE:
		{
			switch(m_sdType)
			{
			case BsUiSDTYPE_NORMAL:
			case BsUiSDTYPE_GAUGE:
				{
					if (m_bMovingTic)
					{
						// tic 위치이동
						m_nTicIndex = GetTicIndex(pMsgToken->pt.x, pMsgToken->pt.y);

						// 알린다.
						xwMsg_NotifySlider info;
						
						info.nMinRange = m_nMinRange;
						info.nMaxRange = m_nMaxRange;
						info.nCurIndex = m_nTicIndex;

						g_BsUiSystem.NotifyMessage(m_hParentWnd, this->GetHWnd(), XWMSG_SD_CHANGED, (WPARAM)&info);
					}
					break;
				}
			case BsUiSDTYPE_NUMERAL:
			case BsUiSDTYPE_TEXT:		break;
			}
		}
		break;
	case XWMSG_CMDSD_SETTIC:
		{
			switch(m_sdType)
			{
			case BsUiSDTYPE_NORMAL:
			case BsUiSDTYPE_GAUGE:
				{
					SetBarPos(pMsgToken->wParam);
					break;
				}
			case BsUiSDTYPE_NUMERAL:
			case BsUiSDTYPE_TEXT: break;
			}
		}
		break;
	case XWMSG_CMDSD_SETRANGE:
		{
			switch(m_sdType)
			{
			case BsUiSDTYPE_NORMAL:
			case BsUiSDTYPE_GAUGE:
				{
					SetRange(pMsgToken->wParam, pMsgToken->lParam);
					break;
				}
			case BsUiSDTYPE_NUMERAL:
			case BsUiSDTYPE_TEXT: break;
			}
		}
		break;
	case XWMSG_KEYDOWN:
		{
			switch(m_sdType)
			{
			case BsUiSDTYPE_NORMAL:
			case BsUiSDTYPE_NUMERAL:
			case BsUiSDTYPE_TEXT:
			case BsUiSDTYPE_GAUGE:
				{
					switch (pMsgToken->lParam)
					{
					case MENU_INPUT_LEFT:
						{
							if(m_sdOrientation == BsUiSDO_HORIZONAL)
								AddBarPos(false);
						}
						break;
					case MENU_INPUT_RIGHT:
						{
							if(m_sdOrientation == BsUiSDO_HORIZONAL)
								AddBarPos(true);
						}
						break;
					case MENU_INPUT_UP:
						{
							if(m_sdOrientation == BsUiSDO_VERTICAL)
								AddBarPos(true);
						}
						break;
					case MENU_INPUT_DOWN:
						{
							if(m_sdOrientation == BsUiSDO_VERTICAL)
								AddBarPos(false);
						}
						break;
					}
					break;
				}
			}

			g_BsUiSystem.NotifyMessage(m_hParentWnd, this->GetHWnd(), XWMSG_KEYDOWN, 0, pMsgToken->lParam);
		}
		break;

	default:
		break;
	}

	return BsUiWindow::ProcMessage(pMsgToken);
}


int BsUiSlider::AddBarPos(bool bAdd)
{
	if(m_bBarPosBreak == true){
		return (m_nTicIndex + m_nMinRange);
	}

	int nBarPos = GetBarPos();
	if(bAdd == true)
	{	
		if(nBarPos+1 > m_nMaxRange)
		{
			//gamenote : Slider 내에서 이동 못할때..
			return nBarPos;
		}
		nBarPos += 1;
#ifndef _BSUI_TOOL
		g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CURSOR");
#endif
		//gamenote : Slider 내에서 이동할때..
	}
	else
	{	
		if(nBarPos-1 < m_nMinRange)
		{
			//gamenote : Slider 내에서 이동 못할때..
			return nBarPos;
		}
		nBarPos -= 1;
#ifndef _BSUI_TOOL
		g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CURSOR");
#endif
		//gamenote : Slider 내에서 이동할때..
	}

	return SetBarPos(nBarPos);
}

int BsUiSlider::SetBarPos(int n)
{
	BsAssert(n >= m_nMinRange && n <= m_nMaxRange);
	m_nTicIndex = n - m_nMinRange;
	return (m_nTicIndex + m_nMinRange);
}

int BsUiSlider::SetRange(int nMin, int nMax)
{
	BsAssert(nMin < nMax);

	m_nMinRange = nMin;
	m_nMaxRange = nMax;

	if (m_nTicIndex > (m_nMaxRange - m_nMinRange))
		m_nTicIndex = m_nMaxRange - m_nMinRange;

	m_nTicCount = nMax - nMin;

	return 0;
}

bool BsUiSlider::IsHitTic(int x, int y)
{
	RECT rect;
	GetWindowRect(&rect);

	if (rect.left <= x && x < rect.right && rect.top <= y && y < rect.bottom)
		return true;

	return false;
}

void BsUiSlider::GetTicPos(POINT& pt)
{
	switch (m_sdOrientation)
	{
	case BsUiSDO_HORIZONAL:
		{
			int nWidth = (GetWindowSize().x - m_ptBarSize.x);
			int dx = m_nTicIndex * nWidth / m_nTicCount;

			pt.x = dx + GetWindowPos().x;
			pt.y = GetWindowPos().y + (GetWindowSize().y / 2);
		}
		break;
	case BsUiSDO_VERTICAL:
		{
			int nHeight = (GetWindowSize().y - m_ptBarSize.y);
			int dy = m_nTicIndex * nHeight / m_nTicCount;

			pt.x = GetWindowPos().x + (GetWindowSize().x / 2);
			pt.y = dy + GetWindowPos().y;
		}
		break;
	default:
		BsAssert(false);
		break;
	}
}

int BsUiSlider::GetTicIndex(int x, int y)
{
	int index = -1;
	switch (m_sdOrientation)
	{
	case BsUiSDO_HORIZONAL:
		{
			int dx = x - (GetWindowPos().x + m_ptBarSize.x / 2);
			int nValue = GetWindowSize().x - m_ptBarSize.x;
			if(nValue == 0)
				nValue = 1;
			index = dx * m_nTicCount / nValue;
		}
		break;
	case BsUiSDO_VERTICAL:
		{
			int dy = y - (GetWindowPos().y + m_ptBarSize.y / 2);
			int nValue = GetWindowSize().y - m_ptBarSize.y;
			if(nValue == 0)
				nValue = 1;
			index = dy * m_nTicCount / nValue;
		}
		break;
	default:
		BsAssert(false);
		break;
	}

	if (index < 0)
		index = 0;
	if (index > m_nTicCount)
		index = m_nTicCount;

	return index;
}

int	BsUiSlider::GetCurBarUVID()
{
	return m_nBarUVID[BsUiWindow::GetCurFocusState()];
}

int	BsUiSlider::GetCurBar1UVID()
{
	BsUiFocusState focusState = BsUiWindow::GetCurFocusState();
	if(focusState == BsUiFS_SELECTED)
	{
		if(GetBarPos() == m_nMinRange)
			return m_nBar1UVID[BsUiFS_DEFAULTED];
	}
	return m_nBar1UVID[focusState];
}

int	BsUiSlider::GetCurBar2UVID()
{
	BsUiFocusState focusState = BsUiWindow::GetCurFocusState();
	if(focusState == BsUiFS_SELECTED)
	{
		if(GetBarPos() == m_nMaxRange)
			return m_nBar2UVID[BsUiFS_DEFAULTED];
	}
	return m_nBar2UVID[focusState];
}

void BsUiSlider::GetLineRect(RECT* rect)
{	
	GetWindowRect(rect);
	if(m_sdOrientation == BsUiSDO_HORIZONAL)
	{
		int nY = rect->top + (rect->bottom - rect->top)/2;
		rect->top = nY-2;
		rect->bottom = nY+2;
	}
	else
	{
		int nX = rect->left + (rect->right - rect->left)/2;
		rect->left = nX-2;
		rect->right= nX+2;
	}
}

void BsUiSlider::GetBarRect(RECT* rect)
{
	POINT ptCur;
	GetTicPos(ptCur);
	
	if(m_sdOrientation == BsUiSDO_HORIZONAL)
	{
		rect->left = ptCur.x;
		rect->right = ptCur.x + m_ptBarSize.x;
		rect->top = ptCur.y - m_ptBarSize.y/2;
		rect->bottom = ptCur.y + m_ptBarSize.y/2;
	}
	else
	{
		rect->left = ptCur.x - m_ptBarSize.x/2;
		rect->right = ptCur.x + m_ptBarSize.x/2;
		rect->top = ptCur.y;
		rect->bottom = ptCur.y + m_ptBarSize.y;
	}
}

void BsUiSlider::GetBar1Rect(RECT* rect)
{	
	POINT ptPos = GetWindowPos();
	POINT ptSize = GetWindowSize();
	
	if(m_sdOrientation == BsUiSDO_HORIZONAL)
	{
		rect->left = ptPos.x;
		rect->right = ptPos.x + m_ptBarSize.x;
		rect->top = (ptPos.y + (ptSize.y/2)) - (m_ptBarSize.y/2);
		rect->bottom = (ptPos.y + (ptSize.y/2)) + (m_ptBarSize.y/2);
	}
	else
	{
		rect->left = (ptPos.x + (ptSize.x/2)) - (m_ptBarSize.x/2);
		rect->right = (ptPos.x + (ptSize.x/2)) + (m_ptBarSize.x/2);
		rect->top = ptPos.y;
		rect->bottom = ptPos.y + m_ptBarSize.y;
	}
}

void BsUiSlider::GetBar2Rect(RECT* rect)
{
	POINT ptPos = GetWindowPos();
	POINT ptSize = GetWindowSize();
	
	if(m_sdOrientation == BsUiSDO_HORIZONAL)
	{	
		rect->left = (ptPos.x + ptSize.x) - m_ptBarSize.x;
		rect->right = ptPos.x + ptSize.x;
		rect->top = (ptPos.y + (ptSize.y/2)) - (m_ptBarSize.y/2);
		rect->bottom = (ptPos.y + (ptSize.y/2)) + (m_ptBarSize.y/2);
	}
	else
	{
		rect->left = (ptPos.x + (ptSize.x/2)) - (m_ptBarSize.x/2);
		rect->right = (ptPos.x + (ptSize.x/2)) + (m_ptBarSize.x/2);
		rect->top = (ptPos.y + ptSize.y) - m_ptBarSize.y;
		rect->bottom = ptPos.y + ptSize.y;
	}
}

void BsUiSlider::GetGaugeBarRect(RECT* rect)
{
	POINT ptPos = GetWindowPos();
	POINT ptSize = GetWindowSize();
	POINT pt;
	GetTicPos(pt);

	if(m_sdOrientation == BsUiSDO_HORIZONAL)
	{	
		rect->left = ptPos.x;
		rect->right = pt.x;
		rect->top = (ptPos.y + (ptSize.y/2)) - (m_ptBarSize.y/2);
		rect->bottom = (ptPos.y + (ptSize.y/2)) + (m_ptBarSize.y/2);
	}
	else
	{
		rect->left = (ptPos.x + (ptSize.x/2)) - (m_ptBarSize.x/2);
		rect->right = (ptPos.x + (ptSize.x/2)) + (m_ptBarSize.x/2);
		rect->top = ptPos.y;
		rect->bottom = pt.y;
	}
}

BsUiItemInfo* BsUiSlider::GetCurItem()
{
	if(m_nTicIndex >= (int)m_Items.size())
		return NULL;

	return m_Items[m_nTicIndex];
}

int BsUiSlider::AddItem(char* szText, DWORD dwData)
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

int BsUiSlider::AddItem(int nTextID, DWORD dwData)
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

int BsUiSlider::RemoveItem(int nIndex)
{
	BsAssert((int)m_Items.size() > nIndex);

	BsUiItemInfo* pInfo = m_Items[nIndex];
	BsAssert(pInfo);

	delete[] pInfo->szText;
	delete pInfo;

	m_Items.erase(m_Items.begin() + nIndex);
	return nIndex;
}


void BsUiSlider::ClearItem()
{
	Release();
}