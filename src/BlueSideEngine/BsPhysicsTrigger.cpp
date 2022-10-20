#include "stdafx.h"
#include "BsPhysicsTrigger.h"
#include "BsPhysicsMgr.h"


CBsPhysicsTriggerBase::CBsPhysicsTriggerBase()
{
	m_pTriggerActor = NULL;
}

CBsPhysicsTriggerBase::~CBsPhysicsTriggerBase()
{
    ReleaseTriggerActor();
}

void CBsPhysicsTriggerBase::CreateTriggerBox( D3DXVECTOR3 Size, D3DXVECTOR3 Pivot, D3DXMATRIX matObj )
{
	BsAssert( m_pTriggerActor == NULL);
	m_pTriggerActor = CBsPhysicsBoxTrigger::Create(Size, Pivot, matObj);
	m_pTriggerActor->SetShapeUserData( this );	
}

void CBsPhysicsTriggerBase::ReleaseTriggerActor()
{
	if( m_pTriggerActor )
	{
		delete m_pTriggerActor;
		m_pTriggerActor = NULL;
	}	
}

void CBsPhysicsTriggerBase::SetTriggerPose( D3DXMATRIX &matObj )
{
	if( m_pTriggerActor ) {
	    m_pTriggerActor->SetGlobalPose( matObj );
	}
}
