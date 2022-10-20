#include "stdafx.h"
#include "BsMaterial.h"
#include "BsObject.h"
#include "SemanticsDefine.h"
#include "BsShadowMgr.h"
#include "BsLODWorld.h"
#include "BsImageProcess.h"
#include "BsFileIO.h"

CBsObject*			CBsMaterial::s_pHoldingObject = NULL;
int					CBsMaterial::s_nHoldingTechnique = 0;
float				CBsMaterial::s_fAlpha = 1.f;
DWORD				CBsMaterial::s_dwTechUseFlag = 0xFFFFFFFF;

#ifdef _PACKED_RESOURCES
CHAR *				CBsMaterial::s_pCompiledFX = NULL;
COMPILEDFXHEADER *	CBsMaterial::s_pHeader = NULL;
CHAR *				CBsMaterial::s_pFXData = NULL;
DWORD				CBsMaterial::s_dwFXCount = 0;
#endif

#ifdef _XBOX
FXLEffectPool*		CBsMaterial::s_pEffectPool = NULL;
#else
ID3DXEffectPool*	CBsMaterial::s_pEffectPool = NULL;
#endif
std::vector<EffectParam_Info>	CBsMaterial::s_SharedParameters;

D3DXMATRIX CBsMaterial::s_matWorld = D3DXMATRIX(1.f, 0.f, 0.f, 0.f,
												0.f, 1.f, 0.f, 0.f,
												0.f, 0.f, 1.f, 0.f,
												0.f, 0.f, 0.f, 1.f );

CBsMaterial::CBsMaterial()
{
	m_szMaterialFileName[0]=NULL;
	m_pEffect	= NULL;
	m_nRefCount	= 0;
	m_hMatrixArray = NULL;
	
	for(int i=0;i<BS_MAX_TECHNIQUE;++i) {
		m_nMatrixArrayRegIndex[i] = -1;
		m_nMatrixArrayRegCount[i] = 0;
	}

	m_bUseMaterialSystem = FALSE;
}

CBsMaterial::~CBsMaterial()
{
	Clear();
}

void CBsMaterial::Clear()
{
	for(int i=0;i<BS_MAX_TECHNIQUE;++i) {
		m_InputElements[i].clear();
	}
	m_Parameters.clear();
	m_Samplers.clear();
	m_CustomParameters.clear();

	SAFE_RELEASE(m_pEffect);
}

int CBsMaterial::Release()
{
	int nNewRefCount = InterlockedDecrement( reinterpret_cast<volatile long*>(&m_nRefCount) );

	BsAssert( nNewRefCount >= 0 );

	return nNewRefCount;
}

void CBsMaterial::AddParam(D3DXHANDLE hHandle, PARAMETER_TYPE nParamType)
{
	EffectParam_Info info;

	info.m_hParam = hHandle;
	info.m_type	  = nParamType;
	info.m_dwTechUseFlag = s_dwTechUseFlag;
	m_Parameters.push_back(info);
}

void CBsMaterial::AddSharedParam(D3DXHANDLE hHandle, PARAMETER_TYPE nParamType)
{
	UINT uiCount = s_SharedParameters.size();
	bool bFound = false;
#ifdef _XBOX
	FXLPARAMETER_DESC paramdesc;
	m_pEffect->GetParameterDesc(hHandle, &paramdesc);
	if(!(paramdesc.Flags & FXLPFLAG_SHARED)) {
		BsAssert( 0 && "Not Shared parameter");
		DebugString("%s : Not Shared Parameter!!\n", m_szMaterialFileName);
	}
#endif
	for(UINT i=0;i<uiCount;++i) {
		if(s_SharedParameters[i].m_type == nParamType) {
			// 있는 데이타입니다. 바로 리턴!!
			if(s_SharedParameters[i].m_hParam != hHandle) {
				DebugString("%s : Check Shared Parameter Rule!!!\n", m_szMaterialFileName);
			}
			return;
		}
	}
	// 없는 Type입니다. 추가합니다.
	EffectParam_Info info;
	info.m_hParam = hHandle;
	info.m_type = nParamType;
	info.m_dwTechUseFlag = s_dwTechUseFlag;
//	s_pEffectPool->SetParameterRegisterUpdate(hHandle, FXLREGUPDATE_MANUAL);
	s_SharedParameters.push_back(info);
}


void CBsMaterial::AddCustomParam(D3DXHANDLE hHandle, PARAMETER_TYPE nParamType)
{
	EffectParam_Info info;
	info.m_hParam = hHandle;
	info.m_type = nParamType;
	m_CustomParameters.push_back(info);	// [PREFIX:nomodify] 2006/2/15 junyash PS#5169 actually this method push_back uninitialized info.m_dwTechUseFlag, but not using m_dwTechUseFlag on m_CustomParameters.
}

void CBsMaterial::AddSampler(D3DXHANDLE hHandle, int nIndex, TEXTURE_TYPE nSamplerType)
{
	EffectSampler_Info info;

	info.m_hParam = hHandle;
	info.m_nSamplerIndex = nIndex;
	info.m_type = nSamplerType;
	m_Samplers.push_back(info);
}

#ifdef _XBOX
void CBsMaterial::ParseEffect()
{
	BsAssert(m_pEffect);
	//**************************************************************
	// Input Stream 정보 얻어온다.
	//**************************************************************
	FXLHANDLE			hTechnique;
	FXLTECHNIQUE_DESC	TechniqueDesc;
	FXLHANDLE			hPass, hPassInTech[BS_MAX_TECHNIQUE];
	FXLPASS_DESC		PassDesc;

	BsAssert( m_EffectDesc.Techniques <= BS_MAX_TECHNIQUE && "Technique 갯수 제한!!");
	for( UINT iTech = 0; iTech < m_EffectDesc.Techniques; iTech++ ) {
		hTechnique = m_pEffect->GetTechniqueHandleFromIndex( iTech );
		m_pEffect->GetTechniqueDesc( hTechnique, &TechniqueDesc );
		BsAssert( TechniqueDesc.Passes == 1 && "현재 Pass는 1개만 지원!!" );
		for( UINT iPass = 0; iPass < TechniqueDesc.Passes; iPass++ ) {
			hPass = m_pEffect->GetPassHandleFromIndex( hTechnique, iPass );
			hPassInTech[ iTech ] = hPass;
			m_pEffect->GetPassDesc( hPass, &PassDesc );

			UINT NumVSSemanticsUsed;
			D3DXSEMANTIC pVSSemantics[MAXD3DDECLLENGTH];

			if( !PassDesc.pVertexShaderFunction ||
				FAILED( D3DXGetShaderInputSemantics( PassDesc.pVertexShaderFunction, pVSSemantics, &NumVSSemanticsUsed ) ) )
			{
				continue;
			}

			for( UINT iSem = 0; iSem < NumVSSemanticsUsed; iSem++ ) {
				// Vertex Declaration!!
				VERTEX_ELEMENT info;
				info.Usage = pVSSemantics[iSem].Usage;
				info.UsageIndex = pVSSemantics[iSem].UsageIndex;
				m_InputElements[iTech].push_back(info);
			}
		}
	}

	//**************************************************
	// Parameter 갯수와 Desc얻어온다.
	// Sampler 갯수와 Desc얻어온다.
	//**************************************************
	FXLHANDLE			hParam;
	FXLPARAMETER_DESC	ParamDesc;
	int					nTextureIndex = 0;
	for( UINT uiParam = 0; uiParam < m_EffectDesc.Parameters; uiParam++ ) {
		hParam = m_pEffect->GetParameterHandleFromIndex( uiParam );
		m_pEffect->GetParameterDesc( hParam, &ParamDesc );

		s_dwTechUseFlag = 0;
		for( UINT iTech = 0; iTech < m_EffectDesc.Techniques; iTech++) {
			FXLPARAMETER_CONTEXT Context = m_pEffect->GetParameterContext( hPassInTech[iTech], hParam);
			if( Context != 0 ) {
				s_dwTechUseFlag |= (1 << iTech);
			}
		}
		//********************************************************************
		//	Effect compile시에 D3DXSHADER_FXLPARAMETERS_AS_VARIABLE_NAMES 를
		//	setting해서는 안된다.
		//	왜냐하면, pName이 Variable Name으로 setting이 되기 때문이다.
		//********************************************************************

		// Matrix Class
		if( ParamDesc.pName != NULL && (ParamDesc.Class == FXLDCLASS_RMATRIX || ParamDesc.Class == FXLDCLASS_CMATRIX)) {
			if( _strcmpi( ParamDesc.pName, SEM_WORLD ) == 0 ) {
				AddParam(hParam, PARAM_WORLD);
			}
			else if( _strcmpi( ParamDesc.pName, SEM_VIEW ) == 0 ) {
				AddSharedParam(hParam, PARAM_VIEW);									//	Shared
			}
			else if( _strcmpi( ParamDesc.pName, SEM_VIEWINVERSE ) == 0 ) {
				AddSharedParam(hParam, PARAM_VIEWINVERSE);							//	Shared
			}
			else if( _strcmpi( ParamDesc.pName, SEM_VIEWINVERSETRANSPOSE ) == 0 ) {
				AddSharedParam(hParam, PARAM_VIEWINVERSETRANSPOSE);					//	Shared
			}
			else if( _strcmpi( ParamDesc.pName, SEM_PROJECTION ) == 0 ) {
				AddSharedParam(hParam, PARAM_PROJECTION);							//	Shared
			}
			else if( _strcmpi( ParamDesc.pName, SEM_WORLDVIEW ) == 0 ) {
				AddParam(hParam, PARAM_WORLDVIEW);
			}
			else if( _strcmpi( ParamDesc.pName, SEM_WORLDVIEWINVERSETRANSPOSE ) == 0 ) {
				AddParam(hParam, PARAM_WORLDVIEWINVERSETRANSPOSE);
			}
			else if( _strcmpi( ParamDesc.pName, SEM_VIEWPROJECTION ) == 0 ) {
				AddSharedParam(hParam, PARAM_VIEWPROJECTION);						//	Shared
			}
			else if( _strcmpi( ParamDesc.pName, SEM_WORLDVIEWPROJECTION ) == 0 ) {
				AddParam(hParam, PARAM_WORLDVIEWPROJECTION);
			}
			else if( _strcmpi( ParamDesc.pName, SEM_WORLDINVERSETRANSPOSE ) == 0 ) {
				AddParam(hParam, PARAM_WORLDINVERSETRANSPOSE);
			}
			else if( _strcmpi( ParamDesc.pName, SEM_SHADOW_ALL ) == 0 ) {
				AddParam(hParam, PARAM_SHADOW_ALL);
			}
			else if( _strcmpi( ParamDesc.pName, SEM_SHADOW_NOWORLD_ALL ) == 0 ) {
				AddParam(hParam, PARAM_SHADOW_NOWORLD_ALL);
			}
			else if( _strcmpi( ParamDesc.pName, SEM_SHADOW_NOVIEW_ALL ) == 0 ) {
				AddParam(hParam, PARAM_SHADOW_NOVIEW_ALL);
			}		
			else {
				//				assert( 0 && "Invalid Matrix Parameter!!" );
			}
		}

		if( ParamDesc.pName != NULL && ParamDesc.Class == FXLDCLASS_CONTAINER && ParamDesc.Content == FXLPACONTENT_ARRAY) {
			if( _strcmpi( ParamDesc.pName, SEM_WORLDMATRIXARRAY ) == 0 ) {
				m_hMatrixArray = hParam;
				m_pEffect->SetParameterRegisterUpdate(m_hMatrixArray, FXLREGUPDATE_MANUAL);
				UINT nVSRegIndex = -1;
				UINT nVSRegCount = -1;
				for(UINT i=0;i<m_EffectDesc.Techniques;++i) {
					DWORD dwCurrentTechBit = 1 << i;
					if( s_dwTechUseFlag & dwCurrentTechBit ) {
						FXLHANDLE hTech = m_pEffect->GetTechniqueHandleFromIndex(i);
						FXLHANDLE hPass = m_pEffect->GetPassHandleFromIndex(hTech, 0);			// Pass 0 만 쓴다.
						FXLPARAMETER_CONTEXT shaderfrag = m_pEffect->GetParameterContext(hPass, m_hMatrixArray);

						if(shaderfrag & FXLPCONTEXT_VERTEXSHADERCONSTANTF) {
							m_pEffect->GetParameterRegister(hPass, m_hMatrixArray, FXLPCONTEXT_VERTEXSHADERCONSTANTF, &nVSRegIndex, &nVSRegCount);
							m_nMatrixArrayRegIndex[i] = nVSRegIndex;
							m_nMatrixArrayRegCount[i] = nVSRegCount;
						}
					}
				}	// End for
			}
			else if( _strcmpi( ParamDesc.pName, SEM_POINTLIGHT_INFO) == 0 ) {
				AddParam(hParam, PARAM_POINTLIGHT_INFO);
			}
			else if( _strcmpi( ParamDesc.pName, SEM_POINTLIGHT_COLOR) == 0 ) {
				AddParam(hParam, PARAM_POINTLIGHT_COLOR);
			}		
		}


		// Vector Class
		if( ParamDesc.pName != NULL && ( ParamDesc.Class == D3DXPC_VECTOR )) {
			if( _strcmpi( ParamDesc.pName, SEM_MATERIALAMBIENT ) == 0 ) {
				AddCustomParam(hParam, PARAM_MATERIALAMBIENT);
			}
			else if( _strcmpi( ParamDesc.pName, SEM_MATERIALDIFFUSE ) == 0 ) {
				AddCustomParam(hParam, PARAM_MATERIALDIFFUSE);
			}
			else if( _strcmpi( ParamDesc.pName, SEM_MATERIALSPECULAR ) == 0 ) {
				AddCustomParam(hParam, PARAM_MATERIALSPECULAR);
			}
			else if( _strcmpi( ParamDesc.pName, SEM_RCP_WORLDSIZE ) == 0 ) {
				AddSharedParam(hParam, PARAM_RCP_WORLDSIZE);							//	Shared
			}
			else if( _strcmpi( ParamDesc.pName, SEM_LIGHTDIRECTION ) == 0 ) {
				AddSharedParam(hParam, PARAM_LIGHTDIRECTION);							//	Shared
			}
			else if( _strcmpi( ParamDesc.pName, SEM_VIEW_LIGHTDIRECTION ) == 0 ) {
				AddSharedParam(hParam, PARAM_VIEW_LIGHTDIRECTION);						//	Shared
			}
			else if( _strcmpi( ParamDesc.pName, SEM_SHADOW_VIEW_LIGHTDIRECTION) == 0 ) {	
				AddParam(hParam, PARAM_SHADOW_VIEW_LIGHTDIRECTION);
			}
			else if( _strcmpi( ParamDesc.pName, SEM_LIGHTDIFFUSE ) == 0 ) {
				AddSharedParam(hParam, PARAM_LIGHTDIFFUSE);								//	Shared
			}
			else if( _strcmpi( ParamDesc.pName, SEM_LIGHTSPECULAR ) == 0 ) {
				AddSharedParam(hParam, PARAM_LIGHTSPECULAR);							//	Shared
			}
			else if( _strcmpi( ParamDesc.pName, SEM_LIGHTAMBIENT ) == 0 ) {
				AddSharedParam(hParam, PARAM_LIGHTAMBIENT);								//	Shared
			}
			else if( _strcmpi( ParamDesc.pName, SEM_CAMERAPOSITION ) == 0 ) {
				AddSharedParam(hParam, PARAM_CAMERAPOSITION);							//	Shared
			}
			else if( _strcmpi( ParamDesc.pName, SEM_FOGFACTOR ) == 0 ) {
				AddSharedParam(hParam, PARAM_FOGFACTOR);								//	Shared
			}
			else if( _strcmpi( ParamDesc.pName, SEM_FOGCOLOR ) == 0 ) {
				AddSharedParam(hParam, PARAM_FOGCOLOR);									//	Shared
			}
			else if( _strcmpi( ParamDesc.pName, SEM_UVANIMATION_POSITION ) == 0 ) {
				AddCustomParam(hParam, PARAM_UVANIMATION_POSITION);
			}
			else if( _strcmpi( ParamDesc.pName, SEM_UVANIMATION_ROTATION ) == 0 ) {
				AddCustomParam(hParam, PARAM_UVANIMATION_ROTATION);
			}
			else if( _strcmpi( ParamDesc.pName, SEM_UVANIMATION_SCALE    ) == 0 ) {
				AddCustomParam(hParam, PARAM_UVANIMATION_SCALE);
			}			
			else {
				//				assert( 0 && "Invalid Vector Parameter!!" );
			}
		}

		// Scalar Class
		if( ParamDesc.pName != NULL && ( ParamDesc.Class == D3DXPC_SCALAR )) {
			if( _strcmpi( ParamDesc.pName, SEM_SPECULARPOWER ) == 0 ) {
				AddCustomParam(hParam, PARAM_SPECULARPOWER);
			}
			else if( _strcmpi( ParamDesc.pName, SEM_BUMPPOWER ) == 0 ) {
				AddCustomParam(hParam, PARAM_BUMPPOWER);
			}
			else if( _strcmpi( ParamDesc.pName, SEM_REFLECTPOWER ) == 0 ) {
				AddCustomParam(hParam, PARAM_REFLECTPOWER);
			}
			else if( _strcmpi( ParamDesc.pName, SEM_TIME ) == 0 ) {
				AddParam(hParam, PARAM_TIME);
			}
			else if( _strcmpi( ParamDesc.pName, SEM_MORPH_WEIGHT ) == 0 ) {
				AddCustomParam(hParam, PARAM_MORPH_WEIGHT);
			}
			else if( _strcmpi( ParamDesc.pName, SEM_EFFECT_CUSTOM0 ) == 0 ) {
				AddCustomParam(hParam, PARAM_EFFECT_CUSTOM0);
			}
			else if( _strcmpi( ParamDesc.pName, SEM_EFFECT_CUSTOM1 ) == 0 ) {
				AddCustomParam(hParam, PARAM_EFFECT_CUSTOM1);
			}
			else if( _strcmpi( ParamDesc.pName, SEM_EFFECT_CUSTOM2 ) == 0 ) {
				AddCustomParam(hParam, PARAM_EFFECT_CUSTOM2);
			}
			else if( _strcmpi( ParamDesc.pName, SEM_EFFECT_CUSTOM3 ) == 0 ) {
				AddCustomParam(hParam, PARAM_EFFECT_CUSTOM3);
			}
			else if( _strcmpi( ParamDesc.pName, SEM_NOISEPOWER) == 0 ) {
				AddCustomParam(hParam, PARAM_NOISEPOWER);
			}
			else if( _strcmpi( ParamDesc.pName, SEM_WATERSPEED) == 0 ) {
				AddCustomParam(hParam, PARAM_WATERSPEED);
			}

			else {
				//				assert( 0 && "Invalid Scalar Parameter!!" );
			}
		}

		// Sampler Class
		/*
		if(ParamDesc.Class == D3DXPC_OBJECT && (ParamDesc.Type == D3DXPT_TEXTURE ||
		ParamDesc.Type == D3DXPT_TEXTURE1D || ParamDesc.Type == D3DXPT_TEXTURE2D ||
		ParamDesc.Type == D3DXPT_TEXTURE3D || ParamDesc.Type == D3DXPT_TEXTURECUBE))
		{
		if(ParamDesc.pName==NULL) {
		AddSampler(hParam, nTextureIndex, TEXTURE_NORMAL);
		}
		else if(_strcmpi( ParamDesc.pName, "Environment" ) == 0 ) {
		AddSampler(hParam, nTextureIndex, TEXTURE_CUBE);
		}
		else if(_strcmpi( ParamDesc.pName, "Volume" ) == 0 ) {
		AddSampler(hParam, nTextureIndex, TEXTURE_VOLUME);
		}
		else {
		AddSampler(hParam, nTextureIndex, TEXTURE_NORMAL);
		}
		nTextureIndex++;
		}
		*/
		if(ParamDesc.Type == FXLDTYPE_SAMPLER) {
			// 먼저 Kernel이 관리하는 Sampler인지 검색후 Custom Sampler쪽으로 보낸다.
			if(_strcmpi( ParamDesc.pName, "shadowSampler") == 0) {
				AddParam(hParam, PARAM_SHADOWMAP);							//	Shared
			}
			else if(_strcmpi( ParamDesc.pName, "lightmapSampler") == 0) {
				AddParam(hParam, PARAM_WORLDLIGHTMAP);							//	Shared
			}
			else if(_strcmpi( ParamDesc.pName, "ScreenSampler") == 0) {
				AddParam(hParam, PARAM_SCREENBUFFER );
			}
			else if(_strcmpi( ParamDesc.pName, "reflectmapSampler") == 0) {
				AddParam(hParam, PARAM_REFLECTMAP);
			}
			else {
				switch(ParamDesc.Class) {
					case FXLDCLASS_SAMPLER:
					case FXLDCLASS_SAMPLER1D:
					case FXLDCLASS_SAMPLER2D:
						AddSampler(hParam, nTextureIndex, TEXTURE_NORMAL);
						break;
					case FXLDCLASS_SAMPLER3D:
						AddSampler(hParam, nTextureIndex, TEXTURE_VOLUME);
						break;
					case FXLDCLASS_SAMPLERCUBE:
						AddSampler(hParam, nTextureIndex, TEXTURE_CUBE);
						break;
					default:
						BsAssert( 0 && "안준영한테 연락바람" );
						break;
				}
				nTextureIndex++;
			}
		}
	}	// end for()
}
#else	// PC D3D Effect Framework
void CBsMaterial::ParseEffect()
{
	BsAssert(m_pEffect);
	//**************************************************************
	// Input Stream 정보 얻어온다.
	//**************************************************************
	D3DXHANDLE hTechnique;
	D3DXTECHNIQUE_DESC TechniqueDesc;
	D3DXHANDLE hPass;
	D3DXPASS_DESC PassDesc;

	BsAssert( m_EffectDesc.Techniques <= BS_MAX_TECHNIQUE && "Technique 갯수 제한!!");
	for( UINT iTech = 0; iTech < m_EffectDesc.Techniques; iTech++ ) {
		hTechnique = m_pEffect->GetTechnique( iTech );
		m_pEffect->GetTechniqueDesc( hTechnique, &TechniqueDesc );
		BsAssert( TechniqueDesc.Passes == 1 && "현재 Pass는 1개만 지원!!" );
		for( UINT iPass = 0; iPass < TechniqueDesc.Passes; iPass++ ) {
			hPass = m_pEffect->GetPass( hTechnique, iPass );
			m_pEffect->GetPassDesc( hPass, &PassDesc );

			UINT NumVSSemanticsUsed;
			D3DXSEMANTIC pVSSemantics[MAXD3DDECLLENGTH];

			if( !PassDesc.pVertexShaderFunction ||
				FAILED( D3DXGetShaderInputSemantics( PassDesc.pVertexShaderFunction, pVSSemantics, &NumVSSemanticsUsed ) ) )
			{
				continue;
			}

			for( UINT iSem = 0; iSem < NumVSSemanticsUsed; iSem++ ) {
				// Vertex Declaration!!
				VERTEX_ELEMENT info;
				info.Usage = pVSSemantics[iSem].Usage;
				info.UsageIndex = pVSSemantics[iSem].UsageIndex;
				m_InputElements[iTech].push_back(info);
			}
		}
	}
	//**************************************************
	// Parameter 갯수와 Desc얻어온다.
	// Sampler 갯수와 Desc얻어온다.
	//**************************************************
	D3DXHANDLE			hParam;
	D3DXPARAMETER_DESC	ParamDesc;
	int					nTextureIndex = 0;
	for( UINT uiParam = 0; uiParam < m_EffectDesc.Parameters; uiParam++ ) {
		hParam = m_pEffect->GetParameter ( NULL, uiParam );
		m_pEffect->GetParameterDesc( hParam, &ParamDesc );

		// Matrix Class
		if( ParamDesc.Semantic != NULL && 
			( ParamDesc.Class == D3DXPC_MATRIX_ROWS || ParamDesc.Class == D3DXPC_MATRIX_COLUMNS ) ) 
		{
			if( _strcmpi( ParamDesc.Semantic, SEM_WORLD ) == 0 ) {
				AddParam(hParam, PARAM_WORLD);
			}
			else if( _strcmpi( ParamDesc.Semantic, SEM_WORLDMATRIXARRAY ) == 0 ) {
				m_hMatrixArray = hParam;
			}
			else if( _strcmpi( ParamDesc.Semantic, SEM_VIEW ) == 0 ) {
				AddParam(hParam, PARAM_VIEW);
			}
			else if( _strcmpi( ParamDesc.Semantic, SEM_VIEWINVERSE ) == 0 ) {
				AddParam(hParam, PARAM_VIEWINVERSE);
			}
			else if( _strcmpi( ParamDesc.Semantic, SEM_VIEWINVERSETRANSPOSE ) == 0 ) {
				AddParam(hParam, PARAM_VIEWINVERSETRANSPOSE);
			}
			else if( _strcmpi( ParamDesc.Semantic, SEM_PROJECTION ) == 0 ) {
				AddParam(hParam, PARAM_PROJECTION);
			}
			else if( _strcmpi( ParamDesc.Semantic, SEM_WORLDVIEW ) == 0 ) {
				AddParam(hParam, PARAM_WORLDVIEW);
			}
			else if( _strcmpi( ParamDesc.Semantic, SEM_WORLDVIEWINVERSETRANSPOSE ) == 0 ) {
				AddParam(hParam, PARAM_WORLDVIEWINVERSETRANSPOSE);
			}
			else if( _strcmpi( ParamDesc.Semantic, SEM_VIEWPROJECTION ) == 0 ) {
				AddParam(hParam, PARAM_VIEWPROJECTION);
			}
			else if( _strcmpi( ParamDesc.Semantic, SEM_WORLDVIEWPROJECTION ) == 0 ) {
				AddParam(hParam, PARAM_WORLDVIEWPROJECTION);
			}
			else if( _strcmpi( ParamDesc.Semantic, SEM_WORLDINVERSETRANSPOSE ) == 0 ) {
				AddParam(hParam, PARAM_WORLDINVERSETRANSPOSE);
			}
			else if( _strcmpi( ParamDesc.Semantic, SEM_SHADOW_ALL ) == 0 ) {
				AddParam(hParam, PARAM_SHADOW_ALL);
			}
			else if( _strcmpi( ParamDesc.Semantic, SEM_SHADOW_NOWORLD_ALL ) == 0 ) {
				AddParam(hParam, PARAM_SHADOW_NOWORLD_ALL);
			}
			else if( _strcmpi( ParamDesc.Semantic, SEM_SHADOW_NOVIEW_ALL ) == 0 ) {
				AddParam(hParam, PARAM_SHADOW_NOVIEW_ALL);
			}
			else {
				//				assert( 0 && "Invalid Matrix Parameter!!" );
			}
		}

		// Vector Class
		if( ParamDesc.Semantic != NULL && ( ParamDesc.Class == D3DXPC_VECTOR )) {
			if( _strcmpi( ParamDesc.Semantic, SEM_MATERIALAMBIENT ) == 0 ) {
				AddCustomParam(hParam, PARAM_MATERIALAMBIENT);
			}
			else if( _strcmpi( ParamDesc.Semantic, SEM_MATERIALDIFFUSE ) == 0 ) {
				AddCustomParam(hParam, PARAM_MATERIALDIFFUSE);
			}
			else if( _strcmpi( ParamDesc.Semantic, SEM_MATERIALSPECULAR ) == 0 ) {
				AddCustomParam(hParam, PARAM_MATERIALSPECULAR);
			}
			else if( _strcmpi( ParamDesc.Semantic, SEM_RCP_WORLDSIZE ) == 0 ) {
				AddParam(hParam, PARAM_RCP_WORLDSIZE);
			}
			else if( _strcmpi( ParamDesc.Semantic, SEM_LIGHTDIRECTION ) == 0 ) {
				AddParam(hParam, PARAM_LIGHTDIRECTION);
			}
			else if( _strcmpi( ParamDesc.Semantic, SEM_VIEW_LIGHTDIRECTION ) == 0 ) {
				AddParam(hParam, PARAM_VIEW_LIGHTDIRECTION);
			}			
			else if( _strcmpi( ParamDesc.Semantic, SEM_SHADOW_VIEW_LIGHTDIRECTION) == 0 ) {
				AddParam(hParam, PARAM_SHADOW_VIEW_LIGHTDIRECTION);
			}
			else if( _strcmpi( ParamDesc.Semantic, SEM_LIGHTDIFFUSE ) == 0 ) {
				if( _strcmpi(ParamDesc.Name, "LightColor" ) == 0 ) {
					AddParam(hParam, PARAM_LIGHTDIFFUSE);
				}
			}
			else if( _strcmpi( ParamDesc.Semantic, SEM_LIGHTSPECULAR ) == 0 ) {
				if( _strcmpi(ParamDesc.Name, "LightSpecular" ) == 0 ) {
					AddParam(hParam, PARAM_LIGHTSPECULAR);
				}
			}
			else if( _strcmpi( ParamDesc.Semantic, SEM_LIGHTAMBIENT ) == 0 ) {
				if( _strcmpi(ParamDesc.Name, "LightAmbient" ) == 0 ) {
					AddParam(hParam, PARAM_LIGHTAMBIENT);
				}
			}
			else if( _strcmpi( ParamDesc.Semantic, SEM_CAMERAPOSITION ) == 0 ) {
				AddParam(hParam, PARAM_CAMERAPOSITION);
			}
			else if( _strcmpi( ParamDesc.Semantic, SEM_FOGFACTOR ) == 0 ) {
				AddParam(hParam, PARAM_FOGFACTOR);
			}
			else if( _strcmpi( ParamDesc.Semantic, SEM_FOGCOLOR ) == 0 ) {
				AddParam(hParam, PARAM_FOGCOLOR);
			}
			else if( _strcmpi( ParamDesc.Semantic, SEM_UVANIMATION_POSITION ) == 0 ) {
				AddCustomParam(hParam, PARAM_UVANIMATION_POSITION);
			}
			else if( _strcmpi( ParamDesc.Semantic, SEM_UVANIMATION_ROTATION ) == 0 ) {
				AddCustomParam(hParam, PARAM_UVANIMATION_ROTATION);
			}
			else if( _strcmpi( ParamDesc.Semantic, SEM_UVANIMATION_SCALE    ) == 0 ) {
				AddCustomParam(hParam, PARAM_UVANIMATION_SCALE);
			}			
			else {
				//				assert( 0 && "Invalid Vector Parameter!!" );
			}
		}

		// Scalar Class
		if( ParamDesc.Semantic != NULL && ( ParamDesc.Class == D3DXPC_SCALAR )) {
			if( _strcmpi( ParamDesc.Semantic, SEM_SPECULARPOWER ) == 0 ) {
				AddCustomParam(hParam, PARAM_SPECULARPOWER);
			}
			else if( _strcmpi( ParamDesc.Semantic, SEM_BUMPPOWER ) == 0 ) {
				AddCustomParam(hParam, PARAM_BUMPPOWER);
			}
			else if( _strcmpi( ParamDesc.Semantic, SEM_REFLECTPOWER ) == 0 ) {
				AddCustomParam(hParam, PARAM_REFLECTPOWER);
			}
			else if( _strcmpi( ParamDesc.Semantic, SEM_TIME ) == 0 ) {
				AddParam(hParam, PARAM_TIME);
			}
			else if( _strcmpi( ParamDesc.Semantic, SEM_MORPH_WEIGHT ) == 0 ) {
				AddCustomParam(hParam, PARAM_MORPH_WEIGHT);
			}
			else if( _strcmpi( ParamDesc.Semantic, SEM_EFFECT_CUSTOM0 ) == 0 ) {
				AddCustomParam(hParam, PARAM_EFFECT_CUSTOM0);
			}
			else if( _strcmpi( ParamDesc.Semantic, SEM_EFFECT_CUSTOM1 ) == 0 ) {
				AddCustomParam(hParam, PARAM_EFFECT_CUSTOM1);
			}
			else if( _strcmpi( ParamDesc.Semantic, SEM_EFFECT_CUSTOM2 ) == 0 ) {
				AddCustomParam(hParam, PARAM_EFFECT_CUSTOM2);
			}
			else if( _strcmpi( ParamDesc.Semantic, SEM_EFFECT_CUSTOM3 ) == 0 ) {
				AddCustomParam(hParam, PARAM_EFFECT_CUSTOM3);
			}
			else if( _strcmpi( ParamDesc.Semantic, SEM_NOISEPOWER) == 0 ) {
				AddCustomParam(hParam, PARAM_NOISEPOWER);
			}
			else if( _strcmpi( ParamDesc.Semantic, SEM_WATERSPEED) == 0 ) {
				AddCustomParam(hParam, PARAM_WATERSPEED);
			}
			else {
				//				assert( 0 && "Invalid Scalar Parameter!!" );
			}
		}

		// Sampler Class
		if(ParamDesc.Class == D3DXPC_OBJECT && (ParamDesc.Type == D3DXPT_TEXTURE ||
			ParamDesc.Type == D3DXPT_TEXTURE1D || ParamDesc.Type == D3DXPT_TEXTURE2D ||
			ParamDesc.Type == D3DXPT_TEXTURE3D || ParamDesc.Type == D3DXPT_TEXTURECUBE))
		{
			if(ParamDesc.Semantic==NULL) {
				AddSampler(hParam, nTextureIndex, TEXTURE_NORMAL);
			}
			else if(_strcmpi( ParamDesc.Semantic, "Environment" ) == 0 ) {
				AddSampler(hParam, nTextureIndex, TEXTURE_CUBE);
			}
			else if(_strcmpi( ParamDesc.Semantic, "Volume" ) == 0 ) {
				AddSampler(hParam, nTextureIndex, TEXTURE_VOLUME);
			}
			else if(_strcmpi( ParamDesc.Semantic, SEM_SHADOWMAP ) == 0 ) {
				AddParam(hParam, PARAM_SHADOWMAP);
			}
			else if(_strcmpi( ParamDesc.Semantic, SEM_WORLDLIGHTMAP ) == 0 ) {
				AddParam(hParam, PARAM_WORLDLIGHTMAP);
			}
			else if(_strcmpi( ParamDesc.Semantic, SEM_SCREENBUFFER ) == 0 ) {
				AddParam(hParam, PARAM_SCREENBUFFER);
			}
			else if(_strcmpi( ParamDesc.Semantic, SEM_REFLECTMAP ) == 0 ) {
				AddParam(hParam, PARAM_REFLECTMAP);
			}	
			else {
				AddSampler(hParam, nTextureIndex, TEXTURE_NORMAL);
			}
			nTextureIndex++;
		}
	}	// end for()
}
#endif // _WIN32

HRESULT CBsMaterial::Reload()
{
	Clear();
	return LoadMaterial(GetMaterialFullName(), m_bUseMaterialSystem);
}

int CBsMaterial::QueryTechnique(int nBoneLinkCount)
{
	if(nBoneLinkCount<(int)m_EffectDesc.Techniques) {
		return nBoneLinkCount;
	}
	else {
		return 0;
	}
}


#ifdef _PACKED_RESOURCES
HRESULT CBsMaterial::LoadFXFromCache(const char* pszFileName)
{
	LPD3DXBUFFER pErrorList = NULL;
	VOID * pData = NULL;

	for(DWORD i = 0; i < s_dwFXCount; i++)
	{
		if(stricmp(pszFileName, s_pHeader[i].szFileName)==0)
		{
			pData = (VOID*)(s_pFXData + s_pHeader[i].dwFXOffset);
			break;
		}
	}
	
	if(pData == NULL)
	{
		BsAssert( pData && "Couldn't find effect\n" );
		return E_FAIL;
	}

	if(FXLCreateEffect( CBsKernel::GetInstance().GetD3DDevice(), pData, s_pEffectPool, &m_pEffect) != S_OK)
	{
		BsAssert( 0 && "Couldn't compile effect\n" );
		SAFE_RELEASE( pErrorList );
		return E_FAIL;
	}
	return S_OK;
}
#endif // _PACKED_RESOURCES


HRESULT CBsMaterial::LoadMaterial(const char* pszFileName, BOOL bUseMaterialSystem/*=TRUE*/)
{
	HRESULT hr;
	m_bUseMaterialSystem = bUseMaterialSystem;
	AvailMemoryDifference temp(0, (char*)pszFileName);

#ifdef _XBOX
#ifdef _PACKED_RESOURCES
	hr = LoadFXFromCache(pszFileName);
#else
	hr = CBsKernel::GetInstance().CreateFXLEffectFromFile(pszFileName, &m_pEffect, s_pEffectPool);
#endif // _PACKED_RESOURCES

#else	// For PC
	hr = CBsKernel::GetInstance().CreateEffectFromFile(pszFileName, &m_pEffect, s_pEffectPool);
#endif

	if(hr==S_OK) {
#ifdef _XBOX
		m_pEffect->GetEffectDesc(&m_EffectDesc);
#else
		m_pEffect->GetDesc(&m_EffectDesc);
#endif
		AddRef();
	}
	else 
	{
#ifdef _USAGE_TOOL_
		DebugString( "File [%s] Failed Compile Material!!\n",pszFileName);
#else
		BsAssert( 0 && "Failed Compile Material!!" );
		DebugString( "File [%s] Failed Compile Material!!\n",pszFileName);
#endif
	}

	SetMaterialName(pszFileName);
	if(m_bUseMaterialSystem)
        ParseEffect();
	return hr;
}

const char* CBsMaterial::GetMaterialName() 
{	
	char *pFindPtr;

	pFindPtr=strrchr(m_szMaterialFileName, '\\');
	if(pFindPtr){
		return pFindPtr+1;
	}
	else{
		return m_szMaterialFileName; 
	}
}

int CBsMaterial::GetMaterialCustomSemanticsName(char *pName, const size_t pName_len)  //aleksger - safe string
{
	char *pt=pName;

	size_t nCount = m_CustomParameters.size();
	BsAssert(pName_len >= nCount*32);
	for(size_t i=0;i<nCount;++i) {
		switch (m_CustomParameters[i].m_type) {
			case PARAM_CUSTOM_DEFAULT:		strcpy_s(pt, 32, SEM_DEFAULT);			break;
			case PARAM_MATERIALAMBIENT:		strcpy_s(pt, 32, SEM_MATERIALAMBIENT);	break;
			case PARAM_MATERIALDIFFUSE:		strcpy_s(pt, 32, SEM_MATERIALDIFFUSE);	break;
			case PARAM_MATERIALSPECULAR:	strcpy_s(pt, 32, SEM_MATERIALSPECULAR);	break;
			case PARAM_SPECULARPOWER:		strcpy_s(pt, 32, SEM_SPECULARPOWER);		break;
			case PARAM_BUMPPOWER:			strcpy_s(pt, 32, SEM_BUMPPOWER);			break;
			case PARAM_REFLECTPOWER:		strcpy_s(pt, 32, SEM_REFLECTPOWER);		break;
			case PARAM_UVANIMATION_POSITION:	strcpy_s(pt, 32, SEM_UVANIMATION_POSITION);  break;
			case PARAM_UVANIMATION_ROTATION:	strcpy_s(pt, 32, SEM_UVANIMATION_ROTATION);  break;
			case PARAM_UVANIMATION_SCALE   :	strcpy_s(pt, 32, SEM_UVANIMATION_SCALE);  break;
			case PARAM_MORPH_WEIGHT:		strcpy_s(pt, 32, SEM_MORPH_WEIGHT);		break;
			case PARAM_EFFECT_CUSTOM0:		strcpy_s(pt, 32, SEM_EFFECT_CUSTOM0);		break;
			case PARAM_EFFECT_CUSTOM1:		strcpy_s(pt, 32, SEM_EFFECT_CUSTOM1);		break;
			case PARAM_EFFECT_CUSTOM2:		strcpy_s(pt, 32, SEM_EFFECT_CUSTOM2);		break;
			case PARAM_EFFECT_CUSTOM3:		strcpy_s(pt, 32, SEM_EFFECT_CUSTOM3);		break;
			case PARAM_NOISEPOWER:		strcpy_s(pt, 32, SEM_NOISEPOWER);	break;
			case PARAM_WATERSPEED:		strcpy_s(pt, 32, SEM_WATERSPEED);	break;			

			default: strcpy_s(pt, 32,"?"); break;
		}
		pt+=32;
	}
	return nCount;
}

inline void set_info(std::vector<SEM_INFO>* pInfo, char *name, PARAMETER_TYPE type, D3DXMATRIX &value) 
{
	SEM_INFO info;
	strcpy(info.m_name,name);
	info.m_type=type;
	info.m_value=value;

	pInfo->push_back(info);
}

int CBsMaterial::GetMaterialSemanticsInfo(std::vector<SEM_INFO>* pInfo)
{
	return 0;
}

// 여기에 패러미터가 지정되도록 외부에서 처리 해 줍니다
int CBsMaterial::GetMaterialCustomSemanticsInfo(std::vector<SEM_INFO>* pInfo) 
{
	pInfo->clear();	// 메모리 초기화 합니다
	int nCount = m_CustomParameters.size();
	PARAMETER_TYPE type;
	D3DXMATRIX value;
	for(int i=0;i<nCount;++i) {
		type=m_CustomParameters[i].m_type;
		GetCustomParameterValue(type,&value);

		switch (type) {
			case PARAM_CUSTOM_DEFAULT:		set_info(pInfo, SEM_DEFAULT,type,value); break;
			case PARAM_MATERIALAMBIENT:		set_info(pInfo, SEM_MATERIALAMBIENT,type,value); break;
			case PARAM_MATERIALDIFFUSE:		set_info(pInfo, SEM_MATERIALDIFFUSE,type,value); break;
			case PARAM_MATERIALSPECULAR:	set_info(pInfo, SEM_MATERIALSPECULAR,type,value); break;
			case PARAM_SPECULARPOWER:		set_info(pInfo, SEM_SPECULARPOWER,type,value); break;
			case PARAM_BUMPPOWER:			set_info(pInfo, SEM_BUMPPOWER, type,value);	break;
			case PARAM_REFLECTPOWER:		set_info(pInfo, SEM_REFLECTPOWER, type, value);	break;
			case PARAM_UVANIMATION_POSITION:	set_info(pInfo, SEM_UVANIMATION_POSITION, type, value);	break;
			case PARAM_UVANIMATION_ROTATION:	set_info(pInfo, SEM_UVANIMATION_ROTATION, type, value);	break;
			case PARAM_UVANIMATION_SCALE   :	set_info(pInfo, SEM_UVANIMATION_SCALE,    type, value);	break;
			case PARAM_MORPH_WEIGHT:			set_info(pInfo, SEM_MORPH_WEIGHT,         type, value);	break;
			case PARAM_EFFECT_CUSTOM0:			set_info(pInfo, SEM_EFFECT_CUSTOM0,       type, value);	break;
			case PARAM_EFFECT_CUSTOM1:			set_info(pInfo, SEM_EFFECT_CUSTOM1,       type, value);	break;
			case PARAM_EFFECT_CUSTOM2:			set_info(pInfo, SEM_EFFECT_CUSTOM2,       type, value);	break;
			case PARAM_EFFECT_CUSTOM3:			set_info(pInfo, SEM_EFFECT_CUSTOM3,       type, value);	break;
			case PARAM_NOISEPOWER:		set_info(pInfo, SEM_NOISEPOWER,type,value); break;
			case PARAM_WATERSPEED:		set_info(pInfo, SEM_WATERSPEED,type,value); break;

			default: set_info(pInfo,"?",type,value); break;
		}
	}

	BsAssert(pInfo->size()==nCount && "서로 크기가 다릅니다");

	return nCount;
}

void CBsMaterial::CreateEffectPool()
{
	if(!s_pEffectPool) {
#ifdef _XBOX
		FXLCreateEffectPool(&s_pEffectPool);
#else
		D3DXCreateEffectPool(&s_pEffectPool);
#endif
	}
#ifdef _PACKED_RESOURCES
	if(!s_pCompiledFX)
	{
		char szFullName[MAX_PATH];

		strcpy(szFullName, g_BsKernel.GetShaderDirectory());
		strcat(szFullName, "N3Shaders.ccz");
		if(CBsFileIO::LoadFileDecompress(szFullName, &s_pCompiledFX) == S_OK)
		{
			s_dwFXCount = *(DWORD*)s_pCompiledFX;
			s_pHeader = (COMPILEDFXHEADER*)(s_pCompiledFX + sizeof(DWORD));
			s_pFXData = (CHAR*)&s_pHeader[s_dwFXCount];
		}
		for(DWORD i = 0; i < s_dwFXCount; i++)
		{
			sprintf(szFullName, g_BsKernel.GetShaderDirectory());
			strcat(szFullName, s_pHeader[i].szFileName);
			strcpy(s_pHeader[i].szFileName, szFullName);
		}
	}
#endif
}


void CBsMaterial::DestroyEffectPool()
{
	s_SharedParameters.clear();
	SAFE_RELEASE(s_pEffectPool);
#ifdef _PACKED_RESOURCES
	if(s_pCompiledFX)
	{
		CBsFileIO::FreeBuffer(s_pCompiledFX);
		s_pCompiledFX = NULL;
		s_dwFXCount = 0;
		s_pHeader = NULL;
		s_pFXData = NULL;
	}
#endif
}

int CBsMaterial::GetTextureType(int nSamplerIndex)
{
	int nSize = m_Samplers.size();
	if(nSamplerIndex>=nSize) {
		BsAssert( 0 && "Invalid Sampler Index!!" );
		return TEXTURE_NORMAL;
	}
	return m_Samplers[nSamplerIndex].m_type;
}


void CBsMaterial::SetParameters()
{
	int nCount = m_Parameters.size();
	if( nCount == 0 ) return;

	DWORD dwCurrentTechBit = 1 << s_nHoldingTechnique;
	EffectParam_Info *pInfo = &m_Parameters.front();    
	for(int i = 0; i < nCount; ++i) {
		if( pInfo->m_dwTechUseFlag & dwCurrentTechBit ) {
			SetParam(pInfo->m_hParam, pInfo->m_type);
		}		
		pInfo++;
	}
}

void CBsMaterial::SetSharedParameter(C3DDevice* pDevice)
{
#ifdef _XBOX
	UINT uiCount = s_SharedParameters.size();
	D3DXHANDLE hHandle;
	PARAMETER_TYPE type;
	for(UINT i=0;i<uiCount;++i) {
		hHandle = s_SharedParameters[i].m_hParam;
		type = s_SharedParameters[i].m_type;
		switch(type) {
		case PARAM_VIEW:
			s_pEffectPool->SetMatrixF(hHandle, (const float*)CBsKernel::GetInstance().GetParamViewMatrix() );
			break;
		case PARAM_VIEWINVERSE:
			s_pEffectPool->SetMatrixF(hHandle, (const float*)CBsKernel::GetInstance().GetParamInvViewMatrix() );
			break;
		case PARAM_VIEWINVERSETRANSPOSE:
			s_pEffectPool->SetMatrixF(hHandle, (const float*)CBsKernel::GetInstance().GetParamViewMatrix() );
			break;
		case PARAM_PROJECTION:
			s_pEffectPool->SetMatrixF(hHandle, (const float*)CBsKernel::GetInstance().GetParamProjectionMatrix() );
			break;
		case PARAM_VIEWPROJECTION:
			{
				D3DXMATRIX matViewProj;
				s_pEffectPool->SetMatrixF(hHandle, (const float*)D3DXMatrixMultiply(&matViewProj, CBsKernel::GetInstance().GetParamViewMatrix(), CBsKernel::GetInstance().GetParamProjectionMatrix()) );
			}
			break;
		case PARAM_RCP_WORLDSIZE:
			if(g_BsKernel.GetWorldLightMap() != -1) {
				s_pEffectPool->SetVectorF(hHandle, (const float*)&D3DXVECTOR4(g_BsKernel.GetRcpWorldXSize(), g_BsKernel.GetRcpWorldZSize(), 0.f, 0.f));
			}
			break;
		case PARAM_LIGHTDIRECTION:
			if(g_BsKernel.GetDirectionLightIndex()>=0)
                s_pEffectPool->SetVectorF(hHandle, (const float*)CBsKernel::GetInstance().GetLightDirection());
			break;
		case PARAM_VIEW_LIGHTDIRECTION:
			{
				if(g_BsKernel.GetDirectionLightIndex()>=0) {
					D3DXVECTOR4 vecViewLight;
					D3DXVec4Transform(&vecViewLight, CBsKernel::GetInstance().GetLightDirection(), g_BsKernel.GetParamViewMatrix());
					s_pEffectPool->SetVectorF(hHandle, (const float*)&vecViewLight);
				}
			}
			break;
		case PARAM_LIGHTDIFFUSE:
			if(g_BsKernel.GetDirectionLightIndex()>=0)
                s_pEffectPool->SetVectorF(hHandle, (const float*)CBsKernel::GetInstance().GetLightDiffuse());
			break;
		case PARAM_LIGHTSPECULAR:
			if(g_BsKernel.GetDirectionLightIndex()>=0)
				s_pEffectPool->SetVectorF(hHandle, (const float*)CBsKernel::GetInstance().GetLightSpecular());
			break;
		case PARAM_LIGHTAMBIENT:
			if(g_BsKernel.GetDirectionLightIndex()>=0)
                s_pEffectPool->SetVectorF(hHandle, (const float*)CBsKernel::GetInstance().GetLightAmbient());
			break;
		case PARAM_CAMERAPOSITION:
			BsAssert( 0 && "Removed Parameter!!");
			break;
		case PARAM_FOGFACTOR:
			s_pEffectPool->SetVectorF(hHandle, (const float*)g_BsKernel.GetParamFogFactor());
			break;
		case PARAM_FOGCOLOR:
			s_pEffectPool->SetVectorF(hHandle, (const float*)g_BsKernel.GetParamFogColor());
			break;
/*		case PARAM_SHADOWMAP:
			{
				if(g_BsKernel.GetWorld())
					s_pEffectPool->SetSampler(hHandle, g_BsKernel.GetShadowMgr()->GetShadowMap());
				else
					s_pEffectPool->SetSampler(hHandle, (LPDIRECT3DBASETEXTURE9)g_BsKernel.GetTexturePtr(g_BsKernel.GetBlankTexture()));
			}
			break;
		case PARAM_WORLDLIGHTMAP:
			{
				int nWorldLightMap = g_BsKernel.GetWorldLightMap();
				if( nWorldLightMap == -1 ) {
					s_pEffectPool->SetSampler(hHandle, (LPDIRECT3DBASETEXTURE9)g_BsKernel.GetTexturePtr(g_BsKernel.GetBlankTexture()));
					break;
				}
				s_pEffectPool->SetSampler(hHandle, (LPDIRECT3DBASETEXTURE9)g_BsKernel.GetTexturePtr(nWorldLightMap));
			}
			break;		*/
		}
	}
#endif
}


bool CBsMaterial::SetBoneTransformMatrices(BSMATRIX* pBoneMatrices, int nCount)
{
	if(!m_hMatrixArray)
		return false;
#ifdef _XBOX
	int nVSRegIndex = m_nMatrixArrayRegIndex[s_nHoldingTechnique];
	D3DVECTOR4* pCached = NULL;
	for(int i=0;i<nCount;++i) {
        BSMATRIX matBone;
		BsMatrixTranspose(&matBone, &pBoneMatrices[i]);
		g_BsKernel.GetDevice()->GetD3DDevice()->BeginVertexShaderConstantF1(nVSRegIndex+3*i, &pCached, 3);
		XMStoreVector4A(pCached, matBone.r[0] );
		XMStoreVector4A(pCached+1, matBone.r[1] );
		XMStoreVector4A(pCached+2, matBone.r[2] );
		g_BsKernel.GetDevice()->GetD3DDevice()->EndVertexShaderConstantF1();
	}
#else
	SetMatrixArray(m_hMatrixArray, pBoneMatrices, nCount );
#endif
	return true;
}

void CBsMaterial::SetMaterialName(const char* pszMaterialName)
{
	strcpy(m_szMaterialFileName, pszMaterialName);
}

int CBsMaterial::SearchCustomParameterIndex(PARAMETER_TYPE type)
{
	UINT size = m_CustomParameters.size();
	for(UINT ui=0;ui<size;++ui) {
		if(m_CustomParameters[ui].m_type == type)
			return ui;
	}
	BsAssert( 0 && "Invalid Custom Parameter Search!!" );
	return -1;
}

void CBsMaterial::GetCustomParameterValue(PARAMETER_TYPE type, void* pBuf)
{
	int nIndex = SearchCustomParameterIndex(type);
	if(nIndex<0) {
		BsAssert( 0 && "Invalid Search Custom Parameter type!!" );
		pBuf = NULL;
		return;
	}
#ifdef _XBOX
	/*
	// Real Time Rendering시에 있어서는 안된다.!!
	FXLPARAMETER_DESC desc;
	m_pEffect->GetParameterDesc(m_CustomParameters[nIndex].m_hParam, &desc);
	switch(desc.Class) {
	case FXLDCLASS_SCALAR:
	m_pEffect->GetScalarF(m_CustomParameters[nIndex].m_hParam, (FLOAT*)pBuf);
	break;
	case FXLDCLASS_VECTOR:
	m_pEffect->GetVectorF(m_CustomParameters[nIndex].m_hParam, (FLOAT*)pBuf);
	break;
	case FXLDCLASS_RMATRIX:
	case FXLDCLASS_CMATRIX:
	m_pEffect->GetMatrixF(m_CustomParameters[nIndex].m_hParam, (FLOAT*)pBuf);
	break;
	//		case FXLDCLASS_SAMPLER = 4,
	//		case FXLDCLASS_SAMPLER1D = 5,
	//		case FXLDCLASS_SAMPLER2D = 6,
	//		case FXLDCLASS_SAMPLER3D = 7,
	//		case FXLDCLASS_SAMPLERCUBE = 8,
	//		case FXLDCLASS_CONTAINER = 9,
	}
	*/
	m_pEffect->GetParameter(m_CustomParameters[nIndex].m_hParam, pBuf);

#else
	m_pEffect->GetValue(m_CustomParameters[nIndex].m_hParam, pBuf, D3DX_DEFAULT);
#endif
}

void CBsMaterial::SetSamplers(int* pnTextureList)
{
	int nCount = m_Samplers.size();
	for(int i=0;i<nCount;++i) {
		SetTexture(m_Samplers[i].m_hParam, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(pnTextureList[i]) );
	}
}

std::vector<VERTEX_ELEMENT>& CBsMaterial::GetVertexElements(int nIndex)
{
	if(nIndex >= 0 && nIndex<(int)m_EffectDesc.Techniques) { // mruete: prefix bug 343: added lower bound to existing check
		return m_InputElements[nIndex];
	}
	return m_InputElements[0];
}

PARAMETER_TYPE CBsMaterial::ConvertSematicToParameter(const char* pSem)
{
	if( _strcmpi( pSem, SEM_MATERIALAMBIENT ) == 0 ) {
		return PARAM_MATERIALAMBIENT;
	}
	else if( _strcmpi( pSem, SEM_MATERIALDIFFUSE ) == 0 ) {
		return PARAM_MATERIALDIFFUSE;
	}
	else if( _strcmpi( pSem, SEM_MATERIALSPECULAR ) == 0 ) {
		return PARAM_MATERIALSPECULAR;
	}
	else if( _strcmpi( pSem, SEM_SPECULARPOWER ) == 0 ) {
		return PARAM_SPECULARPOWER;
	}
	else if( _strcmpi( pSem, SEM_BUMPPOWER ) == 0 ) {
		return PARAM_BUMPPOWER;
	}
	else if( _strcmpi( pSem, SEM_REFLECTPOWER ) == 0 ) {
		return PARAM_REFLECTPOWER;
	}
	else if( _strcmpi( pSem, SEM_UVANIMATION_POSITION ) == 0 ) {
		return PARAM_UVANIMATION_POSITION;
	}
	else if( _strcmpi( pSem, SEM_UVANIMATION_ROTATION ) == 0 ) {
		return PARAM_UVANIMATION_ROTATION;
	}
	else if( _strcmpi( pSem, SEM_UVANIMATION_SCALE ) == 0 ) {
		return PARAM_UVANIMATION_SCALE;
	}
	else if( _strcmpi( pSem, SEM_MORPH_WEIGHT) == 0 ) {
		return PARAM_MORPH_WEIGHT;
	}
	else if( _strcmpi( pSem, SEM_EFFECT_CUSTOM0) == 0 ) {
		return PARAM_EFFECT_CUSTOM0;
	}
	else if( _strcmpi( pSem, SEM_EFFECT_CUSTOM1) == 0 ) {
		return PARAM_EFFECT_CUSTOM1;
	}
	else if( _strcmpi( pSem, SEM_EFFECT_CUSTOM2) == 0 ) {
		return PARAM_EFFECT_CUSTOM2;
	}
	else if( _strcmpi( pSem, SEM_EFFECT_CUSTOM3) == 0 ) {
		return PARAM_EFFECT_CUSTOM3;
	}
	else if( _strcmpi( pSem, SEM_NOISEPOWER) == 0 ) {
		return PARAM_NOISEPOWER;
	}
	else if( _strcmpi( pSem, SEM_WATERSPEED) == 0 ) {
		return PARAM_WATERSPEED;
	}	
	else {
		BsAssert( 0 && "Invalid Parameter!!" );
	}

	return PARAM_CUSTOM_DEFAULT;
}

#ifdef _XBOX
char* CBsMaterial::GetSamplerSemanticName(int nSamplerIndex)
{
	BsAssert(m_pEffect);

	D3DXHANDLE			hParam;
	FXLPARAMETER_DESC	ParamDesc;
	int					nTextureIndex = 0;

	for( UINT uiParam = 0; uiParam < m_EffectDesc.Parameters; uiParam++ ) {
		hParam = m_pEffect->GetParameterHandleFromIndex( uiParam );
		m_pEffect->GetParameterDesc( hParam, &ParamDesc );

		if(ParamDesc.Class == D3DXPC_OBJECT && (ParamDesc.Type == D3DXPT_TEXTURE ||
			ParamDesc.Type == D3DXPT_TEXTURE1D || ParamDesc.Type == D3DXPT_TEXTURE2D ||
			ParamDesc.Type == D3DXPT_TEXTURE3D || ParamDesc.Type == D3DXPT_TEXTURECUBE))
		{
			if (nTextureIndex==nSamplerIndex) {
				return (char *)ParamDesc.pName;
			}
			nTextureIndex++;
		}
	}
	BsAssert(0 && "이름 얻어오기 실패");
	return NULL;
}
#else
char* CBsMaterial::GetSamplerSemanticName(int nSamplerIndex)
{
	BsAssert(m_pEffect);

	D3DXHANDLE			hParam;
	D3DXPARAMETER_DESC	ParamDesc;
	int					nTextureIndex = 0;

	for( UINT uiParam = 0; uiParam < m_EffectDesc.Parameters; uiParam++ ) {
		hParam = m_pEffect->GetParameter ( NULL, uiParam );
		m_pEffect->GetParameterDesc( hParam, &ParamDesc );

		if(ParamDesc.Class == D3DXPC_OBJECT && (ParamDesc.Type == D3DXPT_TEXTURE ||
			ParamDesc.Type == D3DXPT_TEXTURE1D || ParamDesc.Type == D3DXPT_TEXTURE2D ||
			ParamDesc.Type == D3DXPT_TEXTURE3D || ParamDesc.Type == D3DXPT_TEXTURECUBE))
		{
			if (nTextureIndex==nSamplerIndex) {
				return (char *)ParamDesc.Name;
			}
			nTextureIndex++;
		}
	}
	BsAssert(0 && "이름 얻어오기 실패");
	return NULL;
}
#endif


/*
*	Class : CBsVertexDeclaration
*/
CBsVertexDeclaration::CBsVertexDeclaration()
{
	m_pVertexDecl = NULL;
	m_pVertexElement = NULL;
	m_nRefCount	= 0;
}

CBsVertexDeclaration::~CBsVertexDeclaration()
{
	if(m_pVertexElement) {
		delete[] m_pVertexElement;
		m_pVertexElement = NULL;
	}
	Clear();
}

void CBsVertexDeclaration::AddRef()
{
#if defined(_XBOX)
	InterlockedIncrement( reinterpret_cast<volatile long*>(&m_nRefCount) );
#else
	++m_nRefCount;
#endif
}

int CBsVertexDeclaration::Release()
{
	int nNewRefCount = InterlockedDecrement( reinterpret_cast<volatile long*>(&m_nRefCount) );

	BsAssert( nNewRefCount >= 0 );

	return nNewRefCount;
}

void CBsVertexDeclaration::Clear()
{
	SAFE_RELEASE(m_pVertexDecl);
}
HRESULT CBsVertexDeclaration::Reload()
{
	Clear();
	return AllocDeviceResource(m_pVertexElement);
}

HRESULT CBsVertexDeclaration::LoadVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements)
{
	HRESULT hr;
	hr = AllocDeviceResource(pVertexElements);
	D3DVERTEXELEMENT9 decl[256];
	UINT numElements = 256;
	m_pVertexDecl->GetDeclaration(decl, &numElements);
	m_pVertexElement = new D3DVERTEXELEMENT9[numElements];
	memcpy(m_pVertexElement, decl, sizeof(D3DVERTEXELEMENT9)*numElements);
	AddRef();
	return hr;
}

HRESULT CBsVertexDeclaration::AllocDeviceResource(CONST D3DVERTEXELEMENT9* pVertexElements)
{
	return g_BsKernel.GetDevice()->CreateVertexDeclaration(pVertexElements, &m_pVertexDecl);
}

bool CBsVertexDeclaration::IsEqualVertexElements(D3DVERTEXELEMENT9* pVertexElements)
{
	bool bIsEqual = true;
	D3DVERTEXELEMENT9 decl[32];
	UINT numElements = 32;

	m_pVertexDecl->GetDeclaration(decl, &numElements);

	for(UINT j = 0 ; j<numElements ; ++j) {
		if( (decl[j].Method != pVertexElements[j].Method ) ||
			(decl[j].Offset != pVertexElements[j].Offset ) ||
			(decl[j].Stream != pVertexElements[j].Stream ) ||
			(decl[j].Type != pVertexElements[j].Type ) ||
			(decl[j].UsageIndex != pVertexElements[j].UsageIndex ) )			
		{
			return false;
		}
	}
	return true;
}



