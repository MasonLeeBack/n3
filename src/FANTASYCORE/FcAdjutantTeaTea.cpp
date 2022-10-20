#include "StdAfx.h"
#include "FcAdjutantObject.h"

#include "FcFXCommon.h"
#include "FcFxManager.h"

#include "FcWorld.h"
#include "./Data/SignalType.h"
#include "FcProjectile.h"
#include "FcAIObject.h"
#include "FcAISearchSlot.h"
#include "FcAdjutantTeaTea.h"


CFcAdjutantTeaTea::CFcAdjutantTeaTea(CCrossVector *pCross)
: CFcAdjutantObject( pCross )
{
	SetAdjutantClassID( Class_ID_Adjutant_TeaTea );
}

CFcAdjutantTeaTea::~CFcAdjutantTeaTea()
{
}

void CFcAdjutantTeaTea::SignalCheck( ASSignalData *pSignal )
{
	switch( pSignal->m_nID )
	{
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
			ASPartsData *pPartsData;
			CCrossVector Cross;
			D3DXVECTOR3 Position;

			D3DXMATRIX matWeapon;
			int nWeaponIndex = m_WeaponList[ pSignal->m_pParam[0] ].nObjectIndex;
			matWeapon = *( D3DXMATRIX * )g_BsKernel.SendMessage( nWeaponIndex, BS_GET_OBJECT_MAT );
			memcpy( &Cross.m_XVector, &matWeapon._11, sizeof( D3DXVECTOR3 ) );
			memcpy( &Cross.m_YVector, &matWeapon._21, sizeof( D3DXVECTOR3 ) );
			memcpy( &Cross.m_ZVector, &matWeapon._31, sizeof( D3DXVECTOR3 ) );
			memcpy( &Cross.m_PosVector, &matWeapon._41, sizeof( D3DXVECTOR3 ) );

			Param.pSignal = pSignal;
			Param.hParent = m_Handle;
			Param.vTarget = m_Cross.m_ZVector;
			D3DXVec3Normalize( &Param.vTarget, &Param.vTarget );
			Param.vTarget *= 35.f;

			pPartsData = m_pUnitInfoData->GetPartsGroupInfo( m_WeaponList[ pSignal->m_pParam[ 0 ] ].nPartsIndex );

			Handle = CFcProjectile::CreateObject( CFcProjectile::THROW_STONE, &Cross );
			Handle->Initialize( pPartsData->m_pSkinIndex[0], &Param );
		}
		return;
	default:
		break;
	} 
	CFcAdjutantObject::SignalCheck( pSignal );
}

