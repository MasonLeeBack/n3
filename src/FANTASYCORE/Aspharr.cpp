#include "StdAfx.h"
#include "InputPad.h"
#include ".\aspharr.h"
#include "AsData.h"
#include "FcGlobal.h"
#include ".\\data\\AnimType.h"
#include ".\\data\\SignalType.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


CAspharr::CAspharr( CCrossVector *pCross )
	: CFcHeroObject( pCross )
{
	SetHeroClassID( Class_ID_Hero_Aspharr );
	m_nSAttackFrame = 0;
	m_fRagdollDistY = 0.f;
}

CAspharr::~CAspharr(void)
{
}

void CAspharr::Process()
{
	if( m_nAniIndex == 95 )
	{
		if( CInputPad::GetInstance().GetKeyPressTick( g_FCGameData.nEnablePadID, PAD_INPUT_X ) == 0 )
		{
			ChangeAnimationByIndex( 97 );
		}
	}
	else if( m_nAniIndex == SATTACK_X )
	{
		if( CInputPad::GetInstance().GetKeyPressTick( g_FCGameData.nEnablePadID, PAD_INPUT_X ) )
		{
			m_nSAttackFrame++;
		}
		else
		{
			if( m_nSAttackFrame > 50 )
			{
				ChangeAnimationByIndex( SATTACK_X_LARGE );
			}
			else
			{
				ChangeAnimationByIndex( SATTACK_X_SMALL );
			}
			m_nSAttackFrame = 0;
		}
	}
	else if( m_nAniIndex == SATTACK_Y )
	{
		if( CInputPad::GetInstance().GetKeyPressTick( g_FCGameData.nEnablePadID, PAD_INPUT_Y ) )
		{
			m_nSAttackFrame++;
		}
		else
		{
			if( m_nSAttackFrame > 50 )
			{
				ChangeAnimationByIndex( SATTACK_Y_LARGE );
			}
			else
			{
				ChangeAnimationByIndex( SATTACK_Y_SMALL );
			}
			m_nSAttackFrame = 0;
		}
	}

	CFcHeroObject::Process();
}

bool CAspharr::Render()
{
	if( m_CatchHandle ) {    		
		if( m_fRagdollDistY > 0.f ) {
			float fAngleScale = ( (m_fRagdollDistY) / 80.f );
			fAngleScale = BsMin(1.2f, fAngleScale);	
			g_BsKernel.SendMessage( m_nEngineIndex, BS_SET_CLEAR_BONE_ROTATION );
			g_BsKernel.SendMessage( m_nEngineIndex, BS_SET_BONE_ROTATION, (DWORD)"Bip01 Spine", (DWORD)&D3DXVECTOR3( -10.f*fAngleScale, 0.f, 0.f ) );
			g_BsKernel.SendMessage( m_nEngineIndex, BS_SET_BONE_ROTATION, (DWORD)"Bip01 Spine1", (DWORD)&D3DXVECTOR3( -10.f*fAngleScale, 0.f, 0.f ) );
			g_BsKernel.SendMessage( m_nEngineIndex, BS_SET_BONE_ROTATION, (DWORD)"Bip01 Spine2", (DWORD)&D3DXVECTOR3( -10.f*fAngleScale, 0.f, 0.f ) );
		}
		else {
			g_BsKernel.SendMessage( m_nEngineIndex, BS_SET_CLEAR_BONE_ROTATION );
		}
	}
	return CFcHeroObject::Render();
}

bool CAspharr::Catch( GameObjHandle Handle )
{
	if( ( m_CatchHandle ) || ( m_nCurrentWeaponIndex == -1 ) )
	{
		return false;
	}
	if( Handle->IsDie() ) return false;

	int nLinkCount;

	nLinkCount = g_BsKernel.SendMessage( m_WeaponList[ m_nCurrentWeaponIndex ].nObjectIndex, BS_GET_LINKDUMMY_COUNT );
	if( nLinkCount <= 0 )
	{
		return false;
	}
	Handle->RideOut();

	m_CatchHandle = Handle;
	m_CatchHandle->RunRagdoll();
	g_BsKernel.SendMessage( m_CatchHandle->GetEngineIndex(), BS_PHYSICS_SIMULATION, TRUE );


	//g_BsKernel.SendMessage( m_WeaponList[ m_nCurrentWeaponIndex ].nObjectIndex, BS_LINKOBJECT, 0, 
		//m_CatchHandle->GetEngineIndex(), nBoneIndex );
	ChangeAnimation( ANI_TYPE_ATTACK, 8 );
	m_CatchHandle->ChangeAnimation( ANI_TYPE_DOWN, 0 );	// Ragdoll 상태일때 AI 못돌게 할려고..
	m_nCatchBoneIndex = g_BsKernel.SendMessage( m_CatchHandle->GetEngineIndex(), BS_GET_BONE_INDEX, ( DWORD )"Bip01 Spine1" );

	return true;
}

void CAspharr::SendCatchMsg()
{
	if( m_CatchHandle )
	{
		int nCatchEngineIndex = m_CatchHandle->GetEngineIndex();		

		BSMATRIX *pMatrix = (BSMATRIX*)g_BsKernel.SendMessage( nCatchEngineIndex, BS_GET_BONE_WORLD_MATRIX, (DWORD)"Bip01 Spine1");
// [PREFIX:beginmodify] 2006/2/25 junyash PS#5810 dereferencing NULL pointer
		BsAssert(pMatrix);
// [PREFIX:endmodify] junyash
		m_fRagdollDistY = (m_CatchHandle->GetCrossVector()->GetPosition().y - GetCrossVector()->GetPosition().y) + (pMatrix->_42 - 90.f);

		m_CatchHandle->SetCatchedHero( true );
		DebugString("CAspharr::Render() Catch : LinkObject(%d)\n", nCatchEngineIndex);
		g_BsKernel.SendMessage( m_WeaponList[ m_nCurrentWeaponIndex ].nObjectIndex, BS_LINKOBJECT, 0, nCatchEngineIndex );
		g_BsKernel.SendMessage(  nCatchEngineIndex, BS_SET_RAGDOLL_CATCH_BONE, m_nCatchBoneIndex);
	}
}

void CAspharr::SendUnlinkCatchMsg( int nHandIndex /*= -1*/ )
{
	if( m_CatchHandle )
	{		
		int nCatchEngineIndex = m_CatchHandle->GetEngineIndex();
		m_CatchHandle->SetCatchedHero( false );
		DebugString("CAspharr::Render() Uncatch : UnLinkObject(%d)\n", nCatchEngineIndex);
		g_BsKernel.SendMessage( m_WeaponList[ m_nCurrentWeaponIndex ].nObjectIndex, BS_UNLINKOBJECT, nCatchEngineIndex, ( DWORD )&m_UnlinkVelocity );
		m_CatchHandle->UnlinkRagdoll();
		m_CatchHandle.Identity();
		g_BsKernel.SendMessage( m_nEngineIndex, BS_SET_CLEAR_BONE_ROTATION );
	}
}
void CAspharr::LoadWeaponTrailTexture()
{
	int nTextureId;
	g_BsKernel.chdir("Fx");
	nTextureId = g_BsKernel.LoadTexture( "Aspharr_SwordTrail01.dds" );
	if( nTextureId != -1 ) 
		m_WeaponTrailTexIDs.push_back(nTextureId);

	nTextureId = g_BsKernel.LoadTexture( "Aspharr_SwordTrail02.dds" );
	if( nTextureId != -1 ) 
		m_WeaponTrailTexIDs.push_back(nTextureId);

	m_nWeaponTrailOffsetTexID = g_BsKernel.LoadTexture( "Aspharr_SwordTrail_Normals.dds" );
	g_BsKernel.chdir("..");
}