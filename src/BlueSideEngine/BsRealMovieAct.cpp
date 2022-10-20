#include "stdafx.h"
#include "BsRealMovieFile.h"
#include "BsKernel.h"

#include "BsRealMovieMsg.h"
#include "BsRealMovieAct.h"

#include "BsRealMovieObject.h"
#include "BsRealMovie.h"

extern VOID	ChangeTickIntoTime( DWORD dwTick, float& fTime);

void	CRMAct::Clear(void)
{
	SubData.m_fTime = -1.f;
	SubData.m_dwTick = -1;
	SubData.m_nAniIndex = -1;
	memset( m_szName , 0, eFileNameLength );
}

void	CRMAct::SetStartTime(DWORD dwTick)
{
	// SubData.m_fTime = fTime;		
	SubData.m_dwTick = dwTick;
	SubData.m_fTime = (float)dwTick / g_fRMFps;

}

// 1) NameLen , Name
// 2) AniIndex,
// 3) Time
void	CRMAct::Save(BStream *pStream)
{
	int nLen = strlen( m_szName );
	pStream->Write( &nLen , sizeof(int), ENDIAN_FOUR_BYTE );
	if( nLen )
		pStream->Write( m_szName, nLen , 0 );

	//pStream->Write( &m_nAniIndex, sizeof(int) , ENDIAN_FOUR_BYTE );
	//pStream->Write( &m_fTime, sizeof(float), ENDIAN_FOUR_BYTE );

	nLen = sizeof( SubData ); //eActPackSize;		// String 을 뺀 Bone Class 크기 저장.
	pStream->Write( &nLen , sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Write( &SubData, nLen, ENDIAN_FOUR_BYTE );
}

void	CRMAct::Load(BStream *pStream)
{
	/*
	memset( m_szName , 0, eFileNameLength );
	int nLen;
	pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );
	if( nLen )
	pStream->Read( m_szName, nLen , 0 );
	*/

	//pStream->Read( &m_nAniIndex, sizeof(int), ENDIAN_FOUR_BYTE );
	//pStream->Read( &m_fTime, sizeof(float), ENDIAN_FOUR_BYTE );

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
			SubData.m_dwTick = (DWORD)( SubData.m_fTime * eFps40 );
		}
	}
}

int		CRMAct::GetAniLength(void)
{
	return m_pParent->GetAniLength( SubData.m_nAniIndex );
}

int		CRMAct::GetFrameSize(void)
{
	int nFrmSize = GetAniLength();
	int nEndTrim;

	if( GetEndFrame() != 0 )
		nEndTrim = nFrmSize - GetEndFrame();
	else
		nEndTrim = 0;

	nFrmSize -= GetStartFrame();
	nFrmSize -= nEndTrim;

	return nFrmSize;
}

BOOL	CRMAct::RecalcAniFrameSize(void)
{
	int nAniLen = GetAniLength();

	if( SubData.m_nEndFrame > nAniLen )
	{
		SubData.m_nEndFrame = nAniLen - 1;
		return TRUE;
	}

	return FALSE;
}

#ifdef _USAGE_TOOL_

extern VOID	ChangeNewTickIntoTime(DWORD& dwTick,float& fTime);

void	CRMAct::ChangeFPS(void)
{
	ChangeNewTickIntoTime( SubData.m_dwTick, SubData.m_fTime );
}
#endif