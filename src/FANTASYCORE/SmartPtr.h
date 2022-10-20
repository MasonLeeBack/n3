#pragma once

#include <vector>

template <class T>
class CSmartPtr
{
public:
	CSmartPtr() 
	{	
		m_nHandle=0;
		m_pInstance=0;

		nDebugRef = 0;
	}
	CSmartPtr(int nHandle, T *pInstance) 
	{
		m_nHandle=nHandle;
		m_pInstance=pInstance;

		nDebugRef = 0;
	}
	template<class T2>
	CSmartPtr(CSmartPtr<T2> &Ptr) 
	{
		m_nHandle=Ptr.m_nHandle;
		m_pInstance=reinterpret_cast<T *>(Ptr.m_pInstance);

		nDebugRef = 0;
	}
	~CSmartPtr() {}

public:
	union
	{
		int m_nHandle;
#ifdef _XBOX
		struct
		{
			short int m_nID;
			short int m_nIndex;
		};
#else
		struct
		{
			short int m_nIndex;
			short int m_nID;
		};
#endif
	};
	T *m_pInstance;


	int nDebugRef;		// debug

public:
	void Identity() 
	{
		m_nHandle=0;
		m_pInstance=0;

		if( nDebugRef > 0 )
		{
			assert(0);
		}
	}

	int GetHandle() { return m_nHandle; }
	int GetIndex()	{ return m_nIndex; }
	int GetID()		{ return m_nID; }
	T *GetPointer() { return m_pInstance; }
	operator T*()
	{
		return m_pInstance;
	}
	bool operator!() const
	{
		if(m_pInstance){
			return false;
		}
		else{
			return true;
		}
	}
	T *operator->()
	{
		return(m_pInstance);
	}
	T &operator*()
	{
		return(*m_pInstance);
	}

	template <class T2>
	CSmartPtr<T> &operator=(const CSmartPtr<T2> &Sour)
	{
		m_nHandle=Sour.m_nHandle;
		m_pInstance=reinterpret_cast<T *>(Sour.m_pInstance);

		return *this;
	}
	template <class T2>
	bool operator==(const CSmartPtr<T2> &Sour)
	{
		if((m_nHandle==Sour.m_nHandle)&&(m_pInstance==reinterpret_cast<T *>(Sour.m_pInstance))){
			return true;
		}
		else{
			return false;
		}
	}
	template <class T2>
	bool operator!=(const CSmartPtr<T2> &Sour)
	{ 
		return(!(*this==Sour)); 
	}
};

template <class T>
class CSmartPtrMng
{
public:
	CSmartPtrMng(int nPoolSize)
	{
		int i;

		m_SmartPtrList.resize(nPoolSize);
		for(i=0;i<nPoolSize;i++){
			m_SmartPtrList[i]=CSmartPtr<T>(i+1, 0);
		}
		m_nFreeIndex=0;
		m_nCount = 0;
	}
	~CSmartPtrMng()
	{
	}
	void Reset()
	{
		int i, nPoolSize;

		nPoolSize = m_SmartPtrList.size();
		for( i = 0; i < nPoolSize; i++ )
		{
			m_SmartPtrList[ i ] = CSmartPtr< T >( i + 1, 0 );
		}
		m_nFreeIndex = 0;
		m_nCount = 0;
	}

	int m_nCount;
protected:
	int m_nFreeIndex;
	std::vector< CSmartPtr<T> > m_SmartPtrList;

public:
	CSmartPtr<T> operator[] (int nID)
	{
		return m_SmartPtrList[nID];
	}
	int Size()
	{
		return (int)m_SmartPtrList.size();
	}
	int GetHandle(int nIndex)
	{
		return m_SmartPtrList[nIndex].GetHandle();
	}
	T *GetPointer(int nHandle)
	{
		return m_SmartPtrList[nHandle&0xffff].GetPointer();
	}
	CSmartPtr<T> &CreateHandle(T *pInstance)
	{
		assert(pInstance);

		int nHandle, nID, nNextFreeIndex, nRetIndex;

		if(m_nFreeIndex>=(int)m_SmartPtrList.size()){
			m_SmartPtrList.resize(m_nFreeIndex+1);
			m_SmartPtrList[m_nFreeIndex] = CSmartPtr<T>(m_nFreeIndex+1, 0);
		}
		nNextFreeIndex=m_SmartPtrList[m_nFreeIndex].GetIndex();
		nID=m_SmartPtrList[m_nFreeIndex].GetID();
		nID++;
		nHandle=(nID<<16)+m_nFreeIndex;
		m_SmartPtrList[m_nFreeIndex]=CSmartPtr<T>(nHandle, pInstance);
		nRetIndex=m_nFreeIndex;
		m_nFreeIndex=nNextFreeIndex;
		m_nCount++;

		return m_SmartPtrList[nRetIndex];
	}

	void DeleteHandle( int nHandle )
	{
		assert( nHandle == m_SmartPtrList[ nHandle & 0xffff ].GetHandle() );

		int nIndex, nID;

		nIndex = nHandle & 0xffff;
		nID = nHandle & 0xffff0000;
		m_SmartPtrList[ nIndex ] = CSmartPtr< T >( nID + m_nFreeIndex, 0 );
		m_nFreeIndex = nIndex;
		m_nCount--;
		if( m_nCount <= 0 )
		{
			Reset();
		}
	}
};