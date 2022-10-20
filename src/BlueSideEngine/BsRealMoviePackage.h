#pragma once

#include "BsKernel.h"

#define LIGHT_STR_MAX				32
#define LIGHT_DATA_BLOCK_SIZE		48

#pragma pack( push )
#pragma pack( 4 )

struct stLightPack
{
	struct stName
	{
		char	szLightName[LIGHT_STR_MAX];
	} NameBlock;

	struct stLightData
	{
		CCrossVector crossLight;
	} DataBlock;

	stLightPack();

	void	Save(BStream *pStream,int nNBSize,int nDBSize);
	void	Load(BStream *pStream,int nNBSize,int nDBSize);
};

#pragma pack( pop )

#define POINT_LIGHT_STR_MAX				32

struct stPointLightPack
{
	struct stName
	{
		char	szLightName[POINT_LIGHT_STR_MAX];
	} NameBlock;

	struct stPointLightData
	{
		float			fRadius;	// ¹Ý°æ
		D3DCOLORVALUE	LightColor;
		D3DXVECTOR3		vecPos;
		float			fHeight;
		int				nObjectIndex;
	} DataBlock;

	stPointLightPack()
	{
		memset( NameBlock.szLightName, 0, POINT_LIGHT_STR_MAX );
	}

	void	UpdateLight(void)
	{
		g_BsKernel.SendMessage( DataBlock.nObjectIndex, 
			BS_SET_POINTLIGHT_COLOR,
			(DWORD)&DataBlock.LightColor );  //d3dcolorvalue*
	}

	void	Save(BStream *pStream);
	void	Load(BStream *pStream);
};

struct MatchPointLightName
{
	char* m_pszNamePtr;
	explicit MatchPointLightName( char* szName ) : m_pszNamePtr( szName ) {}

	bool	operator()( const stPointLightPack Pack )
	{
		return ( strcmp( Pack.NameBlock.szLightName , m_pszNamePtr ) == 0 );
	}
};

#define ITEMNAME_MAX	130

struct stPackageItem
{
	struct _DataBlock
	{
		BOOL	bUse;
		int		nType;
		DWORD	dwObjPtr;
	} DataBlock;

	struct _NameBlock
	{
		char	szItemName[ITEMNAME_MAX];
	} NameBlock;
	

	stPackageItem()
	{
		DataBlock.bUse = FALSE;
		DataBlock.nType = 0;
		memset( NameBlock.szItemName, 0, ITEMNAME_MAX );
		DataBlock.dwObjPtr = 0;
	}

	void	Save(BStream *pStream);
	void	Load(BStream *pStream);
};

class CRMPackage
{
public:

	enum
	{
		eRMPackageType_none = 0,
		eRMPackageType_Effect,
		eRMPackageType_Wave,
		eRMPackageType_Lip,
		eRMPackageType_MAX,

		eItemMax = 12,
		eFileNameLength=64,
	};

	CRMPackage()
	{
		memset( m_szName, 0, eFileNameLength );
	};
	~CRMPackage(){};

	// Set
	void	SetName(char *szName)	{	strcpy( m_szName, szName );	}
	void	SetItemName(int nIdx,char* szName)
	{
		if( szName == NULL || szName[0] == NULL )
			memset( m_Item[nIdx].NameBlock.szItemName , 0, eFileNameLength);
		else
			strcpy( m_Item[nIdx].NameBlock.szItemName, szName );
	}

	void	SetItemType(int nIdx,int nType)	{	m_Item[nIdx].DataBlock.nType = nType;}
	void	SetItemUse(int nIdx,BOOL bUse)	{	m_Item[nIdx].DataBlock.bUse = bUse;	}

	// Get
	const char*	GetName(void)			{	return m_szName;				}
	char*	GetItemName(int nIdx)	{	return m_Item[nIdx].NameBlock.szItemName;	}
	int		GetItemType(int nIdx)	{	return m_Item[nIdx].DataBlock.nType;		}
	BOOL	GetItemUse(int nIdx)	{	return m_Item[nIdx].DataBlock.bUse;		}

	void	Save(BStream *pStream);
	void	Load(BStream *pStream);

	float	GetStartTime(void)		{	return SubData.m_fTime;			}
	DWORD	GetStartTick(void)		{	return SubData.m_dwTick;		}

	void	SetStartTime(float fTime,DWORD dwTick)	
	{	
		SubData.m_fTime = fTime;		
		SubData.m_dwTick = dwTick;
	}

	void	RecalcItems(void);
	int		GetUseItemCnt(void)	{	return m_nItemCnt;	}

#ifdef _USAGE_TOOL_
	void	ChangeFPS(void);
#endif

protected:

	char   m_szName[eFileNameLength];

	struct 
	{
		float  m_fTime;
		DWORD	m_dwTick;
	} SubData;
	 
	stPackageItem	m_Item[eItemMax];

	// Play ¿ë
	int		m_nItemCnt;
};


struct MatchPackageName
{
	char* m_pszNamePtr;
	explicit MatchPackageName( char* szName ) : m_pszNamePtr( szName ) {}

	bool	operator()( CRMPackage& Pack )
	{
		return ( strcmp( Pack.GetName() , m_pszNamePtr ) == 0 );
	}
};


