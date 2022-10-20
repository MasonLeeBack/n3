#pragma		once

class C3DDevice;

class CFcMinimapObject
{
public:
	CFcMinimapObject();
	~CFcMinimapObject();

	void Process();
	void Initialize( char* pMinimapFileName );

protected:
	static void CustomRender( void *pThis, C3DDevice *pDev, D3DXMATRIX *matObject );

	static void DrawUVMesh(LPDIRECT3DDEVICE9 pD3dDevice, D3DXVECTOR3 center, DWORD dwColor, float fScaleX, float fScaleY, int nU1, int nV1, int nU2, int nV2, int mapSizeX, int mapSizeY);
	static void DrawUVMesh(LPDIRECT3DDEVICE9 pD3dDevice, D3DXVECTOR3 center, DWORD dwColor, float fRot, float	fScaleX, float fScaleY, int nU1, int nV1, int nU2, int nV2, int mapSizeX, int mapSizeY);
	static void DrawUVMesh(LPDIRECT3DDEVICE9 pD3dDevice, D3DXVECTOR3 center, DWORD dwColor, int width, int height, int nU1, int nV1, int nU2, int nV2, int mapSizeX, int mapSizeY);
	static void DrawUVMesh(LPDIRECT3DDEVICE9 pD3dDevice, D3DXVECTOR3 center, DWORD dwColor, int width, int height, float fRot, float fU1, float fV1, float fU2, float fV2);
	static void DrawUVMesh(LPDIRECT3DDEVICE9 pD3dDevice, D3DXVECTOR3 center, DWORD dwColor, int width, int height, float fU1, float fV1, float fU2, float fV2);
	static void DrawUVMesh(LPDIRECT3DDEVICE9 pD3dDevice, D3DXVECTOR3* pLT, D3DXVECTOR3 *pRB, DWORD dwColorLT, DWORD dwColorRT, DWORD dwColorLB, DWORD dwColorRB, float fU1, float fV1, float fU2, float fV2);

protected:
	int m_nEngineIndex;
	int m_nBackgroundTexIndex;

	LPDIRECT3DDEVICE9		m_pD3DDevice;
	float m_fZoom;
};

