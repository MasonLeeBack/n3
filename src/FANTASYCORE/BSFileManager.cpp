#include "stdafx.h"

#ifdef _USAGE_TOOL_
	#include "..\BSMapTool.h"
	#include "BSMisc.h"

	#include "MapNode.h"

	#include ".\Model\PropModel.h"
	#include ".\Model\PropManager.h"

	#include ".\Model\BSAreaNode.h"
	#include ".\Model\BSTroopNode.h"
	#include ".\Model\BSManagerT.h"

	#include ".\Model\MapAttrNode.h"
	#include ".\Model\BSLineManager.h"
	#include ".\Model\BSPathManager.h"
#else
#endif //_USAGE_TOOL_

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

#include "BSFileManager.h"

#include "bstreamext.h"
#include "BsKernel.h"

int		CalcBitCnt(int nValue)
{
	int nCnt = 0;

	for( int i = 0 ; i < 32 ; i++ )
	{
		if( nValue & 0x01 )
			nCnt++;

		nValue >>= 1;
	}

	return nCnt;
}


CBSMapCore::CBSMapCore() :
	m_nXSize( 0 ),
	m_nYSize( 0 ),
	m_nHeightXSize( 0 ),
	m_nHeightYSize( 0 ),
	m_pHeightData( NULL ) ,
	m_pdwMulColor( NULL ) ,
    m_pdwAddColor( NULL ) ,
	m_nWaterCount( 0 ) ,
	m_pWaterHeight( NULL ),
	m_bHasExAttrMap( TRUE )
{
	int i;

#ifdef _USAGE_TOOL_
	m_pPropManager = NULL;
	m_pLightCross = NULL;

	memset( &m_ResInfo , 0 , sizeof(RESOURCE_INFO) );
#endif

	for(i=0;i<LAYER_MAX;i++){
		m_szLayer[i]=new char[STRING_MAX];
	}

	for( i = 0 ; i < LAYER_ATTR_MAX ; ++i )
	{
		m_nLayerAttr[i] = i;
	}
	
	InitLayerInfo();

	m_SkyboxInfo.nLen = 0;
	m_SkyboxInfo.m_pszFileName = NULL;

	m_CubeMapInfo.nLen = 0;
	m_CubeMapInfo.m_pszFileName = NULL;

	m_AIFileInfo.nLen = 0;
	m_AIFileInfo.m_pszFileName = NULL;

	m_TextFileInfo.nLen = 0;
	m_TextFileInfo.m_pszFileName = NULL;

	m_CameraFileInfo.nLen = 0;
	m_CameraFileInfo.m_pszFileName = NULL;

	m_LightMapFileInfo.nLen = 0;
	m_LightMapFileInfo.m_pszFileName = NULL;

	m_BillboardDDSInfo.nLen = 0;
	m_BillboardDDSInfo.m_pszFileName = NULL;

	m_nBillboardIndexGap = 0;

#ifndef _USAGE_TOOL_
	m_nAttrXSize = 0;
	m_nAttrYSize = 0;
	m_pAttrMap = NULL;
#endif

	m_pAttrMapEx = NULL;

	m_nNavEventCount = 0;
	m_pNavEventIndexList = NULL;

	m_nColEventCount = 0;
	m_pColEventIndexList = NULL;

	m_pAttrByteMap = NULL;
	m_pZoneManager = NULL;
	m_pTempAttrBuffer = NULL;
}

CBSMapCore::~CBSMapCore()
{
	int i;

	for(i=0;i<LAYER_MAX;i++){
		delete [] m_szLayer[i];
	}
	ReleaseHeightMap();

	m_PropList.clear();
	m_BillboardList.clear();
	m_TroopList.clear();
	m_AreaList.clear();

	m_NavList.clear();
	m_NavIndexList.clear();

	m_ColList.clear();
	m_ColIndexList.clear();

	m_SkinIndexList.clear();

	ClearNavEventIndexBuffer();
	ClearColEventIndexBuffer();

	ClearSkyboxName();
	ClearCubeMapName();
	ClearAIFileName();
	ClearTextFileName();
	ClearCameraFileName();
	ClearLightMapFileName();
	ClearBillboardDDSName();

	if( m_pdwAddColor ) delete []m_pdwAddColor;
	if( m_pdwMulColor ) delete []m_pdwMulColor;

#ifndef _USAGE_TOOL_

	if( m_pAttrMap )
	{
		delete [] m_pAttrMap;
		m_pAttrMap = NULL;
	}

	m_nAttrXSize = 0;
	m_nAttrYSize = 0;

	if( m_pAttrByteMap )
	{
		delete [] m_pAttrByteMap;
		m_pAttrByteMap = NULL;
	}

	m_nABXSize = 0;
	m_nABYSize = 0;

	if( m_pZoneManager )
		delete m_pZoneManager;
#endif

	DeleteAttrExBuffer();
}

void	CBSMapCore::DeleteContents(void)
{
	ReleaseHeightMap();
	ReleaseColorMap();

	ClearSkyboxName();
	ClearCubeMapName();
	ClearAIFileName();
	ClearTextFileName();
	ClearCameraFileName();
	ClearLightMapFileName();
	ClearBillboardDDSName();

	m_nBillboardIndexGap = 0;

	// 길찾기용.
	m_NavList.clear();
	m_NavIndexList.clear();
	ClearNavEventIndexBuffer();

	// Collision
	m_ColList.clear();
	m_ColIndexList.clear();
	ClearColEventIndexBuffer();

	m_SkinIndexList.clear();
	m_nWaterCount = 0;

	if( m_pWaterHeight )
	{
		delete [] m_pWaterHeight;
		m_pWaterHeight = NULL;
	}

	for( int i = 0 ; i < LAYER_ATTR_MAX ; ++i )
	{
		m_nLayerAttr[i] = i;
	}

	if( m_pZoneManager )
	{
		delete m_pZoneManager;
		m_pZoneManager = NULL;
	}

	DeleteAttrExBuffer();
	m_bHasExAttrMap = TRUE;
}


void	CBSMapCore::SetMapFileInfo( char* pFileName )
{
	strcpy( m_cMapFileName, pFileName );
}

void	CBSMapCore::SetSkyboxName(char *szFilename)
{
	if( szFilename == NULL || szFilename[0] == NULL )
		return;

	ClearSkyboxName();

	int nSize = strlen(szFilename) + 1;
	m_SkyboxInfo.m_pszFileName = new char[ nSize ];
	memset( m_SkyboxInfo.m_pszFileName , 0 , nSize );
	m_SkyboxInfo.nLen = nSize;

	strcpy_s( m_SkyboxInfo.m_pszFileName, nSize, szFilename ); //aleksger - safe string
}

const char*	CBSMapCore::GetSkyboxName(void)
{
	return m_SkyboxInfo.m_pszFileName;
}

void	CBSMapCore::ClearSkyboxName(void)
{
	if( m_SkyboxInfo.m_pszFileName )
	{
		delete [] m_SkyboxInfo.m_pszFileName;
		m_SkyboxInfo.m_pszFileName = NULL;
		m_SkyboxInfo.nLen = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
//
// Cube Map Name 관련
void	CBSMapCore::SetCubeMapName(char *szFilename)
{
	if( szFilename == NULL || szFilename[0] == NULL )
		return;

	ClearCubeMapName();

	int nSize = strlen( szFilename ) + 1;
	m_CubeMapInfo.m_pszFileName = new char[ nSize ];
	memset( m_CubeMapInfo.m_pszFileName, 0 , nSize );
	m_CubeMapInfo.nLen = nSize;

	strcpy_s( m_CubeMapInfo.m_pszFileName, nSize, szFilename ); //aleksger - safe string
}

const char* CBSMapCore::GetCubeMapName(void)
{
	return m_CubeMapInfo.m_pszFileName;
}

void	CBSMapCore::ClearCubeMapName(void)
{
	if( m_CubeMapInfo.m_pszFileName )
	{
		delete [] m_CubeMapInfo.m_pszFileName;
		m_CubeMapInfo.m_pszFileName = NULL;
		m_CubeMapInfo.nLen = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
//
// AI File 관련
void	CBSMapCore::SetAIFileName(char *szFilename)
{
	if( szFilename == NULL || szFilename[0] == NULL )
		return;

	ClearAIFileName();

	int nSize = strlen( szFilename ) + 1;
	m_AIFileInfo.m_pszFileName = new char[ nSize ];
	memset( m_AIFileInfo.m_pszFileName, 0 , nSize );
	m_AIFileInfo.nLen = nSize;

	strcpy_s( m_AIFileInfo.m_pszFileName, nSize, szFilename ); //aleksger - safe string
}

const char* CBSMapCore::GetAIFileName(void)
{
	return m_AIFileInfo.m_pszFileName;
}

void	CBSMapCore::ClearAIFileName(void)
{
	if( m_AIFileInfo.m_pszFileName )
	{
		delete [] m_AIFileInfo.m_pszFileName;
		m_AIFileInfo.m_pszFileName = NULL;
		m_AIFileInfo.nLen = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
//
// Text File 관련
void	CBSMapCore::SetTextFileName(char *szFilename)
{
	if( szFilename == NULL || szFilename[0] == NULL )
		return;

	ClearTextFileName();

	int nSize = strlen( szFilename ) + 1;
	m_TextFileInfo.m_pszFileName = new char[ nSize ];
	memset( m_TextFileInfo.m_pszFileName, 0 , nSize );
	m_TextFileInfo.nLen = nSize;

	strcpy_s( m_TextFileInfo.m_pszFileName, nSize, szFilename ); //aleksger - safe string
}

const char* CBSMapCore::GetTextFileName(void)
{
	return m_TextFileInfo.m_pszFileName;
}

void	CBSMapCore::ClearTextFileName(void)
{
	if( m_TextFileInfo.m_pszFileName )
	{
		delete [] m_TextFileInfo.m_pszFileName;
		m_TextFileInfo.m_pszFileName = NULL;
		m_TextFileInfo.nLen = 0;
	}
}

// Camera File 관련
void	CBSMapCore::SetCameraFileName(char *szFilename)
{
	if( szFilename == NULL || szFilename[0] == NULL )
		return;

	ClearCameraFileName();

	int nSize = strlen( szFilename ) + 1;
	m_CameraFileInfo.m_pszFileName = new char[ nSize ];
	memset( m_CameraFileInfo.m_pszFileName, 0 , nSize );
	m_CameraFileInfo.nLen = nSize;

	strcpy_s( m_CameraFileInfo.m_pszFileName, nSize, szFilename ); //aleksger - safe string
}

const char* CBSMapCore::GetCameraFileName(void)
{
	return m_CameraFileInfo.m_pszFileName;
}

void	CBSMapCore::ClearCameraFileName(void)
{
	if( m_CameraFileInfo.m_pszFileName )
	{
		delete [] m_CameraFileInfo.m_pszFileName;
		m_CameraFileInfo.m_pszFileName = NULL;
		m_CameraFileInfo.nLen = 0;
	}
}

void	CBSMapCore::SetLightMapFileName(char *szFilename)
{
	ClearLightMapFileName();

	if( szFilename == NULL || szFilename[0] == NULL )
	{
		return;
	}

	// ClearLightMapFileName();

	int nSize = strlen( szFilename ) + 1;
	m_LightMapFileInfo.m_pszFileName = new char[ nSize ];
	memset( m_LightMapFileInfo.m_pszFileName, 0 , nSize );
	m_LightMapFileInfo.nLen = nSize;

	strcpy_s(m_LightMapFileInfo.m_pszFileName, nSize, szFilename ); //aleksger - safe string

}

const char* CBSMapCore::GetLightMapFileName(void)
{
	return m_LightMapFileInfo.m_pszFileName;
}

void	CBSMapCore::ClearLightMapFileName(void)
{
	if( m_LightMapFileInfo.m_pszFileName )
	{
		delete [] m_LightMapFileInfo.m_pszFileName;
		m_LightMapFileInfo.m_pszFileName = NULL;
		m_LightMapFileInfo.nLen = 0;
	}
}

void	CBSMapCore::SetBillboardDDSName(char* szFilename)
{
	ClearBillboardDDSName();

	if( szFilename == NULL || szFilename[0] == NULL )
	{
#ifdef _USAGE_TOOL_
		CheckBillboardDDS();
#endif
		return;
	}

	int nSize = strlen( szFilename ) + 1;
	m_BillboardDDSInfo.m_pszFileName = new char[ nSize ];
	memset( m_BillboardDDSInfo.m_pszFileName, 0 , nSize );
	m_BillboardDDSInfo.nLen = nSize;

	strcpy_s( m_BillboardDDSInfo.m_pszFileName, nSize, szFilename ); //aleksger - safe string
	// 툴에서만
#ifdef _USAGE_TOOL_
	CheckBillboardDDS();
#endif
}

void	CBSMapCore::CheckBillboardDDS(void)
{
	if( m_BillboardDDSInfo.m_pszFileName == NULL || m_BillboardDDSInfo.m_pszFileName[0] == NULL )
	{
		m_nBillboardIndexGap = 0;
		return;
	}

	if( _stricmp( "tile_bb_fm.dds" , m_BillboardDDSInfo.m_pszFileName ) == 0 )
		m_nBillboardIndexGap = 2900;
	else if( _stricmp( "tile_bb_oc.dds" , m_BillboardDDSInfo.m_pszFileName ) == 0 )
		m_nBillboardIndexGap = 2917;
	else if( _stricmp( "tile_bb_gv.dds" , m_BillboardDDSInfo.m_pszFileName ) == 0 )
		m_nBillboardIndexGap = 2921;
	else if( _stricmp( "tile_bb_mp.dds" , m_BillboardDDSInfo.m_pszFileName ) == 0 )
		m_nBillboardIndexGap = 2926;
	else
		m_nBillboardIndexGap = 0;
}

const char* CBSMapCore::GetBillboardDDSName(void)
{
	return m_BillboardDDSInfo.m_pszFileName;
}

void	CBSMapCore::ClearBillboardDDSName(void)
{
	if( m_BillboardDDSInfo.m_pszFileName )
	{
		delete [] m_BillboardDDSInfo.m_pszFileName;
		m_BillboardDDSInfo.m_pszFileName = NULL;
		m_BillboardDDSInfo.nLen = 0;
	}
}


void	CBSMapCore::InitHeightMap(int cx,int cy)
{
	if( !m_pHeightData )
	{
		m_pHeightData = new short[ (cx+1) * (cy+1) ];

		m_nHeightXSize = cx+1;
		m_nHeightYSize = cy+1;

		memset( m_pHeightData , 0 , sizeof(short) * ( m_nHeightXSize * m_nHeightYSize ) );

#ifdef _USAGE_TOOL_
		CBsHFWorld* pWorld = (CBsHFWorld*)g_BsKernel.GetWorld();

		if( pWorld )
			pWorld->SetHeightBufferPtr( m_pHeightData );
#endif
	}
}

void	CBSMapCore::ResetHeightMap(void)
{
	ReleaseHeightMap();
	InitHeightMap( m_nXSize , m_nYSize );
}

void	CBSMapCore::ReleaseHeightMap(void)
{
	if( m_pHeightData )
	{
		delete [] m_pHeightData;
		m_pHeightData = NULL;

		m_nHeightXSize = 0;
		m_nHeightYSize = 0;
	}
}

void	CBSMapCore::InitColorMap(int cx,int cy)
{
	InitBrightMap( cx, cy );
	InitAddColorMap( cx, cy );
}

void	CBSMapCore::InitBrightMap(int cx,int cy)
{
	int nSize = (cx+1) * (cy+1);

	if( !m_pdwMulColor )
	{
		m_pdwMulColor = new DWORD[ nSize ];
		memset( m_pdwMulColor , 0xFF , sizeof(DWORD) * nSize );

#ifdef _USAGE_TOOL_
		CBsHFWorld* pWorld = (CBsHFWorld*)g_BsKernel.GetWorld();

		if( pWorld )
			pWorld->SetMulColorBufferPtr( m_pdwMulColor );
#endif


	}
}

void	CBSMapCore::ReleaseBrightMap(void)
{
	if( m_pdwMulColor )
	{
		delete [] m_pdwMulColor;
		m_pdwMulColor = NULL;
	}
}

void	CBSMapCore::InitAddColorMap(int cx,int cy)
{
	int nSize = (cx+1) * (cy+1);

	if( !m_pdwAddColor )
	{
		m_pdwAddColor = new DWORD[ nSize ];
		memset( m_pdwAddColor , 0x00 , sizeof(DWORD) * nSize );

#ifdef _USAGE_TOOL_
		CBsHFWorld* pWorld = (CBsHFWorld*)g_BsKernel.GetWorld();

		if( pWorld )
			pWorld->SetAddColorBufferPtr( m_pdwAddColor );
#endif
	}
}

void	CBSMapCore::ReleaseAddColorMap(void)
{
	if( m_pdwAddColor )
	{
		delete [] m_pdwAddColor;
		m_pdwAddColor = NULL;
	}
}

void	CBSMapCore::ReleaseColorMap(void)
{
	ReleaseBrightMap();
	ReleaseAddColorMap();
}


void	CBSMapCore::InitLayerInfo(void)
{
	int i;

	for(i=0;i<LAYER_MAX;i++){
		memset( m_szLayer[i] , 0 , sizeof(TCHAR) * STRING_MAX );
	}
}

bool	CBSMapCore::ClearNavEventIndexBuffer(void)
{
	m_nNavEventCount = 0;

	if( m_pNavEventIndexList )
	{
		delete [] m_pNavEventIndexList;
		m_pNavEventIndexList = NULL;	
		return true;
	}
	return false;
}

// 갯수만큼 버퍼 만든다.
bool	CBSMapCore::CreateNavEventIndexBuffer(int nCount)	
{
	ClearNavEventIndexBuffer();
	m_nNavEventCount = nCount;

	if( nCount > 0 )
	{
		m_pNavEventIndexList = new intVector[ nCount ];

		return true;
	}

	return false;
}

bool	CBSMapCore::ClearColEventIndexBuffer(void)
{
	m_nColEventCount = 0;

	if( m_pColEventIndexList )
	{
		delete [] m_pColEventIndexList;
		m_pColEventIndexList = NULL;	
		return true;
	}
	return false;
}

bool	CBSMapCore::CreateColEventIndexBuffer(int nCount)
{
	if( nCount > 0 )
	{
		ClearColEventIndexBuffer();

		m_pColEventIndexList = new intVector[ nCount ];
		m_nColEventCount = nCount;

		return true;
	}

	return false;
}





#ifdef _USAGE_TOOL_

bool	CBSMapCore::SetNavList(int nVecCnt,int nIdxCnt,D3DXVECTOR3* pSrc,int* pIdxSrc)
{
	m_NavList.clear();
	m_NavIndexList.clear();

	if( nVecCnt <= 0 || nIdxCnt <= 0 )
		return false;	// 문제 있음.

	m_NavList.resize( nVecCnt );
	m_NavIndexList.resize( nIdxCnt );

	// 복사
	memcpy( &m_NavList[0], pSrc, sizeof(D3DXVECTOR3) * nVecCnt );
	memcpy( &m_NavIndexList[0], pIdxSrc, sizeof(int) * nIdxCnt );

	// 높이를 적용.

	for( int i = 0 ; i < nVecCnt ; ++i )
	{
		m_NavList[i].y = GetHeight( m_NavList[i].x, m_NavList[i].z , m_pHeightData );
	}


	return true;
}


// 1) nIndex : 저장할 곳의 버퍼 인덱스
// 2) pIdxBuffer : 소스 인덱스가 저장되어 있는 버퍼
// 3) nCount : 소스 인덱스의 갯수  Face * 3
bool	CBSMapCore::SetNavEventIndexBuffer(int nIndex,int* pIdxBuffer,int nCount)
{
	if( nIndex < m_nNavEventCount )
	{
		m_pNavEventIndexList[nIndex].clear();

		if( nCount > 0 )
		{
			m_pNavEventIndexList[nIndex].resize( nCount );

			memcpy( &m_pNavEventIndexList[nIndex][0], pIdxBuffer, sizeof(int)*nCount);
			return true;
		}
	}

	return false;
}

bool	CBSMapCore::SetColList(int nVecCnt,int nIdxCnt,D3DXVECTOR3* pSrc,int* pIdxSrc)
{
	m_ColList.clear();
	m_ColIndexList.clear();

	if( nVecCnt <= 0 || nIdxCnt <= 0 )
		return false;	// 문제 있음.

	m_ColList.resize( nVecCnt );
	m_ColIndexList.resize( nIdxCnt );

	// 복사
	memcpy( &m_ColList[0], pSrc, sizeof(D3DXVECTOR3) * nVecCnt );
	memcpy( &m_ColIndexList[0], pIdxSrc, sizeof(int) * nIdxCnt );

	// 높이를 적용.

	for( int i = 0 ; i < nVecCnt ; ++i )
	{
		m_ColList[i].y = GetHeight( m_ColList[i].x, m_ColList[i].z , m_pHeightData );
	}

	return true;
}

bool	CBSMapCore::SetColBuffer(int nVecCnt,D3DXVECTOR3* pSrc)
{
	m_ColList.clear();

	if( nVecCnt <= 0 )
		return false;	// 문제 있음.

	m_ColList.resize( nVecCnt );
	memcpy( &m_ColList[0], pSrc, sizeof(D3DXVECTOR3) * nVecCnt );

	for( int i = 0 ; i < nVecCnt ; ++i )
	{
		m_ColList[i].y = GetHeight( m_ColList[i].x, m_ColList[i].z , m_pHeightData );
	}

	return true;
}

bool	CBSMapCore::SetColEventIndexBuffer(int nIndex,int* pIdxBuffer,int nCount)
{
	if( nIndex < m_nColEventCount )
	{
		m_pColEventIndexList[nIndex].clear();

		if( nCount > 0 )
		{
			m_pColEventIndexList[nIndex].resize( nCount );

			memcpy( &m_pColEventIndexList[nIndex][0], pIdxBuffer, sizeof(int)*nCount);
			return true;
		}
	}

	return false;
}





///////////////////////////////////////////////////////////////////////////////

bool	CBSMapCore::SaveTerrain(CFile *fp)
{
	int i;
	int nOffsetTbl[32] = { 0,};
	char szType[4] = "BST";
	int nVersion = FC_MAP_VER;

	// 1) Type
	fp->Write( szType , 4 );

	// 2)
	nVersion = ByteSwap32( nVersion );
	fp->Write( &nVersion , sizeof(int));


	// Offset Table Count
	int nTblMask = 
		MAPID_MAININFO|MAPID_HEIGHT|MAPID_BRIGHT|MAPID_COLOR|MAPID_LIGHT|MAPID_PROP|MAPID_ATTRIBUTE|MAPID_MAPSET|MAPID_BILLBOARD;
	int nTblMaskForSave = nTblMask;

	//
	int nTblCnt = 0;
	nTblCnt = CalcBitCnt( nTblMask );

	int nTblCntForSave = nTblCnt;	// int nSaveBlockCnt = GetBitCnt( nTblMask );
	int nEmptyValue = 0;

	// 3)
	nTblMaskForSave = ByteSwap32( nTblMaskForSave );
	fp->Write( &nTblMaskForSave , sizeof(int));

	// 4)
	nTblCntForSave = ByteSwap32( nTblCntForSave );
	fp->Write( &nTblCntForSave , sizeof(int));

	DWORD dwTempPos = (DWORD)fp->GetPosition();


	// 5)
	for( i = 0 ; i < nTblCnt ; i++ )
	{
		fp->Write( &nEmptyValue , sizeof(int));
	}

	memset( nOffsetTbl , 0x00 , sizeof( nOffsetTbl ) );

	int nCnt = 0;
	int nCurMask = 1;

	for( i = 0 ; i < 32 ; i++ , nCurMask <<= 1 )
	{
		// Mask 채크.
		if( nTblMask & nCurMask )
		{
			switch( (nTblMask & nCurMask ) )
			{
			case MAPID_MAININFO :
				nOffsetTbl[i] = (int)fp->GetPosition();
				SaveMapInfo( fp );
				SaveResInfo( fp );
				break;
			case MAPID_HEIGHT :
				nOffsetTbl[i] = (int)fp->GetPosition();
				SaveHeight( fp );
				break;
			case MAPID_BRIGHT :
				nOffsetTbl[i] = (int)fp->GetPosition();
				SaveBrightMap( fp );
				break;
			case MAPID_COLOR :
				nOffsetTbl[i] = (int)fp->GetPosition();
				SaveColorMap( fp );
				break;
			case MAPID_LIGHT :
				nOffsetTbl[i] = (int)fp->GetPosition();
				SaveLightInfo( fp );
				break;
			case MAPID_PROP :
				nOffsetTbl[i] = (int)fp->GetPosition();
				SaveProp( fp );
				SavePropExtensionData( fp );
				break;
			case MAPID_ATTRIBUTE :
				nOffsetTbl[i] = (int)fp->GetPosition();
				SaveAttributeMap( fp );
				break;
			case MAPID_MAPSET :
				nOffsetTbl[i] = (int)fp->GetPosition();
				SaveMapSet( fp );
				break;
			case MAPID_BILLBOARD :
				nOffsetTbl[i] = (int)fp->GetPosition();
				SaveBillboard( fp );
				break;
			case MAPID_BILLBOARDEX :
				nOffsetTbl[i] = (int)fp->GetPosition();
				SaveBillboardEx( fp );
				break;
			/*case MAPID_ATTRIBUTEEX :
				nOffsetTbl[i] = (int)fp->GetPosition();
				SaveAttributeMapEx( fp );
				break;*/
			}
			nCnt++;
			if( nCnt == nTblCnt )
				break;
		}
	}

	// 테이블 주소 설정.
	fp->Seek( dwTempPos , CFile::begin );

	nCnt = 0;
	for( i = 0 ; i < 32 ; i++ )
	{
		if( nOffsetTbl[i] )
		{
			int nTemp = ByteSwap32( nOffsetTbl[i] );
			fp->Write( &nTemp , sizeof(int));

			nCnt++;
			if( nCnt == nTblCnt )
				break;
		}
	}

	return true;
}

// #define SAVE_MASK 0x0000000F
// |MAPID_PROP|MAPID_LIGHT|MAPID_AREA|
// MAPID_TROOP|MAPID_MAPSET|MAPID_WATER|MAPID_PATH
// MAPID_NAVMESH|MAPID_NAVEVENT|MAPID_COLLISION|MAPID_SKINLIST
#define SAVE_MASK (0x0000FFFF|MAPID_ATTREXMAP|MAPID_BILLBOARD|MAPID_PROPZONEDATA|MAPID_ATTRIBUTEEX)


bool	CBSMapCore::SaveFile(CFile *fp)
{
	int i;
	int nOffsetTbl[32] = { 0,};
	char szType[4] = "FCM";
	int nVersion = FC_MAP_VER;

	// 1) 
	fp->Write( szType , 4 );

	// 2)
	nVersion = ByteSwap32( nVersion );
	fp->Write( &nVersion , sizeof(int));


	// Offset Table Count
	int nTblMask = SAVE_MASK;
	int nTblMaskForSave = nTblMask;

	//
	int nTblCnt = 0;
	nTblCnt = CalcBitCnt( nTblMask );

	int nTblCntForSave = nTblCnt;	// int nSaveBlockCnt = GetBitCnt( nTblMask );
	int nEmptyValue = 0;

	// 3)
	nTblMaskForSave = ByteSwap32( nTblMaskForSave );
	fp->Write( &nTblMaskForSave , sizeof(int));

	// 4)
	nTblCntForSave = ByteSwap32( nTblCntForSave );
	fp->Write( &nTblCntForSave , sizeof(int));

	DWORD dwTempPos = (DWORD)fp->GetPosition();


	// 5)
	for( i = 0 ; i < nTblCnt ; i++ )
	{
		fp->Write( &nEmptyValue , sizeof(int));
	}

	memset( nOffsetTbl , 0x00 , sizeof( nOffsetTbl ) );

	int nCnt = 0;
	int nCurMask = 1;

	for( i = 0 ; i < 32 ; i++ , nCurMask <<= 1 )
	{
		// Mask 채크.
		if( nTblMask & nCurMask )
		{
			switch( (nTblMask & nCurMask ) )
			{
			case MAPID_MAININFO :
				nOffsetTbl[i] = (int)fp->GetPosition();
				SaveMapInfo( fp );
				SaveResInfo( fp );
				break;
			case MAPID_HEIGHT :
				nOffsetTbl[i] = (int)fp->GetPosition();
				SaveHeight( fp );
				break;
			case MAPID_BRIGHT :
				nOffsetTbl[i] = (int)fp->GetPosition();
				SaveBrightMap( fp );
				break;
			case MAPID_COLOR :
				nOffsetTbl[i] = (int)fp->GetPosition();
				SaveColorMap( fp );
				break;
			case MAPID_LIGHT :
				nOffsetTbl[i] = (int)fp->GetPosition();
				SaveLightInfo( fp );
				break;
			/*
			case MAPID_SKINLIST :
				nOffsetTbl[i] = (int)fp->GetPosition();
				SaveSkinPack( fp );
				break;
			*/
			case MAPID_PROP :
				nOffsetTbl[i] = (int)fp->GetPosition();
				SaveProp( fp );
				SavePropExtensionData( fp );
				break;
			case MAPID_ATTRIBUTE :
				nOffsetTbl[i] = (int)fp->GetPosition();
				SaveAttributeMap( fp );
				break;
			case MAPID_AREA :
				nOffsetTbl[i] = (int)fp->GetPosition();
				SaveArea( fp );
				SaveAreaExtensionData( fp );	// 확장 데이터
				break;
			case MAPID_TROOP :
				nOffsetTbl[i] = (int)fp->GetPosition();
				SaveTroop( fp );
				break;
			case MAPID_MAPSET :
				nOffsetTbl[i] = (int)fp->GetPosition();
				SaveMapSet( fp );
				break;
			case MAPID_WATER :
				nOffsetTbl[i] = (int)fp->GetPosition();
				SaveWaterLine( fp );
				break;
			case MAPID_PATH :
				nOffsetTbl[i] = (int)fp->GetPosition();
				SavePath( fp );
				break;
			case MAPID_NAVMESH :
				nOffsetTbl[i] = (int)fp->GetPosition();
				SaveNavList( fp );
				break;
			case MAPID_NAVEVENT :
				nOffsetTbl[i] = (int)fp->GetPosition();
				SaveNavEventList( fp );
				break;
			case MAPID_COLLISION :
				nOffsetTbl[i] = (int)fp->GetPosition();
				SaveColList( fp );
				break;
			case MAPID_SKINLIST :
				nOffsetTbl[i] = (int)fp->GetPosition();
				SaveUsedSkinList( fp );
				break;
			case MAPID_ATTREXMAP :
				nOffsetTbl[i] = (int)fp->GetPosition();
				SaveAttrByteMap( fp );
				break;
			case MAPID_BILLBOARD :
				nOffsetTbl[i] = (int)fp->GetPosition();
				SaveBillboard( fp );
				break;
			case MAPID_BILLBOARDEX :
				nOffsetTbl[i] = (int)fp->GetPosition();
				SaveBillboardEx( fp );
				break;

			case MAPID_PROPZONEDATA:
				nOffsetTbl[i] = (int)fp->GetPosition();
				SavePropZoneData( fp );
				break;
			case MAPID_ATTRIBUTEEX :
				nOffsetTbl[i] = (int)fp->GetPosition();
				SaveAttributeMapEx( fp );
				break;
			}

			nCnt++;
			if( nCnt == nTblCnt )
				break;
		}
	}

	// 테이블 주소 설정.
	fp->Seek( dwTempPos , CFile::begin );

	nCnt = 0;
	for( i = 0 ; i < 32 ; i++ )
	{
		if( nOffsetTbl[i] )
		{
			int nTemp = ByteSwap32( nOffsetTbl[i] );
			fp->Write( &nTemp , sizeof(int));

			nCnt++;
			if( nCnt == nTblCnt )
				break;
		}
	}

	return true;
}

/*

1) 4 Bytes : Layer0 문자열 길이
2) n Bytes : Layer0 문자열

3) 4 Bytes : Layer1 문자열 길이
4) n Bytes : Layer1 문자열

5) 4 Bytes : Layer2 문자열 길이
6) n Bytes : Layer2 문자열

7) 4 Bytes : Layer3 문자열 길이
8) n Bytes : Layer3 문자열

9) 4 Bytes : Layer4 문자열 길이
10) n Bytes : Layer4 문자열

11) 4 Bytes : BlendTexture 문자열 길이
12) n Bytes : BlendTexture 문자열


*/

int		CBSMapCore::SaveMapInfo(CFile *fp)
{
	int nSize;
	int nSizeForSave;
	int nOffset = 0;

	for( int i = 0 ; i < 6 ; i++ )
	{
		nSize = (int)_tcslen( m_szLayer[i] );
		nSizeForSave = ByteSwap32( nSize );

		fp->Write( &nSizeForSave , sizeof(int));

		nOffset += sizeof(int);

		if( nSize != 0 )
		{
			fp->Write( m_szLayer[i] , nSize );
			nOffset += nSize;
		}
	}

	return nOffset;
}

int		CBSMapCore::SaveResInfo(CFile *fp)
{
	int nSizeForSave;
	int nOffset = 0;

	nSizeForSave = ByteSwap32( sizeof(RESOURCE_INFO) );
	fp->Write( &nSizeForSave , sizeof(int));
	nOffset += sizeof(int);
	fp->Write( &m_ResInfo , sizeof(RESOURCE_INFO) );
	nOffset += sizeof(RESOURCE_INFO);

	return nOffset;
}

/*
1) 4 Bytes : xsize					(int)
2) 4 Bytes : ysize					(int)
3) 4 Bytes : Height Buffer size		(int) : xsize+1 * ysize+1 * sizeof(short)

4) n Bytes : -- Offset list

*/
int		CBSMapCore::SaveHeight(CFile *fp)
{
	int nXSize = ByteSwap32( m_nXSize );
	int nYSize = ByteSwap32( m_nYSize );

	int nOffset = (m_nXSize+1) * (m_nYSize+1);

	int nBufferSize = ByteSwap32( (nOffset * sizeof(short)) );

	short *pTempBuf = new short[ nOffset ];

	for( int i = 0 ; i < nOffset ; i++ )
	{
		pTempBuf[i] = ByteSwap16( m_pHeightData[i] );
	}

	fp->Write( &nXSize , sizeof(int));
	fp->Write( &nYSize , sizeof(int));
	fp->Write( &nBufferSize , sizeof(int));

	fp->Write( pTempBuf , sizeof(short) * nOffset );

	delete [] pTempBuf;

	return nOffset;
}

int		CBSMapCore::SaveBrightMap(CFile *fp)
{
	int nXSize = ByteSwap32( m_nXSize );
	int nYSize = ByteSwap32( m_nYSize );

	int nOffset = (m_nXSize+1) * (m_nYSize+1);

	int nBufferSize = ByteSwap32( (nOffset * sizeof(DWORD)) );

	DWORD *pTempBuf = new DWORD[ nOffset ];

	for( int i = 0 ; i < nOffset ; i++ )
	{
		pTempBuf[i] = ByteSwap32( m_pdwMulColor[i] );
	}

	fp->Write( &nXSize , sizeof(int));
	fp->Write( &nYSize , sizeof(int));
	fp->Write( &nBufferSize , sizeof(int));

	fp->Write( pTempBuf , sizeof(DWORD) * nOffset );

	delete [] pTempBuf;

	return nOffset;
}

int		CBSMapCore::SaveColorMap(CFile *fp)
{
	int nXSize = ByteSwap32( m_nXSize );
	int nYSize = ByteSwap32( m_nYSize );

	int nOffset = (m_nXSize+1) * (m_nYSize+1);

	int nBufferSize = ByteSwap32( (nOffset * sizeof(DWORD)) );

	DWORD *pTempBuf = new DWORD[ nOffset ];

	for( int i = 0 ; i < nOffset ; i++ )
	{
		pTempBuf[i] = ByteSwap32( m_pdwAddColor[i] );
	}

	fp->Write( &nXSize , sizeof(int));
	fp->Write( &nYSize , sizeof(int));
	fp->Write( &nBufferSize , sizeof(int));

	fp->Write( pTempBuf , sizeof(DWORD) * nOffset );

	delete [] pTempBuf;

	return nOffset;
}

int		CBSMapCore::SaveProp(CFile *fp)
{
	int nOffset = 0;
	CBSPropPtr pProp;

	int nSizeForSave;
	int nSize = m_pPropManager->GetPropCount();
	nSizeForSave = ByteSwap32(nSize);

	int nIdForSave;
	int nIndexForSave;
	int nSkinForSave;

	DWORD dwEmpty = 0;
	DWORD dwPosTemp[3];

	short nXForSave,nYForSave;

	fp->Write( &nSizeForSave , sizeof(int));


	for( int i = 0 ; i < nSize ; i++ )
	{
		pProp = (*m_pPropManager)[i];

		nIdForSave = ByteSwap32( pProp->m_nId );
		nIndexForSave = ByteSwap32( pProp->m_nObjIndex );
		nSkinForSave = ByteSwap32( pProp->m_nSkinIndex );
		dwEmpty = ByteSwap32( pProp->m_dwScaleForUndo );

		MemSwap32( dwPosTemp, pProp->m_Cross.m_PosVector, sizeof(dwPosTemp) );

		nXForSave = ByteSwap16( (short)pProp->m_nXIndex );
		nYForSave = ByteSwap16( (short)pProp->m_nYIndex );

		fp->Write( &nIdForSave , sizeof(int));
		fp->Write( &nIndexForSave , sizeof(int));
		fp->Write( &nSkinForSave , sizeof(int));

		fp->Write( dwPosTemp, sizeof(dwPosTemp) );

		fp->Write( &dwEmpty , sizeof(DWORD));

		fp->Write( &nXForSave , sizeof(WORD));
		fp->Write( &nYForSave , sizeof(WORD));

		nOffset += sizeof( PROP_INFO );
	}

	return nOffset;
}

int		CBSMapCore::SaveBillboard(CFile *fp)
{
	int nOffset = 0;
	CBSPropPtr pProp;

	int nSizeForSave;
	int nSize = m_pPropManager->GetBillboardPropCount();
	nSizeForSave = ByteSwap32(nSize);

	int nIdForSave;
	int nIndexForSave;
	int nSkinForSave;

	DWORD dwEmpty = 0;
	DWORD dwPosTemp[3];

	short nXForSave,nYForSave;

	fp->Write( &nSizeForSave , sizeof(int));

	for( int i = 0 ; i < nSize ; i++ )
	{
		pProp = m_pPropManager->GetBillboardProp( i );	//	pProp = (*m_pPropManager)[i];

		// 저장시 채크
		if( pProp->PropInWorld() )
		{
			nIdForSave = ByteSwap32( pProp->m_nId );
			nIndexForSave = ByteSwap32( pProp->m_nObjIndex );
			nSkinForSave = ByteSwap32( ( pProp->m_nSkinIndex ) - m_nBillboardIndexGap );
			dwEmpty = ByteSwap32( pProp->m_dwScaleForUndo );

			MemSwap32( dwPosTemp, pProp->m_Cross.m_PosVector, sizeof(dwPosTemp) );

			nXForSave = ByteSwap16( (short)pProp->m_nXIndex );
			nYForSave = ByteSwap16( (short)pProp->m_nYIndex );

			fp->Write( &nIdForSave , sizeof(int));
			fp->Write( &nIndexForSave , sizeof(int));
			fp->Write( &nSkinForSave , sizeof(int));

			fp->Write( dwPosTemp, sizeof(dwPosTemp) );

			fp->Write( &dwEmpty , sizeof(DWORD));

			fp->Write( &nXForSave , sizeof(WORD));
			fp->Write( &nYForSave , sizeof(WORD));

			nOffset += sizeof( PROP_INFO );
		}
	}

	return nOffset;
}

int		CBSMapCore::SaveBillboardEx(CFile *fp)
{
	return 0;
}

int		CBSMapCore::SavePropZoneData(CFile *fp )
{
	MapFilePropZoneManager pZoneManager;
	pZoneManager.Initialize( m_nXSize, m_nYSize );

	int nSize = m_pPropManager->GetPropCount();
	for( int i = 0 ; i < nSize ; i++ )
	{
		CBSPropPtr pProp = (*m_pPropManager)[i];
		pZoneManager.AddProp( pProp->GetObjIndex() );
	}
	pZoneManager.CaculateBoundingBox();


	int nValue = ByteSwap32( pZoneManager.m_nZoneSize );
	fp->Write( &nValue, sizeof(int) );

	nValue = ByteSwap32( pZoneManager.m_nXSize );
	fp->Write( &nValue, sizeof(int) );

	nValue = ByteSwap32( pZoneManager.m_nYSize );
	fp->Write( &nValue, sizeof(int) );

	for( int i=0; i<pZoneManager.m_nXSize * pZoneManager.m_nYSize; i++ )
	{
		DWORD dwPosTemp[3];
		fp->Write( &(pZoneManager.m_pZoneData[i].bProp), sizeof(bool) );

		MemSwap32( dwPosTemp, &pZoneManager.m_pZoneData[i].Box.C, sizeof(dwPosTemp) );
		fp->Write( &dwPosTemp, sizeof(dwPosTemp) );
		for( int j=0; j<3; j++ )
		{
			MemSwap32( dwPosTemp, &pZoneManager.m_pZoneData[i].Box.A[j], sizeof(dwPosTemp) );
			fp->Write( &dwPosTemp, sizeof(dwPosTemp) );
		}
		MemSwap32( dwPosTemp, pZoneManager.m_pZoneData[i].Box.E, sizeof(dwPosTemp) );
		fp->Write( &dwPosTemp, sizeof(dwPosTemp) );
		for( int j=0; j<8; j++ )
		{
			MemSwap32( dwPosTemp, &pZoneManager.m_pZoneData[i].Box.V[j], sizeof(dwPosTemp) );
			fp->Write( &dwPosTemp, sizeof(dwPosTemp) );
		}
	}
	return true;
}


// Prop_Info 에서 확장된 데이터들
int		CBSMapCore::SavePropExtensionData(CFile *fp)
{
	int nOffset = 0;
	CBSPropPtr pProp;
	int nExtDataCount = 0;		// 확장 데이터를 가지고 있는 prop들의 수
	int nSize = m_pPropManager->GetPropCount();

	int nSizeForSave;
	int nIndexForSave;

	DWORD dwHeadPos = (DWORD)fp->GetPosition();
	fp->Write( &nOffset , sizeof(int));				// fp->Write( &nSizeForSave , sizeof(int));
	nOffset += sizeof(int);

	// 확장 데이터 사이즈 저장하기

	int nExtSize = CAPTION_MAX + (sizeof(int)*PROP_PARAM_EXT_MAX) + STR_MAX;
	nSizeForSave = ByteSwap32( nExtSize );
	fp->Write( &nSizeForSave, sizeof(int));
	nOffset += sizeof(int);
	
	for( int i = 0 ; i < nSize ; i++ )
	{
		pProp = (*m_pPropManager)[i];

		if( pProp->HasPropName() )
		{
			++nExtDataCount;

			// 순서
			// 1) 인덱스   : 4
			// 2) PropName : 32

			nIndexForSave = ByteSwap32( i );
			fp->Write( &nIndexForSave, sizeof(int));

			fp->Write( pProp->m_szPropName, CAPTION_MAX );
			fp->Write( pProp->m_szExtBuffer, STR_MAX );

			for( int k = 0 ; k < PROP_PARAM_EXT_MAX ; ++k )
			{
                nIndexForSave = ByteSwap32( pProp->m_dwParamExt[k] );
                fp->Write( &nIndexForSave, sizeof(DWORD));
			}

			

			nOffset += ( sizeof(int) + CAPTION_MAX + (sizeof(DWORD) * PROP_PARAM_EXT_MAX) );
		}
	}

	// 2중 안전장치인데.. 쓸 필요가 있을지는..
	nIndexForSave = -1;
	fp->Write( &nIndexForSave, sizeof(int));
	nOffset += sizeof(int);

	// 현재 위치 저장하고, 루프 돌기 이전으로 돌아감
    DWORD dwTailPos = (DWORD)fp->GetPosition();
	fp->Seek( dwHeadPos, CFile::begin );

	// 확장 데이터가 있는 데이터들의 개수만 다시 저장.
	nIndexForSave = ByteSwap32( nExtDataCount );
	fp->Write( &nIndexForSave , sizeof(int));

	// 다시 원위치
	fp->Seek( dwTailPos, CFile::begin );

	return nOffset;
}

int		CBSMapCore::SaveUsedSkinList(CFile *fp)
{
	int nOffset = 0;
	int nCnt,nCntForSave;
	int* pBuf;

	nCnt = m_pPropManager->GetUsedSkinList( &pBuf );

	if( nCnt )
	{
		nCntForSave = ByteSwap32( nCnt );
		MemSwap32( pBuf, pBuf, nCnt*4 );

		fp->Write( &nCntForSave , sizeof(int) );
		fp->Write( pBuf , sizeof(int)*nCnt );

		nOffset = sizeof(int) * (nCnt+1);

		delete [] pBuf;
	}
	else
	{
		fp->Write( &nCnt, sizeof(int) );	// 0
		nOffset = sizeof(int);
	}
	/*
	int nStrSize;
	int nStrSizeForSave;
	int nSize = (int)m_SkinPool.size();
	int nSizeForSave = ByteSwap32( nSize );

	fp->Write( &nSizeForSave , sizeof(int));
	nOffset += sizeof(int);

	for( int i = 0 ; i < nSize ; i++ )
	{
		// Index 저장.
		nStrSize = m_SkinPool[i].m_nIndex;
		nStrSizeForSave = ByteSwap32( nStrSize );
		fp->Write( &nStrSizeForSave , sizeof(int));
		nOffset += sizeof(int);

		// skin name size 저장
		nStrSize = (int)_tcslen( m_SkinPool[i].m_szSkinname );
		nStrSizeForSave = ByteSwap32( nStrSize );

		fp->Write( &nStrSizeForSave , sizeof(int));
		nOffset += sizeof(int);

		if( nStrSize != 0 )
		{
			// skin name 저장.
			fp->Write( m_SkinPool[i].m_szSkinname , nStrSize );
			nOffset += nStrSize;
		}
	}
	*/

	return nOffset;
}

int		CBSMapCore::SaveAttrByteMap(CFile *fp)
{
	int nChunk = ByteSwap32( 2 );
	int nx = ByteSwap32( m_nABXSize );
	int ny = ByteSwap32( m_nABYSize );

	fp->Write( &nChunk, sizeof(int) );
	fp->Write( &nx, sizeof(int) );
	fp->Write( &ny, sizeof(int) );
	fp->Write( m_pAttrByteMap, m_nABXSize * m_nABYSize );

	// 2번째 Chunk.
	for( int i = 0 ; i < 5 ; ++i )
	{
		int nLayerAttr = ByteSwap32( m_nLayerAttr[i] );
        fp->Write( &nLayerAttr, sizeof(int) );
	}

#ifdef _DEBUG

	CFile file;

	file.Open( _T("AttrMap.bin"), CFile::modeCreate | CFile::modeWrite );

	int x = 0;
	for( int y = 0 ; y < m_nABYSize ; ++y )
	{
		file.Write( m_pAttrByteMap + x , m_nABXSize );
		file.Write( "\r\n", 2 );
		x += m_nABXSize;
	}

	file.Close();

#endif

	return (m_nABXSize * m_nABYSize + 12 + ( sizeof(int) * 5 ) );


}


int		CBSMapCore::SaveLightInfo(CFile *fp)
{
	int nOffset = 0;
	int nLightCnt = 3; // Diffuse , Specular , Ambient
	D3DCOLORVALUE	ColorValue;
	DWORD dwR,dwG,dwB,dwA,dwTemp = 0;
	DWORD dwXYZ[3];

	// X Vector
	/*
	memcpy( dwXYZ , &m_pLightCross->m_XVector , sizeof(float) * 3);
	dwR = ByteSwap32( dwXYZ[0] );		
	dwG = ByteSwap32( dwXYZ[1] );
	dwB = ByteSwap32( dwXYZ[2] );
	fp->Write( &dwR , sizeof(DWORD) );
	fp->Write( &dwG , sizeof(DWORD) );
	fp->Write( &dwB , sizeof(DWORD) );
	*/

	MemSwap32( dwXYZ,&m_pLightCross->m_XVector, sizeof(float) * 3 );
	fp->Write( dwXYZ , sizeof(DWORD)*3 );

	MemSwap32( dwXYZ,&m_pLightCross->m_YVector, sizeof(float) * 3 );
	fp->Write( dwXYZ , sizeof(DWORD)*3 );

	MemSwap32( dwXYZ,&m_pLightCross->m_ZVector, sizeof(float) * 3 );
	fp->Write( dwXYZ , sizeof(DWORD)*3 );

	MemSwap32( dwXYZ,&m_pLightCross->m_PosVector, sizeof(float) * 3 );
	fp->Write( dwXYZ , sizeof(DWORD)*3 );

	nOffset += (sizeof(D3DXVECTOR3) * 4);
	

	int nTemp = ByteSwap32( nLightCnt );

	fp->Write( &nTemp , sizeof(int));
	nOffset += 4;

	for( int i = 0 ; i < nLightCnt ; i++ )
	{
        RGBToD3DColor( ColorValue , m_dwLight[i]  );

		dwR = DWORD( ColorValue.r );
		dwG = DWORD( ColorValue.g );
		dwB = DWORD( ColorValue.b );
		dwA = DWORD( ColorValue.a );

		dwR = ByteSwap32( dwR );
		dwG = ByteSwap32( dwG );
		dwB = ByteSwap32( dwB );
		dwA = ByteSwap32( dwA );

		fp->Write( &dwA , sizeof(DWORD) );
		fp->Write( &dwR , sizeof(DWORD) );
		fp->Write( &dwG , sizeof(DWORD) );
		fp->Write( &dwB , sizeof(DWORD) );

		// 임시 버퍼, 
		fp->Write( &dwTemp , sizeof(DWORD) );

		nOffset += 20;
	}

	return nOffset;
}

int		CBSMapCore::SaveArea(CFile *fp)
{
	AREA_INFO areaInfo;
	CBSAreaPack *pArea;

	int nOffset = 0;
	int nCntForSave;
	DWORD dwBuffer[6];	// Uid-Type- Left Top Right Bottom
	DWORD dwTemp[2] = {0,0};

	int nCnt = m_pAreaManager->GetAreaCount();
	nCntForSave = ByteSwap32( nCnt );

	fp->Write( &nCntForSave , sizeof(int));
	nOffset += 4;

	for( int i = 0 ; i < nCnt ; ++i )
	{
		pArea = m_pAreaManager->GetArea( i );

		pArea->GetAreaInfo( &areaInfo );

		memcpy( dwBuffer , &areaInfo , sizeof(AREA_INFO));

		for( int k = 0 ; k < 6 ; ++k )
            dwBuffer[k] = ByteSwap32( dwBuffer[k] );

		fp->Write( dwBuffer , sizeof( AREA_INFO ) );
		fp->Write( dwTemp , 8 );	// 8 bytes 여유분.
		nOffset += 32;
	}

	return nOffset;
}

// AREA_ATTR 정보를 저장한다.
int		CBSMapCore::SaveAreaExtensionData(CFile *fp)
{
	CBSAreaPack *pArea;
	AREA_ATTR	areaAttr;
	int nOffset = 0;
	int nNumBufSize = 0;
	int nTxtBufSize = CAPTION_MAX;
	int nExtBufSize = sizeof( AREA_ATTR );
	int nCntForSave;

	int nCnt = m_pAreaManager->GetAreaCount();
	nCntForSave = ByteSwap32( nCnt );

	// 0) Area 개수
	fp->Write( &nCntForSave , sizeof(int));		nOffset += 4;

	// 1) 전체 Buffer Size
	nCntForSave = ByteSwap32( nExtBufSize );
	fp->Write( &nCntForSave, sizeof(int));		nOffset += 4;

	// 문자열 데이터와 정수 데이터를 따로 저장한다.
	// 2) 수치 데이터 Buffer Size
	nCntForSave = nNumBufSize;
	fp->Write( &nCntForSave, sizeof(int));		nOffset += 4;
	
	// 3) 문자열 데이터 Buffer Size
	nCntForSave = ByteSwap32( nTxtBufSize );
	fp->Write( &nCntForSave, sizeof(int));		nOffset += 4;

	// 4) Area 개수만큼  수치 데이터 버퍼, 문자열 데이터 버퍼 저장.
	for( int i = 0 ; i < nCnt ; ++i )
	{
		pArea = m_pAreaManager->GetArea( i );

		pArea->GetSwappedAreaAttr( &areaAttr );

		if( nNumBufSize )
		{
            fp->Write( &areaAttr, nNumBufSize );
			nOffset += nNumBufSize;
		}

		if( nTxtBufSize )
		{
			fp->Write( (&areaAttr) + nNumBufSize, nTxtBufSize );
			nOffset += nTxtBufSize;
		}
	}

	return nOffset;
}


/*

	+ Area_Info Block
		0) Block Size

	+ Troop_Info Block
		0) Block Size

 
 */
int		CBSMapCore::SaveTroop(CFile *fp)
{
	int nOffset = 0;
	int nCntForSave;
	int	nCntForChunk = 3;	// 내부 블럭이 2개다.
	int nStrBSForSave;

	TROOP_INFO troopInfo;

	int nStrBlockSize = sizeof(troopInfo.m_troopAttr.StrBlock);
	// Strblock를 제외한 크기.
	int nTroopAttrSize = sizeof(TROOP_ATTR) - nStrBlockSize;

	int nSizeForAInfo = ByteSwap32( sizeof(AREA_INFO));
	int nSizeForTAttr = ByteSwap32( nTroopAttrSize );

	CBSTroopPack*	pTroop;
		

	int nCnt = m_pTroopManager->GetAreaCount();
	nCntForSave = ByteSwap32( nCnt );
	nCntForChunk = ByteSwap32( nCntForChunk );
	nStrBSForSave = ByteSwap32( nStrBlockSize );
	

	fp->Write( &nCntForChunk, sizeof(int));
	fp->Write( &nCntForSave,  sizeof(int));
	nOffset += 8;

	for( int i = 0 ; i < nCnt ; ++i )
	{
		pTroop = m_pTroopManager->GetArea( i );
		pTroop->GetSwappedTroopInfo( &troopInfo );

		// Chunk 1.
		fp->Write( &nSizeForAInfo , sizeof(int) );	
		fp->Write( &troopInfo.m_areaInfo , sizeof(AREA_INFO));
		nOffset += (sizeof(AREA_INFO) + 4);

		// Chunk 2.
		fp->Write( &nSizeForTAttr , sizeof(int) );	
		fp->Write( &troopInfo.m_troopAttr , nTroopAttrSize );
		nOffset += (nTroopAttrSize + 4);

		// Chunk 3.
		fp->Write( &nStrBSForSave, sizeof(int) );
		fp->Write( &troopInfo.m_troopAttr.StrBlock, nStrBlockSize );
		nOffset += (nStrBlockSize + 4);
	}

	return nOffset;
}

int		CBSMapCore::SaveMapSet(CFile *fp)
{
	// MemSwap32( pInfo, &m_troopInfo, sizeof(TROOP_INFO));

	//
	int nOffset = 0;
	int nCntForChunk = 10;
	int nTempForSave;
	BYTE byBuffer[ sizeof(SET_INFO) ];

	nTempForSave = ByteSwap32( nCntForChunk );

	// 1) Chunk 갯수
	fp->Write( &nTempForSave, sizeof(int));
	nOffset += 4;

	// 2) Chunk #1 크기
	nTempForSave = ByteSwap32( sizeof(SET_INFO));
	fp->Write( &nTempForSave, sizeof(int));
	nOffset += 4;

	// 3) Chunk #1 실 이미지
	MemSwap32( byBuffer, m_pDefaultSetInfo, sizeof(SET_INFO));
	fp->Write( byBuffer, sizeof(SET_INFO));
	nOffset += sizeof(SET_INFO);

	// 4) Chunk #2 크기
	int nSkyboxInfoSize = sizeof(int) + m_SkyboxInfo.nLen;
	nTempForSave = ByteSwap32( nSkyboxInfoSize );
	fp->Write( &nTempForSave , sizeof(int));

	// 5) Chunk #2 실 이미지
	fp->Write( m_SkyboxInfo.m_pszFileName, m_SkyboxInfo.nLen );

	// 6) Chunk #3 크기
	int nCubeMapInfoSize = sizeof(int) + m_CubeMapInfo.nLen;
	nTempForSave = ByteSwap32( nCubeMapInfoSize );
	fp->Write( &nTempForSave , sizeof(int));

	// 7) Chunk #3 실 이미지
	fp->Write( m_CubeMapInfo.m_pszFileName, m_CubeMapInfo.nLen );

	// -- Chunk #4 ---------------------------------------------------------------
	// 8) Chunk #4 크기
	int nAIInfoSize = sizeof(int) + m_AIFileInfo.nLen;
	nTempForSave = ByteSwap32( nAIInfoSize );
	fp->Write( &nTempForSave , sizeof(int));

	// 9) Chunk #4 실 이미지
	fp->Write( m_AIFileInfo.m_pszFileName, m_AIFileInfo.nLen );

	// -- Chunk #5 ---------------------------------------------------------------
	// 8) Chunk #5 크기
	int nTextInfoSize = sizeof(int) + m_TextFileInfo.nLen;
	nTempForSave = ByteSwap32( nTextInfoSize );
	fp->Write( &nTempForSave , sizeof(int));

	// 9) Chunk #5 실 이미지
	fp->Write( m_TextFileInfo.m_pszFileName, m_TextFileInfo.nLen );

	// -- Chunk #6 ---------------------------------------------------------------
	// 10) Chunk #6 크기
	int nCameraFileTextInfoSize = sizeof(int) + m_CameraFileInfo.nLen;
	nTempForSave = ByteSwap32( nCameraFileTextInfoSize );
	fp->Write( &nTempForSave , sizeof(int));

    // 11) Chunk #6 실 이미지
	fp->Write( m_CameraFileInfo.m_pszFileName, m_CameraFileInfo.nLen );

	// -- Chunk #7 ---------------------------------------------------------------
	// 10) Chunk #7 크기
	int nLightMapFileTextInfoSize = sizeof(int) + m_LightMapFileInfo.nLen;
	nTempForSave = ByteSwap32( nLightMapFileTextInfoSize );
	fp->Write( &nTempForSave , sizeof(int));

	// 11) Chunk #7 실 이미지
	fp->Write( m_LightMapFileInfo.m_pszFileName, m_LightMapFileInfo.nLen );

	// -- Chunk #8 --------------------------------------------------------------- 
	// 12) Chunk #8 크기.
	int nBBoardNameInfoSize = sizeof(int) + m_BillboardDDSInfo.nLen;
	nTempForSave = ByteSwap32( nBBoardNameInfoSize );
	fp->Write( &nTempForSave , sizeof(int));

	// 13) Chunk #8 실 이미지
	fp->Write( m_BillboardDDSInfo.m_pszFileName, m_BillboardDDSInfo.nLen );

	// -- Chunk #9 --------------------------------------------------------------- 
	nTempForSave = ByteSwap32( 1 );	// #9의 데이터 개수
	fp->Write( &nTempForSave , sizeof(int));

	// Chunk #9 실 데이터.
	nTempForSave = ByteSwap32( m_nWaterCount );
	fp->Write( &nTempForSave , sizeof(int));
	
	return nOffset;
}



int		CBSMapCore::SaveAttributeMap(CFile *fp)
{
	int nVersion = 1;	// 80 이하여야 함. 맵 최소 사이즈가 40(Attr:80) 이상이기 때문.
	int nOffset = 0;
	int xsize = m_pAttrManager->GetXSize();
	int ysize = m_pAttrManager->GetYSize();
	int nLength = xsize * ysize;
	int nTemp;

	// 버전 추가.
	nTemp = ByteSwap32( nVersion );
	fp->Write( &nTemp, sizeof(int));

	// x, y size 저장.
	nTemp = ByteSwap32( xsize );
	fp->Write( &nTemp, sizeof(int));
	nTemp = ByteSwap32( ysize );
	fp->Write( &nTemp, sizeof(int));
	nOffset += 12;

	CreateTempBuffer( xsize, ysize );
	ChangeAttrDataIntoGameAttr();

	// fp->Write( m_pAttrManager->GetAttrBuffer(), nLength );
	fp->Write( m_pTempAttrBuffer , nLength );
	nOffset += nLength;

	// 속성값 바이너리 저장 테스트
	/*
	CFile file;

	file.Open( _T("AttrMap.bin"), CFile::modeCreate | CFile::modeWrite );

	file.Write( &xsize, sizeof(int));
	file.Write( &ysize, sizeof(int));
	file.Write( m_pAttrManager->GetAttrBuffer(), nLength );

	file.Close();
	*/

	return nOffset;
}

int		CBSMapCore::SaveAttributeMapEx(CFile *fp)
{
	int nVersion = 1;	// 80 이하여야 함. 맵 최소 사이즈가 40(Attr:80) 이상이기 때문.
	int nOffset = 0;
	int xsize = m_pAttrManager->GetXSize();
	int ysize = m_pAttrManager->GetYSize();
	int nLength = xsize * ysize;
	int nTemp;

	// 버전 추가.
	nTemp = ByteSwap32( nVersion );
	fp->Write( &nTemp, sizeof(int));

	// x, y size 저장.
	nTemp = ByteSwap32( xsize );
	fp->Write( &nTemp, sizeof(int));
	nTemp = ByteSwap32( ysize );
	fp->Write( &nTemp, sizeof(int));
	nOffset += 12;

	CreateAttrExBuffer( xsize, ysize );
	CreateDiagonalAttrMap( m_pTempAttrBuffer, m_pAttrMapEx, xsize, ysize );

	fp->Write( m_pAttrMapEx , nLength );

	DeleteAttrExBuffer();
	DeleteTempBuffer();

	nOffset += nLength;
	return nOffset;
}

int		CBSMapCore::SaveWaterLine(CFile *fp)
{
	return m_pLineManager->SaveForTool( fp );
}

int		CBSMapCore::SaveWaterLineEx(CFile *fp)
{
	// return m_pLineManager->
	return 0;
}

int		CBSMapCore::SavePath(CFile *fp)
{
	return m_pPathManager->SaveForTool( fp );
}

// 1) Vertex Count
// 1-1)	Vertex (float * 3 ) * Cnt
// 2) Index Count
// 2-1) Index ( int ) * Idx Cnt
int		CBSMapCore::SaveNavList(CFile *fp)
{
	int nOffset = 0;
	BYTE *pTempBuf;
	int nBufSize;

	int nSize;
	int nSizeForSave;

	nSize = (int)m_NavList.size();
	nSizeForSave = ByteSwap32( nSize );
	
	// 1) 저장
	fp->Write( &nSizeForSave, sizeof(int));
	nOffset += sizeof(int);

	if( nSize > 0 )
	{
		nBufSize = nSize * sizeof(D3DXVECTOR3);
		pTempBuf = new BYTE[ nBufSize ];
		MemSwap32( pTempBuf, &m_NavList[0], nBufSize );

		// 1-1) 저장.
		fp->Write( pTempBuf, nBufSize );
		nOffset += nBufSize;

		delete [] pTempBuf;
	}

	nSize = (int)m_NavIndexList.size();
	nSizeForSave = ByteSwap32( nSize );

	// 2) 저장
	fp->Write( &nSizeForSave, sizeof(int));
	nOffset += sizeof(int);

	if( nSize > 0 )
	{
		nBufSize = nSize * sizeof(int);
		pTempBuf = new BYTE[ nBufSize ];
		MemSwap32( pTempBuf, &m_NavIndexList[0], nBufSize );

		// 2-1) 저장.
		fp->Write( pTempBuf, nBufSize );
		nOffset += nBufSize;

		delete [] pTempBuf;
	}

	return nOffset;
}

int		CBSMapCore::SaveNavEventList(CFile *fp)
{
	// m_nNavEventCount
	// 
	int nOffset = 0;
	int nSize;
	int nCntForSave;
	int nBufSize;
	BYTE *pTempBuf;

	// 1) 버퍼 개수 저장.
	nCntForSave = ByteSwap32( m_nNavEventCount );
	fp->Write( &nCntForSave, sizeof(int));
	nOffset += sizeof(int);

	for( int i = 0 ; i < m_nNavEventCount ; ++i )
	{
		// 2-1) x번째 이벤트 버퍼 개수 저장.
		nSize = (int)m_pNavEventIndexList[i].size();

		nCntForSave = ByteSwap32( nSize );
		fp->Write( &nCntForSave, sizeof(int));
		nOffset += sizeof(int);

		if( nSize > 0 )
		{
			nBufSize = nSize * sizeof(int);
			pTempBuf = new BYTE[ nBufSize ];
			MemSwap32( pTempBuf, &m_pNavEventIndexList[i][0], nBufSize );

			// 2-2) 버퍼 저장
			fp->Write( pTempBuf, nBufSize );
			nOffset += nBufSize;

			delete [] pTempBuf;
		}
	}

	return nOffset;
}

int		CBSMapCore::SaveColList(CFile *fp)
{
	int nOffset = 0;
	BYTE *pTempBuf;
	int nBufSize;

	int nSize;
	int nSizeForSave;

	// Main List 저장
	{
		nSize = (int)m_ColList.size();
		nSizeForSave = ByteSwap32( nSize );

		// 1) 저장
		fp->Write( &nSizeForSave, sizeof(int));
		nOffset += sizeof(int);

		if( nSize > 0 )
		{
			nBufSize = nSize * sizeof(D3DXVECTOR3);
			pTempBuf = new BYTE[ nBufSize ];
			MemSwap32( pTempBuf, &m_ColList[0], nBufSize );

			// 1-1) 저장.
			fp->Write( pTempBuf, nBufSize );
			nOffset += nBufSize;

			delete [] pTempBuf;
		}

		nSize = (int)m_ColIndexList.size();
		nSizeForSave = ByteSwap32( nSize );

		// 2) 저장
		fp->Write( &nSizeForSave, sizeof(int));
		nOffset += sizeof(int);

		if( nSize > 0 )
		{
			nBufSize = nSize * sizeof(int);
			pTempBuf = new BYTE[ nBufSize ];
			MemSwap32( pTempBuf, &m_ColIndexList[0], nBufSize );

			// 2-1) 저장.
			fp->Write( pTempBuf, nBufSize );
			nOffset += nBufSize;

			delete [] pTempBuf;
		}
	}

	// 그외 첫번째 리스트
	{
		int nCntForSave;
		// 1) 버퍼 개수 저장.
		nCntForSave = ByteSwap32( m_nColEventCount );
		fp->Write( &nCntForSave, sizeof(int));
		nOffset += sizeof(int);

		for( int i = 0 ; i < m_nColEventCount ; ++i )
		{
			// 2-1) x번째 이벤트 버퍼 개수 저장.
			nSize = (int)m_pColEventIndexList[i].size();

			nCntForSave = ByteSwap32( nSize );
			fp->Write( &nCntForSave, sizeof(int));
			nOffset += sizeof(int);

			if( nSize > 0 )
			{
				nBufSize = nSize * sizeof(int);
				pTempBuf = new BYTE[ nBufSize ];
				MemSwap32( pTempBuf, &m_pColEventIndexList[i][0], nBufSize );

				// 2-2) 버퍼 저장
				fp->Write( pTempBuf, nBufSize );
				nOffset += nBufSize;

				delete [] pTempBuf;
			}
		}
	}
	return nOffset;
}


#include "NavigationMesh.h"

struct NAVEVENTINFO
{
	int nStartIndex;
	int nNumNavIndex;
};

int CBSMapCore::SaveNavFile( char* pMapFileName )
{
	int nNumNavEventInfo;
	NAVEVENTINFO* pNavEventInfo = NULL;

	NavigationMesh* pNavigationMesh;
	pNavigationMesh = new NavigationMesh;

	bool bReadyNavFile = false;

	char cNavFile[MAX_PATH];
//	strcpy( cNavFile, GetMapFileName() );
	strcpy( cNavFile, pMapFileName );
	char* pTempStr = strstr( cNavFile, "." );
	while(1)
	{
		char* pNextStr = strstr( pTempStr+1, "." );
		if( pNextStr == NULL )
			break;

		pTempStr = pNextStr;
	}

	pTempStr[1] = NULL;
	strcat( pTempStr, "nav" );

	BMemoryStream ReadStream;
	VOID *pData = NULL;

	int nTotalCnt = GetNavIndexListCount();
	int nEventCnt = GetNavEventCount();
	for( int i=0; i<nEventCnt; i++ )
		nTotalCnt += GetNavEventIndexBufferCount( i );

	pNavigationMesh->InitCell( nTotalCnt/3 );

	int nCnt = GetNavIndexListCount();
	int nStartCnt = nCnt/3;
	for( int i=0; i<nCnt; i+=3 )
	{
		int nIndex = GetNavVectorIndex(i);
		D3DXVECTOR3 Pos = GetNavVector(nIndex);
		D3DXVECTOR3 Pos1( Pos.x, Pos.y, Pos.z );
		nIndex = GetNavVectorIndex(i+1);
		Pos = GetNavVector(nIndex);
		D3DXVECTOR3 Pos2( Pos.x, Pos.y, Pos.z );
		nIndex = GetNavVectorIndex(i+2);
		Pos = GetNavVector(nIndex);
		D3DXVECTOR3 Pos3( Pos.x, Pos.y, Pos.z );
		pNavigationMesh->AddCell( Pos1, Pos2, Pos3, i/3 );
	}

	int nStartIndex = -1;
	nNumNavEventInfo = nEventCnt;
	if( nNumNavEventInfo > 0 )
		pNavEventInfo = new NAVEVENTINFO[nNumNavEventInfo];

	for( int i=0; i<nEventCnt; i++ )
	{
		nStartIndex = -1;
		int nCnt = GetNavEventIndexBufferCount( i );
		if( nCnt == 0 )
			continue;

		int* pIndexBuf = GetNavEventIndexBuffer( i );
		for( int j=0; j<nCnt; j+=3 )
		{
			D3DXVECTOR3 Pos = GetNavVector( pIndexBuf[j] );
			D3DXVECTOR3 Pos1( Pos.x, Pos.y, Pos.z );
			Pos = GetNavVector( pIndexBuf[j+1] );
			D3DXVECTOR3 Pos2( Pos.x, Pos.y, Pos .z );
			Pos = GetNavVector( pIndexBuf[j+2] );
			D3DXVECTOR3 Pos3( Pos.x, Pos.y, Pos.z );
			int nIndex = pNavigationMesh->AddCell( Pos1, Pos2, Pos3, nStartCnt + j/3 );
			NavigationCell* pCell = pNavigationMesh->Cell( nIndex );
			pCell->m_bEvent = true;

			if( nStartIndex == -1 )
				nStartIndex = nIndex;
		}
		nStartCnt += nCnt/3;
		pNavEventInfo[i].nStartIndex = nStartIndex;
		pNavEventInfo[i].nNumNavIndex = nCnt / 3;
	}
	//		Collision Mesh 생성 없음
/*
	m_pMapInfo->ClearNav();
	m_pMapInfo->ClearCol();
	m_pMapInfo->ClearNavEventIndexBuffer();
*/
	pNavigationMesh->LinkCells();

	BFileStream stream( cNavFile, BFileStream::create );
	if( !stream.Valid() )
		BsAssert(0);

	SYSTEMTIME MapTime;
	memset( &MapTime, 0, sizeof(SYSTEMTIME) );
	stream.Write( &MapTime , sizeof(SYSTEMTIME), 2 );		// dummy
	pNavigationMesh->Save( &stream );

	stream.Write( &nNumNavEventInfo, sizeof(int), 4 );
	for( int i=0; i<nNumNavEventInfo; i++ )
		stream.Write( &(pNavEventInfo[i]), sizeof(NAVEVENTINFO), 4 );

	if( pNavigationMesh )
		delete pNavigationMesh;
	if( pNavEventInfo )
		delete[] pNavEventInfo;

	return 1;
}


bool	CBSMapCore::LoadTerrain(BStream *fp)
{
	int i;
	int nOffsetTbl[32] = { 0,};
	char szType[4]; // = "BST";
	int nVersion; // = FC_MAP_VER;

	memset( nOffsetTbl , 0x00 , sizeof( nOffsetTbl ) );

	// 1) 
	fp->Read( szType , 4 );

	if( strcmp( szType , "BST" ) != 0 )
		return false;	// Terrain 파일이 아니구먼..

	// 2)
	fp->Read( &nVersion , sizeof(int) , ENDIAN_FOUR_BYTE );

#ifdef WIN32
	//	TRACE("맵파일 버전 : %d \n", nVersion );
#endif

	// Offset Table Count
	int nTblMask=0;
	int nTblMaskForLoad = nTblMask;

	int nTblCnt = 1;
	int nTblCntForLoad = nTblCnt;
	int nEmptyValue = 0;

	// 3)
	fp->Read( &nTblMask , sizeof(int) , ENDIAN_FOUR_BYTE );

	// 4)
	fp->Read( &nTblCntForLoad , sizeof(int) , ENDIAN_FOUR_BYTE );

	DWORD dwTempPos = (DWORD)fp->Tell();

	nTblCnt = CalcBitCnt( nTblMask );

	if( nTblCnt != nTblCntForLoad )
	{
#ifdef WIN32
		//		TRACE("뭔가 이상이 있는겨.. TblCnt [%d] , TblCntForLoad [%d]\n", nTblCnt , nTblCntForLoad );
#endif
		return false;
	}

	int nTempPos;
	int nCnt = 0;
	int nCurMask = 1;

	// 오프셋들만 따로 저장.
	for( i = 0 ; i < 32 ; i++ , nCurMask <<= 1 )
	{
		if( nTblMask & nCurMask )
		{
			fp->Read( &nTempPos , sizeof(int) , ENDIAN_FOUR_BYTE );
			nOffsetTbl[i] = nTempPos;
			nCnt++;

			if( nCnt == nTblCnt )
				break;
		}
	}

	nCnt = 0;
	nCurMask = 1;

	for( i = 0 ; i < 32 ; i++ , nCurMask <<= 1 )
	{
		if( nTblMask & nCurMask )
		{
			switch( (nTblMask & nCurMask ) )
			{
			case MAPID_MAININFO :
				fp->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadMapInfo( fp );
				if( fp->Tell() != nOffsetTbl[i+1] )
					LoadResInfo( fp );
				else
				{
#ifdef _USAGE_TOOL_
					memset( &m_ResInfo, 0 , sizeof(RESOURCE_INFO) );
#endif
				}
				nCnt++;
				break;
			case MAPID_HEIGHT :
				fp->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadHeight( fp );
				nCnt++;
				break;
			case MAPID_BRIGHT :
				fp->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadBrightMap( fp );
				nCnt++;
				break;
			case MAPID_COLOR :
				fp->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadColorMap( fp );
				nCnt++;
				break;
			case MAPID_LIGHT :
				fp->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadLightInfo( fp );
				nCnt++;
				break;
			case MAPID_PROP :
				fp->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadProp( fp );
				if( fp->Tell() != nOffsetTbl[i+1] )
				{
					LoadPropExtensionData( fp );
				}
				nCnt++;
				break;
			case MAPID_ATTRIBUTE :
				fp->Seek( nOffsetTbl[i] , BStream::fromBegin );

				// 이전 맵 처리 때문에.. 오프셋이 같으면 다음으로 그냥 넘어감.
				if( nOffsetTbl[i] != nOffsetTbl[i+1] )
					LoadAttributeMap( fp );
				nCnt++;
				break;
			case MAPID_MAPSET :
				fp->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadMapSet( fp );
				nCnt++;
				break;
			case MAPID_BILLBOARD :
				fp->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadBillboard( fp );
				nCnt++;
				break;
			}

			if( nCnt == nTblCnt )
				break;
		}
	}

	return true;

}

BOOL	CBSMapCore::ChangeAttrDataIntoGameAttr(VOID)
{
	m_pAttrManager->ChangeAttrBufToGameBuf( m_pTempAttrBuffer );

	return TRUE;
}


#endif

BOOL	CBSMapCore::CreateTempBuffer(int xsize,int ysize)
{
	DeleteTempBuffer();
	m_pTempAttrBuffer = new BYTE[ xsize * ysize ];

	if( m_pTempAttrBuffer )
		return TRUE;
	return FALSE;
}

VOID	CBSMapCore::DeleteTempBuffer(VOID)
{
	if( m_pTempAttrBuffer )
	{
		delete [] m_pTempAttrBuffer;
		m_pTempAttrBuffer = NULL;
	}
}

BOOL	CBSMapCore::CreateDiagonalAttrMap(BYTE* pAttrMapBuf,BYTE* pAttrMapExBuf,int xsize,int ysize)
{
	if( ( pAttrMapBuf == NULL ) || ( pAttrMapExBuf == NULL ) )
		return FALSE;

	BYTE *pBuffer = pAttrMapBuf;
	BYTE *pBufferEx = pAttrMapExBuf;

	// Test Siva
	// 맵 대각선 어튜리비트 생성
	///////////////////
	int nCurX[4];
	int nCurY[4];
	bool bCheckFlag[4] = { false, };
	BYTE cHeroMask = 0;

	for( int j=0; j<ysize; j++ ) 
	{
		for( int i=0; i<xsize; i++ ) 
		{
			// 왼쪽위, 오른쪽 아래
			BYTE CurAttr = *(pBuffer + (xsize*j) + i );
			BYTE CurAttrEx = *(pBufferEx + (xsize*j) + i);

			if( CurAttr != 0 ) continue;				// 이미 속성 존재 하면 넘김.
			if( CurAttrEx != 0 ) continue;		// 확장 속성 존재 하면 넘김.

			// Top,Left 부터 시계방향으로
			nCurX[0] = i, nCurY[0] = j-1;
			nCurX[1] = i+1, nCurY[1] = j;
			nCurX[2] = i, nCurY[2] = j+1;
			nCurX[3] = i-1, nCurY[3] = j;

			cHeroMask = 0;
			for( int k=0; k<4; k++ ) 
			{
				// 범위 밖에여도 막힌걸로 판단.
				if( nCurX[k] == -1 || nCurY[k] == -1 || nCurX[k] == xsize || nCurY[k] == ysize ) 
				{
					bCheckFlag[k] = false;
				}
				else 
				{
					BYTE Attr = *( pBuffer + (xsize*nCurY[k]) + nCurX[k] );
					BYTE AttrEx = *( pBufferEx + (xsize*nCurY[k]) + nCurX[k] );

					// 속성이 있고, 다리 속성이 아니고, 대각선 속성이 없을 때.
					if( Attr != 0 && Attr != 4 && ( ( AttrEx & 0x03 ) == 0 ) ) 
					{
						bCheckFlag[k] = true;
					}
					else bCheckFlag[k] = false;
					if( ( Attr & 0x10 ) >> 4 == 1 ) cHeroMask = 0x10;
				}
			}

			BYTE ResultAttr = 0;

			if( bCheckFlag[0] == true && bCheckFlag[1] == true && bCheckFlag[2] == false && bCheckFlag[3] == false ) 
				ResultAttr = 2;
			else if( bCheckFlag[0] == false && bCheckFlag[1] == true && bCheckFlag[2] == true && bCheckFlag[3] == false ) 
				ResultAttr = 1;
			else if( bCheckFlag[0] == false && bCheckFlag[1] == false && bCheckFlag[2] == true && bCheckFlag[3] == true ) 
				ResultAttr = 4;
			else if( bCheckFlag[0] == true && bCheckFlag[1] == false && bCheckFlag[2] == false && bCheckFlag[3] == true ) 
				ResultAttr = 3;
			if( ResultAttr != 0 ) ResultAttr |= cHeroMask;

			// *( pBuffer + (xsize*j)+i ) = ( ( ResultAttr << 6 ) & 0xC0 ) + ( CurAttr & 0x3F );
			*( pBufferEx + (xsize*j)+i ) = ResultAttr; // ( ( ResultAttr << 4 ) & 0xF0 ); // + ( CurAttr & 0x0F );
		}
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

bool	CBSMapCore::LoadAndProcessFile(BStream *fp)
{
	int i;
	int nOffsetTbl[32] = { 0,};
	char szType[4]; // = "FCM";
	int nVersion; // = FC_MAP_VER;

	memset( nOffsetTbl , 0x00 , sizeof( nOffsetTbl ) );

	// 1) 
	fp->Read( szType , 4 );

	if( strcmp( szType , "FCM" ) != 0 )
		return false;	// 맵파일이 아니구먼..

	// 2)
	fp->Read( &nVersion , sizeof(int) , ENDIAN_FOUR_BYTE );

#ifdef WIN32
//	TRACE("맵파일 버전 : %d \n", nVersion );
#endif

	// Offset Table Count
	int nTblMask = 0x00000002;
	int nTblMaskForLoad = nTblMask;

	int nTblCnt = 1;
	int nTblCntForLoad = nTblCnt;
	int nEmptyValue = 0;

	// 3)
	fp->Read( &nTblMask , sizeof(int) , ENDIAN_FOUR_BYTE );

	// 4)
	fp->Read( &nTblCntForLoad , sizeof(int) , ENDIAN_FOUR_BYTE );

	DWORD dwTempPos = (DWORD)fp->Tell();

	nTblCnt = CalcBitCnt( nTblMask );

	if( nTblCnt != nTblCntForLoad )
	{
#ifdef WIN32
//		TRACE("뭔가 이상이 있는겨.. TblCnt [%d] , TblCntForLoad [%d]\n", nTblCnt , nTblCntForLoad );
#endif
		return false;
	}

	int nTempPos;
	int nCnt = 0;
	int nCurMask = 1;

	m_bHasExAttrMap = FALSE;

	// 오프셋들만 따로 저장.
	for( i = 0 ; i < 32 ; i++ , nCurMask <<= 1 )
	{
		if( nTblMask & nCurMask )
		{
			fp->Read( &nTempPos , sizeof(int) , ENDIAN_FOUR_BYTE );
			nOffsetTbl[i] = nTempPos;
			nCnt++;

			if( nCnt == nTblCnt )
				break;
		}
	}

	nCnt = 0;
	nCurMask = 1;

	for( i = 0 ; i < 32 ; i++ , nCurMask <<= 1 )
	{
		if( nTblMask & nCurMask )
		{
			switch( (nTblMask & nCurMask ) )
			{
			case MAPID_MAININFO :
				fp->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadMapInfo( fp );

				if( ( i < 31 ) && ( fp->Tell() != nOffsetTbl[i+1] ) )
				{
					LoadResInfo( fp );
				}
				else
				{
#ifdef _USAGE_TOOL_
					memset( &m_ResInfo, 0 , sizeof(RESOURCE_INFO) );
					// LoadResInfo( NULL );	// 초기화 시킴.
#endif
				}
				nCnt++;
				break;
			case MAPID_HEIGHT :
				fp->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadHeight( fp );
				nCnt++;
				break;
			case MAPID_BRIGHT :
				fp->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadBrightMap( fp );
				nCnt++;
				break;
			case MAPID_COLOR :
				fp->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadColorMap( fp );
				nCnt++;
				break;
			case MAPID_LIGHT :
				fp->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadLightInfo( fp );
				nCnt++;
				break;
			/*
			case MAPID_SKINLIST :
				fp->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadSkinPack( fp );
				nCnt++;
				break;
			*/
			case MAPID_PROP :
				fp->Seek( nOffsetTbl[i] , BStream::fromBegin );

				if( nVersion != FC_MAP_VER )
					LoadPropOldVer( fp );
				else
                    LoadProp( fp );

				if( fp->Tell() != nOffsetTbl[i+1] )
				{
                    LoadPropExtensionData( fp );
				}
				nCnt++;
				break;
			case MAPID_ATTRIBUTE :
				fp->Seek( nOffsetTbl[i] , BStream::fromBegin );

				// 이전 맵 처리 때문에.. 오프셋이 같으면 다음으로 그냥 넘어감.
				if( nOffsetTbl[i] != nOffsetTbl[i+1] )
                    LoadAttributeMap( fp );
				nCnt++;
				break;
			case MAPID_AREA :
				fp->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadArea( fp );
				if( fp->Tell() != nOffsetTbl[i+1])
				{
					LoadAreaExtensionData( fp );
				}
				nCnt++;
				break;
			case MAPID_TROOP :
				fp->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadTroop( fp );
				nCnt++;
				break;
			case MAPID_MAPSET :
				fp->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadMapSet( fp );
				nCnt++;
				break;
			case MAPID_WATER :
				fp->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadWaterLine( fp );
				nCnt++;
				break;
			case MAPID_PATH :
				fp->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadPath( fp );
				nCnt++;
				break;
			case MAPID_NAVMESH :
				fp->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadNavList( fp );
				nCnt++;
				break;
			case MAPID_NAVEVENT :
				fp->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadNavEventList( fp );
				nCnt++;
				break;
			case MAPID_COLLISION :
				fp->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadColList( fp );
				nCnt++;
				break;
			case MAPID_SKINLIST :
				fp->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadUsedSkinList( fp );
				nCnt++;
				break;
			case MAPID_ATTREXMAP :
				fp->Seek( nOffsetTbl[i], BStream::fromBegin );
				LoadAttrByteMap( fp );
				nCnt++;
				break;
			case MAPID_BILLBOARD :
				fp->Seek( nOffsetTbl[i], BStream::fromBegin );
				LoadBillboard( fp );
				nCnt++;
				break;
			case MAPID_PROPZONEDATA:
				fp->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadPropZoneData( fp );
				nCnt++;
				break;
			case MAPID_ATTRIBUTEEX :
				fp->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadAttributeMapEx( fp );
				m_bHasExAttrMap = TRUE;
				nCnt++;
				break;
			}

			if( nCnt == nTblCnt )
				break;
		}
	}

#ifndef _USAGE_TOOL_

	if( !m_bHasExAttrMap )
	{
		CreateAttrExBuffer( m_nAttrXSize, m_nAttrYSize );
        CreateDiagonalAttrMap( m_pAttrMap, m_pAttrMapEx, m_nAttrXSize, m_nAttrYSize );
	}

#endif

	return true;
}


int		CBSMapCore::LoadMapInfo(BStream *fp)
{
	int nSize;
	int nOffset = 0;

	InitLayerInfo();

	for( int i = 0 ; i < 6 ; i++ )
	{
		fp->Read( &nSize , sizeof(int) , ENDIAN_FOUR_BYTE );
		
		nOffset += sizeof(int);

		if( nSize != 0 )
		{
			fp->Read( m_szLayer[i] , nSize );
			nOffset += nSize;
		}
	}

	return nOffset;
}

int		CBSMapCore::LoadResInfo(BStream *fp)
{
	if( fp == NULL )
	{
#ifdef _USAGE_TOOL_
		memset( &m_ResInfo, 0 , sizeof(RESOURCE_INFO) );
#endif
		return 0;
	}

	int nSize;
	int nOffset = 0;

	fp->Read( &nSize , sizeof(int) , ENDIAN_FOUR_BYTE );
	nOffset += sizeof(int);

#ifdef _USAGE_TOOL_

	if( nSize != 0 )
	{
		fp->Read( &m_ResInfo, nSize );
		nOffset += nSize;
	}

#endif

	return nOffset;
}

int		CBSMapCore::LoadHeight(BStream *fp)
{
	int nXSize;
	int nYSize;
	int nBuffersize;
	int nOffset;

	fp->Read( &nXSize , sizeof(int) , ENDIAN_FOUR_BYTE );
	fp->Read( &nYSize , sizeof(int) , ENDIAN_FOUR_BYTE );
	fp->Read( &nBuffersize , sizeof(int) , ENDIAN_FOUR_BYTE );

	// 일단 여기에
	m_nXSize = nXSize;
	m_nYSize = nYSize;

	nOffset = nBuffersize / sizeof(short);

	// 버퍼 만들기
	InitHeightMap( nXSize , nYSize );

	fp->Read( m_pHeightData , nBuffersize , ENDIAN_TWO_BYTE );

	return nOffset;
}

int		CBSMapCore::LoadBrightMap(BStream *fp)
{
	int nXSize;
	int nYSize;
	int nBuffersize;
	int nOffset;

	fp->Read( &nXSize , sizeof(int) , ENDIAN_FOUR_BYTE );
	fp->Read( &nYSize , sizeof(int) , ENDIAN_FOUR_BYTE );
	fp->Read( &nBuffersize , sizeof(int) , ENDIAN_FOUR_BYTE );

	nOffset = nBuffersize / sizeof(DWORD);

	// 버퍼 만들기
	InitBrightMap( nXSize , nYSize );

	fp->Read( m_pdwMulColor , nBuffersize , ENDIAN_FOUR_BYTE );

	return nOffset;
}

int		CBSMapCore::LoadColorMap(BStream *fp)
{
	int nXSize;
	int nYSize;
	int nBuffersize;
	int nOffset;

	fp->Read( &nXSize , sizeof(int) , ENDIAN_FOUR_BYTE );
	fp->Read( &nYSize , sizeof(int) , ENDIAN_FOUR_BYTE );
	fp->Read( &nBuffersize , sizeof(int) , ENDIAN_FOUR_BYTE );

	nOffset = nBuffersize / sizeof(DWORD);

	// 버퍼 만들기
	InitAddColorMap( nXSize , nYSize );

	fp->Read( m_pdwAddColor , nBuffersize , ENDIAN_FOUR_BYTE );

	return nOffset;
}

int		CBSMapCore::LoadProp(BStream *fp)
{
	int nSize;
	int nSuccessCnt=0;
	int nOffset = 0;

	PROP_INFO PropInfo;
	memset( &PropInfo, 0, sizeof(PROP_INFO) );

	short nX,nY; // dwParam2;

	float fMaxXSize = (float)(m_nXSize * 200);
	float fMaxZSize = (float)(m_nYSize * 200);


#ifndef _USAGE_TOOL_
	m_PropList.clear();
#else
	// m_pPropManager->MakePropPool( )

#endif
	const int propInfoSize = sizeof(PROP_INFO) - sizeof(PROP_INFO_EXT);

	fp->Read( &nSize , sizeof(int) , ENDIAN_FOUR_BYTE );			// 0) Prop 갯수		- 4 Bytes

	for( int i = 0 ; i < nSize ; i++ )
	{
		fp->Read(&PropInfo, propInfoSize , ENDIAN_FOUR_BYTE);

#ifndef _USAGE_TOOL_
		m_PropList.push_back(PropInfo);
#endif

		fp->Read( &nX , sizeof( WORD ) , ENDIAN_TWO_BYTE );			// 8) Attribute X	- 2 Bytes
		fp->Read( &nY , sizeof( WORD ) , ENDIAN_TWO_BYTE );			// 9) Attribute Y	- 2 Bytes

		nOffset += 32;

		if( PropInfo.Pos.fPos[0] < 0.f || PropInfo.Pos.fPos[0] > fMaxXSize )
			continue;
		if( PropInfo.Pos.fPos[2] < 0.f || PropInfo.Pos.fPos[2] > fMaxZSize )
			continue;

#ifdef _USAGE_TOOL_
		
		if( m_pPropManager->AddNewPropFromLoad( 
			PropInfo.nSkinIndex, 
			//fX,fY,fZ,
			PropInfo.Pos.fPos[0],
			PropInfo.Pos.fPos[1],
			PropInfo.Pos.fPos[2],
			PropInfo.dwParam1 ,(int)nX, (int)nY ) )
		{
			m_pPropManager->GetLatestProp()->SetUId( PropInfo.nUId );

			// 추가됨.
			// m_pPropManager->GetLatestProp()->SetPropName( )
			++nSuccessCnt;
		}
		
		/*
		Rotate 의 경우 1024 값을 4로 나눈 값을 사용. 255단계로 나눔.
		복원시 

		( (BYTE)(Data>>24) ) * 4

		Scale 의 경우 0.01 단계로, 배율 0.5 ~ 1.5 값까지 적용됨.
		복원시

		0.5f +  float((BYTE)(Data>>16)) * 0.01f;
		*/
#endif

	}

#ifdef _USAGE_TOOL_
	TRACE("--- CBSMapCore::LoadProp : Props cnt [%d] , Success cnt[%d]\n",nSize,nSuccessCnt);
#endif

	return nOffset;
}

int		CBSMapCore::LoadBillboard(BStream *fp)
{
	int nSize;
	int nSuccessCnt=0;
	int nOffset = 0;

	SIMPLE_BILLBOARD BillboardInfo;
#ifndef _USAGE_TOOL_
	SIMPLE_BILLBOARD* pBBInfo;
#endif
	PROP_INFO PropInfo;

	memset( &PropInfo, 0, sizeof(PROP_INFO) );
	memset( &BillboardInfo, 0 , sizeof(SIMPLE_BILLBOARD) );

	short nX,nY; // dwParam2;
	const int propInfoSize = sizeof(PROP_INFO) - sizeof(PROP_INFO_EXT);

	fp->Read( &nSize , sizeof(int) , ENDIAN_FOUR_BYTE );			// 0) Prop 갯수		- 4 Bytes

	// Test
#ifndef _USAGE_TOOL_
	// m_PropList.clear();
	m_BillboardList.clear();
	m_BillboardList.resize( nSize );

#endif

#ifdef _USAGE_TOOL_
	if( m_nBillboardIndexGap == 0 )
	{
		// m_pPropManager->ClearBillboardPropPools();
		m_pPropManager->ReleaseBillboardProps();
		return nOffset;
	}
#endif

	for( int i = 0 ; i < nSize ; i++ )
	{
		fp->Read(&PropInfo, propInfoSize , ENDIAN_FOUR_BYTE);

#ifndef _USAGE_TOOL_
		// 숫자가 많아져서 열라 느려져서 빼놓는다..
//		m_PropList.push_back(PropInfo);

		pBBInfo = &m_BillboardList[i];

		pBBInfo->vecPos.x = PropInfo.Pos.fPos[0];
		pBBInfo->vecPos.y = PropInfo.Pos.fPos[1];
		pBBInfo->vecPos.z = PropInfo.Pos.fPos[2];
		pBBInfo->nBillboardIndex = PropInfo.nSkinIndex;

		// m_BillboardList.push_back( BillboardInfo );
#endif

		fp->Read( &nX , sizeof( WORD ) , ENDIAN_TWO_BYTE );			// 8) Attribute X	- 2 Bytes
		fp->Read( &nY , sizeof( WORD ) , ENDIAN_TWO_BYTE );			// 9) Attribute Y	- 2 Bytes

		nOffset += 32;

#ifdef _USAGE_TOOL_
				
		if( m_pPropManager->AddNewPropFromLoad( 
			PropInfo.nSkinIndex + m_nBillboardIndexGap, 
			//fX,fY,fZ,
			PropInfo.Pos.fPos[0],
			PropInfo.Pos.fPos[1],
			PropInfo.Pos.fPos[2],
			PropInfo.dwParam1 ,(int)nX, (int)nY ) )
		{
			m_pPropManager->GetLatestProp()->SetUId( PropInfo.nUId );
			++nSuccessCnt;
		}
		/*
		Rotate 의 경우 1024 값을 4로 나눈 값을 사용. 255단계로 나눔.
		복원시 

		( (BYTE)(Data>>24) ) * 4

		Scale 의 경우 0.01 단계로, 배율 0.5 ~ 1.5 값까지 적용됨.
		복원시

		0.5f +  float((BYTE)(Data>>16)) * 0.01f;
		*/
#endif
	}

#ifdef _USAGE_TOOL_
	TRACE("--- CBSMapCore::LoadBillboard : Props cnt [%d] , Success cnt[%d]\n",nSize,nSuccessCnt);
#endif

	return nOffset;
}

int CBSMapCore::LoadPropZoneData( BStream *fp )
{
	if( m_pZoneManager )
		delete m_pZoneManager;

	m_pZoneManager = new MapFilePropZoneManager;

	fp->Read( &m_pZoneManager->m_nZoneSize, sizeof(int), 4 );
	fp->Read( &m_pZoneManager->m_nXSize, sizeof(int), 4 );
	fp->Read( &m_pZoneManager->m_nYSize, sizeof(int), 4 );

	if( m_pZoneManager->m_pZoneData )
		delete[] m_pZoneManager->m_pZoneData;

	m_pZoneManager->m_pZoneData = new MAPFILEPROPZONEDATA[m_pZoneManager->m_nXSize * m_pZoneManager->m_nYSize ];

	for( int i=0; i<m_pZoneManager->m_nXSize * m_pZoneManager->m_nYSize; i++ )
	{
		fp->Read( &(m_pZoneManager->m_pZoneData[i].bProp), sizeof(bool) );
		fp->Read( &(m_pZoneManager->m_pZoneData[i].Box.C), sizeof(float)*3, 4 );

		// BSVector땜시 루프돈다.
		for( int j=0; j<3; j++ )
			fp->Read( &(m_pZoneManager->m_pZoneData[i].Box.A[j]), sizeof(float)*3, 4 );

		fp->Read( &(m_pZoneManager->m_pZoneData[i].Box.E), sizeof(float)*3, 4 );

		for( int j=0; j<8; j++ )
			fp->Read( &(m_pZoneManager->m_pZoneData[i].Box.V[j]), sizeof(float)*3, 4 );
	}
	return true;


	return 0;
}


int		CBSMapCore::LoadPropOldVer(BStream *fp)
{
	int nSize;
	int nSuccessCnt=0;
	int nOffset = 0;

	PROP_INFO PropInfo;
	memset( &PropInfo, 0, sizeof(PROP_INFO) );

	short nX,nY; // dwParam2;

#ifndef _USAGE_TOOL_
	m_PropList.clear();
#else
	// m_pPropManager->MakePropPool( )
#endif

	const int propInfoSize = sizeof(PROP_INFO) - sizeof(PROP_INFO_EXT);

	fp->Read( &nSize , sizeof(int) , ENDIAN_FOUR_BYTE );			// 0) Prop 갯수		- 4 Bytes

	for( int i = 0 ; i < nSize ; i++ )
	{
		fp->Read(&PropInfo, propInfoSize , ENDIAN_FOUR_BYTE);

#ifndef _USAGE_TOOL_
		PropInfo.Pos.fPos[0] = (float)PropInfo.Pos.dwPos[0];
		PropInfo.Pos.fPos[1] = (float)PropInfo.Pos.dwPos[1];
		PropInfo.Pos.fPos[2] = (float)PropInfo.Pos.dwPos[2];

		m_PropList.push_back(PropInfo);
#endif

		fp->Read( &nX , sizeof( WORD ) , ENDIAN_TWO_BYTE );			// 8) Attribute X	- 2 Bytes
		fp->Read( &nY , sizeof( WORD ) , ENDIAN_TWO_BYTE );			// 9) Attribute Y	- 2 Bytes

		nOffset += 32;

#ifdef _USAGE_TOOL_

		// float fX,fY,fZ;
		PropInfo.Pos.fPos[0] = (float)PropInfo.Pos.dwPos[0];
		PropInfo.Pos.fPos[1] = (float)PropInfo.Pos.dwPos[1];
		PropInfo.Pos.fPos[2] = (float)PropInfo.Pos.dwPos[2];
		
		if( m_pPropManager->AddNewPropFromLoad( 
			PropInfo.nSkinIndex, 
			PropInfo.Pos.fPos[0],
			PropInfo.Pos.fPos[1],
			PropInfo.Pos.fPos[2],
			PropInfo.dwParam1 ,(int)nX, (int)nY ) )
		{
			m_pPropManager->GetLatestProp()->SetUId( PropInfo.nUId );

			// 추가됨.
			// m_pPropManager->GetLatestProp()->SetPropName( )
			++nSuccessCnt;
		}

		/*
		Rotate 의 경우 1024 값을 4로 나눈 값을 사용. 255단계로 나눔.
		복원시 

		( (BYTE)(Data>>24) ) * 4

		Scale 의 경우 0.01 단계로, 배율 0.5 ~ 1.5 값까지 적용됨.
		복원시

		0.5f +  float((BYTE)(Data>>16)) * 0.01f;
		*/
#endif

	}

#ifdef _USAGE_TOOL_
	TRACE("--- CBSMapCore::LoadProp : Props cnt [%d] , Success cnt[%d]\n",nSize,nSuccessCnt);
#endif

	return nOffset;
}

int		CBSMapCore::LoadPropExtensionData(BStream *fp)
{
	int nSize;
	int nBufSize;
	int nIndex;
	int nSuccessCnt=0;
	int nOffset = 0;
	PROP_INFO_EXT	PropExtInfo;
	BOOL bHasExtBuffer;
	memset( &PropExtInfo, 0, sizeof(PROP_INFO_EXT));

	int nPoolSize;

#ifdef _USAGE_TOOL_
	nPoolSize = m_pPropManager->GetPropCount();
#else
	nPoolSize = (int)m_PropList.size();
#endif

	fp->Read( &nSize, sizeof(int), ENDIAN_FOUR_BYTE );
	fp->Read( &nBufSize, sizeof(int), ENDIAN_FOUR_BYTE );
	nOffset += ( sizeof(int) + sizeof(int) );

	int nParamSize;

	if( nBufSize == sizeof(PROP_INFO_EXT) )
	{
        nParamSize = nBufSize - ( CAPTION_MAX + STR_MAX );
		bHasExtBuffer = TRUE;
	}
	else
	{
		nParamSize = nBufSize - CAPTION_MAX;
		bHasExtBuffer = FALSE;
	}

	for( int i = 0; i < nSize ; ++i )
	{
		fp->Read( &nIndex, sizeof(int), ENDIAN_FOUR_BYTE );

		fp->Read( PropExtInfo.szPropName, CAPTION_MAX );

		if( bHasExtBuffer )
			fp->Read( PropExtInfo.szExtBuffer , STR_MAX );

		fp->Read( PropExtInfo.dwParamExt, nParamSize, ENDIAN_FOUR_BYTE );

		nOffset += (sizeof(int) + nBufSize);

		if( nIndex < nPoolSize )
		{

#ifdef _USAGE_TOOL_
			CBSPropPtr pProp;

			pProp = (*m_pPropManager)[nIndex];
			pProp->SetPropName( PropExtInfo.szPropName );
			if( bHasExtBuffer )
                pProp->SetExtBuffer( PropExtInfo.szExtBuffer );

			memcpy( pProp->m_dwParamExt,
				PropExtInfo.dwParamExt, sizeof(DWORD) * PROP_PARAM_EXT_MAX  );
			++nSuccessCnt;
#else
            memcpy( m_PropList[nIndex].szPropName, PropExtInfo.szPropName, CAPTION_MAX );
			memcpy( m_PropList[nIndex].szExtBuffer, PropExtInfo.szExtBuffer, STR_MAX );
			memcpy( m_PropList[nIndex].dwParamExt, PropExtInfo.dwParamExt, 
				sizeof(DWORD) * PROP_PARAM_EXT_MAX );
#endif
		}
	}

	fp->Read( &nSize, sizeof(int), ENDIAN_FOUR_BYTE ); // -1 값이면 종료.
	nOffset += sizeof(int);

	return nOffset;
}



int		CBSMapCore::LoadUsedSkinList(BStream *fp)
{
	int nOffset = 0;
	int nSize;
	
	fp->Read( &nSize , sizeof(int) , ENDIAN_FOUR_BYTE );
	nOffset += sizeof(int);

	m_SkinIndexList.clear();

	if( nSize )
	{
		m_SkinIndexList.resize( nSize );

		//for( int i = 0 ; i < nSize ; ++i )
		{
			fp->Read( &m_SkinIndexList[0], nSize * sizeof(int), ENDIAN_FOUR_BYTE );
		}

		nOffset += nSize * sizeof(int);
	}

	return nOffset;
}

int		CBSMapCore::LoadAttrByteMap(BStream *fp)
{
	int nOffset = 0;

#ifndef _USAGE_TOOL_
	int nChunk;
	int nx;
	int ny;

	fp->Read( &nChunk, sizeof(int), ENDIAN_FOUR_BYTE );

	for( int nC = 0 ; nC < nChunk ; ++nC )
	{
		if( nC == 0 )
		{
            fp->Read( &nx, sizeof(int), ENDIAN_FOUR_BYTE );
            fp->Read( &ny, sizeof(int), ENDIAN_FOUR_BYTE );

            m_nABXSize = nx;
            m_nABYSize = ny;

			if( nx > 0 && ny > 0 )
			{
				m_pAttrByteMap = new BYTE[ nx * ny ];
				fp->Read( m_pAttrByteMap, nx * ny );
			}
		}
		else if( nC == 1 )
		{
			for( int i = 0 ; i < 5 ; ++i )
			{
                fp->Read( &m_nLayerAttr[i], sizeof(int), ENDIAN_FOUR_BYTE );
			}
		}
	}

	nOffset = 12 + nx * ny + 20;
#else

	m_pAttrByteMap = NULL;
	m_nABXSize = 0;
	m_nABYSize = 0;

	int nChunk;
	int nx;
	int ny;

	fp->Read( &nChunk, sizeof(int), ENDIAN_FOUR_BYTE );

	for( int nC = 0 ; nC < nChunk ; ++nC )
	{
		if( nC == 0 )
		{
			fp->Read( &nx, sizeof(int), ENDIAN_FOUR_BYTE );
			fp->Read( &ny, sizeof(int), ENDIAN_FOUR_BYTE );
			fp->Seek( nx * ny , BStream::fromNow );
		}
		else if( nC == 1 )
		{
			for( int i = 0 ; i < 5 ; ++i )
			{
				fp->Read( &m_nLayerAttr[i], sizeof(int), ENDIAN_FOUR_BYTE );
			}
		}
	}
	nOffset = 12 + nx * ny + 20;
#endif

	return nOffset;
}
#ifndef _USAGE_TOOL_
BYTE CBSMapCore::GetLayerAttr(int x,int y)
{

	if(m_pAttrByteMap)
	{
		return *( m_pAttrByteMap + x + y * m_nABXSize );
	}
	else{
		return 0;
	}
}
#endif

int		CBSMapCore::LoadLightInfo(BStream *fp)
{
    int nOffset = 0;
	int nCnt;
	D3DCOLORVALUE	ColorValue;
	DWORD dwXYZ[3];
	DWORD dwR,dwG,dwB,dwA,dwTemp;

	fp->Read( dwXYZ , sizeof(DWORD) * 3 , ENDIAN_FOUR_BYTE );
#ifdef _USAGE_TOOL_
	memcpy( &m_pLightCross->m_XVector , dwXYZ , sizeof(float) * 3 );
#endif

	fp->Read( dwXYZ , sizeof(DWORD) * 3 , ENDIAN_FOUR_BYTE );
#ifdef _USAGE_TOOL_
	memcpy( &m_pLightCross->m_YVector , dwXYZ , sizeof(float) * 3 );
#endif

	fp->Read( dwXYZ , sizeof(DWORD) * 3 , ENDIAN_FOUR_BYTE );
#ifdef _USAGE_TOOL_
	memcpy( &m_pLightCross->m_ZVector , dwXYZ , sizeof(float) * 3 );
#endif

	fp->Read( dwXYZ , sizeof(DWORD) * 3 , ENDIAN_FOUR_BYTE );
#ifdef _USAGE_TOOL_
	memcpy( &m_pLightCross->m_PosVector , dwXYZ , sizeof(float) * 3 );
#endif

	// 라이트 요소 갯수
	fp->Read( &nCnt , sizeof(int) , ENDIAN_FOUR_BYTE );
	nOffset += 4;

	for( int i = 0 ; i < nCnt ; i++ )
	{
		fp->Read( &dwA, sizeof(DWORD), ENDIAN_FOUR_BYTE );
		fp->Read( &dwR, sizeof(DWORD), ENDIAN_FOUR_BYTE );
		fp->Read( &dwG, sizeof(DWORD), ENDIAN_FOUR_BYTE );
		fp->Read( &dwB, sizeof(DWORD), ENDIAN_FOUR_BYTE );

		fp->Read( &dwTemp, sizeof(DWORD), ENDIAN_FOUR_BYTE );

		ColorValue.a = (float)dwA;
		ColorValue.r = (float)dwR;
		ColorValue.g = (float)dwG;
		ColorValue.b = (float)dwB;

#ifdef _USAGE_TOOL_
		m_dwLight[i] = D3DColorToRGB( ColorValue );

		// 라이트 적용.

#else
		// 게임의 경우 ColorValue 값을 그대로 쓴다.
#endif

		nOffset += 20;
	}

	return nOffset;
}

int		CBSMapCore::LoadArea(BStream *fp)
{
	int nOffset = 0;
	int nCnt;
	AREA_PACK	areaPack;
	AREA_INFO	areaInfo;
	DWORD		dwTemp[2];

	memset( &areaPack, 0, sizeof(AREA_PACK));

#ifndef _USAGE_TOOL_
	m_AreaList.clear();
#endif


	fp->Read( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );
	nOffset += 4;

	for( int i = 0 ; i < nCnt ; i++ )
	{
		fp->Read( &areaInfo, sizeof(AREA_INFO), ENDIAN_FOUR_BYTE );
		fp->Read( dwTemp, sizeof(DWORD)*2 , ENDIAN_FOUR_BYTE );
		nOffset += 32;

#ifdef _USAGE_TOOL_
		m_pAreaManager->AddBlankArea( &areaInfo );
#else
		areaPack.m_areaInfo = areaInfo;
		m_AreaList.push_back( areaPack );
#endif
	}

	return nOffset;
}

// 확장 데이터 읽기.
int		CBSMapCore::LoadAreaExtensionData(BStream *fp)
{
	int nOffset = 0;
	AREA_ATTR	areaAttr;
#ifdef _USAGE_TOOL_
	CBSAreaPack *pArea;
#endif

	int nAreaCnt;
	int nExtBufSize;
	int nNumBufSize;
	int nTxtBufSize;

	int	nTxtBufOffset = sizeof(AREA_ATTR) - CAPTION_MAX;

	fp->Read( &nAreaCnt, sizeof(int), ENDIAN_FOUR_BYTE );		nOffset += 4;
	fp->Read( &nExtBufSize, sizeof(int), ENDIAN_FOUR_BYTE );	nOffset += 4;
	fp->Read( &nNumBufSize, sizeof(int), ENDIAN_FOUR_BYTE );	nOffset += 4;
	fp->Read( &nTxtBufSize, sizeof(int), ENDIAN_FOUR_BYTE );	nOffset += 4;

	for( int i = 0 ; i < nAreaCnt ; ++i )
	{
#ifdef _USAGE_TOOL_
		pArea = m_pAreaManager->GetArea( i );
#endif
		if( nNumBufSize )
		{
            fp->Read( &areaAttr, nNumBufSize, ENDIAN_FOUR_BYTE );
			nOffset += nNumBufSize;
#ifndef _USAGE_TOOL_	// 게임일때
			memcpy( &m_AreaList[i].m_areaAttr, &areaAttr, nNumBufSize );
#endif
		}

		if( nTxtBufSize )
		{
			fp->Read( (&areaAttr) + nTxtBufOffset , nTxtBufSize );
			nOffset += nTxtBufSize;

#ifndef _USAGE_TOOL_	// 게임일때
			memcpy( (&m_AreaList[i].m_areaAttr) + nTxtBufOffset, 
				(&areaAttr) + nNumBufSize , nTxtBufSize );
#endif
		}

#ifdef _USAGE_TOOL_
		pArea->SetAreaAttr( &areaAttr );
#endif 
	}

	return nOffset;
}

int		CBSMapCore::LoadTroop(BStream *fp)
{
	int nOffset = 0;
	int nChunkCnt;
	int nCnt;
	int nSize;
	TROOP_INFO	troopInfo;
	int nStrBufOffset;

#ifndef _USAGE_TOOL_
	m_TroopList.clear();
#endif //_USAGE_TOOL_

	fp->Read( &nChunkCnt, sizeof(int), ENDIAN_FOUR_BYTE );
	fp->Read( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );
	nOffset += 8;

	memset( &troopInfo, -1 , sizeof(TROOP_INFO));
	memset( &troopInfo.m_troopAttr.StrBlock, 0 , sizeof(troopInfo.m_troopAttr.StrBlock));

	int nStrBlockSize = sizeof(troopInfo.m_troopAttr.StrBlock);
	nStrBufOffset = sizeof(TROOP_ATTR) - (TROOP_ATTR_STRSIZE + nStrBlockSize);

	for( int i = 0 ; i < nCnt ; i++ )
	{
		// Chunk 1. 기본 영역 정보.
		fp->Read( &nSize, sizeof(int), ENDIAN_FOUR_BYTE );
		fp->Read( &troopInfo.m_areaInfo , nSize, ENDIAN_FOUR_BYTE );
		nOffset += (nSize + 4);

		if( nChunkCnt > 1 )
		{
            // Chuck 2. Troop 속성 정보.
            fp->Read( &nSize, sizeof(int), ENDIAN_FOUR_BYTE );

			int   nStcSize = nSize - TROOP_ATTR_STRSIZE;
			BYTE* pBuffer = (BYTE*)&troopInfo.m_troopAttr;
			
            fp->Read( pBuffer , nStcSize , ENDIAN_FOUR_BYTE );

			pBuffer += nStrBufOffset; // nStcSize;
			fp->Read( pBuffer, TROOP_ATTR_STRSIZE );

			nOffset += (nSize + 4);

			if( nStrBufOffset > nStcSize )
			{
				troopInfo.m_troopAttr.SetDefExtBuffer();
			}
		}

		if( nChunkCnt > 2 )
		{
			fp->Read( &nSize, sizeof(int), ENDIAN_FOUR_BYTE );
			fp->Read( &troopInfo.m_troopAttr.StrBlock, nSize );

			// -1값 채크.
			for( int n = 0 ; n < 5 ; ++n )
			{
                if( troopInfo.m_troopAttr.StrBlock.szUnitAI[n][0] == -1 )
					troopInfo.m_troopAttr.StrBlock.szUnitAI[n][0] = NULL;
			}

			if( troopInfo.m_troopAttr.StrBlock.szLeaderUnitAI[0] == -1 )
				troopInfo.m_troopAttr.StrBlock.szLeaderUnitAI[0] = NULL;
		}
		else
		{
			memset( &troopInfo.m_troopAttr.StrBlock, 0, CAPTION_MAX * 6 );
		}

#ifdef _USAGE_TOOL_
		CBSTroopPack*	pTroop;
		pTroop = m_pTroopManager->GetBlankElement();	// 새로운 노드 얻어옴.

		{
			if( troopInfo.m_troopAttr.nLeaderUnitType == 59 )
				troopInfo.m_troopAttr.nLeaderUnitType = 89;

			for( int t = 0 ; t < 5 ; ++t )
			{
				if( troopInfo.m_troopAttr.nUnitType[t] == 59 )
					troopInfo.m_troopAttr.nUnitType[t] = 89;
			}
		}

#ifdef _DEBUG
		TRACE("TroopInfo : AID[%d] TID[%d] Txt[%s]\n", 
			troopInfo.m_areaInfo.nUId,
			troopInfo.m_troopAttr.nUId,
			troopInfo.m_troopAttr.szCaption );
#endif
		if( troopInfo.m_areaInfo.fEX - troopInfo.m_areaInfo.fSX < 100.f )
		{
			troopInfo.m_areaInfo.fSX -= 100.f;
			troopInfo.m_areaInfo.fEX += 100.f;
		}

		if( troopInfo.m_areaInfo.fEZ - troopInfo.m_areaInfo.fSZ < 100.f )
		{
			troopInfo.m_areaInfo.fSZ -= 100.f;
			troopInfo.m_areaInfo.fEZ += 100.f;
		}

		if( pTroop )
		{
			pTroop->SetTroopInfo( &troopInfo );
		}
#else
		m_TroopList.push_back(troopInfo);
#endif
	}

	return nOffset;
}

int		CBSMapCore::LoadMapSet(BStream *fp)
{
	int nOffset = 0;
	int nChunkSize;
	int nChunkCnt;

	// 1) Chunk 갯수
	fp->Read( &nChunkCnt, sizeof(int) , ENDIAN_FOUR_BYTE );
	nChunkCnt -= 1;
	nOffset += 4;

	// 기존에 오는 것은 없다.
	ClearLightMapFileName();
	ClearBillboardDDSName();

	for( int i = 0 ; i < nChunkCnt ; ++i )
	{
		switch( i )
		{
		case 0 :
			{
				// 2) Chunk #1 크기
				fp->Read( &nChunkSize, sizeof(int), ENDIAN_FOUR_BYTE );
#ifdef _USAGE_TOOL_
				if( nChunkSize == 172 )  // 테스트...
				{
					fp->Read( m_pDefaultSetInfo, nChunkSize, ENDIAN_FOUR_BYTE );
					m_pDefaultSetInfo->SceneData.fHighlightThreshold = 0.9f;
				}
				else if( nChunkSize == 176 )
				{
					fp->Read( m_pDefaultSetInfo, nChunkSize, ENDIAN_FOUR_BYTE );
				}
				else if( nChunkSize <= ( sizeof(SET_INFO) - 16 ) ) // 16바이트 빠진 옛 데이터.
				{
					fp->Read( &m_pDefaultSetInfo->nVersion, sizeof(int), ENDIAN_FOUR_BYTE );
					fp->Read( &m_pDefaultSetInfo->crossLight.m_XVector, sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
					fp->Read( &m_pDefaultSetInfo->crossLight.m_YVector, sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
					fp->Read( &m_pDefaultSetInfo->crossLight.m_ZVector, sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
					fp->Read( &m_pDefaultSetInfo->crossLight.m_PosVector, sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
					int nRemainBytes = nChunkSize - 52;
					fp->Read( &m_pDefaultSetInfo->clrLightAmbient, nRemainBytes, ENDIAN_FOUR_BYTE );
				}

				if( nChunkSize < 156 )
				{
					m_pDefaultSetInfo->SceneData.fSceneIntensity = 1.f;
					m_pDefaultSetInfo->SceneData.fBlurIntensity = 0.f;
					m_pDefaultSetInfo->SceneData.fGlowIntensity = 0.f;
					m_pDefaultSetInfo->SceneData.fHighlightThreshold = 0.9f;
				}
#else
				if( nChunkSize == sizeof( SET_INFO) ) {
					fp->Read( &m_LightInfo, nChunkSize, ENDIAN_FOUR_BYTE );
				}
				else if( nChunkSize < 156 )
				{
					fp->Read( &m_LightInfo.nVersion, sizeof(int), ENDIAN_FOUR_BYTE );
					fp->Read( &m_LightInfo.crossLight.m_XVector, sizeof(float)*3, ENDIAN_FOUR_BYTE );
					fp->Read( &m_LightInfo.crossLight.m_YVector, sizeof(float)*3, ENDIAN_FOUR_BYTE );
					fp->Read( &m_LightInfo.crossLight.m_ZVector, sizeof(float)*3, ENDIAN_FOUR_BYTE );
					fp->Read( &m_LightInfo.crossLight.m_PosVector, sizeof(float)*3, ENDIAN_FOUR_BYTE );
					int nRemainBytes = nChunkSize - 52;
					fp->Read( &m_LightInfo.clrLightAmbient, nRemainBytes, ENDIAN_FOUR_BYTE );
				}
				else if( nChunkSize == 156 )
				{
					fp->Read( &m_LightInfo.nVersion, sizeof(int), ENDIAN_FOUR_BYTE );
					fp->Read( &m_LightInfo.crossLight.m_XVector, sizeof(float)*3, ENDIAN_FOUR_BYTE );
					fp->Read( &m_LightInfo.crossLight.m_YVector, sizeof(float)*3, ENDIAN_FOUR_BYTE );
					fp->Read( &m_LightInfo.crossLight.m_ZVector, sizeof(float)*3, ENDIAN_FOUR_BYTE );
					fp->Read( &m_LightInfo.crossLight.m_PosVector, sizeof(float)*3, ENDIAN_FOUR_BYTE );

					fp->Read( &m_LightInfo.clrLightAmbient, sizeof(float)*4, ENDIAN_FOUR_BYTE );
					fp->Read( &m_LightInfo.clrLightDiffuse, sizeof(float)*4, ENDIAN_FOUR_BYTE );
					fp->Read( &m_LightInfo.clrLightSpecular, sizeof(float)*4, ENDIAN_FOUR_BYTE );
					fp->Read( &m_LightInfo.vecFogColor, sizeof(float)*4, ENDIAN_FOUR_BYTE );

					fp->Read( &m_LightInfo.fNear, sizeof(float), ENDIAN_FOUR_BYTE );
					fp->Read( &m_LightInfo.fFar, sizeof(float), ENDIAN_FOUR_BYTE );
					fp->Read( &m_LightInfo.fBlur, sizeof(float), ENDIAN_FOUR_BYTE );
					fp->Read( &m_LightInfo.fGlow, sizeof(float), ENDIAN_FOUR_BYTE );

					fp->Read( &m_LightInfo.fProjectionZMax, sizeof( float ) , ENDIAN_FOUR_BYTE );
					fp->Read( m_LightInfo.fSightFar, sizeof( float ) * 2 , ENDIAN_FOUR_BYTE );

					fp->Read( &m_LightInfo.SceneData.fSceneIntensity, sizeof( float ), ENDIAN_FOUR_BYTE );
					fp->Read( &m_LightInfo.SceneData.fBlurIntensity, sizeof( float ), ENDIAN_FOUR_BYTE );
					fp->Read( &m_LightInfo.SceneData.fGlowIntensity, sizeof( float ), ENDIAN_FOUR_BYTE );
				}
				else if( nChunkSize == 172 || nChunkSize == 176 ) // Dummy 붙은것.
				{	
					float fDummy;

					fp->Read( &m_LightInfo.nVersion, sizeof(int), ENDIAN_FOUR_BYTE );
					fp->Read( &m_LightInfo.crossLight.m_XVector, sizeof(float)*3, ENDIAN_FOUR_BYTE );
					fp->Read( &fDummy, sizeof(float), ENDIAN_FOUR_BYTE );
					fp->Read( &m_LightInfo.crossLight.m_YVector, sizeof(float)*3, ENDIAN_FOUR_BYTE );
					fp->Read( &fDummy, sizeof(float), ENDIAN_FOUR_BYTE );
					fp->Read( &m_LightInfo.crossLight.m_ZVector, sizeof(float)*3, ENDIAN_FOUR_BYTE );
					fp->Read( &fDummy, sizeof(float), ENDIAN_FOUR_BYTE );
					fp->Read( &m_LightInfo.crossLight.m_PosVector, sizeof(float)*3, ENDIAN_FOUR_BYTE );
					fp->Read( &fDummy, sizeof(float), ENDIAN_FOUR_BYTE );
					int nRemainBytes = nChunkSize - 52;

					fp->Read( &m_LightInfo.clrLightAmbient, sizeof(float)*4, ENDIAN_FOUR_BYTE );
					fp->Read( &m_LightInfo.clrLightDiffuse, sizeof(float)*4, ENDIAN_FOUR_BYTE );
					fp->Read( &m_LightInfo.clrLightSpecular, sizeof(float)*4, ENDIAN_FOUR_BYTE );
					fp->Read( &m_LightInfo.vecFogColor, sizeof(float)*4, ENDIAN_FOUR_BYTE );

					fp->Read( &m_LightInfo.fNear, sizeof(float), ENDIAN_FOUR_BYTE );
					fp->Read( &m_LightInfo.fFar, sizeof(float), ENDIAN_FOUR_BYTE );
					fp->Read( &m_LightInfo.fBlur, sizeof(float), ENDIAN_FOUR_BYTE );
					fp->Read( &m_LightInfo.fGlow, sizeof(float), ENDIAN_FOUR_BYTE );

					fp->Read( &m_LightInfo.fProjectionZMax, sizeof( float ) , ENDIAN_FOUR_BYTE );
					fp->Read( m_LightInfo.fSightFar, sizeof( float ) * 2 , ENDIAN_FOUR_BYTE );

					fp->Read( &m_LightInfo.SceneData.fSceneIntensity, sizeof( float ), ENDIAN_FOUR_BYTE );
					fp->Read( &m_LightInfo.SceneData.fBlurIntensity, sizeof( float ), ENDIAN_FOUR_BYTE );
					fp->Read( &m_LightInfo.SceneData.fGlowIntensity, sizeof( float ), ENDIAN_FOUR_BYTE );

					if( nChunkSize == 176 )
					{
						fp->Read( &m_LightInfo.SceneData.fHighlightThreshold, sizeof( float ), ENDIAN_FOUR_BYTE );
					}
				}
				
				if( m_LightInfo.crossLight.m_XVector == D3DXVECTOR3( 1,0,0) &&
					m_LightInfo.crossLight.m_YVector == D3DXVECTOR3( 0,1,0) &&
					m_LightInfo.crossLight.m_ZVector == D3DXVECTOR3( 0,0,1) )
				{
					m_LightInfo.crossLight.RotatePitch( 128 );
				}

				if( nChunkSize < 156 )
				{
					m_LightInfo.SceneData.fSceneIntensity = 1.f;
					m_LightInfo.SceneData.fBlurIntensity = 0.f;
					m_LightInfo.SceneData.fGlowIntensity = 0.f;
					m_LightInfo.SceneData.fHighlightThreshold = 0.9f;
				}
#endif
				nOffset += ( nChunkSize + sizeof(int) );
			}
			break;
		case 1 :
			{
				// 3) Chunk #2 크기
				fp->Read( &nChunkSize, sizeof(int), ENDIAN_FOUR_BYTE );
				nOffset += nChunkSize;
				nChunkSize -= sizeof(int);	// 임시영역 4bytes

				ClearSkyboxName();

				if( nChunkSize == 0 )
				{
					// 기본 이름 설정.
					SetSkyboxName("sky.skin");
				}
				else
				{
                    m_SkyboxInfo.m_pszFileName = new char[nChunkSize];
                    memset( m_SkyboxInfo.m_pszFileName , 0 , nChunkSize);

					m_SkyboxInfo.nLen = nChunkSize;
                    fp->Read( m_SkyboxInfo.m_pszFileName , nChunkSize );
				}
			}
			break;
		case 2 :
			{
				// 4) Chunk #3 크기
				fp->Read( &nChunkSize, sizeof(int), ENDIAN_FOUR_BYTE );
				nOffset += nChunkSize;
				nChunkSize -= sizeof(int);

				ClearCubeMapName();

				if( nChunkSize == 0 )
				{
					// 후에 기본 이름 설정할것.
					// SetCubeMapName();
				}
				else
				{
					m_CubeMapInfo.m_pszFileName = new char[ nChunkSize ];
					memset( m_CubeMapInfo.m_pszFileName, 0, nChunkSize );

					m_CubeMapInfo.nLen = nChunkSize;
					fp->Read( m_CubeMapInfo.m_pszFileName, nChunkSize );
				}
			}
			break;
		case 3 :
			{
				// 5) Chunk #4 크기
				fp->Read( &nChunkSize, sizeof(int), ENDIAN_FOUR_BYTE );
				nOffset += nChunkSize;
				nChunkSize -= sizeof(int);

				ClearAIFileName();

				if( nChunkSize == 0 )
				{
				}
				else
				{
					m_AIFileInfo.m_pszFileName = new char[ nChunkSize ];
					memset( m_AIFileInfo.m_pszFileName, 0, nChunkSize );

					m_AIFileInfo.nLen = nChunkSize;
					fp->Read( m_AIFileInfo.m_pszFileName, nChunkSize );
				}
			}
			break;
		case 4 :
			{
				// 5) Chunk #5 크기
				fp->Read( &nChunkSize, sizeof(int), ENDIAN_FOUR_BYTE );
				nOffset += nChunkSize;
				nChunkSize -= sizeof(int);

				ClearTextFileName();

				if( nChunkSize == 0 )
				{
				}
				else
				{
					m_TextFileInfo.m_pszFileName = new char[ nChunkSize ];
					memset( m_TextFileInfo.m_pszFileName, 0, nChunkSize );

					m_TextFileInfo.nLen = nChunkSize;
					fp->Read( m_TextFileInfo.m_pszFileName, nChunkSize );
				}
			}
			break;
		case 5 :
			{
				// 6) Chunk #6 크기
				fp->Read( &nChunkSize, sizeof(int), ENDIAN_FOUR_BYTE );
				nOffset += nChunkSize;
				nChunkSize -= sizeof(int);

				ClearCameraFileName();

				if( nChunkSize == 0 )
				{
				}
				else
				{
					m_CameraFileInfo.m_pszFileName = new char[ nChunkSize ];
					memset( m_CameraFileInfo.m_pszFileName, 0, nChunkSize );

					m_CameraFileInfo.nLen = nChunkSize;
					fp->Read( m_CameraFileInfo.m_pszFileName, nChunkSize );
				}
			}
			break;
		case 6 :
			{
				// 7) Chunk #7 크기
				fp->Read( &nChunkSize, sizeof(int), ENDIAN_FOUR_BYTE );
				nOffset += nChunkSize;
				nChunkSize -= sizeof(int);

				ClearLightMapFileName();

				if( nChunkSize == 0 )
				{
				}
				else
				{
					m_LightMapFileInfo.m_pszFileName = new char[ nChunkSize ];
					memset( m_LightMapFileInfo.m_pszFileName, 0, nChunkSize );

					m_LightMapFileInfo.nLen = nChunkSize;
					fp->Read( m_LightMapFileInfo.m_pszFileName, nChunkSize );
				}
			}
			break;
		case 7 :
			{
				// 8) Chunk #8 크기
				fp->Read( &nChunkSize, sizeof(int), ENDIAN_FOUR_BYTE );
				nOffset += nChunkSize;
				nChunkSize -= sizeof(int);

				ClearBillboardDDSName();

				if( nChunkSize == 0)
				{

				}
				else
				{
                    m_BillboardDDSInfo.m_pszFileName = new char[ nChunkSize ];
					memset( m_BillboardDDSInfo.m_pszFileName, 0, nChunkSize );

					m_BillboardDDSInfo.nLen = nChunkSize;
					fp->Read( m_BillboardDDSInfo.m_pszFileName, nChunkSize );

					CheckBillboardDDS();
				}
			}
			break;
		case 8 :
			{
				fp->Read( &nChunkSize, sizeof(int), ENDIAN_FOUR_BYTE );
				nOffset += nChunkSize;

				fp->Read( &m_nWaterCount, sizeof(int), ENDIAN_FOUR_BYTE );
				nOffset += sizeof(int);
			}
			break;
		}
	}

	return nOffset;
}

int		CBSMapCore::LoadAttributeMap(BStream *fp)
{
	int nVersion = 0;
	int nOffset = 0;
	int xsize,ysize;
	BYTE *pBuffer;

	fp->Read( &nVersion , sizeof(int) , ENDIAN_FOUR_BYTE );

	if( nVersion >= 80 ) // 이전 맵
	{
		xsize = nVersion;
		fp->Read( &ysize , sizeof(int) , ENDIAN_FOUR_BYTE );
		nVersion = 0;
	}
	else
	{
        fp->Read( &xsize , sizeof(int) , ENDIAN_FOUR_BYTE );
        fp->Read( &ysize , sizeof(int) , ENDIAN_FOUR_BYTE );
	}

	nOffset += 8;

#ifdef _USAGE_TOOL_
	m_pAttrManager->MakeAttributeMap( xsize>>1 , ysize>>1 );

	if( nVersion == 0 ) // 이전 맵이면
        pBuffer = m_pAttrManager->GetAttrBuffer();
	else
	{
		pBuffer = new BYTE[ xsize * ysize ];
	}
#else	// 게임일때

	m_nAttrXSize = xsize;
	m_nAttrYSize = ysize;

	if( m_pAttrMap )
	{
		delete [] m_pAttrMap;
		m_pAttrMap = NULL;
	}

	m_pAttrMap = new BYTE[ xsize * ysize ];
	pBuffer = m_pAttrMap;

#endif

	fp->Read( pBuffer , xsize*ysize );

#ifdef _USAGE_TOOL_

	if( nVersion > 0 ) // 맵툴에 맞게끔 속성 변환
	{
		m_pAttrManager->ChangeGameBufToToolBuf( pBuffer );
		delete [] pBuffer;
	}
#endif

	nOffset += (xsize * ysize);
	return nOffset;
}

int		CBSMapCore::LoadAttributeMapEx(BStream *fp)
{
	int nVersion = 0;
	int nOffset = 0;
	int xsize,ysize;
	BYTE *pBuffer;

	fp->Read( &nVersion , sizeof(int) , ENDIAN_FOUR_BYTE );

	if( nVersion >= 80 ) // 이전 맵
	{
		xsize = nVersion;
		fp->Read( &ysize , sizeof(int) , ENDIAN_FOUR_BYTE );
		nVersion = 0;
	}
	else
	{
		fp->Read( &xsize , sizeof(int) , ENDIAN_FOUR_BYTE );
		fp->Read( &ysize , sizeof(int) , ENDIAN_FOUR_BYTE );
	}

	nOffset += 8;

#ifndef _USAGE_TOOL_ // 게임일때

	CreateAttrExBuffer( xsize , ysize );
		
	pBuffer = m_pAttrMapEx;
	fp->Read( pBuffer , xsize*ysize );

#else
	fp->Seek( xsize * ysize , BStream::fromNow );
#endif

	nOffset += (xsize * ysize);
	return nOffset;
}


struct WATER_VERTEX
{
    D3DXVECTOR3 pos;
	D3DCOLOR color;
};

int		CBSMapCore::LoadWaterLine(BStream *fp)
{
	int nOffset = 0;
	int nChunkCnt;

	fp->Read( &nChunkCnt , sizeof(int) , ENDIAN_FOUR_BYTE );

	// takogura: prefix bug 5833: using nChunkCnt uninitialized -> OK
	for( int i = 0 ; i < nChunkCnt ; ++i )
	{
	
#ifdef _USAGE_TOOL_
		int nSize;
		if( i == 0 ) // 0번째 블럭인 경우 물 라인.
		{
			fp->Read( &nSize, sizeof(int) , ENDIAN_FOUR_BYTE );

			BYTE*	pBuffer;

			pBuffer = new BYTE[ nSize + sizeof(int)  ];
			fp->Read( pBuffer+4 , nSize );

			memcpy( pBuffer , &nSize , sizeof(int));

			CMemFile memFile;
			memFile.Attach( pBuffer , nSize + sizeof(int) );

			// 
			nOffset = m_pLineManager->LoadLineListForTool( &memFile );

			delete [] pBuffer;
		}

#else	
		// Fc 게임 쪽

		if( i == 0 ) // 
		{
			m_nWaterCount++;
			/*
			int nSize;
			fp->Read( &nSize, sizeof(int) , ENDIAN_FOUR_BYTE );

			
			BYTE*	pBuffer;

			pBuffer = new BYTE[ nSize  ];
			fp->Read( pBuffer , nSize );

#ifdef _XBOX	// 어차피 임시이므로 Xenon 에서 Endian Swap 해서 읽는다.
			for(int n=0; n<nSize/4; n++) {
				BYTE *ptr = pBuffer + (n * 4);
				std::reverse(ptr, ptr+4);
			}
#endif


			int nNumWaterWay = *(DWORD*)(pBuffer+4);
			WATER_VERTEX *pWaterVertices = (WATER_VERTEX*)(pBuffer+8);

			m_nWaterCount = nNumWaterWay;

			if( m_pWaterHeight )
			{
				delete [] m_pWaterHeight;
				m_pWaterHeight = NULL;
			}

			//m_pWaterHeight = new float[nSize];


			D3DXVECTOR3 vecSide;

			for(int i = 0; i < nNumWaterWay; i++) 
			{
 
				int nIndex1 = std::max(0, i-1);
				int nIndex2 = std::min(i, nNumWaterWay-2);

				D3DXVECTOR3 vNext1 = pWaterVertices[nIndex1+1].pos - pWaterVertices[nIndex1].pos ;
				D3DXVECTOR3 vNext2 = pWaterVertices[nIndex2+1].pos - pWaterVertices[nIndex2].pos ;
		

			}                       
			// 

			delete [] pBuffer;
			*/
		}
#endif
		/*
		if( i == 1 ) // 두번째 블럭인 경우..
		{
		}
		*/
	}

	return nOffset;
}

int		CBSMapCore::LoadWaterLineEx(BStream *fp)
{
	return 0;
}

int		CBSMapCore::LoadPath(BStream *fp)
{
	int nOffset = 0;
	int nChunkCnt;
	int nNextPos;
	int nDataCnt;
	int nBlockSize;

	fp->Read( &nChunkCnt , sizeof(int) , ENDIAN_FOUR_BYTE );
	nOffset += sizeof(int);

	// takogura: prefix bug 5834: using nChunkCnt uninitialized -> OK
	for( int cn = 0 ; cn < nChunkCnt ; ++cn )
	{
		if( cn == 0 )	// 첫번째 묶
		{
			PATH_PACK temp;

			// VECTOR3 값들이 있는 위치.
			// 1) INFO
			// 2) DATA..
			fp->Read( &nNextPos , sizeof(int) , ENDIAN_FOUR_BYTE );
			nOffset += sizeof(int);

			// temp 블럭의 크기.
			fp->Read( &nBlockSize , sizeof(int) , ENDIAN_FOUR_BYTE );
			nOffset += sizeof(int);

			fp->Read( &nDataCnt , sizeof(int) , ENDIAN_FOUR_BYTE );
			nOffset += sizeof(int);

			for( int i = 0 ; i < nDataCnt ; ++i )
			{
				fp->Read( &temp.pathInfo , nBlockSize-CAPTION_MAX , ENDIAN_FOUR_BYTE );
				fp->Read( &temp.pathInfo.szCaption, CAPTION_MAX );
				nOffset += nBlockSize;

#ifdef _USAGE_TOOL_
				m_pPathManager->CreatePathByInfo( temp.pathInfo );
#else
				m_PathList.push_back( temp );
#endif
			}

			fp->Seek( nNextPos , BStream::fromBegin );

			for( int i = 0 ; i < nDataCnt ; ++i )
			{
				int nDotCnt;
				COLORVERTEX	vecTemp;

				fp->Read( &nDotCnt, sizeof(int), ENDIAN_FOUR_BYTE );
				nOffset += sizeof(int);

				for( int n = 0; n < nDotCnt ; ++n )
				{
					fp->Read( &vecTemp, sizeof(COLORVERTEX), ENDIAN_FOUR_BYTE );
					nOffset += sizeof(COLORVERTEX);
#ifdef _USAGE_TOOL_
					m_pPathManager->GetPack(i)->AddLineToTail( vecTemp.Vertex, true );
#else
                    m_PathList[i].pathData.vecPosPool.push_back( vecTemp.Vertex );
#endif
				}
			}
		}
	}

	return nOffset;
}

int		CBSMapCore::LoadNavList(BStream *fp)
{
	int nOffset = 0;
	int nSize;

	m_NavList.clear();
	m_NavIndexList.clear();

	fp->Read( &nSize , sizeof(int), ENDIAN_FOUR_BYTE );
	nOffset += sizeof(int);

	if( nSize > 0 )
	{
		m_NavList.resize( nSize );
	
        fp->Read( &m_NavList[0], nSize * sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
		nOffset += ( nSize * sizeof(D3DXVECTOR3) );
	}

	fp->Read( &nSize , sizeof(int), ENDIAN_FOUR_BYTE );
	nOffset += sizeof(int);

	if( nSize > 0 )
	{
		m_NavIndexList.resize( nSize );

		fp->Read( &m_NavIndexList[0], nSize * sizeof(int), ENDIAN_FOUR_BYTE );
		nOffset += (nSize * sizeof(int));
	}

	return nOffset;
}

int		CBSMapCore::LoadNavEventList(BStream *fp)
{
	int nOffset = 0;
	int nSize;
	int nIdxCnt;

	ClearNavEventIndexBuffer();

	fp->Read( &nSize , sizeof(int), ENDIAN_FOUR_BYTE );
	nOffset += sizeof(int);

	if( nSize > 0 )
	{
		CreateNavEventIndexBuffer( nSize );

		for( int i = 0 ; i < nSize ; ++i )
		{
			fp->Read( &nIdxCnt, sizeof(int), ENDIAN_FOUR_BYTE );

			m_pNavEventIndexList[i].clear();

			if( nIdxCnt > 0 )
			{
                m_pNavEventIndexList[i].resize( nIdxCnt );

                fp->Read( &m_pNavEventIndexList[i][0], nIdxCnt * sizeof(int), ENDIAN_FOUR_BYTE );
                nOffset += ( nIdxCnt * sizeof(int));
			}
		}
	}

	return nOffset;
}

int		CBSMapCore::LoadColList(BStream *fp)
{
	int nOffset = 0;
	int nSize;

	// Main 읽기
	{
		m_ColList.clear();
		m_ColIndexList.clear();

		fp->Read( &nSize , sizeof(int), ENDIAN_FOUR_BYTE );
		nOffset += sizeof(int);

		if( nSize > 0 )
		{
			m_ColList.resize( nSize );

			fp->Read( &m_ColList[0], nSize * sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
			nOffset += ( nSize * sizeof(D3DXVECTOR3) );
		}

		fp->Read( &nSize , sizeof(int), ENDIAN_FOUR_BYTE );
		nOffset += sizeof(int);

		if( nSize > 0 )
		{
			m_ColIndexList.resize( nSize );

			fp->Read( &m_ColIndexList[0], nSize * sizeof(int), ENDIAN_FOUR_BYTE );
			nOffset += (nSize * sizeof(int));
		}

		// return nOffset;
	}

	// 두번째 리스트부터.
	{
		int nIdxCnt;

		ClearColEventIndexBuffer();

		fp->Read( &nSize , sizeof(int), ENDIAN_FOUR_BYTE );
		nOffset += sizeof(int);

		if( nSize > 0 )
		{
			CreateColEventIndexBuffer( nSize );

			for( int i = 0 ; i < nSize ; ++i )
			{
				fp->Read( &nIdxCnt, sizeof(int), ENDIAN_FOUR_BYTE );

				m_pColEventIndexList[i].clear();

				if( nIdxCnt > 0 )
				{
					m_pColEventIndexList[i].resize( nIdxCnt );

					fp->Read( &m_pColEventIndexList[i][0], nIdxCnt * sizeof(int), ENDIAN_FOUR_BYTE );
					nOffset += ( nIdxCnt * sizeof(int));
				}
			}
		}
	}

	return nOffset;
}








MapFilePropZoneManager::MapFilePropZoneManager()
{
	m_nZoneSize = PROP_ZONE_SIZE;
	m_nXSize = m_nYSize = 0;
	m_pZoneData = NULL;
}

MapFilePropZoneManager::~MapFilePropZoneManager()
{
	if( m_pZoneData )
	{
        delete[] m_pZoneData;
		m_pZoneData = NULL;
	}
}

void MapFilePropZoneManager::Initialize( int nMapXSize, int nMapYSize )
{
	m_nXSize = nMapXSize * 200 / PROP_ZONE_SIZE;
	if( ((nMapXSize * 200) % PROP_ZONE_SIZE) > 0 )		++m_nXSize;

	m_nYSize = nMapYSize * 200 / PROP_ZONE_SIZE;
	if( ((nMapYSize * 200) % PROP_ZONE_SIZE) > 0 )		++m_nYSize;

	m_pZoneData = new MAPFILEPROPZONEDATA[ m_nXSize * m_nYSize ];
}

void MapFilePropZoneManager::AddProp( int nEngineIndex )
{
	const D3DXMATRIX* pMat = g_BsKernel.GetEngineObjectPtr( nEngineIndex )->GetObjectMatrix();

DebugString( "Prop pos %d, %f, %f\n", nEngineIndex, pMat->_41, pMat->_43 );

	int nCurX = ((int)pMat->_41) / PROP_ZONE_SIZE;
	int nCurY = ((int)pMat->_43) / PROP_ZONE_SIZE;

	m_pZoneData[ nCurX + nCurY * m_nXSize ].vecObjIndex.push_back( nEngineIndex );
}

void MapFilePropZoneManager::CaculateBoundingBox()
{
	int nGroupCnt = m_nXSize * m_nYSize;
	for( int i=0; i<nGroupCnt; i++ )
	{
		int nCnt = m_pZoneData[i].vecObjIndex.size();
		if(nCnt == 0)
		{
			m_pZoneData[i].bProp = false;
			continue;
		}

		AABB groupAABB;
		groupAABB.Reset();

		for( int j = 0; j < nCnt; j++ )
		{
			AABB CurBox = *((AABB *)g_BsKernel.SendMessage( m_pZoneData[i].vecObjIndex[j], BS_GET_BOUNDING_BOX ));
			const D3DXMATRIX* pMat = g_BsKernel.GetEngineObjectPtr( m_pZoneData[i].vecObjIndex[j] )->GetObjectMatrix();

			Box3 B;

			B.E[0] = CurBox.GetExtent().x;
			B.E[1] = CurBox.GetExtent().y;
			B.E[2] = CurBox.GetExtent().z; 

			B.C = CurBox.GetCenter();
			D3DXVec3TransformCoord( (D3DXVECTOR3*)(&B.C), (D3DXVECTOR3*)&B.C, pMat );
			memcpy(&B.A[0], &(pMat->_11), sizeof(BSVECTOR));
			memcpy(&B.A[1], &(pMat->_21), sizeof(BSVECTOR));
			memcpy(&B.A[2], &(pMat->_31), sizeof(BSVECTOR));
			B.compute_vertices();

            groupAABB.Merge(B);

		}
		m_pZoneData[i].Box.C = groupAABB.GetCenter();

		m_pZoneData[i].Box.A[0]=BSVECTOR(1.f,0.f,0.f);
		m_pZoneData[i].Box.A[1]=BSVECTOR(0.f,1.f,0.f);
		m_pZoneData[i].Box.A[2]=BSVECTOR(0.f,0.f,1.f);

		m_pZoneData[i].Box.E[0]=groupAABB.GetExtent().x;
		m_pZoneData[i].Box.E[1]=groupAABB.GetExtent().y;
		m_pZoneData[i].Box.E[2]=groupAABB.GetExtent().z;
		m_pZoneData[i].Box.compute_vertices();
		
		m_pZoneData[i].bProp = true;
	}

}

