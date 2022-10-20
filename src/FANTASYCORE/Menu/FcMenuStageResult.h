#pragma once

#include "BsUiLayer.h"
#include "FcMenuForm.h"
#include "FcGlobal.h"

class CFcMenuStageResultLayer;
class CFcMenuStageResult : public CFcMenuForm
{
public:
	CFcMenuStageResult(_FC_MENU_TYPE nType);

	void RenderProcess();
	void ProcessMsg(int nMsgAsk, BsUiHANDLE hWnd, int nMsgRsp);

	void SetTime(int nTime)			{ m_ntime = nTime; }
	void GoToTitleorWorldMap();

protected:
	void RenderNormal();
	void RenderMoveLayer(int nTick);

	int		CheckClearChar();
	void	CheckAchievement();
	void	CheckHighestLevelAllClearAchievement();

protected:
	int m_ntime;
	DWORD m_dwClearMission;

	CFcMenuStageResultLayer*	m_pLayer;

};



class CFcMenuStageResultLayer : public BsUiLayer
{
public:
	CFcMenuStageResultLayer(CFcMenuForm* pMenu);

	void			Initialize();
	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);

protected:
	void UpdateStageResultInfo();
	void UpdateClearTime(StageResultInfo* pStageInfo);
	void UpdatePlayGrade(StageResultInfo* pStageInfo);
	void UpdateBackground();
	void UpdateTextCtrl(BsUiCLASS hClassID, int nValue);
	void UpdateNewIcon(BsUiCLASS hClassID);

	void UpdateSpecialLevelUp();
	void UpdateSpecialItem();
	void UpdateSpecialMessage(bool bOn, int nLineCount = 0);

protected:
	CFcMenuForm*	m_pMenu;
	bool			m_bLevelUp;
	bool			m_bSpecialItem;

	char			m_cFullItem[1024];
	int				m_nItemCount;
};