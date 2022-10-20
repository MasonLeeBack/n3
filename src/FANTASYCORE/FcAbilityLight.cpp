#include "stdafx.h"
#include "FcGameObject.h"
#include "FcAbilityLight.h"
#include "FcWorld.h"
#include "FcFxManager.h"
#include "BsImageProcess.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG



#define ABIL_LIGHT_RADIUS		25.f
#define ABIL_LIGHT_FRAME		2





CAbLightLomoTable::CAbLightLomoTable(float fStartValue)
{
	m_nTable = 8;
	m_pTable = new TimeValue[m_nTable];

	m_pTable[0]= TimeValue(62, fStartValue);
	m_pTable[1]= TimeValue(65, 1.f);
	m_pTable[2]= TimeValue(114, 1.f);
	m_pTable[3]= TimeValue(128, 0.f);
	m_pTable[4]= TimeValue(130, 0.f);
	m_pTable[5]= TimeValue(135, 1.f);
	m_pTable[6]= TimeValue(230, 1.f);
	m_pTable[7]= TimeValue(280, fStartValue);
}

CAbLightSceneIntensityTable::CAbLightSceneIntensityTable(float fStartValue)
{
	m_nTable = 11;
	m_pTable = new TimeValue[m_nTable];

	m_pTable[0] = TimeValue(30,  fStartValue);
	m_pTable[1] = TimeValue(45,  0.4f);
	m_pTable[2] = TimeValue(50,  fStartValue);
	m_pTable[3] = TimeValue(56,  fStartValue);
	m_pTable[4] = TimeValue(64,  0.2f);
	m_pTable[5] = TimeValue(95,  fStartValue);
	m_pTable[6] = TimeValue(120, fStartValue);
	m_pTable[7]	= TimeValue(126, 0.2f);
	m_pTable[8] = TimeValue(130, 0.8f);
	m_pTable[9]	= TimeValue(152, 0.4f);
	m_pTable[10]	= TimeValue(195, fStartValue);


}

CAbLightGlowIntensityTable::CAbLightGlowIntensityTable(float fStartValue)
{
	m_nTable = 13;

	m_pTable = new TimeValue[m_nTable];

	m_pTable[0] = TimeValue(0,  0.0f);
	m_pTable[1] = TimeValue(15, 0.2f);
	m_pTable[2] = TimeValue(39, 0.6f);
	m_pTable[3] = TimeValue(50, 0.1f);
	m_pTable[4] = TimeValue(54, 0.0f);
	m_pTable[5] = TimeValue(58, 0.0f);
	m_pTable[6] = TimeValue(66, 0.8f);
	m_pTable[7] = TimeValue(76, 0.4f);
	m_pTable[8] = TimeValue(100, 0.0f);
	m_pTable[9] = TimeValue(126, 0.7f);
	m_pTable[10] = TimeValue(139, 0.2f);
	m_pTable[11] = TimeValue(170, 0.0f);
	m_pTable[12] = TimeValue(220, fStartValue);
	
}

CAbLightHighlightTable::CAbLightHighlightTable(float fStartValue)
{
	m_nTable = 9;

	m_pTable = new TimeValue[m_nTable];
	m_pTable[0] = TimeValue(32,  fStartValue);
	m_pTable[1] = TimeValue(45,  0.3f);
	m_pTable[2] = TimeValue(58,  fStartValue);
	m_pTable[3] = TimeValue(60,  fStartValue);
	m_pTable[4] = TimeValue(68,  0.2f);
	m_pTable[5] = TimeValue(75,  fStartValue);
	m_pTable[6]	= TimeValue(125, fStartValue);
	m_pTable[7]	= TimeValue(127, 0.3f);
	m_pTable[8]	= TimeValue(160, fStartValue);
}





//const int c_lomo_step = 8;

CFcAbilityLight::CFcAbilityLight( GameObjHandle hParent ) : CFcAbilityBase( hParent )
{
	int i = 0;

	m_nCurFrame = 0;
	m_nCurCmdCount = 0;
	m_Pos = D3DXVECTOR3(0,0,0);
	m_fMaxRadius = 0.f;
	m_nMaxFrame  = 0;

	char szFxFilePath[MAX_PATH];

	g_BsKernel.chdir("Fx");
	sprintf(szFxFilePath,"%sFX_HC_OL2.bfx",g_BsKernel.GetCurrentDirectory() );
	m_nFXID = g_BsKernel.LoadFXTemplate(-1, szFxFilePath);
	g_BsKernel.chdir("..");

	m_Cross.Reset();
	m_nObjID = -1;
	m_bThrowPath = true;
	m_nLomoFXID = -1;

	m_fOrgSceneIntensity = 0.f;
	m_fOrgBlurIntensity = 0.f;
	m_fOrgGlowIntensity = 0.f;
	m_fOrgHighlightTredhold = 0.f;



	m_pLomoTable = NULL;
	m_pSceneTable = NULL;
	m_pGlowTable = NULL;
	m_pHlightTable = NULL;
}

CFcAbilityLight::~CFcAbilityLight()
{
	Finalize();

	if(m_pLomoTable)
		delete m_pLomoTable, m_pLomoTable = NULL;

	if(m_pSceneTable)
		delete m_pSceneTable, m_pSceneTable = NULL;
	
	if(m_pGlowTable)
		delete m_pGlowTable, m_pGlowTable = NULL;

	if(m_pHlightTable)
		delete m_pHlightTable, m_pHlightTable = NULL;

	if ( m_nFXID != -1 ) {
		g_BsKernel.ReleaseFXTemplate( m_nFXID );
		m_nFXID = -1;
	}
}


void CFcAbilityLight::Initialize(DWORD dwParam1 /*= 0*/, DWORD dwParam2 /*= 0*/)
{
	m_Cross.Reset();
	m_Cross.m_PosVector = m_hParent->GetPos() + (m_hParent->GetCrossVector()->m_ZVector) * 3500.0f;

	float	fMapXSize, fMapYSize;
	g_FcWorld.GetMapSize( fMapXSize, fMapYSize );
	if (m_Cross.m_PosVector.x >= 0.0f && m_Cross.m_PosVector.x < fMapXSize && 
		m_Cross.m_PosVector.z >= 0.0f && m_Cross.m_PosVector.z < fMapYSize)
		m_Cross.m_PosVector.y = g_FcWorld.GetLandHeight(m_Cross.m_PosVector.x, m_Cross.m_PosVector.z);

//////

	BsAssert( m_nObjID == -1 );

    m_nObjID = g_BsKernel.CreateFXObject( m_nFXID );
    if( m_nObjID == -1 ) return;

    g_BsKernel.SetFXObjectState( m_nObjID, CBsFXObject::PLAY );
	g_BsKernel.UpdateObject( m_nObjID, m_Cross );

//////
	
	m_nLomoFXID = g_pFcFXManager->Create(FX_TYPE_LOMOFILTER);
	g_pFcFXManager->SendMessage(m_nLomoFXID, FX_INIT_OBJECT);
//	g_pFcFXManager->SendMessage(m_nLomoFXID, FX_PLAY_OBJECT);

	m_fOrgSceneIntensity = g_BsKernel.GetImageProcess()->GetSceneIntensity();
	m_fOrgBlurIntensity = g_BsKernel.GetImageProcess()->GetBlurIntensity();
	m_fOrgGlowIntensity = g_BsKernel.GetImageProcess()->GetGlowIntensity();
	m_fOrgHighlightTredhold = g_BsKernel.GetImageProcess()->GetHighlightThreshold();

	m_pLomoTable	= new CAbLightLomoTable(0.0f);
	m_pSceneTable	= new CAbLightSceneIntensityTable(1.f);
	m_pGlowTable	= new CAbLightGlowIntensityTable(m_fOrgGlowIntensity);
	m_pHlightTable	= new CAbLightHighlightTable(m_fOrgHighlightTredhold);

	g_FcWorld.InitTrueOrbDamagedTroops();
}


void CFcAbilityLight::Process()
{
	int i = 0;

	m_nCurFrame++;

	if (m_nCurFrame == 40)
		g_FcWorld.GiveRangeDamageLight( m_hParent,&m_Cross.m_PosVector,3500.f, 2000.0f, -0.1f, 0,HT_ABIL_LIGHT,m_nOrbGenType,m_nOrbGenPer );
	
	if (m_nCurFrame == 132)
	{
		g_FcWorld.GiveRangeDamageLight(m_hParent,&m_Cross.m_PosVector,3500.f, 2500.0f, 1.5f, 1,HT_ABIL_LIGHT,m_nOrbGenType,m_nOrbGenPer );
		g_FcWorld.GivePhysicsRange( m_hParent,&m_Cross.m_PosVector,3500.f, 500.0f, 1000000, PROP_BREAK_TYPE_SPECIAL_ATTACK );
	}

	if (m_nCurFrame == 62)
		g_pFcFXManager->SendMessage(m_nLomoFXID, FX_PLAY_OBJECT);

	if(m_nCurFrame >= 62 )
	{

		float fResult = m_pLomoTable->GetVaule(m_nCurFrame) ;
		g_pFcFXManager->SendMessage(m_nLomoFXID, FX_SET_LOMOINTENSITY, ( DWORD)&fResult);
	}

	float fScene = m_pSceneTable->GetVaule(m_nCurFrame);
	g_BsKernel.GetImageProcess()->SetSceneIntensity(fScene*m_fOrgSceneIntensity);
	g_BsKernel.GetImageProcess()->SetBlurIntensity(fScene*m_fOrgBlurIntensity);
	float fGlow = m_pGlowTable->GetVaule(m_nCurFrame);
	g_BsKernel.GetImageProcess()->SetGlowIntensity(fGlow);
	float fHight = m_pHlightTable->GetVaule(m_nCurFrame);
	g_BsKernel.GetImageProcess()->SetHighlightThreshold(fHight);



///////////////////
	if( g_BsKernel.GetFXObjectState( m_nObjID ) == CBsFXObject::STOP ) 
	{
		m_bFinished = TRUE;	
		return;
	}

/////////////////
}


void CFcAbilityLight::Update()
{
	g_BsKernel.UpdateObject(m_nObjID, m_Cross);	
}


void CFcAbilityLight::Finalize()
{
///////////////
	g_BsKernel.DeleteObject(m_nObjID);
	/*g_pFcFXManager->SendMessage(m_nLomoFXID, FX_DELETE_OBJECT);
	m_nLomoFXID = -1;*/
	SAFE_DELETE_FX(m_nLomoFXID, FX_TYPE_LOMOFILTER);
	g_BsKernel.GetImageProcess()->SetSceneIntensity(m_fOrgSceneIntensity);
	g_BsKernel.GetImageProcess()->SetBlurIntensity(m_fOrgBlurIntensity);
	g_BsKernel.GetImageProcess()->SetGlowIntensity(m_fOrgGlowIntensity);
	g_BsKernel.GetImageProcess()->SetHighlightThreshold(m_fOrgHighlightTredhold);

//////////////
	g_FcWorld.FinishTrueOrbDamagedTroops();
}