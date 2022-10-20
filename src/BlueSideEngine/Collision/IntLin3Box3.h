/*************************************************************************/
/** IntCapTri3.h

  * @author	Kim Jae Hyun	<neigex@phantagram.com>
  * @since	03-Dec-02	Created
**************************************************************************/
#ifndef	__INTERSECTION_LINE3_BOX3_H__
#define	__INTERSECTION_LINE3_BOX3_H__

struct	D3DXVECTOR3;
struct	Ray3;
struct	Segment3;
struct	AABB;
struct	Box3;

bool TestIntersection( const Ray3 & R, const AABB & B, float * pt );
bool TestIntersection( const Ray3 & R, const AABB & B );

bool TestIntersection( const Ray3 & R, const Box3 & B );
bool TestIntersection( const Segment3 & S, const Box3 & B );

bool TestIntersection( const D3DXVECTOR3 & V, const Box3 & B );
bool TestIntersection( const Box3& B1, const Box3& BaseBox );

bool GetIntersectPoint( D3DXVECTOR3 &vVec1, D3DXVECTOR3 &vVec2, D3DXVECTOR3 &vVec3, D3DXVECTOR3 &vVec4, D3DXVECTOR3 &vResult );

#ifdef _XBOX
BOOL IntersectRayTriangle( XMVECTOR Origin, XMVECTOR Direction, XMVECTOR V0, XMVECTOR V1, XMVECTOR V2, FLOAT* pDist );
#endif

#endif	// !__INTERSECTION_LINE3_BOX3_H__