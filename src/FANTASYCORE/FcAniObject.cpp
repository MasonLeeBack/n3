#include "StdAfx.h"
#include "BsKernel.h"
#include "Input.h"

#include "FcAniObject.h"
#include "FcPhysicsLoader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

CFcAniObject::CFcAniObject( CCrossVector *pCross, bool bInsertProcess/* = true*/ )
	: CFcBaseObject( pCross, bInsertProcess )
{
	ResetAniIndex();
	m_fPrevFrame=0.0f;
	SetFrameAdd( 1.0f );

	m_nChangeAni=-1;
	m_fChangeFrame=0.0f;
	m_nBlendFrame=0;
	m_nStiffFrame = 0;
/*
	m_fGravity=DEFAULT_GRAVITY;
	m_fXVelocity=0.0f;
	m_fYVelocity=0.0f;
	m_fZVelocity=0.0f;
*/
	m_pCollisionMesh = NULL;	
	m_pRagdollSetting = NULL;	
	m_nRagDollMode = RAGDOLL_MODE_NONE;
}

CFcAniObject::~CFcAniObject()
{
	if( m_pCollisionMesh ) {
		CFcPhysicsLoader::ClearCollisionMesh( m_pCollisionMesh );
		m_pCollisionMesh = NULL;		
	}

	if( m_pRagdollSetting && m_bRagdollSettingDelete ) {
		CFcPhysicsLoader::ClearRagdollSetting( m_pRagdollSetting);
		m_pRagdollSetting= NULL;
	}	
}

void CFcAniObject::ResetAniIndex()
{
	m_nAniIndex = 0;
	m_fFrame = 0.0f;
}

void CFcAniObject::Process()
{
	m_fPrevFrame = m_fFrame;
	m_fFrame += GetFrameAdd();

	CheckAnimationEnd();
}


void CFcAniObject::PostProcess()
{
	if( m_nChangeAni != -1 )
	{
		m_nBlendFrame--;
		if( m_nBlendFrame <= 0 )
		{
			m_nChangeAni =- 1;
		}
		else
		{
			m_fBlendWeight = ( m_nBlendFrame + 1 ) / ( float )m_nBaseBlendFrame;
		}
	}
}

bool CFcAniObject::Render()
{
	if( m_nRagDollMode == RAGDOLL_MODE_LINK )
	{
		return false;
	}

	g_BsKernel.SetCurrentAni( m_nEngineIndex, m_nAniIndex, ( float )( int )( m_fFrame ) );

	if( m_nChangeAni != -1 )
	{
		g_BsKernel.BlendAni( m_nEngineIndex, m_nChangeAni, ( float )( int )( m_fChangeFrame ), m_fBlendWeight );
	}

	return CFcBaseObject::Render();
}

int CFcAniObject::Initialize(int nSkinIndex, int nAniIndex, int nShadowMeshIndex/*=-1*/)
{
	m_nEngineIndex=CBsKernel::GetInstance().CreateAniObjectFromSkin(nSkinIndex, nAniIndex);

	return 1;
}

void CFcAniObject::SetAnimation( int nAniIndex, int nBlendFrame/*=6*/ )
{
	if( m_nAniIndex != nAniIndex )
	{
		if(m_nBlendFrame<=0)	// 이전 블렌딩이 아직 끝나지 않았으면 이런 블렌딩을 계속하게 한다..
		{	
			m_nBaseBlendFrame = nBlendFrame + 1;
			m_nBlendFrame = nBlendFrame;
			m_nChangeAni = m_nAniIndex;
			m_fChangeFrame = m_fFrame - GetFrameAdd();
			if( m_fChangeFrame < 0 )
			{
				m_fChangeFrame = 0;
			}
		}
	}

	m_nAniIndex = nAniIndex;
	m_fPrevFrame = 0.0f;
	m_fFrame = 0.0f; 
}

void CFcAniObject::NextAnimation()
{
	m_fPrevFrame = 0.0f;
	m_fFrame = 0.0f;
}

void CFcAniObject::CreateCollisionMesh(const char *pFileName)
{
	BsAssert( m_pCollisionMesh == NULL);
	m_pCollisionMesh = CFcPhysicsLoader::LoadCollisionMesh( pFileName );
    g_BsKernel.SendMessage( m_nEngineIndex, BS_PHYSICS_COLLISIONMESH, (DWORD) m_pCollisionMesh);
}

void CFcAniObject::SetCollisionMesh(PHYSICS_COLLISION_CONTAINER *pData)
{	
	g_BsKernel.SendMessage( m_nEngineIndex, BS_PHYSICS_COLLISIONMESH, (DWORD) pData);
}

void CFcAniObject::CreateRagdoll(const char *pFileName)
{
	BsAssert( m_pRagdollSetting == NULL) ;
	m_pRagdollSetting = CFcPhysicsLoader::LoadRagdollSetting( pFileName );
	m_bRagdollSettingDelete = true;
}

void CFcAniObject::SetRagdoll(PHYSICS_RAGDOLL_CONTAINER *pData)
{
	BsAssert( m_pRagdollSetting == NULL) ;
	m_pRagdollSetting = pData;
	m_bRagdollSettingDelete = false;
}

void CFcAniObject::RunRagdoll( int nSetting )
{
	g_BsKernel.SetPreCalcAni( m_nEngineIndex, true);
	g_BsKernel.SendMessage(m_nEngineIndex, BS_PHYSICS_RAGDOLL, (DWORD)m_pRagdollSetting, nSetting );
}

int GetProcessTick();
void CFcAniObject::CheckAnimationEnd()
{
	int nAniLength;

	nAniLength = g_BsKernel.GetAniLength( m_nEngineIndex, m_nAniIndex );

	if( ( nAniLength - 1 ) < m_fFrame )
	{
		if( m_nStiffFrame )
		{
			m_nStiffFrame--;
			m_fFrame -= GetFrameAdd();
			if( m_fFrame < 0 )
			{
				m_fFrame = 0.0f;
			}
			m_fPrevFrame = m_fFrame;
		}
		else
		{
			NextAnimation();
		}
	}
}

