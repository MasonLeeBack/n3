#pragma once

#include "FcBaseObject.h"
#include "FcGameObject.h"
#include "FcHeroObject.h"

class ASSignalData;
class CVigkvagk : public CFcHeroObject
{
public:
	CVigkvagk( CCrossVector *pCross );
	virtual ~CVigkvagk();

protected:
	static int s_nRotationAngle;
	bool m_bRotation;
	bool m_bBackAttack;
	GameObjHandle m_LeftCatchHandle;

	virtual void RotationProcess();
	bool PickupWeapon();
	bool CheckLevel( CFcProp *pProp );
	int m_nRotationAngle;
	bool m_bApplyDamage;
	bool m_bRotationFlag;

	static int s_nRefCount;
	static ASSignalData *s_pHitSignal;
public:
	virtual void Initialize( INITGAMEOBJECTDATA* pData, int nForce, int nTeam, TroopObjHandle hTroop, float fScaleMin, float fScaleMax );
	virtual float GetFrameAdd();
	virtual void Process();
	virtual void ProcessFlocking();
	virtual void MoveZ(float fDist);
	virtual void ProcessKeyEvent(int nKeyCode, KEY_EVENT_PARAM *pKeyParam);
	virtual int FindExactAni( int nAniType );
	virtual void SignalCheck( ASSignalData *pSignal );
	virtual bool Catch( GameObjHandle Handle );
	bool CatchLeftHand( GameObjHandle Handle );
	bool CatchRightHand( GameObjHandle Handle );
	void UnlinkCatchObject( int nX, int nY, int nZ, bool bEvent = false, int nHandIndex = -1 );

	virtual void CmdMove( int nX, int nY, float fSpeed = -1.f, int nAniType = ANI_TYPE_RUN, int nAniIndex = 0 );
	virtual void CmdStop( int nAniType = ANI_TYPE_STAND, int nAniIndex = 0 );
	virtual bool CheckDefense();

	virtual bool CalculateDamage(int nAttackPower);
	virtual void CalculateParticlePosition( CCrossVector &Cross );

	void SendCatchMsg();
	void SendUnlinkCatchMsg( int nHandIndex = -1 );
	virtual bool IsFlockable( GameObjHandle Handle );

//	void ProcessMove();

	virtual bool GetExactHitAni( int &nAniType, int &nAniIndex );
	virtual void GetExactHitVelocity( float &fVelocityZ, float &fVelocityY );
	virtual void LookAtHitUnit( D3DXVECTOR3 *pDirection );
};
