#include "StdAfx.h"
#include "FcProjectileDirectArrow.h"
#include "FcFXManager.h"
#include "FcUnitObject.h"
#include "FcTroopObject.h"
#include "FcGlobal.h"
#include "ASData.h"
#include "DebugUtil.h"
#include "Data/SignalType.h"
#include "Box3.h"
#include "Ray3.h"
#include "IntLin3Box3.h"
#include "FcWorld.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


ASSignalData *CFcProjectileDirectArrow::s_pHitSignal = NULL;

CFcProjectileDirectArrow::CFcProjectileDirectArrow( CCrossVector *pCross )
: CFcProjectile( pCross )
{
	m_nFXLineIndex = -1;
	m_nDestroyTick = -1;

	if( s_pHitSignal == NULL ) {
		s_pHitSignal = new ASSignalData;
		s_pHitSignal->m_nID = SIGNAL_TYPE_HIT_TEMP;
		s_pHitSignal->m_NumParam = HIT2_SIGNAL_LENGTH;
		s_pHitSignal->m_pParam = new int[s_pHitSignal->m_NumParam];

		memset( s_pHitSignal->m_pParam, 0, sizeof(int) * s_pHitSignal->m_NumParam );

		s_pHitSignal->m_Hit2AniType = ANI_TYPE_HIT;
		s_pHitSignal->m_Hit2AniIndex = 0;
		s_pHitSignal->m_Hit2PushY = 5;
		s_pHitSignal->m_Hit2PushZ = -10;
		s_pHitSignal->m_Hit2SoundAction = ATT_NONE;
		s_pHitSignal->m_Hit2WeaponMaterial = WT_BOW;

	}

	m_nFxid = -1;
	m_bPierce = false;
}

CFcProjectileDirectArrow::~CFcProjectileDirectArrow()
{
	SAFE_DELETE_FX(m_nFXLineIndex,FX_TYPE_LINETRAIL);
	if(m_nFxid != -1 )
	{
		g_BsKernel.DeleteObject( m_nFxid );
		m_nFxid = -1;
	}
}

void CFcProjectileDirectArrow::ProcessDamage()
{
	std::vector<GameObjHandle> VecList;
	g_FcWorld.GetEnemyObjectListInRange( m_nParentTeam, &m_Cross.m_PosVector, m_fVelocity, VecList );
	if( VecList.empty() ) return;

	Box3 Box;
	Segment3 Seg3;

	for( DWORD i=0; i<VecList.size(); i++ ) {
		if( VecList[i] == m_Param.hParent ) continue;
		if( !VecList[i]->IsHittable( m_Param.hParent ) ) continue;

		VecList[i]->GetBox3( Box, VecList[i]->GetBoundingBox() );
		Seg3.P.x = m_Cross.m_PosVector.x;
		Seg3.P.y = m_Cross.m_PosVector.y;
		Seg3.P.z = m_Cross.m_PosVector.z;

		Seg3.D.x = m_Cross.m_PosVector.x - m_PrevPos.x;
		Seg3.D.y = m_Cross.m_PosVector.y - m_PrevPos.y;
		Seg3.D.z = m_Cross.m_PosVector.z - m_PrevPos.z;

		if( !TestIntersection( Seg3, Box ) ) continue;

		HIT_PARAM HitParam = MakeHitParam( s_pHitSignal );

		VecList[i]->CmdHit( &HitParam );

		if( m_bPierce == false ) {
			m_nDestroyTick = m_nTrailLifeTick * 5;
			m_bUpdateObject = false;
		}

	}
}

int CFcProjectileDirectArrow::Initialize( int nSkinIndex, PROJECTILE_PARAM *pParam )
{
	int nResult = CFcProjectile::Initialize( nSkinIndex, pParam );

	m_nDestroyTick = 400;
	m_nTrailLifeTick = 20;

	if( m_nFxid == -1 )
	{
		m_nFXLineIndex = g_pFcFXManager->Create(FX_TYPE_LINETRAIL);
	}

	m_fVelocity = 100.f;
	float nE = 25.f;
	float nW = 0.8f;
	static char szFile[128] = "arrowline.dds";
	g_BsKernel.LoadTexture( szFile );

	if(m_nFXLineIndex != -1)
	{
		g_pFcFXManager->SendMessage( m_nFXLineIndex, FX_INIT_OBJECT, m_nTrailLifeTick, (DWORD)&nW, (DWORD)&nE );
		g_pFcFXManager->SendMessage( m_nFXLineIndex, FX_SET_TEXTURE, (DWORD)szFile );
		g_pFcFXManager->SendMessage( m_nFXLineIndex, FX_PLAY_OBJECT );
	}

	m_PrevPos = m_Cross.m_PosVector;

	if( m_nFxid != -1 ) {
		m_nFxid = g_BsKernel.CreateFXObject( m_nFxid );
		g_BsKernel.SetFXObjectState( m_nFxid, CBsFXObject::PLAY, -1 ); 
	}

	m_nParentTeam = m_Param.hParent->GetTeam();

	return nResult;
}

void CFcProjectileDirectArrow::Process()
{
	if( m_nDestroyTick != -1 ) {
		--m_nDestroyTick;
		if( m_nDestroyTick == 0 ) { Delete(); }

	}


	if( m_bUpdateObject ) 
	{ 

		if( CheckHitGround() )
		{
			m_nDestroyTick = m_nTrailLifeTick * 5;
		//	m_Cross.m_PosVector = newPos;
		//	m_Cross.m_PosVector.y = g_BsKernel.GetLandHeight( m_Cross.m_PosVector.x, m_Cross.m_PosVector.z ) - 10.f;  
			m_Cross.m_PosVector.y = GetLandHeight();
			m_bUpdateObject = false;
		}
		else
		{
			m_PrevPos = m_Cross.m_PosVector;
			m_Cross.MoveFrontBack( m_fVelocity );

		}
	

		ProcessDamage();
		if(m_nFXLineIndex != -1)
			g_pFcFXManager->SendMessage( m_nFXLineIndex, FX_UPDATE_OBJECT, (DWORD)&m_Cross.m_PosVector );

		if( m_nFxid != -1 )
			g_BsKernel.UpdateObject(m_nFxid, m_Cross);
	}
}
