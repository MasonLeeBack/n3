#pragma once
#include "FcGameObject.h"

class CFcVelocityCtrl;
class CFcFlyUnitObject : public CFcGameObject
{
public:
	CFcFlyUnitObject( CCrossVector *pCross );
	virtual ~CFcFlyUnitObject();

protected:
	// ��������
	bool m_bFlying;
	// �ּ� ���� ��
	float m_fMinHeight;
	// ȸ����
	static int s_nRotateAngle;
	// ��ġ��
	static int s_nPitchAngle; 
	// ȸ���� ���� �ӵ�, �ִ� ���� ��
	static int s_nRollingAngle;
	int m_nMaxRollingAngle;
	bool m_bRotate;

	bool m_bAttackFlag;

	// Pitch
	int m_nPitchAngle;


	// ���ݽ� ��
	float m_fAttackHeight;

	// ����
	int m_nRollAngle;
	int m_nPrevRollAngle;

	float m_fCurHeight;
	float m_fHeight;
	float m_fFlyGravity;
	float m_fPrevHeight;
	bool m_bDirectDie;
	bool m_bFallDamage;


	CFcVelocityCtrl *m_pVelocityCtrl;
	CFcVelocityCtrl *m_pHeightCtrl;

public:
	// �θ� Virtual Funcition
	virtual void Process();
	virtual void PostProcess();
	virtual void SignalCheck( ASSignalData *pSignal );
	virtual bool Render();

	virtual void CmdMove( int nX, int nY, float fSpeed = -1.f, int nAniType = ANI_TYPE_RUN, int nAniIndex = 0 );
	virtual void CmdStop( int nAniType = ANI_TYPE_STAND, int nAniIndex = 0 );
	virtual void CmdLookUp( GameObjHandle Handle );
	virtual void CmdVictory( float fX, float fY );
	virtual void CmdPlace( float nX, float nY );

	virtual bool IsMove();
	virtual void MoveZ(float fDist);
	virtual void RotationProcess();
//	virtual void PositionProcess();
	virtual void ProcessFlocking();
	virtual bool IsFlockable( GameObjHandle Handle );

	virtual bool GetExactHitAni( int &nAniType, int &nAniIndex );
	virtual bool CheckHit();
	virtual bool IsHittable( GameObjHandle Handle );


	// �ú� ��������� ���� �ʿ�
	CFcVelocityCtrl *GetVelocityCtrl() { return m_pVelocityCtrl; }

protected:
	virtual void CheckMoveLock( TroopObjHandle hTroop );		// ������ �� ���������� �ϴ� ��
	void ProcessMove();
	void ProcessAttack();
	float GetStopableDistance();
};