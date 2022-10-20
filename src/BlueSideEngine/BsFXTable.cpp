#include "StdAfx.h"
#include "BsFXTable.h"
#include "BsFXTableValue.h"
#include "bstreamext.h"

bool SortFunc( CBsFXTableValue *pObj1, CBsFXTableValue *pObj2 )
{
	if( pObj1->GetKey() < pObj2->GetKey() ) return true;
	return false;
}



CBsFXTable::CBsFXTable()
{
	Reset();
}

CBsFXTable::~CBsFXTable()
{
	Reset();
}

void CBsFXTable::AddValue( CBsFXTableValue *pValue )
{
	m_pVecTable.push_back( pValue );
	CalcRange();
}

void CBsFXTable::RemoveValue( DWORD dwIndex )
{
	if( dwIndex < 0 || dwIndex >= m_pVecTable.size() ) return;

	delete m_pVecTable[dwIndex];
	m_pVecTable.erase( m_pVecTable.begin() + dwIndex );

	CalcRange();
}

void CBsFXTable::RemoveValue( CBsFXTableValue *pValue )
{
	for( DWORD i=0; i<m_pVecTable.size(); i++ ) {
		if( m_pVecTable[i] == pValue ) {
			delete m_pVecTable[i];
			m_pVecTable.erase( m_pVecTable.begin() + i );
			CalcRange();
			break;
		}
	}
}

void CBsFXTable::Load( BStream *pStream )
{
	Reset();
	DWORD dwCount;
	DWORD dwValueType;
	float fKey;
	pStream->Read( &dwCount, sizeof(DWORD), ENDIAN_FOUR_BYTE );

	for( DWORD i=0; i<dwCount; i++ ) {
		pStream->Read( &dwValueType, sizeof(DWORD), ENDIAN_FOUR_BYTE );

		CBsFXTableValue *pValue = CBsFXTableValue::CreateThis( dwValueType );

		pStream->Read( &fKey, sizeof(float), ENDIAN_FOUR_BYTE );
		pStream->Read( pValue->GetValue(), pValue->GetSize(), ENDIAN_FOUR_BYTE );

		pValue->SetKey( fKey );
		m_pVecTable.push_back( pValue );
	}
	CalcRange();
}

void CBsFXTable::Save( BStream *pStream )
{
	DWORD dwCount = GetCount();
	pStream->Write( &dwCount, sizeof(DWORD), ENDIAN_FOUR_BYTE );

	for( DWORD i=0; i<dwCount; i++ ) {
		CBsFXTableValue *pValue = GetValueFromIndex(i);
		int nType = pValue->GetValueType();
		float fKey = pValue->GetKey();

		pStream->Write( &nType, sizeof(int), ENDIAN_FOUR_BYTE );
		pStream->Write( &fKey, sizeof(float), ENDIAN_FOUR_BYTE );
		pStream->Write( pValue->GetValue(), pValue->GetSize(), ENDIAN_FOUR_BYTE );
	}
}

void CBsFXTable::Reset()
{
	m_fMinKey = 0.f;
	m_fMaxKey = 0.f;
	SAFE_DELETE_PVEC( m_pVecTable );
}

DWORD CBsFXTable::GetCount()
{
	return (DWORD)m_pVecTable.size();
}

CBsFXTableValue *CBsFXTable::GetValueFromIndex( DWORD dwIndex )
{
	if( dwIndex <0 || dwIndex >= m_pVecTable.size() ) return NULL;
	return m_pVecTable[dwIndex];
}

float CBsFXTable::GetKeyMin()
{
	return m_fMinKey;
}

float CBsFXTable::GetKeyMax()
{
	return m_fMaxKey;
}

void CBsFXTable::CalcRange()
{
	m_fMinKey = 0.f;
	m_fMaxKey = 0.f;
	for( DWORD i=0; i<m_pVecTable.size(); i++ ) {
		float fKey = m_pVecTable[i]->GetKey();
		if( m_fMinKey > fKey ) m_fMinKey = fKey;
		else if( m_fMaxKey < fKey ) m_fMaxKey = fKey;
	}
}

void CBsFXTable::Sort( bool (__cdecl *Func)(CBsFXTableValue *, CBsFXTableValue *) )
{
	if( Func == NULL ) std::sort( m_pVecTable.begin(), m_pVecTable.end(), SortFunc );
	else std::sort( m_pVecTable.begin(), m_pVecTable.end(), Func );
}

#if defined(_PREFIX_) || defined(_PREFAST_)
void *CBsFXTable::GetInterpolation( float fInterpolationKey, void *(__stdcall *Func)(float, std::vector<CBsFXTableValue *> *, DWORD) )
#else
void *CBsFXTable::GetInterpolation( float fInterpolationKey, void *(__cdecl *Func)(float, std::vector<CBsFXTableValue *> *, DWORD) )
#endif
{
	DWORD dwCount = (DWORD)m_pVecTable.size();
	if( dwCount == 0 ) return NULL;
	if( dwCount == 1 ) return m_pVecTable[0]->GetValue();
	if( fInterpolationKey < m_fMinKey ) return m_pVecTable[0]->GetValue();
	if( fInterpolationKey > m_fMaxKey ) return m_pVecTable[dwCount-1]->GetValue();

	for( DWORD i=0; i<dwCount-1; i++ ) {
		float fKey[2] = { m_pVecTable[i]->GetKey(), m_pVecTable[i+1]->GetKey() };
		if( fInterpolationKey >= fKey[0] && fInterpolationKey <= fKey[1] ) {
			float fWeight = ( 1.f / ( fKey[1] - fKey[0] ) ) * ( fInterpolationKey - fKey[0] );
			if( !Func ) return m_pVecTable[i]->GetValue();
			return Func( fWeight, &m_pVecTable, i );
		}
	}
	assert(0&&"보간이 될 수 없데요~ 말이되? 응? 응? 응?");
	return NULL;
}

// 보간용 콜백함수 기본값들..
void *CBsFXTable::IP_Float_Value_Linear( float fWeight, std::vector<CBsFXTableValue *> *pVecList, DWORD dwIndex )
{
	static float fResult;
	float *pf[2] = { (float*)(*pVecList)[dwIndex]->GetValue(), (float*)(*pVecList)[dwIndex+1]->GetValue() };

	fResult = *pf[0] + ( ( *pf[1] - *pf[0] ) * fWeight );

	return &fResult;
}

void *CBsFXTable::IP_Vec3_Value_Linear(float fWeight, std::vector<CBsFXTableValue *> *pVecList, DWORD dwIndex )
{
	static D3DXVECTOR3 vResult;
	D3DXVECTOR3 *pVec[2] = { (D3DXVECTOR3*)(*pVecList)[dwIndex]->GetValue(), (D3DXVECTOR3*)(*pVecList)[dwIndex+1]->GetValue() };

	D3DXVec3Lerp( &vResult, pVec[0], pVec[1], fWeight );

	return &vResult;
}

void *CBsFXTable::IP_Quat_Value_Linear(float fWeight, std::vector<CBsFXTableValue *> *pVecList, DWORD dwIndex )
{
	static D3DXQUATERNION vResult;
	D3DXQUATERNION *pQuat[2] = { (D3DXQUATERNION*)(*pVecList)[dwIndex]->GetValue(), (D3DXQUATERNION*)(*pVecList)[dwIndex+1]->GetValue() };
	D3DXQuaternionSlerp( &vResult, pQuat[0], pQuat[1], fWeight );

	return &vResult;
}

void *CBsFXTable::IP_Vec4_Value_Linear(float fWeight, std::vector<CBsFXTableValue *> *pVecList, DWORD dwIndex )
{
	static D3DXVECTOR4 vResult;
	D3DXVECTOR4 *pvVec4[2] = { (D3DXVECTOR4*)(*pVecList)[dwIndex]->GetValue(), (D3DXVECTOR4*)(*pVecList)[dwIndex+1]->GetValue() };

	D3DXVec4Lerp( &vResult, pvVec4[0], pvVec4[1], fWeight );

	return &vResult;
}

void *CBsFXTable::IP_Vec3ToQuat_Value_Linear(float fWeight, std::vector<CBsFXTableValue *> *pVecList, DWORD dwIndex )
{
	static D3DXQUATERNION Result;
//	D3DXQUATERNION Quat[2];
	D3DXVECTOR3 *pVec[2] = { (D3DXVECTOR3*)(*pVecList)[dwIndex]->GetValue(), (D3DXVECTOR3*)(*pVecList)[dwIndex+1]->GetValue() };

	D3DXVECTOR3 vVec;
	D3DXVec3Lerp( &vVec, pVec[0], pVec[1], fWeight );
//	float fPitch = ( pVec[0]->x + ( pVec[1]->x - pVec[0]->x ) * fWeight ) / 180.f * D3DX_PI;
//	float fYaw = ( pVec[0]->y + ( pVec[1]->y - pVec[0]->y ) * fWeight ) / 180.f * D3DX_PI;
//	float fRoll = ( pVec[0]->z + ( pVec[1]->z - pVec[0]->z ) * fWeight ) / 180.f * D3DX_PI;
	vVec.x = vVec.x / 180.f * D3DX_PI;
	vVec.y = vVec.y / 180.f * D3DX_PI;
	vVec.z = vVec.z / 180.f * D3DX_PI;

	D3DXQuaternionRotationYawPitchRoll( &Result, vVec.x, vVec.y, vVec.z );
//	D3DXQuaternionRotationYawPitchRoll( &Quat[0], pVec[0]->x / 180.f * D3DX_PI, pVec[0]->y / 180.f * D3DX_PI, pVec[0]->z / 180.f * D3DX_PI );
//	D3DXQuaternionRotationYawPitchRoll( &Quat[1], pVec[1]->x / 180.f * D3DX_PI, pVec[1]->y / 180.f * D3DX_PI, pVec[1]->z / 180.f * D3DX_PI );
//	D3DXQuaternionSlerp( &Result, &Quat[0], &Quat[1], fWeight );

	return &Result;
}


void *CBsFXTable::IP_Float_Value_HS(float fWeight, std::vector<CBsFXTableValue *> *pVecList, DWORD dwIndex )
{
	return NULL;
}

void *CBsFXTable::IP_Vec3_Value_HS(float fWeight, std::vector<CBsFXTableValue *> *pVecList, DWORD dwIndex )
{
	static D3DXVECTOR3 vResult;
	D3DXVECTOR3 *pVec[4] = { NULL, (D3DXVECTOR3*)(*pVecList)[dwIndex]->GetValue(), (D3DXVECTOR3*)(*pVecList)[dwIndex+1]->GetValue(), NULL };

	if( (int)dwIndex - 1 >= 0 ) 
		pVec[0] = (D3DXVECTOR3*)(*pVecList)[dwIndex-1]->GetValue();
	else pVec[0] = pVec[1];

	if( dwIndex + 2 < pVecList->size() )
		pVec[3] = (D3DXVECTOR3*)(*pVecList)[dwIndex+2]->GetValue();
	else pVec[3] = pVec[2];

	D3DXVec3CatmullRom( &vResult, pVec[0], pVec[1], pVec[2], pVec[3], fWeight );
//	D3DXVec3Lerp( &vResult, pVec[0], pVec[1], fWeight );

	return &vResult;
}

void *CBsFXTable::IP_Quat_Value_HS(float fWeight, std::vector<CBsFXTableValue *> *pVecList, DWORD dwIndex )
{
	return NULL;
}

void CBsFXTable::operator = ( CBsFXTable &e )
{
	for( DWORD i=0; i<e.GetCount(); i++ ) {
		CBsFXTableValue *pSorValue = e.GetValueFromIndex(i);
		BsAssert( NULL != pSorValue );	// mruete: prefix bug 326: added assert
		CBsFXTableValue *pValue = CBsFXTableValue::CreateThis( pSorValue->GetValueType() );
		BsAssert( NULL != pValue );	// mruete: prefix bug 326: pre-emptive assert added
		pValue->SetKey( pSorValue->GetKey() );
		pValue->SetValue( pSorValue->GetValue() );
		m_pVecTable.push_back( pValue );
	}
	CalcRange();
}