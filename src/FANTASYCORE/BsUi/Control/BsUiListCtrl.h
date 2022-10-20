#ifndef _BsUi_ListCtrl_H_
#define _BsUi_ListCtrl_H_

#include "BSuiWindow.h"

// ListCtrl Type
#define BsUiLBTYPE_NORMAL		0					// 일반
#define BsUiLBTYPE_FIXSELPOS	1					// 선택칸 고정
#define BsUiLBTYPE_ICON			2					// icon 방식

typedef struct _BsUiListItem {
	_BsUiListItem(){
		nItem = -1;
		nSubItem = -1;
	};

	int				nItem;
	int				nSubItem;
	BsUiItemInfo	Item;
} BsUiListItem;

typedef	vector <BsUiListItem*>		BsUiListITEM;


class BsUiListCtrl : public BsUiWindow {
protected:
	int							m_nLbType;			// 어떤 타입인가.
	int							m_nItemWidth;		// 하나의 item의 폭
	int							m_nItemHeight;		// 하나의 item의 높이
	int							m_nCols;			// 컬럼 수 (BsUiLBTYPE_ICON에서만 1이상)
	int							m_nLines;			// 한 화면에서 보여지는 라인 수
	
	vector <BsUiItemInfo*>		m_Items;			// 선택항목들
	BsUiListITEM				m_lItems;

	int							m_nTopIndex;		// 보여지는 것중에 제일 앞에 있는 아이템 인덱스
	int							m_nCurSel;			// 현재 선택항목
	bool						m_bDrawSelBox;

public:
	BsUiListCtrl();
	BsUiListCtrl(BsUiCLASS hClass, POINT pos, BsUiWindow* pParent);
	~BsUiListCtrl();

	bool			Create(BsUiCLASS hClass, int x, int y, BsUiWindow* pParent);
	void			Release();

	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);

public:
	char*			GetString(int index) { return m_Items[index]->szText; }

	int				GetCurSel() { return m_nCurSel; }
	int				SetCurSel(int nIndex);
	
	int				GetLbType()				{ return m_nLbType; }
	void			SetLbType(int nLbType)	{ m_nLbType = nLbType; }
	int				GetColumns()			{ return m_nCols; }
	void			SetColumns(int nCols);
	int				GetLines()				{ return m_nLines; }
	void			SetLines(int nLines);

	void			SetItemData(int nIndex, DWORD dwData);
	DWORD			GetItemData(int nIndex);
	int				GetItemCount() { return (int)m_Items.size(); }

	void			ClearItem();
	int				AddString(const char* pStr);
	int				DeleteString(int nIndex);
	int				FindString(int nStartAfter, const char* pStr);
	int				SelectString(int nStartAfter, const char* pStr);
	
	bool			AddItem(char* pStr, int nItem, int nSubItem = 0);
	bool			AddSubItem(int nTextId, int nItem, int nSubItem);
	bool			DeleteItem(int nItem);
	bool			DeleteItem(int nItem, int nSubItem);
	bool			ChangeItem(int nItem, int nSubItem, char* pStr);
	void			SetItemData(int nItem, int nSubItem, DWORD dwData);
	

public:
	int				GetCountPerPage();
	int				GetTopIndex() { return m_nTopIndex; }
	int				SetTopIndex( int nIndex );
	int				HitTest(POINT* pPt);	// pPt는 화면좌표
	void			GetItemRect(int nIndex, RECT& rect);
	BsUiAttrId		GetCurFontAttr(int nIndex);
	bool			IsDrawSelBox()						{ return m_bDrawSelBox; }
	void			SetDrawSelBox(bool bDrawSelBox)		{ m_bDrawSelBox = bDrawSelBox; }
	virtual	void	ResizeWindowSize(ChangeWindowSize nFlag, POINT pos);

protected:
	void			UpdateTopIndex();
};

#endif

