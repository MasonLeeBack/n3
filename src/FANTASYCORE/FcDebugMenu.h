#pragma once

#include "SmartPtr.h"
#include "BsUiLayer.h"
#include "FcMenuManager.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CFcMenuDebug : public CFcMenuForm
{
public:
	CFcMenuDebug(_FC_MENU_TYPE nType);
	~CFcMenuDebug();

	void Process() {;}

protected:
	BsUiHANDLE m_hMenuMain;
	BsUiHANDLE m_hMenuSub;
};


class BsUiMainDebug : public BsUiLayer {
public:
	BsUiMainDebug(CFcMenuForm* pMenu);
	void			Initailize();
	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);
	void			OpenWorldLoadList();
	void			FreeCamMode();
	void			CamEditMode();
	void			CamSave();
	void			StartTitle();

protected:
	void			OnKeyDownListBox(xwMessageToken* pMsgToken);

	CFcMenuForm*	m_pMenu;
};

typedef CSmartPtr< BsUiMainDebug > DebugMenuHandle;

class BsUiSubDebug : public BsUiLayer {
public:
	void			Initailize();
	void			InitList();
	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);

protected:
	void			OnKeyDownListBox(xwMessageToken* pMsgToken);
};