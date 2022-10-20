#pragma once

#include "BsUiLayer.h"
#include "FcMenuForm.h"

class CFcMenuOpt2Layer;

class CFcMenuOption : public CFcMenuForm
{
public:
	CFcMenuOption(_FC_MENU_TYPE nType, _FC_MENU_TYPE nPreType);
	
	_FC_MENU_TYPE GetPreMenuType()				{ return m_nPreType; }

	virtual void Process();
	virtual void RenderProcess();

	void RenderMoveLayer(int nTick);
	
protected:
	int				m_nTick;
	bool			m_bXUIVibration;

	BsUiLayer*			m_pOption0;
	BsUiLayer*			m_pOption1;
	CFcMenuOpt2Layer*	m_pOption2;
	_FC_MENU_TYPE		m_nPreType;
};


class CFcMenuOpt2Layer : public BsUiLayer
{
public:
	CFcMenuOpt2Layer(CFcMenuForm* pMenu);
	void			Initialize();
	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);

protected:
	CFcMenuForm*	m_pMenu;

protected:
	void	OnKeyDownBtVib(xwMessageToken* pMsgToken);
	void	OnKeyDownBtBGM(xwMessageToken* pMsgToken);
	void	OnKeyDownBtSE(xwMessageToken* pMsgToken);
	void	OnKeyDownBtCamY(xwMessageToken* pMsgToken);
	void	OnKeyDownBtCamX(xwMessageToken* pMsgToken);
	void	OnKeyDownBtVoice(xwMessageToken* pMsgToken);
	void	OnKeyDownBtConst(xwMessageToken* pMsgToken);

protected:
	bool	m_bOptionKey;

	BsUiAttrId	m_nSelAttrId;
	BsUiAttrId	m_nDefAttrId;

	int			m_nSelUVId;
	int			m_nDefUVId;
};