#include "StdAfx.h"
#include "BsKernel.h"
#include "FcWorld.h"
#include "FcBaseObject.h"
#include "FcCameraObject.h"
#include "FcInterfaceManager.h"

#include "FCUtil.h"
#include "InputPad.h"
#include "bstreamext.h"
#include "DebugUtil.h"
#include "FcGlobal.h"
#include "BsUiFont.h"
#include "FcSoundManager.h"
#include "FcRealtimeMovie.h"
#include "FcTroopObject.h"

#include "GenericCamEditor.h"
#include "InputPad.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


extern CInputPad			g_InputPad;




// CFcCameraObject class

CFcCameraObject::CFcCameraObject(CCrossVector *pCross)
	: CFcBaseObject(pCross)
{
	m_fCameraDist=0.0f;
	m_nRotatePitchAngle = 0;
//	m_nRotatePitchAngle = 50;
	m_nRotateYawAngle = 0;
	m_fCameraZoom=0.0f;
	m_OriginalPos=pCross->m_PosVector;

	m_bFreeze = false;
//	m_bFreeCamMode=false;

	m_nInputPort=0;

	m_nPresetCam = -1;

//	memset(m_ppCamList,0,sizeof(CCrossVector*) * CAMERA_PRESET_MAX);
//	m_nCamList = 0;
	m_nCurEditCam = 0;
	m_nSavedSeqID = -1;
	m_bPreset = false;
	m_bInputEnable = false;

	m_fNormalModeDX = 0.f;
	m_fNormalModeDY = 0.f;

	m_nDelayFogTick = 0;

	m_fFogNear = 10.f;
	m_fFogFar  = 10000.f;

	m_fNearZ = 10.f;
	m_fFarZ  = 10000.f;

	SetCamFov(D3DX_PI*0.25f);

	m_vecUpdateRangeCenter = D3DXVECTOR2( 0.f, 0.f );

//	m_bRThumbPress = false;

/*
	BFileStream F(g_BsKernel.GetFullName("default.cam"));

	if (F.Valid() == true)
	{
		int nScan;

		for(nScan = 0;nScan < CAMERA_PRESET_MAX;nScan++)
		{
			BOOL	Valid;

			F.Read(&Valid,sizeof(BOOL), 4);

			if (Valid)
			{
				m_ppCamList[nScan] = new CAMINFO;
				m_ppCamList[nScan]->Load( &F );

//				m_ppCamList[nScan]->Cross.m_XVector = D3DXVECTOR3( 1.f, 0.f, 0.f );
//				m_ppCamList[nScan]->Cross.m_YVector = D3DXVECTOR3( 0.f, 1.f, 0.f );
//				m_ppCamList[nScan]->Cross.m_ZVector = D3DXVECTOR3( 0.f, 0.f, 1.f );
//				m_ppCamList[nScan]->Cross.m_PosVector = D3DXVECTOR3( 0.f, 0.f, 0.f );

//				m_ppCamList[nScan]->fFov = 1.f;
//				m_ppCamList[nScan]->fSway = 0.f;



			}
		}
	}
*/
//	m_pChooser = NULL;
	m_pEventCamera = NULL;
	m_pRMEventCamera = NULL;
	m_cDefaultFileName[0] = NULL;
	m_cUserFileName[0] = NULL;
	m_bDebugFreeCamMode = false;

	m_bUseBCCam = false;
}

CFcCameraObject::~CFcCameraObject()
{
/*
	int nScan;

	for(nScan = 0;nScan < CAMERA_PRESET_MAX;nScan++)
	{
		if (m_ppCamList[nScan] != NULL)
		{
			delete m_ppCamList[nScan];
			m_ppCamList[nScan] = NULL;
		}
	}
*/

	SAFE_DELETE( m_pCamEditor );			// m_pEventCamera 앞에서 지워야 한다.
	SAFE_DELETE( m_pEventCamera );
	SAFE_DELETE( m_pRMEventCamera );
	
//	SAFE_DELETE( m_pChooser );
}


//if(g_MenuHandle->GetCurMenuType() == _FC_MENU_DEBUG)
//	return;


// Render에서 하면 쓰레드 문제 생김
void CFcCameraObject::UpdateUI()
{
	if(m_pCamEditor)
		m_pCamEditor->Process();
/*
	if( m_pEventCamera->IsEditMode() )
	{
		int nStartY = 720 - ( m_pEventCamera->GetNumStatus() + 2 ) * 32 - 32;
		char	buf[1024];

		int nCurSeq = m_pEventCamera->GetCurSeqIndex();

		char cCamStr[32];
		if( nCurSeq < 1000 )			strcpy( cCamStr, "Default" );
		else							strcpy( cCamStr, "User" );

		sprintf( buf, "X:Delete, L-RTrigger:Change value, B:Test, Test cancel, Y:Copy, LThumb+Y:Paste %s", cCamStr );
		g_pFont->DrawUIText(50, nStartY, -1, -1, buf);
		nStartY += 32;

		int nDisplaySeqIndex = nCurSeq;
		if( nCurSeq >= 1000 )
		{
			nDisplaySeqIndex -= 1000;
		}
		if( m_pEventCamera->IsTest() )
		{
			if( GetProcessTick() % 40 < 20 )
				sprintf( buf," SeqIndex: %3d Play", nDisplaySeqIndex );
			else
				sprintf( buf," SeqIndex: %3d", nDisplaySeqIndex );
		}
		else
		{
			sprintf( buf," SeqIndex: %3d", nDisplaySeqIndex );
		}

		char* pSeqName = GetSeqName( nCurSeq );
		if( pSeqName )
		{
			strcat( buf,  "  ");
			strcat( buf,  pSeqName );
		}

		if( m_nCursorState == EDITCAM_CURSOR_STATE_SEQ_SELECT )
		{
			buf[0] = '>';
		}
		g_pFont->DrawUIText(50, nStartY, -1, -1, buf);

		nStartY += 32;
		int nCnt = m_pEventCamera->GetNumStatus();
		for( int i=0; i<nCnt; i++ )
		{
			FC_CAM_STATUS* pStatus = m_pEventCamera->GetCurStatus( i );

			char* cTargetTypeName = GetTypeName( pStatus->nTargetType );
			char* cEyeTypeName = GetTypeName( pStatus->nEyeType );
			char* cSwayTypeName = GetSwayTypeName( pStatus->nSwayType );
			
//			sprintf( buf," %3d, Dur: %4d, Accel: %2d, FOV: %1.2f, Sway: %3.1f, T: %12s, DOF: %4d, ROLL: %1.2f, ", 
//			i, pStatus->nDuration, pStatus->nAccelType, pStatus->fFov, pStatus->fSway, cTypeName, 
//			pStatus->nDOF, pStatus->fRoll );

			BsAssert( pStatus->nAccelType >= 0 && pStatus->nAccelType < 3 );

			sprintf( buf,"%3d, Dur:%4d, %3s, FOV:%1.2f, %6s, E:%12s, T:%12s, DOF:%4d, ROLL:%4d", 
				i, pStatus->nDuration, CamAccelType[pStatus->nAccelType], pStatus->fFov, cSwayTypeName, 
				cEyeTypeName, cTargetTypeName, pStatus->nDOF, pStatus->nRoll );

			if( m_nCursorY == i )
			{
				int nMark = g_CamEditCursorXPos[ m_nCursorX ];
				buf[ nMark ] = '>';
			}

			g_pFont->DrawUIText(50, nStartY, -1, -1, buf);
			nStartY += 32;
		}

		if( m_nCursorState == EDITCAM_CURSOR_STATE_SEQ_ADD )
		{
			sprintf( buf,"Press A to add" );
			g_pFont->DrawUIText(50, nStartY, -1, -1, buf);
		}
	}
*/
}


static float CAMERA_PREVENT_BLOCKING_LIMIT = 100.f;

bool CFcCameraObject::Render()
{
	bool bRet;

	bRet = CFcBaseObject::Render();

	CBsCamera *pCamera;
	pCamera = GetCamObj();
	if( !g_pFcRealMovie->IsPlay() )	{
		if( pCamera ) {
			pCamera->SetFOVByProcess( m_pEventCamera->GetFov() );
		}
	}
	else {
		if(pCamera) {
			pCamera->SetFOVByProcess(GetCamFov());
		}
	}

	/*
	 *	시야 가리는 유닛 알파블렌딩 처리!!
	 */
	const float fRcpLimit = 1.f/CAMERA_PREVENT_BLOCKING_LIMIT;
	std::vector< GameObjHandle > Objects;
	g_FcWorld.GetObjectListInRange(&m_Cross.m_PosVector, CAMERA_PREVENT_BLOCKING_LIMIT, Objects);
	UINT uiNearUnitsCount =Objects.size();
	D3DXVECTOR2 v2CamPos = D3DXVECTOR2(m_Cross.m_PosVector.x, m_Cross.m_PosVector.z);
	D3DXVECTOR2 v2UnitPos;
	for(UINT i=0;i<uiNearUnitsCount;++i) {
		if(Objects[i]->GetClassID() < CFcGameObject::Class_ID_Adjutant) {
			// Hero 이외의 Unit들만 Alpha Blocking사용
			v2UnitPos = Objects[i]->GetPosV2();
			float fLength = D3DXVec2Length(&(v2UnitPos-v2CamPos));
			if(fLength<CAMERA_PREVENT_BLOCKING_LIMIT) {
				float fAlpha = fLength*fRcpLimit;
				fAlpha = BsMax(fAlpha, 0.f);
				fAlpha = BsMin(fAlpha, 1.f);
				g_BsKernel.SendMessage(Objects[i]->GetEngineIndex(), BS_ENABLE_OBJECT_ALPHABLEND, TRUE);
				g_BsKernel.SendMessage(Objects[i]->GetEngineIndex(), BS_SET_OBJECT_ALPHA, DWORD(&fAlpha));
				g_BsKernel.SendMessage(Objects[i]->GetEngineIndex(), BS_RESTORE_OBJECT_ALPHABLEND);
			}
		}
	}

	return bRet;
}

int CFcCameraObject::Initialize(int nCamIndex, bool bUseDivide, int nInputPort, char* pDefaultCameraFileName, char* pUserCameraFileName )
{
	if(bUseDivide){
		m_nEngineIndex=CBsKernel::GetInstance().CreateCameraObject(-1, nCamIndex*0.505f, 0.0f, 0.495f, 1.0f);
	}
	else{
		m_nEngineIndex=CBsKernel::GetInstance().CreateCameraObject(-1, 0.0f, 0.0f, 1.0f, 1.0f);
	}

	m_nInputPort=nInputPort;

//	m_pChooser = new CFcCameraChooser();
	m_pEventCamera = new CGenericCamera( GetCamObj() );
	m_pEventCamera->EnableFreeCam( true );
	m_pEventCamera->SetToggleCamDist(g_FCGameData.bCameraToggle);

	m_pCamEditor = new CCamEditor( m_pEventCamera );
	m_pRMEventCamera = new CFcEventCamera( GetCamObj() );

	BsAssert( strlen(pDefaultCameraFileName) < 64 );
	BsAssert( strlen(pUserCameraFileName) < 64 );

	strcpy( m_cDefaultFileName, pDefaultCameraFileName );
	strcpy( m_cUserFileName, pUserCameraFileName );

	m_pEventCamera->SeqLoad( m_cDefaultFileName );
	m_pEventCamera->SeqUserLoad( m_cUserFileName );

	m_pEventCamera->SeqPlay( 20 );


/*
m_pEventCamera->SeqSave();
m_pEventCamera->SeqUserSave();
*/

/*
	g_BsKernel.chdir( "camera" );

	char cFileStr1[MAX_PATH], cFileStr2[MAX_PATH];
	strcpy( cFileStr1, g_BsKernel.GetFullName( pDefaultCameraFileName ) );
	strcpy( cFileStr2, g_BsKernel.GetFullName( pUserCameraFileName ) );

	if( m_pEventCamera->Load( cFileStr1, cFileStr2 ) == false )
	{
		if( m_pEventCamera->Save( cFileStr1, cFileStr2 ) == false )
		{
			BsAssert( 0 );
		}
	}
	g_BsKernel.chdir( ".." );
*/

	return m_nEngineIndex;
}

void CFcCameraObject::Process()
{
	if( ( m_bFreeze ) || ( !m_bEnable ) )
	{
		return;
	}

	SetAlphaBlocking();

	if( g_pFcRealMovie->IsPlay() && m_bDebugFreeCamMode == false )
	{
		if( !m_bUseBCCam )
		{
			m_pRMEventCamera->Process();
			m_pRMEventCamera->PostProcess();
			m_Cross = *(m_pRMEventCamera->GetCamCross());
#ifdef _DEBUG
			DebugString( "RMPos %d, %d, %d, %d\n", GetProcessTick(), (int)(m_Cross.m_PosVector.x), 
											(int)(m_Cross.m_PosVector.y), (int)(m_Cross.m_PosVector.z) );
#endif
		}
	}
	else
	{
		if( m_bDebugFreeCamMode )
 			ProcessFreeCam();	// 내부에서 m_Cross 세팅
		else {
			m_pEventCamera->Process();

			ProcessNormalMode();
			m_Cross = *(m_pEventCamera->GetCamCross());
			m_Cross.MoveRightLeft( m_fNormalModeDX );
			m_Cross.MoveUpDown( m_fNormalModeDY );
		}
		
	}
	
#if 1	// tohoshi
	{	
		D3DXVECTOR3 camDirV3 = m_Cross.m_ZVector;//  CamHandle->GetCrossVector()->m_ZVector;
		float fDist = D3DXVec3LengthSq( &camDirV3);
		if( fDist == 0.f )
			camDirV3 = D3DXVECTOR3( 0.f, 0.f, 1.f );
		else
			D3DXVec3Normalize( &camDirV3, &camDirV3 );
		
		D3DXVECTOR3 Pos = m_Cross.m_PosVector;
		//Pos.y += GetUnitHeight();
		g_pSoundManager->SetListenerPos( Pos, camDirV3);
	}
#endif

	ProcessFog();
}

void CFcCameraObject::AttachObject( CSmartPtr<CFcBaseObject> Handle )
{
	CCrossVector *pCross;

	m_AttachHandle = Handle; 
	pCross = m_AttachHandle->GetCrossVector();
	m_Cross.m_PosVector = pCross->m_PosVector - pCross->m_ZVector * 100.0f;
	m_PrevPos = m_Cross.m_PosVector;
	m_OriginalPos = m_PrevPos;
}


void CFcCameraObject::SetTargetTroop( TroopObjHandle Handle )
{
	m_pEventCamera->SetTargetTroop( Handle );
}
	

bool CFcCameraObject::IsFreeCamMode()
{
	return m_bDebugFreeCamMode;
}


void CFcCameraObject::SetFreeCamMode( bool bMode )
{
	m_bDebugFreeCamMode = bMode;
}

void CFcCameraObject::SetFreeCamMode_( bool bMode )
{
	m_pEventCamera->EnableFreeCam( bMode );

	/*
	if( m_bFreeCamMode == false && bMode == true )
	{
	m_pEventCamera->SeqStop();
	SaveCam();
	}
	else if( m_bFreeCamMode == true && bMode == false )
	RestoreCam();

	m_bFreeCamMode = bMode;
	*/
}




bool CFcCameraObject::IsCamPreset(void)
{
	return (m_bPreset | m_pEventCamera->IsEditMode() );
}


bool CFcCameraObject::IsEditMode()
{
	return m_pEventCamera->IsEditMode();
}

void	CFcCameraObject::SetEditMode( bool bMode )
{
	if( bMode )
	{
		g_InputPad.HookOn( true );
		m_pCamEditor->Activate();
	}
	else
	{
		g_InputPad.HookOn( false );
		m_pCamEditor->Deactivate();
	}

	m_pEventCamera->SetEditMode( bMode );


}



void CFcCameraObject::Save(void)
{
//	g_BsKernel.chdir( "camera" );

//	char cDefaultFileName[MAX_PATH], cUserFileName[MAX_PATH];
//	strcpy( cDefaultFileName, g_BsKernel.GetFullName( m_cDefaultFileName ) );
//	strcpy( cUserFileName, g_BsKernel.GetFullName( m_cUserFileName ) );

	m_pEventCamera->SeqSave();
	m_pEventCamera->SeqUserSave();

/*
	if( m_pEventCamera->Save( cDefaultFileName, cUserFileName ) == false )
	{
		BsAssert( 0 );
	}
*/
//	g_BsKernel.chdir( ".." );
}

CBsCamera*	CFcCameraObject::GetCamObj()
{
	CBsCamera *pCamObj;

	pCamObj = (CBsCamera*) g_BsKernel.GetEngineObjectPtr(m_nEngineIndex);

	return pCamObj;
}

/*
void CFcCameraObject::SetSway( CAM_SWAY_TYPE nType )
{
	m_pEventCamera->SetSway( nType );
}
*/

bool CFcCameraObject::SetCurDuration( int nTick )
{
//	return m_pEventCamera->SetCurDuration( nTick );
	return false;
}


void CFcCameraObject::SetFogColor( D3DXVECTOR4* pVec )
{
	g_BsKernel.SetFogColor( m_nEngineIndex, pVec );
}

void CFcCameraObject::SetFogFactor( float fStart, float fEnd )
{
	m_fFogNear = fStart;
	m_fFogFar  = fEnd;
	g_BsKernel.SetFogFactor( m_nEngineIndex, fStart, fEnd );
}

void CFcCameraObject::SetProjectionMatrix( float fNear, float fFar )
{
	m_fNearZ = fNear;
	m_fFarZ  = fFar;
	g_BsKernel.SetProjectionMatrix( m_nEngineIndex, m_fNearZ, m_fFarZ );
}

void CFcCameraObject::ChangeFog( D3DXVECTOR4* pColor, float fNear, float fFar, int nTick )
{
	const D3DXVECTOR4* pCurFog = g_BsKernel.GetFogColor( m_nEngineIndex );
	m_vecDeltaFogColor = *pColor - *pCurFog;
	++nTick;
	BsAssert( nTick > 0 );
	m_vecDeltaFogColor = m_vecDeltaFogColor / (float)nTick;
	m_nDelayFogTick = nTick;
	float fCurNear = g_BsKernel.GetFogStart( m_nEngineIndex );
	float fCurFar = g_BsKernel.GetFogEnd( m_nEngineIndex );
	m_fDeltaFogNear = ( fNear - fCurNear ) / (float)nTick;
	m_fDeltaFogFar = ( fFar - fCurFar ) / (float)nTick;
}

void CFcCameraObject::SetDevCam( bool bEnable )
{
	m_pEventCamera->SetDevMode( bEnable );
}

void CFcCameraObject::SetUserSeq(int nSeqId, int nDuration, bool bForce)
{
	m_pEventCamera->SetUserSeq( nSeqId, nDuration, bForce );
}

int CFcCameraObject::SeqStop()
{
	m_pEventCamera->SeqStop();
	return -1;
}


void CFcCameraObject::SetMarkPoint( D3DXVECTOR2* pPos )
{
	m_pEventCamera->SetMarkedPoint( pPos );
}

void CFcCameraObject::SetUserTarget( TroopObjHandle pTarget )
{
	m_pEventCamera->SetUserTarget( pTarget);
}

void CFcCameraObject::SetAlphaBlocking()
{
	if( g_pFcRealMovie->IsPlay() )		// TODO : 조건 넣어줘야 함
	{
		g_FcWorld.SetAlphaBlocking( false );
		return;
	}

	CAMSTATUS* pStatus = m_pEventCamera->GetCamStatus();
	if( pStatus->nTarget == 4 /*CAM_ATTACH_PLAYER*/ )
		g_FcWorld.SetAlphaBlocking( true );
	else
		g_FcWorld.SetAlphaBlocking( false );
}

void CFcCameraObject::MovePlayBack( int nDelayTick )
{
	m_pEventCamera->MovePlayBack( nDelayTick );
}
 

bool CFcCameraObject::LookTarget( FC_CAM_STATUS* pStatus, CCrossVector* pCross )
{
/*
	D3DXVECTOR3 Eye, Dir;
	if( m_pChooser->GetCamPos( pStatus, &Eye, &Dir ) == false )
		return false;

	pCross->Reset();
	pCross->m_PosVector = Eye;
	D3DXVec3Scale( &Dir, &Dir, 100.f );		// Dir이 너무 작으면 LookAt이 부정확하다
	Dir = Dir + Eye;
	pCross->LookAt( &Dir );

	if( pCross->m_XVector.x == 0.f )
		DebugString( "Look err\n" );
*/
	return true;
}

void CFcCameraObject::SeqPlay(int nSeqId)
{
	m_pEventCamera->SeqPlay( nSeqId );
}


void CFcCameraObject::SeqSave(void)
{
	m_pEventCamera->SeqSave();
}

void CFcCameraObject::SeqUserSave(void)
{
	m_pEventCamera->SeqUserSave();
}


bool CFcCameraObject::SeqLoad(char *pFileName)
{
	return m_pEventCamera->SeqLoad( pFileName );
}

bool CFcCameraObject::SeqUserLoad(char *pFileName)
{
	return m_pEventCamera->SeqUserLoad( pFileName );
}

const char* CFcCameraObject::GetSeqFileName(void)
{
	return m_pEventCamera->GetSeqFileName();
}

const char* CFcCameraObject::GetSeqFileName2(void)
{
	return m_pEventCamera->GetSeqFileName2();
}


const char* CFcCameraObject::GetUserSeqFileName(void)
{
	return m_pEventCamera->GetUserSeqFileName();
}

void CFcCameraObject::SetRMCamSeq( FC_CAM_SEQ* pSeq )
{
	m_pRMEventCamera->SetSeq( pSeq );
}


void CFcCameraObject::ProcessEditMode()
{
/*
	if( !m_pEventCamera->IsTest() )
		m_pEventCamera->SetCamCross( m_pEventCamera->GetCross() );

	CCrossVector* pCross = m_pEventCamera->GetCamCross();

	if( CInputPad::GetInstance().GetKeyPressTick(m_nInputPort, PAD_INPUT_LTRIGGER) == 0 )
	{
		
		if(CInputPad::GetInstance().GetKeyPressTick(m_nInputPort, PAD_INPUT_LSTICK)){
			KEY_EVENT_PARAM* pParam = CInputPad::GetInstance().GetKeyParam(m_nInputPort, 0);
//			m_fEditModeDX += pParam->nPosX * 0.0015f;
//			m_fEditModeDY += pParam->nPosY * 0.0015f;

			pCross->MoveRightLeft( pParam->nPosX * 0.0015f );
			pCross->MoveFrontBack( pParam->nPosY * 0.0015f );
		}
	}
	else
	{
		if(CInputPad::GetInstance().GetKeyPressTick(m_nInputPort, PAD_INPUT_LSTICK))
		{
			KEY_EVENT_PARAM* pParam = CInputPad::GetInstance().GetKeyParam(m_nInputPort, 0);
			pCross->MoveRightLeft(pParam->nPosX *0.001f);
			pCross->MoveUpDown(pParam->nPosY*0.001f);
		}
	}

	
	if(CInputPad::GetInstance().GetKeyPressTick(m_nInputPort, PAD_INPUT_RSTICK))
	{
		KEY_EVENT_PARAM* pParam = CInputPad::GetInstance().GetKeyParam(m_nInputPort, 1);
		pCross->RotateYaw(-int(pParam->nPosX *0.0001f));
		pCross->RotatePitch(int(pParam->nPosY*0.0001f));
	}


	

	// paste
	if(CInputPad::GetInstance().GetKeyPressTick(m_nInputPort, PAD_INPUT_Y) &&
		CInputPad::GetInstance().GetKeyPressTick(m_nInputPort, PAD_INPUT_LTHUMB)){
			if( m_pBackUpSeq )
			{
				int nIndex = m_pEventCamera->GetCurSeqIndex();
				FC_CAM_SEQ* pSeq = m_pEventCamera->GetSeq( nIndex );
				pSeq->vecStatus.clear();
				*pSeq = *m_pBackUpSeq;
			}
		}
		// 현재 Index 카피
	else if(CInputPad::GetInstance().GetKeyPressTick(m_nInputPort, PAD_INPUT_Y)){
		int nIndex = m_pEventCamera->GetCurSeqIndex();
		m_pBackUpSeq = m_pEventCamera->GetSeq( nIndex );
	}
	pCross->m_YVector=D3DXVECTOR3(0.f, 1.f, 0.f);
	pCross->UpdateVectors();
*/
}

void CFcCameraObject::ProcessFreeCam()
{
/*
//	CCrossVector* pCross = m_pEventCamera->GetCamCross();
	CCrossVector* pCross = m_pEventCamera->GetCamCross();
	if(CInputPad::GetInstance().GetKeyPressTick(m_nInputPort, PAD_INPUT_LSTICK)){
		KEY_EVENT_PARAM* pParam = CInputPad::GetInstance().GetKeyParam(m_nInputPort, 0);
		pCross->MoveRightLeft( pParam->nPosX * 0.0015f );
		pCross->MoveFrontBack( pParam->nPosY * 0.0015f );
	}
	if(CInputPad::GetInstance().GetKeyPressTick(m_nInputPort, PAD_INPUT_RSTICK)){
		KEY_EVENT_PARAM* pParam = CInputPad::GetInstance().GetKeyParam(m_nInputPort, 1);
		pCross->RotateYaw(-int(pParam->nPosX *0.0001f));
		pCross->RotatePitch(int(pParam->nPosY*0.0001f));
	}

	if(CInputPad::GetInstance().GetKeyPressTick(m_nInputPort, PAD_INPUT_A)){
		pCross->MoveUpDown(-10.f);
	}
	if(CInputPad::GetInstance().GetKeyPressTick(m_nInputPort, PAD_INPUT_B)){
		pCross->MoveUpDown(10.f);
	}
	pCross->m_YVector=D3DXVECTOR3(0.f, 1.f, 0.f);
	pCross->UpdateVectors();
//	m_pEventCamera->SetRoll();
*/

	if(CInputPad::GetInstance().GetKeyPressTick(m_nInputPort, PAD_INPUT_LSTICK)){
		KEY_EVENT_PARAM* pParam = CInputPad::GetInstance().GetKeyParam(m_nInputPort, 0);
		m_Cross.MoveRightLeft( pParam->nPosX * 0.0015f );
		m_Cross.MoveFrontBack( pParam->nPosY * 0.0015f );
	}
	if(CInputPad::GetInstance().GetKeyPressTick(m_nInputPort, PAD_INPUT_RSTICK)){
		KEY_EVENT_PARAM* pParam = CInputPad::GetInstance().GetKeyParam(m_nInputPort, 1);
		m_Cross.RotateYaw(-int(pParam->nPosX *0.0001f));
		m_Cross.RotatePitch(int(pParam->nPosY*0.0001f));
	}

	if(CInputPad::GetInstance().GetKeyPressTick(m_nInputPort, PAD_INPUT_A)){
		m_Cross.MoveUpDown(-10.f);
	}
	if(CInputPad::GetInstance().GetKeyPressTick(m_nInputPort, PAD_INPUT_B)){
		m_Cross.MoveUpDown(10.f);
	}
	m_Cross.m_YVector=D3DXVECTOR3(0.f, 1.f, 0.f);
	m_Cross.UpdateVectors();
	//	m_pEventCamera->SetRoll();
}

void CFcCameraObject::ProcessNormalMode()
{
	if( m_bPreset == true )
		return;

	if( m_pCamEditor->IsActivated() )
		return;

	if( g_InterfaceManager.GetInstance().GetMenuMgr()->GetCurMenuType() == _FC_MENU_DEBUG )
		return;

	if( CInputPad::GetInstance().IsBreak() || CInputPad::GetInstance().IsHookOn() || !CInputPad::GetInstance().IsEnable() )
	{
		return;
	}

	float	fDX, fDY;
// [beginmodify] 2006/1/16 junyash PS#1259,3249,3251, TCR#033PER multi controller
	#if 0
	KEY_EVENT_PARAM* pParam = CInputPad::GetInstance().GetKeyParam(-1, 1);
	#else
	KEY_EVENT_PARAM* pParam = CInputPad::GetInstance().GetKeyParam(m_nInputPort, 1);
	#endif
// [endmodify] junyash
	fDX = (float)pParam->nPosX;
	fDY = (float)pParam->nPosY;

	if (fDX > JOYPAD_ANALOG_DEADZONE)
		fDX -= (float) JOYPAD_ANALOG_DEADZONE;
	else if (fDX < -JOYPAD_ANALOG_DEADZONE)
		fDX += (float) JOYPAD_ANALOG_DEADZONE;
	else
		fDX = 0.0f;

	if (fDY > JOYPAD_ANALOG_DEADZONE)
		fDY -= (float) JOYPAD_ANALOG_DEADZONE;
	else if (fDY < -JOYPAD_ANALOG_DEADZONE)
		fDY += (float) JOYPAD_ANALOG_DEADZONE;
	else
		fDY = 0.0f;

	fDX = fDX / ((float) (JOYPAD_ANALOG_RANGE_MAX - JOYPAD_ANALOG_DEADZONE)) * 0.075f;
	fDY = fDY / ((float) (JOYPAD_ANALOG_RANGE_MAX - JOYPAD_ANALOG_DEADZONE)) * 0.075f;

// [beginmodify] 2006/1/27 junyash PS#2795, TCR#067GP use Gamer profile setting for Vertical View Control
	#if 0
	// Up/Down & Left/Right axis wrong
	if( g_FCGameData.ConfigInfo.bCameraUD_Reverse )
	{
		fDX = -fDX;
	}
	if( g_FCGameData.ConfigInfo.bCameraLR_Reverse )
	{
		fDY = -fDY;
	}
	#else
	if( g_FCGameData.ConfigInfo.bCameraUD_Reverse )
	{
		fDY = -fDY;
	}
	if( g_FCGameData.ConfigInfo.bCameraLR_Reverse )
	{
		fDX = -fDX;
	}
	#endif
// [endmodify] junyash

	if (fDX != 0.0f || fDY != 0.0f)
		m_pEventCamera->MoveFreeCam(fDX, fDY);

	if( CInputPad::GetInstance().GetKeyPressTick( m_nInputPort, PAD_INPUT_RTHUMB ) == 1 ) {
		m_pEventCamera->ToggleCamDist();	
		g_FCGameData.bCameraToggle = m_pEventCamera->IsToggleCamDist();
	}

	if( CInputPad::GetInstance().GetKeyPressTick( m_nInputPort, PAD_INPUT_LTRIGGER ) == 1 )
		MovePlayBack( 2 );

/*
	if( CInputPad::GetInstance().GetKeyPressTick( m_nInputPort, PAD_INPUT_LTRIGGER ) == 1 &&
		CInputPad::GetInstance().GetKeyPressTick( m_nInputPort, PAD_INPUT_RTRIGGER ) == 1 )
		m_pEventCamera->MovePlayBack();
*/
 }

 void CFcCameraObject::ProcessFog()
 {
	 // 포그 조절
	 if( m_nDelayFogTick > 0 )
	 {
		 m_nDelayFogTick--;
		 const D3DXVECTOR4* pCurFog = g_BsKernel.GetFogColor( m_nEngineIndex );
		 D3DXVECTOR4 FogColor = *pCurFog + m_vecDeltaFogColor;
		 SetFogColor( &FogColor );
		 float fCurNear = g_BsKernel.GetFogStart( m_nEngineIndex );
		 float fCurFar = g_BsKernel.GetFogEnd( m_nEngineIndex );
		 fCurNear += m_fDeltaFogNear;
		 fCurFar += m_fDeltaFogFar;
		 SetFogFactor( fCurNear, fCurFar );
	 }
 }


 