#pragma once

class CCrossVector;

#include "Input.h"
#include "FcAbilityManager.h"
#include "FcUnitObject.h"
#include "FcProp.h"

#define MAX_TRUE_ORB_SPARK		2000
#define MAX_ORB_SPARK			500

#define DEFAULT_COMBO_KEEP_FRAME	60

#define FALL_SUCCESS_FRAME		7

#define DASH_STOP_FRAME 120

class CFcHeroObject : public CFcUnitObject
{
public:
	CFcHeroObject(CCrossVector *pCross);
	virtual ~CFcHeroObject();

protected:
	GameObj_ClassID m_HeroClassID;
	int m_nPlayerIndex;

	int m_nFallFrame;
	int m_nLoopInputCount;
	BOOL m_bUsedInput;
	int m_nInput;
	int m_nInputPressCount[ PAD_INPUT_COUNT ];

	int m_nComboCount;
	int m_nComboKeepFrame;
	int m_nOrbKillCount;
	int m_nMaxOrbKillCount;
	int m_nKillCount;
	int m_nTrueOrbSpark;
	float m_fOrbSparkFrame;
	int m_nTotalOrbSpark;
	int m_nOrbSpark;
	int m_nMaxOrbSparkAdd;
	int	m_nOrbSparkAdd;
	int m_nOrbSparkAddWithTrueOrb;
	int m_nTrueOrbSparkAdd;
	int m_nOrbSplitPercent;
	float m_fOrbAttackKeepSpeed;
	bool m_bEnableTrueOrbSpark;
	int m_nOrbSparkOn;
	bool m_bChargeMode;

	int m_nMotionBlurIndex;
	int m_nDashFrame;

	FC_ABILITY m_nOrbSparkType;

	int m_nCatchBoneIndex;
	D3DXVECTOR3 m_UnlinkVelocity;
	GameObjHandle m_CatchHandle;


	std::vector<int> m_WeaponTrailTexIDs;
	int				 m_nWeaponTrailOffsetTexID;

	float m_fJumpSpeed;
	bool	m_bOrbSpark;

	bool m_bStopOrb;

	int m_nDieType;	// 0 : 걍 뒤진다. 1 : 쓰러진다.. HERO_RENEW_FRAME 후에 다시 일어나 싸운다. 2 : 쓰러져 있는다..
	int m_nDieRenewFrame;
	float m_fOnlyPlayerDamage;


// Attribute Add
	int m_nMoveSpeedAdd;
	bool m_bTrampleDamage;
	int m_nDashLengthAdd;
	bool m_bEnableDash;
	bool m_bEnableJump;
	bool m_bEnableDefense;
	bool m_bNotUseOrbSpecialAttack;
	bool m_bAlwaysSmallDamage;
	int m_nSpecialAttackUseOrb;
	int m_nOrbAttackPowerAdd;
	bool m_bChargeModeGod;
	bool m_bAutoOrbAttack;
	bool	m_bTrueOrbSparkActivity;

public:
	GameObj_ClassID GetHeroClassID() { return m_HeroClassID; }
	void SetHeroClassID( GameObj_ClassID ClassID ) { m_HeroClassID = ClassID; }

	virtual void Initialize( INITGAMEOBJECTDATA* pData, int nForce, int nTeam, TroopObjHandle hTroop, float fScaleMin, float fScaleMax );
	void SetPlayerIndex(int nIndex) { m_nPlayerIndex=nIndex; }
	int GetPlayerIndex() { return m_nPlayerIndex; }
	virtual void ProcessKeyEvent(int nKeyCode, KEY_EVENT_PARAM *pKeyParam);
	//void CreateParts();
	void Enable(bool bEnable, bool bFullHP = false);

	bool	GetOrbSparkActivity() { return m_bTrueOrbSparkActivity;	};
	void	SetOrbSparkActivity(bool p_b) { m_bTrueOrbSparkActivity = p_b;	};

	// 부모 Virtual Function
	virtual void Process();
	virtual bool Render();
	virtual void SignalCheck( ASSignalData *pSignal );
	virtual void StartOrbSpark();
	virtual void StopOrbSpark();
	virtual void PostProcess();
	virtual void ProcessFlocking();
	virtual void NextAnimation();
	virtual void SendHitSignal( HIT_PARAM *pHitParam, GameObjHandle Handle );
	void HitSuccess();
	virtual bool CheckDefense();

	virtual bool IsHittable( GameObjHandle Handle );
	virtual bool IsFlockable( GameObjHandle Handle );

	virtual bool RideOn( GameObjHandle RiderHandle );
	virtual bool RideOut();
	virtual bool OrbAttackFxOn()	 { return true; };
	virtual bool OrbAttackFxOff()	 { return true; };
	virtual void OrbAttack()	{};
	virtual void NormalAttack()	{};
	void ResetKeyProcess();


	int GetAttackPower();
	virtual void CmdMove( int nX, int nY, float fSpeed = -1.f, int nAniType = ANI_TYPE_RUN, int nAniIndex = 0 );
	virtual void CmdStop( int nAniType = ANI_TYPE_STAND, int nAniIndex = 0 );

	int GetMaxOrbKillCount() { return m_nMaxOrbKillCount; }
	void AddKillCount( int nAdd, GameObjHandle Handle );
	int GetKillCount()		{ return m_nKillCount; }
	void DropPrizeItem( int nItemIndex );
	int GetComboCount()		{ return m_nComboCount; }
	void AddExp( int nExp, GameObjHandle Handle, int nKillCount = 0 );
	int GetEXP()			{ return m_nExp; }
	int GetNextExp();

	virtual bool CalculateDamage(int nAttackPower);

	void GotoPressCount( ASSignalData *pSignal );
	bool IsInHitRange( ASSignalData *pSignal, D3DXVECTOR3 *pPosition );
	bool AddDynamicForce( CFcProp *pProp, int nAttackPoint, PROP_BREAK_TYPE Type );
	void HitCheckTempProp( ASSignalData *pSignal );
	void InputCheck( ASSignalData *pSignal );
//	void AIInputCheck( ASSignalData *pSignal );
	void GotoPressCheck( ASSignalData *pSignal );
//	void AIGotoPressCheck( ASSignalData *pSignal );
	virtual	void PhysicsCheck( ASSignalData *pSignal );
	void TrueOrbCheck( ASSignalData *pSignal );
	
	bool IsOrbSparkOn() 
	{ 
		if( ( m_bOrbSpark ) || ( m_bKeepOrbSpark ) )
		{
			return true; 
		}
		return false;
	}
	void SetOrbSpark( int nOrb ) { m_nOrbSpark = nOrb; }
	void AddOrbSparkPercent( int nPercent );
	void AddOrbSpark( int nOrb, bool bUseSplit = true, bool bAddTotal = true );
	void AddTrueOrbSpark( int nOrb, bool bUseSplit = true, bool bAddTotal = true  );	

	void SetOrbSparkAdd( int nPercent ) { m_nOrbSparkAdd = nPercent; }
	int GetOrbSparkAdd() { return m_nOrbSparkAdd; }
	void SetOrbSparkAddWithTrueOrb( int nPercent ) { m_nOrbSparkAddWithTrueOrb = nPercent; }
	int GetOrbSparkAddWithTrueOrb() { return m_nOrbSparkAddWithTrueOrb;}
	
	void SetTrueOrbSparkAdd( int nPercent ) { m_nTrueOrbSparkAdd = nPercent; }
	int GetTrueOrbSparkAdd() { return m_nTrueOrbSparkAdd; }
	void SetOrbSparkSplitPercent( int nPercent ) { m_nOrbSplitPercent = nPercent; }
	int GetOrbSparkSplitPercent() { return m_nOrbSplitPercent; }
	float GetOrbAttackKeepSpeed() { return m_fOrbAttackKeepSpeed; }
	void SetOrbAttackKeepSpeed( float fSpeed ) { m_fOrbAttackKeepSpeed = fSpeed; }
	
	void EnableTrueOrbSpark( bool bEnable ) { m_bEnableTrueOrbSpark = bEnable; }
	bool IsEnableTrueOrbSpark() { return m_bEnableTrueOrbSpark; }

	int GetNumOrbSparkOn() const { return m_nOrbSparkOn; }

	int GetOrbSpark() { return m_nOrbSpark; }
	float GetOrbSparkFrame() { return m_fOrbSparkFrame; }
	int CalcMaxOrbSpark() { return ( int )( MAX_ORB_SPARK * ( 1.0f + m_nMaxOrbSparkAdd / 100.0f ) ); }
	void SetMaxOrbSparkAdd( int nAdd );
	int GetMaxOrbSparkAdd() { return m_nMaxOrbSparkAdd; }
	int GetTrueOrbSpark() { return m_nTrueOrbSpark; }
	void SetTrueOrbSpark(int n) { m_nTrueOrbSpark = n; }
	int GetMaxTrueOrbSpark() { return MAX_TRUE_ORB_SPARK; }

	int GetTotalOrbSpark() { return m_nTotalOrbSpark; }

	virtual float GetFrameAdd();
	bool RideHorse();
	void CheckFall();
	bool ProcessFall();
	void LookAtNearestEnemy( KEY_EVENT_PARAM *pParam, int nCamIndex );

	void SetEnableAI( bool bEnable );
	void AIInput( int nKeyCode );

	virtual bool Catch( GameObjHandle Handle ) { return false; }
	virtual void UnlinkCatchObject( int nX, int nY, int nZ, bool bEvent = false, int nHandIndex = -1 );
	void ProcessUnlink( GameObjHandle Handle, int nX, int nY, int nZ );
	void UnlinkCatch( ASSignalData *pSignal );
	virtual void CmdHit( HIT_PARAM *pHitParam, D3DXVECTOR3 *pHitDir = NULL );
	void CmdPush();
	GameObjHandle GetCatchHandle() { return m_CatchHandle; }
	virtual void SendCatchMsg() {}
	virtual void SendUnlinkCatchMsg( int nHandIndex = -1 ) {}


	int GetWeaponTrailTextureID(int nIdx);
	int GetWeaponTrailOffsetTextureID() {return m_nWeaponTrailOffsetTexID;};
	void ReleaseWeaponTrailTexture();

	virtual bool GetExactHitAni( int &nAniType, int &nAniIndex );
	virtual int FindExactAni( int nAniIndex );

	bool IsChangableWeapon();
//	void ChangeWeapon( char *pSkinName, int nAttackPower );
	virtual bool ChangeWeapon( int nSkinIndex, int nAttackPower );

	float CalcMoveSpeedAdd() { return 1.0f + m_nMoveSpeedAdd / 100.0f; }
	int GetMoveSpeedAdd() { return m_nMoveSpeedAdd; }
	void SetMoveSpeedAdd( int nAdd ) { m_nMoveSpeedAdd = nAdd; }

	virtual void InitOrbAttack2DFx();
	virtual void Resume();
	virtual void SetDie( int nWeaponDropPercent = 100 );
	virtual bool IsDie( bool bCheckEnable = false );

	void SetDieType( int nType ) { m_nDieType = nType; }
	int GetDieType() { return m_nDieType; }
	void SetOnlyPlayerDamageHPPercent( float fPercent ) { m_fOnlyPlayerDamage = fPercent; }

	void EnableInphyTrampleDamage( bool bEnable ) { m_bTrampleDamage = bEnable; }
	void SetDashLengthAdd( int nAdd ) { m_nDashLengthAdd = nAdd; }
	int GetDashLengthAdd() { return m_nDashLengthAdd; }
	void SetEnableDash( bool bEnable ) { m_bEnableDash = bEnable; }
	bool IsEnableDash() { return m_bEnableDash; }
	void SetEnableJump( bool bEnable ) { m_bEnableJump = bEnable; }
	bool IsEnableJump() { return m_bEnableJump; }
	void SetEnableDefense( bool bEnable ) { m_bEnableDefense = bEnable; }
	bool IsEnableDefense() { return m_bEnableDefense; }
	void SetNotUseOrbSpecialAttack( bool bEnable ) { m_bNotUseOrbSpecialAttack = bEnable; }
	bool IsNotUseOrbSpecialAttack() { return m_bNotUseOrbSpecialAttack; }
	void SetAlwaysSmallDamage( bool bEnable ) { m_bAlwaysSmallDamage = bEnable; }
	bool IsAlwaysSmallDamage() { return m_bAlwaysSmallDamage; }
	int GetSpecialAttackUseOrb() { return m_nSpecialAttackUseOrb; }
	void SetSpecialAttackUseOrb( int nUse ) { m_nSpecialAttackUseOrb = nUse; }
	bool IsChargeModeGod() { return m_bChargeModeGod; }
	void SetChargeModeGod( bool bGod ) { m_bChargeModeGod = bGod; }
	bool IsAutoOrbAttack() { return m_bAutoOrbAttack; }
	void SetAutoOrbAttack( bool bGod ) { m_bAutoOrbAttack = bGod; }
	int GetOrbAttackPowerAdd() { return m_nOrbAttackPowerAdd; }
	void SetOrbAttackPowerAdd( int nAdd ) { m_nOrbAttackPowerAdd = nAdd; }

	int GetOrbPercent();
	//_TNT_
#ifdef _TNT_
	//JKB: hook added for automation: warps during random stress
	float HeroWarp(float x, float y) {return SetPos(x,y);};
#endif

};

typedef CSmartPtr<CFcHeroObject> HeroObjHandle;