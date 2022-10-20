#pragma once

#include "FcAbilityBase.h"

class CFcGameObject;
typedef CSmartPtr<CFcGameObject> GameObjHandle;


#define ABIL_EARTH_COLTEST_MAX	2



struct AbilEarthColInfo
{
	void SetData(DWORD argnFrame,
	float	argfRadius,
	int		argnPower,
	int		argnPushY,
	int		argnPushZ,
	float	argfPhysicsRadius,
	float	argfPhysicsPower)
	{
		nFrame = argnFrame;
		fRadius = argfRadius;
		nPower = argnPower;
		nPushY = argnPushY;
		nPushZ = argnPushZ;
		fPhysicsRadius = argfPhysicsRadius;
		fPhysicsPower = argfPhysicsPower;
	}

	DWORD		nFrame;
	float	fRadius;
	int		nPower;
	int		nPushY;
	int		nPushZ;
	float	fPhysicsRadius;
	float	fPhysicsPower;
};

class CAbEarthSceneIntensityTable : public TimeValueTable
{
public:
	CAbEarthSceneIntensityTable(float fStartValue);
};


class CAbEarthGlowIntensityTable : public TimeValueTable
{
public:
	CAbEarthGlowIntensityTable(float fStartValue);
};


class CAbEarthHighlightTable : public TimeValueTable
{
public:
	CAbEarthHighlightTable(float fStartValue);
};



class CFcAbilityEarth : public CFcAbilityBase
{
	int			m_nCastFXId;
	int			m_nFXEarthId;
	int			m_nFXId;
	int			m_nObjID;

	int			m_nFX2DEFFECT;
	int			m_nFX2DEFFECTExp;

	float		m_fDistFromCaster;

	D3DXVECTOR3 m_vPos;
	
	CCrossVector	m_Cross;

	float	m_fScale;

	DWORD		m_dwTick;
	DWORD		m_dwExpfxStartTick;
	int			m_nEarthStormTex;
	int			m_nEarthStormTexExp;

	BOOL		m_bStartExpfx;
	int			m_nMotionBlurIndex;
	int			m_nCastTime;

	std::vector< GameObjHandle > m_Enemy;
	std::vector<CFcProp *> m_vecProps;

	CAbEarthSceneIntensityTable		*m_pSceneTable;
	CAbEarthGlowIntensityTable		*m_pGlowTable;
	CAbEarthHighlightTable			*m_pHlightTable;

	float			m_fOrgSceneIntensity;
	float			m_fOrgGlowIntensity;
	float			m_fOrgHighlightTredhold;

protected:
	AbilEarthColInfo m_ColCheckInfo[ABIL_EARTH_COLTEST_MAX];
public :
	CFcAbilityEarth( GameObjHandle hParent );
	virtual ~CFcAbilityEarth();

	virtual void Initialize(DWORD dwParam1 = 0, DWORD dwParam2 = 0);
	virtual void Finalize();
	virtual void Process();
	void	Update();

	void ReadSOX();


	D3DXVECTOR3* GetPosition() { return& m_Cross.m_PosVector;};

	void DamageProcess(float fOutRadius, float fInnerRadius);
};

