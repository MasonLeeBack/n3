#pragma once

const bool UseNovodex();

//////////  ACTOR


struct PHYSICS_JOINT_INFO
{
	std::string szBoneName1;
	std::string szBoneName2;

	enum { SPHERICAL = 1, REVOLUTE = 2,};
	int nType;

	D3DXVECTOR3 GlobalAxis;
	D3DXVECTOR3 AnchorPos;
	float minAngle;	// revolute
	float maxAngle;// revolute
	bool bBreakable;
	float LinearBreak;
	float AngularBreak;
	int nLevel;
};

struct PHYSICS_DATA_CONTAINER
{
	struct ActorElem
	{
		std::string szBoneName;

		std::string szLinkBoneName;
		std::string szGroupName;
		int		nIteration;

		enum {SHAPE_SPHERE, SHAPE_CAPSULE, SHAPE_BOX};
		int	nShapeType;
		D3DXVECTOR3 Size;
		float fRadius;
		float fHeight;

		D3DXMATRIX WorldMat;
		D3DXMATRIX InverseWorldMat;
	};
	std::vector< ActorElem > ActorList;
	std::vector< PHYSICS_JOINT_INFO > JointList;
	bool	bUseBoundingBox;
	D3DXVECTOR3 BoundingBox[2];
};


struct PHYSICS_COLLISION_CONTAINER
{
	struct Elem
	{
		std::string szLinkName;
		float radius;
		float height;
		BSMATRIX matLocal;
	};
	std::vector< Elem > CollisionList;
};

struct PHYSICS_RAGDOLL_CONTAINER
{
	struct Elem
	{
		std::string szName;
		float	fRadius;
		float	fHeight;
		D3DXMATRIX matLocal;
	};
	std::vector< Elem > RagdollList;
};

#include "BsPhysicsBase.h"

class CBsNullPhyActor : public IBsPhysicsActor
{
public:
	NxActor* GetActor(){ return NULL; }
	void SetBoneName(const char *szName){}
	char *GetBoneName(){ return "NullName"; }
	char *GetLinkBoneName(){ return NULL;}
	char *GetGroupName() {return "NullName"; }
	void SetKinematic(bool bEnable = true){}
	void SetLinkName(std::string szLinkeBoneName){}
	bool IsKinematic(){ return false;}
	bool IsTrigger() {return false;}
	void SetWorldMat(const D3DXMATRIX &mat){}
	void SetInvWorldMat(const D3DXMATRIX &mat){}
	void SetScale(D3DXVECTOR3 Scale) {};
	void SetGlobalPose(const D3DXMATRIX &mat){}
	void SetGlobalPosition(const D3DXVECTOR3 &Pos) {}
	void SetTransform(const D3DXMATRIX &mat){}
	void GetGlobalPose(D3DXMATRIX &mat){D3DXMatrixIdentity(&mat);}	
	void GetGlobalPosition(D3DXVECTOR3 &Pos) {}
	void GetTransform(D3DXMATRIX &mat){D3DXMatrixIdentity(&mat);}
	void SetGroup(const char *szGroupName){}
	D3DXMATRIX GetWorldMat(){ return D3DXMATRIX();}	
	D3DXMATRIX GetInvWorldMat(){ return D3DXMATRIX();}
	void AddForce(D3DXVECTOR3 *Force){}
	void AddVelocity(D3DXVECTOR3 *LinearVelocity, D3DXVECTOR3 *AngularVelocity) {}
	D3DXVECTOR3 GetVelocity(){return D3DXVECTOR3(0,0,0);}			// 소스 컴파일이 안되서 return D3DXVECTOR3() => return D3DXVECTOR3(0,0,0) 로 수정했습니다.
	void Release(){}
	D3DXVECTOR3 GetBoxExtend(){ return D3DXVECTOR3(0,0,0); }		// 소스 컴파일이 안되서 return D3DXVECTOR3() => return D3DXVECTOR3(0,0,0) 로 수정했습니다.
	void SetShapeUserData(void *pData){}	
	void SetSleep(bool bSleep) {}	
};


class CBsPhysicsActor : public IBsPhysicsActor
{
public:	
	CBsPhysicsActor(NxActor *pActor = NULL);
	virtual ~CBsPhysicsActor();

protected:	
	NxActor				*m_pActor;	

	D3DXMATRIX		m_matWorld;
	D3DXMATRIX		m_matInvWorld;
	bool					m_bKinematic;
	bool					m_bTrigger;
	std::string			m_szBoneName;
	std::string			m_szLinkBoneName;
	D3DXVECTOR3		m_halfExtend;
	D3DXVECTOR3		m_Scale;

	std::string			m_szGroupName;
	//bool					m_bAutoDelete;

	
public:		
	NxActor* GetActor() { return m_pActor; }
	void SetBoneName(const char *szName) { m_szBoneName = szName; }
	char *GetBoneName() { return (char *)m_szBoneName.c_str(); }
	char *GetLinkBoneName() { return (char *)m_szLinkBoneName.c_str(); }
	char *GetGroupName() { return (char*)m_szGroupName.c_str();}
	void SetLinkName(std::string szLinkeBoneName) { m_szLinkBoneName = szLinkeBoneName;}
	void SetKinematic(bool bEnable = true);
	bool IsKinematic() { return m_bKinematic; }
	bool IsTrigger() {return m_bTrigger;}
	void SetWorldMat(const D3DXMATRIX &mat) { m_matWorld = mat; }
	void SetInvWorldMat(const D3DXMATRIX &mat) { m_matInvWorld  = mat; }
	void SetScale(D3DXVECTOR3 Scale) { m_Scale = Scale; }
	void SetGlobalPose(const D3DXMATRIX &mat);
	void SetGlobalPosition(const D3DXVECTOR3 &Pos);
	void SetTransform(const D3DXMATRIX &mat);
	void GetGlobalPose(D3DXMATRIX &mat);	
	void GetGlobalPosition(D3DXVECTOR3 &Pos);
	void GetTransform(D3DXMATRIX &mat);
	void SetGroup(const char *szGroupName);
	D3DXMATRIX GetWorldMat() { return m_matWorld; }	
	D3DXMATRIX GetInvWorldMat(){ return m_matInvWorld;}
	void AddForce(D3DXVECTOR3 *Force);
	void AddVelocity(D3DXVECTOR3 *LinearVelocity, D3DXVECTOR3 *AngularVelocity);
	D3DXVECTOR3 GetVelocity();
	void Release();
	D3DXVECTOR3 GetBoxExtend() { return m_halfExtend;}
	void SetShapeUserData(void *pData);		
	void SetSleep(bool bSleep);	
	//void SetAutoDelete( bool bDelete ) { m_bAutoDelete = bDelete;}
	//bool CheckAutoDelete();
};

class CBsPhysicsBone : public CBsPhysicsActor
{	
private:
	CBsPhysicsBone (float Radius, float Height, D3DXMATRIX &matLocal, D3DXMATRIX &matObject, char *szBoneName);
public:
	static IBsPhysicsActor* Create(float Radius, float Height, D3DXMATRIX &matLocal, D3DXMATRIX &matObject, char *szBoneName)
	{
		if(UseNovodex())
			return new CBsPhysicsBone(Radius, Height, matLocal, matObject, szBoneName);
		else
			return new CBsNullPhyActor;
	}
};


class CBsPhysicsBox : public CBsPhysicsActor
{
private:
	CBsPhysicsBox (D3DXVECTOR3 Size, D3DXMATRIX &matObject);
public:	
	static IBsPhysicsActor* Create(D3DXVECTOR3 Size, D3DXMATRIX &matObject)
	{
		if(UseNovodex())
			return new CBsPhysicsBox(Size, matObject);
		else
			return new CBsNullPhyActor;
	}
};

class CBsPhysicsPropBox : public CBsPhysicsActor
{
private:
	CBsPhysicsPropBox (D3DXVECTOR3 Size, D3DXMATRIX &matObject);
public:	
	static IBsPhysicsActor* Create(D3DXVECTOR3 Size, D3DXMATRIX &matObject)
	{
		if(UseNovodex())
			return new CBsPhysicsPropBox(Size, matObject);
		else
			return new CBsNullPhyActor;
	}
};

class CBsPhysicsClothBox : public CBsPhysicsActor
{
private:
	CBsPhysicsClothBox (D3DXVECTOR3 Size, D3DXMATRIX &matObject, int nMode);
public:	
	static IBsPhysicsActor* Create(D3DXVECTOR3 Size, D3DXMATRIX &matObject, int nMode = -1)
	{
		if(UseNovodex())
			return new CBsPhysicsClothBox(Size, matObject, nMode);
		else
			return new CBsNullPhyActor;
	}
};

class CBsPhysicsClothCapsule : public CBsPhysicsActor
{
private:
	CBsPhysicsClothCapsule(float Radius, float Height, D3DXMATRIX &matObject);
public:	
	static IBsPhysicsActor* Create(float Radius, float Height, D3DXMATRIX &matObject)
	{
		if(UseNovodex())
			return new CBsPhysicsClothCapsule(Radius, Height, matObject);
		else
			return new CBsNullPhyActor;
	}
};


class CBsPhysicsBoxTrigger : public CBsPhysicsActor
{
private:
	CBsPhysicsBoxTrigger (D3DXVECTOR3 Size, D3DXVECTOR3 Pivot, D3DXMATRIX &matObjects);
public:	
	static IBsPhysicsActor* Create(D3DXVECTOR3 Size, D3DXVECTOR3 Pivot, D3DXMATRIX &matObject)
	{
		if(UseNovodex())
			return new CBsPhysicsBoxTrigger(Size, Pivot, matObject);
		else
			return new CBsNullPhyActor;
	}
};

class CBsPhysicsSphere : public CBsPhysicsActor
{
private:
	CBsPhysicsSphere(float Radius, D3DXMATRIX &matObject);
public:	
	static IBsPhysicsActor* Create(float Radius, D3DXMATRIX &matObject)
	{
		if(UseNovodex())
			return new CBsPhysicsSphere(Radius, matObject);
		else
			return new CBsNullPhyActor;
	}
};


class CBsPhysicsCollisionSphere : public CBsPhysicsActor
{
private:
	CBsPhysicsCollisionSphere(float Radius, D3DXMATRIX &matObject);
public:	
	static IBsPhysicsActor* Create(float Radius, D3DXMATRIX &matObject)
	{
		if(UseNovodex())
			return new CBsPhysicsCollisionSphere(Radius, matObject);
		else
			return new CBsNullPhyActor;
	}
};

class CBsPhysicsCollisionCapsule : public CBsPhysicsActor
{
private:
	CBsPhysicsCollisionCapsule(float Radius, float Height, D3DXMATRIX &matObject);
public:	
	static IBsPhysicsActor* Create(float Radius, float Height, D3DXMATRIX &matObject)
	{
		if(UseNovodex())
			return new CBsPhysicsCollisionCapsule(Radius, Height, matObject);
		else
			return new CBsNullPhyActor;
	}
};

class CBsPhysicsStone: public CBsPhysicsActor
{
private:
	CBsPhysicsStone(float Radius, D3DXMATRIX &matObject);
public:	
	static IBsPhysicsActor* Create(float Radius, D3DXMATRIX &matObject)
	{
		if(UseNovodex())
			return new CBsPhysicsStone(Radius, matObject);
		else
			return new CBsNullPhyActor;
	}
};

class CBsPhysicsConvexMesh : public CBsPhysicsActor
{
private:
	CBsPhysicsConvexMesh(D3DXVECTOR3 *pVertexBuffer, int nVertexCount, D3DXMATRIX &matObject);
public:	
	static IBsPhysicsActor* Create(D3DXVECTOR3 *pVertexBuffer, int nVertexCount, D3DXMATRIX &matObject)
	{
		if(UseNovodex())
			return new CBsPhysicsConvexMesh(pVertexBuffer, nVertexCount, matObject);
		else
			return new CBsNullPhyActor;
	}
};

class CBsPhysicsTriangleMesh : public CBsPhysicsActor
{
private:
	NxTriangleMesh	*m_pTriangleMesh;
private:
	CBsPhysicsTriangleMesh(D3DXVECTOR3 *pVertexBuffer, int *pIndicesBuffer, int nVertexCount, int nFaceCount, D3DXMATRIX &matObject);
public:	
	virtual ~CBsPhysicsTriangleMesh();
	static IBsPhysicsActor* Create(D3DXVECTOR3 *pVertexBuffer, int *pIndicesBuffer, int nVertexCount, int nFaceCount, D3DXMATRIX &matObject)
	{
		if(UseNovodex())
			return new CBsPhysicsTriangleMesh(pVertexBuffer, pIndicesBuffer, nVertexCount, nFaceCount, matObject);
		else
			return new CBsNullPhyActor;
	}
};

class CBsPhysicsCapsule : public CBsPhysicsActor
{
private:
	CBsPhysicsCapsule(float Radius, float Height, D3DXMATRIX &matObject);
public:	
	static IBsPhysicsActor* Create(float Radius, float Height, D3DXMATRIX &matObject)
	{
		if(UseNovodex())
			return new CBsPhysicsCapsule(Radius, Height, matObject);
		else
			return new CBsNullPhyActor;
	}
};

class CBsPhysicsHeightField: public CBsPhysicsActor
{
private:
	CBsPhysicsHeightField(int nXSize, int nZSize, short int* pHeight);
public:	
	virtual ~CBsPhysicsHeightField();
	static IBsPhysicsActor* Create(int nXSize, int nZSize, short int* pHeight)
	{
		if(UseNovodex())
			return new CBsPhysicsHeightField(nXSize, nZSize, pHeight);
		else
			return new CBsNullPhyActor;
	}
};

class CBsPhysicsPlane : public CBsPhysicsActor
{
private:
	CBsPhysicsPlane(D3DXPLANE plane);
public:	
	static IBsPhysicsActor* Create(D3DXPLANE plane)
	{
		if(UseNovodex())
			return new CBsPhysicsPlane( plane );
		else
			return new CBsNullPhyActor;
	}
};

////////  JOINT

class CBsNullPhyJoint: public IBsPhysicsJoint
{
public:
	void SetBreakable(float fLinear, float fAngular){}
	void SetLimited() {}
};

class NxJoint;
class CBsPhysicsJoint : public IBsPhysicsJoint
{
public:
	CBsPhysicsJoint();
	virtual ~CBsPhysicsJoint();
protected:
	NxJoint  *m_pJoint;		
public:
	void SetBreakable(float fLinear, float fAngular);	
	virtual void SetLimited(){}
	void Release();
};

class CBsPhysicsSphericalJoint : public CBsPhysicsJoint
{
private:
	CBsPhysicsSphericalJoint(IBsPhysicsActor *pActor1, IBsPhysicsActor *pActor2, D3DXVECTOR3 &AnchorPos, int nSelect, int nLevel);
public:
	static IBsPhysicsJoint *Create(IBsPhysicsActor *pActor1, IBsPhysicsActor *pActor2, D3DXVECTOR3 &AnchorPos, int nSelect, int nLevel)
	{
		if(UseNovodex())
			return new CBsPhysicsSphericalJoint(pActor1, pActor2, AnchorPos, nSelect, nLevel);
		else
			return new CBsNullPhyJoint;
	}
};

class CBsPhysicsD6Joint : public CBsPhysicsJoint
{
private:
	CBsPhysicsD6Joint(IBsPhysicsActor *pActor1, IBsPhysicsActor *pActor2, D3DXVECTOR3 &AnchorPos, int nSelect);
public:
	virtual void SetLimited();
	static IBsPhysicsJoint *Create(IBsPhysicsActor *pActor1, IBsPhysicsActor *pActor2, D3DXVECTOR3 &AnchorPos, int nSelect)
	{
		if(UseNovodex())
			return new CBsPhysicsD6Joint(pActor1, pActor2, AnchorPos, nSelect);
		else
			return new CBsNullPhyJoint;
	}
};

class CBsPhysicsRevoulteJoint : public CBsPhysicsJoint
{
private:
	CBsPhysicsRevoulteJoint(IBsPhysicsActor *pActor1, IBsPhysicsActor *pActor2, D3DXVECTOR3 &AnchorPos, PHYSICS_JOINT_INFO &JointInfo);	
public:
	
	static IBsPhysicsJoint *Create(IBsPhysicsActor *pActor1, IBsPhysicsActor *pActor2, D3DXVECTOR3 &AnchorPos, PHYSICS_JOINT_INFO &JointInfo)
	{
		if(UseNovodex())
			return new CBsPhysicsRevoulteJoint(pActor1, pActor2, AnchorPos, JointInfo);
		else
			return new CBsNullPhyJoint;
	}
};

class CBsPhysicsRevoulteJointForTroll : public CBsPhysicsJoint
{
private:
	CBsPhysicsRevoulteJointForTroll(IBsPhysicsActor *pActor1, IBsPhysicsActor *pActor2, D3DXVECTOR3 &AnchorPos, PHYSICS_JOINT_INFO &JointInfo);	
public:

	static IBsPhysicsJoint *Create(IBsPhysicsActor *pActor1, IBsPhysicsActor *pActor2, D3DXVECTOR3 &AnchorPos, PHYSICS_JOINT_INFO &JointInfo)
	{
		if(UseNovodex())
			return new CBsPhysicsRevoulteJointForTroll(pActor1, pActor2, AnchorPos, JointInfo);
		else
			return new CBsNullPhyJoint;
	}
};

////////// GROUP


class NxActorDesc;
class NxStream;
class NxTriangleMesh;
class NxJointDesc;
class NxJoint;
class NxScene;
class NxSphericalJointDesc;
class CBsNullPhyMgr : public IBsPhysicsMgr
{
public:
	void InitPhysics() {}
	void RunPhysics( float fDeltaTime ) {}
	void ReleasePhysics() {}	
	void Clear() {}
	NxActor *CreateActor( NxActorDesc &ActorDesc, IBsPhysicsActor *pPhysicsActor) { return NULL; }
	NxTriangleMesh *CreateTriangleMesh( const NxStream &) { return NULL; }
	NxConvexMesh *CreateConvexMesh( const NxStream &) {return NULL;}
	void ReleaseTriangleMesh(NxTriangleMesh &) {}
	//void ReleaseActor(IBsPhysicsActor *pPhysicsActor) {}
	IBsPhysicsJoint *CreateJoint( IBsPhysicsActor *pActor1, IBsPhysicsActor *pActor2, PHYSICS_JOINT_INFO &JointInfo, D3DXVECTOR3 &AnchorPos, int nSelect, int nLevel) {return new CBsNullPhyJoint;}
	NxJoint *CreateJoint( NxJointDesc &JointDesc ) { return NULL; }
	//void ReleaseJoint( NxJoint *Joint) {}
	void Visualize() {}
	void SetVisible(bool bVisible) {}
	int FindGroupID(const char *szName) { return true;}
	int GenerateGroupID(const char *szName) {return true;}
	void SetGroupCollision(const char *szName1, const char *szName2, bool bEnable) {}
	void AddForce(D3DXVECTOR3 *Force) {}
	void SetEnable(bool bEnable) {}
	void SetGravity(float fGravity) {}	
	void Flush() {}
	bool IsOcclusionCull( D3DXVECTOR3 viewPos, D3DXVECTOR3 vecPoint ) { return false;}
	void SetActiveScene(int nScene){}	
	void SetRenderData( const NxDebugRenderable &data ) {}	
	void SetCustomValue( int nValue ) {}
	int GetCustomValue() { return 0;}
};

class NxPhysicsSDK;
class NxScene;
class CBsPhysicsHeightFieldZone;
class NxMaterial;
class CBsPhysicsMgr : public IBsPhysicsMgr
{
private:
	CBsPhysicsMgr();
public:	
	virtual ~CBsPhysicsMgr();

protected:

	NxPhysicsSDK	*m_pPhysicsSDK;
	
	NxScene			*m_pScene[2];
	NxScene			*m_pActiveScene;

	bool	m_bVisible;	
	bool	m_bEnable;
			
	char m_szGroupName[32][255];	
	
	std::vector< NxMaterial* > m_MaterialList[2];	

	//std::vector< CBsObject* > m_PreProcessObjList;
	
	int				m_nRenderIndex;

	//std::vector< NxActor * > m_DeleteTriggerActorList;
	//std::vector< NxActor * > m_DeleteActorList;
	//std::vector< NxJoint *> m_DeleteJointList;

	std::vector< CBsPhysicsHeightFieldZone* > m_ZoneList;

	struct DebugVertex
	{
		D3DXVECTOR3 position;
		D3DCOLOR color;
	};

	std::vector<DebugVertex> m_PointList[2];
	std::vector<DebugVertex> m_LineList[2];
	std::vector<DebugVertex> m_TriangleList[2];

	int		m_nPhysicsTick;

	int		m_nCustomValue;

public:
	enum 
	{
		CUSTOM_NONE,
		CUSTOM_MYIFEE_CHAIN,
		CUSTOM_LS_SL3_CLOTH,
		CUSTOM_LP_PM_CLOTH,
		CUSTOM_EV_IN_ASPHARR_CLOTH,
	};

public:

	void InitPhysics();
	void RunPhysics( float fDeltaTime );
	void ReleasePhysics();


	int	GetPhysicsTick() {return m_nPhysicsTick;}
	void AddZone( CBsPhysicsHeightFieldZone *pZone ) { m_ZoneList.push_back( pZone ); }	
	void Clear();
	
	NxActor *CreateActor( NxActorDesc &ActorDesc, IBsPhysicsActor *pPhysicsActor);
	NxTriangleMesh *CreateTriangleMesh( const NxStream &stream);
	NxConvexMesh *CreateConvexMesh( const NxStream &stream);
	void ReleaseTriangleMesh(NxTriangleMesh &triangleMesh );
	//void ReleaseActor(IBsPhysicsActor *pPhysicsActor);
	IBsPhysicsJoint *CreateJoint( IBsPhysicsActor *pActor1, IBsPhysicsActor *pActor2, PHYSICS_JOINT_INFO  &JointInfo, D3DXVECTOR3 &AnchorPos, int nSelect, int nLevel);
	NxJoint *CreateJoint( NxJointDesc &JointDesc );	
	//void ReleaseJoint( NxJoint *Joint );
	void Visualize();
	void SetVisible(bool bVisible) { m_bVisible = bVisible; }	
	int FindGroupID(const char *szName);
	int GenerateGroupID(const char *szName);
	void SetGroupCollision(const char *szName1, const char *szName2, bool bEnable);
	void AddForce(D3DXVECTOR3 *Force);	
	void SetEnable(bool bEnable) ;//{ m_bEnable = bEnable; }
	void SetGravity(float fGravity);
	
	void Flush();
	bool IsOcclusionCull( D3DXVECTOR3 viewPos, D3DXVECTOR3 vecPoint );
	void SetRenderData( const NxDebugRenderable &data );

	void SetActiveScene(int nScene) ;
	NxScene* GetActiveScene() { return m_pActiveScene; }

	void SetCustomValue( int nValue ) { m_nCustomValue = nValue; }
	int GetCustomValue() { return m_nCustomValue; }
	
	static IBsPhysicsMgr* Create();	
	
};
 

const float GetWorldScaleMul() ;
const float GetWorldScaleDiv() ;