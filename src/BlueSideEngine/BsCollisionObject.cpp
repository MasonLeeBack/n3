#include "stdafx.h"
/*
#include "BsKernel.h"
#include "CrossVector.h"
#include "BsMesh.h"
#include "BsCollisionObject.h"
#include "BsPhysicsMgr.h"


CBsCollisionObject::CBsCollisionObject(float radius, float height, D3DXMATRIX *matLocal)
{
	CCrossVector cross;
    
	// юс╫ц
	g_BsKernel.GetPhysicsMgr()->SetActorResponse(true);

	if( height== 0.0f ) {
		m_pActor = CBsPhysicsSphere::Create(radius, *cross);
	}
	else {
		m_pActor = CBsPhysicsCapsule::Create(radius, height, *cross);
	}

	g_BsKernel.GetPhysicsMgr()->SetActorResponse(false);

	m_pActor->SetKinematic();
	m_pActor->SetGroup("CollisionObject");

	m_matLocal = *matLocal;	
}

CBsCollisionObject::~CBsCollisionObject()
{
	if( m_pActor ) {
		delete m_pActor;
		m_pActor = NULL;
	}
}


void CBsCollisionObject::InitRender(float fDistanceFromCamera)
{	
	if( fDistanceFromCamera != 0.f ) return;

	D3DXMATRIX matWorld;
	
	D3DXMatrixMultiply(&matWorld, &m_matLocal, &m_matObject);	

	if(m_pActor) {
		m_pActor->SetGlobalPose( matWorld );
	}

}
*/