#include "StdAfx.h"
#include "BsKernel.h"

#include "FcInterfaceManager.h"
#include "FcWorld.h"
#include "FcHeroObject.h"
#include ".\fcitem.h"
#include "FcSoxLoader.h"
#include "bstreamExt.h"
#include "data/ItemDefinitions.h"
#include "FCUtil.h"
#include "DebugUtil.h"
#include "FcGlobal.h"
#include "FcSoundManager.h"
#include "data/FXList.h"
#include "TextTable.h"
#include "FcRealtimeMovie.h"

//#define ENABLE_FULL_STAGE_CLEANOUT

#define _MAX_ITEM_PROBBT				1000000
#define _ITEM_ROTATION_SPEED			3
#define _ITEM_LIFE_TIME					60 //초
#define _START_ALPHA_FRAME				160
#define _ABLE_EATING_TIME				10
#define _WHITE_WOOD_FX_INTERVAL			70
#define _ANGEL_WING_FX_INTERVAL			80
#define _ADD_ITEM_LAND_HEIGHT			50
#define _ADD_WEAPON_ITEM_LAND_HEIGHT	20
#define _ADD_ITEM_WEAPON_RORATE			150

#define _EFFECT_ORB_GAIN_SPEED_PER		150

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

CITemTable		 g_FCItemSOX;
CItemDescTable	 g_FcItemDescTable;
CItemDropTable	 g_FcItemDropTable;
CItemLevelTable  g_FcItemLevelTable;
CItemSetTable	 g_FcItemSetTable;

struct TFn_CompareID
{
	bool operator() ( HeroEquip& A, HeroEquip& B)
	{
		return A.bAccoutered <= B.bAccoutered;
	}
};

//------------------------------------------------------------------------------------------
CFcItemManager::CFcItemManager()
{
	m_SkinList.resize( ITEM_TYPE_LAST );
	for(unsigned int i = 0;i < m_SkinList.size();i++) {	
		m_SkinList[i] = -1;
	}
	m_bDrawItemInfo = false;
	m_nGuadianAttackAdd = 0;
	m_fItemProbbtAdd = 0.f;

	for(int i = 0;i < CIT_LAST;i++ ){	
		m_SetEffectManager[i].Init();		
	}
	m_SetEffectManager[CIT_KNIGHT_MIND].nSoxID	= ITEM_KNIGHT_MIND;
	m_SetEffectManager[CIT_RAGE].nSoxID			= ITEM_RAGE;
	m_SetEffectManager[CIT_ANGEL_BLESSING].nSoxID = ITEM_ANGEL_BLESSING;
	m_SetEffectManager[CIT_GODDEST_DIVINE].nSoxID = ITEM_GODDESS_DIVINE;


}

CFcItemManager::~CFcItemManager()
{
	Finalize();
	for(unsigned int i = 0; i < m_SkinList.size();i++)
	{
		if( m_SkinList[i] != -1){
			g_BsKernel.ReleaseSkin( m_SkinList[i] );
		}
	}
}


// Utility function to unload the skins for the hero items.  This is only really used
// when testing for memory leaks.
//
// In the normal game the hero item skins are not freed explicitly, they are only freed
// when a new hero gets loaded.

void CFcItemManager::UnloadHeroItemSkin()
{
	HeroObjHandle hero = g_FcWorld.GetHeroHandle();
	if ( hero == NULL )
		return;

	ItemDataTable *pData = NULL;
	for(int i = 0; i < g_FCItemSOX.GetItemTableCount(); i++ )
	{
		pData = g_FCItemSOX.GetItemData(i);
		if( CHAR_ID_NONE == pData->nCharSoxID )
			continue;

		if( _stricmp(pData->szSkinName,"noname" ) == 0)
			continue;

		if( hero->GetUnitSoxIndex() == pData->nCharSoxID )
		{
			if( m_SkinList[i] != -1)
			{
				g_BsKernel.ReleaseSkin( m_SkinList[i] );
				m_SkinList[i] = -1;
			}
		}
	}
}

void CFcItemManager::UnloadBasicItemSkin()
{	
	ItemDataTable *pData = NULL;
	for(int i = 0; i < g_FCItemSOX.GetItemTableCount(); i++ )
	{
		pData = g_FCItemSOX.GetItemData(i);
		if( CHAR_ID_NONE != pData->nCharSoxID )
			continue;

		if( _stricmp(pData->szSkinName,"noname") == 0)
			continue;

		if( m_SkinList[i] != -1 )
		{
			g_BsKernel.ReleaseSkin( m_SkinList[i] );
			m_SkinList[i] = -1;
		}
	}
}

void CFcItemManager::VerifyClear()
{
	for(int i = 0; i < g_FCItemSOX.GetItemTableCount(); i++ ){	
		BsAssert( m_SkinList[i] == -1 );
	}
}

void CFcItemManager::LoadHeroItemSkin()
{
#if defined(ENABLE_FULL_STAGE_CLEANOUT)
	LoadBasicItemSkin();
#endif

	int nItemIndex = -1;
	char szSkinFileName[MAX_PATH];

	ItemDataTable *pData = NULL;
	for(int i = 0; i < g_FCItemSOX.GetItemTableCount(); i++ )
	{
		pData = g_FCItemSOX.GetItemData(i);
		if( CHAR_ID_NONE == pData->nCharSoxID )
			continue;
		if( _stricmp(pData->szSkinName,"noname" ) == 0)
			continue;

		if( pData->nItemType == ITP_WEAPON ){ g_BsKernel.chdir( "weapons" ); }
		else{ g_BsKernel.chdir( "Item" ); }		

		sprintf( szSkinFileName,"%s.skin", pData->szSkinName );
		//현재 히어로 미션이랑 비교해서 같으면
		if( g_FcWorld.GetHeroHandle()->GetUnitSoxIndex() == pData->nCharSoxID ){
			if( m_SkinList[i] == -1 ) {	//같으면서 스킨이 로드되지 않았으면
				m_SkinList[i] = g_BsKernel.LoadSkin( -1, szSkinFileName );
				if( m_SkinList[i] == -1){ DebugString("ItemIndex %d couldn't load",i); }
			}
		}
		else{ //현재 미션과 관련이 없으면
			if( m_SkinList[i] != -1) { //지금 현재 히어로 미션이랑 다르면서 로드되어 있는 아이템이라면
				g_BsKernel.ReleaseSkin( m_SkinList[i] ); //해제하고 초기화
				m_SkinList[i] = -1;
			}
		}
		g_BsKernel.chdir( ".." );
	}
}

int CFcItemManager::GetSkinIndex( int nItemSoxID )
{
	return m_SkinList[nItemSoxID];
}

void CFcItemManager::LoadBasicItemSkin() //소모품 포션 등
{
	int nItemIndex = -1;
	char szSkinFileName[MAX_PATH];
	
	ItemDataTable *pData = NULL;
	for(int i = 0; i < g_FCItemSOX.GetItemTableCount(); i++ )
	{
		pData = g_FCItemSOX.GetItemData(i);
		if( CHAR_ID_NONE != pData->nCharSoxID )
			continue;
		if( _stricmp(pData->szSkinName,"noname") == 0)
			continue;

		if( pData->nItemType == ITP_WEAPON ){ g_BsKernel.chdir( "weapons" ); }
		else{ g_BsKernel.chdir( "Item" ); }

		sprintf( szSkinFileName,"%s.skin", pData->szSkinName );
		nItemIndex = g_BsKernel.LoadSkin( -1, szSkinFileName );		
		if(nItemIndex == -1)
		{
			DebugString("ItemIndex %d couldn't load\n",i);
		}

		if( m_SkinList[i] != -1 )
		{
			DebugString("ItemIndex %d is already loaded\n",i);
			g_BsKernel.ReleaseSkin( m_SkinList[i] );
		}

		m_SkinList[i] = nItemIndex;
		g_BsKernel.chdir( ".." );
	}
}

void CFcItemManager::FinishStage()
{
	m_vtOnceEquipSoxID.clear();
	m_vtDurationItem.clear();

	for(int i = 0; i < (int)m_vtItemList.size(); i++ ){	
		m_vtItemList[i].bNew = false;
		m_vtItemList[i].bStopEffect = FALSE;
	}

	for(int i = 0; i < CIT_LAST; i++ ){	
		m_SetEffectManager[i].Init();
	}


#ifdef MAKE_ALL_MISSION_RESLIST
	m_vtItemList.clear();
#endif
	
	
#if defined(ENABLE_FULL_STAGE_CLEANOUT)
	UnloadHeroItemSkin();
	UnloadBasicItemSkin();

	VerifyClear();
#endif

	m_fItemProbbtAdd = 0.f;

}

void CFcItemManager::Finalize()
{
	m_vtDurationItem.clear();
	m_vtItemList.clear();
	m_nGuadianAttackAdd = 0;
	m_nGuadianDefenseAdd = 0;
}

void CFcItemManager::LoadItemData()
{
	g_FCItemSOX.Load();
	g_FcItemLevelTable.Load();
	g_FcItemDropTable.Load();
	g_FcItemSetTable.Load();

#if !defined(ENABLE_FULL_STAGE_CLEANOUT)
	LoadBasicItemSkin();
#endif
}

void CFcItemManager::LoadLocalItemData()
{
	g_FcItemDescTable.Load();
}

int CFcItemManager::GetItemLifeTime()
{
	int nLifeTime = 0;
	for(unsigned int i = 0;i < m_vtItemList.size();i++ )
	{
		if( m_vtItemList[i].bAccoutered == false) continue;

		ItemDataTable *pData =  GetItemDataSoxDirectly( m_vtItemList[i].nItemSoxID );
		nLifeTime += pData->nItemLifeTimeAdd;
	}
	if(nLifeTime <= 0){ nLifeTime = 1; }
	return (FRAME_PER_SEC * _ITEM_LIFE_TIME) * nLifeTime;
	
}

void CFcItemManager::AddItemDropProbbt( float nPer )
{
	m_fItemProbbtAdd += nPer;
}

void CFcItemManager::CreateItemToWorld( int nItemSoxID,D3DXVECTOR3 &Pos,int nDropTableID/* = -1*/ )
{
	if(nItemSoxID != -1)
	{
		CCrossVector Cross;
		ItemHandle Handle;

		//DebugString("Item Probbt : %f SOXID : %d SkinName %s\n",fProbbtStd,vtItemInfo[i].nItemSoxID,GetItemDataSoxDirectly);				
		if( CheckOnlyDropBigRedPortion() ){				
			if(nItemSoxID == ITEM_RED_PORTION_M || nItemSoxID == ITEM_RED_PORTION_S){
				nItemSoxID = ITEM_RED_PORTION_L;
			}					
		}
		if( CheckOnlyDropSmallRedPortion() )
		{
			if(nItemSoxID == ITEM_RED_PORTION_L || nItemSoxID == ITEM_RED_PORTION_M){					
				nItemSoxID = ITEM_RED_PORTION_S;
			}
		}

		if( CheckDropChangeRedToBlue() )
		{
			switch(nItemSoxID)
			{
			case ITEM_RED_PORTION_L:
			case ITEM_RED_PORTION_M:
				nItemSoxID = ITEM_BLUE_PORTION;
				break;
			case ITEM_RED_PORTION_S:
				nItemSoxID = ITEM_BLUE_PORTION_S;
				break;
			}
		}

		Cross.SetPosition( Pos );
		Cross.m_PosVector.y = CBsKernel::GetInstance().GetLandHeight(Cross.m_PosVector.x,Cross.m_PosVector.z);		
		Handle = CFcBaseObject::CreateObject< CFcItem >( &Cross );
		Handle->Initialize( m_SkinList[nItemSoxID], nItemSoxID ,GetItemLifeTime() );
	}
	else if( nDropTableID != -1 )
	{
		CreateItemByDropTable( nDropTableID-1 ,Pos );
	}
}

bool CFcItemManager::IsPortion(int nItemIndex)
{
	switch(nItemIndex)
	{
	case ITEM_RED_PORTION_L: return true;
	case ITEM_RED_PORTION_M: return true;
	case ITEM_RED_PORTION_S: return true;
	case ITEM_BLUE_PORTION:  return true;
	case ITEM_BLUE_PORTION_S: return true;
	case ITEM_WHITE_PORTION_S: return true;
	case ITEM_WHITE_PORTION_M: return true;
	case ITEM_WHITE_PORTION_L: return true;
	}
	return false;
}


void CFcItemManager::GetItemProbbtAdd(float &fItemAddProbbt,float &fPortionProbbt)
{
	for(unsigned int i = 0;i < m_vtItemList.size();i++)
	{
		if( m_vtItemList[i].bAccoutered == false ) {
			continue;
		}

		if(m_vtItemList[i].nItemSoxID != -1)
		{
			int nHPLowerPer = GetItemDataSoxDirectly( m_vtItemList[i].nItemSoxID )->nHPConditionLowerPer;
			if( nHPLowerPer )
			{
				float fHPRatio = (float)g_FcWorld.GetHeroHandle()->GetHP() / (float)g_FcWorld.GetHeroHandle()->GetMaxHP();
				int nMyHPPer = (int)(fHPRatio * 100.f);
				if( nHPLowerPer < nMyHPPer )
					continue;
			}

			fItemAddProbbt += (float)GetItemDataSoxDirectly( m_vtItemList[i].nItemSoxID )->nItemGainProbbt;
			fPortionProbbt += (float)GetItemDataSoxDirectly( m_vtItemList[i].nItemSoxID )->nPotionProbbt;
		}
	}

	for(unsigned int i = 0;i < m_vtDurationItem.size();i++)
	{
		if(m_vtDurationItem[i].nItemSoxID != -1)
		{
			fItemAddProbbt += (float)GetItemDataSoxDirectly( m_vtDurationItem[i].nItemSoxID )->nItemGainProbbt;
			fPortionProbbt += (float)GetItemDataSoxDirectly( m_vtDurationItem[i].nItemSoxID )->nPotionProbbt;
		}
	}	
}

bool CFcItemManager::IsHeroConditionOK( int nSoxID ) //OR조건
{
	bool bAllNone = true;
	ItemDataTable *pItemData = GetItemDataSoxDirectly( nSoxID );
	for( int i = 0;i < FCITEM_HERO_CONDITION_MAX;i++ )
	{
		if( pItemData->nHeroCondition[i] != CHAR_ID_NONE ) 
		{
			bAllNone = false;
			if( pItemData->nHeroCondition[i] == g_FcWorld.GetHeroHandle()->GetUnitSoxIndex() ){
				return true;
			}
		}
	}
	if(bAllNone){
		return true;
	}
	else{
		return false;
	}
}

bool CFcItemManager::IsItemConditionOK( int nSoxID ) //AND조건
{
	ItemDataTable *pItemData = GetItemDataSoxDirectly( nSoxID );
	for( int i = 0;i < FCITEM_ITEM_CONDITION_MAX;i++ )
	{
		if( pItemData->nItemCondition[i] != ITEM_NONE) {
			bool bMatch = false;
			for( int s = 0;s < (int)m_vtItemList.size();s++)
			{
				if( m_vtItemList[s].bAccoutered ) {				
					if( m_vtItemList[s].nItemSoxID == pItemData->nItemCondition[i] ){
						bMatch = true;
					}
				}
			}
			if(bMatch == false)
				return false;
		}
	}
	return true;
}

bool CFcItemManager::CheckOnlyDropBigRedPortion()
{
	unsigned int i = 0;
	for( i = 0;i < m_vtItemList.size();i++ )
	{
		if( m_vtItemList[i].bAccoutered )
		{
			if( GetItemDataSoxDirectly( m_vtItemList[i].nItemSoxID )->bOnlyDropBigRedPortion ){
				return true;
			}
		}
	}

	for( unsigned int i = 0;i < CIT_LAST;i++ )
	{
		if( m_SetEffectManager[i].bEffect )
		{
			if( GetItemDataSoxDirectly( m_SetEffectManager[i].nSoxID )->bOnlyDropBigRedPortion ){
				return true;
			}
		}		
	}	
	return false;
}

bool CFcItemManager::CheckOnlyDropSmallRedPortion()
{
	unsigned int i = 0;
	for( i  = 0;i < m_vtItemList.size();i++ )
	{
		if( m_vtItemList[i].bAccoutered )
		{
			if( GetItemDataSoxDirectly( m_vtItemList[i].nItemSoxID )->bOnlyDropSmallRedPortion ){
				return true;
			}
		}
	}

	for( i = 0;i < CIT_LAST;i++ )
	{
		if( m_SetEffectManager[i].bEffect )
		{
			if( GetItemDataSoxDirectly( m_SetEffectManager[i].nSoxID )->bOnlyDropSmallRedPortion ){
				return true;
			}
		}		
	}
	return false;
}

bool CFcItemManager::CheckDropChangeRedToBlue()
{
	for( unsigned int i = 0;i < m_vtItemList.size();i++ )
	{
		if( m_vtItemList[i].bAccoutered )
		{
			if( GetItemDataSoxDirectly( m_vtItemList[i].nItemSoxID )->bDropChangeRedToBlue ){
				return true;
			}
		}
	}

	for(int i = 0;i < CIT_LAST;i++ )
	{
		if( m_SetEffectManager[i].bEffect )
		{
			if( GetItemDataSoxDirectly( m_SetEffectManager[i].nSoxID )->bDropChangeRedToBlue ){
				return true;
			}
		}		
	}
	return false;
}


void CFcItemManager::CreateItemByDropTable( int nDropTable ,D3DXVECTOR3 &Pos)
{
	BsAssert( nDropTable >= 0 && "You must set item drop table index bigger than -1");

	ItemDropTableInfo *pDropData = g_FcItemDropTable.GetItemDropTableInfo( nDropTable );

	std::vector<ITEM_DROP_INFO> vtItemInfo;
	int i = 0;
	for(i = 0;i < ITEM_DROP_TABLE_MAX;i++)
	{
		if(pDropData->nItemIndex[i] != -1)
		{
			ITEM_DROP_INFO Data;
			Data.nItemSoxID = pDropData->nItemIndex[i];
			Data.nProbt     = pDropData->nProbt[i];
			vtItemInfo.push_back(Data);
		}
	}

	int nSize = vtItemInfo.size();
	if(nSize)
	{
		float fProbbtStd		= (float)Random( _MAX_ITEM_PROBBT );
		float nTotalProbbt		= 0.f;
		float fAddProbbtPortion = 0.f;
		float fAddProbbtItem	= 0.f;		

		GetItemProbbtAdd( fAddProbbtItem,fAddProbbtPortion ); //아이템 나올 확률을 높여주는 아이템을 보유했으면 얻어온다
		for( i = 0; i < nSize; i++ )
		{
			if( !IsHeroConditionOK( vtItemInfo[i].nItemSoxID ) )
				continue;
			if( !IsItemConditionOK( vtItemInfo[i].nItemSoxID ) )
				continue;

			int nItemSoxID = vtItemInfo[i].nItemSoxID;
			float fProbbt = (float)vtItemInfo[i].nProbt;
			fProbbt		 += (fProbbt * (fAddProbbtItem / 100.f));
			fProbbt		 += (fProbbt * (m_fItemProbbtAdd / 100.f));
			nTotalProbbt += fProbbt;

			if( IsPortion( nItemSoxID ) ) { //포션이면
				nTotalProbbt += (fProbbt * (fAddProbbtPortion / 100.f)); //포션 확률 더 높게 해주는 아이템있으면
			}

			if( fProbbtStd < (nTotalProbbt) )
			{
				//DebugString("Item Probbt : %f SOXID : %d SkinName %s\n",fProbbtStd,vtItemInfo[i].nItemSoxID,GetItemDataSoxDirectly);				
				if( CheckOnlyDropBigRedPortion() ){				
					if(nItemSoxID == ITEM_RED_PORTION_M || nItemSoxID == ITEM_RED_PORTION_S){
						nItemSoxID = ITEM_RED_PORTION_L;
					}					
				}
				if( CheckOnlyDropSmallRedPortion() )
				{
					if(nItemSoxID == ITEM_RED_PORTION_L || nItemSoxID == ITEM_RED_PORTION_M){					
						nItemSoxID = ITEM_RED_PORTION_S;
					}
				}

				if( CheckDropChangeRedToBlue() ){
					switch(nItemSoxID)
					{
					case ITEM_RED_PORTION_L:
					case ITEM_RED_PORTION_M:
						nItemSoxID = ITEM_BLUE_PORTION;
						break;
					case ITEM_RED_PORTION_S:
						nItemSoxID = ITEM_BLUE_PORTION_S;
						break;
					}
			
				}

				ItemHandle Handle;
				CCrossVector Cross;
				Cross.m_PosVector = Pos;
				Cross.m_PosVector.y = CBsKernel::GetInstance().GetLandHeight(Cross.m_PosVector.x,Cross.m_PosVector.z);
				Handle = CFcBaseObject::CreateObject< CFcItem >( &Cross );
				Handle->Initialize( m_SkinList[ nItemSoxID ], nItemSoxID, GetItemLifeTime() );
				break;
			}
		}
	}

}

void CFcItemManager::CreateItemToWorld( int nUnitSoxID, int nLevel,CCrossVector *pCross )
{
	if( IsPlayRealMovie() )
	{
		return;
	}

	ITEM_LEVEL_TABLE *pData = g_FcItemLevelTable.GetItemLevelTable( nUnitSoxID );
	BsAssert( nLevel >= 0 && "Level is smaller than 0");

	int nDropTableIndex = pData->nTableIndex[nLevel]-1;
	if(nDropTableIndex < 0)	{
		return;
	}
	CreateItemByDropTable( nDropTableIndex, pCross->m_PosVector );

}

void CFcItemManager::AddItemEffectRange( D3DXVECTOR3 *pPos, float fAreaScale,int nItemSoxID ,bool bPlus)
{
	std::vector< GameObjHandle >vtObjList;
	g_FcWorld.GetObjectListInRangeByTeam( pPos, fAreaScale ,vtObjList, g_FcWorld.GetHeroHandle()->GetTeam() );
	for( unsigned int i = 0;i < vtObjList.size();i++){
		if( !CFcBaseObject::IsValid( vtObjList[i] ) ) continue;
		AddItemEffect( vtObjList[i] ,nItemSoxID ,bPlus );
	}
}

float CFcItemManager::GetDurationTimeAdd()
{
	float fTimeAddPer = 0.f;
	for( unsigned int i = 0;i < m_vtItemList.size();i++)
	{
		if( m_vtItemList[i].bAccoutered == false)
			continue;
		fTimeAddPer += GetItemDataSoxDirectly(	m_vtItemList[i].nItemSoxID )->nDurationTimeAdd;
	}
	return fTimeAddPer;
}

float CFcItemManager::GetHPAddWithGuadian()
{
	float fAddHPPer = 0.f;
	for( unsigned int i = 0;i < m_vtItemList.size();i++)
	{
		if( m_vtItemList[i].bAccoutered == false)
			continue;
		fAddHPPer += GetItemDataSoxDirectly(	m_vtItemList[i].nItemSoxID )->nHPWithGuardian;
	}
	return fAddHPPer;
}

bool CFcItemManager::IsExistItem( int nSoxID )
{
	for(int i = 0;i < (int)m_vtItemList.size();i++)
	{
		if( m_vtItemList[i].nItemSoxID == nSoxID )
			return true;
	}
	return false;
}

bool CFcItemManager::IsSetSuccess(int nSetItemIndex)
{
	ITEM_SET_INFO * pSetItemInfo = g_FcItemSetTable.GetSetItemInfo( nSetItemIndex );

	for(int i = 0;i < MAX_SET_REQUIRE_ITEM;i++)
	{
		if( pSetItemInfo->nItem[i] == -1)
			continue;

		bool bSuccess = false;
		for(int s = 0;s < (int)m_vtItemList.size();s++)
		{
			if( m_vtItemList[s].nItemSoxID == pSetItemInfo->nItem[i] ){			
				bSuccess = true;
			}
		}
		if( !bSuccess )
			return false;
	}
	return true;

}

bool CFcItemManager::IsSetEffectNow( int nSetItemIndex )
{
	ITEM_SET_INFO *pSetItemInfo = g_FcItemSetTable.GetSetItemInfo( nSetItemIndex );

	for(int i = 0;i < CIT_LAST;i++)
	{		
		if( m_SetEffectManager[i].bEffect )
		{
			if( m_SetEffectManager[i].nSoxID == pSetItemInfo->nSetItem){
				return true;
			}
		}
	}
	return false;
}

int CFcItemManager::GetEffectSetItemType( int nSoxID )
{

	for(int i = 0;i < CIT_LAST;i++)
	{
		if( m_SetEffectManager[i].nSoxID == nSoxID ){
			return i;
		}
	}
	BsAssert( 0 && "Set Item index wrong!!" );	
	return CIT_NONE;
}

void CFcItemManager::GiveSetEffect()
{
	for(int i = 0;i < g_FcItemSetTable.GetSetItemCount();i++)
	{
		ITEM_SET_INFO * pSetItemInfo = g_FcItemSetTable.GetSetItemInfo( i );		
		if( pSetItemInfo->nType != SET_ITEM_EFFECT ){ continue; }

		if( IsSetEffectNow(i) ){
			continue;
		}

		int nTypeIndex = GetEffectSetItemType( pSetItemInfo->nSetItem );
		if( nTypeIndex == CIT_NONE ){
			continue;
		}

		if( IsSetSuccess(i) )
		{
			SetItemEffectToHero( pSetItemInfo->nSetItem , true );
			m_SetEffectManager[nTypeIndex].bEffect = true;
			//g_InterfaceManager.SetItemReport( g_FcItemDescTable.GetItemName( pSetItemInfo->nSetItem ) );
		}
		else if(m_SetEffectManager[nTypeIndex].bEffect)
		{
			SetItemEffectToHero( pSetItemInfo->nSetItem , false );
			m_SetEffectManager[nTypeIndex].bEffect = false;
			//g_InterfaceManager.SetItemReport( g_FcItemDescTable.GetItemName( pSetItemInfo->nSetItem ) );
		}
	}
}


void CFcItemManager::MakeSetPrizeItem()
{
	for(int i = 0;i < g_FcItemSetTable.GetSetItemCount();i++)
	{
		ITEM_SET_INFO * pSetItemInfo = g_FcItemSetTable.GetSetItemInfo( i );
		if( IsExistItem( pSetItemInfo->nSetItem ) ){ continue; }
		if( pSetItemInfo->nType != SET_ITEM_MAKE ){ continue; }
		if( IsSetSuccess(i) ){
			AddItem( pSetItemInfo->nSetItem,FALSE,TRUE,TRUE );
		}		
	}
}


void CFcItemManager::AddItem( int nItemSoxID, BOOL bAccounded, BOOL bNew ,BOOL bAddSetPrize /*= FALSE*/)
{
	if(nItemSoxID == -1) {	
		DebugString("Item table set -1");
		return;
	}
	DebugString( "Add item : %d\n", nItemSoxID );
	ItemDataTable *pItemData = g_FCItemSOX.GetItemData( nItemSoxID );
	HeroEquip Equip;	
	DebugString( "ItemData : %s\n", pItemData );
	
	switch( pItemData->nItemType )
	{
	case ITP_NORMAL:
		if( pItemData->nDurTime > 0) { //지속시간 있는 아이템이면
			ITEM_DURATION_INFO Info;
			float fDurTimeAdd = GetDurationTimeAdd();
			Info.nDestTime  = (int)((pItemData->nDurTime * FRAME_PER_SEC) * (1.f + (fDurTimeAdd / 100.f ))); //지속시간
			Info.nItemSoxID = nItemSoxID;
			Info.nPastTime  = 0; //시간이 흐르는건 이걸로 체크함
			Info.nEftInterval = pItemData->nEffectInterval * FRAME_PER_SEC; //시간 간격마다 AddItemEffect를 줘야하는 경우
			Info.Pos = g_FcWorld.GetHeroHandle()->GetPos();

			for(int i = 0;i < (int)m_vtDurationItem.size();i++)
			{
				if( m_vtDurationItem[i].nItemSoxID == nItemSoxID) //같은 종류가 겹쳐서 먹어지면
				{
					SetItemEffectToHero( nItemSoxID, false );
					RestoreEnableEffect( m_vtDurationItem[i] );
					m_vtDurationItem.erase( m_vtDurationItem.begin() + i );
					ApplyEnableEffect();
					--i;
				}
			}

			if( pItemData->fEfftectArea > 0 ) //영역으로 영향을 주는 아이템이면
			{				
				g_FcWorld.GetObjectListInRangeByTeam( &Info.Pos, pItemData->fEfftectArea ,Info.vtObjList, g_FcWorld.GetHeroHandle()->GetTeam() );
				SetItemEffectObjList( Info.vtObjList, nItemSoxID ,true );
			}
			else{ //아니라면 그냥 히어로 한테만
				SetItemEffectToHero( nItemSoxID , true );
			}
			m_vtDurationItem.push_back( Info );
		}
		else{
			if(pItemData->nEffectInterval == 0) {			
				if( pItemData->fEfftectArea > 0.f ) { //주변 영역에 영향을 줘야하는 아이템이라면
					D3DXVECTOR3 Pos = g_FcWorld.GetHeroHandle()->GetPos();
					AddItemEffectRange( &Pos,pItemData->fEfftectArea,nItemSoxID ,true );
				}
				else{
					AddItemEffectToHero( g_FcWorld.GetHeroHandle(), nItemSoxID, true ); //걍 일반적으로 평범한거면 주인공한테만
				}
			}
			else{
				BsAssert( 0 && "This item has Interval time but hasn't duration time");
			}
		}
		ApplyEnableEffect();
		break;
	case ITP_ACCE:
	case ITP_ACCE_SET:
	case ITP_ACCE_UNIQUE:
	case ITP_ACCE_COL: 
	case ITP_ACCE_COL_RLT:
	case ITP_WEAPON:
	case ITP_ACCE_SET_RLT:
		{
			if( IsExistItem( nItemSoxID ) == false )
			{
				Equip.nItemSoxID  = nItemSoxID; 
				Equip.bAccoutered = bAccounded;
				Equip.bNew = bNew;
				m_vtItemList.push_back( Equip );

				if(bAddSetPrize == FALSE){
					MakeSetPrizeItem();
				}
				GiveSetEffect();
				ApplyEnableEffect();
			}
			break;
		}
	}

	
}

void CFcItemManager::ProcessDurationItem()
{
	std::vector< ITEM_DURATION_INFO >::iterator it = m_vtDurationItem.begin();
	while( it != m_vtDurationItem.end() )
	{
		ITEM_DURATION_INFO &di = (*it);
		if( di.nItemSoxID == ITEM_WHITE_WOOD)
		{
			if( (di.nPastTime % _WHITE_WOOD_FX_INTERVAL ) == 0 )
			{
				CCrossVector Cross;
				Cross.SetPosition( di.Pos );
				g_FcWorld.PlaySimpleLinkedFx( FX_ITEM_WHITE01, g_FcWorld.GetHeroHandle(),0 );
			}
		}
		else if( di.nItemSoxID == ITEM_ANGLE_WING_MARK)
		{
			if( (di.nPastTime % _ANGEL_WING_FX_INTERVAL ) == 0 )
			{
				CCrossVector Cross;
				int nEngineIndex = g_FcWorld.GetHeroHandle()->GetEngineIndex();		
				Cross.SetPosition( di.Pos );
				int nBoneIndex = g_BsKernel.SendMessage( nEngineIndex, BS_GET_BONE_INDEX, ( DWORD )"Bip01 Spine2" );
				g_FcWorld.PlaySimpleLinkedFx( FX_ANGEL_WING, g_FcWorld.GetHeroHandle(),nBoneIndex );
			}
		}


		di.nPastTime++;

		ItemDataTable *pData = GetItemDataSoxDirectly( di.nItemSoxID );

		if((*it).nEftInterval){
			if( !( di.nPastTime % di.nEftInterval ) ) { //간격따라 효과주는 아이템					
				if(!pData) { continue; }

				if( pData->fEfftectArea > 0.f ) {
					if( CFcBaseObject::IsValid( g_FcWorld.GetHeroHandle() ) )
					{
						D3DXVECTOR3 Pos = g_FcWorld.GetHeroHandle()->GetPos();
						AddItemEffectRange( &Pos, pData->fEfftectArea, di.nItemSoxID, true );
					}
				}
				else {
					AddItemEffectToHero( g_FcWorld.GetHeroHandle() ,di.nItemSoxID ,true );
				}
			}
		}
		
		if( di.nDestTime < di.nPastTime ) //시간 다됐으면 효과없애자
		{			
			if( pData->fEfftectArea > 0.f ) { //영역으로 영향을 주는 아이템이면
				SetItemEffectObjList( di.vtObjList, di.nItemSoxID , false );
			}
			else{
				SetItemEffectToHero( di.nItemSoxID, false  );
			}
			RestoreEnableEffect( di );
			it = m_vtDurationItem.erase( it );
			ApplyEnableEffect();
			continue;
		}
		it++;
	}
}

const int HERO_STATE_POX_X = 20;

void DrawDebugText(int x,int &y,int w,int h,char *szMsg)
{	
	g_BsKernel.PrintString( x,y,szMsg );
	y += 22;
}


void CFcItemManager::Process()
{
	if( IsPlayRealMovie() || IsPlayEvent() )
		return;

	ProcessEquippedItem();
	ProcessDurationItem();
	HeroObjHandle Handle = g_FcWorld.GetHeroHandle();
	if( m_bDrawItemInfo && CFcBaseObject::IsValid( Handle ) )
	{
		const int MAX_ITEM_MSG = 256;
		char szMsg[MAX_ITEM_MSG];
		int nYPos = 0;

		sprintf(szMsg,"MAX HP : %d",Handle->GetMaxHPAdd() );
		DrawDebugText( HERO_STATE_POX_X, nYPos,400,30,szMsg );

		sprintf(szMsg,"MAX ORB : %d",Handle->GetMaxOrbSparkAdd() );
		DrawDebugText( HERO_STATE_POX_X, nYPos,400,30,szMsg );

		sprintf(szMsg,"ATTACK POWER : %d",Handle->GetAttackPowerAdd() );
		DrawDebugText( HERO_STATE_POX_X, nYPos,400,30,szMsg );

		sprintf(szMsg,"DEFENSE : %d",Handle->GetDefenseAdd() );
		DrawDebugText( HERO_STATE_POX_X, nYPos,400,30,szMsg );

		sprintf(szMsg,"CRITICAL : %d",Handle->GetCriticalAdd() );
		DrawDebugText( HERO_STATE_POX_X, nYPos,400,30,szMsg );

		sprintf(szMsg,"GUARD BREAK : %d",Handle->GetGuardBreak() );
		DrawDebugText( HERO_STATE_POX_X, nYPos,400,30,szMsg );

		sprintf(szMsg,"EXP GAIN SPEED : %d",Handle->GetExpAdd() );
		DrawDebugText( HERO_STATE_POX_X, nYPos,400,30,szMsg );

		sprintf(szMsg,"ATTACK ORB GAIN SPEED : %d", Handle->GetOrbSparkAdd() );
		DrawDebugText( HERO_STATE_POX_X, nYPos,400,30,szMsg );

		sprintf(szMsg,"MOVE SPEED : %d", Handle->GetMoveSpeedAdd() );
		DrawDebugText( HERO_STATE_POX_X, nYPos,400,30,szMsg );

		sprintf(szMsg,"WEAPON REACH : %d", Handle->GetAttackDistAdd() );
		DrawDebugText( HERO_STATE_POX_X, nYPos,400,30,szMsg );

		sprintf(szMsg,"GUADIAN ATTACK POWER : %d", m_nGuadianAttackAdd );
		DrawDebugText( HERO_STATE_POX_X, nYPos,400,30,szMsg );

		sprintf(szMsg,"GUADIAN DEFENSE : %d", m_nGuadianDefenseAdd );
		DrawDebugText( HERO_STATE_POX_X, nYPos,400,30,szMsg );

		sprintf(szMsg,"DASH DISTANCE : %d", Handle->GetDashLengthAdd() );		
		DrawDebugText( HERO_STATE_POX_X, nYPos,400,30,szMsg );

		sprintf(szMsg,"ORB ATTACK TIME : %d", (int)Handle->GetOrbAttackKeepSpeed() );		
		DrawDebugText( HERO_STATE_POX_X, nYPos,400,30,szMsg );
	}
}

void CFcItemManager::ProcessEquippedItem()
{
	ItemDataTable *pData = NULL;
	std::vector<GameObjHandle> vtObjects;
	HeroObjHandle hHero = g_FcWorld.GetHeroHandle();
	if( hHero == NULL ) return;

	D3DXVECTOR3 HeroPos = hHero->GetPos();
	for(unsigned int i = 0;i < m_vtItemList.size();i++)
	{
		if( m_vtItemList[i].bAccoutered == false)
			continue;
		pData = GetItemData(i);
		if(pData == NULL) continue;

		if( pData->nEffectInterval )
		{
			if(! (GetProcessTick() % (FRAME_PER_SEC * pData->nEffectInterval)) ) { //정해진 초마다
				if( pData->nReborn ) 
				{ //계속 살아나게 하는 기능이 있다면
					g_FcWorld.GetObjectListInRangeByDie( &HeroPos, pData->fEfftectArea, vtObjects );
					for(unsigned int s = 0;s < vtObjects.size();s++) {
						if( CFcBaseObject::IsValid( vtObjects[s] ) == false )
							continue;
						if( vtObjects[s]->GetTeam() != hHero->GetTeam())
							continue;
						if( vtObjects[s]->GetClassID() == CFcGameObject::Class_ID_Hero )
							continue;
						vtObjects[s]->Reborn( ((float)pData->nReborn / 100.f) );
					}
				}

				if( hHero->IsDie() ) continue;
				if( pData->nHP ) {				
					hHero->AddHPPercent( pData->nHP );
				}

				if( pData->nIntervalHPAdd ) {				
					hHero->AddHPPercent( pData->nIntervalHPAdd );
				}
			}
		}


		if( pData->nHPConditionLowerPer )
		{
			float fHPRatio = (float)g_FcWorld.GetHeroHandle()->GetHP() / (float)g_FcWorld.GetHeroHandle()->GetMaxHP();
			int nHPPer = (int)(fHPRatio * 100.f);
			if( nHPPer <= pData->nHPConditionLowerPer && m_vtItemList[i].bStopEffect == FALSE ) {
				if( pData->nHPLowerAddOrbGainSpeed ) {				
					g_FcWorld.GetHeroHandle()->SetOrbSparkAdd( g_FcWorld.GetHeroHandle()->GetOrbSparkAdd() + pData->nHPLowerAddOrbGainSpeed );
				}
				m_vtItemList[i].bStopEffect = TRUE;
				ApplyEnableEffect();				
			}
			else if( nHPPer > pData->nHPConditionLowerPer && m_vtItemList[i].bStopEffect == TRUE )
			{
				if( pData->nHPLowerAddOrbGainSpeed ) {				
					g_FcWorld.GetHeroHandle()->SetOrbSparkAdd( g_FcWorld.GetHeroHandle()->GetOrbSparkAdd() - pData->nHPLowerAddOrbGainSpeed );
				}
				m_vtItemList[i].bStopEffect = FALSE;
				ApplyEnableEffect();
			}
		}
	}
}

ItemDataTable *CFcItemManager::GetItemData(int nIndex)
{	
	return g_FCItemSOX.GetItemData( m_vtItemList[nIndex].nItemSoxID );
}

ItemDataTable	*CFcItemManager::GetItemDataSoxDirectly( int nItemSoxID )
{
	return g_FCItemSOX.GetItemData( nItemSoxID );
}

void CFcItemManager::UpdateSort()
{
	//std::sort(m_vtItemList.begin(), m_vtItemList.end(), TFn_CompareID() );
}

char *CFcItemManager::GetItemName(int nItemIndex)
{	
	return g_FcItemDescTable.GetItemName( m_vtItemList[nItemIndex].nItemSoxID );
}

char *CFcItemManager::GetItemDesc(int nItemIndex)
{
	return g_FcItemDescTable.GetItemDesc( m_vtItemList[nItemIndex].nItemSoxID );
}

int CFcItemManager::GetValueByPer(float fPer,int nVal)
{
	return (int)((fPer / 100.f) * (float)nVal);
}

void CFcItemManager::AddItemEffectToHero( HeroObjHandle Handle , int nItemSoxID, bool bPlus )
{
	int nSign = 0;
	(bPlus) ? nSign = 1: nSign = -1;
	ItemDataTable *pData = g_FCItemSOX.GetItemData( nItemSoxID );

	if( pData == NULL ) return;
	if( !CFcBaseObject::IsValid( Handle ) ) return;
	if( Handle->IsDie() ) return;
	Handle->AddOrbSparkPercent( pData->nOrb * nSign );	//오브
	Handle->AddExp( pData->nExpAdd * nSign, Handle );
	g_FcWorld.AddGuardianHP( pData->nGuadianHPAdd * nSign );
	AddItemEffect( Handle,nItemSoxID,bPlus );	

}

int CFcItemManager::GetBluePortionSmallFillWithHP()
{
	int nTotalPercent = 0;
	for( unsigned int i = 0;i < m_vtItemList.size();i++)
	{
		if( m_vtItemList[i].bAccoutered )
		{
			ItemDataTable *pItemData = GetItemDataSoxDirectly(m_vtItemList[i].nItemSoxID);
			nTotalPercent += pItemData->nSmallBluePortionFillHP;
		}
	}
	return nTotalPercent;
}

int CFcItemManager::GetBluePortionBigFillWithHP()
{
	int nTotalPercent = 0;
	for( unsigned int i = 0;i < m_vtItemList.size();i++)
	{
		if( m_vtItemList[i].bAccoutered )
		{
			ItemDataTable *pItemData = GetItemDataSoxDirectly(m_vtItemList[i].nItemSoxID);
			nTotalPercent += pItemData->nBigBluePortionFillHP;
		}
	}
	return nTotalPercent;
}

bool CFcItemManager::IsComsumeItem( int nItemSoxID )
{
	if( GetItemDataSoxDirectly( nItemSoxID )->nItemType == ITP_NORMAL )
	{
		return true;
	}
	return false;

}

void CFcItemManager::AddItemEffect( GameObjHandle Handle ,int nItemSoxID,bool bPlus)
{	
	if( Handle == NULL ) { return; }
	if( !CFcBaseObject::IsValid( Handle ) ) { return; }
	if( Handle->IsDie() ) return;

	int nSign = 0;
	(bPlus) ? nSign = 1: nSign = -1;
	ItemDataTable *pData = GetItemDataSoxDirectly( nItemSoxID );

	if( nItemSoxID >= ITEM_RED_PORTION_L && nItemSoxID <= ITEM_RED_PORTION_S) {	
		g_FcWorld.AddGuardianHP( (int)( pData->nHP * (GetHPAddWithGuadian() /  100.f)) );
	}

	int nSmallBluePortionFillWithHP = GetBluePortionSmallFillWithHP();
	if( nSmallBluePortionFillWithHP && nItemSoxID == ITEM_BLUE_PORTION_S ){	
		Handle->AddHPPercent( nSmallBluePortionFillWithHP * nSign );
	}

	if( IsComsumeItem( nItemSoxID ) && pData->nConsumeItemFillHP ) {
		Handle->AddHPPercent( pData->nConsumeItemFillHP * nSign );
	}

	int nBigBluePortionFillWithHP = GetBluePortionBigFillWithHP();
	if( nBigBluePortionFillWithHP && nItemSoxID == ITEM_BLUE_PORTION ){
		Handle->AddHPPercent( nBigBluePortionFillWithHP * nSign );
	}
	Handle->AddHPPercent( pData->nHP * nSign );
}

void CFcItemManager::NotifyAutoOrbAttack()
{
	for( unsigned int i = 0;i < m_vtItemList.size();i++ ){
		if( m_vtItemList[i].bAccoutered == false)
			continue;
		ItemDataTable *pItemData = GetItemDataSoxDirectly( m_vtItemList[i].nItemSoxID );
		if( pItemData->bAutoOrbAttack )
		{
			SetAccoutered( i , false );
			m_vtOnceEquipSoxID.push_back( m_vtItemList[i].nItemSoxID );
		}
	}
}

bool CFcItemManager::IsUseGuardCharm()
{
	for(int i = 0;i < (int)m_vtItemList.size();i++)
	{
		if( m_vtItemList[i].bAccoutered == false)
			continue;

		if( m_vtItemList[i].nItemSoxID == ITEM_LIFE_CRYSTAL )
			return true;
	}
	return false;
}

void CFcItemManager::NotifyRevival()
{
	const int nNumGuardCharm = 4;

	bool bUseGuardCharm = IsUseGuardCharm();
	if( bUseGuardCharm )
	{
		int nMostCharm = -1;
		int nIndex = -1;
		for(int s = 0;s < (int)m_vtItemList.size();s++)
		{
			if( ITEM_GUARD_CHARM_LV1 <= m_vtItemList[s].nItemSoxID && ITEM_GUARD_CHARM_LV4 >= m_vtItemList[s].nItemSoxID )
			{
				if( nMostCharm < m_vtItemList[s].nItemSoxID ) {						
					nMostCharm = m_vtItemList[s].nItemSoxID;
					nIndex = s;
				}
			}
		}
		if(nIndex != -1) {
			m_vtItemList.erase( m_vtItemList.begin() + nIndex );
		}

		HeroObjHandle hHero = g_FcWorld.GetHeroHandle();
		switch(nMostCharm)
		{
		case ITEM_GUARD_CHARM_LV1: hHero->AddHPPercent( 20 ); break;
		case ITEM_GUARD_CHARM_LV2: hHero->AddHPPercent( 40 ); break;
		case ITEM_GUARD_CHARM_LV3: hHero->AddHPPercent( 60 ); break;
		case ITEM_GUARD_CHARM_LV4: hHero->AddHPPercent( 100 ); break;
		}
	}
	else{
		for( int i = 0;i < (int)m_vtItemList.size();i++ ){
			ItemDataTable *pItemData = GetItemDataSoxDirectly( m_vtItemList[i].nItemSoxID );       
			if( m_vtItemList[i].bAccoutered )
			{			
				if( pItemData->bOrbToHpFillAndGone && g_FcWorld.GetHeroHandle( 0 )->GetHP() <= 0 )
				{
					SetItemEffectToHero( m_vtItemList[i].nItemSoxID, false );
					g_FcWorld.GetHeroHandle( 0 )->AddHP( g_FcWorld.GetHeroHandle( 0 )->GetOrbSpark() );
					m_vtItemList.erase( m_vtItemList.begin() + i );
					--i;
					//인터페이스 알리기
				}
			}
		}
	}
}

void CFcItemManager::NotifyUnitKill( CCrossVector *pCross )
{
	int nTotalProbbt = 0;
	for( unsigned int i = 0;i < m_vtItemList.size();i++ ){	
		nTotalProbbt += GetItemDataSoxDirectly( m_vtItemList[i].nItemSoxID )->nUnitKillBluePotionProbbt;
	}
	if( nTotalProbbt && nTotalProbbt > Random(_MAX_ITEM_PROBBT) ){	
		D3DXVECTOR3 Pos = pCross->GetPosition();
		Pos += (pCross->m_ZVector * 300);
		CreateItemToWorld( ITEM_BLUE_PORTION_S ,Pos );
	}
}

void CFcItemManager::SetItemEffectToHero( int nItemSoxID,bool bPlus  )
{
	HeroObjHandle Handle = g_FcWorld.GetHeroHandle();
	if( !CFcBaseObject::IsValid( Handle ) ) { return; }
	if( Handle == NULL ){ return; }

	int nSign = 0;
	(bPlus) ? nSign = 1: nSign = -1;
	ItemDataTable *pDataBuf = GetItemDataSoxDirectly(nItemSoxID);

	Handle->SetOrbSparkAdd( Handle->GetOrbSparkAdd() + (pDataBuf->nOrbAttackGainSpeed * nSign) );
	Handle->SetOrbSparkSplitPercent( Handle->GetOrbSparkSplitPercent() + (pDataBuf->nOrbSplit * nSign) );
	Handle->SetMoveSpeedAdd( Handle->GetMoveSpeedAdd() + (pDataBuf->nMoveSpeed * nSign) );
	Handle->SetMaxOrbSparkAdd( Handle->GetMaxOrbSparkAdd() + (pDataBuf->nOrbMaxAdd * nSign) );
	Handle->SetOrbAttackKeepSpeed( Handle->GetOrbAttackKeepSpeed() + (nSign * pDataBuf->fOrbAttackKeepTime) );
	Handle->SetSpecialAttackDistAdd( Handle->GetSpecialAttackDistAdd() + (nSign * pDataBuf->nSpecialAttackDistAdd ) );
	Handle->SetTrueOrbSparkAdd( Handle->GetTrueOrbSparkAdd() + (nSign * pDataBuf->nTrueOrbGainSpeed ));
	Handle->SetOrbSparkAddWithTrueOrb( Handle->GetOrbSparkAddWithTrueOrb() + (nSign * pDataBuf->nOrbFillWithTrueOrb ));
	Handle->SetSpecialAttackUseOrb( Handle->GetSpecialAttackUseOrb() + (nSign * pDataBuf->nSpecialAttackUseOrb) );
	Handle->SetOrbAttackPowerAdd( Handle->GetOrbAttackPowerAdd() + (nSign * pDataBuf->nOrbAttackPowerAdd) );
	Handle->SetTrailLengAdd( Handle->GetTrailLengAdd() + (nSign * pDataBuf->nTrailLengthAdd) );
	Handle->SetAvoidCriticalRate( Handle->GetAvoidCriticalRate() + (nSign * pDataBuf->nAvoidCriticalRate) );
	Handle->SetDashLengthAdd( Handle->GetDashLengthAdd() + (nSign * pDataBuf->nDashDist) );


	m_nGuadianAttackAdd += nSign * pDataBuf->nGuardianAttackPoint; //임시로 값 확인하려고
	g_FcWorld.SetGuardianTroopAttackAdd( nSign * pDataBuf->nGuardianAttackPoint );
	m_nGuadianDefenseAdd += nSign * pDataBuf->nGuardianDefensePoint;
	g_FcWorld.SetGuardianTroopDefenseAdd( nSign * pDataBuf->nGuardianDefensePoint );
	g_FcWorld.SetGuardianTroopMaxHPAdd( nSign * pDataBuf->nGuadianMaxHPAdd);
	g_FcWorld.SetGuardianTroopMoveSpeedAdd( nSign * pDataBuf->nGuadianSpeedAdd );	
	g_FcWorld.SetGuardianTrapDamageAdd( nSign * pDataBuf->nGuadianTrapDamage );	

	if(pDataBuf->nEnemyRangeProbbtAdd){
		g_FcWorld.SetEnemyRangeProbbtAdd( nSign * pDataBuf->nEnemyRangeProbbtAdd );
	}
	//g_FcWorld.SetGuardianRangeProbbtAdd( nSign * pDataBuf->nGuadianRangeProbbtAdd );	

	SetItemEffect( Handle,nItemSoxID , bPlus );
}



void CFcItemManager::SetItemEffect(GameObjHandle Handle,int nItemSoxID, bool bPlus )
{
	if(Handle == NULL) { return; }
	if( CFcBaseObject::IsValid( Handle ) == false ) { return; }

	int nSignal;
	(bPlus)? nSignal = 1 : nSignal = -1 ;
	ItemDataTable *pDataBuf = GetItemDataSoxDirectly(nItemSoxID);

	Handle->SetMaxHPAdd( Handle->GetMaxHPAdd() + (pDataBuf->nHPMaxAdd * nSignal) );	
	if(bPlus == false){ //아이템 효과를 뺐는데
		if(Handle->GetHP() > Handle->GetMaxHP()){ //현재 HP가 MAX보다 큰 상태면
			Handle->SetHP( Handle->GetMaxHP() ); //MAX HP랑 동일하게 만듦.
		}
	}

	Handle->SetAttackPowerAdd( Handle->GetAttackPowerAdd() + (pDataBuf->nAttackPoint* nSignal) );
	Handle->SetDefenseAdd( Handle->GetDefenseAdd() + (pDataBuf->nDefensePoint* nSignal) );
	Handle->SetCriticalAdd( Handle->GetCriticalAdd() + ( pDataBuf->nCriticalProbbt * nSignal) );
	Handle->SetGuardBreak( Handle->GetGuardBreak() + ( pDataBuf->nGuardBreak * nSignal ) );
	Handle->SetExpAdd( Handle->GetExpAdd() + ( pDataBuf->nExpGainSpeed * nSignal ) );
	Handle->SetAttackDistAdd( Handle->GetAttackDistAdd() + (pDataBuf->nWeaponReach * nSignal) );
	Handle->SetBossGuardBreak( Handle->GetBossGuardBreak() + (pDataBuf->nGuardBreakBoss * nSignal) );	
	Handle->SetDamageRatioAdd( Handle->GetDamageRatioAdd() + (pDataBuf->nDamageRatioAdd * nSignal)  );
	Handle->SetCriticalRatioAdd( Handle->GetCriticalRatioAdd() + (pDataBuf->nCriticalDamageRatio * nSignal) );	
	Handle->SetMaxHPAdd( Handle->GetMaxHPAdd(false) + (pDataBuf->nHPMaxAddInteger * nSignal), false );


}

void CFcItemManager::SetItemEffectObjList(  std::vector< GameObjHandle > &vtObjList,int nItemSoxID , bool bPlus )
{
	for( unsigned int i = 0;i < vtObjList.size();i++ )
	{
		SetItemEffect( vtObjList[i], nItemSoxID ,bPlus );
	}
}

void CFcItemManager::RestoreEnableEffect( ITEM_DURATION_INFO &DurData )
{
	if( DurData.vtObjList.size() )
	{
		for(unsigned int s = 0;s < DurData.vtObjList.size();s++) {		
			if( DurData.vtObjList[s] == NULL ){ continue; }
			if( CFcBaseObject::IsValid( DurData.vtObjList[s] ) == false){ continue; }

			DurData.vtObjList[s]->SetGodMode( false );
			DurData.vtObjList[s]->SetIgnoreCritical( false );
			DurData.vtObjList[s]->SetRevival( false );
			DurData.vtObjList[s]->SetAutoGuard( false );
			DurData.vtObjList[s]->SetSpecialCriticalRatio( false );	
			DurData.vtObjList[s]->SetRevivalCondition( 0 );
		}
	}
}

void CFcItemManager::ApplyEnableEffect()
{
	ItemDataTable *pItemData = NULL;
	bool bGodMode = false;
	bool bTrampleDamage = false;
	bool bIgnoreCritical = false;
	bool bEnableDash = true;
	bool bEnableJump = true;
	bool bRevival = false;
	bool bEnableDefense = true;
	bool bNotUseOrbSpecialAttack = false;
	bool bAlwaysSmallDamage = false;
	bool bGodModeInCharge = false;
	bool bAutoGuard = false;
	bool bAutoOrbAttack = false;
	bool bSpecialCriticalRatio = false;
	int  nRevivalHpConditon = 0;
	bool bOrbSparkMakeOrb = false;

	unsigned int i = 0;
	for(i = 0;i < m_vtItemList.size();i++ )
	{
		if( m_vtItemList[i].bAccoutered == false){ continue; } //장착하지 않은 장비면
		pItemData = GetItemDataSoxDirectly(	m_vtItemList[i].nItemSoxID );

		if( pItemData->bGodMode ){ bGodMode = true; }
		if( pItemData->bTrampleDamage ){ bTrampleDamage = true;  }
		if( pItemData->bIgnoreCritical ){ bIgnoreCritical = true; }
		if( pItemData->bEnableDash == false ){ bEnableDash = false; }
		if( pItemData->bEnableJump == false ){ bEnableJump = false;  }
		if( pItemData->bRevival ){ bRevival = true; }
		if( pItemData->bEnableDefense == false){ bEnableDefense = false; }
		if( pItemData->bNotUseOrbSpecialAttack ){ bNotUseOrbSpecialAttack = true; }
		if( pItemData->bAlwaysSmallDamage ) { bAlwaysSmallDamage = true; }
		if( pItemData->bGodModeInCharge ) { bGodModeInCharge = true; }
		if( pItemData->bAutoGuard ) { bAutoGuard = true; }
		if( pItemData->bAutoOrbAttack ) { bAutoOrbAttack = true;  }
		if( pItemData->bSpecialCriticalRatio ) {bSpecialCriticalRatio  = true; }
		if( pItemData->nRevivalHPCondition && nRevivalHpConditon == 0) { nRevivalHpConditon = pItemData->nRevivalHPCondition; }
		if( pItemData->bOrbSparkMakeOrb ) { bOrbSparkMakeOrb = true; }

	}

	for(i = 0;i < m_vtDurationItem.size();i++ )
	{
		pItemData = GetItemDataSoxDirectly(	m_vtDurationItem[i].nItemSoxID );
		if( pItemData->bGodMode ){ bGodMode = true; }
		if( pItemData->bTrampleDamage ){ bTrampleDamage = true; }
		if( pItemData->bIgnoreCritical ){ bIgnoreCritical = true; }
		if( pItemData->bEnableDash == false ){ bEnableDash = false; }
		if( pItemData->bEnableJump == false ){ bEnableJump = false; }
		if( pItemData->bRevival ){ bRevival = true; }
		if( pItemData->bEnableDefense == false){ bEnableDefense = false; }
		if( pItemData->bNotUseOrbSpecialAttack ){ bNotUseOrbSpecialAttack = true; }
		if( pItemData->bAlwaysSmallDamage ) { bAlwaysSmallDamage = true; }
		if( pItemData->bGodModeInCharge ) { bGodModeInCharge = true; }
		if( pItemData->bAutoGuard ) { bAutoGuard = true; }
		if( pItemData->bAutoOrbAttack ) { bAutoOrbAttack  = true; }
		if( pItemData->bSpecialCriticalRatio ) {bSpecialCriticalRatio  = true; }
		if( pItemData->nRevivalHPCondition && nRevivalHpConditon == 0) { nRevivalHpConditon = pItemData->nRevivalHPCondition; }
		if( pItemData->bOrbSparkMakeOrb ) { bOrbSparkMakeOrb = true; }
	}

	for(i = 0;i < CIT_LAST;i++ )
	{
		if( m_SetEffectManager[i].bEffect )
		{
			pItemData = GetItemDataSoxDirectly(	m_SetEffectManager[i].nSoxID );
			if( pItemData->bGodMode ){ bGodMode = true; }
			if( pItemData->bTrampleDamage ){ bTrampleDamage = true; }
			if( pItemData->bIgnoreCritical ){ bIgnoreCritical = true; }
			if( pItemData->bEnableDash == false ){ bEnableDash = false; }
			if( pItemData->bEnableJump == false ){ bEnableJump = false; }
			if( pItemData->bRevival ){ bRevival = true; }
			if( pItemData->bEnableDefense == false){ bEnableDefense = false; }
			if( pItemData->bNotUseOrbSpecialAttack ){ bNotUseOrbSpecialAttack = true; }
			if( pItemData->bAlwaysSmallDamage ) { bAlwaysSmallDamage = true; }
			if( pItemData->bGodModeInCharge ) { bGodModeInCharge = true; }
			if( pItemData->bAutoGuard ) { bAutoGuard = true; }
			if( pItemData->bAutoOrbAttack ) { bAutoOrbAttack  = true; }
			if( pItemData->bSpecialCriticalRatio ) {bSpecialCriticalRatio  = true; }
			if( pItemData->nRevivalHPCondition && nRevivalHpConditon == 0) { nRevivalHpConditon = pItemData->nRevivalHPCondition; }
			if( pItemData->bOrbSparkMakeOrb ) { bOrbSparkMakeOrb = true; }			
		}
	}

	if( CFcBaseObject::IsValid( g_FcWorld.GetHeroHandle() ) )
	{
		g_FcWorld.GetHeroHandle()->SetInvincible( bGodMode );
		g_FcWorld.GetHeroHandle()->EnableInphyTrampleDamage( bTrampleDamage );
		g_FcWorld.GetHeroHandle()->SetIgnoreCritical( bIgnoreCritical );
		g_FcWorld.GetHeroHandle()->SetEnableDash( bEnableDash );
		g_FcWorld.GetHeroHandle()->SetEnableJump( bEnableJump );
		g_FcWorld.GetHeroHandle()->SetRevival( bRevival );
		g_FcWorld.GetHeroHandle()->SetEnableDefense( bEnableDefense );
		g_FcWorld.GetHeroHandle()->SetNotUseOrbSpecialAttack( bNotUseOrbSpecialAttack );
		g_FcWorld.GetHeroHandle()->SetAlwaysSmallDamage( bAlwaysSmallDamage );
		g_FcWorld.GetHeroHandle()->SetChargeModeGod( bGodModeInCharge );

		if( g_FcWorld.GetHeroHandle()->GetHeroClassID() != CFcGameObject::Class_ID_Hero_VigkVagk ) {
			g_FcWorld.GetHeroHandle()->SetAutoGuard( bAutoGuard );
		}

		g_FcWorld.GetHeroHandle()->SetAutoOrbAttack( bAutoOrbAttack );
		g_FcWorld.GetHeroHandle()->SetSpecialCriticalRatio( bSpecialCriticalRatio );	
		g_FcWorld.GetHeroHandle()->SetRevivalCondition( nRevivalHpConditon );
		if( g_FcWorld.GetAbilityMng() ){		
			if( bOrbSparkMakeOrb ){
				g_FcWorld.GetAbilityMng()->SetOrbGen( 0, 30 );
			}
			else{
				g_FcWorld.GetAbilityMng()->SetOrbGen( 1, 100 );
			}
		}		
	}
}

bool CFcItemManager::IsOnceEquipItem( int nItemSoxID )
{
	for(unsigned int i = 0;i < m_vtOnceEquipSoxID.size();i++)
	{
		if( m_vtOnceEquipSoxID[i] == nItemSoxID )
			return true;
	}
	return false;
}

void CFcItemManager::SetAccoutered(int nListIndex, bool bAccoutered) //장착시 호출
{
	BsAssert(m_vtItemList.size() > (DWORD)nListIndex);	
	HeroObjHandle HeroHandle = CFcWorld::GetInstance().GetHeroHandle();
	ItemDataTable *pItemData = GetItemData( nListIndex );
	int nSoxID = m_vtItemList[nListIndex].nItemSoxID;

	if(bAccoutered && IsOnceEquipItem( nSoxID ) )
		return;

	if( bAccoutered ) //장착하라는 신호를 받았고
	{
		if( m_vtItemList[nListIndex].bAccoutered == FALSE ){ //아직 장착하지 않은 상태라면
			SetItemEffectToHero( m_vtItemList[nListIndex].nItemSoxID, true ); //착용후 효과적용
		}
	}
	else if( m_vtItemList[nListIndex].bAccoutered == TRUE )
	{//해체 신호를 받았고
		if( m_vtItemList[nListIndex].bStopEffect == TRUE ) //실시간 효과가 적용된 상태이므로
		{
			if( pItemData->nHPLowerAddOrbGainSpeed ) {
				HeroHandle->SetOrbSparkAdd( HeroHandle->GetOrbSparkAdd() - pItemData->nHPLowerAddOrbGainSpeed );
			}
			m_vtItemList[nListIndex].bStopEffect = FALSE;
		}
		SetItemEffectToHero( m_vtItemList[nListIndex].nItemSoxID, false );
	}
	m_vtItemList[nListIndex].bAccoutered = bAccoutered;	
	ApplyEnableEffect();
}

bool CFcItemManager::IsItemDuplication(int nItemSoxID)
{
	for( unsigned int i = 0;i < m_vtItemList.size();i++)
	{
		if( m_vtItemList[i].nItemSoxID == nItemSoxID)
			return true;
	}
	return false;
}

_ACCOUTERING_MSG CFcItemManager::CheckAccoutering(int nListIndex)
{
	BsAssert( 0 <= nListIndex && "Check Equip index less than 0" );
	BsAssert( nListIndex < (int)m_vtItemList.size() && "Check Equip index bigger than size" );

	int nItemSoxID = m_vtItemList[nListIndex].nItemSoxID;
	ItemDataTable	*pItemData = GetItemData( nListIndex );
	if( (g_FcWorld.GetHeroHandle()->GetLevel() + 1 ) < (pItemData->nLevelMinimum - GetItemEquipLevelDown()) ){
		return ACCT_MSG_FAIL_LEVEL;
	}

	if( IsOnceEquipItem( nItemSoxID ) ) {	
		return ACCT_MSG_ONE_EQUIP_THIS_MISSION;
	}
	return ACCT_MSG_ABLE;
}

int CFcItemManager::GetItemEquipLevelDown()
{
	int nTotalDownLevel = 0;
	for( unsigned int i = 0;i < m_vtItemList.size();i++)
	{
		if( m_vtItemList[i].bAccoutered ){		
			nTotalDownLevel += GetItemDataSoxDirectly( m_vtItemList[i].nItemSoxID )->nItemEquipLevelDown;
		}
	}
	return nTotalDownLevel;
}


int CFcItemManager::GetHeroWeaponSkinIndex()
{
	for(unsigned int i = 0;i < m_vtItemList.size();i++)
	{
		ItemDataTable *Data = GetItemData(i);
		if(m_vtItemList[i].bAccoutered)
		{
			if(Data->nItemType == ITP_WEAPON)
			{
				BsAssert( m_SkinList[m_vtItemList[i].nItemSoxID] != -1 );
				return m_SkinList[m_vtItemList[i].nItemSoxID];
			}
		}
	}
	return -1;
}


void CFcItemManager::DeleteItem(int nIndex)
{
	BsAssert( nIndex < (int)m_vtItemList.size());// && "아이템 지우는 인덱스 사이즈 초과");
	BsAssert( nIndex >= 0);// && "아이템 지우는 인덱스 사이즈 미달" );

	m_vtItemList.erase( m_vtItemList.begin() + nIndex );
}

void CFcItemManager::DeleteAllNewItem()
{
	std::vector< HeroEquip >::iterator it = m_vtItemList.begin();
	while( it != m_vtItemList.end() )
	{
		if((*it).bNew){			
			m_vtItemList.erase( it );
			continue;
		}
		it++;
	}
}

void CFcItemManager::UpdateItemEffect()
{
	for(unsigned int i = 0;i < m_vtItemList.size();i++){
		ItemDataTable *pItemData = GetItemDataSoxDirectly( m_vtItemList[i].nItemSoxID );
		if( m_vtItemList[i].bAccoutered ) //특정 조건이 없는 아이템이어야 함.
		{
			SetItemEffectToHero( m_vtItemList[i].nItemSoxID, true );
			if( pItemData->bStartFullOrb ) {
				g_FcWorld.GetHeroHandle()->SetOrbSpark( g_FcWorld.GetHeroHandle()->CalcMaxOrbSpark() );
			}
		}
	}
	GiveSetEffect();
	ApplyEnableEffect();
	g_FcWorld.GetHeroHandle()->SetHP( g_FcWorld.GetHeroHandle()->GetMaxHP() );

}




//------------------------------------------------------------------------------------------


CFcItem::CFcItem( CCrossVector *pCross )
	: CFcBaseObject( pCross )
{
	m_nItemTableIndex = -1;
	m_bEaten = false;
	m_nLifeCount = 0;
}

CFcItem::~CFcItem( )
{
	
}
bool CFcItem::IsNAitem( int nItemTableIndex )
{
	switch( nItemTableIndex )
	{
	case	ITEM_KNIGHT_MIND: return true;
	case	ITEM_RAGE: return true;
	case	ITEM_ANGEL_BLESSING: return true;
	case	ITEM_GODDESS_DIVINE: return true;
	case	ITEM_CAVALRY_SECRET_BOOK: return true;
	case	ITEM_BAG: return true;
	case	ITEM_WEAPON_BAG: return true;
	case	ITEM_HORSE_STATUE: return true;
	case	ITEM_WHITE_PORTION_S: return true;
	case	ITEM_WHITE_PORTION_M: return true;
	case	ITEM_WHITE_PORTION_L: return true;
	case	ITEM_GUARD_CHARM_LV5: return true;
	case	ITEM_GUARD_CHARM_LV6: return true;
	case	ITEM_GUARD_CHARM_LV7: return true;
	case	ITEM_GUARD_CHARM_LV8: return true;
	case	ITEM_GIANT_BANGLE_LV4: return true;
	case	ITEM_GIANT_BANGLE_LV5: return true;
	case	ITEM_GIANT_BANGLE_LV6: return true;
	case	ITEM_GIANT_BANGLE_LV7: return true;
	case	ITEM_GIANT_BANGLE_LV8: return true;
	case	ITEM_HEALTH_BEAD_LV6: return true;
	case	ITEM_HEALTH_BEAD_LV7: return true;
	case	ITEM_HEALTH_BEAD_LV8: return true;
	case	ITEM_GRIFFON_FEATHER_LV1: return true;
	case	ITEM_GRIFFON_FEATHER_LV2: return true;
	case	ITEM_VALOR_FLAG_LV4: return true;
	case	ITEM_VALOR_FLAG_LV5: return true;
	case	ITEM_VALOR_FLAG_LV6: return true;
	case	ITEM_VALOR_FLAG_LV7: return true;
	case	ITEM_VALOR_FLAG_LV8: return true;
	/*case	ITEM_WIND_MANTLE_LV1: return true;
	case	ITEM_WIND_MANTLE_LV2: return true;
	case	ITEM_WIND_MANTLE_LV3: return true;*/
	case	ITEM_WIND_MANTLE_LV4: return true;
	case	ITEM_WIND_MANTLE_LV5: return true;
	case	ITEM_SADDLE_LV1: return true;
	case	ITEM_SADDLE_LV2: return true;
	case	ITEM_SADDLE_LV3: return true;
	case	ITEM_SADDLE_LV4: return true;
	case	ITEM_SADDLE_LV5: return true;
	case	ITEM_SADDLE_LV6: return true;
	case	ITEM_SADDLE_LV7: return true;
	case	ITEM_SADDLE_LV8: return true;
	case	ITEM_DANCER_GLOVE_LV3: return true;
	}
	return false;
}

int CFcItem::Initialize( int nSkinIndex , int nItemTableIndex ,int nItemLifeTime)
{

	if( IsNAitem(nItemTableIndex) )
	{
		char szNA_ItemError[128];
		sprintf( szNA_ItemError,"This is a N/A Item [%d]", nItemTableIndex );
		BsAssert(0 && szNA_ItemError);
		DebugString( szNA_ItemError );
	}
	if( nSkinIndex == -1 )
	{
		char szSkinLoadErr[128];
		sprintf( szSkinLoadErr,"%d index item skin is not Loaded", nItemTableIndex );
		BsAssert(0 && szSkinLoadErr);
	}

	m_Cross.m_PosVector.y += _ADD_ITEM_LAND_HEIGHT;

	m_nItemTableIndex = nItemTableIndex;
	m_nLifeTime = nItemLifeTime;

	if( ITEM_CHURCH_KNIGHT_SPEAR <= nItemTableIndex && nItemTableIndex <= ITEM_GLORIA ){	
		m_Cross.m_PosVector.y += _ADD_WEAPON_ITEM_LAND_HEIGHT;

		if( g_FcWorld.GetHeroHandle())
		{
			int nHero = (int)g_FcWorld.GetHeroHandle()->GetHeroClassID();
			switch(nHero)
			{
			case CFcGameObject::Class_ID_Hero_Aspharr:		
			case CFcGameObject::Class_ID_Hero_Klarrann:
			case CFcGameObject::Class_ID_Hero_VigkVagk:
			case CFcGameObject::Class_ID_Hero_Myifee:
			case CFcGameObject::Class_ID_Hero_Tyurru:
				m_Cross.RotatePitch( _ADD_ITEM_WEAPON_RORATE );
				break;
			}
		}
	}
	return CFcBaseObject::Initialize( nSkinIndex );
	
}


bool CFcItem::IsConsumeItem( const int &nItemTableIndex )
{
	switch(nItemTableIndex)
	{
		case ITEM_RED_PORTION_L:
		case ITEM_RED_PORTION_M:
		case ITEM_RED_PORTION_S:
		case ITEM_BLUE_PORTION:
		case ITEM_BLUE_PORTION_S:
		case ITEM_GRAIL:
		case ITEM_WEAPON_MARK:
		case ITEM_ARMOR_MARK:
		case ITEM_FLYING_SHOES_MARK:
		case ITEM_EXP_2_MARK:
		case ITEM_BONUS_EXP_2_MARK:
		case ITEM_WHISTLE:
		case ITEM_BENUS_STATUE:
			return true;
	}
	return false;
}

void CFcItem::Process()
{
	if( IsPlayRealMovie() || IsPlayEvent() ) { //리얼무비 실행하고 있으면 먹게하지 말고 
		return;
	}

	m_nLifeCount++;
	if( m_nLifeCount > m_nLifeTime ){ //시간 다된 아이템은 없애자
		Delete();
		return;
	}

	int nRemainFrame = m_nLifeTime - m_nLifeCount;
	if( nRemainFrame <= _START_ALPHA_FRAME)
	{
		g_BsKernel.SendMessage( m_nEngineIndex, BS_ENABLE_OBJECT_ALPHABLEND, 1 );
		float fAlphaWeight = (float)nRemainFrame / (float)_START_ALPHA_FRAME;
		g_BsKernel.SendMessage( m_nEngineIndex, BS_SET_OBJECT_ALPHA, ( DWORD )&fAlphaWeight );
	}

	if( m_bEaten == false )
	{		
		float fDistance;
		GameObjHandle HeroHandle;

		// 2P 처리 안함
		HeroHandle = CFcWorld::GetInstance().GetHeroHandle();
		if( HeroHandle && !HeroHandle->IsDie() )
		{
			fDistance = D3DXVec3Length( &( HeroHandle->GetPos() - m_Cross.m_PosVector ) );
			if( fDistance < 100.0f && m_nLifeCount > _ABLE_EATING_TIME)
			{					
				m_bEaten = true;
				Delete();
				g_pSoundManager->PlaySystemSound(SB_COMMON,"SYS_ITEM");
				g_FcItemManager.AddItem( m_nItemTableIndex, false, true );

				g_FcWorld.PlaySimpleFx( FX_ITEM_DISAPPEAR, &m_Cross );


				if( IsConsumeItem( m_nItemTableIndex ) ) {				
					g_InterfaceManager.SetItemReport( g_FcItemDescTable.GetItemName(m_nItemTableIndex) );

				}
				else{
					char szItemInfomation[256] = "";
					char szTempStr[128] = "";	// mruete: prefix bug 577: added safe default
					g_TextTable->GetText(_TEX_GET_ITEM, szTempStr, _countof(szTempStr));
					sprintf( szItemInfomation, szTempStr, g_FcItemDescTable.GetItemName(m_nItemTableIndex), szTempStr );
					g_InterfaceManager.SetItemReport( szItemInfomation );
				}
			}
			else{
				m_Cross.RotateYawByWorld( _ITEM_ROTATION_SPEED );
				if(!(GetProcessTick() % 100 ) ){
					g_FcWorld.PlaySimpleFx( FX_ITEM_APPEAR, &m_Cross );
				}
			}
		}
	}
}

bool CFcItem::Render()
{
	if( IsPlayRealMovie() || IsPlayEvent() ) {
		return false;
	}

	if( m_bEaten == false )  {	
		return CFcBaseObject::Render();
	}
	return false;
}