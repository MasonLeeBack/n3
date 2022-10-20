#include "stdafx.h"
#include "BsKernel.h"
#include "BsMesh.h"
#include "BsSubMesh.h"
#include "FcGlobal.h"
#include "FcFxBase.h"
#include "FcFXWindExplode.h"
#include "BsMaterial.h"


#include "FcWorld.h"
#include "FcHeroObject.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


// [PREFIX:beginmodify] 2006/2/15 junyash PS#751 reports CFcFXWindExplode::FindKeyFrame() bounds vioration g_dwWindTable[], but not exist that method in recent source
//DWORD  g_dwWindTable[40];	// not referred... comment out.
// [PREFIX:endmodify] junyash



CCloudMeshScale::CCloudMeshScale(float fStartValue)
{
	m_nTable = 6;
	m_pTable = new TimeValue[m_nTable];

	m_pTable[0]= TimeValue(1, 0.25f);
	m_pTable[1]= TimeValue(20, 0.35f);
	m_pTable[2]= TimeValue(30, 0.5f);
	m_pTable[3]= TimeValue(45, 0.6f);
	m_pTable[4]= TimeValue(62, 0.62f);
	m_pTable[5]= TimeValue(70, 1.f);
}

CFcFXWind::CFcFXWind()
{
	m_state = READY;
	m_DeviceDataState=FXDS_INITREADY;
	m_fFrame = 1.f;
	m_nObjIdx[0] = -1;
	m_nObjIdx[1] = -1;
	m_ScaleTable = new CCloudMeshScale(0.f);
	m_fScaleX = 1.0f;
	m_fScaleY = 1.0f;
	m_nAniType = 0;
#ifndef _LTCG
	SetFxRtti(FX_TYPE_WINDEXPLODE);
#endif //_LTCG
}

CFcFXWind::~CFcFXWind()
{
	for( int i = 0 ; i < (int)m_vecSkinList.size() ; i++ )
	{
		SAFE_RELEASE_SKIN(m_vecSkinList[i]);
	}
	m_vecSkinList.clear();

	for( int i = 0 ; i < (int)m_vecAniList.size() ; i++ )
	{
		SAFE_RELEASE_ANI(m_vecAniList[i]);
	}
	m_vecAniList.clear();
}


void CFcFXWind::InitDeviceData()
{
	g_BsKernel.chdir("Fx");
	int	nMeshIdx  = g_BsKernel.LoadSkin( -1, "ORBSPARKCLOUD_POINT.Skin" );
	m_vecSkinList.push_back(nMeshIdx);
	int nMeshIdx2 = g_BsKernel.LoadSkin( -1, "ORBSPARKCLOUD.Skin" );
	m_vecSkinList.push_back(nMeshIdx2);
	int nAniIndex = g_BsKernel.LoadAni( -1, "ORBSPARKCLOUD.BA" );
	m_vecAniList.push_back(nAniIndex);
	int nAniIndex2 = g_BsKernel.LoadAni( -1, "ORBSPARKCLOUD2.BA" );
	m_vecAniList.push_back(nAniIndex2);

	if (m_nAniType == 0)
	{
		m_nObjIdx[0]	=	CBsKernel::GetInstance().CreateAniObjectFromSkin(nMeshIdx, nAniIndex);
		m_nObjIdx[1]	=	CBsKernel::GetInstance().CreateAniObjectFromSkin(nMeshIdx2, nAniIndex);
	}
	else
	{
		m_nObjIdx[0]	=	CBsKernel::GetInstance().CreateAniObjectFromSkin(nMeshIdx, nAniIndex2);
		m_nObjIdx[1]	=	CBsKernel::GetInstance().CreateAniObjectFromSkin(nMeshIdx2, nAniIndex2);
	}
	
	
	g_BsKernel.SendMessage(m_nObjIdx[0], BS_SET_POINTSPRITE, 0, 1);
	g_BsKernel.SendMessage(m_nObjIdx[0], BS_SET_POINTSPRITE, 1, 1);
	g_BsKernel.SendMessage(m_nObjIdx[0], BS_SET_POINTSPRITE, 2, 1);

	g_BsKernel.chdir("..");
	m_dwStartTick = GetTick();
}

void CFcFXWind::ReleaseDeviceData()
{
	// Render Thread에서 DeleteObject호출을 막았습니다. by jeremy
//	for(int ii = 0; ii < 2 ; ++ii)
//	{
//		g_BsKernel.DeleteObject(m_nObjIdx[ii]);
//		m_nObjIdx[ii] = -1;
//	}
}

void CFcFXWind::Process()
{
	if(m_nObjIdx[0] < 0)
		return;

	if(m_state != PLAY)
		return;

	m_Matrix = *(D3DXMATRIX*)m_Cross;

	float fAlpha = 1.0f;

	float fSize = 200000;

	float fFrameStart = 200.0f;

	if( m_fFrame > fFrameStart)
	{
		fSize *= (1.0f + (m_fFrame - fFrameStart)*0.05f);
	}

	int nParamIndex0 = g_BsKernel.SendMessage(m_nObjIdx[0], BS_ADD_EDITABLE_PARAMETER, 0, PARAM_EFFECT_CUSTOM0 );
	g_BsKernel.SendMessage(m_nObjIdx[0], BS_SET_EDITABLE_PARAMETER, 0,nParamIndex0, DWORD(&fSize) );
	nParamIndex0 = g_BsKernel.SendMessage(m_nObjIdx[0], BS_ADD_EDITABLE_PARAMETER, 1, PARAM_EFFECT_CUSTOM0 );
	g_BsKernel.SendMessage(m_nObjIdx[0], BS_SET_EDITABLE_PARAMETER, 1,nParamIndex0, DWORD(&fSize) );
	nParamIndex0 = g_BsKernel.SendMessage(m_nObjIdx[0], BS_ADD_EDITABLE_PARAMETER, 2, PARAM_EFFECT_CUSTOM0 );
	g_BsKernel.SendMessage(m_nObjIdx[0], BS_SET_EDITABLE_PARAMETER, 2,nParamIndex0, DWORD(&fSize) );

	float fScale = m_ScaleTable->GetVaule( DWORD(m_fFrame) );//0.5f;

	GET_ANIDISTANCE_INFO Info;
	D3DXVECTOR3 vPos( 0.f, 0.f, 0.f );
	D3DXVECTOR3 Pos( 0.f, 0.f, 0.f );

	
	int nAniLength = g_BsKernel.SendMessage( m_nObjIdx[0], BS_GETANILENGTH, 0 );
	if( nAniLength <= 0 ) 
	{
		DebugString("AniFrame is negative value.");
		nAniLength = 1;
	}
	if( nAniLength <= m_fFrame )
	{
		fScale	*= m_fFrame/(float)nAniLength;
	}
	else if( m_fFrame > 1.f)
	{
		Info.fFrame1 = m_fFrame;
		Info.fFrame2 = m_fFrame-1.f;
		Info.nAni1 = 0;
		Info.nAni2 = 0;
		Info.pVector = &vPos;
		g_BsKernel.GetAniDistance( m_nObjIdx[0], &Info );

		Pos -= m_Cross.m_XVector * vPos.x;
		Pos -= m_Cross.m_YVector * vPos.y;
		Pos -= m_Cross.m_ZVector * vPos.z;
	}

	float fScaleY;
	D3DXMATRIX mtxScale;

	if (m_fFrame < 20.0f)
		fScaleY = fScale * m_fFrame / 20.0f;
	else
		fScaleY = fScale;

	float fScaleGlobalX = 0.5f * m_fScaleX;
	float fScaleGlobalY = 0.3f * m_fScaleY;
	float fScaleGlobalZ = 0.5f;

	D3DXMatrixScaling(&mtxScale, -(0.5f+fScale*0.5f) * fScaleGlobalX, BsMin(1.0f, fScaleY) * fScaleGlobalY, -(0.5f+fScale*0.5f) * fScaleGlobalZ);


	m_Cross.m_PosVector += Pos;
	D3DXMatrixMultiply(&m_Matrix, &mtxScale, &m_Matrix );

	
	//m_Matrix._41 = m_Cross.m_PosVector.x;
	//m_Matrix._42 = m_Cross.m_PosVector.y;
	//m_Matrix._43 = m_Cross.m_PosVector.z;


	float	fFrameAlphaStart = fFrameStart + 10.0f;

	if( m_fFrame > fFrameAlphaStart)
		fAlpha = 1.0f - (m_fFrame - fFrameAlphaStart) * 0.05f;


	nParamIndex0 = g_BsKernel.SendMessage(m_nObjIdx[0], BS_ADD_EDITABLE_PARAMETER, 0, PARAM_EFFECT_CUSTOM1 );
	g_BsKernel.SendMessage(m_nObjIdx[0], BS_SET_EDITABLE_PARAMETER, 0, nParamIndex0, DWORD(&fAlpha) );
	g_BsKernel.SendMessage(m_nObjIdx[0], BS_SET_EDITABLE_PARAMETER, 1, nParamIndex0, DWORD(&fAlpha) );
	g_BsKernel.SendMessage(m_nObjIdx[0], BS_SET_EDITABLE_PARAMETER, 2, nParamIndex0, DWORD(&fAlpha) );


	if(fAlpha < 1.0f)
	{
		g_BsKernel.SendMessage(m_nObjIdx[1], BS_SET_SUBMESH_ALPHABLENDENABLE, 0, true );
		g_BsKernel.SendMessage(m_nObjIdx[1], BS_SET_SUBMESH_ALPHABLENDENABLE, 1, true );
		g_BsKernel.SendMessage(m_nObjIdx[1], BS_SET_SUBMESH_ALPHABLENDENABLE, 2, true );
		g_BsKernel.SendMessage(m_nObjIdx[1], BS_SET_SUBMESH_ALPHA, 0, DWORD(&fAlpha) );
		g_BsKernel.SendMessage(m_nObjIdx[1], BS_SET_SUBMESH_ALPHA, 1, DWORD(&fAlpha) );
		g_BsKernel.SendMessage(m_nObjIdx[1], BS_SET_SUBMESH_ALPHA, 2, DWORD(&fAlpha) );
	}
	else
	{
		g_BsKernel.SendMessage(m_nObjIdx[1], BS_SET_SUBMESH_ALPHABLENDENABLE, 0, false );
		g_BsKernel.SendMessage(m_nObjIdx[1], BS_SET_SUBMESH_ALPHABLENDENABLE, 1, false );
		g_BsKernel.SendMessage(m_nObjIdx[1], BS_SET_SUBMESH_ALPHABLENDENABLE, 2, false );
	}



	if( nAniLength > m_fFrame )
	{

		g_BsKernel.SendMessage( m_nObjIdx[1], BS_SETCURRENTANI, 0, ( DWORD )&m_fFrame ); 
		g_BsKernel.SendMessage( m_nObjIdx[0], BS_SETCURRENTANI, 0, ( DWORD )&m_fFrame ); 
	}
		
		
	m_fFrame++;

	if(m_fFrame > nAniLength)
		m_state = END;
}

void CFcFXWind::Update()
{
	if(m_state != PLAY)
		return;

	if(m_DeviceDataState != FXDS_INITED)
		return;

	int nAniLength = g_BsKernel.SendMessage( m_nObjIdx[0], BS_GETANILENGTH, 0 );

/*
	if( nAniLength <= m_fFrame )
	{
		g_BsKernel.UpdateObject( m_nObjIdx[3], &m_Matrix );
		g_BsKernel.UpdateObject( m_nObjIdx[2], &m_Matrix );
	}
	else*/
	if( nAniLength > m_fFrame )
	{
		D3DXMATRIX	m;

		m = m_Matrix;

		float	fDist = 350.0f;

		m._41 += m_Cross.m_ZVector.x * fDist;
		m._42 += m_Cross.m_ZVector.y * fDist;
		m._43 += m_Cross.m_ZVector.z * fDist;

		g_BsKernel.UpdateObject( m_nObjIdx[1], &m );
		g_BsKernel.UpdateObject( m_nObjIdx[0], &m_Matrix );
	}
}


int CFcFXWind::ProcessMessage(int nCode,DWORD dwParam1, DWORD dwParam2,DWORD dwParam3)
{
	switch(nCode)
	{
	case FX_INIT_OBJECT:
		m_fScaleX = ((float) dwParam1) * .1f;
		m_fScaleY = ((float) dwParam2) * .1f;
		m_nAniType = (int)dwParam3;

		Initialize();
		return 1;

	case FX_PLAY_OBJECT:
		m_state = PLAY;
//		m_dwStartTick = GetProcessTick();
		return 1;

	case FX_STOP_OBJECT:
		m_state = READY;
		return 1;

	case FX_UPDATE_OBJECT:
		m_Cross = *(CCrossVector*)dwParam1;
		return 1;

	case FX_GET_POS:
		{
			D3DXVECTOR3* pvPos = (D3DXVECTOR3*)dwParam1;
			*pvPos = m_Cross.m_PosVector;
		}
		return 1;
	case FX_DELETE_OBJECT:
		{
			// 이건 CFcFXBase::ProcessMessage로 Command를 Pass해야하기 때문에 break로 처리합니다. by jeremy
			// TODO	:	Message 처리 함수가 규칙이 없고, 위험해 보이는 코드가 너무 많습니다. 확인 필요
			for(int i = 0; i < 2 ; ++i) {
				g_BsKernel.DeleteObject(m_nObjIdx[i]);
				m_nObjIdx[i] = -1;
			}
		}
		break;
	}
	if ( CFcFXBase::ProcessMessage( nCode, dwParam1, dwParam2, dwParam3) )
		return 1;
	return  0;
}


void CFcFXWind::PreRender()
{
	CFcFXBase::PreRender();
	if(m_state != PLAY)
		return;

	C3DDevice* pDevice = g_BsKernel.GetDevice();
	float PointSizeMax = 255.f;
//	pDevice->SetRenderState( D3DRS_POINTSPRITEENABLE, TRUE );
	pDevice->SetRenderState( D3DRS_POINTSIZE_MAX, *((DWORD*)&PointSizeMax) );
}