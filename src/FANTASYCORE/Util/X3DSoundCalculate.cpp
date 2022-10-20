//-------------------------------------------------------------------------------------------------
// [ X3DSoundCalculate.cpp ]
//-------------------------------------------------------------------------------------------------
#include "stdafx.h"

#ifdef _XBOX
#include "X3DSoundCalculate.h"

// local variable
static const int f3DImageTableCount  = (sizeof(fX3DCalculateImageTable) / sizeof(fX3DCalculateImageTable[0]));
static const int f3DLengthTableCount = (sizeof(fX3DCalculateLengthTable) / sizeof(fX3DCalculateLengthTable[0]));

// pos -> rate
static float fPos2Rate(float fPos, float fFeildLen, float* pTable, int nCount)
{
	float fPosRate = fPos / fFeildLen;
	
	int nTableOfs0 = (int)floor(fPosRate * (nCount - 1));
	int nTableOfs1 = nTableOfs0 + 1;
	
	float fTableDifferenceValue = pTable[nTableOfs1] - pTable[nTableOfs0];
	float fTableComplementRate  = (fPosRate * (nCount - 1)) - (float)nTableOfs0;
	
	return pTable[nTableOfs0] + (fTableDifferenceValue * fTableComplementRate);
}


//-------------------------------------------------------------------------------------------------
// 3D calculation
//-------------------------------------------------------------------------------------------------
int X3DCalculate(D3DVECTOR pEpos, D3DVECTOR pLpos, D3DVECTOR pCvec, float* pfVolume, float fFieldLen)
{
	XMMATRIX view;
	XMVECTOR eye;
	XMVECTOR foc;
	XMVECTOR dir;
	
	if((pLpos.x == pEpos.x) && (pEpos.x == pEpos.z))
	{
		pEpos.z = pLpos.z + 0.01f;
	}
	
	eye.x = 0.f; 
	eye.y = 0.f; 
	eye.z = 0.f;		// (0, 0, 0)
	
	foc.x = -pCvec.x; 
	foc.y = 0.f; 
	foc.z = pCvec.z;	// focus vector
	
	dir.x = 0.f; 
	dir.y = 1.f; 
	dir.z = 0.f;
	
	view = XMMatrixLookAtLH(eye, foc, dir);	// ‰ñ?s—ñ
	
	XMVECTOR v1;
	v1.x = pEpos.x - pLpos.x;
	v1.y = pEpos.y - pLpos.y;
	v1.z = pEpos.z - pLpos.z;
	
	XMVECTOR dst;
	dst.x = view._11 * v1.x + view._12 * v1.y + view._13 * v1.z; 
	dst.y = view._21 * v1.x + view._22 * v1.y + view._23 * v1.z; 
	dst.z = view._31 * v1.x + view._32 * v1.y + view._33 * v1.z; 
	
	pEpos.x = pLpos.x + dst.x;
	pEpos.y = pLpos.y + dst.y;
	pEpos.z = pLpos.z + dst.z;
	
	float fFL, fFR, fRL, fRR, fFC, fSW;
	
	// image pos
	float fdx = fabs(pEpos.x - pLpos.x);
	float fdz = fabs(pEpos.z - pLpos.z);
	// limitter
	if(fdx > fFieldLen) fdx = fFieldLen;
	if(fdz > fFieldLen) fdz = fFieldLen;
	
	// image rate
	float fXscaleRate = fPos2Rate(fdx, fFieldLen, (float*)&fX3DCalculateImageTable, f3DImageTableCount);
	float fZscaleRate = fPos2Rate(fdz, fFieldLen, (float*)&fX3DCalculateImageTable, f3DImageTableCount);
	
	//--- 3D field rate
	if(pEpos.x >= pLpos.x)
	{
		if(pEpos.z >= pLpos.z)	// 1
		{
			fFL = fXscaleRate;
			fFR = 1.0f;
			fRL = fFL * fZscaleRate;
			fRR = fFR * fZscaleRate;
			
			fFC = ((fFL + fFR) / 2) * 0.7f;
			fSW = 1.f;
		}
		else	// 4
		{
			fRL = fXscaleRate;
			fRR = 1.0f;
			fFL = fRL * fZscaleRate;
			fFR = fRR * fZscaleRate;
			
			fFC = ((fFL + fFR) / 2) * 0.7f;
			fSW = 1.f;
		}
	}
	else
	{
		if(pEpos.z >= pLpos.z)	// 2
		{
			fFL = 1.0f;	
			fFR = fXscaleRate;
			fRL = fFL * fZscaleRate;
			fRR = fFR * fZscaleRate;
			
			fFC = ((fFL + fFR) / 2) * 0.7f;
			fSW = 1.f;
		}
		else	// 3
		{
			fRL = 1.0f;
			fRR = fXscaleRate;
			fFL = fRL * fZscaleRate;
			fFR = fRR * fZscaleRate;
			
			fFC = ((fFL + fFR) / 2) * 0.7f;
			fSW = 1.f;
		}
	}
	
	float fDiagonal = fFieldLen * 1.414f;
	
	D3DXVECTOR3 vLE(pEpos.x - pLpos.x, pEpos.y - pLpos.y, pEpos.z - pLpos.z);
	float fLELen = D3DXVec3Length(&vLE);
	
	if(fLELen > fDiagonal) fLELen = fDiagonal;
	
	// length rate
	float fRate = fPos2Rate(fLELen, fDiagonal, (float*)&fX3DCalculateLengthTable, f3DLengthTableCount);
	
	pfVolume[0] = fFL * fRate;	// FL
	pfVolume[1] = fFR * fRate;	// FR
	pfVolume[2] = fFC * fRate;	// FC
	pfVolume[3] = fSW * fRate;	// SW
	pfVolume[4] = fRL * fRate;	// RL
	pfVolume[5] = fRR * fRate;	// RR
	//char sz[256]; wsprintf(sz, "rate : %f \n", fRate);
	//OutputDebugString(sz);
	return 0;
}
#endif //_XBOX