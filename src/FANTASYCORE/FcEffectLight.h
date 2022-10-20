#pragma once

#include "fcbaseobject.h"

class CFcEffectLight :	public CFcBaseObject
{
public:
	CFcEffectLight( CCrossVector *pCross );
	virtual ~CFcEffectLight(void);

protected:
	int m_nLightIndex;
	float m_fStep;
	float m_fProgress;
	int m_nCurFrame;
	D3DCOLORVALUE m_TargetColor;

public:
	int Initialize( D3DCOLORVALUE Color, float fRange, int nLifeTime );
	void Process();
	bool Render();
};

typedef CSmartPtr< CFcEffectLight > EffectLightHandle;
