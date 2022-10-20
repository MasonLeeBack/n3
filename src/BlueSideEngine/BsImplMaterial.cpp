#include "stdafx.h"
#include "BsImplMaterial.h"
#include "BsKernel.h"
#include "BsObject.h"
#include "BsMaterial.h"

CBsImplMaterial::CBsImplMaterial()
{
	m_nStreamMapperCount = 0;
	m_pStreamMappers = NULL;
	m_pMaterial = NULL;
	m_nMaterialIndex = -1;
	m_nSamplerCount  = -1;
	m_pnTextureIndex = NULL;

	m_bAlphaAlignEnable = false;
	m_bTwoSideEnable = false;
	m_bAlphaBlendEnable = false;
	m_bDepthCehckEnable = true;
	m_fAlpha = 1.f;
    
	m_BlendOp  = D3DBLENDOP_ADD; 
	m_SrcBlend = D3DBLEND_SRCALPHA;
	m_DestBlend= D3DBLEND_INVSRCALPHA;

	m_fCurrentFrameUVAnimation=0.0f;	

	m_bPointSprite = false;
	m_bOnlyDepthWrite = false;

	m_nUVAnimationPos = -1;
	m_nUVAnimationRot = -1;
	m_nUVAnimationSca = -1;
}

CBsImplMaterial::~CBsImplMaterial()
{
	Clear();
}

void CBsImplMaterial::Clear()
{
	SAFE_DELETEA(m_pStreamMappers);

	if(m_pnTextureIndex) {
		delete [] m_pnTextureIndex;
		m_pnTextureIndex = NULL;
	}
	UINT uiSize = m_EditableParameters.size();
	for(UINT i=0;i<uiSize;++i) {
		if( m_EditableParameters[i] ) {
			delete m_EditableParameters[i];
			m_EditableParameters[i] = NULL;
		}
	}
	m_EditableParameters.clear();
}

void CBsImplMaterial::SetAlphaState(C3DDevice* pDevice)
{
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

	pDevice->SetRenderState( D3DRS_BLENDOP, m_BlendOp );
	pDevice->SetRenderState( D3DRS_SRCBLEND, m_SrcBlend );
	pDevice->SetRenderState( D3DRS_DESTBLEND, m_DestBlend );

	pDevice->SetRenderState(D3DRS_ZWRITEENABLE , false);

	m_pMaterial->SetAlpha(m_fAlpha);
}

void CBsImplMaterial::RestoreAlphaState(C3DDevice* pDevice)
{
	m_pMaterial->SetAlpha(1.f);
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

	pDevice->SetRenderState(D3DRS_ZWRITEENABLE , true);
}

int CBsImplMaterial::AddEditableParameter(PARAMETER_TYPE type)
{
	std::vector<EffectParam_Info>&	customs = m_pMaterial->GetCustomParameters();
	
	// 등록된 것중 찾아보고, 없으면 새로 추가하여 Return!!
	int nEditableIndex = SearchEditableParameter(type);

	if(nEditableIndex == -1) {
		// 없으므로 Editable Container에 추가!!
		int nCustomIndex = SearchCustomParameterIndex(type);

		if(nCustomIndex == -1) {
			// 없는 CustomParameter를 요청했으므로 Error처리!!
//			assert( 0 && "존재하지 않은 Custom Parameter요청!!!" );
			return -1;
		}

		VariableType datatype = GetVariableTypeByParam(type);
		CustomParam_Info* pInfo = new CustomParam_Info(datatype);
		pInfo->m_nCustomParamIndex = nCustomIndex;
		pInfo->m_type = type;
		nEditableIndex = m_EditableParameters.size();
        m_EditableParameters.push_back(pInfo);

		return nEditableIndex;
	}
	else {
		return nEditableIndex;
	}
}

void CBsImplMaterial::SetEditableParameter(int nIndex, void* pBuf)
{
	if(nIndex<0)
		return;
	int nSize = m_EditableParameters.size();
	if(nIndex>=nSize) {
		BsAssert( 0 && "Invalid Editable Parameter Index!!" );
		return;
	}
	switch(m_EditableParameters[nIndex]->m_DataType) {
			case varVector:
				memcpy(m_EditableParameters[nIndex]->m_pValue, pBuf, sizeof(D3DXVECTOR4));
				break;
			case varColor:
				memcpy(m_EditableParameters[nIndex]->m_pValue, pBuf, sizeof(D3DXVECTOR4));
				break;
			case varFloat:
				memcpy(m_EditableParameters[nIndex]->m_pValue, pBuf, sizeof(float));
				break;
			case varMatrix4x4:
				memcpy(m_EditableParameters[nIndex]->m_pValue, pBuf, sizeof(D3DXMATRIX));
				break;
			case varBoolean:
				memcpy(m_EditableParameters[nIndex]->m_pValue, pBuf, sizeof(BOOL));
				break;
			default:
				m_EditableParameters[nIndex]->m_pValue=NULL;
				break;
	}
}


void CBsImplMaterial::GetEditableParameter(int nIndex, void* pBuf)
{
	if(nIndex<0)
		return;
	int nSize = m_EditableParameters.size();
	if(nIndex>=nSize) {
		BsAssert( 0 && "Invalid Editable Parameter Index!!" );
		return;
	}
	switch(m_EditableParameters[nIndex]->m_DataType) {
			case varVector:
				memcpy(pBuf, m_EditableParameters[nIndex]->m_pValue, sizeof(D3DXVECTOR4));
				break;
			case varColor:
				memcpy(pBuf, m_EditableParameters[nIndex]->m_pValue, sizeof(D3DXVECTOR4));
				break;
			case varFloat:
				memcpy(pBuf, m_EditableParameters[nIndex]->m_pValue, sizeof(float));
				break;
			case varMatrix4x4:
				memcpy(pBuf, m_EditableParameters[nIndex]->m_pValue, sizeof(D3DXMATRIX));
				break;
			case varBoolean:
				memcpy(pBuf, m_EditableParameters[nIndex]->m_pValue, sizeof(BOOL));
				break;
			default:
				pBuf = NULL;
				break;
	}
}

int CBsImplMaterial::SearchEditableParameter(PARAMETER_TYPE type)
{
	UINT size = m_EditableParameters.size();
	for(UINT ui=0;ui<size;++ui) {
		if(m_EditableParameters[ui]->m_type == type)
			return ui;
	}
	return -1;
}

void CBsImplMaterial::AttachSubMesh(CBsSubMesh* pSubMesh)
{
}


int CBsImplMaterial::LinkMaterial(int nMaterialIndex,  CBsSubMesh* pSubMesh)
{
	BsAssert( nMaterialIndex>=0 && "Invalid Material Index!!" );

	m_pMaterial = g_BsKernel.GetMaterialPtr(nMaterialIndex);
	m_nMaterialIndex = nMaterialIndex;

	Clear();

	// Sampler
	m_nSamplerCount = m_pMaterial->GetSamplerCount();
	m_pnTextureIndex = new int[m_nSamplerCount];
	for(int i=0;i<m_nSamplerCount;++i)
		m_pnTextureIndex[i] = -1;

	// Customized(User Defined) Parameter Value는 이후에 작업 by jeremy!!
	std::vector<EffectParam_Info>&	customs = m_pMaterial->GetCustomParameters();
	UINT uiSize = customs.size();
	for(UINT i=0;i<uiSize;++i) {
		VariableType datatype = GetVariableTypeByParam(customs[i].m_type);
		CustomParam_Info* pInfo = new CustomParam_Info(datatype);
		pInfo->m_type = customs[i].m_type;
		pInfo->m_nCustomParamIndex = i;
		m_EditableParameters.push_back(pInfo);
		m_pMaterial->GetCustomParameterValue(pInfo->m_type, pInfo->m_pValue);
	}

	SetupLOD(pSubMesh->GetLODCount());

	for(int i=0; i<m_nStreamMapperCount; ++i) {
		int nTechniqueIndex = m_pMaterial->QueryTechnique(pSubMesh->GetBoneLinkCount(i));
		int nStreamIndexCount = m_pMaterial->GetVertexElements(nTechniqueIndex).size();
        m_pStreamMappers[i].m_nStreamIndexCount = nStreamIndexCount;
		m_pStreamMappers[i].m_pnStreamIndex = new int[nStreamIndexCount];
	}

	return m_nSamplerCount;
}

void CBsImplMaterial::SetUVAnimation(int v) 
{
	if (v) {
		m_nUVAnimationPos=-2; // AddEditableParameter(PARAM_UVANIMATION_POSITION); // 여기에서 호출하면 에러 납니다
	}
	else {
		m_nUVAnimationPos=-1; // 애니메이션이 없는 경우 입니다
	}
}

void CBsImplMaterial::SetUVAnimationRender(CBsSubMesh *pSubMesh)
{
	// mruete: prefix bug 331: added initializers
	D3DXVECTOR4 pos(0,0,0,1);
	D3DXVECTOR4 rot(0,0,0,1);
	D3DXVECTOR4 sca(1,1,1,1);

	float frame = pSubMesh->GetUVAnimationFrame(m_fCurrentFrameUVAnimation);
		
	int bPos, bRot, bSca;
	bPos = pSubMesh->GetPositionVector(*(D3DXVECTOR3    *)&pos, frame);
	bRot = pSubMesh->GetRotationVector(*(D3DXQUATERNION *)&rot, frame);
	bSca = pSubMesh->GetScaleVector   (*(D3DXVECTOR3    *)&sca, frame);
	BsAssert( bPos && bRot && bSca );	// mruete: prefix bug 331: added assert on return values

	if (m_nUVAnimationPos==-2) {
		m_nUVAnimationPos=AddEditableParameter(PARAM_UVANIMATION_POSITION);
		m_nUVAnimationRot=AddEditableParameter(PARAM_UVANIMATION_ROTATION);
		m_nUVAnimationSca=AddEditableParameter(PARAM_UVANIMATION_SCALE);
	}

	SetEditableParameter(m_nUVAnimationPos, &pos);
	SetEditableParameter(m_nUVAnimationRot, &rot);
	SetEditableParameter(m_nUVAnimationSca, &sca);
}

bool CBsImplMaterial::UnlinkMaterial()
{
	return true;
}

void CBsImplMaterial::SetCustomParameter()
{
	UINT size = m_EditableParameters.size();
	for(UINT ui=0;ui<size;++ui) {
		m_pMaterial->SetCustomParam(m_EditableParameters[ui]->m_nCustomParamIndex, m_EditableParameters[ui]->m_pValue );
	}
}

void CBsImplMaterial::GetCustomParameterValue(PARAMETER_TYPE type, void* pBuf)
{
	m_pMaterial->GetCustomParameterValue(type, pBuf);
}

void CBsImplMaterial::HoldMaterial(C3DDevice* pDevice, CBsObject* pHoldingObject, int nTechIndex, int nLODLevel)
{
	m_pMaterial->Hold(pHoldingObject, nTechIndex);

#ifdef _XBOX
	m_pMaterial->BeginMaterial(nTechIndex, 0);
#else
	m_pMaterial->BeginMaterial(nTechIndex, D3DXFX_DONOTSAVESTATE);
#endif

	g_BsKernel.SetVertexDeclaration(GetVertexDeclIndex(nLODLevel));
	m_pMaterial->SetSamplers(m_pnTextureIndex);
}

void CBsImplMaterial::FreeMaterial()
{
	m_pMaterial->Free();
}

int CBsImplMaterial::SearchCustomParameterIndex(PARAMETER_TYPE type)
{
	std::vector<EffectParam_Info>&	customs = m_pMaterial->GetCustomParameters();
	UINT size = customs.size();
	for(UINT ui=0;ui<size;++ui) {
		if(customs[ui].m_type == type)
			return ui;
	}
//	assert( 0 && "Invalid Custom Parameter Search!!" );
	return -1;
}

VariableType CBsImplMaterial::GetVariableTypeByParam(PARAMETER_TYPE type)
{
	switch(type) {
		case PARAM_MATERIALAMBIENT:
		case PARAM_MATERIALDIFFUSE:
		case PARAM_MATERIALSPECULAR:
		case PARAM_UVANIMATION_POSITION:			
		case PARAM_UVANIMATION_ROTATION:			
		case PARAM_UVANIMATION_SCALE:			
			return varVector; // VariableType

		case PARAM_SPECULARPOWER:
		case PARAM_BUMPPOWER:
		case PARAM_REFLECTPOWER:
		case PARAM_MORPH_WEIGHT:
		case PARAM_EFFECT_CUSTOM0:
		case PARAM_EFFECT_CUSTOM1:
		case PARAM_EFFECT_CUSTOM2:
		case PARAM_EFFECT_CUSTOM3:
		case PARAM_NOISEPOWER:
		case PARAM_WATERSPEED:
			return varFloat; // VariableType
	}

	return varInvalid; // VariableType
}
