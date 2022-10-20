#pragma once

#include "SmartPtr.h"
#include "CrossVector.h"
#include "FcParamVariable.h"
#include "navigationwaypoint.h"


class CFcTroopObject;
class CFcUnitObject;
class CFcHeroObject;
class CFcGameObject;

class NavigationMesh;
class NavigationCell;
class NavigationPath;

class CFcProp;
class CFcBreakableProp;

struct PATH_PACK;
struct TROOP_ATTR;


typedef CSmartPtr<CFcGameObject> GameObjHandle;
//typedef CSmartPtr<CFcUnitObject> UnitObjHandle;
typedef CSmartPtr<CFcHeroObject> HeroObjHandle;
typedef CSmartPtr<CFcTroopObject> TroopObjHandle;

#define DEFAULT_BASE_TROOP_OBJECT_POOL_SIZE		50

#define TROOP_ATTACKED_TYPE_MELEE		0
#define TROOP_ATTACKED_TYPE_RANGE		1
#define TROOP_ATTACK_TYPE_MELEE		0
#define TROOP_ATTACK_TYPE_RANGE		1

#define TROOP_FOLLOWER_MAX			12



enum RET_TYPE_PROCESS_MOVE
{
	RET_TYPE_DONE,
	RET_TYPE_FAIL,
	RET_TYPE_ON_GOING
};


enum TROOPSTATE
{
	TROOPSTATE_NON=0,
	TROOPSTATE_MOVE,
	TROOPSTATE_MOVE_MERCIFULLY,		// m_vecTargetTroops, m_vecCounterattractionTroops
	TROOPSTATE_MOVE_PATH,
	TROOPSTATE_MOVE_PATH_LOOP,
	TROOPSTATE_MOVE_FULL_PATH,
	TROOPSTATE_RETREAT,
	TROOPSTATE_RANGE_ATTACK,
	TROOPSTATE_MELEE_ATTACK,
	TROOPSTATE_ELIMINATED,
	TROOPSTATE_MOVE_ATTACK,
	TROOPSTATE_MELEE_READY,			// 부대가 melee를 위해 겹쳐지고 있는 상태
	TROOPSTATE_HOLD,				// 안 움직이고 적이 오면 공격
	TROOPSTATE_HOLD_MERCIFULLY,
	TROOPSTATE_FOLLOW,				// 어떤 부대를 따라감
	TROOPSTATE_NUM
};


enum TROOPSUBSTATE
{
	TROOPSUBSTATE_WALK,
	TROOPSUBSTATE_RUN,
	TROOPSUBSTATE_BATTLERUN,
	TROOPSUBSTATE_NUM
};

enum TROOPTYPE
{
	TROOPTYPE_NON = -1,
	TROOPTYPE_PLAYER_1,
	TROOPTYPE_PLAYER_2,
	TROOPTYPE_MELEE,
	TROOPTYPE_RANGEMELEE,			
	TROOPTYPE_SIEGE,
	TROOPTYPE_FLYING,
	TROOPTYPE_ON_PROP,
	TROOPTYPE_MOVETOWER,
	TROOPTYPE_SPEAR,
	TROOPTYPE_WALLENEMY,			// 카라랑 바르간다에서 나오는 세랑에서 나와 성위에서 싸우는 특별한 고블린 부대
	TROOPTYPE_RANGE,
	TROOPTYPE_NOBATTLE,
	TROOPTYPE_WALL_DUMMY,			// 성벽을 공격하기 위한 성벽에 붙은 더미 부대
	
	TROOPTYPE_NUM
};


struct TROOPSTATEDATA
{
	TROOPSTATEDATA() {
		State = TROOPSTATE_NON;
		fTargetX = fTargetY = 0.f;
		pPath = NULL;
		nCurPathIndex = -1;
		bForward = true;
		fDist = 0.f;
		bFollowMove = false;
		bForceAttack = false;
	}
	TROOPSTATE State;
	TROOPSUBSTATE SubState;
	float fTargetX, fTargetY;
	TroopObjHandle hTarget;
	PATH_PACK* pPath;
	int nCurPathIndex;
	bool bForward;
	bool bLoop;
	CFcProp* pProp;
	D3DXVECTOR3 TargetPos;
	float fDist;
	bool bFollowMove;
	bool bForceAttack;
};


struct UNIT_OFFSET
{
	D3DXVECTOR2 Dir;
	float fDist;
};




class CFcTroopObject
{
public:
	CFcTroopObject();
	virtual ~CFcTroopObject();

	virtual bool Initialize( int nX, int nY, int nRadius, 
							int nTeam, int nGroup, 
							int nID, char* pName, int nTroopType, 
							int nCreateCount );

	virtual bool ReInit( int nX, int nY, int nDir, bool bNew );

	virtual void Process();
	void PostProcess();
	virtual void DebugRender();

	void SetIndex( int nIndex )				{ m_nTroopIndex = nIndex; }
	int GetIndex()							{ return m_nTroopIndex; };

	void InitUnitArray( int nNum );
	void PostUnitArray();

	void AddLeader( GameObjHandle hObj );
	void AddUnit( GameObjHandle hObj, int nIndex );

	void CmdDie( GameObjHandle hObj );
	void RemoveUnit( GameObjHandle hObj );		// Leader도 여기서 지움

	GameObjHandle GetLeader()				{ return m_hLeader; }
	int GetUnitCount()						{ return m_nUnitNum; }
	GameObjHandle GetUnit( int nIndex )		{ return m_pUnits[nIndex]; }
	GameObjHandle GetFirstLiveUnit();

	int GetAliveUnitCount();
	int GetDeadUnitCount();
	D3DXVECTOR2 GetAverageAliveUnitPos();

	const D3DXVECTOR3& GetPos()				{ return m_Pos; }
	D3DXVECTOR2 GetPosV2()					{ D3DXVECTOR2 Pos(m_Pos.x, m_Pos.z); return Pos; }
	bool GetUnitCenterPos( D3DXVECTOR2* pPos );

	void SetAttackRadius(float r)			{ m_fAttackRadius = r; }
	float GetAttackRadius()					{ return m_fAttackRadius; }	

	void SetMoveSpeed( float fValue )		{ m_fMoveSpeed = fValue; }
	void SetWalkSpeedRate( float fValue )	{ m_fWalkMoveSpeedRate = fValue; }

	void AddHP( int nVal,bool bPercent = true );
	int GetHPAdd()							{ return m_nMaxHPAdd; }
	void SetMaxHPAdd( int nRate );
	void SetMaxHPAdd( float fRate );
	int GetMaxHPAdd()						{ return m_nMaxHPAdd; }
	void SetDefenseAdd( int nRate );
	int GetDefenseAdd()						{ return m_nDefenseAdd; }
	int GetDefense();
	void SetAttackAdd( int nRate );
	int GetAttackAdd();
	void SetTrapDamageAdd( int nRate )	{ m_nTrapDamageAdd = nRate; }
	int GetTrapDamageAdd()				{ return m_nTrapDamageAdd; }
	void SetMoveSpeedAdd( int nRate )	{ m_nMoveSpeedAdd = nRate; }
	int GetMoveSpeedAdd()				{ return m_nMoveSpeedAdd; }

	bool GetUnitsNearbyPlayer( int* pLastTick, std::vector<GameObjHandle> &VecUnits, bool bUpdate );
	int GetUnitsNearbyPlayerCheckTick() { return m_nCheckUnitsNearbyPlayerTick; }

	float GetMoveSpeed();

	virtual int GetBossHP();
	virtual int GetBossMaxHP();

	bool CmdStop();
	bool CmdMoveAttack( float fX, float fY );
//	bool CmdMove( float fX, float fY, float fMoveSpeedRate );
	bool CmdMove( float fX, float fY, bool bRun );
	bool CmdRetreat( float fX, float fY, bool bRun = false );
	bool CmdVictory( float fX, float fY );
	virtual bool CmdAttack( TroopObjHandle hTarget, bool bBoost = false, bool bForce = false );
	bool CmdDefense( TroopObjHandle hTarget );
	virtual bool CmdEngageMercifully( TroopObjHandle hTarget, std::vector<TroopObjHandle>& vecVoidTroops );
	virtual bool IsEnemyInAttackRange( TroopObjHandle hTarget ) { return false; }

	bool CmdAnnihilate();
	bool CmdEnable( bool bEnable, bool bNew = false, bool bExceptGuardian = true );
	bool CmdFollow( TroopObjHandle hTarget, float fDist, bool bBoost = true );
	bool CmdInvulnerable( bool bEnable );
	bool CmdLeaderInvulnerable( bool bEnable );
	bool CmdPlace( float fX, float fY, int nDir );
	bool CmdRenew( float fX, float fY, int nDir );
	bool CmdRun( float fX, float fY );
	bool CmdWalk( float fX, float fY );
	bool CmdMovePath( PATH_PACK* pPath, int nCurIndex, bool bForward, bool bLoop );
	bool CmdMoveFullPath( PATH_PACK* pPath, int nCurIndex, bool bForward );
	bool CmdCrossBridge( CFcProp* pProp );

	bool CmdHold( TroopObjHandle hTarget );
	bool CmdHoldMercifully( TroopObjHandle hTarget );
	virtual bool CmdConquestWall()	{ return true; }		// 세랑에서 나와 성벽에 나오기

	void CmdForceDisable();

	bool IsEliminated();
	bool IsMeleeTarget( TroopObjHandle hTroop );

	bool CanIMelee();

	void AddMeleeEngageTroop( TroopObjHandle hTroop );
	void RemoveMeleeEngageTroop( TroopObjHandle hTroop );

	int GetTroopID()					{ return m_nTroopID; }		// m_nID는 SmartPtr에서 사용
	const char* GetName()				{ return m_cName; }
	TROOPSTATE GetState()				{ return m_State.State; }
	D3DXVECTOR2 GetTargetPosV2() const	{ D3DXVECTOR2 Pos(m_State.fTargetX, m_State.fTargetY); return Pos; }

	bool IsForceAttack()				{ return m_State.bForceAttack; }

	int GetCurrentPathIndex()			{ return m_State.nCurPathIndex; }
	TROOPSUBSTATE GetSubState()			{ return m_State.SubState; }
	TROOPTYPE GetType()					{ return m_Type; }
	int GetForce()						{ return m_nForce; }
	void SetForce(int nForce);
	int GetTeam()						{ return m_nTeam; }
	void SetTeam(int nTeam);
	void ChangeTeam(int nTeam);
	int GetLevel()						{ return m_nLevel; }
	void SetLevel( int nLevel )			{ m_nLevel = nLevel; }

	void SetRadius( float fRadius )		{ m_fOriginalRadius = m_fRadius = fRadius; }
	float GetRadius( bool bEnableMeleeRadius = true)
	{
		if( bEnableMeleeRadius )
		{
			if( m_fMeleeRadius != 0.f )
				return m_fMeleeRadius;
			return m_fRadius;
		}
		else
			return m_fRadius;
	}
	int GetHP() { return m_nHP; }
	int GetMaxHP();

	void SetDir( D3DXVECTOR2& Dir )	{ m_Dir = Dir; }
	D3DXVECTOR2 GetDir(){ return m_Dir; }
	D3DXVECTOR2	GetFacingDirV2() { return m_Dir; }
	int GetFollowerCount(int nTroopId);
	virtual void PostInitialize();

	void SetHandle(CSmartPtr<CFcTroopObject> Handle) { m_Handle=Handle; }

	void Update();
	void Render();

	void SetRenderSkip( bool bRender )		{ m_bRenderSkip = bRender; }
	bool IsRenderSkip()				{ return m_bRenderSkip; }
	bool IsRangeEngaged()							{ return (m_State.State == TROOPSTATE_RANGE_ATTACK); }
	TroopObjHandle GetRangeTarget()					{ return m_State.hTarget; }
	bool IsMeleeEngaged()							{ return ( m_vecEngageTroops.size() != 0 ); }
	int GetMeleeEngageTroopCount()					{ return m_vecEngageTroops.size(); }
	TroopObjHandle GetMeleeEngageTroop( int index )	{ return m_vecEngageTroops[index]; }

	void CheckEngageTroop();
	void ScatterTroop();

	int GetNumLiveSoldier();
	void SetEliminate();

	void OnAttack( int nType, GameObjHandle hUnit );
	void OnAttacked( int nType, TroopObjHandle hTroop );

	bool IsAttacked();
	bool IsAttacked( TroopObjHandle hTroop );
	bool IsAttackedMelee();
	bool IsAttackedRange();
	bool IsEngaged();
	bool IsEnable()			{ return m_bEnable; }
	bool IsPlayerTroop();
	int  GetPlayerIndex();

	bool GetCollisionPropHeight( float fX, float fY, float* pHeight );

	void DebugPathRender();

	int GetAttackedTroops( std::vector<TroopObjHandle> &Objects );
	bool IsAttackedBy( TroopObjHandle hTroop );
	bool IsMeleeAttackedBy( TroopObjHandle hTroop );
	bool IsRangeAttackedBy( TroopObjHandle hTroop );
	TroopObjHandle GetLinkedTroop() { return m_hLinkTroop; }
	TroopObjHandle GetAttackedTroop();
	TroopObjHandle GetTargetTroop()		{ return m_State.hTarget; }

	PATH_PACK* GetFullPath()					{ return m_pCurFullPath; }
	void SetFullPath( PATH_PACK* pPath, int nPathIndex )		{ m_pCurFullPath = pPath; m_nCurFullPathIndex = nPathIndex; }

	std::vector<D3DXVECTOR3> *GetMoveQueue() { return &m_VecNavMoveQueue; }

	void AddVecFormation( int nType, D3DXVECTOR2& Dir, float fDist );
	void GetUnitFormationCross( int nIndex , CCrossVector* pCross );

	void SetUnitOffset();
	void SetDirByClock(int nClock,int nTotalTime = 12); // 시계 방향대로 바라보게 한다.
	void SetHPPercent(int nPercent);
	void AddHPPercent(int nPercent); // nPercent 만큼 각 개체별로 HP를 증가시킨다. eliminated troop은 제외.
	void SetVisibleInMinimap(bool bVisible){ m_bVisibleInMinimap = bVisible; }
	bool IsVisibleInMinimap(){ return m_bVisibleInMinimap; }
	void SetDie();

	void SaveMaterialAmbient();
	void SetMaterialAmbient(D3DXVECTOR4 vColor);
	void RestoreMaterialAmbient();

	void ShowObjectBaseDecal(int nTexIndex, int nTick, int nAlphaMode = 0);

	bool IsMoving();

	void SetTriggerCmd()	{ m_bTriggerCmd = true; }
	void ResetTriggerCmd()	{ m_bTriggerCmd = false; }

	void SetEnableGuardian( bool bEnable )	{ m_bEnableGuardian = bEnable; }
	bool IsEnableGaurdian()					{ return m_bEnableGuardian; }

	void SetInitUnitData( TROOP_ATTR* pData );

	bool LinkTroop( TroopObjHandle hTroop )	{ m_hLinkTroop = hTroop; return true; }

	void SetDropItem( int nItemIndex )		{ m_nItemIndex = nItemIndex; }

	bool IsAttackable() const { return m_bAttackable; }
	void SetAttackable( bool bAttackable ) { m_bAttackable = bAttackable; }
	void SetUntouchable( bool bUntouchable )	{ m_bUntouchable = bUntouchable; }
	bool IsUntouchable() { return m_bUntouchable; }

	void SetFakeMode( bool bFake )	{ m_bFakeMode = bFake; }

	bool IsStopProcess() { return m_bStopProcess; }
	void CheckUnitKind();
	int GetGoblinCount() { return m_nGoblinCount; }
	int GetOrcCount() { return m_nOrcCount; }
	int GetElfCount() { return m_nElfCount; }
	int GetEtcCount() { return m_nEtcUnitCount; }
	void SetDefaultState();

	void DisplayDebugString();

	void SetAdditionalAttackPower(int nAdd) { m_nAdditionalAttackPower = nAdd; }
	void SetAdditionalDefense(int nAdd) { m_nAdditionalDefense = nAdd; }

	void SetGuardian( bool bGuardian )		{ m_bGuardian = bGuardian; m_bUseTriggerCmd = false; }
	bool IsGuardian()						{ return m_bGuardian; }

	TroopObjHandle GetParentFollowTroop() { return m_hFollowTroop; }
	void SetFollowedTroop( TroopObjHandle hTroop );
	int GetFollowedTroopCount()			{ return m_vecFollowedTroop.size(); }
	TroopObjHandle GetFollowedTroop( int nIndex )	{ return m_vecFollowedTroop[nIndex]; }

	void SetFullHP();
	void SetHP();

	int GetCrowdUnitType()	{ return m_nCrowdUnitType; }

	void ResetWayPoint();


template <class T>
	static CSmartPtr<T> CreateObject()
	{
		T *pInstance;
		CSmartPtr<T> Handle;

		pInstance = new T();
		Handle = CFcTroopObject::s_ObjectMng.CreateHandle(pInstance);
		pInstance->SetHandle(Handle);
		return Handle;
	}

	static void UpdateObjects();
	static void RenderObjects();
	static void ProcessObjects();
	static void PostProcessObjects();
	static void ReleaseObjects();
	static void DebugRenderObjects();

	static int s_nTroopStateTable[TROOPSTATE_NUM];

	void SetCheckHPInProcess( bool bCheck ) { m_bCheckHPInProcess = bCheck; }
	bool IsCheckHPInProcess() { return m_bCheckHPInProcess; }

	bool IsFakeDisable()	{ return m_bFakeDisable; }
	void SetFakeDisable( bool bFlag )	{ m_bFakeDisable = bFlag; }

protected:
	bool SetState( TROOPSTATE State, DWORD Param1 = 0, DWORD Param2 = 0, DWORD Param3 = 0, DWORD Param4 = 0  );
	bool IsStopMove( D3DXVECTOR2* pPos );

	bool ProcessMove();
	bool ProcessMoveMercifully();
	bool ProcessMovePath();
	bool ProcessMoveFullPath();
	int ProcessMeleeReady();
	virtual bool ProcessMeleeAttack();
	virtual bool ProcessHold();
	virtual bool ProcessHoldMercifully();
	bool ProcessFollow();
	bool ProcessFormation();

	RET_TYPE_PROCESS_MOVE ProcessMove( float fX, float fY );

	virtual bool CmdRangeAttack( TroopObjHandle hTarget, bool bBoost = false, bool bForce = false );

	int AdjustPos();
	void InitNavCell();
	void PathFind( float& dstX, float& dstY,  int nDebugNo, bool bForce = false );

	virtual bool DoICrossBridge( CFcProp* pProp );
	void GetCrossStartPos( float fRadius, CFcProp* pProp, D3DXVECTOR2* pPos, D3DXVECTOR2* pTargetPos, float fDist, float* pNewRadius = NULL );
	bool IsNavInTargetPos( CFcProp* pProp );

	void UpdatePreState();

	D3DXVECTOR2 GetMercifullyNextPos();
	bool IsEnemyInAttackRange( TroopObjHandle hTarget, float radius );
	void AdjustPositionByAttr();
	bool IsMovableAttr( float fX, float fY, int nRadius, bool bFriend, float fMapXSize, float fMapYSize);
	bool GetNearBridgeAttr( D3DXVECTOR2 Pos, D3DXVECTOR2* MoveDir );
   	void SetMoveSpeedRate( float fRate )	{ m_fMoveSpeedRate = fRate; }

	void CaculateDir( D3DXVECTOR2* pNewDir );

	void OnProcessMeleeFinish();

	void CheckCulling();

	void ResetState();

	bool CanIProcess();

protected:
	
	int m_nTroopID;
	int m_nTroopIndex;				// TroopManager에서 Index
	char m_cName[32];
	bool m_bEnable;
	bool m_bFirstEnable;
	TROOPSTATEDATA m_State;
	TROOPSTATEDATA m_PreState;		// 길 건너는 경우 현재 스테이트를 m_PreState에 넣고 건너는 상태로 들어간다
	TROOPTYPE m_Type;
	float m_fRadius;
	float m_fOriginalRadius;
	float m_fMeleeRadius;
	D3DXVECTOR3 m_Pos;
	D3DXVECTOR3 m_InitPos;
	float m_fMoveSpeed;
	float m_fMoveSpeedRate;
	float m_fWalkMoveSpeedRate;
	int m_nForce;
	int m_nTeam;					// Group이 다르면 적, Team이 달라도 Group이 같으면 같은 편
	int m_nLevel;
	bool m_bCheckHPInProcess;		// TroopManager::Process 에서 HP 체크로 Eliminate 되는거 없어지면 이것두 없에라..

	std::vector<CFcProp*> m_vecNearCollisionProps;

	D3DXVECTOR2	m_Dir;		// Face Dir과 틀림

	GameObjHandle m_hLeader;
	int m_nUnitNum;
	GameObjHandle* m_pUnits;							// Leader 포함
	std::vector<GameObjHandle> m_vecExtraUnits;			// 유닛 생성할 때 부가적으로 생성되는 유닛들(발석차 끄는 녀석 같은 것)
	std::vector<TroopObjHandle> m_vecVoidTroops;		// AI에서 척력을 갖는 부대들
	std::vector<TroopObjHandle> m_vecEngageTroops;		// 싸우고 있는 부대들
	TroopObjHandle m_hFailEngageTroop;					// 범위안에 들어오고 갈 수 없는 곳에 적이 있다면 다시 CMD들어 왔을 때 이거 보고 false로 체크
	int m_nCheckFailEngageTroopTick;					// 적 쫓아가는거 실패한 것 체크한 틱

	std::vector<UNIT_OFFSET> m_vecTroopsFormation[2];	// Formation이 2가지 있다.
	int m_nCurFormationType;

// navigation mesh
	bool m_bUseNavMesh;
	NavigationMesh*	m_pNavMesh;					// navigation mesh (different troop use different nav meshes)
	NavigationCell*	m_pNavCell;					// position in navigation mesh
	NavigationPath* m_pNavPath;					// navigation path
	NavWayPointID	m_NextNavpoint;				// ID of the next waypoint we will move to
	bool			m_bNavPathActive;			// if using navigation path
	D3DXVECTOR2		m_prevPathFindDest;			// last path finding destination
	std::vector<D3DXVECTOR3> m_VecNavMoveQueue;

	NavWayPointID	m_PreNavpoint;				// 다리, 성문 같은것 지나가는 것 체크하기 위해 필요

	TroopObjHandle m_hAttackedRangeTroop;
	int m_nDelayRangeAttackedTick;

	TroopObjHandle m_hAttackedMeleeTroop;
	int m_nDelayMeleeAttackedTick;

	bool m_bForceCmd;							// 명령 강제로 실행
	bool m_bEliminated;
	int m_nEliminatedDelayTick;
	int m_nCrossMoveDelay;						// 차례차례보내기 위해 필요한 딜레이

	PATH_PACK* m_pCurFullPath;					// Full path는 지정되면 리셋 전까지 유지
	int m_nCurFullPathIndex;
	
	bool m_bCross;								// 다리나 성문 지나가는 중이면
	CFcProp* m_pCrossProp;

	int m_nMaxHP;
	int m_nHP;

	D3DXVECTOR2 m_LastMovePos;					// PathFind로 찾는 것만 해당됨

	float m_fAttackRadius;

	struct 	FOLLOWERINFO
	{
		int nFollower;
		int nRotten;
	};
	int	m_nFollower;
	FOLLOWERINFO	m_Follower[TROOP_FOLLOWER_MAX];	
	bool		m_bVisibleInMinimap;
	bool		m_bRenderSkip;

	bool		m_bTriggerCmd;						// Cmd가 Trigger에서 불렸는지
	bool		m_bUseTriggerCmd;					// 현재 Cmd가 Trigger에서 불린 것으로 동작중인지

	bool		m_bEnableGuardian;

	TROOP_ATTR* m_pInitUnitData;

	TroopObjHandle m_hLinkTroop;					// Dragon처럼 연결돼 있는 부대

	int			m_nItemIndex;
	int			m_nKilled;
	int			m_nRemoved;

	bool		m_bBoost;
	bool		m_bAttackable;
	bool		m_bUntouchable;
	bool		m_bStopProcess;

	bool		m_bFakeMode;							// 카메라 안보이는 곳에선 훼이크

	int m_nGoblinCount;
	int m_nOrcCount;
	int m_nElfCount;
	int m_nEtcUnitCount;

	int m_nAdditionalAttackPower;						// Enable 되기전에 파워업 트리거가 사용되었을때 기억했다가 생성후에 적용.
	int m_nAdditionalDefense;

	bool m_bGuardian;
	bool m_bFakeDisable;								// Fake 모드로 들어갈 수 없음

	TroopObjHandle m_hFollowTroop;
	std::vector<TroopObjHandle> m_vecFollowedTroop;		// 쫓아오는 부대

	int m_nMaxHPAdd;
	int m_nDefenseAdd;
	int m_nAttackAdd;
	int m_nTrapDamageAdd;
	int m_nMoveSpeedAdd;

	int m_nCrowdUnitType;

	int m_nCheckUnitsNearbyPlayerTick;
	std::vector<GameObjHandle> m_vecUnitsNearbyPlayer;

	bool m_bForceDisable;
	int m_nLiveUnitNumBackup;

private:										 
	static CSmartPtrMng<CFcTroopObject> s_ObjectMng;
	CSmartPtr<CFcTroopObject> m_Handle;
};



class CFcRangeTroop : public CFcTroopObject
{
public:
	CFcRangeTroop();
	virtual ~CFcRangeTroop();

	bool Initialize( int nX, int nY, int nRadius, int nForce, int nTeam, 
		int nID, char* pName, int nTroopType, int nCreateCount );

	virtual bool CmdAttack( TroopObjHandle hTarget, bool bBoost = false, bool bForce = false );

	virtual void Process();
	

	virtual bool CmdEngageMercifully( TroopObjHandle hTarget, std::vector<TroopObjHandle>& vecVoidTroops );

	virtual bool ProcessHold();
	virtual bool ProcessHoldMercifully();
	virtual void PostInitialize();
	virtual bool IsEnemyInAttackRange( TroopObjHandle hTarget );
	

protected:

	void CheckRangeAttack();
	bool ProcessRangeAttack();		
};




class CFcOnPropTroop : public CFcTroopObject
{
public:
	CFcOnPropTroop();
	virtual ~CFcOnPropTroop();

	bool Initialize( CFcProp* pProp );

//	virtual bool CmdAttack( TroopObjHandle hTarget );
//	virtual void Process();

	int GetUnitNum();
	D3DXVECTOR3 GetUnitPos( int nIndex );

protected:
//	void CheckRangeAttack();
//	bool ProcessRangeAttack();		

protected:
	CFcProp* m_pParentProp;
};



class CFcFlyTroop : public CFcTroopObject
{
public:
	CFcFlyTroop();
	virtual ~CFcFlyTroop();

	bool Initialize( int nX, int nY, int nRadius, int nTeam, int nGroup, 
					int nID, char* pName, int nTroopType, int nCreateCount );

	virtual bool CmdAttack( TroopObjHandle hTarget, bool bBoost = false, bool bForce = false );
	bool CmdEngageMercifully( TroopObjHandle hTarget, std::vector<TroopObjHandle>& vecVoidTroops );

	virtual void Process();
	virtual void PostInitialize();
	virtual bool IsEnemyInAttackRange( TroopObjHandle hTarget );

protected:
	bool ProcessRangeAttack();
	void CheckRangeAttack();
};



enum GUARDIAN_CMD_STATE
{
	GUARDIAN_CMD_STATE_NON,
	GUARDIAN_CMD_STATE_ATTACK,
	GUARDIAN_CMD_STATE_DEFENSE,
	GUARDIAN_CMD_STATE_DEFENSE_HERO,
	GUARDIAN_CMD_STATE_SURROUND_ENEMY
};


class CFcPlayerTroop : public CFcTroopObject
{
public:
	CFcPlayerTroop();
	virtual ~CFcPlayerTroop();

	virtual void Process();

	void SetControl( bool bEnable );

protected:
	bool m_bControl;

};


class CFcMoveTower : public CFcTroopObject
{
public:
	CFcMoveTower();
	~CFcMoveTower();

	virtual void Process();
	virtual bool ProcessMeleeAttack();
};

// 특별한 처리가 필요한 카라랑 바르간다 미션에서 세랑 타고 성벽에 올라가는 적 고블린 부대
class CFcWallEnemyTroop : public CFcTroopObject
{
public:
	CFcWallEnemyTroop();
	~CFcWallEnemyTroop();

	virtual bool Initialize( int nX, int nY, int nRadius, int nForce, int nTeam, 
							int nID, char* pName, int nTroopType, int nCreateCount );

	virtual void Process();
	virtual bool CmdConquestWall();
	bool IsAttackOnWall();

protected:
	int* m_pEscapeTick;
};

class CFcCatapultTroop : public CFcTroopObject
{
public:
	CFcCatapultTroop();
	~CFcCatapultTroop();

	virtual bool CmdAttack( TroopObjHandle hTarget, bool bBoost = false, bool bForce = false );
	virtual void PostInitialize();
	virtual void Process();

protected:
};

class CFcCitizenTroop : public CFcTroopObject
{
public:
	CFcCitizenTroop();
	~CFcCitizenTroop();

	virtual bool CmdAttack( TroopObjHandle hTarget, bool bBoost = false, bool bForce = false );
  	
protected:
};

class CFcWallDummyTroop : public CFcTroopObject
{
public:
	CFcWallDummyTroop();
	~CFcWallDummyTroop();

	virtual void Process();
	virtual void PostInitialize();	// SetFullHP 콜 막기위해 뺀다.
	bool CmdAttack( TroopObjHandle hTarget, bool bBoost, bool bForce = false );

	bool Initialize( CFcProp* pProp );
	CFcProp *GetParentProp() { return (CFcProp*)m_pParentProp; }
	void AddHP( int nValue );

	virtual int GetBossHP();
	virtual int GetBossMaxHP();

protected:
	CFcBreakableProp* m_pParentProp;
};
