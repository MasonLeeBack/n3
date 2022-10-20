#pragma once
#include "FcFXBase.h"
#include "CDefinition.h"
class C3DDevice;




/*-----------------------------------------------------------------------------------
-																					-
-		SRainParam	:		속성구조체												-
-																					-
-										2005.11.30 v.1								-
-																					-
-------------------------------------------------------------------------------------*/
struct SRainParam
{
	int		_nCount;			// 정의된 틱당 나타날 빗방울 수
	int		_nTotalCount;		// 한화면에 나타날 빗방울의 총 갯수
	int		_nTick;				// 빗방울이 나타날 틱
	int		_nTotalTick;		// 빗방울이 나타날 전체 틱
	bool	_bLiner;			// 보간될 것인지를 나타낼 변수.
	SRainParam()
	{
		Reset();
	}
	void Reset()
	{
		memset( this, 0, sizeof(SRainParam));
	}
};
//*********************************************************************************************
//*********************************************************************************************





/*-----------------------------------------------------------------------------------
-																					-
-		SRainUnit	:			계산하기위한 빗방울 구조체							-
-																					-
-										2005.11.30 v.1								-
-																					-
-------------------------------------------------------------------------------------*/
struct SRainUnit				
{
	D3DXVECTOR3 _vPos;
	D3DXVECTOR3 _vDir;
	float		_fWidth;
	float		_fHeight;
	SRainUnit()
	{
		memset( this, 0, sizeof(SRainUnit));
	}
};
//*********************************************************************************************
//*********************************************************************************************





/*-----------------------------------------------------------------------------------
-																					-
-		SRainRenderUnit	:			그리기위한 빗방울 구조체						-
-											.										-
-										2005.11.30 v.1								-
-																					-
-------------------------------------------------------------------------------------*/
struct SRainRenderUnit			
{
	D3DXVECTOR3 _vPos;
	D3DXVECTOR2	_v2Tex;
	SRainRenderUnit()
	{
		memset( this, 0, sizeof(SRainRenderUnit));
	}
};
//*********************************************************************************************
//*********************************************************************************************












/*-----------------------------------------------------------------------------------
-
-				CFcFxRain						Choi Jae Young
-												2005. 11. 30 
-												v 0.01
-							In Blue-Side
-
-
-			note : 비내리는....
-				   
-
------------------------------------------------------------------------------------*/
class CFcFxRain  : public CFcFXBase
{
public:

	void							Process();
	void							PreRender();
	void							Render(C3DDevice *pDevice);
	int								ProcessMessage(int nCode,DWORD dwParam1=0, DWORD dwParam2=0,DWORD dwParam3=0);




private:

	void							Init( SRainParam* p_pRainInfo );
	void							TickCounter();
	void							Clear();
	void							MoveRain();								// 비가 땅에 떨어지면 delete 한다.
	void							MoveRain2();							// 비가 땅에 떨어지면 위치를 다시 올려준다.
	void							PosReset(SRainUnit* p_pRain);			// 비의 위치를 다시 올려 놓는다.
	void							MoveAllRain( D3DXVECTOR3 p_vPos );
	void							MAkeRain( bool p_bMake );
	void							MakeFace();
	void							MAkeRainGround();
	void							Update();

private:

	int								m_nVertexCount;
	int								m_nCountPerTick;
	int								m_nTick;
	int								m_nTexID;
	int								m_nMaterialIndex;	
	int								m_nVertexDeclIdx;		
	int								m_nFaceNum[2];
	int								m_nFxid;
	D3DXHANDLE						m_hWVP;				
	D3DXHANDLE						m_hTexture;			
	SRainParam						m_sRainInfo;
	std::list<SRainUnit*>			m_listRainUnit;
	SRainRenderUnit*				m_psRainRenderUnit[2];




public:
	CFcFxRain(void);
	virtual ~CFcFxRain(void);
};
//*********************************************************************************************
//*********************************************************************************************