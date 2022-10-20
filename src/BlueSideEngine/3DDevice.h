#pragma once

#include "BsCommon.h"

#define MAX_STATE_SAVE 10

#ifndef _XBOX
struct D3DRESOLVE_PARAMETERS;
struct D3DVECTOR4;
#endif

class C3DDevice
{
public:
	C3DDevice();
	virtual ~C3DDevice();
#ifdef _XBOX
	HRESULT Initialize();
	HRESULT Reset(D3DPRESENT_PARAMETERS* pPresentationParameters);
#else
	HRESULT Initialize(HWND hWnd, int nWidth, int nHeight, bool bIsWindowed, bool bMultiThreaded = false);
#endif
	void InitDefaultState();

#ifdef INV_Z_TRANSFORM
	void ClearBuffer(D3DCOLOR COLOR, float fZ = 0.f, DWORD dwStencil = 0);
#else
	void ClearBuffer(D3DCOLOR COLOR, float fZ = 1.f, DWORD dwStencil = 0);
#endif
	void ClearTarget(D3DCOLOR COLOR);
	void ClearZStencilBuffer(float fZ, DWORD dwStencil);

	DWORD GetAlphaRefValue(){ return m_dwAlphaRefValue;	}

	int GetBackBufferWidth() { return m_nBackBufferWidth; }
	int GetBackBufferHeight() { return m_nBackBufferHeight; }

	LPDIRECT3DSURFACE9	GetBackBuffer()	{	return m_pBackBuffer;	}
	LPDIRECT3DSURFACE9	GetDepthStencilSurface()	{	return m_pDepthBuffer;	}
	LPDIRECT3DSURFACE9	GetDepthStencilRTTSurface()	{	return m_pDepthBufferRTT;	}

	void SaveFrontBuffer(char* szFileName);
	void SaveBackBuffer(char* szFileName);

	void SetViewport(const D3DVIEWPORT9* pViewport);
	void GetViewport(D3DVIEWPORT9* pViewport);

	HRESULT BeginScene()	{	return m_pD3DDevice->BeginScene();	}
	HRESULT EndScene()		{	return m_pD3DDevice->EndScene();	}

	void SaveState();
	void RestoreState();

	void ShowFrame();

	void SetRenderTarget(DWORD dwRenderTargetIndex, IDirect3DSurface9* pRenderTarget) { m_pD3DDevice->SetRenderTarget(dwRenderTargetIndex, pRenderTarget); }
	void GetRenderTarget(DWORD dwRenderTargetIndex, IDirect3DSurface9** ppRenderTarget) { m_pD3DDevice->GetRenderTarget(dwRenderTargetIndex, ppRenderTarget); }

	void SetDepthStencilSurface(IDirect3DSurface9 *pZStencilSurface)	{	m_pD3DDevice->SetDepthStencilSurface(pZStencilSurface);	}
	void GetDepthStencilSurface(IDirect3DSurface9 **ppZStencilSurface)	{	m_pD3DDevice->GetDepthStencilSurface(ppZStencilSurface);	}

	void SetVertexShader(IDirect3DVertexShader9* pShader)	{	m_pD3DDevice->SetVertexShader(pShader);	}
	void SetVertexShaderConstantF(UINT startReg, const float* pConstantData, UINT count)	{	m_pD3DDevice->SetVertexShaderConstantF(startReg, pConstantData, count);	}
	void SetPixelShader(IDirect3DPixelShader9* pShader)		{	m_pD3DDevice->SetPixelShader(pShader);	}
	void SetPixelShaderConstantF(UINT startReg, const float* pConstantData, UINT count)	{	m_pD3DDevice->SetPixelShaderConstantF(startReg, pConstantData, count);	}

#ifndef _XBOX
	void SetFVF(DWORD dwFVF)	{	m_pD3DDevice->SetFVF(dwFVF);	}
#endif

	void SetVertexDeclaration(IDirect3DVertexDeclaration9 *pDecl)	{ m_pD3DDevice->SetVertexDeclaration(pDecl);	}
	void SetStreamSource(int nStream, LPDIRECT3DVERTEXBUFFER9 pVB, int nStride)	{	m_pD3DDevice->SetStreamSource(nStream, pVB, 0, nStride );	}
	void SetStreamSource(int nStream, LPDIRECT3DVERTEXBUFFER9 pVB, int nOffsetInBytes, int nStride)	{	m_pD3DDevice->SetStreamSource(nStream, pVB, nOffsetInBytes, nStride);	}
	void SetIndices(LPDIRECT3DINDEXBUFFER9 pIB)									{	m_pD3DDevice->SetIndices(pIB);	}

	void SetTexture(DWORD dwSampler, IDirect3DBaseTexture9 *pTexture)			{	m_pD3DDevice->SetTexture(dwSampler, pTexture);	}

	void AcquireD3DThreadOwnership()
	{
#ifdef _XBOX
		m_pD3DDevice->AcquireThreadOwnership();
#endif
	}
	void ReleaseD3DThreadOwnership()
	{
#ifdef _XBOX
		m_pD3DDevice->ReleaseThreadOwnership();
#endif
	}

#ifdef _XBOX
	//-------------------------------------------------------------------------------------------------------------------------------------------
	void	Resolve( DWORD Flags, CONST D3DRECT *pSourceRect, IDirect3DBaseTexture9 *pDestTexture, CONST D3DPOINT *pDestPoint, UINT DestLevel,
        UINT DestSliceOrFace, CONST D3DVECTOR4 *pClearColor, float ClearZ, DWORD ClearStencil, CONST D3DRESOLVE_PARAMETERS *pParameters )
	{	m_pD3DDevice->Resolve(Flags, pSourceRect, pDestTexture, pDestPoint, DestLevel, DestSliceOrFace, pClearColor, ClearZ, ClearStencil, pParameters );	}
	//-------------------------------------------------------------------------------------------------------------------------------------------

	HRESULT BeginVertices(D3DPRIMITIVETYPE type, UINT uiVertexCount, UINT uiVertexStreamZeroStride, void** ppVertexData)
	{
		return m_pD3DDevice->BeginVertices(type, uiVertexCount, uiVertexStreamZeroStride, ppVertexData);
	}
	void EndVertices() {	m_pD3DDevice->EndVertices();	}

	HRESULT BeginIndexedVertices(D3DPRIMITIVETYPE PrimitiveType, int BaseVertexIndex, int NumVertices, int IndexCount, D3DFORMAT IndexDataFormat,
        UINT VertexStreamZeroStride, void **ppIndexData, void **ppVertexData)
	{
		return m_pD3DDevice->BeginIndexedVertices(
			PrimitiveType,
			BaseVertexIndex,
			NumVertices,
			IndexCount,
			IndexDataFormat,
			VertexStreamZeroStride,
			ppIndexData,
			ppVertexData );
	}
	void EndIndexedVertices() {	m_pD3DDevice->EndIndexedVertices();	}
#else
	void SetTransform(int nTransform, const D3DXMATRIX *pMatrix)	{ m_pD3DDevice->SetTransform((D3DTRANSFORMSTATETYPE)nTransform, pMatrix); }
	void GetTransform(int nTransform, D3DXMATRIX *pMatrix)	{ m_pD3DDevice->GetTransform((D3DTRANSFORMSTATETYPE)nTransform, pMatrix ); }
	void SetTextureStageState(int nStage, int nState, DWORD dwValue) {	m_pD3DDevice->SetTextureStageState(nStage, (D3DTEXTURESTAGESTATETYPE)nState, dwValue);	}
	void GetTextureStageState(int nStage, int nState, DWORD *pdwValue);

	HRESULT StretchRect(IDirect3DSurface9 *pSourceSurface, CONST RECT *pSourceRect, IDirect3DSurface9 *pDestSurface, CONST RECT *pDestRect, D3DTEXTUREFILTERTYPE Filter)
	{
		return m_pD3DDevice->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
	}
#endif
	void SetRenderState(int nState, DWORD dwValue)					{	m_pD3DDevice->SetRenderState((D3DRENDERSTATETYPE)nState, dwValue);	}
	void GetRenderState(int nState, DWORD *pdwValue)				{	m_pD3DDevice->GetRenderState((D3DRENDERSTATETYPE)nState, pdwValue); }

	void SetSamplerState(int nStage, D3DSAMPLERSTATETYPE Type, DWORD dwValue)		{	m_pD3DDevice->SetSamplerState(nStage, Type, dwValue);		}
	void GetSamplerState(int nStage, D3DSAMPLERSTATETYPE Type, DWORD* pdwValue)	{	m_pD3DDevice->GetSamplerState(nStage, Type, pdwValue);	}

	void SetGammaRamp(float fGamma);
	float GetGammaRamp()		{	return m_fGamma;	}

	void BeginTiling(DWORD dwFlags, DWORD dwCount, const D3DRECT* pTileRects, const D3DVECTOR4* pClearColor, float fClearZ, DWORD dwClearStencil)
	{
#ifdef _XBOX
		m_pD3DDevice->BeginTiling(dwFlags, dwCount, pTileRects, pClearColor, fClearZ, dwClearStencil);
#endif
	}
	HRESULT EndTiling(DWORD dwResolveFlags, const D3DRECT *pResolveRects, IDirect3DBaseTexture9 *pDestTexture, const D3DVECTOR4 *pClearColor,
		float fClearZ, DWORD dwClearStencil, const D3DRESOLVE_PARAMETERS *pParameters)
	{
#ifdef _XBOX
		return m_pD3DDevice->EndTiling(dwResolveFlags, pResolveRects, pDestTexture, pClearColor, fClearZ, dwClearStencil, pParameters);
#endif
	}

	// Device가 관리하는 리소스 관련 함수
	HRESULT CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl)
		{
            return m_pD3DDevice->CreateVertexDeclaration( pVertexElements, ppDecl);
		}
	HRESULT CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer)
		{
            return m_pD3DDevice->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, NULL);
		}
	HRESULT CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer)
		{
			return m_pD3DDevice->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, NULL);
		}
	HRESULT CreateEffectFromFile(const char* pszFileName, LPD3DXEFFECT* ppEffect, LPD3DXEFFECTPOOL pEffectPool = NULL);

#ifdef _XBOX
	HRESULT CreateFXLEffectFromFile(const char* pszFileName, FXLEffect** ppEffect, FXLEffectPool* pEffectPool = NULL);
	HRESULT CompileFXLEffectFromFile(const char* pszFileName, LPD3DXBUFFER *pEffectData );
	HRESULT CreateFXLEffectFromMemory(LPVOID pMemoryBuffer, FXLEffect** ppEffect, FXLEffectPool* pEffectPool = NULL);
	LPDIRECT3DBASETEXTURE9 LoadPackedTexture(const char* pFileName, DWORD *pdwSize, CHAR ** ppTexData);
	LPDIRECT3DBASETEXTURE9 RegisterPreLoadedTexture( BYTE * pTextureResource );
#endif

	void* CreateTextureFromFile(const char* pFileName, DWORD dwUsage = 0, CHAR ** ppTexData = NULL);
	void* CreateCubeTextureFromFile(const char* pFileName);
	void* CreateVolumeTextureFromFile(const char* pFileName);

	void* CreateTexture(DWORD *pdwSize, CHAR ** ppTexData, int nWidth, int nHeight, DWORD dwUsage, D3DFORMAT format, D3DPOOL pool=D3DPOOL_DEFAULT);
	void* CreateCubeTexture(int nEdgeLength, DWORD dwUsage, D3DFORMAT format, D3DPOOL pool=D3DPOOL_DEFAULT);
	void* CreateVolumeTexture(int nWidth, int nHeight, int nDepth, DWORD dwUsage, D3DFORMAT format, D3DPOOL pool=D3DPOOL_DEFAULT);

	void DrawIndexedMeshVB(int nPrimitiveType, int nVertexCount, int nPrimitiveCount, 
		int nStartIndex, int nStartVertex);
	void DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT uiMinIndex, UINT uiVertexCount, UINT uiPrimitiveCount, const void* pIndexData, D3DFORMAT IndexDataFormat,
		const void* pVertexStreamZeroData, UINT uiVertexStride);
	void DrawPrimitiveUP(D3DPRIMITIVETYPE primitiveType, UINT uiPrimitiveCount, const void* pVertexStreamZeroData, UINT uiStride) { m_pD3DDevice->DrawPrimitiveUP(	primitiveType, uiPrimitiveCount	,pVertexStreamZeroData, uiStride);}
	void DrawMeshVB(int nPrimitiveType, int nPrimitiveCount, const void *VertexData, int nStride);
	void DrawPrimitiveVB(int nPrimitiveType, int nStartVertex, int nPrimitiveCount);

	DWORD				m_dwDrawCallCount;
	DWORD				m_dwDrawPrimitiveCount;

protected:
	LPDIRECT3D9			m_pD3D;
	LPDIRECT3DDEVICE9	m_pD3DDevice;
	D3DPRESENT_PARAMETERS	m_Param;
	D3DVIEWPORT9		m_ViewPort;

	float				m_fGamma;				// 0.f ~ 1.f 범위

	int					m_nBackBufferWidth;
	int					m_nBackBufferHeight;

	DWORD				m_dwAlphaRefValue;

	LPDIRECT3DSURFACE9	m_pBackBuffer;
	LPDIRECT3DSURFACE9	m_pBackBufferPT;
	LPDIRECT3DSURFACE9	m_pBackBufferOrigin;
	LPDIRECT3DSURFACE9	m_pDepthBuffer;	
	LPDIRECT3DSURFACE9	m_pDepthBufferPT;
	LPDIRECT3DSURFACE9	m_pDepthBufferOrigin;

	LPDIRECT3DSURFACE9	m_pDepthBufferRTT;

	int					m_nCaptureCount;
	IDirect3DStateBlock9*				m_pSaveStateToken[ MAX_STATE_SAVE ];

// #ifdef _USAGE_TOOL_   // xui 쓸려면 이걸 얻어내야 한다.. 할수 없다.. by mapping
public:
	LPDIRECT3DDEVICE9 GetD3DDevice() { return m_pD3DDevice; }
	LPDIRECT3D9		  GetD3D()		 { return m_pD3D;		}
	D3DPRESENT_PARAMETERS *GetPresentParameter() { return &m_Param; }
//#endif

protected:
	IDirect3DVertexShader9* m_pDefaultVertexShader;
	IDirect3DPixelShader9* m_pDefaultPixelShader;
	IDirect3DVertexDeclaration9* m_pDefaultVertexDeclaration;
	volatile bool		m_bReleaseDevice;
	HANDLE 				m_hReleaseEvent;;

public:
	void				CreateDefaultShader();
	void				SetDefaultShader( const D3DXMATRIX *matWVP , D3DXVECTOR4 *diffColor);
	void				ReleaseDeviceNextShow(){m_bReleaseDevice = true;}
	HANDLE				GetReleaseEventHandle(){return m_hReleaseEvent;}
};