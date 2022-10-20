/*************************************************************************/
/** IntSphLin3.cpp

  * @author	Kim Jae Hyun	<neigex@phantagram.com>
  * @since	14-Oct-02	Created
**************************************************************************/
#include	"StdAfx.h"
#include	"Sphere.h"
#include	"Ray3.h"

#include	"DistVec3Lin3.h"
#include	"IntSphLin3.h"


///////////////////////////////////////////////////////////////////////////
/*************************************************************************/
/**
**************************************************************************/
bool
TestIntersection( const Sphere & S, const Line3 & L )
{
	float	dist = SqrDistance( S.C, L );
	return dist <= S.R * S.R;
}

bool
TestIntersection( const Sphere & S, const Ray3 & R )
{
	float	dist = SqrDistance( S.C, R );
	return dist <= S.R * S.R;
}

bool
TestIntersection( const Sphere & S, const Segment3 & Seg )
{
	float	dist = SqrDistance( S.C, Seg );
	return dist <= S.R * S.R;
}


///////////////////////////////////////////////////////////////////////////
/*************************************************************************/
/** t = (-b  +- sqrt(b ^2 - 4ac)) / 2a
	  = (-b' +- sqrt(b'^2 -  ac)) /  a

  * Sphere: (C, R), Line: (P, D)
  * Q(t) = | P + t*D - C |^2
	     = D^2*t^2 + 2*(P-C)*D*t + (P-C)^2
		 = a*t^2 + 2*b*t + c
  * a = D^2, b = (P-C)*D, c = (P-C)^2
**************************************************************************/
bool
FindIntersection( const Sphere & S, const Line3 & L, float * pt0/*=NULL*/, float * pt1/*=NULL*/ )
{
	// set up quadratic Q(t) = a*t^2 + 2*b*t + c
	BSVECTOR	diff = L.P - S.C;						// P - C
	float	a = BsVec3Dot( &L.D, &L.D );				// D^2
	float	b = BsVec3Dot( &diff, &L.D );				// (P-C)*D
	float	c = BsVec3Dot( &diff, &diff ) - S.R * S.R;// (P-C)^2
	float	disc = b*b - a*c;							// discriminant of Q(t)

	if( disc < 0.f ){
		// 만나지 않는다.
		return false;
	}
	else if( 0.f < disc )
	{
		// 두개의 근을 가진다. -> Line과 두 점에서 만난다.
		if( pt0 ){
			float	sqrt_disc = sqrtf(disc);
			float	inv_a = 1.f/a;
			*pt0 = (-b - sqrt_disc)*inv_a;
			if( pt1 ){
				*pt1 = (-b + sqrt_disc)*inv_a;
			}
		}
		return true;
	}
	else{
		// 접한다. float 오차땜시 접하는 일은 거의 안 생기지 싶다.
		if( pt0 ){
			// Q'(t) = 2(a*t + b)
			// Q'(t) = 0, t = -b/a 에서 Q(t)가 최소
			*pt0 = -b/a;
		}
		return true;
	}
}

bool
FindIntersection( const Sphere & S, const Ray3 & R, float * pt0/*=NULL*/, float * pt1/*=NULL*/ )
{
	// set up quadratic Q(t) = a*t^2 + 2*b*t + c
	BSVECTOR	diff = R.P - S.C;						// P - C
	float	a = BsVec3Dot( &R.D, &R.D );				// D^2
	float	b = BsVec3Dot( &diff, &R.D );				// (P-C)*D
	float	c = BsVec3Dot( &diff, &diff ) - S.R * S.R;// (P-C)^2
	float	disc = b*b - a*c;							// discriminant of Q(t)

	if( disc < 0.f ){
		return false;
	}
	else if( 0.f < disc )
	{
		float	sqrt_disc = sqrtf(disc);
		float	inv_a = 1.f/a;
		float	t0 = (-b - sqrt_disc)*inv_a;
		float	t1 = (-b + sqrt_disc)*inv_a;

		if( 0.f <= t0 ){
			// 두 점 모두 교차
			if( pt0 )	*pt0 = t0;
			if( pt1 )	*pt1 = t1;
			return true;
		}
		else if( 0.f <= t1 ){
			// t1 한 점만 교차
			if( pt0 )	*pt0 = t1;	// *
			return true;
		}
		else{
			// 겹치는 두 점 모두 Ray 뒤쪽에 있다.
			return false;
		}
	}
	else{
		float	t0 = -b/a;
		if( 0.f <= t0 ){
			if( pt0 )	*pt0 = t0;
			return true;
		}
		else{
			return false;
		}
	}
}

bool
FindIntersection( const Sphere & S, const Segment3 & Seg, float * pt0/*=NULL*/, float * pt1/*=NULL*/ )
{
	// set up quadratic Q(t) = a*t^2 + 2*b*t + c
	BSVECTOR	diff = Seg.P - S.C;						// P - C
	float	a = BsVec3Dot( &Seg.D, &Seg.D );			// D^2
	float	b = BsVec3Dot( &diff, &Seg.D );			// (P-C)*D
	float	c = BsVec3Dot( &diff, &diff ) - S.R * S.R;// (P-C)^2
	float	disc = b*b - a*c;							// discriminant of Q(t)

	if( disc < 0.f ){
		return false;
	}
	else if( 0.f < disc )
	{
		float	sqrt_disc = sqrtf(disc);
		float	inv_a = 1.f/a;
		float	t0 = (-b - sqrt_disc)*inv_a;
		float	t1 = (-b + sqrt_disc)*inv_a;

		if( 1.f < t0 || t1 < 0.f ){
			// Segment 연장선에서 교차
			return false;
		}
		// t0 <= 1.f && 0.f <= t1
		else if( 0.f <= t0 ){
			if( 1.f < t1 ){
				// Segment가 Sphere를 파고 들어가는 모습.
				// 한 점에서 교차
				if( pt0 )	*pt0 = t0;
				if( pt1 )	*pt1 = t1;
				return true;
			}
			else{
				// Segment의 두 점 모두 교차
				if( pt0 )	*pt0 = t0;
				if( pt1 )	*pt1 = t1;
				return true;
			}
		}
		else{	// 0 <= t1
			// if( 1.f < t0 || t1 < 0.f )에서 두 점 모두 범위를 벗어나면 return하고
			// else if( 0.f <= t0 )에서 t0가 범위 안에 있는 경우를 처리했다.
			// 여기선 (t0 < 0)이므로 t1이 범위 안에 있는 경우이다.
			// Segment가 Sphere를 벗어나려는 모습.
			if( 1.f < t1 ){
				// Segment가 완전히 Sphere 내부에 있는 모습이다.
				return true;
			}
			else{
				// 벗어나려는 모습. 한 점(t1)에서 부딪힌다. (t0는 Segment 시작점 이전에 부딪힌 점)
				if( pt0 )	*pt0 = t1;	// *
				return true;
			}
		}
	}
	else{
		float	t0 = -b/a;
		if( 0.f <= t0 && t0 <= 1.f ){
			if( pt0 )	*pt0 = t0;
			return true;
		}
		else{
			return false;
		}
	}
}
