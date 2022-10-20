#pragma once

#include "FcParamVariable.h"
#include "Data/AIDef.h"

class CFcAIObject;
class CFcParamVariable;

class CFcAIHardCodingFunction;

class CFcAIHardCodingFunction {
public:
	CFcAIHardCodingFunction();
	virtual ~CFcAIHardCodingFunction();

	virtual CFcAIHardCodingFunction *Clone() { return NULL; }
	virtual void ExecuteFunction() = 0;
	virtual void Initialize() = 0;
	virtual void ExecuteCallbackFunction( int nIndex = 0 ) {};

	virtual int GetCallbackTypeCount() { return 1; }
	virtual AI_CALLBACK_TYPE GetCallbackType( int nIndex = 0 ) { return AI_CALLBACK_NONE; }


	virtual void SetNullFuncParam( CFcParamVariable *pParam ) = 0;


protected:
	CFcAIObject *m_pParent;
	char *m_szFunctionName;
	int m_nLevel;
	DWORD m_dwParamCount;
	std::vector<CFcParamVariable *> *m_pVecParam;
	std::vector<int> m_nVecDelay;
	DWORD m_dwPrevTick;

	static std::vector<CFcAIHardCodingFunction *> s_pVecFunction;
	static DWORD s_dwDefineOffset;

public:
	void SetFunctionName( char *szFunctionName );
	void SetParent( CFcAIObject *pParent );
	void SetParamPtr( std::vector<CFcParamVariable *> *pParam );
	DWORD GetParamCount();

	int Random( int nMin, int nMax );
	void SetDelay( DWORD dwSlot, int nDelay );
	bool IsDelay( DWORD dwSlot );
	void Process();
	int GetLevel() { return m_nLevel; }

	// Static Function, 간단한거여서 구지 메니저를 따로 만들 필요를 못느꼈다 ㅡㅡ;
	static void CreateObject();
	static void ReleaseObject( bool bExitGame );
	static int FindFunction( const char *szFunctionName, int nLevel = -1 );
	static CFcAIHardCodingFunction *GetFunction( int nIndex );
	static void ReleaseFunction( int nIndex );
	static int FindEmptySlot();

};

