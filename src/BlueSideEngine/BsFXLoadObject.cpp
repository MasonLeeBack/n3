#include "StdAfx.h"
#include "BsKernel.h"
#include "BsFXObject.h"
#include "BsFXElement.h"
#include "BsFXParticleElement.h"
#include "BsFXMeshElement.h"
#include "BsFXPLElement.h"
#include "bstreamext.h"
#include "BsFXLoadObject.h"
#include "BsFileIO.h"

extern bool IsRenderThread();

#include "BsDebugBreak.h"

#if defined(USE_TEMPLATEDBG)

static Debug::DebugInfo s_templateDbgInfo[] =
{
	{ "d:\\data\\FX\\event_YUABE02_N.bfx", Debug::OnAll, Debug::OnAddRef },
	{ "", Debug::OnNothing, Debug::OnNothing }
};

#endif


float CBsFXTemplate::m_fCurLoadVersion = 0.f;
CBsFXTemplate::CBsFXTemplate()
: m_nRefCount(1)
{
	m_nLoadSize = 0;
}

CBsFXTemplate::~CBsFXTemplate()
{
#if defined(USE_TEMPLATEDBG)
	char const * szFileName = GetFileName();
	Debug::TriggerEvent( s_templateDbgInfo, szFileName, m_nRefCount, Debug::OnDelete );
#endif

	SAFE_DELETE_PVEC( m_pVecElement );
}

BOOL CBsFXTemplate::Load( const char *szFileName )
{
	DWORD dwFileSize;
	VOID *pData;

#ifdef ENABLE_MEM_CHECKER
	char cStr[256];
	sprintf( cStr, "Load FX : %s", szFileName );
	g_BsMemChecker.Start( cStr );
#endif

#ifdef ENABLE_RES_CHECKER
	g_BsResChecker.AddResList( (char*)szFileName );
#endif

	if( FAILED(CBsFileIO::LoadFile( szFileName, &pData, &dwFileSize ) ) )
	{
		CBsConsole::GetInstance().AddFormatString("%s File Not Found", szFileName);
#ifdef ENABLE_MEM_CHECKER
		g_BsMemChecker.End();
#endif
		return FALSE;
	}

	BMemoryStream Stream(pData, dwFileSize);
	BOOL bOk = Load( &Stream );
	if( bOk )
	{
		m_szFileName = szFileName;

#if defined(USE_TEMPLATEDBG)
	char const * szFileName = GetFileName();
	Debug::TriggerEvent( s_templateDbgInfo, szFileName, m_nRefCount, Debug::OnLoad );
#endif
	}

	CBsFileIO::FreeBuffer(pData);

#ifdef ENABLE_MEM_CHECKER
	g_BsMemChecker.End();
#endif

	return bOk;
}

BOOL CBsFXTemplate::Load( BStream *pStream )
{
	HEADER Header;
	pStream->Read( &Header, sizeof(HEADER) );
#ifdef _XBOX
	char pBuf[4];
	memcpy( pBuf, &Header.fVersion, sizeof(float) );
	BStream::SwapByte( pBuf, pBuf + 3 );
	BStream::SwapByte( pBuf + 1, pBuf + 2 );

	memcpy( &Header.fVersion, pBuf, sizeof(float) );
#endif // _XBOX

	m_fCurLoadVersion = Header.fVersion;

	pStream->Read( &m_dwTotalFrame, sizeof(DWORD), ENDIAN_FOUR_BYTE );

	CBsFXElement::ELEMENT_TYPE Type;
	DWORD dwCount;
	pStream->Read( &dwCount, sizeof(DWORD), ENDIAN_FOUR_BYTE );
	for( DWORD i=0; i<dwCount; i++ ) {
		CBsFXElement *pElement = NULL;
		pStream->Read( &Type, sizeof(CBsFXElement::ELEMENT_TYPE), ENDIAN_FOUR_BYTE );
		switch( Type ) {
			case CBsFXElement::PARTICLE:	pElement = new CBsFXParticleElement;	break;
			case CBsFXElement::MESH:		pElement = new CBsFXMeshElement;		break;
			case CBsFXElement::POINT_LIGHT:	pElement = new CBsFXPLElement;			break;
			default:	
				return FALSE;
		}

		BOOL bRet = pElement->Load( pStream );
		if( bRet == FALSE )
		{
			delete pElement;
			return FALSE;
		}

		m_pVecElement.push_back( pElement );
	}

	return TRUE;
}

void CBsFXTemplate::AddRef()
{
	BsAssert( !IsRenderThread() );

	++m_nRefCount;

#if defined(USE_TEMPLATEDBG)
	char const * szFileName = GetFileName();
	Debug::TriggerEvent( s_templateDbgInfo, szFileName, m_nRefCount, Debug::OnAddRef );
#endif
}

int CBsFXTemplate::Release()
{
	BsAssert( !IsRenderThread() );

	int nNewRefCount = --m_nRefCount;

#if defined(USE_TEMPLATEDBG)
	char const * szFileName = GetFileName();
	Debug::TriggerEvent( s_templateDbgInfo, szFileName, m_nRefCount, Debug::OnRelease );
#endif

	return nNewRefCount;
}

int CBsFXTemplate::GetRefCount()
{
	return m_nRefCount;
}

const char *CBsFXTemplate::GetFileName()
{
	return m_szFileName.c_str();
}