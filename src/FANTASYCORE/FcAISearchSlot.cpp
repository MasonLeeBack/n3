#include "StdAfx.h"
#include "FcAISearchSlot.h"
#include "FcAIObject.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

CFcAISearchSlot::CFcAISearchSlot( CFcAIObject *pParent )
{
	m_pParent = pParent;
}

CFcAISearchSlot::~CFcAISearchSlot()
{
	SAFE_DELETE_PVEC( m_pVecSlot );
}

void CFcAISearchSlot::SetHandle( int nSlotIndex, GameObjHandle *pHandle )
{
	SlotStruct *pSlot = GetSlot( nSlotIndex );
	if( pSlot ) {
		if( pSlot->nType != -1 && pSlot->nType != 0 ) {
			BsAssert(0);
			return;
		}
		pSlot->nType = 0;
		if( pHandle ) {
			pSlot->Handle = *pHandle;
		}
		else {
			pSlot->Handle.Identity();
			pSlot->bLockLookAt = false;
		}
	}
	else {
		SlotStruct *pStruct = new SlotStruct;
		pStruct->nType = 0;
		pStruct->nSlotIndex = nSlotIndex;
		pStruct->bLockLookAt = false;
		if( pHandle )
			pStruct->Handle = *pHandle;
		else pStruct->Handle.Identity();
		m_pVecSlot.push_back( pStruct );
	}
}

void CFcAISearchSlot::SetPtr( int nSlotIndex, void *pPtr )
{
	SlotStruct *pSlot = GetSlot( nSlotIndex );
	if( pSlot ) {
		if( pSlot->nType != -1 && pSlot->nType != 1 ) {
			BsAssert(0);
			return;
		}
		pSlot->nType = 1;
		if( pPtr )
			pSlot->pPtr = pPtr;
		else {
			pSlot->pPtr = NULL;
			pSlot->bLockLookAt = false;
		}
	}
	else {
		SlotStruct *pStruct = new SlotStruct;
		pStruct->nType = 1;
		pStruct->nSlotIndex = nSlotIndex;
		pStruct->bLockLookAt = false;
		if( pPtr )
			pStruct->pPtr = pPtr;
		else pStruct->pPtr = NULL;

		m_pVecSlot.push_back( pStruct );
	}
}


GameObjHandle *CFcAISearchSlot::GetHandle( int nSlotIndex )
{
	SlotStruct *pSlot = GetSlot( nSlotIndex );
	if( pSlot && pSlot->Handle ) {
		if( pSlot->nType != -1 && pSlot->nType != 0 ) {
			BsAssert(0);
			return NULL;
		}
		return &pSlot->Handle;
	}
	else {
		SetHandle( nSlotIndex, (GameObjHandle*)NULL );
	}
	return NULL;
}

void *CFcAISearchSlot::GetPtr( int nSlotIndex )
{
	SlotStruct *pSlot = GetSlot( nSlotIndex );

	if( pSlot && pSlot->pPtr ) {
		if( pSlot->nType != -1 && pSlot->nType != 1 ) {
			BsAssert(0);
			return NULL;
		}
		return pSlot->pPtr;
	}
	else {
		SetPtr( nSlotIndex, (void*)NULL );
	}
	return NULL;
}


DWORD CFcAISearchSlot::GetSlotCount()
{
	return m_pVecSlot.size();
}

CFcAISearchSlot::SlotStruct *CFcAISearchSlot::GetSlot( int nSlotIndex )
{
	for( DWORD i=0; i<m_pVecSlot.size(); i++ ) {
		if( m_pVecSlot[i]->nSlotIndex == nSlotIndex ) return m_pVecSlot[i];
	}
	return NULL;
}

int CFcAISearchSlot::GetSlotType( int nSlotIndex )
{
	SlotStruct *pSlot = GetSlot( nSlotIndex );
	if( pSlot == NULL ) {
		SlotStruct *pStruct = new SlotStruct;
		pStruct->nType = -1;
		pStruct->nSlotIndex = nSlotIndex;
		pStruct->bLockLookAt = false;
		pStruct->pPtr = NULL;
		pStruct->Handle.Identity();
		m_pVecSlot.push_back( pStruct );
		
		return -1;
	}
	else return GetSlot(nSlotIndex)->nType;
}

CFcAISearchSlot::SlotStruct *CFcAISearchSlot::GetSlotFromIndex( DWORD dwIndex )
{
	return m_pVecSlot[dwIndex];
}

void CFcAISearchSlot::Process()
{
	if( !m_pParent || !m_pParent->GetUnitObjectHandle() ) return;
	for( DWORD i=0; i<m_pVecSlot.size(); i++ ) {
		switch( m_pVecSlot[i]->nType ) {
			case 0:
				if( m_pVecSlot[i]->Handle && m_pVecSlot[i]->bLockLookAt == true ) {
					m_pParent->GetUnitObjectHandle()->CmdLookUp( m_pVecSlot[i]->Handle );
				}
				break;
			case 1:
				break;				
		}
	}
}

void CFcAISearchSlot::SetLookAt( int nSlotIndex, bool bFlag )
{
	SlotStruct *pStruct = GetSlot(nSlotIndex);
	if( pStruct ) pStruct->bLockLookAt = bFlag;
}