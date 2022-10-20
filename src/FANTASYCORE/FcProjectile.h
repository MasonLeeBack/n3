#pragma once

#include "FCBaseObject.h"
#include "FcCommon.h"

class CFcTroopObject;
typedef CSmartPtr<CFcTroopObject> TroopObjHandle;

class CFcProjectile : public CFcBaseObject
{
public:
	CFcProjectile( CCrossVector *pCross );
	virtual ~CFcProjectile();

	enum PROJECTILE_TYPE {
		ARROW,
		STONE,
		PHYSICS_THROW,
		THROW_STONE,
		DIRECT_ARROW,
		WATERBOMB,
		MAGIC_FX,
		TIMEBOMB,
		ROOTS,
		BLACKHALL,
	};

	static CSmartPtr< CFcProjectile > CreateObject( PROJECTILE_TYPE Type, CCrossVector *pCross );
	static CFcProjectile* GetOneMovingProjectile( TroopObjHandle hTargetTroop );
	static void PhysicsCheck( ASSignalData *pSignal, GameObjHandle Handle, bool p_bPush = false );
	static void ReleaseSignal();
	HIT_PARAM MakeHitParam( ASSignalData *pSignal );

	virtual void SetCustomValue( void* p_pValue ) { };
protected:
	static D3DXVECTOR3 s_gravity;
	PROJECTILE_PARAM m_Param;
	int	   m_nExplotionFxIndex; //사운드 때문에 추가

protected:
	D3DXVECTOR3	GetVelocity( const D3DXVECTOR3& pos, const D3DXVECTOR3& accel, float angle, D3DXVECTOR3 vTarget, bool bAverageDir = true );
	bool CheckHitGround();
	float GetLandHeight();

public:
	virtual int Initialize( int nSkinIndex, PROJECTILE_PARAM *pParam );
	virtual void Process();
	virtual void AddForce(D3DXVECTOR3*) {};
	virtual void AddForce2(D3DXVECTOR3*) {};

	TroopObjHandle GetTargetTroop();
	
	static void GetProjectileList(int p_nRtti, std::vector<CFcProjectile*> &p_vecPro );
	static void GetProjectileList2(int p_nRtti1, int p_nRtti2, std::vector<CFcProjectile*> &p_vecPro );
protected:
	static std::vector< CFcProjectile* > s_Objs;
};

typedef CSmartPtr< CFcProjectile > ProjetileHandle;
