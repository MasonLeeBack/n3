#pragma once

#include "BsFXElement.h"

class BStream;
class CCrossVector;

#define FX_MAX_CUSTOM_PARAMETER_COUNT 4
class CBsFXMeshElement : public CBsFXElement
{
public:
	CBsFXMeshElement( CBsFXElement *pParent = NULL );
	virtual ~CBsFXMeshElement();

protected:
	int m_nSubMeshCount;
	int m_nAmbientCustomParamIndex;
	int m_nDiffuseCustomParamIndex;

	float m_fUVAniLoopRatio;

	CBsFXTable *m_pTableScaleAxis[3];	// 축별 스케일 또 하나 있다~
	int m_nViewAxis;
	int m_nFixedAxis;
	int m_nBlendOP;
	int m_nSourceBlend;
	int m_nDestBlend;
	BOOL m_bCheckZBuffer;

	struct CustomParamInfo {
		int nParamIndex;
		int nTableType;
		CBsFXTable *pTable;
	};
	CustomParamInfo m_CustomParam[FX_MAX_CUSTOM_PARAMETER_COUNT];

	void CalcBillboardMatrix( CCrossVector *pOutput, D3DXVECTOR3 *pUpVec, D3DXVECTOR3 *pPosition, D3DXVECTOR3 *pCameraPos );
public:
	virtual BOOL Activate();

	virtual BOOL Load( BStream *pStream );
	virtual void Process( DWORD dwTime, float fDistanceFromCamera, int count = 0 );

	virtual void DeleteEngineObject();

	static int m_nCount;
};