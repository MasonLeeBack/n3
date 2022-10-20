#pragma once

// BsUiSlider.h: 슬라이더

// jazzcake@hotmail.com
// 2004.0728

#include "BSuiWindow.h"

enum BsUiSlider_TYPE
{
	BsUiSDTYPE_NORMAL,
	BsUiSDTYPE_NUMERAL,
	BsUiSDTYPE_TEXT,
	BsUiSDTYPE_GAUGE,
};

enum BsUiSlider_Orientation
{
	BsUiSDO_HORIZONAL,
	BsUiSDO_VERTICAL,
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class BsUiSlider : public BsUiWindow {
protected:
	int						m_sdType;					// normal, numeral, Text
	int						m_sdOrientation;			// 수평? 수직?
	POINT					m_ptBarSize;				// Bar의 크기

	int						m_nMinRange;
	int						m_nMaxRange;				// 최대/최소치
	int						m_nTicCount;				// 최대 - 최소
	int						m_nTicIndex;				// tic index

	//normal type
	int						m_nBarUVID[BsUiFS_COUNT];
	int						m_nLineUV;					// slider bar UV
	int						m_nBaseUV;

	//numeral & text type
	vector <BsUiItemInfo*>	m_Items;					// text type에만 사용
	int						m_nBar1UVID[BsUiFS_COUNT];
	int						m_nBar2UVID[BsUiFS_COUNT];
	
protected:
	bool					m_bMovingTic;				// tic 이동 중?
	POINT					m_ptMovingTicGap;			// tic move에서 적용할 tick과 마우스의 위치 차이
	int						m_nCurOffset;				// slider 내에서의 위치

	bool					m_bBarPosBreak;

public:
	BsUiSlider();
	BsUiSlider(BsUiCLASS hClass, POINT pos, BsUiWindow* pParent);
	~BsUiSlider()	{ Release(); }

	bool			Create(BsUiCLASS hClass, int x, int y, BsUiWindow* pParent);
	void			Release();

	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);

public:
	int				SetRange(int nMin, int nMax);
	int				SetBarPos(int n);
	int				GetBarPos()							{ return (m_nTicIndex + m_nMinRange); }
	int				AddBarPos(bool bAdd);

	int				GetSDType()							{ return m_sdType; }
	void			SetSDType(int nType)				{ m_sdType = nType; }
	int				GetOrientation()					{ return m_sdOrientation; }
	void			SetOrientation(int nOrientation)	{ m_sdOrientation = nOrientation; }
	POINT			GetBarSize()						{ return m_ptBarSize; }
	void			SetBarSize(POINT ptSize)			{ m_ptBarSize = ptSize; }
	int				GetMinRange()						{ return m_nMinRange; }
	void			SetMinRange(int nRange)				{ SetRange(nRange, m_nMaxRange); }
	int				GetMaxRange()						{ return m_nMaxRange; }
	void			SetMaxRange(int nRange)				{ SetRange(m_nMinRange, nRange); }
	char*			GetCurText();

	bool			IsHitTic(int x, int y);
	void			GetTicPos(POINT& pt);
	int				GetTicIndex(int x, int y);

	void			SetBarPosBreak(bool bBreak)			{ m_bBarPosBreak = bBreak; }
	bool			GetBarPosBreak()					{ return m_bBarPosBreak; }


	int				GetBaseUVID()										{ return m_nBaseUV; }
	void			SetBaseUVID(int nUVID)								{ m_nBaseUV = nUVID; }

	
	//normal type
	void			GetLineRect(RECT* rect);
	void			GetBarRect(RECT* rect);
	int				GetCurBarUVID();
	int				GetBarUVID(BsUiFocusState nState)					{ return m_nBarUVID[nState]; }
	int				GetLineUVID()										{ return m_nLineUV; }
	void			SetBarUVID(BsUiFocusState nState, int nUVID)		{ m_nBarUVID[nState] = nUVID; }
	void			SetLineUVID(int nUVID)								{ m_nLineUV = nUVID; }
		
	//Numeral & Text type
	void			GetBar1Rect(RECT* rect);
	void			GetBar2Rect(RECT* rect);
	int				GetCurBar1UVID();
	int				GetCurBar2UVID();
	int				GetBar1UVID(BsUiFocusState nState)					{ return m_nBar1UVID[nState]; }
	int				GetBar2UVID(BsUiFocusState nState)					{ return m_nBar2UVID[nState]; }
	void			SetBar1UVID(BsUiFocusState nState, int nUVID)		{ m_nBar1UVID[nState] = nUVID; }
	void			SetBar2UVID(BsUiFocusState nState, int nUVID)		{ m_nBar2UVID[nState] = nUVID; }

	// text type
	BsUiItemInfo*	GetCurItem();
	int				AddItem(char* szText, DWORD dwData);
	int				AddItem(int nTextID, DWORD dwData);
	int				RemoveItem(int nIndex);
	void			ClearItem();

	//gauge type
	void			GetGaugeBarRect(RECT* rect);

};

