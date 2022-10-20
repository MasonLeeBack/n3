#include "stdafx.h"
#include "BsBone.h"
#include "BsKernel.h"
#include "bstreamext.h"

#include "BsRealMovieFile.h"
#include "BsRealMovieObject.h"
#include "BsrealMovie.h"
#include "BsFileIO.h"
#include "BsPhysicsMgr.h"

#ifdef _XBOX
#define _OPT_SHINJICH_BSBONE_CPP	0

static const DWORD XMemAllocAttributes = MAKE_XALLOC_ATTRIBUTES( 0, 0, 0, 0, 123, XALLOC_ALIGNMENT_DEFAULT | XALLOC_PHYSICAL_ALIGNMENT_DEFAULT, XALLOC_MEMPROTECT_READWRITE, 0, XALLOC_MEMTYPE_HEAP );
#endif

CBsAniSampledKeyFrame::CBsAniSampledKeyFrame()
{	
	m_nPositionKeyCount=0;
	m_nRotationKeyCount=0;
	m_nScalingKeyCount=0;

	m_PositionAni.nTime=NULL;
	m_PositionAni.Position=NULL;	

	m_RotationAni.nTime=NULL;
	m_RotationAni.Rotation=NULL;

	m_ScalingAni.nTime=NULL;
	m_ScalingAni.Scale=NULL;
	m_ScalingAni.ScaleRot=NULL;
}

CBsAniSampledKeyFrame::~CBsAniSampledKeyFrame()
{
	SAFE_DELETEA(m_PositionAni.nTime);
#if _OPT_SHINJICH_BSBONE_CPP
	// - shinjich
	XMemFreeDefault( m_PositionAni.Position, XMemAllocAttributes );
#else
	SAFE_DELETEA(m_PositionAni.Position);
#endif
	SAFE_DELETEA(m_RotationAni.nTime);
#if _OPT_SHINJICH_BSBONE_CPP
	// - shinjich
	XMemFreeDefault( m_RotationAni.Rotation, XMemAllocAttributes );
#else
	SAFE_DELETEA(m_RotationAni.Rotation);
#endif
	SAFE_DELETEA(m_ScalingAni.nTime);
	SAFE_DELETEA(m_ScalingAni.Scale);
	SAFE_DELETEA(m_ScalingAni.ScaleRot);
}

int CBsAniSampledKeyFrame::LoadAniInfo(BStream* pStream)
{
	// 애니메이션이 없는 경우에도 m_nPositionKeyCount 가 2 인 경우가 있습니다.
	// 필요하다면 루틴을 추가해서 0 으로 만들어 주도록 합니다 (=플러그인에서 처리해도 될 것입니다)
	int i;

	pStream->Read( &m_nPositionKeyCount, sizeof( int ), ENDIAN_FOUR_BYTE );
	if( m_nPositionKeyCount )
	{
#ifdef USE_COMPRESS_TABLE
		m_PositionAni.nTime= new WORD[ m_nPositionKeyCount ];
		D3DXVECTOR3 *PositionTable = new D3DXVECTOR3[ m_nPositionKeyCount ];
		D3DXVECTOR3 vMax = D3DXVECTOR3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
		D3DXVECTOR3 vMin = D3DXVECTOR3(FLT_MAX, FLT_MAX, FLT_MAX);
		for( i = 0; i < m_nPositionKeyCount; i++ )
		{
			DWORD nTime;
			pStream->Read( &nTime, sizeof( int ), ENDIAN_FOUR_BYTE );
			pStream->Read( &PositionTable[i], sizeof( float ) * 3, ENDIAN_FOUR_BYTE );
			BsAssert( nTime < 65536);
			m_PositionAni.nTime[i] = (WORD)nTime;
			D3DXVec3Minimize(&vMin, &vMin, &PositionTable[i]);
			D3DXVec3Maximize(&vMax, &vMax, &PositionTable[i]);
		}

		vMin -= D3DXVECTOR3(1,1,1);
		vMax += D3DXVECTOR3(1,1,1);

		D3DXVECTOR3 vDistRcp =  (vMax - vMin);
		vDistRcp.x = 1 / vDistRcp.x;
		vDistRcp.y = 1 / vDistRcp.y;
		vDistRcp.z = 1 / vDistRcp.z;
#if _OPT_SHINJICH_BSBONE_CPP
		// - shinjich
//		m_PositionAni.Position = new WORD[ m_nPositionKeyCount * 4];
		m_PositionAni.Position = (WORD*) XMemAllocDefault( m_nPositionKeyCount * 4 * sizeof(WORD) + 8, XMemAllocAttributes );
		for( i = 0; i < m_nPositionKeyCount; i++) {
			int x = (int)(65535 * (PositionTable[i].x - vMin.x) * vDistRcp.x) - 32768;
			int y = (int)(65535 * (PositionTable[i].y - vMin.y) * vDistRcp.y) - 32768;
			int z = (int)(65535 * (PositionTable[i].z - vMin.z) * vDistRcp.z) - 32768;
			BsAssert( -32768 <= x && x <= 32767 );
			BsAssert( -32768 <= y && y <= 32767 );
			BsAssert( -32768 <= z && z <= 32767 );
			x = BsMax(-32768, BsMin(32767, x));
			y = BsMax(-32768, BsMin(32767, y));
			z = BsMax(-32768, BsMin(32767, z));
			m_PositionAni.Position[i*4+0] = x;
			m_PositionAni.Position[i*4+1] = y;
			m_PositionAni.Position[i*4+2] = z;
			m_PositionAni.Position[i*4+3] = 0;
		}
/*
		// d3dpack version
		int counter = 0;
		for( i = 0; i < m_nPositionKeyCount; i++ )
		{
			counter++;
			if ( counter >= 2 )
			{
				BSQUATERNION tempPosition, tempPosition2;
				counter = 0;
				tempPosition.x = PositionTable[i].x;
				tempPosition.y = PositionTable[i].y;
				tempPosition.z = PositionTable[i].z;
				tempPosition.w = 0.0f;
				tempPosition2.x = PositionTable[i+1].x;
				tempPosition2.y = PositionTable[i+1].y;
				tempPosition2.z = PositionTable[i+1].z;
				tempPosition2.w = 0.0f;
				tempPosition = __vpkd3d( tempPosition, tempPosition, VPACK_FLOAT16_4, VPACK_64LO, 0 );
				tempPosition = __vpkd3d( tempPosition, tempPosition2, VPACK_FLOAT16_4, VPACK_64LO, 2 );
				*((__vector4*) &m_PositionAni.Position[i * 4]) = tempPosition;
			}
		}
		if ( counter )
		{
			BSQUATERNION tempPosition, tempPosition2;
			static const __declspec(align(16)) DWORD constvzero[4] = { 0, 0, 0, 0 };
			tempPosition2 = __lvx( constvzero, 0 );
			tempPosition.x = PositionTable[i-1].x;
			tempPosition.y = PositionTable[i-1].y;
			tempPosition.z = PositionTable[i-1].z;
			tempPosition.w = 0.0f;
			tempPosition = __vpkd3d( tempPosition, tempPosition, VPACK_FLOAT16_4, VPACK_64LO, 0 );
			tempPosition = __vpkd3d( tempPosition, tempPosition2, VPACK_FLOAT16_4, VPACK_64LO, 2 );
			*((__vector4*) &m_PositionAni.Position[(i - 1) * 4]) = tempPosition;
		}
*/
#else
		// original
		m_PositionAni.Position = new WORD[ m_nPositionKeyCount * 3];
		for( i = 0; i < m_nPositionKeyCount; i++) {
			int x = (int)(65535 * (PositionTable[i].x - vMin.x) * vDistRcp.x);
			int y = (int)(65535 * (PositionTable[i].y - vMin.y) * vDistRcp.y);
			int z = (int)(65535 * (PositionTable[i].z - vMin.z) * vDistRcp.z);
			BsAssert( 0 <= x && x <= 65535 );
			BsAssert( 0 <= y && y <= 65535 );
			BsAssert( 0 <= z && z <= 65535 );
			x = BsMax(0, BsMin(65535, x));
			y = BsMax(0, BsMin(65535, y));
			z = BsMax(0, BsMin(65535, z));
			m_PositionAni.Position[i*3+0] = x;
			m_PositionAni.Position[i*3+1] = y;
			m_PositionAni.Position[i*3+2] = z;
		}
#endif
		delete [] PositionTable;

		m_PositionAni.vMin.x = vMin.x;
		m_PositionAni.vMin.y = vMin.y;
		m_PositionAni.vMin.z = vMin.z;
		D3DXVECTOR3 vAdv = (vMax-vMin) / 65535.f;
		m_PositionAni.vAdv.x = vAdv.x;
		m_PositionAni.vAdv.y = vAdv.y;
		m_PositionAni.vAdv.z = vAdv.z;
#else
		m_PositionAni.nTime= new int[ m_nPositionKeyCount ];
		m_PositionAni.Position = new BSVECTOR[ m_nPositionKeyCount ];
		for( i = 0; i < m_nPositionKeyCount; i++ )
		{
			pStream->Read( &m_PositionAni.nTime[i], sizeof( int ), ENDIAN_FOUR_BYTE );
			pStream->Read( &m_PositionAni.Position[i], sizeof( float ) * 3, ENDIAN_FOUR_BYTE );
		}
#endif
#ifdef USE_POSITION_INDEX_TABLE
		int i, nSize, nCurKey;

		nSize = m_pPositionAni[ m_nPositionKeyCount -1 ].nTime + 1;
		m_PositionKeyIndex.resize( nSize );
		nCurKey = 0;
		for( i = 0; i < nSize; i++ )
		{
			if( ( i >= m_pPositionAni[ nCurKey ].nTime ) && ( i < m_pPositionAni[ nCurKey + 1 ].nTime ) )
			{
				m_PositionKeyIndex[ i ] = nCurKey;
			}
			else
			{
				nCurKey++;
				m_PositionKeyIndex[ i ] = nCurKey;
			}
		}
#endif
	}
	else
	{
		m_PositionAni.nTime = NULL;
		m_PositionAni.Position = NULL;
	}    

	pStream->Read(&m_nRotationKeyCount, sizeof(int), ENDIAN_FOUR_BYTE);
	if( m_nRotationKeyCount )
	{
#ifdef USE_COMPRESS_TABLE
		m_RotationAni.nTime = new WORD[ m_nRotationKeyCount ];
#if _OPT_SHINJICH_BSBONE_CPP
		// - shinjich
		m_RotationAni.Rotation = (short*) XMemAllocDefault( m_nRotationKeyCount * 4 * sizeof(short) + 8, XMemAllocAttributes );
#else
		m_RotationAni.Rotation = new WORD[ m_nRotationKeyCount * 4];
#endif
#if _OPT_SHINJICH_BSBONE_CPP
		for( i = 0; i < m_nRotationKeyCount; i++ )
		{
			DWORD nTime;
			BSQUATERNION Rotation;
			pStream->Read( &nTime, sizeof( int ), ENDIAN_FOUR_BYTE );			
			pStream->Read( &Rotation, sizeof( float ) * 4, ENDIAN_FOUR_BYTE );
			BsAssert( nTime < 65536);
			m_RotationAni.nTime[ i ] = (WORD)nTime;

			int x = (int)(32767.5 * Rotation.x);
			int y = (int)(32767.5 * Rotation.y);
			int z = (int)(32767.5 * Rotation.z);
			int w = (int)(32767.5 * Rotation.w);
			BsAssert( -32768 <= x && x <= 32767 );
			BsAssert( -32768 <= y && y <= 32767 );
			BsAssert( -32768 <= z && z <= 32767 );
			BsAssert( -32768 <= w && w <= 32767 );

			m_RotationAni.Rotation[ i * 4 + 0] = x;
			m_RotationAni.Rotation[ i * 4 + 1] = y;
			m_RotationAni.Rotation[ i * 4 + 2] = z;
			m_RotationAni.Rotation[ i * 4 + 3] = w;
		}
/*
		// d3dpack version
		int counter = 0;
		BSQUATERNION Rotation[2];
		for( i = 0; i < m_nRotationKeyCount; i++ )
		{
			DWORD nTime;
			pStream->Read( &nTime, sizeof( int ), ENDIAN_FOUR_BYTE );			
			pStream->Read( &Rotation[counter], sizeof( float ) * 4, ENDIAN_FOUR_BYTE );
			BsAssert( nTime < 65536);
			m_RotationAni.nTime[ i ] = (WORD)nTime;

			counter++;
			if ( counter >= 2 )
			{
				BSQUATERNION tempRotation;
				counter = 0;
				tempRotation = __vpkd3d( Rotation[0], Rotation[0], VPACK_FLOAT16_4, VPACK_64LO, 0 );
				tempRotation = __vpkd3d( tempRotation, Rotation[1], VPACK_FLOAT16_4, VPACK_64LO, 2 );
				*((__vector4*) &m_RotationAni.Rotation[i * 4]) = tempRotation;

				static BSQUATERNION Rotation1, Rotation2, backup;

				__vector4 packed = tempRotation;
				Rotation1 = __vupkd3d( packed, VPACK_FLOAT16_4 );
				__vector4 packed2 = __vsldoi( packed, packed, 8 );
				Rotation2 = __vupkd3d( packed2, VPACK_FLOAT16_4 );
				backup = __vor( Rotation1, Rotation2 );
			}
		}
		if ( counter )
		{
			static const __declspec(align(16)) DWORD constvzero[4] = { 0, 0, 0, 0 };
			Rotation[1] = __lvx( constvzero, 0 );
			Rotation[0] = __vpkd3d( Rotation[0], Rotation[0], VPACK_FLOAT16_4, VPACK_64LO, 0 );
			Rotation[0] = __vpkd3d( Rotation[0], Rotation[1], VPACK_FLOAT16_4, VPACK_64LO, 2 );
			*((__vector4*) &m_RotationAni.Rotation[(i - 1) * 4]) = Rotation[0];
		}
*/
#else
		// original
		for( i = 0; i < m_nRotationKeyCount; i++ )
		{
			DWORD nTime;
			BSQUATERNION Rotation;
			pStream->Read( &nTime, sizeof( int ), ENDIAN_FOUR_BYTE );			
			pStream->Read( &Rotation, sizeof( float ) * 4, ENDIAN_FOUR_BYTE );
			BsAssert( nTime < 65536);
			m_RotationAni.nTime[ i ] = (WORD)nTime;

			int x = (int)(65535 * (Rotation.x * 0.5f + 0.5f));
			int y = (int)(65535 * (Rotation.y * 0.5f + 0.5f));
			int z = (int)(65535 * (Rotation.z * 0.5f + 0.5f));
			int w = (int)(65535 * (Rotation.w * 0.5f + 0.5f));
			BsAssert( 0 <= x && x <= 65535 );
			BsAssert( 0 <= y && y <= 65535 );
			BsAssert( 0 <= z && z <= 65535 );
			BsAssert( 0 <= w && w <= 65535 );

			m_RotationAni.Rotation[ i * 4 + 0] = x;
			m_RotationAni.Rotation[ i * 4 + 1] = y;
			m_RotationAni.Rotation[ i * 4 + 2] = z;
			m_RotationAni.Rotation[ i * 4 + 3] = w;
		}
#endif
#else
		m_RotationAni.nTime = new int[ m_nRotationKeyCount ];
		m_RotationAni.Rotation = new BSQUATERNION[ m_nRotationKeyCount ];
		for( i = 0; i < m_nRotationKeyCount; i++ )
		{
			pStream->Read( &m_RotationAni.nTime[ i ], sizeof( int ), ENDIAN_FOUR_BYTE );
			pStream->Read( &m_RotationAni.Rotation[ i ], sizeof( float ) * 4, ENDIAN_FOUR_BYTE );
		}
#endif

#ifdef USE_ROTATION_INDEX_TABLE
		int i, nSize, nCurKey;

		nSize = m_pRotationAni[ m_nRotationKeyCount -1 ].nTime + 1;
		m_RotationKeyIndex.resize( nSize );
		nCurKey = 0;
		for( i = 0; i < nSize; i++ )
		{
			if( ( i >= m_pRotationAni[ nCurKey ].nTime ) && ( i < m_pRotationAni[ nCurKey + 1 ].nTime ) )
			{
				m_RotationKeyIndex[ i ] = nCurKey;
			}
			else
			{
				nCurKey++;
				m_RotationKeyIndex[ i ] = nCurKey;
			}
		}
#endif
	}
	else{
		m_RotationAni.nTime = NULL;
		m_RotationAni.Rotation = NULL;
	}

	pStream->Read(&m_nScalingKeyCount, sizeof(int), ENDIAN_FOUR_BYTE);
	if( m_nScalingKeyCount ) 
	{
		m_ScalingAni.nTime = new int[m_nScalingKeyCount];
		m_ScalingAni.Scale = new BSVECTOR[m_nScalingKeyCount];
		m_ScalingAni.ScaleRot = new BSQUATERNION[m_nScalingKeyCount];

		for( i = 0; i < m_nScalingKeyCount; i++ )
		{
			pStream->Read( &m_ScalingAni.nTime[ i ], sizeof( int ), ENDIAN_FOUR_BYTE );
			pStream->Read( &m_ScalingAni.Scale[ i ], sizeof( float ) * 3, ENDIAN_FOUR_BYTE );
			pStream->Read( &m_ScalingAni.ScaleRot[ i ], sizeof( float ) * 4, ENDIAN_FOUR_BYTE );
		}
	} 
	else {
		m_ScalingAni.nTime = NULL;
		m_ScalingAni.Scale = NULL;
		m_ScalingAni.ScaleRot = NULL;
	} 

	//디버깅 char buf[256]; sprintf(buf, "pos=%d rot=%d sca=%d\n", m_nPositionKeyCount,m_nRotationKeyCount,m_nScalingKeyCount); DebugString(buf);

	return 1;
}

//------------------------------------------------------------------------------------------------

int CBsAniSampledKeyFrame::SaveAniInfo(BStream* pStream)   
{
	pStream->Write( &m_nPositionKeyCount, sizeof(int), ENDIAN_FOUR_BYTE);
	for( int i = 0; i < m_nPositionKeyCount; i++ ) {		
		pStream->Write( &m_PositionAni.nTime[i], sizeof(int) , ENDIAN_FOUR_BYTE);
		pStream->Write( &m_PositionAni.Position[i], sizeof(BSVECTOR), ENDIAN_FOUR_BYTE);
	}

	pStream->Write( &m_nRotationKeyCount, sizeof(int), ENDIAN_FOUR_BYTE);
	for( int i = 0; i < m_nRotationKeyCount; i++) {		
		pStream->Write( &m_RotationAni.nTime[i], sizeof(int), ENDIAN_FOUR_BYTE);
		pStream->Write( &m_RotationAni.Rotation[i], sizeof(BSQUATERNION), ENDIAN_FOUR_BYTE);
	}

	pStream->Write( &m_nScalingKeyCount, sizeof(int), ENDIAN_FOUR_BYTE);
	for( int i = 0; i < m_nScalingKeyCount; i++) {			
		pStream->Write( &m_ScalingAni.nTime[i], sizeof(int), ENDIAN_FOUR_BYTE);
		pStream->Write( &m_ScalingAni.Scale[i], sizeof(BSVECTOR), ENDIAN_FOUR_BYTE);
		pStream->Write( &m_ScalingAni.ScaleRot[i], sizeof(BSQUATERNION), ENDIAN_FOUR_BYTE);
	}
	return 1;
}

void CBsAniSampledKeyFrame::CopyAni(CBsAniSampledKeyFrame *pSourceAni)
{
#ifdef USE_COMPRESS_TABLE
	BsAssert( 0 && "USE_COMPRESS_TABLE off");
#else
	CBsAniSampledKeyFrame *pAni=pSourceAni;

	m_nPositionKeyCount=pAni->GetPositionKeyCount();
	m_nRotationKeyCount=pAni->GetRotationKeyCount();
	m_nScalingKeyCount=pAni->GetScalingKeyCount();

	if(m_nPositionKeyCount>0){
		m_PositionAni.nTime =new int[m_nPositionKeyCount];
		m_PositionAni.Position =new BSVECTOR[m_nPositionKeyCount];
		memcpy(m_PositionAni.nTime, pAni->m_PositionAni.nTime, sizeof(int)*m_nPositionKeyCount);
		memcpy(m_PositionAni.Position, pAni->m_PositionAni.Position, sizeof(BSVECTOR)*m_nPositionKeyCount);
	}
	else{
		m_PositionAni.nTime = NULL;
		m_PositionAni.Position = NULL;
	}

	if(m_nRotationKeyCount>0){
		m_RotationAni.nTime = new int[m_nRotationKeyCount];
		m_RotationAni.Rotation = new BSQUATERNION[m_nRotationKeyCount];
		memcpy(m_RotationAni.nTime, pAni->m_RotationAni.nTime, sizeof(int)*m_nRotationKeyCount);
		memcpy(m_RotationAni.Rotation, pAni->m_RotationAni.Rotation, sizeof(BSQUATERNION)*m_nRotationKeyCount);
	}
	else{
		m_RotationAni.nTime = NULL;
		m_RotationAni.Rotation = NULL;
	}

	if(m_nScalingKeyCount>0){
		m_ScalingAni.nTime = new int[m_nScalingKeyCount];
		m_ScalingAni.Scale = new BSVECTOR[m_nScalingKeyCount];
		m_ScalingAni.ScaleRot = new BSQUATERNION[m_nScalingKeyCount];
		memcpy(m_ScalingAni.nTime, pAni->m_ScalingAni.nTime, sizeof(int)*m_nScalingKeyCount);
		memcpy(m_ScalingAni.Scale, pAni->m_ScalingAni.Scale, sizeof(BSVECTOR)*m_nScalingKeyCount);
		memcpy(m_ScalingAni.ScaleRot, pAni->m_ScalingAni.ScaleRot, sizeof(BSQUATERNION)*m_nScalingKeyCount);
	} else {
		m_ScalingAni.nTime = NULL;
		m_ScalingAni.Scale = NULL;
		m_ScalingAni.ScaleRot = NULL;
	}
#endif
}

POSITION_ANI* CBsAniSampledKeyFrame::GetPositionKey( int nIndex )
{
#ifdef _USAGE_TOOL_
	static POSITION_ANI PositionAni;
	PositionAni.nTime = m_PositionAni.nTime[nIndex];
	PositionAni.Position = m_PositionAni.Position[nIndex];
	return &PositionAni;
#else
	return NULL;
#endif
}

ROTATION_ANI* CBsAniSampledKeyFrame::GetRotationKey( int nIndex )
{
#ifdef _USAGE_TOOL_
	static ROTATION_ANI RotationAni;
	RotationAni.nTime = m_RotationAni.nTime[nIndex];
	RotationAni.Rotation = m_RotationAni.Rotation[nIndex];
	return &RotationAni;
#else
	return NULL;
#endif
}

SCALING_ANI* CBsAniSampledKeyFrame::GetScalingKey( int nIndex )
{
#ifdef _USAGE_TOOL_
	static SCALING_ANI ScalingAni;
	ScalingAni.nTime = m_ScalingAni.nTime[nIndex];
	ScalingAni.Scale = m_ScalingAni.Scale[nIndex];
	ScalingAni.ScaleRot = m_ScalingAni.ScaleRot[nIndex];
	return &ScalingAni;
#else
	return NULL;
#endif
}

void CBsAniSampledKeyFrame::DeleteAnimationKey()
{
	if( m_nPositionKeyCount )
	{		
		SAFE_DELETEA(m_PositionAni.nTime);
#ifdef _XBOX
		XMemFreeDefault( m_PositionAni.Position, XMemAllocAttributes );
#else
		SAFE_DELETEA(m_PositionAni.Position);
#endif

	}
	m_nPositionKeyCount = 0;
	if( m_nRotationKeyCount )
	{
		SAFE_DELETEA(m_RotationAni.nTime);
#ifdef _XBOX
		XMemFreeDefault( m_RotationAni.Rotation, XMemAllocAttributes );
#else
		SAFE_DELETEA(m_RotationAni.Rotation);
#endif
	}
	m_nRotationKeyCount = 0;
	if( m_nScalingKeyCount )
	{
		SAFE_DELETEA(m_ScalingAni.nTime);
		SAFE_DELETEA(m_ScalingAni.Scale);
		SAFE_DELETEA(m_ScalingAni.ScaleRot);
	}
	m_nScalingKeyCount = 0;

}

/*int CBsAniSampledKeyFrame::GetPositionVector(D3DXVECTOR3 &Position, float fFrame)
{
int i;
float fWeight;
int nKeyCount, nRangeStart, nRangeEnd;
POSITION_ANI *pPositionAni;

nKeyCount = m_nPositionKeyCount;
if( m_nPositionKeyCount > 1 )
{
pPositionAni=m_pPositionAni;
nRangeStart=0;
nRangeEnd=nKeyCount-1;
while(1){
i=(nRangeStart+nRangeEnd)/2;
if(pPositionAni[i].nTime==fFrame){
Position=pPositionAni[i].Position;
break;
}
else if(pPositionAni[i].nTime<fFrame){
if(pPositionAni[i+1].nTime>=fFrame){
i++;
}
else{
nRangeStart=i+1;
continue;
}
}
else{
if(pPositionAni[i-1].nTime>fFrame){
nRangeEnd=i-1;
continue;
}
}
fWeight=((float)(fFrame-pPositionAni[i-1].nTime))/(pPositionAni[i].nTime-pPositionAni[i-1].nTime);
D3DXVec3Lerp(&Position, &(pPositionAni[i-1].Position), &(pPositionAni[i].Position), fWeight);
break;
}
}
else if( m_nPositionKeyCount == 1 )
{
Position=m_pPositionAni[0].Position;
}
else{
return 0;
}

return 1;
}

int CBsAniSampledKeyFrame::GetRotationVector(D3DXQUATERNION &Rotation, float fFrame)
{
int i;
float fWeight;
int nKeyCount, nCurrentTime, nRangeStart, nRangeEnd;
ROTATION_ANI *pRotationAni;

nKeyCount=m_nRotationKeyCount;
if(m_nRotationKeyCount>1){
pRotationAni=m_pRotationAni;
nRangeStart=0;
nRangeEnd=nKeyCount-1;
while(1){
i=(nRangeStart+nRangeEnd)/2;
nCurrentTime=pRotationAni[i].nTime;
if(nCurrentTime==fFrame){
Rotation=pRotationAni[i].Rotation;
break;
}
else if(nCurrentTime<fFrame){
if(pRotationAni[i+1].nTime>=fFrame){
i++;
nCurrentTime=pRotationAni[i].nTime;
}
else{
nRangeStart=i+1;
continue;
}
}
else{
if(pRotationAni[i-1].nTime>fFrame){
nRangeEnd=i-1;
continue;
}
}
fWeight=((float)(fFrame-pRotationAni[i-1].nTime))/(nCurrentTime-pRotationAni[i-1].nTime);
D3DXVec4Lerp( ( D3DXVECTOR4 * )&Rotation, ( D3DXVECTOR4 * )&( pRotationAni[ i - 1 ].Rotation ), 
( D3DXVECTOR4 * )&( pRotationAni[ i ].Rotation ), fWeight );
break;
}
}
else if(m_nRotationKeyCount==1){
Rotation=m_pRotationAni[0].Rotation;
}
else{
return 0;
}

return 1;
}

int CBsAniSampledKeyFrame::GetScalingVector(D3DXVECTOR3 &Scale, D3DXQUATERNION &ScaleRot, float fFrame)
{
return 0;
int i;
float fWeight;
int nKeyCount, nCurrentTime, nRangeStart, nRangeEnd;
SCALING_ANI *pScalingAni;
nKeyCount=m_nScalingKeyCount;
if(nKeyCount>1) {
pScalingAni=m_pScalingAni;
nRangeStart=0;
nRangeEnd=nKeyCount-1;
while(1){
i=(nRangeStart+nRangeEnd)/2;
nCurrentTime=pScalingAni[i].nTime;
if(nCurrentTime==fFrame){
Scale=pScalingAni[i].Scale;
ScaleRot=pScalingAni[i].ScaleRot;
break;
} else if(nCurrentTime<fFrame){
if(pScalingAni[i+1].nTime>=fFrame) {
i++;
nCurrentTime=pScalingAni[i].nTime;
} else {
nRangeStart=i+1;
continue;
}
} else {
if(pScalingAni[i-1].nTime>fFrame){
nRangeEnd=i-1;
continue;
}
}
fWeight=((float)(fFrame-pScalingAni[i-1].nTime))/(nCurrentTime-pScalingAni[i-1].nTime);
D3DXVec3Lerp(&Scale, &(pScalingAni[i-1].Scale), &(pScalingAni[i].Scale), fWeight);
D3DXQuaternionSlerp(&ScaleRot, &(pScalingAni[i-1].ScaleRot), &(pScalingAni[i].ScaleRot), fWeight);
break;
}
} else if(nKeyCount==1) {
Scale=m_pScalingAni[0].Scale;
ScaleRot=m_pScalingAni[0].ScaleRot;
} else {
return 0;
}
return 1;
}*/

//------------------------------------------------------------------------------------------------

CBsCameraSampledKeyFrame::CBsCameraSampledKeyFrame()
{
	m_nPositionKeyCount=0;
	m_nRotationKeyCount=0;
	m_nScalingKeyCount=0;

	m_pPositionAni=NULL;
	m_pRotationAni=NULL;
	m_pScalingAni=NULL;
}

CBsCameraSampledKeyFrame::~CBsCameraSampledKeyFrame()
{
	Release();
}

void	CBsCameraSampledKeyFrame::Release(void)
{
	if(m_pPositionAni)
	{
		delete [] m_pPositionAni;
		m_pPositionAni = NULL;
	}

	if(m_pRotationAni)
	{
		delete [] m_pRotationAni;
		m_pRotationAni = NULL;
	}

	if(m_pScalingAni)
	{
		delete[] m_pScalingAni;
		m_pScalingAni = NULL;
	}
}

int CBsCameraSampledKeyFrame::Load(char *szFileName)
{
	DWORD dwFileSize;
	VOID *pData;
	char *pFullName=g_BsKernel.GetDirManager()->GetFullName(szFileName);

	if( FAILED(CBsFileIO::LoadFile( pFullName, &pData, &dwFileSize ) ) )
	{
		CBsFileText::ErrorNotFile(pFullName);
		return 0;
	}
	Release();

	BMemoryStream stream(pData, dwFileSize);
	int nRet = LoadCameraInfo(&stream);
	CBsFileIO::FreeBuffer(pData);
	return nRet;
}

int CBsCameraSampledKeyFrame::LoadCameraInfo(BStream* pStream)
{
	pStream->Read(&m_matInverseWorld, sizeof(D3DXMATRIX), ENDIAN_FOUR_BYTE);
	pStream->Read(&m_matLocal, sizeof(D3DXMATRIX), ENDIAN_FOUR_BYTE);
	// pStream->Read(&m_LocalAffine, sizeof(AFFINE_PARTS), ENDIAN_FOUR_BYTE);

#ifdef _XBOX
	int i;

	memset( &m_LocalAffine, 0, sizeof(AFFINE_PARTS));
	pStream->Read(&m_LocalAffine.Translation, sizeof(float)*3, ENDIAN_FOUR_BYTE);
	pStream->Read(&m_LocalAffine.Rotation, sizeof(float)*4, ENDIAN_FOUR_BYTE);
	pStream->Read(&m_LocalAffine.StretchRotation, sizeof(float)*4, ENDIAN_FOUR_BYTE);
	pStream->Read(&m_LocalAffine.Scale, sizeof(float)*3, ENDIAN_FOUR_BYTE);

#else
	pStream->Read(&m_LocalAffine, sizeof(AFFINE_PARTS), ENDIAN_FOUR_BYTE);
#endif

	int nAniType;
	pStream->Read(&nAniType, sizeof(int), ENDIAN_FOUR_BYTE);

	// 애니메이션이 없는 경우에도 m_nPositionKeyCount 가 2 인 경우가 있습니다.
	// 필요하다면 루틴을 추가해서 0 으로 만들어 주도록 합니다 (=플러그인에서 처리해도 될 것입니다)

	pStream->Read(&m_nPositionKeyCount, sizeof(int), ENDIAN_FOUR_BYTE);
	if(m_nPositionKeyCount)
	{
		m_pPositionAni=new POSITION_ANI[m_nPositionKeyCount];

#ifdef _XBOX
		for( i = 0 ; i < m_nPositionKeyCount ; ++i )
		{
			pStream->Read( &m_pPositionAni[i].nTime, sizeof(int), ENDIAN_FOUR_BYTE );
			pStream->Read( &m_pPositionAni[i].Position, sizeof(float)*3, ENDIAN_FOUR_BYTE );
			m_pPositionAni[i].Position.w = 0.f;
		}
#else
		pStream->Read(m_pPositionAni, sizeof(POSITION_ANI)*m_nPositionKeyCount, ENDIAN_FOUR_BYTE);
#endif
	}
	else
	{
		m_pPositionAni=NULL;
	}    

	pStream->Read(&m_nRotationKeyCount, sizeof(int), ENDIAN_FOUR_BYTE);
	if(m_nRotationKeyCount)
	{
		m_pRotationAni=new ROTATION_ANI[m_nRotationKeyCount];
#ifdef _XBOX
		for( i = 0 ; i < m_nRotationKeyCount ; ++i )
		{
			pStream->Read( &m_pRotationAni[i].nTime, sizeof(int), ENDIAN_FOUR_BYTE );
			pStream->Read( &m_pRotationAni[i].Rotation, sizeof(float)*4, ENDIAN_FOUR_BYTE );
		}
#else
		pStream->Read(m_pRotationAni, sizeof(ROTATION_ANI)*m_nRotationKeyCount, ENDIAN_FOUR_BYTE);
#endif
	}
	else
	{
		m_pRotationAni=NULL;
	}

	pStream->Read(&m_nScalingKeyCount, sizeof(int), ENDIAN_FOUR_BYTE);
	if(m_nScalingKeyCount) 
	{
		m_pScalingAni=new SCALING_ANI[m_nScalingKeyCount];

#ifdef _XBOX
		for( i = 0 ; i < m_nScalingKeyCount ; ++i )
		{
			pStream->Read( &m_pScalingAni[i].nTime, sizeof(int), ENDIAN_FOUR_BYTE );
			pStream->Read( &m_pScalingAni[i].Scale, sizeof(float)*3, ENDIAN_FOUR_BYTE );
			m_pScalingAni[i].Scale.w = 0.f;
			pStream->Read( &m_pScalingAni[i].ScaleRot, sizeof(BSQUATERNION), ENDIAN_FOUR_BYTE );
		}
#else
		pStream->Read(m_pScalingAni, sizeof(SCALING_ANI)*m_nScalingKeyCount, ENDIAN_FOUR_BYTE);
#endif
	} else 
	{
		m_pScalingAni=NULL;
	}

	// 디버깅 
	/*
	char buf[256]; 
	int i;
	for (i=0; i<4; i++) {
	sprintf(buf, "I: %f %f %f %f\n", m_matInverseWorld.m[i][0],m_matInverseWorld.m[i][1],m_matInverseWorld.m[i][2],m_matInverseWorld.m[i][3]); 
	DebugString(buf);
	}
	for (i=0; i<4; i++) {
	sprintf(buf, "L: %f %f %f %f\n", m_matLocal.m[i][0],m_matLocal.m[i][1],m_matLocal.m[i][2],m_matLocal.m[i][3]); 
	DebugString(buf);
	}
	sprintf(buf, "RT: %f %f %f %f\n", m_LocalAffine.Rotation.x, m_LocalAffine.Rotation.y, m_LocalAffine.Rotation.z, m_LocalAffine.Rotation.w);
	DebugString(buf);
	sprintf(buf, "SC: %f %f %f\n", m_LocalAffine.Scale.x, m_LocalAffine.Scale.y, m_LocalAffine.Scale.z);
	DebugString(buf);
	sprintf(buf, "SR: %f %f %f %f\n", m_LocalAffine.StretchRotation.x, m_LocalAffine.StretchRotation.y, m_LocalAffine.StretchRotation.z, m_LocalAffine.StretchRotation.w);
	DebugString(buf);
	sprintf(buf, "TS: %f %f %f\n", m_LocalAffine.Translation.x, m_LocalAffine.Translation.y, m_LocalAffine.Translation.z);
	DebugString(buf);

	sprintf(buf, "pos=%d rot=%d sca=%d\n", m_nPositionKeyCount,m_nRotationKeyCount,m_nScalingKeyCount); 
	DebugString(buf);
	for (int i=0; i<m_nPositionKeyCount; i++)  {
	sprintf(buf, "%d : %f %f %f\n", m_pPositionAni[i].nTime,m_pPositionAni[i].Position.x,m_pPositionAni[i].Position.y,m_pPositionAni[i].Position.z); 
	DebugString(buf);
	}
	for (int i=0; i<m_nRotationKeyCount; i++)  {
	sprintf(buf, "%d : %f %f %f %f\n", m_pRotationAni[i].nTime,m_pRotationAni[i].Rotation.x,m_pRotationAni[i].Rotation.y,m_pRotationAni[i].Rotation.z,m_pRotationAni[i].Rotation.w); 
	DebugString(buf);
	}
	*/

	return 1;
}

//------------------------------------------------------------------------------------------------

int CBsCameraSampledKeyFrame::GetPositionVector(BSVECTOR &Position, float fFrame)
{
	int i;
	float fWeight;
	int nKeyCount, nRangeStart, nRangeEnd;
	POSITION_ANI *pPositionAni;

	nKeyCount=m_nPositionKeyCount;
	if(nKeyCount>1)
	{
		pPositionAni=m_pPositionAni;
		nRangeStart=0;
		nRangeEnd=nKeyCount-1;

		while(1)
		{
			i=(nRangeStart+nRangeEnd)/2;

			if(pPositionAni[i].nTime==fFrame)
			{
				Position=pPositionAni[i].Position;
				break;
			}
			/*
			else if( (m_nPositionKeyCount-1) < (int)fFrame )
			{
			Position=pPositionAni[m_nPositionKeyCount-1].Position;
			break;
			}
			*/
			else if(pPositionAni[i].nTime<fFrame)
			{
				if(pPositionAni[i+1].nTime>=fFrame)
				{
					i++;
				}
				else{
					nRangeStart=i+1;
					continue;
				}
			}
			else
			{
				if(pPositionAni[i-1].nTime>fFrame){
					nRangeEnd=i-1;
					continue;
				}
			}
			fWeight=((float)(fFrame-pPositionAni[i-1].nTime))/(pPositionAni[i].nTime-pPositionAni[i-1].nTime);
			BsVec3Lerp(&Position, &(pPositionAni[i-1].Position), &(pPositionAni[i].Position), fWeight);
			break;
		}
	}
	else if(nKeyCount==1){
		Position=m_pPositionAni[0].Position;
	}
	else{
		return 0;
	}

	return 1;
}

int CBsCameraSampledKeyFrame::GetRotationVector(BSQUATERNION &Rotation, float fFrame)
{
	int i;
	float fWeight;
	int nKeyCount, nCurrentTime, nRangeStart, nRangeEnd;
	ROTATION_ANI *pRotationAni;

	nKeyCount=m_nRotationKeyCount;

	if(nKeyCount>1)
	{
		pRotationAni=m_pRotationAni;
		nRangeStart=0;
		nRangeEnd=nKeyCount-1;

		while(1)
		{
			i=(nRangeStart+nRangeEnd)/2;
			nCurrentTime=pRotationAni[i].nTime;

			if(nCurrentTime==fFrame)
			{
				Rotation=pRotationAni[i].Rotation;
				break;
			}
			/*
			else if( ( m_nRotationKeyCount-1 ) < fFrame )
			{
			Rotation=pRotationAni[m_nRotationKeyCount-1].Rotation;
			break;
			}
			*/
			else if(nCurrentTime<fFrame)
			{
				if(pRotationAni[i+1].nTime>=fFrame)
				{
					i++;
					nCurrentTime=pRotationAni[i].nTime;
				}
				else
				{
					nRangeStart=i+1;
					continue;
				}
			}
			else{
				if(pRotationAni[i-1].nTime>fFrame){
					nRangeEnd=i-1;
					continue;
				}
			}
			fWeight=((float)(fFrame-pRotationAni[i-1].nTime))/(nCurrentTime-pRotationAni[i-1].nTime);
			BsQuaternionSlerp(&Rotation, &(pRotationAni[i-1].Rotation), &(pRotationAni[i].Rotation), fWeight);
			break;
		}
	}
	else if(nKeyCount==1){
		Rotation=m_pRotationAni[0].Rotation;
	}
	else{
		return 0;
	}

	return 1;
}

int CBsCameraSampledKeyFrame::GetScalingVector(BSVECTOR &Scale, BSQUATERNION &ScaleRot, float fFrame)
{
	int i;
	float fWeight;
	int nKeyCount, nCurrentTime, nRangeStart, nRangeEnd;
	SCALING_ANI *pScalingAni;
	nKeyCount=m_nScalingKeyCount;
	if(nKeyCount>1) {
		pScalingAni=m_pScalingAni;
		nRangeStart=0;
		nRangeEnd=nKeyCount-1;
		while(1){
			i=(nRangeStart+nRangeEnd)/2;
			nCurrentTime=pScalingAni[i].nTime;
			if(nCurrentTime==fFrame){
				Scale=pScalingAni[i].Scale;
				ScaleRot=pScalingAni[i].ScaleRot;
				break;
			} else if(nCurrentTime<fFrame){
				if(pScalingAni[i+1].nTime>=fFrame) {
					i++;
					nCurrentTime=pScalingAni[i].nTime;
				} else {
					nRangeStart=i+1;
					continue;
				}
			} else {
				if(pScalingAni[i-1].nTime>fFrame){
					nRangeEnd=i-1;
					continue;
				}
			}
			fWeight=((float)(fFrame-pScalingAni[i-1].nTime))/(nCurrentTime-pScalingAni[i-1].nTime);
			BsVec3Lerp(&Scale, &(pScalingAni[i-1].Scale), &(pScalingAni[i].Scale), fWeight);
			BsQuaternionSlerp(&ScaleRot, &(pScalingAni[i-1].ScaleRot), &(pScalingAni[i].ScaleRot), fWeight);
			break;
		}
	} else if(nKeyCount==1) {
		Scale=m_pScalingAni[0].Scale;
		ScaleRot=m_pScalingAni[0].ScaleRot;
	} else {
		return 0;
	}
	return 1;
}

void CBsCameraSampledKeyFrame::ProcessCamera(CCrossVector &CameraCross, float fra) // fra 값이 범위를 벗어나지 않도록 주의 합니다 
{
	BSVECTOR pos3;
	GetPositionVector(pos3,fra);

	BSQUATERNION rot4;	
	GetRotationVector(rot4,fra);

	BSMATRIX m;

#ifdef _XBOX
	BsMatrixTransformation(&m, &XMVectorSet(0.f, 0.f, 0.f, 0.f), &XMVectorSet(0.f, 0.f, 0.f, 0.f), &XMVectorSet(1.f, 1.f, 1.f, 0.f), &XMVectorSet(0.f, 0.f, 0.f, 0.f), &rot4, &pos3);
#else
	D3DXMatrixTransformation(&m, NULL, NULL, NULL, NULL, &rot4, &pos3); //!! 적당한 함수인지 확인 합니다
#endif

	CameraCross.CopyCameraMatrix((D3DXMATRIX&)m);
}

//------------------------------------------------------------------------------------------------

int CBsBone::s_nCalcAniPosition = BS_CALC_POSITION_Y;

CBsBone::CBsBone()
{
	m_nBoneIndex=-1;
	m_bRootBone=false;
	m_nChildCount=0;
	m_ppChildList=NULL;
	m_pParentBone = NULL;

	m_nCurBlendCount = 0;
	m_nAniCount=0;
#if _OPT_SHINJICH_BSBONE_CPP
	m_pBoneRotationflag = FALSE;
#else
	m_pBoneRotation = NULL;
#endif
	m_pAniType=NULL;
	m_ppAniInfo=NULL;

	BsMatrixIdentity( &m_matVertexTransMatrix );	
	m_pParentTransMat = NULL;
}

CBsBone::~CBsBone()
{
	int i;

	ClearAni();
	if(m_ppChildList){
		for(i=0;i<m_nChildCount;i++){
			delete m_ppChildList[i];
		}
		free( m_ppChildList );
		m_ppChildList=NULL;
	}
}

void CBsBone::ClearAni()
{
	int i;

	if(m_ppAniInfo){
		for(i=0;i<m_nAniCount;i++){
			delete m_ppAniInfo[i];
		}
		delete [] m_ppAniInfo;
		m_ppAniInfo=NULL;
	}
	if(m_pAniType){
		delete [] m_pAniType;
		m_pAniType=NULL;
	}
}

CBsBone *CBsBone::LoadBone(BStream* pStream, CBsBone *pRootBone, int nAniCount)
{
	int i;
	char szBoneName[255], szParentName[255];
	CBsBone *pCurBone=NULL, *pParentBone=NULL;

	pStream->Read(szBoneName, 255);
	pStream->Read(szParentName, 255);
	// mruete: prefix bug 300: force-terminate the buffers for added safety against mal-formed data
	szBoneName[254] = 0;
	szParentName[254] = 0;
	if(pRootBone){
		pCurBone=pRootBone->FindBone(szBoneName);
	}
	if(!pCurBone){
		pCurBone=new CBsBone();
		strcpy(pCurBone->m_szBoneName, szBoneName);
		strcpy(pCurBone->m_szParentName, szParentName);
	}
	if(pRootBone){
		pParentBone=pRootBone->FindBone(szParentName);
	}
	if(pParentBone){
		pParentBone->AddChild(pCurBone);
		for(i=0;i<pRootBone->m_nChildCount;i++){
			if(strcmp(pRootBone->m_ppChildList[i]->m_szParentName, pCurBone->m_szBoneName)==0){
				pCurBone->AddChild(pRootBone->m_ppChildList[i]);
				pRootBone->RemoveChildPtr(i);
			}
		}
	}
	else{
		if(pRootBone){
			pRootBone->AddChild(pCurBone);
		}
		else{
			pRootBone=pCurBone;
		}
	}
	pCurBone->LoadBoneInfo( pStream, nAniCount );

	return pRootBone;
}

void CBsBone::LoadBoneInfo(BStream* pStream, int nAniCount)
{
	int i;

	pStream->Read(&m_matInverseWorld, sizeof(D3DXMATRIX), ENDIAN_FOUR_BYTE);
	pStream->Read(&m_matLocal, sizeof(D3DXMATRIX), ENDIAN_FOUR_BYTE);
	pStream->Read(&m_LocalAffine.Translation, sizeof( float ) * 3, ENDIAN_FOUR_BYTE);
	pStream->Read(&m_LocalAffine.Rotation, sizeof( float ) * 4, ENDIAN_FOUR_BYTE);
	pStream->Read(&m_LocalAffine.StretchRotation, sizeof( float ) * 4, ENDIAN_FOUR_BYTE);
	pStream->Read(&m_LocalAffine.Scale, sizeof( float ) * 3, ENDIAN_FOUR_BYTE);

#ifdef _XBOX
	XMVECTOR vecDet;
	m_matWorld = XMMatrixInverse(&vecDet, m_matInverseWorld);
#else
	D3DXMatrixInverse(&m_matWorld, NULL, &m_matInverseWorld);
#endif

	ClearAni();

	m_nAniCount=nAniCount;
	if(m_nAniCount){
		m_ppAniInfo=new CBsAniSampledKeyFrame *[m_nAniCount];
		m_pAniType=new int[m_nAniCount];
		for(i=0;i<m_nAniCount;i++){
			pStream->Read(m_pAniType+i, sizeof(int), ENDIAN_FOUR_BYTE);
			m_ppAniInfo[i]=new CBsAniSampledKeyFrame();
			m_ppAniInfo[i]->LoadAniInfo( pStream );
		}
	}

	return;
}

int CBsBone::SetAnimationIndex(int nIndex)
{
	int i;

	m_nBoneIndex=nIndex;
	for(i=0;i<m_nChildCount;i++){
		nIndex=m_ppChildList[i]->SetAnimationIndex(nIndex+1);
	}

	return nIndex;
}

CBsBone *CBsBone::FindBone(const char *pBoneName)
{
	int i;
	CBsBone *pSearchBone;

	if(strcmp(pBoneName, m_szBoneName)==0){
		return this;
	}
	for(i=0;i<m_nChildCount;i++){
		pSearchBone=m_ppChildList[i]->FindBone(pBoneName);
		if(pSearchBone){
			return pSearchBone;
		}
	}

	return NULL;
}

CBsBone *CBsBone::FindBone(int nIndex)
{
	int i;
	CBsBone *pSearchBone;

	if(nIndex==m_nBoneIndex){
		return this;
	}
	for(i=0;i<m_nChildCount;i++){
		pSearchBone=m_ppChildList[i]->FindBone(nIndex);
		if(pSearchBone){
			return pSearchBone;
		}
	}

	return NULL;
}

int CBsBone::GetChildNodeCount()
{
	int i, nNodeCount;

	nNodeCount=0;
	for(i=0;i<m_nChildCount;i++){
		nNodeCount+=m_ppChildList[i]->GetChildNodeCount();
	}

	return nNodeCount+1;
}

int CBsBone::FindBoneIndex(const char *pBoneName)
{
	int i;
	int nBoneIndex;

	if(strcmp(pBoneName, m_szBoneName)==0){
		return m_nBoneIndex;
	}
	for(i=0;i<m_nChildCount;i++){
		nBoneIndex=m_ppChildList[i]->FindBoneIndex(pBoneName);
		if(nBoneIndex!=-1){
			return nBoneIndex;
		}
	}

	return -1;
}

void CBsBone::AddChild(CBsBone *pChild)
{
#ifdef _DEBUG
	int i;

	for(i=0;i<m_nChildCount;i++){
		if(strcmp(pChild->GetBoneName(), m_ppChildList[i]->GetBoneName())==0){
			BsAssert(0);
		}
	}
#endif // #ifdef _DEBUG
	m_nChildCount++;
	m_ppChildList=(CBsBone **)realloc(m_ppChildList, sizeof(CBsBone *)*m_nChildCount);
	m_ppChildList[m_nChildCount-1]=pChild;
	pChild->SetParentBone(this);
}

void CBsBone::RemoveChildPtr(CBsBone *pChild)
{
	int i;

	for(i=0;i<m_nChildCount;i++){
		if(m_ppChildList[i]==pChild){
			memcpy(m_ppChildList, m_ppChildList+i+1, sizeof(CBsBone *)*(m_nChildCount-i-1));
			m_nChildCount--;
			return;
		}
	}
	BsAssert(0 && "Failed RemoveChildPtr() : No find bone!!" );
}

void CBsBone::RemoveChildPtr(int nIndex)
{
	BsAssert(nIndex<m_nChildCount);
	memcpy(m_ppChildList, m_ppChildList+nIndex+1, sizeof(CBsBone *)*(m_nChildCount-nIndex-1));
	m_nChildCount--;
}

void CBsBone::SetBoneRotation( D3DXVECTOR3 *pRotation )
{
	// - shinjich
#if _OPT_SHINJICH_BSBONE_CPP
	m_pBoneRotationflag = TRUE;
	memcpy( &m_pBoneRotation.x, pRotation, sizeof(D3DXVECTOR3) );
#else
	m_pBoneRotation = pRotation;
#endif
}


/*void CBsBone::CalculateAnimation()
{
int i;
D3DXMATRIX matAni;

GetAnimationMatrix( matAni );
D3DXMatrixMultiply( &m_matTransMatrix, &matAni, m_pParentTransMat );
if( m_pBoneRotation )
{
D3DXVECTOR3 Pos;
D3DXMATRIX Mat;

memcpy( &Pos, &m_matTransMatrix._41, sizeof( D3DXVECTOR3 ) );
D3DXMatrixRotationYawPitchRoll( &Mat, m_pBoneRotation->y / 180.0f * D3DX_PI, 
m_pBoneRotation->x / 180.0f * D3DX_PI, m_pBoneRotation->z / 180.0f * D3DX_PI );
D3DXMatrixMultiply( &m_matTransMatrix, &m_matTransMatrix, &Mat );
memcpy( &m_matTransMatrix._41, &Pos, sizeof( D3DXVECTOR3 ) );
}
D3DXMatrixMultiply( &m_matVertexTransMatrix, &m_matInverseWorld, &m_matTransMatrix );

for( i = 0; i < m_nChildCount; i++ )
{
m_ppChildList[ i ]->SetParentTransMat( &m_matTransMatrix );
}
}*/

void CBsBone::SetAnimationFrameBone( int nAni, float fFrame )
{
	int i;

	m_nAniIndex = nAni;
	m_fFrame = fFrame;
	for( i = 0; i < m_nChildCount; i++ )
	{
		m_ppChildList[ i ]->SetAnimationFrameBone( nAni, fFrame );
	}
}

void CBsBone::BlendAnimationFrame(int nBlendAni, float fBlendFrame, float fBlendWeight)
{
	BsAssert( m_nCurBlendCount < MAX_BLEND_ANI_COUNT );

	if(m_nCurBlendCount >= MAX_BLEND_ANI_COUNT) {
		DebugString("Max Blend Count Exceed!!\n");
	}
	if( m_nAniCount <= nBlendAni ) {
		nBlendAni = 0;
		DebugString("#### %d %d blend animation fail ###\n", m_nAniCount, nBlendAni);
	}
	m_nBlendIndex[ m_nCurBlendCount ] = nBlendAni;
	m_fBlendFrame[ m_nCurBlendCount ] = fBlendFrame;
	m_fBlendWeight[ m_nCurBlendCount ] = fBlendWeight;
	m_nCurBlendCount++;

	int i;

	for( i = 0; i < m_nChildCount; i++ )
	{
		m_ppChildList[ i ]->BlendAnimationFrame( nBlendAni, fBlendFrame, fBlendWeight );
	}
}

void CBsBone::ComputeVertexTransMatrix()
{
	int i;

	BsMatrixMultiply(&m_matVertexTransMatrix, &m_matInverseWorld, &m_matTransMatrix);
	for(i=0;i<m_nChildCount;i++){
		m_ppChildList[i]->ComputeVertexTransMatrix();
	}
}

/*void CBsBone::GetPositionVector(D3DXVECTOR3 &Position, int nAni, float fFrame)
{
if( !m_ppAniInfo[ nAni ]->GetPositionVector( Position, fFrame ) )
{
Position = m_LocalAffine.Translation;
}
}

void CBsBone::GetRotationVector(D3DXQUATERNION &Rotation, int nAni, float fFrame)
{
if( !m_ppAniInfo[ nAni ]->GetRotationVector( Rotation, fFrame ) )
{
Rotation = m_LocalAffine.Rotation;
}
}

void CBsBone::GetScalingVector(D3DXVECTOR3 &Scale, D3DXQUATERNION &ScaleRot, int nAni, float fFrame)
{
if( !m_ppAniInfo[ nAni ]->GetScalingVector( Scale, ScaleRot, fFrame ) )
{
Scale = m_LocalAffine.Scale;
ScaleRot = m_LocalAffine.StretchRotation;
}
}*/

/*int CBsBone::GetAnimationMatrix( D3DXMATRIX &matSource )
{
int i;
D3DXVECTOR3 Position, Position2;
D3DXQUATERNION Rotation, Rotation2;
D3DXVECTOR3 Scale;
D3DXQUATERNION ScaleRot;

if( m_bRootBone )
{
GetPositionVector( Position, m_nAniIndex, 0.f );
GetPositionVector( Position2, m_nAniIndex, m_fFrame );
if( s_nCalcAniPosition & BS_CALC_POSITION_X )
{
Position.x = Position2.x;
}
if( s_nCalcAniPosition & BS_CALC_POSITION_Y )
{
Position.y = Position2.y;
}
if( s_nCalcAniPosition & BS_CALC_POSITION_Z )
{
Position.z = Position2.z;
}
}
else
{
GetPositionVector( Position, m_nAniIndex, m_fFrame );
for( i = 0; i < m_nCurBlendCount; i++ )
{
GetPositionVector( Position2, m_nBlendIndex[ i ], m_fBlendFrame[ i ] );
D3DXVec3Lerp( &Position, &Position, &Position2, m_fBlendWeight[ i ] );
}
}

GetRotationVector( Rotation, m_nAniIndex, m_fFrame );
for( i = 0; i < m_nCurBlendCount; i++ )
{
GetRotationVector( Rotation2, m_nBlendIndex[ i ], m_fBlendFrame[ i ] );
D3DXQuaternionSlerp( &Rotation, &Rotation, &Rotation2, m_fBlendWeight[ i ] );
//		D3DXVec4Lerp( ( D3DXVECTOR4 * )&Rotation, ( D3DXVECTOR4 * )&Rotation, ( D3DXVECTOR4 * )&Rotation2, m_fBlendWeight[ i ] );
}

GetScalingVector( Scale, ScaleRot, m_nAniIndex, m_fFrame );

D3DXMatrixTransformation(&matSource, NULL, &ScaleRot, &Scale, NULL, &Rotation, &Position);

ResetBlendBuffer();

return 1;
}*/


CBsBone* CBsBone::GetChildPtr(int nIndex) const
{
	BsAssert(nIndex < m_nChildCount);
	return m_ppChildList[nIndex];
}

void CBsBone::AddAni(CBsBone *pRootBone, int nIndex)
{
	int i, nAniCount, nDestAniCount;
	CBsBone *pBone;

	if(!pRootBone){
		return;
	}
	nDestAniCount=pRootBone->GetAniCount();
	nAniCount=nDestAniCount+m_nAniCount;
	pBone=pRootBone->FindBone(m_szBoneName);
	if(pBone){
		m_ppAniInfo=(CBsAniSampledKeyFrame **)realloc(m_ppAniInfo, sizeof(CBsAniSampledKeyFrame *)*nAniCount);
		m_pAniType=(int *)realloc(m_pAniType, sizeof(int)*nAniCount);
		memmove(m_ppAniInfo+nIndex+nDestAniCount, m_ppAniInfo+nIndex, sizeof(CBsAniSampledKeyFrame *)*(m_nAniCount-nIndex));
		memmove(m_pAniType+nIndex+nDestAniCount, m_pAniType+nIndex, sizeof(int)*(m_nAniCount-nIndex));
		for(i=nIndex;i<nIndex+nDestAniCount;i++){
			m_pAniType[i]=pBone->m_pAniType[i-nIndex];
			m_ppAniInfo[i]=new CBsAniSampledKeyFrame();
			m_ppAniInfo[i]->CopyAni(pBone->m_ppAniInfo[i-nIndex]);
		}
	}
	m_nAniCount=nAniCount;
	for(i=0;i<m_nChildCount;i++){
		m_ppChildList[i]->AddAni(pRootBone, nIndex);
	}
}

bool CBsBone::CheckAddAni(CBsBone *pRootBone)
{
	int i;

	if(!pRootBone->FindBone(m_szBoneName)){
#ifndef _XBOX
		char buf[1024];
		sprintf( buf, "Can't find bone %s in anim %d", m_szBoneName, m_nAniCount );
		MessageBox( 0, buf, "Error", MB_OK );
#endif
		return false;
	}
	for(i=0;i<m_nChildCount;i++){
		if(!m_ppChildList[i]->CheckAddAni(pRootBone)){
			return false;
		}
	}

	return true;
}

void CBsBone::DeleteAni(int nIndex)
{
	int i;

	delete m_ppAniInfo[nIndex];
	memcpy(m_pAniType+nIndex, m_pAniType+nIndex+1, sizeof(int)*(m_nAniCount-nIndex-1));
	memcpy(m_ppAniInfo+nIndex, m_ppAniInfo+nIndex+1, sizeof(CBsAniSampledKeyFrame *)*(m_nAniCount-nIndex-1));
	m_nAniCount--;
	for(i=0;i<m_nChildCount;i++){
		m_ppChildList[i]->DeleteAni(nIndex);
	}
}

void CBsBone::GetAniDistance(GET_ANIDISTANCE_INFO *pInfo)
{
	BSVECTOR Pos1, Pos2;

	GetPositionVector(Pos1, pInfo->nAni1, pInfo->fFrame1);
	GetPositionVector(Pos2, pInfo->nAni2, pInfo->fFrame2);
	//	*(pInfo->pVector)=Pos1-Pos2;
	BSVECTOR vecRes = Pos1-Pos2;
	memcpy(pInfo->pVector, &vecRes, sizeof(BSVECTOR));
}

void CBsBone::DeleteAnimationKey()
{
	int i;

	for( i = 0; i < m_nAniCount; i++ )
	{
		m_ppAniInfo[ i ]->DeleteAnimationKey();
	}
}
