#pragma once

// BsUiSystem.h: 윈도우 관리자.

// jazzcake@hotmail.com
// 2004.2.9

#include "Singleton.h"

#include "BsUiWindow.h"

#include "BsUiLoader.h"
#include "BsUiStore.h"
#include "BsUiView.h"
#include "BsUiSkin.h"

class BsUiLoader;
class BsUiStore;
class BsUiView;
class BsUiSkin;

class BsUiLayer;
class BsUiButton;
class BsUiListBox;
class BsUiListCtrl;
class BsUiSlider;
class BSuiText;
class BSuiImageCtrl;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
typedef list <BsUiWindow* >				BsUiWINDOWLIST;
typedef BsUiWINDOWLIST::iterator		BsUiWINDOWLISTITOR;

class BsUiSystem : public CSingleton <BsUiSystem > {
private:
	typedef deque <BsUiWindow* >			BsUiWINDOWDEQUE;
	typedef BsUiWINDOWDEQUE::iterator		BsUiWINDOWDEQUEITOR;

public:

protected:
	BsUiWINDOWLIST		m_windows;						// 전체 윈도우 리스트
	BsUiWINDOWDEQUE		m_modalStack;					// modal 처리를 위한 일종의 스택

	BsUiMSGTOKENDEQUE	m_msgQue;						// 전체 메시지 큐

	BsUiSkin*			m_pSkin;
	BsUiView*			m_pView;
	BsUiLoader*			m_pLoader;
	BsUiStore*			m_pStore;
	int					m_nScreenWidth;
	int					m_nScreenHeight;

	int					m_nMenuTick;

public:
	BsUiSystem();
	virtual ~BsUiSystem() { Release(); }

	bool				Create(BsUiSkin* pSkin, BsUiView* pView, BsUiLoader* pLoader, int nScreenWidth, int nScreenHeight);
	bool				Create(BsUiSkin* pSkin, BsUiView* pView, BsUiLoader* pLoader, BsUiStore* pStore, int nScreenWidth, int nScreenHeight);
	void				Release();

	BsUiHANDLE			OpenLayer(BsUiCLASS hClass, int x, int y);
	BsUiHANDLE			OpenLayer(BsUiLayer* pLayer, BsUiCLASS hClass, int x, int y);
	void				CloseLayer(BsUiHANDLE hWnd, bool bSavePos);

	bool				LoadDef(const char* pFilename);

	void				Update();
	void				Draw();
	int					GetMenuTick()							{ return m_nMenuTick; }

	DWORD				DispatchMessage(xwMessageToken* pMsg);
	DWORD				TranslateMessage(DWORD message, WPARAM wParam, LPARAM lParam, POINT* pPt);

	DWORD				SendMessage(BsUiHANDLE hWnd, DWORD message, WPARAM wParam = 0, LPARAM lParam = 0);
	void				PostMessage(BsUiHANDLE hWnd, DWORD message, WPARAM wParam = 0, LPARAM lParam = 0);
	DWORD				NotifyMessage(BsUiHANDLE hNotifyWnd, BsUiHANDLE hWnd, DWORD message, WPARAM wParam = 0, LPARAM lParam = 0);

	bool				AddWindow(BsUiWindow* pWindow);
	void				RemoveWindow(BsUiWindow* pWindow);
	bool				AddChildWindow(BsUiWindow* pParentWnd, BsUiWindow* pWindow);
	bool				RemoveChildWindow(BsUiWindow* pParentWnd, BsUiWindow* pWindow);
	void				RemoveAllWindow(void);
	bool				PushModal(BsUiWindow* pWindow);
	bool				PopModal(BsUiWindow* pWindow, int nResult = 0, void* pRetValue = NULL);
	void				MakeTop(BsUiWindow* pWindow);

	BsUiWindow*			GetWindow(POINT* pPt);
	BsUiWindow*			GetWindow(BsUiCLASS hClass);
	BsUiSkin*			GetSkin()			{ return m_pSkin; }
	BsUiView*			GetView()			{ return m_pView; }
	BsUiLoader*			GetLoader()			{ return m_pLoader; }
	BsUiStore*			GetStore()			{ return m_pStore; }
	int					GetScreenWidth()	{ return m_nScreenWidth; }
	int					GetScreenHeight()	{ return m_nScreenHeight; }

public:
	static BsUiWindow*	m_pGrabbed;				// 마우스를 잡는다.
	static BsUiWindow*	m_pFocus;				// 포커싱된 윈도우 객체

public:
	BsUiWINDOWLIST		GetWindows()	{return m_windows; }

	static void			SetFocusWindow(BsUiWindow* pWindow);
	void				SetFocusWindowClass(BsUiCLASS hClass);
	static BsUiWindow*	GetFocusWindow() { return m_pFocus; }
	static void			SetGrabWindow(BsUiWindow* pWindow) { m_pGrabbed = pWindow; }
	static BsUiWindow*	GetGrabWindow() { return m_pGrabbed; }
	static BsUiWindow*	GetMsgReceiver();


protected:
	BsUiWINDOWLIST		m_SelectedWindows;

public:
	void				SetSelectedWindows(RECT SelectedBox);
	BsUiWINDOWLIST*		GetSelectedWindows()		{ return &m_SelectedWindows; }
	int					GetSelectedWindowsCount()	{ return (int)m_SelectedWindows.size(); }
	void				ClearSelectedWindows()		{ m_SelectedWindows.clear(); }
	bool				EditSelectedWindow(BsUiWindow* pWindow);
	bool				SetSelectedWindow(BsUiWindow* pWindow);
	bool				RemoveSelectedWindow(BsUiWindow* pWindow);
	bool				IsSelectedWindow(BsUiWindow* pWindow);

public:
	BsUiHANDLE			AddWindow(BsUiTYPE uiType, BsUiCLASS hClass, POINT pos, BsUiWindow* pParent);
	bool				RemoveControl(BsUiHANDLE hWnd);

	bool				IsCanMoveSelectedWindows();
	void				MoveSelectedWindows(POINT AddPos);
	void				ResizeSelectedWindows(ChangeWindowSize WindowSizeFlag, POINT AddSize);
	bool				MakeTabOrder(BsUiWindow* pWindow, BsUiTabOrder nOrder);

protected:
	bool				IsWindowInSelectedBox(RECT SelectedBox, RECT rect);

};


#define g_BsUiSystem	BsUiSystem::GetInstance()