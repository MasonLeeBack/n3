#ifndef _BsUi_LAYER_H_
#define _BsUi_LAYER_H_


#include "BSuiWindow.h"


class BsUiLayer : public BsUiWindow {
public:
	BsUiLayer();
	BsUiLayer(BsUiCLASS hClass, POINT pos, BsUiWindow* pParent);
	~BsUiLayer() { Release(); }

	bool			Create(BsUiCLASS hClass, int x, int y, BsUiWindow* pParent);
	bool			Create();
	void			Release();

	void			SetFocusWindowClass(BsUiCLASS hClass);

public:
	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);
};

#endif
