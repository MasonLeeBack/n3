#pragma once

// BsUiWindow.h: BsUi의 기본 window 객체, 영역에 대한 처리만 담당하는 추상적인 윈도우 객체이다. 

// jazzcake@hotmail.com
// 2004.2.9

#include <list>
#include <algorithm>

#include "BsUi.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

typedef int				BsUiAttrId;			// font attribute index

typedef struct _BsUiItemInfo {
	_BsUiItemInfo()
	{	
		nTextID = -1;
		dwData = -1;
		szText = NULL;
	};

	int						nTextID;
	char*					szText;					// column string
	DWORD					dwData;					// additional data
} BsUiItemInfo;

// 기반이 되는 window 객체를 정한다.
class BsUiWindow {
public:
	typedef list <BsUiWindow* >				BsUiWINDOWLIST;
	typedef BsUiWINDOWLIST::iterator		BsUiWINDOWLISTITOR;
	typedef deque <BsUiWindow* >			BsUiWINDOWDEQUE;
	typedef BsUiWINDOWDEQUE::iterator		BsUiWINDOWDEQUEITOR;

private:
	BsUiHANDLE			m_hWnd;				// 윈도우 핸들
	BsUiCLASS			m_hClass;			// 클래스 핸들 (윈도우 리소스)
	BsUiTYPE			m_type;				// 해당 window의 type
	POINT				m_pos;				// 해당 window의 위치 (절대 위치임)
	POINT				m_size;				// 해당 window의 전체 크기 (title-bar가 있을 시에 title-bar 포함)

protected:
	BsUiWINDOWLIST		m_child;			// 해당 window에 물린 child window
	BsUiWINDOWLIST		m_tabOrder;			// 해당 window에 tab-order로 정렬된 child window들

protected:
	BsUiWindow*			m_pParentWnd;		// 부모 윈도우
	BsUiHANDLE			m_hParentWnd;

	DWORD				m_attrDef;			// 생성 시 해당 윈도우의 속성
	DWORD				m_attrCur;			// 현재 해당 윈도우의 속성
	bool				m_bModalBox;		// modal box인가?

protected:
	bool				m_bMoving;			// 현재 움직이는 중인가?
	POINT				m_ptMovingGap;		// window move에서 적용할 버튼과 윈도우 위치의 차이
	ChangeWindowSize	m_WindowSizeFlag;

protected:
	BsUiWindow*			m_pLinkWnd;			// m_pLinkWnd의 상태에 따라 자신의 BsUiFocusState가 영향을 받는다.
	
	BsUiWindow*			m_pChildWndFocus;	// 자식중에 최종적으로 Focus를 받은 window

public:
	BsUiWindow();
	virtual ~BsUiWindow() { Release(); }

	bool			Create(BsUiTYPE type, BsUiCLASS hClass, int x, int y, int cx, int cy, DWORD attr = XWATTR_SHOWWINDOW, BsUiWindow* pParent = NULL);
	void			Release();

	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);

	void			PostMessage(DWORD message, WPARAM wParam = 0, LPARAM lParam = 0);
	DWORD			SendMessage(DWORD message, WPARAM wParam = 0, LPARAM lParam = 0);

	virtual void	Update();
	virtual void	Draw();

	// child window (주로 컨트롤)을 추가/삭제한다.
	void			AddChildWindow( BsUiWindow* pWindow );
	void			RemoveChildWindow( BsUiWindow* pWindow );
	void			SetParentWindow(BsUiWindow* pWindow)	{ m_pParentWnd = pWindow; }
	BsUiWindow*		GetParentWindow()						{ return m_pParentWnd; }
	BsUiWindow*		GetDlgItem(BsUiCLASS hClass);

	// return window handle
	BsUiHANDLE		GetHWnd()	{ return m_hWnd; }
	BsUiCLASS		GetClass()	{ return m_hClass; }
	BsUiTYPE		GetType()	{ return m_type; }

	// 윈도우의 위치를 변경한다.
	void			SetWindowPos(POINT ptPos) { MoveWindow(ptPos.x, ptPos.y, m_size.x, m_size.y); }
	void			SetWindowPos(int x, int y) { MoveWindow(x, y, m_size.x, m_size.y); }
	void			SetWindowPosX(int x) { MoveWindow(x, m_pos.y, m_size.x, m_size.y); }
	void			SetWindowPosY(int y) { MoveWindow(m_pos.x, y, m_size.x, m_size.y); }

	const POINT&	GetWindowPos() { return m_pos; }
	void			GetWindowPos(int& x, int& y) { x = m_pos.x; y = m_pos.y; }

	void			SetWindowSize(POINT size) { MoveWindow(m_pos.x, m_pos.y, size.x, size.y); }
	void			SetWindowSize(int cx, int cy) { MoveWindow(m_pos.x, m_pos.y, cx, cy); }
	void			SetWindowWidth(int cx)	{ MoveWindow(m_pos.x, m_pos.y, cx, m_size.y); }
	void			SetWindowHeight(int cy)	{ MoveWindow(m_pos.x, m_pos.y, m_size.x, cy); }

	const POINT&	GetWindowSize() { return m_size; }
	void			GetWindowSize(int& w, int& h) { w = m_size.x; h = m_size.y; }
	void			GetWindowRect(RECT* pRect) { pRect->left = m_pos.x; pRect->right = m_pos.x + m_size.x; pRect->top = m_pos.y; pRect->bottom = m_pos.y + m_size.y; }

	void			ClientToScreen(POINT& pt);
	void			ScreenToClient(POINT& pt);

	void			SetWindowAttr( DWORD attr )		{ m_attrCur = attr; }
	DWORD			GetWindowAttr()					{ return m_attrCur; }
	bool			IsEnableWindow()				{ return ((m_attrCur & XWATTR_DISABLE) == 0); }
	bool			IsShowWindow()					{ return ((m_attrCur & XWATTR_SHOWWINDOW) != 0); }
	bool			IsMovableWindow()				{ return ((m_attrCur & XWATTR_MOVABLE) != 0); }
	bool			IsDragAndDrop()					{ return ((m_attrCur & XWATTR_DRAG_N_DROP) != 0); }
	void			SetWindowAttr(DWORD attr, bool bValue);

	void			SetLinkWindow(BsUiWindow* pLinkWnd)	{ m_pLinkWnd = pLinkWnd; }
	BsUiWindow*		GetLinkWindow()						{ return m_pLinkWnd;}

	// 기타 함수들
	void			MoveWindow(int x, int y, int cx, int cy);
	virtual	void	ResizeWindowSize(ChangeWindowSize nFlag, POINT pos);

	BsUiWindow*		GetWindowOnPos(POINT* pPt);
	bool			IsWindow() { return (BsUi::BsUiIsValid(m_hWnd) != NULL); }

	void			DoModal();
	void			EndOfModal(int nResult, void* pRetValue);

	BsUiWINDOWLIST	GetChildWindowList() { return m_child; }
	BsUiWindow*		GetChildWindow(BsUiCLASS hClass = NULL);
	BsUiWindow*		GetEnableChildWindow(BsUiCLASS hClass = NULL);
	BsUiWindow*		GetWindow(BsUiCLASS hClass);

	bool				IsMoving()				{ return m_bMoving; }
	ChangeWindowSize	GetWindowSizeFlag()		{ return m_WindowSizeFlag; }
	BsUiFocusState		GetCurFocusState();

	BsUiHANDLE		GetNextChildHandle(BsUiHANDLE hHandle, bool bEnableFocus);
	BsUiHANDLE		GetPrevChildHandle(BsUiHANDLE hHandle, bool bEnableFocus);

	BsUiWindow*		SetFocusNextChileWindow(BsUiHANDLE hHandle, bool bEnableFocus);
	BsUiWindow*		SetFocusPrevChileWindow(BsUiHANDLE hHandle, bool bEnableFocus);

	void			SetChildWndFocus(BsUiWindow* pWindow)	{ m_pChildWndFocus= pWindow; }
	BsUiWindow*		GetChildWndFocus()						{ return m_pChildWndFocus; }
	BsUiWindow*		GetChildWndCurFocus();


//item info
protected:
	BsUiItemInfo	m_Item;
	BsUiAttrId		m_nFontAttr[BsUiFS_COUNT];
	POINT			m_ptItemPos;

public:
	void			SetItem(BsUiItemInfo item);
	BsUiItemInfo	GetItem()					{ return m_Item; }
	void			SetItemText(char* pStr);
	char*			GetItemText()				{ return m_Item.szText; }
	void			SetItemData(DWORD dwData)	{ m_Item.dwData = dwData; }
	DWORD			GetItemData()				{ return m_Item.dwData; }
	void			SetItemTextID(int nTextID);
	int				GetItemTextID()				{ return m_Item.nTextID; }

	void			SetItemPos(POINT ptPos)		{ m_ptItemPos = ptPos; }
	POINT			GetItemPos()				{ return m_ptItemPos; }

	void			SetFontAttr(BsUiFocusState nState, BsUiAttrId nAttrId)	{ m_nFontAttr[nState] = nAttrId; }
	BsUiAttrId		GetFontAttr(BsUiFocusState nState)						{ return m_nFontAttr[nState]; }
	virtual BsUiAttrId		GetCurFontAttr();

//image
protected:
	int				m_nImageMode;
	int				m_nUVID[BsUiFS_COUNT];
	int				m_nBlockID[BsUiFS_COUNT];
	D3DXCOLOR		m_color;
	//자신 또는 child가 BsUiSystem의 m_pFocus가 아닌 m_pChildWndFocus이라면
	//m_pChildWndFocus에 의해 child의 State에 영향을 줄지 아닐지 판단한다.
	bool			m_bViewChildFocus;

public:
	void			SetViewChildFocus(bool bOn)							{ m_bViewChildFocus = bOn; }
	bool			IsViewChildFocus()									{ return m_bViewChildFocus; }

	
	void			SetImageMode(int nMode)								{ m_nImageMode = nMode; }
	void			SetImageUVID(BsUiFocusState nState, int nUVID)		{ m_nUVID[nState] = nUVID; }
	void			SetImageBlock(BsUiFocusState nState, int nBlock)	{ m_nBlockID[nState] = nBlock; }

	int				GetImageMode()										{ return m_nImageMode; }
	int				GetImageUVID(BsUiFocusState nState)					{ return m_nUVID[nState]; }
	int				GetImageBlock(BsUiFocusState nState)				{ return m_nBlockID[nState]; }

	int				GetCurImageUVID();
	int				GetCurImageBlockID();

	D3DXCOLOR		GetColor()											{ return m_color; }
	void			SetColor(D3DXCOLOR clr)								{ m_color = clr; }

	void			MakeTopforChild(BsUiWindow* pWindow);

	bool			MakeTabOrder(BsUiWindow* pWindow, BsUiTabOrder nOrder);
	bool			MakeTabOrderTop(BsUiWindow* pWindow);
	bool			MakeTabOrderUp(BsUiWindow* pWindow);
	bool			MakeTabOrderBottom(BsUiWindow* pWindow);
	bool			MakeTabOrderDown(BsUiWindow* pWindow);
};