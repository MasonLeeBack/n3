/*************************************************************************/
/** IntLin3Box3.cpp

  * @author	Kim Jae Hyun	<neigex@phantagram.com>
  * @since	03-Dec-02	Created
**************************************************************************/
#include	"StdAfx.h"
#include	"xutility"		// std::swap, XBOX and Win32 are the same

#include	"Ray3.h"
#include	"Box3.h"

#include	"IntLin3Box3.h"

//! Integer representation of a floating-point value
#define	IR(x)				((unsigned int&)x)
#define	Line3Box3_epsilon	0.0001f


///////////////////////////////////////////////////////////////////////////
/*************************************************************************/
/** AABB

  * @todo	Pierre Terdiman 의 float 연산을 적게 하고 비교를 많이 하는 version이 있는데 아직 옮기지 않았다.
			그에 따르면 그의 Celeron에서 ~25% 더 빠르다고 하는데, code가 안 예뻐. ㅡㅡ
**************************************************************************/

//----------------------------------------------------------------------------------------------------
// Fast Ray-Box Intersection
//----------------------------------------------------------------------------------------------------
// Original code by Andrew Woo,
// from "Graphics Gems", Academic Press, 1990
// http://www.acm.org/tog/GraphicsGems/gems/RayBox.c
//----------------------------------------------------------------------------------------------------
// Optimized code by Pierre Terdiman, 2000 (~20-30% faster on his Celeron 500)
// http://codercorner.com/Pierre.htm
//----------------------------------------------------------------------------------------------------
// Epsilon value added by Klaus Hartmann. (discarding it saves a few cycles only)
//----------------------------------------------------------------------------------------------------
/*
bool TestIntersection( const Ray3 & R, const AABB & B, float * pt )
{
	bool	inside=true;

	float	MaxT[3]={-1.f,-1.f,-1.f};
	BSVECTOR	coord;

	// Find candidate planes.
	for( int i=0 ; i<3 ; ++i )
	{
		if( R.P[i] < B.Vmin[i] )
		{
			coord[i] = B.Vmin[i];
			inside = false;

			// calculate T distances to candidate planes
			// 여기서 거리(distance)는 시간(time-interval) 개념이다.
			// 곧, Ray가 Direction vector쪽으로 얼마나 Unit time만큼 가면 plane에 부딪히는지를 distance라고 한거
			if( IR(R.D[i]) )	MaxT[i] = (B.Vmin[i] - R.P[i]) / R.D[i];
		}
		else if( B.Vmax[i] < R.P[i]  )
		{
			coord[i] = B.Vmax[i];
			inside = false;

			// calculate T distances to candidate planes
			if( IR(R.D[i]) )	MaxT[i] = (B.Vmax[i] - R.P[i]) / R.D[i];
		}
	}

	// Ray origin inside bounding box
	if( inside )
	{
		//coord = R.P;
		if( pt ){
			*pt = 0.f;
		}
		return true;
	}

	// Get largest of the maxT's for final choice of intersection
	int	nPlane=0;
	if( MaxT[nPlane] < MaxT[1] )	nPlane=1;
	if( MaxT[nPlane] < MaxT[2] )	nPlane=2;

	// Check final candidate actually inside box
	// FPU compare 하지 않기 위함이다. Pierre Terdiman의 재치가 돋보이는데.
	// Andrew Woo는 단지 if (maxT[whichPlane] < 0.) 라고 double로 비교했다.
	if( IR(MaxT[nPlane]) & 0x80000000 ){
		return false;
	}

	for( i=0 ; i<3 ; ++i )
	{
		if( i != nPlane )
		{
			coord[i] = R.P[i] + MaxT[nPlane] * R.D[i];
#ifdef	Line3Box3_epsilon
			if( coord[i] < B.Vmin[i] - Line3Box3_epsilon  ||  B.Vmax[i] + Line3Box3_epsilon < coord[i] )	return false;
#else
			if( coord[i] < B.Vmin[i]  ||  B.Vmax[i] < coord[i] )	return false;
#endif
		}
	}

	if( pt ){
		*pt = MaxT[nPlane];
	}
	return true;	// ray hits box
}
*/

///////////////////////////////////////////////////////////////////////////
/*************************************************************************/
/** AABB
**************************************************************************/
bool
TestIntersection( const Ray3 & R, const AABB & B )
{
	D3DXVECTOR3	h;	// half of AABB
	D3DXVECTOR3	c;	// center of Segment
	//D3DXVECTOR3	w;	// half vector of Segment's directional vector
	D3DXVECTOR3	v;	// v_x = |w_x|

	h.x = 0.5f * (B.Vmax.x - B.Vmin.x);
	c.x = R.P.x - B.Vmin.x + h.x;
	if( h.x < fabsf(c.x)  &&  0.f <= c.x*R.D.x )	return false;

	h.y = 0.5f * (B.Vmax.y - B.Vmin.y);
	c.y = R.P.y - B.Vmin.y + h.y;
	if( h.y < fabsf(c.y)  &&  0.f <= c.y*R.D.y )	return false;

	h.z = 0.5f * (B.Vmax.z - B.Vmin.z);
	c.z = R.P.z - B.Vmin.z + h.z;
	if( h.z < fabsf(c.z)  &&  0.f <= c.z*R.D.z )	return false;

	v = D3DXVECTOR3( fabsf(R.D.x), fabsf(R.D.y), fabsf(R.D.z) );

	float	c_a;
	c_a = c.y*R.D.z - c.z*R.D.y;	if( h.y*v.z + h.z*v.y < fabsf(c_a) )	return false;
	c_a = c.x*R.D.z - c.z*R.D.x;	if( h.x*v.z + h.z*v.x < fabsf(c_a) )	return false;
	c_a = c.x*R.D.y - c.y*R.D.x;	if( h.x*v.y + h.y*v.x < fabsf(c_a) )	return false;

	return true;
}

///////////////////////////////////////////////////////////////////////////
/*************************************************************************/
/** Box3
**************************************************************************/

bool TestIntersection( const Segment3 & S, const Box3 & B )
{
	float fAWdU[3], fADdU[3], fAWxDdU[3], fRhs;
	BSVECTOR kSDir = 0.5f*S.D;
	BSVECTOR kSCen = S.P + kSDir;

	BSVECTOR kDiff = kSCen - B.C;

	fAWdU[0] = (float)fabs((float)( BsVec3Dot( &kSDir, &B.A[0] )));
	fADdU[0] = (float)fabs((float)( BsVec3Dot( &kDiff, &B.A[0] )));
	fRhs = B.E[0] + fAWdU[0];
	if ( fADdU[0] > fRhs )
		return false;

	fAWdU[1] = (float)fabs((float)( BsVec3Dot( &kSDir, &B.A[1] )));
	fADdU[1] = (float)fabs((float)( BsVec3Dot( &kDiff, &B.A[1] )));
	fRhs = B.E[1] + fAWdU[1];
	if ( fADdU[1] > fRhs )
		return false;

	fAWdU[2] = (float)fabs((float)( BsVec3Dot( &kSDir, &B.A[2] )));
	fADdU[2] = (float)fabs((float)( BsVec3Dot( &kDiff, &B.A[2] )));
	fRhs = B.E[2] + fAWdU[2];
	if ( fADdU[2] > fRhs )
		return false;

	BSVECTOR kWxD;
	BsVec3Cross( &kWxD, &kSDir, &kDiff );

	fAWxDdU[0] = (float)fabs((float)( BsVec3Dot( &kWxD, &B.A[0] )));
	fRhs = B.E[1]*fAWdU[2] + B.E[2]*fAWdU[1];
	if ( fAWxDdU[0] > fRhs )
		return false;

	fAWxDdU[1] = (float)fabs((float)( BsVec3Dot( &kWxD, &B.A[1] )));
	fRhs = B.E[0]*fAWdU[2] + B.E[2]*fAWdU[0];
	if ( fAWxDdU[1] > fRhs )
		return false;

	fAWxDdU[2] = (float)fabs((float)( BsVec3Dot( &kWxD, &B.A[2] )));
	fRhs = B.E[0]*fAWdU[1] + B.E[1]*fAWdU[0];
	if ( fAWxDdU[2] > fRhs )
		return false;

	return true;
}

bool TestIntersection( const Ray3 & R, const Box3 & B )
{
	float fWdU[3], fAWdU[3], fDdU[3], fADdU[3], fAWxDdU[3], fRhs;

	BSVECTOR kDiff = R.P - B.C;

	fWdU[0] =  BsVec3Dot( &R.D, &B.A[0]);
	fAWdU[0] = (float)fabs((float)(fWdU[0]));
	fDdU[0] =  BsVec3Dot( &kDiff, &B.A[0]);
	fADdU[0] = (float)fabs((float)(fDdU[0]));
	if ( fADdU[0] > B.E[0] && fDdU[0]*fWdU[0] >= 0.0f )
		return false;

	fWdU[1] =  BsVec3Dot( &R.D, &B.A[1]);
	fAWdU[1] = (float)fabs((float)(fWdU[1]));
	fDdU[1] =  BsVec3Dot( &kDiff, &B.A[1]);
	fADdU[1] = (float)fabs((float)(fDdU[1]));
	if ( fADdU[1] > B.E[1] && fDdU[1]*fWdU[1] >= 0.0f )
		return false;

	fWdU[2] =  BsVec3Dot( &R.D, &B.A[2]);
	fAWdU[2] = (float)fabs((float)(fWdU[2]));
	fDdU[2] =  BsVec3Dot( &kDiff, &B.A[2]);
	fADdU[2] = (float)fabs((float)(fDdU[2]));
	if ( fADdU[2] > B.E[2] && fDdU[2]*fWdU[2] >= 0.0f )
		return false;

	BSVECTOR kWxD;
	BsVec3Cross( &kWxD, &R.D, &kDiff );

	fAWxDdU[0] = (float)fabs((float)( BsVec3Dot( &kWxD, &B.A[0] )));
	fRhs = B.E[1]*fAWdU[2] + B.E[2]*fAWdU[1];
	if ( fAWxDdU[0] > fRhs )
		return false;

	fAWxDdU[1] = (float)fabs((float)( BsVec3Dot( &kWxD, &B.A[1] )));
	fRhs = B.E[0]*fAWdU[2] + B.E[2]*fAWdU[0];
	if ( fAWxDdU[1] > fRhs )
		return false;

	fAWxDdU[2] = (float)fabs((float)( BsVec3Dot( &kWxD, &B.A[2] )));
	fRhs = B.E[0]*fAWdU[1] + B.E[1]*fAWdU[0];
	if ( fAWxDdU[2] > fRhs )
		return false;

	return true;
}

bool TestIntersection ( const D3DXVECTOR3 & V, const Box3 & B )
{
	BSVECTOR	P = *(BSVECTOR*)&V - B.C;
	if( B.E[0] < fabsf(BsVec3Dot( &P, &B.A[0] )) )	return false;
	if( B.E[1] < fabsf(BsVec3Dot( &P, &B.A[1] )) )	return false;
	if( B.E[2] < fabsf(BsVec3Dot( &P, &B.A[2] )) )	return false;
	return true;
}

bool TestIntersection( const Box3& B1, const Box3& BaseBox )
{
	BSVECTOR Vmin, Vmax;
	float fExtentDiagonal[3];

	B1.get_diagonal(Vmin, Vmax, BaseBox.A[0]);
	fExtentDiagonal[0] = fabsf(BsVec3Dot(&((Vmax - Vmin)*0.5f), &(BaseBox.A[0]) ));

	B1.get_diagonal(Vmin, Vmax, BaseBox.A[1]);
	fExtentDiagonal[1] = fabsf(BsVec3Dot(&((Vmax - Vmin)*0.5f), &(BaseBox.A[1]) ));
	
	B1.get_diagonal(Vmin, Vmax, BaseBox.A[2]);
	fExtentDiagonal[2] = fabsf(BsVec3Dot(&((Vmax - Vmin)*0.5f), &(BaseBox.A[2]) ));


	BSVECTOR	P = B1.C - BaseBox.C;
	if( BaseBox.E[0] + fExtentDiagonal[0] < fabsf(BsVec3Dot( &P, &BaseBox.A[0] )) )	return false;
	if( BaseBox.E[1] + fExtentDiagonal[1] < fabsf(BsVec3Dot( &P, &BaseBox.A[1] )) )	return false;
	if( BaseBox.E[2] + fExtentDiagonal[2] < fabsf(BsVec3Dot( &P, &BaseBox.A[2] )) )	return false;
	return true;
}

bool GetIntersectPoint( D3DXVECTOR3 &vVec1, D3DXVECTOR3 &vVec2, D3DXVECTOR3 &vVec3, D3DXVECTOR3 &vVec4, D3DXVECTOR3 &vResult ) 
{
	float t, s;
	float under = ( vVec4.z - vVec3.z ) * ( vVec2.x - vVec1.x ) - ( vVec4.x - vVec3.x ) * ( vVec2.z - vVec1.z );
	if( under == 0.f ) return false;

	float _t = ( vVec4.x - vVec3.x ) * ( vVec1.z - vVec3.z ) - ( vVec4.z - vVec3.z ) * ( vVec1.x - vVec3.x );
	float _s = ( vVec2.x - vVec1.x ) * ( vVec1.z - vVec3.z ) - ( vVec2.z - vVec1.z ) * ( vVec1.x - vVec3.x );

	t = _t / under;
	s = _s / under;
	if( t < 0.f || t > 1.f || s < 0.f || s > 1.f ) return false;
	if( _t == 0.f && _s == 0.f ) return false;
	vResult.x = vVec1.x + t * ( vVec2.x - vVec1.x );
	vResult.z = vVec1.z + t * ( vVec2.z - vVec1.z );
	return true;
}



#ifdef _XBOX
static const XMVECTOR g_VectorZero = { 0.0f, 0.0f, 0.0f, 0.0f };
//-----------------------------------------------------------------------------
// Compute the intersecion of a ray (Origin, Direction) with a triangle 
// (V0, V1, V2).  Return TRUE if there is an interseciton and also set *pDist 
// to the distance along the ray to the intersection.
// 
// The algorithm is based on Moller, Tomas and Trumbore, "Fast, Minimum Storage 
// Ray-Triangle Intersection", Journal of Graphics Tools, vol. 2, no. 1, 
// pp 21-28, 1997.
//-----------------------------------------------------------------------------
BOOL IntersectRayTriangle( XMVECTOR Origin, XMVECTOR Direction, XMVECTOR V0, XMVECTOR V1, XMVECTOR V2, FLOAT* pDist )
{
	static const XMVECTOR Epsilon = { 1e-20f, 1e-20f, 1e-20f, 1e-20f };

	XMVECTOR e1 = V1 - V0;
	XMVECTOR e2 = V2 - V0;

	// p = Direction ^ e2;
	XMVECTOR p = XMVector3Cross( Direction, e2 );

	// det = e1 * p;
	XMVECTOR det = XMVector3Dot( e1, p );

	XMVECTOR u, v, t;

	if ( XMVector3GreaterOrEqual( det, Epsilon ) )
	{
		// Determinate is positive (front side of the triangle).
		XMVECTOR s = Origin - V0;

		// u = s * p;
		u = XMVector3Dot( s, p );

		if ( XMVector3Less( u, g_VectorZero ) || XMVector3Greater( u, det ) )
			return FALSE;

		// q = s ^ e1;
		XMVECTOR q = XMVector3Cross( s, e1 );

		// v = Direction * q;
		v = XMVector3Dot( Direction, q );

		if ( XMVector3Less( v, g_VectorZero ) || XMVector3Greater( u + v, det ) )
			return FALSE;

		// t = e2 * q;
		t = XMVector3Dot( e2, q );

		if ( XMVector3Less( t, g_VectorZero ) )
			return FALSE;
	}
	else if ( XMVector3LessOrEqual( det, -Epsilon ) )
	{
		// Determinate is negative (back side of the triangle).
		XMVECTOR s = Origin - V0;

		// u = s * p;
		u = XMVector3Dot( s, p );

		if ( XMVector3Greater( u, g_VectorZero ) || XMVector3Less( u, det ) )
			return FALSE;

		// q = s ^ e1;
		XMVECTOR q = XMVector3Cross( s, e1 );

		// v = Direction * q;
		v = XMVector3Dot( Direction, q );

		if ( XMVector3Greater( v, g_VectorZero ) || XMVector3Less( u + v, det ) )
			return FALSE;

		// t = e2 * q;
		t = XMVector3Dot( e2, q );

		if ( XMVector3Greater( t, g_VectorZero ) )
			return FALSE;
	}
	else
	{
		// Parallel ray.
		return FALSE;
	}

	XMVECTOR inv_det = XMVectorReciprocal( det );

	t *= inv_det;

	// u * inv_det and v * inv_det are the barycentric cooridinates of the intersection.

	*pDist = t.x;

	return TRUE;
}
#endif