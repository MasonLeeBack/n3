#include "stdafx.h"
#include "BsKernel.h"
#include ".\bsaniCache.h"

CBsAniCache g_BsAniCache( MAX_ANI_CACHE_SIZE, DEFAULT_ANI_CACHE_BONE_COUNT );
CBsAniCache g_LargeBsAniCache( 300, 66 );

CBsAniCache::CBsAniCache( int nCacheSize, int nBoneCount )
{
	int i;

	m_nBoneCount = nBoneCount;
	m_nAniCacheSize = nCacheSize;
	m_pEmptyCacheList = new int[ m_nAniCacheSize ];
	m_ppAniCacheList = new D3DXMATRIX *[ m_nAniCacheSize ];
	for( i = 0; i < m_nAniCacheSize; i++ )
	{
		m_ppAniCacheList[ i ] = new D3DXMATRIX[ m_nBoneCount ];
		m_pEmptyCacheList[ i ] = i;
	}
	m_nEmptyCacheCount = m_nAniCacheSize;
}

CBsAniCache::~CBsAniCache(void)
{
	int i;

	if( m_pEmptyCacheList )
	{
		delete [] m_pEmptyCacheList;
	}
	if( m_ppAniCacheList )
	{
		for( i = 0; i < m_nAniCacheSize; i++ )
		{
			if( m_ppAniCacheList[ i ] )
			{
				delete [] m_ppAniCacheList[ i ];
			}
		}
		delete [] m_ppAniCacheList;
	}
}

unsigned int CBsAniCache::GenerateKey( int nAniFileIndex, int nAniIndex, float fFrame )
{
	unsigned int nKey;

	nKey = ( nAniFileIndex & 0xff ) << 24;
	nKey += ( nAniIndex & 0xff ) << 16;
	nKey += ( int )fFrame;

	return nKey;
}

void CBsAniCache::CacheAni( int nAniFileIndex, int nAniIndex, float fFrame, D3DXMATRIX *pAniCache, int nCount )
{
	BsAssert( nCount <= m_nBoneCount );

	unsigned int nKey;
	int nEmptyIndex;
	std::map< unsigned int, ANI_CACHE_BUFFER >::iterator	it;
	ANI_CACHE_BUFFER CacheBuffer;

	if( m_nEmptyCacheCount <= 0 )
	{
		return;
	}
	nKey = GenerateKey( nAniFileIndex, nAniIndex, fFrame );
	it = m_AniCacheMap.find( nKey );
	if( it != m_AniCacheMap.end() )
	{
		return;
	}
	m_nEmptyCacheCount--;
	nEmptyIndex = m_pEmptyCacheList[ m_nEmptyCacheCount ];
	memcpy( m_ppAniCacheList[ nEmptyIndex ], pAniCache, sizeof( D3DXMATRIX ) * nCount );

	CacheBuffer.nCacheIndex = nEmptyIndex;
	CacheBuffer.nLastHitFrame = g_BsKernel.GetRenderTickByProcess();
	CacheBuffer.nMatrixCount = nCount;

	m_AniCacheMap.insert( std::pair< unsigned int, ANI_CACHE_BUFFER >( nKey, CacheBuffer ) );
}

D3DXMATRIX *CBsAniCache::FindCacheAni( int nAniFileIndex, int nAniIndex, float fFrame, int &nCacheCount )
{
	BsAssert( nAniFileIndex <= 255 && nAniIndex <= 255 );

	unsigned int nKey;
	std::map< unsigned int, ANI_CACHE_BUFFER >::iterator	it;

	nKey = GenerateKey( nAniFileIndex, nAniIndex, fFrame );
	it = m_AniCacheMap.find( nKey );
	if( it != m_AniCacheMap.end() )
	{
		int nFindIndex;

		nFindIndex = it->second.nCacheIndex;
		nCacheCount = it->second.nMatrixCount;
		it->second.nLastHitFrame = g_BsKernel.GetRenderTickByProcess();
		return m_ppAniCacheList[ nFindIndex ];
	}

	return NULL;
}

// 이 함수가 Kernel내 InitRender()외의 곳에서 호출되어야 한다면, GetRenderTickByProcess()의 변경때문에 수정이 필요합니다.
void CBsAniCache::EraseGarbageCash()
{
	if( m_nEmptyCacheCount > ( int )( m_nAniCacheSize * 0.05f ) )
	{
		return;
	}

	int nCurrentTick;
	std::map< unsigned int, ANI_CACHE_BUFFER >::iterator it;

	nCurrentTick = g_BsKernel.GetRenderTickByProcess();
	it = m_AniCacheMap.begin();
	while( it != m_AniCacheMap.end() )
	{
		if( nCurrentTick - it->second.nLastHitFrame > 10 )
		{
			m_pEmptyCacheList[ m_nEmptyCacheCount ] = it->second.nCacheIndex;
			m_nEmptyCacheCount++;
			it = m_AniCacheMap.erase( it );
			continue;
		}
		it++;
	}
}
