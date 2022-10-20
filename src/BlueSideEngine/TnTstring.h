/* aleksger - redefining all strings to use safe strings.
 * The non-counting version (the ones that don't take truncate) will invoke the invalid parameter handler and
 * throw an Access Violation if size of the destination buffer is too small for the operation.
 
 * The counting versions (strncpy_s, strncat_s, etc) will truncate and properly terminate the string
 * in case that the destination is too small for the operation.

 * For the purpose of being able to know when problem occurs, the version that calls the invalid parameter handler
 * has been chosen.
 */
#ifdef _XBOX

#if _XDK_VER < 2571
	// Defining C++ overloads
	#define _CRT_ALTERNATIVE_INLINES 1
	#define _SAFECRT_USE_CPP_OVERLOADS 1
	#include <safecrt.h>
#endif

#define strcpy(dst,src) strcpy_s(dst,src)
#define strncpy(dst,src,cnt) strcpy_s(dst,cnt,src)
#define strcat(dst,src) strcat_s(dst,src)
#define snprintf_s _snprintf_s
#define sprintf sprintf_s
#define vsnprintf vsnprintf_s
#define _vsnprintf _vsnprintf_s
#define _vsnwprintf _vsnwprintf_s

/*	These versions will truncate instead of return empty strings.
   They will still call the invalid parameter handler and therefore raise Access Violation.

	#define strcpy(dst,src) strncpy_s(dst,src,_TRUNCATE)
	#define strncpy(dst,src,cnt) strncpy_s(dst,cnt,src,_TRUNCATE)
	#define strcat(dst,src) strncat_s(dst,src,_TRUNCATE)
*/
#else

// Preventing deprecation warnings.
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

// Taken from xbox stdlib:
#if !defined(_countof)
#if !defined(__cplusplus)
#define _countof(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#else
extern "C++"
{
template <typename _CountofType, size_t _SizeOfArray>
char (*__countof_helper(UNALIGNED _CountofType (&_Array)[_SizeOfArray]))[_SizeOfArray];
#define _countof(_Array) sizeof(*__countof_helper(_Array))
}
#endif
#endif

//Defining safe CRT functions to equivalent strsafe functions.
#define strcpy_s(src,size,dest) StringCchCopy(src,size,dest)
#define strcat_s(src,size,dest) StringCchCat(src,size,dest)
#define strncpy_s(src, size, dest,count) StringCchCopy(src,size,dest)
#define sprintf_s StringCchPrintf
#define vsprintf_s StringCchVPrintf
#define _vsnprintf_s StringCchVPrintf


/*#define strcpy(dst,src) strcpy_s(dst,src)
//#define strncpy(dst,src,cnt) strncpy_s(dst,cnt, src, _TRUNCATE)
#define strncpy(dst,src,cnt) strcpy_s(dst,cnt,src)
#define strcat(dst,src) strcat_s(dst,src)
#define snprintf_s _snprintf_s
#define sprintf sprintf_s
*/
#endif