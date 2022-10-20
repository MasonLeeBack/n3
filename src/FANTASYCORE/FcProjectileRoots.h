#pragma once

#include "FcProjectile.h"
#include "ASData.h"
class CFcVelocityCtrl;


class CFcProjectileRoots : public CFcProjectile
{
public:
	CFcProjectileRoots( CCrossVector *pCross );
	virtual ~CFcProjectileRoots();

	static ASSignalData *s_pHitSignal;
	static int s_nDistance;
	static int s_nGenerateTick;

	struct RootStruct {
		int nState;
		int nCreateTime;
		int nObjectIndex;
		CCrossVector Cross;
		RootStruct() {
			nState = 0;
			nObjectIndex = -1;
		}
	};
protected:
	int m_nCreateTick;
	int m_nDestroyTick;
	int m_nDamageTick;
	int m_nParentTeam;
	int m_nDamageCount;
	int m_nDamageDelay;

	int m_maxRoots;
	std::vector<RootStruct> m_VecRootList;
	

	void AddRoot( CCrossVector *pCross );
	void CalcTerrain( CCrossVector *pCross );
	void ProcessDamage(CCrossVector *pCross);

public:
	virtual int Initialize( int nSkinIndex, int nRoots, PROJECTILE_PARAM *pParam );
	virtual void Process();
	virtual bool Render();
};