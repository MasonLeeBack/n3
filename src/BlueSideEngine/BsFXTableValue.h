#pragma once

class CBsFXTableValue {
public:
	CBsFXTableValue();
	virtual ~CBsFXTableValue();

protected:
	float m_fKey;
	int m_nValueType;

public:
	void SetKey( float fValue );
	float GetKey();

	virtual void SetValue( void *pValue );
	virtual void *GetValue();
	virtual int GetSize();

	void SetValueType( int nValue );
	int GetValueType();

	static CBsFXTableValue *CreateThis( int nValue );
};

class CBsFXColorTableValue : public CBsFXTableValue {
public:
	CBsFXColorTableValue();
	CBsFXColorTableValue( float fKey, D3DXVECTOR3 vColor );

	virtual ~CBsFXColorTableValue();

protected:
	D3DXVECTOR3 m_vColor;

public:
	void SetColor( float fR, float fG, float fB );
	void SetColor( D3DXVECTOR3 &vColor );
	void SetRed( float fValue );
	void SetGreen( float fValue );
	void SetBlue( float fValue );

	D3DXVECTOR3 *GetColor();
	float GetRed();
	float GetGreen();
	float GetBlue();

	virtual void SetValue( void *pValue );
	virtual void *GetValue();
	virtual int GetSize();
};

class CBsFXAlphaTableValue : public CBsFXTableValue {
public:
	CBsFXAlphaTableValue();
	CBsFXAlphaTableValue( float fKey, float fAlpha );

	virtual ~CBsFXAlphaTableValue();

protected:
	float m_fAlpha;

public:
	void SetAlpha( float fValue );

	float GetAlpha();

	virtual void SetValue( void *pValue );
	virtual void *GetValue();
	virtual int GetSize();
};

class CBsFXScaleTableValue : public CBsFXTableValue {
public:
	CBsFXScaleTableValue();
	CBsFXScaleTableValue( float fKey, float fScale );

	virtual ~CBsFXScaleTableValue();

protected:
	float m_fScale;

public:
	void SetScale( float fValue );

	float GetScale();

	virtual void SetValue( void *pValue );
	virtual void *GetValue();
	virtual int GetSize();
};

class CBsFXPositionTableValue : public CBsFXTableValue {
public:
	CBsFXPositionTableValue();
	CBsFXPositionTableValue( float fKey, D3DXVECTOR3 vPos );

	virtual ~CBsFXPositionTableValue();
protected:
	D3DXVECTOR3 m_vPosition;

public:
	void SetPosition( D3DXVECTOR3 &vPos );
	D3DXVECTOR3 *GetPosition();

	virtual void SetValue( void *pValue );
	virtual void *GetValue();
	virtual int GetSize();
};
/*
class CBsFXRotationTableValue : public CBsFXTableValue {
public:
	CBsFXRotationTableValue();
	CBsFXRotationTableValue( float fKey, D3DXQUATERNION Quat );
	CBsFXRotationTableValue( float fKey, D3DXVECTOR3 vAxis, float fAngle );

	virtual ~CBsFXRotationTableValue();
protected:
	D3DXQUATERNION m_Quat;

public:
	void SetQuaternion( D3DXQUATERNION &Quat );
	D3DXQUATERNION *GetQuaternion();

	virtual void SetValue( void *pValue );
	virtual void *GetValue();
};
*/

class CBsFXRotationTableValue : public CBsFXTableValue {
public:
	CBsFXRotationTableValue();
	CBsFXRotationTableValue( float fKey, D3DXVECTOR3 vAxisAngle );

	virtual ~CBsFXRotationTableValue();
protected:
	D3DXVECTOR3 m_vAxisAngle;
//	D3DXVECTOR4 m_vAxisAndAngle;
//	float m_fAngle;	// Radian

public:
	void SetRotation( D3DXVECTOR3 vAxisAngle );
	D3DXVECTOR3 *GetRotation();

	virtual void SetValue( void *pValue );
	virtual void *GetValue();
	virtual int GetSize();
};