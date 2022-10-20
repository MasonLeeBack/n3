#pragma		once

#include "SmartPtr.h"

class CFcGameObject;
typedef CSmartPtr<CFcGameObject> GameObjHandle;
class CFcAbilityBase;

enum FC_ABILITY
{
	FC_ABILITY_WATER = 0,
	FC_ABILITY_FIRE,
	FC_ABILITY_EARTH,
	FC_ABILITY_SOUL,
	FC_ABILITY_WIND,
	FC_ABILITY_LIGHT_1,
	FC_ABILITY_LIGHT_2,

	FC_ABILITY_1KARROW,

	Fc_ABILITY_NUM
};


class CFcAbilityManager
{
public:
	CFcAbilityManager();
	~CFcAbilityManager();

	bool Initialize();
	void Process();
	void Update();

	int GetCount() { return m_vecObjs.size(); };
	int GetRealAbilityCount();

	int Start( FC_ABILITY Ability, GameObjHandle hParent, DWORD Param1 = 0, DWORD Param2 = 0, DWORD Param3 = 0, DWORD Param4 = 0 );
	void StopAll();

	D3DXVECTOR3* GetAbilityPosition(GameObjHandle hOwner);
	void SetOrbGen( int nOrbGenType, int nOrbGenPer );
	int GetOrbGenType(){ return m_nOrbGenType; }
	int GetOrbGenPer(){ return m_nOrbGenPer; }
	FC_ABILITY GetRealAbilityType() { return m_RealAbilityType; }

	int m_nNotAbilityCount;
	int m_nOrbGenType;
	int m_nOrbGenPer;
	FC_ABILITY m_RealAbilityType;
protected:
	void Clear();

protected:

	std::vector<CFcAbilityBase*> m_vecObjs;
};