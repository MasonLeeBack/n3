#pragma	   once

#include "BsKernel.h"
#include "BsMath.h"

#include "BsShadowMgr.h"
#include "BsLODWorld.h"
#include "BsImageProcess.h"


#define MATERIAL_NAMESIZE	256

struct VERTEX_ELEMENT
{
	UINT			Usage;
	UINT			UsageIndex;
};

struct COMPILEDFXHEADER
{
	char szFileName[MAX_PATH];
	DWORD dwFXOffset;
	DWORD dwFXSize;
};

class CBsObject;

class CBsMaterial
{
public:
	CBsMaterial();
	virtual ~CBsMaterial();

	void Clear();

protected:
	char							m_szMaterialFileName[MATERIAL_NAMESIZE]; // 버퍼
#ifdef _XBOX
	FXLEffect*						m_pEffect;
	FXLEFFECT_DESC					m_EffectDesc;

	static FXLEffectPool*			s_pEffectPool;
#else
	ID3DXEffect*					m_pEffect; // (메모리)
	D3DXEFFECT_DESC					m_EffectDesc;

	static ID3DXEffectPool*			s_pEffectPool;
#endif
	static	std::vector<EffectParam_Info>	s_SharedParameters;

	static CBsObject*				s_pHoldingObject; // Object단위...
	static int						s_nHoldingTechnique;
	static D3DXMATRIX				s_matWorld; // SubMesh단위...
	static float					s_fAlpha;
	static DWORD					s_dwTechUseFlag;

#ifdef _PACKED_RESOURCES
	static CHAR *					s_pCompiledFX;
	static COMPILEDFXHEADER *		s_pHeader;
	static CHAR *					s_pFXData;
	static DWORD					s_dwFXCount;

	HRESULT							LoadFXFromCache(const char* pszFileName);

#endif
	BOOL							m_bUseMaterialSystem;

	std::vector<VERTEX_ELEMENT>		m_InputElements[BS_MAX_TECHNIQUE]; // Vertex Input Info!! (메모리)
	D3DXHANDLE						m_hMatrixArray;				// Bone Transform Matrices (메모리)
	int								m_nMatrixArrayRegIndex[BS_MAX_TECHNIQUE];		// Bone Matrix VS Registry Index
	int								m_nMatrixArrayRegCount[BS_MAX_TECHNIQUE];
	std::vector<EffectParam_Info>	m_Parameters;
	std::vector<EffectParam_Info>	m_CustomParameters;

	std::vector<EffectSampler_Info>	m_Samplers;

	int m_nRefCount;

	void			AddParam(D3DXHANDLE hHandle, PARAMETER_TYPE nParamType);
	void			AddCustomParam(D3DXHANDLE hHandle, PARAMETER_TYPE nParamType);
	void			AddSharedParam(D3DXHANDLE hHandle, PARAMETER_TYPE nParamType);

	void			AddSampler(D3DXHANDLE hHandle, int nIndex, TEXTURE_TYPE nSamplerType);
	void			ParseEffect();
	__forceinline void SetParam(D3DXHANDLE handle, PARAMETER_TYPE type)
	{
		switch(type) {
		case PARAM_WORLD:
			SetMatrix(handle, &s_matWorld);
			break;
		case PARAM_VIEW:
#ifdef _XBOX
			DebugString("%s : Invalid Shared Param(PARAM_VIEW)\n", m_szMaterialFileName);
#else
			SetMatrix(handle, CBsKernel::GetInstance().GetParamViewMatrix() );
#endif
			break;
		case PARAM_VIEWINVERSE:
#ifdef _XBOX
			DebugString("%s : Invalid Shared Param(PARAM_VIEWINVERSE)\n", m_szMaterialFileName);
#else
			SetMatrix(handle, CBsKernel::GetInstance().GetParamInvViewMatrix() );
#endif
			break;
		case PARAM_VIEWINVERSETRANSPOSE:
#ifdef _XBOX
			DebugString("%s : Invalid Shared Param(PARAM_VIEWINVERSETRANSPOSE)\n", m_szMaterialFileName);
#else
			SetMatrix(handle, CBsKernel::GetInstance().GetParamViewMatrix() );
#endif
			break;
		case PARAM_PROJECTION:
#ifdef _XBOX
			DebugString("%s : Invalid Shared Param(PARAM_PROJECTION)\n", m_szMaterialFileName);
#else
			SetMatrix(handle, CBsKernel::GetInstance().GetParamProjectionMatrix() );
#endif
			break;
		case PARAM_WORLDVIEW:
			{
				D3DXMATRIX matWorldView;
				SetMatrix(handle, D3DXMatrixMultiply(&matWorldView, &s_matWorld, CBsKernel::GetInstance().GetParamViewMatrix()) );
			}
			break;
		case PARAM_WORLDVIEWINVERSETRANSPOSE:
			{
				D3DXMATRIX matWorldViewIT;
				D3DXMatrixMultiply(&matWorldViewIT, &s_matWorld, CBsKernel::GetInstance().GetParamViewMatrix());
				D3DXMatrixInverse(&matWorldViewIT, NULL, &matWorldViewIT);
				D3DXMatrixTranspose(&matWorldViewIT, &matWorldViewIT);
				SetMatrix(handle, &matWorldViewIT);
			}
			break;
		case PARAM_VIEWPROJECTION:
			{
#ifdef _XBOX
				DebugString("%s : Invalid Shared Param(PARAM_VIEWPROJECTION)\n", m_szMaterialFileName);
#else
				D3DXMATRIX matViewProj;
				SetMatrix(handle, D3DXMatrixMultiply(&matViewProj, CBsKernel::GetInstance().GetParamViewMatrix(), CBsKernel::GetInstance().GetParamProjectionMatrix()) );
#endif
			}
			break;
		case PARAM_WORLDVIEWPROJECTION:
			{
				D3DXMATRIX matWVP;
				SetMatrix(handle, D3DXMatrixMultiply(&matWVP, &s_matWorld, CBsKernel::GetInstance().GetParamViewProjectionMatrix()) );
			}
			break;
		case PARAM_WORLDINVERSETRANSPOSE:
			{
				// World Inverse Transpose를 World matrix로 같게 했지만, scale이 적용되면, 제대로 바꾸어야함 by jeremy
				SetMatrix(handle, &s_matWorld);
			}
			break;
		case PARAM_LIGHTDIFFUSE:
			{
#ifdef _XBOX
				DebugString("%s : Invalid Shared Param(PARAM_LIGHTDIFFUSE)\n", m_szMaterialFileName);
#else
				SetVector(handle, CBsKernel::GetInstance().GetLightDiffuse());
#endif
			}
			break;
		case PARAM_LIGHTSPECULAR:
			{
#ifdef _XBOX
				DebugString("%s : Invalid Shared Param(PARAM_LIGHTSPECULAR)\n", m_szMaterialFileName);
#else
				SetVector(handle, CBsKernel::GetInstance().GetLightSpecular());
#endif
			}
			break;
		case PARAM_LIGHTAMBIENT:			
			{
#ifdef _XBOX
				DebugString("%s : Invalid Shared Param(PARAM_LIGHTAMBIENT)\n", m_szMaterialFileName);
#else
				SetVector(handle, CBsKernel::GetInstance().GetLightAmbient());
#endif
			}
			break;
		case PARAM_CAMERAPOSITION:
#ifdef _XBOX
			DebugString("%s : Invalid Shared Param(PARAM_CAMERAPOSITION)\n", m_szMaterialFileName);
#else
			BsAssert( 0 && "Removed Parameter!!");
#endif
			break;
		case PARAM_LIGHTDIRECTION:
			{
#ifdef _XBOX
				DebugString("%s : Invalid Shared Param(PARAM_LIGHTDIRECTION)\n", m_szMaterialFileName);
#else
				SetVector(handle, CBsKernel::GetInstance().GetLightDirection());
#endif
			}
			break;
		case PARAM_VIEW_LIGHTDIRECTION:
			{
#ifdef _XBOX
				DebugString("%s : Invalid Shared Param(PARAM_VIEW_LIGHTDIRECTION)\n", m_szMaterialFileName);
#else
				D3DXVECTOR4 vecViewLight;
				D3DXVec4Transform(&vecViewLight, CBsKernel::GetInstance().GetLightDirection(), g_BsKernel.GetParamViewMatrix());
				SetVector(handle, &vecViewLight);
#endif
			}
			break;
		case PARAM_SHADOW_VIEW_LIGHTDIRECTION:
			{				
				D3DXVECTOR4 vecViewLight;
				D3DXVec4Transform(&vecViewLight, &g_BsKernel.GetShadowMgr()->GetLightDirection(), g_BsKernel.GetParamViewMatrix());
				SetVector(handle, &vecViewLight);
			}
			break;
		case PARAM_POINTLIGHT_INFO:
			{
				D3DXVECTOR4 PointInfoArray[MAX_POINT_LIGHT];
				int numLights = g_BsKernel.GetActivePointLightNum();
				memset(PointInfoArray, 0, sizeof(D3DXVECTOR4) * MAX_POINT_LIGHT);
				for( int i = 0; i < numLights; i ++) {
					D3DXVECTOR4 PointInfo = *g_BsKernel.GetActivePointLight(i);
					D3DXVec3TransformCoord((D3DXVECTOR3*)&PointInfo, (D3DXVECTOR3*)&PointInfo, g_BsKernel.GetParamViewMatrix());
					PointInfoArray[i] = PointInfo;
					PointInfoArray[i].w = 1.f / PointInfoArray[i].w;	// rcp 로 넣어준다.
				}
				SetVectorArray(handle, PointInfoArray, numLights);
			}
			break;
		case PARAM_POINTLIGHT_COLOR:
			{				
				D3DXVECTOR4 PointLightColor[MAX_POINT_LIGHT];
				int numLights = g_BsKernel.GetActivePointLightNum();
				memset(PointLightColor, 0, sizeof(D3DXVECTOR4) * MAX_POINT_LIGHT);
				for( int i = 0; i < numLights; i ++) {			
					D3DXVECTOR4 *pColor = g_BsKernel.GetActivePointLightColor(i);
					PointLightColor[i] = *pColor;
				}
				PointLightColor[0].w = (numLights + 0.5f);
				SetVectorArray(handle, PointLightColor, numLights);
			}
			break;		
		case PARAM_TIME:
			SetFloat(handle, CBsKernel::GetInstance().GetRenderTime());
			break;

		case PARAM_FOGFACTOR:
#ifdef _XBOX
			DebugString("%s : Invalid Shared Param(PARAM_FOGFACTOR)\n", m_szMaterialFileName);
#else
			SetVector(handle, CBsKernel::GetInstance().GetParamFogFactor());
#endif
			break;
		case PARAM_FOGCOLOR:
#ifdef _XBOX
			DebugString("%s : Invalid Shared Param(PARAM_FOGCOLOR)\n", m_szMaterialFileName);
#else
			SetVector(handle, CBsKernel::GetInstance().GetParamFogColor());
#endif
			break;
		case PARAM_SHADOW_ALL:
			{
				CBsShadowMgr* pShadowMgr = g_BsKernel.GetShadowMgr();
				D3DXMATRIX matShadowAll;
				if(pShadowMgr->IsShadowPass())
					SetMatrix(handle, D3DXMatrixMultiply(&matShadowAll, &s_matWorld, pShadowMgr->GetLightViewProject()) );
				else
					SetMatrix(handle, D3DXMatrixMultiply(&matShadowAll, &s_matWorld, pShadowMgr->GetLightViewProjectForSampling()));
			}
			break;
		case PARAM_SHADOW_NOWORLD_ALL:
			{
				CBsShadowMgr* pShadowMgr = g_BsKernel.GetShadowMgr();
				if(pShadowMgr->IsShadowPass())
					SetMatrix(handle, pShadowMgr->GetLightViewProject() );
				else
					SetMatrix(handle, pShadowMgr->GetLightViewProjectForSampling());
			}
			break;
		case PARAM_SHADOW_NOVIEW_ALL:
			{
				D3DXMATRIX matShadowAllNoView;
				SetMatrix(handle, CBsKernel::GetInstance().GetShadowMgr()->GetLightViewProjectNoView());
			}
			break;
		case PARAM_SHADOWMAP:
			{
				if(g_BsKernel.GetWorld())
					SetTexture(handle, g_BsKernel.GetShadowMgr()->GetShadowMap());
				else
					SetTexture(handle, (LPDIRECT3DBASETEXTURE9)g_BsKernel.GetTexturePtr(g_BsKernel.GetBlankTexture()));
			}
			break;
		case PARAM_WORLDLIGHTMAP:
			{
				int nWorldLightMap = g_BsKernel.GetWorldLightMap();
				if( nWorldLightMap == -1 ) {
					SetTexture(handle, (LPDIRECT3DBASETEXTURE9)g_BsKernel.GetTexturePtr(g_BsKernel.GetBlankTexture()));
					break;
				}
				SetTexture(handle, (LPDIRECT3DBASETEXTURE9)g_BsKernel.GetTexturePtr(nWorldLightMap));
			}
			break;
		case PARAM_SCREENBUFFER:
			{
				if(g_BsKernel.GetImageProcess()) {
					SetTexture(handle, (LPDIRECT3DBASETEXTURE9)g_BsKernel.GetTexturePtr(g_BsKernel.GetImageProcess()->GetBackBufferTextureBeforeAlphaBlendPass()));
				}
			}
			break;
		case PARAM_RCP_WORLDSIZE:
#ifdef _XBOX
			DebugString("%s : Invalid Shared Param(PARAM_RCP_WORLDSIZE)\n", m_szMaterialFileName);
#else
			if(g_BsKernel.GetWorldLightMap() != -1) {
				SetVector(handle, &D3DXVECTOR4(g_BsKernel.GetRcpWorldXSize(), g_BsKernel.GetRcpWorldZSize(), 0.f, 0.f));
			}
#endif
			break;
		case PARAM_REFLECTMAP:
			{
				SetTexture( handle,  g_BsKernel.GetReflectMap());	
				break;
			}
		}
	}	// End Switch

	void			SetMaterialName(const char* pszMaterialName);
	int				SearchCustomParameterIndex(PARAMETER_TYPE type);

public:
	void AddRef()
	{
		InterlockedIncrement( reinterpret_cast<volatile long*>(&m_nRefCount) );
	}
	int Release();

	int GetRefCount()	{	return m_nRefCount;	}

	HRESULT Reload();
	HRESULT LoadMaterial(const char* pszFileName, BOOL bUseMaterialSystem = TRUE);

	const char* GetMaterialName();
	const char* GetMaterialFullName() { return m_szMaterialFileName; }
	int         GetMaterialCustomSemanticsName(char *pName, const size_t pName_len); // char [N_MAX][32]
	int         GetMaterialSemanticsInfo(std::vector<SEM_INFO>* pInfo); // std::vector<SEM_INFO>*
	int         GetMaterialCustomSemanticsInfo(std::vector<SEM_INFO>* pInfo); // std::vector<SEM_INFO>* 

	static void CreateEffectPool();
	static void DestroyEffectPool();

#ifdef _XBOX
	FXLEffect*			GetEffect()			{	return m_pEffect;		}
	FXLEFFECT_DESC*		GetEffectDesc()		{	return &m_EffectDesc;	}
#else
	ID3DXEffect*		GetEffect()			{	return m_pEffect;		}
	D3DXEFFECT_DESC*	GetEffectDesc()		{	return &m_EffectDesc;	}
#endif

	int	GetSamplerCount()				{	return (int)m_Samplers.size();	}
	int GetTextureType(int nSamplerIndex);

	int QueryTechnique(int nBoneLinkCount);

	void SetParameters();
	static void SetSharedParameter(C3DDevice* pDevice);

	D3DXHANDLE	GetMatrixArrayHandle()	{	return m_hMatrixArray;	}
	D3DXMATRIX*	GetWorldMatrix()		{	return &s_matWorld;		}

	void		SetAlpha(float fAlpha)	{	s_fAlpha = fAlpha;	}

	bool SetBoneTransformMatrices(BSMATRIX* pBoneMatrices, int nCount);

	void SetSamplers(int* pnTextureList);

	__forceinline void SetCustomParam(int nIndex, void* pBuf)
	{
		switch(m_CustomParameters[nIndex].m_type) {
case PARAM_MATERIALAMBIENT:
	{
		D3DXVECTOR4 vecAmb = *(D3DXVECTOR4*)pBuf;
		if(s_fAlpha!=1.f) {
			vecAmb.w = s_fAlpha;
		}
		SetVector(m_CustomParameters[nIndex].m_hParam, &vecAmb);
	}
	break;

case PARAM_MATERIALDIFFUSE:
case PARAM_MATERIALSPECULAR:
case PARAM_UVANIMATION_POSITION:
case PARAM_UVANIMATION_ROTATION:
case PARAM_UVANIMATION_SCALE:						
	SetVector(m_CustomParameters[nIndex].m_hParam, (D3DXVECTOR4*)pBuf);
	break;
case PARAM_SPECULARPOWER:
case PARAM_BUMPPOWER:
case PARAM_REFLECTPOWER:
case PARAM_MORPH_WEIGHT:
case PARAM_NOISEPOWER:
case PARAM_WATERSPEED:
case PARAM_EFFECT_CUSTOM0:
case PARAM_EFFECT_CUSTOM1:
case PARAM_EFFECT_CUSTOM2:
case PARAM_EFFECT_CUSTOM3:
	SetFloat(m_CustomParameters[nIndex].m_hParam, *(float*)pBuf);
	break;
		}
	}

	void BeginMaterial(int nTechIndex, DWORD dwFlags)
	{
#ifdef _XBOX
		m_pEffect->BeginTechniqueFromIndex(nTechIndex, dwFlags);
#else
		m_pEffect->SetTechnique(m_pEffect->GetTechnique(nTechIndex));
		m_pEffect->Begin(0, dwFlags);
#endif
	}

	void BeginMaterial(D3DXHANDLE hTechnique, DWORD dwFlags)
	{
#ifdef _XBOX
		m_pEffect->BeginTechnique(hTechnique, dwFlags);
#else
		m_pEffect->SetTechnique(hTechnique);
		m_pEffect->Begin(0, dwFlags);
#endif
	}

	void EndMaterial()
	{
#ifdef _XBOX
		m_pEffect->EndTechnique();
#else
		m_pEffect->End();
#endif
	}

	void BeginPass(UINT uiPassIndex)
	{
#ifdef _XBOX
		m_pEffect->BeginPassFromIndex(uiPassIndex);
#else
		m_pEffect->BeginPass(uiPassIndex);
#endif
	}
	void EndPass()						{	m_pEffect->EndPass();			}

	void CommitChanges()
	{
#ifdef _XBOX
		m_pEffect->CommitU();
#else
		m_pEffect->CommitChanges();
#endif
	}

	// 렌더링 Object의 참조를 가지고 있다.
	void Hold(CBsObject* pObject, int nTechnique)		{	s_pHoldingObject = pObject; s_nHoldingTechnique = nTechnique;	}
	void Free()							{	s_pHoldingObject = NULL;	}

	void ActiveTechnique(int nTechIndex)	{	s_nHoldingTechnique = nTechIndex;	}
	int	 QueryActiveTechnique()				{	return s_nHoldingTechnique;	}

	std::vector<VERTEX_ELEMENT>&		GetVertexElements(int nIndex);
	int									GetStreamCount(int nTechIndex)	{	return (int)m_InputElements[nTechIndex].size();	}
	std::vector<EffectParam_Info>&		GetParameters()	{	return m_Parameters;	}
	std::vector<EffectParam_Info>&		GetCustomParameters() {	return m_CustomParameters;	}
	std::vector<EffectSampler_Info>&	GetSamplers()	{	return m_Samplers;		}

	void					GetCustomParameterValue(PARAMETER_TYPE type, void* pBuf);
	static PARAMETER_TYPE	ConvertSematicToParameter(const char* pSem);

	char*					GetSamplerSemanticName(int nSamplerIndex);

	// Wrapper Fuction!!
	D3DXHANDLE	GetParameterByName(LPCSTR pName)
	{
#ifdef _XBOX
		return m_pEffect->GetParameterHandle(pName);
#else
		return m_pEffect->GetParameterByName(NULL, pName);
#endif
	}

	D3DXHANDLE GetTechniqueHandle(int nTechIndex)
	{
#ifdef _XBOX
		return m_pEffect->GetTechniqueHandleFromIndex(nTechIndex);
#else
		return m_pEffect->GetTechnique(nTechIndex);
#endif
	}

	void SetTexture(D3DXHANDLE hParam, LPDIRECT3DBASETEXTURE9 pTexture)
	{
#ifdef _XBOX
		m_pEffect->SetSampler(hParam, pTexture);
#else
		m_pEffect->SetTexture(hParam, pTexture);
#endif
	}

	void SetTexture(D3DXHANDLE hParam, int nTextureIndex)
	{
		LPDIRECT3DBASETEXTURE9 pTexture = (LPDIRECT3DBASETEXTURE9)g_BsKernel.GetTexturePtr(nTextureIndex);
		//BsAssert( pTexture && "Invalid Texture!!");
#ifdef _XBOX
		m_pEffect->SetSampler(hParam, pTexture);
#else
		m_pEffect->SetTexture(hParam, pTexture); 
#endif
	}

	__forceinline void SetVector(D3DXHANDLE hParam, const D3DXVECTOR4* pVector)
	{
#ifdef _XBOX
		m_pEffect->SetVectorF(hParam, (FLOAT*)(pVector));
#else
		m_pEffect->SetVector(hParam, pVector);
#endif
	}

	__forceinline void SetVectorArray(D3DXHANDLE hParam, const D3DXVECTOR4* pVector, UINT uiVectorCount)
	{
#ifdef _XBOX
		m_pEffect->SetVectorArrayF(hParam, (FLOAT*)(pVector), uiVectorCount);
#else
		m_pEffect->SetVectorArray(hParam, pVector, uiVectorCount);
#endif
	}

	__forceinline void SetMatrix(D3DXHANDLE hParam, const D3DXMATRIX* pMatrix)
	{
#ifdef _XBOX
		m_pEffect->SetMatrixF(hParam, (FLOAT*)pMatrix);
#else
		m_pEffect->SetMatrix(hParam, pMatrix);
#endif
	}

	__forceinline void SetMatrixArray(D3DXHANDLE hParam, const BSMATRIX* pMatrices, UINT uiMatrixCount)
	{
#ifdef _XBOX
		m_pEffect->SetMatrixArrayF4x4A(hParam, (FLOAT*)pMatrices, uiMatrixCount);
#else
		m_pEffect->SetMatrixArray(hParam, pMatrices, uiMatrixCount);
#endif
	}

	/*__forceinline*/ void SetFloat(D3DXHANDLE hParam, float fValue)
	{
#ifdef _XBOX
		m_pEffect->SetScalarF(hParam, &fValue);
#else
		m_pEffect->SetFloat(hParam, fValue);
#endif
	}

	void SetFloatArray(D3DXHANDLE hParam, const float* pData, UINT uiCount)
	{
#ifdef _XBOX
		m_pEffect->SetScalarArrayF(hParam, (FLOAT*)pData, uiCount);
#else
		m_pEffect->SetFloatArray(hParam, pData, uiCount);
#endif
	}

	void SetInt(D3DXHANDLE hParam, int nValue)
	{
#ifdef _XBOX
		m_pEffect->SetScalarI(hParam, &nValue);
#else
		m_pEffect->SetInt(hParam, nValue);
#endif
	}
};

class CBsVertexDeclaration
{
public:
	CBsVertexDeclaration();
	virtual ~CBsVertexDeclaration();

	void AddRef();
	int Release();

	int	GetRefCount() { return m_nRefCount; }

	void Clear();
	HRESULT Reload();

	HRESULT LoadVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements);
protected:
	int m_nRefCount;

	LPDIRECT3DVERTEXDECLARATION9	m_pVertexDecl;
	D3DVERTEXELEMENT9*				m_pVertexElement;

    HRESULT	AllocDeviceResource(CONST D3DVERTEXELEMENT9* pVertexElements);
public:
	bool	IsEqualVertexElements(D3DVERTEXELEMENT9* pVertexElements);
	LPDIRECT3DVERTEXDECLARATION9	GetVertexDeclaration()	{	return m_pVertexDecl;	}
};