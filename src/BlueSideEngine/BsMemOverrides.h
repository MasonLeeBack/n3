#pragma once

#include "BsMemSystem.h"

#if defined(USE_MEMOVERRIDES) && defined(_XBOX)

// Defines the local memory allocations

void * BsMalloc( unsigned int system, unsigned int size );
void * BsRealloc( void * memory, unsigned int size );
void   BsFree( void * memory );

void * BsAlignedMalloc( unsigned int system, unsigned int size, unsigned int alignment );
void * BsAlignedRealloc( void * memory, unsigned int size, unsigned int alignment );
void   BsAlignedFree( void * memory );

void * BsPhysicalAlloc( unsigned int system, unsigned int requestedSize, ULONG_PTR address, ULONG_PTR alignment, unsigned int protectFlags );
void   BsPhysicalFree( void * memory );

// Override the memory allocations to go through our version

#undef GlobalFree

#define VirtualAlloc(address, size, type, protect) RequiresImplementation
#define VirtualAllocEx(process, address, size, type, protect) RequiresImplementation
#define VirtualFree(address, size, type) RequiresImplementation
#define VirtualFreeEx(process, address, size, type) RequiresImplementation

#define GlobalAlloc(flags, bytes) RequiresImplementation
#define GlobalFree(handle) RequiresImplementation

#define XPhysicalAlloc(size, address, alignment, protect) BsPhysicalAlloc( eMemorySystem_Physical, (size), (address), (alignment), (protect) )
#define XPhysicalAllocEx(size, lowestAddress, highestAddress, alignment, protect) RequiresImplementation
#define XPhysicalFree(address) BsPhysicalFree( (address) )
#define XPhysicalSize(address) BsPhysicalSize( (address) ) RequiresImplementation


#define malloc(size)          BsMalloc( eMemorySystem_Malloc, (size) )
#define realloc(memory, size) BsRealloc( (memory), (size) )
#define free(memory)          BsFree( (memory) )

#define _aligned_malloc(size, alignment)          BsAlignedMalloc( eMemorySystem_Malloc, (size), (alignment) )
#define _aligned_realloc(memory, size, alignment) BsAlignedRealloc( (memory), (size), (alignment) )
#define _aligned_free(memory)                     BsAlignedFree( (memory) )

#define DEBUG_NEW new

#else

// Override our versions to go through the global memory allocations

#define BsMalloc(system, memory) malloc( (size) )
#define BsRealloc(memory, size)  realloc( (memory), (size) )
#define BsFree(memory)           free( (memory) )

#define BsPhysicalAlloc(size, address, alignment, protect) XPhysicalAlloc( (size), (address), (alignment), (protect) )
#define BsPhysicalSize(address) XPhysicalSize( (address) )
#define BsPhysicalFree(address) XPhysicalFree( (address) )

#define BsAlignedMalloc(eMemorySystem_Global, size, alignment) _aligned_malloc( (size), (alignment) )
#define BsAlignedRealloc(memory, size, alignment)              _aligned_realloc( (memory), (size), (alignment) )
#define BsAlignedFree(memory)                                  _aligned_free( (memory) )

#define DEBUG_NEW new

#endif

