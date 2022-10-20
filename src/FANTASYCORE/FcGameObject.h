#pragma once

#include "FcCommon.h"
#include "FcAniObject.h"
#include "FcState.h"
#include "data/AnimType.h"
#include "FcCorrelation.h"
#include "FcUnitJobChange.h"
#include "FcSoxLoader.h"
#include "FcStatusObject.h"

class ASData;
class CAniInfoData;
class ASSignalData;
struct UnitDataInfo;
class CFcGameObject;
class CFcTroopObject;
class CFcProp;
class CFcAIObject;
class CFcEventSequencerMng;
class CFcVelocityCtrl;
struct SOUND_PLAY_INFO;

#define EFFECT_LIGHT_FRAME 4

#define ROTATE_ANGLE			60

#define UNIT_MOVABLE			0x00000001
#define UNIT_ATTACKABLE			0x00000002
#define UNIT_JUMPABLE			0x00000004
#define UNIT_FLOCKABLE			0x00000008
#define UNIT_DEFENSIBLE			0x00000010
#define UNIT_RIDABLE			0x00000020
#define UNIT_RANGE				0x00000040

#define UNIT_STATE_STAY			0x00010000
#define UNIT_STATE_MOVE			0x00020000
#define UNIT_STATE_DAMAGE		0x00040000
#define UNIT_STATE_ATTACK		0x00080000
#define UNIT_STATE_JUMP			0x00100000
#define UNIT_STATE_DIE			0x00200000
#define UNIT_STATE_DOWN			0x00400000
#define UNIT_STATE_TRAMPLE		0x00800000

#define DIE_KEEP_FRAME			800
#define DIE_ALPHA_FRAME_INT		120
#define DIE_ALPHA_FRAME			120.0f
#define WATER_RIPPLE_HEIGHT_MIN		40.f

#define WATER_PARTICLE_HEIGHT		7.f


#define FLOCKING_INTERVAL DEFAULT_FRAME_RATE


struct WEAPON_OBJECT
{
	int nEngineIndex;
	int nObjectIndex;
	int nGroup;
	BOOL bShow;
	int nLinkBoneIndex;
	int nPartsIndex;
};

struct ARMOR_OBJECT
{
	int nObjectIndex;
	int nLinkBoneIndex;
	bool bUsePhysics;
};

typedef CSmartPtr<CFcTroopObject> TroopObjHandle;
typedef CSmartPtr<CFcGameObject> GameObjHandle;
typedef CSmartPtr<CFcAIObject> AIObjHandle;



struct PHYSICS_INIT_INFO {
	int nSkinIndex;
	PHYSICS_DATA_CONTAINER* pData;
};

struct INITGAMEOBJECTDATA
{
	INITGAMEOBJECTDATA()
	{
		pUnitSOX = NULL;
		nUnitIndex = -1;
		for( int i=0; i<10; i++ )
			nSkinIndex[i] = -1;

		nAniIndex = -1;
		nASDataIndex = -1;
		nAniInfoDataIndex = -1;
		pCollisionMesh = NULL;
		pRagdollSetting = NULL;
		nBillboardIndex = -1;
	}

	UnitDataInfo	*pUnitSOX;
	int nUnitIndex;
	int nSkinIndex[10];		// skin 레벨 4개, 베리에이션 최대 10개
	int nAniIndex;

	int nASDataIndex;
	int nAniInfoDataIndex;

	std::vector< PHYSICS_INIT_INFO > vecPhysicsInfo;

	PHYSICS_COLLISION_CONTAINER *pCollisionMesh;
	PHYSICS_RAGDOLL_CONTAINER *pRagdollSetting;

	int nBillboardIndex;

	int nSkinLevel;
};




class CFcGameObject : public CFcAniObject, public CFcUnitJobChange
{
public:
	enum GameObj_ClassID {
		Class_ID_Unknown = -1,
		Class_ID_Unit = 0,
		Class_ID_Fly = 1,
		Class_ID_Horse = 2,
		Class_ID_Archer = 3,
		Class_ID_Catapult = 4,
		Class_ID_MoveTower = 5,
		Class_ID_Adjutant = 6,

		Class_ID_Adjutant_GoblinWizard = 100,
		Class_ID_Adjutant_TeaTea = 101,
		Class_ID_Adjutant_Syumerrt = 102,
		Class_ID_Adjutant_Mylarrvaryss = 103,
		Class_ID_Adjutant_DarkElf = 104,
		Class_ID_Adjutant_KingFrog = 105,
		Class_ID_Adjutant_Yesperratt = 106,
		Class_ID_Adjutant_Dwykfarrio = 107,
		Class_ID_Adjutant_99King = 108,

		Class_ID_Hero = 1000,
		Class_ID_Hero_Aspharr = 1001,
		Class_ID_Hero_Inphy = 1002,
		Class_ID_Hero_Klarrann = 1003,
		Class_ID_Hero_VigkVagk = 1004,
		Class_ID_Hero_Myifee = 1005,
		Class_ID_Hero_Dwingvatt = 1006,
		Class_ID_Hero_Tyurru = 1007,
	};

	enum APPLY_ANI_TO_LOCAL_POSITION_FLAG {
		APPLY_NONE = 0x00,
		APPLY_X = 0x01,
		APPLY_Y = 0x02,
		APPLY_Z = 0x04,
	};

	CFcGameObject(CCrossVector *pCross);
	virtual ~CFcGameObject();

	static GameObjHandle CreateGameObject( GameObj_ClassID ClassID, CCrossVector *pCross, int nParam = -1 );
	void SetClassID( GameObj_ClassID ClassID ) { m_ClassID = ClassID; }
	GameObj_ClassID GetClassID() { return m_ClassID; }	

protected:
	GameObj_ClassID m_ClassID;
	int				m_nUnitSOXIndex;
	UnitDataInfo	*m_pUnitSOX;
	ASData			*m_pASData;
	CAniInfoData	*m_pUnitInfoData;
	D3DXVECTOR3		m_UnitSize;
	float			m_fScale;

	static int		s_nUnitStateTable[ ANI_TYPE_NUM ];

	HIT_PARAM		m_HitParam;

	std::vector< ARMOR_OBJECT >		m_ArmorList;
	std::vector< WEAPON_OBJECT >	m_WeaponList;
	int								m_nCurrentWeaponIndex;		// m_WeaponList 상의 인덱스이다..
	int								m_nDropWeaponIndex;
	int								m_nDropWeaponSkinIndex;
	std::vector< PHYSICS_DATA_CONTAINER* > m_PhysicsInfoList;


	int m_nCurAniType;
	int m_nCurAniAttr;
	int m_nCurAniTypeIndex;
	int m_nCurAniLength;
	int m_nAniLoopCount;

	D3DXVECTOR3 m_MoveTargetPos;
	D3DXVECTOR3 m_PrevPos;

	D3DXVECTOR3 m_PrevCollision1;
	D3DXVECTOR3 m_PrevCollision2;

	D3DXVECTOR3 m_MoveVector;
	D3DXVECTOR3 m_MoveDir;
	bool m_bEnableRotation;

	D3DXVECTOR3 m_LocalAniMove;	// DummyPos 를 구하기 윈한 로컬이동 얻어 놓는 벡터..
	D3DXVECTOR3 m_DummyPos;

	float m_fLastHeight;		// 마지막 지형 높이

	float m_fGravity;
	int m_nGravityChangeFrame;
	D3DXVECTOR3 m_vVelocity;
	float m_fGroundResist;

	int m_nTrailHandle[ 2 ];
	BOOL m_bTrailOn[ 2 ];


	int m_nStunDelay;

	// Game Play Member
	static int s_nIsOrbSparkOn;
	float m_fHP;		// 가디언 등록, 취소 하다보면 int는 소수점이하 오차로 HP가 줄어든다.
	int m_nTeam;
	int m_nForce;		// Team이 틀리면 적, Team이 똑같으면 Force가 달라도 같은 편
	int m_nLevel;
	int m_nExp;
	bool m_bBeatenByPlayer;
	BOOL m_bItemDrop;
	BOOL m_bExpDrop;
	BOOL m_bOrbDrop;
	BOOL m_bDropWeapon;
	int m_nDieFrame;
	bool m_bFreezePos;
	bool m_bGodMode;			// signal에서 세팅
	bool m_bInvulnerable;		// trigger에서 세팅
	bool m_bOnOrbsparkGodMode;	// OrbSpark중의 무적상태
	int m_nOnPropIndex;
	bool m_bDead;
	bool m_bInvincible;
	bool m_bIsInScanner;
	bool m_bKeepOrbSpark;		// 원래 HeroObject에 있어야 하지만.. Virtual 더이상 늘리기 싫어서 여기다 추가..
	bool m_bRagdollUnlink;
	float m_fRageDisposition;	// 분노도. 공격 활률등에 영향 미침
	static int s_nEffectLightFrame;

	// Attribute Add
	int m_nMaxHPAddPer;
	int m_nMaxHPAdd;
	int m_nAttackPowerAdd;
	int m_nDefenseAdd;
	bool m_bDropHorse;
	int m_nCriticalAdd;
	int m_nCriticalRatioAdd;
	bool m_bSpecialCriticalRatio;
	int m_nExpAdd;
	int m_nGuardBreak;
	int m_nBossGuardBreak;
	int m_nAttackDistAdd;
	int m_nTrailLengAdd;
	int m_nLevelAdd;
	int m_nDefenseProbAdd;
	int m_nDamageRatioAdd;
	bool m_bForceMove;
	bool m_bIsCatchable;
	bool m_bIgnoreCritical;
	bool m_bRevival;
	int  m_nRevivalHPCondition;
	bool m_bNowSpecialAttack;
	int m_nSpecialAttackCriticalAdd;
	int m_nSpecialAttackDistAdd;
	bool m_bBoss;
	bool m_bAutoGuard;
	bool m_bForceGenerateOrb;
	int m_nAvoidCriticalRate;

	TroopObjHandle m_hTroop;
	D3DXVECTOR2 m_TroopOffsetPos;		// 부대 중심에서 어느 만큼 떨어져 있는 위치
	//	D3DXVECTOR2 m_OriginalTroopOffsetPos;

	int m_AniPosToLocalPosFlag;

	// 승마용 변수들..
	GameObjHandle m_ParentLinkObjHandle;
	std::vector<GameObjHandle> m_VecLinkObjHandle;

	// 상관관계
	CFcCorrelationMng *m_pCorrelationMng;

	// 우헤헤~
	CFcEventSequencerMng *m_pEventSeq;
	CFcVelocityCtrl *m_pRotateCtrl;
	int m_nPrevRotationSide;

	// 플로킹 관련 체크
	// 플로킹이 한번이라도 됐을경우 m_bApplyFlocking 이 true 가 된다.
	// Move 관련 필요해서 추가.
	bool m_bApplyFlocking;
//	int m_nFlockingInterval;
	float m_fMaxFlockingSize;
	std::vector<GameObjHandle> m_VecFlocking;

	// Process LOD
	int m_nProcessLODInterval;
	int m_nProcessLODOffset;

	// AI Object Handle
	AIObjHandle m_hUnitAIHandle;
	int m_nAIControlIndex;
	int m_nAIPreloadIndex;

	// Move Queue
	int m_nTroopMoveQueueCount;

	int m_nCross;						// 다리 건너는 중

	int m_nMoveLockTick1;				// 낑겼을 때 필요
	int m_nMoveLockTick2;
	D3DXVECTOR2 m_nMoveLockPos;
	int m_nSoundHandle;

	bool m_bPhysicDamageProc;
	bool m_bSoxType;
	float m_fMoveSpeed;
	float m_fMoveSpeedRate;
	float m_fMoveSpeedRateForTroop;		// 부대 쫓아갈 때 속도 조절
	bool m_bBoost;						// 이동 속도 빠르게
	bool m_bMoveDelay;					// true : 이동중, false : 멈춤-이동 딜레이 적용가능
	CFcStatusObject *m_pObjectStatus;

	int m_nMovePathCnt;

protected:
	void ApplyAniPositionToLocalPosition();
	void CheckGroundVelocity();
	void CalculateDummyPosition();
	virtual void ProcessFlocking();
	virtual void PositionProcess();
	virtual void RotationProcess();
	virtual bool CheckDefense();

	void CheckTrueOrbDefense();

	void ProcessCollsionProp();

	virtual float AddPos( float x, float y, float z );
	float SetPos( float x, float y );

	GameObjHandle GetProjecTileTarget( TroopObjHandle hTroop );		// AI에서 정하는게 나을지도

	virtual void CheckMoveLock( TroopObjHandle hTroop );		// 낑겼을 때 빠져나오게 하는 것
	void CheckUnitSpeed( TroopObjHandle hTroop );

	bool CheckDiagonalBlock( float fX, float fY );

	void SetSoxTableType();

	void GetSoundInfo(SOUND_PLAY_INFO *pType,ASSignalData *pSignal);
	bool IsNeedFloorInfoForSound(SOUND_PLAY_INFO *pType);
	void PlaySound(SOUND_PLAY_INFO *pInfo,bool b3D);
	bool IsSoundNoSkipUnit();
	void ProcessObjectStatus();
	void PostProcessObjectStatus();

	bool IsPlayerDamage();
	bool IsHero( int nSoxID );

public:
	virtual bool IsMovable();
	virtual bool IsAttackable();
	virtual bool IsJumpable();
	virtual bool IsFlockable( GameObjHandle Handle );
	virtual bool IsRidable();
	virtual bool IsAttack();
	virtual bool IsJump();
	virtual bool IsHit();
	virtual bool IsMove();
	virtual bool IsDown();
	virtual bool IsDie( bool bCheckEnable = false );
	virtual bool CheckHit();
	virtual bool IsStand();
	virtual bool IsTramplable();
	virtual bool IsDefensible();


	virtual void SetDie( int nWeaponDropPercent = 100 );

	float GetUnitRadius();
	float GetUnitHeight();

	virtual void Initialize( INITGAMEOBJECTDATA* pData, int nForce, int nTeam, TroopObjHandle hTroop, float fScaleMin=1.f, float fScaleMax=1.f );
	//	virtual void Initialize(int nUnitIndex, int nForce, int nTeam, TroopObjHandle hTroop, float fScaleMin=1.f, float fScaleMax=1.f);
	virtual float GetFrameAdd();

	void SetTroop( TroopObjHandle hTroop )	{ m_hTroop = hTroop; }

	bool IsCanMoveBlock( BYTE cAttr, BYTE cAttrEx );
	__forceinline void CheckAndAddPos( D3DXVECTOR2 &vResult, float fWidth, float fHeight );

	const D3DXVECTOR3 &GetPos() { return m_Cross.m_PosVector; }
	D3DXVECTOR2 GetPosV2() { return D3DXVECTOR2(m_Cross.m_PosVector.x, m_Cross.m_PosVector.z); }
	const D3DXVECTOR3 &GetPrevPos() { return m_PrevPos; }

	virtual bool IsHittable( GameObjHandle Handle );
	void CreateOrbObject( GameObjHandle HeroHandle, int nGenerateOrbType, int nOrbPercent );
	void CreateDropWeapon( int nDropIndex, float fForce );
	void DropWeapon();
	void DropArmor();
	void ChangeHitGravity();
	virtual void LookAtHitUnit( D3DXVECTOR3 *pDirection );
	void HitProcess();
	virtual bool CalculateDamage(int nAttackPower);
	virtual void CheckHitLinkObject( GameObjHandle Handle, int &nAniAttr, int &nAniType, int &nAniTypeIndex ) {}
	virtual void CalculateParticlePosition( CCrossVector &Cross );

	void AddPosCheckProp( float &x, float &z );
	void AddPosCheckAttribute( float x, float z );

	int GetAniTypeCount( int nAniAttr, int nAniType );
	void ChangeAnimation( int nAniType, int nIndex, int nAttr = 0, bool bOverlap = false, int nLoopCount = 1, bool bEvent = false );
	void ChangeAnimationByIndex( int nAniIndex, bool bOverlap = false, int nLoopCount = 1, bool bProcesSignal = true );
	void GetBox3( Box3 & B, AABB *pBoundingBox );

	// signal check Function
	void ProcessSignal();
	virtual void SignalCheck( ASSignalData *pSignal );
	void PreProcessSignal();
	void PreSignalCheck( ASSignalData *pSignal );
	bool CollisionCheck( GameObjHandle Handle, D3DXVECTOR3 *pStart, D3DXVECTOR3 *pEnd, float fAddLength );
	virtual void SendHitSignal( HIT_PARAM *pHitParam, GameObjHandle Handle );
	virtual void HitSuccess() {}
	void HitCheck(ASSignalData *pSignal);
	void HitCheckTemp( ASSignalData *pSignal );
	void ParticleCheck(ASSignalData *pSignal);
	void TerrainParticleCheck( ASSignalData *pSignal );
	void LinkedParticleCheck(ASSignalData* pSignal);
	void LinkedToCrossParticleCheck( ASSignalData* pSignal );
	void FXCheck(ASSignalData *pSignal);
	void SoundCheck(ASSignalData *pSignal);
	void SoundYeahCheck(ASSignalData *pSignal);
	void GotoCheck( ASSignalData *pSignal );
	void JumpCheck(ASSignalData *pSignal);
	void AddTrail( int nFrame, ASSignalData *pSignal, int nBookingIndex = 0, int nTrailHandle = 0 );
	void TrailCheck( ASSignalData *pSignal );
	void CameraShockCheck(  ASSignalData *pSignal );
	void JobChangeCheck( ASSignalData *pSignal );
	void MorphWeaponCheck( ASSignalData *pSignal );
	void PushCheck( ASSignalData *pSignal );
	void LinkedFXCheck( ASSignalData *pSignal );
	void AIControlCheck( ASSignalData *pSignal );
	void AIControlRelease( ASSignalData *pSignal );
	void AIControlGoto( ASSignalData *pSignal );
	void AIControlCallback( ASSignalData *pSignal );
	void SummonCheck( ASSignalData *pSignal );

	// 부모 Virtual Function
	virtual void Process();
	virtual void PostProcess();
	virtual bool Render();
	virtual void NextAnimation();
	virtual void CalculateMoveDir(KEY_EVENT_PARAM *pParam, int nCamIndex);
	virtual void DebugRender();


	// Virtual Funcition
	//virtual void CreateParts();
	virtual void CreateParts( INITGAMEOBJECTDATA* pData ); 

	virtual void CmdMove( int nX, int nY, float fSpeed = -1.f, int nAniType = ANI_TYPE_RUN, int nAniIndex = 0 ) {}
	virtual void CmdMove( GameObjHandle &Handle, float fLength = 0.f, float fSpeed = -1.f, int nAniType = ANI_TYPE_RUN, int nAniIndex = 0 ) {}
	virtual void CmdHit( HIT_PARAM *pHitParam, D3DXVECTOR3 *pHitDir = NULL );
	virtual void CmdPush() {}
	virtual void CmdStop( int nAniType = ANI_TYPE_STAND, int nAniIndex = 0 ) {}
	virtual void CmdAttack( GameObjHandle Handle ) {}
	virtual void CmdLookUp( GameObjHandle Handle ) {}
	virtual void CmdVictory( float fX, float fY ) {};
	virtual void CmdPlace( float nX, float nY );
	virtual void ResetMoveVector();
	void SetCrossVector( CCrossVector *pCross );

	virtual void Enable(bool bEnable, bool bFullHP = false);

	// Move Control Function
	void Move( D3DXVECTOR3 *pMove );	// 오브젝트 로컬 중심의 이동이 아니고.. 월드 기준 이동이다..
	virtual void MoveX(float fDist);
	virtual void MoveY(float fDist);
	virtual void MoveZ(float fDist);

	// Etc
	float GetHeightFromLand();
	D3DXVECTOR3 &GetDummyPos() { return m_DummyPos; }
	D3DXVECTOR2 GetDummyPosV2() { return D3DXVECTOR2( m_DummyPos.x, m_DummyPos.z ); }
	//	D3DXVECTOR3 &GetMoveTargetPos() { return m_MoveTargetPos; }
	D3DXVECTOR3 *GetMoveTargetPos() { return &m_MoveTargetPos; }
	D3DXVECTOR3 *GetMoveDir() { return &m_MoveDir; }
	D3DXVECTOR3 *GetMoveVector() { return &m_MoveVector; }
	D3DXVECTOR3 *GetLocalAniMove() { return &m_LocalAniMove; }
	void SetAniPosToLocalPosFlag( int nAniPosToLocalPosFlag ) { m_AniPosToLocalPosFlag = nAniPosToLocalPosFlag; }
	int GetAniPosToLocalPosFlag() { return m_AniPosToLocalPosFlag; }
	int GetUnitSoxIndex() { return m_nUnitSOXIndex; }
	std::vector< ARMOR_OBJECT > *GetArmorList() { return &m_ArmorList; }
	std::vector< WEAPON_OBJECT > *GetWeaponList() { return &m_WeaponList; }

	ASData *GetASData();
	CAniInfoData *GetUnitInfoData();
	void SetASData( ASData *pData );
	void SetUnitInfoData( CAniInfoData *pData );

	bool IsFreezePos() { return m_bFreezePos; }
	void SetFreezePos( bool bFlag ) { m_bFreezePos = bFlag; }

	bool IsOnProp() { return ( m_nOnPropIndex == -1 ) ? false : true ; }
	void SetOnPropIndex( int nIndex ) { m_nOnPropIndex = nIndex; }

	void SetFlocking( bool bFlag, float fMaxSize = 0.0f );
	CFcEventSequencerMng *GetEventSeqMng() { return m_pEventSeq; }

	void SetInScanner( bool bFlag );
	bool IsInScanner() { return m_bIsInScanner; }

	bool IsCatchable() { return m_bIsCatchable; }
	void SetCatchable( bool bCatch ) { m_bIsCatchable = bCatch; }

	bool IsIgnoreCritical() { return m_bIgnoreCritical; }
	void SetIgnoreCritical( bool bIgnore ) { m_bIgnoreCritical = bIgnore; }

	bool IsRevival() { return m_bRevival; }
	void SetRevival( bool bRevival ) { m_bRevival = bRevival; }
	void SetRevivalCondition( int nRevivalHP ){ m_nRevivalHPCondition = nRevivalHP; }

	int GetSpecialAttackCriticalAdd() { return m_nSpecialAttackCriticalAdd; }
	void SetSpecialAttackCriticalAdd( int nAdd ) { m_nSpecialAttackCriticalAdd = nAdd; }

	int GetSpecialAttackDistAdd() { return m_nSpecialAttackDistAdd; }
	void SetSpecialAttackDistAdd( int nAdd ) { m_nSpecialAttackDistAdd = nAdd; }

	bool IsBoss() { return m_bBoss; }
	void SetBoss( bool bBoss ) { m_bBoss = bBoss; }

	bool IsAutoGuard() { return m_bAutoGuard; }
	void SetAutoGuard( bool bGuard ) { m_bAutoGuard = bGuard; }

	bool IsForceGenerateOrb() { return m_bForceGenerateOrb; }
	void SetForceGenerateOrb( bool bForce ) { m_bForceGenerateOrb = bForce; }

	int GetAvoidCriticalRate() { return m_nAvoidCriticalRate; }
	void SetAvoidCriticalRate( int nRate ) { m_nAvoidCriticalRate = nRate; }


	void SetDieFrame( int nFrame ) { m_nDieFrame = nFrame; }
	int GetDieFrame() { return m_nDieFrame; }

	bool IsForceMove() { return m_bForceMove; }
	void SetForceMove( bool bFlag ) { m_bForceMove = bFlag; }

	void SetCurrentWeaponIndex( int nIndex ) { m_nCurrentWeaponIndex = nIndex; }
	int GetCurrentWeaponIndex() { return m_nCurrentWeaponIndex; }

	// AI
	void SetAIHandle( AIObjHandle &Handle );
	AIObjHandle GetAIHandle();

	// Variable Function
	void SetForce( int nForce )			{ m_nForce = nForce; }
	int GetForce()						{ return m_nForce; }
	void SetTeam( int nTeam )			{ m_nTeam = nTeam; }
	int GetTeam()						{ return m_nTeam; }
	int GetHP()							{ return (int)m_fHP; }
	float GetFloatHP()					{ return m_fHP; }
	void SetHP( int nHP )				{ m_fHP = (float)nHP; }
	void AddHPPercent( int nPercent );
	void AddHP(int nAddHP);
	void SetExp(int nExp)				{ m_nExp = nExp; }
	virtual void AddExp( int nExp, GameObjHandle Handle, int nKillCount = 0 );
	virtual void BeginStatus( CFcStatusObject::STATUS_TYPE Type, int nParam1 = 0, int nParam2 = 0, int nParam3 = 0, bool bOverlap = false );

	float GetRageDisposition()			{ return m_fRageDisposition; }
	void SetRageDisposition( float fValue ) { m_fRageDisposition = fValue; }
	//	void SetDefenseProb( int nValue ) { m_nDefenseProb = nValue; }
	//	int GetDefenseProb() { return m_nDefenseProb; }

	void SetLevel( int nLevel );
	__forceinline int GetLevel()
	{
		if( GetClassID() == Class_ID_Hero )
		{
			int nRet;
			nRet = m_nLevel + m_nLevelAdd;
			if( nRet >= MAX_HERO_LEVEL )
			{
				return MAX_HERO_LEVEL - 1;
			}
			return nRet;
		}
		else
		{
			return m_nLevel;
		}
	}
	int GetMaxHP();
	virtual int GetAttackPower();
	int GetDefense();
	int GetDefenseProb();
	int GetCritical();
	float GetCriticalRatio();

	void SetMaxHPAdd( int nAdd ,bool bPercent = true);
	int GetMaxHPAdd(bool bPercent = true );
	void SetAttackPowerAdd( int nAdd ) { m_nAttackPowerAdd = nAdd; }
	int GetAttackPowerAdd() { return m_nAttackPowerAdd; }
	void SetDefenseAdd( int nAdd ) { m_nDefenseAdd = nAdd; }
	int GetDefenseAdd() { return m_nDefenseAdd; }
	void SetDefenseProbAdd( int nAdd ) { m_nDefenseProbAdd = nAdd; }
	int GetDefenseProbAdd() { return m_nDefenseProbAdd; }
	void SetDamageRatioAdd( int nAdd ) { m_nDamageRatioAdd = nAdd; }
	int GetDamageRatioAdd() { return m_nDamageRatioAdd; }
	void SetDropHorse( bool bDrop ) { m_bDropHorse = bDrop; }
	bool GetDropHorse() { return m_bDropHorse; }
	void SetCriticalAdd( int nAdd ) { m_nCriticalAdd = nAdd; }
	int GetCriticalAdd() { return m_nCriticalAdd; }
	void SetCriticalRatioAdd( int nAdd ) { m_nCriticalRatioAdd = nAdd; }
	int GetCriticalRatioAdd() { return m_nCriticalRatioAdd; }
	void SetSpecialCriticalRatio( bool bDrop ) { m_bSpecialCriticalRatio = bDrop; }
	bool GetSpecialCriticalRatio() { return m_bSpecialCriticalRatio; }
	void SetExpAdd( int nAdd ) { m_nExpAdd = nAdd; }
	int GetExpAdd() { return m_nExpAdd; }
	void SetGuardBreak( int nBreak ) { m_nGuardBreak = nBreak; }
	int GetGuardBreak() { return m_nGuardBreak; }
	void SetBossGuardBreak( int nBreak ) { m_nBossGuardBreak = nBreak; }
	int GetBossGuardBreak() { return m_nBossGuardBreak; }
	bool IsGuardBreak( GameObjHandle Hitter );
	void SetAttackDistAdd( int nAdd ) { m_nAttackDistAdd = nAdd; }
	int GetAttackDistAdd() { return m_nAttackDistAdd; }
	void SetTrailLengAdd( int nAdd ) { m_nTrailLengAdd = nAdd; }
	int GetTrailLengAdd() { return m_nTrailLengAdd; }
	float CalcAttackDistAdd() { return m_nAttackDistAdd / 100.0f; }
	void SetGodMode( bool bMode ) { m_bGodMode = bMode; }
	void SetLevelAdd( int nAdd ) { m_nLevelAdd = nAdd; }
	void SetOrbSparkGodMode( bool bMode ) { m_bOnOrbsparkGodMode = bMode; }
	bool IsOrbSparkGodMode() { return m_bOnOrbsparkGodMode; }



	//	void SetMaxHP( int nMaxHP );
	//	void SetAttackPower( int nAttackPoint );
	//	void SetDefense(int nDefencePoint );


	int GetCurAniType();
	int GetCurAniAttr();
	int GetCurAniIndex();

	int GetDischargeOrbSpark();
	int GetGiveExp();

	static void OrbSparkOn();
	static void OrbSparkOff();

	CFcCorrelationMng *GetCorrelationMng() { return m_pCorrelationMng; }
	CFcCorrelation *GetCorrelation( CFcCorrelation::CORRELATION_TYPE Type ) { return m_pCorrelationMng->GetCorrelation( Type ); }

	TroopObjHandle GetParentTroop()	{ return m_hTroop; }
	D3DXVECTOR2 &GetTroopOffset() { return m_TroopOffsetPos; }
	void SetTroopOffset( D3DXVECTOR2* Pos ) { m_TroopOffsetPos = *Pos; }
	// Move Queue Function
	bool GetTroopMovePos( D3DXVECTOR3 &vResult );
	void CheckInTroopMovePos();
	int GetTroopMoveQueueCount() { return m_nTroopMoveQueueCount; }
	void SetTroopMoveQueueCount( int nCount ) { m_nTroopMoveQueueCount = nCount; }
	bool IsUseTroopMoveQueue();


	void SetParentLinkHandle( GameObjHandle Handle );
	GameObjHandle GetParentLinkHandle() { return m_ParentLinkObjHandle; }


	// 타기 -> 타는 놈 입장에서 탈놈을 변수로 넘겨준다.
	virtual bool RideOn( GameObjHandle RiderHandle );
	virtual bool RideOut();

	// 링크 -> 탈놈 입장에서 불려진다. 외부에서 부르는 경우는 없게 하자!!
	virtual bool LinkMe( GameObjHandle Handle, int nPlayerIndex = -1 ) { return false; }
	virtual bool UnLinkMe( GameObjHandle Handle ) { return false; }
	GameObjHandle GetLinkObjHandle( DWORD dwIndex ) { return m_VecLinkObjHandle[dwIndex]; }
	DWORD GetLinkObjCount() { return m_VecLinkObjHandle.size(); }

	// Game Play Member Function
	void GetGamePlayData();


	TroopObjHandle GetTroop()	{ return m_hTroop; }

	// 성문, 다리를 건너는 중인 상태
	int GetCrossState()					{ return m_nCross; }
	void SetCrossState( int nCross )	{ m_nCross = nCross; }

	void SetVelocity(D3DXVECTOR3 *pVelocity) { m_vVelocity = *pVelocity; }
	void SetVelocityX( float fVelocity ) { m_vVelocity.x = fVelocity; }
	void SetVelocityY( float fVelocity ) { m_vVelocity.y = fVelocity; }
	void SetVelocityZ( float fVelocity ) { m_vVelocity.z = fVelocity; }
	D3DXVECTOR3 *GetVelocity() { return &m_vVelocity; }
	float GetVelocityX() { return m_vVelocity.x; }
	float GetVelocityY() { return m_vVelocity.y; }
	float GetVelocityZ() { return m_vVelocity.z; }
	void SetGravity(float fGravity) { m_fGravity = fGravity; }
	float GetGravity() { return m_fGravity; }
	void SetGroundResist( float fValue ) { m_fGroundResist = fValue; }


	//사운드 관련
	void ProcessSound();

	void SetInvincible(bool bInvincible){ m_bInvincible = bInvincible; }


	void SetInvulnerable( bool bEnable )	{ m_bInvulnerable = bEnable; }

	virtual bool GetExactHitAni( int &nAniType, int &nAniIndex );
	virtual void GetExactHitVelocity( float &fVelocityZ, float &fVelocityY );
	virtual int FindExactAni( int nAniIndex );

	// debug 용 파츠 show/hide
	void ShowParts( bool bShow );

	virtual void LoadWeaponTrailTexture() {};
	virtual int	 GetWeaponTrailTextureID(int nIdx) {return -1;};
	virtual int  GetWeaponTrailOffsetTextureID() {return -1;};
	virtual void SetHitProbAdd( int nPer ){};

	D3DXVECTOR3 GetBone0Pos();

	float GetMoveSpeed(){ return m_fMoveSpeed; }
	void  SetMoveSpeed( float fSpeed ){ m_fMoveSpeed = fSpeed; }

	void SetBoost( bool bBoost )	{ m_bBoost = bBoost; }
	bool GetBoost()					{ return m_bBoost; }

	void SetMoveSpeedRate( float fRate )			{ m_fMoveSpeedRate = fRate; }
	void SetMoveSpeedRateForTroop( float fRate )	{ m_fMoveSpeedRateForTroop = fRate; }

	void SetAIControlIndex( int nIndex ) { m_nAIControlIndex = nIndex; }
	int GetAIControlIndex() { return m_nAIControlIndex; }

	void SetAniFrame( int nAniType, int nAniIndex, int nFrame );

	//Item 처리를 위해서 넣음
	/*
	virtual void SetMaxOrbSparkAdd( int nPercent ){};
	virtual void SetMoveSpeedAdd( int nPercent ){};
	virtual void SetOrbSparkSplitPercent( int nPercent ){};
	virtual void SetOrbSparkAdd( int nPercent ){};
	virtual void AddOrbSparkPercent(  int nPercent ){};
	*/
	void Reborn( float fHPPer = 1.f );

	// 이동 딜레이
	bool IsEnabledMoveDelay() const { return m_bMoveDelay; }
	void EnableMoveDelay(bool bEnable = true) { m_bMoveDelay = bEnable; }

	void SetRagDollMode( int nMode );
	void UnlinkRagdoll() { m_bRagdollUnlink = true; }

	HIT_PARAM *GetHitParam() { return &m_HitParam; }
	bool IsVigkVagk();

public:
	bool SaveMaterialAmbient();
	bool SetMaterialAmbient(D3DXVECTOR4 vColor);
	bool RestoreMaterialAmbient();

	float GetDistSqWayPoint();
	void ResetWayPoint()	{ m_nWayPointStartIndex = 0; m_nWayPointNum = 0; }
	int GetWayPointNum() { return m_nWayPointNum; }
	void PushWayPoint( D3DXVECTOR2* Pos );
	void PopWayPoint();
	bool GetCurWayPoint( D3DXVECTOR2& Pos );

protected:
	D3DXVECTOR4*		m_pSaveMaterialAmbient;

	int m_nWayPointStartIndex;
	int m_nWayPointNum;
	D3DXVECTOR2 m_WayPoint[5];
};
