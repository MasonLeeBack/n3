#pragma once 

class CBsWater
{
public:
	CBsWater();
	virtual ~CBsWater();

protected:

	bool	m_bReady;

	//LPDIRECT3DSURFACE9	m_pDepthSurface;

	LPDIRECT3DSURFACE9	m_pReflectSurface;
	LPDIRECT3DTEXTURE9	m_pReflectTexture;
	D3DVIEWPORT9			m_waterViewport;

	float							m_fWaterHeight;
	std::vector< Box3 >		m_BoxList;
	std::vector< BYTE* >	m_WaterCheckBufferList;
	bool							m_bWaterRendered;

public:
    bool IsVisible();
	bool IsWaterRendered();
	float GetWaterDistance( D3DXVECTOR3 Pos );
	bool IsWaterInside( D3DXVECTOR3 Pos );
	float GetWaterHeight( ) { return m_fWaterHeight; }
	void MakeReadyForUse();
	void Clear();
	void Render( C3DDevice *pDevice ,	std::vector< int >	*pRenderUpdateList );
	LPDIRECT3DTEXTURE9 GetReflectMap() { return m_pReflectTexture; }

	void AddWaterBoundingBox( AABB *pBox, BYTE *pWaterCheckBuffer );
};
