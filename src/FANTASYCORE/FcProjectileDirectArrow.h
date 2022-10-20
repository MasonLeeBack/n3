#pragma once

#include "FcProjectile.h"
#include "ASData.h"

class CFcProjectileDirectArrow : public CFcProjectile
{
public:
	CFcProjectileDirectArrow( CCrossVector *pCross );
	virtual ~CFcProjectileDirectArrow();

	static ASSignalData *s_pHitSignal;
	void SetFxID( int nFxIndex ) { m_nFxid = nFxIndex; }
	void SetPierce( bool bEnable ) { m_bPierce = bEnable; }
	void SetDestroyTick( int nValue ) { m_nDestroyTick = nValue; }
protected:

	int m_nFXLineIndex;
	int m_nTrailLifeTick;
	int m_nDestroyTick;
	int m_nParentTeam;

	float m_fVelocity;
	D3DXVECTOR3 m_PrevPos;

	int	m_nFxid;
	bool m_bPierce;

	void ProcessDamage();

public:
	virtual int Initialize( int nSkinIndex, PROJECTILE_PARAM *pParam );
	virtual void Process();
};