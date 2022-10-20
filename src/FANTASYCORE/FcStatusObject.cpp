#include "StdAfx.h"
#include "FcStatusObject.h"
#include "BsKernel.h"
#include "FcGameObject.h"
#include "FcWorld.h"
#include "Data/FXList.h"
#include "FcHeroObject.h"
#include "FcAIObject.h"
#include "FcAISearchSlot.h"
#include "FcTroopObject.h"

CFcStatusObject::CFcStatusObject() 
{ 
	m_bFinish = false; 
}

CFcStatusObject::~CFcStatusObject()
{
}

CFcStatusObject *CFcStatusObject::BeginStatus( GameObjHandle Handle, STATUS_TYPE Type, int nParam1, int nParam2, int nParam3  )
{
	CFcStatusObject *pStatus = NULL;
	switch( Type ) {
		case STATUS_CHAOS:			pStatus = new CFcStatusChaos();			break;
		case STATUS_DOWN_SPEED:		pStatus = new CFcStatusDownMoveSpeed();	break;
		case STATUS_RECOVERY_HP:	pStatus = new CFcStatusRecoveryHP();	break;
		case STATUS_IGNITION:		pStatus = new CFcStatusIgnition();		break;
	}
	if( pStatus == NULL ) return NULL;

	pStatus->m_Handle = Handle;
	pStatus->m_Type = Type;

	pStatus->m_nParam[0] = nParam1;
	pStatus->m_nParam[1] = nParam2;
	pStatus->m_nParam[2] = nParam3;

	pStatus->BeginStatus();
	return pStatus;
}


/*
 *	Chaos
 */
CFcStatusChaos::CFcStatusChaos()
{
	m_nParticleObjectIndex = -1;
}

CFcStatusChaos::~CFcStatusChaos()
{
	if( m_nParticleObjectIndex != -1 ) {
		g_BsKernel.StopParticlePlay( m_nParticleObjectIndex );
		m_nParticleObjectIndex = -1;
	}
}

void CFcStatusChaos::BeginStatus()
{
	m_CrossVector = *m_Handle->GetCrossVector();
	m_CrossVector.m_PosVector.y += m_Handle->GetUnitHeight();
	m_nParticleObjectIndex = g_BsKernel.CreateParticleObject( 83, true, true, m_CrossVector );

	m_nTimer = m_nParam[0];

	m_nPrevTeam = m_Handle->GetTeam();
	while(1) {
		m_Handle->SetTeam( Random(8) );
		if( m_Handle->GetTeam() != m_nPrevTeam ) break;
	}
}

void CFcStatusChaos::FinishStatus()
{
	if( m_nParticleObjectIndex != -1 ) {
		g_BsKernel.StopParticlePlay( m_nParticleObjectIndex );
		m_nParticleObjectIndex = -1;
	}

	m_Handle->SetTeam( m_nPrevTeam );
	if( m_Handle->GetAIHandle() ) {
		m_Handle->GetAIHandle()->GetSearchSlot()->SetHandle( 0, NULL );
	}
}

void CFcStatusChaos::Process()
{
	if( m_nTimer <= 0 ) {
		m_bFinish = true;
		return;
	}
	else --m_nTimer;

	if( m_nTimer == 40 ) {
		g_BsKernel.StopParticlePlay( m_nParticleObjectIndex );
		m_nParticleObjectIndex = -1;
	}

	m_CrossVector = *m_Handle->GetCrossVector();
	m_CrossVector.m_PosVector.y += m_Handle->GetUnitHeight();

	if( m_nTimer > 20 && m_nParticleObjectIndex != -1)
		g_BsKernel.UpdateParticle( m_nParticleObjectIndex, m_CrossVector );

	if( m_Handle->GetAIHandle() ) {
		GameObjHandle *pHandle = m_Handle->GetAIHandle()->GetSearchSlot()->GetHandle(0);
		if( pHandle == NULL || ( pHandle && CFcBaseObject::IsValid( (*pHandle) ) && (*pHandle)->GetTeam() != m_Handle->GetTeam() ) ) {
			m_Handle->GetAIHandle()->GetSearchSlot()->SetHandle( 0, NULL );
			TroopObjHandle hTroop = m_Handle->GetTroop();

			std::vector<GameObjHandle> vecList;
			GameObjHandle Handle;
			for( int i=0; i<hTroop->GetUnitCount(); i++ ) {
				Handle = hTroop->GetUnit(i);
				if( CFcBaseObject::IsValid( Handle ) && !Handle->IsDie() ) {
					vecList.push_back( Handle );
				}
			}
			Handle = vecList[ Random( (int)vecList.size() ) ];
			m_Handle->GetAIHandle()->GetSearchSlot()->SetHandle( 0, &Handle );
		}
	}
}

/*
 *	Recovery HP
 */

void CFcStatusRecoveryHP::BeginStatus()
{
	CCrossVector Cross;
	g_FcWorld.PlaySimpleFx( FX_GBMADO05CURE, m_Handle->GetCrossVector(), 1 );
	m_Handle->AddHPPercent( m_nParam[0] );
	m_bFinish = true;
}
/*
 *	Down Speed
 */
CFcStatusDownMoveSpeed::CFcStatusDownMoveSpeed()
{
	m_nFxObjectIndex = -1;
}

CFcStatusDownMoveSpeed::~CFcStatusDownMoveSpeed()
{
	if ( m_nFxObjectIndex != -1 )
	{
		g_BsKernel.DeleteObject( m_nFxObjectIndex );
		m_nFxObjectIndex = -1;
	}
}

void CFcStatusDownMoveSpeed::Process()
{
	if( m_nTimer <= 0 ) {
		if( m_nFxObjectIndex == -1 ) m_bFinish = true;

		if( g_BsKernel.GetFXObjectState( m_nFxObjectIndex ) == CBsFXObject::PLAY )
			g_BsKernel.SetFXObjectState( m_nFxObjectIndex, CBsFXObject::STOPLOOP );

		if( g_BsKernel.GetFXObjectState( m_nFxObjectIndex ) == CBsFXObject::STOP ) {
			m_bFinish = true;
			return;
		}
	}
	else --m_nTimer;

	/*
	if( m_Handle->GetClassID() == CFcGameObject::Class_ID_Hero && ((CFcHeroObject*)m_Handle.GetPointer())->GetPlayerIndex() != -1 && m_Handle->GetCurAniType() == ANI_TYPE_RUN ) {
		CFcHeroObject *pHero = (CFcHeroObject *)m_Handle.GetPointer();
		float fSpeed = CHeroLevelTableLoader::GetInstance().GetMoveSpeed( m_Handle->GetUnitSoxIndex(), m_Handle->GetLevel() ) * m_Handle->GetFrameAdd();
		m_Handle->SetMoveSpeed( fSpeed * ( m_nParam[1] / 100.f ) );
	}
	else {
		float fSpeed = m_Handle->GetUnitInfoData()->GetMoveSpeed( m_Handle->GetAniIndex() );
		m_Handle->SetMoveSpeed( fSpeed * ( m_nParam[1] / 100.f ) );
	}
	*/
	m_CrossVector = *m_Handle->GetCrossVector();
	g_BsKernel.UpdateObject( m_nFxObjectIndex, m_CrossVector );
}

void CFcStatusDownMoveSpeed::PostProcess()
{
	if( m_Handle->IsForceMove() ) return;
	D3DXVECTOR3 *pMoveVec = m_Handle->GetMoveVector();
	*pMoveVec *= (float)( m_nParam[1] / 100.f );

	pMoveVec = m_Handle->GetLocalAniMove();
	*pMoveVec *= (float)( m_nParam[1] / 100.f );
}

void CFcStatusDownMoveSpeed::BeginStatus()
{
	BsAssert( m_nFxObjectIndex == -1 );

	m_nFxObjectIndex = g_BsKernel.CreateFXObject( FX_GBMADO06SLOW );
	BsAssert( m_nFxObjectIndex >= 0 );
	g_BsKernel.SetFXObjectState( m_nFxObjectIndex, CBsFXObject::PLAY, -1 );

	m_nTimer = m_nParam[0];
}

void CFcStatusDownMoveSpeed::FinishStatus()
{
	if( m_nFxObjectIndex != -1 ) 
	{
		g_BsKernel.DeleteObject( m_nFxObjectIndex );
		m_nFxObjectIndex = -1;
	}

	if( m_Handle->GetClassID() == CFcGameObject::Class_ID_Hero && ((CFcHeroObject*)m_Handle.GetPointer())->GetPlayerIndex() != -1 ) {
		CFcHeroObject *pHero = (CFcHeroObject *)m_Handle.GetPointer();
		float fSpeed = CHeroLevelTableLoader::GetInstance().GetMoveSpeed( m_Handle->GetUnitSoxIndex(), m_Handle->GetLevel() ) * m_Handle->GetFrameAdd() * pHero->GetMoveSpeedAdd();
		m_Handle->SetMoveSpeed( fSpeed );
		return;
	}

	float fSpeed = m_Handle->GetUnitInfoData()->GetMoveSpeed( m_Handle->GetAniIndex() );
	m_Handle->SetMoveSpeed( fSpeed );
}

/*
 *	Ignition
 */
CFcStatusIgnition::CFcStatusIgnition()
{
	m_nFxObjectIndex = -1;
}

CFcStatusIgnition::~CFcStatusIgnition()
{
	if ( m_nFxObjectIndex != -1 )
	{
		g_BsKernel.DeleteObject( m_nFxObjectIndex );
		m_nFxObjectIndex = -1;
	}
}

void CFcStatusIgnition::Process()
{
	if( m_nTimer <= 0 ) {
		if( m_nFxObjectIndex == -1 ) m_bFinish = true;

		if( g_BsKernel.GetFXObjectState( m_nFxObjectIndex ) == CBsFXObject::PLAY )
			g_BsKernel.SetFXObjectState( m_nFxObjectIndex, CBsFXObject::STOPLOOP );

		if( g_BsKernel.GetFXObjectState( m_nFxObjectIndex ) == CBsFXObject::STOP ) {
			m_bFinish = true;
			return;
		}
	}
	else --m_nTimer;

	if( m_nTimer > 0 ) {
		if( m_nTimer % 40 == 0 ) {
			int nPrevHP = m_Handle->GetHP();
			m_Handle->CalculateDamage( m_nParam[1] );
			if( m_Handle->GetHP() <= 0 ) {
				m_Handle->SetHP( nPrevHP );
				m_nTimer = 0;
			}
		}
		if( m_nTimer % 200 == 0 && Random(2) == 0 ) {
			m_Handle->ChangeAnimation( ANI_TYPE_HIT, 0 );
		}
	}

	m_CrossVector = *m_Handle->GetCrossVector();
	g_BsKernel.UpdateObject( m_nFxObjectIndex, m_CrossVector );
}

void CFcStatusIgnition::BeginStatus()
{
	m_nFxObjectIndex = g_BsKernel.CreateFXObject( FX_MIRA03BURNING );
	BsAssert( m_nFxObjectIndex >= 0 );
	g_BsKernel.SetFXObjectState( m_nFxObjectIndex, CBsFXObject::PLAY, -1 );

	m_nTimer = m_nParam[0];
}

void CFcStatusIgnition::FinishStatus()
{
	if( m_nFxObjectIndex != -1 )
	{
		g_BsKernel.DeleteObject( m_nFxObjectIndex );
		m_nFxObjectIndex = -1;
	}
}
