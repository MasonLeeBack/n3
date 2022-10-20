//--------------------------------------------------------------------------------------
// AtgUtil.cpp
//
// Helper functions and typing shortcuts for samples
//
// Xbox Advanced Technology Group.
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "stdafx.h"
#include "AtgUtil.h"
#include "BsUtil.h"
#include "BsFileIO.h"

// Global access to the main D3D device
extern LPDIRECT3DDEVICE9 g_pd3dDevice;

namespace ATG
{

// Static shaders used for helper functions
static D3DVertexDeclaration* g_pGradientVertexDecl   = NULL;
static D3DVertexShader*      g_pGradientVertexShader = NULL;
static D3DPixelShader*       g_pGradientPixelShader  = NULL;

// Flag that can be modified externally to control whether execution should continue
// even after a fatal error
BOOL g_bContinueOnError = FALSE;



//--------------------------------------------------------------------------------------
// Name: DebugSpewV()
// Desc: Internal helper function
//--------------------------------------------------------------------------------------
static VOID DebugSpewV( const CHAR* strFormat, const va_list pArgList )
{
    CHAR str[2048];
    // Use the secure CRT to avoid buffer overruns. Specify a count of
    // _TRUNCATE so that too long strings will be silently truncated
    // rather than triggering an error.
    vsnprintf_s( str, ARRAYSIZE(str), _TRUNCATE, strFormat, pArgList );
//    OutputDebugStringA( str );
	DebugString( str );
}


//--------------------------------------------------------------------------------------
// Name: DebugSpew()
// Desc: Prints formatted debug spew
//--------------------------------------------------------------------------------------
VOID CDECL DebugSpew( const CHAR* strFormat, ... )
{
    va_list pArgList;
    va_start( pArgList, strFormat );
    DebugSpewV( strFormat, pArgList );
    va_end( pArgList );
}


//--------------------------------------------------------------------------------------
// Name: FatalError()
// Desc: Prints formatted debug spew and breaks into the debugger
//--------------------------------------------------------------------------------------
VOID CDECL FatalError( const CHAR* strFormat, ... )
{
    va_list pArgList;
    va_start( pArgList, strFormat );
    DebugSpewV( strFormat, pArgList );
    va_end( pArgList );

	_DEBUGBREAK;
//    ATG_DebugBreak();

#if 0
    // Optionally exit the app
    if( FALSE == g_bContinueOnError )
        exit(0);
#endif
}


//--------------------------------------------------------------------------------------
// Name: GetVideoSettings()
// Desc: Return various display settings
//--------------------------------------------------------------------------------------
VOID GetVideoSettings( UINT* pdwDisplayWidth, UINT* pdwDisplayHeight,
                       BOOL* pbWidescreen )
{
    // Return the user-preferences reported via the video mode
    XVIDEO_MODE VideoMode;
    XGetVideoMode( &VideoMode );
    if( pdwDisplayWidth )    (*pdwDisplayWidth)  = BsMin( VideoMode.dwDisplayWidth, 1280 );
    if( pdwDisplayHeight )   (*pdwDisplayHeight) = BsMin( VideoMode.dwDisplayHeight, 720 );
    if( pbWidescreen )       (*pbWidescreen)     = VideoMode.fIsWideScreen;
}


//--------------------------------------------------------------------------------------
// Name: GetTitleSafeArea()
// Desc: Returns the title safe area for the given display mode
//--------------------------------------------------------------------------------------
D3DRECT GetTitleSafeArea()
{
    D3DDISPLAYMODE mode;
    g_pd3dDevice->GetDisplayMode( 0, &mode );

    D3DRECT rcSafeArea;
    rcSafeArea.x1 = (LONG)(mode.Width  * 0.1f);
    rcSafeArea.y1 = (LONG)(mode.Height * 0.1f);
    rcSafeArea.x2 = (LONG)(mode.Width  * 0.9f);
    rcSafeArea.y2 = (LONG)(mode.Height * 0.9f);
    return rcSafeArea;
}


//--------------------------------------------------------------------------------------
// Name: CreateNormalizationCubeMap()
// Desc: Creates a cubemap and fills it with normalized UVW vectors
//--------------------------------------------------------------------------------------
HRESULT CreateNormalizationCubeMap( DWORD dwSize, D3DCubeTexture** ppCubeMap )
{
    // Create the cube map
    HRESULT hr = g_pd3dDevice->CreateCubeTexture( dwSize, 1, 0, D3DFMT_LIN_Q8W8V8U8, 
                                                  D3DPOOL_DEFAULT, ppCubeMap, NULL );
    if( FAILED( hr ) )
    {
        ATG_PrintError( "CreateCubeTexture() failed.\n" );
        return hr;
    }
    
    // Allocate temp space for swizzling the cubemap surfaces
    DWORD* pSourceBits = new DWORD[ dwSize * dwSize ];

    // Fill all six sides of the cubemap
    for( DWORD i=0; i<6; i++ )
    {
        // Lock the i'th cubemap surface
        D3DSurface* pCubeMapFace;
        (*ppCubeMap)->GetCubeMapSurface( (D3DCUBEMAP_FACES)i, 0, &pCubeMapFace );

        // Write the RGBA-encoded normals to the surface pixels
        DWORD*   pPixel = pSourceBits;
        XMVECTOR n = XMVectorSet( 0,0,0,0 );
        FLOAT    w, h;

        for( DWORD y = 0; y < dwSize; y++ )
        {
			BsAssert( dwSize > 1 );	// mruete: prefix bug 486: added assert
            h  = (FLOAT)y / (FLOAT)(dwSize-1);  // 0 to 1
            h  = ( h * 2.0f ) - 1.0f;           // -1 to 1
            
            for( DWORD x = 0; x < dwSize; x++ )
            {
                w = (FLOAT)x / (FLOAT)(dwSize-1);   // 0 to 1
                w = ( w * 2.0f ) - 1.0f;            // -1 to 1

                // Calc the normal for this texel
                switch( i )
                {
                    case D3DCUBEMAP_FACE_POSITIVE_X:    // +x
                        n.x = +1.0;
                        n.y = -h;
                        n.z = -w;
                        break;
                        
                    case D3DCUBEMAP_FACE_NEGATIVE_X:    // -x
                        n.x = -1.0;
                        n.y = -h;
                        n.z = +w;
                        break;
                        
                    case D3DCUBEMAP_FACE_POSITIVE_Y:    // y
                        n.x = +w;
                        n.y = +1.0;
                        n.z = +h;
                        break;
                        
                    case D3DCUBEMAP_FACE_NEGATIVE_Y:    // -y
                        n.x = +w;
                        n.y = -1.0;
                        n.z = -h;
                        break;
                        
                    case D3DCUBEMAP_FACE_POSITIVE_Z:    // +z
                        n.x = +w;
                        n.y = -h;
                        n.z = +1.0;
                        break;
                        
                    case D3DCUBEMAP_FACE_NEGATIVE_Z:    // -z
                        n.x = -w;
                        n.y = -h;
                        n.z = -1.0;
                        break;
                }

                // Store the normal as an signed QWVU vector
                n = XMVector3Normalize( n );
                *pPixel++ = VectorToQWVU( n );
            }
        }
        
        // Swizzle the result into the cubemap face surface
        D3DLOCKED_RECT lock;
        pCubeMapFace->LockRect( &lock, 0, 0L );
        memcpy( lock.pBits, pSourceBits, dwSize*dwSize*sizeof(DWORD) );
        pCubeMapFace->UnlockRect();

        // Release the cubemap face
        pCubeMapFace->Release();
    }

    delete [] pSourceBits;
    return S_OK;
}

//--------------------------------------------------------------------------------------
// Name: SetVertexElement()
// Desc: Helper function for creating vertex declarations
//--------------------------------------------------------------------------------------
inline D3DVERTEXELEMENT9 SetVertexElement( WORD &Offset, DWORD Type,
                                           BYTE Usage, BYTE UsageIndex )
{
    D3DVERTEXELEMENT9 Element;
    Element.Stream     = 0;
    Element.Offset     = Offset;
    Element.Type       = Type;
    Element.Method     = D3DDECLMETHOD_DEFAULT;
    Element.Usage      = Usage;
    Element.UsageIndex = UsageIndex;

    switch( Type )
    {
        case D3DDECLTYPE_FLOAT1:   Offset += 1*sizeof(FLOAT); break;
        case D3DDECLTYPE_FLOAT2:   Offset += 2*sizeof(FLOAT); break;
        case D3DDECLTYPE_FLOAT3:   Offset += 3*sizeof(FLOAT); break;
        case D3DDECLTYPE_FLOAT4:   Offset += 4*sizeof(FLOAT); break;
        case D3DDECLTYPE_D3DCOLOR: Offset += 1*sizeof(DWORD); break;
    }
    return Element;
}


//--------------------------------------------------------------------------------------
// Name: BuildVertexDeclFromFVF()
// Desc: Helper function to create vertex declarations
//--------------------------------------------------------------------------------------
VOID BuildVertexDeclFromFVF( DWORD dwFVF, D3DVERTEXELEMENT9* pDecl )
{
    WORD wOffset = 0;

    // Handle position
    switch( dwFVF & D3DFVF_POSITION_MASK )
    {
        case D3DFVF_XYZ:
            *pDecl++ = SetVertexElement( wOffset, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION, 0 ); break;
        case D3DFVF_XYZW:
            *pDecl++ = SetVertexElement( wOffset, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_POSITION, 0 ); break;
        case D3DFVF_XYZB1:
            *pDecl++ = SetVertexElement( wOffset, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION, 0 );
            *pDecl++ = SetVertexElement( wOffset, D3DDECLTYPE_FLOAT1, D3DDECLUSAGE_BLENDWEIGHT, 0 ); break;
        case D3DFVF_XYZB2:
            *pDecl++ = SetVertexElement( wOffset, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION, 0 );
            *pDecl++ = SetVertexElement( wOffset, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_BLENDWEIGHT, 0 ); break;
        case D3DFVF_XYZB3:
            *pDecl++ = SetVertexElement( wOffset, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION, 0 );
            *pDecl++ = SetVertexElement( wOffset, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_BLENDWEIGHT, 0 ); break;
        case D3DFVF_XYZB4:
            *pDecl++ = SetVertexElement( wOffset, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION, 0 );
            *pDecl++ = SetVertexElement( wOffset, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_BLENDWEIGHT, 0 ); break;
    }

    // Handle normal, diffuse, and specular
    if( dwFVF & D3DFVF_NORMAL )    *pDecl++ = SetVertexElement( wOffset, D3DDECLTYPE_FLOAT3,   D3DDECLUSAGE_NORMAL, 0 );
    if( dwFVF & D3DFVF_DIFFUSE )   *pDecl++ = SetVertexElement( wOffset, D3DDECLTYPE_D3DCOLOR, D3DDECLUSAGE_COLOR, 0 );
    if( dwFVF & D3DFVF_SPECULAR )  *pDecl++ = SetVertexElement( wOffset, D3DDECLTYPE_D3DCOLOR, D3DDECLUSAGE_COLOR, 1 );

    // Handle texture coordinates
    DWORD dwNumTextures = (dwFVF & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;

    for( DWORD i=0; i<dwNumTextures; i++ )
    {
        LONG lTexCoordSize = ( dwFVF & (0x00030000<<(i*2)) );

        if( lTexCoordSize == D3DFVF_TEXCOORDSIZE1(i) ) *pDecl++ = SetVertexElement( wOffset, D3DDECLTYPE_FLOAT1, D3DDECLUSAGE_TEXCOORD, (BYTE)i );
        if( lTexCoordSize == D3DFVF_TEXCOORDSIZE2(i) ) *pDecl++ = SetVertexElement( wOffset, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, (BYTE)i );
        if( lTexCoordSize == D3DFVF_TEXCOORDSIZE3(i) ) *pDecl++ = SetVertexElement( wOffset, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_TEXCOORD, (BYTE)i );
        if( lTexCoordSize == D3DFVF_TEXCOORDSIZE4(i) ) *pDecl++ = SetVertexElement( wOffset, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_TEXCOORD, (BYTE)i );
    }

    // End the declarator
    pDecl->Stream     = 0xff;
    pDecl->Offset     = 0;
    pDecl->Type       = (DWORD)D3DDECLTYPE_UNUSED;
    pDecl->Method     = 0;
    pDecl->Usage      = 0;
    pDecl->UsageIndex = 0;
}



//--------------------------------------------------------------------------------------
// Vertex and pixel shaders for gradient background rendering
//--------------------------------------------------------------------------------------
static const CHAR* g_strGradientShader =
"struct VS_IN                                              \n"
"{                                                         \n"
"   float4   Position     : POSITION;                      \n"
"   float4   Color        : COLOR0;                        \n"
"};                                                        \n"
"                                                          \n"
"struct VS_OUT                                             \n"
"{                                                         \n"
"   float4 Position       : POSITION;                      \n"
"   float4 Diffuse        : COLOR0;                        \n"
"};                                                        \n"
"                                                          \n"
"VS_OUT GradientVertexShader( VS_IN In )                   \n"
"{                                                         \n"
"   VS_OUT Out;                                            \n"
"   Out.Position = In.Position;                            \n"
"   Out.Diffuse  = In.Color;                               \n"
"   return Out;                                            \n"
"}                                                         \n"
"                                                          \n"
"                                                          \n"
"float4 GradientPixelShader( VS_OUT In ) : COLOR0          \n"
"{                                                         \n"
"   return In.Diffuse;                                     \n"
"}                                                         \n";


//--------------------------------------------------------------------------------------
// Name: CreateGradientShaders()
// Desc: Creates the global gradient shaders
//--------------------------------------------------------------------------------------
HRESULT CreateGradientShaders()
{
    // Create vertex declaration
    if( NULL == g_pGradientVertexDecl )
    {   
        D3DVERTEXELEMENT9 decl[] = 
        {
            { 0,  0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
            { 0, 16, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
            D3DDECL_END()
        };

        if( FAILED( g_pd3dDevice->CreateVertexDeclaration( decl, &g_pGradientVertexDecl ) ) )
            return E_FAIL;
    }

    // Create vertex shader
    if( NULL == g_pGradientVertexShader )
    {
        ID3DXBuffer* pShaderCode;
        if( FAILED( D3DXCompileShader( g_strGradientShader, strlen(g_strGradientShader),
                                       NULL, NULL, "GradientVertexShader", "vs.2.0", 0,
                                       &pShaderCode, NULL, NULL ) ) )
            return E_FAIL;

        if( FAILED( g_pd3dDevice->CreateVertexShader( (DWORD*)pShaderCode->GetBufferPointer(),
                                                      &g_pGradientVertexShader ) ) )
            return E_FAIL;
            
        pShaderCode->Release();
    }
    
    // Create pixel shader.
    if( NULL == g_pGradientPixelShader )
    {
        ID3DXBuffer* pShaderCode;
        if( FAILED( D3DXCompileShader( g_strGradientShader, strlen(g_strGradientShader),
                                       NULL, NULL, "GradientPixelShader", "ps.2.0", 0,
                                       &pShaderCode, NULL, NULL ) ) )
            return E_FAIL;

        if( FAILED( g_pd3dDevice->CreatePixelShader( (DWORD*)pShaderCode->GetBufferPointer(),
                                                     &g_pGradientPixelShader ) ) )
            return E_FAIL;
            
        pShaderCode->Release();
    }

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Name: RenderBackground()
// Desc: Draws a gradient filled background
//--------------------------------------------------------------------------------------
VOID RenderBackground( DWORD dwTopColor, DWORD dwBottomColor )
{
    // Clear the depth-buffer
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL,
                         0x00000000, 1.0f, 0L );

    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE,  FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_FILLMODE,   D3DFILL_SOLID );
    g_pd3dDevice->SetRenderState( D3DRS_CULLMODE,   D3DCULL_CCW );
    g_pd3dDevice->SetRenderState( D3DRS_VIEWPORTENABLE, TRUE );

    // Draw a background-filling quad
    struct VERTEX
    { 
        FLOAT sx, sy, sz, rhw;
        DWORD Color;
    };

    VERTEX v[3] = 
    {
        {       -1.0f,              1.0f,        0.0f, 1.0f, dwTopColor },
        {        1.0f,              1.0f,        0.0f, 1.0f, dwTopColor },
        {       -1.0f,             -1.0f,        0.0f, 1.0f, dwBottomColor },
    };

    // The shaders will be created on the first call to CreateGradientShaders()
    // and then re-used for subsequent calls
    if( FAILED( CreateGradientShaders() ) )
        FatalError( "Couldn't create shaders for RenderBackground" );

    g_pd3dDevice->SetVertexDeclaration( g_pGradientVertexDecl );
    g_pd3dDevice->SetVertexShader( g_pGradientVertexShader );
    g_pd3dDevice->SetPixelShader( g_pGradientPixelShader );
    g_pd3dDevice->DrawPrimitiveUP( D3DPT_RECTLIST, 1, v, sizeof(VERTEX) );
}


//--------------------------------------------------------------------------------------
// Name: LoadVertexShader()
// Desc: Loads pre-compiled vertex shader microcode from the specified file and
//       creates a vertex shader resource.
//--------------------------------------------------------------------------------------
HRESULT LoadVertexShader( const CHAR* strFileName, LPDIRECT3DVERTEXSHADER9* ppVS )
{
    HRESULT hr;
    VOID* pCode = NULL;

	if( FAILED( hr = CBsFileIO::LoadFile( strFileName, &pCode ) ) )
        return hr;

	hr = g_pd3dDevice->CreateVertexShader( (DWORD*)pCode, ppVS );
	CBsFileIO::FreeBuffer( pCode );

    return hr;
}


//--------------------------------------------------------------------------------------
// Name: LoadPixelShader()
// Desc: Loads pre-compiled pixel shader microcode from the specified file and
//       creates a pixel shader resource.
//--------------------------------------------------------------------------------------
HRESULT LoadPixelShader( const CHAR* strFileName, LPDIRECT3DPIXELSHADER9* ppPS )
{
    HRESULT hr;
    VOID* pCode = NULL;

	if( FAILED( hr = CBsFileIO::LoadFile( strFileName, &pCode ) ) )
        return hr;
    
	hr = g_pd3dDevice->CreatePixelShader( (DWORD*)pCode, ppPS );
	CBsFileIO::FreeBuffer( pCode );

    return hr;
}


//--------------------------------------------------------------------------------------
// Name: AppendVertexElements()
// Desc: Helper function for building an array of vertex elements.
//--------------------------------------------------------------------------------------
VOID AppendVertexElements( D3DVERTEXELEMENT9* pDstElements, DWORD dwSrcStream, 
                           D3DVERTEXELEMENT9* pSrcElements, DWORD dwSrcUsageIndex,
                           DWORD dwSrcOffset )
{
    // Find the end of the destination stream
    while( pDstElements->Stream != 0xff && pDstElements->Type != 0L )
        pDstElements++;

    // Add the source elements
    for(;;)
    {
        pDstElements->Stream     = (WORD)dwSrcStream;
        pDstElements->Offset     = pSrcElements->Offset + (WORD)dwSrcOffset;
        pDstElements->Type       = pSrcElements->Type;
        pDstElements->Method     = pSrcElements->Method;
        pDstElements->Usage      = pSrcElements->Usage;
        pDstElements->UsageIndex = (BYTE)dwSrcUsageIndex;

        if( pSrcElements->Stream == 0xff )
        {
            pDstElements->Stream     = 0xff;
            pDstElements->Offset     = 0;
            pDstElements->Type       = (DWORD)D3DDECLTYPE_UNUSED;
            pDstElements->Method     = 0;
            pDstElements->Usage      = 0;
            pDstElements->UsageIndex = 0;
            break;
        }

        pSrcElements++;
        pDstElements++;
    }
}


//-----------------------------------------------------------------------------
// Endian-swapping functions
//-----------------------------------------------------------------------------
inline WORD ReverseEndianness( WORD in )
{
    WORD out;
    ((BYTE*)&out)[0] = ((BYTE*)&in)[1];
    ((BYTE*)&out)[1] = ((BYTE*)&in)[0];
    return out;
}

inline DWORD ReverseEndianness( DWORD in )
{
    DWORD out;
    ((BYTE*)&out)[0] = ((BYTE*)&in)[3];
    ((BYTE*)&out)[1] = ((BYTE*)&in)[2];
    ((BYTE*)&out)[2] = ((BYTE*)&in)[1];
    ((BYTE*)&out)[3] = ((BYTE*)&in)[0];
    return out;
}


} // namespace ATG
