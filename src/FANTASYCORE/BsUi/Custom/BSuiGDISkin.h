#pragma once

#include "BsUiSkin.h"
#include "CrossVector.h"

//--------------------------------------------------------------------
class BsUiGDISkin : public BsUiSkin {
public:
	BsUiGDISkin();
	~BsUiGDISkin();

	virtual int			GetTime();
	virtual	bool		IsCanMove(BsUiWindow* pWnd, POINT pt);		// tool에서 쓸 수 있게 
	virtual int			GetCanSizeState(BsUiWindow* pWnd, POINT pt);

////////////////////////////////////////////////////////////
// tool 

protected:
	bool	m_bSelectedBox;
	RECT	m_rectSelected;

public:
	void	SetSelectedBox(bool bFlag, POINT* point);	
	void	MoveSelectedBox(POINT* point);
	bool	IsSelectedBox()					{ return m_bSelectedBox; }
	RECT	GetSelectedRect()				{ return m_rectSelected; }
};