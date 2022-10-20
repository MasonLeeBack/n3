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
		// ������ �ʴ´�.
		return false;
	}
	else if( 0.f < disc )
	{
		// �ΰ��� ���� ������. -> Line�� �� ������ ������.
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
		// ���Ѵ�. float �������� ���ϴ� ���� ���� �� ������ �ʹ�.
		if( pt0 ){
			// Q'(t) = 2(a*t + b)
			// Q'(t) = 0, t = -b/a ���� Q(t)�� �ּ�
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
			// �� �� ��� ����
			if( pt0 )	*pt0 = t0;
			if( pt1 )	*pt1 = t1;
			return true;
		}
		else if( 0.f <= t1 ){
			// t1 �� ���� ����
			if( pt0 )	*pt0 = t1;	// *
			return true;
		}
		else{
			// ��ġ�� �� �� ��� Ray ���ʿ� �ִ�.
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
			// Segment ���弱���� ����
			return false;
		}
		// t0 <= 1.f && 0.f <= t1
		else if( 0.f <= t0 ){
			if( 1.f < t1 ){
				// Segment�� Sphere�� �İ� ���� ���.
				// �� ������ ����
				if( pt0 )	*pt0 = t0;
				if( pt1 )	*pt1 = t1;
				return true;
			}
			else{
				// Segment�� �� �� ��� ����
				if( pt0 )	*pt0 = t0;
				if( pt1 )	*pt1 = t1;
				return true;
			}
		}
		else{	// 0 <= t1
			// if( 1.f < t0 || t1 < 0.f )���� �� �� ��� ������ ����� return�ϰ�
			// else if( 0.f <= t0 )���� t0�� ���� �ȿ� �ִ� ��츦 ó���ߴ�.
			// ���⼱ (t0 < 0)�̹Ƿ� t1�� ���� �ȿ� �ִ� ����̴�.
			// Segment�� Sphere�� ������� ���.
			if( 1.f < t1 ){
				// Segment�� ������ Sphere ���ο� �ִ� ����̴�.
				return true;
			}
			else{
				// ������� ���. �� ��(t1)���� �ε�����. (t0�� Segment ������ ������ �ε��� ��)
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
