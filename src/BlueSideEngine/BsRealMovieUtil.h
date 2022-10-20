#pragma once

#include "BsKernel.h"
#include "bstreamext.h"

//	+ stEvent
//	+ stObjPack
//	+ stBonePack
//	+ stMapInfo
//  + stAreaSet
//  + msgRMTNotify

// Event Pool용
struct stEvent
{
	int		nType;
	void*	pEventObj;
};

struct stObjPack
{
	int nType;		// 타입
	void* pObject;	// 오브젝트 주소
	float fHeightDis;	// y 높이값 차이.
};

struct stBonePack
{
	int	nObjIndex;
	DWORD dwBoneNamePtr;
	D3DXVECTOR3	vecRot;
};

// #define EVENT_COL_MAX		8
#define EVENT_COL_MAX		96

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) if(p) { delete []p; p = NULL; }
#endif //SAFE_DELETE_ARRAY

struct stMapInfo 
{
	enum
	{
		eTextTblSize = 64,
	};

	int		nFileNameSize;
	char*	pszFileName;

	int		nFullPathSize;
	char*	pszFullPath;


	int		nAreaID;
	int		nLightID;

	int		nPointLightID;
	int		nPackageID;

	// 추가 ( 저장은 하지 않음. )
	float	m_fNearFog;
	float	m_fFarFog;

	char	szTextTblName[eTextTblSize];
	char	szWaveBnkName[eTextTblSize];


	stMapInfo()
		: pszFileName( NULL )
		, pszFullPath( NULL )
		, nFileNameSize( 0 )
		, nFullPathSize( 0 )
		, nAreaID( 0 )
		, nLightID( 0 )
		, m_fNearFog( -1.f )
		, m_fFarFog( -1.f )
		, nPointLightID( 0 )
		, nPackageID( 0 )
	{
		memset( szTextTblName, 0, eTextTblSize );
		memset( szWaveBnkName, 0, eTextTblSize );
	};

	~stMapInfo()
	{
		ReleaseAll();
	}

	char*	GetFileName()	{	return pszFileName;		}
	char*	GetPathName()	{	return pszFullPath;		}

	void	ReleaseAll()
	{
		nAreaID = 0;
		nLightID = 0;
		nPointLightID = 0;
		nPackageID = 0;
		ClearFileName();
		ClearPath();
	}

	void ClearFileName()	{	SAFE_DELETE_ARRAY( pszFileName );	nFileNameSize=0;}
	void ClearPath()		{	SAFE_DELETE_ARRAY( pszFullPath );	nFullPathSize=0;}

	void SetFileName(char* szName)
	{
		if( szName == NULL )	return;
		ClearFileName();

		int nSize = strlen( szName )+1;
		pszFileName = new char[nSize];
		memset( pszFileName, 0 , nSize );
		nFileNameSize = nSize;

		strcpy_s( pszFileName, nSize, szName ); //aleksger - safe string
	}

	void SetFullPath(char* szName)
	{
		if( szName == NULL )	return;
		ClearPath();

		int nSize = strlen( szName )+1;
		pszFullPath = new char[nSize];
		memset( pszFullPath, 0 , nSize );
		nFullPathSize = nSize;

		strcpy_s ( pszFullPath, nSize, szName ); //aleksger - safe string
	}

	void	SetTextTblName(char* szName)
	{
		if( szName == NULL || szName[0] == NULL )
		{
			memset( szTextTblName, 0 , eTextTblSize );
		}
		else
		{
			strcpy( szTextTblName, szName );
		}
	}

	char*	GetTextTblName(void)
	{
		return szTextTblName;
	}

	void	SetWaveBnkName(char* szName)
	{
		if( szName == NULL || szName[0] == NULL )
		{
			memset( szWaveBnkName, 0 , eTextTblSize );
		}
		else
		{
			strcpy( szWaveBnkName, szName );
		}
	}

	char*	GetWaveBnkName(void)
	{
		return szWaveBnkName;
	}

	void	Save(BStream *pStream)
	{
		int nOffset = 0;
		int nChunkSize = 7;
		pStream->Write( &nChunkSize , sizeof(int), ENDIAN_FOUR_BYTE );

		DWORD dwHeadPos = pStream->Tell();
		pStream->Write( &nOffset, sizeof(DWORD), ENDIAN_FOUR_BYTE );	// 0 다음 시작 위치 0값 저장.

		pStream->Write( &nFileNameSize, sizeof(int), ENDIAN_FOUR_BYTE );
		if( nFileNameSize )
			pStream->Write( pszFileName, nFileNameSize , 0 );

		pStream->Write( &nFullPathSize, sizeof(int), ENDIAN_FOUR_BYTE );
		if( nFullPathSize )
			pStream->Write( pszFullPath, nFullPathSize , 0 );

		// 추가
		pStream->Write( &nAreaID, sizeof(int), ENDIAN_FOUR_BYTE );

		// 추가 #2
		pStream->Write( &nLightID, sizeof(int), ENDIAN_FOUR_BYTE );

		// 추가 #3
		pStream->Write( &nPointLightID, sizeof(int), ENDIAN_FOUR_BYTE );

		// 추가 #4
		pStream->Write( &nPackageID, sizeof(int), ENDIAN_FOUR_BYTE );

		// 추가 #5
		int nLen = strlen( szTextTblName );
		pStream->Write( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );
		if( nLen )
		{
            pStream->Write( szTextTblName, nLen, 0 );
		}

		// 추가 #6
		nLen = strlen( szWaveBnkName );
		pStream->Write( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );
		if( nLen )
		{
			pStream->Write( szWaveBnkName, nLen, 0 );
		}


		DWORD dwCurPos = pStream->Tell();
		pStream->Seek( dwHeadPos,  BStream::fromBegin );

		pStream->Write( &dwCurPos, sizeof(int), ENDIAN_FOUR_BYTE );	// 다음 블럭 시작 위치 저장.

		pStream->Seek( dwCurPos,  BStream::fromBegin );
	}

	void	Load(BStream *pStream)
	{
		ReleaseAll();

		DWORD dwNextPos;
		int nChunkSize;
		int nLen;
		pStream->Read( &nChunkSize, sizeof(int), ENDIAN_FOUR_BYTE );
		pStream->Read( &dwNextPos, sizeof(DWORD), ENDIAN_FOUR_BYTE );

		// takogura: prefix bug 5831: using nChunkSize uninitialized -> OK
		for( int i = 0; i < nChunkSize ; ++i )
		{
			if( i == 0 ) // 첫번째 블럭.
			{
				pStream->Read( &nFileNameSize, sizeof(int), ENDIAN_FOUR_BYTE );
				if( nFileNameSize )
				{
					pszFileName = new char[nFileNameSize];
					pStream->Read( pszFileName, nFileNameSize, 0 );
				}

				pStream->Read( &nFullPathSize, sizeof(int), ENDIAN_FOUR_BYTE );
				if( nFullPathSize )
				{
					pszFullPath = new char[nFullPathSize];
					pStream->Read( pszFullPath, nFullPathSize, 0 );
				}

				
				if( nChunkSize <= 4 )
				{
					// 추가된거 읽기.
					pStream->Read( &nAreaID, sizeof(int), ENDIAN_FOUR_BYTE );
					pStream->Read( &nLightID, sizeof(int), ENDIAN_FOUR_BYTE );
					pStream->Read( &nPointLightID, sizeof(int), ENDIAN_FOUR_BYTE );
				}
				else if( nChunkSize == 5 )
				{
					// 추가된거 읽기.
					pStream->Read( &nAreaID, sizeof(int), ENDIAN_FOUR_BYTE );
					pStream->Read( &nLightID, sizeof(int), ENDIAN_FOUR_BYTE );
					pStream->Read( &nPointLightID, sizeof(int), ENDIAN_FOUR_BYTE );
					pStream->Read( &nPackageID, sizeof(int), ENDIAN_FOUR_BYTE );
				}
				else if( nChunkSize == 6 )
				{
					// 추가된거 읽기.
					pStream->Read( &nAreaID, sizeof(int), ENDIAN_FOUR_BYTE );
					pStream->Read( &nLightID, sizeof(int), ENDIAN_FOUR_BYTE );
					pStream->Read( &nPointLightID, sizeof(int), ENDIAN_FOUR_BYTE );
					pStream->Read( &nPackageID, sizeof(int), ENDIAN_FOUR_BYTE );

					pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );

					memset( szTextTblName, 0, eTextTblSize );

					if( nLen )
					{
						pStream->Read( szTextTblName, nLen ,0 );
					}
				}
				else if( nChunkSize == 7 )
				{
					// 추가된거 읽기.
					pStream->Read( &nAreaID, sizeof(int), ENDIAN_FOUR_BYTE );
					pStream->Read( &nLightID, sizeof(int), ENDIAN_FOUR_BYTE );
					pStream->Read( &nPointLightID, sizeof(int), ENDIAN_FOUR_BYTE );
					pStream->Read( &nPackageID, sizeof(int), ENDIAN_FOUR_BYTE );

					pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );
					memset( szTextTblName, 0, eTextTblSize );
					if( nLen )
					{
						pStream->Read( szTextTblName, nLen ,0 );
					}

					pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );
					memset( szWaveBnkName, 0, eTextTblSize );
					if( nLen )
					{
						pStream->Read( szWaveBnkName, nLen, 0 );
					}
				}
				else
				{
					nAreaID = 0;	// 기본 값 설정.
					nLightID = 0;
					nPointLightID = 0;
					nPackageID = 0;
					memset( szTextTblName, 0, eTextTblSize );
					memset( szWaveBnkName, 0, eTextTblSize );
				}
			}

			
		}

		pStream->Seek( dwNextPos, BStream::fromBegin );
	}
};

#define AREA_STR_MAX				32

struct stAreaSet
{
	struct stCoord
	{
		int		nUId;
		int		nType;
		float	fSX;
		float	fSZ;
		float	fEX;
		float	fEZ;
	} AreaCoord;

	struct stAreaEx
	{
		int	  nType;		// 타입
		int   nDir;			// 바라보고 있는 방향
		int   nCount;		// 개수
		int	  nAction;		// 행동

		char  szAreaName[AREA_STR_MAX];
	} AreaEx;

	stAreaSet()
	{
		memset( AreaEx.szAreaName, 0 , AREA_STR_MAX );
	}

	float	GetWidth(void)	{	return AreaCoord.fEX - AreaCoord.fSX;	}
	float	GetHeight(void)	{	return AreaCoord.fEZ - AreaCoord.fSZ;	}
};

struct msgRMTNotify
{
	int		nType;		// RMTYPE_WAVE,RMTYPE_EFFECT

	char	szKeyName[130];

	DWORD	dwParam1;
	DWORD	dwParam2;
	DWORD	dwParam3;
	DWORD	dwParam4;
	DWORD	dwParam5;
};
