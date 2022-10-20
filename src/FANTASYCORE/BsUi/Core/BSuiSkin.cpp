#include "stdafx.h"
#include "BsUiSkin.h"
#include "BsUiSystem.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
BsUiSkin::BsUiSkin()
{	
	ClearDragDropInfo();
}

BsUiSkin::~BsUiSkin()
{
	Release();
}

bool BsUiSkin::DragObject(BsUiWindow* pHoverWnd, xwMessageToken* pMsgMouseMove)
{
	BsAssert(pHoverWnd);

	if (!IsReadyToDrag())
		return false;

	// ������ �ð��� �����°� üũ�Ͽ� D&D�� �㰡�Ѵ�.
	if (m_ddInfo.bDragging == false)
	{
		if (pMsgMouseMove->time - m_ddInfo.msgButtonDown.time < _BsUi_TIME_BEGIN_DRAGDROP)
			return false;
	}

	return true;
}


bool BsUiSkin::IsCanMove(BsUiWindow* pWnd, POINT pt)
{
	// default
	if (pWnd->IsMovableWindow() && pWnd->GetParentWindow() == NULL)
		return true;

	return false;
}