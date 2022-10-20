#pragma once

#include "BsAniObject.h"

class CBsMesh;
struct PHYSICS_BONE_INFO;
class NxActor;
class CBsAniObject;

#define TEMP_FILENAME_CLOTH_LS_SL3 "c_ln_kb2_cloth.bm"
#define TEMP_FILENAME_CLOTH_LP_PM	"c_lp_pm1_long_shawl.bm"
#define TEMP_FILENAME_PROP_MP_CL17	"p_mp_cl17_br.bm"

class CBsClothObject : public CBsAniObject
{
public:
	CBsClothObject();
	~CBsClothObject();

	virtual void ReInit();

protected:

	std::vector< IBsPhysicsActor* > m_ActorList;
	std::vector< IBsPhysicsJoint* > m_JointList;	
	IBsPhysicsActor *m_pDummyActor;

	std::vector< D3DXMATRIX > m_SleepedActorMatrixList;
	BSVECTOR					m_SleepedBoxExtend;
	BSVECTOR					m_SleepedBoxCenter;

	std::vector<int> m_RenderBoneIndex;
	
	int		m_nLinkCount;
	int		m_nLinkToBoneIndex[50];
	int		m_nLinkFromBoneIndex[50];

	CBsAni	*m_pParentAni;

	int				m_nLastUseTick;

	bool			m_bHair;
	bool			m_bCloth;	

	D3DXVECTOR3 m_prevPos;
	D3DXVECTOR3 m_prevDir;

	bool			m_bExceptRotationCheck;

public:
	bool			m_bSleep;
	int				m_nMoveCorrectionMode;
	
public:
	virtual void Release();
		
	IBsPhysicsActor* FindActor(const char *szBoneName);
	void Create( PHYSICS_DATA_CONTAINER *pPhysicsInfo, CBsClothObject *pClothObj);
	
	bool GetBox3( Box3 &B );
	virtual int GetBoneCount() { return m_RenderBoneIndex.size(); }

	int AttachMesh( CBsMesh* pMesh ); 
	bool CreateActors( PHYSICS_DATA_CONTAINER *pPhysicsInfo);
	
	void Render(C3DDevice *pDevice);
	void RenderShadow(C3DDevice *pDevice);	

	void SetActorMatrix(char *szBoneName, D3DXMATRIX *pMatrix);

	void ProcessLinkedPhysicsObject( DWORD dwParam1 = 0, DWORD dwParam2 = 0);
	virtual bool InitRender(float fDistanceFromCamera);

	void PostInitRender();

	void LinkCharacter(int nCharIndex );
    void AddForce(D3DXVECTOR3 *Force, D3DXVECTOR3 *pPos );	
	virtual int ProcessMessage(DWORD dwCode, DWORD dwParam1/*=0*/, DWORD dwParam2/*=0*/, DWORD dwParam3/*=0*/);

	void SetShapeUserData(void *pData);
	void SetGroup(char *szName);

	void Show( bool bIsShow ); 

	void SetLastUseTick( DWORD dwTick ) { m_nLastUseTick = dwTick;}
	void SetSleep( bool bSleep );

	void MakeSleep();
	void HideCrushActors();

	void MakeActorToMesh();

	void SetMoveCorrectionCount( int nCount ) { m_nMoveCorrectionMode = nCount;}

	void SetPositionAndDirection( D3DXVECTOR3 Pos, D3DXVECTOR3 Dir) {}

	static std::vector< int > m_ClothHandleList;
	static void PostInitRenderObjects();
    	
};
