#include "StdAfx.h"
#include "BsKernel.h"
#include "FcGlobal.h"
#include "Input.h"
#include "FcBaseObject.h"
#include "DebugUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

CSmartPtrMng<CFcBaseObject> CFcBaseObject::s_ObjectMng(DEFAULT_BASE_OBJECT_POOL_SIZE);
CSmartPtr<CFcBaseObject> CFcBaseObject::s_CameraHandle[];
std::vector< CFcBaseObject * > CFcBaseObject::s_ProcessList;
std::vector< CFcBaseObject * > CFcBaseObject::s_DeleteList;


CFcBaseObject::CFcBaseObject( CCrossVector *pCross, bool bInsertProcess/* = true*/ )
{
	m_bEnable=true;
	m_bDelete=false;
	m_nEngineIndex=-1;
	m_bUpdateObject = true;

	m_bCatchedHero = false;

	if( pCross )
	{
		m_Cross = *pCross;
	}
	D3DXMatrixIdentity( &m_ScaleMat );
	m_dwTickInterval = 1;

	if( bInsertProcess )
	{
		s_ProcessList.push_back( this );
	}
	m_nRtti = -1;
}

CFcBaseObject::~CFcBaseObject()
{
	if(m_nEngineIndex!=-1){
		CBsKernel::GetInstance().DeleteObject(m_nEngineIndex);
		m_nEngineIndex=-1;
	}
	
	s_ObjectMng.DeleteHandle( m_Handle.GetHandle() );
	
}

int CFcBaseObject::Initialize(int nSkinIndex, int nShadowMeshIndex/*=-1*/, ObjectType type /*= Normal*/ )
{
	switch ( type) {
		case NORMAL : m_nEngineIndex = CBsKernel::GetInstance().CreateStaticObjectFromSkin(nSkinIndex);
			break;		
		case SKYBOX : m_nEngineIndex = CBsKernel::GetInstance().CreateSkyBoxObjectFromSkin(nSkinIndex);
			break;
		case WATER : m_nEngineIndex = CBsKernel::GetInstance().CreateWaterObjectFromSkin(nSkinIndex);
			break;
		case WATER_BOME : m_nEngineIndex = CBsKernel::GetInstance().CreateStaticObjectFromSkin(nSkinIndex);
			break;
		default : 
			BsAssert(0);		
	}
	return 1;
}

bool CFcBaseObject::IsValid( CSmartPtr< CFcBaseObject > Handle )
{
	int nIndex;

	nIndex = Handle.GetIndex();
	if( s_ObjectMng[ nIndex ].GetHandle() != Handle.GetHandle() )
	{
		return false;
	}

	return true;
}

bool CFcBaseObject::Render()
{
	if( ( m_bUpdateObject == false ) || ( m_nEngineIndex == -1 ) )
	{
		return false;
	}

	D3DXMATRIX RenderMat;
	D3DXMatrixMultiply( &RenderMat, &m_ScaleMat, m_Cross );
	CBsKernel::GetInstance().UpdateObject( m_nEngineIndex, &RenderMat );

	return true;
}

void CFcBaseObject::ProcessKeyEvent(int nKeyCode, KEY_EVENT_PARAM *pKeyParam)
{
	switch(nKeyCode){
		case 16:
			CalculateMoveDir(pKeyParam, 0);
			m_Cross.MoveFrontBack(10.0f);
			break;
	}
}

void CFcBaseObject::CalculateMoveDir(KEY_EVENT_PARAM *pParam, int nCamIndex)
{
	D3DXVECTOR3 KeyVector, ZDir, XDir, UpVec, MoveDir;
	CCrossVector *pCross;

	KeyVector.x=(float)pParam->nPosX;
	KeyVector.y=0.0f;
	KeyVector.z=(float)pParam->nPosY;
	D3DXVec3Normalize(&KeyVector, &KeyVector);

	if(D3DXVec3Length(&KeyVector)<=0.0f){
		return ;
	}

	if(s_CameraHandle[nCamIndex]){
		pCross=s_CameraHandle[nCamIndex]->GetCrossVector();
		ZDir=pCross->m_ZVector;
		ZDir.y=0;
		D3DXVec3Normalize(&ZDir, &ZDir);
		UpVec=D3DXVECTOR3(0, 1, 0);
		D3DXVec3Cross(&XDir, &UpVec, &ZDir);
		MoveDir=XDir*KeyVector.x;
		MoveDir+=ZDir*KeyVector.z;
		D3DXVec3Normalize(&MoveDir, &MoveDir);
	}
	else{
		MoveDir=D3DXVECTOR3(0, 0, 1);
	}

	m_Cross.m_ZVector=MoveDir;
	m_Cross.UpdateVectors();
}

void CFcBaseObject::AddProcessList()
{
	s_ProcessList.push_back( m_Handle.GetPointer() );
}

void CFcBaseObject::ProcessObjects()
{
	int i, nSize;

	nSize = ( int )s_ProcessList.size();
	for( i = 0; i < nSize; i++ )
	{
		if( s_ProcessList[ i ] )
		{
			s_ProcessList[ i ]->Process();
		}
	}
}

void CFcBaseObject::PostProcessObjects()
{
	int i, nSize;

	nSize = ( int )s_ProcessList.size();
	for( i = 0; i < nSize; i++ )
	{
		if( s_ProcessList[ i ] )
		{
			s_ProcessList[ i ]->PostProcess();
		}
	}
}

int g_nDebugProcessCount;
void CFcBaseObject::RenderObjects()
{
	std::vector< CFcBaseObject * >::iterator it;

	g_nDebugProcessCount = 0;
	it = s_ProcessList.begin();
	while( it != s_ProcessList.end() )
	{
		if( ( *it )->IsDelete() )
		{
			it = s_ProcessList.erase( it );
			continue;
		}
		else
		{
			( *it )->Render();
		}
		it++;
		g_nDebugProcessCount++;
	}

	int i, nSize;
	nSize = ( int )s_DeleteList.size();
	for( i = 0; i < nSize; i++ )
	{
		if( s_DeleteList[ i ] )
		{
			delete s_DeleteList[ i ];
		}
	}
	s_DeleteList.clear();
}

void CFcBaseObject::DebugRenderObjects()
{
	int i, nSize;

	nSize=CFcBaseObject::s_ObjectMng.Size();
	for(i=0;i<nSize;i++){
		if(CFcBaseObject::s_ObjectMng[i] && CFcBaseObject::s_ObjectMng[ i ]->IsEnable() ){
			CFcBaseObject::s_ObjectMng[i]->DebugRender();
		}
	}
}

CSmartPtr<CFcBaseObject> CFcBaseObject::GetFcObjectHandle(int nIndex)
{
	BsAssert(CFcBaseObject::s_ObjectMng[nIndex]);

	return CFcBaseObject::s_ObjectMng[nIndex];
}


void CFcBaseObject::ReleaseObjects()
{
	int i;

	for( i=0; i < CFcBaseObject::s_ObjectMng.Size(); i++ ) {
		if( CFcBaseObject::s_ObjectMng[ i ] ) {
			delete CFcBaseObject::s_ObjectMng[ i ];
		}
	}

	s_ProcessList.clear();
	s_DeleteList.clear();
}

void CFcBaseObject::ShowAllObjectParts( bool bShow )
{
	int i;

	for( i=0; i < CFcBaseObject::s_ObjectMng.Size(); i++ ) {
		if( CFcBaseObject::s_ObjectMng[ i ] ) {
			CFcBaseObject::s_ObjectMng[ i ]->ShowParts( bShow );
		}
	}
}

void CFcBaseObject::Enable( bool bEnable )
{ 
	if( m_bEnable == bEnable ) return;
	m_bEnable = bEnable;
}

