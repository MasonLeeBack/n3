#pragma once

#include "BsUiLayer.h"
#include "FcMenuForm.h"

class CFcMenuPause : public CFcMenuForm
{
public:
	CFcMenuPause(_FC_MENU_TYPE nType);

	void SetFocus(int nMenuType);
	virtual void RenderProcess();

	void ProcessMsg(int nMsgAsk, BsUiHANDLE hWnd, int nMsgRsp);
};



class CFcMenuPauseLayer : public BsUiLayer
{
public:
	CFcMenuPauseLayer(CFcMenuForm* pMenu);
	void			Initialize();
	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);

protected:
	void OnKeyDownBtContinue(xwMessageToken* pMsgToken);
	void OnKeyDownBtStatus(xwMessageToken* pMsgToken);
	void OnKeyDownBtEquip(xwMessageToken* pMsgToken);
	void OnKeyDownBtAbility(xwMessageToken* pMsgToken);
	void OnKeyDownBtControl(xwMessageToken* pMsgToken);
	void OnKeyDownBtMission(xwMessageToken* pMsgToken);
	void OnKeyDownBtOption(xwMessageToken* pMsgToken);
	void OnKeyDownBtSelStage(xwMessageToken* pMsgToken);
	void OnKeyDownBtExit(xwMessageToken* pMsgToken);

	CFcMenuForm* m_pMenu;
};


//-----------------------------------------------------------------
//-----------------------------------------------------------------

class CFcMenuSpecialSelStage : public CFcMenuForm
{
public:
	CFcMenuSpecialSelStage(_FC_MENU_TYPE nType);

	void ProcessMsg(int nMsgAsk, BsUiHANDLE hWnd, int nMsgRsp);
	void RenderProcess();

	void SetTime(int nTime)			{ m_ntime = nTime; }

protected:
	int			m_ntime;
};


class CFcMenuSpecialSelStageLayer : public BsUiLayer
{
public:
	CFcMenuSpecialSelStageLayer(CFcMenuForm* pMenu);

	void			Initialize();
	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);
	int				GetStage()			{ return m_nStageId; }

	RECT			GetIErect()		{ return m_rectI_E; }
	RECT			GetIJrect()		{ return m_rectI_J; }
	RECT			GetAErect()		{ return m_rectA_E; }
	RECT			GetAJrect()		{ return m_rectA_J; }

protected:
	void OnKeyDownBtSelectChar(xwMessageToken* pMsgToken);

protected:
	CFcMenuForm*	m_pMenu;
	RECT			m_rectI_E;
	RECT			m_rectI_J;
	RECT			m_rectA_E;
	RECT			m_rectA_J;

	int				m_nStageId;
};