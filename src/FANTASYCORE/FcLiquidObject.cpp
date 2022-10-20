#include "stdafx.h"
#include "FCUtil.h"
#include "BsCommon.h"
#include "BsMesh.h"
#include "BsImageProcess.h"
#include "FcLiquidObject.h"
#include "CrossVector.h"
#include "FcBaseObject.h"
#include "FcGameObject.h"
#include "BsParticle.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


#define		_BEFORE_CAPTURE

inline	void D3DXVec3ToVec4(D3DXVECTOR4 *pV4, D3DXVECTOR3 *pV3, float fW)
{
	pV4->x = pV3->x;
	pV4->y = pV3->y;
	pV4->z = pV3->z;
	pV4->w = fW;
}

CFcLiquidObject::CFcLiquidObject(CCrossVector *pCross)
	: CFcGameObject( pCross )
{
	m_nV = 0;
	
	m_pV = NULL;
	m_pN = NULL;
	m_pB = NULL;

	m_pVT = NULL;
	m_pNT = NULL;

	m_pPL = NULL;
	m_pPI = NULL;

	m_bInit = false;
	m_bGather = false;

	m_fTime = 0.0f;
	m_nVertexDeclIdx = -1;

	g_BsKernel.chdir("material");
 	m_nChunkTex = g_BsKernel.LoadVolumeTexture("waterChunk.dds");
	m_nChunkTex2 = g_BsKernel.LoadVolumeTexture("waterChunk2.dds");
	m_nSpecTex = g_BsKernel.LoadVolumeTexture("waterSpec.dds");
	
	g_BsKernel.chdir("..");

	char fullName[_MAX_PATH];
	strcpy(fullName, g_BsKernel.GetShaderDirectory());
	strcat(fullName, "LiquidChunk.fx");

	HRESULT hr = g_BsKernel.GetDevice()->CreateEffectFromFile(fullName, &m_pEffect);
	if(hr != S_OK) {
		BsAssert( 0 && "FakeLiquid Shader Compile Error!!" );
	}

	D3DVERTEXELEMENT9 decl[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 16, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0, 32, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		{ 0, 44, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
		D3DDECL_END()
	};


	m_nVertexDeclIdx = g_BsKernel.LoadVertexDeclaration( decl );
}
    
CFcLiquidObject::~CFcLiquidObject()
{
	if (m_pV != NULL)
		free (m_pV);

	if (m_pN != NULL)
		free (m_pN);

	if (m_pB != NULL)
		free (m_pB);

	if (m_pVT != NULL)
		free (m_pVT);

	if (m_pNT != NULL)
		free (m_pNT);

	if (m_pPL != NULL)
		free (m_pPL);

	if (m_pPI != NULL)
		free (m_pPI);


	if( m_pVB != NULL )
	{
		m_pVB->Release();
		m_pVB = NULL;
	}

	if( m_pIB != NULL )
	{
		m_pIB->Release();
		m_pIB = NULL;
	}


	if(m_pEffect) {
		m_pEffect->Release();
		m_pEffect = NULL;
	}

	if(m_nVertexDeclIdx != -1)
	{
		g_BsKernel.ReleaseVertexDeclaration(m_nVertexDeclIdx);
		m_nVertexDeclIdx = -1;
	}

}

void CFcLiquidObject::PostProcess()
{
	float fLandHeight = g_BsKernel.GetLandHeight( m_Cross.m_PosVector.x, m_Cross.m_PosVector.z );
	m_Cross.m_PosVector.y=fLandHeight;
}

void CFcLiquidObject::Initialize( INITGAMEOBJECTDATA* pData, int nForce, int nTeam, TroopObjHandle hTroop, float fScaleMin, float fScaleMax )
{
	int	nScan, nDebug;

	CFcGameObject::Initialize( pData, nForce, nTeam, hTroop, fScaleMin, fScaleMax );
	
	g_BsKernel.SendMessage( m_nEngineIndex, BS_SET_CUSTOM_RENDERER, (DWORD)CustomRender, (DWORD)this );

	g_BsKernel.SendMessage( m_nEngineIndex, BS_SHOW_SUBMESH, 0, 0 );
	g_BsKernel.SendMessage( m_WeaponList[ m_nCurrentWeaponIndex ].nObjectIndex, BS_SHOW_SUBMESH, 0, 0 );


	for( nScan = 0; nScan < ( signed )m_ArmorList.size(); nScan++ )
	{
		nDebug = m_ArmorList[ nScan ].nObjectIndex;
		g_BsKernel.SendMessage( nDebug, BS_SHOW_SUBMESH, 0, 0 );
	}
	for( nScan = 0; nScan < ( signed )m_WeaponList.size(); nScan++ )
	{
		nDebug = m_WeaponList[ nScan ].nObjectIndex;
		g_BsKernel.SendMessage( nDebug, BS_SHOW_SUBMESH, 0, 0 );
	}


	g_BsKernel.chdir("particle");
	m_nPGId = g_BsKernel.LoadParticleGroup(-1,"waterChunk.bpf");
	g_BsKernel.chdir("..");

	g_BsKernel.chdir("char/LiquidTest");
	m_nMeshId = g_BsKernel.LoadMesh(-1,"LiquidTest.BM");
	g_BsKernel.chdir("..");

	CBsMesh *pM = g_BsKernel.Get_pMesh(m_nMeshId);

	pM->GetStreamBufferExt(&m_pV, &m_pN, &m_pB, m_nV);

	m_pVT = (D3DXVECTOR3*) malloc(sizeof(D3DXVECTOR3) * m_nV);
	m_pNT = (D3DXVECTOR3*) malloc(sizeof(D3DXVECTOR3) * m_nV);

	m_pPL = (int*) malloc (sizeof(int) * m_nV * PTCTRAIL_MAX);
	m_pPI = (PTCTRAIL*) malloc (sizeof (PTCTRAIL) * m_nV);

	for(nScan = 0;nScan < m_nV; nScan++)
	{
		m_pVT[nScan] = m_pV[nScan] + m_Cross.m_PosVector;
		m_pNT[nScan] = m_pN[nScan];

		m_pPI[nScan].Pos[0] = m_pVT[nScan];
		m_pPI[nScan].Pos[0].y = g_BsKernel.GetLandHeight( m_Cross.m_PosVector.x, m_Cross.m_PosVector.z ) - 500.0f;
		m_pPI[nScan].fAlpha = 0.9f;

		for(int nScan2 = 1;nScan2 < PTCTRAIL_MAX;nScan2++)
			m_pPI[nScan].Pos[nScan2] = m_pPI[nScan].Pos[0];

		memset (&m_pPI[nScan].Acc,0,sizeof(D3DXVECTOR3)*PTCTRAIL_MAX);
		memset (&m_pPI[nScan].Vel,0,sizeof(D3DXVECTOR3)*PTCTRAIL_MAX);
		
		m_pPI[nScan].bBounded = false;
		m_pPI[nScan].nBounce = 0;
	}

//////////

	if( FAILED( g_BsKernel.GetDevice()->CreateVertexBuffer( m_nV * sizeof(POINTSPRITEVERTEX) * 4 * PTCTRAIL_MAX,
		D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &m_pVB ) ) )
		return;

	if( FAILED( g_BsKernel.GetDevice()->CreateIndexBuffer( m_nV * sizeof(WORD) * 6 * PTCTRAIL_MAX,
		0, D3DFMT_INDEX16,D3DPOOL_DEFAULT, &m_pIB ) ) )
		return;


	POINTSPRITEVERTEX	*pV;

	m_pVB->Lock( 0, sizeof(POINTSPRITEVERTEX) * m_nV * 4 * PTCTRAIL_MAX, (VOID**)&pV, 0 );

	for(int i = 0;i < m_nV;i++)
	{
		m_pPI[i].fSize[0] = RandomNumberInRange(80.0f, 110.0f);

		for(int j =0 ;j < PTCTRAIL_MAX; j++)
		{
			m_pPI[i].fSize[j] = m_pPI[i].fSize[0] * .95f;

			int	nId = (i * PTCTRAIL_MAX + j ) * 4;

			D3DXVec3ToVec4(&(pV[nId].Pos),&(m_pPI[i].Pos[j]),m_pPI[i].fSize[j]);
			D3DXVec3ToVec4(&(pV[nId + 1].Pos),&(m_pPI[i].Pos[j]),m_pPI[i].fSize[j]);
			D3DXVec3ToVec4(&(pV[nId + 2].Pos),&(m_pPI[i].Pos[j]),m_pPI[i].fSize[j]);
			D3DXVec3ToVec4(&(pV[nId + 3].Pos),&(m_pPI[i].Pos[j]),m_pPI[i].fSize[j]);
			
			float fW = RandomNumberInRange(0.0f, 1.0f);

			pV[nId].UV = D3DXVECTOR3(0.0f,0.0f,fW);
			pV[nId + 1].UV = D3DXVECTOR3(1.0f,0.0f,fW);
			pV[nId + 2].UV = D3DXVECTOR3(0.0f,1.0f,fW);
			pV[nId + 3].UV = D3DXVECTOR3(1.0f,1.0f,fW);
			pV[nId].Diff	= D3DXVECTOR4(1.0f,1.0f,1.0f,0.9f);
			pV[nId + 1].Diff = D3DXVECTOR4(1.0f,1.0f,1.0f,0.9f);
			pV[nId + 2].Diff = D3DXVECTOR4(1.0f,1.0f,1.0f,0.9f);
			pV[nId + 3].Diff = D3DXVECTOR4(1.0f,1.0f,1.0f,0.9f);
		}
	}

	m_pVB->Unlock();


	WORD *pIndices;
	if( FAILED( m_pIB->Lock( 0, sizeof(WORD) * m_nV * 6 * PTCTRAIL_MAX, (VOID**)&pIndices, 0 ) ) )   
		return;

	// set indices

	for (int i = 0; i < m_nV * PTCTRAIL_MAX; i++)
	{
		pIndices[i * 6]		= i * 4 + 1;
		pIndices[i * 6 + 1] = i * 4;
		pIndices[i * 6 + 2] = i * 4 + 2;
		pIndices[i * 6 + 3] = i * 4 + 1;
		pIndices[i * 6 + 4] = i * 4 + 2;
		pIndices[i * 6 + 5] = i * 4 + 3;
	}

	m_pIB->Unlock();

	Gather(true);

	m_bInit = true;

}


bool	CFcLiquidObject::ComputeBounce(D3DXVECTOR3 *pVelOut, D3DXVECTOR3 *pPos, float fHeight, D3DXVECTOR3 *pVel)
{
	bool		bRValue = false;
	D3DXVECTOR3 vNormal;
	float fAtt = g_BsKernel.GetLandHeight(pPos->x, pPos->z, &vNormal);

	if (pPos->y < fAtt + fHeight)
	{
		pPos->y = fAtt + fHeight;

		float fSpeed = D3DXVec3Length(pVel);
		
		if (fSpeed > 0.0f)
		{
			D3DXVECTOR3	E1, E2;
			float		cosf;
			D3DXVECTOR3	vUnit;

			vUnit = (*pVel) / fSpeed;
			vNormal *= -1.0f;
			
			cosf = D3DXVec3Dot(&vUnit, &vNormal);

			E1 = vNormal * fSpeed * cosf;
			E2 = (*pVel) - E1;

			(*pVelOut) = (E2 - E1 * .99f);

			bRValue = true;
		}
	}

	return bRValue;
}

void	CFcLiquidObject::Gather(bool bOpt)
{
	int	nScan;


	m_bGather = bOpt;

	if (m_bGather)
	{

		for(nScan = 0;nScan < m_nV; nScan++)
		{
			float fA, fD;

			m_pVT[nScan] = m_pV[nScan] + m_Cross.m_PosVector;
			
			fA = RandomNumberInRange(0.0f, 3.141592f * 2.0f);
			fD = RandomNumberInRange(10.0f, 500.0f);

			m_pPI[nScan].Pos[0] = m_Cross.m_PosVector;
			m_pPI[nScan].Pos[0].x += fD * cosf(fA);
			m_pPI[nScan].Pos[0].z += fD * sinf(fA);
			m_pPI[nScan].Pos[0].y = g_BsKernel.GetLandHeight( m_pPI[nScan].Pos[0].x, m_pPI[nScan].Pos[0].z ) - 100.0f;

			for(int nScan2 = 1;nScan2 < PTCTRAIL_MAX; nScan2++)
				m_pPI[nScan].Pos[nScan2] = m_pPI[nScan].Pos[0];

			m_pPI[nScan].Acc[0] = D3DXVECTOR3(0.0f,0.0f,0.0f);
			m_pPI[nScan].Vel[0] = D3DXVECTOR3(0.0f,0.0f,0.0f);
			m_pPI[nScan].nBounce = 0;
			m_pPI[nScan].bBounded = true;
		}

	}
	else
	{
		for(nScan = 0;nScan < m_nV; nScan++)
		{
			D3DXVECTOR3	Diff;

			Diff = m_pPI[nScan].Pos[0] - m_Cross.m_PosVector;

			float	fSpeed = D3DXVec3Length(&Diff);

			if (fSpeed > 1.0f)
				Diff = (Diff / fSpeed) * 5.0f;

			m_pPI[nScan].Vel[0] = Diff;
			m_pPI[nScan].bBounded = false;
		}
	}
}

void CFcLiquidObject::Process()
{
	int	nScan;

	CFcAniObject::Process();

	if (m_bInit==false)
		return;

	for(nScan = 0;nScan < m_nV; nScan ++)
	{
		D3DXVECTOR3	Diff;
		float		fDist;

		if (m_pPI[nScan].bBounded == true)
		{

			Diff = m_pVT[nScan] - m_pPI[nScan].Pos[0];

			fDist = D3DXVec3Length(&Diff);

			if (fDist > 10.0f)
				m_pPI[nScan].Acc[0] = Diff * 0.002f;
			else
				m_pPI[nScan].Acc[0] = Diff * 0.1f / fDist;

			if (m_pPI[nScan].fAlpha < 0.9f)
				m_pPI[nScan].fAlpha += 0.1f;
			
			m_pPI[nScan].nBounce = 0;
		}
		else
		{

			float	fY = g_BsKernel.GetLandHeight( m_pPI[nScan].Pos[0].x, m_pPI[nScan].Pos[0].z);

			if (m_pPI[nScan].Pos[0].y < fY - 400.0f)
				continue;

			m_pPI[nScan].Acc[0] = D3DXVECTOR3(0.0f,-0.7f,0.0f);

			bool	bBounce = false;

			if (m_pPI[nScan].nBounce < 5)
				bBounce = ComputeBounce(&(m_pPI[nScan].Vel[0]), &(m_pPI[nScan].Pos[0]), 10.0f, &(m_pPI[nScan].Vel[0]));

			if (m_pPI[nScan].nBounce > 0)
			{
				m_pPI[nScan].fAlpha -= 0.01f;
				
				if (m_pPI[nScan].fAlpha < 0.0f)
					m_pPI[nScan].fAlpha = 0.0f;
			}

			if (bBounce == true)
				m_pPI[nScan].nBounce++;

		}



		int	nScan2;

		for(nScan2 = 1; nScan2 < PTCTRAIL_MAX; nScan2++)
		{
			float fDistMax = (m_pPI[nScan].fSize[nScan2 + 1] + m_pPI[nScan].fSize[nScan2]) * .3f;

			Diff = m_pPI[nScan].Pos[nScan2-1] - m_pPI[nScan].Pos[nScan2];
			fDist = D3DXVec3Length(&Diff);

			if (fDist < 1.0f)
				m_pPI[nScan].Acc[nScan2] = D3DXVECTOR3(0.0f,0.0f,0.0f);
			else
				m_pPI[nScan].Acc[nScan2] = Diff * 0.002f;

			if (fDist > fDistMax)
			{
				Diff = (Diff / fDist) * fDistMax;
				m_pPI[nScan].Pos[nScan2] = m_pPI[nScan].Pos[nScan2-1] - Diff;
			}

		}

		for(nScan2 = 0; nScan2 < PTCTRAIL_MAX; nScan2++)
			m_pPI[nScan].Vel[nScan2] = (m_pPI[nScan].Vel[nScan2] + m_pPI[nScan].Acc[nScan2]) * .98f;

		if (m_bGather)
		{
			float	fSpeed = D3DXVec3Length(&(m_pPI[nScan].Vel[0]));

			if (fSpeed > 5.0f)
				m_pPI[nScan].Vel[0] = (m_pPI[nScan].Vel[0] / fSpeed) * 5.0f;

		}

		for(nScan2 = 0; nScan2 < PTCTRAIL_MAX; nScan2++)
			m_pPI[nScan].Pos[nScan2] += m_pPI[nScan].Vel[nScan2];
	}

	POINTSPRITEVERTEX	*pV;

	m_pVB->Lock( 0, sizeof(POINTSPRITEVERTEX) * m_nV * 4 * PTCTRAIL_MAX, (VOID**)&pV, 0 );

	for(int i = 0;i < m_nV;i++)
	{
		for(int j = 0;j < PTCTRAIL_MAX; j ++)
		{
			int	nId = (i * PTCTRAIL_MAX + j) * 4;

			D3DXVec3ToVec4(&(pV[nId].Pos),&(m_pPI[i].Pos[j]),pV[nId].Pos.w);
			D3DXVec3ToVec4(&(pV[nId + 1].Pos),&(m_pPI[i].Pos[j]),pV[nId + 1].Pos.w);
			D3DXVec3ToVec4(&(pV[nId + 2].Pos),&(m_pPI[i].Pos[j]),pV[nId + 2].Pos.w);
			D3DXVec3ToVec4(&(pV[nId + 3].Pos),&(m_pPI[i].Pos[j]),pV[nId + 3].Pos.w);

			D3DXVec3ToVec4(&(pV[nId].N),&(m_pNT[i]),1.0f);
			D3DXVec3ToVec4(&(pV[nId + 1].N),&(m_pNT[i]),1.0f);
			D3DXVec3ToVec4(&(pV[nId + 2].N),&(m_pNT[i]),1.0f);
			D3DXVec3ToVec4(&(pV[nId + 3].N),&(m_pNT[i]),1.0f);

			pV[nId].Diff.w = m_pPI[i].fAlpha;
			pV[nId + 1].Diff.w = m_pPI[i].fAlpha;
			pV[nId + 2].Diff.w = m_pPI[i].fAlpha;
			pV[nId + 3].Diff.w = m_pPI[i].fAlpha;
		}
	}

	m_pVB->Unlock();

}

void CFcLiquidObject::CustomRender( void *pThis, C3DDevice *pDevice, D3DXMATRIX *matObject )
{

#ifndef _XBOX

	D3DXMATRIX matWorld;
	D3DXMATRIX matWVP;
	const D3DXMATRIX* matView;
	const D3DXMATRIX* matProj;
    
	UINT iPass, cPasses;

	CFcLiquidObject *pObj = (CFcLiquidObject *)pThis;

	if (pObj->m_bInit == false)
		return;
	
	matView = g_BsKernel.GetParamViewMatrix();
	matProj = g_BsKernel.GetParamProjectionMatrix();

	pObj->m_pEffect->Begin(&cPasses, 0);

	pDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
	pDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
	pDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
	pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	pDevice->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD);
	pDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	pDevice->SetRenderState( D3DRS_LIGHTING, TRUE  );

	pObj->m_fTime += 0.0005f;

	for (iPass = 0; iPass < cPasses; iPass++)
	{
		pObj->m_pEffect->BeginPass(iPass);

		D3DXMatrixIdentity(&matWorld);

		pObj->m_pEffect->SetMatrix( "matView", matView);
		pObj->m_pEffect->SetMatrix( "matProj", matProj);
		pObj->m_pEffect->SetMatrix( "viewInverse", g_BsKernel.GetParamInvViewMatrix());				
		pObj->m_pEffect->SetVector( "lightDir", CBsKernel::GetInstance().GetLightDirection());
		pObj->m_pEffect->SetVector( "lightDiffuse", CBsKernel::GetInstance().GetLightDiffuse());
		pObj->m_pEffect->SetVector( "lightAmbient", CBsKernel::GetInstance().GetLightAmbient());
		

		FLOAT	fT = pObj->m_fTime * 10.0f;

		pObj->m_pEffect->SetFloat( "time", pObj->m_fTime );
		pObj->m_pEffect->SetFloat( "time10", fT);
		pObj->m_pEffect->SetFloat( "time40", fT * 4);


		int nBackBuffer = g_BsKernel.GetImageProcess()->GetBackBufferTexture();
		if( pDevice->IsMSAAEnable() ) 
		{
			pObj->m_pEffect->SetTexture("BackBuffer", pDevice->GetTilingCompleteTexture() );//nBackBuffer
		}		
		else 
		{	
			pObj->m_pEffect->SetTexture("BackBuffer", (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(nBackBuffer));
		}

        pObj->m_pEffect->SetTexture( "Swizzle", (LPDIRECT3DBASETEXTURE9)(g_BsKernel.GetTexturePtr(pObj->m_nChunkTex)));
		pObj->m_pEffect->SetTexture( "Swizzle2", (LPDIRECT3DBASETEXTURE9)(g_BsKernel.GetTexturePtr(pObj->m_nChunkTex2)));
		pObj->m_pEffect->SetTexture( "Spec", (LPDIRECT3DBASETEXTURE9)(g_BsKernel.GetTexturePtr(pObj->m_nSpecTex)));

		pObj->m_pEffect->CommitChanges();

		pDevice->SetStreamSource( 0 , pObj->m_pVB, sizeof(POINTSPRITEVERTEX));
		pDevice->SetIndices( pObj->m_pIB ); // , 0 );

		g_BsKernel.SetVertexDeclaration(pObj->m_nVertexDeclIdx);

		pDevice->DrawIndexedMeshVB(D3DPT_TRIANGLELIST, pObj->m_nV * 4 * PTCTRAIL_MAX, pObj->m_nV * 2 * PTCTRAIL_MAX, 0, 0);
		pObj->m_pEffect->EndPass() ;
	}
	
	pObj->m_pEffect->End();

	pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	pDevice->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
#endif //_XBOX
}