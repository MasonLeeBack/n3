#include "stdafx.h"
#include "FcFxTidalWave.h"
#include "3DDevice.h"
#include "BsMaterial.h"
#include "BsKernel.h"
#include "BsObject.h"
#include "CrossVector.h"
#include "FCWorld.h"
#include "FCGameObject.h"
#include "FCGlobal.h"
#include "FCheroObject.h"
#include "FcProp.h"
#include "FcFxManager.h"
#include "FcUtil.h"
#include "PerfCheck.h"
#include "Data/FXList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

#define TW_HEIGHT_CHECK_VAL		50.f




BSVECTOR	g_SphereTable[SPHERE_TABLE_SIZE][SPHERE_TABLE_SIZE];		// 정사각형 -> 구

#define		_BEFORE_CAPTURE



/*---------------------------------------------------------------------------------
-
-			CFcFXTidalWave::CFcFXTidalWave()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
CFcFXTidalWave::CFcFXTidalWave()
{
	m_pV2 = NULL;
	m_pI2 = NULL;
	m_pVList = NULL;
	m_bInitialized = false;
	m_nFxGenerater = -1;
	m_nMaterialIndex = -1;
	m_nWaterFxID = -1;
#ifndef _LTCG
	SetFxRtti(FX_TYPE_TIDALWAVE);
#endif //_LTCG
}





/*---------------------------------------------------------------------------------
-
-			CFcFXTidalWave::~CFcFXTidalWave()
-					; 소멸자.
-					  
-
---------------------------------------------------------------------------------*/
CFcFXTidalWave::~CFcFXTidalWave()
{
	Clear();
}





/*---------------------------------------------------------------------------------
-
-			CFcFXTidalWave::Init()
-					; .
-					  
-
---------------------------------------------------------------------------------*/
void CFcFXTidalWave::Init(float	fLifeTime)
{
	int	iScan,iScan2;
	float	fStart;
	m_nSwizzleTex = -1;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if( GetWaterDrop() )			// 튜르르 오브 공격이면.
	{
		m_iCol = 50;														// 물 orb의 행 단위의 정점수
		m_iRow = 50;														// 물 orb의 열 단위의 정점수
		m_fInitDist = 5.0f;													// 각 정점간의 최초거리(최소거리도 된다.)

		m_vGravity.x = 0.0f;												// 중력값
		m_vGravity.y = -2.0f;
		m_vGravity.z = 0.0f;


		m_vSafeCenter.x = 0.0f;												// 안전위치(중심) (캐릭터 시전 위치.)
		m_vSafeCenter.y = 0.0f;	
		m_vSafeCenter.z = 0.0f;
		m_fSafeRad = -1.0f;													// 안전위치에서부터의 반경

		m_bSpread = false;													// 퍼져나갈 것 인가? 
		m_vSpreadOrigin.x = 0.0f;
		m_vSpreadOrigin.y = 0.0f;
		m_vSpreadOrigin.z = 0.0f;
		m_fTick = 0.0f;						
		m_fLifeTime = 140.0f;

		m_nVertexDeclIdx = -1;
		m_nFxGenerater = g_pFcFXManager->Create( FX_TYPE_LISTGENERATER );
		g_pFcFXManager->SendMessage(m_nFxGenerater,FX_INIT_OBJECT, FX_TYURR08Y, 120, -1);
		g_pFcFXManager->SendMessage(m_nFxGenerater,FX_PLAY_OBJECT);

	}///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	else							// 일반 오버스파크면.
	{
		m_iCol = 100;														// 물 orb의 행 단위의 정점수
		m_iRow = 100;														// 물 orb의 열 단위의 정점수
		m_fInitDist = 25.0f;												// 각 정점간의 최초거리(최소거리도 된다.)

		m_vGravity.x = 0.0f;												// 중력값
		m_vGravity.y = -2.0f;
		m_vGravity.z = 0.0f;


		m_vSafeCenter.x = 0.0f;												// 안전위치(중심) (캐릭터 시전 위치.)
		m_vSafeCenter.y = 0.0f;	
		m_vSafeCenter.z = 0.0f;
		m_fSafeRad = -1.0f;													// 안전위치에서부터의 반경

		m_bSpread = false;													// 퍼져나갈 것 인가? 
		m_vSpreadOrigin.x = 0.0f;
		m_vSpreadOrigin.y = 0.0f;
		m_vSpreadOrigin.z = 0.0f;
		m_fTick = 0.0f;						
		m_fLifeTime = 130.0f;

		fStart =  PI * 0.25f;										// 펼쳐질 각. 

		for(iScan = 0;iScan < SPHERE_TABLE_SIZE ;iScan++)					//SPHERE_TABLE_SIZE 12
		{
			for(iScan2 = 0;iScan2 < SPHERE_TABLE_SIZE ;iScan2++)
			{
				g_SphereTable[iScan][iScan2].x = ((float)iScan) / SPHERE_TABLE_SIZE - 0.5f;
				g_SphereTable[iScan][iScan2].z = ((float)iScan2) / SPHERE_TABLE_SIZE - 0.5f;
			}
		}

		g_SphereTable[SPHERE_TABLE_SIZE / 2][SPHERE_TABLE_SIZE / 2].x = 0.0f;
		g_SphereTable[SPHERE_TABLE_SIZE / 2][SPHERE_TABLE_SIZE / 2].y = 1.0f;
		g_SphereTable[SPHERE_TABLE_SIZE / 2][SPHERE_TABLE_SIZE / 2].z = 0.0f;

		for(iScan = 0; iScan < SPHERE_TABLE_SIZE / 2 + 1; iScan ++)
		{
			for(iScan2 = iScan; iScan2 < SPHERE_TABLE_SIZE - 1 - iScan;iScan2++)
			{
				float	fRScan;
				float	fRadius;
				float	fY;
				float	fRadian;


				// fRScan = 최소 : fStart     최대 : 0.5*PI   -45도에서부터 90도까지
				fRScan = fStart + ((float)iScan) / ((float)(SPHERE_TABLE_SIZE / 2)) * (PI * .5f - fStart);

				// cos(-45) 에서부터 cos(90) 까지 => 0.7 => 1 => 0 
				fRadius = cosf(fRScan);
				//			fY = sinf(fRScan);
				fY = 0.0f;

				fRadian = ((float)(iScan2 - iScan)) / ((float)(SPHERE_TABLE_SIZE-1-iScan * 2) * PI / 2);

				g_SphereTable[iScan2][iScan].x = fRadius * cosf(fRadian);
				g_SphereTable[iScan2][iScan].y = fY;
				g_SphereTable[iScan2][iScan].z = fRadius * sinf(fRadian);

				g_SphereTable[SPHERE_TABLE_SIZE - iScan - 1][iScan2].x = fRadius * cosf(PI * .5f + fRadian);
				g_SphereTable[SPHERE_TABLE_SIZE - iScan - 1][iScan2].y = fY;
				g_SphereTable[SPHERE_TABLE_SIZE - iScan - 1][iScan2].z = fRadius * sinf(PI * .5f + fRadian);

				g_SphereTable[SPHERE_TABLE_SIZE - iScan2 - 1][SPHERE_TABLE_SIZE - iScan - 1].x = fRadius * cosf(PI + fRadian);
				g_SphereTable[SPHERE_TABLE_SIZE - iScan2 - 1][SPHERE_TABLE_SIZE - iScan - 1].y = fY;
				g_SphereTable[SPHERE_TABLE_SIZE - iScan2 - 1][SPHERE_TABLE_SIZE - iScan - 1].z = fRadius * sinf(PI + fRadian);

				g_SphereTable[iScan][SPHERE_TABLE_SIZE - iScan2 - 1].x = fRadius * cosf(PI * 1.5f + fRadian);
				g_SphereTable[iScan][SPHERE_TABLE_SIZE - iScan2 - 1].y = fY;
				g_SphereTable[iScan][SPHERE_TABLE_SIZE - iScan2 - 1].z = fRadius * sinf(PI * 1.5f + fRadian);
			}
		}
	}

	

	m_G.iV = 0;

	m_vPos.x = 0.0f;
	m_vPos.x = 0.0f;
	m_vPos.x = 0.0f;
	m_fLifeTime = fLifeTime;
	m_fTick = 0.0f;

	// Initialize three vertices for rendering a triangle
	CreateVertices();
	CreateIndices();

	if( GetWaterDrop() )
	{
		g_BsKernel.chdir("fx");
		m_nSwizzleTex = g_BsKernel.LoadTexture("diffuse4water.dds");
		g_BsKernel.chdir("..");
	}
	else
	{
		// 재영씨 이거 파일 없어서 로딩 안되길래 파일 VSS에 넣어놨습니다. yooty
		g_BsKernel.chdir("fx");
		m_nSwizzleTex = g_BsKernel.LoadVolumeTexture("swizzle_vol.dds");
		g_BsKernel.chdir("..");
	}
	
	m_nSplashPtc = 85;
	m_nSplashPtc2 = 87;

	// Material Setting!!
	char fullName[_MAX_PATH];
	strcpy(fullName, g_BsKernel.GetShaderDirectory());
	if( GetWaterDrop() )
		strcat(fullName, "meshdistortionlight2.fx");
	else
		strcat(fullName, "FakeLiquid.fx");

	m_nMaterialIndex = g_BsKernel.LoadMaterial(fullName, FALSE);

	
	D3DVERTEXELEMENT9 decl[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 16, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0, 32, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		
		
		D3DDECL_END()
	};
	m_nVertexDeclIdx = g_BsKernel.LoadVertexDeclaration(decl);
	
	
	m_DeviceDataState = FXDS_INITREADY;

	CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialIndex);
#ifdef _XBOX
	if( GetWaterDrop() )
	{
		m_hWorldViewProj = pMaterial->GetParameterByName( "WorldViewProjection");
		m_hWorld = pMaterial->GetParameterByName( "World");
		m_hSwizzle = pMaterial->GetParameterByName( "diffuseSampler");
		m_hBackBuffer = pMaterial->GetParameterByName( "ScreenSampler");
		m_hlightAmb = pMaterial->GetParameterByName( "MaterialAmbient");
	}
	else
	{
		m_hWorldViewProj = pMaterial->GetParameterByName( "WorldViewProjection");
		m_hWorld = pMaterial->GetParameterByName( "World");
		m_hworldInverseTranspose = pMaterial->GetParameterByName( "WorldInverseTranspose");
		m_hBackBuffer = pMaterial->GetParameterByName( "ScreenSampler");
		m_hSwizzle = pMaterial->GetParameterByName( "Tex2");
		m_hcounter = pMaterial->GetParameterByName( "counter");
		m_hmatSpec = pMaterial->GetParameterByName( "Specular" );
		m_hmatAmb = pMaterial->GetParameterByName( "Ambient1");
		m_hmatDiff = pMaterial->GetParameterByName( "Diffuse3");
	}
	

#else
	if( GetWaterDrop() )
	{
		m_hWorldViewProj = pMaterial->GetParameterByName( "worldViewProj");
		m_hWorld = pMaterial->GetParameterByName( "world");
		m_hSwizzle = pMaterial->GetParameterByName( "diffuseTexture");
		m_hBackBuffer = pMaterial->GetParameterByName( "screenTexture");
		m_hlightAmb = pMaterial->GetParameterByName( "materialAmbient");
	}
	else
	{
		m_hWorldViewProj = pMaterial->GetParameterByName( "worldViewProj");
		m_hWorld = pMaterial->GetParameterByName( "world");
		m_hworldInverseTranspose = pMaterial->GetParameterByName( "worldInverseTranspose");
		m_hcounter = pMaterial->GetParameterByName( "counter");
		m_hmatSpec = pMaterial->GetParameterByName( "matSpec" );
		m_hBackBuffer = pMaterial->GetParameterByName( "screenTexture");
		m_hSwizzle = pMaterial->GetParameterByName( "Swizzle");
		m_hmatAmb = pMaterial->GetParameterByName( "matAmb");
		m_hmatDiff = pMaterial->GetParameterByName( "matDiff");
	}
	
#endif

}





/*---------------------------------------------------------------------------------
-
-			CFcFXTidalWave::CreateVertices()
-					; 버텍스버퍼를 만든다.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFXTidalWave::CreateVertices()
{
	m_iVertex = m_iCol * m_iRow;

	m_pV2 = new CUSTOMVERTEX[m_iVertex];
	
	m_pVList = (_VERTEX*) malloc(m_iVertex * sizeof(_VERTEX));

	memset(m_pVList,0,sizeof(_VERTEX) * m_iVertex);


	for(int iScan = 0;iScan < m_iVertex;iScan++)
	{
		m_pV2[iScan].TexCoord.x = (float)(iScan % m_iCol) / (float)(m_iCol);
		m_pV2[iScan].TexCoord.y = (float)(((int)iScan / m_iCol)) / (float)(m_iRow);
	}
	
}





/*---------------------------------------------------------------------------------
-
-			CFcFXTidalWave::CreateIndices()
-					; 인덱스 버퍼를 만든다.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFXTidalWave::CreateIndices()
{
	m_iTriangle = (m_iCol - 1) * (m_iRow - 1) * 2;

	int nSize = m_iTriangle * sizeof(WORD) * 3;

	m_pI2 = new WORD[nSize];

	// set indices
	for (int iR = 0; iR < m_iRow - 1; iR++)
	{
		for (int iC = 0;iC < m_iCol - 1; iC++)
		{
			int	nTId;
			int nVId[4];

			nTId = (iR * (m_iCol - 1) + iC) * 6;

			nVId[0] = iR * m_iCol + iC;
			nVId[1] = iR * m_iCol + iC + 1;
			nVId[2] = (iR + 1) * m_iCol + iC;
			nVId[3] = (iR + 1) * m_iCol + iC + 1;

			m_pI2[nTId] = nVId[0];
			m_pI2[nTId + 1] = nVId[2];
			m_pI2[nTId + 2] = nVId[1];
			m_pI2[nTId + 3] = nVId[1];
			m_pI2[nTId + 4] = nVId[2];
			m_pI2[nTId + 5] = nVId[3];

		}
	}
}





/*---------------------------------------------------------------------------------
-
-			CFcFXTidalWave::Process()
-					; 
-					  
-
---------------------------------------------------------------------------------*/
void CFcFXTidalWave::Process()
{
	if( m_state == ENDING )
	{
		//Clear();
		m_state = END;
		m_DeviceDataState=FXDS_RELEASEREADY;
	}
	if( m_state == END ) return;

	if (m_bInitialized == false)
		return;

	int nSize = m_vecParticlePos[CBsObject::GetProcessBufferIndex()].size();
	for( int i = 0 ; i < nSize ; i++ )
	{
		sParticle tmp = m_vecParticlePos[CBsObject::GetProcessBufferIndex()][i];
		CCrossVector Ptc;
		Ptc.m_YVector *= tmp._fSpeed * tmp._fcosf / 10.0f;
		Ptc.m_PosVector = tmp._vPos;
		Ptc.UpdateVectors();

		if( tmp._fcosf >= 0.95f )
		{
			g_BsKernel.CreateParticleObject(m_nSplashPtc2,false,false,Ptc, 1.0f + 0.01f * (float)(Random(100)));
		}
		else
		{
			g_BsKernel.CreateParticleObject(m_nSplashPtc,false,false,Ptc, 1.0f + 0.01f * (float)(Random(100)));
		}

	}
	m_vecParticlePos[CBsObject::GetProcessBufferIndex()].clear();

	nSize = m_vecOrbParticle[CBsObject::GetProcessBufferIndex()].size();
	for( int i = 0 ; i < nSize ; i++ )
	{
		sOrbAttackParticle tmp = m_vecOrbParticle[CBsObject::GetProcessBufferIndex()][i];
		g_pFcFXManager->SendMessage( m_nFxGenerater, FX_UPDATE_OBJECT, (DWORD)&tmp._vPos, (DWORD)&tmp._vDir);
	}
	m_vecOrbParticle[CBsObject::GetProcessBufferIndex()].clear();
	

}

void CFcFXTidalWave::Clear()
{
	SAFE_DELETEA(m_pV2);
	SAFE_DELETEA(m_pI2);

	if (m_pVList != NULL)
	{
		free (m_pVList);
		m_pVList = NULL;
	}

	SAFE_RELEASE_VD(m_nVertexDeclIdx);
	SAFE_RELEASE_TEXTURE(m_nSwizzleTex);

	SAFE_DELETE_FX(m_nFxGenerater,FX_TYPE_LISTGENERATER);

	for( int i = 0 ; i < 2 ; i++ ) {
		m_vecOrbParticle[i].clear();
		m_vecParticlePos[i].clear();
	}
}


/*---------------------------------------------------------------------------------
-
-			CFcFXTidalWave::ComputeBounce()
-					; .
-					  
-
---------------------------------------------------------------------------------*/
void	CFcFXTidalWave::ComputeBounce(BSVECTOR *pVelOut, BSVECTOR *pPos, float fHeight, BSVECTOR *pVel)
{

	BSVECTOR vNormal = BSVECTOR(0.0f ,1.0f ,0.0f ); 
	D3DXVECTOR3 vNormal2(vNormal.x,vNormal.y,vNormal.z );

	float fAtt;
	
	float	fMapXSize, fMapYSize;
	g_FcWorld.GetMapSize( fMapXSize, fMapYSize );
	if (pPos->x >= 0.0f && pPos->x < fMapXSize && 
		pPos->z >= 0.0f && pPos->z < fMapYSize)
		fAtt = g_FcWorld.GetLandHeightEx2( pPos->x, pPos->z, m_hParent->GetCrossVector()->m_PosVector.y, true, &vNormal2);
	else
		fAtt = 0.0f;
	
	if( GetWaterDrop() )
	{
		vNormal2.x *= RandomNumberInRange(0.8f, 1.1f);
		vNormal2.y *= RandomNumberInRange(0.8f, 1.1f);
		vNormal2.z *= RandomNumberInRange(0.8f, 1.1f);
	}
	
	D3DXVec3Normalize(&vNormal2, &vNormal2);
	vNormal = BSVECTOR(vNormal2.x,vNormal2.y,vNormal2.z);
		
	if (pPos->y < fAtt + fHeight)
	{
		pPos->y = fAtt + fHeight;

		float fSpeed = BsVec3Length(pVel);

		if(fSpeed > 0.0f)
		{
			BSVECTOR	E1, E2;
			float		cosf;
			BSVECTOR	vUnit;

			vUnit		= (*pVel) / fSpeed;
			if( GetWaterDrop() )
			{
				vNormal		*= -1.0f;//RandomNumberInRange(0.8f, 1.2f);
				cosf		= BsVec3Dot(&vUnit, &vNormal);
				E1			= vNormal * fSpeed * cosf;
				E2			= (*pVel) - E1;
				(*pVelOut) = (E2 - E1 * .99f);

				if(Random(2000) < 6)
				{
					D3DXVECTOR3 vDir(pVelOut->x, 0, pVelOut->z);
					D3DXVec3Normalize(&vDir, &vDir);
					sOrbAttackParticle tmp;
					tmp._vPos = D3DXVECTOR3(pPos->x,pPos->y,pPos->z);
					tmp._vDir = vDir;
					m_vecOrbParticle[CBsObject::GetRenderBufferIndex()].push_back(tmp);
					//g_pFcFXManager->SendMessage( m_nFxGenerater, FX_UPDATE_OBJECT, (DWORD)pPos, (DWORD)&vDir);
					
				}
			}
			else
			{
				vNormal		*= -1.05f;
				cosf		= BsVec3Dot(&vUnit, &vNormal);
				E1			= vNormal * fSpeed * cosf;
				E2			= (*pVel) - E1;
				(*pVelOut) = (E2 - E1 * .99f);
			}
			

			float	fV = fSpeed * cosf;
			BSVECTOR	Diff = (*pPos) - m_vSpreadOrigin;



			if (Random(1000) < 10)
			{
				if (fV > 20.0f && BsVec3Length(&Diff) > 2500.0f)
				{
					sParticle tmp;
					tmp._fSpeed = fSpeed;
					tmp._fcosf = cosf;
					tmp._vPos = D3DXVECTOR3(pPos->x,pPos->y,pPos->z);

					m_vecParticlePos[CBsObject::GetRenderBufferIndex()].push_back(tmp);

				}
			}// if (Random(1000) < 10)
		}//if(fSpeed > 0.0f)
	}
}








/*---------------------------------------------------------------------------------
-
-			CFcFXTidalWave::ProcessSpring()
-					; pOrg 정점을 pObj 정점으로 계산하여 pOut 으로 출력.
-					  
-
---------------------------------------------------------------------------------*/
void	CFcFXTidalWave::ProcessSpring(CUSTOMVERTEX *pV)
{
	BSVECTOR SpreadVelEverage = BSVECTOR(0.f, 0.f, 0.f);
	
	for(int iScan = 0;iScan < m_iVertex;iScan++)
	{
		pV[iScan].Normal = BSVECTOR(0.0f,0.0f,0.0f);
		m_pVList[iScan].Acc = BSVECTOR(0.0f,0.0f,0.0f);


		// 용수철 연결
		if (m_pVList[iScan].bNoEff == false)
		{
			int	iC, iR;
			int	iVId[4] = {-1,-1,-1,-1};	// 근처의 vertex id

			iC = iScan % m_iCol;
			iR = iScan / m_iCol;

			if (iC > 0)
				iVId[0] = iR * m_iCol + iC - 1;

			if (iC < m_iCol - 1)
				iVId[1] = iR * m_iCol + iC + 1;

			if (iR > 0)
				iVId[2] = (iR - 1) * m_iCol + iC;

			if (iR < m_iRow - 1)
				iVId[3] = (iR + 1) * m_iCol + iC;

			int iScan2;

			// 연결된 점들로 부터 힘을 받는다. 

			bool	bLinkedWithNoEffV = false;

			for(iScan2 = 0;iScan2 < 4; iScan2++)
			{
				if (iVId[iScan2] == -1)
					continue;

				if (m_pVList[iVId[iScan2]].bNoEff == true)
					bLinkedWithNoEffV = true;

				BSVECTOR	Acc;

				ComputeSpring(&Acc, 0.02f, m_fInitDist, &(pV[iScan].Pos), &(pV[iVId[iScan2]].Pos));
				m_pVList[iScan].Acc +=  Acc;
			}
			// 중력 적용

			
			if (bLinkedWithNoEffV == false)
				m_pVList[iScan].Acc += m_vGravity;


			////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////
			m_pVList[iScan].Vel += m_pVList[iScan].Acc;
			BSVECTOR* pVel = NULL;
			if( GetWaterDrop() )
			{
				m_pVList[iScan].Vel *= 0.9f; // 저항

				pVel	= &(m_pVList[iScan].Vel);
				float		fSpeed	= BsVec3Length(pVel);

				if (fSpeed > 150.0f)
				{
					(*pVel) /= fSpeed;
					(*pVel) *= 150.0f;
				}
			}
			else
			{
				m_pVList[iScan].Vel *= 0.95f; // 저항

				pVel	= &(m_pVList[iScan].Vel);
				float		fSpeed	= BsVec3Length(pVel);

				if (fSpeed > 350.0f)
				{
					(*pVel) /= fSpeed;
					(*pVel) *= 350.0f;
				}
			}
			

			if (m_bSpread == true)
			{
				BSVECTOR	SpreadVel = pV[iScan].Pos - m_vSpreadOrigin;
				BsVec3Normalize(&SpreadVel, &SpreadVel);
				SpreadVel *= m_fSpreadSpeed;

				pV[iScan].Pos += SpreadVel;
				SpreadVelEverage += SpreadVel;
			}
			
			pV[iScan].Pos += m_pVList[iScan].Vel;
			m_pVList[iScan].Pos = pV[iScan].Pos;
			ComputeBounce(pVel, &(pV[iScan].Pos), TW_HEIGHT_CHECK_VAL, pVel);
			//ProcessSafeRegion(&pV[iScan]);
			
			////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////


		}
	}
	SpreadVelEverage /= (float) m_iVertex * 1.5f;
	m_vSpreadOrigin += SpreadVelEverage;
}







/*---------------------------------------------------------------------------------
-
-			CFcFXTidalWave::ComputeSpring()
-					; pOrg 정점을 pObj 정점으로 계산하여 변화량을 pOut 으로 출력.
-					  
-
---------------------------------------------------------------------------------*/
void	CFcFXTidalWave::ComputeSpring(BSVECTOR *pOut, float fSpr, float fDist0, BSVECTOR *pObj, BSVECTOR *pOrg)
{
	BSVECTOR	_Diff;
	float		_fDist;
	float		_fErr;

	_Diff = (*pOrg) - (*pObj);

	_fDist = BsVec3Length(&_Diff);

	(*pOut) = BSVECTOR(0.0f,0.0f,0.0f);

	if (_fDist > 0.0f)
	{
		_fErr = _fDist - fDist0;
		_Diff = _Diff / _fDist;
		(*pOut) = _Diff * fSpr * _fErr;
	}
}






/*---------------------------------------------------------------------------------
-
-			CFcFXTidalWave::ProcessSafeRegion()
-					; 안전지역 반경에 정점이 들어왔을때 위치를 반경만큼 벌린다.
-					  
-
---------------------------------------------------------------------------------*/
void	CFcFXTidalWave::ProcessSafeRegion( CUSTOMVERTEX *pV)
{
	m_fSafeRad = 50.0f;
	if (m_fSafeRad <= 0.0f)
		return;

	BSVECTOR	Diff;
	float		fDist;

	Diff = pV->Pos - m_vSafeCenter;
	fDist = BsVec3Length(&Diff);

	if (fDist < m_fSafeRad && fDist > 0.0f)
	{
		Diff = (Diff / fDist) * m_fSafeRad;
		pV->Pos = m_vSafeCenter + Diff;
	}
}





/*---------------------------------------------------------------------------------
-
-			CFcFXTidalWave::ProcessNormal()
-					; 정점의 노멀값을 다시 계산한다.
-					  
-
---------------------------------------------------------------------------------*/
void	CFcFXTidalWave::ProcessNormal( CUSTOMVERTEX *pV, WORD *pI)
{
	int i;

	BSVECTOR	V[3];
	BSVECTOR	V1,V2;
	BSVECTOR	N;
	BSVECTOR	Center;
	int			iV1,iV2,iV3;


	for(i = 0; i < m_iTriangle; ++i)
	{
		iV1 = pI[i * 3];
		iV2 = pI[i * 3 + 1];
		iV3 = pI[i * 3 + 2];

		V[0] = pV[iV1].Pos;
		V[1] = pV[iV2].Pos;
		V[2] = pV[iV3].Pos;

#ifndef _XBOX
		Center = V[0] + V[1] + V[2];
		V1 = V[1] - V[0];
		V2 = V[2] - V[0];
		BsVec3Cross(&N, &(V1), &(V2));
		BsVec3Normalize(&N, &N);
		pV[iV1].Normal = (pV[iV1].Normal + N) * 0.5f;
		pV[iV2].Normal = (pV[iV2].Normal + N) * 0.5f;
		pV[iV3].Normal = (pV[iV3].Normal + N) * 0.5f;

#else
		Center = XMVectorAdd( XMVectorAdd(V[0] , V[1]) , V[2]);
		V1 = XMVectorSubtract(V[1], V[0]);
		V2 = XMVectorSubtract(V[2], V[0]);
		BsVec3Cross(&N, &(V1), &(V2));
		BsVec3Normalize(&N, &N);
		pV[iV1].Normal = XMVectorAdd(pV[iV1].Normal , N) * 0.5f;
		pV[iV2].Normal = XMVectorAdd(pV[iV2].Normal , N) * 0.5f;
		pV[iV3].Normal = XMVectorAdd(pV[iV3].Normal , N) * 0.5f;
#endif

	}// for
}







/*---------------------------------------------------------------------------------
-
-			CFcFXTidalWave::Reset()
-					; 리셋 시킨다. (화면에 맺히는 물방울 정지, 초기화,..etc )
-					  
-
---------------------------------------------------------------------------------*/
void CFcFXTidalWave::Reset()
{
	m_bInitialized = false;
	m_fTick = 0.0f;
	if(m_nWaterFxID != -1) {
		g_pFcFXManager->SendMessage(m_nWaterFxID, FX_STOP_OBJECT);
		g_pFcFXManager->SendMessage(m_nWaterFxID, FX_DELETE_OBJECT);
		m_nWaterFxID = -1;
	}

}







/*---------------------------------------------------------------------------------
-
-			CFcFXTidalWave::Render()
-					; .
-					  
-
---------------------------------------------------------------------------------*/
void CFcFXTidalWave::Render(C3DDevice *pDevice)
{

	if (m_bInitialized == false)
		return;

	if( !IsPause() )
	{
		ProcessSpring( m_pV2);
		ProcessNormal(m_pV2,m_pI2);
		m_fTick += 1.0f;
	}
		

	m_vCount = D3DXVECTOR4(0.0f,1.0f,0.0f, 0.0f);
	m_vCount.x += .1f;

	if( GetWaterDrop() )
	{
		m_vCount.w = 1.0f;
		if(m_fLifeTime - m_fTick < 20.0f)
			m_vCount.w = (m_fLifeTime - m_fTick) / 20.0f; 
		if (m_vCount.w < 0.0f)
		{
			m_vCount.w = 0.0f;//스래드 문제 입니다.
			Reset();
		}
		else
			CustomRender(pDevice);
	}
	else
	{
		if(m_fLifeTime - m_fTick < 50.0f)
			m_vCount.y = (m_fLifeTime - m_fTick) / 50.0f; 
		if (m_vCount.y < 0.0f)
		{
			m_vCount.y = 0.0f;
			Reset();
		}
		else
			CustomRender(pDevice);
	}
}




/*---------------------------------------------------------------------------------
-
-			CFcFXTidalWave::ProcessMessage()
-					; 메세지 처리 루틴.
-
-
---------------------------------------------------------------------------------*/
int	CFcFXTidalWave::ProcessMessage(int nCode,DWORD dwParam1, DWORD dwParam2,DWORD dwParam3)
{
	switch(nCode)
	{
	case FX_INIT_OBJECT:
		{
			D3DXVECTOR3	Pos;
			if(m_bInitialized == false)
			{
				m_hParent = *(GameObjHandle*)dwParam2;
				bool bWaterDrop;
				if( dwParam1 == 0 )
					bWaterDrop = false;//(bool)dwParam1;
				else
					bWaterDrop = true;

				SetWaterDrop(bWaterDrop);

				if( GetWaterDrop() )
					Init(50.0f);
				else
					Init(200.0f);

				Reset();
				_Group(0,0,m_iCol-1, m_iRow - 1);
				CCrossVector *Cross = m_hParent->GetCrossVector();				

				if( GetWaterDrop() )
				{
					Pos = Cross->m_PosVector + Cross->m_ZVector * 5.0f + D3DXVECTOR3(0.0f,250.0f,0.0f);
					BSVECTOR PosVector = BSVECTOR(Cross->m_PosVector.x,Cross->m_PosVector.y,Cross->m_PosVector.z);

					BSVECTOR Pos2 = BSVECTOR(Pos.x,Pos.y,Pos.z);
					SetGroupPos(&Pos2,200.0f);
					Explode(&(PosVector), 120.0f);
					SetSpread(&(PosVector), 5.0f);
				}
				else
				{
					Pos = Cross->m_PosVector + Cross->m_ZVector * 200.0f + D3DXVECTOR3(0.0f,35.0f,0.0f);
					BSVECTOR PosVector = BSVECTOR(Cross->m_PosVector.x,Cross->m_PosVector.y,Cross->m_PosVector.z);

					BSVECTOR Pos2 = BSVECTOR(Pos.x,Pos.y,Pos.z);
					SetGroupPos(&Pos2,200.0f);
					Explode(&(PosVector), 350.0f);
					SetSpread(&(PosVector), 20.0f);

					if(m_nWaterFxID == -1)
					{
						m_nWaterFxID = g_pFcFXManager->Create(FX_TYPE_WATERDROP);
						g_pFcFXManager->SendMessage(m_nWaterFxID,FX_INIT_OBJECT);
					}
					g_pFcFXManager->SendMessage(m_nWaterFxID, FX_INIT_OBJECT);
					g_pFcFXManager->SendMessage(m_nWaterFxID, FX_PLAY_OBJECT, 40);
				}


			}
			else{				
				Reset();
			}
			m_state = PLAY;
		}
		return 1;

	case FX_WATER_GET_VERTEX_COUNT:
		{
			int* pCount = (int*)dwParam1;
			*pCount = GetVertexCount();
		}
		return 1;
	case FX_WATER_GET_VERTEX:
		{
			D3DXVECTOR3* pPos = (D3DXVECTOR3*)dwParam2;
			*pPos = GetVertexPos( (int)dwParam1 );
		}
		return 1;
	case FX_WATER_GET_VELOCITY:
		{
			D3DXVECTOR3* pVel = (D3DXVECTOR3*)dwParam2;
			*pVel = GetVertexVel( (int)dwParam1 );
		}
		return 1;
	case FX_DELETE_OBJECT:
		{
#ifndef _LTCG
			if( dwParam1 )
			{
				BsAssert( GetFxRtti() == (int)dwParam1 );
				if(GetFxRtti() != (int)dwParam1) {
					BsAssert(0 && "Fx팀 프로그래머에게 연락좀 주세요. 멈춥니다.");
					_DEBUGBREAK;
				}
				
			}
#endif //_LTCG
			m_state = ENDING;
		}
		return 1;
	case FX_WATER_IS_FINISHED:
		if(m_bInitialized == false){
			return 1;
		}
		return 0;
	}

	if( CFcFXBase::ProcessMessage( nCode, dwParam1, dwParam2, dwParam3) )
		return 1;
	
	return 0;

}
