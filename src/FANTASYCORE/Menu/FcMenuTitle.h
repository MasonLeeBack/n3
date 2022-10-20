#pragma once

#include "BsUiLayer.h"
#include "FcMenuForm.h"

class CFcMenuTitleLayer;
class CFcMenuTitleStartLayer;

class CFcMenuTitle : public CFcMenuForm
{

public:
	CFcMenuTitle(_FC_MENU_TYPE nType, _FC_MENU_TYPE nPrevType);
	~CFcMenuTitle();

	void Process();
	void ProcessMsg(int nMsgAsk, BsUiHANDLE hWnd, int nMsgRsp);
	void RenderProcess();

	void UpdateLayer(_FC_MENU_TYPE nPrevType);
	void SetItem(int nTime)			{ m_ntime = nTime; }

	void ClickStartPage();
	void ClickBackPage();

	bool IsStartLayer();
	bool IsDeviceConfirmed() const { return m_bConfirmDevice; }

protected:
	void InitSavedMachine();
	
protected:
	int							m_ntime;
	_FC_MENU_TYPE				m_PrevMenu;
	bool						m_bConfirmDevice;	
	bool						m_bNeedSaveFixed;
	bool						m_bWaitSaving;
	int							m_nSaveProcessTick;
	bool						m_bRemovedStorage;  
	bool						m_bNeedShowProfileDeleteWarn;

	BsUiLayer*					m_pBaseLayer;
	CFcMenuTitleLayer*			m_pTitleLayer;
	CFcMenuTitleStartLayer*		m_pStartLayer;
};



class CFcMenuTitleStartLayer : public BsUiLayer
{
public:
	CFcMenuTitleStartLayer(CFcMenuTitle* pMenu);
	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);

	bool	IsFirstStartInput()						{ return m_bFirstStartInput; }
	void	SetFirstStartInput(bool bStartInput)	{ m_bFirstStartInput = bStartInput; }

protected:
	bool			m_bFirstStartInput;
	CFcMenuTitle*	m_pMenu;
};



class CFcMenuTitleLayer : public BsUiLayer
{
public:
	CFcMenuTitleLayer(CFcMenuTitle* pMenu)			{ m_pMenu = pMenu; }
	void			Initialize();
	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);

	void EnableKeyDown(bool bEnable = true) { m_bEnableKeyDown = bEnable; }
	bool IsEnableKeyDown() const { return m_bEnableKeyDown; }

protected:
	void OnKeyDownBt1P(xwMessageToken* pMsgToken);
	void OnKeyDownBtLoadData(xwMessageToken* pMsgToken);
	void OnKeyDownBtLibrary(xwMessageToken* pMsgToken);
	void OnKeyDownBtOption(xwMessageToken* pMsgToken);
	void OnKeyDownBtCreadit(xwMessageToken* pMsgToken);
	void OnKeyDownBtTutorial(xwMessageToken* pMsgToken);

protected:
	CFcMenuTitle* m_pMenu;
	bool	m_bEnableKeyDown;
};