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
	CBsBone *m_pRootBone; // �޸�	= 1
	CBsBone **m_ppBoneList;	// �޸� = m_nBoneCount
	int *m_pTotalFrame;	// �޸� = m_nAniCount

	char m_szAniFileName[_MAX_PATH]; // ����

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

	int  AddSkeletonAni(CBsAni *pAni, int nIndex=-1); // �ش� �ε����� <�ִ�>�� �߰� �մϴ� (-1 �̸� ���� �߰� �մϴ�)
	void DeleteSkeletonAni(int nIndex); // �ش� �ε����� <�ִ� ���>�� ���� �մϴ�

	void GetAniDistance(GET_ANIDISTANCE_INFO *pInfo);
	void GetRootBonePos(int nAni, float fFrame,D3DXVECTOR4 *pVector);

	void SetAniFileName(const char *pFileName);
	const char *GetAniFileName();

	CBsBone **GetBoneList() { return m_ppBoneList; }
	int GetBoneNameList(char *szBoneName); // ���̸��� ����Ʈ�� �޽��ϴ� (char 256 ����)

	void GetRootBoneRotation( int nAniIndex, float fFrame, D3DXMATRIX *pRet );
	void GetRootBoneMatrix( int nAniIndex, float fFrame, D3DXMATRIX* pRet, int nCalcAniPosition );

	BSVECTOR GetRootBoneLocalPosition();

	void CalculateRootBonePosition();
	void DeleteAnimationKey( int nBoneIndex );


#ifdef _USAGE_TOOL_

	//											             ���� ���� �̸� �Դϴ�
	//											             |
	static int MakeOneBa(char **ppFileName, int nFile, char *pStoreName); // nFile ���� ba �� �� ���� ba �� ����� �ݴϴ� 
	//					        |
	//                          nFile ���� ba ���� �̸��� ��� �մϴ� 
	int SaveBa( char *pStoreName );

#endif

#ifdef _LOAD_MAP_CHECK_
	int m_nLoadSize;
#endif

};

//------------------------------------------------------------------------------------------------

#endif

