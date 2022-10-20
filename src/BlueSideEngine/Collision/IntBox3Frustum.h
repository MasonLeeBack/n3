/*************************************************************************/
/** IntBox3Frustum.h

  * @author	Kim Jae Hyun	<neigex@phantagram.com>
  * @since	05-Mar-03	Created
**************************************************************************/
#ifndef	__INTERSECTION_BOX3_FRUSTUM_H__
#define	__INTERSECTION_BOX3_FRUSTUM_H__

struct	Box3;
class   Sphere;
class	Frustum;

bool TestIntersection( const Box3 & B, const Frustum & F, float far_offset );
bool TestIntersection( const Box3 & B, const Frustum & F );

bool TestIntersection( const Sphere & S, const Box3 & BaseBox );

#endif	// !__INTERSECTION_BOX3_FRUSTUM_H__
