#include "stdafx.h"
#include "BStreamExt.h"
#include "BsFileIO.h"
#include "BsKernel.h"
#include "AsData.h"
#ifndef _USAGE_TOOL_
#include "FcUtil.h"
#endif
#include "data\signalType.h"

#define AS_VERSION	100
#define PARTS_VERSION	100
#define UNIT_INFO_DATA_VERSION	100


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


ASSignalData::ASSignalData()
{
    m_nFrame = -1;
	m_nRow = -1;
	m_nID = -1;
	m_NumParam = 0;
	m_pParam = NULL;
}

ASSignalData::~ASSignalData()
{
	delete[] m_pParam;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////


ASPartsData::ASPartsData()
{
	m_cBoneName[0] = NULL;
	m_nPartsNum = 0;
	m_ppSkinName = NULL;

	m_pSkinIndex = NULL;
}

ASPartsData::~ASPartsData()
{
	Clear();
}

bool ASPartsData::Save( BStream *pStream)
{
	char cReserved[AS_PARTS_DATA_RESERVED];
	
	memset( cReserved, 0, sizeof(cReserved) );	// mruete: prefix bug 512: added memset

	pStream->Write( m_cBoneName, 64 );
	pStream->Write( &m_nPartsNum, sizeof(short), 2 );

	for( int i=0; i<m_nPartsNum; i++ )
		pStream->Write( m_ppSkinName[i], 64 );

	pStream->Write( &m_nType, sizeof(int), 4 );
	pStream->Write( &m_nGroup, sizeof(int), 4 );
	pStream->Write( &m_nSimulation, sizeof( int ), 4 );

	pStream->Write( cReserved, AS_PARTS_DATA_RESERVED );

	return true;
}

bool ASPartsData::Load( BStream *pStream)
{
	int i;

	pStream->Read( m_cBoneName, 64 );
	pStream->Read(&m_nPartsNum, sizeof(short), 2);

	m_ppSkinName = new char*[m_nPartsNum];
	for( i=0; i<m_nPartsNum; i++ )
		m_ppSkinName[i] = new char[64];

	for( i=0; i<m_nPartsNum; i++ )
		pStream->Read(m_ppSkinName[i], 64);

	pStream->Read( &m_nType, sizeof(int), 4 );
	pStream->Read( &m_nGroup, sizeof(int), 4 );
	pStream->Read( &m_nSimulation, sizeof( int ), 4 );

	pStream->Seek(AS_PARTS_DATA_RESERVED, BStream::fromNow);

	m_pSkinIndex = new int[m_nPartsNum];
	for( int i=0; i<m_nPartsNum; i++ )
	{
		m_pSkinIndex[i] = -1;
	}

	return true;
}

void ASPartsData::SetData(std::vector<std::string>* pPartsVec)
{
	Clear();

	m_nPartsNum = (short)pPartsVec->size();

	m_ppSkinName = new char*[m_nPartsNum];
	for( int i=0; i<m_nPartsNum; i++ )
	{
		m_ppSkinName[i] = new char[64];
		const char* pStr = (*pPartsVec)[i].c_str();
		BsAssert( strlen(pStr)<64 );
		strcpy_s( m_ppSkinName[i], 64, pStr ); //aleksger - safe string
	}
}


void ASPartsData::Clear()
{
	if(m_ppSkinName){
		for( int i=0; i<m_nPartsNum; i++ ){
			if(m_ppSkinName[i]){
				delete[] m_ppSkinName[i];
			}
		}
	}

	m_nPartsNum = 0;
	delete[] m_ppSkinName;
	m_ppSkinName = NULL;

	delete[] m_pSkinIndex;
	m_pSkinIndex = NULL;
}






//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////



ASAniData::ASAniData()
{
	m_nAniType = -1;
	m_nAniIndex = -1;
}

ASAniData::~ASAniData()
{
}


bool ASAniData::Save( BStream *pStream )
{
	char cReserved[AS_ANI_DATA_RESERVED];

	memset(cReserved, 0, AS_ANI_DATA_RESERVED);
	pStream->Write(&m_nAniType, sizeof(int), 4);
	pStream->Write(&m_nAniIndex, sizeof(int), 4);
	pStream->Write(&m_nNextAni, sizeof(int), 4);
	pStream->Write(&m_nBlendFrame, sizeof(int), 4);
	pStream->Write(&m_nMoveSpeed, sizeof(int), 4);
	
	pStream->Write( &m_nAniLength, sizeof( int ), 4 );
	pStream->Write( &m_nAniAttr, sizeof( int ), 4 );
	pStream->Write(cReserved, AS_ANI_DATA_RESERVED);
	return true;
}

bool ASAniData::Load( BStream *pStream )
{
	pStream->Read(&m_nAniType, sizeof(int), 4);
	pStream->Read(&m_nAniIndex, sizeof(int), 4);

	pStream->Read(&m_nNextAni, sizeof(int), 4);
	pStream->Read(&m_nBlendFrame, sizeof(int), 4);
	pStream->Read(&m_nMoveSpeed, sizeof(int), 4);

	pStream->Read( &m_nAniLength, sizeof( int ), 4 );
	pStream->Read( &m_nAniAttr, sizeof( int ), 4 );
	pStream->Seek(AS_ANI_DATA_RESERVED, BStream::fromNow);

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<ASDATA_CONTAINER *> ASData::s_ASDataContainer;

ASData::ASData()
{
	m_nAniNum = 0;
	m_pAniSignal = NULL;

	m_nUseParticleCount = 0;
	m_pUseParticleList = NULL;
	m_nUseFXCount = 0;
	m_pUseFXList = NULL;
}

ASData::~ASData()
{
	Clear();
}

void ASData::Clear()
{
	if( m_pAniSignal )
	{
		delete [] m_pAniSignal;
		m_pAniSignal = NULL;
	}
	if( m_pUseParticleList )
	{
		delete [] m_pUseParticleList;
	}
	if( m_pUseFXList )
	{
		delete [] m_pUseFXList;
	}

	m_HitMatrixList.erase( m_HitMatrixList.begin(), m_HitMatrixList.end() );
}

bool ASData::LoadSignal( const char *pFileName )
{
	int i, nVersion;
	DWORD dwFileSize;
	VOID *pData;

	if( FAILED(CBsFileIO::LoadFile( pFileName, &pData, &dwFileSize ) ) )
	{
		BsAssert( 0 && "File open error" );
		return false;
	}
	Clear();

	BMemoryStream Stream(pData, dwFileSize);
	Stream.Read( &nVersion, sizeof( int ), 4 );
	Stream.Read( &m_nAniNum, sizeof( int ), 4 );
	Stream.Seek( AS_DATA_RESERVED, BStream::fromNow );

	m_pAniSignal = new CAniSignal[ m_nAniNum ];
	for( i = 0; i < m_nAniNum; i++ ) 
	{
		m_pAniSignal[ i ].LoadSignal( &Stream );
	}

	int nHitMatrixCount;
	D3DXMATRIX Matrix;

	Stream.Read( &nHitMatrixCount, sizeof( int ), 4 );
	for( i = 0; i < nHitMatrixCount; i++ )
	{
		Stream.Read( &Matrix, sizeof( D3DXMATRIX ), 4 );
		m_HitMatrixList.push_back( Matrix );
	}

	if( nVersion >= 101 )
	{
		Stream.Read( &m_nUseParticleCount, sizeof( int ), 4 );
		if( m_nUseParticleCount )
		{
			m_pUseParticleList = new int[ m_nUseParticleCount ];
		}
		Stream.Read( m_pUseParticleList, sizeof( int ) * m_nUseParticleCount, 4 );

		Stream.Read( &m_nUseFXCount, sizeof( int ), 4 );
		if( m_nUseFXCount )
		{
			m_pUseFXList = new int[ m_nUseFXCount ];
		}
		Stream.Read( m_pUseFXList, sizeof( int ) * m_nUseFXCount, 4 );
	}

	CBsFileIO::FreeBuffer(pData);
	return true;
}

bool ASData::SaveSignal( const char *pFileName )
{
	int i;
	char cReserved[ AS_DATA_RESERVED ];

	BFileStream Stream( pFileName, BFileStream::create );

//	Clear();

	int nVersion = 101;
	Stream.Write( &nVersion, sizeof( int ), 4 );
	Stream.Write( &m_nAniNum, sizeof( int ), 4 );
	memset( cReserved, 0, AS_DATA_RESERVED );
	Stream.Write( cReserved, AS_DATA_RESERVED );

	for( i = 0; i < m_nAniNum; i++ ) 
	{
		m_pAniSignal[ i ].SaveSignal( &Stream );
	}

#ifdef _USAGE_TOOL_
	CalculateHitMatrix();
#endif // #ifdef _USAGE_TOOL_
	int nHitMatrixCount;

	nHitMatrixCount = m_HitMatrixList.size();
	Stream.Write( &nHitMatrixCount, sizeof( int ), 4 );
	for( i = 0; i < nHitMatrixCount; i++ )
	{
		Stream.Write( &( m_HitMatrixList[ i ] ), sizeof( D3DXMATRIX ), 4 );
	}

// Use Partice & FX Search
	int nUseTable[ 1000 ], nUseCount;

	memset( nUseTable, 0, sizeof( int ) * 1000 );
	for( i = 0; i < m_nAniNum; i++ ) 
	{
		m_pAniSignal[ i ].CheckUseParticle( nUseTable );
	}
	nUseCount = 0;
	for( i = 0; i < 1000; i++ ) 
	{
		if( nUseTable[ i ] )
		{
			nUseCount++;
		}
	}
	Stream.Write( &nUseCount, sizeof( int ), 4 );
	for( i = 0; i < 1000; i++ )
	{
		if( nUseTable[ i ] )
		{
			Stream.Write( &i, sizeof( int ), 4 );
		}
	}

	memset( nUseTable, 0, sizeof( int ) * 1000 );
	for( i = 0; i < m_nAniNum; i++ ) 
	{
		m_pAniSignal[ i ].CheckUseFX( nUseTable );
	}
	nUseCount = 0;
	for( i = 0; i < 1000; i++ ) 
	{
		if( nUseTable[ i ] )
		{
			nUseCount++;
		}
	}
	Stream.Write( &nUseCount, sizeof( int ), 4 );
	for( i = 0; i < 1000; i++ )
	{
		if( nUseTable[ i ] )
		{
			Stream.Write( &i, sizeof( int ), 4 );
		}
	}

	return true;
}

void ASData::AdjustSignalSize( int nSignalID, int nSize )
{
	int i;

	for( i = 0; i < m_nAniNum; i++ ) 
	{
		m_pAniSignal[ i ].AdjustSignalSize( nSignalID, nSize );
	}
}

#ifdef _USAGE_TOOL_
#include "BsAtBridge.h"

void ASData::CalculateHitMatrix()
{
	int i, j, k, nSignalCount;
	CAniSignal *pAniData;
	ASSignalData *pSignalData;
	D3DXMATRIX BoneMat;
	D3DXVECTOR3 vPos;

	m_HitMatrixList.erase(m_HitMatrixList.begin(), m_HitMatrixList.end());
	for( i = 0; i < m_nAniNum; i++ )
	{
		pAniData = GetSignalList( i );
		nSignalCount = pAniData->GetSignalCount();
		for(j = 0; j < nSignalCount; j++ )
		{
			pSignalData = pAniData->GetSignal( j );
			if( pSignalData->m_nID == SIGNAL_TYPE_HIT )
			{		
				pSignalData->m_HitMatrixIndex = ( int )m_HitMatrixList.size();
				for( k = 0; k < pSignalData->m_pParam[ 0 ]; k++ )
				{
					vPos = CBsAtBridge::GetInstance().GetAniDistance( i, i, ( float )( pSignalData->m_nFrame + k ), 0.0f );
					// 임시로 Dummy_WP_R 로 한다.. 다른데 링크된 무기 나올텐데.. 그거 나오면 고치자..
					if( CBsAtBridge::GetInstance().GetBoneMatrix( &BoneMat, "Dummy_WP_R", i, pSignalData->m_nFrame + k ) )
					{
/*						BoneMat._41 += vPos.x;
						BoneMat._42 += vPos.y;
						BoneMat._43 += vPos.z;*/
						m_HitMatrixList.push_back( BoneMat );
					}
				}
			}
			else if( pSignalData->m_nID == SIGNAL_TYPE_SHOW_TRAIL )
			{
				pSignalData->m_TrailMatrixIndex = ( int )m_HitMatrixList.size();
				for( k = 0; k < pSignalData->m_pParam[ 0 ]; k++ )
				{
					vPos = CBsAtBridge::GetInstance().GetAniDistance( i, i, ( float )( pSignalData->m_nFrame + k ), 0.0f );
					if( pSignalData->m_TrailWeaponBone == 0 )
					{
						CBsAtBridge::GetInstance().GetBoneMatrix( &BoneMat, "Dummy_WP_R", i, pSignalData->m_nFrame + k );
					}
					else
					{
						CBsAtBridge::GetInstance().GetBoneMatrix( &BoneMat, pSignalData->m_TrailWeaponBone, i, pSignalData->m_nFrame + k );
					}
					m_HitMatrixList.push_back( BoneMat );
				}
			}
			else if( pSignalData->m_nID == SIGNAL_TYPE_THROW_WEAPON )
			{


				pSignalData->m_ThrowWeaponMatrixIndex = ( int )m_HitMatrixList.size();
				for( k = 0; k < pSignalData->m_ThrowWeaponLength; k++ )
				{
					vPos = CBsAtBridge::GetInstance().GetAniDistance( i, i, ( float )( pSignalData->m_nFrame + k ), 0.0f );

					if( CBsAtBridge::GetInstance().GetBoneMatrix( &BoneMat, pSignalData->m_ThrowWeaponStartBone, i, pSignalData->m_nFrame + k ) == true )
					{
/*						BoneMat._41 += vPos.x;
						BoneMat._42 += vPos.y;
						BoneMat._43 += vPos.z;*/
						m_HitMatrixList.push_back( BoneMat );
					}
					if( CBsAtBridge::GetInstance().GetBoneMatrix( &BoneMat, pSignalData->m_ThrowWeaponEndBone, i, pSignalData->m_nFrame + k ) == true )
					{
/*						BoneMat._41 += vPos.x;
						BoneMat._42 += vPos.y;
						BoneMat._43 += vPos.z;*/
						m_HitMatrixList.push_back( BoneMat );
					}
				}
			}
			
		}
	}
}
#endif // #ifdef _USAGE_TOOL_

void ASData::AdjustSignal( ASData* pASData )		// 시그널 겹친 녀석들 안 겹치게
{
	for( int i=0; i<pASData->GetAniNum(); i++ )
	{
		CAniSignal *pData = pASData->GetSignalList( i );
		while( 1 )
		{
			bool bAgain = false;
			for( int j=0; j<pData->GetSignalCount(); j++ )
			{
				ASSignalData* pSignal = pData->GetSignal( j );
				bool bDone = true;
				for( int w=0; w<pData->GetSignalCount(); w++ )
				{
					if( j == w )
					{
						continue;
					}

					ASSignalData* pTempSignal = pData->GetSignal( w );
					if( pSignal->m_nRow == pTempSignal->m_nRow && pSignal->m_nFrame == pTempSignal->m_nFrame )
					{
						if( pSignal->m_nRow < AS_SIGNAL_ROW_MAX -1 )
							pSignal->m_nRow++;
						else
							pSignal->m_nFrame++;
						bDone = false;
						break;
					}
				}
				if( bDone == false )
				{
					bAgain = true;
					break;
				}
			}
			if( bAgain == false )
			{
				break;
			}
		}
	}
}

/*
int ASData::GetAniNum()
{
	m_pAniSignal->
}
*/

ASData* ASData::LoadASData( const char* pFileName, int* pIndex )
{
	unsigned int i;
	char *pFullName;

	for(i=0;i<s_ASDataContainer.size();i++){
		if(_stricmp(s_ASDataContainer[i]->pFileName, pFileName)==0){
			if( pIndex ) *pIndex = i;
			return s_ASDataContainer[i]->pASData;
		}
	}
	ASDATA_CONTAINER *pContainer;
	ASData *pASData;

	pContainer=new ASDATA_CONTAINER;
	memset(pContainer, 0, sizeof(ASDATA_CONTAINER));
	const size_t pFileName_len = strlen(pFileName)+1;
	pContainer->pFileName=new char[pFileName_len];
	strcpy_s(pContainer->pFileName, pFileName_len, pFileName);//aleksger - safe string

	pFullName=g_BsKernel.GetFullName(pFileName);

#ifdef ENABLE_RES_CHECKER
	g_BsResChecker.AddResList( pFullName );
#endif

	pASData = new ASData();
	pASData->LoadSignal( pFullName );

#ifdef _USAGE_TOOL_
	// 겹친 녀석 안겹치게
	AdjustSignal( pASData );
#endif

	pContainer->pASData = pASData;
	s_ASDataContainer.push_back(pContainer);

	if( pIndex )
		*pIndex = (s_ASDataContainer.size() - 1);

	return pContainer->pASData;
}

bool ASData::SaveASData( const char* pFileName )
{
	unsigned int i;
	char *pFullName;

	for(i=0;i<s_ASDataContainer.size();i++){
		if(_stricmp(s_ASDataContainer[i]->pFileName, pFileName)==0){
			pFullName=g_BsKernel.GetFullName(pFileName);
			s_ASDataContainer[i]->pASData->SaveSignal( pFileName );
			return true;
		}
	}
	BsAssert(0);
	return false;
}

void ASData::DeleteASData(int nIndex)
{
	delete [] s_ASDataContainer[nIndex]->pFileName;
	delete s_ASDataContainer[nIndex]->pASData;
	delete s_ASDataContainer[nIndex];
}


void ASData::DeleteASData(const char *pFileName)
{
	unsigned int i;
	for(i=0;i<s_ASDataContainer.size();i++){
		if(_stricmp(s_ASDataContainer[i]->pFileName, pFileName)==0){
			DeleteASData(i);
			s_ASDataContainer.erase(s_ASDataContainer.begin()+i);
		}
	}
}

void ASData::DeleteASData(ASData *pData)
{
	unsigned int i;

	for(i=0;i<s_ASDataContainer.size();i++){
		if(s_ASDataContainer[i]->pASData==pData){
			DeleteASData(i);
			s_ASDataContainer.erase(s_ASDataContainer.begin()+i);
			break;
		}
	}
}

void ASData::DeleteAllASData()
{
	unsigned int i;

	for(i=0;i<s_ASDataContainer.size();i++){
		DeleteASData(i);
	}
	s_ASDataContainer.erase(s_ASDataContainer.begin(), s_ASDataContainer.end());
}

ASData* ASData::GetData( int nIndex )
{
	BsAssert( nIndex >= 0 && nIndex < (int)s_ASDataContainer.size() );
	return s_ASDataContainer[nIndex]->pASData;
}



///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

CAniSignal::CAniSignal()
{
	m_pSignal = NULL;
}

CAniSignal::~CAniSignal()
{
	if( m_pSignal )
	{
		delete [] m_pSignal;
	}
}

bool CAniSignal::LoadSignal( BStream *pStream )
{
	pStream->Read( &m_nSignalNum, sizeof( int ), 4 );

	if( m_nSignalNum > 0 && m_nSignalNum != 65536 )
	{
		int nInvalidSignal = 0;
		m_pSignal = new ASSignalData[ m_nSignalNum ];


		int nCurOffset = pStream->Tell();
		int w = 0;
		for( int i = 0; i < m_nSignalNum; i++ )
		{
			pStream->Seek( nCurOffset, BStream::fromBegin );

			pStream->Read(&(m_pSignal[w].m_nID), sizeof(int), 4);
			int nOffset;
			pStream->Read(&nOffset, sizeof(int), 4);
			nCurOffset = pStream->Tell();
			pStream->Seek( nOffset, BStream::fromBegin );

			pStream->Read(&(m_pSignal[w].m_nFrame), sizeof(int), 4);
			pStream->Read(&(m_pSignal[w].m_nRow), sizeof(int), 4);

			pStream->Read(&(m_pSignal[w].m_NumParam), sizeof(int), 4);

			BsAssert(m_pSignal[w].m_NumParam>=0 && m_pSignal[w].m_NumParam<32 );
			if( m_pSignal[w].m_NumParam ) {
				m_pSignal[w].m_pParam = new int[m_pSignal[w].m_NumParam];
				pStream->Read(m_pSignal[w].m_pParam, sizeof(int)*m_pSignal[w].m_NumParam, 4);
			}
			else m_pSignal[w].m_pParam = NULL;
			pStream->Seek(AS_SIGNAL_DATA_RESERVED, BStream::fromNow);


			if( m_pSignal[w].m_nFrame < 0 || m_pSignal[w].m_nRow < 0 || m_pSignal[w].m_nRow >= AS_SIGNAL_ROW_MAX )
			{
				++nInvalidSignal;
				continue;
			}

			++w;
		}
		m_nSignalNum -= nInvalidSignal;
	}
	else
	{
		m_pSignal = NULL;
	}

	return true;
}




struct SortSignal
{
	bool	operator()( ASSignalData* pSignal1, ASSignalData* pSignal2 )
	{
		if( pSignal1->m_nFrame < pSignal2->m_nFrame )
		{
			return true;
		}
		else if( pSignal1->m_nFrame == pSignal2->m_nFrame )
		{
			if( pSignal1->m_nRow < pSignal2->m_nRow )
				return true;
			else
				return false;
		}
		else
		{
			return false;
		}
	}
};



bool CAniSignal::SaveSignal( BStream *pStream )
{
	char cReserved[AS_DATA_RESERVED];


	std::vector<ASSignalData*> vecSignal;
	for( int i=0; i<m_nSignalNum; i++ )
	{
		vecSignal.push_back( &(m_pSignal[i]) );
	}

	SortSignal sortSignalFunc;
	std::sort( vecSignal.begin(), vecSignal.end(), sortSignalFunc );



	pStream->Write(&m_nSignalNum, sizeof(int), 4);

	int nSignalOffset = (int)(pStream->Tell());

	int nCnt = vecSignal.size();
	for( int i = 0; i < nCnt; i++ )
	{
		ASSignalData* pSignal = vecSignal[i];
		pStream->Write(&(pSignal->m_nID), sizeof(int), 4);
		int nTemp = 0;	// offset
		pStream->Write(&nTemp, sizeof(int), 4);
	}
	for( int i = 0; i < nCnt; i++ )
	{
		ASSignalData* pSignal = vecSignal[i];

		int nCurOffset = (int)(pStream->Tell());
		pStream->Seek( nSignalOffset+i*sizeof(int)*2 + sizeof(int), BStream::fromBegin );
		pStream->Write( &nCurOffset, sizeof(int), 4 );

		pStream->Seek( nCurOffset, BStream::fromBegin );
		pStream->Write(&(pSignal->m_nFrame), sizeof(int), 4);
		pStream->Write(&(pSignal->m_nRow), sizeof(int), 4);

		int numParam = pSignal->m_NumParam;
		pStream->Write( &numParam, sizeof(int), 4 );
		//		pStream->Write(m_pSignal[i].m_Param, sizeof(int)*NUM_UNIT_SIGNAL_PARAM, 4);
		pStream->Write(pSignal->m_pParam, sizeof(int)*numParam, 4);
		pStream->Write(cReserved, AS_SIGNAL_DATA_RESERVED);
	}

	return true;
}

void CAniSignal::AdjustSignalSize( int nSignalID, int nSize )
{
	int i, *pParam;

	for( i = 0; i < m_nSignalNum; i++ )
	{
		if( m_pSignal[ i ].m_nID == nSignalID )
		{
			if( m_pSignal[ i ].m_NumParam < nSize )
			{
				pParam = new int[ nSize ];
				memset( pParam, 0, sizeof( int ) * nSize );
				memcpy( pParam, m_pSignal[ i ].m_pParam, sizeof( int ) * m_pSignal[ i ].m_NumParam );
				delete [] m_pSignal[ i ].m_pParam;
				m_pSignal[ i ].m_pParam = pParam;
				m_pSignal[ i ].m_NumParam = nSize;
			}
		}
	}
}

void CAniSignal::CheckUseParticle( int *pUseCheck )
{
	int i, j;

	for( i = 0; i < m_nSignalNum; i++ )
	{
		switch( m_pSignal[ i ].m_nID )
		{
		case SIGNAL_TYPE_HIT:
			pUseCheck[ m_pSignal[ i ].m_HitParticle ] = 1;
			pUseCheck[ m_pSignal[ i ].m_HitParticle + 1 ] = 1;
			pUseCheck[ m_pSignal[ i ].m_HitParticle + 2 ] = 1;
			break;
		case SIGNAL_TYPE_HIT_TEMP:
			pUseCheck[ m_pSignal[ i ].m_Hit2Particle ] = 1;
			pUseCheck[ m_pSignal[ i ].m_Hit2Particle + 1 ] = 1;
			pUseCheck[ m_pSignal[ i ].m_Hit2Particle + 2 ] = 1;
			break;
		case SIGNAL_TYPE_HIT_PUSH:
			pUseCheck[ m_pSignal[ i ].m_PushParticle ] = 1;
			pUseCheck[ m_pSignal[ i ].m_PushParticle + 1 ] = 1;
			pUseCheck[ m_pSignal[ i ].m_PushParticle + 2 ] = 1;
			break;
		case SIGNAL_TYPE_CHECK_PHYSICS:
			pUseCheck[ m_pSignal[ i ].m_PhysicsParticle ] = 1;
			break;
		case SIGNAL_TYPE_PARTICLE:
			for( j = 0; j < m_pSignal[ i ].m_ParticleCount; j++ )
			{
				pUseCheck[ m_pSignal[ i ].m_ParticleID + j ] = 1;
			}
			break;
		case SIGNAL_TYPE_LINKED_PARTICLE:
			pUseCheck[ m_pSignal[ i ].m_LinkedParticleID ] = 1;
			break;
		case SIGNAL_TYPE_LINKED_TO_CROSS_PARTICLE:
			pUseCheck[ m_pSignal[ i ].m_LinkedToCrossParticleID ] = 1;
			break;
		case SIGNAL_TYPE_SHOOT_PROJECTILE:
			pUseCheck[ m_pSignal[ i ].m_pParam[ 5 ] ] = 1;
			break;
		}
	}
}

void CAniSignal::CheckUseFX( int *pUseCheck )
{
	int i;

	for( i = 0; i < m_nSignalNum; i++ )
	{
		switch( m_pSignal[ i ].m_nID )
		{
		case SIGNAL_TYPE_FX:
			pUseCheck[ m_pSignal[ i ].m_pParam[ 0 ] ] = 1;
			break;
		case SIGNAL_TYPE_FX_LOCAL:
			pUseCheck[ m_pSignal[ i ].m_pParam[ 0 ] ] = 1;
			break;
		case SIGNAL_TYPE_LINKED_FX:
			pUseCheck[ m_pSignal[ i ].m_pParam[ 1 ] ] = 1;
			break;
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

std::vector< ANI_INFO_CONTAINER * > CAniInfoData::s_AniInfoDataContainer;

CAniInfoData::CAniInfoData()
{
	m_pAni = NULL;
	m_pPartsData = NULL;
}

CAniInfoData::~CAniInfoData()
{
	if( m_pAni )
	{
		delete [] m_pAni;
	}

	if( m_pPartsData )
	{
		delete [] m_pPartsData;
	}
}

bool CAniInfoData::LoadData( const char *pFileName )
{
	int i, nVersion;
	char cReserved[ UNIT_INFO_DATA_RESERVED ];
	DWORD dwFileSize;
	VOID *pData;

	if( FAILED(CBsFileIO::LoadFile( pFileName, &pData, &dwFileSize ) ) )
	{
		CBsConsole::GetInstance().AddFormatString( "%s File Not Found!!!", pFileName );
		return false;
	}

	BMemoryStream Stream(pData, dwFileSize);
	Stream.Read( &nVersion, sizeof( int ), 4 );
	if( nVersion != UNIT_INFO_DATA_VERSION ){
		CBsConsole::GetInstance().AddFormatString( "Invalid Unit Info Data Version" );
// [PREFIX:beginmodify] 2006/2/15 junyash PS#5191 reports memory leak 'pData'
		CBsFileIO::FreeBuffer(pData);
// [PREFIX:endmodify] junyash
		return false;
	}

	Stream.Read( &m_nUnitID, sizeof( int ), 4 );
	Stream.Read( &m_nAniNum, sizeof(int), 4 );
	Stream.Read( cReserved, UNIT_INFO_DATA_RESERVED );
	m_pAni = new ASAniData[ m_nAniNum ];
	for( i = 0; i < m_nAniNum; i++ )
	{
		m_pAni[i].Load( &Stream );
	}

	// 에니메이션 갯수 계산.
	memset( m_nAniTypeCount, 0, sizeof( int ) * ANI_TYPE_NUM * ANI_ATTR_NUM );
	int nAttr;
	for( i = 0; i < m_nAniNum; i++ )
	{
		switch( m_pAni[ i ].GetAniAttr() ) {
				case ANI_ATTR_NONE:		nAttr = 0;	break;
				case ANI_ATTR_BATTLE:	nAttr = 1;	break;
				case ANI_ATTR_BACK:		nAttr = 2;	break;
				case ANI_ATTR_HORSE:	nAttr = 3;	break;
		}
		m_nAniTypeCount[ nAttr ][ m_pAni[ i ].GetAniType() ]++;
	}


	Stream.Read( &m_nPartsGroupNum, sizeof( int ), 4 );

	if( m_nPartsGroupNum > 0 )
	{
		m_pPartsData = new ASPartsData[ m_nPartsGroupNum ];
		for( i = 0; i < m_nPartsGroupNum; i++ )
		{
			m_pPartsData[ i ].Load( &Stream );
		}
	}

	CBsFileIO::FreeBuffer(pData);
	return true;
}

bool CAniInfoData::SaveData( const char *pFileName )
{
	int i, nVersion;
	char cReserved[ UNIT_INFO_DATA_RESERVED ];
	BBufStream Stream;
//	BFileStream Stream( pFileName, BFileStream::create );

	nVersion = UNIT_INFO_DATA_VERSION;
	Stream.Write( &nVersion, sizeof( int ), 4 );

	Stream.Write( &m_nUnitID, sizeof( int ), 4 );
	Stream.Write( &m_nAniNum, sizeof(int), 4 );
	memset( cReserved, 0, UNIT_INFO_DATA_RESERVED );
	Stream.Write( cReserved, UNIT_INFO_DATA_RESERVED );
	for( i = 0; i < m_nAniNum; i++ )
	{
		m_pAni[i].Save( &Stream );
	}

if( strstr( pFileName, "C_DS_GB1_OH" ) != NULL )
{
	if( m_nPartsGroupNum != 2 )
	{
		{
			BsAssert( 0 && "머냐이건!!" );
		}
	}
}


	Stream.Write( &m_nPartsGroupNum, sizeof(int), 4 );
	for( i = 0; i < m_nPartsGroupNum; i++ )
	{
		m_pPartsData[ i ].Save( &Stream );
	}

	BFileStream FStream( pFileName, BFileStream::create );
	if( FStream.Valid() == false )
	{
//		AfxMessageBox( "저장 실패" );
		CBsConsole::GetInstance().AddFormatString( "%s File Create Failed!!!", pFileName );
		return false;
	}

	int nLength = Stream.Length();
	char* pTempBuf = new char[nLength];
	Stream.Read( pTempBuf, nLength, 0 );
	FStream.Write( pTempBuf, nLength, 0 );
	delete[] pTempBuf;

	return true;
}

int CAniInfoData::GetAniAttr( int nAniIndex )
{
	BsAssert( nAniIndex < m_nAniNum );

	return m_pAni[ nAniIndex ].GetAniAttr();
}

int CAniInfoData::GetAniType(int nAniIndex)
{
	BsAssert(nAniIndex<m_nAniNum);

	return m_pAni[nAniIndex].GetAniType();
}

int CAniInfoData::GetAniTypeIndex(int nAniIndex)
{
	BsAssert(nAniIndex<m_nAniNum);

	return m_pAni[nAniIndex].GetAniIndex();
}

int CAniInfoData::FindSimilarAniIndex( int nType, int nAttr )
{
	int i, nFind[ 100 ], nFindCount;

	nFindCount = 0;
	for( i = 0; i < m_nAniNum; i++ )
	{
		if( ( m_pAni[ i ].GetAniType() == nType ) && ( m_pAni[ i ].GetAniAttr() == nAttr ) )
		{
			BsAssert( nFindCount < 100 );

			nFind[ nFindCount ] = i;
			nFindCount++;
		}
	}
	if( nFindCount )
	{
#ifndef _USAGE_TOOL_
		return nFind[ Random( nFindCount ) ];
#else
		return nFind[ 0 ];
#endif
	}
	else
	{
		return -1;
	}
}

int CAniInfoData::GetAniIndex( int nType, int nIndex, int nAttr /*= 0*/ )
{
	int i;

	if( nIndex == -1 )
	{
		int nRet;

		nRet = FindSimilarAniIndex( nType, nAttr );
		if( nRet != -1 )
		{
			return nRet;
		}
		nIndex = 0;
	}

	for( i = 0; i < m_nAniNum; i++ )
	{
		if ( m_pAni[ i ].GetAniType() == nType ) 
		{
			if( ( m_pAni[ i ].GetAniAttr() == nAttr ) && ( m_pAni[ i ].GetAniIndex() == nIndex ) )
			{
				return i;
			}
		}
	}

	int nSameAttrIndex = -1;

	for( i = 0; i < m_nAniNum; i++ )
	{
		if ( m_pAni[ i ].GetAniType() == nType )
		{
			if( ( m_pAni[ i ].GetAniIndex() == nIndex ) || ( m_pAni[ i ].GetAniIndex() == 0 ) )
			{
				return i;
			}
			else if( m_pAni[i].GetAniAttr() == nAttr )
			{
				if( nSameAttrIndex != -1 )
				{
					nSameAttrIndex = i;
				}
			}
		}
	}
	if( nSameAttrIndex != -1 ) 
	{
		return nSameAttrIndex;
	}

//	BsAssert( 0 && "애니메이션 없음" );
	return -1;
}

int CAniInfoData::GetBlendFrame(int nAniIndex)
{
	BsAssert(nAniIndex<m_nAniNum);

	return m_pAni[nAniIndex].GetBlendFrame();
}

int CAniInfoData::GetNextAni(int nAniIndex)
{
	BsAssert(nAniIndex<m_nAniNum);

	return m_pAni[nAniIndex].GetNextAni();
}

int CAniInfoData::GetAniTypeCount( int nAniAttr, int nAniType )
{
	int nAttr;
	switch( nAniAttr ) {
		case ANI_ATTR_NONE:		nAttr = 0;	break;
		case ANI_ATTR_BATTLE:	nAttr = 1;	break;
		case ANI_ATTR_BACK:		nAttr = 2;	break;
		case ANI_ATTR_HORSE:	nAttr = 3;	break;
	}

	return m_nAniTypeCount[nAttr][nAniType]; 
}




void CAniInfoData::SetAniNum( int nNum )
{
	m_nAniNum = nNum;
	BsAssert( m_nAniNum >= 0 );
	BsAssert( m_pAni == NULL );
	if( m_nAniNum > 0 )
	{
		m_pAni = new ASAniData[m_nAniNum];
//		memset( m_pAni, 0, sizeof( ASAniData ) * m_nAniNum );
	}
}

CAniInfoData *CAniInfoData::LoadAniInfoData( const char* pFileName, int* pIndex )
{
	unsigned int i;
	char *pFullName;

	for( i = 0; i < s_AniInfoDataContainer.size(); i++ )
	{
		if( _stricmp( s_AniInfoDataContainer[ i ]->pFileName, pFileName ) == 0 )
		{
			if( pIndex ) *pIndex = i;
			return s_AniInfoDataContainer[ i ]->pAniInfoData;
		}
	}

	ANI_INFO_CONTAINER *pContainer;
	CAniInfoData *pAniInfoData;

	pContainer = new ANI_INFO_CONTAINER;
	memset( pContainer, 0, sizeof( ASDATA_CONTAINER ) );
	const size_t pFileName_len = strlen(pFileName)+1;//aleksger - safe string
	pContainer->pFileName = new char[pFileName_len ];
	strcpy_s( pContainer->pFileName, pFileName_len, pFileName ); 

	pFullName = g_BsKernel.GetFullName( pFileName );

#ifdef ENABLE_RES_CHECKER
	g_BsResChecker.AddResList( pFullName );
#endif

	pAniInfoData = new CAniInfoData();
	if( pAniInfoData->LoadData( pFullName ) == true )
	{
		pContainer->pAniInfoData = pAniInfoData;
	}
	else
	{
		pContainer->pAniInfoData = NULL;
		delete pAniInfoData;
		delete[] pContainer->pFileName;
		delete pContainer;
		return NULL;
	}

	if( pContainer->pAniInfoData != NULL )
		s_AniInfoDataContainer.push_back( pContainer );

	if( pIndex )
		*pIndex = (s_AniInfoDataContainer.size() - 1);

	return pContainer->pAniInfoData;
}


bool CAniInfoData::SaveAniInfoData( const char* pFileName )
{
	unsigned int i;
	char *pFullName;

	for( i = 0; i < s_AniInfoDataContainer.size(); i++ )
	{
		if( _stricmp( s_AniInfoDataContainer[ i ]->pFileName, pFileName ) == 0 )
		{
			pFullName = g_BsKernel.GetFullName( pFileName );
			s_AniInfoDataContainer[ i ]->pAniInfoData->SaveData( pFullName );
			return true;
		}
	}
	BsAssert( 0 );
	return false;
}


void CAniInfoData::DeleteAniInfoData( int nIndex )
{
	delete [] s_AniInfoDataContainer[ nIndex ]->pFileName;
	delete s_AniInfoDataContainer[ nIndex ]->pAniInfoData;
	delete s_AniInfoDataContainer[ nIndex ];
}

void CAniInfoData::DeleteAniInfoData( const char *pFileName )
{
	unsigned int i;

	for( i = 0; i < s_AniInfoDataContainer.size(); i++ )
	{
		if( _stricmp( s_AniInfoDataContainer[ i ]->pFileName, pFileName ) == 0 )
		{
			DeleteAniInfoData( i );
			s_AniInfoDataContainer.erase( s_AniInfoDataContainer.begin() + i );
		}
	}
}

void CAniInfoData::DeleteAniInfoData( CAniInfoData *pData )
{
	unsigned int i;

	for( i = 0; i < s_AniInfoDataContainer.size(); i++ )
	{
		if( s_AniInfoDataContainer[ i ]->pAniInfoData == pData )
		{
			DeleteAniInfoData( i );
			s_AniInfoDataContainer.erase( s_AniInfoDataContainer.begin() + i );
		}
		break;
	}
}

void CAniInfoData::DeleteAllAniInfoData()
{
	unsigned int i;

	for( i = 0; i < s_AniInfoDataContainer.size(); i++ )
	{
		DeleteAniInfoData( i );
	}
	s_AniInfoDataContainer.erase( s_AniInfoDataContainer.begin(), s_AniInfoDataContainer.end() );
}

CAniInfoData* CAniInfoData::GetData( int nIndex )
{
	BsAssert( nIndex >=0 && nIndex < (int)s_AniInfoDataContainer.size() );
	return s_AniInfoDataContainer[nIndex]->pAniInfoData;
}