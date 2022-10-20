// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#pragma inline_depth( 255 )

//#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define NOMINMAX 
// Windows Header Files:
#ifndef _XBOX
	#include <windows.h>
	#include <d3d9.h>
	#include <d3dx9.h>
	#define DIRECTINPUT_VERSION 0x0800
	#include <dinput.h>
#else
	#include <xtl.h>
	#include <fxl.h>
#endif // #ifndef _XBOX

// C RunTime Header Files
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <assert.h>

#pragma	warning(disable:4819)
#pragma	warning(disable:4786)	// identifier was truncated to '255' characters in the debug information
#pragma	warning(disable:4290)	// C++ exception specification ignored except to indicate a function is not __declspec(nothrow)
#include <vector>
#include <list>
#include <deque>
#include <string>
#include <map>
#include <algorithm>
#include <set>
#include <stack>
#include <queue>
//aleksger - safe strings
#include "TnTstring.h"

//#include <string.h>
#include <cmath>
#include <cstdio>

#include "NxPhysics.h"

#ifdef _DEBUG
#define ASSERT(exp) assert(exp)
#else
#define ASSERT(exp)
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#endif //SAFE_RELEASE

#include "BsUtil.h"
#include "BsMemOverrides.h"


//_TNT_
//Include file for XeCR
#ifdef	_TNT_
#ifdef	_XCR_
#include "XCR\\XeCR\\XeCR.h"
#pragma comment(lib, "XCR/XAM/xamps.lib")
#pragma comment(lib, "XCR/XeCR/Release/XeCR.lib")
#pragma comment(lib, "XCR/XenonUtility/Release/XenonUtility.lib")
#endif

#ifdef _TICKET_TRACKER_
#include "TicketTracker\\TicketTracker.h"
#pragma comment(lib, "xbdm.lib")
#pragma comment(lib, "TicketTracker/Release/TicketTracker.lib")
#endif

#ifndef _LTCG
#include <xbdm.h>
#endif
#endif //_TNT_
