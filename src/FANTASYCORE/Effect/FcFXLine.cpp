#include "stdafx.h"
#include "BsMaterial.h"
#include "BsKernel.h"
#include "FcGlobal.h"
#include "FcFXLine.h"
#include "FcUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

#define LINE_MAX_INTERPOLATE  5

CFcFXLineCurve::CFcFXLineCurve()
{
	m_pVertexList = NULL;

	m_nSample = 0;
	m_pSampleT = NULL;
	m_pSample = NULL;
	m_nPoint = 0;

	m_nMaterialIndex = -1;
	m_nTextureID = -1;

	m_fLineWidth = 0.f;
	m_dwStartTick = 0;
	m_nVertexCount = 0;
	m_nMAXVertexCount = 0;
	m_fTexAniSpd = 30.0f;

	m_hTexture = NULL;
	m_hCurTime = NULL;
	m_hLife = NULL;
	m_hLifeRsq = NULL;
	m_hLineWidth = NULL;
	m_hCamPos = NULL;
	m_hTexAniSpd = NULL;
	m_bUpdated = FALSE;

	m_dwAlphaBlendOP = D3DBLENDOP_ADD;
	m_dwSRCBlend	= D3DBLEND_SRCALPHA;
	m_dwDSTBlend	= D3DBLEND_INVSRCALPHA;
	m_bIsVolumeTex = false;
	m_nVertexDeclIdx = -1;
	m_bOnceTexCoord = false;

	m_fAlpha = 1.f;
#ifndef _LTCG
	SetFxRtti(FX_TYPE_LINECURVE);
#endif //_LTCG
}

CFcFXLineCurve::~CFcFXLineCurve()
{
	ReleaseDeviceData();
	if(m_pVertexList) {
		delete[] m_pVertexList;
		m_pVertexList= NULL;
	}

	if(m_pSample) {
		delete[] m_pSample;
		m_pSample= NULL;
	}

	if(m_pSampleT) {
		delete[] m_pSampleT;
		m_pSampleT= NULL;
	}
}

void	CFcFXLineCurve::Process()
{
	if(m_bUpdated) {
		m_state = PLAY;
		m_bUpdated = FALSE;
	}
	else {
		m_state = READY;
	}
}

void	CFcFXLineCurve::PreRender()
{
	CFcFXBase::PreRender();
	if(m_DeviceDataState != FXDS_INITED)
		return;
	int nBuffer = CBsObject::GetRenderBufferIndex();

	int nSize = m_vecUpdateCurve[nBuffer].size();
	if( nSize > 0 ) {
		for( int i = 0 ; i < nSize ; i++ ) {
			D3DXVECTOR3 vTmpPos[12];
			sLineCurveUpdate* tmp = m_vecUpdateCurve[CBsObject::GetRenderBufferIndex()][i];
			int nCount = tmp->_nCount;
			if( nCount > 12 ) {
				nCount = 12;
			}
			memcpy( vTmpPos, tmp->_vPos, sizeof(D3DXVECTOR3) * nCount );
			UpdatePoints( nCount, vTmpPos );
			delete[] tmp->_vPos;
			delete tmp;
		}
		m_vecUpdateCurve[CBsObject::GetRenderBufferIndex()].clear();
	}
}

void	CFcFXLineCurve::Render(C3DDevice *pDevice)
{
#ifdef _XBOX
	if( m_nVertexCount<=0 )
		return;
	const D3DXMATRIX* matViewProj;
	matViewProj = g_BsKernel.GetParamViewProjectionMatrix();

	const D3DXMATRIX* matInvView;
	matInvView = g_BsKernel.GetParamInvViewMatrix();

	LPDIRECT3DDEVICE9 pD3DDevice = pDevice->GetD3DDevice();

	CBsMaterial* pBsMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialIndex);
	pBsMaterial->BeginMaterial(0, 0);
	pBsMaterial->BeginPass(0);

	pDevice->SetRenderState( D3DRS_BLENDOP, m_dwAlphaBlendOP);
	pDevice->SetRenderState( D3DRS_SRCBLEND, m_dwSRCBlend );
	pDevice->SetRenderState( D3DRS_DESTBLEND, m_dwDSTBlend );

	g_BsKernel.SetVertexDeclaration(m_nVertexDeclIdx);

	DWORD dwCurTick = GetTick();
	float fElapse =  float(dwCurTick - m_dwStartTick);


	pBsMaterial->SetFloat(m_hLineWidth, m_fLineWidth);
	pBsMaterial->SetFloat(m_hCurTime, m_fLineLength );
	pBsMaterial->SetFloat(m_hLife, m_fLineLength);
	if(m_bOnceTexCoord)
		pBsMaterial->SetFloat(m_hLifeRsq, 1.f);
	else
		pBsMaterial->SetFloat(m_hLifeRsq, 1.f/40.f);

	pBsMaterial->SetFloat(m_hAlpha,	m_fAlpha);

	D3DXVECTOR4* vCamPos = (D3DXVECTOR4*)(&(matInvView->_41));
	pBsMaterial->SetVector(m_hCamPos, vCamPos );
	pBsMaterial->SetTexture(m_hTexture, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(m_nTextureID));

	pBsMaterial->CommitChanges();
	BYTE *pBuffer = NULL;
	HRESULT hr = pDevice->BeginVertices( D3DPT_TRIANGLESTRIP, m_nVertexCount, sizeof(RazerVertex), (void**)&pBuffer );
	if( SUCCEEDED( hr ) ) {
		memcpy( pBuffer, m_pVertexList, m_nVertexCount * sizeof( RazerVertex ) );
		pDevice->EndVertices();
	}
	else {
		BsAssert( 0 && "Failed BeginVertices()!!" );
	}

	if(m_dwAlphaBlendOP != D3DBLENDOP_ADD)
		pDevice->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD);	
	if(m_dwSRCBlend != D3DBLEND_SRCALPHA)
		pDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	if(m_dwDSTBlend != D3DBLEND_INVSRCALPHA)
		pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

	pBsMaterial->EndPass() ;
	pBsMaterial->EndMaterial();
#endif //_XBOX
}

void	CFcFXLineCurve::Initialize(int nPoint, float fEpsilon, float fLineWidth)
{
	if(nPoint < 2)
		return;

	m_fLineWidth =  fLineWidth;
	m_nPoint = nPoint;

	m_fEpsilon = fEpsilon;
	m_nMAXVertexCount = nPoint * LINE_MAX_INTERPOLATE * 2; //10 MAX INTERPOLATE
	m_DeviceDataState = FXDS_INITREADY;
	m_bIsVolumeTex = false;
}

void	CFcFXLineCurve::UpdatePoints( int nCount, D3DXVECTOR3* pvPoint)
{
	if(m_nPoint < nCount)
		return;

	if(m_nSample < m_nPoint) {
		delete[] m_pSampleT;
		m_pSampleT = NULL;
	}

	if(!m_pSampleT ) {
		m_pSampleT = new D3DXVECTOR3[m_nPoint];
		m_nSample = m_nPoint;
	}

	m_pSampleT[0] =  pvPoint[1] -  pvPoint[0];
	int ii;
	for( ii = 1; ii < nCount-1; ++ii) {
		m_pSampleT[ii] = pvPoint[ii+1] - pvPoint[ii-1];
	}

	m_pSampleT[nCount-1] =  pvPoint[nCount-1] -  pvPoint[nCount-2];

	if( !m_pVertexList ) {
		m_pVertexList = new RazerVertex[m_nMAXVertexCount];
	}

	int nVtxIdx = 2;
	m_pVertexList[0].vPoint = pvPoint[0];
	m_pVertexList[0].vTangent = m_pSampleT[0];
	m_pVertexList[0].fGenTime = 0.f;
	m_pVertexList[0].fV = 0.f;

	memcpy( &m_pVertexList[1], &m_pVertexList[0], sizeof(RazerVertex) );
	m_pVertexList[1].fV = 1.f;

	m_fLineLength = 0.f;
	m_nVertexCount = 2;
	for(ii = 1; ii< nCount; ii++) {
		float fLength = D3DXVec3Length( &(pvPoint[ii] - pvPoint[ii-1]) );

		m_fLineLength += fLength;
		int nInterCount = int(fLength/m_fEpsilon) + 1;
		nInterCount = BsMin(nInterCount, LINE_MAX_INTERPOLATE);
		m_nVertexCount += nInterCount*2;
		float fWeight, fWeightInter = 1.f/(float)nInterCount;
		float fInterTime = 1.f/( (float)(m_nPoint*nInterCount) );
		float fTime =      (float)ii/(float)m_nPoint;
		fTime += fInterTime;
		for( int ij = 1; ij <  nInterCount+1; ++ij) {
			fWeight = fWeightInter*(float)ij;
			D3DXVec3Hermite( &m_pVertexList[nVtxIdx].vPoint, &pvPoint[ii-1], &m_pSampleT[ii-1], 
				&pvPoint[ii], &m_pSampleT[ii], fWeight);

			m_pVertexList[nVtxIdx].fGenTime = fTime;
			m_pVertexList[nVtxIdx].fV = 0.f;
			fTime += fInterTime;

			memcpy( &m_pVertexList[nVtxIdx+1], &m_pVertexList[nVtxIdx], sizeof(RazerVertex) );
			m_pVertexList[nVtxIdx+1].fV = 1.f;

			nVtxIdx+=2;
		}		
	}

	if(m_bOnceTexCoord) {
		m_fLineLength = 1.0f;
	}
	if( nVtxIdx > m_nMAXVertexCount )
	{
		nVtxIdx = m_nMAXVertexCount;
	}

	for( int ii = 2 ; ii <  nVtxIdx-2; ii+=2) {
		m_pVertexList[ii].vTangent = m_pVertexList[ii+2].vPoint - m_pVertexList[ii-2].vPoint;
		m_pVertexList[ii+1].vTangent = m_pVertexList[ii].vTangent;

		m_pVertexList[ii].fGenTime *= m_fLineLength;
		m_pVertexList[ii+1].fGenTime *= m_fLineLength;
	}

	m_pVertexList[0].vTangent = m_pVertexList[2].vPoint - m_pVertexList[0].vPoint;
	m_pVertexList[1].vTangent = m_pVertexList[0].vTangent;
	m_pVertexList[0].fGenTime = 0.f;
	m_pVertexList[1].fGenTime = 0.f;

	if( nVtxIdx-4 < 0 ) {
		DebugString("BufferIndex is negative value.");
		return;
	}

	m_pVertexList[nVtxIdx-2].vTangent = m_pVertexList[nVtxIdx-2].vPoint - m_pVertexList[nVtxIdx-4].vPoint;
	m_pVertexList[nVtxIdx-1].vTangent = m_pVertexList[nVtxIdx-2].vTangent;
	m_pVertexList[nVtxIdx-2].fGenTime *= m_fLineLength;
	m_pVertexList[nVtxIdx-1].fGenTime *= m_fLineLength;
}

void	CFcFXLineCurve::InitDeviceData() 
{

	// TODO : Thread Sync문제로 Texture로드를 Message 처리 하는 부분으로 이동시킴. 확인 필요 by jeremy
//	g_BsKernel.chdir("Fx");
//	if(m_bIsVolumeTex == false ) 
//		m_nTextureID = g_BsKernel.LoadTexture(m_strTexture);
//	else 
//		m_nTextureID = g_BsKernel.LoadVolumeTexture(m_strTexture);
//	g_BsKernel.chdir("..");

	char fullName[_MAX_PATH];
	strcpy(fullName, g_BsKernel.GetShaderDirectory());
#ifdef _XBOX
	strcat(fullName, "LineTrail.fx");
#else 
	strcat(fullName, "LineTrail_PC.fx");
#endif
	m_nMaterialIndex = g_BsKernel.LoadMaterial(fullName);
	BsAssert( m_nMaterialIndex != -1  && "LineTrail Shader Compile Error!!" );

	CBsMaterial* pBsMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialIndex);
#ifdef _XBOX
	m_hTexture = pBsMaterial->GetParameterByName("Sampler");
	m_hCamPos = pBsMaterial->GetParameterByName("CamPos");
	m_hCurTime = pBsMaterial->GetParameterByName("curTime");
	m_hLife = pBsMaterial->GetParameterByName("life");
	m_hLifeRsq = pBsMaterial->GetParameterByName("lifeRsq");
	m_hLineWidth = pBsMaterial->GetParameterByName("fLineWidth");
	m_hTexAniSpd = pBsMaterial->GetParameterByName("fTexAniSpd");
	m_hAlpha = pBsMaterial->GetParameterByName("fAlpha");
#else
	m_hTexture = pBsMaterial->GetParameterByName("Texture");
	m_hCamPos = pBsMaterial->GetParameterByName("CamPos");
	m_hCurTime = pBsMaterial->GetParameterByName("curTime");
	m_hLife = pBsMaterial->GetParameterByName("life");
	m_hLifeRsq = pBsMaterial->GetParameterByName("lifeRsq");
	m_hLineWidth = pBsMaterial->GetParameterByName("fLineWidth");
	m_hTexAniSpd = pBsMaterial->GetParameterByName("fTexAniSpd");
	m_hAlpha = pBsMaterial->GetParameterByName("fAlpha");
#endif

	D3DVERTEXELEMENT9 decl[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0 },
		{ 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};

	m_nVertexDeclIdx = g_BsKernel.LoadVertexDeclaration(decl);
}

void CFcFXLineCurve::ReleaseDeviceData()
{
	SAFE_RELEASE_MATERIAL(m_nMaterialIndex);

	// TODO: 확인부탁드립니다. by jeremy
	SAFE_RELEASE_TEXTURE(m_nTextureID);

	m_bIsOrb = false;

	SAFE_RELEASE_VD(m_nVertexDeclIdx);

	return;
}


int CFcFXLineCurve::ProcessMessage(int nCode,DWORD dwParam1/*=0*/, DWORD dwParam2/*=0*/,DWORD dwParam3/*=0*/)
{	
	if ( CFcFXBase::ProcessMessage( nCode, dwParam1, dwParam2, dwParam3) )
		return 1;

	switch(nCode)
	{
	case FX_INIT_OBJECT:
		Initialize( (int)dwParam1, *(float*)dwParam2, *(float*)dwParam3 );
		break;
	case FX_PLAY_OBJECT:
		m_state = PLAY;
		break;
	case FX_STOP_OBJECT:
		m_state = READY;
		break;
	case FX_UPDATE_OBJECT:
		{
			sLineCurveUpdate* tmp = new sLineCurveUpdate;
			tmp->_nCount = (int)dwParam1;
			tmp->_vPos = new D3DXVECTOR3[12];
			memcpy( tmp->_vPos, (D3DXVECTOR3*)dwParam2, sizeof(D3DXVECTOR3)*tmp->_nCount );
			int nBuffer = CBsObject::GetProcessBufferIndex();
			m_vecUpdateCurve[nBuffer].push_back(tmp);
			m_bUpdated = TRUE;
			m_state = PLAY;
		}
		break;
	case FX_SET_TEXTURE:
		{
			BsAssert( m_nTextureID == -1 && "Invalid FX Message!!" );
			strcpy( m_strTexture, (char*)dwParam1); //aleksger - prefix bug 729 - more robust string handling.
			g_BsKernel.chdir("Fx");
			if(m_bIsVolumeTex == false ) 
				m_nTextureID = g_BsKernel.LoadTexture(m_strTexture);
			else 
				m_nTextureID = g_BsKernel.LoadVolumeTexture(m_strTexture);
			g_BsKernel.chdir("..");

			if( dwParam2 != 0 )
				m_bOnceTexCoord = true;
		}
		break;
	case FX_SET_BLENDOP:
		m_dwAlphaBlendOP = dwParam1;
		break;
	case FX_SET_SRCBLEND:
		m_dwSRCBlend = dwParam1;
		break;
	case FX_SET_DSTBLEND:
		m_dwDSTBlend = dwParam1;
		break;
	case FX_SET_ALPHA:
		m_fAlpha = *(float*)dwParam1;
		break;
	}
	return 0;
}






CFcFXLineTrail::CFcFXLineTrail()
{
	m_dwEndTime		= 0;
	m_nQueueBeginIdx = -1;
	m_nQueueEndIdx  = -1;

	m_dwSRCBlend = D3DBLEND_SRCCOLOR;
	m_dwDSTBlend = D3DBLEND_ONE;
	m_bIsVolumeTex = false;
	m_bIsOrb = false;
#ifndef _LTCG
	SetFxRtti(FX_TYPE_LINETRAIL);
#endif //_LTCG
}

void CFcFXLineTrail::Initialize( int nLife, float fLineWidth, float fEpsilon)
{
	m_fLife = (float)nLife;
	m_fLineWidth =  fLineWidth;
	m_dwStartTick = GetTick();


	m_fLineWidth =  fLineWidth*10.f;
	m_fEpsilon = fEpsilon;
	m_nMAXVertexCount = nLife * LINE_MAX_INTERPOLATE * 2*2; //10 MAX INTERPOLATE
	m_DeviceDataState = FXDS_INITREADY;

	if(!m_pVertexList) {
		m_pVertexList = new RazerVertex[nLife * LINE_MAX_INTERPOLATE*2];
	}
	else {
		BsAssert( 0 && "Invalid data!!");
	}

	if(!m_pSample) {
		if(m_pSampleT) {
			delete[] m_pSampleT;
			m_pSampleT = NULL;
		}
	}

	m_pSample = new D3DXVECTOR3[int(m_fLife)];
	m_pSampleT = new D3DXVECTOR3[int(m_fLife)];
}

void CFcFXLineTrail::UpdatePoint( D3DXVECTOR3* vPos, D3DXVECTOR3* vTngt)
{
	m_nQueueEndIdx++;
	if(m_nQueueEndIdx >= int(m_fLife))
		m_nQueueEndIdx = 0;

	if(m_nQueueEndIdx == m_nQueueBeginIdx)
		m_nQueueBeginIdx++;

	if( m_nQueueBeginIdx >= int(m_fLife) )
		m_nQueueBeginIdx = 0;

	if(m_nQueueBeginIdx == -1)
		m_nQueueBeginIdx = 0;


	m_pSample[m_nQueueEndIdx] = *vPos;
	if(vTngt) {
		m_pSampleT[m_nQueueEndIdx] = *vTngt;
	}
	else {
		if(m_nSample>=1) {
			int nLastIdx = m_nQueueEndIdx-1;
			if( nLastIdx < 0 ) {
				nLastIdx = int(m_fLife) + nLastIdx;
			}
			m_pSampleT[m_nQueueEndIdx] = *vPos-m_pSample[nLastIdx];
		}
		else {
			m_pSampleT[m_nQueueEndIdx] = D3DXVECTOR3(0.f, 0.f, 1.f );
		}
	}

	if(m_nSample < int(m_fLife))
		m_nSample++;


	DWORD dwCurTick = GetTick();
	m_dwEndTime = dwCurTick + (DWORD)m_fLife;
	m_dwLastUpdateTick = dwCurTick; 
}


int CFcFXLineTrail::ProcessMessage(int nCode,DWORD dwParam1, DWORD dwParam2,DWORD dwParam3)
{	
	if ( CFcFXBase::ProcessMessage( nCode, dwParam1, dwParam2, dwParam3) )
		return 1;
	switch(nCode)
	{
	case FX_INIT_OBJECT:
		Initialize( (int)dwParam1, *(float*)dwParam2, *(float*)dwParam3);
		break;
	case FX_PLAY_OBJECT:
		m_state = PLAY;
		break;
	case FX_STOP_OBJECT:
		m_state = READY;
		break;
	case FX_SET_TEXTURE:
		{
			BsAssert( m_nTextureID == -1 && "Invalid FX Message!!" );
			SAFE_RELEASE_TEXTURE(m_nTextureID);
			strcpy( m_strTexture, (char*)dwParam1); //aleksger - prefix bug 730 - more robust string handling.
			g_BsKernel.chdir("Fx");
			if(m_bIsVolumeTex == false ) 
				m_nTextureID = g_BsKernel.LoadTexture(m_strTexture);
			else 
				m_nTextureID = g_BsKernel.LoadVolumeTexture(m_strTexture);
			g_BsKernel.chdir("..");
		}
		break;
	case FX_SET_VOLUME_TEXTURE:
		{
			BsAssert( m_nTextureID == -1 && "Invalid FX Message!!" );
			SAFE_RELEASE_TEXTURE(m_nTextureID);
			strcpy( m_strTexture, (char*)dwParam1);//aleksger - prefix bug 730 - more robust string handling.
			m_fTexAniSpd = (float)dwParam2;
			m_bIsVolumeTex = true;
			if( dwParam3 == 0 )
				m_bIsOrb = false;
			else
				m_bIsOrb = true;
			g_BsKernel.chdir("Fx");
			if(m_bIsVolumeTex == false ) 
				m_nTextureID = g_BsKernel.LoadTexture(m_strTexture);
			else 
				m_nTextureID = g_BsKernel.LoadVolumeTexture(m_strTexture);
			g_BsKernel.chdir("..");
		}
		break;
	case FX_UPDATE_OBJECT:
		UpdatePoint( (D3DXVECTOR3*)dwParam1, (D3DXVECTOR3*)dwParam2 );
		m_bUpdated = TRUE;
		m_state = PLAY;
		break;
	case FX_GET_POS:
		{
			D3DXVECTOR3** pPos = (D3DXVECTOR3**)dwParam2;
			if(m_nSample < (int)dwParam1) {
				*pPos = NULL;
				break;
			}
			int nIdx = m_nQueueEndIdx - (int)dwParam1;
			if((nIdx < 0)) {
				*pPos = &m_pSample[m_nSample-nIdx];
				break;
			}
			else {
				*pPos = &m_pSample[nIdx];
				break;
			}
		}
	case FX_SET_BLENDOP:
		m_dwAlphaBlendOP = dwParam1;
		break;
	case FX_SET_SRCBLEND:
		m_dwSRCBlend = dwParam1;
		break;
	case FX_SET_DSTBLEND:
		m_dwDSTBlend = dwParam1;
		break;
	case FX_SET_ALPHA:
		m_fAlpha = *(float*)dwParam1;
		break;
	}
	return 0;
}

void CFcFXLineTrail::Render(C3DDevice *pDevice)
{
#ifdef _XBOX
	if( m_nVertexCount <= 2)
		return;
	const D3DXMATRIX* matViewProj;
	matViewProj = g_BsKernel.GetParamViewProjectionMatrix();

	const D3DXMATRIX* matInvView;
	matInvView = g_BsKernel.GetParamInvViewMatrix();

	LPDIRECT3DDEVICE9 pD3DDevice = pDevice->GetD3DDevice();

	CBsMaterial* pBsMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialIndex);
	
	if( !m_bIsVolumeTex )
		pBsMaterial->BeginMaterial(0, 0);
	else
		pBsMaterial->BeginMaterial(1, 0);
	pBsMaterial->BeginPass(0);

	pDevice->SetRenderState( D3DRS_BLENDOP, m_dwAlphaBlendOP);			//D3DBLENDOP_ADD
	pDevice->SetRenderState( D3DRS_SRCBLEND, m_dwSRCBlend );			//D3DBLEND_SRCALPHA
	pDevice->SetRenderState( D3DRS_DESTBLEND, m_dwDSTBlend );			//D3DBLEND_INVSRCALPHA

	g_BsKernel.SetVertexDeclaration(m_nVertexDeclIdx);

	DWORD dwCurTick = GetTick();
	float fElapse =  float(dwCurTick - m_dwStartTick);

	pBsMaterial->SetFloat(m_hLineWidth, m_fLineWidth);
	float fLife	= m_fLife - float(dwCurTick - m_dwLastUpdateTick);
	pBsMaterial->SetFloat(m_hCurTime, (float)m_dwLastUpdateTick );
	if( fElapse > fLife) {
		pBsMaterial->SetFloat(m_hLife, fLife);
		pBsMaterial->SetFloat(m_hLifeRsq, 1.f/fLife);
	}
	else if( fElapse <= 0.f) {
		pBsMaterial->SetFloat(m_hLife, 1.f);
		pBsMaterial->SetFloat(m_hLifeRsq, 1.f);
	}
	else {
		pBsMaterial->SetFloat(m_hLife, fElapse);
		pBsMaterial->SetFloat(m_hLifeRsq, 1.f/fElapse);
	}

	pBsMaterial->SetFloat(m_hAlpha,	m_fAlpha);

	D3DXVECTOR4* vCamPos = (D3DXVECTOR4*)(&(matInvView->_41));
	pBsMaterial->SetVector(m_hCamPos, vCamPos );//(LPDIRECT3DBASETEXTURE9)(g_BsKernel.GetTexturePtr
	if( !m_bIsVolumeTex ) {
        pBsMaterial->SetTexture(m_hTexture, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(m_nTextureID));
	}
	else {
		pBsMaterial->SetFloat(m_hTexAniSpd, m_fTexAniSpd);
		LPDIRECT3DVOLUMETEXTURE9 ptmpTexture = NULL;
		ptmpTexture = (LPDIRECT3DVOLUMETEXTURE9)(g_BsKernel.GetTexturePtr(m_nTextureID));
		pBsMaterial->SetTexture(m_hTexture, ptmpTexture);
	}

	pBsMaterial->CommitChanges();

	BYTE *pBuffer = NULL;
	HRESULT hr = pDevice->BeginVertices( D3DPT_TRIANGLESTRIP, m_nVertexCount, sizeof(RazerVertex), (void**)&pBuffer );
	if( SUCCEEDED( hr ) ) {
		memcpy( pBuffer, m_pVertexList, m_nVertexCount * sizeof( RazerVertex ) );
		pDevice->EndVertices();
	}

	if(m_dwAlphaBlendOP != D3DBLENDOP_ADD)
		pDevice->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD);	
	if(m_dwSRCBlend != D3DBLEND_SRCALPHA)
		pDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	if(m_dwDSTBlend != D3DBLEND_INVSRCALPHA)
		pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

	pBsMaterial->EndPass() ;
	pBsMaterial->EndMaterial();
#endif //_XBOX
}

void CFcFXLineTrail::PreRender()
{
	CFcFXBase::PreRender();
	if(m_DeviceDataState != FXDS_INITED)
		return;

	if(!m_bUpdated)
		return;

	m_bUpdated = FALSE;

	DWORD dwCurTick = GetTick();

	if(m_nSample == 1) {
		return;
	}

	m_nVertexCount = 0;
	int nNextIdx, nCurIdx = m_nQueueBeginIdx;
	nNextIdx = nCurIdx+1;
	if(nNextIdx >= m_nSample)
		nNextIdx = 0;

	dwCurTick -= (m_nSample-1);
	m_pVertexList[m_nVertexCount].vPoint = m_pSample[nCurIdx];
	m_pVertexList[m_nVertexCount].fGenTime = (float)dwCurTick;
	m_pVertexList[m_nVertexCount].fV = 0.f;

	memcpy( &m_pVertexList[m_nVertexCount+1], &m_pVertexList[m_nVertexCount], sizeof(RazerVertex) );
	m_pVertexList[m_nVertexCount+1].fV = 1.f;
	m_nVertexCount+=2;

	for(int ii = 0; ii < m_nSample-1; ++ii) {
 		float fLength = D3DXVec3Length( &( m_pSample[nNextIdx] - m_pSample[nCurIdx]) );
		int nInterCount = int(fLength/m_fEpsilon) + 1;
		nInterCount = BsMin(nInterCount, LINE_MAX_INTERPOLATE );// LINE_MAX_INTERPOLATE);

		float fWeight, fWeightInter = 1.f/(float)nInterCount;
		float fInterTime = 1.f/( (float)(nInterCount) );
		float fTime = float(dwCurTick+ii)+fInterTime;

		for( int ij = 1; ij <= nInterCount; ++ij) {
			fWeight = fWeightInter*(float)ij;
			D3DXVec3Hermite( &m_pVertexList[m_nVertexCount].vPoint , 
				&m_pSample[nCurIdx], &m_pSampleT[nCurIdx],
				&m_pSample[nNextIdx], &m_pSampleT[nNextIdx], fWeight);

			m_pVertexList[m_nVertexCount].fGenTime = fTime;
			m_pVertexList[m_nVertexCount].fV = 0.f;
			fTime += fInterTime;

			memcpy( &m_pVertexList[m_nVertexCount+1], &m_pVertexList[m_nVertexCount], sizeof(RazerVertex) );
			m_pVertexList[m_nVertexCount+1].fV = 1.f;
 			m_nVertexCount+=2;
		}

		nCurIdx++;
		if(nCurIdx >= m_nSample) 
			nCurIdx = 0;

		nNextIdx = nCurIdx+1;
		if(nNextIdx >= m_nSample)
			nNextIdx = 0;
	}	

	for( int ii = 2 ; ii <  m_nVertexCount-2; ii+=2) {
		m_pVertexList[ii].vTangent = m_pVertexList[ii+2].vPoint - m_pVertexList[ii-2].vPoint;
		m_pVertexList[ii+1].vTangent = m_pVertexList[ii].vTangent;
	}

	if( m_nVertexCount > 2) {
		m_pVertexList[0].vTangent = m_pVertexList[2].vPoint - m_pVertexList[0].vPoint;
		m_pVertexList[1].vTangent = m_pVertexList[0].vTangent;

		m_pVertexList[m_nVertexCount-2].vTangent = m_pVertexList[m_nVertexCount-2].vPoint - m_pVertexList[m_nVertexCount-4].vPoint;
		m_pVertexList[m_nVertexCount-1].vTangent = m_pVertexList[m_nVertexCount-2].vTangent;
	}
}

void	CFcFXLineTrail::InitDeviceData()
{
	CFcFXLineCurve::InitDeviceData();
}

void	CFcFXLineTrail::ReleaseDeviceData()
{
	CFcFXLineCurve::ReleaseDeviceData();
}

CFcFXBillboard::CFcFXBillboard()
{
	m_nTextureID	= -1;
	m_nMaterialIndex= -1;
	m_nVBWriteIdx	= 0; 
	m_nMaxBillboard = 0;
	m_nCurBoardCount= 0;

	m_dwAlphaBlendOP = D3DBLENDOP_ADD;
	m_dwSRCBlend	= D3DBLEND_SRCALPHA;
	m_dwDSTBlend	= D3DBLEND_INVSRCALPHA;
	m_dwStartTick = 0;

	m_fBillboardSize =0.f;
	m_fBillboardLife =0.f;
	m_nTmpBuffer	 = 0;
	m_nVertexDeclIdx = -1;
	m_fAlpha = 1.f;
	m_pBBoard = NULL;

#ifndef _LTCG
	SetFxRtti(FX_TYPE_BBOARD);
#endif //_LTCG
}

CFcFXBillboard::~CFcFXBillboard()
{
	ReleaseDeviceData();
}


void	CFcFXBillboard::Process()
{
}

void	CFcFXBillboard::PreRender()
{
	CFcFXBase::PreRender();
	if(m_DeviceDataState != FXDS_INITED)
		return;

	float fCurTick = (float)GetTick();

	m_nTmpBuffer = 0;

	int nSize = m_vecAddBillborad[CBsObject::GetRenderBufferIndex()].size();
	if( nSize > 0 )
	{
		for( int i = 0 ; i < nSize ; ++i )
		{
			D3DXVECTOR3 vPos = m_vecAddBillborad[CBsObject::GetRenderBufferIndex()][i];
			AddBillboard( &vPos );
		}
		m_vecAddBillborad[CBsObject::GetRenderBufferIndex()].clear();
	}
	
	if( ((m_nVBWriteIdx/6) + m_nTmpBuffer) > m_nMaxBillboard)
		m_nTmpBuffer = m_nMaxBillboard - m_nVBWriteIdx/6;
	
	if(m_nTmpBuffer) {
		for(int ii = 0; ii < m_nTmpBuffer ; ii++) {
			m_pBBoard[ii*6].vPos		= m_vTmpBuffer[ii];
			m_pBBoard[ii*6].vTexcoord	= D3DXVECTOR3(0.f, 0.f, fCurTick);
			m_pBBoard[ii*6+1].vPos		= m_vTmpBuffer[ii];
			m_pBBoard[ii*6+1].vTexcoord	= D3DXVECTOR3(1.f, 0.f, fCurTick);
			m_pBBoard[ii*6+2].vPos		= m_vTmpBuffer[ii];
			m_pBBoard[ii*6+2].vTexcoord	= D3DXVECTOR3(0.f, 1.f, fCurTick);
			m_pBBoard[ii*6+3].vPos		= m_vTmpBuffer[ii];
			m_pBBoard[ii*6+3].vTexcoord	= D3DXVECTOR3(0.f, 1.f, fCurTick);
			m_pBBoard[ii*6+4].vPos		= m_vTmpBuffer[ii];
			m_pBBoard[ii*6+4].vTexcoord	= D3DXVECTOR3(1.f, 0.f, fCurTick);
			m_pBBoard[ii*6+5].vPos		= m_vTmpBuffer[ii];
			m_pBBoard[ii*6+5].vTexcoord	= D3DXVECTOR3(1.f, 1.f, fCurTick);
		}

		m_nVBWriteIdx += 6*m_nTmpBuffer;
		
		if( (m_nVBWriteIdx/6) >= m_nMaxBillboard)
			m_nVBWriteIdx = 0;
	}
}

void	CFcFXBillboard::Render(C3DDevice *pDevice)
{
#ifdef _XBOX
	if(m_DeviceDataState != FXDS_INITED)
		return;
	if(m_nCurBoardCount <= 0)
		return;

	const D3DXMATRIX* matViewProj;
	matViewProj = g_BsKernel.GetParamViewProjectionMatrix();

	const D3DXMATRIX* matInvView;
	matInvView = g_BsKernel.GetParamInvViewMatrix();

	LPDIRECT3DDEVICE9 pD3DDevice = pDevice->GetD3DDevice();

	CBsMaterial* pBsMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialIndex);
	pBsMaterial->BeginMaterial(0, 0);
	pBsMaterial->BeginPass(0);

	pDevice->SetRenderState( D3DRS_BLENDOP, m_dwAlphaBlendOP);

	pDevice->SetRenderState( D3DRS_SRCBLEND, m_dwSRCBlend );
	pDevice->SetRenderState( D3DRS_DESTBLEND, m_dwDSTBlend );

	g_BsKernel.SetVertexDeclaration(m_nVertexDeclIdx);

	DWORD dwCurTick = GetTick();

	pBsMaterial->SetFloat(m_hRadius, m_fBillboardSize);
	pBsMaterial->SetFloat(m_hCurTime,	(float)dwCurTick );
	pBsMaterial->SetFloat(m_hLife,		m_fBillboardLife);
	pBsMaterial->SetFloat(m_hLifeRsq,	1.f/m_fBillboardLife);

	D3DXVECTOR4* vCamAxis = (D3DXVECTOR4*)(&(matInvView->_11));
	D3DXVECTOR4 vNCamAxis;
	pBsMaterial->SetVector(m_hCamAxisX, vCamAxis );

	vCamAxis = (D3DXVECTOR4*)(&(matInvView->_21));
	pBsMaterial->SetVector(m_hCamAxisY, vCamAxis );
	pBsMaterial->SetFloat(m_hAlpha, m_fAlpha );

	pBsMaterial->SetTexture(m_hTexture, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(m_nTextureID));

	pBsMaterial->CommitChanges();

	BYTE *pBuffer = NULL;
	if(m_nTmpBuffer) {
		// 빨랑 고쳐주세요. Thread Sync 문제 발생!!
		HRESULT hr = pDevice->BeginVertices( D3DPT_TRIANGLELIST, 6*m_nTmpBuffer, sizeof(FXBBoardVtx), (void**)&pBuffer );
		if( SUCCEEDED( hr ) ) {
			memcpy(pBuffer, m_pBBoard, sizeof(FXBBoardVtx)*6*m_nTmpBuffer );
			pDevice->EndVertices();
		}
		else {
			BsAssert( 0 && "Failed BegineVertices!!" );
		}
	}

	if(m_dwAlphaBlendOP != D3DBLENDOP_ADD)
		pDevice->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD);	
	if(m_dwSRCBlend != D3DBLEND_SRCALPHA)
		pDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	if(m_dwDSTBlend != D3DBLEND_INVSRCALPHA)
		pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

	pBsMaterial->EndPass();
	pBsMaterial->EndMaterial();
#endif //_XBOX
}

void	CFcFXBillboard::Initialize(int nMaxBillboard, float fSize, float fLife )
{
	m_nMaxBillboard = nMaxBillboard;
	m_fBillboardLife = fLife;
	m_fBillboardSize = fSize;
	m_DeviceDataState = FXDS_INITREADY;
}

void	CFcFXBillboard::AddBillboard(D3DXVECTOR3* vPos)
{
	if(m_nTmpBuffer >= 100)
		return;

	m_vTmpBuffer[m_nTmpBuffer] = *vPos;
	m_nTmpBuffer++;
	if(m_nCurBoardCount < m_nMaxBillboard)
		m_nCurBoardCount++;
}

int		CFcFXBillboard::ProcessMessage(int nCode,DWORD dwParam1 , DWORD dwParam2 ,DWORD dwParam3 )
{
	if( CFcFXBase::ProcessMessage( nCode, dwParam1, dwParam2, dwParam3) )
		return 1;

	switch(nCode)
	{
	case FX_INIT_OBJECT:
		Initialize( (int)dwParam1, *(float*)dwParam2, *(float*)dwParam3 );
		break;
	case FX_PLAY_OBJECT:
		m_state = PLAY;
		break;
	case FX_STOP_OBJECT:
		m_state = READY;
		break;
	case FX_SET_TEXTURE:
		{
			BsAssert( m_nTextureID == -1 && "Invalid FX Message!!" );
			SAFE_RELEASE_TEXTURE(m_nTextureID);
			strcpy( m_strTexture, (char*)dwParam1); //aleksger - prefix bug 731 - more robust string handling.
			g_BsKernel.chdir("Fx");
			m_nTextureID = g_BsKernel.LoadVolumeTexture(m_strTexture);
			g_BsKernel.chdir("..");
		}
		break;
	case FX_UPDATE_OBJECT:
		{
			D3DXVECTOR3 vPos = *(D3DXVECTOR3*)dwParam1;
			m_vecAddBillborad[CBsObject::GetProcessBufferIndex()].push_back(vPos);
		}
		break;
	case FX_SET_BLENDOP:
		m_dwAlphaBlendOP = dwParam1;
		break;
	case FX_SET_SRCBLEND:
		m_dwSRCBlend = dwParam1;
		break;
	case FX_SET_DSTBLEND:
		m_dwDSTBlend = dwParam1;
		break;
	case FX_SET_ALPHA:
		m_fAlpha = *(float*)dwParam1;
		break;
	}
	return 0;
}

void CFcFXBillboard::InitDeviceData()
{
	// TODO : Thread Sync문제로 Texture로드를 Message 처리 하는 부분으로 이동시킴. 확인 필요 by jeremy
//	g_BsKernel.chdir("Fx");
//	m_nTextureID = g_BsKernel.LoadVolumeTexture(m_strTexture);
//	g_BsKernel.chdir("..");

	char fullName[_MAX_PATH];
	strcpy(fullName, g_BsKernel.GetShaderDirectory());
	strcat(fullName, "BBoard.fx");
	m_nMaterialIndex = g_BsKernel.LoadMaterial(fullName);
	BsAssert( m_nMaterialIndex != -1  && "FxBBoard Shader Compile Error!!" );

	CBsMaterial* pBsMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialIndex);
#ifdef _XBOX
	m_hTexture = pBsMaterial->GetParameterByName("Sampler");
	m_hCamAxisX = pBsMaterial->GetParameterByName("CamAxisX");
	m_hCamAxisY = pBsMaterial->GetParameterByName("CamAxisY");
	m_hCurTime = pBsMaterial->GetParameterByName("curTime");
	m_hLife = pBsMaterial->GetParameterByName("life");
	m_hLifeRsq = pBsMaterial->GetParameterByName("lifeRsq");
	m_hRadius = pBsMaterial->GetParameterByName("fRadius");
	m_hAlpha = pBsMaterial->GetParameterByName("fAlpha");
#else
	m_hTexture = pBsMaterial->GetParameterByName("Texture");
	m_hCamAxisX = pBsMaterial->GetParameterByName("CamAxisX");
	m_hCamAxisY = pBsMaterial->GetParameterByName("CamAxisY");
	m_hCurTime = pBsMaterial->GetParameterByName("curTime");
	m_hLife = pBsMaterial->GetParameterByName("life");
	m_hLifeRsq = pBsMaterial->GetParameterByName("lifeRsq");
	m_hRadius = pBsMaterial->GetParameterByName("fRadius");
	m_hAlpha = pBsMaterial->GetParameterByName("fAlpha");
#endif

	D3DVERTEXELEMENT9 decl[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};

	m_nVertexDeclIdx = g_BsKernel.LoadVertexDeclaration(decl);

	m_pBBoard = new FXBBoardVtx[6*100];
}

void CFcFXBillboard::ReleaseDeviceData()
{
	SAFE_RELEASE_MATERIAL(m_nMaterialIndex);

	SAFE_RELEASE_TEXTURE(m_nTextureID);

	SAFE_RELEASE_VD(m_nVertexDeclIdx);

	SAFE_DELETEA(m_pBBoard);
}