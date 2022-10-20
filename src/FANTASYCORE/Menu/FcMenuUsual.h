#pragma once

#include "BsUiLayer.h"
#include "FcMenuForm.h"

class CFcMenuUsualLayer;
class CFcMenuUsual : public CFcMenuForm
{
public:
	CFcMenuUsual(_FC_MENU_TYPE nType);
	~CFcMenuUsual();

	void SetFocus(int nMenuType);
	void Reinit();

	void ProcessMsg(int nMsgAsk, BsUiHANDLE hWnd, int nMsgRsp);
	void Process();

	virtual void	RenderProcess();

protected:
	void RenderMoveLayer(int nTick);

protected:
	CFcMenuUsualLayer*	m_pUsual;
};



class CFcMenuUsualLayer : public BsUiLayer
{
public:
	CFcMenuUsualLayer(CFcMenuUsual* pMenu)		{ m_pMenu = pMenu; }
	void			Initialize();
	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);

protected:
	void OnKeyDownBtContinue(xwMessageToken* pMsgToken);
	void OnKeyDownBtSave(xwMessageToken* pMsgToken);
	void OnKeyDownBtLoad(xwMessageToken* pMsgToken);
	void OnKeyDownBtOption(xwMessageToken* pMsgToken);
	void OnKeyDownBtExit(xwMessageToken* pMsgToken);

	CFcMenuUsual* m_pMenu;
};