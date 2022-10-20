//--------------------------------------------------------------------------------------
// CWavebankStream.h
//
// Class for handling wavebank
//
// Xbox Advanced Technology Group.
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#pragma once
#ifdef _XBOX

#include <XAudio.h>
#include <XAct2wb.h>
#include <vector>

//--------------------------------------------------------------------------------------
// definitions
//--------------------------------------------------------------------------------------
const DWORD MAX_STREAMVOICE = 8;            // max # of streams
const DWORD MAX_PACKETCOUNT = 8;            // Packet count for XAudio source voice
const DWORD SECTOR_SIZE = 2048;             // Sector size: 512 in HDD, 2048 in DVD
const DWORD STREAMRATEPERSEC = 44100 * 2;   // Bandwidth of stream (should be changed per stream)
const DWORD STREAMBUFFERINGSIZE = MAX_STREAMVOICE * 1;          // Length of stream buffer (in seconds)
const DWORD READBUFFER_SIZE = ( ( STREAMRATEPERSEC * STREAMBUFFERINGSIZE + (SECTOR_SIZE - 1) )
                              & ~(SECTOR_SIZE - 1 ) );
const DWORD STREAMPACKETSIZE = SECTOR_SIZE;                     // Packet size of a voice packet
const DWORD MAXREADREQUESTSIZE = MAX_STREAMVOICE * 100 * 1024;  // Max read request size
const DWORD READTHRESHOLD = READBUFFER_SIZE - 64 * 1024;        // threshold for read request

const DWORD OVERFLOWTHRESHOLD = 0x70000000; // Counter overflow adjustment threshold

const DWORD EMTPY_PACKETSIZE = 2048;        // Size of zero filled packet that is played
                                            // in a packet shortage situation
class CWavebankStream;

typedef std::vector< INT > READREQUESTLIST;
typedef std::vector< DWORD > PACKETSIZELIST;

//--------------------------------------------------------------------------------------
// Class & Structure
//--------------------------------------------------------------------------------------
struct STREAM_STATS {
    DWORD dwStatus;
    DWORD dwLength;
    DWORD dwPlayPointer;
    DWORD dwWritePointer;
    DWORD dwPlayedPointer;
    FLOAT fMaxReadTime;
    DWORD dwMaxReadSize;
};

enum STREAMSTATUS {
    STREAM_UNINITIALIZED,
    STREAM_PLAY,
    STREAM_STOP,
    STREAM_PAUSE,
};

//--------------------------------------------------------------------------------------
// Name: CStreamVoice
// Desc: Wrapper class of Source voice, manage source voice callbacks
//--------------------------------------------------------------------------------------
class CStreamVoice
{
public:
    CStreamVoice ();
    ~CStreamVoice ();
    HRESULT Initialize ( CWavebankStream * pWavebank, const INT iStreamIndex,
                         const DWORD dwWavebankEntry );
    HRESULT Start();
    HRESULT Stop();
    HRESULT Pause();
    HRESULT Resume();
    IXAudioSourceVoice* GetSourceVoice() { return m_pSourceVoice; };
    void  GetStats( STREAM_STATS* pStats );
    DWORD GetWavebankEntry()    { return m_dwWavebankEntry; };
    DWORD GetStatus()           { return m_dwStatus; };
private:
    DWORD               m_dwStatus;
    IXAudioSourceVoice* m_pSourceVoice;     // Source voice
    CWavebankStream *   m_pWavebank;        // Pointer to wavebank
    INT                 m_iStreamIndex;     // My stream index
    DWORD               m_dwWavebankEntry;  // My wavebank entry
    
    static void PacketCallback( LPCXAUDIOVOICEPACKETCALLBACK pCallbackData );
};


//--------------------------------------------------------------------------------------
// Name: CWavebankStream
// Desc: Class to handle the Wavebank file
//--------------------------------------------------------------------------------------
class CWavebankStream
{
public:
    CWavebankStream () ;
    ~CWavebankStream () ;

    // Initialization
    // Open a wavebank. The function allocates buffer for a wavebank
    HRESULT Open( const CHAR* strFileName ); 
    // Close the wavebank
    VOID    Close();
    // Get # of wave entries in the wavebank
    DWORD GetEntries( void ) ;
    // Get a wave format of the wave entry
    HRESULT GetFormat( const DWORD dwEntry, XAUDIOSOURCEFORMAT * pFormat );
    HRESULT GetFriendlyName( const DWORD dwEntry, char ** lpFriendlyName );
    // Get a duration of the wave entry
    HRESULT GetDuration( const DWORD dwEntry, DWORD * pDuration );
    HRESULT CreateStream( const DWORD dwEntry, CStreamVoice ** pStreamVoice ) ;
    HRESULT ResetStream( const INT iIndex, const DWORD dwEntry );
    
private:
    HRESULT Verify ();
    HRESULT PrimeSound( INT iStreamIndex );
    DWORD   GetFilePointer( INT iStreamIndex, INT iOffset, DWORD * pdwMaxReadSize = NULL );
    VOID    FreePacketData( INT iStreamIndex, DWORD dwSize );
    HRESULT GetAvailablePacket( INT iStreamIndex, DWORD *pPacketSize, VOID **ppData );
    HRESULT GetEmptyPacket( INT iStreamIndex, DWORD *pPacketSize, VOID **ppData );

    static DWORD WINAPI FileReadThread( LPVOID lpParameter );
    static DWORD WINAPI PacketThread( LPVOID lpParameter );

    HANDLE  m_hReadThread;
    HANDLE  m_hPacketThread;
    HANDLE  m_hFile;              // File handle for wavebank SYNC read
    HANDLE  m_hFileBufferred;     // File handle for wavebank SYNC read

    DWORD           m_dwNumStreamVoice;
    CStreamVoice    m_StreamVoice[ MAX_STREAMVOICE ];
    BYTE *          m_pReadBuffer[ MAX_STREAMVOICE ];
    DWORD           m_dwPlayPointer[ MAX_STREAMVOICE ];
    DWORD           m_dwWritePointer[ MAX_STREAMVOICE ];
    DWORD           m_dwPlayedPointer[ MAX_STREAMVOICE ];
    PACKETSIZELIST  m_PacketSizeCue[ MAX_STREAMVOICE ] ;
    
    XAUDIOEVENT     m_hAudioPacketEvent[ MAX_STREAMVOICE + 1 ];
    XAUDIOEVENT     m_hAudioReadEvent[ 2 ];

    READREQUESTLIST m_PlayRequestCue;       //PlayRequestCue
    CRITICAL_SECTION m_CsRingBuffer;

    DWORD               m_dwFileSize;
    WAVEBANKHEADER *    m_pWavebankHeader;
    WAVEBANKDATA *      m_pWavebankData;
    WAVEBANKENTRY *     m_pDataEntry;
    char *              m_pEntryName;
    
    //  Streaming stats
    FLOAT               m_fMaxReadTime;
    DWORD               m_dwMaxReadSize;
  
    friend CStreamVoice;    // Let CStreamVoice invoke private functions
};
#endif

