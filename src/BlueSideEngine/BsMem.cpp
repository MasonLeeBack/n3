#include "stdafx.h"

#if defined(USE_MEMOVERRIDES) && defined(_XBOX)

#include "BsBlockAllocator.h"
#include "BsMemTracker.h"

#include "MGSToolbox\DebugValue.h"

namespace BsMem {

#ifdef USE_MGSTOOLBOX

MGSToolbox::DebugValue g_memoryShowStats("Memory", "Mem-Stats Display", false );
MGSToolbox::DebugValue g_memoryWriteToFile( "Memory", "Mem-Stats Write", false );

MGSToolbox::DebugValue g_blockShowStats("Memory", "Block-Stats Display", false );
MGSToolbox::DebugValue g_blockWriteToFile( "Memory", "Block-Stats Print", false );

void Update()
{
	if ( g_memoryWriteToFile.asBool() )
	{
		g_memoryWriteToFile.fromBool( false );

		static int s_writeCount = 0;

		char fileName[256];
		sprintf( fileName, "d:\\memory_%d.meminfo", s_writeCount );
		++s_writeCount;

		BsMemTracker::WriteInfo( fileName );
	}

	if ( g_blockWriteToFile.asBool() )
	{
		g_blockWriteToFile.fromBool( false );

		BsBlockAllocator::DumpStats();
	}
}

void Display(int x, int y)
{
	if ( g_memoryShowStats.asBool() )
		BsMemTracker::Display(x, y);

	if ( g_blockShowStats.asBool() )
		BsBlockAllocator::Display(x + 500, y);
}

#else

void Update()
{
}

void Display(int x, int y)
{
}

#endif

void WriteInfo(char const * filename)
{
	BsMemTracker::WriteInfo(filename);
}

} // namespace

//------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------

#else

namespace BsMem
{
	void Update() {};
	void Display(int x, int y) {};
	void WriteInfo(char const * filename) {};
}

#endif
