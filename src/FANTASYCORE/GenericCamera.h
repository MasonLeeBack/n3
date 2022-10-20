#pragma once

#include "bstreamext.h"
#include "SmartPtr.h"
//#include "KufModelObject.h"

class CFcTroopObject;
class CFcGameObject;
class CBsCamera;
class CFcProjectile;


typedef CSmartPtr<CFcTroopObject>	TroopObjHandle;
typedef CSmartPtr<CFcGameObject>	GameObjHandle;


#define	_DOF_MIN	0.001f
#define	_DOF_NEAR_MAX	0.95f
#define	_DOF_MAX	0.9999f

#define	CAM_DEFAULT_VIEW_DIR				vec2f(0.0f,1.0f)

#define	CAM_GAUSSIAN_VALUE_STEP				50.0f

#define	CAM_MIN_DIST						150.0f
#define	CAM_MAX_DIST						1500.0f	// target에서 eye가 가장 멀어졌을 때 거리
#define	CAM_SWAY_MAX						10

#define	CAM_DEFAULT_HEIGHT_SAMPLING_SPACE	10.0f	// target에서 pos까지 이 간격으로 바닥의 높이를 뽑아서 카메라가 
													// 걸리는지 측정

#define JOYPAD_ANALOG_DEADZONE		15000
#define JOYPAD_ANALOG_RANGE_MAX		32767

enum	CamShockDir
{
	CAM_SHOCK_X = 1,		// 0b01
	CAM_SHOCK_Y = 2,		// 0b10
	CAM_SHOCK_XY = 3		// 0b11
};

enum	CamSwayType
{
	CAM_SWAY_NONE	= 0,
	CAM_SWAY		= 1,
	CAM_SWAY_SHOCK	= 2
};

struct	CAMSHOCKINFO
{
	int			nState;		// 0: N/A  1: move to new pos, 2: stay at new pos, 3: back to original pos.
	float		fCurX;
	float		fCurY;
	int			nCurTick;

	int			nTick;
	float		fX;
	float		fY;
	float		fSpeed1;
	float		fSpeed2;
};

struct CAMSTATUS
{
	
	int					nEyeSmooth;
	int					nTargetSmooth;
	int					nDuration;

	int					nEye;
	int					nTarget;

	float				EyeX;
	float				EyeY;
	float				EyeZ;

	float				TargetX;		// Target Target
	float				TargetY;
	float				TargetZ;

	float				fFov;

	int					nSway;
	int					nShock;

	int					nDOF;		// 0: false, 1: true, 2: default
	float				fDOFNear;
	float				fDOFFar;

	bool				bFocusing;
	char				nParam1;
	bool				bScreenEffect;

	int					nRoll;		// roll (angle)

	bool				bEyeWorldSmooth;	// world smooth가 켜져 있으면 step의 smooth계산이 world에 대해 행해진다.
	bool				bTargetWorldSmooth;
	
	unsigned char		reserved[32];

};

////////////////////////////////////////////////////////////////////////////////


enum	ACTIONFREECAMRESET
{
	AFCAM_NORMAL,
	AFCAM_REARVIEW,
	AFCAM_RESET
};


class CGenericCamera
{
public:
	CGenericCamera( CBsCamera* pCamera );
	~CGenericCamera();

///////////////////////////////////////////////////////////////////////////////////////////////////
//    외부 사용 용 멤버 함수들
///////////////////////////////////////////////////////////////////////////////////////////////////

	CCrossVector* GetCamCross()					{ return &m_CamCross; }
	void SetCamCross( CCrossVector* pCross )	{ m_CamCross = *pCross; }


	void			SetEditMode(bool bOpt);	// 편집모드 on/off: 편집모드에서는 smoothness가 적용되지 않고
											// CAM_ATTACH_PLAYER_AIR_TROOP에서 unit을 계속 바꿔준다는 등의
											// 일이 생기지 않는다.

	bool			IsEditMode(void);
	bool			IsEventSeq(void) {return m_bEventSeq; }
	bool			IsCollisionEnabled(void);
	bool			IsCamPitchLimited(void) {return m_bCamPitchLimit;}

	void			SetDirectMode(bool bOpt);
	void			EnableCollision(bool bOpt);
	
	void			SetUserSeq(int nSeqId, int nDuration, bool bForce);
	void			SeqStop(void);
	bool			IsSeqEnded(void);

	void			SetNoSway(bool bOpt)	{ m_bNoSway = bOpt; }
	bool			IsSwayOn(void)			{ return !m_bNoSway; }

	void			SetDevMode(bool bOpt);
	bool			IsDevMode(void);

	void			GatherSystemInfo(void);		// view에게 엔진의 fog near/far값, zclip near far 값등을 구해오게 시킨다.
	bool			IsTargetChanged(void);

	void			SetUserTarget(TroopObjHandle hTarget);	// 사용자가 카메라의 타겟을 직접 정해 준다.
	void			UnsetUserTarget(void);				// 카메라가 현재 선택된 부대를 타겟으로 사용한다.

///////////////////////////////////////////////////////////////////////////////////////////////////
	void			NewSeq(int nId);
	void			DelSeq(int nId);

	int				SeqAddCam(int nSeqId, CAMSTATUS *pStatus);
	int				SeqDelCam(int nSeqId);
	int				SeqDelCam(int nSeqId, int nIndex);
	CAMSTATUS*	SeqGetCam(int nSeqId, int nIndex);
	int				SeqGetCurSeqId(void);
	int				SeqGetCurCamId(void);
	int				SeqGetSize(int nSeqId);
	int				SeqGetLength(int nSeqId);

	void			SeqPlay(int nSeqId);

	void			SeqSave(void);
	void			SeqUserSave(void);
	bool			SeqLoad(char *pFileName);
	bool			SeqUserLoad(char *pFileName);
	const	char*	GetSeqFileName(void);
	const	char*	GetSeqFileName2(void);

	const	char*	GetUserSeqFileName(void);

	GameObjHandle	GetActionCamTarget(void); // 플레이어 alpha fog용. 현재 카메라가 액션 모드 카메라이면 player unit을 리턴한다.

	virtual	void	Process();

	void			MoveFreeCam(float fA, float fB);
	void			EnableFreeCam(bool bOpt);

	ACTIONFREECAMRESET	m_nActionFreeCamReset;
	
	void			GetCamPos3(D3DXVECTOR3  *vPos);
	void			GetCamPos2(D3DXVECTOR2 *v2Pos);
	void			GetCamDir3(D3DXVECTOR3  *vPos);

	void			GetCamDir(D3DXVECTOR2 *vDir);

	float			GetCamFov();

	CAMSTATUS*	GetCamStatus(void);

	void SetTargetTroop( TroopObjHandle Troop )	{ m_Target = Troop; }

	TroopObjHandle	GetTargetTroop(void) {return m_Target;}
	GameObjHandle	GetTargetUnit(void)	{return m_TargetUnit;}

	GameObjHandle	GetCamTarget(void);
	
	bool			IsMicAttachedOnPlayer(D3DXVECTOR3  *pPos, D3DXVECTOR2 *pDir);	// 0: Eye, 1: Player
	float			GetScreenAlign(void);
	void			SetScreenAlign(float fAlign);
	float			GetCurScreenAlign(void) { return m_fAlign;}

	bool			GetLatestAbilityPos(D3DXVECTOR3 	*pPos, D3DXVECTOR2 *pCurPos);

	float			m_fActionPlayerFowardScale;	// 키를 기준으로 액션 모드에서 플레이어 앞쪽을 얼만큼 멀리 보여줄지 수치
	float			m_fActionPlayerFowardSpeedScale;	// 스피드 기준으로 액션 모드에서 플레이어 앞쪽을 얼만큼 멀리 보여줄지 수치 

	bool			GetCrdNDir(int nCamType, int nParm, D3DXVECTOR3  *pPos, D3DXVECTOR3  *pDir);	// view에서만 사용.
	void			GetCursorPos(D3DXVECTOR2 *pPos);
	void			GetTroopPos(D3DXVECTOR2 *pPos);

	void			SetMarkedTroop(TroopObjHandle Troop);
	void			SetMarkedPoint(D3DXVECTOR2* pPoint);
	int				GetTick(void);

	bool			IsVirgin(void) {return m_bVirgin;}

	void			ChangeView(void);

	void			CamProcessPreset(void);
	void			CorrectCam();
	void			CamProcessFreeInAction(float fA, float fB);
	void			CamCollision(D3DXVECTOR3 *Pos, D3DXVECTOR3 *Target);

	bool			GetMatrix(int nType, int nParm, D3DXMATRIX *pMatrix);

	bool			IsErrorInUserCam(void) {return m_bUserCamErr;}

	D3DXVECTOR3			GetTargetPosV3();

	void			ToggleCamDist();
	void			SetToggleCamDist(bool bToggle);
	bool			IsToggleCamDist() const;
	void			MovePlayBack( int nDelayTick = 2 );

	float			GetFov()		{ return m_fCurFov; }

	void			SpecialCam(int nId, int nDur);	// nId: 0 ~ 8

	void			Shock3D(D3DXVECTOR3	*pCenter, float fRadius, float fX, float fY, int nDur);	// fX, fY: 0.0f ~ 1.0f

protected:

	void			CamProcessSway(void);
	void			CamProcessShock3D(void);

	enum	{
		CAMVIEW_ZOOMIN	= 0,
		CAMVIEW_ZOOMOUT
	};

	CBsCamera*		m_pCamera;

	CCrossVector	m_CamCross;

	int				m_nCurViewMode;

	bool			m_bEditMode;
	bool			m_bNoSway;
	bool			m_bDevMode;
	bool			m_bCollisionEnable;

	bool			m_bEventSeq;
	bool			m_bForceSeq;

	bool			m_bCamDirectMode;
	int				m_nShakeCount;
	D3DXVECTOR2			m_vEpicenter;

	D3DXVECTOR2			GetTargetPos();
	

	void			CopyCamStatus(void);

	void			PushCam(void);
	void			PopCam(void);

	void			ProcessSeq(void);

	void			ViewProcess();

/////////////// chooser 및 attach 관련 //////////////////////////////////////////////

	void			Chooser(void);
	void			AttachActionCamera(int nParm, D3DXVECTOR3	*pPos, D3DXVECTOR3	*pDir);

/////////////////////////////////////////////////////////////////////////////////////

	void			_SeqSave(int nOffSet, int nSize, char *pFileName);
	bool			_SeqLoad(int nOffset, int nSize, char *pFileName);
	void			SeqLoadVer13(int nOffset, int nSize, BMemoryStream *pBS);
	void			SeqLoadVer20(int nOffset, int nSize, BMemoryStream *pBS);

	void			InitActionCam(void);

	void			ObjPoseToMatrix(D3DXMATRIX *pOut, D3DXVECTOR3 *pPos, D3DXVECTOR3 *pZDir);

	// utilities

	TroopObjHandle 	GetPlayerAirTroop();
	void			GetTroopCrdNDir(TroopObjHandle pTroop, D3DXVECTOR3  *pPos, D3DXVECTOR3  *pDir);
	void			GetUnitCrdNDir(GameObjHandle Unit, D3DXVECTOR3  *pPos, D3DXVECTOR3  *pDir);
	void			GetTroopActualCrdNDir(TroopObjHandle pTroop, D3DXVECTOR3  *pPos, D3DXVECTOR3  *pDir);


	CAMSTATUS		m_Status;
	bool			m_bSaved;
	CAMSTATUS		m_SavedStatus;
	int				m_nSavedCurSeq;
	int				m_nSavedCurSeqCamId;
	bool			m_bUserCamErr;

	bool			m_bTargetChanged;
	TroopObjHandle	m_Target;
	GameObjHandle	m_TargetUnit;

	bool			m_bRunCurSeq;

	bool			m_bEyeInForest;
	bool			m_bTargetInForest;

	bool			m_bUserTarget;
	TroopObjHandle	m_UserTarget;


	typedef	struct
	{
		int				nSize;
		CAMSTATUS*	pCam;
	} CamSeq;

	int				m_nSeqList;
	CamSeq			*m_pSeqList;

	int				m_nCurSeq;
	int				m_nCurSeqCamId;

	int				m_nMarkedTick;
	int				m_nLastTime;

	int				m_nTickPlayBegan;
	int				m_nDuration;

	bool			m_bEyeUniformVel;
	bool			m_bTargetUniformVel;

	bool			m_bFreeCam;
	float			m_fFreeCamB;
	float			m_fFreeCamDA, m_fFreeCamDB;

	int				m_nRandom0;
	int				m_nRandom1;

	int				m_nDoingNothingCount;
	TroopObjHandle	m_PrevTroop;
	int				m_nPrevTargeted;

	char			m_SeqFileName[MAX_PATH];
	char			m_SeqFileName2[MAX_PATH];
	char			m_UserSeqFileName[MAX_PATH];

	int				m_nIsInForestCounter;
	bool			m_bIsCamForestModePrev;
	bool			m_bIsCamForestMode;

	bool			m_bIsMicAttachedOnPlayer;
	float			m_fAlign;
	float			m_fScreenAlign;

	D3DXVECTOR2		m_CursorPos;
	bool			m_bCursorPosChanged;

	D3DXVECTOR2		m_vActionEye;		// CAM_ATTACH_ACTION_EYE1을 위한 eye rel vec
	int				m_nLastNearestEnemy;
	float			m_fActionEyeHMod;

	bool			m_bCamPitchLimit;	// 카메라가 땅을 보지 못하게 막는다.
	bool			m_bIsHazeOn;

	int				m_nDefaultActionCam;

	bool			m_bPlayerSet;			// 플레이어가 누구인지 셋팅 되었음.
	TroopObjHandle	m_MarkedTroop;
	D3DXVECTOR2		m_MarkedPoint;
	float			m_fEyeAltitude;
	unsigned int	m_nInternalTick;
	bool			m_bAbilityPosValid;
	D3DXVECTOR3 	m_LatestAbilityPos;
	int				m_nLatestAbilityIdx;
	int				m_nPlaceInWorldMap;
	bool			m_bVirgin;

	// view
	D3DXVECTOR3		m_CamPos,m_CamTarget;
	D3DXVECTOR3		m_CamLastPos, m_CamLastTarget;
	float			m_fFreeCamTargetAlt;
	float			m_fFreeCamDist;
	float			m_fCurFreeCamDist;
	
	D3DXVECTOR3		m_CamCurPos,m_CamCurTarget;
	D3DXVECTOR3		m_CamCurLocalPos,m_CamCurLocalTarget;


	D3DXVECTOR3		m_FreeCamPos,m_FreeCamTarget;
	int				m_nStepPos, m_nStepTarget;

	float			m_fFov;
	float			m_fCurFov;

	int				m_nCamRoll;
	int				m_nCamCurRoll;

	float			m_fDOFNear;
	float			m_fDOFFar;


	int m_nPrevEye;
	int m_nPrevTarget;

	CFcProjectile* m_pProjectile;

	bool m_bDOF;

	D3DXVECTOR3 m_HandCamVel;
	D3DXVECTOR3 m_HandCamRelCrd;

	float m_fCamDist;

	int		m_nOrbCamTick;
	int		m_nActCamCounter;
	int		m_nFreeCamMoveCount;
	int		m_nFreeCamBLockCount;
	float	m_fDirCosPrev;
	float	m_fDirSinPrev;
	bool	m_bDefaultCamVar;

	bool	m_bModifyCamHeightByLand;

	D3DXVECTOR2		m_SwayPos;
	D3DXVECTOR2		m_SwayVel;

	bool			m_b3DShock;
	D3DXVECTOR3		m_v3DShockCenter;
	float			m_f3DShockX;
	float			m_f3DShockY;
	float			m_f3DShockRadius;
	unsigned int	m_n3DShockBegin;
	int				m_n3DShockDur;
	float			m_fShockPushX;
	float			m_fShockPushY;

	int				m_nSpecialCam;
	unsigned int	m_nSpecialCamBegin;
	int				m_nSpecialCamDur;
};
