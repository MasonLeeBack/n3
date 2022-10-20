#include "fcheroobject.h"
#pragma once

#define THROW_MODE_NONE		0
#define THROW_MODE_START	1
#define THROW_MODE_RETURN	2

class CMyifee : public CFcHeroObject
{
public:
	CMyifee( CCrossVector *pCross );
	virtual ~CMyifee(void);

protected:
	int m_nLineHandle;
	std::vector< D3DXVECTOR3 > m_LinePosBuf;

	float m_fMaxThrowLength;
	float m_fThrowLength;
	bool m_bThrowMode;
	D3DXMATRIX m_WeaponMat;
	D3DXMATRIX m_HandMat;
	D3DXVECTOR3 m_CenterPos;
	D3DXVECTOR3 m_NextWeaponPos;
	float m_fThrowSpeed;

	ASSignalData *m_pThrowTrailSignal;

public:
	void Initialize( INITGAMEOBJECTDATA* pData, int nForce, int nTeam, TroopObjHandle hTroop, float fScaleMin=1.0f, float fScaleMax=1.0f );
	void ProcessThrowWeapon();
	void Process();

	void SignalCheck( ASSignalData *pSignal );
	void GetNextFrameCenterPosition( D3DXVECTOR3 *pPosition, int nCurFrame, ASSignalData *pSignal, int nThrowBending );
	void ThrowWeapon( ASSignalData *pSignal );
	void ReturnWeapon( ASSignalData *pSignal );
	void ThrowWeapon( bool bThrow );
	bool Render();
	void SendCatchMsg();
	void SendUnlinkCatchMsg( int nHandIndex = -1 );
	bool Catch( GameObjHandle Handle );

	void AddThrowTrail( int nTrailFrame, ASSignalData *pSignal, int nTrailHandle = 0 );
	void MyifeeTrailCheck( ASSignalData *pSignal );
	void HitCheckTempMyifee( ASSignalData *pSignal );
	void HitCheckTempMyifeeProp( ASSignalData *pSignal );
	void CancelThrow();
	void CmdHit( HIT_PARAM *pHitParam, D3DXVECTOR3 *pHitDir = NULL );

	void LoadWeaponTrailTexture();

	void InitOrbAttack2DFx();
	bool ChangeWeapon( int nSkinIndex, int nAttackPower );
	void InitWeapon();
	void StartOrbSpark();
	void StopOrbSpark();
};
