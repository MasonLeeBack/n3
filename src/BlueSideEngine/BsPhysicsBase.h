#pragma once

class NxActor;
class NxActorDesc;
class NxTriangleMesh;
class NxConvexMesh;
class NxJoint;
class NxJointDesc;
class NxStream;
class NxSphericalJointDesc;
class CBsObject;
class NxScene;

class IBsPhysicsActor
{
public:

	virtual ~IBsPhysicsActor() {}
	virtual NxActor* GetActor() = 0;
	virtual void SetBoneName(const char *) = 0;
	virtual char *GetBoneName() = 0;
	virtual char *GetLinkBoneName() = 0;
	virtual char *GetGroupName() = 0;
	virtual void SetKinematic(bool = 1) = 0;
	virtual void SetLinkName(std::string) = 0;
	virtual bool IsKinematic() = 0;
	virtual bool IsTrigger() = 0;
	virtual void SetWorldMat(const D3DXMATRIX &) = 0;
	virtual void SetInvWorldMat(const D3DXMATRIX &) = 0;
	virtual void SetScale(D3DXVECTOR3 ) = 0;
	virtual void SetGlobalPose(const D3DXMATRIX &) = 0;
	virtual void SetGlobalPosition(const D3DXVECTOR3 &) = 0;
	virtual void SetTransform(const D3DXMATRIX &) = 0;
	virtual void GetGlobalPose(D3DXMATRIX &) = 0;	
	virtual void GetGlobalPosition(D3DXVECTOR3 &) = 0;
	virtual void GetTransform(D3DXMATRIX &) = 0;
	virtual void SetGroup(const char *) = 0;
	virtual D3DXMATRIX GetWorldMat() = 0;	
	virtual D3DXMATRIX GetInvWorldMat() = 0;
	virtual void AddForce(D3DXVECTOR3 *) = 0;
	virtual void AddVelocity(D3DXVECTOR3 *LinearVelocity, D3DXVECTOR3 *AngularVelocity) = 0;
	virtual D3DXVECTOR3 GetVelocity() = 0;
	virtual void Release() = 0;
	virtual D3DXVECTOR3 GetBoxExtend() = 0;
	virtual void SetShapeUserData(void *) = 0;		
	virtual void SetSleep(bool bSleep) = 0;	
	//virtual void SetAutoDelete( bool bEnable ) = 0;
};

class IBsPhysicsJoint
{
public:
	virtual ~IBsPhysicsJoint() {}
	virtual void SetBreakable(float fLinear, float fAngular) = 0;
	virtual void SetLimited() = 0;
};

struct PHYSICS_JOINT_INFO;
struct PHYSICS_DATA_CONTAINER;
struct PHYSICS_COLLISION_CONTAINER;
struct PHYSICS_RAGDOLL_CONTAINER;
struct JointElem;
class NxDebugRenderable;
struct Box3;
class IBsPhysicsMgr
{
public:
	virtual ~IBsPhysicsMgr() {}
	virtual void InitPhysics() = 0;
	virtual void RunPhysics( float fDeltaTime ) = 0;
	virtual void ReleasePhysics() = 0;	
	virtual NxActor *CreateActor( NxActorDesc &ActorDesc, IBsPhysicsActor *pPhysicsActor) = 0;
	virtual NxTriangleMesh *CreateTriangleMesh( const NxStream &) = 0;
	virtual NxConvexMesh *CreateConvexMesh( const NxStream &) = 0;
	//virtual void ReleaseActor(IBsPhysicsActor *pPhysicsActor) = 0;	
	virtual IBsPhysicsJoint *CreateJoint( IBsPhysicsActor *pActor1, IBsPhysicsActor *pActor2, PHYSICS_JOINT_INFO &JointInfo, D3DXVECTOR3 &AnchorPos, int nSelect, int nLevel ) = 0;
	virtual NxJoint *CreateJoint( NxJointDesc &JointDesc ) = 0;
	//virtual void ReleaseJoint( NxJoint *Joint) = 0;
	virtual void Visualize() = 0;
	virtual void SetVisible(bool bVisible) = 0;
	virtual int FindGroupID(const char *szName) = 0;
	virtual int GenerateGroupID(const char *szName) = 0;
	virtual void SetGroupCollision(const char *szName1, const char *szName2, bool bEnable) = 0;
	virtual void AddForce(D3DXVECTOR3 *Force) = 0;
	virtual void SetEnable(bool bEnable) = 0;
	virtual void SetGravity(float fGravity) = 0;	
	virtual void Flush() = 0;
	virtual bool IsOcclusionCull( D3DXVECTOR3 viewPos, D3DXVECTOR3 vecPoint ) = 0;
	virtual void SetActiveScene(int nScene) = 0;		
	virtual void ReleaseTriangleMesh(NxTriangleMesh &) = 0;
	virtual void SetRenderData( const NxDebugRenderable &data ) = 0;
	virtual void Clear() = 0;
	virtual void SetCustomValue( int nValue ) = 0;
	virtual int GetCustomValue() = 0;

};



