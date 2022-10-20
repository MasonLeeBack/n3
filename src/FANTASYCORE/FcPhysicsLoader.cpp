#include "stdafx.h"
#include "BsKernel.h"
#include "BsPhysicsMgr.h"
#include "Parser.h"
#include "FcPhysicsLoader.h"
#include "SharedResource.h"
#include "./Util/Token.h"

#include <io.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


struct LoadResourceFn
{
    void load(const char *pFileName, PHYSICS_DATA_CONTAINER *pData);
	void load(const char *pFileName, PHYSICS_COLLISION_CONTAINER *pData);
	void load(const char *pFileName, PHYSICS_RAGDOLL_CONTAINER *pData);
};

SharedResource< PHYSICS_DATA_CONTAINER > s_PhysicsResource;
SharedResource< PHYSICS_COLLISION_CONTAINER > s_CollisionResource;
SharedResource< PHYSICS_RAGDOLL_CONTAINER > s_RagdollResource;

void CFcPhysicsLoader::ClearAll()
{
	s_PhysicsResource.ClearAll();
	s_CollisionResource.ClearAll();
	s_RagdollResource.ClearAll();
}


PHYSICS_DATA_CONTAINER* CFcPhysicsLoader::LoadPhysicsData(const char *pFileName)
{
	return s_PhysicsResource.Load( pFileName );
}

void CFcPhysicsLoader::ClearPhysicsData( PHYSICS_DATA_CONTAINER *pPhysicsInfo)
{
    s_PhysicsResource.Clear(pPhysicsInfo);
}

PHYSICS_COLLISION_CONTAINER* CFcPhysicsLoader::LoadCollisionMesh(const char *pFileName)
{
	return s_CollisionResource.Load( pFileName );	
}

void CFcPhysicsLoader::ClearCollisionMesh( PHYSICS_COLLISION_CONTAINER *pCollisionInfo)
{
	if( pCollisionInfo ) {
		s_CollisionResource.Clear( pCollisionInfo );
	}	
}

PHYSICS_RAGDOLL_CONTAINER* CFcPhysicsLoader::LoadRagdollSetting(const char *pFileName)
{
	return s_RagdollResource.Load( pFileName );
}

void CFcPhysicsLoader::ClearRagdollSetting( PHYSICS_RAGDOLL_CONTAINER*pRagdollInfo)
{
	if( pRagdollInfo ) {
		s_RagdollResource.Clear( pRagdollInfo );
	}
}

void LoadResourceFn::load(const char *pFileName, PHYSICS_RAGDOLL_CONTAINER *pData)
{
	char cBinFileName[MAX_PATH];
	char cScriptFileName[MAX_PATH];

	sprintf( cBinFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), pFileName );
	sprintf( cScriptFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), pFileName );

	char* pStr = strrchr( cBinFileName, '.' );
	BsAssert( pStr );
	pStr[1] = NULL;
	//strcat( pStr,  "bin" );
	strcat(cBinFileName, "bin"); // aleksger - safe string

	g_BsResChecker.AddResList( cBinFileName );

	TokenBuffer tb;
	int nWriteTime = -1;

	if( tb.Load( cBinFileName ) == false ) {	
		BsAssert( 0 && "Binary 파일없음");
		return ;
	}


	while( 1 ) {
		if( tb.IsEnd() ) {
			break;
		}

		PHYSICS_RAGDOLL_CONTAINER::Elem ragdollInfo;

		ragdollInfo.szName = tb.GetString();
		tb++;
		ragdollInfo.fRadius = tb.GetReal();
		tb++;
		ragdollInfo.fHeight = tb.GetReal();
		tb++;
		
		for(int i=0; i<4; i++)
			for(int j=0; j<4; j++) {
				ragdollInfo.matLocal.m[i][j] = tb.GetReal();
				tb++;
			}

		pData->RagdollList.push_back( ragdollInfo );
	}
}

void LoadResourceFn::load(const char *pFileName, PHYSICS_DATA_CONTAINER *pData)
{
	char cBinFileName[MAX_PATH];
	char cScriptFileName[MAX_PATH];

	sprintf( cBinFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), pFileName );
	sprintf( cScriptFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), pFileName );

	char* pStr = strrchr( cBinFileName, '.' );
	BsAssert( pStr );
	pStr[1] = NULL;
	//strcat( pStr, "bin" );  //aleksger - safe string
	strcat(cBinFileName, "bin");

	TokenBuffer tb;
	
	pData->bUseBoundingBox = false;

	g_BsResChecker.AddResList( cBinFileName );

	if( tb.Load( cBinFileName ) == false ) {	
		BsAssert( 0 && "Binary 파일없음");
		return ;
	}

	pData->bUseBoundingBox = false;

	int nVersion = tb.GetInteger();
	tb++;
	while( 1 )
	{
		if( tb.IsEnd() )
		{
			break;
		}
		if( _stricmp( "ActorInfo", tb.GetString() ) == 0 )
		{
			tb++;
			D3DXVECTOR3  Size;
			const char *szShapeType;
			PHYSICS_DATA_CONTAINER::ActorElem ActorInfo;

			ActorInfo.szBoneName = tb.GetString();
			tb++;	
			ActorInfo.szLinkBoneName = tb.GetString();
			tb++;
			ActorInfo.szGroupName = tb.GetString();
			tb++;
			ActorInfo.nIteration = tb.GetInteger();
			tb++;

			for(int i=0; i<4; i++)
				for(int j=0; j<4; j++) {
					ActorInfo.WorldMat.m[i][j] = tb.GetReal();
					tb++;
				}

				float l1 = fabs(1.f-D3DXVec3Length( (D3DXVECTOR3*)&ActorInfo.WorldMat._11 ));
				float l2 = fabs(1.f-D3DXVec3Length( (D3DXVECTOR3*)&ActorInfo.WorldMat._21 ));
				float l3 = fabs(1.f-D3DXVec3Length( (D3DXVECTOR3*)&ActorInfo.WorldMat._31 ));

				D3DXVECTOR3 Scale;
				Scale.x = D3DXVec3Length((D3DXVECTOR3*)&ActorInfo.WorldMat._11);
				Scale.y = D3DXVec3Length((D3DXVECTOR3*)&ActorInfo.WorldMat._21);
				Scale.z = D3DXVec3Length((D3DXVECTOR3*)&ActorInfo.WorldMat._31);

				D3DXMATRIX matInvScale, matObject;
				D3DXMatrixScaling(&matInvScale, 1 / Scale.x, 1 / Scale.y, 1 / Scale.z);
				D3DXMatrixMultiply(&ActorInfo.WorldMat, &matInvScale, &ActorInfo.WorldMat);

				if( l1 > 0.0001f || l2 > 0.0001f || l3 > 0.0001f) {
					char szMsg[255];
					sprintf(szMsg, "Novodex 에러> %s : %s Actor에 스케일 성분 있음\n", pFileName, ActorInfo.szBoneName.c_str());
					DebugString( szMsg );
				}

				if(nVersion >= 101) {
					szShapeType = tb.GetString();
					tb++;
					if(_stricmp(szShapeType, "Sphere") == 0) {
						ActorInfo.nShapeType = PHYSICS_DATA_CONTAINER::ActorElem::SHAPE_SPHERE;
						ActorInfo.fRadius = tb.GetReal();
						tb++;
					}
					else if(_stricmp(szShapeType, "Capsule") == 0) {
						ActorInfo.nShapeType = PHYSICS_DATA_CONTAINER::ActorElem::SHAPE_CAPSULE;
						ActorInfo.fRadius = tb.GetReal();
						tb++;
						ActorInfo.fHeight = fabs(tb.GetReal());
						tb++;
					}
					else if(_stricmp(szShapeType, "Box") == 0) {
						ActorInfo.nShapeType = PHYSICS_DATA_CONTAINER::ActorElem::SHAPE_BOX;
						ActorInfo.Size.x = tb.GetReal();
						tb++;
						ActorInfo.Size.y = tb.GetReal();
						tb++;
						ActorInfo.Size.z = tb.GetReal();
						tb++;
					}
					else  {
						BsAssert( false );
					}
				}
				else {
					ActorInfo.nShapeType = PHYSICS_DATA_CONTAINER::ActorElem::SHAPE_BOX;
					ActorInfo.Size.x = tb.GetReal();
					tb++;
					ActorInfo.Size.y = tb.GetReal();
					tb++;
					ActorInfo.Size.z = tb.GetReal();
					tb++;	
				}

				for(int i=0; i<4; i++)
					for(int j=0; j<4; j++) {
						ActorInfo.InverseWorldMat.m[i][j] = tb.GetReal();
						tb++;
					}	
				// 재계산
				D3DXMatrixInverse(&ActorInfo.InverseWorldMat, 0, &ActorInfo.WorldMat);

				pData->ActorList.push_back( ActorInfo );

		}
		else if( _stricmp( "JointInfo", tb.GetString()) == 0)
		{
			tb++;
			PHYSICS_JOINT_INFO LinkInfo;
			LinkInfo.nLevel = 0;

			const char *szJointType, *szJointName;

			szJointType = tb.GetString();
			tb++;

			if( _stricmp(szJointType, "Point_Point") == 0) {
				LinkInfo.nType = 1;
			}
			else if( _stricmp(szJointType, "Hinge") == 0) {
				LinkInfo.nType = 2;
			}
			else {
				BsAssert ( false );
			}

			szJointName = tb.GetString();
			tb++;

			if( LinkInfo.nType == 2 ) {
				if( nVersion >= 102 ) {
					LinkInfo.minAngle = tb.GetReal();
					tb++;
					LinkInfo.maxAngle = tb.GetReal();
					tb++;
					LinkInfo.GlobalAxis.x = tb.GetReal();
					tb++;
					LinkInfo.GlobalAxis.y = tb.GetReal();
					tb++;
					LinkInfo.GlobalAxis.z = tb.GetReal();
					tb++;
				}
				else {
					LinkInfo.minAngle = -90.0f;
					LinkInfo.maxAngle = 90.0f;
					LinkInfo.GlobalAxis = D3DXVECTOR3(0, 1, 0);
				}
			}	

			LinkInfo.szBoneName1 = tb.GetString();
			tb++;
			LinkInfo.szBoneName2 = tb.GetString();
			tb++;



			LinkInfo.AnchorPos.x = tb.GetReal();
			tb++;
			LinkInfo.AnchorPos.y = tb.GetReal();
			tb++;
			LinkInfo.AnchorPos.z = tb.GetReal();
			tb++;

			const char *szBreakable = tb.GetString();
			if( _stricmp(szBreakable, "true") == 0) {
				LinkInfo.bBreakable = true;
			}
			else if( _stricmp(szBreakable, "false") == 0) {
				LinkInfo.bBreakable = false;
			}
			else {
				BsAssert(false);
			}
			tb++;

			LinkInfo.LinearBreak = tb.GetReal();
			tb++;
			LinkInfo.AngularBreak = tb.GetReal();
			tb++;

			pData->JointList.push_back( LinkInfo );	
		}
		else if( _stricmp( "BoundingBoxInfo", tb.GetString()) == 0)
		{
			tb++;			
			pData->bUseBoundingBox = true;
			pData->BoundingBox[0].x = tb.GetReal(); tb++;
			pData->BoundingBox[0].y = tb.GetReal(); tb++;
			pData->BoundingBox[0].z = tb.GetReal(); tb++;
			pData->BoundingBox[1].x = tb.GetReal(); tb++;
			pData->BoundingBox[1].y = tb.GetReal(); tb++;
			pData->BoundingBox[1].z = tb.GetReal(); tb++;
		}
		else
		{
			DebugString( "Invalid Physics Objects Info!!!\n" );
			return ;
		}
	}
}

void LoadResourceFn::load(const char *pFileName, PHYSICS_COLLISION_CONTAINER *pData)
{
	char cBinFileName[MAX_PATH];
	char cScriptFileName[MAX_PATH];

	sprintf( cBinFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), pFileName );
	sprintf( cScriptFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), pFileName );

	char* pStr = strrchr( cBinFileName, '.' );
	BsAssert( pStr );
	pStr[1] = NULL;
	//strcat( pStr, "bin" ); //aleksger - safe string
	strcat(cBinFileName, "bin"); 

	g_BsResChecker.AddResList( cBinFileName );

	TokenBuffer tb;
	if( tb.Load( cBinFileName ) == false ) {	
		BsAssert( 0 && "Binary 파일없음");
		return ;
	}

	while( 1 ) {
		if( tb.IsEnd() ) {
			break;
		}

		PHYSICS_COLLISION_CONTAINER::Elem collisionInfo;

		std::string szName;

		szName = tb.GetString();
		tb++;			
		collisionInfo.szLinkName = tb.GetString();
		tb++;
		collisionInfo.radius = tb.GetReal();
		tb++;

		if( _stricmp( "Sphere", szName.c_str()) == 0 ) {					
			collisionInfo.height = 0.0f;
		}
		else if( _stricmp( "Capsule", szName.c_str() ) == 0 ) {	
			collisionInfo.height = tb.GetReal();	
			tb++;
		}
		else {
			BsAssert( 0 && "Collision Mesh Format invalid");
		}

		for(int i=0; i<4; i++)
			for(int j=0; j<4; j++) {
				collisionInfo.matLocal.m[i][j] = tb.GetReal();
				tb++;
			}
			pData->CollisionList.push_back( collisionInfo );
	}
}



void CFcPhysicsLoader::LoadGlobalPhysicsSetting(const char *pFileName)
{
	Parser Parser;
	TokenList Tokenlist;

	char* pFullName = g_BsKernel.GetFullName( pFileName );
	g_BsResChecker.AddResList( pFullName );

	Parser.Create();
	Parser.EnableComments( true );
	if( !Parser.ProcessSource( g_BsKernel.GetCurrentDirectory(), pFileName, &Tokenlist ) )
	{
		DebugString( "Collision Group File Not Found!!!\n" );
		return ;
	}

	if( !Parser.ProcessHeaders( &Tokenlist ) )
	{
		DebugString( "Collision Group Info Process Failed!!!\n" );
		return ;
	}

	Parser.ProcessMacros( &Tokenlist );
	TokenList::iterator itr = Tokenlist.begin();

	

	while( 1 ) {
		if( itr == Tokenlist.end() ) {
			break;
		}

		const char *pContents = itr->GetVariable();
		itr++;	

		if(strcmp(pContents, "SPHERICAL_JOINT_SETTING") == 0) {	

			int nIndex = itr->GetInteger(); itr++;

			int bUseTwistLimit = itr->GetInteger(); itr++;
			float TwistMin = itr->GetReal(); itr++;
			float TwistMax = itr->GetReal(); itr++;
			
			int bUseSwingLimit = itr->GetInteger(); itr++;
			float SwingVal = itr->GetReal(); itr++;
			
			int bUseTwistSpring = itr->GetInteger(); itr++;
			float TwistSpringVal = itr->GetReal(); itr++;
			float TwistSpringDamper = itr->GetReal(); itr++;
			
			int bUseSwingSpring = itr->GetInteger(); itr++;
			float SwingSpringVal = itr->GetReal(); itr++;
			float SwingSpringDamper = itr->GetReal(); itr++;
			
			int bUseProjectionMode = itr->GetInteger(); itr++;
			float ProjectDistance = itr->GetReal(); itr++;

			NxSphericalJointDesc sphericalDesc;

			float fRestitution = 0.f;
			float fHardness = 0.f;

			if( bUseTwistLimit ) {
				sphericalDesc.flags |= NX_SJF_TWIST_LIMIT_ENABLED;
				sphericalDesc.twistLimit.low.value = TwistMin;
				sphericalDesc.twistLimit.low.restitution = fRestitution;
				sphericalDesc.twistLimit.low.hardness = fHardness;
				sphericalDesc.twistLimit.high.value = TwistMax;				
				sphericalDesc.twistLimit.high.restitution = fRestitution;
				sphericalDesc.twistLimit.high.hardness = fHardness;
			}
			if( bUseSwingLimit ) {
				sphericalDesc.flags |= NX_SJF_SWING_LIMIT_ENABLED;
				sphericalDesc.swingLimit.value = SwingVal;
				sphericalDesc.swingLimit.hardness = fHardness;
				sphericalDesc.swingLimit.restitution = fRestitution;
			}
			if( bUseTwistSpring ) {
				sphericalDesc.flags |= NX_SJF_TWIST_SPRING_ENABLED;
				sphericalDesc.twistSpring.spring = TwistSpringVal;
				sphericalDesc.twistSpring.damper = TwistSpringDamper;
				
			}
			if( bUseSwingSpring ) {
				sphericalDesc.flags |= NX_SJF_SWING_SPRING_ENABLED;
				sphericalDesc.swingSpring.spring = SwingSpringVal;
				sphericalDesc.swingSpring.damper = SwingSpringDamper;
			}
			if( bUseProjectionMode ) {
                sphericalDesc.projectionMode = NX_JPM_POINT_MINDIST;
				sphericalDesc.projectionDistance = ProjectDistance;
			}
			//g_BsKernel.GetPhysicsMgr()->AddSetting(nIndex, sphericalDesc);
		}
		else if (strcmp(pContents, "SELECT_JOINT") == 0) {
			const char *szFileName = itr->GetString(); itr++;
			int nIndex = itr->GetInteger(); itr++;
			//g_BsKernel.GetPhysicsMgr()->AddSettingSelect(szFileName, nIndex);
		}
		else if(strcmp(pContents, "COLLISION_GROUP") == 0) {
			const char *pName1 = itr->GetString(); itr++;	
			const char *pName2 = itr->GetString(); itr++;	
			bool bCollide = itr->GetBoolean(); itr++;	
			g_BsKernel.GetPhysicsMgr()->SetGroupCollision( pName1, pName2, bCollide );

		}	

	}
}

