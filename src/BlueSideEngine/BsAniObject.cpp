#include "stdafx.h"
#include "BsAniObject.h"
#include "BsMesh.h"
#include "BsKernel.h"
#include "BsParticle.h"
#include "BsPhysicsMgr.h"
#include "BsClothObject.h"
#include "BsAniCache.h"
#include "BsBillboardMgr.h"
#include "BsCommon.h"
#include "BsInstancingMgr.h"

#define _OPT_SHINJICH_BSANIOBJECT_CPP	0

CBsSaveMatrix CBsAniObject::s_GlobalSaveMatrixBuffer[ DOUBLE_BUFFERING ];

CBsAniObject::CBsAniObject()
{
	SetObjectType(BS_ANIMATION_OBJECT);
	// Ani Setting
	m_nAniFileIndex = -1;
	m_pAni=NULL;
	
	m_nLinkObjectCount = 0;
	m_nLinkParticleCount = 0;

	ResetAniIndex();
	SetShadowCastType(BS_SHADOW_BUFFER);

	m_nCalcAniPosition = BS_CALC_POSITION_Y;
	m_BoneRotation.reserve( MIN_BONE_ROTATION_COUNT );

	D3DXMatrixIdentity(&m_matProcessRootBone);

	m_bRagdollEnable=false;
	m_bSimulationMode=false;

	m_nRagdollCount = 0;	
	m_unlinkVelocity = D3DXVECTOR3(0,0,0);

	m_nCatchBoneIndex = -1;		

	m_nAniCacheType = 0;
	m_nAniFileIndex = 0;
	m_nBillBoardIndex = -1;

	m_pSaveMatrix[ 0 ] = NULL;
	m_pSaveMatrix[ 1 ] = NULL;

	m_bIsBillBoardRender[0] = false;
	m_bIsBillBoardRender[1] = false;

	m_bIsBillBoardRenderBefore = false;

	m_bEnableInstancing = false;
}

CBsAniObject::~CBsAniObject()
{
	/*
	for( int i = 0; i < m_nLinkObjectCount; i++ ) {		
		CBsObject *pObject = g_BsKernel.GetEngineObjectPtr(m_nLinkObjectHandle[i]);
		if( pObject ) {
			pObject->SetParentIndex( -1 );
		}
	}
	*/
}

void CBsAniObject::Release()
{
	int i, nSize;

	SAFE_DELETE_PVEC( m_BoneActors );
	SAFE_DELETE_PVEC( m_Joints );

	nSize = m_CollisionBody.size();
	for( i = 0; i < nSize; i++) {
		delete m_CollisionBody[i].pActor;
	}
	m_CollisionBody.clear();
}

void CBsAniObject::ResetAniIndex()
{
	m_nCurrentAniSize = 0;
	m_nBlendAniSize = 0;
}

void	CBsAniObject::PopBlend(void)
{
	if( m_nBlendAniSize > 0 )
	{
		m_nBlendAniSize--;
	}
}


void CBsAniObject::ReInit()
{
	CBsObject::ReInit();
	m_pMesh->LinkBone(m_pAni);
}

void CBsAniObject::LinkObjectByName(const char *pBoneName, int nObjectID)
{
	int nBoneIndex;

	nBoneIndex=m_pAni->FindBoneIndex(pBoneName);

	BsAssert( nBoneIndex != -1 );

	LinkObject(nBoneIndex, nObjectID);
}

void CBsAniObject::LinkObject(int nBoneIndex, int nObjectID)
{
	DebugString("CBsAniObject(%d) : Request Link Object(%d, %d)\n", GetKernelPoolIndex(), nBoneIndex, nObjectID);

	BsAssert(nObjectID!=-1);
	BsAssert(m_nLinkObjectCount<MAX_LINK_COUNT);

	if( nObjectID == -1)
		return;
	if( m_nLinkObjectCount>=MAX_LINK_COUNT)
		return;

	BsAssert( 0 <= nBoneIndex && nBoneIndex < m_pAni->GetBoneCount());
	
	g_BsKernel.GetEngineObjectPtr(nObjectID)->ResetProcessMatrix();

	KERNEL_COMMAND cmd;
	cmd.nCommand = OBJECT_COMMAND_LINKOBJECT;
	cmd.nBuffer[ 0 ] = GetKernelPoolIndex();
	cmd.nBuffer[ 1 ] = nBoneIndex;
	cmd.nBuffer[ 2 ] = nObjectID;

	g_BsKernel.GetPreKernelCommand().push_back(cmd);
}

const D3DXMATRIX* CBsAniObject::UnlinkObject(int nObjectID, D3DXVECTOR3 *unlinkVelocity )
{
	DebugString("CBsAniObject(%d) : Request Unlink Object(%d)\n", GetKernelPoolIndex(), nObjectID);

	BsAssert(nObjectID!=-1);

	// 래그돌인경우 Kinematic 으로 세팅되었던 Bone을 다시 Simulation 으로 바꿔줌
	CBsObject* pObject = g_BsKernel.GetEngineObjectPtr( nObjectID );
	BsAssert( pObject && "Unlink Fail : Invalid Object Index");
	if( pObject->IsRagdoll()) {				
		pObject->ProcessRagdollUnlink( unlinkVelocity );
	}
	KERNEL_COMMAND cmd;
	cmd.nCommand = OBJECT_COMMAND_UNLINKOBJECT;
	cmd.nBuffer[ 0 ] = GetKernelPoolIndex();
	cmd.nBuffer[ 1 ] = nObjectID;
	g_BsKernel.GetPreKernelCommand().push_back(cmd);

	return pObject->GetObjectMatrixByProcess();
}

void CBsAniObject::UnlinkBone(int nBoneID)
{
	DebugString("CBsAniObject(%d) : Request UnlinkBone(%d)\n", GetKernelPoolIndex(), nBoneID);
	KERNEL_COMMAND cmd;
	cmd.nCommand = OBJECT_COMMAND_UNLINKBONE;
	cmd.nBuffer[ 0 ] = GetKernelPoolIndex();
	cmd.nBuffer[ 1 ] = nBoneID;

	g_BsKernel.GetPreKernelCommand().push_back(cmd);
}

void CBsAniObject::ReqLinkObject(KERNEL_COMMAND* pCmd)
{
	DebugString("CBsAniObject(%d) : Link Object(%d, %d)\n", GetKernelPoolIndex(), pCmd->nBuffer[1], pCmd->nBuffer[2]);
	BsAssert( pCmd->nBuffer[0]== GetKernelPoolIndex() && "Error : Invalid Link Request!!");

	if( m_nLinkObjectCount >= MAX_LINK_COUNT )	// ReqLinkObject 에서 해줘야 제대로 처리가 됩니다.
		return;
	
	for (int i=0; i<m_nLinkObjectCount; i++) {
		if ( m_nLinkObjectHandle[i] == pCmd->nBuffer[2] && m_nLinkBoneIndex[i] == pCmd->nBuffer[1]  ) {
			return;
		}
	}
	m_nLinkBoneIndex[m_nLinkObjectCount]=pCmd->nBuffer[1];
	m_nLinkObjectHandle[m_nLinkObjectCount]=pCmd->nBuffer[2];

	CBsObject* pObject = g_BsKernel.GetEngineObjectPtr(pCmd->nBuffer[2]);
	pObject->SetRootObject(false);
	pObject->ResetRenderMatrix();

	//pObject->SetParentIndex( m_nKernelPoolIndex );

	m_nLinkObjectCount++;
}

void CBsAniObject::ReqUnlinkObject(KERNEL_COMMAND* pCmd)
{
	DebugString("CBsAniObject(%d) : Unlink Object(%d)\n", GetKernelPoolIndex(), pCmd->nBuffer[1]);
	int nObjectID = pCmd->nBuffer[1];
	for(int i=0;i<m_nLinkObjectCount;i++){
		if(m_nLinkObjectHandle[i]==nObjectID){
			memcpy(m_nLinkObjectHandle+i, m_nLinkObjectHandle+i+1, sizeof(int)*(m_nLinkObjectCount-i-1));
			memcpy(m_nLinkBoneIndex+i, m_nLinkBoneIndex+i+1, sizeof(int)*(m_nLinkObjectCount-i-1));
			m_nLinkObjectCount--;

			g_BsKernel.GetEngineObjectPtr(nObjectID)->SetRootObject(true);			// 확인 필요!! by jeremy

			//g_BsKernel.GetEngineObjectPtr(nObjectID)->SetParentIndex( -1 );

			return;
		}
	}
	return;
}

void CBsAniObject::ReqUnlinkBone(KERNEL_COMMAND* pCmd)
{
	DebugString("CBsAniObject(%d) : UnlinkBone(%d)\n", GetKernelPoolIndex(), pCmd->nBuffer[1]);
	int nBoneIndex = pCmd->nBuffer[1];
	for(int i=0;i<m_nLinkObjectCount;i++){
		if(m_nLinkBoneIndex[i]==nBoneIndex){

			//g_BsKernel.GetEngineObjectPtr(m_nLinkObjectHandle[i])->SetParentIndex( -1 );

			memcpy(m_nLinkObjectHandle+i, m_nLinkObjectHandle+i+1, sizeof(int)*(m_nLinkObjectCount-i-1));
			memcpy(m_nLinkBoneIndex+i, m_nLinkBoneIndex+i+1, sizeof(int)*(m_nLinkObjectCount-i-1));
			m_nLinkObjectCount--;
			i--;
		}
	}
}


void CBsAniObject::ProcessLinkParticle()
{
	int i;
	BSMATRIX matLink;

	RemoveInvalidLinkParticle();

	for( i = 0; i < m_nLinkParticleCount; ++i )
	{
		if( m_nLinkParticleHandle[ i ] >= 0 ) 
		{
			BSMATRIX matRotY = BSMATRIX( 
										0, 0, -1, 0, 
										0, 1,  0, 0, 
										1, 0,  0, 0, 
										0, 0,  0, 1 );	// BsMatrixRotationY(&matRotY, D3DX_PI*0.5f);
			BsMatrixMultiply( &matLink, &matRotY, m_pAni->FindBone( m_nLinkParticleBoneIndex[ i ] )->GetTransMatrix() );
			BsMatrixMultiply( &matLink, &matLink, ( BSMATRIX * )GetObjectMatrix() );
		   	BsMatrixMultiply( &matLink, m_matLinkParticle + i, &matLink );
			g_BsKernel.GetEngineParticlePtr( m_nLinkParticleHandle[ i ] )->SetParticleMatrix( ( D3DXMATRIX * )&matLink );
			g_BsKernel.GetEngineParticlePtr( m_nLinkParticleHandle[ i ] )->ShowParticle( true );
		}
	}
}

// Link Particle
void CBsAniObject::LinkParticleByName(const char *pBoneName, int nParticleHandle, D3DXMATRIX* pMatrix/*=NULL*/)
{
	int nBoneIndex;

	nBoneIndex=m_pAni->FindBoneIndex(pBoneName);
	LinkParticle(nBoneIndex, nParticleHandle, pMatrix);
}

void CBsAniObject::LinkParticle(int nBoneIndex, int nParticleHandle, D3DXMATRIX* pMatrix/*=NULL*/)
{
	RemoveInvalidLinkParticle();	
	// 삭제 예정!! by jeremy
	if(m_nLinkParticleCount>=MAX_LINK_PARTICLE_COUNT) {
		DebugString("%s : Link Particle Exceed = %d", m_pMesh->GetMeshFileName(), m_nLinkParticleCount );
		return;
	}

	// Check 요망!! by jeremy
	for (int i=0; i<m_nLinkParticleCount; i++) {
		if ( m_nLinkParticleHandle[i] == nParticleHandle && m_nLinkParticleBoneIndex[i] == nBoneIndex ) {
			return;
		}
	}
	m_nLinkParticleBoneIndex[m_nLinkParticleCount]=nBoneIndex;
	m_nLinkParticleHandle[m_nLinkParticleCount]=nParticleHandle;
	if(pMatrix) {
		m_matLinkParticle[m_nLinkParticleCount] = *pMatrix;
		D3DXVECTOR3 vecNormal(0.0f, 1.0f, 0.0f);
		D3DXVec3TransformNormal(&vecNormal, &vecNormal, pMatrix);
		g_BsKernel.GetEngineParticlePtr(nParticleHandle)->SetParticleScale(D3DXVec3Length(&vecNormal));
	}
	else {
		D3DXMatrixIdentity(m_matLinkParticle+m_nLinkParticleCount);
	}
	g_BsKernel.GetEngineParticlePtr( nParticleHandle )->SetLinked( true );
	m_nLinkParticleCount++;
}

const D3DXMATRIX* CBsAniObject::UnlinkParticle(int nParticleHandle)
{
	int i;

	for( i = 0; i < m_nLinkParticleCount; i++ )
	{
		if( m_nLinkParticleHandle[ i ] == nParticleHandle )
		{
			g_BsKernel.StopParticlePlay( m_nLinkParticleHandle[ i ] );
			memcpy( m_nLinkParticleHandle + i, m_nLinkParticleHandle + i + 1, 
				sizeof( int ) * ( m_nLinkParticleCount - i - 1 ) );
			memcpy( m_nLinkParticleBoneIndex + i, m_nLinkParticleBoneIndex + i + 1, 
				sizeof( int ) * ( m_nLinkParticleCount - i - 1 ) );
			memcpy( m_matLinkParticle + i, m_matLinkParticle + i + 1, 
				sizeof( D3DXMATRIX ) * ( m_nLinkParticleCount - i - 1 ) );
			m_nLinkParticleCount--;
			return g_BsKernel.GetEngineParticlePtr( nParticleHandle )->GetParticleMatrix();
		}
	}
	return NULL;
}

void CBsAniObject::UnlinkParticleByBone(int nBoneIndex)
{
	int i;

	for( i = 0; i < m_nLinkParticleCount; i++ )
	{
		if( m_nLinkParticleBoneIndex[ i ] == nBoneIndex )
		{
			g_BsKernel.StopParticlePlay( m_nLinkParticleHandle[ i ] );
			memcpy( m_nLinkParticleHandle + i, m_nLinkParticleHandle + i + 1, 
				sizeof( int ) * ( m_nLinkParticleCount - i - 1 ) );
			memcpy( m_nLinkParticleBoneIndex + i, m_nLinkParticleBoneIndex + i + 1, 
				sizeof( int ) * ( m_nLinkParticleCount - i - 1 ) );
			memcpy( m_matLinkParticle + i, m_matLinkParticle + i + 1, 
				sizeof( D3DXMATRIX ) * ( m_nLinkParticleCount - i - 1 ) );
			m_nLinkParticleCount--;
			i--;
		}
	}
}

void CBsAniObject::RemoveInvalidLinkParticle()
{
	for(int i=0;i<m_nLinkParticleCount;++i) {
		if(!g_BsKernel.IsPlayParticleObject(m_nLinkParticleHandle[i])) {
			memcpy(m_nLinkParticleHandle+i, m_nLinkParticleHandle+i+1, sizeof(int)*(m_nLinkParticleCount-i-1));
			memcpy(m_nLinkParticleBoneIndex+i, m_nLinkParticleBoneIndex+i+1, sizeof(int)*(m_nLinkParticleCount-i-1));
			memcpy(m_matLinkParticle+i, m_matLinkParticle+i+1, sizeof(D3DXMATRIX)*(m_nLinkParticleCount-i-1));
			m_nLinkParticleCount--;
			i--;
		}
	}
}

void CBsAniObject::ProcessLinkFX()
{
	// 끝난 FX들 지워준다.
	while( 1 )
	{
		bool bFinish = true;
		for( int i=0; i<m_nLinkObjectCount; i++ )
		{
			CBsObject* pObject = g_BsKernel.GetEngineObjectPtr(m_nLinkObjectHandle[i]);
			if( pObject->GetObjectType() != BS_FX_OBJECT )
				continue;

			CBsFXObject* pFX = (CBsFXObject *)pObject;
			if( pFX->GetCurState() == CBsFXObject::STOP )
			{
//				UnlinkObject( m_nLinkObjectHandle[i] );
				KERNEL_COMMAND cmd;
				cmd.nCommand = OBJECT_COMMAND_UNLINKOBJECT;
				cmd.nBuffer[0] = GetKernelPoolIndex();
				cmd.nBuffer[1] = m_nLinkObjectHandle[i];
				ReqUnlinkObject(&cmd);
				bFinish = false;
				break;
			}
		}
		if( bFinish == true )
		{
			break;
		}
	}
}

void CBsAniObject::GetRagdollPosAndVelocity( D3DXVECTOR3 *pPos, D3DXVECTOR3 *pVel )
{
	BsAssert( m_bRagdollEnable == true );
	BsAssert( m_bSimulationMode == true );
	BsAssert( !m_BoneActors.empty() );	

	if( pPos ) {
		D3DXMATRIX MatRootBone;
		m_BoneActors[0]->GetTransform( MatRootBone );
		*pPos = *(D3DXVECTOR3*)&MatRootBone._41;
	}

	if( pVel ) {
        *pVel = m_BoneActors[0]->GetVelocity();
	}
}

void CBsAniObject::ObjectAlphaBlend(bool bIsEnable)
{
	CBsObject::ObjectAlphaBlend(bIsEnable);
	for(int i=0;i<m_nLinkObjectCount;++i) {
		if(!(m_nLinkObjectHandle[i]&0x80000000)){
			g_BsKernel.GetEngineObjectPtr(m_nLinkObjectHandle[i])->ObjectAlphaBlend(bIsEnable);
		}
	}
}

/*
void CBsAniObject::ExcuteEnableObjectAlphaBlend(bool bIsEnable)
{
	// in Render Thread
	CBsObject::ExcuteEnableObjectAlphaBlend(bIsEnable);
	for(int i=0;i<m_nLinkObjectCount;++i) {
		if(!(m_nLinkObjectHandle[i]&0x80000000)){
			g_BsKernel.GetEngineObjectPtr(m_nLinkObjectHandle[i])->ExcuteEnableObjectAlphaBlend(bIsEnable);
		}
	}
}
*/

void CBsAniObject::SetObjectAlphaWeight(float fAlpha)
{
	CBsObject::SetObjectAlphaWeight(fAlpha);
	for(int i=0;i<m_nLinkObjectCount;++i) {
		if(!(m_nLinkObjectHandle[i]&0x80000000)){
			g_BsKernel.GetEngineObjectPtr(m_nLinkObjectHandle[i])->SetObjectAlphaWeight(fAlpha);
		}
	}
}

void CBsAniObject::RestoreAlphaBlend()
{
	CBsObject::RestoreAlphaBlend();
	for(int i=0;i<m_nLinkObjectCount;++i) {
		if(!(m_nLinkObjectHandle[i]&0x80000000)){
			g_BsKernel.GetEngineObjectPtr(m_nLinkObjectHandle[i])->RestoreAlphaBlend();
		}
	}
}

void CBsAniObject::Process()
{
	CBsObject::Process();
	for(int i=0;i<m_nLinkObjectCount;++i) {
		if(!(m_nLinkObjectHandle[i]&0x80000000)){
			g_BsKernel.GetEngineObjectPtr(m_nLinkObjectHandle[i])->Process();
		}
	}
}

void CBsAniObject::PostCalculateAnimation()
{
	m_BoneRotation.clear();
	/*
	for(int i=0;i<m_nLinkObjectCount;++i) {
		if(!(m_nLinkObjectHandle[i]&0x80000000)){
			CBsObject* pObject = g_BsKernel.GetEngineObjectPtr(m_nLinkObjectHandle[i]);			
			pObject->PostCalculateAnimation();
		}
	}*/	
}

void CBsAniObject::SetAnimation()
{
	int i, nCount;
	for( i = 0; i < m_nCurrentAniSize; i++ )
	{
		if( m_nCurrentAniBone[ i ] == 0 )	// m_nCurrentAniBone[ i ] 이 0 이면 Root bone 이다..
		{
			m_pAni->SetAnimationFrame( m_nCurrentAni[ i ], m_fCurrentFrame[ i ] );
		}
		else
		{
			m_pAni->SetAnimationFrameBone( m_nCurrentAni[ i ], m_fCurrentFrame[ i ], m_nCurrentAniBone[ i ] );
		}
	}
	for( i = 0; i < m_nBlendAniSize; i++ )
	{
		m_pAni->BlendAnimationFrame( m_nBlendAni[ i ], m_fBlendFrame[ i ], m_fBlendWeight[ i ], m_nBlendAniBone[ i ] );
	}
	nCount = ( int )m_BoneRotation.size();
	for( i = 0; i < nCount; i++ )
	{
		m_pAni->SetBoneRotation( m_BoneRotation[ i ].szBoneName, &m_BoneRotation[ i ].Rotation );
	}
	m_pAni->SetCalcAniPosition( m_nCalcAniPosition );
}

bool CBsAniObject::InitRender(float fDistanceFromCam)
{		
	SetDistanceFromCam( fDistanceFromCam );

#ifndef _USAGE_TOOL_
	if(UsingBillBoard() && m_nBillBoardIndex>=0) {
		float fLimitDistance;
		if(m_bIsBillBoardRenderBefore) {
			fLimitDistance = g_BsKernel.GetBillboardMgr()->GetLimitMinimumDistance();
		}
		else {
			fLimitDistance = g_BsKernel.GetBillboardMgr()->GetLimitMaximumDistance();
		}
		if (fDistanceFromCam > fLimitDistance) {
			D3DXVECTOR3 vecWorldPos;
			const float fValidRange = SPRITE_RANGE-(1.f/63.f);
			memcpy( &vecWorldPos, &( GetObjectMatrixByProcess()->_41 ), sizeof( D3DXVECTOR3 ) );
			float fAniFrameRatio = (m_fCurrentFrame[0] / m_pAni->GetAniLength(m_nCurrentAni[0])) * fValidRange;
			g_BsKernel.GetBillboardMgr()->AddObjectBillboard(m_nBillBoardIndex, vecWorldPos, this->GetSkinIndex(), m_nCurrentAni[0], 
				GetAlphaWeight(), fAniFrameRatio);
			m_bIsBillBoardRender[m_sCurrentProcessBuffer] = true;
			m_bIsBillBoardRenderBefore = true;
			return false;
		}
		else {
			m_bIsBillBoardRender[m_sCurrentProcessBuffer] = false;
			m_bIsBillBoardRenderBefore = false;
		}
	}
#endif
	CBsKernel::s_nDrawTotalAniCount++;

	if( !InitRenderRagdoll() ) {
		D3DXMATRIX *pAniCache;
		int nCacheCount;

		if( m_nAniCacheType )
		{
			if( fDistanceFromCam > 1000.0f )	// 10미터 밖에 있는 궁병은 본로테이션 안한다..
			{
				m_BoneRotation.clear();
			}
/*			if( fDistanceFromCam > 2500.0f )	// 15미터 밖에 있는 애들은 블렌드 애니 안한다.
			{
				m_nBlendAniSize = 0;
			}*/
		}
		pAniCache = NULL;
		if( ( m_nAniCacheType ) && ( m_nBlendAniSize <= 0 ) && ( m_BoneRotation.size() <= 0 ) )
		{
			if( m_nAniCacheType == 1 )
			{
				pAniCache = g_BsAniCache.FindCacheAni( m_nAniFileIndex, m_nCurrentAni[ 0 ], m_fCurrentFrame[ 0 ], nCacheCount );
			}
			else
			{
				pAniCache = g_LargeBsAniCache.FindCacheAni( m_nAniFileIndex, m_nCurrentAni[ 0 ], m_fCurrentFrame[ 0 ], nCacheCount );
			}
		}
		if( pAniCache )
		{
			m_pSaveMatrix[ m_sCurrentProcessBuffer ] = pAniCache;
		}
		else
		{
			SetAnimation();
			AllocProcessSaveMatrixBuffer();
			m_pAni->CalculateAnimationMatrix( this );

			if( ( m_nAniCacheType ) && ( m_nBlendAniSize <= 0 ) && ( m_BoneRotation.size() <= 0 ) )
			{
				if( m_nAniCacheType == 1 )
				{
					g_BsAniCache.CacheAni( m_nAniFileIndex, m_nCurrentAni[ 0 ], m_fCurrentFrame[ 0 ], m_pSaveMatrix[ m_sCurrentProcessBuffer ], m_pAni->GetBoneCount() );
				}
				else
				{
					g_LargeBsAniCache.CacheAni( m_nAniFileIndex, m_nCurrentAni[ 0 ], m_fCurrentFrame[ 0 ], m_pSaveMatrix[ m_sCurrentProcessBuffer ], m_pAni->GetBoneCount() );
				}
			}
		}
	}

static CBsAniObject* pDebugAniObj = NULL;
static CBsObject* pDebugObject = NULL;
static int nDebugHandle = -1;

	BSMATRIX matLink;
	for(int i=0;i<m_nLinkObjectCount;i++){
		// Linked Object도 UpdateObject() 영향받도록 바꿉니다.!!
		CBsObject* pObject = g_BsKernel.GetEngineObjectPtr(m_nLinkObjectHandle[i]);

pDebugObject = pObject;
nDebugHandle = m_nLinkObjectHandle[i];
pDebugAniObj = this;

		if( pObject->IsShow()) {
			if( pObject->GetObjectType() != BS_SIMULATE_OBJECT ) {
				BsMatrixMultiply( &matLink, (BSMATRIX*)( m_pAni->FindBone( m_nLinkBoneIndex[ i ] )->GetWorldMatrix() ), 
					(BSMATRIX*)( GetProcessMatrix( m_nLinkBoneIndex[ i ] ) ) );
				BsMatrixMultiply( &matLink, &matLink, (BSMATRIX*)GetObjectMatrixByProcess() );
				pObject->SetObjectMatrix((D3DXMATRIX*)&matLink);
			}
			pObject->InitRender(fDistanceFromCam);
		}
	}
	return true;
}

void CBsAniObject::RefreshAni()
{

	SetAnimation();
	m_pAni->CalculateAnimationMatrix( NULL );

	//****************************************************************************************************	
	BSMATRIX matLink;
	for(int i=0;i<m_nLinkObjectCount;i++){		
		CBsObject* pObject = g_BsKernel.GetEngineObjectPtr(m_nLinkObjectHandle[i]);
		BsMatrixMultiply( &matLink, (BSMATRIX*)m_pAni->FindBone(m_nLinkBoneIndex[i])->GetTransMatrix(), (BSMATRIX*)GetObjectMatrix() );
		pObject->SetObjectMatrix((D3DXMATRIX*)&matLink);
		pObject->RefreshAni();		
	}
	//****************************************************************************************************	
}

D3DXMATRIX *CBsAniObject::GetBoneMatrix(const char *pBoneName)
{
	CBsBone *pBone;

	pBone=m_pAni->FindBone(pBoneName);
	BsAssert(pBone && "Invalid Bone Name");
	
	return (D3DXMATRIX*)(pBone->GetTransMatrix());
}

D3DXMATRIX *CBsAniObject::GetBoneWorldMatrix(const char *pBoneName)
{
	CBsBone *pBone;

	pBone=m_pAni->FindBone(pBoneName);
	BsAssert(pBone && "Invalid Bone Name");

	return (D3DXMATRIX*)pBone->GetWorldMatrix();
}


void CBsAniObject::SetAniPtr( CBsAni *pAni, int nAniFileIndex )
{
	m_pAni = pAni;
	m_nAniFileIndex = nAniFileIndex;
}

void CBsAniObject::Render(C3DDevice *pDevice)
{
	//************************************************************************************
	//	Child 오브텍트의 경우만 check하도록 바꾸어야 합니다. by jeremy
	float fDistanceFromCam = GetDistanceFromCam();
#ifndef _USAGE_TOOL_
	if(UsingBillBoard() && m_nBillBoardIndex>=0) {
		if(m_bIsBillBoardRender[m_sCurrentRenderBuffer]) {
			return;
		}
	}
#endif
	//************************************************************************************

	CBsObject::Render(pDevice);

	CBsObject* pObject;
	for(int i=0;i<m_nLinkObjectCount;++i) {
		if(!(m_nLinkObjectHandle[i]&0x80000000)){
			pObject=g_BsKernel.GetEngineObjectPtr(m_nLinkObjectHandle[i]);
			// Linked Object도 UpdateObject()호출의 영향을 받도록 바꿉니다.!!
			if(pObject->IsShow()) {
				pObject->PreRender(pDevice);				
				pObject->SetCenter( m_vecCenter );
				pObject->Render(pDevice);
				pObject->PostRender(pDevice);
			}
		}
	}
}

void CBsAniObject::RenderAlpha(C3DDevice *pDevice)
{
	//************************************************************************************
	//	Child 오브텍트의 경우만 check하도록 바꾸어야 합니다. by jeremy
	float fDistanceFromCam = GetDistanceFromCam();
#ifndef _USAGE_TOOL_
	if(UsingBillBoard() && m_nBillBoardIndex>=0) {
		if(m_bIsBillBoardRender[m_sCurrentRenderBuffer]) {
			return;
		}
	}
#endif
	//************************************************************************************
	CBsObject::RenderAlpha(pDevice);
}

void CBsAniObject::RenderShadow(C3DDevice* pDevice) //$$ LOD 관련 확인 필요 합니다
{
	//************************************************************************************
	//	Child 오브텍트의 경우만 check하도록 바꾸어야 합니다. by jeremy
	float fDistanceFromCam = GetDistanceFromCam();
#ifndef _USAGE_TOOL_
	if(UsingBillBoard() && m_nBillBoardIndex>=0) {
		if(m_bIsBillBoardRender[m_sCurrentRenderBuffer]) {
			return;
		}
	}
#endif
	//************************************************************************************
	CBsObject::RenderShadow(pDevice);
	
	CBsObject* pObject;
	for(int i=0;i<m_nLinkObjectCount;++i) {
		if(!(m_nLinkObjectHandle[i]&0x80000000)) {
			pObject=g_BsKernel.GetEngineObjectPtr(m_nLinkObjectHandle[i]);
			if(pObject->IsShow() && pObject->GetMeshPt()) {
				pObject->PreRender(pDevice);
				pObject->RenderShadow(pDevice);
				pObject->PostRender(pDevice);
			}
		}
	}
}

void CBsAniObject::SetCurrentAni(SET_ANI_INFO *pInfo)
{
	SetCurrentAni(pInfo->nAniIndex, pInfo->fFrame, pInfo->nBoneIndex);
}

void CBsAniObject::SetCurrentAni(int nAni, float fFrame, int nBoneIndex)
{
	if( nBoneIndex == 0 )	// root bone 에 ani 셋팅하면 현재 ani clear 해준다..
	{
		ResetAniIndex();
	}

	BsAssert( m_nCurrentAniSize < MAX_CURRENT_ANI_SIZE );

	m_nCurrentAni[ m_nCurrentAniSize ] = nAni;
	m_fCurrentFrame[ m_nCurrentAniSize ] = fFrame;
	m_nCurrentAniBone[ m_nCurrentAniSize ] = nBoneIndex;
	m_nCurrentAniSize++;
}

void CBsAniObject::BlendAni(BLEND_ANI_INFO *pInfo)
{
	BlendAni(pInfo->nAniIndex, pInfo->fFrame, pInfo->fWeight, pInfo->nBoneIndex);
}

void CBsAniObject::BlendAni(int nBlendAni, float fBlendFrame, float fBlendWeight, int nBoneIndex)
{
	BsAssert( m_nBlendAniSize < MAX_BLEND_ANI_SIZE );

	m_nBlendAni[ m_nBlendAniSize ] = nBlendAni;
	m_fBlendFrame[ m_nBlendAniSize ] = fBlendFrame;
	m_fBlendWeight[ m_nBlendAniSize ] = fBlendWeight;
	m_nBlendAniBone[ m_nBlendAniSize ] = nBoneIndex;
	m_nBlendAniSize++;
}

void CBsAniObject::FrontInsertBlendAni( int nBlendAni, float fBlendFrame, float fBlendWeight, int nBoneIndex )
{
	// mruete: prefix bug 293: added m_nBlendAniSize >= 0
	BsAssert( m_nBlendAniSize >= 0 && m_nBlendAniSize < MAX_BLEND_ANI_SIZE );

	memmove( m_nBlendAni + 1, m_nBlendAni, sizeof( int ) * m_nBlendAniSize );
	memmove( m_fBlendFrame + 1, m_fBlendFrame, sizeof( float ) * m_nBlendAniSize );
	memmove( m_fBlendWeight + 1, m_fBlendWeight, sizeof( float ) * m_nBlendAniSize );
	memmove( m_nBlendAniBone + 1, m_nBlendAniBone, sizeof( int ) * m_nBlendAniSize );
	m_nBlendAni[ 0 ] = nBlendAni;
	m_fBlendFrame[ 0 ] = fBlendFrame;
	m_fBlendWeight[ 0 ] = fBlendWeight;
	m_nBlendAniBone[ 0 ] = nBoneIndex;
	m_nBlendAniSize++;
}

void CBsAniObject::SetBoneRotation( const char *pBoneName, D3DXVECTOR3 *pRotation )
{
	BONE_ROTATION_INFO Info;

	strcpy( Info.szBoneName, pBoneName );
	memcpy( &Info.Rotation, pRotation, sizeof( D3DXVECTOR3 ) );
	m_BoneRotation.push_back( Info );
}

// TODO : 이부분을 고치실 거면, 밑에 CBsRMAniObject::GetBox3()도 고쳐주세요.
bool CBsAniObject::GetBox3( Box3 & B )
{
	if( m_pMesh )
	{
		if( m_bSimulationMode ) {	// 시뮬레이션의 경우 BoundingBox 를 다시 계산해준다.

			if( m_BoneActors.empty() ) {
				return false;
			}

			AABB ragdollAABB;
			ragdollAABB.Reset();

			int i, nCount;

			float fMaxExtent = 0.f;

			nCount = m_BoneActors.size();
			for(i = 0; i < nCount; i++) {

				IBsPhysicsActor *pActor = m_BoneActors[i];

				BSVECTOR Pos;
				pActor->GetGlobalPosition(*(D3DXVECTOR3*)&Pos);

				fMaxExtent = __max(fMaxExtent, pActor->GetBoxExtend().x);
				fMaxExtent = __max(fMaxExtent, pActor->GetBoxExtend().y);
				fMaxExtent = __max(fMaxExtent, pActor->GetBoxExtend().z);

				ragdollAABB.Merge( Pos );
			}

			ragdollAABB.Vmin -= BSVECTOR(fMaxExtent, fMaxExtent, fMaxExtent);
			ragdollAABB.Vmax += BSVECTOR(fMaxExtent, fMaxExtent, fMaxExtent);

			BSVECTOR vExtent = ragdollAABB.GetExtent();

#if _OPT_SHINJICH_BSANIOBJECT_CPP
			*((BSVECTOR*) B.E) = vExtent;
#else
			B.E[0] = vExtent.x;
			B.E[1] = vExtent.y;
			B.E[2] = vExtent.z;
#endif
			B.C = ragdollAABB.GetCenter();
			m_vecCenter = *(D3DXVECTOR3*)&B.C;

			B.A[ 0 ] = BSVECTOR(1, 0, 0);
			B.A[ 1 ] = BSVECTOR(0, 1, 0);
			B.A[ 2 ] = BSVECTOR(0, 0, 1);

			return true;
		}
		else {
			D3DXMATRIX *pMatrix;
			D3DXMATRIX matRootBone;
			AABB* pBB = m_pMesh->GetBoundingBox();

#if _OPT_SHINJICH_BSANIOBJECT_CPP
			*((BSVECTOR*) B.E) = pBB->GetExtent();
#else
			D3DXVECTOR3	tmpVec = *(D3DXVECTOR3*)&pBB->GetExtent();
			B.E[0] = tmpVec.x;
			B.E[1] = tmpVec.y;
			B.E[2] = tmpVec.z; // 익스텐드 벡터
#endif
			pMatrix = GetObjectMatrixByProcess();
			BSVECTOR vecRootPos = pBB->GetCenter();//m_pAni->GetRootBoneLocalPosition();
			BsMatrixTranslation((BSMATRIX*)&matRootBone, vecRootPos.x, vecRootPos.y, vecRootPos.z);
			D3DXMatrixMultiply( &matRootBone, &matRootBone, pMatrix);

			memcpy(&B.C, &(matRootBone._41), sizeof(BSVECTOR));
			m_vecCenter = *(D3DXVECTOR3*)&B.C;

#if _OPT_SHINJICH_BSANIOBJECT_CPP
			// ...should remove lhs
			B.A[0] = __lvx( &(matRootBone._11), 0 );
			B.A[1] = __lvx( &(matRootBone._21), 0 );
			B.A[2] = __lvx( &(matRootBone._31), 0 );
#else
			memcpy(&B.A[0], &(matRootBone._11), sizeof(BSVECTOR));
			memcpy(&B.A[1], &(matRootBone._21), sizeof(BSVECTOR));
			memcpy(&B.A[2], &(matRootBone._31), sizeof(BSVECTOR));
#endif
			return true;
		}
	}
	return false;
}

int CBsAniObject::ProcessMessage(DWORD dwCode, DWORD dwParam1/*=0*/, DWORD dwParam2/*=0*/, DWORD dwParam3/*=0*/)
{
	switch(dwCode){
		case BS_USE_ANI_CACHE:
			m_nAniCacheType = dwParam1 > 0;
			if( m_nAniCacheType )
			{
				if( m_pAni->GetBoneCount() > g_LargeBsAniCache.CacheBufferSize() )
				{
					m_nAniCacheType = 0;
				}
				else if( m_pAni->GetBoneCount() > g_BsAniCache.CacheBufferSize() )
				{
					m_nAniCacheType = 2;
				}
				else
				{
					m_nAniCacheType = 1;
				}
			}
			return 1;
		case BS_ANI_FILE_INDEX:
			return m_nAniFileIndex;
		case BS_GETANILENGTH:
			return GetAniLength( dwParam1 );
		case BS_GETANIDISTANCE:
			GetAniDistance((GET_ANIDISTANCE_INFO *)dwParam1);
			return 1;
		case BS_GETANICOUNT:
			return m_pAni->GetAniCount();
		case BS_SETCURRENTANI:
			SetCurrentAni(dwParam1, *((float *)dwParam2), 0);
			return 1;
		case BS_BLENDANI:
			BlendAni(dwParam1, *((float *)dwParam2), *((float *)dwParam3), 0);
			return 1;
		case BS_SETCURRENTANI_BONE:
			SetCurrentAni((SET_ANI_INFO *)dwParam1);
			return 1;
		case BS_BLENDANI_BONE:
			BlendAni((BLEND_ANI_INFO *)dwParam1);
			return 1;
		case BS_GET_BONECOUNT:
			return m_pAni->GetBoneCount();
		case BS_GET_BONENAME:
			return (int)m_pAni->GetBoneName(dwParam1);
		case BS_GET_BONE_NAME_LIST:
			return m_pAni->GetBoneNameList((char *)dwParam1);
		case BS_LINKOBJECT_NAME:
			LinkObjectByName((const char *)dwParam1, dwParam2);
			return 1;
		case BS_LINKOBJECT:
			LinkObject(dwParam1, dwParam2);
			return 1;	
		case BS_UNLINKOBJECT:	
			return (int)UnlinkObject(dwParam1, (D3DXVECTOR3*)dwParam2);
		case BS_UNLINKOBJECT_BONE:
			BsAssert(dwParam2 == 0);
			UnlinkBone(dwParam1);
			return 1;
		case BS_LINKPARTICLE_NAME:
			LinkParticleByName((const char *)dwParam1, dwParam2, (D3DXMATRIX*)dwParam3);
			return 1;
		case BS_LINKPARTICLE:
			LinkParticle(dwParam1, dwParam2, (D3DXMATRIX*)dwParam3);
			return 1;
		case BS_UNLINKPARTICLE:
			BsAssert(dwParam2 == 0);
			return (int)UnlinkParticle(dwParam1);
		case BS_UNLINKPARTICLE_BONE:
			BsAssert(dwParam2 == 0);
			UnlinkParticleByBone(dwParam1);
			return 1;
		case BS_REFRESH_ANI:
			RefreshAni();
			return 1;
		case BS_GET_BONE_MATRIX:
			return (int)GetBoneMatrix((char *)dwParam1);
		case BS_GET_BONE_WORLD_MATRIX:
			return (int)GetBoneWorldMatrix( (char*)dwParam1 );
		case BS_GET_BONE_INDEX:
			return m_pAni->FindBoneIndex((char *)dwParam1);
		case BS_SET_CALC_ANI_POSITION:
			m_nCalcAniPosition = dwParam1;
			return 1;
		case BS_SET_BONE_ROTATION:
			SetBoneRotation( ( char * )dwParam1, ( D3DXVECTOR3 * )dwParam2 );
			return 1;	
		case BS_SET_CLEAR_BONE_ROTATION:
			m_BoneRotation.clear();
			return 1;
		case BS_GET_ROOT_BONE_ROTATION:
			m_pAni->GetRootBoneRotation( dwParam1, *( float * )dwParam2, ( D3DXMATRIX * )dwParam3 );
			return 1;
		case BS_GET_ROOT_BONE_MATRIX:
			{
				m_pAni->GetRootBoneMatrix( dwParam1, *( float * )dwParam2, ( D3DXMATRIX * )dwParam3, m_nCalcAniPosition );
				m_matProcessRootBone = *(D3DXMATRIX*)dwParam3;
			}
			return 1;
		case BS_PHYSICS_COLLISIONMESH:
			MakeCollisionBody( (PHYSICS_COLLISION_CONTAINER*)dwParam1 );
			return 1;
		case BS_PHYSICS_RAGDOLL:
			MakeRagdoll( (PHYSICS_RAGDOLL_CONTAINER*)dwParam1, dwParam2);
			return 1;
		case BS_PHYSICS_SIMULATION:
			SetSimulationMode( dwParam1 == 1 );
			return 1;		
		case BS_SET_RAGDOLL_CATCH_BONE:
			m_nCatchBoneIndex = dwParam1;
			return 1;
		case BS_SET_BILLBOARD_INDEX:
			m_nBillBoardIndex = dwParam1;
			return 1;
		case BS_GET_RAGDOLL_POS_VELOCITY:
			GetRagdollPosAndVelocity( (D3DXVECTOR3*)dwParam1, (D3DXVECTOR3*)dwParam2);
			return 1;		
		default:
			break;
	}

	return CBsObject::ProcessMessage(dwCode, dwParam1, dwParam2, dwParam3);
}

void CBsAniObject::MakeRagdoll( PHYSICS_RAGDOLL_CONTAINER *pSetting, int nSetting )
{

	int i, j, nBoneCount, nSettingCount;

	if( m_bRagdollEnable ) {
		return ;
	}

	nBoneCount = m_pAni->GetBoneCount();

	nSettingCount = pSetting ? pSetting->RagdollList.size() : 0;

	D3DXMATRIX matObject;
	D3DXVECTOR3 Scale;
	D3DXMATRIX *pMatrix;

	pMatrix = GetObjectMatrixByProcess();
	Scale.x = D3DXVec3Length((D3DXVECTOR3*)&pMatrix->_11);
	Scale.y = D3DXVec3Length((D3DXVECTOR3*)&pMatrix->_21);
	Scale.z = D3DXVec3Length((D3DXVECTOR3*)&pMatrix->_31);

	D3DXMATRIX matInvScale;
	D3DXMatrixScaling(&matInvScale, 1 / Scale.x, 1 / Scale.y, 1 / Scale.z);
	D3DXMatrixMultiply(&matObject, &matInvScale, pMatrix);

	for( i = 0; i < nBoneCount; i++) {
		
		CBsBone *pBone = m_pAni->FindBone(i);
		const char *pBoneName = pBone->GetBoneName();

		D3DXMATRIX matBone;
		D3DXMATRIX matWorld = *(D3DXMATRIX*)(pBone->GetWorldMatrix());

		matWorld._41 *= Scale.x;
		matWorld._42 *= Scale.y;
		matWorld._43 *= Scale.z;

		D3DXMatrixMultiply(&matBone, &matWorld, &matObject);

		float l1 = fabs(1.f-D3DXVec3Length( (D3DXVECTOR3*)&matWorld._11 ));
		float l2 = fabs(1.f-D3DXVec3Length( (D3DXVECTOR3*)&matWorld._21 ));
		float l3 = fabs(1.f-D3DXVec3Length( (D3DXVECTOR3*)&matWorld._31 ));


		D3DXVECTOR3 Scale2;
		Scale2.x = D3DXVec3Length((D3DXVECTOR3*)&matWorld._11);
		Scale2.y = D3DXVec3Length((D3DXVECTOR3*)&matWorld._21);
		Scale2.z = D3DXVec3Length((D3DXVECTOR3*)&matWorld._31);

		D3DXMATRIX matInvScale2;
		D3DXMatrixScaling(&matInvScale2, 1 / Scale2.x, 1 / Scale2.y, 1 / Scale2.z);
		D3DXMatrixMultiply(&matWorld, &matInvScale2, &matWorld);

		float fRadius = 20.f, fHeight = 10.f;
		D3DXMATRIX matLocal;
		D3DXMatrixIdentity(&matLocal);

		for (j = 0; j < nSettingCount; j++) {
			if(strcmp(pSetting->RagdollList[j].szName.c_str(), pBoneName) == 0 ) {
				fRadius = pSetting->RagdollList[j].fRadius;
				fHeight = pSetting->RagdollList[j].fHeight;
				matLocal = pSetting->RagdollList[j].matLocal;
			}
		}

		fRadius *= Scale.x;
		fHeight *= Scale.y;

		IBsPhysicsActor *pBoneActor = CBsPhysicsBone::Create(fRadius, fHeight, matLocal, matBone, (char *)pBoneName);

		pBoneActor->SetInvWorldMat( *(D3DXMATRIX*)pBone->GetInvWorldMatrix() );
		pBoneActor->SetWorldMat( *(D3DXMATRIX*)pBone->GetWorldMatrix() );
		pBoneActor->SetBoneName( (char *) pBoneName );
		pBoneActor->SetKinematic();
		pBoneActor->SetScale( Scale );
		pBoneActor->SetGroup("Ragdoll");
		//pBoneActor->SetSaveType( SAVE_TRANSFORM );

		m_BoneActors.push_back( pBoneActor );
	}

	// create physics joints
	MakeJointRecurse( m_pAni->GetRootBone(), nSetting );

	m_bRagdollEnable = true;
}

void CBsAniObject::MakeJointRecurse(CBsBone *pBone, int nSetting)
{
	int i, nCount;

	nCount = pBone->GetChildCount();

	for( i = 0; i < nCount; i++) {
		CBsBone *pChildBone = pBone->GetChildPtr(i);

		int nBoneIndex1 = pBone->GetBoneIndex();
		int nBoneIndex2 = pChildBone->GetBoneIndex();

		D3DXVECTOR3 Scale;
		D3DXMATRIX *pMatrix;

		pMatrix = GetObjectMatrixByProcess();
		Scale.x = D3DXVec3Length((D3DXVECTOR3*)&pMatrix->_11);
		Scale.y = D3DXVec3Length((D3DXVECTOR3*)&pMatrix->_21);
		Scale.z = D3DXVec3Length((D3DXVECTOR3*)&pMatrix->_31);

		D3DXMATRIX matObject;
		D3DXMATRIX matWorld;
		D3DXMATRIX matInvScale;
		D3DXMatrixScaling(&matInvScale, 1 / Scale.x, 1 / Scale.y, 1 / Scale.z);
		D3DXMatrixMultiply(&matObject, &matInvScale, pMatrix);
		matWorld = *(D3DXMATRIX*)(pChildBone->GetWorldMatrix());
		matWorld._41 *= Scale.x;
		matWorld._42 *= Scale.y;
		matWorld._43 *= Scale.z;
		D3DXMatrixMultiply(&matObject, &matWorld, &matObject);

		D3DXVECTOR3 AnchorPos = *(D3DXVECTOR3*)(&matObject._41);

		IBsPhysicsJoint *pJoint = CBsPhysicsD6Joint::Create(m_BoneActors[nBoneIndex1], m_BoneActors[nBoneIndex2], AnchorPos, nSetting);

		m_Joints.push_back( pJoint );

		MakeJointRecurse( pChildBone, nSetting );
	}
}

void CBsAniObject::SetSimulationMode( bool bSimulation )
{
	int i, nSize; 
	if( m_bRagdollEnable == false) return;
	if( bSimulation == m_bSimulationMode ) return;

	nSize = m_BoneActors.size();
	for( i = 0; i < nSize; i++) {
		m_BoneActors[i]->SetKinematic( !bSimulation );
	}
	m_bSimulationMode = bSimulation;
}

bool CBsAniObject::InitRenderRagdoll()
{
	int i, nCount;
	// 시뮬레이션용 
	if( !m_bRagdollEnable ) return false;

	// BsEngine -> Novodex (SetAnimation) Keyframe System
	nCount = m_BoneActors.size();
	
	// Novodex -> BsEngine (GetAnimation) Ragdoll System
	if( m_bSimulationMode == true ) {		
		AllocProcessSaveMatrixBuffer();
		D3DXMATRIX matInv;
		D3DXMatrixInverse( &matInv, 0, GetObjectMatrixByProcess() );
		for( i = 0; i < nCount; i++) {
			D3DXMATRIX VertexTransMat;
			NxActor *pNxActor = m_BoneActors[i]->GetActor();			
			m_BoneActors[i]->GetTransform( VertexTransMat );
			D3DXMatrixMultiply(&VertexTransMat, &VertexTransMat, &matInv);
			SaveMatrix( &VertexTransMat, i );
		}
		return true;
	}
	return false;
}

void CBsAniObject::SetRagdollMatrix( bool bLocalAxis, D3DXMATRIX *pMatrix)
{
	if( m_nCatchBoneIndex == -1) {
		return;
	}
	if( m_BoneActors.empty() ) {
		return;
	}
	IBsPhysicsActor *pActor = m_BoneActors[ m_nCatchBoneIndex ];

	pActor->SetKinematic( );

	if ( bLocalAxis ) {
		D3DXMATRIX matLink;
		D3DXMATRIX matInv = *(D3DXMATRIX*)(m_pAni->FindBone(m_nCatchBoneIndex)->GetInvWorldMatrix());
		D3DXMatrixMultiply(&matLink, &matInv, pMatrix);
		pActor->SetTransform( matLink );
	}
	else {
		D3DXMATRIX matLink;
		D3DXMATRIX matInv = *(D3DXMATRIX*)(m_pAni->FindBone(m_nCatchBoneIndex)->GetInvWorldMatrix());

		D3DXMATRIX matRot = matInv;
		matRot._41 = matRot._42 = matRot._43 = 0.f;
		D3DXMatrixTranspose(&matRot, &matRot);
		D3DXMatrixMultiply(&matLink, &matInv, &matRot);
		D3DXMatrixMultiply(&matLink, &matLink, pMatrix);
		pActor->SetTransform( matLink );
	}
}

CBsObject *g_pDebugObject = NULL;
void CBsAniObject::ProcessRagdollUnlink( D3DXVECTOR3 *pSpeed )
{
	int i, nSize;

g_pDebugObject = this;
DebugString( "Unlink Ragdoll %x\n", this );
	if( m_nCatchBoneIndex == -1 )
	{
		//BsAssert( 0 && "Duplicate Unlink Ragdoll" );
		DebugString( "Duplicate Unlink Ragdoll \n");
		return;
	}

	nSize = m_BoneActors.size();

	m_BoneActors[ m_nCatchBoneIndex ]->SetKinematic( false );
	D3DXVECTOR3 Vel = m_BoneActors[ m_nCatchBoneIndex ]->GetVelocity()*100.f;	

	bool bValidVelocity =  D3DXVec3Length( pSpeed ) > 0.00001f;
	float fVel = D3DXVec3Length(&Vel);
	float fNewVelocity = fVel;
	fNewVelocity = BsMin(BsMax(2600.f, fVel), 2800.f);		

	float fScale = fNewVelocity / fVel;
	nSize = m_BoneActors.size();

	NxVec3 vAngular;
	D3DXVec3Normalize((D3DXVECTOR3*)&vAngular, &Vel);

	float fPosUp = 0.f;

	for(i = 0; i < nSize; i++) {

		NxActor *pNxActor = m_BoneActors[i]->GetActor();
		if(pNxActor) {	
			if( i == m_nCatchBoneIndex ) {				
				pNxActor->setMass( pNxActor->getMass() * 2.f);
			}
			pNxActor->addTorque(vAngular * 3000.f, NX_ACCELERATION);
			pNxActor->setMaxAngularVelocity(FLT_MAX);
			NxVec3 linVel = pNxActor->getLinearVelocity() * fScale;				

			const float fClampSpeed = 30.f;
			if( bValidVelocity ) {
				NxVec3 speed = *(NxVec3*)pSpeed;
				speed.y = BsMax( -5.f, speed.y);
				pNxActor->setLinearVelocity( speed );

			}
			else {
				linVel.x = BsMax(-fClampSpeed, BsMin(linVel.x, fClampSpeed));
				linVel.y = BsMax(-fClampSpeed, BsMin(linVel.y, fClampSpeed));
				linVel.z = BsMax(-fClampSpeed, BsMin(linVel.z, fClampSpeed));
				pNxActor->setLinearVelocity(linVel);
			}				
			pNxActor->setSleepAngularVelocity( 1.5f );
			pNxActor->setSleepLinearVelocity( 2.7f );
			
			NxVec3 Pos;			
			Pos = pNxActor->getGlobalPosition();
			float fHeightY = g_BsKernel.GetLandHeight( Pos.x*100.f, Pos.z*100.f);
			if( Pos.y*100.f < fHeightY + 50.f) {
				Pos.y = (fHeightY+10.f) * 0.01f;
				pNxActor->setGlobalPosition( Pos );                
				pNxActor->setLinearVelocity( NxVec3(0,0,0));
				pNxActor->setAngularVelocity( NxVec3(0,0,0));
			}
		}
	}

	nSize = m_Joints.size();
	for( i = 0; i < nSize; i++) {
		IBsPhysicsJoint *pJoint = m_Joints[ i ];
		pJoint->SetLimited();
	}
	m_nCatchBoneIndex = -1;
}

void CBsAniObject::ProcessLinkedPhysicsObject( DWORD dwParam1, DWORD dwParam2 )
{
	int i, nSize;
	
	SetAnimation();

	m_pAni->CalculateAnimationMatrix( NULL );

	for(i = 0; i < m_nLinkObjectCount; i++) {
	
		BSMATRIX matLink;

		CBsObject* pObject = (CBsObject*)g_BsKernel.GetEngineObjectPtr(m_nLinkObjectHandle[i]);	

		CBsBone *pBone = m_pAni->FindBone(m_nLinkBoneIndex[i]);

		if(pObject->IsRagdoll()) {
			BsMatrixMultiply(&matLink, pBone->GetTransMatrix(), (BSMATRIX*)GetObjectMatrixByProcess());
			pObject->SetObjectMatrix(GetObjectMatrixByProcess());	
			pObject->SetRagdollMatrix( true, (D3DXMATRIX*)&matLink);
		}
		else {
			// BsClothObject
			if( pObject->GetObjectType() == BS_SIMULATE_OBJECT ) {
				pObject->SetObjectMatrix(GetObjectMatrixByProcess());	
				D3DXMATRIX matRootBone;
				D3DXMatrixMultiply(  &matRootBone, (D3DXMATRIX*)m_pAni->GetRootBone()->GetTransMatrix(), GetObjectMatrixByProcess());
				pObject->ProcessLinkedPhysicsObject( (DWORD)&matRootBone._41, (DWORD)&matRootBone._31);
			}
			// BsAniObject, BsObject	
			else {
				BsMatrixMultiply(&matLink, pBone->GetTransMatrix(), (BSMATRIX*)GetObjectMatrixByProcess());
				pObject->SetObjectMatrix((D3DXMATRIX*)&matLink);	
				pObject->ProcessLinkedPhysicsObject();
			}
		}
	}

	// Link Command 가 불려진직후에도 Ragdoll Matrix 처리를 해주어야 된다.
	std::vector<KERNEL_COMMAND>& kernelCommand = g_BsKernel.GetPreKernelCommand();

	nSize = kernelCommand.size();
	for( i = 0; i < nSize; i++) {

		KERNEL_COMMAND *pCmd = &kernelCommand[i];

		if( pCmd->nCommand == OBJECT_COMMAND_LINKOBJECT &&
			pCmd->nBuffer[0] == GetKernelPoolIndex() ) {		

			int nLinkObjectIndex = pCmd->nBuffer[2];
			int nLinkBoneIndex = pCmd->nBuffer[1];

			CBsObject* pObject = (CBsObject*)g_BsKernel.GetEngineObjectPtr( nLinkObjectIndex );	

			if( pObject->IsRagdoll( ) ) {				
				CBsBone *pBone = m_pAni->FindBone( nLinkBoneIndex );
				BSMATRIX matLink;
				BsMatrixMultiply(&matLink, pBone->GetTransMatrix(), (BSMATRIX*)GetObjectMatrixByProcess());
				pObject->SetObjectMatrix(GetObjectMatrixByProcess());	
				pObject->SetRagdollMatrix( true, (D3DXMATRIX*)&matLink);			
			}
			if( pObject->GetObjectType() == BS_SIMULATE_OBJECT ) {
				pObject->SetObjectMatrix(GetObjectMatrixByProcess());	
				pObject->ProcessLinkedPhysicsObject();
			}
		}
	}


	// 링크된 충돌용 액터들 처리
	nSize = m_CollisionBody.size();
	for( i = 0; i < nSize; i++) {		
		
		int nLinkBoneIndex = m_CollisionBody[i].nBoneIndex;
		CBsBone *pBone = m_pAni->FindBone(nLinkBoneIndex);
		BSMATRIX matTrans;
		BsMatrixMultiply(&matTrans, &m_CollisionBody[i].matLocal, pBone->GetTransMatrix());
		BSMATRIX matObject((float*)GetObjectMatrixByProcess());
		BsMatrixMultiply(&matTrans, &matTrans, &matObject);
		
		m_CollisionBody[i].pActor->SetGlobalPose( *(D3DXMATRIX*)&matTrans );
	}

	if( m_bRagdollEnable == true && m_bSimulationMode == false ) {		
		nSize = m_BoneActors.size();
		for( i = 0; i < nSize; i++) {
			BSMATRIX matObject((float*)GetObjectMatrixByProcess());
			BSMATRIX *pVertexTransMat = m_pAni->FindBone(i)->GetTransMatrix();
			BsMatrixMultiply(&matObject, pVertexTransMat, &matObject);
			m_BoneActors[i]->SetTransform( *(D3DXMATRIX*)&matObject );
		}
		m_nRagdollCount++;
		if(m_nRagdollCount > 3) { // 2번이상 입력해주면 변환시킴
			SetSimulationMode( true );
			nSize = m_Joints.size();
			for( i = 0; i < nSize; i++) {
				IBsPhysicsJoint *pJoint = m_Joints[ i ];
				pJoint->SetLimited();
			}
		}		
	}
}

void CBsAniObject::MakeCollisionBody( PHYSICS_COLLISION_CONTAINER *pBody)
{
	int i, nSize;

	nSize = m_CollisionBody.size();
	for( i = 0; i < nSize; i++) {
		delete m_CollisionBody[i].pActor;
	}

	nSize = pBody->CollisionList.size();	
	m_CollisionBody.resize(nSize);

	for( i = 0; i < nSize; i++) {

		PHYSICS_COLLISION_CONTAINER::Elem Part = pBody->CollisionList[i];

		if( g_BsKernel.GetPhysicsMgr()->GetCustomValue() == CBsPhysicsMgr::CUSTOM_EV_IN_ASPHARR_CLOTH ) {
			BsAssert( nSize == 2 );
			if( i == 0 ) {
				Part.radius *= 0.3f;
				Part.height *= 0.3f;
			}
		}

		D3DXVECTOR3 Scale;

		Scale.x = D3DXVec3Length((D3DXVECTOR3*)&GetObjectMatrixByProcess()->_11);
		Scale.y = D3DXVec3Length((D3DXVECTOR3*)&GetObjectMatrixByProcess()->_21);
		Scale.z = D3DXVec3Length((D3DXVECTOR3*)&GetObjectMatrixByProcess()->_31);

		D3DXMATRIX matInvScale, matObject;
		D3DXMatrixScaling(&matInvScale, 1 / Scale.x, 1 / Scale.y, 1 / Scale.z);
		D3DXMatrixMultiply( &matObject, &matInvScale, GetObjectMatrixByProcess() );

		IBsPhysicsActor *pActor;
		if( Part.height == 0.0f ) {
			pActor = CBsPhysicsCollisionSphere::Create(Part.radius * Scale.x, matObject);
		}
		else {
			pActor = CBsPhysicsCollisionCapsule::Create(Part.radius * Scale.x, Part.height * Scale.y, matObject);
		}

		pActor->SetScale( Scale );
		pActor->SetKinematic();
		pActor->SetGroup("CollisionObject");

		
		m_CollisionBody[i].fRadius = Part.radius;
		m_CollisionBody[i].fHeight = Part.height;
		m_CollisionBody[i].pActor = pActor;
		m_CollisionBody[i].matLocal = Part.matLocal;
		m_CollisionBody[i].nBoneIndex = m_pAni->GetBoneIndex( Part.szLinkName.c_str() );
	}
}

// 추가
void	CBsAniObject::CurAniPopFront()
{
	// mruete: prefix bug 295: added m_nCurrentAniSize <= MAX_CURRENT_ANI_SIZE
	BsAssert( m_nCurrentAniSize > 0 && m_nCurrentAniSize <= MAX_CURRENT_ANI_SIZE );

	memcpy( m_nCurrentAni, m_nCurrentAni + 1, sizeof( int ) * ( m_nCurrentAniSize - 1 ) );
	memcpy( m_fCurrentFrame, m_fCurrentFrame + 1, sizeof( float ) * ( m_nCurrentAniSize - 1 ) );
	memcpy( m_nCurrentAniBone, m_nCurrentAniBone + 1, sizeof( int ) * ( m_nCurrentAniSize - 1 ) );
	m_nCurrentAniSize--;
}


void	CBsAniObject::RecalcAni(void)
{
	BlendAni( m_nCurrentAni[0], m_fCurrentFrame[0], 0.5f, m_nCurrentAniBone[0] );
	CurAniPopFront();
}



/******************************************************************************************
Class Name : CBsRMAniObject
******************************************************************************************/
bool CBsRMAniObject::GetBox3( Box3 & B )
{
	if( m_pMesh )
	{
		if( m_bSimulationMode ) {	// 시뮬레이션의 경우 BoundingBox 를 다시 계산해준다.

			if( m_BoneActors.empty() ) {
				return false;
			}

			AABB ragdollAABB;
			ragdollAABB.Reset();

			int i, nCount;

			float fMaxExtent = 0.f;

			nCount = m_BoneActors.size();
			for(i = 0; i < nCount; i++) {

				IBsPhysicsActor *pActor = m_BoneActors[i];

				BSVECTOR Pos;
				pActor->GetGlobalPosition(*(D3DXVECTOR3*)&Pos);

				fMaxExtent = __max(fMaxExtent, pActor->GetBoxExtend().x);
				fMaxExtent = __max(fMaxExtent, pActor->GetBoxExtend().y);
				fMaxExtent = __max(fMaxExtent, pActor->GetBoxExtend().z);

				ragdollAABB.Merge( Pos );
			}

			ragdollAABB.Vmin -= BSVECTOR(fMaxExtent, fMaxExtent, fMaxExtent);
			ragdollAABB.Vmax += BSVECTOR(fMaxExtent, fMaxExtent, fMaxExtent);

			BSVECTOR vExtent = ragdollAABB.GetExtent();

#if _OPT_SHINJICH_BSANIOBJECT_CPP
			*((BSVECTOR*) B.E) = vExtent;
#else
			B.E[0] = vExtent.x;
			B.E[1] = vExtent.y;
			B.E[2] = vExtent.z;
#endif
			B.C = ragdollAABB.GetCenter();
			m_vecCenter = *(D3DXVECTOR3*)&B.C;

			B.A[ 0 ] = BSVECTOR(1, 0, 0);
			B.A[ 1 ] = BSVECTOR(0, 1, 0);
			B.A[ 2 ] = BSVECTOR(0, 0, 1);

			return true;
		}
		else {
			D3DXMATRIX *pMatrix;
			D3DXMATRIX matRootBone;
			AABB* pBB = m_pMesh->GetBoundingBox();

#if _OPT_SHINJICH_BSANIOBJECT_CPP
			*((BSVECTOR*) B.E) = pBB->GetExtent();
#else
			D3DXVECTOR3	tmpVec = *(D3DXVECTOR3*)&pBB->GetExtent();
			B.E[0] = tmpVec.x;
			B.E[1] = tmpVec.y;
			B.E[2] = tmpVec.z; // 익스텐드 벡터
#endif
			pMatrix = GetObjectMatrixByProcess();
			m_pAni->GetRootBoneMatrix( m_nCurrentAni[0], m_fCurrentFrame[0], &matRootBone, m_nCalcAniPosition );
			D3DXMatrixMultiply( &matRootBone, &matRootBone, pMatrix);

			memcpy(&B.C, &(matRootBone._41), sizeof(BSVECTOR));
			m_vecCenter = *(D3DXVECTOR3*)&B.C;

#if _OPT_SHINJICH_BSANIOBJECT_CPP
			// ...should remove lhs
			B.A[0] = __lvx( &(matRootBone._11), 0 );
			B.A[1] = __lvx( &(matRootBone._21), 0 );
			B.A[2] = __lvx( &(matRootBone._31), 0 );
#else
			memcpy(&B.A[0], &(matRootBone._11), sizeof(BSVECTOR));
			memcpy(&B.A[1], &(matRootBone._21), sizeof(BSVECTOR));
			memcpy(&B.A[2], &(matRootBone._31), sizeof(BSVECTOR));
#endif
			return true;
		}
	}
	return false;
}