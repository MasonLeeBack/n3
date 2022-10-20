#include "stdafx.h"

#include "FcMenuMainOpening.h"
#include "BSuiSystem.h"

#include "BsUiClassIDManager.h"
#include "BsUiFontAttrManager.h"

#include "FcInterfaceManager.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


#define MAIN_OPENING_TIME			120

CFcMenuMainOpening::CFcMenuMainOpening(_FC_MENU_TYPE nType)
: CFcMenuForm(nType)
{
	AddUiLayer(SN_MainOpeningMovie, -1, -1);
	m_ntime = g_BsUiSystem.GetMenuTick();

	SetStatus(_MS_NORMAL);
}

void CFcMenuMainOpening::RenderProcess()
{
	int ntime = g_BsUiSystem.GetMenuTick();

	BsUiHANDLE hLayer = GetUiLayer(SN_MainOpeningMovie);
	if(hLayer != NULL && m_ntime + MAIN_OPENING_TIME < ntime)
	{
		RemoveUiLayer(hLayer);
		g_MenuHandle->PostMessage(fcMSG_TITLE_START);
	}
}