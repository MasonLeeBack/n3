/*************************************************************************/
/** Sphere.h

  * @since	14-Oct-02	Created
**************************************************************************/
#ifndef	__PRIMITIVE_SPHERE3_H__
#define	__PRIMITIVE_SPHERE3_H__

class Sphere
{
public:
	Sphere(): R(0.f){}
	Sphere( BSVECTOR _C, float _R ): C(_C), R(_R){}

	inline void operator=(const Sphere & S)
	{
		C = S.C;
		R = S.R;
	}

	BSVECTOR		C;		// Center
	float			R;		// Radius
};

#endif	// !__PRIMITIVE_SPHERE3_H__
