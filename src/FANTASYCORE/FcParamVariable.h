#pragma once

#include "FcUtil.h"
class CFcParamVariable {
public:
	enum VT {
		UNKNOWN = -1,
		INT,
		FLOAT,
		CHAR,
		STRING,
		VECTOR,
		PTR,
		BOOLEAN,
		RANDOM,
		CUSTOM,
	};

	CFcParamVariable();
	CFcParamVariable( VT Type );
	CFcParamVariable( int nValue );
	CFcParamVariable( void *pPtr );
	CFcParamVariable( float fValue );
	CFcParamVariable( char cValue );
	CFcParamVariable( char *szValue );
	CFcParamVariable( D3DXVECTOR3 Vec );
	CFcParamVariable( bool bValue );
	CFcParamVariable( int nMin, int nMax );
	virtual ~CFcParamVariable();

	union {
		void *m_pPtr;

		int m_nValue;
		float m_fValue;
		char m_cValue;
		char *m_szValue;
		bool m_bValue;
		D3DXVECTOR3 *m_pVecValue;
		short m_nRandomValue[2];
	};

protected:
	VT m_Type;
	char *m_szDescription;

public:
	VT GetType() { return m_Type; }
	void SetDescription( const char *szDescription );
	const char *GetDescription();
	void Clear( VT Type = UNKNOWN );

	void SetVariable( void *pPtr );
	void SetVariable( int nValue );
	void SetVariable( float fValue );
	void SetVariable( char cValue );
	void SetVariable( char *szValue );
	void SetVariable( D3DXVECTOR3 Vec );
	void SetVariable( bool bValue );
	void SetVariable( int nMin, int nMax );

	void *GetVariablePtr() { return m_pPtr; }
	virtual int GetVariableInt() { return m_nValue; }
	float GetVariableFloat() { return m_fValue; }
	char GetVariableChar() { return m_cValue; }
	char *GetVariableString() { return m_szValue; }
	D3DXVECTOR3 GetVariableVector() { return *m_pVecValue; }
	D3DXVECTOR3 *GetVariableVectorPtr() { return m_pVecValue; }
	bool GetVariableBool() { return m_bValue; }
	short *GetVariableRandom() { return m_nRandomValue; }
	short GetGenRandom() { return m_nRandomValue[0] + Random( m_nRandomValue[1] - m_nRandomValue[0] ); }

	void operator = ( CFcParamVariable &Param );
	bool operator > ( CFcParamVariable &Param );
	bool operator < ( CFcParamVariable &Param );
	bool operator >= ( CFcParamVariable &Param );
	bool operator <= ( CFcParamVariable &Param );
	bool operator == ( CFcParamVariable &Param );
	bool operator != ( CFcParamVariable &Param );

	CFcParamVariable &operator += ( CFcParamVariable &a );
	CFcParamVariable &operator -= ( CFcParamVariable &a );
	CFcParamVariable &operator *= ( CFcParamVariable &a );
	CFcParamVariable &operator /= ( CFcParamVariable &a );
};
