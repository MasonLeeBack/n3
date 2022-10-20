//--------------------------------------------------------------------------------------
//    CWavebankStream.cpp
//
// Xbox Advanced Technology Group.
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "stdafx.h"
#ifdef _XBOX

#include <xtl.h>
#include <XAudio.h>
#include <XAct2wb.h>
#include "CWavebankStream.h"
#include "BsUtil.h"
#include "BSFileIO.h"

//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------
BYTE g_ReadBuffer[ MAX_STREAMVOICE ][ READBUFFER_SIZE ];

//--------------------------------------------------------------------------------------
// Name: CWavebankStream::Ctor
// Desc: 
//--------------------------------------------------------------------------------------
CWavebankStream::CWavebankStream()
{
    m_dwFileSize = NULL;
    m_pWavebankHeader = NULL;
    m_pWavebankData = NULL;
    m_pDataEntry = NULL;
    m_pEntryName = NULL;
    m_dwNumStreamVoice = 0;
}


//--------------------------------------------------------------------------------------
// Name: CWavebankStream::Dtor
// Desc: 
//--------------------------------------------------------------------------------------
CWavebankStream::~CWavebankStream()
{
    Close();
}


//--------------------------------------------------------------------------------------
// Name: CWavebankStream::Open
// Desc: Open the wave bank file and verify
//--------------------------------------------------------------------------------------
HRESULT CWavebankStream::Open( const CHAR* strFileName )
{
    BsAssert( strFileName != NULL );

    HRESULT hr;

    // Read the file header
	m_hFileBufferred = CBsFileIO::BsCreateFile( strFileName, GENERIC_READ, FILE_SHARE_READ, NULL, 
                               OPEN_EXISTING, 0, NULL );
    if( INVALID_HANDLE_VALUE == m_hFileBufferred )
        BsAssert( 0 &&"Error opening Wavebank file\n" );

    // Reopen the file with No buffering
    m_hFile = CBsFileIO::BsCreateFile( strFileName, GENERIC_READ, FILE_SHARE_READ, NULL, 
                               OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL );
    if( INVALID_HANDLE_VALUE == m_hFile )
        BsAssert( 0 && "Error opening Wavebank file\n" );

    m_dwFileSize = GetFileSize( m_hFile, NULL );
    m_pWavebankHeader = (WAVEBANKHEADER *)malloc( sizeof(WAVEBANKHEADER) );
    if( NULL == m_pWavebankHeader )
        BsAssert( 0 && "Error opening Wavebank file\n" );

    DWORD dwBytesRead;
    CBsFileIO::BsReadFile( m_hFileBufferred, m_pWavebankHeader, sizeof(WAVEBANKHEADER), &dwBytesRead, NULL );
    if( dwBytesRead != sizeof(WAVEBANKHEADER) )
        BsAssert( 0 && "Error opening Wavebank file\n" );

    // Verify header
    if( FAILED( hr = Verify() ) )
        BsAssert2( 0 && "Error %#X verifying Wavebank file\n", hr );

    // Allocate wavebank data
    DWORD dwAllocateSize = m_pWavebankHeader->Segments[ WAVEBANK_SEGIDX_BANKDATA ].dwLength;
    m_pWavebankData = (WAVEBANKDATA *)malloc( dwAllocateSize );
    if( NULL == m_pWavebankData )
        BsAssert( 0 && "Error allocating buffer\n" );

    // Read wavebank data
    CBsFileIO::BsReadFile( m_hFileBufferred, m_pWavebankData, dwAllocateSize, &dwBytesRead, NULL );
    if( dwBytesRead != dwAllocateSize )
        BsAssert( 0 && "Error opening Wavebank file\n" );
    
    if( !(m_pWavebankData->dwFlags & WAVEBANK_TYPE_STREAMING)) //aleksger: prefix bug 831: Operator precedence.
        BsAssert( 0 && "Only streaming Wave bank is supported.\n" );
    
    // Allocate wavebank data entry
    dwAllocateSize = m_pWavebankHeader->Segments[ WAVEBANK_SEGIDX_ENTRYMETADATA ].dwLength;
    m_pDataEntry = (WAVEBANKENTRY *)malloc( dwAllocateSize );
    if( NULL == m_pDataEntry )
        BsAssert( 0 && "Error allocating buffer\n" );

    // Read wavebank entry
    CBsFileIO::BsReadFile( m_hFileBufferred, m_pDataEntry, dwAllocateSize, &dwBytesRead, NULL );
    if( dwBytesRead != dwAllocateSize )
        BsAssert( 0 && "Error opening Wavebank file\n" );
    
    // Allocate wavebank entry names
    dwAllocateSize = m_pWavebankHeader->Segments[ WAVEBANK_SEGIDX_ENTRYNAMES ].dwLength;
    if( dwAllocateSize )
    {
        m_pEntryName = (char *)malloc( dwAllocateSize );
        if( NULL == m_pEntryName )
            BsAssert( 0 && "Error allocating buffer\n" );

        // Read wavebank entry
        CBsFileIO::BsReadFile( m_hFileBufferred, m_pEntryName, dwAllocateSize, &dwBytesRead, NULL );
        if( dwBytesRead != dwAllocateSize )
            BsAssert( 0 && "Error opening Wavebank file\n" );
    }

    for( int i = 0; i < MAX_STREAMVOICE; ++i )
    {
        m_pReadBuffer[ i ] = &g_ReadBuffer[ i ][ 0 ];
        m_hAudioPacketEvent[ i ] = XAudioCreateEvent( FALSE, FALSE );
    }
    m_hAudioPacketEvent[ MAX_STREAMVOICE ] = XAudioCreateEvent( FALSE, FALSE );

    m_hAudioReadEvent[ 0 ] = XAudioCreateEvent( FALSE, FALSE );
    m_hAudioReadEvent[ 1 ] = XAudioCreateEvent( FALSE, FALSE );
    InitializeCriticalSection( &m_CsRingBuffer ) ;

    // Start up file read handling thread
    m_hReadThread = CreateThread( NULL, 0, FileReadThread, this, 0, NULL );
    SetThreadPriority( m_hReadThread, THREAD_PRIORITY_ABOVE_NORMAL );
    
    m_hPacketThread = CreateThread( NULL, 0, PacketThread, this, 0, NULL );
    SetThreadPriority( m_hPacketThread, THREAD_PRIORITY_ABOVE_NORMAL );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Name: CWavebankStream::Close
// Desc: Close wavebank and release objects
//--------------------------------------------------------------------------------------
VOID    CWavebankStream::Close()
{
    if( m_hAudioReadEvent[ 1 ] )
    {
        XAudioSetEvent( m_hAudioReadEvent[ 1 ] );    // Terminate file read thread
        WaitForSingleObject( m_hReadThread, INFINITE );
    }

    if( m_hAudioPacketEvent[ MAX_STREAMVOICE ] )
    {
        // Terminate file read thread
        XAudioSetEvent( m_hAudioPacketEvent[ MAX_STREAMVOICE ] );
        WaitForSingleObject( m_hPacketThread, INFINITE );
    }

    if( m_pWavebankHeader )
    {
        free( m_pWavebankHeader );
        m_pWavebankHeader = NULL;
    }

    if( m_pWavebankData )
    {
        free( m_pWavebankData );
        m_pWavebankData = NULL;
    }

    if( m_pDataEntry )
    {
        free( m_pDataEntry );
        m_pDataEntry = NULL;
    }

    if( m_pEntryName )
    {
        free( m_pEntryName );
        m_pEntryName = NULL;
    }

    if( m_hFile )
    {
        CBsFileIO::BsCloseFileHandle( m_hFile );
        m_hFile = NULL;
    }

    if( m_hFileBufferred )
    {
        CBsFileIO::BsCloseFileHandle( m_hFileBufferred );
        m_hFileBufferred = NULL;
    }

    if( m_hReadThread )
    {
        CloseHandle( m_hReadThread );
        m_hReadThread = NULL;
    }

    if( m_hPacketThread )
    {
        CloseHandle( m_hPacketThread );
        m_hPacketThread = NULL;
    }

    for( INT i = 0; i < 2; ++i )
    {
        if( m_hAudioReadEvent[ i ] )
        {
            XAudioFreeEvent( m_hAudioReadEvent[ i ] );
            m_hAudioReadEvent[ i ] = NULL;
        }
    }

    for( INT i = 0; i <= MAX_STREAMVOICE; ++i )
    {
        if( m_hAudioPacketEvent[ i ] )
        {
            XAudioFreeEvent( m_hAudioPacketEvent[ i ] );
            m_hAudioPacketEvent[ i ] = NULL;
        }
    }

    DeleteCriticalSection( &m_CsRingBuffer );
}


//--------------------------------------------------------------------------------------
// Name: CWavebankStream::GetEntries
// Desc: Retrieve a number of entries in the wavebank
//--------------------------------------------------------------------------------------
DWORD CWavebankStream::GetEntries( void )
{
    BsAssert ( m_pWavebankData != NULL );
    return m_pWavebankData->dwEntryCount;
}


//--------------------------------------------------------------------------------------
// Name: CWavebankStream::GetFormat
// Desc: Get a wave format
//--------------------------------------------------------------------------------------
HRESULT CWavebankStream::GetFormat( const DWORD dwEntry, XAUDIOSOURCEFORMAT * pFormat )
{
    BsAssert( m_pWavebankData != NULL );
    BsAssert( m_pDataEntry != NULL );
    BsAssert( pFormat != NULL );

    // Check entry #
    if( dwEntry >= m_pWavebankData->dwEntryCount )
        return S_FALSE;

    if( m_pWavebankData->dwFlags & WAVEBANK_FLAGS_COMPACT )
    {
        // Only PCM is supported now
        if( m_pWavebankData->CompactFormat.wFormatTag != WAVEBANKMINIFORMAT_TAG_PCM )
            return S_FALSE;
        if(  m_pWavebankData->CompactFormat.wBitsPerSample ==
             WAVEBANKMINIFORMAT_BITDEPTH_8 )
            pFormat->SampleType = XAUDIOSAMPLETYPE_8BITPCM;
        else if (  m_pWavebankData->CompactFormat.wBitsPerSample ==
                   WAVEBANKMINIFORMAT_BITDEPTH_16 )
            pFormat->SampleType = XAUDIOSAMPLETYPE_16BITPCM;
        else
            return S_FALSE;
        pFormat->SampleRate = m_pWavebankData->CompactFormat.nSamplesPerSec;
        pFormat->ChannelCount = m_pWavebankData->CompactFormat.nChannels;
    }
    else
    {
        // Only PCM is supported now
        if( m_pDataEntry[ dwEntry ].Format.wFormatTag != WAVEBANKMINIFORMAT_TAG_PCM )
            return S_FALSE;

        if(  m_pDataEntry[ dwEntry ].Format.wBitsPerSample ==
             WAVEBANKMINIFORMAT_BITDEPTH_8 )
            pFormat->SampleType = XAUDIOSAMPLETYPE_8BITPCM;
        else if (  m_pDataEntry[ dwEntry ].Format.wBitsPerSample ==
                   WAVEBANKMINIFORMAT_BITDEPTH_16 )
            pFormat->SampleType = XAUDIOSAMPLETYPE_16BITPCM;
        else
            return S_FALSE;

        pFormat->SampleRate = m_pDataEntry[ dwEntry ].Format.nSamplesPerSec;
        pFormat->ChannelCount = m_pDataEntry[ dwEntry ].Format.nChannels;
    }
    return S_OK;
}


//--------------------------------------------------------------------------------------
// Name: CWavebankStream::GetDuration
// Desc: Get a duration of a wave entry
//--------------------------------------------------------------------------------------
HRESULT CWavebankStream::GetDuration( const DWORD dwEntry, DWORD * pDuration )
{
    BsAssert( m_pWavebankData != NULL );
    BsAssert( m_pDataEntry != NULL );
    BsAssert( pDuration != NULL );

    // Check entry #
    if( dwEntry >= m_pWavebankData->dwEntryCount )
        return S_FALSE;

    // Fill out format data
    *pDuration = m_pDataEntry[ dwEntry ].PlayRegion.dwLength;

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Name: CWavebankStream::GetFriendlyName
// Desc: Retrieve entry's friendly name
//--------------------------------------------------------------------------------------
HRESULT CWavebankStream::GetFriendlyName( const DWORD dwEntry, char ** lpFriendlyName )
{
    BsAssert( m_pWavebankHeader != NULL );

    // Check entry #
    if( dwEntry >= m_pWavebankData->dwEntryCount )
        return S_FALSE;

    // No entry name
    if( !m_pEntryName )
        return S_FALSE;

    // Fill out data
    *lpFriendlyName = &m_pEntryName[ dwEntry * WAVEBANK_ENTRYNAME_LENGTH ];
    return S_OK;
}


//--------------------------------------------------------------------------------------
// Name: CWavebankStream::Verify
// Desc: Verify the wave bank header
//--------------------------------------------------------------------------------------
HRESULT CWavebankStream::Verify ()
{
    BsAssert( m_pWavebankHeader != NULL );

    // Verify header
    if (WAVEBANK_HEADER_SIGNATURE != m_pWavebankHeader->dwSignature )
    {
        BsAssert( 0 && "Invalid Wavebank format\n" );
    }
    if (WAVEBANK_HEADER_VERSION < m_pWavebankHeader->dwVersion)
    {
        BsAssert( 0 && "Wave bank version more recent than this tool supports.\n" );
    }

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Name: CWavebankStream::CreateStream
// Desc: Create CStreamVoice instance
//--------------------------------------------------------------------------------------
HRESULT CWavebankStream::CreateStream( const DWORD dwEntry, CStreamVoice ** pStreamVoice )
{
    BsAssert( pStreamVoice != NULL );
    BsAssert( m_pWavebankData != NULL );
    BsAssert( m_hFile != NULL );

    // Check entry #
    if( dwEntry >= m_pWavebankData->dwEntryCount )
        return S_FALSE;                                 // dwEntry exceeds # of entry

    if( m_dwNumStreamVoice >= MAX_STREAMVOICE )         // number of stream exceeds limitation
        return S_FALSE;

    for( int i = 0; i < MAX_STREAMVOICE; ++i )
    {
        // if the class is not initialized, initialize the stream
        if( m_StreamVoice[ i ].GetStatus() == STREAM_UNINITIALIZED )
        {
            m_StreamVoice[ i ].Initialize( this, i, dwEntry );
            PrimeSound( i ) ;
            m_dwNumStreamVoice ++;
            *pStreamVoice = &m_StreamVoice[ i ];
            break;
        }
        BsAssert( 0 && "No stream is avalable." );
    }
    return S_OK;
}


//--------------------------------------------------------------------------------------
// Name: CWavebankStream::ResetStream
// Desc: Reset stream instance
//--------------------------------------------------------------------------------------
HRESULT CWavebankStream::ResetStream( const INT iIndex, const DWORD dwEntry )
{
    BsAssert( m_pWavebankData != NULL );
    BsAssert( m_hFile != NULL );

    // Check entry #
    if( dwEntry >= m_pWavebankData->dwEntryCount )
        return S_FALSE;                             // dwEntry exceeds # of entry

    if( iIndex >= MAX_STREAMVOICE )                 // number of stream exceeds limitation
        return S_FALSE;
        
    if( m_StreamVoice[ iIndex ].GetStatus() == STREAM_STOP )
        // Reset can be done only in STOP status
        m_StreamVoice[ iIndex ].Initialize( this, iIndex, dwEntry );
        PrimeSound( iIndex ) ;
    return S_OK;
}


//--------------------------------------------------------------------------------------
// Name: CWavebankStream::PrimeSound
// Desc: Create a buffer and prime stream data
//--------------------------------------------------------------------------------------
HRESULT CWavebankStream::PrimeSound( INT iStreamIndex )
{
    BsAssert( m_pWavebankData != NULL );
    BsAssert( m_hFile != NULL );

    SetFilePointer( m_hFile, GetFilePointer( iStreamIndex, 0 ), 0, FILE_BEGIN );

    DWORD dwBytesRead;
    CBsFileIO::BsReadFile( m_hFile, m_pReadBuffer[ iStreamIndex ], READBUFFER_SIZE, &dwBytesRead, NULL );

    if( dwBytesRead != READBUFFER_SIZE )
        BsAssert( 0 && "Error opening Wavebank file\n" );

    // Initialize Read/Write pointer of ring buffer
    EnterCriticalSection( &m_CsRingBuffer );

    m_dwPlayPointer[ iStreamIndex ] = 0;
    m_dwPlayedPointer[ iStreamIndex ] = 0;
    m_dwWritePointer[ iStreamIndex ] = dwBytesRead;
 
    READREQUESTLIST::iterator it;
    for( it = m_PlayRequestCue.begin(); it != m_PlayRequestCue.end(); ++it )
    {
        if( *it == iStreamIndex )
        {
            m_PlayRequestCue.erase( it );
            break;
        }
    }

    LeaveCriticalSection( &m_CsRingBuffer );
    return S_OK;
}

//--------------------------------------------------------------------------------------
// Name: CWavebankStream::GetFilePointer
// Desc: Get file pointer position of streaming wavebank including lapping around
//--------------------------------------------------------------------------------------
DWORD CWavebankStream::GetFilePointer( INT iStreamIndex,
                                       INT iOffset, DWORD * pdwMaxReadSize )
{
    BsAssert( m_pWavebankHeader != NULL );
    BsAssert( iOffset >= 0 );
    // Check source data loop region
    DWORD dwEntry = m_StreamVoice[ iStreamIndex ].GetWavebankEntry();
    DWORD dwLoopLength = m_pDataEntry[ dwEntry ].PlayRegion.dwLength;
    DWORD dwFileReadPos = iOffset % dwLoopLength;

    // Loop check ?

    // Return maximum possible read size
    if( pdwMaxReadSize ) *pdwMaxReadSize = dwLoopLength - dwFileReadPos;

    DWORD dwDataOffset = m_pWavebankHeader->Segments[ WAVEBANK_SEGIDX_ENTRYWAVEDATA ].dwOffset;
    return dwDataOffset + m_pDataEntry[ dwEntry ].PlayRegion.dwOffset + dwFileReadPos;
}


//--------------------------------------------------------------------------------------
// Name: CWavebankStream::GetAvailablePacket
// Desc: Retrieve a pointer to available packet data 
//--------------------------------------------------------------------------------------
HRESULT CWavebankStream::GetAvailablePacket( INT iStreamIndex,
                                             DWORD *pPacketSize, VOID **ppData )
{
    // Now the class handles as a packet size of all streams are same.

    EnterCriticalSection( &m_CsRingBuffer );
    DWORD dwAvailableData = m_dwWritePointer[ iStreamIndex ] - m_dwPlayPointer[ iStreamIndex ];
    if( dwAvailableData <= 0 )
    {
        LeaveCriticalSection( &m_CsRingBuffer );
        *pPacketSize = 0;
        return S_OK;
    }

    DWORD dwPlayOffset = m_dwPlayPointer[ iStreamIndex ] % READBUFFER_SIZE;

    // Check buffer lap around
    DWORD dwSize;
    if( dwPlayOffset + dwAvailableData > READBUFFER_SIZE )
    {
        dwSize = READBUFFER_SIZE - dwPlayOffset;
    }
    else
        dwSize = dwAvailableData;;

    if( dwSize > STREAMPACKETSIZE )
        dwSize = STREAMPACKETSIZE;

    *ppData = m_pReadBuffer[ iStreamIndex ] + dwPlayOffset;
    *pPacketSize = dwSize;
    m_dwPlayPointer[ iStreamIndex ] += dwSize;

    // Add the packet size in the cue
    m_PacketSizeCue[ iStreamIndex ].push_back( dwSize );

    LeaveCriticalSection( &m_CsRingBuffer );
    
    return S_OK;
}


//--------------------------------------------------------------------------------------
// Name: CWavebankStream::GetEmptyPacket
// Desc: Fill out the buffer with zero and return pointer
//       This is called when packet shortage happens
//--------------------------------------------------------------------------------------
HRESULT CWavebankStream::GetEmptyPacket( INT iStreamIndex, DWORD *pPacketSize, VOID **ppData )
{
    EnterCriticalSection( &m_CsRingBuffer );
    DWORD dwAvailableData = m_dwWritePointer[ iStreamIndex ] - m_dwPlayPointer[ iStreamIndex ];
    if( dwAvailableData <= 0 )
    {
        DWORD dwPlayOffset = m_dwPlayPointer[ iStreamIndex ] % READBUFFER_SIZE;

        // Check buffer lap around
        DWORD dwSize;
        if( dwPlayOffset + EMTPY_PACKETSIZE > READBUFFER_SIZE )
        {
            dwSize = READBUFFER_SIZE - dwPlayOffset;
        }
        else
            dwSize = EMTPY_PACKETSIZE;;

        if( dwSize > STREAMPACKETSIZE )
            dwSize = STREAMPACKETSIZE;

        m_dwPlayPointer[ iStreamIndex ] += EMTPY_PACKETSIZE;
        m_dwWritePointer[ iStreamIndex ] = m_dwPlayPointer[ iStreamIndex ];
        *ppData = m_pReadBuffer[ iStreamIndex ] + dwPlayOffset;
        *pPacketSize = dwSize;

        // Add the packet size in the cue
        m_PacketSizeCue[ iStreamIndex ].push_back( dwSize );

        LeaveCriticalSection( &m_CsRingBuffer );

        ZeroMemory( m_pReadBuffer[ iStreamIndex ] + dwPlayOffset, dwSize );
        return S_OK;
    }

    // If there is a packet date, retrieve it
    LeaveCriticalSection( &m_CsRingBuffer );
    return GetAvailablePacket( iStreamIndex, pPacketSize, ppData );
}


//--------------------------------------------------------------------------------------
// Name: CWavebankStream::FreePacketData
// Desc: Free out packet data that has been played
//--------------------------------------------------------------------------------------
VOID CWavebankStream::FreePacketData( INT iStreamIndex, DWORD dwSize )
{
    // Note: Read threshold is better to be adjusted by bitrate of the each stream
    EnterCriticalSection( &m_CsRingBuffer );

    m_dwPlayedPointer[ iStreamIndex ] += dwSize;

    if( m_dwWritePointer[ iStreamIndex ] - m_dwPlayPointer[ iStreamIndex ] < READTHRESHOLD )
    {
        READREQUESTLIST::iterator it;
        for( it = m_PlayRequestCue.begin(); it != m_PlayRequestCue.end(); ++it )
        {
            if( *it == iStreamIndex )
                break;      // Read request is already in the cue
        }

        if( it == m_PlayRequestCue.end() )
        {
            // Add a read request in the cue
            m_PlayRequestCue.push_back( iStreamIndex );
        }
        XAudioSetEvent( m_hAudioReadEvent[ 0 ] );
    }

    LeaveCriticalSection( &m_CsRingBuffer );
}
    

//--------------------------------------------------------------------------------------
// Name: CWavebankStream::Packet handling thread
// Desc: Thread to manage free packet data
//--------------------------------------------------------------------------------------
DWORD WINAPI CWavebankStream::PacketThread( LPVOID lpParameter )
{
    CWavebankStream * pStream = (CWavebankStream *) lpParameter;

    for( ;; )
    {
        INT iEventIndex = XAudioWaitForMultipleObjects( MAX_STREAMVOICE + 1,
                                                        pStream->m_hAudioPacketEvent,
                                                        FALSE,
                                                        INFINITE,
                                                        FALSE ) - WAIT_OBJECT_0;
        if ( iEventIndex == MAX_STREAMVOICE )
            break;      // Terminate thread

        CStreamVoice* pStreamVoice = (CStreamVoice *)&pStream->m_StreamVoice[ iEventIndex ];

        // Pop event from the cue
        DWORD dwPacketSize = pStream->m_PacketSizeCue[ iEventIndex ].front();
        pStream->m_PacketSizeCue[ iEventIndex ].erase(
                                      pStream->m_PacketSizeCue[ iEventIndex ].begin() );

        // Free up played packet region
        pStream->FreePacketData( iEventIndex, (DWORD)dwPacketSize ) ;
        
        XAUDIOPACKET Packet = { 0 };
        // Find out how much sample is available now
        pStream->GetAvailablePacket( iEventIndex, &dwPacketSize, &Packet.pBuffer );
              
        if( dwPacketSize )
        {
            Packet.BufferSize = dwPacketSize;
            Packet.pContext = (VOID *) dwPacketSize;
            // Submit new packet
            pStreamVoice->GetSourceVoice()->SubmitPacket( &Packet, 0 );
        }
        else
        {
            // Sound disturbed, retry later or when read from the media is done.
            DebugString( "Sound disturbed\n" );
            // Do something to retry
            // Now retrieve empty data and play it.
            pStream->GetEmptyPacket( iEventIndex, &dwPacketSize, &Packet.pBuffer );
            Packet.BufferSize = dwPacketSize;
            Packet.pContext = (VOID *) dwPacketSize;
            // Submit new packet
            pStreamVoice->GetSourceVoice()->SubmitPacket( &Packet, 0 );
        }

    }
    return 0;
}


//--------------------------------------------------------------------------------------
// Name: CWavebankStream::FileIOThread
// Desc: Thread to handle all file read request
//--------------------------------------------------------------------------------------
DWORD WINAPI CWavebankStream::FileReadThread( LPVOID lpParameter )
{
    CWavebankStream * pStream = (CWavebankStream *) lpParameter;

    for( ;; )
    {
        INT iEventIndex = XAudioWaitForMultipleObjects( 2,
                                                        pStream->m_hAudioReadEvent,
                                                        FALSE,
                                                        INFINITE,
                                                        FALSE ) - WAIT_OBJECT_0;
        if ( iEventIndex == 1 )
            break;      // Terminate thread

        // Note: read requests can be managed better.
        // i.e. Sorting by sector address, request frequency change depending on bitrate etc
        // For now, just submitting asynchronous ReadFile () here.
        
        EnterCriticalSection( &pStream->m_CsRingBuffer );

        INT iStreamIndex = pStream->m_PlayRequestCue.front();    // Pop event from the cue

        DWORD dwWriteOffset = pStream->m_dwWritePointer[ iStreamIndex ] % READBUFFER_SIZE;
        DWORD dwSize = pStream->m_dwPlayedPointer[ iStreamIndex ]
                    - pStream->m_dwWritePointer[ iStreamIndex ] + READBUFFER_SIZE - SECTOR_SIZE;
        // Check buffer lap around
        if( dwWriteOffset + dwSize > READBUFFER_SIZE )
            dwSize = READBUFFER_SIZE - dwWriteOffset;
        
        DWORD dwMaxSize;
        OVERLAPPED ov = { 0 };
        ov.Offset = pStream->GetFilePointer( iStreamIndex,
                                            pStream->m_dwWritePointer[ iStreamIndex ],
                                            &dwMaxSize );

        if( dwSize > dwMaxSize )
            dwSize = dwMaxSize;
        if( dwSize > MAXREADREQUESTSIZE )
            dwSize = MAXREADREQUESTSIZE;
        
        BYTE* pReadBuffer = pStream->m_pReadBuffer[ iStreamIndex ] + dwWriteOffset;

        LeaveCriticalSection( &pStream->m_CsRingBuffer );
        
        LARGE_INTEGER liReadStart, liReadEnd;
        DWORD dwSizeRead;
   
        QueryPerformanceCounter( &liReadStart );

        // make sure request addresses are sector boundary?
        if( dwSize & ( SECTOR_SIZE - 1 ) | ov.Offset & ( SECTOR_SIZE - 1 ) )
        {
            // Fallback to bufferred access
			if( FALSE == CBsFileIO::BsReadFile( pStream->m_hFileBufferred,
                                pReadBuffer,
                                dwSize, &dwSizeRead,
                                &ov ) )
            {
                BsAssert( 0 && "Failed to read a file.\nThe title must handle the read error situation." );
                // Read error
                // Note: Do something
            }
        }
        else
        {
            dwSize = ( dwSize + ( SECTOR_SIZE - 1 ) )  & ~( SECTOR_SIZE - 1 );
            ov.Offset = ov.Offset & ~( SECTOR_SIZE - 1 );

            if( FALSE == CBsFileIO::BsReadFile( pStream->m_hFile,
                                pReadBuffer,
                                dwSize, &dwSizeRead,
                                &ov ) )
            {
                BsAssert( 0 && "Failed to read a file. The title must handle the read error situation." );
                // Read error
                // Note: Do something
            }
        }
        QueryPerformanceCounter( &liReadEnd );

        if( dwSize )
        {
            // Update write pointer
            EnterCriticalSection( &pStream->m_CsRingBuffer );
            pStream->m_dwWritePointer[ iStreamIndex ] += dwSize;

            // Overflow check
            if( pStream->m_dwPlayedPointer[ iStreamIndex ] > OVERFLOWTHRESHOLD )
            {
                pStream->m_dwPlayPointer[ iStreamIndex ]   -= OVERFLOWTHRESHOLD;
                pStream->m_dwPlayedPointer[ iStreamIndex ] -= OVERFLOWTHRESHOLD;
                pStream->m_dwWritePointer[ iStreamIndex ]  -= OVERFLOWTHRESHOLD;
            }
            // Remove read request from the cue
            pStream->m_PlayRequestCue.erase( pStream->m_PlayRequestCue.begin() );

            // If no request in the cue,
            if( !pStream->m_PlayRequestCue.size() )
                XAudioResetEvent( pStream->m_hAudioReadEvent[ 0 ] );

            LARGE_INTEGER liFrequency;
            QueryPerformanceFrequency( &liFrequency );

            FLOAT fReadTime = FLOAT( liReadEnd.QuadPart - liReadStart.QuadPart )
                                        / FLOAT ( liFrequency.QuadPart );

            if( pStream->m_fMaxReadTime < fReadTime )
            {
                pStream->m_fMaxReadTime = fReadTime;
                pStream->m_dwMaxReadSize = dwSize;
            }
            LeaveCriticalSection( &pStream->m_CsRingBuffer );
        }
    }
    return 0;
}


//--------------------------------------------------------------------------------------
// Streamer class
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Name: CStreamVoice::ctor
// Desc: construct CStreamVoice instance
//--------------------------------------------------------------------------------------
CStreamVoice::CStreamVoice ()
{
    m_pSourceVoice = NULL;
    m_pWavebank = NULL;
    m_dwStatus = STREAM_UNINITIALIZED;
}


//--------------------------------------------------------------------------------------
// Name: CStreamVoice::dtor
// Desc: destruct CStreamVoice instance
//--------------------------------------------------------------------------------------
CStreamVoice::~CStreamVoice ()
{
    if( m_pSourceVoice )
    {
        m_pSourceVoice->Release();
        m_pSourceVoice = NULL;
    }
}


//--------------------------------------------------------------------------------------
// Name: CStreamVoice::Initialize
// Desc: Initialize CStreamVoice instance
//--------------------------------------------------------------------------------------
HRESULT CStreamVoice::Initialize ( CWavebankStream * pWavebank, const INT iStreamIndex,
                                   const DWORD dwWavebankEntry )
{
    BsAssert( pWavebank != NULL );

    HRESULT hr = S_OK;
    if( m_pSourceVoice )    // Already in use?
        m_pSourceVoice->Release();

    XAUDIOSOURCEVOICEINIT SourceVoiceInit = { 0 };
    if( FAILED( hr = pWavebank->GetFormat( dwWavebankEntry, &SourceVoiceInit.Format ) ) )
        BsAssert2( 0 && "Failed to get format %#X\n", hr );

    SourceVoiceInit.MaxPacketCount = MAX_PACKETCOUNT;
    SourceVoiceInit.pfnPacketCompletionCallback = PacketCallback; 
    SourceVoiceInit.pContext = this;

    if ( FAILED( hr = XAudioCreateSourceVoice( &SourceVoiceInit, &m_pSourceVoice ) ) )
        BsAssert2( 0 && "Error %#X calling XAudioCreateSourceVoice\n", hr );

    m_pWavebank = pWavebank;
    m_iStreamIndex = iStreamIndex;
    m_dwWavebankEntry = dwWavebankEntry;
    m_dwStatus = STREAM_STOP;

    return hr;
}


//--------------------------------------------------------------------------------------
// Name: CStreamVoice::PacketCallback
// Desc: Packet callback routine
//--------------------------------------------------------------------------------------
void CStreamVoice::PacketCallback( LPCXAUDIOVOICEPACKETCALLBACK pVoicePacket )
{

    // Get a pointer to the sample
    CStreamVoice* pStreamVoice = (CStreamVoice *)pVoicePacket->pVoiceContext;

    if( pStreamVoice->m_dwStatus != STREAM_PLAY )
        return;

    XAudioSetEvent( pStreamVoice->m_pWavebank->m_hAudioPacketEvent[ pStreamVoice->m_iStreamIndex ] );
    
}


//--------------------------------------------------------------------------------------
// Name: CStreamVoice::Start
// Desc: Start streaming voice
//--------------------------------------------------------------------------------------
HRESULT CStreamVoice::Start()
{
    BsAssert( m_pWavebank != NULL );

    HRESULT hr = S_OK;

    if( m_dwStatus != STREAM_STOP )
        return S_FALSE;

    // Set up packet
    XAUDIOPACKET Packet = { 0 };

    // Find out how big the sample is
    DWORD dwPacketSize;
    for( INT i = 0; i < MAX_PACKETCOUNT; ++i )
    {
        m_pWavebank->GetAvailablePacket( m_iStreamIndex, &dwPacketSize, &Packet.pBuffer );
        if( !dwPacketSize )
        {
            // No samples in a buffer
            return S_FALSE;
        }
        Packet.BufferSize = dwPacketSize;
        Packet.pContext = (VOID *) dwPacketSize;

        // Submit packet
        if ( FAILED( hr = m_pSourceVoice->SubmitPacket(
                        &Packet, 0 ) ) )
            BsAssert2( 0 && "Error %#X calling SubmitPacket\n", hr );
    }
    // Play the source voice
    if ( FAILED( hr = m_pSourceVoice->Start( 0 ) ) )
        BsAssert2( 0 && "Error %#X calling Start\n", hr );

    m_dwStatus = STREAM_PLAY;

    return hr;
}


//--------------------------------------------------------------------------------------
// Name: CStreamVoice::Stop
// Desc: Stop streaming voice
//--------------------------------------------------------------------------------------
HRESULT CStreamVoice::Stop()
{
    BsAssert( m_pWavebank != NULL );
    HRESULT hr = S_OK;

    if( m_dwStatus != STREAM_PLAY )
        return S_FALSE;

    m_dwStatus = STREAM_STOP;
    m_pSourceVoice->FlushPackets();
    // Play the source voice
    if ( FAILED( hr = m_pSourceVoice->Stop( 0 ) ) )
        BsAssert2( 0 && "Error %#X calling Stop\n", hr );
    m_pWavebank->m_PacketSizeCue[ m_iStreamIndex ].empty();

    m_pWavebank->PrimeSound( m_iStreamIndex );
    return hr;
}


//--------------------------------------------------------------------------------------
// Name: CStreamVoice::Pause
// Desc: Stop streaming voice
//--------------------------------------------------------------------------------------
HRESULT CStreamVoice::Pause()
{
    BsAssert( m_pWavebank != NULL );
    HRESULT hr = S_OK;

    if( m_dwStatus != STREAM_PLAY )
        return S_FALSE;

    m_dwStatus = STREAM_PAUSE;
    m_pSourceVoice->FlushPackets();
    // Play the source voice
    if ( FAILED( hr = m_pSourceVoice->Stop( 0 ) ) )
        BsAssert2( 0 && "Error %#X calling Stop\n", hr );
    return hr;
}


//--------------------------------------------------------------------------------------
// Name: CStreamVoice::Resume
// Desc: Stop streaming voice
//--------------------------------------------------------------------------------------
HRESULT CStreamVoice::Resume()
{
    BsAssert( m_pWavebank != NULL );
    HRESULT hr = S_OK;

    if( m_dwStatus != STREAM_PAUSE )
        return S_FALSE;

    m_dwStatus = STREAM_STOP;
    Start();
        
    return hr;
}


//--------------------------------------------------------------------------------------
// Name: CStreamVoice::GetStatus
// Desc: Retrieve stream status
//       This function is only for retrieving info for sample UI
//--------------------------------------------------------------------------------------
void CStreamVoice::GetStats( STREAM_STATS* pStats)
{
    pStats->dwStatus = m_dwStatus;
    if( m_dwStatus == STREAM_UNINITIALIZED )
    {
        return;
    }

    EnterCriticalSection( &m_pWavebank->m_CsRingBuffer );
    pStats->dwLength = m_pWavebank->m_pDataEntry[ m_dwWavebankEntry ].PlayRegion.dwLength;
    pStats->dwPlayPointer = m_pWavebank->m_dwPlayPointer[ m_iStreamIndex ];
    pStats->dwWritePointer = m_pWavebank->m_dwWritePointer[ m_iStreamIndex ];
    pStats->dwPlayedPointer = m_pWavebank->m_dwPlayedPointer[ m_iStreamIndex ];
    pStats->fMaxReadTime = m_pWavebank->m_fMaxReadTime;
    pStats->dwMaxReadSize = m_pWavebank->m_dwMaxReadSize;
    LeaveCriticalSection( &m_pWavebank->m_CsRingBuffer );

}
#endif