#pragma once

#include "BsUiView.h"
#include "CrossVector.h"

class BsUiWindow;
class BsUiLayer;
class BsUiButton;
class BsUiListBox;
class BsUiListCtrl;
class BsUiSlider;
class BsUiImageCtrl;

class BsUiText;
typedef struct _BsUiImageInfo
{
	BsUiHANDLE			hWnd;
	int					nIndex;
	int					nTextId;
} BsUiImageInfo;

class BsUiGDIView : public BsUiView {
public:
	BsUiGDIView();
	~BsUiGDIView();

	virtual void	Draw(BsUiWindow* pWindow);
	virtual bool	GetText(int nTextId, char* szText, const size_t szText_len); //aleksger - safe string length

protected:
	void		DrawUiLayer(BsUiLayer* pWindow);
	void		DrawUiButton(BsUiButton* pWindow);
	void		DrawUiListBox(BsUiListBox* pWindow);
	void		DrawUiListCtrl(BsUiListCtrl* pWindow);
	void		DrawUiText(BsUiText* pWindow);
	void		DrawUiImageCtrl(BsUiImageCtrl* pWindow);
	void		DrawUiSlider(BsUiSlider* pWindow);
	void		DrawUiSliderNormal(BsUiSlider* pWindow);
	void		DrawUiSliderNumeral(BsUiSlider* pWindow);
	void		DrawUiSliderText(BsUiSlider* pWindow);
	void		DrawUiSliderGauge(BsUiSlider* pWindow);

	void		DrawWindow(BsUiWindow* pWindow);
	void		DrawWindow(BsUiTYPE nType, RECT rect, int nImageMode, int nUVID, int nBlockID, D3DXCOLOR clr);
	void		DrawWindow(BsUiTYPE nType, POINT pos, POINT size, int nImageMode, int nUVID, int nBlockID, D3DXCOLOR clr);
	

	void		DrawText(BsUiWindow* pWindow);
	void		DrawText(RECT rect, POINT ptTextPos, char* szFontAttr, char* szText, float fAlpha, BsUiFocusState focusState);
	void		DrawText(POINT pos, POINT size, char* szFontAttr, char* szText, float fAlpha, BsUiFocusState focusState);
	
	void		TransFontAttr(char* szDesFontAttr, const size_t szDesFontAttr_len, char* szSrcFontAttr); //aleksger - safe string

	void		TransPosSizeforScrollnZoom(POINT& pos, POINT& size);
	void		TransRectforScrollnZoom(RECT& rect);
	
	//size에 의해 scale 변화
	void		DrawImage(int nUVId, POINT pos, POINT size, D3DXCOLOR clr=D3DXCOLOR(1.f,1.f,1.f,1.f));
	void		DrawImage(int nUVId, RECT rect, D3DXCOLOR clr=D3DXCOLOR(1.f,1.f,1.f,1.f));

	//scale에 의해 변화
	void		DrawImage(int nUVId, POINT pos, POINT size, float fScaleX, float fScaleY, D3DXCOLOR clr=D3DXCOLOR(1.f,1.f,1.f,1.f));
	void		DrawImage(int nUVId, RECT rect, float fScaleX, float fScaleY, D3DXCOLOR clr=D3DXCOLOR(1.f,1.f,1.f,1.f));
	void		DrawImage(int nUVId, POINT ptCenter, float fScaleX, float fScaleY, D3DXCOLOR clr=D3DXCOLOR(1.f,1.f,1.f,1.f));

protected:
	D3DXCOLOR m_ColorFill; 
	D3DXCOLOR m_ColorWhite; 
	D3DXCOLOR m_ColorBlack;
	D3DXCOLOR m_ColorBox; 
	
public:
	void	CreateSetButtonBox(int x1, int y1, int x2, int y2, int nThick); // 버튼 형태의 박스
	void	CreateSetButtonBox_s(int x1, int y1, int sx, int sy, int nThick);
	void	SetButtonBoxColor(D3DXCOLOR f, D3DXCOLOR w, D3DXCOLOR b);
	
	void	CreateSetLineBox(int x1, int y1, int x2, int y2, int nThick); //line 형태의 박스
	void	CreateSetLineBox_s(int x1, int y1, int sx, int sy, int nThick);
	void	SetLineBoxColor(D3DXCOLOR f);

//--------------------------------------------------------------------
//ui tool
#ifdef _BSUI_TOOL
public:
	void			InitKernel(HWND hwnd, int nWidth,int nHeight);

	bool			DrawViewScale(POINT size);
	bool			DrawTargetWindow(void);
	void			DrawSelectedBox();

	bool			DrawEditingFont(void);

	bool			DrawEditingTexture(void);
protected:
	POINT			GetScrollPosforPage();
	int				GetZoomforPage();

	bool			DrawAllUVBoxnID();
	bool			DrawSelUVBoxnID();
	bool			DrawSelectedBoxforUV();

protected:
	CCrossVector		m_Cross;

	std::vector<BsUiImageInfo*> m_ImageInfoList;

	typedef struct _BsUiGroupedInfo {
		BsUiHANDLE			hWnd;
		int					nBoxIndex;
	} BsUiGroupedInfo;

	std::vector<BsUiGroupedInfo>	m_GroupedInfo;
	int								m_nDragBoxIndex;
#endif
};