#include "stdafx.h"

#include "BsKernel.h"
#include "BsMesh.h"
#include "BsSubMesh.h"
#include "FcGlobal.h"
#include "FcFxBase.h"
#include "FcFXEarthExplode.h"
#include "BsMaterial.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


//#include "FcWorld.h"
//#include "FcHeroObject.h"


//DWORD m_dwKeyTable[26];// = { 0, 2, 4, 6, 8, 10 12 14 16  18 20};


CEarthLerper::CEarthLerper(float fStartValue)
{
	m_nTable = 5;
	m_pTable = new TimeValue[m_nTable];

	m_pTable[0]= TimeValue(0, 0.f);
	m_pTable[1]= TimeValue(20, 0.1f);
	m_pTable[2]= TimeValue(60, 0.2f);
	m_pTable[3]= TimeValue(70, 1.f);

	m_pTable[4]= TimeValue(135, 1.f);
}






CFcFXEarth::CFcFXEarth()
{

	m_state = READY;
	m_DeviceDataState=FXDS_INITREADY;
//	m_fFrame = 0.f;

	m_pEarthLerper = new CEarthLerper(0.f);
#ifndef _LTCG
	SetFxRtti(FX_TYPE_EARTHEXPLODE);
#endif //_LTCG
}

CFcFXEarth::~CFcFXEarth()
{
	for(int ii = 0; ii < 1 ; ++ii)
	{
		g_BsKernel.DeleteObject(m_nObjIdx[ii]);
		m_nObjIdx[ii] = -1;
	}

	for( int i = 0 ; i < (int)m_vecSkinList.size() ; i++ )
	{
		SAFE_RELEASE_SKIN(m_vecSkinList[i]);
	}
	m_vecSkinList.clear();
}

void CFcFXEarth::Initialize()
{
	g_BsKernel.chdir("Fx");

	int nSkinIdx = g_BsKernel.LoadSkin( -1, "earthtummy.skin" );
	m_vecSkinList.push_back(nSkinIdx);
	for(int ii = 0; ii < 1; ii++)
		m_nObjIdx[ii]	=	CBsKernel::GetInstance().CreateStaticObjectFromSkin(nSkinIdx);

	g_BsKernel.chdir("..");
	m_DeviceDataState = FXDS_INITREADY; 
};
void CFcFXEarth::InitDeviceData()
{
	
}

void CFcFXEarth::ReleaseDeviceData()
{
	
}

void CFcFXEarth::Process()
{
	if(m_nObjIdx[0] < 0)
		return;

	float fETick = float(m_dwTick);

	float fTime = fETick/140.f;

	if(fTime > 1.f)
		fTime = 1.f;


	//	fTime = 0.0f;

	float fScale = 1.f;

	//	 if( fTime > 0.9f )
	//		fScale = (1.f-fTime) * 10.f;


	
	D3DXMatrixIdentity( &m_Matrix );
	m_Matrix._11 = 0.5f;
	m_Matrix._22 = 0.5f;
	m_Matrix._33 = 0.5f;



	m_Matrix._41 = m_vPos.x;
	m_Matrix._42 = m_vPos.y;// + 500;
	m_Matrix._43 = m_vPos.z;

	float fAlpha = 1.0f;

	//	int nParamIndex0 = g_BsKernel.SendMessage(m_nObjIdx[0], BS_ADD_EDITABLE_PARAMETER, 0, PARAM_EFFECT_CUSTOM0 ); // Normaliz
	//	int nParamIndex1 = g_BsKernel.SendMessage(m_nObjIdx[0], BS_ADD_EDITABLE_PARAMETER, 0, PARAM_EFFECT_CUSTOM1 );
	//	int nParamIndex2 = g_BsKernel.SendMessage(m_nObjIdx[0], BS_ADD_EDITABLE_PARAMETER, 0, PARAM_EFFECT_CUSTOM2 );
	//	int nParamIndex3 = g_BsKernel.SendMessage(m_nObjIdx[0], BS_ADD_EDITABLE_PARAMETER, 0, PARAM_EFFECT_CUSTOM3 );

	D3DXVECTOR4 vePoint;

	//	float t = m_pEarthLerper->GetVaule(m_dwTick);
	//	vePoint.x = 0.3f + (0.9f- 0.3f)*t;// 0.3~ 0.9
	//	vePoint.y = 0.1f + (0.1f- 0.4f)*t;// 0.4~ 0.1
	//	vePoint.z = 0.6f + (0.2f- 0.6f)*t;// 0.6~ 0.2
	vePoint.w = 0.f;



	vePoint.x = -9.f*(fETick- 45.f)*(fETick- 45.f)/16000.f+0.9f;// a(x-b)^2+c
	vePoint.x = BsMax(vePoint.x, 0.f);

	vePoint.y = -5.f*(fETick- 30.f)*(fETick- 30.f)/6250.f+0.5f;
	vePoint.y = BsMax(vePoint.y, 0.f);

	vePoint.z = -7.f*(fETick- 20.f)*(fETick- 20.f)/12.f+0.7f;
	vePoint.z = BsMax(vePoint.z, 0.f);


	vePoint.w = 0.f; 
	if( fTime > 0.3f)
		vePoint.w = (fTime-0.3f)*2.f; // LerpValue

	if(vePoint.w > 1.f)
		vePoint.w = 1.f;

	m_dwTick++;
}

void CFcFXEarth::Update()
{
//	for(int ii = 0; ii < 1 ; ii++)
//	{
		g_BsKernel.UpdateObject( m_nObjIdx[0], &m_Matrix );
//	}
}



int CFcFXEarth::ProcessMessage(int nCode,DWORD dwParam1, DWORD dwParam2,DWORD dwParam3)
{
	switch(nCode)
	{
		case FX_INIT_OBJECT:
			Initialize();
			return 1;

		case FX_PLAY_OBJECT:
			m_state = PLAY;
			m_dwTick = 0;
			return 1;

		case FX_STOP_OBJECT:
			m_state = READY;
			return 1;

		case FX_UPDATE_OBJECT:
			m_vPos = *(D3DXVECTOR3*)dwParam1;
			return 1;
	}
	if ( CFcFXBase::ProcessMessage( nCode, dwParam1, dwParam2, dwParam3) )
		return 1;
	return  0;
}


void CFcFXEarth::PreRender()
{
	CFcFXBase::PreRender();
	if(m_state != PLAY)
		return;
}