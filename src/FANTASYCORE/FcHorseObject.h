#pragma once

#include "FCUnitObject.h"

class CFcVelocityCtrl;
class CFcHorseObject : public CFcUnitObject
{
public:
	CFcHorseObject( CCrossVector *pCross );
	virtual ~CFcHorseObject(void);

protected:
	int m_nLinkPlayerIndex;
	int m_nRunFrame;
	static int s_nRotationAngle;
	static int s_nMaxRollAngle;
	static int s_nRollValue;
	
	CFcVelocityCtrl *m_pVelocityCtrl;
	float m_fCurMaximumSpeed;
	int m_nRotationAniType;
	float m_fPrevRotationAngle;

	int m_nRollAngle;
	int m_nPrevRollAngle;
	bool m_bStopFlag;

public:
	virtual void Process();
	virtual bool Render();
	virtual bool LinkMe( GameObjHandle Handle, int nPlayerIndex = -1 );
	virtual bool UnLinkMe( GameObjHandle Handle );
	virtual void ProcessFlocking();

	virtual void Initialize( INITGAMEOBJECTDATA* pData, int nForce, int nTeam, TroopObjHandle hTroop, float fScaleMin=1.0f, float fScaleMax=1.0f );
	virtual void CmdAttack( GameObjHandle Handle );
	virtual void CmdMove( int nX, int nY, float fSpeed = -1.f, int nAniType = ANI_TYPE_RUN, int nAniIndex = 0 );
	virtual void CmdStop( int nAniType = ANI_TYPE_STAND, int nAniIndex = 0 );
	virtual void RotationProcess();
	virtual void MoveZ(float fDist);
	virtual void PositionProcess();
	virtual bool IsHittable( GameObjHandle Handle );
	virtual bool IsFlockable( GameObjHandle Handle ); 
	virtual void CheckHitLinkObject( GameObjHandle Handle, int &nAniAttr, int &nAniType, int &nAniTypeIndex );


	void ProcessKeyEvent(int nKeyCode, KEY_EVENT_PARAM *pKeyParam);

protected:
	virtual bool GetCollisionPropHeight( float fX, float fY, float* pHeight);
	virtual void CheckMoveLock( TroopObjHandle hTroop );		// 낑겼을 때 빠져나오게 하는 것

	void ProcessChildAnimation();
	void ProcessMove();
	void ProcessRoll();
	float GetStopableDistance();
};

typedef CSmartPtr<CFcHorseObject> HorseObjHandle;
