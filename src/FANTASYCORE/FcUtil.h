#pragma once

class BMemoryStream;


void SetRandomSeed( unsigned long lSeed );
unsigned long GetRandomSeed();
float RandomNumberInRange( float Min, float Max );
int RandomNumberInRange( int nMin, int nMax );
int Random( int nRand );
int RandomforMenu( int nRand );

void RemoveEXT( char* pFileName );

void GetWide( const CHAR* strThin, WCHAR* strWide, DWORD dwMax );


bool IntLineToLine( D3DXVECTOR3 &vSor1, D3DXVECTOR3 &vSor2, D3DXVECTOR3 &vTar1, D3DXVECTOR3 &vTar2 );
bool IntLineToLine( D3DXVECTOR2 &vSor1, D3DXVECTOR2 &vSor2, D3DXVECTOR2 &vTar1, D3DXVECTOR2 &vTar2 );

char *ComposeFormatString(const char *szFormat, ...);
char * ReplaceEXT( const char *pFileName , char *szNewExt);	