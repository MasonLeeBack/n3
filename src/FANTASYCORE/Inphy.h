#include "FcBaseObject.h"
#include "FcGameObject.h"
#include "FcHeroObject.h"

#pragma once

#define TRAMPLE_STEP			18.0F
#define LOCKON_ATTACK_START		78
#define LOCKON_ATTACK_FALL		79

#define INPHY_SATTACK_Y			104
#define INPHY_SATTACK_TRASH		106

class CInphy : public CFcHeroObject
{
public:
	CInphy( CCrossVector *pCross );
	virtual ~CInphy(void);

protected:
	D3DXVECTOR3 m_CustomMoveStep;
	GameObjHandle m_FindHandle;
	GameObjHandle m_LockHandle;
	D3DXVECTOR3 m_LockTargetPos;
	int m_nLockonMoveFrame;
	bool m_bInitFound;
	std::vector< GameObjHandle > m_FindResult;
	bool m_bFreezCamera;

	float m_fRagdollDistY;

public:
	void TrampleCheck();
	void LockAttack();
	void CheckSAttack();
	void Process();
	void FindTramplableObject( D3DXVECTOR3 *pPosition, GameObjHandle &Handle, float fCheckRadius, float fSearchMin, float fSearchMax );

	void LoadWeaponTrailTexture();

	bool Catch( GameObjHandle Handle );
	void SendCatchMsg();
	void SendUnlinkCatchMsg( int nHandIndex = -1 );

	virtual bool Render();

	void InitOrbAttack2DFx();
	void SignalCheck( ASSignalData *pSignal );
};
