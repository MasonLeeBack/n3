#pragma once

#include "FcGameObject.h"
#include "SmartPtr.h"
#include "Token.h"
#include "Data/AiDef.h"

class CFcAIObject;
class CFcAIElement;
class BStream;
class CFcParamVariable;
class CFcAIGlobalVariableMng;
class Parser;
class CFcAIFunc;
class CFcAISearchSlot;

typedef CSmartPtr<CFcAIObject> AIObjHandle;

#define DEFAULT_AI_OBJECT_POOL_SIZE		1000
#define MAX_LOD_INTERVAL DEFAULT_FRAME_RATE

class CFcAIFuncParam;
class CFcAIObject
{
protected:
	CFcAIObject();

public:
	virtual ~CFcAIObject();

template <class T>
	static CSmartPtr<T> CreateObject()
	{
		T *pInstance;
		CSmartPtr<T> Handle;

		pInstance = new T;
		Handle = CFcAIObject::s_ObjectMng.CreateHandle(pInstance);
		pInstance->SetHandle(Handle);

		return Handle;
	}
	static DWORD s_dwLocalTick;
	static DWORD s_dwTickIntervalOffset;
	static char s_szErrorMessage[512];
	static std::vector<CFcAIObject *> s_pVecEnableList;
	static std::vector<CFcAIObject *> s_pVecDisableList;
	static std::vector<CFcAIObject *> s_pVecFunctionList;
	static std::vector<CFcAIObject *> s_pVecPostEnableList;
	static std::vector<CFcAIObject *> s_pVecPostDisableList;

	BOOL m_bCallbackProcess[AI_CALLBACK_NUM];
protected:
	bool m_bIsFunction;
	bool m_bReset;
	bool m_bEnable;
	bool m_bPreLoad;
	bool m_bCallbackFunc;
	int m_nForceProcess;
#ifdef _XBOX
	char m_szFileName[64];
#else
	std::string m_szFileName;
#endif

	CSmartPtr<CFcAIObject> m_Handle;
	std::vector<CFcAIElement *> m_pVecElement;
	std::vector<CFcAIElement *> m_pVecCallbackElement[AI_CALLBACK_NUM];

	// 전역 변수 관리
	CFcAIGlobalVariableMng *m_pGlobalVariableMng;
	// Include Func List
	std::vector<CFcAIFunc *> m_pVecFunc;
	std::vector<BOOL> m_bVecFuncUse;
	std::vector<CFcAIFuncParam *> m_pVecParamFunc;	// Callback 호출 시 리스트 필요.
	// Search Slot
	CFcAISearchSlot *m_pSearchSlot;
	
	GameObjHandle m_hUnitObject;
	GameObjHandle m_hCurUnitObject;
	DWORD m_dwTickInterval;
	DWORD m_dwCreateTick;
	int m_nDestroyTick;

	int m_nLoadLevel;	// -1 = 레벨 분기 없음, 0이상 레벨별 분기됌.
	int m_nLoadModeType;	// -1 = shared, 0 = friend, 1 = enemy
	// Condition, Action이 Operator 가 있을경우 사용하는 임시 변수들
	std::vector<CFcParamVariable*> m_pVecTempOperatorParam;
	std::vector<int> m_nVecTempOperator;

protected:
	virtual CFcAIObject *IsExist( const char *szFileName, int nLevel, bool bEnemy );
	bool LoadScript( const char *szFileName );
	void Reset();

	// parser 관련
	virtual void InitParser( Parser *pParser );
	virtual void ProcessParser( TokenBuffer &itr ) {}
	virtual bool SetExtensionVariable( CFcParamVariable **pParam, TokenBuffer &itr );

	int GetFunctionIndex( const char *szStr );
	void SetExtensionVariableFromPV( CFcParamVariable *pVariable, TokenBuffer &itr );

public:
	static CSmartPtrMng<CFcAIObject> s_ObjectMng;
	static void InitializeObjects();
	static void ProcessObjects();
	static void ReleaseObjects( bool bExitGame = false );
	static void ReinitializeObjects();
	static void PostListProcess();
	static int GetPreloadIndex( AIObjHandle &Handle );
	static const char *GetPreloadName( int nIndex );

public:
	bool IsFunction();
	void SetHandle( AIObjHandle Handle );

	void SetEnable( bool bEnable );
	bool IsEnable();

	bool IsDestroyThis();

	virtual void GetFuncParamList( const char *szStr, std::vector<int> &vecIndex );
	virtual void PauseFunction( int nFunctionIndex, bool bPause );

	virtual bool Initialize( const char *szFileName, GameObjHandle hUnitObject, bool bPreLoad = false );

	bool Reinitialize();
	const char *GetFileName();
	bool IsBranchLevel();
	bool IsBranchMode();

	CFcAIFunc *GetFunction( DWORD dwIndex );

	void Process();
	void ProcessLOD();

	// Etc
	void SetUnitObjectHandle( GameObjHandle &Handle );
	GameObjHandle GetUnitObjectHandle();
	CFcAIGlobalVariableMng *GetGlobalVariableMng();
	virtual CFcAISearchSlot *GetSearchSlot();
	CFcAIElement *GetElement( DWORD dwIndex );
	void CheckParentType( CFcAIElement *pElement );

	DWORD GetTickInterval();
	void SetTickInterval( DWORD dwValue );

	void SetForceProcessTick( int nValue );

	virtual void ProcessCallbackElement();
	void OnCallback( AI_CALLBACK_TYPE Type );
	void AddFuncParamList( CFcAIFuncParam *pParam ) { m_pVecParamFunc.push_back( pParam ); }
	bool IsCallbackFunc() { return m_bCallbackFunc; }

	virtual void operator = ( const CFcAIObject &Obj );
};
