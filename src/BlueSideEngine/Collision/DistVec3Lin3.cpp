/*************************************************************************/
/** DistVec3Lin3.cpp

  * @author	Kim Jae Hyun	<neigex@phantagram.com>
  * @since	08-Oct-02	Created
**************************************************************************/
#include	"StdAfx.h"
#include	"Ray3.h"

#include	"DistVec3Lin3.h"


float
SqrDistance( const BSVECTOR & P, const Line3 & L, float * pt/*=NULL*/ )
{
	BSVECTOR	diff = P - L.P;
	float	sqr_len = BsVec3Dot( &L.D, &L.D );
	float	t = BsVec3Dot( &diff, &L.D ) / sqr_len;
	diff -= t * L.D;

	if( pt ){
		*pt = t;
	}

	return BsVec3Dot( &diff, &diff );
}


float
SqrDistance( const BSVECTOR & P, const Ray3 & R, float * pt/*=NULL*/ )
{
	BSVECTOR	diff = P - R.P;
	float	t = BsVec3Dot( &diff, &R.D );

	if( t <= 0.f ){
		t = 0.f;
	}
	else{
		t /= BsVec3Dot( &R.D, &R.D );
		diff -= t * R.D;
	}

	if( pt ){
		*pt = t;
	}

	return BsVec3Dot( &diff, &diff );
}

/*
float
SqrDistance( const D3DXVECTOR3 & P, const Segment3 & S, float * pt )
{
	D3DXVECTOR3	diff = P - S.C;
	float	t = D3DXVec3Dot( &diff, &S.U );

	if( t < -S.r ){
		// Segment 시작점 쪽
		t = -S.r;
		diff += S.rU;
	}
	else if( t > S.r ){
		// Segment 끝점 쪽
		t = S.r;
		diff -= S.rU;
	}
	else{
		// Segment 내부
		diff -= t * S.U;
	}

	if( pt ){
		*pt = t;
	}

	return D3DXVec3Dot( &diff, &diff );
}
*/

float
SqrDistance( const BSVECTOR & P, const Segment3 & S, float * pt/*=NULL*/ )
{
	BSVECTOR	diff = P - S.P;
	float	t = BsVec3Dot( &diff, &S.D );

	if( t <= 0.f ){
		t = 0.f;
	}
	else{
		float	sqr_len = BsVec3Dot( &S.D, &S.D );
		if( sqr_len <= t ){
			t = 1.f;
			diff -= S.D;
		}
		else{
			t /= sqr_len;
			diff -= t * S.D;
		}
	}

	if( pt ){
		*pt = t;
	}

	return BsVec3Dot( &diff, &diff );
}
