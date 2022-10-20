#include "StdAfx.h"
#include ".\fceffectlight.h"

CFcEffectLight::CFcEffectLight( CCrossVector *pCross )
	: CFcBaseObject( pCross )
{
	m_nLightIndex = -1;
	m_nCurFrame = 0;
	m_fProgress = 0.0f;
}

CFcEffectLight::~CFcEffectLight(void)
{
	if( m_nLightIndex != -1 )
	{
		g_BsKernel.DeleteObject( m_nLightIndex );
	}
}

int CFcEffectLight::Initialize( D3DCOLORVALUE Color, float fRange, int nLifeTime )
{
	m_nLightIndex = g_BsKernel.CreateLightObject( POINT_LIGHT );
	if( m_nLightIndex != -1 )
	{
		m_fStep = 1.0f / nLifeTime;
		m_TargetColor = Color;
		g_BsKernel.SendMessage( m_nLightIndex, BS_SET_POINTLIGHT_RANGE, ( DWORD )&fRange );
	}

	return m_nLightIndex;
}

void CFcEffectLight::Process()
{
	if( m_nLightIndex != -1 )
	{
		m_fProgress += m_fStep;
		if( m_fProgress > 0.0f )
		{
			if( m_fProgress >= 1.0f )
			{
				m_fProgress = 1.0f;
				m_fStep = -m_fStep;
			}

			return;
		}
	}

	Delete();
}

bool CFcEffectLight::Render()
{
	if( m_nLightIndex != -1 )
	{
/*		D3DCOLORVALUE Color;

		Color.r = m_TargetColor.r * m_fProgress;
		Color.g = m_TargetColor.g * m_fProgress;
		Color.b = m_TargetColor.b * m_fProgress;
		Color.a = 1.0f;*/
//		g_BsKernel.SendMessage( m_nLightIndex, BS_SET_POINTLIGHT_COLOR, ( DWORD )&Color );
		g_BsKernel.SendMessage( m_nLightIndex, BS_SET_POINTLIGHT_COLOR, ( DWORD )&m_TargetColor );
		g_BsKernel.UpdateObject( m_nLightIndex, m_Cross );
	}

	return true;
}

