#pragma once

#include "FcProjectile.h"
#include "ASData.h"
class CFcVelocityCtrl;

class CFcProjectileMagicFx : public CFcProjectile
{
public:
	CFcProjectileMagicFx( CCrossVector *pCross );
	virtual ~CFcProjectileMagicFx();

	static ASSignalData *s_pHitSignal;
protected:
	int m_nFxObjectIndex[2];
	bool m_bHidePosition;
	int m_nDestroyTick;
	bool m_bDestroy;
	int m_nParentTeam;
	int m_nTargetType;
	bool m_bExplosion;
	CFcVelocityCtrl *m_pVelocity;
	int m_nHitType;
protected:
	void ProcessDamage();
	bool CheckExp();
public:
	int Initialize( int nThrowFxIndex, int nExpFxIndex, PROJECTILE_PARAM *pParam );
	virtual void Process();
	virtual bool Render();

	CFcVelocityCtrl *GetVelocityCtrl() { return m_pVelocity; }
	void SetDirectHideThrowFx( bool bFlag ) { m_bHidePosition = !bFlag; }
	void SetHitType( int nAniType );
	void SetDestroyTick( int nValue ) { m_nDestroyTick = nValue; }
};