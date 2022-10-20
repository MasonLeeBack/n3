#include "stdafx.h"
#include "FcEventCamera.h"
#include "BsCamera.h"
#include "bstreamext.h"
#include "FcCommon.h"
#include "DebugUtil.h"
#include "FcUtil.h"
#include "BsUtil.h"
#include "BsFileIO.h"
#include "FcCameraObject.h"


//#include "FcGlobal.h"	// RealMovieTool때문에 사용못함

#ifndef _USE_MOVIETOOL
	#include "FcGlobal.h"
	#include "FcTroopManager.h"
	#include "FcTroopObject.h"
	#include "FcWorld.h"
	#include "FcGameObject.h"
#endif

#define EVENT_CAM_MAX	1000


CAMERATYPE g_CameraType[CAM_ATTACH_NUM] = 
{
	{ CAM_ATTACH_NON,				"Non" },
	{ CAM_ATTACH_LOCAL_CRD,			"LocalCrd" },
	{ CAM_ATTACH_LOCAL_AXIS,		"LocalAxis" },
	{ CAM_ATTACH_PLAYER,			"Player1" },		// "PlyrLclAxis"
	{ CAM_ATTACH_PLAYER2,			"Player2" },		// "PlyrLclAxs2"
	{ CAM_ATTACH_GRDNLDR1_1,		"Guardian1_1" },
	{ CAM_ATTACH_GRDNLDR1_2,		"Guardian1_2" },
	{ CAM_ATTACH_GRDNLDR2_1,		"Guardian2_1" },
	{ CAM_ATTACH_GRDNLDR2_2,		"Guardian2_2" },
	{ CAM_ATTACH_LOCAL_CRD_AREA,	"LclCrdArea" },	// "Air"
	{ CAM_ATTACH_BY_NAME,			"NotSupport4" },	// "Name"
};


CAMSWAYTYPE g_CameraSwayType[CAM_SWAY_NUM] =
{
	{ CAM_SWAY_NON, "SwayNon" },
	{ CAM_SWAY_SWAY1, "Sway1" },
	{ CAM_SWAY_SWAY2, "Sway2" },
	{ CAM_SWAY_SHOCK1, "Shock1" },
	{ CAM_SWAY_SHOCK2, "Shock2" },
};




bool FC_CAM_STATUS::Save( BStream* pStream )
{
	pStream->Write( &EyePos, sizeof(D3DXVECTOR3), 4 );
	pStream->Write( &TargetPos, sizeof(D3DXVECTOR3), 4 );
	pStream->Write( &nDuration, sizeof(int), 4 );
	pStream->Write( &fFov, sizeof(float), 4 );
	pStream->Write( &nTargetType, sizeof(float), 4 );
	pStream->Write( &nDOF, sizeof(float), 4 );
	pStream->Write( &nRoll, sizeof(int), 4 );
	pStream->Write( &nAccelType, sizeof(int), 4 );
	pStream->Write( &nEyeType, sizeof(int), 4 );

	pStream->Write( &Reserved, CAM_STATUS_RESERVED, 4 );
	return true;
}

bool FC_CAM_STATUS::Load( BStream* pStream )
{
	pStream->Read( &EyePos, sizeof(D3DXVECTOR3), 4 );
	pStream->Read( &TargetPos, sizeof(D3DXVECTOR3), 4 );
	pStream->Read( &nDuration, sizeof(int), 4 );
	pStream->Read( &fFov, sizeof(float), 4 );
	pStream->Read( &nTargetType, sizeof(float), 4 );
	pStream->Read( &nDOF, sizeof(float), 4 );
	pStream->Read( &nRoll, sizeof(int), 4 );
	pStream->Read( &nAccelType, sizeof(int), 4 );
	pStream->Read( &nEyeType, sizeof(int), 4 );

	pStream->Seek( CAM_STATUS_RESERVED, BStream::fromNow );
	return true;
}



bool FC_CAM_SEQ::Save( BStream* pStream )
{
	int nNum = vecStatus.size();
	pStream->Write( &nNum, sizeof(int), 4 );

	for( int i=0; i<nNum; i++ )
	{
		FC_CAM_STATUS* pStatus = &(vecStatus[i]);
		pStatus->Save( pStream );
	}
	return true;
}

bool FC_CAM_SEQ::Load( BStream* pStream )
{
	int nNum;
	pStream->Read( &nNum, sizeof(int), 4 );

	vecStatus.clear();
	for( int i=0; i<nNum; i++ )
	{
		FC_CAM_STATUS Status;
		Status.Load( pStream );
		vecStatus.push_back( Status );
	}
	return true;
}



///////////////////////////////////////////////
// CFcCameraChooser

CFcCameraChooser::CFcCameraChooser()
{
	m_Type = CAM_ATTACH_NON;
	m_bCheckLandHeight = true;
}

CFcCameraChooser::~CFcCameraChooser()
{

}


void CFcCameraChooser::Process()
{
/*
	switch( m_Type )
	{
	case CAM_ATTACH_NON:
		break;
	case CAM_ATTACH_PLAYER:
		break;
	case CAM_ATTACH_PLAYER2:
		break;
	case CAM_ATTACH_GUARDIAN:
		break;
	case CAM_ATTACH_PLAYER_AIR:
		break;
	case CAM_ATTACH_BY_NAME:
		break;
	}
*/
}


void CFcCameraChooser::SetTarget( CAM_ATTACH_TYPE Type )
{

}

 


bool CFcCameraChooser::GetCamPos( FC_CAM_STATUS* pStatus, D3DXVECTOR3* pEye, D3DXVECTOR3* pDir, D3DXVECTOR3* pPreEye, D3DXVECTOR3* pPreDir )
{
	CCrossVector EyeCross, TargetCross;

	CAM_TARGET_TYPE EyeType = GetTarget( (CAM_ATTACH_TYPE)pStatus->nEyeType, &EyeCross );
	CAM_TARGET_TYPE TargetType = GetTarget( (CAM_ATTACH_TYPE)pStatus->nTargetType, &TargetCross);

	switch( EyeType )
	{
	case CAM_TARGET_TYPE_ERROR:
		return false;

	case CAM_TARGET_TYPE_NON:
		*pEye = pStatus->EyePos;
		break;

	case CAM_TARGET_TYPE_LOCAL_AXIS:		// Target cross에 따라 eye 바껴야 한다.
		{
			D3DXVECTOR3 Pos;

			D3DXMATRIX Mat = *TargetCross;
			Mat._41 = Mat._42 = Mat._43 = 0.f;

			D3DXVec3TransformCoord( &Pos, &(pStatus->EyePos), &Mat);

			D3DXVECTOR3 TargetPos;
			D3DXVec3TransformCoord( &TargetPos, &(pStatus->TargetPos), &Mat);

			*pEye = Pos + TargetPos + TargetCross.m_PosVector;
			*pDir = ( TargetCross.m_PosVector + pStatus->TargetPos ) - *pEye;

//			*pDir = pStatus->TargetPos - pStatus->EyePos;
//			float fDist = D3DXVec3Length( &(pStatus->EyePos) );
//			*pEye = TargetCross.m_PosVector + pStatus->TargetPos + TargetCross.m_ZVector * -fDist;
//			*pDir = TargetCross.m_PosVector + pStatus->TargetPos - *pEye;

			D3DXVec3Normalize( pDir, pDir );
		}
		return true;

	case CAM_TARGET_TYPE_LOCAL_CRD:
		*pEye = TargetCross.m_PosVector + pStatus->EyePos;
		*pDir = TargetCross.m_PosVector - *pEye;
		
//		*pDir = pStatus->TargetPos - pStatus->EyePos;
		D3DXVec3Normalize( pDir, pDir );
		return true;

	case CAM_TARGET_TYPE_LOCAL_CRD_AREA:
		*pEye = TargetCross.m_PosVector + pStatus->EyePos;
		*pDir = pStatus->TargetPos - pStatus->EyePos;
		D3DXVec3Normalize( pDir, pDir );
		return true;

	case CAM_TARGET_TYPE_NORMAL:
		*pEye = EyeCross.m_PosVector + pStatus->EyePos;
		break;

	default:
		BsAssert(0);
		break;
	}

	switch( TargetType )
	{
	case CAM_TARGET_TYPE_ERROR:
		return false;

	case CAM_TARGET_TYPE_NON:
		*pDir = pStatus->TargetPos;
		break;

	case CAM_TARGET_TYPE_LOCAL_AXIS:		// Target cross에 따라 eye 바껴야 한다.
		*pDir = pStatus->TargetPos;
		return true;

	case CAM_TARGET_TYPE_LOCAL_CRD:
		*pDir = pStatus->TargetPos;
		return true;

	case CAM_TARGET_TYPE_NORMAL:
		*pDir = ( TargetCross.m_PosVector + pStatus->TargetPos ) - *pEye;
		break;

	default:
		BsAssert(0);
		break;
	}


	return true;
}


CAM_TARGET_TYPE CFcCameraChooser::GetTarget( CAM_ATTACH_TYPE Type, CCrossVector* pCross )
{
	switch( Type )
	{
	case CAM_ATTACH_NON:
		return CAM_TARGET_TYPE_NON;

#ifndef _USE_MOVIETOOL

	case CAM_ATTACH_LOCAL_CRD:
		return CAM_TARGET_TYPE_LOCAL_CRD;

	case CAM_ATTACH_LOCAL_AXIS:
		return CAM_TARGET_TYPE_LOCAL_AXIS;

	case CAM_ATTACH_PLAYER:
		{
			GameObjHandle Handle = g_FcWorld.GetHeroHandle();
			if( Handle )
			{
				*pCross = *Handle->GetCrossVector();
//				pCross->m_PosVector.y = g_FcWorld.GetLandHeight( pCross->m_PosVector.x, pCross->m_PosVector.z );
				pCross->m_PosVector.y += Handle->GetUnitHeight();

				return CAM_TARGET_TYPE_NORMAL;
			}
			else
			{
				DebugString( "Can't find player1" );
			}
		}
		return CAM_TARGET_TYPE_ERROR;

	case CAM_ATTACH_PLAYER2:
		{
			// Not use
			BsAssert( 0 );
		}
		return CAM_TARGET_TYPE_ERROR;

	case CAM_ATTACH_GRDNLDR1_1:
		{
			TroopObjHandle Handle = g_FcWorld.GetTroopmanager()->GetGuardianTroop( 0 );
			if( Handle )
			{
				GameObjHandle hLeader = Handle->GetLeader();
				if( hLeader )
				{
					*pCross = *hLeader->GetCrossVector();
					pCross->m_PosVector.y += hLeader->GetUnitHeight();
					return CAM_TARGET_TYPE_NORMAL;
				}
			}
		}
		return CAM_TARGET_TYPE_ERROR;

	case CAM_ATTACH_GRDNLDR1_2:
		{
			TroopObjHandle Handle = g_FcWorld.GetTroopmanager()->GetGuardianTroop( 1 );
			if( Handle )
			{
				GameObjHandle hLeader = Handle->GetLeader();
				if( hLeader )
				{
					*pCross = *hLeader->GetCrossVector();
					pCross->m_PosVector.y += hLeader->GetUnitHeight();
					return CAM_TARGET_TYPE_NORMAL;
				}
			}
		}
		return CAM_TARGET_TYPE_ERROR;

	case CAM_ATTACH_GRDNLDR2_1:
		BsAssert( 0 );		// Not use
		return CAM_TARGET_TYPE_ERROR;

	case CAM_ATTACH_GRDNLDR2_2:
		BsAssert( 0 );		// Not use
		return CAM_TARGET_TYPE_ERROR;

	case CAM_ATTACH_LOCAL_CRD_AREA:
		return CAM_TARGET_TYPE_LOCAL_CRD_AREA;

	case CAM_ATTACH_BY_NAME:
		break;
#endif
	}

	return CAM_TARGET_TYPE_ERROR;
}



///////////////////////////////////////////////
// CFcEventCamera class

CFcEventCamera::CFcEventCamera( CBsCamera* pCamera )
{
	m_bEnable = false;
	//	m_nNumSeq = 0;
	m_pDefaultSeqInfo = NULL;
	m_pUserSeqInfo = NULL;
	m_nCurSeq = 0;
	m_nCurInfo = 0;
	m_nCurDuration = 0;

	m_pCamera = pCamera;

	m_bEditMode = false;

	m_pDefaultSeqInfo = new FC_CAM_SEQ[EVENT_CAM_MAX];
	m_pUserSeqInfo = new FC_CAM_SEQ[EVENT_CAM_MAX];

	m_bTest = false;

	m_pChooser = new CFcCameraChooser();

	m_SwayType = CAM_SWAY_NON;
	m_fSway = 0.0f;
	m_fAccX = 0.0f;
	m_fAccY = 0.0f;
	m_fVelX = 0.0f;
	m_fVelY = 0.0f;

	m_nShockBegin = -1;
	m_nDur = -1;
	m_fShock = 0.0f;
//	m_ShockDir = CAM_SHOCK_X;
	m_fDiffX = 0.0f;
	m_fDiffY = 0.0f;
	m_fDVX = 0.0f;
	m_fDVY = 0.0f;

	m_nProcessTick = 0;

	m_bPause = false;

	m_nCurRoll = 0;
	m_nTargetRoll = 0;
}


CFcEventCamera::~CFcEventCamera()
{
	SAFE_DELETE( m_pChooser );
	SAFE_DELETEA( m_pDefaultSeqInfo );
	SAFE_DELETEA( m_pUserSeqInfo );
}


bool CFcEventCamera::Initialize()
{
	m_nProcessTick = 0;
	return true;
}

void CFcEventCamera::Process()
{
	if( m_bEditMode )
	{
		if( m_bTest )
			ProcessSeqPlay();
	}
	else
		ProcessSeqPlay();

	ProcessShock();
	ProcessSway();
/*
#ifdef _USE_MOVIETOOL
	PostProcess();
//	SetRoll();
#endif
*/

	m_Cross = m_CamCross;
	m_CamCross.m_PosVector = m_Cross.m_PosVector + m_Cross.m_XVector * m_fDiffX + m_Cross.m_YVector * m_fDiffY;
	SetRoll();

	++m_nProcessTick;
}

void CFcEventCamera::PostProcess()
{
	// m_Cross는 효과 주기 전에 정보 가지고 있음
	m_Cross = m_CamCross;
	m_CamCross.m_PosVector = m_Cross.m_PosVector + m_Cross.m_XVector * m_fDiffX + m_Cross.m_YVector * m_fDiffY;

	SetRoll();
}

void CFcEventCamera::Clear()
{
	for(int i=0; i<EVENT_CAM_MAX; i++ )
	{
		m_pDefaultSeqInfo[i].vecStatus.clear();
		m_pUserSeqInfo[i].vecStatus.clear();
	}
}



bool CFcEventCamera::Load( char* pDefaultFileName, char* pUserFileName )
{
	int nDummy;

	SAFE_DELETEA( m_pDefaultSeqInfo );
	SAFE_DELETEA( m_pUserSeqInfo );

	m_pDefaultSeqInfo = new FC_CAM_SEQ[EVENT_CAM_MAX];
	m_pUserSeqInfo = new FC_CAM_SEQ[EVENT_CAM_MAX];

	DWORD dwFileSize;
	VOID *pData;

	if( FAILED(CBsFileIO::LoadFile( pDefaultFileName, &pData, &dwFileSize ) ) )
	{
		DebugString( "Camera file load fail" );
		BsAssert( 0 );
		return false;
	}
	BMemoryStream stream(pData, dwFileSize);;
	stream.Read( &nDummy, sizeof(int), 4 );
	for( int i=0; i<EVENT_CAM_MAX; i++ )
		m_pDefaultSeqInfo[i].Load( &stream );

	CBsFileIO::FreeBuffer(pData);

	if( FAILED(CBsFileIO::LoadFile( pUserFileName, &pData, &dwFileSize ) ) )
	{
		DebugString( "Camera file load fail" );
		BsAssert( 0 );
		return false;
	}
	BMemoryStream stream2(pData, dwFileSize);
	stream2.Read( &nDummy, sizeof(int), 4 );
	for( int i=0; i<EVENT_CAM_MAX; i++ )
		m_pUserSeqInfo[i].Load( &stream2 );

	CBsFileIO::FreeBuffer(pData);

	return true;
}

bool CFcEventCamera::Save( char* pDefaultFileName, char* pUserFileName )
{
	BFileStream stream( pDefaultFileName, BFileStream::create );
	if( stream.Valid() == false )
	{
		BsAssert( 0 && "Camera file save fail" );
		return false;
	}

	int nDummy = 0;
	stream.Write( &nDummy, sizeof(int), 4 );
	for( int i=0; i<EVENT_CAM_MAX; i++ )
		m_pDefaultSeqInfo[i].Save( &stream );

	BFileStream stream2( pUserFileName, BFileStream::create );
	if( stream2.Valid() == false )
	{
		BsAssert( 0 && "Camera file save fail" );
		return false;
	}
	stream2.Write( &nDummy, sizeof(int), 4 );
	for( int i=0; i<EVENT_CAM_MAX; i++ )
		m_pUserSeqInfo[i].Save( &stream2 );

	return true;
}

bool CFcEventCamera::IsSeq( int nSeqIndex )
{
	FC_CAM_SEQ* pSeq = GetSeq( nSeqIndex );
	return ( 0 < (int)pSeq->vecStatus.size() );
}


void CFcEventCamera::PlaySeq( int nSeqIndex )
{
	FC_CAM_SEQ* pSeq = GetSeq( nSeqIndex );
	if( pSeq->vecStatus.size() == 0 )
	{
		DebugString( "Camera isn't ready" );
		return;
	}
	
	m_nCurSeq = nSeqIndex;
	m_nCurInfo = 0;
	m_nCurDuration = 0;
	m_bEnable = true;

	int nStatusCnt = GetSeq(m_nCurSeq)->vecStatus.size();
	if( nStatusCnt > 0 )
	{
		FC_CAM_STATUS* pCurInfo = &( GetSeq(m_nCurSeq)->vecStatus[m_nCurInfo] );
		D3DXVECTOR3 Eye, Dir;
		bool result = m_pChooser->GetCamPos( pCurInfo, &Eye, &Dir );
		BsAssert(result && "Failed to obtain Camera Position"); //aleksger: prefix bug 837: Make sure that variables are initialized
		m_PreEye = Eye;
		m_PreDir = Dir;

		CCrossVector Cross;
		Cross.Reset();
		Cross.m_PosVector = Eye;
		D3DXVECTOR3 UpVec( 0.f, 1.f, 0.f );
		D3DXVECTOR3 TargetPos = Dir + Eye;
		Cross.LookAt( &TargetPos, &UpVec );
		m_CamCross = Cross;
	}
}

// 실기 무비 툴에서 사용
void CFcEventCamera::SetSeq( FC_CAM_SEQ* pSeq )
{
	m_vecExSeq.vecStatus.clear();
	m_vecExSeq = *pSeq;
	m_nCurSeq = -1;
	m_nCurInfo = 0;
	m_nCurDuration = 0;
	m_nTargetRoll = 0;
	m_nCurRoll = 0;
	m_bEnable = true;

	int nStatusCnt = GetSeq(m_nCurSeq)->vecStatus.size();
	if( nStatusCnt > 0 )
	{
		FC_CAM_STATUS* pCurInfo = &( GetSeq(m_nCurSeq)->vecStatus[m_nCurInfo] );
		D3DXVECTOR3 UpVec( 0.f, 1.f, 0.f );

		CCrossVector Cross;
		Cross.m_PosVector = pCurInfo->EyePos;
		D3DXVECTOR3 TargetPos = pCurInfo->TargetPos + pCurInfo->EyePos;
		Cross.LookAt( &TargetPos, &UpVec );
		m_CamCross = Cross;

		m_nCurRoll = pCurInfo->nRoll;
	}
}

void CFcEventCamera::PlayCurSeq()
{
	PlaySeq( m_nCurSeq );
}

int CFcEventCamera::StopSeq()
{
	m_bEnable = false;
	int nDuration = m_nCurDuration;
	m_nCurDuration = 0;
	return nDuration;
}


void CFcEventCamera::SetEditMode( bool bMode )
{
	/*
	if (bMode == true)
	SaveCam();
	else
	RestoreCam();
	*/

	m_bEditMode = bMode;
	m_bTest = false;
}


void CFcEventCamera::AddNewStatus()
{
	FC_CAM_STATUS Status;
	Status.TargetPos = m_CamCross.m_ZVector;
	Status.EyePos = m_CamCross.m_PosVector;

	FC_CAM_SEQ* pSeq = GetSeq( m_nCurSeq );
	pSeq->vecStatus.push_back( Status );
}

bool CFcEventCamera::EraseStatus( int nIndex )
{
	FC_CAM_SEQ* pSeq = GetSeq( m_nCurSeq );

	std::vector<FC_CAM_STATUS>::iterator it = pSeq->vecStatus.begin();
	it = it + nIndex;
	pSeq->vecStatus.erase( it );
	return true;
}

FC_CAM_STATUS* CFcEventCamera::GetCurStatus( int nIndex )
{
	FC_CAM_SEQ* pSeq = GetSeq( m_nCurSeq );

	BsAssert( m_nCurSeq >= 0 && m_nCurSeq < (EVENT_CAM_MAX * 2) );
	BsAssert( nIndex >= 0 && nIndex < (int)pSeq->vecStatus.size() );

	return &(pSeq->vecStatus[nIndex]);
}


int CFcEventCamera::GetNumStatus()
{
	FC_CAM_SEQ* pSeq = GetSeq( m_nCurSeq );
	return pSeq->vecStatus.size();
}

void CFcEventCamera::SetNextSeq( int nOffset )
{
	if( nOffset < 0 )
	{
		m_nCurSeq += nOffset;
		if( m_nCurSeq < 0 )
			m_nCurSeq = 0;
	}
	else if( nOffset > 0 )
	{
		m_nCurSeq += nOffset;
		if( m_nCurSeq >= EVENT_CAM_MAX * 2 )		// Defaut + User
			m_nCurSeq = EVENT_CAM_MAX * 2 - 1;
	}
	else
		BsAssert( 0 );
}

FC_CAM_SEQ* CFcEventCamera::GetSeq( int nIndex )
{
	if( nIndex == -1 )
	{
		return &m_vecExSeq;
	}
	if( nIndex >= EVENT_CAM_MAX )
	{
		BsAssert( nIndex < EVENT_CAM_MAX*2 );
		return &(m_pUserSeqInfo[nIndex-EVENT_CAM_MAX]);
	}
	return &(m_pDefaultSeqInfo[nIndex]);
}


void CFcEventCamera::SetSway( CAM_SWAY_TYPE nType )
{
	if( m_SwayType == nType )
	{
		return;
	}
	m_SwayType = nType;
	switch( m_SwayType )
	{
	case CAM_SWAY_NON:		m_fSway = 0.f;		break;
	case CAM_SWAY_SWAY1:	m_fSway = 7.f;		break;
	case CAM_SWAY_SWAY2:	m_fSway = 300.f;	break;
	case CAM_SWAY_SHOCK1:
		{
			m_fShock = 10.f;
			m_fSway = 1.f;

			m_fDiffY = 0.f;
			m_fDiffX = RandomNumberInRange(0.5f,1.0f);
			if (Random(2) == 0)
				m_fDiffX *= -1.0f;

			float	fNorm;
			fNorm = sqrt(m_fDiffX * m_fDiffX + m_fDiffY * m_fDiffY);
			m_fDiffX = m_fDiffX / fNorm * m_fShock;
			m_fDiffY = m_fDiffY / fNorm * m_fShock;

			m_fDVX = m_fDVY = 0.0f;
		}
		break;
	case CAM_SWAY_SHOCK2:
		{
			m_fShock = 10.f;
			m_fSway = 1.f;

			m_fDiffX = 0.f;
			m_fDiffY = RandomNumberInRange(0.5f,1.0f);
			if (Random(2) == 0)
				m_fDiffY *= -1.0f;

			float	fNorm;
			fNorm = sqrt(m_fDiffX * m_fDiffX + m_fDiffY * m_fDiffY);
			m_fDiffX = m_fDiffX / fNorm * m_fShock;
			m_fDiffY = m_fDiffY / fNorm * m_fShock;

			m_fDVX = m_fDVY = 0.0f;
		}
		break;
	}
}

/*
void CFcEventCamera::Shock(int nTick, float fShock, CamShockDir ShockDir)
{
	m_nShockBegin = m_nProcessTick;
	m_nDur = nTick;
	m_fShock = fShock;
	m_ShockDir = ShockDir;

	m_fDiffX = 0.0f;
	m_fDiffY = 0.0f;

	if (m_ShockDir & CAM_SHOCK_Y)
	{
		m_fDiffY = RandomNumberInRange(0.5f,1.0f);

		if (Random(2) == 0)
			m_fDiffY *= -1.0f;
	}

	if (m_ShockDir & CAM_SHOCK_X)
	{
		m_fDiffX = RandomNumberInRange(0.5f,1.0f);

		if (Random(2) == 0)
			m_fDiffX *= -1.0f;
	}

	float	fNorm;
	fNorm = sqrt(m_fDiffX * m_fDiffX + m_fDiffY * m_fDiffY);
	m_fDiffX = m_fDiffX / fNorm * m_fShock;
	m_fDiffY = m_fDiffY / fNorm * m_fShock;

	m_fDVX = 0.0f;
	m_fDVY = 0.0f;
}
*/

int CFcEventCamera::GetSeqLength( int nIndex )
{
	int nLength = 0;
	FC_CAM_SEQ* pSeq = GetSeq( nIndex );
	int nCnt = pSeq->vecStatus.size();
	for( int i=0; i<nCnt; i++ )
	{
		nLength += pSeq->vecStatus[i].nDuration;
	}
	return nLength;
}

bool CFcEventCamera::GetCurState( int nIndex, int nTick, CCrossVector* pCross, float* pFov, int* pSway, int* pRoll )
{
	FC_CAM_SEQ* pSeq = GetSeq( nIndex );

	int nCurDuration = 0;
	int nCurTick = 0;
	FC_CAM_STATUS* pCurInfo = NULL;
	FC_CAM_STATUS* pNextInfo = NULL;

	int nCnt = pSeq->vecStatus.size();
	for( int i=0; i<nCnt; i++ )
	{
		pCurInfo = &(pSeq->vecStatus[i]);
		if( pCurInfo->nDuration == 0 )		// 0이면 무한
			break;

		nCurTick += pCurInfo->nDuration;
		nCurDuration = nTick - ( nCurTick - pCurInfo->nDuration );
		if( nTick < nCurTick )
		{
			if( i + 1 < nCnt )
				pNextInfo = &(pSeq->vecStatus[i + 1]);

			break;
		}
	}
	if( pCurInfo == NULL )
	{
		return false;
	}

	*pSway = pCurInfo->nSwayType;
	*pRoll = pCurInfo->nRoll;

	if( pNextInfo == NULL )
	{
		// 타입에 따라 처리
		D3DXVECTOR3 Eye, Dir;
		bool result = m_pChooser->GetCamPos( pCurInfo, &Eye, &Dir, &m_PreEye, &m_PreDir );
		BsAssert(result && "Failed to obtain Camera Position"); //aleksger: prefix bug 838: Must make sure that variables are initalized.

		m_PreEye = Eye;
		m_PreDir = Dir;

		pCross->Reset();
		pCross->m_PosVector = Eye;
		D3DXVECTOR3 UpVec( 0.f, 1.f, 0.f );
		D3DXVECTOR3 TargetPos = Dir + Eye;
		pCross->LookAt( &TargetPos, &UpVec );
		*pFov = pCurInfo->fFov;
		*pSway = pCurInfo->nSwayType;

		if( pCurInfo->nDuration == 0 )
			return true;
		
		if( nCurDuration >= pCurInfo->nDuration )
			return false;

		++nCurDuration;
	}
	else
	{
		D3DXVECTOR3 CurEye, CurDir;
		D3DXVECTOR3 NextEye, NextDir;
		m_pChooser->GetCamPos( pCurInfo, &CurEye, &CurDir, NULL, NULL );
		m_pChooser->GetCamPos( pNextInfo, &NextEye, &NextDir, NULL, NULL );

		float fWeight;
		if( pCurInfo->nAccelType == 0 )
		{
			if( pCurInfo->nDuration > 0 )
				fWeight = (float)(nCurDuration + 1) / (float)pCurInfo->nDuration;
			else
				fWeight = 1.f;
		}
		else if( pCurInfo->nAccelType == 1 )		// 가속
		{
			if( pCurInfo->nDuration > 0 )
			{
				float fN = (float)nCurDuration;
				float fM = (float)pCurInfo->nDuration;
				fWeight = ( fN * ( fN + 1 ) ) / ( fM * ( fM + 1 ) );
			}
			else
				fWeight = 1.f;
		}
		else if( pCurInfo->nAccelType == 2 )		// 감속
		{
			if( pCurInfo->nDuration > 0 )
			{
				float fN = (float)( pCurInfo->nDuration - nCurDuration );
				float fM = (float)pCurInfo->nDuration;
				fWeight = ( fN * ( fN + 1 ) ) / ( fM * ( fM + 1 ) );
				fWeight = 1.f - fWeight;
			}
			else
				fWeight = 1.f;
		}
		else if( pCurInfo->nAccelType == 3 )		// Duration만큼 유지하다 다음 status로 순간 이동
		{
			fWeight = 0.f;
			if( pCurInfo->nDuration <= nCurDuration + 1 )
				fWeight = 1.f;
		}

		if( fWeight > 1.f )
			fWeight = 1.f;

		if( fWeight < 0.f )
			fWeight = 0.f;

		// FOV 세팅
		*pFov = pCurInfo->fFov + ( ( pNextInfo->fFov - pCurInfo->fFov ) * fWeight );

		// 위치 보정
		D3DXVec3Lerp( &(pCross->m_PosVector), &CurEye, &NextEye, fWeight );

		// 회전 보정
		D3DXVECTOR3 NewDir;
		D3DXVec3Lerp( &NewDir, &CurDir, &NextDir, fWeight );

		//			D3DXVECTOR3 UpVec( 0.f, 1.f, 0.f );
		// LookAt을 쓰면 m_PosVector가 너무 큰 값이라 dir 다시 구할 때 float 값이 짤려서 방향이 제대로 안나온다.
		//			pCross->LookAt( &NewDir, &UpVec );

		pCross->m_ZVector=NewDir;
		D3DXVec3Normalize(&pCross->m_ZVector, &pCross->m_ZVector);
		pCross->m_YVector=D3DXVECTOR3( 0.f, 1.f, 0.f );
		D3DXVec3Cross(&pCross->m_XVector, &pCross->m_YVector, &pCross->m_ZVector);
		D3DXVec3Normalize(&pCross->m_XVector, &pCross->m_XVector);
		D3DXVec3Cross(&pCross->m_YVector, &pCross->m_ZVector, &pCross->m_XVector);
		D3DXVec3Normalize(&pCross->m_YVector, &pCross->m_YVector);


/*
		// 위치 계산
		D3DXVec3Lerp( &(pCross->m_PosVector), &(pCurInfo->EyePos), &(pNextInfo->EyePos), fWeight );

		// 회전 계산
		// TODO: Target 위치 바꾸는 것 고려해야 한다.
		if( pCurInfo->bUseTarget )
		{
			D3DXVECTOR3 UpVec( 0.f, 1.f, 0.f );
			pCross->LookAt( &(pCurInfo->TargetPos), &UpVec );
		}
		else
		{
			D3DXVECTOR3 CurDir = pCurInfo->TargetPos;
			D3DXVECTOR3 TargetDir = pNextInfo->TargetPos;
			D3DXVECTOR3 NewDir;
			D3DXVec3Lerp( &NewDir, &CurDir, &TargetDir, fWeight );
//			D3DXVec3Normalize( &NewDir, &NewDir );

//			NewDir = NewDir + pCross->m_PosVector;


//			D3DXVECTOR3 UpVec( 0.f, 1.f, 0.f );
// LookAt을 쓰면 m_PosVector가 너무 큰 값이라 dir 다시 구할 때 float 값이 짤려서 방향이 제대로 안나온다.
//			pCross->LookAt( &NewDir, &UpVec );

			pCross->m_ZVector=NewDir;
			D3DXVec3Normalize(&pCross->m_ZVector, &pCross->m_ZVector);
			pCross->m_YVector=D3DXVECTOR3( 0.f, 1.f, 0.f );
			D3DXVec3Cross(&pCross->m_XVector, &pCross->m_YVector, &pCross->m_ZVector);
			D3DXVec3Normalize(&pCross->m_XVector, &pCross->m_XVector);
			D3DXVec3Cross(&pCross->m_YVector, &pCross->m_ZVector, &pCross->m_XVector);
			D3DXVec3Normalize(&pCross->m_YVector, &pCross->m_YVector);
		}
*/
		m_nTargetRoll = (int)((float)pCurInfo->nRoll * (1.f - fWeight)+ (float)pNextInfo->nRoll * fWeight );
	}


	return true;
}

bool CFcEventCamera::SaveCurStatus( int nIndex )
{
	CCrossVector EyeCross, TargetCross;
	FC_CAM_STATUS* pStatus = GetCurStatus( nIndex );

	if( pStatus->nEyeType == CAM_TARGET_TYPE_LOCAL_AXIS && pStatus->nTargetType == CAM_TARGET_TYPE_LOCAL_AXIS  )
	{
		return false;
	}

//	if( pStatus->nEyeType == pStatus->nTargetType )
//		return false;

	CAM_TARGET_TYPE EyeType = m_pChooser->GetTarget( (CAM_ATTACH_TYPE)pStatus->nEyeType, &EyeCross );
	CAM_TARGET_TYPE TargetType = m_pChooser->GetTarget( (CAM_ATTACH_TYPE)pStatus->nTargetType, &TargetCross);

	switch( EyeType )
	{
	case CAM_TARGET_TYPE_ERROR:
		return false;

	case CAM_TARGET_TYPE_NON:
		pStatus->EyePos = GetCross()->m_PosVector;
		break;

	case CAM_TARGET_TYPE_LOCAL_AXIS:		// Target cross에 따라 eye 바껴야 한다.
		{
			D3DXMATRIX Mat, IMat;

			Mat = *TargetCross;
			Mat._41 = Mat._42 = Mat._43 = 0.f;
			D3DXMatrixInverse( &IMat, NULL, &Mat );
			D3DXVECTOR3 Pos = GetCross()->m_PosVector - TargetCross.m_PosVector;
			D3DXVec3TransformCoord( &(pStatus->EyePos), &Pos, &IMat );
			float fDist = D3DXVec3Length( &(pStatus->EyePos) );
			D3DXVECTOR3 TargetPos = GetCross()->m_ZVector * fDist;
//			pStatus->TargetPos = TargetPos + GetCross()->m_PosVector - TargetCross.m_PosVector;
			TargetPos = TargetPos + GetCross()->m_PosVector - TargetCross.m_PosVector;
			D3DXVec3TransformCoord( &(pStatus->TargetPos), &TargetPos, &IMat );
		}
		return true;

	case CAM_TARGET_TYPE_LOCAL_CRD:
	case CAM_TARGET_TYPE_LOCAL_CRD_AREA:
		{
			pStatus->EyePos = GetCross()->m_PosVector - TargetCross.m_PosVector;
			pStatus->TargetPos = D3DXVECTOR3( 0.f, 0.f, 0.f );
/*
			pStatus->EyePos = GetCross()->m_PosVector - TargetCross.m_PosVector;
			float fDist = D3DXVec3Length( &(pStatus->EyePos) );
			D3DXVECTOR3 TargetPos = GetCross()->m_ZVector * fDist;
			pStatus->TargetPos = TargetPos + GetCross()->m_PosVector - TargetCross.m_PosVector;
*/
		}
		return true;

	case CAM_TARGET_TYPE_NORMAL:
		pStatus->EyePos = GetCross()->m_PosVector - EyeCross.m_PosVector;
		break;

	default:
		BsAssert(0);
		break;
	}

	switch( TargetType )
	{
	case CAM_TARGET_TYPE_ERROR:
		return false;

	case CAM_TARGET_TYPE_NON:
	case CAM_TARGET_TYPE_LOCAL_AXIS:
	case CAM_TARGET_TYPE_LOCAL_CRD:
	case CAM_TARGET_TYPE_LOCAL_CRD_AREA:
		pStatus->TargetPos = GetCross()->m_ZVector;
		break;

	case CAM_TARGET_TYPE_NORMAL:
		{
//			pStatus->EyePos = GetCross()->m_PosVector - TargetCross.m_PosVector;
			float fDist = D3DXVec3Length( &(pStatus->EyePos) );
			D3DXVECTOR3 TargetPos = GetCross()->m_ZVector * fDist;
			pStatus->TargetPos = TargetPos + GetCross()->m_PosVector - TargetCross.m_PosVector;
//			pStatus->TargetPos = TargetCross.m_PosVector;
		}
		break;

	default:
		BsAssert(0);
		break;
	}

	return true;
}


bool CFcEventCamera::InitStatus( CAM_ATTACH_TYPE EyeType, CAM_ATTACH_TYPE TargetType, FC_CAM_STATUS* pStatus )
{
	if( EyeType == TargetType ) 
	{
		return false;
	}


	CCrossVector EyeCross;
	CAM_TARGET_TYPE CamEyeType = m_pChooser->GetTarget( EyeType, &EyeCross );

	CCrossVector TargetCross;
	CAM_TARGET_TYPE CamTargetType = m_pChooser->GetTarget( TargetType, &TargetCross );

	// 이런 경우는 없다.
	if( CamTargetType == CAM_TARGET_TYPE_LOCAL_AXIS || 
		CamTargetType == CAM_TARGET_TYPE_LOCAL_CRD ||
		CamTargetType == CAM_TARGET_TYPE_LOCAL_CRD_AREA )
	{
		return false;
	}

	

	switch( CamEyeType )
	{
	case CAM_TARGET_TYPE_ERROR:
		break;
	case CAM_TARGET_TYPE_NON:
		pStatus->EyePos = GetCross()->m_PosVector;
		break;
	case CAM_TARGET_TYPE_LOCAL_AXIS:
	case CAM_TARGET_TYPE_LOCAL_CRD:
	case CAM_TARGET_TYPE_LOCAL_CRD_AREA:
		pStatus->EyePos = GetCross()->m_PosVector - TargetCross.m_PosVector;
		break;
	case CAM_TARGET_TYPE_NORMAL:
		pStatus->EyePos = D3DXVECTOR3( 0.f, 0.f, 0.f );
		break;
	default:
		BsAssert(0);
	}

	switch( CamTargetType )
	{
	case CAM_TARGET_TYPE_ERROR:
		break;
	case CAM_TARGET_TYPE_NON:
		pStatus->TargetPos = GetCross()->m_ZVector;
		break;
	case CAM_TARGET_TYPE_LOCAL_AXIS:
	case CAM_TARGET_TYPE_LOCAL_CRD:
	case CAM_TARGET_TYPE_LOCAL_CRD_AREA:
		pStatus->TargetPos = D3DXVECTOR3( 0.f, 0.f, 1.f );
		break;
	case CAM_TARGET_TYPE_NORMAL:
		pStatus->TargetPos = D3DXVECTOR3( 0.f, 0.f, 0.f );
		break;
	default:
		BsAssert(0);
	}
	return true;
}



bool CFcEventCamera::GetTargetPos( CCrossVector* pCross )
{
	if( m_nCurSeq < 0 || m_nCurInfo < 0 )
		return false;

	FC_CAM_SEQ* pSeq = GetSeq( m_nCurSeq );
	if( (int)pSeq->vecStatus.size() <= m_nCurInfo )
	{
		return false;
	}
	FC_CAM_STATUS* pStatus = &(GetSeq( m_nCurSeq )->vecStatus[m_nCurInfo]);	
	if( m_pChooser->GetTarget( (CAM_ATTACH_TYPE)pStatus->nTargetType, pCross ) == CAM_TARGET_TYPE_ERROR )
		return false;

#ifndef _USAGE_TOOL_
	if( GetProcessTick() % FRAME_PER_SEC == 0 )
		DebugString( "CamTargetPos %d, %d, %d\n", m_nCurSeq, m_nCurInfo, pStatus->nTargetType );
#endif

	pCross->m_PosVector += pStatus->TargetPos;

	return true;
}

bool CFcEventCamera::IsTarget()
{
	if( m_nCurSeq < 0 || m_nCurInfo < 0 )
		return false;

	FC_CAM_SEQ* pSeq = GetSeq( m_nCurSeq );
	if( (int)pSeq->vecStatus.size() <= m_nCurInfo )
		return false;

	FC_CAM_STATUS* pStatus = &(GetSeq( m_nCurSeq )->vecStatus[m_nCurInfo]);	
	if( pStatus->nTargetType == CAM_ATTACH_NON )
		return false;

	return true;
}


bool CFcEventCamera::SetCurDuration( int nTick )
{
	m_nCurDuration = nTick;
	return true;
}

void CFcEventCamera::SetRoll()
{
/*
	int nSize = GetSeq( m_nCurSeq )->vecStatus.size();
	if( m_bEnable == true && nSize > 0 )
	{
		FC_CAM_STATUS* pStatus = &(GetSeq( m_nCurSeq )->vecStatus[m_nCurInfo]);
		m_nTargetRoll = pStatus->nRoll;
	}
*/

/*
//	if( m_nProcessTick %  == 0 )
//	{
		if( m_nCurRoll > m_nTargetRoll )
		{
			--m_nCurRoll;
		}
		if( m_nCurRoll < m_nTargetRoll )
		{
			++m_nCurRoll;
		}
//	}
*/

//	m_CamCross.RotateRoll( m_nCurRoll );
	m_CamCross.RotateRoll( m_nTargetRoll );
}



void CFcEventCamera::ProcessSeqPlay()
{
	if( m_bEnable == false )
		return;

	CCrossVector Cross;
	float fFov = 0.f;
	int nSway = 0;
	int nRoll = 0;
	bool bRet = GetCurState( m_nCurSeq, m_nCurDuration, &Cross, &fFov, &nSway, &nRoll );
	if( bRet == false )
	{
		m_bEnable = false;
		m_nCurDuration = 0;
		return;
	}
	SetSway( (CAM_SWAY_TYPE)nSway );

	CSmartPtr<CFcCameraObject> hCamera = CFcBaseObject::GetCameraObjectHandle( 0 );
	hCamera->SetCamFov(fFov);

	m_CamCross = *Cross;
//	m_nCurRoll = nRoll;

	if( m_bPause == false )
		++m_nCurDuration;
}


void CFcEventCamera::ProcessShock()
{
	int	nCurCounter;
	int	nTick;

	if (m_nShockBegin == -1)
		return;

	nCurCounter = m_nProcessTick;

	nTick = nCurCounter - m_nShockBegin;

	if (m_nDur < nTick)
	{
		m_nShockBegin = -1;
		m_fDiffX = 0.0f;
		m_fDiffY = 0.0f;
		m_fDVX = 0.0f;
		m_fDVY = 0.0f;
	}
	else
	{
		m_fDVX += m_fDiffX * -0.9f;
		m_fDVY += m_fDiffY * -0.9f;

		m_fDVX *= .99f;
		m_fDVY *= .99f;

		m_fDiffX += m_fDVX;
		m_fDiffY += m_fDVY;
	}
}

void CFcEventCamera::ProcessSway()
{
	switch( m_SwayType )
	{
	case CAM_SWAY_NON:			
		m_fDiffX = 0.f;
		m_fDiffY = 0.f;
		break;

	case CAM_SWAY_SWAY1:
	case CAM_SWAY_SWAY2:
		if (m_fSway > 0.0f)
		{
			m_fVelX2 = (RandomNumberInRange(0.0f, 1.0f) - 0.5f) * m_fSway * 10.0f;
			m_fVelY2 = (RandomNumberInRange(0.0f, 1.0f) - 0.5f) * m_fSway * 10.0f;

			m_fVelX += ((m_fVelX2 - m_fVelX) - m_fDiffX * 1.5f ) * .01f;
			m_fVelY += ((m_fVelY2 - m_fVelY) - m_fDiffY * 1.5f ) * .01f;

			m_fVelX *= 0.7f;
			m_fVelY *= 0.7f;

			m_fDiffX += m_fVelX;
			m_fDiffY += m_fVelY;
		}
		break;

	case CAM_SWAY_SHOCK1:
	case CAM_SWAY_SHOCK2:
		m_fDVX += m_fDiffX * -0.9f;
		m_fDVY += m_fDiffY * -0.9f;

		m_fDVX *= .99f;
		m_fDVY *= .99f;

		m_fDiffX += m_fDVX;
		m_fDiffY += m_fDVY;
		break;
	}
}

