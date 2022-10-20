#ifndef __KUF_CAM_EDITOR__
#define __KUF_CAM_EDITOR__


#define	CAM_SWAY_TYPE_MAX		3
#define	CAM_DIST_MIN			5.0f
#define	CAM_POPUP_DURATION		400

class CGenericCamera;

struct CAMSTATUS;

class	CCamEditor
{
public:
	
	CCamEditor( CGenericCamera *pCam );
	~CCamEditor(void);

	void	Activate(void);
	void	Deactivate(void);
	bool	IsActivated(void);

	void	Process(void);
	void	ProcessUI(bool CamModified);
	bool	PopUpOpen(void);
	void	PopUpProcess(void);
	void	PopUpClose(void);

	CGenericCamera*	GetCam(void);

	int			SeqGetCurId(void);
	void		SeqSetCurId(int nId);
	void		SeqPlay(void);
	void		SeqStop(void);
	bool		SeqIsPlaying(void);

	void		ShowCurCam(void);

protected:

	typedef struct
	{
		int					nEyeSmooth;
		int					nTargetSmooth;
		int					nDuration;

		int					nEye;
		int					nTarget;

		bool				bEyeWorldSmooth;
		bool				bTargetWorldSmooth;

		D3DXVECTOR3			*Eye;
		D3DXVECTOR3			*Target;

		D3DXVECTOR3			_Eye;
		D3DXVECTOR3			_Target;

		float				fFov;

		int					nSway;
		int					nShock;

		int					nDOF;		// 0: false, 1: true, 2: default
		float				fDOFNear;
		float				fDOFFar;
		
		bool				bFocusing;
		bool				bScreenEffect;
		char				nParam1;

		bool				bEyeHeightMod;	// 향하는 방향으로 올라가는 경사가 있을 경우 눈을 낮춰 줌.

		int					nRoll;

	} CamEditing;

	int					m_nCurItem;

	void				PrintCamStatus(CamEditing *pCam, int nCurSel, int nY);
	void				ModifyCamSeq(void);
	bool				ModifyCamStatus(CamEditing *pCam);
	bool				MoveCam(CamEditing *pCam, D3DXVECTOR3	*pEye, D3DXVECTOR3 *pTarget);
	void				ApplyCam(CamEditing *pCam);

	void				ObjPoseToMatrix(D3DXMATRIX *pOut, D3DXVECTOR3 *pPos, D3DXVECTOR3 *pZDir);

	void				GetMatrix(CamEditing *pCam, D3DXMATRIX *pEyeOut, D3DXMATRIX *pTargetOut);
	void				CamCoordToGlobal(D3DXVECTOR3 *pEyeOut, D3DXVECTOR3 *pTargetOut, CamEditing *pCam);
	void				GlobalToCamCoord(CamEditing *pCam,D3DXVECTOR3 *pEyeIn, D3DXVECTOR3 *pTargetIn);

	void				CamStatus2CamEditing(CamEditing *pEdit, CAMSTATUS *pStatus);
	void				CamEditing2CamStatus(CAMSTATUS *pStatus, CamEditing *pEdit);

	void				SetCamStatus(CAMSTATUS *pStatus);

	void				PopUpClear(void);
	void				PopUpAdd(char *pStr);

	D3DXVECTOR3			m_CurEyeGlobal;
	D3DXVECTOR3			m_CurEyeLocal;

	D3DXVECTOR3			m_CurTargetGlobal;
	D3DXVECTOR3			m_CurTargetLocal;

	CGenericCamera		*m_pCamera;

	bool				m_bActivated;

	int					m_nCurSeqId;
	int					m_nCurCamId;
	CamEditing			m_Cam;

	bool				m_bCamCopied;
	CamEditing			m_CamCopied;

	int					m_nEyeSmoothness;
	int					m_nTargetSmoothness;

	float				m_fMoveStep;
	float				m_fRotateStep;
	bool				m_bCamPlaying;

	int					m_nDefTableSize;
	char				**m_ppDefTable;


	bool				m_bPopUp;
	int					m_nPopUpActualSize;
	int					m_nPopUpSize;
	int					m_nPopUpCsr;
	int					m_nPopUpOffset;
	int					m_nPopUpWinWidth;
	int					m_nPopUpWinHeight;
	int					m_nPopUpX;
	int					m_nPopUpY;
	char				**m_ppPopUp;

	int					m_nEditMenuY;

	int					m_nDisableTick;
};

#endif