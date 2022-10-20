#include "stdafx.h"
#include "FcFXSimplePlay.h"
#include "Parser.h"
#include "Token.h"
#include "BSKernel.h"
#include "FcGameObject.h"
#include "./Data/FXList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG




CFcFxSimplePlayInfo::CFcFxSimplePlayInfo(int iHandle)
{
	m_nCreateType = 0;
    m_Handle  = iHandle;
    m_nObjectIndex = -1;
//	m_nParentObjectIndex = -1;
	m_nBoneIndex = -1;

}

CFcFxSimplePlayInfo::CFcFxSimplePlayInfo(int iHandle, GameObjHandle &Handle /*int nParentObjectIndex*/, int nBoneIndex)
{
	m_nCreateType = 1;
	m_Handle  = iHandle;
	m_hParent = Handle;
	m_nObjectIndex = -1;
//	m_nParentObjectIndex = nParentObjectIndex;
	m_nBoneIndex = nBoneIndex;
}

CFcFxSimplePlayInfo::CFcFxSimplePlayInfo(int iHandle, GameObjHandle &Handle )
{
	m_nCreateType = 2;
	m_Handle  = iHandle;
	m_hParent = Handle;
	m_nObjectIndex = -1;
//	m_nParentObjectIndex = -1;
	m_nBoneIndex = -1;
}

CFcFxSimplePlayInfo::~CFcFxSimplePlayInfo()
{	
	int nEngineIdx = -1;
	if(m_hParent && CFcBaseObject::IsValid( m_hParent ) )
	{
		nEngineIdx = m_hParent->GetEngineIndex();
		if(m_nCreateType == 1 && nEngineIdx != -1&& m_nObjectIndex != -1)	
		{
			DebugString("Linked SimplePlay Unlink- Parent %d, obj %d\n", nEngineIdx, m_nObjectIndex);
			g_BsKernel.SendMessage( nEngineIdx, BS_UNLINKOBJECT, m_nObjectIndex);
			m_hParent.Identity();
		}
	}

	
	if(m_nObjectIndex != -1)
	{
		g_BsKernel.DeleteObject(m_nObjectIndex);
		m_nObjectIndex = -1;
	}
}

void CFcFxSimplePlayInfo::Process()
{
	if(m_nObjectIndex < 0)
	{
		return;
	}
	if( g_BsKernel.GetFXObjectState( m_nObjectIndex ) == CBsFXObject::STOP ) return;
	switch( m_nCreateType ) {
		case 0:
			g_BsKernel.UpdateObject( m_nObjectIndex, m_Cross );
			break;
		case 1:
//			g_BsKernel.UpdateObject( m_nObjectIndex, m_Cross );
			break;
		case 2:
			{
				if( !CFcBaseObject::IsValid( m_hParent ) ) break;
				m_Cross.m_PosVector = m_hParent->GetCrossVector()->m_PosVector + m_vCenter;
				g_BsKernel.UpdateObject( m_nObjectIndex, m_Cross );
			}
			break;
	}
}

void CFcFxSimplePlayInfo::BeginPlay( int nLoop )
{
    m_nObjectIndex = g_BsKernel.CreateFXObject( m_Handle );
    if( m_nObjectIndex == -1 ) 
	{
		DebugString("%d Invalid FX handle", m_Handle);
		_DEBUGBREAK;
		return;
	}
	int nParentEngineIdx = -1;
	if(m_hParent)
		nParentEngineIdx = m_hParent->GetEngineIndex();
	switch( m_nCreateType ) {
		case 0:
			break;
		case 1:
			if(nParentEngineIdx != -1)
				g_BsKernel.SendMessage( nParentEngineIdx, BS_LINKOBJECT, m_nBoneIndex, m_nObjectIndex );
			break;
		case 2:
			m_vCenter = m_Cross.m_PosVector - m_hParent->GetCrossVector()->m_PosVector;
			break;
	}

	if( ( m_nCreateType == 1 ) && ( nParentEngineIdx != -1 ) )	// Link 됐을때는 UpdateObject 필요 없이 FX 잠깐 Pause 시켜놓고 실제로 Link가 활성화 될때 Resume시킨다 by mapping
	{
	    g_BsKernel.SetFXObjectState( m_nObjectIndex, CBsFXObject::PAUSE, nLoop );
	}
	else
	{
	    g_BsKernel.SetFXObjectState( m_nObjectIndex, CBsFXObject::PLAY, nLoop );
		g_BsKernel.UpdateObject( m_nObjectIndex, m_Cross );
	}
}

bool CFcFxSimplePlayInfo::IsDestroy()
{
    CBsFXObject::STATE State = g_BsKernel.GetFXObjectState( m_nObjectIndex );
    if( State == CBsFXObject::STOP ) return true;
    return false;
}

bool CFcFxSimplePlayInfo::SetDestroy()
{
	int nEngineIdx = -1;
	if(m_hParent)
		nEngineIdx = m_hParent->GetEngineIndex();

	if(m_nCreateType == 1 && nEngineIdx != -1)	//	if( m_nParentObjectIndex != -1) 
	{
		DebugString("Linked SimplePlay Unlink- Parent %d, obj %d\n", nEngineIdx, m_nObjectIndex);
		g_BsKernel.SendMessage( nEngineIdx, BS_UNLINKOBJECT, m_nObjectIndex);
		m_hParent.Identity();
		//		m_nParentObjectIndex = -1;
	}	
	g_BsKernel.SetFXObjectState( m_nObjectIndex , CBsFXObject::STOP);
	return true;
}



//---------------------------------------------------------------------------------------------------------
CFcFXSimplePlay::CFcFXSimplePlay()
{
    m_state = PLAY;
	memset( m_szFxFilePath, 0, sizeof(m_szFxFilePath) );
#ifndef _LTCG
	SetFxRtti(FX_TYPE_SIMPLE_PLAY);
#endif //_LTCG
}

void CFcFXSimplePlay::ClearFxObjList()
{
	if( !m_FxObjectList.empty() )
	{
		std::list<CFcFxSimplePlayInfo*>::iterator iter;
		for( iter = m_FxObjectList.begin() ; iter != m_FxObjectList.end() ; )
		{
			CFcFxSimplePlayInfo* tmp = *iter;
			if( tmp )
			{
				iter = m_FxObjectList.erase(iter);
				delete tmp;
			}
			else
			{
				++iter;
			}
		}
		m_FxObjectList.clear();
	}
}
CFcFXSimplePlay::~CFcFXSimplePlay()
{
	//SAFE_DELETE_PVEC( m_FxObjectList );
	ClearFxObjList();

	m_ParticleList.clear();

	for( DWORD i=0; i<m_FxTemplateList.size(); i++ )
	{
		g_BsKernel.ReleaseFXTemplate( m_FxTemplateList[i] );
		m_FxTemplateList[i] = -1;
	}
	m_FxTemplateList.clear();
}

void CFcFXSimplePlay::Load( char *szFileName, int *pUseTable )
{
	if( szFileName == NULL ) {
		if( m_szFxFilePath[0] ) { //aleksger: prefix bug 745: Checking automatic array instead of local value.
			ClearFxObjList();

			for( DWORD i=0; i<m_FxTemplateList.size(); i++ ) {
				g_BsKernel.ReleaseFXTemplate( m_FxTemplateList[i] );
				m_FxTemplateList[i] = -1;
			}
			m_FxTemplateList.clear();

			Load( m_szFxFilePath, pUseTable );
		}
		return;
	}
    bool result;
    Parser parser;
    TokenList toklist;
    parser.Create();
    parser.EnableComments( true );
    char szFxFilePath[MAX_PATH];

    result = parser.ProcessSource( g_BsKernel.GetCurrentDirectory(), szFileName, &toklist );
	strcpy( m_szFxFilePath, szFileName );
    if( result == false )
    {
        BsAssert(0 && "Fx Table Read Error");
        return;
    }
    result = parser.ProcessHeaders( &toklist );
    if( result == false )
    {
        BsAssert(0 && "Fx Table Read Error");
        return;
    }
    parser.ProcessMacros( &toklist );
    TokenList::iterator itr = toklist.begin();

    int nCnt=0;

	
    g_BsKernel.chdir( "FX" );

    while( 1 )
    {
        if( itr == toklist.end() )
        {
            break;
        }
//        BsAssert( nCnt == itr->GetInteger() && "FXList Type 순서 안맞음" );
		BsAssert( itr->IsInteger() );
		int nID = itr->GetInteger();		// 겹치면 안됨


        itr++;
		BsAssert( itr->IsString() );
		sprintf(szFxFilePath,"%s%s",g_BsKernel.GetCurrentDirectory(),itr->GetString());

		itr++;
		BsAssert( itr->IsInteger() );
		int nUseType =  itr->GetInteger();
		
		if(nUseType == FCFX_USE_SIGNAL)
			AddFx( nID, szFxFilePath, pUseTable[ nID ] != 0 );
		else if(nUseType == FCFX_USE_COMMON)
			AddFx( nID, szFxFilePath, true );
        itr++;
        ++nCnt;
    }

    g_BsKernel.chdir( ".." );
}

void CFcFXSimplePlay::Process()
{
    std::list<PARTICLE_BY_PARENT>::iterator it;

	if( !m_ParticleList.empty() )
	{
		for( it = m_ParticleList.begin(); it != m_ParticleList.end();) 
		{
			PARTICLE_BY_PARENT pData = *it;
			// TODO: 이거 잘 되는지 체크 필요
			if( g_BsKernel.IsPlayParticleObject(pData.nParticleHandle) == false ) 
			{
				it = m_ParticleList.erase( it );
			}
			else 
			{
				CCrossVector cvParticle = *(pData.hParent->GetCrossVector());
				cvParticle.m_PosVector += pData.vecOffset;
				g_BsKernel.UpdateParticle(pData.nParticleHandle, cvParticle);
				++it;
			}
		}
	}
}

void CFcFXSimplePlay::Update()
{
	if( !m_FxObjectList.empty() )
	{
		std::list<CFcFxSimplePlayInfo*>::iterator iter;
		for( iter = m_FxObjectList.begin() ; iter != m_FxObjectList.end() ; )
		{
			CFcFxSimplePlayInfo* tmp = *iter;
			if( tmp->IsDestroy() == true )
			{
				iter = m_FxObjectList.erase(iter);
				delete tmp;
			}
			else
			{
				tmp->Process();
				++iter;
			}
		}
	}
}

void CFcFXSimplePlay::Render(C3DDevice *pDevice)
{
	
}

void CFcFXSimplePlay::AddFx( int nID, char *szFxFilePath, bool bLoad )
{
	if( bLoad )
	{
		DebugString( "Pre-loading Fx ID: %d (%s)\n", nID, szFxFilePath );

		int nFxHandle = g_BsKernel.LoadFXTemplate( nID, szFxFilePath );
		BsAssert( nFxHandle != -1 && "Fx load fail" );
		BsAssert( nID == -1 || nID == nFxHandle );

		m_FxTemplateList.push_back( nFxHandle );
	}
	else
	{
		m_FxTemplateList.push_back( -1 );
	}
}

void CFcFXSimplePlay::PlayFx(int iHandle,CCrossVector *pCross, int nLoop)
{
    BsAssert( iHandle >=0 && iHandle < (int)m_FxTemplateList.size() );

	/*
    m_nObjectIndex = g_BsKernel.CreateFXObject( m_FxTemplateList[iHandle] );
    if( m_nObjectIndex == -1 ) return;

    g_BsKernel.SetFXObjectState( m_nObjectIndex, CBsFXObject::PLAY );
    g_BsKernel.UpdateObject( m_nObjectIndex, Cross );

    m_FxObjectList.push_back( m_nObjectIndex );
	*/

	if( m_FxTemplateList[iHandle] == -1 )
	{
		DebugString( "Invalid value! %d \n", iHandle );
		_DEBUGBREAK;
	}

    CFcFxSimplePlayInfo *pFxPlayInfo = new CFcFxSimplePlayInfo( m_FxTemplateList[iHandle] );
    pFxPlayInfo->SetCross(*pCross);
    pFxPlayInfo->BeginPlay( nLoop );    
    m_FxObjectList.push_back( pFxPlayInfo );
}

void CFcFXSimplePlay::PlayLinkedFx( int iHandle, GameObjHandle hParnet/*int nParentObjectIndex*/, int nBoneIndex,  int nLoop )
{
	if( m_FxTemplateList[iHandle] == -1 )
	{
		DebugString( "Invalid value! %d \n", iHandle );
		_DEBUGBREAK;
	}
	CFcFxSimplePlayInfo *pFxPlayInfo = new CFcFxSimplePlayInfo( m_FxTemplateList[iHandle], hParnet, nBoneIndex );
//	pFxPlayInfo->SetCross(*pCross);
	pFxPlayInfo->BeginPlay( nLoop );    
	m_FxObjectList.push_back( pFxPlayInfo );
}

void CFcFXSimplePlay::PlayFxByParent( int iHandle, GameObjHandle hParent, CCrossVector *pCross, int nLoop )
{
	if( m_FxTemplateList[iHandle] == -1 )
	{
		DebugString( "Invalid value! %d \n", iHandle );
		_DEBUGBREAK;
	}

	CFcFxSimplePlayInfo *pFxPlayInfo = new CFcFxSimplePlayInfo( m_FxTemplateList[iHandle], hParent );
	pFxPlayInfo->SetCross(*pCross);
	pFxPlayInfo->BeginPlay( nLoop );    
	m_FxObjectList.push_back( pFxPlayInfo );
}

void CFcFXSimplePlay::PlayParticleByParent( int nID, GameObjHandle hParent, bool bLoop, bool bIterate, float fScale, D3DXVECTOR3* pOffset/*=NULL*/)
{
    PARTICLE_BY_PARENT Data;
    BsAssert( hParent );
    CCrossVector* pCross = hParent->GetCrossVector();
    Data.nParticleHandle = g_BsKernel.CreateParticleObject( nID, bLoop, bIterate, *pCross, fScale );
    //	BsAssert( Data.nParticleHandle & 0xffff < 0 );
    if(pOffset)
        Data.vecOffset = *pOffset;
    else
        Data.vecOffset = D3DXVECTOR3(0.f, 0.f, 0.f);

    Data.hParent = hParent;

    m_ParticleList.push_back( Data );
}


int CFcFXSimplePlay::ProcessMessage(int nCode,DWORD dwParam1, DWORD dwParam2,DWORD dwParam3)
{
    switch(nCode)
    {
    case FX_INIT_OBJECT:
        {
            Load( ( char * )dwParam1, ( int * )dwParam2 );
        }
        break;
    case FX_PLAY_OBJECT:
        {
            PlayFx(dwParam1,(CCrossVector *)dwParam2, dwParam3);
        }
        break;

	case FX_LINKED_PLAY_OBJECT:
		{
			//PlayLinkedFx(dwParam1, (dwParam2>>16), (dwParam2& 0xffff), (CCrossVector *)dwParam3, 1 );		// 루프 처리 없음

			FX_BY_PARENT_PLAY_FOR *pData = (FX_BY_PARENT_PLAY_FOR *)dwParam1;
			PlayLinkedFx( pData->nFxID, pData->hParent, pData->nBoneIdx, 1 );
		}
		break;
	case FX_PLAY_OBJECT_PARENT:
		{
			FX_BY_PARENT_PLAY_FOR *pData = (FX_BY_PARENT_PLAY_FOR *)dwParam1;
			PlayFxByParent( pData->nFxID, pData->hParent, pData->pCross, 1 );		// 루프 처리 없음
		}
		break;
    case FX_PLAY_SIMPLE_PARENT_PARTICLE:
        {
            PARTICLE_BY_PARENT_PLAY_FOR *pData = (PARTICLE_BY_PARENT_PLAY_FOR *)dwParam1;
            PlayParticleByParent(pData->nParticleID,pData->hParent,pData->bLoop,pData->bIterate,pData->fScale,pData->vecOffset);
        }
        break;
	case FX_LINKED_STOP_OBJECT:
		{
			FX_BY_PARENT_PLAY_FOR *pData = (FX_BY_PARENT_PLAY_FOR *)dwParam1;
			StopLinkedFx( pData->hParent , pData->nBoneIdx );
		}

	}
	if( CFcFXBase::ProcessMessage( nCode, dwParam1, dwParam2, dwParam3) )
		return 1;
    return 0;
}

void CFcFXSimplePlay::StopLinkedFx( GameObjHandle hParent , int nBoneIndex)
{
	if( !m_FxObjectList.empty() )
	{
		std::list<CFcFxSimplePlayInfo*>::iterator iter;
		for( iter = m_FxObjectList.begin() ; iter != m_FxObjectList.end() ; iter++ )
		{
			CFcFxSimplePlayInfo* tmp = *iter;
			if( tmp->IsEqual( hParent, nBoneIndex ) == true )
			{
				tmp->SetDestroy();
			}
			
		}
	}

	/*for( DWORD i=0; i<m_FxObjectList.size(); i++ ) 
	{
		if( m_FxObjectList[i]->IsEqual( hParent, nBoneIndex ) == true ) 
		{
			m_FxObjectList[i]->SetDestroy();
		}
	}*/
}
