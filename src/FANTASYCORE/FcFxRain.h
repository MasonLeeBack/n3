#pragma once
#include "FcFXBase.h"
#include "CDefinition.h"
class C3DDevice;




/*-----------------------------------------------------------------------------------
-																					-
-		SRainParam	:		�Ӽ�����ü												-
-																					-
-										2005.11.30 v.1								-
-																					-
-------------------------------------------------------------------------------------*/
struct SRainParam
{
	int		_nCount;			// ���ǵ� ƽ�� ��Ÿ�� ����� ��
	int		_nTotalCount;		// ��ȭ�鿡 ��Ÿ�� ������� �� ����
	int		_nTick;				// ������� ��Ÿ�� ƽ
	int		_nTotalTick;		// ������� ��Ÿ�� ��ü ƽ
	bool	_bLiner;			// ������ �������� ��Ÿ�� ����.
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
-		SRainUnit	:			����ϱ����� ����� ����ü							-
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
-		SRainRenderUnit	:			�׸������� ����� ����ü						-
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
-			note : �񳻸���....
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
	void							MoveRain();								// �� ���� �������� delete �Ѵ�.
	void							MoveRain2();							// �� ���� �������� ��ġ�� �ٽ� �÷��ش�.
	void							PosReset(SRainUnit* p_pRain);			// ���� ��ġ�� �ٽ� �÷� ���´�.
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