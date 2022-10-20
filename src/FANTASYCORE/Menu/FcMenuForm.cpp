#include "stdafx.h"
#include "FcMenuForm.h"

#include "BSuiSystem.h"
#include "FcSoundManager.h"

CFcMenuForm::CFcMenuForm(_FC_MENU_TYPE nType)
{
	m_hForm = (DWORD)this;
	m_nType = nType;
	m_bShowOn = true;
	m_nFormTick = 0;
	SetStatus(_MS_OPEN);
}

CFcMenuForm::~CFcMenuForm()
{	
	DWORD dwCount = m_HandleList.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		BsUiHANDLE handle = m_HandleList[i];
		BsAssert(handle != NULL);

		g_BsUiSystem.CloseLayer(handle, false);
	}

	m_HandleList.clear();
}

BsUiHANDLE CFcMenuForm::AddUiLayer(BsUiLayer* pLayer, BsUiCLASS hClass, int x, int y)
{
	BsUiHANDLE hLayer = g_BsUiSystem.OpenLayer(pLayer, hClass, x, y);
	BsAssert(hLayer != NULL);

	m_HandleList.push_back(hLayer);

	return hLayer;
}

BsUiHANDLE CFcMenuForm::AddUiLayer(BsUiCLASS hClass, int x, int y)
{
	BsUiHANDLE hLayer = g_BsUiSystem.OpenLayer(hClass, x, y);
	BsAssert(hLayer != NULL);

	m_HandleList.push_back(hLayer);

	return hLayer;
}

void CFcMenuForm::RemoveUiLayer(BsUiHANDLE handle)
{ 
	BsAssert(handle != NULL);
	DWORD dwCount = m_HandleList.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		if(m_HandleList[i] == handle)
		{
			m_HandleList.erase(m_HandleList.begin()+i);
			g_BsUiSystem.CloseLayer(handle, false);
			break;
		}
	}
}

BsUiHANDLE CFcMenuForm::GetUiLayer(BsUiCLASS hClass)
{
	DWORD dwCount = m_HandleList.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		BsUiHANDLE handle = m_HandleList[i];
		BsAssert(handle != NULL);

		BsUiWindow* pWindow = BsUi::BsUiGetWindow(handle);
		BsAssert(pWindow != NULL);

		if(pWindow->GetClass() == hClass){
			return handle;
		}
	}

	return NULL;
}

BsUiHANDLE CFcMenuForm::GetUiLayerforIndex(int nIndex)
{
	BsAssert((DWORD)nIndex < m_HandleList.size());
	return m_HandleList[nIndex];
}

void CFcMenuForm::SetShowOn(bool bShow)
{ 
	m_bShowOn = bShow;
	DWORD dwCount = m_HandleList.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		BsUiHANDLE handle = m_HandleList[i];
		BsAssert(handle != NULL);

		BsUiWindow* pWindow = BsUi::BsUiGetWindow(handle);
		BsAssert(pWindow != NULL);

		pWindow->SetWindowAttr(XWATTR_SHOWWINDOW, bShow);
	}
}


void CFcMenuForm::SetStatus(_MENU_STATUS status, bool bSoundEffect)
{	
	if(m_Status == status){
		return;
	}
	
	m_Status = status;

	if(bSoundEffect)
	{
		switch(m_Status)
		{
		case _MS_OPEN:		break;
		case _MS_OPENING:	g_pSoundManager->PlaySystemSound( SB_COMMON, "SYS_SLIDE_A"); break;
		case _MS_NORMAL:	break;
		case _MS_CLOSING:	g_pSoundManager->PlaySystemSound( SB_COMMON, "SYS_SLIDE_B"); break;
		case _MS_CLOSE:		break;
		}
	}

	m_nFormTick = g_BsUiSystem.GetMenuTick();
}