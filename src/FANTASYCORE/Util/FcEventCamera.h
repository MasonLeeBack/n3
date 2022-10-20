#pragma	  once

#include "CrossVector.h"

class BStream;
class CBsCamera;

#define DEFAULT_CAMERA_SPEED		20.0f
#define CAMERA_TARGET_HEIGHT		140.0f
#define CAM_STATUS_RESERVED			488


enum CAM_TARGET_TYPE
{
	CAM_TARGET_TYPE_ERROR,
	CAM_TARGET_TYPE_NON,
	CAM_TARGET_TYPE_LOCAL_AXIS,
	CAM_TARGET_TYPE_LOCAL_CRD,
	CAM_TARGET_TYPE_LOCAL_CRD_AREA,
	CAM_TARGET_TYPE_NORMAL,
};



enum CAM_SEQ_ID
{
// 0 - 99 까지는  지정
	CAM_SEQ_ID_WATER = 100,
	CAM_SEQ_ID_FIRE,
	CAM_SEQ_ID_EARTH,
	CAM_SEQ_ID_SOUL,
	CAM_SEQ_ID_WIND,
	CAM_SEQ_ID_LIGHT_1,
	CAM_SEQ_ID_LIGHT_2,

	CAM_SEQ_ID_NUM
};

#define CAM_ABILITY_SEQ_START	CAM_SEQ_ID_WATER




enum CAM_ATTACH_TYPE
{
	CAM_ATTACH_NON,
	CAM_ATTACH_LOCAL_CRD,
	CAM_ATTACH_LOCAL_AXIS,
	CAM_ATTACH_PLAYER,
	CAM_ATTACH_PLAYER2,
	CAM_ATTACH_GRDNLDR1_1,		// 1 player의 1번째 호위병
	CAM_ATTACH_GRDNLDR1_2,		// 1 player의 2번째 호위병
	CAM_ATTACH_GRDNLDR2_1,		// 2 player의 1번째 호위병
	CAM_ATTACH_GRDNLDR2_2,		// 2 player의 2번째 호위병
	CAM_ATTACH_LOCAL_CRD_AREA,	// Loca_Crd랑 똑같은데 약간의 Area를 두어서 그 영역을 벗어났을 경우만 쫓아감
	CAM_ATTACH_BY_NAME,			// 실기 무비에서 사용

	CAM_ATTACH_NUM
};

struct CAMERATYPE
{
	CAM_ATTACH_TYPE Type;
	char cName[12];
};

enum CAM_SWAY_TYPE
{
	CAM_SWAY_NON,
	CAM_SWAY_SWAY1,
	CAM_SWAY_SWAY2,
	CAM_SWAY_SHOCK1,
	CAM_SWAY_SHOCK2,

	CAM_SWAY_NUM
};

struct CAMSWAYTYPE
{
	CAM_SWAY_TYPE Type;
	char cName[12];
};


struct CAM_SEQ_NAME
{
	CAM_SEQ_ID ID;
	char cName[32];
};



struct FC_CAM_STATUS
{
	FC_CAM_STATUS()
	{
		EyePos = D3DXVECTOR3( 0.f, 0.f, 0.f );
		TargetPos = D3DXVECTOR3( 0.f, 0.f, 0.f );

		TempPos = D3DXVECTOR3( 0.f, 0.f, 0.f );

		nDuration = 0;
		bUseTarget = FALSE;
		nTargetType = 0;
		nEyeType = 0;
		//		nAngleX = nAngleY = nAngleZ = 0;
		fFov = 0.8f;
		nSwayType = 0;
		nDOF = 0;
		nRoll = 0;

		nAccelType = 0;
		memset( Reserved, 0, CAM_STATUS_RESERVED );
	};

	bool Save( BStream* pStream );
	bool Load( BStream* pStream );

	D3DXVECTOR3 EyePos;
	D3DXVECTOR3 TargetPos;		// EyePos로 부터 로컬 좌표

	D3DXVECTOR3 TempPos;		// Edit할 때 임시 저장소

	int nDuration;

	BOOL bUseTarget;
	int nTargetType;
	int nEyeType;
	//	int nAngleX, nAngleY, nAngleZ;

	float fFov;
	int nSwayType;		// sway type
	int nDOF;
	int nRoll;
	int nAccelType;


	char Reserved[CAM_STATUS_RESERVED];
};

struct FC_CAM_SEQ
{
	std::vector<FC_CAM_STATUS> vecStatus;

	bool Save( BStream* pStream );
	bool Load( BStream* pStream );
};



///////////////////////////////////////////////
// CFcCameraChooser

class CFcCameraChooser
{
public:
	CFcCameraChooser();
	~CFcCameraChooser();

	void Process();
	void SetTarget( CAM_ATTACH_TYPE Type );
	bool GetCamPos( FC_CAM_STATUS* pStatus, D3DXVECTOR3* pEye, D3DXVECTOR3* pDir, D3DXVECTOR3* pPreEye = NULL, D3DXVECTOR3* pPreDir = NULL );
	CAM_TARGET_TYPE GetTarget( CAM_ATTACH_TYPE Type, CCrossVector* pCross );

protected:
	CAM_ATTACH_TYPE m_Type;
	CCrossVector m_Cross;

	bool m_bCheckLandHeight;
};



///////////////////////////////////////////////
// CFcEventCamera class

class CFcEventCamera
{
public:
	CFcEventCamera( CBsCamera* pCamera );
	~CFcEventCamera();

	bool Initialize();
	void Process();
	void PostProcess();

	void Clear();

	bool Load( char* pDefaultFileName, char* pUserFileName );
	bool Save( char* pDefaultFileName, char* pUserFileName );

	bool IsSeq( int nSeqIndex );
	void PlaySeq( int nSeqIndex );
	void SetSeq( FC_CAM_SEQ* pSeq );
	void PlayCurSeq();

	int StopSeq();
	void SetEnable( bool bEnable )			{ m_bEnable = bEnable; }
	bool IsEnable()							{ return m_bEnable; }
	void SetEditMode( bool bMode );		
	bool IsEditMode()						{ return m_bEditMode; }

	CCrossVector* GetCamCross()				{ return &m_CamCross; }
	void SetCamCross( CCrossVector* pCross )	{ m_CamCross = *pCross; }

	CCrossVector* GetCross()				{ return &m_Cross; }
	void SetCross( CCrossVector* pCross )	{ m_Cross = *pCross; }


	void SetTest( bool bSet )				{ m_bTest = bSet; }
	bool IsTest()							{ return m_bTest; }

	void AddNewStatus();
	bool EraseStatus( int nIndex );

	FC_CAM_STATUS* GetCurStatus( int nIndex );
	int GetNumStatus();

	void SetNextSeq( int nOffset );
	int GetCurSeqIndex()	{ return m_nCurSeq; }

	FC_CAM_SEQ* GetSeq( int nIndex );

	float GetSway()		{ return m_fSway; }
//	void SetSway( CAM_SWAY_TYPE nType ) { m_fSway = fSway; if( m_fSway == 0.f ) { m_fDiffX = m_fDiffY = 0.f; } }
	void SetSway( CAM_SWAY_TYPE nType );

//	void Shock(int nTick, float fShock, CamShockDir ShockDir);

	int GetSeqLength( int nIndex );
	bool GetCurState( int nIndex, int nTick, CCrossVector* pCross, float* pFov, int* pSway, int* pRoll );

	bool SaveCurStatus( int nIndex );
	bool InitStatus( CAM_ATTACH_TYPE EyeType, CAM_ATTACH_TYPE TargetType, FC_CAM_STATUS* pStatus );

	bool GetTargetPos( CCrossVector* pCross );
	bool IsTarget();

	void SetPause( bool bPause)		{ m_bPause = bPause; }	// 실기무비툴에서 사용
	bool IsPause()					{ return m_bPause; }	// 실기무비툴에서 사용

	bool SetCurDuration( int nTick );

	void SetRoll();

protected:
	
	void ProcessSeqPlay();
	void ProcessShock();
	void ProcessSway();

protected:
	bool m_bEnable;
	FC_CAM_SEQ* m_pDefaultSeqInfo;
	FC_CAM_SEQ* m_pUserSeqInfo;

	FC_CAM_STATUS m_CurStatus;
	int m_nCurSeq;
	int m_nCurInfo;
	int m_nCurDuration;
	CCrossVector m_Cross;
	CBsCamera* m_pCamera;
	bool m_bEditMode;
	bool m_bTest;
	CCrossVector m_CamCross;
	FC_CAM_SEQ m_vecExSeq;		// 실기 무비 같은 곳에서 직접 스테이트를 날려준다.
	D3DXVECTOR3 m_pCurPos;
	CFcCameraChooser* m_pChooser;
	CAM_SWAY_TYPE m_SwayType;
	float		m_fSway;
	float		m_fAccX, m_fAccY;
	float		m_fVelX, m_fVelY;
	float		m_fVelX2, m_fVelY2;
	float		m_fDiffX,m_fDiffY;
	float		m_fDVX, m_fDVY;


	int		m_nShockBegin;
	int		m_nDur;
	float		m_fShock;
//	CamShockDir	m_ShockDir;

	int m_nProcessTick;		// RealMovieTool땜시 만듬
	bool m_bPause;

	int m_nCurRoll;
	int m_nTargetRoll;

	D3DXVECTOR3 m_PreEye, m_PreDir;
};


extern CAMERATYPE g_CameraType[CAM_ATTACH_NUM];
extern CAMSWAYTYPE g_CameraSwayType[CAM_SWAY_NUM];
extern CAM_SEQ_NAME g_SeqName[CAM_SEQ_ID_NUM];