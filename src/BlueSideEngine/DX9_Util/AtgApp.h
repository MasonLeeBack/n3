//--------------------------------------------------------------------------------------
// AtgApp.h
//
// Application class for samples
//
// Xbox Advanced Technology Group.
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#pragma once
#ifndef ATGAPP_H
#define ATGAPP_H

// Disable warning C4324: structure padded due to __declspec(align())
// Warning C4324 comes up often in samples derived from the Application class, since 
// they use XMVECTOR, XMMATRIX, and other member data types that must be aligned.
#pragma warning( disable:4324 )

namespace ATG
{


//--------------------------------------------------------------------------------------
// Error codes
//--------------------------------------------------------------------------------------
#define ATGAPPERR_MEDIANOTFOUND       0x82000003

// Global access to the main D3D device
extern LPDIRECT3DDEVICE9  g_pd3dDevice;

//--------------------------------------------------------------------------------------
// A base class for creating sample Xbox applications. To create a simple
// Xbox application, simply derive this class and override the following
// functions:
//    Initialize() - To initialize the device-dependant objects
//    Update()     - To animate the scene
//    Render()     - To render the scene
//--------------------------------------------------------------------------------------
class Application
{
public:
    // Main objects used for creating and rendering the 3D scene
    static D3DPRESENT_PARAMETERS m_d3dpp;
    static LPDIRECT3DDEVICE9     m_pd3dDevice;

    // Overridable functions
    virtual HRESULT Initialize() = 0;
    virtual HRESULT Update() = 0;
    virtual HRESULT Render() = 0;

    // Function to create and run the application
    VOID Run();
};

} // namespace ATG

#endif // ATGAPP_H
