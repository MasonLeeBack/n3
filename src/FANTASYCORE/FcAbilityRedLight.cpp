
#include "stdafx.h"
#include "FcGameObject.h"
#include "FcAbilityRedLight.h"
#include "FcWorld.h"
#include "FcFxManager.h"
#include "BsImageProcess.h"
#include "FCUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG



#define ABIL_REDLIGHT_RADIUS		25.f
#define ABIL_REDLIGHT_FRAME		2



//const int c_lomo_step = 8;

CFcAbilityRedLight::CFcAbilityRedLight( GameObjHandle hParent ) : CFcAbilityBase( hParent )
{
	int i = 0;

	m_nCurFrame = 0;
	m_nCurCmdCount = 0;
	m_Pos = D3DXVECTOR3(0,0,0);
	m_fMaxRadius = 0.f;
	m_nMaxFrame  = 0;

	char szFxFilePath[MAX_PATH];

	g_BsKernel.chdir("Fx");
	sprintf(szFxFilePath,"%sFX_HC_OS.bfx",g_BsKernel.GetCurrentDirectory() );
	m_nFXID = g_BsKernel.LoadFXTemplate(-1, szFxFilePath);
	sprintf(szFxFilePath,"%sFX_HC_OS2.bfx",g_BsKernel.GetCurrentDirectory() );
	m_nFXID2 = g_BsKernel.LoadFXTemplate(-1, szFxFilePath);
	sprintf(szFxFilePath,"%sFX_HC_OS3.bfx",g_BsKernel.GetCurrentDirectory() );
	m_nFXID3 = g_BsKernel.LoadFXTemplate(-1, szFxFilePath);
	g_BsKernel.chdir("..");

	m_nCur =0;

	int nScan;
	for(nScan =0 ;nScan < MAX_LIGHTNING;nScan++)
	{
		m_Cross[nScan].Reset();
		m_nObjID[nScan] = -1;
	}

	m_CrossOrg.Reset();
	m_nTempSky = -1;
	m_nFinalObj = -1;

	m_bThrowPath = true;
	m_nLomoFXID = -1;

	m_fOrgSceneIntensity = 0.f;
	m_fOrgBlurIntensity = 0.f;
	m_fOrgGlowIntensity = 0.f;
	m_fOrgHighlightTredhold = 0.f;
	m_fStep = 0.1f;
	m_nFinalObjFrame = -1;
	m_nMotionBlurIndex = -1;
	m_fBlurAlpha = 0.7f;
}

CFcAbilityRedLight::~CFcAbilityRedLight()
{
	Finalize();
}


void CFcAbilityRedLight::Initialize(DWORD dwParam1 /*= 0*/, DWORD dwParam2 /*= 0*/)
{

	int nScan;

	m_nCur =0;

	m_nTempSky = g_BsKernel.CreateFXObject( m_nFXID2 );
	m_nFinalObj = g_BsKernel.CreateFXObject( m_nFXID3 );

	for( nScan =0; nScan < MAX_LIGHTNING;nScan++)
		m_nObjID[nScan] = g_BsKernel.CreateFXObject( m_nFXID );

	//////

	m_nLomoFXID = g_pFcFXManager->Create(FX_TYPE_LOMOFILTER);
	g_pFcFXManager->SendMessage(m_nLomoFXID, FX_INIT_OBJECT);
	//	g_pFcFXManager->SendMessage(m_nLomoFXID, FX_PLAY_OBJECT);

	m_fOrgSceneIntensity = g_BsKernel.GetImageProcess()->GetSceneIntensity();
	m_fOrgBlurIntensity = g_BsKernel.GetImageProcess()->GetBlurIntensity();
	m_fOrgGlowIntensity = g_BsKernel.GetImageProcess()->GetGlowIntensity();
	m_fOrgHighlightTredhold = g_BsKernel.GetImageProcess()->GetHighlightThreshold();

	m_fCurSI = m_fOrgSceneIntensity;
	m_fCurBI = m_fOrgBlurIntensity;
	m_fCurGI = m_fOrgGlowIntensity;
	m_fCurHT = m_fOrgHighlightTredhold;


	m_CrossOrg.m_PosVector = m_hParent->GetPos() + (m_hParent->GetCrossVector()->m_ZVector) * 3500.0f;

	m_CrossOrg.m_XVector = m_hParent->GetCrossVector()->m_XVector;
	m_CrossOrg.m_YVector = m_hParent->GetCrossVector()->m_YVector;
	m_CrossOrg.m_ZVector = m_hParent->GetCrossVector()->m_ZVector;
	
	g_BsKernel.SetFXObjectState( m_nTempSky, CBsFXObject::PLAY );
	g_BsKernel.UpdateObject(m_nTempSky,m_CrossOrg);

	m_fCurSI = 3.0f;
	m_fCurBI = 3.0f;
	m_fCurGI = 3.0f;
	m_fCurHT = 0.2f;
	m_fBlurAlpha = 0.7f;

	m_nMotionBlurIndex = g_pFcFXManager->Create(FX_TYPE_2DFSEFFECT);
	g_pFcFXManager->SendMessage(m_nMotionBlurIndex, FX_INIT_OBJECT);
	g_pFcFXManager->SendMessage(m_nMotionBlurIndex, FX_SET_FSDEVIDE, 1);
	g_pFcFXManager->SendMessage(m_nMotionBlurIndex, FX_SET_TEXTURE, g_BsKernel.GetImageProcess()->GetBackBufferTexture() );
	g_pFcFXManager->SendMessage(m_nMotionBlurIndex, FX_PLAY_OBJECT);

	g_pFcFXManager->SendMessage(m_nMotionBlurIndex, FX_SET_FSALPHA, (DWORD)&m_fBlurAlpha);

	g_FcWorld.InitTrueOrbDamagedTroops();
}


void CFcAbilityRedLight::Process()
{
	int i = 0;

	m_nCurFrame++;
	
	if ((m_nCurFrame % (Random(4) + 4)) == 0 && m_nCur < MAX_LIGHTNING)
	{

		m_Cross[m_nCur].Reset();

		float fYOff = (float)(Random(10000) - 1500);
		int	nR = (int)fYOff + 1500;

		float fPitch = ((float)nR) / 10000.0f * 180.0f;

		float fXOff = (float)(Random((nR * 3) / 2) - (nR * 3) / 4);
		float fRoll = fXOff / ((float)((nR * 3) / 4)) * 180.0f;

		m_Cross[m_nCur].m_PosVector = m_CrossOrg.m_PosVector + m_CrossOrg.m_XVector * fXOff + m_CrossOrg.m_ZVector * fYOff;
		
		float	fMapXSize, fMapYSize;
		g_FcWorld.GetMapSize( fMapXSize, fMapYSize );
		if (m_Cross[m_nCur].m_PosVector.x >= 0.0f && m_Cross[m_nCur].m_PosVector.x < fMapXSize && 
			m_Cross[m_nCur].m_PosVector.z >= 0.0f && m_Cross[m_nCur].m_PosVector.z < fMapYSize)
            m_Cross[m_nCur].m_PosVector.y = g_FcWorld.GetLandHeight(m_Cross[m_nCur].m_PosVector.x, m_Cross[m_nCur].m_PosVector.z);
		else
			m_Cross[m_nCur].m_PosVector.y = 0.0f;

		m_Cross[m_nCur].m_XVector = m_CrossOrg.m_XVector;
		m_Cross[m_nCur].m_YVector = m_CrossOrg.m_YVector;
		m_Cross[m_nCur].m_ZVector = m_CrossOrg.m_ZVector;

		m_Cross[m_nCur].RotateYaw(Random(180));
		m_Cross[m_nCur].RotatePitch((int)(fPitch));
		m_Cross[m_nCur].RotateRoll((int)(fRoll));

		g_BsKernel.SetFXObjectState( m_nObjID[m_nCur], CBsFXObject::PLAY );
		g_BsKernel.UpdateObject( m_nObjID[m_nCur], m_Cross[m_nCur] );

		D3DXVECTOR3 vPos = m_Cross[m_nCur].m_PosVector;
		_FC_RANGE_DAMAGE rd;
		rd.nType = HT_ABIL_RED_LIGHT;
		rd.Attacker = m_hParent;
		rd.pPosition = &vPos;
		rd.fRadius = 2500.f;
		rd.nPower = 2000;
		rd.nPushY = 40; 
		rd.nPushZ = 10;
		rd.nTeam = m_hParent->GetTeam();
		rd.nNotGenOrb = m_nOrbGenType;
		rd.nOrbGenPer = m_nOrbGenPer;
		g_FcWorld.GiveRangeDamage( rd );

		m_nCur++;

		float fDistFactor = fPitch / 180.0f;

		m_fCurSI = (2.0f - ((float)Random(10)) * 0.1f) * fDistFactor;

		if (Random(10)==0)
			m_fCurSI *= -1.0f;

		m_fCurBI = 1.5f * fDistFactor;
		m_fCurGI = 1.5f * fDistFactor;
		m_fCurHT = 0.55f * fDistFactor;

		std::vector<CFcProp *> vecProps;
		g_FcWorld.GetActiveProps(vPos,2500.f,vecProps);
		if(vecProps.size()){
			for( int ii = 0; ii < (int)vecProps.size() ; ii++)
			{
				CFcBreakableProp* pBreakable = (CFcBreakableProp*) vecProps[ii];
				if(pBreakable->IsCrushProp() || pBreakable->IsDynamicProp())
				{	
					D3DXVECTOR3 PropVec(pBreakable->GetPosV2().x,0,pBreakable->GetPosV2().y);			
					D3DXVECTOR3 Test = PropVec - m_hParent->GetCrossVector()->m_PosVector;
					D3DXVec3Normalize(&Test,&Test);
					Test = Test * 10.0f;
					pBreakable->AddForce( &Test, NULL, 1000000, PROP_BREAK_TYPE_SPECIAL_ATTACK );
				}
			}
			vecProps.clear();
		}
	}

	if (m_nCur == MAX_LIGHTNING 
		&& g_BsKernel.GetFXObjectState( m_nObjID[MAX_LIGHTNING-1] ) == CBsFXObject::PLAY
		&& m_nFinalObjFrame == -1)
	{
		m_nFinalObjFrame = m_nCurFrame;
	}

	if (m_nCurFrame == m_nFinalObjFrame + 15 && m_nFinalObjFrame != -1)
	{
		g_BsKernel.SetFXObjectState( m_nFinalObj, CBsFXObject::PLAY );
		g_BsKernel.UpdateObject( m_nFinalObj, m_CrossOrg );
	}

	if (m_nCurFrame == m_nFinalObjFrame + 72 && m_nFinalObjFrame != -1)
	{
		m_fCurSI = 1.2f;
		m_fCurBI = 1.2f;
		m_fCurGI = 1.2f;
		m_fCurHT = 0.35f;


		D3DXVECTOR3 vPos = m_CrossOrg.m_PosVector;
		float	fMapXSize, fMapYSize;
		g_FcWorld.GetMapSize( fMapXSize, fMapYSize );
		if (vPos.x >= 0.0f && vPos.x < fMapXSize && 
			vPos.z >= 0.0f && vPos.z < fMapYSize)
            vPos.y = g_FcWorld.GetLandHeight(vPos.x,vPos.z);
		else
			vPos.y = 0.0f;
		_FC_RANGE_DAMAGE rd;
		rd.nType = HT_ABIL_RED_LIGHT;
		rd.Attacker = m_hParent;
		rd.pPosition = &vPos;
		rd.fRadius = 3000.0f;
		rd.nPower = 2000;
		rd.nPushY = 70; 
		rd.nPushZ = 10;
		rd.nTeam = m_hParent->GetTeam();
		rd.nNotGenOrb = m_nOrbGenType;
		rd.nOrbGenPer = m_nOrbGenPer;
		g_FcWorld.GiveRangeDamage( rd );
		
		std::vector<CFcProp *> vecProps;
		g_FcWorld.GetActiveProps(vPos,3000.0f,vecProps);
		if(vecProps.size()){
			for( int ii = 0; ii < (int)vecProps.size() ; ii++)
			{
				CFcBreakableProp* pBreakable = (CFcBreakableProp*) vecProps[ii];
				if(pBreakable->IsCrushProp() || pBreakable->IsDynamicProp())
				{	
					D3DXVECTOR3 PropVec(pBreakable->GetPosV2().x,0,pBreakable->GetPosV2().y);			
					D3DXVECTOR3 Test = PropVec - m_hParent->GetCrossVector()->m_PosVector;
					D3DXVec3Normalize(&Test,&Test);
					Test = Test * 10.0f;
					pBreakable->AddForce( &Test, NULL, 1000000, PROP_BREAK_TYPE_SPECIAL_ATTACK );
				}
			}
			vecProps.clear();
		}

	}

	if (m_nCurFrame == m_nFinalObjFrame + 80 && m_nFinalObjFrame != -1)
	{
		m_fCurSI = 0.8f;
		m_fCurBI = 0.8f;
		m_fCurGI = 0.8f;
		m_fCurHT = 0.35f;
	}

	if (m_nFinalObjFrame != -1)
	{
		m_fStep = 0.04f;
		if (m_fBlurAlpha > 0.0f)
			m_fBlurAlpha -= 0.02f;
	}
	else
	{
		if (m_nCur == MAX_LIGHTNING)
			m_fStep = 0.5f + ((float)Random(10)) * .05f;
		else
			m_fStep = 0.1f + ((float)Random(10)) * .05f;
	
	}

	m_fCurSI += (m_fOrgSceneIntensity - m_fCurSI) * m_fStep;
	m_fCurBI += (m_fOrgBlurIntensity - m_fCurBI) * m_fStep;
	m_fCurGI += (m_fOrgGlowIntensity - m_fCurGI) * m_fStep;
	m_fCurHT += (m_fOrgHighlightTredhold - m_fCurHT) * m_fStep;

	g_BsKernel.GetImageProcess()->SetSceneIntensity(m_fCurSI);
	g_BsKernel.GetImageProcess()->SetBlurIntensity(m_fCurBI);
	g_BsKernel.GetImageProcess()->SetGlowIntensity(m_fCurGI);
	g_BsKernel.GetImageProcess()->SetHighlightThreshold(m_fCurHT);

	///////////////////
	if( g_BsKernel.GetFXObjectState( m_nFinalObj ) == CBsFXObject::STOP && 
		g_BsKernel.GetFXObjectState( m_nTempSky ) == CBsFXObject::STOP && 
		m_nCurFrame > m_nFinalObjFrame)  
	{
		m_bFinished = TRUE;	
		return;
	}

	
	/////////////////
}

void CFcAbilityRedLight::Update()
{
	int nScan;

	g_BsKernel.UpdateObject(m_nTempSky,m_CrossOrg);
	g_BsKernel.UpdateObject(m_nFinalObj,m_CrossOrg);
	for(nScan = 0;nScan < m_nCur; nScan++)
	{
		if( g_BsKernel.GetFXObjectState( m_nObjID[nScan] ) == CBsFXObject::PLAY ) 
			g_BsKernel.UpdateObject(m_nObjID[nScan],m_Cross[nScan]);	
	}
}


void CFcAbilityRedLight::Finalize()
{
	///////////////
	int nScan;

	for(nScan = 0;nScan < MAX_LIGHTNING;nScan++) {
		g_BsKernel.DeleteObject(m_nObjID[nScan]);
		m_nObjID[nScan] = -1;
	}

	/*g_pFcFXManager->SendMessage(m_nLomoFXID, FX_DELETE_OBJECT);
	m_nLomoFXID = -1;*/
	SAFE_DELETE_FX(m_nLomoFXID, FX_TYPE_LOMOFILTER);

	if ( m_nFXID != -1 )
	{
		g_BsKernel.ReleaseFXTemplate(m_nFXID);
		m_nFXID = -1;
	}

	if ( m_nFXID2 != -1 )
	{
		g_BsKernel.ReleaseFXTemplate(m_nFXID2);
		m_nFXID2 = -1;
	}

	if ( m_nFXID3 != -1 )
	{
		g_BsKernel.ReleaseFXTemplate(m_nFXID3);
		m_nFXID3 = -1;
	}

	g_BsKernel.GetImageProcess()->SetSceneIntensity(m_fOrgSceneIntensity);
	g_BsKernel.GetImageProcess()->SetBlurIntensity(m_fOrgBlurIntensity);
	g_BsKernel.GetImageProcess()->SetGlowIntensity(m_fOrgGlowIntensity);
	g_BsKernel.GetImageProcess()->SetHighlightThreshold(m_fOrgHighlightTredhold);

	/*if( m_nMotionBlurIndex != -1)
	{
		g_pFcFXManager->SendMessage(m_nMotionBlurIndex,FX_DELETE_OBJECT);
		m_nMotionBlurIndex = -1;
	}*/
	SAFE_DELETE_FX(m_nMotionBlurIndex, FX_TYPE_2DFSEFFECT);

	if (m_nTempSky != -1) {
		g_BsKernel.DeleteObject(m_nTempSky);
		m_nTempSky = -1;
	}

	if (m_nFinalObj != -1) {
		g_BsKernel.DeleteObject(m_nFinalObj);
		m_nFinalObj = -1;
	}

	//////////////

	g_FcWorld.FinishTrueOrbDamagedTroops();
}