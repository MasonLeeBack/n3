#pragma		once

#include "SmartPtr.h"
#include "FcFxBase.h"

class CFcGameObject;
typedef CSmartPtr<CFcGameObject> GameObjHandle;

class CFcAbilityBase
{
public:
	CFcAbilityBase( GameObjHandle hParent );
	virtual ~CFcAbilityBase();

	virtual void Initialize(DWORD dwParam1 = 0, DWORD dwParam2 = 0)= 0;
	virtual void Initialize(bool p_b) {}
	virtual void Finalize() {}
	virtual void Process() {}
	virtual void Update() {}

	virtual D3DXVECTOR3* GetPosition() = 0;

	bool IsFinished()		{ return m_bFinished; }
	GameObjHandle GetParent() { return m_hParent; }
	void SetOrbGen( int nGenType, int nGetPer );

	int		m_nRtti;
	int		GetRtti()		{ return m_nRtti;	}
	void	SetRtti(int n)	{ m_nRtti = n;		}

	

protected:
	GameObjHandle m_hParent;
	bool m_bFinished;
	int  m_nOrbGenType;
	int  m_nOrbGenPer;
};



struct RangeSearchingInfo{
	float fRadius;
	D3DXVECTOR3 Vel;
};