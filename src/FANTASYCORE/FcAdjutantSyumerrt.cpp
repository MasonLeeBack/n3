#include "StdAfx.h"
#include "FcAdjutantSyumerrt.h"

#include "FcFXCommon.h"
#include "FcFxManager.h"

#include "FcWorld.h"
#include "./Data/SignalType.h"
#include "FcProjectile.h"
#include "FcAIObject.h"
#include "FcAISearchSlot.h"
#include "FcAdjutantTeaTea.h"
#include "FcTroopObject.h"
#include "FcProjectileDirectArrow.h"
#include "Data/FXList.h"

#include "FcAbilityManager.h"


CFcAdjutantSyumerrt::CFcAdjutantSyumerrt(CCrossVector *pCross)
: CFcAdjutantObject( pCross )
{
	SetAdjutantClassID( Class_ID_Adjutant_Syumerrt );
	m_nProjectileFXIdx = -1;
}

CFcAdjutantSyumerrt::~CFcAdjutantSyumerrt()
{

}


void CFcAdjutantSyumerrt::Initialize( INITGAMEOBJECTDATA* pData, int nTeam, int nGroup, TroopObjHandle hTroop, float fScaleMin, float fScaleMax )
{
	CFcAdjutantObject::Initialize(pData, nTeam,  nGroup,  hTroop,  fScaleMin, fScaleMax );

	
}




void CFcAdjutantSyumerrt::SignalCheck( ASSignalData *pSignal )
{
	switch( pSignal->m_nID )
	{
	case SIGNAL_TYPE_SHOOT_PROJECTILE:
		if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
		{
			bool bRange = ( CFcTroopObject::s_nTroopStateTable[ GetTroop()->GetState() ] & UNIT_RANGE ) == TRUE ;
			if( bRange == true ) CreateRangeProjectile( pSignal );
			else CreateMeleeProjectile( pSignal );

			/*
			switch(pSignal->m_pParam[6])
			{
			case 0:
				{
					bool bRange = ( CFcTroopObject::s_nTroopStateTable[ GetTroop()->GetState() ] & UNIT_RANGE ) == TRUE ;
					if( bRange ) CreateRangeProjectile( pSignal );
					else CreateMeleeProjectile( pSignal );
				}
				break;
			case 1:  // MUlti
				{
					CreateMultiProjectile( pSignal );
				}
				break;
			case 2: // 1k
				{
					Create1kProjectile( pSignal );
				}
				break;
			case 3: // Pierce
				{
					CreatePierceProjectile( pSignal );
				}
				break;
			}
			*/
		}
		return;
	default:
		break;
	} 
	CFcAdjutantObject::SignalCheck( pSignal );
}

void CFcAdjutantSyumerrt::CreateMeleeProjectile( ASSignalData *pSignal )
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

	Cross = m_Cross;
	Position.x = ( float )pSignal->m_pParam[ 1 ];
	Position.y = ( float )pSignal->m_pParam[ 2 ];
	Position.z = ( float )pSignal->m_pParam[ 3 ];
	D3DXVec3TransformNormal( &Position, &Position, m_Cross );
	Cross.m_PosVector += Position;

	Param.fAngle = 0.f;
	Param.pSignal = pSignal;
	Param.hParent = m_Handle;

	D3DXVECTOR3 vVec = hUnit->GetPos() - GetPos();
	D3DXVec3Normalize( &vVec, &vVec );
	Cross.m_ZVector = vVec;
	Cross.UpdateVectors();

	pPartsData = m_pUnitInfoData->GetPartsGroupInfo( m_WeaponList[ pSignal->m_pParam[ 0 ] ].nPartsIndex );

	switch( m_nCurAniTypeIndex ) {
		case 0:	// 보통
		case 2:	// 보통
			Handle = CFcProjectile::CreateObject( CFcProjectile::DIRECT_ARROW, &Cross );
			Handle->Initialize( pPartsData->m_pSkinIndex[0], &Param );
			break;
		case 1:	// 3Way
			/*
			Handle = CFcProjectile::CreateObject( CFcProjectile::DIRECT_ARROW, &Cross );
			Handle->Initialize( nSkinIndex, &Param );

			Cross.RotateYaw( 90 );
			Handle = CFcProjectile::CreateObject( CFcProjectile::DIRECT_ARROW, &Cross );
			Handle->Initialize( nSkinIndex, &Param );

			Cross.RotateYaw( -180 );
			Handle = CFcProjectile::CreateObject( CFcProjectile::DIRECT_ARROW, &Cross );
			Handle->Initialize( nSkinIndex, &Param );
			*/
			{
				int nArrowCount = 3;
				int nArrowAngle = 90;

				int nAngle = nArrowAngle / ( nArrowCount - 1 );

				for( int i=0; i<nArrowCount; i++ ) {
					if( i == 0 ) Cross.RotateYaw( -nArrowAngle / 2 );
					else Cross.RotateYaw( nAngle );

					Handle = CFcProjectile::CreateObject( CFcProjectile::DIRECT_ARROW, &Cross );
					Handle->Initialize( pPartsData->m_pSkinIndex[0], &Param );
				}
			}
			break;
		case 3:	// 관통
			Handle = CFcProjectile::CreateObject( CFcProjectile::DIRECT_ARROW, &Cross );
			((CFcProjectileDirectArrow*)Handle.GetPointer())->SetFxID( FX_GLOWARROW );
			((CFcProjectileDirectArrow*)Handle.GetPointer())->SetPierce( true );
			Handle->Initialize( pPartsData->m_pSkinIndex[0], &Param );
			break;
		case 4:	// 여러발 난사
			{
				int nArrowCount = 6;
				int nArrowAngle = 90;

				int nAngle = nArrowAngle / ( nArrowCount - 1 );

				for( int i=0; i<nArrowCount; i++ ) {
					if( i == 0 ) Cross.RotateYaw( -nArrowAngle / 2 );
					else Cross.RotateYaw( nAngle );

					Handle = CFcProjectile::CreateObject( CFcProjectile::DIRECT_ARROW, &Cross );
					Handle->Initialize( pPartsData->m_pSkinIndex[0], &Param );
				}
			}
			break;
		case 5:	// 하늘에서 떨어지기
			{
				g_FcWorld.SetAbility( FC_ABILITY_1KARROW, m_Handle, pPartsData->m_pSkinIndex[0]);
			}
			break;
	}
}

void CFcAdjutantSyumerrt::CreateRangeProjectile( ASSignalData *pSignal )
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

	Cross = m_Cross;
	Position.x = ( float )pSignal->m_pParam[ 1 ];
	Position.y = ( float )pSignal->m_pParam[ 2 ];
	Position.z = ( float )pSignal->m_pParam[ 3 ];
	D3DXVec3TransformNormal( &Position, &Position, m_Cross );
	Cross.m_PosVector += Position;

	Param.hParent = m_Handle;
	Param.hTarget = hUnit;

	// Angle 대애~~충~ 구하자..
	D3DXVECTOR3 vTemp1, vTemp2;
	D3DXVECTOR3 vVec1, vVec2;

	vTemp1 = GetPos();			vTemp1.y = 0.f;
	vTemp2 = hUnit->GetPos();	vTemp2.y = 0.f;
	vVec1 = vTemp1 - vTemp2;

	vTemp1 = GetPos();			vTemp1.x = vTemp1.z = 0.f;
	vTemp2 = hUnit->GetPos();	vTemp2.x = vTemp2.z = 0.f;
	vVec2 = vTemp1 - vTemp2;
	float fHeight = GetPos().y - hUnit->GetPos().y;

	D3DXVECTOR3 vResult = vVec1 + vVec2;
	D3DXVec3Normalize( &vResult, &vResult );
	D3DXVec3Normalize( &vVec1, &vVec1 );
	float fDot = D3DXVec3Dot( &vResult, &vVec1 );
	if( fDot >= 1.f ) fDot = 1.f;
	if( fDot <= -1.f ) fDot = -1.f;
	float fAngle = acos(fDot) * 180.f / 3.1415926f;

	if( fHeight <= 0.f ) {
		float fLength = D3DXVec3Length( (D3DXVECTOR3*)&(GetPos() - hUnit->GetPos() ) );
		if( fLength > 3000.f )
			fAngle += PITCHING_MIN_UP_ANGLE;
		else fAngle += ( PITCHING_MIN_UP_ANGLE / 3000.f ) * fLength;
	}
	else {
		fAngle = -fAngle + PITCHING_MIN_DOWN_ANGLE;
	}
	Param.fAngle = fAngle;
	Param.pSignal = pSignal;
	Param.nHitProb = 100;

	pPartsData = m_pUnitInfoData->GetPartsGroupInfo( m_WeaponList[ pSignal->m_pParam[ 0 ] ].nPartsIndex );

	Handle = CFcProjectile::CreateObject( CFcProjectile::ARROW, &Cross );
	Handle->Initialize( pPartsData->m_pSkinIndex[0], &Param );
}

/*
void CFcAdjutantSyumerrt::Create1kProjectile( ASSignalData *pSignal )
{
	GameObjHandle hUnit;
	if( GetAIHandle() ) {
		GameObjHandle *pSearchHandle = GetAIHandle()->GetSearchSlot()->GetHandle(0);
		if( pSearchHandle )
			hUnit = *pSearchHandle;
		else return;
	}
	else return;

	if( !hUnit ) return;


	PROJECTILE_PARAM Param;
	ProjetileHandle Handle;
	ASPartsData *pPartsData;
	int nSkinIndex;
	CCrossVector Cross;
	D3DXVECTOR3 Position;

	for( int ii  = 0; ii < 100 ; ii++)
	{
		Cross = m_Cross;
		Position.x = ( float )pSignal->m_pParam[ 1 ];
		Position.y = ( float )pSignal->m_pParam[ 2 ];
		Position.z = ( float )pSignal->m_pParam[ 3 ];
		D3DXVec3TransformNormal( &Position, &Position, m_Cross );
		Cross.m_PosVector += Position;

		float frandX = RandomNumberInRange( -1.f, 1.f);
		float frandZ = RandomNumberInRange( -1.f, 1.f);
		D3DXVECTOR3 vDir = D3DXVECTOR3( frandX, -2.f, frandZ); 
		D3DXVec3Normalize(&vDir, &vDir);

		Cross.m_ZVector = vDir;
		Cross.UpdateVectors();

		Param.hParent = m_Handle;
		//	Param.hTarget = hUnit;
		Param.pSignal = pSignal;


		pPartsData = m_pUnitInfoData->GetPartsGroupInfo( m_WeaponList[ pSignal->m_pParam[ 0 ] ].nPartsIndex );

		Handle = CFcProjectile::CreateObject( CFcProjectile::DIRECT_ARROW, &Cross );
		Handle->Initialize( pPartsData->m_pSkinIndex[0], &Param );
	}
}




void CFcAdjutantSyumerrt::CreateMultiProjectile( ASSignalData *pSignal )
{
	GameObjHandle hUnit;
	if( GetAIHandle() ) {
		GameObjHandle *pSearchHandle = GetAIHandle()->GetSearchSlot()->GetHandle(0);
		if( pSearchHandle )
			hUnit = *pSearchHandle;
		else return;
	}
	else return;

	if( !hUnit ) return;

	PROJECTILE_PARAM Param;
	ProjetileHandle Handle;
	ASPartsData *pPartsData;
	int nSkinIndex;
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

	D3DXVECTOR3 vVec = hUnit->GetPos() - GetPos();
	D3DXVec3Normalize( &vVec, &vVec );
	Cross.m_ZVector = vVec;
	Cross.UpdateVectors();

	Cross.UpdateVectors();

	pPartsData = m_pUnitInfoData->GetPartsGroupInfo( m_WeaponList[ pSignal->m_pParam[ 0 ] ].nPartsIndex );

	Cross.RotateYaw( -44 );
	Handle = CFcProjectile::CreateObject( CFcProjectile::DIRECT_ARROW, &Cross );
	Handle->Initialize( pPartsData->m_pSkinIndex[0], &Param );

	Cross.RotateYaw( 22 );
	Handle = CFcProjectile::CreateObject( CFcProjectile::DIRECT_ARROW, &Cross );
	Handle->Initialize( pPartsData->m_pSkinIndex[0], &Param );

	Cross.RotateYaw( 22 );
	Handle = CFcProjectile::CreateObject( CFcProjectile::DIRECT_ARROW, &Cross );
	Handle->Initialize( pPartsData->m_pSkinIndex[0], &Param );
	
	Cross.RotateYaw( 22 );
	Handle = CFcProjectile::CreateObject( CFcProjectile::DIRECT_ARROW, &Cross );
	Handle->Initialize( pPartsData->m_pSkinIndex[0], &Param );

	Cross.RotateYaw( 22 );
	Handle = CFcProjectile::CreateObject( CFcProjectile::DIRECT_ARROW, &Cross );
	Handle->Initialize( pPartsData->m_pSkinIndex[0], &Param );
}


void CFcAdjutantSyumerrt::CreatePierceProjectile( ASSignalData *pSignal )
{
	GameObjHandle hUnit;
	if( GetAIHandle() ) {
		GameObjHandle *pSearchHandle = GetAIHandle()->GetSearchSlot()->GetHandle(0);
		if( pSearchHandle )
			hUnit = *pSearchHandle;
		else return;
	}
	else return;

	if( !hUnit ) return;

	PROJECTILE_PARAM Param;
	ProjetileHandle Handle;
	ASPartsData *pPartsData;
	int nSkinIndex;
	CCrossVector Cross;
	D3DXVECTOR3 Position;

	Cross = m_Cross;
	Position.x = ( float )pSignal->m_pParam[ 1 ];
	Position.y = ( float )pSignal->m_pParam[ 2 ];
	Position.z = ( float )pSignal->m_pParam[ 3 ];
	D3DXVec3TransformNormal( &Position, &Position, m_Cross );
	Cross.m_PosVector += Position;

	pPartsData = m_pUnitInfoData->GetPartsGroupInfo( m_WeaponList[ pSignal->m_pParam[ 0 ] ].nPartsIndex );

	Handle = CFcProjectile::CreateObject( CFcProjectile::DIRECT_ARROW, &Cross );

	((CFcProjectileDirectArrow*)Handle.GetPointer())->SetFxID( FX_GLOWARROW );
	((CFcProjectileDirectArrow*)Handle.GetPointer())->SetPierce( true );

	Handle->Initialize( pPartsData->m_pSkinIndex[0], &Param );

}

*/