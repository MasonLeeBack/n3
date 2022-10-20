#include "StdAfx.h"
#include "FcAdjutantObject.h"

#include "FcFXCommon.h"
#include "FcFxManager.h"

#include "FcTroopObject.h"

#include "FcWorld.h"

// Sub Class
#include "FcAdjutantTeaTea.h"
#include "FcAdjutantSyumerrt.h"
#include "FcAdjutantDarkElf.h"
#include "FcAdjutantKingFrog.h"
#include "FcAdjutantGoblinWizard.h"
#include "FcAdjutantMylarrvaryss.h"
#include "FcAdjutantYesperratt.h"
#include "FcAdjutantDwykfarrio.h"
#include "FcAdjutant99King.h"
#include "FcHeroObject.h"

#define ADJUTANT_RENEW_FRAME 800
#define ADJUTANT_RENEW_RECOVERY_HP_RATIO 0.3

CFcAdjutantObject::CFcAdjutantObject(CCrossVector *pCross)
: CFcUnitObject( pCross )
{
	SetClassID( Class_ID_Adjutant );
	m_nDieRenewFrame = 0;
	m_nDieType = 0;
	m_fOnlyPlayerDamage = 0.0f;
}

CFcAdjutantObject::~CFcAdjutantObject()
{
}

GameObjHandle CFcAdjutantObject::CreateAdjutantObject( CCrossVector *pCross, int nParam )
{
	switch( nParam ) {
		case 44:	// 고블린 마도승
			return CFcBaseObject::CreateObject< CFcAdjutantGoblinWizard >( pCross );
		case 46:	// 테아테아
			return CFcBaseObject::CreateObject< CFcAdjutantTeaTea >( pCross );
		case 47:	// 슈멜
			return CFcBaseObject::CreateObject< CFcAdjutantSyumerrt >( pCross );
		case 56:	// 미라바리스
			return CFcBaseObject::CreateObject< CFcAdjutantMylarrvaryss >( pCross );
		case 76:	// 디크파리오
			return CFcBaseObject::CreateObject< CFcAdjutantDwykfarrio >( pCross );
		case 77:	// 구구밤왕
			return CFcBaseObject::CreateObject< CFcAdjutant99King >( pCross );
		case 78:	// 다크엘프 족장
			return CFcBaseObject::CreateObject< CFcAdjutantDarkElf >( pCross );
		case 79:	// 개구리왕
			return CFcBaseObject::CreateObject< CFcAdjutantKingFrog >( pCross );
		case 80:	// 에스페라다
			return CFcBaseObject::CreateObject< CFcAdjutantYesperratt >( pCross );

	}
	return CFcBaseObject::CreateObject< CFcAdjutantObject >( pCross );
}

void CFcAdjutantObject::Initialize( INITGAMEOBJECTDATA* pData, int nTeam, int nGroup, TroopObjHandle hTroop, float fScaleMin, float fScaleMax )
{
	CFcUnitObject::Initialize(pData, nTeam,  nGroup,  hTroop,  fScaleMin, fScaleMax );

	m_nTrailHandle[ 0 ] = g_pFcFXManager->Create( FX_TYPE_WEAPONTRAIL );
	g_pFcFXManager->SendMessage( m_nTrailHandle[ 0 ], FX_INIT_OBJECT );
}


void CFcAdjutantObject::Process()
{
	m_bItemDrop = true;
	CFcUnitObject::Process();

	switch( m_nDieType ) {
		case 1:
			if( m_nDieRenewFrame != 0 )		// 같은 편일 때만 다시 일어나게 처리
				++m_nDieRenewFrame;

			if( m_nDieRenewFrame > ADJUTANT_RENEW_FRAME ) {
				m_fHP = (float)(GetMaxHP() * ADJUTANT_RENEW_RECOVERY_HP_RATIO);

				// 일단 죽었따가 일어나기가 없으니 걍 일어나기루 처리..
				int nIndex = GetCurAniIndex();
				ChangeAnimation( ANI_TYPE_RISE, nIndex );
				m_nDieRenewFrame = 0;
			}
			break;

		default:	break;
	}
}

bool CFcAdjutantObject::Render()
{
	switch( m_nDieType ) {
		case 1:
			m_bDead = false;
			break;
		default:	break;
	}
	return CFcUnitObject::Render();
}


void CFcAdjutantObject::SetDie( int nWeaponDropPercent )
{
	switch( m_nDieType ) {
		case 0:
			if( m_nDieFrame == 0 ) {
				m_nDieFrame = 1;
				if( m_hTroop ) m_hTroop->CmdDie( m_Handle );
			}
			m_bDead = true;
			if( m_bIsInScanner == true ) // <- 아래랑 해서 일단 디버그 스트링땜에 체크해서 넌다..
				SetInScanner( false );
			break;
		case 1:
			if( m_nDieRenewFrame == 0 ) m_nDieRenewFrame = 1;
			break;
		case 2:
			if( m_nDieRenewFrame == 0 ) {
				m_hTroop->CmdDie( m_Handle );
				m_nDieRenewFrame = 1;

				if( m_hTroop ) {
					m_hTroop->CmdDie( m_Handle );
				}
			}
			if( m_bIsInScanner == true )
				SetInScanner( false );
			break;
	}
}

bool CFcAdjutantObject::IsDie( bool bCheckEnable )
{
	switch( m_nDieType ) {
		case 0:	
			if( bCheckEnable && !m_bEnable ) return true;
			return m_bDead;
		case 1:	return false;
		case 2:	return ( m_nDieRenewFrame == 0 ) ? false : true;
	}
	return true;
}

bool CFcAdjutantObject::CheckDefense()
{
	if( m_pUnitInfoData->GetAniTypeCount( ANI_ATTR_NONE, ANI_TYPE_DEFENSE ) == 1 )
		return CFcGameObject::CheckDefense();
	else {
		if( m_bDead ) return false;
		if( !m_HitParam.ObjectHandle ) return false;

		// 진오브일 경우 걍 무시!!
		if( g_FcWorld.IsProcessTrueOrb( GetTeam() ) ) {
			ChangeAnimation( ANI_TYPE_DEFENSE, 3, ANI_ATTR_NONE, false );
			return true;
		}

		if( m_HitParam.ObjectHandle->IsGuardBreak( m_Handle ) || Random(100) < m_HitParam.nGuardBreak ) {
			ChangeAnimation( ANI_TYPE_DEFENSE, 4, ANI_ATTR_NONE, true );
			return false;
		}

		bool bResult = false;
		if( m_nCurAniType == ANI_TYPE_DEFENSE ) {
			ChangeAnimation( ANI_TYPE_DEFENSE, 3, ANI_ATTR_NONE, true );
			bResult = true;
		}
		if( bResult == false && Random( 100 ) < GetDefenseProb() ) {
			ChangeAnimation( ANI_TYPE_DEFENSE, 3, ANI_ATTR_NONE, false );
			bResult = true;
		}
		if( bResult == true ) {
			D3DXVECTOR3 vDir = m_HitParam.ObjectHandle->GetPos() - GetPos();
			vDir.y = 0.f;
			D3DXVec3Normalize( &vDir, &vDir );
			m_MoveDir = vDir;

			float fHitVelocityZ = ( float )m_HitParam.fVelocityZ * 0.6f;
			float fHitVelocityY = ( float )m_HitParam.fVelocityY * 0.6f;
			D3DXVECTOR3 Direction = m_HitParam.ObjectHandle->GetCrossVector()->m_PosVector - m_Cross.m_PosVector;

			Direction.y = 0.0f;
			D3DXVec3Normalize( &Direction, &Direction );

			GetExactHitVelocity( fHitVelocityZ, fHitVelocityY );

			SetVelocityX( Direction.x * fHitVelocityZ );
			SetVelocityZ( Direction.z * fHitVelocityZ );
			SetVelocityY( fHitVelocityY );
			return true;
		}
		return bResult;
	}
}

bool CFcAdjutantObject::IsFlockable( GameObjHandle Handle )
{
	if( Handle == m_Handle ) return false;
	if( Handle->GetParentLinkHandle() == m_Handle ) return false;

	switch( Handle->GetClassID() ) {
		case Class_ID_Hero:
			switch( ((HeroObjHandle)Handle)->GetHeroClassID() ) {
				case Class_ID_Hero_VigkVagk:	return true;
			}
			if( GetTeam() == Handle->GetTeam() ) return true;
			return false;
		case Class_ID_Catapult: return true;
		case Class_ID_Adjutant:	return true;
		default:
			break;
	}
	return false;
}

void CFcAdjutantObject::ProcessFlocking()
{
	if( m_bDead ) return;
//	m_nFlockingInterval++;
//	if( m_nFlockingInterval % FLOCKING_INTERVAL == 0 ) {
	float fScanRange = GetUnitRadius() * 4;
	m_VecFlocking.erase( m_VecFlocking.begin(), m_VecFlocking.end() );
	CFcWorld::GetInstance().GetObjectListInRange( (D3DXVECTOR3*)&GetPos(), fScanRange, m_VecFlocking );
//	}

	D3DXVECTOR3 Direction;
	fScanRange = GetUnitRadius();
	for( int i = 0; i <(int)m_VecFlocking.size(); i++ ) {
		if( !CFcGameObject::IsValid( m_VecFlocking[i] ) ) continue;
		if( !m_VecFlocking[i]->IsFlockable( m_Handle ) ) continue;

		Direction = m_VecFlocking[i]->GetPos() - GetPos();
		Direction.y = 0.f;
		float fSumRadius = fScanRange + m_VecFlocking[ i ]->GetUnitRadius();
		float fLength = D3DXVec3LengthSq( &Direction );
		if( fLength > fSumRadius * fSumRadius ) continue;

		float fSpeed = fSumRadius - sqrtf( fLength );

		D3DXVec3Normalize( &Direction, &Direction );
		m_VecFlocking[i]->Move( &( Direction * fSpeed ) );
		m_VecFlocking[i]->SetFlocking( true, fSumRadius );
	}
}

bool CFcAdjutantObject::CalculateDamage(int nAttackPower)
{
	if( m_fOnlyPlayerDamage > 0.f ) {
		float fHP = m_fHP;
		bool bResult = CFcGameObject::CalculateDamage( nAttackPower );
		if( bResult == true ) {
			if( m_fHP < (float)(GetMaxHP() * m_fOnlyPlayerDamage) ) {
				if( IsPlayerDamage() == false )
					m_fHP = fHP;
			}
			return true;
		}
		return false;
	}
	return CFcGameObject::CalculateDamage( nAttackPower );
}