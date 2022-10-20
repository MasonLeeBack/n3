#pragma once

#define USE_SMALL_BLOCK_ALLOCATOR
//#define USE_SMALL_BLOCK_STATS
//#define USE_SMALL_BLOCK_VALIDATION

#if defined(USE_SMALL_BLOCK_ALLOCATOR) && defined(USE_MEMOVERRIDES) && defined(_XBOX)

namespace BsBlockAllocator {

	typedef unsigned int   U32;
	typedef unsigned short U16;
	typedef unsigned char  U8;

	void   Init();
	bool   IsReservedPointer(void* pmem);
	U32    BlockSizeFromPointer(void* pmem);
	void*  AllocateBlock(U32 blockSize);
	void   FreeBlock(void* pmem);

	void   DumpStats();
	void   Display(int x, int y);
};

#else

namespace BsBlockAllocator {
	inline void Init() {};
	inline void DumpStats() {};
	inline void Display(int x, int y) {};
};

#endif
