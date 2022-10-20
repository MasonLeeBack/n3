#include "StdAfx.h"
#include ".\fcabilitycolumnofwater.h"
#include "FcWorld.h"
#include "FcHeroObject.h"
#include "FcFXManager.h"
#include "FcUtil.h"

class CFcGameObject;
typedef CSmartPtr<CFcGameObject> GameObjHandle;


CFcAbilityColumnOfWater::CFcAbilityColumnOfWater(GameObjHandle hParent)  : CFcAbilityBase( hParent )
{
	m_hParent = hParent;
	m_nFxTemplateID[SMALL_COLUMNWATER] = -1;
	m_nFxTemplateID[BIG_COLUMNWATER] = -1;
	m_nFxID = -1;
}

CFcAbilityColumnOfWater::~CFcAbilityColumnOfWater()
{
	Finalize();
}

void CFcAbilityColumnOfWater::Process()
{
	static int count = 0;
	if( ++count > 50 )		// 200 대신 전체타임 변수
	{
		count = 0;
		m_bFinished = TRUE;
	}

	if( count % 8 == 0 )	// 임시코드. 생성될 간격.
	{
		CreateColumnWater(SMALL_COLUMNWATER);
		g_pFcFXManager->SendMessage(m_nFxID, FX_PLAY_OBJECT);
	}

	if( count == 40 )
	{
		CreateColumnWater(BIG_COLUMNWATER);
		g_pFcFXManager->SendMessage(m_nFxID, FX_PLAY_OBJECT);
	}

}

void CFcAbilityColumnOfWater::CreateColumnWater( WaterType  p_nWaterType )
{
	D3DXVECTOR3 Pos;
	D3DXVECTOR3 vMin, vMax;
	Pos = m_vCharPos + m_vCharDir*800.0f;
	if( p_nWaterType == SMALL_COLUMNWATER )
	{
		vMax = Pos + m_vCharRight * 400 + m_vCharDir*100.0f;
		vMin = Pos - m_vCharRight * 400 - m_vCharDir*100.0f;
		Pos.x = RandomNumberInRange(vMin.x,vMax.x );
		Pos.z = RandomNumberInRange(vMin.z,vMax.z );

		Pos.y = g_BsKernel.GetLandHeight(  Pos.x,  Pos.z );
		Pos.y -= 30.0f;

		g_pFcFXManager->SendMessage(m_nFxID,FX_PLAY_OBJECT);
		g_pFcFXManager->SendMessage(m_nFxID,FX_UPDATE_OBJECT, (DWORD)&Pos, (DWORD)m_nFxTemplateID[SMALL_COLUMNWATER] );

		_FC_RANGE_DAMAGE rd;
		rd.Attacker = m_hParent;
		rd.pPosition = &Pos;
		rd.fRadius = 200.0f;
		rd.nPower = 80;
		rd.nPushY = 10; 
		rd.nPushZ = 20;
		rd.nTeam = -1;
		rd.nNotGenOrb = m_nOrbGenType;
		rd.nNotLookAtHitUnit = 1;
		rd.WeaponType = WT_TYURRU; 
		rd.SoundAttackType = ATT_LITTLE;
		rd.nOrbGenPer = m_nOrbGenPer;
		CFcWorld::GetInstance().GiveRangeDamage( rd );
	}
	else
	{
	
		Pos.y = g_BsKernel.GetLandHeight(  Pos.x,  Pos.z );
		Pos.y -= 30.0f;

		g_pFcFXManager->SendMessage(m_nFxID,FX_PLAY_OBJECT);
		g_pFcFXManager->SendMessage(m_nFxID,FX_UPDATE_OBJECT, (DWORD)&Pos, (DWORD)m_nFxTemplateID[BIG_COLUMNWATER] );

		_FC_RANGE_DAMAGE rd;
		rd.Attacker = m_hParent;
		rd.pPosition = &Pos;
		rd.fRadius = 350.0f;
		rd.nPower = 150;
		rd.nPushY = 20; 
		rd.nPushZ = 40;
		rd.nTeam = -1;
		rd.nNotGenOrb = m_nOrbGenType;
		rd.nNotLookAtHitUnit = 1;
		rd.WeaponType = WT_TYURRU; 
		rd.SoundAttackType = ATT_LITTLE;
		rd.nOrbGenPer = m_nOrbGenPer;

		CFcWorld::GetInstance().GiveRangeDamage( rd );
	}
	
}

void CFcAbilityColumnOfWater::Initialize()
{
	char szFxFilePath[MAX_PATH];
	g_BsKernel.chdir("fx");
	sprintf(szFxFilePath,"%s%s",g_BsKernel.GetCurrentDirectory(),"pump_shock_t2.bfx" );//aaa.bfx");//
	m_nFxTemplateID[SMALL_COLUMNWATER] = g_BsKernel.LoadFXTemplate(-1, szFxFilePath);

	sprintf(szFxFilePath,"%s%s",g_BsKernel.GetCurrentDirectory(),"pump_shock_t1.bfx" );//"aaa.bfx");//
	m_nFxTemplateID[BIG_COLUMNWATER] = g_BsKernel.LoadFXTemplate(-1, szFxFilePath);

	g_BsKernel.chdir("..");


	m_nFxID = g_pFcFXManager->Create(FX_TYPE_COLUMNWATER);
	g_pFcFXManager->SendMessage(m_nFxID,FX_INIT_OBJECT, 110);

	m_vCharPos		 = m_hParent->GetPos();
	m_vCharDir		 = m_hParent->GetCrossVector()->m_ZVector;
	D3DXVec3Cross(&m_vCharRight, &m_vCharDir, &D3DXVECTOR3(0,1,0) );
	D3DXVec3Normalize(&m_vCharRight,&m_vCharRight);

}

void CFcAbilityColumnOfWater::Finalize()
{
	if ( m_nFxTemplateID[BIG_COLUMNWATER] != -1 )
	{
		g_BsKernel.ReleaseFXTemplate( m_nFxTemplateID[BIG_COLUMNWATER] );
		m_nFxTemplateID[BIG_COLUMNWATER] = -1;
	}

	if ( m_nFxTemplateID[SMALL_COLUMNWATER] != -1 )
	{
		g_BsKernel.ReleaseFXTemplate( m_nFxTemplateID[SMALL_COLUMNWATER] );
		m_nFxTemplateID[SMALL_COLUMNWATER] = -1;
	}
}