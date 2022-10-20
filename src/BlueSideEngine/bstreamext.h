#ifndef __BLUE_STREAM_EXT__
#define __BLUE_STREAM_EXT__

/*
 *		bstream.h
 *
 *  This module defines general stream class and several specific
 *  stream classes.
 *
 *  Created in 1998/2 by Young-Hyun Joo
 *
 *  v2 Created in 2000/8 for use in bcs
 *        by Young-Hyun Joo
 *
 *  Last modified on 2002/3/5
 *        by Young-Hyun Joo
 */

/*
 *		BStream
 *
 *	Abstract stream class
 */

#include "StreamBuf.h"
#include <memory>

#define ENDIAN_TWO_BYTE				0x000000002
#define ENDIAN_FOUR_BYTE			0x000000004
#define ENDIAN_EIGHT_BYTE			0x000000008

class BStream
{
public:
    virtual ~BStream() {}

	enum {
		fromBegin	= FILE_BEGIN,	// seek from the begining of stream
		fromNow		= FILE_CURRENT,	// seek from the current positin
		fromEnd		= FILE_END		// seek from the end of stream
	};
	virtual int Seek( long offset, int origin ) = 0;
	
	// mruete: added bAllowPartial parameter, and improved consistency
	// set bAllowPartial to true to allow less than size bytes to be read
	// return value is the actual number of bytes read (will equal size or 0 if bAllowPartial is false)
	virtual unsigned long Read( void* ptr, unsigned long size, int nEndianType=0, bool bAllowPartial=false ) = 0;
		
	virtual int Write( const void* ptr, unsigned long size, int nEndianType=0 ) = 0;
	virtual int Move( BStream& srcStream, unsigned long size );
	virtual bool Eos() const = 0;				// if stream is on EOF	virtual unsigned long Tell() const = 0;
	virtual unsigned long Tell() const = 0;
	virtual unsigned long Length() const = 0;
	virtual bool Valid() const = 0;
	operator bool() const { return Valid(); }
	static void SwapByte(char *pByte1, char *pByte2);
	void ConvertEndian(int nEndianType, char *pBuf, int nSize);

protected:
	DWORD m_cur;

};

/*
 *		BFileStream
 *
 *	Stream class which wraps C low-level file I/O routines.
 */

class BFileStream : public BStream
{
public:
	enum {
        openRead      = 1<<0,
        openWrite     = 2<<0,
        openReadWrite = 3<<0,
        create        = 4<<0		// creates NEW file stream (overwrites exist one)
	};

	BFileStream(const char* filename, int mode = openRead );
	virtual ~BFileStream();

	virtual int Seek( long offset, int origin );
	virtual unsigned long Read( void* ptr, unsigned long size, int nEndianType=0, bool bAllowPartial=false );	// mruete: added bAllowPartial parameter
	virtual int Write( const void* ptr, unsigned long size, int nEndianType=0 );
	virtual bool Eos() const;
	virtual unsigned long Tell() const;
	virtual unsigned long Length() const;
	virtual bool Valid() const;

	// 추가
	void	Close(void);	// 소멸자에만 close 가 있는 관계로 소멸되기전 파일 열기시 공유 위반이 발생함.
private:
	HANDLE hFile;
};

/*
 *		BMemoryStream
 *
 *	A stream class which wraps fixed range of memory block. BMemoryStream do NOT
 *  owns the given memory block.
 */

class BMemoryStream : public BStream
{
public:
	BMemoryStream();
	BMemoryStream(void* buf, unsigned long size);

	void Initialize( void* buf, unsigned long size);

	virtual int Seek( long offset, int origin );
	virtual unsigned long Read( void* ptr, unsigned long size, int nEndianType=0, bool bAllowPartial=false );	// mruete: added bAllowPartial parameter
	virtual int Write( const void* ptr, unsigned long size, int nEndianType=0 );
	virtual int Move( BStream& srcStream, unsigned long size );
	virtual bool Eos() const;
	virtual unsigned long Tell() const;
	virtual unsigned long Length() const;
	virtual bool Valid() const;

	void* Buf() { return static_cast<void*>(m_pBuf); }
	const void* Buf() const { return static_cast<const void*>(m_pBuf); }
	const char* CBuf() const { return (const char*)m_pBuf; }
	operator void* () { return static_cast<void*>(m_pBuf); }
	operator const void* () const { return static_cast<const void*>(m_pBuf); }

protected:
	char *m_pBuf;
	unsigned long m_size;
};


class BBufStream : public BStream 
{
public:
	explicit BBufStream(size_t capacity = CHUNK_SIZE) 
		:m_buf(capacity)
	{}

	// BStream
	virtual int Seek( long offset, int origin ){ 
		switch ( origin ) {
			case fromBegin	: m_buf.Seek(offset, STREAMBUF_BEGIN); break;
			case fromNow	: m_buf.Seek(offset, STREAMBUF_CUR); break;
			case fromEnd	: m_buf.Seek(offset, STREAMBUF_END); break;
			default : BsAssert(0); return 0;
		}
		return (int)m_buf.Tellg();
	}	
	
	virtual unsigned long Read( void* ptr, unsigned long size, int nEndianType=0, bool bAllowPartial=false ) {	// mruete: added bAllowPartial parameter
        unsigned int uRead = m_buf.Read(ptr, size);
        
        // mruete: check to make sure we got what we wanted
        if ( uRead != size && !bAllowPartial )
		{
#if !defined(_PREFIX_) && !defined(_PREFAST_)
			// Hide the assert from prefix/prefast so that we don't hide potentially nasty cases
			BsAssert( "Unexpected partial file read" && 0);
#endif
			return 0;
		}
		// should this only do endian conversion for (non)XBox?
		ConvertEndian(nEndianType, (char*)ptr, size);
		return uRead;
	}
	
	virtual int Write( const void* ptr, unsigned long size, int nEndianType=0 ) {
// [beginmodify] 2006/2/3 junyash PS#4609 detect new[] and delete mis-match
		#if 0
		std::auto_ptr<char> temp(new char[size]);
		memcpy((void*)temp.get(), ptr, size);
		ConvertEndian(nEndianType, temp.get(), size);
		m_buf.Write(temp.get(), size);
		#else
		// auto_ptr uses delete in template... that's not match with array new[]
		// delate char[] is stable but delete[] to make sure.
		char* temp = new char[size];
		memcpy((void*)temp, ptr, size);
		ConvertEndian(nEndianType, temp, size);
		m_buf.Write(temp, size);
		delete[] temp;
		#endif
		return size;
// [endmodify] junyash
	}
	virtual bool Eos() const { BsAssert(0); return true; }					// TODO:
	virtual unsigned long Tell() const { BsAssert(0); return 0; }			// TODO:
	virtual unsigned long Length() const { return m_buf.Size(); }			
	virtual bool Valid() const { return !m_buf.IsEmpty(); }


	//
	void Clear() { m_buf.Clear(); }
private:
	StreamBuf m_buf;
};

class BCountStream : public BStream 
{
public:
	explicit BCountStream(size_t capacity = CHUNK_SIZE) 		
		: m_size(0)
	{}

	// BStream
	virtual int Seek( long offset, int origin ){ return 0;	}		
	virtual unsigned long Read( void* ptr, unsigned long size, int nEndianType=0, bool bAllowPartial=false ) {	// mruete: added bAllowPartial parameter     		
		return 0;
	}	
	virtual int Write( const void* ptr, unsigned long size, int nEndianType=0 ) {return m_size+=size;}
	virtual bool Eos() const { BsAssert(0); return true; }					// TODO:
	virtual unsigned long Tell() const { BsAssert(0); return 0; }			// TODO:
	virtual unsigned long Length() const { return m_size; }			
	virtual bool Valid() const { return m_size!=0; }


	//
	void Clear() { m_size = 0; }
private:
	int m_size;
};

/*
 *		BBufferStream
 *
 *	A stream class which owns memory block and dynamically expands it on demand.
 *  The associated memory block should NOT be freed directly from Buf() pointer.
 *	BBufferStream class can be used as random-access buffers for other sequential
 *  streams.
 */

/*class BBufferStream : public BMemoryStream
{
public:
	// BBufferStream owns memory and dynamically expands it on demand.
	// It should NOT be freed directly from Buf() pointer.
	class mem_overflow {};

	BBufferStream( unsigned long capacity = 256, unsigned int step = 128 );
	BBufferStream( BStream* src, unsigned long size = 0, unsigned int step = 128 );
	virtual ~BBufferStream();

	virtual int Write( const void* ptr, unsigned long size );
	virtual int Move( BStream& srcStream, unsigned long size );

	void Clear();

protected:
	void resize( unsigned int size );
	unsigned long m_capacity;
	unsigned short m_step;
};*/

/*
 *		BSubStream
 *
 *	A stream class which maps to some parts of another stream. Read-only. 
 */

/*class BSubStream : public BStream
{
public:

	BSubStream( BStream* src, bool bOwnSrc = false );
	BSubStream( BStream* src, long length, bool bOwnSrc = false );
	virtual ~BSubStream();

	virtual int Seek( long offset, int origin );
	virtual int Read ( void* ptr, unsigned long size );
	virtual int Write ( const void* ptr, unsigned long size )	{ return 0; }	// Cannot write to substreams!!
	virtual int Move( BStream& srcStream, unsigned long size )	{ return 0; }   // Cannot write to substreams!!
	virtual bool Eos () const									{ return m_cur == m_length; }
	virtual unsigned long Tell () const							{ return m_cur; }
	virtual unsigned long Length () const						{ return m_length; }
	virtual bool Valid() const;

protected:
	BStream* m_src;
	long m_startPos;
	long m_cur, m_length;
	unsigned char m_flag;
};*/

#endif // #ifndef __BLUE_STREAM_EXT__