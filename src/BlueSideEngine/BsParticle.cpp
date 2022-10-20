#include "StdAfx.h"
#include "BStreamExt.h"
#include "BsKernel.h"
#include "BsImageProcess.h"
#include "BsParticle.h"
#include "BsSintable.h"
#include "BsFXTable.h"
#include "BsFXTableValue.h"
#include "BsMaterial.h"

#ifdef _XBOX
extern DWORD g_dwRenderThreadId;
#endif

CBsCriticalSection CBsParticleObject::s_csSharedParticleData;

//#define USE_PARTICLEDBG
#if defined(USE_PARTICLEDBG)
#include "BsDebugBreak.h"
static Debug::DebugInfo s_particleDbgInfo[] =
{
	//{ "dust4archi_005.bpf", Debug::OnAll,     Debug::OnLoad    },
	//{ "FX_PTC_FIRE06.bpf",  Debug::OnAll,     Debug::OnNothing },
	//{ "dust4archi_002.bpf", Debug::OnAll,     Debug::OnLoad    },
	{ "",                   Debug::OnNothing, Debug::OnNothing }
};
#endif

int g_nBlendOP[]={
    D3DBLENDOP_ADD,
    D3DBLENDOP_SUBTRACT,
    D3DBLENDOP_REVSUBTRACT,
    D3DBLENDOP_MIN,
    D3DBLENDOP_MAX,
};

int g_nBlendMode[]={
	D3DBLEND_ZERO,
    D3DBLEND_ONE,             
    D3DBLEND_SRCCOLOR,        
    D3DBLEND_INVSRCCOLOR,     
    D3DBLEND_SRCALPHA,        
    D3DBLEND_INVSRCALPHA,     
    D3DBLEND_DESTALPHA,       
    D3DBLEND_INVDESTALPHA,    
    D3DBLEND_DESTCOLOR,       
    D3DBLEND_INVDESTCOLOR,    
    D3DBLEND_SRCALPHASAT,     
};



CBsParticleMember::CBsParticleMember()
{
	m_bDraw=true;
	m_nTextureIndex=-1;

	m_pVB=NULL;
	m_pIB=NULL;
	m_pIterateVB = NULL;
	m_pIterateIB = NULL;

	memset( &m_Param, 0, sizeof(PARTICLE_MEMBER_PARAM) );

	for( int i=0; i<TABLE_AMOUNT; i++ )
		m_pTable[i] = new CBsFXTable;
}

CBsParticleMember::~CBsParticleMember()
{
	ReleaseTexture();

	for( int i=0; i<TABLE_AMOUNT; i++ )
		delete m_pTable[i];

	Clear();
}

void CBsParticleMember::Clear()
{
	if(m_pVB){
		m_pVB->Release();
		m_pVB=NULL;
	}
	if(m_pIB){
		m_pIB->Release();
		m_pIB=NULL;
	}

	if( m_pIterateVB )
	{
		delete [] m_pIterateVB;
		m_pIterateVB = NULL;
	}
	if( m_pIterateIB )
	{
		delete [] m_pIterateIB;
		m_pIterateIB = NULL;
	}
}

void CBsParticleMember::ReleaseTexture()
{
	SAFE_RELEASE_TEXTURE(m_nTextureIndex);
}

void CBsParticleMember::GetParticleParam(PARTICLE_MEMBER_PARAM *pParam)
{
	memcpy(pParam, &m_Param, sizeof(PARTICLE_MEMBER_PARAM));
}

void CBsParticleMember::SetParticleParam(PARTICLE_MEMBER_PARAM *pParam)
{
	memcpy(&m_Param, pParam, sizeof(PARTICLE_MEMBER_PARAM));
}

PARTICLE_MEMBER_PARAM *CBsParticleMember::GetParticleParam()
{
	return &m_Param;
}

CBsFXTable *CBsParticleMember::GetTable( CBsParticleMember::TABLE_TYPE Type )
{
	return m_pTable[Type];
}



/*---------------------------------------------------------------------------------
-
-			CBsParticleMember::CreateParticleBuffer()
-					; m_Param.nParticleCount 만큼 파티클을 만들고 초기값을 세팅한다.
-					  VB, IB 만든다.
-
---------------------------------------------------------------------------------*/
void CBsParticleMember::CreateParticleBuffer()
{
	int i;
	WORD *pIndex;
	PARTICLEVERTEX *pVertex;
	D3DXVECTOR3 RotationAxis;
	D3DXMATRIX RotationMat;
	float fRotateAngle;

	Clear();

	GenerateTable();

	m_BirthTimeList.resize( m_Param.nParticleCount );		// 파티클 갯수만큼 생성시작시간이 다른건가?
	m_DirectionList.resize( m_Param.nParticleCount );		// 파티클 마다 방향은 다 다를수 있지.
	g_BsKernel.CreateVertexBuffer( m_Param.nParticleCount * sizeof( PARTICLEVERTEX ) * 4, D3DUSAGE_WRITEONLY, 
		0, D3DPOOL_MANAGED, &m_pVB );
	g_BsKernel.CreateIndexBuffer( m_Param.nParticleCount * sizeof( WORD ) * 3 * 2, 0, D3DFMT_INDEX16, 
		D3DPOOL_MANAGED, &m_pIB );
	// Iterate 파티클 버퍼 생성
	m_pIterateVB = new ITERATE_PARTICLE_VERTEX[ m_Param.nParticleCount * 4 ];
	m_pIterateIB = new WORD[ m_Param.nParticleCount * 3 * 2 ];

	m_pVB->Lock( 0, m_Param.nParticleCount * sizeof( PARTICLEVERTEX ) * 4, ( void ** )&pVertex, 0 );

	D3DXVec3Cross( &RotationAxis, &D3DXVECTOR3( 0.0f, 1.0f, 0.0f ), 
		&D3DXVECTOR3( m_Param.LaunchVector.x, m_Param.LaunchVector.y, m_Param.LaunchVector.z ) );
	fRotateAngle = acosf( D3DXVec3Dot( &D3DXVECTOR3( 0.0f, 1.0f, 0.0f ), 
		&D3DXVECTOR3( m_Param.LaunchVector.x, m_Param.LaunchVector.y, m_Param.LaunchVector.z ) ) );
	D3DXMatrixRotationAxis( &RotationMat, &RotationAxis, fRotateAngle );
	for( i = 0; i < m_Param.nParticleCount; i++ )
	{
		D3DXVECTOR3 LaunchVec;
		float fBirth, fLaunch;
		int nAngle;

		nAngle = ( int )( NUM_SINTABLE / 360.0f * m_Param.LaunchVector.w ); //m_Param.LaunchVector.w에는 어떤 값이 저장되어 있지?
		LaunchVec.x = RandomNumberInRange( -FSIN( nAngle ), FSIN( nAngle ) );
		LaunchVec.y = 1 - RandomNumberInRange( -FSIN( nAngle ), FSIN( nAngle ) );
		LaunchVec.z = RandomNumberInRange( -FSIN( nAngle ), FSIN( nAngle ) );
		D3DXVec3TransformNormal( &LaunchVec, &LaunchVec, &RotationMat );
		D3DXVec3Normalize( &LaunchVec, &LaunchVec );

		fBirth = ( i * ( m_Param.nCreationTime / ( float )m_Param.nParticleCount ) ) / m_Param.nLifeTime;
		m_BirthTimeList[ i ] = fBirth;
		fLaunch = RandomNumberInRange( m_Param.fLaunchSpeedMin, m_Param.fLaunchSpeedMax );
		LaunchVec *= fLaunch;
		m_DirectionList[ i ] = LaunchVec;
		pVertex[ i * 4 ].Direction = LaunchVec;
		pVertex[ i * 4 + 1 ].Direction = LaunchVec;
		pVertex[ i * 4 + 2 ].Direction = LaunchVec;
		pVertex[ i * 4 + 3 ].Direction = LaunchVec;

		pVertex[ i * 4 ].nVertexIndex = 0;
		pVertex[ i * 4 + 1 ].nVertexIndex = 1;
		pVertex[ i * 4 + 2 ].nVertexIndex = 2;
		pVertex[ i * 4 + 3 ].nVertexIndex = 3;

		pVertex[ i * 4 ].fBirthTime = fBirth;
		pVertex[ i * 4 + 1 ].fBirthTime = fBirth;
		pVertex[ i * 4 + 2 ].fBirthTime = fBirth;
		pVertex[ i * 4 + 3 ].fBirthTime = fBirth;

		if(m_Param.bUseRandomRotation)
		{
			if( rand() % 2 )
			{
				pVertex[ i * 4 ].nRandRot = 1;
				pVertex[ i * 4 + 1 ].nRandRot = 1;
				pVertex[ i * 4 + 2 ].nRandRot = 1;
				pVertex[ i * 4 + 3 ].nRandRot = 1;
			}
			else
			{
				pVertex[ i * 4 ].nRandRot = -1;
				pVertex[ i * 4 + 1 ].nRandRot = -1;
				pVertex[ i * 4 + 2 ].nRandRot = -1;
				pVertex[ i * 4 + 3 ].nRandRot = -1;
			}
		}
		else
		{
			pVertex[ i * 4 ].nRandRot = 1;
			pVertex[ i * 4 + 1 ].nRandRot = 1;
			pVertex[ i * 4 + 2 ].nRandRot = 1;
			pVertex[ i * 4 + 3 ].nRandRot = 1;
		}
	}
	for( i = 0; i < m_Param.nParticleCount * 4; i++ )
	{
		memcpy( m_pIterateVB + i, pVertex + i, sizeof( PARTICLEVERTEX ) );
		m_pIterateVB[ i ].Position = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
		m_pIterateVB[ i ].Direction2 = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
	}

	m_pVB->Unlock();

	m_pIB->Lock( 0, m_Param.nParticleCount * sizeof( WORD ) * 3 * 2, ( void ** )&pIndex, 0 );
	for( i = 0; i < m_Param.nParticleCount; i++ )
	{
		pIndex[ i * 6 ] = i * 4;
		pIndex[ i * 6 + 1 ] = i * 4 + 1;
		pIndex[ i * 6 + 2 ] = i * 4 + 2;
		pIndex[ i * 6 + 3 ] = i * 4 + 2;
		pIndex[ i * 6 + 4 ] = i * 4 + 1;
		pIndex[ i * 6 + 5 ] = i * 4 + 3;
	}
	memcpy( m_pIterateIB, pIndex, m_Param.nParticleCount * sizeof( WORD ) * 3 * 2 );
	m_pIB->Unlock();
}

void CBsParticleMember::SetParticlePosition( int nSavePos, int nTick, D3DXMATRIX *pParticleMat )
{
	int i, nIndex, nCount;
	ITERATE_SAVE Save;

	GetParticleIndex( nTick, nIndex, nCount );
	if( nIndex != -1 )
	{
		Save.Position = *( D3DXVECTOR3 * )( &pParticleMat->_41 );
		for( i = 0; i < nCount; i++ )
		{
			D3DXVec3TransformNormal( &Save.Direction, &( m_DirectionList[ nIndex + i ] ), pParticleMat );
			CBsKernel::GetInstance().SaveIteratePosition( nSavePos + nIndex + i, &Save );
		}
	}
}

void CBsParticleMember::GetParticleIndex(int nTick, int &nIndex, int &nCount)
{
	int i;
	float fTime1, fTime2;

	nTick %= m_Param.nLifeTime;
	fTime1 = nTick / ( float )m_Param.nLifeTime;
	fTime2 = ( nTick + 1 ) / ( float )m_Param.nLifeTime;
	nIndex = -1;
	nCount = 0;
	for( i = 0; i < m_Param.nParticleCount; i++ )
	{
		if( m_BirthTimeList[ i ] >= fTime1 )
		{
			if( m_BirthTimeList[ i ] < fTime2 )
			{
				if( nIndex == -1 )
				{
					nIndex = i;
				}
				nCount++;
			}
			else
			{
				return;
			}
		}
	}
}

void CBsParticleMember::CopyVertexPosition( int nSavePos )
{
	int i;
	ITERATE_SAVE *pSave;

	for( i = 0; i < m_Param.nParticleCount; i++ )
	{
		pSave = g_BsKernel.GetIteratePosition( nSavePos + i );
		if( pSave == NULL )
		{
			return;
		}
		m_pIterateVB[ i * 4 ].Position = pSave->Position;
		m_pIterateVB[ i * 4 ].Direction2 = pSave->Direction;
		m_pIterateVB[ i * 4 + 1 ].Position = pSave->Position;
		m_pIterateVB[ i * 4 + 1 ].Direction2 = pSave->Direction;
		m_pIterateVB[ i * 4 + 2 ].Position = pSave->Position;
		m_pIterateVB[ i * 4 + 2 ].Direction2 = pSave->Direction;
		m_pIterateVB[ i * 4 + 3 ].Position = pSave->Position;
		m_pIterateVB[ i * 4 + 3 ].Direction2 = pSave->Direction;
	}
}

void CBsParticleMember::RenderParticle( C3DDevice *pDevice, int nTick, int nStopTick, bool bLoop, bool bIterate, float fScale )
{
	int nStartIndex, nEndIndex, nDrawCount;
	float fFramePerParticle;

	if( ( m_pVB ) && ( m_bDraw ) )
	{
		fFramePerParticle = m_Param.nParticleCount / ( float )m_Param.nCreationTime;
		nStartIndex = ( int )( ceilf( ( nTick - m_Param.nLifeTime + 1 ) * fFramePerParticle ) );
		if( nStartIndex < 0 )
		{
			nStartIndex = 0;
		}
		if( bLoop )
		{
			if( ( nStartIndex >= m_Param.nParticleCount ) && ( nTick == nStopTick ) )
			{
				nStartIndex = 0;
			}
		}
		else
		{
			if( nStartIndex >= m_Param.nParticleCount )
			{
				return;
			}
		}
		nEndIndex = ( int )( ceilf( nStopTick * fFramePerParticle ) );
		nDrawCount = nEndIndex - nStartIndex;
		if( nDrawCount > m_Param.nParticleCount )
		{
			nDrawCount = m_Param.nParticleCount;
		}

		assert( nDrawCount <= m_Param.nParticleCount );

		if( nDrawCount > 0 )
		{
			pDevice->SetRenderState( D3DRS_BLENDOP, g_nBlendOP[ m_Param.nBlendOP ] );
			pDevice->SetRenderState( D3DRS_SRCBLEND, g_nBlendMode[ m_Param.nSourceBlendMode ] );
			pDevice->SetRenderState( D3DRS_DESTBLEND, g_nBlendMode[ m_Param.nDestBlendMode ] );

			if( !bIterate )
			{
				pDevice->SetStreamSource( 0, m_pVB, sizeof( PARTICLEVERTEX ) );
				pDevice->SetIndices( m_pIB );
			}
			if( nDrawCount <= m_Param.nParticleCount - nStartIndex )
			{
				if( !bIterate )
				{
					pDevice->DrawIndexedMeshVB( D3DPT_TRIANGLELIST, m_Param.nParticleCount * 4, nDrawCount * 2, 
						nStartIndex * 6, 0 );
				}
				else
				{
					pDevice->DrawIndexedPrimitiveUP( D3DPT_TRIANGLELIST, 0, m_Param.nParticleCount * 4, nDrawCount * 2, 
						m_pIterateIB, D3DFMT_INDEX16, m_pIterateVB, sizeof( ITERATE_PARTICLE_VERTEX ) );
				}
			}
			else
			{
				int nPrimCount;

				nPrimCount = m_Param.nParticleCount - nStartIndex;
				if( nPrimCount > 0 )
				{
					if( !bIterate )
					{
						pDevice->DrawIndexedMeshVB( D3DPT_TRIANGLELIST, m_Param.nParticleCount * 4, nPrimCount * 2, 
							nStartIndex * 6, 0 );
					}
					else
					{
						pDevice->DrawIndexedPrimitiveUP( D3DPT_TRIANGLELIST, 0, m_Param.nParticleCount * 4, 
							nPrimCount * 2, m_pIterateIB + nStartIndex * 6, D3DFMT_INDEX16, m_pIterateVB, sizeof( ITERATE_PARTICLE_VERTEX ) );
					}
				}
				if( bLoop )
				{
					nPrimCount = nDrawCount - m_Param.nParticleCount + nStartIndex;
					if( nPrimCount > 0 )
					{
						if( nPrimCount > m_Param.nParticleCount )
						{ 
							nPrimCount = m_Param.nParticleCount;
						}
						if( !bIterate )
						{
							pDevice->DrawIndexedMeshVB( D3DPT_TRIANGLELIST, m_Param.nParticleCount * 4, 
								nPrimCount * 2, 0, 0 );
						}
						else
						{
							pDevice->DrawIndexedPrimitiveUP( D3DPT_TRIANGLELIST, 0, m_Param.nParticleCount * 4, 
								nPrimCount * 2, m_pIterateIB, D3DFMT_INDEX16, m_pIterateVB, sizeof( ITERATE_PARTICLE_VERTEX ) );
						}
					}
				}
			}
		}
	}
}

void CBsParticleMember::GenerateTable()
{
	int i;
	int nColorKeyCount, nAlphaKeyCount, nScaleKeyCount;

	nColorKeyCount = ( int )m_pTable[ COLOR_TABLE ]->GetCount();
	nAlphaKeyCount = ( int )m_pTable[ ALPHA_TABLE ]->GetCount();
	nScaleKeyCount = ( int )m_pTable[ SCALE_TABLE ]->GetCount();

	if( !nColorKeyCount || !nAlphaKeyCount || !nScaleKeyCount )
	{
		for( i = 0; i < PATICLE_TABLE_COUNT; i++ )
		{
			m_ColorTable[ i ].r = 1.0f;
			m_ColorTable[ i ].g = 1.0f;
			m_ColorTable[ i ].b = 1.0f;
			m_ColorTable[ i ].a = 1.0f;

			m_ScaleTable[ i ] = 50.f;
		}
		return;
	}

	for( i = 0; i < PATICLE_TABLE_COUNT; i++ ) 
	{
		float fKey = i * ( 1.f / ( PATICLE_TABLE_COUNT - 1 ) );

		// Color
		D3DXVECTOR3 *pColor = ( D3DXVECTOR3 * )m_pTable[ COLOR_TABLE ]->GetInterpolation( fKey, 
			CBsFXTable::IP_Vec3_Value_Linear );
		BsAssert( NULL != pColor );	// mruete: prefix bug 348: added assert
		m_ColorTable[ i ].r = pColor->x;
		m_ColorTable[ i ].g = pColor->y;
		m_ColorTable[ i ].b = pColor->z;

		// Alpha
		float *fpAlpha = ( float * )m_pTable[ ALPHA_TABLE ]->GetInterpolation( fKey, 
			CBsFXTable::IP_Float_Value_Linear );
		BsAssert( NULL != fpAlpha );	// mruete: prefix bug 349: added assert
		m_ColorTable[ i ].a = *fpAlpha;

		// Scale
		float *fpScale = ( float * )m_pTable[ SCALE_TABLE ]->GetInterpolation( fKey, 
			CBsFXTable::IP_Float_Value_Linear );
		BsAssert( NULL != fpScale );	// mruete: prefix bug 350: added assert
		m_ScaleTable[ i ] = *fpScale;
	}
}

void CBsParticleMember::LoadParticleMember( BStream* stream, const char *pFileName, int nVersion )
{
	int i;

	if( nVersion < 12 ) // 임시
	{	
		stream->Read( &m_Param, sizeof( PARTICLE_MEMBER_PARAM ) - sizeof( BOOL )* 2, ENDIAN_FOUR_BYTE );

		for( i = 0; i < TABLE_AMOUNT; i++ ) 
		{
			m_pTable[ i ]->Reset();
			m_pTable[ i ]->Load( stream );
		}

		stream->Seek( PARTICLE_MEMBER_PARAM_RESERVED + sizeof( BOOL ), BStream::fromNow );
	}
	else 
	{
		stream->Read( &m_Param, sizeof( PARTICLE_MEMBER_PARAM ), ENDIAN_FOUR_BYTE );
		stream->Seek( PARTICLE_MEMBER_PARAM_RESERVED, BStream::fromNow );

		for( i = 0; i < TABLE_AMOUNT; i++ ) 
		{
			m_pTable[ i ]->Reset();
			m_pTable[ i ]->Load( stream );
		}
	}

	if( strcmp( pFileName, "" ) != NULL )
		LoadTexture( pFileName );

	CreateParticleBuffer();
}

void CBsParticleMember::LoadTexture( const char *pFileName )
{
	ReleaseTexture();
	// Add by Siva
	char *pFullName = g_BsKernel.GetFullName( pFileName );
	
#if defined(_PACKED_RESOURCES)
	m_nTextureIndex = CBsKernel::GetInstance().LoadTexture( pFileName );
#else
	D3DRESOURCETYPE Type;
	UINT Depth;

	D3DXIMAGE_INFO ImageInfo;
	D3DXGetImageInfoFromFile( pFullName, &ImageInfo );
	Type = ImageInfo.ResourceType;
	Depth = ImageInfo.Depth;

	switch( Type ) 
	{
		case D3DRTYPE_TEXTURE:
			m_nTextureIndex = CBsKernel::GetInstance().LoadTexture( pFileName );
			break;
		case D3DRTYPE_VOLUMETEXTURE:
			if( Depth > 0 )
				m_nTextureIndex = CBsKernel::GetInstance().LoadVolumeTexture( pFileName );
			else m_nTextureIndex = CBsKernel::GetInstance().LoadTexture( pFileName );
			break;
		case D3DRTYPE_CUBETEXTURE:
			m_nTextureIndex = CBsKernel::GetInstance().LoadCubeTexture( pFileName );
			break;
		default:
			m_nTextureIndex = CBsKernel::GetInstance().LoadTexture( pFileName );
			break;
	}
#endif
}

void CBsParticleMember::SaveParticleMember(BStream* stream)
{
	char cDummy[ PARTICLE_MEMBER_PARAM_RESERVED ];

	memset( cDummy, 0, PARTICLE_MEMBER_PARAM_RESERVED );

	stream->Write( &m_Param, sizeof( PARTICLE_MEMBER_PARAM ), ENDIAN_FOUR_BYTE );

	stream->Write( cDummy, PARTICLE_MEMBER_PARAM_RESERVED );

	for( int i = 0; i < TABLE_AMOUNT; i++ )
		m_pTable[ i ]->Save( stream );
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

int	CBsParticleGroup::s_nNormalDeclIndex = -1;
int	CBsParticleGroup::s_nIterateDeclIndex = -1;
int			CBsParticleGroup::s_nParticleMaterialIndex = -1;
D3DXVECTOR4 CBsParticleGroup::s_VertexData[ 4 ] = 
{ 
	D3DXVECTOR4( -0.5f, 0.5f, 0.0f, 0.0f ), 
	D3DXVECTOR4( 0.5f, 0.5f, 0.0f, 0.0f ), 
	D3DXVECTOR4( -0.5f, -0.5f, 0.0f, 0.0f ), 
	D3DXVECTOR4( 0.5f, -0.5f, 0.0f, 0.0f ) 
};
D3DXVECTOR4 CBsParticleGroup::s_UVTable[ 4 ] = 
{ 
	D3DXVECTOR4( 0.0f, 0.0f, 0.0f, 0.0f ), 
	D3DXVECTOR4( 1.0f, 0.0f, 0.0f, 0.0f ), 
	D3DXVECTOR4( 0.0f, 1.0f, 0.0f, 0.0f ), 
	D3DXVECTOR4( 1.0f, 1.0f, 0.0f, 0.0f ) 
};
D3DXVECTOR4 CBsParticleGroup::s_ConstantValue = D3DXVECTOR4( 64.0f, 1.0f/64.0f, 4.0f, 0.0f );
D3DXVECTOR4 CBsParticleGroup::s_SinTable[ 64 ];

D3DXHANDLE CBsParticleGroup::s_hView = NULL;
D3DXHANDLE CBsParticleGroup::s_hWorldView = NULL;
D3DXHANDLE CBsParticleGroup::s_hProjection = NULL;
D3DXHANDLE CBsParticleGroup::s_hParticleTime = NULL;
D3DXHANDLE CBsParticleGroup::s_hResistTime = NULL;
D3DXHANDLE CBsParticleGroup::s_hResistScale = NULL;
D3DXHANDLE CBsParticleGroup::s_hLifeTime = NULL;
D3DXHANDLE CBsParticleGroup::s_hGravityAccel = NULL;
D3DXHANDLE CBsParticleGroup::s_hGravityVec = NULL;
D3DXHANDLE CBsParticleGroup::s_hOrigin = NULL;

D3DXHANDLE CBsParticleGroup::s_hRotateStart = NULL;
D3DXHANDLE CBsParticleGroup::s_hRotateRange = NULL;
D3DXHANDLE CBsParticleGroup::s_hColorTable = NULL;
D3DXHANDLE CBsParticleGroup::s_hScaleTable = NULL;

D3DXHANDLE CBsParticleGroup::s_hParticleScale = NULL;
D3DXHANDLE CBsParticleGroup::s_hVertexOffset = NULL;
D3DXHANDLE CBsParticleGroup::s_hUVTable = NULL;
D3DXHANDLE CBsParticleGroup::s_hSinTable = NULL;
D3DXHANDLE CBsParticleGroup::s_hParticleColor = NULL;
D3DXHANDLE CBsParticleGroup::s_hTextureRepeat = NULL;
D3DXHANDLE CBsParticleGroup::s_hTexture = NULL;
D3DXHANDLE CBsParticleGroup::s_hTextureAlpha = NULL;

D3DXHANDLE CBsParticleGroup::s_hTechniqueNormal = NULL;
D3DXHANDLE CBsParticleGroup::s_hTechniqueIterate = NULL;
D3DXHANDLE CBsParticleGroup::s_hTechniqueNormalAlpha = NULL;
D3DXHANDLE CBsParticleGroup::s_hTechniqueIterateAlpha = NULL;


CBsParticleGroup::CBsParticleGroup()
{
	m_nRefCount = 1;
	m_nTotalParticleCount = 0;
	m_nMaxParticleLife = 0;
}

CBsParticleGroup::~CBsParticleGroup()
{
#if defined(USE_PARTICLEDBG)
	char const * particleName = GetParticleFileName();
	Debug::TriggerEvent( s_particleDbgInfo, particleName, m_nRefCount, Debug::OnDelete );
#endif

	Clear();
}

void CBsParticleGroup::Clear()
{
	std::for_each( m_ParticleMembers.begin(), m_ParticleMembers.end(), std_delete() );
	m_ParticleMembers.clear();
}

void CBsParticleGroup::AddRef()
{
	int nNewRefCount = InterlockedIncrement( reinterpret_cast<volatile long*>(&m_nRefCount) );

#if defined(USE_PARTICLEDBG)
	char const * particleName = GetParticleFileName();
	Debug::TriggerEvent( s_particleDbgInfo, particleName, nNewRefCount, Debug::OnAddRef );
#endif
}

int CBsParticleGroup::Release()
{
	int nNewRefCount = InterlockedDecrement( reinterpret_cast<volatile long*>(&m_nRefCount) );

	// If this assert fires then there were too many Release calls on this object
	BsAssert(nNewRefCount >= 0);

#if defined(USE_PARTICLEDBG)
	char const * particleName = GetParticleFileName();
	Debug::TriggerEvent( s_particleDbgInfo, particleName, nNewRefCount, Debug::OnRelease );
#endif

	return nNewRefCount;
}

void CBsParticleGroup::SetParticleFileName(const char * pFileName)
{
	m_szFileName=pFileName;

#if defined(USE_PARTICLEDBG)
	char const * particleName = GetParticleFileName();
	Debug::TriggerEvent( s_particleDbgInfo, particleName, m_nRefCount, Debug::OnAddRef );
#endif
}

const char *CBsParticleGroup::GetParticleFileName()
{ 
	char *pFindPtr;

	pFindPtr = ( char * )strrchr( m_szFileName.c_str(), '\\' );
	if( pFindPtr )
	{
		return pFindPtr + 1;
	}
	else
	{
		return m_szFileName.c_str(); 
	}
}

void CBsParticleGroup::CreateEffect()
{
	char szFullName[ _MAX_PATH ];

	if( s_nParticleMaterialIndex != -1 ) {
		return;
	}

	strcpy( szFullName, CBsKernel::GetInstance().GetShaderDirectory() );
	strcat( szFullName, "ParticleNormal.fx" );

	s_nParticleMaterialIndex = g_BsKernel.LoadMaterial(szFullName, FALSE);

	D3DVERTEXELEMENT9 VertexElement[]=
	{
		{ 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		{ 0, 16, D3DDECLTYPE_SHORT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0 },
		D3DDECL_END()
	};
	D3DVERTEXELEMENT9 IterateElement[]=
	{
		{ 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		{ 0, 16, D3DDECLTYPE_SHORT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0 },
		{ 0, 20, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 32, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 1 },
		D3DDECL_END()
	};

	s_nNormalDeclIndex = g_BsKernel.LoadVertexDeclaration(VertexElement);
	s_nIterateDeclIndex = g_BsKernel.LoadVertexDeclaration(IterateElement);

	CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(s_nParticleMaterialIndex);
#ifdef _XBOX
	s_hView			=	pMaterial->GetParameterByName( "View" );
	s_hWorldView	=	pMaterial->GetParameterByName( "WorldView" );
	s_hProjection	=	pMaterial->GetParameterByName( "Projection" );
	s_hParticleTime	=	pMaterial->GetParameterByName( "TimeValue0" );
	s_hResistTime	=	pMaterial->GetParameterByName( "TimeValue1" );
	s_hResistScale	=	pMaterial->GetParameterByName( "TimeValue2" );
	s_hLifeTime		=	pMaterial->GetParameterByName( "TimeValue3" );
	s_hGravityAccel	=	pMaterial->GetParameterByName( "FloatValue0" );
	s_hGravityVec	=	pMaterial->GetParameterByName( "Vector4Value0" );
	s_hOrigin		=	pMaterial->GetParameterByName( "Vector4Value1" );
	s_hRotateStart	=	pMaterial->GetParameterByName( "FloatValue1" );
	s_hRotateRange	=	pMaterial->GetParameterByName( "FloatValue2" );
	s_hScaleTable	=	pMaterial->GetParameterByName( "FloatValue3" );
	s_hColorTable	=	pMaterial->GetParameterByName( "Vector4Value2" );
	s_hParticleScale=	pMaterial->GetParameterByName( "FloatValue4" );
	s_hVertexOffset	=	pMaterial->GetParameterByName( "FloatValue" );
	s_hUVTable		=	pMaterial->GetParameterByName( "FloatValue5" );
	s_hSinTable		=	pMaterial->GetParameterByName( "FloatValue6" );
	s_hParticleColor=	pMaterial->GetParameterByName( "Vector4Value3" );
	s_hTextureRepeat=	pMaterial->GetParameterByName( "FloatValue7" );
	s_hTexture		=	pMaterial->GetParameterByName( "TextureSampler" );
	s_hTextureAlpha	=	pMaterial->GetParameterByName( "ScreenSampler" );
#else
	s_hView			=	pMaterial->GetParameterByName( "view" );
	s_hWorldView	=	pMaterial->GetParameterByName( "WorldViewMat" );
	s_hProjection	=	pMaterial->GetParameterByName( "projection" );
	s_hParticleTime	=	pMaterial->GetParameterByName( "fParticleTime" );
	s_hResistTime	=	pMaterial->GetParameterByName( "fResistTime" );
	s_hResistScale	=	pMaterial->GetParameterByName( "fResistLength" );
	s_hLifeTime		=	pMaterial->GetParameterByName( "fLifeTime" );
	s_hGravityAccel	=	pMaterial->GetParameterByName( "fGravityAccel" );
	s_hGravityVec	=	pMaterial->GetParameterByName( "GravityVec" );
	s_hOrigin		=	pMaterial->GetParameterByName( "Origin" );
	s_hRotateStart	=	pMaterial->GetParameterByName( "fSpinStart" );
	s_hRotateRange	=	pMaterial->GetParameterByName( "fSpinRange" );
	s_hScaleTable	=	pMaterial->GetParameterByName( "fScaleTable" );
	s_hColorTable	=	pMaterial->GetParameterByName( "ColorTable" );
	s_hParticleScale=	pMaterial->GetParameterByName( "fParticleScale" );
	s_hVertexOffset	=	pMaterial->GetParameterByName( "VertexOffset" );
	s_hUVTable		=	pMaterial->GetParameterByName( "UVTable" );
	s_hSinTable		=	pMaterial->GetParameterByName( "SinTable" );
	s_hParticleColor=	pMaterial->GetParameterByName( "ParticleColor" );
	s_hTextureRepeat=	pMaterial->GetParameterByName( "fTextureRepeat" );
	s_hTexture		=	pMaterial->GetParameterByName( "diffuseTexture" );
	s_hTextureAlpha	=	pMaterial->GetParameterByName( "BackBuffer" );
#endif

	s_hTechniqueNormal = pMaterial->GetTechniqueHandle( 0 );
	s_hTechniqueIterate = pMaterial->GetTechniqueHandle( 1 );
	s_hTechniqueNormalAlpha = pMaterial->GetTechniqueHandle( 2 );
	s_hTechniqueIterateAlpha = pMaterial->GetTechniqueHandle( 3 );

	int i, nIndex;

	for( i = 0; i < 64; ++i )
	{
		nIndex = NUM_SINTABLE / 64 * i;
		s_SinTable[ i ] = D3DXVECTOR4( FCOS( nIndex ), -FSIN( nIndex ), FSIN( nIndex ), FCOS( nIndex ) );
	}
}

void CBsParticleGroup::DeleteEffect()
{
	SAFE_RELEASE_VD(s_nNormalDeclIndex);
	SAFE_RELEASE_VD(s_nIterateDeclIndex);

	SAFE_RELEASE_MATERIAL(s_nParticleMaterialIndex);
}

void CBsParticleGroup::RecreateParticleBuffer()
{
	int i, nSize;

	nSize = ( int )m_ParticleMembers.size();
	m_nTotalParticleCount = 0;
	for( i = 0; i < nSize; i++ )
	{
		m_nTotalParticleCount += m_ParticleMembers[ i ]->GetParticleCount();
		m_ParticleMembers[ i ]->CreateParticleBuffer();
	}
}
void CBsParticleGroup::RecalcTotalParticleCount()
{
	int nSize = ( int )m_ParticleMembers.size();
	m_nTotalParticleCount = 0;
	for(int i = 0; i < nSize; i++)
	{
		m_nTotalParticleCount += m_ParticleMembers[ i ]->GetParticleCount();
	}
}

void CBsParticleGroup::SaveParticleGroup( BStream* stream )
{
	int i, nSize;
	BPF_FILE_HEADER Header;
	char cDummy[ BPF_FILE_HEADER_RESERVED ];

	nSize = ( int )m_ParticleMembers.size();
	memset( cDummy, 0, BPF_FILE_HEADER_RESERVED );
	memset( &Header, 0, sizeof( BPF_FILE_HEADER ) );
	Header.nVersion = BPF_FILE_HEADER_VERSION;
	Header.nMemberCount = nSize;
	stream->Write( &Header, sizeof( BPF_FILE_HEADER ), ENDIAN_FOUR_BYTE );
	stream->Write( cDummy, BPF_FILE_HEADER_RESERVED );

	char szFileName[ _MAX_PATH ];
	for( i = 0; i < nSize; i++ )
	{
#if 0 // old code
		memset( szFileName, 0, _MAX_PATH );
		strcpy( szFileName, m_szTextureNameList[ i ].c_str() );
		char * pFindPtr = strrchr( szFileName, '\\' );
		if( pFindPtr )
		{
			stream->Write( pFindPtr + 1, _MAX_PATH );
		}
		else
		{
			stream->Write( szFileName, _MAX_PATH );
		}
#else // new code // mruete: prefix bug 354: rewrote it for better safety
		memset( szFileName, 0, _MAX_PATH );
		const char * str = m_szTextureNameList[ i ].c_str();
		const char * pFindStr = strrchr( str, '\\' );
		pFindStr = pFindStr ? (pFindStr+1) : str;
		strncpy_s( szFileName, _MAX_PATH, pFindStr, _TRUNCATE );
		stream->Write( szFileName, _MAX_PATH );
#endif	
		m_ParticleMembers[ i ]->SaveParticleMember( stream );
	}
}

int CBsParticleGroup::LoadParticleGroup(BStream* stream)
{
	int i;
	BPF_FILE_HEADER Header;
	char szTextureName[ _MAX_PATH ];

	Clear();
	stream->Read( &Header, sizeof( BPF_FILE_HEADER ), ENDIAN_FOUR_BYTE );

	/*
	if(Header.nVersion<BPF_FILE_HEADER_VERSION){
		return 0;
	}
	*/

	stream->Seek( BPF_FILE_HEADER_RESERVED, BStream::fromNow );
	m_ParticleMembers.resize( Header.nMemberCount );
	for( i = 0; i < Header.nMemberCount; i++ )
	{
		m_ParticleMembers[ i ] = new CBsParticleMember();
		stream->Read( szTextureName, _MAX_PATH );

/*
		// Add by yooty
#if defined(_XBOX) && defined(_PACKED_RESOURCES)
		char* pDot = strstr(szTextureName, ".");
		if(pDot != NULL)
		{
			*pDot = NULL;
			strcat_s(szTextureName, _countof(szTextureName), ".36t");
		}
#endif
*/
		m_ParticleMembers[ i ]->LoadParticleMember( stream, szTextureName, Header.nVersion );
		m_szTextureNameList.push_back( szTextureName );
//		nTexIndex=CBsKernel::GetInstance().LoadTexture(szTextureName);
//		m_ParticleMembers[i]->SetTextureIndex(nTexIndex);
	}
	m_nMaxParticleLife = 0;
	m_nTotalParticleCount = 0;
	for( i = 0; i < Header.nMemberCount; i++ )
	{
		m_nTotalParticleCount += m_ParticleMembers[ i ]->GetParticleCount();
		if( m_nMaxParticleLife < m_ParticleMembers[ i ]->GetLifeTime() )
		{
			m_nMaxParticleLife = m_ParticleMembers[ i ]->GetLifeTime();
		}
	}

	return 1;
}

std::vector< std::string > *CBsParticleGroup::GetTextureNameList()
{
	return &m_szTextureNameList;
}

int CBsParticleGroup::AddParticleMember( PARTICLE_MEMBER_PARAM *pDefaultParam/* =NULL */ )
{
	int nSize;
	PARTICLE_MEMBER_PARAM Param;

	nSize = ( int )m_ParticleMembers.size() + 1;
	m_ParticleMembers.resize( nSize );
	m_ParticleMembers[ nSize - 1 ] = new CBsParticleMember();
	if( pDefaultParam )
	{
		m_ParticleMembers[ nSize - 1 ]->SetParticleParam( pDefaultParam );
	}
	m_szTextureNameList.resize( nSize );
	RecreateParticleBuffer();

	return nSize - 1;
}

int CBsParticleGroup::CopyParticleMember( int nIndex )
{
	int nSize;
	PARTICLE_MEMBER_PARAM Param;

	AddParticleMember();
	nSize = ( int )m_ParticleMembers.size();
	m_ParticleMembers[ nIndex ]->GetParticleParam( &Param );
	m_ParticleMembers[ nSize - 1 ]->SetParticleParam( &Param );
	m_ParticleMembers[ nSize - 1 ]->SetTextureIndex( m_ParticleMembers[ nIndex ]->GetTextureIndex() );
	g_BsKernel.AddTextureRef( m_ParticleMembers[ nIndex ]->GetTextureIndex() );
	RecreateParticleBuffer();

	return nSize - 1;
}

void CBsParticleGroup::DeleteParticleMember( int nIndex )
{
	int nCount, nSize;
	CBsParticleMember *pMember = NULL;
	PARTICLE_MEMBER_PARAM Param;

	nSize = ( int )m_ParticleMembers.size();
	nCount = 0;
	if( nSize )
	{
		delete m_ParticleMembers[ nIndex ];
		m_ParticleMembers.erase( m_ParticleMembers.begin() + nIndex );
		m_szTextureNameList.erase( m_szTextureNameList.begin() + nIndex );
	}
}

void CBsParticleGroup::ChangeParticleMember( int nIndex1, int nIndex2 )
{
	PARTICLE_MEMBER_PARAM Param1, Param2;
	int nTextureIndex1, nTextureIndex2;

	m_ParticleMembers[ nIndex1 ]->GetParticleParam( &Param1 );
	nTextureIndex1 = m_ParticleMembers[ nIndex1 ]->GetTextureIndex();
	m_ParticleMembers[ nIndex2 ]->GetParticleParam( &Param2 );
	nTextureIndex2 = m_ParticleMembers[ nIndex2 ]->GetTextureIndex();

	m_ParticleMembers[ nIndex1 ]->SetParticleParam( &Param2 );
	m_ParticleMembers[ nIndex1 ]->SetTextureIndex( nTextureIndex2 );
	m_ParticleMembers[ nIndex2 ]->SetParticleParam( &Param1 );
	m_ParticleMembers[ nIndex2 ]->SetTextureIndex( nTextureIndex1 );
}

void CBsParticleGroup::DeleteAllParticleMember()
{
	std::for_each( m_ParticleMembers.begin(), m_ParticleMembers.end(), std_delete() );
	m_ParticleMembers.clear();
	m_szTextureNameList.clear();
}

CBsParticleMember *CBsParticleGroup::GetParticleMember( int nIndex )
{
	if( nIndex < 0 || nIndex >= ( int )m_ParticleMembers.size() ) 
		return NULL;

	return m_ParticleMembers[ nIndex ];
}

void CBsParticleGroup::LoadTexture( int nIndex, const char *pFileName ) 
{
	m_szTextureNameList[ nIndex ] = pFileName;

	m_ParticleMembers[ nIndex ]->LoadTexture( pFileName );
}

void CBsParticleGroup::SetCommonParticleConstant()
{
	CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(s_nParticleMaterialIndex);

	pMaterial->SetVectorArray( s_hVertexOffset, s_VertexData, 4 );
	pMaterial->SetVectorArray( s_hUVTable, s_UVTable, 4 );
	pMaterial->SetVectorArray( s_hSinTable, s_SinTable, 64 );
}

void CBsParticleGroup::SetParticleMemberConstant( float fScale, int nMemberIndex, PARTICLE_MEMBER_PARAM *pParam )
{
	D3DXVECTOR4 Parameter;
	float fRotateRange;

	CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(s_nParticleMaterialIndex);

	pMaterial->SetFloat( s_hGravityAccel, pParam->fGravityAccel * fScale );
	Parameter=D3DXVECTOR4( pParam->GravityVec.x, pParam->GravityVec.y, pParam->GravityVec.z, 0.0f );
	pMaterial->SetVector( s_hGravityVec, &Parameter );

	pMaterial->SetFloat( s_hRotateStart, pParam->fRotateStart );
	fRotateRange = pParam->fRotateEnd-pParam->fRotateStart;
	pMaterial->SetFloat( s_hRotateRange, fRotateRange );

	pMaterial->SetFloat( s_hResistTime, pParam->fResistTime );
	pMaterial->SetFloat( s_hResistScale, pParam->fResistRate );

	float *pScale;
	D3DCOLORVALUE *pColor;
	pScale = GetScaleTable( nMemberIndex );
	pColor = GetColorTable( nMemberIndex );
	pMaterial->SetFloatArray( s_hScaleTable, GetScaleTable( nMemberIndex ), PATICLE_TABLE_COUNT );
	pMaterial->SetVectorArray( s_hColorTable, ( D3DXVECTOR4 * )GetColorTable( nMemberIndex ), PATICLE_TABLE_COUNT );

	pMaterial->SetFloat( s_hTextureRepeat, ( float )pParam->nTextureRepeat );
}

void CBsParticleGroup::SetParticleGroupConstant( D3DXMATRIX *pMatParticle, D3DCOLORVALUE *pColor )
{
	D3DXMATRIX MatWorld, MatView, MatProj, MatTransposeView;
	D3DXVECTOR4 Origin;

	CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(s_nParticleMaterialIndex);

	memcpy( &MatView, CBsKernel::GetInstance().GetParamViewMatrix(), sizeof( D3DXMATRIX ) );
	pMaterial->SetMatrix( s_hView, &MatView );

	D3DXVec4Transform( &Origin, ( D3DXVECTOR4 * )&( pMatParticle->_41 ), &MatView );
	D3DXMatrixIdentity( &MatWorld );
	memcpy( &MatWorld, pMatParticle, sizeof( float ) * 12 );
	D3DXMatrixMultiply( &MatView, &MatWorld, &MatView );

	pMaterial->SetMatrix( s_hWorldView, &MatView );
	pMaterial->SetMatrix( s_hProjection, CBsKernel::GetInstance().GetParamProjectionMatrix() );
	pMaterial->SetVector( s_hOrigin, &Origin );

	D3DCOLORVALUE Color;

	Color = *pColor;
	if( m_bUseAlphaFog )
	{
		float fFogValue = ( g_BsKernel.GetActiveCamera()->GetFogEnd() - Origin.z ) / ( g_BsKernel.GetActiveCamera()->GetFogEnd() - g_BsKernel.GetActiveCamera()->GetFogStart() );
		if( fFogValue >= 1.0f ) fFogValue = 1.0f;
		Color.a *= fFogValue;
	}
	pMaterial->SetVector( s_hParticleColor, ( D3DXVECTOR4 * )&Color );
}

void CBsParticleGroup::RenderParticle( C3DDevice *pDevice, D3DXMATRIX *pMatParticle, int nTick, int nStopTick,
									  bool bLoop, bool bIterate, int nSavePos, D3DCOLORVALUE *pColor, float fScale )
{
	int i, nSize, nCurrentTick, nCurrentStopTick, nParticleCount;
	bool bApplyIterate;
	float fLifeTime, fParticleTime;
	D3DXVECTOR4 Time;

	CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(s_nParticleMaterialIndex);

	nSize = ( int )m_ParticleMembers.size();
	if( nSize )
	{
		PARTICLE_MEMBER_PARAM Param;
		nParticleCount = 0;
		for( i = 0; i < nSize; i++ )
		{
			if( !m_ParticleMembers[ i ]->IsDraw() )
			{
				continue;
			}
			if( !bLoop )
			{
				if( nTick >= m_ParticleMembers[ i ]->GetLifeTime() * 2 )
				{
					continue;
				}
			}
			if( nTick - nStopTick >= m_ParticleMembers[ i ]->GetLifeTime() )
			{
				continue;
			}

			m_ParticleMembers[ i ]->GetParticleParam( &Param );
			if( ( bIterate ) && ( !Param.bIgnoreIterate ) )
			{
				bApplyIterate = true;
			}
			else
			{
				bApplyIterate = false;
			}
			if( Param.bUseOffsetMap )
			{
				if( bApplyIterate )
				{
					g_BsKernel.SetVertexDeclaration(s_nNormalDeclIndex);
#ifdef _XBOX
					pMaterial->BeginMaterial(s_hTechniqueIterateAlpha, 0);
#else
					pMaterial->BeginMaterial(s_hTechniqueIterateAlpha, D3DXFX_DONOTSAVESTATE);
#endif
				}
				else
				{
					g_BsKernel.SetVertexDeclaration(s_nNormalDeclIndex);
#ifdef _XBOX
					pMaterial->BeginMaterial(s_hTechniqueNormalAlpha, 0);
#else
					pMaterial->BeginMaterial(s_hTechniqueNormalAlpha, D3DXFX_DONOTSAVESTATE);
#endif
				}
			}
			else
			{
				if( bApplyIterate )
				{
					g_BsKernel.SetVertexDeclaration( s_nIterateDeclIndex );
#ifdef _XBOX
					pMaterial->BeginMaterial(s_hTechniqueIterate, 0);
#else
					pMaterial->BeginMaterial(s_hTechniqueIterate, D3DXFX_DONOTSAVESTATE);
#endif
				}
				else
				{
					g_BsKernel.SetVertexDeclaration( s_nNormalDeclIndex );
#ifdef _XBOX
					pMaterial->BeginMaterial(s_hTechniqueNormal, 0);
#else
					pMaterial->BeginMaterial(s_hTechniqueNormal, D3DXFX_DONOTSAVESTATE);
#endif
				}
			}

//			SetCommonParticleConstant( fScale );
			pMaterial->SetFloat( s_hParticleScale, fScale );

			SetParticleGroupConstant( pMatParticle, pColor );
			SetParticleMemberConstant( fScale, i, &Param );

			if( ( bApplyIterate ) && ( nSavePos != -1 ) )
			{
				m_ParticleMembers[ i ]->CopyVertexPosition( nSavePos + nParticleCount );
				nParticleCount += m_ParticleMembers[ i ]->GetParticleCount();
			}

			fLifeTime = m_ParticleMembers[ i ]->GetLifeTime() / 30.0f;
			nCurrentTick = nTick % m_ParticleMembers[ i ]->GetLifeTime();
			if( nTick / m_ParticleMembers[ i ]->GetLifeTime() )
			{
				nCurrentTick += m_ParticleMembers[ i ]->GetLifeTime();
			}
			nCurrentStopTick = nStopTick % m_ParticleMembers[ i ]->GetLifeTime();
			if( nStopTick / m_ParticleMembers[ i ]->GetLifeTime() )
			{
				nCurrentStopTick += m_ParticleMembers[ i ]->GetLifeTime();
			}
			if( nCurrentStopTick > nCurrentTick )
			{
				nCurrentStopTick -= m_ParticleMembers[ i ]->GetLifeTime();
			}
			fParticleTime = ( nCurrentTick / 30.0f ) / fLifeTime;

			pMaterial->SetFloat( s_hParticleTime, fParticleTime );
			pMaterial->SetFloat( s_hLifeTime, fLifeTime );

			pMaterial->SetTexture( s_hTexture, ( LPDIRECT3DBASETEXTURE9 )g_BsKernel.GetTexturePtr( m_ParticleMembers[ i ]->GetTextureIndex() ) );
			if( Param.bUseOffsetMap ) {
				pMaterial->SetTexture(s_hTextureAlpha, g_BsKernel.GetImageProcess()->GetBackBufferTexture());
			}
			pMaterial->BeginPass( 0 );
			pMaterial->CommitChanges();
			m_ParticleMembers[ i ]->RenderParticle( pDevice, nCurrentTick, nCurrentStopTick, bLoop, bApplyIterate, fScale );
			pMaterial->EndPass();
			pMaterial->EndMaterial();
		}
	}
}

void CBsParticleGroup::SetParticlePosition( int nSavePos, int nTick, D3DXMATRIX *pParticleMat )
{
	int i, nCurPos, nSize;

	nSize = ( int )m_ParticleMembers.size();
	nCurPos = 0;
	for( i = 0; i < nSize; i++ )
	{
		m_ParticleMembers[ i ]->SetParticlePosition( nSavePos + nCurPos, nTick, pParticleMat );
		nCurPos += m_ParticleMembers[ i ]->GetParticleCount();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

CBsParticleObject::CBsParticleObject()
: m_pParticleGroup(NULL)
{
	ResetParticleInfo();
}

CBsParticleObject::~CBsParticleObject()
{
	SAFE_RELEASE( m_pParticleGroup );
}

void CBsParticleObject::RenderParticle(C3DDevice *pDevice)
{
	int nStopTick;
	D3DXMATRIX MatParticle;

	if( m_bShow ) {
		if( m_pParticleGroup ) {
			if( ( m_nStopTick == -1 ) || ( m_nStopTick >= m_nParticleTick ) ) {
				nStopTick = m_nParticleTick;
			}
			else {
				nStopTick = m_nStopTick;
			}
			if( m_bIterate ) {
				if( m_nSavePositionIndex == -1 ) {
					m_nSavePositionIndex = g_BsKernel.AllocIteratePosition( m_pParticleGroup->GetTotalParticleCount() );
				}
				SetIteratePosition();
			}
			m_pParticleGroup->SetUseAlphaFog( m_bUseAlphaFog );
			m_pParticleGroup->RenderParticle( pDevice, &m_MatParticle, m_nParticleTick, nStopTick, m_bLoop, 
				m_bIterate, m_nSavePositionIndex, &m_Color, m_fScale );
		}
	}
	if( m_bLinked ) {	// 링크된 파티클의 경우는 부모따라서 Show/Hide 가 결정된다.
		m_bShow = false;
	}
}

void CBsParticleObject::RenderParticleForTool()
{
	if( m_bIterate ) {
		if( m_nSavePositionIndex == -1 ) {
			m_nSavePositionIndex = g_BsKernel.AllocIteratePosition( m_pParticleGroup->GetTotalParticleCount() );
		}
		SetIteratePosition();
	}
}

void CBsParticleObject::ResetParticleInfo()
{
	SAFE_RELEASE( m_pParticleGroup );

	m_bDelete = false;
	m_bPause = false;
	m_bUse = false;
	m_bShow = true;
	m_bLinked = false;
	m_bIterate = false;
	m_bLoop = false;
	m_pParticleGroup = NULL;
	m_nSavePositionIndex = -1;
	m_nParticleTick = -1;
	m_nStopTick = -1;
	m_Color.r = 1.0f;
	m_Color.g = 1.0f;
	m_Color.b = 1.0f;
	m_Color.a = 1.0f;
	m_fScale = 1.0f;
	m_bUseAlphaFog = true;
	m_bUseCull = true;
}

bool CBsParticleObject::ProcessParticle()
{
	int i;

	if( m_bPause ) {
		return false;
	}

	if( m_bDelete ) {
		return true;
	}

	if( !m_pParticleGroup )	{
		return true;
	}

	//m_nParticleTick++;
	for( i = 0; i < m_pParticleGroup->GetParticleMemberCount(); i++ ) {
		if( m_nParticleTick < m_pParticleGroup->GetParticleMemberLifeTime( i ) * 2 ) {
			break;
		}
	}

	if ( m_bLoop ) {
		if( m_nStopTick != -1 ) {
			if( m_nParticleTick - m_nStopTick >= m_pParticleGroup->GetMaxParticleLife() ) {
				return true;
			}
		}
	}
	else {
		if( m_bAutoDelete && i>=m_pParticleGroup->GetParticleMemberCount() ) {
			return true;
		}
	}

	return false;
}

void CBsParticleObject::SetIteratePosition()
{
	if( m_nSavePositionIndex != -1 ) {
		if( m_nParticleTick == 1 ) {	// TODO : 나중에 anitool에서 링크 일때는 매트릭스 저장해서 파티클 생성해야 한다..
			m_pParticleGroup->SetParticlePosition( m_nSavePositionIndex, 0, &m_MatParticle );
			m_pParticleGroup->SetParticlePosition( m_nSavePositionIndex, 1, &m_MatParticle );
		}
		else {
			m_pParticleGroup->SetParticlePosition( m_nSavePositionIndex, m_nParticleTick, &m_MatParticle );
		}
	}
}

void CBsParticleObject::SetParticleGroupPtr(CBsParticleGroup* pGroup)
{
	BsAssert( pGroup && "Invalid Particle Group!!" );
	if ( pGroup )
		pGroup->AddRef();

	if ( m_pParticleGroup ) {
		m_pParticleGroup->Release();
		m_pParticleGroup = NULL;
	}

	m_pParticleGroup = pGroup;
}

void CBsParticleObject::SetParticleMatrix( D3DXMATRIX *pMatParticle )
{
	if( pMatParticle ) {
		m_MatParticle =* pMatParticle;
	}
}

void CBsParticleObject::AdjustSaveIteratePosition( int nIndex, int nCount )
{
	if( m_nSavePositionIndex > nIndex ) {
		m_nSavePositionIndex -= nCount;
	}
}

void CBsParticleObject::StopPlay( int nStopTick )
{
	if(m_nStopTick == -1 ) {
		if( nStopTick == -1 ) {
			m_nStopTick = m_nParticleTick + 1;
		}
		else {
			m_nStopTick = nStopTick;
		}
	}
}

float CBsParticleMember::RandomNumberInRange( float Min, float Max )
{
	float fRange, fRandom;

	fRandom = float(rand()) / float(RAND_MAX);
	fRange = Max - Min;
	fRandom *= fRange;
	fRandom += Min;

	return fRandom;
}