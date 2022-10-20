#pragma once
#include "FcAdjutantObject.h"
#include "FcProjectile.h"
#include "FcProjectileBlackhall.h"

class CFcAdjutant99King : public CFcAdjutantObject
{
public:
	CFcAdjutant99King(CCrossVector *pCross);
	virtual ~CFcAdjutant99King();

protected:
	bool m_bFloating;
	bool m_bAddHeight;

	bool m_bLinkFx;
	int m_nLinkFxIndex[2];

	CFcVelocityCtrl *m_pVelocityCtrl;
	ProjetileHandle m_BlackHall;

	D3DXVECTOR3 m_TentaclePos;
	int m_nTentacleRemainCount;
	int m_nTentacleInterval;
	int m_nTentacleRandomRange;
	int m_nTentaclePrevTick;
	ASSignalData *m_pTentacleSignal;

public:
	// ºÎ¸ð Virtual Function
	virtual void Process();
	virtual void PostProcess();
	virtual bool Render();
	virtual void SignalCheck( ASSignalData *pSignal );
	virtual bool GetExactHitAni( int &nAniType, int &nAniIndex );
	virtual void GetExactHitVelocity( float &fVelocityZ, float &fVelocityY );
	virtual int FindExactAni( int nAniIndex );
	virtual float AddPos( float x, float y, float z );
	virtual void CmdLookUp( GameObjHandle Handle );
	virtual void CmdStop( int nAniType, int nAniIndex );
	virtual void LookAtHitUnit( D3DXVECTOR3 *pDirection );
	virtual bool CheckDefense();
	virtual bool IsFlockable( GameObjHandle Handle );
	virtual bool CalculateDamage(int nAttackPower);
	virtual void ProcessFlocking();

protected:
	void ProcessMove();
	float GetStopableDistance();
	void CheckBlackHall();
	void CheckTentacle();

	void ProjectileBlackHall( ASSignalData *pSignal );
	void ProjectileEnergyBall( ASSignalData *pSignal );
	void ProjectileTentacle( ASSignalData *pSignal );
	void SiganlCheckWarp( ASSignalData *pSignal );
};