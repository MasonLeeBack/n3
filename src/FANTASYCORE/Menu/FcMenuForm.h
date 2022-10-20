#pragma once

#include "FcMenuDef.h"
#include "BSui.h"

#include "BsUiLayer.h"
#include "BsUiButton.h"
#include "BsUiListBox.h"
#include "BsUiListCtrl.h"
#include "BsUiSlider.h"
#include "BSuiText.h"
#include "BSuiImageCtrl.h"

#include "BSuiLoader.h"
#include "BsUiSkin.h"
#include "BsUiMenuInput.h"

#define _TICK_FADE_IN_BASE				6
#define _TICK_IN_FORM_LEFT				12
#define _TICK_IN_FORM_RIGHT				12
#define _TICK_END_OPENING_CLOSING		12
#define _TICK_END_MSG_CLOSING			20


enum _MENU_STATUS
{
	_MS_OPEN,
	_MS_OPENING,
	_MS_NORMAL,
	_MS_CLOSING,
	_MS_CLOSE,
};

class CFcMenuForm
{
public:
	CFcMenuForm(_FC_MENU_TYPE nType);
	virtual ~CFcMenuForm();

	virtual void RenderProcess()											{ m_Status = _MS_NORMAL; }
	virtual void Process()													{}
	virtual void ProcessMsg(int nMsgAsk, BsUiHANDLE hWnd, int nMsgRsp)		{}

	virtual	DWORD		GetHandle()				{ return m_hForm; }
	_FC_MENU_TYPE		GetMenuType()			{ return m_nType; }
	virtual	void		SetFocus(int nType)		{;}

	BsUiHANDLE 	AddUiLayer(BsUiLayer* pLayer, BsUiCLASS hClass, int x = -1, int y = -1);
	BsUiHANDLE 	AddUiLayer(BsUiCLASS hClass, int x = -1, int y = -1);
	void		RemoveUiLayer(BsUiHANDLE handle);
	BsUiHANDLE	GetUiLayer(BsUiCLASS hClass);
	BsUiHANDLE	GetUiLayerforIndex(int nIndex);

	virtual void	SetShowOn(bool bShow);
	bool			IsShowOn()						{ return m_bShowOn; }

	void			SetStatus(_MENU_STATUS status, bool bSoundEffect = true);
	_MENU_STATUS	GetStatus()						{ return m_Status; }

protected:
	DWORD			m_hForm;
	_FC_MENU_TYPE	m_nType;
	bool			m_bShowOn;
	_MENU_STATUS	m_Status;
	int				m_nFormTick;

	std::vector<BsUiHANDLE>			m_HandleList;
};