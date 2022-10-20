#ifndef _BsUi_BUTTON_H_
#define _BsUi_BUTTON_H_

#include "BSuiWindow.h"

enum BsUiBUTTONMODE {
	BsUiBUTTON_NORMAL,
	BsUiBUTTON_CHECKBUTTON,
};

#define TEXT_BsUiBUTTON_NORMAL		"Normal mode"
#define TEXT_BsUiBUTTON_CHECKBUTTON "Check button mode"

// 버튼 클래스
class BsUiButton : public BsUiWindow {
protected:
	BsUiBUTTONMODE	m_nButtonMode;
	bool			m_bButtonDown;
	int				m_nGroupID;

public:
	BsUiButton();
	BsUiButton(BsUiCLASS hClass, POINT pos, BsUiWindow* pParent);
	virtual ~BsUiButton() { Release(); }

	bool		Create(BsUiCLASS hClass, int x, int y, BsUiWindow* pParent);
	void		Release();

	virtual DWORD		ProcMessage(xwMessageToken* pMsgToken);

	BsUiBUTTONMODE		GetMode()						{ return m_nButtonMode; }
	void				SetMode(BsUiBUTTONMODE mode)	{ m_nButtonMode = mode; }
	bool				GetCheck()						{ return m_bButtonDown; }
	void				SetCheck(bool bCheck)			{ m_bButtonDown = bCheck; }
	int					GetGroupID()					{ return m_nGroupID; }
	void				SetGroupID(int nGroupID)		{ m_nGroupID = nGroupID;}

protected:
	void		Clicked();
};

#endif

