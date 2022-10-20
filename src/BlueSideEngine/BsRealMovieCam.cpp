#include "stdafx.h"
#include "BsRealMovieFile.h"
#include "BsKernel.h"

#include "BsRealMovieCam.h"
#include "BsRealMovieConst.h"

extern BOOL	DoesExistFile(const char *lpszFileName);


CRMCamera::CRMCamera()
: m_fTime( -1.f )
, m_nSkinIndex( -1 )
, m_nObjectIndex( -1 )
, m_fFOV( -1 )
, m_nUseOffsetForBC( 0 )
{
	memset( m_szName, 0, eFileNameLength );
	memset( m_szFileName, 0, eFileNameLength );
	memset( m_szFilePath , 0, eFileNameLength );
}

CRMCamera::~CRMCamera()
{
	RemoveCamObject();
	ReleaseAllPath();

	if( m_nSkinIndex != -1 )
	{
		g_BsKernel.ReleaseSkin( m_nSkinIndex );
		m_nSkinIndex = -1;
	}
}

// BC Cam Process 
void	CRMCamera::ProcessBCCamera(CCrossVector& rCamCross,int nTick)
{
	if( nTick < m_BC.GetMaxTime() ) // GetPositionKeyCount() )
	{
		m_BC.ProcessCamera( rCamCross, (float)nTick );

		if( m_nUseOffsetForBC )
		{
			rCamCross.m_PosVector += m_Cross.m_PosVector; // .x += 0.f;
			//rCamCross.m_PosVector.y += 0.f;
			//rCamCross.m_PosVector.z += 0.f;
		}

		m_crossBCSaved = rCamCross;
	}
	else
		rCamCross = m_crossBCSaved;
}

void	CRMCamera::Save(BStream *pStream)
{
	int nLen;

	nLen = strlen( m_szName );
	pStream->Write( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );
	if( nLen )
		pStream->Write( m_szName , nLen , 0 );

	nLen = strlen( m_szFileName );
	pStream->Write( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );
	if( nLen )
		pStream->Write( m_szFileName , nLen , 0 );

	pStream->Write( &m_nLightYaw, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Write( &m_nLightRoll, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Write( &m_nLightPitch, sizeof(int), ENDIAN_FOUR_BYTE );

	pStream->Write( &m_fTime, sizeof(float), ENDIAN_FOUR_BYTE );
	pStream->Write( &m_fFOV, sizeof(float), ENDIAN_FOUR_BYTE );

	pStream->Write( &m_Cross.m_XVector , sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
	pStream->Write( &m_Cross.m_YVector , sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
	pStream->Write( &m_Cross.m_ZVector , sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
	pStream->Write( &m_Cross.m_PosVector , sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );

	// 데이터 확장시.
	//int nChunkCnt = 0;
	//pStream->Write( &nChunkCnt, sizeof(int) , ENDIAN_FOUR_BYTE );

	// Path 개수
	int nCnt = (int)m_CamPathPool.size();
	int nSize = CAMPATH_BLOCK_SIZE; // 뒤에 ( ( sizeof(COLORVERTEX_EX) * 6 ) + sizeof(float) ) 만큼 제거.
	//sizeof(CRMCamPath) - ( ( sizeof(COLORVERTEX_EX) * 6 ) + sizeof(float) ); 

	pStream->Write( &nSize, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Write( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	for( int i = 0 ; i < nCnt ; ++i )
	{
		CRMCamPath* pPath = m_CamPathPool[i];

		pPath->Save( pStream );
		// pStream->Write( pPath, nSize , ENDIAN_FOUR_BYTE );
	}
}

void	CRMCamera::Load(BStream *pStream,int nVer)
{
	int nLen;

	/*
	pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );
	if( nLen )
	pStream->Read( m_szName , nLen , 0 );
	*/

	pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );
	if( nLen )
		pStream->Read( m_szFileName, nLen , 0 );

	pStream->Read( &m_nLightYaw, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Read( &m_nLightRoll, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Read( &m_nLightPitch, sizeof(int), ENDIAN_FOUR_BYTE );

	pStream->Read( &m_fTime, sizeof(float), ENDIAN_FOUR_BYTE );
	pStream->Read( &m_fFOV, sizeof(float), ENDIAN_FOUR_BYTE );

	pStream->Read( &m_Cross.m_XVector , sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
	pStream->Read( &m_Cross.m_YVector , sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
	pStream->Read( &m_Cross.m_ZVector , sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
	pStream->Read( &m_Cross.m_PosVector , sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );

	//int nChunkCnt;
	//pStream->Read( &nChunkCnt , sizeof(int), ENDIAN_FOUR_BYTE );

	// if( nChunkCnt > 0 )
	// {
	//		// Chunk 처리를 한다.
	// }

	int nSize,nCnt;

	pStream->Read( &nSize, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Read( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	for( int i = 0 ; i < nCnt ; ++i )
	{
		CRMCamPath* pPath = new CRMCamPath;

		pPath->Load( pStream, nVer );
		// pStream->Read( pPath, nSize, ENDIAN_FOUR_BYTE );

		m_CamPathPool.push_back( pPath );

		COLORVERTEX_EX temp;

		temp.Vertex = pPath->m_Cross.m_PosVector;
		temp.dwColor = 0x7FFF0000;

		m_ClrVertexPool.push_back( temp );
	}
}

void	CRMCamera::SaveEx(BStream *pStream)
{
	int nCode = 2;
	int nChunk2Cnt = 1;
	int nLen;
	pStream->Write( &nCode, sizeof(int), ENDIAN_FOUR_BYTE );

	nLen = strlen( m_szFilePath );

	pStream->Write( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );

	if( nLen )
	{
		pStream->Write( m_szFilePath, nLen, 0 );
	}

	// Chunk 2 
	pStream->Write( &nChunk2Cnt, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Write( &m_nUseOffsetForBC, sizeof(int), ENDIAN_FOUR_BYTE );
}

void	CRMCamera::LoadEx(BStream *pStream)
{
	int nCode;
	int nLen;

	pStream->Read( &nCode, sizeof(int), ENDIAN_FOUR_BYTE );

	// takogura: prefix bug 5828: using nCode uninitialized -> OK
	for( int i = 0 ; i < nCode; ++i )
	{
		if( i == 0 )	// 첫번째 블럭.
		{
			pStream->Read( &nLen , sizeof(int), ENDIAN_FOUR_BYTE );

			memset( m_szFilePath, 0 , eFileNameLength );

			if( nLen )
			{
				pStream->Read( m_szFilePath, nLen, 0 );
			}
		}
		else if( i == 1 )
		{
			pStream->Read( &nLen , sizeof(int) , ENDIAN_FOUR_BYTE );

			for( int n = 0 ; n < nLen ; ++n )
			{
				// Chunk 2 - block 1.
				pStream->Read( &m_nUseOffsetForBC, sizeof(int), ENDIAN_FOUR_BYTE );
			}
		}
	}
}

void	CRMCamera::CreateCamObject(void)
{
#ifdef _USAGE_TOOL_
	if( m_nObjectIndex == -1 )
	{
		int nPrevSkinIndex = m_nSkinIndex;

		g_BsKernel.chdir( "RealMovie" );
		m_nSkinIndex = g_BsKernel.LoadSkin( -1, "CAMERA.SKIN" );
		g_BsKernel.chdir( ".." );

		if ( nPrevSkinIndex != -1 )
			g_BsKernel.ReleaseSkin( nPrevSkinIndex );

		if( m_nSkinIndex < 0 )
			return;

		m_nObjectIndex = g_BsKernel.CreateStaticObjectFromSkin( m_nSkinIndex );
	}
#else
	m_nSkinIndex = -1;
	m_nObjectIndex = -1;
#endif
}

void	CRMCamera::RemoveCamObject(void)
{
	// TODO: It may be a better idea to not release the skin here in order
	//       to avoid loading & releasing the same skin all the time; however,
	//       for now I am more interested in making sure that the adding of
	//       references and the releasing of references match up.

	if( m_nSkinIndex != -1 )
	{
		g_BsKernel.ReleaseSkin( m_nSkinIndex );
		m_nSkinIndex = -1;
	}

	if( m_nObjectIndex != -1 ) {
		g_BsKernel.DeleteObject( m_nObjectIndex );
		m_nObjectIndex = -1;
	}
}

void	CRMCamera::UpdateObject(float fFrame)
{
	// 실 오브젝트 등록 부분이 필요.
	// D3DXVECTOR3 pos3;

	/*
	int nRet;

	nRet = m_BC.GetPositionVector( m_Cross.m_PosVector ,fFrame);

	if( nRet )
	*/
#ifdef _USAGE_TOOL_
	g_BsKernel.UpdateObject( m_nObjectIndex, m_Cross );
#endif


}

CRMCamPath*	CRMCamera::SetCrossVector(CCrossVector& rCross)
{
	m_Cross = rCross;

	if( !m_ClrVertexPool.size() )
	{
		return InsertNewPath( rCross, m_fFOV );
	}

	return NULL;
}

void	CRMCamera::SetPosition(D3DXVECTOR3& rPos)
{	
	m_Cross.m_PosVector = rPos;		
	m_CamPathPool[0]->m_Cross.m_PosVector = rPos;
	m_CamPathPool[0]->RecalcPivotLine();
}

void	CRMCamera::SetXPos(float fX)
{
	m_Cross.m_PosVector.x = fX;
	m_CamPathPool[0]->m_Cross.m_PosVector.x = fX;
	m_CamPathPool[0]->RecalcPivotLine();
}

void	CRMCamera::SetYPos(float fY)
{
	m_Cross.m_PosVector.y = fY;
	m_CamPathPool[0]->m_Cross.m_PosVector.y = fY;
	m_CamPathPool[0]->RecalcPivotLine();
}

void	CRMCamera::SetZPos(float fZ)
{
	m_Cross.m_PosVector.z = fZ;
	m_CamPathPool[0]->m_Cross.m_PosVector.z = fZ;
	m_CamPathPool[0]->RecalcPivotLine();
}

void	CRMCamera::CalcDuration( float fCurTime )
{
	// 항상 자신의 앞에 것에 세팅.
	int nCnt = (int)m_CamPathPool.size();

	// mruete: prefix bug 404: fixed according to bug comments, and pulled the i == 0 case out of the loop
	if( nCnt > 2 )
	{
		int nCursor = nCnt - 2;
		float fTempTime;
		int k = nCursor + 1; // Cursor
		float fAccumTime;

		fAccumTime = GetStartTime();
		for( int i = 1 ; i < k ; ++i )
		{
			fTempTime = m_CamPathPool[i-1]->m_fDuration;

			if( fTempTime >= 0.f )
				fAccumTime += fTempTime;
		}

		m_CamPathPool[ nCursor ]->m_fDuration = fCurTime - fAccumTime;
		m_CamPathPool[ nCursor ]->m_nFrame = (int)(m_CamPathPool[nCursor]->m_fDuration * eFps40 );
	}
}

void	CRMCamera::RecalcDuration( void )
{
	int nFrame;
	int nCnt = (int)m_CamPathPool.size();
	// --nCnt;

	for( int i = 0 ; i < nCnt ; ++i )
	{
		nFrame = m_CamPathPool[i]->m_nFrame;
		nFrame = ( nFrame == -1 ) ? 0 : nFrame;

		if( i == 0 )
		{
			m_CamPathPool[i]->m_nCumulativeFrame = nFrame;
		}
		else
		{
			m_CamPathPool[i]->m_nCumulativeFrame = 
				m_CamPathPool[i-1]->m_nCumulativeFrame + nFrame;
		}
		/*
		m_CamPathPool[i]->m_fDuration =
		m_CamPathPool[i+1]->m_fCurTime - m_CamPathPool[i]->m_fCurTime;
		m_CamPathPool[i]->m_nFrame = (int)(m_CamPathPool[i]->m_fDuration * 60.f);
		*/
	}
}

/*
void	CRMCamera::RecalcDuration( int nIndex,float fCurTime)
{
int nFrontPathIdx = nIndex - 1;
int nRearPathIdx = nIndex + 1;

float fOldPathTime = m_CamPathPool[nIndex]->m_fDuration;
float fPathTime;

if( nFrontPathIdx == -1 ) // 머리라면.
{
float fOldTime = GetStartTime();

if( fOldPathTime >= 0.f )
{
float fGap = fOldTime - fCurTime;
float fNewDuration = fOldPathTime + fGap;

m_CamPathPool[nIndex]->m_fDuration = fNewDuration;

// fPathTime = fOldTime + fOldPathTime;	// NextPathTime = STime + Path[0].Duration
}

SetStartTime( fCurTime );	// 시작 시간 맞추고.
}
else
{
// 앞쪽 Duration
// float fFrontGap = m_CamPathPool[nFrontPathIdx]->m_fDuration - 
}
}
*/

#ifndef _XBOX

/*
struct COLORVERTEX_EX
{
D3DXVECTOR3 Vertex;
DWORD dwColor;
};

#define D3DFVF_COLORVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)
*/


void	CRMCamera::DrawCamLine(void)
{
	COLORVERTEX_EX		m_vecCamLine[2];

	m_vecCamLine[0].dwColor = 0xFFFFFF00;
	m_vecCamLine[1].dwColor = 0xFFFFFF00;

	/*
	m_vecCamLine[0].Vertex = m_Cross.m_PosVector;
	m_vecCamLine[0].dwColor = 0xFFFFFF00;
	m_vecCamLine[1].Vertex = m_vecCamLine[0].Vertex;
	m_vecCamLine[1].Vertex.y = 0.f;
	*/

	C3DDevice* pBSDevice = g_BsKernel.GetDevice();
	LPDIRECT3DDEVICE9 pD3Device = pBSDevice->GetD3DDevice();

	// pD3Device->DrawPrimitiveUP( D3DPT_LINELIST, 1, m_vecCamLine, sizeof(COLORVERTEX_EX));

	vecCamPath::iterator it = m_CamPathPool.begin();
	vecCamPath::iterator itEnd = m_CamPathPool.end();

	for( ; it != itEnd ; ++it )
	{
		m_vecCamLine[0].Vertex = (*it)->m_Cross.m_PosVector;
		m_vecCamLine[1].Vertex = m_vecCamLine[0].Vertex;
		m_vecCamLine[1].Vertex.y = 0.f;

		pD3Device->DrawPrimitiveUP( D3DPT_LINELIST, 1, m_vecCamLine, sizeof(COLORVERTEX_EX));
	}

	int nCnt = (int)m_ClrVertexPool.size();

	if( nCnt-1 > 0 )
	{
		pD3Device->DrawPrimitiveUP( 
			D3DPT_LINESTRIP, 
			nCnt - 1,
			&m_ClrVertexPool[0],
			sizeof( COLORVERTEX_EX ) );
	}
}

void	CRMCamera::DrawCamPivotLine(void)
{
	C3DDevice* pDevice = g_BsKernel.GetDevice();
	LPDIRECT3DDEVICE9 pD3Device = pDevice->GetD3DDevice();

	/*
	pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE );
	pDevice->SetRenderState( D3DRS_ZENABLE, TRUE ); // D3DZB_TRUE  );
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE );

	pDevice->SetRenderState(D3DRS_LIGHTING,FALSE);
	pDevice->SetRenderState(D3DRS_FOGENABLE,FALSE);
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

	pDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
	pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
	pDevice->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE,D3DMCS_COLOR1);    

	pDevice->GetD3DDevice()->SetFVF(D3DFVF_COLORVERTEX);
	pDevice->GetD3DDevice()->SetVertexShader( NULL );
	pDevice->GetD3DDevice()->SetPixelShader( NULL );
	*/

	vecCamPath::iterator it = m_CamPathPool.begin();
	vecCamPath::iterator itEnd = m_CamPathPool.end();

	for( ; it != itEnd ; ++it )
	{
		pD3Device->DrawPrimitiveUP(D3DPT_LINELIST, 3 , (*it)->m_vecCross, sizeof(COLORVERTEX_EX));
	}

	/*
	pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW );
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	pDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE );
	*/
}

#endif


void	CRMCamera::Play()
{

}

void	CRMCamera::Process()
{

}

void	CRMCamera::LoadBCResource(void)
{
	char szBCFullName[512];

	if( m_szFileName[0] )
	{
		strcpy( szBCFullName, m_szFilePath );
		strcat( szBCFullName, m_szFileName );

		if( DoesExistFile( szBCFullName ) )
		{
			m_BC.Load( szBCFullName );
		}
	}
}

void	CRMCamera::ReleaseAllPath(void)
{
	vecCamPath::iterator it = m_CamPathPool.begin();
	vecCamPath::iterator itEnd = m_CamPathPool.end();

	for( ; it != itEnd ; ++it )
	{
		delete (*it);
	}

	m_CamPathPool.clear();

	m_ClrVertexPool.clear();
}

CRMCamPath*		CRMCamera::InsertNewPath( CCrossVector& rCross )
{
	return InsertNewPath( rCross , m_fFOV );
}

CRMCamPath*		CRMCamera::InsertNewPath( CCrossVector& rCross,float& rfFov )
{
	CRMCamPath* pPath;

	pPath = new CRMCamPath;

	pPath->m_Cross = rCross;
	pPath->m_fFOV = rfFov;
	pPath->RecalcPivotLine();

	if( !m_CamPathPool.size() )
	{
		m_Cross = rCross;
	}

	m_CamPathPool.push_back( pPath );


	COLORVERTEX_EX temp;

	temp.Vertex = rCross.m_PosVector;
	temp.dwColor = 0x7FFF0000;

	m_ClrVertexPool.push_back( temp );

	return pPath; // (int)m_CamPathPool.size();
}

int		CRMCamera::DeletePath( int nIndex )
{
	if( (int)m_CamPathPool.size() > nIndex )
	{
		delete m_CamPathPool[nIndex];
		m_CamPathPool.erase( m_CamPathPool.begin() + nIndex );

		m_ClrVertexPool.erase( m_ClrVertexPool.begin() + nIndex );

		if( nIndex == 0 ) // 첫번째 카메라가 지워짐.
		{
			if( m_CamPathPool.size() )
				m_Cross = m_CamPathPool[0]->m_Cross;
		}

		RecalcDuration();

		return (int)m_CamPathPool.size();
	}

	return -1;
}

void	CRMCamera::SetPathCrossVector(int nIndex,CCrossVector& rCross)
{
	m_CamPathPool[nIndex]->m_Cross = rCross;
	m_ClrVertexPool[nIndex].Vertex = rCross.m_PosVector;

	m_CamPathPool[nIndex]->RecalcPivotLine();

	if( nIndex == 0 )
		m_Cross = rCross;
}

void	CRMCamera::SetPathXPos(int nIndex,float fX)
{
	m_CamPathPool[nIndex]->m_Cross.m_PosVector.x = fX;
	m_ClrVertexPool[nIndex].Vertex.x = fX;
	m_CamPathPool[nIndex]->RecalcPivotLine();

	if( nIndex == 0 )
		m_Cross.m_PosVector.x = fX;
}

void	CRMCamera::SetPathYPos(int nIndex,float fY)
{
	m_CamPathPool[nIndex]->m_Cross.m_PosVector.y = fY;
	m_ClrVertexPool[nIndex].Vertex.y = fY;
	m_CamPathPool[nIndex]->RecalcPivotLine();

	if( nIndex == 0 )
		m_Cross.m_PosVector.y = fY;
}

void	CRMCamera::SetPathZPos(int nIndex,float fZ)
{
	m_CamPathPool[nIndex]->m_Cross.m_PosVector.z = fZ;
	m_ClrVertexPool[nIndex].Vertex.z = fZ;
	m_CamPathPool[nIndex]->RecalcPivotLine();

	if( nIndex == 0 )
		m_Cross.m_PosVector.z = fZ;
}

void	CRMCamera::SetPathPosition(int nIndex,D3DXVECTOR3& rPos)
{
	m_CamPathPool[nIndex]->m_Cross.m_PosVector = rPos;
	m_ClrVertexPool[nIndex].Vertex = rPos;
	m_CamPathPool[nIndex]->RecalcPivotLine();

	if( nIndex == 0 )
		m_Cross.m_PosVector = rPos;
}

int		CRMCamera::InsertAPathBeforeSelectedPath(int nIndex)
{
	vecCamPath::iterator it = m_CamPathPool.begin() + nIndex;

	if( it != m_CamPathPool.end() )
	{
		CRMCamPath* pPath;

		pPath = new CRMCamPath;


		// 위치 지정.
		D3DXVECTOR3& vecBefore = GetPathPosition( nIndex-1 );
		D3DXVECTOR3& vecAfter = GetPathPosition( nIndex );
		D3DXVECTOR3 vecNewPos;

		CRMCamPath* pBeforePath = GetPathPtr( nIndex - 1 );
		CRMCamPath* pAtferPath = GetPathPtr( nIndex );

		vecNewPos = ( vecBefore + vecAfter ) / 2;

		pPath->m_Cross.SetPosition( vecNewPos );

		pPath->m_nPitch = ( pBeforePath->m_nPitch + pAtferPath->m_nPitch ) >> 1;
		pPath->m_nYaw = ( pBeforePath->m_nYaw + pAtferPath->m_nYaw ) >> 1;
		pPath->m_nRoll = ( pBeforePath->m_nRoll + pAtferPath->m_nRoll ) >> 1;

		pPath->ApplyYawRollPitch();
		pPath->RecalcPivotLine();

		pPath->m_fDuration = pBeforePath->m_fDuration * 0.5f;
		pBeforePath->m_fDuration *= 0.5f;

		m_CamPathPool.insert( it, pPath );

		COLORVERTEX_EX temp;

		temp.Vertex = vecNewPos;
		temp.dwColor = 0x7FFF0000;

		vecClrVertex::iterator itClr = m_ClrVertexPool.begin() + nIndex;

		m_ClrVertexPool.insert( itClr , temp );


		return 1;
	}

	return 0;
}

VOID	CRMCamera::SyncDrawLine(int nIndex)
{
	m_ClrVertexPool[nIndex].Vertex = m_CamPathPool[nIndex]->m_Cross.m_PosVector;
	m_ClrVertexPool[nIndex].Vertex = m_CamPathPool[nIndex]->m_Cross.m_PosVector;
	m_CamPathPool[nIndex]->RecalcPivotLine();

	if( nIndex == 0 )
		m_Cross.m_PosVector = m_CamPathPool[nIndex]->m_Cross.m_PosVector;
}

float	CRMCamera::GetAllPathsDuration(void)
{
	float fAllDur = 0.f;

	vecCamPath::iterator it = m_CamPathPool.begin();
	vecCamPath::iterator itEnd = m_CamPathPool.end();

	for( ; it != itEnd ; ++it )
	{
		if( (*it)->m_fDuration >= 0.f )
		{
			fAllDur += (*it)->m_fDuration;
		}
	}

	return fAllDur;
}

#ifdef _USAGE_TOOL_

extern VOID	ChangeNewTickIntoTime(DWORD& dwTick,float& fTime);

void	CRMCamera::ChangeFPS(void)
{
	DWORD dwTick = (DWORD)( m_fTime * eFps60 );
	ChangeNewTickIntoTime( dwTick, m_fTime );

	float fOldFrame;
	float fNewFrame;
	int nCnt = (int)m_CamPathPool.size();
	--nCnt;

	for( int i = 0 ; i < nCnt ; ++i )
	{
		fOldFrame = (float)( m_CamPathPool[i]->m_nFrame );
		fNewFrame = ( fOldFrame * eFps40 ) / eFps60;

		m_CamPathPool[i]->m_nFrame = (int)fNewFrame;
	}

	RecalcDuration();
}
#endif

void	CRMCamPath::Save( BStream *pStream )
{
	pStream->Write( &m_Cross.m_XVector, sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
	pStream->Write( &m_Cross.m_YVector, sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
	pStream->Write( &m_Cross.m_ZVector, sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
	pStream->Write( &m_Cross.m_PosVector, sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );

	pStream->Write( &m_fFOV, sizeof(float), ENDIAN_FOUR_BYTE );
	pStream->Write( &m_fDuration, sizeof(float), ENDIAN_FOUR_BYTE );

	pStream->Write( &m_nYaw, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Write( &m_nRoll, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Write( &m_nPitch, sizeof(int), ENDIAN_FOUR_BYTE );

	pStream->Write( &m_vecTarget, sizeof(D3DXVECTOR3) , ENDIAN_FOUR_BYTE );

	pStream->Write( &m_nSWayType, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Write( &m_fRoll, sizeof(float), ENDIAN_FOUR_BYTE );
	pStream->Write( &m_nMoveType, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Write( &m_nFrame, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Write( &m_nCumulativeFrame, sizeof(int), ENDIAN_FOUR_BYTE );
}

void	CRMCamPath::Load( BStream *pStream, int nVer )
{
	pStream->Read( &m_Cross.m_XVector, sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
	pStream->Read( &m_Cross.m_YVector, sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
	pStream->Read( &m_Cross.m_ZVector, sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
	pStream->Read( &m_Cross.m_PosVector, sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );

	pStream->Read( &m_fFOV, sizeof(float), ENDIAN_FOUR_BYTE );
	pStream->Read( &m_fDuration, sizeof(float), ENDIAN_FOUR_BYTE );

	pStream->Read( &m_nYaw, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Read( &m_nRoll, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Read( &m_nPitch, sizeof(int), ENDIAN_FOUR_BYTE );

	pStream->Read( &m_vecTarget, sizeof(D3DXVECTOR3) , ENDIAN_FOUR_BYTE );

	pStream->Read( &m_nSWayType, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Read( &m_fRoll, sizeof(float), ENDIAN_FOUR_BYTE );
	pStream->Read( &m_nMoveType, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Read( &m_nFrame, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Read( &m_nCumulativeFrame, sizeof(int), ENDIAN_FOUR_BYTE );

	if( nVer == eOldRtmFileVersion && m_nFrame == -1 )
	{
		m_nFrame = (int)(m_fDuration * eFps60 );
	}
	else if( nVer == eNewRtmFileVersion && m_nFrame == -1 )
	{
		m_nFrame = (int)(m_fDuration * eFps40 );
	}

	RecalcPivotLine();
}
