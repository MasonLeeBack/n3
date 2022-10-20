#include "stdafx.h"

#include "BsUiSystem.h"
#include "BsUiText.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

BsUiText::BsUiText() : BsUiWindow()
{
	
}

BsUiText::BsUiText(BsUiCLASS hClass, POINT pos, BsUiWindow* pParent)
{
	BsAssert(hClass);

	BsUiWindow::Create(BsUiTYPE_TEXT,
		hClass,
		pos.x, pos.y,
		100, 30,
		XWATTR_SHOWWINDOW | XWATTR_MOVABLE | XWATTR_DISABLE,
		pParent);
}

void BsUiText::Release()
{
}

bool BsUiText::Create(BsUiCLASS hClass, int x, int y, BsUiWindow* pParent)
{
	BsAssert(hClass != 0);

	BsUiTextItem* pTextItem = static_cast <BsUiTextItem* > (g_BsUiSystem.GetLoader()->GetClassInstance(hClass));
	BsAssert(pTextItem);

	//text
	BsUiWindow::SetItemTextID(pTextItem->nTextId);
	for(int i=0; i<BsUiFS_COUNT; i++)
	{
		BsUiWindow::SetFontAttr((BsUiFocusState)i, pTextItem->nFontAttr[i]);
	}

	return BsUiWindow::Create(pTextItem->nType, 
		hClass, x, y, 
		pTextItem->size.cx, pTextItem->size.cy, 
		pTextItem->nAttr, pParent);
}

DWORD BsUiText::ProcMessage(xwMessageToken* pMsgToken)
{
	BsAssert(pMsgToken);

	BsUiCLASS hClass = BsUi::BsUiGetClass(pMsgToken->hWnd);
	BsAssert(hClass);
	
	switch (pMsgToken->message)
	{
	case XWMSG_LBUTTONDOWN:
		{
			BsUiSystem::SetGrabWindow(this);
		}
		break;
	}

	return BsUiWindow::ProcMessage(pMsgToken);
}
