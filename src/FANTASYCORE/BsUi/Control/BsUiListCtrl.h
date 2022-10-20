#ifndef _BsUi_ListCtrl_H_
#define _BsUi_ListCtrl_H_

#include "BSuiWindow.h"

// ListCtrl Type
#define BsUiLBTYPE_NORMAL		0					// �Ϲ�
#define BsUiLBTYPE_FIXSELPOS	1					// ����ĭ ����
#define BsUiLBTYPE_ICON			2					// icon ���

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
	int							m_nLbType;			// � Ÿ���ΰ�.
	int							m_nItemWidth;		// �ϳ��� item�� ��
	int							m_nItemHeight;		// �ϳ��� item�� ����
	int							m_nCols;			// �÷� �� (BsUiLBTYPE_ICON������ 1�̻�)
	int							m_nLines;			// �� ȭ�鿡�� �������� ���� ��
	
	vector <BsUiItemInfo*>		m_Items;			// �����׸��
	BsUiListITEM				m_lItems;

	int							m_nTopIndex;		// �������� ���߿� ���� �տ� �ִ� ������ �ε���
	int							m_nCurSel;			// ���� �����׸�
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
	int				HitTest(POINT* pPt);	// pPt�� ȭ����ǥ
	void			GetItemRect(int nIndex, RECT& rect);
	BsUiAttrId		GetCurFontAttr(int nIndex);
	bool			IsDrawSelBox()						{ return m_bDrawSelBox; }
	void			SetDrawSelBox(bool bDrawSelBox)		{ m_bDrawSelBox = bDrawSelBox; }
	virtual	void	ResizeWindowSize(ChangeWindowSize nFlag, POINT pos);

protected:
	void			UpdateTopIndex();
};

#endif

