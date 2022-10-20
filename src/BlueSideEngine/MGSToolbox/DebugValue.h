// Copyright © 2005 Microsoft Corporation

#pragma once

#include "MGSToolboxConfig.h"
#ifdef USE_MGSTOOLBOX

namespace MGSToolbox {

//-----------------------------------------------------------------------------

class DebugValue
{
public:

	DebugValue ( char const * group, char const * name, float value, float min, float max, float step, bool dynamic = false );
	DebugValue ( char const * group, char const * name, int value, int min = INT_MIN, int max = INT_MAX, int step = 1, bool dynamic = false);
	DebugValue ( char const * group, char const * name, bool value, bool dynamic = false );
	DebugValue ( char const * group, char const * name, char const * value, bool dynamic = false );

	typedef void (*void_fn)(void);
	typedef void (*tostring_fn)(char * buffer, int bufferSize);

	DebugValue ( char const * group, char const * name, void_fn pfIncrement, void_fn pfDecrement = NULL, tostring_fn pfGetValue = NULL, bool dynamic = false );

	~DebugValue ( void );

	//----------

	void Increment ( void );
	void Decrement ( void );

	//----------

	char const * getGroup ( void ) const { return m_group; }
	char const * getName  ( void ) const { return m_name; }

	int   asInt   ( void ) const;
	float asFloat ( void ) const;
	bool  asBool  ( void ) const;

	void fromBool( bool value );

	char const * asString ( void ) const;

	void  toString   ( char * buffer, int bufferSize ) const;
	void  fromString ( char * buffer );

	bool  isDirty ( void ) const { return m_dirty; }
	void  setDirty ( bool dirty ) { m_dirty = dirty; }

	//----------

	static DebugValue * getHead ( void ) { return s_pHead; }
	static DebugValue * getTail ( void ) { return s_pTail; }

	DebugValue * getNext ( void ) { return m_pNext; }
	DebugValue * getPrev ( void ) { return m_pPrev; }

	//----------

	static void Sort ( void );

	static DebugValue * Find ( char * group, char * name );

	static bool Set ( char * group, char * name, float value );

	//----------

protected:

	void Link   ( void );
	void Unlink ( void );

	void MakeDynamic ( void );

	//----------
    
	static DebugValue * s_pHead;
	static DebugValue * s_pTail;

	DebugValue * m_pPrev;
	DebugValue * m_pNext;

	char const * m_group;
	char const * m_name;

	enum Type
	{
		DVT_float,
		DVT_int,
		DVT_bool,
		DVT_string,
		DVT_callback,
	};

	Type m_type;
	bool m_dirty;
	bool m_dynamic;

	double m_value;
	double m_min;
	double m_max;
	double m_step;
	char * m_string;

	static const int s_stringLength = 256;

	void_fn  m_pfIncrement;
	void_fn  m_pfDecrement;
	tostring_fn m_pfGetValue;
};

//-----------------------------------------------------------------------------

}; // namespace MGSToolbox {

#endif // USE_MGSTOOLBOX