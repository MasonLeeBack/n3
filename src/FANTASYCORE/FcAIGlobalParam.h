#pragma once
#include "Singleton.h"
#include "FcParamVariable.h"

class CFcAIElement;

class CFcAIGlobalParam : public CSingleton<CFcAIGlobalParam>
{
public:
	CFcAIGlobalParam();
	virtual ~CFcAIGlobalParam();

	struct GlobalParamStruct {
		CFcParamVariable::VT VariableType;
		char *szStr;
		bool bChange;
	};
	static GlobalParamStruct s_GlobalParamList[];
protected:
	DWORD m_dwCount;
	CFcParamVariable m_TempValue;

	void GetValue( CFcAIElement *pElement, int nIndex );

public:
	int GetIndex( const char *szStr );
	bool IsValidGlobalParam( const char *szStr );

	DWORD GetGlobalParamCount();
	const char *GetGlobalParamString( DWORD dwIndex );

	CFcParamVariable *GetGlobalParamValue( CFcAIElement *pElement, CFcParamVariable *pParam );
};

extern CFcAIGlobalParam g_AIGlobalParam;