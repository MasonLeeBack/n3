/*************************************************************************/
/** IntBox3Frustum.cpp

  * @author	Kim Jae Hyun	<neigex@phantagram.com>
  * @since	05-Mar-03	Created
**************************************************************************/
#include	"StdAfx.h"
#include	"Box3.h"
#include	"Frustum.h"
#include	"Sphere.h"
#include	"IntBox3Frustum.h"

/*************************************************************************/
/** 
  * Planes
                  +-----+
				 /| 3  /|
			   +-----5  |
			   | 0|  | 1|
			   |  4--|--+     y z
			   | /  2| /      |/
			   +-----+        +---x

  * Vertices
                  3-----7           011---111
                 /|    /|           /|    /|
               2-----6  |        010---110 |
               |  |  |  |         |  |  |  |
               |  1--|--5         | 001-|-101
               | /   | /          | /   |/
               0-----4           000---100
                  (1)                (2)
**************************************************************************/
static int	LUT_planes[8][3] =
{
	{4, 0, 2},
	{4, 1, 2},
	{4, 0, 3},
	{4, 1, 3},
	{5, 0, 2},
	{5, 1, 2},
	{5, 0, 3},
	{5, 1, 3},
};

//! Integer representation of a floating-point value
#define	IR(x)				((unsigned int&)x)

/*************************************************************************/
/** Box3�� Frustum���� Intersection Test.
	far_offset��ŭ Frustum�� far������ ���ϼ� �ִ�.

  * Box3�� Vertex�� �̸� compute�Ǿ� �������� �����Ѵ�.		(get_diagonal)
  * Frustum�� Vertex�� �̸� compute�Ǿ� �������� �����Ѵ�.	(N, d)
**************************************************************************/
bool
TestIntersection( const Box3 & B, const Frustum & F, float far_offset )
{
	// @note	0 <= Box3.E.i (i=x,y,z) �̾�� �Ѵ�.
	// Ư�� Intersection Test���� ������ ����Ƿ� ���⼭ check�ϱ�� �Ѵ�.
	// Box3 ��ο��� check�ϱ⿣ load�� �ʹ� ũ��.
	// �Ʒ��� �񱳴� 0.f < (float)f ���� ������. ^^
	assert( IR(B.E[0]) ^ 0x80000000 );
	assert( IR(B.E[1]) ^ 0x80000000 );
	assert( IR(B.E[2]) ^ 0x80000000 );

	// Octant Test; Frustum�� 6���߿� 3���� �����Ѵ�. (@see	Real-Time Rendering)
	int			nOctant=0;
	BSVECTOR	D = B.C - F.C;

	if( 0.f < BsVec3Dot( &D, &F.L ) )	nOctant ^= 1;
	if( 0.f < BsVec3Dot( &D, &F.U ) )	nOctant ^= 2;
	if( 0.f < BsVec3Dot( &D, &F.D ) )	nOctant ^= 4;

	int			i=0;
	bool		intersect=false;
	BSVECTOR	Vmin, Vmax;

	if( LUT_planes[nOctant][i] == 5 )
	{
		// get Plane.
		const BSVECTOR	& N = F.N[LUT_planes[nOctant][i]];
		const float			& d = F.d[LUT_planes[nOctant][i]];

		B.get_diagonal( Vmin, Vmax, N );

		if( 0.f < BsVec3Dot(&N, &Vmin) + d - far_offset )
			return false;	// outside
		if( 0.f <= BsVec3Dot(&N, &Vmax) + d - far_offset )
			intersect = true;

		++i;
	}

	for( ; i<3 ; ++i )
	{
		// get Plane.
		const BSVECTOR	& N = F.N[LUT_planes[nOctant][i]];
		const float			& d = F.d[LUT_planes[nOctant][i]];

		B.get_diagonal( Vmin, Vmax, N );

		if( 0.f < BsVec3Dot(&N, &Vmin) + d )
			return false;	// outside
		if( 0.f <= BsVec3Dot(&N, &Vmax) + d )
			intersect = true;
	}
//	if( intersect == true )
//		return true;		// intersect
//	else
//		return true;		// inside
	return true;
}

bool
TestIntersection( const Box3 & B, const Frustum & F )
{
	int			nOctant=0;
	BSVECTOR	D = B.C - F.C;

	if( 0.f < BsVec3Dot( &D, &F.L ) )	nOctant ^= 1;
	if( 0.f < BsVec3Dot( &D, &F.U ) )	nOctant ^= 2;
	if( 0.f < BsVec3Dot( &D, &F.D ) )	nOctant ^= 4;

	int			i=0;
	bool		intersect=false;
	BSVECTOR	Vmin, Vmax;

	for( i=0; i<3 ; ++i )
	{
		// get Plane.
		const BSVECTOR	& N = F.N[LUT_planes[nOctant][i]];
		const float			& d = F.d[LUT_planes[nOctant][i]];

		B.get_diagonal( Vmin, Vmax, N );

		if( 0.f < BsVec3Dot(&N, &Vmin) + d )
			return false;	// outside
		/* 
		if( 0.f <= BsVec3Dot(&N, &Vmax) + d )
			intersect = true;
			*/
	}
//	if( intersect == true )
//		return true;		// intersect
//	else
//		return true;		// inside
	return true;
}

bool TestIntersection( const Sphere & S, const Box3 & BaseBox )
{
	BSVECTOR	P = S.C - BaseBox.C;
	if( BaseBox.E[0] + S.R < fabsf(BsVec3Dot( &P, &BaseBox.A[0] )) )	return false;
	if( BaseBox.E[1] + S.R < fabsf(BsVec3Dot( &P, &BaseBox.A[1] )) )	return false;
	if( BaseBox.E[2] + S.R < fabsf(BsVec3Dot( &P, &BaseBox.A[2] )) )	return false;
	return true;

}