#include "stdafx.h"
#include "BsKernel.h"
#include "BsShadowMgr.h"
#include "BsLODWorld.h"
#include "BsMaterial.h"

//**************************************************************************
//		이부분은 송지상씨가 작업한 부분입니다.
//**************************************************************************
inline void GenerateQuad( WORD*& data, int i, int inc, int stride )
{
	*data++ = WORD(i);
	*data++ = WORD(i + stride + inc);
	*data++ = WORD(i + inc);

	*data++ = WORD(i + stride);
	*data++ = WORD(i + stride + inc);
	*data++ = WORD(i);
}

void GenerateCenterSq( int dim, int inc, WORD* data )
{
	int stride = dim * inc;
	int lastRow = dim * dim - (2 * stride);

	for ( int j=stride; j<lastRow; j += stride ) {

		int rowStart = j + inc;
		int rowEnd = j + dim - (2 * inc);

		for ( int i=rowStart; i<rowEnd; i += inc ) {
			GenerateQuad( data, i, inc, stride );
		}
	}
}

void GenerateSides( int dim, int inc, WORD** array )
{
	int stride = dim * inc;
	int i;

	// TOP------------------
	WORD* data = array[TOP];

	// firstTriangle
	*data++ = WORD(0);
	*data++ = WORD(0 + stride + inc);
	*data++ = WORD(0 + inc);

	// middle triangles
	int rowStart = inc;
	int rowEnd = dim - 1 - inc;

	for ( i=rowStart; i<rowEnd; i += inc ) {
		GenerateQuad( data, i, inc, stride );
	}

	// last triangle
	*data++ = WORD(rowEnd);
	*data++ = WORD(rowEnd + stride);
	*data++ = WORD(rowEnd + inc);

	// LEFT-----------------
	data = array[LEFT];

	// firstTriangle
	*data++ = WORD(0);
	*data++ = WORD(0 + stride);
	*data++ = WORD(0 + stride + inc);

	// middle triangles
	int colStart = stride;
	int colEnd = dim * (dim - 1) - stride;

	for ( i=colStart; i<colEnd; i += stride ) {
		GenerateQuad( data, i, inc, stride );
	}

	// last triangle
	*data++ = WORD(colEnd);
	*data++ = WORD(colEnd + stride);
	*data++ = WORD(colEnd + inc);

	// RIGHT----------------
	data = array[RIGHT];

	// firstTriangle
	*data++ = WORD(dim - 1);
	*data++ = WORD(dim - 1 + stride - inc);
	*data++ = WORD(dim - 1 + stride);

	// middle triangles
	colStart = dim - 1 + stride - inc;
	colEnd = dim * dim - 1 - stride - inc;

	for ( i=colStart; i<colEnd; i += stride ) {
		GenerateQuad( data, i, inc, stride );
	}

	// last triangle
	*data++ = WORD(colEnd);
	*data++ = WORD(colEnd + inc + stride);
	*data++ = WORD(colEnd + inc);

	// BOTTOM---------------
	data = array[BOTTOM];

	rowStart = dim * (dim - 1) - stride + inc;
	rowEnd = rowStart + dim - 1 - (2 * inc);

	// firstTriangle
	*data++ = WORD(rowStart);
	*data++ = WORD(rowStart + stride - inc);
	*data++ = WORD(rowStart + stride);

	// middle triangles
	for ( i=rowStart; i<rowEnd; i += inc ) {
		GenerateQuad( data, i, inc, stride );
	}

	// last triangle
	*data++ = WORD(rowEnd);
	*data++ = WORD(rowEnd + stride);
	*data++ = WORD(rowEnd + inc + stride);
}

void GenerateConnectors( int dim, int highDim, int lowDim, WORD** array )
{
	int highInc = (dim - 1) / highDim;		// increment for higher detail
	int lowInc = (dim - 1) / lowDim;			// increment for lower detail

	int highStride = dim * highInc;
	int lowStride = dim * lowInc;

	int highPos, highEnd;
	int lowPos, lowEnd;

	int ratio = (highDim - 2) / (lowDim + 1);
	bool odd = ((highDim - 2) % (lowDim + 1)) != 0;
	if ( odd )
		ratio += 1;

	int count;

	WORD* data;

	// TOP-----------------
	data = array[TOP];	

	lowPos = 0;
	highPos = highInc + highStride;

	lowEnd = dim - 1;
	highEnd = highStride + dim - 1 - highInc;

	count = ( odd ) ? 1 : 0;

	for ( ; lowPos<=lowEnd; lowPos += lowInc ) {
		while (count < ratio && highPos < highEnd) {
			*data++ = WORD(lowPos);
			*data++ = WORD(highPos);
			*data++ = WORD(highPos + highInc);

			highPos += highInc;
			++count;
		}

		if ( lowPos < lowEnd ) {
			*data++ = WORD(lowPos);
			*data++ = WORD(highPos);
			*data++ = WORD(lowPos + lowInc);
		}

		count = 0;
	}

	// LEFT-----------------
	data = array[LEFT];	

	lowPos = 0;
	highPos = highInc + highStride;

	lowEnd = dim * (dim - 1);
	highEnd = lowEnd - highStride + highInc;

	count = ( odd ) ? 1 : 0;

	for ( ; lowPos<=lowEnd; lowPos += lowStride ) {
		while (count < ratio && highPos < highEnd) {
			*data++ = WORD(lowPos);
			*data++ = WORD(highPos + highStride);
			*data++ = WORD(highPos);

			highPos += highStride;
			++count;
		}

		if ( lowPos < lowEnd ) {
			*data++ = WORD(lowPos);
			*data++ = WORD(lowPos + lowStride);
			*data++ = WORD(highPos);
		}
		count = 0;
	}

	// RIGHT-----------------
	data = array[RIGHT];	

	lowPos = dim - 1;
	highPos = dim - 1 - highInc + highStride;

	lowEnd = (dim * dim) - 1;
	highEnd = lowEnd - highStride - highInc;

	count = ( odd ) ? 1 : 0;

	for ( ; lowPos<=lowEnd; lowPos += lowStride ) {
		while (count < ratio && highPos < highEnd) {
			*data++ = WORD(highPos);
			*data++ = WORD(highPos + highStride);
			*data++ = WORD(lowPos);

			highPos += highStride;
			++count;
		}

		if ( lowPos < lowEnd ) {
			*data++ = WORD(highPos);
			*data++ = WORD(lowPos + lowStride);
			*data++ = WORD(lowPos);
		}

		count = 0;
	}

	// BOTTOM-----------------
	data = array[BOTTOM];	

	lowPos = dim * (dim - 1);
	highPos = lowPos - highStride + highInc;

	lowEnd = (dim * dim) - 1;
	highEnd = lowEnd - highStride - highInc;

	count = ( odd ) ? 1 : 0;

	for ( ; lowPos<=lowEnd; lowPos += lowInc ) {
		while (count < ratio && highPos < highEnd) {
			*data++ = WORD(lowPos);
			*data++ = WORD(highPos + highInc);
			*data++ = WORD(highPos);

			highPos += highInc;
			++count;
		}

		if ( lowPos < lowEnd ) {
			*data++ = WORD(lowPos);
			*data++ = WORD(lowPos + lowInc);
			*data++ = WORD(highPos);
		}

		count = 0;
	}
}

//**************************************************************************
//		Class Name	: CBsLODWorld
//		DESC		: Index buffer가 타일화된 Terrain LOD방법입니다.
//**************************************************************************
CBsLODWorld::CBsLODWorld(int nXSizePerZone/*=40*/, int nYSizePerZone/*=40*/)
{
	m_nXSizePerZone = nXSizePerZone;
	m_nZSizePerZone = nYSizePerZone;
	m_nXVertexCountPerZone = m_nXSizePerZone+1;
	m_nZVertexCountPerZone = m_nZSizePerZone+1;

	m_nVertexCountPerZone = m_nXVertexCountPerZone * m_nZVertexCountPerZone;

	m_pTerrainZones = NULL;

	m_DrawZoneList.clear();
	SetTerrainTechnique(2);
#ifdef _USAGE_TOOL_
	m_nAttributeTextureIndex = -1;
#endif

	m_nLightMapTexture	= -1;
	m_fRcpWorldXSize	= 0.f;
	m_fRcpWorldZSize	= 0.f;

	for (int i=0;i<TOTAL_LEVELS;++i) {
		for (int j=0;j<16;++j) {
			m_DetailLevel[i].TileBodies[j].pIndexBuffer = NULL;
		}
		for (int k=0;k<TOTAL_SIDES;++k) {
			for (int l=0;l<TOTAL_LEVELS;++l) {
				m_DetailLevel[i].TileConnectors[k][l].pIndexBuffer = NULL;
			}
		}
	}
	m_nForceLODLevel = -1;
}

CBsLODWorld::~CBsLODWorld()
{
	Clear();
}

void CBsLODWorld::Create(int nSizeX, int nSizeZ, short int *pHeight, const char **ppFileList, const char* pszLightMapFileName, DWORD *pMulColor, DWORD *pAddColor)
{
	m_nWorldXSize = nSizeX;
	m_nWorldZSize = nSizeZ;

	m_fRcpWorldXSize = 1.f/(float(m_nWorldXSize)*DISTANCE_PER_CELL);
	m_fRcpWorldZSize = 1.f/(float(m_nWorldZSize)*DISTANCE_PER_CELL);

	m_pHeight = pHeight;

	m_pdwMul = pMulColor;
	m_pdwAdd = pAddColor;

	m_nZoneHoriCount = nSizeX/m_nXSizePerZone;
	m_nZoneVertCount = nSizeZ/m_nZSizePerZone;

	m_pMaxHeight=new float[m_nZoneVertCount*m_nZoneHoriCount];
	m_pMinHeight=new float[m_nZoneVertCount*m_nZoneHoriCount];

	for(int i=0;i<m_nZoneVertCount*m_nZoneHoriCount;i++){
		m_pMaxHeight[i] = -FLT_MAX;
		m_pMinHeight[i] = FLT_MAX;
	}

	// Zone의 Visible Test를 위한 Min,Max 저장
	int nWorldXIndexCount = m_nWorldXSize + 1;
	int nWorldZIndexCount = m_nWorldZSize + 1;
	for(int i=0 ; i<m_nZoneVertCount ; ++i) {
		for(int j=0 ; j<m_nZoneHoriCount ; ++j) {
			int nZoneIndex = i*m_nZoneHoriCount+j;
			int nStartXCellIndex = j*m_nXSizePerZone;
			int nStartZCellIndex = i*m_nZSizePerZone;
			int nStartIndex = nStartZCellIndex * nWorldXIndexCount + nStartXCellIndex;
			// Zone에 해당하는 Grid의 Minimum,Maximum Height를 저장한다.
			for(int k=0 ; k<m_nZVertexCountPerZone ; ++k) {
				int nInnerXCellIndex = nStartIndex + (k*nWorldXIndexCount);
				for(int p=0 ; p<m_nXVertexCountPerZone ; ++p) {
					int nCurIndex = nInnerXCellIndex+p;
					if(m_pHeight[nCurIndex]>m_pMaxHeight[nZoneIndex]) {
						m_pMaxHeight[nZoneIndex] = m_pHeight[nCurIndex];
					}
					if(m_pHeight[nCurIndex]<m_pMinHeight[nZoneIndex]) {
						m_pMinHeight[nZoneIndex] = m_pHeight[nCurIndex];
					}
				}
			}
		}
	}

	float fXLengthPerZone = m_nXSizePerZone * DISTANCE_PER_CELL;
	float fZLengthPerZone = m_nZSizePerZone * DISTANCE_PER_CELL;

	float fXLength_2 = fXLengthPerZone*0.5f;
	float fZLength_2 = fZLengthPerZone*0.5f;

	//**************************************************************
	//	Create Vertex Buffer!!!
	//**************************************************************
	m_pTerrainZones = new CBsTerrainZone[m_nZoneHoriCount * m_nZoneVertCount];
	for(int i=0;i<m_nZoneVertCount;++i) {
		for(int j=0;j<m_nZoneHoriCount;++j) {
			int nIndex = i*m_nZoneHoriCount + j;
			g_BsKernel.CreateVertexBuffer(m_nVertexCountPerZone*sizeof(HFVERTEX), D3DUSAGE_WRITEONLY, 0, 
				D3DPOOL_MANAGED, &(m_pTerrainZones[nIndex].pZoneVB));
			WriteZoneData(m_pTerrainZones[nIndex].pZoneVB, nIndex);
			m_pTerrainZones[nIndex].vecCenter
				= D3DXVECTOR3(j*fXLengthPerZone+fXLength_2, (m_pMaxHeight[nIndex]+m_pMinHeight[nIndex])*0.5f, i*fZLengthPerZone+fZLength_2);
			m_pTerrainZones[nIndex].nIndex = nIndex;
			m_pTerrainZones[nIndex].nXIndex = j;
			m_pTerrainZones[nIndex].nZIndex = i;
		}
	}

	// Generate Index Buffer for Detail Level!!
	GenerateDetailLevel();

	// Material Setting!!
	char fullName[_MAX_PATH];
	strcpy(fullName, g_BsKernel.GetShaderDirectory());
#ifdef _XBOX		
	strcat(fullName, "Terrain.fx");
#else
	strcat(fullName, "Terrain_PC.fx");
#endif

	m_nHFMaterialIndex = g_BsKernel.LoadMaterial(fullName, FALSE);

	CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(m_nHFMaterialIndex);
#ifdef _XBOX
	m_hLightDir = pMaterial->GetParameterByName("Direction");
	m_hLightDiffuse = pMaterial->GetParameterByName( "LightDiffuse");
	m_hLightSpecular = pMaterial->GetParameterByName("LightSpecular");
	m_hLightAmbient = pMaterial->GetParameterByName("LightAmbient");
	m_hFogFactor = pMaterial->GetParameterByName("FOGFACTOR");
	m_hFogColor = pMaterial->GetParameterByName("FOGCOLOR");

	m_hWVP = pMaterial->GetParameterByName("WorldViewProjection");
	m_hWV = pMaterial->GetParameterByName("WorldView");
	m_hWVIT = pMaterial->GetParameterByName("WorldViewInverseTranspose");
	m_hShadowMatrix = pMaterial->GetParameterByName("SHADOW_ALL");

	m_hLayer0Texture = pMaterial->GetParameterByName("Layer0Sampler");
	m_hLayer1Texture = pMaterial->GetParameterByName("Layer1Sampler");
	m_hLayer2Texture = pMaterial->GetParameterByName("Layer2Sampler");
	m_hLayer3Texture = pMaterial->GetParameterByName("Layer3Sampler");
	m_hLayer4Texture = pMaterial->GetParameterByName("Layer4Sampler");
	m_hBlendTexture = pMaterial->GetParameterByName("BlendSampler");
	m_hLightMap = pMaterial->GetParameterByName("LightmapSampler");
	m_hShadowTexture = pMaterial->GetParameterByName("ShadowMapSampler");
#else
	m_hLightDir = pMaterial->GetParameterByName("lightDir");
	m_hLightDiffuse = pMaterial->GetParameterByName("lightDiffuse");
	m_hLightSpecular = pMaterial->GetParameterByName("lightSpecular");
	m_hLightAmbient = pMaterial->GetParameterByName("lightAmbient");
	m_hFogFactor = pMaterial->GetParameterByName("fogFactor");
	m_hFogColor = pMaterial->GetParameterByName("fogColor");

	m_hWVP = pMaterial->GetParameterByName("worldViewProj");
	m_hWV = pMaterial->GetParameterByName("WorldView");
	m_hWVIT = pMaterial->GetParameterByName("worldviewInverseTranspose");
	m_hShadowMatrix = pMaterial->GetParameterByName("shadowAll");

	m_hLayer0Texture = pMaterial->GetParameterByName("Layer0texture");
	m_hLayer1Texture = pMaterial->GetParameterByName("Layer1texture");
	m_hLayer2Texture = pMaterial->GetParameterByName("Layer2texture");
	m_hLayer3Texture = pMaterial->GetParameterByName("Layer3texture");
	m_hLayer4Texture = pMaterial->GetParameterByName("Layer4texture");
	m_hBlendTexture = pMaterial->GetParameterByName("BlendTexture");
	m_hLightMap = pMaterial->GetParameterByName("LightmapTexture");
	m_hShadowTexture = pMaterial->GetParameterByName("ShadowMap");
#ifdef _USAGE_TOOL_
	m_hAttributeMap = pMaterial->GetParameterByName("AttributeTexture");
#endif
#endif


	D3DVERTEXELEMENT9 decl[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0, 24, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
		{ 0, 28, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 1 },
		{ 0, 32, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },

		D3DDECL_END()
	};
	m_nTerrainVertexDeclIndex = g_BsKernel.LoadVertexDeclaration(decl);

	for(int i=0;i<6;i++){
		m_nTextureIndex[i] = g_BsKernel.LoadTexture(ppFileList[i]);
	}
	if(pszLightMapFileName) {
		LoadWorldLightMap(pszLightMapFileName);
	}
}

void CBsLODWorld::Clear()
{
	if(m_pMaxHeight){
		delete [] m_pMaxHeight;
		m_pMaxHeight=NULL;
	}
	if(m_pMinHeight){
		delete [] m_pMinHeight;
		m_pMinHeight=NULL;
	}

	SAFE_RELEASE_TEXTURE(m_nLightMapTexture);

	SAFE_RELEASE_MATERIAL(m_nHFMaterialIndex);

	SAFE_RELEASE_VD(m_nTerrainVertexDeclIndex);

	for(int i=0;i<6;i++) {
		SAFE_RELEASE_TEXTURE(m_nTextureIndex[i]);
	}

	if(m_pTerrainZones) {
		// Release VertexBuffer
		for(int i=0;i<m_nZoneVertCount;++i) {
			for(int j=0;j<m_nZoneHoriCount;++j) {
				int nIndex = i*m_nZoneHoriCount + j;
				SAFE_RELEASE(m_pTerrainZones[nIndex].pZoneVB);
			}
		}
		delete[] m_pTerrainZones;
		m_pTerrainZones=NULL;
	}

	// Release IndexBuffer the detail levels
	for (int i=0;i<TOTAL_LEVELS;++i) {
		for (int j=0;j<16;++j) {
			SAFE_RELEASE(m_DetailLevel[i].TileBodies[j].pIndexBuffer);
		}
		for (int k=0;k<TOTAL_SIDES;++k) {
			for (int l=0;l<TOTAL_LEVELS;++l) {
				SAFE_RELEASE(m_DetailLevel[i].TileConnectors[k][l].pIndexBuffer);
			}
		}
	}
}

void CBsLODWorld::Reload()
{
}

void CBsLODWorld::WriteZoneData(IDirect3DVertexBuffer9* pVB, int nZoneIndex)
{
	int nStartCellX, nStartCellZ;
	float fTextureU, fTextureV;
	int nBufIndex, nHeightIndex;

	nStartCellX=nZoneIndex%m_nZoneHoriCount*m_nXSizePerZone;
	nStartCellZ=nZoneIndex/m_nZoneHoriCount*m_nZSizePerZone;

	HFVERTEX *pBuf;
	pVB->Lock(0, 0, (void **)&pBuf, 0);
	for(int i=0;i<m_nZSizePerZone+1;i++){
		if(i+nStartCellZ>m_nWorldZSize){
			break;
		}
		for(int j=0;j<m_nXSizePerZone+1;j++){
			if(j+nStartCellX>m_nWorldXSize){
				break;
			}

			if(i+nStartCellZ<0) {
				continue;
			}

			nBufIndex=(i*(m_nXSizePerZone+1)+j);
			fTextureU = float(nStartCellX+j)/float(m_nWorldXSize);
			fTextureV = 1.f-float(nStartCellZ+i)/float(m_nWorldZSize);

			nHeightIndex=(m_nWorldXSize+1)*(i+nStartCellZ)+j+nStartCellX;

			pBuf[nBufIndex].vecPos.x = j*DISTANCE_PER_CELL;
			pBuf[nBufIndex].vecPos.y = m_pHeight[nHeightIndex];
			pBuf[nBufIndex].vecPos.z = i*DISTANCE_PER_CELL;
			pBuf[nBufIndex].dwMulColor = m_pdwMul[nHeightIndex];
			pBuf[nBufIndex].dwAddColor = m_pdwAdd[nHeightIndex];
			pBuf[nBufIndex].vecUV.x=fTextureU;
			pBuf[nBufIndex].vecUV.y=fTextureV;
			pBuf[nBufIndex].vecNor=GetNormalVector(j+nStartCellX, i+nStartCellZ);
		}
	}
	pVB->Unlock();
}

void CBsLODWorld::GenerateDetailLevel()
{
	//**************************************************************
	//	Create Index Buffer!!!
	//**************************************************************
	WORD *pFaceBuf;
	// Detail Level 0 (Lowest Level)	=> 5 X 5 Tile
	g_BsKernel.CreateIndexBuffer(5*5*2*3*sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16,
		D3DPOOL_MANAGED, &m_DetailLevel[0].TileBodies[0].pIndexBuffer);
	m_DetailLevel[0].TileBodies[0].pIndexBuffer->Lock(0, 0, (void**)&pFaceBuf, 0 );
	WORD nIndex;
	for(WORD i=0; i < 5; ++i ) {
		for(WORD j=0; j < 5; ++j) {
			WORD nXOffset = j*8;
			WORD nZOffset = i*8;
			nIndex = nZOffset*WORD(m_nXVertexCountPerZone) + nXOffset;
			*pFaceBuf = nIndex;
			pFaceBuf++;
			*pFaceBuf = (nIndex+8) + (8*WORD(m_nXVertexCountPerZone));
			pFaceBuf++;
			*pFaceBuf = nIndex +8;
			pFaceBuf++;

			*pFaceBuf=nIndex;
			pFaceBuf++;
			*pFaceBuf=nIndex + (8*WORD(m_nXVertexCountPerZone));
			pFaceBuf++;
			*pFaceBuf=(nIndex+8) + (8*WORD(m_nXVertexCountPerZone));
			pFaceBuf++;
		}
	}

	m_DetailLevel[0].TileBodies[0].pIndexBuffer->Unlock();
	m_DetailLevel[0].TileBodies[0].IndexCount = 5*5*2*3;
	m_DetailLevel[0].TileBodies[0].TriangleCount = 5*5*2;

	const int _TILE_VERT = 41;
	const int MAX_LEVEL = 3;
	int dim[MAX_LEVEL];
	int centerSqSize[MAX_LEVEL];
	int sideSize[MAX_LEVEL];
	int inc[MAX_LEVEL];

	int level;

	int dimension = 10;
	for (int i=0; i<MAX_LEVEL; i++ ) {
		dim[i] = dimension;
		dimension *= 2;

		centerSqSize[i] = (dim[i] - 2) * (dim[i] - 2) * 2 * 3;
		sideSize[i] = ((dim[i] - 2) * 2 + 2) * 3;
		inc[i] = _TILE_VERT / dim[i];
	}

	const int connectingEdgeBase = 5;
	int connectingEdge;
	int j;

	// level 1 (10 x 10)
	level = 0;
	WORD* _Level1_Center = new WORD[centerSqSize[level]];
	WORD* _SidesOfLevel1[TOTAL_SIDES];
	for (int i=0; i<TOTAL_SIDES; i++ )
		_SidesOfLevel1[i] = new WORD[sideSize[level]];

	GenerateCenterSq( _TILE_VERT, inc[level], _Level1_Center );
	GenerateSides( _TILE_VERT, inc[level], _SidesOfLevel1 );

	WORD*** _ConnectorsLevel1 = new WORD**[level + 1];
	int* ConnectorLevel1Size = new int[level + 1];
	connectingEdge = connectingEdgeBase;

	for (int i=0; i<=level; i++ ) {
		_ConnectorsLevel1[i] = new WORD*[TOTAL_SIDES];

		// num triangles = num sides in center sq + num sides in connecting edge
		int connectorSize = (dim[level] - 2);
		connectorSize += connectingEdge;
		connectorSize *= 3;		// three index per triangle

		ConnectorLevel1Size[i] = connectorSize;

		for ( j=0; j<TOTAL_SIDES; j++ )
			_ConnectorsLevel1[i][j] = new WORD[connectorSize];

		GenerateConnectors( _TILE_VERT, dim[level], connectingEdge, _ConnectorsLevel1[i] );

		connectingEdge *= 2;	// double for next level
	}

	// Detail Level 1	=> 11 vertex X 11 vertex
	for (int body=0;body<16;++body) {
		m_DetailLevel[1].TileBodies[body].pIndexBuffer=0;
		m_DetailLevel[1].TileBodies[body].IndexCount = 0;

		int total_indexes=centerSqSize[0];
		if (!(body & (1<<0))) total_indexes += sideSize[0];
		if (!(body & (1<<1))) total_indexes += sideSize[0];
		if (!(body & (1<<2))) total_indexes += sideSize[0];
		if (!(body & (1<<3))) total_indexes += sideSize[0];

		if (total_indexes) {
			g_BsKernel.CreateIndexBuffer(total_indexes*sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16 , D3DPOOL_MANAGED, &m_DetailLevel[1].TileBodies[body].pIndexBuffer);
			{
				WORD* pIndex;
				m_DetailLevel[1].TileBodies[body].pIndexBuffer->Lock(0,total_indexes*2, (void**)&pIndex, D3DLOCK_NOSYSLOCK);
				{
					int index=0;

					// start by copying the center portion of the tile
					for (int center_vert=0;center_vert<centerSqSize[0];++center_vert) {
						pIndex[index++] = _Level1_Center[center_vert];
					}

					for (int side=0;side<TOTAL_SIDES;++side) {
						if (!(body & (1<<side))) {
							for (int data=0;data<sideSize[0];++data) {
								pIndex[index++] = _SidesOfLevel1[side][data];
							}
						}
					}
					m_DetailLevel[1].TileBodies[body].pIndexBuffer->Unlock();
					m_DetailLevel[1].TileBodies[body].IndexCount = total_indexes;
					m_DetailLevel[1].TileBodies[body].TriangleCount = total_indexes/3;
				}
			}
		}
	}
	// create the tile connectors
	for (int side=0;side<TOTAL_SIDES;++side) {
		m_DetailLevel[1].TileConnectors[side][0].pIndexBuffer = 0;
		g_BsKernel.CreateIndexBuffer(ConnectorLevel1Size[0]*2, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16 , D3DPOOL_MANAGED, &m_DetailLevel[1].TileConnectors[side][0].pIndexBuffer);
		WORD* pIndex;
		m_DetailLevel[1].TileConnectors[side][0].pIndexBuffer->Lock(0,ConnectorLevel1Size[0]*2, (void**)&pIndex, D3DLOCK_NOSYSLOCK);
		for (int count=0;count<ConnectorLevel1Size[0];++count) {
			//pIndex[count] = Connect1to0[side][count];
			pIndex[count] = _ConnectorsLevel1[0][side][count];
		}
		m_DetailLevel[1].TileConnectors[side][0].pIndexBuffer->Unlock();
		m_DetailLevel[1].TileConnectors[side][0].IndexCount = ConnectorLevel1Size[0];
		m_DetailLevel[1].TileConnectors[side][0].TriangleCount = ConnectorLevel1Size[0]/3;
		delete[] _ConnectorsLevel1[0][side];
	}

	for(int i=0;i<=level; i++) {
		delete[] _ConnectorsLevel1[i];
	}
	delete[] _ConnectorsLevel1;
	delete[] ConnectorLevel1Size;

	// level 2 (20 x 20)
	level = 1;
	WORD* _Level2_Center = new WORD[centerSqSize[level]];
	WORD* _SidesOfLevel2[TOTAL_SIDES];
	for (int i=0; i<TOTAL_SIDES; i++ )
		_SidesOfLevel2[i] = new WORD[sideSize[level]];

	GenerateCenterSq( _TILE_VERT, inc[level], _Level2_Center );
	GenerateSides( _TILE_VERT, inc[level], _SidesOfLevel2 );

	WORD*** _ConnectorsLevel2 = new WORD**[level + 1];
	int* ConnectorLevel2Size = new int[level + 1];
	connectingEdge = connectingEdgeBase;

	for (int i=0; i<=level; i++ ) {
		_ConnectorsLevel2[i] = new WORD*[TOTAL_SIDES];

		// num triangles = num sides in center sq + num sides in connecting edge
		int connectorSize = (dim[level] - 2);
		connectorSize += connectingEdge;
		connectorSize *= 3;		// three index per triangle

		ConnectorLevel2Size[i] = connectorSize;

		for ( j=0; j<TOTAL_SIDES; j++ )
			_ConnectorsLevel2[i][j] = new WORD[connectorSize];

		GenerateConnectors( _TILE_VERT, dim[level], connectingEdge, _ConnectorsLevel2[i] );

		connectingEdge *= 2;	// double for next level
	}

	// create each of the 16 tile bodies
	for (int body=0;body<16;++body) {
		m_DetailLevel[2].TileBodies[body].pIndexBuffer=0;
		m_DetailLevel[2].TileBodies[body].IndexCount = 0;

		int total_indexes=centerSqSize[1];
		if (!(body & (1<<0))) total_indexes += sideSize[1];
		if (!(body & (1<<1))) total_indexes += sideSize[1];
		if (!(body & (1<<2))) total_indexes += sideSize[1];
		if (!(body & (1<<3))) total_indexes += sideSize[1];

		if (total_indexes) {
			g_BsKernel.CreateIndexBuffer(total_indexes*sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16 , D3DPOOL_MANAGED, &m_DetailLevel[2].TileBodies[body].pIndexBuffer);
			WORD* pIndex;
			m_DetailLevel[2].TileBodies[body].pIndexBuffer->Lock(0,total_indexes*2, (void**)&pIndex, D3DLOCK_NOSYSLOCK);
			int index=0;

			// start by copying the center portion of the tile
			for (int center_vert=0;center_vert<centerSqSize[1];++center_vert) {
				pIndex[index++] = _Level2_Center[center_vert];
			}

			for (int side=0;side<TOTAL_SIDES;++side) {
				if (!(body & (1<<side))) {
					for (int data=0;data<sideSize[1];++data) {
						pIndex[index++] = _SidesOfLevel2[side][data];
					}
				}
			}
			m_DetailLevel[2].TileBodies[body].pIndexBuffer->Unlock();
			m_DetailLevel[2].TileBodies[body].IndexCount = total_indexes;
			m_DetailLevel[2].TileBodies[body].TriangleCount = total_indexes/3;
		}
	}

	// create the tile connectors
	for (int side=0;side<TOTAL_SIDES;++side) {
		// connections to detail level 0
		m_DetailLevel[2].TileConnectors[side][0].pIndexBuffer = 0;
		g_BsKernel.CreateIndexBuffer(ConnectorLevel2Size[0]*sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16 , D3DPOOL_MANAGED, &m_DetailLevel[2].TileConnectors[side][0].pIndexBuffer);
		WORD* pIndex;
		m_DetailLevel[2].TileConnectors[side][0].pIndexBuffer->Lock(0,ConnectorLevel2Size[0]*2, (void**)&pIndex, D3DLOCK_NOSYSLOCK);
		for (int count=0;count<ConnectorLevel2Size[0];++count)
		{
			//pIndex[count] = Connect2to0[side][count];
			pIndex[count] = _ConnectorsLevel2[0][side][count];
		}
		m_DetailLevel[2].TileConnectors[side][0].pIndexBuffer->Unlock();
		m_DetailLevel[2].TileConnectors[side][0].IndexCount = ConnectorLevel2Size[0];
		m_DetailLevel[2].TileConnectors[side][0].TriangleCount = ConnectorLevel2Size[0]/3;

		// connections to detail level 1
		m_DetailLevel[2].TileConnectors[side][1].pIndexBuffer = 0;
		g_BsKernel.CreateIndexBuffer(ConnectorLevel2Size[1]*sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16 , D3DPOOL_MANAGED, &m_DetailLevel[2].TileConnectors[side][1].pIndexBuffer);
		pIndex = NULL;
		m_DetailLevel[2].TileConnectors[side][1].pIndexBuffer->Lock(0,ConnectorLevel2Size[1]*2, (void**)&pIndex, D3DLOCK_NOSYSLOCK);
		for (int count=0;count<ConnectorLevel2Size[1];++count)
		{
			//pIndex[count] = Connect2to1[side][count];
			pIndex[count] = _ConnectorsLevel2[1][side][count];
		}
		m_DetailLevel[2].TileConnectors[side][1].pIndexBuffer->Unlock();
		m_DetailLevel[2].TileConnectors[side][1].IndexCount = ConnectorLevel2Size[1];
		m_DetailLevel[2].TileConnectors[side][1].TriangleCount = ConnectorLevel2Size[1]/3;

		delete[] _ConnectorsLevel2[0][side];
		delete[] _ConnectorsLevel2[1][side];
	}

	for(int i=0;i<=level; i++) {
		delete[] _ConnectorsLevel2[i];
	}
	delete[] _ConnectorsLevel2;
	delete[] ConnectorLevel2Size;

	// level 3 (40 x 40)
	level = 2;
	WORD* _Level3_Center = new WORD[centerSqSize[level]];
	WORD* _SidesOfLevel3[TOTAL_SIDES];
	for (int i=0; i<TOTAL_SIDES; i++ )
		_SidesOfLevel3[i] = new WORD[sideSize[level]];

	GenerateCenterSq( _TILE_VERT, inc[level], _Level3_Center );
	GenerateSides( _TILE_VERT, inc[level], _SidesOfLevel3 );

	WORD*** _ConnectorsLevel3 = new WORD**[level + 1];
	int* ConnectorLevel3Size = new int[level + 1];
	connectingEdge = connectingEdgeBase;

	for (int i=0; i<=level; i++ ) {
		_ConnectorsLevel3[i] = new WORD*[TOTAL_SIDES];

		// num triangles = num sides in center sq + num sides in connecting edge
		int connectorSize = (dim[level] - 2);
		connectorSize += connectingEdge;
		connectorSize *= 3;		// three index per triangle

		ConnectorLevel3Size[i] = connectorSize;

		for (int j=0; j<TOTAL_SIDES; j++ )
			_ConnectorsLevel3[i][j] = new WORD[connectorSize];

		GenerateConnectors( _TILE_VERT, dim[level], connectingEdge, _ConnectorsLevel3[i] );

		connectingEdge *= 2;	// double for next level
	}

	// create each of the 16 tile bodies
	for (int body=0;body<16;++body) {
		m_DetailLevel[3].TileBodies[body].pIndexBuffer=0;
		m_DetailLevel[3].TileBodies[body].IndexCount = 0;

		int total_indexes=centerSqSize[2];
		if (!(body & (1<<0))) total_indexes += sideSize[2];
		if (!(body & (1<<1))) total_indexes += sideSize[2];
		if (!(body & (1<<2))) total_indexes += sideSize[2];
		if (!(body & (1<<3))) total_indexes += sideSize[2];

		if (total_indexes) {
			g_BsKernel.CreateIndexBuffer(total_indexes*sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16 , D3DPOOL_MANAGED, &m_DetailLevel[3].TileBodies[body].pIndexBuffer);
			{
				WORD* pIndex;
				m_DetailLevel[3].TileBodies[body].pIndexBuffer->Lock(0,total_indexes*2, (void**)&pIndex, D3DLOCK_NOSYSLOCK);
				{
					int index=0;

					// start by copying the center portion of the tile
					for (int center_vert=0;center_vert<centerSqSize[2];++center_vert)
					{
						pIndex[index++] = _Level3_Center[center_vert];
					}

					for (int side=0;side<TOTAL_SIDES;++side)
					{
						if (!(body & (1<<side)))
						{
							for (int data=0;data<sideSize[2];++data)
							{
								pIndex[index++] = _SidesOfLevel3[side][data];
							}
						}
					}
					m_DetailLevel[3].TileBodies[body].pIndexBuffer->Unlock();
					m_DetailLevel[3].TileBodies[body].IndexCount = total_indexes;
					m_DetailLevel[3].TileBodies[body].TriangleCount = total_indexes/3;
				}
			}
		}
	}

	// create the tile connectors
	for (int side=0;side<TOTAL_SIDES;++side) {
		// connections to detail level 0
		m_DetailLevel[3].TileConnectors[side][0].pIndexBuffer = 0;
		g_BsKernel.CreateIndexBuffer(ConnectorLevel3Size[0]*sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16 , D3DPOOL_MANAGED, &m_DetailLevel[3].TileConnectors[side][0].pIndexBuffer);
		{
			WORD* pIndex;
			m_DetailLevel[3].TileConnectors[side][0].pIndexBuffer->Lock(0,ConnectorLevel3Size[0]*2, (void**)&pIndex, D3DLOCK_NOSYSLOCK);
			{
				for (int count=0;count<ConnectorLevel3Size[0];++count)
				{
					//pIndex[count] = Connect3to0[side][count];
					pIndex[count] = _ConnectorsLevel3[0][side][count];
				}
				m_DetailLevel[3].TileConnectors[side][0].pIndexBuffer->Unlock();
				m_DetailLevel[3].TileConnectors[side][0].IndexCount = ConnectorLevel3Size[0];
				m_DetailLevel[3].TileConnectors[side][0].TriangleCount = ConnectorLevel3Size[0]/3;
			}
		}

		// connections to detail level 1
		m_DetailLevel[3].TileConnectors[side][1].pIndexBuffer = 0;
		g_BsKernel.CreateIndexBuffer(ConnectorLevel3Size[1]*2, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16 , D3DPOOL_MANAGED, &m_DetailLevel[3].TileConnectors[side][1].pIndexBuffer);
		{
			WORD* pIndex;
			m_DetailLevel[3].TileConnectors[side][1].pIndexBuffer->Lock(0,ConnectorLevel3Size[1]*2, (void**)&pIndex, D3DLOCK_NOSYSLOCK);
			{
				for (int count=0;count<ConnectorLevel3Size[1];++count)
				{
					//pIndex[count] = Connect3to1[side][count];
					pIndex[count] = _ConnectorsLevel3[1][side][count];

				}
				m_DetailLevel[3].TileConnectors[side][1].pIndexBuffer->Unlock();
				m_DetailLevel[3].TileConnectors[side][1].IndexCount = ConnectorLevel3Size[1];
				m_DetailLevel[3].TileConnectors[side][1].TriangleCount = ConnectorLevel3Size[1]/3;
			}
		}

		// connections to detail level 2
		m_DetailLevel[3].TileConnectors[side][2].pIndexBuffer = 0;
		g_BsKernel.CreateIndexBuffer(ConnectorLevel3Size[2]*2, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16 , D3DPOOL_MANAGED, &m_DetailLevel[3].TileConnectors[side][2].pIndexBuffer);
		{
			WORD* pIndex;
			m_DetailLevel[3].TileConnectors[side][2].pIndexBuffer->Lock(0,ConnectorLevel3Size[2]*2, (void**)&pIndex, D3DLOCK_NOSYSLOCK);
			{
				for (int count=0;count<ConnectorLevel3Size[2];++count)
				{
					//pIndex[count] = Connect3to2[side][count];
					pIndex[count] = _ConnectorsLevel3[2][side][count];
				}
				m_DetailLevel[3].TileConnectors[side][2].pIndexBuffer->Unlock();
				m_DetailLevel[3].TileConnectors[side][2].IndexCount = ConnectorLevel3Size[2];
				m_DetailLevel[3].TileConnectors[side][2].TriangleCount = ConnectorLevel3Size[2]/3;
			}
		}

		delete[] _ConnectorsLevel3[0][side];
		delete[] _ConnectorsLevel3[1][side];
		delete[] _ConnectorsLevel3[2][side];
	}

	for(int i=0;i<=level; i++) {
		delete[] _ConnectorsLevel3[i];
	}
	delete[] _ConnectorsLevel3;
	delete[] ConnectorLevel3Size;

	for (int i=0; i<TOTAL_SIDES; i++ ) {
		delete [] _SidesOfLevel1[i];
		delete [] _SidesOfLevel2[i];
		delete [] _SidesOfLevel3[i];
	}

	delete [] _Level1_Center;
	delete [] _Level2_Center;
	delete [] _Level3_Center;
}


void CBsLODWorld::Render(C3DDevice *pDevice)
{
	// Visible Zone Test!!
	TiledZoneVisibleTest();
    
	CBsShadowMgr* pShadowMgr = g_BsKernel.GetShadowMgr();

    CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(m_nHFMaterialIndex);

	pMaterial->BeginMaterial(GetTerrainTechnique(), 0);
	pMaterial->BeginPass(0);

	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);


	D3DXVECTOR4 vecViewLight;
	D3DXVec4Transform(&vecViewLight, CBsKernel::GetInstance().GetLightDirection(), g_BsKernel.GetParamViewMatrix());

	pMaterial->SetVector(m_hLightDir, &vecViewLight);
	pMaterial->SetVector(m_hLightDiffuse, CBsKernel::GetInstance().GetLightDiffuse());
	pMaterial->SetVector(m_hLightSpecular, CBsKernel::GetInstance().GetLightSpecular());
	pMaterial->SetVector(m_hLightAmbient, CBsKernel::GetInstance().GetLightAmbient());

	pMaterial->SetVector(m_hFogFactor, CBsKernel::GetInstance().GetParamFogFactor());
	pMaterial->SetVector(m_hFogColor, CBsKernel::GetInstance().GetParamFogColor());

	pMaterial->SetTexture(m_hLayer0Texture, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(m_nTextureIndex[0]));
	pMaterial->SetTexture(m_hLayer1Texture, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(m_nTextureIndex[1]));
	pMaterial->SetTexture(m_hLayer2Texture, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(m_nTextureIndex[2]));
	pMaterial->SetTexture(m_hLayer3Texture, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(m_nTextureIndex[3]));
	pMaterial->SetTexture(m_hLayer4Texture, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(m_nTextureIndex[4]));
	pMaterial->SetTexture(m_hBlendTexture, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(m_nTextureIndex[5]));

	if(m_nLightMapTexture<0) {
		pMaterial->SetTexture(m_hLightMap, g_BsKernel.GetBlankTexture());
	}
	else {
		pMaterial->SetTexture(m_hLightMap, m_nLightMapTexture );
	}

#ifdef _USAGE_TOOL_
	if(m_nAttributeTextureIndex != -1) {
		pMaterial->SetTexture(m_hAttributeMap, (LPDIRECT3DBASETEXTURE9)CBsKernel::GetInstance().GetTexturePtr(m_nAttributeTextureIndex));
	}
#endif
	pMaterial->SetTexture(m_hShadowTexture, pShadowMgr->GetShadowMap());

	float fOffsetX = 0.5f + (0.5f / (float)(pShadowMgr->GetShadowMapXSize()));
	float fOffsetY = 0.5f + (0.5f / (float)(pShadowMgr->GetShadowMapYSize()));
	float fRange = 1.f;
	D3DXMATRIX texScaleBiasMat( 0.5f,     0.0f,     0.0f,         0.0f,
								0.0f,    -0.5f,     0.0f,         0.0f,
								0.0f,     0.0f,     fRange,       0.0f,
								fOffsetX, fOffsetY, 0.0f,         1.0f );

	float fStartX, fStartZ;
	D3DXMATRIX matWorld, matWVP;

	//pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	{
		UINT nRenderTileCount = m_DrawZoneList.size();
		for(UINT uiOrder=0;uiOrder<nRenderTileCount;++uiOrder) {
			CBsTerrainZone* pTile=m_DrawZoneList[uiOrder];

			fStartX=pTile->nIndex%m_nZoneHoriCount*m_nXSizePerZone*DISTANCE_PER_CELL;
			fStartZ=(pTile->nIndex/m_nZoneHoriCount*m_nZSizePerZone)*DISTANCE_PER_CELL;
			D3DXMatrixTranslation(&matWorld, fStartX, 0.0f, fStartZ);
			pMaterial->SetMatrix(m_hWVP, D3DXMatrixMultiply(&matWVP, &matWorld, g_BsKernel.GetParamViewProjectionMatrix()));
			pMaterial->SetMatrix(m_hWV, D3DXMatrixMultiply(&matWVP, &matWorld, g_BsKernel.GetParamViewMatrix()));

			D3DXMATRIX matWorldViewIT;
			D3DXMatrixMultiply(&matWorldViewIT, &matWorld, CBsKernel::GetInstance().GetParamViewMatrix());
			D3DXMatrixInverse(&matWorldViewIT, NULL, &matWorldViewIT);
			D3DXMatrixTranspose(&matWorldViewIT, &matWorldViewIT);

			pMaterial->SetMatrix(m_hWVIT, &matWorldViewIT);

			// shadow !!
			D3DXMATRIX textureMatrix;
			D3DXMatrixMultiply(&textureMatrix, pShadowMgr->GetLightViewProject(), &texScaleBiasMat);
			D3DXMatrixMultiply(&textureMatrix, &matWorld,&textureMatrix);
			pMaterial->SetMatrix(m_hShadowMatrix, &textureMatrix );

			pMaterial->CommitChanges();

			pDevice->SetStreamSource( 0 , pTile->pZoneVB, sizeof(HFVERTEX));
			g_BsKernel.SetVertexDeclaration(m_nTerrainVertexDeclIndex);
			{
				int body_tile = 0;
				TERRAIN_LEVEL MyLevel = pTile->m_CurrentDetailLevel;
				//				int nIndex = pTile->nIndex;
				int nXIndex = pTile->nXIndex;
				int nZIndex = pTile->nZIndex;

				// examine the tile above this tile
				int nCompareIndex = (nZIndex+1)*m_nZoneHoriCount + nXIndex;
				if (nZIndex<(m_nZoneVertCount-1) && m_pTerrainZones[nCompareIndex].bIsRender && m_pTerrainZones[nCompareIndex].m_CurrentDetailLevel < MyLevel) {
					TERRAIN_LEVEL ThisLevel = m_pTerrainZones[nCompareIndex].m_CurrentDetailLevel;
					body_tile |= 1<<BOTTOM;

					// draw the connecting piece needed
					pDevice->SetIndices(m_DetailLevel[MyLevel].TileConnectors[BOTTOM][ThisLevel].pIndexBuffer);
					pDevice->DrawIndexedMeshVB(D3DPT_TRIANGLELIST, m_nVertexCountPerZone, m_DetailLevel[MyLevel].TileConnectors[BOTTOM][ThisLevel].TriangleCount, 0, 0);
				}

				// examine the tile below this tile
				nCompareIndex = (nZIndex-1)*m_nZoneHoriCount + nXIndex;
				if (nZIndex && m_pTerrainZones[nCompareIndex].bIsRender && m_pTerrainZones[nCompareIndex].m_CurrentDetailLevel < MyLevel)
				{
					TERRAIN_LEVEL ThisLevel = m_pTerrainZones[nCompareIndex].m_CurrentDetailLevel;
					body_tile |= 1<<TOP;

					// draw the connecting piece needed
					pDevice->SetIndices(m_DetailLevel[MyLevel].TileConnectors[TOP][ThisLevel].pIndexBuffer);
					pDevice->DrawIndexedMeshVB(D3DPT_TRIANGLELIST, m_nVertexCountPerZone, m_DetailLevel[MyLevel].TileConnectors[TOP][ThisLevel].TriangleCount, 0, 0);
				}

				// examine the tile to the left this tile
				nCompareIndex = nZIndex*m_nZoneHoriCount+nXIndex-1;
				if (nXIndex && m_pTerrainZones[nCompareIndex].bIsRender && m_pTerrainZones[nCompareIndex].m_CurrentDetailLevel < MyLevel)
				{
					TERRAIN_LEVEL ThisLevel = m_pTerrainZones[nCompareIndex].m_CurrentDetailLevel;
					body_tile |= 1<<LEFT;

					// draw the connecting piece needed
					pDevice->SetIndices(m_DetailLevel[MyLevel].TileConnectors[LEFT][ThisLevel].pIndexBuffer);
					pDevice->DrawIndexedMeshVB(D3DPT_TRIANGLELIST, m_nVertexCountPerZone, m_DetailLevel[MyLevel].TileConnectors[LEFT][ThisLevel].TriangleCount, 0, 0);
				}

				// examine the tile to the right this tile
				nCompareIndex = nZIndex*m_nZoneHoriCount+nXIndex+1;
				if (nXIndex<(m_nZoneHoriCount-1) && m_pTerrainZones[nCompareIndex].bIsRender && m_pTerrainZones[nCompareIndex].m_CurrentDetailLevel < MyLevel)
				{
					TERRAIN_LEVEL ThisLevel = m_pTerrainZones[nCompareIndex].m_CurrentDetailLevel;
					body_tile |= 1<<RIGHT;

					// draw the connecting piece needed
					pDevice->SetIndices(m_DetailLevel[MyLevel].TileConnectors[RIGHT][ThisLevel].pIndexBuffer);
					pDevice->DrawIndexedMeshVB(D3DPT_TRIANGLELIST, m_nVertexCountPerZone, m_DetailLevel[MyLevel].TileConnectors[RIGHT][ThisLevel].TriangleCount, 0, 0);
				}

				// finally, draw the body tile needed
				if (m_DetailLevel[MyLevel].TileBodies[body_tile].pIndexBuffer)
				{
					pDevice->SetIndices(m_DetailLevel[MyLevel].TileBodies[body_tile].pIndexBuffer);
					pDevice->DrawIndexedMeshVB(D3DPT_TRIANGLELIST, m_nVertexCountPerZone, m_DetailLevel[MyLevel].TileBodies[body_tile].TriangleCount, 0, 0);
				}
			}
		}
	}
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);

	pMaterial->EndPass();
	pMaterial->EndMaterial();

	m_DrawZoneList.clear();
}

void CBsLODWorld::TiledZoneVisibleTest()
{
	int nPosX, nPosY, nCurrentZoneIndex;

	const D3DXMATRIX* pCamera = g_BsKernel.GetParamInvViewMatrix();

	nPosX=(int)(pCamera->_41/DISTANCE_PER_CELL/m_nXSizePerZone);
	nPosY=(int)((pCamera->_43/DISTANCE_PER_CELL)/m_nZSizePerZone);
	nCurrentZoneIndex=nPosY*m_nZoneHoriCount+nPosX;
	// 지금은 모든 존을 Check!! by jeremy!!
	for(int i=0;i<m_nZoneVertCount;i++){
		for(int j=0;j<m_nZoneHoriCount;j++){
			int nIndex = i*m_nZoneHoriCount+j;
			if((nCurrentZoneIndex==i*m_nZoneHoriCount+j)||(IsVisibleTestFromFrustum(nIndex))){				
				
				m_pTerrainZones[nIndex].bIsRender = TRUE;

				if( m_nForceLODLevel < 0) {
					D3DXVECTOR3 vecDir = m_pTerrainZones[nIndex].vecCenter - *(D3DXVECTOR3*)&(pCamera->_41);
					m_pTerrainZones[nIndex].fCameraDistance = D3DXVec3Length(&vecDir);
					// Compute Zone's Level.................
					ComputeLevelOfZone(nIndex);
				}
				else {
					m_pTerrainZones[nIndex].m_CurrentDetailLevel = (TERRAIN_LEVEL)m_nForceLODLevel;			// Test !!!!!                    
				}				
				// Add Render Zone!!
				m_DrawZoneList.push_back(m_pTerrainZones+nIndex);
			}
			else {
				// No Visibility!!!
				m_pTerrainZones[nIndex].bIsRender = FALSE;
				m_pTerrainZones[nIndex].m_CurrentDetailLevel = LEVEL_0;			// Test !!!!!
			}
		}
	}
	// Render Tile들을 sort한다.
	std::sort( m_DrawZoneList.begin(), m_DrawZoneList.begin() + m_DrawZoneList.size(), CompareTile );
}

void CBsLODWorld::ComputeLevelOfZone(int ZoneIndex)
{

	if(m_pTerrainZones[ZoneIndex].fCameraDistance		<	30000.f) {
		m_pTerrainZones[ZoneIndex].m_CurrentDetailLevel = LEVEL_3;			// Test !!!!!
	}
	else if(m_pTerrainZones[ZoneIndex].fCameraDistance	<	50000.f) {
		m_pTerrainZones[ZoneIndex].m_CurrentDetailLevel = LEVEL_2;			// Test !!!!!
	}
	else if(m_pTerrainZones[ZoneIndex].fCameraDistance	<	80000.f) {
		m_pTerrainZones[ZoneIndex].m_CurrentDetailLevel = LEVEL_1;			// Test !!!!!
	}
	else {
		// 400미터 이상은 Lowest Level!!!!
		m_pTerrainZones[ZoneIndex].m_CurrentDetailLevel = LEVEL_0;			// Test !!!!!
	}
}	

void CBsLODWorld::RefreshZone(int nZoneIndex)
{
	WriteZoneData(m_pTerrainZones[nZoneIndex].pZoneVB, nZoneIndex);
}