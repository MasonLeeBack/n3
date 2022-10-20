#include "stdafx.h"

#include "FcWorld.h"
#include "FcTroopManager.h"
#include "FcProjectile.h"
#include "FcUtil.h"
#include "FcGlobal.h"
#include "DebugUtil.h"


#include "BsCamera.h"
#include "BsKernel.h"
#include "GenericCamera.h"
#include "BsFileIO.h"
#include "FcWorld.h"



#include "GenericCameraAttach.h"
#include "Data/Camera/GenericCamDef.h"

#include "FcHeroObject.h"
#include "FcTroopObject.h"

#include "FcRealtimeMovie.h"


#define	_CAM_FOREST_MODE_CHANGE_ENDURANCE		150

#define	_CAM_VIEW2_OFFSET		(250 * nCamViewMode)

#ifndef	PI_2
#define PI_2					1.57079632679489661923f
#endif	// !PI_2

#define	CAM_SEQ_FILE_HEADER_SIZE	20

#define	CAM_SEQ_VER13_STR		"KUF2CamSeqVer1.30"
#define	CAM_SEQ_VER14_STR		"GenericCamVer1.40"
#define	CAM_SEQ_VER20_STR		"GenericCamVer2.00"

#define	FREE_CAM_A_MAX			0.8f			// 고개를 좌우로 돌려볼 수 있는 최대 alpha ( pie 기준)
#define	FREE_CAM_B_MIN			-0.35f			// 고개를 위로 들 수 있는 최대 alpha (pie/2 기준)
#define	FREE_CAM_B_MAX			0.25f			// 내려다 볼 수 있는 최대 각도의 alpha (pie/2 기준)
#define	FREE_CAM_B_MAX2			0.60f

#define	DEV_CAM_FILE_NAME		"_DevCam_.$$$"

#define CAM_DIST_NEAR			400.f
#define CAM_DIST_FAR			600.f

#define DEBUG_CAM_DIST_NEAR			1200.f
#define DEBUG_CAM_DIST_FAR			3000.f

#define	CAM_TAKE_BACK_THE_CONTROL	60
#define	CAM_FREECAM_B_LOCK			30

void CriticalException()
{
	BsAssert(0);
}


CGenericCamera::CGenericCamera( CBsCamera* pCamera )
{
	m_pCamera = pCamera;

	memset(&m_Status, 0, sizeof(CAMSTATUS));
	memset(&m_SavedStatus, 0, sizeof(CAMSTATUS));

	m_nSeqList = 0;
	m_pSeqList = NULL;
	m_nLastTime = 0;

	m_nSavedCurSeq = -1;
	m_nSavedCurSeqCamId = -1;

	m_nMarkedTick = -1;
	m_nTickPlayBegan = -1;
	m_nDuration = 0;
	m_nRandom0 = -1;
	m_nRandom1 = -1;

	m_bEyeUniformVel = false;
	m_bTargetUniformVel = false;

	m_bSaved = false;

	m_nCurSeq = 0;
	m_nCurSeqCamId = 0;

	m_nShakeCount = 0;

	m_fFreeCamDA = 0.f;
	m_fFreeCamDB = 0.f;
	m_fFreeCamB = 0.0f;

	m_bFreeCam = false;
	m_nActionFreeCamReset = AFCAM_NORMAL;

	m_bTargetChanged = true;
	m_bCamDirectMode = false;
	
	m_bForceSeq = false;
	m_bEventSeq = false;

	m_bEditMode	 = false;
	m_bNoSway = false;
	m_nDoingNothingCount = 0;
	m_bDevMode = false;

	m_bEyeInForest = false;
	m_bTargetInForest = false;

	m_bUserTarget = false;

	m_SeqFileName[0] = '\x0';
	m_SeqFileName2[0] = '\x0';
	m_UserSeqFileName[0] = '\x0';

	m_nIsInForestCounter = -1;
	m_bIsCamForestModePrev = false;
	m_bIsCamForestMode = false;

	m_bIsMicAttachedOnPlayer = false;
	m_fAlign = 0.0f;
	m_fScreenAlign = 0.0f;

	m_CursorPos = D3DXVECTOR2(0.0f,0.0f);
	m_bCursorPosChanged = false;

	m_vActionEye = D3DXVECTOR2(0.0f,0.0f);

	m_fActionPlayerFowardScale = 0.6f;
	m_fActionPlayerFowardSpeedScale = 0.1f;

	m_nLastNearestEnemy = -1;

	m_nDefaultActionCam = FCCAM_INPHYY_DEFAULT;

	m_bIsHazeOn = false;

	m_bPlayerSet = false;

	m_MarkedPoint = D3DXVECTOR2(0.0f,0.0f);

	m_nInternalTick = 0;
	m_bAbilityPosValid = false;
	m_nPlaceInWorldMap = -1;
	m_nLatestAbilityIdx = -1;
	m_nPrevTargeted = -1;

	m_bVirgin = true;

	m_nCurViewMode = 0;	
	m_bCamPitchLimit = true;
	m_bUserCamErr = false;

	m_CamPos = D3DXVECTOR3(0.f, 0.f, -500.f);
	m_CamTarget = D3DXVECTOR3(0.f, 0.f, 0.f); 

	m_CamLastPos = D3DXVECTOR3(0.0f,0.0f,0.0f);
	m_CamLastTarget = D3DXVECTOR3(0.0f,0.0f,0.0f);

	m_fFreeCamTargetAlt	= 0.0f;
	m_fFreeCamDist		= 0.0f;

	m_FreeCamPos = D3DXVECTOR3(0.f, 0.f, 0.f);
	m_FreeCamTarget = D3DXVECTOR3(0.f, 0.f, 0.f);

	m_CamCurPos = D3DXVECTOR3(0.f, 0.f, -500.f);
	m_CamCurTarget = D3DXVECTOR3(0.f, 0.f, 0.f);

	m_CamCurLocalPos = D3DXVECTOR3(0.f, 0.f, -500.f);
	m_CamCurLocalTarget = D3DXVECTOR3(0.f, 0.f, 0.f);

	m_nStepPos			= 0;
	m_nStepTarget		= 0;

	m_bCamPitchLimit = true;

	m_fFov				= 1.0f;
	m_fCurFov			= 1.0f;

	m_nCamRoll = 0;
	m_nCamCurRoll = 0;

	m_fDOFNear = g_BsKernel.GetDOFFocusNear();
	m_fDOFFar = g_BsKernel.GetDOFFocusFar();

	m_nPrevEye = -1;
	m_nPrevTarget = -1;

	m_pProjectile = NULL;

	m_bDOF = false;

	m_HandCamVel = D3DXVECTOR3(0.f, 0.f, 0.f);
	m_HandCamRelCrd = D3DXVECTOR3(0.f, 0.f, 0.f);

	if( m_bDevMode == false )
		m_fCamDist = CAM_DIST_NEAR;
	else
		m_fCamDist = DEBUG_CAM_DIST_NEAR;

	m_nOrbCamTick = 0;
	m_nActCamCounter = 0;
	m_nFreeCamMoveCount = 0;
	m_bDefaultCamVar = false;
	m_fDirCosPrev = 0.0f;
	m_fDirSinPrev = 0.0f;
	m_bModifyCamHeightByLand = false;
	m_SwayPos = D3DXVECTOR2(0.0f,0.0f);
	m_SwayVel = D3DXVECTOR2(0.0f,0.0f);

	m_fActionEyeHMod = 0.0f;
	m_nFreeCamBLockCount = 0;

	m_b3DShock = false;
	m_v3DShockCenter = D3DXVECTOR3(0.0f,0.0f,0.0f);
	m_f3DShockX = 0.0f;
	m_f3DShockY = 0.0f;
	m_f3DShockRadius = 0.0f;
	m_n3DShockBegin = 0;
	m_n3DShockDur = 0;
	m_fShockPushX = 0.0f;
	m_fShockPushY = 0.0f;

	m_nSpecialCam = -1;
	m_nSpecialCamBegin = 0;
	m_nSpecialCamDur = 0;

	m_bCollisionEnable = true;
}


///////////////////////////////////////////////////////////////////////////////////////
CGenericCamera::~CGenericCamera()
{
	int	nScan;

	for(nScan = 0;nScan < m_nSeqList;nScan++)
	{
		if (m_pSeqList[nScan].nSize > 0)
		{
			m_pSeqList[nScan].nSize = 0;
			free(m_pSeqList[nScan].pCam);
		}
	}

	if (m_nSeqList > 0)
		free (m_pSeqList);

	m_nSeqList = 0;
	m_pSeqList = NULL;
}

///////////////////////////////////////////////////////////////////////////////////////

void	CGenericCamera::SetEditMode(bool bOpt)
{
	m_bEditMode = bOpt;

	if( m_bEditMode == false )
	{
		m_fFreeCamDA = 0.0f;
		m_fFreeCamDB = 0.0f;
		SeqPlay( m_nCurSeq );
	}
}

bool	CGenericCamera::IsEditMode(void)
{
	return m_bEditMode;
}


///////////////////////////////////////////////////////////////////////////////////////
D3DXVECTOR2	CGenericCamera::GetTargetPos()
{
	D3DXVECTOR2	rValue(0.0f,0.0f);

	if (m_Target != NULL)
	{
		GameObjHandle Unit = m_Target->GetLeader();

		rValue.x=Unit->GetPos().x;
		rValue.y=Unit->GetPos().z;
	}


	return rValue;
}


D3DXVECTOR3	CGenericCamera::GetTargetPosV3()
{
	D3DXVECTOR3	rValue(0.f,0.f,0.f);
	if (m_Target != NULL)
	{
		GameObjHandle Unit = m_Target->GetLeader();

		rValue.x=Unit->GetPos().x;
		rValue.y=Unit->GetPos().y;
		rValue.z=Unit->GetPos().z;
	}
	return rValue;
}


void CGenericCamera::ToggleCamDist()
{
	m_bDefaultCamVar = !m_bDefaultCamVar;
}

void CGenericCamera::SetToggleCamDist(bool bToggle) 
{
	m_bDefaultCamVar = bToggle;
}
bool CGenericCamera::IsToggleCamDist() const 
{
	return m_bDefaultCamVar;
}

void  CGenericCamera::MovePlayBack( int nDelayTick )
{
	GameObjHandle hUnit = g_FcWorld.GetHeroHandle();
	if( hUnit )
	{
		D3DXVECTOR3	v3UnitCrd=D3DXVECTOR3(0.f,0.f,0.f);
		D3DXVECTOR3	v3Dir;
		GetUnitCrdNDir( hUnit, &v3UnitCrd, &v3Dir);
		D3DXVECTOR2	v2Dir( v3Dir.x, v3Dir.z );

		m_nFreeCamMoveCount = nDelayTick;

		D3DXVECTOR2 CrossDir( -m_vActionEye.y, m_vActionEye.x );
		float fDirDot = D3DXVec2Dot( &CrossDir, &v2Dir );

		float fDot = D3DXVec2Dot( &v2Dir, &m_vActionEye );
//		m_fFreeCamDA = acos( fDot ) / (float)m_nFreeCamMoveCount;
		if( fDot < 0.999f )		// fDot가 1.0f일때 acos값이 깨질때가 있다.
		{
			if( fDirDot < 0 )
				m_fFreeCamDA = acos( fDot );
			else
				m_fFreeCamDA = -acos( fDot );
		}
	}	
}


///////////////////////////////////////////////////////////////////////////////////////
void	CGenericCamera::SetDevMode(bool bOpt)
{
	m_bDevMode = bOpt;	

	if( m_bDevMode == false )
		m_fCamDist = CAM_DIST_NEAR;
	else
		m_fCamDist = DEBUG_CAM_DIST_NEAR;
}

bool	CGenericCamera::IsDevMode(void)
{
	return m_bDevMode;
}

///////////////////////////////////////////////////////////////////////////////////////
void CGenericCamera::Process()
{
	D3DXVECTOR2	TargetPos;
	TroopObjHandle Target;
	TroopObjHandle CursorIsOn;
	D3DXVECTOR2	v2Cursor;
	D3DXVECTOR2	v2Diff1, v2Diff2;

	m_TargetUnit.Identity();
	m_bIsMicAttachedOnPlayer = false;
	m_bCamPitchLimit = true;
	m_bModifyCamHeightByLand = false;

	if (m_bVirgin)
		CopyCamStatus();

	TroopObjHandle PlayerTroop = g_FcWorld.GetHeroHandle()->GetTroop();
	BsAssert( PlayerTroop );
	
	if (m_bUserTarget == true && m_UserTarget != NULL)
		Target = m_UserTarget;
	else
		Target = g_FcWorld.GetHeroHandle()->GetTroop();

	if( Target )
	{
		if(Target != m_Target)
		{
			m_bTargetChanged = true;
			m_Target = Target;
		}
		else
			m_bTargetChanged = false;
	}

	if (m_bCamDirectMode == false)
	{
		if (m_bEventSeq == false)
		{
			if (m_Target != NULL)

				Chooser();
		}
		ProcessSeq();
	}

	if (m_nDuration > 0 && GetTick() - m_nTickPlayBegan > m_nDuration)
		SeqStop();

	ViewProcess();

	D3DXVECTOR3 camPosV3 = D3DXVECTOR3(m_CamCross.m_PosVector.x, m_CamCross.m_PosVector.y, m_CamCross.m_PosVector.z );
	D3DXVECTOR3 camDirV3 = D3DXVECTOR3(m_CamCross.m_ZVector.x, m_CamCross.m_ZVector.y, m_CamCross.m_ZVector.z );

	D3DXVECTOR2 camPos( camPosV3.x, camPosV3.z );
	D3DXVECTOR2 camDir( camDirV3.x, camDirV3.z );

	float fDist = D3DXVec2LengthSq( &camDir );
	if( fDist == 0.f )
		camDir = D3DXVECTOR2( 1.f, 0.f );
	else
		D3DXVec2Normalize( &camDir, &camDir );

	m_nInternalTick++;
	m_bVirgin = false;
}



void CGenericCamera::ViewProcess()
{
	D3DXVECTOR3		Err;
	D3DXVECTOR3		CamPos, CamTarget;
	CAMSTATUS		*pCam;

	CamProcessPreset();

	CorrectCam();
	CamPos = m_CamPos;
	CamTarget = m_CamTarget;


	pCam = GetCamStatus();

	// ?: Make cam move smoothly
	Err = CamPos - m_CamCurPos;

	if ( D3DXVec3Length(&Err) > 0.0f && m_nStepPos > 0 && pCam->bEyeWorldSmooth == true && IsTargetChanged() == false && IsVirgin() == false)
		m_CamCurPos += Err / (float)m_nStepPos;
	else
		m_CamCurPos = CamPos;

	Err = CamTarget - m_CamCurTarget;
	if ( D3DXVec3Length(&Err) > 0.0f && m_nStepTarget > 0 && pCam->bTargetWorldSmooth == true && IsTargetChanged() == false  && IsVirgin() == false)
		m_CamCurTarget += Err / (float)m_nStepTarget;
	else
		m_CamCurTarget = CamTarget;

	float		fCamDist;
	D3DXVECTOR3	CamDir;

	CamDir = m_CamCurTarget - m_CamCurPos;
	fCamDist = D3DXVec3Length(&CamDir);
	CamDir /= fCamDist;

	if( IsCamPitchLimited() == true)	// 액션 모드에서 카메라가 땅을 보지 않도록 보정합니다.
	{	
		if (CamDir.y < -0.6f)
		{
			CamDir.y = -0.6f;
			CamDir *= fCamDist;

			m_CamCurTarget = m_CamCurPos + CamDir;
		}
	}


	if (m_nStepTarget > 0 && IsTargetChanged() == false  && IsVirgin() == false)
	{
		m_fCurFov += (m_fFov - m_fCurFov) / (float)m_nStepTarget;
		m_nCamCurRoll += (int)((float)(m_nCamRoll - m_nCamCurRoll) / (float) m_nStepTarget);
	}
	else
	{
		m_fCurFov = m_fFov;
		m_nCamCurRoll = m_nCamRoll;
	}

	// Set Dof Focus
	if(!g_pFcRealMovie || !g_pFcRealMovie->IsPlay())
	{
		g_BsKernel.SetDOFFocus(fCamDist);

		// Make dof change smoothly
		float fErr = m_fDOFNear - g_BsKernel.GetDOFFocusNear();

		if (m_nStepPos > 0 && IsTargetChanged() == false  && IsVirgin() == false)
		{
			float fNear = g_BsKernel.GetDOFFocusNear();
			g_BsKernel.SetDOFFocusNear( fNear + fErr / (float) m_nStepPos );
		}
		else
			g_BsKernel.SetDOFFocusNear( m_fDOFNear );

		fErr = m_fDOFFar - g_BsKernel.GetDOFFocusFar();;

		if (m_nStepPos > 0 && IsTargetChanged() == false  && IsVirgin() == false)
		{
			float fFar = g_BsKernel.GetDOFFocusFar();
			g_BsKernel.SetDOFFocusFar( fFar + fErr / (float) m_nStepPos );
		}
		else
			g_BsKernel.SetDOFFocusFar( m_fDOFFar );


		g_BsKernel.SetDOFFocusOutFarDef(); //Default
		g_BsKernel.SetDOFFocusOutNearDef();
	}


	CamCollision(&m_CamCurPos, &m_CamCurTarget);

	// update m_CamCross
	m_CamCross.m_PosVector = m_CamCurPos;
	m_CamCross.m_ZVector   = m_CamCurTarget - m_CamCurPos;
	D3DXVec3Normalize(&m_CamCross.m_ZVector, &m_CamCross.m_ZVector);

	CamProcessSway();
	CamProcessShock3D();


	if (D3DXVec3Length(&m_CamCross.m_ZVector) <= 0.0f)
	m_CamCross.m_ZVector.y = -1.0f;

	m_CamCross.m_YVector = D3DXVECTOR3(0.0f,1.0f,0.0f);
	m_CamCross.UpdateVectors();

	m_CamCross.RotateRoll(m_nCamCurRoll);


	// change sound listening position
//	g_pSound->SetListenerPosDir( m_CamCross.m_PosVector, m_CamCross.m_ZVector );

}

///////////////////////////////////////////////////////////////////////////////////////
void CGenericCamera::GetCamDir(D3DXVECTOR2 *vDir)
{
	D3DXVECTOR3	v3Dir;
	D3DXVECTOR2	v2;

	if (vDir != NULL)
	{
		v3Dir = D3DXVECTOR3(m_CamCross.m_ZVector.x, m_CamCross.m_ZVector.y, m_CamCross.m_ZVector.z );

		v2.x = v3Dir.x;
		v2.y = v3Dir.z;

		float	length = D3DXVec2Length( &v2 );

		if (length > 0.0f)
		{
			vDir->x = v2.x / length;
			vDir->y = v2.y / length;
		}
		else
		{
			vDir->x = 1.0f;
			vDir->y = 0.0f;
		}
	}
}

void	CGenericCamera::GetCamPos3(D3DXVECTOR3 *vPos)
{
	D3DXVECTOR3	v3Pos;

	if (vPos != NULL)
	{
		v3Pos = D3DXVECTOR3( m_CamCross.m_PosVector.x, m_CamCross.m_PosVector.y, m_CamCross.m_PosVector.z );

		vPos->x = v3Pos.x;
		vPos->y = v3Pos.z;
		vPos->z = v3Pos.y;
	}
}

void	CGenericCamera::GetCamPos2(D3DXVECTOR2 *v2Pos)
{
	D3DXVECTOR3	v3Pos;

	if (v2Pos != NULL)
	{
		v3Pos = D3DXVECTOR3( m_CamCross.m_PosVector.x, m_CamCross.m_PosVector.y, m_CamCross.m_PosVector.z );

		v2Pos->x = v3Pos.x;
		v2Pos->y = v3Pos.z;
	}
}


void	CGenericCamera::GetCamDir3(D3DXVECTOR3 *vDir)
{
	D3DXVECTOR3	v3Dir;

	if (vDir != NULL)
	{
		v3Dir = D3DXVECTOR3( m_CamCross.m_ZVector.x, m_CamCross.m_ZVector.y, m_CamCross.m_ZVector.z );

		vDir->x = v3Dir.x;
		vDir->y = v3Dir.z;
		vDir->z = v3Dir.y;

		D3DXVec3Normalize( vDir, vDir );
	}
}

float CGenericCamera::GetCamFov()
{
	return m_pCamera->GetFOVByProcess();
}


CAMSTATUS*	CGenericCamera::GetCamStatus(void)
{
	return &m_Status;
}

void	CGenericCamera::NewSeq(int nId)
{
	CamSeq			*pTemp;

	if (nId >= 0)
	{

		if (m_nSeqList <= nId)
		{

			pTemp = (CamSeq*) malloc(sizeof(CamSeq) * (nId + 1));

			memset(pTemp,0,sizeof(CamSeq)* (nId+1));

			if (m_nSeqList > 0)
			{
				memcpy(pTemp, m_pSeqList, sizeof(CamSeq) * m_nSeqList);
				free(m_pSeqList);
			}

			m_pSeqList = pTemp;
			m_nSeqList = nId + 1;
		}

		if (m_pSeqList[nId].nSize != 0)
			free (m_pSeqList[nId].pCam);

		m_pSeqList[nId].nSize = 0;
		m_pSeqList[nId].pCam = NULL;
	}
}

void	CGenericCamera::DelSeq(int nId)
{
	if (nId >= 0 && nId < m_nSeqList)
	{
		if (m_pSeqList[nId].nSize > 0)
		{
			m_pSeqList[nId].nSize = 0;
			free (m_pSeqList[nId].pCam);
		}

		m_pSeqList[nId].nSize = 0;
		m_pSeqList[nId].pCam = NULL;
	}
	else if (nId == -1)
	{
		int nScan;

		for(nScan = 0;nScan < m_nSeqList;nScan++)
		{
			if (m_pSeqList[nScan].nSize > 0)
			{
				m_pSeqList[nScan].nSize = 0;
				free (m_pSeqList[nScan].pCam);
			}

			m_pSeqList[nScan].nSize = 0;
			m_pSeqList[nScan].pCam = NULL;
		}

		if (m_nSeqList > 0)
			free (m_pSeqList);

		m_nSeqList = 0;
		m_pSeqList = NULL;
	}
}


int		CGenericCamera::SeqAddCam(int nSeqId, CAMSTATUS *pStatus)
{
	CamSeq	*pSeq;
	int		nRValue;

	nRValue = -1;

	if (m_nSeqList > 0 && nSeqId >= 0 && nSeqId < m_nSeqList)
	{
		pSeq = &(m_pSeqList[nSeqId]);

		pSeq->nSize ++;
		pSeq->pCam = (CAMSTATUS*) realloc(pSeq->pCam, sizeof(CAMSTATUS) * pSeq->nSize);
		memcpy(&(pSeq->pCam[pSeq->nSize-1]),pStatus, sizeof(CAMSTATUS));

		nRValue = pSeq->nSize - 1;
	}

	return nRValue;
}

int		CGenericCamera::SeqDelCam(int nSeqId, int nIndex)
{
	CamSeq	*pSeq;
	int		nRValue;
	int		nScan;

	nRValue = -1;

	if (m_nSeqList > 0 && nSeqId >= 0 && nSeqId < m_nSeqList)
	{
		pSeq = &(m_pSeqList[nSeqId]);

		if (pSeq->nSize > 0)
		{
			if (nIndex >= 0 && nIndex < pSeq->nSize)
			{
				for(nScan = nIndex + 1; nScan < pSeq->nSize; nScan++)
					memcpy(&pSeq->pCam[nScan-1],&pSeq->pCam[nScan],sizeof(CAMSTATUS));
			}

			pSeq->nSize --;

			if (pSeq->nSize == 0)
			{
				free (pSeq->pCam);
				pSeq->pCam = NULL;
			}
			else
				pSeq->pCam = (CAMSTATUS*) realloc(pSeq->pCam, sizeof(CAMSTATUS) * pSeq->nSize);
		}

		nRValue = pSeq->nSize;

	}

	return nRValue;
}

int		CGenericCamera::SeqDelCam(int nSeqId)
{
	CamSeq	*pSeq;
	int		nRValue;

	nRValue = -1;

	if (m_nSeqList > 0 && nSeqId >= 0 && nSeqId < m_nSeqList)
	{
		pSeq = &(m_pSeqList[nSeqId]);

		if (pSeq->nSize > 0)
		{
			pSeq->nSize --;

			if (pSeq->nSize == 0)
			{
				free (pSeq->pCam);
				pSeq->pCam = NULL;
			}
			else
				pSeq->pCam = (CAMSTATUS*) realloc(pSeq->pCam, sizeof(CAMSTATUS) * pSeq->nSize);
		}

		nRValue = pSeq->nSize;

	}

	return nRValue;
}

int				CGenericCamera::SeqGetCurSeqId(void)
{
	return	m_nCurSeq;
}


int				CGenericCamera::SeqGetCurCamId(void)
{
	return m_nCurSeqCamId;	
}

CAMSTATUS*	CGenericCamera::SeqGetCam(int nSeqId, int nIndex)
{
	CAMSTATUS *pRValue;
	
	pRValue = NULL;
	
	if (m_nSeqList > 0 && nSeqId >= 0 && nSeqId < m_nSeqList)
	{
		if (nIndex >= 0 && nIndex < m_pSeqList[nSeqId].nSize)
			pRValue = &(m_pSeqList[nSeqId].pCam[nIndex]);
	}


	if (pRValue == NULL)
		pRValue = &m_Status;

	return pRValue;
}

int		CGenericCamera::SeqGetSize(int nSeqId)
{
	int	nRValue;

	if (m_nSeqList > 0 && nSeqId >= 0 && nSeqId < m_nSeqList)
		nRValue = m_pSeqList[nSeqId].nSize;
	else
		nRValue = 0;

	return nRValue;
}

int		CGenericCamera::SeqGetLength(int nSeqId)
{
	int	nScan;
	int	nLen = 0;

	if (nSeqId >= 0 && nSeqId < m_nSeqList)
	{
		for(nScan = 0;nScan < m_pSeqList[nSeqId].nSize;nScan++)
			nLen += m_pSeqList[nSeqId].pCam[nScan].nDuration;
	}

	return nLen;
}

void	CGenericCamera::PushCam(void)
{
	if (m_bSaved == false)
	{
		memcpy(&m_SavedStatus, &m_Status, sizeof(CAMSTATUS));

		m_nSavedCurSeq = m_nCurSeq;
		m_nSavedCurSeqCamId = m_nCurSeqCamId;

		m_bSaved = true;
	}
}

void	CGenericCamera::PopCam(void)
{
	if (m_bSaved == true)
	{
		memcpy(&m_Status, &m_SavedStatus, sizeof(CAMSTATUS));

		SeqPlay(m_nSavedCurSeq);
		m_nCurSeq = m_nSavedCurSeq;
		m_nCurSeqCamId = m_nSavedCurSeqCamId;

		m_bSaved = false;
		m_bVirgin = true;
	}
}


void	CGenericCamera::SeqPlay(int nSeqId)
{
	if (SeqGetSize(nSeqId) > 0 && m_bForceSeq == false)
	{
		m_nDuration = 0;

		m_nCurSeq = nSeqId;	
		m_nCurSeqCamId = 0;

		CopyCamStatus();
	}
}


void	CGenericCamera::SeqStop(void)
{
	m_nCurSeq = 0;
	m_nCurSeqCamId = 0;
	m_nMarkedTick = 0;

	PopCam();

	if (m_bEventSeq == true)
	{
		m_bForceSeq = false;
		m_bEventSeq = false;
		Chooser();
	}
}

bool	CGenericCamera::IsSeqEnded(void)
{
	if (m_nCurSeq != -1 && SeqGetSize(m_nCurSeq) > 0)
		return (m_nCurSeqCamId >= SeqGetSize(m_nCurSeq) - 1 && 
				GetTick() - m_nMarkedTick > m_Status.nDuration);

	return false;
}

void	CGenericCamera::ProcessSeq(void)
{
	CAMSTATUS *pCamStatus;


	if (m_nCurSeq != -1)
	{
		if (m_bEyeUniformVel == true)
			m_Status.nEyeSmooth--;

		if (m_Status.nEyeSmooth < 0)
			m_Status.nEyeSmooth = 0;

		if (m_bTargetUniformVel == true)
			m_Status.nTargetSmooth--;

		if (m_Status.nTargetSmooth < 0)
			m_Status.nTargetSmooth = 0;


		if (SeqGetSize(m_nCurSeq) > 0)
		{
			if (GetTick() - m_nMarkedTick > m_Status.nDuration)
			{
				if (m_nCurSeqCamId < SeqGetSize(m_nCurSeq) - 1)
				{
					m_nCurSeqCamId++;
					CopyCamStatus();
				}
			}
		}
		else
		{
			SeqStop();
		}

		pCamStatus = SeqGetCam(m_nCurSeq, m_nCurSeqCamId);
	}
}

void	CGenericCamera::SeqLoadVer13(int nOffset, int nSize, BMemoryStream *pBS)
{
	int				nScan;
	CamSeq			*pList;
	int				nListSize;
	int				nListEnd;

	struct CAMSTATUS14
	{
		
		int					nEyeSmooth;
		int					nTargetSmooth;
		int					nDuration;

		int					nEye;
		int					nTarget;

		float				EyeX;
		float				EyeY;
		float				EyeZ;

		float				TargetX;		// Target Target
		float				TargetY;
		float				TargetZ;

		float				fFov;

		bool				bShock;
		bool				bNoSpring;
		int					nShockTerm;
		float				fShockVal;
		float				fSpringConst;

		int					nDOF;		// 0: false, 1: true, 2: default
		float				fDOFNear;
		float				fDOFFar;

		bool				bFocusing;
		bool				bIsWaterSway;
		bool				bApplyAlign;
		bool				bScreenEffect;

		int					nRoll;		// roll (angle)

		bool				bEyeWorldSmooth;	// world smooth가 켜져 있으면 step의 smooth계산이 world에 대해 행해진다.
		bool				bTargetWorldSmooth;
		unsigned char		reserved[30];

};

	pBS->Read(&nListSize,sizeof(int), 4);

	if (nListSize > nSize)
		nListSize = nSize;

	if (nListSize > 0)
	{
		pList = (CamSeq*)malloc(sizeof(CamSeq) * nListSize);
		memset(pList, 0, sizeof(CamSeq) * nListSize);

		for(nScan = 0;nScan < nListSize;nScan++)
		{
			pBS->Read(&pList[nScan].nSize,sizeof(int), 4);

			if( pList[nScan].nSize > 0 )
			{
				pList[nScan].pCam = (CAMSTATUS*) malloc(sizeof(CAMSTATUS) * pList[nScan].nSize);
				for(int i=0; i<pList[nScan].nSize; i++)
				{
					CAMSTATUS* D = &(pList[nScan].pCam[i]);
					CAMSTATUS14  C14;

					pBS->Read( &(C14.nEyeSmooth), sizeof(int), 4 );
					pBS->Read( &(C14.nTargetSmooth), sizeof(int), 4 );
					pBS->Read( &(C14.nDuration), sizeof(int), 4 );
					pBS->Read( &(C14.nEye), sizeof(int), 4 );
					pBS->Read( &(C14.nTarget), sizeof(int), 4 );
					pBS->Read( &(C14.EyeX), sizeof(float), 4 );
					pBS->Read( &(C14.EyeY), sizeof(float), 4 );
					pBS->Read( &(C14.EyeZ), sizeof(float), 4 );
					pBS->Read( &(C14.TargetX), sizeof(float), 4 );
					pBS->Read( &(C14.TargetY), sizeof(float), 4 );
					pBS->Read( &(C14.TargetZ), sizeof(float), 4 );

					pBS->Read( &(C14.fFov), sizeof(float), 4 );

					pBS->Read( &(C14.bShock), sizeof(bool) );
					pBS->Read( &(C14.bNoSpring), sizeof(bool) );

					pBS->Read( &(C14.nShockTerm), sizeof(int), 4 );

					pBS->Read( &(C14.fShockVal), sizeof(float), 4 );
					pBS->Read( &(C14.fSpringConst), sizeof(float) );

					pBS->Read( &(C14.nDOF), sizeof(int), 4 );
					pBS->Read( &(C14.fDOFNear), sizeof(float), 4 );
					pBS->Read( &(C14.fDOFFar), sizeof(float), 4 );

					pBS->Read( &(C14.bFocusing), sizeof(bool) );
					pBS->Read( &(C14.bIsWaterSway), sizeof(bool) );
					pBS->Read( &(C14.bApplyAlign), sizeof(bool) );
					pBS->Read( &(C14.bScreenEffect), sizeof(bool) );

					pBS->Read( &(C14.nRoll), sizeof(int), 4 );

					pBS->Read( &(C14.bEyeWorldSmooth), sizeof(bool), 1);
					pBS->Read( &(C14.bTargetWorldSmooth), sizeof(bool), 1);
					pBS->Read( &(C14.reserved), sizeof(unsigned char) * 30 );

					D->nEyeSmooth = C14.nEyeSmooth;
					D->nTargetSmooth = C14.nTargetSmooth;
					D->nDuration = C14.nDuration;

					D->nEye = C14.nEye;
					D->nTarget = C14.nTarget;

					D->EyeX = C14.EyeX;
					D->EyeY = C14.EyeY;
					D->EyeZ = C14.EyeZ;

					D->TargetX = C14.TargetX;		
					D->TargetY = C14.TargetY;
					D->TargetZ = C14.TargetZ;

					D->fFov = C14.fFov;

					D->nSway = 0;
					D->nShock = 0;

					D->nDOF = C14.nDOF;		
					D->fDOFNear = C14.fDOFNear;
					D->fDOFFar = C14.fDOFFar;

					D->bFocusing = C14.bFocusing;
					D->nParam1 = 0;
					D->bScreenEffect = C14.bScreenEffect;

					D->nRoll = C14.nRoll;		

					D->bEyeWorldSmooth = C14.bEyeWorldSmooth;	
					D->bTargetWorldSmooth = C14.bTargetWorldSmooth;
					
					memset(D->reserved,0,32);
				}
			}
		}

		nListEnd = nOffset + nSize;

		if (nListEnd > m_nSeqList)
		{
			m_pSeqList = (CamSeq*) realloc(m_pSeqList, sizeof(CamSeq) * nListEnd);
			memset(&(m_pSeqList[m_nSeqList]),0,sizeof(CamSeq) * (nListEnd - m_nSeqList));

			m_nSeqList = nListEnd;
		}

		for(nScan = nOffset; nScan < nListEnd; nScan++)
		{
			if (m_pSeqList[nScan].pCam != NULL)
			{
				free (m_pSeqList[nScan].pCam);
				m_pSeqList[nScan].nSize = 0;
				m_pSeqList[nScan].pCam = NULL;
			}
		}

		memcpy(&(m_pSeqList[nOffset]), pList, sizeof(CamSeq) * nListSize);

		free (pList);
	}
}

void	CGenericCamera::SeqLoadVer20(int nOffset, int nSize, BMemoryStream *pBS)
{
	int				nScan;
	CamSeq			*pList;
	int				nListSize;
	int				nListEnd;

	pBS->Read(&nListSize,sizeof(int), 4);

	if (nListSize > nSize)
		nListSize = nSize;

	if (nListSize > 0)
	{
		pList = (CamSeq*)malloc(sizeof(CamSeq) * nListSize);
		memset(pList, 0, sizeof(CamSeq) * nListSize);

		for(nScan = 0;nScan < nListSize;nScan++)
		{
			pBS->Read(&pList[nScan].nSize,sizeof(int), 4);

			if( pList[nScan].nSize > 0 )
			{
				pList[nScan].pCam = (CAMSTATUS*) malloc(sizeof(CAMSTATUS) * pList[nScan].nSize);
				for(int i=0; i<pList[nScan].nSize; i++)
				{
					CAMSTATUS* D = &(pList[nScan].pCam[i]);

					pBS->Read( &(D->nEyeSmooth), sizeof(int), 4 );
					pBS->Read( &(D->nTargetSmooth), sizeof(int), 4 );
					pBS->Read( &(D->nDuration), sizeof(int), 4 );
					pBS->Read( &(D->nEye), sizeof(int), 4 );
					pBS->Read( &(D->nTarget), sizeof(int), 4 );
					pBS->Read( &(D->EyeX), sizeof(float), 4 );
					pBS->Read( &(D->EyeY), sizeof(float), 4 );
					pBS->Read( &(D->EyeZ), sizeof(float), 4 );
					pBS->Read( &(D->TargetX), sizeof(float), 4 );
					pBS->Read( &(D->TargetY), sizeof(float), 4 );
					pBS->Read( &(D->TargetZ), sizeof(float), 4 );

					pBS->Read( &(D->fFov), sizeof(float), 4 );

					pBS->Read( &(D->nSway), sizeof(int), 4 );
					pBS->Read( &(D->nShock), sizeof(int), 4 );

					pBS->Read( &(D->nDOF), sizeof(int), 4 );
					pBS->Read( &(D->fDOFNear), sizeof(float), 4 );
					pBS->Read( &(D->fDOFFar), sizeof(float), 4 );

					pBS->Read( &(D->bFocusing), sizeof(bool) );
					pBS->Read( &(D->nParam1), sizeof(char) );
					pBS->Read( &(D->bScreenEffect), sizeof(bool) );

					pBS->Read( &(D->nRoll), sizeof(int), 4 );

					pBS->Read( &(D->bEyeWorldSmooth), sizeof(bool), 1);
					pBS->Read( &(D->bTargetWorldSmooth), sizeof(bool), 1);
					pBS->Read( &(D->reserved), sizeof(unsigned char) * 32 );

				}
			}
		}

		nListEnd = nOffset + nSize;

		if (nListEnd > m_nSeqList)
		{
			m_pSeqList = (CamSeq*) realloc(m_pSeqList, sizeof(CamSeq) * nListEnd);
			memset(&(m_pSeqList[m_nSeqList]),0,sizeof(CamSeq) * (nListEnd - m_nSeqList));

			m_nSeqList = nListEnd;
		}

		for(nScan = nOffset; nScan < nListEnd; nScan++)
		{
			if (m_pSeqList[nScan].pCam != NULL)
			{
				free (m_pSeqList[nScan].pCam);
				m_pSeqList[nScan].nSize = 0;
				m_pSeqList[nScan].pCam = NULL;
			}
		}

		memcpy(&(m_pSeqList[nOffset]), pList, sizeof(CamSeq) * nListSize);

		free (pList);
	}
}

bool	CGenericCamera::_SeqLoad(int nOffset, int nSize, char *pFileName)
{
	char			FileNameWithPath[MAX_PATH];
	char			CamSeqVer[CAM_SEQ_FILE_HEADER_SIZE];
	void *			pData;
	DWORD			dwFileSize;

	g_BsKernel.chdir("Camera");
	g_BsKernel.GetCurrentDirectory(MAX_PATH,FileNameWithPath);
	strcat(FileNameWithPath,pFileName);
	g_BsKernel.chdir("..");

	g_BsResChecker.AddResList( FileNameWithPath );

	if( !FAILED( CBsFileIO::LoadFile( FileNameWithPath, &pData, &dwFileSize ) ) )
	{
		BMemoryStream BMS(pData, dwFileSize);

		BMS.Read(&CamSeqVer,CAM_SEQ_FILE_HEADER_SIZE);


		if (!strcmp(CamSeqVer,CAM_SEQ_VER13_STR) || !strcmp(CamSeqVer,CAM_SEQ_VER14_STR))
			SeqLoadVer13(nOffset, nSize, &BMS);
		else if (!strcmp(CamSeqVer,CAM_SEQ_VER20_STR))
			SeqLoadVer20(nOffset, nSize, &BMS);
		else
			BsAssert ("Corrupted camera file." && 0);

		CBsFileIO::FreeBuffer(pData);
	}
	else {
		BsAssert( "Corrupted camera file" && 0 );
		return false;
	}

	SeqStop();

	return true;
}


void	CGenericCamera::_SeqSave(int nOffSet, int nSize, char *pFileName)
{
	int				nScan;
	char			FileNameWithPath[MAX_PATH];
	char			CamSeqVer[CAM_SEQ_FILE_HEADER_SIZE];
	int				nListSize;

	g_BsKernel.chdir("Camera");
	g_BsKernel.GetCurrentDirectory(MAX_PATH,FileNameWithPath);
	strcat(FileNameWithPath,pFileName);
	g_BsKernel.chdir("..");

	BFileStream		Stream( FileNameWithPath, BFileStream::create );
	if( Stream.Valid() )
	{
		strcpy(CamSeqVer,CAM_SEQ_VER20_STR);
		Stream.Write( CamSeqVer,CAM_SEQ_FILE_HEADER_SIZE );

		if (m_nSeqList <= nOffSet)
			nListSize = 0;
		else
		{
			if (nSize >= m_nSeqList - nOffSet)
				nListSize = m_nSeqList - nOffSet;
			else
				nListSize = nSize;
		}

		Stream.Write( &nListSize,sizeof(int), 4 );
		if (nListSize > 0)
		{
			for(nScan = nOffSet;nScan < nOffSet + nListSize;nScan++)
			{
				Stream.Write( &m_pSeqList[nScan].nSize,sizeof(int), 4 );
				if (m_pSeqList[nScan].nSize > 0)
				{
//					Stream.Write( m_pSeqList[nScan].pCam, sizeof(CAMSTATUS) * m_pSeqList[nScan].nSize );
					for( int i=0; i<m_pSeqList[nScan].nSize; i++ )
					{
						CAMSTATUS* D = &(m_pSeqList[nScan].pCam[i]);
						Stream.Write( &(D->nEyeSmooth), sizeof(int), 4 );
						Stream.Write( &(D->nTargetSmooth), sizeof(int), 4 );
						Stream.Write( &(D->nDuration), sizeof(int), 4 );
						Stream.Write( &(D->nEye), sizeof(int), 4 );
						Stream.Write( &(D->nTarget), sizeof(int), 4 );
						Stream.Write( &(D->EyeX), sizeof(float), 4 );
						Stream.Write( &(D->EyeY), sizeof(float), 4 );
						Stream.Write( &(D->EyeZ), sizeof(float), 4 );
						Stream.Write( &(D->TargetX), sizeof(float), 4 );
						Stream.Write( &(D->TargetY), sizeof(float), 4 );
						Stream.Write( &(D->TargetZ), sizeof(float), 4 );

						Stream.Write( &(D->fFov), sizeof(float), 4 );

						Stream.Write( &(D->nSway), sizeof(int), 4 );
						Stream.Write( &(D->nShock), sizeof(int), 4 );

						Stream.Write( &(D->nDOF), sizeof(int), 4 );
						Stream.Write( &(D->fDOFNear), sizeof(float), 4 );
						Stream.Write( &(D->fDOFFar), sizeof(float), 4 );

						Stream.Write( &(D->bFocusing), sizeof(bool) );
						Stream.Write( &(D->nParam1), sizeof(char) );
						Stream.Write( &(D->bScreenEffect), sizeof(bool) );

						Stream.Write( &(D->nRoll), sizeof(int), 4 );

						Stream.Write( &(D->bEyeWorldSmooth), sizeof(bool), 1);
						Stream.Write( &(D->bTargetWorldSmooth), sizeof(bool), 1);
						
						Stream.Write( &(D->reserved), sizeof(unsigned char) * 32 );
					}
				}
			}
		}
	}
	else {
		BsAssert( "_SeqSave실패 %s\n" && 0 );
//		CriticalException("_SeqSave실패 %s\n", FileNameWithPath);
	}
}

void	CGenericCamera::SeqSave(void)
{
	if (m_SeqFileName[0] != '\x0')
		_SeqSave(0,100,m_SeqFileName);

	if (m_SeqFileName2[0] != '\x0')
		_SeqSave(200,100,m_SeqFileName2);
}

void	CGenericCamera::SeqUserSave(void)
{
	if (m_UserSeqFileName[0] != '\x0')
		_SeqSave(100,100,m_UserSeqFileName);
}

bool	CGenericCamera::SeqLoad(char *pFileName)
{
	char *pPtr;
	strncpy(m_SeqFileName,pFileName,MAX_PATH);
	if(_SeqLoad(0,100,pFileName)==false) {
		return false;
	}
	strncpy(m_SeqFileName2,pFileName,MAX_PATH);

	pPtr = strrchr(m_SeqFileName2,'.');
	
	if (!pPtr)
	{
		BsAssert(pPtr); //aleksger: prefix bug 660: Check that the period was found.
		return false;
	}

	*(pPtr) = '_';
	*(pPtr+1) = '.';
	*(pPtr+2) = 'c';
	*(pPtr+3) = 'a';
	*(pPtr+4) = 'm';
	*(pPtr+5) = '\x0';

	if(_SeqLoad(200,100,m_SeqFileName2)==false) {
		return false;
	}
/*
	if(_SeqLoad(99,1,DEV_CAM_FILE_NAME)==false) {
		return false;
	}
*/
	return true;
}

bool	CGenericCamera::SeqUserLoad(char *pFileName)
{
 	strncpy(m_UserSeqFileName,pFileName,MAX_PATH);
	return _SeqLoad(100,100,pFileName);
}

void CGenericCamera::MoveFreeCam(float fA, float fB)
{
	m_fFreeCamDA = fA;
	m_fFreeCamB += fB;


	if (m_fFreeCamB < FREE_CAM_B_MIN)
		m_fFreeCamB = FREE_CAM_B_MIN;

	if (fB < 0.0f && m_fFreeCamB <= FREE_CAM_B_MAX)
		m_nFreeCamBLockCount = 0;

	if (m_fFreeCamB > FREE_CAM_B_MAX && m_nFreeCamBLockCount < CAM_FREECAM_B_LOCK)
	{
		m_fFreeCamB = FREE_CAM_B_MAX;
		m_nFreeCamBLockCount ++;
	}

	if (m_fFreeCamB > FREE_CAM_B_MAX2)
		m_fFreeCamB = FREE_CAM_B_MAX2;

	m_nFreeCamMoveCount += 2;
	if (m_nFreeCamMoveCount >= CAM_TAKE_BACK_THE_CONTROL)
		m_nFreeCamMoveCount = CAM_TAKE_BACK_THE_CONTROL;
}

void CGenericCamera::EnableFreeCam(bool bOpt)
{
	m_bFreeCam = bOpt;

	if (bOpt == false)
		m_nActionFreeCamReset = AFCAM_NORMAL;
}

bool	CGenericCamera::IsTargetChanged(void)
{
	return m_bTargetChanged;
}


void	CGenericCamera::CopyCamStatus(void)
{
	memcpy(&m_Status,SeqGetCam(m_nCurSeq, m_nCurSeqCamId), sizeof(CAMSTATUS));

	if (m_Status.nEyeSmooth < 0)
	{
		m_Status.nEyeSmooth *= -1;
		m_bEyeUniformVel = true;
	}
	else
		m_bEyeUniformVel = false;

	if (m_Status.nTargetSmooth < 0)
	{
		m_Status.nTargetSmooth *= -1;
		m_bTargetUniformVel = true;
	}
	else
		m_bTargetUniformVel = false;

	m_nMarkedTick = GetTick();
}


void	CGenericCamera::SetDirectMode(bool bOpt)
{
	m_bCamDirectMode = bOpt;
}

void	CGenericCamera::SetUserSeq(int nSeqId, int nDuration, bool bForce)
{
	PushCam();

	m_nTickPlayBegan = GetTick();

	if (bForce == true)		// 만약 현재 camera가 force 상태인데 또 force로 들어오면 새로 들어온 카메라로 셋팅한다.
		m_bForceSeq = false;

	SeqPlay(nSeqId);
	m_nDuration = nDuration;

	m_bForceSeq = bForce;
	m_bEventSeq = true;
}


int CGenericCamera::GetTick()
{
	return m_nInternalTick;
}

void	CGenericCamera::SetUserTarget(TroopObjHandle pTarget)
{
	if (pTarget != NULL)
		m_bUserTarget = true;

	m_UserTarget = pTarget;
}

void	CGenericCamera::UnsetUserTarget(void)
{
	m_bUserTarget = false;
	m_UserTarget.Identity();
}

const	char*	CGenericCamera::GetSeqFileName(void)
{
	return m_SeqFileName;
}

const	char*	CGenericCamera::GetSeqFileName2(void)
{
	return m_SeqFileName2;
}

const	char*	CGenericCamera::GetUserSeqFileName(void)
{
	return m_UserSeqFileName;	
}

bool	CGenericCamera::IsMicAttachedOnPlayer(D3DXVECTOR3 *pPos, D3DXVECTOR2 *pDir)
{
	BsAssert(pPos != NULL && pDir != NULL);
	if (m_bIsMicAttachedOnPlayer)
	{
		GameObjHandle Unit = g_FcWorld.GetHeroHandle();
		(*pPos) = Unit->GetPos();
		D3DXVECTOR3 TempDir = Unit->GetCrossVector()->m_ZVector;
		(*pDir) = D3DXVECTOR2( TempDir.x, TempDir.z );
	}
	return m_bIsMicAttachedOnPlayer;
}

float	CGenericCamera::GetScreenAlign(void)
{
	return m_fScreenAlign;
}

void	CGenericCamera::SetScreenAlign(float fAlign)
{
	m_fScreenAlign = fAlign;
}

GameObjHandle CGenericCamera::GetCamTarget(void)
{
	GameObjHandle RValue;
	if(m_Target)
	{
		if(m_Target->IsPlayerTroop())
			RValue = m_Target->GetLeader();
	}
	return RValue;
}

void	CGenericCamera::GetCursorPos(D3DXVECTOR2 *pPos)
{
	*pPos = m_CursorPos;
}

void	CGenericCamera::GetTroopPos(D3DXVECTOR2 *pPos)
{
	D3DXVECTOR3	Pos,Dir;

	if (m_Target != NULL)
	{

		GetTroopActualCrdNDir(m_Target, &Pos, &Dir);

		pPos->x = Pos.x;
		pPos->y = Pos.z;
	}
}

void	CGenericCamera::SetMarkedTroop(TroopObjHandle Troop)
{
	m_MarkedTroop = Troop;
}

void	CGenericCamera::SetMarkedPoint(D3DXVECTOR2* pPoint)
{
	m_MarkedPoint = *pPoint;
}


/////////// utilities /////////////////////////////////////////////////////////////////

bool	CGenericCamera::IsCollisionEnabled(void)
{
	return m_bCollisionEnable;
}

void	CGenericCamera::EnableCollision(bool bOpt)
{
	m_bCollisionEnable = bOpt;
}


TroopObjHandle CGenericCamera::GetPlayerAirTroop()
{
	int			nTroop;
	int			nScan;
	TroopObjHandle Troop;
	TroopObjHandle Nearest;
	float		fNearestSq;
	D3DXVECTOR2		Pos1,Pos2;

	fNearestSq = FLT_MAX;

	nTroop = g_FcWorld.GetTroopmanager()->GetTroopCount();
	Pos1 = g_FcWorld.GetHeroHandle()->GetTroop()->GetPosV2();

	for( nScan = 0;nScan < nTroop;nScan++ )
	{
		Troop = g_FcWorld.GetTroopObject( nScan );
		Pos2 = Troop->GetPosV2();
		D3DXVECTOR2 Dir = Pos1 - Pos2;
		float fDistSq = D3DXVec2LengthSq( &Dir );

		if( Troop->GetUnitCount() == 0 )
			continue;

		if( Troop->GetUnit(0)->GetClassID() == CFcGameObject::Class_ID_Fly && Troop->GetHP() > 0.0f && fDistSq < fNearestSq ) {
			Nearest = Troop;
			fNearestSq = fDistSq;
		}
	}
	if( nScan >= nTroop )
		Troop.Identity();

	return Nearest;
}

void	CGenericCamera::GetTroopCrdNDir(TroopObjHandle Troop, D3DXVECTOR3 *pPos, D3DXVECTOR3 *pDir)
{
	D3DXVECTOR2	v2;

	if( Troop )
	{
		v2 = Troop->GetPosV2();
		pPos->x = v2.x;
		pPos->z = v2.y;
//		pPos->y = pTroop->GetAltitude();
		pPos->y = 0.f;
		v2 = Troop->GetDir();
		pDir->x = v2.x;
		pDir->z = v2.y;
		pDir->y = 0.0f;
	}
}


void	CGenericCamera::GetUnitCrdNDir(GameObjHandle Unit, D3DXVECTOR3 *pPos, D3DXVECTOR3 *pDir)
{
	D3DXVECTOR2	v2;
	D3DXVECTOR3	v3;

	if(Unit)
	{
		if (pPos)
			*pPos = Unit->GetPos();

		if (pDir)
		{
			*pDir = Unit->GetCrossVector()->m_ZVector;
			pDir->y = 0.0f;

			D3DXVec3Normalize( pDir, pDir );
		}
	}
	else 
		BsAssert(!"Unable to get position and direction. Improper values"); //aleksger: prefix bug 662: Leaving variables in unknown state.
}

void	CGenericCamera::GetTroopActualCrdNDir(TroopObjHandle Troop, D3DXVECTOR3 *pPos, D3DXVECTOR3 *pDir)
{
	D3DXVECTOR2	Dir;

	if (Troop->GetHP() <= 0.0f || Troop->IsEnable() == false)
	{
		GetTroopCrdNDir( Troop,pPos,pDir );
	}
	else
	{
		D3DXVECTOR2 averagePos( Troop->GetAverageAliveUnitPos() );
		pPos->x = averagePos.x;
		pPos->z = averagePos.y;

		bool bProp;
		pPos->y = g_FcWorld.GetLandHeight( pPos->x, pPos->z, &bProp );

		Dir = Troop->GetDir();
		pDir->x = Dir.x;
		pDir->y = 0.0f;
		pDir->z = Dir.y;
	}
}

void	CGenericCamera::ChangeView(void)
{
	if (m_nCurViewMode == CAMVIEW_ZOOMOUT)
		m_nCurViewMode = CAMVIEW_ZOOMIN;
	else
		m_nCurViewMode = CAMVIEW_ZOOMOUT;
}




void	CGenericCamera::CamProcessPreset(void)
{

	D3DXMATRIX	EyeObjMat, TargetObjMat;
	D3DXVECTOR4	Out;
	D3DXVECTOR3	CamPos, CamTarget,Err;
	D3DXVECTOR3	_ObjPos, _ObjDir;
	CAMSTATUS *pCam;

	pCam = GetCamStatus();

	if( IsEditMode()==true )
	{
		m_nStepPos = 0;
		m_nStepTarget = 0;
	}
	else
	{
		m_nStepPos = pCam->nEyeSmooth + 1;
		m_nStepTarget = pCam->nTargetSmooth + 1;
	}
	
	CamPos.x = pCam->EyeX;
	CamPos.y = pCam->EyeY;
	CamPos.z = pCam->EyeZ;

	CamTarget.x = pCam->TargetX;
	CamTarget.y = pCam->TargetY;
	CamTarget.z = pCam->TargetZ;

	Err = CamPos - m_CamCurLocalPos;

	if ( D3DXVec3Length(&Err) > 0.0f && m_nStepPos > 0 && pCam->bEyeWorldSmooth == false && IsTargetChanged() == false && IsVirgin() == false)
		m_CamCurLocalPos += Err / (float)m_nStepPos;
	else
		m_CamCurLocalPos = CamPos;

	Err = CamTarget - m_CamCurLocalTarget;
	if ( D3DXVec3Length(&Err) > 0.0f && m_nStepTarget > 0 && pCam->bTargetWorldSmooth == false && IsTargetChanged() == false  && IsVirgin() == false)
		m_CamCurLocalTarget += Err / (float)m_nStepTarget;
	else
		m_CamCurLocalTarget = CamTarget;

	if (m_nPrevEye != pCam->nEye || m_nPrevTarget != pCam->nTarget)
		m_pProjectile = NULL;

	m_nPrevEye = pCam->nEye;
	m_nPrevTarget = pCam->nTarget;

	D3DXMatrixIdentity(&EyeObjMat);
	D3DXMatrixIdentity(&TargetObjMat);

	if (pCam->nEye == CAM_ATTACH_FREE)
	{
		EyeObjMat._41 = m_CamCurLocalPos.x;
		EyeObjMat._42 = m_CamCurLocalPos.y;
		EyeObjMat._43 = m_CamCurLocalPos.z;
	}

	if (pCam->nTarget == CAM_ATTACH_FREE)
	{
		TargetObjMat._41 = m_CamCurLocalTarget.x;
		TargetObjMat._42 = m_CamCurLocalTarget.y;
		TargetObjMat._43 = m_CamCurLocalTarget.z;
	}

	if( GetMatrix(pCam->nEye, pCam->nParam1, &EyeObjMat) == false )		// 사용 안하는 attach면 false
	{
		pCam->nEye = CAM_ATTACH_ACTION_EYE;
		GetMatrix(pCam->nEye, pCam->nParam1, &EyeObjMat);
	}

	if( GetMatrix(pCam->nTarget, pCam->nParam1, &TargetObjMat) == false )
	{
		pCam->nTarget = CAM_ATTACH_PLAYER;
		GetMatrix(pCam->nTarget, pCam->nParam1, &TargetObjMat);
	}

	switch (pCam->nEye)
	{
	case CAM_ATTACH_LOCAL_AXIS:
		memcpy(&EyeObjMat, &TargetObjMat, sizeof(D3DXMATRIX));
		break;

	case CAM_ATTACH_LOCAL_CRD:
		EyeObjMat._41 = TargetObjMat._41;
		EyeObjMat._42 = TargetObjMat._42;
		EyeObjMat._43 = TargetObjMat._43;

		D3DXMatrixIdentity(&TargetObjMat);

		TargetObjMat._41 = EyeObjMat._41;
		TargetObjMat._42 = EyeObjMat._42;
		TargetObjMat._43 = EyeObjMat._43;
		break;

	case CAM_ATTACH_AXIS_PLAYER_TO_TARGET:
	case CAM_ATTACH_AXIS_PLAYERTROOP_TO_TARGET:
	case CAM_ATTACH_AXIS_TARGETTROOP_TO_TARGET:
	case CAM_ATTACH_AXIS_MARKEDAREA_TO_TARGET:
	case CAM_ATTACH_MARKED_POINT_FROM_CURRENT:


		{
			D3DXVECTOR3	Pos;
			D3DXVECTOR3	Dir;

			Pos.x = EyeObjMat._41;
			Pos.y = EyeObjMat._42;
			Pos.z = EyeObjMat._43;

			Dir.x = TargetObjMat._41 - EyeObjMat._41;
			Dir.y = TargetObjMat._42 - EyeObjMat._42;
			Dir.z = TargetObjMat._43 - EyeObjMat._43;

			float fDist = D3DXVec3Length(&Dir);

			if (fDist > 0.0f)
				Dir /= fDist;
			else
				Dir.y = 1.0f;

			ObjPoseToMatrix(&EyeObjMat,&Pos,&Dir);
		}

		break;


	}


	switch (pCam->nTarget)
	{
	case CAM_ATTACH_LOCAL_AXIS:
		memcpy(&TargetObjMat, &EyeObjMat, sizeof(D3DXMATRIX));
		break;

	case CAM_ATTACH_LOCAL_CRD:
		TargetObjMat._41 = EyeObjMat._41;
		TargetObjMat._42 = EyeObjMat._42;
		TargetObjMat._43 = EyeObjMat._43;

		D3DXMatrixIdentity(&EyeObjMat);

		EyeObjMat._41 = TargetObjMat._41;
		EyeObjMat._42 = TargetObjMat._42;
		EyeObjMat._43 = TargetObjMat._43;
		break;
	}

	if (pCam->nEye != CAM_ATTACH_FREE)
	{
		D3DXVec3Transform(&Out, &m_CamCurLocalPos, &EyeObjMat);

		CamPos.x = Out.x;
		CamPos.y = Out.y;
		CamPos.z = Out.z;
	}

	if (pCam->nTarget != CAM_ATTACH_FREE)
	{
		D3DXVec3Transform(&Out, &m_CamCurLocalTarget, &TargetObjMat);

		CamTarget.x = Out.x;
		CamTarget.y = Out.y;
		CamTarget.z = Out.z;
	}

	m_fFov = pCam->fFov;

	m_CamPos = CamPos;
	m_CamTarget = CamTarget;

	m_nCamRoll = pCam->nRoll;

	if(!g_pFcRealMovie || !g_pFcRealMovie->IsPlay())
	{
		if (pCam->nDOF == 0)
			g_BsKernel.EnableDOF( 0 );
		else
			g_BsKernel.EnableDOF( 1 );
	}

	m_fDOFFar = pCam->fDOFFar;
	m_fDOFNear = pCam->fDOFNear;
}


void CGenericCamera::CorrectCam()
{
	D3DXVECTOR3	Dir;
	float		fLength;

	CAMSTATUS *pCam;

	pCam = GetCamStatus();

	if (pCam->reserved[0] && m_bEditMode == false)
	{
		D3DXVECTOR3	vPos, vDir;
		D3DXVECTOR3	V1, V2, VErr;
		float	fYMod;
		const	int	nSamples = 5;
		float	fLen;

		vPos = m_CamPos;
		vDir = m_CamTarget - m_CamPos;

		fLen = D3DXVec3Length(&vDir);

		if (fLen > 0.0f)
		{
			vDir /= fLen;

			V1 = m_CamPos;

			bool bProp;
			V1.y = g_FcWorld.GetLandHeight( V1.x, V1.z, &bProp );

			int nScan;
			fYMod = 0.0f;
			int nCount = 0;

			for(nScan = 0;nScan < nSamples;nScan++)
			{
				float	fLen;
				bool bProp = false;

				V2 = vPos + vDir * (500.0f + (float)(1500 * nScan));

				float	fMapXSize, fMapYSize;
				g_FcWorld.GetMapSize( fMapXSize, fMapYSize );

				if (V2.x < 0.0f || V2.x >= fMapXSize || V2.z < 0.0f || V2.z >= fMapYSize)
					V2.y = 0.0f;
				else
					V2.y = g_FcWorld.GetLandHeight( V2.x, V2.z, &bProp );

				VErr = V2 - V1;
				fLen = D3DXVec3Length(&VErr);

				if (fLen>0.0f && bProp == false)
				{
					VErr /= fLen;

					if (VErr.y > 0.0f && VErr.y < 0.35f)	// 0도 ~ 20도만 걸러내며, 각도 안에 들더라도 이전 값과 차이가 너무 크면 무시
					{
						fYMod += VErr.y;
						nCount++;
					}
				}

			}

			if (nCount > 0)
			{
				fYMod /= (float)nCount;
				m_fActionEyeHMod += (fYMod - m_fActionEyeHMod) * .05f;
			}
			else
				m_fActionEyeHMod *= 0.25f;

			m_CamPos.y -= m_fActionEyeHMod * 1000.0f; 	// 어차피 너무 낮게 깔리면 CorrectCam에서 걸린다.
		}
												
	}

	Dir = m_CamPos - m_CamTarget;
	fLength = D3DXVec3Length(&Dir);

	if (fLength < CAM_MIN_DIST)
	{
		D3DXVec3Normalize(&Dir, &Dir);
		D3DXVec3Scale(&Dir, &Dir, CAM_MIN_DIST);

		m_CamPos = m_CamTarget + Dir;
	}

	float		fWaterH = g_BsKernel.GetWaterHeight();

	if (g_BsKernel.IsWaterInside(m_CamPos) && m_CamPos.y < fWaterH + 50.0f)
		m_CamPos.y = fWaterH + 50.0f;
}


void CGenericCamera::CamCollision(D3DXVECTOR3 *Pos, D3DXVECTOR3 *Target)
{
	float		fHeight;
	float		fWaterH = g_BsKernel.GetWaterHeight();

	bool bProp;

	if (g_BsKernel.IsWaterInside(*Pos) && Pos->y < fWaterH + 50.0f)
		Pos->y = fWaterH + 50.0f;

	fHeight = g_FcWorld.GetLandHeight( Pos->x, Pos->z, &bProp ) + 30.0f;

	if (Pos->y < fHeight && bProp == false)
		Pos->y = fHeight;

	D3DXVECTOR3	Pos2Target = (*Target) - (*Pos);
	float fDistToTarget = D3DXVec3Length(&Pos2Target);
	float fCheckPos = 0.f;

	if (fDistToTarget > CAM_DEFAULT_HEIGHT_SAMPLING_SPACE)
	{
		D3DXVECTOR3 Contact(0.0f,0.0f,0.0f);	// contact position
		Pos2Target /= fDistToTarget;

		bool  bContact = false;

		// find contact position with greatest land height
		while (fCheckPos < CAM_DEFAULT_HEIGHT_SAMPLING_SPACE * 20.0f)	// eye로 부터 2미터만 10cm로 샘플링
		{
			D3DXVECTOR3	CurPos; 

			CurPos = (*Pos) + Pos2Target * fCheckPos;

			bool bProp;
			fHeight = g_FcWorld.GetLandHeight(CurPos.x, CurPos.z, &bProp) + 30.0f;

///			if (g_BsKernel.IsWaterInside(CurPos) && fHeight < fWaterH + 50.0f)
///				fHeight = fWaterH + 50.0f;

			D3DXVECTOR3 NewContact(CurPos);
			D3DXVECTOR3	Dir;
			float		fDist;
			NewContact.y = fHeight;

			Dir = (*Target) - NewContact;
			fDist = D3DXVec3Length(&Dir);

			if (fDist > 0.0f)
			{
				Dir /= fDist;
				if (Dir.y < Pos2Target.y && Dir.y > -.76f)  // 경사가 -50도 이상이면 무시.
				{
					Pos2Target.y = Dir.y;
					bContact = true;
				}
			}

			fCheckPos += CAM_DEFAULT_HEIGHT_SAMPLING_SPACE;
		}

		if (bContact)
		{
			D3DXVec3Normalize(&Pos2Target, &Pos2Target);
			*Pos = *Target - Pos2Target * fDistToTarget;
		}
	}
}



bool CGenericCamera::GetMatrix(int nType, int nParm, D3DXMATRIX *pMatrix)
{
	D3DXVECTOR3		_ObjPos, _ObjDir;
	TroopObjHandle		Target;

	static TroopObjHandle	PrevTarget;
	D3DXVECTOR3	d3dVec;

	bool bRet = true;

	if(nType != CAM_ATTACH_LOCAL_AXIS && nType != CAM_ATTACH_LOCAL_CRD && nType != CAM_ATTACH_FREE)
	{
		if(nType == CAM_ATTACH_TROOP_PROJECTILE)
		{
			Target = GetTargetTroop();
			if( PrevTarget != Target )
			{
				m_pProjectile = NULL;
				PrevTarget = Target;
			}
			if( Target )
			{
				if (m_pProjectile == NULL)
					m_pProjectile = CFcProjectile::GetOneMovingProjectile( Target );
				
				if (m_pProjectile != NULL)
				{
					d3dVec = m_pProjectile->GetPos();

					_ObjPos.x = d3dVec.x;
					_ObjPos.y = d3dVec.y;
					_ObjPos.z = d3dVec.z;

					_ObjDir.x = 0.0f;
					_ObjDir.y = 0.0f;
					_ObjDir.z = -1.0f;
				}
				else
					bRet = GetCrdNDir(CAM_ATTACH_TROOP_PROJECTILE, nParm, &_ObjPos, &_ObjDir);

				ObjPoseToMatrix(pMatrix, &_ObjPos, &_ObjDir);
			}
		}
		else
		{
			bRet = GetCrdNDir(nType, nParm, &_ObjPos, &_ObjDir);
			ObjPoseToMatrix(pMatrix, &_ObjPos, &_ObjDir);
		}
	}
	return bRet;
}


void	CGenericCamera::SpecialCam(int nId, int nDur)
{
	m_nSpecialCam = nId;

	if (m_nSpecialCam > 8)
		m_nSpecialCam = 8;

	m_nSpecialCamBegin = m_nInternalTick;
	m_nSpecialCamDur = nDur;
}

void CGenericCamera::Chooser(void)
{
	int nNewCamSeq = FCCAM_INPHYY_DEFAULT;
	int	nOffsetByChar = -1;

	// ani type에 따른 camera

	int	nAniType = g_FcWorld.GetHeroHandle()->GetCurAniType();
	int	nHero = g_FcWorld.GetHeroHandle()->GetHeroClassID();

	if (m_nInternalTick - (unsigned int)m_nSpecialCamDur > m_nSpecialCamBegin && m_nSpecialCam >= 0) 
		m_nSpecialCam = -1;

	if (nAniType == ANI_TYPE_DIE)
	{
		nNewCamSeq = FCCAM_PLAYER_DEAD;
		nOffsetByChar = 0;
	}
	else
	{
		if (m_nSpecialCam >= 0)
		{
			if (m_nSpecialCam >= 0 && m_nSpecialCam <= 4)
				nNewCamSeq = FCCAM_INPHYY_SPECIAL0 + m_nSpecialCam;
			else
			{
				nNewCamSeq = FCCAM_INPHYY_SPECIAL5 + m_nSpecialCam - 5;
				nOffsetByChar = 4;
			}
		}
		else
		{
			switch (nAniType)
			{

			case	ANI_TYPE_JUMP_UP:
			case	ANI_TYPE_JUMP_DOWN:
			case	ANI_TYPE_JUMP_HIT:
				nNewCamSeq = FCCAM_INPHYY_JUMP;
				break;

			default:
				nNewCamSeq = FCCAM_INPHYY_DEFAULT;
				break;
			}

			if (m_bDefaultCamVar == true)
				nNewCamSeq += FCCAM_VAR_OFFSET;
		}
	}

	/// orb spark 보여주기 

	CFcAbilityManager *pAM = g_FcWorld.GetAbilityMng();

	if (pAM->GetRealAbilityCount() > 0 && m_nOrbCamTick == 0)	// internal counter가 0이고 Orb spark가 써진 상태라면 initialize
	{
		bool bStartCam = true;
		switch( pAM->GetRealAbilityType() ) {
			case FC_ABILITY_1KARROW:	bStartCam = false;		break;
			default:	break;
		}

		if( bStartCam == true ) {
			m_nOrbCamTick = 1;
			m_fFreeCamB = 0.0f;
			m_fFreeCamDA = 0.0f;
			m_fFreeCamDB = 0.0f;
			m_vActionEye.x = 0.0f;
			m_vActionEye.y = 0.0f;
		}
	}

	if (m_nOrbCamTick >= 1)	// internal counter 가 initialize 되었다면 counter 증가
	{
		m_nOrbCamTick ++;
		nNewCamSeq = FCCAM_INPHYY_ORBSPARK;
	}


	if (nOffsetByChar == -1)
	{
		switch (nHero)
		{
		case	CFcGameObject::Class_ID_Hero_Aspharr:
			nNewCamSeq += FCCAM_ASPHARR_DEFAULT;
			break;

		case	CFcGameObject::Class_ID_Hero_Inphy:
			break;

		case	CFcGameObject::Class_ID_Hero_Klarrann:
			nNewCamSeq += FCCAM_KLARRANN_DEFAULT;
			break;

		case	CFcGameObject::Class_ID_Hero_VigkVagk:
			nNewCamSeq += FCCAM_VIGKBAGK_DEFAULT;
			break;

		case	CFcGameObject::Class_ID_Hero_Myifee:
			nNewCamSeq += FCCAM_MIFYY_DEFAULT;
			break;

		case	CFcGameObject::Class_ID_Hero_Dwingvatt:
			nNewCamSeq += FCCAM_DWINGVATT_DEFAULT;
			break;

		case	CFcGameObject::Class_ID_Hero_Tyurru:
			nNewCamSeq += FCCAM_TYURRU_DEFAULT;
			break;
		}
	}
	else
	{
		switch (nHero)
		{
		case	CFcGameObject::Class_ID_Hero_Inphy:
			break;

		case	CFcGameObject::Class_ID_Hero_Aspharr:
			nNewCamSeq += nOffsetByChar;
			break;

		case	CFcGameObject::Class_ID_Hero_Tyurru:
			nNewCamSeq += nOffsetByChar * 2;
			break;

		case	CFcGameObject::Class_ID_Hero_Myifee:
			nNewCamSeq += nOffsetByChar * 3;
			break;

		case	CFcGameObject::Class_ID_Hero_Klarrann:
			nNewCamSeq += nOffsetByChar * 4;
			break;

		case	CFcGameObject::Class_ID_Hero_Dwingvatt:
			nNewCamSeq += nOffsetByChar * 5;
			break;

		case	CFcGameObject::Class_ID_Hero_VigkVagk:
			nNewCamSeq += nOffsetByChar * 6;
			break;
		}
	}
	
	int		nDur = SeqGetLength(nNewCamSeq);	// 오브 스파크 보여주는 최대 시간

	if (m_nOrbCamTick > nDur)	// 카메라에서 지정된 시간이 되거나 어빌러티가 없으면 더 이상 orb spark는 보여주지 않는다.
		m_nOrbCamTick = -1;

	if (pAM->GetRealAbilityCount() == 0 && m_nOrbCamTick == -1) // orb spark 보여주는 게 끝나고 orbspark도 사라졌으면 다시 초기화
		m_nOrbCamTick = 0;



	// seq play

	if (nNewCamSeq != m_nCurSeq)
		SeqPlay( nNewCamSeq );

	if (m_nFreeCamMoveCount > 0)
		m_nFreeCamMoveCount--;
}

void	CGenericCamera::InitActionCam(void)
{
	GameObjHandle	Unit = g_FcWorld.GetHeroHandle();

	switch(Unit->GetClassID())
	{
	case CFcGameObject::Class_ID_Hero_Aspharr:
	case CFcGameObject::Class_ID_Hero_Inphy:
	case CFcGameObject::Class_ID_Hero_Klarrann:
	case CFcGameObject::Class_ID_Hero_VigkVagk:			
	case CFcGameObject::Class_ID_Hero_Myifee:
		m_nDefaultActionCam = FCCAM_INPHYY_DEFAULT;
		break;
	}
	m_bPlayerSet = true;
}


void	CGenericCamera::ObjPoseToMatrix(D3DXMATRIX *pOut, D3DXVECTOR3 *pPos, D3DXVECTOR3 *pZDir)
{
	CCrossVector	Cross;

	Cross.m_PosVector = *pPos;

	Cross.m_ZVector = *pZDir;

	Cross.m_XVector.x = pZDir->z;
	Cross.m_XVector.y = 0.0f;
	Cross.m_XVector.z = -pZDir->x;

	D3DXVec3Normalize(&(Cross.m_ZVector), &(Cross.m_ZVector));
	Cross.UpdateVectorsX();
	memcpy(pOut, (D3DXMATRIX*)Cross, sizeof(D3DXMATRIX));
}

void	CGenericCamera::CamProcessSway(void)
{
	CAMSTATUS	*pCam;
	int			nStr;
	float		fDiv;
	float		fR;
	bool		bMove = false;
	D3DXVECTOR2	Push;

	pCam = GetCamStatus();

	if (pCam->nSway > 0)
	{
		nStr = CAM_SWAY_MAX - pCam->nSway + 1;
		
		fR = (float)(pCam->nSway) * .5f;

		fDiv = .05f / (float)(nStr);

		if ((m_nInternalTick % nStr) == 0)
		{
			Push.x = (float) Random(100) - 50;
			Push.y = (float) Random(100) - 50;


			if (Push.x == 0.0f && Push.y == 0.0f)
				Push.x = 50.0f;

			D3DXVec2Normalize(&Push, &Push);
			
			Push *= fR;
			m_SwayVel += Push;
		}

		float	fSpeed = D3DXVec2Length(&m_SwayVel);

		if (fSpeed > 0.0f && fSpeed < 0.5f * fR)
			m_SwayVel = (m_SwayVel / fSpeed) * 0.5f * fR;

		if (fSpeed > 3.0f * fR)
			m_SwayVel = (m_SwayVel / fSpeed) * 3.0f * fR;

		float fDist = D3DXVec2Length(&m_SwayPos);
		D3DXVECTOR2	Dir = m_SwayPos;

		if (fDist > 0.0f)
			Dir /= fDist;

		D3DXVECTOR2	Acc = D3DXVECTOR2(-m_SwayVel.y,m_SwayVel.x);
		D3DXVec2Normalize(&Acc, &Acc);

		if (D3DXVec2Dot(&Acc, &Dir) > 0.0f)
			Acc *= -1.0f;

		if (fDist > fR)
			m_SwayVel += -m_SwayPos * .01f + Acc * .05f;
		else
			m_SwayVel += Acc * 0.1f;
		
		m_SwayPos += m_SwayVel * fR * .3f;

		m_CamCross.m_PosVector += m_CamCross.m_XVector * m_SwayPos.x + m_CamCross.m_YVector * m_SwayPos.y;
	}
	else
	{
		m_SwayPos = D3DXVECTOR2(0.0f,0.0f);
		m_SwayVel = D3DXVECTOR2(0.0f,0.0f);
	}
}

void	CGenericCamera::Shock3D(D3DXVECTOR3	*pCenter, float fRadius, float fX, float fY, int nDur)
{
	D3DXVECTOR3	Diff;
	float		fDist;
	float		fPower, _fPower, fPower2;

	if (fRadius < 0.0f)
		fRadius = 0.0f;

	if (fRadius > 5000.0f)
		fRadius = 5000.0f;

	if (fX < 0.0f)
		fX = 0.0f;

	if (fX > 1.5f)
		fX = 1.5f;

	if (fY < 0.0f)
		fY = 0.0f;

	if (fY > 1.5f)
		fY = 1.5f;

	fPower = (fX > fY)?fX:fY;
	_fPower = (m_f3DShockX > m_f3DShockY)?m_f3DShockX:m_f3DShockY;

	if (nDur < 1)
		nDur = 1;

	Diff = m_CamCross.m_PosVector - m_v3DShockCenter;
	fDist = D3DXVec3Length(&Diff);

	if (m_f3DShockRadius <= 0.0f)
		fPower2 = _fPower;
	else if (fDist < m_f3DShockRadius)
		fPower2 = _fPower * (1.0f - fDist / m_f3DShockRadius);
	else
		fPower2 = 0.0f;

	if (m_b3DShock == false || fPower2 < fPower)
	{
		m_b3DShock = true;
		m_v3DShockCenter = *pCenter;
		m_f3DShockX = fX;
		m_f3DShockY = fY;
		m_f3DShockRadius = fRadius;
		m_n3DShockBegin = m_nInternalTick;
		m_n3DShockDur = nDur;
	}
}

void	CGenericCamera::CamProcessShock3D(void)
{
	CAMSTATUS	*pCam;
	D3DXVECTOR2	Push(0.0f,0.0f);

	pCam = GetCamStatus();

	if (pCam->nShock > 0)
	{
		Push.x = (float) Random(100) - 50;
		Push.y = (float) Random(100) - 50;

		if (Push.x == 0.0f && Push.y == 0.0f)
			Push.x = 50.0f;

		D3DXVec2Normalize(&Push, &Push);
		Push *= (float) (pCam->nShock);

	}

	if (m_b3DShock == true)
	{
		if (m_nInternalTick - (unsigned int)m_n3DShockDur < m_n3DShockBegin)
		{
			D3DXVECTOR3	Diff;
			float	fDist;
			float	fX, fY;
			unsigned int nT = m_nInternalTick - m_n3DShockBegin;
			float fAlpha;

			if (m_n3DShockDur > 0)
				fAlpha = 1.0f - ((float) nT) / ((float) m_n3DShockDur);
			else
				fAlpha = 0.0f;

			if (nT <= 1)
			{
				fX = 0.0f;
				fY = 0.0f;

				if (m_f3DShockRadius <= 0.0f)
				{
					fX = m_f3DShockX;
					fY = m_f3DShockY;
				}
				else
				{
					Diff = m_v3DShockCenter - m_CamCross.m_PosVector;
					fDist = D3DXVec3Length(&Diff);

					DebugString("-----------Shock: %f %f\n",fDist, m_f3DShockRadius);
					if (fDist < m_f3DShockRadius)
					{
						fX = m_f3DShockX * (1.0f - fDist / m_f3DShockRadius);
						fY = m_f3DShockY * (1.0f - fDist / m_f3DShockRadius);
					}
				}

				m_fShockPushX = 15.0f * fX * (float)(Random(2) * 2 - 1);
				m_fShockPushY = 15.0f * fY * (float)(Random(2) * 2 - 1);
			}

			Push.x = m_fShockPushX * fAlpha;
			Push.y = m_fShockPushY * fAlpha;

			m_fShockPushX *= -1.0f;
			m_fShockPushY *= -1.0f;
		}
		else
		{
			m_b3DShock = false;
			m_fShockPushX = 0.0f;
			m_fShockPushY = 0.0f;
		}
	}

	m_CamCross.m_PosVector += m_CamCross.m_XVector * Push.x + m_CamCross.m_YVector * Push.y;
}