#pragma once

#include "FcAIObject.h"
#include "FcParamVariable.h"

class CFcAIGlobalVariableMng {
public:
	CFcAIGlobalVariableMng( CFcAIObject *pParent );
	~CFcAIGlobalVariableMng();

	void AddVariable( CFcParamVariable *pVariable );
	CFcParamVariable *GetVariable( DWORD dwIndex );
	int GetVariableIndex( const char *szDescription );

protected:
	CFcAIObject *m_pParent;
	std::vector<CFcParamVariable *> m_pVecList;

public:
	void operator = ( CFcAIGlobalVariableMng &Obj );

};

class CFcParamVariableEx : public CFcParamVariable {
public:
	CFcParamVariableEx( VT Type, CFcAIGlobalVariableMng *pMng = NULL );
	virtual ~CFcParamVariableEx();

	virtual int GetVariableInt();

protected:
	CFcAIGlobalVariableMng *m_pGlobalMng;
	int m_nGlobalValueIndex;
};