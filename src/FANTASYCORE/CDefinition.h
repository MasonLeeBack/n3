/*-----------------------------------------------------------------------------------
-
-				CDefinition.h					Choi Jae Young
-												2005. 8. 1
-													v 0.01
-							In Blue-Side
-
-
-			p.s. Fx 에서 제가(최재영) 사용하기 위해 만들어놓은 정의 목록입니다.
-				 작업하면서 필요한 것들을 다른 소스와 충돌없이 작업하려고 만들어
-				 놓은거라서, 다른 소스에는 영향을 미치지 않습니다.
-
------------------------------------------------------------------------------------*/

#pragma once

#include "bstreamext.h"

#ifdef _WINDOWS_
#define PUBLIC_GETSET_PROP(type, accessorName, varName) \
		public:\
		type varName;\
		public:\
		inline const type Get##accessorName##() const { return varName; } \
		inline type Get##accessorName##() { return varName; } \
		inline void Set##accessorName##( type val ) { varName = val; }

#define PRIVATE_GETSET_PROP(type, accessorName, varName) \
		private:\
		type varName;\
		public:\
		inline const type Get##accessorName##() const { return varName; } \
		inline type Get##accessorName##() { return varName; } \
		inline void Set##accessorName##( type val ) { varName = val; }

#define PROTECTED_GETSET_PROP(type, accessorName, varName) \
		protected:\
		type varName;\
		public:\
		inline const type Get##accessorName##() const { return varName; } \
		inline type Get##accessorName##() { return varName; } \
		inline void Set##accessorName##( type val ) { varName = val; }

#else

#define PUBLIC_GETSET_PROP(type, accessorName, varName) \
		public:\
		type varName;\
		public:\
		type Get##accessorName##() { return varName; } \
		void Set##accessorName##( type val ) { varName = val; }

#define PRIVATE_GETSET_PROP(type, accessorName, varName) \
		private:\
		type varName;\
		public:\
		type Get##accessorName##() { return varName; } \
		void Set##accessorName##( type val ) { varName = val; }

#define PROTECTED_GETSET_PROP(type, accessorName, varName) \
		protected:\
		type varName;\
		public:\
		type Get##accessorName##() { return varName; } \
		void Set##accessorName##( type val ) { varName = val; }
#endif


#define BINARY_VALUE_READER_BSTREAM_NOTRET( file, type, container ) \
	BFileStream*	pStream = NULL;\
	pStream	= new BFileStream(file);\
	if( NULL != pStream ) {\
	pStream->Read( &container, sizeof(type), ENDIAN_FOUR_BYTE);\
	delete pStream;}


