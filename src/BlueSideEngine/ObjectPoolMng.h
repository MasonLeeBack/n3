#pragma once

template < class T >
class CObjectPoolMng
{
public:
	CObjectPoolMng( int nPoolSize )
	{
		int i;

		Resize( nPoolSize );
		for( i = 0; i < nPoolSize; i++ )
		{
			m_ObjectList[ i ] = new T;
			m_HandleList[ i ] = i;
			m_nEmptyList.push_back( i );
		}

		m_nCount = 0;
	}
	~CObjectPoolMng()
	{
		int i, nSize;

		nSize = ( int )m_ObjectList.size();
		for( i = 0; i < nSize; i++ )
		{
			delete m_ObjectList[ i ];
		}
	}

protected:
	int m_nCount;
	std::vector< T * > m_ObjectList;
	std::vector< int > m_HandleList;
	std::vector< int > m_nEmptyList;

public:
	int Count()
	{
		return m_nCount;
	}
	void Resize( int nSize )
	{
		m_ObjectList.resize( nSize );
		m_HandleList.resize( nSize );
		m_nEmptyList.reserve( nSize );
	}
	T *operator[] ( int nID )
	{
		return m_ObjectList[ nID ];
	}
	int Size()
	{
		return ( int )m_ObjectList.size();
	}
	int GetHandle( int nIndex )
	{
		return m_HandleList[ nIndex ];
	}
	int CreateObject()
	{
		int nEmptyIndex;

		if( m_nEmptyList.size() <= 0 )
		{
			int nSize;

			nSize = m_ObjectList.size();
			Resize( nSize + 1 );
			m_ObjectList[ nSize ] = new T;
			m_HandleList[ nSize ] = nSize;
			m_nEmptyList.push_back( nSize );
		}
		nEmptyIndex = m_nEmptyList.back();
		m_nEmptyList.pop_back();
		m_nCount++;

		return m_HandleList[ nEmptyIndex ];
	}

	void DeleteObject( int nHandle )
	{
		int nIndex;

		if( m_nCount )
		{
			nIndex = nHandle & 0xffff;
			if( m_HandleList[ nIndex ] == nHandle )
			{
				m_HandleList[ nIndex ] += 0x10000;
				m_nCount--;
				m_nEmptyList.push_back( nIndex );
			}
		}
	}
};