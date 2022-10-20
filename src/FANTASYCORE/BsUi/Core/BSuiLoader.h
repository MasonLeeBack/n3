#ifndef _BsUi_LOADER_H_
#define _BsUi_LOADER_H_

// BsUiLoader.h: ���� ������ �о UI�� �����ϵ��� �Ѵ�.

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

// ui�� ���¿� ���� ���ҽ� ������ ��� ��ü���̴�. �� ui �ν��Ͻ� ������ ���⼭ �������� �ʴ´�.

// child window�� ���� attach�ϱ� ���� ����
typedef struct _BsUiAttachedItem {
	BsUiCLASS				hClass;									// attach�� ������ Ŭ����
	POINT					pos;									// ��ġ
} BsUiAttachedItem;

typedef vector <BsUiAttachedItem >				BsUiATTACHEDITEMVECT;

// ���� ���� ����
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

// ��ɿ� ���� ����
typedef struct _BsUiCommandItem {
	BsUiCMD					cmd;	
	int						det;									// ��������, -1�̸� ������
	void*					pData;									// �Ʒ� BsUiCmdItem ���� ����
} BsUiCommandItem;

typedef vector <BsUiCommandItem >				BsUiCMDITEMVECT;
typedef BsUiCMDITEMVECT::iterator				BsUiCMDITEMVECTITOR;

// command sub ���� - layer�� ����� ���
typedef struct _BsUiCmdMakeLayer {
	BsUiCLASS				hClass;
	POINT					pos;
} BsUiCmdMakeLayer;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

// BsUiCLASS�� ���� Class instance ���̴�.
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
	BsUiTYPE				nType;									// � Ÿ��? layer? selcombo? listbox?
	SIZE					size;									// ui�� ũ��
	int						nAttr;									// window�� �Ӽ�

	int						nTextId;
	POINT					ptTextPos;
	BsUiAttrId				nFontAttr[3];

	int						nImageMode;
	int						nUVID[3];
	int						nBlockID[3];
	
	BsUiATTACHEDITEMVECT	attached;								// �ش� layer�� ���� control��
} BsUiClassItem;

typedef std::vector <BsUiClassItem* >			BsUiCLASSITEMVECT;
	
typedef std::map <BsUiCLASS, BsUiClassItem* >		BsUiCLASSITEMMAP;
typedef BsUiCLASSITEMMAP::iterator				BsUiCLASSITEMMAPITOR;
typedef BsUiCLASSITEMMAP::value_type				BsUiCLASSITEMMAPVALUE;

// layer ����
typedef struct _BsUiLayerItem : public BsUiClassItem {
	_BsUiLayerItem()
	{
		pos.x = 0; pos.y = 0;
	};

	POINT					pos;
	BsUiCLASSITEMVECT		controlDef;								// �ش� layer���� ���� control�� ��������

} BsUiLayerItem;

// button ����
typedef struct BsUiButtonItem : public BsUiClassItem {
	BsUiButtonItem()
	{
		nGroupID = -1;
		nCheck = -1;
	};
	short					nGroupID;								// grouping id, -1�̸� ������� ����. üũ��ư ��� ����Ѵ�.
	short					nCheck;
} BsUiButtonItem;

// listbox ����
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
	short					nLine;									// �� ���� ��
	short					nDrawSelBox;							// Select item Box
	int						nSelBoxUVID[3];
	int						nSelBoxBlockID[3];
} BsUiListBoxItem;


// listbox ����
typedef struct _BsUiListCtrlItem : public BsUiListBoxItem {
	_BsUiListCtrlItem()
	{
		nLbType = 0;
		nCols = 1;
	};
	
	short					nLbType;								// 0: �Ϲ� ����Ʈ�ڽ�, 1: �߾Ӽ��ý�ũ�� ���, 2: ������ ���
	short					nCols;									// �� �÷� ��
} BsUiListCtrlItem;

// slider ����
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
	short					nSdOrientation; // 0: ����, 1: ����
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
	BsUiCLASSITEMMAP			m_classes;								// ��� layer�� �� ���� control�� class ������ �� �ִ�.
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
