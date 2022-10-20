#pragma once

// BsUiSkin.h: BsUi의 몇가지 사용자 판단을 위임할 객체이다.

// jazzcake@hotmail.com
// 2004.0625

#include "BsUi.h"
#include "BsUiWindow.h"
//#include "BsUiLoader.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//


class BsUiSkin {
protected:
	BsUiDragDropInfo			m_ddInfo;

public:
	BsUiSkin();
	virtual ~BsUiSkin();

	virtual void		Release() {;}
	virtual bool		HookMessage(xwMessageToken* pMsg) { return false; }												// 가로채지 않으면 return false

public:	// Drag&Drop 관련
	virtual bool		ReadyToDrag(BsUiWindow* pWnd, xwMessageToken* pMsgButtonDown, int& nTypeObject, DWORD& dwData) { return false; }	// true를 돌려주면 D&D 준비를 한다.
	virtual bool		DragObject(BsUiWindow* pHoverWnd, xwMessageToken* pMsgMouseMove);												// true를 돌려주면 D&D 시작을 한다.
	virtual void		DropObject(BsUiWindow* pDropWnd, xwMessageToken* pMsgButtonUp) {;}

	void				BeginDragDropInfo()							{ m_ddInfo.bDragging = true; }
	void				ClearDragDropInfo()							{ memset(&m_ddInfo, 0, sizeof(BsUiDragDropInfo)); }
	void				GetDragDropInfo(BsUiDragDropInfo* pInfo)	{ memcpy(pInfo, &m_ddInfo, sizeof(BsUiDragDropInfo)); }
	void				SetDragDropInfo(BsUiDragDropInfo* pInfo)	{ memcpy(&m_ddInfo, pInfo, sizeof(BsUiDragDropInfo)); }
	bool				IsReadyToDrag()								{ return (m_ddInfo.hSourceHwnd != 0 || m_ddInfo.hSourceClass != 0); }
	bool				IsDraggingObject()							{ return m_ddInfo.bDragging; }

	virtual bool		IsCanMove(BsUiWindow* pWnd, POINT pt);		// window item moving 관련
	virtual int			GetCanSizeState(BsUiWindow* pWnd, POINT pt) { return _NOT_CHANGE_SIZE; }
	
	virtual int			GetTime()									{ return 0; }


	
};

#define _BsUi_TIME_BEGIN_DRAGDROP			100				// ms 단위

