/*************************************************************************/
/** IntSphLin3.h

  * @author	Kim Jae Hyun	<neigex@phantagram.com>
  * @since	14-Oct-02	Created
**************************************************************************/
#ifndef	__INTERSECTION_SPHERE_LINE3_H__
#define	__INTERSECTION_SPHERE_LINE3_H__

class	Sphere;
struct	Line3;
struct	Ray3;
struct	Segment3;

bool	TestIntersection( const Sphere & S, const Line3 & L );
bool	TestIntersection( const Sphere & S, const Ray3 & R );
bool	TestIntersection( const Sphere & S, const Segment3 & Seg );

bool	FindIntersection( const Sphere & S, const Line3 & L, float * pt0/*=NULL*/, float * pt1/*=NULL*/ );
bool	FindIntersection( const Sphere & S, const Ray3 & R, float * pt0/*=NULL*/, float * pt1/*=NULL*/ );
bool	FindIntersection( const Sphere & S, const Segment3 & Seg, float * pt0/*=NULL*/, float * pt1/*=NULL*/ );

#endif	// !__INTERSECTION_SPHERE_LINE3_H__
