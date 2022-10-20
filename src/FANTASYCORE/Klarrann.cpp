#include "StdAfx.h"
#include "AsData.h"
#include ".\\data\\AnimType.h"
#include ".\\data\\SignalType.h"
#include ".\klarrann.h"
#include "FcFxManager.h"
#include "FcGlobal.h"
#include "FcWorld.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


CKlarrann::CKlarrann( CCrossVector *pCross )
	: CFcHeroObject( pCross )
{
	SetHeroClassID( Class_ID_Hero_Klarrann );
	m_bOrbAttack		= false;
	m_nFxTemplateIndex	= -1;
	m_nFxObjIndex		= -1;
	m_bOnceLink			= false;

	m_nHammerFxTemplateIndex	= -1;
	m_nHammerFxObjIndex			= -1;	
	m_nAxeFxTemplateIndex		= -1;	
	m_nAxeFxObjIndex			= -1;
	m_nScytheFxTemplateIndex	= -1;
	m_nScytheFxObjIndex			= -1;

	SetBHammer( false );
	SetBAxe( false );	
	SetBScythe( false );
}

CKlarrann::~CKlarrann(void)
{
	if ( m_nFxObjIndex != - 1) {
		g_BsKernel.DeleteObject(m_nFxObjIndex);
		m_nFxObjIndex = -1;
	}

	if ( m_nFxTemplateIndex != -1 ) {
		g_BsKernel.ReleaseFXTemplate( m_nFxTemplateIndex );
		m_nFxTemplateIndex = -1;
	}

	if ( m_nHammerFxObjIndex != - 1) {
		g_BsKernel.DeleteObject(m_nHammerFxObjIndex);
		m_nHammerFxObjIndex = -1;
	}

	if ( m_nHammerFxTemplateIndex != -1 ) {
		g_BsKernel.ReleaseFXTemplate( m_nHammerFxTemplateIndex );
		m_nHammerFxTemplateIndex = -1;
	}

	if ( m_nAxeFxObjIndex != - 1) {
		g_BsKernel.DeleteObject(m_nAxeFxObjIndex);
		m_nAxeFxObjIndex = -1;
	}

	if ( m_nAxeFxTemplateIndex != -1 ) {
		g_BsKernel.ReleaseFXTemplate( m_nAxeFxTemplateIndex );
		m_nAxeFxTemplateIndex = -1;
	}

	if ( m_nScytheFxObjIndex != - 1) {
		g_BsKernel.DeleteObject(m_nScytheFxObjIndex);
		m_nScytheFxObjIndex = -1;
	}

	if ( m_nScytheFxTemplateIndex != -1 ) {
		g_BsKernel.ReleaseFXTemplate( m_nScytheFxTemplateIndex );
		m_nScytheFxTemplateIndex = -1;
	}
}

bool CKlarrann::Catch( GameObjHandle Handle )
{
	if( m_CatchHandle )
	{
		return false;
	}
	if( Handle->IsDie() ) return false;

	g_BsKernel.SetPreCalcAni( m_nEngineIndex, true);

	Handle->RideOut();

	m_CatchHandle = Handle;
	m_CatchHandle->RunRagdoll();
	g_BsKernel.SendMessage( m_CatchHandle->GetEngineIndex(), BS_PHYSICS_SIMULATION, TRUE );

	m_CatchHandle->ChangeAnimation( ANI_TYPE_DOWN, 0 );	// Ragdoll �����϶� AI ������ �ҷ���..
	switch( m_nAniIndex )
	{
	case 112:	// X1������ 
		ChangeAnimationByIndex( 57 );
		m_nCatchBoneIndex = g_BsKernel.SendMessage( m_CatchHandle->GetEngineIndex(), BS_GET_BONE_INDEX, ( DWORD )"Bip01 Head" );
		break;	
	case 58:	// �Ϲ� ���
	case 73:	// �Ϲ����2 
		ChangeAnimationByIndex( 97 );
		m_nCatchBoneIndex = g_BsKernel.SendMessage( m_CatchHandle->GetEngineIndex(), BS_GET_BONE_INDEX, ( DWORD )"Bip01 Head" );
		break;
	case 75:	// �뽬 ���
		ChangeAnimationByIndex( 98 );
		m_nCatchBoneIndex = g_BsKernel.SendMessage( m_CatchHandle->GetEngineIndex(), BS_GET_BONE_INDEX, ( DWORD )"Bip01 Head" );
		break;
	case 66:	// �ʻ� ���
		ChangeAnimationByIndex( 96 );
		m_nCatchBoneIndex = g_BsKernel.SendMessage( m_CatchHandle->GetEngineIndex(), BS_GET_BONE_INDEX, ( DWORD )"Bip01 R Foot" );
		if( m_nCatchBoneIndex == -1 ) {	// ���� ����� �ٸ��� ��� ���� ��´�.
			m_nCatchBoneIndex = g_BsKernel.SendMessage( m_CatchHandle->GetEngineIndex(), BS_GET_BONE_INDEX, ( DWORD )"Bip01 R Hand" );
		}
		break;
	default:
		m_nCatchBoneIndex = g_BsKernel.SendMessage( m_CatchHandle->GetEngineIndex(), BS_GET_BONE_INDEX, ( DWORD )"Bip01 Head" );
	}

	return true;
}

void CKlarrann::SendCatchMsg()
{
	if( m_CatchHandle )
	{
		int nBone;
		nBone = g_BsKernel.SendMessage( m_nEngineIndex, BS_GET_BONE_INDEX, ( DWORD )"Dummy_WP_L" );
		int nCatchEngineIndex = m_CatchHandle->GetEngineIndex();
		m_CatchHandle->SetCatchedHero( true );
		DebugString("CKlarrann::Render() Catch : LinkObject(%d)\n", nCatchEngineIndex);
		g_BsKernel.SendMessage( m_nEngineIndex, BS_LINKOBJECT, nBone, nCatchEngineIndex );
		g_BsKernel.SendMessage( nCatchEngineIndex, BS_SET_RAGDOLL_CATCH_BONE, m_nCatchBoneIndex );
	}
}

void CKlarrann::SendUnlinkCatchMsg( int nHandIndex /*= -1*/ )
{
	if( m_CatchHandle )
	{
		int nCatchEngineIndex = m_CatchHandle->GetEngineIndex();
		m_CatchHandle->SetCatchedHero( false );
		DebugString("CKlarrann::Render() Uncatch : UnLinkObject(%d)\n", nCatchEngineIndex);
		g_BsKernel.SendMessage( m_nEngineIndex, BS_UNLINKOBJECT, nCatchEngineIndex, ( DWORD )&m_UnlinkVelocity );
		m_CatchHandle->UnlinkRagdoll();
		m_CatchHandle.Identity();
	}
}

void CKlarrann::Initialize( INITGAMEOBJECTDATA* pData, int nForce, int nTeam, TroopObjHandle hTroop, float fScaleMin, float fScaleMax )
{
	CFcHeroObject::Initialize( pData, nForce, nTeam, hTroop, fScaleMin, fScaleMax );

	// �Ʒ� �ڵ� ������ �ε��ϴ� fx���� ����ĳ���͸� ���� fx �Դϴ�. �׷��Ƿ� �����ɸ��Ͱ� �ƴѰ�� ���h�ؾ� �մϴ�.
	if( GetHandle() == g_FcWorld.GetHeroHandle(0) ) {
		if( m_nFxTemplateIndex == -1 )
		{
			char szFxFilePath[MAX_PATH];
			g_BsKernel.chdir("fx");
			sprintf(szFxFilePath,"%s%s",g_BsKernel.GetCurrentDirectory(),"kar_streamSword.bfx" );
			g_BsKernel.chdir("..");
			m_nFxTemplateIndex = g_BsKernel.FindFXTemplate(szFxFilePath);
			if( m_nFxTemplateIndex != -1 )
			{
				g_BsKernel.AddFXTemplateRef(m_nFxTemplateIndex);
			}
			else
			{
				DebugString("Find Not Prop Fx File : %s \n",szFxFilePath);
				//BsAssert(0);
			}
		}
		if( m_nFxObjIndex == -1 )
		{
			BsAssert( m_nFxObjIndex == -1 );
			m_nFxObjIndex = g_BsKernel.CreateFXObject( m_nFxTemplateIndex );
		}
	}

	if( m_nHammerFxTemplateIndex == -1 )
	{
		char szFxFilePath[MAX_PATH];
		g_BsKernel.chdir("fx");
		sprintf(szFxFilePath,"%s%s",g_BsKernel.GetCurrentDirectory(),"kra_attack10_Hammer01.bfx" );
		g_BsKernel.chdir("..");
		m_nHammerFxTemplateIndex = g_BsKernel.FindFXTemplate(szFxFilePath);
		if( m_nHammerFxTemplateIndex != -1 )
		{
			g_BsKernel.AddFXTemplateRef(m_nHammerFxTemplateIndex);
		}
		else
		{
			DebugString("Find Not Prop Fx File : %s \n",szFxFilePath);
			BsAssert(0);
		}
	}

	if( m_nAxeFxTemplateIndex == -1 )
	{
		char szFxFilePath[MAX_PATH];
		g_BsKernel.chdir("fx");
		sprintf(szFxFilePath,"%s%s",g_BsKernel.GetCurrentDirectory(),"kra_attack10_Axe01.bfx" );
		g_BsKernel.chdir("..");
		m_nAxeFxTemplateIndex = g_BsKernel.FindFXTemplate(szFxFilePath);
		if( m_nAxeFxTemplateIndex != -1 )
		{
			g_BsKernel.AddFXTemplateRef(m_nAxeFxTemplateIndex);
		}
		else
		{
			DebugString("Find Not Prop Fx File : %s \n",szFxFilePath);
			BsAssert(0);
		}
	}


	if( m_nScytheFxTemplateIndex == -1 )
	{
		char szFxFilePath[MAX_PATH];
		g_BsKernel.chdir("fx");
		sprintf(szFxFilePath,"%s%s",g_BsKernel.GetCurrentDirectory(),"kra_attack10_Scythe01.bfx" );
		g_BsKernel.chdir("..");
		m_nScytheFxTemplateIndex = g_BsKernel.FindFXTemplate(szFxFilePath);
		if( m_nScytheFxTemplateIndex != -1 )
		{
			g_BsKernel.AddFXTemplateRef(m_nScytheFxTemplateIndex);
		}
		else
		{
			DebugString("Find Not Prop Fx File : %s \n",szFxFilePath);
			BsAssert(0);
		}
	}

}

void CKlarrann::SignalCheck( ASSignalData *pSignal )
{
	switch( pSignal->m_nID )
	{
	case SIGNAL_TYPE_CUSTOM_MESSAGE:
		{
			switch( pSignal->m_pParam[0] )
			{
			case 0:		// ��� ����Ʈ ����
				{
					if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
					{
						SetBHammer(true);
					}

				}
				break;
			case 1:		// ��� ����Ʈ �Ҹ�
				{
					if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
					{
						SetBHammer(false);
					}

				}
				break;
			case 2:		// ���� ����Ʈ ����
				{
					if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
					{
						SetBAxe(true);
					}

				}
				break;
			case 3:		// ���� ����Ʈ �Ҹ�
				{
					if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
					{
						SetBAxe(false);
					}

				}
				break;
			case 4:		// �� ����Ʈ ����
				{
					if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
					{
						SetBScythe(true);
					}

				}
				break;
			case 5:		// �� ����Ʈ �Ҹ�
				{
					if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
					{
						SetBScythe(false);
					}

				}
				break;
			case 6:		// �¾�����? ( ��, ��ġ, ���� ����Ʈ�� �������. )
				{
					if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
					{
						SetBScythe(false);
						SetBAxe(false);
						SetBHammer(false);
					}

				}
				break;
			}
		}
		break;
	default:
		CFcHeroObject::SignalCheck( pSignal );
		break;
	}
}


void CKlarrann::Process()
{
	CFcHeroObject::Process();

	if( GetOrbAttack() )
	{
		// �������³༮ �����Ѵ�.
		// ANI_TYPE_SPECIAL_ATTACK �� ani �� �����ϰ�� kar_streamSword.bfx �� �����Ѵ�.
		if( m_nCurAniType != ANI_TYPE_SPECIAL_ATTACK)
		{
			if( m_nFxTemplateIndex == -1 || m_nFxObjIndex == -1 )
			{
				if( m_nFxObjIndex == -1 )
				{
					BsAssert( m_nFxObjIndex == -1 );
					m_nFxObjIndex = g_BsKernel.CreateFXObject( m_nFxTemplateIndex );
				}
			}
			if( !m_bOnceLink && m_nFxObjIndex >= 0 )
			{
				int nBone;
				nBone = g_BsKernel.SendMessage( m_nEngineIndex, BS_GET_BONE_INDEX, ( DWORD )"Dummy_WP_R" );
				g_BsKernel.SendMessage( m_nEngineIndex, BS_LINKOBJECT, nBone, m_nFxObjIndex ); 
				g_BsKernel.SetFXObjectState( m_nFxObjIndex, CBsFXObject::PLAY, -99 );
				g_BsKernel.ShowObject( m_nFxObjIndex, true );
				m_bOnceLink = true;
			}
		}//

	}
	else
	{
		if( m_nFxObjIndex != -1 ) 
		{
			int nBone;
			g_BsKernel.ShowObject( m_nFxObjIndex, false );
			nBone = g_BsKernel.SendMessage( m_nEngineIndex, BS_GET_BONE_INDEX, ( DWORD )"Dummy_WP_R" );
			g_BsKernel.SendMessage( m_nEngineIndex, BS_UNLINKOBJECT, m_nFxObjIndex );
			g_BsKernel.DeleteObject(m_nFxObjIndex);
			m_nFxObjIndex = -1;
			m_bOnceLink = false;
		}//if( m_nFxObjIndex != -1 ) 
	}

	
	if( GetBHammer() )
	{
		if( m_nHammerFxObjIndex == -1 )
		{
			// �����Ѵ�.
			m_nHammerFxObjIndex = g_BsKernel.CreateFXObject( m_nHammerFxTemplateIndex );
			int nBone;
			nBone = g_BsKernel.SendMessage( m_nEngineIndex, BS_GET_BONE_INDEX, ( DWORD )"Dummy_WP_R" );
			g_BsKernel.SendMessage( m_nEngineIndex, BS_LINKOBJECT, nBone, m_nHammerFxObjIndex ); 
			g_BsKernel.SetFXObjectState( m_nHammerFxObjIndex, CBsFXObject::PLAY, -99 );
			g_BsKernel.ShowObject( m_nHammerFxObjIndex, true );
		}
	}
	else
	{
		if( m_nHammerFxObjIndex != -1 )
		{
			// ���ش�.
			int nBone;
			g_BsKernel.ShowObject( m_nHammerFxObjIndex, false );
			nBone = g_BsKernel.SendMessage( m_nEngineIndex, BS_GET_BONE_INDEX, ( DWORD )"Dummy_WP_R" );
			g_BsKernel.SendMessage( m_nEngineIndex, BS_UNLINKOBJECT, m_nHammerFxObjIndex );
			g_BsKernel.DeleteObject(m_nHammerFxObjIndex);
			m_nHammerFxObjIndex = -1;
		}
	}

	if( GetBAxe() )
	{
		if( m_nAxeFxObjIndex == -1 )
		{
			// �����Ѵ�.
			m_nAxeFxObjIndex = g_BsKernel.CreateFXObject( m_nAxeFxTemplateIndex );
			int nBone;
			nBone = g_BsKernel.SendMessage( m_nEngineIndex, BS_GET_BONE_INDEX, ( DWORD )"Dummy_WP_R" );
			g_BsKernel.SendMessage( m_nEngineIndex, BS_LINKOBJECT, nBone, m_nAxeFxObjIndex ); 
			g_BsKernel.SetFXObjectState( m_nAxeFxObjIndex, CBsFXObject::PLAY, -99 );
			g_BsKernel.ShowObject( m_nAxeFxObjIndex, true );
		}
	}
	else
	{
		if( m_nAxeFxObjIndex != -1 )
		{
			// ���ش�.
			int nBone;
			g_BsKernel.ShowObject( m_nAxeFxObjIndex, false );
			nBone = g_BsKernel.SendMessage( m_nEngineIndex, BS_GET_BONE_INDEX, ( DWORD )"Dummy_WP_R" );
			g_BsKernel.SendMessage( m_nEngineIndex, BS_UNLINKOBJECT, m_nAxeFxObjIndex );
			g_BsKernel.DeleteObject(m_nAxeFxObjIndex);
			m_nAxeFxObjIndex = -1;
		}
	}

	if( GetBScythe() )
	{
		if( m_nScytheFxObjIndex == -1 )
		{
			// �����Ѵ�.
			m_nScytheFxObjIndex = g_BsKernel.CreateFXObject( m_nScytheFxTemplateIndex );
			int nBone;
			nBone = g_BsKernel.SendMessage( m_nEngineIndex, BS_GET_BONE_INDEX, ( DWORD )"Dummy_WP_R" );
			g_BsKernel.SendMessage( m_nEngineIndex, BS_LINKOBJECT, nBone, m_nScytheFxObjIndex ); 
			g_BsKernel.SetFXObjectState( m_nScytheFxObjIndex, CBsFXObject::PLAY, -99 );
			g_BsKernel.ShowObject( m_nScytheFxObjIndex, true );
		}
	}
	else
	{
		if( m_nScytheFxObjIndex != -1 )
		{
			// ���ش�.
			int nBone;
			g_BsKernel.ShowObject( m_nScytheFxObjIndex, false );
			nBone = g_BsKernel.SendMessage( m_nEngineIndex, BS_GET_BONE_INDEX, ( DWORD )"Dummy_WP_R" );
			g_BsKernel.SendMessage( m_nEngineIndex, BS_UNLINKOBJECT, m_nScytheFxObjIndex );
			g_BsKernel.DeleteObject(m_nScytheFxObjIndex);
			m_nScytheFxObjIndex = -1;
		}
	}

	if( GetOrbSparkActivity() || GetOrbAttack() || IsPlayRealMovie() || IsPlayEvent())
	{
		DebugString(" IsEnableTrueOrbSpark : %d , GetOrbAttack : %d , IsPlayRealMovie : %d, IsPlayEvent : %d \n",GetOrbSparkActivity(), GetOrbAttack(), IsPlayRealMovie(), IsPlayEvent());
		SetBHammer( false );
		SetBAxe( false );	
		SetBScythe( false );
	}
	SetOrbSparkActivity(false);
}


bool CKlarrann::Render()
{
	return CFcHeroObject::Render();
}

int CKlarrann::FindExactAni( int nAniIndex )
{
	int nAniType = m_pUnitInfoData->GetAniType( nAniIndex );
	int nAniTypeIndex = m_pUnitInfoData->GetAniTypeIndex( nAniIndex );
	int nAniAttr = m_pUnitInfoData->GetAniAttr( nAniIndex );

	switch( m_nAniIndex )
	{
	case 59:
	case 61://�Ӹ����
		if( nAniType == ANI_TYPE_STAND )
		{
			return 59;
		}
		else if( nAniType == ANI_TYPE_RUN  || nAniType == ANI_TYPE_WALK )
		{
			return 61;
		}
		break;
	case 67:
	case 68://�ٸ����
		if( nAniType == ANI_TYPE_STAND )
		{
			return 67;
		}
		else if( nAniType == ANI_TYPE_RUN  || nAniType == ANI_TYPE_WALK )
		{
			return 68;
		}
		break;
	case 85:
	case 86:
	case 87://���꽺��ũ
		if( nAniType == ANI_TYPE_RUN )
		{
			return 87;
		}
		else if( nAniType == ANI_TYPE_WALK )
		{
			return 86;
		}
		else if( nAniType == ANI_TYPE_STAND )
		{
			return 85;
		}
		break;
	case 96:
		if( nAniType == ANI_TYPE_RUN )
		{
			return 96;
		}
		else if( nAniType == ANI_TYPE_WALK )
		{
			return 96;
		}
		else if( nAniType == ANI_TYPE_STAND )
		{
			return 67;
		}
		break;
	case 106:
	case 107:
	case 108://��ġ���
		if( nAniType == ANI_TYPE_RUN )
		{
			return 108;
		}
		else if( nAniType == ANI_TYPE_WALK )
		{
			return 107;
		}
		else if( nAniType == ANI_TYPE_STAND )
		{
			return 106;
		}
		break;
	case 114:
	case 115:
	case 116://�������
		if( nAniType == ANI_TYPE_RUN )
		{
			return 116;
		}
		else if( nAniType == ANI_TYPE_WALK )
		{
			return 115;
		}
		else if( nAniType == ANI_TYPE_STAND )
		{
			return 114;
		}
		break;
	case 121:
	case 122:
	case 123://�������
		if( nAniType == ANI_TYPE_RUN )
		{
			return 123;
		}
		else if( nAniType == ANI_TYPE_WALK )
		{
			return 122;
		}
		else if( nAniType == ANI_TYPE_STAND )
		{
			return 121;
		}
		break;
	}

	return nAniIndex;
}

bool CKlarrann::OrbAttackFxOn()
{
	SetOrbAttack(true );
	return true;
}

bool CKlarrann::OrbAttackFxOff()
{
	SetOrbAttack(false );
	return true;
}

void CKlarrann::InitOrbAttack2DFx()
{
	m_nMotionBlurIndex = g_pFcFXManager->Create( FX_TYPE_XM );
	g_pFcFXManager->SendMessage( m_nMotionBlurIndex, FX_INIT_OBJECT );
}


void CKlarrann::LoadWeaponTrailTexture()
{
	int nTextureId;
	g_BsKernel.chdir("Fx");
	
	nTextureId = g_BsKernel.LoadTexture( "Klarrann_SwordTrail01.dds" );
	if( nTextureId != -1 ) 
		m_WeaponTrailTexIDs.push_back(nTextureId);
	nTextureId = -1;

	nTextureId = g_BsKernel.LoadTexture( "Klarrann_SwordTrail02.dds" );
	if( nTextureId != -1 ) 
		m_WeaponTrailTexIDs.push_back(nTextureId);
	nTextureId = -1;
	
	m_nWeaponTrailOffsetTexID = g_BsKernel.LoadTexture( "Klarrann_SwordTrail_Normals.dds" );
	g_BsKernel.chdir("..");
}
