#include "stdafx.h"
#include "FcFxBase.h"
#include "FcFxCastEarth.h"
#include "BsKernel.h"
#include "BsMaterial.h"
#include "FcUtil.h"
#include "FcGlobal.h"

//#include "FcWorld.h"
//#include "FcHeroObject.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

/*---------------------------------------------------------------------------------
-
-			CFloatStone2::CFloatStone2()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
CFloatStone2::CFloatStone2()
{
	m_nMaterialId		= -1;
	m_nTexture			= -1;

	m_pVB[0]			= NULL;
	m_pVB[1]			= NULL;
	m_pVertexDecl		= NULL;
	m_hWorldViewProj	= NULL;
	m_hInterval			= NULL;
	m_hCenterPos		= NULL;
	m_hInhalePos		= NULL;
	m_hAccmulatePos		= NULL;
	m_hLerpValue		= NULL;
	m_hTexture			= NULL;
	
	Reset();
}





/*---------------------------------------------------------------------------------
-
-			CFloatStone2::Reset()
-					; 초기화.
-					  
-
---------------------------------------------------------------------------------*/
void CFloatStone2::Reset()
{
	m_fAccmulatePos[0]		= -100.0f;
	m_fAccmulatePos[1]		= -100.0f;
	m_fIntervalTime		= -1;
	m_fLerpValue		= 0.0f;
	memset( &m_vCenterPos, 0, sizeof(m_vCenterPos) );
	memset( &m_vInhalePos, 0, sizeof(m_vInhalePos) );

	SetInNumOfStone(-1);
	SetOutNumOfStone(-1);
	SetFloatState(STONE_READY);

#ifndef _LTCG
	SetFxRtti(FX_TYPE_CASTEARTH);
#endif //_LTCG
	
}





/*---------------------------------------------------------------------------------
-
-			CFloatStone2::~CFloatStone2()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
CFloatStone2::~CFloatStone2()
{
	SAFE_RELEASE(m_pVB[0]);
	SAFE_RELEASE(m_pVB[1]);
	SAFE_RELEASE_TEXTURE(m_nTexture);
	SAFE_RELEASE_MATERIAL(m_nMaterialId);
	Finalize();
}





/*---------------------------------------------------------------------------------
-
-			CFloatStone2::Initialize()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
void CFloatStone2::Initialize()
{
	g_BsKernel.chdir("Fx");
	m_nTexture = g_BsKernel.LoadVolumeTexture("stones16_N.dds"); 
	g_BsKernel.chdir("..");
	
	m_sEarthCast._nInNum		= 50;
	m_sEarthCast._fInRadius		= 500.0f;
	m_sEarthCast._fUpTime		= 30.0f;
	m_sEarthCast._fInUpSpd		= 15.0f;
	m_sEarthCast._fFloatTime	= 10.0f;
	m_sEarthCast._nInHeightMax	= 10;
	m_sEarthCast._nInHeightMin	= -50;
	m_sEarthCast._fInSizeMax	= 20.0f;
	m_sEarthCast._fInSizeMin	= 4.0f;
	m_sEarthCast._nOutNum		= 100;
	m_sEarthCast._fOutRadius	= 5000.0f;
	m_sEarthCast._fOutUpSpd		= 18.0f;
	m_sEarthCast._nOutHeightMax = 10;
	m_sEarthCast._nOutHeightMin = -50;
	m_sEarthCast._fOutSizeMax	= 20.0f;
	m_sEarthCast._fOutSizeMin	= 4.0f;

	SetInRadius(m_sEarthCast._fInRadius);
	SetOutRadius(m_sEarthCast._fOutRadius);

	SetInNumOfStone(m_sEarthCast._nInNum);
	SetOutNumOfStone(m_sEarthCast._nOutNum);
	
	InitDeviceData();
}






/*---------------------------------------------------------------------------------
-
-			CFloatStone2::InitDeviceData()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
void CFloatStone2::InitDeviceData()
{
	CreateVB();

	// Material Setting!!
	char fullName[_MAX_PATH];
	strcpy(fullName, g_BsKernel.GetShaderDirectory());
	strcat(fullName, "floatStone.fx");

	m_nMaterialId = g_BsKernel.LoadMaterial(fullName, FALSE);
	BsAssert( m_nMaterialId != -1  && "Shader Compile Error!!" );

	CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialId);

#ifdef _XBOX
	m_hWorldViewProj		= pMaterial->GetParameterByName("WorldViewProjection");
	m_hTexture				= pMaterial->GetParameterByName("SpriteSampler");
#else
	m_hWorldViewProj		= pMaterial->GetParameterByName("worldViewProj");
	m_hTexture				= pMaterial->GetParameterByName("diffuseTexture");
#endif

	m_hCenterPos			= pMaterial->GetParameterByName("vCenterPos");
	m_hCameraPos			= pMaterial->GetParameterByName("vCameraPos");
	m_hInhalePos			= pMaterial->GetParameterByName("vInhalePos");
	m_hInterval				= pMaterial->GetParameterByName("Interval");
	m_hAccmulatePos			= pMaterial->GetParameterByName("AccmulatePos");	
	m_hLerpValue			= pMaterial->GetParameterByName("lerpvalue");

	D3DVERTEXELEMENT9 decl[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 } ,
		{ 0, 16, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 } ,
		D3DDECL_END()
	};

	g_BsKernel.GetDevice()->CreateVertexDeclaration( decl, &m_pVertexDecl );

	m_DeviceDataState = FXDS_INITREADY;
}


/*---------------------------------------------------------------------------------
-
-			CFloatStone2::Finalize()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
void CFloatStone2::Finalize()
{
	ReleaseDeviceData();
	Reset();
}






/*---------------------------------------------------------------------------------
-
-			CFloatStone2::ReleaseDeviceData()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
void CFloatStone2::ReleaseDeviceData()
{
	/*SAFE_RELEASE(m_pVB[0]);
	SAFE_RELEASE(m_pVB[1]);
	SAFE_RELEASE_TEXTURE(m_nTexture);
	SAFE_RELEASE_MATERIAL(m_nMaterialId);*/
	m_DeviceDataState=FXDS_RELEASEREADY;
}





/*---------------------------------------------------------------------------------
-
-			CFloatStone2::CreateVB()
-					; 버텍스 버퍼를 구성한다.
-					  
-
---------------------------------------------------------------------------------*/
void CFloatStone2::CreateVB()
{
	// 안쪽 돌맹이 
	g_BsKernel.CreateVertexBuffer( GetInNumOfStone() * GetInNumOfStone() *sizeof(CUSTOMPOINTSPRITE), 
		D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMPOINTSPRITE, D3DPOOL_MANAGED, &m_pVB[0]);
	{
		CUSTOMPOINTSPRITE	*pV;
		m_pVB[0]->Lock( 0, sizeof(CUSTOMPOINTSPRITE) * GetInNumOfStone() * GetInNumOfStone()  , (VOID**)&pV, 0 );

		D3DXVECTOR4 pVecX(1,0,0,0);
		D3DXVECTOR4 pVecY(0,0,1,0);
		float epsil = GetInRadius() / ( (float)GetInNumOfStone() / 2.0f );
		for( int i = 0 ; i < GetInNumOfStone() ; ++i ) {
			for( int j = 0 ; j < GetInNumOfStone() ; ++j ) {
				int k = i * GetInNumOfStone() + j; 
				pV[k].Pos = GetCenterPos() + ( (float)(GetInNumOfStone()/2 - i) * pVecX * epsil ) + ( (float)(GetInNumOfStone()/2 - j) * pVecY * epsil );
				pV[k].Pos.y += RandomNumberInRange(m_sEarthCast._nInHeightMin, m_sEarthCast._nInHeightMax) * 10.0f;
				pV[k].Pos.w = 1.0f; 
				pV[k].normal.x = (float)RandomNumberInRange( 1 , 16 )/16.0f - 0.032f;				// Text depth
				pV[k].normal.y = RandomNumberInRange( m_sEarthCast._fInSizeMin, m_sEarthCast._fInSizeMax );									// size
			}
		}
		m_pVB[0]->Unlock();
	}

	
	// 바깥쪽 돌맹이
	g_BsKernel.CreateVertexBuffer( GetOutNumOfStone() * GetOutNumOfStone() *sizeof(CUSTOMPOINTSPRITE), 
		D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMPOINTSPRITE, D3DPOOL_MANAGED, &m_pVB[1]);

	{
		CUSTOMPOINTSPRITE	*pV;
		m_pVB[1]->Lock( 0, sizeof(CUSTOMPOINTSPRITE) * GetInNumOfStone() * GetInNumOfStone()  , (VOID**)&pV, 0 );

		D3DXVECTOR4 pVecX(1,0,0,0);
		D3DXVECTOR4 pVecY(0,0,1,0);
		float epsil = GetOutRadius() / ( (float)GetOutNumOfStone() / 2.0f );
		for( int i = 0 ; i < GetOutNumOfStone() ; ++i ) {
			for( int j = 0 ; j < GetOutNumOfStone() ; ++j ) {
				int k = i * GetOutNumOfStone() + j; 
				pV[k].Pos = GetCenterPos() + ( (float)(GetOutNumOfStone()/2 - i) * pVecX * epsil ) + ( (float)(GetOutNumOfStone()/2 - j) * pVecY * epsil );
				pV[k].Pos.y += RandomNumberInRange(m_sEarthCast._nOutHeightMin, m_sEarthCast._nOutHeightMax) * 10.0f;
				pV[k].Pos.w = 1.0f; 
				pV[k].normal.x = (float)RandomNumberInRange( 1 , 16 )/16.0f - 0.032f;				// Text depth
				pV[k].normal.y = RandomNumberInRange( m_sEarthCast._fOutSizeMin, m_sEarthCast._fOutSizeMax );									// size
			}
		}
		m_pVB[1]->Unlock();
	}
}



/*---------------------------------------------------------------------------------
--
-			CFloatStone2::Process()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
void CFloatStone2::Process()
{
	if( m_state == END ) return;
	if( GetFloatState() == STONE_READY ) return;
	/* STATE : STONE_UP, STONE_FLOAT, STONE_HALE, STONE_RAIN, STONE_DOWN, STONE_END */
	switch( GetFloatState() )
	{
	case STONE_UP:		//*******************************************
		{
			m_sEarthCast._fInUpSpd	-= 0.5f;
			m_sEarthCast._fOutUpSpd -= 0.5f;
			if( m_sEarthCast._fInUpSpd	<= 1.0f ) m_sEarthCast._fInUpSpd	= 1.0f;
			if( m_sEarthCast._fOutUpSpd <= 5.0f ) m_sEarthCast._fOutUpSpd	= 5.0f;
			m_fAccmulatePos[0] += m_sEarthCast._fInUpSpd;
			m_fAccmulatePos[1] += m_sEarthCast._fOutUpSpd;
			m_fIntervalTime++;
			if( m_fIntervalTime >= m_sEarthCast._fUpTime ) 
			{
				m_fIntervalTime = 0;
				SetFloatState(STONE_FLOAT);
			}
		}
		break;
	case STONE_FLOAT:	//*******************************************
		{
			m_fAccmulatePos[0] += 1.1f;//sin((float)m_fIntervalTime*0.3f) * 1.2f;
			m_fAccmulatePos[1] += 5.1f;//sin((float)m_fIntervalTime*0.3f) * 1.2f;

			m_fIntervalTime++;
			if( m_fIntervalTime >= m_sEarthCast._fFloatTime ) 
			{
				m_fIntervalTime = 0;
				SetFloatState(STONE_HALE);
			}
		}
		break;
	case STONE_HALE:	//*******************************************
		{
			
			m_fIntervalTime++;
			m_fLerpValue += m_fIntervalTime*m_fIntervalTime*0.01f;
			m_fLerpValue /= 4.0f;
			if( m_fLerpValue >= 1.0f ) m_fLerpValue = 1.0f;
			if( m_fIntervalTime >= 20 ) 
			{
				m_fIntervalTime = 0;
				m_fLerpValue = 0;
				SetFloatState(STONE_END);
			}
		}
		break;
	case STONE_RAIN:	//*******************************************
		{
			SetFloatState(STONE_END);
		}
		break;
	case STONE_END:		//*******************************************
		{
			// 물어보고 결정하자. 리소스를 완전히 지워버릴것인가? 아니면 리소스를 그냥 초기화만 해 버릴 것 인가?
		}
		break;
	}
}






/*---------------------------------------------------------------------------------
-
-			CFloatStone2::PreRender()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
void CFloatStone2::PreRender()
{
	CFcFXBase::PreRender();
}






/*---------------------------------------------------------------------------------
-
-			CFloatStone2::Render()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
void CFloatStone2::Render(C3DDevice *pDevice)
{
	if( GetFloatState() == STONE_READY || GetFloatState() == STONE_END ) return;

	D3DXMATRIX matWVP;
	D3DXMATRIX matView;
	D3DXMATRIX matProj;

	CBsMaterial*	pBsMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialId);

	matView = *g_BsKernel.GetParamViewMatrix();
	matProj = *g_BsKernel.GetParamProjectionMatrix();
	matWVP	= matView * matProj;

	CBsCamera* pCamera = g_BsKernel.GetActiveCamera();
	D3DXVECTOR4 vCamPos(pCamera->GetObjectMatrix()->_41, pCamera->GetObjectMatrix()->_42, pCamera->GetObjectMatrix()->_43,1.0f);

	pDevice->SetRenderState( D3DRS_POINTSPRITEENABLE, TRUE );
	pDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
	
	pBsMaterial->SetMatrix(m_hWorldViewProj, &matWVP);
	pBsMaterial->SetTexture(m_hTexture, (LPDIRECT3DVOLUMETEXTURE9)(g_BsKernel.GetTexturePtr(m_nTexture)) );//LPDIRECT3DBASETEXTURE9
	pBsMaterial->SetFloat(m_hInterval, m_fIntervalTime);
	pBsMaterial->SetFloat(m_hLerpValue, m_fLerpValue);
	pBsMaterial->SetVector(m_hInhalePos, &m_vInhalePos);
	pBsMaterial->SetVector(m_hCameraPos, &vCamPos);

	/* STATE : STONE_UP, STONE_FLOAT, STONE_HALE, STONE_RAIN, STONE_DOWN, STONE_END */
	for(int i = 0 ; i < 2 ; i++ )
	{
		switch( GetFloatState() )
		{
		case STONE_UP:		//******************************************* techn 0
		case STONE_FLOAT:
			{
				pBsMaterial->BeginMaterial(0, 0);
				pBsMaterial->BeginPass(0);
				pBsMaterial->SetFloat(m_hAccmulatePos, m_fAccmulatePos[i]);
			}
			break;
		case STONE_HALE:	//******************************************* techn 1
			{
				pBsMaterial->BeginMaterial(1, 0);
				pBsMaterial->BeginPass(0);
				pBsMaterial->SetFloat(m_hAccmulatePos, m_fAccmulatePos[i]);
			}
			break;
		}

		pDevice->SetVertexDeclaration(m_pVertexDecl);

		pDevice->SetStreamSource( 0 , m_pVB[i] , 0 , sizeof(CUSTOMPOINTSPRITE));

		pBsMaterial->CommitChanges();

		if( i == 0 ) pDevice->DrawPrimitiveVB(D3DPT_POINTLIST, 0, GetInNumOfStone() * GetInNumOfStone() );
		else if( i == 1 ) pDevice->DrawPrimitiveVB(D3DPT_POINTLIST, 0, GetOutNumOfStone() * GetOutNumOfStone() );

		pBsMaterial->EndPass();
		pBsMaterial->EndMaterial();
	}
		

	pDevice->SetRenderState( D3DRS_POINTSPRITEENABLE, FALSE );
	pDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
}



/*---------------------------------------------------------------------------------
-
-			CFloatStone2::ProcessMessage()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
int CFloatStone2::ProcessMessage(int nCode,DWORD dwParam1/*=0*/, DWORD dwParam2/*=0*/,DWORD dwParam3/*=0*/)
{
	switch(nCode)
	{
	case FX_INIT_OBJECT:
		{
			SetFloatState(STONE_READY);

			D3DXVECTOR4 tmpPos;
			tmpPos = *(D3DXVECTOR4*)dwParam1;
			SetCenterPos(tmpPos);
			tmpPos = *(D3DXVECTOR4*)dwParam2;
			SetInhalePos(tmpPos);
			Initialize();

			
		}
		return 1;
	case FX_PLAY_OBJECT:
		{
			SetFloatState(STONE_UP);
			m_state = PLAY;
		}
		return 1;
	case FX_DELETE_OBJECT:
		{
#ifndef _LTCG
			if( dwParam1 )
			{
				BsAssert( GetFxRtti() == (int)dwParam1 );
			}
#endif //_LTCG
			Finalize();
			m_state = END;
		}
		return 1;
	
	}

	if( CFcFXBase::ProcessMessage( nCode, dwParam1, dwParam2, dwParam3) )
		return 1;

	return 0;
}
//***********************************************************************************************
//***********************************************************************************************

/*
사용법
D3DXVECTOR3 CharPos = m_hParent->GetPos();
D3DXVECTOR4 CenterPos(CharPos.x, CharPos.y, CharPos.z, 5.0f );
g_pFcFXManager->SendMessage(m_nCastFXId, FX_INIT_OBJECT, (DWORD)&CenterPos, (DWORD)&InhalePos );
g_pFcFXManager->SendMessage(m_nCastFXId, FX_PLAY_OBJECT);
*/

