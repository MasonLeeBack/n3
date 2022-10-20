#include "StdAfx.h"
#include ".\fcfxrain.h"
#include "BsMaterial.h"
#include "BsKernel.h"
#include "FcWorld.h"
#include "FcFXManager.h"
#include "FcHeroObject.h"
#include "FcCameraObject.h"
#include "FcUtil.h"
#include "Data\\FxList.h"

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) if(p) { delete []p; p = NULL; }
#endif //SAFE_DELETE_ARRAY

D3DXVECTOR3 RandomVector( D3DXVECTOR3 p_vMin, D3DXVECTOR3 p_vMax )
{
	D3DXVECTOR3 vResult;
	vResult.x = RandomNumberInRange(p_vMin.x, p_vMax.x);
	vResult.y = RandomNumberInRange(p_vMin.y, p_vMax.y);
	vResult.z = RandomNumberInRange(p_vMin.z, p_vMax.z);
	return vResult;
}

#define DEBUG_STRINGSSD(a,b,c)		DebugString( "%s %s %d \n",#a, #b, c);

/*---------------------------------------------------------------------------------
-
-			CFcFxRain::CFcFxRain()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
CFcFxRain::CFcFxRain(void)
{
	m_sRainInfo.Reset();
	m_psRainRenderUnit[0]	= NULL;
	m_psRainRenderUnit[1]	= NULL;
	m_nTick				= 0;
	m_nTexID			= -1;
	m_nMaterialIndex	= -1;
	m_nVertexDeclIdx	= -1;
	m_hWVP				= NULL;
	m_hTexture			= NULL;
	m_nFaceNum[0]		= 0;
	m_nFaceNum[1]		= 0;
#ifndef _LTCG
	SetFxRtti(FX_TYPE_RAINS);
#endif //_LTCG
}





/*---------------------------------------------------------------------------------
-
-			CFcFxRain::~CFcFxRain()
-					; .
-					  
-
---------------------------------------------------------------------------------*/
CFcFxRain::~CFcFxRain(void)
{
	Clear();
}





/*---------------------------------------------------------------------------------
-
-			CFcFxRain::Clear()
-					; .
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxRain::Clear()
{
	SAFE_DELETE_ARRAY(m_psRainRenderUnit[0]);
	SAFE_DELETE_ARRAY(m_psRainRenderUnit[1]);
	if( !m_listRainUnit.empty() )
	{
		std::list<SRainUnit*>::iterator iter;
		for( iter = m_listRainUnit.begin() ; iter != m_listRainUnit.end() ; iter++ )
		{
			SRainUnit* tmp = *iter;
			delete tmp;
		}
		m_listRainUnit.clear();
	}
	/*g_pFcFXManager->SendMessage(m_nFxid, FX_DELETE_OBJECT );
	m_nFxid = -1;*/
	SAFE_DELETE_FX( m_nFxid,FX_TYPE_LISTGENERATER);
}






/*---------------------------------------------------------------------------------
-
-			CFcFxRain::Init()
-					; .
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxRain::Init( SRainParam* p_pRainInfo )
{
	m_sRainInfo				= *p_pRainInfo;
	m_nVertexCount			= m_sRainInfo._nTotalCount * 3;
	m_nCountPerTick			= m_sRainInfo._nTick;
	m_psRainRenderUnit[0]	= new SRainRenderUnit[m_sRainInfo._nCount * 100];
	m_psRainRenderUnit[1]	= new SRainRenderUnit[m_sRainInfo._nCount * 100];
	m_nFxid					= g_pFcFXManager->Create(FX_TYPE_LISTGENERATER);
	g_pFcFXManager->SendMessage(m_nFxid,FX_INIT_OBJECT, FX_RAINGROUND, 13, -1);
	g_pFcFXManager->SendMessage(m_nFxid,FX_PLAY_OBJECT);

	g_BsKernel.chdir("Fx");
	m_nTexID = g_BsKernel.LoadTexture("rains.dds");
	g_BsKernel.chdir("..");

	g_BsKernel.chdir("material");
	char fullName[_MAX_PATH];
	strcpy(fullName, g_BsKernel.GetShaderDirectory());
	strcat(fullName, "TextureNoLighting.fx");
	m_nMaterialIndex	= g_BsKernel.LoadMaterial(fullName, FALSE);
	g_BsKernel.chdir("..");

	CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialIndex);
	D3DVERTEXELEMENT9 decl[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};
#ifdef _XBOX
	m_hWVP = pMaterial->GetParameterByName( "WorldViewProjection");
	m_hTexture = pMaterial->GetParameterByName( "diffuseSampler");
#else

	m_hWVP = pMaterial->GetParameterByName( "worldViewProj");
	m_hTexture = pMaterial->GetParameterByName( "diffuseTexture");
#endif
	m_nVertexDeclIdx = g_BsKernel.LoadVertexDeclaration(decl);
}





/*---------------------------------------------------------------------------------
-
-			CFcFxRain::Process()
-					; .
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxRain::Process()
{
	static int OldTick = m_nTick;
	static int OldTick2 = m_nTick;
	int CurrentTick = m_nTick;
	if( m_state == ENDING )
	{
		MAkeRain(false);
		MoveRain();
		TickCounter();
		//MakeFace();
		if(	m_listRainUnit.empty() )
		{
			m_state = END;
			m_DeviceDataState = FXDS_RELEASEREADY;
		}
		OldTick		= 0;
		OldTick2	= 0;
		return;
	}

	if( m_state == END ) return;

	if( m_nTick >= m_sRainInfo._nTotalTick && m_sRainInfo._nTotalTick != -99 )
	{
		m_state = ENDING;
	}

	int nSize = m_listRainUnit.size();

	if( nSize >= m_sRainInfo._nTotalCount )
	{
		OldTick = CurrentTick;
	}
	else if( CurrentTick - OldTick >= m_nCountPerTick )
	{
		OldTick = CurrentTick;
		MAkeRain(true);
	}
	MoveRain2();
	if( CurrentTick - OldTick2 >= 10 )	
	{
		OldTick2 = CurrentTick;
		MAkeRainGround();
	}
	TickCounter();

}




void CFcFxRain::Update()
{
	MakeFace();
}


/*---------------------------------------------------------------------------------
-
-			CFcFxRain::MAkeRain()
-					; 
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxRain::MAkeRain( bool p_bMake )
{
	CameraObjHandle CamHandle = CFcBaseObject::GetCameraObjectHandle(0);
	CCrossVector cross = *CamHandle->GetCrossVector();
	D3DXVECTOR3 vRight;
	D3DXVec3Cross( &vRight, &D3DXVECTOR3(0,1,0), &cross.m_ZVector);
	D3DXVECTOR3 vCameraPos = cross.m_PosVector;
	D3DXVECTOR3 vView = cross.m_ZVector;
	if( p_bMake )
	{
		// 갯수만큼 생성한다.
		D3DXVECTOR3 vMax = vCameraPos + vView * 2500.0f + vRight * 2500.0f;
		D3DXVECTOR3 vMin = vCameraPos + vView * (-1000.0f) + vRight * (-2500.0f);
		for( int i = 0 ; i < m_sRainInfo._nCount ; i++ )
		{
			SRainUnit* tmp = new SRainUnit;
			tmp->_vPos = RandomVector(vMin, vMax);
			tmp->_vPos.y = vCameraPos.y + 1200.0f;
			tmp->_vDir = D3DXVECTOR3(1,0,0) * 3.3f;
			tmp->_vDir.y -= (float)RandomNumberInRange( 42, 50);
			tmp->_fHeight = RandomNumberInRange(0.7f, 1.5f);
			tmp->_fWidth = RandomNumberInRange(0.8f, 1.2f);
			m_listRainUnit.push_back(tmp);
		}

		int num = m_listRainUnit.size();
	}
	else
	{
		// 갯수만큼 줄여나간다. 일단은 코딩 안해도 그만.
	}
}





/*---------------------------------------------------------------------------------
-
-			CFcFxRain::MakeFace()
-					; 
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxRain::MakeFace()
{
	int count = 0;
	if( !m_listRainUnit.empty() )
	{
		std::list<SRainUnit*>::iterator iter;
		CameraObjHandle CamHandle = CFcBaseObject::GetCameraObjectHandle(0);
		CCrossVector cross = *CamHandle->GetCrossVector();
		D3DXVECTOR3 vRight;
		D3DXVec3Cross( &vRight, &D3DXVECTOR3(0,1,0), &cross.m_ZVector);
		vRight.y = 0;
		D3DXVec3Normalize(&vRight, &vRight);
		for( iter = m_listRainUnit.begin() ; iter != m_listRainUnit.end() ; iter++ )
		{
			SRainUnit* tmp = *iter;

			if( (count >= m_sRainInfo._nCount * 100) || (count + 3 >= m_sRainInfo._nCount * 100) )
			{
				return;
			}

			m_psRainRenderUnit[CBsObject::GetProcessBufferIndex()][count]._v2Tex.x = 0.5f;
			m_psRainRenderUnit[CBsObject::GetProcessBufferIndex()][count]._v2Tex.y = 0.0f;
			m_psRainRenderUnit[CBsObject::GetProcessBufferIndex()][count++]._vPos = tmp->_vPos;

			m_psRainRenderUnit[CBsObject::GetProcessBufferIndex()][count]._v2Tex.x = 0.0f;
			m_psRainRenderUnit[CBsObject::GetProcessBufferIndex()][count]._v2Tex.y = 1.0f;
			m_psRainRenderUnit[CBsObject::GetProcessBufferIndex()][count++]._vPos = tmp->_vPos + tmp->_vDir*5.0f*tmp->_fHeight + vRight * 25.0f*tmp->_fWidth;

			m_psRainRenderUnit[CBsObject::GetProcessBufferIndex()][count]._v2Tex.x = 1.0f;
			m_psRainRenderUnit[CBsObject::GetProcessBufferIndex()][count]._v2Tex.y = 1.0f;
			m_psRainRenderUnit[CBsObject::GetProcessBufferIndex()][count++]._vPos = tmp->_vPos + tmp->_vDir*5.0f*tmp->_fHeight - vRight * 25.0f*tmp->_fWidth;
		}	
	}//if
	m_nFaceNum[CBsObject::GetProcessBufferIndex()] = count;
}





/*---------------------------------------------------------------------------------
-
-			CFcFxRain::MAkeRainGround()
-					; 
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxRain::MAkeRainGround()
{
	//바닦에 파티클을 생성한다.
	D3DXVECTOR3 vPos, vDir;
	CameraObjHandle CamHandle = CFcBaseObject::GetCameraObjectHandle(0);
	CCrossVector cross = *CamHandle->GetCrossVector();
	D3DXVECTOR3 vCameraPos = cross.m_PosVector;
	D3DXVECTOR3 vCameraDir = cross.m_ZVector;
	D3DXVECTOR3 vCameraRight;
	D3DXVec3Cross( &vCameraRight, &D3DXVECTOR3(0,1,0), &vCameraDir);
	D3DXVec3Normalize(&vCameraRight,&vCameraRight);
	D3DXVECTOR3 vMax = vCameraPos + vCameraRight * 1300.0f + vCameraDir * 1800.0f;
	D3DXVECTOR3 vMin = vCameraPos - vCameraRight * 1300.0f - vCameraDir * 800.0f;

	HeroObjHandle handle = g_FcWorld.GetHeroHandle(0);
	CCrossVector cross2 = *handle->GetCrossVector();
	float fHeight = cross2.m_PosVector.y;
	fHeight += 40.0f;
	for( int i = 0 ; i < (int)(m_sRainInfo._nTotalCount * 0.003f) ; i++)
	{
		vPos = RandomVector(vMin, vMax);
		vPos.y = fHeight;//g_FcWorld.GetLandHeight(vPos.x, vPos.z);
		g_pFcFXManager->SendMessage(m_nFxid, FX_UPDATE_OBJECT, (DWORD)&vPos, (DWORD)&D3DXVECTOR3(0,0,1));
	}
	
}






/*---------------------------------------------------------------------------------
-
-			CFcFxRain::MoveRain()
-					; list에 있는 녀석들을 하나씩 이동시킨다..
-					  이동시키면서 지면에 닿는 녀석들은 제거 시킨다.
-
---------------------------------------------------------------------------------*/
void CFcFxRain::MoveRain()
{
	if( !m_listRainUnit.empty() )
	{
		std::list<SRainUnit*>::iterator iter;
		for( iter = m_listRainUnit.begin() ; iter != m_listRainUnit.end() ;  )
		{
			SRainUnit* tmp = NULL;
			tmp = *iter;
			if( !tmp ) continue;
			tmp->_vPos += tmp->_vDir;

			HeroObjHandle handle = g_FcWorld.GetHeroHandle(0);
			CCrossVector cross = *handle->GetCrossVector();
			float fHeight = cross.m_PosVector.y;// g_FcWorld.GetLandHeight(tmp->_vPos.x, tmp->_vPos.z);
			if( fHeight >= tmp->_vPos.y + 100.0f )
			{
				iter = m_listRainUnit.erase(iter);
				delete tmp;
				continue;
			}

			iter++;
		}// for
	}//if
}






/*---------------------------------------------------------------------------------
-
-			CFcFxRain::MoveRain()
-					; list에 있는 녀석들을 하나씩 이동시킨다..
-					  이동시키면서 지면에 닿는 녀석들은 제거 시킨다.
-
---------------------------------------------------------------------------------*/
void CFcFxRain::MoveRain2()
{
	if( !m_listRainUnit.empty() )
	{
		std::list<SRainUnit*>::iterator iter;
		for( iter = m_listRainUnit.begin() ; iter != m_listRainUnit.end() ; iter++ )
		{
			SRainUnit* tmp = NULL;
			tmp = *iter;
			if( !tmp ) continue;
			tmp->_vPos += tmp->_vDir;

			HeroObjHandle handle = g_FcWorld.GetHeroHandle(0);
			CCrossVector cross = *handle->GetCrossVector();
			float fHeight = cross.m_PosVector.y;// g_FcWorld.GetLandHeight(tmp->_vPos.x, tmp->_vPos.z);
			if( fHeight >= tmp->_vPos.y + 100.0f )
			{
				PosReset(tmp);
			}
		}// for
	}//if
}





/*---------------------------------------------------------------------------------
-
-			CFcFxRain::PosReset()
-					; .
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxRain::PosReset(SRainUnit* p_pRain)
{
	CameraObjHandle CamHandle = CFcBaseObject::GetCameraObjectHandle(0);
	CCrossVector cross = *CamHandle->GetCrossVector();
	D3DXVECTOR3 vRight;
	D3DXVec3Cross( &vRight, &D3DXVECTOR3(0,1,0), &cross.m_ZVector);
	D3DXVECTOR3 vCameraPos = cross.m_PosVector;
	D3DXVECTOR3 vView = cross.m_ZVector;
	D3DXVECTOR3 vMax = vCameraPos + vView * 1900.0f + vRight * 1900.0f;
	D3DXVECTOR3 vMin = vCameraPos + vView * (-1500.0f) + vRight * (-1900.0f);
	p_pRain->_vPos = RandomVector(vMin, vMax);
	p_pRain->_vPos.y = vCameraPos.y + 1200.0f;
	p_pRain->_vDir = D3DXVECTOR3(1,0,0) * 3.3f;
	p_pRain->_vDir.y -= (float)RandomNumberInRange( 42, 50);
}





/*---------------------------------------------------------------------------------
-
-			CFcFxRain::MoveAllRain()
-					; .
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxRain::MoveAllRain( D3DXVECTOR3 p_vPos )
{
	if( !m_listRainUnit.empty() )
	{
		std::list<SRainUnit*>::iterator iter;
		for( iter = m_listRainUnit.begin() ; iter != m_listRainUnit.end() ; iter++ )
		{
			SRainUnit* tmp = NULL;
			tmp = *iter;
			if( !tmp ) continue;
			tmp->_vPos += p_vPos;
		}// for
	}//if
}






/*---------------------------------------------------------------------------------
-
-			CFcFxRain::TickCounter()
-					; .
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxRain::TickCounter()
{
	m_nTick++;
}






/*---------------------------------------------------------------------------------
-
-			CFcFxRain::PreRender()
-					; .
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxRain::PreRender()
{
	CFcFXBase::PreRender();
}






/*---------------------------------------------------------------------------------
-
-			CFcFxRain::Render()
-					; .
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxRain::Render(C3DDevice *pDevice)
{
	if( m_listRainUnit.empty() || m_nFaceNum[CBsObject::GetRenderBufferIndex()] == 0 ) return;

#ifdef	_XBOX
	LPDIRECT3DDEVICE9 pD3DDevice = pDevice->GetD3DDevice();

	D3DXMATRIX matWorldViewProj;
	D3DXMATRIX matWorld;
	D3DXMatrixIdentity(&matWorld);
	D3DXMatrixMultiply(&matWorldViewProj, &matWorld, CBsKernel::GetInstance().GetParamViewProjectionMatrix());
	CBsMaterial* pBsMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialIndex);
	pBsMaterial->BeginMaterial(0, 0);
	pBsMaterial->BeginPass(0);


	DWORD OldBlend[3];
	pDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &OldBlend[0] );
	pDevice->GetRenderState(D3DRS_SRCBLEND, &OldBlend[1] );
	pDevice->GetRenderState(D3DRS_DESTBLEND, &OldBlend[2] );

	pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	pDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

	g_BsKernel.SetVertexDeclaration(m_nVertexDeclIdx);
	pBsMaterial->SetMatrix( m_hWVP, &matWorldViewProj);
	pBsMaterial->SetTexture(m_hTexture, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(m_nTexID));
	pBsMaterial->CommitChanges();

	int num = m_nFaceNum[CBsObject::GetRenderBufferIndex()];
	if( m_sRainInfo._nCount * 100 <= num ) num = m_sRainInfo._nCount * 100 - 1;
//	DebugString("%s  : %d \n","빗방울 총 갯수",num);

	BYTE *pBuffer = NULL;
	HRESULT hr = pDevice->BeginVertices( D3DPT_TRIANGLELIST, num, sizeof(SRainRenderUnit), (void**)&pBuffer );
	if( SUCCEEDED( hr ) )
	{
		memcpy( pBuffer, m_psRainRenderUnit[CBsObject::GetRenderBufferIndex()], sizeof(SRainRenderUnit)*num );
		pDevice->EndVertices();
	}

	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, OldBlend[0] );
	pDevice->SetRenderState(D3DRS_SRCBLEND, OldBlend[1] );
	pDevice->SetRenderState(D3DRS_DESTBLEND, OldBlend[2] );

	pBsMaterial->EndPass() ;
	pBsMaterial->EndMaterial();
#endif
}





/*---------------------------------------------------------------------------------
-
-			CFcFxRain::ProcessMessage()
-					; .
-					  
-
---------------------------------------------------------------------------------*/
int CFcFxRain::ProcessMessage(int nCode,DWORD dwParam1/*=0*/, DWORD dwParam2/*=0*/,DWORD dwParam3/*=0*/)
{
	switch(nCode)
	{
	case FX_INIT_OBJECT:
		{
			Init( (SRainParam*)dwParam1 );
			m_DeviceDataState= FXDS_INITREADY;
		}
		return 1;
	case FX_PLAY_OBJECT:
		{
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
			m_state = ENDING;
		}
		return 1;

	case FX_UPDATE_OBJECT:
		{
			
		}
		return 1;

	}

	if( CFcFXBase::ProcessMessage( nCode, dwParam1, dwParam2, dwParam3) )
		return 1;

	return 0;
}