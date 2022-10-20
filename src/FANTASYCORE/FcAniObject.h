#pragma once

#include "CrossVector.h"

#include "FcBaseObject.h"

#define RAGDOLL_MODE_NONE	0
#define RAGDOLL_MODE_LINK	1
#define RAGDOLL_MODE_FREE	2

class CFcAniObject : public CFcBaseObject
{
public:
	CFcAniObject( CCrossVector *pCross, bool bInsertProcess = true );
	virtual ~CFcAniObject();
	void ResetAniIndex();

protected:
	int		m_nAniIndex;
	float	m_fFrame;
	float	m_fPrevFrame;
	float	m_fFrameAdd;

	int m_nChangeAni;
	float m_fChangeFrame;
	float m_fBlendWeight;
	int m_nBaseBlendFrame;
	int m_nBlendFrame;
	int m_nStiffFrame;
	
	PHYSICS_COLLISION_CONTAINER *m_pCollisionMesh;
	PHYSICS_RAGDOLL_CONTAINER *m_pRagdollSetting;
	bool	m_bRagdollSettingDelete;	
	int m_nRagDollMode;	
		
public:
	// ºÎ¸ð Virtual Function
	virtual void Process();
	virtual void PostProcess();
	virtual bool Render();
	virtual void NextAnimation();	
	void CheckAnimationEnd();

	int Initialize(int nSkinIndex, int nAniIndex, int nShadowMeshIndex=-1);
	void SetFrameAdd( float fFrameAdd ) { m_fFrameAdd = fFrameAdd; }
	virtual float GetFrameAdd() { return m_fFrameAdd; }
	float GetCurFrame() { return m_fFrame; }

	void SetAnimation( int nAniIndex, int nBlendFrame=6 );

	void CreateCollisionMesh(const char *pFileName);
	void SetCollisionMesh(PHYSICS_COLLISION_CONTAINER *pData);
	void CreateRagdoll(const char *pFileName);
	void SetRagdoll(PHYSICS_RAGDOLL_CONTAINER *pData);
	void RunRagdoll( int nSetting = 0 );	
	virtual void SetRagDollMode( int nMode ) { m_nRagDollMode = nMode; }
	int IsRagDollMode() { return m_nRagDollMode; }
	int GetAniIndex() { return m_nAniIndex; }

	void SetBlendFrame( int nFrame ) { m_nBlendFrame = nFrame; }
};

typedef CSmartPtr<CFcAniObject> AniObjHandle;