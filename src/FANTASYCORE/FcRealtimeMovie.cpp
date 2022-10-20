#include "stdafx.h"
#include "FcRealtimeMovie.h"
#include "BsKernel.h"
#include "BsPhysicsMgr.h"
#include "BsSkin.h"

#include "FcCameraObject.h"
#include "BsRealMovie.h"
#include "BsRealMovieObject.h"
#include "BsRealMovieUtil.h"
#include "BsFileIO.h"

#include "FcWorld.h"
#include "FcHeroObject.h"
#include "FcTroopObject.h"

#include "FcPhysicsLoader.h"
#include "FcInterfaceManager.h"
#include "FcEventCamera.h"
#include "FcGlobal.h"
#include "TextTable.h"

#include "FcSoundManager.h"
#include "FcItem.h"

#include "InputPad.h"

#include "FantasyCore.h"
#include "FcWorld.h"
#include "FcFXManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


// g_InterfaceManager

FcRealtimeMovie* g_pFcRealMovie = NULL;

VOID	(FcRealtimeMovie::*m_pfnRealtimeMovieRenderProcess)(VOID);

BOOL FcRealtimeMovie::m_bPause = FALSE;

FcRealtimeMovie::FcRealtimeMovie()
{
	m_bPostPause = FALSE;
	m_bPlay = false;
	// m_bPause = FALSE;
	m_nTick = 0;
	m_pKey = NULL;
	m_nOldTick = -1;
	m_bLoaded = FALSE;
	m_bDirectMsg = FALSE;

	memset( m_szFileName , 0 , 32 );

	m_pfnRenderProcess = FcRealtimeMovie::EmptyRenderProcess;
	m_pfnWorkProcess = FcRealtimeMovie::EmptyRenderProcess;
	m_pfnPostRenderProcess = FcRealtimeMovie::EmptyRenderProcess;
	SetIsDelete(false);
	ClearConsoleFlag();
	InitSndPool();

	m_hResPack = NULL;
	m_hTexPack = NULL;
}

FcRealtimeMovie::~FcRealtimeMovie()
{
	/*if( IsPlay() )
	{
		Clear();
	}*/
}

VOID	FcRealtimeMovie::EmptyRenderProcess(FcRealtimeMovie* pFcRM)
{
}

// Load 하면 PreAllocateRenderProcess 로 들어옴.
// Load 가 끝나면 개별 리소스 할당 작업을 위해 AllocateRenderProcess 로 넘어감.
VOID	FcRealtimeMovie::PreAllocateRenderProcess(FcRealtimeMovie* pFcRM)
{
	pFcRM->LoadRealMovie( pFcRM->GetFilename() );
	// pFcRM->StartRealMovie();
	pFcRM->m_pfnRenderProcess = FcRealtimeMovie::AllocateRenderProcess;
	// m_pfnRealtimeMovieRenderProcess = FcRealtimeMovie::EmptyRenderProcess;
}

VOID	FcRealtimeMovie::AllocateRenderProcess(FcRealtimeMovie* pFcRM)
{
	CBsRealMovie* pRealMovie = g_BsKernel.GetRealMovie();

	if( pFcRM->m_nTempIndex < pFcRM->m_nTempObjCnt )
	{
        pRealMovie->AllocateSinglyTempResource( pFcRM->m_nTempIndex++ );
	}
	else
	{
		pRealMovie->SyncResources();
		pRealMovie->ReleaseTempBuffer();

		pFcRM->PreLoadFXForRM( pRealMovie );

		pFcRM->m_nTempObjCnt = 0;
		pFcRM->m_nTempIndex = 0;
		pFcRM->m_pfnRenderProcess = FcRealtimeMovie::EmptyRenderProcess;
		pFcRM->m_pfnWorkProcess = FcRealtimeMovie::StartSignalWorkProcess;
	}
}

VOID	FcRealtimeMovie::ReleaseRMDataRenderProcess(FcRealtimeMovie* pFcRM)
{
	CBsRealMovie* pRealMovie = g_BsKernel.GetRealMovie();
	pRealMovie->Clear();
	pFcRM->m_pfnRenderProcess = FcRealtimeMovie::EmptyRenderProcess;
	pFcRM->m_pfnPostRenderProcess = FcRealtimeMovie::ReleasePostProcess;
}

VOID	FcRealtimeMovie::PreLoadRealMovie(char* pFileName)
{
	m_bLoaded = FALSE;
	SetFilename( pFileName );
    m_pfnRenderProcess = FcRealtimeMovie::PreAllocateRenderProcess;
}

VOID	FcRealtimeMovie::StartSignalWorkProcess(FcRealtimeMovie* pFcRM)
{
	pFcRM->m_bLoaded = TRUE;

	if( pFcRM->m_bDirectMsg )
	{
		pFcRM->StartRealMovie();
	}

	pFcRM->m_pfnWorkProcess = FcRealtimeMovie::EmptyRenderProcess;
}

VOID	FcRealtimeMovie::ReleasePostProcess(FcRealtimeMovie* pFcRM)
{
	pFcRM->ReleaseFXHandle();
	pFcRM->m_pfnRenderProcess = FcRealtimeMovie::EmptyRenderProcess;
	pFcRM->m_pfnPostRenderProcess = FcRealtimeMovie::EmptyRenderProcess;
	pFcRM->m_bDirectMsg = FALSE;
}

extern void	CreateDebugFile(int nIndex);
extern void	CloseDebugFile(int nIndex);

VOID	FcRealtimeMovie::DeleteContents(VOID)
{
	CameraObjHandle CamHandle;
	CamHandle = CFcBaseObject::GetCameraObjectHandle( 0 );

	CamHandle->StopEventSeq();
	CamHandle->SetProjectionMatrix( m_fOldNear, m_fOldFar );

	ReleaseFXPool();

	// Unlink만 하기.

	CBsRealMovie* pRealMovie = g_BsKernel.GetRealMovie();
	if( pRealMovie ) {
		pRealMovie->Clear();
	}

	// Text Table 삭제.
	// g_TextTable->ReleaseRealMovieTextTable();

	g_FcWorld.RestoreLight();
	g_FcWorld.SetPlayerEnable( true );
	CInputPad::GetInstance().Break(false);

	g_pSoundManager->RestoreVolCtgy();
	g_pSoundManager->PauseBGM( FALSE );

	m_bPlay = false;
	
	if( m_pKey )
	{
		delete [] m_pKey;
		m_pKey = NULL;
	}

	ClearSndPool();
	// g_InterfaceManager.GetInstance().SetFadeEffect( true  , 0 );

	// Post Process 설정.
	m_pfnPostRenderProcess = FcRealtimeMovie::ReleasePostProcess;

	if( GetConsoleFlag() )
	{
		g_InterfaceManager.GetInstance().SetFadeEffect( true , 0 );
		ClearConsoleFlag();
	}

#ifdef _PACKED_RESOURCES
	if( m_hResPack )
	{
		CBsFileIO::BsDismountPackFile( m_hResPack );
		m_hResPack = NULL;
	}
	if( m_hTexPack )
	{
		CBsFileIO::BsDismountPackFile( m_hTexPack );
		m_hTexPack = NULL;
	}
#endif


	if( GetPostPauseFlag() )
	{
		SetPostPauseFlag( FALSE );
        g_FC.SetPausMenuforXboxGuide();
	}
}

void	FcRealtimeMovie::LoadRealMovie(char* pFileName)
{
	if( m_pKey )
	{
		delete [] m_pKey;
	}

	m_pKey = new msgRMTNotify[96];

	char* pTempStr = strrchr( pFileName, '\\' );
	char cPackName[32];
	BsAssert( pTempStr );
	pTempStr++;
	strcpy( cPackName, pTempStr );
	pTempStr = strstr( cPackName, "." );
	pTempStr[0] = NULL;

#ifdef _PACKED_RESOURCES
	if( m_hResPack )
	{
		CBsFileIO::BsDismountPackFile( m_hResPack );
		m_hResPack = NULL;
	}
	if( m_hTexPack )
	{
		CBsFileIO::BsDismountPackFile( m_hTexPack );
		m_hTexPack = NULL;
	}

	char cFullPackName[256];
	sprintf( cFullPackName, "d:\\data\\PackedResources\\%s_pak_res.pak", cPackName );
	m_hResPack = CBsFileIO::BsMountCompressedPackFile( cFullPackName );
	sprintf( cFullPackName, "d:\\data\\PackedResources\\%s_pak_tex.pak", cPackName );
	m_hTexPack = CBsFileIO::BsMountCompressedPackFile( cFullPackName, true );
#endif

	g_BsKernel.chdir( "realmovie" );
	char* pFullName = g_BsKernel.GetFullName( pFileName );
	g_BsKernel.chdir( ".." );		// RealMovieLoad후 처리

	DebugString("\n-- Realtime Movie Filename : [%s]\n", pFullName );

	DWORD dwFileSize;
	VOID *pData;

	if( FAILED(CBsFileIO::LoadFile( pFullName, &pData, &dwFileSize ) ) )
	{
		BsAssert( 0 && "Cannot play realtime movie" );
	}

	g_BsKernel.RealMovieClear();				// 메모리 삭제 합니다

	BMemoryStream Stream(pData, dwFileSize);
	DebugString("-- Realtime Movie Stream Len : %d\n", Stream.Length() );

	g_BsKernel.RealMovieLoad( &Stream );		// AllocateResource() 포함
	CBsFileIO::FreeBuffer(pData);

	CBsRealMovie* pRealMovie = g_BsKernel.GetRealMovie();
	pRealMovie->m_pfnCreatePhysics = FcRealtimeMovie::LoadPhysicsData;
#ifdef _XBOX
	pRealMovie->m_pfnClearPhysics = FcRealtimeMovie::ReleasePhysicsData;
	pRealMovie->m_pfnCreateCollision = FcRealtimeMovie::LoadCollisionData;
	pRealMovie->m_pfnClearCollision = FcRealtimeMovie::ReleaseCollisionData;
#else
	pRealMovie->m_pfnClearPhysics = NULL;
	pRealMovie->m_pfnCreateCollision = NULL;
	pRealMovie->m_pfnClearCollision = NULL;
#endif
	
	pRealMovie->m_nPlayerWeaponSkinIndex = g_FcItemManager.GetHeroWeaponSkinIndex();
	//  BsAssert( pRealMovie->m_nPlayerWeaponSkinIndex != -1 && "weapon skin index == -1");

	// For Test
	if( pRealMovie->m_nPlayerWeaponSkinIndex != -1 )
	{
        CBsSkin	   *pSkin = g_BsKernel.Get_pSkin( pRealMovie->m_nPlayerWeaponSkinIndex );
		DebugString("** Check Weapon skin name : [%s]\n",pSkin->GetSkinFileName() );
	}
	else
	{
		DebugString( "*** RealtimeMovie : Weapon Skin not found! But, You can ignored this warning.\n" );
	}

	pRealMovie->AllocateResource();
	LoadWaveResource( *pRealMovie );
	

	// Test Table 설정
	// g_TextTable->SetRealMovieTextTable( pRealMovie->GetBGInfo().GetTextTblName() );
	// Sound Bank 설정

	LARGE_INTEGER liFrequency;
	LARGE_INTEGER liStartTime;
	LARGE_INTEGER liCurTime;
	QueryPerformanceFrequency(&liFrequency);
	QueryPerformanceCounter(&liStartTime);


	char* pBnkName = pRealMovie->GetBGInfo().GetWaveBnkName();
	if( pBnkName[0] == NULL )
	{
		DebugString("** Error : (Realtime Movie) WaveBankName Buffer is empty\n");
		// BsAssert( pBnkName[0] );
	}

	// Don't use
	// g_pSoundManager->LoadWaveBank( pBnkName );

	m_nTempIndex = 0;
	m_nTempObjCnt = pRealMovie->GetTempObjBufSize();

	QueryPerformanceCounter(&liCurTime);	
	double dwTime = (double)(( liCurTime.QuadPart - liStartTime.QuadPart ) / ( liFrequency.QuadPart / 1000.f ));
	DebugString("-- Realtime Move Sound Data time (%f ms) PreBufferCnt [%d]\n", dwTime, m_nTempObjCnt );

//	m_bLoaded = TRUE;

	g_pSoundManager->SaveVolCtgy();
	//g_pSoundManager->SetVolumeCtgy( XACT_VC_AMBIENCE, 0.2f );

}

void	FcRealtimeMovie::PlayDirect(char* pFileName)
{
#ifdef MAKE_ALL_MISSION_RESLIST
	return;
#endif

	m_bDirectMsg = TRUE;
	PreLoadRealMovie( pFileName );
}

void FcRealtimeMovie::StartRealMovie()
{
	m_nTick = 0;
	m_nOldTick = -1;
	m_nSavedCamSeqIndex = 0;
	m_bPlay = true;

	// 주인공 HP 채크
	if( g_FcWorld.GetHeroHandle()->GetHP() < 0 )
	{
        DeleteContents();	// 곧바로 종료로 이동.
		return;
	}

	// m_bPostPause = FALSE;

	m_bLoaded = FALSE;
	m_bDirectMsg = FALSE;

	CameraObjHandle CamHandle;
	CamHandle = CFcBaseObject::GetCameraObjectHandle( 0 );
//	CamHandle->SaveCam();

	// 백업
	m_fOldNear = g_BsKernel.GetCamera( CamHandle->GetEngineIndex() )->GetNearZ();
	m_fOldFar = g_BsKernel.GetCamera( CamHandle->GetEngineIndex() )->GetFarZ();

	g_FcWorld.SaveLight();
	g_FcWorld.SetPlayerEnable( false );
	// CInputPad::GetInstance().Break(true);

	CamHandle->SetProjectionMatrix(10.f, 100000.f);

	CBsRealMovie* pRealMovie = g_BsKernel.GetRealMovie();
	pRealMovie->MakeEventMap( &m_vecSndEventList );

	pRealMovie->ClearObjFlags();
	pRealMovie->ResetAllObjectAni();

	pRealMovie->ApplyObjectCullMove( false );
    // g_InterfaceManager.GetInstance().SetFadeEffect( true , 0 );

	// g_BsKernel.EnableDOF( 0 );
	g_pSoundManager->PauseBGM( TRUE );

	g_BsKernel.EnableDOF( 1 );
	g_BsKernel.SetDOFFocus( 1000.f );
	g_BsKernel.SetDOFFocusNear( 1000.f );
	g_BsKernel.SetDOFFocusFar( 1000.f );
}

void	FcRealtimeMovie::LoadWaveResource(CBsRealMovie& rRealMovie)
{
	char szKey[130];
	int nObjCnt = rRealMovie.GetObjectCount();

	CBsRealMovie::iteratorRMObject itObj = rRealMovie.m_mapObject.begin();
	CBsRealMovie::iteratorRMObject itObjEnd = rRealMovie.m_mapObject.end();
    
	for( ; itObj != itObjEnd ; ++itObj )
	{
		mapStrLip& rLip = itObj->second->GetLipMap();

		itStrLip itLip = rLip.begin();
		itStrLip itLipEnd = rLip.end();

		for( ; itLip != itLipEnd ; ++itLip )
		{
			CRMLip *pRMLip = itLip->second;

			if( pRMLip->HasWave() )
			{
				pRMLip->GetKeyString( szKey, _countof(szKey) );	// 데이터 Pool 저장용 Key
				// pRMLip->GetWavFileName(); 이용..
			}
		}
	} // end of for
}


void FcRealtimeMovie::Process()
{
	if( m_bPlay == false )
	//if( ( m_bPlay == false ) || ( m_bPause == TRUE ) )
		return;

	float m_fCurTime;

	if( m_nOldTick == m_nTick )
	{
		m_fCurTime = (float)m_nTick / 40.f;
		g_BsKernel.RealMovieProcess( m_fCurTime, m_nTick );	// 시간에 맞춰 애니메이션 동작 처리.
		return;
	}

	// 실기 무비 3D 사운드 프로세스.
	RMSndProcess();

	static BOOL bUseBCCam = FALSE;
	m_fCurTime = (float)m_nTick / 40.f;
	m_nOldTick = m_nTick;

	// 1초가 1.0
	CBsRealMovie* pRealMovie = g_BsKernel.GetRealMovie();
	CameraObjHandle CamHandle = CFcBaseObject::GetCameraObjectHandle( 0 );
	
	int nCnt = pRealMovie->PreProcess( m_fCurTime, m_nTick, m_pKey );

	for( int i=0; i<nCnt; ++i )
	{
		switch( m_pKey[i].nType )
		{
		case RMTYPE_LIP :
			{
				// DebugString( "Lip: %s\n", m_pKey[i].szKeyName );
				g_pSoundManager->PlaySound( SB_VOICE_IN_GAME, m_pKey[i].szKeyName );
			}
			break;
		case RMTYPE_WAVE :
			{
				g_pSoundManager->PlaySound( SB_COMMON, m_pKey[i].szKeyName );
			}
			break;
		case RMTYPE_EFFECT_FADEIN :
			{
				DebugString("@ FcRealtimeMovie RMTYPE_EFFECT_FADEIN (%d)\n", (int)m_pKey[i].dwParam2 );
				g_InterfaceManager.GetInstance().SetFadeEffect( true , (int)m_pKey[i].dwParam2 );
			}
			break;
		case RMTYPE_EFFECT_FADEOUT :
			{
				DebugString("@ FcRealtimeMovie RMTYPE_EFFECT_FADEOUT (%d)\n", (int)m_pKey[i].dwParam2 );
				g_InterfaceManager.GetInstance().SetFadeEffect( false , (int)m_pKey[i].dwParam2 );
			}
			break;
		case RMTYPE_EFFECT_BLACK :
			{
				DebugString("@ FcRealtimeMovie RMTYPE_EFFECT_BLACK (%d)\n", (int)m_pKey[i].dwParam2 );
				g_InterfaceManager.GetInstance().SetFadeEffect( false , 0 ); //(int)m_pKey[i].dwParam2 );
			}
			break;
		case RMTYPE_EFFECT_CMD_FX :
			{
				CRMEffect* pEffect = (CRMEffect*)m_pKey[i].dwParam1;
				BsAssert( pEffect && "RM Signal error.");

				if( pEffect->GetParam( 0 ) ) // 1 - Enable , 0 - Disable
				{
                    EnableCmdFX( pEffect->GetPosition().x, 
								pEffect->GetPosition().z,
								pEffect->GetFParam( 0 ),
								pEffect->GetFParam( 1 ),
								pEffect->GetParam( 1 ),
								pEffect->GetParam( 2 ) );
				}
				else
				{
					DisableCmdFX( pEffect->GetParam( 1 ) );
				}
			}
			break;
		case RMTYPE_EFFECT_FX :
			{
				CreateFXForRM( (CRMEffect*)m_pKey[i].dwParam1 );
			}
			break;
		case RMTYPE_EFFECT_USECAM :
			{
				if( CamHandle->IsEnable() )
				{
					if( m_pKey[i].dwParam2 == 1 ) // 이전꺼 사용한다.
					{
						UseCamInfo.bUsePreviousCam = TRUE;
						UseCamInfo.nPreviousSeqIndex = m_nSavedCamSeqIndex;
					}
					else
					{
						UseCamInfo.bUsePreviousCam = FALSE;
						UseCamInfo.nPreviousSeqIndex = 0;
					}

					UseCamInfo.nBaseTick = m_pKey[i].dwParam3;

					m_nSavedCamSeqIndex = CamHandle->StopEventSeq();
				}
				else
				{
					UseCamInfo.nBaseTick = m_pKey[i].dwParam3;
				}
			}
			break;
		case RMTYPE_EFFECT_SETTROOP :
			{
				CRMEffect* pEffect = (CRMEffect*)m_pKey[i].dwParam1;
				stAreaSet* pArea;

				char* pTroopName = pEffect->GetFileName();		// Troop Name
				TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );

				if( hTroop )
				{
					
					pArea = pRealMovie->FindArea( pEffect->GetStrParam() );		// Area Name
					if( pArea )
					{
						hTroop->CmdPlace( pArea->AreaCoord.fSX + ( pArea->AreaCoord.fEX - pArea->AreaCoord.fSX ) / 2,
							pArea->AreaCoord.fSZ + ( pArea->AreaCoord.fEZ - pArea->AreaCoord.fSZ ) / 2,
							(int)m_pKey[i].dwParam4 );
						// pArea->AreaEx.nDir

						hTroop->CmdEnable( m_pKey[i].dwParam5 ? true : false );
					}
					else
					{
						hTroop->CmdEnable( m_pKey[i].dwParam5 ? true : false );
					}
				}
				else
				{
					// BsAssert( hTroop && "Troop is not found. But, You can ignore this error." );
					// 무시 해도 됨. 단순히 어떤 시기에 메시지가 나는지 채크 하기 위함. 2005/12/16
				}
			}
			break;
		case RMTYPE_EFFECT_MOVETROOP :
			{
				CRMEffect* pEffect = (CRMEffect*)m_pKey[i].dwParam1;
				stAreaSet* pArea;

				char* pTroopName = pEffect->GetFileName();		// Troop Name
				TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );
				BsAssert( hTroop && "Troop is not found" );
				pArea = pRealMovie->FindArea( pEffect->GetStrParam() );		// Area Name
				// dwParam5
				if( pArea )
				{
					hTroop->CmdMove( pArea->AreaCoord.fSX + ( pArea->AreaCoord.fEX - pArea->AreaCoord.fSX ) / 2,
									pArea->AreaCoord.fSZ + ( pArea->AreaCoord.fEZ - pArea->AreaCoord.fSZ ) / 2,
									m_pKey[i].dwParam5 ? true : false );
									// 1.f );
				}
			}
			break;
		case RMTYPE_EFFECT_USELIGHT :
			{
				CRMEffect* pEffect = (CRMEffect*)m_pKey[i].dwParam1;
				stLightPack* pLight;

				pLight = pRealMovie->FindLightSet( pEffect->GetFileName() );

				if( pLight )
				{
					g_FcWorld.UpdateLightForRealMovie( &(pLight->DataBlock.crossLight) );
				}
			}
			break;
		case RMTYPE_EFFECT_USEDOF :
			{
				CRMEffect* pEffect = (CRMEffect*)m_pKey[i].dwParam1;

				g_BsKernel.EnableDOF( pEffect->GetParam( 0 ) );
				g_BsKernel.SetDOFFocus( pEffect->GetFParam(0) );
				g_BsKernel.SetDOFFocusNear( pEffect->GetFParam(1) );
				g_BsKernel.SetDOFFocusFar( pEffect->GetFParam(2) );
				// g_BsKernel.SetDOFFocusRange( (float)pEffect->m_dwParam[2] );

				g_BsKernel.SetDOFFocusOutNear( (float)pEffect->GetParam(1) );
				g_BsKernel.SetDOFFocusOutFar( (float)pEffect->GetParam(2) );

			}
			break;
		case RMTYPE_EFFECT_USESOUND :
			{
				if( m_pKey[i].dwParam2 == RMTYPE_LIP )
				{
					g_pSoundManager->PlaySound( SB_VOICE_IN_GAME, m_pKey[i].szKeyName );
				}
				else if( m_pKey[i].dwParam4 ) // g_pSoundManager->PlaySound( SB_COMMON, m_pKey[i].szKeyName );
				{


					//PlayRMSound( m_nTick, i, m_pKey[i].szKeyName, 
					//	(D3DXVECTOR3*)m_pKey[i].dwParam4, 
					//	(CRMObject*)m_pKey[i].dwParam3 );
				}
				else
				{
					g_pSoundManager->PlaySound( SB_COMMON, m_pKey[i].szKeyName );
				}
			}
			break;
		}
	}
	
	CRMEffect* pEffect = pRealMovie->GetCamEffectOnTick( m_nTick );

	if( pEffect )
	{
		CRMCamera* pCam = pRealMovie->GetCamera( pEffect->GetFileName() );
		
		if( pCam )
		{
			// TRACE("Process : CamName : %s\n", pCam->GetName() );
///*	^^^^

			if( pCam->HasBCFile() )
			{
				m_fStartTime = m_fCurTime;
				m_dwStartTick = m_nTick;

				bUseBCCam = TRUE;
				m_pRMBCCam = pCam;
				CamHandle->Enable( true );
				CamHandle->SetBCCam( true );
			}
			else
			{
				bUseBCCam = FALSE;
				m_pRMBCCam = NULL;
				m_fStartTime = 0.f;
				m_dwStartTick = 0;

				FC_CAM_SEQ Seq;
				int nPathCnt = pCam->GetPathCnt();

				for( int j=0; j<nPathCnt; j++ )
				{
					FC_CAM_STATUS Status;
					Status.fFov = pCam->GetPathFOV( j );
					if( Status.fFov < 0 )
						Status.fFov = 0.8f;
						// Status.fFov = 1.1f;
					// DebugString("FC_CAM_STATUS fFov = %f CamName : [%s]\n", Status.fFov, pCam->GetName() );

					Status.EyePos = pCam->GetPathPosition( j );
					Status.TargetPos = pCam->GetPathDir( j );
					int nDuration = pCam->GetPathDuration( j ); // (int)( pCam->GetPathDuration( j ) * 60.f );
					if( nDuration < 0 )
						nDuration = 0;

					Status.nDuration = nDuration; //(int)( pCam->GetPathDuration( j ) * 60.f );
					Status.nRoll = (int)pCam->GetRollValue( j );
					Status.nSwayType = pCam->GetSWayType( j );
					Status.nAccelType = pCam->GetMoveType( j );

					// TRACE("Status.nDuration : %d\n", nDuration );

					Seq.vecStatus.push_back( Status );
				}

				CamHandle->SetRMCamSeq( &Seq );
				CamHandle->SetBCCam( false );

				if( UseCamInfo.bUsePreviousCam )	// 이전꺼 사용한다면?
				{
					// TRACE("UseCamInfo.bUsePreviousCam : %d\n\n", UseCamInfo.bUsePreviousCam );
					CamHandle->SetCurDuration( UseCamInfo.nPreviousSeqIndex+1 );
				}
			} // end of else pCam->HasBCFile()
//*/
		}
	}

	if( bUseBCCam )
	{
		CCrossVector crossCam;
		int nTick = (int)( m_nTick - m_dwStartTick );

		if( nTick >= 0 )
		{
			m_pRMBCCam->ProcessBCCamera( crossCam, nTick );
			CamHandle->SetCrossVector( &crossCam );
		}

	}
	else
	{
		// m_pFcCamera->Process();

	}
	
	g_BsKernel.RealMovieProcess( m_fCurTime , m_nTick );	// 시간에 맞춰 애니메이션 동작 처리.

	// ProcessFX();
	
}

void FcRealtimeMovie::Update()
{
	if( m_bPlay == false )
		return;

	float m_fCurTime = (float)m_nTick / 40.f;

	CBsRealMovie* pRealMovie = g_BsKernel.GetRealMovie();

	CameraObjHandle CamHandle;
	CamHandle = CFcBaseObject::GetCameraObjectHandle( 0 );

	if( pRealMovie )
	{
		//// 키 처리.
		//if( ( CInputPad::GetInstance().GetKeyPressTick(-1, PAD_INPUT_A ) == 1 )  ||
		//	( CInputPad::GetInstance().GetKeyPressTick(-1, PAD_INPUT_START ) == 1 ) )
		//{
		//	TerminateRealMove();
		//}

		if( pRealMovie->PostProcess( m_fCurTime ) == 1 )
		{
			DeleteContents();
			return;
		}

		pRealMovie->Update(m_fCurTime);
	}

	ProcessFX();

	if( !CamHandle->IsFreeCamMode() && !m_bPause )
        m_nTick++;
}

void	FcRealtimeMovie::Clear()
{
	CameraObjHandle CamHandle;
	CamHandle = CFcBaseObject::GetCameraObjectHandle( 0 );

	CBsRealMovie* pRealMovie = g_BsKernel.GetRealMovie();

	//CamHandle->StopEventSeq();
	CamHandle->SetProjectionMatrix(m_fOldNear, m_fOldFar);
	// g_pSoundManager->DeleteWaveBank( pRealMovie->GetBGInfo().GetWaveBnkName() );

	pRealMovie->Clear();
	g_FcWorld.RestoreLight();
	g_FcWorld.SetPlayerEnable( true );
	CInputPad::GetInstance().Break(false);

	g_pSoundManager->RestoreVolCtgy();

	m_bPlay = false;

	if( m_pKey )
	{
		delete [] m_pKey;
		m_pKey = NULL;
	}

	ClearSndPool();

	// g_InterfaceManager.GetInstance().SetFadeEffect( true  , 0 );

	return;
}

void FcRealtimeMovie::ResetAni()
{
	if( m_bPlay == false )
		return;
	CBsRealMovie* pRealMovie = g_BsKernel.GetRealMovie();
	if( pRealMovie )
		pRealMovie->ResetAni();

}

void FcRealtimeMovie::SetPause( BOOL bPause )	
{	
	CBsRealMovie *pRealMovie;

	if( bPause != m_bPause )
	{
		pRealMovie = g_BsKernel.GetRealMovie();
		if( bPause )
		{
			QueryPerformanceCounter( &m_liSaveTime );
			pRealMovie->SetPause( (bPause!=0), m_liSaveTime );
		}
		else
		{
			LARGE_INTEGER liCurTime{};

			if( pRealMovie )
			{
				QueryPerformanceCounter( &liCurTime );
				liCurTime.QuadPart = liCurTime.QuadPart - m_liSaveTime.QuadPart;
				pRealMovie->AddLipTime( liCurTime );
			}
			pRealMovie->SetPause( (bPause!=0), liCurTime );
		}
	}
	m_bPause = bPause;	
}

VOID*	FcRealtimeMovie::LoadPhysicsData( char *szFileName )
{
	return (VOID*)CFcPhysicsLoader::LoadPhysicsData( szFileName );
}

VOID FcRealtimeMovie::ReleasePhysicsData( VOID *pPhysicsInfo )
{
	CFcPhysicsLoader::ClearPhysicsData( (PHYSICS_DATA_CONTAINER*) pPhysicsInfo);
}


VOID* FcRealtimeMovie::LoadCollisionData( char *szSkinFileName )
{
    int nUnitCount = CUnitSOXLoader::GetInstance().GetUnitDataCount();

	PHYSICS_COLLISION_CONTAINER *pCollisionInfo = NULL;

	for( int i = 0; i < nUnitCount; i++) {
		UnitDataInfo *pUnitInfo = CUnitSOXLoader::GetInstance().GetUnitData( i );

		char szFileName[MAX_PATH];
		sprintf( szFileName, "%s1.skin", pUnitInfo->cSkinFileName);
        
		if( _stricmp(szFileName, szSkinFileName) == 0 ) {

			char szInfoFileName[MAX_PATH];
			sprintf( szInfoFileName, "as\\%s", pUnitInfo->cUnitInfoFileName);

			CAniInfoData *pAniInfo = CAniInfoData::LoadAniInfoData( szInfoFileName );

			int nPartsGroupCount = pAniInfo->GetPartsGroupCount();
			for( int i = 0; i < nPartsGroupCount; i++ )
			{
				ASPartsData *pPartsData = pAniInfo->GetPartsGroupInfo( i );

				if( pPartsData->m_nSimulation == PHYSICS_COLLISION) {
					
					const char *pszCollName = pPartsData->GetPartsSkinName( 0 );

					// EV_IN 맵에서 아스파의 천이 전령대의몸을 뚫는버그 수정용 PS ID 6103
					if( _stricmp(pszCollName, "char\\C_LP_KM\\PH_PARTS\\C_LP_KM1_CollisionMesh.txt") == 0 &&
						((_stricmp(g_pFcRealMovie->GetFilename(), "M3\\IN\\EVIE\\EVIE.bsrtm") == 0) || 
						(_stricmp(g_pFcRealMovie->GetFilename(), "M3\\AS\\EVAE\\EVAE.bsrtm") == 0) )
						) 
					{
						g_BsKernel.GetPhysicsMgr()->SetCustomValue( CBsPhysicsMgr::CUSTOM_EV_IN_ASPHARR_CLOTH );
					}
					pCollisionInfo = CFcPhysicsLoader::LoadCollisionMesh( pszCollName );
					break;
				}
			}
			break;
		}		
	}
	
	/*
	
	*/
	return pCollisionInfo;
}

VOID FcRealtimeMovie::ReleaseCollisionData( VOID *pCollisionInfo )
{
	CFcPhysicsLoader::ClearCollisionMesh( (PHYSICS_COLLISION_CONTAINER*) pCollisionInfo);
}

//
//
//
#ifdef _DEBUG
extern BOOL	DoesExistFile(const char *lpszFileName);
#endif

VOID	FcRealtimeMovie::PreLoadFXForRM(CBsRealMovie* pRealMovie)
{
	char szFullPath[512];
	g_BsKernel.chdir( "FX" );

	CBsRealMovie::iteratorRMEffect	itEff = pRealMovie->m_mapEffect.begin();
	CBsRealMovie::iteratorRMEffect	itEffEnd = pRealMovie->m_mapEffect.end();

	for( ; itEff != itEffEnd ; ++itEff )
	{
		if( itEff->second->IsFXType() )
		{
			CRMEffect* pEffect = itEff->second;

#ifdef _DEBUG
			if( !DoesExistFile( pEffect->GetFileName() ) )
			{
				BsAssert( 0 && "RM : FX File not found." );
				pEffect->SetEffectType( eNot );	// 동작 않하게끔 타입을 바꿔버림.
				pEffect->SetFXHandle( -1 );
				continue;
			}
#endif

			if( pEffect->HasFileName() )
			{
				sprintf(szFullPath,"%s%s",g_BsKernel.GetCurrentDirectory(),pEffect->GetFileName());

				int nFXHandle = g_BsKernel.LoadFXTemplate( -1, szFullPath );

				if( nFXHandle == -1 )
				{
					char szTemp[512];
					sprintf( szTemp, "\n\n\n RM FXName : '%s' LoadFXTemplate error.\n\n", szFullPath );
					DebugString( szTemp );
					BsAssert( 0 && "RM : LoadFXTemplate error." );
					pEffect->SetEffectType( eNot );	// 멈추지 않고, 동작 않하게끔 타입을 바꿔버림.
					pEffect->SetFXHandle( -1 );
					continue;
				}

				mapStrIntIterator itFind = m_mapFXHandlePool.find( pEffect->GetFileName() );

				if( itFind == m_mapFXHandlePool.end() ) // 없다 새로 등록.
				{
					m_mapFXHandlePool.insert( pairStrInt( pEffect->GetFileName(), nFXHandle ) );
					// m_mapFXHandlePool[ pEffect->GetFileName() ] = nFXHandle;
					pEffect->SetFXHandle( nFXHandle ); // Effect 에도 핸들 등록.
				}
				else
				{
					// Since we are only tracking a single one of these handles in the map we
					// don't want to keep increasing the reference count.

					g_BsKernel.ReleaseFXTemplate( nFXHandle );

					if( itFind->second != nFXHandle )
					{
						// BsAssert( 0 && "RM : Error FXHandle is different.");
						pEffect->SetEffectType( eNot ); // 멈추지 않고, 동작 않하게끔 타입을 바꿔버림.
						pEffect->SetFXHandle( -1 );
						continue;
					}
					else
					{
						pEffect->SetFXHandle( nFXHandle ); // Effect 에도 핸들 등록.
					}
				}

				if( pEffect->HasConditionFXLink() )
				{
					CRMObject* pRMObj = pRealMovie->GetObject( pEffect->GetLinkedObjName() );

					if( pRMObj )
					{
						pEffect->SetLinkedObjIndex( pRMObj->m_nObjectIndex );
					}
				}
			} // Has File Name
			else
			{
				pEffect->SetEffectType( eNot ); // 멈추지 않고, 동작 않하게끔 타입을 바꿔버림.
				pEffect->SetFXHandle( -1 );
			}
		}
	}

	g_BsKernel.chdir( ".." );
}

BOOL	FcRealtimeMovie::CreateFXForRM( CRMEffect* pEffect )
{
	// 핸들 찾기.
	/*
	mapStrIntIterator itFind = m_mapFXHandlePool.find( pEffect->GetFileName() );
	if( itFind == m_mapFXHandlePool.end() ) // 없다 잘못된 경우.
	{
		BsAssert( 0 && "RM : Not found FX Handle." );
		return FALSE;
	}
	*/

	int nEngineIndex;
	int nFXHandle = pEffect->GetFXHandle(); // itFind->second;

#ifdef _DEBUG
	if( nFXHandle == -1 )
	{
		BsAssert( 0 && "RM : Not found FX Handle." );
		return FALSE;
	}
#else
	if( nFXHandle == -1 )
	{
		return FALSE;
	}
#endif

    nEngineIndex = g_BsKernel.CreateFXObject( nFXHandle );
	
	RM::FXData	Data;

	Data.nObjIndex = pEffect->m_nObjectIndex;
	Data.nIndex = nEngineIndex;
	Data.Mat = *(D3DXMATRIX *)(pEffect->SubData.m_Cross);
	Data.pEffect = (VOID*)pEffect;
	pEffect->SetEngineIndex( nEngineIndex );

	m_vecFXPool.push_back( Data );

	g_BsKernel.SendMessage( nEngineIndex, BS_ENABLE_OBJECT_CULL, TRUE );
	
	if( pEffect->m_nObjectIndex != -1 )
	{
		// Link
		g_BsKernel.SendMessage( 
			pEffect->m_nObjectIndex,
			BS_LINKOBJECT_NAME,
			(DWORD)pEffect->GetStrParam(),
			nEngineIndex );
	}

	g_BsKernel.SetFXObjectState( nEngineIndex, CBsFXObject::PLAY );
	return TRUE;
}

VOID	FcRealtimeMovie::ReleaseFXPool(VOID)
{
	int nCnt = m_vecFXPool.size();

	for( int i=0; i<nCnt; i++ )
	{
		RM::FXData* data = &m_vecFXPool[i];

		if( data->nObjIndex != -1 )
		{
			g_BsKernel.SendMessage( 
				data->nObjIndex,
				BS_UNLINKOBJECT,
				data->nIndex );
#ifdef _USAGE_TOOL_
			g_BsKernel.ProcessKernelCommand();
#endif
		}

		if( data->nIndex != -1 ) {
			g_BsKernel.DeleteObject( data->nIndex );
			data->nIndex = -1;
		}
	}
	m_vecFXPool.clear();
}


VOID	FcRealtimeMovie::ReleaseFXHandle(VOID)
{
	// 핸들 삭제.
	mapStrIntIterator it = m_mapFXHandlePool.begin();
	mapStrIntIterator itEnd = m_mapFXHandlePool.end();
	
	for( ; it != itEnd ; ++it )
	{
		if( it->second != -1 )
            g_BsKernel.ReleaseFXTemplate( it->second );
	}

	m_mapFXHandlePool.clear();
}

VOID	FcRealtimeMovie::ProcessFX(VOID)
{
	vecFXData::iterator it = m_vecFXPool.begin();

	while( it != m_vecFXPool.end() )
	{
		RM::FXData* pData = &(*it);

		if( g_BsKernel.GetFXObjectState( pData->nIndex ) == CBsFXObject::STOP )
		{
			if( pData->nObjIndex != -1 )
			{
				g_BsKernel.SendMessage( 
					pData->nObjIndex,
					BS_UNLINKOBJECT,
					pData->nIndex );
#ifdef _USAGE_TOOL_
				g_BsKernel.ProcessKernelCommand();
#endif
			}

			if( pData->nIndex != -1 )
			{
				g_BsKernel.DeleteObject( pData->nIndex );
				pData->nIndex = -1;
			}
			it = m_vecFXPool.erase( it );
		}
		else
			++it;
	}

	int nCnt = (int)m_vecFXPool.size();
	for( int i = 0 ; i < nCnt ; ++i )
	{
		g_BsKernel.UpdateObject( m_vecFXPool[i].nIndex,
			((CRMEffect*)m_vecFXPool[i].pEffect)->SubData.m_Cross );
	}
}

VOID	FcRealtimeMovie::EnableCmdFX(float fSX,float fSZ,float fWidth,float fHeight,int nID,int nTime)
{
	D3DXVECTOR2	AreaSize = D3DXVECTOR2( fWidth, fHeight );
	D3DXVECTOR2	StartPoint = D3DXVECTOR2( fSX, fSZ );

	char szFile[128];
	sprintf( szFile, "%s", "cloud_011dark.dds" ); // 칠흑의 어둠포그 : cloud_011dark //  일반포그 : cloud_t2(2)

	int iDynamicFogID = g_pFcFXManager->Create(FX_TYPE_DYNAMICFOG);

	if (iDynamicFogID != -1) 
	{
		g_pFcFXManager->SendMessage(iDynamicFogID, 
									FX_INIT_OBJECT, 
									(DWORD)&AreaSize, 
									(DWORD)&StartPoint , 
									(DWORD)szFile);

		g_FcWorld.AddDarkDynamicFog( nID, iDynamicFogID);

		g_pFcFXManager->SendMessage(iDynamicFogID, FX_PLAY_OBJECT, nTime );
	}

	return;
}

VOID	FcRealtimeMovie::DisableCmdFX(int nID)
{
	int iDynamicFogID = g_FcWorld.FindDarkDynamicFog( nID );

	if (iDynamicFogID != -1) 
	{
		g_pFcFXManager->SendMessage( iDynamicFogID, FX_DELETE_OBJECT );
		g_FcWorld.EraseDarkDyanmicFog( nID );
	}

	return;
}

VOID	FcRealtimeMovie::TerminateRealMove(VOID)
{
	CBsRealMovie* pRealMovie = g_BsKernel.GetRealMovie();

	if( pRealMovie && m_bPlay )
	{
		int nCnt = pRealMovie->GetLastEvent( m_pKey );

		for( int i = 0 ; i < nCnt ; ++i )
		{
			switch( m_pKey[i].nType )
			{
			case RMTYPE_EFFECT_SETTROOP :
				{
					RMCmdSetTroop( &m_pKey[i] );
				}
				break;
			}
		}

		g_pSoundManager->SaveVolCtgy();
		g_pSoundManager->SetVolumeAll( 0.f );
		g_pSoundManager->StopSoundRM();

		// g_pSoundManager->EnableAutoRestoreVolCtgy( true );
		pRealMovie->SetTerminateTime( ( (float)m_nTick / 40.f ) );
	}
}

VOID	FcRealtimeMovie::RMCmdSetTroop( msgRMTNotify* pKey )
{
	CBsRealMovie* pRealMovie = g_BsKernel.GetRealMovie();
	CRMEffect* pEffect = (CRMEffect*)pKey->dwParam1;
	stAreaSet* pArea;

	char* pTroopName = pEffect->GetFileName();		// Troop Name
	TroopObjHandle hTroop = g_FcWorld.GetTroopObject( pTroopName );

	if( hTroop )
	{
		pArea = pRealMovie->FindArea( pEffect->GetStrParam() );		// Area Name

		if( pArea )
		{
			hTroop->CmdPlace( pArea->AreaCoord.fSX + ( pArea->AreaCoord.fEX - pArea->AreaCoord.fSX ) / 2,
				pArea->AreaCoord.fSZ + ( pArea->AreaCoord.fEZ - pArea->AreaCoord.fSZ ) / 2,
				(int)pKey->dwParam4 );
			// pArea->AreaEx.nDir

			hTroop->CmdEnable( pKey->dwParam5 ? true : false );
		}
		else
		{
			hTroop->CmdEnable( pKey->dwParam5 ? true : false );
		}
	}
	else
	{
		BsAssert( hTroop && "Troop is not found. But, You can ignore this error." );
		// 무시 해도 됨. 단순히 어떤 시기에 메시지가 나는지 채크 하기 위함. 2005/12/16
	}
}

void	FcRealtimeMovie::ClearSndPool(void)
{
	// m_pSndHandlePool
	for( int i = 0 ; i < SNDPOOL_SIZE ; ++i )
	{
		SAFE_DELETE( m_pSndHandlePool[i] );
	}
}

void	FcRealtimeMovie::InitSndPool(void)
{
	memset( m_pSndHandlePool, 0, sizeof(RM3DSndPack*)*SNDPOOL_SIZE );
}

int		FcRealtimeMovie::FindEmptySndSlotIndex(void)
{
	for( int i = 0 ; i < SNDPOOL_SIZE ; ++i )
	{
		if( m_pSndHandlePool[i] == NULL )
			return i;
	}

	BsAssert( 0 && "RM3DSound : There is not empty slot.");
	return -1;
}

int 	FcRealtimeMovie::SetSndSlot(int nIndex,int nHandle,int nSndIndex,CRMObject* pObjPtr)
{
	if( m_pSndHandlePool[ nIndex ] )
	{
		BsAssert( 0 && "RM3DSound : It's not empty. there is data in the slot.");
		return -1;
	}
#ifdef _DEBUG
	SAFE_DELETE( m_pSndHandlePool[ nIndex ] );
#endif

	m_pSndHandlePool[ nIndex ] = new RM3DSndPack;

	m_pSndHandlePool[ nIndex ]->nHandle = nHandle;
	m_pSndHandlePool[ nIndex ]->nSoundIndex = nSndIndex;
	m_pSndHandlePool[ nIndex ]->pObjPtr = (CRMObject*)pObjPtr;

	return nIndex;
}

void	FcRealtimeMovie::RMSndProcess(void)		// 사운드 처리 부분.
{
	for( int i = 0 ; i < SNDPOOL_SIZE ; ++i )
	{
		if( m_pSndHandlePool[i] )
		{
			if( g_pSoundManager->IsOwner( 
										(HANDLE)m_pSndHandlePool[i]->nHandle, 
                                        m_pSndHandlePool[i]->nSoundIndex ) )	// 사용중이라면.
			{
				if( m_pSndHandlePool[i]->pObjPtr )
				{
					g_pSoundManager->SetEmitterPos( 
						m_pSndHandlePool[i]->nSoundIndex,
						m_pSndHandlePool[i]->pObjPtr->GetAniObjPos() );
				}
			}
			else
			{
				DeleteSndSlot( i );
			}
		}
	}
}

void	FcRealtimeMovie::DeleteSndSlot(int nIndex)
{
	SAFE_DELETE( m_pSndHandlePool[ nIndex ] );
}

#ifndef MAKELONG
#define MAKELONG(a, b)      ((LONG)(((WORD)((DWORD)(a) & 0xffff)) | ((DWORD)((WORD)((DWORD)(b) & 0xffff))) << 16))
#endif

//#define SAVE_LOG
#include "DebugUtil.h"


int		FcRealtimeMovie::PlayRMSound( int nTick, int nCol, char* pCurName,D3DXVECTOR3* pPos,CRMObject* pObjPtr)
{
	if( !m_bPause )
	{
		int nHandle = (int)MAKELONG( nCol, nTick );
		int nSndIndex = g_pSoundManager->Play3DSound( (HANDLE)nHandle, SB_COMMON, pCurName, pPos );
/*
//#ifdef SAVE_LOG
		char szFileName[256];
		
		sprintf( szFileName, "D:\\%s_%d_%d.txt", pCurName, nTick,nCol );
		HANDLE file=CreateFile(szFileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);

		if( file==INVALID_HANDLE_VALUE ) 
		{
			sprintf( szFileName,"Tick: %d Col: %d Pos: %f %f %f\n", nTick,nCol,pPos->x, pPos->y, pPos->z );
			DebugString( szFileName );
			BsAssert(0 && "log파일열기실패");
		}
		else
		{
			sprintf( szFileName,"Pos: %f %f %f", pPos->x, pPos->y, pPos->z );
			WriteFile(file, szFileName , strlen(szFileName), NULL, NULL);
			CloseHandle(file);
		}
//#endif
*/

		int nSlotIndex = FindEmptySndSlotIndex();

		if( nSlotIndex != -1 )
		{
			return SetSndSlot( nSlotIndex, nHandle, nSndIndex, pObjPtr );
		}
	}

	return -1;
}