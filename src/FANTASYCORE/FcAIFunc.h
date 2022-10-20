#pragma once
#include "FcAIObject.h"

class CFcAIFunc;
class CFcParamVariable;

// AIObject 를 상속받은 Func 클래스
// ai_func 일 경우 추가적으로 읽는 것들때문에 밖으로 뺐다
// 모양상 조치 않을꺼 가타서 ㅡㅡ;
class CFcAIFunc : public CFcAIObject
{
public:
	CFcAIFunc();
	virtual ~CFcAIFunc();

protected:
	CFcAIObject *m_pParent;
	int m_nHardCodingFunctionIndex;
	// Function Parameter
	// 파서 내에서 사용되는 입력변수들은
	// 새로 생선된 객체가 아니라 m_pVecParam의 포인터를 받아서 사용한다.
	// Delete 시 주의..
	std::vector<CFcParamVariable *> m_pVecParam;

protected:
	virtual CFcAIObject *IsExist( const char *szFileName, int nLevel, bool bEnemy );
	virtual void InitParser( Parser *pParser );
	virtual void ProcessParser( TokenBuffer &itr );
	virtual bool SetExtensionVariable( CFcParamVariable **pParam, TokenBuffer &itr );

public:
	void SetParent( CFcAIObject *pParent );
	CFcAIObject *GetParent();
	void SetHardCodingFunctionIndex( int nIndex );
	int GetHardCodingFunctionIndex();

	virtual void ProcessCallbackElement();

	const char *GetFuncString();
	DWORD GetParamCount();
	CFcParamVariable *GetParam( DWORD dwIndex );
	CFcParamVariable *GetParamFromDescription( const char *szDescription );

	virtual CFcAISearchSlot *GetSearchSlot();

	virtual void GetFuncParamList( const char *szStr, std::vector<int> &vecIndex );
	virtual void PauseFunction( int nFunctionIndex, bool bPause );

	virtual void operator = ( const CFcAIFunc &Obj );
};

// Element 애서 가지구 있어야 하는 클래스
// ai_func 을 공유하기 위해선
// 사용되어진 곳의 파라메터만 따로 가지구 있는게 낮기 땜시..
class CFcAIFuncParam
{
public:
	CFcAIFuncParam( CFcAIFunc *pAIFunc, int nIndex );
	~CFcAIFuncParam();

protected:
	CFcAIFunc *m_pAIFunc;
	int m_nIndex;
	std::vector<CFcParamVariable *> m_pVecParam;
	bool m_bPause;

public:
	void AddParam( CFcParamVariable *pParam );
	void ProcessFunc();
	void ProcessCallbackFunc();
	void Initialize();
	CFcAIFunc *GetFunc() { return m_pAIFunc; }
	void SetPause( bool bPause ) { m_bPause = bPause; }

	int GetIndex() { return m_nIndex; }

	void operator = ( const CFcAIFuncParam &Obj );

};