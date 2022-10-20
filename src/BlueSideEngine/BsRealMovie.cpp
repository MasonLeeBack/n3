#include "stdafx.h"
// #include "BsRealMovieFile.h"
#include "BsRealMovieObject.h"
#include "BsrealMovie.h"
#include "BsRealMovieMsg.h"
#include "BsKernel.h"
#include "BsRealMovieConst.h"
#include "BsRealMoviePackage.h"
#include <io.h>

#include "BsUtil.h"



//------------------------------------------------------------------------------------------------------------------------




// CBsFileBALM CBsRealMovie::st_BALM;
extern CBsFileBALM st_BALM;

// int nDebug = 0;

float	g_fRMFps = 40.f;

const char *g_szMainCamName = "MainCamera";

#ifndef _XBOX

static int User_ftol(float f) 
{ 
#ifndef _WIN64
	int rvalue; 
	__asm fld        f 
	__asm fistp      rvalue
	__asm mov		 eax,rvalue;
#endif
	// return rvalue; 
	return long(f);
} 

#endif

static int		CalcBitCnt(int nValue)
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

FILE*	g_pDebugFp = NULL;
FILE*	g_pDebugFp2 = NULL;

void	CreateDebugFile(int nIndex);
void	CloseDebugFile(int nIndex);

char g_szDebugStr1[256];
char g_szDebugStr2[256];
void	OuputUserDebugString1(char *szTemp);
void	OuputUserDebugString2(char *szTemp);

void	CreateDebugFile(int nIndex)
{
    // 디버깅을 위해.
	if( nIndex == 0 )
		g_pDebugFp = fopen( "e:\\RMDebug1.txt" , "wt" );
	else
		g_pDebugFp2 = fopen( "e:\\RMDebug2.txt" , "wt" );

}

void	CloseDebugFile(int nIndex)
{
	if( nIndex == 0 )
        fclose( g_pDebugFp );
	else
		fclose( g_pDebugFp2 );
}

void	OuputUserDebugString1(char *szTemp)
{
	if( g_pDebugFp )
        fputs( szTemp, g_pDebugFp );
}

void	OuputUserDebugString2(char *szTemp)
{
	if( g_pDebugFp2 )
        fputs( szTemp, g_pDebugFp2 );
}


//------------------------------------------------------------------------------------------------------------------------

bool CBsRealMovie::m_bPause = false;
LARGE_INTEGER CBsRealMovie::m_liSaveTime;

CBsRealMovie::CBsRealMovie() 
{ 
	m_bFadeOut=0;
	m_pDefCam = NULL;
	m_nbUseRMCam = 0;
	m_fTerminateTime = -1.f;
	m_pStaticCam = NULL;
	g_fRMFps = 40.f;
	m_ppObjTempBuf = NULL;
	m_nTempBufSize = 0;


	ChdirStart();
	st_BALM.FLoad_text( g_BsKernel.GetFullName( "RealMovie\\default43.balm" ) ); // 버퍼 메모리 입니다 (free 와 상관 없음) = 공용으로 사용
	ChdirEnd();

	RemoveAllBuffer();

#ifndef _XBOX
	OnInitCommander();
#endif

#ifdef _USAGE_TOOL_
	m_pErrMsgManager = new CBsErrorMsgManager;
	m_bIsDummyLoad = FALSE;
#endif
}

//------------------------------------------------------------------------------------------------------------------------

#define SAVE_MASK (0x00000FFF|RMID_WAVE_EX|RMID_PACKAGE|RMID_CAMERA_EX) // |RMID_EFFECT|RMID_OBJ_EX|RMID_OBJ_EX2|RMID_BONE_EX)

void CBsRealMovie::Save(BStream *pStream)
{
	int i;
	int nOffsetTbl[32] = { 0,};
	char szType[4] = "RMF";
	int nVersion = RM_FILE_VER;

	// 1) 
	pStream->Write( szType , 4 );

	pStream->Write( &nVersion, sizeof(int), ENDIAN_FOUR_BYTE);

	// Offset Table Count
	int nTblMask = SAVE_MASK;
	int nTblCnt = 0;
	int nEmptyValue = 0;

	nTblCnt = CalcBitCnt( nTblMask );

	// 3)
	pStream->Write( &nTblMask , sizeof(int), ENDIAN_FOUR_BYTE);

	// 4)
	pStream->Write( &nTblCnt , sizeof(int), ENDIAN_FOUR_BYTE);

	DWORD dwTempPos = (DWORD)pStream->Tell();

	// 5)
	for( i = 0 ; i < nTblCnt ; i++ )
	{
		pStream->Write( &nEmptyValue , sizeof(int), ENDIAN_FOUR_BYTE );
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
			case RMID_SETINFO :
				nOffsetTbl[i] = (int)pStream->Tell();
				m_BGInfo.Save( pStream );
				break;
			case RMID_OBJECT :
				nOffsetTbl[i] = (int)pStream->Tell();
				SaveObjects( pStream );
				break;
			case RMID_WAVE :
				nOffsetTbl[i] = (int)pStream->Tell();
				SaveWaves( pStream );
				break;
			case RMID_CAMERA :
				nOffsetTbl[i] = (int)pStream->Tell();
				SaveCameras( pStream );
				break;
			case RMID_EFFECT :
				nOffsetTbl[i] = (int)pStream->Tell();
				SaveEffects( pStream );
				break;
			case RMID_OBJ_EX :
				nOffsetTbl[i] = (int)pStream->Tell();
				SaveObjectsEx( pStream );
				break;
			case RMID_OBJ_EX2 :
				nOffsetTbl[i] = (int)pStream->Tell();
				SaveObjectsEx2( pStream );
				break;
			case RMID_BONE_EX :
				nOffsetTbl[i] = (int)pStream->Tell();
				SaveBoneEx( pStream );
				break;
			case RMID_EFFECT_EX :
				nOffsetTbl[i] = (int)pStream->Tell();
				SaveEffectsEx( pStream );
				break;
			case RMID_AREA :
				nOffsetTbl[i] = (int)pStream->Tell();
				SaveArea( pStream );
				break;
			case RMID_LIGHT :
				nOffsetTbl[i] = (int)pStream->Tell();
				SaveLight( pStream );
				break;
			case RMID_POINT_LIGHT :
				nOffsetTbl[i] = (int)pStream->Tell();
				SavePointLight( pStream );
				break;
			case RMID_WAVE_EX :
				nOffsetTbl[i] = (int)pStream->Tell();
				SaveWavesEx( pStream );
				break;
			case RMID_PACKAGE :
				nOffsetTbl[i] = (int)pStream->Tell();
				SavePackage( pStream );
				break;
			case RMID_CAMERA_EX :
				nOffsetTbl[i] = (int)pStream->Tell();
				SaveCamerasEx( pStream );
				break;

			}

			++nCnt;
			if( nCnt == nTblCnt )
				break;
		}
	}

	pStream->Seek( dwTempPos, BStream::fromBegin );

	nCnt = 0;
	for( i = 0 ; i < 32 ; ++i )
	{
		if( nOffsetTbl[i] )
		{
			pStream->Write( &nOffsetTbl[i] , sizeof(int), ENDIAN_FOUR_BYTE );

			++nCnt;
			if( nCnt == nTblCnt )
				break;
		}
	}
}

void CBsRealMovie::SaveObjects(BStream *pStream)
{
	iteratorRMObject itObj = m_mapObject.begin();
	iteratorRMObject itObjEnd = m_mapObject.end();

	int nCnt = (int)m_mapObject.size();
	pStream->Write( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	for( ; itObj != itObjEnd ; ++itObj )
	{
		itObj->second->Save( pStream );
	}
}

void CBsRealMovie::SaveObjectsEx(BStream *pStream)
{
	iteratorRMObject itObj = m_mapObject.begin();
	iteratorRMObject itObjEnd = m_mapObject.end();

	int nCnt = (int)m_mapObject.size();
	pStream->Write( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	for( ; itObj != itObjEnd ; ++itObj )
	{
		itObj->second->SaveEx( pStream );
	}
}

void CBsRealMovie::SaveObjectsEx2(BStream *pStream)
{
	iteratorRMObject itObj = m_mapObject.begin();
	iteratorRMObject itObjEnd = m_mapObject.end();

	int nCnt = (int)m_mapObject.size();
	pStream->Write( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	for( ; itObj != itObjEnd ; ++itObj )
	{
		itObj->second->SaveEx2( pStream );
	}
}



void CBsRealMovie::SaveCameras(BStream *pStream)
{
	iteratorRMCamera	itCam = m_mapCamera.begin();
	iteratorRMCamera	itCamEnd = m_mapCamera.end();

	int nCnt = (int)m_mapCamera.size();
	pStream->Write( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	for( ; itCam != itCamEnd ; ++itCam )
	{
		itCam->second->Save( pStream );
	}
}

void CBsRealMovie::SaveCamerasEx(BStream *pStream)
{
	int nCode = 1;
	iteratorRMCamera	itCam = m_mapCamera.begin();
	iteratorRMCamera	itCamEnd = m_mapCamera.end();

	pStream->Write( &nCode, sizeof(int), ENDIAN_FOUR_BYTE );
	int nCnt = (int)m_mapCamera.size();
	pStream->Write( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	for( ; itCam != itCamEnd ; ++itCam )
	{
		itCam->second->SaveEx( pStream );
	}
}

void CBsRealMovie::SaveWaves(BStream *pStream)
{
	iteratorRMWave	itWav = m_mapWave.begin();
	iteratorRMWave	itWavEnd = m_mapWave.end();

	int nCnt = (int)m_mapWave.size();
	pStream->Write( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	for( ; itWav != itWavEnd ; ++itWav )
	{
		itWav->second->Save( pStream );
	}
}

void CBsRealMovie::SaveWavesEx(BStream *pStream)
{
	iteratorRMWave	itWav = m_mapWave.begin();
	iteratorRMWave	itWavEnd = m_mapWave.end();

	int nCnt = (int)m_mapWave.size();
	pStream->Write( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	for( ; itWav != itWavEnd ; ++itWav )
	{
		itWav->second->SaveEx( pStream );
	}
}

void CBsRealMovie::SaveEffects(BStream *pStream)
{
	iteratorRMEffect	itEff = m_mapEffect.begin();
	iteratorRMEffect	itEffEnd = m_mapEffect.end();

	int nCnt = (int)m_mapEffect.size();
	pStream->Write( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	for( ; itEff != itEffEnd ; ++itEff )
	{
		itEff->second->Save( pStream );
	}
}

void CBsRealMovie::SaveBoneEx(BStream *pStream)
{
	iteratorRMObject itObj = m_mapObject.begin();
	iteratorRMObject itObjEnd = m_mapObject.end();

	int nCnt = (int)m_mapObject.size();
	pStream->Write( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	for( ; itObj != itObjEnd ; ++itObj )
	{
		itObj->second->SaveBoneEx( pStream );
	}
}

void CBsRealMovie::SaveEffectsEx(BStream *pStream)
{
	iteratorRMEffect	itEff = m_mapEffect.begin();
	iteratorRMEffect	itEffEnd = m_mapEffect.end();

	int nCnt = (int)m_mapEffect.size();
	pStream->Write( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	for( ; itEff != itEffEnd ; ++itEff )
	{
		itEff->second->SaveEx( pStream );
	}
}

void CBsRealMovie::SaveArea(BStream *pStream)
{
	int nCode = 1;
	int nLen;
	int nBufSize;
	int nExBufSize;
	int nCnt = (int)m_vecAreaSetPool.size();
    
	pStream->Write( &nCode , sizeof(int), ENDIAN_FOUR_BYTE );	// 청크 코드

	nBufSize = sizeof( stAreaSet::stCoord );	// AreaSet 버퍼 크기 저장.
	nExBufSize = sizeof( stAreaSet::stAreaEx ) - AREA_STR_MAX;	// AreaSet 버퍼 크기 저장.

    pStream->Write( &nBufSize , sizeof(int), ENDIAN_FOUR_BYTE );	// Area 버퍼 크기
	pStream->Write( &nExBufSize , sizeof(int), ENDIAN_FOUR_BYTE );	// Area Ex 버퍼 크기

	pStream->Write( &nCnt , sizeof(int), ENDIAN_FOUR_BYTE );	// Area 개수

	vecAreaSet::iterator it = m_vecAreaSetPool.begin();
	vecAreaSet::iterator itEnd = m_vecAreaSetPool.end();

	for( ; it != itEnd ; ++it )
	{
		pStream->Write( &(it->AreaCoord), nBufSize, ENDIAN_FOUR_BYTE );	// 첫번째 버퍼
		pStream->Write( &(it->AreaEx), nExBufSize, ENDIAN_FOUR_BYTE );	// 두번째 버퍼

		// 확장 버퍼 저장.
		nLen = strlen( it->AreaEx.szAreaName );
		pStream->Write( &nLen, sizeof( int), ENDIAN_FOUR_BYTE );

		if( nLen )
			pStream->Write( it->AreaEx.szAreaName, nLen, 0 );
	}
}

void CBsRealMovie::SaveLight(BStream *pStream)
{
	int nCode = 1;
	int nTextBufSize;
	int nDataBufSize;
	int nCnt = (int)m_vecLightSetPool.size();

	pStream->Write( &nCode , sizeof(int), ENDIAN_FOUR_BYTE );	// 청크 코드

	nTextBufSize = sizeof( stLightPack::stName );	// LightSet Text 버퍼 크기 저장.
	nDataBufSize = LIGHT_DATA_BLOCK_SIZE; // sizeof( stLightPack::stLightData );	// LightSet Data 버퍼 크기 저장.

	pStream->Write( &nTextBufSize , sizeof(int), ENDIAN_FOUR_BYTE );	// Text 버퍼 크기
	pStream->Write( &nDataBufSize , sizeof(int), ENDIAN_FOUR_BYTE );	// Data 버퍼 크기

	pStream->Write( &nCnt , sizeof(int), ENDIAN_FOUR_BYTE );	// Area 개수

	vecLightSet::iterator it = m_vecLightSetPool.begin();
	vecLightSet::iterator itEnd = m_vecLightSetPool.end();

	for( ; it != itEnd ; ++it )
	{
		it->Save( pStream, nTextBufSize, nDataBufSize );
	}
}

void CBsRealMovie::SavePointLight(BStream *pStream)
{
	int nCode = 1;
	int nCnt = (int)m_vecPointLightSetPool.size();

	pStream->Write( &nCode , sizeof(int), ENDIAN_FOUR_BYTE );	// 청크 코드
	pStream->Write( &nCnt , sizeof(int), ENDIAN_FOUR_BYTE );	// 포인트 라이트 개수

	vecPointLightSet::iterator it;
	vecPointLightSet::iterator itEnd = m_vecPointLightSetPool.end();

	for( it = m_vecPointLightSetPool.begin() ; it != itEnd ; ++it )
	{
		it->Save( pStream );
	}
}

void CBsRealMovie::SavePackage(BStream* pStream)
{
	int nCode = 1;
	int nCnt = (int)m_vecPackagePool.size();
	
	pStream->Write( &nCode, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Write( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	vecPackageSet::iterator it = m_vecPackagePool.begin();
	vecPackageSet::iterator itEnd = m_vecPackagePool.end();

	for( ; it != itEnd ; ++it )
	{
		it->Save( pStream );
	}
}

void CBsRealMovie::Load(BStream *pStream)
{
	int i;
	int nOffsetTbl[32] = { 0,};
	char szType[4]; // = "RMF";
	int nVersion; // = FC_MAP_VER;

	// Time Test
	LARGE_INTEGER liFrequency;
	LARGE_INTEGER liStartTime;
	LARGE_INTEGER liCurTime;
	QueryPerformanceFrequency(&liFrequency);
	QueryPerformanceCounter(&liStartTime);

	Clear();

	memset( nOffsetTbl , 0x00 , sizeof( nOffsetTbl ) );

	// 1) 
	pStream->Read( szType , 4 );

	if( strcmp( szType , "RMF" ) != 0 ) // [PREFIX:nomodify] 2006/2/15 junyash PS#5172 szType is initialized in pStream->Read( szType, 4 );
		return;	// RM 파일이 아님.

	// 2)
	pStream->Read( &nVersion , sizeof(int) , ENDIAN_FOUR_BYTE );

#ifdef WIN32
	//	TRACE("맵파일 버전 : %d \n", nVersion );
#endif

	if( nVersion == RM_FILE_OLD_VER || nVersion != RM_FILE_VER )
		g_fRMFps = 60.f;
	else 
		g_fRMFps = 40.f;

	if( nVersion == RM_FILE_OLD_VER + 1 )
        g_fRMFps = 40.f;


	// Offset Table Count
	int nTblMask = 0x00000000;
	int nTblMaskForLoad = nTblMask;

	int nTblCnt = 1;
	int nTblCntForLoad = nTblCnt;
	int nEmptyValue = 0;

	// 3)
	pStream->Read( &nTblMask , sizeof(int) , ENDIAN_FOUR_BYTE );

	// 4)
	pStream->Read( &nTblCntForLoad , sizeof(int) , ENDIAN_FOUR_BYTE );

	DWORD dwTempPos = (DWORD)pStream->Tell();

	nTblCnt = CalcBitCnt( nTblMask );

	if( nTblCnt != nTblCntForLoad )
	{
#ifdef WIN32
		//		TRACE("뭔가 이상이 있는겨.. TblCnt [%d] , TblCntForLoad [%d]\n", nTblCnt , nTblCntForLoad );
#endif
		return;
	}

	int nTempPos;
	int nCnt = 0;
	int nCurMask = 1;

	// 오프셋들만 따로 저장.
	for( i = 0 ; i < 32 ; i++ , nCurMask <<= 1 )
	{
		if( nTblMask & nCurMask )
		{
			pStream->Read( &nTempPos , sizeof(int) , ENDIAN_FOUR_BYTE );
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
			case RMID_SETINFO :
				pStream->Seek( nOffsetTbl[i] , BStream::fromBegin );
				m_BGInfo.Load( pStream );
				++nCnt;
				break;
			case RMID_OBJECT :
				pStream->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadObjects( pStream );
				++nCnt;
				break;
			case RMID_WAVE :
				pStream->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadWaves( pStream );
				++nCnt;
				break;
			case RMID_CAMERA :
				pStream->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadCameras( pStream , nVersion );
				++nCnt;
				break;
			case RMID_EFFECT :
				pStream->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadEffects( pStream );
				++nCnt;
				break;
			case RMID_OBJ_EX :
				pStream->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadObjectsEx( pStream );
				++nCnt;
				break;
			case RMID_OBJ_EX2 :
				pStream->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadObjectsEx2( pStream );
				++nCnt;
				break;
			case RMID_BONE_EX :
				pStream->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadBoneEx( pStream );
				++nCnt;
				break;
			case RMID_EFFECT_EX :
				pStream->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadEffectsEx( pStream );
				++nCnt;
				break;
			case RMID_AREA :
				pStream->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadArea( pStream );
				++nCnt;
				break;
			case RMID_LIGHT :
				pStream->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadLight( pStream );
				++nCnt;
				break;
			case RMID_POINT_LIGHT :
				pStream->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadPointLight( pStream );
				++nCnt;
				break;
			case RMID_WAVE_EX :
				pStream->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadWavesEx( pStream );
				++nCnt;
				break;
			case RMID_PACKAGE :
				pStream->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadPackage( pStream );
				++nCnt;
				break;
			case RMID_CAMERA_EX :
				pStream->Seek( nOffsetTbl[i] , BStream::fromBegin );
				LoadCamerasEx( pStream , nVersion );
				++nCnt;
				break;
			}

			if( nCnt == nTblCnt )
				break;
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////

	QueryPerformanceCounter(&liCurTime);	
	double dwTime = (double)(( liCurTime.QuadPart - liStartTime.QuadPart ) / ( liFrequency.QuadPart / 1000.f ));
	DebugString("-- Realmovie Load time (%f ms)\n", dwTime);

	///////////////////////////////////////////////////////////////////////////////////////

	// Link Time 재기.
	QueryPerformanceCounter(&liStartTime);

	LinkObjToEffects();
	SetDefaultLighting();

	if( nVersion < RM_FILE_VER )
	{
		CheckObjFolder();
		CheckCamFolder();
		CheckWavFolder();
	}

#ifdef _USAGE_TOOL_
	LinkEffectToPackage();	// Effect 와 Package 연결.
#endif

	LinkEffectToWaveRes();	// Effect 와 Wave Res 연결.
//#endif


	QueryPerformanceCounter(&liCurTime);	
	dwTime = (double)(( liCurTime.QuadPart - liStartTime.QuadPart ) / ( liFrequency.QuadPart / 1000.f ));
	DebugString("-- Realmovie Link time (%f ms)\n\n", dwTime);

	g_fRMFps = 40.f;

	return;
}

void CBsRealMovie::LoadObjects(BStream *pStream)
{
	int i,nCnt,nLen;
	char szTempName[eFileNameLength];

	pStream->Read( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	for( i = 0 ; i < nCnt ; ++i )
	{
		pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );

		if( nLen )
		{
			memset( szTempName, 0, eFileNameLength);
			pStream->Read( szTempName, nLen, 0 );

			CRMObject* pRMObj = InsertEmptyObject( szTempName ); // InsertObject( szTempName );
			pRMObj->Load( pStream );
		}
	}
}

void CBsRealMovie::LoadObjectsEx(BStream *pStream)
{

	// 이미 다 읽어 놓은 상태.
	iteratorRMObject itObj = m_mapObject.begin();
	iteratorRMObject itObjEnd = m_mapObject.end();

	int nCnt;
	pStream->Read( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	if( nCnt != (int)m_mapObject.size() )
	{
		BsAssert( 0 && "CBsRealMovie::LoadObjectsEx 오브젝트 수가 동일하지 않습니다.");
		return;
	}
	
	for( ; itObj != itObjEnd ; ++itObj )
	{
		itObj->second->LoadEx( pStream );
	}
}

void CBsRealMovie::LoadObjectsEx2(BStream *pStream)
{

	// 이미 다 읽어 놓은 상태.
	iteratorRMObject itObj = m_mapObject.begin();
	iteratorRMObject itObjEnd = m_mapObject.end();

	int nCnt;
	pStream->Read( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	if( nCnt != (int)m_mapObject.size() )
	{
		BsAssert( 0 && "CBsRealMovie::LoadObjectsEx 오브젝트 수가 동일하지 않습니다.");
		return;
	}

	for( ; itObj != itObjEnd ; ++itObj )
	{
		itObj->second->LoadEx2( pStream );
	}
}


void CBsRealMovie::LoadCameras(BStream *pStream,int nVer)
{
	int i,nCnt,nLen;
	char szTempName[eFileNameLength];

	pStream->Read( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	for( i = 0 ; i < nCnt ; ++i )
	{
		pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );

		if( nLen )
		{
			memset( szTempName, 0, eFileNameLength);
			pStream->Read( szTempName, nLen, 0 );

			CRMCamera* pCam = InsertCamera( szTempName );
			BsAssert(pCam && "Failed to insert camera in LoadCameras()"); // aleksger: prefix bug 370: Failure to insert will lead to memory leak.
			pCam->Load( pStream , nVer );
		}
	}
}

void	CBsRealMovie::LoadCamerasEx(BStream *pStream,int nVer)
{
	int nCode;
	int nCnt;

	pStream->Read( &nCode, sizeof(int), ENDIAN_FOUR_BYTE );

	for( int code = 0 ; code < nCode ; ++code )
	{
		if( code == 0 )
		{
			pStream->Read( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

			if( nCnt != (int)m_mapCamera.size() )
			{
				return;
			}

			iteratorRMCamera	itCam = m_mapCamera.begin();
			iteratorRMCamera	itCamEnd = m_mapCamera.end();

			for( ; itCam != itCamEnd ; ++itCam )
			{
				itCam->second->LoadEx( pStream );
			}
		}
	}
}

void CBsRealMovie::LoadWaves(BStream *pStream)
{
	int i,nCnt,nLen;
	char szTempName[eFileNameLength];

	pStream->Read( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	for( i = 0 ; i < nCnt ; ++i )
	{
		pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );

		if( nLen )
		{
			memset( szTempName, 0, eFileNameLength);
			pStream->Read( szTempName, nLen, 0 );

			CRMWave* pWave = InsertWave( szTempName );
			BsAssert(pWave && "Failed to insert a wave");// aleksger: prefix bug 370: Possible failure to insert.
			pWave->Load( pStream );
		}
	}
}

void CBsRealMovie::LoadWavesEx(BStream *pStream)
{
	iteratorRMWave	itWav = m_mapWave.begin();
	iteratorRMWave	itWavEnd = m_mapWave.end();

	int nCnt;
	pStream->Read( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	if( nCnt != (int)m_mapWave.size() )
	{
		return;
	}

	for( ; itWav != itWavEnd ; ++itWav )
	{
		itWav->second->LoadEx( pStream );
	}
}

void CBsRealMovie::LoadPackage(BStream* pStream)
{
	int nCode;
	int nCnt;
	CRMPackage tmpPack;

	pStream->Read( &nCode, sizeof(int), ENDIAN_FOUR_BYTE );

	for( int code = 0 ; code < nCode ; ++code )
	{
		if( code == 0 )
		{
			ClearAllPackages();

			pStream->Read( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

			for( int i = 0 ; i < nCnt ; ++i )
			{
				tmpPack.Load( pStream );
				m_vecPackagePool.push_back( tmpPack );
			}
		}
	}
}

void CBsRealMovie::LoadEffects(BStream *pStream)
{
	int i,nCnt,nLen;
	char szTempName[eFileNameLength];

	pStream->Read( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	for( i = 0 ; i < nCnt ; ++i )
	{
		pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );

		if( nLen )
		{
			memset( szTempName, 0, eFileNameLength);
			pStream->Read( szTempName, nLen, 0 );

			CRMEffect* pEffect = InsertEff( szTempName );
			BsAssert(pEffect && "Failed to insert an effect");// aleksger: prefix bug 377: Possible failure to insert.
			pEffect->Load( pStream );
		}
	}
}

void	CBsRealMovie::LoadBoneEx(BStream *pStream)
{
	iteratorRMObject itObj = m_mapObject.begin();
	iteratorRMObject itObjEnd = m_mapObject.end();

	int nCnt;
	pStream->Read( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	if( nCnt != (int)m_mapObject.size() )
	{
		//assert( 0 && "CBsRealMovie::LoadObjectsEx 오브젝트 수가 동일하지 않습니다.");
		return;
	}

	for( ; itObj != itObjEnd ; ++itObj )
	{
		itObj->second->LoadBoneEx( pStream );
	}
}

void CBsRealMovie::LoadEffectsEx(BStream *pStream)
{
	iteratorRMEffect	itEff = m_mapEffect.begin();
	iteratorRMEffect	itEffEnd = m_mapEffect.end();

	int nCnt;

	pStream->Read( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	if( nCnt != (int)m_mapEffect.size() )
	{
		//assert( 0 && "CBsRealMovie::LoadObjectsEx 오브젝트 수가 동일하지 않습니다.");
		return;
	}

	for( ; itEff != itEffEnd ; ++itEff )
	{
		itEff->second->LoadEx( pStream );
	}
}

void	CBsRealMovie::LoadArea(BStream* pStream)
{
	int nCode;
	int nLen;
    int nBufSize;
	int nExBufSize;
	int nAreaCnt;

	pStream->Read( &nCode, sizeof(int), ENDIAN_FOUR_BYTE );

	stAreaSet	tmpArea;
	
	for( int code = 0 ; code < nCode ; ++code )
	{
		if( code == 0 )
		{
			m_vecAreaSetPool.clear();

			pStream->Read( &nBufSize, sizeof(int), ENDIAN_FOUR_BYTE );
			pStream->Read( &nExBufSize, sizeof(int), ENDIAN_FOUR_BYTE );


			pStream->Read( &nAreaCnt, sizeof(int), ENDIAN_FOUR_BYTE );

			for( int i = 0 ; i < nAreaCnt ; ++i )
			{
				pStream->Read( &tmpArea.AreaCoord, nBufSize, ENDIAN_FOUR_BYTE );
				pStream->Read( &tmpArea.AreaEx, nExBufSize, ENDIAN_FOUR_BYTE );

				pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );

				memset( tmpArea.AreaEx.szAreaName, 0, AREA_STR_MAX );

				if( nLen )
				{
                    pStream->Read( tmpArea.AreaEx.szAreaName, nLen, 0 );
				}

				m_vecAreaSetPool.push_back( tmpArea );
			}
		}
	}
}

void CBsRealMovie::LoadLight(BStream* pStream)
{
	int nCode;
	int nTextBufSize;
	int nDataBufSize;
	int nCnt;

	pStream->Read( &nCode, sizeof(int), ENDIAN_FOUR_BYTE );

	stLightPack	tmpLight;

	for( int code = 0 ; code < nCode ; ++code )
	{
		if( code == 0 )
		{
			m_vecLightSetPool.clear();

			pStream->Read( &nTextBufSize, sizeof(int), ENDIAN_FOUR_BYTE );
			pStream->Read( &nDataBufSize, sizeof(int), ENDIAN_FOUR_BYTE );


			pStream->Read( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

			memset( tmpLight.NameBlock.szLightName, 0, LIGHT_STR_MAX );

			for( int i = 0 ; i < nCnt ; ++i )
			{
				tmpLight.Load( pStream, nTextBufSize, nDataBufSize );

				/*
				pStream->Read( &tmpLight.NameBlock, nTextBufSize, 0 );

				if( nDataBufSize == sizeof( stLightPack::stLightData ) ) // 64 - 새로운 CrossVector 용
				{
					pStream->Read( &tmpLight.DataBlock, nDataBufSize, ENDIAN_FOUR_BYTE );
				}
				else 
				{		// 48 - 이전 CrossVector 용
					assert( nDataBufSize == sizeof( stLightPack::stLightData )-16 );
					D3DXVECTOR3 XVector, YVector, ZVector, PosVector;		
					pStream->Read( &XVector, sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
					pStream->Read( &YVector, sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
					pStream->Read( &ZVector, sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
					pStream->Read( &PosVector, sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
					tmpLight.DataBlock.crossLight.m_XVector = XVector;
					tmpLight.DataBlock.crossLight.m_YVector = YVector;
					tmpLight.DataBlock.crossLight.m_ZVector = ZVector;
					tmpLight.DataBlock.crossLight.m_PosVector = PosVector;
				}
				*/

				m_vecLightSetPool.push_back( tmpLight );
			}
		}
	}
}

void CBsRealMovie::LoadPointLight(BStream* pStream)
{
	int nCode;
	int nCnt;

	pStream->Read( &nCode, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Read( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	for( int code = 0 ; code < nCode ; ++code )
	{
		if( code == 0 )
		{
			m_vecPointLightSetPool.clear();
			stPointLightPack tmpPLPack;

			for( int i = 0 ; i < nCnt ; ++i )
			{
				tmpPLPack.Load( pStream );
				m_vecPointLightSetPool.push_back( tmpPLPack );
			}
		}
	}
}

void	CBsRealMovie::LinkObjToEffects(void)
{
	iteratorRMEffect	itEff = m_mapEffect.begin();
	iteratorRMEffect	itEffEnd = m_mapEffect.end();

	for( ; itEff != itEffEnd ; ++itEff )
	{
		if( itEff->second->IsCamType() )
		{
			CRMEffect* pEffect = itEff->second;
			pEffect->SetLinkedObjPtr( (DWORD)(GetCamera( pEffect->GetLinkedObjName() ) ) );
		}
#ifdef _USAGE_TOOL_
		if( itEff->second->IsMovable() )
		{
			CRMEffect* pEffect = itEff->second;
			pEffect->ResetEffectHBoardPos();
		}
#endif
	}
}

void	CBsRealMovie::ClearEffectOwnTable(void)
{
	iteratorRMEffect	itEff = m_mapEffect.begin();
	iteratorRMEffect	itEffEnd = m_mapEffect.end();

	for( ; itEff != itEffEnd ; ++itEff )
	{
        itEff->second->m_pOwnerPackage = NULL;
	}
}

void	CBsRealMovie::LinkEffectToPackage(void)
{
	vecPackageSet::iterator it = m_vecPackagePool.begin();
	vecPackageSet::iterator itEnd = m_vecPackagePool.end();

	for( ; it != itEnd ; ++it )
	{
		CRMPackage* pPack = &(*it);

		for( int i = 0 ; i < CRMPackage::eItemMax ; ++i )
		{
			char* pItemName = pPack->GetItemName( i );

			if( pItemName[0] != NULL )	// if( pPack->GetItemUse( i ) )
			{
				CRMEffect* pEffect = GetEffect( pItemName );
				
				if( pEffect )
				{
					pEffect->m_pOwnerPackage = pPack; 
				}
			}
		}
	}
}

void	CBsRealMovie::CheckObjFolder(void)
{
	iteratorRMObject itObj = m_mapObject.begin();
	iteratorRMObject itObjEnd = m_mapObject.end();

	for( ; itObj != itObjEnd ; ++itObj )
	{
		itObj->second->ChangeDataFolder();
	}
}

void	CBsRealMovie::CheckCamFolder(void)
{
	iteratorRMCamera	itCam = m_mapCamera.begin();
	iteratorRMCamera	itCamEnd = m_mapCamera.end();
	char szTemp[_countof(itCam->second->m_szFilePath)];

	for( ; itCam != itCamEnd ; ++itCam )
	{
		if( itCam->second->HasBCFile() )
		{
			sprintf(szTemp, "RealMovie\\%s", itCam->second->m_szFilePath); //aleksger - since the variables are public, using that to clean up code.
			strcpy( itCam->second->m_szFilePath, szTemp);
		}
	}
}

void	CBsRealMovie::CheckWavFolder(void)
{
	iteratorRMWave	itWav = m_mapWave.begin();
	iteratorRMWave	itWavEnd = m_mapWave.end();
	char szTemp[_countof(itWav->second->m_szFilePath)];

	for( ; itWav != itWavEnd ; ++itWav )
	{
		sprintf(szTemp, "RealMovie\\%s", itWav->second->m_szFilePath); //aleksger - since the variables are public, using that to clean up code.
		strcpy( itWav->second->m_szFilePath, szTemp);
	}
}

void	CBsRealMovie::LinkEffectToWaveRes(void)
{
	iteratorRMEffect	itEff = m_mapEffect.begin();
	iteratorRMEffect	itEffEnd = m_mapEffect.end();

	for( ; itEff != itEffEnd ; ++itEff )
	{
		if( itEff->second->IsSndType() )
		{
			CRMEffect* pEffect = itEff->second;
#ifdef _DEBUG
			DWORD dwWaveRes = (DWORD)GetWave( pEffect->GetFileName() );
			if( dwWaveRes != 0 )
			{
				pEffect->SetLinkedObjPtr( dwWaveRes );
			}
			/*
			assert( dwWaveRes );
			pEffect->SetLinkedObjPtr( dwWaveRes );
			*/
#else
			pEffect->SetLinkedObjPtr( (DWORD)GetWave( pEffect->GetFileName() ) );
#endif
		}
	}

	iteratorRMWave	itWav = m_mapWave.begin();
	iteratorRMWave	itWavEnd = m_mapWave.end();

	for( ; itWav != itWavEnd ; ++itWav )
	{
		itWav->second->SetRMObjPtr( 
			(DWORD)GetObject( itWav->second->GetLinkedObjName() ) );
	}
}


void CBsRealMovie::SaveOld(BStream *pStream)
{
	int nCnt;
	m_nVersion = 100;
	pStream->Write(&m_nVersion, sizeof(int), ENDIAN_FOUR_BYTE);
	
	// pStream->Write(&m_szPathName, 128, 0);

	m_BGInfo.Save( pStream );

	iteratorRMObject itObj = m_mapObject.begin();
	iteratorRMObject itObjEnd = m_mapObject.end();

	nCnt = (int)m_mapObject.size();
	pStream->Write( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	for( ; itObj != itObjEnd ; ++itObj )
	{
		itObj->second->Save( pStream );
	}

	iteratorRMWave	itWav = m_mapWave.begin();
	iteratorRMWave	itWavEnd = m_mapWave.end();

	nCnt = (int)m_mapWave.size();
	pStream->Write( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	for( ; itWav != itWavEnd ; ++itWav )
	{
		itWav->second->Save( pStream );
	}

	iteratorRMCamera	itCam = m_mapCamera.begin();
	iteratorRMCamera	itCamEnd = m_mapCamera.end();

    nCnt = (int)m_mapCamera.size();
	pStream->Write( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	for( ; itCam != itCamEnd ; ++itCam )
	{
		itCam->second->Save( pStream );
	}

	iteratorRMEffect	itEff = m_mapEffect.begin();
	iteratorRMEffect	itEffEnd = m_mapEffect.end();

	nCnt = (int)m_mapEffect.size();
	pStream->Write( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	for( ; itEff != itEffEnd ; ++itEff )
	{
		itEff->second->Save( pStream );
	}
}

void CBsRealMovie::LoadOld(BStream *pStream)
{
	Clear();

	char szTempName[eFileNameLength];
	int i,nCnt,nLen;
	int nVer;
	pStream->Read(&nVer, 4, ENDIAN_FOUR_BYTE);


	m_BGInfo.Load( pStream );
	// pStream->Read(&m_szPathName, 128, 0);

	pStream->Read( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	for( i = 0 ; i < nCnt ; ++i )
	{
		pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );

		if( nLen )
		{
			memset( szTempName, 0, eFileNameLength);
			pStream->Read( szTempName, nLen, 0 );

			CRMObject* pRMObj = InsertObject( szTempName );
			pRMObj->Load( pStream );
		}
	}

	pStream->Read( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	for( i = 0 ; i < nCnt ; ++i )
	{
		pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );

		if( nLen )
		{
			memset( szTempName, 0, eFileNameLength);
			pStream->Read( szTempName, nLen, 0 );

			CRMWave* pWave = InsertWave( szTempName );
			BsAssert(pWave && "Failed to insert a wave");// aleksger: prefix bug 370: Possible failure to insert.
			pWave->Load( pStream );
		}
	}

	pStream->Read( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	for( i = 0 ; i < nCnt ; ++i )
	{
		pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );

		if( nLen )
		{
			memset( szTempName, 0, eFileNameLength);
			pStream->Read( szTempName, nLen, 0 );

			CRMCamera* pCam = InsertCamera( szTempName );
			BsAssert(pCam && "Failed to insert camera in LoadOld()"); // aleksger: prefix bug 370: Failure to insert will lead to memory leak.
			pCam->Load( pStream , 100 );
		}
	}

	pStream->Read( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	for( i = 0 ; i < nCnt ; ++i )
	{
		pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );

		if( nLen )
		{
			memset( szTempName, 0, eFileNameLength);
			pStream->Read( szTempName, nLen, 0 );

			CRMEffect* pEffect = InsertEff( szTempName );
			BsAssert(pEffect && "Failed to insert an effect");// aleksger: prefix bug 377: Possible failure to insert.
			pEffect->Load( pStream );
		}
	}

	SetDefaultLighting(); //?
}


void CBsRealMovie::SetDefaultLighting()
{
	if( m_mapCamera.size() )
	{
		std::map< std::string , CRMCamera* >::iterator it;

		it = m_mapCamera.find( g_szMainCamName );

		if( it != m_mapCamera.end() )
		{
			it->second->m_nLightYaw = 50 - 10;
			it->second->m_nLightRoll = 30;
			it->second->m_nLightPitch = 450 - 60;
		}
	}
	else
	{
	}
}

//-----------------------------------------------------------------------------------------------------

void CBsRealMovie::ChdirStart() 
{
#ifdef _USAGE_TOOL_
    g_BsKernel.chdir("Data");
	//g_BsKernel.chdir("RealMovie"); //? 변경 예정 = m_szPathName 값이 제대로 입력되었다면, 여기서 동작 할것입니다
#else
	// g_BsKernel.chdir("RealMovie");
#endif
}

void CBsRealMovie::ChdirEnd() 
{
#ifdef _USAGE_TOOL_
	g_BsKernel.chdir(".."); 
#endif
	// g_BsKernel.chdir(".."); 
}

void CBsRealMovie::Chdir(char* szPath)
{
	g_BsKernel.chdir( szPath ); 
}

//-----------------------------------------------------------------------------------------------------

CRMObject* CBsRealMovie::GetObject(char* szObjName)
{
	iteratorRMObject it = m_mapObject.find( szObjName );

	if( it != m_mapObject.end() )
		return it->second;
	return NULL;
}

CRMEffect*	CBsRealMovie::GetEffect(char* szEffName)
{
	iteratorRMEffect it = m_mapEffect.find( szEffName );

	if( it != m_mapEffect.end() )
		return it->second;
	return NULL;
}

CRMWave*	CBsRealMovie::GetWave(char* szWavName)
{
	iteratorRMWave it = m_mapWave.find( szWavName );

	if( it != m_mapWave.end() )
		return it->second;
	return NULL;
}

CRMCamera*	CBsRealMovie::GetCamera(char* szCamName)
{
	iteratorRMCamera it = m_mapCamera.find( szCamName );

	if( it != m_mapCamera.end() )
		return it->second;
	return NULL;
}

CRMLip*		CBsRealMovie::GetLip(char* szFullName)
{
	if( szFullName == NULL || szFullName[0] == NULL )
		return NULL;

	char szTemp[130];
	// 앞(Lip)::뒤(Obj)
	strncpy( szTemp, szFullName, 130 );
	char* pdest = strstr( szTemp , "::" );
	
	if ( !pdest )	// mruete: prefix bug 388: added check
		return NULL;
		
	*pdest = NULL;

	CRMObject* pObj = GetObject( pdest+2 );
	if( !pObj )
		return NULL;

	CRMLip* pLip = pObj->GetLip( szTemp );

	return pLip;
}

int CBsRealMovie::GetFileSize(char *szFileName) 
{
	BFileStream stream(szFileName); // 디렉토리는 설정 된 상태여야 합니다 
	return stream.Length();
}

//-----------------------------------------------------------------------------------------------------

VOID	CBsRealMovie::AllocateSinglyTempResource(int nIndex)
{
	ChdirStart();

	CRMObject* pObject = GetTempObj( nIndex );

	pObject->CreateNewObject();
	pObject->LoadPartObjects();
	pObject->LoadLipResource();


	ChdirEnd();
}

VOID	CBsRealMovie::ReleaseTempBuffer(VOID)
{
	if( m_ppObjTempBuf )
	{
		delete [] m_ppObjTempBuf;
	}

	m_ppObjTempBuf = NULL;
	m_nTempBufSize = 0;
}

void CBsRealMovie::AllocateResource() 
{
	//g_BsKernel.GetDevice()->AcquireThreadOwnerShip();
	LARGE_INTEGER liFrequency;
	LARGE_INTEGER liStartTime;
	LARGE_INTEGER liCurTime;
	QueryPerformanceFrequency(&liFrequency);
	QueryPerformanceCounter(&liStartTime);

	ChdirStart();

	//-----------------------
	// 캐릭터
	//-----------------------
	iteratorRMObject it;
	iteratorRMObject itEnd = m_mapObject.end();

#ifdef _XBOX
	int i = 0;

	m_nTempBufSize = (int)m_mapObject.size();
	m_ppObjTempBuf = new CRMObject*[m_nTempBufSize];

#endif

	for( it = m_mapObject.begin() ; it != itEnd ; ++it )
	{
		CRMObject* pObject = it->second;

		// TODO: 임시.
#ifdef _XBOX
		if( pObject->SubData.m_nObjType == eObjType_P1_Weapon ) // P1 무기일때
		{
			if( m_nPlayerWeaponSkinIndex != -1 )
			{
				pObject->SetSkinIndexForWeapon( m_nPlayerWeaponSkinIndex ); 
			}
		}
#endif
		
#ifdef _XBOX
		m_ppObjTempBuf[ i++ ] = pObject;
#else
		int nRet = pObject->CreateNewObject();
		
		if( nRet == -1 )
		{
			char szTemp[eFileNameLength];
			strcpy( szTemp, pObject->GetSkinFileName() );

			pObject->ClearNameBuffer();
			nRet = pObject->CreateNewObject();

			ErrorNotFile( szTemp );
		}
		else if( nRet == -2 )
		{
			ErrorNotFile( pObject->GetBaFileName() );
		}
		else
		{
			//-----------------------
			// Parts
			//-----------------------
			pObject->LoadPartObjects();
		}

		//-----------------------
		// 립싱크
		//-----------------------
		pObject->LoadLipResource();
#endif
	}


	// ResetEventFlag
	//-----------------------
	// 웨이브
	//-----------------------
	iteratorRMWave itWav = m_mapWave.begin();
	iteratorRMWave itWavEnd = m_mapWave.end();

	for( ; itWav != itWavEnd ; ++itWav )
	{
		itWav->second->ResetEventFlag();
	}

	//-----------------------
	// 카메라
	//-----------------------
	iteratorRMCamera	itCam = m_mapCamera.begin();
	iteratorRMCamera	itCamEnd = m_mapCamera.end();

	for( ; itCam != itCamEnd ; ++itCam )
	{
		if( itCam->second->HasBCFile() )
		{
			itCam->second->LoadBCResource();
		}
	}
	
	//
	ChdirEnd();

	//싱크..
	/*
	itEnd = m_mapObject.end();
	for( it = m_mapObject.begin() ; it != itEnd ; ++it )
	{
		CRMObject* pObject = it->second;

		for( int i = 0 ; i < CRMObject::eLinkedObjMax ; ++i )
		{
			
			// CRMObject* pLinkObj = pObject->m_LinkedObj[i].pObject;
			// if( pLinkObj != NULL )
			// {
			//	pObject->m_LinkedObj[i].nObjIndex = pLinkObj->m_nObjectIndex;
			// }
		}
	}
	*/

	// 연결된 오브젝트 싱크.
#ifndef _XBOX
	SyncResources();
#endif

	QueryPerformanceCounter(&liCurTime);	
	double dwTime = (double)(( liCurTime.QuadPart - liStartTime.QuadPart ) / ( liFrequency.QuadPart / 1000.f ));
	DebugString("-- Realtime Move Data Allocation time (%f ms)\n", dwTime);
	//g_BsKernel.GetDevice()->ReleaseThreadOwnerShip();
}

VOID	CBsRealMovie::SyncResources(VOID)
{
	DebugString("-- Realtime Move Data Sync \n");

	iteratorRMObject it;
	iteratorRMObject itEnd = m_mapObject.end();

	for( it = m_mapObject.begin() ; it != itEnd ; ++it )
	{
		CRMObject* pObject = it->second;

		if( pObject->m_nObjectIndex == -1 )
			continue;

		for( int i = 0 ; i < CRMObject::eLinkedObjMax ; ++i )
		{
			if( pObject->GetLinkedObjName( i )[0] == NULL )
				continue;

			CRMObject *pLinkedObj = FindObjectByOBJName( pObject->GetLinkedObjName( i ) );

			if( pLinkedObj )
			{
#ifdef _USAGE_TOOL_
				if( pLinkedObj->m_nObjectIndex != -1 )
				{
#else
				assert( pLinkedObj->m_nObjectIndex != -1 );
#endif
				
					if( pObject->GetBoneName( i )[0] != NULL )
					{
						g_BsKernel.SendMessage( 
							pObject->m_nObjectIndex, 
							BS_LINKOBJECT_NAME, 
							(DWORD)pObject->GetBoneName( i ),
							pLinkedObj->m_nObjectIndex );
					}
#ifdef _USAGE_TOOL_
				}
#endif
			}
		}
	}
}



VOID	CBsRealMovie::ResetFlags(VOID)
{
	ClearPlayCam();
	ClearObjFlags();
}

void	CBsRealMovie::ResetAllObjectAni(void)
{
	iteratorRMObject it;
	iteratorRMObject itEnd = m_mapObject.end();

	for( it = m_mapObject.begin() ; it != itEnd ; ++it )
	{
		it->second->ResetAniObject();
	}
}

void	CBsRealMovie::ClearObjFlags(void)
{
	// ClearAniFlags
	iteratorRMObject it;
	iteratorRMObject itEnd = m_mapObject.end();

	for( it = m_mapObject.begin() ; it != itEnd ; ++it )
	{
		it->second->ClearAniFlags();
		it->second->DisableObjectAlphaMode();
	}
}

CRMCamera*	CBsRealMovie::GetCameraByTime(float fTime)
{
	// static CRMCamera* s_pCamera;

	vecCamera::iterator itCam = m_vecCamera.begin();
	vecCamera::iterator itCamEnd = m_vecCamera.end();

	for( ; itCam != itCamEnd ; ++itCam )
	{
		// if( ( (*itCam)->m_fTime - fTime ) <= 0.f )
		if( (*itCam)->m_fTime == fTime )
		{
			m_pStaticCam = (*itCam);
			return (*itCam);
		}
	}

	return m_pStaticCam;

	// assert( 0 && "계산이 꼬였다." );
	// 카메라가 없다.
	// return NULL;
}

CRMEffect*	CBsRealMovie::GetCamEffectOnTick(DWORD dwTick)
{
	iteratorRMEffect	itEff = m_mapEffect.begin();
	iteratorRMEffect	itEffEnd = m_mapEffect.end();

	for( ; itEff != itEffEnd ; ++itEff )
	{
		if( itEff->second->GetStartTick() == dwTick )
		{
            if( itEff->second->IsCamType() )
			{
				return itEff->second;
			}
		}
	}

	return NULL;
}

VOID	CBsRealMovie::ToggleShowBoundingBox(BOOL bShow)
{
	iteratorRMObject it;
	iteratorRMObject itEnd = m_mapObject.end();

	for( it = m_mapObject.begin() ; it != itEnd ; ++it )
	{
		if( it->second->m_nObjectIndex != -1  )
		{
            g_BsKernel.SendMessage( it->second->m_nObjectIndex,
					BS_SHOW_BOUNDINGBOX, (DWORD)bShow );
		}
	}
}

CRMObject*	CBsRealMovie::FindObjectByOBJIndex(int nObjIndex)
{
	iteratorRMObject it;
	iteratorRMObject itEnd = m_mapObject.end();

	for( it = m_mapObject.begin() ; it != itEnd ; ++it )
	{
		if( it->second->m_nObjectIndex == nObjIndex )
			return it->second;
	}

	return NULL;
}

CRMObject*	CBsRealMovie::FindObjectByOBJName(char* szName)
{
	iteratorRMObject it;
	iteratorRMObject itEnd = m_mapObject.end();

	for( it = m_mapObject.begin() ; it != itEnd ; ++it )
	{
		if( strcmp( it->second->m_szName , szName ) == 0 )
			return it->second;
	}

	return NULL;
}

VOID	CBsRealMovie::ApplyObjectCullMove(bool bSet)
{
	iteratorRMObject it;
	iteratorRMObject itEnd = m_mapObject.end();

	for( it = m_mapObject.begin() ; it != itEnd ; ++it )
	{
		if( it->second->m_nObjectIndex != -1 )
		{
            g_BsKernel.SendMessage(
				it->second->m_nObjectIndex,
                BS_ENABLE_OBJECT_CULL, bSet );
		}
	}

	return;
}

int		CBsRealMovie::GetSelectedAllObj(D3DXVECTOR4& rfRect,vecSelObjPack& rvecRetPool,short* pHeightBuf,
										PTR_FUNC_GETHEIGHT pfnGetHeight)
{
	// 먼저 오브젝트.
	// stObjPack
	float fLand;
	int nCnt = 0;
	stObjPack	tempPack;
	
	iteratorRMObject it;
	iteratorRMObject itEnd = m_mapObject.end();

	for( it = m_mapObject.begin() ; it != itEnd ; ++it )
	{
		if( it->second->PtInfRect( rfRect ) )
		{
			tempPack.nType = 0;
			tempPack.pObject = (void*)it->second;

			fLand = (*pfnGetHeight)( it->second->GetPosition().x,
							   it->second->GetPosition().z, 
							   pHeightBuf );

			tempPack.fHeightDis = it->second->GetPosition().y - fLand;	// 차.

			rvecRetPool.push_back( tempPack );

			// 선택 된 표시 하기
			g_BsKernel.SendMessage( it->second->m_nObjectIndex, BS_SHOW_BOUNDINGBOX, (DWORD)true );
			++nCnt;
		}
	}

	iteratorRMCamera itCam;
	iteratorRMCamera itCamEnd = m_mapCamera.end();

	for( itCam = m_mapCamera.begin() ; itCam != itCamEnd ; ++itCam )
	{
		if( itCam->second->PtInfRect( rfRect ) )
		{
			vecCamPath::iterator itPath = itCam->second->m_CamPathPool.begin();
			vecCamPath::iterator itPathEnd = itCam->second->m_CamPathPool.end();

			for( ; itPath != itPathEnd ; ++itPath )
			{
				fLand = (*pfnGetHeight)( (*itPath)->m_Cross.m_PosVector.x,
								   (*itPath)->m_Cross.m_PosVector.z,
                                   pHeightBuf );

				tempPack.nType = 2;
				tempPack.pObject = (void*)(*itPath);
				tempPack.fHeightDis = (*itPath)->m_Cross.m_PosVector.y - fLand;

				rvecRetPool.push_back( tempPack );
				++nCnt;
			}
		}
	}

	return nCnt;
}

int		CBsRealMovie::ResetSelectedAllObjYPos(vecSelObjPack& rvecRetPool,short* pHeightBuf,
											  PTR_FUNC_GETHEIGHT pfnGetHeight)
{
	float fCurHeight;
	int nCnt = 0;
	stObjPack* pPack;
	vecSelObjPack::iterator it = rvecRetPool.begin();
	vecSelObjPack::iterator itEnd = rvecRetPool.end();

	for( ; it != itEnd ; ++it )
	{
		pPack = &(*it);

		if( pPack->nType == 0 ) // RMObject.
		{
			CRMObject* pRMObj = (CRMObject*)pPack->pObject;

			// 새로운 현재 위치 구하기.
			fCurHeight = (*pfnGetHeight)( pRMObj->GetPosition().x,
									pRMObj->GetPosition().z,
									pHeightBuf );

			// 새로 구한 위치에 셋. 붐~~
			pRMObj->GetPosPtr()->y = fCurHeight + pPack->fHeightDis;
			++nCnt;
		}
		else if( pPack->nType == 2 ) // Cam Path.
		{
			CRMCamPath* pCamPath = (CRMCamPath*)pPack->pObject;

			fCurHeight = (*pfnGetHeight)( pCamPath->m_Cross.m_PosVector.x,
									pCamPath->m_Cross.m_PosVector.z,
                                    pHeightBuf );

			// 붐~
			pCamPath->m_Cross.m_PosVector.y = fCurHeight + pPack->fHeightDis;
			++nCnt;
		}
	}

	return nCnt;
}

#ifdef _USAGE_TOOL_
int		CBsRealMovie::ResetSelectedObjectYPos(vecSelObjPack& rvecRetPool,short* pHeightBuf,
								PTR_FUNC_GETHEIGHT pfnGetHeight)
{
	float fCurHeight;
	int nCnt = 0;
	stObjPack* pPack;
	vecSelObjPack::iterator it = rvecRetPool.begin();
	vecSelObjPack::iterator itEnd = rvecRetPool.end();

	for( ; it != itEnd ; ++it )
	{
		pPack = &(*it);

		if( pPack->nType == 0 ) // RMObject.
		{
			CRMObject* pRMObj = (CRMObject*)pPack->pObject;

			// 새로운 현재 위치 구하기.
			fCurHeight = (*pfnGetHeight)( pRMObj->GetPosition().x,
				pRMObj->GetPosition().z,
				pHeightBuf );

			// 새로 구한 위치에 셋. 붐~~
			pRMObj->GetPosPtr()->y = fCurHeight; // + pPack->fHeightDis;
			++nCnt;
		}
	}

	return nCnt;
}

#endif

VOID	CBsRealMovie::ResetCamsPivotLine(VOID)
{
	iteratorRMCamera itCam;
	iteratorRMCamera itCamEnd = m_mapCamera.end();

	for( itCam = m_mapCamera.begin() ; itCam != itCamEnd ; ++itCam )
	{
		int nCnt = itCam->second->GetPathCnt();

		for( int i = 0 ; i < nCnt ; ++i )
		{
            itCam->second->SyncDrawLine( i );
		}
	}
}

// 선택된 표식 지우기.
int 	CBsRealMovie::ClearAllObjSelectedMark(vecSelObjPack& rvecRetPool)
{
	int nCnt=0;
	stObjPack* pPack;
	vecSelObjPack::iterator it = rvecRetPool.begin();
	vecSelObjPack::iterator itEnd = rvecRetPool.end();

	for( ; it != itEnd ; ++it )
	{
		pPack = &(*it);

		if( pPack->nType == 0 ) // RMObject.
		{
			CRMObject* pRMObj = (CRMObject*)pPack->pObject;

			g_BsKernel.SendMessage( pRMObj->m_nObjectIndex, BS_SHOW_BOUNDINGBOX, (DWORD)false );
			++nCnt;
		}
	}

	return nCnt;
}

int		CBsRealMovie::MoveObjects( vecSelObjPack& rvecRetPool, float fMovX, float fMovZ, short* pHeightBuf,
								  PTR_FUNC_GETHEIGHT pfnGetHeight)
{
	float fCurHeight;
	int nCnt = 0;
	stObjPack* pPack;
	vecSelObjPack::iterator it = rvecRetPool.begin();
	vecSelObjPack::iterator itEnd = rvecRetPool.end();

	for( ; it != itEnd ; ++it )
	{
		pPack = &(*it);

		if( pPack->nType == 0 ) // RMObject.
		{
			CRMObject* pRMObj = (CRMObject*)pPack->pObject;

			pRMObj->GetPosPtr()->x += fMovX;
			pRMObj->GetPosPtr()->z += fMovZ;

			// 새로운 현재 위치 구하기.
			fCurHeight = (*pfnGetHeight)( pRMObj->GetPosition().x,
				pRMObj->GetPosition().z,
				pHeightBuf );

			// 새로 구한 위치에 셋. 붐~~
			pRMObj->GetPosPtr()->y = fCurHeight + pPack->fHeightDis;
			++nCnt;
		}
		else if( pPack->nType == 2 ) // Cam Path.
		{
			CRMCamPath* pCamPath = (CRMCamPath*)pPack->pObject;

			pCamPath->m_Cross.m_PosVector.x += fMovX;
			pCamPath->m_Cross.m_PosVector.z += fMovZ;

			fCurHeight = (*pfnGetHeight)( pCamPath->m_Cross.m_PosVector.x,
				pCamPath->m_Cross.m_PosVector.z,
				pHeightBuf );

			// 붐~
			pCamPath->m_Cross.m_PosVector.y = fCurHeight + pPack->fHeightDis;
			++nCnt;
		}
	}
	
	return nCnt;
}

// szWaveName 은 Tool 용
// Lip Wave명 구성 -  LipName::ObjectName (MAX:130)
// bg Wave 명 구성 -  WaveName
int		CBsRealMovie::PreProcess(float fTime,DWORD dwTick,msgRMTNotify* pRMTNotify)
{
	int nType;
	VOID* pObjPtr;
	int nCnt = EventNotify( fTime , dwTick );

	for( int i = 0 ; i < nCnt ; ++i )
	{
		pObjPtr = GetSelectedEvent( i , nType );

		switch( nType )
		{
		case RMTYPE_LIP :
			pRMTNotify[i].nType = RMTYPE_LIP;
			((CRMLip*)pObjPtr)->GetKeyString( pRMTNotify[i].szKeyName, _countof(pRMTNotify[i].szKeyName));//aleksger - safe string
			pRMTNotify[i].dwParam1 = ((CRMLip*)pObjPtr)->GetTextID();
			break;
		case RMTYPE_WAVE :

			if( ((CRMWave*)pObjPtr)->GetUseVoiceCue() == 1 )
				pRMTNotify[i].nType = RMTYPE_LIP;
			else
                pRMTNotify[i].nType = RMTYPE_WAVE;

			// strncpy( pRMTNotify[i].szKeyName, ((CRMWave*)pObjPtr)->GetName(), eFileNameLength );
			((CRMWave*)pObjPtr)->GetKeyString( pRMTNotify[i].szKeyName, _countof(pRMTNotify[i].szKeyName) ); //aleksger - safe string
			pRMTNotify[i].dwParam1 = ((CRMWave*)pObjPtr)->GetTextID(); // Text ID 값을 얻어옴
			break;
		case RMTYPE_EFFECT :
			{
				int nType = ((CRMEffect*)pObjPtr)->m_nType;

				switch( nType )
				{
				case eFadeIn :
					{
						pRMTNotify[i].nType = RMTYPE_EFFECT_FADEIN;
						pRMTNotify[i].dwParam1 = (DWORD)pObjPtr;
						pRMTNotify[i].dwParam2 = (DWORD)((CRMEffect*)pObjPtr)->GetUseTime();
					}
					break;
				case eFadeOut :
					{
						pRMTNotify[i].nType = RMTYPE_EFFECT_FADEOUT;
						pRMTNotify[i].dwParam1 = (DWORD)pObjPtr;
						pRMTNotify[i].dwParam2 = (DWORD)((CRMEffect*)pObjPtr)->GetUseTime();
					}
					break;
				case eBlack :
					{
						pRMTNotify[i].nType = RMTYPE_EFFECT_BLACK;
						pRMTNotify[i].dwParam1 = (DWORD)pObjPtr;
						pRMTNotify[i].dwParam2 = (DWORD)((CRMEffect*)pObjPtr)->GetUseTime();
					}
					break;
				case eSpecialFX :
					{
						pRMTNotify[i].nType = RMTYPE_EFFECT_CMD_FX;
						strncpy( pRMTNotify[i].szKeyName, ((CRMEffect*)pObjPtr)->GetStrParam(), eFileNameLength );
						pRMTNotify[i].dwParam1 = (DWORD)pObjPtr;
						pRMTNotify[i].dwParam2 = ((CRMEffect*)pObjPtr)->GetParam(0);
					}
					break;
				case eFX :
					{
						pRMTNotify[i].nType = RMTYPE_EFFECT_FX;
						strncpy( pRMTNotify[i].szKeyName, ((CRMEffect*)pObjPtr)->GetFileName(), eKeyStringLength );
						pRMTNotify[i].dwParam1 = (DWORD)pObjPtr;
						pRMTNotify[i].dwParam2 = 1;
					}
					break;
				case eUseCam :
					{
						pRMTNotify[i].nType = RMTYPE_EFFECT_USECAM;	 // 카메라 쓸거라고 알림.
						pRMTNotify[i].dwParam1 = (DWORD)pObjPtr;

						if( ((CRMEffect*)pObjPtr)->m_dwParam[0] == 1 )
							pRMTNotify[i].dwParam2 = 1;
						else
							pRMTNotify[i].dwParam2 = 0;

						pRMTNotify[i].dwParam3 = ((CRMEffect*)pObjPtr)->GetStartTick(); // 시작 Tick.
					}
					break;
				case eShowObject :
					{
						pRMTNotify[i].nType = RMTYPE_EFFECT_SHOWOBJECT;	 // 오브젝트 Show On/Off 알림.
						pRMTNotify[i].dwParam1 = (DWORD)pObjPtr;
						pRMTNotify[i].dwParam2 = ((CRMEffect*)pObjPtr)->m_dwParam[0];
					}
					break;
				case eSetTroop :
					{
						pRMTNotify[i].nType = RMTYPE_EFFECT_SETTROOP;	 // 오브젝트 Show On/Off 알림.
						pRMTNotify[i].dwParam1 = (DWORD)pObjPtr;
						pRMTNotify[i].dwParam2 = (DWORD)((CRMEffect*)pObjPtr)->GetFileName(); // Troop Name
						pRMTNotify[i].dwParam3 = (DWORD)((CRMEffect*)pObjPtr)->GetStrParam(); // Area Name
						pRMTNotify[i].dwParam4 = (DWORD)((CRMEffect*)pObjPtr)->GetParam(0);	// Troop Dir
						pRMTNotify[i].dwParam5 = (DWORD)((CRMEffect*)pObjPtr)->GetParam(1);	//0-Disable,1-Enable
					}
					break;
				case eMoveTroop :
					{
						pRMTNotify[i].nType = RMTYPE_EFFECT_MOVETROOP;	 // 오브젝트 Show On/Off 알림.
						pRMTNotify[i].dwParam1 = (DWORD)pObjPtr;
						pRMTNotify[i].dwParam2 = (DWORD)((CRMEffect*)pObjPtr)->GetFileName(); // Troop Name
						pRMTNotify[i].dwParam3 = (DWORD)((CRMEffect*)pObjPtr)->GetStrParam(); // Area Name
						pRMTNotify[i].dwParam4 = (DWORD)((CRMEffect*)pObjPtr)->GetParam(0);
						pRMTNotify[i].dwParam5 = (DWORD)((CRMEffect*)pObjPtr)->GetParam(1);	//0-걷기,1-뛰기
					}
					break;
				case eUseLight :
					{
						pRMTNotify[i].nType = RMTYPE_EFFECT_USELIGHT;	 // 오브젝트 Show On/Off 알림.
						pRMTNotify[i].dwParam1 = (DWORD)pObjPtr;
					}
					break;
				case eUseDOF :
					{
						pRMTNotify[i].nType = RMTYPE_EFFECT_USEDOF;	 // 오브젝트 Show On/Off 알림.
						pRMTNotify[i].dwParam1 = (DWORD)pObjPtr;
					}
					break;
				case eUseWave :
					{
						pRMTNotify[i].nType = RMTYPE_EFFECT_USESOUND;	 // 오브젝트 Show On/Off 알림.
						pRMTNotify[i].dwParam1 = (DWORD)pObjPtr;
#ifdef _XBOX
						CRMWave* pWave = (CRMWave*) ((CRMEffect*)pObjPtr)->GetLinkedObjPtr();
						
						if( pWave )
						{
							DWORD dwWaveOwnerObj = pWave->GetRMObjPtr();
							pWave->GetKeyString( pRMTNotify[i].szKeyName, _countof(pRMTNotify[i].szKeyName) );//aleksger - safe string

							if( pWave->GetUseVoiceCue() == 1 )
								pRMTNotify[i].dwParam2 = RMTYPE_LIP;
							else
								pRMTNotify[i].dwParam2 = RMTYPE_WAVE;

							CRMObject* pRMObj = GetObject( ((CRMEffect*)pObjPtr)->GetLinkedObjName() );

							if( pRMObj )
							{
                                pRMTNotify[i].dwParam3 = (DWORD)pRMObj;
								pRMTNotify[i].dwParam4 = (DWORD)pRMObj->GetAniObjPos();
								// pRMTNotify[i].dwParam5 = (DWORD)pRMObj; // ->m_nObjectIndex;
							}
							else if( dwWaveOwnerObj )
							{
								pRMTNotify[i].dwParam3 = dwWaveOwnerObj;
								pRMTNotify[i].dwParam4 = (DWORD)((CRMObject*)dwWaveOwnerObj)->GetAniObjPos();
								// pRMTNotify[i].dwParam5 = -1;
							}
							else
							{
								pRMTNotify[i].dwParam3 = 0;
								pRMTNotify[i].dwParam4 = 0;
							}
						}
						else
						{
							// BsAssert( pWave && "RMWave Object not found" );
							pRMTNotify[i].nType = RMTYPE_EFFECT_NONE;
						}
#endif
					}
					break;
/*
				case eObjAlphaBlend :
					{
						CRMObject* pRMObj = FindObjectByOBJName( ((CRMEffect*)pObjPtr)->GetLinkedObjName() );

						if( pRMObj )
						{
							if( ((CRMEffect*)pObjPtr)->GetParam( 0 ) )
								pRMObj->ObjectAlphaMode( ((CRMEffect*)pObjPtr)->GetFParam( 0 ) );
							else
								pRMObj->ObjectAlphaMode( 1.f );
						}
					}
					break;
*/
				default :
					{
						pRMTNotify[i].nType = RMTYPE_EFFECT_NONE;
					}
					break;
				} // end of switch

			}
			break;
		case RMTYPE_PACKAGE :
			{
				pRMTNotify[i].nType = RMTYPE_PACKAGE;
				pRMTNotify[i].dwParam1 = (DWORD)pObjPtr;
			}
			break;
		}
	}

	return nCnt;
}

// For Tool
// Mini PreProcess

// void CBsRealMovie::Play(float fTime, D3DXMATRIX *pMatrix, CCrossVector *pCameraCross, CCrossVector *pLightCross) 
void	CBsRealMovie::Process(float fTime,int nTick)
{
	PlayEff_ForProcess( (int)fTime );
	// ProcessBone();
	
	iteratorRMObject it;
	iteratorRMObject itEnd = m_mapObject.end();

	for( it = m_mapObject.begin() ; it != itEnd ; ++it )
	{
		CRMObject* pObject = it->second;

		// Act 만
		if( pObject->PlayAniObject( fTime ) )
		{
			m_nBlend=0;

			// !(GetAniCount() <= 1)
			if( pObject->GetAniCount() > 1 )
			{
				pObject->PlayActEx( nTick, m_nBlend );

#ifdef _USAGE_TOOL_
				// char szTemp[32];
				// sprintf(szTemp,"Blend %d\n", m_nBlend );
				// DebugString( szTemp );
#endif
				if( !m_nBlend )
				{
					pObject->ResetAniObject();
				}
			}
		}

		//pObject->ProcessCurrentAniForLinkedObj();
	}
}



int		CBsRealMovie::PostProcess(float fTime)
{
	return GetEffEvent( fTime );
}


void	CBsRealMovie::Update(float fTime)
{
	//-----------------------
	// 카메라
	//-----------------------
	float fTime60cam=0.f;

	//-----------------------
	// 오브젝트 (캐릭터)
	//-----------------------
	iteratorRMObject it;
	iteratorRMObject itEnd = m_mapObject.end();

	for( it = m_mapObject.begin() ; it != itEnd ; ++it )
	{
		CRMObject* pObject = it->second;

		/*
		if( pObject->PlayAniObject( fTime ) )
		{
			m_nBlend=0;

			// !(GetAniCount() <= 1)
			if( pObject->GetAniCount() > 1 )
			{
				pObject->PlayAct( fTime, m_nBlend );

#ifdef _USAGE_TOOL_
				//char szTemp[32];
				//sprintf(szTemp,"Blend %d\n", m_nBlend );
				//DebugString( szTemp );
#endif
				if( !m_nBlend )
				{
					pObject->ResetAniObject();
				}
		*/
				pObject->Render();
				pObject->PlayFace( (int)(fTime * 40.f), m_nBlend );
				pObject->PlayLip( (int)(fTime * 40.f), m_nBlend );
		/*
			}
		}
		*/
	}

	PlayEff_ ( (int)(fTime * 40.f) );
	PlayWave_( (int)(fTime * 40.f) );

	if( !m_nbUseRMCam )
	{
		iteratorRMCamera itCam = m_mapCamera.begin();
		iteratorRMCamera itCamEnd = m_mapCamera.end();

		for( ; itCam != itCamEnd ; ++itCam )
		{
			// itCam->second->SetCrossVector( *pCameraCross );
			itCam->second->UpdateObject( fTime60cam );
		}
	}

	// RenderBone();
	itEnd = m_mapObject.end();

	for( it = m_mapObject.begin() ; it != itEnd ; ++it )
	{
		CRMObject* pObject = it->second;


		pObject->UpdateObject();

		//pObject->RefreshAni();	// 디버그
		//pObject->ProcessCurrentAniForLinkedObj();
	}
}

void	CBsRealMovie::ProcessBone()
{
	if( !m_vecBonePackPool.size() )
	{
		vecBonePack::iterator it = m_vecBonePackPool.begin();
		vecBonePack::iterator itEnd = m_vecBonePackPool.end();

		for( ; it != itEnd ; ++it )
		{
			//g_BsKernel.SendMessage( it->nObjIndex, 
				//BS_SET_BONE_ROTATION_PROCESS, it->dwBoneNamePtr, (DWORD)&it->vecRot );
			//g_BsKernel.SendMessage( it->nObjIndex, 
				//BS_SET_BONE_ROTATION, it->dwBoneNamePtr, (DWORD)&it->vecRot );

			// 주의 : 추가됨.
			//pRMObj->RefreshAni();	// 디버그
			//pRMObj->ProcessCurrentAniForLinkedObj();
		}

		m_vecBonePackPool.clear();
	}
}


void	CBsRealMovie::ResetAni(void)
{
	iteratorRMObject it;
	iteratorRMObject itEnd = m_mapObject.end();
	CRMObject* pObject;

	for( it = m_mapObject.begin() ; it != itEnd ; ++it )
	{
		pObject = it->second;
		if( pObject->m_pAniObject )
			pObject->m_pAniObject->ResetAniIndex();
	}
}


/*
CRMEffect*	CBsRealMovie::ProcessEffect(CRMEffect* pEffect,float fTime)
{
	int ckLast=-1;
	float ckTime=-1;
	CRMEffect*	pSelectEffect = NULL;

	float fRunTime = fTime;
	float fStartTime = pEffect->GetStartTime();
	float fUseTime = pEffect->GetUseTime();

	if( fRunTime < fStartTime || fStartTime < 0.f )
		return NULL;

	if( (fStartTime + fUseTime) < fTime )
		return NULL;

	fRunTime -= fStartTime;
	int nType = pEffect->GetEffectType();

	if( nType == eTerminate )
		fUseTime = 100;

	pSelectEffect = pEffect;

	switch( nType )
	{
	case eFadeIn :
	case eFadeOut :
	case eBlack :
		if ( ckTime < fStartTime ) 
		{
			ckTime = fStartTime; // 가장 큰 값으로 유지 될 것입니다	
			// ckLast = i;
			pSelectEffect = pEffect;
		}
		break;
	case eSpecialFX :
		break;
	case eTerminate :
		m_bFadeOut = 0;
		break;
	case eUseCam :
		if ( ckTime < fStartTime ) 
		{
			ckTime = fStartTime; // 가장 큰 값으로 유지 될 것입니다	
			// ckLast = i;
			pSelectEffect = pEffect;
		}
		break;
	}

	if( !CRMEffect::EffectProcess( pSelectEffect, fTime ) )
	{
		// 처리 못하는게 있다면..
		int nType = pSelectEffect->m_nType;

		switch( nType )
		{
		case eUseCam :
			{
				CRMCamera* pCam = GetCamera( pSelectEffect->GetFileName() );

				if( pCam )
				{
					m_pStaticCam = pCam;
				}
			}
			break;
		case eShowObject :
			{
				CRMObject* pRMObj = FindObjectByOBJName( pSelectEffect->GetLinkedObjName() );

				if( pRMObj )
				{
#ifdef _USAGE_TOOL_
					if( pRMObj->m_nObjectIndex == -1 )
						return pSelectEffect;
#endif
					// DebugOutput("");
					pRMObj->SetObjShowFlag( (BOOL)pSelectEffect->m_dwParam[0] );
					g_BsKernel.ShowObject( pRMObj->m_nObjectIndex , 
											pSelectEffect->m_dwParam[0] ? true : false );
				}
			}
			break;
		case eUseBone :
			{				
				pSelectEffect->ProcessBoneAniFrame( (int)fTime );
				// --nDebug;	assert( nDebug == 0 );
			}
			break;
		case eSetFog :
			{
				int nCamEngIndex = g_BsKernel.GetCameraHandle( 0 );
				if( nCamEngIndex != -1 )
				{
					g_BsKernel.SetFogFactor( nCamEngIndex, 
						(float)pSelectEffect->m_dwParam[0],
						(float)pSelectEffect->m_dwParam[1] );
				}
			}
			break;
		case eWaveVolume :
			{
			}
			break;
		case eReuseAct :
			{
			}
			break;
		}
	}

	return pSelectEffect;
}
*/


CRMEffect*	CBsRealMovie::ProcessEffect(CRMEffect* pEffect,int nFrame)
{
	int ckLast=-1;
	// float ckTime=-1;
	int ckTime = -1;
	CRMEffect*	pSelectEffect = NULL;

	int nRunTime = nFrame;
	int nStartTime = (int)pEffect->GetStartTick();
	int nUseTime = pEffect->GetUseFrame();

	if( nRunTime < nStartTime || nStartTime < 0 )
		return NULL;

	if( ( nStartTime + nUseTime ) < nFrame )
		return NULL;

	nRunTime -= nStartTime;
	int nType = pEffect->GetEffectType();

	if( nType == eTerminate )
		nUseTime = 4000;		// 40 * 100 초

	pSelectEffect = pEffect;

	switch( nType )
	{
	case eFadeIn :
	case eFadeOut :
	case eBlack :
		if ( ckTime < nStartTime ) 
		{
			ckTime = nStartTime; // 가장 큰 값으로 유지 될 것입니다	
			pSelectEffect = pEffect;
		}
		break;
	case eSpecialFX :
		break;
	case eTerminate :
		m_bFadeOut = 0;
		break;
	case eUseCam :
		if ( ckTime < nStartTime ) 
		{
			ckTime = nStartTime; // 가장 큰 값으로 유지 될 것입니다	
			pSelectEffect = pEffect;
		}
		break;
	}

	if( !CRMEffect::EffectProcess( pSelectEffect, nFrame ) ) // fTime
	{
		// 처리 못하는게 있다면..
		int nType = pSelectEffect->m_nType;

		switch( nType )
		{
		case eUseCam :
			{
				CRMCamera* pCam = GetCamera( pSelectEffect->GetFileName() );

				if( pCam )
				{
					m_pStaticCam = pCam;
					/*
					char szTemp[200];
					sprintf( szTemp, "Selected Cam : %s\n", pCam->GetName() );
					DebugStringA( szTemp );
					*/
				}
			}
			break;
		case eShowObject :
			{
				CRMObject* pRMObj = FindObjectByOBJName( pSelectEffect->GetLinkedObjName() );

				if( pRMObj )
				{
#ifdef _USAGE_TOOL_
					if( pRMObj->m_nObjectIndex == -1 )
						return pSelectEffect;
#endif
					// DebugOutput("");
					pRMObj->SetObjShowFlag( (BOOL)pSelectEffect->m_dwParam[0] );
					g_BsKernel.ShowObject( pRMObj->m_nObjectIndex , 
						pSelectEffect->m_dwParam[0] ? true : false );
				}
			}
			break;
		case eUseBone :
			{				
				pSelectEffect->ProcessBoneAniFrame( nFrame );
				// --nDebug;	assert( nDebug == 0 );
			}
			break;
		case eSetFog :
			{
				int nCamEngIndex = g_BsKernel.GetCameraHandle( 0 );
				if( nCamEngIndex != -1 )
				{
					g_BsKernel.SetFogFactor( nCamEngIndex, 
						(float)pSelectEffect->m_dwParam[0],
						(float)pSelectEffect->m_dwParam[1] );
				}
			}
			break;
		case eWaveVolume :
			{
				/*
				float fRunTime = fTime;
				fRunTime -= pEffectObj->m_fTime;

				float fUseTime = pEffectObj->m_fUseTime;
				if( fUseTime < 0.0001f )
				fUseTime = 0.0001f;

				float fAlpha = ( fUseTime - fRunTime ) / fUseTime;
				*/

			}
			break;
		case eReuseAct :
			{
			}
			break;
		case eObjAlphaBlend :
			{
				CRMObject* pRMObj = FindObjectByOBJName( pSelectEffect->GetLinkedObjName() );

				if( pRMObj )
				{
#ifdef _USAGE_TOOL_
					if( pRMObj->m_nObjectIndex == -1 )
						return pSelectEffect;
#endif
					// DebugOutput("");
					if( pRMObj )
					{
						if( pSelectEffect->IsAlphaEnable() ) //GetParam( 0 ) )
							pRMObj->ObjectAlphaMode( 
								pSelectEffect->GetFParam( 0 ),
								pSelectEffect->GetParam( 0 ),
								pSelectEffect->GetParam( 1 ),
								pSelectEffect->GetParam( 2 ) );
						else
							pRMObj->DisableObjectAlphaMode();
					}
				}
			}
			break;

		}
	}

	return pSelectEffect;
}

void	CBsRealMovie::ProcessSelectedEffect(CRMEffect* pSelectEffect,int nFrame)
{
	if( pSelectEffect )	// ckLast
	{
		if( !CRMEffect::EffectProcess( pSelectEffect, nFrame ) )
		{
			// 처리 못하는게 있다면..
			int nType = pSelectEffect->m_nType;

			switch( nType )
			{
			case eUseCam :
				{
					CRMCamera* pCam = GetCamera( pSelectEffect->GetFileName() );

					if( pCam )
					{
						m_pStaticCam = pCam;
					}
				}
				break;
			}
		}
	}
}


void CBsRealMovie::PlayEff_( int nFrame )	
{
	iteratorRMEffect it;
	iteratorRMEffect itEnd = m_mapEffect.end();
	//CRMEffect*	pEffect = NULL;
	CRMEffect*	pSelectEffect = NULL;
	//float fRunTime;
	//float fStartTime;
	m_bFadeOut = 0;

	for( it = m_mapEffect.begin() ; it != itEnd ; ++it )
	{
		pSelectEffect = ProcessEffect( it->second, nFrame );
	}

	ProcessSelectedEffect( pSelectEffect, nFrame );
}

void	CBsRealMovie::ProcessPackage(CRMPackage* pPack,float fTime,DWORD dwTick)
{
	int nCnt = pPack->GetUseItemCnt();
	int nType;

	for( int i = 0 ; i < nCnt ; ++i )
	{
		nType = pPack->GetItemType( i );

		switch( nType )
		{
		case CRMPackage::eRMPackageType_Effect :
			{
				// CRMEffect* pEffect = GetEffect( pPack->GetItemName( i ) );
				// ProcessEffect( pEffect, )
			}
			break;
		case CRMPackage::eRMPackageType_Wave :
			{
			}
			break;
		}
	}
}

void CBsRealMovie::PlayEff_ForProcess( int nFrame )	
{
	int ckLast=-1;
	float ckTime=-1;

	iteratorRMEffect it;
	iteratorRMEffect itEnd = m_mapEffect.end();
	CRMEffect*	pEffect = NULL;
	CRMEffect*	pSelectEffect = NULL;

	int nRunTime;
	int nStartTime;
	m_bFadeOut = 0;

	for( it = m_mapEffect.begin() ; it != itEnd ; ++it )
	{
		nRunTime = nFrame;
		pEffect = it->second;
		//
		nStartTime = pEffect->GetStartTick();
		int nUseTime = pEffect->GetUseFrame();

		if( nRunTime < nStartTime || nStartTime < 0 )
			continue;

		if( (nStartTime + nUseTime) < nFrame )
			continue;

		nRunTime -= nStartTime;
		pSelectEffect = pEffect;

		if( pSelectEffect->m_nType == eUseBone )
		{
			//assert( nDebug == 0 );
            pSelectEffect->ProcessBoneAniFrameForProcess( nFrame );
			//++nDebug;
        }
	}
}

void CBsRealMovie::PlayWave_( int nFrame )	
{
	iteratorRMWave it;
	iteratorRMWave itEnd = m_mapWave.end();

	for( it = m_mapWave.begin() ; it != itEnd ; ++it )
	{
		// float fRunTime = fTime;
		int nStartTime = (int)it->second->GetStartTick(); // GetStartTime();

		if( nFrame < nStartTime || nStartTime < 0 )
			continue;

		if( it->second->m_nEvent == 0 )	// 이벤트가 한번만 통지 되도록 만들어 주는 코드 입니다
			it->second->m_nEvent = 1;
	}

}

void CBsRealMovie::Clear() 
{
	iteratorRMObject	it = m_mapObject.begin();
	iteratorRMObject	itEnd = m_mapObject.end();

	for( ; it != itEnd ; ++it )
	{
		// DeleteObject( (char *)(it->first.c_str()) );
		delete it->second;
	}

	iteratorRMCamera	itCam = m_mapCamera.begin();
	iteratorRMCamera	itCamEnd = m_mapCamera.end();

	for( ; itCam != itCamEnd ; ++itCam )
	{
		delete itCam->second;
		// DeleteCamera( (char*)(itCam->first.c_str()) );
	}

	iteratorRMWave		itWav = m_mapWave.begin();
	iteratorRMWave		itWavEnd = m_mapWave.end();

	for( ; itWav != itWavEnd ; ++itWav )
	{
		delete itWav->second;
		// DeleteWave( (char*)(itWav->first.c_str()) );
	}

	iteratorRMEffect	itEft = m_mapEffect.begin();
	iteratorRMEffect	itEftEnd = m_mapEffect.end();

	for( ; itEft != itEftEnd ; ++itEft )
	{
		delete itEft->second;
		// DeleteEffect( (char*)(itEft->first.c_str()) );
	}

	m_mapObject.clear();
	m_mapCamera.clear();
	m_mapWave.clear();
	m_mapEffect.clear();

	m_bFadeOut=0;

	m_BGInfo.ReleaseAll();
	m_pSelectedRMObject = NULL;
	m_pSelectedCam = NULL;
	m_pSelectedRMEffect = NULL;

	ClearAreaPool();

	/*
	int nObjectIdx=0;

	//!! 실기 관련 데이타의 클리어 루틴 작성 예정 (주의 요망) 
	// 스킨 클리어          g_BsKernel.DeleteSkin() 
	// BA 클리어 루틴들  	if (m_DoObject[nObjectIdx].m_pAniObject) 
	
	for (nObjectIdx=0; nObjectIdx<m_Object.m_nFileMax; nObjectIdx++) 
	{
		Clear_nObjectIndex_(m_DoObject[nObjectIdx].m_nObjectIndex);
	}
	*/
	// 현재는 프로그램 종료시, 텍스쳐와 같은 리소스 들이 자동 소거 될것입니다
#ifdef _USAGE_TOOL_
	if( m_pErrMsgManager )
	{
		m_pErrMsgManager->ClearAllMsg();
	}
#endif

}

CBsRealMovie::~CBsRealMovie() 
{ 
	ReleaseEventMap();
	Clear(); 

#ifdef _USAGE_TOOL_
	if( m_pErrMsgManager )
	{
        delete m_pErrMsgManager;
		m_pErrMsgManager = NULL;
	}
#endif

	ReleaseTempBuffer();
} 

void CBsRealMovie::DeleteObject(char* szObjName)
{
	if( szObjName != NULL )	
	{
		//!! 현재 루틴이 구성되어 있지 않아서, 호출만 되고 제대로 동작 하지 않습니다 (주의 요망)
		CRMObject* pObj = GetObject( szObjName );

		if( pObj )	// 해당 오브젝트 존재시 삭제.
		{
			delete pObj;
			m_mapObject.erase( szObjName );
		}
	}
}

int		CBsRealMovie::DeleteWave(char* szWavName)
{
	if( !szWavName )
		return 0;

	CRMWave* pWav = GetWave( szWavName );

	if( pWav )
	{
		delete pWav;
		m_mapWave.erase( szWavName );
		return 1;
	}

	return 0;
}

int		CBsRealMovie::DeleteCamera(char* szCamName)
{
	if( !szCamName )
		return 0;

	CRMCamera* pCam = GetCamera( szCamName );

	if( m_pSelectedCam == pCam )
	{
		m_pSelectedCam = NULL;
	}

	if( pCam )
	{
		vecCamera::iterator it = m_vecCamera.begin();
		vecCamera::iterator itEnd = m_vecCamera.end();

		for( ; it != itEnd ; ++it )
		{
			if( (*it) == pCam )
				m_vecCamera.erase( it );
		}

		delete pCam;
		m_mapCamera.erase( szCamName );
		return 1;
	}

	return 0;
}

int		CBsRealMovie::DeleteCameraPath(char* szCamName,int nIndex)
{
	if( !szCamName )
		return 0;

	CRMCamera* pCam = GetCamera( szCamName );

	if( pCam )
	{
		return pCam->DeletePath( nIndex );
	}

	return -1;
}

int		CBsRealMovie::DeleteEffect(char* szEffName)
{
	if( !szEffName )
		return 0;

	CRMEffect* pEff = GetEffect( szEffName );

	if( pEff )
	{
		delete pEff;
		m_mapEffect.erase( szEffName );
		return 1;
	}

	return 0;
}

//------------------------------------------------------------------------------------------------------------------------

// Ret : 0 - 이미 있다, 1 - 
CRMObject*	CBsRealMovie::InsertObject(char* szName)
{
	int nRet;
	int nSize = m_mapObject.size();
	char szTempName[eFileNameLength];
	CRMObject*	pTmpObj;

	if( szName )
	{
		strcpy( szTempName, szName );
		pTmpObj = GetObject( szTempName );
		nSize = 1;

		while( pTmpObj )
		{
			
			sprintf( szTempName, "%s%03d" ,szName, nSize );
			pTmpObj = GetObject( szTempName );
			++nSize;
		}
	}
	else
	{
		do 
		{
			sprintf( szTempName, "NewObject%03d" , nSize );
			pTmpObj = GetObject( szTempName );
			++nSize;

		} while( pTmpObj );
	}

	if( pTmpObj == NULL )	// 없다.
	{
		std::pair< std::map< std::string, CRMObject* >::iterator, bool > pr;

		pTmpObj = new CRMObject;

		pr = m_mapObject.insert( pairStrObj( szTempName, pTmpObj ) );

		if( pr.second == true )	// 정상적으로 생성.
		{
			CRMObject* pNewObject;

			pNewObject = (pr.first)->second;

			pNewObject->SetName( szTempName );
			pNewObject->SetSkinFileName( "empty.skin" );
			// pNewObject->SetBaFileName( //"S1.ba" );				//"empty.ba" );

			ChdirStart();
			Chdir( "RealMovie" );
			nRet = pNewObject->CreateNewObject();
			Chdir( ".." );
			ChdirEnd();

#ifdef _USAGE_TOOL_
			
			if( nRet == - 1 )
				ErrorNotFile( pNewObject->GetSkinFileName() );
			else if( nRet == -2 )
				ErrorNotFile( pNewObject->GetBaFileName() );
#endif

			return pNewObject;
		}

		return NULL;
	}

	return NULL;
}

CRMObject*	CBsRealMovie::InsertEmptyObject(char* szName)
{
	std::pair< std::map< std::string, CRMObject* >::iterator, bool > pr;
	CRMObject*	pTmpObj = new CRMObject;

	pr = m_mapObject.insert( pairStrObj( szName, pTmpObj ) );

	if( pr.second == true )	// 정상적으로 생성.
	{
		CRMObject* pNewObject;

		pNewObject = (pr.first)->second;
		pNewObject->SetName( szName );

		return pNewObject;
	}

	return NULL;
}

// Ret : -1 : 오브젝트가 없음. 0 : 동일한 이름의 Act 존재, 1 : 정상
int CBsRealMovie::InsertAct(char* szObjName,char* szActName)
{
	CRMObject* pObject = GetObject( szObjName );

	if( pObject )
	{
		if( !pObject->GetAct( szActName ) )
		{
            pObject->InsertAct( szActName );
			return 1;
		}
		else
		{
			// 이미 있음.
			return 0;
		}
	}

	return -1;
}

int CBsRealMovie::InsertLip(char* szObjName,char* szLipName)
{
	CRMLip*	pNewLip;
	CRMObject* pObject = GetObject( szObjName );

	if( pObject )
	{
		if( !pObject->GetLip( szLipName ) )
		{
			pNewLip = pObject->InsertLip( szLipName );

			/*
			pNewLip->SetLTFFileName( "empty.ltf" );
			pNewLip->SetWavFileName( "empty.wav" );

			ChdirStart();
			pNewLip->LoadLipResource();
			ChdirEnd();
			*/

			return 1;
		}
		else
		{
			// 이미 있음.
			return 0;
		}
	}

	return -1;

	/*
	if (nTreeIndex!=-1) return -1; // 이후 확장될 가능성이 있습니다
	if (nObject01<0 || nObject01>=m_Object.m_nFileMax) return -1;
	if (!Is_nObjID( m_Object.m_nID[nObject01] )) return -1;

	int nIdx = m_Lip.m_nFileMax;
	if (nIdx<0 || nIdx>=eLipMax) return -1;

	strcpy(m_Lip.m_szLTFFileName[nIdx], "empty.ltf");
	strcpy(m_Lip.m_szWAVFileName[nIdx], "empty.wav");
	m_Lip.m_nObjectID[nIdx]=m_Object.m_nID[nObject01];
	m_Lip.m_fTime[nIdx]=-1;

	ChdirStart();
	m_DoLip[nIdx].m_LTF.FLoad_text_(m_Lip.m_szLTFFileName[nIdx]);
	m_DoLip[nIdx].m_nEvent=0;
	ChdirEnd();

	m_Lip.m_nFileMax++;

	return m_Lip.m_nFileMax-1;
	*/
}

int CBsRealMovie::InsertFace(char* szObjName,char* szFaceName)
{
	CRMObject* pObject = GetObject( szObjName );

	if( pObject )
	{
		if( !pObject->GetFace( szFaceName ) )
		{
            pObject->InsertFace( szFaceName );
			return 1;
		}
		else
		{
			// 이미 있음.
			return 0;
		}
	}

	return -1;
}

int CBsRealMovie::InsertBone(char* szObjName,char* szBoneName)
{
	CRMObject* pObject = GetObject( szObjName );

	if( pObject )
	{
		if( !pObject->GetBone( szBoneName ) )
		{
			pObject->InsertBone( szBoneName );
			return 1;
		}
		else
		{
			return 0;	// 이미 있음.
		}
	}

	return -1;
}

CRMEffect*	 CBsRealMovie::InsertEff(char* szEffName)
{
	int nSize = GetEffectCount();
	char szTempName[eFileNameLength];
	CRMEffect* pTmpEffect;

	if( szEffName == NULL || szEffName[0] == NULL) //aleksger - combining similar code.
	{
		do 
		{
			sprintf( szTempName, "NewEffect%03d" , nSize );
			pTmpEffect = GetEffect( szTempName );
			++nSize;
		} while( pTmpEffect );
	}
	else
	{
            strncpy( szTempName, szEffName, eFileNameLength );
	}

	std::pair< std::map< std::string, CRMEffect* >::iterator, bool > pr;

	pTmpEffect = new CRMEffect; // aleksger: prefix bug 377: Failure to insert will lead to memory leak.

	pr = m_mapEffect.insert( pairStrEff( szTempName, pTmpEffect) );

	if( pr.second == true ) // 정상
	{
		CRMEffect* pNewEffect;
		pNewEffect = (pr.first)->second;
		pNewEffect->SetName( szTempName );
		return pNewEffect;
	}
	else {
		delete pTmpEffect;
		BsAssert(!"Failed to insert an effect as one already exists.");
		return NULL;
	}
	/*
	if (nTreeIndex!=-1) return -1; // 이후 확장될 가능성이 있습니다
	
	int nIdx = m_Effect.m_nFileMax;
	if (nIdx<0 || nIdx>=eEffectMax) return -1;

	m_Effect.m_szFileName[nIdx][0]=0;
	m_Effect.m_nType[nIdx]=eNot; // 비어 있는 값
	m_Effect.m_fTime[nIdx]=-1;
	m_Effect.m_fUseTime[nIdx]=1.5f;

	m_Effect.m_nFileMax++;

	return m_Effect.m_nFileMax-1;
	*/
}

// 사실상 szWaveName 은 사용하지 않음.
// load 시 고려.
CRMWave*	CBsRealMovie::InsertWave(char* szWaveName)
{
	int nSize = GetWaveCount();
	char szTempName[eFileNameLength];
	CRMWave* pTmpWave;

	if( !szWaveName ||szWaveName[0] == NULL ) //aleksger - combining similar code.
	{
		do 
		{
			sprintf( szTempName, "NewWave%03d" , nSize );
			pTmpWave = GetWave( szTempName );
			++nSize;
		} while( pTmpWave );
	}
	else
	{
            strncpy( szTempName, szWaveName, eFileNameLength );
	}

	std::pair< std::map< std::string, CRMWave* >::iterator, bool > pr;

	pTmpWave = new CRMWave; // aleksger: prefix bug 373: Failure to insert will lead to memory leak.

	pr = m_mapWave.insert( pairStrWav( szTempName, pTmpWave ) );

	if( pr.second == true ) // 정상
	{
		CRMWave* pNewWave;
		pNewWave = (pr.first)->second;
		pNewWave->SetName( szTempName );
		return pNewWave;
	}
	else {
		delete pTmpWave;
		BsAssert(!"Failed to insert a wave as one already exists.");
		return NULL;
	}

	/*
	if (nTreeIndex!=-1) return -1; // 이후 확장될 가능성이 있습니다
	if (nObject01_1!=-1) {
		if (nObject01_1<0 || nObject01_1>=m_Object.m_nFileMax) return -1;
	    if (!Is_nObjID( m_Object.m_nID[nObject01_1] )) return -1;
	}

	int nIdx = m_Wave.m_nFileMax;
	if (nIdx<0 || nIdx>=eWaveMax) return -1;

	m_Wave.m_szFileName[nIdx][0]=0;
	m_Wave.m_szName    [nIdx][0]=0;

	m_Wave.m_nObjectID [nIdx] = (nObject01_1!=-1) ? m_Object.m_nID[nObject01_1] : -1;
	m_Wave.m_Xyz       [nIdx].x=0;
	m_Wave.m_Xyz	   [nIdx].y=0;
	m_Wave.m_Xyz	   [nIdx].z=0;
	m_Wave.m_fFadeIn   [nIdx][0]=0;
	m_Wave.m_fFadeOut  [nIdx][0]=0;
	m_Wave.m_bLoop     [nIdx]=0;
	m_Wave.m_fVolume   [nIdx]=1;
	m_Wave.m_fTime     [nIdx]=0;
	m_Wave.m_bDisable  [nIdx]=0;

	m_Wave.m_nFileMax++;

	return m_Wave.m_nFileMax-1;
	*/
}

CRMCamera*	 CBsRealMovie::InsertCamera(char* szCamName)
{
	int nSize = GetCamMapCount();
	char szTempName[eFileNameLength];
	CRMCamera* pTmpCam;

	if( szCamName == NULL || szCamName[0] == NULL) //aleksger - similar actions for two tests combined.
	{
		do 
		{
			sprintf( szTempName, "NewCamera%03d" , nSize );
			pTmpCam = GetCamera( szTempName );
			++nSize;
		} while( pTmpCam );
	}
	else
	{
        strncpy( szTempName, szCamName, eFileNameLength );
	}

	std::pair< std::map< std::string, CRMCamera* >::iterator, bool > pr;

	pTmpCam = new CRMCamera; // aleksger: prefix bug 370: Failure to insert will lead to memory leak.

	pr = m_mapCamera.insert( pairStrCam( szTempName, pTmpCam ) );

	if( pr.second == true ) // 정상
	{
		CRMCamera* pNewCam;
		pNewCam = (pr.first)->second;
		pNewCam->SetName( szTempName );

#ifdef _USAGE_TOOL_
		if( !GetDummyLoadFlag() )
		{
            ChdirStart();
            pNewCam->CreateCamObject();
            ChdirEnd();
		}
#else
		ChdirStart();
		pNewCam->CreateCamObject();
		ChdirEnd();
#endif

		// 순차 접근을 빠르게 하기 위한 입력.
		m_vecCamera.push_back( pNewCam );
		return pNewCam;
	}
	else 
	{
		delete pTmpCam; // aleksger: prefix bug 370: Failure to insert will lead to memory leak.
		BsAssert( !"Failed to insert a camera as one already exists.");
		return NULL;
	}

	

	/*
	if (nTreeIndex!=-1) return -1; // 이후 확장될 가능성이 있습니다

	int nIdx = 0;
	if (nIdx<0 || nIdx>=eCameraMax) return -1;

	m_Camera.m_szFileName[nIdx][0]=0;
	m_Camera.m_szName[nIdx][0]=0;

	m_Camera.m_fTime[nIdx]=-1;

	m_Camera.m_nFileMax=1;

	return m_Camera.m_nFileMax-1;
	*/
}

int		CBsRealMovie::ChangeObjName(char* szOldName,char *szNewName)
{
	std::pair< std::map< std::string, CRMObject* >::iterator, bool > pr;
	CRMObject* pObj = GetObject( szOldName );

	if( pObj )
	{
		// map에서 지우기.
		m_mapObject.erase( szOldName );
		// map에 다시 넣기.

		pObj->SetName( szNewName );
		pr = m_mapObject.insert( pairStrObj( szNewName , pObj ) );

		if( pr.second == true )
			return 1;
	}

	return 0;
}

int		CBsRealMovie::ChangeWaveName(char* szOldName,char* szNewName)
{
	std::pair< std::map< std::string, CRMWave* >::iterator, bool > pr;
	CRMWave*	pWave = GetWave( szOldName );

	if( pWave )
	{
		m_mapWave.erase( szOldName );

		pWave->SetName( szNewName );	// 새 이름으로 갱신
		pr = m_mapWave.insert( pairStrWav( szNewName, pWave ) );

		if( pr.second == true )
			return 1;
	}

	return 0;
}

int		CBsRealMovie::ChangeCameraName(char* szOldName,char* szNewName)
{
	std::pair< std::map< std::string, CRMCamera* >::iterator, bool > pr;
	CRMCamera*	pCamera = GetCamera( szOldName );

	if( pCamera )
	{
		m_mapCamera.erase( szOldName );

		pCamera->SetName( szNewName );	// 새 이름으로 갱신
		pr = m_mapCamera.insert( pairStrCam( szNewName, pCamera ) );

		if( pr.second == true )
			return 1;
	}

	return 0;
}

int		CBsRealMovie::ChangeEffectName(char* szOldName,char* szNewName)
{
	std::pair< std::map< std::string, CRMEffect* >::iterator, bool > pr;
	CRMEffect*	pEffect = GetEffect( szOldName );

	if( pEffect )
	{
		m_mapEffect.erase( szOldName );

		pEffect->SetName( szNewName );
		pr = m_mapEffect.insert( pairStrEff( szNewName, pEffect ) );

		if( pr.second == true )
			return 1;
	}

	return 0;
}


int		CBsRealMovie::ChangeActName(char* szObjName,char* szOldName,char *szNewName)
{
	CRMObject* pObj = GetObject( szObjName );

	if( pObj )
		return pObj->ChangeActName( szOldName, szNewName );

	return 0;
}

int		CBsRealMovie::ChangeLipName(char* szObjName,char* szOldName,char *szNewName)
{
	CRMObject* pObj = GetObject( szObjName );

	if( pObj )
		return pObj->ChangeLipName( szOldName, szNewName );

	return 0;
}

int		CBsRealMovie::ChangeFaceName(char* szObjName,char* szOldName,char *szNewName)
{
	CRMObject* pObj = GetObject( szObjName );

	if( pObj )
		return pObj->ChangeFaceName( szOldName, szNewName );

	return 0;
}

int		CBsRealMovie::ChangeBoneName(char* szObjName,char* szOldName,char *szNewName)
{
	CRMObject* pObj = GetObject( szObjName );

	if( pObj )
		return pObj->ChangeBoneName( szOldName, szNewName );

	return 0;
}

//------------------------------------------------------------------------------------------------------------------------

char *CBsRealMovie::GetSKFilenameBuffer(char* szObjName)
{
	CRMObject* pObj = GetObject( szObjName );
	if( pObj )
		return pObj->GetSkinFileName();
	return NULL;
}

char *CBsRealMovie::GetBAFilenameBuffer(char* szObjName)
{
	CRMObject* pObj = GetObject( szObjName );
	if( pObj )
		return pObj->GetBaFileName();
	return NULL;
}

char* CBsRealMovie::GetLipFilenameBuffer(char* szObjName,char* szLipName)
{
	CRMObject* pObj = GetObject( szObjName );

	if( pObj )
	{
		CRMLip* pLip = pObj->GetLip( szLipName );

		if( pLip )
			return pLip->GetLTFFileName();
	}

	return NULL;
}

char* CBsRealMovie::GetLipWaveNameBuffer(char* szObjName,char* szLipName)
{
	CRMObject* pObj = GetObject( szObjName );

	if( pObj )
	{
		CRMLip* pLip = pObj->GetLip( szLipName );

		if( pLip )
			return pLip->GetWavFileName();
	}

	return NULL;
}

//------------------------------------------------------------------------------------------------------------------------

float CBsRealMovie::GetEvent(int &nLipIndex, int &nObject01, float fCurrentTime)
{
	/*
	nLipIndex=-1;
	nObject01=-1;
	float fMin=999;

	for (int i=0; i<m_Lip.m_nFileMax; i++) {
		float fVal = m_Lip.m_fTime[i]-fCurrentTime;

		if (m_DoLip[i].m_nEvent!=2 && fVal<fMin) {
			fMin=fVal;
			nLipIndex=i;
		}
	}

	if (nLipIndex!=-1 && m_DoLip[nLipIndex].m_nEvent != 1) nLipIndex=-1;

	if (nLipIndex!=-1) {
		nObject01 = Get_nObject01(m_Lip.m_nObjectID[nLipIndex]);

		return m_Lip.m_fTime[nLipIndex]; // 통지시 .. PlayedWaveEvent() 호출이 되어야 하고 .. 그 경우 1 번만 통지 될것입니다 (그렇지 않을때는 계속 통지 됩니다)
	}
	else {
		return 0;
	}
	*/


	return 0.f;
}

void CBsRealMovie::PlayedEvent(int nLipIndex)
{
	BsAssert(nLipIndex>=0 && "bad index");

	// m_DoLip[nLipIndex].m_nEvent=2; // 1 번만 통지 하기 위해서 .. 이벤트를 받았다고 알려 쥽니다
}

void CBsRealMovie::ResetEvent(float fCurrentTime)
{
	/*
	for (int i=0; i<m_Lip.m_nFileMax; i++) {
		if (m_Lip.m_fTime[i] >= fCurrentTime) {
			m_DoLip[i].m_nEvent=0;
		}
	}
	*/
}

//------------------------------------------------------------------------------------------------------------------------

int CBsRealMovie::GetEffEvent(float fCurrentTime)
{
	/*
	for (int i=0; i<m_Effect.m_nFileMax; i++) { 
		float fTime_=fCurrentTime;
		if (fTime_<m_Effect.m_fTime[i] || m_Effect.m_fTime[i]<0) continue;

		if (m_Effect.m_nType[i]==CRMEffect::eTerminate) return 1; // 터미네이터 이벤트 일때 1 을 리턴 합니다
	}
	*/

	if( m_fTerminateTime > 0.f && fCurrentTime >= m_fTerminateTime && fCurrentTime > 0.f )
	{
		m_fTerminateTime = -1.f;
		return 1;
	}

	return 0;
}

void	CBsRealMovie::SetTerminateTime( float fTime )
{
	m_fTerminateTime = fTime;
}

int		CBsRealMovie::GetLastEvent( msgRMTNotify* pRMTNotify )
{
	// m_dwTerminateTick 이용

	int nType;
	VOID* pObjPtr;
	int nRetCnt = 0;
	int nCnt = EventNotify( 0.f , ( m_dwTerminateTick - 1 ) );

	for( int i = 0 ; i < nCnt ; ++i )
	{
		pObjPtr = GetSelectedEvent( i , nType );

		switch( nType )
		{
		case RMTYPE_EFFECT :
			{
				int nType = ((CRMEffect*)pObjPtr)->m_nType;

				switch( nType )
				{
				case eSetTroop :
					{
						pRMTNotify[i].nType = RMTYPE_EFFECT_SETTROOP;	 // 오브젝트 Show On/Off 알림.
						pRMTNotify[i].dwParam1 = (DWORD)pObjPtr;
						pRMTNotify[i].dwParam2 = (DWORD)((CRMEffect*)pObjPtr)->GetFileName(); // Troop Name
						pRMTNotify[i].dwParam3 = (DWORD)((CRMEffect*)pObjPtr)->GetStrParam(); // Area Name
						pRMTNotify[i].dwParam4 = (DWORD)((CRMEffect*)pObjPtr)->GetParam(0);	// Troop Dir
						pRMTNotify[i].dwParam5 = (DWORD)((CRMEffect*)pObjPtr)->GetParam(1);	//0-Disable,1-Enable
						++nRetCnt;
					}
					break;
				}
			}
		}
	}

	return nRetCnt;
}

//------------------------------------------------------------------------------------------------------------------------

float CBsRealMovie::GetWaveEvent(int &nWaveIndex, int &nObject01, float fCurrentTime)
{
	/*
	nWaveIndex=-1;
	nObject01=-1;
	float fMin=999;

	for (int i=0; i<m_Wave.m_nFileMax; i++) {
		float fVal = m_Wave.m_fTime[i]-fCurrentTime;

		if (m_DoWave[i].m_nEvent!=2 && fVal<fMin) {
			fMin=fVal;
			nWaveIndex=i;
		}
	}

	if (nWaveIndex!=-1 && m_DoWave[nWaveIndex].m_nEvent != 1) nWaveIndex=-1;

	if (nWaveIndex!=-1) {
		nObject01 = Get_nObject01(m_Wave.m_nObjectID[nWaveIndex]);

		return m_Wave.m_fTime[nWaveIndex]; // 통지시 .. PlayedWaveEvent() 호출이 되어야 하고 .. 그 경우 1 번만 통지 될것입니다 (그렇지 않을때는 계속 통지 됩니다)
	}
	else {
		return 0;
	}
	*/

	return 0;
}

void CBsRealMovie::PlayedWaveEvent(int nWaveIndex)
{
	BsAssert(nWaveIndex>=0 && "bad index");
	// m_DoWave[nWaveIndex].m_nEvent=2; // 1 번만 통지 하기 위해서 .. 이벤트를 받았다고 알려 쥽니다
}

void CBsRealMovie::ResetWaveEvent(float fCurrentTime)
{
	/*
	for (int i=0; i<m_Wave.m_nFileMax; i++) {
		if (m_Wave.m_fTime[i] >= fCurrentTime) {
			m_DoWave[i].m_nEvent=0;
		}
	}
	*/
}

#ifdef _USAGE_TOOL_

int		CBsRealMovie::FindEndOfMovieTime(VOID)
{
	iteratorRMEffect it;
	iteratorRMEffect itEnd = m_mapEffect.end();
	CRMEffect*	pEffect = NULL;
	int nTemp;
	int nTime = -1;

	for( it = m_mapEffect.begin() ; it != itEnd ; ++it )
	{
		if( it->second->GetEffectType() == eTerminate )
		{
			nTemp = (int)it->second->GetStartTick();
			if( nTemp > nTime )
				nTime = nTemp;
		}
	}

	return nTime;
}

#endif


#ifndef _XBOX

void	CBsRealMovie::DrawCamLines(void)
{
	vecCamera::iterator it = m_vecCamera.begin();
	vecCamera::iterator itEnd = m_vecCamera.end();

	for( ; it != itEnd ; ++it )
	{
		(*it)->DrawCamLine();
	}
}

void	CBsRealMovie::DrawCamPivot(void)
{
	vecCamera::iterator it = m_vecCamera.begin();
	vecCamera::iterator itEnd = m_vecCamera.end();

	for( ; it != itEnd ; ++it )
	{
		(*it)->DrawCamPivotLine();
	}

	/*
	if( m_vecCamera.size() )
	{
		(*m_vecCamera.begin())->DrawCamPivotLine();
	}
	*/
}

void CBsRealMovie::OnInitCommander(void)
{
    // m_BRMCommander;

	m_BRMCommander.clear();
	
	m_BRMCommander.insert( pairIntFunc( RM_INSERT_OBJ, CBsRealMovie::OnInsertNewObj ) );
	m_BRMCommander.insert( pairIntFunc( RM_INSERT_ACT, CBsRealMovie::OnInsertNewAct ) );
	m_BRMCommander.insert( pairIntFunc( RM_INSERT_LIP, CBsRealMovie::OnInsertNewLip ) );
	m_BRMCommander.insert( pairIntFunc( RM_INSERT_FACE, CBsRealMovie::OnInsertNewFace ) );
	m_BRMCommander.insert( pairIntFunc( RM_INSERT_BONE, CBsRealMovie::OnInsertNewBone ) );

	m_BRMCommander.insert( pairIntFunc( RM_INSERT_WAVE, CBsRealMovie::OnInsertNewWave ) );
	m_BRMCommander.insert( pairIntFunc( RM_INSERT_CAMERA, CBsRealMovie::OnInsertNewCam ) );
	m_BRMCommander.insert( pairIntFunc( RM_INSERT_EFFECT, CBsRealMovie::OnInsertNewEffect ) );
	m_BRMCommander.insert( pairIntFunc( RM_INSERT_PARTICLE, CBsRealMovie::OnInsertNewParticle ) );

	m_BRMCommander.insert( pairIntFunc( RM_GET_NODE_PTR, CBsRealMovie::OnGetNodePtr ) );
	m_BRMCommander.insert( pairIntFunc( RM_SET_TIME_BY_NODEPTR, CBsRealMovie::OnSetTimeByNodePtr ) );
	m_BRMCommander.insert( pairIntFunc( RM_GET_NODE_NAME_BY_NODEPTR, CBsRealMovie::OnGetNodeNameByNodePtr ) );

	m_BRMCommander.insert( pairIntFunc( RM_GET_OBJECT_MEMBER, CBsRealMovie::OnGetObjectMember ) );
	m_BRMCommander.insert( pairIntFunc( RM_GET_ACT_MEMBER, CBsRealMovie::OnGetActMember ) );
	m_BRMCommander.insert( pairIntFunc( RM_GET_LIP_MEMBER, CBsRealMovie::OnGetLipMember ) );
	m_BRMCommander.insert( pairIntFunc( RM_GET_FACE_MEMBER, CBsRealMovie::OnGetFaceMember ) );
	m_BRMCommander.insert( pairIntFunc( RM_GET_BONE_MEMBER, CBsRealMovie::OnGetBoneMember ) );

	m_BRMCommander.insert( pairIntFunc( RM_GET_WAVE_MEMBER, CBsRealMovie::OnGetWaveMember ) );
	m_BRMCommander.insert( pairIntFunc( RM_GET_CAMERA_MEMBER, CBsRealMovie::OnGetCameraMember ) );
	m_BRMCommander.insert( pairIntFunc( RM_GET_EFFECT_MEMBER, CBsRealMovie::OnGetEffectMember ) );



	m_BRMCommander.insert( pairIntFunc( RM_GET_OBJ_PTR, CBsRealMovie::OnGetObjPtr ) );
	m_BRMCommander.insert( pairIntFunc( RM_GET_ACT_PTR, CBsRealMovie::OnGetActPtr ) );
	m_BRMCommander.insert( pairIntFunc( RM_GET_LIP_PTR, CBsRealMovie::OnGetLipPtr ) );
	m_BRMCommander.insert( pairIntFunc( RM_GET_FACE_PTR, CBsRealMovie::OnGetFacePtr ) );
	m_BRMCommander.insert( pairIntFunc( RM_GET_BONE_PTR, CBsRealMovie::OnGetBonePtr ) );
	m_BRMCommander.insert( pairIntFunc( RM_GET_WAVE_PTR, CBsRealMovie::OnGetWavePtr ) );
	m_BRMCommander.insert( pairIntFunc( RM_GET_CAMERA_PTR, CBsRealMovie::OnGetCameraPtr ) );
	m_BRMCommander.insert( pairIntFunc( RM_GET_EFFECT_PTR, CBsRealMovie::OnGetEffectPtr ) );

	m_BRMCommander.insert( pairIntFunc( RM_CHANGE_OBJ_MEMBER, CBsRealMovie::OnChangeObjMember ) );
	m_BRMCommander.insert( pairIntFunc( RM_CHANGE_ACT_MEMBER, CBsRealMovie::OnChangeActMember ) );
	m_BRMCommander.insert( pairIntFunc( RM_CHANGE_LIP_MEMBER, CBsRealMovie::OnChangeLipMember ) );
	m_BRMCommander.insert( pairIntFunc( RM_CHANGE_FACE_MEMBER, CBsRealMovie::OnChangeFaceMember ) );
	m_BRMCommander.insert( pairIntFunc( RM_CHANGE_BONE_MEMBER, CBsRealMovie::OnChangeBoneMember ) );
	m_BRMCommander.insert( pairIntFunc( RM_CHANGE_WAVE_MEMBER, CBsRealMovie::OnChangeWaveMember ) );
	m_BRMCommander.insert( pairIntFunc( RM_CHANGE_CAMERA_MEMBER, CBsRealMovie::OnChangeCameraMember ) );
	m_BRMCommander.insert( pairIntFunc( RM_CHANGE_EFFECT_MEMBER, CBsRealMovie::OnChangeEffectMember ) );


	m_BRMCommander.insert( pairIntFunc( RM_DELETE_OBJ, CBsRealMovie::OnDeleteObj ) );
	m_BRMCommander.insert( pairIntFunc( RM_DELETE_ACT, CBsRealMovie::OnDeleteAct ) );
	m_BRMCommander.insert( pairIntFunc( RM_DELETE_LIP, CBsRealMovie::OnDeleteLip ) );
	m_BRMCommander.insert( pairIntFunc( RM_DELETE_FACE, CBsRealMovie::OnDeleteFace ) );
	m_BRMCommander.insert( pairIntFunc( RM_DELETE_BONE, CBsRealMovie::OnDeleteBone ) );
	m_BRMCommander.insert( pairIntFunc( RM_DELETE_WAVE, CBsRealMovie::OnDeleteWave ) );
	m_BRMCommander.insert( pairIntFunc( RM_DELETE_CAMERA, CBsRealMovie::OnDeleteCamera ) );
	m_BRMCommander.insert( pairIntFunc( RM_DELETE_CAMERA_PATH, CBsRealMovie::OnDeleteCameraPath ) );

	m_BRMCommander.insert( pairIntFunc( RM_DELETE_EFFECT, CBsRealMovie::OnDeleteEffect ) );
	m_BRMCommander.insert( pairIntFunc( RM_PRE_PLAY, CBsRealMovie::OnPrePlay ) );

	m_BRMCommander.insert( pairIntFunc( RM_COMMAND, CBsRealMovie::OnRmCommand ) );
	m_BRMCommander.insert( pairIntFunc( RM_ENVIRONMENT_CMD, CBsRealMovie::OnRmEnvironmentCmd ) );
}

#endif

void CBsRealMovie::RemoveAllBuffer(void)
{
	m_Header.Set();

	memset(&m_szPathName, 0, sizeof(m_szPathName));

	iteratorRMObject	it;
	iteratorRMObject	itEnd = m_mapObject.end();

	for( it = m_mapObject.begin() ; it != itEnd ; ++it )
	{
		delete it->second;
	}

	iteratorRMCamera	itCam = m_mapCamera.begin();
	iteratorRMCamera	itCamEnd = m_mapCamera.end();

	for( ; itCam != itCamEnd ; ++itCam )
	{
		delete itCam->second;
	}

	iteratorRMWave		itWav = m_mapWave.begin();
	iteratorRMWave		itWavEnd = m_mapWave.end();

	for( ; itWav != itWavEnd ; ++itWav )
	{
		delete itWav->second;
	}

	iteratorRMEffect	itEft = m_mapEffect.begin();
	iteratorRMEffect	itEftEnd = m_mapEffect.end();

	for( ; itEft != itEftEnd ; ++itEft )
	{
		delete itEft->second;
	}

	m_mapObject.clear();
	m_mapCamera.clear();
	m_mapWave.clear();
	m_mapEffect.clear();

	ClearLightPool();
	ClearAllPointLights();
	ClearAllPackages();

	SetDefaultLighting();

	m_BGInfo.ReleaseAll();
	m_pSelectedRMObject = NULL;
	m_pSelectedCam = NULL;
	m_pSelectedRMEffect = NULL;

	ClearAreaPool();	

#ifdef _USAGE_TOOL_
	g_BsKernel.PreKernelCommand();
	g_BsKernel.ProcessKernelCommand();
#endif
}

//------------------------------------------------------------------------------------------------------------------------

void CBsRealMovie::SetCurPath(LPCTSTR szCurPath, LPCTSTR szFilename)
{
	int nCurPath=strlen(szCurPath);      // c:/dir1/
	int nFilename=strlen(szFilename), i; // c:/dir1/dir2/fname  -->  dir2
	for (i=nFilename-1; i>=0; i--) {
		if (szFilename[i]=='/' || szFilename[i]=='\\') {
			break;
		}
	}
	
	if (i!=-1 && nFilename>nCurPath) {
		strncpy(m_szPathName, szFilename+nCurPath+1, i-nCurPath-1);
		m_szPathName[i-nCurPath-1]=0;
	}
}

//------------------------------------------------------------------------------------------------------------------------

void	CBsRealMovie::InsertEvent( DWORD dwTick, int nType, void* pEventObj )
{
	mapDwordVector::iterator it;

	it = m_mapEvent.find( dwTick );

	if( it != m_mapEvent.end() )
	{
		// 찾았다.
		// std::vector< stEvent >	EmptyVector;
		stEvent sEvent;
		sEvent.nType = nType;
		sEvent.pEventObj = pEventObj;

		it->second.push_back( sEvent );
	}
	else
	{
		std::pair< std::map< DWORD , std::vector< stEvent > >::iterator, bool > pr;
		std::vector< stEvent >	EmptyVector;

		pr = m_mapEvent.insert( std::pair< DWORD , std::vector<stEvent> >( dwTick , EmptyVector ) );

		if( pr.second == true )
		{
			mapDwordVector::iterator itNew = pr.first;
			stEvent sEvent;
			sEvent.nType = nType;
			sEvent.pEventObj = pEventObj;

			itNew->second.push_back( sEvent );
		}
	}
}

bool	CBsRealMovie::DeleteEvent( DWORD dwTick , int nType )
{
	// 시간으로 찾기.
	std::map< DWORD , std::vector< stEvent > >::iterator it = m_mapEvent.find( dwTick );

	if( it != m_mapEvent.end() )
	{
		// 이벤트 찾기.
		std::vector<stEvent>& rvecEventPool = it->second;
		std::vector<stEvent>::iterator itEvent;
		std::vector<stEvent>::iterator itEnd = rvecEventPool.end();

		for( itEvent = rvecEventPool.begin() ; itEvent != itEnd ; ++itEvent )
		{
			if( itEvent->nType == nType )
			{
				rvecEventPool.erase( itEvent );
				break;
			}
		}

		// 이벤트가 다 비었다면 삭제.
		if( !rvecEventPool.size() )
		{
            m_mapEvent.erase( it );
		}

		return true;
	}

	return false;
}

void	CBsRealMovie::MakeEventMap(vecEffWavList* pEventPool)
{
	BsAssert( pEventPool && "EventPool is Null.. " );
	// 루프 돌면서 이벤트 넣기
	iteratorRMObject it;
	iteratorRMObject itEnd = m_mapObject.end();

	for( it = m_mapObject.begin() ; it != itEnd ; ++it )
	{
		mapStrLip::iterator itLip,itLipEnd;

		itLip = it->second->GetLipMap().begin();
		itLipEnd = it->second->GetLipMap().end();

		// 일단 Lip 정보 넣기.
		for( ; itLip != itLipEnd ; ++ itLip )
		{
			// if( itLip->second->GetStartTime() > -1.f )
			if( (int)itLip->second->GetStartTick() > -1 )
			{
				InsertEvent( itLip->second->GetStartTick(), //GetStartTime(), 
					RMTYPE_LIP, (void*)itLip->second );
			}
		}

		// Face 초기화
		mapStrFace::iterator itFace,itFaceEnd;

		itFace = it->second->GetFaceMap().begin();
		itFaceEnd = it->second->GetFaceMap().end();

		for( ; itFace != itFaceEnd ; ++itFace )
		{
			itFace->second->ResetAniCursor();
		}
	}

	// Wave 넣기.
	iteratorRMWave	itW;
	iteratorRMWave	itWEnd = m_mapWave.end();

	for( itW = m_mapWave.begin() ; itW != itWEnd ; ++itW )
	{
		if( (int)itW->second->GetStartTick() > -1 )
		{
			InsertEvent( itW->second->GetStartTick(), RMTYPE_WAVE, (void*)itW->second );
		}
	}

	// Effect 찾기.
	float fTemp;
	int   nTemp;
	m_nTerminateFlag = 0;
	m_fTerminateTime = -1.f;

	iteratorRMEffect	itEft = m_mapEffect.begin();
	iteratorRMEffect	itEftEnd = m_mapEffect.end();

	for( ; itEft != itEftEnd ; ++itEft )
	{
		if( (int)itEft->second->GetStartTick() > -1 )
		{
			InsertEvent( itEft->second->GetStartTick(), RMTYPE_EFFECT, (void*)itEft->second );
		}

		if( itEft->second->GetEffectType() == eTerminate )
		{
			fTemp = itEft->second->GetStartTime();
			nTemp = itEft->second->GetStartTick();

			if( fTemp > 0.5f && nTemp != -1 )
			{
                m_fTerminateTime = (float)nTemp / 40.f; // fTemp;
                m_dwTerminateTick = (DWORD)nTemp;
			}
		}
		else if( itEft->second->GetEffectType() == eUseWave )
		{
			rmEffWavSet temp;
			iteratorRMWave itWav = m_mapWave.find( itEft->second->GetFileName() );

			if( itWav != m_mapWave.end() )
			{
				temp.dwEffectPtr = (DWORD)itEft->second;
				temp.dwWavePtr = (DWORD)itWav->second;
				pEventPool->push_back( temp );
			}
		}

		// 게임시 계산 과정 없애기 위함.
		if( itEft->second->GetEffectType() == eUseBone )
		{
			itEft->second->MakeUseBoneData();
			itEft->second->MakeAverageDegree();

			itEft->second->MakeUseBoneDataForProcess();
			itEft->second->MakeAverageDegreeForProcess();
		}
	}

	// Package 넣기.
	// 넣기 전에 시간 정리.
	ConformAllItmesTime();

	vecPackageSet::iterator itPack = m_vecPackagePool.begin();
	vecPackageSet::iterator itPackEnd = m_vecPackagePool.end();

	for( ; itPack != itPackEnd ; ++itPack )
	{
		if( (int)itPack->GetStartTick() > -1 )
		{
			// InsertEvent( itPack->GetStartTick(), RMTYPE_PACKAGE, (void*)&(*itPack) );
			for( int i = 0 ; i < CRMPackage::eItemMax ; ++i )
			{
				if( itPack->GetItemUse( i ) )
				{
					switch( itPack->GetItemType( i ) )
					{
					case CRMPackage::eRMPackageType_Effect :
						{
							CRMEffect* pEffect = GetEffect( itPack->GetItemName( i ) );

							InsertEvent( pEffect->GetStartTick(), RMTYPE_EFFECT, (void*)pEffect );

							if( pEffect->GetEffectType() == eTerminate )
							{
								// m_fTerminateTime = pEffect->GetStartTime();
								m_dwTerminateTick = pEffect->GetStartTick();
								m_fTerminateTime = (float)m_dwTerminateTick / 40.f;
							}
							else if( pEffect->GetEffectType() == eUseWave )
							{
								rmEffWavSet temp;
								iteratorRMWave itWav = m_mapWave.find( pEffect->GetFileName() );

								if( itWav != m_mapWave.end() )
								{
									temp.dwEffectPtr = (DWORD)pEffect;
									temp.dwWavePtr = (DWORD)itWav->second;
									pEventPool->push_back( temp );
								}
							}


							// 게임시 계산 과정 없애기 위함.
							if( pEffect->GetEffectType() == eUseBone )
							{
								pEffect->MakeUseBoneData();
								pEffect->MakeAverageDegree();

								pEffect->MakeUseBoneDataForProcess();
								pEffect->MakeAverageDegreeForProcess();
							}
						}
						break;
					case CRMPackage::eRMPackageType_Wave :
						{
							CRMWave* pWave = GetWave( itPack->GetItemName( i ) );

							InsertEvent( pWave->GetStartTick(), RMTYPE_WAVE, (void*)pWave );
						}
						break;
					case CRMPackage::eRMPackageType_Lip :
						{
							CRMLip* pLip = GetLip( itPack->GetItemName( i ) );

							InsertEvent( pLip->GetStartTick(), RMTYPE_LIP, (void*)pLip );
						}
						break;
					}
				}
			}
		}
	}

#ifndef _XBOX
	// 카메라 정렬?
	SortCameraWithTime();
#endif
}


#ifndef _XBOX

typedef CRMCamera*	CRMCameraPtr;

struct SortCam
{
	bool	operator()( CRMCameraPtr pCam1, CRMCameraPtr pCam2 )
	{
		return pCam1->m_fTime > pCam2->m_fTime;
	}
};

void	CBsRealMovie::SortCameraWithTime(void)
{
	SortCam sortFunc;
	std::sort( m_vecCamera.begin(), m_vecCamera.end(), sortFunc );

	return;
}

#endif

void	CBsRealMovie::ReleaseEventMap(void)
{
	m_mapEvent.clear();
}

int		CBsRealMovie::EventNotify( float fTime , DWORD dwTick )
{
	int i;
	mapDwordVector::iterator it;

	it = m_mapEvent.find( dwTick ); // fTime );

	m_nEventCnt = 0;

	if( it != m_mapEvent.end() )
	{
		std::vector<stEvent>& rvecEventPool = it->second;
		std::vector<stEvent>::iterator itEvent;
		std::vector<stEvent>::iterator itEnd = rvecEventPool.end();

		m_nEventCnt = (int)rvecEventPool.size();

		for( itEvent = rvecEventPool.begin(), i = 0 ; itEvent != itEnd ; ++itEvent, ++i )
		{
			m_EventArray[i].nType = itEvent->nType;
			m_EventArray[i].pEventObj = itEvent->pEventObj;
			/*
			if( itEvent->nType == rnType )
			{
				return itEvent->pEventObj;
			}
			*/
		}
	}

	return m_nEventCnt;
}

void	CBsRealMovie::ApplyCamDurationToEffect(CRMCamera* pRMCam)
{
	//
	iteratorRMEffect it = m_mapEffect.begin();
	iteratorRMEffect itEnd = m_mapEffect.end();
	float fUseTime;

	if( pRMCam )
	{
		int nCnt = pRMCam->GetPathCnt();

		if( nCnt > 0 )
		{
			int nFrames = pRMCam->GetCumulativeFrame( nCnt - 1 );

			if( nFrames > 0 )
			{
                fUseTime = ((float)nFrames) / g_fRMFps;
			}
			else
			{
				return;
			}
		}
		else
		{
			return;	// mruete: prefix bug 394: don't let the function continue unless fUseTime was initialized
		}
	}
	else
	{
		return;
	}

	for( ; it != itEnd ; ++it )
	{
		if( it->second->IsCamType() )
		{
			CRMEffect* pEffect = it->second;

			// 연결된 같은 Cam 이라면..
			if( (CRMCamera*)pEffect->GetLinkedObjPtr() == pRMCam )
			{
				pEffect->SetUseTime( fUseTime );
			}
		}
	}
}

//------------------------------------------------------------------------------------------------------------------------
int		CBsRealMovie::GetObjList( vecString& rvecList )
{
	int nCnt = 0;
	iteratorRMObject it;
	iteratorRMObject itEnd = m_mapObject.end();

	// Act이름-Object이름 :: 형식으로 만들기.
	for( it = m_mapObject.begin() ; it != itEnd ; ++it )
	{
		rvecList.push_back( it->first );
		++nCnt;
	}

	return nCnt;
}

int		CBsRealMovie::GetActList( vecString& rvecList )
{
	int nCnt = 0;
	iteratorRMObject it;
	iteratorRMObject itEnd = m_mapObject.end();

	// Act이름-Object이름 :: 형식으로 만들기.
	for( it = m_mapObject.begin() ; it != itEnd ; ++it )
	{
		nCnt += it->second->GetActNameList( rvecList );
	}

	return nCnt;
}

int		CBsRealMovie::GetLipList( vecString& rvecList )
{
	int nCnt = 0;
	iteratorRMObject it;
	iteratorRMObject itEnd = m_mapObject.end();

	for( it = m_mapObject.begin() ; it != itEnd ; ++it )
	{
		nCnt += it->second->GetLipNameList( rvecList );
	}

	return nCnt;
}

int		CBsRealMovie::GetFaceList( vecString& rvecList )
{
	int nCnt = 0;
	iteratorRMObject it;
	iteratorRMObject itEnd = m_mapObject.end();

	for( it = m_mapObject.begin() ; it != itEnd ; ++it )
	{
		nCnt += it->second->GetFaceNameList( rvecList );
	}

	return nCnt;
}

int		CBsRealMovie::GetBoneList( vecString& rvecList )
{
	int nCnt = 0;
	iteratorRMObject it;
	iteratorRMObject itEnd = m_mapObject.end();

	for( it = m_mapObject.begin() ; it != itEnd ; ++it )
	{
		nCnt += it->second->GetBoneNameList( rvecList );
	}

	return nCnt;
}

int		CBsRealMovie::GetWaveList( vecString& rvecList )
{
	int nCnt = 0;
	iteratorRMWave it;
	iteratorRMWave itEnd = m_mapWave.end();

	for( it = m_mapWave.begin() ; it != itEnd ; ++it )
	{
		rvecList.push_back( it->first );
		++nCnt;
	}

	return nCnt;
}

int		CBsRealMovie::GetCamList( vecString& rvecList )
{
	int nCnt = 0;
	iteratorRMCamera it;
	iteratorRMCamera itEnd = m_mapCamera.end();

	for( it = m_mapCamera.begin() ; it != itEnd ; ++it )
	{
		// if( it->second->GetStartTime() < 0.f )
		{
            rvecList.push_back( it->first );
            ++nCnt;
		}
	}

	return nCnt;
}

int		CBsRealMovie::GetEffectList( vecString& rvecList )
{
	int nCnt = 0;
	iteratorRMEffect it;
	iteratorRMEffect itEnd = m_mapEffect.end();

	for( it = m_mapEffect.begin() ; it != itEnd ; ++it )
	{
		if( it->second->GetStartTime() < 0.f )
		{
            rvecList.push_back( it->first );
            ++nCnt;
		}
	}

	return nCnt;
}

VOID*	CBsRealMovie::GetNodePtrByName( int nType,char *szCaption)
{
	char szTemp[130];

	switch( nType )
	{
	case RMTYPE_ACT :
		{
			// 앞(Act)::뒤(Obj)
			strncpy( szTemp, szCaption, 130 );
			char* pdest = strstr( szTemp , "::" );
			if ( !pdest )	// mruete: prefix bug 397: added check
				return NULL;
				
			*pdest = NULL;

			CRMObject* pObj = GetObject( pdest+2 );
			if( !pObj )
				return NULL;

			CRMAct* pAct = pObj->GetAct( szTemp );
			
			return (VOID*)pAct;
		}
		break;
	case RMTYPE_LIP :
		{
			// 앞(Lip)::뒤(Obj)
			strncpy( szTemp, szCaption, 130 );
			char* pdest = strstr( szTemp , "::" );
			if ( !pdest )	// mruete: prefix bug 397: added check
				return NULL;
				
			*pdest = NULL;

			CRMObject* pObj = GetObject( pdest+2 );
			if( !pObj )
				return NULL;

			CRMLip* pLip = pObj->GetLip( szTemp );

			return (VOID*)pLip;
		}
		break;
	case RMTYPE_FACE :
		{
			// 앞(Lip)::뒤(Obj)
			strncpy( szTemp, szCaption, 130 );
			char* pdest = strstr( szTemp , "::" );
			if ( !pdest )	// mruete: prefix bug 397: added check
				return NULL;
				
			*pdest = NULL;

			CRMObject* pObj = GetObject( pdest+2 );
			if( !pObj )
				return NULL;

			CRMFace* pFace = pObj->GetFace( szTemp );

			return (VOID*)pFace;
		}
		break;
	case RMTYPE_BONE :
		{
			// 앞(Bone)::뒤(Obj)
			strncpy( szTemp, szCaption, 130 );
			char* pdest = strstr( szTemp , "::" );
			if ( !pdest )	// mruete: prefix bug 397: added check
				return NULL;
				
			*pdest = NULL;

			CRMObject* pObj = GetObject( pdest+2 );
			if( !pObj )
				return NULL;

			CRMBone* pBone = pObj->GetBone( szTemp );

			return (VOID*)pBone;
		}
		break;
	case RMTYPE_WAVE :
		{
			CRMWave* pWav = GetWave( szCaption );
			return (VOID*)pWav;
		}
		break;
	case RMTYPE_CAM :
		{
			CRMCamera* pCam = GetCamera( szCaption );
			return (VOID*)pCam;
		}
		break;
	case RMTYPE_EFFECT :
		{
			CRMEffect* pEffect = GetEffect( szCaption );
			return (VOID*)pEffect;
		}
		break;
	case RMTYPE_PACKAGE :
		{
			CRMPackage* pPack = FindPackage( szCaption );
			return (VOID*)pPack;
		}
		break;
	}

	return NULL;
}


void	CBsRealMovie::ClearAreaPool(void)
{	
	m_vecAreaSetPool.clear();	
}


void	CBsRealMovie::ResizeAreaPool(int nNewSize)
{
	ClearAreaPool();
	m_vecAreaSetPool.resize( nNewSize );
}

int		CBsRealMovie::GetAreaPoolSize(void)
{
	return (int)m_vecAreaSetPool.size();
}

stAreaSet*	CBsRealMovie::GetAreaSetFromPool(int nIndex)
{
	return &(m_vecAreaSetPool[ nIndex ]);
}

stAreaSet*	CBsRealMovie::FindArea(char* szAreaName)
{
	vecAreaSet::iterator it = m_vecAreaSetPool.begin();
	vecAreaSet::iterator itEnd = m_vecAreaSetPool.end();

	for( ; it != itEnd ; ++it )
	{
		if( strcmp( it->AreaEx.szAreaName, szAreaName ) == 0 )
			return &(*it);
	}

	return NULL;
}



void	CBsRealMovie::ClearLightPool(void)
{
	m_vecLightSetPool.clear();
}

void	CBsRealMovie::ResizeLightPool(int nNewSize)
{
	ClearLightPool();
	m_vecLightSetPool.resize( nNewSize );
}

int		CBsRealMovie::GetLightPoolSize(void)
{
	return (int)m_vecLightSetPool.size();
}

stLightPack*	CBsRealMovie::GetLightSetFromPool(int nIndex)
{
	return &(m_vecLightSetPool[nIndex]);
}

stLightPack*	CBsRealMovie::FindLightSet(char* szLightName)
{
	vecLightSet::iterator it = m_vecLightSetPool.begin();
	vecLightSet::iterator itEnd = m_vecLightSetPool.end();

	for( ; it != itEnd ; ++it )
	{
		if( strcmp( it->NameBlock.szLightName, szLightName ) == 0 )
			return &(*it);
	}

	return NULL;

}


stPointLightPack*	CBsRealMovie::CreateEmptyPointLight(void)
{
	stPointLightPack	tmpPack;
	m_vecPointLightSetPool.push_back( tmpPack );
	return &(*(m_vecPointLightSetPool.end()-1));
}

BOOL	CBsRealMovie::CreateNewPointLight(char* pszLightName, const size_t pszLightName_len) //aleksger - safe string
{
	int nCnt = 0;
	char szTempName[eCaptionLength];

	do 
	{
		int nID = m_BGInfo.nPointLightID++;
		sprintf( szTempName , "PLight%02d", nID );
		++nCnt;

		if( nCnt > 5 )
		{
			return FALSE;
		}

	} while( FindPointLight( szTempName ) != NULL );

	stPointLightPack* pNewLight = CreateEmptyPointLight();

	strncpy( pNewLight->NameBlock.szLightName, szTempName , eCaptionLength );
	pNewLight->NameBlock.szLightName[ eCaptionLength - 1 ] = NULL;

	strcpy_s( pszLightName, pszLightName_len, pNewLight->NameBlock.szLightName );
	return TRUE;
}

stPointLightPack*	CBsRealMovie::FindPointLight(char* szLightName)
{
	MatchPointLightName matchName( szLightName );

	vecPointLightSet::iterator p = std::find_if( m_vecPointLightSetPool.begin(),
												m_vecPointLightSetPool.end(), 
												matchName );

	if( p != m_vecPointLightSetPool.end() )
	{
		return &(*p);
	}

	return NULL;
}

BOOL	CBsRealMovie::DeletePointLight(char* szLightName)
{
	MatchPointLightName matchName( szLightName );

	vecPointLightSet::iterator p = std::find_if( m_vecPointLightSetPool.begin(),
		m_vecPointLightSetPool.end(), 
		matchName );

	if( p != m_vecPointLightSetPool.end() )
	{
		m_vecPointLightSetPool.erase( p );
		return TRUE;
	}

	return FALSE;
}

CRMPackage*	CBsRealMovie::FindPackage(char* szPackName)
{
	MatchPackageName matchName( szPackName );

	vecPackageSet::iterator p = std::find_if( m_vecPackagePool.begin(),
											m_vecPackagePool.end(),
											matchName );

	if( p != m_vecPackagePool.end() )
	{
		return &(*p);
	}

	return NULL;
}

CRMPackage*	CBsRealMovie::CreateEmptyPackage(void)
{
	CRMPackage	tmpPack;
	m_vecPackagePool.push_back( tmpPack );
	return &(*(m_vecPackagePool.end()-1));
}

BOOL		CBsRealMovie::CreateNewPackage(char* szPackName, const size_t szPackName_len) //aleksger - safe string
{
	int nCnt = 0;
	char szTempName[eCaptionLength];

	do 
	{
		int nID = m_BGInfo.nPackageID++;
		sprintf( szTempName, "Package%02d", nID );
		++nCnt;

		if( nCnt > 5 )
		{
			return FALSE;
		}
	} while( FindPackage( szTempName ) != NULL );

	CRMPackage* pNewPack = CreateEmptyPackage();

	pNewPack->SetName( szTempName );
	strcpy_s( szPackName, szPackName_len, szTempName );
	return TRUE;
}

BOOL	CBsRealMovie::DeletePackage(char* szPackName)
{
	MatchPackageName matchName( szPackName );

	vecPackageSet::iterator p = std::find_if( m_vecPackagePool.begin(),
		m_vecPackagePool.end(),
		matchName );

	if( p != m_vecPackagePool.end() )
	{
		m_vecPackagePool.erase( p );
		return TRUE;
	}

	return FALSE;
}

void	CBsRealMovie::ConformAllItmesTime(void)
{
	vecPackageSet::iterator p = m_vecPackagePool.begin();
	vecPackageSet::iterator pEnd = m_vecPackagePool.end();

	for( ; p != pEnd ; ++p )
	{
		ConformItmesTimeToPackTimeCore( &(*p) );
	}
}

void	CBsRealMovie::ConformItmesTimeToPackTime(char* szPackName)
{
	MatchPackageName matchName( szPackName );

	vecPackageSet::iterator p = std::find_if( m_vecPackagePool.begin(),
		m_vecPackagePool.end(),
		matchName );

	if( p != m_vecPackagePool.end() )
	{
		ConformItmesTimeToPackTimeCore( &(*p) );
	}
}

void	CBsRealMovie::ConformItmesTimeToPackTimeCore(CRMPackage* pPack)
{
	for( int i = 0 ; i < CRMPackage::eItemMax ; ++i )
	{
		if( pPack->GetItemUse( i ) )
		{
			if( pPack->GetItemType( i ) == CRMPackage::eRMPackageType_Effect )
			{
				CRMEffect* pEffect = GetEffect( pPack->GetItemName( i ) );

				if( pEffect )
                    pEffect->SetStartTime( pPack->GetStartTime(), pPack->GetStartTick() );
			}
			else if( pPack->GetItemType( i ) == CRMPackage::eRMPackageType_Wave )
			{
				CRMWave* pWave = GetWave( pPack->GetItemName( i ) );

				if( pWave )
                    pWave->SetStartTime( pPack->GetStartTime(), pPack->GetStartTick() );
			}
			else if( pPack->GetItemType( i ) == CRMPackage::eRMPackageType_Lip )
			{
				CRMLip* pLip = GetLip( pPack->GetItemName( i ) );

				if( pLip )
					pLip->SetStartTime( pPack->GetStartTime(), pPack->GetStartTick() );
			}
		}
	}
}

void	CBsRealMovie::ConformItmesTimeToPackTimeCore(CRMPackage* pPack,int nIndex)
{
	if( pPack->GetItemUse( nIndex ) )
	{
		if( pPack->GetItemType( nIndex ) == CRMPackage::eRMPackageType_Effect )
		{
			CRMEffect* pEffect = GetEffect( pPack->GetItemName( nIndex ) );

			if( pEffect )
                pEffect->SetStartTime( pPack->GetStartTime(), pPack->GetStartTick() );
		}
		else if( pPack->GetItemType( nIndex ) == CRMPackage::eRMPackageType_Wave )
		{
			CRMWave* pWave = GetWave( pPack->GetItemName( nIndex ) );

			if( pWave )
                pWave->SetStartTime( pPack->GetStartTime(), pPack->GetStartTick() );
		}
		else if( pPack->GetItemType( nIndex ) == CRMPackage::eRMPackageType_Lip )
		{
			CRMLip* pLip = GetLip( pPack->GetItemName( nIndex ) );

			if( pLip )
				pLip->SetStartTime( pPack->GetStartTime(), pPack->GetStartTick() );
		}
	}
	/*
	else
	{
		if( pPack->GetItemType( nIndex ) == CRMPackage::eRMPackageType_Effect )
		{
			CRMEffect* pEffect = GetEffect( pPack->GetItemName( nIndex ) );

			if( pEffect )
                pEffect->SetStartTime( -1 , -1 );
		}
		else if( pPack->GetItemType( nIndex ) == CRMPackage::eRMPackageType_Wave )
		{
			CRMWave* pWave = GetWave( pPack->GetItemName( nIndex ) );

			if( pWave )
                pWave->SetStartTime( -1 , -1 );
		}
	}
	*/
}

void	CBsRealMovie::ResetItemsTime(CRMPackage* pPack,int nIndex)
{
	if( pPack->GetItemType( nIndex ) == CRMPackage::eRMPackageType_Effect )
	{
		CRMEffect* pEffect = GetEffect( pPack->GetItemName( nIndex ) );

		if( pEffect )
			pEffect->SetStartTime( -1 , -1 );
	}
	else if( pPack->GetItemType( nIndex ) == CRMPackage::eRMPackageType_Wave )
	{
		CRMWave* pWave = GetWave( pPack->GetItemName( nIndex ) );

		if( pWave )
			pWave->SetStartTime( -1 , -1 );
	}
	else if( pPack->GetItemType( nIndex ) == CRMPackage::eRMPackageType_Lip )
	{
		CRMLip* pLip = GetLip( pPack->GetItemName( nIndex ) );

		if( pLip )
			pLip->SetStartTime( -1, -1 );
	}
}

#ifndef _XBOX

#ifdef _USAGE_TOOL_
int		CBsRealMovie::ChangeTimeOfAllData(void)
{
	// 먼저 가벼운것 부터 시작해 보자.
	iteratorRMObject itObj = m_mapObject.begin();
	iteratorRMObject itObjEnd = m_mapObject.end();

	for( ; itObj != itObjEnd ; ++itObj )
	{
		itObj->second->ChangeTimeOfAllData();
	}

	iteratorRMWave itWave = m_mapWave.begin();
	iteratorRMWave itWaveEnd = m_mapWave.end();

	for( ; itWave != itWaveEnd ; ++itWave )
	{
		itWave->second->ChangeFPS();
	}

	iteratorRMCamera itCam = m_mapCamera.begin();
	iteratorRMCamera itCamEnd = m_mapCamera.end();

	for( ; itCam != itCamEnd ; ++itCam )
	{
		itCam->second->ChangeFPS();
	}

	iteratorRMEffect itEff = m_mapEffect.begin();
	iteratorRMEffect itEffEnd = m_mapEffect.end();

	for( ; itEff != itEffEnd ; ++itEff )
	{
		itEff->second->ChangeFPS();
	}

	vecPackageSet::iterator itPack = m_vecPackagePool.begin();
	vecPackageSet::iterator itPackEnd = m_vecPackagePool.end();

	for( ; itPack != itPackEnd ; ++itPack )
	{
		itPack->ChangeFPS();
	}

	return 0;
}
#endif



static bool	RaySphereIntersect( D3DXVECTOR3 &vsCenter , 
						   D3DXVECTOR3 &vPickRayOrig , 
						   D3DXVECTOR3 &vPickRayDir ,
						   float fRadius )
{
	//*    RTR 에 나온 방법..
	D3DXVECTOR3 vecLength;

	// l = c - o
	vecLength = vsCenter - vPickRayOrig;

	// s = l Dot d;
	float fS = D3DXVec3Dot( &vecLength , &vPickRayDir );

	// l^2 = l * l;
	float fl2 = D3DXVec3Dot( &vecLength, &vecLength ); // D3DXVec3LengthSq( &vecLength );

	// float fLPow = fl2 * fl2;
	float fRPow = fRadius * fRadius;

	// if ( s < 0 and l^2 > r^2 ) 
	if( fS < 0.f && fl2 > fRPow )
	{
		// 구 안에 없다.
		return false;
	}

	float fSPow = fS * fS;

	// m^2 = l^2 - s^2
	float fMPow = fl2 - fSPow;

	if( fMPow > fRPow )
	{
		return false;
	}


	return true;
}

CRMObject*	CBsRealMovie::CheckHitObject(D3DXVECTOR3 &vPickRayDir,
									 D3DXVECTOR3 &vPickRayOrig)
{
	iteratorRMObject it = m_mapObject.begin();
	iteratorRMObject itEnd = m_mapObject.end();

	// 이전 선택 상자 지우기.
	if( m_pSelectedRMObject )
	{
		g_BsKernel.SendMessage( 
			m_pSelectedRMObject->m_nObjectIndex, BS_SHOW_BOUNDINGBOX, (DWORD)false );
	}

	for( ; it != itEnd ; ++it )
	{
		CRMObject* pRMObject = it->second;

		AABB *pBox = NULL;
		pBox = (AABB *)g_BsKernel.SendMessage( pRMObject->m_nObjectIndex , BS_GET_BOUNDING_BOX );

		D3DXVECTOR2 vecMax( pBox->Vmax.x, pBox->Vmax.z ); // .y = 0.f;
		D3DXVECTOR2 vecMin( pBox->Vmin.x, pBox->Vmin.z ); // pBox->Vmin.y = 0.f;
		D3DXVECTOR2 vecLength = vecMax - vecMin;
		float fRadius = D3DXVec2Length( &vecLength );
		fRadius *= 0.5f;
		
		if( RaySphereIntersect( pRMObject->GetPosition(), 
			vPickRayOrig, vPickRayDir, fRadius ) )
		{
			// 찾았다.
			m_pSelectedRMObject = pRMObject;
			return pRMObject;
		}
	}

	return NULL;
}

CRMObject*	CBsRealMovie::CheckHitObjectEx(int cx,int cy,int posx,int posy,D3DXMATRIXA16* pmatViewProj)
{
	iteratorRMObject it = m_mapObject.begin();
	iteratorRMObject itEnd = m_mapObject.end();

	// D3DXVECTOR3 vecPos;		// Obj 의 위치.
	D3DXVECTOR3 scrPos;

	// 이전 선택 상자 지우기.
	if( m_pSelectedRMObject && m_pSelectedRMObject->m_nObjectIndex != -1 )
	{
		g_BsKernel.SendMessage( 
			m_pSelectedRMObject->m_nObjectIndex, BS_SHOW_BOUNDINGBOX, (DWORD)false );
	}

	for( ; it != itEnd ; ++it )
	{
		CRMObject* pRMObject = it->second;

		D3DXVec3TransformCoord( &scrPos, 
			&pRMObject->GetPosition(), 
			pmatViewProj );

		if (scrPos.z > 0.f) 
		{
			int x = int( ((scrPos.x + 1.f) * cx) / 2.f );
			int y = int( ((1.f - scrPos.y) * cy) / 2.f );

			int xsize = posx - x;
			int ysize = posy - y;
			int nLen;

			nLen = (int)sqrt( (float)((xsize*xsize) + (ysize*ysize)) );

			if( nLen < 30 )
			{
				// 찾았다.
				m_pSelectedRMObject = pRMObject;
				return pRMObject;
			}
		}
	}

	return NULL;
}


CRMCamera*	CBsRealMovie::CheckHitCameraEx(int cx,int cy,int posx,int posy,
										   D3DXMATRIXA16* pmatViewProj,D3DXVECTOR3& rPos,
										   int& rnPathIndex)
{
	iteratorRMCamera it = m_mapCamera.begin();
	iteratorRMCamera itEnd = m_mapCamera.end();

	// D3DXVECTOR3 vecPos;		// Obj 의 위치.
	D3DXVECTOR3 scrPos;
	// D3DXVECTOR3	vecCamPos;

	// 이전 선택 상자 지우기.
	if( m_pSelectedCam )
	{
		g_BsKernel.SendMessage( 
			m_pSelectedCam->m_nObjectIndex, BS_SHOW_BOUNDINGBOX, (DWORD)false );
	}

	for( ; it != itEnd ; ++it )
	{
		CRMCamera* pCamera = it->second;

		int nPCnt = pCamera->GetPathCnt();

		for( int i = 0; i < nPCnt ; ++i )
		{
			D3DXVECTOR3& rvecCamPos = pCamera->GetPathPosition( i );
			D3DXVECTOR2 vecLen( rPos.x - rvecCamPos.x, rPos.z - rvecCamPos.z );

			float fLen = D3DXVec2Length( &vecLen );

			if( fLen < 50.f )
			{
				rPos = rvecCamPos;
				rnPathIndex = i;

				m_pSelectedCam = pCamera;
				return m_pSelectedCam;
			}
		}

		/*

		D3DXVec3TransformCoord( &scrPos, 
			&pCamera->m_vecGround, 
			pmatViewProj );

		if (scrPos.z > 0.f) 
		{
			int x = int( ((scrPos.x + 1.f) * cx) / 2.f );
			int y = int( ((1.f - scrPos.y) * cy) / 2.f );

			int xsize = posx - x;
			int ysize = posy - y;
			int nLen;

			nLen = (int)sqrt( (float)((xsize*xsize) + (ysize*ysize)) );

			if( nLen < 30 )
			{
				// 찾았다.
				m_pSelectedCam = pCamera;
				return m_pSelectedCam;
			}
		}
		*/
	}

	return NULL;
}

CRMEffect*	CBsRealMovie::CheckHitEffectEx(int cx,int cy,int posx,int posy,D3DXMATRIXA16* pmatViewProj)
{
	iteratorRMEffect it = m_mapEffect.begin();
	iteratorRMEffect itEnd = m_mapEffect.end();

	// D3DXVECTOR3 vecPos;		// Obj 의 위치.
	D3DXVECTOR3 scrPos;

	// 이전 선택 상자 지우기.
	/*
	if( m_pSelectedRMObject )
	{
		g_BsKernel.SendMessage( 
			m_pSelectedRMObject->m_nObjectIndex, BS_SHOW_BOUNDINGBOX, (DWORD)false );
	}
	*/

	for( ; it != itEnd ; ++it )
	{
		CRMEffect* pRMEffect = it->second;

		if( !pRMEffect->IsMovable() )
			continue;

		D3DXVec3TransformCoord( &scrPos, 
			&pRMEffect->GetPosition(), 
			pmatViewProj );

		if (scrPos.z > 0.f) 
		{
			int x = int( ((scrPos.x + 1.f) * cx) / 2.f );
			int y = int( ((1.f - scrPos.y) * cy) / 2.f );

			int xsize = posx - x;
			int ysize = posy - y;
			int nLen;

			nLen = (int)sqrt( (float)((xsize*xsize) + (ysize*ysize)) );

			if( nLen < 30 )
			{
				// 찾았다.
				// m_pSelectedRMObject = pRMObject;
				m_pSelectedRMEffect = pRMEffect;
				return pRMEffect;
			}
		}
	}

	return NULL;
}


#endif

































//------------------------------------------------------------------------------------------------------------------------
// 외부로 연결되는 인터페이스
DWORD	CBsRealMovie::RealMovieCommand(int nMsg,DWORD dwParam1/*=0*/,DWORD dwParam2/*=0*/)
{
	// 디버그 일때만..
#ifndef _XBOX
	mapCmder::iterator it;

	it = m_BRMCommander.find( nMsg );

	if( it != m_BRMCommander.end() )
	{
		return it->second( this, dwParam1, dwParam2 );
	}
#endif

	return 0L;

	// 후에 이렇게 바뀜.
	// m_BRMCommander[ nMsg ]( this, dwParam1, dwParam2 );

	/*
	if( nMsg < RM_DEF_MSG_END )
		return RealMovieDefCommand( nMsg, dwParam1, dwParam2 );
	else if( nMsg < RM_DELETE_MSG_END )
		return RealMovieDelCommand( nMsg, dwParam1, dwParam2 );
	return -1L;
	*/
}

DWORD	CBsRealMovie::RealMovieDefCommand(int nMsg,DWORD dwParam1/*=0*/,DWORD dwParam2/*=0*/)
{
	switch( nMsg )
	{
	case RM_APPLY_EFFECTFILE :
		{
			// m_Effect.m_szFileName[ (int)dwParam1 ];	// 적용 시켜야 함.
			return 1L;
		}
		break;
	case RM_RESET_TIME_ACT :
		{
			char* szObjName = (char*)dwParam1;
			char* szActName = (char*)dwParam2;

			m_mapObject[ szObjName ]->GetAct( szActName )->SetStartTime( -1.f, -1 );
			// m_Act.m_fTime[dwParam1]=-1;
			return 1L;
		}
		break;
	case RM_RESET_TIME_LIP :
		{
			// m_Lip.m_fTime[dwParam1]=-1;
			char* szObjName = (char*)dwParam1;
			char* szLipName = (char*)dwParam2;

			m_mapObject[ szObjName ]->GetLip( szLipName )->SetStartTime( -1.f, -1 );
			return 1L;
		}
		break;
	case RM_RESET_TIME_FACE :
		{
			// m_Face.m_fTime[dwParam1]=-1;
			char* szObjName = (char*)dwParam1;
			char* szFaceName = (char*)dwParam2;

			m_mapObject[ szObjName ]->GetFace( szFaceName )->SetStartTime( -1.f, -1 );
			return 1L;
		}
		break;
	case RM_RESET_TIME_WAVE :
		{
			// m_Wave.m_fTime[dwParam1]=-1;
			char* szWaveName = (char*)dwParam1;

			m_mapWave[ szWaveName ]->SetStartTime( -1.f , -1 );
			return 1L;
		}
		break;
	case RM_RESET_TIME_CAMERA :
		{
			// m_Camera.m_fTime[dwParam1]=-1;
			char* szCamName = (char*)dwParam1;
			m_mapCamera[ szCamName ]->SetStartTime( -1.f );
			return 1L;
		}
		break;
	case RM_RESET_TIME_EFF :
		{
			// m_Effect.m_fTime[dwParam1]=-1;  // 타임 리셋 (유즈 타임 은 유지)
			char* szEffName = (char*)dwParam1;
			m_mapEffect[ szEffName ]->SetStartTime( -1.f, -1 );
			return 1L;
		}
		break;

	case RM_SET_DISABLE_WAVE :
		{
			// m_Wave.m_bDisable[dwParam1] = dwParam2;	
			char* szWaveName = (char*)dwParam1;
			m_mapWave[szWaveName ]->SetDisable( (int)dwParam2 );
		}
		break;
	case RM_SET_DISABLE_EFF :
		{
			// m_Effect.m_bDisable[dwParam1] = dwParam2;
			char* szEffName = (char*)dwParam1;
			m_mapEffect[ szEffName ]->SetDisable( (int)dwParam2 );
		}
		break;
	}

	return 0L;
}

DWORD	CBsRealMovie::RealMovieDelCommand(int nMsg,DWORD dwParam1/*=0*/,DWORD dwParam2/*=0*/)
{
	return 0L;
}

#ifndef _XBOX

DWORD CBsRealMovie::OnEmptyFunc(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	MessageBox( NULL, "아직 준비가 안된 명령어가 입력 되었습니다.","Error", MB_OK );
	return 0L;
}

DWORD CBsRealMovie::OnPrePlay(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	pObj->ReleaseEventMap();
	pObj->MakeEventMap( (vecEffWavList*)dwParam1 );

	pObj->SyncResources();
	return 0L;
}

// dwParam1 : char *pszName
DWORD CBsRealMovie::OnInsertNewObj(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	// 동일한 이름 있나 채크.
	// if( !dwParam1 )	return 0L;
	rmMsgItem* pParam = (rmMsgItem*)dwParam1;
	char* szRetNameBuf = (char*)dwParam2;

	if( pParam )
	{
		CRMObject* pNewObj = pObj->InsertObject( pParam->szObjName );
		strcpy( szRetNameBuf , pNewObj->GetName() );
		pParam->dwRetParam = (DWORD)pNewObj;
		return 1L;
	}
	
	return 0L;
}

DWORD CBsRealMovie::OnInsertNewAct(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* pParam = (rmMsgItem*)dwParam1;
	char* szRetNameBuf = (char*)dwParam2;

	if( !pParam )
		return 0L;

	CRMObject*	pSelObj;
	char szTempName[eFileNameLength];
	pSelObj = pObj->GetObject( pParam->szObjName );

	if( !pSelObj )
		return 0L;

	int nSize = pSelObj->GetActCount();
	CRMAct* pTmpAct;

	do 
	{
		sprintf( szTempName, "NewAct%03d" , nSize );
		pTmpAct = pSelObj->GetAct( szTempName );
		++nSize;
	} while( pTmpAct );

	if( pObj->InsertAct( pParam->szObjName, szTempName ) == 1 )
	{
		char* szRetNameBuf = (char*)dwParam2;
		strcpy( szRetNameBuf , szTempName );
		return 1L;
	}

    return 0L;
}

DWORD CBsRealMovie::OnInsertNewLip(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* pParam = (rmMsgItem*)dwParam1;
	char* szRetNameBuf = (char*)dwParam2;

	if( !pParam )
		return 0L;

	CRMObject*	pSelObj;
	char szTempName[eFileNameLength];
	pSelObj = pObj->GetObject( pParam->szObjName );

	if( !pSelObj )
		return 0L;

	int nSize = pSelObj->GetLipCount();
	CRMLip* pTmpLip;

	do 
	{
		sprintf( szTempName, "NewLip%03d" , nSize );
		pTmpLip = pSelObj->GetLip( szTempName );
		++nSize;
	} while( pTmpLip );

	if( pObj->InsertLip( pParam->szObjName, szTempName ) == 1 )
	{
		char* szRetNameBuf = (char*)dwParam2;
		strcpy( szRetNameBuf , szTempName );
		return 1L;
	}

	return 0L;
}

DWORD CBsRealMovie::OnInsertNewFace(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* pParam = (rmMsgItem*)dwParam1;
	char* szRetNameBuf = (char*)dwParam2;

	if( !pParam )
		return 0L;

	CRMObject*	pSelObj;
	char szTempName[eFileNameLength];
	pSelObj = pObj->GetObject( pParam->szObjName );

	if( !pSelObj )
		return 0L;

	int nSize = pSelObj->GetFaceCount();
	CRMFace* pTmpFace;

	do 
	{
		sprintf( szTempName, "NewFace%03d" , nSize );
		pTmpFace = pSelObj->GetFace( szTempName );
		++nSize;
	} while( pTmpFace );

	if( pObj->InsertFace( pParam->szObjName, szTempName ) == 1 )
	{
		char* szRetNameBuf = (char*)dwParam2;
		strcpy( szRetNameBuf , szTempName );
		return 1L;
	}

	return 0L;
}

DWORD CBsRealMovie::OnInsertNewBone(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* pParam = (rmMsgItem*)dwParam1;
	char* szRetNameBuf = (char*)dwParam2;

	if( !pParam )
		return 0L;

	CRMObject*	pSelObj;
	char szTempName[eFileNameLength];
	pSelObj = pObj->GetObject( pParam->szObjName );

	if( !pSelObj )
		return 0L;

	int nSize = pSelObj->GetBoneCount();
	CRMBone* pTmpBone;

	do 
	{
		sprintf( szTempName, "NewBone%03d" , nSize );
		pTmpBone = pSelObj->GetBone( szTempName );
		++nSize;
	} while( pTmpBone );

	if( pObj->InsertBone( pParam->szObjName, szTempName ) == 1 )
	{
		char* szRetNameBuf = (char*)dwParam2;
		strcpy( szRetNameBuf , szTempName );
		return 1L;
	}

	return 0L;
}

DWORD CBsRealMovie::OnInsertNewWave(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* pParam = (rmMsgItem*)dwParam1;
	char* szRetNameBuf = (char*)dwParam2;

	if( pParam )
	{
		CRMWave* pNewWave = pObj->InsertWave( pParam->szObjName );
		BsAssert(pNewWave && "Failed to insert a wave");// aleksger: prefix bug 370: Possible failure to insert.
		strcpy( szRetNameBuf , pNewWave->GetName() );
		return 1L;
	}

	return 0L;
}

DWORD CBsRealMovie::OnInsertNewCam(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* pParam = (rmMsgItem*)dwParam1;
	char* szRetNameBuf = (char*)dwParam2;

	if( pParam )
	{
		CRMCamera* pNewCam = pObj->InsertCamera( pParam->szObjName );
		BsAssert(pNewCam && "Failed to insert camera in LoadCameras()"); // aleksger: prefix bug 370: Failure to insert will lead to memory leak.
		strncpy( szRetNameBuf , pNewCam->GetName() , 63 );

		// pParam->dwRetParam = (DWORD)pNewCam->GetPathPtr( 0 );
		return 1L;
	}

	return 0L;
}

DWORD CBsRealMovie::OnInsertNewEffect(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* pParam = (rmMsgItem*)dwParam1;
	char* szRetNameBuf = (char*)dwParam2;

	if( pParam )
	{
		CRMEffect* pNewEffect = pObj->InsertEff( pParam->szObjName );
		BsAssert(pNewEffect && "Failed to insert an effect");// aleksger: prefix bug 377: Possible failure to insert.
		strncpy( szRetNameBuf , pNewEffect->GetName() , 63 );
		return 1L;
	}

	return 0L;
}

DWORD CBsRealMovie::OnInsertNewParticle(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* pParam = (rmMsgItem*)dwParam1;
	char* szRetNameBuf = (char*)dwParam2;

	if( pParam )
	{
		return 1L;
	}

	return 0L;
}

DWORD CBsRealMovie::OnGetNodePtr(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	return (DWORD)pObj->GetNodePtrByName( (int)dwParam1, (char*)dwParam2 );
}

DWORD CBsRealMovie::OnSetTimeByNodePtr(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgTimeItemByNodePtr* prmItem = (rmMsgTimeItemByNodePtr*)dwParam1;

	switch( prmItem->nType )
	{
	case RMTYPE_ACT :
		{
			CRMAct* pAct = (CRMAct*)prmItem->dwNodePtr;
			pAct->SetStartTime( prmItem->fTime, prmItem->dwTime );
		}
		return 1L;
	case RMTYPE_LIP :
		{
			CRMLip* pLip = (CRMLip*)prmItem->dwNodePtr;
			pLip->SetStartTime( prmItem->fTime , prmItem->dwTime );
		}
		return 1L;
	case RMTYPE_FACE :
		{
			CRMFace* pFace = (CRMFace*)prmItem->dwNodePtr;
			pFace->SetStartTime( prmItem->fTime , prmItem->dwTime );
		}
		return 1L;
	case RMTYPE_CAM :
		{
			CRMCamera* pCam = (CRMCamera*)prmItem->dwNodePtr;
			pCam->SetStartTime( prmItem->fTime );

#ifndef _XBOX
			// Sort
			pObj->SortCameraWithTime();
#endif

		}
		return 1L;
	case RMTYPE_WAVE :
		{
			CRMWave* pWave = (CRMWave*)prmItem->dwNodePtr;
			pWave->SetStartTime( prmItem->fTime, prmItem->dwTime );
			// assert( 0 && "아직 준비 안되었음");
		}
		return 1L;
	case RMTYPE_EFFECT :
		{
			CRMEffect* pEffect = (CRMEffect*)prmItem->dwNodePtr;
			pEffect->SetStartTime( prmItem->fTime, prmItem->dwTime );
			//assert( 0 && "아직 준비 안되었음");
		}
		return 1L;
	case RMTYPE_PACKAGE :
		{
			CRMPackage* pPack = (CRMPackage*)prmItem->dwNodePtr;
			pPack->SetStartTime( prmItem->fTime, prmItem->dwTime );

			// 미리 연결된것들이 있다면 전체 시간 맞춰 주기.
			pObj->ConformItmesTimeToPackTimeCore( pPack );
			//
		}
		return 1L;
	}

	return 0L;
}

DWORD CBsRealMovie::OnGetNodeNameByNodePtr(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgTimeItemByNodePtr* prmItem = (rmMsgTimeItemByNodePtr*)dwParam1;
	char* pszNameBuffer = (char*)dwParam2;

	switch( prmItem->nType )
	{
	case RMTYPE_ACT :
		{
			CRMAct* pAct = (CRMAct*)prmItem->dwNodePtr;
			strncpy( pszNameBuffer, pAct->GetName() , eFileNameLength );
		}
		return 1L;
	case RMTYPE_LIP :
		{
			CRMLip* pLip = (CRMLip*)prmItem->dwNodePtr;
			strncpy( pszNameBuffer, pLip->GetName() , eFileNameLength );
		}
		return 1L;
	case RMTYPE_FACE :
		{
			CRMFace* pFace = (CRMFace*)prmItem->dwNodePtr;
			strncpy( pszNameBuffer, pFace->GetName() , eFileNameLength );
		}
		return 1L;
	case RMTYPE_WAVE :
		{
			CRMWave* pWave = (CRMWave*)prmItem->dwNodePtr;
			strncpy( pszNameBuffer, pWave->GetName(), eFileNameLength );
		}
		return 1L;
	case RMTYPE_CAM :
		{
			CRMCamera* pCam = (CRMCamera*)prmItem->dwNodePtr;
			strncpy( pszNameBuffer, pCam->GetName(), eFileNameLength );
		}
		return 1L;
	case RMTYPE_EFFECT :
		{
			CRMEffect* pEffect = (CRMEffect*)prmItem->dwNodePtr;
			strncpy( pszNameBuffer, pEffect->GetName(), eFileNameLength );
		}
		return 1L;
	case RMTYPE_PACKAGE :
		{
			CRMPackage* pPack = (CRMPackage*)prmItem->dwNodePtr;
			strncpy( pszNameBuffer, pPack->GetName(), eFileNameLength );
		}
		return 1L;
	}

	return 0L;
}

DWORD CBsRealMovie::OnGetObjectMember(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* prmItem = (rmMsgItem*)dwParam1;
	CRMObject* pRMObj;

	pRMObj = pObj->GetObject( prmItem->szObjName );

	if( pRMObj )
	{
		switch( prmItem->nType )
		{
		case RMT_SKINFILENAME :
			{
				char** ppSFName = (char**)dwParam2;
				*ppSFName = pRMObj->GetSkinFileName();
			}
			return 1L;
		case RMT_BAFILENAME :
			{
				char** ppBAName = (char**)dwParam2;
				*ppBAName = pRMObj->GetBaFileName();
			}
			return 1L;
		case RMT_ANICOUNT :
			{
				int* prCnt = (int*)dwParam2;
				*prCnt = pRMObj->GetAniCount();
			}
			return 1L;
		}
	}

	return 0L;
}

DWORD CBsRealMovie::OnGetActMember(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* prmItem = (rmMsgItem*)dwParam1;
	CRMObject* pRMObj;

	pRMObj = pObj->GetObject( prmItem->szObjName );

	if( !pRMObj )
		return 0L;

	CRMAct*	pAct = pRMObj->GetAct( prmItem->szSubName );

	if( pAct )
	{
		switch( prmItem->nType )
		{
		case RMT_OBJNAME :
			{
				char** ppObjName = (char**)dwParam2;
				*ppObjName = pAct->GetName();
			}
			return 1;
		case RMT_ANIINDEX :
			{
				int* pRet = (int*)dwParam2;
				*pRet = pAct->GetAniIndex();
			}
			return 1;
		case RMT_STARTTIME :
			{
				float* pfRet = (float*)dwParam2;
				*pfRet = pAct->GetStartTime();
				prmItem->dwRetParam = (DWORD)pAct;
			}
			return 1;
		case RMT_STARTTICK :
			{
				int* pnRet = (int*)dwParam2;
				*pnRet = (int)pAct->GetStartTick();
				prmItem->dwRetParam = (DWORD)pAct;
			}
			return 1;
		}
	}

	return 0L;
}

DWORD CBsRealMovie::OnGetLipMember(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* prmItem = (rmMsgItem*)dwParam1;
	CRMObject* pRMObj;

	pRMObj = pObj->GetObject( prmItem->szObjName );

	if( !pRMObj )
		return 0L;

	CRMLip*	pLip = pRMObj->GetLip( prmItem->szSubName );

	if( pLip )
	{
		switch( prmItem->nType )
		{
		case RMT_OBJNAME :
			{
				char** ppObjName = (char**)dwParam2;
				*ppObjName = pLip->GetName();
			}
			return 1L;
		case RMT_LTFNAME :
			{
				char** ppLtfname = (char**)dwParam2;
				*ppLtfname = pLip->GetLTFFileName();
			}
			return 1L;
		case RMT_WAVNAME :
			{
				char** ppWavname = (char**)dwParam2;
				*ppWavname = pLip->GetWavFileName();
			}
			return 1L;
		case RMT_STARTTIME :
			{
				float* pfRet = (float*)dwParam2;
				*pfRet = pLip->GetStartTime();
				prmItem->dwRetParam = (DWORD)pLip;
			}
			return 1L;
		case RMT_STARTTICK :
			{
				int* pnRet = (int*)dwParam2;
				*pnRet = (int)pLip->GetStartTick();
				prmItem->dwRetParam = (DWORD)pLip;
			}
			return 1;
		}
	}

	return 0L;
}

DWORD CBsRealMovie::OnGetFaceMember(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* prmItem = (rmMsgItem*)dwParam1;
	CRMObject* pRMObj;

	pRMObj = pObj->GetObject( prmItem->szObjName );

	if( !pRMObj )
		return 0L;

	CRMFace*	pFace = pRMObj->GetFace( prmItem->szSubName );

	if( pFace )
	{
		switch( prmItem->nType )
		{
		case RMT_OBJNAME :
		case RMT_FACENAME :
			{
				char** ppObjName = (char**)dwParam2;
				*ppObjName = pFace->GetName();
			}
			return 1L;
		case RMT_ANIINDEX :
			{
				int* pRet = (int*)dwParam2;
				*pRet = pFace->GetAniIndex();
			}
			return 1L;
		case RMT_STARTTIME :
			{
				float* pfRet = (float*)dwParam2;
				*pfRet = pFace->GetStartTime();
				prmItem->dwRetParam = (DWORD)pFace;
			}
			return 1L;
		case RMT_STARTTICK :
			{
				int* pnRet = (int*)dwParam2;
				*pnRet = (int)pFace->GetStartTick();
				prmItem->dwRetParam = (DWORD)pFace;
			}
			return 1;
		}
	}
	return 0L;
}

DWORD CBsRealMovie::OnGetBoneMember(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* prmItem = (rmMsgItem*)dwParam1;
	CRMObject* pRMObj;

	pRMObj = pObj->GetObject( prmItem->szObjName );

	if( !pRMObj )
		return 0L;

	CRMBone*	pBone = pRMObj->GetBone( prmItem->szSubName );

	if( pBone )
	{
		switch( prmItem->nType )
		{
		case RMT_STARTTIME :
			{
				float* pfRet = (float*)dwParam2;
				*pfRet = pBone->GetStartTime();
				prmItem->dwRetParam = (DWORD)pBone;
			}
			return 1L;
		case RMT_STARTTICK :
			{
				int* pnRet = (int*)dwParam2;
				*pnRet = (int)pBone->GetStartTick();
				prmItem->dwRetParam = (DWORD)pBone;
			}
			return 1L;
		case RMT_BONENAME :
			{
				char** ppObjName = (char**)dwParam2;
				*ppObjName = pBone->GetName();
			}
			return 1L;
		case RMT_BONE_INDEX :
			{
				int* pRet = (int*)dwParam2;
				*pRet = pBone->GetBoneIndex();
			}
			return 1L;
		case RMT_BONE_ROT_X :
			{
				float* pfRet = (float*)dwParam2;
				*pfRet = pBone->GetRotX();
			}
			return 1L;
		case RMT_BONE_ROT_Y :
			{
				float* pfRet = (float*)dwParam2;
				*pfRet = pBone->GetRotY();
			}
			return 1L;
		case RMT_BONE_ROT_Z :
			{
				float* pfRet = (float*)dwParam2;
				*pfRet = pBone->GetRotZ();
			}
			return 1L;
		case RMT_BONE_USETIME :
			{
				float* pfRet = (float*)dwParam2;
				*pfRet = pBone->GetUseTime();
			}
			return 1L;
		}
	}

	return 0L;
}


DWORD CBsRealMovie::OnGetWaveMember(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* prmItem = (rmMsgItem*)dwParam1;
	CRMWave* pRMWave;

	pRMWave = pObj->GetWave( prmItem->szObjName );

	if( pRMWave )
	{
		switch( prmItem->nType )
		{
		case RMT_STARTTIME :
			{
				float* pfRet = (float*)dwParam2;
				*pfRet = pRMWave->GetStartTime();
				prmItem->dwRetParam = (DWORD)pRMWave;
			}
			return 1L;
		case RMT_STARTTICK :
			{
				int* pnRet = (int*)dwParam2;
				*pnRet = (int)pRMWave->GetStartTick();
				prmItem->dwRetParam = (DWORD)pRMWave;
			}
			return 1L;
		}
	}

	return 0L;
}

DWORD CBsRealMovie::OnGetCameraMember(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* prmItem = (rmMsgItem*)dwParam1;
	CRMCamera* pRMCamera;

	pRMCamera = pObj->GetCamera( prmItem->szObjName );

	if( pRMCamera )
	{
		switch( prmItem->nType )
		{
		case RMT_STARTTIME :
			{
				float* pfRet = (float*)dwParam2;
				*pfRet = pRMCamera->GetStartTime();
				prmItem->dwRetParam = (DWORD)pRMCamera;
			}
			return 1L;
		case RMT_STARTTICK :
			{
				// int* pnRet = (int*)dwParam2;
				// *pnRet = pRMCamera->GetStartTic
			}
			return 1L;
		case RMT_CROSSVECTOR :
			{
				CCrossVector** ppCross = (CCrossVector**)dwParam2;
				*ppCross = &pRMCamera->m_Cross;
			}
			return 1L;
		case RMT_PATH_CROSS :
			{
				CCrossVector** ppCross = (CCrossVector**)dwParam2;
				*ppCross = &(pRMCamera->m_CamPathPool[ prmItem->nReturn ]->m_Cross);
				prmItem->dwRetParam = (DWORD)pRMCamera->m_CamPathPool[ prmItem->nReturn ];
			}
			return 1L;
		case RMT_PATHCOUNT :
			{
				int* pnRet = (int*)dwParam2;
				*pnRet = (int)pRMCamera->m_CamPathPool.size();
			}
			return 1L;
		}
	}

	return 0L;
}

DWORD CBsRealMovie::OnGetEffectMember(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* prmItem = (rmMsgItem*)dwParam1;
	CRMEffect* pRMEffect;

	pRMEffect = pObj->GetEffect( prmItem->szObjName );

	if( pRMEffect )
	{
		switch( prmItem->nType )
		{
		case RMT_STARTTIME :
			{
				float* pfRet = (float*)dwParam2;
				*pfRet = pRMEffect->GetStartTime();
				prmItem->dwRetParam = (DWORD)pRMEffect;
			}
			return 1L;
		case RMT_STARTTICK :
			{
				int* pnRet = (int*)dwParam2;
				*pnRet = (int)pRMEffect->GetStartTick();
				prmItem->dwRetParam = (DWORD)pRMEffect;
			}
			return 1L;
		case RMT_EFFECTLINKTYPE :
			{
				int* pnRet = (int*)dwParam2;
				*pnRet = pRMEffect->GetLinkedType();
			}
			return 1L;
		case RMT_EFFECTPARAM1 :
		case RMT_EFFECTPARAM2 :
		case RMT_EFFECTPARAM3 :
			{
				DWORD* pdwParam = (DWORD*)dwParam2;
				*pdwParam = pRMEffect->GetParam( prmItem->nType - RMT_EFFECTPARAM1 );
			}
			return 1L;
		}
	}

	return 0L;
}

DWORD CBsRealMovie::OnGetObjPtr(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* prmItem = (rmMsgItem*)dwParam1;

	return (DWORD)pObj->GetObject( prmItem->szObjName );
}

DWORD CBsRealMovie::OnGetActPtr(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* prmItem = (rmMsgItem*)dwParam1;
	CRMObject* pRMObj;

	pRMObj = pObj->GetObject( prmItem->szObjName );

	if( pRMObj )
	{
		return (DWORD)pRMObj->GetAct( prmItem->szSubName );
	}

	return 0L;
}

DWORD CBsRealMovie::OnGetLipPtr(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* prmItem = (rmMsgItem*)dwParam1;
	CRMObject* pRMObj;

	pRMObj = pObj->GetObject( prmItem->szObjName );

	if( pRMObj )
	{
		return (DWORD)pRMObj->GetLip( prmItem->szSubName );
	}

	return 0L;
}

DWORD CBsRealMovie::OnGetFacePtr(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* prmItem = (rmMsgItem*)dwParam1;
	CRMObject* pRMObj;

	pRMObj = pObj->GetObject( prmItem->szObjName );

	if( pRMObj )
	{
		return (DWORD)pRMObj->GetFace( prmItem->szSubName );
	}

	return 0L;
}

DWORD CBsRealMovie::OnGetBonePtr(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* prmItem = (rmMsgItem*)dwParam1;
	CRMObject* pRMObj;

	pRMObj = pObj->GetObject( prmItem->szObjName );

	if( pRMObj )
	{
		return (DWORD)pRMObj->GetBone( prmItem->szSubName );
	}

	return 0L;
}

DWORD CBsRealMovie::OnGetWavePtr(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* prmItem = (rmMsgItem*)dwParam1;
	return (DWORD)pObj->GetWave( prmItem->szObjName );
}

DWORD CBsRealMovie::OnGetCameraPtr(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* prmItem = (rmMsgItem*)dwParam1;
	return (DWORD)pObj->GetCamera( prmItem->szObjName );
}

DWORD CBsRealMovie::OnGetEffectPtr(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* prmItem = (rmMsgItem*)dwParam1;
	return (DWORD)pObj->GetEffect( prmItem->szObjName );
}



DWORD CBsRealMovie::OnChangeObjMember(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* prmItem = (rmMsgItem*)dwParam1;

	switch( prmItem->nType )
	{
	case RMT_OBJNAME :
        return (DWORD)pObj->ChangeObjName( prmItem->szObjName , (char*)dwParam2 );
	case RMT_SKINFILENAME :
		{
			CRMObject *pRMObj = pObj->GetObject( prmItem->szObjName );
			if( !pRMObj )	return 0L;

			pRMObj->SetSkinFileName( (char*)dwParam2 );
		}
		return 1;
	case RMT_BAFILENAME :
		{
			CRMObject *pRMObj = pObj->GetObject( prmItem->szObjName );
			if( !pRMObj )	return 0L;

			pRMObj->SetBaFileName( (char*)dwParam2 );
		}
		return 1;
	case RMT_OBJ_POS_X :
		{
			CRMObject *pRMObj = pObj->GetObject( prmItem->szObjName );
			if( !pRMObj )	return 0L;

			pRMObj->GetPosPtr()->x = *(float*)dwParam2;
		}
		return 1;
	case RMT_OBJ_POS_Y :
		{
			CRMObject *pRMObj = pObj->GetObject( prmItem->szObjName );
			if( !pRMObj )	return 0L;

			pRMObj->GetPosPtr()->y = *(float*)dwParam2;
		}
		return 1;
	case RMT_OBJ_POS_Z :
		{
			CRMObject *pRMObj = pObj->GetObject( prmItem->szObjName );
			if( !pRMObj )	return 0L;

			pRMObj->GetPosPtr()->z = *(float*)dwParam2;
		}
		return 1;
	case RMT_OBJ_ROTATE :
		{
			CRMObject *pRMObj = pObj->GetObject( prmItem->szObjName );
			if( !pRMObj )	return 0L;

			pRMObj->SetAbsoluteRot( (int)dwParam2 );
		}
		return 1;
	case RMT_RELOADTYPE :
		{
			CRMObject *pRMObj = pObj->GetObject( prmItem->szObjName );
			if( !pRMObj )	return 0L;

			pObj->ChdirStart();
			pRMObj->ReLoadObject();
			pObj->ChdirEnd();
		}
		return 1;
	/*
	case RMT_HAIRSKINNAME :
		{
			CRMObject *pRMObj = pObj->GetObject( prmItem->szObjName );
			if( !pRMObj )	return 0L;

			pRMObj->SetHairSkinName( (char*)dwParam2 );
		}
		return 1;
	case RMT_MANTLESKINNAME :
		{
			CRMObject *pRMObj = pObj->GetObject( prmItem->szObjName );
			if( !pRMObj )	return 0L;

			pRMObj->SetMantleSkinName( (char*)dwParam2 );
		}
		return 1;
	*/
	case RMT_PHYSICSSKINNAME :
		{
			CRMObject *pRMObj = pObj->GetObject( prmItem->szObjName );
			if( !pRMObj )	return 0L;

			pRMObj->SetPhysicsSkinName( 
				(int)prmItem->dwRetParam, 
				(char*)dwParam2 );
		}
		return 1;
	case RMT_PHYSICSAPPLY :
	case RMT_LINKBONEAPPLY :
		{
			CRMObject *pRMObj = pObj->GetObject( prmItem->szObjName );
			if( !pRMObj )	return 0L;

			int nIndex = (int)prmItem->dwRetParam;
			CRMObject *pLinkedObj = pObj->FindObjectByOBJName( pRMObj->GetLinkedObjName( nIndex) );

			if( pLinkedObj )
			{
				if( pRMObj->GetBoneName( nIndex )[0] != NULL )
				{
					pRMObj->LinkObject( pLinkedObj, pRMObj->GetBoneName( nIndex ) );
				}
			}
		}
		return 1;
	case RMT_LINKBONE :
		{
			CRMObject *pRMObj = pObj->GetObject( prmItem->szObjName );
			if( !pRMObj )	return 0L;

			pRMObj->SetBoneName( 
				(int)prmItem->dwRetParam,
				(char*)dwParam2 );
		}
		return 1;
	case RMT_UNLINKBONE :
		{
			CRMObject *pRMObj = pObj->GetObject( prmItem->szObjName );
			if( !pRMObj )	return 0L;

			int nIndex = (int)prmItem->dwRetParam;

			if( pRMObj->GetLinkedObjName( nIndex )[0] != NULL )
			{
				CRMObject *pLinkedObj = pObj->FindObjectByOBJName( pRMObj->GetLinkedObjName( nIndex) );

				if( pLinkedObj )
				{
					g_BsKernel.SendMessage( 
						pRMObj->m_nObjectIndex, 
						BS_UNLINKOBJECT,
						pLinkedObj->m_nObjectIndex );
				}
			}

			if( dwParam2 == 1 )
			{
				pRMObj->ClearPhysicsSkinName( nIndex );
			}
			else if( dwParam2 == 2 )
			{
				pRMObj->ClearBoneName( nIndex );
			}

			// ClearPhysicsSkinName
		}
		return 1;
	//case RMT_LINKBONEAPPLY :{}return 1;
	}

	return 0L;
}

DWORD CBsRealMovie::OnChangeActMember(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* prmItem = (rmMsgItem*)dwParam1;

	switch( prmItem->nType )
	{
	case RMT_ACTNAME :
		return (DWORD)pObj->ChangeActName( prmItem->szObjName, prmItem->szSubName, (char*)dwParam2 );
	case RMT_ANIINDEX :
		{
			CRMObject* pRMObj = pObj->GetObject( prmItem->szObjName );
			pRMObj->GetAct( prmItem->szSubName )->SetAniIndex( (int)dwParam2 );

			int nLen = pRMObj->GetAniLength( (int)dwParam2 );
			prmItem->nReturn = nLen;
			// m_pAniObject->GetAniLength( nAniIndex )
		}
		return 1;
	case RMT_STARTTIME :
		{
			CRMObject* pRMObj = pObj->GetObject( prmItem->szObjName );
			pRMObj->GetAct( prmItem->szSubName )->SetStartTime( *((float*)dwParam2), -1 );
			BsAssert( 0 && "Act::RMT_STARTTIME 이 쓰임. (에러 무시해도 됨.)");
		}
		return 1;
	case RMT_STARTTICK :
		{
			CRMObject* pRMObj = pObj->GetObject( prmItem->szObjName );
			CRMAct* pAct = pRMObj->GetAct( prmItem->szSubName );
			pAct->SetStartTime( (float)dwParam2 / g_fRMFps , dwParam2 );
			prmItem->dwRetParam = (DWORD)pAct;
		}
		return 1;

	case RMT_ACTREPEAT :
		{
			CRMObject* pRMObj = pObj->GetObject( prmItem->szObjName );
			pRMObj->GetAct( prmItem->szSubName )->SetRepeat( (int)dwParam2 );

			// int nLen = pRMObj->GetAniLength( (int)dwParam2 );
			// prmItem->nReturn = nLen;
		}
		return 1;
	case RMT_ACT_STARTFRAME :
		{
			CRMObject* pRMObj = pObj->GetObject( prmItem->szObjName );
			CRMAct* pAct = pRMObj->GetAct( prmItem->szSubName );
			pAct->SetStartFrame( (int)dwParam2 );
			prmItem->dwRetParam = (DWORD)pAct;
		}
		return 1;
	case RMT_ACT_ENDFRAME :
		{
			CRMObject* pRMObj = pObj->GetObject( prmItem->szObjName );
			CRMAct* pAct = pRMObj->GetAct( prmItem->szSubName );
			pAct->SetEndFrame( (int)dwParam2 );
			prmItem->dwRetParam = (DWORD)pAct;
		}
		return 1;
	}
	return 0L;
}

DWORD CBsRealMovie::OnChangeLipMember(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* prmItem = (rmMsgItem*)dwParam1;

	switch( prmItem->nType )
	{
	case RMT_TEXT_ID :
		{
			CRMObject* pRMObj = pObj->GetObject( prmItem->szObjName );
			CRMLip* pLip = pRMObj->GetLip( prmItem->szSubName );
			pLip->SetTextID( (int)dwParam2 );
		}
		return 1;
	case RMT_LIPNAME :
		return (DWORD)pObj->ChangeLipName( prmItem->szObjName, prmItem->szSubName, (char*)dwParam2 );
	case RMT_LTFNAME :
		{
			CRMObject* pRMObj = pObj->GetObject( prmItem->szObjName );
			CRMLip* pLip = pRMObj->GetLip( prmItem->szSubName );
			pLip->SetLTFFileName( (char*)dwParam2 );

			pObj->ChdirStart();
			
			pLip->LoadLipResource();
			// pLip->GetLTFObject().FLoad_text_( pLip->GetLTFFileName() );
			// pLip->ResetEventFlag();
			pObj->ChdirEnd();
		}
		return 1;
	case RMT_WAVNAME :
		{
			CRMObject* pRMObj = pObj->GetObject( prmItem->szObjName );
			CRMLip* pLip = pRMObj->GetLip( prmItem->szSubName );
			pLip->SetWavFileName( (char*)dwParam2 );
			DebugString( "RMT_WAVNAME 구동. Wave 등록 필요하지 않을까?\n");
			prmItem->dwRetParam = (DWORD)pLip;
		}
		return 1;
	case RMT_STARTTIME :
		{
			CRMObject* pRMObj = pObj->GetObject( prmItem->szObjName );
			CRMLip* pLip = pRMObj->GetLip( prmItem->szSubName );
			// pLip->SetStartTime( *(float*)dwParam2 , -1 );
			pLip->SetStartTick( dwParam2 );
		}
		return 1;
	}
	
	return 0L;
}

DWORD CBsRealMovie::OnChangeFaceMember(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* prmItem = (rmMsgItem*)dwParam1;

	switch( prmItem->nType )
	{
	case RMT_FACENAME :
        return (DWORD)pObj->ChangeFaceName( prmItem->szObjName, prmItem->szSubName, (char*)dwParam2 );
	case RMT_ANIINDEX :
		{
			CRMObject* pRMObj = pObj->GetObject( prmItem->szObjName );
			pRMObj->GetFace( prmItem->szSubName )->SetAniIndex( (int)dwParam2 );
		}
		return 1;
	case RMT_STARTTIME :
		{
			CRMObject* pRMObj = pObj->GetObject( prmItem->szObjName );
			pRMObj->GetFace( prmItem->szSubName )->SetStartTime( *(float*)dwParam2, -1 );
			BsAssert( 0 && "Face::RMT_STARTTIME 이 쓰임. (에러 무시해도 됨.)");
		}
		return 1;
	case RMT_FACE_USETIME :
		{
			CRMObject* pRMObj = pObj->GetObject( prmItem->szObjName );
			pRMObj->GetFace( prmItem->szSubName )->SetUseTime( *(float*)dwParam2 );
		}
		return 1;
	case RMT_FACECOMPLEMENT :
		{
			CRMObject* pRMObj = pObj->GetObject( prmItem->szObjName );
			pRMObj->GetFace( prmItem->szSubName )->SetComplement( *(float*)dwParam2 );
		}
		return 1;
	}

	return 0;
}

DWORD CBsRealMovie::OnChangeBoneMember(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* prmItem = (rmMsgItem*)dwParam1;

	switch( prmItem->nType )
	{
	case RMT_STARTTIME :
		{
			CRMObject* pRMObj = pObj->GetObject( prmItem->szObjName );
			pRMObj->GetBone( prmItem->szSubName )->SetStartTime( *(float*)dwParam2,-1 );
			BsAssert( 0 && "Bone::RMT_STARTTIME 이 쓰임. (에러 무시해도 됨.)");
		}
		return 1;
	case RMT_BONENAME :
		return (DWORD)pObj->ChangeBoneName( prmItem->szObjName, prmItem->szSubName, (char*)dwParam2 );
	case RMT_BONE_NAME0 :
	case RMT_BONE_NAME1 :
	case RMT_BONE_NAME2 :
	case RMT_BONE_NAME3 :
		{
			int nIndex = prmItem->nType - RMT_BONE_NAME0;

			CRMObject* pRMObj = pObj->GetObject( prmItem->szObjName );
			CRMBone* pBone = pRMObj->GetBone( prmItem->szSubName );

			pBone->SetBone( nIndex, (char*)dwParam2 );
		}
		return 1;
	case RMT_BONE_USE0 :
	case RMT_BONE_USE1 :
	case RMT_BONE_USE2 :
	case RMT_BONE_USE3 :
		{
			int nIndex = prmItem->nType - RMT_BONE_USE0;

			CRMObject* pRMObj = pObj->GetObject( prmItem->szObjName );
			CRMBone* pBone = pRMObj->GetBone( prmItem->szSubName );

			pBone->SetBoneUse( nIndex, (BOOL)dwParam2 );
		}
		return 1;
	case RMT_BONE_INDEX :
		{
			CRMObject* pRMObj = pObj->GetObject( prmItem->szObjName );
			pRMObj->GetBone( prmItem->szSubName )->SetBoneIndex( (int)dwParam2 );
		}
		return 1;
	case RMT_BONE_ROT_X :
		{
			CRMObject* pRMObj = pObj->GetObject( prmItem->szObjName );
			pRMObj->GetBone( prmItem->szSubName )->SetRotX( *(float*)dwParam2 );
		}
		return 1;
	case RMT_BONE_ROT_Y :
		{
			CRMObject* pRMObj = pObj->GetObject( prmItem->szObjName );
			pRMObj->GetBone( prmItem->szSubName )->SetRotY( *(float*)dwParam2 );
		}
		return 1;
	case RMT_BONE_ROT_Z :
		{
			CRMObject* pRMObj = pObj->GetObject( prmItem->szObjName );
			pRMObj->GetBone( prmItem->szSubName )->SetRotZ( *(float*)dwParam2 );
		}
		return 1;
	case RMT_BONE_USETIME :
		{
			CRMObject* pRMObj = pObj->GetObject( prmItem->szObjName );
			pRMObj->GetBone( prmItem->szSubName )->SetUseTime( *(float*)dwParam2 );
		}
		return 1;
	}

	return 0;
}

DWORD CBsRealMovie::OnChangeWaveMember(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* prmItem = (rmMsgItem*)dwParam1;

	switch( prmItem->nType )
	{
	case RMT_STARTTIME :
		{
			CRMWave* pWave = pObj->GetWave( prmItem->szObjName );
			pWave->SetStartTime( *(float*)dwParam2, -1L );
		}
		return 1L;
	case RMT_TEXT_ID :
		{
			CRMWave* pWave = pObj->GetWave( prmItem->szObjName );
			pWave->SetTextID( (int)dwParam2 );
		}
		return 1L;
	case RMT_USE_VOICE_CUE :
		{
			CRMWave* pWave = pObj->GetWave( prmItem->szObjName );
			pWave->SetUseVoiceCur( (int)dwParam2 );
		}
		return 1L;
	case RMT_WAVNAME :
		return (DWORD)pObj->ChangeWaveName( prmItem->szObjName , (char*)dwParam2 );
	case RMT_WAVFILENAME :
		{
			CRMWave* pWave = pObj->GetWave( prmItem->szObjName );
			pWave->SetWaveFileName( (char*)dwParam2 );
			prmItem->dwRetParam = (DWORD)pWave;
		}
		return 1L;
	case RMT_WAVVOLUME :
		{
			CRMWave* pWave = pObj->GetWave( prmItem->szObjName );
			pWave->SetVolume( *((float*)dwParam2) );
		}
		return 1L;
	case RMT_WAVLOOP :
		{
			CRMWave* pWave = pObj->GetWave( prmItem->szObjName );
			pWave->SetLoop( (int)dwParam2 );
		}
		return 1L;
	case RMT_WAVELINK_OBJ :
		{
			CRMWave* pWave = pObj->GetWave( prmItem->szObjName );
			pWave->SetLinkedObjName( (char*)dwParam2 );
			prmItem->dwRetParam = (DWORD)pWave;
		}
		return 1L;
	}

	return 0L;
}

DWORD CBsRealMovie::OnChangeCameraMember(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* prmItem = (rmMsgItem*)dwParam1;

	switch( prmItem->nType )
	{
	case RMT_OBJ_POS_X :
		{
			CRMCamera* pCam = pObj->GetCamera( prmItem->szObjName );
			if( !pCam )		return 0L;
			pCam->SetXPos( *(float*)dwParam2 );
		}
		return 1;
	case RMT_OBJ_POS_Y :
		{
			CRMCamera* pCam = pObj->GetCamera( prmItem->szObjName );
			if( !pCam )		return 0L;
			pCam->SetYPos( *(float*)dwParam2 );
		}
		return 1;
	case RMT_OBJ_POS_Z :
		{
			CRMCamera* pCam = pObj->GetCamera( prmItem->szObjName );
			if( !pCam )		return 0L;
			pCam->SetZPos( *(float*)dwParam2 );
		}
		return 1;
	
	case RMT_STARTTIME :
		{
			CRMCamera* pCam = pObj->GetCamera( prmItem->szObjName );
			pCam->SetStartTime( *(float*)dwParam2 );

#ifndef _XBOX
			// Sort
			pObj->SortCameraWithTime();
#endif
		}
		return 1L;
	case RMT_RELOADTYPE :
		{
			CRMCamera* pCam = pObj->GetCamera( prmItem->szObjName );

			pObj->ChdirStart();
			pCam->LoadBCResource();
			pCam->CreateCamObject();
			pObj->ChdirEnd();

			if( pObj->m_mapCamera.size() == 1 )	// 첫 카메라.
			{
				pObj->m_pDefCam = pCam;
				// pCam->SetStartTime( 0.f );
			}
		}
		return 1L;

	case RMT_CAMERANAME :
		return (DWORD)pObj->ChangeCameraName( prmItem->szObjName, (char*)dwParam2 );
	case RMT_CAMFILENAME :
		{
			CRMCamera* pCam = pObj->GetCamera( prmItem->szObjName );
			pCam->SetFileName( (char*)dwParam2 );

			/*
			pObj->ChdirStart();
			pCam->GetBCameraObject().Load( pCam->GetFileName() );
			pObj->ChdirEnd();
			*/
		}
		return 1L;
	case RMT_CAM_USEOFFSET :
		{
			CRMCamera* pCam = pObj->GetCamera( prmItem->szObjName );
			if( !pCam )		return 0L;
			pCam->SetUseOffset( (int)dwParam2 );
		}
		return 1L;
	case RMT_CAM_FOV :
		{
			CRMCamera* pCam = pObj->GetCamera( prmItem->szObjName );
			pCam->SetFOV( *(float*)dwParam2 );
		}
		return 1L;
	case RMT_CAM_PATH_X :
		{
			CRMCamera* pCam = pObj->GetCamera( prmItem->szObjName );
			if( !pCam )		return 0L;
			pCam->SetPathXPos( prmItem->nReturn, *(float*)dwParam2 );
		}
		return 1L;
	case RMT_CAM_PATH_Y :
		{
			CRMCamera* pCam = pObj->GetCamera( prmItem->szObjName );
			if( !pCam )		return 0L;
			pCam->SetPathYPos( prmItem->nReturn, *(float*)dwParam2 );
		}
		return 1L;
	case RMT_CAM_PATH_Z :
		{
			CRMCamera* pCam = pObj->GetCamera( prmItem->szObjName );
			if( !pCam )		return 0L;
			pCam->SetPathZPos( prmItem->nReturn, *(float*)dwParam2 );
		}
		return 1L;
	case RMT_CAM_PATH_FOV :
		{
			CRMCamera* pCam = pObj->GetCamera( prmItem->szObjName );
			if( !pCam )		return 0L;
			pCam->SetPathFOV( prmItem->nReturn , *(float*)dwParam2 );
		}
		return 1L;
	case RMT_CAM_PATH_DURATION :
		{
			CRMCamera* pCam = pObj->GetCamera( prmItem->szObjName );
			if( !pCam )		return 0L;
			pCam->SetPathDuration( prmItem->nReturn , (int)dwParam2 );
			pCam->RecalcDuration();

			pObj->ApplyCamDurationToEffect( pCam );
		}
		return 1L;
	case RMT_CAM_PATH_SWAYTYPE :
		{
			CRMCamera* pCam = pObj->GetCamera( prmItem->szObjName );
			if( !pCam )		return 0L;
			pCam->SetSWayType( prmItem->nReturn, (int)dwParam2 );
		}
		return 1L;
	case RMT_CAM_PATH_ROLL :
		{
			CRMCamera* pCam = pObj->GetCamera( prmItem->szObjName );
			if( !pCam )		return 0L;
			pCam->SetRollValue( prmItem->nReturn, *(float*)dwParam2 );
		}
		return 1L;
	case RMT_CAM_PATH_MOVETYPE :
		{
			CRMCamera* pCam = pObj->GetCamera( prmItem->szObjName );
			if( !pCam )		return 0L;
			pCam->SetMoveType( prmItem->nReturn, (int)dwParam2 );
		}
		return 1L;
	}

	return 0L;
}

DWORD CBsRealMovie::OnChangeEffectMember(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* prmItem = (rmMsgItem*)dwParam1;

	switch( prmItem->nType )
	{
	case RMT_OBJ_POS_X :
		{
			CRMEffect* pEffect = pObj->GetEffect( prmItem->szObjName );
			pEffect->SetXPos( *(float*)dwParam2 );
		}
		return 1L;
	case RMT_OBJ_POS_Y :
		{
			CRMEffect* pEffect = pObj->GetEffect( prmItem->szObjName );
			pEffect->SetYPos( *(float*)dwParam2 );
		}
		return 1L;
	case RMT_OBJ_POS_Z :
		{
			CRMEffect* pEffect = pObj->GetEffect( prmItem->szObjName );
			pEffect->SetZPos( *(float*)dwParam2 );
		}
		return 1L;
	case RMT_EFFECTNAME :
		return (DWORD)pObj->ChangeEffectName( prmItem->szObjName, (char*)dwParam2 );
	case RMT_EFFECTTYPE :
		{
			CRMEffect* pEffect = pObj->GetEffect( prmItem->szObjName );
			prmItem->nReturn = pEffect->GetEffectType();	// 이전 데이터 리턴.
			prmItem->dwRetParam = (DWORD)pEffect;
			pEffect->SetEffectType( (int)dwParam2 );
		}
		return 1L;
	case RMT_EFFECTFILENAME :
		{
			CRMEffect* pEffect = pObj->GetEffect( prmItem->szObjName );
			pEffect->SetFileName( (char*)dwParam2 );
			prmItem->dwRetParam = (DWORD)pEffect;

			// Link 타입 찾기.
			if( pEffect->IsCamType() )
			{
				CRMCamera* pCam = pObj->GetCamera( (char*)dwParam2 );
				pEffect->SetLinkedObjPtr( (DWORD)pCam );

				pObj->ApplyCamDurationToEffect( pCam );
			}
			else
			{
				pEffect->SetLinkedObjPtr( 0L );
			}
		}
		return 1L;
	case RMT_EFFECTLINKEDOBJNAME :
		{
			CRMEffect* pEffect = pObj->GetEffect( prmItem->szObjName );
			pEffect->SetLinkedObjName( (char*)dwParam2 );
			prmItem->dwRetParam = (DWORD)pEffect;
		}
		return 1L;
	case RMT_EFFECTLIFETIME :
		{
			CRMEffect* pEffect = pObj->GetEffect( prmItem->szObjName );

			if( pEffect->GetEffectType() == eUseCam )
			{
				prmItem->nReturn = eUseCam;
				prmItem->dwRetParam = (DWORD)pEffect;
			}

			// pEffect->SetUseTime( *(float*)dwParam2 );
			pEffect->SetUseFrame( (int)dwParam2 );
		}
		return 1L;
	case RMT_EFFECTLINKTYPE :
		{
			CRMEffect* pEffect = pObj->GetEffect( prmItem->szObjName );
			pEffect->SetLinkedType( (int)dwParam2 );
		}
		return 1L;
	case RMT_EFFECTSTRPARAM :
		{
			CRMEffect* pEffect = pObj->GetEffect( prmItem->szObjName );
			pEffect->SetStrParam( (char*)dwParam2 );
			prmItem->dwRetParam = (DWORD)pEffect;
		}
		return 1L;
	case RMT_EFFECTPARAM1 :
	case RMT_EFFECTPARAM2 :
	case RMT_EFFECTPARAM3 :
		{
			CRMEffect* pEffect = pObj->GetEffect( prmItem->szObjName );
			pEffect->SetParam( prmItem->nType - RMT_EFFECTPARAM1 , dwParam2 );
		}
		return 1L;
	case RMT_EFFECT_YAW :
		{
			CRMEffect* pEffect = pObj->GetEffect( prmItem->szObjName );
			pEffect->CrossAbsoluteYaw( (int)dwParam2 );
		}
		return 1L;
	case RMT_EFFECT_ROLL :
		{
			CRMEffect* pEffect = pObj->GetEffect( prmItem->szObjName );
			pEffect->CrossAbsoluteRoll( (int)dwParam2 );
		}
		return 1L;
	case RMT_EFFECT_PITCH :
		{
			CRMEffect* pEffect = pObj->GetEffect( prmItem->szObjName );
			pEffect->CrossAbsolutePitch( (int)dwParam2 );
		}
		return 1L;
	case RMT_EFFECT_FLOAT_PARAM1 :
	case RMT_EFFECT_FLOAT_PARAM2 :
	case RMT_EFFECT_FLOAT_PARAM3 :
	case RMT_EFFECT_FLOAT_PARAM4 :
		{
			CRMEffect* pEffect = pObj->GetEffect( prmItem->szObjName );
			pEffect->SetFParam( prmItem->nType - RMT_EFFECT_FLOAT_PARAM1 , 
				*(float*)dwParam2 );
		}
		return 1L;
	case RMT_EFFECT_LINK_FX :
		{
			CRMEffect* pEffect = pObj->GetEffect( prmItem->szObjName );
			CRMObject* pRMObj = pObj->FindObjectByOBJName( pEffect->GetLinkedObjName() );

			if( dwParam2 == 1 )	// On
                pEffect->SetLinkedObjIndex( pRMObj->m_nObjectIndex );
			else // Off
				pEffect->SetLinkedObjIndex( -1 );
		}
		return 1L;
	}

	return 0L;
}

DWORD CBsRealMovie::OnDeleteObj(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* prmItem = (rmMsgItem*)dwParam1;

	CRMObject* pRMObj = pObj->GetObject( prmItem->szObjName );
	if( pRMObj )
	{
		pRMObj->SetSkinIndexForWeapon( -1 );	// 툴의 경우 오브젝트 지울때, 스킨 인덱스를 날려 버림.
        pObj->DeleteObject( prmItem->szObjName );
	}
	return 1;
}

DWORD CBsRealMovie::OnDeleteAct(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* prmItem = (rmMsgItem*)dwParam1;
	CRMObject* pRMObj = pObj->GetObject( prmItem->szObjName );

	if( pRMObj )
	{
		// Delete 성공시 true.
		return pRMObj->DeleteAct( prmItem->szSubName ) ? 1 : 0;
	}

	return 0;
}

DWORD CBsRealMovie::OnDeleteLip(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* prmItem = (rmMsgItem*)dwParam1;
	CRMObject* pRMObj = pObj->GetObject( prmItem->szObjName );

	if( pRMObj )
	{
		return pRMObj->DeleteLip( prmItem->szSubName ) ? 1 : 0;
	}
	return 0;
}

DWORD CBsRealMovie::OnDeleteFace(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* prmItem = (rmMsgItem*)dwParam1;
	CRMObject* pRMObj = pObj->GetObject( prmItem->szObjName );

	if( pRMObj )
	{
		return pRMObj->DeleteFace( prmItem->szSubName ) ? 1 : 0;
	}
	return 0;
}

DWORD CBsRealMovie::OnDeleteBone(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* prmItem = (rmMsgItem*)dwParam1;
	CRMObject* pRMObj = pObj->GetObject( prmItem->szObjName );

	if( pRMObj )
	{
		return pRMObj->DeleteBone( prmItem->szSubName ) ? 1 : 0;
	}
	return 0;
}

DWORD CBsRealMovie::OnDeleteWave(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* prmItem = (rmMsgItem*)dwParam1;
	return pObj->DeleteWave( (char*)prmItem->szObjName );
}

DWORD CBsRealMovie::OnDeleteCamera(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* prmItem = (rmMsgItem*)dwParam1;
	return pObj->DeleteCamera( (char*)prmItem->szObjName );
}

DWORD CBsRealMovie::OnDeleteCameraPath(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* prmItem = (rmMsgItem*)dwParam1;
	return pObj->DeleteCameraPath( (char*)prmItem->szObjName , (int)dwParam2 );
}

DWORD CBsRealMovie::OnDeleteEffect(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmMsgItem* prmItem = (rmMsgItem*)dwParam1;
	return pObj->DeleteEffect( (char*)prmItem->szObjName );
}

DWORD CBsRealMovie::OnRmEnvironmentCmd(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	rmEnvCmd* pEnvCmd = (rmEnvCmd*)dwParam1;

	switch( pEnvCmd->nCmdType )
	{
	case ENVCMD_CAM_FLAG :
		{
			pObj->m_nbUseRMCam = (int)pEnvCmd->dwParam1;
		}
		return 1L;
	}

	return 0L;
}

DWORD CBsRealMovie::OnRmCommand(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2)
{
	DWORD dwRet = 0;
	rmCommand* pCmdPack = (rmCommand*)dwParam1;
	vecPtrList* pList = (vecPtrList*)dwParam2;

	switch( pCmdPack->nCmdType )
	{
	case RMCMD_GET_OBJ_PTR_LIST :
		{
			iteratorRMObject it = pObj->m_mapObject.begin();
			iteratorRMObject itEnd = pObj->m_mapObject.end();

			for( ; it != itEnd ; ++it )
			{
				pList->push_back( (VOID*)it->second );
				++dwRet;
			}
		}
		break;
	case RMCMD_GET_ACT_PTR_LIST :
		{
			CRMObject* pRMObject = pObj->GetObject( pCmdPack->szObjName );

			if( pRMObject )
			{
				itStrAct it = pRMObject->m_mapAct.begin();
				itStrAct itEnd = pRMObject->m_mapAct.end();

				for( ; it != itEnd ; ++it )
				{
					pList->push_back( (VOID*)it->second );
					++dwRet;
				}
			}
		}
		break;
	case RMCMD_GET_LIP_PTR_LIST :
		{
			CRMObject* pRMObject = pObj->GetObject( pCmdPack->szObjName );

			if( pRMObject )
			{
				itStrLip it = pRMObject->m_mapLip.begin();
				itStrLip itEnd = pRMObject->m_mapLip.end();

				for( ; it != itEnd ; ++it )
				{
					pList->push_back( (VOID*)it->second );
					++dwRet;
				}
			}
		}
		break;
	case RMCMD_GET_FACE_PTR_LIST :
		{
			CRMObject* pRMObject = pObj->GetObject( pCmdPack->szObjName );

			if( pRMObject )
			{
				itStrFace it = pRMObject->m_mapFace.begin();
				itStrFace itEnd = pRMObject->m_mapFace.end();

				for( ; it != itEnd ; ++it )
				{
					pList->push_back( (VOID*)it->second );
					++dwRet;
				}
			}
		}
		break;
	case RMCMD_GET_BONE_PTR_LIST :
		{
			CRMObject* pRMObject = pObj->GetObject( pCmdPack->szObjName );

			if( pRMObject )
			{
				itStrBone it = pRMObject->m_mapBone.begin();
				itStrBone itEnd = pRMObject->m_mapBone.end();

				for( ; it != itEnd ; ++it )
				{
					pList->push_back( (VOID*)it->second );
					++dwRet;
				}
			}
		}
		break;
	case RMCMD_GET_WAVE_PTR_LIST :
		{
			iteratorRMWave it = pObj->m_mapWave.begin();
			iteratorRMWave itEnd = pObj->m_mapWave.end();

			for( ; it != itEnd ; ++it )
			{
				pList->push_back( (VOID*)it->second );
				++dwRet;
			}
		}
		break;
	case RMCMD_GET_CAMERA_PTR_LIST :
		{
			iteratorRMCamera it = pObj->m_mapCamera.begin();
			iteratorRMCamera itEnd = pObj->m_mapCamera.end();

			for( ; it != itEnd ; ++it )
			{
				pList->push_back( (VOID*)it->second );
				++dwRet;
			}
		}
		break;
	case RMCMD_GET_EFFECT_PTR_LIST :
		{
			iteratorRMEffect it = pObj->m_mapEffect.begin();
			iteratorRMEffect itEnd = pObj->m_mapEffect.end();

			for( ; it != itEnd ; ++it )
			{
				pList->push_back( (VOID*)it->second );
				++dwRet;
			}
		}
		break;

	// 맵 바꾸기.
	case RMCMD_SET_MAPFILE :
		{
			pObj->m_BGInfo.SetFileName( pCmdPack->szObjName );
			pObj->m_BGInfo.SetFullPath( pCmdPack->szSubName );
			dwRet = 1;
		}
		break;
	case RMCMD_SET_FOV :
		{
			dwRet = 1;
		}
		break;
	}

	return dwRet;
}
	

#endif

void CBsRealMovie::AddLipTime( LARGE_INTEGER liAddTime )
{
	iteratorRMObject it;
	iteratorRMObject itEnd = m_mapObject.end();

	for( it = m_mapObject.begin() ; it != itEnd ; ++it )
	{
		CRMObject* pObject = it->second;
		pObject->AddLipTime( liAddTime );
	}
}

void CBsRealMovie::SetPause( bool bPause, LARGE_INTEGER SaveTime )
{
	m_bPause = bPause;
	m_liSaveTime = SaveTime;
}

LARGE_INTEGER CBsRealMovie::GetLipTime()
{
	if( !m_bPause )
	{
		QueryPerformanceCounter( &m_liSaveTime );
	}

	return m_liSaveTime;
}

//------------------------------------------------------------------------------------------------------------------------
/* 

SDK 내용 모음

double OC3AnimationTrack::GetValueAtTime( double time ) const

	// Special case: 
	// spefified time is the endTime of the keyList.
	// The interval assigned to this key is the same one 
	// as the one assigned to the previous key. 
	// The only difference is that for this special case, 
	// the parametric time will be equal to one, 
	// and for the time of the previous key it is zero.

	// Set common references (립 싱크용 프레임 2 개)
	const OC3AnimationKey *thisKey = &_keys[indexForThisTime    ];
	const OC3AnimationKey *nextKey = &_keys[indexForThisTime + 1];

	// Compute parametric time
	double t0 = thisKey->time;
	double t1 = nextKey->time;
	double parametricTime = ( time - t0 ) / ( t1 - t0 ); // 시간 변위
	double tsquared = parametricTime * parametricTime; // 2 곱한것 (제곱)
	double tcubed   = tsquared * parametricTime; // 3 곱한것

	// Calculate the requested value
	double v0     = thisKey->value;	// 웨이트 값
	double deltav = nextKey->value - v0;

	value = -2*deltav*tcubed + 3*deltav*tsquared + v0; // 원하는 값	= -2(v1-v0)ppp + 3(v1-v0)pp + v0
									                   //           = (v1-v0)(-2ppp + 3pp) + v0  

	// If the value is very close to zero, set it to be equal to zero.
	if( fabs(value) < k2Epsilon ) value = 0.0;

*/
//------------------------------------------------------------------------------------------------------------------------


#ifdef _USAGE_TOOL_

#include <set>


VOID	CBsRealMovie::CheckingCurrentRealMovie(vecString& rResult,setString* psetTroop/*=NULL*/)
{
	BOOL bResult;
	char szTempMsg[256];
	std::vector<CRMEffect*>	vecEffTemp;
	std::vector<CRMEffect*>	vecEffTroop;
	std::vector<CRMEffect*>	vecEffEOM;
	std::vector<CRMEffect*>	vecEffFx;

	sprintf( szTempMsg, "** Check Wave bank ** \r\n\r\n" );
	rResult.push_back( szTempMsg );
	sprintf( szTempMsg, "WaveBank Name : [%s]\r\n" , GetBGInfo().GetWaveBnkName() );
	rResult.push_back( szTempMsg );

	iteratorRMObject itObj = m_mapObject.begin();
	iteratorRMObject itObjEnd = m_mapObject.end();

	sprintf( szTempMsg, "\r\n** Check weapon object of player ** \r\n\r\n" );
	rResult.push_back( szTempMsg );

	// 무기 검사.
	for( bResult = FALSE ; itObj != itObjEnd ; ++itObj )
	{
		if( itObj->second->GetObjType() == eObjType_P1_Weapon )
		{
			sprintf( szTempMsg, "P1 Weapon Object : [%s] SkinName : [%s] \\ [%s] \r\n", 
				itObj->second->GetName(),
				itObj->second->GetSkinSubPath(),
				itObj->second->GetSkinFileName() );
			rResult.push_back( szTempMsg );
			bResult = TRUE;
		}
	}

	if( bResult == FALSE )
	{
		sprintf( szTempMsg, "Warring : There is not Weapon object of player 1.\r\n" );
		rResult.push_back( szTempMsg );
	}

	// 연결된 사운드 검사.
	sprintf( szTempMsg, "\r\n** Check Sound Signal. ** \r\n\r\n" );
	rResult.push_back( szTempMsg );

	iteratorRMWave	itWav = m_mapWave.begin();
	iteratorRMWave	itWavEnd = m_mapWave.end();

	for( ; itWav != itWavEnd ; ++itWav )
	{
		sprintf( szTempMsg, "Sound Resource Name : [%s]\tDefault Linked ObjName [%s]\r\n",
			itWav->second->GetWaveFileName(), itWav->second->GetLinkedObjName() );
		rResult.push_back( szTempMsg );
	}

	sprintf( szTempMsg, "\r\n\r\n" );
	rResult.push_back( szTempMsg );

	iteratorRMEffect	itEff = m_mapEffect.begin();
	iteratorRMEffect	itEffEnd = m_mapEffect.end();

	for( bResult = FALSE ; itEff != itEffEnd ; ++itEff )
	{
		if( itEff->second->IsSndType() )
		{
			CRMWave* pWaveObj = GetWave( itEff->second->GetFileName() );

			sprintf( szTempMsg, "Use Sound : WaveName [%s]\t\tFrame [%d]\tExist[ %c ]\tObName[ %s ]\r\n",
				itEff->second->GetFileName(),
				(int)itEff->second->GetStartTick(),
				( ( pWaveObj ) ? 'O' : 'X' ),
				itEff->second->GetLinkedObjName() );
			rResult.push_back( szTempMsg );
		}
		else if( itEff->second->IsFadeType() )
		{
			vecEffTemp.push_back( itEff->second );
		}
		else if( itEff->second->IsTroopType() )
		{
			vecEffTroop.push_back( itEff->second );
		}
		else if( itEff->second->IsEOMType() )
		{
			vecEffEOM.push_back( itEff->second );
		}
		else if( itEff->second->IsFXType() )
		{
			vecEffFx.push_back( itEff->second );
		}
	}

	sprintf( szTempMsg, "\r\n\r\n** Check Fade In/Out signal. ** \r\n\r\n" );
	rResult.push_back( szTempMsg );

	std::vector<CRMEffect*>::iterator itFadeEff = vecEffTemp.begin();
	std::vector<CRMEffect*>::iterator itFadeEffEnd = vecEffTemp.end();

	for( ; itFadeEff != itFadeEffEnd ; ++itFadeEff )
	{
		sprintf( szTempMsg, "%s Type :\tFrame [%d] duration [%d]\r\n",
			( ( (*itFadeEff)->m_nType == eFadeIn ) ? "FadeIn" : "FadeOut" ),
			(int)(*itFadeEff)->GetStartTick(),
			(*itFadeEff)->GetUseFrame() );
			rResult.push_back( szTempMsg );
	}

	sprintf( szTempMsg, "\r\n\r\n** Check End of Movie signal. ** \r\n\r\n" );
	rResult.push_back( szTempMsg );

	itFadeEff = vecEffEOM.begin();
	itFadeEffEnd = vecEffEOM.end();

	for( ; itFadeEff != itFadeEffEnd ; ++itFadeEff )
	{
		sprintf( szTempMsg, "EndOfMovie Type :\tFrame [%d]\r\n",
			(int)(*itFadeEff)->GetStartTick() );
		rResult.push_back( szTempMsg );
	}

	if( psetTroop )
	{
		sprintf( szTempMsg, "\r\n\r\n** Check Troop signal. ** \r\n\r\n" );
		rResult.push_back( szTempMsg );

		std::set< std::string >::iterator itTroop;

		std::vector<CRMEffect*>::iterator itTroopEff = vecEffTroop.begin();
		std::vector<CRMEffect*>::iterator itTroopEffEnd = vecEffTroop.end();

		for( ; itTroopEff != itTroopEffEnd ; ++itTroopEff )
		{
			itTroop = psetTroop->find( (*itTroopEff)->GetFileName() );

			if( itTroop != psetTroop->end() )
			{
				char szAreaResult[64];
				char* szArea;
				
				if( (*itTroopEff)->HasStrParam() )
				{
					szArea = (*itTroopEff)->GetStrParam();
                    if( FindArea( szArea ) == NULL )
					{
						strcpy( szAreaResult , "Do not exist!" );
					}
					else
					{
						strcpy( szAreaResult , szArea );
					}
				}
				else
				{
					strcpy( szAreaResult , "Game position" );
				}

				// 존재한다.
				sprintf( szTempMsg, "Troop [%s] \tArea [%s] \tParam1 [%d]\tFrame [%d]\r\n",
					(*itTroopEff)->GetFileName(),
					szAreaResult,
					(*itTroopEff)->GetParam(1),
					(int)(*itTroopEff)->GetStartTick() );
				rResult.push_back( szTempMsg );
			}
			else
			{
				// 맵상에 존재하지 않는다.
				sprintf( szTempMsg, "There is no troop [%s] in the map.\tArea [%s] \tParam1 [%d]\tFrame [%d]\r\n",
					(*itTroopEff)->GetFileName(),
					(*itTroopEff)->GetStrParam(),
					(*itTroopEff)->GetParam(1),
					(int)(*itTroopEff)->GetStartTick() );
				rResult.push_back( szTempMsg );
			}
		}
	}

	// FX Signal 채크
	sprintf( szTempMsg, "\r\n\r\n** Check FX signal. ** \r\n\r\n" );
	rResult.push_back( szTempMsg );

	itFadeEff = vecEffFx.begin();
	itFadeEffEnd = vecEffFx.end();

	for( ; itFadeEff != itFadeEffEnd ; ++itFadeEff )
	{
		sprintf( szTempMsg, "FX Type : [%s] \tFrame [%d]\r\n",
			(*itFadeEff)->GetFileName(),
			(int)(*itFadeEff)->GetStartTick() );
		rResult.push_back( szTempMsg );
	}
}

VOID	CBsRealMovie::CheckingCurrentRealMovieEx(vecString& rResult,setString* psetTroop/*=NULL*/)
{
	BOOL bResult;
	char szTempMsg[256];

	std::vector<CRMEffect*>	vecEffFx;

	iteratorRMObject itObj = m_mapObject.begin();
	iteratorRMObject itObjEnd = m_mapObject.end();

	sprintf( szTempMsg, "** Check objects ** \r\n" );
	rResult.push_back( szTempMsg );

	// 무기 검사.
	for( bResult = FALSE ; itObj != itObjEnd ; ++itObj )
	{
		sprintf( szTempMsg, "Obj : [%s]\t\t SkinPath : [%s] [%s] \t\tBA : [%s] [%s]\r\n", 
				itObj->second->GetName(),
				itObj->second->GetSkinSubPath(),
				itObj->second->GetSkinFileName(),
				itObj->second->GetBAFileSubPath(),
				itObj->second->GetBaFileName() );
			rResult.push_back( szTempMsg );
	}

	iteratorRMEffect
		itEff = m_mapEffect.begin();
	iteratorRMEffect
		itEffEnd = m_mapEffect.end();

	for( bResult = FALSE ; itEff != itEffEnd ; ++itEff )
	{
		if( itEff->second->IsSndType() )
		{
			CRMWave* pWaveObj = GetWave( itEff->second->GetFileName() );

			if( !pWaveObj )
			{
				if( itEff->second->HasFileName() )
				{
					sprintf( szTempMsg, "Use Sound : WaveName [%s]\t\tFrame [%d]\tExist[ %c ]\tObName[ %s ]\r\n",
						itEff->second->GetFileName(),
						(int)itEff->second->GetStartTick(),
						( ( pWaveObj ) ? 'O' : 'X' ),
						itEff->second->GetLinkedObjName() );
				}
				else
				{
					sprintf( szTempMsg, "Use Sound : WaveName [%s]\t\tFrame [%d]\tExist[ %c ]\tObName[ %s ] <-- Critical error!!. Does not have name.\r\n",
						itEff->second->GetFileName(),
						(int)itEff->second->GetStartTick(),
						( ( pWaveObj ) ? 'O' : 'X' ),
						itEff->second->GetLinkedObjName() );
				}

				rResult.push_back( szTempMsg );
			}
			else if( (int)itEff->second->GetStartTick() < 0 )
			{
				sprintf( szTempMsg, "Use Sound : WaveName [%s]\t\tFrame [%d]\tExist[ %c ]\tObName[ %s ]\r\n",
					itEff->second->GetFileName(),
					(int)itEff->second->GetStartTick(),
					( ( pWaveObj ) ? 'O' : 'X' ),
					itEff->second->GetLinkedObjName() );
				rResult.push_back( szTempMsg );
			}
		}
		else if( itEff->second->IsFXType() )
		{
			vecEffFx.push_back( itEff->second );
		}
	}

	std::vector<CRMEffect*>::iterator itFXEff = vecEffFx.begin();
	std::vector<CRMEffect*>::iterator itFXEffEnd = vecEffFx.end();

	for(  ; itFXEff != itFXEffEnd ; ++itFXEff )
	{
		sprintf( szTempMsg, "FX Type : [%s] \tFrame [%d] FileName [%s]\r\n",
			(*itFXEff)->GetFileName(),
			(int)(*itFXEff)->GetStartTick(),
			(*itFXEff)->GetStrParam() );
		rResult.push_back( szTempMsg );
	}
}


#endif




