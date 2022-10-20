#pragma once

#include "FcUnitObject.h"
#include "Box3.h"

class CFcVelocityCtrl;
class CFcMoveTowerObject : public CFcUnitObject
{
public:
	CFcMoveTowerObject( CCrossVector *pCross );
	virtual ~CFcMoveTowerObject();

protected:
	static float s_fRotationAngle;
	static char *s_szWheelBoneName[];

	Box3 m_Collision;
	CFcVelocityCtrl *m_pVelocityCtrl;
	CFcVelocityCtrl *m_pRotationCtrl[2];
	bool m_bWorkable;
	bool m_bStopFlag;
	float m_fCurMaximumSpeed;
	float m_fWheelAngle[12];
	int m_nRotationAniIndex;

	bool m_bOpen;

	void ProcessMove();
	float GetStopableDistance();
	float GetStopableAngle();
	void ProcessWheelSpeed();

	std::vector<TroopObjHandle> m_VecWallTroop;

	bool IsNeedNewWallTroop();
	void AddNewWallTroop();
	void ProcessFlush();

public:
	virtual void Process();
	virtual void PostProcess();
	virtual bool Render();
	virtual void ProcessFlocking();

	virtual void Initialize( INITGAMEOBJECTDATA* pData, int nForce, int nTeam, TroopObjHandle hTroop, float fScaleMin=1.0f, float fScaleMax=1.0f );
	virtual void CmdMove( int nX, int nY, float fSpeed = -1.f, int nAniType = ANI_TYPE_RUN, int nAniIndex = 0 );
	virtual void CmdStop( int nAniType = ANI_TYPE_STAND, int nAniIndex = 0 );
	virtual void RotationProcess();
	virtual void MoveZ(float fDist);
	virtual void PositionProcess();

	virtual bool IsHittable( GameObjHandle Handle );
	virtual bool IsFlockable( GameObjHandle Handle ); 

	virtual bool LinkMe( GameObjHandle Handle, int nPlayerIndex = -1 );
	virtual bool UnLinkMe( GameObjHandle Handle );

	virtual void CheckMoveLock( TroopObjHandle hTroop ) {}

	virtual void SignalCheck( ASSignalData *pSignal );
	virtual void DebugRender();
};