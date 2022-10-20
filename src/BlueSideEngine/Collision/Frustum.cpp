/*************************************************************************/
/** Frustum.cpp

  * @author	Kim Jae Hyun	<neigex@phantagram.com>
  * @since	06-Mar-03	Created
**************************************************************************/
#include	"StdAfx.h"
#include	"Frustum.h"

void Frustum::compute_vertices()
{
	BSVECTOR	kD = n*D;
	BSVECTOR	kL = l*L;
	BSVECTOR	kU = u*U;

	V[0] = kD - kL - kU;	// 000
	V[1] = kD + kL - kU;	// 001
	V[2] = kD - kL + kU;	// 010
	V[3] = kD + kL + kU;	// 011

	for( int i=0, p=4 ; i<4 ; ++i, ++p ){
		V[p] = E + ratio*V[i];
		V[i] += E;
	}
}

static int	LUT_points[6] = {0, 1, 0, 2, 0, 4};

void Frustum::update()
{
	C = E + (n+f)*0.5f * D;

	N[0] = -n*L - l*D;
	N[1] = n*L - l*D;
	N[2] = -n*U - u*D;
	N[3] = n*U - u*D;
	N[4] = -D;
	N[5] = D;

	int i;
	for( i=0 ; i<4 ; ++i ){
		BsVec3Normalize( &N[i], &N[i] );
		d[i] = -BsVec3Dot( &N[i], &V[LUT_points[i]] );
	}
	for( ; i<6 ; ++i ){
		d[i] = -BsVec3Dot( &N[i], &V[LUT_points[i]] );
	}
}