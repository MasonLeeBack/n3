#pragma once

#include "BsUiLayer.h"
#include "FcMenuForm.h"

class CFcMenuArtWorkLayer;
class CFcMenuArtViewerLayer;

//--------------------------------------------------------------------
class CFcMenuArtWork : public CFcMenuForm
{
public:
	CFcMenuArtWork(_FC_MENU_TYPE nType);
	virtual void ProcessMsg(int nMsgAsk, BsUiHANDLE hWnd, int nMsgRsp);
	virtual void RenderProcess();

	void ChangePage(BsUiCLASS hClass, int nItemID = -1);

	void UpdatePointLayer();
	void UpdateNeedPoint(DWORD dwItemID);

protected:
	void RenderMoveLayer(int nTick);

protected:
	BsUiLayer*					m_pBaseLayer;
	BsUiLayer*					m_pPointLayer;
	CFcMenuArtWorkLayer*		m_pArtWorkLayer;
	CFcMenuArtViewerLayer*		m_pViewerLayer;

	BsUiCLASS					m_hClassCurLayer;
};

//--------------------------------------------------------------------
class CFcMenuArtWorkLayer : public BsUiLayer
{
public:
	CFcMenuArtWorkLayer(CFcMenuArtWork* pMenu)			{ m_pMenu = pMenu; }
	void			Initialize();
	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);

protected:
	void OnKeyDown_UP(xwMessageToken* pMsgToken);
	void OnKeyDown_DOWN(xwMessageToken* pMsgToken);
	void OnKeyDown_LEFT(xwMessageToken* pMsgToken);
	void OnKeyDown_RIGHT(xwMessageToken* pMsgToken);
	void OnKeyDown_SHOULDER_LEFT(xwMessageToken* pMsgToken);
	void OnKeyDown_SHOULDER_RIGHT(xwMessageToken* pMsgToken);
	void ClickAItem(BsUiWindow* pWindow);

	void UpdateImageInfo();

protected:
	CFcMenuArtWork* m_pMenu;

	int		m_nMaxItamCount;
	int		m_nCurPage;
};

//--------------------------------------------------------------------
class CFcMenuArtViewerLayer : public BsUiLayer
{
public:
	CFcMenuArtViewerLayer(CFcMenuArtWork* pMenu)		{ m_pMenu = pMenu; }
	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);
	void			UpdateViewer(int nItemID);

protected:
	CFcMenuArtWork* m_pMenu;
	DWORD	m_hClass;
	int		m_nItemID;
};
