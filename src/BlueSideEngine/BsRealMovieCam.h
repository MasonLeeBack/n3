#pragma once

#include <vector>

#pragma pack( push )
#pragma pack( 4 )

struct COLORVERTEX_EX
{
	D3DXVECTOR3 Vertex;
	DWORD dwColor;
};

#define D3DFVF_COLORVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)


class CRMCamPath
{
public :

	CRMCamPath()
		: m_fFOV( -1.f )
		, m_fDuration( -1.f )
		, m_nYaw( 0 )
		, m_nRoll( 0 )
		, m_nPitch( 0 )
		, m_vecTarget( 0.f, 0.f, 0.f )
		, m_fCurTime( -1.f )
		, m_nSWayType( 0 )
		, m_fRoll( 0.f )
		, m_nMoveType( 0 )
		, m_nFrame( -1 )
	{
		m_vecCross[0].dwColor = 0xFFFF00FF;
		m_vecCross[1].dwColor = 0xFFFF00FF;

		m_vecCross[2].dwColor = 0xFF0000FF;
		m_vecCross[3].dwColor = 0xFF0000FF;

		m_vecCross[4].dwColor = 0xFF00FF00;
		m_vecCross[5].dwColor = 0xFF00FF00;
	}

	VOID	SetYawRollPitch(int nY,int nR,int nP)
	{
		m_nYaw = nY;
		m_nRoll = nR;
		m_nPitch = nP;
	}

	VOID	SetRotateForIndex(int nIndex,int nVal)
	{
		switch( nIndex )
		{
		case 0 : SetYaw( nVal );	break;
		case 1 : SetRoll( nVal );	break;
		case 2 : SetPitch( nVal );	break;
		}
	}

	VOID	SetYaw(int nY)
	{
		m_nYaw = nY;
		ApplyYawRollPitch();
	}

	VOID	SetRoll(int nR)
	{
		m_nRoll = nR;
		ApplyYawRollPitch();
	}

	VOID	SetPitch(int nP)
	{
		m_nPitch = nP;
		ApplyYawRollPitch();
	}

	VOID	ApplyYawRollPitch(VOID)
	{
		D3DXVECTOR3 vecPos = m_Cross.m_PosVector;
		m_Cross.Reset();
		m_Cross.m_PosVector = vecPos;

		m_Cross.RotateYawByWorld( m_nYaw );
		m_Cross.RotatePitch( m_nPitch );
		m_Cross.RotateRoll( m_nRoll );

		RecalcPivotLine();
	}

	VOID	RecalcPivotLine(VOID)
	{
		D3DXVECTOR3 vecTemp1;
		D3DXVECTOR3 vecTemp2;

		C3DDevice* pDevice = g_BsKernel.GetDevice();
		LPDIRECT3DDEVICE9 pD3Device = pDevice->GetD3DDevice();

		vecTemp1 = m_Cross.m_XVector + m_Cross.m_PosVector;
		vecTemp2 = ( (m_Cross.m_XVector) * 100.f) + m_Cross.m_PosVector;

		m_vecCross[0].Vertex = vecTemp1;
		m_vecCross[1].Vertex = vecTemp2;

		vecTemp1 = (m_Cross.m_ZVector * 400.f) + m_Cross.m_PosVector;
		// vecTemp2 = (-(cv->m_ZVector) * 100.f ) + cv->m_PosVector;
		vecTemp2 = (m_Cross.m_ZVector) + m_Cross.m_PosVector;

		m_vecCross[2].Vertex = vecTemp1;
		m_vecCross[3].Vertex = vecTemp2;

		vecTemp1 = (m_Cross.m_YVector * 100.f) + m_Cross.m_PosVector;
		vecTemp2 = (m_Cross.m_YVector) + m_Cross.m_PosVector;

		m_vecCross[4].Vertex = vecTemp1;
		m_vecCross[5].Vertex = vecTemp2;
	}

	BOOL	PtInfRect(D3DXVECTOR4& rfRc)
	{
		float *fx = &m_Cross.m_PosVector.x;
		float *fz = &m_Cross.m_PosVector.z;

		if( *fx >= rfRc.x && *fx <= rfRc.y && *fz >= rfRc.z	&& *fz <= rfRc.w )
		{
			return TRUE;
		}

		return FALSE;
	}

	float	GetFOV(void)	{	return m_fFOV;	}

	void	Save( BStream *pStream );
	void	Load( BStream *pStream, int nVer );

	CCrossVector  m_Cross;
	float		  m_fFOV;
	float		  m_fDuration;	// 다음 패스까지 진행 시간.

	// 추가
	int			m_nYaw;
	int			m_nRoll;
	int			m_nPitch;

	// 추가
	D3DXVECTOR3   m_vecTarget;		// 지면.. 카메라 패스가 시작될곳.

	// 추가
	int			m_nSWayType;
	float		m_fRoll;
	int			m_nMoveType;
	int			m_nFrame;			// Duration 을 대체하는 값.

	int			m_nCumulativeFrame;	// 누적된 프레임 수.


	COLORVERTEX_EX	m_vecCross[6];	// Pivot 그리기용.
	float			m_fCurTime;		// 내부에서 쓰이는 시간 계산용.

protected :

};

#define CAMPATH_BLOCK_SIZE	100

typedef std::vector<CRMCamPath*>	vecCamPath;
typedef std::vector<COLORVERTEX_EX>	vecClrVertex;


// 카메라를 관리 합니다 (BC 파일 32 개 = 맥스에서 작업한 프리 카메라), 라이팅도 추가로 관리 합니다
class CRMCamera 
{
public :

	enum 
	{ 
		eFileNameLength=64,
		eCameraFirstSize=eFileNameLength*2, 
	};

	CRMCamera();
	~CRMCamera();

	void	SetName(char *szName)	{	strcpy( m_szName, szName );	}
	char*	GetName(void)			{	return m_szName;			}

	void	SetFileName(char *szFile)	
	{	
		if( szFile == NULL || szFile[0] == NULL )
			memset( m_szFileName, 0 , eFileNameLength );
		else
			strcpy( m_szFileName, szFile );	
	}

	char*	GetFileName(void)			{	return m_szFileName;			}
	int		HasBCFile(void)				{	return ( m_szFileName[0] != NULL );	}

	void	SetBCPath(char* szPath)
	{
		if( szPath == NULL || szPath[0] == NULL )
			memset( m_szFilePath , 0, eFileNameLength);
		else
			strcpy( m_szFilePath, szPath );
	}

	char*	GetBCPath(void)			{	return m_szFilePath;			}

	void	SetStartTime(float fTime)	{	m_fTime = fTime;		}
	float	GetStartTime(void)			{	return m_fTime;			}

	void	SetFOV(float fVal)			{	m_fFOV = fVal;			}
	float	GetFOV(void)				{	return m_fFOV;			}

	void	SetUseOffset(int nVal)		{	m_nUseOffsetForBC = nVal;	}
	int		GetUseOffset(void)			{	return m_nUseOffsetForBC;	}

	CBsCameraSampledKeyFrame&	GetBCameraObject()	{	return m_BC;		}
	int		GetBCCamPosKeyCnt()					{	return m_BC.GetPositionKeyCount(); }
	void	ProcessBCCamera(CCrossVector& rCamCross,int nTick);

	void		Save(BStream *pStream);
	void		Load(BStream *pStream,int nVer);

	void		SaveEx(BStream *pStream);
	void		LoadEx(BStream *pStream);

	void	CreateCamObject(void);
	void	RemoveCamObject(void);
	void	UpdateObject(float fFrame);
	CRMCamPath*	SetCrossVector(CCrossVector& rCross);
	D3DXVECTOR3		GetPosition()			{	return m_Cross.m_PosVector;		}
	D3DXVECTOR3*	GetPosPtr()				{	return &m_Cross.m_PosVector;		}
	void	SetPosition(D3DXVECTOR3& rPos);//	{	m_Cross.m_PosVector = rPos;		}

	void	SetXPos(float fX);
	void	SetYPos(float fY);
	void	SetZPos(float fZ);


	// Pool 관리.
	void	ReleaseAllPath(void);
	CRMCamPath*	InsertNewPath( CCrossVector& rCross );
	CRMCamPath*	InsertNewPath( CCrossVector& rCross,float& rfFov );
	int		DeletePath( int nIndex );

	// Path 관련.
	int		GetPathCnt(void)	{	return (int)m_CamPathPool.size();	}
	CRMCamPath*	GetPathPtr(int nIndex)	{	return m_CamPathPool[ nIndex ];	}

	void	SetPathCrossVector(int nIndex,CCrossVector& rCross);

	void	SetPathXPos(int nIndex,float fX);
	void	SetPathYPos(int nIndex,float fY);
	void	SetPathZPos(int nIndex,float fZ);
	void	SetPathPosition(int nIndex,D3DXVECTOR3& rPos);

	int		InsertAPathBeforeSelectedPath(int nIndex);
	VOID	SyncDrawLine(int nIndex);

	float	GetAllPathsDuration(void);

	void	CalcDuration( float fCurTime );
	void	RecalcDuration( void );
	// void	RecalcDuration( int nIndex,float fCurTime);

	float	GetPathFOV(int nIndex)
	{
		return m_CamPathPool[nIndex]->m_fFOV;
	}

	void	SetPathFOV(int nIndex,float fFov)
	{
		m_CamPathPool[nIndex]->m_fFOV = fFov;
	}

	/*
	float	GetPathDuration(int nIndex)
	{
	return m_CamPathPool[nIndex]->m_fDuration;
	}

	void	SetPathDuration(int nIndex,float fDur)
	{
	m_CamPathPool[nIndex]->m_fDuration = fDur;
	}
	*/

	int		GetPathDuration(int nIndex)
	{
		return m_CamPathPool[nIndex]->m_nFrame;
	}

	int		GetCumulativeFrame(int nIndex)
	{
		return m_CamPathPool[nIndex]->m_nCumulativeFrame;
	}

	void	SetPathDuration(int nIndex,int nFrame)
	{
		m_CamPathPool[nIndex]->m_nFrame = nFrame;
	}

	D3DXVECTOR3&	GetPathPosition(int nIndex)
	{
		return m_CamPathPool[nIndex]->m_Cross.m_PosVector;
	}

	D3DXVECTOR3		GetPathDir(int nIndex )
	{
		return m_CamPathPool[nIndex]->m_Cross.m_ZVector;
	}

	D3DXVECTOR3*	GetPathPosPtr(int nIndex)
	{
		return &(m_CamPathPool[nIndex]->m_Cross.m_PosVector);
	}

	D3DXVECTOR3&	GetPathTarget(int nIndex)
	{
		return m_CamPathPool[nIndex]->m_vecTarget;
	}

	void	SetSWayType(int nIndex,int nType)	{	m_CamPathPool[nIndex]->m_nSWayType = nType;	}
	void	SetRollValue(int nIndex,float fRoll){	m_CamPathPool[nIndex]->m_fRoll = fRoll;		}
	void	SetMoveType(int nIndex,int nType)	{	m_CamPathPool[nIndex]->m_nMoveType = nType;	}

	int		GetSWayType(int nIndex)	{	return m_CamPathPool[nIndex]->m_nSWayType;	}
	float	GetRollValue(int nIndex){	return m_CamPathPool[nIndex]->m_fRoll;		}
	int		GetMoveType(int nIndex)	{	return m_CamPathPool[nIndex]->m_nMoveType;	}

	BOOL	PtInfRect(D3DXVECTOR4& rfRc)
	{
		vecCamPath::iterator it = m_CamPathPool.begin();
		vecCamPath::iterator itEnd = m_CamPathPool.end();

		for( ; it != itEnd ; ++it )
		{
			if( (*it)->PtInfRect( rfRc ) )
			{
				return TRUE;
			}
		}

		return FALSE;
	}


#ifndef _XBOX

	void	DrawCamLine(void);
	void	DrawCamPivotLine(void);

#endif

	void Play();
	void Process();

	// BC관련
	void	LoadBCResource(void);

#ifdef _USAGE_TOOL_
	void	ChangeFPS(void);
#endif



public :

	char   m_szName[eFileNameLength];
	char   m_szFileName[eFileNameLength];
	char   m_szFilePath[eFileNameLength];

	int    m_nLightYaw;	// 라이팅 벡터에 적용할 내용 입니다 (내용 확장 예정)
	int    m_nLightRoll;
	int    m_nLightPitch;

	float  m_fTime;

	int		m_nCamYaw;
	int		m_nCamRoll;
	int		m_nCamPitch;

	// 추가 데이터
	int		m_nUseOffsetForBC;	// BC Cam에서 Offset 기능을 쓸것인가.


	// 출력용..
	int           m_nSkinIndex;
	int           m_nObjectIndex;
	CCrossVector  m_Cross;
	float		  m_fFOV;
	D3DXVECTOR3   m_vecGround;		// 지면.. 카메라 패스가 시작될곳.

	CBsCameraSampledKeyFrame m_BC;

	vecCamPath		m_CamPathPool;

	// Path 출력용
	vecClrVertex	m_ClrVertexPool;

	// BC용
	CCrossVector	m_crossBCSaved;
};

#pragma pack( pop )