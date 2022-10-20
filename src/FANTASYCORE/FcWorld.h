#pragma once

#include "Singleton.h"
#include "RangeScan.h"
#include "SmartPtr.h"
#include "FcGameObject.h"
#include "FcSpeech.h"
#include "BSFileManager.h"
#include "ASData.h"
#include "FcProp.h"
#include "FcTroopManager.h"
#include "./Data\\Sound\\FcSoundPlayDefinitions.h"
#include "FcCommon.h"


#define MAX_HIT_SIGNAL_COUNT 100

#define MAX_TIME_MARK	16
#define INVALID_TIME_MARK	0xcdcdcdcd

#define MAX_PARTICLE_LOAD_COUNT		200
#define MAX_FX_LOAD_COUNT			300

struct StageResultInfo;
struct AREA_INFO;
class CFcTroopManager;
class CFcPropManager;
class CFcTriggerCommander;
class CFcMinimap;
class NavigationMesh;
class NavCollisionMesh;
class FcInterfaceManager;
class NavigationCell;
class CFcAbilityManager;

class CFcHeroObject;
typedef CSmartPtr<CFcHeroObject> HeroObjHandle;

class CFcTroopObject;
typedef CSmartPtr<CFcTroopObject> TroopObjHandle;

struct PATH_PACK;
struct PROPTYPEDATA;

struct SET_INFO;
enum HIT_TYPE;
enum SOUND_ATTACK_TYPE;
enum SOUND_PROP_MATERIAL;

enum CHARINFO
{
	CHAR_ASPA = 0,
	CHAR_INPHY,
	CHAR_TURURU,
	CHAR_MYPHY,
	CHAR_TROLL,
	CHAR_GOBLIN,
	CHAR_MOKSA,
};

struct MISSION_OBJECTIVE
{
	MISSION_OBJECTIVE()
	{
		nID = -1;
		nTextID = -1;
		nClear = 0;
	}

	int nID;
	int nTextID;
	int nClear;
};

struct UNITLOADINFO 
{
	int nUnitIndex;
	int nSkinLevel;
	int nLevel;
	bool bEnemy;
};

struct AILOADINFO
{
	int nUnitIndex;
	std::string szAIName;
	int nLevel;
	bool bEnemy;
};


struct _FC_RANGE_DAMAGE
{
	GameObjHandle Attacker;
	D3DXVECTOR3 *pPosition;
	float fRadius;
	int nPower;
	int nPushY; 
	int nPushZ;
	int nTeam;
	int nNotGenOrb;
	int nNotLookAtHitUnit;
	bool p_bTeamDamage;
	D3DXVECTOR3 *pHitDirection;
	HIT_TYPE nType;
	float fYMax;
	float fYMin;
	SOUND_WEAPON_TYPE WeaponType;
	SOUND_ATTACK_TYPE SoundAttackType;
	int nOrbGenPer;
	int nHitRemainFrame;
	int nAdjutantHitRemainRatio;
	int	nGuardBreak;
	bool	bDisableDynamicProp;
	_FC_RANGE_DAMAGE()
	{
		D3DXVECTOR3 *pPosition = NULL;
		fRadius = 0.f;
		nPower = 0;
		nPushY = 0;
		nPushZ = 0;
		nTeam = -1;
		nNotGenOrb = GENERATE_ORB_NORMAL;
		nNotLookAtHitUnit = 0;
		p_bTeamDamage = false;
		pHitDirection = NULL;
		nType = HT_NORMAL_ATTACK;
		fYMax = 1000000.f;
		fYMin = -1000000.f;
		WeaponType =WT_NONE;
		SoundAttackType=ATT_SKIP;
		nOrbGenPer = 100;
		nHitRemainFrame = 10;
		nAdjutantHitRemainRatio = 1;
		bDisableDynamicProp = false;
		nGuardBreak = 0;
	}
};

typedef std::vector<MISSION_OBJECTIVE>	FcMissionObjList;

enum MISSION_FINISH_TYPE
{
	MISSION_FINISH_TYPE_NON = -1,
	MISSION_FINISH_TYPE_SUCCESS,
	MISSION_FINISH_TYPE_FAIL
};

struct NAVEVENTINFO
{
	int nStartIndex;
	int nNumNavIndex;
};

struct LIGHTSETDATA
{
	int nID;
	SET_INFO* pInfo;
};


struct SHOW_VAR_INFO
{
	char m_cVarTextStr[64];
	int m_nVarTextID;
};

struct SHOWEXTRAPARAM
{
	SHOWEXTRAPARAM()
	{
		Clear();
	}
	void Clear()
	{
		m_nCurShowVarID = -1;
		m_nCurShowGaugeID = -1;
	}

	int m_nCurShowVarID;
	std::vector<SHOW_VAR_INFO> m_vecShowVar;

	int m_nCurShowGaugeID;
	std::vector<int> m_vecShowGaugeVarID;
};



struct ObjectImpl
{
	struct Intface_Object
	{
		typedef GameObjHandle ID;
		typedef float  R;

		static R GetX(ID obj)	{ return obj->GetPos().x; }
		static R GetY(ID obj)	{ return obj->GetPos().z; }
		static bool Valid(ID obj) { return CFcBaseObject::IsValid(obj); }
	};

	RangeScan<Intface_Object>	ObjectScanner;
};


class CFcWorld : public CSingleton<CFcWorld>
{
public:
	CFcWorld();
	~CFcWorld();
	void Clear();

protected:
	CBSMapCore			*m_pMapInfo;

	ObjectImpl			*m_pObjectImpl;

	CFcTroopManager		*m_pTroopManager;
	CFcPropManager		*m_pPropManager;
	CFcTriggerCommander *m_pTriggerCommander;
	CFcAbilityManager	*m_pAbilityManager;

	HeroObjHandle	m_HeroHandle;
	int				m_nLightIndex;
	bool			m_bLoad;
	int				m_nCamIndex[2];

	std::vector<int> m_vecWaterList;
	std::vector<int> m_vecParticleGroupList;

	NavigationMesh* m_pNavigationMesh;
	NavCollisionMesh* m_pNavCollisionMesh;

	int m_nNumNavEventInfo;
	NAVEVENTINFO* m_pNavEventInfo;

	MISSION_FINISH_TYPE m_nMissionFinish;

	int m_nNumTimeMark;
	int m_TimeMark[MAX_TIME_MARK];
    int m_nFxSimplePlay;

	std::vector<LIGHTSETDATA> m_vecLightSet;

	int				m_nCurHitSignal;
	ASSignalData	m_HitSignal[ MAX_HIT_SIGNAL_COUNT ];

	int m_nStageLevel;
	LARGE_INTEGER	m_liStageStartTime;
//	LARGE_INTEGER	m_liStartPauseTime;
//	LARGE_INTEGER	m_liStagePauseTime;

	std::vector<INITGAMEOBJECTDATA> m_vecInitGameObj;
	std::vector< int > m_LoadedSkinList;
	std::vector< int > m_LoadedAniList;

	SHOWEXTRAPARAM m_ShowExtraParam;

	//---------Dynamic fog 임시 테스트용
	int				m_iDynamicFogID;
	int				m_iDarkDynamicFogID;
	std::vector<std::pair<int,int> > m_vecDarkDynFog;
	int				m_iRainID;
	
	FcMissionObjList m_vecMissionObjective;

	char m_VarTextStr[64];
	int m_VarTextID;
	int m_VarGaugeID;

	std::vector<TroopObjHandle> m_vecResultCalcTroops;

	// 크기가 있는 오브젝트들 레인지스켄 하기 위한 리스트
	std::vector<GameObjHandle> m_vecBigSizeScanner;

	bool m_bBackupForRM;
	bool m_bBackupAIForRM;

	int m_nRMStartTick;
	std::vector<TroopObjHandle> m_vecBackupTroopsForRM;
	std::vector<TroopObjHandle> m_vecBackupTroopsDisableAIForRM;

	int m_nUseParticleTable[ MAX_PARTICLE_LOAD_COUNT ];
	int m_nUseFXTable[ MAX_FX_LOAD_COUNT ];

	// 라이트/포그/씬 서서히 바꾸기용
	bool m_bChangeLightStart;		
	int	m_nChangeLightSec;
	int	m_nChangeLightOldTick;	
	D3DXVECTOR4 m_vCurDiffuse, m_vIncDiffuse,
				m_vCurSpecular, m_vIncSpecular,
				m_vCurAmbient, m_vIncAmbient;

	bool m_bChangeFogStart;
	int	m_nChangeFogSec;
	int	m_nChangeFogOldTick;	
	D3DXVECTOR4 m_vCurFogColor, m_vIncFogColor;
	int m_nChangeFogNear, m_nChangeFogFar, m_nChangeFogTick;
	
	bool m_bChangeSceneStart;
	int	m_nChangeSceneSec;
	int	m_nChangeSceneOldTick;	
	float m_fCurScene,
		m_fCurBlur,
		m_fCurGlow,
		m_fIncScene,
		m_fIncBlur,
		m_fIncGlow;
	
	std::vector<AREA_INFO> m_vecTroopEvnetArea;

	int m_nGuardianKillCount;
	int m_nGuardianOrb;

	int m_nBGMID;

	int m_nReducePhysicsDamage;

	bool m_bIsDemoSkip;
	int m_nDemoType;		// -1 : non, 0 : RM, 1 : Cut scene

	bool m_bUseTrueOrb;
	std::vector<TroopObjHandle> m_vecDamagedTrueOrbTroops;

public:
	int LoadWorld(const char *pFileName, int nStageLevel = 0 );
	void CreateHeightFieldActor();
	void CreateCamera( int nCameraCount, char* pDefaultCameraFileName, char* pUserCameraFileName );
	void CreateSky();
	HeroObjHandle CreateHero(int nUnitIndex, float fX, float fY);
	void CreateGuardianTroop( HeroObjHandle hHero );
	void CreateObjects();
	int LoadMeshData( int nSlot, const char *pFileName );
	int LoadAniData( int nSlot, const char *pFileName );
	void CreateProps();
	void CreateLight();
	void CreateNavigationMesh();
	void CreateWater(const char *pMapFileName);
	bool LoadParticleList();
	void ReleaseParticleList();
    void CreateFxToPlay( IN VectorFxStr* p_VecFxStr );
	void Process();
	void PostProcess() { if( m_pTroopManager ) m_pTroopManager->PostProcess(); }

	ObjectImpl *GetObjectImp() { return m_pObjectImpl; }

	void Update();
	void DebugRender();

	void SetFog( D3DXVECTOR4* vecFogColor, float fNear, float fFar );
	void SetAlphaBlocking( bool bEnable );

	HeroObjHandle GetHeroHandle() { return m_HeroHandle; }
	HeroObjHandle GetHeroHandle( int nIndex ) { return m_HeroHandle; }		// 컴팔 안되서 넣어놓음 yooty

	// 살아있는 놈들만 검색된다.
	void GetObjectListInRange( D3DXVECTOR3 *pPosition, float fRadius, std::vector<GameObjHandle> &Objects );
	void GetObjectListInRangeByTeam( D3DXVECTOR3 *pPosition, float fRadius, std::vector<GameObjHandle> &Objects, int nTeam );
	void GetEnemyObjectListInRange( int nMyGroup, D3DXVECTOR3 *pPosition, float fRadius, std::vector<GameObjHandle> &Objects, bool p_bIncludeDead = true );
	// 죽은 애들 검색
	void GetObjectListInRangeByDie( D3DXVECTOR3 *pPosition, float fRadius, std::vector<GameObjHandle> &Objects );

	void GetEnemyObjectListInRangeDir( int nMyGroup, D3DXVECTOR3 *pPosition, D3DXVECTOR3 *pDir, float fDotValue, float fRadius, std::vector<GameObjHandle> &Objects, float fMinimumDist = 0.0f);
	D3DXVECTOR3 GetEnemyObjectInRangeDirNear( int nMyGroup, D3DXVECTOR3 *pPosition, D3DXVECTOR3 *pDir, float fDotValue, float fRadius, int nCount = 0, float fMinimumDist = 0.0f );

	void AddUnitToScanner( GameObjHandle Unit );
	void RemoveUnitToScanner( GameObjHandle Unit );
	void MoveUnitToScanner(GameObjHandle Unit, D3DXVECTOR3 *pPrevPos);

	void GetTroopListInRange( D3DXVECTOR2 *pPosition, float fRadius, std::vector<TroopObjHandle> &Objects );
	void GetEnemyTroopListInRange( D3DXVECTOR2 *pPosition, int nTeam, float fRadius, std::vector<TroopObjHandle> &Objects );
	TroopObjHandle GetNearMeleeEnemyInRange( D3DXVECTOR2 *pPosition, int nTeam, float fRadius );		

	CBSMapCore* GetMapInfo() { return m_pMapInfo; }

	AREA_INFO* GetAreaInfo( char* pName );
	TroopObjHandle GetTroopObject( char* pName );
	TroopObjHandle GetTroopObject( int nIndex );

	CFcTroopManager* GetTroopmanager()			{ return m_pTroopManager; }

	void SetTroopAI( TroopObjHandle hTroop, int nAIID, bool bGuardianSkip = true );
	void SetPathToTroopAI( char* pPathName, int nTroopAIID );	
	void EnableTroopAI( TroopObjHandle hTroop, bool bEnable );

	PROPTYPEDATA* GetPropTypeData_( int index );
	CFcProp*  GetProp( char* pName );
	CFcPropManager *GetPropManager() { return m_pPropManager; }

	// Attribute Function
	// 너무 많이 불려서 inline으로 처리
	BYTE GetAttr( float fX, float fY ) {
		return m_pMapInfo->GetAttr( (int)fX / MAP_ATTR_SIZE, (int)fY / MAP_ATTR_SIZE );
	}
	BYTE GetAttr( D3DXVECTOR3 &vPos ) {
		return m_pMapInfo->GetAttr( (int)vPos.x / MAP_ATTR_SIZE, (int)vPos.z / MAP_ATTR_SIZE );
	}
	BYTE GetAttr( int nX, int nY ) {
		return m_pMapInfo->GetAttr( nX, nY );
	}

	BYTE GetAttrEx( float fX, float fY ) {
		return m_pMapInfo->GetAttrEx( (int)fX / MAP_ATTR_SIZE, (int)fY / MAP_ATTR_SIZE );
	}
	BYTE GetAttrEx( D3DXVECTOR3 &vPos ) {
		return m_pMapInfo->GetAttrEx( (int)vPos.x / MAP_ATTR_SIZE, (int)vPos.z / MAP_ATTR_SIZE );
	}
	BYTE GetAttrEx( int nX, int nY ) {
		return m_pMapInfo->GetAttrEx( nX, nY );
	}

	int GetAttrXSize() {
		return m_pMapInfo->GetAttrXSize();
	}
	int GetAttrYSize() {
		return m_pMapInfo->GetAttrYSize();
	}
	///////////////////////

	void GetMapSize(float &fWidth,float &fHeight);
	int GetTroopCount();

	int		GetPathCount();
	PATH_PACK *GetPathInfo(int nIndex);
	void LinkNavCells( std::vector<NavigationCell*>& navCells, int nNavEventInfoIndex );
	void UnlinkNavCells( int nNavEventInfoIndex );

	NavigationMesh* GetNavMesh()	{ return m_pNavigationMesh; }
	NavCollisionMesh *GetNavColMesh() { return m_pNavCollisionMesh; }

	void GetActiveProps( D3DXVECTOR3& Pos, float rRadius, std::vector<CFcProp*>& vecProps );
	void SetMissionComplete( MISSION_FINISH_TYPE Type )		{ m_nMissionFinish = Type; }

	// 작업해야 하는 함수들
	CFcTriggerCommander *GetTriggerCommander(){return m_pTriggerCommander;}

	void SetTriggerVar( int nVarID, int nValue );
	void SetTriggerEnable(int iTriggerIndex,bool bEnable);

	void StopTrigger();
	
	int GetTriggerVarInt( int nVarID );
	void SetEnableTroopInArea( float fSX, float fSY, float fEX, float fEY );
	void SetDisableEnemyTroopInArea( float fSX, float fSY, float fEX, float fEY );
	void SetEnableEnemyTroopInArea( float fSX, float fSY, float fEX, float fEY );
	void SetDisableFriendTroopInArea( float fSX, float fSY, float fEX, float fEY );
	void SetEnableFriendTroopInArea( float fSX, float fSY, float fEX, float fEY );
	
	void SetTimeMark( int nID );
	int GetTimeMark( int nID );

	void CmdGuardianAttack();
	void CmdGuardianDefense();
	bool CmdRegisterGuardian( int nIndex );

	int  GetPlayerCamIndex( int nPlayer );

	void SetBuildingAttrAtCrossAttr( PROPTYPEDATA* pData, DWORD dwRotAndScale, D3DXMATRIX* pMat, bool bSet );


	MISSION_FINISH_TYPE GetMissionFinishType()							{ return m_nMissionFinish; }
	void				SetMissionFinishType(MISSION_FINISH_TYPE nType)	{ m_nMissionFinish = nType; }

	void SetGuardianTroop( TroopObjHandle hTroop );

    void PlaySimpleFx(int nFxIndex,CCrossVector *pCross, int nLoop = 1); //Fx툴에서 제작된 Fx들을 위해서...
    void PlaySimpleParticleByParent( int nID, GameObjHandle hParent, bool bLoop, bool bIterate, float fScale, D3DXVECTOR3* pOffset=NULL);

	void PlaySimpleLinkedFx( int nFxIndex, GameObjHandle Handle, int nBoneIndex);
	void StopSimpleLinkedFx( GameObjHandle Handle, int nBoneIndex );
	void PlaySimpleFxByParent( int nFxIndex, GameObjHandle Handle, CCrossVector *pCross );

	void SetAbility( int nID, GameObjHandle hParent, DWORD Param1 = 0, DWORD Param2 = 0, DWORD Param3 = 0, DWORD Param4 = 0 );

	bool LoadLightSet( char* pFileName );
	int	GetLightIndex() const { return m_nLightIndex; }
	bool ChangeFogSet( int nLightSetID, int nTick );

	void CreateDynamicFog();

	// Light/Fog/Scene Chage Effect 용
	void ChangeLight(const D3DXVECTOR4 &vDiffuse, const D3DXVECTOR4 &vSpecular, const D3DXVECTOR4 &vAmbient, int nSec);
	bool IsChangeLightStop() const { return !m_bChangeLightStart; }
	void ChangeFog(const D3DXVECTOR4 &vColor, int nNear, int nFar, int nTick, int nSec);
	bool IsChangeFogStop() const { return !m_bChangeFogStart; }
	void ChangeScene(float fScene, float fGlow, float fBlur, int nSec);
	bool IsChangeSceneStop() const { return !m_bChangeSceneStart; }

	bool IsLoad()					{ return m_bLoad; }

	bool SetEnemyAttr( AREA_INFO* pInfo, bool bSet );
	void SetEnableDynamicProp( bool bEnable, float fSX, float fSY, float fEX, float fEY );

	void GiveRangeDamageLight( GameObjHandle Attacker, D3DXVECTOR3 *pPosition, float fMaxRadius, float fHeight, float fVXFactor, int nFuncType ,HIT_TYPE nType = HT_NORMAL_ATTACK,int nOrbGenType = GENERATE_ORB_DISABLE,int nOrbGenPer = 0);

	void GiveRangeDamage( _FC_RANGE_DAMAGE &rd,bool p_bAniDown =true, bool p_bIncludeDead = true );

	void GiveRangeDamageCustom( _FC_RANGE_DAMAGE &rd, bool p_bAniDown, float p_fPercent1, float p_fPercent2 );

	void GiveRangeDamage( GameObjHandle Attacker, D3DXVECTOR3 *pPosition, float fRadius, int nPower, int nPushY, 
		int nPushZ, int nTeam = -1 , int nNotGenOrb = 0, int nNotLookAtHitUnit = 0, bool p_bTeamDamage = false, D3DXVECTOR3 *pHitDirection = NULL,HIT_TYPE nType = HT_NORMAL_ATTACK, float fYMax = 1000000.f, float fYMin = -1000000.f,SOUND_WEAPON_TYPE WeaponType =WT_NONE,SOUND_ATTACK_TYPE SoundAttackType=ATT_SKIP, bool p_bAniDown = true );

	void GivePhysicsRange( GameObjHandle Attacker, D3DXVECTOR3* pPos, float fRadius, float fPower, int nDamage, PROP_BREAK_TYPE nType, bool bDisableDynamicProp = false );
	void GivePhysicsRange( GameObjHandle Attacker, D3DXVECTOR3* pPos, float fRadius, float fPower );
	void GiveDamage( GameObjHandle Attacker, GameObjHandle Hitter, int nPower, int nAniType, int nAniTypeIndex, D3DXVECTOR3 *pVelocity , int bNotGenOrb = 0, int nNotLookAtHitUnit = 0, D3DXVECTOR3 *pPosition = NULL,HIT_TYPE nType = HT_NORMAL_ATTACK,int nOrbGenPer = 0);

	void SetPlayerEnable( bool bEnable );
	float GetLandHeight( float fX, float fY, bool* pPropOut = NULL );	// mruete: prefix bug 648: renamed bProp -> bPropOut
	float GetLandHeight2( float fX, float fY, float p_fCharHeight, bool* pPropOut = NULL );	// mruete: prefix bug 648: renamed bProp -> bPropOut
	float GetLandHeightEx( float fX, float fY, D3DXVECTOR3* N = NULL );	// mruete: prefix bug 648: renamed bProp -> bPropOut
	float GetLandHeightEx2( float fX, float fY, float p_fCharHeight, bool bRender = false, D3DXVECTOR3* N = NULL );	// mruete: prefix bug 648: renamed bProp -> bPropOut
	int GetLandAttrIndex( float fX, float fY );

	void AddPosCheckCollisionProp( D3DXVECTOR3 Pos, float fHeight, float &fAddX, float &fAddZ );
	void AddPosCheckDynamicProp( D3DXVECTOR3 Pos, float &fAddX, float &fAddZ );

	char *GetMapFileName(){return m_pMapInfo->GetMapFileName();}

	int GetStagePlayTime();

	INITGAMEOBJECTDATA* GetInitGameObjectData( int nUnitIndex, int nLevel );

	int GetSimpleFXPlayIndex() { return m_nFxSimplePlay; }
	

	void AddObjective( int nID, int nTextID, bool bClear);
	void ClearObjective( int nID );
	void RemoveObjective( int nID );
	FcMissionObjList* GetMIssionObjective()			{ return &m_vecMissionObjective; }

	void ShowVarText( char* pStr, int nVarID );
	void HideVarText( int nVarID );
	void ShowVarGauge( int nVarID );
	void HideVarGauge( int nVarID );

	void AddResultCalcTroop( TroopObjHandle hTroop );

	float GetGuardianLiveRatePercent();
	float GetToopsLiveRatePercent(); //x 사용안함
	float GetFriendlyTroopsLiveRatePercent();

	bool IsProcessTrueOrb( int nTeam );
	void SetTrueOrbTimer( int nTime ) { m_nTrueOrbTimer = nTime; }

	void GetItemList( std::vector<CFcProp*>& vecItems );
	CFcAbilityManager *GetAbilityMng() { return m_pAbilityManager; }

	// RealMovie 용
	void	SaveLight(void);
	void	RestoreLight(void);
	void	UpdateLightForRealMovie(CCrossVector* pCross);

	void SetMarkPoint( D3DXVECTOR2* pPos );

	CCrossVector				m_crossOldLight;
	int							m_nFXIndex;

	//---------Dynamic fog용
	int GetFXDynamicFogID(void) const { return m_iDynamicFogID; }
	void SetFXDynamicFogID(int iId) { m_iDynamicFogID = iId; }

	int GetFXDarkDynamicFogID(void) const { return m_iDarkDynamicFogID; }
	void SetFXDarkDynamicFogID(int iId) { m_iDarkDynamicFogID = iId; }
	
	void AddDarkDynamicFog(int index, int iDynamicFogID);
	int FindDarkDynamicFog(int index);
	void EraseDarkDyanmicFog(int index);
	//--------Rain fx 용
	int GetFXRainID() const { return m_iRainID; }
	void SetFXRainID(int iId) { m_iRainID=iId; }

	int GetStageLevel() { return m_nStageLevel; }

	void StartRealmovie( float fSX, float fEX, float fSY, float fEY );
	void StartTroopAIDisableForRealmovie( float fSX, float fEX, float fSY, float fEY );

	//--------
	void EnableBlizzardEffect(int nAlpha, int nIntervalTick); // 0~100
	void DisableBlizzardEffect();

	void EnableVigBlurEffect(); 
	void DisableVigBlurEffect();

	// 크기 큰 놈들 래인지 스캔 보조
	void AddBigSizeScanner( GameObjHandle &Handle );
	void RemoveBigSizeScanner( GameObjHandle &Handle );

	void AddTroopEventArea( AREA_INFO* pArea );
	void SetTroopEventArea( TroopObjHandle hTroop, TroopObjHandle hTargetTroop, float fDist );

	int GetGuardianKillCount()					{ return m_nGuardianKillCount; }
	void AddGuardianKillCount( int nAdd )		{ m_nGuardianKillCount += nAdd; }
	int GetGuardianOrb()						{ return m_nGuardianOrb; }
	void AddGuardianOrb( int nAdd )				{ m_nGuardianOrb += nAdd; }

	void SetBGMID( int nBGMID )					{ m_nBGMID = nBGMID; }
	int GetBGMID()								{ return m_nBGMID; }
	bool CanIPlayBGM();
	void PlayLastBGM();

	void SetGuardianTroopDefenseAdd( int nRate );
	void SetGuardianTroopAttackAdd( int nRate );
	void SetGuardianTroopMoveSpeedAdd( int nRate );
	void SetGuardianTroopMaxHPAdd( int nRate );
	void SetGuardianTrapDamageAdd( int nRate );
	void AddGuardianHP( int nRate, bool bPercent = true );
	void SetEnemyRangeProbbtAdd(int nPer);

	void SetReducePhysicsDamage( int nRate )	{ m_nReducePhysicsDamage = nRate; }
	int GetReducePhysicsDamage()				{ return m_nReducePhysicsDamage; }

	bool IsDemoSkip()							{ return m_bIsDemoSkip; }
	void SetDemoSkip( bool bSkip )				{ m_bIsDemoSkip = bSkip; }
	void SetDemoType( int nType )				{ m_nDemoType = nType; }
	int GetDemoType()							{ return m_nDemoType; }

	void SetHeroEventBlockArea( bool bFlag )	{ m_bIsHeroEventBlockArea = bFlag; }
	bool IsHeroEventBlockArea()					{ return m_bIsHeroEventBlockArea; }

	int GetCurMapLevel();

	bool IsUseTrueOrb()							{ return m_bUseTrueOrb; }
	void InitTrueOrbDamagedTroops();
	void FinishTrueOrbDamagedTroops();
	void AddTroopDamagedTrueOrb( TroopObjHandle hTroop );

	void ResetUnitAILOD();

	//_TNT_
#ifdef _TNT_
	//JKB: hook added for automation: warps during random stress to nearest enemy
	bool WarpHeroToEnemy(int heroIndex, float range);
#endif
	int m_nTrueOrbTimer;
	GameObjHandle m_UseTrueOrbHandle;

	int	m_nBlizzardEffectIdx;
	int	m_nBlizzardEffectTexIdx;
	int m_nVigBlurEffectIdx;

	std::vector<int>		m_vecFxObj;
	bool					m_bIsHeroEventBlockArea;

	StageResultInfo*		m_pCurStageResultInfo;

protected:
	bool LoadTrigger( const char* pFileName );
	D3DXVECTOR2 GetCameraDir( int nPlayer );
	void CheckUnitUpdateObject();
	int CalcPlayerCount();
	bool LoadUnitData();
//	bool IsUnitIndex( int nType, int nSkinLevel, bool bEnemy, std::vector<UNITLOADINFO>& vecUnitIndex );
	void CheckAndAddUnitPreLoad( int nType, int nSkinLevel, int nLevel, bool bEnemy, std::vector<UNITLOADINFO>& vecUnitIndex );
	void CheckAndAddAIPreLoad( int nType, char *szAIName, int nLevel, bool bEnemy, std::vector<AILOADINFO>& vecAIList );
	int GetSkinLevel( int nType, int nLevel );
	void ProcessExtraInterfaceParam();


	// Pre Fx Loading 관련
	void	MainHeroFxLoading( int p_nClassId, bool p_bPlayer = true );


};

extern CFcWorld		g_FcWorld;
