#pragma once


//#include "FcGameObject.h"
//#include "FcHeroObject.h"

#include "SmartPtr.h"
#include "CrossVector.h"

#define PLAYER_TEAM	0

#define TROOP_SOUND_CROWD_COUNT			5

class CFcProp;
class CFcWorld;
struct TROOP_INFO;
struct TROOP_ATTR;

class CFcHeroObject;
class CFcTroopObject;
class CFcGuardianTroop;
class CFcPlayerTroop;
class CFcGameObject;
class CFcGuardianManager;

typedef CSmartPtr<CFcTroopObject> TroopObjHandle;
typedef CSmartPtr<CFcHeroObject> HeroObjHandle;
typedef CSmartPtr<CFcGuardianTroop> GuardianTroopObjHandle;
typedef CSmartPtr<CFcPlayerTroop> PlayerTroopObjHandle;
typedef CSmartPtr<CFcGameObject> GameObjHandle;


enum TROOP_FORMATION_TYPE
{
	TROOP_FORMATION_TYPE_RECT,
	TROOP_FORMATION_TYPE_CIRCLE
};


struct ENALBE_TROOP_LIST_INFO
{
	ENALBE_TROOP_LIST_INFO()
	{
		nID = -1;
		nOneEnableTick = 6;
	}

	int nID;
	int nOneEnableTick;
	std::vector<TroopObjHandle> m_vecTroops;
};


struct GuardianTroopInfo
{
	GuardianTroopInfo()
	{
		hGuardian.Identity();
		nPreAIID = -1;
	};

	TroopObjHandle hGuardian;
	int nPreAIID;
};



	
class CFcTroopManager
{
public:
	CFcTroopManager();
	~CFcTroopManager();

	bool Initialize();	
	virtual void Process();
	virtual void PostProcess();

	TroopObjHandle AddTroop( CFcWorld* pWorld, TROOP_INFO* pTroopInfo );
	TroopObjHandle AddPlayerTroop( CFcWorld* pWorld, TROOP_INFO* pTroopInfo );

	TroopObjHandle AddTroopOnProp( CFcWorld* pWorld, CFcProp* pProp );
	TroopObjHandle AddTroopWallDummy( CFcWorld* pWorld, CFcProp* pProp );

	const D3DXVECTOR3& GetPos( int nIndex );

	void CmdMove( int nIndex, float fX, float fY );
	void CmdGuardianAttack();
	void CmdGuardianDefense();
	bool CmdRegisterGuardian( int nIndex );

	void AddHero( HeroObjHandle Handle );

	void GetListInRange( D3DXVECTOR2 *pPosition, float fRadius, std::vector<TroopObjHandle> &Objects );
	void GetEnemyListInRange( D3DXVECTOR2 *pPosition, int nTeam, float fRadius, std::vector<TroopObjHandle> &Objects );
	void GetFriendlyListInRange( D3DXVECTOR2 *pPosition, int nTeam, float fRadius, std::vector<TroopObjHandle> &Objects );
	void GetListInRangeAndArea( D3DXVECTOR2 *pPosition, float fRadius, float fSX, float fSY, float fEX, float fEY, std::vector<TroopObjHandle> &Objects );
	TroopObjHandle GetNearMeleeEnemyInRange( D3DXVECTOR2 *pPosition, int nTeam, float fRadius );

	int GetTroopCount()							{ return m_vecTroop.size(); }
	TroopObjHandle GetTroop( int nIndex )		{ return m_vecTroop[nIndex]; }

	TroopObjHandle GetTroopByName( char* pName );

	void UpdateObjects();
	void RenderObjects();

	int SetEnableTroopByName( char* pStr );

	void SetDisableTroopInArea( float fSX, float fSY, float fEX, float fEY );
	int SetEnableTroopInArea( float fSX, float fSY, float fEX, float fEY );
	void SetDisableEnemyTroopInArea( float fSX, float fSY, float fEX, float fEY );
	int SetEnableEnemyTroopInArea( float fSX, float fSY, float fEX, float fEY );
	void SetDisableFriendTroopInArea( float fSX, float fSY, float fEX, float fEY );
	int SetEnableFriendTroopInArea( float fSX, float fSY, float fEX, float fEY );

	void SetGuardianTroop( TroopObjHandle hTroop );
	TroopObjHandle GetGuardianTroop( int nIndex );
	void SetGuardianEnable( int nIndex, bool bEnable );

	void SetFirstGuardian();

	void DebugPathRender();
	HeroObjHandle GetHero() { return m_hHero; }

	TroopObjHandle GetGuardianTarget();
	TroopObjHandle GetLockOnTarget();

	void SetGuardianAI( int nAttackAI, int nDefenseAI, int nNormalAI );

	bool RegisterGuardian( int nIndex );
	bool RegisterCancelGuardian( int nIndex );
	int GetGuardianNum();

	bool IsGuardian( TroopObjHandle hTroop );

	void CreateUnits( TroopObjHandle hTroop, TROOP_ATTR* pInitInfo );

	float GetGuardianLiveRatePercent();
	float GetFriendlyTroopsLiveRatePercent();

	bool IsFinishEnableTroops( int nID );
	void EliminateTroop( TroopObjHandle hTroop );

	void PostInitialize();

	void SetGuardianTroopDefenseAdd( int nRate );
	void SetGuardianTroopAttackAdd( int nRate );
	void SetGuardianTroopMoveSpeedAdd( int nRate );
	void SetGuardianTroopMaxHPAdd( int nRate );
	void SetGuardianTroopTrapDamageAdd( int nRate );
	void AddGuardianTroopHP( int nVal, bool bPercent = true );


protected:
	void InitTroopUnitCount();
	bool CheckFakeTroop( TroopObjHandle hTroop );

	bool IsValidMelee( TroopObjHandle hMe, TroopObjHandle hTarget );
	bool IsMeleeRange( TroopObjHandle hMe, TroopObjHandle hTarget );

	HeroObjHandle* GetHero( float x, float y, float range);

//	GameObjHandle CreateGameObject( int nUnitIndex, CCrossVector &Cross );

	TroopObjHandle InitTroop( int Type, TROOP_INFO* pTroopInfo, int nX, int nY, int nRange, int nUnitCount );
	float InitVecFormation( TroopObjHandle hTroop, bool bLeader, int nLeaderPosType, 
							int nRow, int nCol, TROOP_FORMATION_TYPE FormationType, int nDensity);

	void ProcessFake();
	void CaculateFakeMode( TroopObjHandle hTroop );
	void CaculateMeleeFakeMode( TroopObjHandle hTroop, TroopObjHandle hEnemy );

protected:
	std::vector<TroopObjHandle> m_vecTroop;

	HeroObjHandle m_hHero;
	PlayerTroopObjHandle m_hPlayerTroop;

//	GuardianTroopInfo m_GuardianInfo[2];		// 호위부대 정보 2개
//	int m_nGuardianAttackAI;
//	int m_nGuardianDefenseAI;
//	int m_nGuardianNormalAI;

	

	TroopObjHandle m_FirstGuardian[2];

	int m_nCurEnableID;
	std::vector< ENALBE_TROOP_LIST_INFO > m_vecEnableTroops;		// enable할 부대들
	std::vector<bool> m_vecFakeState;
	CFcGuardianManager* m_pGuardianManager;
};


