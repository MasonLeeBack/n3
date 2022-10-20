#pragma once

#include "BsUiLayer.h"
#include "FcMenuForm.h"
#include "FcGlobal.h"

#ifdef _XBOX
#include "AtgSignIn.h"
#endif //_XBOX


#define SAVEMENU_SLOTS_MAX 3
class CFcMenuSaveDataLayer;

class CFcMenuSaveData : public CFcMenuForm
{
	enum SAVE_STATES { SAVE_STATE_NONE, SAVE_STATE_NEWDATA, SAVE_STATE_OVERWRITE, SAVE_STATE_NEWFIXED };

public:
	CFcMenuSaveData(_FC_MENU_TYPE nType, _FC_MENU_TYPE nPrevType, DWORD dwClearChar);
	
	void ProcessMsg(int nMsgAsk, BsUiHANDLE hWnd, int nMsgRsp);
	void Process();
	virtual void RenderProcess();

	_FC_MENU_TYPE GetPrevType()			{ return m_nPrevType; }

	void NotifyDeviceChanged(BsUiHANDLE	hWnd) {	m_hWarningMsg = hWnd; }
	DWORD GetClearChar()				{ return m_dwClearChar; }

	void CloseAllAboutStorage();

protected:
	void RenderMoveLayer(int nTick);

protected:
	_FC_MENU_TYPE	m_nPrevType;
	DWORD			m_dwClearChar;
	SAVE_STATES		m_nSaveState;
	BsUiHANDLE		m_hFocusedWnd;
	BsUiHANDLE		m_hMessageBox;
	BsUiHANDLE		m_hWarningMsg;


	CFcMenuSaveDataLayer*	m_pLayer;
	bool			m_bNowSaving;
	bool			m_bWaitSaving;
	int				m_nSaveProcessTick;

	bool			m_bConfirmDevice;
	bool			m_bNeedShowProfileDeleteWarn;

	bool			m_bCanExit;

};



class CFcMenuSaveDataLayer : public BsUiLayer
{
	typedef std::vector< pair<std::string, XCONTENT_DATA*> > ITEMS_TYPE;
public:
	CFcMenuSaveDataLayer(CFcMenuSaveData* pMenu);
	void			Initialize();
	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);

	bool			IsFocusedOnListBtn() ;

	void			UpdateListBox();
	void			UpdateLoadInfo();

#ifdef _XBOX
	XCONTENT_DATA* GetSelectedContent() const { return m_items[m_nStartLine+m_nFocusedBtn].second; }
#endif //_XBOX
protected:
	void			OnKeyDownLbSaveData(xwMessageToken* pMsgToken);
	void			UpdateLoadStageImage(int nHeroSoxID, int nStageId);
	void			UpdateLoadRankImage(int nRank) ;
	void			UpdateLoadHeroImage(int nSlot, int nHeroId);
	void			UpdateSlots();


protected:
	CFcMenuSaveData*	m_pMenu;
	int					m_nLoadImageStageID;

	ITEMS_TYPE		m_items;
	BsUiButton*		m_pBtns[SAVEMENU_SLOTS_MAX];
	BsUiImageCtrl*	m_pImages[SAVEMENU_SLOTS_MAX];
	int				m_nStartLine;
	int				m_nFocusedBtn;
	bool			m_bEnableNewSaveData;
};