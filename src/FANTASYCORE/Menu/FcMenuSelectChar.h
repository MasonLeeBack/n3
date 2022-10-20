#pragma once

#include "BsUiLayer.h"
#include "FcMenuForm.h"

typedef std::vector<DWORD>		ObjHandleLIST;

enum{
	_SEL_SECTION_SEL_CHAR,
	_SEL_SECTION_SEL_TO_FIX_CHAR,
	_SEL_SECTION_FIX_CHAR,
	_SEL_SECTION_FIX_TO_SEL_CHAR,
};

struct SelCharInfo
{
	int				nSoxID;
	int				nMissionType;
	D3DXVECTOR3		vCameraFixPosPerChar;
	float			fObjScale;
	bool			bOpenOffset;
	int				nNameTextID;
	int				nInfoTextID;
};

class CFcMenuSelCharLayer;

class CFcMenuSelChar : public CFcMenuForm
{
public:
	CFcMenuSelChar(_FC_MENU_TYPE nType);
	~CFcMenuSelChar();
	
	virtual void RenderProcess();
	void ProcessMsg(int nMsgAsk, BsUiHANDLE hWnd, int nMsgRsp);

	ObjHandleLIST*	GetObjHandleLIST()				{ return &m_3DObjHandle; }
	void	SetCircleRotateDir(DWORD dwKeyValue);
	void	SetRotateYawDir(DWORD dwKeyValue);
	DWORD	GetRotateYawDir()						{ return m_dwRotateDir; }
	
	int		GetSection()							{ return m_nSection; }	
	void	SetSection(int nSection);

	int		GetCurObjIndex()						{ return m_nCurObjIndex; }

protected:
	void RenderNormal();
	void RenderMoveLayer(int nTick);

	void CreateObject();
	void CloseObject();
	void CircleRotate();
	void ObjRotateYaw();
	void ChangeFormation();

	void UpdateCameraLight(D3DXVECTOR3 vCameraPos, D3DXVECTOR3 vLightPos);
	bool CheckEnablePlayer(int nCharPos);

protected:
	ObjHandleLIST			m_3DObjHandle;
	DWORD					m_dwBaseCircleHandle;
		
	DWORD					m_dwRotateDir;

	int						m_nFomationRotateAngle;
	int						m_nCurObjIndex;
	
	int						m_nRotateYawAngle;
	int						m_nCumulationAngle;
	float					m_fLerp;

	int						m_nSection;
	CFcMenuSelCharLayer*	m_pCharLayer;
	int						m_nSkinforInphyyWeapon;
};

class CFcMenuSelCharLayer : public BsUiLayer
{
public:
	CFcMenuSelCharLayer(CFcMenuSelChar* pMenu)		{ m_pMenu = pMenu; }
	void			Initialize();
	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);

	void UpdateCharInfo(int nCharPos);

protected:
	void OnKeyDownBtSelectChar(xwMessageToken* pMsgToken);
	void ShowCharInfo(bool bShow);

protected:
	CFcMenuSelChar*			m_pMenu;
};