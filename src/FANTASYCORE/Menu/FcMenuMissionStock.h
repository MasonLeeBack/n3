#pragma once

#include "BsUiLayer.h"
#include "FcMenuForm.h"
#include "FcGlobal.h"

class CFcMenuMssnHeroLayer;
class CFcMenuMssnStageLayer;

class CFcMenuMissionDB : public CFcMenuForm
{
public:
	CFcMenuMissionDB(_FC_MENU_TYPE nType, DWORD dwItemID);
	virtual void RenderProcess();

	void UpdateHeroLayer(DWORD dwHeroType);
	void UpdateStageLayer(DWORD dwHeroType, DWORD dwStageType);

	void ClickAItem(BsUiHANDLE hWnd);

protected:
	void UpdateMissionData();	
	void RenderMoveLayer(int nTick);

public:
	BsUiLayer*				m_pBaseLayer;
	CFcMenuMssnHeroLayer*	m_pHeroLayer;
	CFcMenuMssnStageLayer*	m_pStageLayer;

protected:	
	int						m_nStageOpenList[MAX_MISSION_TYPE][MAX_STAGE];
	int						m_nStageItemList[MAX_MISSION_TYPE][MAX_STAGE];
	int						m_nStageSortList[MAX_MISSION_TYPE][MAX_STAGE];
};

//--------------------------------------------------------------------
class CFcMenuMssnHeroLayer : public BsUiLayer
{
public:
	CFcMenuMssnHeroLayer(CFcMenuMissionDB* pMenu)			{ m_pMenu = pMenu; }
	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);

protected:
	void OnKeyDownLbMissionDB(xwMessageToken* pMsgToken);

protected:
	CFcMenuMissionDB* m_pMenu;
};

//--------------------------------------------------------------------
class CFcMenuMssnStageLayer : public BsUiLayer
{
public:
	CFcMenuMssnStageLayer(CFcMenuMissionDB* pMenu)			{ m_pMenu = pMenu; }
	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);

protected:
	void OnKeyDownLbStageDB(xwMessageToken* pMsgToken);

protected:
	CFcMenuMissionDB* m_pMenu;
};



//--------------------------------------------------------------------
//--------------------------------------------------------------------
//--------------------------------------------------------------------
class CFcMenuMissionDBSubLayer;
class CFcMenuMissionDBSub : public CFcMenuForm
{
public:
	CFcMenuMissionDBSub(_FC_MENU_TYPE nType, DWORD dwItemID);
	virtual void RenderProcess();

	DWORD	GetItemID()			{ return m_dwItemID; }

protected:
	void UpdateMissionDB_Sub(DWORD dwItemID);
	void UpdateText(BsUiCLASS hClass, int nValue);
	void RenderMoveLayer(int nTick);
	void UpdateClearTime(int nFullSec);

protected:
	CFcMenuMissionDBSubLayer*		m_pLayer;
	DWORD							m_dwItemID;
};

//--------------------------------------------------------------------
class CFcMenuMissionDBSubLayer : public BsUiLayer
{
public:
	CFcMenuMissionDBSubLayer(CFcMenuMissionDBSub* pMenu)		{ m_pMenu = pMenu; }
	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);

protected:
	CFcMenuMissionDBSub* m_pMenu;
};