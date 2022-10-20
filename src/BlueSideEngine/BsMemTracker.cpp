#include "stdafx.h"

#if defined(USE_MEMOVERRIDES) && defined(_XBOX)

#pragma warning(disable:4200)

#include <xbdm.h>

#include "BsMemTracker.h"
#include "BsMemSystem.h"
#include "BsDmAllocator.h"

#include "MGSToolBox\SimpleFont.h"
#include "MGSToolBox\GPUStateSave.h"
#include "MGSToolBox\DebugValue.h"
#include "MGSToolBox\Utils.h"

#if defined(TRACK_ALLOCATIONS)

// Generally you don't want asserts in the memory system, use this when attempting to verify the system

#define BsMemTrackerAssert assert

namespace BsMemTracker
{

//------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------

// Utility class to take a critical section when the class enters the scope and to leave the critical section when
// the class leaves the scope.

class AutoCriticalSection {

public:
	AutoCriticalSection( CRITICAL_SECTION& cs )
		: m_cs( cs )
	{
		EnterCriticalSection( &m_cs );
	}

	~AutoCriticalSection()
	{
		LeaveCriticalSection( &m_cs );
	}

private:
	CRITICAL_SECTION& m_cs;

};

//------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------

// Constants

enum {
	eVersion = 2,             // Version number of the memory tracker, change this when the internals change
	eMaxCallstackSize = 8,    // Maximum depth of the callstack to record
	eSkipCalls = 1,           // Number of functions to skip when recording callstacks
	eMaxSystems = 128,        // Maximum number of systems to track
	eMemTag = 'MTAG',
};

// Local structure definitions

struct MemoryArea {
	char         name[16];
	unsigned int numAllocations;
	unsigned int numBytesRequested;
	unsigned int numBytesAllocated;
};

struct MemoryBlock {
	MemoryBlock * next;
	MemoryBlock * prev;
	unsigned int  system;
	unsigned int  bytesRequested;
	unsigned int  bytesAllocated;
	unsigned int  callstackDepth;
	void *        callstack[];
};

struct MemTag {
	MemoryBlock * info;
	unsigned int  tag;
};

typedef std::map< unsigned int, MemoryBlock *, less<unsigned int>, dm_allocator< pair<const unsigned int, void *> > > MemoryMap;

//------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------

// Local variables

#if defined(TRACK_ALLOCATIONS_VERBOSE)
static MemoryMap *      s_map;
#endif

static MemoryArea       s_total;
static MemoryArea       s_systems[ eMaxSystems ];
static MemoryBlock *    s_blocks;
static CRITICAL_SECTION s_cs;

//------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------

// Private functions

static void Register( MemorySystem system, char const * name );
static void AddInternal( MemoryBlock * memInfo );
static void RemoveInternal( MemoryBlock * memInfo );

static MemoryBlock * AllocateMemInfo( unsigned int system, unsigned int bytesRequested, unsigned int bytesAllocated );
static void          ReleaseMemInfo( MemoryBlock * memInfo );

//------------------------------------------------------------------------------------------------------------------------
// Implementation:
//------------------------------------------------------------------------------------------------------------------------

void Register( MemorySystem system, const char * name )
{
	memset( &s_systems[system].name, 0, sizeof(s_systems[system].name) );
	strcpy_s( s_systems[system].name, _countof(s_systems[system].name), name );
}

//------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------

MemoryBlock * AllocateMemInfo( unsigned int system, unsigned int bytesRequested, unsigned int bytesAllocated )
{
	MemoryBlock * memInfo = 0;

#if defined(TRACK_ALLOCATIONS_VERBOSE)
	
	void * callstack[ eMaxCallstackSize+eSkipCalls ];
	int numCalls = 0;

#if defined(TRACK_ALLOCATIONS_CALLSTACK)

	HRESULT hr = DmCaptureStackBackTrace( eMaxCallstackSize+eSkipCalls, reinterpret_cast<void**>(&callstack) );
	BsMemTrackerAssert( SUCCEEDED(hr) );

	if ( SUCCEEDED(hr) )
	{
		while ( numCalls < eMaxCallstackSize )
		{
			if ( callstack[numCalls+eSkipCalls] == 0 )
				break;

			++numCalls;
		}
	}

#endif

	unsigned int callstackSize = numCalls * sizeof(void*);
	unsigned int memInfoSize = sizeof(MemoryBlock) + callstackSize;

	memInfo = (MemoryBlock*)DmAllocatePool( memInfoSize );
	if ( memInfo )
	{
		Add( eMemorySystem_Debug, memInfoSize );

		memInfo->next           = 0;
		memInfo->prev           = 0;
		memInfo->system         = system;
		memInfo->bytesRequested = bytesRequested;
		memInfo->bytesAllocated = bytesAllocated;
		memInfo->callstackDepth = numCalls;

		if ( callstackSize )
			memcpy( memInfo->callstack, callstack+eSkipCalls, callstackSize );

		AddInternal( memInfo );
	}

#endif

	return memInfo;
}

//------------------------------------------------------------------------------------------------------------------------

void ReleaseMemInfo( MemoryBlock * memInfo )
{
	if ( memInfo == 0 )
		return;

	RemoveInternal( memInfo );

	unsigned int callstackSize = memInfo->callstackDepth * sizeof(void*);
	unsigned int memInfoSize = sizeof(MemoryBlock) + callstackSize;

	Remove( eMemorySystem_Debug, memInfoSize );
	DmFreePool( memInfo );
}

//------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------

void Init()
{
	ZeroMemory( &s_total, sizeof(s_total) );
	ZeroMemory( &s_systems, sizeof(s_systems) );

	InitializeCriticalSection( &s_cs );

	memset( &s_total, 0, sizeof(s_total) );
	memset( &s_systems, 0, sizeof(s_systems) );

	strcpy_s( s_total.name, _countof(s_total.name), "total" );

	Register( eMemorySystem_Debug,          "debug" );
	Register( eMemorySystem_BlockAllocator, "block_alloc" );
	Register( eMemorySystem_Global,         "global" );
	Register( eMemorySystem_Graphics,       "graphics" );
	Register( eMemorySystem_Online,         "online" );
	Register( eMemorySystem_XMV,            "xmv" );
	Register( eMemorySystem_Audio,          "audio" );
	Register( eMemorySystem_XUI,            "xui" );
	Register( eMemorySystem_XboxKernel,     "kernel" );
	Register( eMemorySystem_XAPI,           "xapi" );
	Register( eMemorySystem_XBDM,           "xbdm" );
	Register( eMemorySystem_BSX,            "bsx" );
	Register( eMemorySystem_AtgResource,    "atg resource" );
	Register( eMemorySystem_Malloc,         "malloc" );
	Register( eMemorySystem_AlignedMalloc,  "aligned_malloc" );
	Register( eMemorySystem_Physical,       "physical" );
	Register( eMemorySystem_Unknown,        "unknown" );

	// Manually allocate the memory map and use placement-new.  This has to be done because otherwise the
	// new function that we have overriden will be called without the memory map being initialized.

	void * memory = DmAllocatePool( sizeof(MemoryMap) );

#if defined(TRACK_ALLOCATIONS_VERBOSE)
	s_map = reinterpret_cast<MemoryMap*>(memory);
	::new( memory ) MemoryMap();
#endif
}

//------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------

void AddInternal( MemoryBlock * memInfo )
{
	BsMemTrackerAssert( memInfo );
	BsMemTrackerAssert( memInfo->system < eMaxSystems );

	s_total.numAllocations += 1;
	s_total.numBytesRequested += memInfo->bytesRequested;
	s_total.numBytesAllocated += memInfo->bytesAllocated;

	s_systems[ memInfo->system ].numAllocations += 1;
	s_systems[ memInfo->system ].numBytesRequested += memInfo->bytesRequested;
	s_systems[ memInfo->system ].numBytesAllocated += memInfo->bytesAllocated;

	if ( s_blocks )
		s_blocks->prev = memInfo;

	memInfo->prev = 0;
	memInfo->next = s_blocks;

	s_blocks = memInfo;
}

//------------------------------------------------------------------------------------------------------------------------

inline bool IsAlignedTo( unsigned int size, unsigned int alignment )
{
	return ((size & (alignment - 1)) == 0);
}

inline int AlignTo( unsigned int size, unsigned int alignment )
{
	return (size + alignment - 1) & ~(alignment - 1);
}

//------------------------------------------------------------------------------------------------------------------------

int CalculateSizeWithTag( unsigned int requestedSize )
{
#if defined(TRACK_ALLOCATIONS_VERBOSE)

	// Calculate the total size required.

	unsigned int requiredSize = requestedSize + sizeof(MemTag);

	// Adjust the size to maintain the original alignment.

	if ( IsAlignedTo( requestedSize, 16 ) )
		requiredSize = AlignTo( requiredSize, 16 );
	else if ( IsAlignedTo( requestedSize, 8 ) )
		requiredSize = AlignTo( requiredSize, 8 );
	else if ( IsAlignedTo( requestedSize, 4 ) )
		requiredSize = AlignTo( requiredSize, 4 );

	return requiredSize;

#else

	return requestedSize;

#endif
}

//------------------------------------------------------------------------------------------------------------------------

MemTag * RetrieveTag( void * memory, unsigned int bytesAllocated )
{
#if defined(TRACK_ALLOCATIONS_VERBOSE)

	return reinterpret_cast<MemTag*>(reinterpret_cast<unsigned int>(memory) + bytesAllocated - sizeof(MemTag));

#else

	return NULL;

#endif
}

//------------------------------------------------------------------------------------------------------------------------

MemoryBlock * RetrieveMemInfo( void * memory, unsigned int bytesAllocated )
{

#if defined(TRACK_ALLOCATIONS_VERBOSE)

	MemoryMap::iterator find = s_map->find( reinterpret_cast<unsigned int>(memory) );
	if ( find != s_map->end() )
	{
		return find->second;
	}

	MemTag * memTag = RetrieveTag( memory, bytesAllocated );

	// If this assert fires there was either memory corruption, or this memory has already been freed.
	BsAssert( memTag->tag == eMemTag );

	return memTag->info;

#else

	return NULL;

#endif
}

//------------------------------------------------------------------------------------------------------------------------

void Add( unsigned int system, unsigned int bytesAllocated )
{
	AutoCriticalSection acs( s_cs );

	BsMemTrackerAssert( system < eMaxSystems );

	s_total.numAllocations += 1;
	s_total.numBytesRequested += bytesAllocated;
	s_total.numBytesAllocated += bytesAllocated;

	s_systems[ system ].numAllocations += 1;
	s_systems[ system ].numBytesRequested += bytesAllocated;
	s_systems[ system ].numBytesAllocated += bytesAllocated;
}

//------------------------------------------------------------------------------------------------------------------------

void Add( void * memory, unsigned int system, unsigned int bytesRequested, unsigned int bytesAllocated )
{
	AutoCriticalSection acs( s_cs );

	BsMemTrackerAssert( s_map );
	BsMemTrackerAssert( system < eMaxSystems );

#if defined(TRACK_ALLOCATIONS_VERBOSE)

	// Allocate and initialize the memory info block

	MemoryBlock * memInfo = AllocateMemInfo( system, bytesRequested, bytesAllocated );
	if ( memInfo )
	{
		s_map->insert( MemoryMap::value_type( reinterpret_cast<unsigned int>(memory), memInfo ) );
	}
	else
	{
		// Track the memory even if we failed to allocte a block of memory to keep track of other information.
		Add( eMemorySystem_Global, bytesAllocated ); 
	}

#else // TRACK_ALLOCATIONS_VERBOSE

	Add( eMemorySystem_Global, bytesAllocated ); 

#endif // TRACK_ALLOCATIONS_VERBOSE
}

//------------------------------------------------------------------------------------------------------------------------

void AddWithTag( void * memory, unsigned int system, unsigned int bytesRequested, unsigned int bytesAllocated )
{
	AutoCriticalSection acs( s_cs );

	BsMemTrackerAssert( system < eMaxSystems );

#if defined(TRACK_ALLOCATIONS_VERBOSE)

	// Allocate and initialize the memory info block

	MemoryBlock * memInfo = AllocateMemInfo( system, bytesRequested, bytesAllocated );
	if ( memInfo )
	{
		MemTag * memTag = RetrieveTag( memory, bytesAllocated );
		memTag->info = memInfo;
		memTag->tag  = eMemTag;
	}
	else
	{
		// Track the memory even if we failed to allocte a block of memory to keep track of other information.
		Add( eMemorySystem_Global, bytesAllocated ); 
	}

#else // TRACK_ALLOCATIONS_VERBOSE

	Add( eMemorySystem_Global, bytesAllocated ); 

#endif // TRACK_ALLOCATIONS_VERBOSE
}

//------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------

void RemoveInternal( MemoryBlock * memInfo )
{
	BsMemTrackerAssert( memInfo );
	BsMemTrackerAssert( memInfo->system < eMaxSystems );

	BsMemTrackerAssert( s_total.numAllocations > 0 );
	BsMemTrackerAssert( s_total.numBytesAllocated >= memInfo->bytesAllocated );
	BsMemTrackerAssert( s_systems[ memInfo->system ].numBytesAllocated >= memInfo->bytesAllocated );

	s_total.numAllocations -= 1;
	s_total.numBytesRequested -= memInfo->bytesRequested;
	s_total.numBytesAllocated -= memInfo->bytesAllocated;

	s_systems[ memInfo->system ].numAllocations -= 1;
	s_systems[ memInfo->system ].numBytesRequested -= memInfo->bytesRequested;
	s_systems[ memInfo->system ].numBytesAllocated -= memInfo->bytesAllocated;

	if ( memInfo->prev )
		memInfo->prev->next = memInfo->next;

	if ( memInfo->next )
		memInfo->next->prev = memInfo->prev;

	if ( s_blocks == memInfo )
		s_blocks = memInfo->next;
}

//------------------------------------------------------------------------------------------------------------------------

void Remove( unsigned int system, unsigned int bytesAllocated )
{
	AutoCriticalSection acs( s_cs );

	BsMemTrackerAssert( system < eMaxSystems );

	BsMemTrackerAssert( s_total.numAllocations > 0 );
	BsMemTrackerAssert( s_total.numBytesAllocated >= bytesAllocated );
	BsMemTrackerAssert( s_systems[ system ].numBytesAllocated >= bytesAllocated );

	s_total.numAllocations -= 1;
	s_total.numBytesRequested -= bytesAllocated;
	s_total.numBytesAllocated -= bytesAllocated;

	s_systems[ system ].numAllocations -= 1;
	s_systems[ system ].numBytesRequested -= bytesAllocated;
	s_systems[ system ].numBytesAllocated -= bytesAllocated;
}

//------------------------------------------------------------------------------------------------------------------------

void Remove( void * memory, unsigned int bytesAllocated )
{
	if ( memory == 0 )
		return;

	AutoCriticalSection acs( s_cs );

#if defined(TRACK_ALLOCATIONS_VERBOSE)

	BsMemTrackerAssert( s_map );

	MemoryBlock * memInfo = 0;

	MemoryMap::iterator find = s_map->find( reinterpret_cast<unsigned int>(memory) );
	if ( find != s_map->end() )
	{
		// This is an externally tracked memory block, extract the tracking information
		// and remove it from the external tracking structure.

		memInfo = find->second;
		s_map->erase( find );
	}
	else
	{
		// It's not externally tracked, that means it should have a memory tag at the end of the
		// allocation.

		MemTag * memTag = RetrieveTag( memory, bytesAllocated );

		// If this assert fires there was either memory corruption, or this memory has already been freed.
		BsMemTrackerAssert( memTag->tag == eMemTag ); 

		if ( memTag->tag == eMemTag )
		{
			memInfo = memTag->info;
			memTag->tag = 0; // Mark this memory as no longer being tracked.
		}
	}

	if ( memInfo )
	{
		ReleaseMemInfo( memInfo );
	}
	else
	{
		Remove( eMemorySystem_Global, bytesAllocated );
	}

#else

	Remove( eMemorySystem_Global, bytesAllocated ); 

#endif // TRACK_ALLOCATIONS_VERBOSE
}

//------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------

unsigned int MemSize( void * memory, unsigned int bytesAllocated )
{
	if ( memory == 0 )
		return 0;

#if defined(TRACK_ALLOCATIONS_VERBOSE)

	AutoCriticalSection acs( s_cs );

	MemoryBlock * memInfo = RetrieveMemInfo( memory, bytesAllocated );
	return memInfo->bytesRequested; 

#else

	return bytesAllocated;

#endif

}

//------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------

char const * SystemName( char const * systemName, unsigned int systemId )
{
	static char qualifiedName[16];

	if ( systemName == 0 || systemName[0] == 0 )
	{
		sprintf_s( qualifiedName, _countof(qualifiedName), "unknown (%d)", systemId );
	}
	else
	{
		sprintf_s( qualifiedName, _countof(qualifiedName), "%s", systemName );
	}

	return qualifiedName;
}

bool WriteWrapper( HANDLE file, void const * buffer, unsigned long bufferLength, bool & result )
{
	if ( !result )
		return result;

	unsigned long bytesWritten = 0;

	BOOL writeResult = WriteFile( file, buffer, bufferLength, &bytesWritten, NULL );
	if ( (writeResult == 0) || (bytesWritten != bufferLength) )
		result = false;

	return result;
}

bool WriteBuffered( HANDLE file, void const * buffer, unsigned long bufferLength, unsigned char * tempBuffer, unsigned int tempBufferSize, unsigned int & tempBufferOffset, bool & result )
{
	if ( !result )
		return result;

	while ( bufferLength )
	{

		if ( tempBufferOffset + bufferLength < tempBufferSize )
		{
			memcpy( tempBuffer + tempBufferOffset, buffer, bufferLength );
			tempBufferOffset += bufferLength;

			return result;
		}

		unsigned int remainingLength = tempBufferSize - tempBufferOffset;

		memcpy( tempBuffer + tempBufferOffset, buffer, remainingLength );
		WriteWrapper( file, tempBuffer, tempBufferSize, result );

		buffer        = reinterpret_cast<unsigned char const *>(buffer) + remainingLength;
		bufferLength -= remainingLength;

		tempBufferOffset = 0;
	}

	return result;
}

bool WriteBufferedFlush( HANDLE file, unsigned char const * tempBuffer, unsigned int tempBufferSize, unsigned int & tempBufferOffset, bool & result )
{
	if ( !result )
		return result;

	if ( tempBufferOffset )
	{
		WriteWrapper( file, tempBuffer, tempBufferOffset, result );
		tempBufferOffset = 0;
	}

	return result;
}

//------------------------------------------------------------------------------------------------------------------------

void WriteInfo( const char * filename )
{
	AutoCriticalSection acs( s_cs );

	HANDLE file = CreateFile( filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0 );
	if ( file == INVALID_HANDLE_VALUE )
		return;

	const unsigned int writeBufferLength = 128 * 1024;
	unsigned char * writeBuffer = ::new unsigned char [ writeBufferLength ];
	unsigned int writeBufferOffset = 0;
	bool writeSuccess = true;

	// Write the version number

	unsigned int version = eVersion;
	WriteBuffered( file, &version, sizeof(version), writeBuffer, writeBufferLength, writeBufferOffset, writeSuccess );

	// Calculate how many systems actually have allocations (note that we always write out at least
	//   the total allocations)

	unsigned int numSystems = 1;
	for ( unsigned int index = 0; index < eMaxSystems; ++index )
	{
		if ( s_systems[index].numAllocations )
			++numSystems;
	}

	// Write out the system information

	unsigned int numSystemsWritten = 0;

	WriteBuffered( file, &numSystems, sizeof(numSystems), writeBuffer, writeBufferLength, writeBufferOffset, writeSuccess );

	unsigned int systemId = static_cast<unsigned int>(-1);
	WriteBuffered( file, &systemId, sizeof(systemId), writeBuffer, writeBufferLength, writeBufferOffset, writeSuccess );
	WriteBuffered( file, &s_total.name, sizeof(s_total.name), writeBuffer, writeBufferLength, writeBufferOffset, writeSuccess );
	WriteBuffered( file, &s_total.numAllocations, sizeof(s_total.numAllocations), writeBuffer, writeBufferLength, writeBufferOffset, writeSuccess );
	WriteBuffered( file, &s_total.numBytesAllocated, sizeof(s_total.numBytesAllocated), writeBuffer, writeBufferLength, writeBufferOffset, writeSuccess );
	WriteBuffered( file, &s_total.numBytesRequested, sizeof(s_total.numBytesRequested), writeBuffer, writeBufferLength, writeBufferOffset, writeSuccess );

	++numSystemsWritten;

	for ( unsigned int index = 0; index < eMaxSystems; ++index )
	{
		if ( s_systems[index].numAllocations )
		{
			WriteBuffered( file, &index, sizeof(index), writeBuffer, writeBufferLength, writeBufferOffset, writeSuccess );
			WriteBuffered( file, &s_systems[index].name, sizeof(s_systems[index].name), writeBuffer, writeBufferLength, writeBufferOffset, writeSuccess );
			WriteBuffered( file, &s_systems[index].numAllocations, sizeof(s_systems[index].numAllocations), writeBuffer, writeBufferLength, writeBufferOffset, writeSuccess );
			WriteBuffered( file, &s_systems[index].numBytesAllocated, sizeof(s_systems[index].numBytesAllocated), writeBuffer, writeBufferLength, writeBufferOffset, writeSuccess );
			WriteBuffered( file, &s_systems[index].numBytesRequested, sizeof(s_systems[index].numBytesRequested), writeBuffer, writeBufferLength, writeBufferOffset, writeSuccess );

			++numSystemsWritten;
		}
	}

	BsMemTrackerAssert( numSystemsWritten == numSystems );

	// Write out the allocations

	MemoryBlock * current = s_blocks;
	while ( current )
	{
		WriteBuffered( file, &current->system, sizeof(current->system), writeBuffer, writeBufferLength, writeBufferOffset,  writeSuccess );
		WriteBuffered( file, &current->bytesRequested, sizeof(current->bytesRequested), writeBuffer, writeBufferLength, writeBufferOffset, writeSuccess );
		WriteBuffered( file, &current->bytesAllocated, sizeof(current->bytesAllocated), writeBuffer, writeBufferLength, writeBufferOffset, writeSuccess );
		WriteBuffered( file, &current->callstackDepth, sizeof(current->callstackDepth), writeBuffer, writeBufferLength, writeBufferOffset, writeSuccess );

		for ( unsigned int index = 0; index < current->callstackDepth; ++index )
		{
			unsigned int address = reinterpret_cast<unsigned int>( current->callstack[index] );
			WriteBuffered( file, &address, sizeof(address), writeBuffer, writeBufferLength, writeBufferOffset,  writeSuccess );
		}

		current = current->next;
	}

	WriteBufferedFlush( file, writeBuffer, writeBufferLength, writeBufferOffset, writeSuccess );

	CloseHandle( file );

	::delete [] writeBuffer;
}

//------------------------------------------------------------------------------------------------------------------------

void Display( int x, int y )
{
#if defined(USE_MGSTOOLBOX)

	AutoCriticalSection acs( s_cs );

	unsigned int numSystems =  0;
	for ( int i = 0; i < eMaxSystems; ++i )
	{
		if ( s_systems[i].numAllocations )
			++numSystems;
	}

	int lineHeight = MGSToolbox::SimpleFont::getHeight();

	MGSToolbox::RenderState_2D_PC state;
	MGSToolbox::DrawRect( x, y, x + 450, y + lineHeight * (10 + numSystems), 0x80000000 );

	x = x + lineHeight;
	y = y + lineHeight;

	MGSToolbox::SimpleFont::Printf( x, y, 0xffffffff,
		"Total Memory Usage:\n"
		"---------------------\n"
		"    Num Allocations: %d\n"
		"    Bytes Requested: %7.2f KB\t%4.2f MB\n"
		"    Bytes Allocated: %7.2f KB\t%4.2f MB\n",

		s_total.numAllocations,
		s_total.numBytesRequested / 1024.f,
		s_total.numBytesRequested / (1024.f * 1024.f),
		s_total.numBytesAllocated / 1024.f,
		s_total.numBytesAllocated / (1024.f * 1024.f)
	);

	y = y + lineHeight * 6;

	MGSToolbox::SimpleFont::Printf( x, y, 0xffffffff,
		"System Memory Usage:\n"
		"--------------------\n"
	);

	y = y + lineHeight * 2;

	for ( int i = 0; i < eMaxSystems; ++i )
	{
		if ( s_systems[i].numAllocations == 0 )
			continue;

		MGSToolbox::SimpleFont::Printf( x, y, 0xffffffff,
			"    %-16s: %-9d %9.2f KB %7.2f MB\n",
			SystemName( s_systems[i].name, i ),
			s_systems[i].numAllocations,
			s_systems[i].numBytesAllocated / (1024.f),
			s_systems[i].numBytesAllocated / (1024.f * 1024.f)
		);

		y = y + lineHeight;
	}

#endif
}

} // namespace BsMemTracker

#endif // defined(TRACK_ALLOCATIONS)

//------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------

#endif // defined(MEM_OVERRIDES) && defined(_XBOX)
