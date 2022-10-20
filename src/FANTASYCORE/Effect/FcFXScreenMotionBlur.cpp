#include "stdafx.h"
#include "FcFXScreenMotionBlur.h"
#include "FcFxBase.h"
#include "BsKernel.h"
#include "BsMaterial.h"
#include "BsImageProcess.h"
#include "FcGlobal.h"
#include "FcUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

struct s2DFXVALUE
{
	float _fNoiseIntensity;
	float _fLomoTick;
	float _fSmooth;
	float _fBlendRate;
	float _fSpeed;
	s2DFXVALUE()
	{
		memset( this, 0, sizeof(s2DFXVALUE));
	}
};


CFcFXScreenMotionBlur ::CFcFXScreenMotionBlur()
{
	//CFcFXBase::CFcFXBase(); //aleksger: prefix bug 734: Calling constructor directly does not have any effect.
	m_pVB = NULL;
	m_nMaterialIndex = -1;
	m_fBlendFactor = 1.f;
	m_fScaleFactor = 0.9f;
	m_nNoiseTextureId = -1;
	m_nTableTextureId = -1;

	m_fNoiseIntensity	= 0.7f;
	m_fLomoThick		= 0.6f;
	m_fNoisySmooth		= 1.2f;
	m_fSceneBlendRate	= 4.f;
	m_fSpeed			= 1.0f;
	m_nPriority = c_MAX_PRIORITY;

	m_nVertexDeclIdx = -1;
	m_fRandom[0] = 0.f;
	m_fRandom[1] = 0.f;
	m_vColor = D3DXVECTOR4(1,1,1,1);
#ifndef _LTCG
	SetFxRtti(FX_TYPE_MOTIONBLUR);
#endif //_LTCG
}


CFcFXScreenMotionBlur :: ~CFcFXScreenMotionBlur()
{

}

void CFcFXScreenMotionBlur :: Initialize()
{
	CFcFXBase::Initialize();
}

void CFcFXScreenMotionBlur :: InitDeviceData()
{
	g_BsKernel.chdir("Fx");
	m_nNoiseTextureId = g_BsKernel.LoadTexture("Noise.dds");
	m_nTableTextureId = g_BsKernel.LoadTexture("Table.dds");
	g_BsKernel.chdir("..");

	char fullName[_MAX_PATH];
	strcpy(fullName, g_BsKernel.GetShaderDirectory());
	strcat(fullName, "MotionBlur.fx");
	m_nMaterialIndex = g_BsKernel.LoadMaterial(fullName, FALSE);
	BsAssert( m_nMaterialIndex != -1  && "ScreeenMotionBlur Shader Compile Error!!" );

	CBsMaterial* pBsMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialIndex);
#ifdef _XBOX
	hBackTexture = pBsMaterial->GetParameterByName("backSampler");
	hNoiseTexture= pBsMaterial->GetParameterByName("noiseSampler");
	hTableTexture = pBsMaterial->GetParameterByName("tableSampler");
//	hBlendFactor = pBsMaterial->GetParameterByName("fBlendFactor");
//	hScaleFactor = pBsMaterial->GetParameterByName("fScale");
	hTime		 = pBsMaterial->GetParameterByName("fTime");
	hConst		 = pBsMaterial->GetParameterByName("vConst");
	hConst2		 = pBsMaterial->GetParameterByName("vConst2");
	hColor		 = pBsMaterial->GetParameterByName("vColor");
#else
	hBackTexture = pBsMaterial->GetParameterByName("backTexture");
	hNoiseTexture = pBsMaterial->GetParameterByName("noiseTexture");
	hTableTexture = pBsMaterial->GetParameterByName("tableTexture");
//	hBlendFactor = pBsMaterial->GetParameterByName("fBlendFactor");
//	hScaleFactor = pBsMaterial->GetParameterByName("fScale");
	hTime		 = pBsMaterial->GetParameterByName("fTime");
	hConst		 = pBsMaterial->GetParameterByName("vConst");
	hConst2		 = pBsMaterial->GetParameterByName("vConst2");
	hColor		 = pBsMaterial->GetParameterByName("vColor");
#endif

	D3DVERTEXELEMENT9 decl[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 8, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};
	m_nVertexDeclIdx = g_BsKernel.LoadVertexDeclaration(decl);

	SAFE_RELEASE(m_pVB);

	g_BsKernel.CreateVertexBuffer( 4*sizeof(MOTIONBLUR_VERTEX),
		D3DUSAGE_WRITEONLY, sizeof(MOTIONBLUR_VERTEX), D3DPOOL_MANAGED, &m_pVB);
	MOTIONBLUR_VERTEX *pData;
	m_pVB->Lock(0, 0, (void **)&pData, 0);
	pData[0].vPos = D3DXVECTOR2(-1.f,  1.f);//, 0.1f);
	pData[0].vTex = D3DXVECTOR2( 0.f, 0.f);
	pData[1].vPos = D3DXVECTOR2( 1.f,  1.f);//, 0.1f);
	pData[1].vTex = D3DXVECTOR2( 1.f, 0.f);
	pData[2].vPos = D3DXVECTOR2(-1.f, -1.f);//, 0.1f);
	pData[2].vTex = D3DXVECTOR2( 0.f, 1.f);
	pData[3].vPos = D3DXVECTOR2( 1.f, -1.f);//, 0.1f);
	pData[3].vTex = D3DXVECTOR2( 1.f, 1.f);
	m_pVB->Unlock();
}

void CFcFXScreenMotionBlur :: ReleaseDeviceData()
{
	SAFE_RELEASE_MATERIAL(m_nMaterialIndex);

	SAFE_RELEASE(m_pVB);

	SAFE_RELEASE_VD(m_nVertexDeclIdx);

	SAFE_RELEASE_TEXTURE(m_nNoiseTextureId);
	SAFE_RELEASE_TEXTURE(m_nTableTextureId);
}

void CFcFXScreenMotionBlur :: Process()
{
	float fRumble = (1.f-m_fScaleFactor)*0.5f;
	m_fRandom[0] = RandomNumberInRange( 0.f, fRumble*2.f) - fRumble;
	m_fRandom[1] = RandomNumberInRange( 0.f, fRumble*2.f) - fRumble;
}


void CFcFXScreenMotionBlur :: Render(C3DDevice* pDevice)
{
	if( IsPlayRealMovie() || IsPlayEvent() ) return;

	if(m_state != PLAY)
		return;
	LPDIRECT3DDEVICE9 pD3DDevice = pDevice->GetD3DDevice();

	CBsMaterial* pBsMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialIndex);
	int nBackBuffer = g_BsKernel.GetImageProcess()->GetBackBufferTexture();
	
	pBsMaterial->BeginMaterial(0, 0);
	pBsMaterial->BeginPass(0);
	
	pBsMaterial->SetTexture(hBackTexture, nBackBuffer);

	pBsMaterial->SetTexture(hNoiseTexture, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(m_nNoiseTextureId));
	pBsMaterial->SetTexture(hTableTexture, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(m_nTableTextureId));

	int nTick = GetTick()%60;
	pBsMaterial->SetFloat(hTime, float(nTick)/60.f * m_fSpeed  );

	static D3DXVECTOR4 vConst;
	float fRumble = (1.f-m_fScaleFactor)*0.5f;
	vConst.x = m_fRandom[0];
	vConst.y = m_fRandom[1];
	vConst.z = m_fBlendFactor;
	vConst.w = m_fScaleFactor;
	pBsMaterial->SetVector(hConst, &vConst );

	vConst.x = m_fNoiseIntensity;
	vConst.y = m_fLomoThick;
	vConst.z = m_fNoisySmooth;
	vConst.w = m_fSceneBlendRate;
	pBsMaterial->SetVector(hConst2, &vConst );

	pBsMaterial->SetVector(hColor, &m_vColor );

	pBsMaterial->CommitChanges();

	g_BsKernel.SetVertexDeclaration(m_nVertexDeclIdx);
	pD3DDevice->SetStreamSource( 0 , m_pVB , 0 , sizeof(MOTIONBLUR_VERTEX));	
	pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2 );
	pBsMaterial->EndPass() ;
	pBsMaterial->EndMaterial();
}

int CFcFXScreenMotionBlur :: ProcessMessage(int nCode,DWORD dwParam1, DWORD dwParam2,DWORD dwParam3)
{
	if ( CFcFXBase::ProcessMessage( nCode, dwParam1, dwParam2, dwParam3) )
		return 1;
	switch(nCode)
	{
	case FX_INIT_OBJECT:
		Initialize();
		/*if( dwParam1 )
		{
			s2DFXVALUE tmp = *(s2DFXVALUE*)dwParam1;
			m_fSceneBlendRate = tmp._fBlendRate;
			m_fNoisySmooth = tmp._fSmooth;
			m_fNoiseIntensity = tmp._fNoiseIntensity;
			m_fLomoThick = tmp._fLomoTick;
			m_fSpeed	= tmp._fSpeed;
		}*/
		if( dwParam1 )
		{
			m_vColor = *(D3DXVECTOR4*)dwParam1;
		}
		return 1;

	case FX_PLAY_OBJECT:
		m_state = PLAY;
		return 1;

	case FX_STOP_OBJECT:
		m_state = READY;
		return 1;

	case FX_SETBLUR_INTENCITY:
		m_fBlendFactor = *(float*)dwParam1;
		return 1;
	}
	
	return  0;
}




CFcFXXMB::CFcFXXMB()
{
	//CFcFXBase::CFcFXBase();  //aleksger: prefix bug 736: Calling constructor directly does not have any effect.
	m_pVB = NULL;
	m_nMaterialIndex = -1;

	hBackTexture = NULL;
	hTime = NULL;
	m_nPriority = c_MAX_PRIORITY;
	m_nVertexDeclIdx = -1;
#ifndef _LTCG
	SetFxRtti(FX_TYPE_XM);
#endif //_LTCG
}


CFcFXXMB::~CFcFXXMB()
{

}


void CFcFXXMB::InitDeviceData()
{
	char fullName[_MAX_PATH];
	strcpy(fullName, g_BsKernel.GetShaderDirectory());
	strcat(fullName, "XMB.fx");
	m_nMaterialIndex = g_BsKernel.LoadMaterial(fullName, FALSE);
	BsAssert( m_nMaterialIndex != -1  && "ScreeenMotionBlur Shader Compile Error!!" );

	CBsMaterial* pBsMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialIndex);
#ifdef _XBOX
	hBackTexture = pBsMaterial->GetParameterByName("SceneSampler");
	hTime = pBsMaterial->GetParameterByName("TIME");
#else
	hBackTexture = pBsMaterial->GetParameterByName("SceneMap");
	hTime = pBsMaterial->GetParameterByName("fTime");
#endif

	D3DVERTEXELEMENT9 decl[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		D3DDECL_END()
	};
	m_nVertexDeclIdx = g_BsKernel.LoadVertexDeclaration(decl);

	SAFE_RELEASE(m_pVB);

	g_BsKernel.CreateVertexBuffer( 4*sizeof(D3DXVECTOR2),
		D3DUSAGE_WRITEONLY, sizeof(D3DXVECTOR2), D3DPOOL_MANAGED, &m_pVB);
	D3DXVECTOR2 *pData;
	m_pVB->Lock(0, 0, (void **)&pData, 0);
	pData[0] = D3DXVECTOR2(-1.f,  1.f);//, 0.1f);
	pData[1] = D3DXVECTOR2( 1.f,  1.f);//, 0.1f);
	pData[2] = D3DXVECTOR2(-1.f, -1.f);//, 0.1f);
	pData[3] = D3DXVECTOR2( 1.f, -1.f);//, 0.1f);
	m_pVB->Unlock();
}


void CFcFXXMB::ReleaseDeviceData()
{
	SAFE_RELEASE_MATERIAL(m_nMaterialIndex);

	SAFE_RELEASE(m_pVB);
	
	SAFE_RELEASE_VD(m_nVertexDeclIdx);
}


void CFcFXXMB::Process()
{
}

void CFcFXXMB::Render(C3DDevice *pDevice)
{
	if( IsPlayRealMovie() || IsPlayEvent() ) {
		return;
	}

	if(m_state != PLAY)
		return;
	LPDIRECT3DDEVICE9 pD3DDevice = pDevice->GetD3DDevice();

	CBsMaterial* pBsMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialIndex);
	int nBackBuffer = g_BsKernel.GetImageProcess()->GetBackBufferTexture();

	pBsMaterial->BeginMaterial(0, 0);
	pBsMaterial->BeginPass(0);

	pBsMaterial->SetTexture(hBackTexture, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(nBackBuffer));

	int nTick = GetTick();
	pBsMaterial->SetFloat(hTime, float(nTick)/20.f  );
/*
	int nTick = ::GetProcessTick()%60;
	pBsMaterial->SetFloat(hTime, float(nTick)/60.f  );

	static D3DXVECTOR4 vConst;
	float fRumble = (1.f-m_fScaleFactor)*0.5f;
	vConst.x = RandomNumberInRange( 0.f, fRumble*2.f) - fRumble;
	vConst.y = RandomNumberInRange( 0.f, fRumble*2.f) - fRumble;
	vConst.z = m_fBlendFactor;
	vConst.w = m_fScaleFactor;
	pBsMaterial->SetVector(hConst, &vConst );
*/
	pBsMaterial->CommitChanges();

	g_BsKernel.SetVertexDeclaration(m_nVertexDeclIdx);
	pD3DDevice->SetStreamSource( 0 , m_pVB , 0 , sizeof(D3DXVECTOR2));	
	pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2 );
	pBsMaterial->EndPass() ;
	pBsMaterial->EndMaterial();
}

int  CFcFXXMB::ProcessMessage(int nCode,DWORD dwParam1, DWORD dwParam2,DWORD dwParam3)
{
	switch(nCode)
	{
	case FX_INIT_OBJECT:
		Initialize();
		return 1;

	case FX_PLAY_OBJECT:
		m_state = PLAY;
		return 1;

	case FX_STOP_OBJECT:
		m_state = READY;
		return 1;

	case FX_SETBLUR_INTENCITY:
//		m_fBlendFactor = *(float*)dwParam1;
		return 1;
	}
	if ( CFcFXBase::ProcessMessage( nCode, dwParam1, dwParam2, dwParam3) )
		return 1;
	return  0;
}




void CFcFXXMB::Initialize()
{
	CFcFXBase::Initialize();
}











CFcFXLomoFilter::CFcFXLomoFilter()
{
	//CFcFXBase::CFcFXBase();  //aleksger: prefix bug 738: Calling constructor directly does not have any effect.
	m_pVB = NULL;
	m_nMaterialIndex = -1;
	m_nTextureID = -1;
	m_nPriority = c_MAX_PRIORITY;


	m_nNoiseTextureId = -1;

	m_fNoiseIntensity	= 0.6f;
	m_fLomoThick		= 0.7f;
	m_fNoisySmooth		= 6.f;
	m_fSceneBlendRate	= 0.5f;


	m_fFactor = 0.f;
	m_nVertexDeclIdx = -1;
#ifndef _LTCG
	SetFxRtti(FX_TYPE_LOMOFILTER);
#endif //_LTCG
}


void CFcFXLomoFilter::InitDeviceData()
{
	g_BsKernel.chdir("Fx");
	m_nNoiseTextureId = g_BsKernel.LoadTexture("Noise.dds");
//	m_nTextureID = g_BsKernel.LoadTexture("Table3.dds");
	g_BsKernel.chdir("..");

	char fullName[_MAX_PATH];
	strcpy(fullName, g_BsKernel.GetShaderDirectory());
	strcat(fullName, "LomoFilter.fx");
	m_nMaterialIndex = g_BsKernel.LoadMaterial(fullName, FALSE);
	BsAssert( m_nMaterialIndex != -1  && "LomoFilter Shader Compile Error!!" );

	CBsMaterial* pBsMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialIndex);
#ifdef _XBOX
//	hTableTexture = pBsMaterial->GetParameterByName("tableSampler");
	hNoiseTexture = pBsMaterial->GetParameterByName("noiseSampler");
	hTime		 = pBsMaterial->GetParameterByName("fTime");
	hConst		 = pBsMaterial->GetParameterByName("vConst");
	hRandom		 = pBsMaterial->GetParameterByName("fRandom");

#else
//	hTableTexture = pBsMaterial->GetParameterByName("tableTexture");
	hNoiseTexture = pBsMaterial->GetParameterByName("noiseTexture");
	hTime		 = pBsMaterial->GetParameterByName("fTime");
	hConst		 = pBsMaterial->GetParameterByName("vConst");
	hRandom		 = pBsMaterial->GetParameterByName("fRandom");
#endif

	D3DVERTEXELEMENT9 decl[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 8, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};

	m_nVertexDeclIdx = g_BsKernel.LoadVertexDeclaration(decl);

	SAFE_RELEASE(m_pVB);

	g_BsKernel.CreateVertexBuffer( 4*sizeof(MOTIONBLUR_VERTEX),
		D3DUSAGE_WRITEONLY, sizeof(MOTIONBLUR_VERTEX), D3DPOOL_MANAGED, &m_pVB);
	MOTIONBLUR_VERTEX *pData;
	m_pVB->Lock(0, 0, (void **)&pData, 0);
	pData[0].vPos = D3DXVECTOR2(-1.f,  1.f);//, 0.1f);
	pData[0].vTex = D3DXVECTOR2( 0.f, 0.f);
	pData[1].vPos = D3DXVECTOR2( 1.f,  1.f);//, 0.1f);
	pData[1].vTex = D3DXVECTOR2( 1.f, 0.f);
	pData[2].vPos = D3DXVECTOR2(-1.f, -1.f);//, 0.1f);
	pData[2].vTex = D3DXVECTOR2( 0.f, 1.f);
	pData[3].vPos = D3DXVECTOR2( 1.f, -1.f);//, 0.1f);
	pData[3].vTex = D3DXVECTOR2( 1.f, 1.f);
	m_pVB->Unlock();
}

void CFcFXLomoFilter::ReleaseDeviceData()
{
	SAFE_RELEASE_MATERIAL(m_nMaterialIndex);

	SAFE_RELEASE(m_pVB);

	SAFE_RELEASE_VD(m_nVertexDeclIdx);

	SAFE_RELEASE_TEXTURE(m_nNoiseTextureId);

	SAFE_RELEASE_TEXTURE(m_nTextureID);
}

void CFcFXLomoFilter::Process()
{
}

void CFcFXLomoFilter::Render(C3DDevice *pDevice)
{
	if( IsPlayRealMovie() || IsPlayEvent() ) {
		return;
	}

	if(m_state != PLAY)
		return;
	LPDIRECT3DDEVICE9 pD3DDevice = pDevice->GetD3DDevice();

	CBsMaterial* pBsMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialIndex);

	pBsMaterial->BeginMaterial(0, 0);
	pBsMaterial->BeginPass(0);

//	pBsMaterial->SetTexture(hTableTexture, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(m_nTextureID));
	pBsMaterial->SetTexture(hNoiseTexture, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(m_nNoiseTextureId));

	int nTick = GetTick();//%120;
	pBsMaterial->SetFloat(hTime, float(nTick)/120.f );
	float fRand = cos( float(nTick)/0.5f)*0.2f+1.f;
	pBsMaterial->SetFloat(hRandom, fRand );


	D3DXVECTOR4 vConst;

	vConst.x = m_fNoiseIntensity;
	vConst.y = m_fLomoThick * m_fFactor;
	vConst.z = m_fNoisySmooth;
	vConst.w = m_fSceneBlendRate;
	pBsMaterial->SetVector(hConst, &vConst );

	pBsMaterial->CommitChanges();
	
	g_BsKernel.SetVertexDeclaration(m_nVertexDeclIdx);
	pD3DDevice->SetStreamSource( 0 , m_pVB , 0 , sizeof(MOTIONBLUR_VERTEX));	
	pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2 );
	pBsMaterial->EndPass() ;
	pBsMaterial->EndMaterial();
}

int CFcFXLomoFilter::ProcessMessage(int nCode,DWORD dwParam1 , DWORD dwParam2 ,DWORD dwParam3 )
{
	if ( CFcFXBase::ProcessMessage( nCode, dwParam1, dwParam2, dwParam3) )
		return 1;
	switch(nCode)
	{
	case FX_INIT_OBJECT:
		Initialize();
		/*if( dwParam1 )
		{
			s2DFXVALUE tmp = *(s2DFXVALUE*)dwParam1;
			m_fSceneBlendRate = tmp._fBlendRate;
			m_fNoisySmooth = tmp._fSmooth;
			m_fNoiseIntensity = tmp._fNoiseIntensity;
			m_fLomoThick = tmp._fLomoTick;
		}*/
		return 1;

	case FX_PLAY_OBJECT:
		m_state = PLAY;
		return 1;

	case FX_STOP_OBJECT:
		m_state = READY;
		return 1;

	case FX_SET_LOMOINTENSITY:
		m_fFactor = *(float*)dwParam1;
		return 1;
	}

	return  0;
}




//-------------------------------------------------------------------------------------------------

CFcFXInppyyBlur::CFcFXInppyyBlur()
{
	m_nPriority = c_MAX_PRIORITY;
	m_bCustom2 = false;
#ifndef _LTCG
	SetFxRtti(FX_TYPE_INPHYMOTIONBLUR);
#endif //_LTCG
}


CFcFXInppyyBlur::~CFcFXInppyyBlur()
{
	if(g_BsKernel.GetImageProcess())
		g_BsKernel.GetImageProcess()->SetExtraFilter(FILTER_NONE);
}


void CFcFXInppyyBlur::Initialize()
{
	CFcFXBase::Initialize();
}





void CFcFXInppyyBlur::Process()
{
}

void CFcFXInppyyBlur::Render(C3DDevice *pDevice)
{
	if( IsPlayRealMovie() || IsPlayEvent() ) {
		m_state = READY;
		if(g_BsKernel.GetImageProcess())
			g_BsKernel.GetImageProcess()->SetExtraFilter(FILTER_NONE);
	}
	
}

int  CFcFXInppyyBlur::ProcessMessage(int nCode,DWORD dwParam1, DWORD dwParam2,DWORD dwParam3)
{
	switch(nCode)
	{
	case FX_INIT_OBJECT:
		Initialize();
		if( dwParam1 ) m_bCustom2 = true;
		return 1;

	case FX_PLAY_OBJECT:
		m_state = PLAY;
		if( !m_bCustom2 && g_BsKernel.GetImageProcess())
			g_BsKernel.GetImageProcess()->SetExtraFilter(FILTER_CUSTOMBLUR1);
		else if( m_bCustom2 && g_BsKernel.GetImageProcess() )
			g_BsKernel.GetImageProcess()->SetExtraFilter(FILTER_CUSTOMBLUR2);
		return 1;

	case FX_STOP_OBJECT:
		m_state = READY;
		if(g_BsKernel.GetImageProcess())
			g_BsKernel.GetImageProcess()->SetExtraFilter(FILTER_NONE);
		return 1;

	case FX_SETBLUR_INTENCITY:
		//		m_fBlendFactor = *(float*)dwParam1;
		return 1;

	case FX_DELETE_OBJECT:
#ifndef _LTCG
		if( dwParam1 )
		{
			BsAssert( GetFxRtti() == (int)dwParam1 );
		}
#endif //_LTCG
		if(g_BsKernel.GetImageProcess())
			g_BsKernel.GetImageProcess()->SetExtraFilter(FILTER_NONE);
		m_state=END;
		m_DeviceDataState=FXDS_RELEASEREADY;
		return 1;
	}
	if ( CFcFXBase::ProcessMessage( nCode, dwParam1, dwParam2, dwParam3) )
		return 1;
	return  0;
}





CFcFX2DFSEffect::CFcFX2DFSEffect()
{
	//CFcFXBase::CFcFXBase();  //aleksger: prefix bug 741: Calling constructor directly does not have any effect.
	m_nPriority = c_MAX_PRIORITY-1;


	m_pVB = NULL;
	m_nMaterialIndex = -1;
	m_nTextureID = -1;
	
	m_fAlpha = 1.f;
	m_nDevide = 4;
	m_nInterval = 1;

	m_dwStartTick = 0;
	m_nVertexDeclIdx = -1;
	m_nTechnique = 0;
	SetEnvir2DEffect(false);
#ifndef _LTCG
	SetFxRtti(FX_TYPE_2DFSEFFECT);
#endif //_LTCG
}



void CFcFX2DFSEffect::InitDeviceData()
{
	char fullName[_MAX_PATH];
	strcpy(fullName, g_BsKernel.GetShaderDirectory());
	strcat(fullName, "2DFSEffect.fx");
	m_nMaterialIndex = g_BsKernel.LoadMaterial(fullName, FALSE);
	BsAssert( m_nMaterialIndex != -1  && "2DFSEffect Shader Compile Error!!" );

	CBsMaterial* pBsMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialIndex);
#ifdef _XBOX
	//	hTableTexture = pBsMaterial->GetParameterByName("tableSampler");
	hTexture = pBsMaterial->GetParameterByName("diffuseSampler");
	hFactor	 = pBsMaterial->GetParameterByName("vFactor");
#else
	//	hTableTexture = pBsMaterial->GetParameterByName("tableTexture");
	hTexture = pBsMaterial->GetParameterByName("diffuseTexture");
	hFactor	 = pBsMaterial->GetParameterByName("vFactor");
#endif

	D3DVERTEXELEMENT9 decl[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 8, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};
	m_nVertexDeclIdx = g_BsKernel.LoadVertexDeclaration(decl);

	SAFE_RELEASE(m_pVB);
	
	g_BsKernel.CreateVertexBuffer( 4*sizeof(MOTIONBLUR_VERTEX),
		D3DUSAGE_WRITEONLY, sizeof(MOTIONBLUR_VERTEX), D3DPOOL_MANAGED, &m_pVB);
	MOTIONBLUR_VERTEX *pData;
	m_pVB->Lock(0, 0, (void **)&pData, 0);
	pData[0].vPos = D3DXVECTOR2(-1.f,  1.f);//, 0.1f);
	pData[0].vTex = D3DXVECTOR2( 0.f, 0.f);
	pData[1].vPos = D3DXVECTOR2( 1.f,  1.f);//, 0.1f);
	pData[1].vTex = D3DXVECTOR2( 1.f, 0.f);
	pData[2].vPos = D3DXVECTOR2(-1.f, -1.f);//, 0.1f);
	pData[2].vTex = D3DXVECTOR2( 0.f, 1.f);
	pData[3].vPos = D3DXVECTOR2( 1.f, -1.f);//, 0.1f);
	pData[3].vTex = D3DXVECTOR2( 1.f, 1.f);
	m_pVB->Unlock();
}

void CFcFX2DFSEffect::ReleaseDeviceData()
{
	SAFE_RELEASE_MATERIAL(m_nMaterialIndex);

	SAFE_RELEASE(m_pVB);

	SAFE_RELEASE_VD(m_nVertexDeclIdx);
}

void CFcFX2DFSEffect::Process()
{
}

void CFcFX2DFSEffect::Render(C3DDevice *pDevice)
{
	if( !GetEnvir2DEffect() && (IsPlayEvent() || IsPlayRealMovie() ) ) return;
	if(m_state != PLAY)
		return;
	LPDIRECT3DDEVICE9 pD3DDevice = pDevice->GetD3DDevice();

	CBsMaterial* pBsMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialIndex);

	pBsMaterial->BeginMaterial(m_nTechnique, 0);
	pBsMaterial->BeginPass(0);

	if( m_nTextureID != -1 )//2D 화면 이펙트 중에 행업되는 경우가 있습니다.
        pBsMaterial->SetTexture(hTexture, m_nTextureID);

	int nTick =( GetTick()-m_dwStartTick)/m_nInterval;
	nTick = nTick%( m_nDevide * m_nDevide );

	D3DXVECTOR4 vConst;
	vConst.x = float(nTick);
	vConst.y = m_fAlpha;//*(fSin*0.5f+0.5f);
	vConst.z = float(m_nDevide);
	vConst.w = 1.f/float(m_nDevide);
	pBsMaterial->SetVector(hFactor, &vConst );

	g_BsKernel.SetVertexDeclaration(m_nVertexDeclIdx);
	pD3DDevice->SetStreamSource( 0 , m_pVB , 0 , sizeof(MOTIONBLUR_VERTEX));	
	pBsMaterial->CommitChanges();
	pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2 );
	pBsMaterial->EndPass() ;
	pBsMaterial->EndMaterial();
}

int CFcFX2DFSEffect::ProcessMessage(int nCode,DWORD dwParam1 , DWORD dwParam2 ,DWORD dwParam3 )
{
	if ( CFcFXBase::ProcessMessage( nCode, dwParam1, dwParam2, dwParam3) )
		return 1;
	switch(nCode)
	{
	case FX_INIT_OBJECT:
		Initialize();
		m_nTechnique = dwParam1;
		return 1;

	case FX_PLAY_OBJECT:
		m_state = PLAY;
		m_dwStartTick = GetTick();
		if(dwParam1) SetEnvir2DEffect(true);
		return 1;

	case FX_STOP_OBJECT:
		m_state = READY;
		return 1;

	case FX_SET_TEXTURE:
		m_nTextureID = dwParam1;
		return 1;

	case FX_SET_FSALPHA:
		m_fAlpha = *(float*)dwParam1;
		return 1;

	case FX_SET_FSINTERVAL:
		m_nInterval = dwParam1;
		return 1;
	case FX_SET_FSDEVIDE:
		m_nDevide = dwParam1;
		return 1;
	}

	return  0;
}