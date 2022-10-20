// Copyright 2005 Microsoft Corporation

#include "stdafx.h"
#include "Profiler.h"

#ifdef USE_MGSTOOLBOX

#include <xtl.h>
#include <assert.h>

#include "Utils.h"
#include "GPUStateSave.h"
#include "DebugValue.h"
#include "SimpleFont.h"

namespace MGSToolbox {

//-----------------------------------------------------------------------------

DebugValue g_profilerCpuEnable     ( "Profiler", "CPU Enable", false );

DebugValue g_profilerCpuEntryCount ( "Profiler", "CPU Entry Count", 3, 1, 100 );
DebugValue g_profilerCpuEntryStart ( "Profiler", "CPU Entry Start", 0, 0, 100 );

DebugValue g_profilerCpuPosX       ( "Profiler", "CPU Position X", 100 );
DebugValue g_profilerCpuPosY       ( "Profiler", "CPU Position Y", 100 );


DebugValue g_profilerGpuEnable     ( "Profiler", "GPU Enable", false );

DebugValue g_profilerGpuEntryCount ( "Profiler", "GPU Entry Count", 3, 1, 100 );
DebugValue g_profilerGpuEntryStart ( "Profiler", "GPU Entry Start", 0, 0, 100 );

DebugValue g_profilerGpuPosX       ( "Profiler", "GPU Position X", 100 );
DebugValue g_profilerGpuPosY       ( "Profiler", "GPU Position Y", 100 );


DebugValue g_profilerGraphWidth    ( "Profiler", "Graph Width", 400 );

DebugValue g_profilerFrameCount    ( "Profiler", "Frame Count", 32, 1, 32 );
DebugValue g_profilerBarHeight     ( "Profiler", "Bar Height", 2 );

DebugValue g_profilerEntrySpacing  ( "Profiler", "Entry Spacing", 3 );

DebugValue g_profilerExactGPU      ( "Profiler", "Exact GPU Timings", false );

DebugValue g_profilerTargetFPS     ( "Profiler", "Target FPS", 30, 30, 60, 30 );

//-----------------------------------------------------------------------------

double g_ticksToMsec;

//----------

Profile ProfileManager::m_cpuFrames[MAX_PROFILE_FRAMES];
Profile ProfileManager::m_gpuFrames[MAX_PROFILE_FRAMES];

int ProfileManager::m_frameCount;
int ProfileManager::m_frameCursor;
int ProfileManager::m_frameCursorValid;

int ProfileManager::m_cpuDepth;
int ProfileManager::m_gpuDepth;

bool ProfileManager::m_bInstalled;

bool ProfileRenderer::m_bInstalled;

//-----------------------------------------------------------------------------

void __cdecl writeTrueCallback ( DWORD context )
{
	bool * pValue = (bool*)context;

	*pValue = true;
}

void __cdecl writeTimeCallback ( DWORD context )
{
	LARGE_INTEGER * pTime = (LARGE_INTEGER*)context;

	QueryPerformanceCounter(pTime);
}

//----------

void GPUBeginTimeMark ( __int64 & time )
{
	DWORD d = (DWORD)&time;

	if(g_profilerExactGPU.asBool())
	{
		g_pDevice->InsertCallback(D3DCALLBACK_IDLE,writeTimeCallback,d);
	}
	else
	{
		g_pDevice->InsertCallback(D3DCALLBACK_IMMEDIATE,writeTimeCallback,d);
	}
}

void GPUEndTimeMark ( __int64 & time )
{
	DWORD d = (DWORD)&time;

	if(g_profilerExactGPU.asBool())
	{
		g_pDevice->InsertCallback(D3DCALLBACK_IDLE,writeTimeCallback,d);
	}
	else
	{
		g_pDevice->InsertCallback(D3DCALLBACK_IMMEDIATE,writeTimeCallback,d);
	}
}

void GPUValidMark ( bool & value )
{
	DWORD d = (DWORD)&value;

	g_pDevice->InsertCallback(D3DCALLBACK_IMMEDIATE,writeTrueCallback,d);
}






//-----------------------------------------------------------------------------

CPUProfiler::CPUProfiler ( char const * const name, unsigned int color ) 
{
	m_index = ProfileManager::CPUBegin(name,color);
}

CPUProfiler::~CPUProfiler()
{
	ProfileManager::CPUEnd(m_index);
}

//-----------------------------------------------------------------------------

GPUProfiler::GPUProfiler ( char const * const name, unsigned int color )
{
	m_index = ProfileManager::GPUBegin(name,color);
}

GPUProfiler::~GPUProfiler()
{
	ProfileManager::GPUEnd(m_index);
}









//-----------------------------------------------------------------------------

float ProfileEntry::getTimeMsec ( void ) const
{
	return float( double(m_timeEnd - m_timeBegin) * g_ticksToMsec );
}

//-----------------------------------------------------------------------------

void ProfileEntry::Clear ( void )
{
	m_color = 0;
	m_timeBegin = 0;
	m_timeEnd = 0;
}







//-----------------------------------------------------------------------------

Profile::Profile()
{
	for(int i = 0; i < MAX_PROFILE_ENTRIES; i++)
	{
		m_entries[i].m_index = i;
	}
	
	m_count = 0;
}

//----------

void Profile::Clear ( void )
{
	for(int i = 0; i < m_count; i++)
	{
		m_entries[i].Clear();
	}

	m_bValid = false;
	m_count = 0;
}

//----------

void Profile::Coalesce ( Profile const & source )
{
	Profile & dest = *this;

	dest.Clear();

	for(int i = 0; i < source.m_count; i++)
	{
		ProfileEntry const & sourceEntry = source.m_entries[i];

		int found = -1;

		for(int j = 0; j < dest.m_count; j++)
		{
			ProfileEntry & destEntry = dest.m_entries[j];

			if(!stricmp(sourceEntry.m_name,destEntry.m_name))
			{
				found = j;
				break;
			}
		}

		if(found != dest.m_count)
		{
			dest.m_entries[found].m_timeEnd += (sourceEntry.m_timeEnd - sourceEntry.m_timeBegin);
		}
		else
		{
			dest.m_entries[dest.m_count] = sourceEntry;

			dest.m_count++;
		}
	}
}

//----------

ProfileEntry & Profile::getNextEntry ( void )
{
	assert(m_count < MAX_PROFILE_ENTRIES);

	return m_entries[m_count++];
}






//-----------------------------------------------------------------------------

void ProfileManager::Install ( void )
{
	if(m_bInstalled) return;

	__int64 freq;

	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);

	g_ticksToMsec = 1000.0 / double(freq);

	//----------

	m_frameCursor = 0;
	m_frameCount = 0;

	m_cpuDepth = 0;
	m_gpuDepth = 0;

	for(int i = 0; i < MAX_PROFILE_FRAMES; i++)
	{
		m_cpuFrames[i].Clear();
		m_gpuFrames[i].Clear();
		
		m_cpuFrames[i].m_index = i;
		m_gpuFrames[i].m_index = i;
	}
}

void ProfileManager::Remove ( void )
{
}

//-----------------------------------------------------------------------------

void ProfileManager::BeginFrame ( void )
{
	m_frameCursor = (m_frameCursor + 1) % MAX_PROFILE_FRAMES;

	m_cpuFrames[m_frameCursor].Clear();
	m_gpuFrames[m_frameCursor].Clear();

	m_cpuDepth = 0;
	m_gpuDepth = 0;
}

//----------

void ProfileManager::EndFrame ( void )
{
	m_cpuFrames[m_frameCursor].m_bValid = true;

	GPUValidMark(m_gpuFrames[m_frameCursor].m_bValid);

	if(m_frameCount < MAX_PROFILE_FRAMES) m_frameCount++;
}

//-----------------------------------------------------------------------------

int ProfileManager::CPUBegin ( char const * const name, unsigned int color )
{
	m_cpuDepth++;

	ProfileEntry & entry = m_cpuFrames[m_frameCursor].getNextEntry();

	entry.m_name = name;
	entry.m_color = color;
	entry.m_depth = m_cpuDepth;
	entry.m_timeBegin = 0xDEADBEEFDEADBEEF;
	entry.m_timeEnd = 0xDEADBEEFDEADBEEF;

	QueryPerformanceCounter((LARGE_INTEGER*)(&entry.m_timeBegin));

	return entry.m_index;
}

//----------

void ProfileManager::CPUEnd ( int index )
{
	__int64 time;

	QueryPerformanceCounter((LARGE_INTEGER*)(&time));

	m_cpuFrames[m_frameCursor].m_entries[index].m_timeEnd = time;;

	m_cpuDepth--;
}

//-----------------------------------------------------------------------------

int ProfileManager::GPUBegin ( char const * const name, unsigned int color )
{
	m_gpuDepth++;

	ProfileEntry & entry = m_gpuFrames[m_frameCursor].getNextEntry();

	entry.m_name = name;
	entry.m_color = color;
	entry.m_depth = m_gpuDepth;
	entry.m_timeBegin = 0xDEADBEEFDEADBEEF;
	entry.m_timeEnd = 0xDEADBEEFDEADBEEF;

	GPUBeginTimeMark(entry.m_timeBegin);

	return entry.m_index;
}

//----------

void ProfileManager::GPUEnd ( int index )
{
	ProfileEntry & entry = m_gpuFrames[m_frameCursor].m_entries[index];

	GPUEndTimeMark(entry.m_timeEnd);

	m_gpuDepth--;
}

//-----------------------------------------------------------------------------

void ProfileManager::DumpStats ( void )
{
	__int64 freq;

	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);

	//----------

	Profile & cpuProfile = m_cpuFrames[m_frameCursor];
	Profile & gpuProfile = m_gpuFrames[m_frameCursor];

	if(!gpuProfile.m_bValid)
	{
		dprintf("Latest GPU profile isn't yet valid (GPU still working)\n");

		g_pDevice->BlockUntilIdle();
	}

	if(!gpuProfile.m_bValid)
	{
		dprintf("Error, still wasn't valid after block\n");
	}

	//----------
    
	dprintf("CPU ProfileFrame - \n");

	for(int i = 0; i < cpuProfile.m_count; i++)
	{
		ProfileEntry & entry = cpuProfile.m_entries[i];

		__int64 time = entry.m_timeEnd - entry.m_timeBegin;

		float msec = float(1000.0f * ((double)time / (double)freq));

		dprintf("%s - %f\n",entry.m_name,msec);
	}

	//----------

	dprintf("GPU ProfileFrame - \n");

	for(int i = 0; i < gpuProfile.m_count; i++)
	{
		ProfileEntry & entry = gpuProfile.m_entries[i];

		__int64 time = entry.m_timeEnd - entry.m_timeBegin;

		float msec = float(1000.0f * ((double)time / (double)freq));

		dprintf("%s - %f\n",entry.m_name,msec);
	}
}

//-----------------------------------------------------------------------------

Profile * ProfileManager::getCurrentCpuProfile ( void )
{
	return &m_cpuFrames[m_frameCursor];
}

//----------

Profile * ProfileManager::getCurrentGpuProfile ( void )
{
	return &m_gpuFrames[m_frameCursor];
}

//----------

Profile * ProfileManager::getLastValidCpuProfile ( void )
{
	int cursor = m_frameCursor;

	for(int i = 0; i < MAX_PROFILE_FRAMES; i++)
	{
		if(m_cpuFrames[cursor].m_bValid)
		{
			return &m_cpuFrames[cursor];
		}

		cursor = (cursor + MAX_PROFILE_FRAMES - 1) % MAX_PROFILE_FRAMES;
	}

	return NULL;
}

//----------

Profile * ProfileManager::getLastValidGpuProfile ( void )
{
	int cursor = m_frameCursor;

	for(int i = 0; i < MAX_PROFILE_FRAMES; i++)
	{
		if(m_gpuFrames[cursor].m_bValid)
		{
			return &m_gpuFrames[cursor];
		}

		cursor = (cursor + MAX_PROFILE_FRAMES - 1) % MAX_PROFILE_FRAMES;
	}

	return NULL;
}








//-------------------------------------------------------------------------------------

void ProfileRenderer::DrawBars ( Profile const & profile, int x, int y, int entryStart, int entryCount, int entryStride, int barHeight )
{
	SimpleRect * cursor = BeginRects(entryCount);

	memset(cursor,0,sizeof(SimpleRect)*entryCount);

	float targetFrameLength = 1000.0f / g_profilerTargetFPS.asFloat();

	float msecToPixels = float(g_profilerGraphWidth.asInt()) / targetFrameLength;
	
	for(int i = 0; i < entryCount; i++)
	{
		int entryIndex = i + entryStart;

		if(entryIndex >= profile.m_count) break;

		ProfileEntry const & entry = profile.m_entries[entryIndex];

		int x1 = x;
		int x2 = x1 + int(entry.getTimeMsec() * msecToPixels);

		int y1 = y + (i * entryStride);
		int y2 = y1 + barHeight;

		DrawRect(cursor,x1,y1,x2,y2,entry.m_color);
	}

	EndRects(cursor);
}

//----------

void ProfileRenderer::DrawLabels ( Profile const & profile, int x, int y, int entryStart, int entryCount, int entryStride )
{
	for(int i = 0; i < entryCount; i++)
	{
		int entryIndex = i + entryStart;

		if(entryIndex >= profile.m_count) break;

		ProfileEntry const & entry = profile.m_entries[entryIndex];

		int x1 = x - (strlen(entry.m_name) * SimpleFont::getWidth()) - 5;
		int y1 = y + (i * entryStride);

		SimpleFont::Printf(x1,y1,entry.m_name);
	}
}

//-----------------------------------------------------------------------------

void ProfileRenderer::DrawGraph ( Profile * profiles, int graphX, int graphY, int entryStart, int entryCount, int frameCount, int barHeight )
{
	int entrySpace  = g_profilerEntrySpacing.asInt();
	int entryHeight = (frameCount * barHeight) + entrySpace;
	int entryStride = (frameCount * barHeight) + entrySpace;

	int graphWidth  = g_profilerGraphWidth.asInt();
	int graphHeight = (entryCount * entryHeight) + ((entryCount - 1) * entrySpace);

	//----------
	// Draw the backdrop

	{
		int x1 = graphX - 4;
		int y1 = graphY - 4;

		int x2 = x1 + graphWidth + 8;
		int y2 = y1 + graphHeight + 8;

		DrawRect(x1,y1,x2,y2,0x80808080);
	}

	//----------
	// Draw the profile bars

	for(int i = 0; i < frameCount; i++)
	{
		int profileIndex = (ProfileManager::m_frameCursor + (MAX_PROFILE_FRAMES - 1) - i) % MAX_PROFILE_FRAMES;

		Profile * pProfile = &profiles[profileIndex];

		if(pProfile == NULL) continue;
		if(!pProfile->m_bValid) continue;

		int x1 = graphX;
		int y1 = graphY + (barHeight * i);
		
		DrawBars(*pProfile,x1,y1,entryStart,entryCount,entryStride,barHeight);
	}

	//----------
	// Draw the msec lines

	{
		float fx = float(graphX);
		float fy = float(graphY);

		float targetFrameLength = 1000.0f / g_profilerTargetFPS.asFloat();

		float msecToPixels = float(graphWidth) / targetFrameLength;

		for(int i = 0; i <= targetFrameLength; i++)
		{
			if(i%5 == 0)
			{
				DrawRect2(int(fx),int(fy),1,graphHeight,0x70FF6060);
			}
			else
			{
				DrawRect2(int(fx),int(fy),1,graphHeight,0x40A0A0A0);
			}

			DrawRect2(int(fx+1),int(fy),1,graphHeight,0x20000000);

			fx += msecToPixels * 1.0f;
		}
	}

	//----------
	// Draw the labels

	{
		Profile * pProfile = &profiles[ProfileManager::m_frameCursor];

		if(pProfile) DrawLabels(*pProfile,graphX,graphY,entryStart,entryCount,entryStride);
	}
}

//-----------------------------------------------------------------------------

void ProfileRenderer::DrawGraphs ( void )
{
	PIXBeginNamedEvent(0xFF0000FF,"ProfileRenderer::DrawGraphs");

	//----------

	RenderState_2D_PC state;

	if(g_profilerCpuEnable.asBool())
		DrawGraph ( ProfileManager::m_cpuFrames,
		            g_profilerCpuPosX.asInt(),
					g_profilerCpuPosY.asInt(),
					g_profilerCpuEntryStart.asInt(),
					g_profilerCpuEntryCount.asInt(),
					g_profilerFrameCount.asInt(),
					g_profilerBarHeight.asInt() );
	
	if(g_profilerGpuEnable.asBool())
		DrawGraph ( ProfileManager::m_gpuFrames,
		            g_profilerGpuPosX.asInt(),
					g_profilerGpuPosY.asInt(),
					g_profilerGpuEntryStart.asInt(),
					g_profilerGpuEntryCount.asInt(),
					g_profilerFrameCount.asInt(),
					g_profilerBarHeight.asInt() );

	//----------

	PIXEndNamedEvent();
}

//-----------------------------------------------------------------------------

}; // namespace MGSToolbox {

#endif