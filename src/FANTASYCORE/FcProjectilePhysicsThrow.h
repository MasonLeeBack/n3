#pragma once
#include "FcProjectile.h"
#include "ASData.h"

class CFcProjectilePhysicsThrow : public CFcProjectile
{
public:
	CFcProjectilePhysicsThrow( CCrossVector *pCross );
	virtual ~CFcProjectilePhysicsThrow();

	static ASSignalData *s_pHitSignal;
protected:

	static int s_nDeleteTick;
	static int s_nAlphaTick;

	int m_nDestroyTick;
	IBsPhysicsActor	*m_pActor;
	float m_fRadius;
	D3DXVECTOR3 m_vPrevPos;
	bool m_bApplyDamage;
	int m_nParentTeam;

	D3DXMATRIX m_matLocal;

	void ProcessDamage();
public:
	virtual int Initialize( int nSkinIndex, PROJECTILE_PARAM *pParam );
	virtual void Process();

};