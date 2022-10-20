#pragma once

#include "BsUiLayer.h"
#include "FcMenuForm.h"

class CFcMenuLibLayer;
class CFcMenuLibNewLayer;

class CFcMenuLibrary : public CFcMenuForm
{
public:
	CFcMenuLibrary(_FC_MENU_TYPE nType, _FC_MENU_TYPE nPrevType);
	virtual void RenderProcess();

protected:
	void UpdateLibLayer(_FC_MENU_TYPE nPrevType);
	void UpdateEtcLayer(_FC_MENU_TYPE nPrevType);
	void RenderMoveLayer(int nTick);

public:
	BsUiLayer*			m_pBaseLayer;	//layer0
	CFcMenuLibLayer*	m_pLibLayer;	//layer1
	BsUiLayer*			m_pPointLayer;	//layer2
	CFcMenuLibNewLayer*	m_pNewLayer;	//layer3

protected:
	_FC_MENU_TYPE		m_nPrevType;
};


class CFcMenuLibLayer : public BsUiLayer
{
public:
	CFcMenuLibLayer(CFcMenuLibrary* pMenu) { m_pMenu = pMenu; }
	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);

protected:
	CFcMenuLibrary* m_pMenu;
};


class CFcMenuLibNewLayer : public BsUiLayer
{
public:
	CFcMenuLibNewLayer(CFcMenuLibrary* pMenu) { m_pMenu = pMenu; }
	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);

protected:
	CFcMenuLibrary* m_pMenu;
};


//------------------------------------------------------------
//------------------------------------------------------------
