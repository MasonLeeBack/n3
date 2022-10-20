#include "stdafx.h"

#include "BsUiStore.h"
#include "BsKernel.h"

#include "BSuiSystem.h"
#include "BSuiClassIDManager.h"
#include "BSuiDefine.h"

#include "BsUiLayer.h"
#include "BsUiButton.h"
#include "BsUiListBox.h"
#include "BsUiListCtrl.h"
#include "BsUiSlider.h"
#include "BsUiText.h"
#include "BsUiImageCtrl.h"

extern BsUiClassIDManager g_ClassIdMgr;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

BsUiStore::BsUiStore()
{
	;
}

BsUiStore::~BsUiStore()
{
	Release();
}

bool BsUiStore::SaveWindowList(const char* szFilename)
{
	FILE *fp = fopen(szFilename, "wt");

	if(fp==NULL){
		BsAssert(0);
		return false;
	}

	SaveParam(fp, NULL, BsUiCMD_VERSION, 0);

	list <BsUiWindow*> pWindowList = g_BsUiSystem.GetWindows();
	list <BsUiWindow*>::iterator itr = pWindowList.begin();

	while (itr != pWindowList.end())
	{
		BsUiWindow* pWindow = (*itr);
		BsAssert(pWindow);

		SaveWindow(fp, pWindow);

		++itr;
	}

	fclose(fp);

	return true;
}

bool BsUiStore::SaveWindow(FILE *fp, BsUiWindow* pWindow)
{
	BsAssert(pWindow);

	BsUiWindow* pWnd = pWindow;
	int nParentCnt = 0;
	while(1)
	{
		pWnd = pWnd->GetParentWindow();
		if(pWnd == NULL)
			break;
		nParentCnt++;
	}

	switch(pWindow->GetType())
	{
	case BsUiTYPE_LAYER:		SaveLayer(fp, (BsUiLayer*)pWindow, nParentCnt); break;
	case BsUiTYPE_BUTTON:		SaveButton(fp, (BsUiButton*)pWindow, nParentCnt); break;
	case BsUiTYPE_LISTBOX:		SaveListBox(fp, (BsUiListBox*)pWindow, nParentCnt); break;
	case BsUiTYPE_LISTCTRL:		SaveListCtrl(fp, (BsUiListCtrl*)pWindow, nParentCnt); break;
	case BsUiTYPE_TEXT:			SaveText(fp, (BsUiText*)pWindow, nParentCnt); break;
	case BsUiTYPE_IMAGECTRL:	SaveImageCtrl(fp, (BsUiImageCtrl*)pWindow, nParentCnt); break;
	case BsUiTYPE_SLIDER:		SaveSlider(fp, (BsUiSlider*)pWindow, nParentCnt); break;

	case BsUiTYPE_CHECKBUTTON:	return false;
	}

	SaveChildWindow(fp, pWindow, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_END_BRACE, nParentCnt);

	return true;
}


bool BsUiStore::SaveLayer(FILE *fp, BsUiLayer* pWindow, int nParentCnt)
{	
	SaveParam(fp, pWindow, BsUiCMD_BEGIN_LAYER, nParentCnt);

	SaveParam(fp, pWindow, BsUiCMD_SETCLASSID, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_SETPOS, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_SETSIZE, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_ATTRIBUTE, nParentCnt);

	SaveParam(fp, pWindow, BsUiCMD_SETIMAGE, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_SET_UV, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_SET_BLOCK, nParentCnt);
	
	return true;
}



bool BsUiStore::SaveButton(FILE *fp, BsUiButton* pWindow, int nParentCnt)
{
	SaveParam(fp, pWindow, BsUiCMD_BEGIN_BUTTON, nParentCnt);

	SaveParam(fp, pWindow, BsUiCMD_SETCLASSID, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_SETSIZE, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_ATTRIBUTE, nParentCnt);

	SaveParam(fp, pWindow, BsUiCMD_SETGROUP, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_SETCHECK, nParentCnt);

	SaveParam(fp, pWindow, BsUiCMD_SETIMAGE, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_SET_UV, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_SET_BLOCK, nParentCnt);

	SaveParam(fp, pWindow, BsUiCMD_TEXTID, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_TEXT_POS, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_FONT_ATTR, nParentCnt);


	return true;
}

bool BsUiStore::SaveListBox(FILE *fp, BsUiListBox* pWindow, int nParentCnt)
{
	SaveParam(fp, pWindow, BsUiCMD_BEGIN_LISTBOX, nParentCnt);

	SaveParam(fp, pWindow, BsUiCMD_SETCLASSID, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_SETSIZE, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_ATTRIBUTE, nParentCnt);
	
	SaveParam(fp, pWindow, BsUiCMD_SETLINE, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_DRAWSELBOX, nParentCnt);
	
	SaveParam(fp, pWindow, BsUiCMD_SETIMAGE, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_SET_SELBOXUV, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_SET_UV, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_SET_SELBOXBLOCK, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_SET_BLOCK, nParentCnt);

	SaveParam(fp, pWindow, BsUiCMD_TEXT_POS, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_FONT_ATTR, nParentCnt);
	
	return true;
}

bool BsUiStore::SaveListCtrl(FILE *fp, BsUiListCtrl* pWindow, int nParentCnt)
{
	SaveParam(fp, pWindow, BsUiCMD_BEGIN_LISTBOX, nParentCnt);

	SaveParam(fp, pWindow, BsUiCMD_SETCLASSID, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_SETSIZE, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_ATTRIBUTE, nParentCnt);

	SaveParam(fp, pWindow, BsUiCMD_SETLBTYPE, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_SETCOLUMN, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_SETLINE, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_DRAWSELBOX, nParentCnt);

	SaveParam(fp, pWindow, BsUiCMD_SETIMAGE, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_SET_UV, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_SET_BLOCK, nParentCnt);

	SaveParam(fp, pWindow, BsUiCMD_TEXT_POS, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_FONT_ATTR, nParentCnt);

	return true;
}

bool BsUiStore::SaveText(FILE *fp, BsUiText* pWindow, int nParentCnt)
{
	SaveParam(fp, pWindow, BsUiCMD_BEGIN_TEXT, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_SETCLASSID, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_SETSIZE, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_ATTRIBUTE, nParentCnt);

	SaveParam(fp, pWindow, BsUiCMD_TEXTID, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_FONT_ATTR, nParentCnt);

	return true;
}

bool BsUiStore::SaveImageCtrl(FILE *fp, BsUiImageCtrl* pWindow, int nParentCnt)
{
	SaveParam(fp, pWindow, BsUiCMD_BEGIN_IMAGE, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_SETCLASSID, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_SETSIZE, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_ATTRIBUTE, nParentCnt);

	SaveParam(fp, pWindow, BsUiCMD_SETIMAGE, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_SET_UV, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_SET_BLOCK, nParentCnt);

	SaveParam(fp, pWindow, BsUiCMD_FILENAME, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_IMG_LOCAL, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_UV, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_REALSIZE, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_REVERSE, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_ROTATE, nParentCnt);

	return true;
}


bool BsUiStore::SaveSlider(FILE *fp, BsUiSlider* pWindow, int nParentCnt)
{
	SaveParam(fp, pWindow, BsUiCMD_BEGIN_SLIDER, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_SETCLASSID, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_SETSIZE, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_ATTRIBUTE, nParentCnt);

	SaveParam(fp, pWindow, BsUiCMD_SETSDTYPE, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_SETSDORIENTATION, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_SETRANGE, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_SETBARSIZE, nParentCnt);

	SaveParam(fp, pWindow, BsUiCMD_SETIMAGE, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_SET_BARUV, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_SETSDLINEUV, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_SETSDBASEUV, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_SET_BAR1UV, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_SET_BAR2UV, nParentCnt);

	SaveParam(fp, pWindow, BsUiCMD_TEXT_POS, nParentCnt);
	SaveParam(fp, pWindow, BsUiCMD_FONT_ATTR, nParentCnt);
	
	return true;
}


bool BsUiStore::SaveChildWindow(FILE *fp, BsUiWindow* pWindow, int nParentCnt)
{
	BsUiWindow::BsUiWINDOWLIST childList = pWindow->GetChildWindowList();
	if(childList.size() <= 0)
		return false;

	SaveParam(fp, pWindow,  BsUiCMD_BEGIN_ATTACH, nParentCnt);
	
	BsUiWindow::BsUiWINDOWLISTITOR itr = childList.begin();
	while (itr != childList.end())
	{
		BsUiWindow* pChildWindow = (*itr);

		SaveParam(fp, pChildWindow, BsUiCMD_ADDITEM, nParentCnt);
		
		++itr;
	}

	SaveParam(fp, pWindow,  BsUiCMD_END_BRACE, nParentCnt+1);

	itr = childList.begin();
	while (itr != childList.end())
	{
		BsUiWindow* pChildWindow = (*itr);
		SaveWindow(fp, pChildWindow);

		++itr;
	}

	return true;
}


bool BsUiStore::SaveParam(FILE *fp, BsUiWindow* pWindow, int cmd, int nParentCount)
{	
	char szTab[64];
	memset(szTab, 0, 64);
	for(int i=0; i<nParentCount; ++i)
	{
		strcat(szTab, "\t");
	}

	char szTag[256];
	memset(szTag, 0, 256);

	switch((BsUiCMD)cmd)
	{
	case BsUiCMD_VERSION:
		{
			sprintf(szTag, "UI_VERSION(\"%s\")\n\n", _UI_VERSION);
			break;
		}
	case BsUiCMD_END_BRACE:
		{
			sprintf(szTag, "%s}\n\n", szTab);
			break;
		}
	case BsUiCMD_BEGIN_LAYER:
		{
			char* szName = g_ClassIdMgr.GetName(pWindow->GetClass());
			sprintf(szTag, "%sLAYER (\"%s\")\n%s{\n", szTab, szName, szTab);
			break;
		}
	case BsUiCMD_BEGIN_BUTTON:
		{
			char* szName = g_ClassIdMgr.GetName(pWindow->GetClass());
			sprintf(szTag, "%sBUTTON (\"%s\")\n%s{\n", szTab, szName, szTab);
			break;
		}
	case BsUiCMD_BEGIN_TEXT:
		{
			char* szName = g_ClassIdMgr.GetName(pWindow->GetClass());
			sprintf(szTag, "%sTEXT (\"%s\")\n%s{\n", szTab, szName, szTab);
			break;
		}
	case BsUiCMD_BEGIN_LISTBOX:
		{
			char* szName = g_ClassIdMgr.GetName(pWindow->GetClass());
			sprintf(szTag, "%sLISTBOX (\"%s\")\n%s{\n", szTab, szName, szTab);
			break;
		}
	case BsUiCMD_BEGIN_IMAGE:
		{
			char* szName = g_ClassIdMgr.GetName(pWindow->GetClass());
			sprintf(szTag, "%sIMAGE (\"%s\")\n%s{\n", szTab, szName, szTab);
			break;
		}
	case BsUiCMD_BEGIN_SLIDER:
		{
			char* szName = g_ClassIdMgr.GetName(pWindow->GetClass());
			sprintf(szTag, "%sSLIDER (\"%s\")\n%s{\n", szTab, szName, szTab);
			break;
		}
	case BsUiCMD_BEGIN_CHECKBUTTON: return false;
	case BsUiCMD_BEGIN_ATTACH:
		{	
			sprintf(szTag, "\n%s\tATTACH\n%s\t{\n", szTab, szTab);
			break;
		}
	case BsUiCMD_ADDITEM:
		{
			int nClass = pWindow->GetClass();
			int nPosX = pWindow->GetWindowPos().x;
			int nPosY = pWindow->GetWindowPos().y;
			sprintf(szTag, "%s\t\tADDITEM = CLASSID(%d), POS(%d, %d)\n", szTab, nClass, nPosX, nPosY);
			break;
		}
	case BsUiCMD_BEGIN_VALUE:		return false;
	//default
	case BsUiCMD_SETCLASSID:
		{
			int nClass = pWindow->GetClass();
			sprintf(szTag, "%s\tCLASSID = %d\n", szTab, nClass);
			break;
		}
	case BsUiCMD_SETSIZE:
		{
			int nSizeX = pWindow->GetWindowSize().x;
			int nSizeY = pWindow->GetWindowSize().y;
			sprintf(szTag, "%s\tSIZE = %d, %d\n", szTab, nSizeX, nSizeY);
			break;
		}
	case BsUiCMD_ATTRIBUTE:
		{
			int nAttr = pWindow->GetWindowAttr();
			sprintf(szTag, "%s\tATTRIBUTE = %d\n", szTab, nAttr);
			break;
		}
	case BsUiCMD_SETIMAGE:
		{
			int nMode = pWindow->GetImageMode();
			sprintf(szTag, "%s\tIMAGE_MODE = %d\n", szTab, nMode);
			break;
		}
	case BsUiCMD_SET_UV:
		{
			int nSelUVId = pWindow->GetImageUVID(BsUiFS_SELECTED);
			int nDefUVId = pWindow->GetImageUVID(BsUiFS_DEFAULTED);
			int nDisUVId = pWindow->GetImageUVID(BsUiFS_DISABLE);

			if(nSelUVId == -1 && nDefUVId == -1 && nDisUVId == -1){
				break;
			}

			sprintf(szTag, "%s\tUV_MODE: %d, %d, %d\n",
				szTab, nSelUVId, nDefUVId, nDisUVId);
			break;
		}
	case BsUiCMD_SET_BLOCK:
		{
			return true;

			int nSelBlock = pWindow->GetImageBlock(BsUiFS_SELECTED);
			int nDefBlock = pWindow->GetImageBlock(BsUiFS_DEFAULTED);
			int nDisBlock = pWindow->GetImageBlock(BsUiFS_DISABLE);

			if(nSelBlock == -1 && nDefBlock == -1 && nDisBlock == -1){
				break;
			}

			sprintf(szTag, "%s\tBLOCK_MODE: %d, %d, %d\n",
				szTab, nSelBlock, nDefBlock, nDisBlock);
			break;
		}
	case BsUiCMD_SETSTRING:			return false;
	case BsUiCMD_BEGIN_ON:			return false;
	case BsUiCMD_BEGIN_ONDEFAULT:	return false;
	//type layer
	case BsUiCMD_SETPOS:
		{
			int nPosX = pWindow->GetWindowPos().x;
			int nPosY = pWindow->GetWindowPos().y;
			sprintf(szTag, "%s\tPOS = %d, %d\n", szTab, nPosX, nPosY);
			break;
		}
	//type button
	case BsUiCMD_SETCHECK:
		{
			int nCheck = ((BsUiButton*)pWindow)->GetCheck();
			sprintf(szTag, "%s\tCHECK = %d\n", szTab, nCheck);
			break;
		}
	case BsUiCMD_SETGROUP:
		{
			int nGroup = ((BsUiButton*)pWindow)->GetGroupID();

			if(nGroup == -1){
				break;
			}

			sprintf(szTag, "%s\tGROUP = %d\n", szTab, nGroup);
			break;
		}
	//type list box
	case BsUiCMD_SETLINE:
		{	
			int nValue =  ((BsUiListBox*)pWindow)->GetLines();
			sprintf(szTag, "%s\tLINE = %d\n", szTab, nValue);
			break;
		}
	case BsUiCMD_LISTITEM:		return false;
	case BsUiCMD_DRAWSELBOX:
		{
			int nValue =  ((BsUiListBox*)pWindow)->IsDrawSelBox();
			sprintf(szTag, "%s\tSHOW_SELBOX = %d\n", szTab, nValue);
			break;
		}
	case BsUiCMD_SET_SELBOXUV:
		{
			int nSelUVId = ((BsUiListBox*)pWindow)->GetSelBoxUVID(BsUiFS_SELECTED);
			int nDefUVId = ((BsUiListBox*)pWindow)->GetSelBoxUVID(BsUiFS_DEFAULTED);
			int nDisUVId = ((BsUiListBox*)pWindow)->GetSelBoxUVID(BsUiFS_DISABLE);

			if(nSelUVId == -1 && nDefUVId == -1 && nDisUVId == -1){
				break;
			}

			sprintf(szTag, "%s\tSELBOX_UV_MODE: %d, %d, %d\n",
				szTab, nSelUVId, nDefUVId, nDisUVId);
			break;
		}
	case BsUiCMD_SET_SELBOXBLOCK:
		{
			int nSelBlock = ((BsUiListBox*)pWindow)->GetSelBoxBlock(BsUiFS_SELECTED);
			int nDefBlock = ((BsUiListBox*)pWindow)->GetSelBoxBlock(BsUiFS_DEFAULTED);
			int nDisBlock = ((BsUiListBox*)pWindow)->GetSelBoxBlock(BsUiFS_DISABLE);

			if(nSelBlock == -1 && nDefBlock == -1 && nDisBlock == -1){
				break;
			}

			sprintf(szTag, "%s\tSELBOX_BLOCK_MODE: %d, %d, %d\n",
				szTab, nSelBlock, nDefBlock, nDisBlock);
			break;
		}
	// type listctrl
	case BsUiCMD_SETLBTYPE:
		{
			int nValue =  ((BsUiListCtrl*)pWindow)->GetLbType();
			sprintf(szTag, "%s\tLBTYPE = %d\n", szTab, nValue);
			break;
		}
	case BsUiCMD_SETCOLUMN:
		{
			int nValue =  ((BsUiListCtrl*)pWindow)->GetColumns();
			sprintf(szTag, "%s\tCOLUMN = %d\n", szTab, nValue);
			break;
		}
	//type slider
	case BsUiCMD_SETSDTYPE:
		{
			int nValue = ((BsUiSlider*)pWindow)->GetSDType();
			sprintf(szTag, "%s\tSDTYPE = %d\n", szTab, nValue);
			break;
		}
	case BsUiCMD_SETSDORIENTATION:
		{
			int nValue = ((BsUiSlider*)pWindow)->GetOrientation();
			sprintf(szTag, "%s\tSDOTYPE = %d\n", szTab, nValue);
			break;
		}
	case BsUiCMD_SETRANGE:
		{
			int nMin = ((BsUiSlider*)pWindow)->GetMinRange();
			int nMax = ((BsUiSlider*)pWindow)->GetMaxRange();
			sprintf(szTag, "%s\tRANGE = %d, %d\n", szTab, nMin, nMax);
			break;
		}
	case BsUiCMD_SETBARSIZE:
		{
			POINT ptValue = ((BsUiSlider*)pWindow)->GetBarSize();
			sprintf(szTag, "%s\tBAR_SIZE = %d, %d\n", szTab, ptValue.x, ptValue.y);
			break;
		}
	case BsUiCMD_SET_BARUV:
		{
			int nSelBar = ((BsUiSlider*)pWindow)->GetBarUVID(BsUiFS_SELECTED);
			int nDefBar = ((BsUiSlider*)pWindow)->GetBarUVID(BsUiFS_DEFAULTED);
			int nDisBar = ((BsUiSlider*)pWindow)->GetBarUVID(BsUiFS_DISABLE);

			if(nSelBar == -1 && nDefBar == -1 && nDisBar == -1){
				break;
			}

			sprintf(szTag, "%s\tBAR_UV : %d, %d, %d\n",
				szTab, nSelBar, nDefBar, nDisBar);
			break;
		}
	case BsUiCMD_SETSDLINEUV:
		{
			int nValue = ((BsUiSlider*)pWindow)->GetLineUVID();

			if(nValue == -1){
				break;
			}

			sprintf(szTag, "%s\tSD_LINE_UV = %d\n", szTab, nValue);
			break;
		}
	case BsUiCMD_SETSDBASEUV:
		{
			int nValue = ((BsUiSlider*)pWindow)->GetBaseUVID();

			if(nValue == -1){
				break;
			}

			sprintf(szTag, "%s\tSD_BASE_UV = %d\n", szTab, nValue);
			break;
		}
	case BsUiCMD_SET_BAR1UV:
		{
			int nSelBar = ((BsUiSlider*)pWindow)->GetBar1UVID(BsUiFS_SELECTED);
			int nDefBar = ((BsUiSlider*)pWindow)->GetBar1UVID(BsUiFS_DEFAULTED);
			int nDisBar = ((BsUiSlider*)pWindow)->GetBar1UVID(BsUiFS_DISABLE);

			if(nSelBar == -1 && nDefBar == -1 && nDisBar == -1){
				break;
			}

			sprintf(szTag, "%s\tBAR1_UV : %d, %d, %d\n",
				szTab, nSelBar, nDefBar, nDisBar);
			break;
		}
	case BsUiCMD_SET_BAR2UV:
		{
			int nSelBar = ((BsUiSlider*)pWindow)->GetBar2UVID(BsUiFS_SELECTED);
			int nDefBar = ((BsUiSlider*)pWindow)->GetBar2UVID(BsUiFS_DEFAULTED);
			int nDisBar = ((BsUiSlider*)pWindow)->GetBar2UVID(BsUiFS_DISABLE);

			if(nSelBar == -1 && nDefBar == -1 && nDisBar == -1){
				break;
			}

			sprintf(szTag, "%s\tBAR2_UV : %d, %d, %d\n",
				szTab, nSelBar, nDefBar, nDisBar);
			break;
		}
	//text
	case BsUiCMD_TEXTID:
		{
			int nValue = pWindow->GetItemTextID();

			if(nValue == -1){
				break;
			}

			sprintf(szTag, "%s\tTEXTID = %d\n", szTab, nValue);
			break;
		}
	case BsUiCMD_TEXT_POS:
		{
			int nPosX = pWindow->GetItemPos().x;
			int nPosY = pWindow->GetItemPos().y;

			if(nPosX == 0 && nPosY == 0){
				break;
			}

			sprintf(szTag, "%s\tTEXTPOS = %d, %d\n", 
				szTab, nPosX, nPosY);
			break;
		}
	case BsUiCMD_FONT_ATTR:
		{
			char* szName1 = g_FontAttrMgr.GetName(pWindow->GetFontAttr(BsUiFS_SELECTED));
			char* szName2 = g_FontAttrMgr.GetName(pWindow->GetFontAttr(BsUiFS_DEFAULTED));
			char* szName3 = g_FontAttrMgr.GetName(pWindow->GetFontAttr(BsUiFS_DISABLE));

			if(strcmp(szName1, _TEXT_DEFAULT_ATTR_NAME) == NULL &&
				strcmp(szName2, _TEXT_DEFAULT_ATTR_NAME) == NULL &&
				strcmp(szName3, _TEXT_DEFAULT_ATTR_NAME) == NULL){
				break;
			}

			sprintf(szTag, "%s\tFONTATTRID = \"%s\", \"%s\", \"%s\"\n", 
				szTab, szName1, szName2, szName3);
			break;
		}
	//image
	case BsUiCMD_FILENAME:
		{
			BsUiImageCtrl* pImageCtrl = (BsUiImageCtrl*)pWindow;
						
			sprintf(szTag, "%s\tFILE_NAME = \"%s\"\n", szTab, pImageCtrl->GetImageInfo()->cFileName);
			break;
		}
	case BsUiCMD_IMG_LOCAL:
		{
			int nValue = (((BsUiImageCtrl*)pWindow)->GetImageInfo()->bLocal)? 1:0;
			sprintf(szTag, "%s\tFILE_LOCAL = %d\n", szTab, nValue);
			break;
		}
	case BsUiCMD_UV:
		{
			int u1 = ((BsUiImageCtrl*)pWindow)->GetImageInfo()->u1;
			int v1 = ((BsUiImageCtrl*)pWindow)->GetImageInfo()->v1;
			int u2 = ((BsUiImageCtrl*)pWindow)->GetImageInfo()->u2;
			int v2 = ((BsUiImageCtrl*)pWindow)->GetImageInfo()->v2;

			sprintf(szTag, "%s\tUV = %d, %d, %d, %d\n", szTab, u1, v1, u2, v2);
			break;
		}
	case BsUiCMD_REALSIZE:
		{
			int nValue = (((BsUiImageCtrl*)pWindow)->GetImageInfo()->bRealSize)? 1:0;
			sprintf(szTag, "%s\tREALSIZE = %d\n", szTab, nValue);
			break;
		}
	case BsUiCMD_REVERSE:
		{
			int nValue = (((BsUiImageCtrl*)pWindow)->GetImageInfo()->bReverse)? 1:0;
			sprintf(szTag, "%s\tREVERSE = %d\n", szTab, nValue);
			break;
		}
	case BsUiCMD_ROTATE:
		{
			int nValue = ((BsUiImageCtrl*)pWindow)->GetImageInfo()->nRot;
			sprintf(szTag, "%s\tROTATE = %d\n", szTab, nValue);
			break;
		}
	}

	fwrite(&szTag, sizeof(char)*strlen(szTag), 1, fp);

	return true;
}

