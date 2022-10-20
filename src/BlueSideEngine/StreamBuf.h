
#ifndef _StreamBuf_h_
#define _StreamBuf_h_


// created by seo bong-soo 

// last updated 2005.11.8 by bong

#include <string>
#include <vector>
#include <algorithm>

//namespace gs2 
//{

	const int CHUNK_SIZE = 40;
	enum eStreamBufPos {
		STREAMBUF_BEGIN = 0,
		STREAMBUF_CUR,
		STREAMBUF_END,
	};


	class StreamBuf 
	{	
	public:		
		explicit StreamBuf(size_t capacity=CHUNK_SIZE) 
			: m_pBuf(new char[capacity]), m_capacity(capacity)	
		{
			_Reset();
		}
		StreamBuf(const StreamBuf &other) 
			:	m_pBuf(_NewCopy(other.m_pRead, other.Size())), 
				m_capacity(other.Size())		
		{
			m_pRead = m_pBuf;
			m_pWrite = m_pBuf + other.Size();
		}
		
		~StreamBuf() { delete []m_pBuf; }

		void	Seek(size_t offset, eStreamBufPos pos) {
			switch ( pos ) {
				case STREAMBUF_BEGIN: m_pRead = m_pBuf; break;
				case STREAMBUF_END : m_pRead = m_pWrite + offset; break;
				case STREAMBUF_CUR : m_pRead += offset; break;
				default : assert( false );
			}			
		}
		size_t	Tellg() const { return m_pRead - m_pBuf; }
		size_t	Size() const { return m_pWrite - m_pRead; }
		size_t	Capacity() const { return m_capacity; }
		bool	IsEmpty() const {	return m_pWrite == m_pRead; }
		void	Clear() {	_Reset();}
		void	Reserve(size_t capacity) 
		{
			if(m_capacity >= capacity)
				return;
			char *pNewBuf = new char[capacity];
			//try {				
				memcpy(pNewBuf, m_pRead, Size());
			//} catch (...) {
			//	delete []pNewBuf;
			//	throw;
			//}
			m_pWrite = pNewBuf + ( m_pWrite - m_pRead );
			m_pRead = pNewBuf;
			m_capacity = capacity;
			std::swap(m_pBuf, pNewBuf);
			delete []pNewBuf;
		}

		char* Data() const { return m_pRead; }

		void Write(const void *pSrc, size_t size) {
			_Write((const char*)pSrc, size, false);
		}
		size_t Read(void *pDest, size_t size) {
			return _Read((char*)pDest, size);
		}
		template<class T>
		void ReadOnly(T& dest) {
			memcpy(&dest, m_pRead, sizeof(T));
		}
		

		StreamBuf&	operator = ( const StreamBuf &other) 
		{
			char *pNewBuf = _NewCopy(other.m_pRead, other.Size());			
			m_capacity = other.Size();			
			m_pRead = pNewBuf;
			m_pWrite = pNewBuf + other.Size();
			std::swap(pNewBuf, m_pBuf);
			delete []pNewBuf;
			return *this;
		}

		template < class T > StreamBuf&	operator << ( const T &arg ) 
		{	
			_Write((const char*)&arg, sizeof(T));
			return *this;
		}
		template < class T > StreamBuf&	operator >> ( T &arg ) 
		{
			_Read((char*)&arg, sizeof(T));
			return *this;
		}
		/*StreamBuf&	operator << (const char *szStr) 
		{
			_Write(szStr, strlen(szStr)+1);
			return *this;
		}*/
		//! 포인터의 무효화 현상이 생기므로 구현과 사용을 하지 않는다.(string 으로 대체)
		//StreamBuf&	operator >> (char *szStr);

		StreamBuf&	operator << (const std::string &str) 
		{   
			_Write(str.c_str(), str.size()+1);
			return *this;
		}
		StreamBuf&	operator >> (std::string &str) 
		{
			str = m_pRead;
			m_pRead+=str.size()+1;
			if(IsEmpty()) _Reset();
			return *this;
		}
		template < class T >	StreamBuf& operator << (const std::vector<T> &vec) 
		{
			//_Write((const char*)&vec[0], vec.size()*sizeof(T), true);
			*this << size_t(vec.size());
			for (std::vector<T>::const_iterator citer=vec.begin (); citer != vec.end (); ++citer) 
				*this << *citer;

			return *this;
		}
		
		template < class T >	StreamBuf& operator >> (std::vector<T>	&vec) 
		{	
			size_t len;
			*this >> len;
			for ( std::size_t t = 0 ; t < len ; ++ t ){
				T obj ;
				*this >> obj ;
				vec.push_back ( obj ) ;
			}			
			return *this;
		}		

	private:
		void	_Reset() { m_pRead = m_pWrite = m_pBuf; }
		char*	_NewCopy(const char *pSrc, size_t size) {
			char *pNewBuf = new char[size];
			//try {
				memcpy(pNewBuf, pSrc, size);
			//}catch(...) {
			//	delete []pNewBuf;
			//	throw;
			//}
			return pNewBuf;
		}
		size_t	_EnoughChunkSize(size_t capacity) const 
		{
			return (capacity/CHUNK_SIZE + ((capacity%CHUNK_SIZE)?1:0))*CHUNK_SIZE;
		}
		void _Write(const char *pSrc, size_t size, bool bRecLen=false) 
		{
			if(m_capacity <= (m_pWrite-m_pBuf+size+(bRecLen)*sizeof(size_t))) {		
				_ReallocCopy(pSrc, size, bRecLen);
				return;
			}
			char *pWrite = m_pWrite;
			if(bRecLen) {	
				memcpy(pWrite, (const void*)&size, sizeof(size_t));
				pWrite += sizeof(size_t);
			}
			memcpy(pWrite, pSrc,size);
			m_pWrite = pWrite + size;			
		}
		size_t _Read(char *pDest, size_t size) 
		{
			// mruete: don't read past what's available
			size_t available = Size();
			if ( size > Size() )
				size = Size();

			memcpy(pDest, m_pRead, size);
			m_pRead+=size;
			if(IsEmpty()) 
				_Reset();
				
			return size;
		}

		void _ReallocCopy(const char *pSrc, size_t size, bool bRecLen) {
			size_t chunkSize = _EnoughChunkSize(Size()+size+(bRecLen)*sizeof(size_t));
			char *pNewBuf = new char[chunkSize];
			char *pWrite = pNewBuf;
			//try {										
				memcpy(pWrite, m_pRead, Size());
				pWrite += Size();
				if(bRecLen) {						
					memcpy(pWrite, (const void*)&size, sizeof(size_t));
					pWrite += sizeof(size_t);
				}
				memcpy(pWrite, pSrc, size);
				pWrite += size;
			//}catch(...) {
			//	delete []pNewBuf;
			//	throw;
			//}
			m_capacity = chunkSize;
			m_pWrite = pWrite;
			m_pRead = pNewBuf;				
			std::swap(m_pBuf, pNewBuf);
			delete []pNewBuf;				
			return;
		}


		size_t	m_capacity;
		char	*m_pBuf;		
		
		char	*m_pRead;
		char	*m_pWrite;		
	};


//}

#endif // _StreamBuf_h_