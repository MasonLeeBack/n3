#include "stdafx.h"

#include "FcMenuOption.h"
#include "BSuiSystem.h"

#include "BsUiClassIDManager.h"
#include "BsUiFontAttrManager.h"

#include "FcInterfaceManager.h"
#include "FcGlobal.h"

#include "InputPad.h"
#include "FcSoundManager.h"

#include "FantasyCore.h"
#include "FcLiveManager.h"
#include "BsMoviePlayer.h"


#ifdef _XBOX
#include "atgsignin.h"
#include "FcLiveManager.h"
#endif //_XBOX

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


CFcMenuOption::CFcMenuOption(_FC_MENU_TYPE nType, _FC_MENU_TYPE nPreType)
: CFcMenuForm(nType)
{
	m_nPreType = nPreType;

	m_pOption0 = new BsUiLayer();
	AddUiLayer(m_pOption0, SN_Option, -1, -1);

	m_pOption1 = new BsUiLayer();
	AddUiLayer(m_pOption1, SN_Option1, -1, -1);

	m_pOption2 = new CFcMenuOpt2Layer(this);
	AddUiLayer(m_pOption2, SN_Option2, -1, -1);
	m_pOption2->Initialize();

#ifdef _XBOX
	m_bXUIVibration = g_FC.GetLiveManager()->m_bController_Vibration;
#else
	m_bXUIVibration = false;
#endif
	g_FCGameData.ConfigInfo.bVibration = m_bXUIVibration;
	CInputPad::GetInstance().SetVibrationOn(g_FCGameData.ConfigInfo.bVibration);

	BsUiSlider* pSlider = (BsUiSlider*)(m_pOption2->GetWindow(SD_OptionVib));
	pSlider->SetBarPos(!m_bXUIVibration ? 1 : 0);

	pSlider->SetWindowAttr(XWATTR_DISABLE, !m_bXUIVibration);
}

void CFcMenuOption::Process()
{
#ifdef _XBOX
	bool bXUIVibration = g_FC.GetLiveManager()->m_bController_Vibration;
#else
	bool bXUIVibration = false;
#endif
	if(m_bXUIVibration == bXUIVibration){
		return;
	}

	m_bXUIVibration = bXUIVibration;
	g_FCGameData.ConfigInfo.bVibration = m_bXUIVibration;
	CInputPad::GetInstance().SetVibrationOn(g_FCGameData.ConfigInfo.bVibration);
		
	BsUiSlider* pSlider = (BsUiSlider*)(m_pOption2->GetWindow(SD_OptionVib));
	pSlider->SetBarPos(!m_bXUIVibration ? 1 : 0);

	pSlider->SetWindowAttr(XWATTR_DISABLE, !m_bXUIVibration);
	
}

void CFcMenuOption::RenderProcess()
{
	switch(GetStatus())
	{
	case _MS_OPEN:
		{	
			m_pOption0->SetColor(D3DXCOLOR(1, 1, 1, 0.f));
			m_pOption1->SetColor(D3DXCOLOR(1, 1, 1, 0.f));
			m_pOption2->SetColor(D3DXCOLOR(1, 1, 1, 0.f));

			SetStatus(_MS_OPENING);
			break;
		}
	case _MS_OPENING:
		{
			int nTick = g_BsUiSystem.GetMenuTick() - m_nFormTick;
			RenderMoveLayer(nTick);

			if(nTick >= _TICK_END_OPENING_CLOSING){
				SetStatus(_MS_NORMAL);
			}
			break;
		}
	case _MS_NORMAL: break;
	case _MS_CLOSING:
		{
			int nTick = g_BsUiSystem.GetMenuTick() - m_nFormTick;
			RenderMoveLayer(_TICK_END_OPENING_CLOSING - nTick);

			if(nTick >= _TICK_END_OPENING_CLOSING){
				SetStatus(_MS_CLOSE);
			}
			break;
		}
	case _MS_CLOSE:	break;
	}
}

#define _START_POS_X_OPT1				-450
#define _END_POS_X_OPT1					0
#define _START_POS_X_OPT2				880
#define _END_POS_X_OPT2					400

void CFcMenuOption::RenderMoveLayer(int nTick)
{
	//------------------------------------------------------
	//opt 0
	if(nTick <= _TICK_FADE_IN_BASE)
	{
		float fAlpha = (float)nTick / _TICK_FADE_IN_BASE;
		m_pOption0->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
	}

	//------------------------------------------------------
	//opt 1
	if(nTick <= _TICK_IN_FORM_LEFT)
	{
		float frate = 1.f - ((float)nTick / _TICK_IN_FORM_LEFT);
		int nX = (int)(_START_POS_X_OPT1 * frate);
		m_pOption1->SetWindowPosX(nX);
		
		float fAlpha = 1.f - frate;
		m_pOption1->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
	}


	//------------------------------------------------------
	//opt 2
	if(nTick <= _TICK_IN_FORM_RIGHT)
	{
		float frate = (float)nTick / _TICK_IN_FORM_RIGHT;
		int nX = _END_POS_X_OPT2 + int((_START_POS_X_OPT2 - _END_POS_X_OPT2) * (1.f - frate));
		m_pOption2->SetWindowPosX(nX);

		float fAlpha = frate;
		m_pOption2->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
	}
}


//-----------------------------------------------------------------------------------------------------
CFcMenuOpt2Layer::CFcMenuOpt2Layer(CFcMenuForm* pMenu)
: BsUiLayer()
{
	m_pMenu = pMenu;
	m_bOptionKey = false;
}

void CFcMenuOpt2Layer::Initialize()
{
	//-------------------------------------------------------------------
	//sound
	BsUiSlider* pSlider = NULL;
	pSlider = (BsUiSlider*)GetWindow(SD_OptionBGM);
	pSlider->SetLinkWindow(GetWindow(BT_OptionBGM));
	pSlider->SetBarPos(g_FCGameData.ConfigInfo.nBGMVolume);

	pSlider = (BsUiSlider*)GetWindow(SD_OptionSE);
	pSlider->SetLinkWindow(GetWindow(BT_OptionSE));
	pSlider->SetBarPos(g_FCGameData.ConfigInfo.nSoundVolume);

	pSlider = (BsUiSlider*)GetWindow(SD_OptionVoice);
	pSlider->SetLinkWindow(GetWindow(BT_OptionVoice));
	pSlider->SetBarPos(g_FCGameData.ConfigInfo.nVoiceVolume);

	//-------------------------------------------------------------------
	//camera
	pSlider = (BsUiSlider*)GetWindow(SD_OptionCamY);
	pSlider->SetLinkWindow(GetWindow(BT_OptionCamY));
	pSlider->ClearItem();
	pSlider->AddItem(_TEX_CAM_NARMAL, 0);
	pSlider->AddItem(_TEX_CAM_INVERTED, 1);
	pSlider->SetMaxRange(1);
	pSlider->SetBarPos(g_FCGameData.ConfigInfo.bCameraUD_Reverse ? 1 : 0);

	pSlider = (BsUiSlider*)GetWindow(SD_OptionCamX);
	pSlider->SetLinkWindow(GetWindow(BT_OptionCamX));
	pSlider->ClearItem();
	pSlider->AddItem(_TEX_CAM_NARMAL, 0);
	pSlider->AddItem(_TEX_CAM_INVERTED, 1);
	pSlider->SetMaxRange(1);
	pSlider->SetBarPos(g_FCGameData.ConfigInfo.bCameraLR_Reverse ? 1 : 0);

	//-------------------------------------------------------------------
	//vibration
	pSlider = (BsUiSlider*)GetWindow(SD_OptionVib);
	pSlider->SetLinkWindow(GetWindow(BT_OptionVib));
	pSlider->ClearItem();
	pSlider->AddItem(_TEX_ON, 0);
	pSlider->AddItem(_TEX_OFF, 1);
	pSlider->SetMaxRange(1);
	pSlider->SetBarPos(!g_FCGameData.ConfigInfo.bVibration ? 1 : 0);

	//-------------------------------------------------------------------
	//contrast
	pSlider = (BsUiSlider*)GetWindow(SD_Contrast);
	pSlider->SetLinkWindow(GetWindow(BT_OptionContrast));
	pSlider->SetBarPos(g_FCGameData.ConfigInfo.nContrast);

	SetFocusWindowClass(SN_Option2);
}

DWORD CFcMenuOpt2Layer::ProcMessage(xwMessageToken* pMsgToken)
{
	BsAssert(pMsgToken);
	switch(pMsgToken->message)
	{
	case XWMSG_KEYDOWN:
		{
			switch (pMsgToken->lParam)
			{
			case MENU_INPUT_B:
				{
					m_pMenu->SetStatus(_MS_CLOSING);
#ifdef _XBOX				
					if(g_FCGameData.m_DeviceID != XCONTENTDEVICE_ANY
						&& ATG::SignIn::IsUserSignedIn(g_FCGameData.nEnablePadID))
						g_FCGameData.SaveFixedGameData();
#endif
					_FC_MENU_TYPE nPreMenuType = ((CFcMenuOption*)m_pMenu)->GetPreMenuType();
					switch(nPreMenuType)
					{
					case _FC_MENU_TITLE:	g_MenuHandle->PostMessage(fcMSG_TITLE_START); break;
					case _FC_MENU_PAUSE:	g_MenuHandle->PostMessage(fcMSG_PAUSE_START); break;
					case _FC_MENU_USUAL:	g_MenuHandle->PostMessage(fcMSG_SUB_TO_USUAL); break;
					default: BsAssert(0);
					}

					break;
				}
			case MENU_INPUT_UP:
				{
					BsUiWindow* pPrev = SetFocusPrevChileWindow(pMsgToken->hWnd, true);
					if(pPrev != NULL && pPrev->GetHWnd() != pMsgToken->hWnd){
						g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CURSOR");
					}
					break;
				}	
			case MENU_INPUT_DOWN:
				{
					BsUiWindow* pNext = SetFocusNextChileWindow(pMsgToken->hWnd, true);
					if(pNext != NULL && pNext->GetHWnd() != pMsgToken->hWnd){
						g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CURSOR");
					}
					break;
				}
			default:
				{
					BsUiCLASS hClass = BsUi::BsUiGetClass(pMsgToken->hWnd);
					switch(hClass)
					{
					//sound
					case BT_OptionBGM:		OnKeyDownBtBGM(pMsgToken); break;
					case BT_OptionSE:		OnKeyDownBtSE(pMsgToken); break;
					case BT_OptionVoice:	OnKeyDownBtVoice(pMsgToken); break;
					//cam
					case BT_OptionCamY:		OnKeyDownBtCamY(pMsgToken); break;
					case BT_OptionCamX:		OnKeyDownBtCamX(pMsgToken); break;
					//vib
					case BT_OptionVib:		OnKeyDownBtVib(pMsgToken); break;
					//contrast
					case BT_OptionContrast:	OnKeyDownBtConst(pMsgToken); break;
					}
				}
			}
			break;
		}
	}

	return BsUiWindow::ProcMessage(pMsgToken);
}



void CFcMenuOpt2Layer::OnKeyDownBtBGM(xwMessageToken* pMsgToken)
{
	switch (pMsgToken->lParam)
	{
	case MENU_INPUT_LEFT:
		{
			((BsUiSlider*)GetWindow(SD_OptionBGM))->AddBarPos(false);
			break;
		}
	case MENU_INPUT_RIGHT:
		{
			((BsUiSlider*)GetWindow(SD_OptionBGM))->AddBarPos(true);
			break;
		}
	}

	switch (pMsgToken->lParam)
	{
	case MENU_INPUT_LEFT:
	case MENU_INPUT_RIGHT:
		{
			int nPos = ((BsUiSlider*)GetWindow(SD_OptionBGM))->GetBarPos();
			g_FCGameData.ConfigInfo.nBGMVolume = nPos;
			g_pSoundManager->SetVolume(FC_MENU_VOL_BGM, nPos * 0.1f);
			g_BsMoviePlayer.SetVolume( (int)((nPos * 0.1f) * (float)_MAX_BINK_VOLUME) );
		}
	}
}

void CFcMenuOpt2Layer::OnKeyDownBtSE(xwMessageToken* pMsgToken)
{
	switch (pMsgToken->lParam)
	{
	case MENU_INPUT_LEFT:
		{
			((BsUiSlider*)GetWindow(SD_OptionSE))->AddBarPos(false);
			break;
		}
	case MENU_INPUT_RIGHT:
		{
			((BsUiSlider*)GetWindow(SD_OptionSE))->AddBarPos(true);
			break;
		}
	}

	switch (pMsgToken->lParam)
	{
	case MENU_INPUT_LEFT:
	case MENU_INPUT_RIGHT:
		{
			int nPos = ((BsUiSlider*)GetWindow(SD_OptionSE))->GetBarPos();
			g_FCGameData.ConfigInfo.nSoundVolume = nPos;
			g_pSoundManager->SetVolume(FC_MENU_VOL_EFT_SOUND, nPos * 0.1f);
		}
	}
}

void CFcMenuOpt2Layer::OnKeyDownBtVoice(xwMessageToken* pMsgToken)
{
	switch (pMsgToken->lParam)
	{
	case MENU_INPUT_LEFT:
		{
			((BsUiSlider*)GetWindow(SD_OptionVoice))->AddBarPos(false);
			break;
		}
	case MENU_INPUT_RIGHT:
		{
			((BsUiSlider*)GetWindow(SD_OptionVoice))->AddBarPos(true);
			break;
		}
	}

	switch (pMsgToken->lParam)
	{
	case MENU_INPUT_LEFT:
	case MENU_INPUT_RIGHT:
		{
			int nPos = ((BsUiSlider*)GetWindow(SD_OptionVoice))->GetBarPos();
			g_FCGameData.ConfigInfo.nVoiceVolume = nPos;
			g_pSoundManager->SetVolume(FC_MENU_VOL_VOICE, nPos * 0.1f);
		}
	}
}

void CFcMenuOpt2Layer::OnKeyDownBtCamY(xwMessageToken* pMsgToken)
{
	switch(pMsgToken->lParam)
	{
	case MENU_INPUT_LEFT:
	case MENU_INPUT_RIGHT:
		{
			BsUiSlider* pSlider = (BsUiSlider*)GetWindow(SD_OptionCamY);
			switch(pMsgToken->lParam)
			{
			case MENU_INPUT_LEFT:
				{
					pSlider->AddBarPos(false);
					g_FCGameData.ConfigInfo.bCameraUD_Reverse = false;
					break;
				}
			case MENU_INPUT_RIGHT:
				{
					pSlider->AddBarPos(true);
					g_FCGameData.ConfigInfo.bCameraUD_Reverse = true;
					break;
				}
			}

			break;
		}
	}
}


void CFcMenuOpt2Layer::OnKeyDownBtCamX(xwMessageToken* pMsgToken)
{
	switch(pMsgToken->lParam)
	{
	case MENU_INPUT_LEFT:
	case MENU_INPUT_RIGHT:
		{
			BsUiSlider* pSlider = (BsUiSlider*)GetWindow(SD_OptionCamX);
			switch(pMsgToken->lParam)
			{
			case MENU_INPUT_LEFT:
				{
					pSlider->AddBarPos(false);
					g_FCGameData.ConfigInfo.bCameraLR_Reverse = false;
					break;
				}
			case MENU_INPUT_RIGHT:
				{
					pSlider->AddBarPos(true);
					g_FCGameData.ConfigInfo.bCameraLR_Reverse = true;
					break;
				}
			}

			break;
		}
	}
}

void CFcMenuOpt2Layer::OnKeyDownBtVib(xwMessageToken* pMsgToken)
{
	switch(pMsgToken->lParam)
	{
	case MENU_INPUT_LEFT:
	case MENU_INPUT_RIGHT:
		{
			BsUiSlider* pSlider = (BsUiSlider*)GetWindow(SD_OptionVib);
			if(pSlider->IsEnableWindow() == true)
			{
				switch(pMsgToken->lParam)
				{
				case MENU_INPUT_LEFT:
					{
						pSlider->AddBarPos(false);
						g_FCGameData.ConfigInfo.bVibration = true;
						break;
					}
				case MENU_INPUT_RIGHT:
					{
						pSlider->AddBarPos(true);
						g_FCGameData.ConfigInfo.bVibration = false;
						break;
					}
				}

				CInputPad::GetInstance().SetVibrationOn(g_FCGameData.ConfigInfo.bVibration);
			}
			break;
		}
	}
}


void CFcMenuOpt2Layer::OnKeyDownBtConst(xwMessageToken* pMsgToken)
{
	switch (pMsgToken->lParam)
	{
	case MENU_INPUT_LEFT:
		{
			((BsUiSlider*)GetWindow(SD_Contrast))->AddBarPos(false);
			break;
		}
	case MENU_INPUT_RIGHT:
		{
			((BsUiSlider*)GetWindow(SD_Contrast))->AddBarPos(true);
			break;
		}
	}

	switch (pMsgToken->lParam)
	{
	case MENU_INPUT_LEFT:
	case MENU_INPUT_RIGHT:
		{
			int nPos = ((BsUiSlider*)GetWindow(SD_Contrast))->GetBarPos();
			g_FCGameData.ConfigInfo.nContrast = nPos;

			// Write Gamma
			g_BsKernel.SetGammaRamp(float(nPos)/10.f);
			break;
		}
	}
}