#include "StdAfx.h"
#include "ShortPathFinder.h"
#include "FcGameObject.h"
#include "BsKernel.h"
#include "FcWorld.h"
#include "BsFileManager.h"
#include "FcTroopObject.h"
#include "FcGlobal.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

bool CShortPathFinder::s_bAttrTable[256] = { false, };

int CShortPathFinder::s_pnArray[4][7] = {
	{ 1, 7, 2, 6, 3, 5, 4 },
	{ 1, 2, 3, 7, 6, 5, 4 },
	{ 7, 6, 5, 1, 2, 3, 4 },
	{ 4, 5, 3, 6, 2, 7, 1 },
};

CShortPathFinder::CShortPathFinder( CFcGameObject *pParent )
{
	if( s_bAttrTable[0] == false ) {
		s_bAttrTable[0x00] = true;
		s_bAttrTable[0x04] = true;
	}
	m_pParent = pParent;
	m_fLength = 100.f * 100.f;

	m_nPrevDir = -1;
	m_nCurDir = 0;
	m_bCheckNewPath = false;

	m_bLockPath = false;
	m_nLockPathTick = 0;
	m_vLockDir = D3DXVECTOR3( 0.f, 0.f, 0.f );
}

CShortPathFinder::~CShortPathFinder()
{
	m_pParent = NULL;
}

void CShortPathFinder::SetLength( float fLength )
{
	m_fLength = fLength * fLength;
}

float CShortPathFinder::GetLength()
{
	return sqrtf( m_fLength );
}

#include "CollisionMesh.h"
void CShortPathFinder::Process()
{
	m_bCheckNewPath = false;
	if( m_bLockPath == true ) {
		D3DXVECTOR3 *pCurDir = m_pParent->GetMoveDir();

		--m_nLockPathTick;
		*pCurDir = m_vLockDir;

		if( m_nLockPathTick <= 0 ) {
			D3DXVECTOR3 vDir = *m_pParent->GetMoveTargetPos() - m_pParent->GetDummyPos();
			vDir.y = 0.f;
			m_fTargetLength = D3DXVec3LengthSq( &vDir );
			D3DXVec3Normalize( &vDir, &vDir );

			if( GetPath( m_pParent->GetDummyPos(), vDir ) == false ) {
				*pCurDir = vDir;
			}
			else {
				*pCurDir = m_vResult - m_pParent->GetDummyPos();
				pCurDir->y = 0.f;
				D3DXVec3Normalize( pCurDir, pCurDir );
				m_bCheckNewPath = true;
			}

			m_nLockPathTick = 0;
			m_bLockPath = false;
		}
		return;
	}
	int nAniType = m_pParent->GetCurAniType() & 0x00FF;
	if( nAniType != ANI_TYPE_WALK && nAniType != ANI_TYPE_RUN  ) {
		m_nPrevDir = -1;
		return;
	}

	D3DXVECTOR3 vDir = *m_pParent->GetMoveTargetPos() - m_pParent->GetDummyPos();
	D3DXVECTOR3 *pCurDir = m_pParent->GetMoveDir();
	vDir.y = 0.f;
	m_fTargetLength = D3DXVec3LengthSq( &vDir );
	D3DXVec3Normalize( &vDir, &vDir );
	if( D3DXVec3LengthSq( &vDir ) <= 0.f ) return;

	if( GetPath( m_pParent->GetDummyPos(), vDir ) == false ) {
		CheckAndAddBlock();
		if( D3DXVec3LengthSq( pCurDir ) <= 0.f ) return;
		*pCurDir = vDir;
		BsAssert( (pCurDir->x == pCurDir->x) && (pCurDir->y == pCurDir->y) && (pCurDir->z == pCurDir->z) );
	}
	else {
		CheckAndAddBlock();
		*pCurDir = m_vResult - m_pParent->GetDummyPos();
		pCurDir->y = 0.f;
		D3DXVec3Normalize( pCurDir, pCurDir );
		m_bCheckNewPath = true;
	}
}

void CShortPathFinder::CheckAndAddBlock()
{
	int nIndex = GetBlockIndex( m_pParent->GetDummyPos() );

	if( m_nVecBlockList.size() == 0 ) m_nVecBlockList.push_back( nIndex );
	else if( nIndex != m_nVecBlockList[0] ) m_nVecBlockList.insert( m_nVecBlockList.begin(), nIndex );

	if( m_nVecBlockList.size() > 2 ) m_nVecBlockList.erase( m_nVecBlockList.end() - 1 );
}

int CShortPathFinder::GetBlockIndex( D3DXVECTOR3 &vPos )
{
	int nX = (int)(vPos.x / 100);
	int nY = (int)(vPos.z / 100);
	return nX + nY * g_FcWorld.GetAttrXSize();
}

void CShortPathFinder::Render()
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
	for( DWORD i=0; i<8; i++ ) pDevice->SetTexture( i, NULL );


	struct LINE_VERTEX
	{
	D3DXVECTOR3 v;
	DWORD       color;
	};
	static LINE_VERTEX pVertices[2];


	pDevice->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );

	// 타겟팅 라인
	pVertices[0].v = m_pParent->GetPos(); pVertices[0].v.y += 100.f;
	pVertices[1].v = *m_pParent->GetMoveTargetPos();	pVertices[1].v.y += 100.f;
	pVertices[0].color = 0xFFFFFFFF;
	pVertices[1].color = 0xFFFFFFFF;
	pDevice->DrawPrimitiveUP( D3DPT_LINELIST, 1, pVertices, sizeof(LINE_VERTEX) );

	if( D3DXVec3LengthSq( &m_pParent->GetCrossVector()->m_ZVector ) > 0.0f ) {
	pVertices[0].v = m_pParent->GetPos();	pVertices[0].v.y += 100.f;
	pVertices[1].v = m_pParent->GetPos() + (m_pParent->GetCrossVector()->m_ZVector*1000.f);	pVertices[1].v.y += 100.f;
	pVertices[0].color = 0xFFFFFF00;
	pVertices[1].color = 0xFFFFFF00;
	pDevice->DrawPrimitiveUP( D3DPT_LINELIST, 1, pVertices, sizeof(LINE_VERTEX) );
	}


	pDevice->SetTransform( D3DTS_WORLD, &matWorld );
	pDevice->EndScene();
	pDevice->RestoreState();
	#endif //_XBOX
	
}

bool CShortPathFinder::IsMoveBlock( D3DXVECTOR3 &vPos )
{
	return IsMoveBlock( (int)vPos.x / MAP_ATTR_SIZE, (int)vPos.z / MAP_ATTR_SIZE );
}

bool CShortPathFinder::IsMoveBlock( int nX, int nY )
{
	if( s_bAttrTable[ g_FcWorld.GetAttr( nX, nY ) ] == false ) return false;
	static BYTE cAttrEx = g_FcWorld.GetAttrEx( nX, nY );
	// 대각선도 못가는데
//	if( (cAttrEx & 0x07) > 0 ) return false;
	if( m_pParent->GetTeam() == 0 && ( cAttrEx & 0x08 ) >> 3 == 1 ) return false;
	if( ( cAttrEx & 0x10 ) >> 4 == 1 ) return false;

	return true;
}

bool CShortPathFinder::IsMoveBlock( float fX, float fY )
{
	return IsMoveBlock( (int)fX / MAP_ATTR_SIZE, (int)fY / MAP_ATTR_SIZE );
}

bool CShortPathFinder::GetPath( const D3DXVECTOR3 &vPos, const D3DXVECTOR3 &vDir )
{
	m_vSource = vPos;
	m_vDirection = vDir;

	m_nCount = 0;

	if( s_bAttrTable[ g_FcWorld.GetAttr( m_vSource ) ] == false ) return false;

	if( IsNoFinder() == true ) {
		return false;
	}
	else {
		static float fRadian[7] = { 0.78539819f, 1.5707964f, 2.3561945f, 3.1415927f, 3.9269910f, 4.7123890f, 5.4977875f };
		int *pnArray;

		int nCurDir = FindNextWayPoint();

		if( nCurDir == 0 ) {
			m_nCurDir = 0;
			m_nVecBlockList.clear();
			pnArray = s_pnArray[m_nPrevDir+1];
			if( IsMoveBlock( m_vSource + ( m_vDirection * MAP_ATTR_SIZE ) ) == true ) {
				return false;
			}
			// 2번 어트류비트일경우 부대중심 체크후에 첫번째 방향을 설정해 준다.
			if( m_BlockAttr == 2 ) {
				D3DXMATRIX matRotate;
				D3DXVECTOR3 vTemp, vTempPos;
				DWORD dwMinLength = -1;
				int nArrayIndex = -1;

				int nX = (int)m_vSource.x / 100;
				int nZ = (int)m_vSource.z / 100;
				vTempPos.x = ( nX * 100 ) + 50.f;
				vTempPos.y = 0.f;
				vTempPos.z = ( nZ * 100 ) + 50.f;
				for( int i=1; i<8; i++ ) {
					D3DXMatrixIdentity( &matRotate );
					D3DXMatrixRotationY( &matRotate, fRadian[ pnArray[i-1] - 1 ] );
					D3DXVec3TransformNormal( &vTemp, &m_vDirection, &matRotate );

					vTemp = vTempPos + ( vTemp * (float)MAP_ATTR_SIZE );

					if( IsMoveBlock( vTemp ) == false ) continue;
					DWORD dwLength = (DWORD)D3DXVec3LengthSq( (D3DXVECTOR3*)&( m_pParent->GetTroop()->GetPos() - vTemp ) );

					if( dwMinLength == -1 ) {
						dwMinLength = dwLength;
						nArrayIndex = pnArray[i-1];
					}
					else {
						if( dwMinLength > dwLength ) {
							dwMinLength = dwLength;
							nArrayIndex = pnArray[i-1];
						}
					}
				}
				if( nArrayIndex == -1 ) {
					pnArray = s_pnArray[0];
				}
				else {
					if( nArrayIndex <= 3 ) {
						pnArray = s_pnArray[1];
					}
					else {
						pnArray = s_pnArray[2];
					}
				}
			}
			else {
				pnArray = s_pnArray[m_nPrevDir+1];
			}

			D3DXMATRIX matRotate;
			D3DXVECTOR3 vTemp, vTempPos;

			int nX = (int)m_vSource.x / 100;
			int nZ = (int)m_vSource.z / 100;
			vTempPos.x = ( nX * 100 ) + 50.f;
			vTempPos.y = 0.f;
			vTempPos.z = ( nZ * 100 ) + 50.f;
			int i = 1;
			for( i=1; i<8; i++ ) {
				D3DXMatrixIdentity( &matRotate );
				D3DXMatrixRotationY( &matRotate, fRadian[ pnArray[i-1] - 1 ] );
				D3DXVec3TransformNormal( &vTemp, &m_vDirection, &matRotate );

				vTemp = vTempPos + ( vTemp * (float)MAP_ATTR_SIZE );

				if( IsMoveBlock( vTemp ) == false ) continue;
				m_vResult = vTemp;
				if( pnArray[i-1] <= 3 ) m_nPrevDir = 0;
				else if( pnArray[i-1] >= 5 ) m_nPrevDir = 1;
				else m_nPrevDir = 2;
				break;
			}
			if( i == 8 ) {
				m_vResult = m_vSource;
				m_nPrevDir = -1;
			}
			return true;
		}
		else {
			D3DXMATRIX matRotate;
			D3DXVECTOR3 vTemp, vTempPos;

			int nX = (int)m_vSource.x / 100;
			int nZ = (int)m_vSource.z / 100;
			vTempPos.x = ( nX * 100 ) + 50.f;
			vTempPos.y = 0.f;
			vTempPos.z = ( nZ * 100 ) + 50.f;

			static int nArray[2][16] = { 
				{ 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7 },
				{ 0, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1 },
			};

			bool bFlag = false;
			int i = m_nCurDir;
			for( i=m_nCurDir; i<16; i++ ) {
				if( nArray[nCurDir-1][i] != 0 ) {
					D3DXMatrixIdentity( &matRotate );
					D3DXMatrixRotationY( &matRotate, fRadian[ nArray[nCurDir-1][i] - 1 ] );
					D3DXVec3TransformNormal( &vTemp, &m_vDirection, &matRotate );
				}
				else {
					vTemp = m_vDirection;
				}
				vTemp = vTempPos + ( vTemp * (float)MAP_ATTR_SIZE );

				if( IsMoveBlock( vTemp ) == false ) continue;
				if( std::find( m_nVecBlockList.begin(), m_nVecBlockList.end(), GetBlockIndex( vTemp ) ) != m_nVecBlockList.end() ) continue;
				m_vResult = vTemp;
				m_nCurDir = i - 1;
				if( m_nCurDir < 0 ) m_nCurDir = 0;
				break;
			}
			if( i == 16 ) {
				m_nCurDir = 0;
				m_nVecBlockList.clear();
				return false;
			}
		}
		m_nPrevDir = nCurDir - 1;
		return true;
	}
}

bool CShortPathFinder::IsNoFinder()
{
	D3DXVECTOR3 vVec = m_vSource;
	m_nCount = 0;

	return FindNextDirBlock( vVec );
}

bool CShortPathFinder::FindNextDirBlock( D3DXVECTOR3 &vPos )
{
	/*	// 속도상..크게 문제 없으면 너치말자..
	int nX = (int)(vPos.x / MAP_ATTR_SIZE);
	int nY = (int)(vPos.y / MAP_ATTR_SIZE);
	if( nX < 0 || nX >= g_FcWorld.GetAttrXSize() ||
	nY < 0 || nY >= g_FcWorld.GetAttrYSize() ) return false;
	*/

	if( m_nCount > 100 ) return true;
	m_nCount++;
	BYTE Attr = g_FcWorld.GetAttr( vPos );
	if( IsMoveBlock( vPos ) == false ) {
		m_BlockAttr = Attr;
		return false;
	}


	float fCurLength = D3DXVec3LengthSq( (D3DXVECTOR3*)&(vPos - m_vSource) );
	//	if( fCurLength >= m_fLength ) return false;
	if( fCurLength >= m_fTargetLength ) return true;

	return FindNextDirBlock( vPos + ( m_vDirection * MAP_ATTR_SIZE ) );
}

bool CShortPathFinder::IsNewPath()
{
	return m_bCheckNewPath;
}

void CShortPathFinder::LockPath( D3DXVECTOR3 &vDir, int nValue )
{
	m_bLockPath = true;
	m_nLockPathTick = nValue;
	m_vLockDir = vDir;
}

int CShortPathFinder::CalcSearchDir( D3DXVECTOR3 *pDir )
{
	int i, nRet;
	float fMaxDot, fDot;
	D3DXVECTOR3 vDir, vOrient( 0.0f, 0.0f, 1.0f );
	D3DXMATRIX matRotate;

	fMaxDot = -FLT_MAX;
	D3DXMatrixRotationY( &matRotate, 45.0f * ( D3DX_PI / 180.0f ) );
	for( i = 0; i < 8; i++ )
	{
		fDot = D3DXVec3Dot( &vOrient, pDir );
		if( fDot > fMaxDot )
		{
			fMaxDot = fDot;
			nRet = i;
		}
		D3DXVec3TransformNormal( &vOrient, &vOrient, &matRotate );
	}

	return nRet;
}

int g_nDirAddX[ 8 ] = { 0, 1, 1, 1, 0, -1, -1, -1 };
int g_nDirAddZ[ 8 ] = { 1, 1, 0, -1, -1, -1, 0, 1 };

bool CShortPathFinder::FindLeftPoint( int &nDir, int &nPosX, int &nPosZ )
{
	int i;

	nDir %= 8;
	for( i = 0; i < 8; i++ )
	{
		if( IsMoveBlock( nPosX + g_nDirAddX[ nDir ], nPosZ + g_nDirAddZ[ nDir ] ) == true )
		{
			nPosX += g_nDirAddX[ nDir ];
			nPosZ += g_nDirAddZ[ nDir ];
//			if( i )
			{
				nDir += 2;
				nDir %= 8;
			}
			return true;
		}
		nDir += 7;
		nDir %= 8;
	}

	return false;
}

bool CShortPathFinder::FindRightPoint( int &nDir, int &nPosX, int &nPosZ )
{
	int i;

	nDir %= 8;
	for( i = 0; i < 8; i++ )
	{
		if( IsMoveBlock( nPosX + g_nDirAddX[ nDir ], nPosZ + g_nDirAddZ[ nDir ] ) == true )
		{
			nPosX += g_nDirAddX[ nDir ];
			nPosZ += g_nDirAddZ[ nDir ];
//			if( i )
			{
				nDir += 6;
				nDir %= 8;
			}
			return true;
		}
		nDir++;
		nDir %= 8;
	}

	return false;
}

int CShortPathFinder::FindNextWayPoint()
{
	int i;
	D3DXVECTOR3 vOrientDir;
	int nStartX, nStartZ;
	int nLeftDir, nRightDir;
	int nLeftX, nLeftZ, nRightX, nRightZ;
	int nSearchCount;
	int nOrientX, nOrientZ;

	nStartX = ( int )( m_pParent->GetDummyPos().x / MAP_ATTR_SIZE ); 
	nStartZ = ( int )( m_pParent->GetDummyPos().z / MAP_ATTR_SIZE );
	nRightX = nLeftX = nStartX;
	nRightZ = nLeftZ = nStartZ;

	vOrientDir = *m_pParent->GetMoveTargetPos() - m_pParent->GetDummyPos();
	vOrientDir.y = 0.0f;
	D3DXVec3Normalize( &vOrientDir, &vOrientDir );

	nLeftDir = CalcSearchDir( &vOrientDir);
	nRightDir = nLeftDir;

	nOrientX = ( int )( m_pParent->GetMoveTargetPos()->x / MAP_ATTR_SIZE ) - ( int )( m_pParent->GetDummyPos().x / MAP_ATTR_SIZE );
	nOrientZ = ( int )( m_pParent->GetMoveTargetPos()->z / MAP_ATTR_SIZE ) - ( int )( m_pParent->GetDummyPos().z / MAP_ATTR_SIZE );

	for( i = 0; i < 8; i++ )
	{
		if( IsMoveBlock( nStartX + g_nDirAddX[ nLeftDir ], nStartZ + g_nDirAddZ[ nLeftDir ] ) == false )
		{
			break;
		}
		nLeftDir += 7;
		nLeftDir %= 8;
	}
	if( i >= 8 )
	{
		return 0;
	}
	for( i = 0; i < 8; i++ )
	{
		if( IsMoveBlock( nStartX + g_nDirAddX[ nRightDir ], nStartZ + g_nDirAddZ[ nRightDir ] ) == false )
		{
			break;
		}
		nRightDir ++;
		nRightDir %= 8;
	}

	nSearchCount = 0;

	while( 1 )
	{
		int nValue2;

		if( !FindLeftPoint( nLeftDir, nLeftX, nLeftZ ) )
		{
			return 0;
		}
		nValue2 = ( nLeftZ - nStartZ ) * nOrientX - ( nLeftX - nStartX ) * nOrientZ;
		if( nSearchCount )
		{
			if( ( abs( nLeftZ - nStartZ ) != nSearchCount + 1 ) && ( abs( nLeftX - nStartX ) != nSearchCount + 1 ) )
			{
				if( ( nLeftX - nStartX ) * nOrientX + ( nLeftZ - nStartZ )  * nOrientZ >= 0 )
				{
					if( nValue2 < 0 )
					{
						return 2;
					}
				}
			}
		}

		if( !FindRightPoint( nRightDir, nRightX, nRightZ ) )
		{
			return 0;
		}
		nValue2 = ( nRightZ - nStartZ ) * nOrientX - ( nRightX - nStartX ) * nOrientZ;
		if( nSearchCount )
		{
			if( ( abs( nRightZ - nStartZ ) != nSearchCount + 1 ) && ( abs( nRightX - nStartX ) != nSearchCount + 1 ) )
			{
				if( ( nRightX - nStartX ) * nOrientX + ( nRightZ - nStartZ )  * nOrientZ >= 0 )
				{
					if( nValue2 > 0 )
					{
						return 1;
					}
				}
			}
		}

		nSearchCount++;
		if( nSearchCount > 1000 )
		{
			break;
		}
	}

	return 0;
}
