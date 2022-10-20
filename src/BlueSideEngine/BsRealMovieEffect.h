#pragma once

#pragma pack( push )
#pragma pack( 4 )

// #include "BsRealMovieConst.h"

class CRMPackage;

// 이펙트 파티클 데이타를 수록 합니다 
class CRMEffect 
{
public :

	enum	enumLinkedType
	{
		LTYPE_NONE = 0,
		LTYPE_FILE,
		LTYPE_OBJECT,
		LTYPE_WAVE,
		LTYPE_CAMERA,

		LTYPE_MAX,

		EFFECTPARAM_MAX=3,
		eFileNameLength=64,
		eKeyStringLength=130,
	};

	CRMEffect() 
		: m_fTime(-1.f)
		,m_nType(0)
		,m_fUseTime(-1.f)
		,m_bDisable(0) 
		,m_nLinkedType( LTYPE_NONE )
		,m_dwLinkedObject( 0 )
		,m_pOwnerPackage( NULL )
		,m_nObjectIndex(-1)
	{
		memset( m_szFileName, 0, eKeyStringLength );
		memset( m_dwParam, 0, sizeof(DWORD)*EFFECTPARAM_MAX );
		memset( m_szLinkedObjectName, 0, eFileNameLength );
		memset( m_szStringParam, 0, eFileNameLength );

		SubData.m_Cross.Reset();

		SubData.m_nYaw = 0;
		SubData.m_nRoll = 0;
		SubData.m_nPitch = 0;
		SubData.m_dwTick = -1;
		SubData.m_nListIndex = -1;
		SubData.m_nUseFrame = -1;

		SubData.m_fParam[0] = 0.f;
		SubData.m_fParam[1] = 0.f;
		SubData.m_fParam[2] = 0.f;
		SubData.m_fParam[3] = 0.f;

		m_vecAngleForRender.x = 0.f;
		m_vecAngleForRender.y = 0.f;
		m_vecAngleForRender.z = 0.f;

		m_vecAngleForProcess.x = 0.f;
		m_vecAngleForProcess.y = 0.f;
		m_vecAngleForProcess.z = 0.f;
	};
	~CRMEffect(){};

	void	SetName(char *szName)	{	strcpy( m_szName, szName );	}
	char*	GetName(void)			{	return m_szName;			}

	void	SetFileName(char *szFile)	{	strcpy( m_szFileName, szFile );	}
	char*	GetFileName(void)			{	return m_szFileName;			}
	BOOL	HasFileName()				{	return (m_szFileName[0] != NULL); }

	void	SetLinkedObjName(char *szFile)	{	strcpy( m_szLinkedObjectName, szFile );	}
	char*	GetLinkedObjName(void)			{	return m_szLinkedObjectName;			}
	BOOL	HasLinkedObjName()				{	return (m_szLinkedObjectName[0] != NULL);}
	BOOL	HasConditionFXLink()			
	{	
		return ( IsFXType() && HasLinkedObjName() && HasFileName() && HasStrParam() );
	}

	void	SetStrParam(char *szText)		
	{	
		if( szText == NULL || szText[0] == NULL )
			memset( m_szStringParam, 0 , eFileNameLength );
		else
			strcpy( m_szStringParam, szText );	
	}
	char*	GetStrParam(void)				{	return m_szStringParam;	}
	BOOL	HasStrParam()					{	return (m_szStringParam[0] != NULL );	}

	void	SetStartTime(float fTime,DWORD dwTick)	
	{	
		m_fTime = fTime;		
		SubData.m_dwTick = dwTick;
	}

	float	GetStartTime(void)			{	return m_fTime;			}
	DWORD	GetStartTick(void)			{	return SubData.m_dwTick;}

	void	SetUseTime(float fTime)		
	{	
		m_fUseTime = fTime;		
		SubData.m_nUseFrame = (int)(m_fUseTime * 40.f);
	}
	void	SetUseFrame(int nFrame)		
	{
		SubData.m_nUseFrame = nFrame;
		m_fUseTime = (float)nFrame / 40.f;
	}
	float	GetUseTime(void)			{	return m_fUseTime;		}
	int		GetUseFrame(void)			{	return SubData.m_nUseFrame;	}

	void	SetDisable(int nSet)			{	m_bDisable = nSet;		}
	int		GetDisable(void)				{	return m_bDisable;		}

	void	SetEffectType(int nType)		{	m_nType = nType;		}
	int		GetEffectType(void)				{	return m_nType;			}

	void	SetLinkedType(int nType)		{	m_nLinkedType = nType;	}
	int		GetLinkedType(void)				{	return m_nLinkedType;	}

	void	SetParam(int nIndex,DWORD dwParam)		{	m_dwParam[nIndex] = dwParam;	}
	DWORD	GetParam(int nIndex)					{	return m_dwParam[nIndex];		}

	void	SetFParam(int nIndex,float fParam)	{	SubData.m_fParam[nIndex] = fParam;	}
	float	GetFParam(int nIndex)				{	return SubData.m_fParam[nIndex];	}

	void	SetListIndex(int nVal)				{	SubData.m_nListIndex = nVal;		}
	int		GetListIndex(void)					{	return SubData.m_nListIndex;		}

	D3DXVECTOR3		GetPosition()			{	return SubData.m_Cross.m_PosVector;		}
	D3DXVECTOR3*	GetPosPtr()				{	return &(SubData.m_Cross.m_PosVector);	}

	void	SetPosition(D3DXVECTOR3& rPos);//	{	SubData.m_Cross.m_PosVector = rPos;		}
#ifdef _USAGE_TOOL_
	void	ResetEffectHBoardPos(void);		// 
#endif


	void	SetXPos(float fX);				//{	SubData.m_Cross.m_PosVector.x = fX;		}
	void	SetYPos(float fY);				//{	SubData.m_Cross.m_PosVector.y = fY;		}
	void	SetZPos(float fZ);				//{	SubData.m_Cross.m_PosVector.z = fZ;		}

	BOOL	IsMovable(VOID)					{	return (m_nType == eSpecialFX || m_nType == eFX );			}
	BOOL	IsFXType(VOID)					{	return (m_nType == eFX );	}
	BOOL	IsCmdFXType(VOID)				{	return (m_nType == eSpecialFX);	}
	BOOL	IsCamType(VOID)					{	return m_nType == eUseCam;	}
	BOOL	IsSndType(VOID)					{	return m_nType == eUseWave;	}
	BOOL	IsSndVolumeType(VOID)			{	return m_nType == eWaveVolume;	}
	BOOL	IsTroopType(VOID)				{	return (m_nType == eSetTroop || m_nType == eMoveTroop);	}
	BOOL	IsAlphaEnable(VOID)				{	return ( strcmp(m_szFileName,"ON") == 0 );	}
	BOOL	IsFadeType(VOID)				{	return (m_nType == eFadeIn || m_nType == eFadeOut );	}
	BOOL	IsEOMType(VOID)					{	return (m_nType == eTerminate);	}

	void		Save(BStream *pStream);
	void		Load(BStream *pStream);

	void		SaveEx(BStream *pStream);
	void		LoadEx(BStream *pStream);

	// 간단한 기능들을 담고 있다.
	static void	FadeIn(float fAlpha);
	static void	FadeOut(float fAlpha);
	static void	FillBlack(void);

	// static	int	EffectProcess(CRMEffect* pEffectObj,float fTime);
	static	int	EffectProcess(CRMEffect* pEffectObj,int nTime);

	void	SetHPos( D3DXVECTOR3 &pos, DWORD dwColor );

	VOID	CrossRotateYaw(int nAngle);
	VOID	CrossRotatePitch(int nAngle);
	VOID	CrossRotateRoll(int nAngle);

	VOID	CrossAbsoluteYaw(int nAngle);
	VOID	CrossAbsoluteRoll(int nAngle);
	VOID	CrossAbsolutePitch(int nAngle);

	int		GetYaw(void)	{	return SubData.m_nYaw;	}
	int		GetRoll(void)	{	return SubData.m_nRoll;	}
	int		GetPitch(void)	{	return SubData.m_nPitch;}

	VOID	SetLinkedObjPtr(DWORD dwData)	{	m_dwLinkedObject = dwData;	}
	DWORD	GetLinkedObjPtr(VOID)			{	return m_dwLinkedObject;	}

	void	SetLinkedObjIndex(int nIdx)		{	m_nObjectIndex = nIdx;		}
	int		GetLinkedObjIndex()				{	return m_nObjectIndex;		}

	void	SetEngineIndex(int nIdx)		{	m_nEngineIndex = nIdx;		}
	int		GetEngineIndex()				{	return m_nEngineIndex;		}

	void	SetFXHandle(int nHandle)		{	m_nHandle = nHandle;		}
	int		GetFXHandle()					{	return m_nHandle;			}

	BOOL	PtInfRect(D3DXVECTOR4& rfRc)
	{
		float *fx = &SubData.m_Cross.m_PosVector.x;
		float *fz = &SubData.m_Cross.m_PosVector.z;

		if( *fx >= rfRc.x && *fx <= rfRc.y && *fz >= rfRc.z	&& *fz <= rfRc.w )
		{
			return TRUE;
		}

		return FALSE;
	}

	VOID	MakeUseBoneData(VOID);
	VOID	MakeAverageDegree(VOID);

	VOID	MakeUseBoneDataForProcess(VOID);
	VOID	MakeAverageDegreeForProcess(VOID);

	VOID	ProcessBoneAniFrame( int nCurTick );
	VOID	ProcessBoneAniFrameForProcess( int nCurTick ); // 피직스 프로세스용.

#ifdef _USAGE_TOOL_
	void	ChangeFPS(void);
#endif

	VOID	DropWeapon(VOID);
    
public :

	char   m_szName[eFileNameLength];
	char   m_szFileName[eKeyStringLength];



	int    m_nType; // 해당 이펙트의 타입 입니다
	float  m_fTime;

	int		m_nLinkedType;	// 연결된 오브젝트 타입. (File,Obj,Wav,Cam)
	DWORD	m_dwParam[3];
	float  m_fUseTime;	// Sound 시에 volume 값으로도 사용가능.

	int    m_bDisable; // 이 값이 설정 되면 구동되지 않습니다

	char   m_szLinkedObjectName[eFileNameLength];
	char   m_szStringParam[eFileNameLength];

	// 추가될 데이터는 이곳에.
	struct
	{
		DWORD	m_dwTick;			// m_fTime :: StartTick
		CCrossVector  m_Cross;	

		int		m_nYaw;
		int		m_nRoll;
		int		m_nPitch;

		float	m_fParam[4];

		int		m_nListIndex;		// List Type에서 사용할거.
		int		m_nUseFrame;		// m_fUseTime :: UseTime
	} SubData;

	// 단순히 그리기 위해서
	struct COLORVERTEX
	{
		D3DXVECTOR3 Vertex;
		DWORD dwColor;
	};

	COLORVERTEX		m_vecHBoard[6];
	int				m_nEngineIndex;	// 파티클, FX용.
	DWORD			m_dwLinkedObject;					 // 오브젝트 이름 통해서.

	// UseBone 용
	int				m_nKeepUpTick;	// 유지 시간.

	int				m_nStartAveDegree[3];
	int				m_nEndAveDegree[3];

	float			m_fStartAveDegree[3];
	float			m_fEndAveDegree[3];

	D3DXVECTOR3		m_vecAngleForRender;
	D3DXVECTOR3		m_vecAngleForProcess;

	int				m_nKeepUpTickForProcess;

	float			m_fStartAveDegreeForProcess[3];
	float			m_fEndAveDegreeForProcess[3];

	// Package 용
	CRMPackage*		m_pOwnerPackage;
	int				m_nObjectIndex;		// For FX
	int				m_nHandle;			// FX Pool 에 등록되어 있는 핸들.

};

#pragma pack( pop )
