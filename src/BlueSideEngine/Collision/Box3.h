/*************************************************************************/
/** Box3.h

  * @since	03-Dec-02	Created
**************************************************************************/
#ifndef	__PRIMITIVE_BOX3_H__
#define	__PRIMITIVE_BOX3_H__

#include "BsMath.h"

#define _OPT_SHINJICH_BOX3	0

struct Box3
{
	void compute_vertices()
	{
		BSVECTOR	_A[3] = { E[0]*A[0], E[1]*A[1], E[2]*A[2] };

		V[0] = C - _A[0] - _A[1] - _A[2];	// 0008
		V[1] = C + _A[0] - _A[1] - _A[2];	// 001
		V[2] = C - _A[0] + _A[1] - _A[2];	// 010
		V[3] = C + _A[0] + _A[1] - _A[2];	// 011
		V[4] = C - _A[0] - _A[1] + _A[2];	// 100
		V[5] = C + _A[0] - _A[1] + _A[2];	// 101
		V[6] = C - _A[0] + _A[1] + _A[2];	// 110
		V[7] = C + _A[0] + _A[1] + _A[2];	// 111
	}
	void get_diagonal( BSVECTOR & Vmin, BSVECTOR & Vmax, const BSVECTOR & N ) const
	{
#if _OPT_SHINJICH_BOX3
		// change definision of BsVec3Dot() from XMVector3Dot().x to XMVector3Dot() simply !!
		// - shinjich
		int	nIdx;
		XMVECTOR vtemp[3];
		XMVECTOR vzero, vone, vtwo, vfour;
#if _DEBUG
		vzero.x = 0.0f;	// dummy for debug
#else
#pragma warning(disable:4700)
#endif
		vzero    = __vxor( vzero, vzero );	// create zero
		vfour    = __vnor( vzero, vzero );
		vone     = __vadduwm( vfour, vfour );
		vone     = __vxor( vone, vfour );	// create one
		vtwo     = __vadduwm( vone, vone );	// create two
		vfour    = __vadduwm( vtwo, vtwo );	// create four
		vtemp[0] = __vmsum3fp( N, A[0] );
		vtemp[1] = __vmsum3fp( N, A[1] );
		vtemp[2] = __vmsum3fp( N, A[2] );
		vtemp[0] = __vcmpgtfp( vtemp[0], vzero );
		vtemp[1] = __vcmpgtfp( vtemp[1], vzero );
		vtemp[2] = __vcmpgtfp( vtemp[2], vzero );
		vtemp[0] = __vand( vtemp[0], vone );
		vtemp[1] = __vand( vtemp[1], vtwo );
		vtemp[2] = __vand( vtemp[2], vfour );
		vtemp[0] = __vor( vtemp[0], vtemp[1] );
		vtemp[0] = __vor( vtemp[0], vtemp[2] );
		nIdx = vtemp[0].u[0];
#else
		int	nIdx=0;
		if( 0.f < BsVec3Dot( &N, &A[0] ) )	nIdx ^= 1;
		if( 0.f < BsVec3Dot( &N, &A[1] ) )	nIdx ^= 2;
		if( 0.f < BsVec3Dot( &N, &A[2] ) )	nIdx ^= 4;
#endif
		Vmax = V[nIdx];
		Vmin = V[7-nIdx];	// 7 is a Magic Number.
	}

	BSVECTOR	C;		// Center
	BSVECTOR	A[3];	// Axis
	float		E[3];	// Extent
	BSVECTOR	V[8];	// Vertices
};

struct AABB
{
	void Merge( BSVECTOR v) {
		if( Vmin.x > v.x ) Vmin.x = v.x;
		if( Vmin.y > v.y ) Vmin.y = v.y;
		if( Vmin.z > v.z ) Vmin.z = v.z;
		if( Vmax.x < v.x ) Vmax.x = v.x;
		if( Vmax.y < v.y ) Vmax.y = v.y;
		if( Vmax.z < v.z ) Vmax.z = v.z;
	}
	void Merge( AABB &aabb ) {
#ifdef _XBOX
		Vmin.x = std::min(Vmin.x, aabb.Vmin.x);
		Vmin.y = std::min(Vmin.y, aabb.Vmin.y);
		Vmin.z = std::min(Vmin.z, aabb.Vmin.z);
		Vmax.x = std::max(Vmax.x, aabb.Vmax.x);
		Vmax.y = std::max(Vmax.y, aabb.Vmax.y);
		Vmax.z = std::max(Vmax.z, aabb.Vmax.z);
#else
		Vmin.x = min(Vmin.x, aabb.Vmin.x);
		Vmin.y = min(Vmin.y, aabb.Vmin.y);
		Vmin.z = min(Vmin.z, aabb.Vmin.z);
		Vmax.x = max(Vmax.x, aabb.Vmax.x);
		Vmax.y = max(Vmax.y, aabb.Vmax.y);
		Vmax.z = max(Vmax.z, aabb.Vmax.z);
#endif
	}
	void Merge( Box3 &B ) {	// compjute_vertices µÈ°Í
		for(int i = 0; i < 8; i++) {
			Merge( B.V[i] );
		}
	}
	void Reset() {
		Vmin = BSVECTOR(FLT_MAX, FLT_MAX, FLT_MAX);
		Vmax = BSVECTOR(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	}
	AABB() {}
	AABB( BSVECTOR _Vmin, BSVECTOR _Vmax ): Vmin(_Vmin), Vmax(_Vmax){}
	BSVECTOR	Vmin;
	BSVECTOR	Vmax;

	BSVECTOR GetCenter()	{	return (Vmax+Vmin)*0.5f;	}
	BSVECTOR GetExtent()		{	return (Vmax-Vmin)*0.5f;	}
};

struct Grid7x7
{
	enum { eNx=7, eNz=7, };

	void compute_vertices(D3DXVECTOR3 *Vmin1, D3DXVECTOR3 *Vmax1, D3DXVECTOR3 *Vmin2, D3DXVECTOR3 *Vmax2, 
		                  D3DXVECTOR3 *Vmin3, D3DXVECTOR3 *Vmax3, D3DXVECTOR3 *Vmin4, D3DXVECTOR3 *Vmax4)
	{
		float ax1=(Vmax1->x - Vmin1->x)/(eNx-1);
		float ay1=(Vmax1->y - Vmin1->y)/(eNx-1);
		float az1=(Vmax1->z - Vmin1->z)/(eNx-1);

		float ax2=(Vmax2->x - Vmin2->x)/(eNx-1);
		float ay2=(Vmax2->y - Vmin2->y)/(eNx-1);
		float az2=(Vmax2->z - Vmin2->z)/(eNx-1);

		for (int i=0; i<eNx; i++) {
			V[      i] = D3DXVECTOR3(Vmin1->x + ax1*i, Vmin1->y + ay1*i, Vmin1->z + az1*i);
			V[eNx  +i] = D3DXVECTOR3(Vmin2->x + ax2*i, Vmin2->y + ay2*i, Vmin2->z + az2*i);
		}

		float ax3=(Vmax3->x - Vmin3->x)/(eNx-1);
		float ay3=(Vmax3->y - Vmin3->y)/(eNx-1);
		float az3=(Vmax3->z - Vmin3->z)/(eNx-1);

		float ax4=(Vmax4->x - Vmin4->x)/(eNx-1);
		float ay4=(Vmax4->y - Vmin4->y)/(eNx-1);
		float az4=(Vmax4->z - Vmin4->z)/(eNx-1);

		for (int i=0; i<eNx; i++) {
			V[eNx*2+i] = D3DXVECTOR3(Vmin3->x + ax3*i, Vmin3->y + ay3*i, Vmin3->z + az3*i);
			V[eNx*3+i] = D3DXVECTOR3(Vmin4->x + ax4*i, Vmin4->y + ay4*i, Vmin4->z + az4*i);
		}
	}

	D3DXVECTOR3	V[eNx*2 + eNz*2];	// Vertices
};

#endif	// !__PRIMITIVE_BOX3_H__