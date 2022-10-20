#include "StdAfx.h"
#include "FcProjectileRoots.h"
#include "FcGlobal.h"
#include "FcGameObject.h"
#include "FcVelocityCtrl.h"
#include "BsFXObject.h"
#include "Data/SignalType.h"
#include "Data/FXList.h"
#include "FcWorld.h"

ASSignalData *CFcProjectileRoots::s_pHitSignal = NULL;

int CFcProjectileRoots::s_nDistance = 900;
int CFcProjectileRoots::s_nGenerateTick = 50;

CFcProjectileRoots::CFcProjectileRoots( CCrossVector *pCross )
: CFcProjectile( pCross )
{
	if( s_pHitSignal == NULL ) {
		s_pHitSignal = new ASSignalData;
		s_pHitSignal->m_nID = SIGNAL_TYPE_HIT_TEMP;
		s_pHitSignal->m_NumParam = HIT2_SIGNAL_LENGTH;
		s_pHitSignal->m_pParam = new int[s_pHitSignal->m_NumParam];

		memset( s_pHitSignal->m_pParam, 0, sizeof(int) * s_pHitSignal->m_NumParam );

		s_pHitSignal->m_Hit2Length = 120;
		s_pHitSignal->m_Hit2AniType = ANI_TYPE_DOWN;
		s_pHitSignal->m_Hit2AniIndex = 0;
		s_pHitSignal->m_Hit2PushY = 25;
		s_pHitSignal->m_Hit2PushZ = -15;
	}
}

CFcProjectileRoots::~CFcProjectileRoots()
{
	for( DWORD i=0; i<m_VecRootList.size(); i++ ) {
		if( m_VecRootList[i].nObjectIndex == -1 ) continue;
		g_BsKernel.DeleteObject( m_VecRootList[i].nObjectIndex );
	}

	m_VecRootList.clear();
}

int CFcProjectileRoots::Initialize( int nSkinIndex, int nRoots, PROJECTILE_PARAM *pParam )
{
	int nResult = CFcProjectile::Initialize( nSkinIndex, pParam );

	m_maxRoots = nRoots;
	m_nDamageCount = 1;
	m_nCreateTick = GetProcessTick();
	m_nParentTeam = m_Param.hParent->GetTeam();

	AddRoot( &m_Cross );
	m_Cross.MoveFrontBack( (float)s_nDistance );	
	m_nDestroyTick = 1;
	if(!m_VecRootList.empty()) 
	{
		CBsFXObject *pObject = (CBsFXObject *)g_BsKernel.GetEngineObjectPtr( m_VecRootList[0].nObjectIndex );		
		if(pObject) {
			m_nDestroyTick = pObject->GetTotalFrame()*nRoots;
			m_nDamageCount = pObject->GetTotalFrame();
		}
	}
	return nResult;
}

void CFcProjectileRoots::Process()
{
	if( m_nDestroyTick <= 0 ) {
		Delete();
		return;
	}
	else 
		--m_nDestroyTick;

	if( ( GetProcessTick() - m_nCreateTick ) % s_nGenerateTick == 0 ) {
		if(int(m_VecRootList.size()) < m_maxRoots) {
			AddRoot( &m_Cross );
			m_Cross.MoveFrontBack( (float)s_nDistance );
		}
	}

	RootStruct *pRoot;
	for( DWORD i=0; i<m_VecRootList.size(); i++ ) {
		pRoot = &m_VecRootList[i];

		switch( pRoot->nState ) {
			case 0:
				g_BsKernel.SetFXObjectState( pRoot->nObjectIndex, CBsFXObject::PLAY );
				pRoot->nState = 1;
				break;
			case 1:
				break;
		}
		CalcTerrain( &pRoot->Cross );
		if( GetProcessTick() - pRoot->nCreateTime < 120 ) {
			if( GetProcessTick() % 5 == 0 ) {
				ProcessDamage( &pRoot->Cross );
			}
		}
	}

	
}

bool CFcProjectileRoots::Render()
{
	for( DWORD i=0; i<m_VecRootList.size(); i++ ) {
		if( m_VecRootList[i].nObjectIndex == -1 ) continue;
		g_BsKernel.UpdateObject( m_VecRootList[i].nObjectIndex, m_VecRootList[i].Cross );
	}
	return true;
}

void CFcProjectileRoots::AddRoot( CCrossVector *pCross )
{
	RootStruct Struct;
	Struct.Cross = *pCross;
	Struct.nCreateTime = GetProcessTick();
	Struct.nObjectIndex = g_BsKernel.CreateFXObject( FX_ESPROOTS );

	m_VecRootList.push_back( Struct );

}

void CFcProjectileRoots::CalcTerrain( CCrossVector *pCross )
{
	D3DXVECTOR3 vNor;
	float fHeight = g_BsKernel.GetLandHeight( pCross->m_PosVector.x, pCross->m_PosVector.z, &vNor );

	pCross->m_PosVector.y = fHeight;
	pCross->m_YVector = vNor;
	pCross->UpdateVectorsY();
}


void CFcProjectileRoots::ProcessDamage(CCrossVector *pCross)
{
	HIT_PARAM HitParam;
	HitParam = MakeHitParam( s_pHitSignal );
	HitParam.Position = pCross->m_PosVector;

	std::vector< GameObjHandle > VecList;
	g_FcWorld.GetEnemyObjectListInRange( m_nParentTeam, &pCross->m_PosVector, m_Param.fSize, VecList );

	for( DWORD i=0; i<VecList.size(); i++ ) {
		if( VecList[i]->IsHittable( HitParam.ObjectHandle ) )
			VecList[i]->CmdHit( &HitParam );
	}
}

