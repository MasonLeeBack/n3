#pragma once

#include "CrossVector.h"
#include "FcGameObject.h"

class CFcTroopObject;

typedef CSmartPtr<CFcTroopObject>	TroopObjHandle;

class CFcWaveObject : public CFcGameObject
{
public:
	CFcWaveObject(CCrossVector *pCross);
	virtual ~CFcWaveObject();
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
	// 부모 Virtual Function
	virtual void Process();
	virtual void PostProcess();
//	virtual void NextAnimation();

	virtual void Initialize( INITGAMEOBJECTDATA* pData, int nForce, int nTeam, TroopObjHandle hTroop, float fScaleMin=1.0f, float fScaleMax=1.0f );
	int Initialize( int nSkinIndex, int nAniIndex, int nShadowMeshIndex = -1, int nType = 0 );

	int GetVertexCount()	{ return m_nVertexCount; }
	int GetFaceCount()		{ return m_nFaceCount; }

	bool IsInit()	{ return m_bInit; }

/*
	void ChangeAnimation(int nAniIndex, int nBlendFrame=6, bool bOverlap=false);
	void SetSecondAnimation(int nAniIndex, const char *pBoneName);
*/



protected:
	static void CustomRender( void *pThis, C3DDevice *pDev, D3DXMATRIX *matObject );

public:
	ID3DXEffect* m_pEffect;		// 사용하는 녀석끼리 공유하게 해야 한다.

protected:
	static int m_nVertexCount;
	static int m_nFaceCount;
	static LPDIRECT3DVERTEXBUFFER9 m_pVB;
	static LPDIRECT3DVERTEXBUFFER9 m_pNormalVB;
	static LPDIRECT3DINDEXBUFFER9 m_pIB;
	int		m_nVertexDeclIdx;

	int			m_nSwizzleTex;
	int			m_nSplashPtc;

	bool		m_bInit;
};

typedef CSmartPtr<CFcWaveObject> WaveObjHandle;