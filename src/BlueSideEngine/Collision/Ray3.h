/*************************************************************************/
/** Ray3.h

  * @since	11-Oct-02	Created
**************************************************************************/
#ifndef	__PRIMITIVE_RAY3_H__
#define	__PRIMITIVE_RAY3_H__

/*************************************************************************/
/** L(t) = P + t*D, t e {R}
**************************************************************************/
struct Line3
{
	Line3(){}
	Line3( BSVECTOR _P, BSVECTOR _D ): P(_P), D(_D){}
	BSVECTOR	P;	// a Point in Line
	BSVECTOR	D;	// Direction, which may be not a Unit Vector.
};

/*************************************************************************/
/** R(t) = P + t*D, for 0 <= t.
**************************************************************************/
struct Ray3
{
	Ray3(){}
	Ray3( BSVECTOR _P, BSVECTOR _D ): P(_P), D(_D){}
	BSVECTOR	P;	// a Point in Line
	BSVECTOR	D;	// Direction, which may be not a Unit Vector.
};

/*************************************************************************/
/** S(t) = P + t *D, t e [ 0, 1]					...(1)
	S(t) = C + t'*U, t'e [-r, r]					...(2)

  * The first formula(1) is implemented.
**************************************************************************/
struct Segment3
{
	Segment3(){}
	Segment3( BSVECTOR _P, BSVECTOR _D ): P(_P), D(_D){}

	/*
	D3DXVECTOR3	C;	// Center
	D3DXVECTOR3	U;	// Directional Unit Vector
	float		r;	// the half magnitude of a Segment's Length
	D3DXVECTOR3	rU;	// r * U; for an efficient work
	*/

	BSVECTOR	P;	// a Point in Line
	BSVECTOR	D;	// Direction, which may be not a Unit Vector.
};

#endif	// !__PRIMITIVE_RAY3_H__
