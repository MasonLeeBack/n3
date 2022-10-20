#pragma once

#include "SmartPtr.h"

class CFcTroopAIObject;
typedef CSmartPtr<CFcTroopAIObject>	TroopAIObjHandle;
class BStream;
class CFcParamVariable;
struct TROOPAIGROUPDATA;
class CFcTroopObject;
typedef CSmartPtr<CFcTroopObject>	TroopObjHandle;
struct TROOPAITRIGGERDATA;
class CFcProp;
struct AREA_INFO;


#define DEFAULT_TROOP_AI_OBJECT_POOL_SIZE		40

#define FILTER_DATA_ADD			0
#define FILTER_DATA_SUBTRACT	1

#define MAX_AI_PATH_ID			128
#define MAX_AI_GUIDE_DATA		16

struct FILTER_DATA
{
	int nOper;
	int nType;
};


struct AIGUIDE_DATA
{
	AIGUIDE_DATA()
	{
		pProp = NULL;
		pArea = NULL;
	}

	CFcProp* pProp;
	AREA_INFO* pArea;
};



class CFcTroopAIObject
{
protected:
	CFcTroopAIObject();

public:
	virtual ~CFcTroopAIObject();

	template <class T>
		static CSmartPtr<T> CreateObject()
	{
		T *pInstance;
		CSmartPtr<T> Handle;

		pInstance = new T;
		Handle = CFcTroopAIObject::s_ObjectMng.CreateHandle(pInstance);
		pInstance->SetHandle(Handle);

		return Handle;
	}

protected:
	CSmartPtr< CFcTroopAIObject > m_Handle;
	TROOPAIGROUPDATA* m_pGroupData;
	TroopObjHandle m_hTroop;

	std::vector<TroopObjHandle> m_VecTargetObjects;
	int m_nAIID;
	bool m_bEnable;
	char m_cCurLabel[32];

	static int s_PathID[MAX_AI_PATH_ID];
	static AIGUIDE_DATA s_AIGuideData[MAX_AI_GUIDE_DATA];

	D3DXVECTOR2 m_PreHeroPos;		// use follow
	TroopObjHandle m_hGuardianTarget;

	TroopObjHandle m_hTroopByIDFilter;

	int m_nLastPath1, m_nLastPath2, m_nLastPathType, m_nLastPathDir;

public:
	static bool InitLoadScript( const char *szFileName );		// 한번만 한다.

	static void ProcessObjects();
	static void ReleaseObjects();

	static void SetAI( TroopObjHandle hTroop, int nAIID );
	static void SetAIPath( int nPathID, int nAIPathID );
	static void SetEnableAI( TroopObjHandle hTroop, bool bEnable );
	static bool IsEnableAI( TroopObjHandle hTroop );

	static int GetAI( TroopObjHandle hTroop );
	static void SetAIGuide( int nIndex, CFcProp* pProp, AREA_INFO* pAreaInfo );

protected:
	static std::vector<TROOPAIGROUPDATA*> s_VecGroupData;
	static CSmartPtrMng<CFcTroopAIObject> s_ObjectMng;

	

public:
	bool IsParentTroop()					{ if( m_hTroop == NULL ) { return false; } return true; }

	int GetAIID()							{ return m_nAIID; }
	TroopObjHandle GetParentTroop()			{ return m_hTroop; }
	void SetEnable( bool bEnable )			{ m_bEnable = bEnable; }
	bool IsEnable()							{ return m_bEnable; }

	void SetHandle( TroopAIObjHandle Handle ) { m_Handle = Handle; }
	void SetGuardianTarget( TroopObjHandle hTroop ) { m_hGuardianTarget = hTroop; }
	virtual bool Initialize( int nAIID, TroopObjHandle hTroopObject );
	virtual void Process();

	void operator = ( const CFcTroopAIObject &Obj );

protected:
	bool CheckCmd( TROOPAITRIGGERDATA* pData );
	bool CheckIf( TROOPAITRIGGERDATA* pData );

	bool CmdEngage( TROOPAITRIGGERDATA* pData );
	bool CmdEngageMercifully( TROOPAITRIGGERDATA* pData );
	bool CmdEngageSmart( TROOPAITRIGGERDATA* pData );
	bool CmdEngageStanding( TROOPAITRIGGERDATA* pData );
	bool CmdEngageStandingMercifully( TROOPAITRIGGERDATA* pData );
	bool CmdDisengage( TROOPAITRIGGERDATA* pData );
	bool CmdDefense( TROOPAITRIGGERDATA* pData );
	bool CmdDefenseProp( TROOPAITRIGGERDATA* pData );
	bool CmdUseAbility( TROOPAITRIGGERDATA* pData );
	bool CmdMoveOnPath( TROOPAITRIGGERDATA* pData );
	bool CmdMoveOnPathLoop( TROOPAITRIGGERDATA* pData );
	bool CmdMoveOnPathRandom( TROOPAITRIGGERDATA* pData );
	bool CmdMoveOnPathWithAvoidEnemy( TROOPAITRIGGERDATA* pData );
	bool CmdMoveOnFullPath( TROOPAITRIGGERDATA* pData );
	bool CmdAttackEnemyNearSomebody( TROOPAITRIGGERDATA* pData );
	bool CmdAttackWall( TROOPAITRIGGERDATA* pData );
	bool CmdSetTargetID( TROOPAITRIGGERDATA* pData );
	bool CmdFollow( TROOPAITRIGGERDATA* pData );
	bool CmdEngageGuardianToTarget( TROOPAITRIGGERDATA* pData );
	bool CmdEngageArea( TROOPAITRIGGERDATA* pData );
	bool CmdDisengageOnAttacked( TROOPAITRIGGERDATA* pData );
	bool CmdEngageMercifullyArea( TROOPAITRIGGERDATA* pData );

	bool MoveByAIGuide( D3DXVECTOR2* pPos, int nRange, FILTER_DATA* pFilter, int nNumFilter );


/*
	bool CmdDisengage( TROOPAITRIGGERDATA* pData );
	bool CmdModeEngage( TROOPAITRIGGERDATA* pData );
	bool CmdModeStopAndDefense( TROOPAITRIGGERDATA* pData );
	bool CmdResonableEngage( TROOPAITRIGGERDATA* pData );

	bool ConIsTarget( TROOPAIELEMENTDATA* pData );
	bool ConTargetHP( TROOPAIELEMENTDATA* pData );
	bool ConIsIdle( TROOPAIELEMENTDATA* pData );
	bool ConRatio( TROOPAIELEMENTDATA* pData );
*/

	bool _IfHP( TROOPAITRIGGERDATA* pData );
	bool _IfMeleeEngaged( TROOPAITRIGGERDATA* pData );
	bool _IfEngaged( TROOPAITRIGGERDATA* pData );
	bool _IfNumFriedlyTroop( TROOPAITRIGGERDATA* pData );
	bool _IfNumEnemyTroop( TROOPAITRIGGERDATA* pData );
	bool _IfThereIsTroop( TROOPAITRIGGERDATA* pData );
	bool _IfIAm( TROOPAITRIGGERDATA* pData );

	TroopObjHandle CheckTargetTroop( D3DXVECTOR2* pPos, int nRange, FILTER_DATA* pFilter, int nNumFilter, std::vector<TroopObjHandle> &Objects, int nMaxEngage = 999999 );

	CFcProp* CheckTargetProp( D3DXVECTOR2* pPos, int nRange, FILTER_DATA* pFilter, int nNumFilter );
	AREA_INFO* CheckTargetArea( D3DXVECTOR2* pPos, int nRange, FILTER_DATA* pFilter, int nNumFilter );

	int GetNumEngage( TroopObjHandle hTroop, int nFilter );
	bool CompareValue( int nValue1, char* pOperStr, int nValue2 );
	bool CheckFilter( TroopObjHandle hTroop, TroopObjHandle hTargetTroop, FILTER_DATA* pFilter, int nNumFilter );
	int SetFilterData( CFcParamVariable* Variable, FILTER_DATA** ppFilter );

	PATH_PACK* GetNearFullPath( D3DXVECTOR3& Pos, int nPath1, int nPath2, int nTypeDir );
	PATH_PACK* GetBestPathNode( D3DXVECTOR3& Pos, int nPath1, int nPath2, int nMoveDirType, int* nPathIndex  );
	PATH_PACK* GetRandomPath( D3DXVECTOR3& Pos, int nPath1, int nPath2, int* nPathIndex, int nMaxUsePath );

	bool CanICmdMove( TroopObjHandle hTroop, PATH_PACK* pPath, int nMoveDirType, bool bLoop);	

	bool IsMovableAttr( float fX, float fY, int nRadius );

	bool IsOnPathState();
};	
