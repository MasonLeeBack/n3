#include "fcheroobject.h"
#pragma once

class CKlarrann : public CFcHeroObject
{
public:
	CKlarrann( CCrossVector *pCross );
	virtual ~CKlarrann(void);
	
	bool			GetOrbAttack()								{ return m_bOrbAttack;	}
	void			SetOrbAttack( bool p_bOrbAttack )			{ m_bOrbAttack = p_bOrbAttack; }
	
	int				m_nFxTemplateIndex;									// 오브어텍 상태일때, 나타날 fx index
	int				m_nFxObjIndex;										// 위의 fx 에 대한 Object Index

	int				m_nHammerFxTemplateIndex;							// 망치
	int				m_nHammerFxObjIndex;								// 망치 Object Index
	int				m_nAxeFxTemplateIndex;								// 도끼
	int				m_nAxeFxObjIndex;									// 도끼 Object Index
	int				m_nScytheFxTemplateIndex;							// 낫
	int				m_nScytheFxObjIndex;								// 낫 Object Index

protected:
	bool			m_bOrbAttack;								// 현재 상태가 오브어택 상태인지를 나타내는 변수
	bool			m_bOnceLink;								// 한번만 링크하기 위해서 생성해둔 변수.
	bool			m_bHammer;									// 망치
	bool			m_bAxe;										// 도끼
	bool			m_bScythe;									// 낫
public:
	bool			GetBHammer()	{ return m_bHammer;	};
	bool			GetBAxe()		{ return m_bAxe;	};
	bool			GetBScythe()	{ return m_bScythe;	};

	void			SetBHammer( bool p_b )	{ m_bHammer = p_b;	};
	void			SetBAxe( bool p_b )		{ m_bAxe = p_b;	};
	void			SetBScythe( bool p_b )	{ m_bScythe = p_b;	};

	bool			Catch( GameObjHandle Handle );
	bool			Render();
	void			Process();
	void			SendCatchMsg();
	void			SendUnlinkCatchMsg( int nHandIndex = -1 );
	void			Initialize( INITGAMEOBJECTDATA* pData, int nForce, int nTeam, TroopObjHandle hTroop, float fScaleMin, float fScaleMax );
	int				FindExactAni( int nAniIndex );
	void			SignalCheck( ASSignalData *pSignal );

	virtual bool	OrbAttackFxOn();
	virtual bool	OrbAttackFxOff();

	void InitOrbAttack2DFx();
	void LoadWeaponTrailTexture();
};
