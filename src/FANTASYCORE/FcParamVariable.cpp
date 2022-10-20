#include "StdAfx.h"
#include "FcCommon.h"
#include "FcParamVariable.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


char *CopyString( const char *szStr )
{
	if( szStr == NULL ) return NULL;
	int nLength = (int)strlen(szStr);
	char *pResult = new char[nLength+1];
	memcpy( pResult, szStr, nLength );
	pResult[nLength] = 0;

	return pResult;
}


CFcParamVariable::CFcParamVariable()
{
	m_Type = UNKNOWN;
	m_szDescription = NULL;
	m_pPtr = NULL;
}

CFcParamVariable::CFcParamVariable( VT Type )
{
	m_Type = Type;
	m_szDescription = NULL;
	m_pPtr = NULL;

	switch( Type ) {
		case VECTOR:
			m_pVecValue = new D3DXVECTOR3;
			m_pVecValue->x = m_pVecValue->y = m_pVecValue->z = 0.f;
			break;

	}
}

CFcParamVariable::CFcParamVariable( int nValue )
{
	m_Type = UNKNOWN;
	m_szDescription = NULL;
	m_pPtr = NULL;
	SetVariable( nValue );
}

CFcParamVariable::CFcParamVariable( void *pPtr )
{
	m_Type = UNKNOWN;
	m_szDescription = NULL;
	m_pPtr = NULL;
	SetVariable( pPtr );
}

CFcParamVariable::CFcParamVariable( float fValue )
{
	m_Type = UNKNOWN;
	m_szDescription = NULL;
	m_pPtr = NULL;
	SetVariable( fValue );
}

CFcParamVariable::CFcParamVariable( char cValue )
{
	m_Type = UNKNOWN;
	m_szDescription = NULL;
	m_pPtr = NULL;
	SetVariable( cValue );
}

CFcParamVariable::CFcParamVariable( char *szValue )
{
	m_Type = UNKNOWN;
	m_szDescription = NULL;
	m_pPtr = NULL;
	SetVariable( szValue );

}

CFcParamVariable::CFcParamVariable( D3DXVECTOR3 Vec )
{
	m_Type = UNKNOWN;
	m_szDescription = NULL;
	m_pPtr = NULL;
	SetVariable( Vec );
}

CFcParamVariable::CFcParamVariable( bool bValue )
{
	m_Type = UNKNOWN;
	m_szDescription = NULL;
	m_pPtr = NULL;
	SetVariable( bValue );
}

CFcParamVariable::CFcParamVariable( int nMin, int nMax )
{
	m_Type = UNKNOWN;
	m_szDescription = NULL;
	m_pPtr = NULL;
	SetVariable( nMin, nMax  );
}


CFcParamVariable::~CFcParamVariable()
{
	Clear();
}

void CFcParamVariable::SetDescription( const char *szDescription )
{
	SAFE_DELETEA( m_szDescription );
	if( szDescription == NULL ) return;
	m_szDescription = CopyString( szDescription );
}

const char *CFcParamVariable::GetDescription()
{
	return m_szDescription;
}


void CFcParamVariable::Clear( VT Type )
{
	SAFE_DELETEA( m_szDescription );
	switch( m_Type ) {
		case STRING:
		case CUSTOM:
			SAFE_DELETEA( m_szValue );
			break;
		case VECTOR:
			SAFE_DELETE( m_pVecValue );
			break;
	}
	m_pPtr = NULL;
	m_Type = Type;
}

void CFcParamVariable::SetVariable( void *pPtr )
{
	m_pPtr = pPtr;
	if( m_Type != UNKNOWN )		{ BsAssert( m_Type == PTR || m_Type == CUSTOM ); }
	else						{ m_Type = PTR; }
}

void CFcParamVariable::SetVariable( int nValue )
{
	m_nValue = nValue;
	if( m_Type != UNKNOWN )			{ BsAssert( m_Type == INT || m_Type == CUSTOM ); }
	else						{ m_Type = INT; }
}

void CFcParamVariable::SetVariable( float fValue )
{
	m_fValue = fValue;
	if( m_Type != UNKNOWN )		{ BsAssert( m_Type == FLOAT || m_Type == CUSTOM ); }
	else						{ m_Type = FLOAT; }
}

void CFcParamVariable::SetVariable( char cValue )
{
	m_cValue = cValue;
	if( m_Type != UNKNOWN )		{ BsAssert( m_Type == CHAR || m_Type == CUSTOM ); }
	else						{ m_Type = CHAR; }
}

void CFcParamVariable::SetVariable( char *szValue )
{
	SAFE_DELETEA( m_szValue );
	m_szValue = CopyString( szValue );
	if( m_Type != UNKNOWN )		{ BsAssert( m_Type == STRING || m_Type == CUSTOM ); }
	else						{ m_Type = STRING; }
}

void CFcParamVariable::SetVariable( bool bValue )
{
	m_bValue = bValue;
	if( m_Type != UNKNOWN )		{ BsAssert( m_Type == BOOLEAN || m_Type == CUSTOM ); }
	else						{ m_Type = BOOLEAN; }
}

void CFcParamVariable::SetVariable( D3DXVECTOR3 Vec )
{
	if( m_pVecValue == NULL )
	{
		m_pVecValue = new D3DXVECTOR3;
		m_pVecValue->x = m_pVecValue->y = m_pVecValue->z = 0.f;
	}

	*m_pVecValue = Vec;

	if( m_Type != UNKNOWN )		{ BsAssert( m_Type == VECTOR || m_Type == CUSTOM ); }
	else						{ m_Type = VECTOR; }

	
}

void CFcParamVariable::SetVariable( int nMin, int nMax )
{
	m_nRandomValue[0] = nMin;
	m_nRandomValue[1] = nMax;
}

void CFcParamVariable::operator = ( CFcParamVariable &Param )
{
	SAFE_DELETEA( m_szDescription );
	m_szDescription = CopyString( Param.m_szDescription );
	switch( Param.m_Type ) {
		case INT:		SetVariable( Param.GetVariableInt() );		break;
		case FLOAT:		SetVariable( Param.GetVariableFloat() );	break;
		case CHAR:		SetVariable( Param.GetVariableChar() );		break;
		case STRING:	SetVariable( Param.GetVariableString() );	break;
		case VECTOR:	SetVariable( Param.GetVariableVector() );	break;
		case PTR:		SetVariable( Param.GetVariablePtr() );		break;
		case BOOLEAN:	SetVariable( Param.GetVariableBool() );		break;
		case RANDOM:	SetVariable( Param.GetVariableRandom()[0], Param.GetVariableRandom()[1] );		break;
		case CUSTOM:	SetVariable( Param.GetVariableString() );		break;
	}
}

bool CFcParamVariable::operator > ( CFcParamVariable &Param )
{
	switch( m_Type ) {
		case INT:		return ( m_nValue > Param.m_nValue );
		case FLOAT:		return ( m_fValue > Param.m_fValue );
		case CHAR:		return ( m_cValue > Param.m_cValue );
		case VECTOR:	
			switch( Param.m_Type ) {
				case INT:		return ( D3DXVec3LengthSq( m_pVecValue ) > Param.m_nValue * Param.m_nValue );
				case FLOAT:		return ( D3DXVec3LengthSq( m_pVecValue ) > Param.m_fValue * Param.m_fValue );
				case VECTOR:	return ( *m_pVecValue > *Param.m_pVecValue );
			}
		case RANDOM:
			switch( Param.m_Type ) {
				case INT:		return ( GetGenRandom() > Param.m_nValue );
			}
	}
	return false;
}

#include "FcGlobal.h"
bool CFcParamVariable::operator < ( CFcParamVariable &Param )
{
	switch( m_Type ) {
		case INT:		return ( m_nValue < Param.m_nValue );
		case FLOAT:		return ( m_fValue < Param.m_fValue );
		case CHAR:		return ( m_cValue < Param.m_cValue );
		case VECTOR:	
			switch( Param.m_Type ) {
				case INT:		return ( D3DXVec3LengthSq( m_pVecValue ) < Param.m_nValue * Param.m_nValue );
				case FLOAT:		return ( D3DXVec3LengthSq( m_pVecValue ) < Param.m_fValue * Param.m_fValue );
			}
		case RANDOM:
			switch( Param.m_Type ) {
				case INT:	return ( GetGenRandom() < Param.m_nValue );
			}
	}
	return false;
}

bool CFcParamVariable::operator >= ( CFcParamVariable &Param )
{
	switch( m_Type ) {
		case INT:		return ( m_nValue >= Param.m_nValue );
		case FLOAT:		return ( m_fValue >= Param.m_fValue );
		case CHAR:		return ( m_cValue >= Param.m_cValue );
		case VECTOR:
			switch( Param.m_Type ) {
				case INT:		return ( D3DXVec3LengthSq( m_pVecValue ) >= Param.m_nValue * Param.m_nValue );
				case FLOAT:		return ( D3DXVec3LengthSq( m_pVecValue ) >= Param.m_fValue * Param.m_fValue );
			}
		case RANDOM:
			switch( Param.m_Type ) {
				case INT:		return ( GetGenRandom() >= Param.m_nValue );
			}
	}
	return false;
}

bool CFcParamVariable::operator <= ( CFcParamVariable &Param )
{
	switch( m_Type ) {
		case INT:		return ( m_nValue <= Param.m_nValue );
		case FLOAT:		return ( m_fValue <= Param.m_fValue );
		case CHAR:		return ( m_cValue <= Param.m_cValue );
		case VECTOR:
			switch( Param.m_Type ) {
				case INT:		return ( D3DXVec3LengthSq( m_pVecValue ) <= Param.m_nValue * Param.m_nValue );
				case FLOAT:		return ( D3DXVec3LengthSq( m_pVecValue ) <= Param.m_fValue * Param.m_fValue );
			}
		case RANDOM:
			switch( Param.m_Type ) {
				case INT:		return ( GetGenRandom() <= Param.m_nValue );
			}
	}
	return false;
}

bool CFcParamVariable::operator == ( CFcParamVariable &Param )
{
	switch( m_Type ) {
		case INT:		return ( m_nValue == Param.m_nValue );
		case FLOAT:		return ( m_fValue == Param.m_fValue );
		case CHAR:		return ( m_cValue == Param.m_cValue );
		case STRING:	return ( strcmp( m_szValue, Param.m_szValue ) == 0 ) ? true : false;
		case VECTOR:
			switch( Param.m_Type ) {
				case INT:		return ( D3DXVec3LengthSq( m_pVecValue ) == Param.m_nValue * Param.m_nValue );
				case FLOAT:		return ( D3DXVec3LengthSq( m_pVecValue ) == Param.m_fValue * Param.m_fValue );
				case VECTOR:	return ( *m_pVecValue == *Param.m_pVecValue ) ? true : false;
			}
		case RANDOM:
			switch( Param.m_Type ) {
				case INT:		return ( GetGenRandom() == Param.m_nValue );
			}
		case PTR:	return ( m_pPtr == Param.m_pPtr );
	}
	return false;
}

bool CFcParamVariable::operator != ( CFcParamVariable &Param )
{
	switch( m_Type ) {
		case INT:		return ( m_nValue != Param.m_nValue );
		case FLOAT:		return ( m_fValue != Param.m_fValue );
		case CHAR:		return ( m_cValue != Param.m_cValue );
		case STRING:	return ( strcmp( m_szValue, Param.m_szValue ) == 0 ) ? false : true;
		case VECTOR:
			switch( Param.m_Type ) {
				case INT:		return ( D3DXVec3LengthSq( m_pVecValue ) != Param.m_nValue * Param.m_nValue );
				case FLOAT:		return ( D3DXVec3LengthSq( m_pVecValue ) != Param.m_fValue * Param.m_fValue );
				case VECTOR:	return ( *m_pVecValue != *Param.m_pVecValue ) ? false : true;
			}
		case RANDOM:
			switch( Param.m_Type ) {
				case INT:		return ( GetGenRandom() != Param.m_nValue );
			}
		case PTR:	
			{
				return ( m_pPtr != Param.m_pPtr );
			}
	}
	return false;
}

CFcParamVariable &CFcParamVariable::operator += ( CFcParamVariable &a )
{
	switch( m_Type ) {
		case INT:
			switch( a.m_Type ) {
				case INT:	m_nValue += a.m_nValue;		break;
				case FLOAT:	m_nValue += (int)a.m_fValue;	break;
			}
			break;
		case FLOAT:
			switch( a.m_Type ) {
				case INT:	m_fValue += (float)a.m_nValue;	break;
				case FLOAT:	m_fValue += a.m_fValue;	break;
			}
			break;
	}
	return *this;
}

CFcParamVariable &CFcParamVariable::operator -= ( CFcParamVariable &a )
{
	switch( m_Type ) {
		case INT:
			switch( a.m_Type ) {
				case INT:	m_nValue -= a.m_nValue;		break;
				case FLOAT:	m_nValue -= (int)a.m_fValue;	break;
			}
			break;
		case FLOAT:
			switch( a.m_Type ) {
				case INT:	m_fValue -= (float)a.m_nValue;	break;
				case FLOAT:	m_fValue -= a.m_fValue;	break;
			}
			break;
	}
	return *this;
}

CFcParamVariable &CFcParamVariable::operator *= ( CFcParamVariable &a )
{
	switch( m_Type ) {
		case INT:
			switch( a.m_Type ) {
				case INT:	m_nValue *= a.m_nValue;		break;
				case FLOAT:	m_nValue = int( m_nValue * a.m_fValue);	break;
			}
			break;
		case FLOAT:
			switch( a.m_Type ) {
				case INT:	m_fValue *= (float)a.m_nValue;	break;
				case FLOAT:	m_fValue *= a.m_fValue;	break;
			}
			break;
	}
	return *this;
}

CFcParamVariable &CFcParamVariable::operator /= ( CFcParamVariable &a )
{
	switch( m_Type ) {
		case INT:
			switch( a.m_Type ) {
				case INT:	m_nValue /= a.m_nValue;		break;
				case FLOAT:	m_nValue /= (int)a.m_fValue;	break;
			}
			break;
		case FLOAT:
			switch( a.m_Type ) {
				case INT:	m_fValue /= (float)a.m_nValue;	break;
				case FLOAT:	m_fValue /= a.m_fValue;	break;
			}
			break;
	}
	return *this;
}
