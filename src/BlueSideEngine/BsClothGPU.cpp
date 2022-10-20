#include "stdafx.h"
#include "BsKernel.h"
#include "BsClothGPU.h"
#ifdef _XBOX
#include "MS_Cloth/ClothGPU/CClothGPUCharacter.h"

class CBsClothGPU: public IBsClothGPU
{
protected:
	bool							m_bReady;
	CClothGPUCharacter*				m_pCClothGPUCharacter;

	//BSMATRIX						m_InitialMat;
	//BSMATRIX						m_InvInitialMat;
	//BSMATRIX						m_TransMat;
	//BSMATRIX						m_TransRootMat;

	//float							m_fTime;
	//float							m_fElapsedTime;
	//float							m_fFirstTime;

	//PHYSICS_COLLISION_CONTAINER*	m_pCollisionMesh;
	//std::vector< SClothGPUCollisionSphereTbl >		m_CollisionSpheres[ DOUBLE_BUFFERING ];
	//std::vector< SClothGPUCollisionEllipsoidTbl >	m_CollisionEllipsoids[ DOUBLE_BUFFERING ];

public:
	bool IsReady()
	{
		return	m_bReady;
	}

	CBsClothGPU()
	{
		m_bReady				= false;
		m_pCClothGPUCharacter	= NULL;
	}	

	HRESULT Initialize( void* pCClothGPUCharacter )
	{
		m_pCClothGPUCharacter	= ( CClothGPUCharacter* )pCClothGPUCharacter;

		m_bReady = true;

		return S_OK;
	}

	void Destroy()
	{
		m_pCClothGPUCharacter->Release();
	}

	void Update()
	{
//		g_BsKernel.BeginPIXEvent(255, 0, 0, "GPU Cloth Update");

		if( !m_bReady )
			return;
		/*
		{
			// Sphere
			int nRenderIndex = CBsObject::GetRenderBufferIndex();

			if( m_CollisionSpheres[nRenderIndex].empty()) {
				m_pCClothGPUCollisionSphere->SetCollisionSpheres( 0, NULL );
				m_pCClothGPUL->SetCollisionSpheres( 0, NULL );
				m_pCClothGPUR->SetCollisionSpheres( 0, NULL );
			}
			else {
				m_pCClothGPUCollisionSphere->SetCollisionSpheres( m_CollisionSpheres[nRenderIndex].size(),
																  &m_CollisionSpheres[nRenderIndex].front() );
				m_pCClothGPUCollisionSphere->GenerateTexture();
				m_pCClothGPUL->SetCollisionSpheres( m_pCClothGPUCollisionSphere->GetNbCollisionSpheres(),
													m_pCClothGPUCollisionSphere->GetTexture() );
				m_pCClothGPUR->SetCollisionSpheres( m_pCClothGPUCollisionSphere->GetNbCollisionSpheres(),
													m_pCClothGPUCollisionSphere->GetTexture() );
			}

			// Ellipsoid
			if( m_CollisionEllipsoids[nRenderIndex].empty() ) {
				m_pCClothGPUCollisionEllipsoid->SetCollisionEllipsoids( 0, NULL );
				m_pCClothGPUL->SetCollisionEllipsoids( 0, NULL );
				m_pCClothGPUR->SetCollisionEllipsoids( 0, NULL );
			}
			else {			
				m_pCClothGPUCollisionEllipsoid->SetCollisionEllipsoids( m_CollisionEllipsoids[nRenderIndex].size(),
																		&m_CollisionEllipsoids[nRenderIndex].front() );
				m_pCClothGPUCollisionEllipsoid->GenerateTexture();
				m_pCClothGPUL->SetCollisionEllipsoids( m_pCClothGPUCollisionEllipsoid->GetNbCollisionEllipsoids(),
													   m_pCClothGPUCollisionEllipsoid->GetTexture() );
				m_pCClothGPUR->SetCollisionEllipsoids( m_pCClothGPUCollisionEllipsoid->GetNbCollisionEllipsoids(),
													   m_pCClothGPUCollisionEllipsoid->GetTexture() );
			}

			m_CollisionSpheres[ nRenderIndex ].clear();
			m_CollisionEllipsoids[ nRenderIndex ].clear();
		}
		*/

		m_pCClothGPUCharacter->Update();

//		g_BsKernel.EndPIXEvent();
	}

	
	void SetCollisionPos(float fRadius, float fHeight, BSMATRIX Mat) 
	{
		if( !m_bReady )
			return;
		/*
		int nIndex = CBsObject::GetProcessBufferIndex();
		if( fHeight == 0.f) {
			BSMATRIX matTrans;
			BsMatrixMultiply(&matTrans, &Mat, &m_InvInitialMat);
			SClothGPUCollisionSphereTbl sphere;
			sphere.pBone = NULL;
			sphere.fRadius = fRadius;
			sphere.vCenter.x = matTrans._41;
			sphere.vCenter.y = matTrans._42;
			sphere.vCenter.z = matTrans._43;
			m_CollisionSpheres[nIndex].push_back(sphere);
		}
		else {

			BSMATRIX matTrans;
			BsMatrixMultiply(&matTrans, &Mat, &m_InvInitialMat);

			SClothGPUCollisionEllipsoidTbl ellipsoid;
			ellipsoid.pBone = new XMMATRIX((FLOAT*)&matTrans);			
			ellipsoid.pBone->_41 = 0.f;
			ellipsoid.pBone->_42 = 0.f;
			ellipsoid.pBone->_43 = 0.f;
			ellipsoid.vCenter.x = matTrans._41;
			ellipsoid.vCenter.y = matTrans._42;//Mat._42 - m_InitialMat._42;
			ellipsoid.vCenter.z = matTrans._43;//Mat._43 - m_InitialMat._43;
			ellipsoid.vScale.x = fRadius;
			ellipsoid.vScale.y = fHeight*0.5f;
			ellipsoid.vScale.z = fRadius;
			m_CollisionEllipsoids[nIndex].push_back(ellipsoid);
		}
		*/
		
		m_pCClothGPUCharacter->SetObjectCollision( fRadius, fHeight, Mat );
	}

	void	SetObjectMatrices( CBsAniObject* pCBsAniObject )
	{
		if( !m_bReady )
			return;

		m_pCClothGPUCharacter->SetObjectMatrices( pCBsAniObject );
	}

	void Render()
	{
//		g_BsKernel.BeginPIXEvent(255, 0, 0, "GPU Cloth Render");

		if( !m_bReady )
			return;

		m_pCClothGPUCharacter->Render();

//		g_BsKernel.EndPIXEvent();
	}

	void RenderShadow()
	{
		if( !m_bReady )
			return;
	}
};

static CBsClothGPU	s_ClothGPU;

#endif

class CBsNullClothGPU : public IBsClothGPU
{
public:
	HRESULT Initialize( void* pCClothGPUCharacter ) {return S_FALSE;}
	void Destroy() {}
	void Update() {}
	void Render() {}
	void RenderShadow() {}
	void SetCollisionPos(float fRadius, float fHeight, BSMATRIX Mat) {}	
	void SetObjectMatrices( CBsAniObject* pCBsAniObject )	{}
	bool IsReady() {return false;}
};

static CBsNullClothGPU s_NullClothGPU;

IBsClothGPU& IBsClothGPU::GetInstance()
{
#ifdef _XBOX
	return s_ClothGPU;
#else
	return s_NullClothGPU;
#endif
}
