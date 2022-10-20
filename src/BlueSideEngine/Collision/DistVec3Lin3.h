/*************************************************************************/
/** DistVec3Lin3.h

  * @author	Kim Jae Hyun	<neigex@phantagram.com>
  * @since	08-Oct-02	Created
**************************************************************************/
#ifndef	__DISTANCE_VECTOR3_LINE3_H__
#define	__DISTANCE_VECTOR3_LINE3_H__

struct	Line3;
struct	Ray3;
struct	Segment3;

float	SqrDistance( const BSVECTOR & P, const Line3 & L, float * pt=NULL );
float	SqrDistance( const BSVECTOR & P, const Ray3 & R, float * pt=NULL );
float	SqrDistance( const BSVECTOR & P, const Segment3 & S, float * pt=NULL );

#endif	// !__DISTANCE_VECTOR3_LINE3_H__
