#include "stdafx.h"
#include "BsSinTable.h"

CBsSinTable::CBsSinTable()
{	
	int i;

	m_pSinTable=new float [NUM_SINTABLE+NUM_SINTABLE/4];
	m_pCosTable=m_pSinTable+NUM_SINTABLE/4;

	for(i=0;i<(NUM_SINTABLE+NUM_SINTABLE/4);i++){
		m_pSinTable[i]=float(sin(PI*i*2/NUM_SINTABLE));
	}
	m_pCosTable[NUM_SINTABLE/4]=0;
}

CBsSinTable::~CBsSinTable()
{
	delete [] m_pSinTable;
}


CBsSinTable g_SinTable;

float *g_pSinTable=g_SinTable.m_pSinTable;
float *g_pCosTable=g_SinTable.m_pCosTable;
