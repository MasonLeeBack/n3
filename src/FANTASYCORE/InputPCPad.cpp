#include "stdafx.h"
#include "BsKernel.h"
#include "BStreamExt.h"
#include "InputPad.h"
#include "FcGlobal.h"
#include "FcUtil.h"
#include "FantasyCore.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


static int g_nKeyCode[MAX_KEY_CODE_COUNT]={
	0, 1, 2, 3, 8, 9, 12, 14, 13, 15, 6, 7, 10, 11, 4, 5, -1, -1
};

int g_nStickCount = 0;
LPDIRECTINPUTDEVICE8 g_pJoyStick=NULL;

BOOL CALLBACK EnumAxesCallback(const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext);
BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext);

CInputPad::CInputPad()
{
	int i;

	m_nInputMode = 0;
	m_pStream = NULL;
	m_nInputSaveCount = 0;
	m_pStream = NULL;

	m_pDI=NULL;
	for( i = 0; i < MAX_PC_PAD_COUNT; i++ )
	{
		m_pJoyStick[ i ] = NULL;
		memset(m_KeyPressTick[i], 0, sizeof(int)*MAX_KEY_CODE_COUNT);
	}
}


CInputPad::~CInputPad()
{
	int i;

	if(m_pDI){
		m_pDI->Release();
	}
	for( i = 0; i < MAX_PC_PAD_COUNT; i++ )
	{
		if( m_pJoyStick[ i ] ){
			m_pJoyStick[ i ]->Unacquire();
			m_pJoyStick[ i ]->Release();
		}
	}

	if( m_pStream )
	{
		delete m_pStream;
	}
}

int CInputPad::Create( int nPort )
{
	int i;

    if(FAILED(DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_pDI, NULL))){
		return 0;
	}
	if(FAILED(m_pDI->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, m_pDI, DIEDFL_ATTACHEDONLY))){
		return 0;
	}
	for( i = 0; i < MAX_PC_PAD_COUNT; i++ )
	{
		if( !m_pJoyStick[ i ] ){
			DebugString("Joystick not found.");
			return 0;
		}

		if(FAILED(m_pJoyStick[ i ]->SetDataFormat(&c_dfDIJoystick2))){
			return 0;
		}
	#pragma warning(disable:4312)
		if(FAILED(m_pJoyStick[ i ]->SetCooperativeLevel((HWND)nPort, DISCL_EXCLUSIVE|DISCL_FOREGROUND))){
			return 0;
		}
	#pragma warning(disable:4312)
		m_diDevCaps.dwSize=sizeof(DIDEVCAPS);
		if(FAILED(m_pJoyStick[ i ]->GetCapabilities(&m_diDevCaps))){
			return 0;
		}
		if(FAILED(m_pJoyStick[ i ]->EnumObjects(EnumAxesCallback, (void*)( m_pJoyStick[ i ] ), DIDFT_AXIS))){
			return 0;
		}
		m_pJoyStick[ i ]->Acquire();
	}

    return S_OK;
}

int CInputPad::Process()
{
	if( m_nInputMode == 0 )
	{
		return ProcessNormal();
	}
	else
	{
		return ProcessMacro();
	}
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
				if( m_KeyPressTick[ InputSave.nPort ][ InputSave.nKeyCode ] == 0 )
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
	int i, j;
    HRESULT     hr;
    DIJOYSTATE2 js;
	KEY_EVENT_PARAM KeyParam;
	short int sAnalogBuf[4];

	if( !m_pJoyStick[ 0 ] ){
		return 1;
	}

	for( j = 0; j < MAX_PC_PAD_COUNT; j++ )
	{
		if( m_pJoyStick[ j ] == NULL ){
			continue;
		}

		if(FAILED(m_pJoyStick[ j ]->Poll())){
			do{
				hr=m_pJoyStick[ j ]->Acquire();
			}
			while(hr==DIERR_INPUTLOST);

			return 1;
		}
	}

	for( j = 0; j < MAX_PC_PAD_COUNT; j++ )
	{
		memset( &KeyParam, 0, sizeof( KEY_EVENT_PARAM ) );
		memset( &js, 0,  sizeof( DIJOYSTATE2 ) );
		if(FAILED(GetDeviceState(j, sizeof(DIJOYSTATE2), &js))){
			return 1;
		}
/*
		if(FAILED(m_pJoyStick[ j ]->GetDeviceState(sizeof(DIJOYSTATE2), &js))){
			return 1;
		}
*/

		for(i=0;i<MAX_KEY_CODE_COUNT;i++){
			if(g_nKeyCode[i]==-1){
				continue;
			}
			if(js.rgbButtons[g_nKeyCode[i]]&0x80){
				m_KeyPressTick[j][i]++;
				if(m_KeyPressTick[j][i] == 1 )
				{
					SendKeyEvent(j, i, &KeyParam);
				}
			}
			else{
				if(m_KeyPressTick[j][i]){
					SendKeyEvent(j, KEY_RELEASE_FLAG+i, &KeyParam);
					m_KeyPressTick[j][i]=0;
				}
			}
		}

		sAnalogBuf[0]=(short int)js.lX;
		sAnalogBuf[1]=-(short int)js.lY;
		sAnalogBuf[2]=(short int)(js.lZ+js.lRx);
		sAnalogBuf[3]=-(short int)(js.lRz+js.lRy);

		for(i=0;i<2;i++){
			if((sAnalogBuf[i*2]>ANALOG_THRESH_HOLD)||(sAnalogBuf[i*2]<-ANALOG_THRESH_HOLD)||(sAnalogBuf[i*2+1]>ANALOG_THRESH_HOLD)||(sAnalogBuf[i*2+1]<-ANALOG_THRESH_HOLD)){
				KeyParam.nPosX=sAnalogBuf[i*2];
				KeyParam.nPosY=sAnalogBuf[i*2+1];
				m_KeyPressTick[j][16+i]++;
				m_KeyParam[j][i]=KeyParam;
				SendKeyEvent(j, 16+i, &KeyParam);
			}
			else{
				if(m_KeyPressTick[j][16+i]){
					SendKeyEvent(j, KEY_RELEASE_FLAG+16+i, &KeyParam);
					m_KeyPressTick[j][16+i]=0;
					m_KeyParam[j][i].nPosX=0;
					m_KeyParam[j][i].nPosY=0;
				}
			}
		}
	}

	return 1;
}

BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE *pdidInstance, VOID *pContext)
{
    HRESULT hr;
	LPDIRECTINPUTDEVICE8 pJoyStick;

	if(!pContext){
	    return DIENUM_STOP;
	}

    hr=((LPDIRECTINPUT8)pContext)->CreateDevice(pdidInstance->guidInstance, (LPDIRECTINPUTDEVICE8 *)&pJoyStick, NULL);

	CInputPad::GetInstance().SetDevice( g_nStickCount, pJoyStick );
	g_nStickCount++;

    if(FAILED(hr)){
        return DIENUM_CONTINUE;
	}

	if( g_nStickCount >= MAX_PC_PAD_COUNT )
	{
	    return DIENUM_STOP;
	}
	else
	{
        return DIENUM_CONTINUE;
	}
}

BOOL CALLBACK EnumAxesCallback(const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext)
{
    DIPROPRANGE diprg; 

    diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
    diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
    diprg.diph.dwHow        = DIPH_BYID; 
    diprg.diph.dwObj        = pdidoi->dwType; // Specify the enumerated axis
    diprg.lMin              = ANALOG_RANGE_MIN;
    diprg.lMax              = ANALOG_RANGE_MAX;
    
	if( FAILED(((LPDIRECTINPUTDEVICE8)pContext)->SetProperty(DIPROP_RANGE, &diprg.diph))){
		return DIENUM_STOP;
	}

    return DIENUM_CONTINUE;
}

KEY_EVENT_PARAM *CInputPad::GetKeyParam(int nPort, int nStick)
{
	if(nPort==-1){
		return m_KeyParam[0]+nStick;
	}
	else{
		return m_KeyParam[nPort]+nStick;
	}
}

HRESULT CInputPad::GetDeviceState( int nPort, DWORD cbData, void* lpvData )
{
	if( m_pJoyStick[ nPort ] ){
		if(FAILED(m_pJoyStick[ nPort ]->GetDeviceState(cbData, lpvData))){
			return 1;
		}
	}
	else{
		GetKeyboardState( (DIJOYSTATE2*)lpvData );
	}

	return 0;
}


void CInputPad::GetKeyboardState( DIJOYSTATE2* js )
{
	// 왼쪽 아날로그
	js->lRx = js->lRy = js->lX = js->lY = 0;
	if( GetAsyncKeyState( VK_UP ) & 0x8000 ){
		js->lRy -= ANALOG_RANGE_MAX;
	}

	if( GetAsyncKeyState( VK_DOWN ) & 0x8000 )		{ js->lRy += ANALOG_RANGE_MAX; }
	if( GetAsyncKeyState( VK_RIGHT ) & 0x8000 )		{ js->lRx += ANALOG_RANGE_MAX; }
	if( GetAsyncKeyState( VK_LEFT ) & 0x8000 )		{ js->lRx -= ANALOG_RANGE_MAX; }

	// 오른쪽 아날로그
	if( GetAsyncKeyState( 'W' ) & 0x8000 )			{ js->lY -= ANALOG_RANGE_MAX; }
	if( GetAsyncKeyState( 'S' ) & 0x8000 )			{ js->lY += ANALOG_RANGE_MAX; }
	if( GetAsyncKeyState( 'D' ) & 0x8000 )			{ js->lX += ANALOG_RANGE_MAX; }
	if( GetAsyncKeyState( 'A' ) & 0x8000 )			{ js->lX -= ANALOG_RANGE_MAX; }

	//A버튼
	if( GetAsyncKeyState( VK_END ) & 0x8000 )		{ js->rgbButtons[0] = 0x80; }
	else											{ js->rgbButtons[0] = 0; }

	//B버튼
	if( GetAsyncKeyState( VK_NEXT ) & 0x8000 )		{ js->rgbButtons[1] = 0x80; }
	else											{ js->rgbButtons[1] = 0; }

	//X버튼
	if( GetAsyncKeyState( VK_HOME ) & 0x8000 )		{ js->rgbButtons[2] = 0x80; }
	else											{ js->rgbButtons[2] = 0; }

	//Y버튼
	if( GetAsyncKeyState( VK_PRIOR ) & 0x8000 )		{ js->rgbButtons[3] = 0x80; }
	else											{ js->rgbButtons[3] = 0; }
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
		for(i=0;i<MAX_PC_PAD_COUNT;i++){
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
		for(i=0;i<MAX_PC_PAD_COUNT;i++){
			if(m_KeyPressTick[i][nKeyCode]>nPressScale){
				nPressScale=1;
			}
		}
		return nPressScale;
	}
	else{
		return m_KeyPressTick[nPort][nKeyCode];
	}
}

#include "FcBaseObject.h"
#include "FcGameObject.h"
void CInputPad::SendKeyEvent(int nPort, int nKeyCode, KEY_EVENT_PARAM *pParam)
{
	if( m_pStream )
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
		m_pStream->Write( &InputSave, sizeof( INPUT_SAVE ), 4 );
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
	///memset(m_GamePad, 0, sizeof(XINPUT_GAMEPAD)*MAX_XBOX_PAD_COUNT);
	memset(m_KeyPressTick, 0, sizeof(int)*MAX_PC_PAD_COUNT*MAX_KEY_CODE_COUNT);
	//memset(m_KeyPressScale, 0, sizeof(int)*MAX_XBOX_PAD_COUNT*MAX_KEY_CODE_COUNT);
}