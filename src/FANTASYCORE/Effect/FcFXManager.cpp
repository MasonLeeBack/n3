#include "stdafx.h"
#include "BsKernel.h"
#include "3DDevice.h"
#include "FcFXManager.h"
#include "FcFXBase.h"
#include "FcFXSimpleTrail.h"
#include "FcFXScreenWaterDrop.h"
#include "FcFXLensFlare.h"
#include "FcFXTidalWave.h"
#include "FcFXScreenMotionBlur.h"
#include "FcFXSimplePlay.h"
#include "FcFXEarthExplode.h"
#include "FcFXCastEarth.h"
#include "FcFXWindExplode.h"
#include "FcFXDynamicFog.h"
#include "FcFXLine.h"
#include "FcFXColumnOfWater.h"
#include "FcFxDropOfWater.h"
#include "fcfxspoutsofwater.h"
#include ".\fcfxgenerater.h"
#include ".\fcfxparticlegenerater.h"
#include ".\fcfxburning.h"
#include ".\fcfxrain.h"
#include ".\fcfxshotskin.h"
#include "FcFXLightScattering.h"

//ObrSpark
#include "FcFXMeteo.h"


#include "FcGlobal.h"
#include "FcUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


const int _MAX_FXCOUNT = 2000;

CFcFXManager::CFcFXManager()
{
	m_nMaxFx = 1000;
	m_FXVector = new CFcFXBase*[m_nMaxFx];


	for(int ii = 0; ii < m_nMaxFx ; ++ii)
	{
		m_FXVector[ii] = NULL;
	}
	g_BsKernel.AttachCustomRender(this);

	//preload
	g_BsKernel.chdir("Fx");
	//Earth
//	char szBuf[128];
//	for(int ii = 0; ii < 26 ; ++ii)
//	{
//		sprintf( szBuf, "fx_earth_boom_mesh.%d.bm", ii*2+1);
//		g_BsKernel.LoadMesh( -1, szBuf ); // n개
//	}
//	g_BsKernel.LoadTexture("fx_earth_trail_map_03.dds");

	int nTexID = g_BsKernel.LoadVolumeTexture("twist_N.dds");
	m_PreLoadTexIdxs.push_back(nTexID);
	nTexID = g_BsKernel.LoadVolumeTexture("twist_N2.dds");
	m_PreLoadTexIdxs.push_back(nTexID);
	nTexID = g_BsKernel.LoadVolumeTexture("smoke_vol_256_b.dds");
	m_PreLoadTexIdxs.push_back(nTexID);
	nTexID = g_BsKernel.LoadTexture("arrowline.dds");
	m_PreLoadTexIdxs.push_back(nTexID);
	g_BsKernel.chdir("..");

	char fullName[_MAX_PATH];
	strcpy(fullName, g_BsKernel.GetShaderDirectory());
	strcat(fullName, "DynamicFog.fx");
	int nMaterilIndex = g_BsKernel.LoadMaterial(fullName, FALSE);
	m_PreLoadTexIdxs.push_back(nMaterilIndex);


	strcpy(fullName, g_BsKernel.GetShaderDirectory());
#ifdef _XBOX
	strcat(fullName, "LineTrail.fx");
#else 
	strcat(fullName, "LineTrail_PC.fx");
#endif
	nMaterilIndex = g_BsKernel.LoadMaterial(fullName, FALSE);
	m_PreLoadTexIdxs.push_back(nMaterilIndex);


	//preload~

	m_dwLastProcessTick = -1;

//#ifdef _DEBUG
//	m_nCurrentFXCount = 0;
//#endif
}

CFcFXManager::~CFcFXManager()
{
	int nPreLoadTex = m_PreLoadTexIdxs.size();
	for( int ii = 0; ii < nPreLoadTex ; ++ii)
	{
		SAFE_RELEASE_TEXTURE(m_PreLoadTexIdxs[ii]);
	}
	m_PreLoadTexIdxs.clear();

	int nPreMaterial = m_PreMaterialIdxs.size();
	for( int ii = 0; ii < nPreMaterial ; ++ii)
	{
		SAFE_RELEASE_MATERIAL(m_PreMaterialIdxs[ii]);
	}
	m_PreMaterialIdxs.clear();

	Reset();
	//for(int i = 0 ; i < m_nMaxFx ; ++i) 
	//{
	//	if(m_FXVector[i])
	//	{
	//		delete m_FXVector[i];
	//		m_FXVector[i] = NULL;
	//	}
	//}
//	m_FXVector.clear();

	delete[] m_FXVector;
	g_BsKernel.DetachCustomRender();
}

void CFcFXManager::Reset()
{
	for(int i = 0 ; i < m_nMaxFx ; ++i) 
	{
		if(m_FXVector[i])
		{
			m_FXVector[i]->ProcessMessage(FX_DELETE_OBJECT);
			m_FXVector[i]->ReleaseDeviceData(); // ReleaseDeviceData
			delete m_FXVector[i];
			m_FXVector[i] = NULL;
		}
	}
}

void CFcFXManager::Process()
{
//	CreateBookedFX();
	//if ( m_dwLastProcessTick == ::GetProcessTick() )
	//	return;

	//m_dwLastProcessTick = ::GetProcessTick();

	ProcessBookedMessage();

	//int vectorSize = m_FXVector.size();
	int nTotalfx = 0;

	{
		THREAD_AUTOLOCK(&m_csFxManager);

		for(int i = 0 ; i < m_nMaxFx ; ++i) 
		{
			if(m_FXVector[i])
			{
				nTotalfx++;
				if(m_FXVector[i]->IsFinished() )
				{
					//delete m_FXVector[i];
					//m_FXVector[i] = NULL;
				}
				else if( m_FXVector[i]->GetState() == PLAY || m_FXVector[i]->GetState() == ENDING)
					m_FXVector[i]->Process();
			}
		}
	}

	CFcFXBase::Tick();
}

void CFcFXManager::Render(C3DDevice *pDevice)
{
	//int vectorSize = m_FXVector.size();
	int nPriority = 0;

	pDevice->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
	pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE);
	pDevice->SetRenderState( D3DRS_ALPHAREF, 0x01);

	pDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
	pDevice->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD);
	pDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE);


	for(int i = 0 ; i < m_nMaxFx ; ++i) 
	{
		if( m_FXVector[i] && ( m_FXVector[i]->GetState() == PLAY) && (m_FXVector[i]->GetDState() == FXDS_INITED) )
			m_FXVector[i]->RenderZero(pDevice);
	}
	for(;nPriority <= c_MAX_PRIORITY; nPriority++ )
	{
		for(int i = 0 ; i < m_nMaxFx ; ++i) 
		{
			if(m_FXVector[i])
			{
				if(m_FXVector[i]->GetPriority() != nPriority)
					continue;

				if(m_FXVector[i]->IsFinished() )
				{
//					delete m_FXVector[i];
//					m_FXVector[i] = NULL;					
//#ifdef _DEBUG
//					m_nCurrentFXCount--;
//					DebugString("FX Count %d\n", m_nCurrentFXCount);
//#endif
				}
				else if( ( m_FXVector[i]->GetState() == PLAY || m_FXVector[i]->GetState() == ENDING) 
					&& m_FXVector[i]->GetDState() == FXDS_INITED )
				{
					m_FXVector[i]->Render(pDevice);
				}
			}
		}
	}
	pDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
	pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE);
	pDevice->SetRenderState( D3DRS_ALPHAREF, 0x7F);
	pDevice->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD);
	pDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW);


	{
		for(int i = 0 ; i < m_nMaxFx ; ++i) 
		{
			if(m_FXVector[i] && m_FXVector[i]->IsFinished() )
			{
				THREAD_AUTOLOCK(&m_csFxManager);

				delete m_FXVector[i];
				m_FXVector[i] = NULL;
			}
		}
	}
}

int CFcFXManager::SendMessage(int nHandle, int nCode, DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{
	if(nHandle < 0)
	{
		return 0;
	}
	if( nHandle >= m_nMaxFx )
	{
		return 0;
	}
	if(m_FXVector[nHandle])
	{
		return m_FXVector[nHandle]->ProcessMessage(nCode, dwParam1, dwParam2, dwParam3);
	}
	DebugString("Fx Unhandled msg %d\n", nHandle);
	return 0;
}


int CFcFXManager::SendMessageQueue(int nHandle, int nCode, DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{
	BOOKEDMSG msg;
	msg.nHandle = nHandle;
	msg.nCode	= nCode;
	msg.dwParam1= dwParam1;
	msg.dwParam2= dwParam2;
	msg.dwParam3= dwParam3;

	m_listMSG.push_back(msg);
	return 0;
}

void CFcFXManager::ProcessBookedMessage()
{
	int nCount = m_listMSG.size();

	int nHandle;
	int nCode;
	DWORD dwParam1, dwParam2, dwParam3;
	for(int  ii = 0 ; ii < nCount; ++ii)
	{
		nHandle = m_listMSG[ii].nHandle;
		nCode	= m_listMSG[ii].nCode;
		dwParam1= m_listMSG[ii].dwParam1;
		dwParam2= m_listMSG[ii].dwParam2;
		dwParam3= m_listMSG[ii].dwParam3;

		if(nHandle < 0)
		{
			continue;
		}
		if( nHandle >= m_nMaxFx )
		{
			continue;
		}
		if(m_FXVector[nHandle])
		{
			m_FXVector[nHandle]->ProcessMessage(nCode, dwParam1, dwParam2, dwParam3);
		}
	}
	m_listMSG.clear();
}

int CFcFXManager::Create( FX_TYPE eType)
{
	int  nEmptySlot = GetEmptySlot();
	if( nEmptySlot==-1 ) 
	{
		BsAssert( 0 && "No Empty FX Slot!!");
		return -1;
	}

	CFcFXBase* pFx = NULL;
	switch(eType) 
	{
	case FX_TYPE_WEAPONTRAIL:
		pFx =new CFcFXSimpleTrail();
		break;
	case FX_TYPE_WATERDROP:
		pFx =new CFcFXScreenWaterDrop();
		break;
	case FX_TYPE_LENSFLARE:
		pFx =new CFcFXLensFlare();
		break;
	case FX_TYPE_TIDALWAVE:
		pFx =new CFcFXTidalWave();
		break;
	case FX_TYPE_MOTIONBLUR:
		pFx =new CFcFXScreenMotionBlur();
		break;	
	case FX_TYPE_METEOR:
		pFx =new CFcFXMeteor();
		break;
    case FX_TYPE_SIMPLE_PLAY:
        pFx =new CFcFXSimplePlay();
        break;
	case FX_TYPE_EARTHEXPLODE:
		// pFx =new CFcFXEarthExplode();
		pFx =new CFcFXEarth();
        break;
	case FX_TYPE_CASTEARTH:
		pFx =new CFloatStone2();
		break;
	case FX_TYPE_WINDEXPLODE:
//		pFx =new CFcFXWindExplode();
		pFx =new CFcFXWind();
		break;
	case FX_TYPE_XM:
		pFx =new CFcFXXMB();
		break;
	case FX_TYPE_LINECURVE:
		pFx =new CFcFXLineCurve();
		break;
	case FX_TYPE_LINETRAIL:
		pFx =new CFcFXLineTrail();
		break;
	case FX_TYPE_DYNAMICFOG:
		pFx = new CFcFXFog();
		break;
	case FX_TYPE_BBOARD:
		pFx = new CFcFXBillboard();
		break;
	case FX_TYPE_LOMOFILTER:
		pFx = new CFcFXLomoFilter();
		break;
	case FX_TYPE_PIECEOFMETEOR:
		pFx = new CPieceOfMeteor();
		break;
	case FX_TYPE_INPHYMOTIONBLUR:
		pFx =new CFcFXInppyyBlur();
		break;
	case FX_TYPE_COLUMNWATER:
		pFx =new CFcFXColumnOfWater();
		break;
	case FX_TYPE_2DFSEFFECT:
		pFx =new CFcFX2DFSEffect();
		break;
	case FX_TYPE_DROPOFWATER:
		pFx =new CFcFxDropOfWater();
		break;
	case FX_TYPE_SPOUTSOFWATER:
		pFx =new CFcFxSpoutsOfWater();
		break;
	case FX_TYPE_BURNING:
		pFx =new CFcFxBurning();
		break;
	case FX_TYPE_GENERATER:
		pFx =new CFcFxGenerater();
		break;
	case FX_TYPE_THROWGENERATER:
		pFx =new CFcFxThrowGenerater();
		break;
	case FX_TYPE_PARTICLEGENERATER:
		pFx =new CFcFxParticleGenerater();
		break;
	case FX_TYPE_LIGHTSCATTERING:
		pFx =new CFcFXLightScattering();
		break;
	case FX_TYPE_LISTGENERATER:
		pFx =new CFcFxListGenerater();
		break;//
	case FX_TYPE_RANDOMGENERATER:
		pFx =new CFcFxRandomGenerater();
		break;
	case FX_TYPE_RAINS:
		pFx =new CFcFxRain;
		break;
	case FX_TYPE_SHOTSKIN:
		pFx =new CFcFxShotSkin;
		break;
	}

	if(pFx)	{
		m_FXVector[nEmptySlot] = pFx;
		return nEmptySlot;
	}
    
	BsAssert(0 &&"Fx Create Fail!!");
	return -1;
}



int CFcFXManager::GetEmptySlot()
{
//	int nSize = m_FXVector.size();
	int nSize  = m_nMaxFx;
	int i;
	for( i= 0 ; i < nSize ; ++i) 
	{
		if( NULL == m_FXVector[i] )
			break;
	}

	if( i >= nSize )
	{
		DebugString( "Pool 부족\n" );
		return -1;
	}
	else 
		return i;
}

void CFcFXManager::PreUpdate()
{
	//int vectorSize = m_FXVector.size();
	for(int i = 0 ; i < m_nMaxFx ; ++i) 
	{
		if(m_FXVector[i]/*&& ( m_FXVector[i]->GetState() == PLAY || m_FXVector[i]->GetState() == ENDING)*/)
		{
			m_FXVector[i]->PreRender();
		}
	}
}

void CFcFXManager::Update()
{
	THREAD_AUTOLOCK(&m_csFxManager);
	for(int i = 0 ; i < m_nMaxFx ; ++i) 
	{
		if(m_FXVector[i] /*&& ( m_FXVector[i]->GetState() == PLAY || m_FXVector[i]->GetState() == ENDING)*/)
		{
			m_FXVector[i]->Update();
		}
	}
}

void CFcFXManager::PreLoadByChar(int nChar, bool p_bPlayer /*= true*/)
{
	switch(nChar)
	{
	case 0:		//As
		{
			g_BsKernel.chdir("Fx");
			char szFxFilePath[MAX_PATH];
			int nIdx = -1;
			if( p_bPlayer )
			{
				sprintf(szFxFilePath,"%sFX_HC_OL2.bfx",g_BsKernel.GetCurrentDirectory() );
				nIdx = g_BsKernel.LoadFXTemplate(-1, szFxFilePath);
				if( nIdx != -1 ) 
                    m_PreLoadBfxIdxsChr.push_back(nIdx);
				else
				{
					DebugString( "PreLoding Hero Fx File Not Finding : %s \n", szFxFilePath);
					BsAssert(0);
				}
				nIdx = -1;
			}
			
			g_BsKernel.chdir("..");

			nIdx = -1;
			strcpy(szFxFilePath, g_BsKernel.GetShaderDirectory());
			if( p_bPlayer )
			{
				strcat(szFxFilePath, "LomoFilter.fx");
				nIdx = g_BsKernel.LoadMaterial(szFxFilePath, FALSE);
				if( nIdx != -1 ) 
					m_PreMaterialIdxsChr.push_back(nIdx);
				else
				{
					DebugString( "PreLoding Hero Fx File Not Finding : %s \n", szFxFilePath);
					BsAssert(0);
				}
				nIdx = -1;
			}

			g_BsKernel.chdir("Fx");
			int nTextureId = g_BsKernel.LoadTexture( "Aspharr_SwordTrail01.dds" );
			if( nTextureId != -1 ) 
				m_PreLoadTexIdxsChr.push_back(nTextureId);

			nTextureId = g_BsKernel.LoadTexture( "Aspharr_SwordTrail02.dds" );
			if( nTextureId != -1 ) 
				m_PreLoadTexIdxsChr.push_back(nTextureId);
			
			nTextureId = g_BsKernel.LoadTexture( "Aspharr_SwordTrail_Normals.dds" );
			if( nTextureId != -1 ) 
				m_PreLoadTexIdxsChr.push_back(nTextureId);
			g_BsKernel.chdir("..");
		}
		break;
	case 1:		//in
		{
			g_BsKernel.chdir("Fx");
			int nIdx = -1;
			char szFxFilePath[MAX_PATH];
			if( p_bPlayer )
			{
				nIdx = g_BsKernel.LoadTexture("Soultwinkle.dds");
				if( nIdx != -1 ) 
                    m_PreLoadTexIdxsChr.push_back(nIdx);
				else
				{
					DebugString( "PreLoding Hero Fx File Not Finding : Soultwinkle.dds \n");
					BsAssert(0);
				}
				nIdx = -1;

				nIdx = g_BsKernel.LoadTexture("SoulTrail.dds");
				if( nIdx != -1 ) 
                    m_PreLoadTexIdxsChr.push_back(nIdx);
				else
				{
					DebugString( "PreLoding Hero Fx File Not Finding : SoulTrail.dds \n");
					BsAssert(0);
				}
				nIdx = -1;
				
				nIdx = g_BsKernel.LoadTexture("SoulGhost.dds");
				if( nIdx != -1 ) 
                    m_PreLoadTexIdxsChr.push_back(nIdx);
				else
				{
					DebugString( "PreLoding Hero Fx File Not Finding : SoulGhost.dds \n");
					BsAssert(0);
				}
				nIdx = -1;
				
				sprintf(szFxFilePath,"%sFX_HC_OL2.bfx",g_BsKernel.GetCurrentDirectory() );
				nIdx = g_BsKernel.LoadFXTemplate(-1, szFxFilePath);
				if( nIdx != -1 ) 
					m_PreLoadBfxIdxsChr.push_back(nIdx);
				else
				{
					DebugString( "PreLoding Hero Fx File Not Finding : %s \n", szFxFilePath);
					BsAssert(0);
				}
				nIdx = -1;
			}

			nIdx = -1;		
			
			nIdx = g_BsKernel.LoadTexture("Inphyy_SwordTrail01.dds");
			if( nIdx != -1 ) 
				m_PreLoadTexIdxsChr.push_back(nIdx);
			else
			{
				DebugString( "PreLoding Hero Fx File Not Finding : Inphyy_SwordTrail01.dds \n");
				BsAssert(0);
			}
			nIdx = -1;

			nIdx = g_BsKernel.LoadTexture("Inphyy_SwordTrail02.dds");
			if( nIdx != -1 ) 
				m_PreLoadTexIdxsChr.push_back(nIdx);
			else
			{
				DebugString( "PreLoding Hero Fx File Not Finding : Inphyy_SwordTrail02.dds \n");
				BsAssert(0);
			}
			nIdx = -1;

			nIdx = g_BsKernel.LoadTexture("Inphyy_SwordTrail03.dds");
			if( nIdx != -1 ) 
				m_PreLoadTexIdxsChr.push_back(nIdx);
			else
			{
				DebugString( "PreLoding Hero Fx File Not Finding : Inphyy_SwordTrail03.dds \n");
				BsAssert(0);
			}
			nIdx = -1;

			nIdx = g_BsKernel.LoadTexture("Inphyy_SwordTrail_Normals.dds");
			if( nIdx != -1 ) 
				m_PreLoadTexIdxsChr.push_back(nIdx);
			else
			{
				DebugString( "PreLoding Hero Fx File Not Finding : Inphyy_SwordTrail_Normals.dds \n");
				BsAssert(0);
			}
			nIdx = -1;

			g_BsKernel.chdir("..");

			strcpy(szFxFilePath, g_BsKernel.GetShaderDirectory());
			strcat(szFxFilePath, "BBoard.fx");
			nIdx = g_BsKernel.LoadMaterial(szFxFilePath, FALSE);
			if( nIdx != -1 ) 
				m_PreMaterialIdxsChr.push_back(nIdx);
			else
			{
				DebugString( "PreLoding Hero Fx File Not Finding : %s \n", szFxFilePath);
				BsAssert(0);
			}
			nIdx = -1;

			if( p_bPlayer )
			{
				strcpy(szFxFilePath, g_BsKernel.GetShaderDirectory());
				strcat(szFxFilePath, "XMB.fx");
				nIdx = g_BsKernel.LoadMaterial(szFxFilePath, FALSE);
				if( nIdx != -1 ) 
                    m_PreMaterialIdxsChr.push_back(nIdx);
				else
				{
					DebugString( "PreLoding Hero Fx File Not Finding : %s \n", szFxFilePath);
					BsAssert(0);
				}
				nIdx = -1;
			}

		}
		break;
	case 2:		//my
		{
			g_BsKernel.chdir("Fx");
			int nIdx = -1;

			nIdx = g_BsKernel.LoadTexture("Myifee_SwordTrail01.dds");
			if( nIdx != -1 ) 
				m_PreLoadTexIdxsChr.push_back(nIdx);
			else
			{
				DebugString( "PreLoding Hero Fx File Not Finding : Myifee_SwordTrail01.dds \n");
				BsAssert(0);
			}
			nIdx = -1;

			nIdx = g_BsKernel.LoadTexture("Myifee_SwordTrail02.dds");
			if( nIdx != -1 ) 
				m_PreLoadTexIdxsChr.push_back(nIdx);
			else
			{
				DebugString( "PreLoding Hero Fx File Not Finding : Myifee_SwordTrail02.dds \n");
				BsAssert(0);
			}
			nIdx = -1;

			nIdx = g_BsKernel.LoadTexture("Myifee_SwordTrail03.dds");
			if( nIdx != -1 ) 
				m_PreLoadTexIdxsChr.push_back(nIdx);
			else
			{
				DebugString( "PreLoding Hero Fx File Not Finding : Myifee_SwordTrail03.dds \n");
				BsAssert(0);
			}
			nIdx = -1;

			nIdx = g_BsKernel.LoadTexture("Myifee_SwordTrail_Normals.dds");
			if( nIdx != -1 ) 
				m_PreLoadTexIdxsChr.push_back(nIdx);
			else
			{
				DebugString( "PreLoding Hero Fx File Not Finding : Myifee_SwordTrail_Normals.dds \n");
				BsAssert(0);
			}
			nIdx = -1;

			nIdx = g_BsKernel.LoadTexture("C_LP_WM_CHIAN_Fx.dds");
			if( nIdx != -1 ) 
				m_PreLoadTexIdxsChr.push_back(nIdx);
			else
			{
				DebugString( "PreLoding Hero Fx File Not Finding : C_LP_WM_CHIAN_Fx.dds \n");
				BsAssert(0);
			}
			nIdx = -1;

			nIdx = -1;
			g_BsKernel.chdir("..");
		}
		break;
	case 3:		//ty
		{
			g_BsKernel.chdir("Fx");
			//skin
			int nIdx = -1;
			nIdx = g_BsKernel.LoadSkin( -1, "WATERBOMBSPHERE.skin" );
			if( nIdx != -1 ) 
                m_PreLoadSkinIdxsChr.push_back(nIdx);
			else
			{
				DebugString( "PreLoding Hero Fx File Not Finding : WATERBOMBSPHERE.skin \n");
				BsAssert(0);
			}
			nIdx = -1;

			for( int i = 0 ; i < 35 ; i++ )						
			{
				char buff[100];
				if( i+1 < 10 )
					sprintf( buff, "%s%d.skin" , "WATER_COLIDE_HU00",i+1);
				else
					sprintf( buff, "%s%d.skin" , "WATER_COLIDE_HU0",i+1);
				nIdx = g_BsKernel.LoadSkin( -1, buff );
				if( nIdx != -1 ) 
                    m_PreLoadSkinIdxsChr.push_back(nIdx);
				else
				{
					DebugString( "PreLoding Hero Fx File Not Finding : %s \n",buff);
					BsAssert(0);
				}
				nIdx = -1;
			}
			
			// load texture
			nIdx = g_BsKernel.LoadTexture("diffuse4water.dds");
			if( nIdx != -1 ) 
				m_PreLoadTexIdxsChr.push_back(nIdx);
			else
			{
				DebugString( "PreLoding Hero Fx File Not Finding : diffuse4water.dds \n");
				BsAssert(0);
			}
			nIdx = -1;

			if( p_bPlayer )
			{
				nIdx = g_BsKernel.LoadVolumeTexture("swizzle_vol.dds");
				if( nIdx != -1 ) 
					m_PreLoadTexIdxsChr.push_back(nIdx);
				else
				{
					DebugString( "PreLoding Hero Fx File Not Finding : swizzle_vol.dds \n");
					BsAssert(0);
				}
				nIdx = -1;
			}
			

			// load skin
			nIdx = g_BsKernel.LoadSkin( -1, "WSHOT_T001.SKIN" );
			m_PreLoadSkinIdxsChr.push_back(nIdx);
			nIdx = -1;

			// Levia
			nIdx = g_BsKernel.LoadSkin( -1, "Ryvius.skin" );
			m_PreLoadSkinIdxsChr.push_back(nIdx);
			nIdx = -1;

			g_BsKernel.chdir("..");

			//load material
			char szFxFilePath[MAX_PATH];
			strcpy(szFxFilePath, g_BsKernel.GetShaderDirectory());
			strcat(szFxFilePath, "meshdistortionlight2.fx");
			nIdx = g_BsKernel.LoadMaterial(szFxFilePath, FALSE);
			if( nIdx != -1 ) 
				m_PreMaterialIdxsChr.push_back(nIdx);
			else
			{
				DebugString( "PreLoding Hero Fx File Not Finding : %s \n", szFxFilePath);
				BsAssert(0);
			}
			nIdx = -1;

			if( p_bPlayer )
			{
				strcpy(szFxFilePath, g_BsKernel.GetShaderDirectory());
				strcat(szFxFilePath, "FakeLiquid.fx");
				nIdx = g_BsKernel.LoadMaterial(szFxFilePath, FALSE);
				if( nIdx != -1 ) 
					m_PreMaterialIdxsChr.push_back(nIdx);
				else
				{
					DebugString( "PreLoding Hero Fx File Not Finding : %s \n", szFxFilePath);
					BsAssert(0);
				}
				nIdx = -1;
			}

		}
		break;
	case 4:		//dw
		{
			g_BsKernel.chdir("Fx");
			int nIdx = -1;

			if( p_bPlayer )
			{
				nIdx = g_BsKernel.LoadTexture("WindOrbDustBillboard.dds");
				if( nIdx != -1 ) 
					m_PreLoadTexIdxsChr.push_back(nIdx);
				else
				{
					DebugString( "PreLoding Hero Fx File Not Finding : %s \n","WindOrbDustBillboard.dds");
					BsAssert(0);
				}
				nIdx = -1;
				
				nIdx = g_BsKernel.LoadTexture("WindOrbDustBillboard2.dds");
				if( nIdx != -1 ) 
					m_PreLoadTexIdxsChr.push_back(nIdx);
				else
				{
					DebugString( "PreLoding Hero Fx File Not Finding : %s \n","WindOrbDustBillboard2.dds");
					BsAssert(0);
				}
				nIdx = -1;
			}


			nIdx = g_BsKernel.LoadTexture("Trail.dds");
			if( nIdx != -1 ) 
				m_PreLoadTexIdxsChr.push_back(nIdx);
			else
			{
				DebugString( "PreLoding Hero Fx File Not Finding : %s \n","Trail.dds");
				BsAssert(0);
			}
			nIdx = -1;
			
			if( p_bPlayer )
			{
				nIdx = g_BsKernel.LoadSkin( -1, "ORBSPARKCLOUD_POINT.Skin" );
				if( nIdx != -1 ) 
					m_PreLoadSkinIdxsChr.push_back(nIdx);
				else
				{
					DebugString( "PreLoding Hero Fx File Not Finding : %s \n","ORBSPARKCLOUD_POINT.Skin");
					BsAssert(0);
				}
				nIdx = -1;

				nIdx = g_BsKernel.LoadSkin( -1, "ORBSPARKCLOUD.Skin" );
				if( nIdx != -1 ) 
					m_PreLoadSkinIdxsChr.push_back(nIdx);
				else
				{
					DebugString( "PreLoding Hero Fx File Not Finding : %s \n","ORBSPARKCLOUD.Skin");
					BsAssert(0);
				}
				nIdx = -1;
				
				nIdx = g_BsKernel.LoadAni( -1, "ORBSPARKCLOUD.BA" );
				if( nIdx != -1 ) 
					m_PreLoadAniIdxsChr.push_back(nIdx);
				else
				{
					DebugString( "PreLoding Hero Fx File Not Finding : %s \n","ORBSPARKCLOUD.BA");
					BsAssert(0);
				}
				nIdx = -1;

				nIdx = g_BsKernel.LoadAni( -1, "ORBSPARKCLOUD2.BA" );
				if( nIdx != -1 ) 
					m_PreLoadAniIdxsChr.push_back(nIdx);
				else
				{
					DebugString( "PreLoding Hero Fx File Not Finding : %s \n","ORBSPARKCLOUD2.BA");
					BsAssert(0);
				}
				nIdx = -1;
			}
			
			//skin
			/*nIdx = g_BsKernel.LoadSkin( -1, "FX_WIND_CASTING_TRAIL_SMALL.skin" );
			if( nIdx != -1 ) 
				m_PreLoadSkinIdxsChr.push_back(nIdx);
			else
			{
				DebugString( "PreLoding Hero Fx File Not Finding : %s \n","FX_WIND_CASTING_TRAIL_SMALL.skin");
				BsAssert(0);
			}
			nIdx = -1;

			nIdx = g_BsKernel.LoadSkin( -1, "FX_WIND_CASTING_TRAIL_BIG.skin" );
			if( nIdx != -1 ) 
				m_PreLoadSkinIdxsChr.push_back(nIdx);
			else
			{
				DebugString( "PreLoding Hero Fx File Not Finding : %s \n","FX_WIND_CASTING_TRAIL_BIG.skin");
				BsAssert(0);
			}
			nIdx = -1;*/

			nIdx = g_BsKernel.LoadTexture("Dwingvatt_SwordTrail01.dds");
			if( nIdx != -1 ) 
				m_PreLoadTexIdxsChr.push_back(nIdx);
			else
			{
				DebugString( "PreLoding Hero Fx File Not Finding : Dwingvatt_SwordTrail01.dds \n");
				BsAssert(0);
			}
			nIdx = -1;

			nIdx = g_BsKernel.LoadTexture("Dwingvatt_SwordTrail02.dds");
			if( nIdx != -1 ) 
				m_PreLoadTexIdxsChr.push_back(nIdx);
			else
			{
				DebugString( "PreLoding Hero Fx File Not Finding : Dwingvatt_SwordTrail02.dds \n");
				BsAssert(0);
			}
			nIdx = -1;

			nIdx = g_BsKernel.LoadTexture("Dwingvatt_SwordTrail03.dds");
			if( nIdx != -1 ) 
				m_PreLoadTexIdxsChr.push_back(nIdx);
			else
			{
				DebugString( "PreLoding Hero Fx File Not Finding : Dwingvatt_SwordTrail03.dds \n");
				BsAssert(0);
			}
			nIdx = -1;

			nIdx = g_BsKernel.LoadTexture("Dwingvatt_SwordTrail_Normals.dds");
			if( nIdx != -1 ) 
				m_PreLoadTexIdxsChr.push_back(nIdx);
			else
			{
				DebugString( "PreLoding Hero Fx File Not Finding : Dwingvatt_SwordTrail_Normals.dds \n");
				BsAssert(0);
			}
			nIdx = -1;

			g_BsKernel.chdir("..");
		}
		break;	
	case 5:		//Vg
		{
			char szFxFilePath[MAX_PATH];
			g_BsKernel.chdir("Fx");
			int nIdx = -1;
			if( p_bPlayer )
			{
			
				sprintf(szFxFilePath,"%sFX_HC_OE.bfx",g_BsKernel.GetCurrentDirectory() );
				nIdx = g_BsKernel.LoadFXTemplate(-1, szFxFilePath);
				if( nIdx != -1 ) 
					m_PreLoadBfxIdxsChr.push_back(nIdx);
				else
				{
					DebugString( "PreLoding Hero Fx File Not Finding : %s \n",szFxFilePath);
					BsAssert(0);
				}
				nIdx = -1;

				nIdx = g_BsKernel.LoadTexture("EarthOrb2d.dds");
				if( nIdx != -1 ) 
					m_PreLoadTexIdxsChr.push_back(nIdx);
				else
				{
					DebugString( "PreLoding Hero Fx File Not Finding : %s \n","EarthOrb2d.dds");
					BsAssert(0);
				}
				nIdx = -1;

				nIdx = g_BsKernel.LoadVolumeTexture("stones16_N.dds");
				if( nIdx != -1 ) 
					m_PreLoadTexIdxsChr.push_back(nIdx);
				else
				{
					DebugString( "PreLoding Hero Fx File Not Finding : %s \n","stones16_N.dds");
					BsAssert(0);
				}
				nIdx = -1;

				nIdx = g_BsKernel.LoadTexture("EarthOrb2dDustExp.dds");
				if( nIdx != -1 ) 
					m_PreLoadTexIdxsChr.push_back(nIdx);
				else
				{
					DebugString( "PreLoding Hero Fx File Not Finding : %s \n","EarthOrb2dDustExp.dds");
					BsAssert(0);
				}
				nIdx = -1;
				
				//skin
				nIdx = g_BsKernel.LoadSkin( -1, "earthtummy.skin" );
				if( nIdx != -1 ) 
					m_PreLoadSkinIdxsChr.push_back(nIdx);
				else
				{
					DebugString( "PreLoding Hero Fx File Not Finding : %s \n","earthtummy.skin");
					BsAssert(0);
				}
				nIdx = -1;
			}


			g_BsKernel.chdir("..");

			//material
			if( p_bPlayer )
			{
				strcpy(szFxFilePath, g_BsKernel.GetShaderDirectory());
				strcat(szFxFilePath, "floatStone.fx");
				nIdx = g_BsKernel.LoadMaterial(szFxFilePath, FALSE);
				if( nIdx != -1 ) 
					m_PreMaterialIdxsChr.push_back(nIdx);
				else
				{
					DebugString( "PreLoding Hero Fx File Not Finding : %s \n","floatStone.fx");
					BsAssert(0);
				}
				nIdx = -1;
			}

			strcpy(szFxFilePath, g_BsKernel.GetShaderDirectory());
			strcat(szFxFilePath, "2DFSEffect.fx");
			nIdx = g_BsKernel.LoadMaterial(szFxFilePath, FALSE);
			if( nIdx != -1 ) 
				m_PreMaterialIdxsChr.push_back(nIdx);
			else
			{
				DebugString( "PreLoding Hero Fx File Not Finding : %s \n","2DFSEffect.fx");
				BsAssert(0);
			}
			nIdx = -1;

		}
		break;
	case 6:		//kr
		{
			char szFxFilePath[MAX_PATH];
			g_BsKernel.chdir("Fx");
			int nIdx = -1;

			if( p_bPlayer)
			{
				sprintf(szFxFilePath,"%sFX_HC_OS.bfx",g_BsKernel.GetCurrentDirectory() );
				nIdx = g_BsKernel.LoadFXTemplate(-1, szFxFilePath);
				if( nIdx != -1 ) 
					m_PreLoadBfxIdxsChr.push_back(nIdx);
				else
				{
					DebugString( "PreLoding Hero Fx File Not Finding : %s \n",szFxFilePath);
					BsAssert(0);
				}
				nIdx = -1;

				sprintf(szFxFilePath,"%sFX_HC_OS2.bfx",g_BsKernel.GetCurrentDirectory() );
				nIdx = g_BsKernel.LoadFXTemplate(-1, szFxFilePath);
				if( nIdx != -1 ) 
					m_PreLoadBfxIdxsChr.push_back(nIdx);
				else
				{
					DebugString( "PreLoding Hero Fx File Not Finding : %s \n",szFxFilePath);
					BsAssert(0);
				}
				nIdx = -1;

				sprintf(szFxFilePath,"%sFX_HC_OS3.bfx",g_BsKernel.GetCurrentDirectory() );
				nIdx = g_BsKernel.LoadFXTemplate(-1, szFxFilePath);
				if( nIdx != -1 ) 
					m_PreLoadBfxIdxsChr.push_back(nIdx);
				else
				{
					DebugString( "PreLoding Hero Fx File Not Finding : %s \n",szFxFilePath);
					BsAssert(0);
				}
				nIdx = -1;

				sprintf(szFxFilePath,"%skar_streamSword.bfx",g_BsKernel.GetCurrentDirectory() );
				nIdx = g_BsKernel.LoadFXTemplate(-1, szFxFilePath);
				if( nIdx != -1 ) 
					m_PreLoadBfxIdxsChr.push_back(nIdx);
				else
				{
					DebugString( "PreLoding Hero Fx File Not Finding : %s \n",szFxFilePath);
					BsAssert(0);
				}
				nIdx = -1;
			}
			
			nIdx = g_BsKernel.LoadTexture("Klarrann_SwordTrail01.dds");
			if( nIdx != -1 ) 
				m_PreLoadTexIdxsChr.push_back(nIdx);
			else
			{
				DebugString( "PreLoding Hero Fx File Not Finding : Klarrann_SwordTrail01.dds \n");
				BsAssert(0);
			}
			nIdx = -1;

			nIdx = g_BsKernel.LoadTexture("Klarrann_SwordTrail02.dds");
			if( nIdx != -1 ) 
				m_PreLoadTexIdxsChr.push_back(nIdx);
			else
			{
				DebugString( "PreLoding Hero Fx File Not Finding : Klarrann_SwordTrail02.dds \n");
				BsAssert(0);
			}
			nIdx = -1;

			nIdx = g_BsKernel.LoadTexture("Klarrann_SwordTrail_Normals.dds");
			if( nIdx != -1 ) 
				m_PreLoadTexIdxsChr.push_back(nIdx);
			else
			{
				DebugString( "PreLoding Hero Fx File Not Finding : Klarrann_SwordTrail_Normals.dds \n");
				BsAssert(0);
			}
			nIdx = -1;

			sprintf(szFxFilePath,"%skra_attack10_Hammer01.bfx",g_BsKernel.GetCurrentDirectory() );
			nIdx = g_BsKernel.LoadFXTemplate(-1, szFxFilePath);
			if( nIdx != -1 ) 
				m_PreLoadBfxIdxsChr.push_back(nIdx);
			else
			{
				DebugString( "PreLoding Hero Fx File Not Finding : %s \n",szFxFilePath);
				BsAssert(0);
			}
			nIdx = -1;

			sprintf(szFxFilePath,"%skra_attack10_Axe01.bfx",g_BsKernel.GetCurrentDirectory() );
			nIdx = g_BsKernel.LoadFXTemplate(-1, szFxFilePath);
			if( nIdx != -1 ) 
				m_PreLoadBfxIdxsChr.push_back(nIdx);
			else
			{
				DebugString( "PreLoding Hero Fx File Not Finding : %s \n",szFxFilePath);
				BsAssert(0);
			}
			nIdx = -1;

			sprintf(szFxFilePath,"%skra_attack10_Scythe01.bfx",g_BsKernel.GetCurrentDirectory() );
			nIdx = g_BsKernel.LoadFXTemplate(-1, szFxFilePath);
			if( nIdx != -1 ) 
				m_PreLoadBfxIdxsChr.push_back(nIdx);
			else
			{
				DebugString( "PreLoding Hero Fx File Not Finding : %s \n",szFxFilePath);
				BsAssert(0);
			}
			nIdx = -1;

			g_BsKernel.chdir("..");


			strcpy(szFxFilePath, g_BsKernel.GetShaderDirectory());
			strcat(szFxFilePath, "2DFSEffect.fx");
			nIdx = g_BsKernel.LoadMaterial(szFxFilePath, FALSE);
			if( nIdx != -1 ) 
				m_PreMaterialIdxsChr.push_back(nIdx);
			else
			{
				DebugString( "PreLoding Hero Fx File Not Finding : %s \n",szFxFilePath);
				BsAssert(0);
			}
			nIdx = -1;
		}
		break;
	}

	// 공통적인 프리 로딩 파일들.
	g_BsKernel.chdir("Fx");

	// 텍스쳐 로딩.
	//char szFxFilePath[MAX_PATH];
	int nIdx = g_BsKernel.LoadTexture("FX0080_SwordTrail_X_01.dds");
	if( nIdx != -1 ) 
		m_PreLoadTexIdxsChr.push_back(nIdx);
	else
	{
		DebugString( "PreLoding Hero Fx File Not Finding : %s \n","FX0080_SwordTrail_X_01.dds");
		BsAssert(0);
	}
	nIdx = -1;
	
	nIdx = g_BsKernel.LoadTexture("FX0080_SwordTrail_X_Normals.dds");
	if( nIdx != -1 ) 
		m_PreLoadTexIdxsChr.push_back(nIdx);
	else
	{
		DebugString( "PreLoding Hero Fx File Not Finding : %s \n","FX0080_SwordTrail_X_Normals.dds");
		BsAssert(0);
	}
	nIdx = -1;

	nIdx = g_BsKernel.LoadTexture("Noise.dds");
	if( nIdx != -1 ) 
		m_PreLoadTexIdxsChr.push_back(nIdx);
	else
	{
		DebugString( "PreLoding Hero Fx File Not Finding : %s \n","Noise.dds");
		BsAssert(0);
	}
	nIdx = -1;

	nIdx = g_BsKernel.LoadTexture("Table.dds");
	if( nIdx != -1 ) 
		m_PreLoadTexIdxsChr.push_back(nIdx);
	else
	{
		DebugString( "PreLoding Hero Fx File Not Finding : %s \n","Table.dds");
		BsAssert(0);
	}
	nIdx = -1;

	g_BsKernel.chdir("..");

}


void CFcFXManager::ReleasePreLoadedByChar()
{
	int nPreLoadTex = m_PreLoadTexIdxsChr.size();
	for( int ii = 0; ii < nPreLoadTex ; ++ii)
	{
		SAFE_RELEASE_TEXTURE(m_PreLoadTexIdxsChr[ii]);
	}
	m_PreLoadTexIdxsChr.clear();

	int nPreMaterial = m_PreMaterialIdxsChr.size();
	for( int ii = 0; ii < nPreMaterial ; ++ii)
	{
		SAFE_RELEASE_MATERIAL(m_PreMaterialIdxsChr[ii]);
	}
	m_PreMaterialIdxsChr.clear();

	int nPreSkin = m_PreLoadSkinIdxsChr.size();
	for( int ii = 0; ii < nPreSkin ; ++ii)
	{
		if(m_PreLoadSkinIdxsChr[ii] > -1) {
			g_BsKernel.ReleaseSkin(m_PreLoadSkinIdxsChr[ii]);
			m_PreLoadSkinIdxsChr[ii] = -1;
		}
	}
	m_PreLoadSkinIdxsChr.clear();

	int nPreAni = m_PreLoadAniIdxsChr.size();
	for( int ii = 0; ii < nPreAni ; ++ii)
	{
		SAFE_RELEASE_ANI(m_PreLoadAniIdxsChr[ii]);
	}
	m_PreLoadAniIdxsChr.clear();

	int nPreBfx = m_PreLoadBfxIdxsChr.size();
	for( int ii = 0; ii < nPreBfx ; ++ii)
	{
		if(m_PreLoadBfxIdxsChr[ii] > -1) {
			g_BsKernel.ReleaseFXTemplate(m_PreLoadBfxIdxsChr[ii]);
			m_PreLoadBfxIdxsChr[ii] = -1;
		}
	}
	m_PreLoadBfxIdxsChr.clear();
}