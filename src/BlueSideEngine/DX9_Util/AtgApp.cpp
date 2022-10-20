//--------------------------------------------------------------------------------------
// AtgApp.cpp
//
// Application class for samples
//
// Xbox Advanced Technology Group.
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "stdafx.h"
#include "AtgApp.h"
#include "AtgUtil.h"

// Ignore warning about "unused" pD3D variable
#pragma warning( disable: 4189 )

namespace ATG
{

//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------

// Global access to the main D3D device
//extern IDirect3DDevice9* g_pd3dDevice;

// Private access to the main D3D device
LPDIRECT3DDEVICE9     Application::m_pd3dDevice = NULL;

// The device-creation presentation params with reasonable defaults
D3DPRESENT_PARAMETERS Application::m_d3dpp = 
{
    640,                // BackBufferWidth;
    480,                // BackBufferHeight;
    D3DFMT_A8R8G8B8,    // BackBufferFormat;
    1,                  // BackBufferCount;
    D3DMULTISAMPLE_NONE,// MultiSampleType;
    0,                  // MultiSampleQuality;
    D3DSWAPEFFECT_DISCARD, // SwapEffect;
    NULL,               // hDeviceWindow;
    FALSE,              // Windowed;
    TRUE,               // EnableAutoDepthStencil;
    D3DFMT_D24S8,       // AutoDepthStencilFormat;
    0,                  // Flags;
    0,                  // FullScreen_RefreshRateInHz;
    D3DPRESENT_INTERVAL_IMMEDIATE, // FullScreen_PresentationInterval;
};


//--------------------------------------------------------------------------------------
// Name: Run()
// Desc: Creates the D3D device, calls Initialize() and enters an infinite loop
//       calling Update() and Render()
//--------------------------------------------------------------------------------------
VOID Application::Run()
{
    HRESULT hr;
    // Allow global access to the device
    m_pd3dDevice = g_pd3dDevice;

    // Initialize the app's device-dependent objects
    if( FAILED( hr = Initialize() ) )
    {
        ATG_PrintError( "Call to Initialize() failed!\n" );
        DebugBreak();
    }

    //// Run the game loop
    //for(;;)
    //{
    //    // Update the scene
    //    Update();

    //    // Render the scene
    //    Render();
    //}
}

} // namespace ATG
