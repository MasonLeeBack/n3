#include "StdAfx.h"
#include "FcUnit9SObject.h"
#include "Data/SignalType.h"
#include "ASData.h"
#include "FcProjectile.h"
#include "FcAIObject.h"
#include "FcAISearchSlot.h"
#include "FcProjectileMagicFx.h"
#include "Data/FXList.h"
#include "FcVelocityCtrl.h"


CFcUnit9SObject::CFcUnit9SObject(CCrossVector *pCross) 
: CFcUnitObject( pCross )
{
}

CFcUnit9SObject::~CFcUnit9SObject()
{
}

void CFcUnit9SObject::SignalCheck( ASSignalData *pSignal )
{
	switch( pSignal->m_nID ) {
		case SIGNAL_TYPE_SHOOT_PROJECTILE:
			if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
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
				Param.hTarget = hUnit;
				Param.fSize = 180.f;

				D3DXVECTOR3 vVec = hUnit->GetPos() - GetPos();
				vVec.y = 0.f;
				D3DXVec3Normalize( &vVec, &vVec );
				Cross.m_ZVector = vVec;
				Cross.UpdateVectors();

				Handle = CFcProjectile::CreateObject( CFcProjectile::MAGIC_FX, &Cross );

				((CFcProjectileMagicFx*)Handle.GetPointer())->Initialize( FX_99BALL02, FX_99BALL01, &Param );
				((CFcProjectileMagicFx*)Handle.GetPointer())->SetDestroyTick( 220 );
				CFcVelocityCtrl *pCtrl = ((CFcProjectileMagicFx*)Handle.GetPointer())->GetVelocityCtrl();
				pCtrl->SetMaximumSpeed( 25.f );

			}
			break;
		default:
			break;
	}
	CFcUnitObject::SignalCheck( pSignal );
}
