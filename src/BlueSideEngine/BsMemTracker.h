#pragma once

//#define TRACK_ALLOCATIONS 
//#define TRACK_ALLOCATIONS_VERBOSE
//#define TRACK_ALLOCATIONS_CALLSTACK

//------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------

#if defined(USE_MEMOVERRIDES) && defined(_XBOX) && defined(TRACK_ALLOCATIONS)

namespace BsMemTracker{

	void Init();

	int CalculateSizeWithTag( unsigned int requrestedSize );

	void Add( unsigned int system, unsigned int bytesAllocated );
	void Add( void * memory, unsigned int system, unsigned int bytesRequested, unsigned int bytesAllocated );
	void AddWithTag( void * memory, unsigned int system, unsigned int bytesRequested, unsigned int bytesAllocated );
	void Remove( unsigned int system, unsigned int bytesAllocated );
	void Remove( void * memory, unsigned int bytesAllocated );

	unsigned int MemSize( void * memory, unsigned int bytesAllocated );

	void WriteInfo( char const * filename );
	void Display( int x, int y );
};

#else

namespace BsMemTracker {

	inline void Init() {}

	inline int CalculateSizeWithTag( unsigned int requestedSize ) { return requestedSize; }


	inline void Add( unsigned int system, unsigned int bytesAllocated ) {}
	inline void Add( void * memory, unsigned int system, unsigned int bytesRequested, unsigned int bytesAllocated ) {}
	inline void AddWithTag( void * memory, unsigned int system, unsigned int bytesRequested, unsigned int bytesAllocated ) {}
	inline void Remove( unsigned int system, unsigned int bytesAllocated ) {}
	inline void Remove( void * memory, unsigned int bytesAllocated ) {}

	inline unsigned int MemSize( void * memory, unsigned int bytesAllocated ) { return bytesAllocated; }

	inline void WriteInfo( char const * filename ) {}
	inline void Display( int x, int y ) {}
};

#endif
