#pragma once

#include "BsUiLayer.h"
#include "FcMenuForm.h"

class CFcMenuCharLeftLayer;
class CFcMenuCharRightLayer;

class CFcMenuCharStock : public CFcMenuForm
{
public:
	CFcMenuCharStock(_FC_MENU_TYPE nType, DWORD dwMissionType);
	virtual void ProcessMsg(int nMsgAsk, BsUiHANDLE hWnd, int nMsgRsp);
	virtual void RenderProcess();
	
	void UpdatePointLayer();
	void UpdateLeftLayer();
	void UpdateRightLayer();

	void SetCharFocus(DWORD dwMissionType);

	void ClickAItem(BsUiHANDLE hWnd);

protected:
	void RenderMoveLayer(int nTick);
	void UpdateSetButton(int nItemID, BsUiWindow* pButton, BsUiWindow* pImg, BsUiWindow* pNew);
	void UpdateNeedPoint(DWORD dwMissionType);

protected:
	BsUiLayer*				m_pBaseLayer;
	BsUiLayer*				m_pPointLayer;
	CFcMenuCharLeftLayer*	m_pCharLeftLayer;
	CFcMenuCharRightLayer*	m_pCharRightLayer;
};



class CFcMenuCharLeftLayer : public BsUiLayer
{
public:
	CFcMenuCharLeftLayer(CFcMenuCharStock* pMenu)		{ m_pMenu = pMenu; }
	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);

protected:
	CFcMenuCharStock*	m_pMenu;
};


class CFcMenuCharRightLayer : public BsUiLayer
{
public:
	CFcMenuCharRightLayer(CFcMenuCharStock* pMenu)		{ m_pMenu = pMenu; }
	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);

protected:
	CFcMenuCharStock*	m_pMenu;
};



//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
class CFcMenuCharStockSubLayer;
class CFcMenuCharStockSub : public CFcMenuForm
{
public:
	CFcMenuCharStockSub(_FC_MENU_TYPE nType, DWORD dwData);
	virtual void RenderProcess();

	DWORD GetMissionType()		{ return m_dwMissionType; }
	
protected:
	void UpdateBaseLayer(DWORD dwMissionType);
	void UpdateLeftLayer(DWORD dwMissionType);
	void UpdateRightLayer(DWORD dwMissionType);
	void RenderMoveLayer(int nTick);
	
protected:
	BsUiLayer*	m_pBaseLayer;
	BsUiLayer*	m_pLeftLayer;
	CFcMenuCharStockSubLayer*	m_pRightLayer;

	DWORD m_dwMissionType;
};



class CFcMenuCharStockSubLayer : public BsUiLayer
{
public:
	CFcMenuCharStockSubLayer(CFcMenuCharStockSub* pMenu)	{ m_pMenu = pMenu; }
	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);

protected:
	CFcMenuCharStockSub* m_pMenu;
};
