#pragma once

#include "Singleton.h"

#define MAX_ANI_CACHE_SIZE			4000
#define DEFAULT_ANI_CACHE_BONE_COUNT	32

struct ANI_CACHE_BUFFER
{
	int nLastHitFrame;
	int nCacheIndex;
	int nMatrixCount;
};

class CBsAniCache// : public CSingleton< CBsAniCache >
{
public:
	CBsAniCache( int nCacheSize, int nBoneCount );
	virtual ~CBsAniCache(void);

protected:
	int m_nAniCacheSize;
	int m_nBoneCount;
	int m_nEmptyCacheCount;
	int *m_pEmptyCacheList;
	std::map< unsigned int, ANI_CACHE_BUFFER > m_AniCacheMap;
	D3DXMATRIX **m_ppAniCacheList;

public:
	unsigned int GenerateKey( int nAniFileIndex, int nAniIndex, float fFrame );
	void CacheAni( int nAniFileIndex, int nAniIndex, float fFrame, D3DXMATRIX *pAniCash, int nCount );
	D3DXMATRIX *FindCacheAni( int nAniFileIndex, int nAniIndex, float fFrame, int &nCacheCount );
	void ClearCache() 
	{ 
		int i;

		m_nEmptyCacheCount = m_nAniCacheSize; 
		m_AniCacheMap.clear(); 
		for( i = 0; i < m_nEmptyCacheCount; i++ )
		{
			m_pEmptyCacheList[ i ] = i;
		}
	}
	void EraseGarbageCash();
	int CacheBufferSize() { return m_nBoneCount; }
};

extern CBsAniCache g_BsAniCache;
extern CBsAniCache g_LargeBsAniCache;
