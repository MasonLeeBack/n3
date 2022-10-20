#pragma once
#include "FcUnitObject.h"

class CFcAdjutantObject : public CFcUnitObject
{
public:
	CFcAdjutantObject(CCrossVector *pCross);
	virtual ~CFcAdjutantObject();

protected:

	int m_nDieType;	// 0 : 걍 뒤진다. 1 : 쓰러진다.. ADJUTANT_RENEW_FRAME 후에 다시 일어나 싸운다. 2 : 쓰러져 있는다..
	int m_nDieRenewFrame;
	float m_fOnlyPlayerDamage;
	GameObj_ClassID m_AdjutantClassID;

public:
	GameObj_ClassID GetAdjutantClassID() { return m_AdjutantClassID; }
	void SetAdjutantClassID( GameObj_ClassID ClassID ) { m_AdjutantClassID = ClassID; }

	static GameObjHandle CreateAdjutantObject( CCrossVector *pCross, int nParam = -1 );
	// 부모 Virtual Function
	virtual void Initialize( INITGAMEOBJECTDATA* pData, int nTeam, int nGroup, TroopObjHandle hTroop, float fScaleMin=1.f, float fScaleMax=1.f );
	virtual void Process();
	virtual bool Render();

	virtual void SetDie( int nWeaponDropPercent = 100 );
	virtual bool CheckDefense();
	virtual bool IsFlockable( GameObjHandle Handle );
	virtual void ProcessFlocking();
	virtual bool CalculateDamage(int nAttackPower);

	virtual bool IsDie( bool bCheckEnable = false );

	void SetDieType( int nType ) { m_nDieType = nType; }
	int GetDieType() { return m_nDieType; }
	void SetOnlyPlayerDamageHPPercent( float fPercent ) { m_fOnlyPlayerDamage = fPercent; }
};