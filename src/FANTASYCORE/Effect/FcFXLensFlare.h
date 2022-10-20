#pragma	   once

#include "FcFXBase.h"

//-----------------------------------------------------------------------------
//	Data :	2003. 1. 26
//	Desc :	LensFlare Effect
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// Name: SVert
// Desc: Vertices used to render light and flare elements
//-----------------------------------------------------------------------------
struct SVert
{
	D3DXVECTOR3 Pos;
	D3DXVECTOR2 UV;
};

//-----------------------------------------------------------------------------
// Name: Struct SFlareElement
// Desc: represents a flare element
//-----------------------------------------------------------------------------
struct SFlareElement
{
	int						nTexture;	// alpha Texture Index in Kernel
	D3DXVECTOR2             Size;       // flare size in projection space
	D3DXVECTOR4             Color;
	FLOAT                   fDistance;  // distance along element axis
};

//-----------------------------------------------------------------------------
// Name: Struct SLightElement
// Desc: represents the flare light
//-----------------------------------------------------------------------------
struct SLightElement
{
	int					nTexture;		// alpha Texture Index in Kernel
	D3DXVECTOR2         BaseSize;       // size of light center
	D3DXVECTOR2         FlareOutSize;   // size of "flare out"
	UINT                uiNumFlareOuts; // number of flare outs to draw between base and flareout size

	D3DXVECTOR4         Color;          // light color and alpha
	D3DXVECTOR2         ViewCenter;     // center point for element axis in projection space
};


//-----------------------------------------------------------------------------
// Name: class CFcFXLensFlare
// Desc: encapsulates the lens flare
//-----------------------------------------------------------------------------
class CFcFXLensFlare : public CFcFXBase
{
public:
	enum FLARE_MODE { FLARE_BEAM=0, FLARE_CIRCLE };
	CFcFXLensFlare();    
	~CFcFXLensFlare();

	HRESULT Create(SLightElement* pLightElement,                        // Initialized the lens flare
		SFlareElement* pFlareElements, UINT uiNumElements,
		const D3DXVECTOR2& MaxFlareSize );

	void	Process() {}
	void	Render(C3DDevice* pDevice);
	int		ProcessMessage(int nCode,DWORD dwParam1=0, DWORD dwParam2=0,DWORD dwParam3=0);
private:
	void					SetLightIntensity(float fPercent);		// Light 강도 조절...

	HRESULT                 SetNumFullLightPixels();    // Sets the number of full light pixels
	int                     RasterPos( FLOAT fNum );    // Determines where a vertex will be rendered

	// 외부에서  Setting!!
	FLARE_MODE					m_FlareMode;	
	float						m_fLensFlareAlphaScale;	// Light Flare Alpha Percent!!
	int							m_nPow;					// camera방향과 빛방향의 내적에 의한 빛 산란 정도 조절

	D3DXVECTOR3					m_vecLightDir;

	float						m_fVisibilityRatio;

	SFlareElement*				m_pFlareElements;           // Array of flare elements
	UINT						m_uiNumFlareElements;       // Number of flare elements
	D3DXVECTOR2					m_MaxElementSize;           // Maximum size for an element (clamp)
	SLightElement*				m_pLightElement;            // Light element
	int							m_nBackBufferCopy;          // The copy of the back buffer used for flare out
	UINT						m_uiNumFullLightPixels;     // Number of visible pixels for the non-occluded light
	FLOAT						m_bPreviouslyFullyVisible;  // Hint not to copy back buffer
	IDirect3DVertexBuffer9*		m_pQuadVB;                  // VB for light and flare elements

	int									m_nVertexDeclIdx;
	int									m_nMaterialIndex;
	D3DXHANDLE							m_hMatWVP;
	D3DXHANDLE							m_hTexture;
	D3DXHANDLE							m_hLightColor;
	D3DXHANDLE							m_hTexCoordOffset;


};