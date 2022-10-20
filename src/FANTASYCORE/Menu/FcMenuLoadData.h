#pragma once

#include "BsUiLayer.h"
#include "FcMenuForm.h"
#include "FcGlobal.h"

#ifdef _XBOX
#include "AtgSignIn.h"
#endif //_XBOX

#define LOADMENU_SLOTS_MAX 3

class CFcMenuLoadDataLayer;
class CFcMenuLoadData : public CFcMenuForm
{
public:
	CFcMenuLoadData(_FC_MENU_TYPE nType, _FC_MENU_TYPE nPrevType);
	~CFcMenuLoadData();
	
	void			ProcessMsg(int nMsgAsk, BsUiHANDLE hWnd, int nMsgRsp);
	void			Process();
	virtual void	RenderProcess();

	_FC_MENU_TYPE	GetPrevType()		{ return m_nPrevType; }

	void NotifyDeviceChanged(BsUiHANDLE	hWnd) {	m_hWarningMsg = hWnd; }
	void CloseAllAboutStorage();

protected:
	void RenderMoveLayer(int nTick);

protected:
	_FC_MENU_TYPE m_nPrevType;

	BsUiHANDLE		m_hWarningMsg;
	CFcMenuLoadDataLayer*	m_pLayer;

	bool			m_bConfirmDevice;
	bool			m_bNeedShowProfileDeleteWarn;
	
	bool			m_bNeedSaveFixed;
	bool			m_bWaitSaving;
	int				m_nSaveProcessTick;



};



class CFcMenuLoadDataLayer : public BsUiLayer
{
	typedef std::vector< pair<std::string, XCONTENT_DATA*> > ITEMS_TYPE;
public:
	CFcMenuLoadDataLayer(CFcMenuForm* pMenu);
	void			Initialize();
	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);

#ifdef _XBOX
	XCONTENT_DATA* GetSelectedContent() const { return m_items.empty()? NULL:m_items[m_nStartLine+m_nFocusedBtn].second; }

#endif //_XBOX

	bool			IsFocusedOnListBtn() ;

protected:
	void OnKeyDownLbLoadData(xwMessageToken* pMsgToken);
	void UpdateListBox();
	void UpdateLoadInfo();
	void UpdateLoadRankImage(int nRank);
	void UpdateLoadStageImage(int nHeroSoxID, int nStageId);
	void UpdateLoadHeroImage(int nSlot, int nHeroId);
	void UpdateSlots();

protected:
	CFcMenuForm*	m_pMenu;
	int				m_nLoadImageStageID;

	ITEMS_TYPE		m_items;
	BsUiButton*		m_pBtns[LOADMENU_SLOTS_MAX];
	BsUiImageCtrl*	m_pImages[LOADMENU_SLOTS_MAX];
	int				m_nStartLine;
	int				m_nFocusedBtn;
};