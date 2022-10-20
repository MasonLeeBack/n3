#pragma once


#include "FcAbilityBase.h"

class CFcGameObject;
typedef CSmartPtr<CFcGameObject> GameObjHandle;

class CFcAbilityWind : public CFcAbilityBase
{
	int			m_nFXId[2];

	int			m_nDustEffect;
	int			m_nDustEffect2;

	int			m_nDustTexId;
	int			m_nDustTexId2;
	float		m_fFarfromCaster;

#define	TRAILS			30
#define	TRAILPOINTS		10

	float		m_fDir;
	float		m_fTrailLen;

	bool		m_TrailInit;
	typedef struct
	{
		int nHandle;
		int nSeed[TRAILPOINTS];
		D3DXVECTOR3	P;
		D3DXVECTOR3 Pos[TRAILPOINTS];
		D3DXVECTOR3	Vel[TRAILPOINTS];
	} _TRAIL;
	
	_TRAIL m_Trail[TRAILS];

	int			m_dwTick;
	int			m_dwStartTick;
	int			m_dwEndTick;

	CCrossVector	m_Cross[2];

	int				m_nMotionBlurIndex;
	float			m_fBlurAlpha;

#define	BILLBOARDS		80
#define	BILLBOARDS2		80

	int				m_nBBID;
	int				m_nBBID2;

	
	typedef struct
	{
		D3DXVECTOR3 Pos;
		float		fSpeed;
	} _BILLBOARD;

	_BILLBOARD	m_BB[BILLBOARDS];
	_BILLBOARD	m_BB2[BILLBOARDS2];

	void	StartEff();
	void	StartEff2();

public :
	CFcAbilityWind( GameObjHandle hParent );
	virtual ~CFcAbilityWind();

	virtual void Initialize(DWORD dwParam1 = 0, DWORD dwParam2 = 0);
	virtual void Finalize();
	virtual void Process();

	void ReadSOX();

	D3DXVECTOR3* GetPosition() { return& m_Cross[0].m_PosVector;};
};