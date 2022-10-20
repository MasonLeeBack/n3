#pragma once

#if defined(USE_MEMOVERRIDES)

enum MemorySystem {
	eMemorySystem_Debug,
	eMemorySystem_BlockAllocator,
	eMemorySystem_Global,
	eMemorySystem_Graphics,
	eMemorySystem_Online,
	eMemorySystem_XMV,
	eMemorySystem_Audio,
	eMemorySystem_XUI,
	eMemorySystem_XboxKernel,
	eMemorySystem_XAPI,
	eMemorySystem_XBDM,
	eMemorySystem_BSX,
	eMemorySystem_AtgResource,
	eMemorySystem_Malloc,
	eMemorySystem_AlignedMalloc,
	eMemorySystem_Physical,
	eMemorySystem_Unknown,
};

#endif
