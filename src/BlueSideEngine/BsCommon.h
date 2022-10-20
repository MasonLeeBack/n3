#pragma once

//------------------------------------------------------------------------------------------------
#ifdef _XBOX
	#define INV_Z_TRANSFORM
#endif

//------------------------------------------------------------------------------------------------

#define NEAR_ZERO				1e-12
#define SHADOW_LIMIT_FROMVIEW	6000.0f

#define MAX_TEXTURE_STAGE			16			// device에서 가져오는것으로 변경해야함!!
#define MAX_BONEMATRIX_ARRAY		50		// device에서 가져오는것으로 변경해야함!!

#define BS_TRANSFORM_VIEW			D3DTS_VIEW
#define BS_TRANSFORM_PROJECTION		D3DTS_PROJECTION
#define BS_TRANSFORM_WORLD			D3DTS_WORLD

//*************************************************************************
//				Important Define!!
#define BS_MAX_TECHNIQUE				8
#define BS_MAX_BONELINK_TECHNIQUE		5
//*************************************************************************

#define MAX_BONE_ANI_COUNT			5

#define MAX_UNSIGNED_SHORT			2<<15

#define BS_CALC_POSITION_X			0x00000001
#define BS_CALC_POSITION_Y			0x00000002
#define BS_CALC_POSITION_Z			0x00000004

enum BS_SHADOW_TYPE
{
	BS_SHADOW_NONE = 0,
	BS_SHADOW_BUFFER,
	BS_SHADOW_VOLUME,
	BS_SHADOW_IMPOSTER,
};

enum BS_CLIP_TYPE
{
	BS_CLIP_NONE = -1,
	BS_CLIP_TYPE0,
	BS_CLIP_TYPE1,
	BS_CLIP_TYPE2,
	BS_CLIP_TYPE_COUNT,
};

enum BS_UI_VIEWER_TYPE
{
	_UI_FULL_VIEWER	= -1,
	_UI_1P_VIEWER,
	_UI_2P_VIEWER,

	_UI_VIEWER_MAX,
};

enum BS_UI_MODE {
	_Ui_Mode_Debug,					// 디버깅 문자열을 출력 합니다 = 윈도우의 OutText()
	_Ui_Mode_Box,					// Texture가 없는 단순한 사각형
	_Ui_Mode_Image,					// ui 컨트롤들을 오려서 사용할수 있습니다
	_Ui_Mode_Text,					// 이미지 문자 입니다 (data/font.dds 를 사용합니다)
		
	//예외 처리
	_Ui_Mode_Exception,				// base의 render와 다른때에 그린다.ui 컨트롤들을 오려서 사용할수 있습니다(_Ui_Mode_Image역할)
	_Ui_Mode_Movie,					//
	_Ui_Mode_BGMovie,				// ui render시에 back ground movie를 먼저 그린다.
	_Ui_Mode_Box_ZTEST,				// box 중에 z가 필요하다.
	_Ui_Mode_ALPHA,					// alpha map
	_Ui_Mode_Mesh,					// Mesh를 그린다.
	_Ui_Mode_Mesh_Gauge,			// Mesh로 구성된 Gauge를 그린다.
	_Ui_Mode_Mesh_Gauge_Volumn,		// Mesh로 구성된 Gauge를 그린다.(tex3d)


	_Ui_Mode_Max,					// 엘리먼트 MAX 값 입니다 ----------------------------
};

enum _Ui_Exception_TYPE
{
	_Ui_Exception_NONE = -1,

	_Ui_Exception_MINIMAP_SOURCE,
	_Ui_Exception_MINIMAP_ALPHA,

	_Ui_Exception_MAX,
};


enum BS_PRIMITIVE_TYPE
{
	BS_PT_POINTLIST,
	BS_PT_LINELIST,
	BS_PT_LINESTRIP,
	BS_PT_TRIANGLELIST,
	BS_PT_TRIANGLESTRIP,
	BS_PT_TRIANGLEFAN,
	BS_PT_QUADLIST,
	//BS_PT_FORCE_DWORD,  삭제 예정
};


enum BS_DECLTYPE { 
    BS_TYPE_FLOAT1 = 0,	// = 값이 차례되로 있어야 합니다	   	   
    BS_TYPE_FLOAT2 = 1,	// =	   
    BS_TYPE_FLOAT3 = 2, // =      
    BS_TYPE_FLOAT4 = 3,	// = 
    BS_TYPE_D3DCOLOR = 4,
    BS_TYPE_UBYTE4 = 5,
    BS_TYPE_SHORT2 = 6,	// = 값이 차례되로 있어야 합니다	   	   
    BS_TYPE_SHORT4 = 7,	// = 
    BS_TYPE_UBYTE4N = 8,
    BS_TYPE_SHORT2N = 9,
    BS_TYPE_SHORT4N = 10,
    BS_TYPE_USHORT2N = 11,
    BS_TYPE_USHORT4N = 12,
    BS_TYPE_UDEC3 = 13,
    BS_TYPE_DEC3N = 14,
    BS_TYPE_FLOAT16_2 = 15,
    BS_TYPE_FLOAT16_4 = 16,
	BS_TYPE_HEND3N = 17,			// Xenon Extension
    BS_TYPE_UNUSED = 18,
};

enum BS_DECLUSAGE {	
    BS_USAGE_POSITION = 0,	   
    BS_USAGE_BLENDWEIGHT = 1,  
    BS_USAGE_BLENDINDICES = 2, 
    BS_USAGE_NORMAL = 3,	   
    BS_USAGE_PSIZE = 4,
    BS_USAGE_TEXCOORD = 5,	   
    BS_USAGE_TANGENT = 6,      
    BS_USAGE_BINORMAL = 7,	   
    BS_USAGE_TESSFACTOR = 8,
    BS_USAGE_POSITIONT = 9,
    BS_USAGE_COLOR = 10,
    BS_USAGE_FOG = 11,
    BS_USAGE_DEPTH = 12,
    BS_USAGE_SAMPLE = 13
};

//------------------------------------------------------------------------------------------------

struct BM_FILE_HEADER
{
	enum  { eFileID=5293, eVersion=15, eLOD=5, eLODOffset=4*4, };

	int		nOffset;
	int		nFileID; 
	int		nVersion; 
	int		nSubMeshCount;	
	int     nLOD;   // 1,5 (파일로 개별적으로 접근할때 eLODOffset 을 사용 합니다)  
	int     nUVAni; // 0,1

	void Set(int nCount) {
		nOffset=sizeof(BM_FILE_HEADER);
		nFileID=eFileID;
		nVersion=eVersion; 
		nSubMeshCount=nCount;
		nLOD=eLOD;
		nUVAni=0;
	}
};

struct BM_SUBMESH_HEADER
{
	//											 탄젠트, 바이노말 스페이스를 만들어 줍니다 (범프가 있는 서브메쉬는 자동 설정 됩니다)
	//											 |
	//											 ------------------
	enum { ePHYSIQUE=0x01, eBLEND_PHYSIQUE=0x02, eMAKE_TANGENT=0x04, }; 
	//	   ------------------------------------
	//	   |
	//     BM 과 BA 를 맥스에서 뽑을때 Blend 옵션을 서로 다르게 해서 뽑지 않도록 합니다 (주의)

	int					nOffset;
	int					nMeshFlag; // 여러 가지 정보들을 모아서 가지고 있습니다	(파일에서 읽을때 이부분 부터 nOffset-4 만큼 읽습니다)
	int					nTexChannelCount; // 채널 갯수 입니다
	BS_PRIMITIVE_TYPE	nPrimitiveType; 
	int					nVertexCount;	
	int					nFaceCount;	
	// nOffset==24 : Version Up

	enum { eStreamNumberMax=10, }; // <2004 12 21 이후 버전 호환>
	int					nStreamNumber; 
	int					nStreamType [eStreamNumberMax];
	int					nStreamUsage[eStreamNumberMax];
	// nOffset==24+84 : Version Up

	int                 nLODAllocated; // <2005 01 28 이후 버전 호환> m_nStreamLODAllocated (추가된 CBsStream 의 갯수 입니다)
	// nOffset==24+84+4 : Version Up

	D3DXMATRIX			matLinkDummy;
	// nOffset==24+84+4+16 : Version Up

	void Set() {
		memset(&nOffset, 0, sizeof(BM_SUBMESH_HEADER));
		nOffset=sizeof(BM_SUBMESH_HEADER);
	}
};

struct BM_MATERIAL_HEADER
{
	enum { eStage7=7, eChannel4=4, };
	
	enum {
	  eDI= 1, // diffuse
	  eSH= 3, // specular
	  eSI= 5, // self illuminat
	  eBU= 8, // bump
	  eDP=11, // displacement
	};

	char	szMaterialName[256]; // 버퍼
	float	fDiffuse[4];
	float	fAmbient[4];
	float	fSpecular[4];
	float	fPower;
	int		nTextureType[eStage7]; // 위에서 정의 된 값 참조 바랍니다
	char	szTextureName[eStage7][255]; // 버퍼
	char	cChannel[eStage7];
};

struct BM_PHYSIQUE_DATA
{
	int		nVertexIndex;
	int		nBone;
	float	fWeight;
};

#define BA_ANIMATION_KEYFRAME          0x00000001
#define BA_ANIMATION_SAMPLED_KEYFRAME  0x00000002 // 현재 사용 하고 있는 방식 입니다

struct BA_FILE_HEADER
{
	enum  { eFileID=9293, eVersion=12, };

	int		nOffset;
	int		nFileID; 
	int		nVersion; 
	int		nAniCount;
	int		nBoneCount;
	int		nAniType;

	void Set(int AniCount, int BoneCount) {
		nOffset=sizeof(BA_FILE_HEADER);
		nFileID=eFileID;
		nVersion=eVersion;
		nAniCount=AniCount;
		nBoneCount=BoneCount;
		nAniType=BA_ANIMATION_SAMPLED_KEYFRAME;
	}
};

//------------------------------------------------------------------------------------------------
// Effect Parameter Infomation
//------------------------------------------------------------------------------------------------

enum PARAMETER_TYPE {
	PARAM_WORLD = 0,				// Object's world matrix
	PARAM_WORLDMATRIXARRAY,			// Animated Object's world matrices => Not Supported!!
	PARAM_VIEW,						// Kernel's view matrix
	PARAM_VIEWINVERSE,				// Kernel's viewinverse matrix
	PARAM_VIEWINVERSETRANSPOSE,		// Kernel's viewinverse transpose matrix
	PARAM_PROJECTION,				// Kernel's projection matrix
	PARAM_WORLDVIEW,				// Object's worldview matrix
	PARAM_WORLDVIEWINVERSETRANSPOSE,// Object's Worldviewinverse transpose matrix
	PARAM_VIEWPROJECTION,			// Kernel's viewprojection matrix
	PARAM_WORLDVIEWPROJECTION,		// Object's worldviewprojection matrix
	PARAM_WORLDINVERSETRANSPOSE,	// Object's world inverse transpose matrix
	PARAM_LIGHTDIFFUSE,				// Kernel's Light Diffuse Color
	PARAM_LIGHTSPECULAR,			// Kernel's Light Specular Color
	PARAM_LIGHTAMBIENT,				// Kernel's Light Ambient Color
	PARAM_CAMERAPOSITION,			// Kernel's cameraposition
	PARAM_LIGHTDIRECTION,			// Kernel's light direction( Directional Light )
	PARAM_VIEW_LIGHTDIRECTION,		// Kernel's light direction in View Space	==> 최적화필요(뷰공간 라이트 방향은 한번만 계산가능)
	PARAM_POINTLIGHTNUM,					
	PARAM_POINTLIGHT_INFO,					
	PARAM_POINTLIGHT_COLOR,

	PARAM_TIME,						// Kernel's Time(Scalar)

	PARAM_FOGFACTOR,				// fogEnd, fogRange
	PARAM_FOGCOLOR,					// r,g,b

	PARAM_SHADOW_ALL,				// Kernel's Shadow All Matrix
	PARAM_SHADOW_NOWORLD_ALL,
	PARAM_SHADOW_NOVIEW_ALL,		// Kernel's Shadow No View, All matrix
	PARAM_SHADOW_VIEW_LIGHTDIRECTION,		// Shadow LightDirection ViewSpace
	PARAM_SHADOWMAP,				// Kernel's Shadow Map

	PARAM_WORLDLIGHTMAP,			// Kernel's LightMap
	PARAM_RCP_WORLDSIZE,			// Kernel's Rcp World Size

	PARAM_SCREENBUFFER,				// Kernel's Backbuffer Copy
	PARAM_REFLECTMAP,				// Kernel's ReflectMap
	
	// Custom Parameter!!
	PARAM_CUSTOM_DEFAULT,			// Dummy!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	PARAM_MATERIALAMBIENT,			// D3DVECTOR4
	PARAM_MATERIALDIFFUSE,			// D3DVECTOR4
	PARAM_MATERIALSPECULAR,			// D3DVECTOR4
	PARAM_SPECULARPOWER,			// Scalar
	PARAM_BUMPPOWER,				// Scalar
	PARAM_REFLECTPOWER,				// Scalar
	PARAM_UVANIMATION_POSITION,		// D3DVECTOR4 = 텍스쳐 애니메이션
	PARAM_UVANIMATION_ROTATION,		// D3DVECTOR4
	PARAM_UVANIMATION_SCALE,		// D3DVECTOR4
	PARAM_MORPH_WEIGHT,				// Scalar
	PARAM_EFFECT_CUSTOM0,			// Scalar
	PARAM_EFFECT_CUSTOM1,			// Scalar
	PARAM_EFFECT_CUSTOM2,			// Scalar
	PARAM_EFFECT_CUSTOM3,			// Scalar
	PARAM_NOISEPOWER,					// Scalar
	PARAM_WATERSPEED,						// Scalar	
	
};

enum TEXTURE_TYPE {
	TEXTURE_NORMAL = 0,
	TEXTURE_CUBE,
	TEXTURE_VOLUME,
};

//***************************************************************************************

struct EffectParam_Info
{
	D3DXHANDLE		m_hParam;
	PARAMETER_TYPE	m_type;
	DWORD		m_dwTechUseFlag;
};

enum VariableType	{ varInvalid=0, varBoolean, varFloat, varVector, varMatrix4x4, varColor };
struct CustomParam_Info
{
	CustomParam_Info(VariableType type) {
		m_nCustomParamIndex = -1;
		m_type = PARAM_CUSTOM_DEFAULT;
		m_DataType=type;
		m_bIsEdited = FALSE;
		switch(m_DataType) {
			case varBoolean:
				m_pValue=new BOOL; // 4 byte
				break;
			case varFloat:
				m_pValue=new D3DXVECTOR4; // D3DXFLOAT16(= WORD value) x,y,z,w
				break;
			case varVector:
				m_pValue=new D3DXVECTOR4; // D3DXFLOAT16(= WORD value) x,y,z,w
				break;
			case varMatrix4x4:
				m_pValue=new D3DXMATRIX; // float m[4][4]
				break;
			case varColor:
				m_pValue=new D3DXCOLOR; // float r,g,b,a
				break;
			default:
				m_pValue=NULL;
				break;
		}
	}
	~CustomParam_Info() {
		switch(m_DataType) {
			case varBoolean:
				delete (BOOL*)m_pValue;
				break;
			case varFloat:
				delete (D3DXVECTOR4*)m_pValue;
				break;
			case varVector:
				delete (D3DXVECTOR4*)m_pValue;
				break;
			case varMatrix4x4:
				delete (D3DXMATRIX*)m_pValue;
				break;
			case varColor:
				delete (D3DXCOLOR*)m_pValue;
				break;
			default:
				m_pValue=NULL;
		}
	}
	int				m_nCustomParamIndex;
	PARAMETER_TYPE	m_type;
	VariableType	m_DataType;
	LPVOID			m_pValue;
	BOOL			m_bIsEdited;
};

struct EffectSampler_Info
{
	int				m_nSamplerIndex;
	D3DXHANDLE		m_hParam;
	TEXTURE_TYPE	m_type;
};

struct ITERATE_SAVE
{
	D3DXVECTOR3 Position;
	D3DXVECTOR3 Direction;
};
//***************************************************************************************

enum LIGHT_TYPE {
	DIRECTIONAL_LIGHT = 0,
	POINT_LIGHT,
	SPOT_LIGHT,
};

//------------------------------------------------------------------------------------------------
// Message Definition
//------------------------------------------------------------------------------------------------

#define BS_PARTICLE_PROCESS 0x80000000 // CBsKernel::SendMessage() 에서 사용 합니다
#define BS_KERNEL_PROCESS   0x10000000


// Object Message
#define BS_REINIT_OBJECT					10		// Reinitialize Object!!										커널 재세팅
#define BS_SHADOW_CAST						11		// 0=NONE,1=BUFFER,2=VOLUME,3=IMPOSTER
#define BS_ENABLE_OBJECT_CULL				12

#define BS_GET_SKIN_INDEX					40
#define BS_GET_OBJECT_MAT					50
#define BS_ENABLE_OBJECT_ALPHABLEND			51		// dwParam1 = Boolean
#define BS_SET_OBJECT_ALPHA					52		// dwParam1 = Alpha Weight(between 0 and 1)
#define BS_RESTORE_OBJECT_ALPHABLEND		53		// No Parameter
#define BS_GET_USE_TEXTURE_NAME				54		// dwParam1 = nSkinIndex dwParam2 = ptr std::vector< std::string >

#define BS_SET_POINTSPRITE					60		// dwParam1 = SubMesh Index, dwParam2 = Boolean

#define BS_GET_SUBMESH_COUNT				100		// No Parameter													섭메쉬 갯수
#define BS_GET_LOD_COUNT  				    101		// No Parameter													LOD 갯수 =  1..
#define BS_DEC_LOD_LEVEL  				    102		// No Parameter													LOD 레벨 = -1..N-2
#define BS_INC_LOD_LEVEL  				    103		// No Parameter													LOD 레벨 = -1..N-2

#define BS_GET_UV_ANI     					104		// dw1=SubMesh Index											UV 애니가 있다면 -1 이 아닌 <커스텀 패러미터 UV 포지션> 의 인덱스를 리턴 합니다 = 0..N-1
#define BS_SET_UV_ANI_FRAME					105		// dw1=SubMesh Index, dwParam2 = <float*> frame 0.0f~1.0f		UV 애니메이션 프레임을 지정 합니다 (float* 로 변수를 넘겨 주세요)
						
#define BS_GET_BOOKING_COUNT				106		// No Parameter            return (int         )                Dummy갯수 얻기 입니다
#define BS_GET_BOOKING_POSITION             107		// dwParam1=BookingIndex    return (D3DXVECTOR3*)                Get Dummy Position 
#define BS_GET_COLLISION_COUNT				108		// No Parameter            return (int         )                Dummy갯수 얻기 입니다
#define BS_GET_COLLISION_POSITION           109		// dwParam1=CollisionIndex  return (D3DXVECTOR3*)                Get Dummy Position 
#define BS_GET_LINKDUMMY_COUNT				300		// No Paramter
#define BS_GET_LINKDUMMY_MATRIX				301		// dwParam1=LinkDummyIndex , return Dummy Matrix

#define BS_SHOW_SUBMESH						110		// dwParam1 = SubMesh Index, dwParam2 = Show/Hide(Boolean)
#define BS_SHOW_BOUNDINGBOX					111		// dwParam1 = Show/Hide(Boolean), dwParam2= DWORD color
#define BS_SHOW_GRID					    112		// dwParam1 = Show/Hide(Boolean)

#define BS_SET_SUBMESH_MATERIAL				113		// dw1=SubMesh Index,	dw2=Material Index						섭메쉬 매트리얼 세팅
#define BS_GET_SUBMESH_NAME					114		// dw1=SubMesh Index											섭메쉬 이름
#define BS_GET_SUBMESH_BONE_COUNT			115		// dw1=SubMesh Index											본 갯수
#define BS_GET_SUBMESH_BONE_NAME			116		// dw1=SubMesh Index,	dw2=Bone Index							본 이름
#define BS_GET_SUBMESH_BONE_LINK            117     // dw1=SubMesh Index                                            섭메쉬 본 블렌드 갯수
#define BS_GET_SUBMESH_PRIMITIVE_COUNT		118		// dw1=SubMesh Index											서브 메쉬의 폴리곤(프리미티브) 갯수
#define BS_GET_SUBMESH_PRIMITIVE_TYPE		119		// dwParam1 = SubMesh Index


#define BS_GET_SUBMESH_MATERIAL_INDEX		120		// dw1=SubMesh Index											해당 서브 메쉬의 매트리얼 인덱스
#define BS_GET_SUBMESH_MATERIAL_NAME		121     // dw1=SubMesh Index											해당 서브 메쉬의 매트리얼 이름
#define BS_GET_SUBMESH_CUSTOM_PARAMETERS	122		// dw1=SubMesh Index, dw2=std::vector<SEM_INFO>*, return N
	struct SEM_INFO {
		char           m_name[32];
		PARAMETER_TYPE m_type;
		D3DXMATRIX     m_value;	// float m[4][4];
		//			   |
		//			   float        : m[0][0]
		//			   float4 color : m[0]    = m[0][0] m[0][1] m[0][2] m[0][3]
		//             matrix       : m
	};

#define BS_GET_MESH_POINTER 				123		// No Parameter													메쉬 포인터를 넘겨 줍니다
#define BS_GET_MESH_PRIMITIVE_COUNT_LOD     124		// No Parameter													메쉬에서 사용된 삼각형 갯수를 현재 LOD 기준으로 넘겨 줍니다

#define BS_SET_SUBMESH_ALPHABLENDENABLE		600		// dw1=SubMesh Index,	dw2=Boolean
#define BS_SET_SUBMESH_ALPHA				601		// dw1=SubMesh Index,	dw2=alpha(float)
#define BS_GET_SUBMESH_ALPHABLENDENABLE		602		// dw1=SubMesh Index											해당 서브 메쉬의 알파블랜드 여부 리턴(Boolean)
#define BS_GET_SUBMESH_ALPHA				603		// dw1=SubMesh Index											해당 서브 메쉬의 알파값(float) 리턴
#define BS_SET_SUBMESH_TWOSIDEENABLE		604		// dwParam1 = SubMesh Index, dwParam2 = Boolean
#define BS_GET_SUBMESH_TWOSIDEENABLE		605		// dwParam1 = SubMesh Index
#define BS_SET_SUBMESH_ALPHA_ALIGN_ENABLE	606		// dwParam1 = SubMesh Index, dwParam2 = Boolean
#define BS_GET_SUBMESH_ALPHA_ALIGN_ENABLE	607		// dwParam1 = SubMesh
#define BS_SET_SUBMESH_BLENDOP				608		// dwParam1 = SubMesh Index, dwParam2 = BLENDOP
#define BS_SET_SUBMESH_SRCBLEND				609		// dwParam1 = SubMesh Index, dwParam2 = SRCBLEND
#define BS_SET_SUBMESH_DESTBLEND			610		// dwParam1 = SubMesh Index, dwParam2 = DESTBLEND
#define BS_SET_SUBMESH_DEPTHCHECKENABLE		611		// dwParam1 = SubMesh Index, dwParam2 = Boolean


#define BS_SET_SAMPLER						140		// dw1=SubMesh Index,	dw2=Sampler Index, dw3=Texture Index	섭메쉬 샘플러 세팅
#define BS_GET_SAMPLER_COUNT				141		// dw1=SubMesh Index											해당 서브 메쉬의 텍스쳐(샘플러) 갯수
#define BS_GET_SAMPLER_TEXTURE_INDEX		142		// dw1=SubMesh Index,	dw2=Sampler Index						해당 서브 메쉬의 해당 샘플러의 텍스쳐 인덱스
#define BS_GET_SAMPLER_TEXTURE_NAME			143		// dw1=SubMesh Index,	dw2=Sampler Index						해당 서브 메쉬의 텍스쳐 이름
#define BS_GET_SAMPLER_SEMANTIC_NAME		144     // dw1=SubMesh Index,	dw2=Sampler Index						해당 서브 메쉬의 해당 샘플러의 텍스쳐 타입 문자열 (fx 참조) = diffuse, specular
#define BS_GET_SAMPLER_TEXTURE_TYPE			145		// dwParam1=SubMesh Index, dwParam2=Sampler Index


#define BS_GET_BOUNDING_BOX					150		// No Parameter 												메쉬의 바운딩 박스 AABB* 를 리턴 합니다
#define BS_SET_BOUNDING_BOX_BM				151		// No Parameter 												메쉬의 바운딩 박스 수정
#define BS_WRITE_BOUNDING_BOX				153		// dw1=File Name 												메쉬의 바운딩 박스만 파일에 저장 합니다

#define	BS_SET_CUSTOM_RENDERER				161		// dwParam1=CallBackFunc dwParam2=Ptr of CallObject
#define BS_REFRESH_CAMERA					162		// No Parameter
 
// Custom Effect Parameter Message
#define BS_ADD_EDITABLE_PARAMETER			200		// dwParam1=SubMesh Index, dwParam2=Parameter Type, return index
#define BS_SET_EDITABLE_PARAMETER			201		// dwParam1=SubMesh Index, dwParam2=Editable Param Index, dwParam3=Input Data
#define BS_GET_EDITABLE_PARAMETER			202		// dwParam1=SubMesh Index, dwParam2=Editable Param Index, dwParam3=Output Data

// AniObject Message
#define BS_USE_ANI_CACHE					497		// dwParam=bool
#define BS_ANI_FILE_INDEX					499
#define BS_GETANILENGTH						500		// dwParam1=Ani Number
#define BS_GETANIDISTANCE					501		// dwParam1=ptr GET_ANIDISTANCE_INFO
	struct GET_ANIDISTANCE_INFO{
		int nAni1;
		int nAni2;
		float fFrame1;
		float fFrame2;
		D3DXVECTOR3 *pVector;
	};
#define BS_GETANICOUNT						502		// none
#define BS_SETCURRENTANI					503		// dw1=Ani Number,		dw2=Ani Frame
#define BS_BLENDANI							504		// dw1=Blend Ani Number,dw2=Ani Frame,     dw3=Blend Weight
#define BS_SETCURRENTANI_BONE				505		// dw1=ptr AniStruct
	struct SET_ANI_INFO{
		int nAniIndex;
		float fFrame;
		int nBoneIndex;
	};
#define BS_BLENDANI_BONE					506		// dw1=ptr BlendStruct
	struct BLEND_ANI_INFO{
		int nAniIndex;
		float fFrame;
		float fWeight;
		int nBoneIndex;
	};
#define BS_GET_BONE_NAME_LIST				507  	// dw1=char[MAX][256]						                    애니메이션에 저장된 본 리스트 = 리턴 값은 카운트 입니다
#define BS_SET_CALC_ANI_POSITION			508		// dw1=Postion Calc flag
#define BS_GET_BOX3							509		// dw1 = return ptr Box3

#define BS_LINKOBJECT_NAME					510		// dwParam1=BoneName nParam2 Object ID
#define BS_LINKOBJECT						511		// dwParam1=Bone Index nParam2 Object ID
#define BS_UNLINKOBJECT						512		// dwParam1=Object ID  dwParam2=unlink velocity
#define BS_UNLINKOBJECT_BONE				513		// dwParam1=Bone ID

#define BS_LINKPARTICLE_NAME				514		// dwParam1=BoneName nParam2 Object ID
#define BS_LINKPARTICLE						515		// dwParam1=Bone Index nParam2 Object ID
#define BS_UNLINKPARTICLE					516		// dwParam1=Particle ID
#define BS_UNLINKPARTICLE_BONE				517		// dwParam1=Bone ID

#define BS_REFRESH_ANI						519		// none
#define BS_GET_BONE_MATRIX					520		// dwParam1=Bone name
#define BS_GET_BONE_INDEX					521		// dwParam1=Bone Name
#define BS_SET_RAGDOLL_CATCH_BONE			522		// dwParam1=Bone Index
#define BS_GET_BONE_WORLD_MATRIX			523		// dwParam1 = Bone Name

#define BS_GET_ROOT_BONE_ROTATION			530		// dwParam1=Ani Index dwParam2=ptr Frame dwParam3=ptr D3DXMATRIX
#define BS_GET_ROOT_BONE_MATRIX				531		// dwParam1=Ani Index dwParam2=ptr Frame dwParam3=ptr D3DXMATRIX

#define BS_GET_BONECOUNT					532		// none
#define BS_GET_BONENAME						533		// dwParam1=Bone Index

#define BS_ENABLE_BILLBOARD					534		// dwParam1=Boolean
#define BS_SET_BILLBOARD_INDEX				535		// dwParam1=BillBoard Index

#define BS_GET_RAGDOLL_POS_VELOCITY			536		// dwParam1=ptr D3DXVECTOR3(out), dwParam2=ptr D3DXVECTOR3(out)
#define BS_SET_CLIPPING_TYPE				537		// dwParam1=BS_CLIP_TYPE

#define BS_ENABLE_LOD						538		// dwParam1=Boolean

// Physics Message
#define BS_PHYSICS_LINK_CHARACTER		550
#define BS_PHYSICS_ADDFORCE					551
#define BS_PHYSICS_COLLISIONMESH			552
#define BS_PHYSICS_RAGDOLL					553
#define BS_PHYSICS_SIMULATION				554
#define BS_PHYSICS_MAKE_SLEEP				555
#define BS_PHYSICS_HIDE_CRUSH_ACTOR	556
#define BS_PHYSICS_SET_ACTOR_MATRIX	557

// BoneRotation Message
#define BS_SET_BONE_ROTATION				560		// dwParam1=BoneName dwParam2=Rotation Vector( 각도를 넘겨야함.. 라디안 아님.. )
#define BS_SET_BONE_ROTATION_PROCESS		561		// dwParam1=BoneName dwParam2=Rotation Vector( 각도를 넘겨야함.. 라디안 아님.. )
#define BS_SET_CLEAR_BONE_ROTATION			562		// 

// Kernel Message
#define BS_GET_TEXTURESIZE				   1010		// dw1=Texture Index,   dw2=SIZE*
#define BS_GET_FX_NAME_LIST				   1011		// dw1=Search String,   dw2=char[MAX][256]						FX 이름 리스트 = 리턴 값은 카운트 입니다
#define BS_SET_LIGHT_DIFFUSE			   1020		// dw1=Diffuse
#define BS_SET_LIGHT_SPECULAR			   1021		// dw1=Specular
#define BS_SET_LIGHT_AMBIENT			   1022		// dw1=Ambient
#define BS_GET_LIGHT_DIFFUSE			   1023		// none
#define BS_GET_LIGHT_SPECULAR			   1024		// none
#define BS_GET_LIGHT_AMBIENT			   1025		// none

#define BS_SET_POINTLIGHT_INFO				1026		// dw1=PointLightInfo
#define BS_GET_POINTLIGHT_INFO				1027		// none
#define BS_GET_POINTLIGHT_COLOR			1028
#define BS_SET_POINTLIGHT_RANGE			1029		// dw1=float
#define BS_SET_POINTLIGHT_COLOR			1030		// dw1=d3dcolorvalue*
//------------------------------------------------------------------------------------------------

struct std_delete{
	template< typename T >
	void operator()( T * ptr ) const{
		if( ptr ){
			delete ptr;
			ptr=NULL;
		}
	}
};

struct std_delete_array{
	template< typename T >
		void operator()( T * ptr ) const{
			if( ptr ){
				delete [] ptr;
				ptr=NULL;
			}
		}

};

#ifndef SAFE_DELETE
#define SAFE_DELETE( p ) { if(p) {delete p; p=NULL;} }
#endif //SAFE_DELETE

#ifndef SAFE_DELETE_PVEC
#define SAFE_DELETE_PVEC( vec ) {							\
	std::for_each( vec.begin(), vec.end(), std_delete() );	\
	vec.clear();											\
	vec.swap( vec );										\
}
#endif //SAFE_DELETE_PVEC


#ifndef SAFE_DELETE_VEC
#define SAFE_DELETE_VEC( vec ) {							\
	vec.clear();											\
	vec.swap( vec );										\
}
#endif //SAFE_DELETE_VEC



#ifndef SAFE_DELETE_ARRAY_PVEC
#define SAFE_DELETE_ARRAY_PVEC( vec ) {							\
	std::for_each( vec.begin(), vec.end(), std_delete_array() );	\
	vec.clear();											\
	vec.swap( vec );										\
}
#endif // SAFE_DELETE_ARRAY_PVEC

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#endif //SAFE_RELEASE

#ifndef SAFE_RELEASE_MATERIAL
#define SAFE_RELEASE_MATERIAL(p)      { if(p!=-1) { g_BsKernel.ReleaseMaterial(p); p=-1; } }
#endif //SAFE_RELEASE_MATERIAL

#ifndef SAFE_RELEASE_TEXTURE
#define SAFE_RELEASE_TEXTURE(p)      { if(p!=-1) { g_BsKernel.ReleaseTexture(p); p=-1; } }
#endif //SAFE_RELEASE_TEXTURE

#ifndef SAFE_RELEASE_VD
#define SAFE_RELEASE_VD(p)      { if(p!=-1) { g_BsKernel.ReleaseVertexDeclaration(p); p=-1; } }
#endif //SAFE_RELEASE_VD

#ifndef SAFE_RELEASE_FX
#define SAFE_RELEASE_FX(p)      { if(p!=-1) { g_BsKernel.ReleaseFXTemplate(p); p=-1; } }
#endif //SAFE_RELEASE_FX

#ifndef SAFE_RELEASE_SKIN
#define SAFE_RELEASE_SKIN(p)	{ if(p!=-1) { g_BsKernel.ReleaseSkin(p); p=-1;	} }
#endif // SAFE_RELEASE_SKIN

#ifndef SAFE_RELEASE_ANI
#define SAFE_RELEASE_ANI(p)		{ if(p!=-1) { g_BsKernel.ReleaseAni(p); p=-1; } }
#endif // SAFE_RELEASE_ANI

#ifndef SAFE_RELEASE_PARTICLE_GROUP
#define SAFE_RELEASE_PARTICLE_GROUP(p)	{ if(p!=-1) { g_BsKernel.ReleaseParticleGroup(p); p=-1;	} }
#endif // SAFE_RELEASE_PARTICLE_GROUP

#ifndef SAFE_RELEASE_FXTEMPLATE
#define SAFE_RELEASE_FXTEMPLATE(p)		{ if(p!=-1) { g_BsKernel.ReleaseFXTemplate(p); p=-1; } }
#endif // SAFE_RELEASE_FXTEMPLATE

#ifndef SAFE_DELETEA
#define SAFE_DELETEA(p) if(p) { delete []p; p = NULL; }
#endif //SAFE_DELETEA

#define BsMax(a,b)            (((a) > (b)) ? (a) : (b))
#define BsMin(a,b)            (((a) < (b)) ? (a) : (b))



#ifndef SAFE_FX_OPERATE
	#ifdef _LTCG	
		#define SAFE_FX_OPERATE(func,Index,rtti)	\
		{\
			func;\
		}
	#else	
		#define SAFE_FX_OPERATE(func,Index,rtti)	\
		{\
			int	nRtti=-1;\
			g_pFcFXManager->SendMessage(Index,FX_GET_RTTI,(DWORD)&nRtti);\
			BsAssert( rtti == nRtti && "fx팀 프로그래머에게 연락부탁. 바로 멈춥니다." );\
			if( rtti != nRtti ){\
				DebugString( "여기 걸리면 fx팀 프로그래머에게 이야기좀 해주세요. Get Rtti, %d :: 기존 Rtti: %d \n", nRtti, rtti);\
				_DEBUGBREAK;\
			}\
			func;\
		}
	#endif //_LTCG
#endif //SAFE_FX_OPERATE



#ifndef SAFE_DELETE_FX
	#ifdef _LTCG	
		#define SAFE_DELETE_FX(p,rtti)      { if(p!=-1) { g_pFcFXManager->SendMessage(p, FX_DELETE_OBJECT);; p=-1; } }
	#else	
		#define SAFE_DELETE_FX(p,rtti)      { if(p!=-1) { g_pFcFXManager->SendMessage(p, FX_DELETE_OBJECT);; p=-1; } }
	#endif //_LTCG

#endif //SAFE_RELEASE_FX

#ifdef _LTCG
	#define _DEBUGBREAK	NULL
#else
	#define _DEBUGBREAK __debugbreak();
#endif
