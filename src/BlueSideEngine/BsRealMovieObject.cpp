#include "stdafx.h"
#include "BsRealMovieFile.h"
#include "BsPhysicsMgr.h"
// #include "BsrealMovie.h"
#include "BsKernel.h"
#include "BsRealMovieObject.h"

#include "BsRealMovieMsg.h"
#include <io.h>

#include "BsRealMovie.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


// #include "FcPhysicsLoader.h"

extern CBsFileBALM st_BALM;






BOOL	DoesExistFile(const char *lpszFileName)
{
#ifndef _PACKED_RESOURCES

	_finddata_t	fnd;
	intptr_t fhnd;
	char szBuffer[512];

	g_BsKernel.GetCurrentDirectory( 512, szBuffer );

	// strcat( szBuffer, "\\");
	strcat( szBuffer, lpszFileName );

	if( ( fhnd = _findfirst( szBuffer, &fnd ) ) == -1 )
		return FALSE;

	_findclose(fhnd);

#endif

	return TRUE;
}

void Debug_LS(int nLTFId, int nBALMId, int y)
{
	//            Impersonator Studio 에서 만들어 지는 43 개의 음 (WAV 파일을 분석하여 LTF 파일로 기록 합니다)
	//	          |
	static struct SMap {
		char  m_szSound[9];	// 43 개의 음
		float m_fWeight;
		char  m_szAni[14]; // 15 개의 애니 Impersonator Documentation.chm 에 있는 그림을 참조 합니다 (BA=1)
	}
	st_map[43]={
		"Iy",		0.85f, "Eat <EE>",
			"Ih",		0.85f, "If <Ih>",
			"Eh",		0.85f, "If <Ih>",
			"Ey",		0.85f, "If <Ih>",
			"Ae",		0.85f, "If <Ih>",
			"Aa",		0.85f, "Ox <Ao>",
			"Aw",		0.85f, "If <Ih>",
			"Ay",		0.85f, "If <Ih>",
			"Ah",		0.85f, "If <Ih>",
			"Ao",		0.85f, "Ox <Ao>",
			"Oy",		0.85f, "Oat <Oh>",
			"Ow",		0.85f, "Oat <Oh>",
			"Uh",		0.85f, "Oat <Oh>",
			"Uw",		0.85f, "Oat <Oh>",
			"Er",		0.85f, "Earth <Er>",
			"Ax",		0.85f, "If <Ih>",
			"S",		0.85f, "Size <S/Z>",
			"Sh",		0.85f, "Church <Ch/J>",
			"Z",		0.85f, "Size <S/Z>",
			"Zh",		0.85f, "Church <Ch/J>",
			"F",		0.70f, "Fave <F/V>",
			"Th",		0.85f, "Though <Th>",
			"V",		0.70f, "Fave <F/V>",
			"Dh",		0.85f, "Though <Th>",
			"M",		0.85f, "Bump <B/M/P>",
			"N",		0.85f, "New <N/NG>",
			"Ng",		0.85f, "New <N/NG>",
			"L",		0.85f, "Told <T/L/D>",
			"R",		0.85f, "Roar <R>",
			"W",		0.85f, "Wet <W/OO>",
			"Y",		0.85f, "Wet <W/OO>",
			"Hh",		0.85f, "If <Ih>",
			"B",		0.85f, "Bump <B/M/P>",
			"D",		0.85f, "Told <T/L/D>",
			"Jh",		0.85f, "Church <Ch/J>",
			"G",		0.20f, "Cage <K/G>",
			"P",		0.85f, "Bump <B/M/P>",
			"T",		0.85f, "Told <T/L/D>",
			"K",		0.20f, "Cage <K/G>",
			"Ch",		0.85f, "Church <Ch/J>",
			"Sil",		0.00f, "..",
			"ShortSil", 0.20f, "If <Ih>",
			"Flap",		0.50f, "Told <T/L/D>",
	};

	
	char str[128];
	sprintf(
	str,"%d/43   %d/16   %s   %4.2f   %s",
	nLTFId, nBALMId, st_map[nLTFId].m_szSound, st_map[nLTFId].m_fWeight, st_map[nLTFId].m_szAni
	);

	// g_BsKernel.DrawUIText(10,y, 10+600, y + 30, str);
	DebugString( str );

//	D3DCOLORVALUE color = {1.f, 1.f, 1.f, 1.f};
//	g_BsKernel.CreateUIDebug(10, y, str, color);

}

extern float	g_fRMFps; //  = 60.f;

VOID	ChangeTickIntoTime( DWORD dwTick, float& fTime)
{
	//int nMin = dwTick / 3600;
	//int nMinRest = dwTick % 3600;

	//int nSec = nMinRest / 60;
	//int nSecRest = nMinRest % 60;
	//fTime = nMin * 60 + nSec + ( nSecRest / 60.f );

	fTime = (float)dwTick / g_fRMFps;
}

VOID	ChangeNewTickIntoTime(DWORD& dwTick,float& fTime)
{
	if( fTime < 0.f )
	{
		fTime = -1.f;
		dwTick = -1;
		return;
	}
	// 60 fps -> 40 fps
	float fOldTick = (float)dwTick;
	float fNewTick = ( fOldTick * eFps40 ) / eFps60;

	dwTick = (DWORD)fNewTick;
	fTime = (float)dwTick / eFps40;
}

VOID	ChangeNewTickIntoTime(int& nTick,float& fTime)
{
	if( fTime < 0.f )
	{
		fTime = -1.f;
		nTick = -1;
		return;
	}
	// 60 fps -> 40 fps
	float fOldTick = (float)nTick;
	float fNewTick = ( fOldTick * eFps40 ) / eFps60;

	nTick = (int)fNewTick;
	fTime = (float)nTick / eFps40;
}


CRMObject::CRMObject()
{
	memset( m_szName, 0, eFileNameLength );
	memset( m_szSKFileName, 0, eFileNameLength );
	memset( m_szBAFileName, 0, eFileNameLength );

	int i;

	for( i = 0 ; i < eLinkedObjMax ; ++i )
	{
		memset( m_szLinkedObjName[i], 0, eFileNameLength );
		memset( m_szBoneName[i], 0 , eFileNameLength );
	}

	for( i = 0 ; i < ePartsMax ; ++i )
	{
		memset( m_ObjPack[i].m_szSubPath , 0, eFileNameLength);
		memset( m_ObjPack[i].m_szPartsName , 0, eFileNameLength);
		m_ObjPack[i].bUse = 0;
		m_ObjPack[i].nPartsObjIndex = -1;
		m_ObjPack[i].nPartsSkinIndex = -1;
	}

	m_nObjectIndex = -1;
	m_nSkinIndex = -1;
	m_nAniIndex = -1;
	m_pAniObject = NULL;
	m_nAniCount = 0;

	m_nHairSkinIndex = -1;
	m_nHairObjIndex = -1;

	memset( &SubData, 0, sizeof(SubData) );
	// SubData.m_nRot = 0;
	// SubData.m_nObjType = 0;
	// SubData.m_nDataType = 0;

	m_nOldAniIndex = 0;
	m_fOldFrame = 0.f;
	m_nAlphaMode = 0;
	m_bHideObj = FALSE;
	m_bUseObjectAlphaMode = FALSE;

	m_nRenderAniIndex = 0;
	m_fRenderFrame = 0.f;
	m_pCollisionInfo = NULL;

	memset( m_szSkinSubPath , 0, eFileNameLength);
	memset( m_szBAFileSubPath , 0, eFileNameLength);
}

CRMObject::~CRMObject()
{
	RemoveObject();

	/*
	memset( m_szName, 0, eFileNameLength );
	memset( m_szSKFileName, 0, eFileNameLength );
	memset( m_szBAFileName, 0, eFileNameLength );
	*/

	m_nObjectIndex = -1;
	// m_nSkinIndex = -1;
	m_pAniObject = NULL;
	m_nAniCount = 0;

	RemoveSkinNObj();

	SAFE_RELEASE_ANI(m_nAniIndex);
}

void	CRMObject::RemoveActList(void)
{
	mapStrAct::iterator it;
	mapStrAct::iterator itEnd = m_mapAct.end();

	for( it = m_mapAct.begin() ; it != itEnd ; ++it )
	{
		delete it->second;
	}

	m_mapAct.clear();
}

void	CRMObject::RemoveLipList(void)
{
	mapStrLip::iterator it;
	mapStrLip::iterator itEnd = m_mapLip.end();

	for( it = m_mapLip.begin() ; it != itEnd ; ++it )
	{
		delete it->second;
	}

	m_mapLip.clear();
}

void	CRMObject::RemoveFaceList(void)
{
	mapStrFace::iterator it;
	mapStrFace::iterator itEnd = m_mapFace.end();

	for( it = m_mapFace.begin() ; it != itEnd ; ++it )
	{
		delete it->second;
	}

	m_mapFace.clear();
}

void	CRMObject::RemoveBoneList(void)
{
	mapStrBone::iterator it;
	mapStrBone::iterator itEnd = m_mapBone.end();

	for( it = m_mapBone.begin() ; it != itEnd ; ++it )
	{
		delete it->second;
	}

	m_mapBone.clear();
}

// 파츠 지우기..
BOOL	CRMObject::DeleteParts( int nIdx )
{
	if( m_ObjPack[nIdx].nPartsObjIndex != -1 )
	{
#ifdef _DEBUG
		char szTemp[256];
		sprintf( szTemp, "-- before Delete Parts ObjName:[%s] PartsName:[%s]\r\n",
			m_szName,m_ObjPack[nIdx].m_szPartsName);
		DebugString( szTemp );
#endif

		g_BsKernel.SendMessage( m_nObjectIndex, 
								BS_UNLINKOBJECT,
								m_ObjPack[nIdx].nPartsObjIndex );

#ifdef _USAGE_TOOL_
		g_BsKernel.ProcessKernelCommand();
#endif

		// g_BsKernel.SetPreCalcAni( m_nObjectIndex , false);
		g_BsKernel.DeleteObject( m_ObjPack[nIdx].nPartsObjIndex );

#ifdef _XBOX
		if( m_ObjPack[nIdx].nPartsSkinIndex != -1 )
		{
			g_BsKernel.ReleaseSkin( m_ObjPack[nIdx].nPartsSkinIndex );
			m_ObjPack[nIdx].nPartsSkinIndex = -1;
		}
#endif
		m_ObjPack[nIdx].nPartsObjIndex = -1;
		m_ObjPack[nIdx].bUse = 0;
	}
	else
	{
#ifdef _XBOX
		if( m_ObjPack[nIdx].nPartsSkinIndex != -1 )
		{
			g_BsKernel.ReleaseSkin( m_ObjPack[nIdx].nPartsSkinIndex );
			m_ObjPack[nIdx].nPartsSkinIndex = -1;
		}
#endif
		m_ObjPack[nIdx].nPartsObjIndex = -1;
		m_ObjPack[nIdx].bUse = 0;
	}

	return TRUE;
}

CRMAct*		CRMObject::FindAct(char* szActName)
{
	mapStrAct::iterator it = m_mapAct.find( szActName );

	if( it != m_mapAct.end() )
	{
		return it->second;
	}

	return NULL;
}

// 후에 시간별로 정렬하는 루틴 구성할것.
CRMAct*		CRMObject::FindActWithTime( float fTime )
{
	BsAssert( 0 && "Not used" );
	return NULL;
}

CRMLip*		CRMObject::FindLip(char* szLipName)
{
	mapStrLip::iterator it = m_mapLip.find( szLipName );

	if( it != m_mapLip.end() )
	{
		return it->second;
	}

	return NULL;
}

CRMFace*	CRMObject::FindFace(char* szFaceName)
{
	mapStrFace::iterator it = m_mapFace.find( szFaceName );

	if( it != m_mapFace.end() )
	{
		return it->second;
	}

	return NULL;
}

CRMBone*	CRMObject::FindBone(char* szBoneName)
{
	mapStrBone::iterator it = m_mapBone.find( szBoneName );

	if( it != m_mapBone.end() )
	{
		return it->second;
	}

	return NULL;
}

bool	CRMObject::DeleteAct(char* szActName)
{
	CRMAct* pAct = FindAct( szActName );

	if( pAct )
	{
		delete pAct;
        m_mapAct.erase( szActName );
		return true;
	}

	return false;
}

bool	CRMObject::DeleteLip(char* szLipName)
{
	CRMLip* pLip = FindLip( szLipName );

	if( pLip )
	{
		delete pLip;
		m_mapLip.erase( szLipName );
		return true;
	}

	return false;
}

bool	CRMObject::DeleteFace(char* szFaceName)
{
	CRMFace* pFace = FindFace( szFaceName );

	if( pFace )
	{
		delete pFace;
		m_mapFace.erase( szFaceName );
		return true;
	}

	return false;
}

bool		CRMObject::DeleteBone(char* szBoneName)
{
	CRMBone* pBone = FindBone( szBoneName );

	if( pBone )
	{
		delete pBone;
		m_mapBone.erase( szBoneName );
		return true;
	}

	return false;
}

CRMAct*		CRMObject::InsertAct(char* szActName)
{
	std::pair< std::map< std::string, CRMAct* >::iterator, bool > pr;

	CRMAct* pAct = new CRMAct;
	pAct->SetName( szActName );

	pr = m_mapAct.insert( pairStrAct( szActName, pAct ));

	if( pr.second == true )
	{
		(pr.first)->second->m_pParent = this;
		return (pr.first)->second;
	}
	else {
		delete pAct; // aleksger: prefix bug 427: Insertion may fail and result in a memory leak.
		BsAssert(!"Failure to insert an act within CRMObject::InsertAct()");
        return NULL;
	}
}

CRMLip*		CRMObject::InsertLip(char* szLipName)
{
	std::pair< std::map< std::string, CRMLip* >::iterator, bool > pr;

    CRMLip* pLip = new CRMLip;
	pLip->SetName( szLipName );

	pr = m_mapLip.insert( pairStrLip( szLipName, pLip ));

	if( pr.second == true )
	{
		(pr.first)->second->m_pParent = this;
		return (pr.first)->second;
	}
	else 
	{
		delete pLip; // aleksger: prefix bug 428: Insertion may fail and result in a memory leak.
		BsAssert(!"Failure to insert an act within CRMObject::InsertLip()");
		return NULL;
	}
}
CRMFace*	CRMObject::InsertFace(char* szFaceName)
{
	std::pair< std::map< std::string, CRMFace* >::iterator, bool > pr;

	CRMFace* pFace = new CRMFace;
	pFace->SetName( szFaceName );

	pr = m_mapFace.insert( pairStrFace( szFaceName, pFace ));

	if( pr.second == true )
	{
		(pr.first)->second->m_pParent = this;
		return (pr.first)->second;
	}
	else 
	{
		delete pFace; // aleksger: prefix bug 428: Insertion may fail and result in a memory leak.
		BsAssert(!"Failure to insert an act within CRMObject::InsertFace()");
		return NULL;
	}
}

CRMBone*	CRMObject::InsertBone(char* szBoneName)
{
	std::pair< std::map< std::string, CRMBone* >::iterator, bool > pr;

	CRMBone* pBone = new CRMBone;
	pBone->SetName( szBoneName );

	pr = m_mapBone.insert( pairStrBone( szBoneName, pBone ) );

	if( pr.second == true )
	{
		( pr.first )->second->m_pParent = this;
		return ( pr.first )->second;
	}
	else 
	{
		delete pBone; // aleksger: prefix bug 433: Insertion may fail and result in a memory leak.
		BsAssert(!"Failure to insert an act within CRMObject::InsertBone()");
		return NULL;
	}
}

CRMAct*		CRMObject::GetAct(char* szActName)
{
	return FindAct( szActName );
}

CRMLip*		CRMObject::GetLip(char* szLipName)
{
	return FindLip( szLipName );
}

CRMFace*	CRMObject::GetFace(char* szFaceName)
{
	return FindFace( szFaceName );
}

CRMBone*	CRMObject::GetBone(char* szBoneName)
{
	return FindBone( szBoneName );
}

int		CRMObject::ChangeActName(char* szOldName,char* szNewName)
{
	std::pair< std::map< std::string, CRMAct* >::iterator, bool > pr;
	CRMAct* pAct = GetAct( szOldName );

	if( pAct )
	{
		m_mapAct.erase( szOldName );

		pAct->SetName( szNewName );
		pr = m_mapAct.insert( pairStrAct( szNewName, pAct ));

		if( pr.second == true )
			return 1;
	}

	return 0;
}

int		CRMObject::ChangeLipName(char* szOldName,char* szNewName)
{
	std::pair< std::map< std::string, CRMLip* >::iterator, bool > pr;
	CRMLip* pLip = GetLip( szOldName );

	if( pLip )
	{
		m_mapLip.erase( szOldName );

		pLip->SetName( szNewName );
		pr = m_mapLip.insert( pairStrLip( szNewName, pLip ));

		if( pr.second == true )
			return 1;
	}

	return 0;
}

int		CRMObject::ChangeFaceName(char* szOldName,char* szNewName)
{
	std::pair< std::map< std::string, CRMFace* >::iterator, bool > pr;
	CRMFace* pFace = GetFace( szOldName );

	if( pFace )
	{
		m_mapFace.erase( szOldName );

		pFace->SetName( szNewName );
		pr = m_mapFace.insert( pairStrFace( szNewName, pFace ));

		if( pr.second == true )
			return 1;
	}

	return 0;
}

int		CRMObject::ChangeBoneName(char* szOldName,char* szNewName)
{
	std::pair< std::map< std::string, CRMBone* >::iterator, bool > pr;
	CRMBone* pBone = GetBone( szOldName );

	if( pBone )
	{
		m_mapBone.erase( szOldName );

		pBone->SetName( szNewName );
		pr = m_mapBone.insert( pairStrBone( szNewName, pBone ));

		if( pr.second == true )
			return 1;
	}

	return 0;
}

void	CRMObject::RemoveObjWithId(void)
{
	int i;

	for( i = 0 ; i < ePartsMax ; ++i )
	{
		DeleteParts( i );
	}

	CBsRealMovie* pRealMovie = g_BsKernel.GetRealMovie();

	if( pRealMovie->m_pfnClearPhysics != NULL )
	{
		for( i = 0 ; i < (int)m_PhysicsInfoList.size() ; ++i )
		{
			(*pRealMovie->m_pfnClearPhysics)( (VOID*)m_PhysicsInfoList[i] );
		}
	}
	else
	{
#ifdef _USAGE_TOOL_	// Tool 이 RealMovie 툴만 있는거 아니다... 이러면 다른툴 컴파일 안된다..
		DebugString( "m_pfnClearPhysics 값이 NULL 위험함.\r\n" );
#endif
	}

	m_PhysicsInfoList.clear();

	ClearCollisionContainer();

	if( m_nObjectIndex != -1 )
	{
#ifdef _XBOX
		g_BsKernel.SendMessage( m_nObjectIndex, BS_ENABLE_LOD , 1 );
#endif
		g_BsKernel.SetPreCalcAni( m_nObjectIndex , false);
		g_BsKernel.DeleteObject( m_nObjectIndex );

#ifdef _XBOX
		SAFE_RELEASE_SKIN(m_nSkinIndex);
		SAFE_RELEASE_ANI(m_nAniIndex);
#endif

		m_nObjectIndex = -1;
		m_pAniObject = NULL;
	}
}

void	CRMObject::RemoveSkinNObj(void)
{
	RemoveObjWithId();

	DebugString("-- Realmovie RemoveSkinNObj [%s]\n", m_szName );

	if( m_nSkinIndex != -1 )
	{
        g_BsKernel.ReleaseSkin( m_nSkinIndex );
		m_nSkinIndex = -1;
	}

	for( int i = 0 ; i < ePartsMax ; ++i )
	{
		if( m_ObjPack[i].nPartsSkinIndex != -1 )
		{
			g_BsKernel.ReleaseSkin( m_ObjPack[i].nPartsSkinIndex );
			m_ObjPack[i].nPartsSkinIndex = -1;
		}
	}
}

void	CRMObject::RemoveObject(void)
{
	RemoveActList();
	RemoveLipList();
	RemoveFaceList();
	RemoveBoneList();

	// 실 오브젝트 삭제 부분.
	RemoveObjWithId();
}

void	CRMObject::ClearNameBuffer(void)
{
	memset( m_szBAFileName, 0, eFileNameLength );
	memset( m_szSKFileName, 0, eFileNameLength );
}

int		CRMObject::CreateNewObject(void)
{
	char szSkinFullName[512];
	char szBAFullName[512];

	strcpy( szSkinFullName , m_szSkinSubPath );
	strcat( szSkinFullName , m_szSKFileName );

	DebugString("-- Realtime CreateNewObject : [%s] [%s]\n", m_szSKFileName, m_szBAFileName );

	int nSkinIndex;

#ifndef _USAGE_TOOL_
	if( SubData.m_nObjType == eObjType_P1_Weapon )
	{
		nSkinIndex = GetSkinIndexForWeapon();	// 외부에서 세팅된다.
		m_szBAFileName[0] = NULL;

		if( nSkinIndex == -1 ) // 없으면 로드
		{
			nSkinIndex = g_BsKernel.LoadSkin( -1 , szSkinFullName ); // m_szSKFileName );
		}
		else
		{
			g_BsKernel.AddSkinRef( nSkinIndex );
		}
	}
	else
#endif
	{
		// szSkinFullName
#ifdef _USAGE_TOOL_
		nSkinIndex = g_BsKernel.FindSkin( m_szSKFileName );

		if( nSkinIndex != -1 )	// 이미 있다.
		{
			g_BsKernel.AddSkinRef( nSkinIndex );
		}
		else
#endif
		{
			nSkinIndex = g_BsKernel.LoadSkin( -1 , szSkinFullName ); // m_szSKFileName );
		}
	}

	if( nSkinIndex < 0 )
	{
#ifndef _USAGE_TOOL_
		assert( 0 && "RM error : LoadSkin. skin file not found");
#endif

		return -1;
	}

	if ( m_nSkinIndex != -1 )
		g_BsKernel.ReleaseSkin( m_nSkinIndex );

	m_nSkinIndex = nSkinIndex;

	// 있을때
	if( m_szBAFileName[0] )	// && DoesExistFile( m_szBAFileName ) )
	{
		strcpy( szBAFullName , m_szBAFileSubPath );
		strcat( szBAFullName , m_szBAFileName );

		if( DoesExistFile( szBAFullName ) )
		{
			// TODO : 확인해주세요. by jeremy
			SAFE_RELEASE_ANI(m_nAniIndex);
			m_nAniIndex = g_BsKernel.LoadAni( -1, szBAFullName, false );

			if( m_nAniIndex < 0 )
			{
				m_pAniObject = NULL;
				m_nAniCount = 0;
				return -2;
			}

			if( m_szSKFileName[0] )
			{
				m_nObjectIndex = g_BsKernel.CreateRMAniObjectFromSkin(nSkinIndex, m_nAniIndex);
				g_BsKernel.SendMessage( m_nObjectIndex, BS_SET_CALC_ANI_POSITION, 7 );
				SetAniObject();
			}
			else
			{
				m_nObjectIndex = g_BsKernel.CreateAniObject( nSkinIndex, m_nAniIndex );
				g_BsKernel.SendMessage( m_nObjectIndex, BS_SET_CALC_ANI_POSITION, 7 );
				SetAniObject();
			}

			// 본 채크
			m_nBoneIndex  = m_pAniObject->LSFindBoneIndex("B_M_Root");

			if( m_nBoneIndex == -1 )
			{
#ifdef _USAGE_TOOL_
				//char szTemp[200];
				//sprintf(szTemp,"%s 파일에 B_M_Root 본이 존재하지 않습니다.(Lip 불가)", m_szBAFileName );
				//MessageBox( NULL, szTemp , "Warning", MB_OK );
				// DebugString( szTemp );
#endif
			}

#ifdef _XBOX
			g_BsKernel.SendMessage( m_nObjectIndex, BS_ENABLE_LOD , 0 );
#endif

		}
		else	// 그 폴더에 BA 파일이 존재 하지 않는다.
		{
			if( m_szSKFileName[0] )
			{
				m_nObjectIndex = g_BsKernel.CreateStaticObjectFromSkin( nSkinIndex );
			}
			else
			{
				m_nObjectIndex = g_BsKernel.CreateStaticObject( nSkinIndex );
			}

			m_pAniObject = NULL;
			m_nAniCount = 0;
#ifdef _USAGE_TOOL_
			/*
			char szTemp[200];
			sprintf(szTemp,"%s 파일이 존재하지 않습니다.", m_szBAFileName );
			MessageBox( NULL, szTemp , "Warning", MB_OK );
			*/
#endif
		}
	}
	else
	{
		if( m_szSKFileName[0] )
		{
#ifndef _USAGE_TOOL_	// 게임이면 등록 않함.
			if( strcmp( m_szSKFileName, "empty.skin" ) != 0 )
			{
                m_nObjectIndex = g_BsKernel.CreateStaticObjectFromSkin( nSkinIndex );
			}
			else
			{
				m_nObjectIndex = -1;
			}
#else
			m_nObjectIndex = g_BsKernel.CreateStaticObjectFromSkin( nSkinIndex );
#endif
		}
		else
		{
			m_nObjectIndex = g_BsKernel.CreateStaticObject( nSkinIndex );
		}

		m_pAniObject = NULL;
		m_nAniCount = 0;
	}

	return 0;
}

int		CRMObject::LoadParts(int nIdx)	// Set 도 같이 함
{
	if( m_ObjPack[nIdx].m_szPartsName[0] == NULL )
		return 0;

	if( !m_pAniObject )
	{
#ifdef _USAGE_TOOL_
		char szTemp[200];
		sprintf(szTemp,
			"%s Object가 Static Object이므로 Parts를 붙일 수 었습니다.\r\nParts 이름은 지워집니다.",
			m_szSKFileName);
		MessageBox( NULL, szTemp , "Error", MB_OK );

		memset( m_ObjPack[nIdx].m_szPartsName, 0, eFileNameLength );
#endif
		return 0;
	}

	if( m_nObjectIndex == -1 )
	{
#ifdef _USAGE_TOOL_
		char szTemp[255];
		sprintf( szTemp, "오브젝트 생성 이전엔 %s skin을 등록할 수 없습니다.",m_ObjPack[nIdx].m_szPartsName);
		MessageBox( NULL, szTemp, "Error", MB_OK );
#else
		char szTemp[255];
		sprintf( szTemp, "Don't create %s parts skin.",m_ObjPack[nIdx].m_szPartsName);
		BsAssert2( 0, szTemp ); // mruete: prefix bug 425: Changed to BsAssert2
#endif
		return 0;
	}

	// Text 파일 설정하기
	char szTxtName[256];
	char szPhysicsTxtName[512];
	strcpy( szTxtName, m_ObjPack[nIdx].m_szPartsName );
	szTxtName[ strlen( m_ObjPack[nIdx].m_szPartsName ) - 5 ] = NULL;
	strcat( szTxtName, ".txt");

	strcpy( szPhysicsTxtName , m_ObjPack[nIdx].m_szSubPath );
	strcat( szPhysicsTxtName , szTxtName );

	char szPhysicsSkinName[512];
	strcpy( szPhysicsSkinName , m_ObjPack[nIdx].m_szSubPath );
	strcat( szPhysicsSkinName , m_ObjPack[nIdx].m_szPartsName );

	// 후에 없을 부분.
	DebugString("-- Realtime Movie Parts[%d] : Skin[%s] Path[%s]\n", nIdx,
		m_ObjPack[nIdx].m_szPartsName, 
		m_ObjPack[nIdx].m_szSubPath );

#ifdef _USAGE_TOOL_
	if( DoesExistFile( szPhysicsTxtName ) && DoesExistFile( szPhysicsSkinName ) )
#else
	if( DoesExistFile( szPhysicsSkinName ) )
#endif
	{
		// 존재함.
		PHYSICS_DATA_CONTAINER *pContainer;
		pContainer = (PHYSICS_DATA_CONTAINER *)CreatePartsContainer( szPhysicsTxtName );

		if( !pContainer )
		{
#ifdef _USAGE_TOOL_
			MessageBox( NULL, "(PHYSICS_DATA_CONTAINER*) LoadPhysicsData Error", "Error", MB_OK );
#else
			BsAssert( 0 && "(PHYSICS_DATA_CONTAINER*) LoadPhysicsData Error" );
#endif
			return 0;
		}
		
		// Skin 존재함.

		int nPartsSkinIndex = g_BsKernel.FindSkin( m_ObjPack[nIdx].m_szPartsName ); // szPhysicsSkinName );
		DebugString("-- Realtime Movie Parts : SkinIndex [%d]\n", nPartsSkinIndex );

		if( nPartsSkinIndex != -1 )
		{
			g_BsKernel.AddSkinRef( nPartsSkinIndex );
		}
		else
		{
			nPartsSkinIndex = g_BsKernel.LoadSkin( -1, szPhysicsSkinName );
			assert( nPartsSkinIndex != -1 );

			if( nPartsSkinIndex == -1 )
			{
                DebugString("-- Realtime Movie LoadSkin Error!! (-1) [%s]\n",szPhysicsSkinName );
			}
		}

		if( nPartsSkinIndex > -1 )
		{
			//RefreshAni();	// LoadParts 시 알림.
			
            //ProcessCurrentAniForLinkedObj();

			D3DXMATRIX matObject;
			D3DXMatrixIdentity(&matObject);

			// ResetAniObject();

			// 순서상으로 엊갈려서 UpdateObject 를 미리 해줘야 느려지는 문제가 해결됩니다..
			CCrossVector Cross = m_Cross;
			Cross.SetPosition( Cross.GetPosition() + D3DXVECTOR3(0,50000,0));
            g_BsKernel.UpdateObject( m_nObjectIndex, Cross);	

			int nPartsObjectIndex = g_BsKernel.CreateClothObjectFromSkin( 
				nPartsSkinIndex , pContainer, Cross);		// 캐릭터 CrossVector 로 위치 초기화해줍니다..
			g_BsKernel.ShowObject(nPartsObjectIndex, true);

			g_BsKernel.SetPreCalcAni(m_nObjectIndex, true);

			if ( m_ObjPack[nIdx].nPartsSkinIndex != -1 )
				g_BsKernel.ReleaseSkin( m_ObjPack[nIdx].nPartsSkinIndex );

			if ( m_ObjPack[nIdx].nPartsObjIndex != -1 )
				g_BsKernel.DeleteObject( m_ObjPack[nIdx].nPartsObjIndex );

			m_ObjPack[nIdx].nPartsSkinIndex = nPartsSkinIndex;
			m_ObjPack[nIdx].nPartsObjIndex = nPartsObjectIndex;

			LinkParts( nIdx );
		}
		return 1;
	}
#ifdef _USAGE_TOOL_
	else
	{
		if( !DoesExistFile( szPhysicsTxtName ) )
		{
			char szTemp[255];
			sprintf( szTemp, "%s 파일을 찾을 수 가 없습니다.",szTxtName);
			MessageBox( NULL, szTemp, "Error", MB_OK );
		}
		if( !DoesExistFile( szPhysicsSkinName ) )
		{
			char szTemp[255];
			sprintf( szTemp, "%s 파일을 찾을 수 가 없습니다.", m_ObjPack[nIdx].m_szPartsName);
			MessageBox( NULL, szTemp, "Error", MB_OK );
		}
	}
#endif

	return 0;
}

void	CRMObject::RefreshAni(void)
{
	// g_BsKernel.SendMessage( m_nObjectIndex , BS_REFRESH_ANI, TRUE);
}

void	CRMObject::LinkParts(int nIdx)
{
	g_BsKernel.SendMessage( m_ObjPack[nIdx].nPartsObjIndex, BS_PHYSICS_LINK_CHARACTER, m_nObjectIndex);
	//g_BsKernel.SendMessage( m_nObjectIndex, BS_LINKOBJECT, 0, m_ObjPack[nIdx].nPartsObjIndex);
}

BOOL	CRMObject::CheckSamePartsContainer(PHYSICS_DATA_CONTAINER *pNewContainer)
{
	std::vector< PHYSICS_DATA_CONTAINER* >::iterator it = m_PhysicsInfoList.begin();
	std::vector< PHYSICS_DATA_CONTAINER* >::iterator itEnd = m_PhysicsInfoList.end();

	for( ; it != itEnd ; ++it )
	{
		if( pNewContainer == (*it) )
			return TRUE;
	}

	return FALSE;
}

VOID*	CRMObject::CreatePartsContainer(char* szPhysicsTxtName)
{
	CBsRealMovie* pRealMovie = g_BsKernel.GetRealMovie();
	PHYSICS_DATA_CONTAINER *pContainer;

	pContainer = (PHYSICS_DATA_CONTAINER*)
		(*pRealMovie->m_pfnCreatePhysics)( szPhysicsTxtName );

	if( pContainer != NULL )
	{
#ifdef _USAGE_TOOL_
		if( !CheckSamePartsContainer( pContainer ) ) // 해줄필요없을것같습니다..
#endif
		{
            m_PhysicsInfoList.push_back( pContainer );
		}

        return (VOID*)pContainer;
	}

	return NULL;
}

void CRMObject::CreateCollisionContainer()
{
	CBsRealMovie* pRealMovie = g_BsKernel.GetRealMovie();
	
	ClearCollisionContainer();	

	if(pRealMovie->m_pfnCreateCollision != NULL ) {
		m_pCollisionInfo = (PHYSICS_COLLISION_CONTAINER*)
			(*pRealMovie->m_pfnCreateCollision)( m_szSKFileName );
		if( m_pCollisionInfo != NULL ) {
			g_BsKernel.SendMessage( m_nObjectIndex, BS_PHYSICS_COLLISIONMESH, (DWORD) m_pCollisionInfo );
		}
		g_BsKernel.GetPhysicsMgr()->SetCustomValue( CBsPhysicsMgr::CUSTOM_NONE );
	}
}

void CRMObject::ClearCollisionContainer()
{
	if( m_pCollisionInfo == NULL ) {
		return;
	}

	CBsRealMovie* pRealMovie = g_BsKernel.GetRealMovie();
	
	if( pRealMovie->m_pfnClearCollision != NULL ) {
		(*pRealMovie->m_pfnClearCollision)( m_pCollisionInfo );
		m_pCollisionInfo = NULL;
	}	
}

BOOL	CRMObject::SetAniObject(void)
{
	CBsAniObject *pt = (CBsAniObject *)g_BsKernel.GetEngineObjectPtr(m_nObjectIndex); 
	m_pAniObject = pt;
	m_nAniCount = pt->GetAniCount();
	ResetAniObject();

	return TRUE;
}

int		CRMObject::ReLoadObject(void)
{
	int nRet;
	RemoveObjWithId();
	nRet = CreateNewObject();

	for( int i = 0 ; i < ePartsMax ; ++i )
	{
		LoadParts( i );
	}
	CreateCollisionContainer();

	return nRet;
}

int		CRMObject::LoadPartObjects(void)
{
	LARGE_INTEGER liFrequency;
	LARGE_INTEGER liStartTime;
	LARGE_INTEGER liCurTime;
	QueryPerformanceFrequency(&liFrequency);
	QueryPerformanceCounter(&liStartTime);

	for( int i = 0 ; i < ePartsMax ; ++i )
	{
		LoadParts( i );
	}
	CreateCollisionContainer();

	QueryPerformanceCounter(&liCurTime);	
	double dwTime = (double)(( liCurTime.QuadPart - liStartTime.QuadPart ) / ( liFrequency.QuadPart / 1000.f ));
	DebugString("-- Realtime Move [%s] Object LoadParts time (%f ms)\n", GetName(), dwTime);

	return 0;
}


int		CRMObject::LoadLipResource(void)
{
//	char szLipFullName[512];
	int nCnt = 0;
	CRMLip* pLip;
	mapStrLip::iterator it;
	mapStrLip::iterator itEnd = m_mapLip.end();

	for( it = m_mapLip.begin() ; it != itEnd ; ++it )
	{
		pLip = it->second;

		if( pLip->LoadLipResource() )
            ++nCnt;

		/*
		if( pLip->HasLTFFileName() )
		{
			strcpy( szLipFullName, pLip->GetLipPath() );
			strcat( szLipFullName, pLip->GetLTFFileName() );

            pLip->GetLTFObject().FLoad_text_( szLipFullName );//pLip->GetLTFFileName() );
            pLip->ResetEventFlag();
            ++nCnt;
		}
		*/
	}

	return nCnt;
}

int		CRMObject::GetAniLength(int nAniIndex)
{
	if( m_pAniObject )
	{
		m_pAniObject // 참조 값을 보장 받기 위해서 매번 직접 대입 합니다
			= (CBsAniObject *)g_BsKernel.GetEngineObjectPtr( m_nObjectIndex );

		if( nAniIndex < m_nAniCount )	// 주의!!!.. 카운트 확인할것.. 
            return m_pAniObject->GetAniLength( nAniIndex );
	}

	return -1;
}

float	CRMObject::GetAniTime(int nAniIndex)
{
	if( m_pAniObject )
	{
		m_pAniObject // 참조 값을 보장 받기 위해서 매번 직접 대입 합니다
			= (CBsAniObject *)g_BsKernel.GetEngineObjectPtr( m_nObjectIndex );

		if( nAniIndex < m_nAniCount )	// 주의!!!.. 카운트 확인할것.. 
			return ((float)m_pAniObject->GetAniLength( nAniIndex )) / g_fRMFps;
	}

	return 0.f;
}

int		CRMObject::PlayAniObject( float fTime )
{
	if( m_pAniObject )
	{
        m_pAniObject // 참조 값을 보장 받기 위해서 매번 직접 대입 합니다
            = (CBsAniObject *)g_BsKernel.GetEngineObjectPtr( m_nObjectIndex );

        BsAssert( m_pAniObject && m_pAniObject->GetAniLength(0) > 0 && "no ani info"); // 비어 있는 BA 를 만든 경우 입니다

		return 1;
	}

	return 0;
}

void	CRMObject::ResetAniObject(void)
{
	if( m_pAniObject )
	{
		m_pAniObject->SetCurrentAni(m_nOldAniIndex, m_fOldFrame, 0);
	}
}

void	CRMObject::UpdateObject(void)
{
	// 실 오브젝트 등록 부분이 필요.
	if( !m_bHideObj && m_nObjectIndex != -1 )
	{
		g_BsKernel.UpdateObject( m_nObjectIndex, m_Cross );

		for( int i = 0 ; i < ePartsMax ; ++i )
		{
			if( m_ObjPack[i].bUse && m_ObjPack[i].nPartsObjIndex != -1 )
			{
				// RefreshAni();
				//g_BsKernel.UpdateObject( m_ObjPack[i].nPartsObjIndex, m_Cross );
			}
		}
	}
	else
	{
		m_bHideObj = FALSE;
	}
}

void	CRMObject::SetRot(int nRot)
{
	SubData.m_nRot += nRot;

	if( SubData.m_nRot < 0 )
		SubData.m_nRot += 1024;
	if( SubData.m_nRot >= 1024 )
		SubData.m_nRot -= 1024;

	m_Cross.RotateYaw( nRot );
}

void	CRMObject::SetAbsoluteRot(int nRot)
{
	D3DXVECTOR3 vecPos = m_Cross.GetPosition();

	m_Cross.Reset();

	m_Cross.m_PosVector = vecPos;

	SubData.m_nRot = 0;
	SetRot( nRot );
}


void	CRMObject::PlayAct( float fTime, int& rnBlend )
{
	// if( m_nAniCount <= 1 ) return;
	mapStrAct::iterator it = m_mapAct.begin();
	mapStrAct::iterator itEnd = m_mapAct.end();
	float fActTime;
	int nAniIndex;
	int nRepeat;
	int nOneTime;
	float fRunTime;

	for( ; it != itEnd ; ++it )
	{
		fActTime = it->second->GetStartTime();
		fRunTime = fTime;	// 현재 시간.

		if( fActTime > fRunTime || fActTime < 0.f )
			continue;

		fRunTime -= fActTime;
		nAniIndex = it->second->GetAniIndex();

		if( nAniIndex <= 0 )			// -1, 0
			continue;

		if( nAniIndex >= m_nAniCount )	// 행동 데이타가 있는지 확인 합니다
			continue;

		float fTime60 = fRunTime * g_fRMFps; // 60 fps (게임 관련 맥스 리소스는 모두 60 fps 입니다)

		/*
		if( strcmp( GetName() , "INP_walk" ) == 0 )
		{
			DebugString("INP_walk : Time:%f ", fTime60 );
		}
		*/

		// float fAniTime =  (float)( m_pAniObject->GetAniLength( nAniIndex ) - 1 );
		float fAniTime =  (float)( it->second->GetFrameSize() );

		
		nRepeat = it->second->GetRepeat();	// 반복. 1이면 필요 없을듯.
		nOneTime = (int)fAniTime;
		fAniTime *= (float)nRepeat;
		
		
		if ( fTime60 >= fAniTime ) 
			continue;

		// 반복..
		
		int nCurTime = (int)fTime60;
		nCurTime = nCurTime % nOneTime;
		fTime60 = (float)nCurTime;
		

		// 시작 값 더하기
		fTime60 += (float)( it->second->GetStartFrame() );

		// ********** Render 할 AniIndex, Frame 설정.
		m_nRenderAniIndex = nAniIndex;
		m_fRenderFrame = fTime60;

		// Debug
		/*
		if( strcmp( GetName() , "Myifee" ) == 0 )
		{
			DebugString("\tAni Index : %d RenderFrame : %f \n", m_nRenderAniIndex, m_fRenderFrame );
		}
		*/
		

		//////////////////////////////////////////////////////////////////////////
		// m_pAniObject->SetCurrentAni( nAniIndex, fTime60, 0);

		m_nOldAniIndex = nAniIndex;
		m_fOldFrame = fTime60;

		++rnBlend;

		if( rnBlend >= 2 )
		{
			// m_pAniObject->RecalcAni();
		}
	}

	if( !m_mapAct.size() )
	{
		m_nRenderAniIndex = 0;
		m_fRenderFrame = 0.f;

		//////////////////////////////////////////////////////////////////////////
		// m_pAniObject->SetCurrentAni( 0, 0.f, 0);
	}
}

void	CRMObject::PlayActEx( int nTick, int& rnBlend )
{
	// if( m_nAniCount <= 1 ) return;
	mapStrAct::iterator it = m_mapAct.begin();
	mapStrAct::iterator itEnd = m_mapAct.end();
	
	int nAniIndex;
	int nOneTime;

	int nActTime;
	int nRunTime;

	for( ; it != itEnd ; ++it )
	{
		nActTime = (int)it->second->GetStartTick();
		nRunTime = nTick;

		if( nActTime > nRunTime || nActTime < 0 )
		{
			continue;
		}

		nRunTime -= nActTime;
		nAniIndex = it->second->GetAniIndex();

		if( nAniIndex <= 0 )			// -1, 0
		{
			continue;
		}

		if( nAniIndex >= m_nAniCount )	// 행동 데이타가 있는지 확인 합니다
		{
			continue;
		}

		int nAniTime =  it->second->GetFrameSize();

		nOneTime = nAniTime;
		nAniTime *= it->second->GetRepeat();

		if ( nRunTime >= nAniTime ) 
		{
			continue;
		}

		// 반복..
		int nCurTime = nRunTime;
		nCurTime = nCurTime % nOneTime;
		nRunTime = nCurTime;

		// 시작 값 더하기
		nRunTime += it->second->GetStartFrame();

		// ********** Render 할 AniIndex, Frame 설정.
		m_nRenderAniIndex = nAniIndex;
		m_fRenderFrame = (float)nRunTime;

		// Debug
		/*
		if( strcmp( GetName() , "Badokk" ) == 0 && m_fRenderFrame > 980.f )
		{
			DebugString("\tAni Index : %d RenderFrame : %f [%d]\n", m_nRenderAniIndex, m_fRenderFrame,nAniTime );
		}
		*/
		

		//////////////////////////////////////////////////////////////////////////
		m_nOldAniIndex = nAniIndex;
		m_fOldFrame = m_fRenderFrame;

		++rnBlend;

		if( rnBlend >= 2 )
		{
			// m_pAniObject->RecalcAni();
		}
	}

	if( !m_mapAct.size() )
	{
		m_nRenderAniIndex = 0;
		m_fRenderFrame = 0.f;

		//////////////////////////////////////////////////////////////////////////
	}
}

void	CRMObject::RenderAct( void )
{
	if( m_pAniObject )
	{
		m_pAniObject->SetCurrentAni( m_nRenderAniIndex, m_fRenderFrame, 0);
	}
}

void	CRMObject::Render(void)
{
	if( m_pAniObject )
	{
        m_pAniObject->SetCurrentAni( m_nRenderAniIndex, m_fRenderFrame, 0);
	}
}

#define FLOAT_EQ(x,v) (((v - FLT_EPSILON) < x) && (x <( v + FLT_EPSILON)))

void	CRMObject::PlayFace( int nTime, int& rnBlend )
{
	mapStrFace::iterator it = m_mapFace.begin();
	mapStrFace::iterator itEnd = m_mapFace.end();

	int nFaceTime;
	int nAniIndex;

	int nRunTime;

	int nAniCount = 0;

	DWORD dwFaceTick;

	m_vecTempFacePool.clear();

	for( ; it != itEnd ; ++it )
	{
		CRMFace* pFace = it->second;

		nFaceTime = pFace->GetStartTick();
		dwFaceTick = pFace->GetStartTick();
		nRunTime = nTime;

		if( nFaceTime > nTime || nFaceTime < 0 )
			continue;

		nRunTime -= nFaceTime;

		nAniIndex = pFace->GetAniIndex();

		if( nAniIndex >= m_nAniCount )	// 표정 데이터 있는지 확인.
			continue;

		int nUseTime = pFace->GetUseFrame(); //  GetUseTime();

		if( nRunTime > nUseTime )		// 사용시간 보다 크면 다음으로.
			continue;

		if( nRunTime < 0 ) // FLT_EPSILON )
			pFace->ResetAniCursor();

		float fAniCur = pFace->GetAniCursor();
/*
#ifdef _USAGE_TOOL_
		char szTemp[200];
		sprintf(szTemp,"Face AniCur %f , fRunTime %f\n", fAniCur, fRunTime );
		DebugString( szTemp );
#endif
*/

		int nBoneIndex;
		nBoneIndex = g_BsKernel.SendMessage( m_nObjectIndex, BS_GET_BONE_INDEX, ( DWORD )"B_Root" );

		if( nAniCount >= 1 )
		{
			CRMFace* pOldFace = *(m_vecTempFacePool.end()-1);

			int nOldTick = (int)pOldFace->GetStartTick();

			if( nOldTick < nFaceTime )
			{
				m_pAniObject->PopBlend();
				m_pAniObject->SetCurrentAni( pOldFace->GetAniIndex(), 1.f, nBoneIndex );
				m_pAniObject->BlendAni( nAniIndex, 1.f, fAniCur , nBoneIndex );
			}
			else if( nOldTick == nFaceTime ) // 동일한 위치.
			{
				m_pAniObject->BlendAni( nAniIndex, 1.f, fAniCur , nBoneIndex );
			}
			else
			{
				m_pAniObject->PopBlend();
				m_pAniObject->SetCurrentAni( nAniIndex, 1.f, nBoneIndex );
				m_pAniObject->BlendAni( pOldFace->GetAniIndex(), 1.f, pOldFace->GetAniCursor() , nBoneIndex );
			}
		}
		else
		{
            m_pAniObject->BlendAni( nAniIndex, 1.f, fAniCur , nBoneIndex );
		}

		pFace->IncAniCursor();

		m_vecTempFacePool.push_back( pFace );
		
		++rnBlend;

		++nAniCount;
	}
}

void	CRMObject::PlayLip( int nTime, int& rnBlend )
{
/*
	if( m_pAniObject->GetAniLength(0)!=21 )	// 임시??
		return;
*/

	mapStrLip::iterator it = m_mapLip.begin();
	mapStrLip::iterator itEnd = m_mapLip.end();

	int nLipTime;
	int nRunTime;
	CRMLip*	pLip;
	int j;
	int nDbgY=40; //500+60;

	for( ; it != itEnd ; ++it )
	{
		pLip = it->second;
		nLipTime = pLip->GetStartTick(); // GetStartTime();
		nRunTime = nTime;

		if( nRunTime < nLipTime || nLipTime < 0 )
			continue;

		nRunTime -= nLipTime;
		CBsFileLTF& rLTFPack = pLip->GetLTFObject();

		if( rLTFPack.m_pData == NULL )
			continue;

		float fRunTime;
#ifdef _USAGE_TOOL_
		fRunTime = (float)nRunTime / 40.f;
#else
		if( pLip->m_nEvent == 0 )
		{
			fRunTime = 0.0f;
		}
		else
		{
			LARGE_INTEGER liCurTime, liFrequency;

			liCurTime = CBsRealMovie::GetLipTime();
			QueryPerformanceFrequency( &liFrequency );
			fRunTime = ( ( float )( liCurTime.QuadPart - pLip->m_liStartTime.QuadPart ) ) / liFrequency.QuadPart;
			if( fRunTime < 0.0f )
			{
				fRunTime = 0.0f;
			}
		}
#endif
		if( fRunTime > rLTFPack.m_pData[ rLTFPack.m_nMax - 1 ].m_fEnd )
			continue;	// 전체 애니 시간과 비교 합니다

		if( pLip->m_nEvent == 0 )	// 이벤트가 한번만 통지 되도록 만들어 주는 코드
		{
			pLip->m_nEvent = 1;
#ifndef _USAGE_TOOL_
			QueryPerformanceCounter( &pLip->m_liStartTime );
#endif
		}

#ifdef _USAGE_TOOL_
		/*
		static char cStr[256];
		sprintf( cStr, "Time : %d  Use Lip : [%s]\n", nTime, pLip->GetName() );
		DebugString( cStr );
		*/
#endif

		int nLTFId=0, nLTFIdOld=0;
		int nBALMId=0, nBALMIdOld=0;
		float fWeight=0.f, fWeightOld=0.f;

		for ( j=0 ; j < rLTFPack.m_nMax; j++) // 매칭 데이타를 통해 값을 만듭니다
		{ 
			if (   fRunTime >= rLTFPack.m_pData[j].m_fBegin 
				&& fRunTime <= rLTFPack.m_pData[j].m_fEnd  ) // 시간 정보가 일치하는지 검사합니다
			{ 
				{
					nLTFId	= rLTFPack.m_pData[j].m_nID; // 해당 시간 j 의 id 를 가져 옵니다
					nBALMId	= st_BALM.m_nAniBuf[nLTFId];
				}

				if( j )	// j > 0 
				{
					nLTFIdOld  = rLTFPack.m_pData[j-1].m_nID;
					nBALMIdOld = st_BALM.m_nAniBuf[nLTFIdOld];
					fWeightOld = st_BALM.m_fWeightBuf[nLTFIdOld];
				}

				int ck=0;
				float fBALMTime = rLTFPack.m_pData[j].m_fEnd - rLTFPack.m_pData[j].m_fBegin;
				const float cfEnd=0.20f; 

/*
				if( j == rLTFPack.m_nMax-1 && fBALMTime > cfEnd) 
				{
					//nModeDebug_LS=2;
					fBALMTime=cfEnd; // 마지막인 경우에는 입을 바로 닫습니다
					ck=1;
				}
*/
				const float cfSilent=0.30f;
				if( nLTFId==40 && fBALMTime > 0.7f ) 
				{
					//nModeDebug_LS=5;
					fBALMTime=cfSilent; // 무음인 경우에도 입을 조정 합니다 
					ck=1;
				}

				if( fBALMTime <= 0 ) 
					fBALMTime=0.001f; // 0 으로 나누어 지지 않게 합니다

				fWeight = ( fRunTime - rLTFPack.m_pData[j].m_fBegin ) / fBALMTime; // 0..1
//				fWeightOld = 1.f - fWeight;

				if (ck) // <1> 
				{ 
					fWeight=fWeight*fWeight;
				}
/*
				else
				{
					fWeight=st_BALM.m_fWeightBuf[nLTFId] * fWeight;
				}
*/
				else	// <2>
				{ 
					fWeight = -2*fWeight*fWeight*fWeight + 3*fWeight*fWeight;
					if (fWeight < 0.0001f) 
						fWeight = 0;
				}

				if( fWeight > 1.f ) 
					fWeight = 1.f;
				else if( fWeight < 0.f )
					fWeight = 0.f;

				// fWeight *= st_BALM.m_fWeightBuf[nLTFId]; // 매칭에 있는 가중치를 적용 시킵니다
				// 캐릭터 마다 추가 가중치를 줄수 있는, 코드 위치 입니다
				break; // 반드시 실행 될 것입니다
			}
		}

		/*
		if (bDrawUITextDebug_LS) {
			DrawUITextDebug_LS(nLTFId, nBALMId, nDbgY); // 디버깅  
			nDbgY+=24;
		}
		*/

		// Debug_LS(nLTFId, nBALMId, nDbgY); // 디버깅  
		nDbgY+=24;

		// 본 찾던것을 -1 값일때만 다시 한번 찾는 것으로 변경.
		// int nBoneIndex  = m_pAniObject->LSFindBoneIndex("B_M_Root");

/*		m_pAniObject->FrontInsertBlendAni( 0, (float)nBALMId, fWeight, nBoneIndex );
//		m_pAniObject->FrontInsertBlendAni( 0, (float)nBALMIdOld, fWeightOld, nBoneIndex );	// 1.0 입과 얼굴과의 가중치, 표정 있을 때 
		m_pAniObject->FrontInsertBlendAni( 0, (float)nBALMIdOld, 1.0f, nBoneIndex );	// 1.0 입과 얼굴과의 가중치, 표정 있을 때 */

		if( m_nBoneIndex != -1 )
		{
            m_pAniObject->BlendAni( 0, (float)nBALMIdOld, 1.0f, m_nBoneIndex );
            m_pAniObject->BlendAni( 0, (float)nBALMId, fWeight, m_nBoneIndex );
		}
		/*
		else
		{
			m_nBoneIndex = m_pAniObject->LSFindBoneIndex("B_M_Root");
			if( m_nBoneIndex == -1 )
			{
				BsAssert( 0 && "[PlayLip] Bone index == -1");
			}
		}
		*/


		//char cStr[256];
		//sprintf( cStr, "BALM Weight %d, %d, %f, %f\n", nBALMId, nBALMIdOld, fWeight, fWeightOld );
		//DebugString( cStr );


/*		m_pAniObject->SetCurrentAni(0, (float)nBALMIdOld,          nBoneIndex); // 여기에도 가중치를 주는 코드가 필요 합니다
		m_pAniObject->BlendAni     (0, (float)nBALMId,     fWeight, nBoneIndex);*/

		++rnBlend;
	}
}

int		CRMObject::GetActNameList( vecString& rvecList )
{
	int nCnt = 0;
	itStrAct it;
	itStrAct itEnd = m_mapAct.end();
	char szTemp[130];

	for( it = m_mapAct.begin() ; it != itEnd ; ++it )
	{
		if( it->second->GetStartTime() < 0.f )
		{
            sprintf( szTemp, "%s::%s", it->second->GetName() , m_szName );
            rvecList.push_back( szTemp ); //it->second->GetName() );
            ++nCnt;
		}
	}

	return nCnt;
}

int		CRMObject::GetLipNameList( vecString& rvecList )
{
	int nCnt = 0;
	itStrLip it;
	itStrLip itEnd = m_mapLip.end();
	char szTemp[130];

	for( it = m_mapLip.begin() ; it != itEnd ; ++it )
	{
		if( it->second->GetStartTime() < 0.f && it->second->HasWave() )
		{
			sprintf( szTemp, "%s::%s", it->second->GetName() , m_szName );
			rvecList.push_back( szTemp );
			++nCnt;
		}
	}

	return nCnt;
}

int		CRMObject::GetFaceNameList( vecString& rvecList )
{
	int nCnt = 0;
	itStrFace it;
	itStrFace itEnd = m_mapFace.end();
	char szTemp[130];

	for( it = m_mapFace.begin() ; it != itEnd ; ++it )
	{
		if( it->second->GetStartTime() < 0.f )
		{
            sprintf( szTemp, "%s::%s", it->second->GetName() , m_szName );
            rvecList.push_back( szTemp );
            ++nCnt;
		}
	}

	return nCnt;
}

int		CRMObject::GetBoneNameList( vecString& rvecList )
{
	int nCnt = 0;
	itStrBone it;
	itStrBone itEnd = m_mapBone.end();
	char szTemp[130];

	for( it = m_mapBone.begin() ; it != itEnd ; ++it )
	{
		if( it->second->GetStartTime() < 0.f )
		{
			sprintf( szTemp, "%s::%s", it->second->GetName() , m_szName );
			rvecList.push_back( szTemp );
			++nCnt;
		}
	}

	return nCnt;
}

int		CRMObject::GetOriginalBoneNameList( vecString& rvecList )
{
	int nCnt = 0;
	itStrBone it;
	itStrBone itEnd = m_mapBone.end();
	// char szTemp[130];

	for( it = m_mapBone.begin() ; it != itEnd ; ++it )
	{
		if( it->second->GetStartTime() < 0.f )
		{
			// sprintf( szTemp, "%s", it->second->GetName() );
			rvecList.push_back( it->second->GetName() );
			++nCnt;
		}
	}

	return nCnt;
}

/*
 1) NameSize,	ObjName
 2) SkinSize,	SkinName
 3) BaSize,		BaName
 4) m_Cross,
 5) Act...
 6) Lip...
 7) Face...
 */
void	CRMObject::Save(BStream *pStream)
{
	int nSize;
	int nLen = strlen( m_szName );
	int nSkLen = strlen( m_szSKFileName );
	int nBaLen = strlen( m_szBAFileName );

	pStream->Write( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );
	if( nLen )
        pStream->Write( m_szName, nLen , 0 );
	pStream->Write( &nSkLen, sizeof(int), ENDIAN_FOUR_BYTE );
	if( nSkLen )
        pStream->Write( m_szSKFileName, nSkLen, 0 );
	pStream->Write( &nBaLen, sizeof(int), ENDIAN_FOUR_BYTE );
	if( nBaLen )
        pStream->Write( m_szBAFileName, nBaLen, 0 );

	// int nSize = ( sizeof(D3DXVECTOR3) * 4 ) + sizeof(int);

	pStream->Write( &m_Cross.m_XVector , sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
	pStream->Write( &m_Cross.m_YVector , sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
	pStream->Write( &m_Cross.m_ZVector , sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
	pStream->Write( &m_Cross.m_PosVector , sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );

	nSize = sizeof( SubData ); // eObjectSize;
	pStream->Write( &nSize, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Write( &SubData, nSize, ENDIAN_FOUR_BYTE );

	/*
	pStream->Write( &m_nRot, sizeof(int), ENDIAN_FOUR_BYTE );
	int nExDataSize = 0;
	pStream->Write( &nExDataSize, sizeof(int), ENDIAN_FOUR_BYTE );
	*/

	// Act 저장.
	{
		itStrAct it;
		itStrAct itEnd = m_mapAct.end();

		nSize = m_mapAct.size();
		pStream->Write( &nSize, sizeof(int), ENDIAN_FOUR_BYTE );

		for( it = m_mapAct.begin() ; it != itEnd ; ++it )
		{
			it->second->Save( pStream );
		}
	}

	// Lip 저장.
	{
		itStrLip it;
		itStrLip itEnd = m_mapLip.end();

		nSize = m_mapLip.size();
		pStream->Write( &nSize, sizeof(int), ENDIAN_FOUR_BYTE );

		for( it = m_mapLip.begin() ; it != itEnd ; ++it )
		{
			it->second->Save( pStream );
		}
	}

	// Face 저장.
	{
		itStrFace it;
		itStrFace itEnd = m_mapFace.end();

		nSize = m_mapFace.size();
		pStream->Write( &nSize, sizeof(int), ENDIAN_FOUR_BYTE );

		for( it = m_mapFace.begin() ; it != itEnd ; ++it )
		{
			it->second->Save( pStream );
		}
	}
}

void	CRMObject::Load(BStream *pStream)
{
	int nCnt;
	int nSize;

	char szTempName[eFileNameLength];
	int nLen;

	/*
	pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );
	if( nLen )
		pStream->Read( m_szName, nLen, 0 );
	*/

	pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );
	if( nLen )
	{
		memset( m_szSKFileName, 0, eFileNameLength );
		pStream->Read( m_szSKFileName, nLen, 0 );
	}

	pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );
	if( nLen )
	{
		memset( m_szBAFileName, 0, eFileNameLength );
		pStream->Read( m_szBAFileName, nLen, 0 );
	}

	pStream->Read( &m_Cross.m_XVector , sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
	pStream->Read( &m_Cross.m_YVector , sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
	pStream->Read( &m_Cross.m_ZVector , sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
	pStream->Read( &m_Cross.m_PosVector , sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );

	pStream->Read( &nSize, sizeof(int), ENDIAN_FOUR_BYTE );
	
	if( nSize )
	{
		pStream->Read( &SubData, nSize, ENDIAN_FOUR_BYTE );
	}

	// pStream->Read( &m_nRot, sizeof(int), ENDIAN_FOUR_BYTE );

	pStream->Read( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	// Act Load
	for( int i = 0 ; i < nCnt ; ++i )
	{
		pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );

		if( nLen )
		{
			CRMAct* pAct;
			memset( szTempName, 0, eFileNameLength);
			pStream->Read( szTempName, nLen , 0 );
            pAct = InsertAct( szTempName );
			BsAssert(pAct && "Failed to insert an act");// aleksger: prefix bug 427: Possible failure to insert.
			pAct->Load( pStream );
		}
	}

	pStream->Read( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	// Lip Load
	for( int i = 0 ; i < nCnt ; ++i )
	{
		pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );

		if( nLen )
		{
			CRMLip* pLip;
			memset( szTempName, 0, eFileNameLength);
			pStream->Read( szTempName, nLen , 0 );
			pLip = InsertLip( szTempName );
			BsAssert(pLip && "Failed to insert an lip");// aleksger: prefix bug 428: Possible failure to insert.
			pLip->Load( pStream );
		}
	}

	pStream->Read( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	// Face Load
	for( int i = 0 ; i < nCnt ; ++i )
	{
		pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );

		if( nLen )
		{
			CRMFace* pFace;
			memset( szTempName, 0, eFileNameLength);
			pStream->Read( szTempName, nLen , 0 );
			pFace = InsertFace( szTempName );
			BsAssert(pFace && "Failed to insert an face");// aleksger: prefix bug 429: Possible failure to insert.

			pFace->Load( pStream );
		}
	}
}

void	CRMObject::SaveEx(BStream *pStream)
{
	int nLen;

	for( int i = 0 ; i < eLinkedObjMax ; ++i )
	{
		nLen = strlen( m_szLinkedObjName[i] );
		pStream->Write( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );

		if( nLen )
			pStream->Write( m_szLinkedObjName[i], nLen, 0 );
	}
}

void	CRMObject::LoadEx(BStream *pStream)
{
	int nLen;

	for( int i = 0 ; i < eLinkedObjMax ; ++i )
	{
		pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );

		memset( m_szLinkedObjName[i], 0, eFileNameLength );

		if( nLen )
			pStream->Read( m_szLinkedObjName[i], nLen, 0 );
	}
}

void	CRMObject::SaveEx2(BStream *pStream)
{
	int i;
	int nLen;
	int nExCode = 3;
	pStream->Write( &nExCode, sizeof(int), ENDIAN_FOUR_BYTE );

	CBsRealMovie* pRM = g_BsKernel.GetRealMovie();

	for( i = 0 ; i < eLinkedObjMax ; ++i )
	{
		nLen = strlen( m_szBoneName[i] );
		pStream->Write( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );

		if( nLen )
			pStream->Write( m_szBoneName[i], nLen, 0 );
	}

	// 확장 코드.
	// Skin Path
	nLen = strlen( m_szSkinSubPath );
	pStream->Write( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );

	if( nLen )
		pStream->Write( m_szSkinSubPath, nLen, 0 );

	// BA Path
	nLen = strlen( m_szBAFileSubPath );
	pStream->Write( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );

	if( nLen )
		pStream->Write( m_szBAFileSubPath, nLen, 0 );

	// Parts
	for( i = 0 ; i < ePartsMax ; ++i )
	{
		pStream->Write( &m_ObjPack[i].bUse , sizeof(int), ENDIAN_FOUR_BYTE );

		nLen = strlen( m_ObjPack[i].m_szSubPath );
		pStream->Write( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );

		if( nLen )
			pStream->Write( &m_ObjPack[i].m_szSubPath, nLen, 0 );

		nLen = strlen( m_ObjPack[i].m_szPartsName );
		pStream->Write( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );

		if( nLen )
			pStream->Write( &m_ObjPack[i].m_szPartsName, nLen, 0 );
	}

	// LipEx
	nLen = (int)m_mapLip.size();	// Lip 갯수 저장.
	pStream->Write( &nLen , sizeof(int), ENDIAN_FOUR_BYTE );

	mapStrLip::iterator itLip = m_mapLip.begin();
	mapStrLip::iterator itLipEnd = m_mapLip.end();

	for( ; itLip != itLipEnd ; ++itLip )
	{
		itLip->second->SaveEx( pStream );
	}
}

void	CRMObject::LoadEx2(BStream *pStream)
{
	int nLen;
	int nExCode;

	pStream->Read( &nExCode, sizeof(int), ENDIAN_FOUR_BYTE );

	for( int k = 0 ; k < nExCode ; ++k )
	{
		if( k == 0 )
		{
			for( int i = 0 ; i < eLinkedObjMax ; ++i )
			{
				pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );

				memset( m_szBoneName[i], 0, eFileNameLength );

				if( nLen )
					pStream->Read( m_szBoneName[i], nLen, 0 );
			}
			
		} // i == ExCode
		else if( k == 1 )	// 두번째 파트
		{
			pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );
			memset( m_szSkinSubPath, 0, eFileNameLength );

			if( nLen )
				pStream->Read( m_szSkinSubPath, nLen, 0 );

			pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );
			memset( m_szBAFileSubPath, 0, eFileNameLength );

			if( nLen )
				pStream->Read( m_szBAFileSubPath, nLen, 0 );

			for( int i = 0 ; i < ePartsMax ; ++i )
			{
				pStream->Read( &m_ObjPack[i].bUse , sizeof(int), ENDIAN_FOUR_BYTE );

				pStream->Read( &nLen , sizeof(int), ENDIAN_FOUR_BYTE );
				memset( m_ObjPack[i].m_szSubPath, 0, eFileNameLength );

				if( nLen )
					pStream->Read( m_ObjPack[i].m_szSubPath, nLen, 0 );

				pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );
				memset( m_ObjPack[i].m_szPartsName, 0, eFileNameLength );

				if( nLen )
					pStream->Read( m_ObjPack[i].m_szPartsName, nLen, 0 );
			}
		}
		else if( k == 2 )	// 세번째 파트
		{
			pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );

			if( nLen != (int)m_mapLip.size() )
			{
#ifdef _USAGE_TOOL_
				MessageBox( NULL, "Lip 데이터 개수가 맞지 않습니다.","Load Error", MB_OK );
				return;
#endif
			}

			mapStrLip::iterator itLip = m_mapLip.begin();
			mapStrLip::iterator itLipEnd = m_mapLip.end();

			for( ; itLip != itLipEnd ; ++itLip )
			{
				itLip->second->LoadEx( pStream );
			}
		}
	}
}

void	CRMObject::SaveBoneEx(BStream *pStream)
{
	itStrBone it;
	itStrBone itEnd = m_mapBone.end();

	int nSize = m_mapBone.size();
	pStream->Write( &nSize, sizeof(int), ENDIAN_FOUR_BYTE );

	for( it = m_mapBone.begin() ; it != itEnd ; ++it )
	{
		it->second->Save( pStream );
		it->second->SaveEx( pStream );
	}
}

void	CRMObject::LoadBoneEx(BStream *pStream)
{
	int nCnt;
	int nLen;
	char szTempName[ eFileNameLength ];

	pStream->Read( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	// Bone Load
	for( int i = 0 ; i < nCnt ; ++i )
	{
		pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );

		if( nLen )
		{
			CRMBone* pBone;
			memset( szTempName, 0, eFileNameLength);
			pStream->Read( szTempName, nLen , 0 );
			pBone = InsertBone( szTempName );
			BsAssert(pBone && "Failed to insert an bone");// aleksger: prefix bug 433: Possible failure to insert.

			pBone->Load( pStream );
			pBone->LoadEx( pStream );
		}
	}
}


void	CRMObject::ToggleAlphaMode(void)
{
	m_nAlphaMode = !m_nAlphaMode;

	float fAlphaVal;
	int k;
	int nSbCount;

	// True 면 이미 Alpha 먹힌 놈.
	if( g_BsKernel.SendMessage( m_nObjectIndex , BS_GET_SUBMESH_ALPHABLENDENABLE ) )
	{
		fAlphaVal = 1.f;

		nSbCount = g_BsKernel.SendMessage( m_nObjectIndex , BS_GET_SUBMESH_COUNT );

		for( k = 0 ; k < nSbCount ; ++k )
		{
			g_BsKernel.SendMessage( m_nObjectIndex , BS_SET_SUBMESH_ALPHABLENDENABLE, k, false );
			g_BsKernel.SendMessage( m_nObjectIndex , BS_SET_SUBMESH_ALPHA, k , (DWORD)&fAlphaVal );
		}
	}
	else
	{
		fAlphaVal = 0.25f;

		nSbCount = g_BsKernel.SendMessage( m_nObjectIndex , BS_GET_SUBMESH_COUNT );

		for( k = 0 ; k < nSbCount ; ++k )
		{
			g_BsKernel.SendMessage( m_nObjectIndex , BS_SET_SUBMESH_ALPHABLENDENABLE, k, true );
			g_BsKernel.SendMessage( m_nObjectIndex , BS_SET_SUBMESH_ALPHA, k , (DWORD)&fAlphaVal );
		}
	}
}

void	CRMObject::ObjectAlphaMode(float fAlphaValue,DWORD dwOp,DWORD dwSrc,DWORD dwDest)
{
	int k;
	int nSbCount;

	nSbCount = g_BsKernel.SendMessage( m_nObjectIndex , BS_GET_SUBMESH_COUNT );

	for( k = 0 ; k < nSbCount ; ++k )
	{
		g_BsKernel.SendMessage( m_nObjectIndex , BS_SET_SUBMESH_ALPHABLENDENABLE, k, TRUE );
		g_BsKernel.SendMessage( m_nObjectIndex , BS_SET_SUBMESH_ALPHA, k , (DWORD)&fAlphaValue );

		g_BsKernel.SendMessage( m_nObjectIndex , BS_SET_SUBMESH_BLENDOP, k , (DWORD)dwOp );
		g_BsKernel.SendMessage( m_nObjectIndex , BS_SET_SUBMESH_SRCBLEND, k , (DWORD)dwSrc );
		g_BsKernel.SendMessage( m_nObjectIndex , BS_SET_SUBMESH_DESTBLEND, k , (DWORD)dwDest );
	}

	m_bUseObjectAlphaMode = TRUE;
}

void	CRMObject::DisableObjectAlphaMode(void)
{
	int k;
	int nSbCount;

	if( m_bUseObjectAlphaMode == TRUE )
	{
		nSbCount = g_BsKernel.SendMessage( m_nObjectIndex , BS_GET_SUBMESH_COUNT );

		for( k = 0 ; k < nSbCount ; ++k )
		{
			g_BsKernel.SendMessage( m_nObjectIndex , BS_SET_SUBMESH_ALPHABLENDENABLE, k, FALSE );
		}

		m_bUseObjectAlphaMode = FALSE;
	}
}

BOOL	CRMObject::PtInfRect(D3DXVECTOR4& rfRc)
{
	float *fx = &m_Cross.m_PosVector.x;
	float *fz = &m_Cross.m_PosVector.z;

	if( *fx >= rfRc.x && *fx <= rfRc.y && *fz >= rfRc.z	&& *fz <= rfRc.w )
	{
		return TRUE;
	}

	return FALSE;
}

#ifdef _USAGE_TOOL_

int	CRMObject::ChangeTimeOfAllData(void)
{

	// ChangeFPS
	itStrAct itAct = m_mapAct.begin();
	itStrAct itActEnd = m_mapAct.end();

	for( ; itAct != itActEnd ; ++itAct )
	{
		itAct->second->ChangeFPS();
	}

	itStrLip itLip = m_mapLip.begin();
	itStrLip itLipEnd = m_mapLip.end();

	for( ; itLip != itLipEnd ; ++itLip )
	{
		itLip->second->ChangeFPS();
	}

	itStrFace itFace = m_mapFace.begin();
	itStrFace itFaceEnd = m_mapFace.end();

	for( ; itFace != itFaceEnd ; ++itFace )
	{
		itFace->second->ChangeFPS();
	}

	itStrBone itBone = m_mapBone.begin();
	itStrBone itBoneEnd = m_mapBone.end();

	for( ; itBone != itBoneEnd ; ++itBone )
	{
		itBone->second->ChangeFPS();
	}

	return 0;
}

#endif

BOOL	CRMObject::ChangeDataFolder(VOID)
{
	char szTemp[64];

	if( SubData.m_nDataType == 0 ) // Old Type
	{	
		strcpy( szTemp, m_szSkinSubPath );
		strcpy( m_szSkinSubPath , "RealMovie\\");
		strcat( m_szSkinSubPath , szTemp );

		strcpy( szTemp, m_szBAFileSubPath );
		strcpy( m_szBAFileSubPath , "RealMovie\\");
		strcat( m_szBAFileSubPath , szTemp );
		SubData.m_nDataType = 1;
	}

	itStrLip itLip = m_mapLip.begin();
	itStrLip itLipEnd = m_mapLip.end();

	for( ; itLip != itLipEnd ; ++itLip )
	{
		itLip->second->ChangeDataFolder();
	}

	for( int i = 0 ; i < ePartsMax ; ++i )
	{
        if( m_ObjPack[i].m_szPartsName[0] != NULL ||
			m_ObjPack[i].m_szSubPath[0] != NULL )
		{
			strcpy( szTemp, m_ObjPack[i].m_szSubPath );
			strcpy( m_ObjPack[i].m_szSubPath , "RealMovie\\");
			strcat( m_ObjPack[i].m_szSubPath , szTemp );
		}
	}

	return TRUE;
}


void	CRMObject::DropWeapon(void)
{
	/*
	WeaponMat = *( D3DXMATRIX * )g_BsKernel.SendMessage( m_nEngineIndex, BS_UNLINKOBJECT, m_WeaponList[ m_nCurrentWeaponIndex ].nObjectIndex );
	memcpy( &Cross.m_XVector, &WeaponMat._11, sizeof( D3DXVECTOR3 ) );
	memcpy( &Cross.m_YVector, &WeaponMat._21, sizeof( D3DXVECTOR3 ) );
	memcpy( &Cross.m_ZVector, &WeaponMat._31, sizeof( D3DXVECTOR3 ) );
	memcpy( &Cross.m_PosVector, &WeaponMat._41, sizeof( D3DXVECTOR3 ) );
	*/

	D3DXMATRIX* pWMat;

	pWMat = (D3DXMATRIX*)g_BsKernel.SendMessage( m_nObjectIndex, BS_GET_OBJECT_MAT );
// [PREFIX:beginmodify] 2006/2/25 junyash PS#5802 dereferencing NULL pointer
	BsAssert(pWMat);
// [PREFIX:endmodify] junyash
	// g_BsKernel.GetEngineObjectPtr( ) // GetObjectMatrix
	memcpy( &m_Cross.m_XVector, &pWMat->_11, sizeof( D3DXVECTOR3 ) );
	memcpy( &m_Cross.m_YVector, &pWMat->_21, sizeof( D3DXVECTOR3 ) );
	memcpy( &m_Cross.m_ZVector, &pWMat->_31, sizeof( D3DXVECTOR3 ) );
	memcpy( &m_Cross.m_PosVector, &pWMat->_41, sizeof( D3DXVECTOR3 ) );

	return;
}

// D3DXMATRIX* CRMObject::GetAniObjMatrix(void)
D3DXVECTOR3*	CRMObject::GetAniObjPos(void)
{
#ifdef _DEBUG
	if( m_nObjectIndex != -1 )
	{
		if( m_pAniObject )
		{
			D3DXVECTOR3* pRootPos = &m_Cross.m_PosVector;
			D3DXVECTOR4 vecPos;
			m_pAniObject->Get_pAni()->GetRootBonePos(m_nOldAniIndex,m_fOldFrame, &vecPos);

			m_vecSndPos.x = pRootPos->x - vecPos.x;
			m_vecSndPos.y = pRootPos->y;					// + m_vecSndPos.y;
			m_vecSndPos.z = pRootPos->z - vecPos.z;

			return &m_vecSndPos;
		}
		else
		{
			D3DXMATRIX* pMat
				= (D3DXMATRIX*)g_BsKernel.SendMessage( m_nObjectIndex, BS_GET_OBJECT_MAT );
			m_vecSndPos.x = pMat->_41;
			m_vecSndPos.y = pMat->_42;
			m_vecSndPos.z = pMat->_43;

			return &m_vecSndPos;
		}
	}
	else
	{
		m_vecSndPos.x = 0.f;
		m_vecSndPos.y = 0.f;
		m_vecSndPos.z = 0.f;
	}

	return &m_vecSndPos;
#else
	if( m_pAniObject )
	{
		D3DXVECTOR3* pRootPos = &m_Cross.m_PosVector;
		D3DXVECTOR4 vecPos;
		m_pAniObject->Get_pAni()->GetRootBonePos(m_nOldAniIndex,m_fOldFrame, &vecPos);

		m_vecSndPos.x = pRootPos->x - vecPos.x;
		m_vecSndPos.y = pRootPos->y; // + m_vecSndPos.y;
		m_vecSndPos.z = pRootPos->z - vecPos.z;
		
		return &m_vecSndPos;
	}
	else
	{
		if( m_nObjectIndex != -1 )
		{
			D3DXMATRIX* pMat
				= (D3DXMATRIX*)g_BsKernel.SendMessage( m_nObjectIndex, BS_GET_OBJECT_MAT );
			m_vecSndPos.x = pMat->_41;
			m_vecSndPos.y = pMat->_42;
			m_vecSndPos.z = pMat->_43;
			return &m_vecSndPos;
		}
		else
		{
			m_vecSndPos.x = 0.f;
			m_vecSndPos.y = 0.f;
			m_vecSndPos.z = 0.f;
			return &m_vecSndPos;
		}
	}
#endif
}

void	CRMObject::SetSkinIndexForWeapon(int nSIdx)
{	
	if ( m_nSkinIndex == nSIdx )
		return;

	if ( nSIdx != -1 )
		g_BsKernel.AddSkinRef( nSIdx );

	if ( m_nSkinIndex != -1 )
		g_BsKernel.ReleaseSkin( m_nSkinIndex );

	m_nSkinIndex = nSIdx;
}

int		CRMObject::GetSkinIndexForWeapon()
{
	return m_nSkinIndex;
}

void	CRMObject::LinkObject(CRMObject *pLinkedObject,char* szBoneName)
{
	g_BsKernel.SendMessage( 
		m_nObjectIndex, 
		BS_LINKOBJECT_NAME, 
		(DWORD)szBoneName,
		pLinkedObject->m_nObjectIndex );
}

void	CRMObject::LinkFXObject(int nFXIndex,char* szBoneName)
{
	g_BsKernel.SendMessage( 
		m_nObjectIndex, 
		BS_LINKOBJECT_NAME, 
		(DWORD)szBoneName,
		nFXIndex );
}

void	CRMObject::UnlinkFXObject(int nFXIndex)
{
	g_BsKernel.SendMessage( 
		m_nObjectIndex, 
		BS_UNLINKOBJECT,
		nFXIndex );
}

void CRMObject::AddLipTime( LARGE_INTEGER liAddTime )
{
	mapStrLip::iterator it = m_mapLip.begin();
	mapStrLip::iterator itEnd = m_mapLip.end();
	CRMLip*	pLip;

	for( ; it != itEnd ; ++it )
	{
		pLip = it->second;
		pLip->m_liStartTime.QuadPart += liAddTime.QuadPart;
	}
}

