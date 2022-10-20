#include "StdAfx.h"
#include ".\fcdropweapon.h"
#include "BsPhysicsMgr.h"
#include "FcGlobal.h"
#include "FcUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

DWORD CFcDropWeapon::m_sRenderTick = 0;
std::vector	<CFcDropWeapon*> CFcDropWeapon::m_AgentAllList;

CFcDropWeapon::CFcDropWeapon( CCrossVector *pCross )
	: CFcBaseObject( pCross )
{
	m_pActor = NULL;
}

CFcDropWeapon::~CFcDropWeapon(void)
{
	if( m_pActor )
	{
		delete m_pActor;
		m_pActor = NULL;
	}

	int i, nSize;
	
	nSize = m_AgentAllList.size();
	for( i = 0; i < nSize; i++) {
		if( m_AgentAllList[i] == this ) {
            m_AgentAllList.erase( m_AgentAllList.begin() + i );
			break;
		}
	}
}

int CFcDropWeapon::Initialize( int nObjectIndex, D3DXVECTOR3 *pForceVector )
{
	AABB *pBox;
	D3DXVECTOR3 Size, LinVel, AngVel;

	pBox = ( AABB * )g_BsKernel.SendMessage( nObjectIndex, BS_GET_BOUNDING_BOX );
	Size = *(D3DXVECTOR3*)&(( pBox->Vmax - pBox->Vmin ) * 0.5);
	
	D3DXMatrixTranslation(&m_matLocal, -pBox->GetCenter().x*1.5f, -pBox->GetCenter().y*1.5f, -pBox->GetCenter().z*1.5f);
	
	m_nEngineIndex = nObjectIndex;
	m_nDeleteFrame = DROP_WEAPON_DELETE_FRAME;

	DWORD dwCurrentTick = GetProcessTick();
	if( dwCurrentTick > m_sRenderTick  + DROP_WEAPON_SHARE_FRAME ) 
	{
		m_AgentAllList.clear();
		m_sRenderTick = dwCurrentTick;
	}

	if( m_AgentAllList.size() >= DROP_MAX_WEAPON_COUNT_ATONCE ) 
	{        
		int nWeaponIndex = RandomNumberInRange(0, DROP_MAX_WEAPON_COUNT_ATONCE - 1);
		
		D3DXMATRIX matOrigin = *m_AgentAllList[ nWeaponIndex ]->m_CrossOrigin;
		m_DistanceMat = *m_Cross;
		matOrigin._41 = matOrigin._42 = matOrigin._43 = 0.f;
		m_DistanceMat._41 = m_DistanceMat._42 = m_DistanceMat._43 = 0.f;

		D3DXMatrixTranspose(&matOrigin, &matOrigin);
		D3DXMatrixMultiply(&m_DistanceMat, &matOrigin, &m_DistanceMat);
		*(D3DXVECTOR3*)&m_DistanceMat._41 = m_Cross.GetPosition() - m_AgentAllList[ nWeaponIndex ]->m_CrossOrigin.GetPosition() ;
		
		m_AgentAllList[ nWeaponIndex ]->AddAgentWeapon( this );		
		return 1;
	}

	m_CrossOrigin = m_Cross;
	m_AgentAllList.push_back( this );

	m_pActor = CBsPhysicsBox::Create( Size, *( ( D3DXMATRIX *)m_Cross ) );

	if( !m_pActor->GetActor() ) 
	{
		return 0;
	}
 
	m_pActor->GetActor()->setLinearDamping( 0.1f );
	m_pActor->GetActor()->setAngularDamping( 0.1f );
	m_pActor->GetActor()->setMaxAngularVelocity( FLT_MAX );

/*	float fForce = RandomNumberInRange(13.f, 18.f);
	LinVel = *pForceVector * fForce;*/

	LinVel = *pForceVector;
	AngVel = D3DXVECTOR3( 0.0f, 0.0f, -RandomNumberInRange(10.f, 13.f) );
	m_pActor->AddVelocity( &LinVel, &AngVel );

	return 1;
}

void CFcDropWeapon::SetAgentMatrix( D3DXMATRIX *AgentMat)
{
	D3DXVECTOR3 vPos = *(D3DXVECTOR3*)(&AgentMat->_41);
	D3DXMatrixMultiply( m_Cross, AgentMat, &m_DistanceMat );

	vPos += *(D3DXVECTOR3*)(&m_DistanceMat._41);
	m_Cross.SetPosition(vPos);

	D3DXMatrixMultiply( m_Cross, &m_matLocal, m_Cross);
}

void CFcDropWeapon::Process()
{
	if( m_pActor ) 
	{
		int i, nSize;
		D3DXMATRIX ActorMat;
		m_pActor->GetGlobalPose( (D3DXMATRIX&)m_Cross );

		nSize = m_AgentList.size();

		for( i = 0; i < nSize; i++)
		{
            m_AgentList[i]->SetAgentMatrix( m_Cross );
		}
		D3DXMatrixMultiply( m_Cross, &m_matLocal, m_Cross);
	}
	


	m_nDeleteFrame--;
	if( m_nDeleteFrame < DROP_WEAPON_ALPHA_FRAME )
	{
		if( m_nDeleteFrame <= 0 )
		{
			Delete();
			return;
		}
		float fAlphaWeight;

		fAlphaWeight = m_nDeleteFrame / ( float )DROP_WEAPON_ALPHA_FRAME;

		// 임시코드
		if( g_BsKernel.GetEngineObjectPtr( m_nEngineIndex ) != NULL ) {
			g_BsKernel.SendMessage( m_nEngineIndex, BS_ENABLE_OBJECT_ALPHABLEND, 1 );
			g_BsKernel.SendMessage( m_nEngineIndex, BS_SET_OBJECT_ALPHA, ( DWORD )&fAlphaWeight );
		}
		else {
			DebugString("FcDropWeapon 에러\n");
		}
	}
}
