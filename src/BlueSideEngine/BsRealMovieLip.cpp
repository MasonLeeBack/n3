#include "stdafx.h"
#include "BsRealMovieFile.h"
#include "BsKernel.h"

#include "BsRealMovieMsg.h"
#include "BsRealMovieLip.h"

#include "BsRealMovieObject.h"
#include "BsRealMovie.h"

extern VOID	ChangeTickIntoTime( DWORD dwTick, float& fTime);
extern BOOL	DoesExistFile(const char *lpszFileName);

#ifdef _USAGE_TOOL_
static void	MemSwap32(void *pDest,void *pSrc,long nCnt)
{
	__asm
	{
		mov		ecx,	nCnt
		shr		ecx,	2		// nCnt / 4 4바이트 단위로.

		mov		edi,	pDest
		mov		esi,	pSrc

Loop_MS32 :

		mov		eax,	[esi]
		bswap	eax
		mov		[edi],	eax

		add		edi,	4
		add		esi,	4

		loop	Loop_MS32		// 반복
	}
}
#endif

BOOL	CRMLip::ChangeDataFolder(VOID)
{
	char szTemp[64];
	strcpy( szTemp, m_szFilePath );
	strcpy( m_szFilePath , "RealMovie\\");
	strcat( m_szFilePath , szTemp );

	return TRUE;
}

BOOL	CRMLip::LoadLipResource(void)
{
	char szLipFullName[512];

	if( m_szLTFFileName[0] )
	{
		strcpy( szLipFullName, m_szFilePath );
		strcat( szLipFullName, m_szLTFFileName );

		// 임시 코드 넣기.
#ifdef _USAGE_TOOL_
		if( DoesExistFile( szLipFullName ) == FALSE )
		{
			//char szTemp[512];
			//sprintf( szTemp, "패스 명이 '%s'로 되어 있습니다. 잘못된 값이 들어가 있습니다. 수정할 데이터 값을 직접 넣어주십시요.",szLipFullName);
			//MessageBox( NULL, szTemp, "임시 코드",MB_OK );

			char szTemp[256];

			strcpy( szTemp, m_szFilePath );

			int nLen = strlen( szTemp );
			nLen /= 4;

			for( int i = 0 ; i < nLen ; ++i )
			{
				MemSwap32( szTemp+(i*4) , szTemp+(i*4) , 4 );
			}

			strcpy( szLipFullName, szTemp );
			strcat( szLipFullName, m_szLTFFileName );

			if( DoesExistFile( szLipFullName ) == FALSE )
			{
				MessageBox( NULL, "해당 파일이 존재하지 않습니다.", "Error",MB_OK );
				return FALSE;			
			}
		}
#endif

		GetLTFObject().FLoad_text_( szLipFullName ); // m_szLTFFileName );
		ResetEventFlag();
		return TRUE;
	}

	return FALSE;
}

void	CRMLip::GetKeyString(char* pszRetKey, const size_t pszRetKey_len)//aleksger - safe string
{
#ifndef _XBOX
	strcpy_s( pszRetKey, pszRetKey_len, m_szName );
	strcat_s( pszRetKey, pszRetKey_len, "::" );
	strcat_s( pszRetKey, pszRetKey_len, m_pParent->GetName() );
#else
	strcpy_s( pszRetKey, pszRetKey_len, m_szWAVFileName );
	pszRetKey[ strlen( m_szWAVFileName ) - 4 ] = NULL;
#endif
}

bool	CRMLip::HasWave(void)
{
	if( m_szLTFFileName[0] != NULL && m_szWAVFileName[0] != NULL )
		return true;
	return false;
}

void	CRMLip::Save(BStream *pStream)
{
	int nLen;

	nLen = strlen( m_szName );
	pStream->Write( &nLen , sizeof(int), ENDIAN_FOUR_BYTE );
	if( nLen )
		pStream->Write( m_szName, nLen , 0 );

	nLen = strlen( m_szLTFFileName );
	pStream->Write( &nLen , sizeof(int), ENDIAN_FOUR_BYTE );
	if( nLen )
		pStream->Write( m_szLTFFileName, nLen , 0 );

	nLen = strlen( m_szWAVFileName );
	pStream->Write( &nLen , sizeof(int), ENDIAN_FOUR_BYTE );
	if( nLen )
		pStream->Write( m_szWAVFileName, nLen , 0 );

	nLen = sizeof( SubData );	// eLipPackSize;		// String 을 뺀 Bone Class 크기 저장.
	pStream->Write( &nLen , sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Write( &SubData, nLen, ENDIAN_FOUR_BYTE );

	// pStream->Write( &m_fTime, sizeof(float), ENDIAN_FOUR_BYTE );
}


void	CRMLip::Load(BStream *pStream)
{
	int nLen;
	// memset( m_szName , 0, eFileNameLength );
	memset( m_szLTFFileName, 0, eFileNameLength );
	memset( m_szWAVFileName, 0, eFileNameLength );

	/*
	pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );
	if( nLen )
	pStream->Read( m_szName, nLen , 0 );
	*/

	pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );
	// takogura: prefix bug 5830: using nLen uninitialized -> OK
	if( nLen )
		pStream->Read( m_szLTFFileName, nLen , 0 );

	pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );
	// takogura: prefix bug 5830: using nLen uninitialized -> OK
	if( nLen )
		pStream->Read( m_szWAVFileName, nLen , 0 );

	pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );
	// takogura: prefix bug 5830: using nLen uninitialized -> OK
	if( nLen )
	{
		pStream->Read( &SubData, nLen, ENDIAN_FOUR_BYTE );

		if( SubData.m_dwTick != -1 )
		{
			ChangeTickIntoTime( SubData.m_dwTick , SubData.m_fTime );
		}
	}
	// pStream->Read( &m_fTime, sizeof(float), ENDIAN_FOUR_BYTE );
}

void	CRMLip::SaveEx(BStream *pStream)
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

void	CRMLip::LoadEx(BStream *pStream)
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

void	CRMLip::ChangeFPS(void)
{
	ChangeNewTickIntoTime( SubData.m_dwTick, SubData.m_fTime );
}
#endif