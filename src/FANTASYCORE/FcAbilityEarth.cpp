 #include "stdafx.h"
#include "FcGameObject.h"
#include "FcAbilityEarth.h"
#include "FcFxManager.h"
#include "FcFXCommon.h"
#include "FcWorld.h"
#include "BsImageProcess.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG



CAbEarthSceneIntensityTable::CAbEarthSceneIntensityTable(float fStartValue) // 화면 밝기. highlight과는 별도
{
	m_nTable = 6;

	m_pTable = new TimeValue[m_nTable];

	m_pTable[0] = TimeValue(0, fStartValue);
	m_pTable[1] = TimeValue(5,	fStartValue + 1.0f);
	m_pTable[2] = TimeValue(10,	fStartValue);
	m_pTable[3] = TimeValue(80, fStartValue);
	m_pTable[4] = TimeValue(90, fStartValue + 2.0f);
	m_pTable[5] = TimeValue(95, fStartValue);

}

CAbEarthGlowIntensityTable::CAbEarthGlowIntensityTable(float fStartValue) // highlight 세기
{
	m_nTable = 6;

	m_pTable = new TimeValue[m_nTable];

	m_pTable[0] = TimeValue(0, fStartValue);
	m_pTable[1] = TimeValue(10,	fStartValue - 1.0f);
	m_pTable[2] = TimeValue(25,	fStartValue);
	m_pTable[3] = TimeValue(90,	fStartValue);
	m_pTable[4] = TimeValue(100, fStartValue - 2.0f);
	m_pTable[5] = TimeValue(150,fStartValue);

}

CAbEarthHighlightTable::CAbEarthHighlightTable(float fStartValue)	// highlight 영역
{
	m_nTable = 6;

	m_pTable = new TimeValue[m_nTable];

	m_pTable[0] = TimeValue(0, fStartValue);
	m_pTable[1] = TimeValue(15,	fStartValue + 0.75f);
	m_pTable[2] = TimeValue(20,	fStartValue);
	m_pTable[3] = TimeValue(80, fStartValue);
	m_pTable[4] = TimeValue(95,fStartValue - 0.7f);
	m_pTable[5] = TimeValue(150, fStartValue);
}



CFcAbilityEarth::CFcAbilityEarth( GameObjHandle hParent ) : CFcAbilityBase( hParent )
{
	m_nFXEarthId = -1;

	// Parse Vari..
	m_fDistFromCaster = 10000.f;

//	m_fScale = 1200.f;

	//현재 EarthExp가 전체 135프레임 


	char szFxFilePath[MAX_PATH];

	g_BsKernel.chdir("Fx");
	sprintf(szFxFilePath,"%sFX_HC_OE.bfx",g_BsKernel.GetCurrentDirectory() );
	m_nFXId = g_BsKernel.LoadFXTemplate(-1, szFxFilePath);

	m_nEarthStormTex = g_BsKernel.LoadTexture("EarthOrb2d.dds");
	m_nEarthStormTexExp = g_BsKernel.LoadTexture("EarthOrb2dDustExp.dds");
	g_BsKernel.chdir("..");

	m_Cross.Reset();
	m_nObjID = -1;
	m_nFX2DEFFECT = -1;
	m_nFX2DEFFECTExp = -1;
	m_bStartExpfx = FALSE;

	m_nCastFXId = -1;
	m_nMotionBlurIndex = -1;

	m_pSceneTable = NULL;
	m_pGlowTable = NULL;
	m_pHlightTable = NULL;
	m_nCastTime = 0;

}

CFcAbilityEarth::~CFcAbilityEarth()
{
	Finalize();
}

void CFcAbilityEarth :: Initialize(DWORD dwParam1 /*= 0*/, DWORD dwParam2 /*= 0*/)
{
	const	float	fHeightFromCaster = 0.0f;
	m_dwTick = 0;
	m_dwExpfxStartTick = 0;
	// Parse Vari..
	m_fDistFromCaster = 9000.f;

	m_fScale = 1000.f;

	m_nCastFXId = g_pFcFXManager->Create(FX_TYPE_CASTEARTH);
	
	if( m_nCastFXId == -1)
		return;
//	m_nFXEarthId = g_pFcFXManager->Create(FX_TYPE_EARTHEXPLODE);
//	if( m_nFXEarthId == -1)
//		return;

	m_nFX2DEFFECT= g_pFcFXManager->Create(FX_TYPE_2DFSEFFECT);
	if( m_nFX2DEFFECT == -1)
		return;
	g_pFcFXManager->SendMessage(m_nFX2DEFFECT, FX_INIT_OBJECT);
	g_pFcFXManager->SendMessage(m_nFX2DEFFECT, FX_PLAY_OBJECT);
	g_pFcFXManager->SendMessage(m_nFX2DEFFECT, FX_SET_TEXTURE, m_nEarthStormTex);


	m_nFX2DEFFECTExp = g_pFcFXManager->Create(FX_TYPE_2DFSEFFECT);
	if( m_nFX2DEFFECTExp == -1)
		return;
	g_pFcFXManager->SendMessage(m_nFX2DEFFECTExp, FX_INIT_OBJECT);
	g_pFcFXManager->SendMessage(m_nFX2DEFFECTExp, FX_SET_TEXTURE, m_nEarthStormTexExp);
	g_pFcFXManager->SendMessage(m_nFX2DEFFECTExp, FX_SET_FSINTERVAL, 2);

	m_Cross.Reset();
	m_Cross.m_PosVector = m_hParent->GetPos() + (m_hParent->GetCrossVector()->m_ZVector) * m_fDistFromCaster;
	m_Cross.m_PosVector.y += fHeightFromCaster;

	D3DXVECTOR3 CharPos = m_hParent->GetPos();
	D3DXVECTOR4 CenterPos(CharPos.x, CharPos.y, CharPos.z, 5.0f );
	D3DXVECTOR4 InhalePos(m_Cross.m_PosVector.x, m_Cross.m_PosVector.y, m_Cross.m_PosVector.z, 1.0f);
	g_pFcFXManager->SendMessage(m_nCastFXId, FX_INIT_OBJECT, (DWORD)&CenterPos, (DWORD)&InhalePos );
	g_pFcFXManager->SendMessage(m_nCastFXId, FX_PLAY_OBJECT);

	m_nObjID = g_BsKernel.CreateFXObject( m_nFXId );

	g_BsKernel.UpdateObject( m_nObjID, m_Cross );
	g_BsKernel.SetFXObjectState( m_nObjID, CBsFXObject::PLAY );


	m_nMotionBlurIndex = g_pFcFXManager->Create(FX_TYPE_2DFSEFFECT);
	g_pFcFXManager->SendMessage(m_nMotionBlurIndex, FX_INIT_OBJECT);
	g_pFcFXManager->SendMessage(m_nMotionBlurIndex, FX_SET_FSDEVIDE, 1);
	g_pFcFXManager->SendMessage(m_nMotionBlurIndex, FX_SET_TEXTURE, g_BsKernel.GetImageProcess()->GetBackBufferTexture() );
	g_pFcFXManager->SendMessage(m_nMotionBlurIndex, FX_PLAY_OBJECT);


	g_FcWorld.GetEnemyObjectListInRange(m_hParent->GetTeam(), &m_Cross.m_PosVector, m_fDistFromCaster, m_Enemy );
	//g_FcWorld.GetActiveProps(m_Cross.m_PosVector,m_fDistFromCaster,m_vecProps);
	
	m_fOrgSceneIntensity = g_BsKernel.GetImageProcess()->GetSceneIntensity();
	m_fOrgGlowIntensity = g_BsKernel.GetImageProcess()->GetGlowIntensity();
	m_fOrgHighlightTredhold = g_BsKernel.GetImageProcess()->GetHighlightThreshold();

	m_pSceneTable	= new CAbEarthSceneIntensityTable(1.f);
	m_pGlowTable	= new CAbEarthGlowIntensityTable(m_fOrgGlowIntensity);
	m_pHlightTable	= new CAbEarthHighlightTable(m_fOrgHighlightTredhold);

	g_FcWorld.InitTrueOrbDamagedTroops();
}

void CFcAbilityEarth :: Finalize()
{
	/*if( m_nCastFXId != -1)
	{
		g_pFcFXManager->SendMessage(m_nCastFXId,FX_DELETE_OBJECT);
		m_nCastFXId = -1;
	}*/
	SAFE_DELETE_FX(m_nCastFXId, FX_TYPE_CASTEARTH);

	/*if( m_nFX2DEFFECT != -1)
	{
		g_pFcFXManager->SendMessage(m_nFX2DEFFECT,FX_DELETE_OBJECT);
		m_nFX2DEFFECT = -1;
	}*/
	SAFE_DELETE_FX(m_nFX2DEFFECT, FX_TYPE_2DFSEFFECT);

	/*if( m_nFX2DEFFECTExp != -1)
	{
		g_pFcFXManager->SendMessage(m_nFX2DEFFECTExp,FX_DELETE_OBJECT);
		m_nFX2DEFFECTExp = -1;
	}*/
	SAFE_DELETE_FX(m_nFX2DEFFECTExp, FX_TYPE_2DFSEFFECT);

	/*if( m_nMotionBlurIndex != -1)
	{
		g_pFcFXManager->SendMessage(m_nMotionBlurIndex,FX_DELETE_OBJECT);
		m_nMotionBlurIndex = -1;
	}*/
	SAFE_DELETE_FX(m_nMotionBlurIndex, FX_TYPE_2DFSEFFECT);

	g_BsKernel.GetImageProcess()->SetSceneIntensity(m_fOrgSceneIntensity);
	g_BsKernel.GetImageProcess()->SetGlowIntensity(m_fOrgGlowIntensity);
	g_BsKernel.GetImageProcess()->SetHighlightThreshold(m_fOrgHighlightTredhold);

	SAFE_RELEASE_TEXTURE(m_nEarthStormTex);
	SAFE_RELEASE_TEXTURE(m_nEarthStormTexExp);

	if ( m_nFXId != -1 )
	{
		g_BsKernel.ReleaseFXTemplate( m_nFXId );
		m_nFXId = -1;
	}

	g_BsKernel.DeleteObject(m_nObjID);

	g_FcWorld.FinishTrueOrbDamagedTroops();
}

void CFcAbilityEarth :: Process()
{
	float fAlpha;
	
	D3DXVECTOR3  vDist = m_Cross.m_PosVector - m_hParent->GetPos();

	// 대략 fx 커지는 속도가 틱당 45정도 .. ->사용후 200틱정도에 카메라에 도달
	float	fDist = D3DXVec3Length(&vDist);
	float fCurFxSize = (m_fDistFromCaster/200.f)*(float)m_dwTick;

	if(fCurFxSize<=fDist)
	{
		fAlpha = BsMin((fCurFxSize)/fDist, 1.f);
//		fAlpha *= fAlpha;
		fAlpha = (fAlpha*1.75f)-1.f;
		fAlpha = BsMax( 0.f, fAlpha);
	}
	else
	{
		fAlpha = (fCurFxSize - fDist)/((m_fDistFromCaster/200.f)*250.f-fDist);
		//fAlpha = 1.f - (fAlpha*fAlpha);
		fAlpha = 1.f - fAlpha;
	}
	g_pFcFXManager->SendMessage(m_nFX2DEFFECT, FX_SET_FSALPHA, (DWORD)&fAlpha);
	fAlpha*=0.8f;
	g_pFcFXManager->SendMessage(m_nMotionBlurIndex, FX_SET_FSALPHA, (DWORD)&fAlpha);


	float fScene = m_pSceneTable->GetVaule(m_dwTick);
	g_BsKernel.GetImageProcess()->SetSceneIntensity(fScene*m_fOrgSceneIntensity);
	float fGlow = m_pGlowTable->GetVaule(m_dwTick);
	g_BsKernel.GetImageProcess()->SetGlowIntensity(fGlow);
	float fHight = m_pHlightTable->GetVaule(m_dwTick);
	g_BsKernel.GetImageProcess()->SetHighlightThreshold(fHight);

	fAlpha = 0.f;
	if(m_bStartExpfx == FALSE)
	{
		float fTap = (fCurFxSize - fDist);
		fTap /= (m_fDistFromCaster/200.f);

		if(fTap > -8.f  && fTap < 24.f)
		{
			m_bStartExpfx = TRUE;
			g_pFcFXManager->SendMessage(m_nFX2DEFFECTExp, FX_PLAY_OBJECT);
			g_pFcFXManager->SendMessage(m_nFX2DEFFECTExp, FX_SET_FSALPHA, (DWORD)&fAlpha);
			m_dwExpfxStartTick = m_dwTick;
		}
	}
	else
	{
//		fAlpha = fabs(float(16-(m_dwTick-m_dwExpfxStartTick) )) /16.f;
		fAlpha = 1.f;

		if(m_dwTick-m_dwExpfxStartTick > 32)
			fAlpha = 0.f;
		g_pFcFXManager->SendMessage(m_nFX2DEFFECTExp, FX_SET_FSALPHA, (DWORD)&fAlpha);
	}

	

	if(m_dwTick >= 101)
	{
		DWORD dwTick = m_dwTick - 100;
		float fInnerRadius = (m_fDistFromCaster/95.f)*(float)(m_dwTick-1);
		float fOutRadius = (m_fDistFromCaster/95.f)*(float)(m_dwTick);
		DamageProcess( fCurFxSize, fInnerRadius);
	}

	if( m_dwTick == 101 )
	{
		g_FcWorld.GetActiveProps(m_Cross.m_PosVector,m_fDistFromCaster,m_vecProps);
		if(m_vecProps.size()){
			for( int ii = 0; ii < (int)m_vecProps.size() ; ii++)
			{
				CFcBreakableProp* pBreakable = (CFcBreakableProp*) m_vecProps[ii];
				if(pBreakable->IsCrushProp() || pBreakable->IsDynamicProp())
				{	
					D3DXVECTOR3 PropVec(pBreakable->GetPosV2().x,0,pBreakable->GetPosV2().y);			
					D3DXVECTOR3 Test = PropVec - m_hParent->GetCrossVector()->m_PosVector;
					D3DXVec3Normalize(&Test,&Test);
					Test = Test * 10.0f;
					pBreakable->AddForce( &Test, NULL, 1000000, PROP_BREAK_TYPE_SPECIAL_ATTACK );
				}
			}
			m_vecProps.clear();
		}
	}

	m_dwTick++;

	if( m_nObjID > -1 && g_BsKernel.GetFXObjectState( m_nObjID ) == CBsFXObject::STOP && m_dwTick >= 260) 
	{
		m_bFinished = TRUE;	
		return;
	}
}

void CFcAbilityEarth :: Update()
{
	/*m_nCastTime++;
	if( m_nCastTime < 150 ) return;*/

	if( m_nObjID > -1 )
		g_BsKernel.UpdateObject(m_nObjID,m_Cross);	
}



void CFcAbilityEarth :: DamageProcess(float fOutRadius, float fInnerRadius)
{
//	std::vector< GameObjHandle > Result;

//	g_FcWorld.GetObjectListInRange( &m_Cross.m_PosVector, fMaxRadius, Result );
	int nSize = ( int )m_Enemy.size();

	D3DXVECTOR3	Pos;
	D3DXVECTOR3	Err;
	D3DXVECTOR3 vPush;
	
	float fDist;

	std::vector< GameObjHandle >::iterator it;
	it = m_Enemy.begin();
	while( it != m_Enemy.end() )
	{
		GameObjHandle tmp = *it;

		// !! fix !!
		if( CFcBaseObject::IsValid( tmp ) == false  )
		{
			it = m_Enemy.erase( it );
			continue;
		}

		if( m_hParent->GetTeam() == tmp->GetTeam() )
		{
			it = m_Enemy.erase( it );
			continue;
		}

		/*if ( (*it)->GetTeam() == 0)
		{
			it++;
			continue;
		}*/

		Pos = (*it)->GetPos();
		Err = Pos - (m_Cross.m_PosVector);
		fDist = D3DXVec3Length(&Err);
		
		fOutRadius *= 1.5f;

		if( fDist >fOutRadius )
		{
			it++;
			continue;
		}

		float fForce = fDist/m_fDistFromCaster;
		vPush = D3DXVECTOR3(0.f, 1.f-fForce, -fForce*0.5f);
		D3DXVec3Normalize(&vPush, &vPush);
		vPush *= 100.f;
		CFcWorld::GetInstance().GiveDamage( m_hParent, (*it), 20000/*(*it)->GetMaxHP() + 100*/  , ANI_TYPE_DOWN, 0, &vPush , m_nOrbGenType, 1, &m_Cross.m_PosVector,HT_ABIL_EARTH,m_nOrbGenPer );
		it = m_Enemy.erase( it );
	}

	
	
}

void CFcAbilityEarth :: ReadSOX()
{

}