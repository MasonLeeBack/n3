#pragma once
#include "fcbaseobject.h"
#include "FcGlobal.h"
#include ".\\Data\\ItemDefinitions.h"

class CFcHeroObject;
class BFileStream;
struct ItemDataTable;
class CFcItem;

typedef CSmartPtr<CFcHeroObject> HeroObjHandle;
typedef CSmartPtr<CFcGameObject> GameObjHandle;
typedef CSmartPtr< CFcItem > ItemHandle;

enum _ACCOUTERING_MSG
{
	ACCT_MSG_ABLE,		 //���� ������ ���
	ACCT_MSG_FAIL_LEVEL, //������ ���Ƽ� ���� ���ϴ� ���
	ACCT_MSG_ONE_EQUIP_THIS_MISSION,
};

class CFcItem : public CFcBaseObject
{
public:
	CFcItem( CCrossVector *pCross );
	virtual ~CFcItem(void);
	int Initialize( int nSkinIndex , int nItemTableIndex , int nItemLifeTime);
	void Process();
	bool Render();
	bool IsConsumeItem( const int &nItemTableIndex );
	bool IsNAitem( int nItemTableIndex );

protected:	
	int			m_nItemTableIndex;
	bool		m_bEaten;
	short		m_nLifeTime;
	int			m_nFXObjectIndex;
	int			m_nLifeCount;

};

class CFcItemManager
{
public:
	struct ITEM_DURATION_INFO
	{
		int	  nItemSoxID;
		DWORD nPastTime;
		DWORD nDestTime;
		int   nEftInterval;
		std::vector< GameObjHandle >vtObjList;
		D3DXVECTOR3 Pos;

		ITEM_DURATION_INFO()
		{
			nItemSoxID = -1;
			nPastTime = 0;
			nDestTime = 0;
			nEftInterval = 0;
		}
	};
	struct ITEM_DROP_INFO
	{
		int nItemSoxID;
		int nProbt;
	};

	enum EFFECT_SET_ITEM_TYPE
	{
		CIT_NONE = -1,
		CIT_KNIGHT_MIND,
		CIT_RAGE,
		CIT_ANGEL_BLESSING,
		CIT_GODDEST_DIVINE,
		CIT_LAST
	};

	struct EFFECT_SET_ITEM_MANAGE
	{
		int nSoxID;
		bool bEffect;
		EFFECT_SET_ITEM_MANAGE()
		{
			nSoxID = -1;
			bEffect = false;
		}
		void Init()
		{
			bEffect = false;
		}
	};

public:

	CFcItemManager();
	~CFcItemManager();
	void LoadItemData();
	void LoadLocalItemData();		//loadLocalSox();
	void Finalize();
	void FinishStage();
	void CreateItemToWorld( int nUnitSoxID, int nLevel,CCrossVector *pCross ); //ĳ���Ͱ� ������ �� �Լ� ȣ��
	void CreateItemToWorld( int nItemSoxID,D3DXVECTOR3 &Pos,int nDropTableID = -1); //�������� ���� �������� �̰ɷ� ����	
	void AddItem( int nItemSoxID, BOOL bAccounded, BOOL bNew , BOOL bAddSetPrize = FALSE );		//���ΰ��� �������� �����Ÿ� �̳��� ������ �߻��ϸ�, �̰����� ������ ���� ���������� �׳� �ٷ� ����������������� �Ǵ����ش�.
	void AddItem( HeroEquip Equip )						{ AddItem(Equip.nItemSoxID, Equip.bAccoutered, Equip.bNew); }
	void SetAccoutered(int nListIndex, bool bAccoutered);


	ItemDataTable	*GetItemData( int nIndex );
	ItemDataTable	*GetItemDataSoxDirectly( int nItemSoxID );
	int GetHeroWeaponSkinIndex();
	
	_ACCOUTERING_MSG	CheckAccoutering(int nListIndex);

	BOOL	IsAccoutered(int nListIndex)					{ BsAssert(m_vtItemList.size() > (DWORD)nListIndex); return m_vtItemList[nListIndex].bAccoutered; }
	BOOL	IsNewItem(int nListIndex)						{ BsAssert(m_vtItemList.size() > (DWORD)nListIndex); return m_vtItemList[nListIndex].bNew; }

	HeroEquip*	GetHeroEquip(int nListIndex)				{ BsAssert(m_vtItemList.size() > (DWORD)nListIndex); return &m_vtItemList[nListIndex]; }

	void SetNewItem(int nListIndex, bool bNew)			{ BsAssert(m_vtItemList.size() > (DWORD)nListIndex); m_vtItemList[nListIndex].bNew = bNew; }
	void UpdateSort();
	void AddItemEffect( GameObjHandle Handle , int nItemSoxID, bool bPlus );
	void AddItemEffectRange( D3DXVECTOR3 *pPos, float fAreaScale ,int nItemSoxID, bool bPlus );
	void AddItemEffectToHero( HeroObjHandle Handle , int nItemSoxID, bool bPlus );
	void Process();

	int  GetItemCount(){ return (int)m_vtItemList.size(); }
	char *GetItemName(int nListIndex);
	char *GetItemDesc(int nListIndex);
	
	int  GetValueByPer(float fPer,int nVal);
	bool IsPortion(int nListIndex);
	void GetItemProbbtAdd(float &fItemAddProbbt, float &fPortionProbbt);

	void SetItemEffect( GameObjHandle Handle,int nItemSoxID , bool bPlus );
	void SetItemEffectObjList( std::vector< GameObjHandle > &vtObjList,int nItemSoxID, bool bPlus  );
	void SetItemEffectToHero( int nItemSoxID, bool bPlus );
	void ApplyEnableEffect();
	void RestoreEnableEffect( ITEM_DURATION_INFO &DurData );
	bool IsItemDuplication(int nItemSoxID);
	void DeleteItem(int nIndex); //�ε����� ���ؼ� ����µ� �ε����� �и��Ƿ� ����
	void DeleteAllNewItem(); //�ش� �̼ǿ��� ���� ������ �鸸 ���ش�.
	void LoadHeroItemSkin();  //Ư�� ����� ������

	int GetSkinIndex( int nItemSoxID );
	void NotifyUnitKill( CCrossVector *pCross );
	void NotifyRevival();
	float GetDurationTimeAdd();
	float GetHPAddWithGuadian();

	void UpdateItemEffect();
	bool IsHeroConditionOK( int nSoxID );
	bool IsItemConditionOK( int nSoxID );
	void SetDrawItemInfo(bool bDraw){ m_bDrawItemInfo = bDraw; }

	int GetDurationItemCount(){ return (int)m_vtDurationItem.size(); }
	ITEM_DURATION_INFO *GetDurationItem( int nIndex ){ BsAssert(nIndex >= 0); BsAssert(nIndex < (int)m_vtDurationItem.size()); return &m_vtDurationItem[nIndex]; }	
	void NotifyAutoOrbAttack();
	void AddItemDropProbbt( float nPer );

protected:
	void LoadBasicItemSkin(); //ó���� �ε��س��� ������	
	void UnloadBasicItemSkin();
	void UnloadHeroItemSkin();
	void VerifyClear();
	void ProcessDurationItem();
	void ProcessEquippedItem();
	bool IsHeroHPLower();
	int GetBluePortionSmallFillWithHP();
	int GetBluePortionBigFillWithHP();
	int GetItemEquipLevelDown();
	bool CheckOnlyDropBigRedPortion();
	bool CheckOnlyDropSmallRedPortion();
	bool CheckDropChangeRedToBlue();
	bool IsComsumeItem( int nItemSoxID );
	int GetItemLifeTime();
	bool IsExistItem( int nSoxID );
	void GiveSetEffect();
	void MakeSetPrizeItem();
	bool IsSetSuccess(int nSetItemIndex);
	void CreateItemByDropTable( int nDropTable ,D3DXVECTOR3 &Pos); //�������� ���� �������� �̰ɷ� ����
	bool IsOnceEquipItem( int nItemSoxID );

	bool IsSetEffectNow( int nSetItemIndex );
	int GetEffectSetItemType( int nSoxID );
	bool IsUseGuardCharm();

protected:
	std::vector< HeroEquip >m_vtItemList;	
	std::vector< ITEM_DURATION_INFO >m_vtDurationItem;
	std::vector< int >m_SkinList;
	std::vector< int >m_vtOnceEquipSoxID;

	bool						m_bDrawItemInfo;
	EFFECT_SET_ITEM_MANAGE		m_SetEffectManager[CIT_LAST];

	int			m_nGuadianAttackAdd;
	int			m_nGuadianDefenseAdd;

	float		m_fItemProbbtAdd;

	


};

extern CFcItemManager		g_FcItemManager;
