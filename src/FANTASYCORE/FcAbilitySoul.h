
#pragma once

#include "FcAbilityBase.h"

class CFcGameObject;
typedef CSmartPtr<CFcGameObject> GameObjHandle;


enum SOULSTATE 
{
	OBSOUL_READY = 0, 
	OBSOUL_BORN, 
	OBSOUL_TRACE, 
	OBSOUL_CATCH,
	OBSOUL_RETURN,
};

class CFcSoulTrail
{
protected:
	D3DXVECTOR3 m_vDir;
	D3DXVECTOR3 m_vPos;


	D3DXVECTOR3 m_vTarget;
	float		m_fSpeed;
	int			m_nFXLineIdx;

	int			m_nTargetIdx;


	//	BOOL	  m_bCatch;
	SOULSTATE m_state;



	float		m_fRand[6];

public:
	CFcSoulTrail();
	virtual ~CFcSoulTrail();

	void Initialize( D3DXVECTOR3* pStartPos );
	void UpdateTarget(D3DXVECTOR3* pPos);
	void Process();


	void SetState(SOULSTATE state) { m_state = state; };

	void SetSpeed(float fSpeed) { m_fSpeed = fSpeed;};
	BOOL IsCatch() {return (m_state == OBSOUL_CATCH);};
	SOULSTATE GetState() { return m_state; };
	void SetDirection(D3DXVECTOR3* pDir ) { m_vDir = *pDir;};


	D3DXVECTOR3* GetPos()
	{
		return &m_vPos;
	}

	void	SetTargetIdx(int nIdx)	{ m_nTargetIdx = nIdx; };
	int		GetTargetIdx()			{ return m_nTargetIdx;}
};


class CFcGhostTrail : public CFcSoulTrail
{
public:
	void Initialize( D3DXVECTOR3* pStartPos );
};

class CAbSoulSceneIntensityTable : public TimeValueTable
{
public:
	CAbSoulSceneIntensityTable(float fStartValue);
};


class CAbSoulGlowIntensityTable : public TimeValueTable
{
public:
	CAbSoulGlowIntensityTable(float fStartValue);
};

class CAbSoulHighlightTable : public TimeValueTable
{
public:
	CAbSoulHighlightTable(float fStartValue);
};



class CFcAbilitySoul : public CFcAbilityBase
{
	CFcSoulTrail* m_SoulTrails;
	CFcGhostTrail* m_GhostTrails;
	DWORD		  m_dwCounter;

	D3DXVECTOR3  m_vReturnPoint;;

	int			m_nTarget;
	std::vector<GameObjHandle> m_Targets;


	int			m_nSoulTrail;
	int			m_nInitedTrail;
	
	int			m_nFxboardID;
	int			m_nFxFilterID;

	CAbSoulSceneIntensityTable	*m_pSceneTable;
	CAbSoulGlowIntensityTable	*m_pGlowTable;
	CAbSoulHighlightTable		*m_pHlightTable;

	float			m_fOrgSceneIntensity;
	float			m_fOrgBlurIntensity;
	float			m_fOrgGlowIntensity;
	float			m_fOrgHighlightTredhold;

	D3DXVECTOR3	m_vPos;

#ifdef _DEBUG
	int			m_nKill;
#endif


public:
	CFcAbilitySoul( GameObjHandle hParent );
	virtual ~CFcAbilitySoul();

	virtual void Initialize(DWORD dwParam1 = 0, DWORD dwParam2 = 0);

	virtual void Finalize();

	virtual void Process();

	void ReadSOX() {};

	D3DXVECTOR3* GetPosition() 
	{ 
		if(m_nInitedTrail == 0 || m_SoulTrails == NULL)
			return NULL;
		else
			return m_SoulTrails[0].GetPos() ;
	};
};
