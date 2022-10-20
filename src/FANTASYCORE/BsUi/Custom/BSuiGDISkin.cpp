#include "stdafx.h"
#include "BsUiGDISkin.h"

#ifndef _BSUI_TOOL
#include "FcGlobal.h"
#endif

#include "BsUiSystem.h"
#include "BsKernel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


BsUiGDISkin::BsUiGDISkin()
{
	m_bSelectedBox = false;
	m_rectSelected.left = m_rectSelected.right = 0;
	m_rectSelected.top = m_rectSelected.bottom = 0;
}

BsUiGDISkin::~BsUiGDISkin()
{
}

int	BsUiGDISkin::GetTime()
{
	return GetProcessTick();
}

bool BsUiGDISkin::IsCanMove(BsUiWindow* pWnd, POINT pt)
{	
	RECT rect;
	pWnd->GetWindowRect(&rect);

	RECT MoveRect;
	MoveRect.left = rect.left;
	MoveRect.top = rect.top;
	MoveRect.right = rect.left + 10;
	MoveRect.bottom = rect.top + 10;

	if(pt.x >= MoveRect.left && pt.x <= MoveRect.right &&
		pt.y >= MoveRect.top && pt.y <= MoveRect.bottom)
	{
		return true;
	}

	return false;
}

int BsUiGDISkin::GetCanSizeState(BsUiWindow* pWnd, POINT pt)
{
	RECT rect;
	pWnd->GetWindowRect(&rect);

	RECT pointRect;
	for(int i=_CHANGE_SIZE_LEFT; i<_END_CHANGE_SIZE; i++)
	{
		switch(i)
		{
		case _CHANGE_SIZE_LEFT:	
			{
				pointRect.left = rect.left;
				pointRect.top = rect.top + (rect.bottom - rect.top) / 2 - 5;
				pointRect.right = rect.left+10;
				pointRect.bottom = rect.top + (rect.bottom - rect.top) / 2 + 5;
				break;
			}
		case _CHANGE_SIZE_RIGHT:
			{
				pointRect.left = rect.right-10;
				pointRect.top = rect.top + (rect.bottom - rect.top) / 2 - 5;
				pointRect.right = rect.right;
				pointRect.bottom = rect.top + (rect.bottom - rect.top) / 2 + 5;
				break;
			}
		case _CHANGE_SIZE_TOP:
			{
				pointRect.left = rect.left + (rect.right - rect.left) / 2 - 5;
				pointRect.top = rect.top; 
				pointRect.right = rect.left + (rect.right - rect.left) / 2 + 5;
				pointRect.bottom = rect.top + 10;
				break;
			}
		case _CHANGE_SIZE_BOTTOM:
			{
				pointRect.left = rect.left + (rect.right - rect.left) / 2 - 5;
				pointRect.top = rect.bottom - 10;
				pointRect.right = rect.left + (rect.right - rect.left) / 2 + 5;
				pointRect.bottom = rect.bottom;
				break;
			}
		case _CHANGE_SIZE_LEFTTOP:
			{
				//pointRect.left = rect.left;
				//pointRect.top = rect.top; 
				//pointRect.right = rect.left + 10;
				//pointRect.bottom = rect.top + 10;
				break;
			}
		case _CHANGE_SIZE_LEFTBOTTOM: 
			{
				pointRect.left = rect.left;
				pointRect.top = rect.bottom - 10;
				pointRect.right = rect.left + 10;
				pointRect.bottom = rect.bottom;
				break;
			}
		case _CHANGE_SIZE_RIGHTTOP:
			{
				pointRect.left = rect.right - 10;
				pointRect.top = rect.top;
				pointRect.right = rect.right;
				pointRect.bottom = rect.top + 10;
				break;
			}
		case _CHANGE_SIZE_RIGHTBOTTOM:
			{
				pointRect.left =  rect.right - 10;
				pointRect.top = rect.bottom - 10;
				pointRect.right = rect.right;
				pointRect.bottom = rect.bottom;
				break;
			}
		default: BsAssert(0);
		}

		if(pt.x >= pointRect.left && pt.x <= pointRect.right &&
			pt.y >= pointRect.top && pt.y <= pointRect.bottom)
			return (ChangeWindowSize)i;
	}

	return _NOT_CHANGE_SIZE;
}


void BsUiGDISkin::SetSelectedBox(bool bFlag, POINT* point)
{
	if(bFlag)
	{	
		m_bSelectedBox = true;
		m_rectSelected.left = m_rectSelected.right = point->x;
		m_rectSelected.top = m_rectSelected.bottom = point->y;
	}
	else
	{
		m_bSelectedBox = false;

		RECT rect;
		rect.left = (m_rectSelected.left <= m_rectSelected.right) ? m_rectSelected.left : m_rectSelected.right;
		rect.top = (m_rectSelected.top <= m_rectSelected.bottom) ? m_rectSelected.top : m_rectSelected.bottom;

		rect.right = (m_rectSelected.left < m_rectSelected.right) ? m_rectSelected.right : m_rectSelected.left;
		rect.bottom = (m_rectSelected.top < m_rectSelected.bottom) ? m_rectSelected.bottom : m_rectSelected.top;

		g_BsUiSystem.SetSelectedWindows(rect);

		m_rectSelected.left = m_rectSelected.right = 0;
		m_rectSelected.top = m_rectSelected.bottom = 0;
	}
}

void BsUiGDISkin::MoveSelectedBox(POINT* point)
{
	m_rectSelected.right = point->x;
	m_rectSelected.bottom = point->y;
/*
	RECT rect;
	rect.left = (m_rectSelected.left <= m_rectSelected.right) ? m_rectSelected.left : m_rectSelected.right;
	rect.top = (m_rectSelected.top <= m_rectSelected.bottom) ? m_rectSelected.top : m_rectSelected.bottom;

	rect.right = (m_rectSelected.left < m_rectSelected.right) ? m_rectSelected.right : m_rectSelected.left;
	rect.bottom = (m_rectSelected.top < m_rectSelected.bottom) ? m_rectSelected.bottom : m_rectSelected.top;
	
	g_BsUiSystem.SetSelectedWindows(rect);
	*/
}

