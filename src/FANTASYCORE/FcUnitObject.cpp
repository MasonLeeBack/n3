#include "stdafx.h"
#include "FcUnitObject.h"
#include "ASData.h"
#include ".\\data\\AnimType.h"
#include "ShortPathFinder.h"
#include "FcSOXLoader.h"
#include "FcTroopObject.h"
#include "FcGlobal.h"
#include "FcWorld.h"
#include "DebugUtil.h"
#include "FcHeroObject.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG




CFcUnitObject::CFcUnitObject(CCrossVector *pCross) : CFcGameObject( pCross )
{
	SetClassID( Class_ID_Unit );
	m_fMoveSpeed = 5.f;
	m_bUsePathFind = true;

	m_pSPathFind		= new CShortPathFinder( this );
	m_bCheckMoveLock	= false;
	m_nMoveLockLength	= -1;

	m_bRandomStand = true;
	m_fMoveTargetLength = 0.f;
	m_hMoveTargetHandle.Identity();

	m_nChangeMoveAniIndex = -1;
}

CFcUnitObject::~CFcUnitObject()
{
	SAFE_DELETE( m_pSPathFind );

}


void CFcUnitObject::Process()
{	
	CFcGameObject::Process();
	if( m_bUsePathFind && !m_bDead )
	{
		m_pSPathFind->Process();
	}
	ProcessMove();

	if( m_pSPathFind->IsNewPath() )
		++m_nMovePathCnt;
	else
		m_nMovePathCnt = 0;
}

void CFcUnitObject::PostProcess()
{
	CFcGameObject::PostProcess();
}

void CFcUnitObject::CmdMove( int nX, int nY, float fSpeed, int nAniType, int nAniIndex )
{
	if( m_bDead ) return;
	if( !IsMovable() ) return;

	D3DXVECTOR3 vTemp = D3DXVECTOR3( (float)nX, 0.f, (float)nY );
	D3DXVECTOR3 Pos = m_Cross.GetPosition();

	ResetMoveVector();

	float fTemp;
	int nChangeAniIndex = FindExactAni(	m_pUnitInfoData->GetAniIndex( nAniType & 0x00FF, nAniIndex, nAniType & 0xFF00 ) );
	if( fSpeed == -1.f ) {
		int nIndex = m_pUnitInfoData->GetAniIndex( m_pUnitInfoData->GetAniType( nChangeAniIndex ), m_pUnitInfoData->GetAniTypeIndex( nChangeAniIndex ), m_pUnitInfoData->GetAniAttr( nChangeAniIndex ) );
		fTemp = (float)m_pUnitInfoData->GetMoveSpeed( nIndex );
	}
	else fTemp = fSpeed;
	fTemp = fTemp * m_fMoveSpeedRateForTroop;

	float fLength = D3DXVec3LengthSq( (D3DXVECTOR3*)&(vTemp - D3DXVECTOR3( Pos.x, 0.f, Pos.z )) );
	if( fLength < fTemp * fTemp || fLength < (float)GetUnitRadius() * (float)GetUnitRadius() ) {
		m_MoveTargetPos = Pos;
		BsAssert( m_MoveTargetPos.x == m_MoveTargetPos.x );
		return;
	}

	if( m_bBoost )
	{
		fTemp = fTemp * m_fMoveSpeedRate;
	}

	m_fMoveSpeed = fTemp;
	m_MoveTargetPos = vTemp;
	BsAssert( m_MoveTargetPos.x == m_MoveTargetPos.x );
	m_MoveDir = vTemp - D3DXVECTOR3( Pos.x, 0.f, Pos.z );
	BsAssert( (m_MoveDir.x == m_MoveDir.x) && (m_MoveDir.y == m_MoveDir.y) && (m_MoveDir.z == m_MoveDir.z) );
	D3DXVec3Normalize(&m_MoveDir, &m_MoveDir);

	SetFlocking( false );


	if( ( (nAniType & 0x00FF) == m_nCurAniType ) && nAniIndex == -1 ) return;
	if( m_nChangeMoveAniIndex == nChangeAniIndex ) return;
	if( m_nChangeMoveAniIndex == m_nAniIndex ) return;
	
	m_nChangeMoveAniIndex = nChangeAniIndex;
//	m_bChangeMoveAni = true;
//	ChangeAnimation( nAniType & 0x00FF, nAniIndex, nAniType & 0xFF00 );
}

void CFcUnitObject::CmdMove( GameObjHandle &Handle, float fLength, float fSpeed, int nAniType, int nAniIndex )
{
	if( !CFcBaseObject::IsValid( Handle ) ) return;
	CmdMove( (int)Handle->GetPos().x, (int)Handle->GetPos().z, fSpeed, nAniType, nAniIndex );
	m_hMoveTargetHandle = Handle;
	m_fMoveTargetLength = ( fLength == 0.f ) ? 1.f : fLength;
}

void CFcUnitObject::CmdStop( int nAniType, int nAniIndex )
{
	if( m_bDead ) return;
	if( IsMove() || m_nChangeMoveAniIndex != -1 )
	{
		m_nChangeMoveAniIndex = -1;

		if( IsMove() ) {
			int nFindAni = FindExactAni( m_pUnitInfoData->GetAniIndex( nAniType & 0x00FF, nAniIndex, nAniType & 0xFF00 ) );
			ChangeAnimation( m_pUnitInfoData->GetAniType( nFindAni ), m_pUnitInfoData->GetAniTypeIndex( nFindAni ), m_pUnitInfoData->GetAniAttr( nFindAni ) );
//			ChangeAnimation( nAniType & 0x00FF, nAniIndex, nAniType & 0xFF00 );
		}
	}
	ResetMoveVector();
}
/*
void CFcUnitObject::CmdAttack()
{
	if( !IsAttackable() )
	{
		return;
	}

	if( m_nCurAniType == ANI_TYPE_JUMP_UP )
	{
		ChangeAnimation( ANI_TYPE_JUMP_ATTACK, 0 );
	}
	else
	{
		ChangeAnimation( ANI_TYPE_ATTACK, 0 );
	}
}
*/

// 즉시 방향을 바꾼다
void CFcUnitObject::CmdLookUp( GameObjHandle Handle )
{
	if( m_nCurAniType == ANI_TYPE_DASH ) return;
	if( !CFcBaseObject::IsValid( Handle ) ) return;

	D3DXVECTOR3 TargetPos = Handle->GetPos();
	D3DXVECTOR3 Pos = m_Cross.GetPosition();
	
	m_MoveDir.x = TargetPos.x - Pos.x;
	m_MoveDir.y = 0.f;
	m_MoveDir.z = TargetPos.z - Pos.z;
	BsAssert( (m_MoveDir.x == m_MoveDir.x) && (m_MoveDir.y == m_MoveDir.y) && (m_MoveDir.z == m_MoveDir.z) );
	D3DXVec3Normalize(&m_MoveDir, &m_MoveDir);

	m_MoveTargetPos = TargetPos;
	BsAssert( m_MoveTargetPos.x == m_MoveTargetPos.x );

}

void CFcUnitObject::CmdVictory( float fX, float fY )
{
	if( m_bDead ) return;
//	D3DXVECTOR3 Dir;

	D3DXVECTOR3 Pos = m_Cross.GetPosition();

	m_MoveDir.x = fX - Pos.x;
	m_MoveDir.y = 0.f;
	m_MoveDir.z = fY - Pos.z;
	BsAssert( (m_MoveDir.x == m_MoveDir.x) && (m_MoveDir.y == m_MoveDir.y) && (m_MoveDir.z == m_MoveDir.z) );
	D3DXVec3Normalize(&m_MoveDir, &m_MoveDir);

	m_Cross.m_ZVector=m_MoveDir;
	m_Cross.UpdateVectors();

	if( IsStand() )
	{
		ChangeAnimation( ANI_TYPE_VICTORY, 0 );
	}
}

void CFcUnitObject::ProcessMove()
{
	if( m_ParentLinkObjHandle ) return;

	if( m_nCurAniType == ANI_TYPE_WALK || m_nCurAniType == ANI_TYPE_RUN || m_nChangeMoveAniIndex != -1 ) {
		bool bMove = true;
		if( m_fMoveTargetLength != 0.f && ( m_bUsePathFind && !m_pSPathFind->IsNewPath() && !m_pSPathFind->IsLockPath() ) ) {
			if( !CFcBaseObject::IsValid( m_hMoveTargetHandle ) ) {
				CmdStop( ANI_TYPE_STAND | GetCurAniAttr(), -1 );
				bMove = false;
			}
			else {
				D3DXVECTOR3 vVec = m_MoveTargetPos - GetDummyPos();
				vVec.y = 0.f;

				float fDistance = 0.f;
				fDistance = m_fMoveTargetLength;
				if( m_bApplyFlocking == true && m_fMaxFlockingSize > fDistance ) {
					fDistance = m_fMaxFlockingSize;
				}
				fDistance += 1.f;	// 아씨..이상하게 안스네..

				if( D3DXVec3Length( &vVec ) <= fDistance ) {
					CmdStop( ANI_TYPE_STAND | GetCurAniAttr(), -1 );
					bMove = false;
				}
				else {
					m_MoveTargetPos = m_hMoveTargetHandle->GetPos();
					BsAssert( m_MoveTargetPos.x == m_MoveTargetPos.x );
					m_MoveDir = vVec;
					BsAssert( (m_MoveDir.x == m_MoveDir.x) && (m_MoveDir.y == m_MoveDir.y) && (m_MoveDir.z == m_MoveDir.z) );
					D3DXVec3Normalize( &m_MoveDir, &m_MoveDir );
				}
			}
		}
		else {
			if( D3DXVec3LengthSq( &m_MoveTargetPos ) > 0.f ) {
				D3DXVECTOR3 vPos = GetDummyPos() - m_MoveTargetPos;
				vPos.y = 0.f;

				float fLength = D3DXVec3Length( &vPos );

				float fDistance = 0.f;
				fDistance = m_fMoveSpeed;
				if( m_bApplyFlocking == true && m_fMaxFlockingSize > fDistance )  {
					fDistance = m_fMaxFlockingSize;
				}
				fDistance += 1.f;	// 아씨..이상하게 안스네..

				if( fLength <= fDistance ) {
					CmdStop( ANI_TYPE_STAND | GetCurAniAttr(), -1 );
					bMove = false;
				}
			}
		}
		if( bMove ) MoveZ( m_fMoveSpeed );
		if( m_nChangeMoveAniIndex != -1 && !m_bDead && IsMovable() && m_nAniIndex != m_nChangeMoveAniIndex ) {
//			ChangeAnimationByIndex( m_nChangeMoveAniIndex );
			ChangeAnimation( m_pUnitInfoData->GetAniType( m_nChangeMoveAniIndex ), m_pUnitInfoData->GetAniTypeIndex( m_nChangeMoveAniIndex ), m_pUnitInfoData->GetAniAttr( m_nChangeMoveAniIndex ) );
			// 자연스러운 부대 움직임을 위해 추가
			BsAssert( m_nCurAniLength > 0 );
			if( m_ClassID == Class_ID_Unit || m_ClassID == Class_ID_Archer )
				m_fFrame = (float)Random( m_nCurAniLength-1 );
		}
		m_nChangeMoveAniIndex = -1;
	}
}

#include "FcCorrelation.h"
void CFcUnitObject::DebugRender()
{
	CFcGameObject::DebugRender();

	if( m_pSPathFind ) m_pSPathFind->Render();


	/*
#ifndef _XBOX
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


	struct LINE_VERTEX
	{
		D3DXVECTOR3 v;
		DWORD       color;
	};
	static LINE_VERTEX pVertices[2];


	pDevice->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );

	CFcCorrelationTargeting *pCorrelation = (CFcCorrelationTargeting *)GetCorrelation( CFcCorrelation::CT_TARGETING_COUNTER );
	std::vector<GameObjHandle> *pVecList = pCorrelation->GetList();
//	D3DXVECTOR3 vPos[2];
	pVertices[0].v = GetPos();
	pVertices[0].v.y += 50.f;
	for( DWORD i=0; i<pVecList->size(); i++ ) {
		if( !(*pVecList)[i] ) {
			pVertices[0].color = 0xFF000000;
			pVertices[1].color = 0xFF000000;
			continue;
		}
		if( !CFcBaseObject::IsValid( (*pVecList)[i] ) ) {
			pVertices[0].color = 0xFFFFFFFF;
			pVertices[1].color = 0xFFFFFFFF;
			continue;
		}
		pVertices[0].color = 0xFFFF0000;
		pVertices[1].color = 0xFF00FFFF;
		pVertices[1].v = (*pVecList)[i]->GetPos();
		pVertices[1].v.y += 100.f;

		pDevice->DrawPrimitiveUP( D3DPT_LINELIST, 1, pVertices, sizeof(LINE_VERTEX) );
	}

	pDevice->SetTransform( D3DTS_WORLD, &matWorld );
	pDevice->EndScene();
	pDevice->RestoreState();
#endif //_XBOX
	*/

}


void CFcUnitObject::CheckMoveLock( TroopObjHandle hTroop )
{
	CFcGameObject::CheckMoveLock( hTroop );
	/*
	if( m_pSPathFind->IsNewPath() ) m_bCheckMoveLock = true;

	if( m_bCheckMoveLock == true && GetProcessTick() % 60 != (int)m_nEngineIndex % 60 && s_nUnitStateTable[ m_nCurAniType ] & UNIT_MOVABLE ) {
		if( m_nMoveLockTick == -1 ) {
			m_nMoveLockLength = (int)D3DXVec2LengthSq( (D3DXVECTOR2*)&(GetPosV2() - m_TroopOffsetPos) );
			m_nMoveLockTick = GetProcessTick();
			m_vMoveLockPos = m_TroopOffsetPos;
		}
		else {
			int nLength = (int)D3DXVec2LengthSq( (D3DXVECTOR2*)&(GetPosV2() - m_TroopOffsetPos) );
			int nPrevLength = (int)D3DXVec2LengthSq( (D3DXVECTOR2*)&(GetPosV2() - m_vMoveLockPos) );
			int nTick = GetProcessTick();

			if( m_nMoveLockLength < nPrevLength && m_nMoveLockLength < nLength ) {
				m_nMoveLockTick = -1;
				m_Cross.m_PosVector = hTroop->GetPos();
				m_Cross.m_PosVector.x += m_TroopOffsetPos.x;
				m_Cross.m_PosVector.z += m_TroopOffsetPos.y;
				m_Cross.m_PosVector.y = g_BsKernel.GetLandHeight( m_Cross.m_PosVector.x, m_Cross.m_PosVector.z );
				CFcWorld::GetInstance().MoveUnitToScanner(m_Handle, &m_Cross.m_PosVector);
			}
		}

		m_bCheckMoveLock = false;
	}
	*/
}



bool CFcUnitObject::IsFlockable( GameObjHandle Handle )
{
	switch( Handle->GetClassID() ) {
		case Class_ID_Horse:	return true;
		case Class_ID_Catapult: return true;
		case Class_ID_MoveTower: return true;
		case Class_ID_Hero:
			switch( ((HeroObjHandle)Handle)->GetHeroClassID() ) {
				case Class_ID_Hero_VigkVagk:	
					if( !IsDie() ) return true;
					break;
			}
			if( Handle->IsJump() ) return false;
			break;
		default:
			break;
	}
	return CFcGameObject::IsFlockable( Handle );
}

int CFcUnitObject::FindExactAni( int nAniIndex )
{
	if( m_bRandomStand == true ) {
		int nAniType = m_pUnitInfoData->GetAniType(nAniIndex);
		int nAniTypeIndex = m_pUnitInfoData->GetAniTypeIndex(nAniIndex);
		int nAniAttr = m_pUnitInfoData->GetAniAttr(nAniIndex);

		switch( nAniType ) {
			case ANI_TYPE_STAND:	return m_pUnitInfoData->GetAniIndex( ANI_TYPE_STAND, -1, nAniAttr );
			default:	break;
		}
	}
 	return CFcGameObject::FindExactAni(nAniIndex);
}

void CFcUnitObject::LockShortPathFinder( D3DXVECTOR3 &vDir, int nTick )
{
	if( m_bUsePathFind ) m_pSPathFind->LockPath( vDir, nTick );
}

void CFcUnitObject::ResetMoveVector()
{
	m_hMoveTargetHandle.Identity();
	m_fMoveTargetLength = 0.f;
	CFcGameObject::ResetMoveVector();
}