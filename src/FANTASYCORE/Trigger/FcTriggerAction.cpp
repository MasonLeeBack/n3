#include "stdafx.h"
#include "FcTriggerAction.h"
#include "FcWorld.h"
#include "BSFileManager.h"
#include "FcTroopObject.h"
#include "FcInterfaceManager.h"
#include "FcTriggerCommander.h"
#include "FcGlobal.h"
#include "FcProp.h"
#include "FcCameraObject.h"
#include "BsSinTable.h"
#include "InputPad.h"
#include "FcHeroObject.h"
#include "FcUtil.h"
#include "FcSoundManager.h"
#include "FcRealtimeMovie.h"
#include "BSTriggerManager.h"
#include "FcTroopManager.h"
#include "DebugUtil.h"
#include "BSTriggerCodeDefine.h"
#include "FcFXManager.h"
#include "FcTroopAIObject.h"
#include "TextTable.h"
#include "FcPropManager.h"
#include "FcItem.h"
#include "FcAdjutantObject.h"
#include "FcSOXLoader.h"
#include "BsSkyBoxObject.h"
#include "CrossVector.h"
#include "FcGameObject.h"
#include "FcAIObject.h"


#include <limits>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

extern CTroopSOXLoader		g_TroopSOX;

int CFcActAIDisable::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	if( pTroopName == NULL )
	{
		DebugString("AIDisable invalid param!\n");
	}
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	g_FcWorld.EnableTroopAI( hTroop, false );
	return ACTION_STATE_FINISH;
}



int CFcActAIEnable::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	if( pTroopName == NULL )
	{
		DebugString("AIEnable invalid param!\n");
	}
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	g_FcWorld.EnableTroopAI( hTroop, true );
	return ACTION_STATE_FINISH;
}



int CFcActAISet::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	int nAIID = GetParamInt( 1 );

	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	g_FcWorld.SetTroopAI( hTroop, nAIID );
/*
	if( hTroop->IsEnable() == false )
		hTroop->CmdEnable( true );
*/

	return ACTION_STATE_FINISH;
}


int CFcActAISetPath::DoCommand()
{
	char* pPathName = GetParamStr(0);
	int nAIPathID = GetParamInt(1);
	g_FcWorld.SetPathToTroopAI( pPathName, nAIPathID );
	return ACTION_STATE_FINISH;
}



int CFcActBGMFade::DoCommand()
{
	int nSec = GetParamInt(0);
	int nPer = GetParamInt(1);
	g_pSoundManager->BGMFade(nSec * FRAME_PER_SEC,nPer);
	return ACTION_STATE_FINISH;
}


int CFcActBGMMute::DoCommand()
{
	bool bMute;
	if(GetParamInt(0)){ bMute = true;}
	else{ bMute = false; } 	
	g_pSoundManager->MuteBGM( bMute );
	return ACTION_STATE_FINISH;
}


int CFcActBGMPlay::DoCommand()
{
	int nBGMID = GetParamInt(0);
	int nLoop  = GetParamInt(1);

	if( g_FcWorld.GetBGMID() != nBGMID )
	{
		g_FcWorld.SetBGMID( nBGMID );
#ifdef _XBOX
		g_pSoundManager->PlayBGM( nBGMID, 0, 0 );
#endif
	}
	// nLoop 무조건 0
//	g_pSoundManager->PlayBGM( nBGMID , nLoop );
	/*if( g_pSoundManager->IsBGMPlay() ){	
		return ACTION_STATE_FINISH;
	}*/
	
	return ACTION_STATE_FINISH;
}



int CFcActBGMStop::DoCommand()
{
	g_pSoundManager->StopBGM();
	g_FcWorld.SetBGMID( -1 );
	return ACTION_STATE_FINISH;
}



int CFcActBGMVolume::DoCommand()
{
	//int nBGMID	  = GetParamInt(0);
	int nPercent  = GetParamInt(0);
	g_pSoundManager->SetVolumeBGM(nPercent);
	return ACTION_STATE_FINISH;
}



int CFcActCamReset::DoCommand()
{
	CameraObjHandle CamHandle;
	CamHandle = CFcBaseObject::GetCameraObjectHandle( 0 );
	CamHandle->SeqStop();
	CamHandle->SetUserTarget(g_FcWorld.GetHeroHandle()->GetTroop());
	return ACTION_STATE_FINISH;
}



int CFcActCamSet::DoCommand()
{
	int	nPresetId = GetParamInt( 0 );
	CameraObjHandle CamHandle = CFcBaseObject::GetCameraObjectHandle( 0 );
	CamHandle->SetUserSeq( nPresetId, 0, true );

	g_FcWorld.ResetUnitAILOD();

	DebugString( "ActCamSet %d\n", nPresetId );

	return ACTION_STATE_FINISH;
}



int CFcActCamTargetArea::DoCommand()
{
	char *pAreaName = GetParamStr( 0 );
	AREA_INFO* pInfo = g_FcWorld.GetAreaInfo( pAreaName );

	float fCX = ( pInfo->fEX - pInfo->fSX ) / 2;
	float fCZ = ( pInfo->fEZ - pInfo->fSZ ) / 2;
	float fX = pInfo->fSX + fCX;
	float fZ = pInfo->fSZ + fCZ;

	D3DXVECTOR2 Pos = D3DXVECTOR2( fX, fZ );
	CameraObjHandle CamHandle = CFcBaseObject::GetCameraObjectHandle( 0 );
	CamHandle->SetMarkPoint( &Pos );
	return ACTION_STATE_FINISH;
}



int CFcActCamTargetTroop::DoCommand()
{
	
	char* pTroopName = GetParamStr( 0 );
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );

	CameraObjHandle CamHandle = CFcBaseObject::GetCameraObjectHandle( 0 );
	CamHandle->SetUserTarget(hTroop);

	return ACTION_STATE_FINISH;
}


void CFcActDelay::Initialize()
{
	m_BeginTime = -1;
	m_nCount = 0;
}

int CFcActDelay::DoCommand()
{
	if(m_BeginTime == -1)
		m_BeginTime = GetProcessTick();

	if(GetProcessTick() - m_BeginTime >= FRAME_PER_SEC)
	{
		m_nCount++;
		m_BeginTime = GetProcessTick();
	}

	if(m_nCount < GetParamInt(0))
		return ACTION_STATE_WORKING;
	
	return ACTION_STATE_FINISH;
}



int CFcActDemoDelay::DoCommand()
{
	BsAssert( 0 );
	return ACTION_STATE_FINISH;
}


int CFcActEffectFadeIn::DoCommand()
{
	if(m_bStart == false){
		int nSec = GetParamInt(0);
		g_InterfaceManager.GetInstance().SetFadeEffect( true , GetParamInt(0) );
		m_bStart = true;
	}
	else{
		if(g_InterfaceManager.GetInstance().IsFadeInStop()){
			return ACTION_STATE_FINISH;
		}
	}
	return ACTION_STATE_WORKING;

}



int CFcActEffectFadeInColor::DoCommand()
{
	if(m_bStart == false){
		int nSec = GetParamInt(0);
		D3DXVECTOR3 Color((float)GetParamInt(1) / 255.f,(float)GetParamInt(2) / 255.f,(float)GetParamInt(3) / 255.f);
		g_InterfaceManager.GetInstance().SetFadeEffect( true , nSec , &Color);
		m_bStart = true;
	}
	else{
		if(g_InterfaceManager.GetInstance().IsFadeInStop()){
			return ACTION_STATE_FINISH;
		}
	}
	return ACTION_STATE_WORKING;
}




int CFcActEffectFadeOut::DoCommand()
{
	if(m_bStart == false){
		int nSec = GetParamInt(0);
		g_InterfaceManager.GetInstance().SetFadeEffect( false , nSec );
		m_bStart = true;
	}
	else{
		if(g_InterfaceManager.GetInstance().IsFadeOutStop()){
			return ACTION_STATE_FINISH;
		}
	}
	return ACTION_STATE_WORKING;
}




int CFcActEffectFadeOutColor::DoCommand()
{
	if(m_bStart == false){
		int nSec = GetParamInt( 0 );
		D3DXVECTOR3 Color((float)GetParamInt(1) / 255.f,(float)GetParamInt(2) / 255.f,(float)GetParamInt(3) / 255.f);
		g_InterfaceManager.GetInstance().SetFadeEffect( false , nSec , &Color);
		m_bStart = true;
	}
	else{
		if(g_InterfaceManager.GetInstance().IsFadeOutStop()){
			return ACTION_STATE_FINISH;
		}
	}
	return ACTION_STATE_WORKING;

}



int CFcActEffectFog::DoCommand()
{
	int nFogID = GetParamInt( 0 );
	int nSec = GetParamInt( 1 ) * FRAME_PER_SEC;
	g_FcWorld.ChangeFogSet( nFogID, nSec );
	return ACTION_STATE_FINISH;
}



int CFcActEffectFXRandomSet::DoCommand()
{    
    int nFxID		= GetParamInt( 0 );
    char* pAreaName = GetParamStr( 1 );
    int nPlayNum    = GetParamInt( 2 );
    int nDir		= GetParamInt( 3 );

    AREA_INFO *pAreaInfo = g_FcWorld.GetAreaInfo( pAreaName );
    for(int i = 0;i < nPlayNum;i++)
    {
        CCrossVector  Cross;
        Cross.m_ZVector = D3DXVECTOR3(0.0f,0.0f,-1.0f);
        Cross.UpdateVectors();
        Cross.RotateYaw((1023 / 12) * nDir);

        Cross.m_PosVector.x = pAreaInfo->fSX + (float)Random((int)(pAreaInfo->fEX - pAreaInfo->fSX));
        Cross.m_PosVector.z = pAreaInfo->fSZ + (float)Random((int)(pAreaInfo->fEZ - pAreaInfo->fSZ));

        Cross.m_PosVector.y = g_BsKernel.GetLandHeight( Cross.m_PosVector.x, Cross.m_PosVector.z );
        CFcWorld::GetInstance().PlaySimpleFx(nFxID,&Cross);
    }
	return ACTION_STATE_FINISH;
}



int CFcActEffectFXSet::DoCommand()
{
	CCrossVector  Cross;
	int nFxID		= GetParamInt(0);
	char* pAreaName = GetParamStr(1);
	int nDir		= GetParamInt(2);
	
	Cross.m_ZVector = D3DXVECTOR3(0.0f,0.0f,-1.0f);
	Cross.UpdateVectors();

	AREA_INFO *pAreaInfo = g_FcWorld.GetAreaInfo( pAreaName );
	Cross.m_PosVector.x = (pAreaInfo->fSX + pAreaInfo->fEX) * 0.5f;
	Cross.m_PosVector.z = (pAreaInfo->fSZ + pAreaInfo->fEZ) * 0.5f;
	Cross.m_PosVector.y = g_BsKernel.GetLandHeight( Cross.m_PosVector.x, Cross.m_PosVector.z );	
	Cross.RotateYaw((1023 / 12) * nDir);
    CFcWorld::GetInstance().PlaySimpleFx(nFxID,&Cross);	
	return ACTION_STATE_FINISH;
}



int CFcActEffectGlowOff::DoCommand()
{
	BsAssert( 0 );
	return ACTION_STATE_FINISH;
}



int CFcActEffectGlowOn::DoCommand()
{
	BsAssert( 0 );
	return ACTION_STATE_FINISH;
}



int CFcActEffectLensflareOff::DoCommand()
{
	BsAssert( 0 );
	return ACTION_STATE_FINISH;
}



int CFcActEffectLensflareOn::DoCommand()
{
	BsAssert( 0 );
	return ACTION_STATE_FINISH;
}



int CFcActEffectLight::DoCommand()
{
	BsAssert( 0 );
	return ACTION_STATE_FINISH;
}



int CFcActEffectMotionblurOff::DoCommand()
{
	BsAssert( 0 );
	return ACTION_STATE_FINISH;
}



int CFcActEffectMotionblurOn::DoCommand()
{
	BsAssert( 0 );
	return ACTION_STATE_FINISH;
}



int CFcActEffectRainOff::DoCommand()
{
	BsAssert( 0 );
	return ACTION_STATE_FINISH;
}



int CFcActEffectRainOn::DoCommand()
{
	BsAssert( 0 );
	return ACTION_STATE_FINISH;
}



int CFcActEffectSetFire::DoCommand()
{
	BsAssert( 0 );
	return ACTION_STATE_FINISH;
}



int CFcActEffectSnowOff::DoCommand()
{
	BsAssert( 0 );
	return ACTION_STATE_FINISH;
}



int CFcActEffectSnowOn::DoCommand()
{
	BsAssert( 0 );
	return ACTION_STATE_FINISH;
}



int CFcActGateClose::DoCommand()
{
	char* pPropName = GetParamStr(0);

	CFcProp* pProp = g_FcWorld.GetProp( pPropName );
	BsAssert( pProp && pProp->IsGateProp() ); //aleksger: prefix bug 820:GetProp() may return NULL.
	CFcGateProp* pGate = (CFcGateProp *)pProp;
	BsAssert( pGate && "Can't found gate" );
	pGate->CmdClose();
	return ACTION_STATE_FINISH;
}



int CFcActGateOpen::DoCommand()
{
	char* pPropName = GetParamStr(0);

	// 다운캐스팅 체크하는 부분 없다!!
	CFcProp* pProp = g_FcWorld.GetProp( pPropName );
	if( !pProp->IsGateProp() )
	{
		DebugString( "Wrong gate prop! %s\n", pPropName );
		BsAssert( 0 && "Wrong gate prop" );
	}
	CFcGateProp* pGate = (CFcGateProp *)g_FcWorld.GetProp( pPropName );
	BsAssert(pGate && "Unable to get property"); //aleksger: prefix bug 821: GetProp may reutrn NULL.

	pGate->CmdOpen();
	return ACTION_STATE_FINISH;
}



int CFcActHookInputOff::DoCommand()
{
	CInputPad::GetInstance().HookOn(false);
	return ACTION_STATE_FINISH;
}



int CFcActHookInputOn::DoCommand()
{
	g_FcWorld.GetHeroHandle()->ChangeAnimation( ANI_TYPE_STAND, 0, g_FcWorld.GetHeroHandle()->GetCurAniAttr(), false, 1, true );
	CInputPad::GetInstance().HookOn(true);
	g_FcWorld.GetHeroHandle()->UnlinkCatchObject( 0, -1, 0, true );
	return ACTION_STATE_FINISH;
}



int CFcActInterfaceOff::DoCommand()
{
	g_InterfaceManager.GetInstance().ForceShowInterface(false);
	return ACTION_STATE_FINISH;
}



int CFcActInterfaceOn::DoCommand()
{
	g_InterfaceManager.GetInstance().ForceShowInterface(true);
	return ACTION_STATE_FINISH;
}



int CFcActLetterBoxOff::DoCommand()
{
	g_InterfaceManager.GetInstance().ShowLetterBox(false);
	g_FcWorld.SetPlayerEnable( true );

	HeroObjHandle hHero = g_FcWorld.GetHeroHandle();
	if( hHero && hHero->GetTroop() )
	{
		hHero->GetTroop()->CmdInvulnerable( false );
	}

	CameraObjHandle CamHandle;
	CamHandle = CFcBaseObject::GetCameraObjectHandle( 0 );
	CamHandle->SeqStop();
	CamHandle->SetUserTarget(g_FcWorld.GetHeroHandle()->GetTroop());
	g_FcWorld.ResetUnitAILOD();

	g_FcWorld.SetDemoType( -1 );

	CInputPad::GetInstance().Break(false);		
	// TGS 임시 by Siva		캐릭터 못 움직이는 경우 땜시 처리
	if( g_FcWorld.GetHeroHandle() ) {
		*g_FcWorld.GetHeroHandle()->GetMoveTargetPos() = D3DXVECTOR3( 0.f, 0.f, 0.f );
	}

	if(g_InterfaceManager.GetInstance().GetLetterBoxAlpha() < 0.1f)
	{
		g_InterfaceManager.GetInstance().ForceShowInterface(true);

		return ACTION_STATE_FINISH;
	}
	return ACTION_STATE_WORKING;
}



int CFcActLetterBoxOn::DoCommand()
{
	g_FcWorld.SetPlayerEnable( false );
	g_InterfaceManager.GetInstance().ShowLetterBox(true);
	CInputPad::GetInstance().Break(true);

	HeroObjHandle hHero = g_FcWorld.GetHeroHandle();
	hHero->ChangeAnimation( ANI_TYPE_STAND, 0, hHero->GetCurAniAttr(), false, 1, true );
	hHero->UnlinkCatchObject( 0, -1, 0, true );
	if( hHero && hHero->GetTroop() )
	{
		hHero->GetTroop()->CmdInvulnerable( true );
	}

	g_InterfaceManager.GetInstance().ForceShowInterface(false);
	g_FcWorld.SetDemoType( 1 );		// 1은 컷신타입
	g_FcWorld.SetDemoSkip( false );

	CameraObjHandle CamHandle;
	CamHandle = CFcBaseObject::GetCameraObjectHandle( 0 );

	/*if(g_InterfaceManager.GetInstance().GetLetterBoxAlpha() < 0.95f)
	{
		return ACTION_STATE_WORKING;
	}*/
	return ACTION_STATE_FINISH;
	
}



int CFcActMapOff::DoCommand()
{
	g_InterfaceManager.GetInstance().MinimapShow(false);
	return ACTION_STATE_FINISH;
}



int CFcActMapOn::DoCommand()
{
	g_InterfaceManager.GetInstance().MinimapShow(true);
	return ACTION_STATE_FINISH;
}



int CFcActMapScale::DoCommand()
{
	g_InterfaceManager.GetInstance().MinimapZoomChange(GetParamFloat(0));
	return ACTION_STATE_FINISH;
}


void CFcActMessageReport::Initialize()
{
	m_bPlayed = false;
}

int CFcActMessageReport::DoCommand()
{
	int nTextID = GetParamInt( 0 );	
	int nTick = GetParamInt( 1 );	
	g_InterfaceManager.GetInstance().SetReport( nTextID );
	return ACTION_STATE_FINISH;
}


void CFcActMessageSay::Initialize()
{
	m_bPlayed = false;
}

int CFcActMessageSay::DoCommand()
{
	int nPortraitID;
	int nTextID;
	GetParamInt( 0, nPortraitID );	
	GetParamInt( 1, nTextID );	
	if(g_InterfaceManager.GetInstance().IsSpeechPlay() == false)
	{
		if(m_bPlayed == false)
		{
			g_InterfaceManager.GetInstance().SetSpeech(nPortraitID, nTextID );
			m_bPlayed = true;
			return ACTION_STATE_WORKING;
		}
		else
		{
			return ACTION_STATE_FINISH;
		}
	}
	return ACTION_STATE_WORKING;
}



void CFcActMessageShow::Initialize()
{
	m_bShow = false;
}

int CFcActMessageShow::DoCommand()
{
	if(m_bShow == false){
		int nTextID = GetParamInt(0);
		g_InterfaceManager.GetInstance().ShowTextInLetterBox( nTextID );
		m_bShow = true;
	}
	else
	{
		if(g_InterfaceManager.GetInstance().IsShowTextInLetterBox())
			return ACTION_STATE_FINISH;
	}
	return ACTION_STATE_WORKING;
}


void CFcActMessageShowXY::Initialize()
{
	m_bShow = false;
}

int CFcActMessageShowXY::DoCommand()
{
	if(m_bShow == false){
		int nX      = GetParamInt(0);
		int nY	    = GetParamInt(1);
		int nSec    = GetParamInt(2);
		int nTextID = GetParamInt(3);
		g_InterfaceManager.GetInstance().SetMsgShowXY( nX, nY, nSec, nTextID);
		m_bShow = true;
	}
	else
	{
		if(g_InterfaceManager.GetInstance().IsMsgShowXYEnd())
			return ACTION_STATE_FINISH;
	}	
	return ACTION_STATE_WORKING;
}



int CFcActMissionCompleted::DoCommand()
{
	g_FcWorld.SetMissionComplete( MISSION_FINISH_TYPE_SUCCESS );
	g_FcWorld.StopTrigger();
	return ACTION_STATE_FINISH;
}



int CFcActMissionFailed::DoCommand()
{
	g_FcWorld.SetMissionComplete( MISSION_FINISH_TYPE_FAIL );

	if( _strcmpi( g_FCGameData.cLastFailMissionName, g_FCGameData.cMapFileName ) == 0 )
	{
		g_FCGameData.nNumMissionFail++;
	}
	else
	{
		strcpy( g_FCGameData.cLastFailMissionName, g_FCGameData.cMapFileName );
		g_FCGameData.nNumMissionFail = 1;
	}

	g_FcWorld.StopTrigger();
	return ACTION_STATE_FINISH;
}



int CFcActOrbAdd::DoCommand()
{
	int nOrb = GetParamInt( 0 );
	CFcWorld::GetInstance().GetHeroHandle()->AddOrbSpark( nOrb );
	return ACTION_STATE_FINISH;
}



int CFcActPointAreaOff::DoCommand()
{
	char *pAreaName = GetParamStr( 0 );
	g_InterfaceManager.GetInstance().MinimapAreaOn(false, pAreaName);
	return ACTION_STATE_FINISH;
}



int CFcActPointAreaOn::DoCommand()
{
	float fMapWidth,fMapHeight;
	CFcWorld::GetInstance().GetMapSize( fMapWidth , fMapHeight );

	char *pAreaName = GetParamStr( 0 );
	AREA_INFO *pAreaInfo = g_FcWorld.GetAreaInfo( pAreaName );

	g_InterfaceManager.GetInstance().MinimapAreaOn(true, pAreaName,
		pAreaInfo->fSX, pAreaInfo->fSZ,
		pAreaInfo->fEX, pAreaInfo->fEZ,
		GetParamInt(1) * FRAME_PER_SEC);

	return ACTION_STATE_FINISH;
}



int CFcActPointFriendInArea::DoCommand()
{
    float fMapWidth,fMapHeight;
    char *pAreaName = GetParamStr( 0 );
    int nTick = GetParamInt(1) * FRAME_PER_SEC;

    CFcWorld::GetInstance().GetMapSize(fMapWidth,fMapHeight);
    AREA_INFO *pAreaInfo = g_FcWorld.GetAreaInfo( pAreaName );

    for(int i = 0;i < g_FcWorld.GetTroopCount();i++)
    {
		TroopObjHandle hTroop = g_FcWorld.GetTroopObject(i);
		if(hTroop->GetTeam() != 0) continue; //현재 그룹이 0이 아니면 다 다른편
        if( hTroop->IsEnable() == false ) continue;
		if( hTroop->IsEliminated() == true ) continue;
        if(pAreaInfo->fSX > hTroop->GetPos().x) continue;
        if(pAreaInfo->fEX < hTroop->GetPos().x) continue;
        if(pAreaInfo->fSZ > hTroop->GetPos().z) continue;
        if(pAreaInfo->fEZ < hTroop->GetPos().z) continue;

        g_InterfaceManager.GetInstance().MinimapPointOn(true, hTroop->GetName(), hTroop, nTick);
    }
	return ACTION_STATE_FINISH;
}



int CFcActPointHostileInArea::DoCommand()
{
    float fMapWidth,fMapHeight;
    char *pAreaName = GetParamStr( 0 );
    int nTick = GetParamInt(1) * FRAME_PER_SEC;

    CFcWorld::GetInstance().GetMapSize(fMapWidth,fMapHeight);
    AREA_INFO *pAreaInfo = g_FcWorld.GetAreaInfo( pAreaName );

    for(int i = 0;i < g_FcWorld.GetTroopCount();i++)
    {
		TroopObjHandle hTroop = g_FcWorld.GetTroopObject(i);
		if( hTroop->GetTeam() == 0 ) continue; //현재 그룹이 0이 아니면 다 다른편
		if( hTroop->IsEnable() == false ) continue;
		if( hTroop->IsEliminated() == true ) continue;
        if( pAreaInfo->fSX > hTroop->GetPos().x ) continue;
        if( pAreaInfo->fEX < hTroop->GetPos().x ) continue;
        if( pAreaInfo->fSZ > hTroop->GetPos().z ) continue;
        if( pAreaInfo->fEZ < hTroop->GetPos().z ) continue;

        g_InterfaceManager.GetInstance().MinimapPointOn(true, hTroop->GetName(), hTroop, nTick);
    }
	return ACTION_STATE_FINISH;
}



int CFcActPointTroopOFf::DoCommand()
{
    char cTroopName[32];
	GetParamStr(0, cTroopName, _countof(cTroopName));
    TroopObjHandle hTroop = g_FcWorld.GetTroopObject( cTroopName );
    g_InterfaceManager.GetInstance().MinimapPointOn( false, cTroopName );

	return ACTION_STATE_FINISH;
}

int CFcActPointTroopOn::DoCommand()
{
    char *cTroopName = GetParamStr(0);
    int   nTime      = GetParamInt(1);

    TroopObjHandle hTroop = g_FcWorld.GetTroopObject( cTroopName );
    g_InterfaceManager.GetInstance().MinimapPointOn( true, cTroopName, hTroop, nTime * FRAME_PER_SEC);
	return ACTION_STATE_FINISH;
}



int CFcActSoundSet::DoCommand()
{
    //임시....
	//g_pSoundManager->PlaySound(SB_UNIT,0);
    //g_pSoundManager->PlayBGM();
	//g_pSoundManager->Play3DSound(SB_UNIT,0,g_FcWorld.GetHeroHandle()->GetPos());
	return ACTION_STATE_FINISH;
}



int CFcActSoundUnset::DoCommand()
{
	BsAssert( 0 );
	return ACTION_STATE_FINISH;
}



int CFcActTimeMark::DoCommand()
{
	int nTimeMarkID;
	GetParamInt( 0, nTimeMarkID );	
	g_FcWorld.SetTimeMark( nTimeMarkID );
	return ACTION_STATE_FINISH;
}



int CFcActTriggerActivate::DoCommand()
{
	g_FcWorld.SetTriggerEnable(GetParamInt(0),true);
	return ACTION_STATE_FINISH;
}



int CFcActTriggerDeactivate::DoCommand()
{
	g_FcWorld.SetTriggerEnable(GetParamInt(0),false);
	return ACTION_STATE_FINISH;
}



int CFcActTriggerLoop::DoCommand()
{
	g_FcWorld.GetTriggerCommander()->SetLoopCurTrg(true);
	return ACTION_STATE_WORKING;
}



int CFcActTriggerReset::DoCommand()
{
	int nID = GetParamInt(0);
	DebugString( "TriggerReset %d\n", nID );
	g_FcWorld.GetTriggerCommander()->ResetTrigger( nID );
	return ACTION_STATE_FINISH;
}



int CFcActTriggerResetAll::DoCommand()
{
	g_FcWorld.GetTriggerCommander()->ResetAll();
	return ACTION_STATE_FINISH;
}



int CFcActTriggerResetThis::DoCommand()
{
	return ACTION_STATE_RESET;
}



int CFcActTroopAnimation::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	int nAni = GetParamInt( 1 );
	int nIndex = GetParamInt( 2 );
	int nAttr = GetParamInt( 3 );
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );

	// TODO: 동작 시간 약간 랜덤하게 줘야 한다.

	int nCnt = hTroop->GetUnitCount();
	for( int i=0; i<nCnt; i++ )
	{
		GameObjHandle hUnit = hTroop->GetUnit( i );

		if( hUnit == NULL )
			continue;
        
		CFcGameObject::GameObj_ClassID ClassID = hUnit->GetClassID();
		if( ClassID == CFcGameObject::Class_ID_Horse ||
			ClassID == CFcGameObject::Class_ID_Catapult ||
			ClassID == CFcGameObject::Class_ID_MoveTower ||
			ClassID == CFcGameObject::Class_ID_Fly )
			continue;

		if( hUnit->IsDie() )
			continue;

		hUnit->ChangeAnimation( nAni, nIndex, nAttr );

		if( hUnit == g_FcWorld.GetHeroHandle() )
			g_FcWorld.GetHeroHandle()->UnlinkCatchObject( 0, -1, 0, true );
	}
	return ACTION_STATE_FINISH;
}



int CFcActTroopAnnihilate::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	hTroop->CmdAnnihilate();
	return ACTION_STATE_FINISH;
}



int CFcActTroopAttack::DoCommand()
{
	char cTroopName[32], cTargetTroopName[32];
	GetParamStr(0, cTroopName, _countof(cTroopName));
	GetParamStr(1, cTargetTroopName, _countof(cTargetTroopName));
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( cTroopName );
	TroopObjHandle hTargetTroop = g_FcWorld.GetTroopObject( cTargetTroopName );

	hTroop->SetTriggerCmd();
	hTroop->CmdAttack( hTargetTroop );
	hTroop->ResetTriggerCmd();
	return ACTION_STATE_FINISH;
}



int CFcActTroopAttackLeader::DoCommand()
{
	BsAssert( 0 );
	return ACTION_STATE_FINISH;
}



int CFcActTroopDisable::DoCommand()
{
	char cTroopName[32];
	GetParamStr(0, cTroopName, _countof(cTroopName));
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( cTroopName );
	hTroop->CmdEnable( false );
	return ACTION_STATE_FINISH;
}



int CFcActTroopDisableAll::DoCommand()
{
	int nCnt = g_FcWorld.GetTroopmanager()->GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTroop = g_FcWorld.GetTroopmanager()->GetTroop( i );
		if( hTroop->GetType() == TROOPTYPE_PLAYER_1 ||
			hTroop->GetType() == TROOPTYPE_PLAYER_2 )
			continue;

		hTroop->CmdEnable( false );
	}
	return ACTION_STATE_FINISH;
}



int CFcActTroopDisableInArea::DoCommand()
{
	char *pAreaName = GetParamStr( 0 );
	AREA_INFO* pInfo = g_FcWorld.GetAreaInfo( pAreaName );
	g_FcWorld.SetDisableEnemyTroopInArea( pInfo->fSX, pInfo->fSZ, pInfo->fEX, pInfo->fEZ );
	g_FcWorld.SetDisableFriendTroopInArea( pInfo->fSX, pInfo->fSZ, pInfo->fEX, pInfo->fEZ );
	return ACTION_STATE_FINISH;
}



int CFcActTroopEnable::DoCommand()
{
	char cTroopName[32];
	GetParamStr(0, cTroopName, _countof(cTroopName));
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( cTroopName );
	hTroop->CmdEnable( true );
	return ACTION_STATE_FINISH;
}



int CFcActEnableInArea::DoCommand()
{
	if( m_bFirst )
	{
		char *pAreaName = GetParamStr( 0 );
		AREA_INFO* pInfo = g_FcWorld.GetAreaInfo( pAreaName );
		m_nID = g_FcWorld.GetTroopmanager()->SetEnableTroopInArea( pInfo->fSX, pInfo->fSZ, pInfo->fEX, pInfo->fEZ );
		m_bFirst = false;
	}
	else
	{
		if( g_FcWorld.GetTroopmanager()->IsFinishEnableTroops( m_nID ) )
		{
			return ACTION_STATE_FINISH;
		}
	}
	return ACTION_STATE_WORKING;
}



int CFcActEnemyDisableInArea::DoCommand()
{
	char *pAreaName = GetParamStr( 0 );
	AREA_INFO* pInfo = g_FcWorld.GetAreaInfo( pAreaName );
	g_FcWorld.SetDisableEnemyTroopInArea( pInfo->fSX, pInfo->fSZ, pInfo->fEX, pInfo->fEZ );
	return ACTION_STATE_FINISH;
}



int CFcActEnemyEnableInArea::DoCommand()
{
	if( m_bFirst )
	{
		char *pAreaName = GetParamStr( 0 );
		AREA_INFO* pInfo = g_FcWorld.GetAreaInfo( pAreaName );
		m_nID = g_FcWorld.GetTroopmanager()->SetEnableEnemyTroopInArea( pInfo->fSX, pInfo->fSZ, pInfo->fEX, pInfo->fEZ );
		m_bFirst = false;
	}
	else
	{
		if( g_FcWorld.GetTroopmanager()->IsFinishEnableTroops( m_nID ) )
		{
			return ACTION_STATE_FINISH;
		}
	}
	return ACTION_STATE_WORKING;
}



int CFcActTroopFllow::DoCommand()
{
	char cTroopName[32], cTargetTroopName[32];
	float fDist;
	GetParamStr(0, cTroopName, _countof(cTroopName));
	GetParamStr(1, cTargetTroopName, _countof(cTargetTroopName));
	GetParamFloat(2, fDist );

	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( cTroopName );
	TroopObjHandle hTargetTroop = g_FcWorld.GetTroopObject( cTargetTroopName );

	hTroop->SetTriggerCmd();
	hTroop->CmdFollow( hTargetTroop, fDist );
	hTroop->ResetTriggerCmd();
	return ACTION_STATE_FINISH;
}



int CFcActTroopFriendDisableInArea::DoCommand()
{
	char *pAreaName = GetParamStr( 0 );
	AREA_INFO* pInfo = g_FcWorld.GetAreaInfo( pAreaName );
	g_FcWorld.SetDisableFriendTroopInArea( pInfo->fSX, pInfo->fSZ, pInfo->fEX, pInfo->fEZ );
	return ACTION_STATE_FINISH;
}



int CFcActTroopFriendEnableInArea::DoCommand()
{
	if( m_bFirst )
	{
		char *pAreaName = GetParamStr( 0 );
		AREA_INFO* pInfo = g_FcWorld.GetAreaInfo( pAreaName );
		m_nID = g_FcWorld.GetTroopmanager()->SetEnableFriendTroopInArea( pInfo->fSX, pInfo->fSZ, pInfo->fEX, pInfo->fEZ );
		m_bFirst = false;
	}
	else
	{
		if( g_FcWorld.GetTroopmanager()->IsFinishEnableTroops( m_nID ) )
		{
			return ACTION_STATE_FINISH;
		}
	}
	return ACTION_STATE_WORKING;
}



int CFcActTroopHPFill::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	int nPercent = GetParamInt( 1 );

	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	hTroop->AddHPPercent( nPercent );
	return  ACTION_STATE_FINISH;
}



int CFcActTroopHPMAX::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );

	BsAssert( hTroop );
	if( hTroop->IsEliminated() == true )
		return ACTION_STATE_FINISH;

	
	int nCnt = hTroop->GetUnitCount();
	for( int i=0; i<nCnt; i++ )
	{
		GameObjHandle hUnit = hTroop->GetUnit( i );
		if( hUnit == NULL )
			continue;

		if( hUnit->IsDie() || hUnit->GetHP() <= 0 ) continue;
		hUnit->SetHP( hUnit->GetMaxHP() );
	}

	return ACTION_STATE_FINISH;
}



int CFcActTroopInvulnerable::DoCommand()
{
	char cTroopName[32];
	GetParamStr(0, cTroopName, _countof(cTroopName));

	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( cTroopName );
	hTroop->CmdInvulnerable( true );
	return ACTION_STATE_FINISH;
}



int CFcActTroopLeaderInvulnerable::DoCommand()
{
	char cTroopName[32];
	GetParamStr(0, cTroopName, _countof(cTroopName));

	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( cTroopName );
	hTroop->CmdLeaderInvulnerable( true );
	return ACTION_STATE_FINISH;
}



int CFcActTroopLeaderVulnerable::DoCommand()
{
	char cTroopName[32];
	GetParamStr(0, cTroopName, _countof(cTroopName));

	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( cTroopName );
	hTroop->CmdLeaderInvulnerable( false );
	return ACTION_STATE_FINISH;
}



int CFcActTroopPlace::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	char *pAreaName = GetParamStr( 1 );
	int nDir = GetParamInt( 2 );

	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	AREA_INFO* pInfo = g_FcWorld.GetAreaInfo( pAreaName );

	float fCX = ( pInfo->fEX - pInfo->fSX ) / 2;
	float fCZ = ( pInfo->fEZ - pInfo->fSZ ) / 2;
	float fX = pInfo->fSX + fCX;
	float fZ = pInfo->fSZ + fCZ;
		
	hTroop->SetDirByClock(nDir, 12);
	hTroop->CmdPlace( fX, fZ, nDir );
	

	DebugString( "CmdPlace!!!! %s, %s, %d", pTroopName, pAreaName, nDir );

	return ACTION_STATE_FINISH;
}



int CFcActTroopRangeAttackArea::DoCommand()
{
	BsAssert( 0 );
	return ACTION_STATE_FINISH;
}



int CFcActTroopRangeAttackWall::DoCommand()
{
	BsAssert( 0 );
	return ACTION_STATE_FINISH;
}



int CFcActTroopRenew::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	char* pAreaName = GetParamStr( 1 );
	int nDir = GetParamInt( 2 );

	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	AREA_INFO* pInfo = g_FcWorld.GetAreaInfo( pAreaName );

	int nDX = (int)(pInfo->fEX - pInfo->fSX);
	int nDZ = (int)(pInfo->fEZ - pInfo->fSZ);

	float fDX = (float)(Random(nDX));
	float fDZ = (float)(Random(nDZ));

	float fX = pInfo->fSX + fDX;
	float fZ = pInfo->fSZ + fDZ;

	hTroop->CmdRenew( fX, fZ, nDir );		// 방향 추가해야 한다.
	return ACTION_STATE_FINISH;
}



int CFcActTroopRenewOutOfSight::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	char* pAreaName = GetParamStr( 1 );
	int nDir = GetParamInt( 2 );

	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	AREA_INFO* pInfo = g_FcWorld.GetAreaInfo( pAreaName );

	int nDX = (int)(pInfo->fEX - pInfo->fSX);
	int nDZ = (int)(pInfo->fEZ - pInfo->fSZ);

	float fDX = (float)Random(nDX);
	float fDZ = (float)Random(nDZ);

	float fX = pInfo->fSX + fDX;
	float fZ = pInfo->fSZ + fDZ;

// 보이지 않는 거리 체크 여기서 해야 한다.
//	BsAssert(0);

	hTroop->CmdRenew( fX, fZ, nDir );
	return ACTION_STATE_FINISH;
}



int CFcActTroopRun::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	char *pAreaName = GetParamStr( 1 );

	AREA_INFO* pInfo = g_FcWorld.GetAreaInfo( pAreaName );
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );

	float fCX = ( pInfo->fEX - pInfo->fSX ) / 2;
	float fCZ = ( pInfo->fEZ - pInfo->fSZ ) / 2;

	float fX = pInfo->fSX + fCX;
	float fZ = pInfo->fSZ + fCZ;

	hTroop->SetTriggerCmd();
	hTroop->CmdRun( fX, fZ );
	hTroop->ResetTriggerCmd();
	return ACTION_STATE_FINISH;
}



int CFcActTroopStop::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );

	hTroop->SetTriggerCmd();
	hTroop->CmdStop();
	hTroop->ResetTriggerCmd();
	return ACTION_STATE_FINISH;
}



int CFcActTroopVulnerable::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	hTroop->CmdInvulnerable( false );
	return ACTION_STATE_FINISH;
}



int CFcActTroopWalk::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	char* pAreaName = GetParamStr( 1 );

	AREA_INFO* pInfo = g_FcWorld.GetAreaInfo( pAreaName );
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );

	float fCX = ( pInfo->fEX - pInfo->fSX ) / 2;
	float fCZ = ( pInfo->fEZ - pInfo->fSZ ) / 2;

	float fX = pInfo->fSX + fCX;
	float fZ = pInfo->fSZ + fCZ;

	hTroop->SetTriggerCmd();
	hTroop->CmdWalk( fX, fZ );
	hTroop->ResetTriggerCmd();
	return ACTION_STATE_FINISH;
}



int CFcActVarIncrease::DoCommand()
{
	int nVarID = GetParamInt( 0 );
	int nValue = GetParamInt( 1 );

	int nCurValue = g_FcWorld.GetTriggerVarInt( nVarID );
	nCurValue += nValue;
	g_FcWorld.SetTriggerVar( nVarID, nCurValue );

	return ACTION_STATE_FINISH;
}



int CFcActVarSet::DoCommand()
{
	int nVarID = GetParamInt( 0 );
	int nValue = GetParamInt( 1 );
	g_FcWorld.SetTriggerVar( nVarID, nValue );
	return ACTION_STATE_FINISH;
}



int CFcActWallCollapse::DoCommand()
{
	BsAssert( 0 );
	return ACTION_STATE_FINISH;
}


int CFcActWallSetHP::DoCommand()
{
	char* pPropName = GetParamStr( 0 );
	int nPer = GetParamInt( 1 );

	CFcProp *pProp = g_FcWorld.GetProp( pPropName );
	if( !pProp->IsCrushProp() )
	{
		DebugString( "ActWallSetHP Error! : Prop is not breakable type\n" );
	}
	CFcBreakableProp* pBreakableProp = (CFcBreakableProp*)pProp;
	int nMaxHP = pBreakableProp->GetMaxHP();

	int nHP = nMaxHP * nPer / 100;
	if( nHP < 0 )
		nHP = 0;
	if( nHP > nMaxHP )
		nHP = nMaxHP;

	pBreakableProp->SetHP( nHP );
	return ACTION_STATE_FINISH;
}


int CFcPropPush::DoCommand()
{
	char* pPropName = GetParamStr( 0 );
	char* pAreaName = GetParamStr( 1 );
	float fForce = (float)GetParamInt( 2 );

	AREA_INFO *pAreaInfo = g_FcWorld.GetAreaInfo( pAreaName );
	D3DXVECTOR3 AreaPos;
	
	AreaPos.x = (pAreaInfo->fSX + pAreaInfo->fEX) * 0.5f;
	AreaPos.z = (pAreaInfo->fSZ + pAreaInfo->fEZ) * 0.5f;
	AreaPos.y = g_BsKernel.GetLandHeight( AreaPos.x, AreaPos.z );

	
	// 다운캐스팅 체크하는 부분 없다!!
	CFcProp *pProp = g_FcWorld.GetProp( pPropName );
	if( pProp == NULL )
	{
		DebugString( "Prop name is not setting PropPush %s\n", pPropName );
		BsAssert(0&& "Prop name is not setting PropPush"); //aleksger: prefix bug 824: Must get a property and not continue.
		return ACTION_STATE_FINISH;
	}

	D3DXVECTOR2 PropPosV2;

	if( pProp->IsCrushProp() )
	{
		CFcBreakableProp* pBreakable = (CFcBreakableProp*) pProp;
		PropPosV2 = pBreakable->GetPosV2();
				
		D3DXVECTOR3 PropPos, Force, Dir;

		PropPos.x = PropPosV2.x;
		PropPos.z = PropPosV2.y;
		PropPos.y = pBreakable->GetMat()._42;

		Dir = AreaPos - PropPos;
		D3DXVec3Normalize(&Dir, &Dir);

		Force = Dir * fForce;

		pBreakable->AddForce( &Force, NULL, -1, PROP_BREAK_TYPE_EVENT );
	}
	else if( pProp->IsDynamicProp() ) {

		CFcDynamicProp* pDynamic = (CFcDynamicProp*) pProp;
		PropPosV2 = pDynamic->GetPosV2();
		
		D3DXVECTOR3 PropPos, Force, Dir;

		PropPos.x = PropPosV2.x;
		PropPos.z = PropPosV2.y;
		PropPos.y = pDynamic->GetMat()._42;

		Dir = AreaPos - PropPos;
		D3DXVec3Normalize(&Dir, &Dir);

		Force = Dir * fForce;

		pDynamic->AddForce( &Force, NULL, -1, PROP_BREAK_TYPE_EVENT );
	}

	return ACTION_STATE_FINISH;
}


int CFcDestroyBridge::DoCommand()
{
	char* pPropName = GetParamStr( 0 );
	// 다운캐스팅 체크하는 부분 없다!!
	CFcProp* pProp = g_FcWorld.GetProp( pPropName );
	if( pProp == NULL )
		return ACTION_STATE_FINISH;

	if( pProp->IsCrushProp() == false )
	{
		BsAssert( 0 );
		DebugString( "부서지는 프랍이 아닌데 부서지라는 명령을 내렸다!! %s\n", pPropName );
		return ACTION_STATE_FINISH;
	}
	CFcBreakableProp * pBreakable = (CFcBreakableProp *)pProp;
	pBreakable->Destroy();return ACTION_STATE_FINISH;
}



int CFcSetGuardianTroop::DoCommand()
{
	// 사용안함
	BsAssert(0);
	return ACTION_STATE_FINISH;

	char* pTroopName = GetParamStr( 0 );
	int nPlayer = GetParamInt( 1 );
	int nAttackAIID = GetParamInt( 2 );
	int nGuardAIID = GetParamInt( 3 );

//	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
//	g_FcWorld.SetGuardianTroop( nPlayer, hTroop, nAttackAIID, nGuardAIID );
	return ACTION_STATE_FINISH;
}


int CFcActSetEnemyAttr::DoCommand()
{
	char* pAreaName = GetParamStr( 0 );
	AREA_INFO* pInfo = g_FcWorld.GetAreaInfo( pAreaName );
	g_FcWorld.SetEnemyAttr( pInfo, true );
	return ACTION_STATE_FINISH;
}

int CFcActResetEnemyAttr::DoCommand()
{
	char* pAreaName = GetParamStr( 0 );
	AREA_INFO* pInfo = g_FcWorld.GetAreaInfo( pAreaName );
	g_FcWorld.SetEnemyAttr( pInfo, false );
	return ACTION_STATE_FINISH;
}


int CFcActEnableDynamicProp::DoCommand()
{
	char* pAreaName = GetParamStr( 0 );
	AREA_INFO* pInfo = g_FcWorld.GetAreaInfo( pAreaName );
	g_FcWorld.SetEnableDynamicProp( true, pInfo->fSX, pInfo->fSZ, pInfo->fEX, pInfo->fEZ );
	return ACTION_STATE_FINISH;
}

int CFcActDisableDynamicProp::DoCommand()
{
	char* pAreaName = GetParamStr( 0 );
	AREA_INFO* pInfo = g_FcWorld.GetAreaInfo( pAreaName );
	g_FcWorld.SetEnableDynamicProp( false, pInfo->fSX, pInfo->fSZ, pInfo->fEX, pInfo->fEZ );
	return ACTION_STATE_FINISH;
}

void CFcPlayRealtimeMovie::Initialize()	
{
}

int CFcPlayRealtimeMovie::DoCommand()
{
	// char* szTemp[64];
	// int nID = GetParamInt( 0 );
	// sprintf( (char*)szTemp, "%d.bsrtm", nID );

//	if( m_bStart == false )
//	{
		g_pFcRealMovie->PlayDirect( GetParamStr(0) ); // (char*)szTemp );			// test
//		g_pFcRealMovie->Play( (char*)szTemp );			// test
//		m_bStart = true;
		g_FcWorld.SetDemoType( 0 );		// 0은 RMType
//	}
/*
	if( m_bStart )
	{
		if( g_pFcRealMovie->IsPlay() == false && g_pFcRealMovie->IsSetDirectMsg() == false )
		{
			g_FcWorld.SetDemoType( -1 );	// reset
			return ACTION_STATE_FINISH;
		}
	}
*/
	return ACTION_STATE_FINISH;
}


int CFcTroopHPGaugeShow::DoCommand()
{
	int nGroup;
	bool bOnOff = false;
	
	nGroup = GetParamInt(0);
	if(GetParamInt(1)){ bOnOff = true; } 

	//g_InterfaceManager.SetUnitGauge( 1 , bOnOff ); //1 은 UnitSOXID 상수임
	if( nGroup == 0 )
		g_FCGameData.bShowFriendlyGauge = bOnOff;
	else
		g_FCGameData.bShowEnemyGauge = bOnOff;

	return ACTION_STATE_FINISH;
}


//
void GetTroopCustomCondition(std::vector<TroopObjHandle> &TroopList, AREA_INFO*	pInfo ,int nTroopType,int nGroup)
{
	TroopObjHandle hTroop;
	for(int i = 0;i < g_FcWorld.GetTroopmanager()->GetTroopCount();i++){
		hTroop = g_FcWorld.GetTroopmanager()->GetTroop(i);	                                      	
		if(hTroop->GetType() != nTroopType || hTroop->IsEliminated()) { continue; }
		if(nGroup == TROOP_MINE && hTroop->GetTeam() != 0) { continue; }
		if(nGroup == TROOP_ENEMY && hTroop->GetTeam() == 0){ continue; }

		D3DXVECTOR3	Pos = hTroop->GetPos();
		if(Pos.x < pInfo->fSX || Pos.x > pInfo->fEX || Pos.z < pInfo->fSZ || Pos.z > pInfo->fEZ) continue;
		TroopList.push_back(hTroop);
	}
}



int CTroopTypeAIDisableInArea::DoCommand()
{
	char* pAreaName  = GetParamStr( 0 );
	int   nTeam = GetParamInt( 1 );
	int   nTroopType = GetParamInt( 2 );
	AREA_INFO* pInfo = g_FcWorld.GetAreaInfo( pAreaName );

	std::vector<TroopObjHandle>vecTroopList;
	GetTroopCustomCondition(vecTroopList,pInfo,nTroopType,nTeam);
	for(unsigned int i = 0;i < vecTroopList.size();i++){	
		g_FcWorld.EnableTroopAI(vecTroopList[i],false);
	}
	return ACTION_STATE_FINISH;
}


int CTroopTypeAIEnableInArea::DoCommand()
{
	char* pAreaName  = GetParamStr( 0 );
	int   nGroup	 = GetParamInt( 1 );
	int   nTroopType = GetParamInt( 2 );
	AREA_INFO* pInfo = g_FcWorld.GetAreaInfo( pAreaName );

	std::vector<TroopObjHandle>vecTroopList;
	GetTroopCustomCondition(vecTroopList,pInfo,nTroopType,nGroup);
	for(unsigned int i = 0;i < vecTroopList.size();i++){	
		g_FcWorld.EnableTroopAI(vecTroopList[i],true);
	}
	return ACTION_STATE_FINISH;
}

int CTroopTypeAISetInArea::DoCommand()
{
	char* pAreaName  = GetParamStr( 0 );
	int   nGroup	 = GetParamInt( 1 );
	int   nTroopType = GetParamInt( 2 );
	int   nAIID		 = GetParamInt( 3 );

	AREA_INFO*	   pInfo = g_FcWorld.GetAreaInfo( pAreaName );	

	std::vector<TroopObjHandle>vecTroopList;
	GetTroopCustomCondition(vecTroopList,pInfo,nTroopType,nGroup);
	for(unsigned int i = 0;i < vecTroopList.size();i++){	
		g_FcWorld.SetTroopAI(vecTroopList[i],nAIID);
	}
	return ACTION_STATE_FINISH;
}


int CTroopTypeAnimationInArea::DoCommand()
{
	DebugString("구현안된 트리거CTroopTypeAnimationInArea\n");
	return ACTION_STATE_FINISH;
}

int CTroopTypeKillAllInArea::DoCommand()
{
	char* pAreaName  = GetParamStr( 0 );
	int   nGroup		 = GetParamInt( 1 );
	int   nTroopType = GetParamInt( 2 );

	AREA_INFO*	   pInfo = g_FcWorld.GetAreaInfo( pAreaName );	

	std::vector<TroopObjHandle>vecTroopList;
	GetTroopCustomCondition(vecTroopList,pInfo,nTroopType,nGroup);
	for(unsigned int i = 0;i < vecTroopList.size();i++){
		vecTroopList[i]->SetDie();
	}
	return ACTION_STATE_FINISH;
}


int CTroopTypeDIsableInArea::DoCommand()
{
	char*		   pAreaName  = GetParamStr( 0 );
	int			   nGroup	  = GetParamInt( 1 );
	int			   nTroopType = GetParamInt( 2 );
	AREA_INFO*	   pInfo	  = g_FcWorld.GetAreaInfo( pAreaName );

	std::vector<TroopObjHandle>vecTroopList;
	GetTroopCustomCondition(vecTroopList,pInfo,nTroopType,nGroup);
	for(unsigned int i = 0;i < vecTroopList.size();i++){
		vecTroopList[i]->CmdEnable(false);
	}
	return  ACTION_STATE_FINISH;
}


int CTroopTypeHPFillInArea::DoCommand()
{
	char* pAreaName  = GetParamStr( 0 );
	int   nGroup		 = GetParamInt( 1 );
	int   nTroopType = GetParamInt( 2 );
	int  nHPPercent = GetParamInt( 3 );
	AREA_INFO*	   pInfo	  = g_FcWorld.GetAreaInfo( pAreaName );

	std::vector<TroopObjHandle>vecTroopList;
	GetTroopCustomCondition(vecTroopList,pInfo,nTroopType,nGroup);
	for(unsigned int i = 0;i < vecTroopList.size();i++){
		TroopObjHandle hTroop = vecTroopList[i];
		hTroop->AddHPPercent( nHPPercent );
	}
	return  ACTION_STATE_FINISH;
}

int CTroopTypeHPMaxInArea::DoCommand()
{
	char* pAreaName  = GetParamStr( 0 );
	int   nGroup		 = GetParamInt( 1 );
	int   nTroopType = GetParamInt( 2 );

	AREA_INFO*	   pInfo = g_FcWorld.GetAreaInfo( pAreaName );
	std::vector<TroopObjHandle>vecTroopList;
	GetTroopCustomCondition(vecTroopList,pInfo,nTroopType,nGroup);
	for(unsigned int i = 0;i < vecTroopList.size();i++){
		vecTroopList[i]->SetHPPercent(100);
	}
	return  ACTION_STATE_FINISH;
}


int CTroopTypeWalk::DoCommand()
{
	char* pAreaName   = GetParamStr( 0 );
	int   nGroup	  = GetParamInt( 1 );
	int   nTroopType  = GetParamInt( 2 );
	char* pTargetArea = GetParamStr( 3 );
    
	AREA_INFO*	   pInfo = g_FcWorld.GetAreaInfo( pAreaName );
	AREA_INFO*	   pTargetInfo = g_FcWorld.GetAreaInfo( pTargetArea );

	std::vector<TroopObjHandle>vecTroopList;

	GetTroopCustomCondition(vecTroopList,pInfo,nTroopType,nGroup);
	for(unsigned int i = 0;i < vecTroopList.size();i++){
		D3DXVECTOR3 Pos = vecTroopList[i]->GetPos();
		D3DXVECTOR3 Dest;
		//aleksger: prefix bug 826: Dest.x may not be initialized in some cases
		BsAssert(Pos.x != pTargetInfo->fSX && pTargetInfo->fEX < Pos.x && "Pos.x is outside bounds"); 
		if( pTargetInfo->fSX < Pos.x && pTargetInfo->fEX > Pos.x ){		
			Dest.x = Pos.x;
		}
		else{
			if(pTargetInfo->fSX > Pos.x){			
				Dest.x = pTargetInfo->fSX;
			}
			else if(pTargetInfo->fEX < Pos.x){
				Dest.x = pTargetInfo->fEX;
			}
		}

		//aleksger: prefix bug 826: Dest.z may not be initialized in some cases
		BsAssert(Pos.z != pTargetInfo->fSZ && pTargetInfo->fEZ < Pos.z && "Pos.z is outside bounds"); 
		if( pTargetInfo->fSZ < Pos.z && pTargetInfo->fEZ > Pos.z ){
			Dest.z = Pos.z;
		}
		else{
			if(pTargetInfo->fSZ > Pos.z){			
				Dest.z = pTargetInfo->fSZ;
			}
			else if(pTargetInfo->fEZ < Pos.z){
				Dest.z = pTargetInfo->fEZ;
			}
		}

		TroopObjHandle hTroop = vecTroopList[i];
		hTroop->SetTriggerCmd();
		hTroop->CmdWalk(Dest.x,Dest.z);
		hTroop->ResetTriggerCmd();
	}
	return  ACTION_STATE_FINISH;
}

int CTroopTypeRun::DoCommand()
{
	char* pAreaName   = GetParamStr( 0 );
	int   nGroup	  = GetParamInt( 1 );
	int   nTroopType  = GetParamInt( 2 );
	char* pTargetArea = GetParamStr( 3 );

	AREA_INFO*	   pInfo = g_FcWorld.GetAreaInfo( pAreaName );
	AREA_INFO*	   pTargetInfo = g_FcWorld.GetAreaInfo( pTargetArea );
	std::vector<TroopObjHandle>vecTroopList;

	BsAssert( pInfo && pTargetInfo && "TroopTypeRun Area noe found" );

	GetTroopCustomCondition(vecTroopList,pInfo,nTroopType,nGroup);
	for(unsigned int i = 0;i < vecTroopList.size();i++){
		TroopObjHandle hTroop = vecTroopList[i];

		hTroop->SetTriggerCmd();
		hTroop->CmdRun((pTargetInfo->fSX + pTargetInfo->fEX) * 0.5f,(pTargetInfo->fSZ + pTargetInfo->fEZ) * 0.5f);
		hTroop->ResetTriggerCmd();
	}
	return  ACTION_STATE_FINISH;
}

int CTroopTypeStopInArea::DoCommand()
{
	char* pAreaName   = GetParamStr( 0 );
	int   nGroup		  = GetParamInt( 1 );
	int   nTroopType  = GetParamInt( 2 );

	AREA_INFO*	   pInfo = g_FcWorld.GetAreaInfo( pAreaName );
	std::vector<TroopObjHandle> vecTroopList;
	GetTroopCustomCondition(vecTroopList,pInfo,nTroopType,nGroup);

	BsAssert( pInfo && "CTroopTypeStopInArea Area noe found" );

	for(unsigned int i = 0;i < vecTroopList.size();i++)
	{
		TroopObjHandle hTroop = vecTroopList[i];
		hTroop->SetTriggerCmd();
		hTroop->CmdStop();
		hTroop->ResetTriggerCmd();
	}

	return  ACTION_STATE_FINISH;
}

int CTroopRangeAttackToArea::DoCommand()
{
	//미완성 Range 부대의 공격 타입이 없음
	BsAssert( 0 );
	return  ACTION_STATE_FINISH;
}



int CTroopTypeSetDirection::DoCommand()
{
	char* pAreaName   = GetParamStr( 0 );
	int   nGroup	  = GetParamInt( 1 );
	int   nTroopType  = GetParamInt( 2 );
	int   nClockDir	  = GetParamInt( 3 );

	AREA_INFO*	   pInfo = g_FcWorld.GetAreaInfo( pAreaName );
	std::vector<TroopObjHandle>vecTroopList;
	GetTroopCustomCondition(vecTroopList,pInfo,nTroopType,nGroup);
	for(unsigned int i = 0;i < vecTroopList.size();i++){
		vecTroopList[i]->SetDirByClock(nClockDir);
	}
	return  ACTION_STATE_FINISH;
}

int CTroopTypeShowInMinimap::DoCommand()
{	
	char* pAreaName   = GetParamStr( 0 );
	int   nGroup	  = GetParamInt( 1 );
	int   nTroopType  = GetParamInt( 2 );
	AREA_INFO*	   pInfo = g_FcWorld.GetAreaInfo( pAreaName );
	std::vector<TroopObjHandle>vecTroopList;
	GetTroopCustomCondition(vecTroopList,pInfo,nTroopType,nGroup);
	for(unsigned int i = 0;i < vecTroopList.size();i++){
		vecTroopList[i]->SetVisibleInMinimap( true );
	}
	return  ACTION_STATE_FINISH;
}

int CTroopTypeHideInMinimap::DoCommand()
{
	char* pAreaName   = GetParamStr( 0 );
	int   nGroup	  = GetParamInt( 1 );
	int   nTroopType  = GetParamInt( 2 );
	AREA_INFO*	   pInfo = g_FcWorld.GetAreaInfo( pAreaName );
	std::vector<TroopObjHandle>vecTroopList;
	GetTroopCustomCondition( vecTroopList, pInfo, nTroopType, nGroup);
	for(unsigned int i = 0;i < vecTroopList.size();i++){
		vecTroopList[i]->SetVisibleInMinimap( false );
	}
	return  ACTION_STATE_FINISH;
}

int CTroopShowInMinimap::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	hTroop->SetVisibleInMinimap(true);

	return  ACTION_STATE_FINISH;
}

int CTroopHideInMinimap::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	hTroop->SetVisibleInMinimap(false);
	return  ACTION_STATE_FINISH;
}

int CUnitOnPropShowInMinimap::DoCommand()
{
	char* pPropName = GetParamStr( 0 );
	TroopObjHandle hTroop = g_FcWorld.GetTroopmanager()->GetTroopByName( pPropName );
	hTroop->SetVisibleInMinimap( true );

	return  ACTION_STATE_FINISH;
}

int CUnitOnPropHideInMinimap::DoCommand()
{
	char* pPropName = GetParamStr( 0 );
	TroopObjHandle hTroop = g_FcWorld.GetTroopmanager()->GetTroopByName( pPropName );
	hTroop->SetVisibleInMinimap( false );

	return  ACTION_STATE_FINISH;
}


int CFcActPlayerAddOrb::DoCommand()
{
	int nPlayer = GetParamInt( 0 );
	int nOrb = GetParamInt( 1 );
	BsAssert( nPlayer == 0 || nPlayer == 1 );
	g_FcWorld.GetHeroHandle( nPlayer )->AddOrbSpark( nOrb );
	return  ACTION_STATE_FINISH;
}

int CFcActTeamShowHPGauge::DoCommand()
{
	int nTeam = GetParamInt( 0 );
	bool bShow = ( GetParamInt( 1 ) != 0 );
//	g_FcWorld.ShowTeamHP( nTeam, bShow );
	return  ACTION_STATE_FINISH;
}

int CFcActTeamShowHPGaugeInArea::DoCommand()
{
	char* pAreaName = GetParamStr( 0 );
	int nTeam = GetParamInt( 1 );
	int nType = GetParamInt( 2 );
	bool bShow = ( GetParamInt( 3 ) != 0 );
//	g_FcWorld.ShowTeamHP( nTeam, bShow, nType, pAreaName );
	return  ACTION_STATE_FINISH;
}

int CFcActTroopsAIEnable::DoCommand()
{
	char* pTroopStr = GetParamStr( 0 );

	CFcTroopManager* pTroopManager = g_FcWorld.GetTroopmanager();
	int nCnt = pTroopManager->GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hHandle = pTroopManager->GetTroop( i );

		char* pTempStr = strstr( (char*)hHandle->GetName(), pTroopStr ); 
		if( pTempStr == NULL )
			continue;

		g_FcWorld.EnableTroopAI( hHandle, true );
	}
	return  ACTION_STATE_FINISH;
}

int CFcActTroopsAIDisable::DoCommand()
{
	char* pTroopStr = GetParamStr( 0 );

	CFcTroopManager* pTroopManager = g_FcWorld.GetTroopmanager();
	int nCnt = pTroopManager->GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hHandle = pTroopManager->GetTroop( i );

		char* pTempStr = strstr( (char*)hHandle->GetName(), pTroopStr ); 
		if( pTempStr == NULL )
			continue;

		g_FcWorld.EnableTroopAI( hHandle, false );
	}
	return  ACTION_STATE_FINISH;
}

int CFcActTroopsSetAI::DoCommand()
{
	char* pTroopStr = GetParamStr( 0 );
	int nAIID = GetParamInt( 1 );

	CFcTroopManager* pTroopManager = g_FcWorld.GetTroopmanager();
	int nCnt = pTroopManager->GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hHandle = pTroopManager->GetTroop( i );

		char* pTempStr = strstr( (char*)hHandle->GetName(), pTroopStr ); 
 		if( pTempStr == NULL )
			continue;

		g_FcWorld.SetTroopAI( hHandle, nAIID );
		g_FcWorld.EnableTroopAI( hHandle, true );
	}
	return  ACTION_STATE_FINISH;
}

int CFcActTroopsPlayAction::DoCommand()
{
	char* pTroopStr = GetParamStr( 0 );
	int nAni = GetParamInt( 1 );
	int nIndex = GetParamInt( 2 );

	CFcTroopManager* pTroopManager = g_FcWorld.GetTroopmanager();
	int nCnt = pTroopManager->GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hHandle = pTroopManager->GetTroop( i );
		if( !hHandle->IsEnable() )
			continue;

		if( hHandle->IsEliminated() )
			continue;
		
		char* pTempStr = strstr( (char*)hHandle->GetName(), pTroopStr ); 
		if( pTempStr == NULL )
			continue;

		// TODO: 동작 시간 약간 랜덤하게 줘야 한다.
		int nUCnt = hHandle->GetUnitCount(); //aleksger: prefix bug 828: Local variable hiding global scope.
		for( int i=0; i<nUCnt; i++ )
		{
			GameObjHandle hUnit = hHandle->GetUnit( i );

			CFcGameObject::GameObj_ClassID ClassID = hUnit->GetClassID();
			if( ClassID == CFcGameObject::Class_ID_Horse ||
				ClassID == CFcGameObject::Class_ID_Catapult ||
				ClassID == CFcGameObject::Class_ID_MoveTower ||
				ClassID == CFcGameObject::Class_ID_Fly )
				continue;

			if( hUnit->IsDie() )
				continue;

//			hUnit->ChangeAnimation( nAni, nIndex, hUnit->GetCurAniAttr(), false, 1, true );
			hUnit->ChangeAnimation( nAni, nIndex, 1 );		// Attr 공격중(1)으로 하드코딩!

			if( hUnit == g_FcWorld.GetHeroHandle() )
				g_FcWorld.GetHeroHandle()->UnlinkCatchObject( 0, -1, 0, true );
		}
	}
	return  ACTION_STATE_FINISH;
}

int CFcActTroopsAnnihilate::DoCommand()
{
	char* pTroopStr = GetParamStr( 0 );

	CFcTroopManager* pTroopManager = g_FcWorld.GetTroopmanager();
	int nCnt = pTroopManager->GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hHandle = pTroopManager->GetTroop( i );
		if( !hHandle->IsEnable() )
			continue;

		if( hHandle->IsEliminated() )
			continue;

		char* pTempStr = strstr( (char*)hHandle->GetName(), pTroopStr ); 
		if( pTempStr == NULL )
			continue;

		hHandle->CmdAnnihilate();
	}
	return  ACTION_STATE_FINISH;
}

int CFcActTroopsDisable::DoCommand()
{
	char* pTroopStr = GetParamStr( 0 );

	CFcTroopManager* pTroopManager = g_FcWorld.GetTroopmanager();
	int nCnt = pTroopManager->GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hHandle = pTroopManager->GetTroop( i );
		if( !hHandle->IsEnable() )
			continue;

		char* pTempStr = strstr( (char*)hHandle->GetName(), pTroopStr ); 
		if( pTempStr == NULL )
			continue;

		hHandle->CmdEnable( false );
	}
	return  ACTION_STATE_FINISH;
}

int CFcActTroopsHPFill::DoCommand()
{
	char* pTroopStr = GetParamStr( 0 );
	int nPercent = GetParamInt( 1 );

	CFcTroopManager* pTroopManager = g_FcWorld.GetTroopmanager();
	int nCnt = pTroopManager->GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hHandle = pTroopManager->GetTroop( i );
		if( !hHandle->IsEnable() )
			continue;

		if( hHandle->IsEliminated() )
			continue;

		char* pTempStr = strstr( (char*)hHandle->GetName(), pTroopStr ); 
		if( pTempStr == NULL )
			continue;

		hHandle->AddHPPercent( nPercent );
	}
	return  ACTION_STATE_FINISH;
}

int CFcActTroopsHPMax::DoCommand()
{
	char* pTroopStr = GetParamStr( 0 );

	CFcTroopManager* pTroopManager = g_FcWorld.GetTroopmanager();
	int nCnt = pTroopManager->GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hHandle = pTroopManager->GetTroop( i );
		if( !hHandle->IsEnable() )
			continue;

		if( hHandle->IsEliminated() )
			continue;

		char* pTempStr = strstr( (char*)hHandle->GetName(), pTroopStr ); 
		if( pTempStr == NULL )
			continue;

		hHandle->SetHPPercent( 100 );
	}
	return  ACTION_STATE_FINISH;
}

int CFcActTroopsInvulnerable::DoCommand()
{
	char* pTroopStr = GetParamStr( 0 );

	CFcTroopManager* pTroopManager = g_FcWorld.GetTroopmanager();
	int nCnt = pTroopManager->GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hHandle = pTroopManager->GetTroop( i );
		if( !hHandle->IsEnable() )
			continue;

		if( hHandle->IsEliminated() )
			continue;

		char* pTempStr = strstr( (char*)hHandle->GetName(), pTroopStr ); 
		if( pTempStr == NULL )
			continue;

		hHandle->CmdInvulnerable( true );
	}
	return  ACTION_STATE_FINISH;
}

int CFcActTroopsVulnerable::DoCommand()
{
	char* pTroopStr = GetParamStr( 0 );

	CFcTroopManager* pTroopManager = g_FcWorld.GetTroopmanager();
	int nCnt = pTroopManager->GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hHandle = pTroopManager->GetTroop( i );
		if( !hHandle->IsEnable() )
			continue;

		if( hHandle->IsEliminated() )
			continue;

		char* pTempStr = strstr( (char*)hHandle->GetName(), pTroopStr ); 
		if( pTempStr == NULL )
			continue;

		hHandle->CmdInvulnerable( false );
	}
	return  ACTION_STATE_FINISH;
}

int CFcActTroopsWalk::DoCommand()
{
	char* pTroopStr = GetParamStr( 0 );
	char* pAreaName = GetParamStr( 1 );
	
	AREA_INFO*	   pInfo = g_FcWorld.GetAreaInfo( pAreaName );
	float fCX = ( pInfo->fEX - pInfo->fSX ) / 2;
	float fCZ = ( pInfo->fEZ - pInfo->fSZ ) / 2;

	// TODO : 부대 마다 가까운 위치로 고쳐야 한다.
	float fX = pInfo->fSX + fCX;
	float fZ = pInfo->fSZ + fCZ;

	CFcTroopManager* pTroopManager = g_FcWorld.GetTroopmanager();
	int nCnt = pTroopManager->GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hHandle = pTroopManager->GetTroop( i );
		if( !hHandle->IsEnable() )
			continue;

		if( hHandle->IsEliminated() )
			continue;

		char* pTempStr = strstr( (char*)hHandle->GetName(), pTroopStr ); 
		if( pTempStr == NULL )
			continue;

		hHandle->SetTriggerCmd();
		hHandle->CmdWalk( fX, fZ );
		hHandle->ResetTriggerCmd();
	}
	return  ACTION_STATE_FINISH;
}

int CFcActTroopsRun::DoCommand()
{
	char* pTroopStr = GetParamStr( 0 );
	char* pAreaName = GetParamStr( 1 );

	AREA_INFO*	   pInfo = g_FcWorld.GetAreaInfo( pAreaName );
	float fCX = ( pInfo->fEX - pInfo->fSX ) / 2;
	float fCZ = ( pInfo->fEZ - pInfo->fSZ ) / 2;

	// TODO : 부대 마다 가까운 위치로 고쳐야 한다.
	float fX = pInfo->fSX + fCX;
	float fZ = pInfo->fSZ + fCZ;

	CFcTroopManager* pTroopManager = g_FcWorld.GetTroopmanager();
	int nCnt = pTroopManager->GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hHandle = pTroopManager->GetTroop( i );
		if( !hHandle->IsEnable() )
			continue;

		if( hHandle->IsEliminated() )
			continue;

		char* pTempStr = strstr( (char*)hHandle->GetName(), pTroopStr ); 
		if( pTempStr == NULL )
			continue;

		hHandle->SetTriggerCmd();
		hHandle->CmdRun( fX, fZ );
		hHandle->ResetTriggerCmd();
	}
	return  ACTION_STATE_FINISH;
}

int CFcActTroopsStop::DoCommand()
{
	char* pTroopStr = GetParamStr( 0 );
	CFcTroopManager* pTroopManager = g_FcWorld.GetTroopmanager();
	int nCnt = pTroopManager->GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hHandle = pTroopManager->GetTroop( i );
		if( !hHandle->IsEnable() )
			continue;

		if( hHandle->IsEliminated() )
			continue;

		char* pTempStr = strstr( (char*)hHandle->GetName(), pTroopStr ); 
		if( pTempStr == NULL )
			continue;

		hHandle->SetTriggerCmd();
		hHandle->CmdStop();
		hHandle->ResetTriggerCmd();
	}

	return  ACTION_STATE_FINISH;
}

int CFcActTroopsRangeAttackArea::DoCommand()
{
	BsAssert(0);
	return  ACTION_STATE_FINISH;
}

int CFcActTroopsSetDir::DoCommand()
{
	char* pTroopStr = GetParamStr( 0 );
	int nClockDir = GetParamInt( 1 );
	CFcTroopManager* pTroopManager = g_FcWorld.GetTroopmanager();
	int nCnt = pTroopManager->GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hHandle = pTroopManager->GetTroop( i );

		char* pTempStr = strstr( (char*)hHandle->GetName(), pTroopStr ); 
		if( pTempStr == NULL )
			continue;

		hHandle->SetDirByClock(nClockDir);
	}
	return  ACTION_STATE_FINISH;
}

int CFcActTroopsShowMinimap::DoCommand()
{
	char* pTroopStr = GetParamStr( 0 );
	bool bShow = ( GetParamInt( 1 ) != 0 );
	CFcTroopManager* pTroopManager = g_FcWorld.GetTroopmanager();
	int nCnt = pTroopManager->GetTroopCount();
	TroopObjHandle hHandle;
	for( int i=0; i<nCnt; i++ )
	{
		hHandle = pTroopManager->GetTroop( i );
		if( !hHandle->IsEnable() )
			continue;

		if( hHandle->IsEliminated() )
			continue;

		char* pTempStr = strstr( (char*)hHandle->GetName(), pTroopStr ); 
		if( pTempStr == NULL )
			continue;

		g_InterfaceManager.GetInstance().MinimapPointOn(bShow, hHandle->GetName(), hHandle, 1000000);
	}
	return  ACTION_STATE_FINISH;
}

int CFcActTroopsShowHPGauge::DoCommand()
{
	BsAssert( 0 );
	return  ACTION_STATE_FINISH;
}

int CFcActTroopsChangeGroup::DoCommand()
{
	BsAssert( 0 );
	return  ACTION_STATE_FINISH;
}


int CFcActDynamicFogEnable::DoCommand()
{
	char *pAreaName1 = GetParamStr(0);
	AREA_INFO *pAreaInfo = g_FcWorld.GetAreaInfo( pAreaName1 );
	D3DXVECTOR2	AreaSize = D3DXVECTOR2(pAreaInfo->GetWidth(), pAreaInfo->GetHeight());
	D3DXVECTOR2	StartPoint = D3DXVECTOR2(pAreaInfo->fSX, pAreaInfo->fSZ);

	char szFile[128];
	sprintf( szFile, "%s", "cloud_t2(2).dds" ); // 칠흑의 어둠포그 : cloud_011dark //  일반포그 : cloud_t2(2)

	if (g_FcWorld.GetFXDynamicFogID() != -1) {
		int nFogID = g_FcWorld.GetFXDynamicFogID();
		SAFE_DELETE_FX( nFogID, FX_TYPE_DYNAMICFOG );
		g_FcWorld.SetFXDynamicFogID(-1);

		g_FcWorld.CreateDynamicFog();

		g_pFcFXManager->SendMessage(g_FcWorld.GetFXDynamicFogID(), FX_INIT_OBJECT, (DWORD)&AreaSize, (DWORD)&StartPoint , (DWORD)szFile);		
		g_pFcFXManager->SendMessage(g_FcWorld.GetFXDynamicFogID(),FX_PLAY_OBJECT, 10);
	}
	else
	{
		g_FcWorld.CreateDynamicFog();
		BsAssert( g_FcWorld.GetFXDynamicFogID() != -1 );
		g_pFcFXManager->SendMessage(g_FcWorld.GetFXDynamicFogID(), FX_INIT_OBJECT, (DWORD)&AreaSize, (DWORD)&StartPoint , (DWORD)szFile);		
		g_pFcFXManager->SendMessage(g_FcWorld.GetFXDynamicFogID(),FX_PLAY_OBJECT, 10);
	}

	return  ACTION_STATE_FINISH;
}


int CFcActDynamicFogCross::DoCommand()
{
	BsAssert( 0 && "Not use TriggerAction!" );

	char *pAreaName1 = GetParamStr(0);
	char *pAreaName2 = GetParamStr(1);

	D3DXVECTOR3 vStartPosition, vEndPosition, vDirCross;
	AREA_INFO *pAreaInfo = g_FcWorld.GetAreaInfo( pAreaName1 );
	float fx, fz;
	fx = pAreaInfo->fSX + pAreaInfo->GetWidth() / 2;
	fz = pAreaInfo->fSZ + pAreaInfo->GetHeight() / 2;
	vStartPosition = D3DXVECTOR3(fx, g_BsKernel.GetInstance().GetHeight(fx, fz), fz);
	pAreaInfo = g_FcWorld.GetAreaInfo( pAreaName2 );
	fx = pAreaInfo->fSX + pAreaInfo->GetWidth() / 2;
	fz = pAreaInfo->fSZ + pAreaInfo->GetHeight() / 2;
	vEndPosition = D3DXVECTOR3(fx, g_BsKernel.GetInstance().GetHeight(fx, fz), fz);
	vDirCross = vEndPosition - vStartPosition;
	D3DXVec3Cross(&vDirCross, &D3DXVECTOR3(0.0f, 1.0f, 0.0f), &vDirCross);
	D3DXVec3Normalize(&vDirCross, &vDirCross);

	if (g_FcWorld.GetFXDynamicFogID() != -1) {
		g_pFcFXManager->SendMessage(g_FcWorld.GetFXDynamicFogID(), FX_UPDATE_OBJECT, (DWORD)&vStartPosition, (DWORD)&vDirCross);
	}
	return ACTION_STATE_FINISH;
}

int CFcActDynamicFogDisable::DoCommand()
{
	if (g_FcWorld.GetFXDynamicFogID() != -1) {
//		g_pFcFXManager->SendMessage(g_FcWorld.GetFXDynamicFogID(), FX_DELETE_OBJECT);
		int nFogID = g_FcWorld.GetFXDynamicFogID();
		SAFE_DELETE_FX( nFogID, FX_TYPE_DYNAMICFOG );
		g_FcWorld.SetFXDynamicFogID(-1);
	}
	return ACTION_STATE_FINISH;
}


int CFcActSetDOF::DoCommand()
{
	int nFocus = GetParamInt( 0 );
	int nRange = GetParamInt( 1 );
	int nNear = GetParamInt( 2 );
	int nFar = GetParamInt( 3 );

	g_BsKernel.EnableDOF( TRUE );
	g_BsKernel.SetDOFFocus( (float)nFocus );
	g_BsKernel.SetDOFFocusRange( (float)nRange );
	g_BsKernel.SetDOFFocusNear( (float)nNear);
	g_BsKernel.SetDOFFocusFar( (float)nFar);

	return ACTION_STATE_FINISH;
}


int CFcActDisableDOF::DoCommand()
{
	g_BsKernel.EnableDOF( FALSE );
	return ACTION_STATE_FINISH;
}

int CFcActDestroyCrumbleStone::DoCommand()
{
	char* pPropName = GetParamStr( 0 );
	CFcProp* pProp = g_FcWorld.GetProp( pPropName );
	
	if (pProp == NULL) {
		DebugString(__FUNCTION__ " : Failed to attain prop %s ", pPropName);
		return ACTION_STATE_FINISH;
	}

	BsAssert(pProp && pProp->IsBreakAndBlockWayProp()); //aleksger: prefix bug :GetProp() may return NULL;
	CFcBreakAndBlockWayProp * pBreaAndBlockWayProp = (CFcBreakAndBlockWayProp *)pProp;
	pBreaAndBlockWayProp->Destroy();
	if(g_pSoundManager){ g_pSoundManager->PlaySystemSound(SB_COMMON,"EV_EAURVARRIA_01"); }
	return ACTION_STATE_FINISH;
}

int CFcActSetPropAIGuide::DoCommand()
{
	int nGuideIndex = GetParamInt( 0 );
	char* pPropName = GetParamStr( 1 );
	CFcProp* pProp = g_FcWorld.GetProp( pPropName );
	BsAssert(pProp); //aleksger: prefix bug :GetProp() may return NULL;
	CFcTroopAIObject::SetAIGuide( nGuideIndex, pProp, NULL );
	return ACTION_STATE_FINISH;
}

int CFcActSetAreaAIGuide::DoCommand()
{
	int nGuideIndex = GetParamInt( 0 );
	char *pAreaName1 = GetParamStr( 1 );
	AREA_INFO *pAreaInfo = g_FcWorld.GetAreaInfo( pAreaName1 );
	CFcTroopAIObject::SetAIGuide( nGuideIndex, NULL, pAreaInfo );
	return ACTION_STATE_FINISH;
}

int CFcActSetGuardianAI::DoCommand()
{
	int nAttackAI = GetParamInt( 0 );
	int nDefenseAI = GetParamInt( 1 );
	int nNormalAI = GetParamInt( 2 );

	g_FcWorld.GetTroopmanager()->SetGuardianAI( nAttackAI, nDefenseAI, nNormalAI );

	return ACTION_STATE_FINISH;
}



int CFcActMoveMoveTower::DoCommand()
{
	char *pAreaName1 = GetParamStr(0);
	AREA_INFO *pInfo = g_FcWorld.GetAreaInfo( pAreaName1 );

	char* pPropName = GetParamStr( 1 );
	CFcProp* pProp = g_FcWorld.GetProp( pPropName );

	if( pProp == NULL )
	{
		DebugString( "MoveTower cannot found Target\n" );
		return ACTION_STATE_FINISH;
	}


	// TODO: Area에 타워 부대를 생성하는 루틴 필요
	float fCX = ( pInfo->fEX - pInfo->fSX ) / 2;
	float fCZ = ( pInfo->fEZ - pInfo->fSZ ) / 2;
	float fX = pInfo->fSX + fCX;
	float fZ = pInfo->fSZ + fCZ;
	
	D3DXVECTOR2 Pos( fX, fZ );
	std::vector<TroopObjHandle> Objects;
	g_FcWorld.GetTroopmanager()->GetListInRange( &Pos, 1000.f, Objects );

	int nCnt = Objects.size();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTroop = Objects[i];
		if( hTroop->GetType() == TROOPTYPE_MOVETOWER )
		{
			// pProp에 가장 가까운 booking으로 위치 설정
			float fNearestDist = -1.f;
			D3DXVECTOR2 NearestPos( -1.f, -1.f );

			int nBookingCnt = pProp->GetBookingNum();
			BsAssert( nBookingCnt > 0 );
			for( int i=0; i<nBookingCnt; i++ )
			{
				CCrossVector Cross;
				D3DXMATRIX matObject;
				matObject = *(D3DXMATRIX *)g_BsKernel.SendMessage( pProp->GetEngineIndex(), BS_GET_OBJECT_MAT );

				Cross.m_PosVector = *(pProp->GetBookingPos( i ));// + pProp->GetPos();
				D3DXVec3TransformCoord( &Cross.m_PosVector, &Cross.m_PosVector, &matObject );

				D3DXVECTOR2 Dir = hTroop->GetPosV2() - D3DXVECTOR2( Cross.m_PosVector.x, Cross.m_PosVector.z );
				float fDist = D3DXVec2Length( &Dir );
				if( fNearestDist < 0 || fNearestDist > fDist )
				{
					fNearestDist = fDist;
					NearestPos = D3DXVECTOR2( Cross.m_PosVector.x, Cross.m_PosVector.z );
				}
			}

			if( fNearestDist > 0 )
			{
				hTroop->SetTriggerCmd();
				hTroop->CmdMove( NearestPos.x, NearestPos.y, 1.f );
				hTroop->ResetTriggerCmd();
			}
			else
			{
				BsAssert( 0 );
			}
		}
	}
	return ACTION_STATE_FINISH;
}



int CFcActAddObjective::DoCommand()
{
	int nID = GetParamInt( 0 );
	int nTextID = GetParamInt( 1 );
//	int nSecond = GetParamInt( 2 );

	// TODO: Tick 처리해야 함
//	int nTick = nSecond * FRAME_PER_SEC;
	g_FcWorld.AddObjective( nID, nTextID, false );
	return ACTION_STATE_FINISH;
}



int CFcActRemoveObjective::DoCommand()
{
	int nID = GetParamInt( 0 );
	g_FcWorld.RemoveObjective( nID );
	return ACTION_STATE_FINISH;
}



int CFcActClearObjective::DoCommand()
{
	int nID = GetParamInt( 0 );
	g_FcWorld.ClearObjective( nID );
	return ACTION_STATE_FINISH;
}



int CFcActDisplayVar ::DoCommand()
{
	int nVarID = GetParamInt( 0 );
	int nTextID = GetParamInt( 1 );
	int nX = GetParamInt( 2 );
	int nY = GetParamInt( 3 );
	int nVariableValue = -1;
	char szTempStr[STR_SPEECH_MAX] = "";
	if( nTextID != -1 )
	{
		g_TextTable->GetText(nTextID,szTempStr,_countof(szTempStr));
	}
	
	g_FcWorld.ShowVarText( szTempStr, nVarID );
	return ACTION_STATE_FINISH;
}

int CFcActHideVar::DoCommand()
{
	int nVarID = GetParamInt( 0 );
	g_FcWorld.HideVarText( nVarID );
	return ACTION_STATE_FINISH;
}



int CFcActDisplayVarGauge::DoCommand()
{
	int nVarID = GetParamInt( 0 );
	int nX = GetParamInt( 1 );
	int nY = GetParamInt( 2 );
	int nVariableValue = -1;
	nVariableValue = g_FcWorld.GetTriggerVarInt( nVarID );
	g_FcWorld.ShowVarGauge( nVarID );
	return ACTION_STATE_FINISH;
}


int CFcActHideVarGauge::DoCommand()
{
	int nVarID = GetParamInt( 0 );
	g_FcWorld.HideVarGauge( nVarID );
	return ACTION_STATE_FINISH;
}





int CFcActReleaseGuardian::DoCommand()
{
	int nType = GetParamInt( 0 );
	if( nType == 0 )
	{
		for( int i=0; i<2; i++ )
		{
			if( g_FcWorld.GetTroopmanager()->RegisterCancelGuardian( i ) == true )
				break;
		}
	}
	else if( nType == 1 )
	{
		for( int i=0; i<2; i++ )
			g_FcWorld.GetTroopmanager()->RegisterCancelGuardian( i );
	}
	else 
	{
		int nIndex = nType-2;
		BsAssert( nIndex == 0 || nIndex == 1 );
		g_FcWorld.GetTroopmanager()->RegisterCancelGuardian( nIndex );
	}

	return ACTION_STATE_FINISH;
}


int CFcActSetVarLiveTroopUnitCount::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	int nVarID = GetParamInt( 1 );
	int nCnt = hTroop->GetAliveUnitCount();
	if(!hTroop->IsEnable())
		nCnt = 0;
	g_FcWorld.SetTriggerVar( nVarID, nCnt );
	return ACTION_STATE_FINISH;
}

int CFcActSetVarDeadTroopUnitCount::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	int nVarID = GetParamInt( 1 );
	int nCnt = hTroop->GetDeadUnitCount();
	if(!hTroop->IsEnable())
		nCnt = 0;
	g_FcWorld.SetTriggerVar( nVarID, nCnt );
	return ACTION_STATE_FINISH;
}


int CFcActSetVarLiveForceUnitCount::DoCommand()
{
	int nForceID = GetParamInt( 0 );
	int nVarID = GetParamInt( 1 );
	int nLiveUnitCnt = 0;

	int nCnt = g_FcWorld.GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTroop = g_FcWorld.GetTroopObject( i );
		if( hTroop->GetForce() != nForceID )
			continue;
		if(!hTroop->IsEnable())
			continue;

		nLiveUnitCnt += hTroop->GetAliveUnitCount();
	}
	g_FcWorld.SetTriggerVar( nVarID, nLiveUnitCnt );
	return ACTION_STATE_FINISH;
}

int CFcActSetVarDeadForceUnitCount::DoCommand()
{
	int nForceID = GetParamInt( 0 );
	int nVarID = GetParamInt( 1 );
	int nLiveUnitCnt = 0;

	int nCnt = g_FcWorld.GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTroop = g_FcWorld.GetTroopObject( i );
		if( hTroop->GetForce() != nForceID )
			continue;
		if(!hTroop->IsEnable())
			continue;

		nLiveUnitCnt += hTroop->GetDeadUnitCount();
	}
	g_FcWorld.SetTriggerVar( nVarID, nLiveUnitCnt );
	return ACTION_STATE_FINISH;
}


int CFcActPlayAmbiSoundOn::DoCommand()
{
 	int nAmbiSoundCueID = GetParamInt( 0 );
	if( g_pSoundManager->IsAmbiencePlay() ) {	
		return ACTION_STATE_FINISH;
	}
	g_pSoundManager->PlayAmbience( nAmbiSoundCueID );
	return ACTION_STATE_FINISH;
}

int CFcActPlayAmbiSoundOff::DoCommand()
{
	g_pSoundManager->StopAmbience();
	return ACTION_STATE_FINISH;
}


int CFcActSetNextStage::DoCommand()
{
	char* pNextStageName = GetParamStr( 0 );
	BsAssert( strlen( pNextStageName ) < 64 );
	strcpy( g_FCGameData.cNextStageMapFileName, pNextStageName );
	return ACTION_STATE_FINISH;
}

int CFcActLinkElfNDragon::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	TroopObjHandle hElfTroop = g_FcWorld.GetTroopObject( pTroopName );
	pTroopName = GetParamStr( 1 );
	TroopObjHandle hDragonTroop = g_FcWorld.GetTroopObject( pTroopName );

	BsAssert( hDragonTroop->GetType() == TROOPTYPE_FLYING );
	hElfTroop->LinkTroop( hDragonTroop );
	hDragonTroop->LinkTroop( hElfTroop );

	return ACTION_STATE_FINISH;
}

int CFcActShowBossGauge::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	int nTextID = GetParamInt( 1 );
	g_InterfaceManager.SetEnemyBossGauge( hTroop, nTextID );

	return ACTION_STATE_FINISH;
}

int CFcActHideBossGauge::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	g_InterfaceManager.RemoveEnemyBossGauge( hTroop );

	return ACTION_STATE_FINISH;
}


int CFcActAddTroopLiveRate::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	g_FcWorld.AddResultCalcTroop( hTroop );

	return ACTION_STATE_FINISH;
}

int CFcActSetDynamicPropGroupInArea::DoCommand()
{
	char* pAreaName = GetParamStr( 0 );
	int nTeam = GetParamInt( 1 );

 	AREA_INFO *pAreaInfo = g_FcWorld.GetAreaInfo( pAreaName );

	std::vector<CFcProp*> vecProps;
	g_FcWorld.GetPropManager()->GetActiveProps( pAreaInfo->fSX, pAreaInfo->fSZ, pAreaInfo->fEX, pAreaInfo->fEZ, 
												vecProps );

	int nCnt = vecProps.size();
	for( int i=0; i<nCnt; i++ )
	{
		CFcProp* pProp = vecProps[i];
		if( pProp->IsDynamicProp() )
		{
			CFcDynamicProp* pDynamicProp = (CFcDynamicProp*)pProp;
			pDynamicProp->SetTeam( nTeam );
		}
	}
	return ACTION_STATE_FINISH;
}




int CFcActDropItemAtTroopPos::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	int nItemIndex = GetParamInt( 1 );

	D3DXVECTOR3 Pos = hTroop->GetPos();
	Pos.x += (float)Random( 400 ) - 200.f;
	Pos.z += (float)Random( 400 ) - 200.f;

	g_FcItemManager.CreateItemToWorld( nItemIndex, Pos );
	return ACTION_STATE_FINISH;
}

int CFcActDropItemAtLeaderPos::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	int nItemIndex = GetParamInt( 1 );

	GameObjHandle hLeader = hTroop->GetLeader();
	if( hLeader == NULL )
	{
		if( hTroop->GetUnitCount() > 0 )
			hLeader = hTroop->GetUnit( 0 );
	}

	if( hLeader )
	{
		D3DXVECTOR3 Pos = hLeader->GetPos();

		Pos.x += (float)Random( 400 ) - 200.f;
		Pos.z += (float)Random( 400 ) - 200.f;

		g_FcItemManager.CreateItemToWorld( nItemIndex, Pos );
	}
	else
	{
		DebugString( "Cannot found leader in CFcActDropItemAtLeaderPos\n" );
	}
	return ACTION_STATE_FINISH;
}

int CFcActDropItemInArea::DoCommand()
{
	char* pAreaName = GetParamStr( 0 );
	AREA_INFO *pAreaInfo = g_FcWorld.GetAreaInfo( pAreaName );
	int nItemIndex = GetParamInt( 1 );

	D3DXVECTOR3 Pos;
	Pos.x = pAreaInfo->fSX + ( pAreaInfo->fEX - pAreaInfo->fSX ) / 2.f;
	Pos.z = pAreaInfo->fSZ + ( pAreaInfo->fEZ - pAreaInfo->fSZ ) / 2.f;
	Pos.y = g_BsKernel.GetLandHeight( Pos.x, Pos.z );

	g_FcItemManager.CreateItemToWorld( nItemIndex, Pos );
	return ACTION_STATE_FINISH;
}


int CFcActSetRealtimeMovieAfterMissionEnd::DoCommand()
{
	char* pStr = GetParamStr( 0 );
	strcpy( g_FCGameData.cFinishRealtimeMovieFileName, pStr );
	return ACTION_STATE_FINISH;
};


int CFcActStopTrigger::DoCommand()
{
	g_FcWorld.GetTriggerCommander()->SetStopTrigger( true );
	return ACTION_STATE_FINISH;
}


int CFcActTroopsEnable::DoCommand()
{
	if( m_bFirst )
	{
		char* pTroopStr = GetParamStr( 0 );
		m_nID = g_FcWorld.GetTroopmanager()->SetEnableTroopByName( pTroopStr );
		m_bFirst = false;
	}
	else
	{
		if( g_FcWorld.GetTroopmanager()->IsFinishEnableTroops( m_nID ) )
		{
			return ACTION_STATE_FINISH;
		}
	}
	return ACTION_STATE_WORKING;
}


int CFcActChangeTeam::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	int nNewTeamID = GetParamInt( 1 );
	int nNewForce = GetParamInt( 2 );

	//hTroop->SetTeam(nNewTeamID);
	hTroop->ChangeTeam(nNewTeamID); 
	hTroop->SetForce(nNewForce);

	return ACTION_STATE_FINISH;
}

int CFcActGuardianWalk::DoCommand() {
	int nIndex = GetParamInt( 0 );
	char* pAreaName = GetParamStr( 1 );

	AREA_INFO* pInfo = g_FcWorld.GetAreaInfo( pAreaName );	
	TroopObjHandle hTroop = g_FcWorld.GetTroopmanager()->GetGuardianTroop(nIndex);
	if(!hTroop)
		return ACTION_STATE_FINISH;

	float fCX = ( pInfo->fEX - pInfo->fSX ) / 2;
	float fCZ = ( pInfo->fEZ - pInfo->fSZ ) / 2;

	float fX = pInfo->fSX + fCX;
	float fZ = pInfo->fSZ + fCZ;

	hTroop->SetTriggerCmd();
	hTroop->CmdWalk( fX, fZ );
	hTroop->ResetTriggerCmd();
		
	return ACTION_STATE_FINISH;
}

int CFcActGuardianRun::DoCommand() {
	int nIndex = GetParamInt( 0 );
	char* pAreaName = GetParamStr( 1 );

	AREA_INFO* pInfo = g_FcWorld.GetAreaInfo( pAreaName );	
	TroopObjHandle hTroop = g_FcWorld.GetTroopmanager()->GetGuardianTroop(nIndex);
	if(!hTroop)
		return ACTION_STATE_FINISH;

	float fCX = ( pInfo->fEX - pInfo->fSX ) / 2;
	float fCZ = ( pInfo->fEZ - pInfo->fSZ ) / 2;

	float fX = pInfo->fSX + fCX;
	float fZ = pInfo->fSZ + fCZ;

	hTroop->SetTriggerCmd();
	hTroop->CmdRun( fX, fZ );
	hTroop->ResetTriggerCmd();
		
	return ACTION_STATE_FINISH;
}

int CFcActGuardianStop::DoCommand() {
	int nIndex = GetParamInt( 0 );	
	TroopObjHandle hTroop = g_FcWorld.GetTroopmanager()->GetGuardianTroop(nIndex);
	if(!hTroop)
		return ACTION_STATE_FINISH;


	hTroop->SetTriggerCmd();
	hTroop->CmdStop();
	hTroop->ResetTriggerCmd();

	return ACTION_STATE_FINISH;
}

int CFcActGuardianSetDir::DoCommand() {
	int nIndex = GetParamInt( 0 );	
	int nClockDir = GetParamInt( 1 );	

	TroopObjHandle hTroop = g_FcWorld.GetTroopmanager()->GetGuardianTroop(nIndex);
	if(!hTroop)
		return ACTION_STATE_FINISH;

	hTroop->SetDirByClock(nClockDir);

	return ACTION_STATE_FINISH;
}

int CFcActGuardianRetreat::DoCommand() {
	int nIndex = GetParamInt( 0 );
	char* pAreaName = GetParamStr( 1 );

	AREA_INFO* pInfo = g_FcWorld.GetAreaInfo( pAreaName );	
	TroopObjHandle hTroop = g_FcWorld.GetTroopmanager()->GetGuardianTroop(nIndex);
	if(!hTroop)
		return ACTION_STATE_FINISH;


	float fCX = ( pInfo->fEX - pInfo->fSX ) / 2;
	float fCZ = ( pInfo->fEZ - pInfo->fSZ ) / 2;

	float fX = pInfo->fSX + fCX;
	float fZ = pInfo->fSZ + fCZ;

	hTroop->SetTriggerCmd();
	hTroop->CmdRetreat( fX, fZ );
	hTroop->ResetTriggerCmd();
		
	return ACTION_STATE_FINISH;
}

int CFcActGuardianAISet::DoCommand()
{
	int nIndex = GetParamInt( 0 );
	int nAIID = GetParamInt( 1 );

	TroopObjHandle hTroop = g_FcWorld.GetTroopmanager()->GetGuardianTroop(nIndex);
	if(!hTroop)
		return ACTION_STATE_FINISH;


	g_FcWorld.SetTroopAI( hTroop, nAIID );
	return ACTION_STATE_FINISH;
}

int CFcActTroopPickoutEnable::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	bool bPickout = GetParamInt( 1 ) ? true : false;
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	hTroop->SetEnableGuardian(bPickout);
	return  ACTION_STATE_FINISH;
}

int CFcActGuardianIncPower::DoCommand()
{
	int nIndex = GetParamInt( 0 );	
	int nPow = GetParamInt( 1 );

	TroopObjHandle hTroop = g_FcWorld.GetTroopmanager()->GetGuardianTroop( nIndex);
	if(!hTroop)
		return ACTION_STATE_FINISH;

	
	int nUnits = hTroop->GetUnitCount();
	for(int i=0; i < nUnits; ++i) 
		hTroop->GetUnit(i)->SetAttackPowerAdd(nPow); // 내부적으로 퍼센티지 계산
	
	return  ACTION_STATE_FINISH;
}

int CFcActGuardianIncRange::DoCommand()
{
	int nIndex = GetParamInt( 0 );	
	int nInc = GetParamInt( 1 );	

	TroopObjHandle hTroop = g_FcWorld.GetTroopmanager()->GetGuardianTroop(nIndex);
	if(!hTroop)
		return ACTION_STATE_FINISH;

	float nRadius = hTroop->GetAttackRadius();
	nRadius += hTroop->GetAttackRadius()*(nInc/100);
	hTroop->SetAttackRadius(nRadius);

	return  ACTION_STATE_FINISH;
}

int CFcActTroopIncPower::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );		
	int nPow = GetParamInt( 1 );
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );

	int nUnits = hTroop->GetUnitCount();
	for(int i=0; i < nUnits; ++i) 
		hTroop->GetUnit(i)->SetAttackPowerAdd(nPow); // 내부적으로 퍼센티지 계산

	return  ACTION_STATE_FINISH;
}

int CFcActTroopIncRange::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	int nInc = GetParamInt( 1 );	
	
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );

	float nRadius = hTroop->GetAttackRadius();
	nRadius += hTroop->GetAttackRadius()*(nInc/100);
	hTroop->SetAttackRadius(nRadius);

	return  ACTION_STATE_FINISH;
}

int CFcActEnableBlizzardEffect::DoCommand()
{
	int nAlpha = GetParamInt( 0 );
	int nIntervalTick = GetParamInt( 1 );
	g_FcWorld.EnableBlizzardEffect(nAlpha, nIntervalTick);
	return  ACTION_STATE_FINISH;
}
int CFcActDisableBlizzardEffect::DoCommand()
{
	g_FcWorld.DisableBlizzardEffect();
	return  ACTION_STATE_FINISH;
}

int CFcActShowMissionSelectMenu::DoCommand()
{	
	g_FCGameData.nSpecialSelStageState = SPECIAL_STAGE_MENU_ON;
	return  ACTION_STATE_FINISH;
}


int CFcActSetPlayerLevelExp::DoCommand()
{
	int nLv = GetParamInt( 0 );
	int nExp = GetParamInt( 1 );

	if(!g_bIsStartTitleMenu)
	{
		HeroObjHandle hHero = g_FcWorld.GetHeroHandle();
		hHero->SetLevel(nLv);
		hHero->SetExp(nExp);
	}
	
	return  ACTION_STATE_FINISH;
}


int CFcStartRealtimeMovie::DoCommand()
{
	char* pAreaName = GetParamStr( 0 );
	AREA_INFO* pInfo = g_FcWorld.GetAreaInfo( pAreaName );

	g_FcWorld.StartRealmovie( pInfo->fSX, pInfo->fEX, pInfo->fSZ, pInfo->fEZ );

	return  ACTION_STATE_FINISH;
}

int CFcActChangeLight::DoCommand() 
{
//	if(!m_bStart) {

//		m_bStart = true;
		int nSec = GetParamInt(0);
		
		D3DXVECTOR4 vDestDiffuse((float)GetParamInt(1), (float)GetParamInt(2),(float)GetParamInt(3), (float)GetParamInt(4)),
			vDestSpecular((float)GetParamInt(5), (float)GetParamInt(6), (float)GetParamInt(7), (float)GetParamInt(8)),
			vDestAmbient((float)GetParamInt(9), (float)GetParamInt(10), (float)GetParamInt(11), (float)GetParamInt(12));	
		vDestDiffuse/=256;
		vDestSpecular/=256;
		vDestAmbient/=256;
		g_FcWorld.GetInstance().ChangeLight(vDestDiffuse, vDestSpecular, vDestAmbient, nSec);
//	}
//	else {
//		if(g_FcWorld.GetInstance().IsChangeLightStop())
//			return ACTION_STATE_FINISH;
//	}
	
//	return ACTION_STATE_WORKING;
	return ACTION_STATE_FINISH; // 바로 다음 액션 실행하게
}

int CFcActChangeFog::DoCommand() 
{
	//if(!m_bStart) {

	//	m_bStart = true;
		int nSec = GetParamInt(0);

		int nNear = GetParamInt(1);
		int nFar = GetParamInt(2);

		D3DXVECTOR4 col((float)GetParamInt(3), (float)GetParamInt(4), (float)GetParamInt(5), (float)GetParamInt(6)); 
		col/=256;

		int nTick = GetParamInt(7);

		g_FcWorld.GetInstance().ChangeFog(col, nNear, nFar, nTick, nSec);
	//}
	//else {
	//	if(g_FcWorld.GetInstance().IsChangeFogStop())
	//		return ACTION_STATE_FINISH;
	//}
	
//	return ACTION_STATE_WORKING;
	return ACTION_STATE_FINISH; // 바로 다음 액션 실행하게
}

int CFcActChangeScene::DoCommand() 
{
	//if(!m_bStart) {

	//	m_bStart = true;
		int nSec = GetParamInt(0);
		float fScene = (float)GetParamInt(1)/100.f,
			fBlur = (float)GetParamInt(2)/100.f,
			fGlow = (float)GetParamInt(3)/100.f;

		g_FcWorld.GetInstance().ChangeScene(fScene, fBlur, fGlow, nSec);
	//}
	//else {
	//	if(g_FcWorld.GetInstance().IsChangeSceneStop())
	//		return ACTION_STATE_FINISH;
	//}
	//


	//return ACTION_STATE_WORKING;
	return ACTION_STATE_FINISH; // 바로 다음 액션 실행하게
}



int CFcActVarClearTime::DoCommand() 
{
	int nVarID = GetParamInt( 0 );
	g_FcWorld.SetTriggerVar( nVarID, g_FcWorld.GetStagePlayTime() );
	return ACTION_STATE_FINISH;
}

int CFcActVarKills::DoCommand() 
{
	int nVarID = GetParamInt( 0 );
	HeroObjHandle Handle = g_FcWorld.GetHeroHandle();
	BsAssert(Handle);	
	g_FcWorld.SetTriggerVar( nVarID, Handle->GetKillCount() );
	return ACTION_STATE_FINISH;
}


int CFcActVarNormalKills::DoCommand() 
{	
	return ACTION_STATE_FINISH;
}


int CFcActVarOrbKills::DoCommand() 
{	
	int nVarID = GetParamInt( 0 );
	HeroObjHandle Handle = g_FcWorld.GetHeroHandle();
	BsAssert(Handle);	
	g_FcWorld.SetTriggerVar( nVarID, Handle->GetMaxOrbKillCount() );

	return ACTION_STATE_FINISH;
}


int CFcActVarPhysicsKills::DoCommand() 
{	
	return ACTION_STATE_FINISH;
}


int CFcActVarHighestCombo::DoCommand() 
{
	int nVarID = GetParamInt( 0 );
	g_FcWorld.SetTriggerVar( nVarID,g_InterfaceManager.GetMaxComboCount() );

	return ACTION_STATE_FINISH;
}


int CFcActVarSum::DoCommand() 
{
	int nDestVarID = GetParamInt( 0 ),
	nVarA = g_FcWorld.GetTriggerVarInt(GetParamInt(1)),
	nVarB = g_FcWorld.GetTriggerVarInt(GetParamInt(2));
	g_FcWorld.SetTriggerVar( nDestVarID, nVarA + nVarB );
	 
	return ACTION_STATE_FINISH;
}


int CFcActVarSub::DoCommand() 
{
	int nDestVarID = GetParamInt( 0 ),
	nVarA = g_FcWorld.GetTriggerVarInt(GetParamInt(1)),
	nVarB = g_FcWorld.GetTriggerVarInt(GetParamInt(2));
	g_FcWorld.SetTriggerVar( nDestVarID, nVarA - nVarB );

	return ACTION_STATE_FINISH;
}


int CFcActVarMul::DoCommand() 
{
	int nDestVarID = GetParamInt( 0 ),
	nVarA = g_FcWorld.GetTriggerVarInt(GetParamInt(1)),
	nVarB = g_FcWorld.GetTriggerVarInt(GetParamInt(2));
	g_FcWorld.SetTriggerVar( nDestVarID, nVarA * nVarB );

	return ACTION_STATE_FINISH;
}


int CFcActVarDiv::DoCommand() 
{
	int nDestVarID = GetParamInt( 0 ),
	nVarA = g_FcWorld.GetTriggerVarInt(GetParamInt(1)),
	nVarB = g_FcWorld.GetTriggerVarInt(GetParamInt(2));
	g_FcWorld.SetTriggerVar( nDestVarID, nVarA / nVarB );

	return ACTION_STATE_FINISH;
}


int CFcActVarMod::DoCommand() 
{
	int nDestVarID = GetParamInt( 0 ),
	nVarA = g_FcWorld.GetTriggerVarInt(GetParamInt(1)),
	nVarB = g_FcWorld.GetTriggerVarInt(GetParamInt(2));
	g_FcWorld.SetTriggerVar( nDestVarID, nVarA % nVarB );

	return ACTION_STATE_FINISH;
}




int CFcActTroopsSetAttackable::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	bool bAttackable = (GetParamInt( 1 ) != 0);
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	hTroop->SetAttackable( bAttackable );
	return ACTION_STATE_FINISH;
}


int CFcActTrueOrbAdd::DoCommand()
{
	int nOrb = GetParamInt( 0 );
	CFcWorld::GetInstance().GetHeroHandle()->AddTrueOrbSpark( nOrb );
	return ACTION_STATE_FINISH;
}


int CFcTroopSetUntouchable::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	bool bUntouchable = (GetParamInt( 1 ) != 0);
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	hTroop->SetUntouchable( bUntouchable );
	return ACTION_STATE_FINISH;
}

int CFcActVarClearTimeScore::DoCommand() 
{
	int nVarID = GetParamInt( 0 );
	int nClearScore = max(0, ( 3600 - max(g_FcWorld.GetStagePlayTime(), 0) )/200)+18;
	if(nClearScore > 36)
		nClearScore = 36;
	g_FcWorld.SetTriggerVar( nVarID, nClearScore );
	return ACTION_STATE_FINISH;
}


const std::pair<int, int> KILLCOUNT_SCORETABLE[] = 
	{ 
		make_pair( 0, 0 ),
		make_pair( 20, 10 ), // 퍼센티지, 스코어
		make_pair( 35, 15 ),
		make_pair( 50, 20 ),
		make_pair( 65, 25 ),
		make_pair( 80, 30 ),
		make_pair( 95, 35 ),
		make_pair( 100, 40 )
	};

int CFcActVarKillsScore::DoCommand() 
{
	int nVarID		= GetParamInt( 0 );
	int nTotalEnemy = max( GetParamInt( 1 ), 1);

	HeroObjHandle Handle = g_FcWorld.GetHeroHandle();
	BsAssert(Handle);	

	int nScore= int(((float)Handle->GetKillCount()/nTotalEnemy)*55);
	//int nScore		= 0;
	
	//for(int i=0; i<8; ++i) {
	//	if( KILLCOUNT_SCORETABLE[i].first >= nKillPercent) {
	//		nScore = KILLCOUNT_SCORETABLE[i].second;
	//		break;
	//	}
	//}
	

	g_FcWorld.SetTriggerVar( nVarID, nScore );
	return ACTION_STATE_FINISH;
}


const std::pair<int, int> ORBKILLCOUNT_SCORETABLE[] = 
	{ 
		make_pair( 0, 0),
		make_pair( 100, 6 ), // 퍼센티지, 스코어
		make_pair( 200, 8 ),
		make_pair( 300, 10 ),
		make_pair( 400, 12 ),
		make_pair( 500, 14 ),
		make_pair( 600, 16 ),
		make_pair( 700, 18 ),
		make_pair( 800, 20 ),
		make_pair( 900, 22 ),
		make_pair( 1000, 24 ),
		make_pair( numeric_limits<int>::max() , 26 ),
	};

int CFcActVarOrbKillsScore::DoCommand() 
{
	int nVarID = GetParamInt( 0 );	

	HeroObjHandle Handle = g_FcWorld.GetHeroHandle();
	BsAssert(Handle);	

	int nOrbKillCount = Handle->GetMaxOrbKillCount();
	int nScore = 0;

	for(int i=0; i<12; ++i) {
		if( ORBKILLCOUNT_SCORETABLE[i].first >= nOrbKillCount) {
			nScore = ORBKILLCOUNT_SCORETABLE[i].second;			
			break;
		}
	}
	g_FcWorld.SetTriggerVar( nVarID, nScore );
	return ACTION_STATE_FINISH;	
}


const std::pair<int, int> MAXCOMBO_SCORETABLE[] = 
	{ 
		make_pair( 0, 0),
		make_pair( 100, 3 ), // 퍼센티지, 스코어
		make_pair( 200, 6 ),
		make_pair( 500, 9 ),
		make_pair( 1000, 10 ),
		make_pair( 1500, 11 ),
		make_pair( 2000, 12 ),
		make_pair( 2500, 13 ),
		make_pair( 3000, 14 ),
		make_pair( 3500, 15 ),
		make_pair( 4000, 16 ),
		make_pair( 4500, 17 ),
		make_pair( 5000, 18 ),
		make_pair( 5500, 19 ),
		make_pair( numeric_limits<int>::max() , 20 ),
	};

int CFcActVarHighestComboScore::DoCommand() 
{
	int nVarID = GetParamInt( 0 );	

	int nHighestCombo = g_InterfaceManager.GetMaxComboCount();
	int nScore = 0;

	for(int i=0; i<15; ++i) {
		if( MAXCOMBO_SCORETABLE[i].first >= nHighestCombo) {
			nScore = MAXCOMBO_SCORETABLE[i].second;			
			break;
		}
	}
	g_FcWorld.SetTriggerVar( nVarID, nScore );
	return ACTION_STATE_FINISH;	
}


const std::pair<int, int> SURVIVERATE_SCORETABLE[] = 
	{ 
		make_pair( 0, 7),
		make_pair( 10, 7 ), // 퍼센티지, 스코어
		make_pair( 20, 7 ),
		make_pair( 30, 7 ),
		make_pair( 40, 7 ),
		make_pair( 50, 7 ),
		make_pair( 60, 7 ),
		make_pair( 70, 7 ),
		make_pair( 80, 8 ),
		make_pair( 90, 9 ),
		make_pair( 100 , 10 ),
	};

int CFcActVarSurviveRateScore::DoCommand() 
{
	int nVarID		= GetParamInt( 0 );
	int nTotalFriendly = max( GetParamInt( 1 ), 1);
	int nSurvivor = 0;

	CFcTroopManager* pTroopManager = g_FcWorld.GetTroopmanager();
	int nCnt = pTroopManager->GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hHandle = pTroopManager->GetTroop( i );
		if( hHandle->GetTeam()!=0)
			continue;
        nSurvivor+=hHandle->GetAliveUnitCount();	
	}
	if(nSurvivor > nTotalFriendly)
		nSurvivor = nTotalFriendly;

	float fKillPercent= ((float)nSurvivor/nTotalFriendly)*100;
	int nScore		= 0;
	
	for(int i=0; i<11; ++i) {
		if( SURVIVERATE_SCORETABLE[i].first >= fKillPercent) {
			nScore = SURVIVERATE_SCORETABLE[i].second;
			break;
		}
	}

	g_FcWorld.SetTriggerVar( nVarID, nScore );
	return ACTION_STATE_FINISH;	
}


const std::pair<int, int> ITEM_SCORETABLE[] = 
	{ 
		make_pair( 0, 0),
		make_pair( 5, 1 ), // 퍼센티지, 스코어
		make_pair( 10, 2 ),
		make_pair( 20, 3 ),
		make_pair( 30, 4 ),
		make_pair( numeric_limits<int>::max() , 5 ),
	};

int CFcActVarItemScore::DoCommand() 
{
	int nVarID = GetParamInt( 0 );	

	int nItems = g_FcItemManager.GetItemCount();
	int nScore = 0;

	for(int i=0; i<6; ++i) {
		if( ITEM_SCORETABLE[i].first >= nItems) {
			nScore = ITEM_SCORETABLE[i].second;			
			break;
		}
	}
	g_FcWorld.SetTriggerVar( nVarID, nScore );
	

	return ACTION_STATE_FINISH;
}

int CFcSetAdjutantType::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	int nType = GetParamInt( 1 );

	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	if( !hTroop ) return ACTION_STATE_FINISH;
	int nCnt = hTroop->GetUnitCount();
	for( int i=0; i<nCnt; i++ )
	{
		GameObjHandle hUnit = hTroop->GetUnit( i );
		if(!hUnit)
			continue;
		switch( hUnit->GetClassID() ) {
			case CFcGameObject::Class_ID_Adjutant:
				((CFcAdjutantObject*)hUnit.m_pInstance)->SetDieType(  nType  );
				break;
			case CFcGameObject::Class_ID_Hero:
				((CFcHeroObject*)hUnit.m_pInstance)->SetDieType(  nType  );
				break;
		}
	}
	return ACTION_STATE_FINISH;
}


int CFcActAddTroopEventArea::DoCommand()
{
	char *pAreaName = GetParamStr( 0 );
	AREA_INFO* pInfo = g_FcWorld.GetAreaInfo( pAreaName );

	g_FcWorld.AddTroopEventArea( pInfo );
	return ACTION_STATE_FINISH;
}

int CFcActSetTroopEventArea::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	float fDist = (float) GetParamInt( 1 );
	pTroopName = GetParamStr( 2 );
	TroopObjHandle hTargetTroop = g_FcWorld.GetTroopObject( pTroopName );


	g_FcWorld.SetTroopEventArea( hTroop, hTargetTroop, fDist );
	return ACTION_STATE_FINISH;
}

int CFcForceInvulnerable::DoCommand()
{
	int nTeam = GetParamInt( 0 );
	int nForce = GetParamInt( 1 );

	int nCnt = g_FcWorld.GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTroop = g_FcWorld.GetTroopObject( i );

		if( !hTroop->IsEnable() )
			continue;

		if( !hTroop->IsEliminated() )
			continue;

		if( hTroop->GetTeam() != nTeam || hTroop->GetForce() != nForce )
			continue;

		hTroop->CmdInvulnerable( true );
	}
	return ACTION_STATE_FINISH;
}

int CFcForceVulnerable::DoCommand()
{
	int nTeam = GetParamInt( 0 );
	int nForce = GetParamInt( 1 );

	int nCnt = g_FcWorld.GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTroop = g_FcWorld.GetTroopObject( i );

		if( !hTroop->IsEnable() )
			continue;

		if( !hTroop->IsEliminated() )
			continue;

		if( hTroop->GetTeam() != nTeam || hTroop->GetForce() != nForce )
			continue;

		hTroop->CmdInvulnerable( false );
	}
	return ACTION_STATE_FINISH;

}


int CFcActResultRank::DoCommand()
{
	int nRank = GetParamInt( 0 );

	if( g_FCGameData.bCheatRank == false )
	{
		// S 랭크 예외 처리
		if( nRank == 0 )
		{
			int nHeroID = g_MissionToSoxID[g_FCGameData.nPlayerType];
			HeroRecordInfo* pHeroRecordInfo = g_FCGameData.tempFixedSave.GetHeroRecordInfo(nHeroID);
			StageResultInfo* pStageResultInfo = NULL;
			if(pHeroRecordInfo)
				pStageResultInfo = pHeroRecordInfo->GetStageResultInfo(g_FCGameData.SelStageId);

			if( pStageResultInfo )
			{
				if( pStageResultInfo->nEnemyLevelUp < 0 )
					nRank = 1;
			}
			else
			{
				if( g_FCGameData.stageInfo.nEnemyLevelUp < 0 )
					nRank = 1;
			}
		}
		g_FCGameData.stageInfo.nPlayGrade = nRank;
	}
	return ACTION_STATE_FINISH;
}


int CFcActRankPrize::DoCommand()
{
	int nPrize = GetParamInt( 0 );
	if( nPrize >= 0 && nPrize < ITEM_TYPE_LAST )
		g_FCGameData.stageInfo.nSpecialItem[PRIZE_ITEM_RANKLESS] = nPrize;
	else
		DebugString( "CFcActRankPrize()에서 없는 아이템을 보상으로 주려고 합니다! %d\n", nPrize );

	return ACTION_STATE_FINISH;
}

int CFcActResultBonus::DoCommand()
{
	int nBonus = g_FcWorld.GetTriggerVarInt(GetParamInt(0));;		
	g_FCGameData.stageInfo.nBonusPoint = nBonus;
	return ACTION_STATE_FINISH;
}

int CFcActResultRankTable::DoCommand()
{
	int nScore = g_FcWorld.GetTriggerVarInt(GetParamInt(0));
	if( g_FCGameData.bCheatRank == false )
	{
		g_FCGameData.stageInfo.nPlayGrade = 0;	

		for(int i=1; i<6; ++i) {
			int nBound = GetParamInt( i );
			if(nBound >= nScore)	{
				g_FCGameData.stageInfo.nPlayGrade = 6-i;
				break;
			}
		}

		// S 랭크 예외 처리
		if( g_FCGameData.stageInfo.nPlayGrade == 0 )
		{
			int nHeroID = g_MissionToSoxID[g_FCGameData.nPlayerType];
			HeroRecordInfo* pHeroRecordInfo = g_FCGameData.tempFixedSave.GetHeroRecordInfo(nHeroID);
			StageResultInfo* pStageResultInfo = NULL;
			if(pHeroRecordInfo)
				pStageResultInfo = pHeroRecordInfo->GetStageResultInfo(g_FCGameData.SelStageId);

			if( pStageResultInfo )
			{
				if( pStageResultInfo->nEnemyLevelUp < 0 )
					g_FCGameData.stageInfo.nPlayGrade = 1;
			}
		}
		else
		{
			if( g_FCGameData.stageInfo.nEnemyLevelUp < 0 )
				g_FCGameData.stageInfo.nPlayGrade = 1;
		}
	}
	return ACTION_STATE_FINISH;
}


int CFcActCatapultAttackWall::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	if( hTroop->GetType() != TROOPTYPE_SIEGE )
	{
		DebugString( "Invalid set CFcActCatapultAttackWall%s \n", pTroopName );
		BsAssert( 0 && "Invalid set CFcActCatapultAttackWall" );
	}
	char* pPropName = GetParamStr(1);
	CFcProp* pProp = g_FcWorld.GetProp( pPropName );
	BsAssert( pProp && pProp->IsWallProp() ); //aleksger: prefix bug : GetProp() may return NULL.

	// 더미 부대 이름이 프랍 이름과 똑같음
	TroopObjHandle hTargetTroop = g_FcWorld.GetTroopObject( pPropName );
	hTroop->CmdAttack( hTargetTroop );

	return ACTION_STATE_FINISH;
}

int CFcActShowWallHPGuage::DoCommand()
{
	char* pPropName = GetParamStr( 0 );
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pPropName );	// 똑같은 이름의 부대가 생긴다.
	if( hTroop->GetType() != TROOPTYPE_WALL_DUMMY )
	{
		DebugString( "Invalid set CFcActShowWallHPGuage %s \n", pPropName );
		BsAssert( 0 && "Invalid set CFcActShowWallHPGuage" );
	}
	int nTextID = GetParamInt( 1 );
	g_InterfaceManager.SetTroopGauge( hTroop, nTextID );

	return ACTION_STATE_FINISH;
}


int CFcActSetTrueOrbEnable::DoCommand()
{
	bool bEnable = (GetParamInt(0) == 1);		

	if(false == bEnable)
		g_FcWorld.GetHeroHandle()->SetTrueOrbSpark(0);
	g_FcWorld.GetHeroHandle()->EnableTrueOrbSpark(bEnable);
	return ACTION_STATE_FINISH;
}


int CFcActShowTroopGauge::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	int nTextID = GetParamInt( 1 );
	g_InterfaceManager.SetTroopGauge( hTroop, nTextID );

	return ACTION_STATE_FINISH;
}

int CFcActHideTroopGauge::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	g_InterfaceManager.RemoveTroopGauge( hTroop );

	return ACTION_STATE_FINISH;
}


int CFcActVarTotalUnits::DoCommand()
{
	int nVarID = GetParamInt( 0 );
	int nForce = GetParamInt( 1 );
	int nTeam = GetParamInt( 2 );

	int nTotalUnits = 0;
	
	CFcTroopManager* pTroopManager = g_FcWorld.GetTroopmanager();
	int nCnt = pTroopManager->GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hHandle = pTroopManager->GetTroop( i );
		if(!hHandle)
			continue;

		if(hHandle->GetForce() == nForce && hHandle->GetTeam() == nTeam)
			nTotalUnits += hHandle->GetUnitCount();		
	}
	
	g_FcWorld.SetTriggerVar( nVarID, nTotalUnits );

	return ACTION_STATE_FINISH;
}

int CFcActVarOrbSparkOn::DoCommand()
{
	int nVarID = GetParamInt( 0 );

	int nOrbSparkOn = CFcWorld::GetInstance().GetHeroHandle()->GetNumOrbSparkOn();

	g_FcWorld.SetTriggerVar( nVarID, nOrbSparkOn);

	return ACTION_STATE_FINISH;
}

int CFcActPropRegen::DoCommand()
{	
	char* pPropName = GetParamStr( 0 );	
	CFcProp * pBreakable = g_FcWorld.GetProp( pPropName );	

	// ToDo: 

	return ACTION_STATE_FINISH;
}

int CFcActTroopIncPowerSox::DoCommand()
{	
	int nForce = GetParamInt(0);
	int nTeam = GetParamInt(1);
	int nPercent = max(GetParamInt(2), 1);

	CFcTroopManager* pTroopManager = g_FcWorld.GetTroopmanager();
	int nCnt = pTroopManager->GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hHandle = pTroopManager->GetTroop( i );
		if(!hHandle)
			continue;
		if(hHandle->GetForce() == nForce && hHandle->GetTeam() == nTeam)
		{
			if(!hHandle->IsEnable()) {
				hHandle->SetAdditionalAttackPower(-100+nPercent);
				continue;
			}

			int nUnits = hHandle->GetUnitCount();
			for(int j=0; j < nUnits; ++j) {
				if(!hHandle->GetUnit(j))
					continue;
				hHandle->GetUnit(j)->SetAttackPowerAdd(-100+nPercent); // 내부적으로 퍼센티지 계산
			}
		}
	}

	return ACTION_STATE_FINISH;
}


int CFcActTroopIncRangeSox::DoCommand()
{	
	int nForce = GetParamInt(0);
	int nTeam = GetParamInt(1);
	int nPercent = max(GetParamInt(2), 1);

	CFcTroopManager* pTroopManager = g_FcWorld.GetTroopmanager();
	int nCnt = pTroopManager->GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hHandle = pTroopManager->GetTroop( i );
		if(!hHandle)
			continue;
		if(hHandle->GetForce() == nForce && hHandle->GetTeam() == nTeam)
		{
			float fRange = (float)g_TroopSOX.GetRange( hHandle->GetType() , hHandle->GetLevel() );
			hHandle->SetAttackRadius(fRange*(float(nPercent)/100.f) );
		}
	}

	return ACTION_STATE_FINISH;
}

int CFcActTroopIncPowerSoxID::DoCommand()
{	
	char* pTroopName = GetParamStr( 0 );
	int nPercent = max(GetParamInt(1), 1);

	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	if(!hTroop->IsEnable()) {
		hTroop->SetAdditionalAttackPower(-100+nPercent);
		return ACTION_STATE_FINISH;
	}
	
	int nUnits = hTroop->GetUnitCount();
	for(int i=0; i < nUnits; ++i) {
		if(!hTroop->GetUnit(i))
					continue;
		hTroop->GetUnit(i)->SetAttackPowerAdd(-100+nPercent);
	}

	return ACTION_STATE_FINISH;
}

int CFcActTroopIncRangeSoxID::DoCommand()
{	
	char* pTroopName = GetParamStr( 0 );
	int nPercent = max(GetParamInt(1), 1);

	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	float fRange = (float)g_TroopSOX.GetRange( hTroop->GetType() , hTroop->GetLevel() );
	hTroop->SetAttackRadius(fRange*(float(nPercent)/100.f));

	return ACTION_STATE_FINISH;
}


int CFcActVarTotalUnitsTeam::DoCommand()
{
	int nVarID = GetParamInt( 0 );	
	int nTeam = GetParamInt( 1 );

	int nTotalUnits = 0;
	
	CFcTroopManager* pTroopManager = g_FcWorld.GetTroopmanager();
	int nCnt = pTroopManager->GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hHandle = pTroopManager->GetTroop( i );
		if(hHandle->GetTeam() == nTeam)
			nTotalUnits += hHandle->GetUnitCount();		
	}
	
	g_FcWorld.SetTriggerVar( nVarID, nTotalUnits );

	return ACTION_STATE_FINISH;
}


int CFcActVarTroopHP::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	int nVarID = GetParamInt( 1 );
		
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	
	g_FcWorld.SetTriggerVar( nVarID, hTroop->GetHP() );

	return ACTION_STATE_FINISH;
}


// 사용안함
int CFcActDarkDynamicFogEnable::DoCommand()
{
	char *pAreaName1 = GetParamStr(0);
	AREA_INFO *pAreaInfo = g_FcWorld.GetAreaInfo( pAreaName1 );
	D3DXVECTOR2	AreaSize = D3DXVECTOR2(pAreaInfo->GetWidth(), pAreaInfo->GetHeight());
	D3DXVECTOR2	StartPoint = D3DXVECTOR2(pAreaInfo->fSX, pAreaInfo->fSZ);

	char szFile[128];
	sprintf( szFile, "%s", "cloud_011dark.dds" ); // 칠흑의 어둠포그 : cloud_011dark //  일반포그 : cloud_t2(2)

	if (g_FcWorld.GetFXDynamicFogID() != -1) {
		g_pFcFXManager->SendMessage(g_FcWorld.GetFXDarkDynamicFogID(), FX_INIT_OBJECT, (DWORD)&AreaSize, (DWORD)&StartPoint , (DWORD)szFile);
		g_pFcFXManager->SendMessage(g_FcWorld.GetFXDynamicFogID(),FX_PLAY_OBJECT, 10);
	}

	return  ACTION_STATE_FINISH;
}

// "FXID[string]을 부대[TroopID]의 유닛에 [int], [int], [int] 좌표에 [int](0:한번,1:반복), [int](0:제자리,1:유닛따라다님)으로 처리한다."
int CFcActSetTroopFX::DoCommand()
{
	char* pFXFileStr = GetParamStr( 0 );
	char* pTroopName = GetParamStr( 1 );
//	D3XVECTOR3 OffsetPos;
//	OffsetPos.x = (int)GetParamInt( 2 );
//	OffsetPos.y = (int)GetParamInt( 3 );
//	OffsetPos.z = (int)GetParamInt( 4 );

//	bool bLoop = ( GetParamInt( 5 ) != 0 );
//	bool bAttachUnit = ( GetParamInt( 6 ) != 0 );

	int nFXID = -1;
	g_BsKernel.chdir("fx");
	if( pFXFileStr )
	{
		// philt: this potentially leaks one of the FX templates.

		char szFxFilePath[256];
		sprintf(szFxFilePath,"%s%s",g_BsKernel.GetCurrentDirectory(), pFXFileStr );
		nFXID = g_BsKernel.LoadFXTemplate(-1, szFxFilePath);
	}
	g_BsKernel.chdir("..");

	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	int nCnt = hTroop->GetUnitCount();
	for( int i=0; i<nCnt; i++ )
	{
		GameObjHandle hUnit = hTroop->GetUnit( i );
		if( nFXID != -1 )
		{
			g_FcWorld.PlaySimpleLinkedFx( nFXID, hUnit, 0 );
		}
	}
	return  ACTION_STATE_FINISH;
}												 

int CFcActChangeSkyBox::DoCommand()
{
	char* szSkyBoxName = GetParamStr( 0 );

	// 기존 스카이 박스 삭제
	if(CBsSkyBoxObject::s_nKernelIndex != -1) {
		g_BsKernel.DeleteObject(CBsSkyBoxObject::s_nKernelIndex);
		CBsSkyBoxObject::s_nKernelIndex = -1;
	}

	// 새 스카이 박스 추가
	int nSkySkin;
	BaseObjHandle SkyHandle;
	CCrossVector Cross;

	CBsKernel::GetInstance().chdir("sky");
	nSkySkin = g_FcWorld.LoadMeshData( -1, szSkyBoxName );
	CBsKernel::GetInstance().chdir("..");

	BsAssert( nSkySkin >= 0 && "Cannot load Skybox" );

	Cross.SetPosition(0.0f, 0.0f, 0.0f);
	SkyHandle=CFcBaseObject::CreateObject<CFcBaseObject>(&Cross);
	SkyHandle->Initialize(nSkySkin, -1, CFcBaseObject::SKYBOX);

	return ACTION_STATE_FINISH;
}


int CFcActSetRankPrizeS::DoCommand()
{
	if(g_FCGameData.stageInfo.nPlayGrade!=0)
		return ACTION_STATE_FINISH;

	int nPrize = GetParamInt( 0 );

	if( nPrize >= 0 && nPrize < ITEM_TYPE_LAST )
		g_FCGameData.stageInfo.nSpecialItem[PRIZE_ITEM_RANK_S] = nPrize;
	else
		DebugString( "CFcActSetRankPrizeS()에서 없는 아이템을 보상으로 주려고 합니다! %d\n", nPrize );

	return ACTION_STATE_FINISH;
}


int CFcActSetRankPrizeA::DoCommand()
{
	if(g_FCGameData.stageInfo.nPlayGrade!=1)
		return ACTION_STATE_FINISH;

	int nPrize = GetParamInt( 0 );
	if( nPrize >= 0 && nPrize < ITEM_TYPE_LAST )
		g_FCGameData.stageInfo.nSpecialItem[PRIZE_ITEM_RANK_A] = nPrize;
	else
		DebugString( "CFcActSetRankPrizeA()에서 없는 아이템을 보상으로 주려고 합니다! %d\n", nPrize );
	
	return ACTION_STATE_FINISH;
}

int CFcActGuardianPlace::DoCommand() 
{
	
	int nIndex = GetParamInt( 0 );
	char *pAreaName = GetParamStr( 1 );
	int nDir = GetParamInt( 2 );
	
	TroopObjHandle hTroop = g_FcWorld.GetTroopmanager()->GetGuardianTroop(nIndex);
	if(!hTroop)
		return ACTION_STATE_FINISH;

	AREA_INFO* pInfo = g_FcWorld.GetAreaInfo( pAreaName );

	float fCX = ( pInfo->fEX - pInfo->fSX ) / 2;
	float fCZ = ( pInfo->fEZ - pInfo->fSZ ) / 2;
	float fX = pInfo->fSX + fCX;	
	float fZ = pInfo->fSZ + fCZ;

	hTroop->CmdPlace( fX, fZ, nDir );

	return ACTION_STATE_FINISH;
}

int CFcActSetItemIndex::DoCommand()
{
	char* pPropName = GetParamStr(0);
	int nItemIndex = GetParamInt(1);

	CFcProp* pProp = g_FcWorld.GetProp( pPropName );
	if(!pProp || !pProp->IsItemProp())
		return ACTION_STATE_FINISH;
	CFcItemProp* pItem = (CFcItemProp*)pProp;    
	pItem->SetItemIndex(nItemIndex);

	return ACTION_STATE_FINISH;	
}


int CFcActVarGuardianKillCount::DoCommand() 
{
	int nVarID = GetParamInt( 0 );
	g_FcWorld.SetTriggerVar( nVarID, g_FcWorld.GetGuardianKillCount() );
	return ACTION_STATE_FINISH;
}

int CFcActVarGuardianOrb::DoCommand() 
{
	int nVarID = GetParamInt( 0 );
	g_FcWorld.SetTriggerVar( nVarID, g_FcWorld.GetGuardianOrb() );
	return ACTION_STATE_FINISH;
}


int CFcActHideWallHPGuage::DoCommand()
{
	char* pPropName = GetParamStr( 0 );
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pPropName );	// 똑같은 이름의 부대가 생긴다.
	if( hTroop->GetType() != TROOPTYPE_WALL_DUMMY )
	{
		DebugString( "Invalid set CFcActShowWallHPGuage %s \n", pPropName );
		BsAssert( 0 && "Invalid set CFcActShowWallHPGuage" );
	}
	g_InterfaceManager.RemoveTroopGauge( hTroop );
	return ACTION_STATE_FINISH;
}


int CFcActDarkDynamicFogEnableEx::DoCommand()
{
	int sx = GetParamInt(0);
	int sy = GetParamInt(1);
	int width = GetParamInt(2);
	int height = GetParamInt(3);
	int index = GetParamInt(4);
		
	D3DXVECTOR2	AreaSize = D3DXVECTOR2(float(width), float(height));
	D3DXVECTOR2	StartPoint = D3DXVECTOR2(float(sx), float(sy));

	char szFile[128];
	sprintf( szFile, "%s", "cloud_011dark.dds" ); // 칠흑의 어둠포그 : cloud_011dark //  일반포그 : cloud_t2(2)

	int iDynamicFogID = g_pFcFXManager->Create(FX_TYPE_DYNAMICFOG);

	if (iDynamicFogID != -1) {
		g_pFcFXManager->SendMessage(iDynamicFogID, FX_INIT_OBJECT, (DWORD)&AreaSize, (DWORD)&StartPoint , (DWORD)szFile);
		g_pFcFXManager->SendMessage(iDynamicFogID,FX_PLAY_OBJECT, 10);
		g_FcWorld.AddDarkDynamicFog(index, iDynamicFogID);
	}

	return  ACTION_STATE_FINISH;
}

int CFcActDarkDynamicFogDisableEx::DoCommand()
{
	int index = GetParamInt(0);
	
	int iDynamicFogID = g_FcWorld.FindDarkDynamicFog(index);

	if (iDynamicFogID != -1) {
		//g_pFcFXManager->SendMessage(iDynamicFogID, FX_DELETE_OBJECT);
		SAFE_DELETE_FX(iDynamicFogID,FX_TYPE_DYNAMICFOG);
		g_FcWorld.EraseDarkDyanmicFog(index);
	}

	return  ACTION_STATE_FINISH;
}


int CFcActSetItemBoxEnable::DoCommand()
{
	char* pPropName = GetParamStr(0);
	
	CFcProp* pProp = g_FcWorld.GetProp( pPropName );
	if(!pProp || !pProp->IsItemProp())
		return ACTION_STATE_FINISH;
	CFcItemProp* pItem = (CFcItemProp*)pProp;    
	pItem->SetEnable();

	return ACTION_STATE_FINISH;	
}


int CFcActSetItemBoxDisable::DoCommand()
{
	char* pPropName = GetParamStr(0);
	
	CFcProp* pProp = g_FcWorld.GetProp( pPropName );
	if(!pProp || !pProp->IsItemProp())
		return ACTION_STATE_FINISH;
	CFcItemProp* pItem = (CFcItemProp*)pProp;    
	pItem->SetDisable();
	
	return ACTION_STATE_FINISH;	
}


struct SRainParam
{
	int		_nCount;			// 정의된 틱당 나타날 빗방울 수
	int		_nTotalCount;		// 한화면에 나타날 빗방울의 총 갯수
	int		_nTick;				// 빗방울이 나타날 틱
	int		_nTotalTick;		// 빗방울이 나타날 전체 틱
	bool	_bLiner;			// 보간될 것인지를 나타낼 변수.	
};

int CFcActEffectRainEnable::DoCommand() 
{
	SRainParam tmp;
	tmp._nCount = GetParamInt(0);
	tmp._nTotalCount = GetParamInt(1);
	tmp._nTick = GetParamInt(2);	
	tmp._nTotalTick = GetParamInt(3);
	tmp._bLiner = (GetParamInt(4)==1);
	
	
	int id = g_pFcFXManager->Create(FX_TYPE_RAINS);
	g_pFcFXManager->SendMessage(id,FX_INIT_OBJECT, (DWORD)&tmp);
	g_pFcFXManager->SendMessage(id,FX_PLAY_OBJECT);

	g_FcWorld.SetFXRainID(id);

	return ACTION_STATE_FINISH;
}

int CFcActEffectRainDisable::DoCommand() 
{
	if(g_FcWorld.GetFXRainID()!=-1)
	{
		g_pFcFXManager->SendMessage(g_FcWorld.GetFXRainID(),FX_DELETE_OBJECT);	
		g_FcWorld.SetFXRainID(-1);
	}


	return ACTION_STATE_FINISH;
}


int CFcActSkipText::DoCommand()
{
	return ACTION_STATE_FINISH;
}


int CFcActSkipRealMovie::DoCommand()
{
	g_pFcRealMovie->TerminateRealMove();
	return ACTION_STATE_FINISH;
}

int CFcActDelayOnRealMovie::DoCommand()
{
	if( g_pFcRealMovie->IsPlay() == false && g_pFcRealMovie->IsSetDirectMsg() == false )
	{
		return ACTION_STATE_FINISH;
	}
	return ACTION_STATE_WORKING;
}

// 첫번째 단 한 유닛만 바꾼다.
int CFcActSetWeaponFromTroop::DoCommand()
{
	char* pTargetTroopName = GetParamStr( 0 );
	TroopObjHandle hTarget = g_FcWorld.GetTroopObject( pTargetTroopName );

	char* pDesTroopName = GetParamStr( 1 );
	TroopObjHandle hDest = g_FcWorld.GetTroopObject( pDesTroopName );

	GameObjHandle hTargetUnit = hTarget->GetFirstLiveUnit();
	GameObjHandle hDestUnit = hDest->GetFirstLiveUnit();

	if( hTargetUnit == NULL || hDestUnit == NULL )
	{
		return ACTION_STATE_FINISH;
	}

	//if( hDestUnit->IsDie() )
	//{
	//	return ACTION_STATE_FINISH;
	//}

	std::vector<WEAPON_OBJECT>* pTargetList = hTargetUnit->GetWeaponList();
	std::vector<WEAPON_OBJECT>* pDestList = hDestUnit->GetWeaponList();

	int nTargetCnt = pTargetList->size();
	int nDestCnt = pDestList->size();

	if( nTargetCnt != nDestCnt )
	{
		BsAssert( 0 && "무기 타입이 다릅니다" );
		return ACTION_STATE_FINISH;
	}

	for( int i=0; i<nTargetCnt; i++ )
	{
		(*pTargetList)[i].bShow = (*pDestList)[i].bShow;
		if( (*pTargetList)[i].nEngineIndex == (*pDestList)[i].nEngineIndex ) {
			if( (*pTargetList)[i].nObjectIndex != -1 ) {
				g_BsKernel.ShowObject( (*pTargetList)[i].nObjectIndex, (*pTargetList)[i].bShow == TRUE );
			}
			continue;
		}
		(*pTargetList)[i].nEngineIndex = (*pDestList)[i].nEngineIndex;

		int nObjectIndex = g_BsKernel.CreateStaticObjectFromSkin( (*pDestList)[i].nEngineIndex );
		g_BsKernel.SendMessage( hTargetUnit->GetEngineIndex(), BS_UNLINKOBJECT_BONE, (*pTargetList)[i].nLinkBoneIndex );
		g_BsKernel.DeleteObject( (*pTargetList)[i].nObjectIndex );
		(*pTargetList)[i].nObjectIndex = -1;

		g_BsKernel.SendMessage( hTargetUnit->GetEngineIndex(), BS_LINKOBJECT, 
			( DWORD )(*pTargetList)[ i ].nLinkBoneIndex, nObjectIndex );
		g_BsKernel.ShowObject( nObjectIndex, (*pTargetList)[i].bShow == TRUE );
		(*pTargetList)[i].nObjectIndex = nObjectIndex;
	}
	hTargetUnit->SetCurrentWeaponIndex( hDestUnit->GetCurrentWeaponIndex() );
	return ACTION_STATE_FINISH;
}


int CFcSetTroopAIDisableInAreaForRM::DoCommand()
{
	char *pAreaName = GetParamStr( 0 );
	AREA_INFO* pInfo = g_FcWorld.GetAreaInfo( pAreaName );

	g_FcWorld.StartTroopAIDisableForRealmovie( pInfo->fSX, pInfo->fEX, pInfo->fSZ, pInfo->fEZ );

	return ACTION_STATE_FINISH;
}

int CFcActVarScoreTroopHP::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	int	nMaxScore = GetParamInt( 1 );
	int nVarID = GetParamInt( 2 );
	
	TroopObjHandle hTarget = g_FcWorld.GetTroopObject( pTroopName );	
	if(!hTarget)
		return ACTION_STATE_FINISH;	
	int nHP = hTarget->GetHP();
	int nMaxHP = hTarget->GetMaxHP();

	if(GameObjHandle hLeader = hTarget->GetLeader())
	{
		nHP += hLeader->GetHP();
		nMaxHP += hLeader->GetMaxHP();
	}

	int score = int((nHP/float(nMaxHP))*nMaxScore);

	g_FcWorld.SetTriggerVar( nVarID, score );
	
	return ACTION_STATE_FINISH;
}
int CFcActVarScoreTroopTeamHP::DoCommand()
{
	int nTeam = GetParamInt( 0 );
	int	nMaxScore = GetParamInt( 1 );
	int nVarID = GetParamInt( 2 );
	
	int nTotalMaxHP = 0;
	int nTotalHP = 0;
 
	int nCnt = g_FcWorld.GetTroopmanager()->GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTroop = g_FcWorld.GetTroopmanager()->GetTroop( i );
			
		if(!hTroop) 
			continue; 
		if(hTroop->GetTeam() == nTeam) {			
			nTotalMaxHP += hTroop->GetMaxHP();
			nTotalHP += hTroop->GetHP();

			if(GameObjHandle hLeader = hTroop->GetLeader())
			{
				nTotalHP += hLeader->GetHP();
				nTotalMaxHP += hLeader->GetMaxHP();
			}
		}
	}
	int score = int((nTotalHP/float(nTotalMaxHP))*nMaxScore);

	g_FcWorld.SetTriggerVar( nVarID, score );
		
	return ACTION_STATE_FINISH;
}
int CFcActVarScoreTroopNum::DoCommand()
{
	int nTeam = GetParamInt( 0 );
	int	nMaxScore = GetParamInt( 1 );
	int nVarID = GetParamInt( 2 );
	
	int nMaxTroop = 0;
	int nTotalAlive = 0;


	int nCnt = g_FcWorld.GetTroopmanager()->GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTroop = g_FcWorld.GetTroopmanager()->GetTroop( i );
			
		if(!hTroop) 
			continue; 
		if(hTroop->GetTeam() == nTeam) {			
			++nMaxTroop;
			if(hTroop->GetHP() > 0)
				++nTotalAlive;
		}
	}
	int score = int((nTotalAlive/float(nMaxTroop))*nMaxScore);

	g_FcWorld.SetTriggerVar( nVarID, score );
	
	return ACTION_STATE_FINISH;
}
int CFcActVarScoreRange::DoCommand()
{
	int nSrcVarID = GetParamInt( 0 );
	int nMaxVar = GetParamInt( 1 );
	int	nMaxScore = GetParamInt( 2 );
	int nDestVarID = GetParamInt( 3 );

	int nSrcVar = g_FcWorld.GetTriggerVarInt(nSrcVarID);
	
	int score = int((nSrcVar/float(nMaxVar))*nMaxScore);

	g_FcWorld.SetTriggerVar( nDestVarID, score );
	
	return ACTION_STATE_FINISH;
}

int CFcActVarScoreBrokenProp::DoCommand()
{
	int nMaxProp = GetParamInt( 0 );	
	int	nMaxScore = GetParamInt( 1 );
	int nVarID = GetParamInt( 2 );

	int nBrokenProp = 0;

	CFcPropManager *pPropMng = g_FcWorld.GetPropManager();	

	int nCnt = pPropMng->GetActivePropCount();
	for(int i=0; i<nCnt; ++i) {
		CFcProp* pProp = pPropMng->GetActiveProp(i);
		if(pProp && pProp->IsCrushProp())	{
			CFcBreakableProp* pBreakable = (CFcBreakableProp*) pProp;
			if(pBreakable->IsBreak())
				++nBrokenProp;
		}
	}

	int score = int((nBrokenProp/float(nMaxProp))*nMaxScore);

	g_FcWorld.SetTriggerVar( nVarID, score );
	
	return ACTION_STATE_FINISH;
}


int CFcActSetPropCrush::DoCommand()
{
	char* pPropName = GetParamStr(0);
	bool bCrush = (GetParamInt(1) != 0);
	CFcProp* pProp = g_FcWorld.GetProp( pPropName );

	if( pProp->IsCrushProp() )
	{
		CFcBreakableProp* pBreakableProp = (CFcBreakableProp*)pProp;
		pBreakableProp->SetEnableCrush( bCrush );
	}
	return ACTION_STATE_FINISH;
}


int CFcActSetCorpseDelay::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	int	nDelayTick = GetParamInt( 1 ) * FRAME_PER_SEC;

	TroopObjHandle hTarget = g_FcWorld.GetTroopObject( pTroopName );

	int nCnt = hTarget->GetUnitCount();
	for( int i=0; i<nCnt; i++ )
	{
		GameObjHandle hUnit = hTarget->GetUnit( i );

		if( hUnit )
		{
			hUnit->SetHP( 0 );
			hUnit->ChangeAnimation( ANI_TYPE_DIE, 0 );
			hUnit->RotateYaw( Random( 1024 ) );
			hUnit->SetDieFrame( DIE_KEEP_FRAME - nDelayTick );
		}
	}
	return ACTION_STATE_FINISH;
}


int CFcActPlaySoundEffect::DoCommand()
{
	char* pSoundName = GetParamStr( 0 );
	g_pSoundManager->PlaySystemSound( SB_COMMON, pSoundName );
	return ACTION_STATE_FINISH;
}

int CFcActAddMaxHP::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	int	nHPPer = GetParamInt( 1 );
	TroopObjHandle hTarget = g_FcWorld.GetTroopObject( pTroopName );
	hTarget->SetMaxHPAdd( nHPPer );
	return ACTION_STATE_FINISH;
}

int CFcActShowTroopHPGaugeExceptLeader::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	int nTextID = GetParamInt( 1 );
	g_InterfaceManager.SetTroopGauge( hTroop, nTextID, false );

	return ACTION_STATE_FINISH;
}

int CFcActSetDefenseRef::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	int nPercent = max(GetParamInt(1), 1);

	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	if(!hTroop->IsEnable()) {
		hTroop->SetAdditionalDefense(-100+nPercent);
		return ACTION_STATE_FINISH;
	}

	int nUnits = hTroop->GetUnitCount();
	for(int i=0; i < nUnits; ++i) {
		if(!hTroop->GetUnit(i))
			continue;
		hTroop->GetUnit(i)->SetDefenseAdd(-100+nPercent);
	}

	return ACTION_STATE_FINISH;
}

int CFcActShowCircleAreaInMinimap::DoCommand()
{
	char *pAreaName = GetParamStr( 0 );
//	g_InterfaceManager.GetInstance().MinimapAreaOn(true, pAreaName);
	return ACTION_STATE_FINISH;
}

int CFcActHideCircleAreaInMinimap::DoCommand()
{
	char *pAreaName = GetParamStr( 0 );
//	g_InterfaceManager.GetInstance().MinimapAreaOn(false, pAreaName);
	return ACTION_STATE_FINISH;
}

int CFcTroopAnimationWithDelay::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	int nAni = GetParamInt( 1 );
	int nIndex = GetParamInt( 2 );
	int nAttr = GetParamInt( 3 );
	int nDelay = GetParamInt( 4 );

	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );

	if( m_bStart == false )
	{
		m_bStart = true;
		int nCnt = hTroop->GetUnitCount();
		m_pCheckTick = new int[nCnt];
		for( int i=0; i<nCnt; i++ )
		{
			m_pCheckTick[i] = Random( nDelay )+1;
		}
		return ACTION_STATE_WORKING;
	}
	else
	{
		bool bExist = false;
		int nCnt = hTroop->GetUnitCount();
		for( int i=0; i<nCnt; i++ )
		{
			if( m_pCheckTick[i] <= 0 )
				continue;

			m_pCheckTick[i]--;
			if( m_pCheckTick[i] == 0 )
			{
				GameObjHandle hUnit = hTroop->GetUnit( i );

				if( hUnit == NULL )
					continue;

				CFcGameObject::GameObj_ClassID ClassID = hUnit->GetClassID();
				if( ClassID == CFcGameObject::Class_ID_Horse ||
					ClassID == CFcGameObject::Class_ID_Catapult ||
					ClassID == CFcGameObject::Class_ID_MoveTower ||
					ClassID == CFcGameObject::Class_ID_Fly )
					continue;

				if( hUnit->IsDie() )
					continue;

				hUnit->ChangeAnimation( nAni, nIndex, nAttr );

				if( hUnit == g_FcWorld.GetHeroHandle() )
					g_FcWorld.GetHeroHandle()->UnlinkCatchObject( 0, -1, 0, true );
			}
			else
			{
				bExist = true;
			}
		}
		if( bExist )
		{
			return ACTION_STATE_WORKING;
		}
	}
	return ACTION_STATE_FINISH;
}


int CFcActTroopDamagedOnlyPlayer::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	int nPer = GetParamInt( 1 );

	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	if( !hTroop ) return ACTION_STATE_FINISH;
	int nCnt = hTroop->GetUnitCount();
	for( int i=0; i<nCnt; i++ )
	{
		GameObjHandle hUnit = hTroop->GetUnit( i );
		if(!hUnit)
			continue;
		switch( hUnit->GetClassID() ) {
			case CFcGameObject::Class_ID_Adjutant:
				((CFcAdjutantObject*)hUnit.m_pInstance)->SetOnlyPlayerDamageHPPercent( (float)nPer * 0.01f );
				break;
			case CFcGameObject::Class_ID_Hero:
				((CFcHeroObject*)hUnit.m_pInstance)->SetOnlyPlayerDamageHPPercent( (float)nPer * 0.01f );
				break;
		}
	}
	return ACTION_STATE_FINISH;
}

int CFcActItemProbbtAdd::DoCommand()
{
	float fAddProbbt = GetParamFloat(0);
	g_FcItemManager.AddItemDropProbbt( fAddProbbt );
	return ACTION_STATE_FINISH;
}


int CFcActVarSurvivalRateScoreVarArg::DoCommand() 
{
	int nVarID		= GetParamInt( 0 );
	int nTotalFriendlyVarID = GetParamInt( 1 );
	int nTotalFriendly = max( g_FcWorld.GetTriggerVarInt( nTotalFriendlyVarID ), 1);
	int nSurvivor = 0;

	CFcTroopManager* pTroopManager = g_FcWorld.GetTroopmanager();
	int nCnt = pTroopManager->GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hHandle = pTroopManager->GetTroop( i );
		if( hHandle->GetTeam()!=0)
			continue;
        nSurvivor+=hHandle->GetAliveUnitCount();	
	}
	if(nSurvivor > nTotalFriendly)
		nSurvivor = nTotalFriendly;

	float fKillPercent= ((float)nSurvivor/nTotalFriendly)*100;
	int nScore		= 0;
	
	for(int i=0; i<11; ++i) {
		if( SURVIVERATE_SCORETABLE[i].first >= fKillPercent) {
			nScore = SURVIVERATE_SCORETABLE[i].second;
			break;
		}
	}

	g_FcWorld.SetTriggerVar( nVarID, nScore );
	return ACTION_STATE_FINISH;	
}


int CFcActVarKillsScoreVarArg::DoCommand() 
{
	int nVarID		= GetParamInt( 0 );
	int nTotalEnemyVarID = GetParamInt( 1 );
	int nTotalEnemy = max( g_FcWorld.GetTriggerVarInt( nTotalEnemyVarID ), 1);

	HeroObjHandle Handle = g_FcWorld.GetHeroHandle();
	BsAssert(Handle);	

	int nScore= int(((float)Handle->GetKillCount()/nTotalEnemy)*35);
	//int nScore		= 0;
	//
	//for(int i=0; i<8; ++i) {
	//	if( KILLCOUNT_SCORETABLE[i].first >= nKillPercent) {
	//		nScore = KILLCOUNT_SCORETABLE[i].second;
	//		break;
	//	}
	//}

	g_FcWorld.SetTriggerVar( nVarID, nScore );
	return ACTION_STATE_FINISH;
}


int CFcActEnableEventTroopAI::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	int nCnt = hTroop->GetUnitCount();
	for( int i=0; i<nCnt; i++ )
	{
		GameObjHandle hUnit = hTroop->GetUnit( i );
		if( hUnit == NULL )
			continue;
		if( hUnit->IsDie() || hUnit->IsEnable() == false )
			continue;

		if( hUnit->GetAIHandle() )
		{
			hUnit->GetAIHandle()->SetForceProcessTick( 1 );
		}
	}
	return ACTION_STATE_FINISH;
}

int CFcActDisableEventTroopAI::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	int nCnt = hTroop->GetUnitCount();
	for( int i=0; i<nCnt; i++ )
	{
		GameObjHandle hUnit = hTroop->GetUnit( i );
		if( hUnit == NULL )
			continue;
		if( hUnit->IsDie() || hUnit->IsEnable() == false )
			continue;

		if( hUnit->GetAIHandle() )
		{
			hUnit->GetAIHandle()->SetForceProcessTick( -1 );
		}
	}
	return ACTION_STATE_FINISH;
}


int CFcActPointTroopStrOff::DoCommand()
{
	char* pTroopStr = GetParamStr( 0 );

	CFcTroopManager* pTroopManager = g_FcWorld.GetTroopmanager();
	int nCnt = pTroopManager->GetTroopCount();
	TroopObjHandle hHandle;
	for( int i=0; i<nCnt; i++ )
	{
		hHandle = pTroopManager->GetTroop( i );
		if( !hHandle->IsEnable() )
			continue;

		if( hHandle->IsEliminated() )
			continue;

		char* pTempStr = strstr( (char*)hHandle->GetName(), pTroopStr ); 
		if( pTempStr == NULL )
			continue;

		g_InterfaceManager.GetInstance().MinimapPointOn( false, hHandle->GetName());
	}
    

	return ACTION_STATE_FINISH;
}

int CFcActPointTroopStrOn::DoCommand()
{
    char* pTroopStr = GetParamStr( 0 );
    int   nTime     = GetParamInt(1);
	bool bShow = ( nTime != 0 );

	CFcTroopManager* pTroopManager = g_FcWorld.GetTroopmanager();
	int nCnt = pTroopManager->GetTroopCount();
	TroopObjHandle hHandle;
	for( int i=0; i<nCnt; i++ )
	{
		hHandle = pTroopManager->GetTroop( i );
		if( !hHandle->IsEnable() )
			continue;

		if( hHandle->IsEliminated() )
			continue;

		char* pTempStr = strstr( (char*)hHandle->GetName(), pTroopStr ); 
		if( pTempStr == NULL )
			continue;

		g_InterfaceManager.GetInstance().MinimapPointOn(bShow, hHandle->GetName(), hHandle, nTime * FRAME_PER_SEC);
	}

	return ACTION_STATE_FINISH;
}


int CFcActSetUnitAIEnable::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	CFcTroopManager* pTroopManager = g_FcWorld.GetTroopmanager();
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	int nCnt = hTroop->GetUnitCount();
	for( int i=0; i<nCnt; i++ )
	{
		GameObjHandle hUnit = hTroop->GetUnit( i );
		if( hUnit == NULL )
			continue;

		if( hUnit->IsEnable() == false )
			continue;

		if( hUnit->IsDie() == false )
			continue;

		if( hUnit->GetAIHandle() )
			hUnit->GetAIHandle()->SetEnable( true );
	}

	return ACTION_STATE_FINISH;
}

int CFcActSetUnitAIDisable::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );

	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	int nCnt = hTroop->GetUnitCount();
	for( int i=0; i<nCnt; i++ )
	{
		GameObjHandle hUnit = hTroop->GetUnit( i );
		if( hUnit == NULL )
			continue;

		if( hUnit->IsEnable() == false )
			continue;

		if( hUnit->IsDie() == false )
			continue;

		if( hUnit->GetAIHandle() )
			hUnit->GetAIHandle()->SetEnable( false );
	}

	return ACTION_STATE_FINISH;
}


int CFcActTroopLastDisable::DoCommand()
{
	char* pTroopName = GetParamStr( 0 );
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
	hTroop->CmdForceDisable();
	return ACTION_STATE_FINISH;
}


int CFcActTroopsLastDisable::DoCommand()
{
	char* pTroopStr = GetParamStr( 0 );

	CFcTroopManager* pTroopManager = g_FcWorld.GetTroopmanager();
	int nCnt = pTroopManager->GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hHandle = pTroopManager->GetTroop( i );
		char* pTempStr = strstr( (char*)hHandle->GetName(), pTroopStr ); 
		if( pTempStr == NULL )
			continue;

		hHandle->CmdForceDisable();
	}
	return ACTION_STATE_FINISH;
}

int CFcActVarMissionLevel::DoCommand()
{
	int nVarID = GetParamInt( 0 );

	int nHeroID = g_MissionToSoxID[g_FCGameData.nPlayerType];
	HeroRecordInfo* pHeroRecordInfo = g_FCGameData.tempFixedSave.GetHeroRecordInfo(nHeroID);
	StageResultInfo* pStageResultInfo = NULL;
	if(pHeroRecordInfo)
		pStageResultInfo = pHeroRecordInfo->GetStageResultInfo(g_FCGameData.SelStageId);

	int nLevel = -1;
	if( pStageResultInfo )
		nLevel = pStageResultInfo->nEnemyLevelUp + 3;
	else
		nLevel = g_FCGameData.stageInfo.nEnemyLevelUp + 3;

	g_FcWorld.SetTriggerVar( nVarID, nLevel );

	return ACTION_STATE_FINISH;
}

int CFcActVigMotionBlurEnable::DoCommand()
{
	g_FcWorld.EnableVigBlurEffect();
	return ACTION_STATE_FINISH;
}
int CFcActVigMotionBlurDisable::DoCommand()
{
	g_FcWorld.DisableVigBlurEffect();
	return ACTION_STATE_FINISH;
}

int CFcActSeSoundPlayingOff::DoCommand()
{
#ifdef _XBOX
	g_pSoundManager->SetSeSoundPlayingOn( false );
#endif
	return ACTION_STATE_FINISH;
}

int CFcActSeSoundPlayingOn::DoCommand()
{
#ifdef _XBOX
	g_pSoundManager->SetSeSoundPlayingOn( true );
#endif
	return ACTION_STATE_FINISH;
}

