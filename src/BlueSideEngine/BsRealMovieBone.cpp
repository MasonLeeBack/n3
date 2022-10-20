#include "stdafx.h"
#include "BsRealMovieFile.h"
#include "BsKernel.h"

#include "BsRealMovieMsg.h"
#include "BsRealMovieBone.h"

#include "BsRealMovieObject.h"
#include "BsRealMovie.h"

extern VOID	ChangeTickIntoTime( DWORD dwTick, float& fTime);

void	CRMBone::Save(BStream *pStream)
{
	int nLen = strlen( m_szName );
	pStream->Write( &nLen , sizeof(int), ENDIAN_FOUR_BYTE );
	if( nLen )
		pStream->Write( m_szName, nLen , 0 );

	nLen = sizeof(SubData);	//eBonePackSize;		// String 을 뺀 Bone Class 크기 저장.
	pStream->Write( &nLen , sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Write( &SubData, nLen, ENDIAN_FOUR_BYTE );
}

void	CRMBone::Load(BStream *pStream)
{
	int nLen;
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

void	CRMBone::SaveEx(BStream *pStream)
{
	int nLen;
	int nExCode = 2;	// 블럭 갯수.
	pStream->Write( &nExCode, sizeof(int), ENDIAN_FOUR_BYTE );

	nExCode = eBoneMax;		// 본 네임 갯수 저장.
	pStream->Write( &nExCode, sizeof(int), ENDIAN_FOUR_BYTE );

	for( int i = 0 ; i < nExCode ; ++i )
	{
		nLen = strlen( m_szBone[i] );
		pStream->Write( &nLen , sizeof(int), ENDIAN_FOUR_BYTE );
		if( nLen )
			pStream->Write( m_szBone[i], nLen , 0 );
	}

	nExCode = eBoneMax;		// use 갯수 저장.
	pStream->Write( &nExCode, sizeof(int), ENDIAN_FOUR_BYTE );

	for( int i = 0 ; i < nExCode ; ++i )
	{
		pStream->Write( &m_bUseBone[i], sizeof(BOOL), ENDIAN_FOUR_BYTE );
	}
}

void	CRMBone::LoadEx(BStream *pStream)
{
	int nLen;
	int nCount;
	int nExCode;

	pStream->Read( &nExCode, sizeof(int), ENDIAN_FOUR_BYTE );

	for( int cc = 0 ; cc < nExCode ; ++cc )	// Chunk Count
	{
		if( cc == 0 )	// 첫번째 블럭.
		{
			pStream->Read( &nCount , sizeof(int), ENDIAN_FOUR_BYTE );

			// takogura: prefix bug 5827: using nCount uninitialized -> OK
			for( int i = 0 ; i < nCount ; ++i )
			{
				pStream->Read( &nLen , sizeof(int), ENDIAN_FOUR_BYTE );
				if( nLen )
				{
					pStream->Read( m_szBone[i], nLen, 0 );
				}
				else
				{
					memset( m_szBone[i], 0, eFileNameLength );
				}
			}
		}
		else if( cc == 1 )	// 두번째 블럭
		{
			pStream->Read( &nCount , sizeof(int), ENDIAN_FOUR_BYTE );

			for( int i = 0 ; i < nCount ; ++i )
			{
				pStream->Read( &m_bUseBone[i], sizeof(BOOL), ENDIAN_FOUR_BYTE );
			}
		}
	}
}

#ifdef _USAGE_TOOL_

extern VOID	ChangeNewTickIntoTime(DWORD& dwTick,float& fTime);

void	CRMBone::ChangeFPS(void)
{
	ChangeNewTickIntoTime( SubData.m_dwTick, SubData.m_fTime );
}
#endif
