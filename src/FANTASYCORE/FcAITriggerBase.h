#pragma		once

#include <stdio.h>

class CFcAIElement;

#include "FcParamVariable.h"
#include "FcAIGlobalParam.h"
#include "FcAISearch.h"
class CFcAIBase
{
protected:
	CFcAIBase();

public:
	virtual ~CFcAIBase();

	int GetID()									{ return m_nID; };
	void AddParam( CFcParamVariable *pParam )	{ 
		m_pVecParam.push_back( pParam ); 
		if( pParam ) m_pVecOperator.push_back( NULL );
	}
	void AddOperatorParam( std::vector<CFcParamVariable *> &pVecOperatorParam, std::vector<int> &nVecOperator );
	void SetParent( CFcAIElement *pParent )		{ m_pParent = pParent; }

	void SetNullFuncParam( CFcParamVariable *pParam );

	void operator = ( CFcAIBase &Obj );

protected:
	int m_nID;
	std::vector<CFcParamVariable *> m_pVecParam;

	struct OperatorStruct {
		CFcParamVariable *pResult;
		std::vector<int> nVecOperator;
		std::vector<CFcParamVariable *> pVecParam;
		bool bPreCalc;
	};
//	std::deque<bool> m_bDqIsOperator;
	std::vector<OperatorStruct *> m_pVecOperator;

	CFcAIElement *m_pParent;

protected:
	void CalcOperatorValue( DWORD dwIndex );

	inline CFcParamVariable *GetParam( DWORD dwIndex ) {
		if( m_pVecParam[dwIndex] )
			return CFcAIGlobalParam::GetInstance().GetGlobalParamValue( m_pParent, m_pVecParam[dwIndex] );
		else {
			if( m_pVecOperator[dwIndex]->bPreCalc == true && m_pVecOperator[dwIndex]->pResult ) return m_pVecOperator[dwIndex]->pResult;
			else {
				CalcOperatorValue( dwIndex );
				return m_pVecOperator[dwIndex]->pResult;
			}
		}
		return NULL;
	}
	inline int GetParamInt( DWORD dwIndex )
	{
		if( m_pVecParam[dwIndex] )
			return CFcAIGlobalParam::GetInstance().GetGlobalParamValue( m_pParent, m_pVecParam[dwIndex] )->GetVariableInt();
		else {
			if( m_pVecOperator[dwIndex]->bPreCalc == true && m_pVecOperator[dwIndex]->pResult ) return m_pVecOperator[dwIndex]->pResult->GetVariableInt();
			else {
				CalcOperatorValue( dwIndex );
				return m_pVecOperator[dwIndex]->pResult->GetVariableInt();
			}
		}
		return 0;
	}

	inline float GetParamFloat( DWORD dwIndex )
	{
		if( m_pVecParam[dwIndex] )
			return CFcAIGlobalParam::GetInstance().GetGlobalParamValue( m_pParent, m_pVecParam[dwIndex] )->GetVariableFloat();
		else {
			if( m_pVecOperator[dwIndex]->bPreCalc == true && m_pVecOperator[dwIndex]->pResult ) return m_pVecOperator[dwIndex]->pResult->GetVariableFloat();
			else {
				CalcOperatorValue( dwIndex );
				return m_pVecOperator[dwIndex]->pResult->GetVariableFloat();
			}
		}
		return 0.f;
	}
	inline void *GetParamPtr( DWORD dwIndex ) { return m_pVecParam[dwIndex]->GetVariablePtr(); }
	inline char GetParamChar( DWORD dwIndex ) { return m_pVecParam[dwIndex]->GetVariableChar(); }
	inline char *GetParamString( DWORD dwIndex ) { return m_pVecParam[dwIndex]->GetVariableString(); }
	inline D3DXVECTOR3 GetParamVector( DWORD dwIndex ) { return m_pVecParam[dwIndex]->GetVariableVector(); }
	inline D3DXVECTOR3 *GetParamVectorPtr( DWORD dwIndex ) { return m_pVecParam[dwIndex]->GetVariableVectorPtr(); }
	inline bool GetParamBool( DWORD dwIndex ) { return m_pVecParam[dwIndex]->GetVariableBool(); }
	inline short *GetParamRandom( DWORD dwIndex ) { return m_pVecParam[dwIndex]->GetVariableRandom(); }

};

class CFcAIConditionBase : public CFcAIBase
{
public:
	CFcAIConditionBase();
	virtual ~CFcAIConditionBase();

	virtual bool CheckCondition() { return false; }

	static bool Compare( int nValue1, int nValue2, int nOp );
	static bool Compare( float fValue1, float fValue2, int nOp );

protected:

public:
	static CFcAIConditionBase *CreateObject( int nID, CFcAISearch::SEARCHER_TYPE SearcherType );
//	void operator = ( CFcAIConditionBase &Obj );

};

class CFcAIActionBase : public CFcAIBase
{
public:
	CFcAIActionBase();
	virtual ~CFcAIActionBase();

	virtual void Init() {};							// 액션 다시 실행할 때 초기화 다시 해 줄 필요 있음
	virtual int Command() { return TRUE; };

protected:

public:
	static CFcAIActionBase *CreateObject( int nID, CFcAISearch::SEARCHER_TYPE SearcherType );
//	void operator = ( CFcAIActionBase &Obj );

};
