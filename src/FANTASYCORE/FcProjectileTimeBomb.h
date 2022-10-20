#pragma once

#pragma once

#include "FcProjectile.h"
#include "ASData.h"

class CFcProjectileTimeBomb : public CFcProjectile
{
public:
	CFcProjectileTimeBomb( CCrossVector *pCross );
	virtual ~CFcProjectileTimeBomb();

	static ASSignalData *s_pHitSignal;
protected:
	int m_nFxObjectIndex[2];
	int m_nDestroyTick;
	int m_nDamageTick;
	int m_nParentTeam;
	int m_nDamageCount;
	int m_nDamageDelay;

	void ProcessDamage();
public:
	int Initialize( int nStartFxIndex, int nExpFxIndex, int nStartDamageTime, int nDamageCount, int nDamageTick, PROJECTILE_PARAM *pParam );
	virtual void Process();
	virtual bool Render();
};