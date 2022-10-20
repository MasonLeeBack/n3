#pragma once

#include "BsUiLayer.h"
#include "FcMenuForm.h"
#include "FcMenuWorldMap.h"

class CFcMenuGrd0Layer;
class CFcMenuGrd1Layer;
class CFcMenuGrd2Layer;

class CFcMenuGuardian : public CFcMenuForm
{
public:
	CFcMenuGuardian(_FC_MENU_TYPE nType);
	~CFcMenuGuardian();

	void ProcessMsg(int nMsgAsk, BsUiHANDLE hWnd, int nMsgRsp);
	WorldMapPointInfo* GetPointInfo()			{ return m_pPointInfo; }

	virtual void RenderProcess();
	void RenderMoveLayer(int nTick);
	void RenderNormal();

	void UpdateGrdImgList(BsUiCLASS hImg, int nSelType);
	
protected:
	void Initialize();
	void UpdateSelGrdImg(BsUiLayer* pLayer, BsUiImageCtrl* pImgCtrl, int ntempTick);
	void UpdateGrdImg(BsUiImageCtrl* pImgCtrl, int nType, bool bSel);

public:
	int		m_nTick;

	int		m_nGrdType_L;
	int		m_nGrdType_R;

	CFcMenuGrd0Layer*	m_pGrd0Layer;
	CFcMenuGrd1Layer*	m_pGrd1Layer;
	CFcMenuGrd2Layer*	m_pGrd2Layer;
	
	int		g_GrdTypeList[GT_MAX];
	int		m_nSelGrdTick_L;
	int		m_nSelGrdTick_R;

protected:
	WorldMapPointInfo* m_pPointInfo;
};

//-------------------------------------------------
class CFcMenuGrd0Layer : public BsUiLayer
{
public:
	CFcMenuGrd0Layer() {}
};



//-------------------------------------------------
class CFcMenuGrd1Layer : public BsUiLayer
{
public:
	CFcMenuGrd1Layer(CFcMenuGuardian* pMenu);

	void			Initialize();
	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);

protected:
	CFcMenuGuardian*	m_pMenu;
};


//-------------------------------------------------
class CFcMenuGrd2Layer : public BsUiLayer
{
public:
	CFcMenuGrd2Layer(CFcMenuGuardian* pMenu);

	void			Initialize();
	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);

protected:
	CFcMenuGuardian*	m_pMenu;
};