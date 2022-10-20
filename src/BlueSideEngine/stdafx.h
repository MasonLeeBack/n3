// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#pragma	warning(disable:4819)
#define NOMINMAX

#pragma inline_depth( 255 )


#ifdef _XBOX
	#include <xtl.h>
	#include <fxl.h>
	#include <xgraphics.h>
#else
	#ifdef _DEBUG
		#define D3D_DEBUG_INFO
	#endif
	#include <d3d9.h>
	#include <d3dx9.h>
#endif

// TODO: reference additional headers your program requires here

#include <stdio.h>
#include <assert.h>

#include <vector>
#include <deque>
#include <list>
#include <set>
#include <map>
#include <hash_map>
#include <algorithm>
#include <string>

//aleksger - safe strings
#include "TnTstring.h"

#include "BsCommon.h"
#include "BsMath.h"
#include "BsUtil.h"

#include "NxPhysics.h"
#include "NxCooking.h"
#include "NxStream.h"

#include <stdlib.h>
#include <comdef.h>

#include "BsMemOverrides.h"

#ifdef _USAGE_TOOL_
	#ifdef _MSC_VER
	#ifdef _DEBUG //memory leak test
	#include <crtdbg.h>
	#define malloc(a) _malloc_dbg(a,_NORMAL_BLOCK,__FILE__,__LINE__)
	inline void*  operator new(size_t size, LPCSTR strFileName, INT iLine)
	{return _malloc_dbg(size, _NORMAL_BLOCK, strFileName, iLine);}
	inline void operator delete(void *pVoid, LPCSTR strFileName, INT iLine)
	{_free_dbg(pVoid, _NORMAL_BLOCK);}
	#define new new(__FILE__, __LINE__)
	#define CHECK_MEMORY_LEAK _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF|_CRTDBG_ALLOC_MEM_DF);
	#else
	#define CHECK_MEMORY_LEAK
	#endif //_DEBUG
	#else
	#define CHECK_MEMORY_LEAK
	#endif 
#endif


