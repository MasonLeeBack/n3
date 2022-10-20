#pragma once

#include "BsUi.h"
#include "BsUiWindow.h"

class BsUiLayer;
class BsUiButton;
class BsUiListBox;
class BsUiListCtrl;
class BsUiSlider;
class BsUiText;
class BsUiImageCtrl;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class BsUiStore
{

public:
	BsUiStore();
	virtual ~BsUiStore();

	virtual void	Release() {;}

	virtual bool	SaveWindowList(const char* szFilename);

protected:
	bool			SaveWindow(FILE *fp, BsUiWindow* pWindow);

	bool 			SaveLayer(FILE *fp, BsUiLayer* pWindow, int nParentCnt);
	bool 			SaveButton(FILE *fp, BsUiButton* pWindow, int nParentCnt);
	bool 			SaveListBox(FILE *fp, BsUiListBox* pWindow, int nParentCnt);
	bool			SaveListCtrl(FILE *fp, BsUiListCtrl* pWindow, int nParentCnt);
	bool 			SaveText(FILE *fp, BsUiText* pWindow, int nParentCnt);
	bool 			SaveImageCtrl(FILE *fp, BsUiImageCtrl* pWindow, int nParentCnt);
	bool			SaveSlider(FILE *fp, BsUiSlider* pWindow, int nParentCnt);
	

	bool			SaveChildWindow(FILE *fp, BsUiWindow* pWindow, int nParentCnt);
	
	bool			SaveParam(FILE *fp, BsUiWindow* pWindow, int cmd, int nParentCount);
};