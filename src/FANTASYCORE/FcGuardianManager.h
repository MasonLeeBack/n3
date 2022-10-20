#pragma once

#include "SmartPtr.h"

class CFcTroopObject;
class CFcTroopManager;
typedef CSmartPtr<CFcTroopObject> TroopObjHandle;

enum GUARDIAN_CMDTYPE
{
	GUARDIAN_CMDTYPE_ATTACK,
	GUARDIAN_CMDTYPE_DEFENSE
};

class CFcGuardianManager
{
public:
	CFcGuardianManager( CFcTroopManager* pManager );
	~CFcGuardianManager();

	void Process();

	bool IsGuardian( int nIndex ) { BsAssert( nIndex < 2 && nIndex >=0 ); if( m_hTroop[nIndex] ) { return true; } return false; }
	void AddGuardian( TroopObjHandle hTroop );
	bool AddGuardian( int nIndex );
	void AddGuardian();
	bool RemoveGuardian( int nIndex );
	int GetGuardianNum();
	bool IsGuardian( TroopObjHandle hTroop );
	void CmdAttack();
	void CmdDefense();
	TroopObjHandle GetGuardianTroop( int nIndex )	{ BsAssert( nIndex < 2 && nIndex >=0 ); return m_hTroop[nIndex]; }
	void SetGuardianEnable( int nIndex, bool bEnable );

	void SetGuardianPropertyTable();

	TroopObjHandle GetGuardianTarget() { return m_GuardianTarget; }
	TroopObjHandle GetLockOnTarget() { return m_LockOnTarget; }

	void EliminateTroop( TroopObjHandle hTroop );

	void SetGuardianTroopDefenseAdd( int nRate );
	void SetGuardianTroopAttackAdd( int nRate );
	void SetGuardianTroopMaxHPAdd( int nRate );
	void SetGuardianTroopMoveSpeedAdd( int nRate );
	void SetGuardianTroopTrapDamageAdd(  int nRate );
	void AddGuardianTroopHP( int nRate ,bool bPercent = true);

protected:
	void AttackProcess();
	void DefenseProcess();
	void FollowProcess();

	bool ProcessAttackTarget( TroopObjHandle hTroop );

	void CheckGuardianTarget();
	void CheckLockOnTarget();

	bool IsDoingCmdAttack( TroopObjHandle hTroop, TroopObjHandle hTarget );
	bool IsDoingCmdDefense( TroopObjHandle hTroop );

protected:
	TroopObjHandle m_hTroop[2];
	TroopObjHandle m_GuardianTarget;			// ��ó�� ȣ�������� ������ �� �ִ� �δ�
	TroopObjHandle m_LockOnTarget;				// ���µ� �δ�

	GUARDIAN_CMDTYPE m_CmdType;
	CFcTroopManager* m_pTroopManager;

	TroopObjHandle m_hCmdAttackTarget[2];		// ���� ����� �������� �����ϰ� �ִ� �δ�

	bool m_bCmdDefenseMove[2];					// ���潺 ����� �������� ������ �޷����� ���ΰ�
	int m_nDefenseAddRate[2];
	int m_nAttackAddRate[2];
	bool m_bUpdateProperty[2];
	bool m_bEnable[2];

	int m_nDefenseAdd;
	int m_nAttackAdd;
  	int m_nAddMoveSpeedRate;
	int m_nAddDamageTrap;
};