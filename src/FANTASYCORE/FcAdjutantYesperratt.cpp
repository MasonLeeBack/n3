#include "StdAfx.h"
#include "FcAdjutantObject.h"

#include "FcFXCommon.h"
#include "FcFxManager.h"

#include "FcWorld.h"
#include "./Data/SignalType.h"
#include "FcProjectile.h"
#include "FcAIObject.h"
#include "FcAISearchSlot.h"
#include "FcAdjutantYesperratt.h"
#include "FcProjectileTimeBomb.h"
#include "FcProjectileRoots.h"
#include "FcUtil.h"
#include "Data/FXList.h"
#include "FcTroopObject.h"
#include "FcProjectileMagicFx.h"
#include "FcVelocityCtrl.h"
#include "FcGlobal.h"
#include "FcSoundManager.h"


CFcAdjutantYesperratt::CFcAdjutantYesperratt(CCrossVector *pCross)
: CFcAdjutantObject( pCross )
{
	SetAdjutantClassID( Class_ID_Adjutant_Yesperratt );
	m_nVolcanoRemainCount = 0;
	m_pVolcanoSignal = NULL;
}

CFcAdjutantYesperratt::~CFcAdjutantYesperratt()
{
}

void CFcAdjutantYesperratt::SignalCheck( ASSignalData *pSignal )
{
	switch( pSignal->m_nID )
	{
	case SIGNAL_TYPE_SHOOT_PROJECTILE:
		if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
		{
			switch( m_nCurAniTypeIndex ) {	
				case 0:
					ProectileRoots( pSignal );
					break;
				case 1:
					ProjectileVolcano( pSignal );
					break;
				case 3:
					ProjectileNiddle( pSignal );
					break;
			}
		}
		return;
	default:
		break;
	} 
	CFcAdjutantObject::SignalCheck( pSignal );
}


void CFcAdjutantYesperratt::ProectileRoots( ASSignalData *pSignal )
{
	GameObjHandle hUnit;
	if( GetAIHandle() ) {
		GameObjHandle *pSearchHandle = GetAIHandle()->GetSearchSlot()->GetHandle(0);
		if( pSearchHandle )
			hUnit = *pSearchHandle;
		else return;
	}
	else return;
	if( !CFcBaseObject::IsValid( hUnit )  ) return;

	CCrossVector Cross;
	PROJECTILE_PARAM Param;
	ProjetileHandle Handle;

	Param.hParent = m_Handle;
	Param.pSignal = pSignal;
	// 뿌리 Damage 영역
	Param.fSize = 300.f;

	Cross = m_Cross;

	// 레벨에 따라 조절해주3
	int nArrowCount = 3;
	int nArrowAngle = 170;

	int nAngle = nArrowAngle / ( nArrowCount - 1 );

	for( int i=0; i<nArrowCount; i++ ) {
		if( i == 0 ) Cross.RotateYaw( -nArrowAngle / 2 );
		else Cross.RotateYaw( nAngle );

		Cross.MoveFrontBack( 400.f );
		Handle = CFcProjectile::CreateObject( CFcProjectile::ROOTS, &Cross );
		((CFcProjectileRoots*)Handle.GetPointer())->Initialize(-1, 3, &Param );
		Cross.MoveFrontBack( -400.f );
	}

	m_RootsSound.Init();
	m_RootsSound.nHandle = g_pSoundManager->Play3DSound(this,SB_COMMON,"EM_YR_ATTACK_01", &m_Cross.m_PosVector);
}

void CFcAdjutantYesperratt::ProjectileVolcano( ASSignalData *pSignal )
{
	GameObjHandle hUnit;
	if( GetAIHandle() ) {
		GameObjHandle *pSearchHandle = GetAIHandle()->GetSearchSlot()->GetHandle(0);
		if( pSearchHandle )
			hUnit = *pSearchHandle;
		else return;
	}
	else return;
	if( !CFcBaseObject::IsValid( hUnit ) ) return;

	// 레벨에 따라 조절해 주삼
//	m_VolcanoCross = *hUnit->GetCrossVector();
	m_VolcanoPos = hUnit->GetPos();
	m_nVolcanoRemainCount = 8;
	m_nVolcanoInterval = 20;
	m_nVolcanoRandomRange = 500;
	m_pVolcanoSignal = pSignal;

	m_nVolcanoPrevTick = GetProcessTick();
	/*
	for( int i=0; i<nCount; i++ ) {
		Cross = *hUnit->GetCrossVector();
		Cross.RotateYaw( Random(1024) );
		Cross.MoveFrontBack( (float)Random( nRange ) );

		Handle = CFcProjectile::CreateObject( CFcProjectile::TIMEBOMB, &Cross );
		((CFcProjectileTimeBomb*)Handle.GetPointer())->Initialize( FX_ESPVOLCANO, -1, 18, 4, 10, &Param );
	}
	*/
}


void CFcAdjutantYesperratt::ProjectileNiddle( ASSignalData *pSignal )
{
	GameObjHandle hUnit;
	if( GetAIHandle() ) {
		GameObjHandle *pSearchHandle = GetAIHandle()->GetSearchSlot()->GetHandle(0);
		if( pSearchHandle )
			hUnit = *pSearchHandle;
		else return;
	}
	else return;
	if( !CFcBaseObject::IsValid( hUnit ) ) return;

	CCrossVector Cross;
	PROJECTILE_PARAM Param;
	ProjetileHandle Handle;

	Param.hParent = m_Handle;
	Param.pSignal = pSignal;
	Param.fSize = 150.f;

	Cross = *hUnit->GetCrossVector();
	Cross.RotateYaw( Random(1024) );
	Cross.MoveFrontBack( (float)Random( 150 ) );

	Handle = CFcProjectile::CreateObject( CFcProjectile::TIMEBOMB, &Cross );
	((CFcProjectileTimeBomb*)Handle.GetPointer())->Initialize( FX_ESPNIDDLE, -1, 18, 4, 10, &Param );
}

void CFcAdjutantYesperratt::Process()
{
	if( m_RootsSound.nPlayCount )
	{
		if( m_RootsSound.nHandle != -1 )
		{
			if( g_pSoundManager->IsOwner( this, m_RootsSound.nHandle ) == false) {							
				m_RootsSound.nHandle = g_pSoundManager->Play3DSound(this,SB_COMMON,"EM_YR_ATTACK_01", &m_Cross.m_PosVector);
				m_RootsSound.nPlayCount--;
			}			
		}	
	}

	CFcAdjutantObject::Process();
	if( m_nVolcanoRemainCount > 0 ) {
		if( GetProcessTick() - m_nVolcanoPrevTick >= m_nVolcanoInterval ) {
			m_nVolcanoPrevTick += m_nVolcanoInterval;

			CCrossVector Cross;
			PROJECTILE_PARAM Param;
			ProjetileHandle Handle;
			D3DXVECTOR3 vNor;

			Param.hParent = m_Handle;
			Param.pSignal = m_pVolcanoSignal;
			Param.fSize = 150.f;


			Cross.m_PosVector = m_VolcanoPos;
			Cross.m_PosVector.y = g_BsKernel.GetLandHeight( m_VolcanoPos.x, m_VolcanoPos.z, &vNor );
			Cross.m_YVector = vNor;
			Cross.UpdateVectorsY();

			Cross.RotateYaw( Random(1024) );
			Cross.MoveFrontBack( (float)Random( m_nVolcanoRandomRange ) );

			Handle = CFcProjectile::CreateObject( CFcProjectile::TIMEBOMB, &Cross );
			((CFcProjectileTimeBomb*)Handle.GetPointer())->Initialize( FX_ESPVOLCANO, -1, 18, 4, 10, &Param );

			m_nVolcanoRemainCount--;
		}
	}
}