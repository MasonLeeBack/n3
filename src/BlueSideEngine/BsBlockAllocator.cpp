#include <stdafx.h>

#if defined(_XBOX)

#include "BsBlockAllocator.h"
#include "BsMemTracker.h"

#include "MGSToolBox\SimpleFont.h"
#include "MGSToolBox\GPUStateSave.h"
#include "MGSToolBox\DebugValue.h"
#include "MGSToolBox\Utils.h"

#if defined(USE_SMALL_BLOCK_ALLOCATOR) && defined(USE_MEMOVERRIDES) && defined(_XBOX)

// Generally you don't want asserts in the memory system, use this when attempting to verify the system

#define memory_assert
#define memory_print     DebugString
#define memory_print_nl  "\n"

#undef VirtualAlloc
#undef VirtualFree

//
// The block allocator works by pre-allocating a given virtual memory range (512 MB by default).  It
// divides this virtual memory range into several areas (64 by default).  Each one of these areas is
// then associated with a specific allocation size.
//
// An area is then sub-divided into chunks which is the granularity at which physical memory is
// allocated.  The system keeps track of each chunk and links the free chunks together in a list.
//
// The chunks are then further subdivided into blocks which are the actual allocations of the
// specific size.
//
// +--------------- Area --------------+
// +-- Chunk --+-- Chunk --+-- Chunk --+
// +- B -+- B -+- B -+- B -+- B -+- F -+   B = Block, F = Chunk Footer
//
// The nice thing about doing things this way is that you can determine what area and chunk an allocation
// comes from very quickly by simply looking at the virtual address.
//


namespace BsBlockAllocator {

	CRITICAL_SECTION CriticalSection;                   // Critical section to make the allocation functions thread-safe
	bool             CriticalSectionInitialized;

	// Total of 512 MB of virtual space (out of 2GB)

	enum {
		MaxNumAreas        = 64,                        // The number of fixed sized allocation.
		SizePerArea        = 8 * 1024 * 1024,           // The maximum amount of memory allocated for a type of allocation.
		MaxTotalSize       = MaxNumAreas * SizePerArea, // Total virtual memory range to reserve.
		ChunkSize          = 64 * 1024,                 // 64K page size
		LargestBlockSize   = 256,                       // Constant for now, but could be dynamic in the future.

		NullChunk          = 0xFFFF,                    // Special chunk index indicating NULL chunk in the free list.
		AllocatedChunk     = 0xFFFE,                    // Special chunk index indicating that this chunk is currently allocated (and thus not in the free list).
	};

	U32 VirtualAddressStart = 0;
	U32 VirtualAddressEnd   = 0;

	U32 AreaCount = 0;

	U32  BlockSizes[MaxNumAreas] = { 0 };               // The size of the blocks for each of the areas.
	U32  ChunkSizes[MaxNumAreas] = { 0 };               // The chunk sizes for each of the areas.
	U32  ChunkTotals[MaxNumAreas] = { 0 };              // The total number of chunks in each area.
	U16* ChunkFreeList[MaxNumAreas] = { 0 };            // The chunk list for each area (each value is the pointer to the next chunk).
	U16  ChunkFreeListFirst[MaxNumAreas] = { 0 };       // The head of the list of free chunks (using the chunk list elements).

	U8   SizeToArea[LargestBlockSize / 4] = { 0 };

	// Chunk specific data structures

	struct BlockHeader {
		BlockHeader* nextBlock;
	};

	struct ChunkFooter {
		ChunkFooter* prevChunk;   // Previous chunk in list
		ChunkFooter* nextChunk;   // Next chunk in list
		BlockHeader* firstBlock;  // First block of list of blocks inside of chunk
		U32          numBlocks;   // Total number of blocks inside block list
	};

	ChunkFooter * ChunksWithAvailableBlocks[MaxNumAreas] = { 0 }; // The head of the list of chunks that contain free blocks

	// Stats to help track usage:

#if defined(USE_SMALL_BLOCK_STATS)
	U32 NumChunksAllocated[MaxNumAreas] = { 0 };
	U32 MaxChunksAllocated[MaxNumAreas] = { 0 };
	U32 NumBlocksAllocated[MaxNumAreas] = { 0 };
	U32 MaxBlocksAllocated[MaxNumAreas] = { 0 };
	U32 TotalChunks      = 0;
	U32 TotalBytes       = 0;
	U32 TotalOverhead    = 0;
	U32 MaxTotalChunks   = 0;
	U32 MaxTotalBytes    = 0;
	U32 MaxTotalOverhead = 0;
#endif

	// Helper class to automatically take and release a critical section:

	class AutoCriticalSection
	{

	public:
		AutoCriticalSection( CRITICAL_SECTION & criticalSection )
			: m_criticalSection(criticalSection)
		{
			EnterCriticalSection(&m_criticalSection);
		}

		~AutoCriticalSection()
		{
			LeaveCriticalSection(&m_criticalSection);
		}

	private:

		CRITICAL_SECTION & m_criticalSection;
	};

	bool IsReservedPointer( void* pmem )
	{
		return ULONG(pmem) >= VirtualAddressStart && ULONG(pmem) < VirtualAddressEnd;
	}

	U32 SizeToAreaIndex( U32 size )
	{
		memory_assert( size > 0 );
		memory_assert( size <= LargestBlockSize );

		return SizeToArea[((size + 3) / 4) - 1];
	}

	U32 PointerToAreaIndex( void* pmem )
	{
		memory_assert( IsReservedPointer(pmem) );
		return (ULONG(pmem) - VirtualAddressStart) / SizePerArea;
	}

	U32 BlockSizeFromPointer( void* pmem )
	{
		return BlockSizes[PointerToAreaIndex(pmem)];
	}

#if defined(USE_SMALL_BLOCK_VALIDATION)
	void Validate()
	{
		for (U32 area = 0; area < AreaCount; ++area)
		{
			U32 numFreeChunks = 0;

			U32 freeChunk = ChunkFreeListFirst[area];
			while ( freeChunk != NullChunk )
			{
				freeChunk = ChunkFreeList[area][freeChunk];
				++numFreeChunks;
			}

			U32 numAllocatedChunks = 0;

			for (U32 chunk = 0; chunk < ChunkTotals[area]; ++chunk)
			{
				if ( ChunkFreeList[area][chunk] == AllocatedChunk )
					++numAllocatedChunks;
			}

			memory_assert( numFreeChunks + numAllocatedChunks == ChunkTotals[area] );

		#if defined(USE_SMALL_BLOCK_STATS)
			memory_assert( numAllocatedChunks == NumChunksAllocated[area] );
		#endif
		}
	}
#else
	#define Validate()
#endif

	void AddArea( U32 blockSize, U32 chunkSize )
	{
		// Validate the arguments.

		memory_assert( (blockSize % 4) == 0 );									// The block size is aligned
		memory_assert( AreaCount < MaxNumAreas );								// Enough room to add an area
		memory_assert( blockSize <= LargestBlockSize );							// The block size is less than the largest (for the size to area mapping)
		memory_assert( AreaCount == 0 || BlockSizes[AreaCount-1] < blockSize );	// The block size is larger than the previous one

		BlockSizes[AreaCount] = blockSize;
		ChunkSizes[AreaCount] = chunkSize;

		U32  const numChunks  = ChunkTotals[AreaCount] = SizePerArea / chunkSize;
		U16* const freeList   = ChunkFreeList[AreaCount] = (U16*) ::HeapAlloc(::GetProcessHeap(), 0, numChunks * sizeof(U16));

		BsMemTracker::Add( eMemorySystem_BlockAllocator, numChunks * sizeof(U16) );

		for (U32 i = 0; i < numChunks - 1; ++i) {
			freeList[i] = i+1;
		}

		freeList[numChunks-1] = NullChunk;

		++AreaCount;
	}

	void InitSizeLookup()
	{
		U32 areaIndex = 0;

		for (U32 i = 0; i < LargestBlockSize / 4; ++i)
		{
			// Calculate the size for which to initialize the lookup for

			U32 size = (i+1) * 4;

			// Scan the areas for the appropriate one

			for (; areaIndex < MaxNumAreas; ++areaIndex)
			{
				if ( size <= BlockSizes[areaIndex] )
					break;
			}

			// Validate that there was an area for this particular size
					
			memory_assert( areaIndex < MaxNumAreas );

			// Validate that the alignment rules are maintained

			memory_assert( ((size %  4) != 0) || ((BlockSizes[areaIndex] %  4) == 0) );
			memory_assert( ((size %  8) != 0) || ((BlockSizes[areaIndex] %  8) == 0) );
			memory_assert( ((size % 16) != 0) || ((BlockSizes[areaIndex] % 16) == 0) );

			SizeToArea[i] = areaIndex;
		}

	}

	void Init()
	{
		if ( CriticalSectionInitialized )
			return;

		InitializeCriticalSection( &CriticalSection );
		CriticalSectionInitialized = true;

		AutoCriticalSection cs( CriticalSection );

		// Setup the memory areas for the fixed sized allocations

		AddArea(  4,  ChunkSize );
		AddArea(  8,  ChunkSize );
		AddArea(  12, ChunkSize );
		AddArea(  16, ChunkSize ); // 16
		AddArea(  20, ChunkSize );
		AddArea(  24, ChunkSize );
		AddArea(  28, ChunkSize );
		AddArea(  32, ChunkSize ); // 16
		AddArea(  36, ChunkSize );
		AddArea(  40, ChunkSize );
		AddArea(  44, ChunkSize );
		AddArea(  48, ChunkSize ); // 16
		AddArea(  52, ChunkSize );
		AddArea(  56, ChunkSize );
		AddArea(  60, ChunkSize );
		AddArea(  64, ChunkSize ); // 16
		AddArea(  68, ChunkSize );
		AddArea(  72, ChunkSize );
		AddArea(  76, ChunkSize );
		AddArea(  80, ChunkSize ); // 16
		AddArea(  84, ChunkSize );
		AddArea(  88, ChunkSize );
	    AddArea(  92, ChunkSize );
		AddArea(  96, ChunkSize ); // 16
	    AddArea( 100, ChunkSize );
	    AddArea( 104, ChunkSize );
	    AddArea( 108, ChunkSize );
		AddArea( 112, ChunkSize ); // 16
	    AddArea( 116, ChunkSize );
		AddArea( 120, ChunkSize );
		AddArea( 124, ChunkSize );
		AddArea( 128, ChunkSize ); // 16
	    AddArea( 132, ChunkSize );
		AddArea( 136, ChunkSize );
		AddArea( 140, ChunkSize );
		AddArea( 144, ChunkSize ); // 16
	    AddArea( 148, ChunkSize );
	    AddArea( 152, ChunkSize );
	    AddArea( 156, ChunkSize );
		AddArea( 160, ChunkSize ); // 16
		AddArea( 164, ChunkSize );
	    AddArea( 168, ChunkSize );
		AddArea( 172, ChunkSize );
		AddArea( 176, ChunkSize ); // 16
		AddArea( 180, ChunkSize );
		AddArea( 184, ChunkSize );
		AddArea( 188, ChunkSize );
		AddArea( 192, ChunkSize ); // 16
		AddArea( 196, ChunkSize );
		AddArea( 200, ChunkSize );
		AddArea( 204, ChunkSize );
		AddArea( 208, ChunkSize ); // 16
		AddArea( 212, ChunkSize );
		AddArea( 216, ChunkSize );
		AddArea( 220, ChunkSize );
		AddArea( 224, ChunkSize ); // 16
	    AddArea( 228, ChunkSize );
	    AddArea( 232, ChunkSize );
	    AddArea( 236, ChunkSize );
		AddArea( 240, ChunkSize ); // 16
		AddArea( 244, ChunkSize );
		AddArea( 248, ChunkSize );
	    AddArea( 252, ChunkSize );
		AddArea( 256, ChunkSize ); // 16

		InitSizeLookup();

		// Reserve the virtual space.	

		U32 TotalSize = AreaCount * SizePerArea;

		memory_assert( TotalSize <= MaxTotalSize );

		VirtualAddressStart = ULONG(::VirtualAlloc(0, TotalSize, MEM_RESERVE, PAGE_READWRITE));
		VirtualAddressEnd = VirtualAddressStart + TotalSize;
	}

	void* FindChunkFromPtr( void* chunkMemory )
	{
		// The pointer must be part of the block allocator.
		memory_assert(IsReservedPointer(chunkMemory));

		U32 const areaIndex = PointerToAreaIndex(chunkMemory);

		// The chunk must already have been initialized
		memory_assert(ChunkSizes[areaIndex] > 0);

		U32 const areaStart = VirtualAddressStart + SizePerArea * areaIndex;
		U16 const chunk = (U32(chunkMemory) - areaStart) / ChunkSizes[areaIndex];

		return (void*) (areaStart + ChunkSizes[areaIndex] * chunk);
	}

	void* AllocChunk( U32 areaIndex )
	{
		U16 const chunk = ChunkFreeListFirst[areaIndex];

		if ( chunk == NullChunk ) {
			return NULL; // Out of memory
		}

		memory_assert(ChunkSizes[areaIndex] * chunk < SizePerArea);

		// Unhook the chunk from the free chunks
	   
		ChunkFreeListFirst[areaIndex] = ChunkFreeList[areaIndex][chunk];
		ChunkFreeList[areaIndex][chunk] = AllocatedChunk;

		// Allocate the chunk memory

		U32 const areaStart = VirtualAddressStart + SizePerArea * areaIndex;

		void* chunkMemory = (void*) (areaStart + ChunkSizes[areaIndex] * chunk);

		DWORD allocationFlags = MEM_COMMIT | MEM_NOZERO;
		if ( ChunkSizes[areaIndex] >= 64 * 1024 )
			allocationFlags |= MEM_LARGE_PAGES;

		::VirtualAlloc( chunkMemory, ChunkSizes[areaIndex], allocationFlags, PAGE_READWRITE );

#if defined(USE_SMALL_BLOCK_STATS)
		TotalOverhead += ((sizeof(ChunkFooter) + BlockSizes[areaIndex] - 1) / BlockSizes[areaIndex]) * BlockSizes[areaIndex];
		if ( TotalOverhead > MaxTotalOverhead )
			MaxTotalOverhead = TotalOverhead;

		TotalChunks++;
		if ( TotalChunks > MaxTotalChunks )
			MaxTotalChunks = TotalChunks;

		NumChunksAllocated[areaIndex] += 1;
		if ( NumChunksAllocated[areaIndex] > MaxChunksAllocated[areaIndex] )
			MaxChunksAllocated[areaIndex] = NumChunksAllocated[areaIndex];
#endif

		return chunkMemory;
	}

	void FreeChunk( void* chunkMemory )
	{
		// The pointer must be part of the block allocator
		memory_assert(IsReservedPointer(chunkMemory));

		U32 const areaIndex = PointerToAreaIndex(chunkMemory);

		U32 const areaBegin = VirtualAddressStart + SizePerArea * areaIndex;
		U16 const chunk = (U16)((ULONG(chunkMemory) - areaBegin) / ChunkSizes[areaIndex]);

		// The chunk we are trying to free must already have been allocated
		memory_assert(ChunkFreeList[areaIndex][chunk] == AllocatedChunk);

		::VirtualFree(chunkMemory, ChunkSizes[areaIndex], MEM_DECOMMIT);

		// Add the chunk to the free list
	   
		ChunkFreeList[areaIndex][chunk] = ChunkFreeListFirst[areaIndex];
		ChunkFreeListFirst[areaIndex] = chunk;

#if defined(USE_SMALL_BLOCK_STATS)
		TotalOverhead -= ((sizeof(ChunkFooter) + BlockSizes[areaIndex] - 1) / BlockSizes[areaIndex]) * BlockSizes[areaIndex];

		--TotalChunks;
		NumChunksAllocated[areaIndex] -= 1;
#endif
	}


	/////////////////////////////////////////////////////////////////////

	void HookupChunk( U32 areaIndex, ChunkFooter * chunkFooter )
	{
		chunkFooter->nextChunk = ChunksWithAvailableBlocks[areaIndex];
		chunkFooter->prevChunk = 0;

		if ( chunkFooter->nextChunk )
		{
			assert( chunkFooter->nextChunk->prevChunk == 0 );
			chunkFooter->nextChunk->prevChunk = chunkFooter;
		}

		ChunksWithAvailableBlocks[areaIndex] = chunkFooter;
	}

	void UnhookChunk( U32 areaIndex, ChunkFooter * chunkFooter )
	{
		if ( chunkFooter->nextChunk )
		{
			memory_assert( chunkFooter->nextChunk->prevChunk == chunkFooter );
			chunkFooter->nextChunk->prevChunk = chunkFooter->prevChunk;
		}

		if ( chunkFooter->prevChunk )
		{
			memory_assert( chunkFooter->prevChunk->nextChunk == chunkFooter );
			chunkFooter->prevChunk->nextChunk = chunkFooter->nextChunk;
		}

		if ( ChunksWithAvailableBlocks[areaIndex] == chunkFooter ) {
			ChunksWithAvailableBlocks[areaIndex] = chunkFooter->nextChunk;
		}
	}

	void UnhookHeadChunk( U32 areaIndex, ChunkFooter * chunkFooter )
	{
		memory_assert( ChunksWithAvailableBlocks[areaIndex] == chunkFooter );

		ChunksWithAvailableBlocks[areaIndex] = chunkFooter->nextChunk;
		if ( chunkFooter->nextChunk )
		{
			memory_assert( chunkFooter->nextChunk->prevChunk == chunkFooter );
			chunkFooter->nextChunk->prevChunk = 0;
		}
	}

	/////////////////////////////////////////////////////////////////////

	ChunkFooter * InitializeChunk( void* chunkMemory, U32 chunkSize, U32 blockSize )
	{
		ChunkFooter * chunkFooter = (ChunkFooter*)((U8*)chunkMemory + chunkSize - sizeof(ChunkFooter));

		U32 const numBlocks = (chunkSize - sizeof(ChunkFooter)) / blockSize;

		// There should always be enough room in a chunk for at least one block
		memory_assert( numBlocks > 0 );

		chunkFooter->nextChunk  = 0;
		chunkFooter->prevChunk  = 0;
		chunkFooter->numBlocks  = numBlocks;
		chunkFooter->firstBlock = (BlockHeader*)(chunkMemory);

		// Initialize the available block list

		BlockHeader * prevBlock = (BlockHeader*)(chunkMemory);

		for (U32 blockIndex = 1; blockIndex < numBlocks; ++blockIndex)
		{
			BlockHeader * currentBlock = (BlockHeader*)((U8*)chunkMemory + blockSize * blockIndex);

			prevBlock->nextBlock = currentBlock;
			prevBlock = currentBlock;
		}

		prevBlock->nextBlock = 0;

		return chunkFooter;
	}

	/////////////////////////////////////////////////////////////////////

	ChunkFooter* GetChunkWithAvailableBlocks( U32 areaIndex )
	{
		ChunkFooter * chunkFooter = ChunksWithAvailableBlocks[areaIndex];
		if ( chunkFooter )
			return chunkFooter;

		// Commit a new chunk from our virtual memory range

		void* chunkMemory = AllocChunk( areaIndex );
		if ( !chunkMemory )
			return NULL;

		// Initialize the empty blocks in the new chunk

		chunkFooter = InitializeChunk( chunkMemory, ChunkSizes[areaIndex], BlockSizes[areaIndex] );

		// Hook the new chunks into the list of chunks with available blocks

		HookupChunk( areaIndex, chunkFooter );

		return chunkFooter;
	}

	void* AllocateBlockFromArea( U32 areaIndex )
	{
		AutoCriticalSection cs(CriticalSection);

		ChunkFooter* chunkFooter = GetChunkWithAvailableBlocks( areaIndex );
		if ( !chunkFooter )
			return NULL;

#if defined(USE_SMALL_BLOCK_STATS)
		TotalBytes += BlockSizes[areaIndex];
		if ( TotalBytes > MaxTotalBytes )
			MaxTotalBytes = TotalBytes;

		NumBlocksAllocated[areaIndex] += 1;
		if ( NumBlocksAllocated[areaIndex] > MaxBlocksAllocated[areaIndex] )
			MaxBlocksAllocated[areaIndex] = NumBlocksAllocated[areaIndex];
#endif

		memory_assert( chunkFooter == ChunksWithAvailableBlocks[areaIndex] );
		memory_assert( chunkFooter->numBlocks > 0 );
		memory_assert( chunkFooter->firstBlock != 0 );

		// Unhook the block memory from the chunk

		BlockHeader* blockHeader = chunkFooter->firstBlock;

		chunkFooter->firstBlock = blockHeader->nextBlock;
		chunkFooter->numBlocks -= 1;

		// If the chunk is now completely full, unhook the chunk from the list of chunks with available blocks

		if ( chunkFooter->numBlocks == 0 ) {
			UnhookHeadChunk( areaIndex, chunkFooter );
		}

		memory_assert( (chunkFooter->firstBlock != 0) || (chunkFooter->numBlocks == 0) );

		return blockHeader;
	}

	void* AllocateBlock( U32 blockSize )
	{
		Init();

		Validate();

		// Check to see if the allocation can be handled by the block allocator

		if ( blockSize == 0 || blockSize > LargestBlockSize )
			return NULL;

		// Calculate which small-block area this allocation should go to

		U32 areaIndex = SizeToAreaIndex(blockSize);
		void* memory = AllocateBlockFromArea( areaIndex );

		Validate();

		return memory;
	}

	/////////////////////////////////////////////////////////////////////

	void FreeBlock( void* blockMemory )
	{
		AutoCriticalSection cs(CriticalSection);

		Validate();

		U32 const areaIndex = PointerToAreaIndex(blockMemory);

#if defined(USE_SMALL_BLOCK_STATS)
		TotalBytes -= BlockSizes[areaIndex];
		NumBlocksAllocated[areaIndex] -= 1;
#endif

		// The chunk must already have been initialized
		memory_assert(ChunkSizes[areaIndex] > 0);

		U32 const areaStart = VirtualAddressStart + SizePerArea * areaIndex;
		U16 const chunk = (U32(blockMemory) - areaStart) / ChunkSizes[areaIndex];

		void* chunkMemory = (void*)(areaStart + ChunkSizes[areaIndex] * chunk);

		ChunkFooter* chunkFooter = (ChunkFooter*)((U8*)chunkMemory + ChunkSizes[areaIndex] - sizeof(ChunkFooter));

#if 0
		// Validation to see if the block is really allocated

		BlockHeader * currentBlockHeader = chunkFooter->firstBlock;
		while ( currentBlockHeader )
		{
			memory_assert( currentBlockHeader != reinterpret_cast<BlockHeader*>(blockMemory) );
			currentBlockHeader = currentBlockHeader->nextBlock;
		}
#endif

		// See if this is the last block in the chunk, if so we can free the memory

		U32 numTotalBlocks = (ChunkSizes[areaIndex] - sizeof(ChunkFooter)) / BlockSizes[areaIndex];
		if ( chunkFooter->numBlocks + 1 == numTotalBlocks )
		{
			UnhookChunk( areaIndex, chunkFooter );
			FreeChunk( chunkMemory );

			Validate();

			return;
		}

		// Hook the block into the free block list of the chunk

		BlockHeader* blockHeader = (BlockHeader*)(blockMemory);

		blockHeader->nextBlock  = chunkFooter->firstBlock;

		chunkFooter->firstBlock = blockHeader;
		chunkFooter->numBlocks += 1;

		// If the chunk just went from completely full to having a block available hook the chunk into the list of chunks with
		// available blocks.

		if ( chunkFooter->numBlocks == 1 ) {
			HookupChunk( areaIndex, chunkFooter );
		}

		memory_assert( chunkFooter->firstBlock != 0 );

		Validate();
	}

#if defined(USE_SMALL_BLOCK_STATS)

	void DumpStats()
	{
		AutoCriticalSection cs(CriticalSection);

		memory_print( "#: \tBlock Size\t# Chunks:\t# Blocks:\t# Bytes: \tChunk Size:" memory_print_nl );

		U32 totalWastage = 0;

		for (U32 index = 0; index < MaxNumAreas; ++index)
		{
			U32 wastage = MaxChunksAllocated[index] * ChunkSizes[index] - MaxBlocksAllocated[index] * BlockSizes[index];
			totalWastage += wastage;

			memory_print( "%-2d\t%-10d\t%-10d\t%-10d\t%-10d\t%-10d" memory_print_nl ,
				index,
				BlockSizes[index],
				MaxChunksAllocated[index],
				MaxBlocksAllocated[index],
				MaxBlocksAllocated[index] * BlockSizes[index],
				ChunkSizes[index] );

			MaxChunksAllocated[index] = NumChunksAllocated[index];
			MaxBlocksAllocated[index] = NumBlocksAllocated[index];
		}

		memory_print( memory_print_nl );

		memory_print( "Maximum Allocations:" memory_print_nl );

		memory_print( "Max Chunks  : %d (%d bytes)" memory_print_nl, MaxTotalChunks, MaxTotalChunks * ChunkSize );
		memory_print( "Max Bytes   : %d" memory_print_nl, MaxTotalBytes );
		memory_print( "Max Overhead: %d" memory_print_nl, MaxTotalOverhead );

		memory_print( "\nCurrent Allocations:" memory_print_nl );

		memory_print( "Chunks  : %d (%d bytes)" memory_print_nl, TotalChunks, TotalChunks * ChunkSize );
		memory_print( "Bytes   : %d" memory_print_nl, TotalBytes );
		memory_print( "Overhead: %d" memory_print_nl, TotalOverhead );

		memory_print( "\nTotal Wastage: %d" memory_print_nl, totalWastage );

		memory_print( memory_print_nl );
	}


	void Display( int x, int y )
	{
	#if defined(USE_MGSTOOLBOX)

		AutoCriticalSection cs(CriticalSection);

		U32 totalWastage = 0;

		for (U32 index = 0; index < MaxNumAreas; ++index)
		{
			U32 wastage = MaxChunksAllocated[index] * ChunkSizes[index] - MaxBlocksAllocated[index] * BlockSizes[index];
			totalWastage += wastage;

			MaxChunksAllocated[index] = NumChunksAllocated[index];
			MaxBlocksAllocated[index] = NumBlocksAllocated[index];
		}

		int lineHeight = MGSToolbox::SimpleFont::getHeight();

		MGSToolbox::RenderState_2D_PC state;
		MGSToolbox::DrawRect( x, y, x + 250, y + lineHeight * 16, 0x80000000 );

		x += lineHeight;
		y += lineHeight;

		MGSToolbox::SimpleFont::Printf( x, y, 0xffffffff,
			"Maximum Allocations:\n"
			"----------------------\n"
			"Max Chunks  : %d (%d bytes)\n" 
			"Max Bytes   : %d\n" 
			"Max Overhead: %d\n"
			"\nCurrent Allocations:\n"
			"----------------------\n"
			"Chunks  : %d (%d bytes)\n"
			"Bytes   : %d\n"
			"Overhead: %d\n"
			"\nTotal Wastage: %d\n",

			MaxTotalChunks, MaxTotalChunks * ChunkSize,
			MaxTotalBytes,
			MaxTotalOverhead,
			TotalChunks, TotalChunks * ChunkSize,
			TotalBytes,
			TotalOverhead,
			totalWastage
		);

	#endif
	}

#else

	void DumpStats()
	{
	}

	void Display(int x, int y)
	{
	}

#endif

} // namespace

#endif // defined(USE_SMALL_BLOCK_ALLOCATOR)

#endif // defined(_XBOX)