#include "StdAfx.h"
#include "BsKernel.h"
#include "BsFXElement.h"
#include "BsFXTable.h"
#include "bstreamext.h"
#include "BsFXTableValue.h"


CBsFXElement::CBsFXElement( CBsFXElement *pParent )
{
	m_Type = CBsFXElement::UNKNOWN;

	if( pParent ) {
		m_bClone = true;

		m_Type = pParent->m_Type;
		m_nObjectIndex = -1;
		m_nLoadIndex = pParent->m_nLoadIndex;
		m_pmatParent = NULL;
		m_bShow = TRUE;
		m_dwStartFrame = pParent->m_dwStartFrame;
		m_dwEndFrame = pParent->m_dwEndFrame;
		m_nPositionInterpolationType = pParent->m_nPositionInterpolationType;
		D3DXMatrixIdentity( &m_matObject );

		for( DWORD i=0; i<TABLE_TYPE_AMOUNT; i++ ) {
			m_pTable[i] = pParent->m_pTable[i];
		}
	}
	else {
		m_bClone = false;

		m_nObjectIndex = -1;
		m_nLoadIndex = -1;
		m_pmatParent = NULL;
		m_bShow = TRUE;
		m_nPositionInterpolationType = 0;
		D3DXMatrixIdentity( &m_matObject );

		for( DWORD i=0; i<TABLE_TYPE_AMOUNT; i++ ) {
			m_pTable[i] = new CBsFXTable;
		}
	}
}

CBsFXElement::~CBsFXElement()
{
	if( m_bClone == false ) {
		for( DWORD i=0; i<TABLE_TYPE_AMOUNT; i++ ) {
			delete m_pTable[i];
			m_pTable[i] = NULL;
		}
	}

	BsAssert( m_nLoadIndex == -1 );
}

CBsFXElement::ELEMENT_TYPE CBsFXElement::GetType()
{
	return m_Type;
}

CBsFXTable *CBsFXElement::GetTable( TABLE_TYPE Type )
{
	return m_pTable[Type];
}

DWORD CBsFXElement::GetStartFrame()
{
	return m_dwStartFrame;
}

DWORD CBsFXElement::GetEndFrame()
{
	return m_dwEndFrame;
}

void CBsFXElement::Show()
{
	m_bShow = TRUE;
}

void CBsFXElement::Hide()
{
	m_bShow = FALSE;
}

BOOL CBsFXElement::IsShow()
{
	return m_bShow;
}

void CBsFXElement::SetParentMatrix( D3DXMATRIX *pmat )
{
	m_pmatParent = pmat;
}

BOOL CBsFXElement::Activate()
{
	return TRUE;
}

int CBsFXElement::GetObjectIndex()
{
	return m_nObjectIndex;
}

void CBsFXElement::Process( DWORD dwTime, float fDistanceFromCamera, int count /*= 0*/ )
{
	if( m_bShow == FALSE ) return;
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
	// Scale
	float *fpScale = (float *)m_pTable[SCALE_TABLE]->GetInterpolation( fPercent, CBsFXTable::IP_Float_Value_Linear );
	BsAssert( NULL != fpScale );	// mruete: prefix bug 314: added assert
	D3DXVECTOR3 vScale = D3DXVECTOR3( *fpScale, *fpScale, *fpScale );
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

	// 그담 칼라, 알파 보간!!
	float *fpAlpha = (float *)m_pTable[ALPHA_TABLE]->GetInterpolation( fPercent, CBsFXTable::IP_Float_Value_Linear );
	D3DXVECTOR3 *pColor = (D3DXVECTOR3 *)m_pTable[COLOR_TABLE]->GetInterpolation( fPercent, CBsFXTable::IP_Vec3_Value_Linear );
	m_Color.r = pColor->x;
	m_Color.g = pColor->y;
	m_Color.b = pColor->z;
	m_Color.a = *fpAlpha;
}

BOOL CBsFXElement::Load( BStream *pStream )
{
	m_szFileName.clear();
	ReadString( &m_szFileName, pStream );

	pStream->Read( &m_dwStartFrame, sizeof(DWORD), ENDIAN_FOUR_BYTE );
	pStream->Read( &m_dwEndFrame, sizeof(DWORD), ENDIAN_FOUR_BYTE );
	pStream->Read( &m_nPositionInterpolationType, sizeof(int), ENDIAN_FOUR_BYTE );

	for( int i=0; i<TABLE_TYPE_AMOUNT; i++ ) {
		m_pTable[i]->Reset();
		m_pTable[i]->Load( pStream );
	}
	return TRUE;
}
D3DXMATRIX *CBsFXElement::GetMatrix()
{
	return &m_matObject;
}

void ReadString( std::string *szStr, BStream *pStream )
{
	int nLength;
	char szBuf[512] = {0,};
	pStream->Read( &nLength, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Read( szBuf, nLength );
	*szStr = szBuf;
}

int CBsFXElement::GetPositionInterpolationType()
{
	return m_nPositionInterpolationType;
}
