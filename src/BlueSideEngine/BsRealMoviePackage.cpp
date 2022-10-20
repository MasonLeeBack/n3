#include "stdafx.h"

#include "BsRealMovieObject.h"
#include "BsrealMovie.h"
#include "BsRealMovieMsg.h"
#include "BsKernel.h"
#include <io.h>

#include "BsRealMoviePackage.h"

char* g_lpszPackageStr[] = 
{
	"Not",
	"Effect",
	"Wave",
	"Lip",
};


stLightPack::stLightPack()
{
	memset( NameBlock.szLightName, 0 , LIGHT_STR_MAX );
}

void	stLightPack::Save(BStream *pStream,int nNBSize,int nDBSize)
{
	pStream->Write( NameBlock.szLightName, nNBSize, 0 );

	pStream->Write( &DataBlock.crossLight.m_XVector, sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
	pStream->Write( &DataBlock.crossLight.m_YVector, sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
	pStream->Write( &DataBlock.crossLight.m_ZVector, sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
	pStream->Write( &DataBlock.crossLight.m_PosVector, sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
}

void	stLightPack::Load(BStream *pStream,int nNBSize,int nDBSize)
{
	pStream->Read( NameBlock.szLightName, nNBSize, 0 );

	pStream->Read( &DataBlock.crossLight.m_XVector, sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
	pStream->Read( &DataBlock.crossLight.m_YVector, sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
	pStream->Read( &DataBlock.crossLight.m_ZVector, sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
	pStream->Read( &DataBlock.crossLight.m_PosVector, sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
}


void	stPointLightPack::Save(BStream *pStream)
{
	int nBlockSize;
	nBlockSize = sizeof(NameBlock);	//eBonePackSize;		// String 을 뺀 Bone Class 크기 저장.

	pStream->Write( &nBlockSize, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Write( &NameBlock, nBlockSize, 0 );

	nBlockSize = sizeof(DataBlock);
	pStream->Write( &nBlockSize, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Write( &DataBlock, nBlockSize, ENDIAN_FOUR_BYTE );
}

void	stPointLightPack::Load(BStream *pStream)
{
	int nBlockSize;
	memset( NameBlock.szLightName, 0, POINT_LIGHT_STR_MAX );
	pStream->Read( &nBlockSize, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Read( &NameBlock, nBlockSize, 0 );

	pStream->Read( &nBlockSize, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Read( &DataBlock, nBlockSize, ENDIAN_FOUR_BYTE );
}

//////////////////////////////////////////////////////////////////////////
//
//	CRMPackage
//
//////////////////////////////////////////////////////////////////////////
void	CRMPackage::Save(BStream *pStream)
{
	int nItemCnt = eItemMax;
	int nSubDataSize = sizeof( SubData );
	int nLen = strlen( m_szName );

	pStream->Write( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );
	if( nLen )
        pStream->Write( m_szName, nLen, 0 );

	pStream->Write( &nSubDataSize, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Write( &SubData, nSubDataSize, ENDIAN_FOUR_BYTE );

	pStream->Write( &nItemCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	for( int i = 0 ; i < nItemCnt ; ++i )
	{
		m_Item[i].Save( pStream );
	}
}

void	CRMPackage::Load(BStream *pStream)
{
	int nLen;
	int nItemCnt;
	int nSubDataSize;

	pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );

	memset( m_szName, 0, eFileNameLength );

	if( nLen )
	{
		pStream->Read( m_szName, nLen, 0 );
	}

	pStream->Read( &nSubDataSize, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Read( &SubData, nSubDataSize, ENDIAN_FOUR_BYTE );

	pStream->Read( &nItemCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	for( int i = 0 ; i < nItemCnt ; ++i )
	{
		m_Item[i].Load( pStream );
	}
}

void	CRMPackage::RecalcItems(void)
{
	m_nItemCnt = 0;

	for( int i = 0 ; i < eItemMax ; ++i )
	{
		m_nItemCnt += ( m_Item[i].DataBlock.bUse ? 1 : 0 );
	}
}

#ifdef _USAGE_TOOL_

extern VOID	ChangeNewTickIntoTime(DWORD& dwTick,float& fTime);

void	CRMPackage::ChangeFPS(void)
{
	ChangeNewTickIntoTime( SubData.m_dwTick, SubData.m_fTime );
}
#endif

//////////////////////////////////////////////////////////////////////////
//
//	CRMPackage
//
//////////////////////////////////////////////////////////////////////////
void	stPackageItem::Save(BStream *pStream)
{
	int nSize = sizeof( DataBlock );
	pStream->Write( &nSize, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Write( &DataBlock, nSize, ENDIAN_FOUR_BYTE );

	nSize = sizeof( NameBlock );
	pStream->Write( &nSize, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Write( &NameBlock, nSize, 0 );
}

void	stPackageItem::Load(BStream *pStream)
{
	int nSize;
	pStream->Read( &nSize, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Read( &DataBlock, nSize, ENDIAN_FOUR_BYTE );

	pStream->Read( &nSize, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Read( &NameBlock, nSize , 0 );
}