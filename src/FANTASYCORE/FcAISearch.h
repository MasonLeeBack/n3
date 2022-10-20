#pragma once

#include "Data/AIDef.h"

class CFcAIObject;
class CFcParamVariable;

class CFcAISearch {
public:
	CFcAISearch( CFcAIObject *pParent );
	virtual ~CFcAISearch();

	static int s_nTickIntervalOffset;
	enum SEARCHER_TYPE {
		UNKNOWN = -1,
		GAMEOBJECT,
		PROP,
	};
protected:
	struct SearchFilterStruct {
		AI_SEARCH_FILTER Filter;
		DWORD dwParamOffset;
	};
	struct SearchConStruct {
		AI_SEARCH_CON Con;
		DWORD dwParamOffset;
	};

	SEARCHER_TYPE m_SearcherType;
	AI_SEARCH_TYPE m_Type;
	std::vector<SearchFilterStruct> m_VecFilter;
	std::vector<SearchConStruct> m_VecCon;

	CFcAIObject *m_pParent;

	int m_nStandTick;
	int m_nProcessTick;
	int m_nPrevSearchTick;
	std::vector<CFcParamVariable*> m_pVecParam;

	virtual void ProcessFilter();
	virtual bool CheckCondition();

public:
	SEARCHER_TYPE GetSearcherType();
	virtual bool Process( int nTick );
	virtual int GetProcessCount();
	virtual void *GetSearchProcess( int nIndex );
	void operator = ( CFcAISearch &Obj );

	void SetStandTick( int nTick ) { m_nStandTick = nTick; }

	void SetType( AI_SEARCH_TYPE Type );
	void AddFilter( AI_SEARCH_FILTER Filter );
	void AddCondition( AI_SEARCH_CON Con );
	void SetProcessTick( int nTick );
	void AddParam( CFcParamVariable *pParam );
	void AddParamAlloc( CFcParamVariable &Param );
	void SetNullFuncParam( CFcParamVariable *pParam );
};