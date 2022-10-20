#pragma once

enum SIDES
{
	TOP=0,
	LEFT,
	RIGHT,
	BOTTOM,
	TOTAL_SIDES
};

enum TERRAIN_LEVEL {
	LEVEL_0 = 0,
	LEVEL_1,
	LEVEL_2,
	LEVEL_3,
	TOTAL_LEVELS
};

struct INDEX_BUFFER {
	IDirect3DIndexBuffer9*	pIndexBuffer;
	int						IndexCount;
	int						TriangleCount;
};

struct DETAIL_LEVEL {
	INDEX_BUFFER 	TileBodies[16];
	INDEX_BUFFER	TileConnectors[TOTAL_SIDES][TOTAL_LEVELS];
};

class CBsTerrainZone {
public:
	int							nIndex;
	short						nXIndex;
	short						nZIndex;
	BOOL						bIsRender;

	TERRAIN_LEVEL				m_CurrentDetailLevel;		// Render할때만 유효!!!
	IDirect3DVertexBuffer9*		pZoneVB;
	D3DXVECTOR3					vecCenter;
	float						fCameraDistance;
};