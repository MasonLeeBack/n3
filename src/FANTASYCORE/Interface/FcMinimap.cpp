#include "stdafx.h"
#include "BSKernel.h"
#include "FcWorld.h"
#include "FcGlobal.h"
#include "InputPad.h"
#include "FcHeroObject.h"
#include "FcTroopObject.h"
#include "DebugUtil.h"
#include "FcMinimap.h"
#include "FcInterfaceManager.h"
#include "FcWorld.h"
#include "BsSinTable.h"

#include "BsUiUVManager.h"
#include ".\\data\\Menu\\BsUiUVID.h"

#include "FcMiniMapRTT.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


#define _FC_MINIMAP_POS_X	924
#define _FC_MINIMAP_POS_Y	62

#define _FC_MINIMAP_SCALE_X	300
#define _FC_MINIMAP_SCALE_Y	300

#define _FC_MINIMAP_U		256
#define _FC_MINIMAP_V		256





//m_Map.SetPos(924, 62);
//m_Map.SetUV(0,0,256,256);
//m_Map.SetScale(FC_MINIMAP_SIZEX, FC_MINIMAP_SIZEY);


float CFcMinimap::m_fStartX;
float CFcMinimap::m_fStartY;
float CFcMinimap::m_fEndX;
float CFcMinimap::m_fEndY;
float CFcMinimap::m_fSaveStartX;
float CFcMinimap::m_fSaveStartY;
float CFcMinimap::m_fSaveEndX;
float CFcMinimap::m_fSaveEndY;
float CFcMinimap::m_fScaleX;
float CFcMinimap::m_fScaleY;


CFcMinimap::CFcMinimap()
{
	m_fStartX		= 0.f;
	m_fStartY		= 0.f;
	m_fEndX			= 1.f;
	m_fEndY			= 1.f;
	m_fSaveStartX	= 0.f;
	m_fSaveStartY	= 0.f;
	m_fSaveEndX		= 1.f;
	m_fSaveEndY		= 1.f;
	m_fScaleX		= 1.f;
	m_fScaleY		= 1.f;	
	m_bShowMinimap  = true;
	m_fMoveSpeed	= 0.01f;
	m_nCurAttackIconIndex = 0;
	Zoom(1.f);
	m_fZoom = 1.f;
	m_fNewZoom = 1.f;

	CFCMinimapObject *pMinimapObj = new CFCMinimapBackGround;
	pMinimapObj->Initialize(); //일단 스테이지 선택 없이
	m_ObjList.push_back(pMinimapObj);

	pMinimapObj = new CFCMinimapUnit;
	((CFCMinimapUnit *)pMinimapObj)->Initialize((CFCMinimapBackGround *)m_ObjList[0]);
	m_ObjList.push_back(pMinimapObj);

/*
	CBsUiAlphaMapTexture* pRTTexture = new CBsUiAlphaMapTexture();
	BsAssert(g_BsKernel.GetRTTManager() != NULL);
	m_hMapRTTHandle = g_BsKernel.GetRTTManager()->Create((CBsGenerateTexture*)pRTTexture, 512, 512);
	m_nMapTextureId = g_BsKernel.GetRTTManager()->GetRTTextureID(m_hMapRTTHandle);
	*/
}

CFcMinimap::~CFcMinimap()
{
	for(unsigned int i = 0;i < m_ObjList.size();i++)
	{
		delete m_ObjList[i];
	}
	m_ObjList.clear();
/*
	if(m_hMapRTTHandle != NULL)
	{
		BsAssert(g_BsKernel.GetRTTManager() != NULL);
		g_BsKernel.GetRTTManager()->Release(m_hMapRTTHandle);
		m_hMapRTTHandle = NULL;
		m_nMapTextureId = -1;
	}
	*/
}


void CFcMinimap::ZoomProcess()
{
	m_fZoom += (m_fNewZoom - m_fZoom)*0.05f;

	if(fabs(m_fZoom - m_fNewZoom) < 0.1f){
		m_fZoom = m_fNewZoom;
	}

	Zoom(m_fZoom);

}

void CFcMinimap::ShowToggle()
{
	if(m_bShowMinimap){
		m_bShowMinimap = false;
	}
	else{
		m_bShowMinimap = true;
	}
}


void CFcMinimap::Zoom(float fScale)
{
	float fZoomVal = (1.f / fScale);
	float sx = m_fStartX;
	float ex = m_fEndX;

	m_fStartX = sx + ((ex - sx) * 0.5f) - (fZoomVal * 0.5f);
	m_fEndX   = sx + ((ex - sx) * 0.5f) + (fZoomVal * 0.5f);

	if(m_fEndX > 1.f){
		m_fStartX -= m_fEndX - 1.f;
		m_fEndX = 1.f;
	}
	if(m_fStartX < 0.f){
		m_fEndX -= m_fStartX;
		m_fStartX = 0.f;
	}

	float sy = m_fStartY;
	float ey = m_fEndY;
	m_fStartY = sy + ((ey - sy) * 0.5f) - (fZoomVal * 0.5f);
	m_fEndY   = sy + ((ey - sy) * 0.5f) + (fZoomVal * 0.5f);

	if(m_fEndY > 1.f){
		m_fStartY -= m_fEndY - 1.f;
		m_fEndY = 1.f;
	}
	if(m_fStartY < 0.f){
		m_fEndY -= m_fStartY;
		m_fStartY = 0.f;
	}

	m_fScaleX = m_fEndX - m_fStartX;
	m_fScaleY = m_fEndY - m_fStartY;
}


void CFcMinimap::MoveX(float fX)
{
	m_fStartX += fX;
	m_fEndX += fX;

	if(m_fEndX > 1.f){
		m_fEndX = 1.f;
		m_fStartX = m_fEndX - m_fScaleX;
	}
	if(m_fStartX < 0.f){
		m_fStartX = 0.f;
		m_fEndX = m_fStartX + m_fScaleX;
	}
	m_fSaveStartX = m_fStartX;
	m_fSaveEndX = m_fEndX;
	
}

void CFcMinimap::MoveY(float fY)
{
	m_fStartY += fY;
	m_fEndY += fY;
	
	if(m_fEndY > 1.f){
		m_fEndY = 1.f;
		m_fStartY = m_fEndY - m_fScaleY;
	}
	if(m_fStartY < 0.f)	{
		m_fStartY = 0.f;
		m_fEndY = m_fStartY + m_fScaleY;
	}
	m_fSaveStartY = m_fStartY;
	m_fSaveEndY = m_fEndY;

}

void CFcMinimap::Update()
{
	if(m_bShow == false){
		return;
	}

	ZoomProcess();

	g_BsKernel.SetUIViewerIndex(_UI_FULL_VIEWER);

	for(unsigned int i = 0;i < (int)m_ObjList.size();i++)
	{
		m_ObjList[i]->Draw();
	}
}

void CFcMinimap::KeyInput(int nPlayerID, int KeyType,int PressTick)
{
	if(PressTick < 1)
	{
		return;
	}

	if(KeyType == PAD_INPUT_LEFT){
		MoveX(-0.01f);
	}
	else if(KeyType == PAD_INPUT_RIGHT)
	{
		MoveX(0.01f);
	}
	if(KeyType == PAD_INPUT_UP)
	{
		MoveY(-0.01f);
	}
	else if(KeyType == PAD_INPUT_DOWN)
	{
		MoveY(0.01f);		
	}
}


void CFcMinimap::PointOn(bool bShow,const char *szName,CFcTroopObject *pTroop,float fPosX,float fPosY,DWORD nTick)
{
	((CFCMinimapUnit *)m_ObjList[1])->PointOn( bShow , szName , pTroop ,fPosX, fPosY , nTick );
}



//-----------------------------------------------------------------------------------------
CFCMinimapBackGround::CFCMinimapBackGround()
{

}

void CFCMinimapBackGround::GetFileName(char *szBuf,char *szFileName)
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
	//int nPos = strlen(szFileName) - nIndex;
	//memcpy(szBuf,&szFileName[nPos+1],nPos);
	
}

void CFCMinimapBackGround::Initialize()
{
	char szFileName[MAX_PATH];
	memset(szFileName,0,MAX_PATH);
	GetFileName(szFileName,g_FcWorld.GetMapFileName());	
	strcat(szFileName,"_mini.dds");
	
	g_BsKernel.chdir("map");
	m_nMapTexId = g_BsKernel.LoadTexture(szFileName);
	g_BsKernel.chdir("..");

	if(m_nMapTexId == -1) //미니맵이 존재하지 않으면
	{
		g_BsKernel.chdir("Interface");
		m_nMapTexId = g_BsKernel.LoadTexture("TestMinimap.dds");
		BsAssert(m_nMapTexId != -1 && "미니맵이 존재하지 않습니다.");
		g_BsKernel.chdir("..");
	}
}

CFCMinimapBackGround::~CFCMinimapBackGround()
{
	SAFE_RELEASE_TEXTURE(m_nMapTexId);
}

void CFCMinimapBackGround::Draw()
{
	DrawMap();
}


void CFCMinimapBackGround::DrawMap()
{
	g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
		_FC_MINIMAP_POS_X, _FC_MINIMAP_POS_Y,
		_FC_MINIMAP_SCALE_X, _FC_MINIMAP_SCALE_Y,
		0.f, D3DXCOLOR(1.f,1.f,1.f,1.f),
		0.f, m_nMapTexId,
		0, 0, _FC_MINIMAP_U, _FC_MINIMAP_V);
}


//--------------------------------------------------------------------------------------------

CFCMinimapUnit::CFCMinimapUnit()
{
	;
}


CFCMinimapUnit::~CFCMinimapUnit()
{
    for(unsigned int i = 0;i < m_PointInfoList.size();i++) {
        delete m_PointInfoList[i];
    }
    m_PointInfoList.clear();
}

void CFCMinimapUnit::Initialize(CFCMinimapBackGround *pBG)
{
	m_BattleMarkIndex = 0;
	m_pBG = pBG;
}

void CFCMinimapUnit::Draw()
{
	DrawMark();

	RenderProcess();

	DrawTroop();
}


void CFCMinimapUnit::RenderProcess()
{
	if(IsPause() == true){
		return;
	}

	m_fZoomX = (1.f / CFcMinimap::m_fScaleX);
	m_fZoomY = (1.f / CFcMinimap::m_fScaleY);

	float fMapWidth,fMapHeight;
	CFcWorld::GetInstance().GetMapSize(fMapWidth,fMapHeight);

	float fWidthRate, fHeightRate;
	if(fMapWidth > fMapHeight){
		fWidthRate = 1.f;
		fHeightRate = fMapHeight / fMapWidth;
	}
	else if(fMapWidth < fMapHeight){
		fWidthRate = fMapWidth / fMapHeight;
		fHeightRate = 1.f;
	}
	else{
		fWidthRate = 1.f;
		fHeightRate = 1.f;
	}
	/*
	POSITION *prtPos = m_pBG->GetMapDrawInfo()->GetPos();
	POSITION *prtScale = m_pBG->GetMapDrawInfo()->GetScale();

	m_prtPosX = prtPos->x + (int)(prtScale->x * (1.f - fWidthRate) / 2.f); 
	m_prtPosY = prtPos->y + (int)(prtScale->y * (1.f - fHeightRate) / 2.f);
	m_prtScaleX = (int)(prtScale->x * fWidthRate);  
	m_prtScaleY = (int)(prtScale->y * fHeightRate);
	*/

	m_prtPosX = _FC_MINIMAP_POS_X + (int)(_FC_MINIMAP_SCALE_X * (1.f - fWidthRate) / 2.f); 
	m_prtPosY = _FC_MINIMAP_POS_Y + (int)(_FC_MINIMAP_SCALE_Y * (1.f - fHeightRate) / 2.f);
	m_prtScaleX = (int)(_FC_MINIMAP_SCALE_X * fWidthRate);  
	m_prtScaleY = (int)(_FC_MINIMAP_SCALE_Y * fHeightRate);
}


void CFCMinimapUnit::DrawTroop()
{
	int nTroopCount = g_FcWorld.GetTroopCount();

	int nPlayerIndex = -1;

	int nOurCount = 0;
	int *pOurTroopList = new int[nTroopCount];
	memset(pOurTroopList, -1, sizeof(int)*nTroopCount);

	int nFriendlyCount = 0;
	int *pFriendlyTroopList = new int[nTroopCount];
	memset(pFriendlyTroopList, -1, sizeof(int)*nTroopCount);

	int nEnemyCount = 0;
	int *pEnemyTroopList = new int[nTroopCount];
	memset(pEnemyTroopList, -1, sizeof(int)*nTroopCount);

	int nTeam = g_FcWorld.GetHeroHandle(0)->GetForce();

	//Enemy Troop & get player, our troop, friendly troop
	for(int i = 0; i < nTroopCount; i++)
	{
		if(g_FcWorld.GetTroopObject(i)->IsVisibleInMinimap() == false){
			continue;
		}

		if(g_FcWorld.GetTroopObject(i)->IsEliminated() == true){
			continue;
		}

		if(g_FcWorld.GetTroopObject(i)->IsEnable() == false){
			continue;
		}

		//get player, out troop, friendly troop, enemy
		int nType = g_FcWorld.GetTroopObject(i)->GetType();

		if( nType == TROOPTYPE_PLAYER_1 || nType == TROOPTYPE_PLAYER_2){
			nPlayerIndex = i;
			continue;
		}
		else if(g_FcWorld.GetTroopObject(i)->GetTeam() != 0)
		{
			BsAssert(nEnemyCount < nTroopCount);
			pEnemyTroopList[nEnemyCount] = i;
			nEnemyCount++;
		}
		else if(g_FcWorld.GetTroopObject(i)->GetForce() == nTeam)
		{
			BsAssert(nOurCount < nTroopCount);
			pOurTroopList[nOurCount] = i;
			nOurCount++;
		}
		else
		{
			BsAssert(nFriendlyCount < nTroopCount);
			pFriendlyTroopList[nFriendlyCount] = i;
			nFriendlyCount++;
		}
	}

	float fMapWidth,fMapHeight;
	CFcWorld::GetInstance().GetMapSize(fMapWidth,fMapHeight);

	//player Arrow
	if(nPlayerIndex != -1)
	{	
		D3DXVECTOR3 TroopPosVec = g_FcWorld.GetTroopObject(nPlayerIndex)->GetPos();
		D3DXVECTOR2 Dir = g_FcWorld.GetTroopObject(nPlayerIndex)->GetDir();

		float fAngle = acos( D3DXVec2Dot( &Dir, &D3DXVECTOR2( 0.f, 1.f ) ) );
		if( Dir.x < 0.f ) fAngle = -fAngle;

		D3DXVECTOR2 pos = D3DXVECTOR2(TroopPosVec.x / fMapWidth, 1.f - TroopPosVec.z / fMapHeight);
		DrawPlayerWave(pos);
		DrawPlayerArrow(pos, fAngle);
	}

	//enemy Troop
	for(int i=0; i<nEnemyCount; i++)
	{
		int nIndex = pEnemyTroopList[i];
		D3DXVECTOR3 TroopPosVec = g_FcWorld.GetTroopObject(nIndex)->GetPos();
		UVImage* pImg = g_UVMgr.GetUVImage(_UV_MiniMap_Enemy_s);
		DrawUnit(TroopPosVec.x / fMapWidth, 1.f - TroopPosVec.z / fMapHeight, pImg);
	}
	delete[] pEnemyTroopList;

	//friendly Troop
	for(int i=0; i<nFriendlyCount; i++)
	{
		int nIndex = pFriendlyTroopList[i];
		D3DXVECTOR3 TroopPosVec = g_FcWorld.GetTroopObject(nIndex)->GetPos();
		UVImage* pImg = g_UVMgr.GetUVImage(_UV_MiniMap_Our_Force_s);
		DrawUnit(TroopPosVec.x / fMapWidth, 1.f - TroopPosVec.z / fMapHeight, pImg);
	}
	delete[] pFriendlyTroopList;

	//Our Troop
	for(int i=0; i<nOurCount; i++)
	{
		int nIndex = pOurTroopList[i];
		D3DXVECTOR3 TroopPosVec = g_FcWorld.GetTroopObject(nIndex)->GetPos();
		UVImage* pImg = g_UVMgr.GetUVImage(_UV_MiniMap_Our_Force_s);
		DrawUnit(TroopPosVec.x / fMapWidth, 1.f - TroopPosVec.z / fMapHeight, pImg);
	}
	delete[] pOurTroopList;

	//player
	if(nPlayerIndex != -1)
	{	
		D3DXVECTOR3 TroopPosVec = g_FcWorld.GetTroopObject(nPlayerIndex)->GetPos();
		UVImage* pImg = g_UVMgr.GetUVImage(_UV_MiniMap_Player);
		DrawUnit(TroopPosVec.x / fMapWidth, 1.f - TroopPosVec.z / fMapHeight, pImg);
	}
}

void CFCMinimapUnit::DrawPlayerWave(D3DXVECTOR2 pos)
{
	int nTick = GetProcessTick() % 30;
	if(nTick / 5 >= 5){
		return;
	}

	UVImage* pImg = g_UVMgr.GetUVImage(_UV_MiniMap_player_circle_1 + nTick / 6);
	if( pImg == NULL){
		return;
	}

	int nX = (int)(m_prtPosX + ((pos.x * (float)m_prtScaleX) * m_fZoomX));
	nX -= (int)(CFcMinimap::m_fStartX * ((float)m_prtScaleX * m_fZoomX));

	int nY = (int)(m_prtPosY + ((pos.y * (float)m_prtScaleY) * m_fZoomY));
	nY -= (int)(CFcMinimap::m_fStartY * ((float)m_prtScaleY * m_fZoomY));

	int sizeX = pImg->u2 - pImg->u1;
	int sizeY = pImg->v2 - pImg->v1;

	g_BsKernel.DrawUIBox(_Ui_Mode_Image,
		nX - sizeX/2, nY - sizeY/2,
		nX + sizeX/2, nY + sizeY/2,
		0.f, D3DXCOLOR(1.f,1.f,1.f,1.f),
		0.f,	pImg->nTexId,
		pImg->u1, pImg->v1, pImg->u2, pImg->v2);
}

void CFCMinimapUnit::DrawPlayerArrow(D3DXVECTOR2 pos, float fAngle)
{
	pos.x = m_prtPosX + ((pos.x * (float)m_prtScaleX) * m_fZoomX);
	pos.x -= (int)(CFcMinimap::m_fStartX * ((float)m_prtScaleX * m_fZoomX));

	pos.y = m_prtPosY + ((pos.y * (float)m_prtScaleY) * m_fZoomY);
	pos.y -= (int)(CFcMinimap::m_fStartY * ((float)m_prtScaleY * m_fZoomY));

	UVImage* pImg = g_UVMgr.GetUVImage(_UV_MiniMap_player_direction);

	float fWidth = 10.f;
	float fHeight = 20.f;

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

	g_BsKernel.DrawUIBox_Pos4(_Ui_Mode_Image,
		(int)vPos1.x, (int)vPos1.y,
		(int)vPos2.x, (int)vPos2.y,
		(int)vPos3.x, (int)vPos3.y,
		(int)vPos4.x, (int)vPos4.y,
		0.f,  D3DXCOLOR(1, 1, 1, 1),
		pImg->nTexId,
		pImg->u1, pImg->v1, pImg->u2, pImg->v2);
}


void CFCMinimapUnit::DrawUnit(float fPosRatioX, float fPosRatioY, UVImage* pImg, float fAngle)
{
	if( pImg == NULL){
		return;
	}

	int nX = m_prtPosX + (int)((fPosRatioX * (float)m_prtScaleX) * m_fZoomX);
	nX -= (int)(CFcMinimap::m_fStartX * ((float)m_prtScaleX * m_fZoomX));

	int nY = m_prtPosY + (int)((fPosRatioY * (float)m_prtScaleY) * m_fZoomY);
	nY -= (int)(CFcMinimap::m_fStartY * ((float)m_prtScaleY * m_fZoomY));

	int sizeX = pImg->u2 - pImg->u1;
	int sizeY = pImg->v2 - pImg->v1;

	g_BsKernel.DrawUIBox(_Ui_Mode_Image,
		nX - sizeX/2, nY - sizeY/2,
		nX + sizeX/2, nY + sizeY/2,
		0.f, D3DXCOLOR(1.f,1.f,1.f,1.f),
		fAngle,	pImg->nTexId,
		pImg->u1, pImg->v1, pImg->u2, pImg->v2);
}

void CFCMinimapUnit::PointOn(bool bShow,const char *szName,CFcTroopObject *pTroop,float fPosX,float fPosY,DWORD nTick)
{
	if(bShow)
	{
		MinimapPointInfo *pData = new MinimapPointInfo;
		
		float fMapWidth,fMapHeight;
		CFcWorld::GetInstance().GetMapSize( fMapWidth , fMapHeight );

		pData->fPointX = fPosX / fMapWidth;
		pData->fPointY = 1.f - (fPosY / fMapHeight);

		pData->nTimeTick = nTick;
		pData->pTroop	= pTroop;
		strcpy( pData->szName, szName );
		m_PointInfoList.push_back(pData);
	}
	else{
		for(unsigned int i = 0;i < m_PointInfoList.size();i++){
			MinimapPointInfo* pInfo = m_PointInfoList[i];
			if( _stricmp( pInfo->szName,szName) == 0)
			{
				delete m_PointInfoList[i];
				m_PointInfoList.erase(m_PointInfoList.begin() + i);
				break;
			}
		}
	}
}



void CFCMinimapUnit::DrawBorder()
{
	/*
	//Minimap decoration
	UVImage* pImg = g_UVMgr.GetUVImage(_UV_MiniMap_Base);
	g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
		964, 54, 220, 216,
		0.f, D3DXCOLOR(1.f,1.f,1.f,1.f),
		0.f, pImg->nTexId,
		pImg->u1, pImg->v1, pImg->u2, pImg->v2);
	*/
}



void CFCMinimapUnit::DrawMark()
{
	for(unsigned int i = 0;i < m_PointInfoList.size();i++)
	{
		if(!IsPause())
		{
			m_PointInfoList[i]->nTimeTick--;

			if(!(GetProcessTick() % 5))
			{
				m_PointInfoList[i]->nMarkIndex++;
				if(m_PointInfoList[i]->nMarkIndex >= MAX_MARK_FRAME){
					m_PointInfoList[i]->nMarkIndex = 0;
				}
			}
		}

		if(m_PointInfoList[i]->pTroop) //Troop표시라면
		{
			float fMapWidth,fMapHeight;
			CFcWorld::GetInstance().GetMapSize( fMapWidth , fMapHeight );

			D3DXVECTOR3 TroopPos = m_PointInfoList[i]->pTroop->GetPos();
			m_PointInfoList[i]->fPointX = TroopPos.x / fMapWidth;
			m_PointInfoList[i]->fPointY = 1.f - (TroopPos.z / fMapHeight);
		}

		UVImage* pImg = g_UVMgr.GetUVImage(_UV_MiniMap_TroopID_O_2);// + m_PointInfoList[i]->nMarkIndex);
		DrawUnit(m_PointInfoList[i]->fPointX,m_PointInfoList[i]->fPointY, pImg);

		if(!IsPause())
		{
			CFcTroopObject* pTroop = m_PointInfoList[i]->pTroop;
			if( pTroop )
			{
				// 부대가 전멸했거나 disable됐으면 지워줌
				if( !pTroop->IsEnable() || pTroop->IsEliminated() )
				{
					m_PointInfoList.erase(m_PointInfoList.begin() + i);
					i--;
				}
				else if(m_PointInfoList[i]->nTimeTick <= 0)
				{
					m_PointInfoList.erase(m_PointInfoList.begin() + i);
					i--;
				}
			}
			else
			{
				if(m_PointInfoList[i]->nTimeTick <= 0)
				{
					m_PointInfoList.erase(m_PointInfoList.begin() + i);
					i--;
				}
			}
		}
	}
}