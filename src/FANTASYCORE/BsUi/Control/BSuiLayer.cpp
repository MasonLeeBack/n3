#include "stdafx.h"

#include "toputil.h"

#include "BsUiLayer.h"

#include "BsUiSystem.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////////////////////////////////
//

BsUiLayer::BsUiLayer() : BsUiWindow()
{
}


BsUiLayer::BsUiLayer(BsUiCLASS hClass, POINT pos, BsUiWindow* pParent) : BsUiWindow()
{
	BsAssert(hClass != 0);
	
	BsUiWindow::Create(BsUiTYPE_LAYER, 
		hClass, pos.x, pos.y,
		200, 200,
		XWATTR_SHOWWINDOW | XWATTR_MOVABLE,
		pParent);
}

bool BsUiLayer::Create(BsUiCLASS hClass, int x, int y, BsUiWindow* pParent)
{
	BsAssert(hClass != 0);

	// layer 정보 기록
	BsUiLayerItem* pLayerItem = static_cast <BsUiLayerItem* > (g_BsUiSystem.GetLoader()->GetClassInstance(hClass));
	BsAssert(pLayerItem);

	// layer 정보를 읽어서 윈도우를 설정한다.
	return BsUiWindow::Create(pLayerItem->nType, 
		hClass, x, y, 
		pLayerItem->size.cx, pLayerItem->size.cy,
		pLayerItem->nAttr, pParent);
}

void BsUiLayer::Release()
{	
}

DWORD BsUiLayer::ProcMessage(xwMessageToken* pMsgToken)
{
	BsAssert(pMsgToken);

//	BsUiCLASS hClass = BsUi::BsUiGetClass(pMsgToken->hWnd);
//	BsAssert(hClass);
//	BsUiLayerItem* pLayerItem = (BsUiLayerItem* ) BsUiLoader::GetInstance().GetClassInstance(this->GetClass());
//	BsAssert(pLayerItem);

	return BsUiWindow::ProcMessage(pMsgToken);
}

void BsUiLayer::SetFocusWindowClass(BsUiCLASS hClass)
{
	BsUiWindow* pWindow = GetWindow(hClass);
	BsUiSystem::SetFocusWindow(pWindow);
}