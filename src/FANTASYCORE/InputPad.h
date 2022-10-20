#pragma once

#include "Input.h"
#include "Singleton.h"
#include "CrossVector.h"

#define ANALOG_THRESH_HOLD 12800
#define ANALOG_RANGE_MAX 32767
#define ANALOG_RANGE_MIN -32767

#define PAD_KEY_CODE_COUNT 9
#define PAD_ANALOG_KEY_CODE_COUNT 4

#define KEY_RELEASE_FLAG	0x80000000
#define DIGITAL_KEY_COUNT	14
#define MAX_XBOX_PAD_COUNT	4
#define MAX_PC_PAD_COUNT	2
#define MAX_KEY_CODE_COUNT	18

// [beginmodify] 2006/1/16 junyash TODO: PS#2601,2612 controller remove
// maybe move to remove UI control source file.
#define PAD_REMOVE_USE_WITH_XNOTIFY
#define PAD_REMOVE_ANNOUNCE_WITH_BLADE
#define PAD_REMOVE_TEST_PLAYERS_ONLY
// [endmodify] junyash

#define INP_FILE_VERSION	10
#define INP_FILE_HEADER_STRING	"Input Save File Version 0.1"
struct INP_FILE_HEADER
{
	int nVersion;
	int nInputCount;
	int nPlayerCount;
	unsigned long lRandomSeed;
	char szMapName[ 256 ];
	char szHeaderString[ 256 ];
};

struct VIBRATION_INFO
{
	int nPort;
	int nMortor;
	int nIncrease;
	int nDuration;
	int nCurSpeed;
	int nMaxSpeed;
	int nStartDelay;
};

struct INPUT_SAVE
{
	int nTick;
	int nPort;
	int nKeyCode;
	KEY_EVENT_PARAM KeyParam;
	CCrossVector Cross;
};

class BFileStream;
#ifdef _XBOX
class CInputPad : public CInput, public CSingleton<CInputPad>
{
public:
	CInputPad();
	virtual ~CInputPad();

protected:
	int m_nInputMode;
	BFileStream *m_pStream;
	int m_nInputSaveCount;
	std::vector< INPUT_SAVE > m_InputSaveList;
	int m_nCurrentInput;

	int m_nPort;
	int m_nLastKeyPressPort;
	XINPUT_GAMEPAD m_GamePad[MAX_XBOX_PAD_COUNT];
	int m_KeyPressTick[MAX_XBOX_PAD_COUNT][MAX_KEY_CODE_COUNT];
	int m_KeyPressScale[MAX_XBOX_PAD_COUNT][MAX_KEY_CODE_COUNT];
	KEY_EVENT_PARAM m_KeyParam[MAX_XBOX_PAD_COUNT][2];
	bool m_bPadRemoved[ MAX_XBOX_PAD_COUNT ];
	bool m_bVibrationOn;

// vibration
	std::vector< VIBRATION_INFO > m_VibrationList;

public:
	virtual int Create( int nPort);
	virtual int Process();
	int ProcessNormal();
	int ProcessMacro();
	int GetKeyPressTick(int nPort, int nKeyCode);
	int GetKeyPressScale(int nPort, int nKeyCode);
	KEY_EVENT_PARAM *GetKeyParam(int nPort, int nStick);

	int GetLastKeyPressPort() { return m_nLastKeyPressPort; }

	void SendKeyEvent(int nPort, int nKeyCode, KEY_EVENT_PARAM *pParam);
	void Save();
	void StopSave();
	void Play( const char *pFileName );
	void ResetKeyBuffer();

	void AddVibration( int nPort, int nMortor, int nMaxSpeed, int nInitSpeed, int nIncreasePercent, int nDuration, int nStartDelay = 0 );
	void ProcessVibration();
	void SetVibration( int nPort, int nLeftMortor, int nRightMortor );
	bool IsPadRemoved( int nPort = 0 )						{ return m_bPadRemoved[ nPort ]; }

	void SetVibrationOn(bool bOn)			{ m_bVibrationOn = bOn; }
	bool IsVibrationOn()					{ return m_bVibrationOn; }
};
#else
class CInputPad : public CInput, public CSingleton <CInputPad>
{
public:
	CInputPad();
	virtual ~CInputPad();

protected:
	int m_nInputMode;
	BFileStream *m_pStream;
	int m_nInputSaveCount;
	std::vector< INPUT_SAVE > m_InputSaveList;
	int m_nCurrentInput;

	LPDIRECTINPUT8 m_pDI;
	LPDIRECTINPUTDEVICE8 m_pJoyStick[ MAX_PC_PAD_COUNT ];
	DIDEVCAPS m_diDevCaps;
	DIJOYSTATE2 m_PrevState;

	int m_KeyPressTick[MAX_PC_PAD_COUNT][MAX_KEY_CODE_COUNT];
	KEY_EVENT_PARAM m_KeyParam[MAX_PC_PAD_COUNT][2];
	bool m_bPadRemoved[ MAX_PC_PAD_COUNT ];

public:
	virtual int Create( int nPort);
	virtual int Process();
	int ProcessNormal();
	int ProcessMacro();
	void SetDevice( int nPort, LPDIRECTINPUTDEVICE8 pStick ) { m_pJoyStick[ nPort ] = pStick; }
	int GetKeyPressTick(int nPort, int nKeyCode);
	int GetKeyPressScale(int nPort, int nKeyCode);
	KEY_EVENT_PARAM *GetKeyParam(int nPort, int nStick);

	HRESULT GetDeviceState( int nPort, DWORD cbData, void* lpvData );
	void GetKeyboardState( DIJOYSTATE2* js );

	int GetLastKeyPressPort() { return -1; }

	void SendKeyEvent(int nPort, int nKeyCode, KEY_EVENT_PARAM *pParam);
	void Save();
	void StopSave();
	void Play( const char *pFileName );
	void ResetKeyBuffer();

	void AddVibration( int nPort, int nMortor, int nMaxSpeed, int nInitSpeed, int nIncreasePercent, int nDuration, int nStartDelay = 0 ) {}
	void SetVibration( int nPort, int nLeftMortor, int nRightMortor ) {}
	bool IsPadRemoved( int nPort = 0 )		{ return m_bPadRemoved[ nPort ]; }

	void SetVibrationOn(bool bOn) {};
	bool IsVibrationOn() { return false; };

};
#endif // #ifdef _XBOX

