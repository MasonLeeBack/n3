//-------------------------------------------------------------------------------------------------
// [ X3DSoundCalculate.h ]
//-------------------------------------------------------------------------------------------------
#ifndef __X3DCALCULATE_H__
#define __X3DCALCULATE_H__

// image table
const float fX3DCalculateImageTable[] = 
{
	1.000f,
	0.700f,
	0.450f,
	0.250f,
	0.200f,
	0.180f,
	0.160f,
	0.140f,
	0.120f,
	0.100f,
	0.090f,
	0.080f,
	0.070f,
	0.060f,
	0.050f,
	0.040f,
	0.000f,
};

// length table
#if 0
// lin
const float fX3DCalculateLengthTable[] = 
{
	1.000f,
	0.938f,
	0.875f,
	0.813f,
	0.750f,
	0.688f,
	0.625f,
	0.563f,
	0.500f,
	0.438f,
	0.375f,
	0.313f,
	0.250f,
	0.188f,
	0.125f,
	0.063f,
	0.000f
};
#else
// lin^2
const float fX3DCalculateLengthTable[] = 
{
	1.000f,
	0.879f,
	0.766f,
	0.660f,
	0.563f,
	0.473f,
	0.391f,
	0.316f,
	0.250f,
	0.191f,
	0.141f,
	0.098f,
	0.063f,
	0.035f,
	0.016f,
	0.004f,
	0.000f
};
#endif

int X3DCalculate(D3DVECTOR pEpos, D3DVECTOR pLpos, D3DVECTOR pCvec, float* pfVolume, float fFieldLen);



#endif //__X3DCALCULATE_H__
