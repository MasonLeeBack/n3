#include "stdafx.h"

#include "GenericCameraAttach.h"
#include "GenericCamEditor.h"
#include "BsKernel.h"
#include "toputil.h"
#include "InputPad.h"
#include "BsUiFont.h"
#include "GenericCamera.h"

#include "FcInterfaceManager.h"

#include "FcWorld.h"		// 제너릭하지 않다!..yooty



void CriticalException();



CCamEditor::CCamEditor( CGenericCamera *pCam )
{
	char	FileNameWithPath[MAX_PATH];

	m_pCamera = pCam;

	memset(&m_Cam,0,sizeof(CamEditing));

	m_bActivated = false;
	m_nCurItem = -1;

	m_Cam.Eye = &m_CurEyeGlobal;
	m_Cam.Target = &m_CurTargetGlobal;

	m_fMoveStep = 1.0f;
	m_fRotateStep = 0.02f;

	m_nCurSeqId = 0;
	m_nCurCamId	= 0;
	m_bCamPlaying = false;

	g_BsKernel.GetCurrentDirectory(MAX_PATH,FileNameWithPath);

	strcat(FileNameWithPath,"Camera\\GenericCamDef.h");

	m_ppDefTable = headerFileToStringTable(FileNameWithPath,&m_nDefTableSize);
	if(m_ppDefTable==NULL) {
		CriticalException();
	}

	m_nPopUpSize = 0;
	m_nPopUpActualSize = 0;
	m_ppPopUp = NULL;
	m_nPopUpCsr = 0;
	m_nPopUpWinHeight = 0;
	m_nPopUpWinWidth = 0;

	m_nPopUpX = 0;
	m_nPopUpY = 0;
	m_nEditMenuY = 0;
	m_nPopUpOffset = 0;
	m_nDisableTick = 0;
}

CCamEditor :: ~CCamEditor(void)
{
	Deactivate();
	int nScan;
	for(nScan = 0;nScan < m_nDefTableSize;nScan++)
	{
		if (m_ppDefTable[nScan] != NULL)
			free (m_ppDefTable[nScan]);
	}

	free (m_ppDefTable);

	if (m_ppPopUp != NULL)
	{
		for(nScan =0 ;nScan < m_nPopUpActualSize; nScan++)
			free(m_ppPopUp[nScan]);

		free(m_ppPopUp);
	}
}

void	CCamEditor :: Activate(void)
{
	CAMSTATUS *pStatus;

	m_pCamera->SetDirectMode(true);
	m_pCamera->SetEditMode(true);
	m_pCamera->EnableFreeCam(false);

	m_nCurSeqId = m_pCamera->SeqGetCurSeqId();
	m_nCurCamId = 0;
	
	pStatus = m_pCamera->SeqGetCam(m_nCurSeqId,0);

	if (pStatus != NULL)
		SetCamStatus(pStatus);

	m_Cam.Eye = &m_CurEyeGlobal;
	m_Cam.Target = &m_CurTargetGlobal;

	m_bActivated = true;

	CInputPad::GetInstance().ResetKeyBuffer();
	m_bCamPlaying = false;
	m_nDisableTick = 10;
}

void	CCamEditor :: Deactivate(void)
{
	m_pCamera->SetDirectMode(false);
	m_pCamera->SetEditMode(false);

	m_pCamera->EnableFreeCam(true);
//	m_pJoypadCtl->UseAnalog1AsDPad(true);

	m_bActivated = false;
	CInputPad::GetInstance().ResetKeyBuffer();
	m_bCamPlaying = false;
	
}


bool	CCamEditor :: IsActivated(void)
{
	return m_bActivated;
}


void	CCamEditor :: Process(void)
{
	bool		CamModified1, CamModified2;
	D3DXVECTOR3	EyeG,TargetG;

	// 메뉴에서 입력이 여기까지 도착해서 먹기 때문에 활성화 된 다음 10틱동안은 동작하지 않는다.

	if (m_bActivated)
	{

		if (g_InterfaceManager.GetMenuMgr()->GetCurMenuHandle() != NULL)
			m_nDisableTick = 10;

		if (m_nDisableTick >= 0)
			m_nDisableTick--;

		if (m_nDisableTick <= 0)
		{
			if (m_bCamPlaying == false)
				CamModified1 = ModifyCamStatus(&m_Cam);
			else
				CamModified1 = false;

			CamCoordToGlobal(&EyeG, &TargetG, &m_Cam);
			CamModified2 = MoveCam(&m_Cam, &EyeG, &TargetG);
			GlobalToCamCoord(&m_Cam, &EyeG, &TargetG);

			ProcessUI(CamModified1 || CamModified2);

			if ((CamModified1 == true || CamModified2 == true) && m_bCamPlaying == false)
				CamEditing2CamStatus(m_pCamera->GetCamStatus(), &m_Cam);

			ModifyCamSeq();
		}
	}
}

void	CCamEditor::ObjPoseToMatrix(D3DXMATRIX *pOut, D3DXVECTOR3 *pPos, D3DXVECTOR3 *pZDir)
{
	CCrossVector	Cross;

	Cross.m_PosVector.x = pPos->x;
	Cross.m_PosVector.y = pPos->y;
	Cross.m_PosVector.z = pPos->z;

	Cross.m_ZVector.x = pZDir->x;
	Cross.m_ZVector.y = pZDir->y;
	Cross.m_ZVector.z = pZDir->z;

	Cross.m_XVector.x = pZDir->z;
	Cross.m_XVector.y = 0.0f;
	Cross.m_XVector.z = -pZDir->x;

	D3DXVec3Normalize(&(Cross.m_ZVector), &(Cross.m_ZVector));
	
	Cross.UpdateVectorsX();

	memcpy(pOut, (D3DXMATRIX*)Cross, sizeof(D3DXMATRIX));
}


void	CCamEditor::GetMatrix(CamEditing *pCam ,D3DXMATRIX *pEyeOut, D3DXMATRIX *pTargetOut)
{
	D3DXVECTOR3		_ObjPos, _ObjDir;

	D3DXMatrixIdentity(pEyeOut);
	D3DXMatrixIdentity(pTargetOut);

	if (pCam->nEye == CAM_ATTACH_FREE)
	{
		pEyeOut->_41 = pCam->Eye->x;
		pEyeOut->_42 = pCam->Eye->y;
		pEyeOut->_43 = pCam->Eye->z;
	}
	
	if (pCam->nTarget == CAM_ATTACH_FREE)
	{
		pTargetOut->_41 = pCam->Target->x;
		pTargetOut->_42 = pCam->Target->y;
		pTargetOut->_43 = pCam->Target->z;
	}

	if (pCam->nEye != CAM_ATTACH_LOCAL_AXIS && pCam->nEye != CAM_ATTACH_LOCAL_CRD && pCam->nEye != CAM_ATTACH_FREE)
	{
		m_pCamera->GetCrdNDir(pCam->nEye, pCam->nParam1, &_ObjPos, &_ObjDir);
		ObjPoseToMatrix(pEyeOut, &_ObjPos, &_ObjDir);
	}

	if (pCam->nTarget != CAM_ATTACH_LOCAL_AXIS && pCam->nTarget != CAM_ATTACH_LOCAL_CRD && pCam->nTarget != CAM_ATTACH_FREE)
	{
		m_pCamera->GetCrdNDir(pCam->nTarget, pCam->nParam1, &_ObjPos, &_ObjDir);
		ObjPoseToMatrix(pTargetOut, &_ObjPos, &_ObjDir);
	}

	if (pCam->nEye == CAM_ATTACH_LOCAL_AXIS)
		memcpy(pEyeOut, pTargetOut, sizeof(D3DXMATRIX));

	if (pCam->nEye == CAM_ATTACH_LOCAL_CRD)
	{
		pEyeOut->_41 = pTargetOut->_41;
		pEyeOut->_42 = pTargetOut->_42;
		pEyeOut->_43 = pTargetOut->_43;

		D3DXMatrixIdentity(pTargetOut);

		pTargetOut->_41 = pEyeOut->_41;
		pTargetOut->_42 = pEyeOut->_42;
		pTargetOut->_43 = pEyeOut->_43;
	}


	if (pCam->nTarget == CAM_ATTACH_LOCAL_AXIS)
		memcpy(pTargetOut, pEyeOut, sizeof(D3DXMATRIX));

	if (pCam->nTarget == CAM_ATTACH_LOCAL_CRD)
	{
		pTargetOut->_41 = pEyeOut->_41;
		pTargetOut->_42 = pEyeOut->_42;
		pTargetOut->_43 = pEyeOut->_43;

		D3DXMatrixIdentity(pEyeOut);

		pEyeOut->_41 = pTargetOut->_41;
		pEyeOut->_42 = pTargetOut->_42;
		pEyeOut->_43 = pTargetOut->_43;
	}

}


void	CCamEditor::GlobalToCamCoord(CamEditing *pCam,D3DXVECTOR3 *pEyeIn, D3DXVECTOR3 *pTargetIn)
{
	D3DXMATRIX	EyeObjMat, TargetObjMat;
	float		fD;
	
	D3DXVECTOR4	Out;

	if (pCam->nEye == CAM_ATTACH_FREE)
		*pCam->Eye = *pEyeIn;

	if (pCam->nTarget == CAM_ATTACH_FREE)
		*pCam->Target = *pTargetIn;

	GetMatrix(pCam, &EyeObjMat, &TargetObjMat);

	D3DXMatrixInverse(&EyeObjMat, &fD, &EyeObjMat);
	D3DXMatrixInverse(&TargetObjMat, &fD, &TargetObjMat);

	if (pCam->nEye != CAM_ATTACH_FREE)
	{
		D3DXVec3Transform(&Out, pEyeIn, &EyeObjMat);

		pCam->Eye->x = Out.x;
		pCam->Eye->y = Out.y;
		pCam->Eye->z = Out.z;
	}

	if (pCam->nTarget != CAM_ATTACH_FREE)
	{
		D3DXVec3Transform(&Out, pTargetIn, &TargetObjMat);

		pCam->Target->x = Out.x;
		pCam->Target->y = Out.y;
		pCam->Target->z = Out.z;
	}
}


void	CCamEditor::CamCoordToGlobal(D3DXVECTOR3 *pEyeOut, D3DXVECTOR3 *pTargetOut, CamEditing *pCam)
{

	D3DXMATRIX	EyeObjMat, TargetObjMat;
	D3DXVECTOR4	Out;

	GetMatrix(pCam,&EyeObjMat, &TargetObjMat);

	if (pCam->nEye != CAM_ATTACH_FREE)
	{
		D3DXVec3Transform(&Out, pCam->Eye, &EyeObjMat);
		
		pEyeOut->x = Out.x;
		pEyeOut->y = Out.y;
		pEyeOut->z = Out.z;
	}
	else
		*pEyeOut = *pCam->Eye;


	if (pCam->nTarget != CAM_ATTACH_FREE)
	{
		D3DXVec3Transform(&Out, pCam->Target, &TargetObjMat);

		pTargetOut->x = Out.x;
		pTargetOut->y = Out.y;
		pTargetOut->z = Out.z;
	}
	else
		*pTargetOut = *pCam->Target;

}


bool	CCamEditor::MoveCam(CamEditing *pCam, D3DXVECTOR3 *pEye, D3DXVECTOR3 *pTarget)
{
	D3DXVECTOR3	Left;
	D3DXVECTOR3 CamDir;
	D3DXVECTOR3	CamEye;
	D3DXVECTOR3	Up(0.0f,1.0f,0.0f);
	D3DXMATRIX	RotMat;
	D3DXVECTOR4	Result;
	float		fHeight;
	bool		bMoving = false;
	bool		bRotating = false;
	float		fDist;

	CamEye = *pEye;
	CamDir = (*pTarget) - (*pEye);

	fDist = D3DXVec3Length(&CamDir);

	if (fDist < CAM_DIST_MIN)
	{
		fDist = CAM_DIST_MIN;
		bMoving = true;
	}

	D3DXVec3Normalize(&CamDir, &CamDir);

	bool bProp;
	fHeight = g_FcWorld.GetLandHeight( CamEye.x, CamEye.z, &bProp );
	if (CamEye.y < fHeight && m_pCamera->IsCollisionEnabled() == true)
		CamEye.y = fHeight;

	D3DXVec3Cross(&Left,&CamDir,&Up);
	D3DXVec3Normalize(&Left, &Left);

	if( CInputPad::GetInstance().GetKeyPressTick(-1, PAD_INPUT_BLACK) > 0 &&
		CInputPad::GetInstance().GetKeyPressTick(-1, PAD_INPUT_WHITE) > 0 )
	{
		CAMSTATUS *pStatus;
		pStatus = m_pCamera->GetCamStatus();
		if( CInputPad::GetInstance().GetKeyPressTick(-1, PAD_INPUT_LTHUMB) > 0 ||
			CInputPad::GetInstance().GetKeyPressTick(-1, PAD_INPUT_RTHUMB) > 0 )
		{
			pStatus->EyeX = 0.0f;
			pStatus->EyeY = 0.0f;
			pStatus->EyeZ = 0.0f;
		}
		else
		{
			pStatus->TargetX = 0.0f;
			pStatus->TargetY = 0.0f;
			pStatus->TargetZ = 0.0f;
		}
		

		SetCamStatus(pStatus);
	}
	else
	{
		if( CInputPad::GetInstance().GetKeyPressTick(-1, PAD_INPUT_WHITE) > 0 )
		{
			CamEye.y += m_fMoveStep * 2.0f;
			bMoving = true;

			if (CamEye.y < fHeight && m_pCamera->IsCollisionEnabled() == true)
				CamEye.y = fHeight;
		}

		if( CInputPad::GetInstance().GetKeyPressTick(-1, PAD_INPUT_BLACK) > 0 )
		{
			CamEye.y -= m_fMoveStep * 2.0f;
			bMoving = true;

			if (CamEye.y < fHeight && m_pCamera->IsCollisionEnabled() == true)
				CamEye.y = fHeight;
		}
	}

	if( CInputPad::GetInstance().GetKeyPressTick( -1, PAD_INPUT_RSTICK) )
	{
		KEY_EVENT_PARAM* pParam = CInputPad::GetInstance().GetKeyParam(-1, 1);
		if( pParam->nPosX < - JOYPAD_ANALOG_DEADZONE )
		{
			D3DXMatrixRotationAxis(&RotMat,&Up,-m_fRotateStep);
			D3DXVec3Transform(&Result, &CamDir, &RotMat);
			CamDir.x = Result.x;
			CamDir.y = Result.y;
			CamDir.z = Result.z;
			bRotating = true;
		}
	}

	if( CInputPad::GetInstance().GetKeyPressTick( -1, PAD_INPUT_RSTICK) )
	{
		KEY_EVENT_PARAM* pParam = CInputPad::GetInstance().GetKeyParam(-1, 1);
		if( pParam->nPosX > JOYPAD_ANALOG_DEADZONE )
		{
			D3DXMatrixRotationAxis(&RotMat,&Up,m_fRotateStep);
			D3DXVec3Transform(&Result, &CamDir, &RotMat);
			CamDir.x = Result.x;
			CamDir.y = Result.y;
			CamDir.z = Result.z;
			bRotating = true;
		}
	}

	if( CInputPad::GetInstance().GetKeyPressTick( -1, PAD_INPUT_RSTICK) )
	{
		KEY_EVENT_PARAM* pParam = CInputPad::GetInstance().GetKeyParam(-1, 1);
		if( pParam->nPosY > JOYPAD_ANALOG_DEADZONE )
		{
			D3DXMatrixRotationAxis(&RotMat,&Left,-m_fRotateStep);
			D3DXVec3Transform(&Result, &CamDir, &RotMat);
			CamDir.x = Result.x;
			CamDir.y = Result.y;
			CamDir.z = Result.z;
			bRotating = true;
		}
	}

	if( CInputPad::GetInstance().GetKeyPressTick( -1, PAD_INPUT_RSTICK) )
	{
		KEY_EVENT_PARAM* pParam = CInputPad::GetInstance().GetKeyParam(-1, 1);
		if( pParam->nPosY < -JOYPAD_ANALOG_DEADZONE )
		{
			D3DXMatrixRotationAxis(&RotMat,&Left,m_fRotateStep);
			D3DXVec3Transform(&Result, &CamDir, &RotMat);
			CamDir.x = Result.x;
			CamDir.y = Result.y;
			CamDir.z = Result.z;
			bRotating = true;
		}
	}

	if( CInputPad::GetInstance().GetKeyPressTick( -1, PAD_INPUT_LSTICK) )
	{
		KEY_EVENT_PARAM* pParam = CInputPad::GetInstance().GetKeyParam(-1, 0);
		if( pParam->nPosX < -JOYPAD_ANALOG_DEADZONE )
		{
			CamEye += Left * m_fMoveStep;
			bMoving = true;
		}
	}

	if( CInputPad::GetInstance().GetKeyPressTick( -1, PAD_INPUT_LSTICK) )
	{
		KEY_EVENT_PARAM* pParam = CInputPad::GetInstance().GetKeyParam(-1, 0);
		if( pParam->nPosX > JOYPAD_ANALOG_DEADZONE )
		{
			CamEye -= Left * m_fMoveStep;
			bMoving = true;
		}
	}

	if( CInputPad::GetInstance().GetKeyPressTick( -1, PAD_INPUT_LSTICK) )
	{
		KEY_EVENT_PARAM* pParam = CInputPad::GetInstance().GetKeyParam(-1, 0);
		if( pParam->nPosY < -JOYPAD_ANALOG_DEADZONE )
		{
			CamEye -= CamDir * m_fMoveStep;
			fDist += m_fMoveStep;
			bMoving = true;
		}
	}

	if( CInputPad::GetInstance().GetKeyPressTick( -1, PAD_INPUT_LSTICK) )
	{
		KEY_EVENT_PARAM* pParam = CInputPad::GetInstance().GetKeyParam(-1, 0);
		if( pParam->nPosY > JOYPAD_ANALOG_DEADZONE )
		{
			CamEye += CamDir * m_fMoveStep;
			bMoving = true;

			// 이게 맞는건지 확인 해주세요 ... jiesang
			if (fDist > CAM_DIST_MIN + m_fMoveStep)
				fDist -= m_fMoveStep;
		}
	}

	if( CInputPad::GetInstance().GetKeyPressTick( -1, PAD_INPUT_START ) >= 1 && bRotating == false && bMoving == true)
	{
		CamDir = (*pTarget) - CamEye;
		D3DXVec3Normalize(&CamDir, &CamDir);
	}


	float	fRotBound, fMoveBound;

	if( CInputPad::GetInstance().GetKeyPressTick( -1, PAD_INPUT_RTHUMB ) > 0 ||
		CInputPad::GetInstance().GetKeyPressTick( -1, PAD_INPUT_LTHUMB ) > 0 )
	{
		fRotBound = 0.002f;
		fMoveBound = 10.0f;
	}
	else
	{
		fRotBound = 0.2f;
		fMoveBound = 300.0f;
	}

	if (bMoving == true)
		m_fMoveStep++;

	if (m_fMoveStep > fMoveBound)
		m_fMoveStep = fMoveBound;

	if (bMoving == false)
		m_fMoveStep = 1.0f;


	if (bRotating == true)
		m_fRotateStep += 0.005f;

	if (m_fRotateStep > fRotBound)
		m_fRotateStep = fRotBound;

	if (bRotating == false)
		m_fRotateStep = 0.002f;

	*pEye = CamEye;
	*pTarget = CamEye + CamDir * fDist;

	return ((bMoving == true) || (bRotating == true));
}

void	CCamEditor::CamStatus2CamEditing(CamEditing *pEdit, CAMSTATUS *pStatus)
{
	pEdit->Eye = &(pEdit->_Eye);
	pEdit->Target = &(pEdit->_Target);

	pEdit->nEyeSmooth = pStatus->nEyeSmooth;
	pEdit->nTargetSmooth = pStatus->nTargetSmooth;

	pEdit->bEyeWorldSmooth = pStatus->bEyeWorldSmooth;
	pEdit->bTargetWorldSmooth = pStatus->bTargetWorldSmooth;

	pEdit->nEye = pStatus->nEye;
	pEdit->nTarget = pStatus->nTarget;
	pEdit->fFov = pStatus->fFov;

	pEdit->Eye->x = pStatus->EyeX;
	pEdit->Eye->y = pStatus->EyeY;
	pEdit->Eye->z = pStatus->EyeZ;

	pEdit->Target->x = pStatus->TargetX;
	pEdit->Target->y = pStatus->TargetY;
	pEdit->Target->z = pStatus->TargetZ;

	pEdit->nDuration = pStatus->nDuration;

	pEdit->nDOF = pStatus->nDOF;
	pEdit->fDOFFar = pStatus->fDOFFar;
	pEdit->fDOFNear = pStatus->fDOFNear;

	pEdit->bScreenEffect = pStatus->bScreenEffect;
	pEdit->nParam1 = pStatus->nParam1;
	pEdit->bFocusing = pStatus->bFocusing;

	pEdit->nRoll = pStatus->nRoll;

	if (pEdit->nDOF < 0 || pEdit->nDOF > 2)
		pEdit->nDOF = 2;

	if (pEdit->nDOF == 2)
	{
		pEdit->fDOFNear = 10.f;
		pEdit->fDOFFar = 1500.f;
	}

	pEdit->nSway = pStatus->nSway;
	pEdit->nShock = pStatus->nShock;

	pEdit->bEyeHeightMod = (pStatus->reserved[0])?1:0;
}


CGenericCamera* CCamEditor::GetCam(void)
{
	return m_pCamera;
}

void	CCamEditor::CamEditing2CamStatus(CAMSTATUS *pStatus, CamEditing *pEdit)
{
	if (pEdit->nEye == CAM_ATTACH_FREE)
		pEdit->Eye = &m_CurEyeGlobal;
	else
		pEdit->Eye = &m_CurEyeLocal;

	if (pEdit->nTarget == CAM_ATTACH_FREE)
		pEdit->Target = &m_CurTargetGlobal;
	else
		pEdit->Target = &m_CurTargetLocal;

	pStatus->nEyeSmooth = pEdit->nEyeSmooth;
	pStatus->nTargetSmooth = pEdit->nTargetSmooth;

	pStatus->bEyeWorldSmooth = pEdit->bEyeWorldSmooth;
	pStatus->bTargetWorldSmooth = pEdit->bTargetWorldSmooth;

	pStatus->nEye = pEdit->nEye;
	pStatus->nTarget = pEdit->nTarget;
	pStatus->fFov = pEdit->fFov;

	pStatus->EyeX = pEdit->Eye->x;
	pStatus->EyeY = pEdit->Eye->y;
	pStatus->EyeZ = pEdit->Eye->z;

	pStatus->TargetX = pEdit->Target->x;
	pStatus->TargetY = pEdit->Target->y;
	pStatus->TargetZ = pEdit->Target->z;

	pStatus->nDuration = pEdit->nDuration;
	
	pStatus->nDOF = pEdit->nDOF;
	pStatus->fDOFFar = pEdit->fDOFFar;
	pStatus->fDOFNear = pEdit->fDOFNear;

	pStatus->bScreenEffect = pEdit->bScreenEffect;
	pStatus->nParam1 = pEdit->nParam1;

	pStatus->bFocusing = pEdit->bFocusing;

	pStatus->nRoll= pEdit->nRoll;

	pStatus->nSway = pEdit->nSway;
	pStatus->nShock = pEdit->nShock;

	pStatus->reserved[0] = (pEdit->bEyeHeightMod)?1:0;
}

void	CCamEditor::SeqPlay(void)
{
	if (m_nCurSeqId >= 0)
	{
		m_bCamPlaying = true;
		m_pCamera->SetDirectMode(false);
		m_pCamera->SetUserSeq(m_nCurSeqId,0,false);
	}
}

void	CCamEditor::SeqStop(void)
{
	CAMSTATUS CamStatus;

	memcpy(&CamStatus, m_pCamera->GetCamStatus(), sizeof(CAMSTATUS));

	m_bCamPlaying = false;
	m_pCamera->SeqStop();

	m_pCamera->SetDirectMode(true);

	SetCamStatus(&CamStatus);
}

// 임의의 cam status를 편집하고 싶으면 이 함수를 불러라!


void	CCamEditor::SetCamStatus(CAMSTATUS *pStatus)
{
	memcpy(m_pCamera->GetCamStatus(), pStatus, sizeof(CAMSTATUS));
	CamStatus2CamEditing(&m_Cam, pStatus);

	m_CurEyeGlobal = *(m_Cam.Eye);
	m_CurEyeLocal = *(m_Cam.Eye);

	m_CurTargetGlobal = *(m_Cam.Target);
	m_CurTargetLocal = *(m_Cam.Target);
}

int		CCamEditor::SeqGetCurId(void)
{
	return m_nCurSeqId;
}

void	CCamEditor::SeqSetCurId(int nId)
{
	m_nCurSeqId = nId;
}

bool	CCamEditor::SeqIsPlaying(void)
{
	return m_bCamPlaying;
}

void	CCamEditor::ShowCurCam(void)
{
	char	CamName[1024];
	int		nCamSeqId;

	nCamSeqId = m_pCamera->SeqGetCurSeqId();

	if (nCamSeqId >= m_nDefTableSize || nCamSeqId < 0)
		strcpy(CamName,"NOT DEFINED");
	else
	{
		if (m_ppDefTable[nCamSeqId] == NULL)
			strcpy(CamName,"NONAME");
		else
			strcpy(CamName,m_ppDefTable[nCamSeqId]);
	}

	g_pFont->DrawUIText(50,400,640,480,CamName);	
}

