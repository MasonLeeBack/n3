#include "StdAfx.h"
#include "FcArcherUnitObject.h"
#include "ASData.h"
#include "BsKernel.h"
#include "FcAIObject.h"
#include "FcAISearchSlot.h"
#include "FcProp.h"
#include "./Data/SignalType.h"
#include "FcProjectile.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


struct BoneRotationStruct {
	char *szBoneName;
	int nBoneIndex;
	float fRatio;
};

BoneRotationStruct g_szArcherRotateBoneList[] = {
	{ "Bip01 Spine", -1, 0.333333f },
	{ "Bip01 Spine1", -1, 0.333333f },
	{ "Bip01 Spine2", -1, 0.333333f },
	NULL,
};

BoneRotationStruct g_szArcherPitchBoneList[] = {
	{ "Bip01 Spine2", -1, 0.2f },
	{ "Bip01 L Clavicle", -1, 0.8f },
	{ "Bip01 R Clavicle", -1, 0.6f },
	NULL,
};

CFcArcherUnitObject::CFcArcherUnitObject(CCrossVector *pCross)
: CFcUnitObject( pCross )
{
	SetClassID( Class_ID_Archer );
	m_bApplyTracking = false;
	m_bApplyPitching = false;

	m_fCurPitchingAngle = 0.f;
	m_fCurRotationAngle = 0.f;
	m_nDieCount = -1;
	m_nHitProb = 50;
	m_nHitProbAdd = 0;
}


CFcArcherUnitObject::~CFcArcherUnitObject()
{
}

/*
void CFcArcherUnitObject::Initialize( INITGAMEOBJECTDATA* pData, int nTeam, int nGroup, TroopObjHandle hTroop, float fScaleMin, float fScaleMax )
{
}
void CFcArcherUnitObject::Initialize( int nUnitIndex, int nTeam, int nGroup, TroopObjHandle hTroop, float fScaleMin, float fScaleMax )
{
}
*/


bool CFcArcherUnitObject::Render()
{
	bool bRet;

	bRet = CFcUnitObject::Render();
	if( !bRet )
	{
		return false;
	}
	g_BsKernel.SendMessage( m_nEngineIndex, BS_SET_CLEAR_BONE_ROTATION );
	if( m_bApplyTracking || m_fCurRotationAngle != 0.f ) {
		D3DXVECTOR3 vRotate;
		for( DWORD i=0; ; i++ ) {
			char *szBoneName = g_szArcherRotateBoneList[i].szBoneName;
			if( szBoneName == NULL ) break;

			vRotate = D3DXVECTOR3( 0.f, m_fCurRotationAngle * g_szArcherRotateBoneList[i].fRatio, 0.f );
			g_BsKernel.SendMessage( m_nEngineIndex, BS_SET_BONE_ROTATION, (DWORD)szBoneName, (DWORD)&vRotate );
		}
	}

	if( m_bApplyPitching || m_fCurPitchingAngle != 0.f ) {
		D3DXVECTOR3 vRotate;
		for( DWORD i=0; ; i++ ) {
			char *szBoneName = g_szArcherPitchBoneList[i].szBoneName;
			if( szBoneName == NULL ) break;

			vRotate = D3DXVECTOR3( m_fCurPitchingAngle * g_szArcherPitchBoneList[i].fRatio, 0.f, 0.f );
			g_BsKernel.SendMessage( m_nEngineIndex, BS_SET_BONE_ROTATION, (DWORD)szBoneName, (DWORD)&vRotate );
		}	
	}

	return true;
}

#include "FcGlobal.h"
void CFcArcherUnitObject::Process()
{
	if( IsOnProp() == true ) {
		switch( m_nDieCount ) {
			case -1:
				{
					/*
					if( m_bDead == true ) {
						m_nDieCount = 0;
						SetFreezePos( false );
					}
					CFcProp *pProp = (CFcProp*)m_nOnPropIndex;
					if( pProp->IsBreak() ) {
						m_nDieCount = 0;
						SetFreezePos( false );

						HIT_PARAM HitParam;
						HitParam.ObjectHandle = m_Handle;
						HitParam.nAttackPower = 0;
						HitParam.pHitSignal = g_FCGameData.GetDefaultProjectileHitSignal( -1 );
						HitParam.Position = m_Cross.m_PosVector - D3DXVECTOR3( 10, 10, 10 );
						HitParam.nHitRemainFrame = 6;
						CmdHit( &HitParam );
					}
					*/
					bool bChecked = false;
					if( ( GetCurAniType() & 0x00FF ) == ANI_TYPE_DIE && IsRagDollMode() == RAGDOLL_MODE_NONE ) bChecked = true;
					CFcProp *pProp = (CFcProp*)m_nOnPropIndex;
					if( pProp->IsBreak() ) bChecked = true;

					if( bChecked == true ) {
						m_nDieCount = 0;
						RunRagdoll();
						SetRagDollMode( RAGDOLL_MODE_FREE );
						SetDie();
					}
				}
				break;
			case 2:
				g_BsKernel.SendMessage( m_nEngineIndex, BS_PHYSICS_SIMULATION, TRUE );
				break;
		}
		if( m_nDieCount != -1 ) m_nDieCount++;
	}
	CFcUnitObject::Process();
}

void CFcArcherUnitObject::PostProcess()
{
	if( IsSubJob() == false && GetCurAniAttr() == ANI_ATTR_BATTLE ) {
		switch( GetCurAniType() & 0x00FF ) {
			case ANI_TYPE_STAND:
			case ANI_TYPE_ATTACK:
				m_bApplyTracking = true;
				m_bApplyPitching = true;
				break;
		}
	}
	else {
		m_bApplyTracking = false;
		m_bApplyPitching = false;
	}

	ProcessTargetTracking();
	ProcessTargetPitching();


	CFcUnitObject::PostProcess();
}

void CFcArcherUnitObject::ProcessTargetTracking()
{
	if( !m_bApplyTracking && m_fCurRotationAngle != 0.f ) {
		if( m_fCurRotationAngle > 0.f ) {
			m_fCurRotationAngle -= SPINE_ROTATION_ANGLE;
			if( m_fCurRotationAngle > 0.f ) m_fCurRotationAngle = 0.f;
		}
		else {
			m_fCurRotationAngle += SPINE_ROTATION_ANGLE;
			if( m_fCurRotationAngle < 0.f ) m_fCurRotationAngle = 0.f;
		}
	}
}

void CFcArcherUnitObject::ProcessTargetPitching()
{
	D3DXVECTOR3 vTemp1, vTemp2;
	D3DXVECTOR3 vVec1, vVec2;

	GameObjHandle hUnit;
	bool bResult = true;
	if( GetAIHandle() ) {
		GameObjHandle *pSearchHandle = GetAIHandle()->GetSearchSlot()->GetHandle(0);
		if( pSearchHandle )
			hUnit = *pSearchHandle;
		else bResult = false;
	}
	else bResult = false;

	if( !CFcBaseObject::IsValid( hUnit ) ) bResult = false;
	if( bResult == false || ( !m_bApplyTracking && m_fCurPitchingAngle != 0.f ) ) {
		if( m_fCurPitchingAngle > 0.f ) {
			m_fCurPitchingAngle -= SPINE_PITCHING_ANGLE;
			if( m_fCurPitchingAngle < 0.f ) m_fCurPitchingAngle = 0.f;
		}
		if( m_fCurPitchingAngle < 0.f ) {
			m_fCurPitchingAngle += SPINE_PITCHING_ANGLE;
			if( m_fCurPitchingAngle > 0.f ) m_fCurPitchingAngle = 0.f;
		}
		return;
	}

	vTemp1 = GetPos();			vTemp1.y = 0.f;
	vTemp2 = hUnit->GetPos();	vTemp2.y = 0.f;
	vVec1 = vTemp1 - vTemp2;

	vTemp1 = GetPos();			vTemp1.x = vTemp1.z = 0.f;
	vTemp2 = hUnit->GetPos();	vTemp2.x = vTemp2.z = 0.f;
	vVec2 = vTemp1 - vTemp2;
	float fHeight = GetPos().y - hUnit->GetPos().y;

	D3DXVECTOR3 vResult = vVec1 + vVec2;

	if( D3DXVec3LengthSq( &vResult ) <= 0.f || D3DXVec3LengthSq( &vVec1 ) <= 0.f ) return;

	D3DXVec3Normalize( &vResult, &vResult );
	D3DXVec3Normalize( &vVec1, &vVec1 );
	float fDot = D3DXVec3Dot( &vResult, &vVec1 );
	float fAngle = acos(fDot) * 180.f / 3.1415926f;
	if( fAngle > MAX_SPINE_PITCHING_ANGLE ) fAngle = MAX_SPINE_PITCHING_ANGLE;

	if( fHeight <= 0.f ) {
		float fLength = D3DXVec3Length( (D3DXVECTOR3*)&(GetPos() - hUnit->GetPos() ) );
		if( fLength > 3000.f )
			fAngle = -fAngle - PITCHING_MIN_UP_ANGLE;
		else fAngle = -fAngle - ( PITCHING_MIN_UP_ANGLE / 3000.f ) * fLength;
	}
	else {
		fAngle -= PITCHING_MIN_DOWN_ANGLE;
	}

	if( m_fCurPitchingAngle > fAngle ) {
		m_fCurPitchingAngle -= SPINE_PITCHING_ANGLE;
		if( m_fCurPitchingAngle < fAngle ) m_fCurPitchingAngle = fAngle;
	}
	if( m_fCurPitchingAngle < fAngle ) {
		m_fCurPitchingAngle += SPINE_PITCHING_ANGLE;
		if( m_fCurPitchingAngle > fAngle ) m_fCurPitchingAngle = fAngle;
	}
}

void CFcArcherUnitObject::RotationProcess()
{
	if( m_bApplyTracking == true ) {
		float fDot, fLength;
		D3DXVECTOR3 CrossVec;

		fLength=D3DXVec3LengthSq(&m_MoveDir);
		if(fLength<=0.0f){
			return;
		}  

		fDot=D3DXVec3Dot( &m_MoveDir, &m_Cross.m_ZVector );
		float fAngle = acos( fDot ) * 180.f / 3.1415926f;
		if( fAngle > MAX_SPINE_ROTATION_ANGLE ){
			D3DXVec3Cross(&CrossVec, &m_MoveDir, &(m_Cross.m_ZVector));
			if( CrossVec.y > 0 ){
				m_Cross.RotateYaw( 20 );
			}
			else{
				m_Cross.RotateYaw( -20 );
			}
		}
		else {
			D3DXVec3Cross( &CrossVec, &m_MoveDir, &m_Cross.m_ZVector );
			if( CrossVec.y > 0.f ){
				m_fCurRotationAngle -= SPINE_ROTATION_ANGLE;
				if( m_fCurRotationAngle < -fAngle ) m_fCurRotationAngle = -fAngle;
			}
			else{
				m_fCurRotationAngle += SPINE_ROTATION_ANGLE;
				if( m_fCurRotationAngle > fAngle ) m_fCurRotationAngle = fAngle;
			}
		}
	}
	else {
		CFcUnitObject::RotationProcess();
		m_fCurRotationAngle = 0.f;
	}
}


void CFcArcherUnitObject::SignalCheck( ASSignalData *pSignal )
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
				float fAddProbRatio = ((float)m_nHitProbAdd / 100.f);
				Param.nHitProb = m_nHitProb + (int)(fAddProbRatio * (float)m_nHitProb);
				pPartsData = m_pUnitInfoData->GetPartsGroupInfo( m_WeaponList[ pSignal->m_pParam[ 0 ] ].nPartsIndex );
				if( CFcBaseObject::IsValid( Param.hTarget ) == true )
				{
					Handle = CFcProjectile::CreateObject( CFcProjectile::ARROW, &Cross );
					Handle->Initialize( pPartsData->m_pSkinIndex[0], &Param );
				}
			}
			return;
		case SIGNAL_TYPE_CUSTOM_MESSAGE:
			if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
			{
				switch( pSignal->m_pParam[0] ) {
					case 0:
						if( GetAIHandle() )
							GetAIHandle()->GetSearchSlot()->SetHandle( 0, NULL );
						break;
				}
			}
			return;
	}

	CFcUnitObject::SignalCheck( pSignal );
}

bool CFcArcherUnitObject::IsDefensible()
{
	if( IsSubJob() == false ) return false;
	return CFcUnitObject::IsDefensible();
}
void CFcArcherUnitObject::SetHitProbAdd( int nPer )
{
	m_nHitProbAdd += nPer;
}

void CFcArcherUnitObject::SetHitProb( int nProb )
{
	m_nHitProb = nProb;

}
