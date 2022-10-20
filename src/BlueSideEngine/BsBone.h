#pragma once

#include "BsKernel.h"
#include "CrossVector.h"
#include "BsMath.h"

#ifdef _XBOX
#include <vectorintrinsics.h>
#endif

// - shinjich
#define _OPT_SHINJICH_BSBONE_H	0

//#define USE_POSITION_INDEX_TABLE
//#define USE_ROTATION_INDEX_TABLE
//------------------------------------------------------------------------------------------------
#ifndef _USAGE_TOOL_
#define USE_COMPRESS_TABLE
#endif

#define MAX_BLEND_ANI_COUNT 5

struct POSITION_ANI
{
	int nTime;
	BSVECTOR Position;
};

struct ROTATION_ANI
{
	int				nTime;
	BSQUATERNION	Rotation;
};

struct SCALING_ANI
{
	int				nTime;
	BSVECTOR		Scale;
	BSQUATERNION	ScaleRot;
};

struct POSITION_ANI_SOA
{	
#ifdef USE_COMPRESS_TABLE
	WORD *nTime;
	WORD *Position;
	BSVECTOR vMin;
	BSVECTOR vAdv;
#else
	int *nTime;
	BSVECTOR *Position;
#endif
};

struct ROTATION_ANI_SOA
{
#ifdef USE_COMPRESS_TABLE
	WORD	*nTime;
#if _OPT_SHINJICH_BSBONE_H
	short	*Rotation;
#else
	WORD	*Rotation;
#endif
#else
	int				*nTime;
	BSQUATERNION	*Rotation;
#endif
};

struct SCALING_ANI_SOA
{
	int				*nTime;
	BSVECTOR	*Scale;
	BSQUATERNION	*ScaleRot;
};

struct AFFINE_PARTS
{
	BSVECTOR		Translation;
	BSQUATERNION	Rotation;
	BSQUATERNION	StretchRotation;
	BSVECTOR		Scale;
};

class BStream;

//------------------------------------------------------------------------------------------------

class CBsAniSampledKeyFrame
{
public:
	CBsAniSampledKeyFrame();
	virtual ~CBsAniSampledKeyFrame();

protected:
	int m_nPositionKeyCount;
	int m_nRotationKeyCount;
	int	m_nScalingKeyCount;

	POSITION_ANI_SOA m_PositionAni;
	ROTATION_ANI_SOA m_RotationAni;
	SCALING_ANI_SOA  m_ScalingAni;

#ifdef USE_POSITION_INDEX_TABLE
	std::vector< short int > m_PositionKeyIndex;
#endif
#ifdef USE_ROTATION_INDEX_TABLE
	std::vector< short int > m_RotationKeyIndex;
#endif

public:
	int  LoadAniInfo(BStream* pStream);
	int  SaveAniInfo(BStream* pStream);

	void CopyAni(CBsAniSampledKeyFrame *pAni);

#ifdef USE_POSITION_INDEX_TABLE
	__forceinline int  GetPositionVector(BSVECTOR &Position, float fFrame)
	{
		if( m_nPositionKeyCount > 1 )
		{
			int nIndex;
			float fWeight;
			nIndex = m_PositionKeyIndex[ ( int )fFrame ];
			if( m_pPositionAni[ nIndex ].nTime == fFrame )
			{
				Position = m_pPositionAni[ nIndex ].Position;
			}
			else
			{
				fWeight = ( ( float )( fFrame - m_pPositionAni[ nIndex ].nTime ) ) / ( m_pPositionAni[ nIndex + 1].nTime - m_pPositionAni[ nIndex ].nTime );
				D3DXVec3Lerp( &Position, &( m_pPositionAni[ nIndex ].Position ), &( m_pPositionAni[ nIndex + 1 ].Position ), fWeight );
			}
		}
		else if( m_nPositionKeyCount == 1 )
		{
			Position=m_PositionAni.Position[0];
		}
		else{
			return 0;
		}

		return 1;
	}
#else
	__forceinline int  GetPositionVector(BSVECTOR &Position, float fFrame)
	{
		int i;
		float fWeight;
		int nKeyCount, nRangeStart, nRangeEnd;

		nKeyCount=m_nPositionKeyCount;
		if(nKeyCount>1){			
			nRangeStart=0;
			nRangeEnd=nKeyCount-1;
			while(1){
				i=(nRangeStart+nRangeEnd)>>1;
				if(m_PositionAni.nTime[i]==fFrame){
#ifdef USE_COMPRESS_TABLE
#if _OPT_SHINJICH_BSBONE_H
					// - shinjich
					static const __declspec(align(16)) DWORD constvbias[4] = { 32768, 32768, 32768, 32768 };
					BSQUATERNION temp1, vbias;

					__vector4 *pPosition = (__vector4*) &m_PositionAni.Position[i*4];
					vbias = __lvx( constvbias, 0 );
					if ( ((DWORD) pPosition & 0xf) )
					{
						pPosition = (__vector4*) &m_PositionAni.Position[(i-1)*4];
						temp1 = __vupklsh( *pPosition );
					}
					else
					{
						temp1 = __vupkhsh( *pPosition );
					}
					temp1 = __vaddsws( temp1, vbias );
					temp1 = __vcfsx( temp1, 0 );
					Position = __vmaddfp( temp1, m_PositionAni.vAdv, m_PositionAni.vMin );
/*
					// d3dpack version
					__vector4* vpointer = ((__vector4*) &m_PositionAni.Position[i*4]);
					if ( ((DWORD) vpointer & 0xf) )
					{
						__vector4 packed = *((__vector4*) &m_PositionAni.Position[(i-1)*4]);
						packed = __vsldoi( packed, packed, 8 );
						Position = __vupkd3d( packed, VPACK_FLOAT16_4 );
					}
					else
					{
						Position = __vupkd3d( *vpointer, VPACK_FLOAT16_4 );
					}
*/
#else
					// original
					WORD *pPosition = &m_PositionAni.Position[i*3];
//					WORD *pPosition = &m_PositionAni.Position[i*4];
					Position.x =  pPosition[0] * m_PositionAni.vAdv.x + m_PositionAni.vMin.x ;
					Position.y = pPosition[1] * m_PositionAni.vAdv.y +  m_PositionAni.vMin.y ;
					Position.z =  pPosition[2] * m_PositionAni.vAdv.z + m_PositionAni.vMin.z ;
#endif
#else
					Position=m_PositionAni.Position[i];
#endif
					break;
				}
				else if(m_PositionAni.nTime[i]<fFrame){
					if(m_PositionAni.nTime[i+1]>=fFrame){
						i++;
					}
					else{
						nRangeStart=i+1;
						continue;
					}
				}
				else{
					if(m_PositionAni.nTime[i-1]>fFrame){
						nRangeEnd=i-1;
						continue;
					}
				}
				fWeight=((float)(fFrame-m_PositionAni.nTime[i-1]))/(m_PositionAni.nTime[i]-m_PositionAni.nTime[i-1]);
#ifdef USE_COMPRESS_TABLE
				BSVECTOR Position1, Position2;
#if _OPT_SHINJICH_BSBONE_H
				// - shinjich
				static const __declspec(align(16)) DWORD constvbias[4] = { 32768, 32768, 32768, 32768 };
				BSQUATERNION temp1, temp2, vbias;

				__vector4 *pPosition = (__vector4*) &m_PositionAni.Position[(i-1)*4];
				vbias = __lvx( constvbias, 0 );
				if ( ((DWORD) pPosition & 0xf) )
				{
					pPosition = (__vector4*) &m_PositionAni.Position[(i-2)*4];
					temp1 = __vupklsh( *pPosition );
					pPosition = (__vector4*) &m_PositionAni.Position[i*4];
					temp2 = __vupkhsh( *pPosition );
				}
				else
				{
					temp1 = __vupkhsh( *pPosition );
					temp2 = __vupklsh( *pPosition );
				}
				temp1 = __vaddsws( temp1, vbias );
				temp2 = __vaddsws( temp2, vbias );
				temp1 = __vcfsx( temp1, 0 );
				temp2 = __vcfsx( temp2, 0 );
				Position1 = __vmaddfp( temp1, m_PositionAni.vAdv, m_PositionAni.vMin );
				Position2 = __vmaddfp( temp2, m_PositionAni.vAdv, m_PositionAni.vMin );
/*
				// d3dpack version
				__vector4* vpointer = ((__vector4*) &m_PositionAni.Position[(i-1)*4]);
				if ( ((DWORD) vpointer & 0xf) )
				{
					__vector4 packed = *((__vector4*) &m_PositionAni.Position[(i-2)*4]);
					packed = __vsldoi( packed, packed, 8 );
					Position1 = __vupkd3d( packed, VPACK_FLOAT16_4 );
					__vector4 packed2 = *((__vector4*) &m_PositionAni.Position[i*4]);
					Position2 = __vupkd3d( packed2, VPACK_FLOAT16_4 );
				}
				else
				{
					__vector4 packed = *vpointer;
					Position1 = __vupkd3d( packed, VPACK_FLOAT16_4 );
					__vector4 packed2 = __vsldoi( packed, packed, 8 );
					Position2 = __vupkd3d( packed2, VPACK_FLOAT16_4 );
				}
*/
#else
				//original
				WORD *pPosition = &m_PositionAni.Position[i*3-3];
//				WORD *pPosition = &m_PositionAni.Position[i*4-4];

				Position1.x =  pPosition[0] * m_PositionAni.vAdv.x + m_PositionAni.vMin.x ;
				Position1.y = pPosition[1] * m_PositionAni.vAdv.y +  m_PositionAni.vMin.y ;
				Position1.z =  pPosition[2] * m_PositionAni.vAdv.z + m_PositionAni.vMin.z ;
				Position2.x =  pPosition[3] * m_PositionAni.vAdv.x + m_PositionAni.vMin.x ;
				Position2.y = pPosition[4] * m_PositionAni.vAdv.y +  m_PositionAni.vMin.y ;
				Position2.z =  pPosition[5] * m_PositionAni.vAdv.z + m_PositionAni.vMin.z ;
/*
				Position1.x =  pPosition[0] * m_PositionAni.vAdv.x + m_PositionAni.vMin.x ;
				Position1.y = pPosition[1] * m_PositionAni.vAdv.y +  m_PositionAni.vMin.y ;
				Position1.z =  pPosition[2] * m_PositionAni.vAdv.z + m_PositionAni.vMin.z ;
				Position2.x =  pPosition[4] * m_PositionAni.vAdv.x + m_PositionAni.vMin.x ;
				Position2.y = pPosition[5] * m_PositionAni.vAdv.y +  m_PositionAni.vMin.y ;
				Position2.z =  pPosition[6] * m_PositionAni.vAdv.z + m_PositionAni.vMin.z ;
*/
#endif
				BsVec3Lerp(&Position, &Position1, &Position2, fWeight);
#else
				BsVec3Lerp(&Position, &(m_PositionAni.Position[i-1]), &(m_PositionAni.Position[i]), fWeight);
#endif
				break;
			}
		}
		else if(nKeyCount==1){
#ifdef USE_COMPRESS_TABLE
#if _OPT_SHINJICH_BSBONE_H
			// - shinjich
			static const __declspec(align(16)) DWORD constvbias[4] = { 32768, 32768, 32768, 32768 };
			BSQUATERNION temp1, vbias;

			__vector4 *pPosition = (__vector4*) &m_PositionAni.Position[0];
			vbias = __lvx( constvbias, 0 );
			temp1 = __vupkhsh( *pPosition );
			temp1 = __vaddsws( temp1, vbias );
			temp1 = __vcfsx( temp1, 0 );
			Position = __vmaddfp( temp1, m_PositionAni.vAdv, m_PositionAni.vMin );
/*
			// d3dpack version
			__vector4 packed = *((__vector4*) &m_PositionAni.Position[0]);
//			packed = __vsldoi( packed, packed, 8 );
			Position = __vupkd3d( packed, VPACK_FLOAT16_4 );
*/
#else
			//original
			Position.x =  m_PositionAni.Position[0] * m_PositionAni.vAdv.x + m_PositionAni.vMin.x ;
			Position.y = m_PositionAni.Position[1] * m_PositionAni.vAdv.y +  m_PositionAni.vMin.y ;
			Position.z =  m_PositionAni.Position[2] * m_PositionAni.vAdv.z + m_PositionAni.vMin.z ;
#endif
#else
			Position=m_PositionAni.Position[0];
#endif
		}
		else{
			return 0;
		}
		return 1;
	}
#endif

#ifdef USE_ROTATION_INDEX_TABLE
	__forceinline int  GetRotationVector(D3DXQUATERNION &Rotation, float fFrame)
	{
		if( m_nRotationKeyCount > 1 )
		{
			int nIndex;
			float fWeight;
			nIndex = m_RotationKeyIndex[ ( int )fFrame ];
			if( m_pRotationAni[ nIndex ].nTime == fFrame )
			{
				Rotation = m_pRotationAni[ nIndex ].Rotation;
			}
			else
			{
				fWeight = ( ( float )( fFrame - m_pRotationAni[ nIndex ].nTime ) ) / ( m_pRotationAni[ nIndex + 1].nTime - m_pRotationAni[ nIndex ].nTime );
				D3DXVec4Lerp( ( D3DXVECTOR4 * )&Rotation, ( D3DXVECTOR4 * )&( m_pRotationAni[ nIndex ].Rotation ), 
					( D3DXVECTOR4 * )&( m_pRotationAni[ nIndex + 1 ].Rotation ), fWeight );
			}
		}
		else if( m_nRotationKeyCount == 1 )
		{
			Rotation = m_pRotationAni[ 0 ].Rotation;
		}
		else{
			return 0;
		}
		return 1;
	}
#else
	__forceinline int  GetRotationVector(BSQUATERNION &Rotation, float fFrame)
	{
		int i;
		float fWeight;
		int nKeyCount, nCurrentTime, nRangeStart, nRangeEnd;

		nKeyCount=m_nRotationKeyCount;
		if(nKeyCount>1){
			nRangeStart=0;
			nRangeEnd=nKeyCount-1;
			while(1){
				i=(nRangeStart+nRangeEnd)>>1;
				nCurrentTime=m_RotationAni.nTime[i];
				if(nCurrentTime==fFrame){
#ifdef USE_COMPRESS_TABLE
#if _OPT_SHINJICH_BSBONE_H
					// - shinjich
					BSQUATERNION temp1, vbias;
					static const __declspec(align(16)) float constfbias[4] = { 1.0f / 32767.5f, 1.0f / 32767.5f, 1.0f / 32767.5f, 1.0f / 32767.5f };
					__vector4 *pRotation = (__vector4*) &m_RotationAni.Rotation[i*4];
					if ( ((DWORD) pRotation & 0xf) )
					{
						pRotation = (__vector4*) &m_RotationAni.Rotation[(i-1)*4];
						vbias = __lvx( constfbias, 0 );
						temp1 = __vupklsh( *pRotation );
					}
					else
					{
						vbias = __lvx( constfbias, 0 );
						temp1 = __vupkhsh( *pRotation );
					}
					temp1 = __vcfsx( temp1, 0 );
					Rotation = __vmulfp( temp1, vbias );
#else
					// original
					const float fBias = 1 / 32767.5f;
					WORD *pRotation = &m_RotationAni.Rotation[i*4];

					Rotation.x =  pRotation[0] * fBias - 1.f;
					Rotation.y = pRotation[1] * fBias  - 1.f;
					Rotation.z =  pRotation[2] * fBias - 1.f;
					Rotation.w =  pRotation[3] * fBias - 1.f;
/*
					Rotation.x =  pRotation[0] * fBias;
					Rotation.y = pRotation[1] * fBias;
					Rotation.z =  pRotation[2] * fBias;
					Rotation.w =  pRotation[3] * fBias;
*/
#endif
#else
					Rotation=m_RotationAni.Rotation[i];
#endif
					break;
				}
				else if(nCurrentTime<fFrame){
					if(m_RotationAni.nTime[i+1]>=fFrame){
						i++;
						nCurrentTime=m_RotationAni.nTime[i];
					}
					else{
						nRangeStart=i+1;
						continue;
					}
				}
				else{
					if(m_RotationAni.nTime[i-1]>fFrame){
						nRangeEnd=i-1;
						continue;
					}
				}
				fWeight=((float)(fFrame-m_RotationAni.nTime[i-1]))/(nCurrentTime-m_RotationAni.nTime[i-1]);

#ifdef USE_COMPRESS_TABLE
#if _OPT_SHINJICH_BSBONE_H
				// - shinjich
				__vector4 *pRotation = (__vector4*) &m_RotationAni.Rotation[(i-1)*4];
				BSQUATERNION Rotation1, Rotation2;
//				BSQUATERNION temp1, temp2, mask1, mask2, vzero, vmask, vbias, vminus1;
				BSQUATERNION temp1, temp2, vbias;
//				static const __declspec(align(16)) DWORD constvzero[4] = { 0, 0, 0, 0 };
//				static const __declspec(align(16)) DWORD constvmask[4] = { 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000 };
				static const __declspec(align(16)) float constfbias[4] = { 1.0f / 32767.5f, 1.0f / 32767.5f, 1.0f / 32767.5f, 1.0f / 32767.5f };
//				static const __declspec(align(16)) float constminus1[4] = { -1.0f, -1.0f, -1.0f, -1.0f };
#if 0
				// for debug...
				Rotation1.x = pRotation[0];
				Rotation1.y = pRotation[1];
				Rotation1.z = pRotation[2];
				Rotation1.w = pRotation[3];

				Rotation2.x = pRotation[4];
				Rotation2.y = pRotation[5];
				Rotation2.z = pRotation[6];
				Rotation2.w = pRotation[7];
#endif
				if ( ((DWORD) pRotation & 0xf) )
				{
					pRotation = (__vector4*) &m_RotationAni.Rotation[(i-2)*4];
					vbias = __lvx( constfbias, 0 );
					temp1 = __vupklsh( *pRotation );
					pRotation = (__vector4*) &m_RotationAni.Rotation[i*4];
					temp2 = __vupkhsh( *pRotation );
				}
				else
				{
//					vzero = __lvx( constvzero, 0 );
//					vmask = __lvx( constvmask, 0 );
					vbias = __lvx( constfbias, 0 );
//					vminus1 = __lvx( constminus1, 0 );
					temp1 = __vupkhsh( *pRotation );
					temp2 = __vupklsh( *pRotation );
//					mask1 = __vcmpgtsw( vzero, temp1 );
//					mask2 = __vcmpgtsw( vzero, temp2 );
//					mask1 = __vand( mask1, vmask );
//					mask2 = __vand( mask2, vmask );
//					temp1 = __vxor( temp1, mask1 );
//					temp2 = __vxor( temp2, mask2 );
				}
				temp1 = __vcfsx( temp1, 0 );
				temp2 = __vcfsx( temp2, 0 );
//				Rotation1 = __vmaddfp( temp1, vbias, vminus1 );
//				Rotation2 = __vmaddfp( temp2, vbias, vminus1 );
				Rotation1 = __vmulfp( temp1, vbias );
				Rotation2 = __vmulfp( temp2, vbias );
//				Rotation1 = __vaddfp( Rotation1, vminus1 );
//				Rotation2 = __vaddfp( Rotation2, vminus1 );
#if 0
					// for debug...
					static BSQUATERNION RotationX, RotationY;
					BSQUATERNION RotationH, RotationL;
					const float fBias = 1 / 65536.0f;
					WORD *pRotationx = &m_RotationAni.Rotation[(i-1)*4];

					RotationH.x =  pRotationx[0] * fBias - 1.f;
					RotationH.y = pRotationx[1] * fBias  - 1.f;
					RotationH.z =  pRotationx[2] * fBias - 1.f;
					RotationH.w =  pRotationx[3] * fBias - 1.f;

					RotationL.x =  pRotationx[4] * fBias - 1.f;
					RotationL.y = pRotationx[5] * fBias  - 1.f;
					RotationL.z =  pRotationx[6] * fBias - 1.f;
					RotationL.w =  pRotationx[7] * fBias - 1.f;

				if (
					Rotation1.x != RotationH.x ||
					Rotation1.y != RotationH.y ||
					Rotation1.z != RotationH.z ||
					Rotation1.w != RotationH.w ||
					Rotation2.x != RotationL.x ||
					Rotation2.y != RotationL.y ||
					Rotation2.z != RotationL.z ||
					Rotation2.w != RotationL.w
				)
				{
					RotationX = __vsubfp( Rotation1, RotationH );
					RotationY = __vsubfp( Rotation2, RotationL );
				}
#endif
/*
				// d3dpack version
				BSQUATERNION Rotation1, Rotation2;
				__vector4 packed = *((__vector4*) &m_RotationAni.Rotation[(i-1)*4]);
				Rotation1 = __vupkd3d( packed, VPACK_FLOAT16_4 );
				__vector4 packed2 = __vsldoi( packed, packed, 8 );
				Rotation2 = __vupkd3d( packed2, VPACK_FLOAT16_4 );
*/
#else
				// original
				const float fBias = 1 / 32767.5f;
				WORD *pRotation = &m_RotationAni.Rotation[(i-1)*4];
				BSQUATERNION Rotation1, Rotation2;

				Rotation1.x =  pRotation[0] * fBias - 1.f;
				Rotation1.y = pRotation[1] * fBias  - 1.f;
				Rotation1.z =  pRotation[2] * fBias - 1.f;
				Rotation1.w =  pRotation[3] * fBias - 1.f;

				Rotation2.x =  pRotation[4] * fBias - 1.f;
				Rotation2.y = pRotation[5] * fBias  - 1.f;
				Rotation2.z =  pRotation[6] * fBias - 1.f;
				Rotation2.w =  pRotation[7] * fBias - 1.f;
/*
				Rotation1.x =  pRotation[0] * fBias;
				Rotation1.y = pRotation[1] * fBias;
				Rotation1.z =  pRotation[2] * fBias;
				Rotation1.w =  pRotation[3] * fBias;

				Rotation2.x =  pRotation[4] * fBias;
				Rotation2.y = pRotation[5] * fBias;
				Rotation2.z =  pRotation[6] * fBias;
				Rotation2.w =  pRotation[7] * fBias;
*/
#endif
				BsQuaternionSlerp( &Rotation, &Rotation1, &Rotation2, fWeight );
#else
				BsQuaternionSlerp( &Rotation, &m_RotationAni.Rotation[ i - 1 ], &m_RotationAni.Rotation[ i ], fWeight );
				//				D3DXVec4Lerp( ( D3DXVECTOR4 * )&Rotation, ( D3DXVECTOR4 * )&( m_RotationAni[ i - 1 ].Rotation ), 
				//					( D3DXVECTOR4 * )&( m_RotationAni[ i ].Rotation ), fWeight );
#endif
				break;
			}
		}
		else if(nKeyCount==1){
#ifdef USE_COMPRESS_TABLE
#if _OPT_SHINJICH_BSBONE_H
			// - shinjich
			BSQUATERNION temp1, vbias;
			static const __declspec(align(16)) float constfbias[4] = { 1.0f / 32767.5f, 1.0f / 32767.5f, 1.0f / 32767.5f, 1.0f / 32767.5f };
			__vector4 *pRotation = (__vector4*) &m_RotationAni.Rotation[0];

			vbias = __lvx( constfbias, 0 );
			temp1 = __vupkhsh( *pRotation );
			temp1 = __vcfsx( temp1, 0 );
			Rotation = __vmulfp( temp1, vbias );
#else
			// original
			const float fBias = 1 / 32767.5f;
			WORD *pRotation = m_RotationAni.Rotation;

			Rotation.x =  pRotation[0] * fBias - 1.f;
			Rotation.y = pRotation[1] * fBias  - 1.f;
			Rotation.z =  pRotation[2] * fBias - 1.f;
			Rotation.w =  pRotation[3] * fBias - 1.f;
/*
			Rotation.x =  pRotation[0] * fBias;
			Rotation.y = pRotation[1] * fBias;
			Rotation.z =  pRotation[2] * fBias;
			Rotation.w =  pRotation[3] * fBias;
*/
#endif
#else
			Rotation=m_RotationAni.Rotation[0];
#endif
		}
		else{
			return 0;
		}

		return 1;
	}
#endif
	__forceinline int  GetScalingVector(BSVECTOR &Scale, BSQUATERNION &ScaleRot, float fFrame)
	{
		int i;
		float fWeight;
		int nKeyCount, nCurrentTime, nRangeStart, nRangeEnd;
		nKeyCount=m_nScalingKeyCount;
		if(nKeyCount>1) {
			nRangeStart=0;
			nRangeEnd=nKeyCount-1;
			while(1){
				i=(nRangeStart+nRangeEnd)>>1;
				nCurrentTime=m_ScalingAni.nTime[i];
				if(nCurrentTime==fFrame){
					Scale=m_ScalingAni.Scale[i];
					ScaleRot=m_ScalingAni.ScaleRot[i];
					break;
				} else if(nCurrentTime<fFrame){
					if(m_ScalingAni.nTime[i+1]>=fFrame) {
						i++;
						nCurrentTime=m_ScalingAni.nTime[i];
					} else {
						nRangeStart=i+1;
						continue;
					}
				} else {
					if(m_ScalingAni.nTime[i-1]>fFrame){
						nRangeEnd=i-1;
						continue;
					}
				}
				fWeight=((float)(fFrame-m_ScalingAni.nTime[i-1]))/(nCurrentTime-m_ScalingAni.nTime[i-1]);
				BsVec3Lerp(&Scale, &(m_ScalingAni.Scale[i-1]), &(m_ScalingAni.Scale[i]), fWeight);
				BsQuaternionSlerp(&ScaleRot, &(m_ScalingAni.ScaleRot[i-1]), &(m_ScalingAni.ScaleRot[i]), fWeight);
				break;
			}
		} else if(nKeyCount==1) {
			Scale=m_ScalingAni.Scale[0];
			ScaleRot=m_ScalingAni.ScaleRot[0];
		} else {
			return 0;
		}
		return 1;
	}

	int  GetPositionKeyCount() { return m_nPositionKeyCount; }
	int  GetRotationKeyCount() { return m_nRotationKeyCount; }
	int  GetScalingKeyCount()  { return m_nScalingKeyCount;	}

	POSITION_ANI *GetPositionKey( int nIndex );
	ROTATION_ANI *GetRotationKey( int nIndex );
	SCALING_ANI *GetScalingKey( int nIndex );

	void DeleteAnimationKey();
};

//------------------------------------------------------------------------------------------------

class CBsCameraSampledKeyFrame // 맥스에서 작업한 카메라 익스포트 데이타 입니다 *.BC
{
public:
	CBsCameraSampledKeyFrame();
	virtual ~CBsCameraSampledKeyFrame();

protected:
	D3DXMATRIX m_matInverseWorld;
	D3DXMATRIX m_matLocal;
	AFFINE_PARTS m_LocalAffine;

	int m_nPositionKeyCount;
	int m_nRotationKeyCount;
	int	m_nScalingKeyCount;

	POSITION_ANI *m_pPositionAni;
	ROTATION_ANI *m_pRotationAni;
	SCALING_ANI  *m_pScalingAni; //@@ 삭제 예정 입니다 (카메라에서 스케일 값을 사용하지 않습니다) 

public:

	void	Release(void);

	int  Load(char *szFileName); // *.bc (성공적으로 로드 되었을때 1 을 리턴 합니다)	 
	int  LoadCameraInfo(BStream* pStream);

	int  GetPositionVector(BSVECTOR &Position, float fFrame);
	int  GetRotationVector(BSQUATERNION &Rotation, float fFrame);
	int  GetScalingVector(BSVECTOR &Scale, BSQUATERNION &ScaleRot, float fFrame);

	int  GetPositionKeyCount() { return m_nPositionKeyCount; }
	int  GetRotationKeyCount() { return m_nRotationKeyCount; }
	int  GetScalingKeyCount()  { return m_nScalingKeyCount;	}

	int  GetMaxTime() { return GetPositionKey(GetPositionKeyCount()-1)->nTime; } // 이 값은 항상 존재 합니다 (언제나 익스포트 되는 데이타 입니다)

	POSITION_ANI *GetPositionKey( int nIndex ) { return &m_pPositionAni[nIndex]; }
	ROTATION_ANI *GetRotationKey( int nIndex ) { return &m_pRotationAni[nIndex]; }
	SCALING_ANI *GetScalingKey( int nIndex ) { return &m_pScalingAni[nIndex]; }

	void ProcessCamera(CCrossVector	&CameraCross, float fra); // 해당 프레임의 CameraCross 를 계산합니다
};

//------------------------------------------------------------------------------------------------

extern char g_szDebugStr1[256];
extern char g_szDebugStr2[256];
extern void	OuputUserDebugString1(char *szTemp);
extern void	OuputUserDebugString2(char *szTemp);

class CBsBone
{
public:
	CBsBone();
	virtual ~CBsBone();

protected:
	int m_nBoneIndex;
	bool m_bRootBone;
	char m_szBoneName[255];	// 버퍼
	char m_szParentName[255]; // 버퍼
	int m_nChildCount;
	CBsBone **m_ppChildList; // 메모리
	CBsBone *m_pParentBone; // 메모리

	int m_nAniIndex;
	float m_fFrame;

	int m_nCurBlendCount;
	int m_nBlendIndex[ MAX_BLEND_ANI_COUNT ];
	float m_fBlendFrame[ MAX_BLEND_ANI_COUNT ];
	float m_fBlendWeight[ MAX_BLEND_ANI_COUNT ];

	int m_nAniCount;
	static int s_nCalcAniPosition;

	BSMATRIX m_matInverseWorld;
	BSMATRIX m_matWorld;
	BSMATRIX m_matLocal;
	AFFINE_PARTS m_LocalAffine;
	// -shinjich
#if _OPT_SHINJICH_BSBONE_H
	BOOL m_pBoneRotationflag;
	XMVECTOR m_pBoneRotation;
#else
	D3DXVECTOR3 *m_pBoneRotation;
#endif

	int *m_pAniType; // 메모리
	CBsAniSampledKeyFrame **m_ppAniInfo; // 메모리

	BSMATRIX m_matTransMatrix;
	BSMATRIX m_matVertexTransMatrix; // Identity	
	BSMATRIX *m_pParentTransMat;
public:
	void SetRootBone(bool bRootBone) { m_bRootBone=bRootBone; }
	int  GetBoneIndex() { return m_nBoneIndex; }

	void ClearAni();

	static CBsBone *LoadBone(BStream* pStream, CBsBone* pRootBone, int nAniCount);
	void			LoadBoneInfo(BStream* pStream, int nAniCount);

	int SetAnimationIndex(int nIndex);

	CBsBone *FindBone(const char *pBoneName);
	CBsBone *FindBone(int nIndex);

	int GetChildNodeCount();
	int FindBoneIndex(const char *pBoneName);

	void AddChild(CBsBone *pChild);
	void RemoveChildPtr(CBsBone *pChild);
	void RemoveChildPtr(int nIndex);

	const char *GetBoneName() { return m_szBoneName; }
	const char *GetParentName() { return m_szParentName; }
	void SetParentTransMat( BSMATRIX *pMat ) { m_pParentTransMat = pMat; }

	void SetBoneRotation( D3DXVECTOR3 *pRotation );
	void SetCalcAniPosition( int nCalcAniPosition ) { s_nCalcAniPosition = nCalcAniPosition; }
	__forceinline void CalculateAnimation()
	{
		int i;
		BSMATRIX matAni;

		GetAnimationMatrix( matAni );
		BsMatrixMultiply( &m_matTransMatrix, &matAni,  m_pParentTransMat );			

		// - shinjich
#if _OPT_SHINJICH_BSBONE_H
		if( m_pBoneRotationflag )
		{
			static const __declspec(align(16)) float constfangles[4] = { ((1.0f / 180.0f) * D3DX_PI), ((1.0f / 180.0f) * D3DX_PI), ((1.0f / 180.0f) * D3DX_PI), 0.0f };
			BSVECTOR Pos;
			BSMATRIX Mat;
			XMVECTOR Angles;
			Pos = __lvx( &m_matTransMatrix._41, 0 );
			Angles = __lvx( &constfangles, 0 );
			Angles = __vmulfp( Angles, m_pBoneRotation );
			Mat = XMMatrixRotationRollPitchYawFromVector(Angles);
			BsMatrixMultiply( &m_matTransMatrix, &m_matTransMatrix, &Mat );
			*((BSVECTOR*) (&m_matTransMatrix._41)) = __lvx( &Pos, 0 );
		}
		BsMatrixMultiply( &m_matVertexTransMatrix, &m_matInverseWorld, &m_matTransMatrix );
		m_pBoneRotationflag = FALSE;
#else
		if( m_pBoneRotation )
		{
			BSVECTOR Pos;
			BSMATRIX Mat;
			memcpy( &Pos, &m_matTransMatrix._41, sizeof( BSVECTOR ) );
			BsMatrixRotationYawPitchRoll( &Mat, m_pBoneRotation->y / 180.0f * D3DX_PI, 
				m_pBoneRotation->x / 180.0f * D3DX_PI, m_pBoneRotation->z / 180.0f * D3DX_PI );
			BsMatrixMultiply( &m_matTransMatrix, &m_matTransMatrix, &Mat );
			memcpy( &m_matTransMatrix._41, &Pos, sizeof( BSVECTOR ) );
		}
		BsMatrixMultiply( &m_matVertexTransMatrix, &m_matInverseWorld, &m_matTransMatrix );
		m_pBoneRotation = NULL;
#endif
		for( i = 0; i < m_nChildCount; i++ )
		{
			m_ppChildList[ i ]->SetParentTransMat( &m_matTransMatrix );
		}
	}

	__forceinline void SetAnimationFrame( int nAni, float fFrame )
	{
		m_nAniIndex = nAni;
		m_fFrame = fFrame;
	}
	void SetAnimationFrameBone( int nAni, float fFrame );
	void BlendAnimationFrame(int nBlendAni, float fBlendFrame, float fBlendWeight);
	void ResetBlendBuffer() { m_nCurBlendCount = 0; }

	void ComputeVertexTransMatrix();

	__forceinline void GetPositionVector(BSVECTOR &Position, int nAni, float fFrame)
	{
		if( !m_ppAniInfo[ nAni ]->GetPositionVector( Position, fFrame ) )
		{
			Position = m_LocalAffine.Translation;
		}
	}
	__forceinline void GetRotationVector(BSQUATERNION &Rotation, int nAni, float fFrame)
	{
		if( !m_ppAniInfo[ nAni ]->GetRotationVector( Rotation, fFrame ) )
		{
			Rotation = m_LocalAffine.Rotation;
		}
	}
	__forceinline void GetScalingVector(BSVECTOR &Scale, BSQUATERNION &ScaleRot, int nAni, float fFrame)
	{
		if(m_ppAniInfo[ nAni ]->GetScalingKeyCount())
		{
			m_ppAniInfo[ nAni ]->GetScalingVector( Scale, ScaleRot, fFrame );
		}
		else
		{
			Scale = m_LocalAffine.Scale;
			ScaleRot = m_LocalAffine.StretchRotation;
		}
	}
	__forceinline int  GetAnimationMatrix( BSMATRIX &matSource )
	{
		int i;
		BSVECTOR Position, Position2;
		BSQUATERNION Rotation, Rotation2;
		BSVECTOR Scale;
		BSQUATERNION ScaleRot;

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
				BsVec3Lerp( &Position, &Position, &Position2, m_fBlendWeight[ i ] );
			}
		}

		GetRotationVector( Rotation, m_nAniIndex, m_fFrame );
		for( i = 0; i < m_nCurBlendCount; i++ )
		{
			GetRotationVector( Rotation2, m_nBlendIndex[ i ], m_fBlendFrame[ i ] );
			BsQuaternionSlerp( &Rotation, &Rotation, &Rotation2, m_fBlendWeight[ i ] );
			//		D3DXVec4Lerp( ( D3DXVECTOR4 * )&Rotation, ( D3DXVECTOR4 * )&Rotation, ( D3DXVECTOR4 * )&Rotation2, m_fBlendWeight[ i ] );
		}

		GetScalingVector( Scale, ScaleRot, m_nAniIndex, m_fFrame );

#ifdef _XBOX
		// - shinjich
#if _OPT_SHINJICH_BSBONE_H
		XMVECTOR vzero;
#if _DEBUG
		vzero.x = 0.0f;	// dummy for debug
#else
#pragma warning(disable:4700)
#endif
		vzero = __vxor( vzero, vzero );
		BsMatrixTransformation(&matSource, &vzero, &ScaleRot, &Scale, &vzero, &Rotation, &Position);
#else
		BsMatrixTransformation(&matSource, &XMVectorSet(0.f, 0.f, 0.f, 0.f), &ScaleRot, &Scale, &XMVectorSet(0.f, 0.f, 0.f, 0.f), &Rotation, &Position);
#endif
#else
		D3DXMatrixTransformation(&matSource, NULL, &ScaleRot, &Scale, NULL, &Rotation, &Position);
#endif
		ResetBlendBuffer();

		return 1;
	}

	BSMATRIX *GetTransMatrix() { return &m_matTransMatrix; }
	const BSMATRIX *GetInvWorldMatrix() const { return &m_matInverseWorld; }
	const BSMATRIX *GetWorldMatrix() const { return &m_matWorld; }
	const BSMATRIX* GetVertexTransMatrix() const { return &m_matVertexTransMatrix; }
	const BSMATRIX *GetLocalMatrix() const { return &m_matLocal; }
	const AFFINE_PARTS *GetLocalAffine() const { return &m_LocalAffine; }
	CBsAniSampledKeyFrame* GetAniInfo(int nAniIndex) const { return m_ppAniInfo[nAniIndex]; }

	int GetChildCount() const { return m_nChildCount; }
	CBsBone* GetChildPtr(int nIndex) const;

	int GetAniCount() { return m_nAniCount; }

	int GetPositionKeyCount(int nAni) { return m_ppAniInfo[nAni]->GetPositionKeyCount(); }
	int GetRotationKeyCount(int nAni) { return m_ppAniInfo[nAni]->GetRotationKeyCount(); }

	void AddAni(CBsBone *pRootBone, int nIndex);
	bool CheckAddAni(CBsBone *pRootBone);
	void DeleteAni(int nIndex);
	void GetAniDistance(GET_ANIDISTANCE_INFO *pInfo);

	//D3DXMATRIX GetWorldTransMatrix(SET_ANI_INFO *pAni, BLEND_ANI_INFO *pBlend, int nCalcAniPosition, D3DXMATRIX matChild = D3DXMATRIX(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1));
	void SetParentBone(CBsBone *pBone) { m_pParentBone = pBone; }
	void DeleteAnimationKey();

};

//------------------------------------------------------------------------------------------------