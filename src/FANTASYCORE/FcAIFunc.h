#pragma once
#include "FcAIObject.h"

class CFcAIFunc;
class CFcParamVariable;

// AIObject �� ��ӹ��� Func Ŭ����
// ai_func �� ��� �߰������� �д� �͵鶧���� ������ ����
// ���� ��ġ ������ ��Ÿ�� �Ѥ�;
class CFcAIFunc : public CFcAIObject
{
public:
	CFcAIFunc();
	virtual ~CFcAIFunc();

protected:
	CFcAIObject *m_pParent;
	int m_nHardCodingFunctionIndex;
	// Function Parameter
	// �ļ� ������ ���Ǵ� �Էº�������
	// ���� ������ ��ü�� �ƴ϶� m_pVecParam�� �����͸� �޾Ƽ� ����Ѵ�.
	// Delete �� ����..
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

// Element �ּ� ������ �־�� �ϴ� Ŭ����
// ai_func �� �����ϱ� ���ؼ�
// ���Ǿ��� ���� �Ķ���͸� ���� ������ �ִ°� ���� ����..
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