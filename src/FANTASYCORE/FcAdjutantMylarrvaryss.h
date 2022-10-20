#pragma once
#include "FcAdjutantObject.h"

class CFcAdjutantMylarrvaryss : public CFcAdjutantObject
{
public:
	CFcAdjutantMylarrvaryss(CCrossVector *pCross);
	virtual ~CFcAdjutantMylarrvaryss();

protected:
	void ProjectileFireball( ASSignalData *pSignal );
	void ProjectileAirBomb( ASSignalData *pSignal );

public:
	// ºÎ¸ð Virtual Function
	virtual void SignalCheck( ASSignalData *pSignal );
};