/*************************************************************************/
/** Frustum.h

  * @author	Kim Jae Hyun	<neigex@phantagram.com>
  * @since	05-Mar-03	Created
**************************************************************************/
#ifndef	__PRIMITIVE_FRUSTUM_H__
#define	__PRIMITIVE_FRUSTUM_H__

class C3DDevice;

/*************************************************************************/
/** Frustum
	The four corners of the frustum in the near plane are
	E + s0*l*L + s1*u*U + n*D where |s0| = |s1| = 1 (four choices).
	The four corners of the frustum in the far plane are
	E + (f/n)*(s0*l*L + s1*u*U) where |s0| = |s1| = 1 (four choices).
**************************************************************************/
class Frustum
{
public:
	Frustum(): l(1.f), u(1.f){}
	void compute_vertices();
	void update();
	void render( C3DDevice* pDevice, D3DCOLOR color=0x88ff7722 ) const;

	BSVECTOR	E;			// Origin
	BSVECTOR	L, U, D;	// Left, Up, Direction
	float		l, u;		// extents left, up
	float		n, f;		// near, far

	float		ratio, d[6];	// far/near, distance
	BSVECTOR	C, V[8], N[6];	// Center, Vertices, Normal
};

#endif	// !__PRIMITIVE_FRUSTUM_H__
