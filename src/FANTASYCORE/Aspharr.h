
#include "FcBaseObject.h"
#include "FcGameObject.h"
#include "FcHeroObject.h"

#pragma once

#define SATTACK_X		96	
#define SATTACK_X_SMALL	97	
#define SATTACK_X_LARGE	109	
#define SATTACK_Y		99
#define SATTACK_Y_SMALL	100
#define SATTACK_Y_LARGE	110

class CAspharr : public CFcHeroObject
{
public:
	CAspharr( CCrossVector *pCross );
	virtual ~CAspharr(void);

protected:
	int m_nSAttackFrame;

	float m_fRagdollDistY;

public:
	void Process();
	virtual bool Render();

	bool Catch( GameObjHandle Handle );
	void SendCatchMsg();
	void SendUnlinkCatchMsg( int nHandIndex = -1 );

	void LoadWeaponTrailTexture();	
};
