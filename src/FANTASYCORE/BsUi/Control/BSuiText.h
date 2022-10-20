#ifndef _BsUi_TEXT_H_
#define _BsUi_TEXT_H_

#include "BSuiWindow.h"

class BsUiText : public BsUiWindow {
protected:
	
public:
	BsUiText();
	BsUiText(BsUiCLASS hClass, POINT pos, BsUiWindow* pParent);
	virtual ~BsUiText() { Release(); }

	bool		Create(BsUiCLASS hClass, int x, int y, BsUiWindow* pParent);
	void		Release();

	virtual DWORD		ProcMessage(xwMessageToken* pMsgToken);
};

#endif

