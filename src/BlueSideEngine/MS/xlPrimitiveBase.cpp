///////////////////////////////////////////////////////////////////////////////
// xlPrimitiveBase.cpp
// Collection: xlPrimitiveBase
//-----------------------------------------------------------------------------

//=============================================================================
// includes
//=============================================================================
#include "stdafx.h"
#ifdef	_XBOX
#include "xlPrimitiveBase.h"
//=============================================================================
// namespace
//=============================================================================
namespace xl {
	//=============================================================================
	// macros
	//=============================================================================
	//#define DEBUG_VS   // Uncomment this line to debug vertex shaders 
	//#define DEBUG_PS   // Uncomment this line to debug pixel shaders 

	//=============================================================================
	// structs
	//=============================================================================
	//=============================================================================
	// classes
	//=============================================================================
	//=============================================================================
	// variables
	//=============================================================================
	//=============================================================================
	// methods
	//=============================================================================

	//-----------------------------------------------------------------------------
	// Name: PrimitiveBase()
	// Desc: 
	//-----------------------------------------------------------------------------
	PrimitiveBase::PrimitiveBase()
	{
		m_pd3dDevice		= NULL;

		m_pVertexBuffer		= NULL;
		m_dwNbVertices		= 0;
		m_dwVertexStride	= 0;

		m_pIndexBuffer		= NULL;
		m_dwNbIndices		= 0;
		m_dwIndxSize		= 0;

		m_pLineIndexBuffer  = NULL;
		m_dwNbLineIndices   = 0;
		m_dwLineIndxSize    = 0;

		for( DWORD i = 0; i < PRIMITIVEBASE_NB_TEXTURE; i++ ){
			m_pTexture[ i ] = NULL;
		}

		for( DWORD i = 0; i < PRIMITIVEBASE_NB_TEXTURE; i++ ){
			m_dwTextureDependenceFlag[ i ] = FALSE;
		}

		m_pVertexShader		= NULL;
		m_pVertexDeclaration= NULL;
		m_pPixelShader		= NULL;
		m_pEffect           = NULL;
		m_pEffectFXL		= NULL;
	}

	//-----------------------------------------------------------------------------
	// Name: ~PrimitiveBase()
	// Desc: 
	//-----------------------------------------------------------------------------
	PrimitiveBase::~PrimitiveBase()
	{
	}

	//-----------------------------------------------------------------------------
	// Name: Create()
	// Desc: 
	//-----------------------------------------------------------------------------
	PrimitiveBase*	PrimitiveBase::Create( LPDIRECT3DDEVICE pd3dDevice )
	{
		PrimitiveBase*  pPrimitiveBase;
		if( NULL == ( pPrimitiveBase = new PrimitiveBase ) )
			return	NULL;

		if( FAILED( pPrimitiveBase->Construct( pd3dDevice ) ) ){
			pPrimitiveBase->Release();
			return	NULL;
		}

		return pPrimitiveBase;
	}

	//-----------------------------------------------------------------------------
	// Name: Construct()
	// Desc: 
	//-----------------------------------------------------------------------------
	HRESULT	PrimitiveBase::Construct( LPDIRECT3DDEVICE9 pd3dDevice )
	{
		// store
		m_pd3dDevice = pd3dDevice;

		return	S_OK;
	}

	//-----------------------------------------------------------------------------
	// Name: Release()
	// Desc: 
	//-----------------------------------------------------------------------------
	void	PrimitiveBase::Release()
	{
		SAFE_RELEASE( m_pVertexBuffer );
		SAFE_RELEASE( m_pIndexBuffer );
		SAFE_RELEASE( m_pLineIndexBuffer );

		for( DWORD i = 0; i < PRIMITIVEBASE_NB_TEXTURE; i++ ){
			if( m_dwTextureDependenceFlag[ i ] ){
				SAFE_RELEASE( m_pTexture[ i ] );
				m_dwTextureDependenceFlag[ i ] = FALSE;
			}
		}

		SAFE_RELEASE( m_pVertexShader );
		SAFE_RELEASE( m_pVertexDeclaration );

		SAFE_RELEASE( m_pPixelShader );
		SAFE_RELEASE( m_pEffect );
	}

	//-----------------------------------------------------------------------------
	// Name: CreateVertexBuffer()
	// Desc: 
	//-----------------------------------------------------------------------------
	LPDIRECT3DVERTEXBUFFER	PrimitiveBase::CreateVertexBuffer( DWORD dwNbVertices, DWORD dwVertexStride, DWORD dwUsage, void *pVertices )
	{
		// store
		m_dwNbVertices		= dwNbVertices;
		m_dwVertexStride	= dwVertexStride;

		// create the vertex buffer.
		if( FAILED( m_pd3dDevice->CreateVertexBuffer( dwNbVertices * dwVertexStride,
			dwUsage,
			0L,
			0L,
			&m_pVertexBuffer,
			NULL ) ) ){
				return	NULL;
			}

			if( pVertices ){
				void	*pPrimitiveBaseV;
				if( FAILED( m_pVertexBuffer->Lock( 0, 0, &pPrimitiveBaseV, 0 ) ) )
					return	NULL;

				memcpy( pPrimitiveBaseV, pVertices, dwNbVertices * dwVertexStride );
				m_pVertexBuffer->Unlock();
			}

			return	m_pVertexBuffer;
	}

	//-----------------------------------------------------------------------------
	// Name: SetVertexBuffer()
	// Desc: 
	//-----------------------------------------------------------------------------
	HRESULT PrimitiveBase::SetVertexBuffer( void* pVertices )
	{   
		if( pVertices ){
			void	*pPrimitiveBaseV;
			if( FAILED( m_pVertexBuffer->Lock( 0, 0, &pPrimitiveBaseV, 0 ) ) )
				return	NULL;

			memcpy( pPrimitiveBaseV, pVertices, m_dwNbVertices * m_dwVertexStride );
			m_pVertexBuffer->Unlock();
		}

		return  S_OK;
	}

	//-----------------------------------------------------------------------------
	// Name: CreateIndexBuffer()
	// Desc: 
	//-----------------------------------------------------------------------------
	LPDIRECT3DINDEXBUFFER	PrimitiveBase::CreateIndexBuffer( DWORD dwNbIndices, DWORD dwIndxSize, D3DFORMAT d3dFormat, void* pIndices )
	{
		// store
		m_dwNbIndices = dwNbIndices;
		m_dwIndxSize = dwIndxSize;

		if( FAILED( m_pd3dDevice->CreateIndexBuffer( dwNbIndices * dwIndxSize,
			D3DUSAGE_WRITEONLY,
			d3dFormat,
			D3DPOOL_DEFAULT,
			&m_pIndexBuffer,
			NULL ) ) ){
				return	NULL;
			}


			if( pIndices ){
				void	*pPrimitiveBaseI;
				if( FAILED( m_pIndexBuffer->Lock( 0, 0, &pPrimitiveBaseI, 0 ) ) )
					return	NULL;

				memcpy( pPrimitiveBaseI, pIndices, dwNbIndices * dwIndxSize );
				m_pIndexBuffer->Unlock();
			}

			return	m_pIndexBuffer;
	}

	//-----------------------------------------------------------------------------
	// Name: CreateLineIndexBuffer()
	// Desc: 
	//-----------------------------------------------------------------------------
	LPDIRECT3DINDEXBUFFER	PrimitiveBase::CreateLineIndexBuffer( DWORD dwNbLineIndices, DWORD dwLineIndxSize, D3DFORMAT d3dFormat, void* pLineIndices )
	{
		// store
		m_dwNbLineIndices = dwNbLineIndices;
		m_dwLineIndxSize = dwLineIndxSize;

		if( FAILED( m_pd3dDevice->CreateIndexBuffer( dwNbLineIndices * dwLineIndxSize,
			D3DUSAGE_WRITEONLY,
			d3dFormat,
			D3DPOOL_DEFAULT,
			&m_pLineIndexBuffer,
			NULL ) ) ){
				return	NULL;
			}


			if( pLineIndices ){
				void	*pPrimitiveBaseI;
				if( FAILED( m_pLineIndexBuffer->Lock( 0, 0, &pPrimitiveBaseI, 0 ) ) )
					return	NULL;

				memcpy( pPrimitiveBaseI, pLineIndices, dwNbLineIndices * dwLineIndxSize );
				m_pLineIndexBuffer->Unlock();
			}

			return	m_pLineIndexBuffer;
	}

	//-----------------------------------------------------------------------------
	// Name: CreateTexture()
	// Desc: 
	//-----------------------------------------------------------------------------
	LPDIRECT3DTEXTURE   PrimitiveBase::CreateTexture( DWORD dwN, UINT uWidth, UINT  uHeight,
		UINT  uMipLevels,       DWORD dwUsage, D3DFORMAT d3dFormat )
	{
		if( FAILED( D3DXCreateTexture( m_pd3dDevice, uWidth, uHeight, uMipLevels, dwUsage, d3dFormat, 0, &m_pTexture[ dwN ] ) ) ){
			return  NULL;
		}

		m_dwTextureDependenceFlag[ dwN ] = TRUE;

		return  m_pTexture[ dwN ]; 
	}

	//-----------------------------------------------------------------------------
	// Name: CreateTexture()
	// Desc: 
	//-----------------------------------------------------------------------------
	LPDIRECT3DTEXTURE	PrimitiveBase::CreateTexture( DWORD dwN, const TCHAR* strResourceFileName )
	{
		HRESULT	hr;
		if( FAILED( hr = D3DXCreateTextureFromFile( m_pd3dDevice, strResourceFileName, &m_pTexture[ dwN ] ) ) ){			
			return	NULL;
		}
		m_dwTextureDependenceFlag[ dwN ] = TRUE;

		return	m_pTexture[ dwN ];
	}

	//-----------------------------------------------------------------------------
	// Name: SetTexture()
	// Desc: 
	//-----------------------------------------------------------------------------
	HRESULT PrimitiveBase::SetTexture( DWORD dwN, LPDIRECT3DTEXTURE pTexture )
	{
		m_pTexture[ dwN ] = pTexture;

		m_dwTextureDependenceFlag[ dwN ] = FALSE;

		return	S_OK;
	}

	//-----------------------------------------------------------------------------
	// Name: ReleaseTexture()
	// Desc: 
	//-----------------------------------------------------------------------------
	void	PrimitiveBase::ReleaseTexture( DWORD dwN )
	{
		SAFE_RELEASE( m_pTexture[ dwN ] );
		m_dwTextureDependenceFlag[ dwN ] = FALSE;
	}

	//-----------------------------------------------------------------------------
	// Name: CreateVertexShader()
	// Desc: 
	//-----------------------------------------------------------------------------
	LPDIRECT3DVERTEXSHADER	PrimitiveBase::CreateVertexShader( D3DVERTEXELEMENT* decl, const TCHAR* strShaderFilename )
	{
		// check shader version
		D3DCAPS9 caps;
		m_pd3dDevice->GetDeviceCaps( &caps );
		if( caps.VertexShaderVersion < D3DVS_VERSION( 1, 1 ) )
			return	NULL;

		// create vertex shader
		{
			HRESULT			hr;
			LPD3DXBUFFER	pCode;
			LPD3DXBUFFER	pErrorMsgs; 

			if( FAILED( hr = m_pd3dDevice->CreateVertexDeclaration( decl, &m_pVertexDeclaration ) ) ){
				return	NULL;
			}

			DWORD dwFlags = 0;
#if defined( _DEBUG ) || defined( DEBUG )
			dwFlags |= D3DXSHADER_DEBUG;
#endif
			if( FAILED( hr = D3DXAssembleShaderFromFile( strShaderFilename, NULL, NULL, dwFlags, &pCode, &pErrorMsgs ) ) ){
				return	NULL;
			}

			// create the vertex shader
			hr = m_pd3dDevice->CreateVertexShader( ( DWORD * )pCode->GetBufferPointer(), &m_pVertexShader );
			pCode->Release();
			if( FAILED( hr ) )
				return	NULL;
		}

		return	m_pVertexShader;
	}

	//-----------------------------------------------------------------------------
	// Name: ReleaseVertexShader()
	// Desc: 
	//-----------------------------------------------------------------------------
	void	PrimitiveBase::ReleaseVertexShader()
	{
		SAFE_RELEASE( m_pVertexShader );
		SAFE_RELEASE( m_pVertexDeclaration );
	}

	//-----------------------------------------------------------------------------
	// Name: CreatePixelShader()
	// Desc: 
	//-----------------------------------------------------------------------------
	LPDIRECT3DPIXELSHADER	PrimitiveBase::CreatePixelShader( const TCHAR* strShaderFilename )
	{
		D3DCAPS9 caps;
		m_pd3dDevice->GetDeviceCaps( &caps );
		if( caps.PixelShaderVersion < D3DPS_VERSION( 2, 0 ) )
			return	NULL;

		// create pixel shader
		{
			HRESULT			hr;
			LPD3DXBUFFER	pCode;                  // buffer with the assembled shader code
			LPD3DXBUFFER	pErrorMsgs;             // buffer with error messages

			DWORD dwFlags = 0;
#if defined( _DEBUG ) || defined( DEBUG )
			dwFlags |= D3DXSHADER_DEBUG;
#endif
			if( FAILED( hr = D3DXAssembleShaderFromFile( strShaderFilename, NULL, NULL, dwFlags, &pCode, &pErrorMsgs ) ) ){
				return	NULL;
			}

			hr = m_pd3dDevice->CreatePixelShader( ( DWORD* )pCode->GetBufferPointer(), &m_pPixelShader );
			pCode->Release();
			if( FAILED( hr ) )
				return	NULL;
		}

		return	m_pPixelShader;
	}

	//-----------------------------------------------------------------------------
	// Name: ReleasePixelShader()
	// Desc: 
	//-----------------------------------------------------------------------------
	void	PrimitiveBase::ReleasePixelShader()
	{
		SAFE_RELEASE( m_pPixelShader );
	}


	//-----------------------------------------------------------------------------
	// Name: PrimitiveBase::CreateEffectFXL()
	// Arguments:
	//	decl		vertex decleration
	//	pCode		code
	//	dwSize		size
	// Returns:
	//	LPFXLEFFECT
	//-----------------------------------------------------------------------------
	LPFXLEFFECT	PrimitiveBase::CreateEffectFXL( const D3DVERTEXELEMENT	decl[],
		const LPVOID			pCode,
		DWORD					dwSize )
	{
		// decl
		m_pVertexElement	= ( D3DVERTEXELEMENT* )decl;

		HRESULT			hr;

		if( FAILED( hr = m_pd3dDevice->CreateVertexDeclaration( decl, &m_pVertexDeclaration ) ) ){
			return	NULL;
		}

		if( pCode ){

			// compile
			LPD3DXBUFFER	pEffectData;
			LPD3DXBUFFER	pErrorMsgs	= NULL;
			DWORD           dwFlags		= D3DXSHADER_FXLPARAMETERS_AS_VARIABLE_NAMES;

			if( FAILED( hr = FXLCompileEffect( ( CHAR* )pCode, dwSize,
				NULL, NULL,
				dwFlags,
				&pEffectData, &pErrorMsgs ) ) ){
					return	NULL;
				}

				if( FAILED( hr = FXLCreateEffect( m_pd3dDevice,
					pEffectData->GetBufferPointer(),
					NULL,
					&m_pEffectFXL ) ) ){						
						return	NULL;
					}

					SAFE_RELEASE( pEffectData );
					SAFE_RELEASE( pErrorMsgs );
		}

		return	m_pEffectFXL;
	}


	//-----------------------------------------------------------------------------
	// Name: PrimitiveBase::CompileEffectFXL()
	// Arguments:
	//	decl			vertex decleration
	//	pCode			code
	//	dwSize			size
	//	pCompiledCode
	// Returns:
	//	LPD3DXBUFFER
	//-----------------------------------------------------------------------------
	LPD3DXBUFFER	PrimitiveBase::CompileEffectFXL( const LPVOID			pCode,
		DWORD					dwSize,
		LPD3DXBUFFER*			ppCompiledCodeBuffer,
		LPD3DXBUFFER*			ppErrorMsgBuffer )
	{
		// decl
		HRESULT			hr;

		if( NULL == pCode )
			return	NULL;

		// compile
		DWORD		dwFlags		= D3DXSHADER_SKIPOPTIMIZATION | 
			D3DXSHADER_FXLPARAMETERS_AS_VARIABLE_NAMES;

		if( FAILED( hr = FXLCompileEffect( ( CHAR* )pCode, dwSize,
			NULL, NULL,
			dwFlags,
			ppCompiledCodeBuffer,
			ppErrorMsgBuffer ) ) ){				
				return	NULL;
			}

			return	*ppCompiledCodeBuffer;
	}


	//-----------------------------------------------------------------------------
	// Name: PrimitiveBase::CreateEffectFXLCompiled()
	// Arguments:
	//	VOID*			pCode
	//	FXLEffectPool*	pPool	
	// Returns:
	//	LPFXLEFFECT
	//-----------------------------------------------------------------------------
	LPFXLEFFECT	PrimitiveBase::CreateEffectFXLCompiled( const D3DVERTEXELEMENT	decl[],
		const LPVOID			pCompiledCode,
		FXLEffectPool*			pPool )
	{
		// decl
		m_pVertexElement	= ( D3DVERTEXELEMENT* )decl;

		HRESULT			hr;
		if( FAILED( hr = m_pd3dDevice->CreateVertexDeclaration( decl, &m_pVertexDeclaration ) ) ){			
			return	NULL;
		}

		if( pCompiledCode ){

			if( FAILED( hr = FXLCreateEffect( m_pd3dDevice,
				pCompiledCode,
				NULL,
				&m_pEffectFXL ) ) ){					
					return	NULL;
				}
		}

		return	m_pEffectFXL;
	}


	//-----------------------------------------------------------------------------
	// Name: PrimitiveBase::CreateEffect()
	// Arguments:
	//	decl		vertex decleration
	//	strEffectFileName   effect file name
	// Returns:
	//	LPD3DXEFFECT
	//-----------------------------------------------------------------------------
	LPD3DXEFFECT    PrimitiveBase::CreateEffect( const D3DVERTEXELEMENT	decl[],
		const TCHAR*			strEffectFileName )
	{
		// decl
		HRESULT			hr;

		if( FAILED( hr = m_pd3dDevice->CreateVertexDeclaration( decl, &m_pVertexDeclaration ) ) ){
			return	NULL;
		}

		// compile
		LPD3DXBUFFER	pErrorMsgs = NULL;
		DWORD           dwFlags	= 0;
#if defined( _DEBUG ) || defined( DEBUG )
		dwFlags |= D3DXSHADER_DEBUG;
		//	dwFlags |= D3DXSHADER_SKIPOPTIMIZATION;
#endif
		/*
		#ifdef DEBUG_VS
		dwFlags |= D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT;
		#endif
		#ifdef DEBUG_PS
		dwFlags |= D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT;
		#endif
		*/
		if( FAILED( hr = D3DXCreateEffectFromFile( m_pd3dDevice, strEffectFileName, NULL, NULL, dwFlags, NULL, &m_pEffect, &pErrorMsgs ) ) ){
			return	NULL;
		}

		SAFE_RELEASE( pErrorMsgs );

		return	m_pEffect;
	}


	//=============================================================================
	// namespace
	//=============================================================================
} // end of namespace
#endif
