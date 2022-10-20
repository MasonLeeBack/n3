#include "stdafx.h"
#include "BsKernel.h"
#include "FcGlobal.h"
#include "FcFxBase.h"
#include "FcFXSimpleTrail.h"
#include "BsImageProcess.h"
#include "BsMaterial.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG



//int CFcFXSimpleTrail::_MAX_POOL = 200;
//int CFcFXSimpleTrail::_MAX_INTERPOLATE = 10;
//int CFcFXSimpleTrail::_MAX_STEP			= 3;


CFcFXSimpleTrail::CFcFXSimpleTrail()
{
	//CFcFXBase::CFcFXBase(); //aleksger: prefix bug 747: explicit calls to constructors are extraneous.
	m_nSampleTrailIndex = 0;
	ZeroMemory( m_SampleTrail, sizeof(TRAIL_INFO)*3);

	m_TrailColor	= D3DXVECTOR4(1.0f,1.0f,1.0f, 1.f);
	m_fLifeTime		= 30; //단위 프레임
	m_fTrailEndTime = 0.f;
	m_nPoolIdx		= 0;

	m_nMaterialIndex	= -1;
	m_nTextureID		= -1;
	m_nOffsetTextureID	= -1;
	m_nRecentAccumCnt	= 0;

	m_fCurTime = 0.f;
	m_fStartTime = 0.f;
	m_nAdded = 0;

	m_fLength = 0.f;

	m_nLoadedTexCount = 10;
	m_pLoadedTexIDs = new int[m_nLoadedTexCount];
	memset( m_pLoadedTexIDs, -1, sizeof(int)*m_nLoadedTexCount );
	m_bTextureChange = FALSE;
	m_nDefTextureID = -1;
	m_nDefOffsetTextureID = -1;

	m_nBufferIdx = 0;

	m_nVertexDeclIdx = -1;	

	m_dwAlphaBlendOP = D3DBLENDOP_ADD;
	m_dwSRCBlend	= D3DBLEND_ONE;
	m_dwDSTBlend	= D3DBLEND_ONE;
#ifndef _LTCG
	SetFxRtti(FX_TYPE_WEAPONTRAIL);
#endif //_LTCG

}

CFcFXSimpleTrail::~CFcFXSimpleTrail()
{
	if(m_pLoadedTexIDs)
	{
		delete []m_pLoadedTexIDs;
		m_pLoadedTexIDs = NULL;
	}
}

void CFcFXSimpleTrail::Initialize()
{
	CFcFXBase::Initialize();
}

void	CFcFXSimpleTrail::InitDeviceData()
{
	g_BsKernel.chdir("Fx");
	m_nDefTextureID= g_BsKernel.LoadTexture("FX0080_SwordTrail_X_01.dds");
	m_nDefOffsetTextureID = g_BsKernel.LoadTexture("FX0080_SwordTrail_X_Normals.dds");
	g_BsKernel.chdir("..");

	// TODO : 이거 확인해주세요. Reference Count로 관리되는 변수를 copy해서 쓰는 코드는 좋지 않습니다. by jeremy
	if(m_nTextureID == -1)
		m_nTextureID = m_nDefTextureID;

	if(m_nOffsetTextureID == -1)
		m_nOffsetTextureID = m_nDefOffsetTextureID;
	
	char fullName[_MAX_PATH];
	strcpy(fullName, g_BsKernel.GetShaderDirectory());
//#ifdef _XBOX
	strcat(fullName, "SimpleTrail.fx");
//else
//	strcat(fullName, "SimpleTrail_PC.fx");
//#endif
	m_nMaterialIndex = g_BsKernel.LoadMaterial(fullName, FALSE);
	BsAssert( m_nMaterialIndex != -1  && "SimpleTrail Shader Compile Error!!" );

	CBsMaterial* pBsMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialIndex);
#ifdef _XBOX
	m_hVP = pBsMaterial->GetParameterByName("ViewProjection");
	m_hDiffTexture = pBsMaterial->GetParameterByName("diffSampler");
	m_hEnvTexture = pBsMaterial->GetParameterByName("envSampler");
	m_hNormalTexture = pBsMaterial->GetParameterByName("normalSampler");

	m_hCurTime = pBsMaterial->GetParameterByName("elapsedtime");
	m_hLife = pBsMaterial->GetParameterByName("unitsscale");
	m_hLifeRsq = pBsMaterial->GetParameterByName("lifeRsq");
	m_hColor = pBsMaterial->GetParameterByName("Diffuse0");
	m_hAlpha = pBsMaterial->GetParameterByName("fTAlpha");
#else
	m_hVP = pBsMaterial->GetParameterByName("ViewProj");
	m_hDiffTexture = pBsMaterial->GetParameterByName("diffTexture");
	m_hEnvTexture = pBsMaterial->GetParameterByName("envTexture");
	m_hNormalTexture = pBsMaterial->GetParameterByName("normalTexture");

	m_hCurTime = pBsMaterial->GetParameterByName("curTime");
	m_hLife = pBsMaterial->GetParameterByName("life");
	m_hLifeRsq = pBsMaterial->GetParameterByName("lifeRsq");
	m_hColor = pBsMaterial->GetParameterByName("fColor");
	m_hAlpha = pBsMaterial->GetParameterByName("fTAlpha");
#endif

	D3DVERTEXELEMENT9 decl[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};

	m_nVertexDeclIdx = g_BsKernel.LoadVertexDeclaration(decl);
}

void	CFcFXSimpleTrail::ReleaseDeviceData()
{
	SAFE_RELEASE_MATERIAL(m_nMaterialIndex);

	SAFE_RELEASE_VD(m_nVertexDeclIdx);

	// TODO : 이것도 제대로 되고 있는지 궁금합니다. 확인해주세요.
	SAFE_RELEASE_TEXTURE(m_nDefTextureID);

	m_nTextureID = -1;

	SAFE_RELEASE_TEXTURE(m_nDefOffsetTextureID);

	m_nOffsetTextureID = -1;

	if(m_pLoadedTexIDs)
	{
		for(int ii = 0; ii < m_nLoadedTexCount; ++ii)
		{
			SAFE_RELEASE_TEXTURE(m_pLoadedTexIDs[ii]);
		}
	}

//	delete[] m_pLoadedTexIDs;
//	m_pLoadedTexIDs = NULL
}

void CFcFXSimpleTrail::Process()
{
	m_fCurTime = float(GetTick());
	
	if( m_fCurTime > m_fTrailEndTime )
		m_state = READY;


	//if(m_nAdded == 0)
	//{
	//	//CUT Trail
	//	m_nRecentAccumCnt	= 0;
	//	m_nSampleTrailIndex	= 0;
	//}
	return;
}

void CFcFXSimpleTrail::PreRender()
{
	CFcFXBase::PreRender();

	int nSize = m_vecAddTrailPos[CBsObject::GetRenderBufferIndex()].size();
	if( nSize > 0 )
	{
		for( int i = 0 ; i < nSize ; ++i )
		{
			sAddTrail tmp = m_vecAddTrailPos[CBsObject::GetRenderBufferIndex()][i];
			AddTrail( tmp.vPos );
		}
		m_vecAddTrailPos[CBsObject::GetRenderBufferIndex()].clear();
	}

	//DebugString(" Trail Add per tick %d\n", m_nAdded);
	int nAdded = m_nAdded;
	if(nAdded) 
	{	
		//DebugString(" Trail Add per tick %d\n", m_nAdded);
		int nSampleIdx[3];
		int nSample = m_nSampleTrailIndex + (10-nAdded);
		int nRecentAccumCnt = m_nRecentAccumCnt;

		nSample = nSample%10;
		for(int ii = 0; ii < nAdded; ii++)
		{
			nSampleIdx[0] = nSample;
			if((m_nRecentAccumCnt-ii) >= 3)
			{
				nSampleIdx[1]=(nSample+9)%10;			// 1프로세스 전 추가된 Sample
				nSampleIdx[2]=(nSample+8)%10;			// 2프로세스 전 오래전 추가된 Sample
				//AddCatmullromTrace(nSampleIdx);
				Add(nSampleIdx);
			}
			else if((m_nRecentAccumCnt -ii) == 2) 
			{
				nSampleIdx[1]=(nSample+9)%10;			// 1프로세스 전 추가된 Sample
				nSampleIdx[2]=(nSample+9)%10;			// 1프로세스 전 추가된 Sample
				//AddCatmullromTrace(nSampleIdx);
				Add(nSampleIdx);
			}

			nSample++;
			m_nAdded--;

			if(m_nAdded != 0)
			{
				break;
			}
		}
	} 
	//else 
	//{
	//	//CUT Trail
	//	m_nRecentAccumCnt	= 0;
	//	m_nSampleTrailIndex	= 0;
	//}


	if(m_bTextureChange)
	{
		g_BsKernel.chdir("Fx");
		int nTextureId = g_BsKernel.LoadTexture( m_strTexture.c_str() );
		g_BsKernel.chdir("..");

		int ii = 0;
		for(;  ii <  m_nLoadedTexCount; ++ii)
		{
			if(nTextureId == m_pLoadedTexIDs[ii])
			{
				m_nTextureID = nTextureId;
				break;
			}
			else if ( m_pLoadedTexIDs[ii] == -1 )
			{
				m_pLoadedTexIDs[ii] = nTextureId;
				m_nTextureID = nTextureId;
				break;
			}
		}


		if(ii == m_nLoadedTexCount )
		{
			//BsAssert( 0 && "TextureID 버퍼를 늘려줘야겠어요~");
			int *pTemp = new int [m_nLoadedTexCount*2];
			memset( pTemp, -1, sizeof(int)*m_nLoadedTexCount*2 );
			memcpy(pTemp, m_pLoadedTexIDs, sizeof(int)*m_nLoadedTexCount );
			delete[] m_pLoadedTexIDs;
			m_pLoadedTexIDs = pTemp;
			
			m_pLoadedTexIDs[m_nLoadedTexCount] = nTextureId;
			m_nTextureID = nTextureId;
			m_nLoadedTexCount*=2;

			m_bTextureChange = FALSE;
		}
	}
}

void CFcFXSimpleTrail::Render(C3DDevice *pDevice)
{
	if(m_state != PLAY)
		return;

	if(m_nTextureID == -1)
	{
		if( m_nDefTextureID == -1)
            return;
		else m_nTextureID = m_nDefTextureID;
	}


	const D3DXMATRIX* matViewProj;
	matViewProj = g_BsKernel.GetParamViewProjectionMatrix();

	LPDIRECT3DDEVICE9 pD3DDevice = pDevice->GetD3DDevice();
	
	CBsMaterial* pBsMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialIndex);
	pBsMaterial->BeginMaterial(2, 0);
	pBsMaterial->BeginPass(0);
	pDevice->SetRenderState( D3DRS_BLENDOP, m_dwAlphaBlendOP);
	pDevice->SetRenderState( D3DRS_SRCBLEND, m_dwSRCBlend );
	pDevice->SetRenderState( D3DRS_DESTBLEND, m_dwDSTBlend );

	g_BsKernel.SetVertexDeclaration(m_nVertexDeclIdx);
//	pD3DDevice->SetStreamSource( 0 , m_pVB , 0 , sizeof(TRAIL_VERTEX));
	
	static bool bNew = true;
	if(bNew)
	{
		pBsMaterial->SetFloat(m_hCurTime, m_fLastUpdateTime);
		pBsMaterial->SetMatrix( m_hVP, matViewProj);

		float fAppliedLifeTime = (m_fCurTime-m_fStartTime);

		float fLife;
		if(  fAppliedLifeTime > m_fLifeTime )
			fLife = m_fLifeTime - (m_fCurTime - m_fLastUpdateTime);
		
		
		else if( fAppliedLifeTime <= 0.f)
			fLife = 1.f;
		else
			fLife = m_fLastUpdateTime - m_fStartTime;

		pBsMaterial->SetFloat(m_hLife, fLife);
		pBsMaterial->SetFloat(m_hLifeRsq, 1.f/fLife);
		float fTAlpha = BsMin( (fLife/(m_fLifeTime*0.3f)), 1.f );

		pBsMaterial->SetFloat(m_hAlpha, fTAlpha );
	}
	else
	{
		pBsMaterial->SetFloat(m_hCurTime, m_fCurTime);
	//	m_pEffect->SetFloat(m_hLife, m_fLifeTime);
		pBsMaterial->SetMatrix( m_hVP, matViewProj);
		
		float fAppliedLifeTime = (m_fCurTime-m_fStartTime);
		
		if( fAppliedLifeTime > m_fLifeTime)
		{
			pBsMaterial->SetFloat(m_hLife, m_fLifeTime);
			pBsMaterial->SetFloat(m_hLifeRsq, 1.f/m_fLifeTime);
		}
		else if( fAppliedLifeTime <= 0.f)
		{
			pBsMaterial->SetFloat(m_hLife, 1.f);
			pBsMaterial->SetFloat(m_hLifeRsq, 1.f);
		}
		else
		{
			pBsMaterial->SetFloat(m_hLife, fAppliedLifeTime);
			pBsMaterial->SetFloat(m_hLifeRsq, 1.f/fAppliedLifeTime);
		}
		pBsMaterial->SetFloat(m_hAlpha, 1.f );
	}


	pBsMaterial->SetVector(m_hColor, &m_TrailColor);

	int nBackBuffer = g_BsKernel.GetImageProcess()->GetBackBufferTextureBeforeAlphaBlendPass();
/*	if( pDevice->IsMSAAEnable() ) 
	{
		pBsMaterial->SetTexture(m_hEnvTexture, pDevice->GetTilingCompleteTexture() );//nBackBuffer
	}		
	else 
	{
		pBsMaterial->SetTexture(m_hEnvTexture, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(nBackBuffer));
	}*/

	pBsMaterial->SetTexture(m_hEnvTexture, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(nBackBuffer));

	
	pBsMaterial->SetTexture(m_hDiffTexture, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(m_nTextureID));
	pBsMaterial->SetTexture(m_hNormalTexture, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(m_nOffsetTextureID));
	pBsMaterial->CommitChanges();
//	pD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, c_TRAIL_MAX_POOL*2*3 );

#ifdef _XBOX
	BYTE *pBuffer = NULL;
	HRESULT hr = pDevice->BeginVertices( D3DPT_TRIANGLELIST, c_TRAIL_MAX_POOL*6*c_TRAIL_MAX_STEP , sizeof(TRAIL_VERTEX), (void**)&pBuffer );
	if( SUCCEEDED( hr ) )
	{
		memcpy( pBuffer, m_aBuffer, c_TRAIL_MAX_POOL*6*c_TRAIL_MAX_STEP * sizeof( TRAIL_VERTEX ) );
		pDevice->EndVertices();
	}
#endif// _XBOX

	pDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

#ifdef _XBOX
//	pDevice->SetStreamSource( 0 , NULL, 0 );
#endif

	pBsMaterial->EndPass() ;
	pBsMaterial->EndMaterial();
}

void CFcFXSimpleTrail::RenderZero(C3DDevice *pDevice)
{
	if(m_state != PLAY)
		return;


	const D3DXMATRIX* matViewProj;
	matViewProj = g_BsKernel.GetParamViewProjectionMatrix();

	LPDIRECT3DDEVICE9 pD3DDevice = pDevice->GetD3DDevice();

	CBsMaterial* pBsMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialIndex);
	pBsMaterial->BeginMaterial(1, 0);
	pBsMaterial->BeginPass(0);

	g_BsKernel.SetVertexDeclaration(m_nVertexDeclIdx);
//	pD3DDevice->SetStreamSource( 0 , m_pVB , 0 , sizeof(TRAIL_VERTEX));

	pBsMaterial->SetFloat(m_hCurTime, m_fCurTime);
	//	m_pEffect->SetFloat(m_hLife, m_fLifeTime);
	pBsMaterial->SetMatrix( m_hVP, matViewProj);

	float fAppliedLifeTime = (m_fCurTime-m_fStartTime);

	if( fAppliedLifeTime > m_fLifeTime)
	{
		pBsMaterial->SetFloat(m_hLife, m_fLifeTime);
		pBsMaterial->SetFloat(m_hLifeRsq, 1.f/m_fLifeTime);
	}
	else if( fAppliedLifeTime <= 0.f)
	{
		pBsMaterial->SetFloat(m_hLife, 1.f);
		pBsMaterial->SetFloat(m_hLifeRsq, 1.f);
	}
	else
	{
		pBsMaterial->SetFloat(m_hLife, fAppliedLifeTime);
		pBsMaterial->SetFloat(m_hLifeRsq, 1.f/fAppliedLifeTime);
	}


	pBsMaterial->SetVector(m_hColor, &m_TrailColor);

	int nBackBuffer = g_BsKernel.GetImageProcess()->GetBackBufferTextureBeforeAlphaBlendPass();
	/*	if( pDevice->IsMSAAEnable() ) 
	{
	pBsMaterial->SetTexture(m_hEnvTexture, pDevice->GetTilingCompleteTexture() );//nBackBuffer
	}		
	else 
	{
	pBsMaterial->SetTexture(m_hEnvTexture, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(nBackBuffer));
	}*/

	pBsMaterial->SetTexture(m_hEnvTexture, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(nBackBuffer));


	pBsMaterial->SetTexture(m_hDiffTexture, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(m_nTextureID));
	pBsMaterial->SetTexture(m_hNormalTexture, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(m_nOffsetTextureID));
	pBsMaterial->CommitChanges();
//	pD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, c_TRAIL_MAX_POOL*2*3 );

#ifdef _XBOX
	BYTE *pBuffer = NULL;
	HRESULT hr = pDevice->BeginVertices( D3DPT_TRIANGLELIST, c_TRAIL_MAX_POOL*6*c_TRAIL_MAX_STEP , sizeof(TRAIL_VERTEX), (void**)&pBuffer );
	if( SUCCEEDED( hr ) )
	{
		memcpy( pBuffer, m_aBuffer, c_TRAIL_MAX_POOL*6*c_TRAIL_MAX_STEP * sizeof( TRAIL_VERTEX ) );
		pDevice->EndVertices();
	}
#endif //_XBOX

#ifdef _XBOX
//		pDevice->SetStreamSource( 0 , NULL, 0 );
#endif

	pBsMaterial->EndPass() ;
	pBsMaterial->EndMaterial();
}

int CFcFXSimpleTrail::ProcessMessage(int nCode,DWORD dwParam1/*=0*/, DWORD dwParam2/*=0*/,DWORD dwParam3/*=0*/)
{	
	if ( CFcFXBase::ProcessMessage( nCode, dwParam1, dwParam2, dwParam3) )
		return 1;
	
	switch(nCode)
	{
	case FX_INIT_OBJECT:
		Initialize(/*(char*)dwParam1*/);
		return 1;
	case FX_STOP_OBJECT:
		break;
	case FX_DELETE_OBJECT:
		//DebugString("SimpleTral Deleted%n");
#ifndef _LTCG
		if( dwParam1 )
		{
			BsAssert( GetFxRtti() == (int)dwParam1 );
		}
#endif //_LTCG
		m_state=END;
		m_DeviceDataState=FXDS_RELEASEREADY;
		return 1;
	case FX_START_TRAIL:
//		StartTrail((D3DXVECTOR3*)dwParam1, *(float*)(dwParam2));
		m_nRecentAccumCnt	= 0;
		m_nSampleTrailIndex	= 0;
		m_fStartTime = float(GetTick());
		return 1;
	case FX_ADD_TRAIL:
		{
			sAddTrail tmp;
			tmp.Set((D3DXVECTOR3*)dwParam1);
			m_vecAddTrailPos[CBsObject::GetProcessBufferIndex()].push_back(tmp);
			//AddTrail((D3DXVECTOR3*)dwParam1);
		}
		return 1;
	case FX_SETCOLOR:
		SetColor(*((D3DXVECTOR3*)dwParam1));
		m_nRecentAccumCnt	= 0; // Temp
		m_nSampleTrailIndex	= 0;
		m_fStartTime = float(GetTick());
		return 1;
	case FX_SETLIFE:
		m_fLifeTime = *(float*)dwParam1;
		return 1;
	case FX_SET_DIFFTEXTURE:
//		m_strTexture = (char*)dwParam1;
//		m_bTextureChange = TRUE;
		//if(dwParam1 == ~0x0)
		//	m_nTextureID = m_nDefTextureID;
		//else
			m_nTextureID  = (int)dwParam1;
		return 1;

	case FX_SET_OFFSETTEXTURE:
		if(dwParam1 == ~0x0)
			m_nOffsetTextureID = m_nDefOffsetTextureID;
		else
			m_nOffsetTextureID  = (int)dwParam1;

		return 1;

	case FX_SET_BLENDOP:
		m_dwAlphaBlendOP = dwParam1;
		return 1;

	case FX_SET_SRCBLEND:
		m_dwSRCBlend = dwParam1;
		return 1;

	case FX_SET_DSTBLEND:
		m_dwDSTBlend = dwParam1;
		return 1;


	}
	return 0;
}


void CFcFXSimpleTrail::AddTrail(const D3DXVECTOR3* pPos)
{
	float fCreateTime=float(GetTick());
	m_SampleTrail[m_nSampleTrailIndex].vUp	=pPos[1];
	m_SampleTrail[m_nSampleTrailIndex].vDown=pPos[0];
	m_SampleTrail[m_nSampleTrailIndex].fCreateTime=fCreateTime;//+ m_fLifeTime;

	
	m_NextSample.vUp = pPos[3];
	m_NextSample.vDown = pPos[2];

//	if(m_nRecentAccumCnt == 0)
//		m_fStartTime = fCreateTime;

	m_nSampleTrailIndex++;
	m_nRecentAccumCnt++;

	m_state = PLAY;
	if(m_nSampleTrailIndex>=10) {
		m_nSampleTrailIndex = 0;
	}

	m_nAdded++;

	//DebugString("AddTrail (PROCESS)%f %d\n", fCreateTime, m_nAdded);

	// Last Trace Time 갱신
	m_fTrailEndTime = fCreateTime + m_fLifeTime;

	m_fLength = D3DXVec3Length(  &(pPos[1] - pPos[0]) );
	m_fLastUpdateTime = fCreateTime;
}

void CFcFXSimpleTrail::StartTrail(const D3DXVECTOR3* pPos, float fLife)
{
	m_nRecentAccumCnt	= 0;
	m_nSampleTrailIndex	= 0;
	m_fLifeTime			= fLife;
	m_state				= PLAY;
	AddTrail(pPos);

	m_fLength = D3DXVec3Length(  &(pPos[1] - pPos[0]) );
}

/*
void CFcFXSimpleTrail::AddLerpTrace(int* nIdx)
{
	// nIdx [0]->Cur,  [1]->1Prosess Ago
	TRAIL_VERTEX* pData;
	m_pVB->Lock(m_nPoolIdx*sizeof(TRAIL_VERTEX)*6, sizeof(TRAIL_VERTEX)*6, (void**)&pData, 0);

	D3DXVECTOR3 vSubUp, vSubDn;
	D3DXVec3Subtract( &vSubUp, &(m_SampleTrail[ nIdx[0] ].vUp), &(m_SampleTrail[ nIdx[1] ].vUp) );
	D3DXVec3Subtract( &vSubDn, &(m_SampleTrail[ nIdx[0] ].vDown), &(m_SampleTrail[ nIdx[1] ].vDown) );
	float fDot = D3DXVec3Dot( &vSubUp, &vSubDn);
	if(fDot > 0.f)
	{
		pData[0].vPos		= m_SampleTrail[ nIdx[0] ].vDown;
		pData[0].fCreateTime= m_SampleTrail[ nIdx[0] ].fCreateTime;
		pData[0].fTextureV	= 0.f;
//		pData[0].vColor		= m_TrailColor;

		pData[1].vPos		= m_SampleTrail[ nIdx[0] ].vUp;
		pData[1].fCreateTime= m_SampleTrail[ nIdx[0] ].fCreateTime;
		pData[1].fTextureV	= 1.f;
//		pData[1].vColor		= m_TrailColor;

		pData[2].vPos		= m_SampleTrail[ nIdx[1] ].vUp;
		pData[2].fCreateTime= m_SampleTrail[ nIdx[1] ].fCreateTime;
		pData[2].fTextureV	= 1.f;
//		pData[2].vColor		= m_TrailColor;

		pData[3].vPos		= m_SampleTrail[ nIdx[1] ].vUp;
		pData[3].fCreateTime= m_SampleTrail[ nIdx[1] ].fCreateTime;
		pData[3].fTextureV	= 1.f;
//		pData[3].vColor		= m_TrailColor;

		pData[4].vPos		= m_SampleTrail[ nIdx[1] ].vDown;
		pData[4].fCreateTime= m_SampleTrail[ nIdx[1] ].fCreateTime;
		pData[4].fTextureV	= 0.f;
//		pData[4].vColor		= m_TrailColor;

		pData[5].vPos		= m_SampleTrail[ nIdx[0] ].vDown;
		pData[5].fCreateTime= m_SampleTrail[ nIdx[0] ].fCreateTime;
		pData[5].fTextureV	= 0.f;
//		pData[5].vColor		= m_TrailColor;
	}
	else
	{

		pData[0].vPos		= m_SampleTrail[ nIdx[1] ].vDown;
		pData[0].fCreateTime= m_SampleTrail[ nIdx[1] ].fCreateTime;
		pData[0].fTextureV	= 0.f;
//		pData[0].vColor		= m_TrailColor;

		pData[1].vPos		= m_SampleTrail[ nIdx[0] ].vUp;
		pData[1].fCreateTime= m_SampleTrail[ nIdx[0] ].fCreateTime;
		pData[1].fTextureV	= 1.f;
//		pData[1].vColor		= m_TrailColor;

		pData[2].vPos		= m_SampleTrail[ nIdx[1] ].vUp;
		pData[2].fCreateTime= m_SampleTrail[ nIdx[1] ].fCreateTime;
		pData[2].fTextureV	= 1.f;
//		pData[2].vColor		= m_TrailColor;

		pData[3].vPos		= m_SampleTrail[ nIdx[1] ].vDown;
		pData[3].fCreateTime= m_SampleTrail[ nIdx[1] ].fCreateTime;
		pData[3].fTextureV	= 0.f;
//		pData[3].vColor		= m_TrailColor;

		pData[4].vPos		= m_SampleTrail[ nIdx[1] ].vUp;
		pData[4].fCreateTime= m_SampleTrail[ nIdx[1] ].fCreateTime;
		pData[4].fTextureV	= 1.f;
//		pData[4].vColor		= m_TrailColor;

		pData[5].vPos		= m_SampleTrail[ nIdx[0] ].vDown;
		pData[5].fCreateTime= m_SampleTrail[ nIdx[0] ].fCreateTime;
		pData[5].fTextureV	= 0.f;
//		pData[5].vColor		= m_TrailColor;
	}
	
	m_pVB->Unlock();

	++m_nPoolIdx;
	if(m_nPoolIdx>=CFcFXSimpleTrail::_MAX_POOL)
		m_nPoolIdx=0;
}
*/

void CFcFXSimpleTrail::Add(int* nIdx)
{
	float fLengthDn = D3DXVec3Length(&(m_SampleTrail[ nIdx[0] ].vUp-m_SampleTrail[ nIdx[1] ].vUp));

	int nInterpolateCount=int(fLengthDn/10.f)+1;
	if(nInterpolateCount>c_TRAIL_MAX_INTERPOLATE)
		nInterpolateCount=c_TRAIL_MAX_INTERPOLATE;


	float fIntervalTime=m_SampleTrail[ nIdx[0] ].fCreateTime-m_SampleTrail[ nIdx[1] ].fCreateTime;
	float fIntervalTerm=1.f/float(nInterpolateCount);
	float fInterpolatedIntervalTime=fIntervalTime*fIntervalTerm;

	D3DXVECTOR3 vPrevUp			=m_SampleTrail[ nIdx[1] ].vUp;
	D3DXVECTOR3 vPrevDn			=m_SampleTrail[ nIdx[1] ].vDown;
	float		fPrevCreateTime	=m_SampleTrail[ nIdx[1] ].fCreateTime;

	D3DXVECTOR3 vInterpolatedPosUp;
	D3DXVECTOR3 vInterpolatedPosDn;
	float		fInterpolatedCreateTime;

//	int nIdx = m_nBufferIdx;

	for(int ii = 0; ii < nInterpolateCount; ++ii)
	{
		D3DXVec3CatmullRom(&vInterpolatedPosUp, 
			&m_SampleTrail[ nIdx[2] ].vUp, &m_SampleTrail[ nIdx[1] ].vUp,
			&m_SampleTrail[ nIdx[0] ].vUp, &m_NextSample.vUp, 
			float(ii+1)*fIntervalTerm);

		D3DXVec3CatmullRom(&vInterpolatedPosDn, 
			&m_SampleTrail[ nIdx[2] ].vDown, &m_SampleTrail[ nIdx[1] ].vDown, 
			&m_SampleTrail[ nIdx[0] ].vDown, &m_NextSample.vDown, 
			float(ii+1)*fIntervalTerm);

		D3DXVECTOR3 vDir;
		D3DXVec3Normalize( &vDir, &(vInterpolatedPosUp - vInterpolatedPosDn) );
		vInterpolatedPosUp = vInterpolatedPosDn + (vDir*m_fLength);

		fInterpolatedCreateTime = fPrevCreateTime+fInterpolatedIntervalTime;


		D3DXVECTOR3 vInterUpSteped, vInterDnSteped, vPrevUpSteped, vPrevDnSteped;

		vInterUpSteped = vInterpolatedPosUp;
		vPrevUpSteped = vPrevUp;
		
		float fMaxStep = float(c_TRAIL_MAX_STEP);
		for(int nStep = 1; nStep <= c_TRAIL_MAX_STEP; ++nStep)
		{
			D3DXVec3Lerp(&vInterDnSteped , &vInterpolatedPosUp, &vInterpolatedPosDn, float(nStep)/fMaxStep);
			D3DXVec3Lerp(&vPrevDnSteped, &vPrevUp, &vPrevDn, float(nStep)/fMaxStep);

			m_aBuffer[m_nBufferIdx].vPos		= vInterDnSteped;
			m_aBuffer[m_nBufferIdx].fCreateTime= fInterpolatedCreateTime;
			m_aBuffer[m_nBufferIdx].fTextureV	= float(c_TRAIL_MAX_STEP-nStep)/fMaxStep;
			m_nBufferIdx++;

			m_aBuffer[m_nBufferIdx].vPos		= vInterUpSteped;
			m_aBuffer[m_nBufferIdx].fCreateTime= fInterpolatedCreateTime;
			m_aBuffer[m_nBufferIdx].fTextureV	= float(c_TRAIL_MAX_STEP-nStep+1)/fMaxStep;
			m_nBufferIdx++;

			m_aBuffer[m_nBufferIdx].vPos		= vPrevUpSteped;
			m_aBuffer[m_nBufferIdx].fCreateTime= fPrevCreateTime;
			m_aBuffer[m_nBufferIdx].fTextureV	= float(c_TRAIL_MAX_STEP-nStep+1)/fMaxStep;
			m_nBufferIdx++;

			m_aBuffer[m_nBufferIdx].vPos		= vPrevUpSteped;
			m_aBuffer[m_nBufferIdx].fCreateTime= fPrevCreateTime;
			m_aBuffer[m_nBufferIdx].fTextureV	= float(c_TRAIL_MAX_STEP-nStep+1)/fMaxStep;
			m_nBufferIdx++;

			m_aBuffer[m_nBufferIdx].vPos		= vPrevDnSteped;
			m_aBuffer[m_nBufferIdx].fCreateTime= fPrevCreateTime;
			m_aBuffer[m_nBufferIdx].fTextureV	= float(c_TRAIL_MAX_STEP-nStep)/fMaxStep;
			m_nBufferIdx++;

			m_aBuffer[m_nBufferIdx].vPos		= vInterDnSteped;
			m_aBuffer[m_nBufferIdx].fCreateTime= fInterpolatedCreateTime;
			m_aBuffer[m_nBufferIdx].fTextureV	= float(c_TRAIL_MAX_STEP-nStep)/fMaxStep;
			m_nBufferIdx++;

			vInterUpSteped = vInterDnSteped;
			vPrevUpSteped = vPrevDnSteped;

			
			if(m_nBufferIdx >= c_TRAIL_MAX_STEP*6*c_TRAIL_MAX_POOL)
				m_nBufferIdx = 0;

		}
		
		// Previous Term 갱신
		vPrevUp	=vInterpolatedPosUp;
		vPrevDn	=vInterpolatedPosDn;
		fPrevCreateTime=fInterpolatedCreateTime;
	}
}

/*
void CFcFXSimpleTrail::AddCatmullromTrace(int* nIdx)
{
	// nIdx [0]->Cur,  [1]->1Prosess Ago, [2]->2Prosess Ago

	float fLengthDn = D3DXVec3Length(&(m_SampleTrail[ nIdx[0] ].vUp-m_SampleTrail[ nIdx[1] ].vUp));

	int nInterpolateCount=int(fLengthDn/10.f)+1;
	if(nInterpolateCount>c_TRAIL_MAX_INTERPOLATE)
		nInterpolateCount=c_TRAIL_MAX_INTERPOLATE;
		

	float fIntervalTime=m_SampleTrail[ nIdx[0] ].fCreateTime-m_SampleTrail[ nIdx[1] ].fCreateTime;
	float fIntervalTerm=1.f/float(nInterpolateCount);
	float fInterpolatedIntervalTime=fIntervalTime*fIntervalTerm;

	D3DXVECTOR3 vPrevUp			=m_SampleTrail[ nIdx[1] ].vUp;
	D3DXVECTOR3 vPrevDn			=m_SampleTrail[ nIdx[1] ].vDown;
	float		fPrevCreateTime	=m_SampleTrail[ nIdx[1] ].fCreateTime;

	D3DXVECTOR3 vInterpolatedPosUp;
	D3DXVECTOR3 vInterpolatedPosDn;
	float		fInterpolatedCreateTime;

	
	TRAIL_VERTEX* pData = NULL;

	
//	D3DXVECTOR3 vSubUp, vSubDn;
//	D3DXVec3Subtract( &vSubUp, &(m_SampleTrail[ nIdx[0] ].vUp), &(m_SampleTrail[ nIdx[1] ].vUp) );
//	D3DXVec3Subtract( &vSubDn, &(m_SampleTrail[ nIdx[0] ].vDown), &(m_SampleTrail[ nIdx[1] ].vDown) );
//	float fDot = D3DXVec3Dot( &vSubUp, &vSubDn);

	int ij =0;
	


	for(;ij < nInterpolateCount;)
	{
		int nLockSize = (nInterpolateCount) - ij;
		
		
		if(m_nPoolIdx + nLockSize > c_TRAIL_MAX_POOL)
		{
			nLockSize = c_TRAIL_MAX_POOL- m_nPoolIdx;
		}
		

		m_pVB->Lock(m_nPoolIdx*sizeof(TRAIL_VERTEX)*6*c_TRAIL_MAX_STEP,sizeof(TRAIL_VERTEX)*6*3*nLockSize ,(void**)&pData, 0);

		if(pData == NULL)
		{
			DebugString("STrail Pool Idx %d, nLockSize %d\n", m_nPoolIdx, nLockSize);
		}
		
		for(int ii = ij ; ii < ij+nLockSize ; ++ii ) 
		{
			D3DXVec3CatmullRom(&vInterpolatedPosUp, 
				&m_SampleTrail[ nIdx[2] ].vUp, &m_SampleTrail[ nIdx[1] ].vUp,
				&m_SampleTrail[ nIdx[0] ].vUp, &m_NextSample.vUp, 
				float(ii+1)*fIntervalTerm);

			D3DXVec3CatmullRom(&vInterpolatedPosDn, 
				&m_SampleTrail[ nIdx[2] ].vDown, &m_SampleTrail[ nIdx[1] ].vDown, 
				&m_SampleTrail[ nIdx[0] ].vDown, &m_NextSample.vDown, 
				float(ii+1)*fIntervalTerm);

			D3DXVECTOR3 vDir;
			D3DXVec3Normalize( &vDir, &(vInterpolatedPosUp - vInterpolatedPosDn) );
			vInterpolatedPosUp = vInterpolatedPosDn + (vDir*m_fLength);

			fInterpolatedCreateTime = fPrevCreateTime+fInterpolatedIntervalTime;


			D3DXVECTOR3 vInterUpSteped, vInterDnSteped, vPrevUpSteped, vPrevDnSteped;

			vInterUpSteped = vInterpolatedPosUp;
			vPrevUpSteped = vPrevUp;

			float fMaxStep = float(c_TRAIL_MAX_STEP);
			for(int nStep = 1; nStep <= c_TRAIL_MAX_STEP; ++nStep)
			{
				D3DXVec3Lerp(&vInterDnSteped , &vInterpolatedPosUp, &vInterpolatedPosDn, float(nStep)/fMaxStep);
				D3DXVec3Lerp(&vPrevDnSteped, &vPrevUp, &vPrevDn, float(nStep)/fMaxStep);

				pData[0].vPos		= vInterDnSteped;
				pData[0].fCreateTime= fInterpolatedCreateTime;
				pData[0].fTextureV	= float(c_TRAIL_MAX_STEP-nStep)/fMaxStep;

				pData[1].vPos		= vInterUpSteped;
				pData[1].fCreateTime= fInterpolatedCreateTime;
				pData[1].fTextureV	= float(c_TRAIL_MAX_STEP-nStep+1)/fMaxStep;

				pData[2].vPos		= vPrevUpSteped;
				pData[2].fCreateTime= fPrevCreateTime;
				pData[2].fTextureV	= float(c_TRAIL_MAX_STEP-nStep+1)/fMaxStep;

				pData[3].vPos		= vPrevUpSteped;
				pData[3].fCreateTime= fPrevCreateTime;
				pData[3].fTextureV	= float(c_TRAIL_MAX_STEP-nStep+1)/fMaxStep;


				pData[4].vPos		= vPrevDnSteped;
				pData[4].fCreateTime= fPrevCreateTime;
				pData[4].fTextureV	= float(c_TRAIL_MAX_STEP-nStep)/fMaxStep;

				pData[5].vPos		= vInterDnSteped;
				pData[5].fCreateTime= fInterpolatedCreateTime;
				pData[5].fTextureV	= float(c_TRAIL_MAX_STEP-nStep)/fMaxStep;
                
				vInterUpSteped = vInterDnSteped;
				vPrevUpSteped = vPrevDnSteped;
				pData+=6;

			}

			



//			// Previous Term 갱신
			vPrevUp	=vInterpolatedPosUp;
			vPrevDn	=vInterpolatedPosDn;
			fPrevCreateTime=fInterpolatedCreateTime;
		}
		
		m_pVB->Unlock();

		ij += nLockSize;
		m_nPoolIdx += nLockSize;
		if(m_nPoolIdx  >= c_TRAIL_MAX_POOL)
		{
			m_nPoolIdx = 0;
		}
	}
}
*/
