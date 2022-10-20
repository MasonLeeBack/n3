#include "stdafx.h"
#include "BStreamExt.h"
#include "BsSkin.h"
#include "BsKernel.h"
#include "BsMaterial.h"

#if defined(USE_SKINDBG)

#include "BsDebugBreak.h"

static Debug::DebugInfo s_skinDbgInfo[] =
{
	{ "C_LP_KF_W1.skin", Debug::OnAll, Debug::OnNothing },
	{ "", Debug::OnNothing, Debug::OnNothing }
};

#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

#define BS_SKIN_VERSION				0x1004			// submesh당 rendering 정보와 여분 공간 추가
//#define BS_SKIN_VERSION				0x1003			// submesh당 alpha 정보추가
//#define BS_SKIN_VERSION				0x1002			// custom parameter string으로 key값 변경
//#define BS_SKIN_VERSION				0x1001			// custom parameter 추가
//#define BS_SKIN_VERSION				0x1000

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CBsSkin::CBsSkin()
{
	m_szSkinFileName = NULL;
	m_szMeshFileName = NULL;
	m_nMeshIndex = -1;
	m_ppSubMesh = NULL;
#ifdef _LOAD_MAP_CHECK_
	m_nLoadSize = 0;
	m_nUseCount = 0;
#endif
	m_nRefCount = 1;
}

CBsSkin::~CBsSkin()
{
#if defined(USE_SKINDBG)
	char const * skinName = GetSkinFileName();
	Debug::TriggerEvent( s_skinDbgInfo, skinName, m_nRefCount, Debug::OnDelete );
#endif

	if (m_ppSubMesh)
	{
		for (int i=0; i<m_nSubMeshCount; ++i)
		{
			g_BsKernel.FnaPoolClear(m_ppSubMesh[i]->m_szFxFileName);

			for (int j=0; j<m_ppSubMesh[i]->m_nSamplerCount; ++j)
			{
				g_BsKernel.FnaPoolClear(m_ppSubMesh[i]->m_ppSamplers[j]->m_pSamplerFile);

				SAFE_RELEASE_TEXTURE(m_ppSubMesh[i]->m_ppSamplers[j]->m_nSamplerIndex);

				delete m_ppSubMesh[i]->m_ppSamplers[j];
			}

			SAFE_RELEASE_MATERIAL(m_ppSubMesh[i]->m_nFxIndex);

			delete [] m_ppSubMesh[i]->m_ppSamplers;

			UINT uiSize = m_ppSubMesh[i]->m_CustomParameters.size();
			for(UINT ui=0;ui<uiSize;++ui) {
				delete m_ppSubMesh[i]->m_CustomParameters[ui];
				m_ppSubMesh[i]->m_CustomParameters[ui] = NULL;
			}
			m_ppSubMesh[i]->m_CustomParameters.clear();
			delete m_ppSubMesh[i];
		}

		delete [] m_ppSubMesh;
	}

	if (m_nMeshIndex != -1)
		g_BsKernel.ReleaseMesh(m_nMeshIndex);
	if (m_szMeshFileName)
		g_BsKernel.FnaPoolClear(m_szMeshFileName);
	if (m_szSkinFileName)
        g_BsKernel.FnaPoolClear(m_szSkinFileName);
}

void CBsSkin::AddRef()
{
#if defined(_XBOX)

	int nNewRefCount = InterlockedIncrement( reinterpret_cast<volatile long*>(&m_nRefCount) );

#if defined(USE_SKINDBG)
	char const * skinName = GetSkinFileName();
	Debug::TriggerEvent( s_skinDbgInfo, skinName, nNewRefCount, Debug::OnAddRef );
#endif

#else

	++m_nRefCount;

#endif
}

int CBsSkin::Release()
{
	int nNewRefCount = InterlockedDecrement( reinterpret_cast<volatile long*>(&m_nRefCount) );

	BsAssert( nNewRefCount >= 0 );

#if defined(USE_SKINDBG)
	char const * skinName = GetSkinFileName();
	Debug::TriggerEvent( s_skinDbgInfo, skinName, nNewRefCount, Debug::OnRelease );
#endif

	return nNewRefCount;
}

int CBsSkin::LoadSkin(BStream* pStream)
{
	BsAssert(pStream);

	// 청크별로 돌면서 읽는다.
	int nVersion;
	int nChunkID;

	do {
		pStream->Read(&nChunkID, sizeof(int), ENDIAN_FOUR_BYTE); // 청크 ID

		switch (nChunkID)
		{
		case 0:	// header: version[4], meshfile[s]
			{
				pStream->Read(&nVersion, sizeof(int), ENDIAN_FOUR_BYTE);
				BsAssert(nVersion >= 0x1003 && "버젼이 틀립니다."); // 0x1004 지금 버전	 

				m_szMeshFileName = g_BsKernel.FnaPoolReadAdd(pStream);

				// load mesh	 
				m_nMeshIndex = g_BsKernel.LoadMesh(-1, m_szMeshFileName);
				if( m_nMeshIndex == -1 )
					return -1;
			}
			break;

		case 1:	// submesh
			{
				pStream->Read(&m_nSubMeshCount, sizeof(int), ENDIAN_FOUR_BYTE);

				m_ppSubMesh = new SBsSubMesh_Info* [m_nSubMeshCount];
				BsAssert(m_ppSubMesh);

				for (int i=0; i<m_nSubMeshCount; ++i)
				{
					m_ppSubMesh[i] = new SBsSubMesh_Info;
					BsAssert(m_ppSubMesh[i]);

					// default value 지정
					{
						m_ppSubMesh[i]->m_bUseAlphaBlend = false;
						m_ppSubMesh[i]->m_bUseTwoSideRender = false;
						m_ppSubMesh[i]->m_bUseAlphaAlignRender = false;
					}

					m_ppSubMesh[i]->m_szFxFileName = g_BsKernel.FnaPoolReadAdd(pStream);
					pStream->Read(&m_ppSubMesh[i]->m_nSamplerCount, sizeof(int), ENDIAN_FOUR_BYTE);

					// fx 지정
					{
						char fullName[_MAX_PATH];
						strcpy(fullName, g_BsKernel.GetShaderDirectory());
						strcat(fullName, m_ppSubMesh[i]->m_szFxFileName);

						m_ppSubMesh[i]->m_nFxIndex = g_BsKernel.LoadMaterial(fullName);
					}

					m_ppSubMesh[i]->m_ppSamplers = new SBsSampler_Info* [m_ppSubMesh[i]->m_nSamplerCount];
					BsAssert(m_ppSubMesh[i]->m_ppSamplers);

					CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(m_ppSubMesh[i]->m_nFxIndex);
					BsAssert(pMaterial);

					for (int j=0; j<m_ppSubMesh[i]->m_nSamplerCount; ++j)
					{
						m_ppSubMesh[i]->m_ppSamplers[j] = new SBsSampler_Info;
						BsAssert(m_ppSubMesh[i]->m_ppSamplers[j]);

                        m_ppSubMesh[i]->m_ppSamplers[j]->m_pSamplerFile = g_BsKernel.FnaPoolReadAdd(pStream);

						// load texture
						if (strlen(m_ppSubMesh[i]->m_ppSamplers[j]->m_pSamplerFile) > 0)
						{
							int nTextureType = pMaterial->GetTextureType(j);
							switch(nTextureType) {
								case TEXTURE_NORMAL:
									m_ppSubMesh[i]->m_ppSamplers[j]->m_nSamplerIndex = 
										g_BsKernel.LoadTexture(m_ppSubMesh[i]->m_ppSamplers[j]->m_pSamplerFile);
									break;
								case TEXTURE_CUBE:
									m_ppSubMesh[i]->m_ppSamplers[j]->m_nSamplerIndex = 
										g_BsKernel.LoadCubeTexture(m_ppSubMesh[i]->m_ppSamplers[j]->m_pSamplerFile);
									break;
								case TEXTURE_VOLUME:
									m_ppSubMesh[i]->m_ppSamplers[j]->m_nSamplerIndex = 
										g_BsKernel.LoadVolumeTexture(m_ppSubMesh[i]->m_ppSamplers[j]->m_pSamplerFile);
									break;
							}
						}
						else
						{
							m_ppSubMesh[i]->m_ppSamplers[j]->m_nSamplerIndex = -1;
							DebugString( " %s 's Sampler %d/%d Bad \n", m_szMeshFileName , i, j);
						}
					}

					// Custom Parameter Setting!!
					if (nVersion >= 0x1001)
					{
						int nParamCount;
						pStream->Read(&nParamCount, sizeof(int), ENDIAN_FOUR_BYTE);

//						pLoader->SetSubMeshCstParamCount(i, nParamCount);

						for (int j=0; j<nParamCount; ++j)
						{
							PARAMETER_TYPE readtype;
							int	 nFileNameSize = 0;
							pStream->Read(&nFileNameSize, sizeof(int), ENDIAN_FOUR_BYTE);
							char szParamName[256];

                            // Semantic 으로 Parameter의 type을 확인한다.
							pStream->Read(szParamName, nFileNameSize );
							pStream->Read(&readtype, sizeof(int), ENDIAN_FOUR_BYTE);
							PARAMETER_TYPE inputtype = CBsMaterial::ConvertSematicToParameter(szParamName);

							switch (inputtype)
							{
							case PARAM_MATERIALAMBIENT:
							case PARAM_MATERIALDIFFUSE:
							case PARAM_MATERIALSPECULAR:
							case PARAM_UVANIMATION_POSITION:			
							case PARAM_UVANIMATION_ROTATION:			
							case PARAM_UVANIMATION_SCALE:								
								{
									D3DXVECTOR4 fVec4;

									pStream->Read(&fVec4.x, sizeof(float), ENDIAN_FOUR_BYTE);
									pStream->Read(&fVec4.y, sizeof(float), ENDIAN_FOUR_BYTE);
									pStream->Read(&fVec4.z, sizeof(float), ENDIAN_FOUR_BYTE);
									pStream->Read(&fVec4.w, sizeof(float), ENDIAN_FOUR_BYTE);

									CustomParam_Info* pInfo = new CustomParam_Info(varVector);
									memcpy(pInfo->m_pValue, &fVec4, sizeof(D3DXVECTOR4));
									pInfo->m_nCustomParamIndex = j;
									pInfo->m_type = inputtype;
									m_ppSubMesh[i]->m_CustomParameters.push_back(pInfo);
								}
								break;

							case PARAM_SPECULARPOWER:
							case PARAM_BUMPPOWER:
							case PARAM_REFLECTPOWER:
							case PARAM_MORPH_WEIGHT:
							case PARAM_NOISEPOWER:
							case PARAM_WATERSPEED:
								{
									float fValue;
									pStream->Read(&fValue, sizeof(float), ENDIAN_FOUR_BYTE);
									CustomParam_Info* pInfo = new CustomParam_Info(varFloat);
									memcpy(pInfo->m_pValue, &fValue, sizeof(float));
									pInfo->m_nCustomParamIndex = j;
									pInfo->m_type = inputtype;
									m_ppSubMesh[i]->m_CustomParameters.push_back(pInfo);
								}
								break;
							case PARAM_EFFECT_CUSTOM0:
							case PARAM_EFFECT_CUSTOM1:
							case PARAM_EFFECT_CUSTOM2:
							case PARAM_EFFECT_CUSTOM3:
								break;
							default:
								BsAssert(false && "지원하지 않는 Custom Paramter입니다.");
								break;
							}
						}
					}

					// submesh별 정보 추가
					if (nVersion >= 0x1003)
					{
						BOOL bUseAlphaBlend;
						float	fAlpha;
						pStream->Read(&bUseAlphaBlend, sizeof(BOOL), ENDIAN_FOUR_BYTE);
						pStream->Read(&fAlpha, sizeof(float), ENDIAN_FOUR_BYTE);
						m_ppSubMesh[i]->m_bUseAlphaBlend = bUseAlphaBlend;
						m_ppSubMesh[i]->m_fAlpha = fAlpha;
					}

					if (nVersion >= 0x1004)
					{
						// 총 128byte의 공간을 마련한다. 이후 여기에 추가
						int nOddSize = 128;

						if (nVersion <= 0x1004)
						{
							BOOL bUseTwoSideRender;
							BOOL bUseAlphaAlignRender;
							pStream->Read(&bUseTwoSideRender, sizeof(BOOL), ENDIAN_FOUR_BYTE);
							pStream->Read(&bUseAlphaAlignRender, sizeof(BOOL), ENDIAN_FOUR_BYTE);
							m_ppSubMesh[i]->m_bUseTwoSideRender = (bUseTwoSideRender==TRUE);
							m_ppSubMesh[i]->m_bUseAlphaAlignRender = (bUseAlphaAlignRender==TRUE);

							nOddSize -= 8; // BOOL 값 2 개
						}

						// 나머지 띄우기
						pStream->Seek(nOddSize, SEEK_CUR);
					}
				}
			}
			break;

		case -1:	// end
			break;

		default:
			return -1;
		}
	} while (nChunkID != -1);

	return 0;
}

const char *CBsSkin::GetSkinFileName()
{ 
	char *pFindPtr;

	pFindPtr=strrchr(m_szSkinFileName, '\\');
	if(pFindPtr){
		return pFindPtr+1;
	}
	else{
		return m_szSkinFileName; 
	}
}

void CBsSkin::SetSkinFileName(char* pFilename)
{
	m_szSkinFileName = pFilename;

#if defined(USE_SKINDBG)

	// We do this here instead of the CBsSkin::Load function because
	// we don't have a filename at the time that the actual loading
	// is being done.

	char const * skinName = GetSkinFileName();
	Debug::TriggerEvent( s_skinDbgInfo, skinName, m_nRefCount, Debug::OnLoad );
#endif
}

D3DXVECTOR4 CBsSkin::GetSkinMaterialAVParam(PARAMETER_TYPE type)
{
	D3DXVECTOR4 Return = D3DXVECTOR4(0, 0, 0, 0);
	D3DXVECTOR4* pColor = new D3DXVECTOR4[m_nSubMeshCount];
	for (int i = 0; i < m_nSubMeshCount; i++) {
		for (int j = 0; j < (int)m_ppSubMesh[i]->m_CustomParameters.size(); j++) {
			CustomParam_Info* pInfo = m_ppSubMesh[i]->m_CustomParameters[j];
			if (pInfo->m_type == type) {
				pColor[i] = (*(D3DXVECTOR4*)pInfo->m_pValue);
			}
		}
	}
	for (int i = 0; i < m_nSubMeshCount; i++) {
		Return += pColor[i];
	}
	Return.x = Return.x / m_nSubMeshCount;
	Return.y = Return.y / m_nSubMeshCount;
	Return.z = Return.z / m_nSubMeshCount;
	Return.w = Return.w / m_nSubMeshCount;
	delete [] pColor;
	pColor = NULL;
	return Return;
}

void CBsSkin::GetTextureList( std::vector< std::string > *pNameList )
{
	int i, j;
	std::string szFileName;

	for( i = 0; i < m_nSubMeshCount; i++ )
	{
		if( m_ppSubMesh[ i ] )
		{
			for( j = 0; j < m_ppSubMesh[ i ]->m_nSamplerCount; j++ )
			{
				if( m_ppSubMesh[ i ]->m_ppSamplers[ j ]->m_pSamplerFile )
				{
					szFileName = m_ppSubMesh[ i ]->m_ppSamplers[ j ]->m_pSamplerFile;
					pNameList->push_back( szFileName );
				}
			}
		}
	}
}
