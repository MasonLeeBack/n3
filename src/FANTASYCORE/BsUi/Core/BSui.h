#pragma once
#pragma warning ( disable : 4786) 

#include <deque>
#include "BsUiException.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class BsUiWindow;

/*

	BsUiSystem			- 시스템 객체로 생각하면 된다. 입력 메시지를 받아서, 전달해주며 모달 처리등을 담당한다.
	BsUiWindow			- 모든 윈도우 객체의 기반 클래스이다.

	[메시지 파이프라인]

	메시지가 시스템(BsUiSystem)에 오면 Update()함수를 통해 가야할 목적지로 향하게 된다.

*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

// 메시지 타입
#define XWMSG_BEGIN				1;	// 사용하지 않음

// 결과통보
#define XWMSG_OPENWINDOW		100							// 통지 메시지
#define XWMSG_CLOSEWINDOW		101							// 통지 메시지
#define XWMSG_SELCHANGED		102							// 선택변경 (콤보, 리스트박스 외의 일반 메뉴에서도 통한다. setfocus이후에 온다.)
#define XWMSG_ENDOFMODAL		104
#define XWMSG_SETFOCUS			105
#define XWMSG_KILLFOCUS			106

// 명령
#define XWMSG_MAKETOP			200
#define XWMSG_MAKECENTER		201
#define XWMSG_MAKECLOSE			202							// 강제로 닫기
#define XWMSG_MAKEPAINT			203							// 기본화면출력

// Notify
#define XWMSG_BN_CLICKED		300							// Button Notify
#define XWMSG_SD_CHANGED		400							// Slider Notify

// 마우스 & 키
#define XWMSG_LBUTTONDOWN		1000
#define XWMSG_LBUTTONUP			1001
#define XWMSG_LBUTTONDBLCLK		1002
#define XWMSG_RBUTTONDOWN		1003
#define XWMSG_RBUTTONUP			1004
#define XWMSG_RBUTTONDBLCLK		1005
#define XWMSG_MOUSEMOVE			1006
#define XWMSG_MOUSEWHEEL		1007
#define XWMSG_KEYDOWN			1100						// wParam: nFlag, lParam: vk-code

// 컨트롤에 명령
#define XWMSG_CMDSD_SETTIC		1300						// tic 현재 위치설정
#define XWMSG_CMDSD_SETRANGE	1301						// range 설정

// System (-1에게 발송된 메시지)
#define XWMSG_CHECKBUTTON		9000

// 유저정의
#define XWMSG_USER				9999

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

// window 속성
enum xwAttribute {
	XWATTR_SHOWWINDOW			= 0x0001,		// 윈도우를 보이는가?
	XWATTR_MOVABLE				= 0x0002,		// 움직일 수 있는가?
	XWATTR_ALWAYSTOP			= 0x0004,		// 항상 top 인가?
	XWATTR_ALWAYSBOTTOM			= 0x0008,		// 항상 top 인가?
	XWATTR_DISABLE				= 0x0010,		// 사용하지 않는가?
	XWATTR_USETABORDER			= 0x0020,		// tab-order 사용하는가?
	XWATTR_DRAG_N_DROP			= 0x0040,		// 사용하는가?
};

enum BsUiFocusState {
	BsUiFS_SELECTED,
	BsUiFS_DEFAULTED,
	BsUiFS_DISABLE,

	BsUiFS_COUNT,
};

enum BsUiTabOrder {
	BsUiTO_TOP,
	BsUiTO_UP,
	BsUiTO_DOWN,
	BsUiTO_BOTTOM,
};

enum BsUiTYPE
{
	BsUiTYPE_NONE						= -1,
	BsUiTYPE_LAYER						= 0,
	BsUiTYPE_BUTTON,
	BsUiTYPE_LISTBOX,
	BsUiTYPE_LISTCTRL,
	BsUiTYPE_TEXT,
	BsUiTYPE_IMAGECTRL,
	BsUiTYPE_SLIDER,

	BsUiTYPE_CHECKBUTTON,

	BsUiTYPE_END,
};


enum ChangeWindowSize
{
	_NOT_CHANGE_SIZE = -1,
	_CHANGE_SIZE_LEFT,
	_CHANGE_SIZE_RIGHT,
	_CHANGE_SIZE_TOP,
	_CHANGE_SIZE_BOTTOM,

	_CHANGE_SIZE_LEFTTOP,
	_CHANGE_SIZE_LEFTBOTTOM,
	_CHANGE_SIZE_RIGHTTOP,
	_CHANGE_SIZE_RIGHTBOTTOM,

	_END_CHANGE_SIZE,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

// 사용할 데이터 핸들 타입
typedef DWORD				BsUiHANDLE;			// 윈도우 인스턴스 핸들
typedef DWORD				BsUiCLASS;			// 각 윈도우의 외형정보에 대한 핸들

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

// 실제 이 객체의 포인터 값을 BsUiHANDLE로 사용한다.
typedef struct _BsUiWindowDesc {
	BsUiCLASS			nClass;
	BsUiHANDLE			hParentWnd;
	BsUiWindow*			pThis;
	BsUiWindow*			pParent;
} BsUiWindowDesc;

typedef vector <BsUiWindowDesc* >		BsUiWINDOWDESCVECT;
typedef BsUiWINDOWDESCVECT::iterator		BsUiWINDOWDESCVECTITOR;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

// 내부적으로 사용하는 메시지 토큰
struct xwMessageToken {
	BsUiHANDLE		hWnd;						// 메시지가 발생된 window handle
	DWORD			message;
	DWORD			wParam;
	DWORD			lParam;
	DWORD			time;
	POINT			pt;
};

// 메시지 토큰 Queue
typedef std::deque <xwMessageToken* >		BsUiMSGTOKENDEQUE;
typedef BsUiMSGTOKENDEQUE::iterator		BsUiMSGTOKENDEQUEITOR;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

// 메시지 별 전달구조체
typedef struct {
	BsUiWindow*		pWindow;
	DWORD			nResult;
	void*			pRetValue;
} xwMsg_EndOfModal;								// modal이 닫힐 때의 정보

typedef struct {
	int				nCode;
	int				nParam1;
	int				nParam2;
	int				nParam3;
} xwMsg_Controller;

typedef struct {
	int				nMinRange;
	int				nMaxRange;
	int				nCurIndex;
} xwMsg_NotifySlider;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

// 드래그 & 드랍용 객체
typedef struct _BsUiDragDropInfo {
	bool			bDragging;					// drag & drop 중인가?

	BsUiHANDLE		hSourceHwnd;
	BsUiCLASS		hSourceClass;
	BsUiHANDLE		hTargetHwnd;
	BsUiCLASS		hTargetClass;

	xwMessageToken	msgButtonDown;
	xwMessageToken	msgButtonUp;

	int				nTypeObject;				// 옮기는 오브젝트의 종류
	DWORD			dwData;						// 사용자 정의 데이터
} BsUiDragDropInfo;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

// 기본 함수들 
namespace BsUi {
	extern bool				BsUiStartUp();
	extern void				BsUiShutDown();
	extern BsUiHANDLE		BsUiMakeHandle(BsUiWindow* pThis, BsUiCLASS nClass, BsUiHANDLE hParentWnd);
	extern void				BsUiDestroyHandle(BsUiHANDLE hHandle);
	extern BsUiWindow*		BsUiGetWindow(BsUiHANDLE hHandle);
	extern BsUiCLASS		BsUiGetClass(BsUiHANDLE hHandle);
	extern bool				BsUiIsValid(BsUiHANDLE hHandle);
};

