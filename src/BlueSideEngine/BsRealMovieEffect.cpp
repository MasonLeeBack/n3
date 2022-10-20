#include "stdafx.h"
#include "BsRealMovieFile.h"
#include "BsKernel.h"

#include "BsRealMovieMsg.h"
#include "BsRealMoviePackage.h"

#include "BsRealMovieEffect.h"

#include "BsRealMovieObject.h"

#include "BsRealMovie.h"
#include "BsRealMovieConst.h"


char* g_lpszEffectStr[] = 
{
	"Not",
	"Fade In",
	"Fade Out",
	"Black",
	"Cmd FX",
	"End of Movie",
	"Custom Type",
	"Use Camera",
	"FX",
	"Show Object",
	"Use Bone",
	"Set Fog",
	"Set Troop",
	"Move Troop",
	"Use Light",
	"Use DOF",
	"Use Sound",
	"Sound Volume",
	"Drop Weapon",
	"AlphaBlend"
};


extern VOID	ChangeTickIntoTime( DWORD dwTick, float& fTime);

void	CRMEffect::FadeIn(float fAlpha)
{
	g_BsKernel.GetDevice()->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD );
	g_BsKernel.GetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	g_BsKernel.GetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

	D3DXCOLOR clr = D3DXCOLOR(0,0,0,   fAlpha);
	g_BsKernel.DrawUIBox_s(_Ui_Mode_Box, 0,0, 
		g_BsKernel.GetDevice()->GetBackBufferWidth(),
		g_BsKernel.GetDevice()->GetBackBufferHeight(), 
		0.f,
		clr);
}

void	CRMEffect::FadeOut(float fAlpha)
{
	g_BsKernel.GetDevice()->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD );
	g_BsKernel.GetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	g_BsKernel.GetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

	D3DXCOLOR clr = D3DXCOLOR(0,0,0, 1-fAlpha);
	g_BsKernel.DrawUIBox_s(_Ui_Mode_Box, 0,0, 
		g_BsKernel.GetDevice()->GetBackBufferWidth(),
		g_BsKernel.GetDevice()->GetBackBufferHeight(), 
		0.f,
		clr);
}

void	CRMEffect::FillBlack(void)
{
	g_BsKernel.GetDevice()->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD );
	g_BsKernel.GetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	g_BsKernel.GetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

	D3DXCOLOR clr = D3DXCOLOR(0,0,0, 1);
	g_BsKernel.DrawUIBox_s(_Ui_Mode_Box, 0,0, 
		g_BsKernel.GetDevice()->GetBackBufferWidth(),
		g_BsKernel.GetDevice()->GetBackBufferHeight(),
		0.f,
		clr);
}



void	CRMEffect::Save(BStream *pStream)
{
	int nLen;

	nLen = strlen( m_szName );
	pStream->Write( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );
	if( nLen )
		pStream->Write( m_szName , nLen , 0 );

	nLen = strlen( m_szFileName );
	pStream->Write( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );
	if( nLen )
		pStream->Write( m_szFileName , nLen , 0 );

	pStream->Write( &m_nType, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Write( &m_fTime, sizeof(float), ENDIAN_FOUR_BYTE );
	pStream->Write( &m_fUseTime, sizeof(float), ENDIAN_FOUR_BYTE );
	pStream->Write( &m_bDisable, sizeof(int), ENDIAN_FOUR_BYTE );

	pStream->Write( &m_nLinkedType, sizeof(int), ENDIAN_FOUR_BYTE );
	nLen = 3;
	pStream->Write( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );		// Param 갯수
	pStream->Write( m_dwParam, sizeof(DWORD)*nLen, ENDIAN_FOUR_BYTE );

	nLen = sizeof( SubData );
	pStream->Write( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );		// SubData 갯수


	pStream->Write( &SubData.m_dwTick, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Write( &SubData.m_Cross.m_XVector, sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
	pStream->Write( &SubData.m_Cross.m_YVector, sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
	pStream->Write( &SubData.m_Cross.m_ZVector, sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );
	pStream->Write( &SubData.m_Cross.m_PosVector, sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE );

	pStream->Write( &SubData.m_nYaw, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Write( &SubData.m_nRoll, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Write( &SubData.m_nPitch, sizeof(int), ENDIAN_FOUR_BYTE );

	pStream->Write( SubData.m_fParam, sizeof(float)*4, ENDIAN_FOUR_BYTE );

	pStream->Write( &SubData.m_nListIndex, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Write( &SubData.m_nUseFrame, sizeof(int), ENDIAN_FOUR_BYTE );

	// pStream->Write( &SubData, nLen, ENDIAN_FOUR_BYTE );
}

void	CRMEffect::Load(BStream *pStream)
{
	int nLen;

	/*
	pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );
	if( nLen )
	pStream->Read( m_szName , nLen , 0 );
	*/

	pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );

	// takogura: prefix bug 5829: using nLen uninitialized -> OK
	if( nLen )
		pStream->Read( m_szFileName, nLen , 0 );

	pStream->Read( &m_nType, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Read( &m_fTime, sizeof(float), ENDIAN_FOUR_BYTE );
	pStream->Read( &m_fUseTime, sizeof(float), ENDIAN_FOUR_BYTE );
	pStream->Read( &m_bDisable, sizeof(int), ENDIAN_FOUR_BYTE );

	pStream->Read( &m_nLinkedType, sizeof(int), ENDIAN_FOUR_BYTE );

	// nLen 개수만큼 Param 읽기.
	pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Read( m_dwParam, sizeof(DWORD)*nLen, ENDIAN_FOUR_BYTE );

	// nLen 개수만큼 SubData 읽기.
	pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );

	if( nLen )
	{
		if( nLen == 84 )
		{
			pStream->Read( &SubData.m_dwTick, sizeof(int), ENDIAN_FOUR_BYTE );
			pStream->Read( &SubData.m_Cross.m_XVector, sizeof(float)*3, ENDIAN_FOUR_BYTE );
			pStream->Read( &SubData.m_Cross.m_YVector, sizeof(float)*3, ENDIAN_FOUR_BYTE );
			pStream->Read( &SubData.m_Cross.m_ZVector, sizeof(float)*3, ENDIAN_FOUR_BYTE );
			pStream->Read( &SubData.m_Cross.m_PosVector, sizeof(float)*3, ENDIAN_FOUR_BYTE );

			pStream->Read( &SubData.m_nYaw, sizeof(int), ENDIAN_FOUR_BYTE );
			pStream->Read( &SubData.m_nRoll, sizeof(int), ENDIAN_FOUR_BYTE );
			pStream->Read( &SubData.m_nPitch, sizeof(int), ENDIAN_FOUR_BYTE );

			pStream->Read( SubData.m_fParam, sizeof(float)*4, ENDIAN_FOUR_BYTE );

			pStream->Read( &SubData.m_nListIndex, sizeof(int), ENDIAN_FOUR_BYTE );
		}
		else if( nLen == 104 ) // sizeof(SubData) )
		{
			pStream->Read( &SubData.m_dwTick, sizeof(int), ENDIAN_FOUR_BYTE );
			pStream->Read( &SubData.m_Cross.m_XVector, sizeof(float)*3, ENDIAN_FOUR_BYTE );
			pStream->Read( &SubData.m_Cross.m_YVector, sizeof(float)*3, ENDIAN_FOUR_BYTE );
			pStream->Read( &SubData.m_Cross.m_ZVector, sizeof(float)*3, ENDIAN_FOUR_BYTE );
			pStream->Read( &SubData.m_Cross.m_PosVector, sizeof(float)*3, ENDIAN_FOUR_BYTE );

			pStream->Read( &SubData.m_nYaw, sizeof(int), ENDIAN_FOUR_BYTE );
			pStream->Read( &SubData.m_nRoll, sizeof(int), ENDIAN_FOUR_BYTE );
			pStream->Read( &SubData.m_nPitch, sizeof(int), ENDIAN_FOUR_BYTE );

			pStream->Read( SubData.m_fParam, sizeof(float)*4, ENDIAN_FOUR_BYTE );

			pStream->Read( &SubData.m_nListIndex, sizeof(int), ENDIAN_FOUR_BYTE );
			pStream->Read( &SubData.m_nUseFrame, sizeof(int), ENDIAN_FOUR_BYTE );

            // pStream->Read( &SubData, nLen, ENDIAN_FOUR_BYTE );
		}
		else
		{
			DebugString( "** CRMEffect::Load Error!! Data size is not equal.\n");
			BsAssert( 0 && "Data size is not equal. :( ");
		}

		if( SubData.m_dwTick != -1 )
		{
			ChangeTickIntoTime( SubData.m_dwTick, m_fTime );
		}

		if( sizeof( SubData ) > nLen ) // 서브데이터 블럭 사이즈가 크면.
		{
			if( m_fUseTime > -1.f )
                SubData.m_nUseFrame = (int)(m_fUseTime * 40.f);
			else
				SubData.m_nUseFrame = -1;
		}
	}
}

void	CRMEffect::SaveEx(BStream *pStream)
{
	int nStrCnt = 2;
	int nLen;

	pStream->Write( &nStrCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	nLen = strlen( m_szLinkedObjectName );
	pStream->Write( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );
	if( nLen )
		pStream->Write( m_szLinkedObjectName, nLen, 0 );

	// 2번째. m_szStringParam
	nLen = strlen( m_szStringParam );
	pStream->Write( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );
	if( nLen )
		pStream->Write( m_szStringParam, nLen, 0 );

}

void	CRMEffect::LoadEx(BStream *pStream)
{
	int nCnt;
	int nLen;

	pStream->Read( &nCnt, sizeof(int), ENDIAN_FOUR_BYTE );

	for( int i = 0 ; i < nCnt ; ++i )
	{
		switch( i )
		{
		case 0 :
			{
				pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );
				if( nLen )
					pStream->Read( m_szLinkedObjectName, nLen, 0 );
			}
			break;
		case 1 :
			{
				pStream->Read( &nLen, sizeof(int), ENDIAN_FOUR_BYTE );
				if( nLen )
					pStream->Read( m_szStringParam, nLen, 0 );
			}
			break;
		}
	}
}

//int 	CRMEffect::EffectProcess(CRMEffect* pEffectObj,float fTime)
//{
//	// int nFadeOut = 0;
//	int nType = pEffectObj->m_nType;
//
//	switch( nType )
//	{
//#ifdef _USAGE_TOOL_
//	case eFadeIn :
//		{
//			float fRunTime = fTime;
//			fRunTime -= pEffectObj->m_fTime;
//
//			float fUseTime = pEffectObj->m_fUseTime;
//			if( fUseTime < 0.0001f )
//				fUseTime = 0.0001f;
//
//			float fAlpha = ( fUseTime - fRunTime ) / fUseTime;
//
//			CRMEffect::FadeIn( fAlpha );
//		}
//		return 1;
//	case eFadeOut :
//		{
//			float fRunTime = fTime;
//			fRunTime -= pEffectObj->m_fTime;
//
//			float fUseTime = pEffectObj->m_fUseTime;
//			if( fUseTime < 0.0001f )
//				fUseTime = 0.0001f;
//
//			float fAlpha = ( fUseTime - fRunTime ) / fUseTime;
//
//			CRMEffect::FadeOut( fAlpha );
//			if (1-fAlpha==1) 
//			{
//				// nFadeOut=1;
//				CRMEffect::FillBlack();
//			}
//		}
//		return 1;
//	case eBlack :
//		{
//			// nFadeOut = 1;
//			CRMEffect::FillBlack();
//		}
//		return 1;
//#endif
//	case eSpecialFX :
//	case eTerminate :
//	case eCustomType :
//		return 1;
//	}
//
//	return 0;
//}

int		CRMEffect::EffectProcess(CRMEffect* pEffectObj,int nTime)
{
	int nType = pEffectObj->m_nType;

	switch( nType )
	{
#ifdef _USAGE_TOOL_
	case eFadeIn :
		{
			float fRunTime = (float)nTime;
			fRunTime -= (float)pEffectObj->GetStartTick(); // m_fTime;

			float fUseTime = (float)pEffectObj->GetUseFrame(); // m_fUseTime;
			if( fUseTime < 0.025f )
				fUseTime = 0.025f;

			float fAlpha = ( fUseTime - fRunTime ) / fUseTime;

			CRMEffect::FadeIn( fAlpha );
		}
		return 1;
	case eFadeOut :
		{
			float fRunTime = (float)nTime;
			fRunTime -= pEffectObj->GetStartTick(); // m_fTime;

			float fUseTime = (float)pEffectObj->GetUseFrame();
			if( fUseTime < 0.025f )
				fUseTime = 0.025f;

			float fAlpha = ( fUseTime - fRunTime ) / fUseTime;

			CRMEffect::FadeOut( fAlpha );
			if (1-fAlpha==1) 
			{
				// nFadeOut=1;
				CRMEffect::FillBlack();
			}
		}
		return 1;
	case eBlack :
		{
			// nFadeOut = 1;
			CRMEffect::FillBlack();
		}
		return 1;
#endif
	case eSpecialFX :
	case eTerminate :
	case eCustomType :
		return 1;
	case eDropWeapon :
		{
			pEffectObj->DropWeapon();
		}
		return 1;
	}

	return 0;
}

void	CRMEffect::SetPosition(D3DXVECTOR3& rPos)	
{	
	SubData.m_Cross.m_PosVector = rPos;	
	SetHPos( rPos, (m_nType == eSpecialFX)?0x80ff9999:0x808080FF );
}

#ifdef _USAGE_TOOL_
void	CRMEffect::ResetEffectHBoardPos(void)
{
	SetHPos( SubData.m_Cross.m_PosVector, 
		(m_nType == eSpecialFX)?0x80ff9999:0x808080FF );
}
#endif

void	CRMEffect::SetXPos(float fX)
{	
	SubData.m_Cross.m_PosVector.x = fX;		
	SetHPos( SubData.m_Cross.m_PosVector, (m_nType == eSpecialFX)?0x80ff9999:0x808080FF );
}

void	CRMEffect::SetYPos(float fY)
{	
	SubData.m_Cross.m_PosVector.y = fY;	
	SetHPos( SubData.m_Cross.m_PosVector, (m_nType == eSpecialFX)?0x80ff9999:0x808080FF );
}

void	CRMEffect::SetZPos(float fZ)				
{	
	SubData.m_Cross.m_PosVector.z = fZ;	
	SetHPos( SubData.m_Cross.m_PosVector, (m_nType == eSpecialFX)?0x80ff9999:0x808080FF );
}

VOID	CRMEffect::CrossRotateYaw(int nAngle)
{
	SubData.m_nYaw += nAngle;
	if( SubData.m_nYaw > 1024 )
		SubData.m_nYaw -= 1024;

	SubData.m_Cross.RotateYawByWorld( nAngle );
}

VOID	CRMEffect::CrossRotatePitch(int nAngle)
{
	SubData.m_nPitch += nAngle;
	if( SubData.m_nPitch > 1024 )
		SubData.m_nPitch -= 1024;

	SubData.m_Cross.RotatePitch( nAngle );
}

VOID	CRMEffect::CrossRotateRoll(int nAngle)
{
	SubData.m_nRoll += nAngle;
	if( SubData.m_nRoll > 1024 )
		SubData.m_nRoll -= 1024;

	SubData.m_Cross.RotateRoll( nAngle );
}

VOID	CRMEffect::CrossAbsoluteYaw(int nAngle)
{
	D3DXVECTOR3 vecPos = SubData.m_Cross.m_PosVector;
	SubData.m_nYaw = nAngle;
	SubData.m_Cross.Reset();
	SubData.m_Cross.m_PosVector = vecPos;

	SubData.m_Cross.RotateYawByWorld( SubData.m_nYaw );
	SubData.m_Cross.RotatePitch( SubData.m_nPitch );
	SubData.m_Cross.RotateRoll( SubData.m_nRoll );
}

VOID	CRMEffect::CrossAbsoluteRoll(int nAngle)
{
	D3DXVECTOR3 vecPos = SubData.m_Cross.m_PosVector;
	SubData.m_nRoll = nAngle;
	SubData.m_Cross.Reset();
	SubData.m_Cross.m_PosVector = vecPos;

	SubData.m_Cross.RotateYawByWorld( SubData.m_nYaw );
	SubData.m_Cross.RotatePitch( SubData.m_nPitch );
	SubData.m_Cross.RotateRoll( SubData.m_nRoll );
}

VOID	CRMEffect::CrossAbsolutePitch(int nAngle)
{
	D3DXVECTOR3 vecPos = SubData.m_Cross.m_PosVector;
	SubData.m_nPitch = nAngle;
	SubData.m_Cross.Reset();
	SubData.m_Cross.m_PosVector = vecPos;

	SubData.m_Cross.RotateYawByWorld( SubData.m_nYaw );
	SubData.m_Cross.RotatePitch( SubData.m_nPitch );
	SubData.m_Cross.RotateRoll( SubData.m_nRoll );
}

// Particle : 0x80ff9999
// FX       : 0x808080FF
void	CRMEffect::SetHPos( D3DXVECTOR3 &pos, DWORD dwColor )
{	
	// m_vecHPos = pos;
	D3DXVECTOR3 Pos0( pos.x - 50.f , pos.y + 5.f , pos.z - 50.f );
	D3DXVECTOR3 Pos1( pos.x - 50.f , pos.y + 5.f , pos.z + 50.f );
	D3DXVECTOR3 Pos2( pos.x + 50.f , pos.y + 5.f , pos.z + 50.f );
	D3DXVECTOR3 Pos3( pos.x + 50.f , pos.y + 5.f , pos.z - 50.f );

	//m_vecHBoard
	m_vecHBoard[0].Vertex = Pos0;
	m_vecHBoard[0].dwColor = dwColor;

	m_vecHBoard[1].Vertex = Pos1;
	m_vecHBoard[1].dwColor = dwColor;

	m_vecHBoard[2].Vertex = Pos2;
	m_vecHBoard[2].dwColor = dwColor;

	m_vecHBoard[3].Vertex = Pos0;
	m_vecHBoard[3].dwColor = dwColor;

	m_vecHBoard[4].Vertex = Pos2;
	m_vecHBoard[4].dwColor = dwColor;

	m_vecHBoard[5].Vertex = Pos3;
	m_vecHBoard[5].dwColor = dwColor;
};

VOID	CRMEffect::MakeUseBoneData(VOID)
{
	DWORD dwUseTime = (int)(m_fUseTime * g_fRMFps); // 총 사용시간.
	m_nKeepUpTick = dwUseTime - m_dwParam[0] - m_dwParam[1];	// 유지시간 - 스타트 타임 - 엔드 타임.
}

VOID	CRMEffect::MakeUseBoneDataForProcess(VOID)
{
	DWORD dwUseTime = (int)(m_fUseTime * g_fRMFps); // 총 사용시간.
	m_nKeepUpTickForProcess = dwUseTime - m_dwParam[0] - m_dwParam[1];	// 유지시간 - 스타트 타임 - 엔드 타임.
}



VOID	CRMEffect::MakeAverageDegree(VOID)
{
	if( m_dwParam[0] == 0 )
	{
		m_fStartAveDegree[0] = 0.f;
		m_fStartAveDegree[1] = 0.f;
		m_fStartAveDegree[2] = 0.f;
	}
	else
	{
		m_fStartAveDegree[0] = (float)SubData.m_nYaw / (float)m_dwParam[0];
		m_fStartAveDegree[1] = (float)SubData.m_nRoll / (float)m_dwParam[0];
		m_fStartAveDegree[2] = (float)SubData.m_nPitch / (float)m_dwParam[0];

	}

	if( m_dwParam[1] == 0 )
	{
		m_fEndAveDegree[0] = 0.f;
		m_fEndAveDegree[1] = 0.f;
		m_fEndAveDegree[2] = 0.f;
	}
	else
	{
		m_fEndAveDegree[0] = (float)SubData.m_nYaw / (float)m_dwParam[1];
		m_fEndAveDegree[1] = (float)SubData.m_nRoll / (float)m_dwParam[1];
		m_fEndAveDegree[2] = (float)SubData.m_nPitch / (float)m_dwParam[1];
	}
}

VOID	CRMEffect::MakeAverageDegreeForProcess(VOID)
{
	if( m_dwParam[0] == 0 )
	{
		m_fStartAveDegreeForProcess[0] = 0.f;
		m_fStartAveDegreeForProcess[1] = 0.f;
		m_fStartAveDegreeForProcess[2] = 0.f;
	}
	else
	{
		m_fStartAveDegreeForProcess[0] = (float)SubData.m_nYaw / (float)m_dwParam[0];
		m_fStartAveDegreeForProcess[1] = (float)SubData.m_nRoll / (float)m_dwParam[0];
		m_fStartAveDegreeForProcess[2] = (float)SubData.m_nPitch / (float)m_dwParam[0];

	}

	if( m_dwParam[1] == 0 )
	{
		m_fEndAveDegreeForProcess[0] = 0.f;
		m_fEndAveDegreeForProcess[1] = 0.f;
		m_fEndAveDegreeForProcess[2] = 0.f;
	}
	else
	{
		m_fEndAveDegreeForProcess[0] = (float)SubData.m_nYaw / (float)m_dwParam[1];
		m_fEndAveDegreeForProcess[1] = (float)SubData.m_nRoll / (float)m_dwParam[1];
		m_fEndAveDegreeForProcess[2] = (float)SubData.m_nPitch / (float)m_dwParam[1];
	}
}


extern char g_szDebugStr1[256];
extern char g_szDebugStr2[256];
extern void	OuputUserDebugString1(char *szTemp);
extern void	OuputUserDebugString2(char *szTemp);
// 일단 되게 하라. Render 용
VOID	CRMEffect::ProcessBoneAniFrame( int nCurTick )
{
	int nEndTime;
	// int nCurTick = (int)(fCurTime * g_fRMFps);

	CBsRealMovie* pRM = g_BsKernel.GetRealMovie();
	CRMObject* pRMObj = pRM->GetObject( m_szLinkedObjectName );

	if( !pRMObj || pRMObj->m_nObjectIndex == -1 )
		return;

	CRMBone *pBone = pRMObj->FindBone( m_szFileName );

	if( !pBone )
		return;

	int nTime = nCurTick - (int)SubData.m_dwTick;
	// static D3DXVECTOR3	vecAngle;

#ifdef _USAGE_TOOL_
	if( nTime == 0 )
	{
		MakeUseBoneData();
		MakeAverageDegree();
	}
#endif

	// if( nTime 
	D3DXVECTOR3* pvecAngle;

	// 시작 애니 구현 시간 보다 작다면. 본 애니
	pvecAngle = &m_vecAngleForRender;

	if( nTime <= (int)m_dwParam[0] )
	{
		pvecAngle->x = ( (float)( nTime * m_fStartAveDegree[0] ) );
		pvecAngle->y = ( (float)( nTime * m_fStartAveDegree[1] ) );
		pvecAngle->z = ( (float)( nTime * m_fStartAveDegree[2] ) );
	}
	// 시작 애니 구현 시간 + 유지 시간 보다 크다면, 원상복귀 애니
	else if( nTime > (int)m_dwParam[0] + m_nKeepUpTick )	
	{
		// 거꾸로 된 시간 구하기.
		nEndTime = m_dwParam[1] - ( nTime - ( m_dwParam[0] + m_nKeepUpTick ) );

		pvecAngle->x = ( (float)( nEndTime * m_fEndAveDegree[0] ) );
		pvecAngle->y = ( (float)( nEndTime * m_fEndAveDegree[1] ) );
		pvecAngle->z = ( (float)( nEndTime * m_fEndAveDegree[2] ) );
	}
	else
	{
		pvecAngle->x = ( (float)SubData.m_nYaw );
		pvecAngle->y = ( (float)SubData.m_nRoll );
		pvecAngle->z = ( (float)SubData.m_nPitch );
	}

	// assert( D3DXVec3Length( &(*pvecAngle - m_vecAngleForProcess)) < 0.0001f );

	g_BsKernel.SendMessage( pRMObj->m_nObjectIndex, BS_SET_CLEAR_BONE_ROTATION );

	for( int i = 0 ; i < CRMBone::eBoneMax ; ++i )
	{
		if( pBone->m_bUseBone[i] && pBone->m_szBone[i][0] != NULL )
		{
			g_BsKernel.SendMessage( pRMObj->m_nObjectIndex, 
				BS_SET_BONE_ROTATION, (DWORD)(char*)pBone->m_szBone[i], (DWORD)pvecAngle );
		}
	}
}

// 편법..
VOID	CRMEffect::ProcessBoneAniFrameForProcess( int nCurTick )
{
	int nEndTime;
	// int nCurTick = (int)(fCurTime * g_fRMFps);

	CBsRealMovie* pRM = g_BsKernel.GetRealMovie();
	CRMObject* pRMObj = pRM->GetObject( m_szLinkedObjectName );

	if( !pRMObj || pRMObj->m_nObjectIndex == -1 )
		return;

	CRMBone *pBone = pRMObj->FindBone( m_szFileName );

	if( !pBone )
		return;

	int nTime = nCurTick - (int)SubData.m_dwTick;
	// static D3DXVECTOR3	vecAngle;

#ifdef _USAGE_TOOL_
	if( nTime == 0 )
	{
		MakeUseBoneDataForProcess();
		MakeAverageDegreeForProcess();
	}
#endif

	// if( nTime 
	D3DXVECTOR3* pvecAngle;

	// 시작 애니 구현 시간 보다 작다면. 본 애니
	pvecAngle = &m_vecAngleForProcess;

	if( nTime <= (int)m_dwParam[0] )
	{
		pvecAngle->x = ( (float)( nTime * m_fStartAveDegreeForProcess[0] ) );
		pvecAngle->y = ( (float)( nTime * m_fStartAveDegreeForProcess[1] ) );
		pvecAngle->z = ( (float)( nTime * m_fStartAveDegreeForProcess[2] ) );
	}
	// 시작 애니 구현 시간 + 유지 시간 보다 크다면, 원상복귀 애니
	else if( nTime > (int)m_dwParam[0] + m_nKeepUpTickForProcess )	
	{
		// 거꾸로 된 시간 구하기.
		nEndTime = m_dwParam[1] - ( nTime - ( m_dwParam[0] + m_nKeepUpTickForProcess ) );

		pvecAngle->x = ( (float)( nEndTime * m_fEndAveDegreeForProcess[0] ) );
		pvecAngle->y = ( (float)( nEndTime * m_fEndAveDegreeForProcess[1] ) );
		pvecAngle->z = ( (float)( nEndTime * m_fEndAveDegreeForProcess[2] ) );
	}
	else
	{
		pvecAngle->x = ( (float)SubData.m_nYaw );
		pvecAngle->y = ( (float)SubData.m_nRoll );
		pvecAngle->z = ( (float)SubData.m_nPitch );
	}

	g_BsKernel.SendMessage( pRMObj->m_nObjectIndex, BS_SET_CLEAR_BONE_ROTATION );

	for( int i = 0 ; i < CRMBone::eBoneMax ; ++i )
	{
		if( pBone->m_bUseBone[i] && pBone->m_szBone[i][0] != NULL )
		{
			g_BsKernel.SendMessage( pRMObj->m_nObjectIndex, 
				BS_SET_BONE_ROTATION_PROCESS, (DWORD)(char*)pBone->m_szBone[i], (DWORD)pvecAngle );
		}
	}
}

#ifdef _USAGE_TOOL_

extern VOID	ChangeNewTickIntoTime(DWORD& dwTick,float& fTime);

void	CRMEffect::ChangeFPS(void)
{
	ChangeNewTickIntoTime( SubData.m_dwTick, m_fTime );
}
#endif

VOID	CRMEffect::DropWeapon(VOID)
{
	CBsRealMovie* pRM = g_BsKernel.GetRealMovie();
	CRMObject* pRMObj = pRM->GetObject( GetLinkedObjName() );	// Object Name
	CRMObject* pRMWeaponObj = pRM->GetObject( GetFileName() );	// Link weapon

	// 무기 떨구기 위한 시작 좌표 얻기.
	pRMWeaponObj->DropWeapon();

	// UNLINK를 통한 무기 떨구기..
	g_BsKernel.SendMessage( pRMObj->m_nObjectIndex, BS_UNLINKOBJECT,pRMWeaponObj->m_nObjectIndex );

	//AABB *pBox;
	//D3DXVECTOR3 Size, LinVel, AngVel;
	//pBox = ( AABB * )g_BsKernel.SendMessage( pRMWeaponObj->m_nObjectIndex, BS_GET_BOUNDING_BOX );
	//Size = *(D3DXVECTOR3*)&(( pBox->Vmax - pBox->Vmin ) * 0.5);
}