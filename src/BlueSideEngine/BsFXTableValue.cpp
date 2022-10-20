#include "StdAfx.h"
#include "BsFXTableValue.h"

// CBsFXTableValue
CBsFXTableValue::CBsFXTableValue()
{
	m_fKey = 0.f;
	m_nValueType = -1;
}

CBsFXTableValue::~CBsFXTableValue()
{
}
void CBsFXTableValue::SetValue( void *pValue )
{
}

void *CBsFXTableValue::GetValue()
{
	return NULL;
}

void CBsFXTableValue::SetKey( float fValue )
{
	m_fKey = fValue;
}

float CBsFXTableValue::GetKey()
{
	return m_fKey;
}

int CBsFXTableValue::GetSize()
{
	return 0;
}

void CBsFXTableValue::SetValueType( int nValue )
{
	m_nValueType = nValue;
}

int CBsFXTableValue::GetValueType()
{
	return m_nValueType;
}

CBsFXTableValue *CBsFXTableValue::CreateThis( int nValue )
{
	switch( nValue ) {
		case 0: return new CBsFXColorTableValue;
		case 1: return new CBsFXAlphaTableValue;
		case 2:	return new CBsFXScaleTableValue;
		case 3:	return new CBsFXPositionTableValue;
		case 4:	return new CBsFXRotationTableValue;
	}
	return NULL;
}

// CBsFXColorTable

CBsFXColorTableValue::CBsFXColorTableValue()
{
	m_vColor = D3DXVECTOR3( 0.f, 0.f, 0.f );
	m_nValueType = 0;
}

CBsFXColorTableValue::CBsFXColorTableValue( float fKey, D3DXVECTOR3 vColor )
{
	m_fKey = fKey;
	m_vColor = vColor;
	m_nValueType = 0;
}

CBsFXColorTableValue::~CBsFXColorTableValue()
{
}

void CBsFXColorTableValue::SetColor( float fR, float fG, float fB )
{
	m_vColor = D3DXVECTOR3( fR, fG, fB );
}

void CBsFXColorTableValue::SetColor( D3DXVECTOR3 &vColor )
{
	m_vColor = vColor;
}

void CBsFXColorTableValue::SetRed( float fValue )
{
	m_vColor.x = fValue;
}

void CBsFXColorTableValue::SetGreen( float fValue )
{
	m_vColor.y = fValue;
}

void CBsFXColorTableValue::SetBlue( float fValue )
{
	m_vColor.z = fValue;
}

D3DXVECTOR3 *CBsFXColorTableValue::GetColor()
{
	return &m_vColor;
}

float CBsFXColorTableValue::GetRed()
{
	return m_vColor.x;
}

float CBsFXColorTableValue::GetGreen()
{
	return m_vColor.y;
}

float CBsFXColorTableValue::GetBlue()
{
	return m_vColor.z;
}

void CBsFXColorTableValue::SetValue( void *pValue )
{
	memcpy( &m_vColor, pValue, sizeof(D3DXVECTOR3) );
}

void *CBsFXColorTableValue::GetValue()
{
	return &m_vColor;
}

int CBsFXColorTableValue::GetSize()
{
	return sizeof(D3DXVECTOR3);
}

// CBsFXAlphaTable
CBsFXAlphaTableValue::CBsFXAlphaTableValue()
{
	m_fAlpha = 1.f;
	m_nValueType = 1;
}

CBsFXAlphaTableValue::CBsFXAlphaTableValue( float fKey, float fAlpha )
{
	m_fKey = fKey;
	m_fAlpha = fAlpha;
	m_nValueType = 1;
}

CBsFXAlphaTableValue::~CBsFXAlphaTableValue()
{
}


void CBsFXAlphaTableValue::SetAlpha( float fValue )
{
	m_fAlpha = fValue;
}

float CBsFXAlphaTableValue::GetAlpha()
{
	return m_fAlpha;
}

void CBsFXAlphaTableValue::SetValue( void *pValue )
{
	memcpy( &m_fAlpha, pValue, sizeof(float) );
}

void *CBsFXAlphaTableValue::GetValue()
{
	return &m_fAlpha;
}

int CBsFXAlphaTableValue::GetSize()
{
	return sizeof(float);
}


// CBsFXScaleTable
CBsFXScaleTableValue::CBsFXScaleTableValue()
{
	m_fScale = 1.f;
	m_nValueType = 2;
}

CBsFXScaleTableValue::CBsFXScaleTableValue( float fKey, float fScale )
{
	m_fKey = fKey;
	m_fScale = fScale;
	m_nValueType = 2;
}

CBsFXScaleTableValue::~CBsFXScaleTableValue()
{
}

void CBsFXScaleTableValue::SetScale( float fValue )
{
	m_fScale = fValue;
}

float CBsFXScaleTableValue::GetScale()
{
	return m_fScale;
}

void CBsFXScaleTableValue::SetValue( void *pValue )
{
	memcpy( &m_fScale, pValue, sizeof(float) );
}

void *CBsFXScaleTableValue::GetValue()
{
	return &m_fScale;
}

int CBsFXScaleTableValue::GetSize()
{
	return sizeof(float);
}

// CBsFXPositionTable
CBsFXPositionTableValue::CBsFXPositionTableValue()
{
	m_vPosition = D3DXVECTOR3( 0, 0, 0 );
	m_nValueType = 3;
}

CBsFXPositionTableValue::CBsFXPositionTableValue( float fKey, D3DXVECTOR3 vPos )
{
	m_fKey = fKey;
	m_vPosition = vPos;
	m_nValueType = 3;
}

CBsFXPositionTableValue::~CBsFXPositionTableValue()
{
}

void CBsFXPositionTableValue::SetPosition( D3DXVECTOR3 &vPos )
{
	m_vPosition = vPos;
}

D3DXVECTOR3 *CBsFXPositionTableValue::GetPosition()
{
	return &m_vPosition;
}

void CBsFXPositionTableValue::SetValue( void *pValue )
{
	memcpy( &m_vPosition, pValue, sizeof(D3DXVECTOR3) );
}
void *CBsFXPositionTableValue::GetValue()
{
	return &m_vPosition;
}

int CBsFXPositionTableValue::GetSize()
{
	return sizeof(D3DXVECTOR3);
}

// CBsFXRotationTable
CBsFXRotationTableValue::CBsFXRotationTableValue()
{
	m_vAxisAngle = D3DXVECTOR3( 0, 0, 0 );
	m_nValueType = 4;
}

CBsFXRotationTableValue::CBsFXRotationTableValue( float fKey, D3DXVECTOR3 vAxisAngle )
{
	m_fKey = fKey;
	m_vAxisAngle = vAxisAngle;
	m_nValueType = 4;
}

CBsFXRotationTableValue::~CBsFXRotationTableValue()
{
}

void CBsFXRotationTableValue::SetRotation( D3DXVECTOR3 vAxisAngle )
{
	m_vAxisAngle = vAxisAngle;
}

D3DXVECTOR3 *CBsFXRotationTableValue::GetRotation()
{
	return &m_vAxisAngle;
}

void CBsFXRotationTableValue::SetValue( void *pValue )
{
	memcpy( &m_vAxisAngle, pValue, sizeof(D3DXVECTOR3) );
}

void *CBsFXRotationTableValue::GetValue()
{
	return &m_vAxisAngle;
}

int CBsFXRotationTableValue::GetSize()
{
	return sizeof(D3DXVECTOR3);
}

/*
CBsFXRotationTableValue::CBsFXRotationTableValue()
{
	m_Quat = D3DXQUATERNION( 0, 0, 0, 0 );
}

CBsFXRotationTableValue::CBsFXRotationTableValue( float fKey, D3DXQUATERNION Quat )
{
	m_fKey = fKey;
	m_Quat = Quat;
}

CBsFXRotationTableValue::CBsFXRotationTableValue( float fKey, D3DXVECTOR3 vAxis, float fAngle )
{
	m_fKey = fKey;
	D3DXQuaternionRotationAxis( &m_Quat, &vAxis, fAngle );
}

CBsFXRotationTableValue::~CBsFXRotationTableValue()
{
}

void CBsFXRotationTableValue::SetQuaternion( D3DXQUATERNION &Quat )
{
	m_Quat = Quat;
}

D3DXQUATERNION *CBsFXRotationTableValue::GetQuaternion()
{
	return &m_Quat;
}

void CBsFXRotationTableValue::SetValue( void *pValue )
{
	memcpy( &m_Quat, pValue, sizeof(D3DXQUATERNION) );
}

void *CBsFXRotationTableValue::GetValue()
{
	return &m_Quat;
}
*/