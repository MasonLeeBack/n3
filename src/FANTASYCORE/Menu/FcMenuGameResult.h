#pragma once

#include "BsUiLayer.h"
#include "FcMenuForm.h"

class CFcMenuGameResult : public CFcMenuForm
{
public:
	CFcMenuGameResult(_FC_MENU_TYPE nType);
	void RenderProcess();

protected:
	void SetNextMenufromSuccess();
	void SetNextMenufromFailed();

protected:
	int			m_ntime;
	int			m_nLimitTick;
	bool		m_bLoadRealMovie;
	bool		m_bPlayRealMovie;
};


//---------------------------------------------------------
//---------------------------------------------------------
class CFcMenuFailedLayer;
class CFcMenuFailed : public CFcMenuForm
{
public:
	CFcMenuFailed(_FC_MENU_TYPE nType);

	void ProcessMsg(int nMsgAsk, BsUiHANDLE hWnd, int nMsgRsp);
	virtual void RenderProcess();

protected:
	void RenderMoveLayer(int nTick);

protected:
	CFcMenuFailedLayer* m_pLayer;
};

class CFcMenuFailedLayer : public BsUiLayer
{
public:
	CFcMenuFailedLayer(CFcMenuForm* pMenu)	{ m_pMenu = pMenu; }

	void			Initialize();
	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);

protected:
	void OnKeyDownBtRestart(xwMessageToken* pMsgToken);
	void OnKeyDownBtStageSel(xwMessageToken* pMsgToken);
	void OnKeyDownBtGotoTitle(xwMessageToken* pMsgToken);

protected:
	CFcMenuForm* m_pMenu;
};