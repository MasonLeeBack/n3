#include "stdafx.h"
#include "BsRealMovieFile.h"
#include "BsKernel.h"

#include "BsRealMovieWave.h"

//#include "BsRealMovieMsg.h"
//#include <io.h>
//#include "BsRealMovie.h"

extern VOID	ChangeTickIntoTime( DWORD dwTick, float& fTime);


CRMWave::CRMWave()
: m_nEvent(0)
{
	memset( m_szName, 0, eFileNameLength );
	memset( m_szFileName, 0, eFileNameLength );
	memset( m_szObjName, 0, eFileNameLength );
	memset( m_szFilePath, 0, eFileNameLength );

	SubData.m_Xyz.x = 0.f;
	SubData.m_Xyz.y = 0.f;
	SubData.m_Xyz.z = 0.f;

	SubData.m_bLoop = 0;
	SubData.m_fVolume = 1.f;
	SubData.m_fTime = -1.f;
	SubData.m_bDisable = 0;
	SubData.m_dwTick = -1;
	SubData.m_nTextID = -1;
	SubData.m_nUseVoiceCue = 0;

	memset( m_fFadeIn, 0, sizeof(float) * eWave32 );
	memset( m_fFadeOut, 0, sizeof(float) * eWave32 );
}

void	CRMWave::Save(BStream *pStream)
{
	int nLen;

	nLen = strlen( m_szName );
	pStream->Write( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );
	if( nLen )
		pStream->Write( m_szName , nLen , 0 );

	nLen = strlen( m_szFileName );
	pStream->Write( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );
	if( nLen )
		pStream->Write( m_szFileName, nLen , 0 );

	nLen = strlen( m_szObjName );
	pStream->Write( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );
	if( nLen )
		pStream->Write( m_szObjName, nLen, 0 );

	nLen = sizeof( SubData );
	pStream->Write( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Write( &SubData, nLen , ENDIAN_FOUR_BYTE );
}

void	CRMWave::Load(BStream *pStream)
{
	int nLen;

	/*
	pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );
	if( nLen )
	pStream->Read( m_szName , nLen , 0 );
	*/

	pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );
	// takogura: prefix bug 5832: using nLen uninitialized -> OK
	if( nLen )
		pStream->Read( m_szFileName, nLen , 0 );

	pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );
	// takogura: prefix bug 5832: using nLen uninitialized -> OK
	if( nLen )
		pStream->Read( m_szObjName, nLen, 0 );

	pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );

	if( nLen )
	{
		pStream->Read( &SubData, nLen, ENDIAN_FOUR_BYTE );

		if( SubData.m_dwTick != -1 )
		{
			ChangeTickIntoTime( SubData.m_dwTick , SubData.m_fTime );
		}
	}
}

void	CRMWave::SaveEx(BStream *pStream)
{
	int nCode = 1;
	int nLen;
	pStream->Write( &nCode, sizeof(int), ENDIAN_FOUR_BYTE );

	nLen = strlen( m_szFilePath );

	pStream->Write( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );

	if( nLen )
	{
		pStream->Write( m_szFilePath, nLen, 0 );
	}
}

void	CRMWave::LoadEx(BStream *pStream)
{
	int nCode;
	int nLen;

	pStream->Read( &nCode, sizeof(int), ENDIAN_FOUR_BYTE );

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
	}
}

#ifdef _USAGE_TOOL_

extern VOID	ChangeNewTickIntoTime(DWORD& dwTick,float& fTime);

void	CRMWave::ChangeFPS(void)
{
	ChangeNewTickIntoTime( SubData.m_dwTick, SubData.m_fTime );
}
#endif

void	CRMWave::GetKeyString(char* pszRetKey, const size_t pszRetKey_len) //aleksger - safe string
{
#ifndef _XBOX
	strcpy_s( pszRetKey, pszRetKey_len, GetName() );
#else
	strcpy_s( pszRetKey, pszRetKey_len, m_szFileName );
	pszRetKey[ strlen( m_szFileName ) - 4 ] = NULL;
#endif
}
