#pragma once
#include "FcGameObject.h"

class CFcAIObject;
class CFcAISearchSlot {
public:
	CFcAISearchSlot( CFcAIObject *pParent );
	virtual ~CFcAISearchSlot();

	struct SlotStruct {
		int nSlotIndex;

		int nType;
		GameObjHandle Handle;
		void *pPtr;

		bool bLockLookAt;
	};
protected:
	std::vector<SlotStruct *> m_pVecSlot;
	CFcAIObject *m_pParent;

	SlotStruct *GetSlot( int nSlotIndex );

public:
	DWORD GetSlotCount();

	void SetHandle( int nSlotIndex, GameObjHandle *pHandle );
	void SetPtr( int nSlotIndex, void *pPtr );

	GameObjHandle *GetHandle( int nSlotIndex );
	void *GetPtr( int nSlotIndex );

	int GetSlotType( int nSlotIndex );
	void SetLookAt( int nSlotIndex, bool bFlag );

	SlotStruct *GetSlotFromIndex( DWORD dwIndex );

	void Process();
};