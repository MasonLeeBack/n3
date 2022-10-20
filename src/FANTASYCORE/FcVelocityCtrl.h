#pragma once

class CFcVelocityCtrl
{
public:
	CFcVelocityCtrl();
	CFcVelocityCtrl( float fStartSpeed, float fMaximumSpeed, float fMinimumSpeed, float fAcceleration, float fDeceleration );
	virtual ~CFcVelocityCtrl();

	enum VELOCITY_TYPE {
		ACCELERATION,
		DECELERATION,
	};

protected:
	float m_fStartSpeed;
	float m_fMaximumSpeed;
	float m_fMinimumSpeed;
	float m_fAcceleration;
	float m_fDeceleration;

	float m_fCurSpeed;

public:
	void Reset();
	float GetSpeed( VELOCITY_TYPE Type );

	void SetStartSpeed( float fValue );
	void SetMaximumSpeed( float fValue );
	void SetMinimumSpeed( float fValue );
	void SetAcceleration( float fValue );
	void SetDeceleration( float fValue );
	void SetCurSpeed( float fValue );

	float GetMaximumSpeed() { return m_fMaximumSpeed; }
	float GetDeceleration() { return m_fDeceleration; }
	float GetAcceleration() { return m_fAcceleration; }
	float GetMinSpeed() { return m_fMinimumSpeed; }
	float GetMaxSpeed() { return m_fMaximumSpeed; }
	float GetCurSpeed() { return m_fCurSpeed; }
};