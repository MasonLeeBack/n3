#include "StdAfx.h"
#include "FcVelocityCtrl.h"
#include "BsKernel.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


CFcVelocityCtrl::CFcVelocityCtrl()
{
	m_fStartSpeed = 1.f;
	m_fMaximumSpeed = 1.f;
	m_fMinimumSpeed = 0.f;
	m_fAcceleration = 1.1f;
	m_fDeceleration = 0.9f;

	m_fCurSpeed = 0.f;
}

CFcVelocityCtrl::CFcVelocityCtrl( float fStartSpeed, float fMaximumSpeed, float fMinimumSpeed, float fAcceleration, float fDeceleration )
{
	m_fStartSpeed = fStartSpeed;
	m_fMaximumSpeed = fMaximumSpeed;
	m_fMinimumSpeed = fMinimumSpeed;
	m_fAcceleration = fAcceleration;
	m_fDeceleration = fDeceleration;

	m_fCurSpeed = 0.f;
}

CFcVelocityCtrl::~CFcVelocityCtrl()
{
}

void CFcVelocityCtrl::Reset()
{
	m_fCurSpeed = m_fStartSpeed;
}

float CFcVelocityCtrl::GetSpeed( VELOCITY_TYPE Type )
{
	switch( Type ) {
		case ACCELERATION:
			if( m_fCurSpeed < m_fStartSpeed )
				m_fCurSpeed = m_fStartSpeed;
			m_fCurSpeed *= m_fAcceleration;
			if( m_fCurSpeed > m_fMaximumSpeed ) 
				m_fCurSpeed = m_fMaximumSpeed;
			break;
		case DECELERATION:
			if( m_fCurSpeed <= 0.f ) {
				m_fCurSpeed = 0.f;
				break;
			}
			m_fCurSpeed *= m_fDeceleration;
			if( m_fCurSpeed < m_fMinimumSpeed ) 
				m_fCurSpeed = m_fMinimumSpeed;
			break;
	};

	return m_fCurSpeed;
}

void CFcVelocityCtrl::SetCurSpeed( float fValue )
{
	m_fCurSpeed = fValue;
}

void CFcVelocityCtrl::SetStartSpeed( float fValue )
{
	m_fStartSpeed = fValue;
}

void CFcVelocityCtrl::SetMaximumSpeed( float fValue )
{
	m_fMaximumSpeed = fValue;
	if( m_fCurSpeed > m_fMaximumSpeed )
		m_fCurSpeed = m_fMaximumSpeed;
}

void CFcVelocityCtrl::SetMinimumSpeed( float fValue )
{
	m_fMinimumSpeed = fValue;
	if( m_fCurSpeed < m_fMinimumSpeed )
		m_fCurSpeed = m_fMinimumSpeed;
}

void CFcVelocityCtrl::SetAcceleration( float fValue )
{
	m_fAcceleration = fValue;
}

void CFcVelocityCtrl::SetDeceleration( float fValue )
{
	m_fDeceleration = fValue;
}