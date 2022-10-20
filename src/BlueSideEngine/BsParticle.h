#pragma once

/*
2005. 9. 1 : ���� �м��ϸ鼭 �ڵ� ������ �߽��ϴ�. ������ ���� �ʾҽ��ϴ�. ���翵.
*/

#include "BsFXTable.h"

#define			PATICLE_TABLE_COUNT					21
#define			PARTICLE_MEMBER_PARAM_RESERVED		252
#define			BPF_FILE_HEADER_RESERVED			120
#define			BPF_FILE_HEADER_VERSION				12

class BStream;
class C3DDevice;

/*-----------------------------------------------------------------------------------
-
-		����ü ���� : PARTICLEVERTEX, ITERATE_PARTICLE_VERTEX, BPF_FILE_HEADER
-							Choi Jae Young						2005. 9. 1
-																v 0.01
-							In Blue-Side
-
-
-			note : Particle ���ο� ���Ǵ� ����ü.
-				   
-
------------------------------------------------------------------------------------*/
struct PARTICLEVERTEX						// ��Ʈ�����θ� ���ŵǾ� �� �� �ִ� ��ƼŬ(��Ʈ���� ������ shader �� const������)
{
	D3DXVECTOR3		Direction;
	float 			fBirthTime;
	short 			nVertexIndex;
	short			nRandRot;
};

struct ITERATE_PARTICLE_VERTEX				// lock, unLock �� ���ؼ� ���ŵǾ����� ��ƼŬ ( ���� �̳༮�� �׿� ����. )
{
	D3DXVECTOR3 	Direction;
	float			fBirthTime;
	short 			nVertexIndex;
	short 			nRandRot;
	D3DXVECTOR3 	Position;
	D3DXVECTOR3 	Direction2;
	
};

struct PARTICLE_MEMBER_PARAM
{
	int				nParticleCount;
	int				nLifeTime;
	int				nCreationTime;
	float			fLaunchSpeedMin;
	float			fLaunchSpeedMax;
	D3DXVECTOR4		LaunchVector;
	float			fGravityAccel;
	D3DXVECTOR3		GravityVec;
	float			fRotateStart;
	float			fRotateEnd;
	float			fResistTime;
	float			fResistRate;
	int				nTextureRepeat;
	int				nAlphaType;
	BOOL			bUseRandomRotation;
	int				nBlendOP;
	int				nSourceBlendMode;
	int				nDestBlendMode;
	BOOL			bIgnoreIterate;
	BOOL			bUseOffsetMap;
};
struct BPF_FILE_HEADER						// .bpf ���� ���� ���.
{
	int				nVersion;
	int				nMemberCount;
};
//*********************************************************************************************
//*********************************************************************************************






/*-----------------------------------------------------------------------------------
-
-				CBsParticleMember				Choi Jae Young
-												2005. 9. 1
-												v 0.01
-							In Blue-Side
-
-
-			note : Particle ���� Ŭ����.(������ ��)
-				   
-
------------------------------------------------------------------------------------*/
class CBsParticleMember
{
public:
	enum TABLE_TYPE 
	{
		COLOR_TABLE,
		ALPHA_TABLE,
		SCALE_TABLE,

		TABLE_AMOUNT,
	};

public:

	void						EnableDraw(bool bDraw)		{ m_bDraw=bDraw;				}		
	bool						IsDraw()					{ return m_bDraw;				}				// �׸��� �ִ°�? or �׷ȴ°�? or �׸����ΰ�?
	int 						GetParticleCount()			{ return m_Param.nParticleCount;}				
	int 						GetLifeTime()				{ return m_Param.nLifeTime;		}
	int 						GetTextureIndex()			{ return m_nTextureIndex;		}
	void						SetTextureIndex(int nIndex) { m_nTextureIndex=nIndex;		}					
	void 						SetParticleParam(PARTICLE_MEMBER_PARAM *pParam);
	float*						GetScaleTable()				{ return m_ScaleTable;			}
	D3DCOLORVALUE*				GetColorTable()				{ return m_ColorTable;			}
	float						RandomNumberInRange(float Min, float Max);
	CBsFXTable*					GetTable( TABLE_TYPE Type );												// Add by Siva // pTable�� �����ϴ���.. ���� m_ScaleTable, m_ColorTable�� ���� ��� ���ΰ�?
	void 						CreateParticleBuffer();
	void 						SetParticlePosition(int nSavePos, int nTick, D3DXMATRIX *pParticleMat);		// SavePos?? -_-;;
	void 						GetParticleIndex(int nTick, int &nIndex, int &nCount);
	void 						CopyVertexPosition(int nSavePos);											// �뵵�� ����?
	void 						RenderParticle(C3DDevice *pDevice, int nTick, int nStopTick, bool bLoop, bool bIterate, float fScale);	// CBsParticleGroup ���� ���
	void 						LoadTexture( const char *pFileName );										// CBsParticleGroup ���� ���
	void 						LoadParticleMember(BStream* stream, const char *pFileName, int nVersion = BPF_FILE_HEADER_VERSION );
	void 						SaveParticleMember(BStream* stream);
	void 						GenerateTable();
	void 						ReleaseTexture();
	void						Clear();																	// CBsParticleGroup �� ���������� ���
	void 						GetParticleParam(PARTICLE_MEMBER_PARAM *pParam);							// CBsParticleGroup ���� ���// �Ʒ��Լ��� ��������?
	PARTICLE_MEMBER_PARAM*		GetParticleParam();															// CBsParticleGroup ���� ���



    

    
protected:

	bool						m_bDraw;
	int							m_nTextureIndex;													
	PARTICLE_MEMBER_PARAM		m_Param;															
	CBsFXTable*					m_pTable[TABLE_AMOUNT];														// �Ʒ� table���� �����..
	D3DCOLORVALUE				m_ColorTable[PATICLE_TABLE_COUNT];
	float						m_ScaleTable[PATICLE_TABLE_COUNT];
	std::vector< D3DXVECTOR3 >	m_DirectionList;															// ��ƼŬ �ϳ��� ���� ���� �ƴѰ�? �׷��ٸ� ��ƼŬ ����ü�� dir����??
	std::vector<float>			m_BirthTimeList;															// ��ƼŬ ����ü�� birth���� �����??
	LPDIRECT3DVERTEXBUFFER9		m_pVB;
	LPDIRECT3DINDEXBUFFER9		m_pIB;
	ITERATE_PARTICLE_VERTEX*	m_pIterateVB;																// Iterate �� ���ؽ� ����.(?)
	WORD*						m_pIterateIB;






public:
	CBsParticleMember();
	~CBsParticleMember();
	

};
//*********************************************************************************************
//*********************************************************************************************






/*-----------------------------------------------------------------------------------
-
-				CBsParticleGroup				Choi Jae Young
-												2005. 9. 1
-												v 0.01
-							In Blue-Side
-
-
-			note : Particle ���� Ŭ����.(������ ��)
-				   
-
------------------------------------------------------------------------------------*/
class CBsParticleGroup
{
public:

	int 								GetTotalParticleCount()					{ return m_nTotalParticleCount; }
	int 								GetMaxParticleLife()					{ return m_nMaxParticleLife;	}
	int 								GetParticleMemberCount()				{ return (int)m_ParticleMembers.size(); }
	int 								GetParticleMemberLifeTime(int nIndex)	{ return m_ParticleMembers[nIndex]->GetLifeTime(); }
	void								EnableDraw(int nIndex, bool bDraw)		{ m_ParticleMembers[nIndex]->EnableDraw(bDraw); }
	void 								SetUseAlphaFog(bool bUse)				{ m_bUseAlphaFog=bUse; }
	void 								SetParticleFileName(const char *pFileName);
	const char*							GetParticleFileName();
	const char*							GetParticleFileFullName() { return m_szFileName.c_str(); }
	static void 						CreateEffect();
	static void 						DeleteEffect();
	void 								CreateParticleBuffer(int nIndex) { m_ParticleMembers[nIndex]->CreateParticleBuffer(); }
	void 								RecreateParticleBuffer();
	void 								RecalcTotalParticleCount();
	void 								SaveParticleGroup(BStream* stream);
	int									LoadParticleGroup(BStream* stream);
	int									AddParticleMember(PARTICLE_MEMBER_PARAM *pDefaultParam=NULL);
	int									CopyParticleMember(int nIndex);
	void 								DeleteParticleMember(int nIndex);
	void 								ChangeParticleMember(int nIndex1, int nIndex2);
	CBsParticleMember*					GetParticleMember(int nIndex);
	void 								DeleteAllParticleMember();
	void 								LoadTexture(int nIndex, const char *pFileName);
	std::vector<std::string>*			GetTextureNameList();
	void 								GetParticleParam(int nIndex, PARTICLE_MEMBER_PARAM *pParam) { m_ParticleMembers[nIndex]->GetParticleParam(pParam); }
	void 								SetParticleParam(int nIndex, PARTICLE_MEMBER_PARAM *pParam) { m_ParticleMembers[nIndex]->SetParticleParam(pParam); }
	float*								GetScaleTable(int nIndex) { return m_ParticleMembers[nIndex]->GetScaleTable(); }
	D3DCOLORVALUE*						GetColorTable(int nIndex) { return m_ParticleMembers[nIndex]->GetColorTable(); }
	CBsFXTable*							GetTable( int nIndex, CBsParticleMember::TABLE_TYPE Type ) { return m_ParticleMembers[nIndex]->GetTable( Type ); }
	static void							SetCommonParticleConstant();
	void 								SetParticleMemberConstant(float fScale, int nMemberIndex, PARTICLE_MEMBER_PARAM *pParam);
	void 								SetParticleGroupConstant(D3DXMATRIX *pMatParticle, D3DCOLORVALUE *pColor);
	void 								RenderParticle(C3DDevice *pDevice, D3DXMATRIX *pMatParticle, int nTick, int nStopTick, bool bLoop, bool bIterate, int nSavePos, D3DCOLORVALUE *pColor, float fScale);
	void 								SetParticlePosition(int nSavePos, int nTick, D3DXMATRIX *pParticleMat);
	void 								Clear();

	void								AddRef();
	int									Release();

	int									GetRefCount() { return m_nRefCount; }


protected:

	int									m_nRefCount;
	int									m_nTotalParticleCount;
	int									m_nMaxParticleLife;
	bool								m_bUseAlphaFog;

	static int							s_nNormalDeclIndex;
	static int							s_nIterateDeclIndex;
	static int							s_nParticleMaterialIndex;
	static D3DXVECTOR4 					s_VertexData[4];
	static D3DXVECTOR4 					s_ConstantValue;
	static D3DXVECTOR4 					s_UVTable[4];
	static D3DXVECTOR4 					s_SinTable[64];

	static D3DXHANDLE 					s_hView;
	static D3DXHANDLE 					s_hWorldView;
	static D3DXHANDLE 					s_hProjection;
	static D3DXHANDLE 					s_hParticleTime;
	static D3DXHANDLE 					s_hResistTime;
	static D3DXHANDLE 					s_hResistScale;
	static D3DXHANDLE 					s_hLifeTime;
	static D3DXHANDLE 					s_hGravityAccel;
	static D3DXHANDLE 					s_hGravityVec;
	static D3DXHANDLE 					s_hOrigin;

	static D3DXHANDLE 					s_hRotateStart;
	static D3DXHANDLE 					s_hRotateRange;
	static D3DXHANDLE 					s_hColorTable;
	static D3DXHANDLE 					s_hScaleTable;

	static D3DXHANDLE 					s_hParticleScale;
	static D3DXHANDLE 					s_hVertexOffset;
	static D3DXHANDLE 					s_hUVTable;
	static D3DXHANDLE 					s_hSinTable;
	static D3DXHANDLE 					s_hParticleColor;
	static D3DXHANDLE 					s_hTextureRepeat;
	static D3DXHANDLE 					s_hTexture;
	static D3DXHANDLE 					s_hTextureAlpha;

	static D3DXHANDLE 					s_hTechniqueNormal;
	static D3DXHANDLE 					s_hTechniqueIterate;
	static D3DXHANDLE 					s_hTechniqueNormalAlpha;
	static D3DXHANDLE 					s_hTechniqueIterateAlpha;

	std::vector<CBsParticleMember*>		m_ParticleMembers;
	std::vector<std::string>			m_szTextureNameList;
	std::string							m_szFileName;

public:

	CBsParticleGroup();
	~CBsParticleGroup();
	/*
	void SetColorKeyList(int nIndex, std::vector<PARTICLE_COLOR_KEY> &ColorKeyList) { m_ParticleMembers[nIndex]->SetColorKeyList(ColorKeyList); }
	std::vector<PARTICLE_COLOR_KEY> *GetColorKeyList(int nIndex) { return m_ParticleMembers[nIndex]->GetColorKeyList(); }
	void SetAlphaKeyList(int nIndex, std::vector<PARTICLE_ALPHA_KEY> &AlphaKeyList) { m_ParticleMembers[nIndex]->SetAlphaKeyList(AlphaKeyList); }
	std::vector<PARTICLE_ALPHA_KEY> *GetAlphaKeyList(int nIndex) { return m_ParticleMembers[nIndex]->GetAlphaKeyList(); }
	void SetScaleKeyList(int nIndex, std::vector<PARTICLE_SCALE_KEY> &ScaleKeyList)	{ m_ParticleMembers[nIndex]->SetScaleKeyList(ScaleKeyList); }
	std::vector<PARTICLE_SCALE_KEY> *GetScaleKeyList(int nIndex)	{ return m_ParticleMembers[nIndex]->GetScaleKeyList(); }
	*/
};
//*********************************************************************************************
//*********************************************************************************************


/*-----------------------------------------------------------------------------------
-
-				CBsParticleObject				Choi Jae Young
-												2005. 9. 1
-												v 0.01
-							In Blue-Side
-
-
-			note : Particle ���� Ŭ����.(������ ��)
-				   
-
------------------------------------------------------------------------------------*/
class CBsParticleObject
{
public:
	void					Delete() {	m_bDelete = true;	}
	bool					IsDelete()										{ return m_bDelete; }
	void 					SetUse(bool bUse)								{ m_bUse=bUse;		}
	bool 					GetUse()										{ return m_bUse;	}
	void 					ShowParticle(bool bShow)						{ m_bShow=bShow;	}
	bool 					IsShowParticle()								{ return m_bShow;	}
	void					SetLinked( bool bLinked ) { m_bLinked = bLinked; }
	bool					IsLinked() { return m_bLinked; }
	void 					PauseParticle(bool bPause)						{ m_bPause=bPause;	}
	bool 					IsPauseParticle()								{ return m_bPause;	}
	void 					SetIterate(bool bIterate)						{ m_bIterate=bIterate;	}
	bool 					GetIterate()									{ return m_bIterate;	}
	void 					SetLoop(bool bLoop)								{ m_bLoop=bLoop;	}
	bool 					GetLoop()										{ return m_bLoop;	}
	void 					SetAutoDelete( bool bFlag ) 					{ m_bAutoDelete = bFlag;}
	bool 					GetAutoDelete()									{ return m_bAutoDelete;	}
	void 					ResetParticleInfo();
	void 					RenderParticle(C3DDevice *pDevice);
	void					RenderParticleForTool();
	bool 					ProcessParticle();
	void 					SetIteratePosition();
	void 					SetParticleGroupPtr(CBsParticleGroup *pGroup);
	CBsParticleGroup*		GetParticleGroupPtr()							{ return m_pParticleGroup; }

	void					SetParticleTick( int nTick )					{ m_nParticleTick = nTick; }
	int						GetParticleTick()								{ return m_nParticleTick;	}

	void 					SetParticleMatrix(D3DXMATRIX *pMatParticle);
	void 					SetParticleScale(float fScale)					{ m_fScale=fScale; }
	float					GetParticleScale()								{ return m_fScale; }
	D3DXMATRIX*				GetParticleMatrix()								{ return &m_MatParticle;	}
	void					AdjustSaveIteratePosition(int nIndex, int nCount);
	int						GetSaveIteratePosition()						{ return m_nSavePositionIndex;		}
	void					SetSaveIteratePosition(int nPosition)			{ m_nSavePositionIndex=nPosition;	}
	int						GetTotalParticleCount()							{ return m_pParticleGroup->GetTotalParticleCount(); }
	void					StopPlay(int nStopTick);
	void					SetParticleColor(D3DCOLORVALUE *pColor)			{ m_Color=*pColor; }
	void					SetUseAlphaFog(bool bUse)						{ m_bUseAlphaFog=bUse;		}
	bool					IsUsingAlphaFog()								{ return m_bUseAlphaFog;	}
	void					UpdateParticle()								{ m_nParticleTick++;		}
	bool					UseCull() { return m_bUseCull; }
	void					SetUseCull( bool bUse ) { m_bUseCull = bUse; }

protected:
	int						m_nSavePositionIndex;
	int						m_nStopTick;
	int						m_nParticleTick;
	float					m_fScale;
	bool					m_bDelete;	// mruete: prefix bug 356: moved this down for better structure packing
	bool					m_bUse;
	bool					m_bShow;
	bool					m_bLinked;
	bool					m_bPause;
	bool					m_bIterate;
	bool					m_bLoop;
	bool					m_bUseAlphaFog;
	bool					m_bAutoDelete;
	bool					m_bUseCull;
	CBsParticleGroup*		m_pParticleGroup;
	D3DCOLORVALUE			m_Color;
	D3DXMATRIX				m_MatParticle;


public:
	CBsParticleObject();
	~CBsParticleObject();

	static	CBsCriticalSection	s_csSharedParticleData;
};