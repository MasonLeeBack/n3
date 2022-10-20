#ifndef _BsUi_LOADER_H_
#define _BsUi_LOADER_H_

// BsUiLoader.h: 설정 파일을 읽어서 UI를 구성하도록 한다.

// jazzcake@hotmail.com
// 2004.4.7

#include <vector>
#include <map>

#include "Singleton.h"
#include "BsUi.h"
#include "BsUiDefine.h"
#include "BsUiFontAttrManager.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class tpGrammer;

void AssertDebug(tpGrammer* pGrammer, const char* pStr);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

// ui의 형태에 대한 리소스 정보를 담는 객체들이다. 각 ui 인스턴스 정보는 여기서 관리되지 않는다.

// child window를 위해 attach하기 위한 정보
typedef struct _BsUiAttachedItem {
	BsUiCLASS				hClass;									// attach할 윈도우 클래스
	POINT					pos;									// 위치
} BsUiAttachedItem;

typedef vector <BsUiAttachedItem >				BsUiATTACHEDITEMVECT;

// 값에 대한 정보
typedef struct _BsUiValueItem {
	_BsUiValueItem(){
		nID = -1;
		dwData = -1;
	};

	int						nID;
	DWORD					dwData;
} BsUiValueItem;

typedef vector <BsUiValueItem >					BsUiVALUEITEMVECT;
typedef BsUiVALUEITEMVECT::iterator				BsUiVALUEITEMVECTITOR;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

// 명령에 대한 정보
typedef struct _BsUiCommandItem {
	BsUiCMD					cmd;	
	int						det;									// 범위한정, -1이면 전영역
	void*					pData;									// 아래 BsUiCmdItem 형의 정보
} BsUiCommandItem;

typedef vector <BsUiCommandItem >				BsUiCMDITEMVECT;
typedef BsUiCMDITEMVECT::iterator				BsUiCMDITEMVECTITOR;

// command sub 구조 - layer를 만드는 명령
typedef struct _BsUiCmdMakeLayer {
	BsUiCLASS				hClass;
	POINT					pos;
} BsUiCmdMakeLayer;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

// BsUiCLASS에 대한 Class instance 들이다.
typedef struct _BsUiClassItem {
	_BsUiClassItem()
	{
		hClass = 0;
		nType = BsUiTYPE_NONE;
		size.cx = 0;
		size.cy = 0;
		nAttr = 0;

		nTextId = -1;
		ptTextPos.x = 0;
		ptTextPos.y = 0;

		nImageMode = _IMAGE_MODE_DEFAULT;


		for(int i=0; i<BsUiFS_COUNT; i++)
		{	
			nUVID[i] = -1;
			nBlockID[i] = -1;
			nFontAttr[i] = -1;
		}

	};

	BsUiCLASS				hClass;									// class id
	BsUiTYPE				nType;									// 어떤 타입? layer? selcombo? listbox?
	SIZE					size;									// ui의 크기
	int						nAttr;									// window의 속성

	int						nTextId;
	POINT					ptTextPos;
	BsUiAttrId				nFontAttr[3];

	int						nImageMode;
	int						nUVID[3];
	int						nBlockID[3];
	
	BsUiATTACHEDITEMVECT	attached;								// 해당 layer에 붙은 control들
} BsUiClassItem;

typedef std::vector <BsUiClassItem* >			BsUiCLASSITEMVECT;
	
typedef std::map <BsUiCLASS, BsUiClassItem* >		BsUiCLASSITEMMAP;
typedef BsUiCLASSITEMMAP::iterator				BsUiCLASSITEMMAPITOR;
typedef BsUiCLASSITEMMAP::value_type				BsUiCLASSITEMMAPVALUE;

// layer 정보
typedef struct _BsUiLayerItem : public BsUiClassItem {
	_BsUiLayerItem()
	{
		pos.x = 0; pos.y = 0;
	};

	POINT					pos;
	BsUiCLASSITEMVECT		controlDef;								// 해당 layer에서 사용될 control의 설정값들

} BsUiLayerItem;

// button 정보
typedef struct BsUiButtonItem : public BsUiClassItem {
	BsUiButtonItem()
	{
		nGroupID = -1;
		nCheck = -1;
	};
	short					nGroupID;								// grouping id, -1이면 사용하지 않음. 체크버튼 등에서 사용한다.
	short					nCheck;
} BsUiButtonItem;

// listbox 정보
typedef struct _BsUiListBoxItem : public BsUiClassItem {
	_BsUiListBoxItem()
	{	
		nLine = 1;
		nDrawSelBox = 1;

		for(int i=0; i<BsUiFS_COUNT; i++)
		{
			nSelBoxUVID[i] = -1;
			nSelBoxBlockID[i] = -1;
		}
	};

	BsUiVALUEITEMVECT		values;
	short					nLine;									// 총 라인 수
	short					nDrawSelBox;							// Select item Box
	int						nSelBoxUVID[3];
	int						nSelBoxBlockID[3];
} BsUiListBoxItem;


// listbox 정보
typedef struct _BsUiListCtrlItem : public BsUiListBoxItem {
	_BsUiListCtrlItem()
	{
		nLbType = 0;
		nCols = 1;
	};
	
	short					nLbType;								// 0: 일반 리스트박스, 1: 중앙선택스크롤 방식, 2: 아이콘 방식
	short					nCols;									// 총 컬럼 수
} BsUiListCtrlItem;

// slider 정보
typedef struct _BsUiSliderItem : public BsUiClassItem {
	_BsUiSliderItem()
	{
		nSdType = 0;
		nSdOrientation = 0;
		ptBarSize.x = 10;
		ptBarSize.y = 30;
		nMinRange = 0;
		nMaxRange = 10;

		for(int i=0; i<BsUiFS_COUNT; i++)
		{
			nBarUVID[i] = -1;
			nBar1UVID[i] = -1;
			nBar2UVID[i] = -1;
		}
		nLineUV = -1;
		nBaseUV = -1;
	};

	short					nSdType;
	short					nSdOrientation; // 0: 수평, 1: 수직
	POINT					ptBarSize;
	short					nMinRange;
	short					nMaxRange;
	short					nBarUVID[3];
	short					nLineUV;
	short					nBaseUV;
	short					nBar1UVID[3];
	short					nBar2UVID[3];
} BsUiSliderItem;

typedef struct _BsUiTextItem : public BsUiClassItem {
} BsUiTextItem;

typedef struct _BsUiImageCtrlItem : public BsUiClassItem {
	_BsUiImageCtrlItem()
	{	
		nRealSize = 0;
		nLocal = 0;
		u1 = v1 = u2 = v2 = 0;
		nRealSize = 0;
		nReverse = 0;
		nRot = 0;
	};

	char	szFileName[256];
	short	nLocal;
	int		u1, v1, u2, v2;
	short	nRealSize;
	short	nReverse;
	int		nRot;
} BsUiImageCtrlItem;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class BsUiWindow;
class BsUiLayer;
class BsUiButton;
class BsUiListBox;
class BsUiListCtrl;
class BsUiSlider;
class BsUiText;
class BsUiImageCtrl;

class BsUiLoader
{
#ifndef _BSUI_TOOL
protected:
	BsUiCLASSITEMMAP			m_classes;								// 모든 layer와 그 안의 control의 class 정보가 들어가 있다.
#else
public:
	BsUiCLASSITEMMAP			m_classes;
#endif

public:
	BsUiLoader();
	virtual	~BsUiLoader();


	virtual bool			Create(const char* pFilename);
	virtual void			Release(){ReleaseClasses();};

	virtual BsUiLayer*			OpenLayer(BsUiCLASS hClass, int x, int y);
	virtual void				OpenLayer(BsUiLayer* pLayer, BsUiCLASS hClass, int x, int y);
	virtual BsUiClassItem*		GetClassInstance(BsUiCLASS hClass);

protected:
	void				InitGrammer();
	void				FreeClassInstance(BsUiClassItem* pItem);
	void				ReleaseClasses();

protected:
	void				OpenChild(BsUiLayer* pLayer, BsUiClassItem* pClass);
	BsUiLayer*			MakeLayer(BsUiCLASS hClass, int x, int y, BsUiWindow* pParent);
	BsUiButton*			MakeButton(BsUiCLASS hClass, int x, int y, BsUiWindow* pParent);
	BsUiListBox*		MakeListBox(BsUiCLASS hClass, int x, int y, BsUiWindow* pParent);
	BsUiListCtrl*		MakeListCtrl(BsUiCLASS hClass, int x, int y, BsUiWindow* pParent);
	BsUiSlider*			MakeSlider(BsUiCLASS hClass, int x, int y, BsUiWindow* pParent);
	BsUiText*			MakeText(BsUiCLASS hClass, int x, int y, BsUiWindow* pParent);
	BsUiImageCtrl*		MakeImageCtrl(BsUiCLASS hClass, int x, int y, BsUiWindow* pParent);

protected:
	bool				LoadClassInfoDef(tpGrammer* pGrammer, BsUiCMD cmd);
	BsUiLayerItem*		LoadLayerDef(tpGrammer* pGrammer, const char* pName);
	BsUiButtonItem*		LoadButtonDef(tpGrammer* pGrammer, const char* pName);
	BsUiListBoxItem*	LoadListBoxDef(tpGrammer* pGrammer, const char* pName);
	BsUiListCtrlItem*	LoadListCtrlDef(tpGrammer* pGrammer, const char* pName);
	BsUiSliderItem*		LoadSliderDef(tpGrammer* pGrammer, const char* pName);
	BsUiTextItem*		LoadTextDef(tpGrammer* pGrammer, const char* pName);
	BsUiImageCtrlItem*	LoadImageCtrlDef(tpGrammer* pGrammer, const char* pName);

	void				LoadValueItem(tpGrammer* pGrammer, BsUiVALUEITEMVECT& item);
	void				LoadAttachItem(tpGrammer* pGrammer, BsUiATTACHEDITEMVECT& vect);

protected:
	tpGrammer			*m_pGrammer;
};

#endif
