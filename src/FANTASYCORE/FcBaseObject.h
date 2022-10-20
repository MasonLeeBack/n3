#pragma once

#include "BsKernel.h"
#include "Box3.h"
#include "SmartPtr.h"
#include "FcCommon.h"

#include "CrossVector.h"

#define DEFAULT_BASE_OBJECT_POOL_SIZE		10

class CBsKernel;
struct KEY_EVENT_PARAM;

class CFcBaseObject;
typedef CSmartPtr<CFcBaseObject> BaseObjHandle;

class CFcBaseObject
{
protected:
	CFcBaseObject( CCrossVector *pCross, bool bInsertProcess = true );

public:
	virtual ~CFcBaseObject();

template <class T>
	static CSmartPtr<T> CreateObject(CCrossVector *pCross)
	{
		T *pInstance;
		CSmartPtr<T> Handle;

		pInstance=new T(pCross);
		Handle=CFcBaseObject::s_ObjectMng.CreateHandle(pInstance);

Handle.nDebugRef++;

		pInstance->SetHandle(Handle);


		return Handle;
	}


protected:
	int m_nEngineIndex;
	CCrossVector m_Cross;
	D3DXMATRIX m_ScaleMat;
	bool m_bEnable;
	bool m_bDelete;
	bool m_bUpdateObject;
	bool m_bCatchedHero;					// For catch debugging

	CSmartPtr<CFcBaseObject> m_Handle;
	DWORD m_dwTickInterval;
	int	m_nRtti;
	static CSmartPtr<CFcBaseObject> s_CameraHandle[MAX_PLAYER_COUNT];

public:
	enum ObjectType { 
		NORMAL,
		SKYBOX,
		WATER,
		WATER_BOME,
	};

public:
	int	GetRtti() { return m_nRtti;	};
	void SetRtti( int p_nRtti ) { m_nRtti = p_nRtti;};
	static CSmartPtrMng<CFcBaseObject> s_ObjectMng;
	static std::vector< CFcBaseObject * > s_ProcessList;
	static std::vector< CFcBaseObject * > s_DeleteList;


public:
	int Initialize(int nSkinIndex, int nShadowMeshIndex = -1, ObjectType type = NORMAL );

	static bool IsValid( CSmartPtr< CFcBaseObject > Handle );
	bool IsEnable() { return m_bEnable; }
	bool IsDelete() { return m_bDelete; }
	void Delete() 
	{ 
		m_bDelete=true; 

		if( IsCatchedHero() )
		{
			// TODO : Link된 상태로 Delete되는 버그 잡기위해 삽입 => 버그 수정후 삭제!!
			_DEBUGBREAK;
			DebugString( "Catched unit delete! %d\n", m_nEngineIndex );
			BsAssert( 0 && "Catched unit delete!" );
		}
		s_DeleteList.push_back( this );
	}

	void SetCatchedHero( bool bFlag )		{ m_bCatchedHero = bFlag; }
	bool IsCatchedHero()					{ return m_bCatchedHero; }

	void SetHandle(CSmartPtr<CFcBaseObject> Handle) { m_Handle=Handle; }
	BaseObjHandle GetHandle() { return m_Handle; }
	void SetTickInterval( DWORD dwTickInterval ) { m_dwTickInterval = dwTickInterval; }
	DWORD GetTickInterval() { return m_dwTickInterval; }

	void SetUpdateObject( bool bEnable )	{ m_bUpdateObject = bEnable; }

	void RotateYaw( int nAngle ) { m_Cross.RotateYaw( nAngle ); }

	// Virtual Funcition
	virtual void Enable(bool bEnable);
	virtual void Process() {}
	virtual void PostProcess() {}
	virtual bool Render();
	virtual void ProcessKeyEvent(int nKeyCode, KEY_EVENT_PARAM *pKeyParam);
	virtual void CalculateMoveDir(KEY_EVENT_PARAM *pParam, int nCamIndex);
	void AddProcessList();

	// Debug 용 CustomRender
	virtual void DebugRender() {}
	virtual void ShowParts( bool bShow ) {}

	virtual void SetCrossVector( CCrossVector *pCross ) { m_Cross = *pCross; }
	CCrossVector *GetCrossVector() { return &m_Cross; }
	D3DXVECTOR3 GetPos() { return m_Cross.m_PosVector; }
	int GetEngineIndex() { return m_nEngineIndex; }
	void SetEngineIndex( int nIndex ) { m_nEngineIndex = nIndex; }
	AABB *GetBoundingBox() { return (AABB *)g_BsKernel.SendMessage(m_nEngineIndex, BS_GET_BOUNDING_BOX); }

	static void SetCameraObjectHandle(int nPlayerIndex, CSmartPtr<CFcBaseObject> Handle) { s_CameraHandle[nPlayerIndex]=Handle; }
	static CSmartPtr<CFcBaseObject> GetCameraObjectHandle(int nPlayerIndex) { return s_CameraHandle[nPlayerIndex]; }

	static void ProcessObjects();
	static void PostProcessObjects();
	static void RenderObjects();
	static void ReleaseObjects();
	static CSmartPtr<CFcBaseObject> GetFcObjectHandle(int nIndex);
	static void DebugRenderObjects();
	static void ShowAllObjectParts( bool bShow );
};