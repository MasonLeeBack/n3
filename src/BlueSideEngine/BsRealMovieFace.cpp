#include "stdafx.h"
#include "BsRealMovieFile.h"
#include "BsKernel.h"

#include "BsRealMovieMsg.h"
#include "BsRealMovieFace.h"

#include "BsRealMovieObject.h"
#include "BsRealMovie.h"

extern VOID	ChangeTickIntoTime( DWORD dwTick, float& fTime);

void	CRMFace::SetStartTime(DWORD dwTick)
{
	// SubData.m_fTime = fTime;		
	SubData.m_dwTick = dwTick;
	SubData.m_fTime = (float)dwTick / g_fRMFps;
}

void	CRMFace::Save(BStream *pStream)
{
	int nLen = strlen( m_szName );
	pStream->Write( &nLen , sizeof(int), ENDIAN_FOUR_BYTE );
	if( nLen )
		pStream->Write( m_szName, nLen , 0 );

	nLen = sizeof( SubData ); //eFacePackSize;		// String 을 뺀 Bone Class 크기 저장.
	pStream->Write( &nLen , sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Write( &SubData , nLen, ENDIAN_FOUR_BYTE );
}

void	CRMFace::Load(BStream *pStream)
{
	int nLen;
	pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );

	if( nLen )
	{
		pStream->Read( &SubData, nLen, ENDIAN_FOUR_BYTE );

		if( SubData.m_dwTick != -1 )
		{
			ChangeTickIntoTime( SubData.m_dwTick , SubData.m_fTime );
		}
		else if( SubData.m_fTime > -1.f )
		{
			SubData.m_dwTick = (DWORD)(SubData.m_fTime * g_fRMFps );
		}

		if( SubData.m_nUseFrame == -1 && SubData.m_fUseTime > -1.f )
		{
			SubData.m_nUseFrame = (int)(SubData.m_fUseTime * g_fRMFps );
		}
	}
}

void	CRMFace::SetComplement(float fTime)	
{	
	SubData.m_fComplement = fTime;	
	RecalcComplement();
}

void	CRMFace::RecalcComplement(void)
{
	int nAniLen = m_pParent->GetAniLength( SubData.m_nAniIndex );

	if( nAniLen != -1 )
	{
		SubData.m_fAniLen = (float)( nAniLen - 1 );
		SubData.m_fAniInc = SubData.m_fAniLen / SubData.m_fComplement;

		if( SubData.m_fAniInc > 1.f )
			SubData.m_fAniInc = 1.f;
	}
}

#ifdef _USAGE_TOOL_

extern VOID	ChangeNewTickIntoTime(DWORD& dwTick,float& fTime);
extern VOID	ChangeNewTickIntoTime(int& nTick,float& fTime);

void	CRMFace::ChangeFPS(void)
{
	ChangeNewTickIntoTime( SubData.m_dwTick, SubData.m_fTime );
	ChangeNewTickIntoTime( SubData.m_nUseFrame, SubData.m_fUseTime );
}
#endif