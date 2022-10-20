#include "StdAfx.h"
#include "BsFXPLElement.h"
#include "BsFXTable.h"
#include "BsKernel.h"
#include "BsCommon.h"
#include "bstreamext.h"

CBsFXPLElement::CBsFXPLElement( CBsFXElement *pParent )
: CBsFXElement( pParent )
{
	m_Type = CBsFXElement::POINT_LIGHT;
	m_nObjectIndex = -1;

	if( pParent ) {
		m_nAttenuation = ((CBsFXPLElement*)pParent)->m_nAttenuation;
	}
	else {
		m_nAttenuation = 500;
	}
}

CBsFXPLElement::~CBsFXPLElement()
{

}

void CBsFXPLElement::DeleteEngineObject()
{
	if( m_nObjectIndex != -1 ) {
		g_BsKernel.DeleteObject( m_nObjectIndex );
		m_nObjectIndex = -1;
	}
}

BOOL CBsFXPLElement::Activate()
{
	m_nObjectIndex = g_BsKernel.CreateLightObject( ::POINT_LIGHT );
	return TRUE;
}

BOOL CBsFXPLElement::Load( BStream *pStream )
{
	CBsFXElement::Load( pStream );

	pStream->Read( &m_nAttenuation, sizeof(int), ENDIAN_FOUR_BYTE );

	return TRUE;
}

void CBsFXPLElement::Process( DWORD dwTime, float fDistanceFromCamera, int count /*= 0*/ )
{
	if( m_bShow == FALSE ) return;
	if( dwTime < m_dwStartFrame || dwTime >= m_dwEndFrame ) return;

	float fTime = (float)(dwTime - m_dwStartFrame);
	DWORD dwLength = m_dwEndFrame - m_dwStartFrame;
	if( dwLength > 1 ) dwLength--;
	float fPercent = ( 1.f / (float)dwLength ) * ( dwTime - m_dwStartFrame );

	// 보간 하자!!
	// 일단 포지션
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
	BsAssert( NULL != pPos );	// mruete: prefix bug 323: added assert

	// Range
	float *fpScale = (float *)m_pTable[SCALE_TABLE]->GetInterpolation( fPercent, CBsFXTable::IP_Float_Value_Linear );
	BsAssert( NULL != fpScale );	// mruete: prefix bug 321: added assert
	*fpScale *= (float)m_nAttenuation;

	m_matObject._41 = pPos->x;
	m_matObject._42 = pPos->y;
	m_matObject._43 = pPos->z;
	D3DXMatrixMultiply( &m_matObject, &m_matObject, m_pmatParent );

	// 디퓨즈, 스펙 칼라
	D3DXVECTOR3 *pColor = (D3DXVECTOR3 *)m_pTable[COLOR_TABLE]->GetInterpolation( fPercent, CBsFXTable::IP_Vec3_Value_Linear );
	BsAssert( NULL != pColor );	// mruete: prefix bug 323: added assert
	m_Color.r = pColor->x;
	m_Color.g = pColor->y;
	m_Color.b = pColor->z;
	m_Color.a = 1.f;

	g_BsKernel.SendMessage( m_nObjectIndex, BS_SET_POINTLIGHT_COLOR, (DWORD)&m_Color );
	g_BsKernel.SendMessage( m_nObjectIndex, BS_SET_POINTLIGHT_RANGE, (DWORD)&(*fpScale) );

	// Update Object
	g_BsKernel.UpdateObject( m_nObjectIndex, &m_matObject );
	/*
	CBsObject* pObject = g_BsKernel.GetEngineObjectPtr( m_nObjectIndex );
	pObject->InitRender( fDistanceFromCamera );
	pObject->SetObjectMatrix( &m_matObject );
	*/
}

void CBsFXPLElement::SetCurFrame( DWORD dwValue )
{
}

DWORD CBsFXPLElement::GetCurFrame()
{
	return 0;
}
