#include "stdafx.h"

#include "BSuiGDISkin.h"

#include "BsUiSystem.h"
#include "BsUiGDIView.h"

#include "BSuiWindow.h"
#include "BsUiLayer.h"
#include "BsUiButton.h"
#include "BsUiListBox.h"
#include "BsUiListCtrl.h"
#include "BsUiSlider.h"
#include "BSuiText.h"
#include "BSuiImageCtrl.h"

#include "BsUiClassIDManager.h"
#include "BsUiFontAttrManager.h"
#include "BsUiUVManager.h"
#include "TextTable.h"
#include "BsUiFont.h"
#include "BsKernel.h"

#ifdef _BSUI_TOOL
#include "MainFrm.h"
#include "BSUIToolDoc.h"
#include "BSUIToolView.h"
#endif


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


extern BsUiClassIDManager	g_ClassIdMgr;
extern BsUiFontAttrManager	g_FontAttrMgr;
extern FCTextTable*			g_TextTable;

BsUiGDIView::BsUiGDIView()
{	
#ifdef _BSUI_TOOL
	m_nDragBoxIndex = -1;
#endif

	m_ColorFill =D3DXCOLOR(0.7f,0.7f,0.7f,1);
	m_ColorWhite=D3DXCOLOR(1,   1,   1,   1);
	m_ColorBlack=D3DXCOLOR(0,   0,   0,   1);
	m_ColorBox = D3DXCOLOR(1,   1,   1,   1);
}

BsUiGDIView::~BsUiGDIView()
{	
#ifdef _BSUI_TOOL
	if(g_pFont != NULL)
	{
		delete g_pFont;
		g_pFont = NULL;
	}
	CBsKernel::DestroyInstance();
#endif
}


bool BsUiGDIView::GetText(int nTextId, char* szText, const size_t szText_len)
{	
	if(g_TextTable == NULL)
		return false;
		
	if(g_TextTable->GetText(nTextId, szText, szText_len) == false)
		return false;

	return true;
}

void BsUiGDIView::Draw(BsUiWindow* pWindow)
{
	switch(pWindow->GetType())
	{
	case BsUiTYPE_LAYER:	DrawUiLayer((BsUiLayer*)pWindow); break;
	case BsUiTYPE_BUTTON:	DrawUiButton((BsUiButton*)pWindow); break;
	case BsUiTYPE_LISTBOX:	DrawUiListBox((BsUiListBox*)pWindow); break;
	case BsUiTYPE_LISTCTRL:	DrawUiListCtrl((BsUiListCtrl*)pWindow); break;
	case BsUiTYPE_TEXT:		DrawUiText((BsUiText*)pWindow); break;
	case BsUiTYPE_IMAGECTRL:DrawUiImageCtrl((BsUiImageCtrl*)pWindow); break;
	case BsUiTYPE_SLIDER:	DrawUiSlider((BsUiSlider*)pWindow); break;
	}
}

void BsUiGDIView::DrawUiLayer(BsUiLayer* pWindow)
{	
	DrawWindow((BsUiWindow*)pWindow);
}

void BsUiGDIView::DrawUiButton(BsUiButton* pWindow)
{	
	DrawWindow((BsUiWindow*)pWindow);
	DrawText((BsUiWindow*)pWindow);
}

void BsUiGDIView::DrawUiListBox(BsUiListBox* pWindow)
{
	DrawWindow((BsUiWindow*)pWindow);

	int nCurSel = 0;
	if(pWindow->GetItemCount() != 0){
		nCurSel = pWindow->GetCurSel();
	}

	RECT rect;
	pWindow->GetItemRect(nCurSel, rect);

	D3DXCOLOR clr = pWindow->GetColor();
	if(pWindow->GetParentWindow() != NULL){
		clr.a *= pWindow->GetParentWindow()->GetColor().a;
	}

	if(pWindow->IsDrawSelBox())
	{	
		DrawWindow(pWindow->GetType(), rect,
			pWindow->GetImageMode(),
			pWindow->GetCurSelBoxUVID(),
			pWindow->GetCurSelBoxBlockID(),
			clr);
	}


#ifndef _BSUI_TOOL
	int nEnd = pWindow->GetTopIndex() + pWindow->GetLines();
	if (nEnd > pWindow->GetItemCount())
		nEnd = pWindow->GetItemCount();

	for(int i=pWindow->GetTopIndex(); i<nEnd; i++)
	{
		if(strlen(pWindow->GetString(i)) == 0)
			continue;

		RECT rect;
		pWindow->GetItemRect(i, rect);

		BsUiFocusState focusState = pWindow->GetCurFocusState();
		if(focusState == BsUiFS_SELECTED)// || focusState == BsUiFS_DEFAULTED)
		{
			if(pWindow->GetCurSel() == i)
			{
				BsUiAttrId nDisAttrID = pWindow->GetFontAttr(BsUiFS_DISABLE);
				DrawText(rect, pWindow->GetItemPos(),
					g_FontAttrMgr.GetAttrText(nDisAttrID),
					(char*)(pWindow->GetString(i)), clr.a,
					BsUiFS_DISABLE);

				focusState = BsUiFS_SELECTED;

			}else{
				focusState = BsUiFS_DEFAULTED;
			}
		}		

		DrawText(rect, pWindow->GetItemPos(),
			g_FontAttrMgr.GetAttrText(pWindow->GetCurFontAttr(i)),
			(char*)(pWindow->GetString(i)), clr.a,
			focusState);
	}

#else
	for(int i=0; i<pWindow->GetLines(); i++)
	{	
		RECT rect;
		pWindow->GetItemRect(i, rect);

		BsUiAttrId AttrId = pWindow->GetCurFontAttr(i);
		if(pWindow->IsEnableWindow() != false &&
            pWindow->GetCurSel() == -1 && i==0 &&
			BsUiSystem::GetFocusWindow() == pWindow)
		{
			AttrId = pWindow->GetFontAttr(BsUiFS_SELECTED);
		}
/*
		DrawText(rect, pWindow->GetItemPos(),
			g_FontAttrMgr.GetAttrText(AttrId),
			g_ClassIdMgr.GetName(pWindow->GetClass()));
			*/
	}
#endif
}

void BsUiGDIView::DrawUiListCtrl(BsUiListCtrl* pWindow)
{
	DrawWindow((BsUiWindow*)pWindow);

	D3DXCOLOR clr = pWindow->GetColor();
	if(pWindow->GetParentWindow() != NULL){
		clr.a *= pWindow->GetParentWindow()->GetColor().a;
	}

	if(pWindow->IsDrawSelBox())
	{
		int nCurSel = 0;
		if(pWindow->GetItemCount() != 0)
		{
			nCurSel = pWindow->GetCurSel();
		}

		RECT rect;
		pWindow->GetItemRect(nCurSel, rect);

		DrawWindow(pWindow->GetType(), rect,
			pWindow->GetImageMode(),
			pWindow->GetCurImageUVID(),
			pWindow->GetCurImageBlockID(),
			clr);
	}


#ifndef _BSUI_TOOL
	int nEnd = pWindow->GetTopIndex() + pWindow->GetCountPerPage();
	if (nEnd > pWindow->GetItemCount())
		nEnd = pWindow->GetItemCount();

	for(int i=pWindow->GetTopIndex(); i<nEnd; i++)
	{
		if(strlen(pWindow->GetString(i)) == 0)
			continue;

		RECT rect;
		pWindow->GetItemRect(i, rect);

		DrawText(rect, pWindow->GetItemPos(),
			g_FontAttrMgr.GetAttrText(pWindow->GetCurFontAttr(i)),
			(char*)(pWindow->GetString(i)), clr.a,
			pWindow->GetCurFocusState());
	}

#else

	for(int i=0; i<pWindow->GetLines(); i++)
	{	
		RECT rect;
		pWindow->GetItemRect(i, rect);

		BsUiAttrId AttrId = pWindow->GetCurFontAttr(i);
		if(pWindow->IsEnableWindow() != false &&
			pWindow->GetCurSel() == -1 && i==0 &&
			BsUiSystem::GetFocusWindow() == pWindow)
		{
			AttrId = pWindow->GetFontAttr(BsUiFS_SELECTED);
		}
	
		DrawText(rect, pWindow->GetItemPos(),
			g_FontAttrMgr.GetAttrText(AttrId),
			g_ClassIdMgr.GetName(pWindow->GetClass()), clr.a,
			pWindow->GetCurFocusState());
	}
#endif
}

void BsUiGDIView::DrawUiText(BsUiText* pWindow)
{
#ifdef _BSUI_TOOL
	DrawWindow((BsUiWindow*)pWindow);
#endif
	DrawText((BsUiWindow*)pWindow);
}

void BsUiGDIView::DrawUiImageCtrl(BsUiImageCtrl* pWindow)
{
	switch(pWindow->GetImageMode())
	{
	case _IMAGE_MODE_NONE: break;
	case _IMAGE_MODE_DEFAULT:
	case _IMAGE_MODE_UV:
	case _IMAGE_MODE_BLOCK:
		{
			DrawWindow((BsUiWindow*)pWindow);
			break;
		}
	case _IMAGE_MODE_FILE:
		{
		
			POINT pos = pWindow->GetWindowPos();
			POINT size = pWindow->GetWindowSize();

			TransPosSizeforScrollnZoom(pos, size);

			ImageCtrlInfo* Img = pWindow->GetImageInfo();

			if(Img->nTexId != -1)
			{
				D3DXCOLOR clr = pWindow->GetColor();
				if(pWindow->GetParentWindow() != NULL){
					clr.a *= pWindow->GetParentWindow()->GetColor().a;
				}

				BsUiImageCtrl* pImgCtrl = (BsUiImageCtrl*)pWindow;
				CBsKernel::GetInstance().DrawUIBox_s(_Ui_Mode_Image,
					pos.x, pos.y,
					size.x, size.y,
					0.f, clr,
					0.f,
					Img->nTexId, 
					Img->u1, Img->v1,
					Img->u2, Img->v2,
					-1, pImgCtrl->IsRenderTargetTexture());
			}
			break;
		}
	}
	
}

void BsUiGDIView::DrawUiSlider(BsUiSlider* pWindow)
{	
	switch(pWindow->GetSDType())
	{
		case BsUiSDTYPE_NORMAL:		DrawUiSliderNormal(pWindow); break;
		case BsUiSDTYPE_NUMERAL:	DrawUiSliderNumeral(pWindow); break;
		case BsUiSDTYPE_TEXT:		DrawUiSliderText(pWindow); break;
		case BsUiSDTYPE_GAUGE:		DrawUiSliderGauge(pWindow); break;
		default: BsAssert(0);
	}
}

void BsUiGDIView::DrawUiSliderNormal(BsUiSlider* pWindow)
{
	RECT rectWindow, rectLine, rectBar;
	pWindow->GetWindowRect(&rectWindow);
	pWindow->GetLineRect(&rectLine);
	pWindow->GetBarRect(&rectBar);

	D3DXCOLOR clr = pWindow->GetColor();
	if(pWindow->GetParentWindow() != NULL){
		clr.a *= pWindow->GetParentWindow()->GetColor().a;
	}

	DrawWindow(pWindow->GetType(), rectWindow,
		pWindow->GetImageMode(),
		pWindow->GetBaseUVID(), -1,
		clr);
	
	DrawWindow(pWindow->GetType(), rectLine,
		pWindow->GetImageMode(),
		pWindow->GetLineUVID(), -1,
		clr);
	
	DrawWindow(pWindow->GetType(), rectBar,
		pWindow->GetImageMode(),
		pWindow->GetCurBarUVID(), -1,
		clr);
}

void BsUiGDIView::DrawUiSliderGauge(BsUiSlider* pWindow)
{
	RECT rectWindow, rectBar;
	pWindow->GetWindowRect(&rectWindow);
	pWindow->GetGaugeBarRect(&rectBar);

	D3DXCOLOR clr = pWindow->GetColor();
	if(pWindow->GetParentWindow() != NULL){
		clr.a *= pWindow->GetParentWindow()->GetColor().a;
	}

	DrawWindow(pWindow->GetType(), rectWindow,
		pWindow->GetImageMode(),
		pWindow->GetBaseUVID(), -1,
		clr);

	if(pWindow->GetImageMode() == _IMAGE_MODE_UV)
	{
		int nUVId = pWindow->GetCurBarUVID();
#ifdef _BSUI_TOOL
		UVImage* Img = g_EditUVMgr.GetUVImage(nUVId);
#else
		UVImage* Img = g_UVMgr.GetUVImage(nUVId);
#endif
		if(Img == NULL){
			return;
		}

		POINT size;
		size.x = rectBar.right - rectBar.left;
		size.y = rectBar.bottom - rectBar.top;
		
		int tempSX = Img->u2 - Img->u1;
		int tempSY = Img->v2 - Img->v1;
		if(tempSX >= size.x){
			tempSX = size.x;
		}

		if(tempSY >= size.y){
			tempSY = size.y;
		}

		int u1 = Img->u1;
		int v1 = Img->v1;
		int u2 = Img->u1 + tempSX;
		int v2 = Img->v1 + tempSY;

		D3DXCOLOR clr1 = D3DXCOLOR(1, 1, 1, clr.a);
		CBsKernel::GetInstance().DrawUIBox_s(_Ui_Mode_Image,
			rectBar.left, rectBar.top,
			size.x, size.y,
			0.f, clr1,
			0.f,
			Img->nTexId,
			u1, v1,
			u2, v2);
	}
	else
	{
		DrawWindow(pWindow->GetType(), rectBar,
			pWindow->GetImageMode(),
			pWindow->GetCurBarUVID(), -1,
			clr);
	}
}

void BsUiGDIView::DrawUiSliderNumeral(BsUiSlider* pWindow)
{	
	RECT rectWindow, rectBar1, rectBar2;
	pWindow->GetWindowRect(&rectWindow);
	pWindow->GetBar1Rect(&rectBar1);
	pWindow->GetBar2Rect(&rectBar2);

	D3DXCOLOR clr = pWindow->GetColor();
	if(pWindow->GetParentWindow() != NULL){
		clr.a *= pWindow->GetParentWindow()->GetColor().a;
	}

	DrawWindow(pWindow->GetType(), rectWindow,
		pWindow->GetImageMode(),
		pWindow->GetBaseUVID(), -1,
		clr);

	DrawWindow(pWindow->GetType(), rectBar1,
		pWindow->GetImageMode(),
		pWindow->GetCurBar1UVID(), -1,
		clr);
	
	DrawWindow(pWindow->GetType(), rectBar2,
		pWindow->GetImageMode(),
		pWindow->GetCurBar2UVID(), -1,
		clr);

	char szCount[64];
	sprintf(szCount, "%d", pWindow->GetBarPos());
	BsUiAttrId nAttrID = pWindow->GetCurFontAttr();

	RECT rect;
	pWindow->GetWindowRect(&rect);

	if(pWindow->GetCurFocusState() == BsUiFS_SELECTED)
	{
		BsUiAttrId nDisAttrID = pWindow->GetFontAttr(BsUiFS_DISABLE);
		DrawText(rect, pWindow->GetItemPos(),
			g_FontAttrMgr.GetAttrText(nDisAttrID),
			szCount, clr.a,
			BsUiFS_DISABLE);
	}

	DrawText(rect, pWindow->GetItemPos(),
		g_FontAttrMgr.GetAttrText(nAttrID),
		szCount, clr.a,
		pWindow->GetCurFocusState());
}

void BsUiGDIView::DrawUiSliderText(BsUiSlider* pWindow)
{
	RECT rectWindow, rectBar1, rectBar2;
	
	pWindow->GetBar1Rect(&rectBar1);
	pWindow->GetBar2Rect(&rectBar2);

	rectWindow.left = rectBar1.right;
	rectWindow.top = rectBar1.top;
	rectWindow.right = rectBar2.left;
	rectWindow.bottom = rectBar2.bottom;

	D3DXCOLOR clr = pWindow->GetColor();
	if(pWindow->GetParentWindow() != NULL){
		clr.a *= pWindow->GetParentWindow()->GetColor().a;
	}

	DrawWindow(pWindow->GetType(), rectWindow,
		pWindow->GetImageMode(),
		pWindow->GetBaseUVID(), -1,
		clr);
	
	DrawWindow(pWindow->GetType(), rectBar1,
		pWindow->GetImageMode(),
		pWindow->GetCurBar1UVID(), -1,
		clr);
	
	DrawWindow(pWindow->GetType(), rectBar2,
		pWindow->GetImageMode(),
		pWindow->GetCurBar2UVID(), -1,
		clr);
	
	BsUiAttrId nAttrID = pWindow->GetCurFontAttr();
	BsUiItemInfo* pItemInfo = pWindow->GetCurItem();
	char szCurText[512];
	if(pItemInfo != NULL && pItemInfo->szText != NULL)
	{
		strcpy(szCurText, pItemInfo->szText);	
	}
	else
	{
		strcpy(szCurText, g_ClassIdMgr.GetName(pWindow->GetClass()));
	}

	RECT rect;
	pWindow->GetWindowRect(&rect);

	if(pWindow->GetCurFocusState() == BsUiFS_SELECTED)
	{
		BsUiAttrId nDisAttrID = pWindow->GetFontAttr(BsUiFS_DISABLE);
		DrawText(rect, pWindow->GetItemPos(),
			g_FontAttrMgr.GetAttrText(nDisAttrID),
			szCurText, clr.a,
			BsUiFS_DISABLE);
	}

	DrawText(rect, pWindow->GetItemPos(),
		g_FontAttrMgr.GetAttrText(nAttrID),
		szCurText, clr.a,
		pWindow->GetCurFocusState());
}

void BsUiGDIView::DrawWindow(BsUiWindow* pWindow)
{
	POINT pos = pWindow->GetWindowPos();
	POINT size = pWindow->GetWindowSize();

	D3DXCOLOR clr = pWindow->GetColor();
	if(pWindow->GetParentWindow() != NULL){
		clr.a *= pWindow->GetParentWindow()->GetColor().a;
	}

	DrawWindow(pWindow->GetType(), pos, size,
		pWindow->GetImageMode(),
		pWindow->GetCurImageUVID(),
		pWindow->GetCurImageBlockID(),
		clr);
}


void BsUiGDIView::DrawWindow(BsUiTYPE nType, RECT rect, int nImageMode, int nUVID, int nBlockID, D3DXCOLOR clr)
{
	POINT pos, size;
	pos.x = rect.left;
	pos.y = rect.top;
	size.x = rect.right - rect.left;
	size.y = rect.bottom - rect.top;
	
	DrawWindow(nType, pos, size, nImageMode, nUVID, nBlockID, clr);
}

void BsUiGDIView::DrawWindow(BsUiTYPE nType, POINT pos, POINT size, int nImageMode, int nUVID, int nBlockID, D3DXCOLOR clr)
{
	TransPosSizeforScrollnZoom(pos, size);

	switch(nImageMode)
	{
	case _IMAGE_MODE_NONE: break;
	case _IMAGE_MODE_DEFAULT:
		{
			switch(nType)
			{
			case BsUiTYPE_LAYER:
			case BsUiTYPE_BUTTON:
			case BsUiTYPE_LISTBOX:
			case BsUiTYPE_LISTCTRL:
			case BsUiTYPE_SLIDER:
				{
					CreateSetButtonBox_s(pos.x, pos.y, size.x, size.y, 1);
					break;
				}
			case BsUiTYPE_TEXT:
			case BsUiTYPE_IMAGECTRL:
				{
					CreateSetLineBox_s(pos.x, pos.y, size.x, size.y, 1);
					break;
				}
			}
			break;
		}
	case _IMAGE_MODE_UV:
		{
			BsUiUVId nUVId = nUVID;
			if(nUVId != -1)
			{	
#ifdef _BSUI_TOOL
				UVImage* Img = g_EditUVMgr.GetUVImage(nUVId);
#else
				UVImage* Img = g_UVMgr.GetUVImage(nUVId);
#endif
				if(Img != NULL)
				{
					CBsKernel::GetInstance().DrawUIBox_s(_Ui_Mode_Image,
						pos.x, pos.y,
						size.x, size.y,
						0.f, clr,
						0.f,
						Img->nTexId,
						Img->u1, Img->v1,
						Img->u2, Img->v2);
				}
			}
			break;
		}
	case _IMAGE_MODE_BLOCK:
		{
			nBlockID;
		}
		break;
	}
}


void BsUiGDIView::DrawText(BsUiWindow* pWindow)
{
	POINT pos = pWindow->GetWindowPos();
	POINT size = pWindow->GetWindowSize();

	POINT ptTextPos = pWindow->GetItemPos();
	pos.x += ptTextPos.x;
	pos.y += ptTextPos.y;
	size.x -= ptTextPos.x;
	size.y -= ptTextPos.y;

	char szText[512];
	if(pWindow->GetItemText() != NULL)
	{
		strcpy(szText, pWindow->GetItemText());	
	}
	else
	{
		return;
		strcpy(szText, g_ClassIdMgr.GetName(pWindow->GetClass()));
	}
	
	float fAlpha = pWindow->GetColor().a;
	fAlpha *= pWindow->GetParentWindow()->GetColor().a;

	if(pWindow->GetCurFocusState() == BsUiFS_SELECTED)
	{
		BsUiAttrId nDisAttrID = pWindow->GetFontAttr(BsUiFS_DISABLE);
		DrawText(pos, size,
			g_FontAttrMgr.GetAttrText(nDisAttrID),
			szText, fAlpha,
			BsUiFS_DISABLE);
	}

	BsUiAttrId nAttrID = pWindow->GetCurFontAttr();
	DrawText(pos, size,
		g_FontAttrMgr.GetAttrText(nAttrID),
		szText, fAlpha,
		pWindow->GetCurFocusState());
}


void BsUiGDIView::DrawText(RECT rect, POINT ptTextPos, char* szFontAttr, char* szText, float fAlpha, BsUiFocusState focusState)
{
	POINT pos, size;
	pos.x = rect.left + ptTextPos.x;
	pos.y = rect.top + ptTextPos.y;
	size.x = rect.right - rect.left + ptTextPos.x;
	size.y = rect.bottom - rect.top + ptTextPos.y;

	DrawText(pos, size, szFontAttr, szText, fAlpha, focusState);
}


void BsUiGDIView::DrawText(POINT pos, POINT size, char* szFontAttr, char* szText, float fAlpha, BsUiFocusState focusState)
{
	if(szText == NULL){
		return;
	}

	TransPosSizeforScrollnZoom(pos, size);

	char sztemp[512]="";
	if(szFontAttr != NULL){
		TransFontAttr(sztemp, _countof(sztemp), szFontAttr);
	}

	strcat(sztemp, szText);

#ifndef _BSUI_TOOL
	if(focusState == BsUiFS_SELECTED)
	{
		int nTick = g_BsUiSystem.GetMenuTick() % 40;
		if(nTick < 10){
			fAlpha = fAlpha * nTick * 0.1f;
		}
		else if(nTick >= 30){
			fAlpha = fAlpha * (40 - nTick) * 0.1f;
		}
	}
#endif
	g_pFont->DrawUIText(pos.x, pos.y, pos.x+size.x, pos.y+size.y, sztemp, fAlpha);
}

void BsUiGDIView::TransPosSizeforScrollnZoom(POINT& pos, POINT& size)
{
#ifdef _BSUI_TOOL
	POINT ptAddPos = GetScrollPosforPage();
	int zoom = GetZoomforPage();

	pos.x = (pos.x * zoom) - ptAddPos.x;
	pos.y = (pos.y * zoom) - ptAddPos.y;
	size.x *= zoom; size.y *= zoom;
#endif
}

void BsUiGDIView::TransRectforScrollnZoom(RECT& rect)
{
#ifdef _BSUI_TOOL
	POINT ptAddPos = GetScrollPosforPage();
	int zoom = GetZoomforPage();

	POINT pos, size;
	pos.x = rect.left; pos.y = rect.top;
	size.x = rect.right-rect.left; size.y = rect.bottom-rect.top;

	pos.x = (pos.x * zoom) - ptAddPos.x;
	pos.y = (pos.y * zoom) - ptAddPos.y;
	size.x *= zoom; size.y *= zoom;

	rect.left = pos.x;
	rect.top = pos.y;
	rect.right = pos.x + size.x;
	rect.bottom = pos.y + size.y;
#endif
}

void BsUiGDIView::TransFontAttr(char* szDesFontAttr, const size_t szDesFontAttr_len, char* szSrcFontAttr)
{
#ifdef _BSUI_TOOL
	SFontAttribute pFontAttr;
	g_pFont->GetFontAttrStruct(&pFontAttr, szSrcFontAttr);
	
	int zoom = GetZoomforPage();

	pFontAttr.fScaleY *= zoom;
	pFontAttr.fScaleX *= zoom;
	pFontAttr.nSpaceX *= zoom;
	pFontAttr.nSpaceY *= zoom;

	g_pFont->GetFontAttrText(szDesFontAttr, szDesFontAttr_len,  &pFontAttr);
#else
	strcpy_s(szDesFontAttr, szDesFontAttr_len, szSrcFontAttr); //aleksger - safe string
#endif
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#ifdef _BSUI_TOOL

void BsUiGDIView::InitKernel(HWND hwnd, int nWidth,int nHeight)
{
	static int re;
	if (!re)
	{	
#ifndef _XBOX
		CBsKernel::CreateInstance();
		CBsKernel::GetInstance().Initialize( hwnd , nWidth , nHeight , true);
		CBsKernel::GetInstance().EnableImageProcess(false);
		CBsKernel::GetInstance().CreateUIforTool();	// 반드시 호출 합니다 
#else
		CBsKernel::GetInstance().CreateUI(); // 반드시 호출 합니다 
#endif	
		
		CBsKernel::GetInstance().SetUIViewerIndex(_UI_FULL_VIEWER);

#ifdef _BSUI_TOOL
		g_BsKernel.chdir("data");
#endif
		g_pFont = NULL;
		g_pFont = new BsUiFont();

		g_ClassIdMgr.Load();
		g_FontAttrMgr.Load();
		g_EditUVMgr.Load();

		re = 1;
		return;
	}

	if( CBsKernel::GetInstance().IsActive())
	{
#ifndef _XBOX
		//TRACE("이미 커널이 있다\n");
		CBsKernel::GetInstance().ReInitialize( hwnd, nWidth , nHeight , true, true);
#endif
	}	
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool BsUiGDIView::DrawViewScale(POINT size)
{
	POINT pos;
	pos.x = 0; pos.y = 0;
	TransPosSizeforScrollnZoom(pos, size);

	D3DXCOLOR clr = D3DXCOLOR(0.4f, 0.4f, 0.4f, 1.f);
	CBsKernel::GetInstance().DrawUIBox_s(_Ui_Mode_Box, pos.x, pos.y, size.x, size.y, 0.f, clr);

	return true;
}

bool BsUiGDIView::DrawTargetWindow(void)
{
	BsUiWindow* pFocusWindow = BsUiSystem::GetFocusWindow();
	BsUiWindow* pGrabWindow = BsUiSystem::GetGrabWindow();
	BsUiWindow* pWindow = NULL;
	if(pGrabWindow != NULL)
	{
		pWindow = pGrabWindow;
	}
	else if(pFocusWindow != NULL)
	{
		pWindow = pFocusWindow;
	}
	else
	{
		return false;
	}

	RECT rect;
	pWindow->GetWindowRect(&rect);

	TransRectforScrollnZoom(rect);
	
	CreateSetLineBox(rect.left, rect.top, rect.right, rect.bottom, 1);

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
				
				pointRect.left -= 1;
				pointRect.right -= 1;
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
				
				pointRect.top -= 1;
				pointRect.bottom -= 1;
				break;
			}
		case _CHANGE_SIZE_LEFTTOP:
			{
				pointRect.left = rect.left;
				pointRect.top = rect.top; 
				pointRect.right = rect.left + 10;
				pointRect.bottom = rect.top + 10;
				break;
			}
		case _CHANGE_SIZE_LEFTBOTTOM: 
			{
				pointRect.left = rect.left;
				pointRect.top = rect.bottom - 10;
				pointRect.right = rect.left + 10;
				pointRect.bottom = rect.bottom;
				
				pointRect.top -= 1;
				pointRect.bottom -= 1;
				break;
			}
		case _CHANGE_SIZE_RIGHTTOP:
			{
				pointRect.left = rect.right - 10;
				pointRect.top = rect.top;
				pointRect.right = rect.right;
				pointRect.bottom = rect.top + 10;				
				
				pointRect.left -= 1;
				pointRect.right -= 1;
				break;
			}
		case _CHANGE_SIZE_RIGHTBOTTOM:
			{
				pointRect.left =  rect.right - 10;
				pointRect.top = rect.bottom - 10;
				pointRect.right = rect.right;
				pointRect.bottom = rect.bottom;
				
				pointRect.left -= 1;
				pointRect.top -= 1;
				pointRect.right -= 1;
				pointRect.bottom -= 1;
				break;
			}
		}

		CreateSetLineBox(pointRect.left, pointRect.top, pointRect.right, pointRect.bottom, 1);
	}

	return true;
}

void BsUiGDIView::DrawSelectedBox()
{
	BsUiGDISkin* pSkin = (BsUiGDISkin*)BsUiSystem::GetInstance().GetSkin();
	RECT rect = pSkin->GetSelectedRect();

	TransRectforScrollnZoom(rect);
	
	if(rect.left != 0 || rect.right != 0 || rect.top != 0 || rect.bottom != 0)
	{
		int sx = (rect.left <= rect.right) ? rect.left : rect.right;
		int sy = (rect.top <= rect.bottom) ? rect.top : rect.bottom;

		int ex = (rect.left < rect.right) ? rect.right : rect.left;
		int ey = (rect.top < rect.bottom) ? rect.bottom : rect.top;

		CreateSetLineBox(sx, sy, ex, ey, 1);
	}

	BsUiWINDOWLIST* pSelectedWindows = BsUiSystem::GetInstance().GetSelectedWindows();
	BsUiWINDOWLISTITOR itr = pSelectedWindows->begin();
	while (itr != pSelectedWindows->end())
	{
		BsUiWindow* pWindow = (*itr);
		BsAssert(pWindow);

		RECT rect;
		pWindow->GetWindowRect(&rect);

		TransRectforScrollnZoom(rect);

		CreateSetLineBox(rect.left, rect.top, rect.right-1, rect.bottom-1, 1);

		++itr;
	}

}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool BsUiGDIView::DrawEditingFont(void)
{
	POINT size;
	size.x = 800; size.y = 600;
	DrawViewScale(size);

	char szText[256];
	memset(szText, 0, 256);
	sprintf(szText, "%sFont Attribute Viewer :", _TEXT_DEFAULT_ATTR);
	g_pFont->DrawUIText(50, 50, -1, -1, szText);

	char* pszAttr = g_FontAttrMgr.GetAttrText(g_FontAttrMgr.GetCurAttrId());
	sprintf(szText, "%sABCDEF \n abcdef", pszAttr);
	g_pFont->DrawUIText(70, 100, -1, -1, szText);

	return true;
}


bool BsUiGDIView::DrawEditingTexture(void)
{
	int nTextureId = g_EditUVMgr.GetCurTextureId();
	if(nTextureId == -1)
		return false;
	
	SIZE Texsize = CBsKernel::GetInstance().GetTextureSize(nTextureId);

	POINT size;
	size.x = Texsize.cx; size.y = Texsize.cy;
	DrawViewScale(size);

	RECT rect;
	rect.left = 0; rect.top = 0;
	rect.right = Texsize.cx; rect.bottom = Texsize.cy;

	TransRectforScrollnZoom(rect);
	
	D3DXCOLOR clr = D3DXCOLOR(1, 1, 1, 1);
	g_BsKernel.DrawUIBox(_Ui_Mode_Image,
		rect.left, rect.top, rect.right, rect.bottom,
		0.f, clr,
		0.f,
		nTextureId,
		0, 0, Texsize.cx, Texsize.cy);
		
	DrawAllUVBoxnID();
	DrawSelUVBoxnID();
	
	if(g_EditUVMgr.GetEditUVState() != _EditUV_SEL)
		DrawSelectedBoxforUV();	

	return true;
}

bool BsUiGDIView::DrawAllUVBoxnID()
{
	int nCurUVId = g_EditUVMgr.GetCurUVId();

	if(g_EditUVMgr.IsShowBlock())
	{
		D3DXCOLOR clr = D3DXCOLOR(0.19f, 0.3f, 0.3f, 1.f);

		BsUiUVLIST* pUVList = g_EditUVMgr.GetUVList(g_EditUVMgr.GetCurTextureId());
		if(pUVList != NULL)
		{
			for (DWORD i=0; i<pUVList->size(); ++i)
			{
				UVImage Img = (*pUVList)[i];
				
				if(nCurUVId == Img.nId)
					continue;

				RECT rect;
				rect.left = Img.u1; rect.top = Img.v1;
				rect.right = Img.u2; rect.bottom = Img.v2;
				TransRectforScrollnZoom(rect);

				CreateSetLineBox(rect.left, rect.top, rect.right, rect.bottom, 1);
			}
		}
	}

	if(g_EditUVMgr.IsShowID())
	{
		BsUiUVLIST* pUVList = g_EditUVMgr.GetUVList(g_EditUVMgr.GetCurTextureId());
		if(pUVList != NULL)
		{
			for (DWORD i=0; i<pUVList->size(); ++i)
			{
				UVImage Img = (*pUVList)[i];

				if(nCurUVId == Img.nId)
					continue;

				char szId[256];
				sprintf(szId, "@(color=255,255,255,255)@(scale=0.6,0.6)%d", Img.nId);

				RECT rect;
				rect.left = Img.u1; rect.top = Img.v1;
				rect.right = Img.u2; rect.bottom = Img.v2;
				TransRectforScrollnZoom(rect);

				g_pFont->DrawUIText(rect.left, rect.top, -1, -1, szId);
			}
		}
	}

	return true;
}

bool BsUiGDIView::DrawSelUVBoxnID()
{
	int nCurUVId = g_EditUVMgr.GetCurUVId();
	
	if(nCurUVId != -1)
	{
		UVImage* Img = g_EditUVMgr.GetUVImage(nCurUVId);

		RECT rect;
		rect.left = Img->u1; rect.top = Img->v1;
		rect.right = Img->u2; rect.bottom = Img->v2;
		TransRectforScrollnZoom(rect);
        
		D3DXCOLOR clr = D3DXCOLOR(0.f, 1.f, 0.5f, 1.f);
		CreateSetLineBox(rect.left, rect.top, rect.right, rect.bottom, 1);

		if(g_EditUVMgr.IsShowID())
		{
			char szId[256];
			sprintf(szId, "@(color=139,197,197,255)@(scale=0.6,0.6)%d", Img->nId);
			g_pFont->DrawUIText(rect.left-1, rect.top-1, -1, -1, szId);
		}
	}

	return true;
}


bool BsUiGDIView::DrawSelectedBoxforUV()
{
	RECT rect = g_EditUVMgr.GetSelectedBox();

	TransRectforScrollnZoom(rect);

	if(rect.left != 0 || rect.right != 0 || rect.top != 0 || rect.bottom != 0)
	{
		int sx = (rect.left <= rect.right) ? rect.left : rect.right;
		int sy = (rect.top <= rect.bottom) ? rect.top : rect.bottom;

		int ex = (rect.left < rect.right) ? rect.right : rect.left;
		int ey = (rect.top < rect.bottom) ? rect.bottom : rect.top;

		D3DXCOLOR clr = D3DXCOLOR(1, 1, 1, 1);
		CreateSetLineBox(sx, sy, ex, ey, 1);
	}

	return true;
}

POINT BsUiGDIView::GetScrollPosforPage()
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CBsUiToolView* pView = (CBsUiToolView*)pFrame->GetActiveView();
	
	return pView->GetScrollPosition();
}

int	BsUiGDIView::GetZoomforPage()
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CBsUiToolDoc* pDoc = (CBsUiToolDoc*)pFrame->GetActiveDocument();
	CBsUiToolView* pView = (CBsUiToolView*)pFrame->GetActiveView();

	return pView->GetZoomValue();
}
#endif


void BsUiGDIView::DrawImage(int nUVId, POINT pos, POINT size, float fScaleX, float fScaleY, D3DXCOLOR clr)
{
	POINT ptCenter;
	ptCenter.x = pos.x + (size.x/2);
	ptCenter.y = pos.y + (size.y/2);
	DrawImage(nUVId, ptCenter, fScaleX, fScaleY, clr);
}

void BsUiGDIView::DrawImage(int nUVId, RECT rect, float fScaleX, float fScaleY, D3DXCOLOR clr)
{
	POINT ptCenter;
	ptCenter.x = rect.left + (rect.right - rect.left)/2;
	ptCenter.y = rect.top + (rect.bottom - rect.top)/2;
	DrawImage(nUVId, ptCenter, fScaleX, fScaleY, clr);
}

void BsUiGDIView::DrawImage(int nUVId, POINT ptCenter, float fScaleX, float fScaleY, D3DXCOLOR clr)
{
	float m_fAlpha = 1.f;

#ifdef _BSUI_TOOL
	UVImage* Img = g_EditUVMgr.GetUVImage(nUVId);
#else
	UVImage* Img = g_UVMgr.GetUVImage(nUVId);
#endif
	if(Img == NULL)
		return;

	POINT size;
	size.x = (int)((Img->u2 - Img->u1) * fScaleX);
	size.y = (int)((Img->v2 - Img->v1) * fScaleY);

	POINT pos;
	pos.x = ptCenter.x - (size.x / 2);
	pos.y = ptCenter.y - (size.y / 2);

	if( clr.a > m_fAlpha)
	{
		clr.a = m_fAlpha;
	}

	g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
		pos.x, pos.y,
		size.x, size.y,
		0.f, clr,
		0.f,
		Img->nTexId,
		Img->u1, Img->v1,
		Img->u2, Img->v2);
}

void BsUiGDIView::DrawImage(int nUVId, POINT pos, POINT size, D3DXCOLOR clr)
{
	float m_fAlpha = 1.f;

#ifdef _BSUI_TOOL
	UVImage* Img = g_EditUVMgr.GetUVImage(nUVId);
#else
	UVImage* Img = g_UVMgr.GetUVImage(nUVId);
#endif
	if(Img == NULL)
		return;

	if( clr.a > m_fAlpha)
	{
		clr.a = m_fAlpha;
	}

	g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
		pos.x, pos.y,
		size.x, size.y,
		0.f, clr,
		0.f,
		Img->nTexId,
		Img->u1, Img->v1,
		Img->u2, Img->v2);
}

void BsUiGDIView::DrawImage(int nUVId, RECT rect, D3DXCOLOR clr)
{
	POINT pos, size;
	pos.x = rect.left;
	pos.y = rect.top;
	size.x = rect.right - rect.left;
	size.y = rect.bottom - rect.top;
	
	DrawImage(nUVId, pos, size, clr);
}


void BsUiGDIView::CreateSetButtonBox_s(int x1, int y1, int sx, int sy, int nThick)
{
	CreateSetButtonBox(x1, y1, x1+sx, y1+sy, nThick);
}

void BsUiGDIView::CreateSetButtonBox(int x1, int y1, int x2, int y2, int nThick)
{
	g_BsKernel.DrawUIBox(_Ui_Mode_Box, x1+nThick, y1+nThick, x2-nThick, y2-nThick, 0.f, m_ColorFill);

	g_BsKernel.DrawUIBox(_Ui_Mode_Box, x1, y1, x2, y1+nThick, 0.f, m_ColorWhite);	// 위쪽 수평
	g_BsKernel.DrawUIBox(_Ui_Mode_Box, x1, y2-nThick, x2, y2, 0.f, m_ColorBlack);	// 아래쪽 수평
	g_BsKernel.DrawUIBox(_Ui_Mode_Box, x1, y1+nThick, x1+nThick, y2-nThick, 0.f, m_ColorWhite);	// 왼쪽
	g_BsKernel.DrawUIBox(_Ui_Mode_Box, x2-nThick, y1+nThick, x2, y2-nThick, 0.f, m_ColorBlack);	// 오른쪽
}

void BsUiGDIView::SetButtonBoxColor(D3DXCOLOR f, D3DXCOLOR w, D3DXCOLOR b)
{ 
	m_ColorFill=f;
	m_ColorWhite=w;
	m_ColorBlack=b;
}

void BsUiGDIView::CreateSetLineBox_s(int x1, int y1, int sx, int sy, int nThick)
{
	CreateSetLineBox(x1, y1, x1+sx, y1+sy, nThick);
}


void BsUiGDIView::CreateSetLineBox(int x1, int y1, int x2, int y2, int nThick)
{
	g_BsKernel.DrawUIBox(_Ui_Mode_Box, x1, y1, x2, y1+nThick, 0.f, m_ColorBox);	// 위쪽 수평
	g_BsKernel.DrawUIBox(_Ui_Mode_Box, x1, y2-nThick, x2, y2, 0.f, m_ColorBox);	// 아래쪽 수평
	g_BsKernel.DrawUIBox(_Ui_Mode_Box, x1, y1+nThick, x1+nThick, y2-nThick, 0.f, m_ColorBox);	// 왼쪽
	g_BsKernel.DrawUIBox(_Ui_Mode_Box, x2-nThick, y1+nThick, x2, y2-nThick, 0.f, m_ColorBox);	// 오른쪽
}

void BsUiGDIView::SetLineBoxColor(D3DXCOLOR b)
{ 
	m_ColorBox = b;
}