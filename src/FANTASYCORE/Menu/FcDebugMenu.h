#pragma once

#include "SmartPtr.h"

#include "BsUiLayer.h"
#include "FcMenuForm.h"

#ifndef _LTCG
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CFcMenuDebug : public CFcMenuForm
{
public:
	CFcMenuDebug(_FC_MENU_TYPE nType);
};


class BsUiMainDebug : public BsUiLayer {
public:
	BsUiMainDebug(CFcMenuForm* pMenu);
	void			Initialize();
	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);
	void			OpenWorldLoadList();
	void			SetLocalLanguage();
	void			SetPlayerCount();
	void			FreeCamMode();
	void			CamEditMode();
	void			CamSave();
	void			StartTitle();
	void			ToggleSaveSet();
	void			OpenSavePlayFile();
	void			StartPauseMenu();
	void			StartStatusMenu();

	void			UpdateListBox();

protected:
	void			InitSize();
	void			OnKeyDownListBox(xwMessageToken* pMsgToken);

	CFcMenuForm*	m_pMenu;
};

typedef CSmartPtr< BsUiMainDebug > DebugMenuHandle;

class BsUiSubDebug : public BsUiLayer {
public:
	BsUiSubDebug(CFcMenuForm* pMenu);
	void			Initialize();
	void			InitWorldLoadList();
	void			InitLocalLanguage();
	void			InitPlayList();
	
	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);

protected:
	void			InitSize();
	void			OnKeyDownListBox(xwMessageToken* pMsgToken);
	void			LoadWorldMap();
	void			LoadPlayfile();
	void			SetLocalLanguage();

	int				m_nOpType;
	CFcMenuForm*	m_pMenu;
};

#endif