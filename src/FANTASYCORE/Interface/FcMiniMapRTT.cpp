#include "stdafx.h"
#include "BsKernel.h"
#include "FcMiniMapRTT.h"
#include "BsMaterial.h"

#include "FcHeroObject.h"
#include "FcTroopObject.h"
#include "FcWorld.h"
#include "FcGlobal.h"
#include "FcCameraObject.h"
#include "FcInterfaceManager.h"

#include "BsUiUVManager.h"
#include ".\\data\\Menu\\BsUiUVID.h"

#define _FC_MINIMAP_POS_X			924
#define _FC_MINIMAP_POS_Y			62
#define _FC_MINIMAP_SIZE			256		//80000이하
#define _FC_MINIMAP_SIZE_L			512		//80000이상
#define _FC_MINIMAP_SCALE_XY		300

#define _FC_MAP_MAX_SIZE			(1600 * 100)
#define _FC_MAP_TEX_SIZE			512				//256				//1600m : view = 512 : UVmapsize

#define _MAX_POINT_FRAME	2
#define _AREA_ALPHA_FRAME	8

#define	_PLAYER_SIGHT_RADIAN_MAX		(240 * 100)
#define _PLAYER_SIGHT_RADIAN_MIN		(10 * 100)
#define _FC_MINIMAP_SIGHT_UV			128

#define _DRAW_CIRCLE_SIGN_TICK			30
#define _DRAW_CIRCLE_SIGN_FULL_TICK		300

CFcUiMiniMap::CFcUiMiniMap()
{
	m_nMapTexID = -1;
	m_hSourceRTTHandle = NULL;
	m_nSourceTextureId = -1;
	m_hAlphaRTTHandle = NULL;
	m_nAlphaTextureId = -1;

	LoadMiniMap();
	LoadViewPort();
	LoadOffset();
	LoadRtt();
}

void CFcUiMiniMap::LoadViewPort()
{
	int nRttSize = _FC_MINIMAP_SIZE;
	if(m_MiniMapSize.cx > _FC_MINIMAP_SIZE){
		nRttSize = _FC_MAP_TEX_SIZE;
	}

	m_pViewport = new D3DVIEWPORT9();
	m_pViewport->X = 0;
	m_pViewport->Y = 0;
	m_pViewport->Width = nRttSize;
	m_pViewport->Height = nRttSize;
	m_pViewport->MinZ = 1.f;
	m_pViewport->MaxZ = 0.f;
}

void CFcUiMiniMap::LoadOffset()
{
	CFcWorld::GetInstance().GetMapSize(m_fMapWidth, m_fMapHeight);
	m_fMapRate = (m_fMapWidth >= m_fMapHeight) ? m_fMapWidth : m_fMapHeight;
	
	if(m_MiniMapSize.cx > _FC_MINIMAP_SIZE)
	{	
		float fMiniMapWidth = m_fMapWidth * _FC_MAP_TEX_SIZE / _FC_MAP_MAX_SIZE;
		float fMiniMapHeight = m_fMapHeight * _FC_MAP_TEX_SIZE / _FC_MAP_MAX_SIZE;
		m_fMiniMapRate = (fMiniMapWidth >= fMiniMapHeight) ? fMiniMapWidth : fMiniMapHeight;

		m_fMiniMapRateX = fMiniMapWidth;
		m_fMiniMapRateY = fMiniMapHeight;

		m_fMiniMapAddPosX = (_FC_MAP_TEX_SIZE - fMiniMapWidth) / 2.f;
		m_fMiniMapAddPosY = (_FC_MAP_TEX_SIZE - fMiniMapHeight) / 2.f;

		m_fMapSightRadian = _PLAYER_SIGHT_RADIAN_MAX * m_fMiniMapRate / m_fMapRate;
	}
	else
	{
		m_fMiniMapRate = _FC_MINIMAP_SIZE;
		float fWidthRate(1.f), fHeightRate(1.f);
		if(m_fMapWidth > m_fMapHeight){
			fHeightRate = m_fMapHeight / m_fMapWidth;
		}
		else if(m_fMapWidth < m_fMapHeight){
			fWidthRate = m_fMapWidth / m_fMapHeight;
		}

		m_fMiniMapRateX = 0;
		m_fMiniMapRateY = 0;

		m_fMiniMapAddPosX = _FC_MINIMAP_SIZE * (1.f - fWidthRate) / 2.f;
		m_fMiniMapAddPosY = _FC_MINIMAP_SIZE * (1.f - fHeightRate) / 2.f;

		m_fMapSightRadian = _PLAYER_SIGHT_RADIAN_MAX * m_fMiniMapRate / m_fMapRate;
	}
}

void CFcUiMiniMap::LoadMiniMap()
{
	char szFileName[MAX_PATH];
	memset(szFileName, 0, MAX_PATH);
	GetFileName(szFileName, g_FcWorld.GetMapFileName());	
	strcat(szFileName,"_mini.dds");

	g_BsKernel.chdir("map");
	m_nMapTexID = g_BsKernel.LoadTexture(szFileName);
	g_BsKernel.chdir("..");

	if(m_nMapTexID == -1) //미니맵이 존재하지 않으면
	{
		g_BsKernel.chdir("Interface");
		m_nMapTexID = g_BsKernel.LoadTexture("TestMinimap.dds");
		BsAssert(m_nMapTexID != -1 && "미니맵이 존재하지 않습니다.");
		g_BsKernel.chdir("..");
	}

	m_MiniMapSize =  g_BsKernel.GetTextureSize(m_nMapTexID);
	BsAssert(m_MiniMapSize.cx == m_MiniMapSize.cy);
}

void CFcUiMiniMap::LoadRtt()
{
	CBsOffScreenManager* pBsOffScr = g_BsKernel.GetRTTManager();
	if(pBsOffScr == NULL)
	{
		BsAssert(pBsOffScr != NULL);
		return;
	}

	int nRttSize = _FC_MINIMAP_SIZE;
	if(m_MiniMapSize.cx > _FC_MINIMAP_SIZE){
		nRttSize = _FC_MAP_TEX_SIZE;
	}

	float fWidth = m_pViewport->Width/(float)_SCREEN_WIDTH;
	float fHeight = m_pViewport->Height/(float)_SCREEN_WIDTH;

	CBsUiAlphaMapTexture* pAlphaRTTexture = new CBsUiAlphaMapTexture(this);
	m_hAlphaRTTHandle = g_BsKernel.GetRTTManager()->Create((CBsGenerateTexture*)pAlphaRTTexture,
		nRttSize, nRttSize,
		0.f, 0.f, fWidth, fHeight, true);
	m_nAlphaTextureId = g_BsKernel.GetRTTManager()->GetRTTextureID(m_hAlphaRTTHandle);

	CBsUiSourceMapTexture* pSourceRTTexture = new CBsUiSourceMapTexture(this);
	m_hSourceRTTHandle = g_BsKernel.GetRTTManager()->Create((CBsGenerateTexture*)pSourceRTTexture,
		nRttSize, nRttSize,
		0.f, 0.f, fWidth, fHeight, true);
	m_nSourceTextureId = g_BsKernel.GetRTTManager()->GetRTTextureID(m_hSourceRTTHandle);

}

CFcUiMiniMap::~CFcUiMiniMap()
{
	if(m_hAlphaRTTHandle != NULL)
	{
		g_BsKernel.GetRTTManager()->Release(m_hAlphaRTTHandle);
		m_hAlphaRTTHandle = NULL;
		m_nAlphaTextureId = -1;
	}

	if(m_hSourceRTTHandle != NULL)
	{	
		g_BsKernel.GetRTTManager()->Release(m_hSourceRTTHandle);
		m_hSourceRTTHandle = NULL;
		m_nSourceTextureId = -1;
	}	
}

void CFcUiMiniMap::ReleaseData()
{
	SAFE_RELEASE_TEXTURE(m_nMapTexID);

	SAFE_DELETE(m_pViewport);
}

void CFcUiMiniMap::Update()
{
	if(m_hSourceRTTHandle != NULL){
		g_BsKernel.GetRTTManager()->GetRTTextrue(m_hSourceRTTHandle)->SetEnable(m_bShow);
	}

	if(m_hAlphaRTTHandle != NULL){
		g_BsKernel.GetRTTManager()->GetRTTextrue(m_hAlphaRTTHandle)->SetEnable(m_bShow);
	}

	if(m_bShow == false){
		return;
	}
	
	int nU1(0), nV1(0), nU2(0), nV2(0);
	GetMinimapUVPos(nU1, nV1, nU2, nV2);
		
	if(m_nAlphaTextureId != -1)
	{
		g_BsKernel.DrawUIBox_s(_Ui_Mode_ALPHA,
			_FC_MINIMAP_POS_X, _FC_MINIMAP_POS_Y, _FC_MINIMAP_SCALE_XY, _FC_MINIMAP_SCALE_XY,
			0.f, D3DXCOLOR(1.f,1.f,1.f,1.f),
			0.f, m_nMapTexID,
			nU1, nV1, nU2, nV2,
			m_nAlphaTextureId,
			true);
	}

	if(m_nSourceTextureId != -1)
	{
		g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
			_FC_MINIMAP_POS_X, _FC_MINIMAP_POS_Y, _FC_MINIMAP_SCALE_XY, _FC_MINIMAP_SCALE_XY,
			0.f, D3DXCOLOR(1.f,1.f,1.f,1.f),
			0.f, m_nSourceTextureId,
			nU1, nV1, nU2, nV2,
			-1, true);
	}
}


void CFcUiMiniMap::GetMinimapUVPos(int &nU1, int &nV1, int &nU2, int &nV2)
{
	int nHalf = _FC_MINIMAP_SIZE / 2;
	int nTexHalfSize = (_FC_MAP_TEX_SIZE * (int)m_fMapRate / _FC_MAP_MAX_SIZE) /2;

	if(m_MiniMapSize.cx == _FC_MINIMAP_SIZE)
	{
		nU1 = 0; nV1 = 0;
		nU2 = _FC_MINIMAP_SIZE; nV2 = _FC_MINIMAP_SIZE;
	}
	//화면에 보여지는 size _FC_MINIMAP_SIZE(256)보다 작거나 같으면 1:1로 찍는다.
	else if(nHalf >= nTexHalfSize)
	{
		nU1 = _FC_MINIMAP_SIZE - nHalf; nV1 = _FC_MINIMAP_SIZE - nHalf;
		nU2 = _FC_MINIMAP_SIZE + nHalf; nV2 = _FC_MINIMAP_SIZE + nHalf;
	}
	else
	{
		HeroObjHandle hHero = g_FcWorld.GetHeroHandle();
		D3DXVECTOR3 posView  = hHero->GetPos();
		D3DXVECTOR2 pos = SetViewToMap(posView.x, posView.z);
		int nX = (int)pos.x;
		int nY = (int)pos.y;

		int nGap = _FC_MINIMAP_SIZE - nTexHalfSize;
		if(nX - nHalf < nGap){
			nX = nGap + nHalf;
		}
		else if(nX + nHalf >= _FC_MAP_TEX_SIZE - nGap){
			nX = (_FC_MAP_TEX_SIZE - nGap) - nHalf;
		}
		
		if(nY - nHalf < nGap){
			nY = nGap + nHalf;
		}
		else if(nY + nHalf >= _FC_MAP_TEX_SIZE - nGap){
			nY = (_FC_MAP_TEX_SIZE - nGap) - nHalf;
		}

		nU1 = nX - nHalf;	nV1 = nY - nHalf;
		nU2 = nX + nHalf;	nV2 = nY + nHalf;
	}
}

void CFcUiMiniMap::GetFileName(char *szBuf,char *szFileName)
{
	int nBufCount = 0;
	bool bStart = false;
	int nIndex = 0;
	for(int i = strlen(szFileName);i > 0;i--)
	{
		if(szFileName[i] == '\\'){
			nIndex = i+1;
			while(szFileName[nIndex] != '.')
			{				
				szBuf[nBufCount] = szFileName[nIndex];
				nIndex++;
				nBufCount++;				
			}
			break;
		}
	}
}


D3DXVECTOR2 CFcUiMiniMap::SetViewToMap(float fViewX, float fViewY)
{
	float fX(0.f), fY(0.f);
	if(m_MiniMapSize.cx > _FC_MINIMAP_SIZE)
	{
		fX = (fViewX/m_fMapWidth) * m_fMiniMapRateX;
		fY = (1.f - (fViewY/m_fMapHeight)) * m_fMiniMapRateY;
	}
	else
	{
		fX = (fViewX/m_fMapRate) * m_fMiniMapRate;
		fY = (1.f - (fViewY/m_fMapRate)) * m_fMiniMapRate;
	}

	D3DXVECTOR2 pos = D3DXVECTOR2(m_fMiniMapAddPosX + fX, m_fMiniMapAddPosY + fY);
	return pos;
}

void CFcUiMiniMap::PointOn(bool bShow, const char *szName, CFcTroopObject *pTroop, DWORD nTick)
{
	if(m_hSourceRTTHandle == NULL){
		return;
	}

	((CBsUiSourceMapTexture*)m_hSourceRTTHandle)->PointOn(bShow, szName, pTroop, nTick);
}

void CFcUiMiniMap::AreaOn(bool bShow, const char *szName, float fStartX, float fStartY, float fEndX, float fEndY, DWORD nTick)
{
	if(m_hSourceRTTHandle == NULL){
		return;
	}

	((CBsUiSourceMapTexture*)m_hSourceRTTHandle)->AreaOn(bShow, szName, fStartX, fStartY, fEndX, fEndY, nTick);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CBsUiSourceMapTexture::CBsUiSourceMapTexture(CFcUiMiniMap* pMiniMap)
: CBsGenerateTexture()
{
	m_pMiniMap = pMiniMap;
	
	g_BsKernel.chdir("interface");
	m_nSightTexID = g_BsKernel.LoadTexture(_FC_MINIMAP_SIGHT);
	g_BsKernel.chdir("..");

	BsAssert(m_nSightTexID != -1);
}

void CBsUiSourceMapTexture::Release()
{
	SAFE_RELEASE_TEXTURE(m_nSightTexID);

	for(unsigned int i = 0;i < m_PointInfoList.size();i++) {
		delete m_PointInfoList[i];
	}
	m_PointInfoList.clear();
}

void CBsUiSourceMapTexture::Create(int nTextureWidth, int nTextureHeight, float fStartX, float fStartY, float fWidth, float fHeight, bool bAlpha)
{	
	CBsGenerateTexture::Create(nTextureWidth, nTextureHeight, fStartX, fStartY, fWidth, fHeight, bAlpha);
}

void CBsUiSourceMapTexture::InitRenderRTT()
{
	g_BsKernel.SetUIException(_Ui_Exception_MINIMAP_SOURCE);
/*
	DrawCamDir();
	DrawArea();
	DrawUnderAttack();
	DrawArrowSet();
	DrawTroopList();
	DrawPlayer();
	DrawPoint();
*/
		
	DrawCamDir();
	DrawArea();
	DrawUnderAttack();
	DrawTroopList();
	DrawPoint();
	DrawArrowSet();
	DrawPlayer();	

	g_BsKernel.SetUIException(_Ui_Exception_NONE);
}

void CBsUiSourceMapTexture::Render(C3DDevice *pDevice)
{
	if(IsEnable() == false){
		return;
	}

	g_BsKernel.LockActiveCamera(GetCameraIndex());

	/////////////////////////////////////////////////////////////
	// Camera Lock하는 곳으로 옮겨야합니다. by jeremy
	pDevice->SetViewport(g_BsKernel.GetActiveCamera()->GetCameraViewport());
	/////////////////////////////////////////////////////////////

	pDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
	pDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE);
	pDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_MAX);


	D3DVIEWPORT9 savedviewport;
	pDevice->GetViewport(&savedviewport);
	pDevice->BeginScene();

	DWORD pm1,pm2,pm3,pm4,pm5,pm6;
	pDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &pm1);
	pDevice->GetRenderState(D3DRS_ALPHATESTENABLE,  &pm2);
	pDevice->GetRenderState(D3DRS_SRCBLEND,         &pm3);
	pDevice->GetRenderState(D3DRS_DESTBLEND,        &pm4);
	pDevice->GetRenderState(D3DRS_FILLMODE,         &pm5);
	pDevice->GetRenderState(D3DRS_ZENABLE,			&pm6);

	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE,  FALSE);
	pDevice->SetRenderState(D3DRS_FILLMODE,         D3DFILL_SOLID);
	pDevice->SetRenderState(D3DRS_ZENABLE,			D3DZB_FALSE);

	//--------------------------------------------------------------
	pDevice->SetViewport(m_pMiniMap->GetViewport());
	pDevice->ClearBuffer(D3DXCOLOR(0,0,0,0));

	//--------------------------------------------------------------
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_SRCBLEND,         D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND,        D3DBLEND_INVSRCALPHA);

	g_BsKernel.RenderExceptionUI(_Ui_Exception_MINIMAP_SOURCE);

	//--------------------------------------------------------------
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, pm1);
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE,  pm2);
	pDevice->SetRenderState(D3DRS_SRCBLEND,         pm3);
	pDevice->SetRenderState(D3DRS_DESTBLEND,        pm4);
	pDevice->SetRenderState(D3DRS_FILLMODE,         pm5);
	pDevice->SetRenderState(D3DRS_ZENABLE,			pm6);

	pDevice->EndScene();
	pDevice->SetViewport(&savedviewport);

	pDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
	pDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ZERO);
	pDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);

	g_BsKernel.UnlockActiveCamera();
}

void CBsUiSourceMapTexture::DrawUnderAttack()
{
	//초기화
	for(int j=0; j<50; j++){
		m_underAttackSign[j].bCheck = false;
	}

	HeroObjHandle Handle = g_FcWorld.GetHeroHandle();
	BsAssert(Handle);
	D3DXVECTOR2 HeroPosVec = D3DXVECTOR2(Handle->GetPos().x, Handle->GetPos().z);

	int nTroopCount = g_FcWorld.GetTroopCount();

	//check underattack sign
	for(int i = 0; i < nTroopCount; i++)
	{
		TroopObjHandle TroopHandle = g_FcWorld.GetTroopObject(i);
		BsAssert(TroopHandle->GetType() != TROOPTYPE_PLAYER_2);

		if(TroopHandle->IsVisibleInMinimap() == false){
			continue;
		}

		if(TroopHandle->IsEliminated() == true){
			continue;
		}

		if(TroopHandle->IsEnable() == false){
			continue;
		}
		
		if( TroopHandle->GetType() == TROOPTYPE_PLAYER_1){
			continue;
		}

		if(TroopHandle->GetTeam() != 0){
			continue;
		}

		if(TroopHandle->IsAttacked() == false){
			continue;
		}

		D3DXVECTOR2 TroopPosVec = D3DXVECTOR2(TroopHandle->GetPos().x, TroopHandle->GetPos().z);
		if(g_FCGameData.bShowMinimapAllObj == false)
		{	
			D3DXVECTOR2 Out;
			D3DXVec2Subtract(&Out, &HeroPosVec, &TroopPosVec);

			float fLen = D3DXVec2Length(&Out);
			float fHalfRadian = _PLAYER_SIGHT_RADIAN_MAX / 2.f;
			if(fLen >= fHalfRadian){
				continue;
			}

			fHalfRadian = _PLAYER_SIGHT_RADIAN_MIN;
			if(fLen < fHalfRadian){
				continue;
			}
		}
		
		bool bSameArea = false;
		for(int j=0; j<50; j++)
		{
			if(m_underAttackSign[j].nCallTick == -1){
				continue;
			}
			
			D3DXVECTOR2 Out;
			D3DXVec2Subtract(&Out, &TroopPosVec, &(m_underAttackSign[j].pos));

			float fLen = D3DXVec2Length(&Out);
			float fHalfRadian = _PLAYER_SIGHT_RADIAN_MIN;
			if(fLen < fHalfRadian)
			{
				m_underAttackSign[j].bCheck = true;
				bSameArea = true;
				break;
			}
		}

		if(bSameArea){
			continue;
		}
		
		int j = 0;
		for(j = 0; j<50; j++)
		{
			if(m_underAttackSign[j].nCallTick == -1){
				break;
			}
		}

		m_underAttackSign[j].bCheck = true;
		m_underAttackSign[j].nCallTick = 0;
		m_underAttackSign[j].pos = TroopPosVec;
	}

	//draw underattack sign
	for(int i=0; i<50; i++)
	{
		if(m_underAttackSign[i].nCallTick == -1){
			continue;
		}

		if(m_underAttackSign[i].bCheck == false)
		{
			m_underAttackSign[i].init();
			continue;
		}

		D3DXVECTOR2 Out;
		D3DXVec2Subtract(&Out, &HeroPosVec, &(m_underAttackSign[i].pos));

		float fLen = D3DXVec2Length(&Out);
		float fHalfRadian = _PLAYER_SIGHT_RADIAN_MIN;
		if(fLen < fHalfRadian){
			continue;
		}

		if(!IsPause()){
			m_underAttackSign[i].nCallTick++;
		}

		DrawSignUnderAttack(&m_underAttackSign[i]);
	}
}

void CBsUiSourceMapTexture::DrawSignUnderAttack(_UNDER_ATTACT_SIGN* info)
{	
	if(info->nCallTick <= _DRAW_CIRCLE_SIGN_TICK)
	{
		D3DXVECTOR2 pos = m_pMiniMap->SetViewToMap(info->pos.x, info->pos.y);
		DrawCircleSign(pos, info->nCallTick);
	}

	int nAddValue = (info->nCallTick % 40  > 20) ? 0 : 1;
	int nUVID = _UV_MiniMap_Battle_0 + nAddValue;
	UVImage* pImg = g_UVMgr.GetUVImage(nUVID);
	if( pImg == NULL){
		return;
	}

	D3DXVECTOR2 pos = m_pMiniMap->SetViewToMap(info->pos.x, info->pos.y);
	int sizeX = pImg->u2 - pImg->u1;
	int sizeY = pImg->v2 - pImg->v1;

	float fsX = pos.x - sizeX/2;
	float fsY = pos.y - sizeY/2;
	float feX = pos.x + sizeX/2;
	float feY = pos.y + sizeY/2;
		
	g_BsKernel.DrawUIBox(_Ui_Mode_Exception,
		(int)fsX , (int)fsY, (int)feX, (int)feY,
		0.f, D3DXCOLOR(1.f,1.f,1.f, 1.f),
		0.f, pImg->nTexId,
		pImg->u1, pImg->v1, pImg->u2, pImg->v2);
}


void CBsUiSourceMapTexture::DrawSignUnderAttackCircle(_UNDER_ATTACT_SIGN* info)
{
	UVImage* pImg = g_UVMgr.GetUVImage(_UV_MiniMap_Battle_circle);
	if( pImg == NULL){
		return;
	}

	D3DXVECTOR2 pos = m_pMiniMap->SetViewToMap(info->pos.x, info->pos.y);
	int sizeX = pImg->u2 - pImg->u1;
	int sizeY = pImg->v2 - pImg->v1;
	
	if ((info->nCallTick % 300)< 30)
	{
		int	nTick = info->nCallTick % 100;

		float fScale = 1.0f + (float)(30 - nTick) * 0.5f;
		float fsX = pos.x - (sizeX/2 * fScale);
		float fsY = pos.y - (sizeY/2 * fScale);
		float feX = pos.x + (sizeX/2 * fScale);
		float feY = pos.y + (sizeY/2 * fScale);

		float fAlpha = nTick / 30.f; //(nTick * 255) / 30;

		g_BsKernel.DrawUIBox(_Ui_Mode_Exception,
			(int)fsX, (int)fsY, (int)feX, (int)feY,
			0.f, D3DXCOLOR(1.f,1.f,1.f,fAlpha),
			0.f, pImg->nTexId,
			pImg->u1, pImg->v1, pImg->u2, pImg->v2);
	}

	float fScale = 0.5f + (float)( 30 - info->nCallTick % 31 ) / 60.0f;
	float fsX = pos.x - (sizeX/2 * fScale);
	float fsY = pos.y - (sizeY/2 * fScale);
	float feX = pos.x + (sizeX/2 * fScale);
	float feY = pos.y + (sizeY/2 * fScale);

	float fAlpha = 0.f;
	int nTickMod = info->nCallTick % 31;
	if (nTickMod < 5){
		fAlpha = nTickMod * 0.2f;
	}
	else if (nTickMod < 25){
		fAlpha = 1.f;
	}
	else{
		fAlpha = (30 - nTickMod) * 0.2f;
	}

	g_BsKernel.DrawUIBox(_Ui_Mode_Exception,
		(int)fsX, (int)fsY, (int)feX, (int)feY,
		0.f, D3DXCOLOR(1.f,1.f,1.f,fAlpha),
		0.f, pImg->nTexId,
		pImg->u1, pImg->v1, pImg->u2, pImg->v2);
}

void CBsUiSourceMapTexture::DrawCircleSign(D3DXVECTOR2 pos, int nTick)
{	
	if (nTick % _DRAW_CIRCLE_SIGN_FULL_TICK >= _DRAW_CIRCLE_SIGN_TICK){
		return;
	}
	
	UVImage* pImg = g_UVMgr.GetUVImage(_UV_MiniMap_Battle_circle);
	if( pImg == NULL){
		return;
	}
	
	int sizeX = pImg->u2 - pImg->u1;
	int sizeY = pImg->v2 - pImg->v1;

	int	nTick1 = nTick % 100;

	float fScale = 1.0f + (float)(30 - nTick1) * 0.5f;
	float fsX = pos.x - (sizeX/2 * fScale);
	float fsY = pos.y - (sizeY/2 * fScale);
	float feX = pos.x + (sizeX/2 * fScale);
	float feY = pos.y + (sizeY/2 * fScale);

	float fAlpha = nTick1 / 30.f;

	g_BsKernel.DrawUIBox(_Ui_Mode_Exception,
		(int)fsX, (int)fsY, (int)feX, (int)feY,
		0.f, D3DXCOLOR(1.f,1.f,1.f,fAlpha),
		0.f, pImg->nTexId,
		pImg->u1, pImg->v1, pImg->u2, pImg->v2);
}


void CBsUiSourceMapTexture::DrawTroopList()
{
	HeroObjHandle Handle = g_FcWorld.GetHeroHandle();
	BsAssert(Handle);

	D3DXVECTOR3 HeroPosVec = Handle->GetPos();

	int nTroopCount = g_FcWorld.GetTroopCount();
	int nTeam = g_FcWorld.GetHeroHandle()->GetForce();

	//아군
	int nFriendlyCount = 0;
	int *pFriendlyTroopList = new int[nTroopCount];
	memset(pFriendlyTroopList, -1, sizeof(int)*nTroopCount);

	int nFriendlyFlyCount = 0;
	int *pFriendlyFlyTroopList = new int[nTroopCount];
	memset(pFriendlyFlyTroopList, -1, sizeof(int)*nTroopCount);

	//적군
	int nEnemyCount = 0;
	int *pEnemyTroopList = new int[nTroopCount];
	memset(pEnemyTroopList, -1, sizeof(int)*nTroopCount);

	int nEnemyFlyCount = 0;
	int *pEnemyFlyTroopList = new int[nTroopCount];
	memset(pEnemyFlyTroopList, -1, sizeof(int)*nTroopCount);
	
	//get Enemy Troop, our troop, friendly troop
	for(int i = 0; i < nTroopCount; i++)
	{
		TroopObjHandle TroopHandle = g_FcWorld.GetTroopObject(i);

		if(TroopHandle->IsVisibleInMinimap() == false){
			continue;
		}

		if(TroopHandle->IsEliminated() == true){
			continue;
		}

		if(TroopHandle->IsEnable() == false){
			continue;
		}

		if(TroopHandle->GetHP() <= 0.f){
			continue;
		}
		
		if( TroopHandle->GetType() == TROOPTYPE_PLAYER_1){
			continue;
		}

		if( TroopHandle->GetType() == TROOPTYPE_WALL_DUMMY ){
			continue;
		}

		BsAssert(TroopHandle->GetType() != TROOPTYPE_PLAYER_2);
		
		if(g_FCGameData.bShowMinimapAllObj == false)
		{
			D3DXVECTOR3 TroopPosVec = TroopHandle->GetPos();
			D3DXVECTOR3 Out;
			D3DXVec3Subtract(&Out, & HeroPosVec, &TroopPosVec);
			float fLen = D3DXVec3Length(&Out);
			float fHalfRadian = _PLAYER_SIGHT_RADIAN_MAX / 2.f;
			if(fLen >= fHalfRadian){
				continue;
			}
		}

		//get out troop, friendly troop, enemy
		if(TroopHandle->GetTeam() != 0)
		{
			if(TroopHandle->GetType() == TROOPTYPE_FLYING)
			{
				BsAssert(nEnemyFlyCount < nTroopCount);
				pEnemyFlyTroopList[nEnemyFlyCount] = i;
				nEnemyFlyCount++;
			}
			else
			{
				BsAssert(nEnemyCount < nTroopCount);
				pEnemyTroopList[nEnemyCount] = i;
				nEnemyCount++;
			}
		}
		else
		{
			if(TroopHandle->GetType() == TROOPTYPE_FLYING)
			{
				BsAssert(nFriendlyFlyCount < nTroopCount);
				pFriendlyFlyTroopList[nFriendlyFlyCount] = i;
				nFriendlyFlyCount++;
			}
			else
			{
				BsAssert(nFriendlyCount < nTroopCount);
				pFriendlyTroopList[nFriendlyCount] = i;
				nFriendlyCount++;
			}
		}
	}
	
	//----------------------------------------------------------------------
	if(GetProcessTick() % 40 > 20)
	{
		DrawTroop(pFriendlyTroopList, nFriendlyCount, false, false);
		DrawTroop(pEnemyTroopList, nEnemyCount, true, false);

		DrawTroop(pFriendlyFlyTroopList, nFriendlyFlyCount, false, false);
		DrawTroop(pEnemyFlyTroopList, nEnemyFlyCount, true, false);
	}
	else
	{
		DrawTroop(pEnemyTroopList, nEnemyCount, true, false);
		DrawTroop(pFriendlyTroopList, nFriendlyCount, false, false);
		
		DrawTroop(pEnemyFlyTroopList, nEnemyFlyCount, true, false);
		DrawTroop(pFriendlyFlyTroopList, nFriendlyFlyCount, false, false);
	}
	//----------------------------------------------------------------------
	
	delete[] pFriendlyTroopList;
	delete[] pFriendlyFlyTroopList;	
	delete[] pEnemyTroopList;
	delete[] pEnemyFlyTroopList;
}

void CBsUiSourceMapTexture::DrawTroop(int* pTroopList, int nTroopCount, bool bEnemy, bool bAir)
{	
	for(int i=0; i<nTroopCount; i++)
	{
		int nIndex = pTroopList[i];
		D3DXVECTOR3 TroopPosVec = g_FcWorld.GetTroopObject(nIndex)->GetPos();

		int nCount = g_FcWorld.GetTroopObject(nIndex)->GetUnitCount();

		int nAddUVID = 0;
		if(bEnemy){
			nAddUVID = 1;
		}

		int nUVID = _UV_MiniMap_Our_Force_l;
		if(bAir == false)
		{
			nUVID = _UV_MiniMap_Our_Force_l + nAddUVID;
			if(nCount < 10){
				nUVID = _UV_MiniMap_Our_Force_s + nAddUVID;
			}else if(nCount < 30){
				nUVID = _UV_MiniMap_Our_Force_m + nAddUVID;
			}
		}else{
			nUVID = _UV_MiniMap_our_dragon + nAddUVID;
		}

		DrawUnit(TroopPosVec.x, TroopPosVec.z, nUVID);
	}
}

void CBsUiSourceMapTexture::DrawPlayer()
{
	HeroObjHandle Handle = g_FcWorld.GetHeroHandle();
	BsAssert(Handle);

	D3DXVECTOR3 TroopPosVec = Handle->GetPos();
	DrawUnit(TroopPosVec.x, TroopPosVec.z, _UV_MiniMap_Player);
}

void CBsUiSourceMapTexture::DrawCamDir()
{
	D3DXVECTOR3 posView = g_FcWorld.GetHeroHandle()->GetPos();
	D3DXVECTOR2 pos = m_pMiniMap->SetViewToMap(posView.x, posView.z);

	float fMapSightRadian = m_pMiniMap->GetMapSightRadian();
	float fX = pos.x - fMapSightRadian/2.f;
	float fY = pos.y - fMapSightRadian/2.f;

	CameraObjHandle CamHandle = CFcBaseObject::GetCameraObjectHandle(0);
	D3DXVECTOR3 TempDir = CamHandle->GetCrossVector()->m_ZVector;
	D3DXVECTOR2 Dir = D3DXVECTOR2( TempDir.x, TempDir.z );
	float fAngle = acos( D3DXVec2Dot( &Dir, &D3DXVECTOR2( 0.f, 1.f ) ) );
	if( Dir.x < 0.f ) fAngle = -fAngle;

	g_BsKernel.DrawUIBox_s(_Ui_Mode_Exception,
		(int)fX, (int)fY, (int)fMapSightRadian, (int)fMapSightRadian,
		0.f, D3DXCOLOR(1.f,1.f,1.f,0.5f),
		fAngle, m_nSightTexID,
		0, 0, _FC_MINIMAP_SIGHT_UV, _FC_MINIMAP_SIGHT_UV);
}


void CBsUiSourceMapTexture::DrawArrowSet()
{
	HeroObjHandle Handle = g_FcWorld.GetHeroHandle();
	BsAssert(Handle);

	//player Arrow
	D3DXVECTOR3 TroopPosVec = Handle->GetPos();
	D3DXVECTOR2 Dir = Handle->GetTroop()->GetDir();

	float fAngle = acos( D3DXVec2Dot( &Dir, &D3DXVECTOR2( 0.f, 1.f ) ) );
	if( Dir.x < 0.f ) fAngle = -fAngle;

	DrawPlayerWave(TroopPosVec.x, TroopPosVec.z);
	DrawPlayerArrow(TroopPosVec.x, TroopPosVec.z, fAngle);
}

void CBsUiSourceMapTexture::DrawPlayerWave(float fViewX, float fViewY)
{
	int nTick = GetProcessTick() % 30;
	if(nTick / 5 >= 5){
		return;
	}

	UVImage* pImg = g_UVMgr.GetUVImage(_UV_MiniMap_player_circle_1 + nTick / 6);
	if( pImg == NULL){
		return;
	}

	D3DXVECTOR2 pos = m_pMiniMap->SetViewToMap(fViewX, fViewY);
	int sizeX = pImg->u2 - pImg->u1;
	int sizeY = pImg->v2 - pImg->v1;

	g_BsKernel.DrawUIBox(_Ui_Mode_Exception,
		(int)(pos.x - sizeX/2), (int)(pos.y - sizeY/2),
		(int)(pos.x + sizeX/2), (int)(pos.y + sizeY/2),
		0.f, D3DXCOLOR(1.f,1.f,1.f,1.f),
		0.f, pImg->nTexId,
		pImg->u1, pImg->v1, pImg->u2, pImg->v2);
}

void CBsUiSourceMapTexture::DrawPlayerArrow(float fViewX, float fViewY, float fAngle)
{
	UVImage* pImg = g_UVMgr.GetUVImage(_UV_MiniMap_player_direction);
	if( pImg == NULL){
		return;
	}

	D3DXVECTOR2 pos = m_pMiniMap->SetViewToMap(fViewX, fViewY);
	float fWidth = (pImg->u2 - pImg->u1)/2.f; 
	float fHeight = (float)(pImg->v2 - pImg->v1);

	D3DXVECTOR2 pos1 = D3DXVECTOR2(0.f, 1.f*fHeight);
	D3DXVECTOR2 pos2;
	pos2.x = cos(fAngle) * pos1.x + sin(fAngle) * pos1.y;
	pos2.y = sin(fAngle) * pos1.x - cos(fAngle) * pos1.y;
	D3DXVECTOR2 pos3 = pos + pos2;

	D3DXVECTOR2 vNorOut;
	D3DXVec2Normalize(&vNorOut, &pos2);

	D3DXVECTOR2 vInvOut = D3DXVECTOR2(vNorOut.y, -vNorOut.x);
	D3DXVECTOR2 vPos1 = pos3 + ( vInvOut * fWidth );
	D3DXVECTOR2 vPos2 = pos3 + ( vInvOut * -fWidth );
	D3DXVECTOR2 vPos3 = pos + ( vInvOut * fWidth );
	D3DXVECTOR2 vPos4 = pos + ( vInvOut * -fWidth );

	g_BsKernel.DrawUIBox_Pos4(_Ui_Mode_Exception,
		(int)vPos1.x, (int)vPos1.y,
		(int)vPos2.x, (int)vPos2.y,
		(int)vPos3.x, (int)vPos3.y,
		(int)vPos4.x, (int)vPos4.y,
		0.f,  D3DXCOLOR(1, 1, 1, 1),
		pImg->nTexId,
		pImg->u1, pImg->v1, pImg->u2, pImg->v2);
}

void CBsUiSourceMapTexture::DrawUnit(float fViewX, float fViewY, int nUVId)
{
	UVImage* pImg = g_UVMgr.GetUVImage(nUVId);
	if( pImg == NULL){
		return;
	}

	D3DXVECTOR2 pos = m_pMiniMap->SetViewToMap(fViewX, fViewY);
	int sizeX = pImg->u2 - pImg->u1;
	int sizeY = pImg->v2 - pImg->v1;

	g_BsKernel.DrawUIBox(_Ui_Mode_Exception,
		(int)(pos.x - sizeX/2), (int)(pos.y - sizeY/2),
		(int)(pos.x + sizeX/2), (int)(pos.y + sizeY/2),
		0.f, D3DXCOLOR(1.f,1.f,1.f,1.f),
		0.f, pImg->nTexId,
		pImg->u1, pImg->v1, pImg->u2, pImg->v2);
}


void CBsUiSourceMapTexture::DrawPoint()
{
	for(unsigned int i = 0;i < m_PointInfoList.size();i++)
	{
		if(!IsPause())
		{
			m_PointInfoList[i]->nTimeTick--;

			if(!(GetProcessTick() % 5))
			{
				m_PointInfoList[i]->nCount++;
				if(m_PointInfoList[i]->nCount >= _MAX_POINT_FRAME){
					m_PointInfoList[i]->nCount = 0;
				}
			}
		}
		
		BsAssert(m_PointInfoList[i]->pTroop);
		D3DXVECTOR3 TroopPos = m_PointInfoList[i]->pTroop->GetPos();		
		
		int nUVId = _UV_MiniMap_TroopID_O_1;				//out force
		if(m_PointInfoList[i]->pTroop->GetTeam() != 0){		//enemy
			nUVId = _UV_MiniMap_TroopID_E_1;
		}

		int nAniUVId = nUVId + m_PointInfoList[i]->nCount;
		DrawUnit(TroopPos.x, TroopPos.z, nAniUVId);

		if(m_PointInfoList[i]->nCount <= _DRAW_CIRCLE_SIGN_TICK)
		{
			D3DXVECTOR2 pos = m_pMiniMap->SetViewToMap(TroopPos.x, TroopPos.z);
			DrawCircleSign(pos, m_PointInfoList[i]->nCount);
		}

		if(!IsPause())
		{
			bool bRemove = false;
			PointInfo* pInfo = m_PointInfoList[i];
			if( pInfo->pTroop )
			{
				if( pInfo->pTroop->IsEnable() == false || pInfo->pTroop->IsEliminated() )
				{
					delete m_PointInfoList[i];
					m_PointInfoList.erase(m_PointInfoList.begin() + i);
					i--;
					bRemove = true;
				}
			}

			if( bRemove == false )
			{
				if(m_PointInfoList[i]->nTimeTick <= 0)
				{
					delete m_PointInfoList[i];
					m_PointInfoList.erase(m_PointInfoList.begin() + i);
					i--;
				}
			}
		}
	}
}

void CBsUiSourceMapTexture::PointOn(bool bShow, const char *szName, CFcTroopObject *pTroop, DWORD nTick)
{
	if(bShow)
	{
		BsAssert(pTroop);

		PointInfo *pData = new PointInfo;
		pData->nTimeTick = nTick;
		pData->pTroop = pTroop;
		strcpy( pData->szName, szName );
		m_PointInfoList.push_back(pData);
	}
	else
	{
		for(unsigned int i=0; i<m_PointInfoList.size(); i++)
		{
			PointInfo* pInfo = m_PointInfoList[i];
			if( _stricmp( pInfo->szName,szName) == 0)
			{
				delete m_PointInfoList[i];
				m_PointInfoList.erase(m_PointInfoList.begin() + i);
				break;
			}
		}
	}
}



void CBsUiSourceMapTexture::DrawArea()
{
	for(unsigned int i = 0;i < m_AreaInfoList.size();i++)
	{
		if(!IsPause())
		{
			m_AreaInfoList[i]->nTimeTick--;
			m_AreaInfoList[i]->nCount++;
		}
		
		int nUVID = _UV_MiniMap_AreaID_1;
		if(m_AreaInfoList[i]->nCount % 40 > 20 ){
			nUVID = _UV_MiniMap_AreaID_2;
		}
		
		UVImage* pImg = g_UVMgr.GetUVImage(nUVID);
		if( pImg == NULL){
			return;
		}

		D3DXVECTOR2 posStart = m_pMiniMap->SetViewToMap(m_AreaInfoList[i]->fsX, m_AreaInfoList[i]->fsY);
		D3DXVECTOR2 posEnd = m_pMiniMap->SetViewToMap(m_AreaInfoList[i]->feX, m_AreaInfoList[i]->feY);

		g_BsKernel.DrawUIBox(_Ui_Mode_Exception,
			(int)posStart.x, (int)posStart.y,
			(int)posEnd.x, (int)posEnd.y,
			0.f, D3DXCOLOR(1.f,1.f,1.f,0.5f),
			0.f, pImg->nTexId,
			pImg->u1, pImg->v1, pImg->u2, pImg->v2);
		

		if(!IsPause())
		{
			if(m_AreaInfoList[i]->nTimeTick <= 0)
			{
				delete m_AreaInfoList[i];
				m_AreaInfoList.erase(m_AreaInfoList.begin() + i);
				i--;
			}
		}
	}
}

/*
void CBsUiSourceMapTexture::DrawArea()
{
	for(unsigned int i = 0;i < m_AreaInfoList.size();i++)
	{
		if(!IsPause())
		{
			m_AreaInfoList[i]->nTimeTick--;
			m_AreaInfoList[i]->nCount++;
		}
		
		UVImage* pImg = g_UVMgr.GetUVImage(_UV_MiniMap_AreaID_1);
		if( pImg == NULL){
			return;
		}

		D3DXVECTOR2 posStart = m_pMiniMap->SetViewToMap(m_AreaInfoList[i]->fsX, m_AreaInfoList[i]->fsY);
		D3DXVECTOR2 posEnd = m_pMiniMap->SetViewToMap(m_AreaInfoList[i]->feX, m_AreaInfoList[i]->feY);
		
		int nTick = m_AreaInfoList[i]->nCount % 40;
		float fAlpha = 1.f;
		if(nTick < 10){
			fAlpha = nTick * 0.1f + 0.2f;
		}
		else if(nTick >= 30){
			fAlpha = (40 - nTick) * 0.1f + 0.2f;
		}

		if(fAlpha > 1.f){
			fAlpha = 1.f;
		}

		g_BsKernel.DrawUIBox(_Ui_Mode_Exception,
			(int)posStart.x, (int)posStart.y,
			(int)posEnd.x, (int)posEnd.y,
			0.f, D3DXCOLOR(1.f,1.f,1.f,fAlpha),
			0.f, pImg->nTexId,
			pImg->u1, pImg->v1, pImg->u2, pImg->v2);

		if(!IsPause())
		{
			if(m_AreaInfoList[i]->nTimeTick <= 0)
			{
				delete m_AreaInfoList[i];
				m_AreaInfoList.erase(m_AreaInfoList.begin() + i);
				i--;
			}
		}
	}
}
*/

void CBsUiSourceMapTexture::AreaOn(bool bShow, const char *szName, float fStartX, float fStartY, float fEndX, float fEndY, DWORD nTick)
{
	if(bShow)
	{
		AreaInfo *pData = new AreaInfo;
		pData->fsX = fStartX;	pData->fsY = fStartY;
		pData->feX = fEndX;		pData->feY = fEndY;
		pData->nTimeTick = nTick;
		strcpy(pData->szName, szName);
		m_AreaInfoList.push_back(pData);
	}
	else{
		for(unsigned int i = 0;i < m_AreaInfoList.size();i++){
			AreaInfo* pInfo = m_AreaInfoList[i];
			if( _stricmp(pInfo->szName, szName) == 0)
			{
				delete m_AreaInfoList[i];
				m_AreaInfoList.erase(m_AreaInfoList.begin() + i);
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CBsUiAlphaMapTexture::CBsUiAlphaMapTexture(CFcUiMiniMap* pMiniMap)
: CBsGenerateTexture()
{	
	m_pMiniMap = pMiniMap;

	g_BsKernel.chdir("interface");
	m_nSightTexID = g_BsKernel.LoadTexture(_FC_MINIMAP_SIGHT);
	g_BsKernel.chdir("..");

	BsAssert(m_nSightTexID != -1);
}

void CBsUiAlphaMapTexture::Release()
{
	SAFE_RELEASE_TEXTURE(m_nSightTexID);
}

void CBsUiAlphaMapTexture::Create(int nTextureWidth, int nTextureHeight, float fStartX, float fStartY, float fWidth, float fHeight, bool bAlpha)
{	
	CBsGenerateTexture::Create(nTextureWidth, nTextureHeight, fStartX, fStartY, fWidth, fHeight, bAlpha);
}

void CBsUiAlphaMapTexture::InitRenderRTT()
{
	g_BsKernel.SetUIException(_Ui_Exception_MINIMAP_ALPHA);

	D3DXVECTOR3 posView = g_FcWorld.GetHeroHandle()->GetPos();
	D3DXVECTOR2 pos = m_pMiniMap->SetViewToMap(posView.x, posView.z);

	float fMapSightRadian = m_pMiniMap->GetMapSightRadian();
	float fX = pos.x - fMapSightRadian/2.f;
	float fY = pos.y - fMapSightRadian/2.f;

	g_BsKernel.DrawUIBox_s(_Ui_Mode_Exception,
		(int)fX, (int)fY, (int)fMapSightRadian, (int)fMapSightRadian,
		0.f, D3DXCOLOR(1.f,1.f,1.f,1.f),
		0.f, m_nSightTexID,
		0, 0, _FC_MINIMAP_SIGHT_UV, _FC_MINIMAP_SIGHT_UV);

	g_BsKernel.SetUIException(_Ui_Exception_NONE);
}

void CBsUiAlphaMapTexture::Render(C3DDevice *pDevice)
{
	if(IsEnable() == false){
		return;
	}

	g_BsKernel.LockActiveCamera(GetCameraIndex());

	/////////////////////////////////////////////////////////////
	// Camera Lock하는 곳으로 옮겨야합니다. by jeremy
	pDevice->SetViewport(g_BsKernel.GetActiveCamera()->GetCameraViewport());
	/////////////////////////////////////////////////////////////

	pDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
	pDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE);
	pDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_MAX);


	D3DVIEWPORT9 savedviewport;
	pDevice->GetViewport(&savedviewport);
	pDevice->BeginScene();

	DWORD pm1,pm2,pm3,pm4,pm5,pm6;
	pDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &pm1);
	pDevice->GetRenderState(D3DRS_ALPHATESTENABLE,  &pm2);
	pDevice->GetRenderState(D3DRS_SRCBLEND,         &pm3);
	pDevice->GetRenderState(D3DRS_DESTBLEND,        &pm4);
	pDevice->GetRenderState(D3DRS_FILLMODE,         &pm5);
	pDevice->GetRenderState(D3DRS_ZENABLE,			&pm6);

	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE,  FALSE);
	pDevice->SetRenderState(D3DRS_FILLMODE,         D3DFILL_SOLID);
	pDevice->SetRenderState(D3DRS_ZENABLE,			D3DZB_FALSE);

	//--------------------------------------------------------------
	pDevice->SetViewport(m_pMiniMap->GetViewport());
	pDevice->ClearBuffer(D3DXCOLOR(0,0,0,0));

	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

	g_BsKernel.RenderExceptionUI(_Ui_Exception_MINIMAP_ALPHA);
	
	//--------------------------------------------------------------
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, pm1);
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE,  pm2);
	pDevice->SetRenderState(D3DRS_SRCBLEND,         pm3);
	pDevice->SetRenderState(D3DRS_DESTBLEND,        pm4);
	pDevice->SetRenderState(D3DRS_FILLMODE,         pm5);
	pDevice->SetRenderState(D3DRS_ZENABLE,			pm6);

	pDevice->EndScene();
	pDevice->SetViewport(&savedviewport);

	pDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
	pDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ZERO);
	pDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);

	g_BsKernel.UnlockActiveCamera();
}
