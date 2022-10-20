#pragma once

class CFcPhysicsLoader
{
public:
	static PHYSICS_DATA_CONTAINER* LoadPhysicsData(const char *pFileName);
	static void ClearPhysicsData( PHYSICS_DATA_CONTAINER *pPhysicsInfo);
	
	static PHYSICS_COLLISION_CONTAINER* LoadCollisionMesh(const char *pFileName);
	static void ClearCollisionMesh( PHYSICS_COLLISION_CONTAINER *pCollisionInfo);
    
	static PHYSICS_RAGDOLL_CONTAINER* LoadRagdollSetting(const char *pFileName);
	static void ClearRagdollSetting( PHYSICS_RAGDOLL_CONTAINER*pRagdollInfo);

	static void LoadGlobalPhysicsSetting(const char *pFileName);

	static void ClearAll();
private:
};
