#include "stdafx.h"
#include "FcProp.h"
#include "BsKernel.h"
#include "BsPhysicsMgr.h"
#include "BsClothObject.h"
#include "CrossVector.h"
#include "FcWorld.h"
#include "FcPropManager.h"
#include "BsMesh.h"
#include "Ray3.h"
#include "Box3.h"
#include "IntLin3Box3.h"
#include "FcGlobal.h"
#include "FcTroopObject.h"
#include "BSFileManager.h"
#include "navigationcell.h"
#include "navigationmesh.h"
#include "FcHeroObject.h"
#include "FcPhysicsLoader.h"
#include "FcInterfaceManager.h"
#include "BsSinTable.h"
#include "FcCommon.h"
#include ".\\data\\FxList.h"
#include "FcItem.h"
#include "FcSoundManager.h"
#include "FcCameraObject.h"

#ifdef _XBOX
#include <..\..\Source\Modules\collision\collision.h>
#endif


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


PropHeightMap::PropHeightMap()
{
	m_StartPos = D3DXVECTOR2( 0.f, 0.f );
	m_nXSize = m_nYSize = 0;
	m_pMap = NULL;
}


PropHeightMap::~PropHeightMap()
{
	SAFE_DELETEA( m_pMap );
}


bool PropHeightMap::Initialize( int nXSize, int nYSize, D3DXVECTOR2* pPos )
{
	m_pMap = new float[nXSize * nYSize];
	m_nXSize = nXSize;
	m_nYSize = nYSize;
	m_StartPos = *pPos;

	return true;
}

void PropHeightMap::SetHeight( int nX, int nY, float fHeight )
{
	BsAssert( nX >=0 && nX < m_nXSize );
	BsAssert( nY >=0 && nY < m_nYSize );

	m_pMap[ nX + nY * m_nXSize ] = fHeight;
}


bool PropHeightMap::GetHeight( float fX, float fY, const D3DXMATRIX* pIMat, float* pHeight )
{
	D3DXVECTOR3 Pos = D3DXVECTOR3( fX, 0.f, fY );
	D3DXVECTOR3 NewPos;

	D3DXVec3TransformCoord( &NewPos, &Pos, pIMat );

	//	fX  -= pIMat->_41;
	//	fY  -= pIMat->_43;

	int nX = (int)((NewPos.x - m_StartPos.x)/100.f);
	int nY = (int)((NewPos.z - m_StartPos.y)/100.f);

	if( nX >= 0 && nX < m_nXSize && nY >= 0 && nY < m_nYSize )
	{
		*pHeight = -(pIMat->_42) + m_pMap[nX + nY * m_nXSize ];
	}
	else
	{
		*pHeight = 0.f;
		return false;
	}
	return true;
}


bool PropHeightMap::Save( char* pFileName )
{
	BFileStream Stream( pFileName, BFileStream::create );
	if( Stream.Valid() == false )
	{
		BsAssert( 0 && "Cannot load PHM file" );
		return false;
	}

	Stream.Write( &m_StartPos.x, sizeof(float), 4 );
	Stream.Write( &m_StartPos.y, sizeof(float), 4 );
	Stream.Write( &m_nXSize, sizeof(int), 4 );
	Stream.Write( &m_nYSize, sizeof(int), 4 );

	Stream.Write( m_pMap, sizeof(float) * m_nXSize * m_nYSize, 4 );

	return true;
}

bool PropHeightMap::Load( char* pFileName )
{
	SAFE_DELETEA( m_pMap );

	BFileStream Stream( pFileName, BFileStream::openRead );
	if( Stream.Valid() == false )
	{
		return false;
	}

	Stream.Read( &m_StartPos.x, sizeof(float), 4 );
	Stream.Read( &m_StartPos.y, sizeof(float), 4 );
	Stream.Read( &m_nXSize, sizeof(int), 4 );
	Stream.Read( &m_nYSize, sizeof(int), 4 );

	m_pMap = new float[m_nXSize * m_nYSize];

	Stream.Read( m_pMap, sizeof(float) * m_nXSize * m_nYSize, 4 );

	return true;
}


PropCollisionMeshInfo::PropCollisionMeshInfo()
{
	m_pVertexBuf = NULL;
	m_nVertexCount = 0;
	m_pFaceBuf = NULL;
	m_nFaceCount = 0;
	m_pCheckFace = NULL;

	m_fBoundingDistSq = 0.f;
}

PropCollisionMeshInfo::~PropCollisionMeshInfo()
{
	SAFE_DELETEA( m_pVertexBuf );
	SAFE_DELETEA( m_pFaceBuf );
	SAFE_DELETEA( m_pCheckFace );
}

void PropCollisionMeshInfo::Initialize( int nMeshIndex, int nObjectIndex, float fMaxScaleSq )
{
	BsAssert( nMeshIndex >= 0 );
	CBsMesh *pMesh = g_BsKernel.Get_pMesh( nMeshIndex );
	pMesh->GetStreamBuffer( &m_pVertexBuf, &m_pFaceBuf, m_nVertexCount, m_nFaceCount );

	m_Mat = (D3DXMATRIX*)g_BsKernel.GetEngineObjectPtr( nObjectIndex )->GetObjectMatrix();

	AABB* pBoundingBox = (AABB*)pMesh->GetBoundingBox();
	D3DXVECTOR2 Dir;
	Dir.x = pBoundingBox->Vmax.x;
	if( Dir.x < -pBoundingBox->Vmin.x )
		Dir.x = -pBoundingBox->Vmin.x;

	Dir.y = pBoundingBox->Vmax.z;
	if( Dir.y < -pBoundingBox->Vmin.z )
		Dir.y = -pBoundingBox->Vmin.z;

	Dir = Dir * sqrt(fMaxScaleSq);

	m_fBoundingDistSq = D3DXVec2LengthSq( &Dir );

	for( int i=0; i<m_nVertexCount; i++ )
	{
		D3DXVECTOR3 Pos;
		D3DXVec3TransformCoord( &Pos, &(m_pVertexBuf[i]), m_Mat );
		m_pVertexBuf[i] = Pos;
	}

	m_pCheckFace = new bool[m_nFaceCount];

	int w = 0;
	for( int i=0; i<m_nFaceCount; ++i )
	{
		D3DXVECTOR3* pPos1 = &(m_pVertexBuf[ m_pFaceBuf[w] ]);
		D3DXVECTOR3* pPos2 = &(m_pVertexBuf[ m_pFaceBuf[w+1] ]);
		D3DXVECTOR3* pPos3 = &(m_pVertexBuf[ m_pFaceBuf[w+2] ]);
		w += 3;

		D3DXVECTOR3 Normal;
		D3DXVECTOR3 Dir1 = *pPos2 - *pPos1;
		D3DXVec3Normalize( &Dir1, &Dir1 );
		D3DXVECTOR3 Dir2 = *pPos3 - *pPos2;
		D3DXVec3Normalize( &Dir2, &Dir2 );
		D3DXVec3Cross( &Normal, &Dir1, &Dir2 );
		D3DXVec3Normalize( &Normal, &Normal );

		float fD = D3DXVec3Dot( &Normal, &D3DXVECTOR3( 0.f, 1.f, 0.f ) );
		if( fD <= 0.f )
		{
			m_pCheckFace[i] = false;
		}
		else
		{
			m_pCheckFace[i] = true;
		}
	}
}

bool PropCollisionMeshInfo::GetHeight( float fX, float fY, float* pHeight, bool bCheckFace )
{
	return false;
}
/// Extension

PropCollisionMeshInfoExt::PropCollisionMeshInfoExt()
{
	m_nStartTriIndex = -1;
	m_nTriCount = 0;
}

PropCollisionMeshInfoExt::~PropCollisionMeshInfoExt()
{
	if( m_nStartTriIndex != -1 ) {

		AABBTree &aabbTree = g_FcWorld.GetPropManager()->GetAABBTree();

		aabbTree.EraseCollisionInfo( m_nStartTriIndex, m_nTriCount );
		m_nStartTriIndex = -1;
	}
}

void PropCollisionMeshInfoExt::Initialize( int nMeshIndex, int nObjectIndex, float fMaxScaleSq )
{
	BsAssert( nMeshIndex >= 0 );
	CBsMesh *pMesh = g_BsKernel.Get_pMesh( nMeshIndex );

	D3DXVECTOR3* pVertexBuf;
	int nVertexCount;
	WORD* pFaceBuf;
	int nFaceCount;

	pMesh->GetStreamBuffer( &pVertexBuf, &pFaceBuf, nVertexCount, nFaceCount );
	D3DXMATRIX *pMatrix = (D3DXMATRIX*) g_BsKernel.GetEngineObjectPtr( nObjectIndex )->GetObjectMatrix();

	for( int i=0; i<nVertexCount; i++ )
	{
		D3DXVec3TransformCoord( &pVertexBuf[i], &(pVertexBuf[i]), pMatrix );
	}
	
	AABBTree &aabbTree = g_FcWorld.GetPropManager()->GetAABBTree();

	for( int i = 0; i < nFaceCount; ++i )
	{
		CollisionTriangleInfo TriangleInfo;

		TriangleInfo.p[0] = pVertexBuf[ pFaceBuf[i*3+0] ];
		TriangleInfo.p[1] = pVertexBuf[ pFaceBuf[i*3+1] ];
		TriangleInfo.p[2] = pVertexBuf[ pFaceBuf[i*3+2] ];

		D3DXVECTOR3 *pos = TriangleInfo.p;

		TriangleInfo.fMinX = BsMin(BsMin( pos[0].x,  pos[1].x), pos[2].x);
		TriangleInfo.fMinZ = BsMin(BsMin( pos[0].z,  pos[1].z), pos[2].z);
		TriangleInfo.fMaxX = BsMax(BsMax( pos[0].x,  pos[1].x), pos[2].x);
		TriangleInfo.fMaxZ = BsMax(BsMax( pos[0].z,  pos[1].z), pos[2].z);
		TriangleInfo.fCenterX = ( TriangleInfo.fMinX + TriangleInfo.fMaxX ) * 0.5f;
		TriangleInfo.fCenterZ = ( TriangleInfo.fMinZ + TriangleInfo.fMaxZ ) * 0.5f;
		
		int nIndex = aabbTree.AddCollisionInfo( TriangleInfo );		
		if( m_nStartTriIndex == -1 ) {
			m_nStartTriIndex = nIndex - 1;
		}
	}
	m_nTriCount = nFaceCount;

	SAFE_DELETEA( pVertexBuf );
	SAFE_DELETEA( pFaceBuf );
}

CFcProp::CFcProp()
{
	m_nObjectIndex = -1;
	m_nTypeDataIndex = -1;
	m_Type = PROP_TYPE_NORMAL;
	m_Attr = 0;
	m_pNamedData = NULL;
	m_bMoveUpdate = false;
	m_bDelete = false;
	SetFxTemplate(0,-1);
	SetFxTemplate(1,-1);
	SetFxIndex(0,-1);
	SetFxIndex(1,-1);
	m_nPropSoundMaterial = -1;
	m_pCollisionMeshInfo = NULL;
	m_bProcess = true;
#ifdef _SHOW_COLLISION_PROP
	m_nCollisionIndex = -1;
#endif
}

CFcProp::~CFcProp()
{
	if( m_nObjectIndex != -1 )
	{
		g_BsKernel.DeleteObject( m_nObjectIndex );
		m_nObjectIndex = -1;
	}
	SAFE_DELETEA( m_pNamedData );

	for( int i = 0 ; i < 2 ; i++ )
	{
		if( GetFxIndex(i) != -1 )
		{
			g_BsKernel.DeleteObject( GetFxIndex(i) );
			SetFxIndex(i, -1);
		}
	}

	for( int i = 0 ; i < 2 ; i++ )
	{
		if( GetFxTemplate(i) != -1 )
		{
			g_BsKernel.ReleaseFXTemplate( GetFxTemplate(i) );
			SetFxTemplate(i, -1);
		}
	}
	DebugString( "~CFcProp %d\n", m_nObjectIndex );
	SAFE_DELETE( m_pCollisionMeshInfo );
}


void CFcProp::Initialize( int nTypeDataIndex, PROPTYPEDATA* pData, char* pName, D3DXVECTOR3* pPos, DWORD dwRotAndScale, DWORD* pParam, char* pParamString )
{
	int nObjectIndex = g_BsKernel.CreateStaticObjectFromSkin( pData->nSkinIndex );
	if( nObjectIndex < 0 )
	{
		BsAssert(0);
		return; 
	}
	// 맵툴의 공식대로 회전, 스케일 조절
	CBsObject* pObj = g_BsKernel.GetEngineObjectPtr( nObjectIndex );
	InitCrossVec( pPos, dwRotAndScale, &m_Cross );
	pObj->SetEntireObjectMatrix( m_Cross );

	g_BsKernel.SendMessage(nObjectIndex, BS_SHADOW_CAST, pData->nShadowType );

	int nAttr = MakeAttr( pData );
	SetInitData( nObjectIndex, nTypeDataIndex, pData->nType, nAttr, pName, m_Cross.m_PosVector, pParam );

	g_BsKernel.chdir("fx");
	for( int i = 0 ; i < 2 ; i++ )
	{
		if( pData->cFXName[i][0] )
		{
			BsAssert( GetFxTemplate(i) == -1 );
			char szFxFilePath[256];
			sprintf(szFxFilePath,"%s%s",g_BsKernel.GetCurrentDirectory(), pData->cFXName[i] );
			int nFx = -1;
			nFx = g_BsKernel.FindFXTemplate(szFxFilePath);
			if( nFx != -1 ){
				g_BsKernel.AddFXTemplateRef(nFx);
			}
			else{
				DebugString("Find Not Prop Fx File : %s \n",szFxFilePath);
				BsAssert(0);
			}
			SetFxTemplate(i, nFx);
		}//if
	}// for
	g_BsKernel.chdir("..");

	if( GetFxTemplate(0) != -1 )
	{
		BsAssert( GetFxIndex(0) == - 1);
		SetFxIndex(0, g_BsKernel.CreateFXObject( GetFxTemplate(0) ) );
		g_BsKernel.SetFXObjectState( GetFxIndex(0), CBsFXObject::PLAY, -99 );
		g_BsKernel.UpdateObject( GetFxIndex(0) , m_Cross);
	}
	m_nPropSoundMaterial = pData->nSndType;
	if( pData->nCollisionMeshIndex > 0 )
	{
		m_pCollisionMeshInfo = new PropCollisionMeshInfoExt;
		float fMaxScale = 0.f;
/*
		float fMaxScale = D3DXVec3LengthSq( &(m_Cross.m_XVector) );
		float fTempScale = D3DXVec3LengthSq( &(m_Cross.m_YVector) );
		if( fTempScale > fMaxScale ) fMaxScale = fTempScale;
		fTempScale = D3DXVec3LengthSq( &(m_Cross.m_ZVector) );
		if( fTempScale > fMaxScale ) fMaxScale = fTempScale;
*/
		m_pCollisionMeshInfo->Initialize( pData->nCollisionMeshIndex, m_nObjectIndex, fMaxScale );

#ifdef _SHOW_COLLISION_PROP
		m_nCollisionIndex = g_BsKernel.CreateStaticObject( pData->nCollisionMeshIndex );

		static int nMaterialIndex = g_BsKernel.LoadMaterial("material\\default.fx");
		BsAssert(nMaterialIndex != -1);
		int nSubMeshCount = g_BsKernel.SendMessage(m_nCollisionIndex, BS_GET_SUBMESH_COUNT);
		for (int i=0; i<nSubMeshCount; ++i)
		{
			g_BsKernel.SendMessage(m_nCollisionIndex, BS_SET_SUBMESH_MATERIAL, i, nMaterialIndex);
		}		
		g_BsKernel.GetEngineObjectPtr( m_nCollisionIndex )->SetEntireObjectMatrix( m_Cross );
#endif
	}
}

void CFcProp::Process()
{	
	if( GetFxIndex(0) != -1 )
	{
		g_BsKernel.UpdateObject( GetFxIndex(0) , m_Cross);
	}
}


void CFcProp::UpdateObject()
{
	CSmartPtr<CFcCameraObject> hCamera = CFcBaseObject::GetCameraObjectHandle( 0 );
	D3DXVECTOR3 Dir = hCamera->GetPos() - GetPos();
	PROPTYPEDATA* pData = g_FcWorld.GetPropTypeData_( m_nTypeDataIndex );
	/*
	float fDist = D3DXVec3LengthSq( &Dir );
	switch( pData->nClippingType )
	{
	case 0:
	if( fDist > 6000.f * 6000.f )
	{
	return;
	}
	break;
	case 1:
	if( fDist > 8000.f * 8000.f )
	{
	return;
	}
	break;
	case 2:
	break;
	}
	*/
	g_BsKernel.UpdateObject( m_nObjectIndex );	

#ifdef _SHOW_COLLISION_PROP
	if( m_nCollisionIndex != -1)
		g_BsKernel.UpdateObject( m_nCollisionIndex );
#endif
}

void CFcProp::SetPos( D3DXVECTOR3* Pos)
{
	if( m_vecOldPos.x != Pos->x || m_vecOldPos.z != Pos->z ) 
	{
		m_vecOldPos = m_vecPos;
		m_vecPos = *Pos;
		m_bMoveUpdate = true;
	}
}

AABB* CFcProp::GetBoundingBox()
{
	return (AABB *)g_BsKernel.SendMessage( m_nObjectIndex, BS_GET_BOUNDING_BOX );

}

void CFcProp::CalculateRadius()
{
	AABB *pBox;

	pBox = GetBoundingBox();
	BsAssert( pBox );	// mruete: prefix bug 583: added assert

	m_fRadius = ( ( pBox->Vmax.x - pBox->Vmin.x ) + ( pBox->Vmax.z - pBox->Vmin.z ) ) / 4;
}


const D3DXMATRIX& CFcProp::GetMat()
{
	const D3DXMATRIX* Mat = g_BsKernel.GetEngineObjectPtr( m_nObjectIndex )->GetObjectMatrix();
	return *Mat;
}

void CFcProp::GetBox3( Box3 & B )
{
	D3DXMATRIX *pMatrix = g_BsKernel.GetEngineObjectPtr( m_nObjectIndex )->GetObjectMatrix();
	AABB *pBoundingBox = GetBoundingBox();

	D3DXVECTOR3	tmpVec = *(D3DXVECTOR3*)&pBoundingBox->GetExtent();
	B.E[0] = tmpVec.x;
	B.E[1] = tmpVec.y;
	B.E[2] = tmpVec.z;

	B.C = pBoundingBox->GetCenter();
	D3DXVec3TransformCoord( ( D3DXVECTOR3 * )&B.C, ( D3DXVECTOR3 * )&B.C, m_Cross );

	memcpy( &( B.A[ 0 ] ), &pMatrix->_11, sizeof( D3DXVECTOR3 ) );
	memcpy( &( B.A[ 1 ] ), &pMatrix->_21, sizeof( D3DXVECTOR3 ) );
	memcpy( &( B.A[ 2 ] ), &pMatrix->_31, sizeof( D3DXVECTOR3 ) );
}

bool CFcProp::GetHeight( float fX, float fY, float* pHeight )
{
	BsAssert( !IsClimbProp() );
	if( IsCollisionProp() )
	{
		PropCollisionMeshInfo* pInfo = GetCollsionMeshInfo();

		// BsAssert( pInfo && "Cannot found collision mesh" );
		PROPTYPEDATA* pData = g_FcWorld.GetPropTypeData_( m_nTypeDataIndex );

		if( pInfo == NULL )
		{

			// DebugString( "%s prop is not collision mesh\n", pData->cName );
			return false;
		}

		return pInfo->GetHeight( fX, fY, pHeight );
	}
	return false;
}

void CFcProp::DebugCollisionPropRender()
{
#ifndef _XBOX
	if( !IsCollisionProp() && !IsClimbProp() )
		return;
#endif

}

int CFcProp::GetBookingNum()
{
	BsAssert( m_nObjectIndex >= 0 );
	return g_BsKernel.SendMessage( m_nObjectIndex, BS_GET_BOOKING_COUNT );
}

D3DXVECTOR3* CFcProp::GetBookingPos( int nIndex )
{
	return ( D3DXVECTOR3 * )g_BsKernel.SendMessage( m_nObjectIndex, BS_GET_BOOKING_POSITION, nIndex );
}


void CFcProp::InitCrossVec( D3DXVECTOR3* pPos, DWORD dwRotAndScale, CCrossVector* pCross )
{
	int nRot   = GET_ROTATE( dwRotAndScale );
	int nXRate = GET_XSCALE( dwRotAndScale );
	int nYRate = GET_YSCALE( dwRotAndScale );
	int nZRate = GET_ZSCALE( dwRotAndScale );

	float fXScale = (float)(0.5f + ( 0.01 * (float)nXRate ));
	float fYScale = (float)(0.5f + ( 0.01 * (float)nYRate ));
	float fZScale = (float)(0.5f + ( 0.01 * (float)nZRate ));

	pCross->m_XVector*=fXScale;
	pCross->m_YVector*=fYScale;
	pCross->m_ZVector*=fZScale;

	nRot = nRot * 4;

	pCross->RotateYaw( nRot );
	pCross->m_PosVector = *pPos;

	pCross->m_XVector*=fXScale;
	pCross->m_ZVector*=fZScale;
}




void CFcProp::SetInitData( int nObjectIndex, int nTypeDataIndex, int Type, int nAttr, char* pName, D3DXVECTOR3 vecPos, DWORD* pParam, char* pParamString )
{
	m_nObjectIndex = nObjectIndex;
	m_nTypeDataIndex = nTypeDataIndex;
	m_Type = (PROP_TYPE)Type;
	m_Attr = nAttr;
	m_bProcess = ( m_Attr & PROP_ATTR_NO_PROCESS ) == 0 ;

	if( pName[0] != NULL )
	{
		BsAssert( m_pNamedData == NULL );
		m_pNamedData = new NAMED_PROP_DATA;
		BsAssert( strlen( pName ) < 64 );
		strcpy( m_pNamedData->cName, pName );
		if( pParam ) 
			memcpy( m_pNamedData->dwParam, pParam, sizeof(DWORD) * PROP_PARAM_MAX );

		if(pParamString)
		{
			BsAssert( strlen(pParamString) < 64 );
			strcpy( m_pNamedData->cParamStr, pParamString );
		}
	}

	m_vecPos = vecPos;
	m_vecOldPos = vecPos;
	CalculateRadius();
}


PropHeightMap* CFcProp::GetHeightMapInfo()
{
	PROPTYPEDATA* pData = g_FcWorld.GetPropTypeData_( m_nTypeDataIndex );
	return &(pData->HeightMapInfo);
}

int CFcProp::MakeAttr( PROPTYPEDATA* pData )
{
	int nAttr = 0;
	if( pData->bCrush )
		nAttr |= PROP_ATTR_CRUSH;
	if( pData->bCollosionMesh )
		nAttr |= PROP_ATTR_COLLISION;
	if( pData->bAlphaBlocking )
		nAttr |= PROP_ATTR_ALPHA_BLOCKING;
	if( pData->bDynamic )
		nAttr |= PROP_ATTR_DYNAMIC;
	if( pData->bClimb)
		nAttr |= PROP_ATTR_CLIMB;

	// 속도 땜시 세팅
	if( (nAttr & ~PROP_ATTR_ALPHA_BLOCKING) == 0 )
	{
		if( pData->cFXName[0][0] == NULL && pData->cFXName[1][0] == NULL )
		{
			nAttr |= PROP_ATTR_NO_PROCESS;
		}
	}

	return nAttr;
}


CFcAnimationProp::CFcAnimationProp()
{
	m_fCurFrame = 0.f;
}

CFcAnimationProp::~CFcAnimationProp()
{

}

void CFcAnimationProp::Initialize( int nTypeDataIndex, PROPTYPEDATA* pData, char* pName, D3DXVECTOR3* pPos, DWORD dwRotAndScale, DWORD* pParam, char* pParamString )
{
	int nObjectIndex = g_BsKernel.CreateAniObjectFromSkin( pData->nSkinIndex, pData->nAniIndex );
	m_nAniLength = g_BsKernel.GetAniLength( nObjectIndex, 0 );

	m_fCurFrame = (float)Random( m_nAniLength );

	if( nObjectIndex < 0 )
	{
		BsAssert(0);
		return; 
	}
	// 맵툴의 공식대로 회전, 스케일 조절
	CBsObject* pObj = g_BsKernel.GetEngineObjectPtr( nObjectIndex );
	InitCrossVec( pPos, dwRotAndScale, &m_Cross );
	pObj->SetEntireObjectMatrix( m_Cross );

	//	if( pData->nShadowType == BS_SHADOW_NONE )
	//	{
	//		g_BsKernel.SendMessage( nObjectIndex, BS_SHADOW_CAST, 0 );
	//	}
	g_BsKernel.SendMessage(nObjectIndex, BS_SHADOW_CAST, pData->nShadowType );

	int nAttr = MakeAttr( pData );
	SetInitData( nObjectIndex, nTypeDataIndex, pData->nType, nAttr, pName, m_Cross.m_PosVector, pParam );

	g_BsKernel.chdir("fx");
	for( int i = 0 ; i < 2 ; i++ )
	{
		if( pData->cFXName[i][0] )
		{
			BsAssert( GetFxTemplate(i) == -1 );
			char szFxFilePath[256];
			sprintf(szFxFilePath,"%s%s",g_BsKernel.GetCurrentDirectory(), pData->cFXName[i] );
			int nFx = -1;
			nFx = g_BsKernel.FindFXTemplate(szFxFilePath);
			if( nFx != -1 ){
				g_BsKernel.AddFXTemplateRef(nFx);
			}
			else{
				DebugString("Find Not Prop Fx File : %s \n",szFxFilePath);
				BsAssert(0);
			}
			SetFxTemplate(i, nFx);
		}//if
	}// for
	g_BsKernel.chdir("..");

	if( GetFxTemplate(0) != -1 )
	{
		BsAssert( GetFxIndex(0) == - 1);
		SetFxIndex(0, g_BsKernel.CreateFXObject( GetFxTemplate(0) ) );
		g_BsKernel.SetFXObjectState( GetFxIndex(0), CBsFXObject::PLAY, -99 );
		g_BsKernel.UpdateObject( GetFxIndex(0) , m_Cross);
	}
	m_nPropSoundMaterial = pData->nSndType;
	if( pData->nCollisionMeshIndex > 0 )
	{
		m_pCollisionMeshInfo = new PropCollisionMeshInfoExt;

		float fMaxScale = D3DXVec3LengthSq( &(m_Cross.m_XVector) );
		float fTempScale = D3DXVec3LengthSq( &(m_Cross.m_YVector) );
		if( fTempScale > fMaxScale ) fMaxScale = fTempScale;
		fTempScale = D3DXVec3LengthSq( &(m_Cross.m_ZVector) );
		if( fTempScale > fMaxScale ) fMaxScale = fTempScale;

		m_pCollisionMeshInfo->Initialize( pData->nCollisionMeshIndex, m_nObjectIndex, fMaxScale );
	}	
}

void CFcAnimationProp::Process()
{
	CFcProp::Process();

	++m_fCurFrame;
	if( (int)m_fCurFrame >= m_nAniLength )
	{
		m_fCurFrame = 0;
	}

	g_BsKernel.SendMessage( m_nObjectIndex, BS_SETCURRENTANI, 0, ( DWORD )&m_fCurFrame ); 
}



//////////////////////////
// CFcDynamicProp class

CFcDynamicProp::CFcDynamicProp()
{
	m_pActor = NULL;
	m_nSoundHandle = -1;
	m_bMove = false;
	m_nAlphaTick = 0;
	m_nTeam = -1;
	m_bEnable = true;
	m_nAddForceRemain = 0;
}

CFcDynamicProp::~CFcDynamicProp()
{
	if( m_pActor ) 
	{
		delete m_pActor;
		m_pActor = NULL;
	}
}

void CFcDynamicProp::Initialize( int nTypeDataIndex, PROPTYPEDATA* pData, char* pName, D3DXVECTOR3* pPos, DWORD dwRotAndScale )
{
	CFcProp::Initialize( nTypeDataIndex, pData, pName, pPos, dwRotAndScale );

	// 돌이 지형속에 파묻히는거 보정해줌
	float fHeight =g_BsKernel.GetLandHeight(m_Cross.m_PosVector.x, m_Cross.m_PosVector.z);
	if( fHeight > m_Cross.m_PosVector.y - m_fRadius) {
		m_Cross.m_PosVector.y = fHeight + (m_fRadius * 0.9f);
	}

	m_pActor = CBsPhysicsStone::Create(m_fRadius, *m_Cross);
	m_pActor->SetGroup("DynamicProp");	

	SetEnable( false );
}


void CFcDynamicProp::Process()
{
	if( !m_pActor ) return;
	if( !m_bEnable ) return;
	NxActor *pNxActor = m_pActor->GetActor();
	if( !pNxActor ) return;

	if( m_nAddForceRemain )
	{
		m_nAddForceRemain--;
	}

	D3DXMATRIX Mat;
	D3DXVECTOR3 Pos;

	m_pActor->GetGlobalPose( Mat );
	Pos = *(D3DXVECTOR3*)(&Mat._41);
	SetPos(&Pos);
	m_Cross = Mat;

	if( m_bMove == true ) {
		if( m_nAlphaTick > 0 ) {
			--m_nAlphaTick;
			float fAlpha = (float)m_nAlphaTick / 40.f;
			g_BsKernel.SendMessage( m_nObjectIndex, BS_ENABLE_OBJECT_ALPHABLEND, 1 );
			g_BsKernel.SendMessage( m_nObjectIndex, BS_SET_OBJECT_ALPHA, (DWORD)&fAlpha );
			if( m_nAlphaTick == 0 ) {
//				Delete();
				/*m_bMove = false;
				SetEnable( false );
				*/
			}
			return;
		}

		//D3DXVECTOR3 Dir = m_pActor->GetVelocity();
		//if( D3DXVec3LengthSq( &Dir ) < 0.03f )
		if( pNxActor->isSleeping() )
		{
			m_nAlphaTick = 40;
			return;
		}

		if( GetFxIndex(1) != -1 )
		{
			g_BsKernel.UpdateObject( GetFxIndex(1) , m_Cross);
		}
	}
	else
	{
		D3DXVECTOR3 Dir = m_pActor->GetVelocity();
		Dir.y = 0;
		if( D3DXVec3LengthSq( &Dir ) > 1.f )
		{
			m_bMove = true;
			if(GetFxTemplate(1)  != -1)
			{
				BsAssert( GetFxIndex(1) == -1 );
				SetFxIndex(1, g_BsKernel.CreateFXObject( GetFxTemplate(1) ) );			
			}
			if(GetFxIndex(1) != -1)
			{
				g_BsKernel.SetFXObjectState( GetFxIndex(1), CBsFXObject::PLAY );
				g_BsKernel.UpdateObject( GetFxIndex(1) , m_Cross);
			}
		}
		if( GetFxIndex(0) != -1 )
		{
			g_BsKernel.UpdateObject( GetFxIndex(0) , m_Cross);
		}
	}


	D3DXVECTOR3 Vel = m_pActor->GetVelocity();
	float fVel = D3DXVec3Length(&Vel);
	D3DXVECTOR3  Dir = Vel * (1.0f / fVel);

	if( fVel > 2.f  )
	{
		int nDamage;
		GameObjHandle Identity;

		nDamage = 200;
		if( g_FcWorld.GetHeroHandle()->GetTeam() == m_nTeam )
		{
			Identity = g_FcWorld.GetHeroHandle();
		}
		else if( g_FcWorld.GetReducePhysicsDamage() )
		{
			nDamage -= (int)( nDamage * ( g_FcWorld.GetReducePhysicsDamage() / 100.0f ) );
		}

		_FC_RANGE_DAMAGE rd;

		rd.fYMax = m_Cross.m_PosVector.y + 200.0f;
		rd.Attacker = Identity;
		rd.pPosition = &Pos;
		rd.fRadius = 200.0f;
		rd.nPower = nDamage;
		rd.nPushY = 15; 
		rd.nPushZ = -20;
		rd.nTeam = m_nTeam;
		rd.nNotGenOrb = GENERATE_ORB_RED;
		rd.bDisableDynamicProp = true;
		rd.nHitRemainFrame = 40;
		g_FcWorld.GiveRangeDamage( rd );		
		Identity.Identity();
		rd.Attacker = Identity;
		rd.nPower = 0;
		rd.nTeam = -1;
		rd.nNotGenOrb = GENERATE_ORB_RED;
		g_FcWorld.GiveRangeDamage( rd );

		if(m_nSoundHandle == -1){
			m_nSoundHandle = g_pSoundManager->Play3DSound(this,SB_COMMON,"ROCK_ROLL_S",&Pos);
		}
		else
		{
			if( g_pSoundManager->IsOwner( this,m_nSoundHandle ) ) {			
				g_pSoundManager->SetEmitterPos( m_nSoundHandle,&Pos );
			}
		}
	}
	else{
		if(m_nSoundHandle != -1){
			g_pSoundManager->Stop3DSound(m_nSoundHandle);
			m_nSoundHandle = -1;
		}
	}

	if(fVel > 2.f ){
		if(Random(5000) < 300)
		{
			CCrossVector Cross;
			Cross.m_PosVector = Pos;
			Cross.m_PosVector -= Dir * m_fRadius;
			Cross.m_PosVector.y -= m_fRadius * 0.7f;
			g_BsKernel.CreateParticleObject(23,false,false,Cross);
		}
	}
}

void CFcDynamicProp::UpdateObject()
{
	if( m_nObjectIndex > 0 )
	{
		g_BsKernel.UpdateObject( m_nObjectIndex, m_Cross );
	}
}

bool CFcDynamicProp::AddForce( D3DXVECTOR3* Force, D3DXVECTOR3 *pPos, int nBreakPower, PROP_BREAK_TYPE Type)
{
	if( m_nAddForceRemain > 0 ) return false;
	if( !m_bEnable ) {
		SetEnable( true );
	}
	D3DXVECTOR3 vAngVel;
	D3DXVec3Cross( &vAngVel, &D3DXVECTOR3(0,1,0), Force);
	m_pActor->AddVelocity( Force , &vAngVel);		
	m_nAddForceRemain = 20;		// 20프레임동안 같은 공격에 안받는다..

	return true;
}

void CFcDynamicProp::SetEnable(bool bEnable)
{
	if( !m_pActor ) return;
	m_pActor->SetSleep(!bEnable);
	NxActor *pNxActor = m_pActor->GetActor();
	if( pNxActor ) {
		if( bEnable == false ) {            
			pNxActor->raiseBodyFlag( NX_BF_KINEMATIC );
		}
		else {
			pNxActor->clearBodyFlag( NX_BF_KINEMATIC );			
		}
	}
	m_bEnable = bEnable;
}

//////////////////////////
// CFcBreakableProp class
CFcBreakableProp::CFcBreakableProp()
{
	m_bDestroyed = false;
	m_pPhysicsData = NULL;
	m_nBreakSkinIndex = -1;
	m_nDeleteAlphaFrame = 0;
	m_nHP = 0;
	m_nMaxHP = 0;
	m_BreakType = PROP_BREAK_TYPE_NON;
	m_BreakPartsType = PROP_BREAK_PARTS_TYPE_NON;
	m_BreakForce = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	m_nCrushParticleIndex = 0;
	m_nAddForceRemain = 0;
	m_nLastAddForceType = PROP_BREAK_TYPE_NON;
	m_nSleepFrame = 0;
	m_bCrushEnable = true;
	m_bAnimation = false;
	m_nBreakableShadowType = 0;	
	m_nAdditionalLeftBaseIndex = -1;
	m_dwRotAndScale = 0;
}

CFcBreakableProp::~CFcBreakableProp()
{
	if( m_pPhysicsData ) {
		CFcPhysicsLoader::ClearPhysicsData( m_pPhysicsData );
	}
	if( m_nAdditionalLeftBaseIndex != -1) {
		g_BsKernel.DeleteObject( m_nAdditionalLeftBaseIndex );
	}

}

bool CFcBreakableProp::IsBreak()
{
	return m_bDestroyed;
}

void CFcBreakableProp::Initialize( int nTypeDataIndex, 
								  PROPTYPEDATA* pData, 
								  PROPTYPEDATA* pPhysicsPropData, 
								  char* pName, 
								  D3DXVECTOR3* pPos, 
								  DWORD dwRotAndScale,
								  DWORD* pParam,
								  char* pParamString )
{

	int nObjectIndex ;
	m_nAniLength = 0;
	m_nPropSoundMaterial = pData->nSndType;

	if( pData->nAniIndex != -1) {
		m_fCurFrame = 0.f;
		nObjectIndex = g_BsKernel.CreateAniObjectFromSkin( pData->nSkinIndex, pData->nAniIndex );
		m_nAniLength = g_BsKernel.GetAniLength( nObjectIndex, 0 );
		m_fCurFrame = (float)Random( m_nAniLength );
		m_bAnimation = true;
	}
	else {
		nObjectIndex = g_BsKernel.CreateStaticObjectFromSkin( pData->nSkinIndex );		
		m_bAnimation = false;
	}
	if( nObjectIndex < 0 )
	{
		BsAssert(0);
		return; 
	}
	// 맵툴의 공식대로 회전, 스케일 조절
	CBsObject* pObj = g_BsKernel.GetEngineObjectPtr( nObjectIndex );

	m_dwRotAndScale = dwRotAndScale;
	InitCrossVec( pPos, dwRotAndScale, &m_Cross );
	pObj->SetEntireObjectMatrix( m_Cross );

	int nAttr = MakeAttr( pData );
	SetInitData( nObjectIndex, nTypeDataIndex, pData->nType, nAttr, pName, m_Cross.m_PosVector, pParam, pParamString );
    
	/*if( m_bAnimation ) {
		m_bProcess = true;
	}
	else {
		m_bProcess = false;
	}*/
	m_bProcess = true;

	m_bDestroyed = false;
	m_bDeleteObject = false;
	m_BreakType = ( PROP_BREAK_TYPE )pData->bCrush;
	m_nCrushParticleIndex = pData->nCrushParticleIndex;

	m_nMaxHP = m_nHP = pData->nCrushLevel;

	
	// 무너지는 타입은 원본에 nBrokenType을 참조합니다. yooty
	m_BreakPartsType = ( PROP_BREAK_PARTS_TYPE )pData->nBrokenType;		// 부셔지는 파츠 타입

	if( pPhysicsPropData )
	{
	
		char cPhysicsFileName[64];
		strcpy( cPhysicsFileName, pPhysicsPropData->cName );
		RemoveEXT( cPhysicsFileName );
		strcat( cPhysicsFileName, ".txt" );

		g_BsKernel.chdir("Prop");
		m_pPhysicsData = CFcPhysicsLoader::LoadPhysicsData(cPhysicsFileName);
		m_nBreakSkinIndex = pPhysicsPropData->nSkinIndex;

// [PREFIX:beginmodify] 2006/2/25 junyash PS#5816 dereferencing NULL pointer
		#if 0
		m_BoundingBox = *( AABB * )g_BsKernel.SendMessage( nObjectIndex, BS_GET_BOUNDING_BOX );
		#else
		AABB *pBoundingBox = ( AABB * )g_BsKernel.SendMessage( nObjectIndex, BS_GET_BOUNDING_BOX );
		BsAssert(pBoundingBox);
		m_BoundingBox = *pBoundingBox;
		#endif
// [PREFIX:endmodify] junyash

		if( m_pPhysicsData->bUseBoundingBox ) {
			m_BoundingBox.Vmin = *(BSVECTOR*)&(m_pPhysicsData->BoundingBox[0]);
			m_BoundingBox.Vmax = *(BSVECTOR*)&(m_pPhysicsData->BoundingBox[1]);
		}
		g_BsKernel.chdir("..");
	}
	else
	{
		m_bDestroyed = true;
	}

	g_BsKernel.SendMessage(nObjectIndex, BS_SHADOW_CAST, pData->nShadowType );
	// 부셔진 후의 오브젝트의 Shadow Type은 원본 오븐젝트의 Shadow를 사용합니다.
	m_nBreakableShadowType = pData->nShadowType;

	g_BsKernel.chdir("fx");
	for( int i = 0 ; i < 2 ; i++ )
	{
		if( pData->cFXName[i][0] )
		{
			char szFxFilePath[256];
			sprintf(szFxFilePath,"%s%s",g_BsKernel.GetCurrentDirectory(), pData->cFXName[i] );
			int nFx = -1;
			nFx = g_BsKernel.FindFXTemplate(szFxFilePath);
			if( nFx != -1 ){
				g_BsKernel.AddFXTemplateRef(nFx);
			}
			else{
				DebugString("Find Not Prop Fx File : %s \n",szFxFilePath);
				BsAssert(0);
			}
			SetFxTemplate(i, nFx);
		}//if
	}// for
	g_BsKernel.chdir("..");

	if( GetFxTemplate(0) != -1 )
	{
		BsAssert( GetFxIndex(0) == -1 );
		SetFxIndex(0, g_BsKernel.CreateFXObject( GetFxTemplate(0) ) );
		g_BsKernel.SetFXObjectState( GetFxIndex(0), CBsFXObject::PLAY, -99 );
		g_BsKernel.UpdateObject( GetFxIndex(0) , m_Cross);
	}
	m_nPropSoundMaterial = pData->nSndType;
	if( pData->nCollisionMeshIndex > 0 )
	{
		m_pCollisionMeshInfo = new PropCollisionMeshInfoExt;

		float fMaxScale = D3DXVec3LengthSq( &(m_Cross.m_XVector) );
		float fTempScale = D3DXVec3LengthSq( &(m_Cross.m_YVector) );
		if( fTempScale > fMaxScale ) fMaxScale = fTempScale;
		fTempScale = D3DXVec3LengthSq( &(m_Cross.m_ZVector) );
		if( fTempScale > fMaxScale ) fMaxScale = fTempScale;

		m_pCollisionMeshInfo->Initialize( pData->nCollisionMeshIndex, m_nObjectIndex, fMaxScale );

#ifdef _SHOW_COLLISION_PROP
		m_nCollisionIndex = g_BsKernel.CreateStaticObject( pData->nCollisionMeshIndex );

		static int nMaterialIndex = g_BsKernel.LoadMaterial("material\\default.fx");
		BsAssert(nMaterialIndex != -1);
		int nSubMeshCount = g_BsKernel.SendMessage(m_nCollisionIndex, BS_GET_SUBMESH_COUNT);
		for (int i=0; i<nSubMeshCount; ++i)
		{
			g_BsKernel.SendMessage(m_nCollisionIndex, BS_SET_SUBMESH_MATERIAL, i, nMaterialIndex);
		}		
		g_BsKernel.GetEngineObjectPtr( m_nCollisionIndex )->SetEntireObjectMatrix( m_Cross );
#endif
	}
}

void CFcBreakableProp::DestroyCheck()
{
	if( m_bDeleteObject )	// BreakableProp 상속받는 다른 Class 에도 Copy & Paste 해줘야된다.
	{
		if( m_pPhysicsData )
		{
			D3DXMATRIX *pMatrix[200];
			char szBoneName[200][256];
			int nBoneCount;

			if( m_bAnimation ) {
				g_BsKernel.SendMessage( m_nObjectIndex, BS_REFRESH_ANI );				
				nBoneCount = g_BsKernel.SendMessage( m_nObjectIndex, BS_GET_BONE_NAME_LIST, (DWORD)szBoneName);
				assert(nBoneCount < 200);
				for( int i = 0; i < nBoneCount; i++) {
					pMatrix[i] = (D3DXMATRIX*)g_BsKernel.SendMessage( m_nObjectIndex, BS_GET_BONE_MATRIX, (DWORD)szBoneName[i]);
				}
			}
			g_BsKernel.DeleteObject( m_nObjectIndex );
			m_nObjectIndex = g_BsKernel.CreateClothObjectFromSkin( m_nBreakSkinIndex, m_pPhysicsData, m_Cross);
			g_BsKernel.SendMessage( m_nObjectIndex, BS_SHADOW_CAST, m_nBreakableShadowType );
			g_BsKernel.SendMessage( m_nObjectIndex, BS_PHYSICS_ADDFORCE, ( DWORD )&m_BreakForce, ( DWORD )&m_Cross.m_PosVector );

			if( m_bAnimation ) {
				for( int i = 0; i < nBoneCount; i++) {
					g_BsKernel.SendMessage( m_nObjectIndex, BS_PHYSICS_SET_ACTOR_MATRIX, (DWORD)szBoneName[i], (DWORD)pMatrix[i]);
				}
			}

			m_bAnimation = false;

			m_nSleepFrame = 1;

			((CBsClothObject*)g_BsKernel.GetEngineObjectPtr(m_nObjectIndex))->SetGroup("BreakableProp");
			m_bDeleteObject = false;

			SAFE_DELETE( m_pCollisionMeshInfo );	// Collision Disable

		}
		else {
			BsAssert( 0 && "Breakable Prop Data Not Found");
			DebugString(" Breakable Prop Data Not Found!\n");
			m_bDeleteObject = false;
		}
	}
}

void CFcBreakableProp::Process()
{
	if( m_nAddForceRemain )
	{
		m_nAddForceRemain--;
	}

	if( m_nSleepFrame ) {
		m_nSleepFrame++;
		if( m_nSleepFrame > 250 ) {
			g_BsKernel.SendMessage(m_nObjectIndex, BS_PHYSICS_MAKE_SLEEP);
			m_nSleepFrame = 0;
			if( m_BreakPartsType ==  PROP_BREAK_PARTS_TYPE_ALL_DISAPPEAR ) {
				m_nDeleteAlphaFrame = 100;
				g_BsKernel.SendMessage(m_nObjectIndex, BS_SHADOW_CAST, BS_SHADOW_NONE );
			}
			if( m_BreakPartsType == PROP_BREAK_PARTS_TYPE_LEFT_BASE) {	
				m_nDeleteAlphaFrame = 100;
				g_BsKernel.SendMessage(m_nObjectIndex, BS_SHADOW_CAST, BS_SHADOW_NONE );

				// 밑둥만 남는 경우는 추가적인 오브젝트 생성해줘야 한다.
				m_nAdditionalLeftBaseIndex = g_BsKernel.CreateClothObjectFromSkin( m_nBreakSkinIndex, m_pPhysicsData, m_Cross);
				g_BsKernel.SendMessage( m_nAdditionalLeftBaseIndex, BS_SHADOW_CAST, m_nBreakableShadowType );
				g_BsKernel.SendMessage( m_nAdditionalLeftBaseIndex, BS_PHYSICS_HIDE_CRUSH_ACTOR );	// 밑둥만 보이게 한다.
				D3DXMATRIX Mat;
				D3DXMatrixIdentity(&Mat);
				Mat._42 = 100000.f;	// 편법이다.. Root 만 엉뚱한곳으로 옮겨버리고, 나머지는 알파로 사라지게 한다..
				g_BsKernel.SendMessage( m_nObjectIndex, BS_PHYSICS_SET_ACTOR_MATRIX, (DWORD)"B_Root", (DWORD)&Mat);
			}
		}
	}

	if( m_nDeleteAlphaFrame > 0 ) {
		--m_nDeleteAlphaFrame;
		float fAlpha = (float)m_nDeleteAlphaFrame / 100.f;
		g_BsKernel.SendMessage( m_nObjectIndex, BS_ENABLE_OBJECT_ALPHABLEND, 1 );
		g_BsKernel.SendMessage( m_nObjectIndex, BS_SET_OBJECT_ALPHA, (DWORD)&fAlpha );
		if( m_nDeleteAlphaFrame == 0 ) {
			if( m_BreakPartsType == PROP_BREAK_PARTS_TYPE_ALL_DISAPPEAR ) {	// 다 부셔지는 경우 완전히 지워도 상관없다.
//				Delete();
			}
			else if( m_BreakPartsType == PROP_BREAK_PARTS_TYPE_LEFT_BASE ) { // 밑둥만 남은경우는 부셔진것들만 미리 지워줘도 된다.
				g_BsKernel.DeleteObject( m_nObjectIndex );
				m_nObjectIndex = m_nAdditionalLeftBaseIndex;	
				m_nAdditionalLeftBaseIndex = -1;
			}
		}		
	}

	if( m_bAnimation ) {
		m_fCurFrame += 1.f;
		if( m_fCurFrame >= m_nAniLength) {
			m_fCurFrame = 0.f;
		}
	}

	DestroyCheck();

	if( m_bDestroyed )
	{
		if( GetFxIndex(0) != -1 )
		{
			g_BsKernel.DeleteObject( GetFxIndex(0) );
			SetFxIndex(0, -1);
			// 프랍에 연결된 포인트 라이트 지워준다.
			g_FcWorld.GetPropManager()->DeletePointLightRange(m_Cross.m_PosVector.x, m_Cross.m_PosVector.z, 0.f);
		}
		if( GetFxIndex(1) != -1 )
		{
			g_BsKernel.UpdateObject( GetFxIndex(1) , m_Cross);
			if(g_BsKernel.GetFXObjectState( GetFxIndex(1)) == CBsFXObject::STOP)
			{
				for( int i = 0 ; i < 2 ; i++ ) {
					if( GetFxIndex(i) != -1 )
					{
						g_BsKernel.DeleteObject(GetFxIndex(i));
						SetFxIndex(i, -1);
					}
				}
			}
		}
	}
}

void CFcBreakableProp::UpdateObject()
{
	if( m_bAnimation ) {		
		g_BsKernel.SetCurrentAni( m_nObjectIndex, 0, m_fCurFrame );		
	}

	g_BsKernel.UpdateObject( m_nObjectIndex );	
	
	if( m_nAdditionalLeftBaseIndex != -1 ) {
		g_BsKernel.UpdateObject( m_nAdditionalLeftBaseIndex );
	}

#ifdef _SHOW_COLLISION_PROP
	if( m_nCollisionIndex != -1)
		g_BsKernel.UpdateObject( m_nCollisionIndex );
#endif


	// Process 에서 Update로 옮겨도 문제가 없는지 확인필요..
	if(!m_bDestroyed)
	{
		if( GetFxIndex(0) != -1 )
		{
			g_BsKernel.UpdateObject( GetFxIndex(0) , m_Cross);
		}
	}

}

bool CFcBreakableProp::Break( int nDamage, PROP_BREAK_TYPE nType )
{
	if( m_bDestroyed ) // || ( m_pPhysicsData == NULL ) ) 
		return false;

	if( m_bCrushEnable == false )
		return false;

	// 이벤트 프랍을 공격하면 무시한다.
	if( m_BreakType == PROP_BREAK_TYPE_EVENT && nType != PROP_BREAK_TYPE_EVENT )
		return false;

	if( m_BreakType == PROP_BREAK_TYPE_DAMGE_EXCEPT_SPECIAL_ATTACK && nType == PROP_BREAK_TYPE_SPECIAL_ATTACK )
		return false;


	switch( m_BreakType )
	{
	case PROP_BREAK_TYPE_NORMAL:
		if( nType == PROP_BREAK_TYPE_NORMAL )
		{
			m_nHP -= nDamage;
			if( m_nHP < 0 )
				m_nHP = 0;
		}
		else if( nType == PROP_BREAK_TYPE_SPECIAL_ATTACK )
		{
			m_nHP -= nDamage * 10;
			if( m_nHP < 0 )
				m_nHP = 0;
		}
		else if( nType == PROP_BREAK_TYPE_EVENT )
		{
			m_nHP -= nDamage;
			if( m_nHP < 0 )
				m_nHP = 0;
		}
		else
		{
			return false;
		}
		break;
	case PROP_BREAK_TYPE_SPECIAL_ATTACK:
		if( nType == PROP_BREAK_TYPE_SPECIAL_ATTACK )
		{
			m_nHP -= nDamage;
			if( m_nHP < 0 )
				m_nHP = 0;
		}
		else
		{
			return false;
		}
		break;
	case PROP_BREAK_TYPE_EVENT:
		if( nType == PROP_BREAK_TYPE_EVENT )
		{
			m_nHP -= nDamage;
			if( m_nHP < 0 )
				m_nHP = 0;
		}
		else
		{
			return false;
		}
		break;
	case PROP_BREAK_TYPE_DAMGE_EXCEPT_SPECIAL_ATTACK:
		if( nType == PROP_BREAK_TYPE_NORMAL )
		{
			m_nHP -= nDamage;
			if( m_nHP < 0 )
				m_nHP = 0;
		}
		else if( nType == PROP_BREAK_TYPE_EVENT )
		{
			m_nHP -= nDamage;
			if( m_nHP < 0 )
				m_nHP = 0;
		}
		else
		{
			return false;
		}
		break;
	default:
		BsAssert(0);
	}

	if( nDamage == -1 )	// damage를 -1 주면 무조건 부신다..
	{
		m_nHP = 0;
	}

	if( g_FCGameData.bCrashAll == true )	// 치트키 켜있으면 무조건 부신다..
	{
		m_nHP = 0;
	}

	m_bProcess = true;

	m_nAddForceRemain = 20;		// 20프레임동안 같은 공격에 안받는다..
	m_nLastAddForceType = nType;	
	if( m_nHP > 0 )
	{		
		g_pSoundManager->PlayPropSound( GetPosPtr() ,(SOUND_PROP_MATERIAL)m_nPropSoundMaterial ,SPS_ATTACKED  );
		return false;
	}


	if( m_nObjectIndex != -1 )
		m_bDeleteObject = true;

	g_pSoundManager->PlayPropSound( GetPosPtr() ,(SOUND_PROP_MATERIAL)m_nPropSoundMaterial ,SPS_DESTROYED );


	if(GetFxIndex(0) != -1)
	{
		g_BsKernel.SetFXObjectState( GetFxIndex(0), CBsFXObject::PLAY, 1 );
	}

	if(GetFxTemplate(1) != -1)
	{ 
		BsAssert( GetFxIndex(1) == -1 );
		SetFxIndex(1, g_BsKernel.CreateFXObject( GetFxTemplate(1) ) );
		g_BsKernel.SetFXObjectState( GetFxIndex(1), CBsFXObject::PLAY, 1 );
		g_BsKernel.UpdateObject( GetFxIndex(1) , m_Cross);
	}

	m_bDestroyed = true;

	if( m_BreakPartsType == PROP_BREAK_PARTS_TYPE_ALL_DISAPPEAR ||
		m_BreakPartsType == PROP_BREAK_PARTS_TYPE_LEFT_ALL )
	{
		D3DXMATRIX Mat = GetMat();
		g_FcWorld.SetBuildingAttrAtCrossAttr( g_FcWorld.GetPropTypeData_(m_nTypeDataIndex), m_dwRotAndScale, &Mat, false );
	}
	return true;
}

void CFcBreakableProp::AddHP( int nHP )
{
	m_nHP += nHP;
	if( m_nHP > m_nMaxHP )
		m_nHP = m_nMaxHP;

	if( m_nHP < 0 )
		m_nHP = 0;
}

// Trigger에서만 불러줘야 한다.
void CFcBreakableProp::Destroy()
{
	if( m_bDestroyed )
		return;

	Break( -1, PROP_BREAK_TYPE_EVENT);
}

bool CFcBreakableProp::AddForce( D3DXVECTOR3* Force, D3DXVECTOR3 *pPos, int nBreakPower, PROP_BREAK_TYPE Type )
{
	if(IsDestroyed()) {
		return false;
	}

	if( ( Type == m_nLastAddForceType ) && ( m_nAddForceRemain > 0 ) )
	{
		return false;
	}

	if( !Break( nBreakPower, Type ) )
	{
		return false;
	}
	m_BreakForce = *Force;
//		g_BsKernel.SendMessage(m_nObjectIndex, BS_PHYSICS_ADDFORCE, (DWORD)Force);
//		m_hObject->AddForce( Force );		

	return true;
}

//////////////////////////
// CFcNavInfo class
CFcPropNavInfo::CFcPropNavInfo()
{
	m_nNavEventIfnoIndex =-1;
	m_bLink = false;
}

CFcPropNavInfo::~CFcPropNavInfo()
{
	int nCnt = m_NavCells.size();
	for( int i=0; i<nCnt; i++ )
	{
		delete m_NavCells[i];
	}
	m_NavCells.clear();
}

void CFcPropNavInfo::SetEventNav( int nIndex, NavigationMesh* pNavigationMesh, NAVEVENTINFO& vecNavEventInfo, bool bLink )
{
	m_NavCells.clear();

	m_nNavEventIfnoIndex = nIndex;

	int nCnt = vecNavEventInfo.nNumNavIndex;
	for( int i=0; i<nCnt; ++i )
	{
		NavigationCell* pNewCell = new NavigationCell;
		*pNewCell = *pNavigationMesh->Cell( vecNavEventInfo.nStartIndex + i );
		m_NavCells.push_back( pNewCell );
	}

	m_bLink = !bLink;

	if( !bLink )
		SetUnlink();
}

void CFcPropNavInfo::SetLink()
{
	if( !m_bLink )
	{
		g_FcWorld.LinkNavCells( m_NavCells, m_nNavEventIfnoIndex );
		m_bLink = true;
	}
}

void CFcPropNavInfo::SetUnlink()
{
	if( m_bLink )
	{
		g_FcWorld.UnlinkNavCells( m_nNavEventIfnoIndex );
		m_bLink = false;
	}
}





//////////////////////////
// CFcGateProp class
CFcGateProp::CFcGateProp()
{
	m_nTypeDataIndex = -1;
	m_nObjectIndex = -1;
	m_State = GATE_STATE_CLOSED;
	m_fCurFrame = 0.f;
	m_nCurAni = 0;
	m_bSoundPlay = false;
	m_dwRotAndScale = 0;
}

CFcGateProp::~CFcGateProp()
{

}


void CFcGateProp::Initialize( int nTypeDataIndex, PROPTYPEDATA* pData, char* pName, D3DXVECTOR3* pPos, DWORD dwRotAndScale )
{
	int nObjectIndex = g_BsKernel.CreateAniObjectFromSkin( pData->nSkinIndex, pData->nAniIndex );
	if( nObjectIndex < 0 )
	{
		BsAssert(0);
		return; 
	}
	// 맵툴의 공식대로 회전, 스케일 조절
	CBsObject* pObj = g_BsKernel.GetEngineObjectPtr( nObjectIndex );
	CCrossVector Cross;
	m_dwRotAndScale = dwRotAndScale;
	InitCrossVec( pPos, dwRotAndScale, &m_Cross );
	pObj->SetEntireObjectMatrix( m_Cross );

	int nAttr = MakeAttr( pData );
	
	SetInitData( nObjectIndex, nTypeDataIndex, pData->nType, nAttr, pName, m_Cross.m_PosVector );

	m_State = GATE_STATE_CLOSED;
	m_nCurAni = 0;
	m_fCurFrame = 0.f;
	g_BsKernel.SetCurrentAni( m_nObjectIndex, 0, m_fCurFrame );

	for( int i=0; i<GATE_STATE_NUM; i++ )
	{
		m_nLength[i] = g_BsKernel.GetAniLength( m_nObjectIndex, i );		
	}

	//	D3DXMATRIX Mat;
	//	Mat = Cross;

	D3DXVECTOR2 Pos(pPos->x, pPos->z );
	D3DXMATRIX Mat = GetMat();
	g_FcWorld.SetBuildingAttrAtCrossAttr( pData, m_dwRotAndScale, &Mat, true );

	g_BsKernel.SendMessage(nObjectIndex, BS_SHADOW_CAST, pData->nShadowType );

	g_BsKernel.chdir("fx");
	for( int i = 0 ; i < 2 ; i++ )
	{
		if( pData->cFXName[i][0] )
		{
			BsAssert( GetFxTemplate(i) == -1 );

			char szFxFilePath[256];
			sprintf(szFxFilePath,"%s%s",g_BsKernel.GetCurrentDirectory(), pData->cFXName[i] );

			BsAssert( GetFxTemplate(i) == -1 );
			int nFx = -1;
			nFx = g_BsKernel.FindFXTemplate(szFxFilePath);
			if( nFx != -1 ){
				g_BsKernel.AddFXTemplateRef(nFx);
			}
			else{
				DebugString("Find Not Prop Fx File : %s \n",szFxFilePath);
				BsAssert(0);
			}
			SetFxTemplate(i, nFx);
		}//if
	}// for
	g_BsKernel.chdir("..");

	if( GetFxTemplate(0) != -1 )
	{
		BsAssert( GetFxIndex(0) == -1 );
		SetFxIndex(0, g_BsKernel.CreateFXObject( GetFxTemplate(0) ) );
		g_BsKernel.SetFXObjectState( GetFxIndex(0), CBsFXObject::PLAY, -99 );
		g_BsKernel.UpdateObject( GetFxIndex(0) , m_Cross);
	}

	m_bProcess = true;
}

#define GATE_OPEN_SOUND_FRAME_RATE 0.8f
#define GATE_CLOSE_SOUND_FRAME_RATE 0.8f
#define GATE_SOUND_LIMIT_ADD	4000.f
#define GATE_SOUND_PLAY_RATE	0.7f

void CFcGateProp::PlayGateSound()
{
	if(m_bSoundPlay == true){	
		return;
	}

	D3DXVECTOR3 Pos = GetPos();
	D3DXVECTOR3 Dir;
	float fLength = D3DXVec3Length( &(Pos - g_pSoundManager->GetListenerPos()));
	D3DXVec3Normalize(&Dir,&(Pos - g_pSoundManager->GetListenerPos()));

	if( fLength > g_pSoundManager->GetDistanceLimit() )
	{
		if( (fLength - g_pSoundManager->GetDistanceLimit()) < GATE_SOUND_LIMIT_ADD) {		
			Pos = g_pSoundManager->GetListenerPos() + (Dir * g_pSoundManager->GetDistanceLimit() * GATE_SOUND_PLAY_RATE);
		}	
	}

	if( m_State == GATE_STATE_OPEN )
	{
		if( (float)m_nLength[GATE_STATE_OPEN] * GATE_OPEN_SOUND_FRAME_RATE < m_fCurFrame){
			g_pSoundManager->Play3DSound(this,SB_COMMON,"GATE_OPEN",&Pos );
			m_bSoundPlay = true;
		}
	}
	else if( m_State == GATE_STATE_CLOSE )
	{
		if( (float)m_nLength[GATE_STATE_CLOSE] * GATE_CLOSE_SOUND_FRAME_RATE < m_fCurFrame)		{
			g_pSoundManager->Play3DSound( this,SB_COMMON,"GATE_CLOSE",&Pos );
			m_bSoundPlay = true;
		}
	}
}

void CFcGateProp::Process()
{
	//CFcProp::Process();

	int nCurAni = 0;;
	if( m_State == GATE_STATE_OPEN )
	{
		m_fCurFrame += 1.f;
		if( m_fCurFrame >= m_nLength[GATE_STATE_OPEN] )
		{			
			nCurAni = GATE_STATE_OPENED;
			m_fCurFrame = 0.f;
			m_State = GATE_STATE_OPENED;

			m_NavInfo.SetLink();
			D3DXMATRIX Mat = GetMat();
			g_FcWorld.SetBuildingAttrAtCrossAttr( g_FcWorld.GetPropTypeData_(m_nTypeDataIndex), m_dwRotAndScale, &Mat, false);
		}
		else
		{		
			nCurAni = GATE_STATE_OPEN;			
			PlayGateSound();
		}
	}
	else if( m_State == GATE_STATE_CLOSE )
	{
		m_fCurFrame += 1.f;
		if( m_fCurFrame >= m_nLength[GATE_STATE_CLOSE] )
		{
			m_State = GATE_STATE_CLOSED;
			m_fCurFrame = 0.f;
			nCurAni = GATE_STATE_CLOSED;

			m_NavInfo.SetUnlink();

			D3DXMATRIX Mat = GetMat();
			// 확인 필요			
			g_FcWorld.SetBuildingAttrAtCrossAttr( g_FcWorld.GetPropTypeData_(m_nTypeDataIndex), m_dwRotAndScale, &Mat, true );
		}
		else
		{
			PlayGateSound();
			nCurAni = GATE_STATE_CLOSE;
		}
	}
	else if( m_State == GATE_STATE_OPENED )
	{
		m_fCurFrame = 0.f;
		nCurAni = GATE_STATE_OPENED;

	}
	else if( m_State == GATE_STATE_CLOSED)
	{
		m_fCurFrame = 0.f;
		nCurAni = GATE_STATE_CLOSED;
	}
	else
	{
		BsAssert( 0 );
	}
	m_nCurAni = nCurAni;

}

void CFcGateProp::UpdateObject()
{
	g_BsKernel.SetCurrentAni( m_nObjectIndex, m_nCurAni, m_fCurFrame );
	g_BsKernel.UpdateObject( m_nObjectIndex );
}

void CFcGateProp::SetEventNav( int nIndex, NavigationMesh* pNavigationMesh, NAVEVENTINFO& vecNavEventInfo, bool bLink )
{
	m_NavInfo.SetEventNav( nIndex, pNavigationMesh, vecNavEventInfo, bLink );
}


bool CFcGateProp::IsOpened()
{
	return ( m_State == GATE_STATE_OPENED );
}

bool CFcGateProp::IsClosed()
{
	return ( m_State == GATE_STATE_CLOSED );
}

bool CFcGateProp::CmdOpen()
{
	m_State = GATE_STATE_OPEN;
	m_bSoundPlay = false;
	return true;
}

bool CFcGateProp::CmdClose()
{
	m_State = GATE_STATE_CLOSE;
	m_bSoundPlay = false;
	return true;
}



//////////////////////////
// CFcBridgeProp class
CFcBridgeProp::CFcBridgeProp()
{
	m_nHP = 1000;
	m_nSleepFrame = 0;
}

CFcBridgeProp::~CFcBridgeProp()
{

}


void CFcBridgeProp::Initialize( int nTypeDataIndex, PROPTYPEDATA* pData, PROPTYPEDATA* pPhysicsPropData, char* pName, D3DXVECTOR3* pPos, DWORD dwRotAndScale )
{
	CFcBreakableProp::Initialize( nTypeDataIndex, pData, pPhysicsPropData, pName, pPos, dwRotAndScale );
}


void CFcBridgeProp::Process()
{
	/*	if(IsBreak()) {

	int nBreakObjIndex = m_hObject->GetEngineIndex();

	if( nBreakObjIndex != m_nObjectIndex ) {
	g_BsKernel.DeleteObject( m_nObjectIndex );
	m_nObjectIndex = nBreakObjIndex;
	}
	}
	*/
	if( m_nSleepFrame ) {
		m_nSleepFrame++;
		if( m_nSleepFrame > 250 ) {
			g_BsKernel.SendMessage(m_nObjectIndex, BS_PHYSICS_MAKE_SLEEP);
			m_nSleepFrame = 0;
		}
	}

	DestroyCheck();

	if( GetFxIndex(0) != -1 )
	{
		g_BsKernel.UpdateObject( GetFxIndex(0) , m_Cross);
	}
}


void CFcBridgeProp::SetEventNav( int nIndex, NavigationMesh* pNavigationMesh, NAVEVENTINFO& vecNavEventInfo, bool bLink )
{
	m_NavInfo.SetEventNav( nIndex, pNavigationMesh, vecNavEventInfo, bLink );
}


void CFcBridgeProp::Destroy()
{
	CFcBreakableProp::Destroy();

	m_NavInfo.SetUnlink();

	D3DXMATRIX Mat = GetMat();
	g_FcWorld.SetBuildingAttrAtCrossAttr( g_FcWorld.GetPropTypeData_(m_nTypeDataIndex),	m_dwRotAndScale, &Mat, true);
}



CFcTrap::CFcTrap()
{
	m_bUse = true;
	m_nTypeDataIndex = -1;
	m_nObjectIndex = -1;
	m_State = TRAP_STATE_HIDED;
	m_fCurFrame = 0.f;
	m_nCurAni = 0;
}

CFcTrap::~CFcTrap()
{

}


void CFcTrap::Initialize( int nTypeDataIndex, PROPTYPEDATA* pData, char* pName, D3DXVECTOR3* pPos, 
						 DWORD dwRotAndScale, DWORD* pParam )
{
	int nObjectIndex = g_BsKernel.CreateAniObjectFromSkin( pData->nSkinIndex, pData->nAniIndex );
	if( nObjectIndex < 0 )
	{
		BsAssert(0);
		return; 
	}
	// 맵툴의 공식대로 회전, 스케일 조절
	CBsObject* pObj = g_BsKernel.GetEngineObjectPtr( nObjectIndex );
	InitCrossVec( pPos, dwRotAndScale, &m_Cross );
	pObj->SetEntireObjectMatrix( m_Cross );

	int nAttr = MakeAttr( pData );
	SetInitData( nObjectIndex, nTypeDataIndex, pData->nType, nAttr, pName, m_Cross.m_PosVector );

	m_State = TRAP_STATE_HIDED;
	m_nCurAni = 0;
	m_fCurFrame = 0.f;
	g_BsKernel.SetCurrentAni( m_nObjectIndex, 0, m_fCurFrame );

	for( int i=0; i<TRAP_STATE_NUM; i++ )
	{
		m_nLength[i] = g_BsKernel.SendMessage( m_nObjectIndex, BS_GETANILENGTH, i );
	}

	BsAssert( pParam );
	m_nTeam = pParam[0];
	m_bRepeat = pParam[1]?true:false;
	if(m_bRepeat) {
		//m_State = TRAP_STATE_ATTACK;
		m_nOldTick = GetProcessTick();
	}
	m_nStartDelayFrame = pParam[2];
	//	D3DXMATRIX Mat;
	//	Mat = Cross;

	//	if( pData->nShadowType == BS_SHADOW_NONE )
	//	{
	//		g_BsKernel.SendMessage( nObjectIndex, BS_SHADOW_CAST, 0 );
	//	}
	g_BsKernel.SendMessage(nObjectIndex, BS_SHADOW_CAST, pData->nShadowType );

	g_BsKernel.chdir("fx");
	for( int i = 0 ; i < 2 ; i++ )
	{
		if( pData->cFXName[i][0] )
		{
			BsAssert( GetFxTemplate(i) == -1 );

			char szFxFilePath[256];
			sprintf(szFxFilePath,"%s%s",g_BsKernel.GetCurrentDirectory(), pData->cFXName[i] );
			int nFx = -1;
			nFx = g_BsKernel.FindFXTemplate(szFxFilePath);
			if( nFx != -1 ){
				g_BsKernel.AddFXTemplateRef(nFx);
			}
			else{
				DebugString("Find Not Prop Fx File : %s \n",szFxFilePath);
				BsAssert(0);
			}
			SetFxTemplate(i, nFx);
		}//if
	}// for
	g_BsKernel.chdir("..");


	if( GetFxTemplate(0) != -1 )
	{
		BsAssert( GetFxIndex(0) == -1 );
		SetFxIndex(0, g_BsKernel.CreateFXObject( GetFxTemplate(0) ) );
		g_BsKernel.SetFXObjectState( GetFxIndex(0), CBsFXObject::PLAY, -99 );
		g_BsKernel.UpdateObject( GetFxIndex(0) , m_Cross);
	}
	m_bProcess = true;
}

#define TRAP_ATTACK_RADIUS		425.f
#define TRAP_DEFAULT_DAMAGE		100
#define TRAP_REPEAT_DELAY		80


void CFcTrap::Process()
{
	//CFcProp::Process();
	if( GetProcessTick() % FRAME_PER_SEC == m_nObjectIndex % FRAME_PER_SEC && m_bUse == true )
	{
		g_FcWorld.GetEnemyObjectListInRange( m_nTeam, &GetPos(), TRAP_ATTACK_RADIUS, m_vecObjs );
		int nCnt = m_vecObjs.size();
		if( nCnt > 0 )
		{
			m_State = TRAP_STATE_ATTACK;

			if( GetFxTemplate(1) != -1 )
			{
				BsAssert( GetFxIndex(1) == -1 );
				SetFxIndex(1, g_BsKernel.CreateFXObject( GetFxTemplate(1) ) );
				g_BsKernel.SetFXObjectState( GetFxIndex(1), CBsFXObject::PLAY );
				g_BsKernel.UpdateObject( GetFxIndex(1) , m_Cross);
			}
		}
	}

	int nCurAni = 0;;
	if( m_State == TRAP_STATE_ATTACK )
	{
		if( GetFxIndex(1) != -1 )
		{
			g_BsKernel.UpdateObject( GetFxIndex(1) , m_Cross);
		}
		m_fCurFrame += 1.f;

		float fAttackFrame = (float)m_nLength[TRAP_STATE_ATTACK] * 0.8f;
		if( m_fCurFrame - 1.f < fAttackFrame && m_fCurFrame > fAttackFrame )
		{
			int nCnt = m_vecObjs.size();
			for( int i=0; i<nCnt; i++ )
			{
				// damage
				GameObjHandle hDummy;
				D3DXVECTOR3 Pos = GetPos();
				float fHeight = GetPos().y;

				_FC_RANGE_DAMAGE rd;
				rd.Attacker = hDummy;
				rd.pPosition = &Pos;
				rd.fRadius = TRAP_ATTACK_RADIUS;
				rd.nPower = TRAP_DEFAULT_DAMAGE;
				rd.nPushY = 30; 
				rd.nPushZ = -13;
				rd.nTeam = m_nTeam;
				rd.nNotGenOrb = 0;
				rd.nNotLookAtHitUnit = 0;
				rd.p_bTeamDamage = false;
				rd.pHitDirection = NULL; 
				rd.nType = HT_NORMAL_ATTACK; 
				rd.fYMax = fHeight + 100.f; 
				rd.fYMin = fHeight - 100.f; 
				rd.nHitRemainFrame = 20;
				g_FcWorld.GiveRangeDamage( rd );
			}
			if(nCnt)
				m_bUse = false;
		}

		if( m_fCurFrame >= m_nLength[TRAP_STATE_ATTACK] )
		{
			m_fCurFrame = 0.f;
			m_State = TRAP_STATE_ATTACKED;			
			nCurAni = TRAP_STATE_ATTACKED;
		}
		else
		{
			nCurAni = TRAP_STATE_ATTACK;
		}
	}
	else if( m_State == TRAP_STATE_HIDE )
	{
		if( GetFxIndex(0) != -1 )
		{
			g_BsKernel.UpdateObject( GetFxIndex(0) , m_Cross);
		}
		m_fCurFrame += 1.f;
		if( m_fCurFrame >= m_nLength[TRAP_STATE_HIDE] )
		{
			m_State = TRAP_STATE_HIDED;				
			nCurAni = TRAP_STATE_HIDED;
			m_fCurFrame = 0.f;

			// 반복시간 체크용
			m_nOldTick = GetProcessTick();		
		}
		else
		{
			nCurAni = TRAP_STATE_HIDE;
		}
	}
	else if( m_State == TRAP_STATE_ATTACKED )
	{
		m_fCurFrame = 0.f;		
		nCurAni = TRAP_STATE_HIDE;
		m_State = TRAP_STATE_HIDE;		
	}
	else if( m_State == TRAP_STATE_HIDED )
	{
		m_fCurFrame = 0.f;

		int nElapsedTick = GetProcessTick()-m_nOldTick;
		if( (m_nStartDelayFrame && nElapsedTick > m_nStartDelayFrame ) || // 시작 딜레이( 값이 1이라도 있으면 시작 )
			(m_bRepeat && nElapsedTick > TRAP_REPEAT_DELAY) ) // 반복 딜레이
		{
			m_nStartDelayFrame=0;
			m_State = TRAP_STATE_ATTACK;
			nCurAni = TRAP_STATE_ATTACK;
			m_bUse = true;
		}
		else
		{
			nCurAni = TRAP_STATE_HIDED;
		}
	}
	else
	{
		BsAssert( 0 );
	}
	m_nCurAni = nCurAni;
	m_bProcess = true;
}

void CFcTrap::UpdateObject()
{
	g_BsKernel.SetCurrentAni( m_nObjectIndex, m_nCurAni, m_fCurFrame );
	g_BsKernel.UpdateObject( m_nObjectIndex, m_Cross );
}







CFcClimbProp::CFcClimbProp()
{

}


CFcClimbProp::~CFcClimbProp()
{

}

void CFcClimbProp::Initialize( int nTypeDataIndex, PROPTYPEDATA* pData, char* pName, D3DXVECTOR3* pPos, DWORD dwRotAndScale, DWORD* pParam)
{
	CFcProp::Initialize( nTypeDataIndex, pData, pName, pPos, dwRotAndScale, pParam );

	const D3DXMATRIX Mat = GetMat();
	D3DXMatrixInverse( &m_IMat, 0, &Mat );
}



bool CFcClimbProp::GetHeight( float fX, float fY, float* pHeight )
{
	PropHeightMap* pInfo = GetHeightMapInfo();

	return pInfo->GetHeight( fX, fY, &m_IMat, pHeight );
}





///////////////

#define SET_BLOCKWAY_ATTR_TICK	60

CFcBreakAndBlockWayProp::CFcBreakAndBlockWayProp()
{

}



CFcBreakAndBlockWayProp::~CFcBreakAndBlockWayProp()
{

}

void CFcBreakAndBlockWayProp::Initialize( int nTypeDataIndex, PROPTYPEDATA* pData, PROPTYPEDATA* pPhysicsPropData, char* pName, D3DXVECTOR3* pPos, DWORD dwRotAndScale )
{
	CFcBreakableProp::Initialize( nTypeDataIndex, pData, pPhysicsPropData, pName, pPos, dwRotAndScale );
	m_bDestroyed = false;
	m_nDeleyTick = 0;	
}


void CFcBreakAndBlockWayProp::Process()
{
	if( m_nDeleyTick > 0 )
	{
		--m_nDeleyTick;
		if( m_nDeleyTick == 0 )
		{
			m_NavInfo.SetUnlink();
		}
	}

	if( m_nSleepFrame ) {
		m_nSleepFrame++;
		if( m_nSleepFrame > 200 ) {
			g_BsKernel.SendMessage(m_nObjectIndex, BS_PHYSICS_MAKE_SLEEP);
			m_nSleepFrame = 0;
		}
	}	

	if( GetFxIndex(0) != -1 )
	{
		g_BsKernel.UpdateObject( GetFxIndex(0) , m_Cross);
	}

	DestroyCheck();

	if( m_bDestroyed )
	{
		if( GetFxIndex(0) != -1 )
		{
			g_BsKernel.DeleteObject( GetFxIndex(0) );
			SetFxIndex(0, -1);
			// 프랍에 연결된 포인트 라이트 지워준다.
			g_FcWorld.GetPropManager()->DeletePointLightRange(m_Cross.m_PosVector.x, m_Cross.m_PosVector.z, 0.f);
		}
		if( GetFxIndex(1) != -1 )
		{
			g_BsKernel.UpdateObject( GetFxIndex(1) , m_Cross);
			if(g_BsKernel.GetFXObjectState( GetFxIndex(1)) == CBsFXObject::STOP)
			{
				for( int i = 0 ; i < 2 ; i++ ) {
					if( GetFxIndex(i) != -1 )
					{
						g_BsKernel.DeleteObject(GetFxIndex(i));
						SetFxIndex(i, -1);
					}
				}
			}
		}
	}
}


void CFcBreakAndBlockWayProp::SetEventNav( int nIndex, NavigationMesh* pNavigationMesh, NAVEVENTINFO& vecNavEventInfo, bool bLink )
{
	m_NavInfo.SetEventNav( nIndex, pNavigationMesh, vecNavEventInfo, bLink );
}


void CFcBreakAndBlockWayProp::Destroy()
{
	D3DXMATRIX* Mat = (D3DXMATRIX*)g_BsKernel.GetEngineObjectPtr( m_nObjectIndex )->GetObjectMatrix();
	CCrossVector Cross;
	Cross = *Mat;
	//g_FcWorld.PlaySimpleFx( FX_PMCL17_COLLAPSE, &Cross );

	CFcBreakableProp::Destroy();
	m_bDestroyed = true;
	m_nDeleyTick = SET_BLOCKWAY_ATTR_TICK;
}



// CFcItemProp class
CFcItemProp::CFcItemProp()
{
	m_nItemIndex = 0;	
	m_bBreak = false;
	m_nDelayTick = 0;
	m_bEnable = true;
}


CFcItemProp::~CFcItemProp()
{
	if( m_nObjectIndex != -1 )
	{
		g_BsKernel.DeleteObject( m_nObjectIndex );
		m_nObjectIndex = -1;
	}
	SAFE_DELETEA( m_pNamedData );
}

void CFcItemProp::Initialize( int nTypeDataIndex, PROPTYPEDATA* pData, char* pName, D3DXVECTOR3* pPos, DWORD* pParam )
{
	int nObjectIndex = g_BsKernel.CreateStaticObjectFromSkin( pData->nSkinIndex );
	if( nObjectIndex < 0 )
	{
		BsAssert(0);
		return; 
	}
	// CrossVector에는 스케일 안들어가는데 ㅠ.ㅠ
	// 맵툴의 공식대로 회전, 스케일 조절
	CBsObject* pObj = g_BsKernel.GetEngineObjectPtr( nObjectIndex );
	//	InitCrossVec( pPos, 0, &m_Cross );
	m_Cross.m_PosVector = *pPos;
	m_Cross.m_PosVector.y = g_BsKernel.GetLandHeight( m_Cross.m_PosVector.x, m_Cross.m_PosVector.z ) + 80.f;
	pObj->SetEntireObjectMatrix( m_Cross );
	g_BsKernel.SendMessage(nObjectIndex, BS_SHADOW_CAST, pData->nShadowType );

	int nAttr = MakeAttr( pData );
	SetInitData( nObjectIndex, nTypeDataIndex, pData->nType, nAttr, pName, m_Cross.m_PosVector, pParam );

	m_nItemIndex = pParam[0];
	//m_bEnable = ( pParam[1] != 0 );

	m_nDelayTick = 30;

	g_BsKernel.chdir("fx");
	for( int i = 0 ; i < 2 ; i++ )
	{
		if( pData->cFXName[i][0] )
		{
			char szFxFilePath[256];
			sprintf(szFxFilePath,"%s%s",g_BsKernel.GetCurrentDirectory(), pData->cFXName[i] );
			int nFx = -1;
			nFx = g_BsKernel.FindFXTemplate(szFxFilePath);
			if( nFx != -1 ){
				g_BsKernel.AddFXTemplateRef(nFx);
			}
			else{
				DebugString("Find Not Prop Fx File : %s \n",szFxFilePath);
				BsAssert(0);
			}
			SetFxTemplate(i, nFx);
		}//if
	}// for
	g_BsKernel.chdir("..");

	if( GetFxTemplate(0) != -1 )
	{
		BsAssert( GetFxIndex(0) == -1 );
		SetFxIndex(0, g_BsKernel.CreateFXObject( GetFxTemplate(0) ) );
		g_BsKernel.SetFXObjectState( GetFxIndex(0), CBsFXObject::PLAY, -99 );
		g_BsKernel.UpdateObject( GetFxIndex(0) , m_Cross);
	}
	m_bProcess = true;
	/*

	// Item random SOX참조해서 발생시켜줘야 함
	if( pParam[0] < 0 )
	{
	BsAssert( 0 );
	}
	else
	{
	g_FcItemManager.CreateItemToWorld( pParam[0], *pPos );
	}
	*/
}


void CFcItemProp::Process()
{
	if(!m_bEnable)
		return;

	CBsObject* pObj = g_BsKernel.GetEngineObjectPtr( m_nObjectIndex );

	m_Cross.RotateYaw( 10 );
	//	pObj->SetEntireObjectMatrix( m_Cross );

	if( GetFxIndex(0) != -1 )
	{
		g_BsKernel.UpdateObject( GetFxIndex(0) , m_Cross);
	}

	if( m_bBreak )
	{
		if( m_nDelayTick > 0 )
		{
			--m_nDelayTick;
			if( m_nDelayTick == 0 )
			{
				if( m_nItemIndex < 0 )
				{
					g_FcItemManager.CreateItemToWorld( -1, m_vecPos , -m_nItemIndex );
				}
				else
				{
					g_FcItemManager.CreateItemToWorld( m_nItemIndex, m_vecPos );
				}
			}
		}
	}
}

void CFcItemProp::UpdateObject()
{
	if(!m_bEnable)
		return;


	if( !m_bBreak )
	{
		g_BsKernel.UpdateObject( m_nObjectIndex, m_Cross );
	}
}

bool CFcItemProp::Break( int nDamage, PROP_BREAK_TYPE nType )
{
	if( m_bBreak || !m_bEnable)
		return false;

	CCrossVector Cross;
	Cross.m_PosVector = m_vecPos;
	g_FcWorld.PlaySimpleFx( FX_BOX_EXPLOSION, &Cross );
	g_pSoundManager->PlaySound( SB_COMMON, "OB_S_WOOD_BREAK" );
	m_bBreak = true;

	g_BsKernel.ShowObject( m_nObjectIndex, false );

	return true;
}

bool CFcItemProp::AddForce( D3DXVECTOR3 *pForce, D3DXVECTOR3 *pPos, int nBreakPower, PROP_BREAK_TYPE Type )
{
	Break( nBreakPower, Type );

	return true;
}

void CFcItemProp::SetEnable() 
{
	m_bEnable = true;
	g_BsKernel.ShowObject( m_nObjectIndex, true );
}

void CFcItemProp::SetDisable() 
{
	m_bEnable = false;
	g_BsKernel.ShowObject( m_nObjectIndex, false );
}
