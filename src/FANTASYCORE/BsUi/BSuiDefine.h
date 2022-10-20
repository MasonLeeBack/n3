#ifndef _BsUi_DEFINE_H_
#define _BsUi_DEFINE_H_

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define _TEXT_TYPE_LAYER		"BsUiTYPE_LAYER"
#define _TEXT_TYPE_BUTTON		"BsUiTYPE_BUTTON"
#define _TEXT_TYPE_LISTBOX		"BsUiTYPE_LISTBOX"
#define _TEXT_TYPE_LISTCTRL		"BsUiTYPE_LISTCTRL"
#define _TEXT_TYPE_TEXT			"BsUiTYPE_TEXT"
#define _TEXT_TYPE_IMAGECTRL	"BsUiTYPE_IMAGECTRL"
#define _TEXT_TYPE_SLIDER		"BsUiTYPE_SLIDER"

#define _TEXT_TYPE_CHECK		"BsUiTYPE_CHECKBUTTON"



/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

#define _UI_VERSION		"v0.001"

enum BsUiCMD
{
	//bsuitype
	BsUiCMD_BEGIN_LAYER					= 1,
	BsUiCMD_BEGIN_BUTTON,
	BsUiCMD_BEGIN_TEXT,
	BsUiCMD_BEGIN_LISTBOX,
	BsUiCMD_BEGIN_LISTCTRL,
	BsUiCMD_BEGIN_IMAGE,
	BsUiCMD_BEGIN_CHECKBUTTON,
	BsUiCMD_BEGIN_SLIDER,

	BsUiCMD_BEGIN_ATTACH					= 50,
	BsUiCMD_BEGIN_VALUE,

	//default
	BsUiCMD_SETCLASSID,
	BsUiCMD_SETSIZE,
	BsUiCMD_ATTRIBUTE,
	BsUiCMD_SETIMAGE,
	BsUiCMD_SET_UV,
	BsUiCMD_SET_UV_OLD,
	BsUiCMD_SET_BLOCK,
	BsUiCMD_SET_BLOCK_OLD,
	BsUiCMD_SETSTRING,
	BsUiCMD_BEGIN_ON						= 150,
	BsUiCMD_BEGIN_ONDEFAULT,

	//type layer
	BsUiCMD_SETPOS,
	BsUiCMD_ADDITEM,

	//type button
	BsUiCMD_SETCHECK,
	BsUiCMD_SETGROUP,
	
	//type list box
	BsUiCMD_SETLBTYPE,
	BsUiCMD_SETLINE,
	BsUiCMD_SETCOLUMN,
	BsUiCMD_LISTITEM,
	BsUiCMD_DRAWSELBOX,
	BsUiCMD_SET_SELBOXUV,
	BsUiCMD_SET_SELBOXUV_OLD,
	BsUiCMD_SET_SELBOXBLOCK,
	BsUiCMD_SET_SELBOXBLOCK_OLD,

	//type slider
	BsUiCMD_SETSDTYPE,
	BsUiCMD_SETSDORIENTATION,
	BsUiCMD_SETRANGE,
	BsUiCMD_SETBARSIZE,
	BsUiCMD_SET_BARUV,
	BsUiCMD_SET_BARUV_OLD,
	BsUiCMD_SETSDLINEUV,
	BsUiCMD_SETSDBASEUV,
	BsUiCMD_SET_BAR1UV,
	BsUiCMD_SET_BAR1UV_OLD,
	BsUiCMD_SET_BAR2UV,
	BsUiCMD_SET_BAR2UV_OLD,

	//type image
	BsUiCMD_FILENAME,
	BsUiCMD_IMG_LOCAL,
	BsUiCMD_UV,
	BsUiCMD_REALSIZE,
	BsUiCMD_REVERSE,
	BsUiCMD_ROTATE,
	
	//text
	BsUiCMD_TEXTID,
	BsUiCMD_TEXT_POS,
	BsUiCMD_FONT_ATTR,
	
	//global Class ID
	BsUiCMD_BEGIN_CLASS_GROUP			= 970,
	BsUiCMD_BEGIN_CLASS_ID				= 971,

	//gloabal Image UV
	BsUiCMD_BEGIN_TEXTURE_NAME			= 980,
	BsUiCMD_BEGIN_UV_ID					= 981,

	//global font attr
	BsUiCMD_BEGIN_FONTATTR_ID			= 990,
	BsUiCMD_BEGIN_FONTATTR_ID_OLD		= 991,

	BsUiCMD_END_BRACE					= 999,
	BsUiCMD_VERSION						= 1000,
};

enum _TEXT_ALIGN
{
	_ALIGN_LEFT = 100,
	_ALIGN_RIGHT,
	_ALIGN_CENTER,
};

//typedef int		BsUiBlockId;
enum _IMAGE_MODE
{	
	_IMAGE_MODE_NONE = -1,
	_IMAGE_MODE_DEFAULT,
	_IMAGE_MODE_UV,
	_IMAGE_MODE_BLOCK,
	_IMAGE_MODE_FILE,			//image ctrl만 사용
};


//_S_ : string
//_I_ : int
//_B_ : bool
enum _CTRL_PROPERTY_TYPE
{
	_CPT_NONE		= -1,

	//Misc----------------------------
	_CPT_S_NAME,			//CString
	_CPT_S_ID,				//short
	_CPT_S_TYPE,			//BsUiType

	//Attribute----------------------------
	_CPT_B_ENABLE,			//bool
	_CPT_B_MOVABLE,			//bool
	_CPT_B_SHOW,			//bool
	_CPT_B_DRAG_N_DROP,

	//pos & size ----------------------------
	_CPT_I_AX,
	_CPT_I_AY,
	_CPT_I_RX,
	_CPT_I_RY,
	_CPT_I_WIDTH,
	_CPT_I_HEIGHT,

	// image mode----------------------------
	_CPT_E_IMAGE_MODE,
	_CPT_BT_IMAGE_ACTUAL_SIZE,
	// uv
	_CPT_BT_SEL_IMAGE_UV,
	_CPT_BT_DEF_IMAGE_UV,
	_CPT_BT_DIS_IMAGE_UV,
	// block
	_CPT_E_SEL_IMAGE_BLOCK,
	_CPT_E_DEF_IMAGE_BLOCK,
	_CPT_E_DIS_IMAGE_BLOCK,
	
	//text----------------------------
	_CPT_I_TEXT_POS_X,
	_CPT_I_TEXT_POS_Y,
	_CPT_BT_TEXTTABLE,
	//font attribute
	_CPT_E_SELECT_FA,
	_CPT_E_DEFAULT_FA,
	_CPT_E_DISABLE_FA,

	//button ctrl ----------------------------
	_CPT_E_BUTTON_MODE,		//button mode
	_CPT_I_GROUPID,			//group id
	_CPT_B_CHECK,			//on/off state - (normal button / check button)

	// list ctrl ----------------------------
	_CPT_I_LINE,
	_CPT_I_COLUMN,
	_CPT_B_SEL_BOX,
	_CPT_BT_SEL_SB_UV,
	_CPT_BT_DEF_SB_UV,
	_CPT_BT_DIS_SB_UV,
	_CPT_E_SEL_SB_BLOCK,
	_CPT_E_DEF_SB_BLOCK,
	_CPT_E_DIS_SB_BLOCK,
	
	// image ctrl ----------------------------
	_CPT_BT_IMAGE_FILE,
	_CPT_B_IMAGE_LOCAL,
	_CPT_I_IMAGE_U1,
	_CPT_I_IMAGE_V1,
	_CPT_I_IMAGE_U2,
	_CPT_I_IMAGE_V2,
	_CPT_B_IMAGE_SIZE,
	_CPT_B_IMAGE_REVERSE,
	_CPT_I_IMAGE_ROTATE,

	// slider ctrl ----------------------------
	_CPT_E_SLIDER_TYPE,
	_CPT_E_SLIDER_ORIENTATION,
	_CPT_I_SLIDER_MIN,
	_CPT_I_SLIDER_MAX,
	_CPT_I_SLIDER_BAR_W,
	_CPT_I_SLIDER_BAR_H,
	//Normal uv
	_CPT_BT_SEL_BAR_UV,
	_CPT_BT_DEF_BAR_UV,
	_CPT_BT_DIS_BAR_UV,
	_CPT_BT_SLIDER_LINE_UV,
	
	//Etc uv
	_CPT_BT_SEL_BAR1_UV,
	_CPT_BT_DEF_BAR1_UV,
	_CPT_BT_DIS_BAR1_UV,
	_CPT_BT_SEL_BAR2_UV,
	_CPT_BT_DEF_BAR2_UV,
	_CPT_BT_DIS_BAR2_UV,

	_CPT_BT_SLIDER_BASE_UV,

	//----------------------------
	//position & size - selected windows Group
	_CPT_I_ADD_X,
	_CPT_I_ADD_Y,
	_CPT_I_ADD_WIDTH,
	_CPT_I_ADD_HEIGHT,
	
	//----------------------------
	//Edit font
	_CPT_S_ATTR_NAME,
	_CPT_B_ATTR_EDIT,
	_CPT_B_ATTR_RESET,
	_CPT_E_ATTR_TYPE,
	_CPT_CLR_ATTR_COLOR_RGB,
	_CPT_I_ATTR_COLOR_A,
	_CPT_I_ATTR_SCALE_WIDTH,
	_CPT_I_ATTR_SCALE_HIEGHT,
	_CPT_E_ATTR_STYLE,
	_CPT_E_ATTR_ALIGN,
	_CPT_I_ATTR_CHAR_SPACE,
	_CPT_I_ATTR_LINE_SPACE,
	_CPT_B_ATTR_WRAP,
	_CPT_B_ATTR_TRUNCATE,
	_CPT_B_ATTR_IGNORE,

	//----------------------------
	//Edit Image
	_CPT_S_TEX_UNIT_NAME,
	_CPT_S_TEX_UNIT_ID,
	_CPT_S_TEX_FILE,
	_CPT_S_TEX_LOCAL,
	_CPT_S_UV_NAME,
	_CPT_S_UV_ID,
	_CPT_I_UV_U1,
	_CPT_I_UV_V1,
	_CPT_I_UV_U2,
	_CPT_I_UV_V2,
	_CPT_I_UV_WIDTH,
	_CPT_I_UV_HEIGHT,
	_CPT_B_UV_SHOW_BLOCK,
	_CPT_B_UV_COLOR_BLOCK,
	_CPT_B_UV_COLOR_SEL_BLOCK,
	_CPT_B_UV_SHOW_ID,	
	_CPT_B_UV_COLOR_ID,
	_CPT_B_UV_COLOR_SEL_ID,

	_CPT_END,
};

enum _VALUETYPE {
	_VT_UNKNOWN = -1,
	_VT_BOOL,
	_VT_INT,
	_VT_CHAR,
	_VT_SIZE,
	_VT_VECTOR3,
	_VT_VECTOR4,
	_VT_COLOR,
	_VT_ENUM,
	_VT_FONT,
	_VT_BUTTON,
	_VT_UVTABLE,
	_VT_BLOCKTABLE,
};


#define _tag_MISC				_T("Misc")
#define _tag_ATTR				_T("Attribute")
#define _tag_POS_SIZE			_T("Position and Size")
							
							
//type						
#define _tag_BUTTON				_T("Button Info")
#define _tag_TEXT				_T("Text Info")
#define _tag_LIST				_T("List Info")
#define _tag_SLIDER				_T("Slider Info")


//image 관련
#define _tag_IMGMODE			_T("Image Mode")

//uv mode
#define _tag_UVMODE				_T("UV Mode")
#define _tag_LBS_UVMODE			_T("Select Box UV")			//listbox
#define _tag_SD_NMR_UVMODE		_T("Normal UV")				//slider Normal
#define _tag_SD_GAUGE_UVMODE	_T("Gauge UV")				//slider Gauge
#define _tag_SD_ETC_UVMODE		_T("Etc UV")				//slider Numeral & Text

//block mode
#define _tag_BLOCKMODE			_T("Block Mode")
#define _tag_LBS_BKMODE			_T("Select Box Block")		//listbox

//file mode
#define _tag_FILEMODE			_T("Image File Mode")

//기타
#define _tag_UV_BLOCKMODE		_T("UV, Block Mode")





struct PropertyDefine
{
	char				*szStr;
	_CTRL_PROPERTY_TYPE	nCPT;
	_VALUETYPE			nVT;
	char				*szDescription; // 세부설명..
	BOOL				bEnable;
};


struct ParameterDefine
{
	char			*szStr;
	PropertyDefine	*pPropertyDef;
	char			*szDescription; // 세부설명..
};


extern ParameterDefine g_Param_CommonSetting[];


struct ParameterCtrl
{
	ParameterDefine	*pParameterDef;
};


extern ParameterDefine g_Param_Selected[];
extern ParameterDefine g_Param_Layer[];
extern ParameterDefine g_Param_Button[];
extern ParameterDefine g_Param_Listbox[];
extern ParameterDefine g_Param_ListCtrl[];
extern ParameterDefine g_Param_Text[];
extern ParameterDefine g_Param_ImageCtrl[];
extern ParameterDefine g_Param_Slider[];

extern ParameterDefine g_Param_EditingFontAttr[];
extern ParameterDefine g_Param_EditingImage[];
extern ParameterDefine g_Param_EditingImageCondition[];
#endif