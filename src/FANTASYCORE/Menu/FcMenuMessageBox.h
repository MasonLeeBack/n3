#pragma once

#include "BsUiLayer.h"
#include "FcMenuForm.h"

class CFcMenuMessageBoxLayer;
class CFcMenuMessageBox : public CFcMenuForm
{
public:
	CFcMenuMessageBox(_FC_MENU_TYPE nType,
		int nMsgAskType,
		DWORD dwMainState,
		CFcMenuForm* pCmdMenu,
		BsUiHANDLE m_hCmdWnd,
		DWORD dwFocusButton,
		DWORD dwValue);
	
	virtual void RenderProcess();
	int		GetMsgAsk()				{ return m_nMsgAskType; }


protected:
	CFcMenuMessageBoxLayer*	m_pMessageBox;
	DWORD					m_dwMainState;
	CFcMenuForm*			m_pCmdMenu;
	BsUiHANDLE				m_hCmdWnd;
	int						m_nMsgAskType;
	DWORD					m_dwFocusButton;
};



class CFcMenuMessageBoxLayer : public BsUiLayer
{
public:
	CFcMenuMessageBoxLayer(CFcMenuForm* pMenu);
	void			Initialize(int nMsgAskType, int nFocusButton, DWORD dwValue);
	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);
	DWORD			GetResponse()		{ return m_dwResponse; }

protected:
	void SetButton(int nMsgAskType, int nFocusButton);

	CFcMenuForm*	m_pMenu;
	int				m_nButtonCount;
	DWORD			m_dwResponse;
};