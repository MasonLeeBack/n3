#ifndef __FC_FXPLAY_SIMEPLE_H__
#define __FC_FXPLAY_SIMEPLE_H__
#include "FcFxBase.h"
#include "CrossVector.h"
#include "SmartPtr.h"

class CFcGameObject;
typedef CSmartPtr<CFcGameObject> GameObjHandle;

struct PARTICLE_BY_PARENT
{
    int nParticleHandle;
    GameObjHandle hParent;
    D3DXVECTOR3 vecOffset;
};

struct PARTICLE_BY_PARENT_PLAY_FOR
{
    int nParticleID;
    bool bLoop;
    bool bIterate;
    int nParticleHandle;
    float fScale;
    GameObjHandle hParent;
    D3DXVECTOR3 *vecOffset;
};

struct FX_BY_PARENT_PLAY_FOR
{
	int nFxID;
	GameObjHandle hParent;
	CCrossVector *pCross;
	int nBoneIdx;
};

struct PARTICLE_BY_PARENT;
struct PARTICLE_BY_PARENT_PLAY_FOR;


// 툴에서 만든 Fx들을 외부 리스트에 넣고 단순 플레이 시키기 위해 만든 클래스
class CFcFxSimplePlayInfo
{
public:
    CFcFxSimplePlayInfo(int iHandle);	
	CFcFxSimplePlayInfo(int iHandle, GameObjHandle &Handle, int nBoneIndex);
	CFcFxSimplePlayInfo(int iHandle, GameObjHandle &Handle );
    ~CFcFxSimplePlayInfo();

    int	 GetHandle() {return m_Handle;}
    void Process();
    void BeginPlay( int nLoop );
    void SetCross(CCrossVector &Cross){m_Cross = Cross;}
    bool IsDestroy();
	bool SetDestroy();
	bool IsEqual(GameObjHandle &hParent, int nBoneIndex)
	{
		if(hParent == m_hParent && m_nBoneIndex == nBoneIndex)
			return true;
		return false;
	}



protected:
	int			m_nCreateType;
    CCrossVector m_Cross;
    int			m_Handle;
    int			m_nObjectIndex;
//	int			m_nParentObjectIndex;
	int			m_nBoneIndex;
	GameObjHandle m_hParent;
	D3DXVECTOR3 m_vCenter;
};


class CFcFXSimplePlay : public CFcFXBase
{
public:
    CFcFXSimplePlay();
    ~CFcFXSimplePlay();
    void Process();
	void Render(C3DDevice *pDevice);//{};
    void Load( char *szFileName, int *pUseTable );
    void PlayFx(int iHandle,CCrossVector *pCross, int nLoop);
	void PlayLinkedFx( int iHandle, GameObjHandle hParent, int nBoneIndex, int nLoop );
	void PlayFxByParent( int iHandle, GameObjHandle hParent, CCrossVector *pCross, int nLoop );
    void PlayParticleByParent( int nID, GameObjHandle hParent, bool bLoop, bool bIterate, float fScale , D3DXVECTOR3* pOffset=NULL);

	void StopLinkedFx( GameObjHandle hParent, int nBoneIndex);
	void Update();

protected:
    void AddFx( int nID, char *szFxFilePath, bool bLoad );

	char m_szFxFilePath[256];
    std::vector<int> m_FxTemplateList;
    std::list<CFcFxSimplePlayInfo*> m_FxObjectList;

    std::list<PARTICLE_BY_PARENT> m_ParticleList;		// Parent의 CrossVec에 영향 받는 녀석들만 넣는다.
    int ProcessMessage(int nCode,DWORD dwParam1=0, DWORD dwParam2=0,DWORD dwParam3=0);
protected:

	void ClearFxObjList();

};
#endif