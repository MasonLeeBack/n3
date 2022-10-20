#include "stdafx.h"

#include "GenericCameraAttach.h"
#include "GenericCamEditor.h"
#include "BsKernel.h"
#include "toputil.h"
#include "InputPad.h"
#include "BsUiFont.h"
#include "GenericCamera.h"

#include "FcInterfaceManager.h"

#define	CAM_STATUS_ITEM_MAX		16

enum	CAMITEM 
{
		_CI_EYESTEP = 0,
		_CI_EYESMOOTHAXIS,
		_CI_EYEATTACH,
		_CI_TARGETSTEP,
		_CI_TARGETSMOOTHAXIS,
		_CI_TARGETATTACH,
		_CI_FOV,
		_CI_SWAY,
		_CI_SHOCK,
		_CI_DUR,
		_CI_ROLL,
		_CI_DOF,
		_CI_DOFNEAR,
		_CI_DOFFAR,
		_CI_EYEHMOD,
		_CI_PARAM0,
};

static	char	*g_CamItemName[CAM_STATUS_ITEM_MAX] = {
													"Eye",
													"Smooth",
													"Attach",
													"Trg",
													"Smooth",
													"Attach",
													"FOV",
													"Sway",
													"Shck",
													"Dur",
													"Roll",
													"DOF",
													"Near",
													"far",
													"EyeH",
													"Prm0"
												};

static int jg_margin = 60;

static	int		g_nItemX[CAM_STATUS_ITEM_MAX] = { 
													20 + jg_margin,			// "EyeStep",
													80 + jg_margin,			// "SmoothType",
													160 + jg_margin,		// "EyeAttach",
													310 + jg_margin,		// "TargetStep",
													370 + jg_margin,		// "SmoothType",
													450 + jg_margin,		// "TargetAttach",
													600 + jg_margin,		// "FOV",
													660 + jg_margin,		// "Sway",
													720 + jg_margin,		// "Shck",
													780 + jg_margin,		// "Dur",
													840 + jg_margin,		// "Roll",
													900 + jg_margin,		// "DOF",
													960 + jg_margin,		// "DOFNr",
													1020 + jg_margin,		// "DOFFr",
													1080 + jg_margin,		// "ScrEf",
													1140 + jg_margin		// "Prm0",
												};

static	char	*g_CamAttachName[CAM_ATTACH_MAX] = {	"Free Cam", 
													"Local Axis",
													"Local Coord",
													"Player Troop",
													"Player Char", 
													"Target Troop",
													"Target Leader",
													"Unit all",
													"Unit Alive",
													"Nearest enemy",
													"Projectile",
													"Action Eye",
													"Orbspark Eye",
													"Leader foward",
													"Nearest enemy2",
													"Marked point",
													"Orbattack Target",
													"PlayerTroop2Trg",
													"Player troop2",
													"Target Alive 3D",
													"Player2Target",
													"PlayerTroop2Trg",
													"TargetTroop2Trg",
													"MarkedArea2Trg",};

static	char	*g_CamSwayName[CAM_SWAY_TYPE_MAX] = {	"Stdy",
														"Sway",
														"Shck"
														};


#define	CAM_TOGGLE_MAX		2

static	char	*g_CamSmoothAxisName[CAM_TOGGLE_MAX] = { "Local",
														 "World" };

#define	CAM_DOF_MAX			3
static	char	*g_CamDofName[CAM_DOF_MAX] = { "OFF",
											   "ON",
											   "DEF"};

static	char	*g_CamOnOffText[CAM_TOGGLE_MAX] = { "Off",
													 "On" };

void	strncpyShorten(char *pTrg, char *pSrc, int nSize)
{
	if (nSize >= 3)
	{
		strncpy_s(pTrg, nSize, pSrc, _TRUNCATE); //aleksger - safe string
		if ((int)strlen(pSrc) > nSize)
		{
			pTrg[nSize - 3] = '.';
			pTrg[nSize - 2] = '.';
			pTrg[nSize - 1] = '\x0';
		}
	}
}

void	CCamEditor :: ProcessUI(bool CamModified)
{
	int			nScan;
	int			nSeqSize;
	CAMSTATUS *pCamSt;
	CamEditing	tempCam;
	char		Buf[1024];
	char		CamName[1024];
	char		CamType[1024];
	

	nSeqSize = m_pCamera->SeqGetSize(m_nCurSeqId);

	m_nEditMenuY = (int)(680.0f - nSeqSize * 20.0f);

	if (m_nCurSeqId >= m_nDefTableSize || m_nCurSeqId < 0)
		strcpy(CamName,"NOT DEFINED");
	else
	{
		if (m_ppDefTable[m_nCurSeqId] == NULL)
			strcpy(CamName,"NONAME");
		else
			strcpy(CamName,m_ppDefTable[m_nCurSeqId]);
	}

	if (m_nCurSeqId < 100)
		strcpy(CamType,"Default");
	else
		strcpy(CamType,"UserDef");

	if (m_bCamPlaying == true)
	{
		m_nCurCamId = m_pCamera->SeqGetCurCamId();
		m_nCurItem = -1;

		SetCamStatus(m_pCamera->GetCamStatus());

		int	nTick;

		nTick = m_pCamera->GetTick();
		
		if (nTick % 10 >= 5)
			sprintf(Buf,"%s #%d : %s PLAY >>", CamType, m_nCurSeqId, CamName);
		else
			sprintf(Buf,"%s #%d : %s", CamType, m_nCurSeqId, CamName);
	}
	else
	{
		sprintf(Buf,"%s #%d : %s", CamType, m_nCurSeqId, CamName);
	}



	g_BsKernel.DrawUIBox_s(_Ui_Mode_Box, 0, m_nEditMenuY - 50, 1280, 768, 0.0f, D3DCOLOR_RGBA(50,50,100,210));
	g_pFont->DrawUIText(50,m_nEditMenuY - 45, -1, -1, Buf);

	PrintCamStatus(NULL,-2,(m_nEditMenuY) - 25);	

	g_BsKernel.DrawUIBox_s(_Ui_Mode_Box, 0, 0, 1280, 70, 0.0f, D3DCOLOR_RGBA(25,25,50,210));
	g_BsKernel.DrawUIBox_s(_Ui_Mode_Box, 0, m_nEditMenuY + m_nCurCamId * 20, 1280, 30, 0.0f, D3DCOLOR_RGBA(25,25,50,210));

	if (m_bCamCopied == true)
	{
		g_pFont->DrawUIText(0, 20, -1, -1, "@(color=255,221,221,255)Copied");
		PrintCamStatus(&m_CamCopied, -999, 40);
	}

	for(nScan = 0;nScan < nSeqSize; nScan++)
	{
		pCamSt = m_pCamera->SeqGetCam(m_nCurSeqId, nScan);

		if (nScan == m_nCurCamId)
		{
			if (CamModified == true && m_bCamPlaying == false)
				CamEditing2CamStatus(pCamSt, &m_Cam);
		}

		CamStatus2CamEditing(&tempCam, pCamSt);

		if (nScan == m_nCurCamId)
			PrintCamStatus(&tempCam ,m_nCurItem,m_nEditMenuY + nScan * 20);
		else
			PrintCamStatus(&tempCam, -999, m_nEditMenuY + nScan * 20);
	}

	if (nSeqSize == m_nCurCamId)
	{
		PrintCamStatus(NULL,-1,m_nEditMenuY + nSeqSize * 20);	
		m_nCurItem = -1;
	}

	g_pFont->DrawUIText(50, 0, -1, -1, "@(reset)@(scale=0.8,0.8)@(color=200,200,250,187)X: Del  A: Paste  Y: Copy  B: Test  L+R+A+X: Clear camera");

	PopUpProcess();
}

void	CCamEditor :: PrintCamStatus(CamEditing *pCam, int nCurSel, int nY)
{
	char	Selected[1024];
	char	Normal[1024];
	char	Str[1024];

	int		nScan;									   

	if (nCurSel == -1)
	{
		if (m_pCamera->GetTick() % 60 > 30)
			g_pFont->DrawUIText(g_nItemX[0] - 20, nY, -1, -1, "@(reset)@(color=255,255,255,255)>");
		else
			g_pFont->DrawUIText(g_nItemX[0] - 20, nY, -1, -1, "@(reset)@(color=255,255,255,255)=");
	}

	if (nCurSel == -2)
	{
		g_pFont->DrawUIText(0,0,-1, -1, "@(color=187,187,255,255)@(scale=0.6,0.8)");

		for(nScan = 0;nScan < CAM_STATUS_ITEM_MAX; nScan++)
			g_pFont->DrawUIText(g_nItemX[nScan],nY, -1, -1, g_CamItemName[nScan]);

		return;
	}

	if (pCam != NULL)
	{
		strcpy(Selected,"@(color=255,255,255,255)@(scale=0.6,0.8)");
		strcpy(Normal,"@(color=200,200,255,255)@(scale=0.6,0.8)");

		for(nScan = 0;nScan < CAM_STATUS_ITEM_MAX;nScan++)
		{
			if (nScan == nCurSel)
				g_pFont->DrawUIText(g_nItemX[nScan],nY, -1, -1, Selected);
			else
				g_pFont->DrawUIText(g_nItemX[nScan],nY, -1, -1, Normal);

			switch (nScan)
			{
			case _CI_EYESTEP:
				if (pCam->nEyeSmooth >= 0)
					sprintf(Str,"%03d",pCam->nEyeSmooth);
				else
					sprintf(Str,"U%03d", -pCam->nEyeSmooth);
				break;

			case _CI_TARGETSTEP:
				if (pCam->nTargetSmooth >= 0)
					sprintf(Str,"%03d",pCam->nTargetSmooth);
				else
					sprintf(Str,"U%03d", -pCam->nTargetSmooth);
				break;

			case _CI_EYESMOOTHAXIS:
			case _CI_TARGETSMOOTHAXIS:
				{
					bool	bWorldSmooth;

					if (nScan == _CI_EYESMOOTHAXIS)
						bWorldSmooth = pCam->bEyeWorldSmooth;
					else
						bWorldSmooth = pCam->bTargetWorldSmooth;

					if (bWorldSmooth)
						sprintf(Str,g_CamSmoothAxisName[1]);
					else
						sprintf(Str,g_CamSmoothAxisName[0]);
				}
				break;

			case _CI_EYEATTACH:
			case _CI_TARGETATTACH:
				{
					int nType;

					if (nScan == _CI_EYEATTACH)
						nType = pCam->nEye;
					else
						nType = pCam->nTarget;

					char Temp[1024];
					strncpyShorten(Temp,g_CamAttachName[nType],15);
					sprintf(Str,"%s",Temp);
				}
				break;

			case _CI_FOV:
				sprintf(Str,"%.2f",pCam->fFov);
				break;

			case _CI_SWAY:
				sprintf(Str,"%d",pCam->nSway);
				break;

			case _CI_SHOCK:
				sprintf(Str,"%d",pCam->nShock);
				break;

			case _CI_DUR:
				sprintf(Str,"%03d",pCam->nDuration);
				break;

			case _CI_ROLL:
				sprintf(Str,"%03d",pCam->nRoll);
				break;

			case _CI_DOF:
				if (pCam->nDOF < 0 || pCam->nDOF > 2)
					pCam->nDOF = 2; 

				sprintf(Str, g_CamDofName[pCam->nDOF]);
				break;

			case _CI_DOFNEAR:
				sprintf(Str,"%.1f",pCam->fDOFNear / 100.0f );
				break;

			case _CI_DOFFAR:
				sprintf(Str,"%.0f",pCam->fDOFFar / 100.0f );
				break;

			case _CI_EYEHMOD:
				if (pCam->bEyeHeightMod)
					sprintf(Str,g_CamOnOffText[1]);
				else
					sprintf(Str,g_CamOnOffText[0]);
				break;

			case _CI_PARAM0:
				sprintf(Str,"%03d",pCam->nParam1);
				break;
			}

			g_pFont->DrawUIText(g_nItemX[nScan],nY, -1, -1, Str);
		}
	}
	else
	{
		g_pFont->DrawUIText(g_nItemX[0],nY,  -1, -1, "@(scale=0.6,0.8) -- Press A to add a new cam. --");
	}
}



void	CCamEditor :: ModifyCamSeq(void)
{
	if(g_MenuHandle->GetCurMenuType() == _FC_MENU_DEBUG)
		return;


	CAMSTATUS *pStatus;
	CAMSTATUS *pCamST;
	bool		bSomethingHappened = false;

	if (m_bCamPlaying == false)
	{
		if (CInputPad::GetInstance().GetKeyPressTick( -1, PAD_INPUT_LTRIGGER ) &&
			CInputPad::GetInstance().GetKeyPressTick( -1, PAD_INPUT_RTRIGGER ) &&
			CInputPad::GetInstance().GetKeyPressTick( -1, PAD_INPUT_A ) &&
			CInputPad::GetInstance().GetKeyPressTick( -1, PAD_INPUT_X ))
		{
			bSomethingHappened = true;
			m_pCamera->DelSeq(-1);
		}

		if(CInputPad::GetInstance().GetKeyPressTick( -1, PAD_INPUT_LTRIGGER ) == 1)
		{
			bSomethingHappened = true;

			if( CInputPad::GetInstance().GetKeyPressTick( -1, PAD_INPUT_RTHUMB ) > 0 ||
				CInputPad::GetInstance().GetKeyPressTick( -1, PAD_INPUT_LTHUMB ) > 0 )
			{
				if( CInputPad::GetInstance().GetKeyPressTick( -1, PAD_INPUT_RTHUMB ) > 0 &&
					CInputPad::GetInstance().GetKeyPressTick( -1, PAD_INPUT_LTHUMB ) > 0 )
					m_nCurSeqId -= 100;
				else
					m_nCurSeqId -= 10;
			}
			else
				m_nCurSeqId--;
						
			if (m_nCurSeqId < 0)
				m_nCurSeqId = 0;

			pStatus = m_pCamera->SeqGetCam(m_nCurSeqId,0);

			if (pStatus != NULL)
				SetCamStatus(pStatus);

			m_nCurCamId = 0;
			m_nCurItem = -1;
							
		}

		if(CInputPad::GetInstance().GetKeyPressTick( -1, PAD_INPUT_RTRIGGER ) == 1)
		{
			bSomethingHappened = true;

			if( CInputPad::GetInstance().GetKeyPressTick( -1, PAD_INPUT_RTHUMB ) > 0 ||
				CInputPad::GetInstance().GetKeyPressTick( -1, PAD_INPUT_LTHUMB ) > 0 )
			{
				if( CInputPad::GetInstance().GetKeyPressTick( -1, PAD_INPUT_RTHUMB ) > 0 &&
					CInputPad::GetInstance().GetKeyPressTick( -1, PAD_INPUT_LTHUMB ) > 0 )
					m_nCurSeqId += 100;
				else
					m_nCurSeqId += 10;			
			}
			else
				m_nCurSeqId++;

			pStatus = m_pCamera->SeqGetCam(m_nCurSeqId,0);

			if (pStatus != NULL)
				SetCamStatus(pStatus);

			m_nCurCamId = 0;
			m_nCurItem = -1;
		}

		if( CInputPad::GetInstance().GetKeyPressTick(-1, PAD_INPUT_A) == 1 && m_bPopUp == false)
		{
			bSomethingHappened = true;

			if (m_nCurCamId >= m_pCamera->SeqGetSize(m_nCurSeqId))
			{
				if (m_pCamera->SeqGetSize(m_nCurSeqId) <= 0)
					m_pCamera->NewSeq(m_nCurSeqId);

//				m_pCamera->GetCamStatus()->nEyeSmooth = m_Cam.nEyeSmooth;
//				m_pCamera->GetCamStatus()->nTargetSmooth = m_Cam.nTargetSmooth;

				m_pCamera->SeqAddCam(m_nCurSeqId, m_pCamera->GetCamStatus());
			
			}
			else
			{
				pCamST = m_pCamera->SeqGetCam(m_nCurSeqId,m_nCurCamId);

				if (m_bCamCopied == true && pCamST != NULL)
				{
					D3DXVECTOR3	*pEye;
					D3DXVECTOR3	*pTarget;

					pEye = m_Cam.Eye;
					pTarget = m_Cam.Target;

					memcpy(&m_Cam, &m_CamCopied, sizeof(CamEditing));

					m_Cam.Eye = pEye;
					m_Cam.Target = pTarget;

					*(m_Cam.Eye) = m_CamCopied._Eye;
					*(m_Cam.Target) = m_CamCopied._Target;

					CamCoordToGlobal(&m_CurEyeGlobal, &m_CurTargetGlobal, &m_Cam);
					
					m_CurEyeLocal = m_Cam._Eye;
					m_CurTargetLocal = m_Cam._Target;

					pCamST = m_pCamera->SeqGetCam(m_nCurSeqId, m_nCurCamId);

					if (pCamST != NULL)
					{
						CamEditing2CamStatus(pCamST, &m_Cam);
						SetCamStatus(pCamST);
					}
				}
			}
		}

		if( CInputPad::GetInstance().GetKeyPressTick(-1, PAD_INPUT_X) == 1 )
		{
			bSomethingHappened = true;

			if (m_nCurCamId >= 0 && m_nCurCamId < m_pCamera->SeqGetSize(m_nCurSeqId))
				m_pCamera->SeqDelCam(m_nCurSeqId, m_nCurCamId);

			if( m_pCamera->SeqGetSize(m_nCurSeqId) == 0 )
				m_nCurCamId = 0;

			pCamST = m_pCamera->SeqGetCam(m_nCurSeqId, m_nCurCamId);

			if (pCamST != NULL)
				SetCamStatus(pCamST);
		}

		if( CInputPad::GetInstance().GetKeyPressTick(-1, PAD_INPUT_Y) == 1 )
		{
			bSomethingHappened = true;

			m_bCamCopied = true;
			memcpy(&m_CamCopied, &m_Cam, sizeof(CamEditing));
			m_CamCopied.Eye = &m_CamCopied._Eye;
			m_CamCopied.Target = &m_CamCopied._Target;
			m_CamCopied._Eye = *(m_Cam.Eye);
			m_CamCopied._Target = *(m_Cam.Target);
		}
	}

	if( CInputPad::GetInstance().GetKeyPressTick(-1, PAD_INPUT_B) == 1 )
	{
		bSomethingHappened = true;

		if (m_bCamPlaying == true)
		{
			CAMSTATUS *pCamSt;

			SeqStop();

			pCamSt = m_pCamera->SeqGetCam(m_nCurSeqId, 0);

			if (pCamSt != NULL)
				SetCamStatus(pCamSt);

			m_nCurCamId = 0;
			m_pCamera->SetEditMode(true);
		}
		else
		{
			m_pCamera->SetEditMode(false);
			SeqPlay();
		}
	}

	if (bSomethingHappened == true)
		PopUpClose();
}

bool	CCamEditor :: ModifyCamStatus(CamEditing *pCam)
{
	int			nNewAttach;
	bool		bCamChanged = false;
	bool		bSaveCam = false;
	D3DXVECTOR3	Dummy;
	CAMSTATUS *pCamSt;

	if (pCam == NULL)
	{
		PopUpClose();
		m_nCurItem = -1;
	}
	else
	{
		if(	CInputPad::GetInstance().GetKeyPressTick( -1, PAD_INPUT_LEFT ) == 1 &&
			CInputPad::GetInstance().GetKeyPressTick( -1, PAD_INPUT_UP ) == 0 && 
			CInputPad::GetInstance().GetKeyPressTick( -1, PAD_INPUT_DOWN ) == 0)
		{
			PopUpClose();
			m_nCurItem = m_nCurItem --;

			if (m_nCurItem == -1)
				bSaveCam = true;

			if (m_nCurItem < -1)
				m_nCurItem = CAM_STATUS_ITEM_MAX - 1;
		}

		if(	CInputPad::GetInstance().GetKeyPressTick( -1, PAD_INPUT_RIGHT ) == 1&&
			CInputPad::GetInstance().GetKeyPressTick( -1, PAD_INPUT_UP ) == 0 && 
			CInputPad::GetInstance().GetKeyPressTick( -1, PAD_INPUT_DOWN ) == 0)
		{
			PopUpClose();
			m_nCurItem++;

			if (m_nCurItem >= CAM_STATUS_ITEM_MAX)
				m_nCurItem = -1;

			if (m_nCurItem == -1)
				bSaveCam = true;
		}

		if (bSaveCam == true)
		{
			pCamSt = m_pCamera->SeqGetCam(m_nCurSeqId, m_nCurCamId);

			if (pCamSt != NULL)
				CamEditing2CamStatus(pCamSt, &m_Cam);		
		}
	}


	bool ThumbPressed = (CInputPad::GetInstance().GetKeyPressTick(-1, PAD_INPUT_RTHUMB) > 0 ||
				CInputPad::GetInstance().GetKeyPressTick(-1, PAD_INPUT_LTHUMB) > 0);

	int	 nSign = 0;
	int	 nCount = 0;

	nCount = CInputPad::GetInstance().GetKeyPressTick(-1, PAD_INPUT_UP);
	if(	nCount > 0 )
		nSign = 1;
	else 
	{
		nCount = CInputPad::GetInstance().GetKeyPressTick(-1, PAD_INPUT_DOWN);
		nSign = -1;
	}
	
	if (nSign != 0 && nCount == 1 || nCount > 20)
	{
		switch(m_nCurItem)
		{


		case _CI_EYESTEP:
			if( ThumbPressed )
				pCam->nEyeSmooth+= 10 * nSign;
			else
				pCam->nEyeSmooth+= nSign;

			break;

		case _CI_TARGETSTEP:

			if( ThumbPressed )
				pCam->nTargetSmooth+= 10 * nSign;
			else
				pCam->nTargetSmooth+= nSign;

			break;


		case _CI_FOV:
			
			if( ThumbPressed )
				pCam->fFov += .1f * (float) nSign;
			else
				pCam->fFov += .005f * (float) nSign;

			
			if (pCam->fFov > 2.5f)
				pCam->fFov = 2.5f;

			if (pCam->fFov < 0.1f)
				pCam->fFov = 0.1f;

			break;



		case _CI_DUR:
			if( ThumbPressed )
				pCam->nDuration += 10 * nSign;
			else
				pCam->nDuration += nSign;

			if (pCam->nDuration <= 0)
				pCam->nDuration = 0;


			break;

		case _CI_ROLL:
			if( ThumbPressed )
				pCam->nRoll += 10 * nSign;
			else
				pCam->nRoll += nSign;

			break;
		

		case _CI_DOFNEAR:
			if( ThumbPressed )
				pCam->fDOFNear += 100.f * (float) nSign;
			else
				pCam->fDOFNear += 10.f * (float) nSign;
			break;

		case _CI_DOFFAR:
			if( ThumbPressed )
				pCam->fDOFFar += 10000.f * (float) nSign;
			else
				pCam->fDOFFar += 500.f * (float) nSign;
			break;
	
	
		case _CI_EYEATTACH:

			if (PopUpOpen() == false)
			{
				if( ThumbPressed )
					nNewAttach = (pCam->nEye + CAM_ATTACH_MAX - 10 * nSign) % CAM_ATTACH_MAX;
				else
					nNewAttach = (pCam->nEye + CAM_ATTACH_MAX - nSign) % CAM_ATTACH_MAX;

				if (IsLocalAttach(pCam->nTarget))
				{
					while (IsLocalAttach(nNewAttach))
						nNewAttach = (nNewAttach + CAM_ATTACH_MAX - nSign) % CAM_ATTACH_MAX;
				}

				if (nNewAttach == CAM_ATTACH_FREE)
					CamCoordToGlobal(&m_CurEyeGlobal, &Dummy, pCam);
	
				pCam->nEye = nNewAttach;
			}

			m_nPopUpCsr = pCam->nEye;
			break;

		case _CI_TARGETATTACH:

			if (PopUpOpen() == false)
			{
				if( ThumbPressed )
					nNewAttach = (pCam->nTarget + CAM_ATTACH_MAX * 10 - 10 * nSign) % CAM_ATTACH_MAX;
				else
					nNewAttach = (pCam->nTarget + CAM_ATTACH_MAX - nSign) % CAM_ATTACH_MAX;

				if (IsLocalAttach(pCam->nEye))
				{
					while (IsLocalAttach(nNewAttach))
						nNewAttach = (nNewAttach + CAM_ATTACH_MAX - nSign) % CAM_ATTACH_MAX;
				}

				if (nNewAttach == CAM_ATTACH_FREE)
					CamCoordToGlobal(&Dummy, &m_CurTargetGlobal, pCam);

				pCam->nTarget = nNewAttach;
			}

			m_nPopUpCsr = pCam->nTarget;
			break;

		case _CI_SWAY:
			pCam->nSway = (pCam->nSway + nSign + CAM_SWAY_MAX) % CAM_SWAY_MAX;
			break;

		case _CI_SHOCK:
			pCam->nShock = (pCam->nShock + nSign + CAM_SWAY_MAX) % CAM_SWAY_MAX;
			break;

		case _CI_DOF:
			
			if (PopUpOpen() == false)
				pCam->nDOF = (pCam->nDOF + 3 + nSign) % 3;

			m_nPopUpCsr = pCam->nDOF;
			break;

		case _CI_EYEHMOD:

			if (PopUpOpen() == false)
				pCam->bEyeHeightMod = !pCam->bEyeHeightMod;

			if (pCam->bEyeHeightMod)
				m_nPopUpCsr = 1;
			else
				m_nPopUpCsr = 0;
			
			break;

		case _CI_PARAM0:
			pCam->nParam1 = (pCam->nParam1 + nSign + 100) % 100;
			break;

		case	_CI_EYESMOOTHAXIS:

			if (PopUpOpen() == false)
				pCam->bEyeWorldSmooth = !pCam->bEyeWorldSmooth;

			if (pCam->bEyeWorldSmooth)
				m_nPopUpCsr = 1;
			else
				m_nPopUpCsr = 0;			
			break;

		case _CI_TARGETSMOOTHAXIS:

			if (PopUpOpen() == false)
				pCam->bTargetWorldSmooth = !pCam->bTargetWorldSmooth;

			if (pCam->bTargetWorldSmooth)
				m_nPopUpCsr = 1;
			else
				m_nPopUpCsr = 0;			
			break;

		case -1:

			pCamSt = m_pCamera->SeqGetCam(m_nCurSeqId, m_nCurCamId);

			if (pCamSt != NULL)
				CamEditing2CamStatus(pCamSt, &m_Cam);

			m_nCurCamId -= nSign;

			if (m_nCurCamId < 0)
				m_nCurCamId = 0;

			if (m_nCurCamId >= m_pCamera->SeqGetSize(m_nCurSeqId))
				m_nCurCamId = m_pCamera->SeqGetSize(m_nCurSeqId);

			pCamSt = m_pCamera->SeqGetCam(m_nCurSeqId, m_nCurCamId);

			if (pCamSt != NULL)
				SetCamStatus(pCamSt);

			break;


		}

		if (m_nCurItem >= 0)
			bCamChanged = true;		
	}
	
	return (bCamChanged);
}


bool	CCamEditor :: PopUpOpen(void)
{
	if (m_bPopUp == true)
		return false;

	m_bPopUp = true;
	PopUpClear();
	int	nScan;

	switch (m_nCurItem)
	{
	case _CI_EYEATTACH:
	case _CI_TARGETATTACH:

		for(nScan =0 ;nScan < CAM_ATTACH_MAX; nScan++)
			PopUpAdd(g_CamAttachName[nScan]);

		break;

	case _CI_SWAY:

		for(nScan = 0;nScan < CAM_SWAY_TYPE_MAX; nScan++)
			PopUpAdd(g_CamSwayName[nScan]);

		break;

	case _CI_DOF:

		for(nScan = 0;nScan < CAM_DOF_MAX; nScan++)
			PopUpAdd(g_CamDofName[nScan]);
		break;

	case _CI_EYEHMOD:
		for(nScan = 0;nScan < CAM_TOGGLE_MAX; nScan++)
			PopUpAdd(g_CamOnOffText[nScan]);
		break;

	case _CI_EYESMOOTHAXIS:
	case _CI_TARGETSMOOTHAXIS:
		for(nScan = 0;nScan < CAM_TOGGLE_MAX; nScan++)
			PopUpAdd(g_CamSmoothAxisName[nScan]);
		break;
	}
	
	if (m_nCurItem >= 0 && m_nCurItem < CAM_STATUS_ITEM_MAX - 1 )
	{
		m_nPopUpWinWidth = g_nItemX[m_nCurItem + 1] - g_nItemX[m_nCurItem];
		m_nPopUpX = g_nItemX[m_nCurItem];
	}
	else if (m_nCurItem == CAM_STATUS_ITEM_MAX - 1)
	{
		m_nPopUpX = g_nItemX[m_nCurItem];
		m_nPopUpWinWidth = 1280 - g_nItemX[m_nCurItem];
	}

	m_nPopUpWinHeight = m_nPopUpSize;

	if (m_nPopUpWinHeight > 14)
		m_nPopUpWinHeight = 14;

	m_nPopUpOffset = m_nPopUpWinHeight / 2;

	m_nPopUpY = m_nEditMenuY - 20 * m_nPopUpOffset;

	if (m_nPopUpY + (m_nPopUpWinHeight * 20) > 700)
	{
		m_nPopUpY = 700 - m_nPopUpWinHeight * 20;
		m_nPopUpOffset = (m_nEditMenuY - m_nPopUpY) / 20;
	}

	m_nPopUpCsr = 0;

	return true;
}

void	CCamEditor :: PopUpProcess(void)
{
	if (m_bPopUp == true && m_nPopUpSize > 0)
	{
		int	nScan;
		char Buf[1024];
		int	nCsr = (m_nPopUpCsr - m_nPopUpOffset + m_nPopUpSize) % m_nPopUpSize;

		g_BsKernel.DrawUIBox_s(_Ui_Mode_Box, m_nPopUpX + 5, m_nPopUpY + 5, m_nPopUpWinWidth, m_nPopUpWinHeight * 20 + 10, 0.0f, D3DCOLOR_RGBA(0,0,0,180));
		g_BsKernel.DrawUIBox_s(_Ui_Mode_Box, m_nPopUpX, m_nPopUpY, m_nPopUpWinWidth, m_nPopUpWinHeight * 20 + 10, 0.0f, D3DCOLOR_RGBA(80,80,120,255));

		for(nScan =0 ;nScan < m_nPopUpWinHeight;nScan++)
		{
			if (nCsr == m_nPopUpCsr)
				strcpy(Buf,"@(scale=0.6,0.8)@(color=255,255,255,255)");
			else
				strcpy(Buf,"@(scale=0.6,0.8)@(color=180,180,190,255)");

			strncat(Buf, m_ppPopUp[nCsr], 500);
			g_pFont->DrawUIText(m_nPopUpX,m_nPopUpY + nScan * 20, -1, -1, Buf);
			nCsr = (nCsr + 1) % m_nPopUpSize;
			
		}
	}
}

void	CCamEditor :: PopUpClose(void)
{
	if (m_bPopUp == false)
		return; 

	m_bPopUp = false;
}

void	CCamEditor :: PopUpClear(void)
{
	m_nPopUpSize = 0;
}

void	CCamEditor :: PopUpAdd(char *pStr)
{
	if (m_nPopUpSize >= m_nPopUpActualSize)
	{
		m_nPopUpActualSize++;
		m_ppPopUp = (char **) realloc (m_ppPopUp, sizeof(char **) * m_nPopUpActualSize);
		m_ppPopUp[m_nPopUpSize] = (char*) malloc(512);
	}

	strncpy_s(m_ppPopUp[m_nPopUpSize],512, pStr,_TRUNCATE); //aleksger - safe string
	m_nPopUpSize++;
}