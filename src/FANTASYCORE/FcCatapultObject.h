#pragma once

#include "FcUnitObject.h"
#include "Box3.h"

class CFcVelocityCtrl;
class CFcCatapultObject : public CFcUnitObject
{
public:
	CFcCatapultObject( CCrossVector *pCross );
	virtual ~CFcCatapultObject();

protected:
	struct LinkBoneInfo {
		char *szBoneName;
		int nWalkAniIndex;
//		int nLeftRotateAniIndex;
//		int nRightRotateAniIndex;
	};
	static float s_fRotationAngle;
	static LinkBoneInfo s_LinkBoneName[];
	static char *s_szWheelBoneName[];

	Box3 m_Collision;
	CFcVelocityCtrl *m_pVelocityCtrl;
	CFcVelocityCtrl *m_pRotationCtrl[2];
	bool m_bWorkable;
	bool m_bStopFlag;
	int m_nRotationAniIndex;
	float m_fCurMaximumSpeed;
	float m_fWheelAngle[4];
	std::vector<int> m_nVecLinkBoneIndex;
	D3DXVECTOR3 m_vTargetPos;
	int m_nSoundMoveHandle;

	// Temp
	bool m_bDieFlag;
	int m_nDieParticleIndex[4];

	// breakable physics
	PHYSICS_DATA_CONTAINER *m_pBreakableInfo;
    int		m_nBreakableSkinIndex;
	int		m_nBreakObjectIndex;

	void ProcessMove();
	void ProcessChildAnimation();
	float GetStopableDistance();
	float GetStopableAngle();
	void ProcessWheelSpeed();
	void Break();	

public:

	virtual void CreateParts(INITGAMEOBJECTDATA *pData);
	virtual void Process();
	virtual void PostProcess();
	virtual bool Render();
	virtual void ProcessFlocking();

	virtual void Initialize( INITGAMEOBJECTDATA* pData, int nForce, int nTeam, TroopObjHandle hTroop, float fScaleMin=1.0f, float fScaleMax=1.0f );
	virtual void CmdAttack( GameObjHandle Handle );
	virtual void CmdMove( int nX, int nY, float fSpeed = -1.f, int nAniType = ANI_TYPE_RUN, int nAniIndex = 0 );
	virtual void CmdStop( int nAniType = ANI_TYPE_STAND, int nAniIndex = 0 );
	virtual void RotationProcess();
	virtual void MoveZ(float fDist);

	virtual bool IsHittable( GameObjHandle Handle );
	virtual bool IsFlockable( GameObjHandle Handle ); 

	virtual void CheckHitLinkObject( GameObjHandle Handle, int &nAniAttr, int &nAniType, int &nAniTypeIndex );

	virtual bool LinkMe( GameObjHandle Handle, int nPlayerIndex = -1 );
	virtual bool UnLinkMe( GameObjHandle Handle );

	virtual void CheckMoveLock( TroopObjHandle hTroop ) {}
	virtual void LookAtHitUnit( D3DXVECTOR3 *pDirection ) {}


	virtual void BeginStatus( CFcStatusObject::STATUS_TYPE Type, int nParam1 = 0, int nParam2 = 0, int nParam3 = 0, bool bOverlap = false ) {}
	virtual bool GetExactHitAni( int &nAniType, int &nAniIndex );

	virtual void CalculateParticlePosition( CCrossVector &Cross );


	virtual void SignalCheck( ASSignalData *pSignal );
	virtual void DebugRender();
	int GetLinkObjAniIndex( int nIndex );

};