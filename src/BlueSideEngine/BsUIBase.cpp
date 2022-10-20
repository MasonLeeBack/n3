#include "stdafx.h"
#include "BsUIBase.h"
#include "BsKernel.h"
#include "BsMaterial.h"
#include "assert.h"
#include "BsImageProcess.h"

#include "BSuiMovieTexture.h"

#include "BsMesh.h"

#define _UI_SQUARE_COUNT		2000
#define _VERTEX_BUFFER_SIZE		4 * _UI_SQUARE_COUNT
#define _INDEX_BUFFER_SIZE		2 * _UI_SQUARE_COUNT

#define _RTT_SIZE				512

enum{
	_POS2_COLOR1,
	_POS4_COLOR1,
	_POS4_COLOR4,
};

//------------------------------------------------------------------------------------------------------------------------
CBsUIElement::CBsUIElement(int mode,
			 int x1, int y1, D3DXCOLOR color1,
			 int x2, int y2, D3DXCOLOR color2,
			 int x3, int y3, D3DXCOLOR color3,
			 int x4, int y4, D3DXCOLOR color4,
			 float fz, float fRot,
			 int nTextureID,
			 int ux1, int uy1, int ux2, int uy2,
			 int nPosColorType, char* szText,
			 int nDummyTexId,
			 bool bRTT,
			 int nMeshIndex,
			 int nViewId,
			 int nExceptionType)
{
	m_nMode = mode;

	m_nX1 = x1; m_nY1 = y1; m_Color1 = color1;
	m_nX2 = x2;	m_nY2 = y2; m_Color2 = color2;
	m_nX3 = x3; m_nY3 = y3; m_Color3 = color3;
	m_nX4 = x4;	m_nY4 = y4; m_Color4 = color4;

	m_fZ = fz;

	m_nTexId = nTextureID;
	
	m_ux1 = ux1; m_uy1 = uy1; m_ux2 = ux2; m_uy2 = uy2;

	m_fRot = fRot;

	m_nPosColorType = nPosColorType;

	m_pText=NULL;
	if(m_nMode == _Ui_Mode_Debug)
	{
		const size_t szText_len = strlen(szText)+1; //aleksger - safe string
		m_pText=new char [szText_len];
		strcpy_s(m_pText, szText_len, szText);
	}

	m_nViewId = nViewId;
	m_nDummyTexId = nDummyTexId;
	m_bRTT = bRTT;
	m_nMeshIndex = nMeshIndex;

	m_nExceptionType = nExceptionType;
}

void CBsUIElement::Clear()
{
	if (m_pText) {
		delete [] m_pText;
		m_pText=NULL;
	}

	if(m_bRTT == false)
	{
		SAFE_RELEASE_TEXTURE(m_nTexId);
		SAFE_RELEASE_TEXTURE(m_nDummyTexId);
	}
}


int CBsUIManager::m_sCurrentProcessBuffer = 0;
int CBsUIManager::m_sCurrentRenderBuffer = 0;

//------------------------------------------------------------------------------------------------------------------------
CBsUIManager::CBsUIManager()
{
	m_nUIVertexDeclIndex = -1;
	m_nUIMeshVertexDeclIndex = -1;

	m_puiv = NULL;
	m_pIndex = NULL;
	
	InitVertexShader();

	SetupShader0();
	SetupShader1();
	SetuoDecl();

	CreateVertexPool();

	//CreateClickFxEffect();

	SetViewport(_UI_FULL_VIEWER, 0.f, 0.f, 1.f, 1.f);
	SetViewport(_UI_1P_VIEWER, 0.f, 0.f, 0.495f, 1.f);
	SetViewport(_UI_2P_VIEWER, 0.505f, 0.f, 0.495f, 1.f);

	m_nExceptionType = _Ui_Exception_NONE;
}

CBsUIManager::~CBsUIManager()
{
	Clear();
}

//------------------------------------------------------------------------------------------------------------------------

void CBsUIManager::ReInitialize()
{
	Release();
    
	ClearElementAll(0);
	ClearElementAll(1);
	ClearElementBGMovie(0);
	ClearElementBGMovie(1);
	ClearElementException(0);
	ClearElementException(1);

	SetupShader0();
	SetupShader1();
	SetuoDecl();

	CreateVertexPool();

	//CreateClickFxEffect();

	SetViewport(_UI_FULL_VIEWER, 0.f, 0.f, 1.f, 1.f);
	SetViewport(_UI_1P_VIEWER, 0.f, 0.f, 0.495f, 1.f);
	SetViewport(_UI_2P_VIEWER, 0.505f, 0.f, 0.495f, 1.f);
}

bool CBsUIManager::CreateElement(int mode, 
								 int x1, int y1, D3DXCOLOR color1,
								 int x2, int y2, D3DXCOLOR color2,
								 int x3, int y3, D3DXCOLOR color3,
								 int x4, int y4, D3DXCOLOR color4,
								 float fz, float fRot,
								 int nTextureID,
								 int ux1, int uy1, int ux2, int uy2,
								 int nPosColorType, char *szText,
								 int nDummyTexId,
								 bool bRTT,
								 int nMeshIndex)
{
	CBsUIElement* pElement = NULL;
	pElement = new CBsUIElement(mode,
		x1, y1, color1,
		x2, y2, color2,
		x3, y3, color3,
		x4, y4, color4,
		fz, fRot, 
		nTextureID,
		ux1, uy1, ux2, uy2,
		nPosColorType, szText,
		nDummyTexId,
		bRTT,
		nMeshIndex,
		m_nViewerIndex,
		m_nExceptionType);

	if(bRTT == false)
	{
		if(nTextureID >= 0)
		{
			CBsTexture* pTexture = g_BsKernel.GetBsTexture(nTextureID);
			pTexture->AddRef();
		}

		if(nDummyTexId >= 0)
		{
			CBsTexture* pTexture = g_BsKernel.GetBsTexture(nDummyTexId);
			pTexture->AddRef();
		}
	}

	int nIndex = GetProcessBufferIndex();
	if(mode == _Ui_Mode_BGMovie)
	{
		m_EListforBGMovie[nIndex].push_back(pElement);
		return true;
	}

	if(mode == _Ui_Mode_Exception)
	{
		m_EListforException[nIndex].push_back(pElement);
		return true;
	}

	switch(m_nViewerIndex)
	{
	case _UI_FULL_VIEWER:
		{
			BsAssert(pElement != NULL);
			m_EListFullMode[nIndex].push_back(pElement);
			break;
		}
	case _UI_1P_VIEWER:
		{
			m_EList1PMode[nIndex].push_back(pElement);
			break;
		}
	case _UI_2P_VIEWER:
		{
			m_EList2PMode[nIndex].push_back(pElement);
			break;
		}
	}

	return true;
}

bool CBsUIManager::CreateElementDebug(int x1, int y1, D3DXCOLOR color, char *szText)
{
	bool bRet = CreateElement(_Ui_Mode_Debug, 
		x1, y1, color,
		-1, -1, -1,
		-1, -1, -1,
		-1, -1, -1,
		0.f, 0.f,
		-1,
		0, 0, 0, 0,
		_POS2_COLOR1, szText);

	return bRet;
}

bool CBsUIManager::CreateElementMesh(BS_UI_MODE mode, int nMeshIndex, int nX, int nY, D3DXCOLOR color, int nTexId,
									 float param1, float param2, float param3, float param4)
{
	switch(mode)
	{
	case _Ui_Mode_Mesh:
	case _Ui_Mode_Mesh_Gauge:
	case _Ui_Mode_Mesh_Gauge_Volumn: break;
	default:
		{
			BsAssert(0);
		}
	}
	
	D3DXCOLOR ExceptValue = D3DXCOLOR(param1, param2, param3, param4);
	bool bRet = CreateElement(mode,
		nX, nY, color,
		-1, -1, ExceptValue,
		-1, -1, -1,
		-1, -1, -1,
		0.f, 0.f,
		nTexId,
		0, 0, 0, 0,
		_POS2_COLOR1, NULL,
		-1, false, nMeshIndex);

	return bRet;
}


bool CBsUIManager::CreateElement_Pos2(int mode,
					int x1, int y1, int x2, int y2,
					float fz, D3DXCOLOR color,
					float fRot,
					int nTextureID,
					int ux1, int uy1, int ux2, int uy2,
					int nDummyTexId,
					bool bRTT)
{
	bool bRet = CreateElement(mode, 
		x1, y1, color,
		x2, y2, -1,
		-1, -1, -1,
		-1, -1, -1,
		fz, fRot,
		nTextureID,
		ux1, uy1, ux2, uy2,
		_POS2_COLOR1, NULL,
		nDummyTexId,
		bRTT);

	return bRet;
}

bool CBsUIManager::CreateElement_Pos4(int mode, 
				   int x1, int y1, int x2, int y2,
				   int x3, int y3, int x4, int y4,
				   float fz, D3DXCOLOR color,
				   int nTextureID,
				   int ux1, int uy1, int ux2, int uy2,
				   int nDummyTexId,
				   bool bRTT)
{
	bool bRet = CreateElement(mode, 
		x1, y1, color,
		x2, y2, -1,
		x3, y3, -1,
		x4, y4, -1,
		fz, 0.f,
		nTextureID,
		ux1, uy1, ux2, uy2,
		_POS4_COLOR1, NULL,
		nDummyTexId,
		bRTT);

	return bRet;
}

bool CBsUIManager::CreateElement_PosColor4(int mode, 
				   int x1, int y1, D3DXCOLOR color1,
				   int x2, int y2, D3DXCOLOR color2,
				   int x3, int y3, D3DXCOLOR color3,
				   int x4, int y4, D3DXCOLOR color4,
				   float fz,
				   int nTextureID,
				   int ux1, int uy1, int ux2, int uy2,
				   int nDummyTexId,
				   bool bRTT)
{
	bool bRet = CreateElement(mode, 
		x1, y1, color1,
		x2, y2, color2,
		x3, y3, color3,
		x4, y4, color4,
		fz, 0.f,
		nTextureID,
		ux1, uy1, ux2, uy2,
		_POS4_COLOR4, NULL,
		nDummyTexId,
		bRTT);

	return bRet;
}


void CBsUIManager::InitVertexShader()
{
	char fullName[_MAX_PATH];
	strcpy(fullName, g_BsKernel.GetShaderDirectory());
	strcat(fullName, "UIBase.fx");
	m_nMaterialIndex0 = g_BsKernel.LoadMaterial(fullName, FALSE);


	memset(fullName, 0, _MAX_PATH);
	strcpy(fullName, g_BsKernel.GetShaderDirectory());
	strcat(fullName, "UIBase_Mesh.fx");
	m_nMaterialIndex1 = g_BsKernel.LoadMaterial(fullName, FALSE);
}

void CBsUIManager::SetupShader0()
{	
	CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialIndex0);

#ifdef _XBOX
	m_hLayerTexture = pMaterial->GetParameterByName("LayerSampler");
	m_hDummyTexture = pMaterial->GetParameterByName("DummySampler");
	m_hVolumnTexture = pMaterial->GetParameterByName("VolumnSampler");
	m_hInputTime = pMaterial->GetParameterByName("Time");
	

	m_hInput_X = pMaterial->GetParameterByName("POS_X");
	m_hInput_Y = pMaterial->GetParameterByName("POS_Y");
	m_hInputClickTime = pMaterial->GetParameterByName("CLICK_TIME");
#else
	m_hLayerTexture = pMaterial->GetParameterByName("LayerTexture");
	m_hDummyTexture = pMaterial->GetParameterByName("DummyTexture");
	m_hVolumnTexture = pMaterial->GetParameterByName("VolumnTexture");
	m_hInputTime = pMaterial->GetParameterByName("time");
	

	m_hInput_X = pMaterial->GetParameterByName("fPos_X");
	m_hInput_Y = pMaterial->GetParameterByName("fPos_Y");
	m_hInputClickTime = pMaterial->GetParameterByName("fClick_time");
#endif
}

void CBsUIManager::SetupShader1()
{
	CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialIndex1);

#ifdef _XBOX
	m_hMeshTex = pMaterial->GetParameterByName("LayerSampler");
	m_hMeshVolTex = pMaterial->GetParameterByName("VolumnSampler");
	
	m_hMesh_X = pMaterial->GetParameterByName("POS_X");
	m_hMesh_Y = pMaterial->GetParameterByName("POS_Y");
	
	m_hMeshset0 = pMaterial->GetParameterByName("MESH_OFFSET0");
	m_hMeshset1 = pMaterial->GetParameterByName("MESH_OFFSET1");
	m_hMeshset2 = pMaterial->GetParameterByName("MESH_OFFSET2");
	m_hMeshset3 = pMaterial->GetParameterByName("MESH_OFFSET3");

	m_hMeshColor = pMaterial->GetParameterByName("MESH_COLOR");
#else
	m_hMeshTex = pMaterial->GetParameterByName("LayerTexture");
	m_hMeshVolTex = pMaterial->GetParameterByName("VolumnTexture");

	m_hMesh_X = pMaterial->GetParameterByName("fPos_X");
	m_hMesh_Y = pMaterial->GetParameterByName("fPos_Y");
	
	m_hMeshset0 = pMaterial->GetParameterByName("fMeshOffset0");
	m_hMeshset1 = pMaterial->GetParameterByName("fMeshOffset1");
	m_hMeshset2 = pMaterial->GetParameterByName("fMeshOffset2");
	m_hMeshset3 = pMaterial->GetParameterByName("fMeshOffset3");
	m_hMeshColor = pMaterial->GetParameterByName("fMeshColor");
#endif
}


void CBsUIManager::SetuoDecl()
{
	D3DVERTEXELEMENT9 decl[] =
	{
		{ 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		{ 0, 20, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },

		D3DDECL_END()
	};
	m_nUIVertexDeclIndex = g_BsKernel.LoadVertexDeclaration(decl);

#ifdef _XBOX
	D3DVERTEXELEMENT9 Meshdecl[] =
	{
		{ 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 1, 0, D3DDECLTYPE_FLOAT16_2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },

		D3DDECL_END()
	};
#else
	D3DVERTEXELEMENT9 Meshdecl[] =
	{
		{ 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 1, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },

		D3DDECL_END()
	};
#endif

	m_nUIMeshVertexDeclIndex = g_BsKernel.LoadVertexDeclaration(Meshdecl);
}


void CBsUIManager::CreateVertexPool()
{
	if(m_puiv == NULL)
	{
		m_puiv = new CBsUIElement::UIVERTEX[_UI_SQUARE_COUNT*4];
	}

	if(m_pIndex == NULL)
	{	
		m_pIndex = new WORD[_UI_SQUARE_COUNT*6];
		UpdateIndex(_UI_SQUARE_COUNT, m_pIndex);
	}
}

void CBsUIManager::Release()
{
	SAFE_RELEASE_VD(m_nUIVertexDeclIndex);
	SAFE_RELEASE_VD(m_nUIMeshVertexDeclIndex);

	if(m_puiv) {
		delete[] m_puiv;
		m_puiv = NULL;
	}

	if(m_pIndex) {
		delete[] m_pIndex;
		m_pIndex = NULL;
	}
}

void CBsUIManager::Clear()
{
	SAFE_RELEASE_MATERIAL(m_nMaterialIndex0);
	SAFE_RELEASE_MATERIAL(m_nMaterialIndex1);

	Release();
	ClearElementAll(0);
	ClearElementAll(1);
	ClearElementBGMovie(0);
	ClearElementBGMovie(1);
	ClearElementException(0);
	ClearElementException(1);
}


void CBsUIManager::ClearElementAll(int nIndex)
{
	DWORD dwCount = m_EListFullMode[nIndex].size();
	for (DWORD i=0; i<dwCount; i++)
	{
		if(m_EListFullMode[nIndex][i] != NULL)
		{
			delete m_EListFullMode[nIndex][i];
			m_EListFullMode[nIndex][i] = NULL;
		}
	}
	m_EListFullMode[nIndex].clear();

	dwCount = m_EList1PMode[nIndex].size();
	for (DWORD i=0; i<dwCount; i++)
	{
		if(m_EList1PMode[nIndex][i] != NULL)
		{
			delete m_EList1PMode[nIndex][i];
			m_EList1PMode[nIndex][i] = NULL;
		}
	}
	m_EList1PMode[nIndex].clear();

	dwCount = m_EList2PMode[nIndex].size();
	for (DWORD i=0; i<dwCount; i++)
	{
		if(m_EList2PMode[nIndex][i] != NULL)
		{
			delete m_EList2PMode[nIndex][i];
			m_EList2PMode[nIndex][i] = NULL;
		}
	}
	m_EList2PMode[nIndex].clear();
}


void CBsUIManager::ClearElementBGMovie(int nIndex)
{
	DWORD dwCount = m_EListforBGMovie[nIndex].size();
	for (DWORD i=0; i<dwCount; i++)
	{
		delete m_EListforBGMovie[nIndex][i];
	}
	m_EListforBGMovie[nIndex].clear();
}

void CBsUIManager::ClearElementException(int nIndex)
{
	DWORD dwCount = m_EListforException[nIndex].size();
	for (DWORD i=0; i<dwCount; i++)
	{
		delete m_EListforException[nIndex][i];
	}
	m_EListforException[nIndex].clear();
}

void CBsUIManager::SetViewport(int nViewIndex, float fX, float fY, float fWidth, float fHeight)
{
	float fScreenWidth = float(g_BsKernel.GetDevice()->GetBackBufferWidth());
	float fScreenHeight = float(g_BsKernel.GetDevice()->GetBackBufferHeight());

	D3DVIEWPORT9* pViewport = NULL;
	switch(nViewIndex)
	{
	case _UI_FULL_VIEWER: pViewport = &m_viewport; break;
	case _UI_1P_VIEWER: pViewport = &m_viewport1P; break;
	case _UI_2P_VIEWER: pViewport = &m_viewport2P; break;
	default: assert(0);
	}

	pViewport->X = int(fScreenWidth * fX);
	pViewport->Y = int(fScreenHeight * fY);
	pViewport->Width = int(fScreenWidth * fWidth);
	pViewport->Height = int(fScreenHeight * fHeight);
#ifdef INV_Z_TRANSFORM
	pViewport->MinZ = 1.f;
	pViewport->MaxZ = 0.f;
#else
	pViewport->MinZ = 0.f;
	pViewport->MaxZ = 1.f;
#endif
}

D3DVIEWPORT9* CBsUIManager::GetViewport(int nViewIndex)
{
	switch(nViewIndex)
	{
	case _UI_FULL_VIEWER: return &m_viewport;
	case _UI_1P_VIEWER: return &m_viewport1P;
	case _UI_2P_VIEWER: return &m_viewport2P;
	default: assert(0);
	}

	return NULL;
}


//----------------------------------------------------------------------------------------
void CBsUIManager::Render(C3DDevice *pDevice)
{
	DWORD pm1,pm2,pm3,pm4,pm5,pm6;
	pDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &pm1);
	pDevice->GetRenderState(D3DRS_ALPHATESTENABLE,  &pm2);
	pDevice->GetRenderState(D3DRS_SRCBLEND,         &pm3);
	pDevice->GetRenderState(D3DRS_DESTBLEND,        &pm4);
	pDevice->GetRenderState(D3DRS_FILLMODE,         &pm5);
	pDevice->GetRenderState(D3DRS_ZENABLE, &pm6);

	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE,  FALSE);
	pDevice->SetRenderState(D3DRS_SRCBLEND,         D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND,        D3DBLEND_INVSRCALPHA);
	pDevice->SetRenderState(D3DRS_FILLMODE,         D3DFILL_SOLID);
	pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	
	int nIndex = GetRenderBufferIndex();
	RenderElement(pDevice, _UI_1P_VIEWER, &m_EList1PMode[nIndex]);
	RenderElement(pDevice, _UI_2P_VIEWER, &m_EList2PMode[nIndex]);
	RenderElement(pDevice, _UI_FULL_VIEWER, &m_EListFullMode[nIndex]);
	
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, pm1);
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE,  pm2);
	pDevice->SetRenderState(D3DRS_SRCBLEND,         pm3);
	pDevice->SetRenderState(D3DRS_DESTBLEND,        pm4);
	pDevice->SetRenderState(D3DRS_FILLMODE,         pm5);
	pDevice->SetRenderState(D3DRS_ZENABLE, pm6);

	ClearElementAll(nIndex);
}

//----------------------------------------------------------------------------------------
void CBsUIManager::RenderBGMovie(C3DDevice *pDevice)
{
	DWORD pm1,pm2,pm3,pm4,pm5,pm6;
	pDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &pm1);
	pDevice->GetRenderState(D3DRS_ALPHATESTENABLE,  &pm2);
	pDevice->GetRenderState(D3DRS_SRCBLEND,         &pm3);
	pDevice->GetRenderState(D3DRS_DESTBLEND,        &pm4);
	pDevice->GetRenderState(D3DRS_FILLMODE,         &pm5);
	pDevice->GetRenderState(D3DRS_ZENABLE, &pm6);

	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE,  FALSE);
	pDevice->SetRenderState(D3DRS_SRCBLEND,         D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND,        D3DBLEND_INVSRCALPHA);
	pDevice->SetRenderState(D3DRS_FILLMODE,         D3DFILL_SOLID);
	pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);

	int nIndex = GetRenderBufferIndex();
	RenderElement(pDevice, _UI_FULL_VIEWER, &m_EListforBGMovie[nIndex]);

	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, pm1);
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE,  pm2);
	pDevice->SetRenderState(D3DRS_SRCBLEND,         pm3);
	pDevice->SetRenderState(D3DRS_DESTBLEND,        pm4);
	pDevice->SetRenderState(D3DRS_FILLMODE,         pm5);
	pDevice->SetRenderState(D3DRS_ZENABLE, pm6);

	ClearElementBGMovie(nIndex);
}

//----------------------------------------------------------------------------------------
void CBsUIManager::RenderException(C3DDevice *pDevice, int nExceptionType)
{
	int nIndex = GetRenderBufferIndex();

	uiElementLIST pTempList;
	int nCurTexId(-1);
	DWORD size = m_EListforException[nIndex].size();
	for(DWORD i=0; i<size; i++)
	{
		CBsUIElement* pElement = m_EListforException[nIndex][i];
		if(pElement == NULL){
			BsAssert(0);
			continue;
		}

		if(pElement->m_nExceptionType != nExceptionType){
			continue;
		}

		BsAssert(pElement->m_nMode == _Ui_Mode_Exception);

		if(i==0){
			nCurTexId = pElement->m_nTexId;
		}
		
		if(pElement->m_nTexId != nCurTexId)
		{
			if(pTempList.size() > 0){
				RenderObjectUp(pDevice, &pTempList);
			}

			nCurTexId = pElement->m_nTexId;
			pTempList.clear();
		}

		pTempList.push_back(pElement);
	}

	if(pTempList.size() > 0){
		RenderObjectUp(pDevice, &pTempList);
	}

	pTempList.clear();

	//ClearElementException()는 RTT가 끝나고 부른다.
}


void CBsUIManager::RenderElement(C3DDevice *pDevice, int nViewIndex, uiElementLIST* pElementList)
{
	if(pElementList->size() <= 0){
		return;
	}

	D3DVIEWPORT9* pViewport = GetViewport(nViewIndex);
	assert(pViewport != NULL);
	pDevice->SetViewport(pViewport);
    
	uiElementLIST pTempList;
	int nCurMode(-1), nCurTexId(-1);
	DWORD size = pElementList->size();
	for(DWORD i=0; i<size; i++)
	{
		CBsUIElement* pElement = (*pElementList)[i];
		if(pElement == NULL){
			BsAssert(0);
			continue;
		}

		if(i==0)
		{
			nCurMode = pElement->m_nMode;
			nCurTexId = pElement->m_nTexId;
		}

		switch(pElement->m_nMode)
		{
		case _Ui_Mode_Debug:
			{
				if(pTempList.size() > 0)
				{
					RenderObjectUp(pDevice, &pTempList);
					pTempList.clear();
				}

				g_BsKernel.RenderUIDebug(pElement->m_nX1,
					pElement->m_nY1,
					pElement->m_pText, 
					GetdwColor(pElement->m_Color1));

				nCurMode = pElement->m_nMode;
				nCurTexId = pElement->m_nTexId;

				continue;
			}
		case _Ui_Mode_Mesh:
		case _Ui_Mode_Mesh_Gauge:
		case _Ui_Mode_Mesh_Gauge_Volumn:
			{
				if(pTempList.size() > 0)
				{
					RenderObjectUp(pDevice, &pTempList);
					pTempList.clear();
				}

				RenderUIMesh(pDevice, pElement);

				nCurMode = pElement->m_nMode;
				nCurTexId = pElement->m_nTexId;

				continue;
			}
		}

		if(pTempList.size() >= _UI_SQUARE_COUNT)
		{
			RenderObjectUp(pDevice, &pTempList);
			pTempList.clear();

			nCurMode = pElement->m_nMode;
			nCurTexId = pElement->m_nTexId;
		}
		else if(nCurTexId != pElement->m_nTexId || nCurMode != pElement->m_nMode)
		{
			if(pTempList.size() > 0){
				RenderObjectUp(pDevice, &pTempList);
			}

			nCurMode = pElement->m_nMode;
			nCurTexId = pElement->m_nTexId;

			pTempList.clear();
		}

		pTempList.push_back(pElement);
	}
    
	if(pTempList.size() > 0){
		RenderObjectUp(pDevice, &pTempList);
	}

	pTempList.clear();
}

void CBsUIManager::RenderObjectUp(C3DDevice *pDevice, uiElementLIST* pElementList)
{
	BsAssert(m_puiv != NULL);

	UpdateVertex(pElementList, m_puiv);
	CBsUIElement* pElement = (*pElementList)[0];
	if(pElement == NULL) {
		return;
	}
	
	CBsTexture* pBsTexture = g_BsKernel.GetBsTexture(pElement->m_nTexId);

	int nTechIndex = 0;
	switch(pElement->m_nMode)
	{
	case _Ui_Mode_Box:
	case _Ui_Mode_Box_ZTEST:
		nTechIndex = 0;
		break;
	case _Ui_Mode_Image:
	case _Ui_Mode_Text:
	case _Ui_Mode_Exception:
	case _Ui_Mode_Movie:
	case _Ui_Mode_BGMovie:
		{
			if( pBsTexture && pBsTexture->GetRenderTargetSurface() ) {
				nTechIndex = 3;		// Blur 가능한 Technique!!
			}
			else {
				nTechIndex = 1;
			}
		}
		break;
	case _Ui_Mode_ALPHA:
		nTechIndex = 2;
		break;
	default: assert(0);
	}

	CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialIndex0);
	switch(pElement->m_nMode)
	{
	case _Ui_Mode_Box:
	case _Ui_Mode_Box_ZTEST:
		break;
	case _Ui_Mode_ALPHA:
		{	
			if(g_BsKernel.GetBsTexture(pElement->m_nTexId) == NULL) {
				BsAssert(0);
				return;
			}
			if(g_BsKernel.GetBsTexture(pElement->m_nDummyTexId) == NULL){
				BsAssert(0);
				return;
			}
			pMaterial->SetTexture(m_hLayerTexture, pElement->m_nTexId);
			pMaterial->SetTexture(m_hDummyTexture, pElement->m_nDummyTexId);
			break;
		}
	default:
		{
			if(g_BsKernel.GetBsTexture(pElement->m_nTexId) == NULL){
				BsAssert(0);
				return;
			}
			pMaterial->SetTexture(m_hLayerTexture, pElement->m_nTexId);
		}
	}
	
	pMaterial->SetFloat(m_hInputTime, (float)g_BsKernel.GetRenderTick());

	pMaterial->SetFloat(m_hInput_X, 0.5f);
	pMaterial->SetFloat(m_hInput_Y, 0.5f);

	static float fa = 0.f;
	if(g_BsKernel.GetRenderTick() % 200 == 0){
		fa = (float)g_BsKernel.GetRenderTick();
	}
	pMaterial->SetFloat(m_hInputClickTime, fa);

	pMaterial->BeginMaterial(nTechIndex, 0);
	pMaterial->BeginPass(0);

	pMaterial->CommitChanges();

	g_BsKernel.SetVertexDeclaration(m_nUIVertexDeclIndex);

	int nCount = pElementList->size();
	DrawIndexedPrimitiveUP(pDevice, nCount);

	pMaterial->EndPass();
	pMaterial->EndMaterial();
	
	//delete[] puiv;
}

void CBsUIManager::DrawIndexedPrimitiveUP(C3DDevice *pDevice, int nCount)
{
	pDevice->DrawIndexedPrimitiveUP( D3DPT_TRIANGLELIST, 0, nCount*4, nCount*2,
		m_pIndex, D3DFMT_INDEX16, m_puiv, sizeof(CBsUIElement::UIVERTEX) );
}


void CBsUIManager::UpdateVertex(uiElementLIST* pElementList, CBsUIElement::UIVERTEX* puiv)
{
	D3DPRESENT_PARAMETERS *pp=g_BsKernel.GetPresentParameer();
	int nBsx(pp->BackBufferWidth), nBsy(pp->BackBufferHeight);
	D3DXVECTOR2 vecSize(0.f, 0.f);
	
	CBsUIElement* pElement = (*pElementList)[0];
	D3DVIEWPORT9 Viewport;
	g_BsKernel.GetDevice()->GetViewport(&Viewport);
	
	switch(pElement->m_nMode)
	{
	case _Ui_Mode_Image:
	case _Ui_Mode_Text:
	case _Ui_Mode_ALPHA:
	case _Ui_Mode_Exception:
		{
			assert(pElement->m_nTexId != -1);

			SIZE size = g_BsKernel.GetTextureSize(pElement->m_nTexId);
			vecSize = D3DXVECTOR2((float)size.cx, (float)size.cy);

			float fScreenWidth = float(g_BsKernel.GetDevice()->GetBackBufferWidth());
			float fScreenHeight = float(g_BsKernel.GetDevice()->GetBackBufferHeight());		

			float fWidth = Viewport.Width / fScreenWidth;
			float fHeight = Viewport.Height / fScreenHeight;

			nBsx = (int)(nBsx * fWidth);
			nBsy = (int)(nBsy * fHeight);

			break;
		}
	case _Ui_Mode_Movie:
	case _Ui_Mode_BGMovie:
		{	
			vecSize = D3DXVECTOR2((float)Viewport.Width, (float)Viewport.Height);
			break;
		}
	case _Ui_Mode_Box:
	case _Ui_Mode_Box_ZTEST:
		{
			vecSize = D3DXVECTOR2(1.f, 1.f);
		}
		break;
	default:
		{
			assert(0);
		}
	}	

	for(DWORD i=0; i<pElementList->size(); i++)
	{	
		pElement = (*pElementList)[i];	// mruete: prefix bug 475: re-using outer variable
		int nIndex = i * 4;
		
		if(pElement->m_nPosColorType == _POS2_COLOR1)
		{	
			float fWidth = (float)(pElement->m_nX2 - pElement->m_nX1);
			float fHeight = (float)(pElement->m_nY2 - pElement->m_nY1);

			D3DXVECTOR2 center;
			center.x = fWidth/2.f + pElement->m_nX1;
			center.y = fHeight/2.f + pElement->m_nY1;

			D3DXVECTOR2 lt = D3DXVECTOR2(-fWidth / 2.0f, -fHeight / 2.0f);
			D3DXVECTOR2 rb = D3DXVECTOR2(fWidth / 2.0f, fHeight / 2.0f);

			D3DXVECTOR2 Crd[4];
			Crd[0] = D3DXVECTOR2(lt.x, rb.y);
			Crd[1] = D3DXVECTOR2(rb.x, rb.y);
			Crd[2] = D3DXVECTOR2(lt.x, lt.y);
			Crd[3] = D3DXVECTOR2(rb.x, lt.y);

			float fRot = pElement->m_fRot;
			D3DXVECTOR2 pos[4];
			for(int nScan = 0;nScan < 4;nScan++)
			{
				pos[nScan].x = (float)cos(fRot) * Crd[nScan].x - (float)sin(fRot) * Crd[nScan].y + center.x;
				pos[nScan].y = (float)sin(fRot) * Crd[nScan].x + (float)cos(fRot) * Crd[nScan].y + center.y;

				pos[nScan].x = (pos[nScan].x / nBsx - 0.5f)*2;
				pos[nScan].y = -(pos[nScan].y / nBsy - 0.5f)*2;
			}

			float fz = pElement->m_fZ;

			// vectex pos
			puiv[nIndex].vecPos		= D3DXVECTOR3(pos[0].x, pos[0].y, fz);
			puiv[nIndex+1].vecPos	= D3DXVECTOR3(pos[1].x, pos[1].y, fz);
			puiv[nIndex+2].vecPos	= D3DXVECTOR3(pos[2].x, pos[2].y, fz);
			puiv[nIndex+3].vecPos	= D3DXVECTOR3(pos[3].x, pos[3].y, fz);
		}
		else
		{
			float fx1= ((float)pElement->m_nX1 / nBsx - 0.5f)*2;
			float fy1=-((float)pElement->m_nY1 / nBsy - 0.5f)*2;
			float fx2= ((float)pElement->m_nX2 / nBsx - 0.5f)*2;
			float fy2=-((float)pElement->m_nY2 / nBsy - 0.5f)*2;
			float fx3= ((float)pElement->m_nX3 / nBsx - 0.5f)*2;
			float fy3=-((float)pElement->m_nY3 / nBsy - 0.5f)*2;
			float fx4= ((float)pElement->m_nX4 / nBsx - 0.5f)*2;
			float fy4=-((float)pElement->m_nY4 / nBsy - 0.5f)*2;
			float fz = pElement->m_fZ;

			// vectex pos
			puiv[nIndex].vecPos		= D3DXVECTOR3(fx3, fy3, fz);
			puiv[nIndex+1].vecPos	= D3DXVECTOR3(fx4, fy4, fz);
			puiv[nIndex+2].vecPos	= D3DXVECTOR3(fx1, fy1, fz);
			puiv[nIndex+3].vecPos	= D3DXVECTOR3(fx2, fy2, fz);
		}
		
		D3DXVECTOR2 vecUVdot1, vecUVdot2;
		switch(pElement->m_nMode)
		{
		case _Ui_Mode_Image:
		case _Ui_Mode_Text:
		case _Ui_Mode_ALPHA:
		case _Ui_Mode_Exception:
			{
				vecUVdot1 = D3DXVECTOR2((float)pElement->m_ux1, (float)pElement->m_uy1);
				vecUVdot2 = D3DXVECTOR2((float)pElement->m_ux2, (float)pElement->m_uy2);
			}
			break;
		case _Ui_Mode_Movie:
		case _Ui_Mode_BGMovie:
			{
				vecUVdot1 = D3DXVECTOR2((float)pElement->m_nX1, (float)pElement->m_nY1);
				vecUVdot2 = D3DXVECTOR2((float)pElement->m_nX2, (float)pElement->m_nY2);
			}
			break;
		case _Ui_Mode_Box:
		case _Ui_Mode_Box_ZTEST:
			{
				vecUVdot1 = D3DXVECTOR2(0.f, 0.f);
				vecUVdot2 = D3DXVECTOR2(1.f, 1.f);
			}
			break;
		default:
			{
				assert(0);
			}
		}

		D3DXVECTOR2 vecUV1(0.f, 0.f), vecUV2(0.f, 0.f);
		D3DXVECTOR2 vecUV3(0.f, 0.f), vecUV4(0.f, 0.f);
		if(vecSize.x != 0.f && vecSize.y != 0.f)
		{
			vecUV1 = D3DXVECTOR2(vecUVdot1.x / vecSize.x, vecUVdot2.y / vecSize.y);
			vecUV2 = D3DXVECTOR2(vecUVdot2.x / vecSize.x, vecUVdot2.y / vecSize.y);
			vecUV3 = D3DXVECTOR2(vecUVdot1.x / vecSize.x, vecUVdot1.y / vecSize.y);
			vecUV4 = D3DXVECTOR2(vecUVdot2.x / vecSize.x, vecUVdot1.y / vecSize.y);
		}

		// texture UV pos
		puiv[nIndex].vecUV	= vecUV1;
		puiv[nIndex+1].vecUV	= vecUV2;
		puiv[nIndex+2].vecUV	= vecUV3;
		puiv[nIndex+3].vecUV	= vecUV4;

		
		// color
		if(pElement->m_nPosColorType != _POS4_COLOR4)
		{
			DWORD dwColor = GetdwColor(pElement->m_Color1);
			puiv[nIndex].dwColor	= dwColor;
			puiv[nIndex+1].dwColor	= dwColor;
			puiv[nIndex+2].dwColor	= dwColor;
			puiv[nIndex+3].dwColor	= dwColor;
		}
		else
		{
			DWORD dwColor1 = GetdwColor(pElement->m_Color1);
			DWORD dwColor2 = GetdwColor(pElement->m_Color2);
			DWORD dwColor3 = GetdwColor(pElement->m_Color3);
			DWORD dwColor4 = GetdwColor(pElement->m_Color4);

			puiv[nIndex].dwColor	= dwColor3;
			puiv[nIndex+1].dwColor	= dwColor4;
			puiv[nIndex+2].dwColor	= dwColor1;
			puiv[nIndex+3].dwColor	= dwColor2;
		}
	}
}

void CBsUIManager::UpdateIndex(int nCount, WORD *pIndex)
{
	for(int i=0;i<nCount;i++){
		pIndex[i*6]=i*4;
		pIndex[i*6+1]=i*4+2;
		pIndex[i*6+2]=i*4+1;

		pIndex[i*6+3]=i*4+1;
		pIndex[i*6+4]=i*4+2;
		pIndex[i*6+5]=i*4+3;
	}
}


void CBsUIManager::RenderUIMesh(C3DDevice *pDevice, CBsUIElement* pElement)
{	
	CBsMesh* pBsMesh = g_BsKernel.Get_pMesh(pElement->m_nMeshIndex);
	int nCount = pBsMesh->GetSubMeshCount();
	if(nCount == 0){
		return;
	}

	float fScreenWidth = float(g_BsKernel.GetDevice()->GetBackBufferWidth());
	float fScreenHeight = float(g_BsKernel.GetDevice()->GetBackBufferHeight());		

	D3DVIEWPORT9 Viewport;
	g_BsKernel.GetDevice()->GetViewport(&Viewport);
	float fWidth = Viewport.Width / fScreenWidth;
	float fHeight = Viewport.Height / fScreenHeight;

	D3DPRESENT_PARAMETERS *pp = g_BsKernel.GetPresentParameer();
	int nBsx = (int)(pp->BackBufferWidth * fWidth);
	int nBsy = (int)(pp->BackBufferHeight * fHeight);
	
	float fX = (((float)pElement->m_nX1 / nBsx) - 0.5f) * 2.f;
	float fY = - (((float)pElement->m_nY1 / nBsy) - 0.5f) * 2.f;

	CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialIndex1);
	for(int i=0; i<nCount; i++)
	{
		int nTechIndex = 6;
		switch(pElement->m_nMode)
		{
		case _Ui_Mode_Mesh:
			{
				nTechIndex = 0;
				pMaterial->SetTexture(m_hMeshTex, pElement->m_nTexId);

				break;
			}
		case _Ui_Mode_Mesh_Gauge:
			{
				nTechIndex = 1;
				pMaterial->SetTexture(m_hMeshTex, pElement->m_nTexId);

				break;
			}
		case _Ui_Mode_Mesh_Gauge_Volumn:
			{
				nTechIndex = 2;
				pMaterial->SetTexture(m_hMeshVolTex, pElement->m_nTexId);
				
				break;
			}
		default: BsAssert(0);
		}

		float fScaleX = pElement->m_Color2.r;
		float fScaleY = pElement->m_Color2.g;
		float fValue = pElement->m_Color2.b;
		float fFrame = pElement->m_Color2.a;

		pMaterial->SetFloat(m_hMeshset0, fScaleX);
		pMaterial->SetFloat(m_hMeshset1, fScaleY);
		pMaterial->SetFloat(m_hMeshset2, fValue);
		pMaterial->SetFloat(m_hMeshset3, fFrame);

		D3DXVECTOR4 vColor;
		vColor.x = pElement->m_Color1.r;
		vColor.y = pElement->m_Color1.g;
		vColor.z = pElement->m_Color1.b;
		vColor.w = pElement->m_Color1.a;
		pMaterial->SetVector(m_hMeshColor, &vColor);
		pMaterial->SetFloat(m_hMesh_X, fX);
		pMaterial->SetFloat(m_hMesh_Y, fY);
		
		CBsSubMesh* pSubMesh = pBsMesh->GetSubMeshPt(i);
		CBsStream* pBsStream = pSubMesh->GetStreamForLOD();

		LPDIRECT3DVERTEXBUFFER9 posVB, uvVB;
		LPDIRECT3DINDEXBUFFER9 posIB, uvIB;
		pBsStream->GetStream(BS_USAGE_POSITION, posVB, posIB );
		pBsStream->GetStream(BS_USAGE_TEXCOORD, uvVB, uvIB );
		int nVBCount = pBsStream->GetVertexCount();
		int nPrimitiveCount = pBsStream->GetPrimitiveCount();

		pDevice->SetIndices(posIB);
		pDevice->SetStreamSource(0, posVB, sizeof(D3DXVECTOR3));
#ifdef _XBOX
		pDevice->SetStreamSource(1, uvVB, sizeof(D3DXFLOAT16)*2);
#else
		pDevice->SetStreamSource(1, uvVB, sizeof(float)*2);
#endif
		pMaterial->BeginMaterial(nTechIndex, 0);
		pMaterial->BeginPass(0);
		
		g_BsKernel.SetVertexDeclaration(m_nUIMeshVertexDeclIndex);

		pMaterial->CommitChanges();

		pDevice->GetD3DDevice()->DrawIndexedPrimitive(pBsStream->GetPrimitiveType(), 0, 0,	nVBCount, 0, nPrimitiveCount);

		pMaterial->EndPass();
		pMaterial->EndMaterial();
	}
}
