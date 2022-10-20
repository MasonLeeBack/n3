// Copyright 2005 Microsoft Corporation

#include "stdafx.h"
#include "GenericCall.h"

#ifdef USE_MGSTOOLBOX

#include <xtl.h>

#include "MGSToolBox.h"

namespace MGSToolbox {

//-----------------------------------------------------------------------------

template < class T >
void PackArg ( char * & cursor, T const & value )
{
	int	align =	sizeof(T) >	16 ? 16	: sizeof(T);

	cursor += align	- 1; 
	(int&)cursor &=	~(align	- 1); 
	*(T*)cursor	= value; 
	cursor += sizeof(T);
}

//----------

template < class T >
T UnpackArg ( void * & args )
{
	int	align =	sizeof(T) >	16 ? 16	: sizeof(T);

	char * & cursor	= (char	* &)args;

	cursor += align	- 1;
	(int&)cursor &=	~(align	- 1);
	T value	= *(T*)cursor;
	cursor += sizeof(T);

	return value;
}

//-----------------------------------------------------------------------------

void tcall_0 ( void * func, void * args, void * )
{
	typedef	void (*fp)(void);

	((fp)func)();
}

template < class A >
void tcall_1 ( void * func, void * args, void * )
{
	typedef	void (*fp)(A);

	A arg1 = UnpackArg<A>(args);

	((fp)func)(arg1);
}

template < class A, class B >
void tcall_2 ( void * func, void * args, void * )
{
	typedef	void (*fp)(A,B);

	A arg1 = UnpackArg<A>(args);
	B arg2 = UnpackArg<B>(args);

	((fp)func)(arg1,arg2);
}

template < class A, class B, class C >
void tcall_3 ( void * func, void * args, void * )
{
	typedef	void (*fp)(A,B,C);

	A arg1 = UnpackArg<A>(args);
	B arg2 = UnpackArg<B>(args);
	C arg3 = UnpackArg<C>(args);

	((fp)func)(arg1,arg2,arg3);
}

template < class A, class B, class C, class D >
void tcall_4 ( void * func, void * args, void * )
{
	typedef	void (*fp)(A,B,C,D);

	A arg1 = UnpackArg<A>(args);
	B arg2 = UnpackArg<B>(args);
	C arg3 = UnpackArg<C>(args);
	D arg4 = UnpackArg<D>(args);

	((fp)func)(arg1,arg2,arg3,arg4);
}

//-----------------------------------------------------------------------------

template < class R >
void tcall_0r ( void * func, void * args, void * out )
{
	typedef	R (*fp)(void);

	*(R*)out = ((fp)func)();
}

template < class R, class A >
void tcall_1r ( void * func, void * args, void * out )
{
	typedef	R (*fp)(A);

	A arg1 = UnpackArg<A>(args);

	*(R*)out = ((fp)func)(arg1);
}

template < class R, class A, class B >
void tcall_2r ( void * func, void * args, void * out )
{
	typedef	R (*fp)(A,B);

	A arg1 = UnpackArg<A>(args);
	B arg2 = UnpackArg<B>(args);

	*(R*)out = ((fp)func)(arg1,arg2);
}

template < class R, class A, class B, class C >
void tcall_3r ( void * func, void * args, void * out )
{
	typedef	R (*fp)(A,B,C);

	A arg1 = UnpackArg<A>(args);
	B arg2 = UnpackArg<B>(args);
	C arg3 = UnpackArg<C>(args);

	*(R*)out = ((fp)func)(arg1,arg2,arg3);
}

template < class R, class A, class B, class C, class D >
void tcall_4r ( void * func, void * args, void * out )
{
	typedef	R (*fp)(A,B,C,D);

	A arg1 = UnpackArg<A>(args);
	B arg2 = UnpackArg<B>(args);
	C arg3 = UnpackArg<C>(args);
	D arg4 = UnpackArg<D>(args);

	*(R*)out = ((fp)func)(arg1,arg2,arg3,arg4);
}

//-----------------------------------------------------------------------------

struct dispatchentry
{
	typedef void (*dispatcher)(void * func, void * args, void * out);

	char * format;
	dispatcher dispatch;
};

#define DISPATCH0v()          { "void",                          tcall_0 }
#define DISPATCH1v(A)         { "void," #A,                      tcall_1<A> }
#define DISPATCH2v(A,B)       { "void," #A "," #B,               tcall_2<A,B> }
#define DISPATCH3v(A,B,C)     { "void," #A "," #B "," #C,        tcall_3<A,B,C> }
#define DISPATCH4v(A,B,C,D)   { "void," #A "," #B "," #C "," #D, tcall_4<A,B,C,D> }

#define DISPATCH0r(R)         { #R,                              tcall_0r<R> }
#define DISPATCH1r(R,A)       { #R "," #A,                       tcall_1r<R,A> }
#define DISPATCH2r(R,A,B)     { #R "," #A "," #B,                tcall_2r<R,A,B> }
#define DISPATCH3r(R,A,B,C)   { #R "," #A "," #B "," #C,         tcall_3r<R,A,B,C> }
#define DISPATCH4r(R,A,B,C,D) { #R "," #A "," #B "," #C "," #D,  tcall_4r<R,A,B,C,D> }


dispatchentry g_dispatchTable[]	= 
{
	DISPATCH0v(),

	DISPATCH0r(int),
	DISPATCH0r(float),
	DISPATCH0r(__vector4),

	DISPATCH1v(int),
	DISPATCH1v(float),
	DISPATCH1v(__vector4),

	DISPATCH1r(int,int),
	DISPATCH1r(int,float),
	DISPATCH1r(int,__vector4),
	DISPATCH1r(float,int),
	DISPATCH1r(float,float),
	DISPATCH1r(float,__vector4),
	DISPATCH1r(__vector4,int),
	DISPATCH1r(__vector4,float),
	DISPATCH1r(__vector4,__vector4),

	DISPATCH2v(int,int),
	DISPATCH2v(int,float),
	DISPATCH2v(int,__vector4),
	DISPATCH2v(float,int),
	DISPATCH2v(float,float),
	DISPATCH2v(float,__vector4),
	DISPATCH2v(__vector4,int),
	DISPATCH2v(__vector4,float),
	DISPATCH2v(__vector4,__vector4),

	DISPATCH4v(int,float,__vector4,int),
};

bool GenericCall ( void * func, char * format, void * args, void * out )
{
	int tableSize = sizeof(g_dispatchTable) / sizeof(dispatchentry);

	for(int i = 0; i < tableSize; i++)
	{
		if(stricmp(format,g_dispatchTable[i].format) == 0)
		{
			g_dispatchTable[i].dispatch(func,args,out);

			return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------

}; // namespace	MGSToolbox

#endif // USE_MGSTOOLBOX