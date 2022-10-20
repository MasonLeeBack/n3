#include "stdafx.h"
#include "BsKernel.h"
#include "BsTexture.h"

#if defined(USE_TEXTUREDBG)

#include "BsDebugBreak.h"

static Debug::DebugInfo s_textureDbgInfo[] =
{
	//{ "twist_N.dds", Debug::OnAll, Debug::ShowAndBreak },
	{ "", Debug::OnNothing, Debug::DoNothing }
};

#endif

volatile DWORD CBsTexture::s_dwTextureUsage = 0;
CBsCriticalSection CBsTexture::s_csDeferredDeletedTexture;

CBsTexture::CBsTexture()
{
	m_pszTextureName = NULL;
	m_pszFileName = NULL;
	m_pTexture = NULL;
	m_nRefCount = 0;
	m_TextureType = TEXTURE_NORMAL;

	m_bFromFile = true;
	m_dwUsage = 0;

	m_pRenderTarget = NULL;
	m_pDepthStencil = NULL;

	m_pTexData = NULL;
	m_dwSize = 0;
}

CBsTexture::~CBsTexture()
{
#if defined(USE_TEXTUREDBG)
	char const * textureName = GetTextureName();
	Debug::TriggerEvent( s_textureDbgInfo, textureName, m_nRefCount, Debug::OnDelete );
#endif

	Clear();
}

void CBsTexture::AddRef()
{
#if defined(_XBOX)
	InterlockedIncrement( reinterpret_cast<volatile long*>(&m_nRefCount) );
#else
	++m_nRefCount;
#endif

#if defined(USE_TEXTUREDBG)
	char const * textureName = GetTextureName();
	Debug::TriggerEvent( s_textureDbgInfo, textureName, m_nRefCount, Debug::OnAddRef );
#endif
}

int CBsTexture::Release()
{
	int nNewRefCount = InterlockedDecrement( reinterpret_cast<volatile long*>(&m_nRefCount) );

	BsAssert( nNewRefCount >= 0 );

#if defined(USE_TEXTUREDBG)
	char const * textureName = GetTextureName();
	Debug::TriggerEvent( s_textureDbgInfo, textureName, nNewRefCount, Debug::OnRelease );
#endif

	if(nNewRefCount<=0){
		DebugString("Delete Texture : %s\n", GetTextureName());
		delete this;
		return 0;
	}
	return nNewRefCount;
}

void CBsTexture::Clear()
{
#ifdef _XBOX
	if(m_pTexture && !(m_pTexture->Common &  D3DCOMMON_D3DCREATED))
	{	// Manually do deferred delete of any textures not allocated by D3D
#ifdef _DEBUG
		g_BsKernel.GetInstance().AddDeferredDeleteResource(m_pTexture, m_pTexData, m_pszTextureName, m_dwSize);
#else
		g_BsKernel.GetInstance().AddDeferredDeleteResource(m_pTexture, m_pTexData, m_dwSize);
#endif
	}
	else
		SAFE_RELEASE(m_pTexture);

	SAFE_RELEASE(m_pRenderTarget);
	SAFE_RELEASE(m_pDepthStencil);

#else
	SAFE_RELEASE(m_pTexture);
#endif

	SAFE_DELETEA(m_pszTextureName);
	m_pszFileName = NULL;
}

HRESULT CBsTexture::LostResource()
{
#ifdef _XBOX
	if(m_pTexture && !(m_pTexture->Common &  D3DCOMMON_D3DCREATED))
	{	// Manually do deferred delete of any textures not allocated by D3D
		s_dwTextureUsage -= m_dwSize;
#ifdef _DEBUG
		g_BsKernel.GetInstance().AddDeferredDeleteResource(m_pTexture, m_pTexData, m_pszTextureName, m_dwSize);
#else
		g_BsKernel.GetInstance().AddDeferredDeleteResource(m_pTexture, m_pTexData, m_dwSize);
#endif
	}
	else
		SAFE_RELEASE(m_pTexture);

#else
	SAFE_RELEASE(m_pTexture);
#endif
	return S_OK;
}

HRESULT CBsTexture::Reload(C3DDevice* pDevice)
{
	if(!m_bFromFile)
		return S_OK;
#ifndef _XBOX
	if(m_dwUsage&D3DUSAGE_DYNAMIC) {
		return S_OK;
	}
#endif
	char szFileName[256];
	strcpy(szFileName, GetTextureFullName());
	Clear();

	return LoadTexture(pDevice, szFileName, m_TextureType);
}

/*
const char *CBsTexture::GetTextureName()
{	
	char *pFindPtr;
	if(!m_pszTextureName)
		return NULL;

	pFindPtr=(char *)strrchr(m_pszTextureName, '\\');
	if(pFindPtr){
		return pFindPtr+1;
	}
	else{
		return m_pszTextureName;
	}
}
*/

SIZE CBsTexture::GetTextureSize(int nLevel)
{	// 일반 텍스쳐로 가정한다.(cubemap, volumemap아닌걸로) by jeremy
	SIZE Size;
	D3DSURFACE_DESC Desc;

	if(m_pTexture){
		((LPDIRECT3DTEXTURE9)m_pTexture)->GetLevelDesc(nLevel, &Desc);
		Size.cx=Desc.Width;
		Size.cy=Desc.Height;
	}
	else{
		Size.cx=0;
		Size.cy=0;
	}
	return Size;
}

HRESULT CBsTexture::LoadTexture(C3DDevice* pDevice, const char* pszTextureFileName, TEXTURE_TYPE type, DWORD dwUsage/* =0 */)
{
	if(pDevice == NULL || pszTextureFileName == NULL)
		return E_FAIL;

	m_TextureType = type;
	m_dwUsage = dwUsage;

#if defined(_XBOX) && defined(_PACKED_RESOURCES)

	char szCompressedFileName[MAX_PATH];
	char *pDot;

	// Change the file extension - A hack for now, but it means that the rest of the
	// game can be unaware of compressed files....
	memset(szCompressedFileName, 0, MAX_PATH);
	memcpy(szCompressedFileName, pszTextureFileName, strlen(pszTextureFileName));
	pDot = strstr(szCompressedFileName, ".");
	if(pDot == NULL)
	{
		BsAssert( 0  && "LoadTexture - Invalid filename\n");
		return E_FAIL;
	}
	*pDot = NULL; //aleksger - safe string - converting to a strcat with known length
	strcat(szCompressedFileName, ".36t");
	BYTE *pTextureResource;

#ifdef ENABLE_RES_CHECKER
	g_BsResChecker.AddResList( szCompressedFileName );
#endif

	if((pTextureResource = CBsFileIO::BsFileIsPhysicalLoaded(szCompressedFileName)) != NULL)
	{
		m_pTexture = pDevice->RegisterPreLoadedTexture(pTextureResource);
		
		// Set m_pTextData to null so that it can't be released in the normal way...
		m_pTexData = NULL;
	}
	else
	{
		m_pTexture = pDevice->LoadPackedTexture(szCompressedFileName, &m_dwSize, &m_pTexData);
		s_dwTextureUsage += m_dwSize;
	}
#else

	switch(type) {
		case TEXTURE_NORMAL:
#ifdef _XBOX
			m_pTexture = (LPDIRECT3DTEXTURE9)pDevice->CreateTextureFromFile(pszTextureFileName, dwUsage, &m_pTexData );
#else
			m_pTexture = (LPDIRECT3DTEXTURE9)pDevice->CreateTextureFromFile(pszTextureFileName, dwUsage);
#endif
			break;
		case TEXTURE_CUBE:
			m_pTexture = (LPDIRECT3DTEXTURE9)pDevice->CreateCubeTextureFromFile(pszTextureFileName);
			break;
		case TEXTURE_VOLUME:
			m_pTexture = (LPDIRECT3DTEXTURE9)pDevice->CreateVolumeTextureFromFile(pszTextureFileName);
			break;
	}

#endif // defined(_XBOX) && defined(_PACKED_RESOURCES)

	if(m_pTexture) {
		const char* pFileName = pszTextureFileName;
		const size_t m_pszTextureName_len = strlen(pFileName)+1; //aleksger - safe string
		m_pszTextureName = new char[m_pszTextureName_len];
		strcpy_s(m_pszTextureName, m_pszTextureName_len, pFileName);
		
		//////////////////////////////////////////////////////////////////////////		
		char *pFindPtr=(char *)strrchr(m_pszTextureName, '\\');
		m_pszFileName = pFindPtr ? (pFindPtr+1) : m_pszTextureName;
		//////////////////////////////////////////////////////////////////////////		

#if defined(USE_TEXTUREDBG)
	char const * textureName = GetTextureName();
	Debug::TriggerEvent( s_textureDbgInfo, textureName, m_nRefCount, Debug::OnLoad );
#endif

		AddRef();
		return S_OK;
	}
	else {
		return E_FAIL;
	}
}

void CBsTexture::SetTextureName(const char* pszTextureName)
{
	SAFE_DELETEA(m_pszTextureName);
	const char* pTextureName = pszTextureName;
	const size_t m_pszTextureName_len = strlen(pTextureName)+1; //aleksger - safe string
	m_pszTextureName = new char[m_pszTextureName_len];
	strcpy_s(m_pszTextureName, m_pszTextureName_len, pTextureName);

	//////////////////////////////////////////////////////////////////////////		
	char *pFindPtr=(char *)strrchr(m_pszTextureName, '\\');
	m_pszFileName = pFindPtr ? (pFindPtr+1) : m_pszTextureName;
	//////////////////////////////////////////////////////////////////////////
}

#ifdef _XBOX
HRESULT CBsTexture::LoadArrayTexture(C3DDevice* pDevice, const char* pszTextureFolderName)
{	
#ifndef _PACKED_RESOURCES
	char szID [][64] = { "(0).dds", "(1).dds", "(2).dds", "(3).dds", "(4).dds", "(5).dds", "(6).dds", "(7).dds", "(8).dds", "(9).dds",
						"(10).dds","(11).dds","(12).dds","(13).dds","(14).dds","(15).dds","(16).dds","(17).dds","(18).dds","(19).dds",
						"(20).dds","(21).dds","(22).dds","(23).dds","(24).dds","(25).dds","(26).dds","(27).dds","(28).dds","(29).dds",
						"(30).dds","(31).dds","(32).dds","(33).dds","(34).dds","(35).dds","(36).dds","(37).dds","(38).dds","(39).dds",
						"(40).dds","(41).dds","(42).dds","(43).dds","(44).dds","(45).dds","(46).dds","(47).dds","(48).dds","(49).dds",
						"(50).dds","(51).dds","(52).dds","(53).dds","(54).dds","(55).dds","(56).dds","(57).dds","(58).dds","(59).dds",
						"(60).dds","(61).dds","(62).dds","(63).dds"};
	pDevice->GetD3DDevice()->CreateArrayTexture(64, 64, UNIT_BILLBOARD_SPRITE_COUNT, 1, 0, D3DFMT_DXT1, 0, (LPDIRECT3DARRAYTEXTURE9*)&m_pTexture, NULL);
	char szFolder[MAX_PATH];
	strcpy( szFolder, pszTextureFolderName );
	strcat( szFolder, "\\" );
	for (int i = 0; i < UNIT_BILLBOARD_SPRITE_COUNT; i++)
	{
		char szFullFilename[MAX_PATH];
		memset(szFullFilename, 0, MAX_PATH);
		strcpy( szFullFilename, szFolder );
		strcat( szFullFilename, "BILL_ " );
		strcat( szFullFilename, szID[i] );
		int nTextureIndex = g_BsKernel.GetInstance().LoadTexture( szFullFilename );
		if (nTextureIndex != -1)
		{
			LPDIRECT3DTEXTURE9 pTex = (LPDIRECT3DTEXTURE9)g_BsKernel.GetInstance().GetTexturePtr(nTextureIndex);
			LPDIRECT3DSURFACE9	pSurface, pDestSurface;
			pTex->GetSurfaceLevel(0, &pSurface);
			LPDIRECT3DARRAYTEXTURE9	pArrayTex = (LPDIRECT3DARRAYTEXTURE9)m_pTexture;
			pArrayTex->GetArraySurface(i, 0, &pDestSurface);
			D3DXLoadSurfaceFromSurface(pDestSurface, NULL, NULL, pSurface, NULL, NULL, D3DX_DEFAULT, D3DCOLOR_XRGB(255, 255, 255));
            SAFE_RELEASE(pDestSurface);
			SAFE_RELEASE(pSurface);
		}
		else 
		{
			SAFE_RELEASE(m_pTexture);
			return E_FAIL;
		}
		SAFE_RELEASE_TEXTURE(nTextureIndex);
	}
#else
	char *pFullPath = g_BsKernel.GetInstance().GetFullName(pszTextureFolderName);
	char szFullFilename[MAX_PATH];
	memset(szFullFilename, 0, MAX_PATH);
	strcpy( szFullFilename, pFullPath );
	char *pFolderName=(char *)strrchr(szFullFilename, '\\');
	char szFolderName[MAX_PATH];
	strcpy(szFolderName, pFolderName);
	strcat(szFullFilename, szFolderName);
	strcat(szFullFilename, ".36T");

#ifdef ENABLE_RES_CHECKER
	g_BsResChecker.AddResList( szFullFilename );
#endif
	m_pTexture = pDevice->LoadPackedTexture(szFullFilename, &m_dwSize, &m_pTexData);
	s_dwTextureUsage += m_dwSize;
#endif
	if(m_pTexture)
	{
		const char* pFileName = pszTextureFolderName;
		const size_t m_pszTextureName_len = strlen(pFileName)+1; //aleksger - safe string
		m_pszTextureName = new char[m_pszTextureName_len];
		strcpy_s(m_pszTextureName, m_pszTextureName_len, pFileName);

		//////////////////////////////////////////////////////////////////////////		
		char *pFindPtr=(char *)strrchr(m_pszTextureName, '\\');
		m_pszFileName = pFindPtr ? (pFindPtr+1) : m_pszTextureName;
		//////////////////////////////////////////////////////////////////////////

		AddRef();
		return S_OK;
	}
	else
	{
		return E_FAIL;
	}
}
#endif

HRESULT CBsTexture::CreateTexture( C3DDevice* pDevice, int nWidth, int nHeight, DWORD dwUsage, D3DFORMAT format, D3DPOOL pool/* =0 */)
{
	BsAssert( m_pTexture == NULL );
	BsAssert( m_pRenderTarget == NULL );
	BsAssert( m_pDepthStencil == NULL );

	m_TextureType = TEXTURE_NORMAL;
	m_pTexture = (LPDIRECT3DTEXTURE9)pDevice->CreateTexture(&m_dwSize, &m_pTexData, nWidth, nHeight, dwUsage, format, pool);
	s_dwTextureUsage += m_dwSize;
	if( dwUsage & D3DUSAGE_RENDERTARGET) {
#ifdef _XBOX
		D3DSURFACE_PARAMETERS TileSurfaceParams;
		TileSurfaceParams.Base = 0;
		TileSurfaceParams.HierarchicalZBase = 0;
		TileSurfaceParams.ColorExpBias = 0;
		pDevice->GetD3DDevice()->CreateRenderTarget(nWidth, nHeight, format, D3DMULTISAMPLE_NONE,
			0, FALSE, &m_pRenderTarget, &TileSurfaceParams );
		TileSurfaceParams.Base += (m_pRenderTarget->Size/GPU_EDRAM_TILE_SIZE);
		pDevice->GetD3DDevice()->CreateDepthStencilSurface(nWidth, nHeight, D3DFMT_D24FS8, D3DMULTISAMPLE_NONE,
			0, FALSE, &m_pDepthStencil, &TileSurfaceParams );
#else
		if( m_pTexture )
		{
            ((LPDIRECT3DTEXTURE9)m_pTexture)->GetSurfaceLevel(0, &m_pRenderTarget);
		}
		else
		{
			BsAssert( 0 && "Error : CBsTexture::CreateTexture. Failed to create texture");
		}
#endif
	}
	if(m_pTexture) {
		m_pszTextureName = NULL;
		m_pszFileName = NULL;
		m_bFromFile = false;
		m_dwUsage = dwUsage;
		AddRef();
		return S_OK;
	}
	else {
		return E_FAIL;
	}
}
#ifdef _XBOX
void CBsTexture::Resolve(C3DDevice* pDevice, DWORD dwFlags, const D3DRECT* pRect, const D3DPOINT* pDestPoint, const D3DVECTOR4* pClearColor, float fClearZ, DWORD dwClearStencil)
{
	pDevice->Resolve(dwFlags, pRect, m_pTexture, pDestPoint, 0, 0, pClearColor, fClearZ, dwClearStencil, NULL );
}
#endif
