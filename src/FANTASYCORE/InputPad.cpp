#include "stdafx.h"
#include "BsKernel.h"
#include "BStreamExt.h"
#include "FcBaseObject.h"
#include "InputPad.h"
#include "FcGlobal.h"
#include "FcUtil.h"
#include "FantasyCore.h"

#include "MGSToolbox\InputHook.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


static int g_nKeyCode[ MAX_KEY_CODE_COUNT ] = 
{
	0x1000, 0x2000, 0x4000, 0x8000, 0x0100, 0x0200, 0x00000001, 0x00000002, 0x00000004, 0x00000008, 
	0x00000010, 0x00000020, 0x00000040, 0x00000080, 10, 11, -1, -1
};

CInputPad::CInputPad()
{
	m_nInputMode = 0;
	m_pStream = NULL;
	m_nInputSaveCount = 0;
	m_pStream = NULL;

	m_nPort = -1;
	m_nLastKeyPressPort = -1;

	m_bVibrationOn = true;

	ResetKeyBuffer();
}

CInputPad::~CInputPad()
{
	if( m_pStream )
	{
		delete m_pStream;
	}
}


//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------

// Deadzone for thumbsticks
#define XBINPUT_DEADZONE 0.25

// Threshold for analog buttons
#define XBINPUT_BUTTONTHRESHOLD 1

int CInputPad::Create( int nPort )
{
	m_nPort = nPort;

	return 1;
}

int CInputPad::Process()
{
	int nRet;

	if( m_nInputMode == 0 )
	{
		nRet = ProcessNormal();
	}
	else
	{
		nRet = ProcessMacro();
	}

	ProcessVibration();

	return nRet;
}

int CInputPad::ProcessMacro()
{
	int i, j, nTick;
	INPUT_SAVE InputSave;

	nTick = GetProcessTick();
	for( i = 0; i < MAX_PC_PAD_COUNT; i++ )
	{
		for( j = 0; j < MAX_KEY_CODE_COUNT; j++ )
		{
			if( m_KeyPressTick[ i ][ j ] )
			{
				m_KeyPressTick[ i ][ j ]++;
			}
		}
	}

	while( m_InputSaveList[ m_nCurrentInput ].nTick <= nTick )
	{
		if( m_InputSaveList[ m_nCurrentInput ].nTick == nTick )
		{
			InputSave = m_InputSaveList[ m_nCurrentInput ];
			m_AttachHandle[ InputSave.nPort ]->SetCrossVector( &InputSave.Cross );
			if( InputSave.nKeyCode & KEY_RELEASE_FLAG )
			{
				m_KeyPressTick[ InputSave.nPort ][ InputSave.nKeyCode ] = 0;
				SendKeyEvent( InputSave.nPort, InputSave.nKeyCode, &InputSave.KeyParam );
				if( ( InputSave.nKeyCode & 0x7fffffff ) >= PAD_INPUT_LSTICK )
				{
					m_KeyParam[ InputSave.nPort ][ ( InputSave.nKeyCode & 0x7fffffff ) - PAD_INPUT_LSTICK ].nPosX = 0;
					m_KeyParam[ InputSave.nPort ][ ( InputSave.nKeyCode & 0x7fffffff ) - PAD_INPUT_LSTICK ].nPosY = 0;
				}
			}
			else
			{
				if( !m_KeyPressTick[ InputSave.nPort ][ InputSave.nKeyCode ] )
				{
					m_KeyPressTick[ InputSave.nPort ][ InputSave.nKeyCode ] = 1;
				}
				SendKeyEvent( InputSave.nPort, InputSave.nKeyCode, &InputSave.KeyParam );
				if( InputSave.nKeyCode >= PAD_INPUT_LSTICK )
				{
					m_KeyParam[ InputSave.nPort ][ InputSave.nKeyCode - PAD_INPUT_LSTICK ] = InputSave.KeyParam;
				}
			}

		}
		m_nCurrentInput++;
		if( m_nCurrentInput >= ( int )m_InputSaveList.size() )
		{
			m_nInputMode = 0;
			break;
		}
	}

	return 1;
}

int CInputPad::ProcessNormal()
{
	int i, nPort;
	short int sAnalogBuf[4];
//    DWORD dwInsertions, dwRemovals;
	XINPUT_STATE xiState;
	KEY_EVENT_PARAM KeyParam;
	

	for( nPort = 0; nPort < MAX_XBOX_PAD_COUNT; nPort++ )
	{
		memset( &xiState, 0, sizeof( XINPUT_STATE ) );

		m_bPadRemoved[ nPort ] = true;
		DWORD result = MGSToolbox::InputHook::GetState( nPort, &xiState );
		if( result == ERROR_SUCCESS )
		{	
			m_bPadRemoved[ nPort ] = false;
		}
		m_GamePad[ nPort ] = xiState.Gamepad;

		memset( &KeyParam, 0, sizeof( KEY_EVENT_PARAM ) );

		for( i = 0; i < DIGITAL_KEY_COUNT; i++ )
		{
			if( m_GamePad[ nPort ].wButtons & g_nKeyCode[ i ] )
			{
				m_KeyPressTick[ nPort ][ i ]++;
				m_KeyPressScale[ nPort ][ i ] = 1;
				if( m_KeyPressTick[ nPort ][ i ] == 1 )
				{
					SendKeyEvent( nPort, i, &KeyParam );
				}
			}
			else
			{
				if( m_KeyPressTick[ nPort ][ i ] )
				{
					SendKeyEvent( nPort, KEY_RELEASE_FLAG+i, &KeyParam );
				}
				m_KeyPressTick[ nPort ][ i ] = 0;
				m_KeyPressScale[ nPort ][ i ] = 0;
			}
		}
		// 트리거 버튼 처리..
		if( m_GamePad[ nPort ].bLeftTrigger )
		{
			m_KeyPressTick[ nPort ][ DIGITAL_KEY_COUNT ]++;
			m_KeyPressScale[ nPort ][ DIGITAL_KEY_COUNT ] = m_GamePad[ nPort ].bLeftTrigger;
			if( m_KeyPressTick[ nPort ][ DIGITAL_KEY_COUNT ] == 1 )
			{
				SendKeyEvent( nPort, DIGITAL_KEY_COUNT, &KeyParam );
			}
		}
		else
		{
			if( m_KeyPressTick[ nPort ][ DIGITAL_KEY_COUNT ] )
			{
				SendKeyEvent( nPort, KEY_RELEASE_FLAG + DIGITAL_KEY_COUNT, &KeyParam );
			}
			m_KeyPressTick[ nPort ][ DIGITAL_KEY_COUNT ] = 0;
			m_KeyPressScale[ nPort ][ DIGITAL_KEY_COUNT ] = 0;
		}
		if( m_GamePad[ nPort ].bRightTrigger )
		{
			m_KeyPressTick[ nPort ][ DIGITAL_KEY_COUNT + 1 ]++;
			m_KeyPressScale[ nPort ][ DIGITAL_KEY_COUNT + 1 ] = m_GamePad[ nPort ].bRightTrigger;
			if( m_KeyPressTick[ nPort ][ DIGITAL_KEY_COUNT + 1 ] == 1 )
			{
				SendKeyEvent( nPort, DIGITAL_KEY_COUNT + 1, &KeyParam );
			}
		}
		else
		{
			if( m_KeyPressTick[ nPort ][ DIGITAL_KEY_COUNT + 1 ] )
			{
				SendKeyEvent( nPort, KEY_RELEASE_FLAG + DIGITAL_KEY_COUNT + 1, &KeyParam );
			}
			m_KeyPressTick[ nPort ][ DIGITAL_KEY_COUNT + 1 ] = 0;
			m_KeyPressScale[ nPort ][ DIGITAL_KEY_COUNT + 1 ] = 0;
		}

		sAnalogBuf[ 0 ] = m_GamePad[ nPort ].sThumbLX;
		sAnalogBuf[ 1 ] = m_GamePad[ nPort ].sThumbLY;
		sAnalogBuf[ 2 ] = m_GamePad[ nPort ].sThumbRX;
		sAnalogBuf[ 3 ] = m_GamePad[ nPort ].sThumbRY;
		for( i = 0; i < 2; i++ )
		{
			if( ( abs( sAnalogBuf[ i * 2 ] ) > ANALOG_THRESH_HOLD ) || ( abs( sAnalogBuf[ i * 2 + 1 ] ) > ANALOG_THRESH_HOLD ) )
			{
				KeyParam.nPosX = sAnalogBuf[ i * 2 ];
				KeyParam.nPosY = sAnalogBuf[ i * 2 + 1 ];
				m_KeyPressTick[ nPort ][ DIGITAL_KEY_COUNT + i + 2 ]++;
				m_KeyPressScale[ nPort ][ DIGITAL_KEY_COUNT + i + 2 ] = 255;
				m_KeyParam[ nPort ][ i ] = KeyParam;
				SendKeyEvent( nPort, DIGITAL_KEY_COUNT + i + 2, &KeyParam );
			}
			else
			{
				if(m_KeyPressTick[ nPort ][ 16 + i ] )
				{
					SendKeyEvent( nPort, KEY_RELEASE_FLAG + DIGITAL_KEY_COUNT + i + 2, &KeyParam);
					m_KeyPressTick[ nPort ][ DIGITAL_KEY_COUNT + i + 2 ] = 0;
					m_KeyPressScale[ nPort ][ DIGITAL_KEY_COUNT + i + 2 ] = 0;
					m_KeyParam[ nPort ][ i ].nPosX = 0;
					m_KeyParam[ nPort ][ i ].nPosY = 0;
				}
			}
		}

		if( ( m_GamePad[ nPort ].bLeftTrigger )&&( m_GamePad[ nPort ].bRightTrigger )
			&&( m_GamePad[ nPort ].wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER ) )
		{
			return 0;
		}
	}

	// 패드 빠졌습니다...
	return 1;
}

int CInputPad::GetKeyPressTick(int nPort, int nKeyCode)
{
	int i, nPressTick;

	if( nKeyCode < 0 )
	{
		return 0;
	}

	if(nPort==-1){
		nPressTick=0;
		for(i=0;i<MAX_XBOX_PAD_COUNT;i++){
			if(m_KeyPressTick[i][nKeyCode]>nPressTick){
				nPressTick=m_KeyPressTick[i][nKeyCode];
			}
		}
		return nPressTick;
	}
	else{
		return m_KeyPressTick[nPort][nKeyCode];
	}
}
int CInputPad::GetKeyPressScale(int nPort, int nKeyCode)
{
	int i, nPressScale;

	if( nKeyCode < 0 )
	{
		return 0;
	}

	if(nPort==-1){
		nPressScale=0;
		for(i=0;i<MAX_XBOX_PAD_COUNT;i++){
			if(m_KeyPressScale[i][nKeyCode]>nPressScale){
				nPressScale=m_KeyPressScale[i][nKeyCode];
			}
		}
		return nPressScale;
	}
	else{
		return m_KeyPressScale[nPort][nKeyCode];
	}
}

KEY_EVENT_PARAM *CInputPad::GetKeyParam(int nPort, int nStick)
{
//	int i, nIndex;

	if( nPort == -1 )
	{
		int i, nMax, nIndex;

		nMax = 0;
		nIndex = 0;
		for( i = 0; i < MAX_XBOX_PAD_COUNT; i++ )
		{
			if( nMax < abs( m_KeyParam[ i ]->nPosX ) + abs( m_KeyParam[ i ]->nPosY ) )
			{
				nMax = abs( m_KeyParam[ i ]->nPosX ) + abs( m_KeyParam[ i ]->nPosY );
				nIndex = i;
			}
		}
		return m_KeyParam[ nIndex ] + nStick;
	}
	else
	{
		return m_KeyParam[ nPort ] + nStick;
	}
}

void CInputPad::SendKeyEvent(int nPort, int nKeyCode, KEY_EVENT_PARAM *pParam)
{
	m_nLastKeyPressPort = nPort;
	if( !m_AttachHandle[ nPort ] )
	{
		return;
	}
	if( m_pStream )
	{
		if( ( nKeyCode & 0x7fffffff ) != PAD_INPUT_START )
		{
			INPUT_SAVE InputSave;

			m_pStream->Seek( sizeof( int ), BFileStream::fromBegin );
			m_nInputSaveCount++;
			m_pStream->Write( &m_nInputSaveCount, sizeof( int ), 4 );

			m_pStream->Seek( 0, BFileStream::fromEnd );
			InputSave.nTick = GetProcessTick();
			InputSave.nKeyCode = nKeyCode;
			InputSave.nPort = nPort;
			InputSave.KeyParam = *pParam;
			InputSave.Cross = *( m_AttachHandle[ nPort ]->GetCrossVector() );
			m_pStream->Write( &InputSave, sizeof( INPUT_SAVE ), 4 );
		}
	}
	CInput::SendKeyEvent( nPort, nKeyCode, pParam );
}

void CInputPad::Save()
{
	INP_FILE_HEADER Header;
	char szFileName[ 1024 ], *pFullName;
	SYSTEMTIME Time;

	memset( &Header, 0, sizeof( INP_FILE_HEADER ) );
	Header.nVersion = INP_FILE_VERSION;
	Header.nPlayerCount = g_FCGameData.nPlayerCount;
	Header.lRandomSeed = GetRandomSeed();
	strcpy( Header.szMapName, g_FCGameData.cMapFileName );
	strcpy( Header.szHeaderString, INP_FILE_HEADER_STRING );

	GetSystemTime( &Time );
//	sprintf( szFileName, "input\\%02d_%02d_%02d_%02d.inp", Time.wMonth, Time.wDay, Time.wHour, Time.wMinute );
	sprintf( szFileName, "input\\%s", "default.inp" );
	pFullName = g_BsKernel.GetFullName( szFileName );

	m_pStream = new BFileStream( pFullName, BFileStream::create );
	m_pStream->Write( &Header, sizeof( INP_FILE_HEADER ), 4 );
	m_nInputSaveCount = 0;
	m_InputSaveList.clear();
}

void CInputPad::StopSave()
{
	if( m_pStream )
	{
		delete m_pStream;
		m_pStream = NULL;
	}
}



void CInputPad::Play( const char *pFileName )
{
	char *pFullName;

	g_BsKernel.chdir( "input" );
	pFullName = g_BsKernel.GetFullName( pFileName );
	g_BsKernel.chdir( ".." );

	BFileStream Stream( pFullName );

	if( Stream.Valid() )
	{
		int i;
		INP_FILE_HEADER Header;
		INPUT_SAVE InputSave;

		Stream.Read( &Header, sizeof( INP_FILE_HEADER ), 4 );
		for( i = 0; i < Header.nInputCount; i++ )
		{
			Stream.Read( &InputSave, sizeof( INPUT_SAVE ), 4 );
			m_InputSaveList.push_back( InputSave );
		}
		m_nInputMode = 1;
		m_nCurrentInput = 0;

		g_FCGameData.bSaveInput = false;
		g_FCGameData.nPlayerCount = Header.nPlayerCount;

		g_FCGameData.lSaveRandomSeed = Header.lRandomSeed;

		strcpy( g_FCGameData.cMapFileName, Header.szMapName );
		g_FC.SetMainState( GAME_STATE_INIT_STAGE, ( DWORD )Header.szMapName );
	}
}

void CInputPad::ResetKeyBuffer()
{
	memset(m_GamePad, 0, sizeof(XINPUT_GAMEPAD)*MAX_XBOX_PAD_COUNT);
	memset(m_KeyPressTick, 0, sizeof(int)*MAX_XBOX_PAD_COUNT*MAX_KEY_CODE_COUNT);
	memset(m_KeyPressScale, 0, sizeof(int)*MAX_XBOX_PAD_COUNT*MAX_KEY_CODE_COUNT);
	memset( m_KeyParam, 0, sizeof( KEY_EVENT_PARAM ) * MAX_XBOX_PAD_COUNT * 2 );
	m_VibrationList.clear();
// [beginmodify] 2006/2/13 junyash PS#4981 doesn't vibrate non active Controller.
	#if 0
	SetVibration( 0, 0, 0 );
	#else
	for( int i = 0; i < MAX_XBOX_PAD_COUNT; i++ )
	{
		SetVibration( i, 0, 0 );
	}
	#endif
// [endmodify] junyash
}

void CInputPad::SetVibration( int nPort, int nLeftMortor, int nRightMortor )
{
	XINPUT_VIBRATION Vibration;

	if( nLeftMortor > 65535 )
	{
		Vibration.wLeftMotorSpeed = 65535;
	}
	else if( nLeftMortor < 0 )
	{
		Vibration.wLeftMotorSpeed = 0;
	}
	else
	{
		Vibration.wLeftMotorSpeed = ( WORD )nLeftMortor;
	}
	if( nRightMortor > 65535 )
	{
		Vibration.wRightMotorSpeed = 65535;
	}
	else if( nRightMortor < 0 )
	{
		Vibration.wRightMotorSpeed = 0;
	}
	else
	{
		Vibration.wRightMotorSpeed = ( WORD )nRightMortor;
	}

	if( m_bVibrationOn )
	{
		XInputSetState( nPort, &Vibration );
	}
}

void CInputPad::AddVibration( int nPort, int nMortor, int nMaxSpeed, int nInitSpeed, int nIncreasePercent, int nDuration, int nStartDelay/* = 0*/ )
{
	VIBRATION_INFO Vibration;

	BsAssert( nDuration > 0 );

	Vibration.nPort = nPort;
	Vibration.nMortor = nMortor;
	Vibration.nMaxSpeed = nMaxSpeed;
	Vibration.nCurSpeed = nInitSpeed;
	Vibration.nIncrease = ( int )( nMaxSpeed * ( nIncreasePercent / 100.0f ) );
	Vibration.nDuration = nDuration;
	Vibration.nStartDelay = nStartDelay;

	m_VibrationList.push_back( Vibration );
}

void CInputPad::ProcessVibration()
{
	int i;
	int nSpeed;
	int nLeftSpeed[ MAX_XBOX_PAD_COUNT ], nRightSpeed[ MAX_XBOX_PAD_COUNT ];
	std::vector< VIBRATION_INFO >::iterator it;

	for( i = 0; i < MAX_XBOX_PAD_COUNT; i++ )
	{
		nLeftSpeed[ i ] = 0;
		nRightSpeed[ i ] = 0;
	}
	it = m_VibrationList.begin();
	while( it != m_VibrationList.end() )
	{
		if( it->nStartDelay <= 0 )
		{
			if( it->nCurSpeed >= it->nMaxSpeed )
			{
				if( it->nDuration > 0 )
				{
					nSpeed = ( int )it->nMaxSpeed;
					it->nDuration--;
				}
				else
				{
					if( it->nIncrease > 0 )
					{
						it->nIncrease = -it->nIncrease;
					}
					nSpeed = it->nCurSpeed + it->nIncrease;
				}
			}
			else
			{
				nSpeed = it->nCurSpeed + it->nIncrease;
			}
			if( nSpeed > 0 )
			{
				if( it->nMortor == 0 )
				{
					if( nSpeed > nLeftSpeed[ it->nPort ] )
					{
						nLeftSpeed[ it->nPort ] = nSpeed;
					}
				}
				else
				{
					if( nSpeed > nRightSpeed[ it->nPort ] )
					{
						nRightSpeed[ it->nPort ] = nSpeed;
					}
				}
				it->nCurSpeed = nSpeed;
			}
			else
			{
				it = m_VibrationList.erase( it );
				continue;
			}
		}
		else
		{
			it->nStartDelay--;
		}
		it++;
	}
	if( m_bEnable )
	{
		for( i = 0; i < MAX_XBOX_PAD_COUNT; i++ )
		{
			SetVibration( i, nLeftSpeed[ i ], nRightSpeed[ i ] );
		}
	}
	else
	{
		for( i = 0; i < MAX_XBOX_PAD_COUNT; i++ )
		{
			SetVibration( i, 0, 0 );
		}
	}
}
