#include "stdafx.h"

#include "BsUtil.h"
#include "tpGrammer.h"
#include "BsKernel.h"

#include "BsUiLayer.h"
#include "BsUiButton.h"
#include "BsUiListBox.h"
#include "BsUiListCtrl.h"
#include "BsUiSlider.h"
#include "BSuiText.h"
#include "BSuiImageCtrl.h"

#include "BsUiSystem.h"

#include "TextTable.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void AssertDebug(tpGrammer* pGrammer, const char* pStr)
{
	DebugString("Error> %d lines - %s\n", pGrammer->GetCurLine(), pStr);

	BsAssert(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Z

BsUiLoader::BsUiLoader()
{
	m_pGrammer = NULL;
}

BsUiLoader::~BsUiLoader()
{
	Release();
	if(m_pGrammer)
	{
		delete m_pGrammer;
		m_pGrammer = NULL;
	}
}

void BsUiLoader::InitGrammer()
{
	m_pGrammer = new tpGrammer();
	// 문법등록
	
	// 문자열 집합이 동일한 작은 셋을 포함하는 경우에는 큰걸 위로 올린다.
	// ex) "addball = %d, %d"와 "addball = %d, %d, %d"의 경우 "addball = %d, %d, %d" 위로 올려야 함.
	m_pGrammer->Add(BsUiCMD_END_BRACE,			"}");
	m_pGrammer->Add(BsUiCMD_VERSION,			"UI_VERSION(%s)");

	m_pGrammer->Add(BsUiCMD_BEGIN_LAYER,		"LAYER(%s) {");
	m_pGrammer->Add(BsUiCMD_BEGIN_BUTTON,		"BUTTON(%s) {");
	m_pGrammer->Add(BsUiCMD_BEGIN_CHECKBUTTON,	"CHECKBUTTON(%s) {");
	m_pGrammer->Add(BsUiCMD_BEGIN_LISTBOX,		"LISTBOX(%s) {");
	m_pGrammer->Add(BsUiCMD_BEGIN_LISTCTRL,		"LISTCTRL(%s) {");
	m_pGrammer->Add(BsUiCMD_BEGIN_SLIDER,		"SLIDER(%s) {");
	m_pGrammer->Add(BsUiCMD_BEGIN_TEXT,			"TEXT(%s) {");
	m_pGrammer->Add(BsUiCMD_BEGIN_IMAGE,		"IMAGE(%s) {");

	m_pGrammer->Add(BsUiCMD_BEGIN_ATTACH,		"ATTACH {");
	m_pGrammer->Add(BsUiCMD_BEGIN_VALUE,		"VALUE {");

	m_pGrammer->Add(BsUiCMD_SETCLASSID,			"CLASSID=%d");
	m_pGrammer->Add(BsUiCMD_SETSIZE,			"SIZE=%d,%d");
	m_pGrammer->Add(BsUiCMD_ATTRIBUTE,			"ATTRIBUTE=%d");

	m_pGrammer->Add(BsUiCMD_ADDITEM,			"ADDITEM=CLASSID(%d),POS(%d,%d)");

	//edit text
	m_pGrammer->Add(BsUiCMD_TEXTID,				"TEXTID=%d");
	m_pGrammer->Add(BsUiCMD_TEXT_POS,			"TEXTPOS=%d,%d");
	m_pGrammer->Add(BsUiCMD_FONT_ATTR,			"FONTATTRID=%s,%s,%s");

	//edit image
	m_pGrammer->Add(BsUiCMD_SETIMAGE,			"IMAGE_MODE=%d");
	m_pGrammer->Add(BsUiCMD_SET_UV,				"UV_MODE:%d,%d,%d");
	m_pGrammer->Add(BsUiCMD_SET_UV_OLD,			"UV_MODE:SelectID=%d,DefaultId=%d,nDisableId=%d");
	m_pGrammer->Add(BsUiCMD_SET_BLOCK,			"BLOCK_MODE:%d,%d,%d");
	m_pGrammer->Add(BsUiCMD_SET_BLOCK_OLD,		"BLOCK_MODE:SelectID=%d,DefaultId=%d,nDisableId=%d");

	//type layer
	m_pGrammer->Add(BsUiCMD_SETPOS,				"POS=%d,%d");					// for Layer

	//type buttom
	m_pGrammer->Add(BsUiCMD_SETCHECK,			"CHECK=%d");					// for BsUiCMD_BEGIN_BUTTON
	m_pGrammer->Add(BsUiCMD_SETGROUP,			"GROUP=%d");					// for BsUiCMD_BEGIN_BUTTON

	//type listbox
	m_pGrammer->Add(BsUiCMD_SETLBTYPE,			"LBTYPE=%d");					// for BsUiCMD_BEGIN_LISTBOX
	m_pGrammer->Add(BsUiCMD_SETLINE,			"LINE=%d");						// for BsUiCMD_BEGIN_LISTBOX
	m_pGrammer->Add(BsUiCMD_SETCOLUMN,			"COLUMN=%d");					// for BsUiCMD_BEGIN_LISTBOX
	m_pGrammer->Add(BsUiCMD_DRAWSELBOX,			"SHOW_SELBOX=%d");				// for BsUiCMD_BEGIN_LISTBOX
	m_pGrammer->Add(BsUiCMD_SET_SELBOXUV,		"SELBOX_UV_MODE:%d,%d,%d");
	m_pGrammer->Add(BsUiCMD_SET_SELBOXUV_OLD,	"SELBOX_UV_MODE:SelectID=%d,DefaultId=%d,nDisableId=%d");
	m_pGrammer->Add(BsUiCMD_SET_SELBOXBLOCK,	"SELBOX_BLOCK_MODE:%d,%d,%d");
	m_pGrammer->Add(BsUiCMD_SET_SELBOXBLOCK_OLD,"SELBOX_BLOCK_MODE:SelectID=%d,DefaultId=%d,nDisableId=%d");
	m_pGrammer->Add(BsUiCMD_LISTITEM,			"LISTITEM=TEXTID(%d),DATA(%d)"); // for BsUiCMD_BEGIN_VALUE
	m_pGrammer->Add(BsUiCMD_SETSTRING,			"STRING=%d,%s");				// for BsUiCMD_BEGIN_VALUE

	//type slider
	m_pGrammer->Add(BsUiCMD_SETSDTYPE,			"SDTYPE=%d");					// for BsUiCMD_BEGIN_SLIDER
	m_pGrammer->Add(BsUiCMD_SETSDORIENTATION,	"SDOTYPE=%d");					// for BsUiCMD_BEGIN_SLIDER
	m_pGrammer->Add(BsUiCMD_SETRANGE,			"RANGE=%d,%d");					// for BsUiCMD_BEGIN_SLIDER
	m_pGrammer->Add(BsUiCMD_SETBARSIZE,			"BAR_SIZE=%d,%d");				// for BsUiCMD_BEGIN_SLIDER
	m_pGrammer->Add(BsUiCMD_SET_BARUV,			"BAR_UV:%d,%d,%d");
	m_pGrammer->Add(BsUiCMD_SET_BARUV_OLD,		"BAR_UV:SelectID=%d,DefaultId=%d,nDisableId=%d");
	m_pGrammer->Add(BsUiCMD_SETSDLINEUV,		"SD_LINE_UV=%d");				// for BsUiCMD_BEGIN_SLIDER
	m_pGrammer->Add(BsUiCMD_SETSDBASEUV,		"SD_BASE_UV=%d");				// for BsUiCMD_BEGIN_SLIDER
	m_pGrammer->Add(BsUiCMD_SET_BAR1UV,			"BAR1_UV:%d,%d,%d");
	m_pGrammer->Add(BsUiCMD_SET_BAR1UV_OLD,		"BAR1_UV:SelectID=%d,DefaultId=%d,nDisableId=%d");
	m_pGrammer->Add(BsUiCMD_SET_BAR2UV,			"BAR2_UV:%d,%d,%d");
	m_pGrammer->Add(BsUiCMD_SET_BAR2UV_OLD,		"BAR2_UV:SelectID=%d,DefaultId=%d,nDisableId=%d");

	//type image
	m_pGrammer->Add(BsUiCMD_FILENAME,			"FILE_NAME=%s");				// for BsUiCMD_BEGIN_IMAGE
	m_pGrammer->Add(BsUiCMD_IMG_LOCAL,			"FILE_LOCAL=%d");				// for BsUiCMD_BEGIN_IMAGE
	m_pGrammer->Add(BsUiCMD_UV,					"UV=%d,%d,%d,%d");				// for BsUiCMD_BEGIN_IMAGE
	m_pGrammer->Add(BsUiCMD_REALSIZE,			"REALSIZE=%d");					// for BsUiCMD_BEGIN_IMAGE
	m_pGrammer->Add(BsUiCMD_REVERSE,			"REVERSE=%d");					// for BsUiCMD_BEGIN_IMAGE
	m_pGrammer->Add(BsUiCMD_ROTATE,				"ROTATE=%d");					// for BsUiCMD_BEGIN_IMAGE
}

bool BsUiLoader::Create(const char* pFilename)
{
	BsAssert(pFilename);

	//ReleaseClasses();
	if(m_pGrammer == NULL)
		InitGrammer();

	m_pGrammer->Create(pFilename, ".");

	int ret = 0;

	do {
		ret = m_pGrammer->Get();

		if(LoadClassInfoDef(m_pGrammer, (BsUiCMD)ret))
			continue;

		switch (ret)
		{
		case BsUiCMD_VERSION:
			{
				//version check;
				break;
			}
		case BsUiCMD_END_BRACE:
			{
				// 모든 레이어 등록 종료
			}
			break;
		case -1:
			{
				AssertDebug(m_pGrammer, "정보이상");
			}
			break;
		}
	} while (ret != 0);

	return true;
}


void BsUiLoader::ReleaseClasses()
{
	BsUiCLASSITEMMAPITOR itr = m_classes.begin();

	while (itr != m_classes.end())
	{
		BsUiClassItem* pItem = (*itr).second;
		BsAssert(pItem);

		FreeClassInstance(pItem);
		++itr;
	}

	m_classes.clear();
}

BsUiLayer* BsUiLoader::OpenLayer(BsUiCLASS hClass, int x, int y)
{
	BsUiLayerItem* pClass = (BsUiLayerItem*)GetClassInstance(hClass);
	BsAssert(pClass);
	BsAssert(pClass->nType == BsUiTYPE_LAYER);

	// layer부터 만든다.
	if(x == -1 && y == -1)
	{
		x = pClass->pos.x;
		y = pClass->pos.y;
	}

	BsUiLayer* pLayer = MakeLayer(hClass, x, y, NULL);
	BsAssert(pLayer);

	OpenChild(pLayer, pClass);

	return pLayer;
}

void BsUiLoader::OpenLayer(BsUiLayer* pLayer, BsUiCLASS hClass, int x, int y)
{
	BsUiLayerItem* pClass = (BsUiLayerItem*)GetClassInstance(hClass);
	BsAssert(pClass);
	BsAssert(pClass->nType == BsUiTYPE_LAYER);

	if(x == -1 && y == -1)
	{
		x = pClass->pos.x;
		y = pClass->pos.y;
	}

	if (pLayer->Create(hClass, x, y, NULL) == false)
	{
		delete pLayer;
	}
	BsAssert(pLayer);

	OpenChild(pLayer, pClass);
}

void BsUiLoader::OpenChild(BsUiLayer* pLayer, BsUiClassItem* pClass)
{
	DWORD dwCount = pClass->attached.size();
	for(DWORD i=0; i<dwCount; ++i)
	{
		BsUiAttachedItem& info = pClass->attached[i];

		BsUiClassItem* pChildClass = GetClassInstance(info.hClass);
		BsAssert(pChildClass);

		switch (pChildClass->nType)
		{
		case BsUiTYPE_LAYER:
			{
				BsUiLayer* pChildLayer = MakeLayer(info.hClass, info.pos.x, info.pos.y, pLayer);
				BsAssert(pChildLayer);
			}
			break;
		case BsUiTYPE_BUTTON:
			{
				BsUiButton* pChildButton = MakeButton(info.hClass, info.pos.x, info.pos.y, pLayer);
				BsAssert(pChildButton);
			}
			break;
		case BsUiTYPE_CHECKBUTTON:
			{
				BsUiButton* pChildButton = MakeButton(info.hClass, info.pos.x, info.pos.y, pLayer);
				BsAssert(pChildButton);

				pChildButton->SetMode(BsUiBUTTON_CHECKBUTTON);
			}
			break;
		case BsUiTYPE_LISTBOX:
			{
				BsUiListBox* pChildListBox = MakeListBox(info.hClass, info.pos.x, info.pos.y, pLayer);
				BsAssert(pChildListBox);
			}
			break;
		case BsUiTYPE_LISTCTRL:
			{
				BsUiListCtrl* pChildListCtrl = MakeListCtrl(info.hClass, info.pos.x, info.pos.y, pLayer);
				BsAssert(pChildListCtrl);
			}
			break;
		case BsUiTYPE_SLIDER:
			{
				BsUiSlider* pSlider = MakeSlider(info.hClass, info.pos.x, info.pos.y, pLayer);
				BsAssert(pSlider);
			}
			break;
		case BsUiTYPE_TEXT:
			{
				BsUiText* pText = MakeText(info.hClass, info.pos.x, info.pos.y, pLayer);
				BsAssert(pText);
			}
			break;
		case BsUiTYPE_IMAGECTRL:
			{
				BsUiImageCtrl* pImage = MakeImageCtrl(info.hClass, info.pos.x, info.pos.y, pLayer);
				BsAssert(pImage);
			}
			break;
		default: BsAssert(0);
		}
	}
}

BsUiClassItem* BsUiLoader::GetClassInstance(BsUiCLASS hClass)
{
	BsUiCLASSITEMMAPITOR itr = m_classes.find(hClass);

	if (itr != m_classes.end())
		return (*itr).second;

	return NULL;
}


void BsUiLoader::FreeClassInstance(BsUiClassItem* pItem)
{
	BsAssert(pItem);
	delete pItem;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

BsUiLayer* BsUiLoader::MakeLayer(BsUiCLASS hClass, int x, int y, BsUiWindow* pParent)
{
	BsUiLayer* pLayer = new BsUiLayer;
	BsAssert(pLayer);
	
	if (pLayer->Create(hClass, x, y, pParent) == false)
	{
		delete pLayer;
		return NULL;
	}

	return pLayer;
}

BsUiButton* BsUiLoader::MakeButton(BsUiCLASS hClass, int x, int y, BsUiWindow* pParent)
{
	BsUiButton* pButton = new BsUiButton;
	BsAssert(pButton);

	if (pButton->Create(hClass, x, y, pParent) == false)
	{
		delete pButton;
		return NULL;
	}

	// value 추가
	BsUiClassItem* pControlItem = static_cast <BsUiClassItem* > (GetClassInstance(hClass));
	BsAssert(pControlItem);

	return pButton;
}

BsUiListBox* BsUiLoader::MakeListBox(BsUiCLASS hClass, int x, int y, BsUiWindow* pParent)
{
	BsUiListBox* pListBox = new BsUiListBox;
	BsAssert(pListBox);
	
	if (pListBox->Create(hClass, x, y, pParent) == false)
	{
		delete pListBox;
		return NULL;
	}

	// value 추가
	BsUiListBoxItem* pListBoxItem = static_cast <BsUiListBoxItem* > (GetClassInstance(hClass));
	BsAssert(pListBoxItem);

	return pListBox;
}

BsUiListCtrl* BsUiLoader::MakeListCtrl(BsUiCLASS hClass, int x, int y, BsUiWindow* pParent)
{
	BsUiListCtrl* pListCtrl = new BsUiListCtrl;
	BsAssert(pListCtrl);

	if (pListCtrl->Create(hClass, x, y, pParent) == false)
	{
		delete pListCtrl;
		return NULL;
	}

	// value 추가
	BsUiListCtrlItem* pListCtrlItem = static_cast <BsUiListCtrlItem* > (GetClassInstance(hClass));
	BsAssert(pListCtrlItem);

	return pListCtrl;
}

BsUiSlider* BsUiLoader::MakeSlider(BsUiCLASS hClass, int x, int y, BsUiWindow* pParent)
{
	BsUiSlider* pSlider = new BsUiSlider;
	BsAssert(pSlider);
	
	if (pSlider->Create(hClass, x, y, pParent) == false)
	{
		delete pSlider;
		return NULL;
	}

	// value 추가
	BsUiClassItem* pControlItem = static_cast <BsUiClassItem* > (GetClassInstance(hClass));
	BsAssert(pControlItem);

	return pSlider;
}

BsUiText* BsUiLoader::MakeText(BsUiCLASS hClass, int x, int y, BsUiWindow* pParent)
{
	BsUiText* pText = new BsUiText;
	BsAssert(pText);

	if (pText->Create(hClass, x, y, pParent) == false)
	{
		delete pText;
		return NULL;
	}

	// value 추가
	BsUiClassItem* pControlItem = static_cast <BsUiClassItem* > (GetClassInstance(hClass));
	BsAssert(pControlItem);

	return pText;
}

BsUiImageCtrl* BsUiLoader::MakeImageCtrl(BsUiCLASS hClass, int x, int y, BsUiWindow* pParent)
{
	BsUiImageCtrl* pImage = new BsUiImageCtrl;
	BsAssert(pImage);

	if (pImage->Create(hClass, x, y, pParent) == false)
	{
		delete pImage;
		return NULL;
	}

	return pImage;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
bool BsUiLoader::LoadClassInfoDef(tpGrammer* pGrammer, BsUiCMD cmd)
{
	BsUiClassItem* pClassItem = NULL;

	switch (cmd)
	{
	case BsUiCMD_BEGIN_LAYER:
		{
			BsUiLayerItem* pLayer = LoadLayerDef(pGrammer, pGrammer->GetParam(0));
			BsAssert(pLayer);

			if (m_classes.insert(BsUiCLASSITEMMAPVALUE(pLayer->hClass, pLayer)).second == false)
			{
				AssertDebug(pGrammer, "동일한 클래스 존재");
			}
			pClassItem = (BsUiClassItem*)pLayer;
		}
		break;
	case BsUiCMD_BEGIN_BUTTON:
		{
			BsUiButtonItem* pButton = LoadButtonDef(pGrammer, pGrammer->GetParam(0));
			BsAssert(pButton);

			if (m_classes.insert(BsUiCLASSITEMMAPVALUE(pButton->hClass, pButton)).second == false)
			{
				AssertDebug(pGrammer, "동일한 클래스 존재");
			}
			pClassItem = (BsUiClassItem*)pButton;
		}
		break;
	case BsUiCMD_BEGIN_CHECKBUTTON:
		{
			BsUiButtonItem* pButton = LoadButtonDef(pGrammer, pGrammer->GetParam(0));
			BsAssert(pButton);

			pButton->nType = BsUiTYPE_CHECKBUTTON;

			if (m_classes.insert(BsUiCLASSITEMMAPVALUE(pButton->hClass, pButton)).second == false)
			{
				AssertDebug(pGrammer, "동일한 클래스 존재");
			}
			pClassItem = (BsUiClassItem*)pButton;
		}
		break;
	case BsUiCMD_BEGIN_LISTBOX:
		{
			BsUiListBoxItem* pListBox = LoadListBoxDef(pGrammer, pGrammer->GetParam(0));
			BsAssert(pListBox);

			if (m_classes.insert(BsUiCLASSITEMMAPVALUE(pListBox->hClass, pListBox)).second == false)
			{
				AssertDebug(pGrammer, "동일한 클래스 존재");
			}
			pClassItem = (BsUiClassItem*)pListBox;
		}
		break;
	case BsUiCMD_BEGIN_LISTCTRL:
		{
			BsUiListCtrlItem* pListCtrl = LoadListCtrlDef(pGrammer, pGrammer->GetParam(0));
			BsAssert(pListCtrl);

			if (m_classes.insert(BsUiCLASSITEMMAPVALUE(pListCtrl->hClass, pListCtrl)).second == false)
			{
				AssertDebug(pGrammer, "동일한 클래스 존재");
			}
			pClassItem = (BsUiClassItem*)pListCtrl;
		}
		break;
	case BsUiCMD_BEGIN_SLIDER:
		{
			BsUiSliderItem* pSlider = LoadSliderDef(pGrammer, pGrammer->GetParam(0));
			BsAssert(pSlider);

			if (m_classes.insert(BsUiCLASSITEMMAPVALUE(pSlider->hClass, pSlider)).second == false)
			{
				AssertDebug(pGrammer, "동일한 클래스 존재");
			}
			pClassItem = (BsUiClassItem*)pSlider;
		}
		break;

	case BsUiCMD_BEGIN_TEXT:
		{
			BsUiTextItem* pText = LoadTextDef(pGrammer, pGrammer->GetParam(0));
			BsAssert(pText);

			if (m_classes.insert(BsUiCLASSITEMMAPVALUE(pText->hClass, pText)).second == false)
			{
				AssertDebug(pGrammer, "동일한 클래스 존재");
			}
			pClassItem = (BsUiClassItem*)pText;
		}
		break;
	case BsUiCMD_BEGIN_IMAGE:
		{
			BsUiImageCtrlItem* pImage = LoadImageCtrlDef(pGrammer, pGrammer->GetParam(0));
			BsAssert(pImage);

			if (m_classes.insert(BsUiCLASSITEMMAPVALUE(pImage->hClass, pImage)).second == false)
			{
				AssertDebug(pGrammer, "동일한 클래스 존재");
			}
			pClassItem = (BsUiClassItem*)pImage;
		}
		break;
	default:
		return false;
	}

	return true;
}


BsUiLayerItem* BsUiLoader::LoadLayerDef(tpGrammer* pGrammer, const char* pName)
{
	BsUiLayerItem* pLayer = new BsUiLayerItem;
	BsAssert(pLayer);

	pLayer->nType = BsUiTYPE_LAYER;
	//pLayer->strName = pName;

	int ret = 0;

	do {
		ret = pGrammer->Get();

		if(LoadClassInfoDef(pGrammer, (BsUiCMD)ret))
			continue;

		switch (ret)
		{
		case BsUiCMD_BEGIN_ATTACH:
			{
				LoadAttachItem(pGrammer, pLayer->attached);
				break;
			}
		case BsUiCMD_SETCLASSID:
			{
				pLayer->hClass= atoi(pGrammer->GetParam(0));
				break;
			}
		case BsUiCMD_ATTRIBUTE:
			{
				pLayer->nAttr = atoi(pGrammer->GetParam(0));
				break;
			}
		case BsUiCMD_SETPOS:
			{
				pLayer->pos.x = atoi(pGrammer->GetParam(0));
				pLayer->pos.y = atoi(pGrammer->GetParam(1));
				break;
			}
		case BsUiCMD_SETSIZE:
			{
				pLayer->size.cx = atoi(pGrammer->GetParam(0));
				pLayer->size.cy = atoi(pGrammer->GetParam(1));
				break;
			}
		case BsUiCMD_SETIMAGE:
			{
				pLayer->nImageMode = atoi(pGrammer->GetParam(0));
				break;
			}
		case BsUiCMD_SET_UV:
		case BsUiCMD_SET_UV_OLD:
			{
				for(int i=0; i<BsUiFS_COUNT; i++)
				{
					pLayer->nUVID[i] = atoi(pGrammer->GetParam(i));
				}
				break;
			}
		case BsUiCMD_SET_BLOCK:
		case BsUiCMD_SET_BLOCK_OLD:
			{
				for(int i=0; i<BsUiFS_COUNT; i++)
				{
					pLayer->nBlockID[i] = atoi(pGrammer->GetParam(i));
				}
				break;
			}
		case BsUiCMD_END_BRACE:
			{	
				return pLayer;
			}
		default:
			{
				AssertDebug(pGrammer, "레이어 정보이상");
				break;
			}
		}
	} while (ret != 0);

	BsAssert(false);
	return pLayer;
}

BsUiButtonItem* BsUiLoader::LoadButtonDef(tpGrammer* pGrammer, const char* pName)
{
	BsAssert(pGrammer);

	BsUiButtonItem* pButton = new BsUiButtonItem;
	BsAssert(pButton);

	pButton->nType = BsUiTYPE_BUTTON;			// default
	//pButton->strName = pName;

	int ret = 0;

	do {
		ret = pGrammer->Get();

		switch (ret)
		{
		case BsUiCMD_SETCLASSID:
			{
				pButton->hClass= atoi(pGrammer->GetParam(0));
			}
			break;
		case BsUiCMD_SETSIZE:
			{
				pButton->size.cx = atoi(pGrammer->GetParam(0));
				pButton->size.cy = atoi(pGrammer->GetParam(1));
			}
			break;
		case BsUiCMD_ATTRIBUTE:
			{
				pButton->nAttr = atoi(pGrammer->GetParam(0));
				break;
			}
		case BsUiCMD_SETCHECK:
			{
				pButton->nCheck = atoi(pGrammer->GetParam(0));
			}
			break;
		case BsUiCMD_SETGROUP:
			{
				pButton->nGroupID = atoi(pGrammer->GetParam(0));
			}
			break;
		case BsUiCMD_TEXTID:
			{
				pButton->nTextId = atoi(pGrammer->GetParam(0));
			}
			break;
		case BsUiCMD_TEXT_POS:
			{
				pButton->ptTextPos.x = atoi(pGrammer->GetParam(0));
				pButton->ptTextPos.y = atoi(pGrammer->GetParam(1));
			}
			break;
		case BsUiCMD_FONT_ATTR:
			{
				for(int i=0; i<BsUiFS_COUNT; i++)
				{
					pButton->nFontAttr[i] = g_FontAttrMgr.GetAttrId((char*)pGrammer->GetParam(i));
				}
				break;
			}
		case BsUiCMD_SETIMAGE:
			{
				pButton->nImageMode = atoi(pGrammer->GetParam(0));
				break;
			}
		case BsUiCMD_SET_UV:
		case BsUiCMD_SET_UV_OLD:
			{
				for(int i=0; i<BsUiFS_COUNT; i++)
				{
					pButton->nUVID[i] = atoi(pGrammer->GetParam(i));
				}
				break;
			}
		case BsUiCMD_SET_BLOCK:
		case BsUiCMD_SET_BLOCK_OLD:
			{
				for(int i=0; i<BsUiFS_COUNT; i++)
				{
					pButton->nBlockID[i] = atoi(pGrammer->GetParam(i));
				}
				break;
			}
		case BsUiCMD_END_BRACE:
			return pButton;
		default:
			AssertDebug(pGrammer, "Button 정보이상");
			break;
		}
	} while (ret != 0);

	BsAssert(false);
	return pButton;
}

BsUiListBoxItem* BsUiLoader::LoadListBoxDef(tpGrammer* pGrammer, const char* pName)
{
	BsAssert(pGrammer);

	BsUiListBoxItem* pListBox = new BsUiListBoxItem;
	BsAssert(pListBox);

	pListBox->nType = BsUiTYPE_LISTBOX;
	//pListBox->strName = pName;

	int ret = 0;

	do {
		ret = pGrammer->Get();

		switch (ret)
		{
		case BsUiCMD_SETCLASSID:
			{
				pListBox->hClass = atoi(pGrammer->GetParam(0));
			}
			break;
		case BsUiCMD_SETSIZE:
			{
				pListBox->size.cx = atoi(pGrammer->GetParam(0));
				pListBox->size.cy = atoi(pGrammer->GetParam(1));
			}
			break;
		case BsUiCMD_ATTRIBUTE:
			{
				pListBox->nAttr = atoi(pGrammer->GetParam(0));
				break;
			}
		case BsUiCMD_SETLINE:
			{
				pListBox->nLine = atoi(pGrammer->GetParam(0));
			}
			break;
		case BsUiCMD_SETIMAGE:
			{
				pListBox->nImageMode = atoi(pGrammer->GetParam(0));
				break;
			}
		case BsUiCMD_SET_UV:
		case BsUiCMD_SET_UV_OLD:
			{
				for(int i=0; i<BsUiFS_COUNT; i++)
				{
					pListBox->nUVID[i] = atoi(pGrammer->GetParam(i));
				}
				break;
			}
		case BsUiCMD_SET_BLOCK:
		case BsUiCMD_SET_BLOCK_OLD:
			{
				for(int i=0; i<BsUiFS_COUNT; i++)
				{
					pListBox->nBlockID[i] = atoi(pGrammer->GetParam(i));
				}
				break;
			}
		case BsUiCMD_DRAWSELBOX:
			{
				pListBox->nDrawSelBox = atoi(pGrammer->GetParam(0));
				break;
			}
		case BsUiCMD_SET_SELBOXUV:
		case BsUiCMD_SET_SELBOXUV_OLD:
			{
				for(int i=0; i<BsUiFS_COUNT; i++)
				{
					pListBox->nSelBoxUVID[i] = atoi(pGrammer->GetParam(i));
				}
				break;
			}
		case BsUiCMD_SET_SELBOXBLOCK:
		case BsUiCMD_SET_SELBOXBLOCK_OLD:
			{
				for(int i=0; i<BsUiFS_COUNT; i++)
				{
					pListBox->nSelBoxBlockID[i] = atoi(pGrammer->GetParam(i));
				}
				break;
			}
		case BsUiCMD_TEXT_POS:
			{
				pListBox->ptTextPos.x = atoi(pGrammer->GetParam(0));
				pListBox->ptTextPos.y = atoi(pGrammer->GetParam(1));
			}
			break;
		case BsUiCMD_FONT_ATTR:
			{
				for(int i=0; i<BsUiFS_COUNT; i++)
				{
					pListBox->nFontAttr[i] = g_FontAttrMgr.GetAttrId((char*)pGrammer->GetParam(i));
				}
				break;
			}
		case BsUiCMD_BEGIN_VALUE:
			{
				LoadValueItem(pGrammer, pListBox->values);
			}
			break;
		case BsUiCMD_END_BRACE:
			return pListBox;
		default:
			{
				switch(ret)
				{
				case BsUiCMD_SETLBTYPE:
				case BsUiCMD_SETCOLUMN: break;
				default: AssertDebug(pGrammer, "ListBox 정보이상");
				}
				break;
			}
		}
	} while (ret != 0);

	BsAssert(false);
	return pListBox;
}

BsUiListCtrlItem* BsUiLoader::LoadListCtrlDef(tpGrammer* pGrammer, const char* pName)
{
	BsAssert(pGrammer);

	BsUiListCtrlItem* pListCtrl = new BsUiListCtrlItem;
	BsAssert(pListCtrl);

	pListCtrl->nType = BsUiTYPE_LISTCTRL;
	//pListCtrl->strName = pName;

	int ret = 0;

	do {
		ret = pGrammer->Get();

		switch (ret)
		{
		case BsUiCMD_SETCLASSID:
			{
				pListCtrl->hClass = atoi(pGrammer->GetParam(0));
			}
			break;
		case BsUiCMD_SETSIZE:
			{
				pListCtrl->size.cx = atoi(pGrammer->GetParam(0));
				pListCtrl->size.cy = atoi(pGrammer->GetParam(1));
			}
			break;
		case BsUiCMD_ATTRIBUTE:
			{
				pListCtrl->nAttr = atoi(pGrammer->GetParam(0));
				break;
			}
		case BsUiCMD_SETLBTYPE:
			{
				pListCtrl->nLbType = atoi(pGrammer->GetParam(0));

				BsAssert(pListCtrl->nLbType == BsUiLBTYPE_NORMAL ||
					pListCtrl->nLbType == BsUiLBTYPE_FIXSELPOS ||
					pListCtrl->nLbType == BsUiLBTYPE_ICON);
			}
			break;
		case BsUiCMD_SETLINE:
			{
				pListCtrl->nLine = atoi(pGrammer->GetParam(0));
			}
			break;
		case BsUiCMD_SETCOLUMN:
			{
				pListCtrl->nCols = atoi(pGrammer->GetParam(0));
			}
			break;
		case BsUiCMD_SETIMAGE:
			{
				pListCtrl->nImageMode = atoi(pGrammer->GetParam(0));
				break;
			}
		case BsUiCMD_SET_UV:
		case BsUiCMD_SET_UV_OLD:
			{
				for(int i=0; i<BsUiFS_COUNT; i++)
				{
					pListCtrl->nUVID[i] = atoi(pGrammer->GetParam(i));
				}
				break;
			}
		case BsUiCMD_SET_BLOCK:
		case BsUiCMD_SET_BLOCK_OLD:
			{
				for(int i=0; i<BsUiFS_COUNT; i++)
				{
					pListCtrl->nBlockID[i] = atoi(pGrammer->GetParam(i));
				}
				break;
			}
		case BsUiCMD_DRAWSELBOX:
			{
				pListCtrl->nDrawSelBox = atoi(pGrammer->GetParam(0));
				break;
			}
		case BsUiCMD_TEXT_POS:
			{
				pListCtrl->ptTextPos.x = atoi(pGrammer->GetParam(0));
				pListCtrl->ptTextPos.y = atoi(pGrammer->GetParam(1));
			}
			break;
		case BsUiCMD_FONT_ATTR:
			{
				for(int i=0; i<BsUiFS_COUNT; i++)
				{
					pListCtrl->nFontAttr[i] = g_FontAttrMgr.GetAttrId((char*)pGrammer->GetParam(i));
				}
				break;
			}
		case BsUiCMD_BEGIN_VALUE:
			{
				LoadValueItem(pGrammer, pListCtrl->values);
			}
			break;
		case BsUiCMD_END_BRACE:
			return pListCtrl;
		default:
			AssertDebug(pGrammer, "ListCtrl 정보이상");
			break;
		}
	} while (ret != 0);

	BsAssert(false);
	return pListCtrl;
}

BsUiSliderItem* BsUiLoader::LoadSliderDef(tpGrammer* pGrammer, const char* pName)
{
	BsAssert(pGrammer);

	BsUiSliderItem* pSlider = new BsUiSliderItem;
	BsAssert(pSlider);

	pSlider->nType = BsUiTYPE_SLIDER;

	int ret = 0;

	do {
		ret = pGrammer->Get();

		switch (ret)
		{
		case BsUiCMD_SETCLASSID:
			{
				pSlider->hClass = atoi(pGrammer->GetParam(0));
			}
			break;
		case BsUiCMD_SETSIZE:
			{
				pSlider->size.cx = atoi(pGrammer->GetParam(0));
				pSlider->size.cy = atoi(pGrammer->GetParam(1));
			}
			break;
		case BsUiCMD_ATTRIBUTE:
			{
				pSlider->nAttr = atoi(pGrammer->GetParam(0));
				break;
			}
		case BsUiCMD_SETSDTYPE:
			{
				pSlider->nSdType = atoi(pGrammer->GetParam(0));
			}
			break;
		case BsUiCMD_SETSDORIENTATION:
			{
				pSlider->nSdOrientation = atoi(pGrammer->GetParam(0));

				BsAssert(pSlider->nSdOrientation == BsUiSDO_HORIZONAL ||
					pSlider->nSdOrientation == BsUiSDO_VERTICAL);
			}
			break;
		case BsUiCMD_SETRANGE:
			{
				pSlider->nMinRange = atoi(pGrammer->GetParam(0));
				pSlider->nMaxRange = atoi(pGrammer->GetParam(1));
			}
			break;
		case BsUiCMD_SETBARSIZE:
			{
				pSlider->ptBarSize.x = atoi(pGrammer->GetParam(0));
				pSlider->ptBarSize.y = atoi(pGrammer->GetParam(1));
			}
			break;
		case BsUiCMD_TEXT_POS:
			{
				pSlider->ptTextPos.x = atoi(pGrammer->GetParam(0));
				pSlider->ptTextPos.y = atoi(pGrammer->GetParam(1));
			}
			break;
		case BsUiCMD_FONT_ATTR:
			{
				for(int i=0; i<BsUiFS_COUNT; i++)
				{
					pSlider->nFontAttr[i] = g_FontAttrMgr.GetAttrId((char*)pGrammer->GetParam(i));
				}
				break;
			}
		case BsUiCMD_SETIMAGE:
			{
				pSlider->nImageMode = atoi(pGrammer->GetParam(0));
				break;
			}
		case BsUiCMD_SET_BARUV:
		case BsUiCMD_SET_BARUV_OLD:
			{
				for(int i=0; i<BsUiFS_COUNT; i++)
				{
					pSlider->nBarUVID[i] = atoi(pGrammer->GetParam(i));
				}
				break;
			}
		case BsUiCMD_SETSDLINEUV:
			{
				pSlider->nLineUV = atoi(pGrammer->GetParam(0));
			}
			break;
		case BsUiCMD_SETSDBASEUV:
			{
				pSlider->nBaseUV = atoi(pGrammer->GetParam(0));
			}
			break;
		case BsUiCMD_SET_BAR1UV:
		case BsUiCMD_SET_BAR1UV_OLD:
			{
				for(int i=0; i<BsUiFS_COUNT; i++)
				{
					pSlider->nBar1UVID[i] = atoi(pGrammer->GetParam(i));
				}
				break;
			}
		case BsUiCMD_SET_BAR2UV:
		case BsUiCMD_SET_BAR2UV_OLD:
			{
				for(int i=0; i<BsUiFS_COUNT; i++)
				{
					pSlider->nBar2UVID[i] = atoi(pGrammer->GetParam(i));
				}
				break;
			}
		case BsUiCMD_END_BRACE:
			return pSlider;
		default:
			AssertDebug(pGrammer, "Slider 정보이상");
			break;
		}
	} while (ret != 0);

	BsAssert(false);
	return pSlider;
}


BsUiTextItem* BsUiLoader::LoadTextDef(tpGrammer* pGrammer, const char* pName)
{
	BsAssert(pGrammer);

	BsUiTextItem* pText = new BsUiTextItem;
	BsAssert(pText);

	pText->nType = BsUiTYPE_TEXT;			// default
	//pText->strName = pName;

	int ret = 0;

	do {
		ret = pGrammer->Get();

		switch (ret)
		{
		case BsUiCMD_SETCLASSID:
			{
				pText->hClass= atoi(pGrammer->GetParam(0));
			}
			break;
		case BsUiCMD_SETSIZE:
			{
				pText->size.cx = atoi(pGrammer->GetParam(0));
				pText->size.cy = atoi(pGrammer->GetParam(1));
			}
			break;
		case BsUiCMD_ATTRIBUTE:
			{
				pText->nAttr = atoi(pGrammer->GetParam(0));
				break;
			}
		case BsUiCMD_TEXTID:
			{
				pText->nTextId = atoi(pGrammer->GetParam(0));
			}
			break;
		case BsUiCMD_FONT_ATTR:
			{
				for(int i=0; i<BsUiFS_COUNT; i++)
				{
					pText->nFontAttr[i] = g_FontAttrMgr.GetAttrId((char*)pGrammer->GetParam(i));
				}
				break;
			}
		case BsUiCMD_END_BRACE:
			return pText;
		default:
			AssertDebug(pGrammer, "Text 정보이상");
			break;
		}
	} while (ret != 0);

	BsAssert(false);
	return pText;
}

BsUiImageCtrlItem* BsUiLoader::LoadImageCtrlDef(tpGrammer* pGrammer, const char* pName)
{
	BsAssert(pGrammer);

	BsUiImageCtrlItem* pImage = new BsUiImageCtrlItem;
	BsAssert(pImage);

	pImage->nType = BsUiTYPE_IMAGECTRL;			// default
	

	int ret = 0;

	do {
		ret = pGrammer->Get();

		switch (ret)
		{
		case BsUiCMD_SETCLASSID:
			{
				pImage->hClass= atoi(pGrammer->GetParam(0));
			}
			break;
		case BsUiCMD_SETSIZE:
			{
				pImage->size.cx = atoi(pGrammer->GetParam(0));
				pImage->size.cy = atoi(pGrammer->GetParam(1));
			}
			break;
		case BsUiCMD_ATTRIBUTE:
			{
				pImage->nAttr = atoi(pGrammer->GetParam(0));
				break;
			}
		case BsUiCMD_SETIMAGE:
			{
				pImage->nImageMode = atoi(pGrammer->GetParam(0));
				break;
			}
		case BsUiCMD_SET_UV:
		case BsUiCMD_SET_UV_OLD:
			{
				for(int i=0; i<BsUiFS_COUNT; i++)
				{
					pImage->nUVID[i] = atoi(pGrammer->GetParam(i));
				}
				break;
			}
		case BsUiCMD_SET_BLOCK:
		case BsUiCMD_SET_BLOCK_OLD:
			{
				for(int i=0; i<BsUiFS_COUNT; i++)
				{
					pImage->nBlockID[i] = atoi(pGrammer->GetParam(i));
				}
				break;
			}
		case BsUiCMD_FILENAME:
			{
				strcpy(pImage->szFileName, pGrammer->GetParam(0));
				break;
			}
		case BsUiCMD_IMG_LOCAL:
			{
				pImage->nLocal = atoi(pGrammer->GetParam(0));
				break;
			}
		case BsUiCMD_UV:
			{
				pImage->u1 = atoi(pGrammer->GetParam(0));
				pImage->v1 = atoi(pGrammer->GetParam(1));
				pImage->u2 = atoi(pGrammer->GetParam(2));
				pImage->v2 = atoi(pGrammer->GetParam(3));
				break;
			}
		case BsUiCMD_REALSIZE:
			{
				pImage->nRealSize = atoi(pGrammer->GetParam(0));
				break;
			}
		case BsUiCMD_REVERSE:
			{
				pImage->nReverse = atoi(pGrammer->GetParam(0));
				break;
			}
		case BsUiCMD_ROTATE:
			{
				pImage->nRot = atoi(pGrammer->GetParam(0));
				break;
			}
		case BsUiCMD_END_BRACE:
			return pImage;
		default:
			AssertDebug(pGrammer, "Image 정보이상");
			break;
		}
	} while (ret != 0);

	BsAssert(false);
	return pImage;
}


void BsUiLoader::LoadValueItem(tpGrammer* pGrammer, BsUiVALUEITEMVECT& vect)
{
	BsAssert(pGrammer);
	BsAssert(vect.empty());

	int ret = 0;

	do {
		ret = pGrammer->Get();

		switch (ret)
		{
		case BsUiCMD_LISTITEM:
			{
				BsUiValueItem value;
				value.nID = atoi(pGrammer->GetParam(0));
				value.dwData = atoi(pGrammer->GetParam(0));
				vect.push_back(value);
			}
			break;
		case BsUiCMD_END_BRACE:
			return;
		default:
			AssertDebug(pGrammer, "LoadValueItem 이상");
			break;
		}
	} while (ret != 0);
}

void BsUiLoader::LoadAttachItem(tpGrammer* pGrammer, BsUiATTACHEDITEMVECT& vect)
{
	BsAssert(pGrammer);

	int ret = 0;

	do {
		ret = pGrammer->Get();

		switch (ret)
		{
		case BsUiCMD_ADDITEM:
			{
				BsUiAttachedItem item;

				item.hClass = atoi(pGrammer->GetParam(0));
				item.pos.x = atoi(pGrammer->GetParam(1));
				item.pos.y = atoi(pGrammer->GetParam(2));

				vect.push_back(item);
			}
			break;
		case BsUiCMD_END_BRACE:
			return;
		default:
			AssertDebug(pGrammer, "Attach 정보이상");
			break;
		}
	} while (ret != 0);
}
