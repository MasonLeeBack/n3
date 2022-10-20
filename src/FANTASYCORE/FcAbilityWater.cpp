#include "stdafx.h"
#include "FcAbilityWater.h"
#include "FcFxManager.h"
#include "FcFxBase.h"
#include "FcFxTidalWave.h"
#include "FcWorld.h"
#include "FcHeroObject.h"
#include "FcGameObject.h"
#include "FcProp.h"
#include "FcUtil.h"
#include "FcGlobal.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

#define TW_FORCE_VAL			10.f
class CFcGameObject;
typedef CSmartPtr<CFcGameObject> GameObjHandle;

// Ʃ���� ���Ѹ��� ���� ���� ������ ũ��
const int g_nPower[9] = { 90, 90, 90, 90, 90, 90, 90, 90, 90 };

// Ʃ���� ���Ѹ��� ���ݽ� ���� ���ĳ��� ��.
int g_nPushY = 5;
int g_nPushX = 10;
/*---------------------------------------------------------------------------------
-
-			CFcAbilityWater::CFcAbilityWater()
-					; ������.
-
---------------------------------------------------------------------------------*/
CFcAbilityWater::CFcAbilityWater( GameObjHandle hParent ) : CFcAbilityBase( hParent )
{
	m_nTidalWaveFxID = -1;
}



/*---------------------------------------------------------------------------------
-
-			CFcAbilityWater::~CFcAbilityWater()
-					; �Ҹ���
-
---------------------------------------------------------------------------------*/
CFcAbilityWater::~CFcAbilityWater()
{
}


/*---------------------------------------------------------------------------------
-
-			CFcAbilityWater::Finalize()
-					; �� ��������ũ �ε����� �ʱ�ȭ �Ѵ�.
-
---------------------------------------------------------------------------------*/
void CFcAbilityWater::Finalize()
{
	/*g_pFcFXManager->SendMessage(m_nTidalWaveFxID,FX_DELETE_OBJECT);
	m_nTidalWaveFxID = -1;*/
	SAFE_DELETE_FX(m_nTidalWaveFxID, FX_TYPE_TIDALWAVE);
	g_FcWorld.FinishTrueOrbDamagedTroops();
}


/*---------------------------------------------------------------------------------
-
-			CFcAbilityWater::Initialize()
-					; �� ��������ũ�� �����ϰ� �� ��������ũ�� �ʱ�ȭ �Ѵ�.
-
---------------------------------------------------------------------------------*/
void CFcAbilityWater::Initialize(DWORD dwParam1 /*= 0*/, DWORD dwParam2 /*= 0*/)
{
	if(m_nTidalWaveFxID == -1)
	{
		m_nTidalWaveFxID = g_pFcFXManager->Create(FX_TYPE_TIDALWAVE);
	}
	g_pFcFXManager->SendMessage(m_nTidalWaveFxID,FX_INIT_OBJECT, false, (DWORD)&m_hParent);
	m_bNotFullOrb = false;

	m_DamageObjManager.SetGameHandle(m_hParent);
	SetRtti(-1);

	g_FcWorld.InitTrueOrbDamagedTroops();
}

void CFcAbilityWater::Initialize(bool p_b)
{
	if(m_nTidalWaveFxID == -1)
	{
		m_nTidalWaveFxID = g_pFcFXManager->Create(FX_TYPE_TIDALWAVE);
	}
	g_pFcFXManager->SendMessage(m_nTidalWaveFxID,FX_INIT_OBJECT, true, (DWORD)&m_hParent);
	m_bNotFullOrb = true;
	m_DamageObjManager.SetGameHandle(m_hParent);
	SetRtti(100);
}

/*---------------------------------------------------------------------------------
-
-			CFcAbilityWater::Process()
-					; �� ��������ũ�� �� ������ ��ġ�� ���� �����ͼ� 
-					  ��������� �Լ��� �ѱ��.
-
---------------------------------------------------------------------------------*/
void CFcAbilityWater::Process()
{
	int nCount;

	D3DXVECTOR3 Pos;
	D3DXVECTOR3 Vel;

	if( IsPlayRealMovie() || IsPlayEvent() ) 
	{
		m_bFinished = true;
		Finalize();
	}

	if( ( GetProcessTick() % 10 ) == 0 )
	{
		g_pFcFXManager->SendMessage( m_nTidalWaveFxID, FX_WATER_GET_VERTEX_COUNT, (DWORD)&nCount );
		for( int i=0; i<nCount; i++ )
		{
			if(i % 100 != 0){
				continue;
			}
			g_pFcFXManager->SendMessage( m_nTidalWaveFxID, FX_WATER_GET_VERTEX, (DWORD)i, (DWORD)&Pos );			
			g_pFcFXManager->SendMessage( m_nTidalWaveFxID, FX_WATER_GET_VELOCITY, (DWORD)i, (DWORD)&Vel );
	
			float fLength = D3DXVec3Length(&(Vel));
			if(fLength > 100.f)
			{
				fLength = 100.f;
			}
			float fPower = fLength * 0.01f;
				
			if(fPower > 0.02f)
			{
				DamageProcess( &Pos, &fPower );
			}
		}
		if(g_pFcFXManager->SendMessage( m_nTidalWaveFxID,FX_WATER_IS_FINISHED)){		
			m_bFinished = true;
			Finalize();
		}
	}
}



/*---------------------------------------------------------------------------------
-
-			CFcAbilityWater::DamageProcess()
-					; �Ѱܹ��� ��ġ������ �� ����Ʈ�� Ŀ�ο��� �Ѱܹ޾� �Ŵ����� �߰�
-					  �ϰ�, vecProps �� �Ŵ����� �߰�. ������ �Ŵ����� ������.
-
---------------------------------------------------------------------------------*/
void CFcAbilityWater::DamageProcess(D3DXVECTOR3 *Pos,float *fPower)
{
	std::vector<GameObjHandle>vtList;


	D3DXVECTOR3 NewPos = *Pos;
	NewPos.y += 400;

	if( m_bNotFullOrb )
		g_FcWorld.GetEnemyObjectListInRange(m_hParent->GetTeam() ,&NewPos,150.f,vtList);
	else
		g_FcWorld.GetEnemyObjectListInRange(m_hParent->GetTeam() ,&NewPos,1500.f,vtList);

	if(vtList.size()){
		m_DamageObjManager.AddObj(vtList,Pos,fPower);
		vtList.clear();	
	}
	std::vector<CFcProp *> vecProps;
	g_FcWorld.GetActiveProps(*Pos,1500.f,vecProps);
	if(vecProps.size()){
		m_DamageObjManager.AddProp(vecProps,Pos);
	}
	m_DamageObjManager.Process(m_bNotFullOrb);
}
//************************************************************************************
//************************************************************************************











/*---------------------------------------------------------------------------------
-
-			CDamageObjManager::Clear()
-					; ������ ��� ����.
-					  
-
---------------------------------------------------------------------------------*/

void CDamageObjManager::Clear()
{
	unsigned int i = 0;
	for(i = 0;i < m_ObjList.size();i++)
		delete m_ObjList[i];
	m_ObjList.clear();

	for(i = 0;i < m_PropList.size();i++)
		delete m_PropList[i];
	m_PropList.clear();
}


/*---------------------------------------------------------------------------------
-
-			CDamageObjManager::Process()
-					; ����Ʈ�� �ִ� ��� ��ü�� ������.
-					  
-
---------------------------------------------------------------------------------*/
void CDamageObjManager::Process( bool p_bNotFullOrbAttack )
{
	D3DXVECTOR3 Pos;
	unsigned int i = 0;
	for(i = 0;i < m_ObjList.size();i++)
	{
		if( CFcBaseObject::IsValid( m_ObjList[i]->GetObjHandle() ) )
		{
			m_ObjList[i]->Process(GetGameHandle(), p_bNotFullOrbAttack);
		}
		else
		{
			m_ObjList[i]->SetDamage(true );
		}
	}
	for(i = 0;i < m_PropList.size();i++)
	{
		m_PropList[i]->Process(GetGameHandle(), p_bNotFullOrbAttack);
	}
}


/*---------------------------------------------------------------------------------
-
-			CDamageObjManager::AddObj()
-					; obj �߰�. 
-					  obj ����Ʈ�� �޾Ƽ� ���� ����� ����Ʈ�� ���� ������ �߰�.
-
---------------------------------------------------------------------------------*/
void CDamageObjManager::AddObj(std::vector<GameObjHandle> &ObjList,D3DXVECTOR3 *Pos,float *fPower)
{	
	for(unsigned int i = 0;i < ObjList.size();i++)
	{
		bool bExist = false;
		unsigned int size = m_ObjList.size();
		for(unsigned int s = 0;s < size;s++)
		{
			int ObjHandle = m_ObjList[s]->GetObjHandle().GetHandle();
			int ParHandle = GetGameHandle().GetHandle();
			if(ObjList[i] == m_ObjList[s]->GetObjHandle() || ParHandle == ObjHandle)
			{
				bExist = true;
				break;
			}
		}
		if(bExist == false )
		{
			CDamageObj *pObj = new CDamageObj;
			pObj->SetObjHandle(ObjList[i]);
			pObj->SetWaterPoint(Pos);
			pObj->SetPower(*fPower);
			m_ObjList.push_back(pObj);
		}
	}
}


/*---------------------------------------------------------------------------------
-
-			CDamageObjManager::AddProp()
-					; Prop �߰�
-					  Prop����Ʈ�� �޾Ƽ� ���� ������ �߰�. 
-
---------------------------------------------------------------------------------*/
void CDamageObjManager::AddProp(std::vector<CFcProp *> &ObjList,D3DXVECTOR3 *Pos)
{	
	for(unsigned int i = 0;i < ObjList.size();i++)
	{
		bool bExist = false;
		for(unsigned int s = 0;s < m_PropList.size();s++)
		{
			if(ObjList[i] == m_PropList[s]->GetPropHandle())
			{
				bExist = true;
				break;
			}
		}
		if(bExist == false)
		{
			CDamageProp *pObj = new CDamageProp;
			pObj->SetPropHandle(ObjList[i]);
			pObj->SetWaterPoint(Pos);
			m_PropList.push_back(pObj);
		}
	}
}
//************************************************************************************
//************************************************************************************












/*---------------------------------------------------------------------------------
-
-			CDamageObj::~CDamageObj()
-					; �Ҹ���
-					  
-
---------------------------------------------------------------------------------*/
CDamageObj::~CDamageObj()
{
	m_ObjHandle->SetGravity(DEFAULT_GRAVITY);
}


/*---------------------------------------------------------------------------------
-
-			CDamageObj::Process()
-					; �������� �޾Ҵ� ������, �׸��� ������� �����ִ��� üũ����
-					  ������Ʈ�� ������.
-
---------------------------------------------------------------------------------*/
void CDamageObj::Process(GameObjHandle p_hParent, bool p_bNotFullOrbAttack)
{
	if(GetDamage() == false && m_ObjHandle->GetHP())
	{
		CCrossVector *pCross = m_ObjHandle->GetCrossVector();
		/* <- �ʿ��ؼ� Ǯ��� �Ѵٸ� Siva ���� ������ ���ּ���. �ϴ� ���Ƴ����ϴ�.
		pCross->m_PosVector.y += (m_WaterPoint->y - pCross->m_PosVector.y) * 0.3f;

		D3DXVECTOR3 Dir = pCross->m_PosVector - p_hParent->GetCrossVector()->m_PosVector;
		Dir.y = 0.0f;
		D3DXVec3Normalize(&Dir,&Dir);
		pCross->m_ZVector = -Dir;													
		pCross->UpdateVectors();
		*/

		float fPowerY = m_fPower * 3 + Random(10);
		float fPowerZ = m_fPower * 10 + Random(10);

		int power = ( (p_hParent->GetLevel()+1) * 5) > 25 ? 25 :  ( (p_hParent->GetLevel()+1) * 5);

		if( p_bNotFullOrbAttack )		// �Ϲ� ������
		{
			_FC_RANGE_DAMAGE rd;
			rd.Attacker = p_hParent;
			rd.pPosition = &pCross->m_PosVector;
			rd.fRadius = 5.0f;
			rd.nPower = g_nPower[p_hParent->GetLevel()];
			rd.nPushY = g_nPushY; 
			rd.nPushZ = g_nPushX;
			rd.nTeam = p_hParent->GetTeam();
			rd.nNotGenOrb = 0;
			rd.nNotLookAtHitUnit = 0;
			rd.p_bTeamDamage = false;
			rd.pHitDirection = &p_hParent->GetCrossVector()->m_ZVector; 
			rd.nGuardBreak = 10;
            g_FcWorld.GiveRangeDamage( rd, false );
		}
		else							// full Orb Attack
		{
			_FC_RANGE_DAMAGE rd;
			rd.nType = HT_ABIL_WATER;
			rd.Attacker = p_hParent;
			rd.pPosition = &pCross->m_PosVector;
			rd.fRadius = 400.0f;
			rd.nPower = 5000;
			rd.nPushY = 15; 
			rd.nPushZ = 70;
			rd.nTeam = p_hParent->GetTeam();
			rd.nNotGenOrb = g_FcWorld.GetAbilityMng()->GetOrbGenType();
			rd.nOrbGenPer = g_FcWorld.GetAbilityMng()->GetOrbGenPer();
			rd.nNotLookAtHitUnit = 0;
			rd.p_bTeamDamage = false;
			rd.pHitDirection = &p_hParent->GetCrossVector()->m_ZVector; 
			g_FcWorld.GiveRangeDamage( rd );
		}
		SetDamage(true );
	}
}
//************************************************************************************
//************************************************************************************











/*---------------------------------------------------------------------------------
-
-			CDamageProp::Process()
-					; ���� ���� �Ȱ��� �κ�.
-					  
-
---------------------------------------------------------------------------------*/
void CDamageProp::Process(GameObjHandle p_hParent, bool p_bNotFullOrbAttack)
{	
	if(GetDamage() == false)
	{
		CFcBreakableProp* pBreakable = (CFcBreakableProp*) m_ObjHandle;
		if(pBreakable->IsCrushProp() || pBreakable->IsDynamicProp())
		{	
			D3DXVECTOR3 PropVec(pBreakable->GetPosV2().x,0,pBreakable->GetPosV2().y);			
			D3DXVECTOR3 Test = PropVec - p_hParent->GetCrossVector()->m_PosVector;
			D3DXVec3Normalize(&Test,&Test);
			
			if(p_bNotFullOrbAttack)
                Test = Test * 0.1f;
			else
				Test = Test * TW_FORCE_VAL;

			if( p_bNotFullOrbAttack ) //�Ϲݰ���
			{
				pBreakable->AddForce( &Test, NULL,  100 );
			}
			else						// Ǯ���� ����ũ
			{
				pBreakable->AddForce( &Test, NULL, 1000000, PROP_BREAK_TYPE_SPECIAL_ATTACK );
			}
		}
		SetDamage(true);
	}
}
//---------------------------------------------------------------------------------------------------