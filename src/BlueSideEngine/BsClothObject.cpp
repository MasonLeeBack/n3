#include "StdAfx.h"
#include "BsKernel.h"
#include "BsMesh.h"
#include "BsAniObject.h"
#include "BsClothObject.h"
#include "BsPhysicsMgr.h"

std::vector< int > CBsClothObject::m_ClothHandleList;

CBsClothObject::CBsClothObject()
{		
	SetObjectType(BS_SIMULATE_OBJECT);	
	m_nLinkCount = 0;
	m_pParentAni = NULL;	

	m_nLastUseTick = -1;

	m_bHair = false;
	m_bCloth = false;
	m_bSleep = false;
	m_nMoveCorrectionMode = 0;
	SetShadowCastType(BS_SHADOW_BUFFER);

	m_pSaveMatrix[ 0 ] = NULL;
	m_pSaveMatrix[ 1 ] = NULL;
	memset(m_nLinkToBoneIndex, 0, sizeof(int) * 50);

	m_SleepedBoxCenter = BSVECTOR(0,0,0);
	m_SleepedBoxExtend = BSVECTOR(1,1,1);

	m_pDummyActor = NULL;

	m_prevPos = D3DXVECTOR3(0,0,0);
	m_prevDir = D3DXVECTOR3(0,0,1);
	
	m_bExceptRotationCheck = false;
}

CBsClothObject::~CBsClothObject()
{
	BsAssert( m_ActorList.empty() );
	BsAssert( m_JointList.empty() );
	BsAssert( m_pDummyActor == NULL );
}

void CBsClothObject::ReInit()
{
	CBsObject::ReInit();
	m_pMesh->StoreBoneList();
}

void CBsClothObject::Release()
{
	SAFE_DELETE_PVEC(m_ActorList);
	SAFE_DELETE( m_pDummyActor );
	SAFE_DELETE_PVEC(m_JointList);
}

bool CBsClothObject::GetBox3( Box3 & B )
{
	if( m_ActorList.empty() ) {
		if( m_SleepedActorMatrixList.empty() ) {
			return false;
		}
		else {			
			B.E[0] = m_SleepedBoxExtend.x;
			B.E[1] = m_SleepedBoxExtend.y;
			B.E[2] = m_SleepedBoxExtend.z;

			B.C = m_SleepedBoxCenter;
			m_vecCenter = *(D3DXVECTOR3*)&m_SleepedBoxCenter;

			B.A[0] = BSVECTOR(1,0,0);
			B.A[1] = BSVECTOR(0,1,0);
			B.A[2] = BSVECTOR(0,0,1);
			
			return true;
		}
	}
	else {
		AABB clothAABB;

		clothAABB.Reset();

		int i, nCount;
		
		nCount = m_ActorList.size();
		float fMaxExtent = 0.f;

		for(i = 0; i < nCount; i++) {

			D3DXVECTOR3 Pos;

			IBsPhysicsActor *pBox = m_ActorList[i];
			if( pBox->IsKinematic()) continue;

			pBox->GetGlobalPosition( Pos );

			fMaxExtent = __max(fMaxExtent, pBox->GetBoxExtend().x);
			fMaxExtent = __max(fMaxExtent, pBox->GetBoxExtend().y);
			fMaxExtent = __max(fMaxExtent, pBox->GetBoxExtend().z);

			BSVECTOR v;
			v.x = Pos.x;
			v.y = Pos.y;
			v.z = Pos.z;
			clothAABB.Merge( v );
		}

		clothAABB.Vmin -= BSVECTOR(fMaxExtent, fMaxExtent, fMaxExtent);
		clothAABB.Vmax += BSVECTOR(fMaxExtent, fMaxExtent, fMaxExtent);

		BSVECTOR vExtent = clothAABB.GetExtent();
		B.E[0] = vExtent.x;
		B.E[1] = vExtent.y;
		B.E[2] = vExtent.z;

		B.C = clothAABB.GetCenter();

		m_vecCenter = *(D3DXVECTOR3*)&B.C;

		B.A[0] = BSVECTOR(1,0,0);
		B.A[1] = BSVECTOR(0,1,0);
		B.A[2] = BSVECTOR(0,0,1);

		return true;
	}
}

int CBsClothObject::AttachMesh( CBsMesh* pMesh )
{
	pMesh->StoreBoneList();

	if( _stricmp(pMesh->GetMeshFileName(), "c_dp_gb1_mantle.bm") == 0 ) {	// N3 전용 임시 코드, Code Clean 시 삭제
		m_bExceptRotationCheck = true;
	}

	int nResult = CBsObject::AttachMesh( pMesh );
	return nResult;
}

// 부셔지는 절벽을 위한 전용코드
void CBsClothObject::MakeActorToMesh()
{
	int i, j;
	
	D3DXMATRIX matObject = *GetObjectMatrixByProcess() ;

	D3DXVECTOR3 Scale;
	Scale.x = D3DXVec3Length((D3DXVECTOR3*)&matObject._11);
	Scale.y = D3DXVec3Length((D3DXVECTOR3*)&matObject._21);
	Scale.z = D3DXVec3Length((D3DXVECTOR3*)&matObject._31);
	
	int	*pBoneBuffer;
	int nVertexCount;
	int nFaceCount;
	WORD *pFaceBuffer;
	D3DXVECTOR3 *pPosition;
	m_pMesh->GetStreamBuffer( &pPosition, &pFaceBuffer, nVertexCount, nFaceCount );
	m_pMesh->GetStreamBufferExt( NULL, NULL, &pBoneBuffer, nVertexCount );

    std::vector< std::vector< int > > FaceData;
	std::vector< std::vector< D3DXVECTOR3 > > VertexData;
	std::vector< int > VertexIndex;
	std::map< int , int > VertexMap;

	int nBoneCount = GetBoneCount();
	FaceData.resize( nBoneCount );
	VertexIndex.resize( nBoneCount );
	VertexData.resize( nBoneCount );
	std::fill( VertexIndex.begin(), VertexIndex.end(), 0);

	for( i = 0; i < nFaceCount; i++)
	{
		for( j = 0; j < 3; j++)
		{
			int nFace = pFaceBuffer[ i * 3 + j ];
			int nBoneIndex = pBoneBuffer[ nFace ] >> 16;
			BsAssert( nBoneIndex >= 0 && nBoneIndex < nBoneCount);
			if( VertexMap.count( nFace ) == 0 ) {
				int nVertexIndex = VertexIndex[ nBoneIndex ];
				VertexMap.insert( std::map< int, int>::value_type( nFace,  nVertexIndex));
				FaceData[ nBoneIndex ].push_back( nVertexIndex );
				D3DXVECTOR3 Pos = pPosition[ nFace ];
				Pos.x *= Scale.x;
				Pos.y *= Scale.y;
				Pos.z *= Scale.z;
				VertexData[ nBoneIndex ].push_back( Pos );
				VertexIndex[ nBoneIndex ]++;
			}
			else
			{
				FaceData[ nBoneIndex ].push_back( VertexMap[ nFace ] );
			}
		}
	}

	D3DXMATRIX matInvScale;
	D3DXMatrixScaling(&matInvScale, 1 / Scale.x, 1 / Scale.y, 1 / Scale.z);
	D3DXMatrixMultiply(&matObject, &matInvScale, &matObject);

	for( i = 0; i < GetBoneCount(); i++)
	{
		IBsPhysicsActor *pActorOrigin = m_ActorList[ m_RenderBoneIndex[i] ];
		if( _stricmp( pActorOrigin->GetBoneName(), "B_Root") == 0 ) {
			continue;
		}
		IBsPhysicsActor *pNewActor = CBsPhysicsTriangleMesh::Create( &VertexData[ i ].front(), &FaceData[ i ].front(), VertexData[ i ].size(), FaceData[ i ].size() / 3, matObject );

		D3DXMATRIX matIdent;
		D3DXMatrixIdentity(&matIdent);

		pNewActor->SetWorldMat( matIdent );
		pNewActor->SetInvWorldMat( matIdent );
		pNewActor->SetBoneName( pActorOrigin->GetBoneName() );

		pNewActor->SetGroup("BreakableProp");
		
		m_ActorList[ m_RenderBoneIndex[i] ] = pNewActor;
		delete pActorOrigin;
	}

	D3DXMATRIX matPlane = matObject;
	matPlane._42 -= 1000.f;

	m_pDummyActor = CBsPhysicsBox::Create( D3DXVECTOR3(5000.f, 100.f, 5000.f), matPlane );
	m_pDummyActor->SetKinematic();
	m_pDummyActor->SetGroup("BreakableProp");
    
	delete [] pPosition;
	delete [] pBoneBuffer;
	delete [] pFaceBuffer;
}

bool CBsClothObject::CreateActors( PHYSICS_DATA_CONTAINER *pPhysicsInfo)
{
	int i, nCount;

	D3DXVECTOR3 Scale;

	D3DXMATRIX matObject = *GetObjectMatrixByProcess() ;

	Scale.x = D3DXVec3Length((D3DXVECTOR3*)&matObject._11);
	Scale.y = D3DXVec3Length((D3DXVECTOR3*)&matObject._21);
	Scale.z = D3DXVec3Length((D3DXVECTOR3*)&matObject._31);

	D3DXMATRIX matInvScale;
	D3DXMatrixScaling(&matInvScale, 1 / Scale.x, 1 / Scale.y, 1 / Scale.z);
	D3DXMatrixMultiply(&matObject, &matInvScale, &matObject);

	nCount = ( int )pPhysicsInfo->ActorList.size();	

	bool bTroll = false;

	for( i = 0; i < nCount; i++ )
	{
		const char *szBoneName = pPhysicsInfo->ActorList[i].szLinkBoneName.c_str();
		if( strstr(szBoneName, "Head" )) m_bHair = true;
		else if( strcmp(szBoneName , "static") && strcmp(szBoneName, "NotCol") && szBoneName[0] != NULL) m_bCloth = true;
		if( strstr( szBoneName, "Pelvis") && nCount == 25) {
			bTroll = true;
		}
	}

	std::map< std::string, IBsPhysicsActor* > actorNameMap;
	for( i = 0; i < nCount; i++ )
	{
		PHYSICS_DATA_CONTAINER::ActorElem ActorInfo = pPhysicsInfo->ActorList[i];

		IBsPhysicsActor *pActor;

		D3DXMATRIX matGlobal, matWorld;

		matWorld = ActorInfo.WorldMat;
		matWorld._41 *= Scale.x;
		matWorld._42 *= Scale.y;
		matWorld._43 *= Scale.z;

		D3DXMatrixMultiply(&matGlobal, &matWorld, &matObject);

		float Radius = ActorInfo.fRadius * Scale.x;
		float Height = ActorInfo.fHeight * Scale.y;

		D3DXVECTOR3 Size;
		Size.x = ActorInfo.Size.x * Scale.x;
		Size.y = ActorInfo.Size.y * Scale.y;
		Size.z = ActorInfo.Size.z * Scale.z;

		switch( ActorInfo.nShapeType )
		{
		case PHYSICS_DATA_CONTAINER::ActorElem::SHAPE_SPHERE: 
			if( m_bCloth || m_bHair) {
				pActor = CBsPhysicsClothCapsule::Create( Radius, 0.f, matGlobal);
			}
			else {
				pActor = CBsPhysicsCapsule::Create( Radius, 0.f, matGlobal);
			}
			break;
		case PHYSICS_DATA_CONTAINER::ActorElem::SHAPE_CAPSULE:	
			if( m_bCloth || m_bHair) {			
				pActor = CBsPhysicsClothCapsule::Create( Radius, Height, matGlobal);
			}
			else {
				pActor = CBsPhysicsCapsule::Create( Radius, Height, matGlobal);
			}
			break;
		case PHYSICS_DATA_CONTAINER::ActorElem::SHAPE_BOX: 
			if( m_bCloth || m_bHair) {
				if( m_bHair ) {
					pActor = CBsPhysicsClothBox::Create( Size, matGlobal , 1);
				}
				else {
					pActor = CBsPhysicsClothBox::Create( Size, matGlobal , 0);
				}				
			}
			else {
				pActor = CBsPhysicsPropBox::Create( Size, matGlobal );
			}
			break;
		default:
			BsAssert( 0 );
		}		
		if( bTroll ) {
			if( pActor->GetActor() ) {
				pActor->GetActor()->raiseActorFlag( NX_AF_DISABLE_COLLISION );
			}
		}
		pActor->SetScale(Scale);
		pActor->SetInvWorldMat( ActorInfo.InverseWorldMat );
		pActor->SetWorldMat( ActorInfo.WorldMat);
		pActor->SetBoneName( (char *)ActorInfo.szBoneName.c_str() );
		if( !ActorInfo.szLinkBoneName.empty())  {

			if(!strcmp(ActorInfo.szLinkBoneName.c_str(), "NotCol")) {
				if(pActor->GetActor())
					pActor->GetActor()->raiseActorFlag( NX_AF_DISABLE_COLLISION );
			}
			else {
				pActor->SetKinematic();
				pActor->SetLinkName(ActorInfo.szLinkBoneName);
				if(pActor->GetActor())
					pActor->GetActor()->raiseActorFlag( NX_AF_DISABLE_COLLISION );
			}
		}

		if( _stricmp(ActorInfo.szBoneName.c_str(), "B_Root") == 0 ) {
			pActor->SetKinematic();
		}

		if( !ActorInfo.szGroupName.empty()) {
			pActor->SetGroup( ActorInfo.szGroupName.c_str() );
		}

		actorNameMap[ ActorInfo.szBoneName ] = pActor;

		m_ActorList.push_back( pActor );
	}

	
	
	int nStartIteration = 14;

	if( m_bHair ) {
		nStartIteration = 10;
	}

	if( g_BsKernel.GetPhysicsMgr()->GetCustomValue() == CBsPhysicsMgr::CUSTOM_MYIFEE_CHAIN ) {
		nStartIteration = 17;
	}

	int nCurrentIterationCount = nStartIteration;

	m_ActorList[0]->GetActor()->setSolverIterationCount( nCurrentIterationCount );

	for( nCurrentIterationCount = nStartIteration; nCurrentIterationCount >= 1 ; nCurrentIterationCount--) 
	{
		bool bUpdate = false;
		nCount = ( int )pPhysicsInfo->JointList.size();
		for( i = 0; i < nCount; i++ )
		{
			PHYSICS_JOINT_INFO &JointInfo = pPhysicsInfo->JointList[ i ];
			IBsPhysicsActor *pActor1 = actorNameMap[ JointInfo.szBoneName1 ];//FindActor(JointInfo.szBoneName1.c_str());
			IBsPhysicsActor *pActor2 = actorNameMap[ JointInfo.szBoneName2 ];//FindActor(JointInfo.szBoneName2.c_str());
			
			if( (m_bCloth||m_bHair) && pActor1 != NULL && pActor2 != NULL )
			{
				NxActor * pActor1Nx = pActor1->GetActor();
				NxActor * pActor2Nx = pActor2->GetActor();
				BsAssert( NULL != pActor1Nx && NULL != pActor2Nx );

				float fPower = 0.6f;			

				if(pActor1Nx->getSolverIterationCount() == nCurrentIterationCount &&
					pActor2Nx->getSolverIterationCount() == 1 ) 
				{
					JointInfo.nLevel = nCurrentIterationCount;
					pActor2Nx->setSolverIterationCount( BsMax(1,nCurrentIterationCount - 1) );
					pActor2Nx->setMass(pActor2Nx->getMass() * powf(fPower, (float)(1 + nStartIteration - nCurrentIterationCount)) );
					bUpdate = true;
				}
			}
		}
		if( !bUpdate ) break;
	}

		
	if( m_bHair ) {
		nCount = m_ActorList.size();
		for( i = 0; i < nCount; i++) {		
			NxActor *pNxActor = m_ActorList[i]->GetActor();
			if( pNxActor ) {
				int nSolverCount = 1 + pNxActor->getSolverIterationCount() - nCurrentIterationCount;
				nSolverCount = BsMax( 1, nSolverCount );				
				pNxActor->setSolverIterationCount( nSolverCount );
				if( nSolverCount >= 5 ) {					
					pNxActor->clearBodyFlag( NX_BF_DISABLE_GRAVITY );
				}				
			}			
		}
	}

	int nSelect = 0;
	if( m_bHair ) nSelect = 3;
	if( m_bCloth ) nSelect = 4;			

	nCount = ( int )pPhysicsInfo->JointList.size();
	for( i = 0; i < nCount; i++ )
	{
		PHYSICS_JOINT_INFO JointInfo = pPhysicsInfo->JointList[ i ];
		
		IBsPhysicsActor *pActor1 = actorNameMap[ JointInfo.szBoneName1 ];//FindActor(JointInfo.szBoneName1.c_str());
		IBsPhysicsActor *pActor2 = actorNameMap[ JointInfo.szBoneName2 ];//FindActor(JointInfo.szBoneName2.c_str());

		D3DXVECTOR3 AnchorPos, LocalPos;

		LocalPos = JointInfo.AnchorPos;

		LocalPos.x *= Scale.x;
		LocalPos.y *= Scale.y;
		LocalPos.z *= Scale.z;

		D3DXVec3TransformCoord(&AnchorPos, &LocalPos, &matObject);

		if ( bTroll ) nSelect = 777;

		int nLevel = JointInfo.nLevel;

		if( m_bHair ) {
			nLevel -= nCurrentIterationCount;
		}
		IBsPhysicsJoint *pJoint = g_BsKernel.GetPhysicsMgr()->CreateJoint( pActor1, pActor2, JointInfo, AnchorPos, nSelect, nLevel );
		// breakable
		if(JointInfo.bBreakable) {
			pJoint->SetBreakable( JointInfo.LinearBreak, JointInfo.AngularBreak);
		}
		m_JointList.push_back( pJoint );
	}

	int nBoneCount = m_pMesh->GetStoredBoneCount();
	if( nBoneCount == 0) {
		char str[255];
		sprintf(str, " 뿌셔지는 프랍 데이타 잘못 %s\n", m_pMesh->GetMeshFileName());
		DebugString( str );
	}
	m_RenderBoneIndex.resize( nBoneCount );

	std::fill(m_RenderBoneIndex.begin(), m_RenderBoneIndex.end(), -1);

	nCount = m_ActorList.size();
	bool bError = false;

	for(i = 0; i < nCount; i++) {
		IBsPhysicsActor *pActor = m_ActorList[i];
		int nBoneIndex = m_pMesh->GetStoredBoneIndex( pActor->GetBoneName() );

		if ( nBoneIndex < 0 || nBoneIndex >= nBoneCount ) { 
			bError = true;
			continue;
		}

		m_RenderBoneIndex[ nBoneIndex ] = i;
	}

	if( m_bCloth || m_bHair )
		SetSleep( true );

	return true;
}


void CBsClothObject::ProcessLinkedPhysicsObject( DWORD dwParam1, DWORD dwParam2 )
{
	int i;
	int nTick = g_BsKernel.GetRenderTickByProcess();
	
	if( m_nLastUseTick <  nTick - 1 ) {
		SetSleep( true );
		m_nMoveCorrectionMode = 4;
		return ;
	}
	else {
		SetSleep( false );
	}

	D3DXVECTOR3 vCurrentPos = *(D3DXVECTOR3*)(dwParam1);
	D3DXVECTOR3 vCurrentDir = *(D3DXVECTOR3*)(dwParam2);
	if( D3DXVec3LengthSq( &(m_prevPos - vCurrentPos)) > 300.f * 300.f ||
		(D3DXVec3Dot(&vCurrentDir, &m_prevDir) < 0.f && !m_bExceptRotationCheck) ) {
		m_nMoveCorrectionMode = 4;
	}

	m_prevPos = vCurrentPos;
	m_prevDir = vCurrentDir;

	for( i = 0; i < m_nLinkCount; i++) {	

		CBsBone *pBone = m_pParentAni->FindBone( m_nLinkToBoneIndex[ i ] );
		BSMATRIX matLink;
		BSMATRIX matObject((FLOAT*)GetObjectMatrixByProcess());
		BsMatrixMultiply(&matLink, pBone->GetTransMatrix(), &matObject);
		BsMatrixMultiply(&matLink, pBone->GetInvWorldMatrix(), &matLink);
		IBsPhysicsActor *pActor = m_ActorList [ m_nLinkFromBoneIndex[ i ] ];
		pActor->SetTransform( (D3DXMATRIX&)matLink );
	}
}

bool CBsClothObject::InitRender(float fDistanceFromCamera)
{	
	SetDistanceFromCam( fDistanceFromCamera );

	m_nLastUseTick = g_BsKernel.GetRenderTickByProcess();
	m_ClothHandleList.push_back( GetKernelPoolIndex() );

	return true;
}

void CBsClothObject::PostInitRender()
{
	int i, nCount;
	BSMATRIX mat;
	BSMATRIX matBone[200];

	BSMATRIX matObject((FLOAT*)GetObjectMatrixByProcess());

	if( m_SleepedActorMatrixList.empty() == false ) {
		nCount = m_SleepedActorMatrixList.size();
		memcpy( matBone, &m_SleepedActorMatrixList[0], sizeof(BSMATRIX) * nCount );
	}
	else {

		if( m_nMoveCorrectionMode > 1 ) {
			if( m_bSleep ) {
				SetSleep( false );
			}
			CBsBone *pBone = m_pParentAni->FindBone( m_nLinkToBoneIndex[ 0 ] );
			BSMATRIX matLink;
			BsMatrixMultiply(&matLink, pBone->GetInvWorldMatrix(), pBone->GetTransMatrix());
			BsMatrixMultiply(&matLink, &matLink, &matObject);

			nCount = m_ActorList.size();
			for( i = 0; i < nCount; i++)	 {			
				matBone[i] = matLink;
				IBsPhysicsActor *pActor = m_ActorList [ i ];	
				pActor->SetKinematic(true);
				pActor->SetTransform( (D3DXMATRIX&)matLink );
			}
			m_nMoveCorrectionMode--;
		}
		else if( m_nMoveCorrectionMode == 1) {

			CBsBone *pBone = m_pParentAni->FindBone( m_nLinkToBoneIndex[ 0 ] );
			BSMATRIX matLink;
			BsMatrixMultiply(&matLink, pBone->GetInvWorldMatrix(), pBone->GetTransMatrix());
			BsMatrixMultiply(&matLink, &matLink, &matObject);

			nCount = m_ActorList.size();
			for( i = 0; i < nCount; i++)	 {

				matBone[i] = matLink;
                int j;	            			
				for( j = 0; j < m_nLinkCount; j++ ) {
					if( m_nLinkFromBoneIndex[ j ] == i ) break;
				}
				IBsPhysicsActor *pActor = m_ActorList [ i ];
				if( j != m_nLinkCount){
					pActor->SetTransform( (D3DXMATRIX&)matLink );					
				}
				else {
					pActor->SetKinematic(false);
				}
			}
			m_nMoveCorrectionMode --;
		}
		else {
			nCount = m_ActorList.size();
			for( i = 0; i < nCount; i++) {			
				m_ActorList[ i ]->GetTransform( (D3DXMATRIX&)matBone[i] );
			}		
		}
	}


	nCount = m_RenderBoneIndex.size();

	BSMATRIX matInvObject;
	BsMatrixInverse(&matInvObject, &matObject );

	AllocProcessSaveMatrixBuffer();
	for( i = 0; i < nCount; i++) {
		if( m_RenderBoneIndex[ i ] == -1 ) {
			BsMatrixIdentity(&mat);
			SaveMatrix( ( D3DXMATRIX * )&mat, i );
			continue;
		}
		mat = matBone[  m_RenderBoneIndex[ i ] ];
		BsMatrixMultiply(&mat, &mat, &matInvObject);			
		SaveMatrix( ( D3DXMATRIX * )&mat, i );
	}
}

void CBsClothObject::Render(C3DDevice *pDevice)
{
	if( !m_bSleep ) {
		CBsObject::Render(pDevice);
	}
}

void CBsClothObject::RenderShadow(C3DDevice *pDevice)
{
	if( !m_bSleep ) {
		CBsObject::RenderShadow(pDevice);
	}
}

void CBsClothObject::Show( bool bIsShow )
{
	m_bShow = bIsShow;
}

void CBsClothObject::LinkCharacter(int nCharIndex )
{
	int i, j, nCount;
	char szBoneName[200][256];	

	int nBoneCount = g_BsKernel.SendMessage( nCharIndex, BS_GET_BONE_NAME_LIST, (DWORD)szBoneName);    
	BsAssert(nBoneCount < 200);

	m_pParentAni = ((CBsAniObject*)g_BsKernel.GetEngineObjectPtr( nCharIndex ))->Get_pAni();

	nCount = m_ActorList.size();
	for(i = 0; i < nCount; i++) {
		IBsPhysicsActor *pActor = m_ActorList[i];		
		pActor->SetGroup("Cloth");
		if(pActor->IsKinematic()) {
			for( j = 0; j < nBoneCount; j++) {
				if( _stricmp(szBoneName[j], pActor->GetLinkBoneName()) == 0 ) {
					m_nLinkToBoneIndex[ m_nLinkCount ] = j;
					m_nLinkFromBoneIndex[ m_nLinkCount ] = i;
					m_nLinkCount++;
					BsAssert( m_nLinkCount <= 50 );
					break;
				}
			}
			if( j == nBoneCount ) {
				DebugString("Novodex Error : Link 되는본 찾을 수 없음\n");
			}			
		}
	}

	g_BsKernel.SendMessage( nCharIndex, BS_LINKOBJECT, 0, GetKernelPoolIndex());

}

void CBsClothObject::AddForce(D3DXVECTOR3 *Force, D3DXVECTOR3 *pPos )
{
	if( m_bHair ) return;
	float fPower = D3DXVec3Length( Force );
	fPower = BsMin( fPower, 20.f );
	D3DXVECTOR3 NewForce ;
	D3DXVec3Normalize(&NewForce, Force );

	D3DXVECTOR3 HitPos = D3DXVECTOR3( 0.0f, 100.0f, 0.0f );

	if( pPos ) {
		HitPos += *pPos;
	}
	else {
		Box3 B;
		GetBox3(B);
		HitPos.x += B.C.x;
		HitPos.y += B.C.y;
		HitPos.z += B.C.z;
	}

	int i, nSize;
	nSize = m_ActorList.size();
	for(i = 0; i < nSize; i++) {
		D3DXMATRIX Mat;
		m_ActorList[i]->GetGlobalPose( Mat );
		D3DXVECTOR3 DirActor = *(D3DXVECTOR3*)(&Mat._41 )- HitPos;
		D3DXVec3Normalize( &DirActor, &DirActor);
		D3DXVECTOR3 vAngular;
		D3DXVec3Cross( &vAngular, &NewForce, &DirActor);

		D3DXVECTOR3 vForce = NewForce;
		vForce.y += ( rand() % 1000 ) / 1000.f  - 0.5f;
		D3DXVec3Normalize( &vForce, &vForce);

		float fAngPower = 0.5f + 0.5f * ( rand() % 1000 ) / 1000.f;

		m_ActorList[i]->AddVelocity( &(vForce * fPower ), &(vAngular * fPower * fAngPower)  );
	}
}

void CBsClothObject::SetActorMatrix(char *szBoneName, D3DXMATRIX *pMatrix)
{
	int i, nSize;
	nSize = m_ActorList.size();
	D3DXMATRIX matObject = *GetObjectMatrixByProcess();
	for( i = 0; i < nSize; i++) {
		if(!_stricmp(m_ActorList[i]->GetBoneName(), szBoneName)) {			
			if(m_ActorList[i]->GetActor()) {

				D3DXMATRIX matTrans;
				D3DXMatrixMultiply(&matTrans, pMatrix, &matObject);
				m_ActorList[i]->SetGlobalPose( matTrans );
			}
			return;
		}
	}
	//BsAssert( 0 && " CBsClothObject::SetMatrix Failed ");
}

void CBsClothObject::HideCrushActors()
{
	AABB clothAABB;
	clothAABB.Reset();
	float fMaxExtent = 0.f;

	int i, nSize;
	nSize = m_ActorList.size();
	for( i = 0; i < nSize; i++) {	
		D3DXMATRIX Mat;
		m_ActorList[ i ]->GetTransform( Mat );
		BSVECTOR Pos;
		IBsPhysicsActor *pBox = m_ActorList[i];
		if( !pBox->IsKinematic()) {

			pBox->GetGlobalPosition( *(D3DXVECTOR3*)&Pos );

			fMaxExtent = __max(fMaxExtent, pBox->GetBoxExtend().x);
			fMaxExtent = __max(fMaxExtent, pBox->GetBoxExtend().y);
			fMaxExtent = __max(fMaxExtent, pBox->GetBoxExtend().z);

			clothAABB.Merge( Pos );
			Mat._42 += 100000.f;
		}
		m_SleepedActorMatrixList.push_back( Mat );
	}

	clothAABB.Vmin -= BSVECTOR(fMaxExtent, fMaxExtent, fMaxExtent);
	clothAABB.Vmax += BSVECTOR(fMaxExtent, fMaxExtent, fMaxExtent);

	m_SleepedBoxCenter = clothAABB.GetCenter();
	m_SleepedBoxExtend = clothAABB.GetExtent();

	Release();
}

int CBsClothObject::ProcessMessage(DWORD dwCode, DWORD dwParam1/*=0*/, DWORD dwParam2/*=0*/, DWORD dwParam3/*=0*/)
{

	switch(dwCode){		
		case BS_PHYSICS_LINK_CHARACTER:
			LinkCharacter(dwParam1);
			return 1;
		case BS_PHYSICS_ADDFORCE:
			AddForce((D3DXVECTOR3*)dwParam1, (D3DXVECTOR3*) dwParam2 );
			return 1;	
		case BS_PHYSICS_MAKE_SLEEP:
            MakeSleep();            
			return 1;
		case  BS_PHYSICS_HIDE_CRUSH_ACTOR:
			HideCrushActors();
			return 1;	
		case  BS_PHYSICS_SET_ACTOR_MATRIX:
			SetActorMatrix((char*)dwParam1, (D3DXMATRIX*)dwParam2);
			return 1;
		default:
			break;
	}

	return CBsObject::ProcessMessage(dwCode, dwParam1, dwParam2, dwParam3);
}

void CBsClothObject::SetShapeUserData(void *pData)
{
	int i, nSize;
	nSize = m_ActorList.size();

	for(i = 0; i < nSize; i++) {
		m_ActorList[i]->SetShapeUserData( pData );
	}
}

void CBsClothObject::SetGroup(char *szName)
{
	int i, nSize;
	nSize = m_ActorList.size();

	for(i = 0; i < nSize; i++) {
		m_ActorList[i]->SetGroup( szName);
	}
}

IBsPhysicsActor* CBsClothObject::FindActor(const char *szBoneName)
{
	int nCount = ( int )m_ActorList.size();
	for(int i = 0; i < nCount; i++ ) {
		if(strcmp(szBoneName, m_ActorList[i]->GetBoneName()) == 0) {
			return m_ActorList[i];
		}
	}
	return NULL;
}

void CBsClothObject::SetSleep( bool bSleep )
{

	if( m_bSleep == bSleep ) return;

	int i, nSize;
	nSize = m_ActorList.size();

	for(i = 0; i < nSize; i++) {
		NxActor *pActor = m_ActorList[i]->GetActor();
		if( !pActor ) continue;
		if( !pActor->isDynamic() ) continue;
		if(bSleep){
			if( !pActor->isSleeping() )	{
				pActor->putToSleep();			
				pActor->raiseActorFlag( NX_AF_DISABLE_COLLISION );
			}
		}
		else {
			if(pActor->isSleeping()) {				
				pActor->clearActorFlag( NX_AF_DISABLE_COLLISION );
				pActor->wakeUp(FLT_MAX);								
			}
		}
	}
	m_bSleep = bSleep;
}

void CBsClothObject::MakeSleep()
{
	if( m_SleepedActorMatrixList.empty() == false ) return;


	AABB clothAABB;
	clothAABB.Reset();
	float fMaxExtent = 0.f;

	int i, nSize;
	nSize = m_ActorList.size();
	for( i = 0; i < nSize; i++) {	
		D3DXMATRIX Mat;
		m_ActorList[ i ]->GetTransform( Mat );
		m_SleepedActorMatrixList.push_back( Mat );
		///
		BSVECTOR Pos;
		IBsPhysicsActor *pBox = m_ActorList[i];
		if( pBox->IsKinematic()) continue;

		pBox->GetGlobalPosition( *(D3DXVECTOR3*)&Pos );

		fMaxExtent = __max(fMaxExtent, pBox->GetBoxExtend().x);
		fMaxExtent = __max(fMaxExtent, pBox->GetBoxExtend().y);
		fMaxExtent = __max(fMaxExtent, pBox->GetBoxExtend().z);

		clothAABB.Merge( Pos );
	}

	clothAABB.Vmin -= BSVECTOR(fMaxExtent, fMaxExtent, fMaxExtent);
	clothAABB.Vmax += BSVECTOR(fMaxExtent, fMaxExtent, fMaxExtent);

	m_SleepedBoxCenter = clothAABB.GetCenter();
	m_SleepedBoxExtend = clothAABB.GetExtent();

	Release();
}


void CBsClothObject::PostInitRenderObjects()
{
	int i, nSize;

	nSize = m_ClothHandleList.size();

	for( i = 0 ; i < nSize; i++) {
		int nClothHandle = m_ClothHandleList[i];
		CBsClothObject *pClothObj = (CBsClothObject*)g_BsKernel.GetEngineObjectPtr(nClothHandle);
		if( pClothObj && pClothObj->GetObjectType() == CBsObject::BS_SIMULATE_OBJECT ) {
			pClothObj->PostInitRender();
		}
	}

	m_ClothHandleList.erase(m_ClothHandleList.begin(), m_ClothHandleList.end());

}
