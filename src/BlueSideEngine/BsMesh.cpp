#include "stdafx.h"
#include "BStreamExt.h"
#include "BsMesh.h"
#include "BsSubMesh.h"
#include "BsPhysiqueSubMesh.h"
#include "BsKernel.h"
#include "BsAni.h"
#include "BsFileIO.h"

int			CBsMesh::s_nDetectBookingIndex; // 임시 변수 입니다
D3DXVECTOR3	CBsMesh::s_vecBooking;
int			CBsMesh::s_nDetectCollisionIndex; // 임시 변수 입니다
D3DXVECTOR3	CBsMesh::s_vecCollision;
int			CBsMesh::s_nDetectLinkDummyIndex;
D3DXMATRIX	CBsMesh::s_matLinkDummy;


int			CBsMesh::s_nDetectMeshCopy; // 임시 변수 입니다

int         CBsMesh::s_bLowLOD = false;

CBsMesh::CBsMesh()
{
	m_nRefCount = 1;
	m_szMeshFileName = NULL;

	m_ppSubMesh = NULL;

	m_nBookingCount	  = 0;
	m_nCollisionCount = 0;
	m_nLinkDummyCount = 0;

	m_ppVertexBuffer=NULL;
	m_nVertexCount=0;
	m_ppFaceBuffer=NULL;
	m_nFaceCount=0;
}

CBsMesh::~CBsMesh()
{
	Clear();
}

int CBsMesh::Release()
{
	--m_nRefCount;
	int nRefCount = m_nRefCount;
	if( m_nRefCount <= 0) {
        delete this;        		
	}
	return nRefCount;
}

void CBsMesh::SetMeshFileName(const char *pFileName)
{
	g_BsKernel.FnaPoolClear(m_szMeshFileName); // 메모리 프리 합니다
	m_szMeshFileName=g_BsKernel.FnaPoolAdd(pFileName); 
}

const char *CBsMesh::GetMeshFileName() 
{
	char *pFindPtr;

	pFindPtr=strrchr(m_szMeshFileName, '\\');
	if(pFindPtr){
		return pFindPtr+1;
	}
	else{
		return m_szMeshFileName;
	}
}

void CBsMesh::SetBoundingBox()   
{				 
	if (m_ppSubMesh) {
		AABB aabb;
		const float mm=10000000;
		aabb.Vmin=BSVECTOR( mm, mm, mm);
		aabb.Vmax=BSVECTOR(-mm,-mm,-mm);

		for(int i=0;i<m_nSubMeshCount;i++){
			m_ppSubMesh[i]->SetBoundingBox(aabb);
		}
		m_BoundingBox=aabb;
		m_fRadius = BsVec3Length(&(m_BoundingBox.GetExtent()));
	}
}

void CBsMesh::WriteBoundingBox(char *szFileName)
{
	char *mem;
	int size;

	{  
		BFileStream stream(szFileName, BFileStream::openRead);

		if(!stream.Valid()){
			CBsConsole::GetInstance().AddFormatString("%s File Not Found", szFileName);
			return;
		}

		size=stream.Length(); 
		mem = new char[size];
		stream.Read(mem, size); 
	}

	{
		SetFileAttributes( szFileName , 0);
		DeleteFile( szFileName );
		BFileStream stream(szFileName, BFileStream::create);	

		if(!stream.Valid()){
			CBsConsole::GetInstance().AddFormatString("%s File Create Failed", szFileName);
// [PREFIX:beginmodify] 2006/2/15 junyash PS#5170 reports leaking memory 'mem = new char[size];'
			delete [] mem;
// [PREFIX:endmodify] junyash
			return;
		}

		stream.Write(mem, size); 
		stream.Seek(m_nBoundingBoxPos, BStream::fromBegin);
		stream.Write(&m_BoundingBox.Vmin, sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE); 
		stream.Write(&m_BoundingBox.Vmax, sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE);
	}

	delete [] mem;
}

void CBsMesh::Clear()
{
	g_BsKernel.FnaPoolClear(m_szMeshFileName); // 메모리 프리 합니다
	m_szMeshFileName = NULL;

	if (m_ppSubMesh) {
		for(int i=0;i<m_nSubMeshCount;i++){
			delete m_ppSubMesh[i]; // CBsPhysiqueSubMesh* 일때를 고려 합니다
		}
		delete [] m_ppSubMesh;
		m_ppSubMesh = NULL;
	}

	m_nBookingCount=0;
	m_vecBookingList.clear();

	m_nCollisionCount=0;
	m_vecCollisionList.clear();

	m_nLinkDummyCount=0;
	m_matLinkDummyList.clear();

	if (m_ppVertexBuffer) {
		delete [] m_ppVertexBuffer;
		m_ppVertexBuffer=NULL;
	}
	if (m_ppFaceBuffer) {
		delete [] m_ppFaceBuffer;
		m_ppFaceBuffer=NULL;
	}
}

int CBsMesh::LoadMesh(BStream *pStream)
{
	if (!pStream) return 0;

	s_nDetectMeshCopy=0; 

	BM_FILE_HEADER headerBMFile;

	int rt=pStream->Read(&headerBMFile.nOffset, sizeof(int), ENDIAN_FOUR_BYTE); // 헤더 파일 구조가 바뀔 경우에는 nOffset 만 먼저 읽어서 확장 합니다
	if (!rt) return 0;
	// headerBMFile.nOffset (현재 버전과 다를 수 있습니다)
	headerBMFile.nLOD=1; // 1 = LOD 를 사용하지 않습니다 
	headerBMFile.nUVAni=0; // (같은 버전이지만 이전에 다른 오프셋을 가졌던 파일을 읽어 오기 위한 코드 입니다)
	pStream->Read(&headerBMFile.nFileID, headerBMFile.nOffset-sizeof(int), ENDIAN_FOUR_BYTE); // 실재 내용을 읽습니다 (headerBMFile.nOffset 주의)

	if (headerBMFile.nVersion<BM_FILE_HEADER::eVersion) { // 버전을 확인 합니다
		DebugString("Invalid BM File Version!!!\n");
		return 0;
	}
	m_nSubMeshCount = headerBMFile.nSubMeshCount;
	
	m_nLOD = headerBMFile.nLOD;	

	m_nLODLevel=0; // 최고 폴리곤 레벨 (디폴트)

	m_nUVAnimation = headerBMFile.nUVAni; // 0,1    
	
	Clear(); // 메모리 소거 합니다

	m_ppSubMesh = new CBsSubMesh* [m_nSubMeshCount]; // 메모리 할당 합니다

	int nFreeBuffer[256], nFreeBufferN=0, i,j;
	BM_SUBMESH_HEADER headerSubMesh;

	for(i=0;i<m_nSubMeshCount;i++){
		pStream->Read(&headerSubMesh.nOffset, 4, ENDIAN_FOUR_BYTE); // 헤더 파일 구조가 바뀔 경우에는 nOffset 만 먼저 읽어서 확장 합니다
		// headerSubMesh.nOffset (현재 버전과 다를 수 있습니다)
		pStream->Read(&headerSubMesh.nMeshFlag, headerSubMesh.nOffset-4, ENDIAN_FOUR_BYTE); 
		
		if (headerSubMesh.nOffset==24) { // 이전 파일들의 값은 0 으로 초기화 합니다
			headerSubMesh.nStreamNumber=0;
			memset(headerSubMesh.nStreamType,  0, 4*BM_SUBMESH_HEADER::eStreamNumberMax); 
			memset(headerSubMesh.nStreamUsage, 0, 4*BM_SUBMESH_HEADER::eStreamNumberMax);
		}

		if (headerSubMesh.nOffset==24 || headerSubMesh.nOffset==24+84) { // 이전 파일들의 값은 0 으로 초기화 합니다 
			headerSubMesh.nLODAllocated=0;
		}

		// Link Dummy를 위해 추가!!
		if (headerSubMesh.nOffset==24 || headerSubMesh.nOffset==24+84 || headerSubMesh.nOffset==24+84+4) {
			D3DXMatrixIdentity( &headerSubMesh.matLinkDummy );
		}

		if (headerSubMesh.nMeshFlag & (BM_SUBMESH_HEADER::ePHYSIQUE | BM_SUBMESH_HEADER::eBLEND_PHYSIQUE)) { // 0x03
			m_ppSubMesh[i]=new CBsPhysiqueSubMesh();
		}
		else {                     
			m_ppSubMesh[i]=new CBsSubMesh();
		}

		m_ppSubMesh[i]->LoadSubMesh(pStream, &headerSubMesh); // 버텍스 스트림을 읽을 것입니다

		if (s_nDetectBookingIndex!=-1) { //	booking 관련 데이타 삭제를 준비 합니다
			m_vecBookingList.push_back(s_vecBooking);
			m_nBookingCount++;
			nFreeBuffer[nFreeBufferN++]=i;
		}

		if (s_nDetectCollisionIndex!=-1) { // collision 관련 데이타 삭제를 준비 합니다
			m_vecCollisionList.push_back(s_vecCollision);
			m_nCollisionCount++;
			nFreeBuffer[nFreeBufferN++]=i;
		}

		if ( s_nDetectLinkDummyIndex != -1 ) {
			m_matLinkDummyList.push_back(s_matLinkDummy);
			m_nLinkDummyCount++;
			nFreeBuffer[nFreeBufferN++]=i;
		}
	}

	m_nBoundingBoxPos=pStream->Tell(); // 나중에 파일에 접근할 경우를 대비해서 저장 합니다
	pStream->Read(&m_BoundingBox.Vmin, sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE); // 플러그인에서 미리 계산 한 것을 읽습니다
	pStream->Read(&m_BoundingBox.Vmax, sizeof(D3DXVECTOR3), ENDIAN_FOUR_BYTE);
	m_fRadius = BsVec3Length(&(m_BoundingBox.GetExtent()));

	// booking 과 collision 관련 데이타를 벡터만 뺴고 모두 제거 합니다

	if (nFreeBufferN) {
		for(i=0; i<nFreeBufferN; i++) {
			int del=nFreeBuffer[i]-i;

			delete m_ppSubMesh[del]; // CBsPhysiqueSubMesh* 일때를 고려 합니다

			for (j=del; j<m_nSubMeshCount-1-i; j++)	{
				m_ppSubMesh[j]=m_ppSubMesh[j+1];
			}
		}

		m_nSubMeshCount-=nFreeBufferN;
	}

	//

	if (m_nLOD>1) {	//$ LOD 관련 추가 데이타를 읽는 부분 입니다
		for (i=0; i<m_nLOD-1; i++) { // 추가 되는 갯수 입니다
			CBsMesh TempMesh; // 임시 메쉬 입니다  
			int rt=TempMesh.LoadMesh(pStream); // 메모리 자동 관리 됩니다

			if (!rt && m_nLOD==5) { // 삭제 예정
				m_nLOD=1; // LOD 관련 파일을 읽을때 에러가 나면 옜날 호환이 안되는 버전(2004.11 월 버전)이므로 강제로 1 로 만들어 줍니다
				// 여기서 파일이름을 메세지 박스로 뛰우면 호환 안되는 파일 이름을 알수 있습니다 (3 개 정도 되는 듯)
				BsAssert(i==0 && "파일이 이상 합니다"); // 0 번이라면 정상 동작 입니다
				break;
			}

			for (j=0; j<m_nSubMeshCount; j++) { 
				// 서브 메쉬 갯수가 다른 LOD 를 유지하기 위해서는,
				// 매트리얼 툴에서 다른 갯수의서브메쉬 합치기를 지원하도록 한후,
				// 여기에서 강제로 루틴 처리 합니다

				m_ppSubMesh[j]->AddLODStream(TempMesh.m_ppSubMesh[j]); // 현재 메쉬의 서브메쉬에 해당 데이타를 추가 해 줍니다
			}
		}
	}

	//

	if (s_nDetectMeshCopy) {
		if (m_ppVertexBuffer) delete [] m_ppVertexBuffer;
		if (m_ppFaceBuffer) delete [] m_ppFaceBuffer;

		GetStreamBufferAll(&m_ppVertexBuffer, &m_ppFaceBuffer, m_nVertexCount, m_nFaceCount);
	}

	return 1;
}

int CBsMesh::Reload()
{
	int nRet;
	char szFileName[256];

	strcpy(szFileName, GetMeshFileFullName());
	Clear();

	DWORD dwFileSize;
	VOID *pData;

	if( FAILED(CBsFileIO::LoadFile( szFileName, &pData, &dwFileSize ) ) )
	{
		CBsConsole::GetInstance().AddFormatString("%s File Not Found", szFileName);
		return -1;
	}
	BMemoryStream Stream(pData, dwFileSize);

	nRet=LoadMesh(&Stream);
	SetMeshFileName(szFileName);

	CBsFileIO::FreeBuffer(pData);

	return nRet;
}

void CBsMesh::LinkBone(CBsAni *pAni)
{
	BsAssert(m_ppSubMesh);
	BsAssert(pAni);

	for (int i=0;i<m_nSubMeshCount;i++){ 
		m_ppSubMesh[i]->LinkBone(pAni);
	}
}

void CBsMesh::LinkBone()
{
	int i;

	for( i = 0; i < m_nSubMeshCount; i++ )
	{
		m_ppSubMesh[i]->LinkBone( m_szBoneList );
	}
}

D3DXVECTOR3* CBsMesh::GetBookingPosition(int nIndex)
{
	BsAssert(nIndex<(int)m_vecBookingList.size() && "Invalid Mesh Booking Index!!");

	if (nIndex==-1) {
		return NULL;
	}
	else if (nIndex < (int)m_vecBookingList.size()) {  // 0 .. n-1
		return &m_vecBookingList[nIndex];
	}

	return NULL;
}

D3DXVECTOR3* CBsMesh::GetCollisionPosition(int nIndex)
{
	BsAssert(nIndex<(int)m_vecCollisionList.size() && "Invalid Mesh Collision Index!!");

	if (nIndex==-1) {
		return NULL;
	}
	else if (nIndex < (int)m_vecCollisionList.size()) {  // 0 .. n-1
		return &m_vecCollisionList[nIndex];
	}

	return NULL;
}

const D3DXMATRIX* CBsMesh::GetLinkDummyMatrix(int nIndex)
{
	BsAssert( nIndex>=0 && "Link Dummy Index Invalide!!" );
	if(nIndex < int(m_matLinkDummyList.size())) {
		return &m_matLinkDummyList[nIndex];
	}
	return NULL;
}
 
int CBsMesh::CalculateLODLevel(float fDistanceFromCamera)
{
//$$ 거리에 따라 LOD 레벨을 설정 합니다
#ifdef _USAGE_TOOL_
	CBsSubMesh::g_nLODLevel_ = m_nLODLevel; // 툴에서는 이 코드가 적용 되어야 합니다
	//                        |
	//                        현재 그려지고 있는 LOD 레벨 입니다  0..m_nLOD-1  (0 일때 가장 폴리곤 수가 많습니다)
#else
	int nCheckLevel;

	if (s_bLowLOD) {
		nCheckLevel = m_nLOD-1;
	}
	else
	{
		int nDistance = (int)(fDistanceFromCamera - (2.f * m_fRadius));

		if(nDistance<500) {
			nCheckLevel = 0;
		}
		else if(nDistance<2000) {
			nCheckLevel = 1;
		}
		else if(nDistance<3000) {
			nCheckLevel = 2;
		}
		else if(nDistance<4000) {
			nCheckLevel = 3;
		}
		else {
			nCheckLevel = 4;
		}


		if(nCheckLevel>=m_nLOD) {
			nCheckLevel = m_nLOD-1;
		}
	}


	CBsSubMesh::g_nLODLevel_=nCheckLevel; // 임시 값으로 값을 넘겨주는 패러미터 용도 입니다
#endif
	return CBsSubMesh::g_nLODLevel_;
}

int CBsMesh::ForceLODLevel(int nLevel)
{
	int nCheckLevel = nLevel;
	if(nCheckLevel>=m_nLOD) {
		nCheckLevel = m_nLOD-1;
	}
	CBsSubMesh::g_nLODLevel_=nCheckLevel;
	return nCheckLevel;
}

void CBsMesh::StoreBoneList()
{
	int i, j, k;
	int nBoneCount, nListSize;
	char *pBoneName;

	if( m_szBoneList.size() > 0 )
	{
		LinkBone();
		return;
	}
	for( i = 0 ; i < m_nSubMeshCount; i++ )
	{
		nBoneCount = m_ppSubMesh[ i ]->GetBoneCount();
		for( j = 0; j < nBoneCount; j++ )
		{
			pBoneName = m_ppSubMesh[ i ]->GetBoneName( j );
			nListSize = ( int )m_szBoneList.size();
			for( k = 0; k < nListSize; k ++ )
			{
				if( _stricmp( pBoneName, m_szBoneList[ k ].c_str() ) == 0 )
				{
					break; // 같은 본 이름 일때 스킵 됩니다 (주의)
				}
			}
			if( k >= nListSize )
			{
				m_szBoneList.push_back( pBoneName );
			}
		}
	}

	LinkBone();
}

int CBsMesh::GetStoredBoneIndex( const char *pBoneName )
{
	int i, nCount;

	nCount = m_szBoneList.size();
	for( i = 0; i < nCount; i++ )
	{
		if( _stricmp( pBoneName, m_szBoneList[ i ].c_str() ) == 0 )
		{
			return i;
		}
	}

	return -1;
}

void CBsMesh::GetStreamBuffer(D3DXVECTOR3 **ppVertexBuffer, WORD **ppFaceBuffer, int &nVertexCount, int &nFaceCount, int nSubMeshIndex)
{
	BsAssert(m_nSubMeshCount>0 && "서브 메쉬가 없습니다");

	m_ppSubMesh[nSubMeshIndex]->GetStreamForLOD()->GetStreamBuffer(ppVertexBuffer,ppFaceBuffer,nVertexCount,nFaceCount);
}

void CBsMesh::GetStreamBuffer( D3DXVECTOR3 **ppVertexBuffer, int &nVertexCount )
{
	BsAssert(m_nSubMeshCount>0 && "서브 메쉬가 없습니다");

	m_ppSubMesh[0]->GetStreamForLOD()->GetStreamBuffer( ppVertexBuffer, nVertexCount );
}

void CBsMesh::GetStreamBufferAll(D3DXVECTOR3 **ppVertexBuffer, WORD **ppFaceBuffer, int &nVertexCount, int &nFaceCount)
{
	BsAssert(m_nSubMeshCount>0 && "서브 메쉬가 없습니다");
    BsAssert(m_nSubMeshCount<16 && "서브 메쉬가 너무 많습니다");

	D3DXVECTOR3 *t_ppVertexBuffer[16];
	WORD *t_ppFaceBuffer[16];
	int t_nVertexCount[16], t_nFaceCount[16];
	nVertexCount=0;
	nFaceCount=0;

	for (int i=0; i<m_nSubMeshCount; i++) {
		m_ppSubMesh[i]->GetStreamForLOD()->GetStreamBuffer(&t_ppVertexBuffer[i], &t_ppFaceBuffer[i], t_nVertexCount[i], t_nFaceCount[i]);
		
		nVertexCount+=t_nVertexCount[i];
		nFaceCount+=t_nFaceCount[i];
	}

	*ppVertexBuffer=new D3DXVECTOR3 [nVertexCount];
	*ppFaceBuffer=new WORD [nFaceCount*3];
	int nOfs=0, nOfsFace=0;

	for (int i=0; i<m_nSubMeshCount; i++) {
		memcpy((*ppVertexBuffer)+nOfs, t_ppVertexBuffer[i], t_nVertexCount[i]*sizeof(D3DXVECTOR3));

		for (int j=0; j<t_nFaceCount[i]*3; j++) {
			((*ppFaceBuffer)+nOfsFace)[j] = (t_ppFaceBuffer[i])[j]+nOfs;
		}
		
		nOfs+=t_nVertexCount[i];
		nOfsFace+=t_nFaceCount[i]*3;

		delete [] t_ppVertexBuffer[i];
		delete [] t_ppFaceBuffer[i];
	}

	// 디버깅 정보 입니다
	/*
	char str[1024];
	for (int i=0; i<nVertexCount; i++) {
		sprintf(str, "%03d  %4.2f  %4.2f  %4.2f\n", i,(*ppVertexBuffer)[i].x,(*ppVertexBuffer)[i].y,(*ppVertexBuffer)[i].z);
		DebugString(str);
	}
	for (int i=0; i<nFaceCount; i++) {
		sprintf(str, "%03d  %d %d %d\n", i, (*ppFaceBuffer)[i*3], (*ppFaceBuffer)[i*3+1], (*ppFaceBuffer)[i*3+2]);
		DebugString(str);
	}
	*/
}

void CBsMesh::GetStreamBufferAll( D3DXVECTOR3 **ppVertexBuffer, int &nVertexCount )
{
	BsAssert(m_nSubMeshCount> 0 && "서브 메쉬가 없습니다");
	BsAssert(m_nSubMeshCount<16 && "서브 메쉬가 너무 많습니다");

	D3DXVECTOR3 *t_ppVertexBuffer[16];
	int t_nVertexCount[16];
	nVertexCount=0;

	for (int i=0; i<m_nSubMeshCount; i++) {
		m_ppSubMesh[i]->GetStreamForLOD()->GetStreamBuffer(&t_ppVertexBuffer[i], t_nVertexCount[i]);
		nVertexCount+=t_nVertexCount[i];
	}

	*ppVertexBuffer=new D3DXVECTOR3 [nVertexCount];
	int nOfs=0;

	for (int i=0; i<m_nSubMeshCount; i++) {
		memcpy((*ppVertexBuffer)+nOfs, t_ppVertexBuffer[i], t_nVertexCount[i]*sizeof(D3DXVECTOR3));

		nOfs+=t_nVertexCount[i];
		delete [] t_ppVertexBuffer[i];
	}

	// 디버깅 정보 입니다
	/*
	char str[1024];
	for (int i=0; i<nVertexCount; i++) {
		sprintf(str, "%03d  %4.2f  %4.2f  %4.2f\n", i,(*ppVertexBuffer)[i].x,(*ppVertexBuffer)[i].y,(*ppVertexBuffer)[i].z);
		DebugString(str);
	}
	*/
}

void CBsMesh::GetStream( UINT uiUsage, LPDIRECT3DVERTEXBUFFER9& pVB, LPDIRECT3DINDEXBUFFER9& pIB )
{
	m_ppSubMesh[0]->GetStreamForLOD()->GetStream( uiUsage, pVB, pIB );
}

void CBsMesh::GetStreamBufferExt(D3DXVECTOR3 **ppPositionBuffer, D3DXVECTOR3 **ppNormalBuffer, int **ppBoneBuffer, int &nVertexCount)
{
	BsAssert(m_nSubMeshCount>0 && "서브 메쉬가 없습니다");

	m_ppSubMesh[0]->GetStreamForLOD()->GetStreamBufferExt( ppPositionBuffer, ppNormalBuffer, ppBoneBuffer, nVertexCount );
}

int CBsMesh::GetPrimitiveCountLOD()
{
	int nPrimitiveCountSum=0;

	for (int i=0;i<m_nSubMeshCount;++i)	{
		nPrimitiveCountSum += m_ppSubMesh[i]->GetPrimitiveCountLOD(m_nLODLevel);
	}

	return nPrimitiveCountSum;
}

//------------------------------------------------------------------------------------------------







