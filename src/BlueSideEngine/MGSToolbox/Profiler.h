// Copyright © 2005 Microsoft Corporation

#pragma once

#include "MGSToolboxConfig.h"
#ifdef USE_MGSTOOLBOX

namespace MGSToolbox {

#define MAX_PROFILE_ENTRIES 256
#define MAX_PROFILE_FRAMES  32

//-----------------------------------------------------------------------------

class CPUProfiler
{
public:

	CPUProfiler  ( char const * const name, unsigned int color );
	~CPUProfiler ();

protected:

	int m_index;
};

//----------

class GPUProfiler
{
public:

	GPUProfiler ( char const * const name, unsigned int color );
	~GPUProfiler();

protected:

	int m_index;
};

//-----------------------------------------------------------------------------

struct ProfileEntry
{
	char const * m_name;
	unsigned int m_color;
	int          m_index;
	int          m_depth;

	__int64      m_timeBegin;
	__int64      m_timeEnd;

	float getTimeMsec ( void ) const;

	void Clear ( void );
};

//-----------------------------------------------------------------------------

struct Profile
{
	Profile();

	void Clear ( void );

	void Coalesce ( Profile const & source );

	ProfileEntry & getNextEntry ( void );

	//----------

	int  m_index;
	bool m_bValid;

	int  m_count;

    ProfileEntry m_entries[MAX_PROFILE_ENTRIES];
};

//-----------------------------------------------------------------------------

class ProfileManager
{
public:

	static void Install ( void );
	static void Remove  ( void );

	//----------

	static void BeginFrame ( void );
	static void EndFrame   ( void );

    static int  CPUBegin ( char const * const name, unsigned int color );
	static void CPUEnd   ( int index );

	static int  GPUBegin ( char const * const name, unsigned int color );
	static void GPUEnd   ( int index );

	//----------

	static void DumpStats ( void );

	static Profile * getCurrentCpuProfile ( void );
	static Profile * getCurrentGpuProfile ( void );

	static Profile * getLastValidCpuProfile ( void );
	static Profile * getLastValidGpuProfile ( void );

protected:

	//----------

	friend class ProfileRenderer;

	static Profile m_gpuFrames[MAX_PROFILE_FRAMES];
	static Profile m_cpuFrames[MAX_PROFILE_FRAMES];

	static int m_frameCount;
	static int m_frameCursor;
	static int m_frameCursorValid;

	static int m_cpuDepth;
	static int m_gpuDepth;

	static bool m_bInstalled;

private:

	ProfileManager();
	~ProfileManager();
};

//-----------------------------------------------------------------------------

class ProfileRenderer
{
public: 

	static void DrawGraphs ( void );
	static void DrawGraph  ( Profile * profiles, int x, int y, int entryStart, int entryCount, int frameCount, int barHeight );

	static void DrawBars   ( Profile const & profile, int x, int y, int entryStart, int entryCount, int entryStride, int barHeight );
	static void DrawLabels ( Profile const & profile, int x, int y, int entryStart, int entryCount, int entryStride );

	static bool m_bInstalled;
};

//-----------------------------------------------------------------------------

}; // namespace MGSToolbox {

#endif // USE_MGSTOOLBOX