#include "stdafx.h"
#include "BsConsole.h"
#include "BsAni.h"
#include "BStreamExt.h"
#include "BsUtil.h"
#include "BsAniObject.h"
#include "BsFileIO.h"

#define _OPT_SHINJICH_BSANI_CPP	0

#if defined(USE_ANIDBG)

#include "BsDebugBreak.h"

static Debug::DebugInfo s_aniDbgInfo[] =
{
	// name:         show:             break:
	{ "C_LS_HS.ba",  Debug::OnAll,     Debug::OnNothing },
	{ "",            Debug::OnNothing, Debug::OnNothing }
};

#endif

CBsAni::CBsAni()
{
	m_nRefCount=1;
	m_nAniCount=0;
	m_nBoneCount=0;
	m_pRootBone=NULL;
	m_ppBoneList=NULL;
	m_pTotalFrame=NULL;
#ifdef _LOAD_MAP_CHECK_
	m_nLoadSize = 0;
#endif
}

CBsAni::~CBsAni()
{
#if defined(USE_ANIDBG)
	char const * szFileName = GetAniFileName();
	Debug::TriggerEvent( s_aniDbgInfo, szFileName ? szFileName : "<unknown>", m_nRefCount, Debug::OnDelete );
#endif

	Clear();
}

void CBsAni::Clear()
{
	if(m_pRootBone){
		delete m_pRootBone;
		m_pRootBone=NULL;
	}
	if(m_ppBoneList){
		delete [] m_ppBoneList;
		m_ppBoneList=NULL;
	}
	if(m_pTotalFrame){
		delete [] m_pTotalFrame;
		m_pTotalFrame=NULL;
	}
}

void CBsAni::AddRef()
{
	int nNewRefCount = InterlockedIncrement(reinterpret_cast<volatile LONG *>(&m_nRefCount));

#if defined(USE_ANIDBG)
	char const * szFileName = GetAniFileName();
	Debug::TriggerEvent( s_aniDbgInfo, szFileName ? szFileName : "<unknown>", nNewRefCount, Debug::OnAddRef );
#endif
}

int CBsAni::Release()
{
	int nNewRefCount = InterlockedDecrement(reinterpret_cast<volatile LONG *>(&m_nRefCount));

	// If this BsAssert fires then there were too many Release calls on this object
	BsAssert(nNewRefCount >= 0);

#if defined(USE_ANIDBG)
	char const * szFileName = GetAniFileName();
	Debug::TriggerEvent( s_aniDbgInfo, szFileName ? szFileName : "<unknown>", nNewRefCount, Debug::OnRelease );
#endif

	return nNewRefCount;
}

int CBsAni::LoadSAF(const char *pFileName, bool bUseRootBone)
{
	DWORD dwFileSize;
	VOID *pData;

	if( FAILED(CBsFileIO::LoadFile( pFileName, &pData, &dwFileSize ) ) )
	{
		CBsConsole::GetInstance().AddFormatString("%s File Not Found", pFileName);
		return 0;
	}
	BMemoryStream stream(pData, dwFileSize);
	int nRet = LoadSAF( &stream, bUseRootBone );
	CBsFileIO::FreeBuffer(pData);
	return nRet;
}

int CBsAni::LoadSAF(BStream* pStream, bool bUseRootBone)
{
	Clear();

	BA_FILE_HEADER ba_header;
	pStream->Read(&ba_header, sizeof(BA_FILE_HEADER), ENDIAN_FOUR_BYTE); // 헤더 파일 구조가 바뀔 경우에는 nOffset 만 먼저 읽어서 확장 합니다

	if(ba_header.nVersion != BA_FILE_HEADER::eVersion) {
		DebugString("Invalid BA File Version!!!\n");
		return 0;
	}
	
	m_nAniCount=ba_header.nAniCount;
	m_nBoneCount=ba_header.nBoneCount;
	// ba_header.nAniType

	m_pTotalFrame=new int[m_nAniCount];
	pStream->Read( m_pTotalFrame, sizeof(int)*m_nAniCount, ENDIAN_FOUR_BYTE);

	int i;
	for( i=0;i<m_nBoneCount;i++){
		m_pRootBone = CBsBone::LoadBone(pStream, m_pRootBone, m_nAniCount);
	}
	if(bUseRootBone){ // default
		m_pRootBone->SetRootBone(true);
	}
	m_pRootBone->SetAnimationIndex(0);
	
	m_ppBoneList=new CBsBone*[m_nBoneCount];
	for(i=0;i<m_nBoneCount;i++){
		m_ppBoneList[i]=m_pRootBone->FindBone(i); // 참조용 입니다
		BsAssert(m_ppBoneList[i] && "Invalid Bone!!");
	}

	CalculateRootBonePosition();

	return 1;
}

void CBsAni::DeleteBone(const char *pBoneName)
{
	CBsBone *pFindBone, *pParentBone;
	int i;

	pFindBone=FindBone(pBoneName);
	if(pFindBone){
		pParentBone=FindBone(pFindBone->GetParentName());
		if(pParentBone){
			pParentBone->RemoveChildPtr(pFindBone);
			for(i=0;i<pFindBone->GetChildCount();i++){
				pParentBone->AddChild(pFindBone->GetChildPtr(i));
				pFindBone->RemoveChildPtr(i);
			}
		}
		m_nBoneCount--;
		delete pFindBone;
		m_pRootBone->SetAnimationIndex(0);
	}
}

void CBsAni::DeleteBone(int nIndex)
{
	CBsBone *pFindBone, *pParentBone;
	int i;

	pFindBone=FindBone(nIndex);
	if(pFindBone){
		pParentBone=FindBone(pFindBone->GetParentName());
		if(pParentBone){
			pParentBone->RemoveChildPtr(pFindBone);
			for(i=0;i<pFindBone->GetChildCount();i++){
				pParentBone->AddChild(pFindBone->GetChildPtr(i));
				pFindBone->RemoveChildPtr(i);
			}
		}
		m_nBoneCount--;
		delete pFindBone;
		m_pRootBone->SetAnimationIndex(0);
	}
}

void CBsAni::SetBoneRotation( const char *pBoneName, D3DXVECTOR3 *pRotation )
{
	FindBone( pBoneName )->SetBoneRotation( pRotation );
}

void CBsAni::SetCalcAniPosition( int nCalcAniPosition )
{	
	int i;

	for( i = 0; i < m_nBoneCount; i++ )
	{
		m_ppBoneList[ i ]->SetCalcAniPosition( nCalcAniPosition );
	}
}

void CBsAni::CalculateAnimationMatrix(CBsObject *pObj)
{	
	int i;
	BSMATRIX Identity;

	BsMatrixIdentity( &Identity );
	m_ppBoneList[ 0 ]->SetParentTransMat( &Identity );

	for( i = 0; i < m_nBoneCount; i++ )
	{
		m_ppBoneList[ i ]->CalculateAnimation();
	}
	if( pObj )
	{
		for( i = 0; i < m_nBoneCount; i++ )
		{
			pObj->SaveMatrix( ( D3DXMATRIX * )m_ppBoneList[ i ]->GetVertexTransMatrix(), i );	
		}
	}
}

void CBsAni::SetAnimationFrame( int nAni, float fFrame )
{
	BsAssert( ( nAni < m_nAniCount ) && ( fFrame >= 0.0f ) && ( fFrame < m_pTotalFrame[ nAni ] ) );

	int i;

	for( i = 0; i < m_nBoneCount; i++ )
	{
		m_ppBoneList[ i ]->SetAnimationFrame( nAni, fFrame );
	}
}

void CBsAni::SetAnimationFrameBone( int nAni, float fFrame, int nBoneIndex )
{
	BsAssert( ( nAni < m_nAniCount ) && ( fFrame >= 0.0f ) && ( fFrame < m_pTotalFrame[ nAni ] ) );

	FindBone( nBoneIndex )->SetAnimationFrameBone( nAni, fFrame );
}

void CBsAni::BlendAnimationFrame(int nBlendAni, float fBlendFrame, float fBlendWeight, int nBoneIndex)
{
	BsAssert((nBlendAni<m_nAniCount)&&(fBlendFrame>=0.0f)&&(fBlendFrame<m_pTotalFrame[nBlendAni]));

	FindBone(nBoneIndex)->BlendAnimationFrame(nBlendAni, fBlendFrame, fBlendWeight);
}


const char *CBsAni::GetBoneName(int nIndex)
{
	CBsBone *pBone;

	pBone=FindBone(nIndex);
	if(!pBone){
		return NULL;
	}

	return pBone->GetBoneName();
}

int CBsAni::AddSkeletonAni(CBsAni *pAni, int nIndex/*=-1*/)
{
	int nAniCount;

	if( ( !pAni ) || ( !m_pRootBone ) )
	{
		return 0;
	}
	if(nIndex==-1){
		nIndex=m_nAniCount;
	}
	nAniCount=pAni->GetAniCount();
	if(!m_pRootBone->CheckAddAni(pAni->GetRootBone())){
		return 0;
	}
	
	m_pTotalFrame=(int *)realloc(m_pTotalFrame, sizeof(int)*(m_nAniCount+nAniCount));
	memmove(m_pTotalFrame+nIndex+nAniCount, m_pTotalFrame+nIndex, sizeof(int)*(m_nAniCount-nIndex));
	memcpy(m_pTotalFrame+nIndex, pAni->GetAniLength(), nAniCount*sizeof(int));

	m_nAniCount+=nAniCount;

	m_pRootBone->AddAni(pAni->GetRootBone(), nIndex);

	return 1;
}

void CBsAni::DeleteSkeletonAni(int nIndex)
{
	BsAssert(nIndex<m_nAniCount);

	m_pRootBone->DeleteAni(nIndex);
	memcpy(m_pTotalFrame+nIndex, m_pTotalFrame+nIndex+1, sizeof(int)*(m_nAniCount-nIndex-1));
	m_nAniCount--;
}

void CBsAni::GetAniDistance( GET_ANIDISTANCE_INFO *pInfo )
{ 
	BsAssert( pInfo->fFrame1 >= 0.0f );
	BsAssert( pInfo->fFrame2 >= 0.0f);

	BsAssert( pInfo->nAni1 < m_nAniCount );
	BsAssert( pInfo->nAni2 < m_nAniCount );

	BsAssert( pInfo->fFrame1 < m_pTotalFrame[ pInfo->nAni1 ] );
	BsAssert( pInfo->fFrame2 < m_pTotalFrame[ pInfo->nAni2 ] );

	if( ( pInfo->nAni1 >= m_nAniCount ) || ( pInfo->nAni2 >= m_nAniCount ) )
	{
		*pInfo->pVector = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
		return;
	}
	if( ( pInfo->fFrame1 >= m_pTotalFrame[ pInfo->nAni1 ] ) || ( pInfo->fFrame2 >= m_pTotalFrame[ pInfo->nAni2 ] ) )
	{
		*pInfo->pVector = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
		return;
	}

	if(m_pRootBone){
		*(pInfo->pVector) = m_RootBonePosition[ pInfo->nAni1 ][ ( int )pInfo->fFrame1 ] - m_RootBonePosition[ pInfo->nAni2 ][ ( int )pInfo->fFrame2 ];
	}
	else{
		*(pInfo->pVector)=D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	}
}

void CBsAni::GetRootBonePos(int nAni, float fFrame, D3DXVECTOR4* pVector)
{
	int nDiv;
	BsAssert((nAni<m_nAniCount)&&(fFrame>=0.f)&&(m_pRootBone));

	if( fFrame < 0.f) {
		fFrame = 0.f;
	}

	nDiv=(int)(fFrame/m_pTotalFrame[nAni]);
	fFrame-=nDiv*m_pTotalFrame[nAni];
	if(fFrame>m_pTotalFrame[nAni]-1){
		fFrame-=m_pTotalFrame[nAni]-1;
	}
	m_pRootBone->GetPositionVector(*(BSVECTOR*)pVector,nAni,fFrame);
}

int CBsAni::GetBoneIndex(const char *pBoneName)
{
	CBsBone *pBone;

	pBone=FindBone(pBoneName);
	if(!pBone){
		return -1;
	}

	return pBone->GetBoneIndex();
}

void CBsAni::SetAniFileName(const char *pFileName)
{
	strcpy(m_szAniFileName, pFileName); //aleksger - safe string

#if defined(USE_ANIDBG)
	char const * szFileName = GetAniFileName();
	Debug::TriggerEvent( s_aniDbgInfo, szFileName ? szFileName : "<unknown>", m_nRefCount, Debug::OnLoad );
#endif
}

const char *CBsAni::GetAniFileName()
{ 
	char *pFindPtr;

	pFindPtr=(char *)strrchr(m_szAniFileName, '\\');
	if(pFindPtr){
		return pFindPtr+1;
	}
	else{
		return m_szAniFileName; 
	}
} 

#ifdef _USAGE_TOOL_

int CBsAni::MakeOneBa(char **ppFileName, int nFile, char *pStoreName) // ststic 함수 입니다

{				
	BFileStream Stream(pStoreName, BFileStream::create);

	if(!Stream.Valid()){
		CBsConsole::GetInstance().AddFormatString("%s File Create Failed", pStoreName);
		return 0;
	}

	CBsAni *ani_buf[256];
	int nBoneCount, nAniSub=0, i,j,k;
	
	if (nFile>256) return 0; // 애니 파일이 많습니다

	for (i=0; i<nFile; i++) {
		ani_buf[i]=new CBsAni;
		ani_buf[i]->LoadSAF(ppFileName[i]);

		nAniSub += ani_buf[i]->m_nAniCount;

		if (i==0) {
			nBoneCount = ani_buf[i]->m_nBoneCount;
		}
		else if (ani_buf[i]->m_nBoneCount != nBoneCount) {
		#ifndef _XBOX
			char buf[256];
			sprintf(buf, "본 갯수 오류 입니다 %s",   ppFileName[i]);
			MessageBox(NULL, buf, "엔진 메세지 박스", MB_OK);
		#endif

			for (j=0; j<i; j++) delete ani_buf[j]; // 메모리

			return 0; // 애니 본 갯수가 같아야 합니다 (메모리 관리 스킵)
		}
	}

	if (nAniSub>256) {
		for (j=0; j<nFile; j++) delete ani_buf[j]; // 메모리

		return 0; // 애니 갯수가 많습니다
	}

	{
		BA_FILE_HEADER ba_header;

		{
			ba_header.Set(nAniSub, ani_buf[0]->GetBoneCount());

			Stream.Write(&ba_header, sizeof(BA_FILE_HEADER), ENDIAN_FOUR_BYTE); 
		}

		{
			for (i=0; i<nFile; i++) { // i,j 만큼 애니 갯수 입니다
				for (j=0; j<ani_buf[i]->m_nAniCount; j++) {
					int v=ani_buf[i]->GetAniLength(j);
					Stream.Write(&v, sizeof(int), ENDIAN_FOUR_BYTE); 
				}
			}
		}

		for (j=0; j<ba_header.nBoneCount; j++) { // 본 갯수 입니다
			CBsBone *pt=ani_buf[0]->FindBone(j);

			Stream.Write(pt->GetBoneName(),   255);
			Stream.Write(pt->GetParentName(), 255);

			Stream.Write(pt->GetInvWorldMatrix(), sizeof(D3DXMATRIX),ENDIAN_FOUR_BYTE);
			Stream.Write(pt->GetLocalMatrix(), sizeof(D3DXMATRIX),ENDIAN_FOUR_BYTE);
			Stream.Write(pt->GetLocalAffine(), sizeof(AFFINE_PARTS),ENDIAN_FOUR_BYTE);

			for (i=0; i<nFile; i++) { // 애니 갯수 입니다
				for (k=0; k<ani_buf[i]->m_nAniCount; k++) {
					CBsBone *pt2=ani_buf[i]->FindBone(j); // 해당 본의 해당 애니
					CBsAniSampledKeyFrame *aif=pt2->GetAniInfo(k); // 각각의 ba_header 는 애니가 1 개 라고 가정 합니다

					int v2=BA_ANIMATION_SAMPLED_KEYFRAME;
					Stream.Write(&v2, sizeof(int), ENDIAN_FOUR_BYTE);

					aif->SaveAniInfo(&Stream);
				}
			}
		}
	}

	for (j=0; j<nFile; j++) delete ani_buf[j]; // 메모리

	return 1;
}

int CBsAni::SaveBa( char *pStoreName )
{
	int i, j;
	BA_FILE_HEADER ba_header;
	BFileStream Stream( pStoreName, BFileStream::create );

	if(!Stream.Valid()){
		CBsConsole::GetInstance().AddFormatString("%s File Create Failed", pStoreName);
		return 0;
	}

	ba_header.Set( m_nAniCount, m_nBoneCount );
	Stream.Write( &ba_header, sizeof(BA_FILE_HEADER), ENDIAN_FOUR_BYTE ); 
	Stream.Write( m_pTotalFrame, sizeof(int) * m_nAniCount, ENDIAN_FOUR_BYTE ); 

	for( i = 0; i < m_nBoneCount; i++ )
	{
		Stream.Write( m_ppBoneList[ i ]->GetBoneName(), 255 );
		Stream.Write( m_ppBoneList[ i ]->GetParentName(), 255 );

		Stream.Write( m_ppBoneList[ i ]->GetInvWorldMatrix(), sizeof( D3DXMATRIX ), ENDIAN_FOUR_BYTE );
		Stream.Write( m_ppBoneList[ i ]->GetLocalMatrix(), sizeof( D3DXMATRIX ),ENDIAN_FOUR_BYTE );
		Stream.Write( m_ppBoneList[ i ]->GetLocalAffine(), sizeof( AFFINE_PARTS ), ENDIAN_FOUR_BYTE );

		for( j = 0; j < m_nAniCount; j++ )
		{ 
			int v2 = BA_ANIMATION_SAMPLED_KEYFRAME;
			Stream.Write(&v2, sizeof(int), ENDIAN_FOUR_BYTE);
			m_ppBoneList[ i ]->GetAniInfo( j )->SaveAniInfo( &Stream );
		}
	}

	return 1;
}

#endif

int CBsAni::GetBoneNameList(char *szBoneName)
{
	for (int i=0; i<m_nBoneCount; i++) {
		CBsBone *pt=m_ppBoneList[i];
		strcpy_s(szBoneName+256*i, 256, pt->GetBoneName()); //aleksger - safer string handling. But should be BsAsserted that szBoneName is of sufficient size.
	}

	return m_nBoneCount;
}

void CBsAni::GetRootBoneRotation( int nAniIndex, float fFrame, D3DXMATRIX *pRet )
{
	BSQUATERNION Quat;

	m_pRootBone->GetRotationVector( Quat, nAniIndex, fFrame );
	BsMatrixRotationQuaternion ( (BSMATRIX*)pRet, &Quat );
}

void CBsAni::GetRootBoneMatrix( int nAniIndex, float fFrame, D3DXMATRIX* pRet, int nCalcAniPosition )
{
	BSQUATERNION Quat;
	BSVECTOR vecPos;

	m_pRootBone->GetRotationVector( Quat, nAniIndex, fFrame );
	m_pRootBone->GetPositionVector(vecPos, nAniIndex, fFrame);
	if( !( nCalcAniPosition & BS_CALC_POSITION_X ) )
	{
		vecPos.x = 0.0f;
	}
	if( !( nCalcAniPosition & BS_CALC_POSITION_Y ) )
	{
		vecPos.y = 0.0f;
	}
	if( !( nCalcAniPosition & BS_CALC_POSITION_Z ) )
	{
		vecPos.z = 0.0f;
	}
#ifdef _XBOX
	// - shinjich
#if _OPT_SHINJICH_BSANI_CPP
	XMVECTOR vzero;
#if _DEBUG
	vzero.x = 0.0f;	// dummy for debug
#else
#pragma warning(disable:4700)
#endif // _DEBUG
	vzero = __vxor( vzero, vzero );
	BsMatrixTransformation((BSMATRIX*)pRet, &vzero, &vzero, &XMVectorSet(1.f, 1.f, 1.f, 0.f), &vzero, &Quat, &vecPos);
#else
	BsMatrixTransformation((BSMATRIX*)pRet, &XMVectorSet(0.f, 0.f, 0.f, 0.f), &XMVectorSet(0.f, 0.f, 0.f, 0.f), &XMVectorSet(1.f, 1.f, 1.f, 0.f), &XMVectorSet(0.f, 0.f, 0.f, 0.f), &Quat, &vecPos);
#endif // _OPT_SHINJICH_BSANI_CPP
#else
	D3DXMatrixTransformation(pRet, NULL, NULL, NULL, NULL, &Quat, &vecPos);
#endif
	D3DXMATRIX matModify(0,0,-1,0, 0,1,0,0, 1,0,0,0, 0,0,0,1);
	//D3DXMatrixRotationY(&matModify, D3DX_PI*0.5f);
	D3DXMatrixMultiply(pRet, &matModify, pRet);
}

BSVECTOR CBsAni::GetRootBoneLocalPosition()
{
	return m_pRootBone->GetLocalAffine()->Translation;
}


void CBsAni::CalculateRootBonePosition()
{
	m_RootBonePosition.clear();

	int i, j;
	BSVECTOR Position;
	CBsBone *pBone;

	pBone = FindBone( "Bip01" );
	if( !pBone )
	{
		pBone = m_pRootBone;
	}
	else
	{
		pBone->SetRootBone( true );
	}
	m_RootBonePosition.resize( m_nAniCount );
	for( i = 0; i < m_nAniCount; i++ )
	{
		// - shinjich
#if _OPT_SHINJICH_BSANI_CPP
		// ...should be more test
		float fj = 0.0f;
		for( j = 0; j < m_pTotalFrame[ i ]; j++, fj += 1.0f )
		{
			pBone->GetPositionVector( Position, i, fj );
			m_RootBonePosition[ i ].push_back( *(D3DXVECTOR3*)&Position );
		}
#else
		for( j = 0; j < m_pTotalFrame[ i ]; j++ )
		{
			pBone->GetPositionVector( Position, i, ( float )j );
			m_RootBonePosition[ i ].push_back( *(D3DXVECTOR3*)&Position );
		}
#endif
	}
}

void CBsAni::DeleteAnimationKey( int nBoneIndex )
{
	m_ppBoneList[ nBoneIndex ]->DeleteAnimationKey();
}
