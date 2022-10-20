#pragma once

#include "FcSOXLoader.h"
#include "FcGlobal.h"

#define LIBRARY_SECTION_GAP		1000

class CFcMenuLibraryManager
{
public:
	CFcMenuLibraryManager();	

	void Update();
	void SetState(int nItemID, int nState);
	int	 GetState(int nItemID);
	int	 GetItemID(LIBRARY_SECTION nSection, DWORD dwData=0);
	int	 GetItemIndex(LIBRARY_SECTION nSection, DWORD dwData);
	int	 GetStartSectionIndex(LIBRARY_SECTION nSection)		{ return m_nStartSectionIndex[nSection]; }
	
	LibraryTable* GetItemData(int nItemID);
	LibraryTable* GetItemDataforIndex(int nIndex);

	bool IsFirstNewItem()	{ return m_bFirstNewItem; }
	bool IsNewItemInSection(LIBRARY_SECTION nSection);

protected:
	bool GetDefaultCondition(LibraryTable* pTableItem);

protected:
	bool m_bFirstNewItem;
	int	 m_nStartSectionIndex[LIB_MAX_COUNT];
};
