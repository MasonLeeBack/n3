#include "StdAfx.h"
#include ".\fcfxburning.h"
#include "FcGameObject.h"



/*---------------------------------------------------------------------------------
--
-			CFcFxBurning::CFcFxBurning()
-					; ������.
-					  
-
---------------------------------------------------------------------------------*/
CFcFxBurning::CFcFxBurning(void)
{
	SetGameObj(NULL);
	SetFxObjIndex(-1);
	SetFxTemplateIndex(-1);
	SetBurn(false);
#ifndef _LTCG
	SetFxRtti(FX_TYPE_BURNING);
#endif //_LTCG
}





/*---------------------------------------------------------------------------------
--
-			CFcFxBurning::~CFcFxBurning()
-					; ������.
-					  
-
---------------------------------------------------------------------------------*/
CFcFxBurning::~CFcFxBurning(void)
{
	if( GetFxObjIndex() != -1 ) g_BsKernel.DeleteObject( GetFxObjIndex() );
	if( GetFxTemplateIndex() != -1 ) g_BsKernel.ReleaseFXTemplate( GetFxTemplateIndex() );
}


/*---------------------------------------------------------------------------------
--
-			CFcFxBurning::Process()
-					; ������.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxBurning::Process()
{
	if( m_state == ENDING )
	{
		m_state = END;
		m_DeviceDataState=FXDS_RELEASEREADY;
	}
	if( m_state == END || m_state == STOP ) return;

	if( GetGameObj() )
	{
		CCrossVector Cross = *GetGameObj()->GetCrossVector();
		Cross.UpdateVectors();
		if( GetFxObjIndex() == -1 )
		{
			SetFxObjIndex( g_BsKernel.CreateFXObject(GetFxTemplateIndex()) );
			g_BsKernel.SetFXObjectState( GetFxObjIndex(), CBsFXObject::PLAY );
			SetBurn(true);
		}
//		g_BsKernel.UpdateObject( GetFxObjIndex(), Cross );

	}
	
// [PREFIX:beginmodify] 2006/2/25 junyash PS#5814 dereferencing NULL pointer
	if( GetGameObj() )
	{
		if( GetBurn() )
		{
			GetGameObj()->SaveMaterialAmbient();
			// m_cAddColor �� ������ ���� ������Ų��.
			// �����ʿ� ��û����. õõ�� ambient ���� ������ �����ιٲٴ� �Լ��� ������� ������ ���ƿ��� �Լ�.
			GetGameObj()->SetMaterialAmbient(D3DXVECTOR4(0,0,0,1));
			
		}
		else
		{
			// m_cAddColor �� 0,0,0 �� �ƴϸ� ������ 0,0,0 ���� �����.
			GetGameObj()->RestoreMaterialAmbient();
		}
	}
// [PREFIX:endmodify] junyash
	
}

void CFcFxBurning::Update()
{
	if( m_state == END || m_state == STOP ) return;

	if( GetGameObj() )
	{
		CCrossVector Cross = *GetGameObj()->GetCrossVector();
		Cross.UpdateVectors();
		if( GetFxObjIndex() == -1 )
		{
			g_BsKernel.UpdateObject( GetFxObjIndex(), Cross );
		}
	}

}




/*---------------------------------------------------------------------------------
-
-			CFcFxBurning::PreRender()
-					; ������.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxBurning::PreRender()
{
	CFcFXBase::PreRender();
}






/*---------------------------------------------------------------------------------
-
-			CFcFxBurning::Render()
-					; .
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxBurning::Render(C3DDevice *pDevice)
{
	
}



/*---------------------------------------------------------------------------------
-
-			CFcFxBurning::ProcessMessage()
-					; .
-					  
-
---------------------------------------------------------------------------------*/
int CFcFxBurning::ProcessMessage(int nCode,DWORD dwParam1/*=0*/, DWORD dwParam2/*=0*/,DWORD dwParam3/*=0*/)
{
	if( CFcFXBase::ProcessMessage( nCode, dwParam1, dwParam2, dwParam3) )
		return 1;

	switch(nCode)
	{
	case FX_INIT_OBJECT:
		{
			SetGameObj((CFcGameObject*)dwParam1);

			int nFxTemplate = static_cast<int>(dwParam2);

			if ( nFxTemplate != -1 )
				g_BsKernel.AddFXTemplateRef( nFxTemplate );

			BsAssert( GetFxTemplateIndex() == -1 );
			SetFxTemplateIndex(nFxTemplate);
		}
		return 1;
	case FX_PLAY_OBJECT:
		{
			m_state = PLAY;
		}
		return 1;
	case FX_RESERVE_OBJECT:
		{
			SetBurn(false);
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