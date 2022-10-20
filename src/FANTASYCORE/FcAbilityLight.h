#pragma once

#include "FcAbilityBase.h"

class CFcGameObject;
typedef CSmartPtr<CFcGameObject> GameObjHandle;


#define ABIL_LIGHT_RADIUS_COUNT		100

struct AbilLightColInfo{
    float fRadius;
	int   nFrame;
};
/*
struct RangeSearchingInfo{
	float fRadius;
	D3DXVECTOR3 Vel;
};
*/

struct LightRangeSeach{
	DWORD nFrame;
	std::vector<RangeSearchingInfo>vtList;
};




class CAbLightLomoTable : public TimeValueTable
{
public :
	CAbLightLomoTable(float fStartValue);
};


class CAbLightSceneIntensityTable : public TimeValueTable
{
public:
	CAbLightSceneIntensityTable(float fStartValue);
};


class CAbLightGlowIntensityTable : public TimeValueTable
{
public:
	CAbLightGlowIntensityTable(float fStartValue);
};


class CAbLightHighlightTable : public TimeValueTable
{
public:
	CAbLightHighlightTable(float fStartValue);
};


class CFcAbilityLight : public CFcAbilityBase
{
public:
	CFcAbilityLight( GameObjHandle hParent );
	virtual ~CFcAbilityLight();

	virtual void Initialize(DWORD dwParam1 = 0, DWORD dwParam2 = 0);
	void Finalize();
	void Process();
	void Update();

	D3DXVECTOR3* GetPosition() { return new D3DXVECTOR3(m_Cross.GetPosition().x, m_Cross.GetPosition().y, m_Cross.GetPosition().z); };


protected:
	AbilLightColInfo	m_fRadius[ABIL_LIGHT_RADIUS_COUNT];
	std::vector<LightRangeSeach>m_vtRangeSeach;

	DWORD			m_nCurFrame;
	int				m_nCurCmdCount;
	D3DXVECTOR3		m_Pos;
	float			m_fMaxRadius;
	DWORD			m_nMaxFrame;
	

	CCrossVector	m_Cross;
	int				m_nFXID;
	int				m_nObjID;
	bool			m_bThrowPath;
	int				m_nLomoFXID;
//	int				m_nCurLomoStep;


	CAbLightLomoTable				*m_pLomoTable;
	CAbLightSceneIntensityTable		*m_pSceneTable;
	CAbLightGlowIntensityTable		*m_pGlowTable;
	CAbLightHighlightTable			*m_pHlightTable;


	float			m_fOrgSceneIntensity;
	float			m_fOrgBlurIntensity;
	float			m_fOrgGlowIntensity;
	float			m_fOrgHighlightTredhold;
};