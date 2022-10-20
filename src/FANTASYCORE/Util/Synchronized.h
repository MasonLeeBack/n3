#ifndef  _SYNCHRONIZED_H_
#define  _SYNCHRONIZED_H_
/*
	Name_Scope Synchronized Pattern
*/
class Synchronized	/* Synchorinize Class */
{
public:

	/*
			CRITICAL_SECTION struct capsule For Synchronized class 
	*/
	class SectionObject
	{
	public:
		SectionObject()
		{
			InitializeCriticalSection( &cs );
		}
		virtual ~SectionObject()
		{
			DeleteCriticalSection( &cs );
		}
		inline void enter()
		{
			EnterCriticalSection( &cs );
		}
		inline void leave()
		{
			LeaveCriticalSection( &cs );
		}
	private:
		CRITICAL_SECTION cs;
	};

	/*

	class SectionObject//		Event HANDLE capsule For Synchronized class 
	{
	public:
		SectionObject()
		{
			m_threadID = 0x00;
			m_recursiveReferenceCount = 0;
			m_event = CreateEvent( NULL, FALSE, TRUE, NULL );
		}
		virtual ~SectionObject()
		{
			ResetEvent( m_event );
			CloseHandle ( m_event );
		}
		inline void enter()
		{
			if ( m_threadID != GetCurrentThreadId() )
			{
				WaitForSingleObject( m_event, INFINITE );
				m_threadID = GetCurrentThreadId();
				m_recursiveReferenceCount = 0;
			}
			InterlockedIncrement( ( LPLONG )&m_recursiveReferenceCount );
		}
		inline void leave()
		{
			if ( m_recursiveReferenceCount == 0 ) return;
			InterlockedDecrement( ( LPLONG )&m_recursiveReferenceCount );
			if ( m_recursiveReferenceCount == 0 )
			{
				m_threadID = 0x00;
				SetEvent( m_event );
			}
		}
	private:
		DWORD m_threadID;
		int m_recursiveReferenceCount;	// OwningThread Recursive Entering
		HANDLE m_event;
	};
	*/
public:
	Synchronized( SectionObject *so )
	{
		m_sectionObject = so;

		m_sectionObject->enter();
	};

	virtual ~Synchronized()
	{
		m_sectionObject->leave();
	};

private:
	SectionObject *m_sectionObject;
};


#endif
