#ifndef _BsUi_LISTBOX_H_
#define _BsUi_LISTBOX_H_

#include "BSuiWindow.h"

class BsUiListBox : public BsUiWindow {
protected:
	
	int			m_nItemWidth;		// 하나의 item의 폭
	int			m_nItemHeight;		// 하나의 item의 높이
	int			m_nLines;			// 한 화면에서 보여지는 라인 수
	
	vector <BsUiItemInfo*>		m_Items;			// 선택항목들
	

	int			m_nTopIndex;		// 보여지는 것중에 제일 앞에 있는 아이템 인덱스
	int			m_nCurSel;			// 현재 선택항목
	bool		m_bDrawSelBox;

	int			m_nSelBoxUVID[BsUiFS_COUNT];
	int			m_nSelBoxBlockID[BsUiFS_COUNT];

public:
	BsUiListBox();
	BsUiListBox(BsUiCLASS hClass, POINT pos, BsUiWindow* pParent);
	~BsUiListBox();

	bool			Create(BsUiCLASS hClass, int x, int y, BsUiWindow* pParent);
	void			Release();

	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);

public:
	char*			GetString(int index) { return m_Items[index]->szText; }

	int				GetCurSel() { return m_nCurSel; }
	int				SetCurSel(int nIndex);
		
	int				GetLines()				{ return m_nLines; }
	void			SetLines(int nLines);

	int				AddItem(char* szText, DWORD dwData);
	int				AddItem(int nTextID, DWORD dwData);
	int				RemoveItem(int nIndex);
	void			ClearItem();

	int				FindItem(int nStartAfter, const char* pStr);
	int				FindItem(int nStartAfter, int nTextID);
	int				FindItem(int nStartAfter, DWORD dwData);

	int				SelectItem(int nStartAfter, const char* pStr);
	int				SelectItem(int nStartAfter, int nTextID);
	int				SelectItem(int nStartAfter, DWORD dwData);

	bool			SetItemText(int nIndex, const char* szText);
	char*			GetItemText(int nIndex);

	bool			SetItemTextId(int nIndex, int TextId);
	int				GetItemTextId(int nIndex);

	bool			SetItemData(int nIndex, DWORD dwData);
	DWORD			GetItemData(int nIndex);

	int				GetItemCount() { return (int)m_Items.size(); }
			
public:
	int				GetPageCount();
	int				GetLastPage();
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

public:
	void			SetSelBoxUVID(BsUiFocusState nState, int nUVID)		{ m_nSelBoxUVID[nState] = nUVID; }
	void			SetSelBoxBlock(BsUiFocusState nState, int nBlock)	{ m_nSelBoxBlockID[nState] = nBlock; }
	
	int				GetSelBoxUVID(BsUiFocusState nState)				{ return m_nSelBoxUVID[nState]; }
	int				GetSelBoxBlock(BsUiFocusState nState)				{ return m_nSelBoxBlockID[nState]; }

	int				GetCurSelBoxUVID();
	int				GetCurSelBoxBlockID();
};

#endif

