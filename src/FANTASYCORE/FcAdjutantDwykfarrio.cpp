#include "StdAfx.h"
#include "FcAdjutantObject.h"
#include "FcAdjutantDwykfarrio.h"

#include "./Data/SignalType.h"
#include "ASData.h"
#include "FcProjectile.h"
#include "FcProjectileMagicFx.h"
#include "Data/FXList.h"
#include "FcVelocityCtrl.h"


CFcAdjutantDwykfarrio::CFcAdjutantDwykfarrio(CCrossVector *pCross)
: CFcAdjutantObject( pCross )
{
	SetAdjutantClassID( Class_ID_Adjutant_Dwykfarrio );
}

CFcAdjutantDwykfarrio::~CFcAdjutantDwykfarrio()
{
}


void CFcAdjutantDwykfarrio::SignalCheck( ASSignalData *pSignal )
{
	switch( pSignal->m_nID ) {
		case SIGNAL_TYPE_SHOOT_PROJECTILE:
			if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
			{
				PROJECTILE_PARAM Param;
				ProjetileHandle Handle;
				CCrossVector Cross;
				D3DXVECTOR3 Position;

				Cross = m_Cross;
				Position.x = ( float )pSignal->m_pParam[ 1 ];
				Position.y = ( float )pSignal->m_pParam[ 2 ];
				Position.z = ( float )pSignal->m_pParam[ 3 ];
				D3DXVec3TransformNormal( &Position, &Position, m_Cross );
				Cross.m_PosVector += Position;

				Param.fAngle = 0.f;
				Param.pSignal = pSignal;
				Param.hParent = m_Handle;
				Param.fSize = 250.f;

				Handle = CFcProjectile::CreateObject( CFcProjectile::MAGIC_FX, &Cross );
				((CFcProjectileMagicFx*)Handle.GetPointer())->Initialize( FX_DICKPHARIO_X, -1, &Param );
				CFcVelocityCtrl *pCtrl = ((CFcProjectileMagicFx*)Handle.GetPointer())->GetVelocityCtrl();
				pCtrl->SetStartSpeed( 30.f );
				pCtrl->SetAcceleration( 1.2f );
				pCtrl->SetMaximumSpeed( 60.f );
				pCtrl->SetMinimumSpeed( 30.f );
			}
			return;
		default:
			break;
	}
	CFcAdjutantObject::SignalCheck( pSignal );
}