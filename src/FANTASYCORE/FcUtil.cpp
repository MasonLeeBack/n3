#include "StdAfx.h"
#include "FcUtil.h"
#include "bstreamext.h"
//#include "DebugUtil.h"
#include "BsUtil.h"

static unsigned long s_RandomSeed = 1000;
static unsigned long s_RandomSeedforMenu = 2000;


// 크루세이더에 쓰던 랜덤 가져다 쓴다...
// Xenon 과의 호환성을 위해서 c 버전으로 바꿨다..
inline long _delphi_Random( unsigned long* pSeed, unsigned long n )
{
	long lRet;

	*pSeed = *pSeed * 0x08088405 + 1;
	lRet = *pSeed % n;

	return lRet;
}

void SetRandomSeed( unsigned long lSeed )
{
	s_RandomSeed = lSeed;
}

unsigned long GetRandomSeed()
{
	return s_RandomSeed;
}

float RandomNumberInRange( float Min, float Max )
{
	float fRange, fRandom;

	if( Min > Max )
	{
		float fTemp = Min;
		Min = Max;
		Max = fTemp;
	}

	fRandom = Random( RAND_MAX ) / ( float )RAND_MAX;
	fRange = Max - Min;
	fRandom *= fRange;
	fRandom += Min;

	return fRandom;
}

int RandomNumberInRange( int nMin, int nMax )
{
	if( nMin > nMax )
	{
		int nTemp = nMin;
		nMin = nMax;
		nMax = nTemp;
	}

	return Random( nMax - nMin + 1 ) + nMin;
}

int Random( int nRand )
{
	if( nRand == 0 ) return 0;
	return _delphi_Random( &s_RandomSeed, ( unsigned long )nRand );
}

int RandomforMenu( int nRand )
{
	if( nRand == 0 ) return 0;
	return _delphi_Random( &s_RandomSeedforMenu, ( unsigned long )nRand );
}

void RemoveEXT( char* pFileName )
{
	char* pLastStr = NULL;
	char* pTemp = pFileName;
	while( 1 )
	{
		pTemp = strrchr( pTemp, '.' );
		if( pTemp )	{ pLastStr = pTemp; ++pTemp; }
		else        { break; }
	}
	if( pLastStr )
	{
		pLastStr[0] = NULL;
	}
}

void GetWide( const CHAR* strThin, WCHAR* strWide, DWORD dwMax )
{
	BsAssert( strThin != NULL );
	BsAssert( strWide != NULL );

	// dwMax includes the null bytes, so must always be at least one.
	// Furthermore, if dwMax is 0, MultiByteToWideChar doesn't do any 
	// conversion, but returns the number of chars it *would* convert.
	BsAssert( dwMax > 0 );

	// Determine how many characters we will convert. Can never be more
	// than dwMax-1
	int nChars = strlen( strThin );
	if( nChars > int(dwMax) - 1 )
		nChars = int(dwMax) - 1;

	// Perform the conversion
	int nWide = MultiByteToWideChar( CP_ACP, 0, strThin, nChars, 
		strWide, dwMax );
//	BsAssert( nChars == nWide - 1 );
	BsAssert( nChars == nWide );
	strWide[nChars] = NULL;
	(void)nWide; // avoid compiler warning in release mode
}

#define PERP( s, t ) {	\
	t.x = -s.y;	\
	t.y = s.x;	\
}

bool IntLineToLine( D3DXVECTOR3 &vSor1, D3DXVECTOR3 &vSor2, D3DXVECTOR3 &vTar1, D3DXVECTOR3 &vTar2 )
{
	D3DXVECTOR2 a, b, c;
	a.x = vSor2.x - vSor1.x;
	a.y = vSor2.z - vSor1.z;

	b.x = vTar2.x - vTar1.x;
	b.y = vTar2.z - vTar1.z;

	c.x = vTar1.x - vSor1.x;
	c.y = vTar1.z - vSor1.z;

	D3DXVECTOR2 vTemp;
	PERP( a, vTemp );
	float d = D3DXVec2Dot( &c, &vTemp );
	PERP( b, vTemp );
	float e = D3DXVec2Dot( &c, &vTemp );
//	PERP( b, vTemp );
	float f = D3DXVec2Dot( &a, &vTemp );

	float t1, t2;
	if( abs(f) > 0.001f ) {
		t1 = d / f;
		t2 = e / f;

		if( t1 >= 0.f && t1 <= 1.f && t2 >= 0.f && t2 <= 1.f ) return true;
	}
	return false;
}

bool IntLineToLine( D3DXVECTOR2 &vSor1, D3DXVECTOR2 &vSor2, D3DXVECTOR2 &vTar1, D3DXVECTOR2 &vTar2 )
{
	D3DXVECTOR2 a, b, c;
	a = vSor2 - vSor1;
	b = vTar2 - vTar1;
	c = vTar1 - vSor1;

	D3DXVECTOR2 vTemp;
	PERP( a, vTemp );
	float d = D3DXVec2Dot( &c, &vTemp );
	PERP( b, vTemp );
	float e = D3DXVec2Dot( &c, &vTemp );
//	PERP( b, vTemp );
	float f = D3DXVec2Dot( &a, &vTemp );

	float t1, t2;
	if( abs(f) > 0.001f ) {
		t1 = d / f;
		t2 = e / f;

		if( t1 >= 0.f && t1 <= 1.f && t2 >= 0.f && t2 <= 1.f ) return true;
	}
	return false;
}

char *ComposeFormatString(const char *szFormat, ...)
{	
	static char buffer[1024];
	va_list arg;
	va_start(arg, szFormat);
	_vsnprintf( buffer, 1024, szFormat, arg );	
	va_end(arg);

	return buffer;
}

char * ReplaceEXT( const char *pFileName , char *szNewExt)
{
	static char buffer[1024];
	strcpy(buffer, pFileName);

	char *ps = strrchr(buffer, '.');
	if( ps ) *ps = NULL;
	
	strcat(buffer, szNewExt);
	return buffer;
}
