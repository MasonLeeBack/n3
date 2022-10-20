#pragma once

#include "BsUiLayer.h"
#include "FcMenuForm.h"

class CFcMenuAbilityLayer;

class CFcMenuAbility : public CFcMenuForm
{
public:
	CFcMenuAbility(_FC_MENU_TYPE nType, _FC_MENU_TYPE nPreType);
	~CFcMenuAbility();

	void ProcessMsg(int nMsgAsk, BsUiHANDLE hWnd, int nMsgRsp) {}
	virtual void RenderProcess();
		
public:
	void UpdateBaseLayer();
	void UpdateNameLayer();

protected:
	void CreateObject();
	void CloseObject();

	void RenderMoveLayer(int nTick);
	
protected:
	DWORD			m_h3DObject;

	BsUiLayer*				m_pBaseLayer;
	BsUiLayer*				m_pNameLayer;
	CFcMenuAbilityLayer*	m_pListLayer;
	int				m_nTick;
};


class CFcMenuAbilityLayer : public BsUiLayer
{
public:
	CFcMenuAbilityLayer(CFcMenuForm* pMenu);
	void			Initialize();
	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);

protected:
	void	UpdateLbAbility();
	void	OnKeyDownLbEquip(xwMessageToken* pMsgToken);
	void	UpdateData(int nPos);

protected:
	CFcMenuForm*	m_pMenu;
};
