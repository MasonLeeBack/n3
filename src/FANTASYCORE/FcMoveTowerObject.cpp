#include "StdAfx.h"
#include "FcMoveTowerObject.h"
#include "FcUnitObject.h"
#include "FcWorld.h"
#include "DistVec3Lin3.h"
#include "IntLin3Box3.h"
#include "FcVelocityCtrl.h"
#include "FcAIObject.h"
#include "FcCatapultSeqEvent.h"
#include "FcTroopObject.h"
#include "FcGlobal.h"
#include "FcWorld.h"
#include "./Data/SignalType.h"
#include "FcProjectile.h"
#include "DebugUtil.h"
#include "FcMoveTowerSeqEvent.h"
#include "FcTroopManager.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


float CFcMoveTowerObject::s_fRotationAngle = 0.3f;

char *CFcMoveTowerObject::s_szWheelBoneName[] = {
	"BoxBone02",
	"BoxBone03",
	"BoxBone04",
	"BoxBone05",

	"BoxBone06",
	"BoxBone08",
	"BoxBone07",
	"BoxBone09",

	"BoxBone12",
	"BoxBone11",
	"BoxBone13",
	"BoxBone10",
};


CFcMoveTowerObject::CFcMoveTowerObject( CCrossVector *pCross )
: CFcUnitObject( pCross )
{
	SetClassID( Class_ID_MoveTower );

	m_pVelocityCtrl = new CFcVelocityCtrl( 0.7f, 10.f, 0.7f, 1.02f, 0.96f );
	m_pRotationCtrl[0] = new CFcVelocityCtrl( 0.1f, s_fRotationAngle, 0.1f, 1.02f, 0.96f );
	m_pRotationCtrl[1] = new CFcVelocityCtrl( 0.1f, s_fRotationAngle, 0.1f, 1.02f, 0.96f );
	m_bStopFlag = true;
	m_fCurMaximumSpeed = 0.f;
	m_nRotationAniIndex = -1;

	memset( m_fWheelAngle, 0, sizeof(m_fWheelAngle) );
	m_bWorkable = true;
}

CFcMoveTowerObject::~CFcMoveTowerObject()
{
	SAFE_DELETE( m_pRotationCtrl[0] );
	SAFE_DELETE( m_pRotationCtrl[1] );
	SAFE_DELETE( m_pVelocityCtrl );
}


void CFcMoveTowerObject::Initialize( INITGAMEOBJECTDATA* pData, int nForce, int nTeam, TroopObjHandle hTroop, float fScaleMin, float fScaleMax )
{
	CFcGameObject::Initialize( pData, nForce, -1, hTroop, fScaleMin, fScaleMax );

	float fMaximumSpeed = (float)m_pUnitInfoData->GetMoveSpeed( m_pUnitInfoData->GetAniIndex( ANI_TYPE_RUN, 0 ) );
	m_pVelocityCtrl->SetMaximumSpeed( fMaximumSpeed );

	g_BsKernel.SendMessage( m_nEngineIndex, BS_USE_ANI_CACHE, 0 );
}

void CFcMoveTowerObject::Process()
{
	CFcGameObject::Process();
	if( m_bWorkable == false ) {
		ProcessFlush();
		return;
	}
	ProcessMove();
	ProcessWheelSpeed();

	for( DWORD i=0; i<m_VecLinkObjHandle.size(); i++ ) {
		if( m_VecLinkObjHandle[i]->IsEnable() )
			m_VecLinkObjHandle[i]->Enable( false );
	}
}

void CFcMoveTowerObject::PostProcess()
{
	CFcUnitObject::PostProcess();
}

bool CFcMoveTowerObject::Render()
{
	bool bRet;

	g_BsKernel.SendMessage( m_nEngineIndex, BS_SET_CLEAR_BONE_ROTATION );
	bRet = CFcUnitObject::Render();
	if( !bRet )
	{
		return false;
	}
	// Wheel
	for( int i=0; i<12; i++ ) {
		D3DXVECTOR3 vRotate = D3DXVECTOR3( m_fWheelAngle[i], 0.f, 0.f );
		g_BsKernel.SendMessage( m_nEngineIndex, BS_SET_BONE_ROTATION, (DWORD)s_szWheelBoneName[i], (DWORD)&vRotate );
	}

	return true;
}

void CFcMoveTowerObject::ProcessWheelSpeed()
{
	switch( m_nCurAniType ) {
		case ANI_TYPE_STAND:
		case ANI_TYPE_RUN:
			if( m_nRotationAniIndex == -1 ) {
				if( m_pVelocityCtrl->GetCurSpeed() <= m_pVelocityCtrl->GetMinSpeed() ) break;
				for( int i=0; i<4; i++ ) {
					m_fWheelAngle[i] += ( m_pVelocityCtrl->GetCurSpeed() * 0.5f );
				}
			}
			else {
				float fTemp[2];
				fTemp[0] = m_pRotationCtrl[0]->GetCurSpeed();
				if( fTemp[0] <= m_pRotationCtrl[0]->GetMinSpeed() ) fTemp[0] = 0.f;
				fTemp[1] = m_pRotationCtrl[1]->GetCurSpeed();
				if( fTemp[1] <= m_pRotationCtrl[1]->GetMinSpeed() ) fTemp[1] = 0.f;
				fTemp[0] -= fTemp[1];

				m_fWheelAngle[4] -= ( fTemp[0] * 4.0f );
				m_fWheelAngle[5] -= ( fTemp[0] * 4.0f );
				m_fWheelAngle[6] -= ( fTemp[0] * 4.0f );
				m_fWheelAngle[7] -= ( fTemp[0] * 4.0f );

				m_fWheelAngle[8] += ( fTemp[0] * 4.0f );
				m_fWheelAngle[9] += ( fTemp[0] * 4.0f );
				m_fWheelAngle[10] += ( fTemp[0] * 4.0f );
				m_fWheelAngle[11] += ( fTemp[0] * 4.0f );
			}
			break;
	}
}



bool CFcMoveTowerObject::IsNeedNewWallTroop()
{
	int nCnt = m_VecWallTroop.size();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTroop = m_VecWallTroop[i];
		if( ((CFcWallEnemyTroop*)(hTroop.m_pInstance))->IsAttackOnWall() == true )
		{
			return false;
		}
	}
	return true;
}

void CFcMoveTowerObject::AddNewWallTroop()
{
	D3DXVECTOR2 Pos = GetPosV2();

	TROOP_INFO Info;
	Info.m_troopAttr.nTroopType = TROOPTYPE_WALLENEMY;
	Info.m_areaInfo.fSX = Pos.x - 200.f;
	Info.m_areaInfo.fSZ = Pos.y - 200.f;
	Info.m_areaInfo.fEX = Pos.x + 200.f;
	Info.m_areaInfo.fEZ = Pos.y + 200.f;
	memset( Info.m_troopAttr.nUnitType, -1, sizeof( Info.m_troopAttr.nUnitType ) );
	Info.m_troopAttr.nLeaderUnitType = -1;
	memset( Info.m_troopAttr.nUnitHorseType, -1, sizeof( Info.m_troopAttr.nUnitHorseType ) );
	Info.m_troopAttr.nLeaderHorseType = -1;
	Info.m_troopAttr.nUnitAI = -1;

	Info.m_troopAttr.nUnitType[0] = 28;
	Info.m_troopAttr.nEnable = TRUE;
	Info.m_troopAttr.nTeam = 1;
	Info.m_troopAttr.nGroup = 1;
	Info.m_troopAttr.nUnitCol = 6;
	Info.m_troopAttr.nUnitRow = 6;

	TroopObjHandle hTroop = g_FcWorld.GetTroopmanager()->AddTroop( &g_FcWorld, &Info );
//	TroopObjHandle hTroop = g_FcWorld.GetTroopmanager()->InitTroop( TROOPTYPE_WALLENEMY, &Info, (int)Pos.x, (int)Pos.y, 100, 30 );
	if( !hTroop ) return;
	
	m_VecWallTroop.push_back( hTroop );
}

void CFcMoveTowerObject::ProcessFlush()
{
	if( IsNeedNewWallTroop() )
	{
		AddNewWallTroop();
	}
}

void CFcMoveTowerObject::ProcessMove()
{
	float fSpeed = 0.f; 
	float fCurSpeed = m_pVelocityCtrl->GetCurSpeed();


	D3DXVECTOR3 vPos = GetDummyPos() - m_MoveTargetPos;
	vPos.y = 0.f;

	float fLength = D3DXVec3Length( &vPos );

	float fDistance = 0.f;
	if( m_bApplyFlocking == false ) fDistance = 0.f;
	else {
		fDistance = m_fMaxFlockingSize;
	}

	if( fLength <= GetStopableDistance() + fDistance ) {
		CmdStop( ANI_TYPE_STOP );
		m_fCurMaximumSpeed = 0.f;
		m_bStopFlag = true;
		fCurSpeed = 0.f;
		m_pVelocityCtrl->SetCurSpeed( 0.f );
	}
	else {
		if( m_bStopFlag == false ) {
			m_fCurMaximumSpeed = (float)m_pUnitInfoData->GetMoveSpeed( m_pUnitInfoData->GetAniIndex( ANI_TYPE_RUN, 0 ) );
			if( m_nRotationAniIndex != -1 )
				m_fCurMaximumSpeed = 0.f;
		}
		else {
			m_fCurMaximumSpeed = 0.f;
		}
	}
	if( fCurSpeed > m_fCurMaximumSpeed ) {
		fSpeed = m_pVelocityCtrl->GetSpeed( CFcVelocityCtrl::DECELERATION );
		if( fSpeed <= m_pVelocityCtrl->GetMinSpeed() ) {
			fSpeed = 0.f;
		}
	}
	else if( fCurSpeed < m_fCurMaximumSpeed ) {
		fSpeed = m_pVelocityCtrl->GetSpeed( CFcVelocityCtrl::ACCELERATION );
		if( fSpeed >= m_fCurMaximumSpeed ) {
			m_pVelocityCtrl->SetCurSpeed( m_fCurMaximumSpeed );
			fSpeed = m_fCurMaximumSpeed;
		}
	}
	else fSpeed = m_fCurMaximumSpeed;

	MoveZ( fSpeed );

	if( fSpeed == 0.f && m_nRotationAniIndex == -1 ) {
		/*
		if( m_pEventSeq->GetEventCount() == 0 ) {
			ChangeAnimation( ANI_TYPE_STAND, 0 );
			m_nBlendFrame = 0;
		}
		*/
	}
	else {
		ChangeAnimation( ANI_TYPE_RUN, 0 );
	}
}

void CFcMoveTowerObject::MoveZ(float fDist)
{
	m_MoveVector+=m_Cross.m_ZVector*fDist; 
}

void CFcMoveTowerObject::CmdMove( int nX, int nY, float fSpeed, int nAniType, int nAniIndex )
{
	if( m_bDead ) return;
	if( !IsMovable() )
	{
		return;
	}

	D3DXVECTOR3 vTemp = D3DXVECTOR3( (float)nX, 0.f, (float)nY );
	D3DXVECTOR3 Pos = m_Cross.GetPosition();

	m_MoveTargetPos = vTemp;
	m_MoveDir = vTemp - D3DXVECTOR3( Pos.x, 0.f, Pos.z );
	D3DXVec3Normalize(&m_MoveDir, &m_MoveDir);

	SetFlocking( false );
	m_bStopFlag = false;
}

void CFcMoveTowerObject::CmdStop( int nAniType, int nAniIndex )
{
	m_bStopFlag = true;
	if( IsMove() ) {
		ChangeAnimation( nAniType & 0x00FF, nAniIndex, nAniType & 0xFF00 );
	}
	if( D3DXVec3LengthSq( &m_MoveDir ) > 0.f )
		m_MoveDir = D3DXVECTOR3( 0.f, 0.f, 0.f );

}

void CFcMoveTowerObject::RotationProcess()
{
	float fLength;
	D3DXVECTOR3 CrossVec;

	//	if( m_pVelocityCtrl->GetCurSpeed() > m_pVelocityCtrl->GetMinSpeed() ) return;
	//////////////////////////////////////////////////
	float fAngle = 0.f;
	switch( m_nRotationAniIndex ) {
		case -1:
			m_pRotationCtrl[0]->GetSpeed( CFcVelocityCtrl::DECELERATION );
			m_pRotationCtrl[1]->GetSpeed( CFcVelocityCtrl::DECELERATION );
			break;
		case 0: 
			m_pRotationCtrl[0]->GetSpeed( CFcVelocityCtrl::ACCELERATION );	
			m_pRotationCtrl[1]->GetSpeed( CFcVelocityCtrl::DECELERATION );
			break;
		case 1: 
			m_pRotationCtrl[0]->GetSpeed( CFcVelocityCtrl::DECELERATION );
			m_pRotationCtrl[1]->GetSpeed( CFcVelocityCtrl::ACCELERATION );	
			break;
	}
	float fTemp[2];
	fTemp[0] = m_pRotationCtrl[0]->GetCurSpeed();
	if( fTemp[0] <= m_pRotationCtrl[0]->GetMinSpeed() ) fTemp[0] = 0.f;
	fTemp[1] = m_pRotationCtrl[1]->GetCurSpeed();
	if( fTemp[1] <= m_pRotationCtrl[1]->GetMinSpeed() ) fTemp[1] = 0.f;
	fAngle = fTemp[0] - fTemp[1];

	if( fAngle != 0.f ) {
		D3DXVECTOR3 Rotate;
		float fTemp = fAngle * 3.1415926f / 180.f;
		Rotate.x=-sin(fTemp)*m_Cross.m_XVector.x+cos(fTemp)*m_Cross.m_ZVector.x;
		Rotate.y=-sin(fTemp)*m_Cross.m_XVector.y+cos(fTemp)*m_Cross.m_ZVector.y;
		Rotate.z=-sin(fTemp)*m_Cross.m_XVector.z+cos(fTemp)*m_Cross.m_ZVector.z;
		m_Cross.m_ZVector=Rotate;
		D3DXVec3Normalize(&m_Cross.m_ZVector, &m_Cross.m_ZVector);
		D3DXVec3Cross(&m_Cross.m_XVector, &m_Cross.m_YVector, &m_Cross.m_ZVector);
		D3DXVec3Normalize(&m_Cross.m_XVector, &m_Cross.m_XVector);
	}
	//////////////////////////////////////////////////


	fLength=D3DXVec3Length(&m_MoveDir);
	if(fLength<=0.0f){
		float fMaximumSpeed = (float)m_pUnitInfoData->GetMoveSpeed( m_pUnitInfoData->GetAniIndex( ANI_TYPE_RUN, 0 ) );
		m_nRotationAniIndex = -1;
		return;
	}

	//	/*
	fLength = D3DXVec3Length( &m_MoveTargetPos );
	if( fLength > 0.f ) {
		D3DXVECTOR3 vTargetVec = m_MoveTargetPos - GetDummyPos();
		vTargetVec.y = 0.f;
		D3DXVec3Normalize( &vTargetVec, &vTargetVec );
		m_MoveDir = vTargetVec;
	}
	//	*/

	float fDot=D3DXVec3Dot(&m_MoveDir, &(m_Cross.m_ZVector));
	fAngle = acos( fDot ) * 180.f / 3.1415926f;

	if( fAngle <= fabs( GetStopableAngle() ) ) {
		m_nRotationAniIndex = -1;
		if( fAngle < s_fRotationAngle ) {
			m_Cross.m_ZVector=m_MoveDir;
			m_Cross.UpdateVectors();
		}
		return;
	}
	else {
		if( fAngle > s_fRotationAngle ) {
			D3DXVec3Cross(&CrossVec, &m_MoveDir, &(m_Cross.m_ZVector));
			if(CrossVec.y>0){
				m_nRotationAniIndex = 0;
			}
			else{
				m_nRotationAniIndex = 1;
			}
			fDot=D3DXVec3Dot(&m_MoveDir, &(m_Cross.m_ZVector));
			fAngle = acos( fDot ) * 180.f / 3.1415926f;
			if( fAngle < s_fRotationAngle ) {
				m_Cross.m_ZVector=m_MoveDir;
				m_Cross.UpdateVectors();

				m_nRotationAniIndex = -1;
			}
			return;
		}
	}

	m_nRotationAniIndex = -1;

	m_Cross.m_ZVector=m_MoveDir;
	m_Cross.UpdateVectors();
	m_MoveDir=D3DXVECTOR3(0.0f, 0.0f, 0.0f);
}

void CFcMoveTowerObject::PositionProcess()
{
	CFcUnitObject::PositionProcess();
}

bool CFcMoveTowerObject::IsHittable( GameObjHandle Handle )
{
	return false;
}

void CFcMoveTowerObject::ProcessFlocking()
{
	/*
	if( m_bDead ) return;

	m_Collision.C = *(BSVECTOR*)&m_Cross.m_PosVector;
	m_Collision.A[0] = *(BSVECTOR*)&m_Cross.m_XVector;
	m_Collision.A[1] = *(BSVECTOR*)&m_Cross.m_YVector;
	m_Collision.A[2] = *(BSVECTOR*)&m_Cross.m_ZVector;
	m_Collision.E[0] = GetUnitRadius() + 50.f;
	m_Collision.E[1] = 3000.f;
	m_Collision.E[2] = GetUnitRadius() + 100.f;

	m_nFlockingInterval++;
	if( m_nFlockingInterval % FLOCKING_INTERVAL == 0 ) {
		float fScanRange = GetUnitRadius() * 4;
		m_VecFlocking.erase( m_VecFlocking.begin(), m_VecFlocking.end() );
		CFcWorld::GetInstance().GetObjectListInRange( (D3DXVECTOR3*)&GetPos(), fScanRange, m_VecFlocking );
	}

	D3DXVECTOR3 vVec[5];
	float fTempLength, fLength;
	D3DXVECTOR3 vTempResult, vResult;
	D3DXVECTOR3 vCross, vTemp, vNor;

	m_Collision.compute_vertices();

	memcpy( &vVec[0].x, &m_Collision.V[0].x, sizeof(D3DXVECTOR3) );
	memcpy( &vVec[1].x, &m_Collision.V[1].x, sizeof(D3DXVECTOR3) );
	memcpy( &vVec[2].x, &m_Collision.V[5].x, sizeof(D3DXVECTOR3) );
	memcpy( &vVec[3].x, &m_Collision.V[4].x, sizeof(D3DXVECTOR3) );
	memcpy( &vVec[4].x, &m_Collision.V[0].x, sizeof(D3DXVECTOR3) );

	for( int i = 0; i <(int)m_VecFlocking.size(); i++ ) {
		if( !CFcGameObject::IsValid( m_VecFlocking[i] ) ) continue;
		if( !m_VecFlocking[i]->IsFlockable( m_Handle ) ) continue;

		if( TestIntersection( m_VecFlocking[i]->GetPos(), m_Collision ) == false ) continue;

		bool bFirst = true;
		vTemp = m_VecFlocking[i]->GetPos();
		vTemp.y = 0.f;
		for( int j=0; j<4; j++ ) {
			vNor = vVec[j] - vVec[j+1];
			vNor.y = 0.f;
			D3DXVec3Normalize( &vNor, &vNor );
			D3DXVec3Cross( &vCross, &vNor, &D3DXVECTOR3( 0.f, 1.f, 0.f ) );
			D3DXVec3Normalize( &vCross, &vCross );

			vCross = m_VecFlocking[i]->GetPos() + ( vCross * 1000.f );
			if( GetIntersectPoint( vVec[j], vVec[j+1], vTemp, vCross, vTempResult ) )  {
				vTempResult.y = 0.f;
				fTempLength = D3DXVec3Length( &( vTemp - vTempResult ) );
				if( bFirst == true || fLength > fTempLength ) {
					bFirst = false;
					fLength = fTempLength;
					vResult = vTempResult;
				}
			}
		}
		if( bFirst == false ) {
			m_VecFlocking[i]->Move( &( vResult - vTemp ) );
		}
	}
	*/
}

float CFcMoveTowerObject::GetStopableDistance()
{
	float fDistance = 0.f;
	float fTemp = m_pVelocityCtrl->GetCurSpeed();
	while(1) {
		fTemp *= m_pVelocityCtrl->GetDeceleration();
		fDistance += fTemp;
		if( fTemp < m_pVelocityCtrl->GetMinSpeed() ) break;
	}
	return fDistance;
}

float CFcMoveTowerObject::GetStopableAngle()
{
	float fAngle = 0.f;
	float fTemp[2] = { m_pRotationCtrl[0]->GetCurSpeed(), m_pRotationCtrl[1]->GetCurSpeed() };

	while(1) {
		fTemp[0] *= m_pRotationCtrl[0]->GetDeceleration();
		if( fTemp[0] <= m_pRotationCtrl[0]->GetMinSpeed() ) fTemp[0] = 0.f;
		fTemp[1] *= m_pRotationCtrl[1]->GetDeceleration();
		if( fTemp[1] <= m_pRotationCtrl[1]->GetMinSpeed() ) fTemp[1] = 0.f;

		fAngle += fTemp[0] - fTemp[1];
		if( fTemp[0] == 0.f && fTemp[1] == 0.f ) break;
		if( fAngle == 0.f ) break;
	}
	return fAngle;
}

bool CFcMoveTowerObject::LinkMe( GameObjHandle Handle, int nPlayerIndex /*= -1*/ )
{
	for( DWORD i=0; i<m_VecLinkObjHandle.size(); i++ ) {
		if( m_VecLinkObjHandle[i] == Handle ) return false;
	}

	if( m_VecLinkObjHandle.size() > 10 ) return false;
	if( Handle->GetParentLinkHandle() ) return false;

	Handle->SetParentLinkHandle( m_Handle );
	Handle->SetAniPosToLocalPosFlag( BS_CALC_POSITION_X | BS_CALC_POSITION_Z );
	char *szBoneName = "Dummy_GB";
	g_BsKernel.SendMessage( m_nEngineIndex, BS_LINKOBJECT_NAME, ( DWORD )szBoneName, Handle->GetEngineIndex() );
	DebugString( "Link : %d, Bone : %s\n", Handle->GetEngineIndex(), szBoneName );

	Handle->Enable( false );
	m_VecLinkObjHandle.push_back( Handle );

	return true;
}

bool CFcMoveTowerObject::UnLinkMe( GameObjHandle Handle )
{
	int nIndex = -1;
	for( DWORD i=0; i<m_VecLinkObjHandle.size(); i++ ) {
		if( m_VecLinkObjHandle[i] == Handle ) {
			nIndex = (int)i;
			break;
		}
	}
	if( nIndex == -1 ) return false;

	g_BsKernel.SendMessage( m_nEngineIndex, BS_UNLINKOBJECT, Handle->GetEngineIndex() );
	DebugString( "Unlink : %d\n", Handle->GetEngineIndex() );
	Handle->SetAniPosToLocalPosFlag( BS_CALC_POSITION_Y );

	GameObjHandle hIdentity;
	m_VecLinkObjHandle.erase( m_VecLinkObjHandle.begin() + nIndex );

	Handle->Enable( true );
	CmdStop();

	return true;
}


void CFcMoveTowerObject::DebugRender()
{
#ifndef _XBOX
	m_Collision.compute_vertices();
	C3DDevice* pDevice = g_BsKernel.GetDevice();
	pDevice->SaveState();
	pDevice->BeginScene();

	D3DXMATRIX mat, matWorld;
	pDevice->GetTransform( D3DTS_WORLD, &matWorld );
	D3DXMatrixIdentity( &mat );
	pDevice->SetTransform( D3DTS_WORLD, &mat );

	pDevice->SetPixelShader(NULL);
	pDevice->SetVertexShader(NULL);
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, false);
	pDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
	for( DWORD i=0; i<8; i++ ) pDevice->SetTexture( i, NULL );
	pDevice->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );


	struct LINE_VERTEX
	{
		D3DXVECTOR3 v;
		DWORD       color;
	};
	static LINE_VERTEX pVertices[8];


	pDevice->SetTransform( D3DTS_WORLD, &matWorld );
	pDevice->EndScene();
	pDevice->RestoreState();
#endif //_XBOX
}


bool CFcMoveTowerObject::IsFlockable( GameObjHandle Handle )
{
	if( m_Handle == Handle ) return false;
	switch( Handle->GetClassID() ) {
		case Class_ID_Catapult: return true;
		case Class_ID_MoveTower: return true;	
		default:
			break;
	}
	return false;
}

void CFcMoveTowerObject::SignalCheck( ASSignalData *pSignal )
{
	switch( pSignal->m_nID ) {
		case SIGNAL_TYPE_CUSTOM_MESSAGE:
			if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
			{
				switch( pSignal->m_pParam[0] ) {
					case 0:
						{
							// 부대나오기 시작 시점
							AddNewWallTroop();
							m_bWorkable = false;

							/*
							CFcMoveTowerSeqGetOffEvent *pElement = new CFcMoveTowerSeqGetOffEvent( m_pEventSeq );
							m_pEventSeq->AddEvent( pElement );
							*/
						}
						break;
				}
			}
			return;

	}
	CFcUnitObject::SignalCheck( pSignal );
}