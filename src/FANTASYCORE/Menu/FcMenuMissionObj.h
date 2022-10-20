#pragma once

#include "BsUiLayer.h"
#include "FcMenuForm.h"

class CFcMenuMissionObjLayer;
class CFcMenuMissionObj : public CFcMenuForm
{
public:
	CFcMenuMissionObj(_FC_MENU_TYPE nType);
	virtual void RenderProcess();

protected:
	void RenderMoveLayer(int nTick);

protected:
	CFcMenuMissionObjLayer* m_pLayer;
};

struct tempMissionObj
{
	int nId;
	int nClear;
	int nTextID;
};

typedef std::vector<tempMissionObj> tempMissionObjList;

class CFcMenuMissionObjLayer : public BsUiLayer
{
public:
	CFcMenuMissionObjLayer(CFcMenuForm* pMenu)		{ m_pMenu = pMenu; }
	void			Initialize();
	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);

protected:
	void	UpdateMissionObj();

protected:
	CFcMenuForm*		m_pMenu;
	int					m_CurMissionObjPage;
	tempMissionObjList	m_MissionObjList;
};
