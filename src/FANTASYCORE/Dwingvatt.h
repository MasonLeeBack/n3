
#include "FcBaseObject.h"
#include "FcGameObject.h"
#include "FcHeroObject.h"

#pragma once

#define TRAMPLE_STEP			18.0F

class CDwingvatt : public CFcHeroObject
{
public:
	CDwingvatt( CCrossVector *pCross );
	virtual ~CDwingvatt(void);

protected:
	GameObjHandle m_FindHandle;
	D3DXVECTOR3 m_CustomMoveStep;

	int m_nIllusionEidx[5];
	int m_nIllusion;

	CCrossVector m_crossIllusion[5];
	int m_nIdxcrossIllusion;


public:
	void Initialize( INITGAMEOBJECTDATA* pData, int nForce, int nTeam, TroopObjHandle hTroop, float fScaleMin, float fScaleMax );
	void FindTramplableObject( D3DXVECTOR3 *pPosition, GameObjHandle &Handle, float fCheckRadius, float fSearchMin, float fSearchMax );
	void TrampleCheck();
	void Process();
	void DwingvattTrailCheck( ASSignalData *pSignal );
	void SignalCheck( ASSignalData *pSignal );
	int FindExactAni( int nAniIndex );

	void LoadWeaponTrailTexture();
	void InitOrbAttack2DFx();
	void DashTrailCheck( ASSignalData *pSignal );
	bool ChangeWeapon( int nSkinIndex, int nAttackPower );
};
