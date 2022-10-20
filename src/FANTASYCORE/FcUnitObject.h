#pragma once

//#ifndef _FCUNITOBJECT_H_
//#define _FCUNITOBJECT_H_

#include "FcCommon.h"
#include "FcGameObject.h"

class CCrossVector;
class CShortPathFinder;


class CFcUnitObject : public CFcGameObject
{
public:
	CFcUnitObject(CCrossVector *pCross);
	virtual ~CFcUnitObject();

protected:
	bool m_bUsePathFind;
	CShortPathFinder *m_pSPathFind;
	bool m_bCheckMoveLock;
	bool m_bRandomStand;

	D3DXVECTOR2 m_vMoveLockPos;
	int m_nMoveLockLength;
	GameObjHandle m_hMoveTargetHandle;
	float m_fMoveTargetLength;
//	bool m_bChangeMoveAni;
	int m_nChangeMoveAniIndex;

public:

	// ºÎ¸ð Virtual Function
	virtual void Process();
	virtual void PostProcess();
	virtual void DebugRender();

	virtual bool IsFlockable( GameObjHandle Handle );

	virtual void CmdMove( int nX, int nY, float fSpeed = -1.f, int nAniType = ANI_TYPE_RUN, int nAniIndex = 0 );
	virtual void CmdMove( GameObjHandle &Handle, float fLength = 0.f, float fSpeed = -1.f, int nAniType = ANI_TYPE_RUN, int nAniIndex = 0 );
	virtual void CmdStop( int nAniType = ANI_TYPE_STAND, int nAniIndex = 0 );
//	virtual void CmdAttack();
	virtual void CmdLookUp( GameObjHandle Handle );
	virtual void CmdVictory( float fX, float fY );
	virtual int FindExactAni( int nAniIndex );
	virtual void ResetMoveVector();

	void EnablePathFind( bool bEnable ) { m_bUsePathFind = bEnable; }
	void SetRandomStand( bool bEnable ) { m_bRandomStand = bEnable; }

	void LockShortPathFinder( D3DXVECTOR3 &vDir, int nTick );

protected:
	virtual void CheckMoveLock( TroopObjHandle hTroop );	
	void ProcessMove();

	//for orb spark movement
};


typedef CSmartPtr<CFcUnitObject> UnitObjHandle;


