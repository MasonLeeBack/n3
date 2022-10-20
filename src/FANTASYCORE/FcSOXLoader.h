#pragma once

#include "Singleton.h"

class BStream;
struct UnitDataInfo
{
	char cUnitName[ 64 ];
	char cSkinDir[ 64 ];
	char cSkinFileName[ 64 ];
	int cSkinVariationNum;
	char cShadowSkinName[ 64 ];
	char cAniName[ 64 ];
	int nMaxHP;
	int	nAttackPoint;
	int nDefensePoint;
	int nUnitType;
	int nUnitSize;
	char cASFileName[ 64 ];
	char cUnitInfoFileName[ 64 ];
	int nSubJobSOXIndex;
	int nUnitHeight;
	char cLevelTableFileName[ 64 ];
	int nTroopType;
	int	nBillboardSize;
	int nMinScale;
	int nMaxScale;
	int nMaxLevel;
	int nLevelStartOffset[4];		// _A, _B, _C, _D 4단계로 구분됨
	bool bUseLevel;
	int nSoundAmor;
	int nSoundWeapon;
	float fWeight;
	char cWBKakegoe[ 64 ];
	char cWBCrowd[ 64 ];
	short nCrowdUnitType;
	char cBillboardName[4][ 64 ];	// 레벨 별로 빌보드 따로 로딩

};

class CUnitSOXLoader : public CSingleton<CUnitSOXLoader>
{
public:
	CUnitSOXLoader(void);
	virtual ~CUnitSOXLoader(void);

	int Load();
	void Clear();

protected:
	int m_nUnitDataCount;
	UnitDataInfo *m_pUnitData;

public:
	UnitDataInfo *GetUnitData(int nIndex) { return m_pUnitData+nIndex; }
	int GetUnitDataCount() { return m_nUnitDataCount; }
	int GetUnitType( int nIndex ) { return m_pUnitData[ nIndex ].nUnitType; }
	int GetAttackPoint( int nIndex ) { return m_pUnitData[ nIndex ].nAttackPoint; }
	int GetDefensePoint( int nIndex ) { return m_pUnitData[ nIndex ].nDefensePoint; }
	char* GetASFileName( int nIndex ) { return (m_pUnitData+nIndex)->cASFileName; }
	char* GetPartsFileName( int nIndex ) { return (m_pUnitData+nIndex)->cUnitInfoFileName; }
	char* GetLevelTableFileName( int nIndex ) { return (m_pUnitData+nIndex)->cLevelTableFileName; }
	int GetTroopType( int nIndex ) { return (m_pUnitData+nIndex)->nTroopType; }
	char* GetBillboardName( int nIndex, int nLevel ) { BsAssert( nLevel>=0 && nLevel<4 ); return (m_pUnitData+nIndex)->cBillboardName[nLevel]; }
	int GetBillboardSize( int nIndex ) { return (m_pUnitData+nIndex)->nBillboardSize; }
	int GetMinScale( int nIndex ) { return (m_pUnitData+nIndex)->nMinScale; }
	int GetMaxScale( int nIndex ) { return (m_pUnitData+nIndex)->nMaxScale; }
	int GetMaxLevel( int nIndex ) { return (m_pUnitData+nIndex)->nMaxLevel; }
	int GetLevelStartOffset( int nIndex, int nLevel )	{ return (m_pUnitData+nIndex)->nLevelStartOffset[nLevel]; }
	bool IsUseLevel( int nIndex )	{ return (m_pUnitData+nIndex)->bUseLevel; }
	float GetWeight( int nIndex ) { return (m_pUnitData+nIndex)->fWeight; }
};


#define MAX_NPC_LEVEL		11

class CLevelTableLoader : public CSingleton<CLevelTableLoader>
{
public:
	CLevelTableLoader();
	virtual ~CLevelTableLoader();

	void Load();
	void Clear();

	struct IntegerValue {
		union {
			int nValue[MAX_NPC_LEVEL+1];
			float fValue[MAX_NPC_LEVEL+1];
		};
	};
	
protected:
	std::vector< IntegerValue > m_AttackData[2];
	std::vector< IntegerValue > m_DefenseData[3];
	std::vector< IntegerValue > m_HPData[2];
	std::vector< IntegerValue > m_CriticalData[2];
	std::vector< IntegerValue > m_CriticalRatioData[2];
	std::vector< IntegerValue > m_OrbSparkData;
	std::vector< IntegerValue > m_GiveExpData;
	std::vector< IntegerValue >	m_DefenseProbData[3];
	std::vector< IntegerValue > m_LevelMatchingData;

	void LoadTable( BStream *pStream, std::vector<IntegerValue> *pVecList, int nOffset = 0, int nTableCount = MAX_NPC_LEVEL );

public:
	int GetAttackPoint( int nUnitIndex, int nLevel, bool bFriend );
	int GetDefensePoint( int nUnitIndex, int nLevel, bool bFriend );
	int GetMaxHP( int nUnitIndex, int nLevel, bool bFriend );
	int GetCritical( int nUnitIndex, int nLevel, bool bFriend );
	float GetCriticalRatio( int nUnitIndex, int nLevel, bool bFriend );
	int GetOrbSparkPoint( int nUnitIndex, int nLevel );
	int GetGiveExpPoint( int nUnitIndex, int nLevel );
	int GetDefenseProb( int nUnitIndex, int nLevel, int nType );
	int GetMatchingLevel( int nUnitIndex, int nLevel );
	float GetTrueOrbDamageAttenuationRatio( int nUnitIndex, int nLevel );


//	void SetAttackPoint( int nUnitIndex, int nLevel, int nAttackPoint );
//	void SetDefensePoint( int nUnitIndex, int nLevel, int nDefencePoint );
//	void SetMaxHP( int nUnitIndex, int nLevel, int nMaxHP);
};

#define MAX_HERO_LEVEL 9

class CHeroLevelTableLoader : public CSingleton<CHeroLevelTableLoader>
{
public:
	CHeroLevelTableLoader();
	virtual ~CHeroLevelTableLoader();

	int Load();

	void Clear();

	struct IntegerValue {
		int nValue[MAX_HERO_LEVEL];
	};

protected:
	std::vector<IntegerValue> m_MoveSpeedData;
	std::vector<IntegerValue> m_SlotCountData;
	std::vector<IntegerValue> m_OrbDecreaseData;
	std::vector<IntegerValue> m_GuardianDefenseData;
	std::vector<IntegerValue> m_GuardianHPData;
	std::vector<IntegerValue> m_GuardianControlTroopCountData;
	std::vector<IntegerValue> m_GuardianControlTroopUnitCountData;
	std::vector<IntegerValue> m_AttackRangeData;

public:
	float GetMoveSpeed( int nUnitIndex, int nLevel );
	int GetSlotCount( int nUnitIndex, int nLevel );
	int GetOrbDecrease( int nUnitIndex, int nLevel );
	int GetGuardianDefensePoint( int nUnitIndex, int nLevel );
	int GetGuardianMaxHP( int nUnitIndex, int nLevel );
	int GetGuardianControlTroopCount( int nUnitIndex, int nLevel );
	int GetGuardianControlTroopUnitCount( int nUnitIndex, int nLevel );
	int GetAttackRange( int nUnitIndex, int nLevel );

};

enum FC_ITEM_PARAM_TYPE
{
	TYPE_NONE = -1,
	TYPE_DURATIONTIME,
	TYPE_EFFECTINTERVAL,
	TYPE_EFFECTAREA,
	TYPE_HP,
	TYPE_ORB,
	TYPE_HPMAXADD,
	TYPE_ORBMAXADD,
	TYPE_ATTACKPOINT,
	TYPE_DEFENSEPOINT,
	TYPE_CRITICAL,
	TYPE_GUARDBREAK,
	TYPE_ITEMGAINPROBBT,
	TYPE_EXPGAINSPEED,
	TYPE_ORBGAINSPEED,
	TYPE_MOVESPEED,
	TYPE_TRAPDAMAGE,
	TYPE_SIEGEDAMAGE,
	TYPE_PIXISDAMAGE,
	TYPE_WEAPONREACH,
	TYPE_DEATHPROBBT,
	TYPE_GUARDIAN_ATTACKPOINT,
	TYPE_POTIONPROBBT,
	TYPE_LVUP,
	TYPE_ORBSPARKFREE,
	TYPE_GODMODE,
	TYPE_ITEMBAG,
	TYPE_GUARDIAN_DEFENSE,
	TYPE_ARROWDEFENSE,
	TYPE_DASHDIST,
	TYPE_ORBSPLIT,
	TYPE_ORBATTACKKEEPTIME,
	TYPE_INPHYYTRAMPLEDAMAGE,
	TYPE_REBORN,
	TYPE_EQUIPDUP,
	TYPE_GUARDIAN_MAXHP,
	TYPE_GUARDIAN_SPEEDADD,
	TYPE_GUARDIAN_HPADD,
	TYPE_GUARDBREAK_BOSS,
	TYPE_IGNORECRITICAL,
	TYPE_ENABLEDASH,
	TYPE_DAMAGERATIOADD,
	TYPE_ENABLEJUMP,
	TYPE_CRITICALDAMAGERATIO,
	TYPE_SPECIALATTACKCRITICALPROBBT,
	TYPE_REVIVAL,
	TYPE_ENABLEDEFENSE,
	TYPE_NOTUSEORBSPECIALATTACK,
	TYPE_SPECIALATTACKDISTADD,
	TYPE_ALWAYSSMALLDAMAGE,
	TYPE_TRUEORBGAINSPEED,
	TYPE_EXPADD,
	TYPE_HPMAXADD_INTEGER,
	TYPE_HP_CONDITION_LOWER_PER,
	TYPE_UNITKILL_BLUEPOTION_PROBBT,
	TYPE_ORBTOHP_FILLANDGONE,
	TYPE_DURATIONTIME_ADD,
	TYPE_HP_WITH_GUARDIAN,
	TYPE_GODMODE_IN_CHARGE,
	TYPE_AUTO_GUARD,
	TYPE_START_FULL_ORB,
	TYPE_SMALL_BLUEPOTION_FILL_HP,
	TYPE_BIG_BLUEPOTION_FILL_HP,
	TYPE_ORB_FILL_WITH_TRUEORB,
	TYPE_ONLY_DROP_BIG_RED_PORTION,
	TYPE_SPECIAL_ATTACK_USE_ORB,
	TYPE_ORB_ATTACK_POWER_ADD,
	TYPE_TRAIL_LENGTH_ADD,
	TYPE_AUTO_ORB_ATTACK,
	TYPE_SPECIAL_CRITICAL_RATIO,
	TYPE_ONLY_DROP_SMALL_RED_PORTION,
	TYPE_DROP_CHANGE_RED_TO_BLUE,
	TYPE_CONSUME_ITEM_FILLHP,
	TYPE_ITEM_EQUIP_LEVEL_DOWN,
	TYPE_HP_LOWER_ADD_ORB_GAIN_SPEED,
	TYPE_ITEM_LIFE_TIME_ADD,
	TYPE_ITEM_SLOT_COUNT_ADD,
	TYPE_AVOID_CRITICAL_RATE,
	TYPE_REVIVAL_HP_CONDITION,
	TYPE_ENEMY_RANGE_PROBBT_ADD,
	TYPE_GUDIAN_RANGE_PROBBT_ADD,
	TYPE_ORB_SPARK_MAKE_ORB,
	TYPE_INTERVAL_HP_ADD,
};

#define FCITEM_HERO_CONDITION_MAX		7
#define FCITEM_ITEM_CONDITION_MAX		3

struct ItemDataTable
{
	char szSkinName[24];
	short nCharSoxID;
	short nItemType;
	char szImageFile[24];

	short nLevelMinimum;

	int  nDurTime;		//지속 시간(시간은 보통 초단위)
	int	 nEffectInterval;
	float fEfftectArea;

	short nHP;			//HP차는 양
	short nOrb;			//오브 차는 양
	short nHPMaxAdd;		//게이지 늘림
	short nOrbMaxAdd;	//오브 게이지 늘림
	short nAttackPoint; //공격력 증감
	short nDefensePoint;  //방어력 증감
	short nCriticalProbbt;  //크리티컬 확률
	short nGuardBreak;		//가드 무시
	short nItemGainProbbt;  //아이템 얻을 확률 증가
	short nExpGainSpeed;	//경험치 올려주는 속도 증감(보통 이런건 퍼센트)
	short nOrbAttackGainSpeed;	//오브 어택 모으는 속도 증감
	short nMoveSpeed;		//이동속도 증감
	short nGuadianTrapDamage;		//함정 데미지 증감
	short nSiegeDamage;		//공성 병기 데미지 증감
	short nPIXISDamage;		//PIXIS 데미지 감소
	short nWeaponReach;		//무기 리치 증감
	short nDeathProbbt;		//적 즉사확율
	short nGuardianAttackPoint;//호위병 공격력
	short nPotionProbbt;	//포션이 떨어질 확률 증가
	short LvUp;				//레벨 업
	bool  bOrbSparkFree;	//오브 스파크 발동
	bool  bGodMode;				//무적이 됨
	short nItemBag;			//아이템 보관 가능
	short nGuardianDefensePoint;	//호위병 방어력 증감
	short nArrowDefense;		//화살에 대한 방어력 증감
	short nDashDist;			//대시 거리 증감
	short nOrbSplit;			//어택오브일때는 리얼오브부가로 얻고 그 반대는 반대로..그 반대입장의 퍼센트지정
	float fOrbAttackKeepTime;	//오브어택 시간 증가
	bool  bTrampleDamage;		//밟기 데미지 증가
	int   nReborn;
	bool  bEquipDup;
	short nGuadianMaxHPAdd;		//가디언의 MAX HP증가	
	short nGuadianSpeedAdd;		//가디언의 Speed 증가
	short nGuadianHPAdd;		//가디언의 HP 증가
	short nGuardBreakBoss;		//상대방이 보스일 경우 가드 브레이크 확률
	bool bIgnoreCritical;
	bool bEnableDash;
	short nDamageRatioAdd;
	bool bEnableJump;
	int nCriticalDamageRatio;
	int nSpecialAttackCriticalProbbt;
	bool bRevival;
	bool bEnableDefense;
	bool bNotUseOrbSpecialAttack;
	int  nSpecialAttackDistAdd;
	bool bAlwaysSmallDamage;
	short nTrueOrbGainSpeed;
	short nExpAdd;
	short nHPMaxAddInteger;
	int nHPConditionLowerPer;
	int nUnitKillBluePotionProbbt;
	bool bOrbToHpFillAndGone; //오브게이지가 HP채우고 사라짐
	short nDurationTimeAdd;
	short nHPWithGuardian;
	bool bGodModeInCharge;
	bool bAutoGuard;
	bool bStartFullOrb;
	short nSmallBluePortionFillHP;
	short nBigBluePortionFillHP;
	short nOrbFillWithTrueOrb;
	bool bOnlyDropBigRedPortion;
	short nSpecialAttackUseOrb;
	short nOrbAttackPowerAdd;
	short nTrailLengthAdd;
	bool bAutoOrbAttack;
	bool bSpecialCriticalRatio;	
	bool bOnlyDropSmallRedPortion;
	bool bDropChangeRedToBlue;
	short nConsumeItemFillHP;
	short nItemEquipLevelDown;
	short nHPLowerAddOrbGainSpeed;
	short nItemLifeTimeAdd;
	short nItemSlotCountAdd;
	short nAvoidCriticalRate;
	short nRevivalHPCondition;
	short nEnemyRangeProbbtAdd;
	short nGuadianRangeProbbtAdd;
	bool  bOrbSparkMakeOrb;
	short nIntervalHPAdd;

	short nHeroCondition[FCITEM_HERO_CONDITION_MAX];
	short nItemCondition[FCITEM_ITEM_CONDITION_MAX];

	ItemDataTable()
	{
		nDurTime = 0;
		nEffectInterval = 0;
		fEfftectArea = 0.f;
		nHP = 0;
		nOrb = 0;
		nHPMaxAdd = 0;
		nOrbMaxAdd = 0;
		nAttackPoint = 0;
		nDefensePoint = 0;
		nCriticalProbbt = 0;
		nGuardBreak = 0;
		nItemGainProbbt = 0;
		nExpGainSpeed = 0;
		nOrbAttackGainSpeed = 0;
		nMoveSpeed = 0;
		nGuadianTrapDamage = 0;
		nSiegeDamage = 0;
		nPIXISDamage = 0;
		nWeaponReach = 0;
		nDeathProbbt = 0;
		nGuardianAttackPoint = 0;
		nPotionProbbt = 0;
		LvUp = 0;
		bOrbSparkFree = 0;
		bGodMode = false;
		nItemBag = 0;
		nGuardianDefensePoint = 0;
		nArrowDefense = 0;
		nDashDist = 0;
		nOrbSplit = 0;
		fOrbAttackKeepTime = 0.f;
		bTrampleDamage = false;
		nReborn = 0;
		bEquipDup = true;
		nGuadianMaxHPAdd = 0;
		nGuadianSpeedAdd = 0;
		nGuadianHPAdd = 0;
		nGuardBreakBoss = 0;
		bIgnoreCritical = false;
		bEnableDash = true;
		nDamageRatioAdd = 0;
		bEnableJump = true;
		nCriticalDamageRatio = 0;
		nSpecialAttackCriticalProbbt = 0;
		bRevival = 0;
		bEnableDefense = true;
		bNotUseOrbSpecialAttack = false;
		nSpecialAttackDistAdd = 0;
		bAlwaysSmallDamage = false;
		nTrueOrbGainSpeed = 0;
		nExpAdd = 0;
		nHPMaxAddInteger = 0;
		nHPConditionLowerPer = 0;
		nUnitKillBluePotionProbbt = 0;
		bOrbToHpFillAndGone = false;
		nDurationTimeAdd = 0;
		nHPWithGuardian = 0;
		bGodModeInCharge = false;
		bAutoGuard = false;
		bStartFullOrb = false;
		nSmallBluePortionFillHP = 0;
		nBigBluePortionFillHP = 0;
		nOrbFillWithTrueOrb = 0;
		bOnlyDropBigRedPortion = false;
		nSpecialAttackUseOrb = 0;
		nOrbAttackPowerAdd = 0;
		nTrailLengthAdd = 0;
		bAutoOrbAttack = false;
		bSpecialCriticalRatio = false;		
		bOnlyDropSmallRedPortion = false;
		bDropChangeRedToBlue = false;
		nConsumeItemFillHP = 0;
		nItemEquipLevelDown = 0;
		nHPLowerAddOrbGainSpeed = 0;
		nItemLifeTimeAdd = 0;
		nItemSlotCountAdd = 0;
		nAvoidCriticalRate = 0;
		nRevivalHPCondition = 0;
		nEnemyRangeProbbtAdd = 0;
		nGuadianRangeProbbtAdd = 0;
		bOrbSparkMakeOrb = false;
		nIntervalHPAdd = 0;


		memset(	nHeroCondition,-1, sizeof(short) * FCITEM_HERO_CONDITION_MAX );
		memset(	nItemCondition,-1, sizeof(short) * FCITEM_ITEM_CONDITION_MAX );
	}
};

#ifndef _LTCG
//#define _SHOW_KOREAN_ITEM_NAME
#endif

class CItemDescTable : public CSingleton<CItemDescTable>
{
public:
	struct ItemDesc
	{
		char *pName;
		char *pDesc;
		ItemDesc()
		{
			pName = NULL;
			pDesc = NULL;
		}
	};	

	CItemDescTable();
	~CItemDescTable();

	void Load();
	void Clear();
	char *GetItemName(int nIndex){ return m_vtDescList[nIndex]->pName; }
	char *GetItemDesc(int nIndex){ return m_vtDescList[nIndex]->pDesc; }

	char *GetKoreanName(int nIndex);

	
protected:
	std::vector<ItemDesc *>m_vtDescList;
#ifdef _SHOW_KOREAN_ITEM_NAME
	std::vector<char *>m_vtKoreanNames;
#endif
};

class CITemTable : public CSingleton<CITemTable>
{
public:
	CITemTable();
	virtual ~CITemTable();
	void Load();
	ItemDataTable *GetItemData(int nIndex){return m_ItemDataList[nIndex];}
	int GetItemTableCount() { return (int)m_ItemDataList.size(); }
	void SetItemParamValue( FC_ITEM_PARAM_TYPE Type, ItemDataTable *pItemData, int nValue );

protected:
	std::vector<ItemDataTable *>m_ItemDataList;

};


#define ITEM_DROP_TABLE_MAX	25

struct ItemDropTableInfo
{
	int nItemIndex[ITEM_DROP_TABLE_MAX];
	int nProbt[ITEM_DROP_TABLE_MAX];

	ItemDropTableInfo()
	{
		for(int i = 0;i < ITEM_DROP_TABLE_MAX;i++)
		{
			nItemIndex[i] = -1;
			nProbt[i] = -1;
		}
	}
};

class CItemDropTable : public CSingleton<CItemDropTable>
{
public:
	CItemDropTable();
	~CItemDropTable();
	void Load();
	ItemDropTableInfo *GetItemDropTableInfo(int nIndex){return m_vtDropInfoList[nIndex];}

protected:
	std::vector<ItemDropTableInfo *>m_vtDropInfoList;

};

#define ITEM_LEVEL_TABLE_MAX	11
struct ITEM_LEVEL_TABLE
{
	int nTableIndex[ITEM_LEVEL_TABLE_MAX];
	ITEM_LEVEL_TABLE()
	{
		for(int i = 0;i < ITEM_LEVEL_TABLE_MAX;i++)
			nTableIndex[i] = -1;

	}
};

class CItemLevelTable : public CSingleton<CItemLevelTable>
{
public:
	CItemLevelTable();
	~CItemLevelTable();
	void Load();
	ITEM_LEVEL_TABLE *GetItemLevelTable(int nUnitSoxID){ return m_LevelTable[nUnitSoxID]; }
protected:
	std::vector<ITEM_LEVEL_TABLE *>m_LevelTable;

};

#define MAX_SET_TABLE		7
#define MAX_SET_REQUIRE_ITEM	5

#define SET_ITEM_MAKE		0
#define SET_ITEM_EFFECT		1

struct ITEM_SET_INFO
{
	int nItem[MAX_SET_REQUIRE_ITEM];
	int nSetItem;
	int nType;
};


class CItemSetTable : public CSingleton<CItemSetTable>
{
public:
	CItemSetTable();
	~CItemSetTable();
	void Load();
	ITEM_SET_INFO *GetSetItemInfo(int nIndex){ return m_SetItemList[nIndex]; }
	int  GetSetItemCount(){ return (int)m_SetItemList.size(); }
protected:
	std::vector<ITEM_SET_INFO *>m_SetItemList;

};

struct SOUND_PLAY_INFO
{
	int nCtgy;
	int nAmor;
	int nFloor;
	int nWeapon;
	int nEnemyWeapon;
	int nWeaponUseType;
	int nWeight;
	int nAttType;
	int nCueID;
	SOUND_PLAY_INFO()
	{
		nCtgy = -1;
		nAmor = -1;
		nFloor = -1;
		nWeapon = -1;
		nEnemyWeapon = -1;
		nWeaponUseType = -1;
		nWeight = -1;
		nAttType = -1;
		nCueID = -1;
	}

};

//===================================================================================
enum CROWD_UNIT_TYPE
{	
	CUT_DRAGON,
	CUT_ELF,
	CUT_GEGE,
	CUT_GOBLIN,
	CUT_HUMAN,
	CUT_NIGHT,
	CUT_ORC,
	CUT_PUKU,
	CUT_TOTAL,
	CUT_LAST
};

struct SOUND_CROWD_INFO
{
	int nUnitCount;
	int nSoundHandle[2];
	char *pPlayCue[2];
	CROWD_UNIT_TYPE UnitType;
	SOUND_CROWD_INFO()
	{
		nUnitCount = 0;
		nSoundHandle[0] = nSoundHandle[1] = -1;
		pPlayCue[0] = pPlayCue[1] = NULL;
		UnitType = CUT_TOTAL;
	}

};

class CSoundPlayCrowdTable : public CSingleton<CSoundPlayCrowdTable>
{
public:
	CSoundPlayCrowdTable();
	~CSoundPlayCrowdTable();
	void Load();
	int  GetTableCount(){ return m_SoundCrowdTable.size(); }
	SOUND_CROWD_INFO *GetTableData(int nIndex){ return m_SoundCrowdTable[nIndex]; }
protected:
	std::vector<SOUND_CROWD_INFO *>m_SoundCrowdTable;
};

//===================================================================================
class CSoundPlayTable : public CSingleton<CSoundPlayTable>
{
public:
	CSoundPlayTable();
	~CSoundPlayTable(){};
	void Load();
	int GetSoundCount() { return (int)m_pSoundList.size(); }
	SOUND_PLAY_INFO *GetSoundPlayTable(int nIndex) {return m_pSoundList[nIndex];} 
 
protected:
	std::vector<SOUND_PLAY_INFO *> m_pSoundList;
};


struct LocalTextTable
{
	int		nTextId;
	char	szText[256];
};


//userdefaulttexttable, defaulttexttable, briefingtexttable
class CLocalTextTableLoader : public CSingleton<CLocalTextTableLoader>
{
public:
	CLocalTextTableLoader();
	virtual ~CLocalTextTableLoader();

	void	Load(BStream *pStream);
	void	Release();
	char*	GetItemData(int nTextId);
	int		GetItemTableCount()		{ return (int)m_ItemDataList.size(); }

protected:
	std::vector<LocalTextTable*>		m_ItemDataList;
};

//===================================================================================
struct AbilityLocalTable
{
	int		nLevel;

	//local
	char	cName[256];
	char	cCommand[256];			//입력 command
	char*	pCaption;
};

class CAbilityTableLoader : public CSingleton<CAbilityTableLoader>
{
public:
	CAbilityTableLoader();
	virtual ~CAbilityTableLoader();
	
	void				Load(int nHeroType);
	void				Release();
	AbilityLocalTable*	GetItemData(int nIndex);
	int					GetItemTableCount()		{ return (int)m_ItemDataList.size(); }

protected:
	void			LoadTable(BStream *pStream);

protected:
	std::vector<AbilityLocalTable*>		m_ItemDataList;
};


//===================================================================================
struct TroopDataInfo 
{
	TroopDataInfo()
	{
		memset( nRangeAttackRange, 0, sizeof(int) * 12 );
	}

	int nRangeAttackRange[12];		// 레벨이 12까지 있다.
	float fMoveSpeed;
};


class CTroopSOXLoader : public CSingleton<CTroopSOXLoader>
{
public:
	CTroopSOXLoader(void);
	virtual ~CTroopSOXLoader(void);

	bool Load();
	void Clear();

protected:
	int m_nDataCount;
	TroopDataInfo *m_pData;

public:
	TroopDataInfo *GetData(int nIndex)			{ return m_pData+nIndex; }
	int GetDataCount()							{ return m_nDataCount; }
	int GetRange( int nIndex, int nLevel )		{ assert( nLevel >= 0 && nLevel < 12 ); return m_pData[ nIndex ].nRangeAttackRange[nLevel]; }
	float GetMoveSpeed( int nIndex )			{ return m_pData[ nIndex ].fMoveSpeed; }
};



//===================================================================================
struct LibraryTable
{
	int		nID;
	int		nPoint;							//Iten을 보기 위한 소비 point
	int		nCharCondition;					//char 조건
	int		nStageCondition;				//stage 조건
	
	char	cImageFileName[64];
	char	cIconFileName[64];
};

class CLibraryTableLoader : public CSingleton<CLibraryTableLoader>
{
public:
	CLibraryTableLoader();
	virtual ~CLibraryTableLoader();
	
	void			Load();
	void			Release();
	LibraryTable*	GetItemData(int nIndex);
	int				GetItemTableCount()		{ return (int)m_ItemDataList.size(); }

protected:
	void			LoadTable(BStream *pStream);

protected:
	std::vector<LibraryTable*>		m_ItemDataList;
};

//===================================================================================
struct CharLocalTable
{	
	CharLocalTable()
	{
		nUnitID = -1;

		//local
		memset(cName, 0, sizeof(char)*256);
		memset(cAge, 0, sizeof(char)*256);
		memset(cPosition, 0, sizeof(char)*256);
		memset(cWeapon, 0, sizeof(char)*256);
		memset(cOrbSpark, 0, sizeof(char)*256);
		pCaption = NULL;
	};

	int		nUnitID;

	//local
	char	cName[256];
	char	cAge[256];
	char	cPosition[256];
	char	cWeapon[256];
	char	cOrbSpark[256];
	char*	pCaption;
};

class CLibCharLocalLoader : public CSingleton<CLibCharLocalLoader>
{
public:
	CLibCharLocalLoader();
	virtual ~CLibCharLocalLoader();
	
	void			Load();
	void			Release();
	CharLocalTable*	GetItemData(int nIndex);
	int				GetItemTableCount()		{ return (int)m_ItemDataList.size(); }

protected:
	void			LoadTable(BStream *pStream);

protected:
	std::vector<CharLocalTable*>		m_ItemDataList;
};

