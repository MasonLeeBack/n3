#include "stdafx.h"
#include "FcGameObject.h"
#include "FcAbility1KArrow.h"
#include "FCUtil.h"
#include "FcFXManager.h"
#include "FcWorld.h"



CFcAbility1KArrow::CFcAbility1KArrow( GameObjHandle hParent ): CFcAbilityBase( hParent )
{
	m_fSpd = 0.f;
}


CFcAbility1KArrow::~CFcAbility1KArrow()
{
	Finalize();
}

void CFcAbility1KArrow::Initialize(DWORD dwParam1/* = 0*/, DWORD dwParam2/* = 0*/) 
{
	m_vPos = m_hParent->GetPos();
	g_FcWorld.GetEnemyObjectListInRange(m_hParent->GetTeam(), &m_vPos, 1000.f, m_Enemy );

	m_vPos.y += 1000.f;
	m_fSpd	= 30.f;

	float nE = 25.f;
	float nW = 0.8f;

	g_BsKernel.chdir("fx");
	static char szFile[128] = "arrowline.dds";
	g_BsKernel.LoadTexture( szFile );
	g_BsKernel.chdir("..");

	for(int ii = 1; ii < _MAX_ARROW_COUNT; ++ii)
	{
		m_Arrows[ii].m_nEngineIdx = g_BsKernel.CreateStaticObjectFromSkin(dwParam1);
		m_Arrows[ii].m_Cross.m_ZVector.x = RandomNumberInRange( -.5f, .5f);
		m_Arrows[ii].m_Cross.m_ZVector.y = 1.f;
		m_Arrows[ii].m_Cross.m_ZVector.z = RandomNumberInRange( -.5f, .5f);
		m_Arrows[ii].m_Cross.m_PosVector = m_vPos;

		m_Arrows[ii].m_Cross.UpdateVectors();
		m_Arrows[ii].m_vVelocity = m_Arrows[ii].m_Cross.m_ZVector * m_fSpd;


		m_Arrows[ii].m_nFxIdx = g_pFcFXManager->Create(FX_TYPE_LINETRAIL);
		if( m_Arrows[ii].m_nFxIdx == -1 ) 
			continue;

		g_pFcFXManager->SendMessage( m_Arrows[ii].m_nFxIdx, FX_INIT_OBJECT, 10, (DWORD)&nW, (DWORD)&nE );
		g_pFcFXManager->SendMessage( m_Arrows[ii].m_nFxIdx, FX_SET_TEXTURE, (DWORD)szFile );
		g_pFcFXManager->SendMessage( m_Arrows[ii].m_nFxIdx, FX_PLAY_OBJECT );

	}

	m_Arrows[0].m_nEngineIdx = g_BsKernel.CreateStaticObjectFromSkin(dwParam1);
	m_Arrows[0].m_Cross.m_ZVector.x = 0.f;
	m_Arrows[0].m_Cross.m_ZVector.y = 1.f;
	m_Arrows[0].m_Cross.m_ZVector.z = 0.f;
	m_Arrows[0].m_Cross.m_PosVector = m_vPos;

	m_Arrows[0].m_Cross.UpdateVectors();
	m_Arrows[0].m_vVelocity = m_Arrows[0].m_Cross.m_ZVector * m_fSpd;
}


void CFcAbility1KArrow::Process() 
{
	for(int ii = 0; ii <  _MAX_ARROW_COUNT; ++ii)
	{
		m_Arrows[ii].m_Cross.m_PosVector += m_Arrows[ii].m_vVelocity;
		m_Arrows[ii].m_vVelocity.y -= 1.2f;
		D3DXVec3Normalize( &(m_Arrows[ii].m_Cross.m_ZVector), &(m_Arrows[ii].m_vVelocity) );
		m_Arrows[ii].m_Cross.UpdateVectors();

		g_pFcFXManager->SendMessage( m_Arrows[ii].m_nFxIdx, FX_UPDATE_OBJECT, (DWORD)&m_Arrows[ii].m_Cross.m_PosVector );
	}

	D3DXVECTOR3 vPush;
	std::vector< GameObjHandle >::iterator it;
	it = m_Enemy.begin();

	float fArrowHeight = m_Arrows[0].m_Cross.m_PosVector.y;
	while( it != m_Enemy.end() )
	{
		if( (*it)->GetPos().y+(*it)->GetUnitHeight() < fArrowHeight )
		{
			it++;
			continue;
		}

		vPush = D3DXVECTOR3(0.f, 0.f, 0.f);
		D3DXVec3Normalize(&vPush, &vPush);
		CFcWorld::GetInstance().GiveDamage( m_hParent, (*it), m_hParent->GetAttackPower() , ANI_TYPE_HIT, 0, &vPush , 0, 0, NULL, HT_NORMAL_ATTACK);
		it = m_Enemy.erase( it );
	}


	if( (fArrowHeight + 200.f) < g_FcWorld.GetLandHeight( m_Arrows[0].m_Cross.m_PosVector.x, m_Arrows[0].m_Cross.m_PosVector.z )  )
		m_bFinished = TRUE;
}

void CFcAbility1KArrow::Update() 
{
	for(int ii = 0; ii <  _MAX_ARROW_COUNT; ++ii)
	{
		if(m_Arrows[ii].m_nEngineIdx != -1)
			g_BsKernel.UpdateObject(m_Arrows[ii].m_nEngineIdx, m_Arrows[ii].m_Cross);
	}
}


void CFcAbility1KArrow::Finalize() 
{
	for(int ii = 0; ii <  _MAX_ARROW_COUNT; ++ii)
	{
		if(m_Arrows[ii].m_nEngineIdx != -1)
		{
			g_BsKernel.DeleteObject(m_Arrows[ii].m_nEngineIdx);
			m_Arrows[ii].m_nEngineIdx = -1;
		}

		/*if( m_Arrows[ii].m_nFxIdx != -1 ) {
			g_pFcFXManager->SendMessage(m_Arrows[ii].m_nFxIdx, FX_DELETE_OBJECT);
			m_Arrows[ii].m_nFxIdx = -1;
		}*/
		SAFE_DELETE_FX(m_Arrows[ii].m_nFxIdx,FX_TYPE_LINETRAIL);
	}
}