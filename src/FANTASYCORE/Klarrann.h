#include "fcheroobject.h"
#pragma once

class CKlarrann : public CFcHeroObject
{
public:
	CKlarrann( CCrossVector *pCross );
	virtual ~CKlarrann(void);
	
	bool			GetOrbAttack()								{ return m_bOrbAttack;	}
	void			SetOrbAttack( bool p_bOrbAttack )			{ m_bOrbAttack = p_bOrbAttack; }
	
	int				m_nFxTemplateIndex;									// ������� �����϶�, ��Ÿ�� fx index
	int				m_nFxObjIndex;										// ���� fx �� ���� Object Index

	int				m_nHammerFxTemplateIndex;							// ��ġ
	int				m_nHammerFxObjIndex;								// ��ġ Object Index
	int				m_nAxeFxTemplateIndex;								// ����
	int				m_nAxeFxObjIndex;									// ���� Object Index
	int				m_nScytheFxTemplateIndex;							// ��
	int				m_nScytheFxObjIndex;								// �� Object Index

protected:
	bool			m_bOrbAttack;								// ���� ���°� ������� ���������� ��Ÿ���� ����
	bool			m_bOnceLink;								// �ѹ��� ��ũ�ϱ� ���ؼ� �����ص� ����.
	bool			m_bHammer;									// ��ġ
	bool			m_bAxe;										// ����
	bool			m_bScythe;									// ��
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
