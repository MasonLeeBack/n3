// Copyright 2005 Microsoft Corporation

#include "stdafx.h"
#include "DebugValue.h"

#ifdef USE_MGSTOOLBOX

#include <xtl.h>
#include <stdio.h>
#include <assert.h>
#include <vector>
#include <algorithm>

namespace MGSToolbox {

DebugValue * DebugValue::s_pHead = NULL;
DebugValue * DebugValue::s_pTail = NULL;

//-----------------------------------------------------------------------------

DebugValue::DebugValue ( char const * group, char const * name, float value, float min, float max, float step, bool dynamic )
: m_pPrev(NULL),
  m_pNext(NULL),
  m_group(group),
  m_name(name),
  m_type(DVT_float),
  m_dirty(false),
  m_dynamic(dynamic),
  m_value(value),
  m_min(min),
  m_max(max),
  m_step(step),
  m_pfIncrement(NULL),
  m_pfDecrement(NULL),
  m_pfGetValue(NULL)
{
	if(m_dynamic) MakeDynamic();

	Link();
}

//----------

DebugValue::DebugValue ( char const * group, char const * name, int value, int min, int max, int step, bool dynamic )
: m_pPrev(NULL),
  m_pNext(NULL),
  m_group(group),
  m_name(name),
  m_type(DVT_int),
  m_dirty(false),
  m_dynamic(false),
  m_value(value),
  m_min(min),
  m_max(max),
  m_step(step),
  m_pfIncrement(NULL),
  m_pfDecrement(NULL),
  m_pfGetValue(NULL)
{
	if(dynamic) MakeDynamic();

	Link();
}

//----------

DebugValue::DebugValue ( char const * group, char const * name, bool value, bool dynamic )
: m_pPrev(NULL),
  m_pNext(NULL),
  m_group(group),
  m_name(name),
  m_type(DVT_bool),
  m_dirty(false),
  m_dynamic(false),
  m_value(value ? 1.0f : 0.0f),
  m_min(0.0f),
  m_max(1.0f),
  m_step(1.0f),
  m_pfIncrement(NULL),
  m_pfDecrement(NULL),
  m_pfGetValue(NULL)
{
	if(dynamic) MakeDynamic();

	Link();
}

//----------

DebugValue::DebugValue ( char const * group, char const * name, char const * value, bool dynamic )
: m_pPrev(NULL),
  m_pNext(NULL),
  m_group(group),
  m_name(name),
  m_type(DVT_string),
  m_dirty(false),
  m_dynamic(false),
  m_value(0.0f),
  m_min(0.0f),
  m_max(0.0f),
  m_step(0.0f),
  m_pfIncrement(NULL),
  m_pfDecrement(NULL),
  m_pfGetValue(NULL)
{
	if(dynamic) MakeDynamic();

	m_string = new char[s_stringLength];

	strncpy(m_string,value,s_stringLength);

	Link();
}

//----------

DebugValue::DebugValue ( char const * group, char const * name, void_fn pfIncrement, void_fn pfDecrement, tostring_fn pfGetValue, bool dynamic )
: m_pPrev(NULL),
  m_pNext(NULL),
  m_group(group),
  m_name(name),
  m_type(DVT_callback),
  m_dirty(false),
  m_dynamic(false),
  m_value(0.0f),
  m_min(0.0f),
  m_max(1.0f),
  m_step(1.0f),
  m_pfIncrement(pfIncrement),
  m_pfDecrement(pfDecrement),
  m_pfGetValue(pfGetValue)
{
	if(dynamic) MakeDynamic();

	Link();
}

//----------

DebugValue::~DebugValue ( void )
{
	if(m_type == DVT_string)
	{
		delete [] m_string;
		m_string = NULL;
	}

	if(m_dynamic)
	{
		delete [] m_group;
		m_group = NULL;

		delete [] m_name;
		m_name = NULL;
	}

	Unlink();
}

//-----------------------------------------------------------------------------

void DebugValue::MakeDynamic ( void )
{
	if(!m_dynamic)
	{
		char * group = new char[s_stringLength];
		char * name = new char[s_stringLength];

		strncpy(group,m_group,s_stringLength);
		strncpy(name,m_name,s_stringLength);

		m_group = group;
		m_name = name;

		m_dynamic = true;
	}
}

//-----------------------------------------------------------------------------

void DebugValue::Link ( void )
{
	if(s_pTail)
	{
		m_pNext = NULL;
		m_pPrev = s_pTail;

		s_pTail->m_pNext = this;
		s_pTail = this;
	}
	else
	{
		s_pHead = this;
		s_pTail = this;

		m_pPrev = NULL;
		m_pNext = NULL;
	}
}

//----------

void DebugValue::Unlink ( void )
{
	if(m_pNext) m_pNext->m_pPrev = m_pPrev;
	if(m_pPrev) m_pPrev->m_pNext = m_pNext;

	if(s_pHead == this) s_pHead = m_pNext;
	if(s_pTail == this) s_pTail = m_pPrev;

	m_pNext = NULL;
	m_pPrev = NULL;
}

//-----------------------------------------------------------------------------

void DebugValue::Increment ( void )
{
	switch(m_type)
	{
	case DVT_float:
	case DVT_int:
		m_value += m_step;
		if(m_value > m_max) m_value = m_max;
		break;

	case DVT_bool:
		m_value = 1.0 - m_value;
		break;

	case DVT_callback:
		if(m_pfIncrement) m_pfIncrement();
		break;

	case DVT_string:
	default:
		break;
	}

	m_dirty = true;
}

//----------

void DebugValue::Decrement ( void )
{
	switch(m_type)
	{
	case DVT_float:
	case DVT_int:
		m_value -= m_step;
		if(m_value < m_min) m_value = m_min;
		break;

	case DVT_bool:
		m_value = 1.0 - m_value;
		break;

	case DVT_callback:
		if(m_pfDecrement) m_pfDecrement();
		break;

	case DVT_string:
	default:
		break;
	}

	m_dirty = true;
}

//-----------------------------------------------------------------------------

int DebugValue::asInt ( void ) const
{
	assert((m_type == DVT_float) || (m_type == DVT_int));

	return (int)m_value;
}

//----------

float DebugValue::asFloat ( void ) const
{
	assert((m_type == DVT_float) || (m_type == DVT_int));

	return (float)m_value;
}

//----------

bool DebugValue::asBool ( void ) const
{
	assert(m_type == DVT_bool);

	return (m_value == 0.0f) ? false : true;
}

//----------

char const * DebugValue::asString ( void ) const
{
	assert(m_type == DVT_string);

	return m_string;
}

//----------

void DebugValue::fromBool ( bool value )
{
	switch(m_type)
	{
	case DVT_float:
	case DVT_int:
	case DVT_bool:
		m_value = value ? 1.0 : 0.0;
		break;
	case DVT_callback:
		break;
	case DVT_string:
		strncpy(m_string,value?"true":"false",s_stringLength);
		break;
	default:
		break;
	}
}

//-----------------------------------------------------------------------------

void DebugValue::toString ( char * buffer, int bufferSize ) const
{
	switch(m_type)
	{
	case DVT_float:
		_snprintf(buffer,bufferSize,"%f (%f,%f : %f)",(float)m_value,(float)m_min,(float)m_max,(float)m_step);
		break;
	case DVT_int:
		{
			if((m_min == INT_MIN) && (m_max == INT_MAX))
			{
				_snprintf(buffer,bufferSize,"%d (-inf,inf : %d)",(int)m_value,(int)m_step);
			}
			else
			{
				_snprintf(buffer,bufferSize,"%d (%d,%d : %d)",(int)m_value,(int)m_min,(int)m_max,(int)m_step);
			}
		}
		break;
	case DVT_bool:
		_snprintf(buffer,bufferSize,"%s",(m_value == 0.0f) ? "false" : "true");
		break;
	case DVT_callback:
		{
			if(m_pfGetValue)
			{
				m_pfGetValue(buffer,bufferSize);
			}
			else
			{
				_snprintf(buffer,bufferSize,"(callback)");
			}
		}
		break;
	case DVT_string:
		strncpy(buffer,m_string,bufferSize);
		break;
	default:
		_snprintf(buffer,bufferSize,"(invalid type)");
		break;
	}
}

//----------

void DebugValue::fromString ( char * buffer )
{
	switch(m_type)
	{
	case DVT_float:
		m_value = atof(buffer);
		break;
	case DVT_int:
		m_value = atoi(buffer);
		break;
	case DVT_bool:
		m_value = (buffer[0] == 't') ? 1.0 : 0.0;
		break;
	case DVT_callback:
		break;
	case DVT_string:
		strncpy(m_string,buffer,s_stringLength);
		break;
	default:
		break;
	}
}

//-----------------------------------------------------------------------------

bool debugValueLess ( DebugValue * A, DebugValue * B )
{
	int groupOrder = stricmp(A->getGroup(),B->getGroup());
	int nameOrder = stricmp(A->getName(),B->getName());

	return (groupOrder < 0) || ((groupOrder == 0) && (nameOrder < 0));
}

void DebugValue::Sort ( void )
{
	std::vector<DebugValue*> values;

	for(DebugValue * cursor = s_pHead; cursor; cursor = cursor->m_pNext)
	{
		values.push_back(cursor);
	}

	std::sort(values.begin(),values.end(),debugValueLess);

	values.front()->m_pPrev = NULL;
	values.back()->m_pNext = NULL;

	for(unsigned int i = 0; i < values.size() - 1; i++)
	{
		values[i]->m_pNext = values[i+1];
		values[i+1]->m_pPrev = values[i];
	}

	s_pHead = values.front();
	s_pTail = values.back();
}

//-----------------------------------------------------------------------------

DebugValue * DebugValue::Find ( char * group, char * name )
{
	for(DebugValue * cursor = s_pHead; cursor; cursor = cursor->m_pNext)
	{
		if(stricmp(group,cursor->m_group)) continue;
		if(stricmp(name,cursor->m_name)) continue;

		return cursor;
	}

	return NULL;
}

//-----------------------------------------------------------------------------

bool DebugValue::Set ( char * group, char * name, float value )
{
	DebugValue * d = Find(group,name);

	if(d) 
	{
		d->m_value = value;
		return true;
	}
	else
	{
		return false;
	}
}

//-----------------------------------------------------------------------------

}; // namespace MGSToolbox

#endif // USE_MGSTOOLBOX