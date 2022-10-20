#ifndef __FC_INTERFACE_MISC_H__
#define __FC_INTERFACE_MISC_H__

#include "FcInterfaceDataDefine.h"
#include "FCinterfaceObj.h"

class CFcImageInfo;
#define MSG_SHOW_MAX_STR	1024


//----------------------------------------------------------------------------------------------------
class CFcFadeEffect : public CFcInterfaceObj
{
public:
    CFcFadeEffect();
    ~CFcFadeEffect();
    void SetFadeOut(int nSec,D3DXVECTOR3 *pColor = NULL);
    void SetFadeIn(int nSec);
    void Process();
	void Update();
	bool IsFadeInStop();
	bool IsFadeOutStop();
	bool IsFade()					{ return !m_bStop; }

protected:
    int         m_nTargetTick;
    int         m_nCurTick;
    float       m_fAlpha;
    bool        m_bFadeOut;
	int 		m_nSaveTick;
	bool		m_bStop;

    D3DXVECTOR3  m_Color;
    D3DVIEWPORT9 m_ViewPort;
};

class CFcMsgShow : public CFcInterfaceObj
{
public:
	CFcMsgShow();
	void SetMsgShowXY(int nX,int nY,int nSec,int nTextTableID);
	void Process();
	void Draw();
	bool IsMsgShowXYEnd();
protected:
	int			   m_nX,m_nY;
	int		       m_nCurSec;
	int			   m_nSaveProcessTick;
	int			   m_TargetSec;
	char		   m_szStr[MSG_SHOW_MAX_STR];
	bool		   m_bSoundUse;
	char		   m_szStrTemp[MSG_SHOW_MAX_STR];
	int			   m_nSoundHandle;
};


#endif
