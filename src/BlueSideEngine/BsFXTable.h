#pragma once

class CBsFXTableValue;
class BStream;

class CBsFXTable {
public:
	CBsFXTable();
	virtual ~CBsFXTable();

protected:
	float m_fMinKey;
	float m_fMaxKey;
	std::vector<CBsFXTableValue *> m_pVecTable;

public:
	// Add / Remove Func
	void AddValue( CBsFXTableValue *pValue );
	void RemoveValue( DWORD dwIndex );
	void RemoveValue( CBsFXTableValue *pValue );
	void Reset();
	void Load( BStream *pStream );
	void Save( BStream *pStream );

	// Etc
	DWORD GetCount();
	CBsFXTableValue *GetValueFromIndex( DWORD dwIndex );
	float GetKeyMin();
	float GetKeyMax();
	void CalcRange();

	// Operator
	void operator = ( CBsFXTable &e );

	// Sort
	void Sort( bool (__cdecl *Func)(CBsFXTableValue *, CBsFXTableValue *) );

	// 보간 값 얻기 함수
	// 포인터를 가지구 있는것은 위험!! 바로 처리해주거나..
	// 값 복사해서 쓰는것이 안전해요~
#if defined(_PREFIX_) || defined(_PREFAST_)
	void *GetInterpolation( float fInterpolationKey, void *(__stdcall *Func)(float, std::vector<CBsFXTableValue *> *, DWORD) = NULL );
#else
	void *GetInterpolation( float fInterpolationKey, void *(__cdecl *Func)(float, std::vector<CBsFXTableValue *> *, DWORD) = NULL );
#endif

	// 보간용 콜백함수 기본값들..
	// 직선 보간
	static void *IP_Float_Value_Linear( float fWeight, std::vector<CBsFXTableValue *> *pVecList, DWORD dwIndex );
	static void *IP_Vec3_Value_Linear( float fWeight, std::vector<CBsFXTableValue *> *pVecList, DWORD dwIndex );
	static void *IP_Quat_Value_Linear( float fWeight, std::vector<CBsFXTableValue *> *pVecList, DWORD dwIndex );
	static void *IP_Vec4_Value_Linear( float fWeight, std::vector<CBsFXTableValue *> *pVecList, DWORD dwIndex );
	static void *IP_Vec3ToQuat_Value_Linear( float fWeight, std::vector<CBsFXTableValue *> *pVecList, DWORD dwIndex );
	// 곡선 보간 - 속 비었음.
	static void *IP_Float_Value_HS( float fWeight, std::vector<CBsFXTableValue *> *pVecList, DWORD dwIndex );
	static void *IP_Vec3_Value_HS( float fWeight, std::vector<CBsFXTableValue *> *pVecList, DWORD dwIndex );
	static void *IP_Quat_Value_HS( float fWeight, std::vector<CBsFXTableValue *> *pVecList, DWORD dwIndex );
};