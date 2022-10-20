#pragma		once
#include "BsObject.h"
#include "BsAni.h"
#include "BsImplMaterial.h"
#include "BsSaveMatrix.h"

#define MAX_LINK_COUNT		12
#define MAX_LINK_PARTICLE_COUNT		3
#define MAX_CURRENT_ANI_SIZE		5
#define MAX_BLEND_ANI_SIZE			5
#define MIN_BONE_ROTATION_COUNT		5
struct BONE_ROTATION_INFO
{
	char szBoneName[ 32 ];
	D3DXVECTOR3 Rotation;
};

class IBsPhysicsActor;
class IBsPhysicsJoint;

struct CollisionPart
{
	float fRadius;
	float fHeight;
    IBsPhysicsActor *pActor;
	BSMATRIX matLocal;
	int nBoneIndex;
};

struct PHYSICS_COLLISION_CONTAINER;
struct PHYSICS_RAGDOLL_CONTAINER;

class CBsAniObject : public CBsObject
{
public:
	CBsAniObject();
	virtual ~CBsAniObject();
	void ResetAniIndex();

	void	PopBlend();

	virtual void ReInit();
	virtual void Release();

protected:
	int		m_nAniFileIndex;
	CBsAni*	m_pAni;

	int		m_nBillBoardIndex;

	int m_nCurrentAniSize;
	int m_nCurrentAni[ MAX_CURRENT_ANI_SIZE ];
	float m_fCurrentFrame[ MAX_CURRENT_ANI_SIZE ];
	int m_nCurrentAniBone[ MAX_CURRENT_ANI_SIZE ];

	int m_nBlendAniSize;
	int m_nBlendAni[ MAX_BLEND_ANI_SIZE ];
	float m_fBlendFrame[ MAX_BLEND_ANI_SIZE ];
	float m_fBlendWeight[ MAX_BLEND_ANI_SIZE ];
	int m_nBlendAniBone[ MAX_BLEND_ANI_SIZE ];

	static CBsSaveMatrix s_GlobalSaveMatrixBuffer[ DOUBLE_BUFFERING ];

	D3DXMATRIX*			m_pSaveMatrix[ DOUBLE_BUFFERING ];

	int m_nCalcAniPosition;
	std::vector< BONE_ROTATION_INFO > m_BoneRotation;	
	//std::vector< BONE_ROTATION_INFO > m_BoneRotationProcess;

	D3DXMATRIX	m_matProcessRootBone;

	// 시뮬레이션용 변수
	bool	m_bRagdollEnable;
	bool	m_bSimulationMode;		// true = ragdoll system, false = keyframe system
	bool	m_bIsBillBoardRender[ DOUBLE_BUFFERING ];		// 거리 제한에 따라 3D Render인지 Billboard Render인지 선택!

	bool	m_bIsBillBoardRenderBefore;						// 
	// Animation LOD 관련
	int m_nAniCacheType;	//	0 : 사용하지 않음 1: BoneCount 32 이하 2: BoneCount 66이하
	
	D3DXVECTOR3 m_unlinkVelocity;
	int		m_nRagdollCount;
	std::vector< IBsPhysicsActor* >	m_BoneActors;
	std::vector< IBsPhysicsJoint* > m_Joints;
		
	std::vector < CollisionPart > m_CollisionBody;

	//*********************************************************
	//		Link 관련 멤버
	//*********************************************************
	int m_nLinkObjectCount;
	int m_nLinkObjectHandle[MAX_LINK_COUNT];
	int m_nLinkBoneIndex[MAX_LINK_COUNT];
	int m_nCatchBoneIndex;

	void LinkObjectByName(const char *pBoneName, int nObjectIndex);
	void LinkObject(int nBoneIndex, int nObjectIndex);
	const D3DXMATRIX *UnlinkObject(int nObjectIndex, D3DXVECTOR3 *unlinkVelocity = NULL);
	void UnlinkBone(int nBoneIndex);		

	int m_nLinkParticleCount;
	int m_nLinkParticleHandle[MAX_LINK_PARTICLE_COUNT];
	int m_nLinkParticleBoneIndex[MAX_LINK_PARTICLE_COUNT];
	D3DXMATRIX m_matLinkParticle[MAX_LINK_PARTICLE_COUNT];
	
	void ProcessLinkParticle();

	void LinkParticleByName(const char* pBoneName, int nParticleIndex, D3DXMATRIX* pMatrix = NULL);
	void LinkParticle(int nBoneIndex, int nParticleHandle, D3DXMATRIX* pMatrix = NULL);
	const D3DXMATRIX* UnlinkParticle(int nParticleIndex);
	void UnlinkParticleByBone(int nBoneIndex);	

	void RemoveInvalidLinkParticle();

	void ProcessLinkFX();

	void GetRagdollPosAndVelocity( D3DXVECTOR3 *pPos, D3DXVECTOR3 *pVel );
	
public:
	void SetAniFileIndex( int nIndex ) { m_nAniFileIndex = nIndex; }
	int GetAniFileIndex() { return m_nAniFileIndex; }
	void SetAniPtr( CBsAni *pAni, int nAniFileIndex );
	int GetAniLength(int nAni) { return m_pAni->GetAniLength(nAni); };
	int GetAniCount() { return m_pAni->GetAniCount(); }
	CBsAni *Get_pAni() { return m_pAni; } // mfc 의 트리 컨트롤을 만들기 위해서 사용 됩니다
	int LSFindBoneIndex(const char *pBoneName) { return m_pAni->FindBoneIndex(pBoneName); } // 립싱크용 본을 찾을때 사용 되었습니다
	void GetAniDistance(GET_ANIDISTANCE_INFO *pInfo) { m_pAni->GetAniDistance(pInfo); }
	void SetCurrentAni(SET_ANI_INFO *pInfo);
	void SetCurrentAni(int nAni, float fFrame, int nBoneIndex);
	void BlendAni(BLEND_ANI_INFO *pInfo);
	void BlendAni(int nBlendAni, float fBlendFrame, float fBlendWeight, int nBoneIndex);
	void FrontInsertBlendAni( int nBlendAni, float fBlendFrame, float fBlendWeight, int nBoneIndex );
	void SetCalcAniFlag( int nFlag ) { m_nCalcAniPosition = nFlag; }
	
	// 실제 Link와 Unlink가 발생합니다.
	void				ReqLinkObject(KERNEL_COMMAND* pCmd);
	void				ReqUnlinkObject(KERNEL_COMMAND* pCmd);
	void				ReqUnlinkBone(KERNEL_COMMAND* pCmd);

	// Simulation
	void MakeCollisionBody( PHYSICS_COLLISION_CONTAINER *pBody);
	void MakeRagdoll( PHYSICS_RAGDOLL_CONTAINER *pSetting = NULL, int nSetting = 0);
	void MakeJointRecurse(CBsBone *pBone, int nSetting );
	void SetSimulationMode( bool bSimulation );
	virtual void ProcessLinkedPhysicsObject( DWORD dwParam1 = 0, DWORD dwParam2 = 0);
	void ProcessRagdollUnlink( D3DXVECTOR3 *unlinkVelocity );
	bool InitRenderRagdoll();

	virtual void Process();	
	virtual bool InitRender(float fDistanceFromCam);
	virtual void PostCalculateAnimation();
	void SetAnimation();
	void RefreshAni();
	D3DXMATRIX *GetBoneMatrix(const char *pBoneName);
	D3DXMATRIX *GetBoneWorldMatrix(const char *pBoneName);

	virtual void Render(C3DDevice *pDevice);
	virtual void RenderAlpha(C3DDevice *pDevice);
	virtual void RenderShadow(C3DDevice* pDevice);

	void SetBoneRotation( const char *pBoneName, D3DXVECTOR3 *pRotation );
	virtual int ProcessMessage(DWORD dwCode, DWORD dwParam1=0, DWORD dwParam2=0, DWORD dwParam3=0);

	virtual bool GetBox3     ( Box3 & B );
	virtual int GetBoneCount() { return m_pAni->GetBoneCount(); }

	virtual void ObjectAlphaBlend(bool bIsEnable);

	virtual void SetObjectAlphaWeight(float fAlpha);
	virtual void RestoreAlphaBlend();

	virtual bool IsRagdoll() { return m_bRagdollEnable && m_bSimulationMode;}
	virtual void SetRagdollMatrix( bool bLocalAxis, D3DXMATRIX *pMatrix);	

	// 추가
	int		GetCurAniCount()			{	return m_nCurrentAniSize;	}
	int		GetCurAniIndex(int nIndex)	{	return m_nCurrentAni[ nIndex ];		}
	float	GetCurAniFrame(int nIndex)	{	return m_fCurrentFrame[ nIndex ];	}
	int		GetCurAniBone(int nIndex)	{	return m_nCurrentAniBone[ nIndex ];	}

	void	CurAniPopFront();
	void	RecalcAni(void);

	void AllocProcessSaveMatrixBuffer() 
	{
		m_pSaveMatrix[ m_sCurrentProcessBuffer ] = s_GlobalSaveMatrixBuffer[ m_sCurrentProcessBuffer ].AllocSaveMatrixBuffer( GetBoneCount() );
	}

	__forceinline void SaveMatrix( const D3DXMATRIX* pMatrix, int nSaveIndex )
	{
		BsAssert( m_pSaveMatrix[ m_sCurrentProcessBuffer] );
		BsAssert( nSaveIndex < GetBoneCount() );
		m_pSaveMatrix[ m_sCurrentProcessBuffer ][nSaveIndex] = *pMatrix;
	}

	bool IsValidRenderMatrix()
	{
		if( m_pSaveMatrix[ m_sCurrentRenderBuffer ] ) {
			return true;
		}
		else {
			return false;
		}
	}

	D3DXMATRIX*	GetRenderMatrix( int nIndex )
	{
		BsAssert( 0 <= nIndex && nIndex < GetBoneCount() );
		return m_pSaveMatrix[ m_sCurrentRenderBuffer ] + nIndex;
	}

	void ResetRenderMatrix()
	{
		m_pSaveMatrix[ m_sCurrentRenderBuffer ] = NULL;
	}

	void ResetProcessMatrix()
	{
		m_pSaveMatrix[ m_sCurrentProcessBuffer ] = NULL;
	}

	__forceinline D3DXMATRIX*	GetProcessMatrix( int nIndex )
	{
		BsAssert( 0 <= nIndex && nIndex < GetBoneCount() );

		return m_pSaveMatrix[ m_sCurrentProcessBuffer ] + nIndex;
	}

	static void ResetAllocSaveMatrixBuffer() { s_GlobalSaveMatrixBuffer[ m_sCurrentRenderBuffer ].ResetAlloc(); }
	static void ClearAllocSaveMatrixBuffer(int index ) { s_GlobalSaveMatrixBuffer[ index ].ResetToDefault(); }
};


/*
 *	class name	: CBsRMAniObject
 *	desc		: GetBox3()에서 에니메이션으로 갱신된 Root Bone을 가져와서 Box를 만들어준다.
 */

class CBsRMAniObject : public CBsAniObject
{
public:
	virtual bool GetBox3( Box3 & B );
};