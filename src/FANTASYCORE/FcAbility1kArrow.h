#pragma once


#include "FcAbilityBase.h"
#include "CrossVector.h"

#define _MAX_ARROW_COUNT 100

class CFcAbility1KArrow : public CFcAbilityBase
{
	struct _ARROW
	{
		CCrossVector m_Cross;
		D3DXVECTOR3 m_vVelocity;
		int m_nEngineIdx;
		int m_nFxIdx;

		_ARROW()
		{
			m_nEngineIdx = -1;
			m_nFxIdx = -1;
		}
	};

	D3DXVECTOR3 m_vPos;
	float		m_fSpd;
	_ARROW		m_Arrows[_MAX_ARROW_COUNT];
	std::vector< GameObjHandle > m_Enemy;


public :

	CFcAbility1KArrow( GameObjHandle hParent ) ;
	virtual ~CFcAbility1KArrow() ;

	virtual void Initialize(DWORD dwParam1 = 0, DWORD dwParam2 = 0) ;
	virtual void Finalize() ;
	virtual void Process() ;
	virtual void Update() ;

	virtual D3DXVECTOR3* GetPosition() { return &m_vPos; };
};