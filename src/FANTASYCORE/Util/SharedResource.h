#pragma once

template < class T , class Fn = LoadResourceFn >
class SharedResource
{
	struct ResPair {
		T *pData;
		char szFileName[255];
		int nRefCount;
	};
	std::vector< ResPair >    m_ResourceList;	
	Fn loadFn;

public:

	T* Load( const char *pFileName )
	{
		char szFileName[255];	// 폴더제외한 파일명만 저장
		const char *ps = pFileName;
		ps = strrchr( pFileName, '\\');
		if( ps )
		{
			strcpy( szFileName, ps+1 );
		}
		else
		{
			strcpy( szFileName, pFileName );
		}

		int i, nSize;
		// 이미 로드 하고 있다면 있는걸 리턴한다.
		nSize = ( int )m_ResourceList.size();
		for( i = 0; i < nSize; i++ )
		{
			if( _stricmp( szFileName, m_ResourceList[ i ].szFileName ) == 0 )
			{
				m_ResourceList[ i ].nRefCount++;
				return m_ResourceList[ i ].pData;
			}
		}

		// 새로운 파일인 경우
		ResPair resource;
		resource.pData = new T;
		loadFn.load( pFileName, resource.pData);
		resource.nRefCount = 1;

		strcpy(resource.szFileName, szFileName);

        m_ResourceList.push_back( resource );

		return resource.pData;
	}

	void Clear(T *pData)
	{
		int i, nSize;
		nSize = m_ResourceList.size();
		for( i = 0; i < nSize; i++ )
		{
			if( m_ResourceList[ i ].pData == pData )
			{
				m_ResourceList[ i ].nRefCount--;
				if( m_ResourceList[ i ].nRefCount == 0) {	// RefCount 가 0 이어도 지우지 않는다..미션 종료시에만 지운다.
					//delete m_ResourceList[i].pData;
					//m_ResourceList.erase( m_ResourceList.begin() + i);
				}
				break;
			}
		}
	}

	void ClearAll() // Map Clear 시에 해준다.
	{
		std::vector< ResPair >::iterator it = m_ResourceList.begin();
		for( ; it != m_ResourceList.end(); ++it) {
			if( it->nRefCount != 0 ) {
				DebugString( " Warning - (%s) : (%d) refCount Remain \n", it->szFileName, it->nRefCount);
			}
			delete it->pData;
		}		
		m_ResourceList.clear();
	}

	void DeleteAll() // 왠만하면 게임종료시에 해준다..
	{
		std::vector< ResPair >::iterator it = m_ResourceList.begin();		
		for( ; it != m_ResourceList.end(); ) {
				delete it->pData;                
		}		
		m_ResourceList.clear();
	}

};
