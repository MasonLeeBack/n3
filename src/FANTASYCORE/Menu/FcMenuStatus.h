#pragma once

#include "BsUiLayer.h"
#include "FcMenuForm.h"

class CFcMenuStatusItemListLayer;
class CFcMenuStatusItemTabLayer;

#define _ITEM_ICON_MAX_COUNT		6
#define _ACCESSORY_MAX_COUNT		5

class CFcMenuStatus : public CFcMenuForm
{
public:
	CFcMenuStatus(_FC_MENU_TYPE nType);
	~CFcMenuStatus();

	void	ProcessMsg(int nMsgAsk, BsUiHANDLE hWnd, int nMsgRsp);
	virtual void RenderProcess();
	
	DWORD	Get3DObject()	{ return m_h3DObject; }
	void	UpdateStatusPage(int nPage);

	void	SetItemSlot(int nSlot, int nItemID);
	int		GetItemSlot(int nSlot);
	void	SetCurSlot(int nSlot);
	int 	GetCurSlot()				{ return m_nCurSlot; }
	int		GetCurPage()				{ return m_nCurPage; }
	int		GetAccessorySlotCount()		{ return m_nAccessorySlotCount; }
	
protected:
	void UpdateItemSlot();
	void CreateLayer();
	void CreateObject();
	void CloseObject();
	
	void RenderMoveLayer(int nTick);

public:
	//misc
	void UpdateHeadLineLayer();					//SN_Status1
	void UpdateInfoLayer(int nItemID);			//SN_Status4
	void UpdateHeroNameLayer();					//SN_Status5_1
	void UpdateGaugeLayer();					//SN_Status5_2
	void UpdateItemTabLayer();					//SN_Status8
		
	//status
	void UpdateCharLayer();						//SN_Status2
	void UpdateStateLayer();					//SN_Status6
	
	//item
	void UpdateItemListLayer();					//SN_Status7
	void UpdateCompareLayer();					//SN_Status3
	
	

protected:
	void UpdateItemImage(BsUiImageCtrl* pImg, int nItemID, int nPage, bool bIcon);
	void UpdateCompareInfo(BsUiCLASS hName, BsUiCLASS hImage,
		BsUiCLASS hListCaption, BsUiCLASS hListValue, int nItemID);
	void GetCompareValue(char* szStr, const size_t szStr_len, int nValue);
	void UpdateItemList();
	
protected:
	BsUiLayer*	m_pBaseLayer;							//SN_Status0
	BsUiLayer*	m_pHeadLineLayer;						//SN_Status1
	BsUiLayer*	m_pCharLayer;							//SN_Status2
	BsUiLayer*	m_pCompareLayer;						//SN_Status3
	BsUiLayer*	m_pInfoLayer;							//SN_Status4
	BsUiLayer*	m_pNameLayer;							//SN_Status5_1
	BsUiLayer*	m_pGaugeLayer;							//SN_Status5_2
	BsUiLayer*	m_pStateLayer;							//SN_Status6
	CFcMenuStatusItemListLayer*		m_pListLayer;		//SN_Status7
	CFcMenuStatusItemTabLayer*		m_pTabLayer;		//SN_Status8

	int			m_ItemSlot[_ITEM_ICON_MAX_COUNT];
	int			m_nCurSlot;
	int			m_nCurPage;

	DWORD		m_h3DObject;
	int			m_nTick;
	int			m_nAccessorySlotCount;
};

//-----------------------------------------------------------------
class CFcMenuStatusItemListLayer : public BsUiLayer
{
public:
	CFcMenuStatusItemListLayer(CFcMenuStatus* pMenu);
	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);

protected:
	void OnKeyDownLbItemTab(xwMessageToken* pMsgToken);
	bool AccountItem(xwMessageToken* pMsgToken);
	bool CancelItem(xwMessageToken* pMsgToken);
	void CheckItemEquipLevelDown();

protected:
	CFcMenuStatus*	m_pMenu;
};

//-----------------------------------------------------------------
class CFcMenuStatusItemTabLayer : public BsUiLayer
{
public:
	CFcMenuStatusItemTabLayer(CFcMenuStatus* pMenu);
	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);

protected:
	CFcMenuStatus*	m_pMenu;
};