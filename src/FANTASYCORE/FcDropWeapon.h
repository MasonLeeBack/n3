#include "fcbaseobject.h"
#pragma once

#define DROP_WEAPON_DELETE_FRAME	160
#define DROP_WEAPON_ALPHA_FRAME		100
#define DROP_MAX_WEAPON_COUNT_ATONCE 5
#define DROP_WEAPON_SHARE_FRAME 15

class CFcDropWeapon : public CFcBaseObject
{
public:
	CFcDropWeapon( CCrossVector *pCross );
	virtual ~CFcDropWeapon(void);

protected:
	IBsPhysicsActor	*m_pActor;
	int m_nDeleteFrame;

	std::vector	<CFcDropWeapon*> m_AgentList;
	D3DXMATRIX	m_DistanceMat;
	D3DXMATRIX	m_matLocal;

	CCrossVector m_CrossOrigin;

	static std::vector	<CFcDropWeapon*> m_AgentAllList;
	static DWORD m_sRenderTick;

public:
	int Initialize( int nObjectIndex, D3DXVECTOR3 *pForceVector );
	void Process();
	void AddAgentWeapon( CFcDropWeapon *pWeapon ) { m_AgentList.push_back( pWeapon ); }
	void SetAgentMatrix( D3DXMATRIX *AgentMat);
};

typedef CSmartPtr<CFcDropWeapon> DropWeaponHandle;
