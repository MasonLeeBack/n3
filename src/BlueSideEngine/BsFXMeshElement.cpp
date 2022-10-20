#include "StdAfx.h"
#include "BsFXElement.h"
#include "BsFXMeshElement.h"
#include "BsKernel.h"
#include "BsFXLoadObject.h"
#include "bstreamext.h"
#include "BsFXTable.h"
#include "BsFXTableValue.h"
#include "CrossVector.h"
#include "BsCommon.h"
int CBsFXMeshElement::m_nCount = 0;
CBsFXMeshElement::CBsFXMeshElement( CBsFXElement *pParent )
: CBsFXElement( pParent )
{
	m_Type = CBsFXElement::MESH;
	m_nAmbientCustomParamIndex = -1;
	m_nDiffuseCustomParamIndex = -1;




	if( pParent ) {
		m_bClone = true;

		m_nViewAxis = ((CBsFXMeshElement*)pParent)->m_nViewAxis;
		m_nFixedAxis = ((CBsFXMeshElement*)pParent)->m_nFixedAxis;

		m_nBlendOP = ((CBsFXMeshElement*)pParent)->m_nBlendOP;
		m_nSourceBlend = ((CBsFXMeshElement*)pParent)->m_nSourceBlend;
		m_nDestBlend = ((CBsFXMeshElement*)pParent)->m_nDestBlend;

		m_bCheckZBuffer = ((CBsFXMeshElement*)pParent)->m_bCheckZBuffer;

		for( int i=0; i<3; i++ )
			m_pTableScaleAxis[i] = ((CBsFXMeshElement*)pParent)->m_pTableScaleAxis[i];
		for( int i=0; i<FX_MAX_CUSTOM_PARAMETER_COUNT; i++ ) {
			m_CustomParam[i].nParamIndex = ((CBsFXMeshElement*)pParent)->m_CustomParam[i].nParamIndex;
			m_CustomParam[i].nTableType = ((CBsFXMeshElement*)pParent)->m_CustomParam[i].nTableType;
			m_CustomParam[i].pTable = ((CBsFXMeshElement*)pParent)->m_CustomParam[i].pTable;
		}
	}
	else {
		m_bClone = false;

		m_nViewAxis = 0;
		m_nFixedAxis = 0;

		m_nBlendOP = (int)D3DBLENDOP_ADD;
		m_nSourceBlend = (int)D3DBLEND_SRCALPHA;
		m_nDestBlend = (int)D3DBLEND_INVSRCALPHA;
		m_bCheckZBuffer = TRUE;

		for( int i=0; i<3; i++ ) {
			m_pTableScaleAxis[i] = new CBsFXTable;
		}
		for( int i=0; i<FX_MAX_CUSTOM_PARAMETER_COUNT; i++ ) {
			m_CustomParam[i].nParamIndex = -1;
			m_CustomParam[i].nTableType = -1;
			m_CustomParam[i].pTable = new CBsFXTable;
		}
	}

	if ( m_nLoadIndex != -1 ) {
		g_BsKernel.AddSkinRef( m_nLoadIndex );
	}
}

CBsFXMeshElement::~CBsFXMeshElement()
{	
	if( m_bClone == false ) {
		for( int i=0; i<3; i++ ) {
			SAFE_DELETE( m_pTableScaleAxis[i] );
		}
		for( int i=0; i<FX_MAX_CUSTOM_PARAMETER_COUNT; i++ ) {
			SAFE_DELETE( m_CustomParam[i].pTable );
		}
	}

    SAFE_RELEASE_SKIN(m_nLoadIndex);	
}

void CBsFXMeshElement::DeleteEngineObject()
{
	if( m_nObjectIndex != -1 )
	{
		g_BsKernel.DeleteObject( m_nObjectIndex );
		m_nObjectIndex = -1;
	}
}

void CBsFXMeshElement::Process( DWORD dwTime, float fDistanceFromCamera, int count /*= 0*/ )
{
	if( m_bShow == FALSE ) return;
	if( m_nObjectIndex == -1 ) return;
	if( dwTime < m_dwStartFrame || dwTime >= m_dwEndFrame ) return;

	float fTime = (float)(dwTime - m_dwStartFrame);
	DWORD dwLength = m_dwEndFrame - m_dwStartFrame;
	if( dwLength > 1 ) dwLength--;
	float fPercent = ( 1.f / (float)dwLength ) * ( dwTime - m_dwStartFrame );
	// 보간 하자!!
	// 일단 포지션, 로테이션, 스케일 보간!!
	// Position
	D3DXVECTOR3 *pPos;
	switch( m_nPositionInterpolationType ) {
		case 0:	// Linear
			pPos = (D3DXVECTOR3 *)m_pTable[POSITION_TABLE]->GetInterpolation( fTime, CBsFXTable::IP_Vec3_Value_Linear );
			break;
		case 1:	// Spline
			pPos = (D3DXVECTOR3 *)m_pTable[POSITION_TABLE]->GetInterpolation( fTime, CBsFXTable::IP_Vec3_Value_HS );
			break;
	}
	BsAssert( NULL != pPos );	// mruete: prefix bug 318: added assert
	// Scale
	// mruete: prefix bug 317: added intermediate variable with assertion
	float * pfScale = (float *)m_pTable[SCALE_TABLE]->GetInterpolation( fPercent, CBsFXTable::IP_Float_Value_Linear );
	BsAssert( NULL != pfScale );
	float fScale = *pfScale;
	// Scale Axis
	float fScaleAxis[3] = { 1.f, 1.f, 1.f };
	for( int i=0; i<3; i++ ) {
		if( m_pTableScaleAxis[i]->GetCount() )
			fScaleAxis[i] = *(float *)m_pTableScaleAxis[i]->GetInterpolation( fPercent, CBsFXTable::IP_Float_Value_Linear );
	}

	D3DXVECTOR3 vScale = D3DXVECTOR3( fScale * fScaleAxis[0], fScale * fScaleAxis[1], fScale * fScaleAxis[2] );
	if( m_nViewAxis == 0 || m_nFixedAxis == 0 ) {
		// Rotation
		// 쿼터니온일경우에 문제가 생김..
		D3DXQUATERNION Quat;
		if( m_pTable[ROTATE_TABLE]->GetCount() == 1 ) {
			D3DXVECTOR3 Temp = *(D3DXVECTOR3 *)(m_pTable[ROTATE_TABLE]->GetValueFromIndex(0)->GetValue());
			Temp.x = Temp.x / 180.f * D3DX_PI;
			Temp.y = Temp.y / 180.f * D3DX_PI;
			Temp.z = Temp.z / 180.f * D3DX_PI;
			D3DXQuaternionRotationYawPitchRoll( &Quat, Temp.x, Temp.y, Temp.z );
		}
		else {
			Quat = *(D3DXQUATERNION *)m_pTable[ROTATE_TABLE]->GetInterpolation( fTime, CBsFXTable::IP_Vec3ToQuat_Value_Linear );
		}

		D3DXMatrixTransformation( &m_matObject, NULL, NULL, &vScale, NULL, &Quat, pPos );
		D3DXMatrixMultiply( &m_matObject, &m_matObject, m_pmatParent );
	}
	else {
		// 축 변환
		static D3DXMATRIX matView, matScale;
		static D3DXVECTOR3 vFixedVec;
		static CCrossVector CrossResult;
		static D3DXVECTOR3 vTempPos;

		switch( m_nFixedAxis ) {
			case 1:	vFixedVec = D3DXVECTOR3( 1.f, 0.f, 0.f );	break;
			case 2:	vFixedVec = D3DXVECTOR3( 0.f, 1.f, 0.f );	break;
			case 3:	vFixedVec = D3DXVECTOR3( 0.f, 0.f, 1.f );	break;
			case 4: vFixedVec = D3DXVECTOR3( 0.f, 0.f, 0.f );	break;
		}
		int nCamIndex = g_BsKernel.GetCameraHandle(0);
		CBsCamera *pCamera = g_BsKernel.GetCamera(nCamIndex);
		matView = *(D3DXMATRIX*)pCamera->GetViewMatrix();
		D3DXMatrixInverse( &matView, NULL, &matView );

		vTempPos = *pPos;// + (*(D3DXVECTOR3*)&m_pmatParent->_41);
		D3DXVec3TransformCoord(&vTempPos, &vTempPos, m_pmatParent);
		D3DXVec3TransformNormal(&vFixedVec, &vFixedVec, m_pmatParent);
		D3DXVec3Normalize(&vFixedVec, &vFixedVec);
		CalcBillboardMatrix( &CrossResult, &vFixedVec, &vTempPos, (D3DXVECTOR3*)&matView._41 );
		
		switch( m_nViewAxis ) {
			case 1: CrossResult.RotateYaw( -256 );	break;
			case 2: CrossResult.RotateYaw( 256 );	break;
			case 3:	CrossResult.RotatePitch( -256 );break;
			case 4:	CrossResult.RotatePitch( 256 );	break;
			case 6: CrossResult.RotateYaw( 512 );	break;
		}

		vScale.x *= D3DXVec3Length((D3DXVECTOR3*)&m_pmatParent->_11);
		vScale.y *= D3DXVec3Length((D3DXVECTOR3*)&m_pmatParent->_21);
		vScale.z *= D3DXVec3Length((D3DXVECTOR3*)&m_pmatParent->_31);


		D3DXMatrixTransformation( &matScale, NULL, NULL, &vScale, NULL, NULL, NULL );
		D3DXMatrixMultiply( &m_matObject, &matScale, CrossResult );
	}

	// 그담 칼라, 알파 보간!!
	float *fpAlpha = (float *)m_pTable[ALPHA_TABLE]->GetInterpolation( fPercent, CBsFXTable::IP_Float_Value_Linear );
	D3DXVECTOR3 *pColor = (D3DXVECTOR3 *)m_pTable[COLOR_TABLE]->GetInterpolation( fPercent, CBsFXTable::IP_Vec3_Value_Linear );
	BsAssert( NULL != fpAlpha && NULL != pColor ); // mruete: prefix bug 319: added assert
	m_Color.r = pColor->x;
	m_Color.g = pColor->y;
	m_Color.b = pColor->z;
	m_Color.a = *fpAlpha;

	D3DXVECTOR4 *vColor = (D3DXVECTOR4 *)&m_Color;
	for( int i=0; i<m_nSubMeshCount; i++ ) {
		g_BsKernel.SendMessage( m_nObjectIndex, BS_SET_EDITABLE_PARAMETER, i, m_nAmbientCustomParamIndex, (DWORD)vColor );
		g_BsKernel.SendMessage( m_nObjectIndex, BS_SET_EDITABLE_PARAMETER, i, m_nDiffuseCustomParamIndex, (DWORD)vColor );



		g_BsKernel.SendMessage( m_nObjectIndex, BS_SET_SUBMESH_DEPTHCHECKENABLE, i, (DWORD)m_bCheckZBuffer );
		if( m_Color.a >= 1.f ) {
			g_BsKernel.SendMessage( m_nObjectIndex, BS_SET_SUBMESH_ALPHABLENDENABLE, i, FALSE );
		}
		else {
			g_BsKernel.SendMessage( m_nObjectIndex, BS_SET_SUBMESH_ALPHABLENDENABLE, i, TRUE );
			g_BsKernel.SendMessage( m_nObjectIndex, BS_SET_SUBMESH_ALPHA, i, (DWORD)&m_Color.a );

			g_BsKernel.SendMessage( m_nObjectIndex, BS_SET_SUBMESH_BLENDOP, i, (DWORD)m_nBlendOP );
			g_BsKernel.SendMessage( m_nObjectIndex, BS_SET_SUBMESH_SRCBLEND, i, (DWORD)m_nSourceBlend );
			g_BsKernel.SendMessage( m_nObjectIndex, BS_SET_SUBMESH_DESTBLEND, i, (DWORD)m_nDestBlend );
		}
	}
	// Custom Parameter
	for( int i=0; i<FX_MAX_CUSTOM_PARAMETER_COUNT; i++ ) {
		if( m_CustomParam[i].nParamIndex == -1 ) continue;

		D3DXVECTOR4 vResult = D3DXVECTOR4( 0.f, 0.f, 0.f, 1.f );
		switch( m_CustomParam[i].nTableType ) {
			case 0:
				vResult.x = *(float*)m_CustomParam[i].pTable->GetInterpolation( fPercent, CBsFXTable::IP_Float_Value_Linear );
				break;
			case 1:
				D3DXVECTOR3 *pColor;
				pColor = (D3DXVECTOR3 *)m_CustomParam[i].pTable->GetInterpolation( fPercent, CBsFXTable::IP_Vec3_Value_Linear );
				vResult.x = pColor->x;
				vResult.y = pColor->y;
				vResult.z = pColor->z;
				break;
			case 2:
				vResult.x = *(float*)m_CustomParam[i].pTable->GetInterpolation( fTime, NULL );
				break;
			case 3:
				vResult.x = *(float*)m_CustomParam[i].pTable->GetInterpolation( fTime, CBsFXTable::IP_Float_Value_Linear );
				break;
		}
		g_BsKernel.SendMessage( m_nObjectIndex, BS_SET_EDITABLE_PARAMETER, (m_CustomParam[i].nParamIndex>>16), (m_CustomParam[i].nParamIndex<<16)>>16, (DWORD)&vResult );
	}

	// UpdateObject
	/*
	CBsObject* pObject = g_BsKernel.GetEngineObjectPtr( m_nObjectIndex );
	pObject->InitRender( fDistanceFromCamera );
	pObject->SetObjectMatrix( &m_matObject );
	*/
	g_BsKernel.UpdateObject( m_nObjectIndex, &m_matObject );
}

BOOL CBsFXMeshElement::Load( BStream *pStream )
{
	CBsFXElement::Load( pStream );

	pStream->Read( &m_fUVAniLoopRatio, sizeof(float), ENDIAN_FOUR_BYTE );
	if( CBsFXTemplate::m_fCurLoadVersion >= 1.5f ) {
		pStream->Read( &m_nViewAxis, sizeof(int), ENDIAN_FOUR_BYTE );
		pStream->Read( &m_nFixedAxis, sizeof(int), ENDIAN_FOUR_BYTE );
		pStream->Read( &m_nBlendOP, sizeof(int), ENDIAN_FOUR_BYTE );
		pStream->Read( &m_nSourceBlend, sizeof(int), ENDIAN_FOUR_BYTE );
		pStream->Read( &m_nDestBlend, sizeof(int), ENDIAN_FOUR_BYTE );

		if( CBsFXTemplate::m_fCurLoadVersion >= 1.6f ) {
			pStream->Read( &m_bCheckZBuffer, sizeof(BOOL), ENDIAN_FOUR_BYTE );
		}

		for( int i=0; i<3; i++ ) {
			m_pTableScaleAxis[i]->Load( pStream );
		}

		int nCustomParam = 0;
		if( CBsFXTemplate::m_fCurLoadVersion >= 1.8f ) 
		{
			nCustomParam = FX_MAX_CUSTOM_PARAMETER_COUNT;

		}
		else if( CBsFXTemplate::m_fCurLoadVersion >= 1.7f ) 
		{
			nCustomParam = 3;

		}
		for( int i=0; i<nCustomParam; i++ ) {
			pStream->Read( &m_CustomParam[i].nParamIndex, sizeof(int), ENDIAN_FOUR_BYTE );
			if( m_CustomParam[i].nParamIndex == -1 ) continue;

			pStream->Read( &m_CustomParam[i].nTableType, sizeof(int), ENDIAN_FOUR_BYTE );
			m_CustomParam[i].pTable->Load( pStream );
		}
		


#ifdef _XBOX
		switch( m_nBlendOP ) {
			case 1:	m_nBlendOP = (int)D3DBLENDOP_ADD;	break;
			case 2:	m_nBlendOP = (int)D3DBLENDOP_SUBTRACT;	break;
			case 3:	m_nBlendOP = (int)D3DBLENDOP_REVSUBTRACT;	break;
			case 4:	m_nBlendOP = (int)D3DBLENDOP_MIN;	break;
			case 5:	m_nBlendOP = (int)D3DBLENDOP_MAX;	break;
		}
		switch( m_nSourceBlend ) {
			case 1:	m_nSourceBlend = D3DBLEND_ZERO;	 break;
			case 2:	m_nSourceBlend = D3DBLEND_ONE;	 break;
			case 3:	m_nSourceBlend = D3DBLEND_SRCCOLOR ;	 break;
			case 4:	m_nSourceBlend = D3DBLEND_INVSRCCOLOR;	 break;
			case 5:	m_nSourceBlend = D3DBLEND_SRCALPHA;	 break;
			case 6:	m_nSourceBlend = D3DBLEND_INVSRCALPHA;	 break;
			case 7:	m_nSourceBlend = D3DBLEND_DESTALPHA;	 break;
			case 8:	m_nSourceBlend = D3DBLEND_INVDESTALPHA;	 break;
			case 9:	m_nSourceBlend = D3DBLEND_DESTCOLOR;	 break;
			case 10:m_nSourceBlend = D3DBLEND_INVDESTCOLOR;	 break;
			case 11:m_nSourceBlend = D3DBLEND_SRCALPHASAT;	 break;
		}

		switch( m_nDestBlend ) {
			case 1:	m_nDestBlend = D3DBLEND_ZERO;	 break;
			case 2:	m_nDestBlend = D3DBLEND_ONE;	 break;
			case 3:	m_nDestBlend = D3DBLEND_SRCCOLOR ;	 break;
			case 4:	m_nDestBlend = D3DBLEND_INVSRCCOLOR;	 break;
			case 5:	m_nDestBlend = D3DBLEND_SRCALPHA;	 break;
			case 6:	m_nDestBlend = D3DBLEND_INVSRCALPHA;	 break;
			case 7:	m_nDestBlend = D3DBLEND_DESTALPHA;	 break;
			case 8:	m_nDestBlend = D3DBLEND_INVDESTALPHA;	 break;
			case 9:	m_nDestBlend = D3DBLEND_DESTCOLOR;	 break;
			case 10:m_nDestBlend = D3DBLEND_INVDESTCOLOR;	 break;
			case 11:m_nDestBlend = D3DBLEND_SRCALPHASAT;	 break;
		}
#endif //_XBOX
	}

	SAFE_RELEASE_SKIN(m_nLoadIndex);

	m_nLoadIndex = g_BsKernel.LoadSkin( -1, m_szFileName.c_str() );

	if( m_nLoadIndex == -1 )
	{
		DebugString( "%s load fail in FX\n", m_szFileName.c_str() );
		return FALSE;
	}

	return TRUE;
}

BOOL CBsFXMeshElement::Activate()
{
	m_nObjectIndex = g_BsKernel.CreateStaticObjectFromSkin( m_nLoadIndex );

	if( m_nObjectIndex == -1 ) return FALSE;

	m_nSubMeshCount = g_BsKernel.SendMessage( m_nObjectIndex, BS_GET_SUBMESH_COUNT );
	for( int i=0; i<m_nSubMeshCount; i++ ) {
		m_nAmbientCustomParamIndex = g_BsKernel.SendMessage( m_nObjectIndex, BS_ADD_EDITABLE_PARAMETER, i, PARAM_MATERIALAMBIENT );
		m_nDiffuseCustomParamIndex = g_BsKernel.SendMessage( m_nObjectIndex, BS_ADD_EDITABLE_PARAMETER, i, PARAM_MATERIALDIFFUSE );

		// Alpha Blend Enable
		g_BsKernel.SendMessage( m_nObjectIndex, BS_SET_SUBMESH_ALPHABLENDENABLE, i, TRUE );
	}

	return TRUE;
}

void CBsFXMeshElement::CalcBillboardMatrix( CCrossVector *pOutput, D3DXVECTOR3 *pUpVec, D3DXVECTOR3 *pPosition, D3DXVECTOR3 *pCameraPos )
{
	D3DXVECTOR3 DirVec, XVec, ZVec;

	if( D3DXVec3LengthSq( pUpVec ) <= 0.f ) {
		D3DXVECTOR3 vUpVec, vCross;

		vUpVec = *pCameraPos - *pPosition;
		D3DXVec3Normalize( &vUpVec, &vUpVec );
		pOutput->m_ZVector = vUpVec;

		D3DXVec3Cross( &vCross, &D3DXVECTOR3( 0.f, 1.f, 0.f ), &vUpVec );
		D3DXVec3Normalize( &vCross, &vCross );
		pOutput->m_XVector = vCross;

		D3DXVec3Cross( &vUpVec, &vUpVec, &vCross );
		D3DXVec3Normalize( &vUpVec, &vUpVec );
		pOutput->m_YVector = vUpVec;

		pOutput->m_PosVector = *pPosition;
		return;
	}

	DirVec = *pCameraPos - *pPosition;
	D3DXVec3Normalize( &DirVec, &DirVec );
	D3DXVec3Cross( &pOutput->m_XVector, pUpVec, &DirVec );
	D3DXVec3Normalize( &pOutput->m_XVector, &pOutput->m_XVector );
	D3DXVec3Cross( &pOutput->m_ZVector,  &pOutput->m_XVector, pUpVec );
	D3DXVec3Normalize( &pOutput->m_ZVector, &pOutput->m_ZVector );

	pOutput->m_YVector = *pUpVec;
	pOutput->m_PosVector = *pPosition;

}
