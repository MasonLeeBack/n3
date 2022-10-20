#include "stdafx.h"
#include "CollisionMesh.h"
#include "FcCommon.h"
#include "BsCommon.h"
#include "FcUtil.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


NavCollisionMesh::NavCollisionMesh()
{
//	m_pGroup = NULL;
}

NavCollisionMesh::~NavCollisionMesh()
{
	SAFE_DELETE_PVEC( m_pVecGroup );
}

void NavCollisionMesh::Initialize( int nGroupCount )
{
	SAFE_DELETE_PVEC( m_pVecGroup );
	if( nGroupCount == 0 ) return;

	for( int i=0; i<nGroupCount; i++ ) {
		NavCollisionGroup *pGroup = new NavCollisionGroup;
		m_pVecGroup.push_back( pGroup );
	}
}

void NavCollisionMesh::AddTri( int nGroup, D3DXVECTOR3* pV1, D3DXVECTOR3* pV2, D3DXVECTOR3* pV3 )
{
	NavTriangle Tri;
	Tri.vPos[0] = *pV1;
	Tri.vPos[1] = *pV2;
	Tri.vPos[2] = *pV3;

	m_pVecGroup[nGroup]->VecTri.push_back( Tri );
}


void NavCollisionMesh::GenerateExternalLine()
{
	int nCnt = m_pVecGroup.size();

	for( int i=0; i<nCnt; i++ ) {
		// 일단 안겹치는 라인들 구하고
		ProcessSinglePoint( m_pVecGroup[i] );
	}

	for( int i=0; i<nCnt; i++ ) {
		// 라인들 가지구 리스트 생성
		NavCollisionGroup *pCurGroup = m_pVecGroup[i];
		if( SortLine( pCurGroup ) == false ) {
			NavCollisionGroup *pGroup = new NavCollisionGroup;
			pGroup->VecLine = pCurGroup->VecLine;
			pCurGroup->VecLine.clear();
			m_pVecGroup.push_back( pGroup );
		}
		GeneratePointList( pCurGroup );
		pCurGroup->dwColor = D3DCOLOR_ARGB( 255, rand()%255, rand()%255, rand()%255 );
	}
}

void NavCollisionMesh::ProcessSinglePoint( NavCollisionGroup *pGroup )
{
	D3DXVECTOR3 vSour[3];

	int nCnt = pGroup->VecTri.size();
	for( int i=0; i<nCnt; i++ ) {
		bool bExist[3] = { true, true, true };
		for( int j=0; j<nCnt; j++ ) {
			if( j == i ) continue;
			for( int k=0; k<3; k++ ) {
				if( bExist[k] == true && CheckExistLine( pGroup->VecTri[i].vPos[k], pGroup->VecTri[i].vPos[ ( k == 2 ) ? 0 : (k+1) ], pGroup->VecTri[j].vPos ) == true ) {
					bExist[k] = false;
//					break;
				}
			}
		}
		if( bExist[0] == true ) {
			NavLine Line( pGroup->VecTri[i].vPos[0], pGroup->VecTri[i].vPos[1] );
			pGroup->VecLine.push_back( Line );
		}
		if( bExist[1] == true ) {
			NavLine Line( pGroup->VecTri[i].vPos[1], pGroup->VecTri[i].vPos[2] );
			pGroup->VecLine.push_back( Line );
		}
		if( bExist[2] == true ) {
			NavLine Line( pGroup->VecTri[i].vPos[2], pGroup->VecTri[i].vPos[0] );
			pGroup->VecLine.push_back( Line );
		}
	}
}

bool NavCollisionMesh::SortLine( NavCollisionGroup *pGroup )
{
	pGroup->VecSortLine.push_back( pGroup->VecLine[0] );
	pGroup->VecLine.erase( pGroup->VecLine.begin() );
	bool bLoopFlag = false;
	for( UINT i=0; i<pGroup->VecLine.size(); i++ ) {
		for( UINT j=0; j<pGroup->VecSortLine.size(); j++ ) {
			if( pGroup->VecLine[i].vPos[1] == pGroup->VecSortLine[j].vPos[0] ) {
				pGroup->VecSortLine.insert( pGroup->VecSortLine.begin() + j, pGroup->VecLine[i] );
				pGroup->VecLine.erase( pGroup->VecLine.begin() + i );
				i--;
				bLoopFlag = false;
				break;
			}
			else if( pGroup->VecLine[i].vPos[0] == pGroup->VecSortLine[j].vPos[1] ) {
				pGroup->VecSortLine.insert( pGroup->VecSortLine.begin() + j + 1, pGroup->VecLine[i] );
				pGroup->VecLine.erase( pGroup->VecLine.begin() + i );
				i--;
				bLoopFlag = false;
				break;
			}
		}
		if( i == pGroup->VecLine.size() - 1 ) {
			if( bLoopFlag == true ) return false;
			i = -1;
			bLoopFlag = true;
		}
	}
	pGroup->VecLine.clear();
	return true;
}

void NavCollisionMesh::GeneratePointList( NavCollisionGroup *pGroup )
{
	int nCnt = pGroup->VecSortLine.size();
	for( int i=0; i<nCnt; i++ ) {
		pGroup->VecPoint.push_back( pGroup->VecSortLine[i].vPos[0] );
		if( i == nCnt - 1 ) {
			pGroup->VecPoint.push_back( pGroup->VecSortLine[i].vPos[1] );
		}
	}
	pGroup->VecSortLine.clear();
}

bool NavCollisionMesh::CheckExistLine( D3DXVECTOR3 &v1, D3DXVECTOR3 &v2, D3DXVECTOR3 *pTar )
{
	if( v1 == pTar[0] && v2 == pTar[1] ) return true;
	if( v1 == pTar[1] && v2 == pTar[2] ) return true;
	if( v1 == pTar[2] && v2 == pTar[0] ) return true;

	if( v1 == pTar[1] && v2 == pTar[0] ) return true;
	if( v1 == pTar[2] && v2 == pTar[1] ) return true;
	if( v1 == pTar[0] && v2 == pTar[2] ) return true;

	return false;
}

#include "BsKernel.h"
#include "3DDevice.h"
void NavCollisionMesh::Render()
{
#ifndef _XBOX
	C3DDevice* pDevice = g_BsKernel.GetDevice();
	pDevice->SaveState();
	pDevice->BeginScene();

	D3DXMATRIX mat, matWorld;
	pDevice->GetTransform( D3DTS_WORLD, &matWorld );
	D3DXMatrixIdentity( &mat );
	pDevice->SetTransform( D3DTS_WORLD, &mat );

	pDevice->SetPixelShader(NULL);
	pDevice->SetVertexShader(NULL);
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, false);
	pDevice->SetRenderState( D3DRS_LIGHTING, FALSE );


	struct LINE_VERTEX
	{
		D3DXVECTOR3 v;
		DWORD       color;
	};
	static LINE_VERTEX pVertices[10000];

	int nCnt = m_pVecGroup.size();
	for( int i=0; i<nCnt; i++ ) {
		int nPointCnt = m_pVecGroup[i]->VecPoint.size();
		for( int j=0; j<nPointCnt; j++ ) {
			pVertices[j].v = m_pVecGroup[i]->VecPoint[j];
			pVertices[j].color = m_pVecGroup[i]->dwColor;
		}
		pDevice->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );
		pDevice->DrawPrimitiveUP( D3DPT_LINESTRIP, m_pVecGroup[i]->VecPoint.size() - 1, pVertices, sizeof(LINE_VERTEX) );
	}
	pDevice->SetTransform( D3DTS_WORLD, &matWorld );
	pDevice->EndScene();
	pDevice->RestoreState();
#endif //_XBOX
}

DWORD NavCollisionMesh::GetGroupCount()
{
	return m_pVecGroup.size();
}

NavCollisionGroup *NavCollisionMesh::GetGroup( DWORD dwIndex )
{
	return m_pVecGroup[dwIndex];
}

bool NavCollisionMesh::FindPath( D3DXVECTOR3 &vPos, D3DXVECTOR3 &vTarget, D3DXVECTOR3 &vResult )
{
	// 찾는다..
	std::vector<D3DXVECTOR3> *pVecResult = NULL;
	int nIndex = -1;
	DWORD dwGroup;
	int nCnt = m_pVecGroup.size();
	for( int i=0; i<nCnt; i++ ) {
		NavCollisionGroup *pGroup = m_pVecGroup[i];
		int nPointCnt = pGroup->VecPoint.size();
		for( int j=0; j<nPointCnt-1; j++ ) {
			if( IntLineToLine( vPos, vTarget, pGroup->VecPoint[j], pGroup->VecPoint[j+1] ) == true ) {
				pVecResult = &pGroup->VecPoint;
				nIndex = (int)j;
				dwGroup = i;
				break;
			}
		}
	}
	if( nIndex == -1 ) return false;

	int nCount = pVecResult->size() / 2;
	int nCurIndex;
	D3DXVECTOR3 *pPos[2];
	bool bFind[2] = { false, false };
	// 오른쪽으루 한번돌구...
	int nSize = (int)pVecResult->size();
	for( int i=1; i<nCount; i++ ) {
		nCurIndex = ( nIndex + i ) % nSize;
		pPos[0] = &(*pVecResult)[nCurIndex];

		if( CheckOpenPoint( dwGroup, *pPos[0], vTarget ) == true ) {
			bFind[0] = true;
			break;
		}
	}
	// 왼쪽으루 한번 돌구
	for( int i=1; i<nCount; i++ ) {
		nCurIndex = ( nIndex - i );
		if( nCurIndex < 0 ) nCurIndex += nSize;
		pPos[1] = &(*pVecResult)[nCurIndex];

		if( CheckOpenPoint( dwGroup, *pPos[1], vTarget ) == true ) {
			bFind[1] = true;
			break;
		}
//		++nFindCount[1];
	}
	if( bFind[0] == true && bFind[1] == false ) vResult = *pPos[0];
	else if( bFind[0] == false && bFind[1] == true ) vResult = *pPos[1];
	else if( bFind[0] == true && bFind[1] == true ) {
		float fLength1 = D3DXVec3LengthSq( (D3DXVECTOR3*)&(vTarget - *pPos[0]) );
		float fLength2 = D3DXVec3LengthSq( (D3DXVECTOR3*)&(vTarget - *pPos[1]) );
		if( fLength1 < fLength2 ) {
			vResult = *pPos[0];
		}
		else {
			vResult = *pPos[1];
		}
	}
	else {
//		BsAssert(0);
		return false;
	}
	return true;
}

/*

void NavCollisionMesh::Perp( D3DXVECTOR2 &vSor, D3DXVECTOR2 &vResult )
{
	vResult.x = -vSor.y;
	vResult.y = vSor.x;
}

bool NavCollisionMesh::Intersect( D3DXVECTOR3 &vSor1, D3DXVECTOR3 &vSor2, D3DXVECTOR3 &vTar1, D3DXVECTOR3 &vTar2 )
{
	D3DXVECTOR2 a, b, c;
	a.x = vSor2.x - vSor1.x;
	a.y = vSor2.z - vSor1.z;

	b.x = vTar2.x - vTar1.x;
	b.y = vTar2.z - vTar1.z;

	c.x = vTar1.x - vSor1.x;
	c.y = vTar1.z - vSor1.z;

	D3DXVECTOR2 vTemp;
	Perp( a, vTemp );
	float d = D3DXVec2Dot( &c, &vTemp );
	Perp( b, vTemp );
	float e = D3DXVec2Dot( &c, &vTemp );
	Perp( b, vTemp );
	float f = D3DXVec2Dot( &a, &vTemp );

	float t1, t2;
	if( abs(f) > 0.001f ) {
		t1 = d / f;
		t2 = e / f;

		if( t1 >= 0.f && t1 <= 1.f && t2 >= 0.f && t2 <= 1.f ) return true;
	}
	return false;
}
*/

bool NavCollisionMesh::CheckOpenPoint( DWORD dwGroup, D3DXVECTOR3 &vPos, D3DXVECTOR3 &vTar )
{
	NavCollisionGroup *pGroup = m_pVecGroup[dwGroup];
	int nCnt = pGroup->VecPoint.size();
	for( int j=0; j<nCnt-1; j++ ) {
		if( vPos == pGroup->VecPoint[j] || vPos == pGroup->VecPoint[j+1] ) continue;
		if( IntLineToLine( vPos, vTar, pGroup->VecPoint[j], pGroup->VecPoint[j+1] ) == true ) {
			return false;
		}
	}
	
	return true;
}