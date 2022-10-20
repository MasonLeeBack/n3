#pragma once

#include "CrossVector.h"
#include "FcBaseObject.h"
#include "GenericCamera.h"
#include "FcEventCamera.h"

class CFcBaseObject;
class CFcCameraObject;
class CGenericCamera;
//class CFcCameraChooser;
class CCamEditor;

struct FC_CAM_STATUS;

#define	CAMERA_PRESET_MAX			50




class CFcCameraObject : public CFcBaseObject
{
public:
	CFcCameraObject(CCrossVector *pCross);
	virtual ~CFcCameraObject();

	
	virtual bool Render();

protected:
	bool m_bFreeze;
	bool m_bDebugFreeCamMode;

	float m_fCameraDist;
	float m_fCameraZoom;
	D3DXVECTOR3 m_OriginalPos;
	D3DXVECTOR3 m_PrevPos;
	int m_nRotateYawAngle;
	int m_nRotatePitchAngle;
	CSmartPtr<CFcBaseObject> m_AttachHandle;

	int m_nInputPort;

	int		m_nPresetCam;
	bool	m_bBtnPress;
	

	int			m_nCurEditCam;

	int			m_nSavedSeqID;
	bool		m_bPreset;
	bool		m_bInputEnable;


	CGenericCamera* m_pEventCamera;

	CFcEventCamera* m_pRMEventCamera;

//	CFcCameraChooser* m_pChooser;
	char m_cDefaultFileName[64];
	char m_cUserFileName[64];

	float m_fEditModeDX;
	float m_fEditModeDY;

	float m_fNormalModeDX;
	float m_fNormalModeDY;

	D3DXVECTOR4 FogColor;
	float m_fFogNear, m_fFogFar;
	float m_fNearZ, m_fFarZ;
	float m_fCamFov;

	D3DXVECTOR2 m_vecUpdateRangeCenter;
	float		m_fUpdateRange;

	D3DXVECTOR4		m_vecDeltaFogColor;
	float			m_fDeltaFogNear;
	float			m_fDeltaFogFar;
	int				m_nDelayFogTick;

	CCamEditor*		m_pCamEditor;

	bool			m_bUseBCCam;

public:
	int Initialize(int nCamIndex, bool bUseDivide, int nInputPort, char* pDefaultCameraFileName, char* pUserCameraFileName);

	void Process();
	void AttachObject(CSmartPtr<CFcBaseObject> Handle);
	void SetTargetTroop( TroopObjHandle Handle );

	void UpdateUI();

	CSmartPtr<CFcBaseObject>	GetAttachObjectHandle()	{	return m_AttachHandle;	}

	void SetFreeze( bool bFreeze ) { m_bFreeze = bFreeze; }
	bool IsFreeze() { return m_bFreeze; }
	bool IsFreeCamMode();					// TODO: IsDebugFreeCamMode로 수정 해야함
	void SetFreeCamMode( bool bMode );		// TODO: SetDebugFreeCamMode로 수정 해야함

	void SetFreeCamMode_( bool bMode );		// TODO: SetFreeCamMode로 수정 해야함


	bool IsEditMode();
	void SetEditMode( bool bMode );
	
//	bool IsCamPreset(void) { return (m_bPreset | m_bCamEditMode); }
	bool IsCamPreset(void);

	void Save(void);

	void			SeqPlay(int nSeqId);

	void			SeqSave(void);
	void			SeqUserSave(void);
	bool			SeqLoad(char *pFileName);
	bool			SeqUserLoad(char *pFileName);
	const	char*	GetSeqFileName(void);
	const	char*	GetSeqFileName2(void);

	const	char*	GetUserSeqFileName(void);

	int StopEventSeq()	{ return SeqStop(); }		// SeqID리턴
	void PlayEventSeq( int nSeqId )	{ SeqPlay( nSeqId ); }		// SeqID리턴

	void SetRMCamSeq( FC_CAM_SEQ* pSeq );

	CBsCamera*	GetCamObj();

	void SetCamFov( float fFOV ) {	m_fCamFov = fFOV;	}
	float GetCamFov() { return m_fCamFov; }
//	void SetSway( CAM_SWAY_TYPE nType );

	bool SetCurDuration( int nTick );

	void SetFogColor( D3DXVECTOR4* pVec );
	const D3DXVECTOR4* GetFogColor() { return g_BsKernel.GetFogColor( m_nEngineIndex ); }
	void SetFogFactor( float fStart, float fEnd );
	void SetProjectionMatrix( float fNear, float fFar );
	void ChangeFog( D3DXVECTOR4* pColor, float fNear, float fFar, int nTick );

	float GetUpdateRange()				{	return m_fUpdateRange;	}
	D3DXVECTOR2	GetUpdateRangeCenter()	{	return m_vecUpdateRangeCenter;	}

	void SetBCCam( bool bEnable )		{ m_bUseBCCam = bEnable; }
	bool IsBCCam()						{ return m_bUseBCCam; }

	void SetDevCam( bool bEnable );

	void SetUserSeq(int nSeqId, int nDuration, bool bForce);
	int SeqStop();

	void SetMarkPoint( D3DXVECTOR2* Pos );
	void SetUserTarget( TroopObjHandle pTarget );

	void SetAlphaBlocking();

	float GetFogFar()	{	return m_fFogFar;	}

	void SetToggleCamDist(bool bToggle) { m_pEventCamera->SetToggleCamDist(bToggle); }
	bool IsToggleCamDist() const { return m_pEventCamera->IsToggleCamDist(); }

	void Shock( D3DXVECTOR3	*pCenter, float fRadius, float fX, float fY, int nDur) { m_pEventCamera->Shock3D( pCenter, fRadius, fX, fY, nDur ); }
	void SpecialCam( int nId, int nDur ) { m_pEventCamera->SpecialCam( nId, nDur ); }

	void MovePlayBack( int nDelayTick = 2 );		// 주인공 뒤쪽으로 카메라 돌려줌

protected:
	bool LookTarget( FC_CAM_STATUS* pStatus, CCrossVector* pCross );

	void ProcessFreeCam();
	void ProcessNormalMode();
	void ProcessEditMode();

	void ProcessFog();
};



typedef CSmartPtr<CFcCameraObject> CameraObjHandle;