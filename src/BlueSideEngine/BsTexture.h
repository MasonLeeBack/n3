#pragma	  once
#include "BsUtil.h"

#define UNIT_BILLBOARD_SPRITE_COUNT	64

class C3DDevice;
class CBsTexture
{
public:
	CBsTexture();
	virtual ~CBsTexture();

	void Clear();

protected:
	char*					m_pszTextureName;	
	const char*					m_pszFileName;	// 파일이름만 빨리 리턴하기 위한 변수. m_pszTextureName 가 바뀔때마다 갱신해줘야 됨에 유의합니다.
	LPDIRECT3DBASETEXTURE9	m_pTexture;
	TEXTURE_TYPE			m_TextureType;
	CHAR *					m_pTexData;
	static volatile DWORD	s_dwTextureUsage;
	DWORD					m_dwSize;

	int							m_nRefCount;

	bool				m_bFromFile;
	DWORD				m_dwUsage;				// Resource가 어디에 할당되는가?

	LPDIRECT3DSURFACE9		m_pRenderTarget;
	LPDIRECT3DSURFACE9		m_pDepthStencil;

public:
	static CBsCriticalSection	s_csDeferredDeletedTexture;
	void AddRef();
	int Release();

	int GetRefCount() { return m_nRefCount; }

	HRESULT				LostResource();
	HRESULT				Reload(C3DDevice* pDevice);

	const char *			GetTextureName() { return m_pszFileName; }
	const char*			GetTextureFullName()		{	return m_pszTextureName; }
	LPDIRECT3DBASETEXTURE9	GetTexturePtr()			{	return m_pTexture;	}
	SIZE				GetTextureSize(int nLevel);
	LPDIRECT3DSURFACE9	GetRenderTargetSurface()	{	return m_pRenderTarget;	}
	LPDIRECT3DSURFACE9	GetDepthStencilSurface()	{	return m_pDepthStencil;	}
	HRESULT				LoadTexture(C3DDevice* pDevice, const char* pszTextureFileName, TEXTURE_TYPE type, DWORD dwUsage = 0);

	void				SetTextureName(const char* pszTextureName);

#ifdef _XBOX
	HRESULT				LoadArrayTexture(C3DDevice* pDevice, const char* pszTextureFolderName);
	static float		GetTextureMmUsage(){ return (float)s_dwTextureUsage; }
	static void			TextureMmReleased( DWORD dwSize ){ s_dwTextureUsage -= dwSize; }
#endif
	HRESULT				CreateTexture( C3DDevice* pDevice, int nWidth, int nHeight, DWORD dwUsage, D3DFORMAT format, D3DPOOL pool=D3DPOOL_DEFAULT);
#ifdef _XBOX
	void				Resolve(C3DDevice* pDevice, DWORD dwFlags, const D3DRECT* pRect, const D3DPOINT* pDestPoint, const D3DVECTOR4* pClearColor, float fClearZ, DWORD dwClearStencil);
#endif
};