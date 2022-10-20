///////////////////////////////////////////////////////////////////////////////
// xlPrimitiveBase.h
// Collection: xlPrimitiveBase
//-----------------------------------------------------------------------------
#ifndef __inc_xlPrimitiveBase_h__
#define __inc_xlPrimitiveBase_h__

//=============================================================================
// includes
//=============================================================================
#include "xlCommon.h"
//=============================================================================
// namespace
//=============================================================================
namespace xl {
//=============================================================================
// macros
//=============================================================================
#define	PRIMITIVEBASE_NB_TEXTURE	8

//=============================================================================
// structs
//=============================================================================
//=============================================================================
// classes
//=============================================================================
class PrimitiveBase{
	protected:
		LPDIRECT3DDEVICE		    m_pd3dDevice;

		LPDIRECT3DVERTEXBUFFER	    m_pVertexBuffer;
		DWORD					    m_dwNbVertices;
 		DWORD						m_dwVertexStride;

		LPDIRECT3DINDEXBUFFER	    m_pIndexBuffer;
		DWORD					    m_dwNbIndices;
        DWORD                       m_dwIndxSize;

        LPDIRECT3DINDEXBUFFER       m_pLineIndexBuffer;
		DWORD				    	m_dwNbLineIndices;
        DWORD                       m_dwLineIndxSize;

        LPDIRECT3DTEXTURE			m_pTexture[ PRIMITIVEBASE_NB_TEXTURE ];
        DWORD                       m_dwTextureDependenceFlag[ PRIMITIVEBASE_NB_TEXTURE ];

        LPDIRECT3DVERTEXSHADER		m_pVertexShader;
		LPDIRECT3DVERTEXDECLARATION	m_pVertexDeclaration;
		LPDIRECT3DPIXELSHADER		m_pPixelShader;
		LPD3DXEFFECT				m_pEffect;
		LPFXLEFFECT					m_pEffectFXL;
		LPD3DVERTEXELEMENT			m_pVertexElement;

	public:
		PrimitiveBase();
		~PrimitiveBase();

		static PrimitiveBase*       Create( LPDIRECT3DDEVICE pd3dDevice );
        HRESULT                     Construct( LPDIRECT3DDEVICE pd3dDevice );
		void					    Release();

        // vertex buffer
		LPDIRECT3DVERTEXBUFFER	    CreateVertexBuffer( DWORD dwNbVertices, DWORD dwVertexStride, DWORD dwUsage, void* pVertices );
        void					    ReleaseVertexBuffer()                               { SAFE_RELEASE( m_pVertexBuffer ); }
        LPDIRECT3DVERTEXBUFFER	    GetVertexBuffer()                                   { return m_pVertexBuffer; }
        HRESULT                     SetVertexBuffer( void* pVertices );
        DWORD					    GetNbVertices()                                     { return m_dwNbVertices; }
		DWORD						GetVertexStride()									{ return m_dwVertexStride; }

        // index buffer
        LPDIRECT3DINDEXBUFFER	    CreateIndexBuffer( DWORD dwNbIndices, DWORD dwIndxSize, D3DFORMAT d3dFormat, void* pIndices );
        void					    ReleaseIndexBuffer()                                { SAFE_RELEASE( m_pIndexBuffer ); }
        LPDIRECT3DINDEXBUFFER	    GetIndexBuffer()                                    { return m_pIndexBuffer; }
        DWORD					    GetNbIndices()                                      { return m_dwNbIndices; }

        // line index buffer
		LPDIRECT3DINDEXBUFFER	    CreateLineIndexBuffer( DWORD dwNbIndices, DWORD dwIndxSize, D3DFORMAT d3dFormat, void* pIndices );
        void					    ReleaseLineIndexBuffer()                            { SAFE_RELEASE( m_pLineIndexBuffer ); }
        LPDIRECT3DINDEXBUFFER	    GetLineIndexBuffer()                                { return m_pLineIndexBuffer; }
        DWORD					    GetNbLineIndices()                                  { return m_dwNbLineIndices; }

        // texture
        LPDIRECT3DTEXTURE		    CreateTexture( DWORD dwN, UINT uWidth, UINT  uHeight,
                                                   UINT  uMipLevels,       DWORD dwUsage, D3DFORMAT d3dFormat );
        LPDIRECT3DTEXTURE		    CreateTexture( DWORD dwN, const TCHAR* strResourceFileName );
        void	 				    ReleaseTexture( DWORD dwN );
        HRESULT					    SetTexture( DWORD dwN, LPDIRECT3DTEXTURE pTexture );
        LPDIRECT3DTEXTURE		    GetTexture( DWORD dwN )                             { return m_pTexture[ dwN ]; }

        // vertex shader
		LPDIRECT3DVERTEXSHADER	    CreateVertexShader( D3DVERTEXELEMENT* decl, const TCHAR* strShaderFilename );
		void					    ReleaseVertexShader();

        // pixel shader
		LPDIRECT3DPIXELSHADER	    CreatePixelShader( const TCHAR* strShaderFilename );
		void					    ReleasePixelShader();

		// effect
		LPD3DXEFFECT				CreateEffect( const D3DVERTEXELEMENT decl[], const TCHAR* strEffectFileName );
		void                        ReleaseEffect()										{ SAFE_RELEASE( m_pEffect ); }		
		LPD3DXEFFECT				GetEffect()											{ return m_pEffect; }

		// effectFXL
		LPFXLEFFECT					CreateEffectFXL( const D3DVERTEXELEMENT	decl[], const LPVOID pCode, DWORD	dwSize );
		void                        ReleaseEffectFXL()									{ SAFE_RELEASE( m_pEffectFXL );
																						  SAFE_RELEASE( m_pVertexDeclaration ); }		
		LPFXLEFFECT					GetEffectFXL()										{ return m_pEffectFXL; }
		LPDIRECT3DVERTEXDECLARATION	GetVertexDeclaration()								{ return m_pVertexDeclaration; }
		LPD3DVERTEXELEMENT			GetVertexElement()									{ return m_pVertexElement; }

		LPD3DXBUFFER				CompileEffectFXL( const LPVOID				pCode,
													  DWORD						dwSize,
													  LPD3DXBUFFER*				pCompiledCodeBuffer,
													  LPD3DXBUFFER*				pErrorMsgBuffer );
		LPFXLEFFECT					CreateEffectFXLCompiled( const D3DVERTEXELEMENT	decl[],
															 const LPVOID pCompiledCode,
															 FXLEffectPool* pPool );
};

//=============================================================================
// namespace
//=============================================================================
} // end of namespace
#endif	// __inc_xlPrimitiveBase_h__
