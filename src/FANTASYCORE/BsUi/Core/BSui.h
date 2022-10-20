#pragma once
#pragma warning ( disable : 4786) 

#include <deque>
#include "BsUiException.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class BsUiWindow;

/*

	BsUiSystem			- �ý��� ��ü�� �����ϸ� �ȴ�. �Է� �޽����� �޾Ƽ�, �������ָ� ��� ó������ ����Ѵ�.
	BsUiWindow			- ��� ������ ��ü�� ��� Ŭ�����̴�.

	[�޽��� ����������]

	�޽����� �ý���(BsUiSystem)�� ���� Update()�Լ��� ���� ������ �������� ���ϰ� �ȴ�.

*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

// �޽��� Ÿ��
#define XWMSG_BEGIN				1;	// ������� ����

// ����뺸
#define XWMSG_OPENWINDOW		100							// ���� �޽���
#define XWMSG_CLOSEWINDOW		101							// ���� �޽���
#define XWMSG_SELCHANGED		102							// ���ú��� (�޺�, ����Ʈ�ڽ� ���� �Ϲ� �޴������� ���Ѵ�. setfocus���Ŀ� �´�.)
#define XWMSG_ENDOFMODAL		104
#define XWMSG_SETFOCUS			105
#define XWMSG_KILLFOCUS			106

// ���
#define XWMSG_MAKETOP			200
#define XWMSG_MAKECENTER		201
#define XWMSG_MAKECLOSE			202							// ������ �ݱ�
#define XWMSG_MAKEPAINT			203							// �⺻ȭ�����

// Notify
#define XWMSG_BN_CLICKED		300							// Button Notify
#define XWMSG_SD_CHANGED		400							// Slider Notify

// ���콺 & Ű
#define XWMSG_LBUTTONDOWN		1000
#define XWMSG_LBUTTONUP			1001
#define XWMSG_LBUTTONDBLCLK		1002
#define XWMSG_RBUTTONDOWN		1003
#define XWMSG_RBUTTONUP			1004
#define XWMSG_RBUTTONDBLCLK		1005
#define XWMSG_MOUSEMOVE			1006
#define XWMSG_MOUSEWHEEL		1007
#define XWMSG_KEYDOWN			1100						// wParam: nFlag, lParam: vk-code

// ��Ʈ�ѿ� ���
#define XWMSG_CMDSD_SETTIC		1300						// tic ���� ��ġ����
#define XWMSG_CMDSD_SETRANGE	1301						// range ����

// System (-1���� �߼۵� �޽���)
#define XWMSG_CHECKBUTTON		9000

// ��������
#define XWMSG_USER				9999

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

// window �Ӽ�
enum xwAttribute {
	XWATTR_SHOWWINDOW			= 0x0001,		// �����츦 ���̴°�?
	XWATTR_MOVABLE				= 0x0002,		// ������ �� �ִ°�?
	XWATTR_ALWAYSTOP			= 0x0004,		// �׻� top �ΰ�?
	XWATTR_ALWAYSBOTTOM			= 0x0008,		// �׻� top �ΰ�?
	XWATTR_DISABLE				= 0x0010,		// ������� �ʴ°�?
	XWATTR_USETABORDER			= 0x0020,		// tab-order ����ϴ°�?
	XWATTR_DRAG_N_DROP			= 0x0040,		// ����ϴ°�?
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

// ����� ������ �ڵ� Ÿ��
typedef DWORD				BsUiHANDLE;			// ������ �ν��Ͻ� �ڵ�
typedef DWORD				BsUiCLASS;			// �� �������� ���������� ���� �ڵ�

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

// ���� �� ��ü�� ������ ���� BsUiHANDLE�� ����Ѵ�.
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

// ���������� ����ϴ� �޽��� ��ū
struct xwMessageToken {
	BsUiHANDLE		hWnd;						// �޽����� �߻��� window handle
	DWORD			message;
	DWORD			wParam;
	DWORD			lParam;
	DWORD			time;
	POINT			pt;
};

// �޽��� ��ū Queue
typedef std::deque <xwMessageToken* >		BsUiMSGTOKENDEQUE;
typedef BsUiMSGTOKENDEQUE::iterator		BsUiMSGTOKENDEQUEITOR;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

// �޽��� �� ���ޱ���ü
typedef struct {
	BsUiWindow*		pWindow;
	DWORD			nResult;
	void*			pRetValue;
} xwMsg_EndOfModal;								// modal�� ���� ���� ����

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

// �巡�� & ����� ��ü
typedef struct _BsUiDragDropInfo {
	bool			bDragging;					// drag & drop ���ΰ�?

	BsUiHANDLE		hSourceHwnd;
	BsUiCLASS		hSourceClass;
	BsUiHANDLE		hTargetHwnd;
	BsUiCLASS		hTargetClass;

	xwMessageToken	msgButtonDown;
	xwMessageToken	msgButtonUp;

	int				nTypeObject;				// �ű�� ������Ʈ�� ����
	DWORD			dwData;						// ����� ���� ������
} BsUiDragDropInfo;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

// �⺻ �Լ��� 
namespace BsUi {
	extern bool				BsUiStartUp();
	extern void				BsUiShutDown();
	extern BsUiHANDLE		BsUiMakeHandle(BsUiWindow* pThis, BsUiCLASS nClass, BsUiHANDLE hParentWnd);
	extern void				BsUiDestroyHandle(BsUiHANDLE hHandle);
	extern BsUiWindow*		BsUiGetWindow(BsUiHANDLE hHandle);
	extern BsUiCLASS		BsUiGetClass(BsUiHANDLE hHandle);
	extern bool				BsUiIsValid(BsUiHANDLE hHandle);
};

