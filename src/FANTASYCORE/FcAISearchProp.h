#pragma once

#include "FcAISearch.h"
#include "FcGameObject.h"

class CFcParamVariable;

class CFcProp;
class CFcAISearchProp : public CFcAISearch {
public:
	CFcAISearchProp( CFcAIObject *pParent );
	virtual ~CFcAISearchProp();

protected:
	std::vector<CFcProp*> m_pVecPropList;

	virtual void ProcessFilter();
	virtual bool CheckCondition();

public:
	virtual bool Process( int nTick );
	virtual int GetProcessCount();
	virtual void *GetSearchProcess( int nIndex );
};