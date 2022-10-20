#pragma once
#include "FcProjectile.h"
#include "ASData.h"
#include "FcGameObject.h"

class CFcProjectileBlackHall : public CFcProjectile
{
public:
	CFcProjectileBlackHall( CCrossVector *pCross );
	virtual ~CFcProjectileBlackHall();

	static ASSignalData *s_pHitSignal;

	void Finish() { m_bFinish = true; }

protected:
	int m_nState;
	bool m_bFinish;

	int m_nCreateTick;
	int m_nNextTick;
	int m_nFXIndex[3];
	int m_nParentTeam;
	std::vector<GameObjHandle> m_vecList;

public:
	virtual int Initialize( int nSkinIndex, PROJECTILE_PARAM *pParam );
	virtual void Process();
	virtual bool Render();

};