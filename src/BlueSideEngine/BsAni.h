#ifndef BsAni_H 
#define BsAni_H
#pragma once

#include "BsBone.h"

class BStream;

//------------------------------------------------------------------------------------------------

class CBsAni
{
public:
	CBsAni();
	virtual ~CBsAni();

protected:

	int m_nRefCount;
	int m_nAniCount;

	int m_nBoneCount;
	CBsBone *m_pRootBone; // 메모리	= 1
	CBsBone **m_ppBoneList;	// 메모리 = m_nBoneCount
	int *m_pTotalFrame;	// 메모리 = m_nAniCount

	char m_szAniFileName[_MAX_PATH]; // 버퍼

	std::vector< std::vector< D3DXVECTOR3 > > m_RootBonePosition;

public:
	void Clear();

	void AddRef();
	int  Release();

	int GetRefCount() { return m_nRefCount; }

	int LoadSAF(BStream* pStream, bool bUseRootBone=true);
	int LoadSAF(const char *pFileName, bool bUseRootBone=true);

	CBsBone *FindBone(const char *pBoneName) { return m_pRootBone->FindBone(pBoneName); }
	CBsBone *FindBone(int nIndex) { return m_ppBoneList[nIndex]; }
	int  FindBoneIndex(const char *pBoneName) { return m_pRootBone->FindBoneIndex(pBoneName); }	

	void DeleteBone(const char *pBoneName);
	void DeleteBone(int nIndex);

	void SetBoneRotation( const char *pBoneName, D3DXVECTOR3 *pRotation );
	void SetCalcAniPosition( int nCalcAniPosition );
	void CalculateAnimationMatrix(CBsObject *pObj);
	void SetAnimationFrame( int nAni, float fFrame );
	void SetAnimationFrameBone( int nAni, float fFrame, int nBoneIndex );
	void BlendAnimationFrame(int nBlendAni, float fBlendFrame, float fBlendWeight, int nBoneIndex);

	void ComputeVertexTransMatrix() { m_pRootBone->ComputeVertexTransMatrix(); }

	int GetBoneCount() const { return m_nBoneCount; }
	int GetAniLength(int nAni) { return m_pTotalFrame[nAni]; }
	int *GetAniLength() { return m_pTotalFrame; }
	int GetAniCount() { return m_nAniCount; }
	CBsBone *GetRootBone() { return m_pRootBone; }
	int GetBoneIndex(const char *pBoneName);
	const char *GetBoneName(int nIndex);

	int  AddSkeletonAni(CBsAni *pAni, int nIndex=-1); // 해당 인덱스에 <애니>를 추가 합니다 (-1 이면 끝에 추가 합니다)
	void DeleteSkeletonAni(int nIndex); // 해당 인덱스의 <애니 요소>를 삭제 합니다

	void GetAniDistance(GET_ANIDISTANCE_INFO *pInfo);
	void GetRootBonePos(int nAni, float fFrame,D3DXVECTOR4 *pVector);

	void SetAniFileName(const char *pFileName);
	const char *GetAniFileName();

	CBsBone **GetBoneList() { return m_ppBoneList; }
	int GetBoneNameList(char *szBoneName); // 본이름을 리스트로 받습니다 (char 256 단위)

	void GetRootBoneRotation( int nAniIndex, float fFrame, D3DXMATRIX *pRet );
	void GetRootBoneMatrix( int nAniIndex, float fFrame, D3DXMATRIX* pRet, int nCalcAniPosition );

	BSVECTOR GetRootBoneLocalPosition();

	void CalculateRootBonePosition();
	void DeleteAnimationKey( int nBoneIndex );


#ifdef _USAGE_TOOL_

	//											             저장 파일 이름 입니다
	//											             |
	static int MakeOneBa(char **ppFileName, int nFile, char *pStoreName); // nFile 개의 ba 를 한 개의 ba 로 만들어 줍니다 
	//					        |
	//                          nFile 개의 ba 파일 이름을 사용 합니다 
	int SaveBa( char *pStoreName );

#endif

#ifdef _LOAD_MAP_CHECK_
	int m_nLoadSize;
#endif

};

//------------------------------------------------------------------------------------------------

#endif

