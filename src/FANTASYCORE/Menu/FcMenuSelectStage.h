#pragma once

#include "BsUiLayer.h"
#include "FcMenuForm.h"

class CFcMenuSelStageInfoLayer;

class CFcMenuSelectStage : public CFcMenuForm
{
public:
	CFcMenuSelectStage(_FC_MENU_TYPE nType);
	void ProcessMsg(int nMsgAsk, BsUiHANDLE hWnd, int nMsgRsp);
	virtual void RenderProcess();

	void Process();

protected:
	void ProcessNextPage();
	void UpdateHeadLineImage();
	void UpdateHelpText();
	void UpdateInfoLayer(int nStageID);

	void RenderNormal();
	void RenderMoveLayer(int nTick);

protected:
	int			m_ntime;
	bool		m_bCheckBriefing;

	BsUiLayer*					m_pBaseLayer;
	BsUiLayer*					m_pFrameLayer;
	CFcMenuSelStageInfoLayer*	m_pInfoLayer;
	int							m_nTargetID;

};



class CFcMenuSelStageInfoLayer : public BsUiLayer
{
public:
	CFcMenuSelStageInfoLayer(CFcMenuSelectStage* pMenu)		{ m_pMenu = pMenu; }
	void			Initialize();
	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);	

protected:
	CFcMenuSelectStage*		m_pMenu;
};