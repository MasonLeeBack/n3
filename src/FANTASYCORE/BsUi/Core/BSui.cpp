#include "stdafx.h"
#include "BsUi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG



////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

static BsUiWINDOWDESCVECT		BsUi_descVector;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

bool BsUi::BsUiStartUp()
{
	BsAssert(BsUi_descVector.empty());
	return true;
}

void BsUi::BsUiShutDown()
{
	BsAssert(BsUi_descVector.size() == 0);
}

BsUiHANDLE BsUi::BsUiMakeHandle(BsUiWindow* pThis, BsUiCLASS nClass, BsUiHANDLE hParentWnd)
{
	BsAssert(pThis);

	BsUiWindowDesc* pDesc = new BsUiWindowDesc;

	pDesc->hParentWnd = hParentWnd;
	pDesc->pParent = BsUiGetWindow(hParentWnd);
	pDesc->nClass = nClass;
	pDesc->pThis = pThis;

	BsUi_descVector.push_back(pDesc);
	return (BsUiHANDLE)BsUi_descVector.back();
}

void BsUi::BsUiDestroyHandle(BsUiHANDLE hHandle)
{
	DWORD dwCount = BsUi_descVector.size();
	for(DWORD i=0; i<dwCount; ++i)
	{
		if (BsUi_descVector[i] == (BsUiWindowDesc*)hHandle)
		{
			delete BsUi_descVector[i];
			BsUi_descVector.erase(BsUi_descVector.begin()+i);
			return;
		}
	}

	BsAssert(false);
}

BsUiWindow* BsUi::BsUiGetWindow(BsUiHANDLE hHandle)
{
	if (hHandle == 0 || BsUiIsValid(hHandle) == false)
		return NULL;

	BsUiWindowDesc* pDesc = (BsUiWindowDesc* ) hHandle;
	return pDesc->pThis;
}

BsUiCLASS BsUi::BsUiGetClass(BsUiHANDLE hHandle)
{
	if (hHandle == 0)
		return NULL;

	BsUiWindowDesc* pDesc = (BsUiWindowDesc* ) hHandle;
	return pDesc->nClass;
}

bool BsUi::BsUiIsValid(BsUiHANDLE hHandle)
{
	if (hHandle == NULL)
		return false;

	return (((BsUiWindowDesc* ) hHandle)->nClass != 0);
}
