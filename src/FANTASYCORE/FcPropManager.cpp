#include "stdafx.h"
#include "FcGlobal.h"
#include "BsFileManager.h"
#include "FcPropManager.h"
#include "BsKernel.h"
#include "Util\\Parser.h"
#include "CrossVector.h"
#include "FcCommon.h"
#include "FcBaseObject.h"
#include "FcCameraObject.h"
#include <fstream>
#include "navigationmesh.h"
#include "FcWorld.h"
#include "BsSinTable.h"
#include "FcUtil.h"
#include "DebugUtil.h"
#include "BsBillboardMgr.h"
#include "BsFileIO.h"
#include "IntBox3Frustum.h"
#include "bstreamext.h"
#include "data/FXList.h"
#ifdef	_XBOX
#include ".\\MS\\CClipTest.h"
#include "IntLin3Box3.h"
#endif
#include <io.h>
#include "FcFXManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


#define START_PROP_SKIN_INDEX		500

#define PROP_CLIPPING_RADIUS		50000

#define MAX_BILLBOARD_GRASS	17

#define EMPTY_SKIN_NAME	"empty.SKIN"

enum FXPROP_TYPE 
{
	FXPROP_LIGHTSCATTERING = 0,	// 빛 스며들어오는 프랍 효과
	FXPROP_POINTLIGHT = 1,			// 포인트 라이트
	FXPROP_LAVA = 2,
	FXPROP_LAVADISTORT = 3, 
	FXPROP_MAGMATWISTA = 4,
	FXPROP_MAGMATWISTB = 5,
	FXPROP_MAGMATWISTC = 6,
	FXPROP_LIGHTSCATTERING_PHOLYA = 7,
};

PropZoneManager::PropZoneManager()
{
	m_nXSize = m_nYSize = 0;
	m_pGroups = NULL;	
}

PropZoneManager::~PropZoneManager()
{
	SAFE_DELETEA( m_pGroups );
}

bool PropZoneManager::Initialize( int nMapXSize, int nMapYSize, int nPropZoneSize )
{
	m_nZoneSize = nPropZoneSize;
	m_nXSize = nMapXSize;
	m_nYSize = nMapYSize;

	BsAssert( m_pGroups == NULL );
	m_pGroups = new PROPZONEDATA [ m_nXSize * m_nYSize ];
	return true;
}

void PropZoneManager::SetZoneData( int nIndex, Box3* pBox )
{
	BsAssert( nIndex < m_nXSize * m_nYSize);
	if( pBox )
	{
		m_pGroups[nIndex].Box = *pBox;
		m_pGroups[nIndex].bIsProp = true;
	}
	else
	{
		m_pGroups[nIndex].bIsProp = false;
	}
}

// 안에서 카메라 계산 해줌
void PropZoneManager::GetVisibleProp( std::vector<CFcProp*>& vecProps, std::vector<int>& vecStaticProps )
{
	// camera
#ifdef	_XBOX
	int			iCameraHandle;
	CBsCamera*	pCBsCamera;
	float		fPers;
	float		fAspect;
	float		fNear;
	float		fFar0, fFar1, fFar2;
	XMMATRIX	mV;
	XMVECTOR	vPlane[ CCLIP_TEST_PLANE_MAX ];
	XMVECTOR	vEye;

	iCameraHandle	= g_BsKernel.GetCameraHandle( 0 );
	pCBsCamera		= g_BsKernel.GetCamera( iCameraHandle );
	fPers			= pCBsCamera->GetFOVByProcess();
	fAspect			= pCBsCamera->GetAspect();
	fNear			= pCBsCamera->GetNearZ();
	fFar0			= pCBsCamera->GetFogEnd()*0.5f;
	fFar1			= pCBsCamera->GetFogEnd();
	fFar2			= pCBsCamera->GetFarZ();

	CSmartPtr<CFcCameraObject> hCamera = CFcBaseObject::GetCameraObjectHandle( 0 );
	CCrossVector* pCross = hCamera->GetCrossVector();
	mV				= _XMMATRIX( ( FLOAT* )	pCross->ConvertViewMatrix() );

	// multi-thread preparation version
	CClipTest::ComputeClipPlane( fPers,
								 fAspect,
								 fNear,
								 fFar0,
								 fFar1,
								 fFar2,
								 mV,
								 vPlane,
								 &vEye );
	CClipTest::SetClipPlane( vPlane );
#else
	const Frustum	& frustumType0 = ((CBsCamera*)g_BsKernel.GetEngineObjectPtr(g_BsKernel.GetCameraHandle(0)))->GetFrustum();
	const Frustum	& frustumType1 = ((CBsCamera*)g_BsKernel.GetEngineObjectPtr(g_BsKernel.GetCameraHandle(0)))->GetFrustumForBig();
	const Frustum	& frustumType2 = ((CBsCamera*)g_BsKernel.GetEngineObjectPtr(g_BsKernel.GetCameraHandle(0)))->GetFrustumForGiant();
#endif

	vecProps.clear();

	for( int i=0; i<m_nYSize; i++ )
	{
		for( int j=0; j<m_nXSize; j++ )
		{
			int nIndex = j + i * m_nXSize;
			if( m_pGroups[ nIndex ].bIsProp == false )
				continue;
			Box3* pBox = &( m_pGroups[ nIndex ].Box );
			BOOL bIsInFrustum[3] = { FALSE, FALSE, FALSE };
#ifdef	_XBOX
			// clip test
			// This works good bat only wroks on XBOX.
			float	fRadiusXRadius	= pBox->E[0]*pBox->E[0] + pBox->E[1]*pBox->E[1] + pBox->E[2]*pBox->E[2];

			// FarZ (TestFar2) is too many UpdateObjectCount, so i change it to FogEnd (TestFar1)
			bIsInFrustum[2] = CClipTest::TestFar2(pBox->C, fRadiusXRadius);
			if(bIsInFrustum[2]) {
				bIsInFrustum[1] = CClipTest::TestFar1(pBox->C, fRadiusXRadius);
				if(bIsInFrustum[1]) {
					bIsInFrustum[0] = CClipTest::Test(pBox->C, fRadiusXRadius);
				}
			}
#else
			// bug
			bIsInFrustum[2] = TestIntersection( *pBox, frustumType2, 0.f );
			if(bIsInFrustum[2]) {
				bIsInFrustum[1] = TestIntersection( *pBox, frustumType1, 0.f );
				if(bIsInFrustum[1]) {
					bIsInFrustum[0] = TestIntersection( *pBox, frustumType0, 0.f );
				}

			}
#endif
			for(int nType = 0 ; nType<3 ; ++nType ) {
				if(bIsInFrustum[nType]) {
					int nCnt = m_pGroups[nIndex].vecProps[nType].size();
					for( int k=0; k<nCnt; k++ ) {
						vecProps.push_back( m_pGroups[ nIndex ].vecProps[nType][k] );
					}
					nCnt = m_pGroups[ nIndex ].vecStaticProps[nType].size();
					for( int k=0; k<nCnt; k++ ) {
						vecStaticProps.push_back( m_pGroups[ nIndex ].vecStaticProps[nType][k] );
					}
				}
			}
		}
	}
}

void PropZoneManager::AddProp( CFcProp* pProp )
{
	D3DXVECTOR2 Pos = pProp->GetPosV2();

	int nX = (int)Pos.x / m_nZoneSize ;
	int nY = (int)Pos.y / m_nZoneSize ;

	if( nX < 0 || nX >= m_nXSize || nY < 0 || nY >= m_nYSize )
	{
		DebugString( "Prop found invalid area %f, %f\n", Pos.x, Pos.y );
		return;
	}

	PROPTYPEDATA* pData = g_FcWorld.GetPropTypeData_( pProp->GetTypeDataIndex() );
	int nClippingType = pData->nClippingType;
	if( nClippingType < 0 )
		nClippingType = 0;
	else if( nClippingType > 2 )
		nClippingType = 2;

	m_pGroups[ nY * m_nXSize + nX ].vecProps[nClippingType].push_back( pProp );
	int nEngineIndex = pProp->GetEngineIndex();
	BsAssert( nClippingType >= 0 && nClippingType < 3 );
	g_BsKernel.SendMessage(nEngineIndex, BS_SET_CLIPPING_TYPE, nClippingType);
}

void PropZoneManager::AddStaticProp( int nObjIndex, int nIndex, int nClippingType )
{
	const D3DXMATRIX* pMat = g_BsKernel.GetEngineObjectPtr( nObjIndex )->GetObjectMatrix();
	D3DXVECTOR2 Pos = D3DXVECTOR2( pMat->_41, pMat->_43 );
	int nX = (int)Pos.x / m_nZoneSize ;
	int nY = (int)Pos.y / m_nZoneSize ;
	if( nX < 0 || nX >= m_nXSize || nY < 0 || nY >= m_nYSize )
	{
		DebugString( "Prop found invalid area %f, %f\n", Pos.x, Pos.y );
		return;
	}
	BsAssert( nClippingType >= 0 && nClippingType < 3 );
	m_pGroups[ nY * m_nXSize + nX ].vecStaticProps[nClippingType].push_back( nIndex );
	g_BsKernel.SendMessage(nObjIndex, BS_SET_CLIPPING_TYPE, nClippingType);
}

void PropZoneManager::RemoveProp( CFcProp* pProp )
{
	D3DXVECTOR2 Pos = pProp->GetPosV2();

	int nX = (int)Pos.x / m_nZoneSize ;
	int nY = (int)Pos.y / m_nZoneSize ;

	if( nX < 0 || nX >= m_nXSize || nY < 0 || nY >= m_nYSize )
	{
		DebugString( "Prop found invalid area %f, %f\n", Pos.x, Pos.y );
		return;
	}

	PROPTYPEDATA* pData = g_FcWorld.GetPropTypeData_( pProp->GetTypeDataIndex() );
	int nClipIndex = pData->nClippingType;
	BsAssert( nClipIndex >=0 && nClipIndex < 3 );


// [PREFIX:beginmodify] 2006/2/16 junyash PS#5210,5211 reports Local declaration of 'nCnt' & 'nIndex' hides declaration of the same name in an outer scope
	// modify scope and variable name
	bool bDelete = false;
	{
		int nIndex = nY * m_nXSize + nX;
		int nCnt = m_pGroups[ nIndex ].vecProps[nClipIndex].size();
		for( int i=0; i<nCnt; i++ )
		{
			CFcProp* pCurProp = m_pGroups[ nIndex ].vecProps[nClipIndex][i];
			if( pCurProp == pProp )
			{
				m_pGroups[ nIndex ].vecProps[nClipIndex].erase( m_pGroups[ nIndex ].vecProps[nClipIndex].begin() + i );
				bDelete = true;
				break;
			}
		}
	}

	if( bDelete == false )
	{
		int nDepth = 1;
		while( 1 )
		{
			int nDepthCnt = nDepth * 2 - 1 + 2;
			int nStartX = nX - nDepth;
			int nStartY = nY - nDepth;

			bool m_bDelete2 = false;
			for( int i=0; i<nDepthCnt; i++ )
			{
				for( int j=0; j<nDepthCnt; j++ )
				{
					if( nStartX + j < 0 || nStartX + j >= m_nXSize || nStartY + i < 0 || nStartY + i >= m_nYSize )
					{
						continue;
					}
					int nIndex = ( nStartY + i ) * m_nXSize + ( nStartX + j );
					int nCnt = m_pGroups[ nIndex ].vecProps[nClipIndex].size();
					for( int k=0; k<nCnt; k++ )
					{
						CFcProp* pCurProp = m_pGroups[ nIndex ].vecProps[nClipIndex][k];
						if( pCurProp == pProp )
						{
							m_pGroups[ nIndex ].vecProps[nClipIndex].erase( m_pGroups[ nIndex ].vecProps[nClipIndex].begin() + k );
							m_bDelete2 = true;
							break;
						}
					}
					if( m_bDelete2 )
						break;
				}
				if( m_bDelete2 )
					break;
			}
			if( m_bDelete2 )
				break;

			++nDepth;

			if( nDepth > 100 )
			{
				break;
			}
		}
	}
// [PREFIX:endmodify] junyash
}

void PropZoneManager::GetNearStaticProps( float fX, float fY, float fRadius, std::vector<int>& vecStaticNearProps )
{
	int nCX = (int)fX / m_nZoneSize;
	int nCY = (int)fY / m_nZoneSize;

	int nSX = nCX - (int)fRadius / m_nZoneSize - 1;
	int nSY = nCY - (int)fRadius / m_nZoneSize - 1;
	int nCount = ((int)fRadius / m_nZoneSize + 1) * 2;

	for( int i=nSX; i<=nSX+nCount; i++ )
	{
		if( i < 0 || i >= m_nXSize )
			continue;

		for( int j=nSY; j<nSY+nCount; j++ )
		{
			if( j < 0 || j >= m_nYSize )
				continue;

			for( int k=0; k<3; k++ )
			{
				int nPropCnt = m_pGroups[ i + j * m_nXSize ].vecStaticProps[k].size();
				for( int l=0; l<nPropCnt; l++ )
				{
					vecStaticNearProps.push_back( m_pGroups[ i + j * m_nXSize ].vecStaticProps[k][l] );
				}
			}
		}
	}
}


/*
void PropZoneManager::CaculateBoundaryBox()
{
int nGroupCnt = m_nXSize * m_nYSize;
for( int i=0; i<nGroupCnt; i++ )
{
AABB Box;
int nCnt = m_pGroups[i].vecProps.size();
if( nCnt >= 1 )
{
CFcProp* pProp = m_pGroups[i].vecProps[0];
Box = *(pProp->GetBoundingBox());

D3DXMATRIX Mat = pProp->GetMat();
D3DXVECTOR3 NewPos1, NewPos2;
D3DXVECTOR3* pPos = (D3DXVECTOR3*)&Box.Vmax;
D3DXVec3TransformCoord( &NewPos1, pPos, &Mat );
pPos = (D3DXVECTOR3*)&Box.Vmin;
D3DXVec3TransformCoord( &NewPos2, pPos, &Mat );

if( NewPos1.x > NewPos2.x ) { Box.Vmax.x = NewPos1.x;	Box.Vmin.x = NewPos2.x; }
else						{ Box.Vmax.x = NewPos2.x;	Box.Vmin.x = NewPos1.x; }
if( NewPos1.y > NewPos2.y ) { Box.Vmax.y = NewPos1.y;	Box.Vmin.y = NewPos2.y; }
else						{ Box.Vmax.y = NewPos2.y;	Box.Vmin.y = NewPos1.y; }
if( NewPos1.z > NewPos2.z ) { Box.Vmax.z = NewPos1.z;	Box.Vmin.z = NewPos2.z; }
else						{ Box.Vmax.z = NewPos2.z;	Box.Vmin.z = NewPos1.z; }

m_pGroups[i].bIsProp = true;
}
else
{
m_pGroups[i].bIsProp = false;
continue;
}

for( int j=1; j<nCnt; j++ )
{
CFcProp* pProp = m_pGroups[i].vecProps[j];
AABB CurBox = *pProp->GetBoundingBox();

D3DXMATRIX Mat = pProp->GetMat();
D3DXVECTOR3 NewPos1, NewPos2;
D3DXVECTOR3* pPos = (D3DXVECTOR3*)&CurBox.Vmax;
D3DXVec3TransformCoord( &NewPos1, pPos, &Mat );
pPos = (D3DXVECTOR3*)&CurBox.Vmin;
D3DXVec3TransformCoord( &NewPos2, pPos, &Mat );

if( NewPos1.x > NewPos2.x ) { CurBox.Vmax.x = NewPos1.x;	CurBox.Vmin.x = NewPos2.x; }
else						{ CurBox.Vmax.x = NewPos2.x;	CurBox.Vmin.x = NewPos1.x; }
if( NewPos1.y > NewPos2.y ) { CurBox.Vmax.y = NewPos1.y;	CurBox.Vmin.y = NewPos2.y; }
else						{ CurBox.Vmax.y = NewPos2.y;	CurBox.Vmin.y = NewPos1.y; }
if( NewPos1.z > NewPos2.z ) { CurBox.Vmax.z = NewPos1.z;	CurBox.Vmin.z = NewPos2.z; }
else						{ CurBox.Vmax.z = NewPos2.z;	CurBox.Vmin.z = NewPos1.z; }

if( CurBox.Vmax.x > Box.Vmax.x )				Box.Vmax.x = CurBox.Vmax.x;
if( CurBox.Vmax.y > Box.Vmax.y )				Box.Vmax.y = CurBox.Vmax.y;
if( CurBox.Vmax.z > Box.Vmax.z )				Box.Vmax.z = CurBox.Vmax.z;
if( CurBox.Vmin.x < Box.Vmin.x )				Box.Vmin.x = CurBox.Vmin.x;
if( CurBox.Vmin.y < Box.Vmin.y )				Box.Vmin.y = CurBox.Vmin.y;
if( CurBox.Vmin.z < Box.Vmin.z )				Box.Vmin.z = CurBox.Vmin.z;
}


nCnt = m_pGroups[i].vecStaticProps.size();
if( m_pGroups[i].bIsProp == false )
{
if( nCnt >= 1 )
{
int nObjectIndex = m_pGroups[i].vecStaticProps[0];
STATICPROPDATA* pStaticData = m_pStaticManager->GetData( nObjectIndex );
Box = *((AABB *)g_BsKernel.SendMessage( pStaticData->nStaticPropIndex, BS_GET_BOUNDING_BOX ));
const D3DXMATRIX* pMat = g_BsKernel.GetEngineObjectPtr( pStaticData->nStaticPropIndex )->GetObjectMatrix();

D3DXVECTOR3 NewPos1, NewPos2;
D3DXVECTOR3* pPos = (D3DXVECTOR3*)&Box.Vmax;
D3DXVec3TransformCoord( &NewPos1, pPos, pMat );
pPos = (D3DXVECTOR3*)&Box.Vmin;
D3DXVec3TransformCoord( &NewPos2, pPos, pMat );

if( NewPos1.x > NewPos2.x ) { Box.Vmax.x = NewPos1.x;	Box.Vmin.x = NewPos2.x; }
else						{ Box.Vmax.x = NewPos2.x;	Box.Vmin.x = NewPos1.x; }
if( NewPos1.y > NewPos2.y ) { Box.Vmax.y = NewPos1.y;	Box.Vmin.y = NewPos2.y; }
else						{ Box.Vmax.y = NewPos2.y;	Box.Vmin.y = NewPos1.y; }
if( NewPos1.z > NewPos2.z ) { Box.Vmax.z = NewPos1.z;	Box.Vmin.z = NewPos2.z; }
else						{ Box.Vmax.z = NewPos2.z;	Box.Vmin.z = NewPos1.z; }

m_pGroups[i].bIsProp = true;
}
}

for( int j=1; j<nCnt; j++ )
{
int nObjectIndex = m_pGroups[i].vecStaticProps[j];
STATICPROPDATA* pStaticData = m_pStaticManager->GetData( nObjectIndex );
AABB CurBox = *((AABB *)g_BsKernel.SendMessage( pStaticData->nStaticPropIndex, BS_GET_BOUNDING_BOX ));
const D3DXMATRIX* pMat = g_BsKernel.GetEngineObjectPtr( pStaticData->nStaticPropIndex )->GetObjectMatrix();
D3DXVECTOR3 NewPos1, NewPos2;
D3DXVECTOR3* pPos = (D3DXVECTOR3*)&CurBox.Vmax;
D3DXVec3TransformCoord( &NewPos1, pPos, pMat );
pPos = (D3DXVECTOR3*)&CurBox.Vmin;
D3DXVec3TransformCoord( &NewPos2, pPos, pMat );

if( NewPos1.x > NewPos2.x ) { CurBox.Vmax.x = NewPos1.x;	CurBox.Vmin.x = NewPos2.x; }
else						{ CurBox.Vmax.x = NewPos2.x;	CurBox.Vmin.x = NewPos1.x; }
if( NewPos1.y > NewPos2.y ) { CurBox.Vmax.y = NewPos1.y;	CurBox.Vmin.y = NewPos2.y; }
else						{ CurBox.Vmax.y = NewPos2.y;	CurBox.Vmin.y = NewPos1.y; }
if( NewPos1.z > NewPos2.z ) { CurBox.Vmax.z = NewPos1.z;	CurBox.Vmin.z = NewPos2.z; }
else						{ CurBox.Vmax.z = NewPos2.z;	CurBox.Vmin.z = NewPos1.z; }

if( CurBox.Vmax.x > Box.Vmax.x )				Box.Vmax.x = CurBox.Vmax.x;
if( CurBox.Vmax.y > Box.Vmax.y )				Box.Vmax.y = CurBox.Vmax.y;
if( CurBox.Vmax.z > Box.Vmax.z )				Box.Vmax.z = CurBox.Vmax.z;
if( CurBox.Vmin.x < Box.Vmin.x )				Box.Vmin.x = CurBox.Vmin.x;
if( CurBox.Vmin.y < Box.Vmin.y )				Box.Vmin.y = CurBox.Vmin.y;
if( CurBox.Vmin.z < Box.Vmin.z )				Box.Vmin.z = CurBox.Vmin.z;
}

m_pGroups[i].Box.C = ( Box.Vmax + Box.Vmin ) * 0.5f;

m_pGroups[i].Box.A[0]=BSVECTOR(1.f,0.f,0.f);
m_pGroups[i].Box.A[1]=BSVECTOR(0.f,1.f,0.f);
m_pGroups[i].Box.A[2]=BSVECTOR(0.f,0.f,1.f);

m_pGroups[i].Box.E[0]=fabsf((Box.Vmax.x - Box.Vmin.x)*0.5f);
m_pGroups[i].Box.E[1]=fabsf((Box.Vmax.y - Box.Vmin.y)*0.5f);
m_pGroups[i].Box.E[2]=fabsf((Box.Vmax.z - Box.Vmin.z)*0.5f);

m_pGroups[i].Box.compute_vertices();
}
}
*/


AABBTree::AABBTree()
{
	m_pAABBTree = NULL;
	m_pTriangleList = NULL;
	m_nNodeDepth = 0;

	for( int i = 0; i < DOUBLE_BUFFERING; i++) 
	for( int j = 0; j < DOUBLE_BUFFERING; j++) {		
		m_pSearchList[ i ][ j ] = NULL;
	}
}

AABBTree::~AABBTree()
{
	Clear();
}

void AABBTree::Clear()
{
	SAFE_DELETEA( m_pAABBTree );
	SAFE_DELETEA( m_pTriangleList );

	for( int i = 0; i < DOUBLE_BUFFERING; i++) 
	for( int j = 0; j < DOUBLE_BUFFERING; j++) {
		SAFE_DELETEA( m_pSearchList[ i ][ j ] );
	}
}

int AABBTree::AddCollisionInfo( const CollisionTriangleInfo &Info )
{
    m_CollisionList.push_back( Info );
	return m_CollisionList.size();
}

void AABBTree::EraseCollisionInfo( int nStartIndex, int nCount ) 
{
	CollisionTriangleInfo *pInfo = &m_CollisionList[ nStartIndex ];
	for( int i = 0; i < nCount; i++) {	
		pInfo->fMinX = FLT_MAX;
		pInfo->fMaxX = -FLT_MAX;
		pInfo->fMinZ = FLT_MAX;
		pInfo->fMaxZ = -FLT_MAX;
		pInfo++;
	}
}

void AABBTree::BuildTree()
{
	if( m_CollisionList.empty() ) {
		return ;
	}

	int nSize = m_CollisionList.size();

	m_nNodeDepth = 0;
	int nNodeCount = nSize;
	while( nNodeCount ) {
		nNodeCount >>= 1;
		m_nNodeDepth++;
	}
	nNodeCount = 1 << m_nNodeDepth ;

	m_pAABBTree = new BoundaryNode[ nNodeCount ];	

	int i, j;
	m_pTriangleList = new int[ nSize ];
	for( i = 0; i < nSize; i++ ) {
		m_pTriangleList[i] = i;
	}

	for( i = 0; i < DOUBLE_BUFFERING; i++) 
	for( j = 0; j < DOUBLE_BUFFERING; j++) {
		m_pSearchList[ i ][ j ] = new int [ nNodeCount / 2 ];
	}

	m_pAABBTree[ 1 ].nStartIndex = 0;
	m_pAABBTree[ 1 ].nNodeCount = nSize;
	m_pAABBTree[ 1 ].fFront = -FLT_MAX;
	m_pAABBTree[ 1 ].fBack = FLT_MAX;

	int nAxis = 0;
	int nHalfNodeSize = nNodeCount >> 1;

	SortByPosX sortX( &m_CollisionList.front() );
	SortByPosZ sortZ( &m_CollisionList.front() );

	for( i = 1; i < nNodeCount; i++) {
		BoundaryNode *pCurrentNode = m_pAABBTree + i;

		int *pFirst = m_pTriangleList + pCurrentNode->nStartIndex;
		int *pLast = m_pTriangleList + pCurrentNode->nStartIndex + pCurrentNode->nNodeCount;

		if( nAxis == 0 ) {
			std::sort( pFirst, pLast, sortX );
		}
		else {
			std::sort( pFirst, pLast, sortZ );
		}

		bool bLeafNode = i >= nHalfNodeSize;
		if( !bLeafNode ) {
			BoundaryNode *pChildNode = m_pAABBTree + (i << 1);
			int nHalfSize = pCurrentNode->nNodeCount >> 1;

			pChildNode[0].nStartIndex = pCurrentNode->nStartIndex;
			pChildNode[0].nNodeCount = nHalfSize;
			pChildNode[1].nStartIndex = pCurrentNode->nStartIndex + nHalfSize;
			pChildNode[1].nNodeCount = pCurrentNode->nNodeCount - nHalfSize;

			pChildNode[0].fFront = FLT_MAX;
			pChildNode[0].fBack = -FLT_MAX;
			pChildNode[1].fFront = FLT_MAX;
			pChildNode[1].fBack = -FLT_MAX;

			int *pIndex = m_pTriangleList + pCurrentNode->nStartIndex;

			if( nAxis == 0) {
				for( int j = 0; j < pChildNode[0].nNodeCount; j++) {
					int nIndex = *pIndex++;
					pChildNode[0].fFront = BsMin( pChildNode[0].fFront, m_CollisionList[ nIndex ].fMinX );
					pChildNode[0].fBack = BsMax( pChildNode[0].fBack, m_CollisionList[ nIndex ].fMaxX );					
				}
				for( int j = 0; j < pChildNode[1].nNodeCount; j++) {
					int nIndex = *pIndex++;
					pChildNode[1].fFront = BsMin( pChildNode[1].fFront, m_CollisionList[ nIndex ].fMinX );
					pChildNode[1].fBack = BsMax( pChildNode[1].fBack, m_CollisionList[ nIndex ].fMaxX );					
				}
			}
			else {
				for( int j = 0; j < pChildNode[0].nNodeCount; j++) {
					int nIndex = *pIndex++;
					pChildNode[0].fFront = BsMin( pChildNode[0].fFront, m_CollisionList[ nIndex ].fMinZ );
					pChildNode[0].fBack = BsMax( pChildNode[0].fBack, m_CollisionList[ nIndex ].fMaxZ );					
				}
				for( int j = 0; j < pChildNode[1].nNodeCount; j++) {
					int nIndex = *pIndex++;
					pChildNode[1].fFront = BsMin( pChildNode[1].fFront, m_CollisionList[ nIndex ].fMinZ );
					pChildNode[1].fBack = BsMax( pChildNode[1].fBack, m_CollisionList[ nIndex ].fMaxZ );
				}
			}
			if( (i & ( i + 1 )) == 0 ) {
				nAxis = 1 - nAxis;
			}
		}
	}
}

bool AABBTree::GetNearTriangleList( float fX, float fY, float fRadius, int *pTriList, int &nCount , bool bRender)
{
	if( !m_pAABBTree ) {
		return false;
	}

	int *pCurrentSearchList;
	int *pNextSearchList;

	if( bRender ) {
		pCurrentSearchList = m_pSearchList[ 0 ][ 0 ];
		pNextSearchList = m_pSearchList[ 0 ][ 1 ];
	}
	else {
		pCurrentSearchList = m_pSearchList[ 1 ][ 0 ];
		pNextSearchList = m_pSearchList[ 1 ][ 1 ];
	}

	pCurrentSearchList[ 0 ] = 1;

	int nCurrentSearchCount = 1;
	int nNextSearchCount = 0;

	int nMaxCount = nCount;
	nCount = 0;

	for( int nDepth = 0; nDepth < m_nNodeDepth; nDepth++ ) 
	{
		int i, nSize;
		nNextSearchCount = 0;
		nSize = nCurrentSearchCount;

		if( nSize == 0) {
			return false;
		}
		for( i = 0; i < nSize; i++) {
			int nIndex = pCurrentSearchList[ i ];

			float fPos = ( nDepth & 1) ? fX : fY;

			if( m_pAABBTree[ nIndex ].fFront - fRadius < fPos &&  m_pAABBTree[ nIndex ].fBack + fRadius > fPos )
			{
				bool bLeaf = ( nDepth == m_nNodeDepth - 1);
				if( bLeaf ) {
					int *pStartTriangle = m_pTriangleList + m_pAABBTree[ nIndex ].nStartIndex;
					int nTriCount = m_pAABBTree[ nIndex ].nNodeCount;
					for( int j = 0; j < nTriCount; j++) {
						pTriList[ nCount++ ] = *pStartTriangle++;
						if( nCount >= nMaxCount ) {
							DebugString(" Warning! - GetNearTriangleList()  Max Count Overflow\n");
							return true;
						}
					}
				}
				else {
					pNextSearchList[ nNextSearchCount++ ] = ( nIndex << 1 );
					pNextSearchList[ nNextSearchCount++ ] = ( nIndex << 1 ) + 1;
				}
			}
		}
		std::swap( pCurrentSearchList, pNextSearchList );
		std::swap( nCurrentSearchCount, nNextSearchCount );
	}
	if( nCount == 0 ){
		return false;
	}
	return true;
}

bool AABBTree::AddPosCheck( D3DXVECTOR3 Pos, float fHeight, float &fAddX, float &fAddY)
{	
#ifdef _XBOX
	Pos.y += fHeight;

	const float fUnitRadius = 100.f;
	const float fBorderSize = 80.f;

	D3DXVECTOR3 Dir(fAddX, 0.f, fAddY);
	float fLength = D3DXVec3Length( &Dir );

	if( fLength < 0.01f || fLength > 1000.f ) 
		return false;

	Dir /= fLength;

	int TriangleList[50];
	int i, nCount = 50;	// MaxCount 넣어준다.

	float fFindX = Pos.x + Dir.x * fUnitRadius * 0.5f;
	float fFindZ = Pos.z + Dir.z * fUnitRadius * 0.5f;
	if( !GetNearTriangleList( fFindX, fFindZ, fUnitRadius, TriangleList, nCount) ) {
		return false;
	}

	bool bCollide = false;

	for( i = 0; i < nCount; i++ )
	{
		CollisionTriangleInfo *pInfo = &m_CollisionList[ TriangleList[i] ];

		if( pInfo->fMaxX < pInfo->fMinX ) 
		{
			continue;	// 지워진 삼각형
		}
		float fDist;

		BSVECTOR v0, v1, v2, Original, Direction;

		v0.x = pInfo->p[0].x;
		v0.y = pInfo->p[0].y;
		v0.z = pInfo->p[0].z;

		v1.x = pInfo->p[1].x;
		v1.y = pInfo->p[1].y;
		v1.z = pInfo->p[1].z;

		v2.x = pInfo->p[2].x;
		v2.y = pInfo->p[2].y;
		v2.z = pInfo->p[2].z;

		Original.x = Pos.x;
		Original.y = Pos.y;
		Original.z = Pos.z;

		Direction.x = Dir.x;
		Direction.y = Dir.y;
		Direction.z = Dir.z;

		if( !IntersectRayTriangle( Original, Direction, v0, v1, v2, &fDist) )
			continue;

		D3DXVECTOR3 Normal;
		D3DXVECTOR3 edge1 = pInfo->p[1] - pInfo->p[0];
		D3DXVECTOR3 edge2 = pInfo->p[2] - pInfo->p[1];
		D3DXVec3Cross( &Normal, &edge1, &edge2 );
		if( D3DXVec3Dot( &Normal, &Dir ) > 0.f ) 
			continue;
		D3DXVec3Normalize( &Normal, &Normal );

		D3DXVECTOR3 Dist(fAddX, 0, fAddY);
		float fDot = D3DXVec3Dot(&Dist, &Normal);
		if( fDot > 0.f )  continue;

		float fDistLength = D3DXVec3Length( &Dist );
		D3DXVECTOR3 vDir = ( fDot * Normal );
		Dist -= vDir;
		float fDirLength = D3DXVec3Length( &vDir );
		fDist *= fDirLength / fDistLength;

		if ( fDist - fDirLength < fBorderSize ) {

			Dist += ( ( fBorderSize - fDist ) * Normal );			

			fAddX = Dist.x;
			fAddY = Dist.z;

			D3DXVECTOR3 vAddPos( fAddX, 0, fAddY);
			D3DXVec3Normalize( &Dir, &vAddPos);
			Pos += vAddPos;
			bCollide = true;
		}
	}
	return bCollide;
#else
	return false;
#endif
}

bool AABBTree::GetHeightEx( float fX, float fY, float *pHeight, bool bRender )
{
	int i, nCount;

	int TriangleList[50];
	nCount = 50;
	if( !GetNearTriangleList( fX, fY, 0.f, TriangleList, nCount , bRender) ) {
		return false;
	}

	D3DXVECTOR3 pos(fX, 0, fY);

	for( i = 0; i < nCount; i++ )
	{
		CollisionTriangleInfo *pInfo = &m_CollisionList[ TriangleList[i] ];

		if( pInfo->fMaxX < fX || pInfo->fMinX > fX || pInfo->fMaxZ < fY || pInfo->fMinZ > fY ) continue;

		D3DXVECTOR3 *pTriangle = pInfo->p;

		D3DXVECTOR3 p1 = pTriangle[1] - pTriangle[0];
		D3DXVECTOR3 p2 = pTriangle[2] - pTriangle[1];
		D3DXVECTOR3 p3 = pTriangle[0] - pTriangle[2];

		D3DXVECTOR3 q1 = pTriangle[0] - pos;
		D3DXVECTOR3 q2 = pTriangle[1] - pos;
		D3DXVECTOR3 q3 = pTriangle[2] - pos;

		float fResult1 = p1.x * q1.z - p1.z * q1.x;
		float fResult2 = p2.x * q2.z - p2.z * q2.x;
		float fResult3 = p3.x * q3.z - p3.z * q3.x;

		if( fResult1 * fResult2 > 0.f && fResult2 * fResult3 > 0.f) {	// 부호가 모두 같으면 포함
			D3DXVECTOR3 Normal;			
			D3DXVec3Cross( &Normal, &p1, &p2 );
			D3DXVec3Normalize( &Normal, &Normal );			
			if( Normal.y < 0.f ) continue;
			*pHeight = (D3DXVec3Dot(&Normal, &pos) - D3DXVec3Dot(&Normal, pTriangle)) / -Normal.y;
			return true;
		}
	}

	return false;
}

struct GRASSBILLBOARDINFO
{
	char cTexName[64];
	int nNum;
	SPRITE_DATA Data[MAX_BILLBOARD_GRASS];
};


CFcPropManager::CFcPropManager()
{
	m_nPropNum = 0;
	m_ppProps = NULL;
	m_pZoneManager = NULL;
	m_bAlphaBlocking = true;
	m_nLightScatteringID = -1;
	m_nMagmaTwistA = -1;
	m_nMagmaTwistB = -1;
	m_nMagmaTwistC = -1;
	m_nLavaDistort = -1;

}

CFcPropManager::~CFcPropManager()
{
	for ( std::vector<STATICPROPDATA>::iterator it = m_vecStaticObjs.begin(); it != m_vecStaticObjs.end(); ++it)
	{
		g_BsKernel.DeleteObject(it->nStaticPropIndex);
	}

	int i, j;
	int nCnt = m_vecFX.size();
	for( i=0; i<nCnt; i++ )
	{
		g_BsKernel.DeleteObject(m_vecFX[i]);
	}

	SAFE_DELETE( m_pPropScanner );
	SAFE_DELETE( m_pActivePropScanner );

	m_vecActiveProps.clear();		//  m_ppProps 에서 다 지움
	m_vecMovableProps.clear();

	nCnt = m_vecTypeData.size();
	for( j=0; j<nCnt; j++ )
	{
		PROPTYPEDATA* pData = &(m_vecTypeData[j]);
		if( pData->nCollisionMeshIndex != -1 )
		{
			g_BsKernel.ReleaseMesh( pData->nCollisionMeshIndex );
		}
		delete[] pData->pAttr;
	}
	m_vecTypeData.clear();

	for( i=0; i<m_nPropNum; i++ )
	{
		SAFE_DELETE( m_ppProps[i] );
	}
	SAFE_DELETEA( m_ppProps );
	SAFE_DELETE( m_pZoneManager );

	/*if (m_nLightScatteringID != -1) {
		g_pFcFXManager->SendMessage(m_nLightScatteringID, FX_DELETE_OBJECT);
		m_nLightScatteringID = -1;
	}*/
	SAFE_DELETE_FX(m_nLightScatteringID,FX_TYPE_LIGHTSCATTERING);

	nCnt = m_vecPointLight.size();
	for( i = 0; i < nCnt; i++) {
		g_BsKernel.DeleteObject( m_vecPointLight[i].first );
	}
	m_vecPointLight.clear();	
	MagmaFxClear();
}

void CFcPropManager::MagmaFxClear()
{
	/*if( m_nMagmaTwistA != -1 )
	{
		g_pFcFXManager->SendMessage(m_nMagmaTwistA, FX_DELETE_OBJECT );
		m_nMagmaTwistA = -1;
	}*/
	SAFE_DELETE_FX(m_nMagmaTwistA,FX_TYPE_LISTGENERATER);
	/*if( m_nMagmaTwistB != -1 )
	{
		g_pFcFXManager->SendMessage(m_nMagmaTwistB, FX_DELETE_OBJECT );
		m_nMagmaTwistB = -1;
	}*/
	SAFE_DELETE_FX(m_nMagmaTwistB,FX_TYPE_LISTGENERATER);
	/*if( m_nMagmaTwistC != -1 )
	{
		g_pFcFXManager->SendMessage(m_nMagmaTwistC, FX_DELETE_OBJECT );
		m_nMagmaTwistC = -1;
	}*/
	SAFE_DELETE_FX(m_nMagmaTwistC,FX_TYPE_LISTGENERATER);
	/*if( m_nLavaDistort != -1 )
	{
		g_pFcFXManager->SendMessage(m_nLavaDistort, FX_DELETE_OBJECT );
		m_nLavaDistort = -1;
	}*/
	SAFE_DELETE_FX(m_nLavaDistort,FX_TYPE_LISTGENERATER);
	if( !m_vecRandomLava.empty() )
	{
		int nSize = m_vecRandomLava.size();
		for( int i = 0 ; i < nSize ; ++i )
		{
			int nHandle = m_vecRandomLava[i];
			//g_pFcFXManager->SendMessage(nHandle, FX_DELETE_OBJECT );
			SAFE_DELETE_FX(nHandle,FX_TYPE_RANDOMGENERATER);
		}
		m_vecRandomLava.clear();
	}
}

bool CFcPropManager::Initialize( float fWidth, float fHeight, int nPropZoneSize )
{	
	m_pPropScanner = new RangeScan< RS_Intf_Prop >;
	m_pPropScanner->Initialize( fWidth, fHeight, fWidth/4.0f, fHeight/4.0f, 300 );

	m_pActivePropScanner = new RangeScan< RS_Intf_Prop >;
	m_pActivePropScanner->Initialize( fWidth, fHeight, fWidth/4.0f, fHeight/4.0f, 300 );

	BsAssert( m_pZoneManager == NULL );
	m_pZoneManager = new PropZoneManager;

	m_fWidth = fWidth;
	m_fHeight = fHeight;

	return true;
}

bool CFcPropManager::LoadTypeDat( char* pFileName, CBSMapCore* pMapInfo, OUT VectorFxStr* p_pVecFxStr /*= NULL*/ )
{
	char cFileName[MAX_PATH];
	sprintf( cFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), pFileName );
	DWORD dwFileSize;
// [PREFIX:beginmodify] 2006/2/16 junyash PS#5212 reports Local declaration of 'pData' hides declaration of the same name in an outer scope. modify variable name 'pData' to 'pDataBuffer'.
	VOID *pDataBuffer;

	g_BsResChecker.AddResList( cFileName );

	if( FAILED(CBsFileIO::LoadFile( cFileName, &pDataBuffer, &dwFileSize ) ) )
	{
		BsAssert( 0 && "Can't load skin dat" );
		return false;
	}

	BMemoryStream stream(pDataBuffer, dwFileSize);
	int nCnt;
	stream.Read( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	m_vecTypeData.clear();
	m_vecTypeData.resize(nCnt);

	for( int i=0; i<nCnt; i++ )
	{
		PROPTYPEDATA* pData = &(m_vecTypeData[i]);
		pData->nSkinIndex = -1;
		stream.Read( &(pData->nID), sizeof(int), ENDIAN_FOUR_BYTE );

		char cTempStr[64];
		stream.Read( cTempStr, 64 );
		BsAssert( strlen(cTempStr)<32 );
		strcpy( pData->cName, cTempStr );

		int nValue;

		stream.Read( &nValue, sizeof(int), ENDIAN_FOUR_BYTE );
		pData->nClippingType = (short)nValue;

		stream.Read( &nValue, sizeof(int), ENDIAN_FOUR_BYTE );
		pData->bCrush = (byte)nValue;
		stream.Read( &nValue, sizeof(int), ENDIAN_FOUR_BYTE );

		pData->bCollosionMesh = false;
		pData->bClimb = false;
		if( nValue == 1 )
			pData->bCollosionMesh = true;
		else if( nValue == 2 )
			pData->bClimb = true;

		stream.Read( &nValue, sizeof(int), ENDIAN_FOUR_BYTE );
		pData->bShadowReceiver = (nValue != 0);

		stream.Read( &nValue, sizeof(int), ENDIAN_FOUR_BYTE );
		pData->bDynamic = (nValue != 0);
		stream.Read( &(pData->nType), sizeof(int), ENDIAN_FOUR_BYTE );
		stream.Read( &(pData->nLinkID), sizeof(int), ENDIAN_FOUR_BYTE );
		stream.Read( &nValue, sizeof(int), ENDIAN_FOUR_BYTE );
		pData->bAlphaBlocking = (nValue != 0 );
		stream.Read( &(pData->nCrushParticleIndex), sizeof(int), ENDIAN_FOUR_BYTE );
		stream.Read( &(pData->nShadowType), sizeof(int), ENDIAN_FOUR_BYTE );

		// 추가 FX Name 읽기
		stream.Read( cTempStr, 32 );
		strcpy( pData->cFXName[0], cTempStr );

		stream.Read( cTempStr, 32 );
		strcpy( pData->cFXName[1], cTempStr );

		// 추가 Crush Level 읽기
		stream.Read( &(pData->nCrushLevel), sizeof(int), ENDIAN_FOUR_BYTE );
		stream.Read( &(pData->nSndType), sizeof(int), ENDIAN_FOUR_BYTE );
		stream.Read( &(pData->nBrokenType), sizeof(int), ENDIAN_FOUR_BYTE );
	}

	CBsFileIO::FreeBuffer(pDataBuffer);
// [PREFIX:endmodify] junyash

	// 사용하는 프랍만 체크
	std::map<int, int> mapUsePropID;

	int nPropCnt = pMapInfo->GetPropCount();
	for( int i=0; i<nPropCnt; i++ )
	{
		PROP_INFO* pInfo = pMapInfo->GetPropInfo( i );

		std::map<int, int>::iterator it = mapUsePropID.find( pInfo->nSkinIndex );
		if( it == mapUsePropID.end() ) {
			PROPTYPEDATA* pPropTypeData = GetPropTypeDataByID( pInfo->nSkinIndex );
			if( pPropTypeData == NULL )
				continue;

			mapUsePropID.insert( std::make_pair(pInfo->nSkinIndex,  1) );

			if( pPropTypeData->cFXName[0][0] ) 
			{
				FxString* tmp1 = new FxString;
				strcpy( tmp1->_strBuffer, pPropTypeData->cFXName[0]);
				p_pVecFxStr->push_back(tmp1);
			}			
			if( pPropTypeData->cFXName[1][0] ) 
			{
				FxString* tmp2 = new FxString;
				strcpy( tmp2->_strBuffer, pPropTypeData->cFXName[1]);
				p_pVecFxStr->push_back(tmp2);
			}
		}
		else {
			(it->second)++;
		}

	}

	for( int i=0; i<nPropCnt; i++ )
	{
		PROP_INFO* pInfo = pMapInfo->GetPropInfo( i );
		PROPTYPEDATA* pPropTypeData = GetPropTypeDataByID( pInfo->nSkinIndex );
		if( pPropTypeData == NULL )
		{
			continue;
		}

		if( pPropTypeData->nLinkID < 0 )
		{
			continue;
		}
		std::map<int, int>::iterator it = mapUsePropID.find( pPropTypeData->nLinkID );
		if( it == mapUsePropID.end() ) {
			mapUsePropID.insert( std::make_pair(pPropTypeData->nLinkID, 1) );
		}
		else {
			(it->second)++;
		}
	}

	char cCollsionFileName[256];
	nCnt = m_vecTypeData.capacity();
	for(int i=0; i<nCnt ; i++)
	{
		std::map<int, int>::iterator it = mapUsePropID.find( m_vecTypeData[i].nID );
		if( it == mapUsePropID.end() ) 
			continue;

		g_BsKernel.SetInstancingBufferSize( it->second );
		m_vecTypeData[i].nSkinIndex = g_FcWorld.LoadMeshData( -1, m_vecTypeData[i].cName );
		g_BsKernel.SetInstancingBufferSize( 5 );
		if( m_vecTypeData[i].nSkinIndex < -1 )
		{
			BsAssert(0);
			return false;
		}

		// 애니있는 타입들
		if( m_vecTypeData[i].nType == PROP_TYPE_GATE || 
			m_vecTypeData[i].nType == PROP_TYPE_TRAP ||
			m_vecTypeData[i].nType == PROP_TYPE_ANIMATION )
		{
			char cGameAniName[256];
			strcpy( cGameAniName, m_vecTypeData[i].cName );
			char* pTempStr = strchr( cGameAniName, '.' );
			if ( pTempStr ) pTempStr[0] = NULL;	// mruete: prefix bug 593: added check
			strcat( cGameAniName, ".ba" );

			m_vecTypeData[i].nAniIndex = g_FcWorld.LoadAniData( -1, cGameAniName );
			if( m_vecTypeData[i].nAniIndex < -1 )
			{
				BsAssert(0);
				return false;
			}
		}

		if( m_vecTypeData[i].bCollosionMesh == true )
		{
			strcpy( cCollsionFileName, m_vecTypeData[i].cName );
			char* pTempStr = strchr( cCollsionFileName, '.' );
			pTempStr[0] = NULL;
			strcat( cCollsionFileName, "_cl.bm" );

			//	CBsMesh *pM = g_BsKernel.Get_pMesh(m_nMeshId);
			m_vecTypeData[i].nCollisionMeshIndex = g_BsKernel.LoadMesh( -1, cCollsionFileName );
			if( m_vecTypeData[i].nCollisionMeshIndex < -1 )
			{
				BsAssert(0);
				return false;
			}
		}
		if( m_vecTypeData[i].bClimb == true )
		{
			//char cFileName[MAX_PATH];	// mruete: prefix bug 594: re-use outer instance
			strcpy( cFileName, m_vecTypeData[i].cName );
			char* pTempStr = strchr( cFileName, '.' );
			pTempStr[0] = NULL;
			strcat( cFileName, ".phm" );

			// 파일이 없다면 새로 만든다.
			if( m_vecTypeData[i].HeightMapInfo.Load( g_BsKernel.GetFullName( cFileName ) ) == false )
			{
				GeneratePropHeightMapFile( m_vecTypeData[i].cName, cFileName, &(m_vecTypeData[i].nDebugClimbMeshIndex) );
				if( m_vecTypeData[i].HeightMapInfo.Load( g_BsKernel.GetFullName( cFileName ) ) == false )
				{
					BsAssert( 0 && "Cannot load PHM file" );
					return false;
				}
			}
		}

/* 필요 없음
		// Dynamic Prop에 대한 처리
		if( m_vecTypeData[i].bDynamic == true )
		{
			strcpy( cCollsionFileName, m_vecTypeData[i].cName );
			char* pTempStr = strchr( cCollsionFileName, '.' );
			pTempStr[0] = NULL;
			strcat( cCollsionFileName, "_dy.bm" );

			m_vecTypeData[i].nDynmicSkinIndex = g_FcWorld.LoadMeshData( -1, cCollsionFileName );
			if( m_vecTypeData[i].nDynmicSkinIndex < -1 )
			{
				BsAssert(0);
				return false;
			}
		}
*/
	}

	return true;
}


bool CFcPropManager::LoadBSAttr( char* pFileName )
{
	char cFileName[MAX_PATH];
	sprintf( cFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), pFileName );

	DWORD dwFileSize;
// [PREFIX:beginmodify] 2006/2/16 junyash PS#5213 reports Local declaration of 'pData' hides declaration of the same name in an outer scope. modify variable name 'pData' to 'pDataBuffer'.
	VOID *pDataBuffer;

#ifdef ENABLE_RES_CHECKER
	g_BsResChecker.AddResList( cFileName );
#endif

	if( FAILED(CBsFileIO::LoadFile( cFileName, &pDataBuffer, &dwFileSize ) ) )
	{
		BsAssert( 0 && "Can't load BSAttr dat" );
		return false;
	}

	int nVersion, nCnt;

	BMemoryStream stream(pDataBuffer, dwFileSize);
	stream.Read( &nVersion, sizeof( int ), 4 );
	stream.Read( &nCnt, sizeof( int ), 4 );

	for( int i=0; i<nCnt; i++ )
	{
		int nStrLength;
		char cSkinName[64];
		stream.Read( &nStrLength, sizeof( int ), 4 );
		stream.Read( cSkinName, nStrLength );
		cSkinName[nStrLength] = NULL;

		int nLeft, nTop;
		stream.Read( &nLeft, sizeof( int ), 4 );
		stream.Read( &nTop, sizeof( int ), 4 );


		stream.Seek( 12, BStream::fromNow );
		
		int nXS, nYS;
		stream.Read( &nXS, sizeof( int ), 4 );
		stream.Read( &nYS, sizeof( int ), 4 );

		BYTE* pBuf = NULL;
		if( nXS * nYS > 0 )
		{
			pBuf = new BYTE[ nXS * nYS ];
			stream.Read( pBuf, nXS * nYS );
		}

		if( pBuf )
		{
			int nCntTypeData = m_vecTypeData.size();	// mruete: prefix bug 597: renamed nCnt -> nCntTypeData
			int j;
			for( j=0; j<nCntTypeData; j++ )
			{
				PROPTYPEDATA* pData = &(m_vecTypeData[j]);
				if( strcmp( pData->cName, cSkinName ) == 0 )
				{
					pData->nAttrLeft = nLeft;
					pData->nAttrTop = nTop;
					pData->nAttrXSize = nXS;
					pData->nAttrYSize = nYS;
					pData->pAttr = pBuf;
					break;
				}
			}

			if( j == nCntTypeData )
			{
				delete [] pBuf;
			}
		}
	}

	/*
	nCnt = m_vecTypeData.size();
	for( int j=0; j<nCnt; j++ )
	{
		PROPTYPEDATA* pData = &(m_vecTypeData[j]);
		//		BsAssert( pData->pAttr );
	}
	*/
	CBsFileIO::FreeBuffer(pDataBuffer);
// [PREFIX:endmodify] junyash

	return true;
}


void CFcPropManager::InitLightScatteringFX( bool bPholya )
{
	if( m_nLightScatteringID == -1 ) {
		// light scattering initialize  사용하는 경우에만 Initialize해준다.
		m_nLightScatteringID = g_pFcFXManager->Create(FX_TYPE_LIGHTSCATTERING);
		g_pFcFXManager->SendMessage( m_nLightScatteringID , FX_PLAY_OBJECT);
		if( bPholya )
			g_pFcFXManager->SendMessage( m_nLightScatteringID , FX_INIT_OBJECT, 0, 0, 2 );// 2 = SkyBox 따라다니는 큰 외곽 빛줄기, 1 = Pholya 용 빛줄기, 0 = 금기의숲 빛줄기
		else
			g_pFcFXManager->SendMessage( m_nLightScatteringID, FX_INIT_OBJECT );
	}	
}

// 맵둘에 찍힌 Prop들 정보는 PROP_INFO
// PropSkin.txt에서 읽은 Prop정보는 PROPTYPEDATA

void CFcPropManager::CreateProps( std::vector<PROP_INFO>& vecProps )
{
	// 포리아 맵 빛줄기 정보 생성
	if( strncmp(g_FCGameData.cMapFileName, "ph_", 3) == 0 ||
		strncmp(g_FCGameData.cMapFileName, "Ph_", 3) == 0 || 
		strncmp(g_FCGameData.cMapFileName, "CEM", 3) == 0 ) {
        InitLightScatteringFX( true );		
	}
	m_nPropNum = vecProps.size();

	BsAssert( m_ppProps == NULL );
	m_ppProps = new CFcProp* [m_nPropNum];
	memset( m_ppProps, 0, sizeof(CFcProp*) * m_nPropNum );

	int nDatIndexTable[5000];
	for( int i=0; i<5000; i++ )
		nDatIndexTable[i] = -1;

	static int s_nDebugCurrentPropIndex;
	for( int i=0; i<m_nPropNum; i++ )
	{
		s_nDebugCurrentPropIndex = i;
		PROP_INFO* pInfo = &(vecProps[i]);
		BsAssert( pInfo->nSkinIndex >= 0 && pInfo->nSkinIndex < 5000 );

		int nDatIndex = nDatIndexTable[pInfo->nSkinIndex];
		if( nDatIndex == -1 )
		{
			int nCnt = m_vecTypeData.capacity();
			for( int i=0; i<nCnt; i++ )
			{
				if( pInfo->nSkinIndex == m_vecTypeData[i].nID )
				{
					nDatIndex = nDatIndexTable[pInfo->nSkinIndex] = i;

					break;
				}
			}
		}
		if( nDatIndex == -1 )
			continue;

		CFcProp* pProp = CreateProp( pInfo, nDatIndex );
		m_ppProps[i] = pProp;

		if( pProp )
		{
			if( IsActiveProp( pProp ) )
				m_pActivePropScanner->AddObject( pProp );

			m_pPropScanner->AddObject( pProp );

			// 이름이 정해져있거나 문, 다리이면 특수한 프랍으로 등록
			if( IsActiveProp( pProp ) )
				m_vecActiveProps.push_back( pProp );

			if( pProp->IsDynamicProp() )
				m_vecMovableProps.push_back( pProp );
			else
				m_pZoneManager->AddProp( pProp );		// 움직이는 애는 등록 안해야 한다.
		}
	}

	m_AABBCollisionTree.BuildTree();
	
	// 더이상 안쓰이므로 미리 지워준다.
#ifndef _SHOW_COLLISION_PROP
	int nCnt = m_vecTypeData.size();
	for( int i = 0; i < nCnt; i++)
	{
		PROPTYPEDATA* pData = &(m_vecTypeData[i]);
		if( pData->nCollisionMeshIndex != -1 )
		{
			g_BsKernel.ReleaseMesh( pData->nCollisionMeshIndex );
			pData->nCollisionMeshIndex = -1;
		}
	}
#endif
}

CFcProp* CFcPropManager::CreateProp(PROP_INFO *pPropInfo, int nDatIndex )
{
	int nID = -1;
	int nSkinIndex = -1;
	PROPTYPEDATA* pData = &(m_vecTypeData[nDatIndex]);
	BsAssert( pData );
	BsAssert( pData->nSkinIndex != -1 );

	char* pName = pPropInfo->szPropName;
	D3DXVECTOR3 Pos( pPropInfo->Pos.fPos[0], pPropInfo->Pos.fPos[1], pPropInfo->Pos.fPos[2] );
	DWORD dwRotAndScale = pPropInfo->dwParam1;
	DWORD* pParam = pPropInfo->dwParamExt;
	char* pParamString = pPropInfo->szExtBuffer;
	int iSkinIndex = pPropInfo->nSkinIndex;

	CFcProp* pProp = NULL;

	switch( pData->nType )
	{
	case PROP_TYPE_GATE:
		{
			CFcGateProp* pGate = new CFcGateProp();
			pGate->Initialize( nDatIndex, pData, pName, &Pos, dwRotAndScale );
			pProp = (CFcProp *)pGate;
		}
		break;

	case PROP_TYPE_BRIDGE:
		{
			CFcBridgeProp* pBridge = new CFcBridgeProp();
			PROPTYPEDATA* pLinkData = NULL;
			if( pData->nLinkID != -1 )
			{
				pLinkData = GetPropTypeDataByID( pData->nLinkID );
				BsAssert( pLinkData );
			}
			pBridge->Initialize( nDatIndex, pData, pLinkData, pName, &Pos, dwRotAndScale );
			pProp = (CFcProp *)pBridge;
		}
		break;

	case PROP_TYPE_ENABLE_ATTACK_WALL:
		{
			if( pData->bCrush != 0 )
			{
				BsAssert( pData->nLinkID != -1 );
				CFcBreakableProp* pBreakable = new CFcBreakableProp();

				pBreakable->Initialize( nDatIndex, pData, GetPropTypeDataByID( pData->nLinkID ), 
					pName, &Pos, dwRotAndScale, pParam );
				pProp = (CFcProp *)pBreakable;
			}
			else
			{
				pProp = new CFcProp();
				pProp->Initialize( nDatIndex, pData, pName, &Pos, dwRotAndScale, pParam);
			}
		}
		break;

	case PROP_TYPE_TRAP:
		{
			CFcTrap* pTrap = new CFcTrap();
			pTrap->Initialize( nDatIndex, pData, pName, &Pos, dwRotAndScale, pParam );

			pProp = (CFcProp *)pTrap;
		}
		break;

	case PROP_TYPE_BILLBOARD:
		{
			//			int iOffsetX = ((BYTE)(dwRotAndScale>>16));
			//			int iOffsetY = ((BYTE)(dwRotAndScale>>8));
			//			float fXScale = (float)(0.5f + ( 0.01 * (float)iOffsetX ));
			//			float fYScale = (float)(0.5f + ( 0.01 * (float)iOffsetY ));
			//			fXScale *= 110.0f;
			//			fYScale *= 90.0f;
			//			g_BsKernel.GetInstance().GetBillboardMgr()->PutPosToPool( *pPos, D3DXVECTOR2( fXScale, fYScale ), iSkinIndex );
			// BsAssert( 0 && "Billboard type은 이곳으로 오면 안됩니다.");
			pProp = NULL;
		}
		break;

	case PROP_TYPE_CRUMBLE:
		{
			CFcBreakAndBlockWayProp* pBreakAndBlockWayProp = new CFcBreakAndBlockWayProp();
			pBreakAndBlockWayProp->Initialize( nDatIndex, pData, GetPropTypeDataByID( pData->nLinkID ), pName, &Pos, dwRotAndScale );
			pProp = (CFcProp *)pBreakAndBlockWayProp;
		}
		break;

	case PROP_TYPE_ITEM:
		{
			CFcItemProp* pItemProp = new CFcItemProp();
			pItemProp->Initialize( nDatIndex, pData, pName, &Pos, pParam );
			pProp = (CFcProp*)pItemProp;
		}
		break;

	case PROP_TYPE_WALL:
		{
			BsAssert( pData->nLinkID != -1 );
			if( pData->bCrush )
			{
				CFcBreakableProp* pBreakable = new CFcBreakableProp();
				pBreakable->Initialize( nDatIndex, pData, GetPropTypeDataByID( pData->nLinkID ), pName, 
					&Pos, dwRotAndScale, pParam );
				pProp = (CFcProp *)pBreakable;
			}
			else
			{
				pProp = new CFcProp();
				pProp->Initialize( nDatIndex, pData, pName, &Pos, dwRotAndScale, pParam );
			}
		}
		break;
	case PROP_TYPE_PICKUP:
		pProp = new CFcProp();
		pProp->Initialize( nDatIndex, pData, pName, &Pos, dwRotAndScale, pParam );
		break;

	case PROP_TYPE_ANIMATION:
		{
			if( pData->bCrush != 0 ) {
				BsAssert( pData->nLinkID != -1 );
				CFcBreakableProp* pBreakable = new CFcBreakableProp();
				pBreakable->Initialize( nDatIndex, pData, GetPropTypeDataByID( pData->nLinkID ), pName, 
					&Pos, dwRotAndScale, pParam );
				pProp = (CFcProp *)pBreakable;
			}
			else {
				CFcAnimationProp* pAniProp = new CFcAnimationProp();
				pAniProp->Initialize( nDatIndex, pData, pName, &Pos, dwRotAndScale, pParam );
				pProp = (CFcProp *)pAniProp;
			}
		}
		break;


	default:
		// Dynamic도 추가해야 함
		if( pData->bCrush != 0 )
		{
			BsAssert( pData->nLinkID != -1 );
			CFcBreakableProp* pBreakable = new CFcBreakableProp();
			pBreakable->Initialize( nDatIndex, pData, GetPropTypeDataByID( pData->nLinkID ), pName, 
				&Pos, dwRotAndScale, pParam );
			pProp = (CFcProp *)pBreakable;
		}
		else if( pData->bDynamic )
		{
			g_BsKernel.chdir("Prop");
			CFcDynamicProp* pDynamic = new CFcDynamicProp();
			pDynamic->Initialize( nDatIndex, pData, pName, &Pos, dwRotAndScale );
			pProp = (CFcProp *)pDynamic;
			g_BsKernel.chdir("..");
		}
		else if( pData->bClimb )
		{
			CFcClimbProp* pClimb = new CFcClimbProp();
			pClimb->Initialize( nDatIndex, pData, pName, &Pos, dwRotAndScale );
			pProp = (CFcProp *)pClimb;
		}
		else if( pData->bCollosionMesh )
		{
			pProp = new CFcProp();
			pProp->Initialize( nDatIndex, pData, pName, &Pos, dwRotAndScale, pParam );
			break;
		}
		else
		{
			// 아무 것도 안하는 프랍은 StaticPropManager에 등록
			if( pName == NULL || pName[0] == NULL )
			{
				int nObjectIndex = g_BsKernel.CreateStaticObjectFromSkin( pData->nSkinIndex );
				if( nObjectIndex < 0 )
				{
					BsAssert(0);
					return NULL; 
				}
				// 맵툴의 공식대로 회전, 스케일 조절
				CBsObject* pObj = g_BsKernel.GetEngineObjectPtr( nObjectIndex );
				CCrossVector Cross;
				CFcProp::InitCrossVec( &Pos, dwRotAndScale, &Cross );
				pObj->SetEntireObjectMatrix( Cross );

				STATICPROPDATA StaticData;
				StaticData.nClippingType = pData->nClippingType;
				StaticData.nStaticPropIndex = nObjectIndex;

				m_vecStaticObjs.push_back( StaticData );	
				int nStaticObjIndex = m_vecStaticObjs.size() - 1;
				m_pZoneManager->AddStaticProp( nObjectIndex, nStaticObjIndex, (int)pData->nClippingType );
			}
			else
			{
				if( !_stricmp(pData->cName, EMPTY_SKIN_NAME ) ) {
					switch( pParam[0] ) 
					{
					case FXPROP_LIGHTSCATTERING :
						{
							InitLightScatteringFX();

							CCrossVector Cross;
							CFcProp::InitCrossVec( &Pos, dwRotAndScale, &Cross );
							g_pFcFXManager->SendMessage( m_nLightScatteringID , FX_INIT_OBJECT, (DWORD)&Cross, pParam[1] , 0);
							break;
						}
					case FXPROP_POINTLIGHT:
						{
							int nIndex = g_BsKernel.CreateLightObject( POINT_LIGHT );
							CBsObject *pObj = g_BsKernel.GetEngineObjectPtr( nIndex );
							CCrossVector Cross;
							CFcProp::InitCrossVec( &Pos, dwRotAndScale, &Cross );
							Cross.SetPosition( Cross.GetPosition() + D3DXVECTOR3(0, (float)pParam[2], 0) );
							g_BsKernel.UpdateObject( nIndex, Cross);
							D3DCOLORVALUE Color;
							Color.r = pParam[3] / 255.f;
							Color.g = pParam[4] / 255.f;
							Color.b = pParam[5] / 255.f;
							Color.a = 1.f;
							float fRange = (float)pParam[1];
							g_BsKernel.SendMessage( nIndex, BS_SET_POINTLIGHT_COLOR, (DWORD)&Color );
							g_BsKernel.SendMessage( nIndex, BS_SET_POINTLIGHT_RANGE, (DWORD)&fRange );

							if( pParam[6] != 0 ) {
								fRange = -1.f;
							}
							m_vecPointLight.push_back( std::make_pair(nIndex, fRange) );							
							break;
						}
					case FXPROP_LAVA:
						{
							CCrossVector Cross;
							CFcProp::InitCrossVec( &Pos, dwRotAndScale, &Cross );
							int nLava = g_pFcFXManager->Create( FX_TYPE_RANDOMGENERATER );
							g_pFcFXManager->SendMessage(nLava, FX_INIT_OBJECT, FX_LAVA, 100, 1);
							Cross.m_PosVector.y += (float)pParam[2];
							g_pFcFXManager->SendMessage(nLava, FX_ADD_VALUE, (DWORD)&Cross.m_PosVector, (DWORD)&Cross.m_ZVector, pParam[1]);
							g_pFcFXManager->SendMessage( nLava, FX_PLAY_OBJECT, 1, pParam[3], pParam[4] );
							m_vecRandomLava.push_back(nLava);
						}
						break;
					case FXPROP_LAVADISTORT:
						{
							if( m_nLavaDistort == -1 )
							{
								m_nLavaDistort = g_pFcFXManager->Create(FX_TYPE_LISTGENERATER);
								g_pFcFXManager->SendMessage( m_nLavaDistort, FX_INIT_OBJECT, FX_LAVADISTORT, 100, -1 );
								g_pFcFXManager->SendMessage( m_nLavaDistort, FX_PLAY_OBJECT, 1, 4,1 );								
							}
							CCrossVector Cross;
							CFcProp::InitCrossVec( &Pos, dwRotAndScale, &Cross );
							Cross.m_PosVector.y += (float)pParam[2];
							g_pFcFXManager->SendMessage( m_nLavaDistort, FX_UPDATE_OBJECT, (DWORD)&Cross.m_PosVector, (DWORD)&Cross.m_ZVector );
						}
						break;
					case FXPROP_MAGMATWISTA:
						{
							if( m_nMagmaTwistA == -1 )
							{
								m_nMagmaTwistA = g_pFcFXManager->Create(FX_TYPE_LISTGENERATER);
								g_pFcFXManager->SendMessage( m_nMagmaTwistA, FX_INIT_OBJECT, FX_MAGMATWISTA, 100, -1 );
								g_pFcFXManager->SendMessage( m_nMagmaTwistA, FX_PLAY_OBJECT, 1 );
							}
							CCrossVector Cross;
							CFcProp::InitCrossVec( &Pos, dwRotAndScale, &Cross );
							Cross.m_PosVector.y += (float)pParam[2];
							g_pFcFXManager->SendMessage( m_nMagmaTwistA, FX_UPDATE_OBJECT, (DWORD)&Cross.m_PosVector, (DWORD)&Cross.m_ZVector );
						}
						break;
					case FXPROP_MAGMATWISTB:
						{
							if( m_nMagmaTwistB == -1 )
							{
								m_nMagmaTwistB = g_pFcFXManager->Create(FX_TYPE_LISTGENERATER);
								g_pFcFXManager->SendMessage( m_nMagmaTwistB, FX_INIT_OBJECT, FX_MAGMATWISTA, 100, -1 );
								g_pFcFXManager->SendMessage( m_nMagmaTwistB, FX_PLAY_OBJECT, 1 );
							}
							CCrossVector Cross;
							CFcProp::InitCrossVec( &Pos, dwRotAndScale, &Cross );
							Cross.m_PosVector.y += (float)pParam[2];
							g_pFcFXManager->SendMessage( m_nMagmaTwistB, FX_UPDATE_OBJECT, (DWORD)&Cross.m_PosVector, (DWORD)&Cross.m_ZVector );
						}
						break;
					case FXPROP_MAGMATWISTC:
						{
							if( m_nMagmaTwistC == -1 )
							{
								m_nMagmaTwistC = g_pFcFXManager->Create(FX_TYPE_LISTGENERATER);
								g_pFcFXManager->SendMessage( m_nMagmaTwistC, FX_INIT_OBJECT, FX_MAGMATWISTA, 100, -1 );
								g_pFcFXManager->SendMessage( m_nMagmaTwistC, FX_PLAY_OBJECT, 1 );
							}
							CCrossVector Cross;
							CFcProp::InitCrossVec( &Pos, dwRotAndScale, &Cross );
							Cross.m_PosVector.y += (float)pParam[2];
							g_pFcFXManager->SendMessage( m_nMagmaTwistC, FX_UPDATE_OBJECT, (DWORD)&Cross.m_PosVector, (DWORD)&Cross.m_ZVector );
						}
						break;
					case FXPROP_LIGHTSCATTERING_PHOLYA:
						{
							InitLightScatteringFX();
							
							CCrossVector Cross;
							CFcProp::InitCrossVec( &Pos, dwRotAndScale, &Cross );							
							g_pFcFXManager->SendMessage( m_nLightScatteringID , FX_INIT_OBJECT, (DWORD)&Cross, pParam[1], 1 );
							//////////////////////////////////////////////////////////////////////////
							int nIndex = g_BsKernel.CreateLightObject( POINT_LIGHT );
							Cross.SetPosition( Cross.GetPosition() + D3DXVECTOR3(0,5,0));	// 조금만 높여준다..오차로 밑으로 내려가는경우 대비
							g_BsKernel.UpdateObject( nIndex, Cross);
							D3DCOLORVALUE Color;
							Color.r = 1.f;
							Color.g = 1.f;
							Color.b = 1.f;
							Color.a = 1.f;
							float fRange = 1000.f;
							g_BsKernel.SendMessage( nIndex, BS_SET_POINTLIGHT_COLOR, (DWORD)&Color );
							g_BsKernel.SendMessage( nIndex, BS_SET_POINTLIGHT_RANGE, (DWORD)&fRange );							
							
							m_vecPointLight.push_back( std::make_pair(nIndex, -1.f ) );
						}
						break;
					}
					/*
					***** Map tool에서 사용될 param 값 *****
					Param1 : id ( 2: 용암에서 치솟는 fx, 3:용암연기 및 일렁이는fx, 4 5 6: 회전해서 올라가는 용암의 외부 일렁이는 fx)
					Param2 : id 가 2번일 경우 확률적 수치 ( 예 : 100 이면 100 번중 랜덤하여 1번 )
					Param3 : 더해질 높이
					****************************************
					*/
				}
				else {	
					pProp = new CFcProp();
					pProp->Initialize( nDatIndex, pData, pName, &Pos, dwRotAndScale );
				}
			}
		}
		break;
	}
	return pProp;
}

void CFcPropManager::GetActiveProps( D3DXVECTOR3& Pos, float rRadius, std::vector<CFcProp*>& vecProps )
{	
	m_pActivePropScanner->Find( Pos.x, Pos.z, rRadius, vecProps );
}

void CFcPropManager::GetActiveProps( float fSX, float fSY, float fEX, float fEY, std::vector<CFcProp*>& vecProps )
{	
	m_pActivePropScanner->Find( fSX, fSY, fEX, fEY, vecProps );
}




void CFcPropManager::UpdateObjects()
{
	const float fLimintAngle = cosf(D3DX_PI*0.2f);
	const float fRcpLimitAngle = 1.f/(1.f - fLimintAngle);
	for( int i=0; i<2; i++ )		// 카메라 2개까지
	{
		CSmartPtr<CFcCameraObject> hCamera = CFcBaseObject::GetCameraObjectHandle(i);
		if( hCamera == NULL ) {
			continue;
		}

		//***********************************************************************
		// 카메라와 Attaching하고 있는 오브젝트 사이의 Prop을 Alpha처리한다.
		//***********************************************************************
		D3DXVECTOR2 vecTargetPos;
		if( hCamera->IsCamPreset() )
		{
			vecTargetPos = hCamera->GetCrossVector()->GetPositionXZ();
		}
		else
		{
			CSmartPtr<CFcBaseObject> hTarget = hCamera->GetAttachObjectHandle();
			vecTargetPos = hTarget->GetCrossVector()->GetPositionXZ();
		}

		D3DXVECTOR2 vecCamPos = hCamera->GetCrossVector()->GetPositionXZ();
		D3DXVECTOR2 vecDirTargetToCam = -(vecTargetPos - vecCamPos);
		float fDistanceTargetToCam = D3DXVec2Length(&vecDirTargetToCam);
		//		BsAssert( fDistanceTargetToCam<5000.f && "Camera와 Target사이의 거리가 너무 멉니다." );

		if(fDistanceTargetToCam < 5000.f && m_bAlphaBlocking) {
			vecDirTargetToCam = vecDirTargetToCam/fDistanceTargetToCam;
			std::vector<int> vecNearProps;
			GetNearAlphaBlockingProps( vecCamPos.x, vecCamPos.y, fDistanceTargetToCam, vecNearProps );
			// ZoneData
			int nNearPropCount = vecNearProps.size();
			D3DXVECTOR2 vecPropPos, vecDirTargetToProp;
			float fDistanceTargetToProp;
			float fAlphaFinal=1.f;
			for(int j=0 ; j<nNearPropCount ; ++j ) {	// mruete: prefix bug 598: renamed to j
				const D3DXMATRIX* pMat = g_BsKernel.GetEngineObjectPtr( vecNearProps[j] )->GetObjectMatrix();
				vecPropPos = D3DXVECTOR2( pMat->_41, pMat->_43 );

				vecDirTargetToProp = vecPropPos - vecTargetPos;
				fDistanceTargetToProp = D3DXVec2Length(&vecDirTargetToProp);
				vecDirTargetToProp = vecDirTargetToProp/fDistanceTargetToProp;
				float fDot = D3DXVec2Dot(&vecDirTargetToProp, &vecDirTargetToCam);
				if(fDot>fLimintAngle) {
					float fGap = (fDot - fLimintAngle)*fRcpLimitAngle;			// Angle고려!!
					float fAlpha = 1.f - fGap;
					//					fAlphaFinal = fAlpha*(fDistanceTargetToProp/fDistanceCamToTarget);	// 거리 고려!!
					// 거리 고려는 다음에 합니다.
					fAlphaFinal= (fAlpha<=0.1f)? 0.1f:fAlpha; 

					int nEngineIndex = vecNearProps[j];
					g_BsKernel.SendMessage(nEngineIndex, BS_ENABLE_OBJECT_ALPHABLEND, TRUE);
					g_BsKernel.SendMessage(nEngineIndex, BS_SET_OBJECT_ALPHA, DWORD(&fAlphaFinal));
					g_BsKernel.SendMessage(nEngineIndex, BS_RESTORE_OBJECT_ALPHABLEND);
				}
			}
		}


		//***********************************************************************
		// Prop들에 대한 UpdateObject()호출!!!
		//***********************************************************************
		//		D3DXVECTOR2 vecUpateCenter = hCamera->GetUpdateRangeCenter();
		UpdateVisibleProps();
	}
}


PROPTYPEDATA* CFcPropManager::GetPropTypeData_( int index )
{
	BsAssert( index < (int)m_vecTypeData.capacity() && index >= 0 );
	return &(m_vecTypeData[index]);
}

PROPTYPEDATA* CFcPropManager::GetPropTypeDataByID( int nID )
{
	int nCnt = m_vecTypeData.capacity();
	for( int i=0; i<nCnt; i++ )
	{
		PROPTYPEDATA* pData = &(m_vecTypeData[i]);
		if( pData->nID == nID )
		{
			return pData;
		}
	}
	//	BsAssert( 0 );
	DebugString( "Invalid Prop ID : %d\n", nID );
	return NULL;
}



void CFcPropManager::DebugCollsionPropRender()
{
#ifndef _XBOX
	CSmartPtr<CFcCameraObject> hCamera = CFcBaseObject::GetCameraObjectHandle(0);
	std::vector<CFcProp*> vecProps;
	D3DXVECTOR2 vecUpateCenter = hCamera->GetUpdateRangeCenter();
	GetVisibleProps( &vecUpateCenter, hCamera->GetUpdateRange(), vecProps );

	int nCnt = vecProps.size();
	for( int i=0; i<nCnt; i++ )
	{
		CFcProp* pProp = vecProps[i];
		pProp->DebugCollisionPropRender();
	}
#endif
}

float CFcPropManager::GetTwinkleLightIntensity( int nTick, int nSeed )
{
	const int nLoop = 3;
	const int nTickTerm = 180;
	const float nDivide = 6;

	int nNewTick = nTick + nSeed * 137;
	float t = (nNewTick % nTickTerm) / (float)nTickTerm;

	for( int j=0;j<nLoop;j++)
		t = sinf( D3DX_PI * t);

	t = ((int)(t * nDivide)) / nDivide;

	static float fRandNum = 0;
	if( nTick % 2 == 0 ) {
		fRandNum = RandomNumberInRange(0.f, 0.3f);
	}
	t = t * 0.7f + fRandNum;

	return t;
}

void CFcPropManager::DeletePointLightRange( float fX, float fZ, float fRange)
{
	for(std::vector< std::pair<int, float> >::iterator it =  m_vecPointLight.begin(); it != m_vecPointLight.end(); ) {
		int nEngineIndex = it->first;
		CBsLightObject *pLightObj = ((CBsLightObject*)g_BsKernel.GetEngineObjectPtr( nEngineIndex ));
		D3DXVECTOR4 pointLightInfo = *pLightObj->GetPointLightInfo();

		float fDistX = fX - pointLightInfo.x;
		float fDistZ = fZ - pointLightInfo.z;
		float fTotalRange = (fRange + it->second );

		if(  (fDistX * fDistX) + (fDistZ * fDistZ)  < fTotalRange * fTotalRange ) {
			g_BsKernel.DeleteObject( nEngineIndex );
			it = m_vecPointLight.erase(it);
		}
		else {
			++it;
		}
	}
}

void CFcPropManager::Process()
{
	int i, nCnt;

	// m_vecProps은 할 필요 없을지도
	for( i=0; i<m_nPropNum; ++i )
	{
		CFcProp *pProp = m_ppProps[i];
		if( pProp == NULL )
			continue;

		if( !pProp->IsNoProcessProp() )		// 여기서 ActiveProp 까지 Process 해준다.
			pProp->Process();
	}

	nCnt = m_vecActiveProps.size();
	for( i=0; i<nCnt ; ++i )
	{
		CFcProp *pProp = m_vecActiveProps[i];		

		if( pProp->IsMoveUpdate())
		{						
			if(pProp->GetX() >= 0 && pProp->GetZ() >= 0 && pProp->GetX() < m_fWidth && pProp->GetZ() < m_fHeight)
			{
				float fOldX = pProp->GetOldX();
				float fOldZ = pProp->GetOldZ();
				m_pActivePropScanner->MoveObject(fOldX, fOldZ, pProp);
				m_pPropScanner->MoveObject(fOldX, fOldZ, pProp);
				pProp->SetMoveUpdate(false);		// 업데이트 완료
			}
			else
			{
				static bool bBreak = true;
				if( bBreak ) {
					//static CFcProp *s_pProp;
					//s_pProp = pProp;
//					_DEBUGBREAK;
				}
			}
		}
		if( pProp->IsDelete() ) {
			RemoveProp(pProp);
			i--;
			nCnt--;
			continue;
		}
	}

	nCnt = m_vecPointLight.size();

	int nTick = GetProcessTick();
	for( i = 0; i < nCnt; i++) {
		int nIndex = m_vecPointLight[i].first;
		float fRange = m_vecPointLight[i].second;
		if( fRange < 0.f ) continue;		// 범위가 음수인 경우는 깜박임 사용안함
		float fRangeMin = fRange * 0.8f;
		float fRangeMax = fRange * 1.3f;
		float t = GetTwinkleLightIntensity( nTick, i );
		fRange = t * (fRangeMax-fRangeMin) + fRangeMin;
		g_BsKernel.SendMessage( nIndex, BS_SET_POINTLIGHT_RANGE, (DWORD)&fRange);
	}
}


CFcProp* CFcPropManager::GetProp( char* pName )
{
	int nCnt = m_vecActiveProps.size();
	for( int i=0; i<nCnt; i++ )
	{
		CFcProp* pProp = m_vecActiveProps[i];
		char* pPropName = pProp->GetName();
		if( pPropName == NULL )
		{
			continue;
		}
		if( _strcmpi(pPropName, pName ) == 0 )
		{
			return pProp;
		}
	}
	return NULL;
}

void CFcPropManager::SetEventNavInProps( NavigationMesh* pNavigationMesh, NAVEVENTINFO* pNavEventInfo, int nNumNavEventInfo)
{
	int nCnt = m_vecActiveProps.size();
	for( int i=0; i<nCnt; i++ )
	{
		CFcProp* pProp = m_vecActiveProps[i];
		if( pProp->IsBridgeProp() )
		{
			D3DXVECTOR2 Pos = pProp->GetPosV2();
			int nNavIndex = SearchNearNavIndex( &Pos, pNavigationMesh, pNavEventInfo, nNumNavEventInfo );
			if( nNavIndex == -1 )
			{
				BsAssert(0 && "Bridge Attribute Needed");
				continue;
			}
			( (CFcBridgeProp*)pProp )->SetEventNav( nNavIndex, pNavigationMesh, pNavEventInfo[nNavIndex], true );
			continue;
		}
		if( pProp->IsGateProp() )
		{
			D3DXVECTOR2 Pos = pProp->GetPosV2();
			int nNavIndex = SearchNearNavIndex( &Pos, pNavigationMesh, pNavEventInfo, nNumNavEventInfo );
			if( nNavIndex == -1 )
			{
				BsAssert(0 && "Gate Attribute Needed");
				continue;
			}
			( (CFcGateProp*)pProp )->SetEventNav( nNavIndex, pNavigationMesh, pNavEventInfo[nNavIndex], false );
			continue;
		}
		if( pProp->IsBreakAndBlockWayProp() )
		{
			D3DXVECTOR2 Pos = pProp->GetPosV2();
			int nNavIndex = SearchNearNavIndex( &Pos, pNavigationMesh, pNavEventInfo, nNumNavEventInfo );
			if( nNavIndex == -1 )
			{
				BsAssert(0 && "BreakAndBlockWay Attribute Needed");
				continue;
			}
			( (CFcBreakAndBlockWayProp*)pProp )->SetEventNav( nNavIndex, pNavigationMesh, pNavEventInfo[nNavIndex], true );
			continue;
		}
	}
}

void CFcPropManager::SetBuildingAttrAtCrossAttr( int nXSize, int nYSize, BYTE* pPropAttr,
												int nAttrX, int nAttrY, int nRot, float fXRate, float fZRate, D3DXMATRIX* pMat, 
												int nAttrBufXSize, int nAttrBufYSize, BYTE* pAttr, BYTE *pAttrEx, bool bSet, int nAttrType )
{
	int i, j, k, m, n, size;
	//int x,y;	// mruete: prefix bug 599: moved these to the for loops to eliminate scope overlap
	int nIndex;

	D3DXVECTOR2 *Buf = new D3DXVECTOR2[ nXSize * nYSize ]; // 회전된 값을 저장하기 위해 미리 잡아둔 버퍼.

	float fBaseX = (nAttrX) * 100.f;
	float fBaseY = (nAttrY) * 100.f;
	for( int y = 0 ; y < nYSize ; ++y )
	{
		for( int x = 0 ; x < nXSize ; ++ x )
		{
			nIndex = x + y * nXSize;

			Buf[ nIndex ].x = fBaseX + ((float)x * 100.f ) + 50.f;
			Buf[ nIndex ].y = fBaseY - ((float)y * 100.f ) + 50.f;
		}
	}

	float fPosX = pMat->_41;
	float fPosZ = pMat->_43;

	size = nXSize * nYSize;

	int m_sx=nAttrBufXSize-2;
	int m_sy=nAttrBufYSize-2;
	int m_ex=1;
	int m_ey=1;

	for( i = 0 ; i < size ; ++i )
	{
		for( j = -1; j <= 1; j++) 
		for( k = -1; k <= 1; k++) 
		{
			float tx = ( Buf[i].x - fPosX + j * 33.f ) * fXRate;
			float ty = ( Buf[i].y - fPosZ + k * 33.f ) * fZRate;

			float fx = ( FCOS( nRot ) * tx ) - ( FSIN( nRot ) * ty );
			float fy = ( FSIN( nRot ) * tx ) + ( FCOS( nRot ) * ty );

			for( m = -1; m <= 1; m++)
			for( n = -1; n <= 1; n++)
			{
				int x = (int)((fx + fPosX + m * 16.f) / 100.f );
				int y = (int)((fy + fPosZ + n * 16.f) / 100.f );

				if( ( x < 2 ) || ( x > nAttrBufXSize-2 ) ) {	continue;	}
				if( ( y < 2 ) || ( y > nAttrBufYSize-2 ) ) {	continue;	}

				if( x < m_sx ) { m_sx = x; }
				if( y < m_sy ) { m_sy = y; }
				if( x > m_ex ) { m_ex = x; }
				if( y > m_ey ) { m_ey = y; }
				
				int nAddress = x + y * nAttrBufXSize;

				int nReadBit = -1;
				int nWriteBit;
				int attr = pPropAttr[ i ];

				if( attr == 0 )
					continue;
				else if( attr == 1 )
				{
					nWriteBit = ( nAttrType == PROP_TYPE_NORMAL ) ? 1 : 2;
				}
				else if( attr == 2 )
				{
					nReadBit = ( nAttrType == PROP_TYPE_NORMAL ) ? -1 : 4;
					nWriteBit = 2;
				}
				else if( attr == 5 )
				{
					nWriteBit = 16;
				}
				else 
					continue;

				if( bSet )
				{
					if( nReadBit < 0 || (pAttr[ nAddress ] & nReadBit) != 0 ) {	// 다리가 아니거나 , 다리면 빨간부분일 때에만 세팅해준다.
						if( (pAttrEx [ nAddress ] & 0x07) == 0 ) {
							pAttr[ nAddress ] |= nWriteBit;
						}
					}
				}
				else
				{
					if( nReadBit < 0) {
						if( nAttrType != PROP_TYPE_NORMAL && nAttrType != PROP_TYPE_FALLDOWN ) {	// 다리인경우 빨간부분만 지운다							
							continue;
						}
						else {
							nReadBit = nWriteBit;		// 일반적인 경우 세팅되있는 비트를 지운다
						}
					}
					if( (pAttr[ nAddress ] & nReadBit) != 0 ) {
						pAttr[ nAddress ] &= ~nWriteBit;
					}
				}
			}
		}
	}

	if( !bSet ) {
		// 남아 있는 대각선 속성들도 지워줍니다.
		for( int my = m_sy-1; my <= m_ey+1; my++)
		for( int mx = m_sx-1; mx <= m_ex+1; mx++) 
		{
			int nAddress = mx + my * nAttrBufXSize;
			int nAddress1, nAddress2;
			int nDiagonal = (pAttrEx [ nAddress ] & 0x07);
			switch( nDiagonal )
			{
			case 1:
				nAddress1 = (mx+1) + (my) * nAttrBufXSize;
				nAddress2 = (mx) + (my+1) * nAttrBufXSize;
	           
				if( ( pAttr [ nAddress1 ] & 0x02 ) == 0  &&
					( pAttr [ nAddress2 ] & 0x02 ) == 0  ) 
				{
					pAttrEx [ nAddress ] &= ~0x17;	// 히어로만 가는 속성까지 지움..
				}
				break;
			case 2:
				nAddress1 = (mx+1) + (my) * nAttrBufXSize;
				nAddress2 = (mx) + (my-1) * nAttrBufXSize;

				if( ( pAttr [ nAddress1 ] & 0x02 ) == 0  &&
					( pAttr [ nAddress2 ] & 0x02 ) == 0 ) 
				{
					pAttrEx [ nAddress ] &= ~0x17;
				}
				break;
			case 3:
				nAddress1 = (mx-1) + (my) * nAttrBufXSize;
				nAddress2 = (mx) + (my-1) * nAttrBufXSize;

				if( ( pAttr [ nAddress1 ] & 0x02 ) == 0  &&
					( pAttr [ nAddress2 ] & 0x02 ) == 0  ) 
				{
					pAttrEx [ nAddress ] &= ~0x17;
				}
				break;
			case 4:
				nAddress1 = (mx-1) + (my) * nAttrBufXSize;
				nAddress2 = (mx) + (my+1) * nAttrBufXSize;

				if( ( pAttr [ nAddress1 ] & 0x02 ) == 0 &&
					( pAttr [ nAddress2 ] & 0x02 ) == 0 ) 
				{
					pAttrEx [ nAddress ] &= ~0x17;
				}
				break;
			}
		}
	}

	delete [] Buf;
}


#define GRASS_SKININDEX_START	2900
#define GRASS_SKININDEX_NUM		200

#define BILLBOARD_INFO_FILENAME		"BillboardGrassInfo.bif"

void CFcPropManager::InitBillboardProps()
{
	float MapXSize, MapYSize;
	g_FcWorld.GetMapSize(MapXSize, MapYSize);

	int nInfoNum;
	GRASSBILLBOARDINFO Info[16];
	if( LoadBillboardGrassInfo( BILLBOARD_INFO_FILENAME, &nInfoNum, Info ) == false )
	{
		BsAssert( 0 && "Cannot found BillboardGrassInfo File." );
	}

	int nBillboardCount = g_FcWorld.GetMapInfo()->GetBillboardCount();
	// 몇번째 빌보드텍스쳐를 사용하는지 검색한다.
	int nCurIndex = -1;

	if( g_FcWorld.GetMapInfo()->GetBillboardDDSName() ) {
		for(int i=0;i<nInfoNum;++i) {
			if(_stricmp(g_FcWorld.GetMapInfo()->GetBillboardDDSName(), Info[i].cTexName)==0) {
				nCurIndex = i;
				break;
			}
		}
		if(nCurIndex<0) {
			BsAssert( 0 && "등록되지 않은 Billboard Texture입니다." );
		}
	}
	else
	{
		DebugString( "Billboad Texture cannot found\n" );
		nBillboardCount = 0;
	}

	if(nBillboardCount) {
		g_BsKernel.GetInstance().GetBillboardMgr()->InitBillBoard((int)MapXSize, (int)MapYSize, 
			g_FcWorld.GetMapInfo()->GetBillboardDDSName(), Info[nCurIndex].nNum, Info[nCurIndex].Data );
	}
	else { // 임시로 넣어줍니다. 구조를 바꿔 이 부분 코드를 고치겠습니다. by russiac
		g_BsKernel.GetInstance().GetBillboardMgr()->InitBillBoard((int)MapXSize, (int)MapYSize, 
			NULL, NULL, NULL );
	}
}

// Clipping type에 따른 처리 아직 없음
void CFcPropManager::GetVisibleProps( D3DXVECTOR2* pPos, float fRange, std::vector<CFcProp*>& vecProps )
{	
	m_pPropScanner->Find( pPos->x, pPos->y, fRange, vecProps );
}


void CFcPropManager::UpdateVisibleProps()
{
	if( !g_FCGameData.bDrawProp )
	{
		return;
	}
	std::vector<CFcProp*> vecProps;
	std::vector<int> vecStaticProps;
	m_pZoneManager->GetVisibleProp( vecProps, vecStaticProps );

	int nCnt = vecProps.size();
	for( int i=0; i<nCnt; i++ )
	{
		vecProps[i]->UpdateObject();
	}

	nCnt = vecStaticProps.size();
	for( int i=0; i<nCnt; i++ )
	{
		int nObjIndex = m_vecStaticObjs[ vecStaticProps[i] ].nStaticPropIndex;
		g_BsKernel.UpdateObject( nObjIndex );
	}

	nCnt = m_vecMovableProps.size();
	for( int i =0; i<nCnt; i++) 
	{
		m_vecMovableProps[i]->UpdateObject();
	}
	/*
	std::vector< CFcProp * > *pResult = NULL;
	m_pPropScanner->ExcuteFor( pPos->x, pPos->y, fRange, UpdatePropObject(), pResult );
	*/
}

#define CHECK_EVENT_NAV_RADIUS		5000.f

// Pos위치에 몇번째 NavEventMesh가 있는지 알려준다.
int CFcPropManager::SearchNearNavIndex( D3DXVECTOR2* Pos, NavigationMesh* pNavigationMesh, NAVEVENTINFO* pNavEventInfo, int nNumNavEventInfo )
{
	for( int i=0; i<nNumNavEventInfo; i++ )
	{
		BsAssert( pNavEventInfo[i].nNumNavIndex > 0 );
		NavigationCell* pCell = pNavigationMesh->Cell( pNavEventInfo[i].nStartIndex + pNavEventInfo[i].nNumNavIndex / 2 );
		D3DXVECTOR3 CellPos = pCell->Vertex( 0 );
		if( CellPos.x > Pos->x - CHECK_EVENT_NAV_RADIUS && CellPos.x < Pos->x + CHECK_EVENT_NAV_RADIUS 
			&& CellPos.z > Pos->y - CHECK_EVENT_NAV_RADIUS && CellPos.z < Pos->y + CHECK_EVENT_NAV_RADIUS  )
		{
			return i;
		}
	}
	return -1;
}


bool CFcPropManager::IsActiveProp( CFcProp* pProp )
{
	if( pProp->GetName() != NULL || 
		pProp->IsBridgeProp() || 
		pProp->IsGateProp() ||
		pProp->IsCollisionProp() ||
		pProp->IsClimbProp() ||
		pProp->IsCrushProp() ||
		pProp->IsDynamicProp() ||
		pProp->IsItemProp() ||
		pProp->IsPickUpProp() )
	{
		return true;
	}
	return false;
}


bool CFcPropManager::GeneratePropHeightMapFile( char* pSkinFileName, char* pFileName, int* pDebugMeshIndex )
{
	PropCollisionMeshInfo Info;
	char cMeshFileName[64];
	strcpy( cMeshFileName, pSkinFileName );
	char* pTempStr = strstr( cMeshFileName, "." );
	BsAssert( pTempStr );
	pTempStr[0] = NULL;
	strcat_s(cMeshFileName, _countof(cMeshFileName), "_cl.bm" ); //aleksger - safe string

	int nMeshIndex = g_BsKernel.LoadMesh( -1, g_BsKernel.GetFullName( cMeshFileName ) );
	if( pDebugMeshIndex )
	{
		*pDebugMeshIndex = nMeshIndex;
	}

	if( nMeshIndex == -1 )
	{
		BsAssert( 0 );
		return false;
	}
	//	int nObjectIndex = g_BsKernel.CreateStaticObjectFromSkin( pData->nSkinIndex );
	int nObjectIndex = g_BsKernel.CreateStaticObject( nMeshIndex );
	if( nObjectIndex < 0 )
	{
		BsAssert(0);
		return false; 
	}
	CBsObject* pObj = g_BsKernel.GetEngineObjectPtr( nObjectIndex );
	CCrossVector Cross;
	pObj->SetObjectMatrix( Cross );

	// 스케일 조절하면 제대로 안된다.
	Info.Initialize( nMeshIndex, nObjectIndex, 1.0f );

	AABB *pBox = (AABB *)g_BsKernel.SendMessage( nObjectIndex, BS_GET_BOUNDING_BOX );
// [PREFIX:beginmodify] 2006/2/25 junyash PS#5817 dereferencing NULL pointer
	BsAssert(pBox);
// [PREFIX:endmodify] junyash
	int nXSize = (int)( ( pBox->Vmax.x - pBox->Vmin.x + 180.f ) / 100.f );	// 여유분 180센티
	int nZSize = (int)( ( pBox->Vmax.z - pBox->Vmin.z + 180.f ) / 100.f );

	D3DXVECTOR2 StartPos = D3DXVECTOR2( pBox->Vmin.x - 90.f, pBox->Vmin.z - 90.f );

	PropHeightMap HeightMapInfo;
	HeightMapInfo.Initialize( nXSize, nZSize, &StartPos );

	for( int i=0; i<nZSize; i++ )
	{
		float fZ = StartPos.y + i * 100.f;
		for( int j=0; j<nXSize; j++ )
		{
			float fX = StartPos.x + j * 100.f;
			float fHeight = 0.f;;
			Info.GetHeight( fX, fZ, &fHeight, true ); 
			HeightMapInfo.SetHeight( j, i, fHeight );
		}
	}
	g_BsKernel.DeleteObject( nObjectIndex );

	if( HeightMapInfo.Save( g_BsKernel.GetFullName( pFileName ) ) == false )
		return false;

	return true;
}


bool CFcPropManager::LoadBillboardGrassInfo( char* pFileName, int* pInfoNum, GRASSBILLBOARDINFO* pInfo )
{
	char cBinFileName[MAX_PATH];
	char cScriptFileName[MAX_PATH];

	sprintf( cBinFileName, "%s%sb", g_BsKernel.GetCurrentDirectory(), pFileName );
	sprintf( cScriptFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), pFileName );

	TokenBuffer tb;

	if( tb.Load( cBinFileName ) == false ) {	
		BsAssert( 0 && "Binary 파일없음");
		return false;
	}

	int nIndex = -1;

	while( 1 )
	{
		if( tb.IsEnd() )
			break;

		if( tb.IsString() )
			++nIndex;
		else
			break;

		strcpy( pInfo[nIndex].cTexName, tb.GetString() );			++tb;
		pInfo[nIndex].nNum = tb.GetInteger();						++tb;
		for( int i=0; i<pInfo[nIndex].nNum; i++ )
		{
			pInfo[nIndex].Data[i].nIndex = tb.GetInteger();		++tb;

			pInfo[nIndex].Data[i].vecTexCoord[0].x = tb.GetReal();		++tb;	++tb;
			pInfo[nIndex].Data[i].vecTexCoord[0].y = tb.GetReal();		++tb;	++tb;
			pInfo[nIndex].Data[i].vecTexCoord[1].x = tb.GetReal();		++tb;	++tb;
			pInfo[nIndex].Data[i].vecTexCoord[1].y = tb.GetReal();		++tb;
		}
	}

	*pInfoNum = nIndex+1;
	return true;
}

void CFcPropManager::GetNearAlphaBlockingProps( float fX, float fY, float fRadius, std::vector<int>& vecNearAlphaProps )
{
	std::vector<CFcProp*> vecNearProps;
	m_pPropScanner->Find( fX, fY, fRadius, vecNearProps);

	int nCnt = vecNearProps.size();
	for( int i=0; i<nCnt; i++ )
	{
		if( vecNearProps[i]->IsAlphaBlocking() == false )
			continue;

		vecNearAlphaProps.push_back( vecNearProps[i]->GetEngineIndex() );
	}

	std::vector<int> vecStaticNearProps;
	m_pZoneManager->GetNearStaticProps( fX, fY, fRadius, vecStaticNearProps );

	// 스택틱 프랍은 알파 체크가 없잖아!!
/*
	nCnt = vecStaticNearProps.size();
	for( int i=0; i<nCnt; i++ )
	{
		if( m_vecStaticObjs[ vecStaticNearProps[i] ].
	}
	if(!vecNearProps[j]->IsAlphaBlocking())
		continue;
*/

	nCnt = vecStaticNearProps.size();
	for( int i=0; i<nCnt; i++ )
	{
		vecNearAlphaProps.push_back( m_vecStaticObjs[ vecStaticNearProps[i] ].nStaticPropIndex );
	}

}




void CFcPropManager::GetObjectListInRange( D3DXVECTOR3 *pPosition, float fRadius, std::vector<CFcProp *> &Objects )
{
	m_pPropScanner->Find( pPosition->x, pPosition->z, fRadius, Objects );
}

void CFcPropManager::RemoveProp( CFcProp *pProp )
{
	DebugString( "Remove Prop %d\n", pProp->GetEngineIndex() );

	for( int i=0; i<m_nPropNum; i++ ) {
		if( m_ppProps[i] == pProp ) {
			bool bRet = m_pPropScanner->RemoveObject( pProp );
			BsAssert( bRet );
			m_ppProps[i] = NULL;
			break;
		}
	}

	int nCnt = m_vecActiveProps.size();
	for( int i=0; i<nCnt; i++ ) {
		if( m_vecActiveProps[i] == pProp ) {
			bool bRet = m_pActivePropScanner->RemoveObject( pProp );
			BsAssert( bRet );
			m_vecActiveProps.erase( m_vecActiveProps.begin() + i );
			break;
		}
	}

	nCnt = m_vecMovableProps.size();
	for( int i=0; i<nCnt; i++ ) {
		if( m_vecMovableProps[i] == pProp ) {			
			m_vecMovableProps.erase( m_vecMovableProps.begin() + i );
			break;
		}
	}

	m_pZoneManager->RemoveProp( pProp );
	delete pProp;
}



void CFcPropManager::GetItemList( std::vector<CFcProp*>& vecItems )
{
	int nCnt = m_vecActiveProps.size();
	for( int i=0; i<nCnt; i++ )
	{
		CFcProp* pProp = m_vecActiveProps[i];
		if( !pProp->IsItemProp() )
			continue;

		vecItems.push_back( pProp );
	}
}


void CFcPropManager::InitZoneData( int nXSize, int nYSize, int nPropZoneSize )
{
	m_pZoneManager->Initialize( nXSize, nYSize, nPropZoneSize );
}




void CFcPropManager::SetZoneData( int nIndex, Box3* pBox )
{
	m_pZoneManager->SetZoneData( nIndex, pBox );
}
