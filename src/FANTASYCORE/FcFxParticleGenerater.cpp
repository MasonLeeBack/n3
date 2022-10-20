#include "StdAfx.h"
#include ".\fcfxparticlegenerater.h"
#include "FcGameObject.h"


/*---------------------------------------------------------------------------------
--
-			CFcFxParticleGenerater::CFcFxParticleGenerater()
-					; 持失切.
-					  
-
---------------------------------------------------------------------------------*/
CFcFxParticleGenerater::CFcFxParticleGenerater(void)
{
	SetGameObj(NULL);
	SetParticleGroupIndex(-1);
	SetParticleIndex(-1);
	SetBTop(false);
	SetBMiddle(false);
	SetLoop(false);
#ifndef _LTCG
	SetFxRtti(FX_TYPE_PARTICLEGENERATER);
#endif //_LTCG
}





/*---------------------------------------------------------------------------------
--
-			CFcFxParticleGenerater::~CFcFxParticleGenerater()
-					; 持失切.
-					  
-
---------------------------------------------------------------------------------*/
CFcFxParticleGenerater::~CFcFxParticleGenerater(void)
{
	if( GetParticleIndex() != -1 )
		g_BsKernel.DeleteParticleObject( GetParticleIndex() );
	if ( GetParticleGroupIndex() != -1 )
		g_BsKernel.ReleaseParticleGroup( GetParticleGroupIndex() );
}





/*---------------------------------------------------------------------------------
--
-			CFcFxParticleGenerater::Process()
-					; 持失切.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxParticleGenerater::Process()
{
	if( m_state == END || m_state == STOP ) return;

	if( GetGameObj() )
	{
		CCrossVector Cross = *GetGameObj()->GetCrossVector();
		D3DXVECTOR3 vPos = Cross.m_PosVector;
		
		if( GetBTop() )
		{
			float fHe = GetGameObj()->GetUnitHeight();
			vPos.y += fHe + 50.0f;
			Cross.m_PosVector = vPos;
		}
		else if( GetBMiddle() )
		{
			float fHe = GetGameObj()->GetUnitHeight();
			vPos.y += fHe * 0.5f;
			Cross.m_PosVector = vPos;
		}
		Cross.UpdateVectors();
		

		if( GetParticleIndex() == -1 )
		{
			SetParticleIndex( g_BsKernel.CreateParticleObject( GetParticleGroupIndex(), GetLoop(), false, Cross, 1.0f, NULL, false  ) );
		}
		g_BsKernel.UpdateParticle( GetParticleIndex(), vPos );
	}
}





/*---------------------------------------------------------------------------------
-
-			CFcFxParticleGenerater::PreRender()
-					; 持失切.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxParticleGenerater::PreRender()
{
	CFcFXBase::PreRender();
}






/*---------------------------------------------------------------------------------
-
-			CFcFxParticleGenerater::Render()
-					; 持失切.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxParticleGenerater::Render(C3DDevice *pDevice)
{

}



/*---------------------------------------------------------------------------------
-
-			CFcFxParticleGenerater::ProcessMessage()
-					; 持失切.
-					  
-
---------------------------------------------------------------------------------*/
int CFcFxParticleGenerater::ProcessMessage(int nCode,DWORD dwParam1/*=0*/, DWORD dwParam2/*=0*/,DWORD dwParam3/*=0*/)
{
	if( CFcFXBase::ProcessMessage( nCode, dwParam1, dwParam2, dwParam3) )
		return 1;

	switch(nCode)
	{
	case FX_INIT_OBJECT:
		{
			if(dwParam1) SetGameObj((CFcGameObject*)dwParam1);

			int nParticleGroup = static_cast<int>(dwParam2);

			if ( nParticleGroup != -1 )
				g_BsKernel.AddParticleGroupRef( nParticleGroup );

			BsAssert( GetParticleGroupIndex() == -1 );
			SetParticleGroupIndex( nParticleGroup );
		}
		return 1;
	case FX_PLAY_OBJECT:
		{
			m_state = PLAY;
			if( dwParam1 == 1 )
				SetBTop(true);
			else if( dwParam1 == 2)
				SetBMiddle(true);

			if( dwParam2 )
				SetLoop(true);
			else
				SetLoop(false);
		}
		return 1;
	case FX_STOP_OBJECT:
		{
			m_state = STOP;
		}
		return 1;
	}

	return 0;
}