#include "stdafx.h"
#include "FcFXScreenWaterDrop.h"
#include "BsKernel.h"
#include "FcGlobal.h"
#include "FcFxBase.h"
#include "BsImageProcess.h"
#include "BsMaterial.h"
#include "FcUtil.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


int	CFcFXScreenWaterDrop::_MAX_DROP		= 50;
int	CFcFXScreenWaterDrop::_MAX_QUAD		= 50*60*5;


//물방울 이동  
float DROP::_fMoveX[120] = {	0.00000f, 0.00000f, 0.00000f, 0.00002f, 0.00003f,		0.00003f, 0.00002f, 0.00002f, 0.00002f, 0.00003f,
								0.00004f, 0.00004f, 0.00004f, 0.00005f, 0.00006f,		0.00007f, 0.00008f, 0.00007f, 0.00005f, 0.00004f,
								0.00004f, 0.00003f, 0.00003f, 0.00002f, 0.00002f,		0.00001f, 0.00000f, 0.00000f, 0.00000f, -0.00001f,
								-0.00002f,-0.00003f, -0.00003f, -0.00004f, -0.00005f,	-0.00005f, -0.00006f, -0.00007f, -0.00007f, -0.00008f,
								-0.00006f,-0.00005f, -0.00005f, -0.00004f, -0.00003f,	-0.00003f, -0.00002f, -0.00002f, -0.00001f,  0.00000f,
								0.00001f, 0.00001f, 0.00001f, 0.00002f, 0.00002f,		0.00002f, 0.00003f, 0.00003f, 0.00004f, 0.00004f,
								0.00004f, 0.00004f, 0.00003f, 0.00003f, 0.00003f,		0.00002f, 0.00002f, 0.00002f, 0.00001f, 0.00001f,
								0.00000f, 0.00000f, -0.00001f, -0.00001f, -0.00002f,	-0.00003f, -0.00004f, -0.00004f, -0.00003f, -0.00002f,
								-0.00001f,0.00000f, 0.00000f, 0.00002f, 0.00003f,		0.00004f, 0.00004f, 0.00005f, 0.00003f, 0.00003f,
								0.00002f, 0.00000f, 0.00000f, 0.00000f, -0.00002f,		-0.00003f, -0.00004f, -0.00005f, -0.00005f, -0.00006f,
								-0.00006f,-0.00005f, -0.00005f, -0.00004f, -0.00003f,	-0.00003f, -0.00002f, -0.00002f, -0.00002f, -0.00001f,
								0.00000f, -0.00001f, -0.00001f, 0.00001f, 0.00002f,	0.00003f, 0.00003f, 0.00002f, 0.00001f, 100.00f};

float DROP::_fMoveY[100] = {	-.00007f, -.00008f, -.00007f, -.00007f, -.00006f,		-.00006f, -.00006f, -.00008f, -.00009f, -.00009f,
								-.00010f, -.00012f, -.00013f, -.00017f, -.00019f,		-.00019f, -.00011f, -.00021f, -.00024f, -.00025f,
								-.00027f, -.00028f, -.00030f, -.00031f, -.00032f,		-.00032f, -.00032f, -.00031f, -.00031f, -.00032f,
								-.00032f, -.00033f, -.00033f, -.00034f, -.00035f,		-.00035f, -.00033f, -.00031f, -.00030f, -.00029f,
								-.00027f, -.00024f, -.00024f, -.00024f, -.00022f,		-.00021f, -.00020f, -.00020f, -.00019f, -.00018f,
								-.00018f, -.00018f, -.00017f, -.00015f, -.00014f,		-.00013f, -.00013f, -.00013f, -.00012f, -.00012f,
								-.00012f, -.00012f, -.00010f, -.00010f, -.00013f,		-.00014f, -.00014f, -.00015f, -.00015f, -.00017f,
								-.00018f, -.00018f, -.00019f, -.00018f, -.00019f,		-.00029f, -.00021f, -.00022f, -.00022f, -.00022f,
								-.00021f, -.00020f, -.00020f, -.00019f, -.00017f,		-.00016f, -.00015f, -.00014f, -.00012f, -.00012f,
								-.00012f, -.00011f, -.00011f, -.00010f, -.00009f,		-.00009f, -.00009f, -.00008f, -.00008f, 100.00f};


void DROP::Set(float fxIn, float fyIn, float fSizeIn, int nFormTimeIn)
{
	fx  = fxIn, fy = fyIn, fSize = fSizeIn;
	nFormTime = (int)( (float)nFormTimeIn/**256.f*fSizeIn*/); // 크기에 비례하게 fSize*2560이 한픽셀, DEFAULT: 10픽셀짜리가 1초 맻힘
	wMoveTableIdxX = RandomNumberInRange( 0,  120);
	wMoveTableIdxY = RandomNumberInRange( 0,  100);

	fAccelX	= (float)RandomNumberInRange( 0,  40);
	fAccelY	= (float)RandomNumberInRange( (int)fAccelX+10,  50);
	fAccelX -= 20.f;
}



void DROP::GetMoveDelta(float &fDxOut, float &fDyOut)
{
	fDxOut = _fMoveX[ GetMoveTableIdx(1) ];
	fDyOut = _fMoveY[ GetMoveTableIdx() ];

	if (fDxOut >= 1.f) 
	{
		ResetMoveTableIdx(1);
		fDxOut = _fMoveX[ 0 ];
	}

	if (fDyOut >= 1.f) 
	{
		ResetMoveTableIdx(0);
		fDyOut = _fMoveY[ 0 ];
	}

	fDxOut *= fAccelX;
	fDyOut *= fAccelY;

}


CFcFXScreenWaterDrop::CFcFXScreenWaterDrop()
{
	//CFcFXBase::CFcFXBase(); //aleksger: prefix bug 743: Calling constructor directly does not have any effect.
	m_fLifeTime		= 180; //단위 : 프레임
	m_nPoolIdx		= 0;
	m_pVBLine		= NULL;
	m_pVBDrop		= NULL;
	m_fDropSize		= 0.01f;
	m_fCurTime		= 0.f;
	m_nGenOften		= 0;

	m_pVBLine		= NULL;
	m_pVBDrop		= NULL;

	m_nTextureId	  = -1;
	m_nMaterialIndex  = -1;
	m_Drops.resize(CFcFXScreenWaterDrop::_MAX_DROP);

	m_nLineVertexDeclIdx = -1;
	m_nDropVertexDeclIdx = -1;
#ifndef _LTCG
	SetFxRtti(FX_TYPE_WATERDROP);
#endif //_LTCG
}


CFcFXScreenWaterDrop::~CFcFXScreenWaterDrop()
{
	m_Drops.clear();
}

void	CFcFXScreenWaterDrop::Initialize()
{
	for(int ii = 0 ; ii < 10 ; ++ii)
		GenDrop();
	CFcFXBase::Initialize();
}

void	CFcFXScreenWaterDrop:: InitDeviceData()
{
	char fullName[_MAX_PATH];
	strcpy(fullName, g_BsKernel.GetShaderDirectory());
	strcat(fullName, "WaterDrop.fx");
	m_nMaterialIndex = g_BsKernel.LoadMaterial(fullName, FALSE);
	BsAssert( m_nMaterialIndex != -1  && "Water Shader Compile Error!!" );

	g_BsKernel.chdir("Fx");
	m_nTextureId = g_BsKernel.LoadTexture("drop.dds");
	g_BsKernel.chdir("..");

	CBsMaterial* pBsMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialIndex);
#ifdef _XBOX
	m_hTexture	= pBsMaterial->GetParameterByName("TextureSampler");
	m_hConst	= pBsMaterial->GetParameterByName("vConst");
	m_hNormTexture	= pBsMaterial->GetParameterByName("NormTextureSampler");
#else
	m_hTexture	= pBsMaterial->GetParameterByName("diffuseTexture");
	m_hConst	= pBsMaterial->GetParameterByName("vConst");
	m_hNormTexture	= pBsMaterial->GetParameterByName("normTexture");
#endif

	D3DVERTEXELEMENT9 decl[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 8, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};

	
	m_nLineVertexDeclIdx = g_BsKernel.LoadVertexDeclaration(decl);

	D3DVERTEXELEMENT9 decl2[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 8, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};
	m_nDropVertexDeclIdx = g_BsKernel.LoadVertexDeclaration(decl2);

	SAFE_RELEASE(m_pVBLine);

	g_BsKernel.CreateVertexBuffer(_MAX_QUAD*sizeof(DROPLINE_VERTEX)*6, 
		D3DUSAGE_WRITEONLY, sizeof(DROPLINE_VERTEX), D3DPOOL_MANAGED, &m_pVBLine);
	DROPLINE_VERTEX *pData;
	m_pVBLine->Lock(0, 0, (void **)&pData, 0);
	ZeroMemory(pData, _MAX_QUAD*sizeof(DROPLINE_VERTEX)*6);
	m_pVBLine->Unlock();

	SAFE_RELEASE(m_pVBDrop);
	g_BsKernel.CreateVertexBuffer(4*sizeof(DROP_VERTEX), 
		D3DUSAGE_WRITEONLY, sizeof(DROP_VERTEX), D3DPOOL_MANAGED, &m_pVBDrop);
	float fAspect = 720.f / 1280.f;
	DROP_VERTEX *pDataDrop;
	m_pVBDrop->Lock(0, 0, (void **)&pDataDrop, 0);
	pDataDrop[0].vPos = D3DXVECTOR2( -fAspect,	1.f);
	pDataDrop[0].vTex = D3DXVECTOR2(  0.f,		0.f);
	pDataDrop[1].vPos = D3DXVECTOR2(  fAspect,	1.f);
	pDataDrop[1].vTex = D3DXVECTOR2(  1.f,		0.f);
	pDataDrop[2].vPos = D3DXVECTOR2( -fAspect, -1.f);
	pDataDrop[2].vTex = D3DXVECTOR2(  0.f,		1.f);
	pDataDrop[3].vPos = D3DXVECTOR2(  fAspect, -1.f);
	pDataDrop[3].vTex = D3DXVECTOR2(  1.f,		1.f);
	m_pVBDrop->Unlock();

}

void	CFcFXScreenWaterDrop::ReleaseDeviceData()
{
	SAFE_RELEASE_MATERIAL(m_nMaterialIndex);
	SAFE_RELEASE(m_pVBLine);

	SAFE_RELEASE_VD(m_nLineVertexDeclIdx);

	SAFE_RELEASE(m_pVBDrop);

	SAFE_RELEASE_VD(m_nDropVertexDeclIdx);

	SAFE_RELEASE_TEXTURE(m_nTextureId);
}


void	CFcFXScreenWaterDrop::Process()
{
	m_fCurTime = float(GetTick());

	

	//Generate Drop
	if(m_nGenOften &&  rand() % 100  < m_nGenOften )
		GenDrop();

	//Check State
	if(m_nGenOften == 0 && IsActiveAnyDrop() == FALSE )
		m_state = READY;
}

void	CFcFXScreenWaterDrop::PreRender()
{
	if( IsPlayEvent() || IsPlayRealMovie() ) return;

	CFcFXBase::PreRender();
	int nDrop = m_Drops.size();
	DROPLINE_VERTEX* pData;

	if( m_state != PLAY ) return;

	float fdx, fdy, fnextX, fnextY;
	for( int ii = 0 ; ii < nDrop ; ++ii )
	{
		if(m_Drops[ii].IsActive() == FALSE)
			continue;

		if(m_Drops[ii].IsFormed() )
			m_Drops[ii].ProcessFormed();
		else
		{
			m_Drops[ii].GetMoveDelta(fdx, fdy);

			fnextX = m_Drops[ii].fx + fdx;
			fnextY = m_Drops[ii].fy + fdy;
			m_pVBLine->Lock(m_nPoolIdx*sizeof(DROPLINE_VERTEX)*6, sizeof(DROPLINE_VERTEX)*6, (void**)&pData, 0);	
			pData[0].vPos.x = m_Drops[ii].fx - m_Drops[ii].fSize;
			pData[0].vPos.y = m_Drops[ii].fy;
			pData[0].vTex.x = 0.f;//(m_Drops[ii].fx - fRefract)*0.5f+0.5f;
			pData[0].vTex.y = -m_Drops[ii].fy*0.5f+0.5f;
			pData[0].vTex.z = m_fCurTime-1.f;

			pData[1].vPos.x = m_Drops[ii].fx + m_Drops[ii].fSize;
			pData[1].vPos.y = m_Drops[ii].fy;
			pData[1].vTex.x = 1.f;//(m_Drops[ii].fx + fRefract)*0.5f+0.5f;
			pData[1].vTex.y = -m_Drops[ii].fy*0.5f+0.5f;
			pData[1].vTex.z = m_fCurTime-1.f;

			pData[2].vPos.x = fnextX - m_Drops[ii].fSize;
			pData[2].vPos.y = fnextY;
			pData[2].vTex.x = 0.f;//(fnextX - fRefract)*0.5f+0.5f;
			pData[2].vTex.y = -fnextY*0.5f+0.5f;
			pData[2].vTex.z = m_fCurTime;


			pData[3].vPos.x = m_Drops[ii].fx + m_Drops[ii].fSize;
			pData[3].vPos.y = m_Drops[ii].fy;
			pData[3].vTex.x = 1.f;//(m_Drops[ii].fx + fRefract)*0.5f+0.5f;
			pData[3].vTex.y = -m_Drops[ii].fy*0.5f+0.5f;
			pData[3].vTex.z = m_fCurTime-1.f;

			pData[4].vPos.x = fnextX + m_Drops[ii].fSize;
			pData[4].vPos.y = fnextY;
			pData[4].vTex.x = 1.f;//(fnextX + fRefract)*0.5f+0.5f;
			pData[4].vTex.y = -fnextY*0.5f+0.5f;
			pData[4].vTex.z = m_fCurTime;

			pData[5].vPos.x = fnextX - m_Drops[ii].fSize;
			pData[5].vPos.y = fnextY;
			pData[5].vTex.x = 0.f;//(fnextX - fRefract)*0.5f+0.5f;
			pData[5].vTex.y = -fnextY*0.5f+0.5f;
			pData[5].vTex.z = m_fCurTime;
			m_pVBLine->Unlock();

			m_Drops[ii].ProcessDown(fdx, fdy);
			++m_nPoolIdx;
			if(m_nPoolIdx >= _MAX_QUAD)
				m_nPoolIdx = 0;
		}
	}
}

void	CFcFXScreenWaterDrop::Render(C3DDevice *pDevice)
{
	if( IsPlayEvent() || IsPlayRealMovie() ) return;
	if(m_state != PLAY)
		return;
	LPDIRECT3DDEVICE9 pD3DDevice = pDevice->GetD3DDevice();

	CBsMaterial* pBsMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialIndex);


	D3DXVECTOR4 vConst(m_fCurTime, m_fLifeTime, 0.f , 0.f);
	//*
	pBsMaterial->BeginMaterial(0, 0);
	pBsMaterial->BeginPass(0);

	int nBackBuffer = g_BsKernel.GetImageProcess()->GetBackBufferTexture();
	pBsMaterial->SetTexture(m_hTexture, nBackBuffer);

	g_BsKernel.SetVertexDeclaration(m_nLineVertexDeclIdx);
	pD3DDevice->SetStreamSource( 0 , m_pVBLine , 0 , sizeof(DROPLINE_VERTEX));
	pBsMaterial->SetVector(m_hConst, &vConst);
	pBsMaterial->CommitChanges();
	pD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, CFcFXScreenWaterDrop::_MAX_QUAD*2 );
	pBsMaterial->EndPass() ;
	pBsMaterial->EndMaterial();
	//*/
	pBsMaterial->BeginMaterial(1, 0);
	pBsMaterial->BeginPass(0);

	pBsMaterial->SetTexture(m_hTexture, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(nBackBuffer));
	pBsMaterial->SetTexture(m_hNormTexture, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(m_nTextureId));

	g_BsKernel.SetVertexDeclaration(m_nDropVertexDeclIdx);
	pD3DDevice->SetStreamSource( 0 , m_pVBDrop , 0 , sizeof(DROP_VERTEX));

	int nDrop = m_Drops.size();
	for( int ii = 0 ; ii < nDrop ; ++ii )
	{
		if( m_Drops[ii].IsActive() )
		{
			vConst = D3DXVECTOR4( m_Drops[ii].fx, m_Drops[ii].fy, m_Drops[ii].fSize*3.f , 0.f);
			pBsMaterial->SetVector(m_hConst, &vConst);
			pBsMaterial->CommitChanges();
			pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2 );
		}
	}
	pBsMaterial->EndPass() ;
	pBsMaterial->EndMaterial();
	//*/
}

int		CFcFXScreenWaterDrop::ProcessMessage(int nCode,DWORD dwParam1/*=0*/, DWORD dwParam2/*=0*/,DWORD dwParam3/*=0*/)
{
	switch(nCode)
	{
	case FX_INIT_OBJECT:
		Initialize();
		return 1;

	case FX_PLAY_OBJECT:
		SetDropGenInfo( (int)dwParam1, dwParam2 ? *(float*)dwParam2 : 0.f);
		m_state = PLAY;
		return 1;

	case FX_STOP_OBJECT:
		SetDropGenInfo(0, 0.f );
		if( (BOOL)dwParam1 == TRUE)
		{
			InactiveAllDrop();
			m_state = READY;
		}
		return 1;

	case FX_SETLIFE:
		m_fLifeTime = *(float*)dwParam1;
		return 1;
	}

	if ( CFcFXBase::ProcessMessage( nCode, dwParam1, dwParam2, dwParam3) )
		return 1;

	return 0;
}

void 	CFcFXScreenWaterDrop::SetDropGenInfo( int nOften, float fMaxSize)
{
	if(fMaxSize > 0.f)
		m_fDropSize = fMaxSize;
	m_nGenOften = nOften;
}


void	CFcFXScreenWaterDrop::GenDrop()
{
	if ( _MAX_DROP*m_nGenOften/100 < GetActiveDropCount() )
		return;

	int nIdx = GetInactiveDropIdx();
	if(nIdx == -1)
		return ;

	float fx = RandomNumberInRange( 0.f,  2.f);
	fx -= 1.f;
	float fy = RandomNumberInRange( 0.5f,  1.f);
	float fSize = RandomNumberInRange( m_fDropSize/10.f, m_fDropSize );


	m_Drops[nIdx].Set(fx, fy, fSize, RandomNumberInRange( 60, 240) );
	m_Drops[nIdx].Active();
}


int CFcFXScreenWaterDrop::GetInactiveDropIdx()
{
	int ii;
	int nRIdx = -1;
	int nDrop = m_Drops.size();
	for( ii = 0 ; ii < nDrop ; ++ii )
	{
		if(m_Drops[ii].IsActive() == FALSE)
		{
			nRIdx = ii;
			break;
		}
	}

	return nRIdx;
}

BOOL CFcFXScreenWaterDrop::IsActiveAnyDrop()
{
	BOOL bReturn = FALSE;

	int ii,nDrop = m_Drops.size();
	for( ii = 0 ; ii < nDrop ; ++ii )
	{
		if( m_Drops[ii].IsActive() )
		{
			bReturn = TRUE;
			break;
		}
	}
	return bReturn;
}

void CFcFXScreenWaterDrop::InactiveAllDrop()
{
	int ii,nDrop = m_Drops.size();
	for( ii = 0 ; ii < nDrop ; ++ii )
		m_Drops[ii].Inactive();
}


int CFcFXScreenWaterDrop::GetActiveDropCount()
{
	int nCount = 0;
	int ii,nDrop = m_Drops.size();
	for( ii = 0 ; ii < nDrop ; ++ii )
	{
		if( m_Drops[ii].IsActive() )
		{
			++nCount;
		}
	}
	return nCount;
}
