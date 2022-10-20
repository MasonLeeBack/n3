#pragma once

#include "FcAbilityBase.h"

class CFcGameObject;
typedef CSmartPtr<CFcGameObject> GameObjHandle;


#define ABIL_REDLIGHT_RADIUS_COUNT		100
#define	MAX_LIGHTNING					25

struct AbilRedLightColInfo{
	float fRadius;
	int   nFrame;
};
/*
struct RangeSearchingInfo{
	float fRadius;
	D3DXVECTOR3 Vel;
};
*/

struct RedLightRangeSeach{
	DWORD nFrame;
	std::vector<RangeSearchingInfo>vtList;
};


class CFcAbilityRedLight : public CFcAbilityBase
{
public:
	CFcAbilityRedLight( GameObjHandle hParent );
	virtual ~CFcAbilityRedLight();

	virtual void Initialize(DWORD dwParam1 = 0, DWORD dwParam2 = 0);
	void Finalize();
	void Process();
	void Update();

	D3DXVECTOR3* GetPosition() { return &(m_CrossOrg.m_PosVector); };

protected:
	AbilRedLightColInfo	m_fRadius[ABIL_REDLIGHT_RADIUS_COUNT];
	std::vector<RedLightRangeSeach>m_vtRangeSeach;

	DWORD			m_nCurFrame;
	int				m_nCurCmdCount;
	D3DXVECTOR3		m_Pos;
	float			m_fMaxRadius;
	DWORD			m_nMaxFrame;

	int				m_nFXID;
	int				m_nFXID2;
	int				m_nFXID3;
	
	int				m_nCur;
	CCrossVector	m_CrossOrg;
	DWORD			m_nFinalObjFrame;
	int				m_nTempSky;
	int				m_nFinalObj;
	int				m_nObjID[MAX_LIGHTNING];
	CCrossVector	m_Cross[MAX_LIGHTNING];
	bool			m_bThrowPath;
	int				m_nLomoFXID;
	//	int				m_nCurLomoStep;

	float			m_fCurSI;
	float			m_fCurBI;
	float			m_fCurGI;
	float			m_fCurHT;

	float			m_fOrgSceneIntensity;
	float			m_fOrgBlurIntensity;
	float			m_fOrgGlowIntensity;
	float			m_fOrgHighlightTredhold;

	float			m_fStep;
	int				m_nMotionBlurIndex;

	float			m_fBlurAlpha;
};

