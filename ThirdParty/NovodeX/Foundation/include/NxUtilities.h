#ifndef NX_FOUNDATION_NXUTILITIES
#define NX_FOUNDATION_NXUTILITIES
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/
#include "Nxf.h"
#include <string.h>
#include "NxVec3.h"
#include "NxBounds3.h"

/**
 Utility calls that don't fit anywhere else.
*/

	NX_INLINE void NxFlexiCopy(const void* src, void* dst, NxU32 nbElem, NxU32 elemSize, NxU32 stride)
		{
		const NxU8* s = (const NxU8*)src;
		NxU8* d = (NxU8*)dst;
		while(nbElem--)
			{
			memcpy(d, s, elemSize);
			d += elemSize;
			s += stride;
			}
		}

	NX_INLINE NxU32 NxNextPowerOfTwo(NxU32 x)
		{
		x |= (x >> 1);
		x |= (x >> 2);
		x |= (x >> 4);
		x |= (x >> 8);
		x |= (x >> 16);
		return x+1;
		}

	/** Returns the angle between two (possibly un-normalized) vectors */
	NX_INLINE NxF32 NxAngle(const NxVec3& v0, const NxVec3& v1)
		{
		NxF32 cos = v0|v1;					// |v0|*|v1|*Cos(Angle)
		NxF32 sin = (v0^v1).magnitude();	// |v0|*|v1|*Sin(Angle)
		return NxMath::atan2(sin, cos);
		}

	NX_INLINE void NxMakeFatEdge(NxVec3& p0, NxVec3& p1, NxF32 fatCoeff)
		{
		NxVec3 delta = p1 - p0;
		delta.setMagnitude(fatCoeff);
		p0 -= delta;
		p1 += delta;
		}

	NX_INLINE void NxComputeNormalCompo(NxVec3& normalCompo, const NxVec3& outwardDir, const NxVec3& outwardNormal)
	{
		normalCompo = outwardNormal * (outwardDir|outwardNormal);
	}

	NX_INLINE void NxComputeTangentCompo(NxVec3& outwardDir, const NxVec3& outwardNormal)
	{
		outwardDir -= outwardNormal * (outwardDir|outwardNormal);
	}

	NX_INLINE void NxDecomposeVector(NxVec3& normalCompo, NxVec3& tangentCompo, const NxVec3& outwardDir, const NxVec3& outwardNormal)
		{
		normalCompo = outwardNormal * (outwardDir|outwardNormal);
		tangentCompo = outwardDir - normalCompo;
		}

	/**
	Computes a point on a triangle using barycentric coordinates. (It's only been extracted as a function
	so that there's no confusion regarding the order in which u and v should be used)
	*/
	NX_INLINE void NxComputeBarycentricPoint(NxVec3& pt, const NxVec3& p0, const NxVec3& p1, const NxVec3& p2, float u, float v)
		{
		// This seems to confuse the compiler...
//		pt = (1.0f - u - v)*p0 + u*p1 + v*p2;
		NxF32 w = 1.0f - u - v;
		pt.x = w*p0.x + u*p1.x + v*p2.x;
		pt.y = w*p0.y + u*p1.y + v*p2.y;
		pt.z = w*p0.z + u*p1.z + v*p2.z;
		}


#endif
