#pragma once
#include "assocarray.h"

class BStream;
class CFcState
{
public:
	CFcState( const char *szFileName );
	virtual ~CFcState();

	enum BLOCK_TYPE {
		BT_UNKNOWN = -1,
		BT_INT,
		BT_FLOAT,
		BT_ENUM,
	};
	struct StateBlock {
		std::string szStr;
		BLOCK_TYPE BlockType;
		union {
			void *pValue;
			float fValue;
			int nValue;
		};
	};

protected:
	std::vector<StateBlock *> m_pVecBlockList;

	assoc_array<int, string_mod_hash_fn> m_Hash;

	void AddStateBlock( StateBlock *pBlock );
public:
	inline int GetIndex( const char *szStr );

	BLOCK_TYPE GetStateType( const char *szStr );
	BLOCK_TYPE GetStateType( DWORD dwIndex );

	void *GetStateValue( const char *szStr );
	int GetStateValueInt( const char *szStr );
	float GetStateValueFloat( const char *szStr );
	void *GetStateValue( DWORD dwIndex );
	int GetStateValueInt( DWORD dwIndex );
	float GetStateValueFloat( DWORD dwIndex );

	void *GetStateValuePtr( const char *szStr );


	void SetStateValue( const char *szStr, void *pValue );
	void SetStateValueInt( const char *szStr, int nValue );
	void SetStateValueFloat( const char *szStr, float fValue );
	void SetStateValue( DWORD dwIndex, void *pValue );
	void SetStateValueInt( DWORD dwIndex, int nValue );
	void SetStateValueFloat( DWORD dwIndex, float fValue );

};