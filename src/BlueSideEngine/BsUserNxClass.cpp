#include "stdafx.h"
#include "BsKernel.h"
#include "BsPhysicsMgr.h"
#include "BsPhysicsTrigger.h"
#include "NxStream.h"
#include "BsUserNxClass.h"

static class OutputStream : public NxUserOutputStream
{

public:
	void  reportError (NxErrorCode code, const char *message, const char *file, int line)
	{
		if  (code < NXE_DB_INFO) {
			char szErrorMsg[255];
			sprintf(szErrorMsg, "Novodex : %s\n", message);
			DebugString(szErrorMsg);
			BsAssert( 0 );			
		}
	}

	NxAssertResponse  reportAssertViolation (const char *message, const char *file, int line)
	{
		char szErrorMsg[255];
		sprintf(szErrorMsg, "Novodex : %s\n", message);
		DebugString(szErrorMsg);
		BsAssert(0);
		return NX_AR_CONTINUE;
	}

	void  print (const char *message)
	{
		char szErrorMsg[255];
		sprintf(szErrorMsg, "Novodex : %s\n", message);
		DebugString(szErrorMsg);
		BsAssert(0);
	}
} s_OutputStream;

static class UserTriggerReport : public NxUserTriggerReport
{
public:
	virtual void onTrigger( NxShape& triggerShape, NxShape& otherShape, NxTriggerFlag status )
	{
		if( triggerShape.appData == NULL || otherShape.appData == NULL ) return;
		CBsPhysicsTriggerBase* pTriggerObj = (CBsPhysicsTriggerBase *) triggerShape.userData;
		if( pTriggerObj ) {
			pTriggerObj->OnTrigger();
		}		
	}
} s_TriggerReport;

static struct DebugRenderer : public NxUserDebugRenderer
{
public:
		
	void DebugRenderer::renderData(const NxDebugRenderable& data) const
	{
		g_BsKernel.GetPhysicsMgr()->SetRenderData( data );
	}
} s_DebugRenderer;

NxUserOutputStream* CBsUserNxClass::GetUserOutputStream()
{
    return &s_OutputStream;
}

NxUserDebugRenderer& CBsUserNxClass::GetUserDebugRenderer()
{
    return s_DebugRenderer;
}


NxUserTriggerReport* CBsUserNxClass::GetUserTriggerReport()
{
    return &s_TriggerReport;
}

UserStream::UserStream(const char* filename, bool load) : fp(NULL)
{
	fp = fopen(filename, load ? "rb" : "wb");
}

UserStream::~UserStream()
{
	if(fp)	fclose(fp);
}

// Loading API
NxU8 UserStream::readByte() const
{
	NxU8 b;
	size_t r = fread(&b, sizeof(NxU8), 1, fp);
	NX_ASSERT(r);
	return b;
}

NxU16 UserStream::readWord() const
{
	NxU16 w;
	size_t r = fread(&w, sizeof(NxU16), 1, fp);
	NX_ASSERT(r);
	return w;
}

NxU32 UserStream::readDword() const
{
	NxU32 d;
	size_t r = fread(&d, sizeof(NxU32), 1, fp);
	NX_ASSERT(r);
	return d;
}

float UserStream::readFloat() const
{
	NxReal f;
	size_t r = fread(&f, sizeof(NxReal), 1, fp);
	NX_ASSERT(r);
	return f;
}

double UserStream::readDouble() const
{
	NxF64 f;
	size_t r = fread(&f, sizeof(NxF64), 1, fp);
	NX_ASSERT(r);
	return f;
}

void UserStream::readBuffer(void* buffer, NxU32 size)	const
{
	size_t w = fread(buffer, size, 1, fp);
	NX_ASSERT(w);
}

// Saving API
NxStream& UserStream::storeByte(NxU8 b)
{
	size_t w = fwrite(&b, sizeof(NxU8), 1, fp);
	NX_ASSERT(w);
	return *this;
}

NxStream& UserStream::storeWord(NxU16 w)
{
	size_t ww = fwrite(&w, sizeof(NxU16), 1, fp);
	NX_ASSERT(ww);
	return *this;
}

NxStream& UserStream::storeDword(NxU32 d)
{
	size_t w = fwrite(&d, sizeof(NxU32), 1, fp);
	NX_ASSERT(w);
	return *this;
}

NxStream& UserStream::storeFloat(NxReal f)
{
	size_t w = fwrite(&f, sizeof(NxReal), 1, fp);
	NX_ASSERT(w);
	return *this;
}

NxStream& UserStream::storeDouble(NxF64 f)
{
	size_t w = fwrite(&f, sizeof(NxF64), 1, fp);
	NX_ASSERT(w);
	return *this;
}

NxStream& UserStream::storeBuffer(const void* buffer, NxU32 size)
{
	size_t w = fwrite(buffer, size, 1, fp);
	NX_ASSERT(w);
	return *this;
}

MemoryWriteBuffer::MemoryWriteBuffer() : currentSize(0), maxSize(0), data(NULL)
{

}

MemoryWriteBuffer::~MemoryWriteBuffer()
{
	if(data){
		delete [] data;
	}	
}

NxStream& MemoryWriteBuffer::storeByte(NxU8 b)
{
	storeBuffer(&b, sizeof(NxU8));
	return *this;
}

NxStream& MemoryWriteBuffer::storeWord(NxU16 w)
{
	storeBuffer(&w, sizeof(NxU16));
	return *this;
}

NxStream& MemoryWriteBuffer::storeDword(NxU32 d)
{
	storeBuffer(&d, sizeof(NxU32));
	return *this;
}

NxStream& MemoryWriteBuffer::storeFloat(NxReal f)
{
	storeBuffer(&f, sizeof(NxReal));
	return *this;
}

NxStream& MemoryWriteBuffer::storeDouble(NxF64 f)
{
	storeBuffer(&f, sizeof(NxF64));
	return *this;
}

NxStream& MemoryWriteBuffer::storeBuffer(const void* buffer, NxU32 size)
{
	NxU32 expectedSize = currentSize + size;
	if(expectedSize > maxSize)
	{
		maxSize = (int)(expectedSize * 1.5);

		unsigned char* newData = new unsigned char[ maxSize ];
		if(data)
		{
			memcpy(newData, data, currentSize);
			delete [] data;
		}
		data = newData;
	}
	memcpy(data+currentSize, buffer, size);
	currentSize += size;
	return *this;
}

MemoryReadBuffer::MemoryReadBuffer(const NxU8* data) : buffer(data)
{

}

MemoryReadBuffer::~MemoryReadBuffer()
{
	// We don't own the data => no delete
}

NxU8 MemoryReadBuffer::readByte() const
{
	NxU8 b;
	memcpy(&b, buffer, sizeof(NxU8));
	buffer += sizeof(NxU8);
	return b;
}

NxU16 MemoryReadBuffer::readWord() const
{
	NxU16 w;
	memcpy(&w, buffer, sizeof(NxU16));
	buffer += sizeof(NxU16);
	return w;
}

NxU32 MemoryReadBuffer::readDword() const
{
	NxU32 d;
	memcpy(&d, buffer, sizeof(NxU32));
	buffer += sizeof(NxU32);
	return d;
}

float MemoryReadBuffer::readFloat() const
{
	float f;
	memcpy(&f, buffer, sizeof(float));
	buffer += sizeof(float);
	return f;
}

double MemoryReadBuffer::readDouble() const
{
	double f;
	memcpy(&f, buffer, sizeof(double));
	buffer += sizeof(double);
	return f;
}

void MemoryReadBuffer::readBuffer(void* dest, NxU32 size) const
{
	memcpy(dest, buffer, size);
	buffer += size;
}
