
#pragma once
#include "fcheroobject.h"

#define MIN_SHOT_ATTACK_LEVEL	1
#define MIN_FLYING_TYURRU_ANI	17

class CFcProjectile;
typedef CSmartPtr< CFcProjectile > ProjetileHandle;

enum WaterType
{
	BIG_COLUMNWATER = 0,
	SMALL_COLUMNWATER,

	COUNT_COLUMNWATER
};

class CLeviaElement
{
	CCrossVector 	m_Cross;
	float 		m_fScale;
	float 		m_fSize;
	bool		m_bIdle;


	//	float 		m_fRotateSpd;
	int			m_nRoateSpd;
	float		m_fScaleSpd;
	float		m_fScaleSin;

public :
	CLeviaElement()
	{
		m_fScale = 0.f;
		m_fSize = 0.f;
		m_bIdle = true;	

		m_nRoateSpd = 0;
		m_fScaleSpd = 0.f;
		m_fScaleSin = 0.f;
	}


	void	Initialize( D3DXVECTOR3& vPos, float fScaleSpd);
	void	Process();
	float	GetCurSize() { return (m_fSize * m_fScale);};
	BOOL	IsIdle() { return m_bIdle; };
	void	Update(int nEngineIdx);
};


#define _LEVIA_ELEMENTCOUNT 45


class CLevia
{
	GameObjHandle m_hMaster;
	CCrossVector 	m_Cross;
	CCrossVector 	m_CrossR;
	float		m_fScale;

	float		m_fRotateRadius;
	float		m_fSinFX;
	float		m_fSinFY;
	float		m_fSinFZ;

	float		m_fRotX;
	float		m_fRotY;
	float		m_fRotZ;

	float		m_fAccel;

	int			m_nLoadFxIdx;
	int			m_nFxIdx;
	int			m_nRyviusSkinIndex;
	int			m_nPointLight;


	CLeviaElement m_Element[_LEVIA_ELEMENTCOUNT];

	int	m_nEngineIdx;
	int	m_nElementEngineIdx[_LEVIA_ELEMENTCOUNT];

	bool	m_bLive;
	bool	m_bMasterLive;
	bool	m_bRelease;

public:
	CLevia();
	virtual ~CLevia() 
	{
		Release();
	}
	void SetMaster( GameObjHandle p_hMaster )	{ m_hMaster = p_hMaster;	};
	bool GetLive()				{ return m_bLive;	};
	void SetLive( bool p_b )	{ m_bLive = p_b;	};

	bool GetMasterLive()				{ return m_bMasterLive;	};
	void SetMasterLive( bool p_b )		{ m_bMasterLive = p_b;	};

	void Initialize(GameObjHandle hMaster);
	void Release();
	void Process();
	void Update();
	void Generate(float fLife, D3DXVECTOR3& vPos  );
	void EffectAndPointLightRemove();
};


struct sShotWaterDrop
{
	bool	_bShotWaterDrop;
	bool	_bNormal;
	int		_nTick;
};


/*-----------------------------------------------------------------------------------
-
-				CTyurru							Choi Jae Young
-												2005. 12. 5
-												v 0.01
-							In Blue-Side
-
-
-			note : 약간 수정. 별한거 없음
-				  
-
------------------------------------------------------------------------------------*/
class CTyurru  : public CFcHeroObject
{

public:

	bool				Render();
	void				Process();
	void				Initialize( INITGAMEOBJECTDATA* pData, int nForce, int nTeam, TroopObjHandle hTroop, float fScaleMin, float fScaleMax );
	virtual bool		OrbAttackFxOn();
	virtual bool		OrbAttackFxOff();
	virtual void		OrbAttack();
	virtual void		NormalAttack();
	virtual void		InitOrbAttack2DFx();
	virtual bool		IsFlockable( GameObjHandle Handle );
	virtual void		ProcessFlocking();
	void				SignalCheck( ASSignalData *pSignal );
	void				PhysicsCheck( ASSignalData *pSignal );
	void				ChangeSecondAniByIndex( int nAniIndex );
	int					CalculateWalkAni( KEY_EVENT_PARAM *pKeyParam );
	void				ProcessKeyEvent( int nKeyCode, KEY_EVENT_PARAM *pKeyParam );
	int					FindExactAni( int nAniIndex );
	void				CmdHit( HIT_PARAM *pHitParam, D3DXVECTOR3 *pHitDir = NULL );
	void				StartOrbSpark();
	void				Resume();
	void				Enable(bool bEnable, bool bFullHP = false);

private:

	bool				GetOrbAttack()								{ return m_bOrbAttack;	}
	void				SetOrbAttack( bool p_bOrbAttack )			{ m_bOrbAttack = p_bOrbAttack; }
	void				CreateColumnWater( WaterType  p_nWaterType, D3DXVECTOR3 p_vPos, int p_nDelayTick = 0 );
	void				ShotWater( int p_nCount, float p_fDist, float p_fDist2, D3DXVECTOR3* p_vPos = NULL );
	void				ShotWaterDrop(float p_fAngle, D3DXVECTOR3 p_vStartPos);
	void				PushWaterBomb();
	void				SetLeviaMaster(GameObjHandle p_hMaster) { m_Levia.SetMaster(p_hMaster); }
	void				RestoreLevia() { m_Levia.SetMaster(m_Handle); }

	bool				IsFloatLog();

protected:
	
	bool				m_bOrbAttack;
	int					m_nFxIndex;
	int					m_nNoramlAttackFx;
	int					m_nColumnWater;
	int					m_nWaterIndex;
	int					m_nColumnWaterID[COUNT_COLUMNWATER];
	int 				m_nSecondBoneIndex;
	int 				m_nSecondBoneIndex2;
	int 				m_nSecondAni;
	int					m_nWaterSphereSkin;
	int					m_nShotWaterDrop;
	float				m_fSecondFrame;
	bool				m_bUseSecondAni;
	CLevia				m_Levia;
	sShotWaterDrop		m_sShotWaterDrop;

	ProjetileHandle		m_ProjectileHandle;
	

	int					m_nTest;
	int					m_nTest2;



public:
	CTyurru(CCrossVector *pCross);
	~CTyurru(void);
};
