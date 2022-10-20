#include "stdafx.h"
#include "BsKernel.h"
#include "BsAniObject.h"
#include "BsPhysicsTrigger.h"
#include "BsPhysicsMgr.h"
#include "BsUserNxClass.h"
#include "BsUtil.h"

static const float cWorldScaleMul = 100.f;
static const float cWorldScaleDiv = 1.f / cWorldScaleMul;

IBsPhysicsMgr *s_pPhysicsMgr = NULL;

int nMaterialDefault = 0;
int nMaterialProp = 1;
int nMaterialIce = 2;
int nMaterialRagdoll = 3;

#ifdef _LTCG
	#define _PROCESS_THREAD_CHECK 
#else
	extern DWORD g_dwRenderThreadId;
	#define _PROCESS_THREAD_CHECK  {  if(GetCurrentThreadId()==g_dwRenderThreadId) { _DEBUGBREAK; } }
#endif

const float GetWorldScaleMul() 
{
	return cWorldScaleMul;
}

const float GetWorldScaleDiv() 
{
	return cWorldScaleDiv;
}

const bool UseNovodex() 
{
	return g_BsKernel.GetNovodexEnable();
}

CBsPhysicsActor::CBsPhysicsActor(NxActor *pActor)
{	
	m_bKinematic = false;
	m_bTrigger = false;
	m_pActor = pActor;
	m_Scale = D3DXVECTOR3(1.f, 1.f, 1.f);
}

CBsPhysicsActor::~CBsPhysicsActor()
{

	Release();
}

void CBsPhysicsActor::SetKinematic(bool bEnable)
{
	if(m_bKinematic == bEnable) return;

	if( bEnable ) {
		m_pActor->raiseBodyFlag( NX_BF_KINEMATIC );
	}
	else {
		m_pActor->clearBodyFlag( NX_BF_KINEMATIC );
	}

	m_bKinematic = bEnable;
}

void CBsPhysicsActor::SetGlobalPose(const D3DXMATRIX &mat)
{
	NxMat34 MoveMat;

	D3DXMATRIX pose(mat);
	pose._41 *= cWorldScaleDiv;
	pose._42 *= cWorldScaleDiv;
	pose._43 *= cWorldScaleDiv;

	MoveMat.setColumnMajor44((NxF32*)&pose);
	
	if(IsKinematic() )
		m_pActor->moveGlobalPose( MoveMat );
	else
		m_pActor->setGlobalPose( MoveMat );
}

void CBsPhysicsActor::SetGlobalPosition(const D3DXVECTOR3 &Pos)
{
	NxVec3 movePos = *(NxVec3*)&Pos;

	movePos.x *= cWorldScaleDiv;
	movePos.y *= cWorldScaleDiv;
	movePos.z *= cWorldScaleDiv;

	if( movePos.isFinite() ) {		
		return;
	}

	if(IsKinematic() )
		m_pActor->moveGlobalPosition( movePos );
	else
		m_pActor->setGlobalPosition( movePos );
}

void CBsPhysicsActor::GetGlobalPosition(D3DXVECTOR3 &Pos)
{
	if( m_pActor ) {
		NxVec3 pos = m_pActor->getGlobalPosition();
		Pos.x = pos.x * cWorldScaleMul;
		Pos.y = pos.y * cWorldScaleMul;
		Pos.z = pos.z * cWorldScaleMul;	
	}
	else {
		Pos = D3DXVECTOR3(0,0,0);
	}
}

void CBsPhysicsActor::GetGlobalPose(D3DXMATRIX &mat)
{
	if( m_pActor ) {
		m_pActor->getGlobalPose().getColumnMajor44( ( NxF32 * )&mat );
		mat._41 *= cWorldScaleMul;
		mat._42 *= cWorldScaleMul;
		mat._43 *= cWorldScaleMul;
	}
	else {
		D3DXMatrixIdentity( &mat );
	}
}

void CBsPhysicsActor::SetTransform(const D3DXMATRIX &mat)
{
	if( m_pActor->isSleeping() ) return;
		
	D3DXMATRIX matTrans;

	D3DXMatrixMultiply(&matTrans, &m_matWorld, &mat);	

	matTrans._41 *= cWorldScaleDiv;
	matTrans._42 *= cWorldScaleDiv;
	matTrans._43 *= cWorldScaleDiv;

	NxMat34 MoveMat;

	MoveMat.setColumnMajor44((NxF32*)&matTrans );	

	if(IsKinematic() )
		m_pActor->moveGlobalPose( MoveMat );
	else
		m_pActor->setGlobalPose( MoveMat );
}

void CBsPhysicsActor::GetTransform(D3DXMATRIX &mat)
{
	if( m_pActor ) {
		D3DXMATRIX matTrans, matScale;
		m_pActor->getGlobalPose().getColumnMajor44( ( NxF32 * )&matTrans );

		matTrans._41 *= cWorldScaleMul;
		matTrans._42 *= cWorldScaleMul;
		matTrans._43 *= cWorldScaleMul;

		D3DXMATRIX matInv = m_matInvWorld;
		matInv._41 *= m_Scale.x;
		matInv._42 *= m_Scale.y;
		matInv._43 *= m_Scale.z;

		D3DXMatrixMultiply(&mat, &matInv, &matTrans);
		D3DXMatrixScaling(&matScale, m_Scale.x, m_Scale.y, m_Scale.z);
		D3DXMatrixMultiply(&mat, &matScale, &mat);
	}
	else {
		D3DXMatrixIdentity( &mat );
	}	
}
// 관리하기 쉽도록 string으로 구분하므로
// string 마다 고유한 Group ID를 부여해줘야 한다.
void CBsPhysicsActor::SetGroup(const char *szGroupName) 
{
	int nID;
	nID = s_pPhysicsMgr->FindGroupID(szGroupName);	
	if(nID == -1) {
		DebugString("Error : Novodex Group Full!!!\n");
	}

	m_szGroupName = szGroupName;

	NxU32 nbShapes = m_pActor->getNbShapes();
	NxShape*const* shapes = m_pActor->getShapes();

	m_pActor->setGroup(nID);

	while (nbShapes--)
	{
		shapes[nbShapes]->setGroup((NxCollisionGroup) nID);
	}

}

void CBsPhysicsActor::AddForce(D3DXVECTOR3 *Force)
{
	NxVec3 force = *(NxVec3*)Force;

	if( !m_pActor || m_pActor->isDynamic() == false) return;
	if(m_pActor->readBodyFlag( NX_BF_KINEMATIC ) ) return;

	m_pActor->addForce(force * 0.01f, NX_VELOCITY_CHANGE );
}

void CBsPhysicsActor::AddVelocity(D3DXVECTOR3 *LinearVelocity, D3DXVECTOR3 *AngularVelocity)
{
	if( !m_pActor || !m_pActor->isDynamic() ) return;
	if( IsKinematic() ) return;

	if( LinearVelocity ) {	
		NxVec3 linearVel = *(NxVec3*)LinearVelocity;	
		m_pActor->setLinearVelocity(m_pActor->getLinearVelocity() + linearVel);
	}

	if( AngularVelocity ) {	
		NxVec3 angularVel  = *(NxVec3*)AngularVelocity;			
		m_pActor->setAngularVelocity(m_pActor->getAngularVelocity() + angularVel);
	}
}

D3DXVECTOR3 CBsPhysicsActor::GetVelocity()
{	
	if( !m_pActor ) {
		return D3DXVECTOR3(0,0,0);
	}
	NxVec3 vel = m_pActor->getLinearVelocity();

	D3DXVECTOR3 Vel = *(D3DXVECTOR3*)&vel;	
	return Vel;
}

void CBsPhysicsActor::SetShapeUserData(void *pData)
{
	NxShape*const* ppShape = m_pActor->getShapes();
	int nCount = m_pActor->getNbShapes();

	while(nCount--) {
		ppShape[ nCount ]->userData = pData;
	}		
}

void CBsPhysicsActor::SetSleep(bool bSleep)
{
	if( !m_pActor || !m_pActor->isDynamic() ) return;

	if(bSleep){
		if( !m_pActor->isSleeping() )
			m_pActor->putToSleep();
	}
	else {
		if(m_pActor->isSleeping()) {
			m_pActor->wakeUp(FLT_MAX);
		}
	}
}

void CBsPhysicsActor::Release()
{
	_PROCESS_THREAD_CHECK
	if( m_pActor ) {		
		((CBsPhysicsMgr*)s_pPhysicsMgr)->GetActiveScene()->releaseActor(*m_pActor);
		/*
		s_pPhysicsMgr->ReleaseActor( this );
		*/
		m_pActor = NULL;
	}
}


CBsPhysicsBone::CBsPhysicsBone (float Radius, float Height, D3DXMATRIX &matLocal, D3DXMATRIX &matObject, char *szBoneName)
{

	D3DXMATRIX LocalMat = matLocal, ObjectMat = matObject;

	D3DXVECTOR3 Scale;

	Scale.x = D3DXVec3Length((D3DXVECTOR3*)&LocalMat._11);
	Scale.y = D3DXVec3Length((D3DXVECTOR3*)&LocalMat._21);
	Scale.z = D3DXVec3Length((D3DXVECTOR3*)&LocalMat._31);

	D3DXMATRIX matInvScale;
	D3DXMatrixScaling(&matInvScale, 1 / Scale.x, 1 / Scale.y, 1 / Scale.z);
	D3DXMatrixMultiply(&LocalMat, &matInvScale, &LocalMat);


	Scale.x = D3DXVec3Length((D3DXVECTOR3*)&ObjectMat._11);
	Scale.y = D3DXVec3Length((D3DXVECTOR3*)&ObjectMat._21);
	Scale.z = D3DXVec3Length((D3DXVECTOR3*)&ObjectMat._31);

	D3DXMatrixScaling(&matInvScale, 1 / Scale.x, 1 / Scale.y, 1 / Scale.z);
	D3DXMatrixMultiply(&ObjectMat, &matInvScale, &ObjectMat);

	NxCapsuleShapeDesc capsuleDesc;
	capsuleDesc.skinWidth = 0.05f;
	capsuleDesc.radius = Radius * cWorldScaleDiv;
	capsuleDesc.height = Height * cWorldScaleDiv;	
	capsuleDesc.materialIndex = nMaterialRagdoll;
	ConvertMatrix(capsuleDesc.localPose, LocalMat, cWorldScaleDiv);

	NxActorDesc actorDesc;
	actorDesc.shapes.pushBack(&capsuleDesc);
	actorDesc.density = 10.f;
	ConvertMatrix(actorDesc.globalPose, ObjectMat, cWorldScaleDiv);

	NxBodyDesc bodyDesc;

	bodyDesc.solverIterationCount = 20;
	//bodyDesc.linearDamping = 3.6f;
	bodyDesc.angularDamping = 0.8f;
	bodyDesc.linearDamping = 0.7f;
	//bodyDesc.angularDamping = 0.05f;
	bodyDesc.sleepLinearVelocity = 0.f;
	bodyDesc.sleepAngularVelocity = 0.f;
	actorDesc.body = &bodyDesc;	
	//actorDesc.flags |= NX_AF_DISABLE_COLLISION;

	BsAssert(actorDesc.isValid());
	m_pActor = s_pPhysicsMgr->CreateActor(actorDesc, this);
	m_pActor->setName( szBoneName );

	m_halfExtend = D3DXVECTOR3(Radius, Height*0.5f, Radius);
}


CBsPhysicsClothBox::CBsPhysicsClothBox (D3DXVECTOR3 Size, D3DXMATRIX &matObject, int nMode)
{
	NxBoxShapeDesc BoxDesc;

	BoxDesc.dimensions.set( Size.x * cWorldScaleDiv, Size.y * cWorldScaleDiv, Size.z * cWorldScaleDiv);				
	BoxDesc.materialIndex = nMaterialIce;
	BoxDesc.skinWidth = 0.03f;

	NxActorDesc actorDesc;

	actorDesc.shapes.pushBack( &BoxDesc );

	NxBodyDesc bodyDesc;
	bodyDesc.linearDamping = 15.f;// 파라미터 대부분 수정 불가
	bodyDesc.solverIterationCount = 1;
	bodyDesc.sleepAngularVelocity = 0.f;//2.f;
	bodyDesc.sleepLinearVelocity = 0.f;//2.f;
	bodyDesc.maxAngularVelocity = FLT_MAX;
	bodyDesc.angularDamping = 0.0f;

	if( nMode == 1) {
		bodyDesc.flags |= NX_BF_DISABLE_GRAVITY;
		bodyDesc.linearDamping = 5.f;	
	}

	actorDesc.body = &bodyDesc;
	actorDesc.density = 5.f;	

	//actorDesc
	ConvertMatrix(actorDesc.globalPose, matObject, cWorldScaleDiv);

	m_pActor = s_pPhysicsMgr->CreateActor(actorDesc, this);

	m_halfExtend = Size;
}

CBsPhysicsClothCapsule::CBsPhysicsClothCapsule(float Radius, float Height, D3DXMATRIX &matObject) 
{

	NxCapsuleShapeDesc capsuleDesc;

	capsuleDesc.radius = Radius *cWorldScaleDiv;
	capsuleDesc.height = BsMax(0.02f, (Height - Radius*2)) * cWorldScaleDiv ;
	capsuleDesc.materialIndex = nMaterialIce;
	capsuleDesc.skinWidth = 0.01f;

	NxActorDesc actorDesc;
	actorDesc.shapes.pushBack(&capsuleDesc);

	NxBodyDesc bodyDesc;
	bodyDesc.linearDamping = 15.f;// 파라미터 대부분 수정 불가
	bodyDesc.solverIterationCount = 1;
	bodyDesc.sleepAngularVelocity = 0.f;//2.f;
	bodyDesc.sleepLinearVelocity = 0.f;//2.f;
	bodyDesc.maxAngularVelocity = FLT_MAX;
	bodyDesc.angularDamping = 0.0f;
	actorDesc.body = &bodyDesc;
	actorDesc.density = 10.f;

	ConvertMatrix( actorDesc.globalPose, matObject, cWorldScaleDiv);

	m_pActor = s_pPhysicsMgr->CreateActor(actorDesc, this);

	m_halfExtend = D3DXVECTOR3(Radius, Height*0.5f, Radius);

	//SetAutoDelete( true );
}


CBsPhysicsBoxTrigger::CBsPhysicsBoxTrigger (D3DXVECTOR3 Size, D3DXVECTOR3 Pivot, D3DXMATRIX &matObject)
{
	NxBoxShapeDesc BoxDesc;
	BoxDesc.dimensions.set( Size.x * cWorldScaleDiv, Size.y * cWorldScaleDiv, Size.z * cWorldScaleDiv);
	BoxDesc.shapeFlags |= NX_TRIGGER_ENABLE;
	BoxDesc.localPose.t = NxVec3(Pivot.x * cWorldScaleDiv, Pivot.y * cWorldScaleDiv, Pivot.z * cWorldScaleDiv);

	NxActorDesc actorDesc;
	actorDesc.shapes.pushBack( &BoxDesc );
	actorDesc.body = NULL;
	actorDesc.density = 0;
	ConvertMatrix(actorDesc.globalPose, matObject, cWorldScaleDiv);
	BsAssert( actorDesc.isValid() );
	m_pActor = s_pPhysicsMgr->CreateActor(actorDesc, this);
	m_bTrigger = true;

	m_halfExtend = Size;
}

CBsPhysicsCollisionSphere::CBsPhysicsCollisionSphere(float Radius, D3DXMATRIX &matObject) 
{
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	NxSphereShapeDesc sphereDesc;
	sphereDesc.materialIndex = nMaterialIce;
	float fRadius = Radius * cWorldScaleDiv;
	sphereDesc.radius = fRadius;
	actorDesc.shapes.pushBack(&sphereDesc);

	actorDesc.body = &bodyDesc;
	actorDesc.density = 10.f;	
	ConvertMatrix(actorDesc.globalPose, matObject, cWorldScaleDiv);

	m_pActor = s_pPhysicsMgr->CreateActor(actorDesc, this);;
	m_halfExtend = D3DXVECTOR3(Radius, Radius, Radius);
}


CBsPhysicsCollisionCapsule::CBsPhysicsCollisionCapsule(float Radius, float Height, D3DXMATRIX &matObject) 
{
	NxCapsuleShapeDesc capsuleDesc;

	capsuleDesc.radius = Radius *cWorldScaleDiv;
	capsuleDesc.height = BsMax(0.f, (Height - Radius*2)) * cWorldScaleDiv ;
	capsuleDesc.materialIndex = nMaterialIce;

	NxActorDesc actorDesc;
	actorDesc.shapes.pushBack(&capsuleDesc);
	NxBodyDesc bodyDesc;
	actorDesc.body = &bodyDesc;
	actorDesc.density = 10.f;

	ConvertMatrix( actorDesc.globalPose, matObject, cWorldScaleDiv);

	m_pActor = s_pPhysicsMgr->CreateActor(actorDesc, this);

	m_halfExtend = D3DXVECTOR3(Radius, Height*0.5f, Radius);
}


// 프랍용 액터들
CBsPhysicsBox::CBsPhysicsBox (D3DXVECTOR3 Size, D3DXMATRIX &matObject)
{
	NxBoxShapeDesc BoxDesc;	

	BoxDesc.dimensions.set( Size.x * cWorldScaleDiv, Size.y * cWorldScaleDiv, Size.z * cWorldScaleDiv);				
	BoxDesc.materialIndex = nMaterialProp;
	BoxDesc.skinWidth = 0.05f;
	
	NxActorDesc actorDesc;

	actorDesc.shapes.pushBack( &BoxDesc );

	NxBodyDesc bodyDesc;
	bodyDesc.linearDamping = 0.65f;
	bodyDesc.angularDamping = 1.0f;

	bodyDesc.solverIterationCount = 1;
	actorDesc.body = &bodyDesc;
	actorDesc.density = 10.f;		

	//actorDesc
	ConvertMatrix(actorDesc.globalPose, matObject, cWorldScaleDiv);

	m_pActor = s_pPhysicsMgr->CreateActor(actorDesc, this);

	m_halfExtend = Size;

	//SetAutoDelete( true );
}

CBsPhysicsPropBox::CBsPhysicsPropBox (D3DXVECTOR3 Size, D3DXMATRIX &matObject)
{
	NxBoxShapeDesc BoxDesc;	

	BoxDesc.dimensions.set( Size.x * cWorldScaleDiv, Size.y * cWorldScaleDiv, Size.z * cWorldScaleDiv);				
	BoxDesc.materialIndex = nMaterialProp;
	BoxDesc.skinWidth = 0.05f;

	NxActorDesc actorDesc;

	actorDesc.shapes.pushBack( &BoxDesc );

	NxBodyDesc bodyDesc;
	bodyDesc.linearDamping = 0.05f;
	bodyDesc.angularDamping = 0.03f;
	
	bodyDesc.solverIterationCount = 4;
	actorDesc.body = &bodyDesc;
	actorDesc.density = 10.f;		
    
	//actorDesc
	ConvertMatrix(actorDesc.globalPose, matObject, cWorldScaleDiv);

	m_pActor = s_pPhysicsMgr->CreateActor(actorDesc, this);

	m_halfExtend = Size;

	//SetAutoDelete( true );
}

CBsPhysicsSphere::CBsPhysicsSphere(float Radius, D3DXMATRIX &matObject) 
{
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	NxSphereShapeDesc sphereDesc;
	float fRadius = Radius * cWorldScaleDiv;
	sphereDesc.radius = fRadius;
	sphereDesc.materialIndex = nMaterialProp;
	sphereDesc.skinWidth = 0.05f;
	actorDesc.shapes.pushBack(&sphereDesc);

	bodyDesc.linearDamping = 0.65f;
	bodyDesc.angularDamping = 1.0f;
	bodyDesc.solverIterationCount = 4;	
	bodyDesc.sleepAngularVelocity *= 2.f;
	bodyDesc.sleepLinearVelocity *= 2.f;

	actorDesc.body = &bodyDesc;
	actorDesc.density = 10.f;	
	ConvertMatrix(actorDesc.globalPose, matObject, cWorldScaleDiv);

	m_pActor = s_pPhysicsMgr->CreateActor(actorDesc, this);;
	m_halfExtend = D3DXVECTOR3(Radius, Radius, Radius);

	//SetAutoDelete( true );
}

CBsPhysicsCapsule::CBsPhysicsCapsule(float Radius, float Height, D3DXMATRIX &matObject) 
{

	NxCapsuleShapeDesc capsuleDesc;

	capsuleDesc.radius = Radius *cWorldScaleDiv;
	capsuleDesc.height = BsMax(0.02f, (Height - Radius*2)) * cWorldScaleDiv ;
	capsuleDesc.materialIndex = nMaterialProp;
	capsuleDesc.skinWidth = 0.05f;
		
	NxActorDesc actorDesc;
	actorDesc.shapes.pushBack(&capsuleDesc);

	NxBodyDesc bodyDesc;
	bodyDesc.linearDamping = 0.05f;
	bodyDesc.angularDamping = 0.03f;
	bodyDesc.solverIterationCount = 4;	
	actorDesc.body = &bodyDesc;
	actorDesc.density = 10.f;

	ConvertMatrix( actorDesc.globalPose, matObject, cWorldScaleDiv);

	m_pActor = s_pPhysicsMgr->CreateActor(actorDesc, this);

	m_halfExtend = D3DXVECTOR3(Radius, Height*0.5f, Radius);

	//SetAutoDelete( true );
}


CBsPhysicsStone::CBsPhysicsStone(float Radius, D3DXMATRIX &matObject) 
{
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	NxSphereShapeDesc sphereDesc;
	float fRadius = Radius * 0.87f *  cWorldScaleDiv;
	sphereDesc.radius = fRadius;
	sphereDesc.materialIndex = nMaterialDefault;
	actorDesc.shapes.pushBack(&sphereDesc);

	float fScale = powf( Radius * Radius / 3.f, 0.5f) * cWorldScaleDiv;
	NxBoxShapeDesc boxDesc1;
	boxDesc1.dimensions.set( fScale, fScale , fScale);	
	actorDesc.shapes.pushBack(&boxDesc1);

	bodyDesc.sleepLinearVelocity = 0.8f;
	bodyDesc.sleepAngularVelocity = 0.6f;
	//bodyDesc.angularDamping = 0.2f;
	//bodyDesc.linearDamping = 0.2f;
	bodyDesc.solverIterationCount = 1;

	actorDesc.body = &bodyDesc;
	actorDesc.density = 10.f;	
	ConvertMatrix(actorDesc.globalPose, matObject, cWorldScaleDiv);

	m_pActor = s_pPhysicsMgr->CreateActor(actorDesc, this);;
	m_halfExtend = D3DXVECTOR3(Radius, Radius, Radius);
}

CBsPhysicsConvexMesh::CBsPhysicsConvexMesh(D3DXVECTOR3 *pVertexBuffer, int nVertexCount, D3DXMATRIX &matObject)
{
	for(int i=0; i<nVertexCount; i++) {
		D3DXVec3Scale(&pVertexBuffer[ i ], &pVertexBuffer[ i ], cWorldScaleDiv);
	}

	NxConvexMeshDesc convexDesc;
	convexDesc.numVertices			= nVertexCount;
	convexDesc.pointStrideBytes		= sizeof( D3DXVECTOR3 );
	convexDesc.points				= pVertexBuffer;	
	convexDesc.flags				= NX_CF_COMPUTE_CONVEX;

	NxConvexShapeDesc convexShapeDesc;
	convexShapeDesc.materialIndex	= nMaterialProp;
	convexShapeDesc.skinWidth = 0.5f;

	// Cooking from memory
	MemoryWriteBuffer buf;
	bool bResult = NxCookConvexMesh(convexDesc, buf);
	BsAssert( bResult );
	convexShapeDesc.meshData = s_pPhysicsMgr->CreateConvexMesh( MemoryReadBuffer(buf.data) );	
	// Release 해주어야 됨..

	BsAssert( convexShapeDesc.meshData != NULL ) ;
	
	NxActorDesc actorDesc;
	actorDesc.shapes.pushBack(&convexShapeDesc);
	NxBodyDesc bodyDesc;
	actorDesc.body			= &bodyDesc;
	actorDesc.density		= 10.f;	
	ConvertMatrix(actorDesc.globalPose, matObject, cWorldScaleDiv);
	m_pActor = s_pPhysicsMgr->CreateActor( actorDesc, this );

	m_halfExtend = D3DXVECTOR3(1000.f, 1000.f, 1000.f);
}

CBsPhysicsTriangleMesh::CBsPhysicsTriangleMesh(D3DXVECTOR3 *pVertexBuffer, int *pIndicesBuffer, int nVertexCount, int nFaceCount, D3DXMATRIX &matObject)
{
	for(int i=0; i<nVertexCount; i++) {
		D3DXVec3Scale(&pVertexBuffer[ i ], &pVertexBuffer[ i ], cWorldScaleDiv);
	}

	// Build physical model
	NxTriangleMeshDesc triangleDesc;
	triangleDesc.numVertices				= nVertexCount;
	triangleDesc.numTriangles				= nFaceCount;
	triangleDesc.pointStrideBytes			= sizeof( D3DXVECTOR3 );
	triangleDesc.triangleStrideBytes		= 3 * sizeof( int );
	triangleDesc.points						= pVertexBuffer;
	triangleDesc.triangles					= pIndicesBuffer;
	triangleDesc.flags							= 0;

	// Cooking from memory
	MemoryWriteBuffer buf;
	bool bResult = NxCookTriangleMesh( triangleDesc, buf );
	BsAssert( bResult );

	m_pTriangleMesh = s_pPhysicsMgr->CreateTriangleMesh( MemoryReadBuffer(buf.data) );

	// Release 해주어야 됨..
	NxTriangleMeshShapeDesc triangleShapeDesc;
	triangleShapeDesc.meshData	= m_pTriangleMesh;
	triangleShapeDesc.materialIndex	= nMaterialProp;
	triangleShapeDesc.skinWidth = 0.1f;
	BsAssert( triangleShapeDesc.meshData != NULL );

	NxActorDesc actorDesc;
	actorDesc.shapes.pushBack( &triangleShapeDesc );

	NxBodyDesc bodyDesc;
	actorDesc.body			= &bodyDesc;
	actorDesc.density		= 10.f;

	ConvertMatrix(actorDesc.globalPose, matObject, cWorldScaleDiv);

	m_pActor = s_pPhysicsMgr->CreateActor( actorDesc, this );
	m_halfExtend = D3DXVECTOR3(1000.f, 1000.f, 1000.f);
}

CBsPhysicsTriangleMesh::~CBsPhysicsTriangleMesh()
{
	Release();
	s_pPhysicsMgr->ReleaseTriangleMesh( *m_pTriangleMesh );
}

class CBsPhysicsHeightFieldZone : public CBsPhysicsTriggerBase
{
protected:
	NxTriangleMeshDesc m_HFDesc;
	NxActor *m_pActor;
	NxTriangleMesh *m_pTriangleMesh;

	bool m_bCrush;
	bool m_bCreated;
	int m_nLastUseTick;

public:
	CBsPhysicsHeightFieldZone( NxTriangleMeshDesc HFDesc ) : m_HFDesc(HFDesc), m_pActor(NULL), m_bCreated(false), m_bCrush(false), m_pTriangleMesh(NULL), m_nLastUseTick(0) {;}
	virtual ~CBsPhysicsHeightFieldZone() {
		Clear();
		if( m_HFDesc.triangles ) {
			delete [] (NxU32*) m_HFDesc.triangles;
			m_HFDesc.triangles = NULL;			
		}
		if( m_HFDesc.points ) {
			delete [] (NxVec3*) m_HFDesc.points;		
			m_HFDesc.points = NULL;			
		}
	}
	virtual void OnTrigger() {
		if( !m_bCreated ) {
			m_bCrush = true;
		}
		m_nLastUseTick = ((CBsPhysicsMgr*)s_pPhysicsMgr)->GetPhysicsTick();
	}
	int GetLastUseTick() { return m_nLastUseTick; }
	bool IsCrush() { return m_bCrush;}
	void Clear() {
		if( m_pActor ) {
			(m_pActor->getScene()).releaseActor(*m_pActor);
			m_pActor = NULL;
			s_pPhysicsMgr->ReleaseTriangleMesh( *m_pTriangleMesh );
		}
	}
	bool IsCreated() { return m_bCreated; }
	void UnLoad() {
		BsAssert( m_pActor != NULL);
		Clear();
		m_bCreated = false;
	}

	void CreateZone() {
		BsAssert( m_pActor == NULL);
		m_bCreated = true;

		NxTriangleMeshShapeDesc HFShapeDesc;

		MemoryWriteBuffer OutputMeshStream;

		NxCookTriangleMesh(m_HFDesc, OutputMeshStream);

		MemoryReadBuffer InputMeshStream(OutputMeshStream.data);

		m_pTriangleMesh = s_pPhysicsMgr->CreateTriangleMesh( InputMeshStream );
		HFShapeDesc.meshData = m_pTriangleMesh;
		HFShapeDesc.meshFlags = NX_MESH_SMOOTH_SPHERE_COLLISIONS;
		HFShapeDesc.materialIndex = nMaterialProp;

		NxActorDesc actorDesc;
		actorDesc.body = NULL;
		actorDesc.density = 0;
		actorDesc.shapes.pushBack( &HFShapeDesc );
		m_pActor = s_pPhysicsMgr->CreateActor( actorDesc, NULL );


		
		int nID;
		nID = s_pPhysicsMgr->FindGroupID("HEIGHT_FIELD");
		BsAssert(nID != -1);
		NxU32 nbShapes = m_pActor->getNbShapes();
		NxShape*const* shapes = m_pActor->getShapes();
		m_pActor->setGroup(nID);
		while (nbShapes--)
		{
			shapes[nbShapes]->setGroup((NxCollisionGroup) nID);
		}
		
		//ReleaseTriggerActor();
		m_bCrush = false;
	}
};

CBsPhysicsHeightField::CBsPhysicsHeightField(int nXSize, int nZSize, short int* pHeight) 
{
	int i, j, m, n, nBufIndex;
	const int nDevideSize =	24;

	NxActorDesc actorDesc;

	for( i = 0; i <= (nZSize-1)/nDevideSize; i++ )
	{
		for( j = 0; j <= (nXSize-1)/nDevideSize; j++ )
		{
			int nZStartIndex = i * nDevideSize;
			int nXStartIndex = j * nDevideSize;

			int nXSecSize = BsMin(nDevideSize, (nXSize - nXStartIndex));
			int nZSecSize = BsMin(nDevideSize, (nZSize - nZStartIndex));

			int nVertexCount = ( nXSecSize + 1 ) * ( nZSecSize + 1 );
			int nFaceCount = nXSecSize * nZSecSize * 2;

			NxVec3 *pVerts = new NxVec3[ nVertexCount ];
			NxU32 *pFaces = new NxU32[ nFaceCount * 3 ];

			nBufIndex = 0;

			for( m = 0; m < nZSecSize; m++ )
			{
				for( n = 0; n < nXSecSize; n++ )
				{
					pFaces[ nBufIndex ] = m * ( nXSecSize + 1 ) + n;
					pFaces[ nBufIndex + 1 ] = ( m + 1 ) * ( nXSecSize + 1 ) + n;
					pFaces[ nBufIndex + 2 ] = ( m + 1 ) * ( nXSecSize + 1 ) + n + 1;

					pFaces[ nBufIndex + 3 ] = m * ( nXSecSize + 1 ) + n;
					pFaces[ nBufIndex + 4 ] = ( m + 1 ) * ( nXSecSize + 1 ) + n + 1;
					pFaces[ nBufIndex + 5 ] =  m * ( nXSecSize + 1 ) + n + 1;
					nBufIndex += 6;
				}
			}

			NxTriangleMeshDesc HFDesc;

			HFDesc.numVertices = nVertexCount;
			HFDesc.numTriangles = nFaceCount;
			HFDesc.pointStrideBytes = sizeof( NxVec3 );
			HFDesc.triangleStrideBytes = 3 * sizeof( NxU32 );
			HFDesc.points = pVerts;
			HFDesc.triangles = pFaces;
			HFDesc.flags = 0;

			HFDesc.heightFieldVerticalAxis = NX_Y;
			HFDesc.heightFieldVerticalExtent = -2000.0;

			D3DXVECTOR3 vMin(FLT_MAX, FLT_MAX, FLT_MAX);
			D3DXVECTOR3 vMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);

			for( m = 0; m <= nZSecSize; m++ )
			{
				for( n = 0; n <= nXSecSize; n++ )
				{
					nBufIndex = m * ( nXSecSize + 1 ) + n;
					int nHeightIndex = (m + nZStartIndex) * ( nXSize + 1 ) + (n + nXStartIndex);

					NxVec3 Pos( (n + nXStartIndex) * 200.0f , pHeight[ nHeightIndex ], (m + nZStartIndex) * 200.0f );

					pVerts[ nBufIndex ] = Pos * cWorldScaleDiv;

					if(vMin.x > Pos.x) vMin.x = Pos.x;
					if(vMin.y > Pos.y) vMin.y = Pos.y;
					if(vMin.z > Pos.z) vMin.z = Pos.z;
					if(vMax.x < Pos.x) vMax.x = Pos.x;
					if(vMax.y < Pos.y) vMax.y = Pos.y;
					if(vMax.z < Pos.z) vMax.z = Pos.z;
				}
			}

			CBsPhysicsHeightFieldZone *pZone = new CBsPhysicsHeightFieldZone(HFDesc);

			D3DXMATRIX mat;
			D3DXMatrixTranslation(&mat, ((vMax.x+vMin.x)*0.5f), ((vMax.y+vMin.y)*0.5f), ((vMax.z+vMin.z)*0.5f));
			pZone->CreateTriggerBox( (vMax-vMin)*0.5f + D3DXVECTOR3(100,100,100), D3DXVECTOR3(0,0,0),  mat);						
			((CBsPhysicsMgr*)s_pPhysicsMgr)->AddZone(pZone);

		}
	}
}

CBsPhysicsHeightField::~CBsPhysicsHeightField( ) 
{
	//((CBsPhysicsMgr*)s_pPhysicsMgr)->ClearZone();	
}

CBsPhysicsPlane::CBsPhysicsPlane(D3DXPLANE plane)
{
	NxPlaneShapeDesc planeDesc;
	planeDesc.d = plane.d * cWorldScaleDiv;
	planeDesc.normal = NxVec3(plane.a, plane.b, plane.c);

	NxActorDesc actorDesc;
	actorDesc.shapes.pushBack( &planeDesc );
	actorDesc.body = NULL;
	actorDesc.density = 0;

	m_pActor = s_pPhysicsMgr->CreateActor(actorDesc, this);
}

CBsPhysicsJoint::CBsPhysicsJoint()
{
	m_pJoint = NULL;	
}

CBsPhysicsJoint::~CBsPhysicsJoint()
{
	Release();
}

void CBsPhysicsJoint::Release()
{
	_PROCESS_THREAD_CHECK
	((CBsPhysicsMgr*)s_pPhysicsMgr)->GetActiveScene()->releaseJoint( *m_pJoint );
	m_pJoint = NULL;
}

void CBsPhysicsJoint::SetBreakable(float fLinear, float fAngular)
{	
	m_pJoint->setBreakable(fLinear*3.f, fAngular);
}

CBsPhysicsSphericalJoint::CBsPhysicsSphericalJoint(IBsPhysicsActor *pActor1, IBsPhysicsActor *pActor2, D3DXVECTOR3 &AnchorPos,  int nSelect, int nLevel)
{
	// mruete: prefix bug 359: added assert, and more asserts later
	// also caching the NxActors and asserting up front, since all cases need them.
	// This saves on virtual function calls as well as removing some redundant code
	BsAssert( NULL != pActor1 && NULL != pActor2 );
	NxActor * pActor1Nx = pActor1->GetActor();
	NxActor * pActor2Nx = pActor2->GetActor();
	BsAssert( NULL != pActor1Nx && NULL != pActor2Nx );

	if( nSelect == 3  ) {	// hair

		NxD6JointDesc d6Desc;

		d6Desc.actor[0] = pActor1Nx;
		d6Desc.actor[1] = pActor2Nx;

		NxVec3 JointPos = *(NxVec3*) &AnchorPos;
		JointPos.multiply( cWorldScaleDiv, JointPos);
		d6Desc.setGlobalAnchor( JointPos );
		NxVec3 localAxis = pActor2Nx->getGlobalPosition() - pActor1Nx->getGlobalPosition();
		localAxis.normalize();
		d6Desc.setGlobalAxis( localAxis );		

		d6Desc.xMotion = NX_D6JOINT_MOTION_LOCKED;
		d6Desc.yMotion = NX_D6JOINT_MOTION_LOCKED;
		d6Desc.zMotion = NX_D6JOINT_MOTION_LOCKED;
		d6Desc.twistMotion = NX_D6JOINT_MOTION_LOCKED;
		d6Desc.swing1Motion = NX_D6JOINT_MOTION_LOCKED;
		d6Desc.swing2Motion = NX_D6JOINT_MOTION_LOCKED;

		d6Desc.projectionMode = NX_JPM_POINT_MINDIST;

		d6Desc.projectionDistance = 0.015f;

		float fAngleScale = nLevel / 5.f;
		fAngleScale = BsMax( 0.f, BsMin( 2.f, fAngleScale ));

		d6Desc.projectionAngle *= (10.f - (fAngleScale * 4.9f));

		m_pJoint = s_pPhysicsMgr->CreateJoint( d6Desc );
	}
	else if (nSelect == 4 ) {	// cloth

		NxSphericalJointDesc JointDesc;

		AnchorPos *= cWorldScaleDiv;	
		NxVec3 JointPos = *(NxVec3*) &AnchorPos;

		JointDesc.actor[0] = pActor1Nx;
		JointDesc.actor[1] = pActor2Nx;

		float fAngleScale = ( nLevel - 11.f ) / 2.f;
		fAngleScale = BsMax( 0.f, BsMin( 1.f, fAngleScale ));

		JointDesc.swingLimit.value = ( 0.1f - 0.05f * fAngleScale) *NxPi;
		JointDesc.swingLimit.restitution = 0;
		JointDesc.swingLimit.hardness = 0;

		JointDesc.twistLimit.low.value = -0.01f*NxPi;
		JointDesc.twistLimit.low.restitution = 0;
		JointDesc.twistLimit.low.hardness = 0;

		JointDesc.twistLimit.high.value = 0.01f*NxPi;
		JointDesc.twistLimit.high.restitution = 0;
		JointDesc.twistLimit.high.hardness = 0;

		JointDesc.swingSpring.spring = 10.f + fAngleScale * 90.f;
		JointDesc.swingSpring.damper = 10.f;

		JointDesc.twistSpring.spring =  100.f;
		JointDesc.twistSpring.damper = 10.f;


		JointDesc.projectionMode = NX_JPM_POINT_MINDIST;
		JointDesc.projectionDistance = 0.015f;

		JointDesc.flags =  NX_SJF_SWING_SPRING_ENABLED | NX_SJF_SWING_LIMIT_ENABLED|  NX_SJF_TWIST_LIMIT_ENABLED | NX_SJF_TWIST_SPRING_ENABLED;

		int nCustom = s_pPhysicsMgr->GetCustomValue();

		if( !pActor1->IsKinematic() && nLevel <= 10 &&  nCustom != CBsPhysicsMgr::CUSTOM_LP_PM_CLOTH ) {
			float fAngleScaleLow = nLevel / 10.f;
			JointDesc.swingLimit.value = (0.06f - 0.05f * fAngleScaleLow ) * NxPi;
			JointDesc.twistLimit.low.value = -0.001f*NxPi;
			JointDesc.twistLimit.high.value = 0.001f*NxPi;
			JointDesc.projectionDistance = 0.01f;

			JointDesc.twistSpring.spring =  0.1f;

			JointDesc.flags =  NX_SJF_SWING_LIMIT_ENABLED|  NX_SJF_TWIST_LIMIT_ENABLED | NX_SJF_TWIST_SPRING_ENABLED;
		}

		if( nCustom == CBsPhysicsMgr::CUSTOM_LS_SL3_CLOTH  )
		{
			JointDesc.swingLimit.value = 0.1f * NxPi;
			JointDesc.twistLimit.low.value = -0.01f*NxPi;
			JointDesc.twistLimit.high.value = 0.01f*NxPi;
			JointDesc.projectionDistance = 0.01f;
			JointDesc.twistSpring.spring =  0.1f;
			JointDesc.flags =  NX_SJF_SWING_LIMIT_ENABLED|  NX_SJF_TWIST_LIMIT_ENABLED | NX_SJF_TWIST_SPRING_ENABLED;
		}

		NxVec3 localAxis = pActor2Nx->getGlobalPosition() - pActor1Nx->getGlobalPosition();
		localAxis.normalize();
		JointDesc.setGlobalAxis( localAxis );						
		JointDesc.setGlobalAnchor( JointPos );	

		m_pJoint = s_pPhysicsMgr->CreateJoint(JointDesc);

	}
	else {	//prop
		NxSphericalJointDesc JointDesc;

		AnchorPos *= cWorldScaleDiv;	
		NxVec3 JointPos = *(NxVec3*) &AnchorPos;
		//
		JointDesc.actor[0] = pActor1Nx;
		JointDesc.actor[1] = pActor2Nx;

		JointDesc.swingLimit.value = 0.15f*NxPi;
		JointDesc.swingLimit.restitution = 0;

		JointDesc.twistLimit.low.value = -0.05f*NxPi;
		JointDesc.twistLimit.low.restitution = 0;

		JointDesc.twistLimit.high.value = 0.05f*NxPi;
		JointDesc.twistLimit.high.restitution = 0;

		JointDesc.projectionMode = NX_JPM_POINT_MINDIST;
		JointDesc.projectionDistance = 0.05f;

		JointDesc.flags = NX_SJF_SWING_LIMIT_ENABLED | NX_SJF_TWIST_LIMIT_ENABLED;

		NxVec3 localAxis = pActor2Nx->getGlobalPosition() - pActor1Nx->getGlobalPosition();
		localAxis.normalize();
		JointDesc.setGlobalAxis( localAxis );
		JointDesc.setGlobalAnchor( JointPos );

		m_pJoint = s_pPhysicsMgr->CreateJoint(JointDesc);
	}
}

CBsPhysicsD6Joint::CBsPhysicsD6Joint(IBsPhysicsActor *pActor1, IBsPhysicsActor *pActor2, D3DXVECTOR3 &AnchorPos,  int nSelect)
{
	NxD6JointDesc d6Desc;

	d6Desc.actor[0] =  pActor1 == NULL ? NULL : pActor1->GetActor();;
	d6Desc.actor[1] =  pActor2 == NULL ? NULL : pActor2->GetActor();;

	NxVec3 JointPos = *(NxVec3*) &AnchorPos;
	JointPos.multiply( cWorldScaleDiv, JointPos);
	d6Desc.setGlobalAnchor( JointPos );
	d6Desc.setGlobalAxis(NxVec3(0,1,0));

	d6Desc.xMotion = NX_D6JOINT_MOTION_LOCKED;
	d6Desc.yMotion = NX_D6JOINT_MOTION_LOCKED;
	d6Desc.zMotion = NX_D6JOINT_MOTION_LOCKED;
	d6Desc.twistMotion = NX_D6JOINT_MOTION_LOCKED;
	d6Desc.swing1Motion = NX_D6JOINT_MOTION_LOCKED;
	d6Desc.swing2Motion = NX_D6JOINT_MOTION_LOCKED;

	d6Desc.projectionMode = NX_JPM_POINT_MINDIST;

	d6Desc.projectionDistance = 0.015f;
	d6Desc.projectionAngle *= 3.f;

	d6Desc.projectionAngle *= 10.f;

// [PREFIX:beginmodify] 2006/2/15 junyash PS#5171 reports dereferencing NULL pointer 'pActor1'.
	//if( nSelect == 0 )
	if( nSelect == 0 && pActor1 )
// [PREFIX:endmodify] junyash
	{
		
		NxActor *pNxActor = pActor1->GetActor();
// [PREFIX:beginmodify] 2006/2/15 junyash PS#5171 reports dereferencing NULL pointer 'pActor1'.
		if ( pActor2 )
		{
			NxActor *pNxActorChild = pActor2->GetActor();	

			if( !strcmp(pNxActor->getName(), "Bip01 Neck") && 
				!strcmp(pNxActorChild->getName(), "Bip01 Head") )
			{
				//d6Desc.projectionAngle *= 25.f;
			}
		}
// [PREFIX:endmodify] junyash

		if(
			strstr(pNxActor->getName(), "Arm") ||
			strstr(pNxActor->getName(), "Thigh") ||
			strstr(pNxActor->getName(), "Calf")
			)		
		{
			d6Desc.twistMotion = NX_D6JOINT_MOTION_LIMITED;
			d6Desc.swing1Motion = NX_D6JOINT_MOTION_LIMITED;
			d6Desc.swing2Motion = NX_D6JOINT_MOTION_LIMITED;

			d6Desc.swing1Limit.value = 0.07f*NxPi;
			d6Desc.swing1Limit.restitution = 0;
			d6Desc.swing1Limit.spring = 15.0;
			d6Desc.swing1Limit.damping = 1.0;

			d6Desc.swing2Limit.value = 0.07f*NxPi;
			d6Desc.swing2Limit.restitution = 0;
			d6Desc.swing2Limit.spring = 15.0;
			d6Desc.swing2Limit.damping = 1.0;

			d6Desc.twistLimit.low.value = -0.01f*NxPi;
			d6Desc.twistLimit.low.restitution = 0;
			d6Desc.twistLimit.low.spring = 15.0;
			d6Desc.twistLimit.low.damping = 1.0;

			d6Desc.twistLimit.high.value = 0.01f*NxPi;
			d6Desc.twistLimit.high.restitution = 0;
			d6Desc.twistLimit.high.spring = 15.0;
			d6Desc.twistLimit.high.damping = 1.0;		
		}
	}
	

	m_pJoint = s_pPhysicsMgr->CreateJoint( d6Desc );
}

void CBsPhysicsD6Joint::SetLimited()
{
	if(!m_pJoint->isD6Joint()) return ;


	NxD6JointDesc d6Desc;

	NxActor *pActor1, *pActor2;
	m_pJoint->getActors(&pActor1, &pActor2);

	if(pActor1 ) {
		pActor1->setSleepLinearVelocity( 2.1f );
		pActor1->setSleepAngularVelocity( 1.15f );
	}
	if(pActor2 ) {
		pActor2->setSleepLinearVelocity( 2.1f );
		pActor2->setSleepAngularVelocity( 1.15f );
	}

	if(!strcmp(pActor1->getName(), "Bip01") ||
		!strcmp(pActor1->getName(), "Bip01 Neck") ||
		strstr(pActor1->getName(), "Clavicle") ||
		strstr(pActor1->getName(), "Spine") )		
	{
		return;
	}

	((NxD6Joint*)m_pJoint)->saveToDesc(d6Desc);

	d6Desc.twistMotion = NX_D6JOINT_MOTION_LIMITED;
	d6Desc.swing1Motion = NX_D6JOINT_MOTION_LIMITED;
	d6Desc.swing2Motion = NX_D6JOINT_MOTION_LIMITED;

	d6Desc.swing1Limit.value = 0.01f*NxPi;
	d6Desc.swing1Limit.restitution = 0;
	d6Desc.swing1Limit.spring = 0.5;
	d6Desc.swing1Limit.damping = 500.0;

	d6Desc.swing2Limit.value = 0.01f*NxPi;
	d6Desc.swing2Limit.restitution = 0;
	d6Desc.swing2Limit.spring = 0.5;
	d6Desc.swing2Limit.damping = 500.0;

	d6Desc.twistLimit.low.value = -0.002f*NxPi;
	d6Desc.twistLimit.low.restitution = 0;
	d6Desc.twistLimit.low.spring = 30.5;
	d6Desc.twistLimit.low.damping = 500.0;

	d6Desc.twistLimit.high.value = 0.002f*NxPi;
	d6Desc.twistLimit.high.restitution = 0;
	d6Desc.twistLimit.high.spring = 30.5;
	d6Desc.twistLimit.high.damping = 500.0;

	d6Desc.projectionMode = NX_JPM_POINT_MINDIST;
	d6Desc.projectionDistance = 0.02f;
	d6Desc.projectionAngle = 0.0001f;

	((NxD6Joint*)m_pJoint)->loadFromDesc(d6Desc);
}

CBsPhysicsRevoulteJoint::CBsPhysicsRevoulteJoint(IBsPhysicsActor *pActor1, IBsPhysicsActor *pActor2, D3DXVECTOR3 &AnchorPos, PHYSICS_JOINT_INFO &JointInfo) {
	NxRevoluteJointDesc JointDesc;

	NxVec3 JointPos = *(NxVec3*) &AnchorPos;
	JointPos.multiply( cWorldScaleDiv, JointPos);
	JointDesc.actor[0] = pActor1->GetActor();
	JointDesc.actor[1] = pActor2->GetActor();
	JointDesc.setGlobalAnchor( JointPos );
	JointDesc.setGlobalAxis( *(NxVec3*)&JointInfo.GlobalAxis );


	pActor1->GetActor()->setLinearDamping( 5.f );
	pActor2->GetActor()->setLinearDamping( 5.f );

	JointDesc.flags |= NX_RJF_LIMIT_ENABLED;
	JointDesc.limit.high.value = (JointInfo.maxAngle/180.0f) * NxPi;
	JointDesc.limit.high.restitution = 0;
	JointDesc.limit.low.value = (JointInfo.minAngle/180.0f) * NxPi;
	JointDesc.limit.low.restitution = 0;

	if(pActor1 && pActor2 && pActor1->IsKinematic() || pActor2->IsKinematic()) {
		JointDesc.flags |= NX_RJF_SPRING_ENABLED;
		//JointDesc.spring.spring = 500.f;
		//JointDesc.spring.damper = 10.f;
	}

	JointDesc.projectionDistance = 0.02f;
	JointDesc.projectionAngle = 0.137f;
	JointDesc.projectionMode = NX_JPM_POINT_MINDIST;

	m_pJoint = s_pPhysicsMgr->CreateJoint(JointDesc);
}

CBsPhysicsRevoulteJointForTroll::CBsPhysicsRevoulteJointForTroll(IBsPhysicsActor *pActor1, IBsPhysicsActor *pActor2, D3DXVECTOR3 &AnchorPos, PHYSICS_JOINT_INFO &JointInfo) {

	// mruete: prefix bug 361: new asserts, and caching the ->GetActor() calls
	BsAssert( NULL != pActor1 && NULL != pActor2 );
	NxActor * pActor1Nx = pActor1->GetActor();
	NxActor * pActor2Nx = pActor2->GetActor();
	BsAssert( NULL != pActor1Nx && NULL != pActor2Nx );

	NxRevoluteJointDesc JointDesc;

	NxVec3 JointPos = *(NxVec3*) &AnchorPos;
	JointPos.multiply( cWorldScaleDiv, JointPos);
	JointDesc.actor[0] = pActor1Nx;
	JointDesc.actor[1] = pActor2Nx;

	pActor1Nx->raiseActorFlag( NX_AF_DISABLE_COLLISION );
	pActor2Nx->raiseActorFlag( NX_AF_DISABLE_COLLISION );

	JointDesc.setGlobalAnchor( JointPos );

	NxVec3 localAxis = pActor1Nx->getGlobalPosition() - pActor2Nx->getGlobalPosition();
	localAxis.normalize();	
	D3DXVec3Cross((D3DXVECTOR3*)&localAxis, (D3DXVECTOR3*)&localAxis, &D3DXVECTOR3(0,1,0));

	JointDesc.setGlobalAxis( localAxis );

	JointDesc.flags |= NX_RJF_LIMIT_ENABLED;
	JointDesc.limit.high.value = (40.f/180.0f) * NxPi;
	JointDesc.limit.high.hardness = 1.0f;
	JointDesc.limit.high.restitution = 0.1f;
	JointDesc.limit.low.value = (-10.f/180.0f) * NxPi;
	JointDesc.limit.low.restitution = 0.1f;
	JointDesc.limit.low.hardness = 1.0f;

	JointDesc.projectionDistance = 0.02f;
	JointDesc.projectionAngle = 0.137f;
	JointDesc.projectionMode = NX_JPM_POINT_MINDIST;

	m_pJoint = s_pPhysicsMgr->CreateJoint(JointDesc);
}


CBsPhysicsMgr::CBsPhysicsMgr()
{
	int i;

	m_pPhysicsSDK = NULL;
	m_pScene[0] = NULL;	
	m_pScene[1] = NULL;	
	m_pActiveScene = NULL;	
	m_bVisible = false;
	m_bEnable = true;

	for( i = 0; i < 32; i++) {
		m_szGroupName[i][0] = NULL;
	}

	m_nRenderIndex = 0;
	m_nPhysicsTick = 0;

	m_nCustomValue = 0;
}

CBsPhysicsMgr::~CBsPhysicsMgr()
{
	ReleasePhysics();
}

void CBsPhysicsMgr::InitPhysics()
{

	m_pPhysicsSDK = NxCreatePhysicsSDK( NX_PHYSICS_SDK_VERSION, NULL,  CBsUserNxClass::GetUserOutputStream());
	BsAssert( m_pPhysicsSDK );
	
	NxInitCooking(NULL);

	m_pPhysicsSDK->setParameter(NX_SKIN_WIDTH, 0.01f);

	m_pPhysicsSDK->setParameter(NX_VISUALIZATION_SCALE, 0.1f);
	m_pPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 1);
	m_pPhysicsSDK->setParameter(NX_VISUALIZE_JOINT_LOCAL_AXES, 1);
	m_pPhysicsSDK->setParameter(NX_VISUALIZE_JOINT_LIMITS, 1);
	m_pPhysicsSDK->setParameter(NX_VISUALIZE_BODY_SLEEP , 1);
	//m_pPhysicsSDK->setParameter(NX_BOUNCE_TRESHOLD , -0.01f);
	//m_pPhysicsSDK->setParameter(NX_CONTINUOUS_CD  , 0.f);

	//m_pPhysicsSDK->setParameter(NX_VISUALIZE_BODY_SLEEP, 1);


	NxSceneDesc sceneDesc;
	sceneDesc.gravity					= NxVec3( 0, -981.f * 2.5f , 0 ) * cWorldScaleDiv;
	sceneDesc.broadPhase				= NX_BROADPHASE_COHERENT ;
	sceneDesc.collisionDetection		= true;	
	sceneDesc.maxTimestep			= 1 / 40.f;
	sceneDesc.timeStepMethod		= NX_TIMESTEP_FIXED;	

	//sceneDesc.hwSceneType 
	//sceneDesc.userContactReport	= CBsUserNxClass::GetUserContactReport();
#ifdef _XBOX
	DWORD dwCurrentProcessor = XSetThreadProcessor(GetCurrentThread(), 5);
#endif
	m_pScene[0] = m_pPhysicsSDK->createScene( sceneDesc );
	m_pScene[1] = m_pPhysicsSDK->createScene( sceneDesc );
	m_pActiveScene = m_pScene[0];
#ifdef _XBOX
	XSetThreadProcessor(GetCurrentThread(), dwCurrentProcessor );
#endif	
	BsAssert( m_pScene[0] );
	BsAssert( m_pScene[1] );

	m_pScene[0]->setUserTriggerReport( CBsUserNxClass::GetUserTriggerReport() );
	m_pScene[1]->setUserTriggerReport( CBsUserNxClass::GetUserTriggerReport() );
	//m_pScene[1]->setUserTriggerReport( CBsUserNxClass::GetUserTriggerReport() );
	/*
	// Set up the filtering equation
	m_pPhysicsSDK->setFilterOps(NX_FILTEROP_OR, NX_FILTEROP_OR, NX_FILTEROP_AND);
	m_pPhysicsSDK->setFilterBool(true);
	NxGroupsMask zeroMask;
	zeroMask.bits0 = zeroMask.bits1 = zeroMask.bits2 = zeroMask.bits3 = 0;
	m_pPhysicsSDK->setFilterConstant0(zeroMask);
	m_pPhysicsSDK->setFilterConstant1(zeroMask);
	*/



	int i, j;
	// Create the default material

	for( i = 0; i < 2; i++) {
		
		NxMaterialDesc material;
		NxMaterial *pMat;

		material.restitution = 0.5f;
		material.staticFriction = 0.6f;
		material.dynamicFriction = 0.7f;
		pMat = m_pScene[i]->createMaterial( material );
		m_MaterialList[i].push_back( pMat );
		nMaterialDefault = pMat->getMaterialIndex();

		// prop materials
		material.restitution = 0.23f;
		material.staticFriction = 0.9f;
		material.dynamicFriction = 0.9f;
		pMat = m_pScene[i]->createMaterial( material );
		m_MaterialList[i].push_back( pMat );
		nMaterialProp = pMat->getMaterialIndex();

		// ice material
		material.restitution = 0.f;
		material.staticFriction = 0.f;
		material.dynamicFriction = 0.f;
		pMat = m_pScene[i]->createMaterial( material );
		m_MaterialList[i].push_back( pMat );
		nMaterialIce = pMat->getMaterialIndex();

		// ragdoll material
		material.restitution = 0.5f;
		material.staticFriction = 0.7f;
		material.dynamicFriction = 0.8f;
		pMat = m_pScene[i]->createMaterial( material );
		m_MaterialList[i].push_back( pMat );
		nMaterialRagdoll = pMat->getMaterialIndex();
	}

	int nDefaultGroup = FindGroupID("DEFAULT");
	BsAssert( nDefaultGroup == 0);
	int nHeightGroup = FindGroupID("HEIGHT_FIELD");

	for( i = 1; i < 32; i++)
	for( j = 1; j < i; j++)
	{
		m_pPhysicsSDK->setGroupCollisionFlag( i ,  j , false);
	}

	for( i = 1; i < 32; i++) {
		m_pPhysicsSDK->setGroupCollisionFlag( i, nHeightGroup, true);
	}	
}

void CBsPhysicsMgr::ReleasePhysics()
{
	Clear();
	NxCloseCooking();

	int nSize;
	for( int k = 0; k < 2; k++) {		
		nSize = m_MaterialList[k].size();
		for( int i = 0; i < nSize; i++) {
			m_pScene[k]->releaseMaterial( *m_MaterialList[k][i] );
		}
		m_MaterialList[k].clear();

		if( m_pScene[k] ) 
		{
			m_pPhysicsSDK->releaseScene( *m_pScene[k] );		
			m_pScene[k] = NULL;		
		}
	}

	if( m_pPhysicsSDK )
	{
		m_pPhysicsSDK->release();
		m_pPhysicsSDK = NULL;
	}

}

void CBsPhysicsMgr::RunPhysics(float fDeltaTime)
{

	//int i, nSize;
	// 트리거 액터들만 runphysics 하기전 지워준다..

	/*
	{
		_PROCESS_THREAD_CHECK
		
		nSize = m_DeleteTriggerActorList.size();
		for( i = 0; i < nSize; i++) {
			(m_DeleteTriggerActorList[i]->getScene()).releaseActor( *m_DeleteTriggerActorList[i] );
		}
		m_DeleteTriggerActorList.clear();	
	}
	*/

	for( int nScene = 0; nScene < 2; nScene++) {
		if( !m_bEnable && nScene == 0) continue;	// 0번만 Disable의 경우 스킵
		m_pScene[ nScene ]->simulate( fDeltaTime );
		m_pScene[ nScene ]->flushStream();
	}		
}

void CBsPhysicsMgr::Flush()
{
	for( int nScene = 0; nScene < 2; nScene++) {
		if( !m_bEnable && nScene == 0) continue;	// 0번만 Disable의 경우 스킵
		m_pScene[ nScene ]->fetchResults( NX_RIGID_BODY_FINISHED, true );	
	}
	++m_nPhysicsTick;

	if( m_bVisible ) {
		//for( int k = 0; k < 2; k++) {
			m_pScene[0]->visualize();	

		//}
		m_pPhysicsSDK->visualize( CBsUserNxClass::GetUserDebugRenderer() );
	}

	int i, nSize;

	nSize = m_ZoneList.size();	
	for( i  = 0; i < nSize; i++ ) {
		CBsPhysicsHeightFieldZone *pZone = m_ZoneList[i];

		if( pZone->IsCrush() ) {
			pZone->CreateZone();
		}
		// 25 second not using...unload  physics terrain zone
		if( pZone->IsCreated() && m_nPhysicsTick - pZone->GetLastUseTick() > 1000 ) {
			pZone->UnLoad();
		}
	}

	
	{
		/*
		_PROCESS_THREAD_CHECK		

		nSize = m_DeleteActorList.size();
		for( i = 0; i < nSize; i++) {			
			(m_DeleteActorList[i]->getScene()).releaseActor( *m_DeleteActorList[i] );
		}
		m_DeleteActorList.clear();
		
		
		nSize = m_DeleteJointList.size();
		for( i = 0; i < nSize; i++) {
			if( m_DeleteJointList[i])
				((NxScene*)m_DeleteJointList[i]->userData)->releaseJoint( *m_DeleteJointList[i] );
		}
		m_DeleteJointList.clear();
		*/
	}

}

void CBsPhysicsMgr::Clear()
{

	int i, nSize;

	nSize = m_ZoneList.size();
	for(i = 0; i < nSize; i++) {
		delete m_ZoneList[i];
	}
	m_ZoneList.clear();
	
	{
			/*
			_PROCESS_THREAD_CHECK

		for( i = 0; i < (int)m_DeleteTriggerActorList.size(); i++) {
			(m_DeleteTriggerActorList[i]->getScene()).releaseActor( *m_DeleteTriggerActorList[i] );
		}
		m_DeleteTriggerActorList.clear();
		
		for( i = 0; i < (int)m_DeleteActorList.size(); i++) {
			(m_DeleteActorList[i]->getScene()).releaseActor( *m_DeleteActorList[i] );
		}
		m_DeleteActorList.clear();
		
	
		nSize = m_DeleteJointList.size();
		for( i = 0; i < nSize; i++) {
			if( m_DeleteJointList[i] )
				((NxScene*)m_DeleteJointList[i]->userData)->releaseJoint( *m_DeleteJointList[i] );
		}
		m_DeleteJointList.clear();		
		*/
	}
	
	// 안지워진 애들은 강제로 지우되 디버그 스트링 뛰워준다..(어디선가 안지워준 경우)
	for( int k = 0; k < 2; k++) {

		int nDeleteActorCount = 0;
		int nDeleteJointCount = 0;
	
		m_pScene[k]->resetJointIterator();	
		while(true) {
			NxJoint *pJoint = m_pScene[k]->getNextJoint();
			if(pJoint == NULL) break;			
			nDeleteJointCount++;
		}

		nSize = m_pScene[k]->getNbActors();
		for( i = 0; i < nSize; i++) {	
			NxActor *pActor = m_pScene[k]->getActors()[i];
			nDeleteActorCount++;
		}	

		if( nDeleteActorCount != 0 || nDeleteJointCount != 0) {
			DebugString( "Novodex Actor Clear Failed (%d actor,%d joint remain) !!\n" , nDeleteActorCount, nDeleteJointCount);
			// 실기무비도중 다른맵을 로딩하면 여기로 들어올 수 있습니다.. 그런 경우는 무시하고 
			// 그이외에는 노보덱스 Actor 를 제대로 지워주지 않은 경우이므로 문제가 생길 수 있습니다.

#ifndef MAKE_ALL_MISSION_RESLIST
			_DEBUGBREAK;
#endif
		}
	}
}

NxActor *CBsPhysicsMgr::CreateActor( NxActorDesc &ActorDesc, IBsPhysicsActor *pPhysicsActor)
{
	_PROCESS_THREAD_CHECK
	if( !ActorDesc.isValid() ) {
		DebugString(" actorDesc Invalid \n");		
		return NULL;
	}
	NxActor *pActor = m_pActiveScene->createActor( ActorDesc );	
	BsAssert( pActor != NULL );
	//pActor->userData = (void *) pPhysicsActor;
	return pActor;
}

NxTriangleMesh *CBsPhysicsMgr::CreateTriangleMesh( const NxStream &stream)
{
	NxTriangleMesh *pMesh = m_pPhysicsSDK->createTriangleMesh( stream );
	return pMesh;
}

NxConvexMesh *CBsPhysicsMgr::CreateConvexMesh( const NxStream &stream)
{
	NxConvexMesh *pMesh = m_pPhysicsSDK->createConvexMesh( stream );
	return pMesh;
}

void CBsPhysicsMgr::ReleaseTriangleMesh( NxTriangleMesh &triangleMesh )
{	
	m_pPhysicsSDK->releaseTriangleMesh( triangleMesh );
}

	/*
void CBsPhysicsMgr::ReleaseActor(IBsPhysicsActor *pPhysicsActor)
{

	NxActor *pActor = pPhysicsActor->GetActor();
	NxShape*const* ppShape = pActor->getShapes();
	int nCount = pActor->getNbShapes();
	while(nCount--) {
		ppShape[ nCount ]->userData = NULL;
	}
	pActor->userData = NULL;
	
	{	
		_PROCESS_THREAD_CHECK
	
		if( pPhysicsActor->IsTrigger() ) {			
			m_DeleteTriggerActorList.push_back( pPhysicsActor->GetActor() );
		}
		else {
			m_DeleteActorList.push_back( pPhysicsActor->GetActor() );//pActor );	
		}		
	}	
}
*/

IBsPhysicsJoint *CBsPhysicsMgr::CreateJoint( IBsPhysicsActor *pActor1, IBsPhysicsActor *pActor2, PHYSICS_JOINT_INFO &JointInfo, D3DXVECTOR3 &AnchorPos, int nSelect, int nLevel)
{
	IBsPhysicsJoint *pJoint = NULL;
	if( JointInfo.nType == PHYSICS_JOINT_INFO::SPHERICAL ) {

		pJoint = CBsPhysicsSphericalJoint::Create(pActor1, pActor2, AnchorPos,  nSelect, nLevel);
	}
	else if( JointInfo.nType == PHYSICS_JOINT_INFO::REVOLUTE ) {
		const int cTroll = 777;
		if( nSelect == cTroll) 
			pJoint = CBsPhysicsRevoulteJointForTroll::Create(pActor1, pActor2, AnchorPos, JointInfo);
		else
			pJoint = CBsPhysicsRevoulteJoint::Create(pActor1, pActor2, AnchorPos, JointInfo);
	}
	else {
		CBsConsole::GetInstance().AddFormatString("Invalid Joint Type!");
	}

	return pJoint;
}

NxJoint *CBsPhysicsMgr::CreateJoint( NxJointDesc &JointDesc )
{
	_PROCESS_THREAD_CHECK
	BsAssert( JointDesc.isValid() );
	NxJoint *pJoint = m_pActiveScene->createJoint( JointDesc );			
	return pJoint;
}

/*
void CBsPhysicsMgr::ReleaseJoint( NxJoint *Joint)
{
	
	_PROCESS_THREAD_CHECK	
	m_DeleteJointList.push_back( Joint );		
}
*/


void CBsPhysicsMgr::Visualize()
{
	if( m_bVisible ) {	

		C3DDevice *pDevice = g_BsKernel.GetDevice();

		D3DXMATRIX mat;
		D3DXMatrixIdentity( &mat );

		pDevice->SetTexture( 0, NULL );
		pDevice->SetVertexShader( NULL );
		pDevice->SetPixelShader( NULL );

#ifdef _XBOX
		D3DXVECTOR4 diffColor(1,1,1,1);
		pDevice->SetDefaultShader( g_BsKernel.GetParamViewProjectionMatrix(), &diffColor);
#else
		pDevice->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );

		pDevice->SetTransform( D3DTS_WORLD, &mat );
		pDevice->SetRenderState(D3DRS_LIGHTING,FALSE);
#endif
		pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

		int nBufferIndex = CBsObject::GetRenderBufferIndex();

		if ( !m_PointList[nBufferIndex].empty()) {
			pDevice->DrawPrimitiveUP( D3DPT_POINTLIST, m_PointList[nBufferIndex].size(), &m_PointList[nBufferIndex][0], sizeof(DebugVertex) );	
		}
		if ( !m_LineList[nBufferIndex].empty() ) {		

			int nDrawPrimCount = m_LineList[nBufferIndex].size()/2;

			for( int i  = 0; i < nDrawPrimCount; i += 4000) {
				int nCount = min((nDrawPrimCount - i), 4000);
				pDevice->DrawPrimitiveUP( D3DPT_LINELIST, nCount , &m_LineList[nBufferIndex][i*2], sizeof(DebugVertex) );
			}

		}
		if ( !m_TriangleList[nBufferIndex].empty() ) {
			int nDrawPrimCount = m_LineList[nBufferIndex].size()/3;
			for( int i  = 0; i < nDrawPrimCount; i += 2000 ) { 
				int nCount = min((nDrawPrimCount - i), 2000);
				pDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST, nCount , &m_TriangleList[nBufferIndex][i*3], sizeof(DebugVertex) );
			}
		}
#ifndef _XBOX
		pDevice->SetRenderState(D3DRS_LIGHTING,TRUE);
#endif
		pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);		
	}
}

int CBsPhysicsMgr::FindGroupID(const char *szName)
{
	int i;
	for( i = 0; i < 32; i++) {
		if(_stricmp(m_szGroupName[ i ], szName) == 0) {
			return i;
		}
	}
	return GenerateGroupID( szName );    	
}

int CBsPhysicsMgr::GenerateGroupID(const char *szName)
{
	int i;
	for( i = 0; i < 32; i++) {
		if( m_szGroupName[ i ][0] == NULL) {
			strcpy( m_szGroupName[ i ], szName);
			return i;			
		}
	}
	return -1;        
}

void CBsPhysicsMgr::SetGroupCollision(const char *szName1, const char *szName2, bool bEnable)
{
	int nID1, nID2;
	nID1 = FindGroupID( szName1 );	
	BsAssert( nID1 != -1 && "No More Group.");

	nID2 = FindGroupID( szName2 );	
	BsAssert( nID2 != -1 && "No More Group.");

	m_pPhysicsSDK->setGroupCollisionFlag(nID1, nID2, bEnable);
}

void CBsPhysicsMgr::AddForce(D3DXVECTOR3 *Force)
{
}

void CBsPhysicsMgr::SetGravity(float fGravity)
{
	m_pActiveScene->setGravity(NxVec3(0, fGravity,0));	
}

bool CBsPhysicsMgr::IsOcclusionCull( D3DXVECTOR3 viewPos, D3DXVECTOR3 vecPoint )
{
	NxVec3 scaledViewPos = *(NxVec3*) (&(viewPos*cWorldScaleDiv));
	
	D3DXVECTOR3 vDir =  vecPoint - viewPos;
	float fDistance = D3DXVec3Length(&vDir);
	vDir *= (1.f/fDistance);
	fDistance *= cWorldScaleDiv;
	NxRay ray1( scaledViewPos,  *(NxVec3*)&vDir);	
	if(!m_pActiveScene->raycastAnyShape(ray1, NX_STATIC_SHAPES, 0xffffffff,fDistance, NULL)) return false;

	return true;
}

void CBsPhysicsMgr::SetActiveScene( int nScene )
{
	m_pActiveScene = m_pScene[nScene];
}

IBsPhysicsMgr* CBsPhysicsMgr::Create() 
{
	BsAssert( s_pPhysicsMgr == NULL);

	if(UseNovodex())
		s_pPhysicsMgr = new CBsPhysicsMgr;
	else
		s_pPhysicsMgr = new CBsNullPhyMgr;

	return s_pPhysicsMgr;
}

void CBsPhysicsMgr::SetEnable(bool bEnable) 
{ 
	m_bEnable = bEnable; 
}

void CBsPhysicsMgr::SetRenderData( const NxDebugRenderable &data )
{
	int nBufferIndex = CBsObject::GetProcessBufferIndex();

	{

		const NxU32   nPoints  = data.getNbPoints();
		const NxDebugPoint *pPoints = data.getPoints();
		
		m_PointList[nBufferIndex].resize(nPoints);

		for ( NxU32 i = 0; i < nPoints; i++ )
		{
			m_PointList[nBufferIndex][ i ].position.x = pPoints[ i ].p.x;
			m_PointList[nBufferIndex][ i ].position.y = pPoints[ i ].p.y;
			m_PointList[nBufferIndex][ i ].position.z = pPoints[ i ].p.z;
			m_PointList[nBufferIndex][ i ].position *= GetWorldScaleMul();
			m_PointList[nBufferIndex][ i ].color = pPoints[ i ].color;
		}
	}
	{		

		const NxU32            nLines  = data.getNbLines();
		const NxDebugLine    *pLines = data.getLines();

		m_LineList[nBufferIndex].resize(nLines*2);

		for ( NxU32 i = 0; i < nLines; i++ )
		{
			m_LineList[nBufferIndex][ ( i * 2 ) + 0 ].position = D3DXVECTOR3( pLines[ i ].p0.x, pLines[ i ].p0.y, pLines[ i ].p0.z ) * GetWorldScaleMul();//+D3DXVECTOR3(0,150,0);
			m_LineList[nBufferIndex][ ( i * 2 ) + 0 ].color  = pLines[ i ].color;
			m_LineList[nBufferIndex][ ( i * 2 ) + 1 ].position = D3DXVECTOR3( pLines[ i ].p1.x, pLines[ i ].p1.y, pLines[ i ].p1.z ) * GetWorldScaleMul();//+D3DXVECTOR3(0,150,0);
			m_LineList[nBufferIndex][ ( i * 2 ) + 1 ].color  = pLines[ i ].color;
		}		
	}
	{

		const NxU32                nTriangles = data.getNbTriangles();
		const NxDebugTriangle    *pTriangles = data.getTriangles();
		
		m_TriangleList[nBufferIndex].resize(nTriangles*3);

		for ( NxU32 i= 0; i < nTriangles; i++ )
		{
			m_TriangleList[nBufferIndex][ ( i * 3 ) + 0 ].position = D3DXVECTOR3( pTriangles[ i ].p0.x, pTriangles[ i ].p0.y, pTriangles[ i ].p0.z ) * GetWorldScaleMul();
			m_TriangleList[nBufferIndex][ ( i * 3 ) + 0 ].color  = pTriangles[ i ].color;
			m_TriangleList[nBufferIndex][ ( i * 3 ) + 1 ].position = D3DXVECTOR3( pTriangles[ i ].p1.x, pTriangles[ i ].p1.y, pTriangles[ i ].p1.z ) * GetWorldScaleMul();
			m_TriangleList[nBufferIndex][ ( i * 3 ) + 1 ].color  = pTriangles[ i ].color;
			m_TriangleList[nBufferIndex][ ( i * 3 ) + 2 ].position = D3DXVECTOR3( pTriangles[ i ].p2.x, pTriangles[ i ].p2.y, pTriangles[ i ].p2.z ) * GetWorldScaleMul();
			m_TriangleList[nBufferIndex][ ( i * 3 ) + 2 ].color  = pTriangles[ i ].color;
		}
	}
}
