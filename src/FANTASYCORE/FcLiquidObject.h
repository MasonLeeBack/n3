#pragma once

#include "CrossVector.h"
#include "FcGameObject.h"

class CFcTroopObject;

typedef CSmartPtr<CFcTroopObject>	TroopObjHandle;

typedef struct
{
	D3DXVECTOR4 Pos;      // position and size
	D3DXVECTOR4 N;
	D3DXVECTOR3	UV;
	D3DXVECTOR4	Diff;
	
} POINTSPRITEVERTEX;


class CFcLiquidObject : public CFcGameObject
{
public:
	CFcLiquidObject(CCrossVector *pCross);
	virtual ~CFcLiquidObject();
//	void ResetAniIndex();

protected:
/*
	int		m_nAniIndex[MAX_BONE_ANI_COUNT];
	float	m_fFrame[MAX_BONE_ANI_COUNT];
	int		m_nAniBoneIndex[MAX_BONE_ANI_COUNT];
	float	m_fPrevFrame;
	float	m_fFrameAdd;

	int m_nChangeAni;
	float m_fChangeFrame;
	int m_nBaseBlendFrame;
	int m_nBlendFrame;
	int m_nStiffFrame;
*/

public:
	// ºÎ¸ð Virtual Function
	virtual void Process();
	virtual void PostProcess();
//	virtual void NextAnimation();

	void Initialize( INITGAMEOBJECTDATA* pData, int nForce, int nTeam, TroopObjHandle hTroop, float fScaleMin=1.0f, float fScaleMax=1.0f );
	int Initialize( int nSkinIndex, int nAniIndex, int nShadowMeshIndex = -1, int nType = 0 );

	bool IsInit()	{ return m_bInit; }

	bool	IsGathered()	{ return m_bGather;}
	void	Gather(bool bOpt);

/*
	void ChangeAnimation(int nAniIndex, int nBlendFrame=6, bool bOverlap=false);
	void SetSecondAnimation(int nAniIndex, const char *pBoneName);
*/



protected:
	static void CustomRender( void *pThis, C3DDevice *pDev, D3DXMATRIX *matObject );

	bool	ComputeBounce(D3DXVECTOR3 *pVelOut, D3DXVECTOR3 *pPos, float fHeight, D3DXVECTOR3 *pVel);

	int			m_nV;
	
	D3DXVECTOR3	*m_pV;
	D3DXVECTOR3	*m_pN;
	int			*m_pB;	// bone id list

	D3DXVECTOR3	*m_pVT;	// vertex list transformed
	D3DXVECTOR3	*m_pNT;	// normal list transformed

#define	PTCTRAIL_MAX		3

	LPDIRECT3DVERTEXBUFFER9	m_pVB;
	LPDIRECT3DINDEXBUFFER9  m_pIB;

	typedef	struct
	{
		D3DXVECTOR3	Pos[PTCTRAIL_MAX];
		float		fSize[PTCTRAIL_MAX];
		D3DXVECTOR3	Vel[PTCTRAIL_MAX];
		D3DXVECTOR3	Acc[PTCTRAIL_MAX];
		bool		bBounded;
		int			nBounce;
		float		fAlpha;
	} PTCTRAIL;
	
	PTCTRAIL	*m_pPI;

	bool		m_bInit;

	int			m_nPGId;
	int			m_nMeshId;

	int			*m_pPL;	// particle list

	bool		m_bGather;

	int			m_nChunkTex;
	int			m_nChunkTex2;
	int			m_nSpecTex;

	ID3DXEffect*					m_pEffect;
	int			m_nVertexDeclIdx;

	float		m_fTime;
};

typedef CSmartPtr<CFcLiquidObject> FCObjHandle;