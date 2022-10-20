#pragma once

#define	PI		3.14159265358979323846f
#define	PI_2	1.57079632679489661923f
#define	SQRT2	1.41421356237309504880f
#define	SQRT1_2	0.707106781186547524401f
#define	NUM_SINTABLE	1024

class CBsSinTable
{
public:
	CBsSinTable();
	~CBsSinTable();

	float *m_pSinTable;
	float *m_pCosTable;
};

extern float *g_pSinTable;
extern float *g_pCosTable;;

#define FSIN(x) g_pSinTable[x]
#define FCOS(x) g_pCosTable[x] 