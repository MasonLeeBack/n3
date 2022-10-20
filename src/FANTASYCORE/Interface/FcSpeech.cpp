#include "stdafx.h"
#include "FcSpeech.h"
#include "BSKernel.h"
#include "FcInterfaceManager.h"
#include "FcGlobal.h"
#include "TextTable.h"

#include "BsUiUVManager.h"
#include ".\\data\\Menu\\BsUiUVID.h"
#include "FcSoundManager.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

#define SPEECH_TIME_MIN		120
#define TICK_PER_CHAR		3

//speech
#define _SPEECH_X				352
#define _SPEECH_Y				525
#define _SPEECH_WIDTH			700
#define _SPEECH_HEIGHT			140
#define _SPEECH_TEXT_X_GAP		46
#define _SPEECH_TEXT_Y_GAP		15
#define _SPEECH_PORTRAIT_X		998
#define _SPEECH_PORTRAIT_Y		420
#define SPEECH_PORTRAIT_SCALE	1.f
#define _SPEECH_X_MOVE_SPEED	25

//report
#define _REPORT_POS_Y			300
#define _REPORT_POS_SY			48
#define _REPORT_BAR_GRADE		50

//item report
#define _ITEM_REPORT_POS_Y		360
#define _ITEM_REPORT_POS_SY		48

//mission goal
#define _MISSION_GOAL_POS_Y		240
#define _MISSION_GOAL_POS_SY	48


CFcSpeech::CFcSpeech(CFcPortrait *pPortrait)
{
	m_bSpeechOn = true;
	BsAssert(pPortrait);
	m_pPortrait = pPortrait;

	m_PortX = 0;
	m_nSaveTick = -1;
}

CFcSpeech::~CFcSpeech()
{
	for(unsigned int i = 0;i < m_TextList.size();i++){	
		delete m_TextList[i];
	}
}

void CFcSpeech::Process()
{
	if(m_TextList.size() > 0)
	{
		FC_SPEECH_DATA *pCurData = m_TextList[0];
		if(pCurData->bAddX == false)
		{
			if(pCurData->bRenderOut == true)
			{
				delete m_TextList[0];
				m_TextList.erase( m_TextList.begin() );
			}
		}
	}

	ProcessSpeech();
}


void CFcSpeech::ProcessSpeech()
{	
	// 텍스트 테이블 인덱스가 주어지면
	// 자막을 정해진 속도에 따라 뿌려주고
	// 대사가 마지막 줄을 다 말했거나 혹은 다 보여주면 다음 스크롤로 옮긴다.
	// 대사량이 다 소진됐으면 페이드 아웃(혹은 그냥 없애기)
	
	if(m_TextList.size() == 0){
		return;
	}

	if(GetProcessTick() - m_nSaveTick == 0){
		m_nSaveTick = GetProcessTick();
		return;
	}
	g_BsKernel.SetUIViewerIndex(_UI_FULL_VIEWER);

	D3DVIEWPORT9 ViewPort;
	g_BsKernel.GetDevice()->GetViewport( &ViewPort );
	FC_SPEECH_DATA *pCurData = m_TextList[0];

	if( pCurData->bAddX )
	{
		m_PortX += _SPEECH_X_MOVE_SPEED;
		if( m_PortX >= 0 ){
			m_PortX = 0;
			if( pCurData->nSoundCueIndex != -1 ) //사운드를 사용하면
			{
				pCurData->nSoundHandle = g_pSoundManager->PlaySound( SB_VOICE_IN_GAME, pCurData->nSoundCueIndex, &pCurData->nSoundHandle ); //사운드 플레이
				pCurData->nSoundCueIndex = -1;
			}

			if( pCurData->bSoundUse) {
				if(pCurData->nPlayTime > 0){ //임시코드				
					(pCurData->nPlayTime > 0 ) ? --pCurData->nPlayTime : pCurData->bAddX = false;
				}
				else{
					if( pCurData->nSoundHandle == -1) { pCurData->bAddX = false; }
				}
				
			}
			else{
				(pCurData->nPlayTime > 0 ) ? --pCurData->nPlayTime : pCurData->bAddX = false;
			}

		}
	}
	else
	{		
		if( m_PortX <= -m_pPortrait->GetUVInfo(pCurData->nPortraitID)->u2 ) {		
			m_PortX = -m_pPortrait->GetUVInfo(pCurData->nPortraitID)->u2;
		}
		else{
			m_PortX -= _SPEECH_X_MOVE_SPEED;
		}
	}
}

void CFcSpeech::Update()
{
	if(m_bShow == false){
		return;
	}

	DrawSpeech();
}

void CFcSpeech::DrawSpeech()
{
	if( m_TextList.size() == 0){
		return;
	}

	FC_SPEECH_DATA *pCurData = m_TextList[0];
	if(pCurData->bRenderOut == true){
		return;
	}

	if(m_bSpeechOn == true)
	{	
		// speech center
		UVImage* pImg = g_UVMgr.GetUVImage(_UV_SpeechMessage);
		g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
			_SPEECH_X, _SPEECH_Y, _SPEECH_WIDTH, _SPEECH_HEIGHT,
			0.f, D3DXCOLOR(1, 1, 1, 0.7f),
			0.f, pImg->nTexId,
			pImg->u1, pImg->v1, pImg->u2, pImg->v2);

		//portrait
		
		int su = m_pPortrait->GetUVInfo(pCurData->nPortraitID)->u1;
		int eu = m_pPortrait->GetUVInfo(pCurData->nPortraitID)->u2;
		int Scalex = eu - su;

		int sv = m_pPortrait->GetUVInfo(pCurData->nPortraitID)->v1;
		int ev = m_pPortrait->GetUVInfo(pCurData->nPortraitID)->v2;
		int Scaley = ev - sv;

		int nX = _SCREEN_WIDTH - (int)((Scalex + m_PortX) * SPEECH_PORTRAIT_SCALE) - 64;
		int nY = _SCREEN_HEIGHT - (int)(Scaley * SPEECH_PORTRAIT_SCALE) - 36;
		
		m_pPortrait->DrawPortrait(pCurData->nPortraitID,
			nX, nY,
			(int)(Scalex * SPEECH_PORTRAIT_SCALE), (int)(Scaley * SPEECH_PORTRAIT_SCALE),
			1.f);

		//text
		g_TextTable->GetText(pCurData->nTextTableID, m_szStr, _countof(m_szStr));
		g_pFont->DrawUIText(0, 0, -1, -1, "@(reset)@(scale=0.8,0.8)@(space=-4)");
		g_pFont->DrawUIText(_SPEECH_X + _SPEECH_TEXT_X_GAP, _SPEECH_Y + _SPEECH_TEXT_Y_GAP,
			_SPEECH_X + _SPEECH_WIDTH - _SPEECH_TEXT_X_GAP, -1, m_szStr);
	}

	if(pCurData->bAddX == false)
	{
		if(m_PortX <= -m_pPortrait->GetUVInfo(pCurData->nPortraitID)->u2 ) {
			pCurData->bRenderOut = true;
		}
	}
}

void CFcSpeech::SetSpeech(int nPortraitID, int nTextTableID)
{
	int su = m_pPortrait->GetUVInfo(nPortraitID)->u1;
	int eu = m_pPortrait->GetUVInfo(nPortraitID)->u2;
	m_PortX = su - eu;
	
	char szStr[STR_SPEECH_MAX] = {0};
	char szSoundCueName[64] = {0};
	FC_SPEECH_DATA *pData = new FC_SPEECH_DATA;
	g_TextTable->GetText(nTextTableID, szStr, _countof(szStr),szSoundCueName, _countof(szSoundCueName));
	
	pData->nPortraitID = nPortraitID;
	pData->nTextTableID = nTextTableID;
	pData->bAddX = true;
	pData->bRenderOut = false;
	pData->nSoundHandle = -1;

#ifndef _MENU_ONLY_TEXT
	if( strlen(szSoundCueName) > 0 ) //사운드 이름이 있으면
	{		
		pData->nPlayTime = -1;
		pData->bSoundUse = true;
		pData->nSoundCueIndex = g_pSoundManager->GetCueIndex( SB_VOICE_IN_GAME,szSoundCueName );

		//임시코드임
		if(pData->nSoundCueIndex == -1)
		{
			int nWidth, nHeight, nLine;
			int nWordCount(0);
			g_pFont->GetTextLengthInfo(nWidth, nHeight, nLine, nWordCount,
				0, 0, _SCREEN_WIDTH, _SCREEN_HEIGHT, szStr);
			
			pData->nPlayTime =  nWordCount * TICK_PER_CHAR;
			if(pData->nPlayTime < SPEECH_TIME_MIN){
				pData->nPlayTime = SPEECH_TIME_MIN;
			}
		}
		/////////////////////////////////
	}
	else
#endif
	{
		int nWidth, nHeight, nLine;
		int nWordCount(0);
		g_pFont->GetTextLengthInfo(nWidth, nHeight, nLine, nWordCount,
			0, 0, _SCREEN_WIDTH, _SCREEN_HEIGHT, szStr);

		pData->nPlayTime =  nWordCount * TICK_PER_CHAR;
		if(pData->nPlayTime < SPEECH_TIME_MIN){
			pData->nPlayTime = SPEECH_TIME_MIN;
		}

		pData->bSoundUse = false;
		pData->nSoundHandle = -1;
		pData->nSoundCueIndex = -1;
	}
	m_TextList.push_back( pData );

	m_nSaveTick = GetProcessTick();
	
}

bool CFcSpeech::IsPlay()
{
	if(m_bSpeechOn == true)
	{
		if(m_TextList.size() > 0){
			return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
CFcReport::CFcReport()
{
	m_iPlayTime = 0;
	memset(m_szStr, 0, sizeof(char)*STR_SPEECH_MAX);

	m_bPlay = false;
	m_nSaveTick = -1;
	m_nReportX = 0;
	m_nBoxLength = 0;
	m_nSoundHandle = -1;
	m_bSoundUse = false;
	m_bCheckRender = false;
}

void CFcReport::Process()
{
	ProcessCheckReport();

	if(m_bPlay == false){
		return;
	}

	if(GetProcessTick() - m_nSaveTick == 0){
		m_nSaveTick = GetProcessTick();
		return;
	}

	if(m_bCheckRender == false){
		return;
	}

	if(m_bSoundUse)
	{
		if( m_bMoveLeft )
		{
			m_nReportX += m_nBoxLength/10;
			if(m_nReportX >= 0)
			{
				m_nReportX = 0;
				if( m_nSoundHandle == -1)
				{
					m_bMoveLeft = false;
				}
			}
		}
		else
		{
			if(m_nReportX >= -m_nBoxLength)
			{
				m_nReportX -= m_nBoxLength/10;
				if(m_nReportX < -m_nBoxLength)
				{
					m_bPlay = false;
					m_nReportX = -m_nBoxLength;				
					m_nTexList.erase(m_nTexList.begin());
				}
			}
		}
	}
	else{
		if(m_iPlayTime)
		{
			if(m_nReportX < 0)
			{
				m_nReportX += m_nBoxLength/10;
				if(m_nReportX > 0){
					m_nReportX = 0;
				}
			}

			if(m_iPlayTime > 0 && m_nReportX == 0) {
				m_iPlayTime--;
			}
		}
		else
		{
			if(m_nReportX >= -m_nBoxLength)
			{
				m_nReportX -= m_nBoxLength/10;
				if(m_nReportX < -m_nBoxLength)
				{
					m_bPlay = false;
					m_nReportX = -m_nBoxLength;				
					m_nTexList.erase(m_nTexList.begin());
				}
			}	
		}
	}
}

void CFcReport::ProcessCheckReport()
{
	if(m_bPlay == false && m_nTexList.size() > 0)
	{
		m_bPlay = true;

		char szStr[STR_SPEECH_MAX];
		char szSoundCueName[64] = {0};			
		bool result = g_TextTable->GetText(m_nTexList[0], szStr, _countof(szStr),szSoundCueName, _countof(szSoundCueName));

		BsAssert(result && "GetText Failed"); //aleksger: prefix bug 771: GetText may return fail and not initalize strings.
		BsAssert(strlen(szStr) < STR_SPEECH_MAX);
		strcpy(m_szStr,szStr);

		int nWidth, nHeight, nLine;
		int nWordCount(0);
		g_pFont->GetTextLengthInfo(nWidth, nHeight, nLine, nWordCount,
			0, 0, _SCREEN_WIDTH, _SCREEN_HEIGHT, m_szStr);

		m_iPlayTime =  nWordCount;

		if(m_iPlayTime < SPEECH_TIME_MIN){
			m_iPlayTime = SPEECH_TIME_MIN;
		}

		m_bMoveLeft = true;
		if(m_nTexList[0] != _TEX_REPORT_ADD_SKILL)
		{	
			if( strlen( szSoundCueName ) > 0 ){ //사운드가 있는 텍스트면			
				m_nSoundHandle = g_pSoundManager->PlaySound(SB_VOICE_IN_GAME,szSoundCueName,&m_nSoundHandle);
				BsAssert(m_nSoundHandle != -1 && "None voice handle");
				m_bSoundUse = true;
			}
			else {
				m_nSoundHandle = -1;
				m_bSoundUse = false;
			}
		}
		else {
			m_nSoundHandle = -1;
			m_bSoundUse = false;
		}

		m_nSaveTick = GetProcessTick();
		m_bCheckRender = false;

		g_pSoundManager->PlaySystemSound( SB_COMMON, "SYS_REPORT_MES");
	}
}

void CFcReport::SetReport(int nTexId)
{	
	m_nTexList.push_back(nTexId);
}

void CFcReport::Update()
{
	if(m_bShow == false){
		return;
	}

	if(m_bCheckRender == false)
	{
		int nWidth(0), nHeight(0), nLine(0);
		int nWordCount(0);
		g_pFont->DrawUIText(0, 0, -1, -1, "@(reset)@(scale=0.8,0.8)@(space=-2)");
		g_pFont->GetTextLengthInfo(nWidth, nHeight, nLine, nWordCount,
			0, 0, -1, -1, m_szStr);

		m_nBoxLength = nWidth + 40 + 20;
		m_nReportX = -m_nBoxLength;
		m_bCheckRender = true;
	}

	if(m_bPlay)
	{
		UVImage* pImg = g_UVMgr.GetUVImage(_UV_SpeechMessage);
		g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
			m_nReportX - _REPORT_BAR_GRADE, _REPORT_POS_Y, m_nBoxLength + 90, _REPORT_POS_SY,
			0.f, D3DXCOLOR(1, 1, 1, 1),
			0.f, pImg->nTexId,
			pImg->u1, pImg->v1, pImg->u2-_REPORT_BAR_GRADE, pImg->v2);

		g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
			m_nReportX - _REPORT_BAR_GRADE + m_nBoxLength + 90, _REPORT_POS_Y, _REPORT_BAR_GRADE, _REPORT_POS_SY,
			0.f, D3DXCOLOR(1, 1, 1, 1),
			0.f, pImg->nTexId,
			pImg->u2-_REPORT_BAR_GRADE, pImg->v1, pImg->u2, pImg->v2);
	
		g_pFont->DrawUIText(0, 0, -1, -1, "@(reset)@(scale=0.8,0.8)@(space=-2)");
		g_pFont->DrawUIText(m_nReportX + 90, _REPORT_POS_Y+6, -1, -1, m_szStr);
	}
}


//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
CFcItemReport::CFcItemReport()
{
	m_iPlayTime = 0;
	memset(m_szStr, 0, sizeof(char)*STR_SPEECH_MAX);

	m_bPlay = false;
	m_nSaveTick = -1;
	m_nReportX = 0;
	m_nBoxLength = 0;
	m_bCheckRender = false;
}

CFcItemReport::~CFcItemReport()
{
	DWORD dwCount = m_nTexList.size();
	for(DWORD i=0; i<dwCount; i++){
		delete[] m_nTexList[i];
	}

	m_nTexList.clear();
}

void CFcItemReport::Process()
{
	if(m_bPlay == false){
		return;
	}

	if(GetProcessTick() - m_nSaveTick == 0){
		m_nSaveTick = GetProcessTick();
		return;
	}

	if(m_bCheckRender == false){
		return;
	}

	if(m_iPlayTime)
	{		
		if(m_nReportX < 0)
		{
			m_nReportX += m_nBoxLength/10;
			if(m_nReportX > 0){
				m_nReportX = 0;
			}
		}

		if(m_iPlayTime > 0 && m_nReportX == 0){
			m_iPlayTime--;
		}
	}
	else
	{
		if(m_nReportX >= -m_nBoxLength)
		{
			m_nReportX -= m_nBoxLength/10;
			if(m_nReportX < -m_nBoxLength)
			{
				m_nReportX = -m_nBoxLength;
				m_bPlay = false;
			}
		}		
	}	
}


void CFcItemReport::SetItemReport(char* szText)
{
	//char* pText = new char[strlen(szText)] + 1;
	//strcpy(pText, szText);
	//m_nTexList.push_back(pText);

	m_bPlay = true;

	strcpy(m_szStr, szText);
	
	int nWidth, nHeight, nLine;
	int nWordCount(0);
	g_pFont->GetTextLengthInfo(nWidth, nHeight, nLine, nWordCount,
		0, 0, _SCREEN_WIDTH, _SCREEN_HEIGHT, m_szStr);

	m_iPlayTime =  nWordCount;

	if(m_iPlayTime < SPEECH_TIME_MIN){
		m_iPlayTime = SPEECH_TIME_MIN;
	}

	m_nSaveTick = GetProcessTick();
	m_bCheckRender = false;
}

void CFcItemReport::Update()
{
	if(m_bShow == false){
		return;
	}

	if(m_bCheckRender == false)
	{
		int nWidth(0), nHeight(0), nLine(0);
		int nWordCount(0);
		g_pFont->DrawUIText(0, 0, -1, -1, "@(reset)@(scale=0.8,0.8)@(space=-2)");
		g_pFont->GetTextLengthInfo(nWidth, nHeight, nLine, nWordCount,
			0, 0, -1, -1, m_szStr);
		
		m_nBoxLength = nWidth + 40 + 20;
		m_nReportX = -m_nBoxLength;

		m_bCheckRender = true;
	}

	if(m_bPlay)
	{
		UVImage* pImg = g_UVMgr.GetUVImage(_UV_SpeechMessage);
		g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
			m_nReportX - _REPORT_BAR_GRADE, _ITEM_REPORT_POS_Y, m_nBoxLength + 90, _ITEM_REPORT_POS_SY,
			0.f, D3DXCOLOR(1, 1, 1, 1),
			0.f, pImg->nTexId,
			pImg->u1, pImg->v1, pImg->u2 - _REPORT_BAR_GRADE, pImg->v2);

		g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
			m_nReportX - _REPORT_BAR_GRADE + m_nBoxLength + 90, _ITEM_REPORT_POS_Y, _REPORT_BAR_GRADE, _REPORT_POS_SY,
			0.f, D3DXCOLOR(1, 1, 1, 1),
			0.f, pImg->nTexId,
			pImg->u2-_REPORT_BAR_GRADE, pImg->v1, pImg->u2, pImg->v2);

		g_pFont->DrawUIText(0, 0, -1, -1, "@(reset)@(scale=0.8,0.8)@(space=-2)");
		g_pFont->DrawUIText(m_nReportX + 90, _ITEM_REPORT_POS_Y+6, -1, -1, m_szStr);
	}
}

//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
#define _MISSION_OBJECT_BAR_Y				120
#define _MISSION_OBJECT_TEXT_Y				50

CFcMissionGoal::CFcMissionGoal()
{
	m_iPlayTime = 0;
	memset(m_szStr, 0, sizeof(char)*STR_SPEECH_MAX);

	m_bPlay = false;
	m_nSaveTick = -1;
	m_nMoveY = 0;
}

void CFcMissionGoal::Process()
{
	if(m_bPlay == false){
		return;
	}

	if(GetProcessTick() - m_nSaveTick == 0){
		m_nSaveTick = GetProcessTick();
		return;
	}

	if(m_iPlayTime)
	{		
		if(m_nMoveY < _MISSION_OBJECT_BAR_Y)
		{
			m_nMoveY += _MISSION_OBJECT_BAR_Y/10;
			if(m_nMoveY > _MISSION_OBJECT_BAR_Y){
				m_nMoveY = _MISSION_OBJECT_BAR_Y;
			}
		}

		if(m_iPlayTime > 0 && m_nMoveY == _MISSION_OBJECT_BAR_Y){
			m_iPlayTime--;
		}
	}
	else
	{
		if(m_nMoveY >= 0)
		{
			m_nMoveY -= _MISSION_OBJECT_BAR_Y/10;
			if(m_nMoveY < 0)
			{
				m_nMoveY = 0;
				m_bPlay = false;
			}
		}		
	}	
}

void CFcMissionGoal::Update()
{
	if(m_bShow == false){
		return;
	}

	if(m_bPlay)
	{	
		UVImage* pImg = g_UVMgr.GetUVImage(_UV_mission_info_bg);
		g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
			0, -10, _SCREEN_WIDTH, m_nMoveY+10,
			0.f, D3DXCOLOR(1, 1, 1, 1),
			0.f, pImg->nTexId,
			pImg->u1, pImg->v1, pImg->u2, pImg->v2);

		g_pFont->DrawUIText(0, 0, -1, -1, "@(reset)@(align=1)@(space=-3)");
		g_pFont->DrawUIText(0, m_nMoveY - (_MISSION_OBJECT_TEXT_Y + 20), _SCREEN_WIDTH, -1, m_szStr);
	}
}


void CFcMissionGoal::SetMissionGoal(char *szStr)
{
	m_bPlay = true;

	BsAssert(strlen(szStr) < STR_SPEECH_MAX);
	strcpy(m_szStr,szStr);

	int nWidth, nHeight, nLine;
	int nWordCount(0);
	g_pFont->GetTextLengthInfo(nWidth, nHeight, nLine, nWordCount,
		0, 0, _SCREEN_WIDTH, _SCREEN_HEIGHT, m_szStr);

	m_iPlayTime =  nWordCount;
	
	if(m_iPlayTime < SPEECH_TIME_MIN){
		m_iPlayTime = SPEECH_TIME_MIN;
	}

	m_nSaveTick = GetProcessTick();
	m_nMoveY = 0;

	g_pSoundManager->PlaySystemSound( SB_COMMON, "SYS_MISSION_OBJ");
}

//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
CFcSpecialMissionText::CFcSpecialMissionText()
{
	memset(m_szStr, 0, sizeof(char)*STR_SPEECH_MAX);	
}


#define _MISSION_TEXT_BASE_LEFT_X		574
#define _MISSION_TEXT_BASE_MIDDLE_X		604
#define _MISSION_TEXT_BASE_RIGHT_X		674
#define _MISSION_TEXT_BASE_Y			45

#define _MISSION_TEXT_WIDTH				70
#define _MISSION_TEXT_Y					52

void CFcSpecialMissionText::Update()
{
	if(m_bShow == false){
		return;
	}

	if(strlen(m_szStr) == 0){
		return;
	}

	if(g_InterfaceManager.IsPlayMissionGoal()){
		return;
	}

	g_pFont->DrawUIText(0, 0, -1, -1, "@(reset)@(scale=0.8,0.8)@(align=1)");

	int nWidth, nHeight, nLine;
	int nWordCount(0);
	g_pFont->GetTextLengthInfo(nWidth, nHeight, nLine, nWordCount,
		0, 0, _SCREEN_WIDTH, _SCREEN_HEIGHT, m_szStr);
	
	int nleftX = _MISSION_TEXT_BASE_LEFT_X;
	int nMiddleX = _MISSION_TEXT_BASE_MIDDLE_X;
	int nRightX = _MISSION_TEXT_BASE_RIGHT_X;

	if(nWidth > _MISSION_TEXT_WIDTH)
	{
		nWidth = (nWidth%2) ? nWidth + 1 : nWidth;

		int nGap = (nWidth - _MISSION_TEXT_WIDTH) / 2;
		nleftX -= nGap;
		nMiddleX -= nGap;
		nRightX += nGap;
	}
	else{
		nWidth = _MISSION_TEXT_WIDTH;
	}
	
	
	UVImage* pLeft = g_UVMgr.GetUVImage(_UV_mission_num_left);
	g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
		nleftX, _MISSION_TEXT_BASE_Y,
		pLeft->u2 - pLeft->u1, pLeft->v2 - pLeft->v1,
		0.f, D3DXCOLOR(1.f, 1.f, 1.f, 1),
		0.f, pLeft->nTexId,
		pLeft->u1, pLeft->v1, pLeft->u2, pLeft->v2);

	UVImage* pMiddle = g_UVMgr.GetUVImage(_UV_mission_num_center);
	g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
		nMiddleX, _MISSION_TEXT_BASE_Y,
		nWidth, pMiddle->v2 - pMiddle->v1,
		0.f, D3DXCOLOR(1.f, 1.f, 1.f, 1),
		0.f, pMiddle->nTexId,
		pMiddle->u1, pMiddle->v1, pMiddle->u2, pMiddle->v2);

	UVImage* pRight = g_UVMgr.GetUVImage(_UV_mission_num_right);
	g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
		nRightX, _MISSION_TEXT_BASE_Y,
		pRight->u2 - pRight->u1, pRight->v2 - pRight->v1,
		0.f, D3DXCOLOR(1.f, 1.f, 1.f, 1),
		0.f, pRight->nTexId,
		pRight->u1, pRight->v1, pRight->u2, pRight->v2);

	g_pFont->DrawUIText(0, _MISSION_TEXT_Y, _SCREEN_WIDTH, _SCREEN_HEIGHT, m_szStr);
	
	memset(m_szStr, 0, STR_SPEECH_MAX);
}

void CFcSpecialMissionText::SetText(char *szStr)
{
	BsAssert(strlen(szStr) < STR_SPEECH_MAX);
	strcpy(m_szStr,szStr);
}


//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
CFcSpecialMissionGauge::CFcSpecialMissionGauge()
{
	m_nValue = -1;
	m_nMaxValue = -1;
	memset(m_szStr, 0, sizeof(char)*STR_SPEECH_MAX);
}

#define _MISSION_GAUGE_LEFTX	86 
#define _MISSION_GAUGE_MIDDLEX	94
#define _MISSION_GAUGE_RIGHTX	266
#define _MISSION_GAUGE_Y		448

#define _MISSION_GAUGE_BAR_X	90
#define _MISSION_GAUGE_BAR_Y	452
#define _MISSION_GAUGE_BAR_LEN	180

#define _MISSION_GAUGE_TEXT_X	90
#define _MISSION_GAUGE_TEXT_Y	460

void CFcSpecialMissionGauge::Update()
{
	if(m_bShow == false){
		return;
	}

	if(m_nValue < 0){
		return;
	}
	
	//------------------------------------------------
	//left
	UVImage* pLeft = g_UVMgr.GetUVImage(_UV_mission_front);

	g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
		_MISSION_GAUGE_LEFTX, _MISSION_GAUGE_Y,
		pLeft->u2 - pLeft->u1, pLeft->v2 - pLeft->v1,
		0.f, D3DXCOLOR(1.f, 1.f, 1.f, 1),
		0.f, pLeft->nTexId,
		pLeft->u1, pLeft->v1, pLeft->u2, pLeft->v2);

	//------------------------------------------------
	//middle
	UVImage* pMiddle = g_UVMgr.GetUVImage(_UV_mission_middle);

	g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
		_MISSION_GAUGE_MIDDLEX, _MISSION_GAUGE_Y,
		_MISSION_GAUGE_BAR_LEN - 8, pMiddle->v2 - pMiddle->v1,
		0.f, D3DXCOLOR(1.f, 1.f, 1.f, 1),
		0.f, pMiddle->nTexId,
		pMiddle->u1, pMiddle->v1, pMiddle->u2, pMiddle->v2);

	//------------------------------------------------
	//right
	UVImage* pRight = g_UVMgr.GetUVImage(_UV_mission_back);

	g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
		_MISSION_GAUGE_RIGHTX, _MISSION_GAUGE_Y,
		pRight->u2 - pRight->u1, pRight->v2 - pRight->v1,
		0.f, D3DXCOLOR(1.f, 1.f, 1.f, 1),
		0.f, pRight->nTexId,
		pRight->u1, pRight->v1, pRight->u2, pRight->v2);

	//------------------------------------------------
	//bar
	int nValue = (int)(m_nValue * _MISSION_GAUGE_BAR_LEN / m_nMaxValue);
	UVImage* pBar = g_UVMgr.GetUVImage(_UV_mission_bar);

	g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
		_MISSION_GAUGE_BAR_X, _MISSION_GAUGE_BAR_Y,
		nValue, pBar->v2 - pBar->v1,
		0.f, D3DXCOLOR(1.f, 1.f, 1.f, 1),
		0.f, pBar->nTexId,
		pBar->u1, pBar->v1, pBar->u2, pBar->v2);

	if(m_szStr[0] != '/0')
	{
		g_pFont->DrawUIText(0, 0, -1, -1, "@(reset)@(scale=0.5,0.5)");
		g_pFont->DrawUIText(_MISSION_GAUGE_TEXT_X, _MISSION_GAUGE_TEXT_Y,
			-1, -1, m_szStr);
	}
	
	m_nValue = -1;
	m_nMaxValue = -1;
	memset(m_szStr, 0, sizeof(char)*STR_SPEECH_MAX);
}

void CFcSpecialMissionGauge::SetValue(int nValue, int nMaxValue, char* pCaption)
{	
	m_nValue = nValue;
	m_nMaxValue = nMaxValue;

	if(pCaption != NULL){
		strcpy(m_szStr, pCaption);
	}else{
		memset(m_szStr, 0, sizeof(char)*STR_SPEECH_MAX);
	}
}