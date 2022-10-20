#pragma once
#include "FcProjectile.h"

class CFcProjectileStone : public CFcProjectile
{
public:
	CFcProjectileStone( CCrossVector *pCross );
	virtual ~CFcProjectileStone();

	static ASSignalData *s_pHitSignal;
protected:

	int m_nDestroyTick;
	bool m_bCheckGround;
	int m_nParentTeam;
	int m_nFXIndex;
	int m_nDestroyFXIndex;

	D3DXVECTOR3 m_Velocity;
	D3DXVECTOR3 m_Accel;
	D3DXVECTOR3 m_PrevPos;
	D3DXVECTOR3 m_WallVector;
	CCrossVector m_DestroyCross;

	bool CheckHitTarget();

public:
	virtual int Initialize( int nSkinIndex, PROJECTILE_PARAM *pParam );
	virtual void Process();
	virtual bool Render();

};