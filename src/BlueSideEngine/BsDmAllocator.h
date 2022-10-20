#pragma once

#include "BsMemTracker.h"
#include "BsMemSystem.h"

template <typename T> class dm_allocator;

// Utility

namespace dm_alloc{
	inline void destruct(char *){}
	inline void destruct(wchar_t*){}
	template <typename T> 
		inline void destruct(T *t){t->~T();}
} // namespace

// Specialized dm_allocator

template <> class dm_allocator<void>
{
public:
	typedef void* pointer;
	typedef const void* const_pointer;
	// reference to void members are impossible.
	typedef void value_type;
	template <class U> 
		struct rebind { typedef dm_allocator<U> other; };
};    

// General dm_allocator

template <typename T>
class dm_allocator
{
public:
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef T value_type;

	template <class U> 
		struct rebind { typedef dm_allocator<U> other; };

	dm_allocator(){}
	pointer address(reference x) const {retuen &x;}
	const_pointer address(const_reference x) const {return &x;}

	pointer allocate(size_type n, dm_allocator<void>::const_pointer hint = 0)
	{
		void * p = DmAllocatePool(n*sizeof(T));
		if ( p )
			BsMemTracker::Add( eMemorySystem_Debug, n*sizeof(T) );

		return static_cast<pointer>(p);
	}
	//for Dinkumware:
	char *_Charalloc(size_type n)
	{
		void * p = DmAllocatePool(n);
		if ( p )
			BsMemTracker::Add( eMemorySystem_Debug, n );
		
		return static_cast<char*>(p);
	}
	// end Dinkumware

	template <class U> dm_allocator(const dm_allocator<U>&){}
	void deallocate(pointer p, size_type n)
	{
		if ( p )
		{
			BsMemTracker::Remove( eMemorySystem_Debug, n*sizeof(T) );
			DmFreePool(p);
		}
	}
	void deallocate(void *p, size_type n)
	{
		if ( p )
		{
			BsMemTracker::Remove( eMemorySystem_Debug, n );
			DmFreePool(p);
		}
	}
	size_type max_size() const throw() {return size_t(-1) / sizeof(value_type);}
	void construct(pointer p, const T& val)
	{
		new(static_cast<void*>(p)) T(val);
	}
	void construct(pointer p)
	{
		new(static_cast<void*>(p)) T();
	}
	void destroy(pointer p){dm_alloc::destruct(p);}
};

template <typename T, typename U>
inline bool operator==(const dm_allocator<T>&, const dm_allocator<U>){return true;}

template <typename T, typename U>
inline bool operator!=(const dm_allocator<T>&, const dm_allocator<U>){return false;}
