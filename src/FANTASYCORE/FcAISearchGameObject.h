#pragma once

#include "FcAISearch.h"
#include "FcGameObject.h"

class CFcParamVariable;


class CFcAISearchGameObject : public CFcAISearch {
public:
	CFcAISearchGameObject( CFcAIObject *pParent );
	virtual ~CFcAISearchGameObject();

protected:
	std::vector<GameObjHandle> m_VecHandleList;
	D3DXVECTOR2 m_vLastSearchPos;

	virtual void ProcessFilter();
	virtual bool CheckCondition();

public:
	virtual bool Process( int nTick );
	virtual int GetProcessCount();
	virtual void *GetSearchProcess( int nIndex );
};