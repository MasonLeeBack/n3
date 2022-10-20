#include <stdafx.h>

#if defined(USE_MEMOVERRIDES) && defined(_XBOX)

#include "BsMemOverrides.h"
#include "BsMemTracker.h"
#include "BsMemSystem.h"
#include "BsBlockAllocator.h"

#undef malloc
#undef realloc
#undef free

#undef _aligned_malloc
#undef _aligned_realloc
#undef _aligned_free

#undef XPhysicalAlloc
#undef XPhysicalFree
#undef XPhysicalSize

//------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------

#if defined(USE_SMALL_BLOCK_ALLOCATOR)

size_t InternalMemsize(void * memory)
{
	if ( BsBlockAllocator::IsReservedPointer( memory ) )
		return BsBlockAllocator::BlockSizeFromPointer( memory );
	else
		return _msize( memory );

	return _msize( memory );
}

void * InternalMalloc(size_t size)
{
	void * result = BsBlockAllocator::AllocateBlock( size );
	if ( result )
		return result;

	return ::malloc( size );
}

void InternalFree(void * memory)
{
	if ( BsBlockAllocator::IsReservedPointer(memory) )
		BsBlockAllocator::FreeBlock(memory);
	else
		free( memory );
}

void * InternalRealloc(void * oldMemory, size_t newSize)
{
	// realloc with a size of 0 behaves just like free()

	if ( newSize == 0 )
	{
		InternalFree( oldMemory );
		return NULL;
	}

	// realloc without a memory pointer behaves just like malloc()

	if ( oldMemory == 0 )
	{
		return InternalMalloc( newSize );
	}

	// realloc with both a memory pointer and a size

	if ( BsBlockAllocator::IsReservedPointer( oldMemory ) )
	{
		size_t oldSize = BsBlockAllocator::BlockSizeFromPointer( oldMemory );
		if ( oldSize == newSize )
			return oldMemory;

		void * newMemory = InternalMalloc( newSize );
		if ( newMemory == NULL )
			return NULL;

		 XMemCpy( newMemory, oldMemory, min(oldSize, newSize) );

		 BsBlockAllocator::FreeBlock( oldMemory );

		return newMemory;
	}

	return ::realloc( oldMemory, newSize );
}

#else

size_t InternalMemsize(void * memory)
{
	return _msize(memory);
}

void * InternalMalloc(size_t size)
{
	return malloc(size);
}

void InternalFree(void * memory)
{
	free( memory );
}

void * InternalRealloc(void * oldMemory, size_t newSize)
{
	return realloc( oldMemory, newSize );
}

#endif // USE_SMALL_BLOCK_ALLOCATOR

//------------------------------------------------------------------------------------------------------------------------
// BsAlloc/BsRealloc/BsFree
//------------------------------------------------------------------------------------------------------------------------

void * BsMalloc( unsigned int system, unsigned int requestedSize )
{
	unsigned int requiredSize = BsMemTracker::CalculateSizeWithTag( requestedSize );
	void * memory = InternalMalloc( requiredSize );

	if ( memory != 0 )
	{
		BsMemTracker::AddWithTag( memory, static_cast<MemorySystem>(system), requestedSize, InternalMemsize( memory ) );
	}
	else
	{
		DebugString( "Malloc: Failed to allocate %4.2f MB\n", static_cast<float>(requestedSize) / (1024.f * 1024.f) );
		_DEBUGBREAK;
	}

	return memory;
}

void * BsRealloc( void * memory, unsigned int requestedSize )
{
	MemorySystem system = eMemorySystem_Global;

	if ( memory )
	{
		// TODO PT: implement retrieval of system ID for an existing allocation

		//system = BsMemTracker::System( memory );
		BsMemTracker::Remove( memory, InternalMemsize( memory ));
	}

	unsigned int requiredSize = requestedSize ? BsMemTracker::CalculateSizeWithTag( requestedSize ) : 0;
	void * result = InternalRealloc( memory, requiredSize );

	if ( result )
		BsMemTracker::AddWithTag( result, system, requestedSize, InternalMemsize( result ) );

	return result;
}

void BsFree( void * memory )
{
	if ( memory == 0 )
		return;

	BsMemTracker::Remove( memory, InternalMemsize( memory ) );
	InternalFree( memory );
}

//------------------------------------------------------------------------------------------------------------------------
// BsAlignedAlloc/BsAlignedRealloc/BsAlignedFree
//------------------------------------------------------------------------------------------------------------------------

void * BsAlignedMalloc( unsigned int system, unsigned int requestedSize, unsigned int alignment )
{
	unsigned int requiredSize = BsMemTracker::CalculateSizeWithTag( requestedSize );

	void * memory = _aligned_malloc( requiredSize, alignment );
	if ( memory != 0 )
	{
		BsMemTracker::AddWithTag( memory, static_cast<MemorySystem>(system), requestedSize, _msize( memory ) );
	}
	else
	{
		DebugString( "AlignedMalloc: Failed to allocate %4.2f MB\n", static_cast<float>(requestedSize) / (1024.f * 1024.f) );
		_DEBUGBREAK;
	}

	return memory;
}

void * BsAlignedRealloc( void * memory, unsigned int requestedSize, unsigned int alignment )
{
    MemorySystem system = eMemorySystem_AlignedMalloc;

    if ( memory )
    {
        // TODO PT: implement retrieval of system ID for an existing allocation

        //system = BsMemTracker::System( memory );
        BsMemTracker::Remove( memory, _msize( memory ));
	}

	unsigned int requiredSize = requestedSize ? BsMemTracker::CalculateSizeWithTag( requestedSize ) : 0;
    void * result = _aligned_realloc( memory, requiredSize, alignment );

    if ( result )
        BsMemTracker::AddWithTag( result, system, requestedSize, _msize( result ) );

    return result;
}

void BsAlignedFree( void * memory )
{
	if ( memory == 0 )
		return;

	BsMemTracker::Remove( memory, _msize( memory ) );
	_aligned_free( memory );
}

//------------------------------------------------------------------------------------------------------------------------
// BsPhysicalAlloc/BsPhysicalFree
//------------------------------------------------------------------------------------------------------------------------

void * BsPhysicalAlloc( unsigned int system, unsigned int requestedSize, ULONG_PTR address, ULONG_PTR alignment, unsigned int protectFlags )
{
	void * memory = XPhysicalAlloc( requestedSize, address, alignment, protectFlags );

	if ( memory != 0 )
	{
		unsigned int allocatedSize = XPhysicalSize( memory );

		unsigned int requiredSize = BsMemTracker::CalculateSizeWithTag( requestedSize );
		if ( allocatedSize >= requiredSize )
			BsMemTracker::AddWithTag( memory, static_cast<MemorySystem>(system), requestedSize, XPhysicalSize( memory ) );
		else
			BsMemTracker::Add( memory, static_cast<MemorySystem>(system), requestedSize, XPhysicalSize( memory ) );
	}
	else
	{
		DebugString( "XPhysicalAlloc: Failed to allocate %4.2f MB\n", static_cast<float>(requestedSize) / (1024.f * 1024.f) );
		_DEBUGBREAK;
	}

	return memory;
}

void BsPhysicalFree( void * memory )
{
	if ( memory == 0 )
		return;

	unsigned int allocatedSize = XPhysicalSize( memory );
	BsMemTracker::Remove( memory, allocatedSize );

	XPhysicalFree( memory );
}

//------------------------------------------------------------------------------------------------------------------------
// new/delete
//------------------------------------------------------------------------------------------------------------------------


// Overrides for new/delete to go through the memory tracking interfaces

// [beginmodify] 2006/2/2 junyash PS#4609 detect new[] and delete mis-match

//#define USE_MISDELETE_DETECTOR

#ifndef USE_MISDELETE_DETECTOR
void * __cdecl operator new( size_t requestedSize )
{
	return BsMalloc( eMemorySystem_Global, requestedSize );
}

void * __cdecl operator new( size_t requestedSize, MemorySystem system )
{
	return BsMalloc( system, requestedSize );
}

void __cdecl operator delete( void * memory )
{
	BsFree( memory );
}

#else // ifndef USE_MISDELETE_DETECTOR

#define WARN_NULL_DELETE		// not problem in C++ regulation, but warn it.
#define MEM_MARK_OF_SCALAR		0x99
#define MEM_MARK_OF_ARRAY		0xAA
#define MEM_MARK_PADDING_S		8
#define MEM_MARK_PADDING_A		12
#define MEM_MARK_SIZE			4
#define VC_ARRAY_INFO_SIZE		4	// please see PS#4609 description

// Overrides for new/delete to go through the memory tracking interfaces

void * __cdecl operator new( size_t requestedSize )
{
	void * pAlloc = (void *)((DWORD)BsMalloc( eMemorySystem_Global, requestedSize + MEM_MARK_PADDING_S + MEM_MARK_SIZE + VC_ARRAY_INFO_SIZE ) + MEM_MARK_PADDING_S + MEM_MARK_SIZE + VC_ARRAY_INFO_SIZE);
	if ( pAlloc ) {
		DWORD * pMark = (DWORD *)pAlloc;
		pMark[-1] = pMark[-2] = MEM_MARK_OF_SCALAR;
	}
	//DebugString( "%s( %d ) = %p\n", __FUNCTION__, requestedSize, pAlloc );
	return pAlloc;
}

void * __cdecl operator new( size_t requestedSize, MemorySystem system )
{
	void * pAlloc = (void *)((DWORD)BsMalloc( system, requestedSize + MEM_MARK_PADDING_S + MEM_MARK_SIZE + VC_ARRAY_INFO_SIZE ) + MEM_MARK_PADDING_S + MEM_MARK_SIZE + VC_ARRAY_INFO_SIZE);
	if ( pAlloc ) {
		DWORD * pMark = (DWORD *)pAlloc;
		pMark[-1] = pMark[-2] = MEM_MARK_OF_SCALAR;
	}
	//DebugString( "%s( %d, system ) = %p\n", __FUNCTION__, requestedSize, pAlloc );
	return pAlloc;
}

void __cdecl operator delete( void * memory )
{
	//DebugString( "%s( %p )\n", __FUNCTION__, memory );
	if ( !memory ) {
		#ifdef WARN_NULL_DELETE
		DebugString( "WARNING! delete NULL\n" );
		#endif // ifdef WARN_NULL_DELETE
		return;
	}
	DWORD * pMark = (DWORD *)memory;
	if ( pMark[-1] != MEM_MARK_OF_SCALAR ) DebugString( "ERROR! delete 0x%p, pointer is allocated by new[] or momory broken.\n", memory );
	BsAssert( pMark[-1] == MEM_MARK_OF_SCALAR && "ERROR! delete pointer is allocated by new[] or momory broken." );
	BsFree( (void*)((DWORD)memory - MEM_MARK_PADDING_S - MEM_MARK_SIZE - VC_ARRAY_INFO_SIZE) );
}

void * __cdecl operator new[]( size_t requestedSize )
{
	void * pAlloc = (void *)((DWORD)BsMalloc( eMemorySystem_Global, requestedSize + MEM_MARK_PADDING_A + MEM_MARK_SIZE ) + MEM_MARK_PADDING_A + MEM_MARK_SIZE);
	if ( pAlloc ) {
		DWORD * pMark = (DWORD *)pAlloc;
		pMark[-1] = MEM_MARK_OF_ARRAY;
	}
	//DebugString( "%s( %d ) = %p\n", __FUNCTION__, requestedSize, pAlloc );
	return pAlloc;
}

void * __cdecl operator new[]( size_t requestedSize, MemorySystem system )
{
	void * pAlloc = (void *)((DWORD)BsMalloc( system, requestedSize + MEM_MARK_PADDING_A + MEM_MARK_SIZE ) + MEM_MARK_PADDING_A + MEM_MARK_SIZE);
	if ( pAlloc ) {
		DWORD * pMark = (DWORD *)pAlloc;
		pMark[-1] = MEM_MARK_OF_ARRAY;
	}
	//DebugString( "%s( %d, system ) = %p\n", __FUNCTION__, requestedSize, pAlloc );
	return pAlloc;
}

void __cdecl operator delete[]( void * memory )
{
	//DebugString( "%s( %p )\n", __FUNCTION__, memory );
	if ( !memory ) {
		#ifdef WARN_NULL_DELETE
		DebugString( "WARNING! delete NULL\n" );
		#endif // ifdef WARN_NULL_DELETE
		return;
	}
	DWORD * pMark = (DWORD *)memory;
	if ( pMark[-1] != MEM_MARK_OF_ARRAY ) DebugString( "ERROR! delete[] 0x%p, pointer is allocated by new or memory broken.\n", memory );
	BsAssert( pMark[-1] == MEM_MARK_OF_ARRAY && "ERROR! delete[] pointer is allocated by new or memory broken." );
	BsFree( (void*)((DWORD)memory - MEM_MARK_PADDING_A - MEM_MARK_SIZE) );
}
#endif // ifndef USE_MISDELETE_DETECTOR
// [endmodify] junyash

//------------------------------------------------------------------------------------------------------------------------
// XMemAlloc/XMemSize/XMemFree
//------------------------------------------------------------------------------------------------------------------------

MemorySystem XAllocatorIdToSystemId( DWORD allocatorId )
{
	MemorySystem system = eMemorySystem_Unknown;

	enum {
		eXALLOCAllocatorId_BSX = 42,
		eXALLOCAllocatorId_AtgResource = 127,
	};

	switch ( allocatorId )
	{
	case eXALLOCAllocatorId_D3D:
	case eXALLOCAllocatorId_D3DX:
	case eXALLOCAllocatorId_XGRAPHICS:
	case eXALLOCAllocatorId_SHADERCOMPILER:
		system = eMemorySystem_Graphics;
		break;
	case eXALLOCAllocatorId_XONLINE:
	case eXALLOCAllocatorId_XHV:
		system = eMemorySystem_Online;
		break;
	case eXALLOCAllocatorId_XMV:
		system = eMemorySystem_XMV;
		break;
	case eXALLOCAllocatorId_XAUDIO:
	case eXALLOCAllocatorId_XACT:
	case eXALLOCAllocatorId_XVOICE:
		system = eMemorySystem_Audio;
		break;
	case eXALLOCAllocatorId_XUI:
		system = eMemorySystem_XUI;
		break;
	case eXALLOCAllocatorId_XBOXKERNEL:
	case eXALLOCAllocatorId_USB:
	case eXALLOCAllocatorId_XASYNC:
		system = eMemorySystem_XboxKernel;
		break;
	case eXALLOCAllocatorId_XAPI:
		system = eMemorySystem_XAPI;
		break;
	case eXALLOCAllocatorId_XBDM:
		system = eMemorySystem_XBDM;
		break;
	case eXALLOCAllocatorId_BSX:
		system = eMemorySystem_BSX;
		break;
	case eXALLOCAllocatorId_AtgResource:
		system = eMemorySystem_AtgResource;
		break;
	default:
		printf( "unknwon allocator id: %d\n", allocatorId );
		assert(0 && "Unknown Allocator ID");
		break;
	}

	return system;
}

// Overrides for the XMemAlloc/XMemFree/XMemSize interfaces

LPVOID WINAPI XMemAlloc(SIZE_T requestedSize, DWORD allocAttributes)
{
	XALLOC_ATTRIBUTES * attributes = (XALLOC_ATTRIBUTES*)&allocAttributes;

	void * memory = XMemAllocDefault( requestedSize, allocAttributes );

	if ( memory )
	{
		MemorySystem system = XAllocatorIdToSystemId( attributes->dwAllocatorId );
		BsMemTracker::Add( memory, system, requestedSize, XMemSizeDefault( memory, allocAttributes ) );
	}
	else
	{
		DebugString( "XMemAlloc: Failed to allocate %4.2f MB\n", static_cast<float>(requestedSize) / (1024.f * 1024.f) );
		_DEBUGBREAK;
	}

	return memory;
}

VOID WINAPI XMemFree(PVOID memory, DWORD allocAttributes)
{
	if ( memory )
		BsMemTracker::Remove( memory, XMemSizeDefault( memory, allocAttributes ) ); 

	return XMemFreeDefault( memory, allocAttributes );
}

SIZE_T WINAPI XMemSize(PVOID memory, DWORD allocAttributes)
{
	if ( memory == 0 )
		return 0;

	unsigned int bytesAllocated = XMemSizeDefault( memory, allocAttributes );
	return BsMemTracker::MemSize( memory, bytesAllocated );
}

#endif // defined(_XBOX) && defined(USE_MEMOVERRIDES)
