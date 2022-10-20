#pragma once
//#include "MS_Cloth/ClothGPU/CClothGPUCharacter.h"
#include "BsAniObject.h"

class IBsClothGPU
{
public:
	virtual HRESULT Initialize( void* pCClothGPUCharacter ) = 0;
	virtual void Destroy() = 0;

	virtual void Update() = 0;
	virtual void Render() = 0;
	virtual void RenderShadow() = 0;
	virtual void SetCollisionPos(float fRadius, float fHeight, BSMATRIX Mat)  = 0;
	virtual void SetObjectMatrices( CBsAniObject* pCBsAniObject )	= 0;
	virtual bool IsReady() = 0;

public:
	static IBsClothGPU& GetInstance();
};

#define g_BsClothGPU IBsClothGPU::GetInstance()
