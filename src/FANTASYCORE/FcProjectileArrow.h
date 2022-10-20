#pragma once
#include "FcProjectile.h"
#include "ASData.h"

class CFcProjectileArrow : public CFcProjectile
{
public:
	CFcProjectileArrow( CCrossVector *pCross );
	virtual ~CFcProjectileArrow();

	static ASSignalData *s_pHitSignal;

protected:
	int m_nFXLineIndex;
	int m_nDestroyTick;
	int m_nTrailLifeTick;
  
	D3DXVECTOR3 m_Velocity;
	D3DXVECTOR3 m_Accel;
	D3DXVECTOR3 m_PrevPos;

	bool CheckHitTarget();

public:
	virtual int Initialize( int nSkinIndex, PROJECTILE_PARAM *pParam );
	virtual void Process();

};