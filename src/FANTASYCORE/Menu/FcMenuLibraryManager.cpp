#include "stdafx.h"
#include "FcMenuLibraryManager.h"

CFcMenuLibraryManager::CFcMenuLibraryManager()
{
	m_bFirstNewItem = false;
	memset(m_nStartSectionIndex, -1, sizeof(int)*LIB_MAX_COUNT);
}

void CFcMenuLibraryManager::Update()
{	
	std::vector<LibraryItem> tempItemList;
	FixedSaveData* pSaveData = &(g_FCGameData.tempFixedSave);

	m_bFirstNewItem = false;
	memset(m_nStartSectionIndex, -1, sizeof(int)*LIB_MAX_COUNT);
	int nNextSection(0);

	//sox에 추가되는 item을 생각해서 sox를 기준으로 한다.
	int nCount = CLibraryTableLoader::GetInstance().GetItemTableCount();
	for(int i=0; i<nCount; i++)
	{
		LibraryTable* pTableItem =  CLibraryTableLoader::GetInstance().GetItemData(i);
		LibraryItem* pLibraryItem = pSaveData->GetLibraryData(pTableItem->nID);

		int nSectionID = pTableItem->nID / LIBRARY_SECTION_GAP;
		if(nNextSection == nSectionID)
		{
			m_nStartSectionIndex[nNextSection] = i;
			nNextSection++;
		}

		//table이 변경이 생긴 경우도 적용된다.
		if(pLibraryItem == NULL)
		{
			LibraryItem item;
			item.nID = pTableItem->nID;
			item.nState = LIS_DEFAULT;

			if(GetDefaultCondition(pTableItem) == false)
			{
				item.nState = LIS_NEW;
				m_bFirstNewItem = true;
			}

			tempItemList.push_back(item);
		}
		else
		{
			switch(pLibraryItem->nState)
			{
			case LIS_DEFAULT:
				{
					//sox이 조건을 비교
					if(GetDefaultCondition(pTableItem) == false)
					{
						pLibraryItem->nState = LIS_NEW;
						m_bFirstNewItem = true;
					}
					break;
				}
			case LIS_NEW:
			case LIS_OPEN: break;
			}
		}

	}

	DWORD dwCount = tempItemList.size();
	for(DWORD i=0; i<dwCount; i++){
		pSaveData->LibraryItemList.push_back(tempItemList[i]);
	}
}

//default일 조건
//   char, stage,  record,        result
//0    -1,  (-1)        X          무조건 false
//1     n,   -1      char          record char에 따라
//2     n,    n      char, stage   record char, stage에 따라
//3    -1,    n         X	       stage 조건만.
bool CFcMenuLibraryManager::GetDefaultCondition(LibraryTable* pTableItem)
{
	FixedSaveData* pSaveData = &(g_FCGameData.tempFixedSave);

	if(pTableItem->nCharCondition == -1)
	{
		int nStageCondition = pTableItem->nStageCondition;
		if(pTableItem->nStageCondition == -1){
			return false;
		}

		for(int i=0; i<CHAR_ID_MAX; i++)
		{
			HeroRecordInfo* pRecordInfo = pSaveData->GetHeroRecordInfo(i);
			if(pRecordInfo == NULL){
				continue;
			}

			StageResultInfo* pStageInfo = pRecordInfo->GetStageResultInfo(nStageCondition);
			if(pStageInfo == NULL){
				continue;
			}

			if(pStageInfo->nMissionClearCount == 0){
				return true;
			}

			return false;
		}

		return true;
	}
	
	HeroRecordInfo* pRecordInfo = pSaveData->GetHeroRecordInfo(pTableItem->nCharCondition);
	if(pRecordInfo == NULL){
		return true;
	}

	if(pTableItem->nStageCondition == -1){
		return false;
	}

	StageResultInfo* pStageInfo = pRecordInfo->GetStageResultInfo(pTableItem->nStageCondition);
	if(pStageInfo == NULL){
		return true;
	}

	if(pStageInfo->nMissionClearCount == 0){
		return true;
	}

	return false;
}

void CFcMenuLibraryManager::SetState(int nItemID, int nState)
{
	if(nItemID == -1){
		return;
	}

	FixedSaveData* pSaveData = &(g_FCGameData.tempFixedSave);
	LibraryItem* pLibraryItem = pSaveData->GetLibraryData(nItemID);
	BsAssert(pLibraryItem);

	pLibraryItem->nState = nState;
}

int	CFcMenuLibraryManager::GetState(int nItemID)
{	
	BsAssert(nItemID != -1);

	FixedSaveData* pSaveData = &(g_FCGameData.tempFixedSave);
	LibraryItem* pLibraryItem = pSaveData->GetLibraryData(nItemID);
	BsAssert(pLibraryItem);

	return pLibraryItem->nState;
}

int	CFcMenuLibraryManager::GetItemID(LIBRARY_SECTION nSection, DWORD dwData)
{
	BsAssert(nSection >= LIB_CHAR && nSection < LIB_MAX_COUNT);
	BsAssert(dwData >= 0 && nSection < LIBRARY_SECTION_GAP);

	return (nSection * LIBRARY_SECTION_GAP) + dwData;
}

int	CFcMenuLibraryManager::GetItemIndex(LIBRARY_SECTION nSection, DWORD dwData)
{	
	BsAssert(nSection >= LIB_CHAR && nSection < LIB_MAX_COUNT);
	BsAssert(dwData >= 0 && nSection < LIBRARY_SECTION_GAP);

	return m_nStartSectionIndex[nSection] + dwData;
}


LibraryTable* CFcMenuLibraryManager::GetItemData(int nItemID)
{	
	int nSection = nItemID / LIBRARY_SECTION_GAP;
	int nId = nItemID % LIBRARY_SECTION_GAP;
	
	int nIndex = m_nStartSectionIndex[nSection] + nId;
	return CLibraryTableLoader::GetInstance().GetItemData(nIndex);
}

LibraryTable* CFcMenuLibraryManager::GetItemDataforIndex(int nIndex)
{	
	return CLibraryTableLoader::GetInstance().GetItemData(nIndex);
}

bool CFcMenuLibraryManager::IsNewItemInSection(LIBRARY_SECTION nSection)
{	
	int nStartID(0), nEndID(0);
	if(nSection != -1)
	{
		nStartID = nSection * LIBRARY_SECTION_GAP;
		nEndID = ((nSection + 1) * LIBRARY_SECTION_GAP) -1;
	}

	FixedSaveData* pSaveData = &(g_FCGameData.tempFixedSave);
	DWORD dwCount = pSaveData->LibraryItemList.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		LibraryItem* pLibraryItem = &(pSaveData->LibraryItemList[i]);
		if(nSection != -1 && (pLibraryItem->nID < nStartID || pLibraryItem->nID > nEndID)){
			continue;
		}

		if(pLibraryItem->nState == LIS_NEW){
			return true;
		}
	}

	return false;
}