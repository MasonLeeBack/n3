#include "stdafx.h"

#include "FcMenuMissionObj.h"
#include "BSuiSystem.h"

#include "BsUiClassIDManager.h"
#include "BsUiFontAttrManager.h"
#include "FcInterfaceManager.h"
#include "FcMessageDef.h"
#include "FcWorld.h"

#include "BsUiUVManager.h"
#include ".\\data\\Menu\\BsUiUVID.h"
#include "TextTable.h"
#include ".\\data\\FontAttr\\BsUiFontAttrID.h"

#define _MISSION_OBJ_ITEM			6

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

CFcMenuMissionObj::CFcMenuMissionObj(_FC_MENU_TYPE nType)
: CFcMenuForm(nType)
{
	m_pLayer = new CFcMenuMissionObjLayer(this);
	AddUiLayer(m_pLayer, SN_MissionObj, -1, -1);
	m_pLayer->Initialize();
}


void CFcMenuMissionObj::RenderProcess()
{
	switch(GetStatus())
	{
	case _MS_OPEN:
		{
			m_pLayer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));

			SetStatus(_MS_OPENING);
			break;
		}
	case _MS_OPENING:
		{
			int nTick = g_BsUiSystem.GetMenuTick() - m_nFormTick;
			RenderMoveLayer(nTick);

			if(nTick >= _TICK_END_OPENING_CLOSING){
				SetStatus(_MS_NORMAL);
			}
			break;
		}
	case _MS_NORMAL:	break;
	case _MS_CLOSING:
		{
			int nTick = g_BsUiSystem.GetMenuTick() - m_nFormTick;
			RenderMoveLayer(_TICK_END_OPENING_CLOSING - nTick);

			if(nTick >= _TICK_END_OPENING_CLOSING){
				SetStatus(_MS_CLOSE);
			}
			break;
		}
	case _MS_CLOSE:	break;
	}
}

void CFcMenuMissionObj::RenderMoveLayer(int nTick)
{
	if(nTick <= _TICK_FADE_IN_BASE)
	{
		float fAlpha = (float)nTick / _TICK_FADE_IN_BASE;
		m_pLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
	}
}


//-----------------------------------------------------------------------------------------------------
void CFcMenuMissionObjLayer::Initialize()
{	
	m_CurMissionObjPage = 0;

	//copy해서 처리해야 할 듯..

	FcMissionObjList* pFcMissionObjList = g_FcWorld.GetMIssionObjective();
	if((int)pFcMissionObjList->size() > 0)
	{
		int nCount = (int)(pFcMissionObjList->size()) - 1;
		for(int j=0; j<2; j++)
		{
			int nClear = 1;
			if(j == 1){
				nClear = 0;
			}

			for(int i=nCount; i >= 0; i--)
			{
				if((*pFcMissionObjList)[i].nClear == nClear){
					continue;
				}

				tempMissionObj temp;
				temp.nId = (*pFcMissionObjList)[i].nID;
				temp.nClear = (*pFcMissionObjList)[i].nClear;
				temp.nTextID = (*pFcMissionObjList)[i].nTextID;

				m_MissionObjList.push_back(temp);
			}
		}
	}

	UpdateMissionObj();
}

DWORD CFcMenuMissionObjLayer::ProcMessage(xwMessageToken* pMsgToken)
{
	BsAssert(pMsgToken);

	switch(pMsgToken->message)
	{
	case XWMSG_KEYDOWN:
		{
			switch(pMsgToken->lParam)
			{
			case MENU_INPUT_B:
				{
					m_pMenu->SetStatus(_MS_CLOSING);
					g_MenuHandle->PostMessage(fcMSG_PAUSE_START);
					break;
				}
			case MENU_INPUT_UP:
				{
					m_CurMissionObjPage--;
					if(m_CurMissionObjPage < 0){
						m_CurMissionObjPage = 0;
					}
					UpdateMissionObj();
				}
				break;
			case MENU_INPUT_DOWN:
				{	
					DWORD dwCount = m_MissionObjList.size() / _MISSION_OBJ_ITEM;
					int nRest = m_MissionObjList.size() % _MISSION_OBJ_ITEM;
					if(nRest > 0) dwCount++;

					m_CurMissionObjPage++;
					if(m_CurMissionObjPage > (int)dwCount-1){
						m_CurMissionObjPage = dwCount-1;
					}
					UpdateMissionObj();
				}
				break;
			}
		}
	case XWMSG_BN_CLICKED:
		{
			break;
		}
	}

	return BsUiWindow::ProcMessage(pMsgToken);
}


void CFcMenuMissionObjLayer::UpdateMissionObj()
{	
	DWORD nCount = m_MissionObjList.size();

	for(int i=0; i<_MISSION_OBJ_ITEM; i++)
	{
		BsUiWindow* pWindow = GetWindow(TT_MissionObj_List_1+i);
		pWindow->SetItemText(NULL);
	}
	
	char cMissionObj[256];
	DWORD dwItemCount = m_CurMissionObjPage * _MISSION_OBJ_ITEM;
	int nCtrlCount = 0;
	for(dwItemCount; dwItemCount<nCount; dwItemCount++)
	{
		int nTextId = m_MissionObjList[dwItemCount].nTextID;
		g_TextTable->GetText(nTextId, cMissionObj, _countof(cMissionObj));
		BsAssert(cMissionObj != NULL);

		BsUiWindow* pWindow = GetWindow(TT_MissionObj_List_1+nCtrlCount);
		pWindow->SetItemText(cMissionObj);

		if(m_MissionObjList[dwItemCount].nClear == 0){
			pWindow->SetFontAttr(BsUiFS_DISABLE, FA_Itallic_Select);
		}
		else{
			pWindow->SetFontAttr(BsUiFS_DISABLE, FA_Itallic_Disable);
		}

		nCtrlCount++;
		if(nCtrlCount == _MISSION_OBJ_ITEM){
			break;
		}
	}
}