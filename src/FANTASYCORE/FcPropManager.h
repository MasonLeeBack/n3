#pragma once

#include "BsKernel.h"
#include "RangeScan.h"
#include "FcProp.h"


#define MAX_BILLBOARDTYPE	4

class NavigationMesh;
struct NAVEVENTINFO;
struct PROP_INFO;
struct SPRITE_DATA;

struct GRASSBILLBOARDINFO;

struct STATICPROPDATA
{
	int nClippingType;
	int nStaticPropIndex;
};



// ���ӿ� ������ �� �ִ� �������� CFcProp���� �����ϴ� ���� �ƴ϶� ���⿡ �׳� �߰��Ѵ�.

class PROPZONEDATA
{
public:
	PROPZONEDATA()
	{
		bIsProp = false;
	}

	std::vector<CFcProp*> vecProps[3];		// Small, Normal, Giant
	std::vector<int> vecStaticProps[3];		// Small, Normal, Giant
	Box3 Box;
	bool bIsProp;
};



class PropZoneManager
{
public:
	PropZoneManager();
	~PropZoneManager();

	bool Initialize( int nMapXSize, int nMapYSize, int nPropZoneSize );

	void SetZoneData( int nIndex, Box3* pBox );

	// �ȿ��� ī�޶� ��� ����
	void GetVisibleProp( std::vector<CFcProp*>& vecProps, std::vector<int>& vecStaticProps );
	void AddProp( CFcProp* pProp );
	void AddStaticProp( int nObjIndex, int nIndex, int nClippingType );
	void RemoveProp( CFcProp* pProp );
//	void CaculateBoundaryBox();		// AddProp �� �� ���� ���

	void GetNearStaticProps( float fX, float fY, float fRadius, std::vector<int>& vecStaticNearProps );

protected:
	int m_nZoneSize;
	int m_nXSize, m_nYSize;
	PROPZONEDATA* m_pGroups;
};






struct PROPTYPEDATA
{
	PROPTYPEDATA() {
		nID = nSkinIndex = nCollisionMeshIndex = nAniIndex = nDynmicSkinIndex = -1;
		cName[0] = NULL;
		cFXName[0][0] = NULL;
		cFXName[1][0] = NULL;
		nClippingType = -1;
		bCrush = 0;
		bCollosionMesh = bClimb = bShadowReceiver = bDynamic = false; //bUseShadowMap = false;
		nShadowType = -1;
		nAttrLeft = nAttrTop = 0;
		nAttrXSize = nAttrYSize = 0;
		pAttr = NULL;
		bLoad = false;
		nDebugClimbMeshIndex = -1;
		nSndType = 0;
	}

	int nID;
	int nSkinIndex;
	int nCollisionMeshIndex;		// �浹�޽� �ε���
	int nAniIndex;					// �ִϸ��̼� �ε���
	char cName[32];
	char cFXName[2][32];
	short nClippingType;			// Clipping type
	byte bCrush;					// �μ����� �༮�ΰ�
	bool bCollosionMesh;			// �浹 �Ǵ� �ö� �� �ִ� �޽�
	bool bClimb;					// �ö�Ż �� �ִ� �༮	(true�� �� ������ ���� ���� �� ����)
	bool bShadowReceiver;			// �׸��� ���ù��ΰ�

	int	 nSndType;					// ���� Type;
	int  nBrokenType;				// �μ����� Type;

	// �߰�
	int nDynmicSkinIndex;			// ���� ������Ʈ�� �޽� �ε���
	bool	bDynamic;				// Dynamic Prop. üũ�Ǹ� _dy.bm�� ���� ������Ʈ�� �޽��� �ε��ؾ� ��.
	int nType;						// Prop type
	int nLinkID;					// ������ ��� ������ ���� �μ��� �� ID�� ������ ����

	bool bAlphaBlocking;				// �÷��̾�� ī�޶� ���̿� ���� �� ���ķ� ó���ϴ���

	int nCrushParticleIndex;
	// bool bUseShadowMap;					// 
	int	nShadowType; 
	int nCrushLevel;					// �μ��� ����.

	int nAttrLeft, nAttrTop;
	int nAttrXSize, nAttrYSize;
	BYTE* pAttr;

	PropHeightMap HeightMapInfo;

	bool bLoad;

	int nDebugClimbMeshIndex;
};




struct RS_Intf_Prop 
{
	typedef CFcProp* ID;
	typedef float  R;

	static R GetX( ID data )	{ return data->GetX(); }
	static R GetY( ID data )	{ return data->GetZ(); }
	static bool Valid( ID data ) { return true; }
};

struct UpdatePropObject
{
	UpdatePropObject() { }
	bool operator() ( CFcProp *pProp ) {
		pProp->UpdateObject();
		return true;
	}
};

struct BoundaryNode
{
	float fFront;
	float fBack;
	int nStartIndex;
	int nNodeCount;
};

class AABBTree
{
public:
	AABBTree();
	virtual ~AABBTree();
	
	void Clear();
	void BuildTree();
	bool GetNearTriangleList( float fX, float fY, float fRadius, int *pTriList, int &nCount , bool bRender = false );
	bool AddPosCheck( D3DXVECTOR3 Pos, float fHeight, float &fAddX, float &fAddY );
	bool GetHeightEx( float fX, float fY, float *pHeight , bool bRender = false );
	int AddCollisionInfo( const CollisionTriangleInfo &Info );
	void EraseCollisionInfo( int nStartIndex, int nCount );

	struct SortByPosX
	{
		CollisionTriangleInfo *m_pInfo;
		SortByPosX( CollisionTriangleInfo *pInfo ) : m_pInfo( pInfo ) {}
		__forceinline const bool operator() ( const int &lhs, const int &rhs )
		{
			return m_pInfo[ lhs ].fCenterX < m_pInfo[ rhs ].fCenterX ;
		}
	};

	struct SortByPosZ
	{
		CollisionTriangleInfo *m_pInfo;
		SortByPosZ( CollisionTriangleInfo *pInfo ) : m_pInfo( pInfo ) {} 
		__forceinline const bool operator() ( const int &lhs, const int &rhs )
		{
			return m_pInfo[ lhs ].fCenterZ < m_pInfo[ rhs ].fCenterZ ;
		}
	};

protected:
	std::vector< CollisionTriangleInfo > m_CollisionList;
	BoundaryNode *m_pAABBTree;
	int *m_pTriangleList;
	int m_nNodeDepth;
	int *m_pSearchList[ DOUBLE_BUFFERING ][ 2 ];
};


class CFcPropManager
{
public:
	CFcPropManager();
	~CFcPropManager();

	bool Initialize( float fWidth, float fHeight, int nPropZoneSize );

	bool LoadTypeDat( char* pFileName, CBSMapCore* pMapInfo, OUT VectorFxStr* p_pVecFxStr = NULL );
	bool LoadBSAttr( char* pFileName );
	void CreateProps( std::vector<PROP_INFO>& vecTroops );
	CFcProp*	CreateProp(PROP_INFO *pPropInfo, int nDatIndex);
	void GetActiveProps( D3DXVECTOR3& Pos, float rRadius, std::vector<CFcProp*>& vecProps );
	void GetActiveProps( float fSX, float fSY, float fEX, float fEY, std::vector<CFcProp*>& vecProps );

	void UpdateObjects();

	PROPTYPEDATA* GetPropTypeData_( int index );
	PROPTYPEDATA* GetPropTypeDataByID( int nID );

	void DebugCollsionPropRender();

	void Process();

	CFcProp* GetProp( char* pName );
	void SetEventNavInProps( NavigationMesh* pNavigationMesh, NAVEVENTINFO* pNavEventInfo, int nNumNavEventInfo );

	void SetBuildingAttrAtCrossAttr( int nXSize, int nYSize, BYTE* pPropAttr,
		int nAttrX, int nAttrY, int nRot, float fXRate, float fZRate, D3DXMATRIX* pMat,
		int nAttrBufXSize, int nAttrBufYSize, BYTE* pAttr, BYTE *pAttrEx, bool bSet, int nAttrType );

	int GetActivePropCount()				{ return m_vecActiveProps.size(); }
	CFcProp* GetActiveProp( int nIndex )	{ return m_vecActiveProps[nIndex]; }

	void InitBillboardProps();

	void GetObjectListInRange( D3DXVECTOR3 *pPosition, float fRadius, std::vector<CFcProp *> &Objects );
	void RemoveProp( CFcProp *pProp );

	void GetItemList( std::vector<CFcProp*>& vecItems );

	void SetAlphaBlocking( bool bEnable )	{ m_bAlphaBlocking = bEnable; }

	void InitZoneData( int nXSize, int nYSize, int nPropZoneSize );
	void SetZoneData( int nIndex, Box3* pBox );

	void DeletePointLightRange( float fX, float fZ, float fRange);

	std::vector<CFcProp*>& GetDynamicProps() { return m_vecMovableProps;}

	AABBTree& GetAABBTree() { return m_AABBCollisionTree; }

protected:
	void GetVisibleProps(D3DXVECTOR2* pPos, float fRange, std::vector<CFcProp*>& vecProps );
	void UpdateVisibleProps();
	int SearchNearNavIndex( D3DXVECTOR2* Pos, NavigationMesh* pNavigationMesh, NAVEVENTINFO* pNavEventInfo, int nNumNavEventInfo );
	bool IsActiveProp( CFcProp* pProp );
	bool GeneratePropHeightMapFile( char* pSkinFileName, char* pFileName, int* pDebugMeshIndex = NULL );

	bool LoadBillboardGrassInfo( char* pFileName, int* pInfoNum, GRASSBILLBOARDINFO* pInfo );

	void InitLightScatteringFX( bool bPholya = false );

	void GetNearAlphaBlockingProps( float fX, float fY, float fRadius, std::vector<int>& vecNearAlphaProps );

	float GetTwinkleLightIntensity( int nTick, int nSeed );	

protected:
	int m_nPropNum;
	CFcProp** m_ppProps;
	std::vector<CFcProp*> m_vecActiveProps;		// �̸��� �ִ� �༮�鸸 ���
	std::vector<CFcProp*> m_vecMovableProps;
	std::vector<PROPTYPEDATA> m_vecTypeData;

	RangeScan< RS_Intf_Prop >*	m_pPropScanner;
	RangeScan< RS_Intf_Prop >*	m_pActivePropScanner;

	PropZoneManager* m_pZoneManager;	

	std::vector<STATICPROPDATA> m_vecStaticObjs;

	std::vector<int> m_vecFX;

	float		m_fWidth;
	float		m_fHeight;

	std::vector<D3DXVECTOR3> m_VecBilboardData[MAX_BILLBOARDTYPE];
	std::vector<D3DXVECTOR2> m_VecBilboardScaleData[MAX_BILLBOARDTYPE];

	bool m_bAlphaBlocking;

	int	m_nLightScatteringID;
	std::vector< std::pair<int, float> > m_vecPointLight;

	int					m_nMagmaTwistA;
	int					m_nMagmaTwistB;
	int					m_nMagmaTwistC;
	int					m_nLavaDistort;
	std::vector<int>	m_vecRandomLava;

	AABBTree		m_AABBCollisionTree;

	void MagmaFxClear();
	
};
