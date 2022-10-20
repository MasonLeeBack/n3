#pragma once

#include "BsUi.h"
#include "BsUiWindow.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class BsUiView {

public:
	BsUiView();
	virtual ~BsUiView();

	virtual void		Draw(BsUiWindow* pWnd)				{;}
	virtual bool		GetText(int nTextId, char* szText, const size_t szText_len)	{ return NULL;}
};