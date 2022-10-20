#include "stdafx.h"
#include "bstreamext.h"
#include "BsFileIO.h"

int BStream::Move( BStream& srcStream, unsigned long size )
{
	char* pBuf = new char[ size ];
	int length = srcStream.Read( pBuf, size );
	Write( pBuf, length );
	delete[] pBuf;
	return length;
}

void BStream::SwapByte(char *pByte1, char *pByte2)
{
	char cTemp;

	cTemp=*pByte1;
	*pByte1=*pByte2;
	*pByte2=cTemp;
}

void BStream::ConvertEndian(int nEndianType, char *pBuf, int nSize)
{
	int i, nCount;

	if(nEndianType==0){
		return;
	}
	nCount=nSize/nEndianType;
	for(i=0;i<nCount;i++){
		switch(nEndianType){
			case ENDIAN_TWO_BYTE:
				SwapByte(pBuf+i*nEndianType, pBuf+i*nEndianType+1);
				break;
			case ENDIAN_FOUR_BYTE:
				SwapByte(pBuf+i*nEndianType, pBuf+i*nEndianType+3);
				SwapByte(pBuf+i*nEndianType+1, pBuf+i*nEndianType+2);
				break;
			case ENDIAN_EIGHT_BYTE:
				SwapByte(pBuf+i*nEndianType, pBuf+i*nEndianType+7);
				SwapByte(pBuf+i*nEndianType+1, pBuf+i*nEndianType+6);
				SwapByte(pBuf+i*nEndianType+2, pBuf+i*nEndianType+5);
				SwapByte(pBuf+i*nEndianType+3, pBuf+i*nEndianType+4);
				break;
		}
	}
}



BFileStream::BFileStream( const char* filename, int mode )
{
	DWORD dwDesiredAccess;
	DWORD dwCreationDisposition;

    switch ( mode ) {
        case openWrite :
			dwDesiredAccess = GENERIC_READ|GENERIC_WRITE;
			dwCreationDisposition = CREATE_ALWAYS;
            break;

        case openReadWrite :
			dwDesiredAccess = GENERIC_READ|GENERIC_WRITE;
			dwCreationDisposition = OPEN_ALWAYS;
            break;

        case create :
			dwDesiredAccess = GENERIC_WRITE;
			dwCreationDisposition = CREATE_ALWAYS;
            break;

        case openRead :
        default :
			dwDesiredAccess = GENERIC_READ;
			dwCreationDisposition = OPEN_EXISTING;
            break;
    }

	hFile = CBsFileIO::BsCreateFile( filename, dwDesiredAccess, 0, 0, dwCreationDisposition, 0, 0);
	m_cur = 0;

//	assert(handle!=-1 && "BFileStream 파일 오픈 실패");	// 추가의 에러 메세지가 있다면 assert 창에서 skip 을 클릭 합니다 
}

BFileStream::~BFileStream()
{
	Close();
}

void BFileStream::Close(void)
{
	if ( hFile != INVALID_HANDLE_VALUE )
	{
		CBsFileIO::BsCloseFileHandle( hFile );
		hFile = INVALID_HANDLE_VALUE;
	}
	m_cur = 0;
}

int BFileStream::Seek( long offset, int origin )
{
	BsAssert(hFile != INVALID_HANDLE_VALUE);
	m_cur = CBsFileIO::BsSetFilePointer( hFile, offset, 0, (DWORD) origin );
	return m_cur;
}

unsigned long BFileStream::Read( void* ptr, unsigned long size, int nEndianType/*=0*/, bool bAllowPartial/*=false*/ )	// mruete: added bAllowPartial parameter
{
	BsAssert(hFile != INVALID_HANDLE_VALUE);

	DWORD dwBytesRead;

	if ( !CBsFileIO::BsReadFile(hFile, ptr, size, &dwBytesRead, 0) )
		return 0;

	m_cur += dwBytesRead;
	
	// mruete: prefix bugs: if a partial read is not requested, return 0.
	if ( dwBytesRead != size && !bAllowPartial )
	{
#if !defined(_PREFIX_) && !defined(_PREFAST_)
		// Hide the assert from prefix/prefast so that we don't hide potentially nasty cases
		BsAssert( "Unexpected partial file read" && 0);
#endif
		return 0;
	}

#ifndef _XBOX
	ConvertEndian(nEndianType, (char *)ptr, size);
#endif // #ifndef _XBOX

	return dwBytesRead;
}

int BFileStream::Write( const void *ptr, unsigned long size, int nEndianType/*=0*/ )
{
	BsAssert(hFile != INVALID_HANDLE_VALUE);

	DWORD dwBytesWritten;
	int nRet;

#ifndef _XBOX
	char *pBuf;

	pBuf=new char[size];
	memcpy(pBuf, ptr, size);
	ConvertEndian(nEndianType, pBuf, size);
	nRet = CBsFileIO::BsWriteFile(hFile, pBuf, size, &dwBytesWritten, 0);
	delete [] pBuf;

#else
	nRet = CBsFileIO::BsWriteFile(hFile, ptr, size, &dwBytesWritten, 0);
#endif // #ifndef _XBOX

	m_cur += dwBytesWritten;
	return nRet;
}


bool BFileStream::Eos () const
{
	BsAssert(hFile != INVALID_HANDLE_VALUE);
	return m_cur == Length();
}

unsigned long BFileStream::Tell() const
{
	BsAssert(hFile != INVALID_HANDLE_VALUE);
	return m_cur;
}

unsigned long BFileStream::Length() const
{
	BsAssert(hFile != INVALID_HANDLE_VALUE);
	return CBsFileIO::BsGetFileSize( hFile, NULL);

}

bool BFileStream::Valid() const
{
	return(hFile != INVALID_HANDLE_VALUE);
}



/////////////////////////////////////////////////////////


BMemoryStream::BMemoryStream()
{
	m_pBuf = NULL;
	m_size = 0;
	m_cur = 0;
}


BMemoryStream::BMemoryStream( void* pBuf, unsigned long size )
{
	m_pBuf = static_cast< char* >( pBuf );
	m_size = size;
	m_cur = 0;
}

void BMemoryStream::Initialize( void* buf, unsigned long size)
{
	assert( m_pBuf == NULL );
	assert( m_size == 0 );
	assert( m_cur == 0 );

	m_pBuf = static_cast< char* >( buf );
	m_size = size;
	m_cur = 0;
}



int BMemoryStream::Seek( long offset, int origin )
{
	switch ( origin ) {
		case fromBegin : m_cur = offset; break;
		case fromNow   : m_cur += offset; break;
		case fromEnd   : m_cur = m_size+offset; break;
		default : assert( false ); return 0;
	}
	if ( m_cur >= m_size ) m_cur = m_size;
	return m_cur;
}

unsigned long BMemoryStream::Read ( void *ptr, unsigned long size, int nEndianType/*=0*/, bool bAllowPartial/*=false*/ )	// mruete: added bAllowPartial parameter )
{
	if ( m_cur + size > m_size )
	{
		// mruete: prefix bugs: If a partial read is not requested, return failure.
		if ( !bAllowPartial )
		{
#if !defined(_PREFIX_) && !defined(_PREFAST_)
			// Hide the assert from prefix/prefast so that we don't hide potentially nasty cases
			//BsAssert( "Unexpected partial file read" && 0);
#endif
			return 0;
		}
		size = m_size-m_cur;
	}
	
	memcpy( ptr, m_pBuf + m_cur, size );

#ifndef _XBOX
	ConvertEndian(nEndianType, (char*)ptr, size);
#endif // #ifndef _XBOX

	m_cur += size;
	return size;
}

int BMemoryStream::Write ( const void *ptr, unsigned long size, int nEndianType/*=0*/ )
{
	if ( m_cur + size > m_size ) size = m_size-m_cur;
#ifndef _XBOX
	char *pBuf;

	pBuf=new char[size];
	memcpy(pBuf, ptr, size);
	ConvertEndian(nEndianType, pBuf, size);
	memcpy( m_pBuf + m_cur, pBuf, size );
	delete [] pBuf;
#else
	memcpy( m_pBuf + m_cur, ptr, size );
#endif // #ifndef _XBOX
	m_cur += size;
	return size;
}

int BMemoryStream::Move( BStream& srcStream, unsigned long size )
{
	if ( m_cur + size > m_size ) size = m_size-m_cur;
	size = srcStream.Read( m_pBuf + m_cur, size );
	m_cur += size;
	return size;
}

bool BMemoryStream::Eos () const
{
	return m_cur == m_size;
}

unsigned long BMemoryStream::Tell () const
{
	return m_cur;
}

unsigned long BMemoryStream::Length () const
{
	return m_size;
}

bool BMemoryStream::Valid() const
{
	return m_pBuf != static_cast< char* >(0);
}

/////////////////////////////////////////////////////////
/*
BBufferStream::BBufferStream( unsigned long capacity, unsigned int step )
    : BMemoryStream( static_cast< char* >( malloc( capacity ) ), 0 ) 
{
	m_capacity = capacity;
	m_step = step;
}

BBufferStream::BBufferStream( BStream* src, unsigned long size, unsigned int step )
	: BMemoryStream( static_cast< char* >(0), 0 ) 
{
	m_capacity = size == 0 ? src->Length() : size;
	m_step = step;
	m_pBuf = static_cast< char* >( malloc( m_capacity ) );
	Read( m_pBuf, m_capacity );
}

BBufferStream::~BBufferStream() {
	free( m_pBuf );
}

void BBufferStream::Clear()
{
	m_size = 0;
	m_cur = 0;
}

void BBufferStream::resize(unsigned int newsize)
{
	m_capacity = ((newsize-1) / m_step + 1) * m_step;
	m_pBuf = static_cast< char* >( realloc( m_pBuf, m_capacity ) );

	if ( m_pBuf == static_cast< char* >(0) ) 
		throw mem_overflow();			// memory overflow (unlikely but possible even on Win32)
}

int BBufferStream::Write ( const void *ptr, unsigned long size )
{
	if ( m_cur + size > m_size ) {
		m_size = m_cur+size;
		if ( m_size > m_capacity ) 
			resize( m_size );
	}
	memcpy( m_pBuf + m_cur, ptr, size );
	m_cur += size;
	return size;
}

int BBufferStream::Move( BStream& srcStream, unsigned long size )
{
	if ( m_cur + size > m_size ) {
		if ( m_cur + size > m_capacity ) 
			resize( m_cur + size );
	}
	size = srcStream.Read( m_pBuf + m_cur, size );
	m_cur += size;
	if ( m_size < m_cur )
		m_size = m_cur;
	return size;
}*/

/////////////////////////////////////////////////////////

/*enum {
	F_OWN_SRC = 1,
	F_VALID   = 2
};

BSubStream::BSubStream( BStream* src, bool bOwnSrc )
{
	m_flag = ( (src != NULL && src->Valid()) ? F_VALID : 0 ) | ( bOwnSrc ? F_OWN_SRC : 0 );
	if ( m_flag & F_VALID ) {
		m_src = src;
		m_startPos = src->Tell();
		m_length = src->Length() - m_startPos;
	} else {
		m_startPos = m_length = 0;
		m_src = NULL;
	}
	m_cur = 0;
}

BSubStream::BSubStream( BStream* src, long length, bool bOwnSrc )
{
	m_flag = ( (src != NULL && src->Valid()) ? F_VALID : 0 ) | ( bOwnSrc ? F_OWN_SRC : 0 );
	if ( m_flag & F_VALID ) {
		m_src = src;
		m_startPos = src->Tell();
		m_length = src->Length() - m_startPos;
		if ( m_length > length ) m_length = length;
	} else {
		m_startPos = m_length = 0;
		m_src = NULL;
	}
	m_cur = 0;
}

BSubStream::~BSubStream()
{
	if ( m_flag & F_OWN_SRC ) 
		delete m_src;
}

int BSubStream::Seek( long offset, int origin )
{
	switch ( origin ) {
		case fromNow :
			m_cur += offset;
			break;
		case fromBegin :
			m_cur = offset;
			break;
		case fromEnd :
			m_cur = offset + m_length;
			break;
		default :
			assert( false );
			return 0;
	}

	if ( m_cur < 0 ) 
		m_cur = 0;
	else if ( m_cur >= m_length ) 
		m_cur = m_length;

	return m_src->Seek( m_cur + m_startPos, fromBegin );
}

int BSubStream::Read ( void* ptr, unsigned long size )
{
	if ( long(m_cur + size) > m_length )
		size = m_length - m_cur;
	m_cur += size;
	return m_src->Read( ptr, size );
}

bool BSubStream::Valid() const
{
	return (m_flag & F_VALID) != 0;
}
*/